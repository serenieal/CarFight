# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunHardpointIntegrityTests.ps1
# Version: v1.4.0
# Date: 2026-09-05
# Description: CF-FQ-047 Hardpoint integrity + P0-07A Physics + P0-07H durable final commit focused Automation runner입니다.
# Scope: Socket/semantic/post-commit warning/legacy receipt/Physics compatibility/Resolver/PostLoad와 isolated /Game durable transaction fixture를 실행하며 Product Asset Save/PIE를 수행하지 않습니다.
# Changelog:
# - v1.4.0: P0-07H DurableFinalCommit filter를 추가해 ApplyAndPersist, dirty pair persistence, partial save/retry, SaveStateUnconfirmed, AppliedState finalize와 TOCTOU를 production Prepare/Execute writer로 직접 회귀검증합니다.
# - v1.3.0: P0-07A PhysicsReceiptCompatibility focused filter를 추가합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "RunDataAuthoringTests.ps1을 찾을 수 없습니다: $Runner"
}

$Tests = @(
    'CarFight.DataAuthoring.CF_FQ_047.VBHAI_P0_04.InventoryClassification',
    'CarFight.DataAuthoring.CF_FQ_047.VBHAI_P0_04.SemanticReadback',
    'CarFight.DataAuthoring.CF_FQ_047.VBHAI_P0_04.PresentationReadback',
    'CarFight.DataAuthoring.CF_FQ_047.P0_06.Resolver.MountLegacyFallback',
    'CarFight.DataAuthoring.CF_FQ_047.P0_06.PhysicsReceiptRefreshBoundary',
    'CarFight.DataAuthoring.CF_FQ_047.P0_07A.PhysicsReceiptCompatibility',
    'CarFight.DataAuthoring.CF_FQ_047.P0_07H.DurableFinalCommit',
    'CarFight.DataAuthoring.CF_FQ_047.MidReview.PostCommitRefreshOutcome',
    'CarFight.DataAuthoring.CF_FQ_047.P0_06.ActualWagon.PostLoadMountIntegrity'
)

foreach ($TestFilter in $Tests) {
    Write-Output "VBHAI_TEST_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter
    if ($LASTEXITCODE -ne 0) {
        Write-Error "VBHAI focused Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "VBHAI_TEST_PASS=$TestFilter"
}

Write-Output 'VBHAI_FOCUSED_AUTOMATION=PASS'
exit 0
