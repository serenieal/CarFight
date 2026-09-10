# CarFight CF-FQ-051 DAO-P0-05 mixed operational focused Automation runner.
# Version: v1.0.0
# Date: 2026-09-10
# Description: actual registered MissileGuidePreset + AmmoData provider의 path-owner/mixed durable/duplicate/stale exact4를 실행하고 disposable physical residue0을 검증합니다.
# Changelog:
# - v1.0.0: DAO-P0-05 actual-provider mixed integration exact4와 /Game/Test/CarFight/DAOP05 + provider-local __AutomationP05__ residue gate를 최초 추가.
# Migration:
# - Product Low/Normal/High, Product canonical Ammo exact0, HeavyFinite/RocketFinite는 mutation하지 않습니다.
# - durable fixture는 /Game/Test/CarFight/DAOP05 및 MissileGuidePreset/AmmoData __AutomationP05__ roots에만 한정합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# Tools 폴더의 부모인 main_game repository root입니다.
$MainGameRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

# DAO-P0-05 test-owned physical Content root입니다.
$FixtureContentRoot = Join-Path $MainGameRoot 'UE\Content\Test\CarFight\DAOP05'

# DAO-P0-05 test-owned Missile provider Staging root입니다.
$MissileFixtureStagingRoot = Join-Path $MainGameRoot 'Authoring\DataAssetStaging\MissileGuidePreset\__AutomationP05__'

# DAO-P0-05 test-owned Ammo provider Staging root입니다.
$AmmoFixtureStagingRoot = Join-Path $MainGameRoot 'Authoring\DataAssetStaging\AmmoData\__AutomationP05__'

# DAO-P0-05 actual-provider focused exact4 Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.PathOwnerContract',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDurable',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDuplicate',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedStale'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# underlying exact-list Automation terminal exit code입니다.
$AutomationExitCode = $LASTEXITCODE

# Automation teardown 뒤 남으면 focused run 전체를 실패시킬 exact test-owned physical residue 목록입니다.
$FixtureResiduePaths = @()
if (Test-Path -LiteralPath $FixtureContentRoot) {
    $FixtureResiduePaths += $FixtureContentRoot
}
if (Test-Path -LiteralPath $MissileFixtureStagingRoot) {
    $FixtureResiduePaths += $MissileFixtureStagingRoot
}
if (Test-Path -LiteralPath $AmmoFixtureStagingRoot) {
    $FixtureResiduePaths += $AmmoFixtureStagingRoot
}

if ($FixtureResiduePaths.Count -gt 0) {
    Write-Error ("DAO-P0-05 fixture residue가 남았습니다: " + ($FixtureResiduePaths -join ', '))
    exit 1
}

exit $AutomationExitCode
