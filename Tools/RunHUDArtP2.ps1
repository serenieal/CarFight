# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.1
# Date: 2026-08-19
# Description: CFHUDArtP2 commandlet을 공식 UE 5.8 Source Build UnrealEditor.exe로 실행하는 bounded wrapper입니다.
# Changelog:
# - v1.0.1: PowerShell 5.1에서 GUI UnrealEditor 직접 호출 뒤 $LASTEXITCODE가 설정되지 않을 수 있는 문제를 제거하고 Start-Process -Wait -PassThru의 실제 ExitCode를 사용합니다.
# - v1.0.0: 같은 CarFight 프로젝트 Editor가 이미 실행 중이면 fail-closed하고, Editor0에서만 P2 commandlet을 실행합니다.
# Migration:
# - 일반 Editor 실행은 계속 Tools/RunEditor.bat를 사용합니다.
# - 이 파일은 UI P2 Asset import/build commandlet 전용이며 Save All이나 일반 Editor lifecycle을 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# CarFight 저장소 루트 절대 경로입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 공식 UE 5.8 Source Build UnrealEditor 실행 파일입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# CarFight 프로젝트 파일 절대 경로입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 현재 실행 중인 UnrealEditor 프로세스 중 CarFight 프로젝트를 소유하는 프로세스를 조회합니다.
$RunningCarFightEditors = @(
    Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine -like '*CarFight_Re.uproject*' }
)

if ($RunningCarFightEditors.Count -gt 0) {
    Write-Error "[CarFight] CFHUDArtP2 blocked: CarFight UnrealEditor is already running. Close the Editor before this commandlet."
    exit 20
}

if (-not (Test-Path -LiteralPath $EditorExecutable)) {
    Write-Error "[CarFight] CFHUDArtP2 blocked: official UE 5.8 UnrealEditor.exe is missing."
    exit 21
}

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    Write-Error "[CarFight] CFHUDArtP2 blocked: CarFight_Re.uproject is missing."
    exit 22
}

Write-Host '[CarFight] Running CFHUDArtP2 commandlet...'

# CFHUDArtP2 Unreal commandlet에 전달할 고정 인수 목록입니다.
$CommandletArguments = @(
    $ProjectFile,
    '-run=CFHUDArtP2',
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nullrhi',
    '-stdout',
    '-FullStdOutLogOutput'
)

# GUI 형식 UnrealEditor 프로세스가 실제 종료할 때까지 기다리고 ExitCode를 제공하는 프로세스 객체입니다.
$CommandletProcess = Start-Process `
    -FilePath $EditorExecutable `
    -ArgumentList $CommandletArguments `
    -Wait `
    -PassThru `
    -NoNewWindow

# Unreal commandlet가 반환한 실제 프로세스 종료 코드입니다.
$CommandletExitCode = $CommandletProcess.ExitCode

if ($CommandletExitCode -ne 0) {
    Write-Error "[CarFight] CFHUDArtP2 failed. Exit code: $CommandletExitCode"
    exit $CommandletExitCode
}

Write-Host '[CarFight] CFHUDArtP2 PASS.'
exit 0
