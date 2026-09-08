# CarFight DAS-P0-04 test-owned residue cleanup.
# Version: v1.1.0
# Date: 2026-09-08
# Description: DAS-P0-04 Automation이 소유하는 exact Content/Staging fixture root만 재귀 삭제하고 residue 0을 검증합니다.
# Changelog:
# - v1.1.0: 이 runner가 physical pre-clean만 소유함을 명확히 하고, resolver-visible AssetRegistry/loaded UObject residue 검증은 in-process Automation teardown이 소유하도록 migration 경계를 기록.
# - v1.0.0: /Game/Test/CarFight/DAStagingP04 physical Content root와 Authoring/DataAssetStaging/__AutomationP04__ root의 bounded cleanup/verification을 추가.
# Migration:
# - Product Low/Normal/High 또는 다른 /Game/Test/CarFight 경로는 삭제하지 않습니다.
# - 이 PowerShell은 Editor process 밖의 physical residue pre-clean만 담당합니다. AssetRegistry/loaded UObject residue 0은 CFDAStagingPilotTests의 in-process CleanupFixtureRoot가 별도로 검증합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# Tools 폴더의 부모인 main_game repository root입니다.
$MainGameRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

# DAS-P0-04 test-owned physical Content root입니다.
$FixtureContentRoot = Join-Path $MainGameRoot 'UE\Content\Test\CarFight\DAStagingP04'

# DAS-P0-04 test-owned canonical Staging root입니다.
$FixtureStagingRoot = Join-Path $MainGameRoot 'Authoring\DataAssetStaging\__AutomationP04__'

# exact test-owned root 하나를 재귀 삭제하고 residue 0을 검증합니다.
function Clear-ExactFixtureRoot {
    param(
        [Parameter(Mandatory = $true)]
        [string]$LiteralPath,
        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    if (Test-Path -LiteralPath $LiteralPath) {
        Remove-Item -LiteralPath $LiteralPath -Recurse -Force
    }

    if (Test-Path -LiteralPath $LiteralPath) {
        throw "$Label cleanup 뒤 residue가 남았습니다: $LiteralPath"
    }
}

Clear-ExactFixtureRoot -LiteralPath $FixtureContentRoot -Label 'DAS-P0-04 Content fixture root'
Clear-ExactFixtureRoot -LiteralPath $FixtureStagingRoot -Label 'DAS-P0-04 Staging fixture root'

Write-Output 'DAS_P0_04_FIXTURE_RESIDUE_COUNT=0'
exit 0
