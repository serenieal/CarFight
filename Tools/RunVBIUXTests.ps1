# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunVBIUXTests.ps1
# Version: v1.0.0
# Date: 2026-09-03
# Description: CF-FQ-046 VBIUX-P0-05 사용자 표시 focused Automation runner입니다.
# Scope: Pure presentation state/diff/driving fixture와 기존 Step 1/5/7/8 affected Builder regression만 실행합니다.
# Changelog:
# - v1.0.0: PresentationStates, FinalReviewDiff, DrivingSummary + existing affected Builder Step regression exact filter를 최초 고정.
# Migration:
# - 기존 RunDataAuthoringTests.ps1을 재사용합니다.
# - Product Asset Save, PIE, broad CarFight.DataAuthoring 전체 replay를 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Exact Data Authoring Automation 실행기를 재사용합니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "RunDataAuthoringTests.ps1을 찾을 수 없습니다: $Runner"
}

# VBIUX presentation + 실제 touched Builder contract에 해당하는 exact Automation 목록입니다.
$Tests = @(
    'CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.PresentationStates',
    'CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.FinalReviewDiff',
    'CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.DrivingSummary',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep1Reference',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep5Physics',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep7FinalReview',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep8Driving'
)

foreach ($TestFilter in $Tests) {
    Write-Output "VBIUX_TEST_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter
    if ($LASTEXITCODE -ne 0) {
        Write-Error "VBIUX focused Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "VBIUX_TEST_PASS=$TestFilter"
}

Write-Output 'VBIUX_FOCUSED_AUTOMATION=PASS'
exit 0
