# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-02
# Description: ApplyMissileTest.py를 공식 UnrealEditor-Cmd에서 Dry Run 또는 Apply로 실행합니다.
# Scope: 신규 MissileTest 자산과 MissileDirectTest 복제 맵만 생성·검증합니다.
# Changelog:
# - v1.0.0: 기본 Dry Run, 명시적 -Apply, UTF-8 결과 JSON 검증과 원본 보호 계약 확인 추가.
# Migration:
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

Write-Host "RESULT_JSON_PATH=$ResultJsonPath"
Write-Host '[CarFight] Direct Missile Test Asset Tool completed successfully.'
exit 0
