# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonTorqueInspect.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 BP_CFVehiclePawn inherited Chaos TorqueCurve read-only Automation exact1 runner입니다.
# Changelog:
# - v1.0.0: TorqueCurveInspect exact test를 headless UnrealEditor에서 실행하고 bounded diagnostic line만 출력.
# Migration:
# - Product Asset/Map/Config/PIE mutation을 하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Repository root입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path

# 공식 CarFight project file입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 공식 UE 5.8 Source Build UnrealEditor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# Exact Automation test입니다.
$TestFilter = 'CarFight.VehicleBuilder.CF_FQ_040.VB_P0_08.TorqueCurveInspect'

# 이번 inspect 전용 로그입니다.
$LogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFWagonTorqueInspect.log'

if (Test-Path -LiteralPath $LogPath -PathType Leaf) {
    Remove-Item -LiteralPath $LogPath -Force
}

$Arguments = @(
    $ProjectFile,
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI',
    ('-ExecCmds="Automation RunTests {0}"' -f $TestFilter),
    '-TestExit="Automation Test Queue Empty"',
    "-abslog=$LogPath",
    '-stdout',
    '-FullStdOutLogOutput'
)

# Exact UnrealEditor process입니다.
$Process = Start-Process -FilePath $EditorExecutable -ArgumentList $Arguments -WorkingDirectory $RepositoryRoot -PassThru
$Process.WaitForExit()
$ExitCode = $Process.ExitCode

if (-not (Test-Path -LiteralPath $LogPath -PathType Leaf)) {
    throw "TorqueCurveInspect log가 생성되지 않았습니다. ExitCode=$ExitCode"
}

# Result와 diagnostic line을 추출합니다.
$LogText = [System.IO.File]::ReadAllText($LogPath, [System.Text.UTF8Encoding]::new($false))
$Success = $LogText -match ('Test Completed\. Result=\{Success\}.*?Path=\{' + [regex]::Escape($TestFilter) + '\}')
$DiagnosticLines = @([regex]::Split($LogText, '\r?\n') | Where-Object {
    $_ -match 'TorqueCurveInspect|Test Completed|Error:'
} | Select-Object -Last 80)

$DiagnosticLines | ForEach-Object { Write-Output $_ }

if ($ExitCode -ne 0 -or -not $Success) {
    exit 1
}

exit 0
