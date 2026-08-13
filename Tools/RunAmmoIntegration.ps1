# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-13
# Description: ApplyAmmoIntegration.py를 공식 UnrealEditor-Cmd에서 Dry Run 또는 Apply로 실행합니다.
# Scope: AMMO-P0-08 Heavy/Ripple finite-ammo 격리 DataAsset, Fitting Snapshot과 사용자 PIE 준비 맵을 생성·검증합니다.
# Changelog:
# - v1.0.0: 기본 Dry Run, 명시적 -Apply, Heavy/Ripple 체인·맵 연결·원본 SHA-256 보호 검증을 추가.
# Migration:
# - -Apply를 전달하지 않으면 .uasset 또는 .umap을 생성하거나 저장하지 않습니다.
# - Apply 성공 후 TestMap_AmmoHeavy와 TestMap_AmmoRipple은 사용자 PIE 직전 준비 상태입니다.

[CmdletBinding()]
param(
    # CarFight가 사용하는 공식 Unreal Engine 5.8 Source Build 루트입니다.
    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source',

    # 실제 P0-08 격리 에셋과 PIE 준비 맵 저장을 허용하는 명시적 스위치입니다.
    [Parameter(Mandatory = $false)]
    [switch]$Apply
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 이 PowerShell 스크립트가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# Tools 디렉터리의 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# Unreal Python Commandlet을 실행할 CarFight 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# P0-08 격리 finite-ammo 에셋과 맵을 생성·검증할 Unreal Python 스크립트입니다.
$PythonScript = Join-Path $ToolsDirectory 'ApplyAmmoIntegration.py'

# 프로젝트 규칙에서 고정한 공식 UnrealEditor-Cmd 실행 파일입니다.
$EditorCommand = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

# UnrealEditor-Cmd를 실행할 프로젝트 작업 디렉터리입니다.
$UnrealWorkingDirectory = Join-Path $RepositoryRoot 'UE'

# Python 도구가 기록할 구조화 결과 JSON 경로입니다.
$ReportPath = Join-Path $UnrealWorkingDirectory 'Saved\AmmoIntegrationApply\report.json'

foreach ($RequiredFile in @($ProjectFile, $PythonScript, $EditorCommand))
{
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf))
    {
        throw "Required file was not found: $RequiredFile"
    }
}

if (Test-Path -LiteralPath $ReportPath -PathType Leaf)
{
    Remove-Item -LiteralPath $ReportPath -Force
}

# 실행 전 환경 변수 값을 종료 후 복원하기 위해 보존합니다.
$PreviousApplyValue = $env:CARFIGHT_AMMO_INTEGRATION_APPLY

# Unreal Python이 Dry Run 또는 Apply를 선택할 현재 실행 환경 변수입니다.
$env:CARFIGHT_AMMO_INTEGRATION_APPLY = if ($Apply) { '1' } else { '0' }

# 공식 UnrealEditor-Cmd PythonScript Commandlet에 전달할 안전한 인수 목록입니다.
$EditorArguments = @(
    $ProjectFile,
    '-run=pythonscript',
    "-script=$PythonScript",
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI'
)

Write-Host '[CarFight] AMMO-P0-08 Integration Asset Tool'
Write-Host ("Mode: {0}" -f $(if ($Apply) { 'Apply' } else { 'DryRun' }))

Push-Location $UnrealWorkingDirectory
try
{
    & $EditorCommand @EditorArguments

    # UnrealEditor-Cmd가 반환한 실제 프로세스 종료 코드입니다.
    $EditorExitCode = $LASTEXITCODE
}
finally
{
    Pop-Location
    if ($null -eq $PreviousApplyValue)
    {
        Remove-Item Env:CARFIGHT_AMMO_INTEGRATION_APPLY -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_AMMO_INTEGRATION_APPLY = $PreviousApplyValue
    }
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf))
{
    throw "AMMO-P0-08 report was not generated. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

# Unreal Python이 UTF-8로 기록한 P0-08 보고서 전체 텍스트입니다.
$ReportText = [System.IO.File]::ReadAllText(
    $ReportPath,
    [System.Text.UTF8Encoding]::new($false))

# 실행 성공과 P0-08 사후조건을 검증할 구조화 보고서 객체입니다.
$Report = $ReportText | ConvertFrom-Json

if (-not $Report.success)
{
    throw "AMMO-P0-08 report returned success=false. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

if (-not $Report.protected_contract_passed)
{
    throw "AMMO-P0-08 protected source contract failed: $ReportPath"
}

if ($Apply)
{
    if (-not $Report.heavy_chain_valid -or -not $Report.ripple_chain_valid)
    {
        throw "AMMO-P0-08 finite-ammo Fitting chain validation failed: $ReportPath"
    }
    if (-not $Report.heavy_map_connected -or -not $Report.ripple_map_connected)
    {
        throw "AMMO-P0-08 PIE preparation map connection failed: $ReportPath"
    }
}

if ($EditorExitCode -ne 0)
{
    Write-Warning "UnrealEditor-Cmd exited with code $EditorExitCode after a successful AMMO-P0-08 report. Review unrelated plugin shutdown errors if present."
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host "EDITOR_EXIT_CODE=$EditorExitCode"
Write-Host '[CarFight] AMMO-P0-08 Integration Asset Tool completed successfully.'
exit 0
