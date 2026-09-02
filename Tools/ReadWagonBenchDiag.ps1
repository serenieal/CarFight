# File: ReadWagonBenchDiag.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Purpose: existing Wagon benchmark log에서 bounded diagnostic lines만 read-only 출력합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$LogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFVehicleBuilderBench.log'

if (-not (Test-Path -LiteralPath $LogPath -PathType Leaf)) {
    throw "Benchmark log가 없습니다: $LogPath"
}

$Lines = Get-Content -LiteralPath $LogPath -Encoding UTF8
$Lines |
    Where-Object {
        $_ -match 'VB-P0-08|Test Completed|Error:|TorqueCurveSample|HighSpeedSample'
    } |
    Select-Object -Last 240 |
    ForEach-Object { Write-Output $_ }

exit 0
