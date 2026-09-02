# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonFinalApply.ps1
# Version: v1.2.0
# Date: 2026-09-02
# Description: CF-FQ-040 actual Wagon USER-approved ESH-03 ChangeUpRPM 5500 Step 7 DefinitionApply exact1 wrapper입니다.
# Changelog:
# - v1.2.0: fresh Final Review에서 확정한 ChangeUpRPM 4500→5500 DefinitionApply ProposalHash 226ea77d...로 exact approval binding을 갱신.
# - v1.1.0: historical 8AT 승인 hash를 current Engine Curve DefinitionApply ProposalHash 809c5523...로 교체.
# - v1.0.0: 직전 USER-approved ProposalHash를 exact commandlet argument로 전달하고 Editor0 guard를 적용.
# Migration:
# - 승인 hash는 fresh Final Review artifact와 exact binding하며 변경되면 fail-closed합니다.
# - commandlet 내부에서 current proposal/diff/pre/post exact guard를 통과한 existing Builder R3 DefinitionApply만 사용하고 Target+Recipe만 저장합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Repository root입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 공식 UE 5.8 Source Build UnrealEditor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# CarFight project file입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# USER가 fresh ESH-03 5500 Final Review에서 승인한 exact DefinitionApply ProposalHash입니다.
$ApprovedHash = '226ea77d192e1af864270ae1687bdbda'

# 같은 project를 소유하는 full UnrealEditor가 실행 중이면 commandlet을 차단합니다.
$RunningCarFightEditors = @(
    Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine -like '*CarFight_Re.uproject*' }
)

if ($RunningCarFightEditors.Count -gt 0) {
    Write-Error '[CarFight] Wagon Final Apply blocked: CarFight UnrealEditor is already running.'
    exit 20
}

foreach ($RequiredFile in @($EditorExecutable, $ProjectFile)) {
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf)) {
        Write-Error "[CarFight] Wagon Final Apply blocked: required file is missing: $RequiredFile"
        exit 21
    }
}

Write-Host "[CarFight] Running actual Wagon USER-approved Step7 DefinitionApply exact1: $ApprovedHash"

# Exact approval-bound one-shot commandlet arguments입니다.
$Arguments = @(
    $ProjectFile,
    '-run=CFBuilderWagonReview',
    "-CFWagonApplyApprovedHash=$ApprovedHash",
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
    Write-Error "[CarFight] Wagon Final Apply failed. Exit code: $ExitCode"
    exit $ExitCode
}

Write-Host '[CarFight] actual Wagon USER-approved Step7 DefinitionApply PASS.'
exit 0
