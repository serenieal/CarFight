# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunHardpointIntegrityAffectedTests.ps1
# Version: v1.0.0
# Date: 2026-09-03
# Description: CF-FQ-047 VBHAI-P0-05 affected regression runner입니다.
# Scope: 실제 touched Hardpoint/Mount/FinalReview/Driving/Presentation contract만 exact filter로 순차 재검증합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "RunDataAuthoringTests.ps1을 찾을 수 없습니다: $Runner"
}

$Tests = @(
    'CarFight.DataAuthoring.CF_FQ_043.VMG_P0_02.RecipeStateTypedRemove',
    'CarFight.DataAuthoring.CF_FQ_043.VMG_P0_03.Step3HardpointPlanning',
    'CarFight.DataAuthoring.CF_FQ_043.VMG_P0_04.Step6MountPlanning',
    'CarFight.DataAuthoring.CF_FQ_043.VMG_P0_05.ExistingPreservation',
    'CarFight.DataAuthoring.CF_FQ_043.VMG_P0_06.ContractMatrix',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep7FinalReview',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep8Driving',
    'CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.PresentationStates',
    'CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.FinalReviewDiff',
    'CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.DrivingSummary'
)

foreach ($TestFilter in $Tests) {
    Write-Output "VBHAI_AFFECTED_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter
    if ($LASTEXITCODE -ne 0) {
        Write-Error "VBHAI affected Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "VBHAI_AFFECTED_PASS=$TestFilter"
}

Write-Output 'VBHAI_AFFECTED_AUTOMATION=PASS'
exit 0
