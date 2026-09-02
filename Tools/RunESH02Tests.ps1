# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunESH02Tests.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 ESH-02 Engine Curve proposal/provenance affected focused Automation runner입니다.
# Changelog:
# - v1.0.0: generic Engine Curve contract, Profile receipt, Final Review exact3을 기존 targeted runner로 순차 검증.
# Migration:
# - Product Asset/Config mutation과 Save를 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
$TestFilters = @(
    'CarFight.DataAuthoring.CF_FQ_040.ESH_02.BuilderEngineCurveContract',
    'CarFight.DataAuthoring.CF_FQ_040.ESH_02.WagonEngineCurveDraft',
    'CarFight.DataAuthoring.CF_FQ_040.ESH_02.WagonEngineCurvePersisted',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderProfileCommit',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_07.FinalReviewApplyUndo'
)

foreach ($TestFilter in $TestFilters) {
    Write-Output "ESH02_TEST_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "ESH-02 focused Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "ESH02_TEST_PASS=$TestFilter"
}

Write-Output "ESH02_EXACT_PASS_COUNT=$($TestFilters.Count)"
exit 0
