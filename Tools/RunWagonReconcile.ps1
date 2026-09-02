# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonReconcile.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 actual Wagon post-WSA Chassis SocketScale + Recipe AssetAdoption reconciliation commandlet을 exact1 실행합니다.
# Changelog:
# - v1.0.0: CarFight Editor0 fail-closed guard, CFBuilderWagonReconcile exact commandlet, terminal exit propagation을 최초 구현.
# Migration:
# - Build를 수행하지 않습니다. Official Build PASS 뒤에만 실행합니다.
# - 같은 CarFight Editor가 실행 중이면 즉시 차단하며 Editor lifecycle/Save All/강제 종료를 수행하지 않습니다.
# - commandlet 자체가 exact Wagon Chassis + Recipe pair-save/rollback을 소유하고 Target은 저장하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# CarFight repository root입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 공식 UE 5.8 Source Build UnrealEditor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# CarFight project file입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 현재 CarFight project를 소유하는 UnrealEditor process입니다.
$RunningCarFightEditors = @(
    Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine -like '*CarFight_Re.uproject*' }
)

if ($RunningCarFightEditors.Count -gt 0) {
    Write-Error '[CarFight] Wagon reconciliation blocked: CarFight UnrealEditor is already running.'
    exit 20
}

foreach ($RequiredFile in @($EditorExecutable, $ProjectFile)) {
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf)) {
        Write-Error "[CarFight] Wagon reconciliation blocked: required file is missing: $RequiredFile"
        exit 21
    }
}

Write-Host '[CarFight] Running actual Wagon post-WSA reconciliation exact1...'

# Bounded commandlet arguments입니다.
$Arguments = @(
    $ProjectFile,
    '-run=CFBuilderWagonReconcile',
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nullrhi',
    '-stdout',
    '-FullStdOutLogOutput'
)

# Exact commandlet terminal process입니다.
$Process = Start-Process -FilePath $EditorExecutable -ArgumentList $Arguments -Wait -PassThru -NoNewWindow
$ExitCode = $Process.ExitCode

if ($ExitCode -ne 0) {
    Write-Error "[CarFight] Wagon reconciliation failed. Exit code: $ExitCode"
    exit $ExitCode
}

Write-Host '[CarFight] actual Wagon post-WSA reconciliation PASS.'
exit 0
