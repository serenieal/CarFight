# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.15.0
# Date: 2026-08-13
# Description: CarFight 전투·피팅·탄약·HUD 런타임 회귀 Automation 실행기
# Scope: 공식 CarFight 엔진에서 Ammo HUD·Fitting Mass·저장 AssetChain, Damage, 실제 BP/DataAsset·저장 맵 1-Pawn·2-Pawn PIE Fitting·Defense Commit, Launcher, Projectile Launch·Pool 격리·요격·추진과 Direct Missile 회귀를 실행하고 필수 테스트 결과를 검증합니다.
# Changelog:
# - v1.15.0: CF-FQ-031 AMMO-P0-08 AssetChain을 필수 목록에 추가해 총 32개 회귀를 검증.
# - v1.14.0: CF-FQ-031 AMMO-P0-07 FittingMass를 필수 목록에 추가해 총 31개 회귀를 검증.
# - v1.13.0: CF-FQ-031 AMMO-P0-06 HUD를 필수 목록에 추가해 총 30개 회귀를 검증.
# - v1.12.0: CF-FQ-031 AMMO-P0-05 Reload를 필수 목록에 추가해 총 29개 회귀를 검증.
# - v1.11.0: CF-FQ-031 AMMO-P0-04 LauncherLock을 필수 목록에 추가해 총 28개 회귀를 검증.
# - v1.10.0: CF-FQ-031 AMMO-P0-03 FireTransaction을 필수 목록에 추가해 총 27개 회귀를 검증.
# - v1.9.0: CF-FQ-031 AMMO-P0-01 Contract와 AMMO-P0-02 Runtime을 필수 목록에 추가해 총 26개 회귀를 검증.
# - v1.8.0: CF-FQ-033 FIT-P0-05 DefenseMapTwoPawnPIE를 필수 목록에 추가해 플레이어 차량 + 대상 SUV를 포함한 총 24개 회귀를 검증.
# - v1.7.0: CF-FQ-033 FIT-P0-05 DefenseMapPIE를 필수 목록에 추가해 실제 M_VehicleDefensePIE 복제 Actor를 포함한 총 23개 회귀를 검증.
# - v1.6.0: CF-FQ-033 FIT-P0-05 DefensePIEPipeline을 필수 목록에 추가해 총 22개 회귀를 검증.
# - v1.5.0: CF-FQ-030 MG-P0-01~04 DirectRuntimeContract를 필수 목록에 추가해 총 21개 회귀를 검증.
# - v1.4.0: CF-FQ-034 FIT-P0-05 InitialMass를 필수 목록에 추가해 총 20개 회귀를 검증.
# - v1.3.0: CF-FQ-034 FIT-P0-04 RuntimeApply와 AtomicBoundary를 필수 목록에 추가해 총 19개 회귀를 검증.
# - v1.2.0: CF-FQ-034 FIT-P0-03 Compatibility와 MassSnapshot을 필수 목록에 추가해 총 17개 회귀를 검증.
# - v1.1.0: DR-P0-04 DebugBlueprintContract를 필수 목록에 추가해 총 15개 회귀를 검증.
# - v1.0.0: CarFight 전체 Automation 실행, DR-P0-03 필수 14개 테스트 존재·Success 검증과 요약 JSON 출력을 추가.
# Migration:
# - 이 스크립트는 소스와 에셋을 수정하지 않으며 UE/Saved/Automation/CombatRuntime 보고서만 갱신합니다.
# - 공식 엔진 D:\UnrealEngine_Source와 현재 저장소의 UE\CarFight_Re.uproject만 사용합니다.

[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 이 스크립트가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# Tools의 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# Automation을 실행할 CarFight Unreal 프로젝트 파일입니다.
$ProjectFilePath = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 프로젝트 규칙에서 고정한 공식 Unreal Editor 명령행 실행 파일입니다.
$UnrealEditorCommandPath = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

# Unreal Automation 원본 보고서를 저장할 디렉터리입니다.
$AutomationReportDirectory = Join-Path $RepositoryRoot 'UE\Saved\Automation\CombatRuntime'

# Unreal Automation이 생성하는 표준 index.json 경로입니다.
$AutomationIndexPath = Join-Path $AutomationReportDirectory 'index.json'

# Admin process.status가 읽을 수 있는 축약 결과 JSON 경로입니다.
$ResultJsonPath = Join-Path $AutomationReportDirectory 'result.json'

# 이번 회귀에서 실행할 CarFight Automation 상위 필터입니다.
$AutomationTestFilter = 'CarFight'

# DR-P0-04와 CF-FQ-029 보호 회귀에서 반드시 존재하고 Success여야 하는 테스트 목록입니다.
$RequiredTestPaths = @(
        'CarFight.Ammo.AMMO_P0_01.Contract',
    'CarFight.Ammo.AMMO_P0_02.Runtime',
        'CarFight.Ammo.AMMO_P0_03.FireTransaction',
        'CarFight.Ammo.AMMO_P0_04.LauncherLock',
        'CarFight.Ammo.AMMO_P0_05.Reload',
        'CarFight.Ammo.AMMO_P0_06.HUD',
        'CarFight.Ammo.AMMO_P0_07.FittingMass',
    'CarFight.Ammo.AMMO_P0_08.AssetChain',
    'CarFight.Damage.DR_P0_01.DataContract',
    'CarFight.Damage.DR_P0_02.HealthCompatibility',
    'CarFight.Damage.DR_P0_02.DirectionalArmor',
    'CarFight.Damage.DR_P0_02.ShieldArmorPenetration',
    'CarFight.Damage.DR_P0_02.OverflowLegacyFallback',
    'CarFight.Damage.DR_P0_02.ShieldRegeneration',
    'CarFight.Damage.DR_P0_03.RuntimeIntegration',
    'CarFight.Damage.DR_P0_04.DebugBlueprintContract',
    'CarFight.Fitting.FIT_P0_03.Compatibility',
    'CarFight.Fitting.FIT_P0_03.MassSnapshot',
    'CarFight.Fitting.FIT_P0_04.RuntimeApply',
    'CarFight.Fitting.FIT_P0_04.AtomicBoundary',
    'CarFight.Fitting.FIT_P0_05.InitialMass',
    'CarFight.Fitting.FIT_P0_05.DefensePIEPipeline',
    'CarFight.Fitting.FIT_P0_05.DefenseMapPIE',
    'CarFight.Fitting.FIT_P0_05.DefenseMapTwoPawnPIE',
    'CarFight.Launcher.LM_P0_02.MuzzleSequence',
    'CarFight.Launcher.LM_P0_03.FirePatternContract',
    'CarFight.Launcher.LM_P0_03B.SchedulerContract',
    'CarFight.Launcher.LM_P0_04.ReleaseContract',
    'CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract',
    'CarFight.Projectile.LM_P0_06.SourceIsolation',
    'CarFight.ProjectilePropulsion.PP_P0_01.RuntimeContract',
    'CarFight.Missile.MG_P0_01_04.DirectRuntimeContract'
)

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

# CarFight 상위 필터를 실행하고 Automation Queue가 비면 종료하도록 전달할 Editor 인수입니다.
$EditorArguments = @(
    $ProjectFilePath,
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nullrhi',
    "-ReportExportPath=$AutomationReportDirectory",
    "-ExecCmds=Automation RunTests $AutomationTestFilter; Quit",
    '-TestExit=Automation Test Queue Empty',
    '-log'
)

Write-Host '[CarFight] Running Damage, Launcher, Pool, propulsion and interception automation regressions...'
& $UnrealEditorCommandPath @EditorArguments
$EditorExitCode = $LASTEXITCODE

# 결과 JSON에 기록할 기본 요약 객체입니다.
$ResultSummary = [ordered]@{
    schema_version = '1.0.0'
    generated_at_utc = [DateTime]::UtcNow.ToString('o')
    test_filter = $AutomationTestFilter
    editor_exit_code = $EditorExitCode
    report_index_path = $AutomationIndexPath
    required_test_count = $RequiredTestPaths.Count
    matched_required_test_count = 0
    succeeded_required_test_count = 0
    failed_required_test_count = 0
    missing_required_test_count = 0
    total_reported_test_count = 0
    total_failed_test_count = 0
    passed = $false
    required_tests = @()
}

if (Test-Path -LiteralPath $AutomationIndexPath -PathType Leaf)
{
    # Unreal Automation이 UTF-8로 기록한 전체 보고서입니다.
    $AutomationReportText = [System.IO.File]::ReadAllText(
        $AutomationIndexPath,
        [System.Text.UTF8Encoding]::new($false))

    # 전체 테스트 상태를 조회할 Automation 보고서 객체입니다.
    $AutomationReport = $AutomationReportText | ConvertFrom-Json

    # 보고서에 기록된 전체 테스트 배열입니다.
    $ReportedTests = @($AutomationReport.tests)
    $ResultSummary.total_reported_test_count = $ReportedTests.Count
    $ResultSummary.total_failed_test_count = @($ReportedTests | Where-Object { $_.state -ne 'Success' }).Count

    # 필수 테스트별 발견 여부와 실제 상태를 보존할 요약 배열입니다.
    $RequiredTestResults = foreach ($RequiredTestPath in $RequiredTestPaths)
    {
        # 정확한 fullTestPath와 일치하는 단일 테스트 결과입니다.
        $MatchedTest = @($ReportedTests | Where-Object { $_.fullTestPath -eq $RequiredTestPath } | Select-Object -First 1)
        if ($MatchedTest.Count -eq 1)
        {
            [ordered]@{
                full_test_path = $RequiredTestPath
                found = $true
                state = [string]$MatchedTest[0].state
            }
        }
        else
        {
            [ordered]@{
                full_test_path = $RequiredTestPath
                found = $false
                state = 'Missing'
            }
        }
    }

    $ResultSummary.required_tests = @($RequiredTestResults)
    $ResultSummary.matched_required_test_count = @($RequiredTestResults | Where-Object { $_.found }).Count
    $ResultSummary.succeeded_required_test_count = @($RequiredTestResults | Where-Object { $_.state -eq 'Success' }).Count
    $ResultSummary.failed_required_test_count = @($RequiredTestResults | Where-Object { $_.found -and $_.state -ne 'Success' }).Count
    $ResultSummary.missing_required_test_count = @($RequiredTestResults | Where-Object { -not $_.found }).Count
    $ResultSummary.passed = (
        $EditorExitCode -eq 0 -and
        $ResultSummary.matched_required_test_count -eq $RequiredTestPaths.Count -and
        $ResultSummary.succeeded_required_test_count -eq $RequiredTestPaths.Count -and
        $ResultSummary.failed_required_test_count -eq 0 -and
        $ResultSummary.missing_required_test_count -eq 0)
}

# Admin과 다음 세션이 안정적으로 읽을 UTF-8 without BOM 결과 JSON입니다.
$ResultJsonText = $ResultSummary | ConvertTo-Json -Depth 8
[System.IO.File]::WriteAllText(
    $ResultJsonPath,
    $ResultJsonText,
    [System.Text.UTF8Encoding]::new($false))

Write-Host "[CarFight] Result JSON: $ResultJsonPath"
Write-Host "[CarFight] Editor exit code: $EditorExitCode"
Write-Host "[CarFight] Required tests: $($ResultSummary.succeeded_required_test_count)/$($RequiredTestPaths.Count) Success"

if (-not $ResultSummary.passed)
{
    Write-Host '[CarFight] Combat runtime automation regressions failed.'
    exit 1
}

Write-Host '[CarFight] Combat runtime automation regressions passed.'
exit 0
