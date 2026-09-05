# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunBuilderTransTests.ps1
# Version: v1.3.0
# Date: 2026-09-05
# Description: CF-FQ-040 Guided Vehicle Builder의 차종 비종속 Transmission authoring contract focused Automation runner입니다.
# Scope: Builder-wide Transmission policy/provenance와 Existing Evidence Refresh, Step 1/5/7/8 affected flow exact9을 한 UnrealEditor process에서 실행합니다. Wagon Asset/PIE/Save를 요구하지 않습니다.
# Changelog:
# - v1.3.0: UFP-02 Minimum Operational Adoption으로 UFP-01에서 검증한 same-process exact-list topology를 canonical runner에 채택. exact9 identity와 log suppression OFF는 유지하고 UnrealEditor launch를 9회에서 1회로 축소.
# - v1.2.0: UFP-01 ROI authority를 위해 exact9 전체 workflow에 Consumer-local System.Diagnostics.Stopwatch 기반 high-resolution monotonic aggregate wall marker를 추가. 테스트 목록/순서/Editor 실행 옵션은 변경하지 않음.
# - v1.1.0: Existing Evidence Refresh service/VM + Step1 Reference exact regression을 추가해 later research enrichment가 Transmission proposal 이전에 안전하게 동작하는지 검증.
# - v1.0.0: Legacy/VehicleSpecific policy, FACT/DERIVED/GAME_BIAS contract, Guided creation, Step 5 Physics, Step 7 Final Review, Step 8 preserved flow exact tests를 최초 고정.
# Migration:
# - v1.3.0부터 canonical Builder Transmission runner는 RunDataAuthoringTests.ps1 -ExactTestNames를 사용해 exact9을 one-process batch로 실행합니다. Process-per-test topology는 UFP-01 baseline evidence로만 보존합니다.
# - WORKFLOW_EXECUTION_WALL_MS는 이 Consumer runner 내부 monotonic operational timing이며 ChatGPT/MCP/process.status/UI wall과 합성하지 않습니다.
# - Product Asset Save, PIE, broad CarFight.DataAuthoring 전체 replay를 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Exact Data Authoring Automation 실행기를 재사용합니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "RunDataAuthoringTests.ps1을 찾을 수 없습니다: $Runner"
}

# Builder-wide Transmission contract에 직접 영향을 받는 exact Automation 목록입니다.
$Tests = @(
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderEvidenceRefresh',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderTransmissionContract',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderProfileCommit',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep1Reference',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderEvidenceRefreshVM',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep5Physics',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep7FinalReview',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep8Driving'
)

# exact9 전체 Consumer workflow의 high-resolution monotonic wall을 측정하는 Stopwatch입니다.
$WorkflowStopwatch = [System.Diagnostics.Stopwatch]::StartNew()

# 현재 .NET Stopwatch가 high-resolution performance counter를 사용하는지 표시합니다.
$WorkflowTimerHighResolution = [System.Diagnostics.Stopwatch]::IsHighResolution

# 현재 Stopwatch performance counter frequency입니다.
$WorkflowTimerFrequency = [System.Diagnostics.Stopwatch]::Frequency

Write-Output ("WORKFLOW_TIMER_HIGH_RESOLUTION={0}" -f $WorkflowTimerHighResolution)
Write-Output ("WORKFLOW_TIMER_FREQUENCY={0}" -f $WorkflowTimerFrequency)
Write-Output 'BUILDER_TRANS_EXECUTION_TOPOLOGY=same_process_exact_list'
Write-Output 'BUILDER_TRANS_EDITOR_LAUNCH_COUNT=1'

# 기존 로그 소비자가 exact requested identity를 계속 확인할 수 있도록 frozen 요청 목록을 그대로 출력합니다.
$Tests | ForEach-Object { Write-Output "BUILDER_TRANS_TEST_BEGIN=$_" }

# UFP-01에서 실제 equivalence/contamination/ROI를 통과한 exact-list one-process 경로를 canonical 실행으로 사용합니다.
& $Runner -ExactTestNames $Tests
# child exact-list runner의 terminal exit code입니다.
$RunnerExitCode = $LASTEXITCODE

if ($RunnerExitCode -ne 0) {
    # 실패한 exact-list workflow 전체의 Consumer wall을 terminal evidence로 고정합니다.
    $WorkflowStopwatch.Stop()
    Write-Output ("WORKFLOW_EXECUTION_WALL_MS={0:F3}" -f $WorkflowStopwatch.Elapsed.TotalMilliseconds)
    Write-Error 'Builder Transmission same-process focused Automation failed.'
    exit $RunnerExitCode
}

# Child가 requested=executed exact equality까지 PASS한 뒤 compatibility marker를 requested exact9 각각에 투영합니다.
$Tests | ForEach-Object { Write-Output "BUILDER_TRANS_TEST_PASS=$_" }

# exact9가 모두 끝난 시점의 Consumer-local monotonic aggregate wall입니다.
$WorkflowStopwatch.Stop()

Write-Output ("WORKFLOW_EXECUTION_WALL_MS={0:F3}" -f $WorkflowStopwatch.Elapsed.TotalMilliseconds)
Write-Output 'BUILDER_TRANSMISSION_FOCUSED_AUTOMATION=PASS'
exit 0
