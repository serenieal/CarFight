# CarFight CCAS-P0-07 focused Automation runner.
# Version: v1.9.0
# Date: 2026-10-06
# Description: CF-FQ-058 CCAS-P0-07 cutover + Production Bridge + exact34 candidate preparation exact25 Automation을 실행합니다.
# Changelog:
# - v1.9.0: First Production Wave exact8/exact48/exact34 candidate preparation + current finite-ammo baseline 회귀를 추가해 focused exact25로 확장.
# - v1.8.0: concrete Production target ↔ existing Weapon/Equipment backend binding 회귀를 추가해 focused exact24로 확장.
# - v1.7.0: F.1 shared single-writer + F.2 multi-Product closure 선철회/재검증 회귀를 추가해 focused exact23으로 확장.
# - v1.6.0: Production transaction v2 durable file-store roundtrip 회귀를 추가해 focused exact22로 확장.
# - v1.5.0: Production Bridge focused exact5 — AmmoSchemaProjection, ImpactGraph, RecoveryResume, AmmoDefault, Runtime explicit ammo apply regression 추가.
# - v1.4.0: Mid-review P1 correction 회귀 exact6 — ManifestTamperBlocked, ExecutionPathBound, PromotionFreshnessGuard, RecoveryFinalize, JournalFailureRecovery, JournalHardFailureVisible 추가.
# - v1.3.0: DurableFileStore production transaction/provenance JSON save→load roundtrip test 추가.
# - v1.2.0: ProviderCutoverBridge existing ICFContentProvider reviewed seam 재사용/absent Product fail-closed test 추가.
# - v1.1.0: PersistentXlsxRoundtrip concrete OpenXLSX staged-write/reopen test 추가.
# - v1.0.0: ManagedCutover/StaleApproval/DirectProductDrift/PartialRecovery/ConditionalBlocked/RetireNoDelete/Provenance exact7 최초 추가.
# Migration:
# - production Workbook authority activation, Product asset delete/rename/move, Current Systems promotion을 수행하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리합니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# CCAS-P0-07 focused exact test 목록입니다.
$ExactTestNames = @(
    'CarFight.CCAS.CF_FQ_058.P0_07.ManagedCutover',
    'CarFight.CCAS.CF_FQ_058.P0_07.StaleApproval',
    'CarFight.CCAS.CF_FQ_058.P0_07.DirectProductDrift',
    'CarFight.CCAS.CF_FQ_058.P0_07.PartialRecovery',
    'CarFight.CCAS.CF_FQ_058.P0_07.ConditionalBlocked',
    'CarFight.CCAS.CF_FQ_058.P0_07.RetireNoDelete',
    'CarFight.CCAS.CF_FQ_058.P0_07.Provenance',
    'CarFight.CCAS.CF_FQ_058.P0_07.ManifestTamperBlocked',
    'CarFight.CCAS.CF_FQ_058.P0_07.ExecutionPathBound',
    'CarFight.CCAS.CF_FQ_058.P0_07.PromotionFreshnessGuard',
    'CarFight.CCAS.CF_FQ_058.P0_07.RecoveryFinalize',
    'CarFight.CCAS.CF_FQ_058.P0_07.JournalFailureRecovery',
    'CarFight.CCAS.CF_FQ_058.P0_07.JournalHardFailureVisible',
    'CarFight.CCAS.CF_FQ_058.P0_07.PersistentXlsxRoundtrip',
    'CarFight.CCAS.CF_FQ_058.P0_07.ProviderCutoverBridge',
    'CarFight.CCAS.CF_FQ_058.P0_07.DurableFileStore',
    'CarFight.ContentAuthoring.P007.ProductionBridge.AmmoSchemaProjection',
    'CarFight.ContentAuthoring.P007.ProductionBridge.ImpactGraph',
    'CarFight.ContentAuthoring.P007.ProductionBridge.RecoveryResume',
    'CarFight.ContentAuthoring.P007.ProductionBridge.AmmoDefault',
    'CarFight.ContentAuthoring.P007.ProductionBridge.SharedClosure',
    'CarFight.ContentAuthoring.P007.ProductionBridge.ConcreteBindings',
    'CarFight.ContentAuthoring.P007.ProductionBridge.TransactionFileStore',
    'CarFight.ContentAuthoring.P007.ProductionBatch.CandidatePreparation',
    'CarFight.RuntimeApply.RTA_P0_03.EquipmentApplySuccess'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
