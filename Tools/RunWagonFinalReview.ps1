# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonFinalReview.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 actual Wagon Step 7 mutation0 Final Review exact1 wrapper입니다.
# Changelog:
# - v1.0.0: Editor0 guard와 CFBuilderWagonReview exact1 실행을 추가.
# Migration:
# - Build를 수행하지 않습니다. Official Build PASS 뒤에만 실행합니다.
# - Product Asset save/apply/undo는 수행하지 않고 Saved review diagnostic만 기록합니다.

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
    Write-Error '[CarFight] Wagon Final Review blocked: CarFight UnrealEditor is already running.'
    exit 20
}

foreach ($RequiredFile in @($EditorExecutable, $ProjectFile)) {
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf)) {
        Write-Error "[CarFight] Wagon Final Review blocked: required file is missing: $RequiredFile"
        exit 21
    }
}

Write-Host '[CarFight] Running actual Wagon Step7 mutation0 Final Review exact1...'

# Read-only one-shot commandlet arguments입니다.
$Arguments = @(
    $ProjectFile,
    '-run=CFBuilderWagonReview',
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
    Write-Error "[CarFight] Wagon Final Review failed. Exit code: $ExitCode"
    exit $ExitCode
}

Write-Host '[CarFight] actual Wagon Step7 mutation0 Final Review PASS.'
exit 0
