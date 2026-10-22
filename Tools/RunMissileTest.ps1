# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.3
# Date: 2026-08-02
# Description: ApplyMissileTest.py를 공식 UnrealEditor-Cmd에서 Dry Run 또는 Apply로 실행합니다.
# Scope: 신규 MissileTest 자산과 MissileDirectTest 복제 맵만 생성·검증합니다.
# Changelog:
# - v1.0.3: HLE-03 일회성 실증 완료 후 고정 쓰기 스위치와 실행 분기를 정리하고 관리형 Dry Run 계약만 유지합니다.
# - v1.0.2: 관리형 HLE-03 고정 1회 저장 실증을 격리된 단일 에셋·별도 보고서로 실행하고 동일 Project Editor·Commandlet 충돌을 출발 전에 확인합니다.
# - v1.0.1: 관리형 Project Dry Run은 Registry UE 신원과 실행별 독립 결과 JSON을 사용하고 기존 Apply 보고서를 보존합니다.
# - v1.0.0: 기본 Dry Run, 명시적 -Apply, UTF-8 결과 JSON 검증과 원본 보호 계약 확인 추가.
# Migration:
# - HLE-03 일회성 쓰기 Operation은 수락 후 등록을 해제했습니다. 기존 -Apply/DryRun 호출 계약은 유지합니다.
# - 관리형 GoPyMCP Project 호출은 별도 report_<id>.json을 생성합니다. 일반 직접 실행의 report.json 및 -Apply 사용법은 그대로 유지합니다.
# - -Apply를 전달하지 않으면 .uasset과 맵을 저장하지 않습니다.

[CmdletBinding()]
param(
    # CarFight 공식 Unreal Engine 소스 루트입니다.
    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source',

    # 신규 미사일 테스트 자산과 복제 맵을 실제 저장할지 여부입니다.
    [Parameter(Mandatory = $false)]
    [switch]$Apply
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 현재 스크립트가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# Tools 상위의 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# Automation과 Python Script가 사용할 Unreal 프로젝트 파일입니다.
$ProjectFilePath = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 미사일 테스트 자산과 복제 맵을 생성할 Unreal Python 스크립트입니다.
$PythonScriptPath = Join-Path $ToolsDirectory 'ApplyMissileTest.py'

# 프로젝트 규칙에서 고정한 공식 UnrealEditor-Cmd 실행 파일입니다.
$UnrealEditorCommandPath = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

# Unreal Python 실행 작업 디렉터리입니다.
$UnrealWorkingDirectory = Join-Path $RepositoryRoot 'UE'

# 구조화 실행 결과 JSON 경로입니다.
$ResultJsonPath = Join-Path $UnrealWorkingDirectory 'Saved\MissileTestApply\report.json'

# 관리형 Project 실행 여부는 Project Registry가 투영한 정확한 소유 신원으로만 판별합니다.
$ManagedProjectExecution = -not [string]::IsNullOrWhiteSpace($env:GOPYMCP_UE_PROJECT_ID)

# 기존 환경 값을 자식 실행 후 원상 복구하기 위해 보존합니다.
$PreviousReportId = $env:CARFIGHT_MISSILE_TEST_REPORT_ID

# 관리형 Dry Run만 사용하는 불투명 실행별 보고서 식별자입니다.
$ManagedReportId = ''
if ($ManagedProjectExecution)
{
    if ($Apply)
    {
        throw '관리형 Headless 작업은 Dry Run 전용입니다. 저장하려면 별도 허가된 Apply 경로를 사용하세요.'
    }
    if ($env:GOPYMCP_UE_PROJECT_ID -ne 'carfight' -or
        $env:GOPYMCP_UE_CAPABILITY_ID -ne 'unreal' -or
        $env:GOPYMCP_UE_TOOLCHAIN_VERSION -ne '5.8')
    {
        throw '관리형 Project/Capability/UE 버전 신원이 CarFight와 일치하지 않습니다.'
    }
    if ([string]::IsNullOrWhiteSpace($env:GOPYMCP_UE_PROJECT_FILE) -or
        [string]::IsNullOrWhiteSpace($env:GOPYMCP_UE_COMMANDLET_EXE) -or
        [string]::IsNullOrWhiteSpace($env:GOPYMCP_UE_ENGINE_ROOT))
    {
        throw '관리형 UE 실행 신원이 불완전합니다.'
    }

    # CarFight Checkout에 속하는 실제 프로젝트 파일을 고정된 기대 경로와 비교합니다.
    $RegisteredProjectFile = [System.IO.Path]::GetFullPath($env:GOPYMCP_UE_PROJECT_FILE)
    if (-not [string]::Equals($RegisteredProjectFile, [System.IO.Path]::GetFullPath($ProjectFilePath), [System.StringComparison]::OrdinalIgnoreCase))
    {
        throw '등록된 UE 프로젝트 파일이 현재 CarFight Checkout과 일치하지 않습니다.'
    }

    # Registry Toolchain의 명시적 Commandlet 실행 파일만 사용합니다.
    $RegisteredCommandlet = [System.IO.Path]::GetFullPath($env:GOPYMCP_UE_COMMANDLET_EXE)
    $ExpectedCommandlet = [System.IO.Path]::GetFullPath(
        (Join-Path $env:GOPYMCP_UE_ENGINE_ROOT 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'))
    if (-not [string]::Equals($RegisteredCommandlet, $ExpectedCommandlet, [System.StringComparison]::OrdinalIgnoreCase))
    {
        throw '등록된 Unreal Commandlet이 Project Toolchain 루트와 일치하지 않습니다.'
    }

    $ProjectFilePath = $RegisteredProjectFile
    $UnrealEditorCommandPath = $RegisteredCommandlet
    $ManagedReportId = [Guid]::NewGuid().ToString('N')
    $ResultJsonPath = Join-Path $UnrealWorkingDirectory ("Saved\MissileTestApply\report_{0}.json" -f $ManagedReportId)
}

foreach ($RequiredFilePath in @($ProjectFilePath, $PythonScriptPath, $UnrealEditorCommandPath))
{
    if (-not (Test-Path -LiteralPath $RequiredFilePath -PathType Leaf))
    {
        throw "필수 파일을 찾을 수 없습니다: $RequiredFilePath"
    }
}

# 기존 환경 변수 값을 실행 종료 뒤 복구하기 위해 저장합니다.
$PreviousApplyValue = $env:CARFIGHT_MISSILE_TEST_APPLY
$env:CARFIGHT_MISSILE_TEST_APPLY = if ($Apply) { '1' } else { '0' }
if ($ManagedProjectExecution)
{
    $env:CARFIGHT_MISSILE_TEST_REPORT_ID = $ManagedReportId
}
else
{
    # 직접 실행에서는 기존 고정 report.json 계약을 유지합니다.
    Remove-Item Env:CARFIGHT_MISSILE_TEST_REPORT_ID -ErrorAction SilentlyContinue
}

# Unreal Python Commandlet에 전달할 안전한 인수 목록입니다.
$EditorArguments = @(
    $ProjectFilePath,
    '-run=pythonscript',
    "-script=$PythonScriptPath",
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI',
    '-NoSound'
)

Write-Host '[CarFight] Direct Missile Test Asset Tool'
Write-Host ("Mode: {0}" -f $(if ($Apply) { 'Apply' } else { 'DryRun' }))

Push-Location $UnrealWorkingDirectory
try
{
    & $UnrealEditorCommandPath @EditorArguments
    # UnrealEditor-Cmd의 실제 종료 코드입니다.
    $EditorExitCode = $LASTEXITCODE
}
finally
{
    Pop-Location
    if ($null -eq $PreviousApplyValue)
    {
        Remove-Item Env:CARFIGHT_MISSILE_TEST_APPLY -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_MISSILE_TEST_APPLY = $PreviousApplyValue
    }
    if ($null -eq $PreviousReportId)
    {
        Remove-Item Env:CARFIGHT_MISSILE_TEST_REPORT_ID -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_MISSILE_TEST_REPORT_ID = $PreviousReportId
    }
}

if ($EditorExitCode -ne 0)
{
    throw "미사일 테스트 자산 Unreal Python 실행이 실패했습니다. ExitCode=$EditorExitCode"
}

if (-not (Test-Path -LiteralPath $ResultJsonPath -PathType Leaf))
{
    throw "미사일 테스트 자산 결과 JSON이 생성되지 않았습니다: $ResultJsonPath"
}

# UTF-8 without BOM 보고서를 읽어 보호 계약과 성공 상태를 검증합니다.
$ResultJsonText = [System.IO.File]::ReadAllText(
    $ResultJsonPath,
    [System.Text.UTF8Encoding]::new($false))
$Result = $ResultJsonText | ConvertFrom-Json

if (-not $Result.success)
{
    throw "미사일 테스트 자산 보고서가 success=false입니다: $ResultJsonPath"
}
if (-not $Result.protected_contract_passed)
{
    throw "기존 Rocket·Launcher·SUV·TestMap 보호 계약이 통과하지 않았습니다: $ResultJsonPath"
}
if ($ManagedProjectExecution -and (
    $Result.mode -ne 'dry_run' -or
    $Result.created_assets.Count -ne 0 -or
    $Result.saved_assets.Count -ne 0))
{
    throw "관리형 Dry Run의 실제 생성/저장 또는 모드 보고가 예상과 다릅니다: $ResultJsonPath"
}

Write-Host "RESULT_JSON_PATH=$ResultJsonPath"
Write-Host '[CarFight] Direct Missile Test Asset Tool completed successfully.'
exit 0
