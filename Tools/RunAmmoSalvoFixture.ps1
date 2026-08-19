# CarFight Ammo+Salvo fixture 생성 runner
# Version: v1.0.0
# Date: 2026-08-14
# Changelog:
# - v1.0.0: UE 5.8 PythonScript commandlet로 CreateAmmoSalvoFixture.py를 실행하고 결과 JSON을 판정합니다.
# Migration:
# - CarFight 공식 엔진 D:\UnrealEngine_Source와 UE\CarFight_Re.uproject만 사용합니다.

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

# 현재 저장소 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build commandlet 실행 파일입니다.
$EditorCommandlet = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
# 이번 fixture를 생성하는 Unreal Python 스크립트입니다.
$PythonScript = Join-Path $PSScriptRoot 'CreateAmmoSalvoFixture.py'
# Python 스크립트가 기록하는 결정적 결과 JSON입니다.
$ResultJson = Join-Path $RepositoryRoot 'UE\Saved\CarFight\AmmoSalvoFixtureResult.json'

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
$EngineExitCode = $LASTEXITCODE

if (-not (Test-Path -LiteralPath $ResultJson -PathType Leaf)) {
    throw "Python commandlet 결과 JSON이 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

$Result = Get-Content -Raw -Encoding utf8 -LiteralPath $ResultJson | ConvertFrom-Json
Write-Host "RESULT_JSON=$ResultJson"
Write-Host "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Host ($Result | ConvertTo-Json -Depth 12 -Compress)

if ([string]$Result.status -ne 'success') {
    exit 1
}

# 현재 실행 중인 Editor의 MCP 포트 충돌 로그가 별도 commandlet 프로세스에 남을 수 있으므로,
# fixture 결과 JSON이 success면 자산 생성 결과를 우선합니다.
exit 0
