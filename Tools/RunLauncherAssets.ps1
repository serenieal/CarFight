# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-07-29
# Description: ApplyLauncherAssets.py를 UnrealEditor-Cmd에서 Dry Run 또는 Apply로 실행합니다.
# Changelog:
# - v1.0.0: 기본 Dry Run과 명시적 -Apply 실행, JSON 보고서 검증을 추가.
# Migration:
# - -Apply를 전달하지 않으면 .uasset을 저장하지 않습니다.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [string]$EngineRoot = 'D:\UnrealEngine_Source',

    [Parameter(Mandatory = $false)]
    [switch]$Apply
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepositoryRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
$PythonScript = Join-Path $PSScriptRoot 'ApplyLauncherAssets.py'
$EditorCommand = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$UnrealWorkingDirectory = Join-Path $RepositoryRoot 'UE'
$ReportPath = Join-Path $UnrealWorkingDirectory 'Saved\LauncherAssetApply\report.json'

foreach ($RequiredFile in @($ProjectFile, $PythonScript, $EditorCommand)) {
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf)) {
        throw "Required file was not found: $RequiredFile"
    }
}

$PreviousApplyValue = $env:CARFIGHT_LAUNCHER_APPLY
$env:CARFIGHT_LAUNCHER_APPLY = if ($Apply) { '1' } else { '0' }

$Arguments = @(
    $ProjectFile,
    '-run=pythonscript',
    "-script=$PythonScript",
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI'
)

Write-Host '[CarFight] Launcher Asset Tool'
Write-Host ("Mode: {0}" -f $(if ($Apply) { 'Apply' } else { 'DryRun' }))

Push-Location $UnrealWorkingDirectory
try {
    & $EditorCommand @Arguments
    $EditorExitCode = $LASTEXITCODE
}
finally {
    Pop-Location
    if ($null -eq $PreviousApplyValue) {
        Remove-Item Env:CARFIGHT_LAUNCHER_APPLY -ErrorAction SilentlyContinue
    }
    else {
        $env:CARFIGHT_LAUNCHER_APPLY = $PreviousApplyValue
    }
}

if ($EditorExitCode -ne 0) {
    throw "Launcher asset Unreal Python process failed with exit code $EditorExitCode."
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf)) {
    throw "Launcher asset report was not generated: $ReportPath"
}

$Report = Get-Content -LiteralPath $ReportPath -Raw -Encoding UTF8 | ConvertFrom-Json
if (-not $Report.success) {
    throw "Launcher asset report returned success=false: $ReportPath"
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host '[CarFight] Launcher Asset Tool completed successfully.'
exit 0
