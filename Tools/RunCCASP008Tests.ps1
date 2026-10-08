# CarFight CCAS-P0-08 focused Automation runner.
# Version: v1.0.0
# Date: 2026-10-07
# Description: CF-FQ-058 CCAS-P0-08 Production Publication Catalog runtime discovery/authorization + legacy fallback + cutover durable readback exact7 Automation을 실행합니다.
# Changelog:
# - v1.0.0: ProductionCatalogAuthorization, ProductionCatalogDiscovery, legacy CatalogOptionSync, RuntimeApply affected regression, ProductionCutoverVerification exact7 최초 추가.
# Migration:
# - canonical Workbook, Production DataAsset, Publication Catalog, Runtime Product를 수정하지 않습니다.
# - 기존 RunDataAuthoringTests.ps1 exact-list runner를 재사용하며 별도 Automation infrastructure를 만들지 않습니다.

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

$ExactTestNames = @(
    'CarFight.RuntimeApply.CF_FQ_058.ProductionCatalogAuthorization',
    'CarFight.RuntimeApply.CF_FQ_058.ProductionCatalogDiscovery',
    'CarFight.RuntimeApply.CF_FQ_044.VRCP_P0_03.CatalogOptionSync',
    'CarFight.RuntimeApply.RTA_P0_04.ExplicitEquipmentApply',
    'CarFight.RuntimeApply.RTA_P0_03.EquipmentValidation',
    'CarFight.RuntimeApply.RTA_P0_03.EquipmentApplySuccess',
    'CarFight.ContentAuthoring.P007.ProductionCutoverVerification'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
