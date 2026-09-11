# CarFight CF-FQ-053 VDR-P0-02 focused Automation runner.
# Version: v1.1.0
# Date: 2026-09-11
# Description: VehicleDefense DACE exact3 + fourth-type mixed operational exact4 tests = Gate 3 focused exact7을 공용 exact-list runner로 검증합니다.
# Changelog:
# - v1.1.0: DACE pre-admission exact3에 OperationalAdmission/CrossTypeDurable/TargetDuplicate/SourceStale exact4를 추가하고 test-owned Content/Staging residue0 gate를 추가했습니다.
# - v1.0.0: DACE descriptor bootstrap, production probe, bootstrap migration exact3를 same-process exact-list로 실행했습니다.
# Migration:
# - v1.1.0은 Gate 3 final focused runner입니다. Product canonical VehicleDefense staging, protected DA_VehicleDefense_Test와 predecessor accepted histories를 mutation하지 않습니다.
# - Disposable mutation은 /Game/Test/CarFight/VDRDefenseP02와 VehicleDefense/Damage provider __Automation* roots에만 한정합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 terminal failure로 처리합니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# Tools 폴더의 부모인 main_game repository root입니다.
$MainGameRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

# VDR-P0-02 test-owned physical Content root입니다.
$FixtureContentRoot = Join-Path $MainGameRoot 'UE\Content\Test\CarFight\VDRDefenseP02'

# VDR-P0-02 VehicleDefense provider-owned test Staging root입니다.
$VehicleDefenseStagingRoot = Join-Path $MainGameRoot 'Authoring\DataAssetStaging\VehicleDefenseData\__AutomationP02__'

# VDR-P0-02 Damage provider-owned test Staging root입니다.
$DamageStagingRoot = Join-Path $MainGameRoot 'Authoring\DataAssetStaging\DamageData\__AutomationVDRP02__'

# VDR-P0-02 Gate 3 final focused exact7 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_02.VehicleDefenseDaceDescriptorBootstrap',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_02.VehicleDefenseDaceProductionProbe',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_02.VehicleDefenseDaceBootstrapMigration',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_02.OperationalAdmission',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_02.CrossTypeDurable',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_02.TargetDuplicate',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_02.SourceStale'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# Underlying exact-list Automation terminal exit code입니다.
$AutomationExitCode = $LASTEXITCODE

# Automation teardown 뒤 남으면 Gate 3 focused run 전체를 실패시킬 exact test-owned physical residue 목록입니다.
$FixtureResiduePaths = @()
if (Test-Path -LiteralPath $FixtureContentRoot) {
    $FixtureResiduePaths += $FixtureContentRoot
}
if (Test-Path -LiteralPath $VehicleDefenseStagingRoot) {
    $FixtureResiduePaths += $VehicleDefenseStagingRoot
}
if (Test-Path -LiteralPath $DamageStagingRoot) {
    $FixtureResiduePaths += $DamageStagingRoot
}

if ($FixtureResiduePaths.Count -gt 0) {
    Write-Error ("VDR-P0-02 fixture residue가 남았습니다: " + ($FixtureResiduePaths -join ', '))
    exit 1
}

exit $AutomationExitCode
