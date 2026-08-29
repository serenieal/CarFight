# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWSAMigrate.ps1
# Version: v1.0.0
# Date: 2026-08-28
# Description: WSA-P0-05 Wagon migration commandlet을 공식 UE 5.8 Source Build에서 exact1 실행합니다.
# Changelog:
# - v1.0.0: Editor0 fail-closed guard와 CFWSAMigrate one-shot commandlet 실행을 추가.
# Migration:
# - 이 wrapper는 Build를 수행하지 않습니다. Official Build PASS 뒤에만 실행합니다.
# - 같은 CarFight Editor가 실행 중이면 즉시 차단하며 Save All/Editor lifecycle 강제 종료를 하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# CarFight 저장소 루트 절대 경로입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 공식 UE 5.8 Source Build UnrealEditor 실행 파일입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# CarFight 프로젝트 파일 절대 경로입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# 현재 실행 중인 UnrealEditor 중 CarFight 프로젝트를 소유하는 프로세스 목록입니다.
$RunningCarFightEditors = @(
    Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine -like '*CarFight_Re.uproject*' }
)

if ($RunningCarFightEditors.Count -gt 0) {
    Write-Error '[CarFight] WSA migration blocked: CarFight UnrealEditor is already running.'
    exit 20
}

foreach ($RequiredFile in @($EditorExecutable, $ProjectFile)) {
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf)) {
        Write-Error "[CarFight] WSA migration blocked: required file is missing: $RequiredFile"
        exit 21
    }
}

Write-Host '[CarFight] Running WSA-P0-05 Wagon migration exact1...'

# CFWSAMigrate commandlet에 전달할 고정 인수 목록입니다.
$CommandletArguments = @(
    $ProjectFile,
    '-run=CFWSAMigrate',
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nullrhi',
    '-stdout',
    '-FullStdOutLogOutput'
)

# Wagon migration commandlet가 실제 종료할 때까지 기다리는 프로세스 객체입니다.
$CommandletProcess = Start-Process -FilePath $EditorExecutable -ArgumentList $CommandletArguments -Wait -PassThru -NoNewWindow

# Unreal commandlet가 반환한 실제 종료 코드입니다.
$CommandletExitCode = $CommandletProcess.ExitCode

if ($CommandletExitCode -ne 0) {
    Write-Error "[CarFight] WSA migration failed. Exit code: $CommandletExitCode"
    exit $CommandletExitCode
}

Write-Host '[CarFight] WSA-P0-05 Wagon migration PASS.'
exit 0
