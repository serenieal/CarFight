# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonTransDraftCheck.ps1
# Version: v1.0.0
# Date: 2026-08-31
# Description: CF-FQ-040 actual Wagon Saved ResearchDraft의 mutation0 Evidence Refresh dry-run 전용 Automation wrapper입니다.
# Scope: actual Wagon persisted Recipe/Evidence와 Saved ResearchDraft를 읽어 prospective EvidenceFingerprint를 검증합니다. Product Asset mutation/Apply/Save를 수행하지 않습니다.
# Changelog:
# - v1.0.0: WagonTransmissionDraft exact Automation 1건만 기존 RunDataAuthoringTests.ps1로 실행하고 terminal PASS를 반환합니다.
# Migration:
# - Generic Builder Transmission focused runner와 분리합니다.
# - actual Wagon regression fixture와 local Saved ResearchDraft가 존재하는 current 작업용 검증 경로입니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Existing exact Data Authoring Automation runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "RunDataAuthoringTests.ps1을 찾을 수 없습니다: $Runner"
}

# Actual Wagon ResearchDraft mutation0 dry-run exact test filter입니다.
$TestFilter = 'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.WagonTransmissionDraft'

Write-Output "WAGON_TRANS_DRAFT_TEST_BEGIN=$TestFilter"
& $Runner -TestFilter $TestFilter
if ($LASTEXITCODE -ne 0) {
    Write-Error "Wagon Transmission Draft dry-run failed: $TestFilter"
    exit $LASTEXITCODE
}

Write-Output "WAGON_TRANS_DRAFT_TEST_PASS=$TestFilter"
Write-Output 'WAGON_TRANSMISSION_DRAFT_DRYRUN=PASS'
exit 0
