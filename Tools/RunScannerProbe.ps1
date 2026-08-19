# CarFight Scanner P0-06 fixture read-only probe runner
# Version: v1.0.0
# Date: 2026-08-18
# Changelog:
# - v1.0.0: UE 5.8 PythonScript commandlet로 ProbeScannerFixture.py를 실행하고 mutation0/save0 결과를 판정합니다.
# Migration:
# - CarFight 공식 엔진과 프로젝트만 사용하며 Asset 저장은 수행하지 않습니다.

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

# 현재 저장소 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# UE 5.8 Source Build commandlet 실행 파일입니다.
$EditorCommandlet = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
# read-only probe Python 스크립트입니다.
$PythonScript = Join-Path $PSScriptRoot 'ProbeScannerFixture.py'
# probe 결과 JSON입니다.
$ResultJson = Join-Path $RepositoryRoot 'UE\Saved\CarFight\ScannerFixtureProbeResult.json'

if (Test-Path -LiteralPath $ResultJson -PathType Leaf) {
    Remove-Item -LiteralPath $ResultJson -Force
}

# 고정 commandlet 인자입니다.
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
# 실제 Engine 종료 코드입니다.
$EngineExitCode = $LASTEXITCODE

if (-not (Test-Path -LiteralPath $ResultJson -PathType Leaf)) {
    throw "Scanner probe 결과 JSON이 없습니다. EngineExitCode=$EngineExitCode"
}

# UTF-8 결과 계약입니다.
$Result = Get-Content -Raw -Encoding utf8 -LiteralPath $ResultJson | ConvertFrom-Json
Write-Host "RESULT_JSON=$ResultJson"
Write-Host "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Host ($Result | ConvertTo-Json -Depth 16 -Compress)

if ([string]$Result.status -ne 'success' -or [int]$Result.mutation -ne 0 -or [int]$Result.save -ne 0) {
    exit 1
}

exit 0
