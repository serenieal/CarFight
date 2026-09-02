# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunBuilderTransTests.ps1
# Version: v1.1.0
# Date: 2026-08-31
# Description: CF-FQ-040 Guided Vehicle Builder의 차종 비종속 Transmission authoring contract focused Automation runner입니다.
# Scope: Builder-wide Transmission policy/provenance와 Existing Evidence Refresh, Step 1/5/7/8 affected flow를 exact filter로 실행합니다. Wagon Asset/PIE/Save를 요구하지 않습니다.
# Changelog:
# - v1.1.0: Existing Evidence Refresh service/VM + Step1 Reference exact regression을 추가해 later research enrichment가 Transmission proposal 이전에 안전하게 동작하는지 검증.
# - v1.0.0: Legacy/VehicleSpecific policy, FACT/DERIVED/GAME_BIAS contract, Guided creation, Step 5 Physics, Step 7 Final Review, Step 8 preserved flow exact tests를 최초 고정.
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

foreach ($TestFilter in $Tests) {
    Write-Output "BUILDER_TRANS_TEST_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Builder Transmission focused Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "BUILDER_TRANS_TEST_PASS=$TestFilter"
}

Write-Output 'BUILDER_TRANSMISSION_FOCUSED_AUTOMATION=PASS'
exit 0
