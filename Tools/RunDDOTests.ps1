# CarFight CF-FQ-052 DDO Damage onboarding focused Automation runner.
# Version: v1.3.0
# Date: 2026-09-11
# Description: DDO-P0-01 exact4 + DDO-P0-02 exact2 + DDO-P0-03 DACE exact4 + DDO-P0-04 three-type mixed exact4를 합친 focused Automation exact14를 실행합니다.
# Changelog:
# - v1.3.0: OperationalAdmission/ThreeTypeDurable/ThreeTypeDuplicate/ThreeTypeStale exact4를 추가해 Damage operational admission exact3와 actual-provider mixed lifecycle/duplicate/TOCTOU를 검증합니다.
# - v1.2.0: DamageDaceDescriptorBootstrap/ProductionProbe/NegativeProbe/BootstrapMigration exact4를 추가해 Damage ContractReady DACE와 exact16 production behavior를 검증합니다.
# - v1.1.0: DurableRoundTrip, StaleGuards exact2를 추가해 Damage ReviewedMutationReady/durable/TOCTOU 경계를 검증합니다.
# - v1.0.0: ParseFingerprint, NegativeContract, PreviewProvider, ProtectedAssetsReadOnly exact4 focused entry를 추가했습니다.
# Migration:
# - 공용 RunDataAuthoringTests.ps1 exact-list mode를 재사용합니다.
# - persisted DA_DamageAsset/DA_DamageArmorPenTest는 read-only 검증만 수행합니다. P0-02 Save/Delete는 /Game/Test/CarFight/DDODamageP02 및 DamageData/__AutomationP02__ disposable root에만 한정합니다.
# - DDO-P0-03 DACE exact4는 memory/transient-only probe입니다. Product canonical Damage Staging exact0과 Missile/Ammo/Damage accepted history는 mutation하지 않습니다.
# - DDO-P0-04 exact4의 Save/Delete는 /Game/Test/CarFight/DDODamageP04와 MissileGuidePreset/AmmoData/DamageData __AutomationP04__ disposable roots에만 한정하며 production mixed admission exact3를 검증합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# DDO-P0-01 exact4 + DDO-P0-02 durable/TOCTOU exact2 + DDO-P0-03 DACE exact4 + DDO-P0-04 mixed exact4 focused Automation exact14 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_01.ParseFingerprint',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_01.NegativeContract',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_01.PreviewProvider',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_01.ProtectedAssetsReadOnly',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_02.DurableRoundTrip',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_02.StaleGuards',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceDescriptorBootstrap',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceProductionProbe',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceNegativeProbe',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceBootstrapMigration',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_04.OperationalAdmission',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_04.ThreeTypeDurable',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_04.ThreeTypeDuplicate',
    'CarFight.DataManagement.CF_FQ_052.DDO_P0_04.ThreeTypeStale'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# 공용 exact-list runner의 terminal exit code입니다.
$AutomationExitCode = $LASTEXITCODE
exit $AutomationExitCode
