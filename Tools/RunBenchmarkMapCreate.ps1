# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunBenchmarkMapCreate.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 전용 M_VehicleBenchmark 최소 골격 생성 commandlet exact1 wrapper입니다.
# Changelog:
# - v1.0.0: Editor0 fail-closed guard와 CFBenchmarkMap exact1 실행을 추가.
# Migration:
# - 기존 M_VehicleBenchmark가 있으면 commandlet이 overwrite하지 않습니다.
# - map 생성 외 Product Asset mutation은 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Repository root입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 공식 UE 5.8 Source Build UnrealEditor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# CarFight project file입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 같은 project를 소유하는 full UnrealEditor가 실행 중이면 commandlet을 차단합니다.
$RunningCarFightEditors = @(
    Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine -like '*CarFight_Re.uproject*' }
)

if ($RunningCarFightEditors.Count -gt 0) {
    Write-Error '[CarFight] Benchmark map create blocked: CarFight UnrealEditor is already running.'
    exit 20
}

foreach ($RequiredFile in @($EditorExecutable, $ProjectFile)) {
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf)) {
        Write-Error "[CarFight] Benchmark map create blocked: required file is missing: $RequiredFile"
        exit 21
    }
}

Write-Host '[CarFight] Creating /Game/Maps/M_VehicleBenchmark exact1...'

# One-shot commandlet arguments입니다.
$Arguments = @(
    $ProjectFile,
    '-run=CFBenchmarkMap',
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nullrhi',
    '-stdout',
    '-FullStdOutLogOutput'
)

# Terminal commandlet process입니다.
$Process = Start-Process -FilePath $EditorExecutable -ArgumentList $Arguments -Wait -PassThru -NoNewWindow
$ExitCode = $Process.ExitCode

if ($ExitCode -ne 0) {
    Write-Error "[CarFight] Benchmark map create failed. Exit code: $ExitCode"
    exit $ExitCode
}

Write-Host '[CarFight] M_VehicleBenchmark create PASS.'
exit 0
