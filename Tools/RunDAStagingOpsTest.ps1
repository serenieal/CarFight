# CarFight Data Asset Staging DAS-P0-05 operational entry Automation runner.
# Version: v1.2.1
# Date: 2026-09-09
# Description: CF-FQ-049 DAS-P0-05 selected Preview→Review state machine, Product mutation0와 Sync rollback exact 1종을 canonical Staging raw-byte residue gate와 함께 실행합니다.
# Changelog:
# - v1.2.1: Windows PowerShell 5.1에서 generic static SequenceEqual 구문이 파싱되지 않는 문제를 explicit byte-length/index 비교로 교정.
# - v1.2.0: Product canonical Staging 3종의 pre-run raw bytes를 snapshot하고 post-run exact 동일성을 검증하며, mismatch 시 원본 복원 뒤 runner 자체를 실패시키는 residue gate를 추가.
# - v1.1.0: selection-bound Preview/Review, stale-review rejection와 forced partial-write rollback 검증을 현재 OperationalEntry exact 1종에 포함.
# - v1.0.0: OperationalEntry exact 1종 실행 경로를 추가.
# Migration:
# - 이 runner는 Product Low/Normal/High UE Asset Apply/Save를 수행하지 않습니다. canonical Product Staging JSON은 test 중 일시 변경될 수 있으나 runner 종료 시 pre-run raw bytes와 exact 동일해야 PASS입니다.
# - post-run mismatch가 있으면 runner가 pre-run bytes를 복원한 뒤 실패합니다.
# - durable Apply/Save safety 회귀는 기존 RunDAStagingTests.ps1 exact 13종이 별도로 소유합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# Tools 폴더의 부모인 main_game repository root입니다.
$MainGameRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))

# DAS-P0-05 Product canonical Staging 3종 physical paths입니다.
$ProductStagingPaths = @(
    (Join-Path $MainGameRoot 'Authoring\DataAssetStaging\MissileGuidePreset\MissileFeel_Low.json'),
    (Join-Path $MainGameRoot 'Authoring\DataAssetStaging\MissileGuidePreset\MissileFeel_Normal.json'),
    (Join-Path $MainGameRoot 'Authoring\DataAssetStaging\MissileGuidePreset\MissileFeel_High.json')
)

# DAS-P0-05 operational entry exact test 목록입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_049.DAS_P0_05.OperationalEntry'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

# test 시작 전 Product Staging raw bytes snapshot입니다.
$PreRunBytesByPath = @{}
foreach ($ProductStagingPath in $ProductStagingPaths) {
    if (-not (Test-Path -LiteralPath $ProductStagingPath -PathType Leaf)) {
        throw "DAS-P0-05 Product canonical Staging file이 없습니다: $ProductStagingPath"
    }
    $PreRunBytesByPath[$ProductStagingPath] = [System.IO.File]::ReadAllBytes($ProductStagingPath)
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# underlying exact-list Automation exit code입니다.
$AutomationExitCode = $LASTEXITCODE

# post-run canonical Staging raw-byte mismatch 목록입니다.
$ResiduePaths = @()
foreach ($ProductStagingPath in $ProductStagingPaths) {
    $BeforeBytes = [byte[]]$PreRunBytesByPath[$ProductStagingPath]
    $IsExactSame = $false
    if (Test-Path -LiteralPath $ProductStagingPath -PathType Leaf) {
        $AfterBytes = [System.IO.File]::ReadAllBytes($ProductStagingPath)
        if ($BeforeBytes.Length -eq $AfterBytes.Length) {
            $IsExactSame = $true
            for ($ByteIndex = 0; $ByteIndex -lt $BeforeBytes.Length; $ByteIndex++) {
                if ($BeforeBytes[$ByteIndex] -ne $AfterBytes[$ByteIndex]) {
                    $IsExactSame = $false
                    break
                }
            }
        }
    }

    if (-not $IsExactSame) {
        $ResiduePaths += $ProductStagingPath
        [System.IO.File]::WriteAllBytes($ProductStagingPath, $BeforeBytes)
    }
}

if ($ResiduePaths.Count -gt 0) {
    Write-Error ("DAS-P0-05 Product Staging test residue를 감지해 pre-run bytes로 복원했습니다: " + ($ResiduePaths -join ', '))
    exit 1
}

Write-Output 'DAS_P0_05_PRODUCT_STAGING_RESIDUE_COUNT=0'
exit $AutomationExitCode
