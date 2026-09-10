# CarFight CF-FQ-051 DAO Ammo typed + durable + DACE focused Automation runner.
# Version: v1.4.0
# Date: 2026-09-10
# Description: DAO-P0-02 typed exact4 + DAO-P0-03 disposable durable exact3 + DAO-P0-04 correction/implementation exact6 + DAO-P0-05 mixed operational exact4를 공용 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.4.0: DAO-P0-05 provider path-owner/mixed durable/duplicate/stale actual-provider exact4를 추가해 총 exact17로 확장.
# - v1.3.0: Ammo DACE descriptor/bootstrap, production probe, negative probe, migration exact4를 추가해 총 exact13으로 확장.
# - v1.2.0: DAO-P0-04 DaceTypeBoundaryCorrection/DaceObservationCorrection exact2를 추가해 총 exact9로 확장.
# - v1.1.0: DAO-P0-03 DurableRoundTrip/StaleDirtyGuards/SaveUncertainty exact3을 추가해 총 exact7로 확장.
# - v1.0.0: DAO-P0-02 exact4 focused test entry를 최초 추가.
# Migration:
# - P0-03 durable tests의 Save/Delete는 /Game/Test/CarFight/DAOAmmoP03 + AmmoData/__AutomationP03__ test-owned root에만 한정합니다.
# - Product canonical Ammo exact0, persisted HeavyFinite/RocketFinite exact2, Missile Product/accepted baseline은 mutation 대상으로 사용하지 않습니다.
# - P0-04 implementation exact4는 memory/transient-only probe이며 Staging/Product 파일 Save를 수행하지 않습니다.
# - P0-05 exact4의 durable Save/Delete는 /Game/Test/CarFight/DAOP05 + MissileGuidePreset/AmmoData __AutomationP05__ test-owned roots에만 한정합니다. Product/HeavyFinite/RocketFinite는 사용하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# DAO-P0-02 typed + DAO-P0-03 durable + DAO-P0-04 DACE + DAO-P0-05 mixed operational exact focused Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_02.ParseFingerprint',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_02.NegativeContract',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_02.PreviewProvider',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_02.CurrentReadOnly',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_03.DurableRoundTrip',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_03.StaleDirtyGuards',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_03.SaveUncertainty',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_04.DaceTypeBoundaryCorrection',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_04.DaceObservationCorrection',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceDescriptorBootstrap',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceProductionProbe',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceNegativeProbe',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceBootstrapMigration',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.PathOwnerContract',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDurable',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDuplicate',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedStale'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# 공용 exact-list runner의 terminal exit code입니다.
$AutomationExitCode = $LASTEXITCODE
exit $AutomationExitCode
