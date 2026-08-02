# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-02
# Description: CF-FQ-030 MG-P0-01~04 Direct 미사일 Runtime Automation 실행기
# Scope: 공식 CarFight 엔진에서 DirectRuntimeContract를 실행하고 정확한 테스트 존재·Success와 결과 JSON을 검증합니다.
# Changelog:
# - v1.0.0: 단일 Direct Missile Runtime Contract 무인 실행과 UTF-8 결과 JSON 생성 추가.
# Migration:
# - 이 스크립트는 소스와 에셋을 수정하지 않으며 UE/Saved/Automation/MissileDirect 보고서만 갱신합니다.

[CmdletBinding()]
param(
    # CarFight가 사용하는 공식 Unreal Engine 소스 루트입니다.
    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 현재 스크립트가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# Tools 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# Automation을 실행할 Unreal 프로젝트 파일입니다.
$ProjectFilePath = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 프로젝트 규칙에서 고정한 공식 UnrealEditor-Cmd 실행 파일입니다.
$UnrealEditorCommandPath = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

# Automation 원본 보고서를 저장할 디렉터리입니다.
$AutomationReportDirectory = Join-Path $RepositoryRoot 'UE\Saved\Automation\MissileDirect'

# Unreal Automation 표준 보고서 JSON 경로입니다.
$AutomationIndexPath = Join-Path $AutomationReportDirectory 'index.json'

# Admin process.status가 읽을 축약 결과 JSON 경로입니다.
$ResultJsonPath = Join-Path $AutomationReportDirectory 'result.json'

# 이번 실행에서 정확히 실행할 Direct Missile Runtime 테스트입니다.
$AutomationTestPath = 'CarFight.Missile.MG_P0_01_04.DirectRuntimeContract'

if (-not (Test-Path -LiteralPath $UnrealEditorCommandPath -PathType Leaf))
{
    throw "공식 UnrealEditor-Cmd.exe를 찾을 수 없습니다: $UnrealEditorCommandPath"
}
if (-not (Test-Path -LiteralPath $ProjectFilePath -PathType Leaf))
{
    throw "CarFight 프로젝트 파일을 찾을 수 없습니다: $ProjectFilePath"
}

if (Test-Path -LiteralPath $AutomationReportDirectory)
{
    Remove-Item -LiteralPath $AutomationReportDirectory -Recurse -Force
}
New-Item -ItemType Directory -Path $AutomationReportDirectory -Force | Out-Null

# 정확한 미사일 테스트를 실행하고 Automation Queue 종료 뒤 에디터를 닫을 인수입니다.
$EditorArguments = @(
    $ProjectFilePath,
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI',
    '-NoSound',
    '-stdout',
    '-FullStdOutLogOutput',
    "-ReportExportPath=$AutomationReportDirectory",
    "-ExecCmds=Automation RunTests $AutomationTestPath; Quit",
    '-TestExit=Automation Test Queue Empty',
    '-log'
)

Write-Host '[CarFight] Running Direct Missile Runtime automation...'
Write-Host "Test: $AutomationTestPath"

Push-Location (Join-Path $RepositoryRoot 'UE')
try
{
    & $UnrealEditorCommandPath @EditorArguments
    # UnrealEditor-Cmd의 실제 종료 코드입니다.
    $EditorExitCode = $LASTEXITCODE
}
finally
{
    Pop-Location
}

# 보고서 누락 때도 남길 기본 구조화 결과입니다.
$ResultSummary = [ordered]@{
    schema_version = '1.0.0'
    generated_at_utc = [DateTime]::UtcNow.ToString('o')
    test_path = $AutomationTestPath
    editor_exit_code = $EditorExitCode
    report_index_path = $AutomationIndexPath
    report_generated = $false
    found = $false
    state = 'Missing'
    duration_seconds = 0.0
    passed = $false
}

if (Test-Path -LiteralPath $AutomationIndexPath -PathType Leaf)
{
    # Unreal Automation이 UTF-8로 기록한 전체 보고서입니다.
    $AutomationReportText = [System.IO.File]::ReadAllText(
        $AutomationIndexPath,
        [System.Text.UTF8Encoding]::new($false))

    # 전체 테스트 결과를 조회할 Automation 보고서 객체입니다.
    $AutomationReport = $AutomationReportText | ConvertFrom-Json

    # 정확한 fullTestPath와 일치하는 테스트 결과입니다.
    $MatchedTest = @($AutomationReport.tests | Where-Object { $_.fullTestPath -eq $AutomationTestPath } | Select-Object -First 1)
    $ResultSummary.report_generated = $true
    if ($MatchedTest.Count -eq 1)
    {
        $ResultSummary.found = $true
        $ResultSummary.state = [string]$MatchedTest[0].state
        if ($null -ne $MatchedTest[0].duration)
        {
            $ResultSummary.duration_seconds = [double]$MatchedTest[0].duration
        }
    }
    $ResultSummary.passed = (
        $EditorExitCode -eq 0 -and
        $ResultSummary.found -and
        $ResultSummary.state -eq 'Success')
}

# Admin과 다음 세션이 안정적으로 읽을 UTF-8 without BOM 결과 JSON입니다.
$ResultJsonText = $ResultSummary | ConvertTo-Json -Depth 5
[System.IO.File]::WriteAllText(
    $ResultJsonPath,
    $ResultJsonText,
    [System.Text.UTF8Encoding]::new($false))

Write-Host "RESULT_JSON_PATH=$ResultJsonPath"
Write-Host "[CarFight] Direct Missile Runtime state: $($ResultSummary.state)"

if (-not $ResultSummary.passed)
{
    Write-Host '[CarFight] Direct Missile Runtime automation failed.'
    exit 1
}

Write-Host '[CarFight] Direct Missile Runtime automation passed.'
exit 0
