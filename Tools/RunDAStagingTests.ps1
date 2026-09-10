# CarFight Data Asset Staging focused Automation runner.
# Version: v1.5.0
# Date: 2026-09-09
# Description: CF-FQ-049 DAS-P0-02 foundation + P0-03 Apply safety + DAS-P0-04 Product Preview/durable fixture focused Automation exact set을 기존 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.5.0: DAO-P0-01 provider-owned StagingRoot 계약에 맞춰 P04 runner residue gate를 MissileGuidePreset provider root 하위로 이동. exact 13종 목록은 변경하지 않음.
# - v1.4.0: P0-04 post-PASS correction에 맞춰 AdapterContractRevision 2 회귀와 loaded-but-unregistered residue 검증을 포함하는 current exact 13종을 유지하고, runner의 physical Content/Staging residue gate를 보존.
# - v1.3.0: DAS-P0-04 exact-list 실행 전 test-owned residue pre-clean과 실행 후 Content/Staging residue 0 verification gate를 추가해 cleanup defect가 전체 focused run을 silent PASS시키지 못하게 강화.
# - v1.2.0: DAS-P0-04 Product mutation0 Preview, durable Create/Update, conflict guards, save uncertainty, PartialApplied 5건을 추가해 exact 13종으로 확장.
# - v1.1.0: DAS-P0-03 one-shot ApprovalLifecycle과 loaded-but-unregistered StableIdentity preflight 2건을 추가해 exact 8종으로 확장.
# - v1.0.0: CF-FQ-045 Registry 28종 coverage 2건과 CF-FQ-049 Staging parse/canonical/Preview/BatchPlanHash 4건을 same-process exact-list로 고정.
# Migration:
# - Product Low/Normal/High는 mutation0 Preview만 수행하며 저장하지 않습니다. 실제 Save/Delete는 DAS-P0-04 test-owned /Game/Test/CarFight/DAStagingP04 fixture에만 한정합니다.
# - 반드시 현재 Source가 공식 BuildEditor.bat로 성공한 뒤 실행해야 하며 stale Editor binary의 결과를 current evidence로 사용하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# DAS-P0-04 exact test-owned residue cleanup runner 경로입니다.
$FixtureCleanupRunner = Join-Path $PSScriptRoot 'ClearDAStagingP04.ps1'

# Tools 폴더의 부모인 main_game repository root입니다.
$MainGameRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

# DAS-P0-04 test-owned physical Content root입니다.
$FixtureContentRoot = Join-Path $MainGameRoot 'UE\Content\Test\CarFight\DAStagingP04'

# DAS-P0-04 test-owned canonical Staging root입니다.
$FixtureStagingRoot = Join-Path $MainGameRoot 'Authoring\DataAssetStaging\MissileGuidePreset\__AutomationP04__'

# DAS-P0-02 foundation + DAS-P0-03 approval/preflight + DAS-P0-04 Pilot fixture가 요구하는 exact Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_045.DAM_P0_02B.CurrentDescriptorMatrix',
    'CarFight.DataManagement.CF_FQ_045.DAM_P0_02B.CurrentCoverageIntegration',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_02.ParseCanonical',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_02.StrictValidation',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_02.PreviewMatrix',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_02.BatchPlanHash',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_03.ApprovalLifecycle',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_03.LoadedIdentityPreflight',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_04.ProductPreview',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_04.DurableFixture',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_04.ConflictGuards',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_04.SaveUncertainty',
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_04.PartialApplied'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

if (-not (Test-Path -LiteralPath $FixtureCleanupRunner -PathType Leaf)) {
    throw "DAS-P0-04 fixture cleanup runner를 찾을 수 없습니다: $FixtureCleanupRunner"
}

# 이전 실패 run의 test-owned residue를 exact root 범위에서만 제거합니다.
& $FixtureCleanupRunner
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# underlying exact-list Automation exit code입니다.
$AutomationExitCode = $LASTEXITCODE

# Automation teardown 뒤 test-owned Content/Staging root가 다시 생겼다면 focused run 전체를 실패시킵니다.
$FixtureResiduePaths = @()
if (Test-Path -LiteralPath $FixtureContentRoot) {
    $FixtureResiduePaths += $FixtureContentRoot
}
if (Test-Path -LiteralPath $FixtureStagingRoot) {
    $FixtureResiduePaths += $FixtureStagingRoot
}

if ($FixtureResiduePaths.Count -gt 0) {
    Write-Error ("DAS-P0-04 fixture residue가 남았습니다: " + ($FixtureResiduePaths -join ', '))
    exit 1
}

exit $AutomationExitCode
