# CarFight CCAS-P0-05 focused Automation runner.
# Version: v1.0.0
# Date: 2026-09-29
# Description: CF-FQ-058 CCAS-P0-05 Catalog / Compare + USER Workflow exact5 Automation을 기존 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.0.0: LiveDualConsumerCatalog/FilterFamilyVariant/SelectedCompareSources/CanonicalWorkbookPreview/ReadOnlyAuthorityGuard exact5 최초 추가.
# Migration:
# - Product Apply/Save, Workbook persistent write, P0-07 authority cutover를 수행하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리합니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# CCAS-P0-05이 요구하는 exact read-only Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.CCAS.CF_FQ_058.P0_05.LiveDualConsumerCatalog',
    'CarFight.CCAS.CF_FQ_058.P0_05.FilterFamilyVariant',
    'CarFight.CCAS.CF_FQ_058.P0_05.SelectedCompareSources',
    'CarFight.CCAS.CF_FQ_058.P0_05.CanonicalWorkbookPreview',
    'CarFight.CCAS.CF_FQ_058.P0_05.ReadOnlyAuthorityGuard'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
