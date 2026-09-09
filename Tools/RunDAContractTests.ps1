# CarFight Data Asset Contract Evolution Guard focused Automation runner.
# Version: v1.3.0
# Date: 2026-09-09
# Description: CF-FQ-050 DACE-P0-01 Foundation부터 P0-04 Migration Impact Guard까지 exact focused Automation 15종을 공용 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.3.0: DACE-P0-04 CanonicalStaging, ResolutionEvidence, PromotionAppend exact3을 추가해 총 15종을 실행합니다.
# - v1.2.0: DACE-P0-03 RevisionBumps, MigrationDeclaration, AcceptedSnapshotChain, CurrentBaseline exact4를 추가해 총 12종을 실행합니다.
# - v1.1.0: DACE-P0-02 SourceReflection, AdapterMapping, SerializerParser, Fingerprint, MaterializerExtractor exact 5종을 추가해 P0-01 exact3과 함께 총 8종을 실행합니다.
# - v1.0.0: ContractDescriptor, BootstrapSnapshot, ProductionProbe exact 3종 runner를 최초 추가.
# Migration:
# - 이 runner는 memory-only JSON/DTO와 transient UObject probe만 실행하며 SyncProduct, ApplyReviewed, SavePackage 또는 canonical Product Staging write를 호출하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# DACE-P0-01/P0-02/P0-03/P0-04가 소유하는 exact focused Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_01.ContractDescriptor',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_01.BootstrapSnapshot',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_01.ProductionProbe',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_02.SourceReflection',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_02.AdapterMapping',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_02.SerializerParser',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_02.Fingerprint',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_02.MaterializerExtractor',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_03.RevisionBumps',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_03.MigrationDeclaration',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_03.AcceptedSnapshotChain',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_03.CurrentBaseline',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_04.CanonicalStaging',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_04.ResolutionEvidence',
    'CarFight.DataManagement.CF_FQ_050.DACE_P0_04.PromotionAppend'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# 공용 exact-list runner의 terminal exit code입니다.
$AutomationExitCode = $LASTEXITCODE
exit $AutomationExitCode
