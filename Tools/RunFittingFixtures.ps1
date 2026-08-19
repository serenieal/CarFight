# CarFight Mobility Fitting Fixture runner
# Version: v1.0.0
# Date: 2026-08-17
# Changelog:
# - v1.0.0: UE 5.8 PythonScript commandlet로 CreateFittingFixtures.py를 실행하고 Snapshot 질량 결과 JSON을 판정합니다.
# Migration:
# - CarFight 공식 엔진 D:\UnrealEngine_Source와 UE\CarFight_Re.uproject만 사용합니다.
# - 실행 전 다른 CarFight Editor가 떠 있지 않은 AI-owned/offline fixture 작업에서 사용합니다.

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

# 현재 저장소 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build commandlet 실행 파일입니다.
$EditorCommandlet = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
# Mobility Fitting Fixture 3종을 생성·검증하는 Unreal Python 스크립트입니다.
$PythonScript = Join-Path $PSScriptRoot 'CreateFittingFixtures.py'
# Python 스크립트가 기록하는 결정적 결과 JSON입니다.
$ResultJson = Join-Path $RepositoryRoot 'UE\Saved\CarFight\FittingMobilityFixtureResult.json'

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    throw "CarFight uproject를 찾을 수 없습니다: $ProjectFile"
}
if (-not (Test-Path -LiteralPath $EditorCommandlet -PathType Leaf)) {
    throw "CarFight 공식 UnrealEditor-Cmd.exe를 찾을 수 없습니다: $EditorCommandlet"
}
if (-not (Test-Path -LiteralPath $PythonScript -PathType Leaf)) {
    throw "Fixture Python 스크립트를 찾을 수 없습니다: $PythonScript"
}

if (Test-Path -LiteralPath $ResultJson -PathType Leaf) {
    Remove-Item -LiteralPath $ResultJson -Force
}

# UE 5.8 PythonScript commandlet에 전달할 고정 인자입니다.
$CommandletArguments = @(
    $ProjectFile,
    '-run=pythonscript',
    "-script=$PythonScript",
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NoSound',
    '-NoLogTimes'
)

& $EditorCommandlet @CommandletArguments
# UnrealEditor-Cmd 프로세스의 실제 종료 코드입니다.
$EngineExitCode = $LASTEXITCODE

if (-not (Test-Path -LiteralPath $ResultJson -PathType Leaf)) {
    throw "Python commandlet 결과 JSON이 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# Python fixture가 기록한 UTF-8 결과 계약입니다.
$Result = Get-Content -Raw -Encoding utf8 -LiteralPath $ResultJson | ConvertFrom-Json
Write-Host "RESULT_JSON=$ResultJson"
Write-Host "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Host ($Result | ConvertTo-Json -Depth 12 -Compress)

if ([string]$Result.status -ne 'success') {
    exit 1
}
if (-not [bool]$Result.mass_order_passed) {
    exit 1
}

exit 0
