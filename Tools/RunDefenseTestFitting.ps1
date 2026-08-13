# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.1.0
# Date: 2026-08-03
# Description: CreateDefenseTestFitting.py를 UnrealEditor-Cmd에서 Dry Run 또는 Apply로 실행합니다.
# Scope: DR-P0-07 테스트 전용 장비·피팅 생성, Snapshot 검증과 M_VehicleDefensePIE 단일 차량 연결 결과를 검증합니다.
# Changelog:
# - v1.1.0: 테스트 전용 Weapon·Preset과 테스트 소유 질량 적용 결과 검증을 포함하도록 범위를 확장.
# - v1.0.0: 기본 Dry Run, 명시적 -Apply, JSON 사후조건 검증과 비관련 MCP HttpListener 오류 격리를 추가.
# Migration:
# - -Apply를 전달하지 않으면 신규 DataAsset과 테스트 맵을 저장하지 않습니다.
# - Apply 성공 판정은 Snapshot 유효성, 맵 연결과 공유 Launcher 보호 계약을 기준으로 합니다.

[CmdletBinding()]
param(
    # CarFight가 사용하는 Unreal Engine 소스 빌드 루트입니다.
    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source',

    # 실제 .uasset 생성과 테스트 맵 참조 저장을 허용하는 명시적 스위치입니다.
    [Parameter(Mandatory = $false)]
    [switch]$Apply
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 이 스크립트가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# Tools의 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# Unreal Python을 실행할 CarFight 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 방어 테스트 피팅 생성·연결을 수행할 Unreal Python 파일입니다.
$PythonScript = Join-Path $ToolsDirectory 'CreateDefenseTestFitting.py'

# 공식 소스 엔진의 UnrealEditor-Cmd 실행 파일입니다.
$EditorCommand = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

# Unreal 명령을 실행할 프로젝트 작업 디렉터리입니다.
$UnrealWorkingDirectory = Join-Path $RepositoryRoot 'UE'

# Python 도구가 기록할 구조화 결과 JSON 경로입니다.
$ReportPath = Join-Path $UnrealWorkingDirectory 'Saved\DefenseTestFittingApply\report.json'

foreach ($RequiredFile in @($ProjectFile, $PythonScript, $EditorCommand))
{
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf))
    {
        throw "Required file was not found: $RequiredFile"
    }
}

# 실행 전 기존 보고서가 현재 실행 결과로 오인되지 않도록 제거합니다.
if (Test-Path -LiteralPath $ReportPath -PathType Leaf)
{
    Remove-Item -LiteralPath $ReportPath -Force
}

# 호출 전 환경 변수 값을 실행 후 복구하기 위해 보존합니다.
$PreviousApplyValue = $env:CARFIGHT_DEFENSE_FITTING_APPLY

# Python 도구가 Dry Run 또는 Apply를 선택할 환경 변수입니다.
$env:CARFIGHT_DEFENSE_FITTING_APPLY = if ($Apply) { '1' } else { '0' }

# UnrealEditor-Cmd PythonScript Commandlet에 전달할 안전한 인수 목록입니다.
$EditorArguments = @(
    $ProjectFile,
    '-run=pythonscript',
    "-script=$PythonScript",
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI'
)

Write-Host '[CarFight] Defense Test Fitting Tool'
Write-Host ("Mode: {0}" -f $(if ($Apply) { 'Apply' } else { 'DryRun' }))

Push-Location $UnrealWorkingDirectory
try
{
    & $EditorCommand @EditorArguments

    # UnrealEditor-Cmd 프로세스의 실제 종료 코드입니다.
    $EditorExitCode = $LASTEXITCODE
}
finally
{
    Pop-Location

    if ($null -eq $PreviousApplyValue)
    {
        Remove-Item Env:CARFIGHT_DEFENSE_FITTING_APPLY -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_DEFENSE_FITTING_APPLY = $PreviousApplyValue
    }
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf))
{
    throw "Defense test fitting report was not generated. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

# Unreal Python이 UTF-8로 기록한 전체 적용 보고서입니다.
$ReportText = [System.IO.File]::ReadAllText(
    $ReportPath,
    [System.Text.UTF8Encoding]::new($false))

# Apply 결과와 보호 계약을 확인할 구조화 보고서 객체입니다.
$Report = $ReportText | ConvertFrom-Json

if (-not $Report.success)
{
    throw "Defense test fitting report returned success=false. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

if ($Apply -and -not $Report.snapshot_valid)
{
    throw "Defense test fitting Snapshot validation did not pass: $ReportPath"
}

if ($Apply -and -not $Report.map_connected)
{
    throw "Defense test fitting was not connected to M_VehicleDefensePIE: $ReportPath"
}

if ($Apply -and -not $Report.protected_contract_passed)
{
    throw "Defense test fitting protected contract was not verified: $ReportPath"
}

# 외부 MCP 플러그인의 HttpListener 포트 충돌이 있어도 Python 보고서의 완전한 사후조건이 PASS면 에셋 작업 결과를 보존합니다.
if ($EditorExitCode -ne 0)
{
    Write-Warning "UnrealEditor-Cmd exited with code $EditorExitCode after a successful Defense test fitting report. Review the Editor log for unrelated plugin errors."
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host "EDITOR_EXIT_CODE=$EditorExitCode"
Write-Host '[CarFight] Defense Test Fitting Tool completed successfully.'
exit 0
