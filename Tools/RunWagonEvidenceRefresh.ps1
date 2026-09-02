# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonEvidenceRefresh.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 actual Wagon canonical ResearchDraft → existing Reference Evidence Refresh exact1 wrapper입니다.
# Changelog:
# - v1.0.0: Editor0 guard와 CFBuilderWagonEvidence commandlet exact1 실행을 추가.
# Migration:
# - Build를 수행하지 않습니다.
# - exact DA_Ref_Wagon package 외에는 저장하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Repository root입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 공식 UE 5.8 Source Build UnrealEditor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# CarFight project file입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 같은 project의 full Editor가 실행 중인지 확인합니다.
$RunningCarFightEditors = @(
    Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine -like '*CarFight_Re.uproject*' }
)

if ($RunningCarFightEditors.Count -gt 0) {
    Write-Error '[CarFight] Wagon Evidence Refresh blocked: CarFight UnrealEditor is already running.'
    exit 20
}

foreach ($RequiredFile in @($EditorExecutable, $ProjectFile)) {
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf)) {
        Write-Error "[CarFight] Wagon Evidence Refresh blocked: required file is missing: $RequiredFile"
        exit 21
    }
}

Write-Host '[CarFight] Running actual Wagon Evidence Refresh exact1...'

# One-shot commandlet args입니다.
$Arguments = @(
    $ProjectFile,
    '-run=CFBuilderWagonEvidence',
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nullrhi',
    '-stdout',
    '-FullStdOutLogOutput'
)

# Terminal process입니다.
$Process = Start-Process -FilePath $EditorExecutable -ArgumentList $Arguments -Wait -PassThru -NoNewWindow
$ExitCode = $Process.ExitCode

if ($ExitCode -ne 0) {
    Write-Error "[CarFight] Wagon Evidence Refresh failed. Exit code: $ExitCode"
    exit $ExitCode
}

Write-Host '[CarFight] actual Wagon Evidence Refresh PASS.'
exit 0
