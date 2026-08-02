# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-07-29
# Description: CarFight Unreal 자산을 변경하지 않고 AssetDump Details 프로필로 읽는 보조 스크립트
# Changelog:
# - v1.0.0: Root, DumpRoot, MaxAssets를 받아 Summary·Details·References BatchDump 실행 경로 추가.
# Migration:
# - 이 스크립트는 .uasset을 저장하거나 수정하지 않는다. Editor 자산 변경은 별도 승인된 도구 또는 사용자 Editor 작업으로 수행한다.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^/Game(?:/.*)?$')]
    [string]$Root,

    [Parameter(Mandatory = $false)]
    [ValidateRange(1, 500)]
    [int]$MaxAssets = 100,

    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source',

    [Parameter(Mandatory = $false)]
    [string]$DumpRoot = 'Plugins/ue-assetdump/Dumped/LauncherDetail'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepositoryRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
$EditorCommand = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$UnrealWorkingDirectory = Join-Path $RepositoryRoot 'UE'
$ResolvedDumpRoot = if ([System.IO.Path]::IsPathRooted($DumpRoot)) {
    [System.IO.Path]::GetFullPath($DumpRoot)
}
else {
    [System.IO.Path]::GetFullPath((Join-Path $UnrealWorkingDirectory ($DumpRoot -replace '/', '\')))
}

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    throw "CarFight project file was not found: $ProjectFile"
}

if (-not (Test-Path -LiteralPath $EditorCommand -PathType Leaf)) {
    throw "UnrealEditor-Cmd.exe was not found: $EditorCommand"
}

$Arguments = @(
    $ProjectFile,
    '-run=AssetDump',
    '-Mode=batchdump',
    "-Root=$Root",
        "-DumpRoot=$ResolvedDumpRoot",
    '-IncludeSummary=true',
    '-IncludeDetails=true',
    '-IncludeGraphs=false',
    '-IncludeReferences=true',
    '-ChangedOnly=false',
    '-WithDependencies=false',
    "-MaxAssets=$MaxAssets",
    '-RebuildIndex=true',
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI'
)

Write-Host "[CarFight] Read-only AssetDump Details"
Write-Host "Root: $Root"
Write-Host "DumpRoot: $ResolvedDumpRoot"
Write-Host "MaxAssets: $MaxAssets"

Push-Location $UnrealWorkingDirectory
try {
    & $EditorCommand @Arguments
    $EditorExitCode = $LASTEXITCODE
}
finally {
    Pop-Location
}

if ($EditorExitCode -ne 0) {
    throw "AssetDump commandlet failed with exit code $EditorExitCode."
}

$RunReportPath = Join-Path $ResolvedDumpRoot 'run_report.json'

if (-not (Test-Path -LiteralPath $RunReportPath -PathType Leaf)) {
    throw "AssetDump run report was not generated: $RunReportPath"
}

Write-Host "RESULT_JSON_PATH=$RunReportPath"
Write-Host '[CarFight] AssetDump Details completed successfully.'
exit 0
