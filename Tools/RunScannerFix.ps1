# CarFight Scanner P0-06 fixture correction runner
# Version: v1.1.0
# Date: 2026-08-18
# Changelog:
# - v1.1.0: dedicated test PlayerPawn/GameMode + map override 교정 결과를 판정하도록 갱신.
# - v1.0.0: 배치 Pawn AutoPossess 기반 exact-1 교정 runner 최초 추가.
# Migration:
# - CarFight 공식 엔진 D:\UnrealEngine_Source와 UE\CarFight_Re.uproject만 사용합니다.
# - 실행 전 사용자가 연 CarFight Editor를 종료한 상태에서만 사용합니다.
# - production asset/source는 변경하지 않고 Scanner test-only Blueprint와 TestMap_ScannerP0만 저장합니다.

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

# 현재 저장소 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build commandlet 실행 파일입니다.
$EditorCommandlet = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
# Scanner P0-06 fixture를 교정하는 Unreal Python 스크립트입니다.
$PythonScript = Join-Path $PSScriptRoot 'FixScannerFixture.py'
# Python 스크립트가 기록하는 결정적 결과 JSON입니다.
$ResultJson = Join-Path $RepositoryRoot 'UE\Saved\CarFight\ScannerFixtureFixResult.json'

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    throw "CarFight uproject를 찾을 수 없습니다: $ProjectFile"
}
if (-not (Test-Path -LiteralPath $EditorCommandlet -PathType Leaf)) {
    throw "CarFight 공식 UnrealEditor-Cmd.exe를 찾을 수 없습니다: $EditorCommandlet"
}
if (-not (Test-Path -LiteralPath $PythonScript -PathType Leaf)) {
    throw "Scanner fixture Python 스크립트를 찾을 수 없습니다: $PythonScript"
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
    throw "Scanner fixture 결과 JSON이 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# Python 교정 스크립트가 기록한 UTF-8 결과 계약입니다.
$Result = Get-Content -Raw -Encoding utf8 -LiteralPath $ResultJson | ConvertFrom-Json
Write-Host "RESULT_JSON=$ResultJson"
Write-Host "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Host ($Result | ConvertTo-Json -Depth 16 -Compress)

if ([string]$Result.status -ne 'success') {
    exit 1
}
if (-not [bool]$Result.player_pawn_cdo_scanner_readback) {
    exit 1
}
if (-not [bool]$Result.game_mode_default_pawn_readback) {
    exit 1
}
if (-not [bool]$Result.map_game_mode_override_readback) {
    exit 1
}
if ([bool]$Result.production_asset_mutation -or [bool]$Result.placed_vehicle_mutation -or [bool]$Result.runtime_source_mutation) {
    exit 1
}

exit 0
