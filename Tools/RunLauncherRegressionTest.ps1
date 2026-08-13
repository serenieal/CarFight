# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-06
# Description: ApplyLauncherRegressionTest.py를 공식 UnrealEditor-Cmd에서 Dry Run 또는 Apply로 실행합니다.
# Scope: DR-PIE-06 Salvo·Ripple 격리 DataAsset과 TestMap 복제본 생성·연결·원본 보호 보고서를 검증합니다.
# Changelog:
# - v1.0.0: 기본 Dry Run, 명시적 -Apply, Salvo·Ripple 체인·맵 Player0 연결·SHA-256 보호 사후조건 검증 추가.
# Migration:
# - -Apply를 전달하지 않으면 .uasset 또는 .umap을 생성하거나 저장하지 않습니다.
# - Apply 성공 후 Salvo는 TestMap_DRSalvo, Ripple은 TestMap_DRRipple에서 검증합니다.

[CmdletBinding()]
param(
    # CarFight가 사용하는 공식 Unreal Engine 소스 빌드 루트입니다.
    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source',

    # 실제 Launcher 회귀 테스트 에셋과 맵 생성을 허용하는 명시적 스위치입니다.
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

# Launcher 회귀 에셋과 맵을 생성·검증할 Unreal Python 스크립트입니다.
$PythonScript = Join-Path $ToolsDirectory 'ApplyLauncherRegressionTest.py'

# 프로젝트 규칙에서 고정한 공식 UnrealEditor-Cmd 실행 파일입니다.
$EditorCommand = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

# UnrealEditor-Cmd를 실행할 프로젝트 작업 디렉터리입니다.
$UnrealWorkingDirectory = Join-Path $RepositoryRoot 'UE'

# Python 도구가 기록할 구조화 결과 JSON 경로입니다.
$ReportPath = Join-Path $UnrealWorkingDirectory 'Saved\LauncherRegressionApply\report.json'

foreach ($RequiredFile in @($ProjectFile, $PythonScript, $EditorCommand))
{
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf))
    {
        throw "Required file was not found: $RequiredFile"
    }
}

# 이전 보고서가 현재 실행 결과로 오인되지 않도록 실행 전에 제거합니다.
if (Test-Path -LiteralPath $ReportPath -PathType Leaf)
{
    Remove-Item -LiteralPath $ReportPath -Force
}

# 호출 전 환경 변수 값을 실행 후 원래 상태로 복구하기 위해 보존합니다.
$PreviousApplyValue = $env:CARFIGHT_LAUNCHER_REGRESSION_APPLY

# Unreal Python 도구가 Dry Run 또는 Apply를 선택할 환경 변수입니다.
$env:CARFIGHT_LAUNCHER_REGRESSION_APPLY = if ($Apply) { '1' } else { '0' }

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

Write-Host '[CarFight] Launcher Regression Test Asset Tool'
Write-Host ("Mode: {0}" -f $(if ($Apply) { 'Apply' } else { 'DryRun' }))

Push-Location $UnrealWorkingDirectory
try
{
    & $EditorCommand @EditorArguments

    # UnrealEditor-Cmd 프로세스가 반환한 실제 종료 코드입니다.
    $EditorExitCode = $LASTEXITCODE
}
finally
{
    Pop-Location

    if ($null -eq $PreviousApplyValue)
    {
        Remove-Item Env:CARFIGHT_LAUNCHER_REGRESSION_APPLY -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_LAUNCHER_REGRESSION_APPLY = $PreviousApplyValue
    }
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf))
{
    throw "Launcher regression report was not generated. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

# Unreal Python이 UTF-8로 기록한 Launcher 회귀 보고서 전체 텍스트입니다.
$ReportText = [System.IO.File]::ReadAllText(
    $ReportPath,
    [System.Text.UTF8Encoding]::new($false))

# 실행 성공과 모든 Launcher 회귀 사후조건을 확인할 구조화 보고서 객체입니다.
$Report = $ReportText | ConvertFrom-Json

if (-not $Report.success)
{
    throw "Launcher regression report returned success=false. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

if (-not $Report.source_contract_valid)
{
    throw "Launcher source Salvo·Muzzle·Projectile contract was not verified: $ReportPath"
}

if (-not $Report.salvo_chain_valid)
{
    throw "Launcher Salvo test chain was not verified: $ReportPath"
}

if (-not $Report.ripple_chain_valid)
{
    throw "Launcher Ripple test chain was not verified: $ReportPath"
}

if ($Apply -and -not $Report.salvo_map_connected)
{
    throw "Launcher Salvo test map Player0 connection was not verified: $ReportPath"
}

if ($Apply -and -not $Report.ripple_map_connected)
{
    throw "Launcher Ripple test map Player0 connection was not verified: $ReportPath"
}

if (-not $Report.protected_contract_passed)
{
    throw "Launcher regression protected source contract was not verified: $ReportPath"
}

# 플러그인 종료 오류가 있어도 Python 보고서 사후조건이 PASS면 결과를 보존하고 종료 코드를 함께 기록합니다.
if ($EditorExitCode -ne 0)
{
    Write-Warning "UnrealEditor-Cmd exited with code $EditorExitCode after a successful Launcher regression report. Review the Editor log for unrelated plugin errors."
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host "EDITOR_EXIT_CODE=$EditorExitCode"
Write-Host '[CarFight] Launcher Regression Test Asset Tool completed successfully.'
exit 0
