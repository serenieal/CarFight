# CarFight CF-FQ-053 VDR-P0-01 focused Automation runner.
# Version: v1.0.0
# Date: 2026-09-11
# Description: VehicleDefenseData Typed Provider + Durable Gate 2 exact6 Automation을 공용 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.0.0: provider/strict/reflection/current exact4 + durable/stale exact2를 same-process exact-list로 실행하고 test-owned physical residue0을 검증합니다.
# Migration:
# - Product canonical VehicleDefense staging, protected DA_VehicleDefense_Test, DACE history와 mixed operational admission은 mutation하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# Tools 폴더의 부모인 main_game repository root입니다.
$MainGameRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

# VDR-P0-01 test-owned physical Content root입니다.
$FixtureContentRoot = Join-Path $MainGameRoot 'UE\Content\Test\CarFight\VDRDefenseP01'

# VDR-P0-01 test-owned provider Staging root입니다.
$FixtureStagingRoot = Join-Path $MainGameRoot 'Authoring\DataAssetStaging\VehicleDefenseData\__AutomationP01__'

# VDR-P0-01 Gate 2 exact focused Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_01.ProviderContract',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_01.StrictContract',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_01.ReflectionParity',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_01.ProtectedCurrentReadOnly',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_01.DurableRoundTrip',
    'CarFight.DataManagement.CF_FQ_053.VDR_P0_01.StaleGuards'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# Underlying exact-list Automation terminal exit code입니다.
$AutomationExitCode = $LASTEXITCODE

# Automation teardown 뒤 남으면 focused run 전체를 실패시킬 exact test-owned physical residue 목록입니다.
$FixtureResiduePaths = @()
if (Test-Path -LiteralPath $FixtureContentRoot) {
    $FixtureResiduePaths += $FixtureContentRoot
}
if (Test-Path -LiteralPath $FixtureStagingRoot) {
    $FixtureResiduePaths += $FixtureStagingRoot
}

if ($FixtureResiduePaths.Count -gt 0) {
    Write-Error ("VDR-P0-01 fixture residue가 남았습니다: " + ($FixtureResiduePaths -join ', '))
    exit 1
}

exit $AutomationExitCode
