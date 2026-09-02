# Copyright (c) CarFight. All Rights Reserved.
# File: CopyVT12Review.ps1
# Version: v1.0.0
# Date: 2026-08-28
# Description: VT12 actual-asset prototype과 reference comparison을 OneDrive Review 폴더로 exact copy하고 SHA를 검증합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$ReviewRoot = 'C:\Users\chaeksong\OneDrive - Smilegate\GoPyMCP_Review'

if (-not (Test-Path -LiteralPath $ReviewRoot -PathType Container)) {
    throw "OneDrive review root missing: $ReviewRoot"
}

$Pairs = @(
    @('SourceArt\UI\HUD\VT\VT12_VehPanelPrototype.png', 'CarFight_VT12_01_VehPanelPrototype.png'),
    @('SourceArt\UI\HUD\VT\VT12_ReferenceCompare.png', 'CarFight_VT12_02_ReferenceCompare.png')
)

foreach ($Pair in $Pairs) {
    $SourcePath = Join-Path $RepositoryRoot $Pair[0]
    $TargetPath = Join-Path $ReviewRoot $Pair[1]

    if (-not (Test-Path -LiteralPath $SourcePath -PathType Leaf)) {
        throw "VT12 review source missing: $SourcePath"
    }

    Copy-Item -LiteralPath $SourcePath -Destination $TargetPath -Force

    $SourceSha = (Get-FileHash -LiteralPath $SourcePath -Algorithm SHA256).Hash.ToLowerInvariant()
    $TargetSha = (Get-FileHash -LiteralPath $TargetPath -Algorithm SHA256).Hash.ToLowerInvariant()

    if ($SourceSha -ne $TargetSha) {
        throw "VT12 review SHA mismatch: source=$SourceSha target=$TargetSha"
    }

    Write-Output ("VT12_COPY_PASS file={0} sha256={1}" -f $Pair[1], $TargetSha)
}

exit 0
