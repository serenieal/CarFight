# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 2.0.0
# Date: 2026-07-31
# Description: CF-FQ-033 차량 방어·손상 Runtime Automation 실행기
# Scope: 공식 CarFight 엔진 경로에서 CarFight.Damage 전체 테스트를 실행하고 Unreal 보고서와 요약 JSON을 생성합니다.
# Changelog:
# - v2.0.0: DR-P0-01 DataContract와 DR-P0-02 Health·Shield·Armor·Penetration·Regeneration 전체 6개 이상 테스트 실행으로 확장.
# - v1.0.0: CarFight.Damage.DR_P0_01.DataContract 전용 무인 실행과 결과 JSON 생성을 추가.
# Migration:
# - 이 스크립트는 소스와 에셋을 수정하지 않으며 UE/Saved/Automation/VehicleDefense 아래의 테스트 보고서만 갱신합니다.
# - passed는 CarFight.Damage 필터의 일치 테스트가 6개 이상이고 모두 Success일 때만 True입니다.

[CmdletBinding()]
param(
    # CarFight가 사용하는 Unreal Engine 소스 빌드 루트입니다.
    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 저장소 루트 경로입니다.
$RepositoryRoot = Split-Path -Parent $PSScriptRoot

# CarFight Unreal 프로젝트 파일 경로입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 공식 Unreal Editor Commandlet 실행 파일 경로입니다.
$EditorCommand = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

# Unreal 명령을 실행할 프로젝트 작업 디렉터리입니다.
$UnrealWorkingDirectory = Join-Path $RepositoryRoot 'UE'

# Unreal Automation HTML·JSON 보고서 출력 폴더입니다.
$AutomationReportDirectory = Join-Path $UnrealWorkingDirectory 'Saved\Automation\VehicleDefense'

# Unreal Automation 기본 보고서 JSON 경로입니다.
$AutomationIndexPath = Join-Path $AutomationReportDirectory 'index.json'

# MCP가 구조화된 결과를 읽을 수 있도록 생성할 요약 JSON 경로입니다.
$ResultJsonPath = Join-Path $AutomationReportDirectory 'result.json'

# 이번 실행에서 사용할 CarFight 차량 방어·손상 Automation 상위 필터입니다.
$AutomationTestFilter = 'CarFight.Damage'

# DR-P0-01 단일 회귀와 DR-P0-02 신규 5개를 합친 최소 기대 테스트 수입니다.
$MinimumExpectedTestCount = 6

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    throw "CarFight project file was not found: $ProjectFile"
}

if (-not (Test-Path -LiteralPath $EditorCommand -PathType Leaf)) {
    throw "UnrealEditor-Cmd.exe was not found: $EditorCommand"
}

if (Test-Path -LiteralPath $AutomationReportDirectory) {
    Remove-Item -LiteralPath $AutomationReportDirectory -Recurse -Force
}

New-Item -ItemType Directory -Path $AutomationReportDirectory -Force | Out-Null

# UnrealEditor-Cmd에 전달할 무인 Automation 인자 목록입니다.
$EditorArguments = @(
    $ProjectFile,
    '-unattended',
    '-nop4',
    '-NullRHI',
    '-NoSound',
    '-NoSplash',
    '-stdout',
    '-FullStdOutLogOutput',
    "-ExecCmds=Automation RunTests $AutomationTestFilter; Quit",
    '-TestExit=Automation Test Queue Empty',
    "-ReportOutputPath=$AutomationReportDirectory"
)

Write-Host '[CarFight] Running complete VehicleDefense automation suite...'
Write-Host "Filter: $AutomationTestFilter"
Write-Host "Report: $AutomationReportDirectory"

Push-Location $UnrealWorkingDirectory
try {
    & $EditorCommand @EditorArguments
    # UnrealEditor-Cmd 프로세스의 실제 종료 코드입니다.
    $EditorExitCode = $LASTEXITCODE
}
finally {
    Pop-Location
}

# Unreal 보고서가 없거나 읽히지 않을 때도 남길 기본 결과입니다.
$Result = [ordered]@{
    schema_version = '1.0.0'
        test_filter = $AutomationTestFilter
    minimum_expected_test_count = $MinimumExpectedTestCount
    editor_exit_code = $EditorExitCode
    report_index_path = $AutomationIndexPath
    report_generated = $false
    succeeded = 0
    succeeded_with_warnings = 0
    failed = 0
    not_run = 0
    in_process = 0
    total_duration_seconds = 0.0
    matched_test_count = 0
    matched_test_states = @()
    passed = $false
}

if (Test-Path -LiteralPath $AutomationIndexPath -PathType Leaf) {
    # Unreal Automation이 생성한 UTF-8 JSON 보고서입니다.
    $AutomationReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $AutomationIndexPath | ConvertFrom-Json

        # CarFight.Damage 상위 필터 아래에서 실제 실행된 전체 테스트 결과 목록입니다.
    $MatchedTests = @($AutomationReport.tests | Where-Object { $_.fullTestPath -like "$AutomationTestFilter*" })

    # 일치한 테스트 중 Success가 아닌 결과 목록입니다.
    $NonSuccessMatchedTests = @($MatchedTests | Where-Object { $_.state -ne 'Success' })

    $Result.report_generated = $true
    $Result.succeeded = [int]$AutomationReport.succeeded
    $Result.succeeded_with_warnings = [int]$AutomationReport.succeededWithWarnings
    $Result.failed = [int]$AutomationReport.failed
    $Result.not_run = [int]$AutomationReport.notRun
    $Result.in_process = [int]$AutomationReport.inProcess
    $Result.total_duration_seconds = [double]$AutomationReport.totalDuration
    $Result.matched_test_count = $MatchedTests.Count
    $Result.matched_test_states = @($MatchedTests | ForEach-Object { [string]$_.state })
    $Result.passed = (
        $EditorExitCode -eq 0 -and
                $MatchedTests.Count -ge $MinimumExpectedTestCount -and
        $NonSuccessMatchedTests.Count -eq 0 -and
        $AutomationReport.failed -eq 0 -and
        $AutomationReport.notRun -eq 0 -and
        $AutomationReport.inProcess -eq 0
    )
}

# MCP 구조화 파서와 호환되는 UTF-8 BOM 없는 JSON 문자열입니다.
$ResultJsonText = $Result | ConvertTo-Json -Depth 5

# Windows PowerShell 5.1에서도 BOM 없는 UTF-8로 결과 JSON을 기록합니다.
[System.IO.File]::WriteAllText(
    $ResultJsonPath,
    $ResultJsonText,
    [System.Text.UTF8Encoding]::new($false))

Write-Host "RESULT_JSON_PATH=$ResultJsonPath"

if (-not $Result.passed) {
        Write-Host '[CarFight] VehicleDefense automation suite failed.'
    exit 1
}

Write-Host '[CarFight] VehicleDefense automation suite passed.'
exit 0
