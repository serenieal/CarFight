# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunCatalogPromoAffectedTests.ps1
# Version: v1.0.0
# Date: 2026-09-03
# Description: CF-FQ-044 VRCP-P0-04의 남은 Builder creation / Mount Guidance affected regression만 bounded 실행합니다.
# Scope: 이미 PASS한 Promotion Service, RuntimeApply, CF-FQ-040 Step8 회귀를 반복하지 않고 CF-FQ-042/043 prefix만 실행합니다.
# Migration:
# - Product Asset Save, PIE, broad CarFight.DataAuthoring 전체 replay를 수행하지 않습니다.
# - Experimental MCP log는 focused Automation evidence에 영향이 없도록 실행 중 category만 suppression합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 기존 Data Authoring targeted Automation runner를 재사용합니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "RunDataAuthoringTests.ps1을 찾을 수 없습니다: $Runner"
}

# CF-FQ-044가 실제로 영향을 받을 수 있는 완료 Feature prefix만 실행합니다.
$AffectedFilters = @(
    'CarFight.DataAuthoring.CF_FQ_042',
    'CarFight.DataAuthoring.CF_FQ_043'
)

foreach ($TestFilter in $AffectedFilters) {
    Write-Output "CATALOG_PROMO_AFFECTED_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Catalog Promotion affected regression failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "CATALOG_PROMO_AFFECTED_PASS=$TestFilter"
}

Write-Output 'CATALOG_PROMO_AFFECTED_AUTOMATION=PASS'
exit 0
