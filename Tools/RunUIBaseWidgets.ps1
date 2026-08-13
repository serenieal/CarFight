# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.1.1
# Date: 2026-08-10
# Description: ApplyUIBaseWidgets.py를 공식 UE 5.8 Python 실행 경로에서 Probe/DryRun/Apply/Readback 모드로 실행합니다.
# Scope: Probe·DryRun·Readback은 Commandlet, 실제 5개 Widget Blueprint 생성 Apply만 Full Editor에서 실행합니다.
# Changelog:
# - v1.1.1: PowerShell의 GUI 실행 비동기 반환을 피하도록 Apply Full Editor를 Start-Process -Wait로 실행하고 실제 ExitCode를 수집.
# - v1.1.0: D1-10B Probe/DryRun/Apply/Readback 실행 경로를 정식 활성화하고 Apply 범위를 정확한 Base Widget 5종으로 고정.
# - v1.0.1: 열린 동일 프로젝트 Editor가 Full Editor Probe를 흡수하는 상황을 피하기 위해 Probe·DryRun·Readback을 UnrealEditor-Cmd PythonScript 경로로 분리.
# - v1.0.0: CarFightEnv 검증, Full Editor Python Probe 실행과 UTF-8 JSON 결과 검증을 최초 추가.
# Migration:
# - 기본 실행은 Probe이며 Content를 생성·수정하지 않습니다.
# - -DryRun과 -Readback은 읽기 전용이며 -Apply만 /Game/CarFight/UI/Base/의 정확한 5개 Widget Blueprint를 생성·검증·저장합니다.

[CmdletBinding()]
param(
    # [v1.0.0] UE 5.8 Widget Blueprint Python Reflection만 읽기 전용으로 수집합니다.
    [Parameter(Mandatory = $false)]
    [switch]$Probe,

    # [v1.0.0] 후속 버전에서 D1-10B 변경 계획만 검증할 모드입니다.
    [Parameter(Mandatory = $false)]
    [switch]$DryRun,

    # [v1.0.0] 후속 버전에서 정확한 5개 Base Widget Asset 생성을 명시적으로 허용할 모드입니다.
    [Parameter(Mandatory = $false)]
    [switch]$Apply,

    # [v1.0.0] 후속 버전에서 저장된 5개 Base Widget을 변경 없이 재검증할 모드입니다.
    [Parameter(Mandatory = $false)]
    [switch]$Readback
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# [v1.0.0] 현재 PowerShell Runner가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# [v1.0.0] Tools 디렉터리 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# [v1.0.0] Engine/Project 경로를 공식 UE 5.8 기준으로 검증할 공용 환경 스크립트입니다.
$EnvironmentGuard = Join-Path $ToolsDirectory 'CarFightEnv.bat'

# [v1.0.0] D1-10B Unreal Python 도구 경로입니다.
$PythonScript = Join-Path $ToolsDirectory 'ApplyUIBaseWidgets.py'

# [v1.0.0] Python 도구가 기록할 구조화 결과 JSON 경로입니다.
$ReportPath = Join-Path $RepositoryRoot 'UE\Saved\UIBaseWidgets\report.json'

# [v1.0.0] 동시에 둘 이상의 실행 모드를 선택하지 않았는지 확인할 선택 개수입니다.
$ModeCount = @($Probe, $DryRun, $Apply, $Readback | Where-Object { $_ }).Count
if ($ModeCount -gt 1)
{
    throw 'Choose only one of -Probe, -DryRun, -Apply or -Readback.'
}

# [v1.0.0] 아무 스위치도 없을 때 가장 안전한 Probe를 기본으로 사용하는 실행 모드입니다.
$RunMode = if ($DryRun) { 'dry_run' } elseif ($Apply) { 'apply' } elseif ($Readback) { 'readback' } else { 'probe' }

foreach ($RequiredFile in @($EnvironmentGuard, $PythonScript))
{
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf))
    {
        throw "Required file was not found: $RequiredFile"
    }
}

# [v1.0.0] CarFightEnv.bat가 출력한 현재 Process용 환경 변수 목록입니다.
$EnvironmentLines = & cmd.exe /d /s /c "call `"$EnvironmentGuard`" && set CARFIGHT_"
if ($LASTEXITCODE -ne 0)
{
    throw "CarFightEnv.bat failed with exit code $LASTEXITCODE"
}

foreach ($EnvironmentLine in $EnvironmentLines)
{
    if ($EnvironmentLine -match '^(CARFIGHT_[^=]+)=(.*)$')
    {
        [Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
    }
}

# [v1.0.1] 실제 Widget Blueprint Apply에 사용할 공용 환경 검증 UnrealEditor.exe 경로입니다.
$EditorExecutable = $env:CARFIGHT_EDITOR_EXE

# [v1.0.1] Probe·DryRun·Readback에 사용할 공용 환경 검증 UnrealEditor-Cmd.exe 경로입니다.
$EditorCommand = $env:CARFIGHT_EDITOR_CMD_EXE

# [v1.0.0] 공용 환경 검증에서 확정된 CarFight uproject 경로입니다.
$ProjectFile = $env:CARFIGHT_UPROJECT

# [v1.0.0] Editor가 프로젝트 상대 경로를 올바르게 해석할 작업 디렉터리입니다.
$UnrealWorkingDirectory = $env:CARFIGHT_UE_DIR

foreach ($RequiredFile in @($ProjectFile, $EditorExecutable, $EditorCommand))
{
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf))
    {
        throw "Required file was not found after environment guard: $RequiredFile"
    }
}

if (Test-Path -LiteralPath $ReportPath -PathType Leaf)
{
    Remove-Item -LiteralPath $ReportPath -Force
}

# [v1.0.0] 실행 뒤 원래 환경으로 복구할 이전 D1-10B 실행 모드 값입니다.
$PreviousRunMode = $env:CARFIGHT_UI_BASE_MODE
$env:CARFIGHT_UI_BASE_MODE = $RunMode

# [v1.0.1] Apply는 Widget Blueprint 실제 생성이므로 UMGEditor가 준비된 Full Editor Python 경로를 예약합니다.
if ($RunMode -eq 'apply')
{
    # [v1.0.1] Full Editor Python Apply에 전달할 인수 목록입니다.
    $EditorProgram = $EditorExecutable
    # [v1.0.1] Full Editor의 Python Script 실행 인수입니다.
    $EditorArguments = @(
        $ProjectFile,
        "-ExecutePythonScript=$PythonScript",
        '-unattended',
        '-nop4',
        '-NoSplash',
        '-NullRHI',
        '-NoSound'
    )
}
else
{
    # [v1.0.1] Slate나 Widget Editor UI를 필요로 하지 않는 Probe·DryRun·Readback의 Commandlet 실행 파일입니다.
    $EditorProgram = $EditorCommand
    # [v1.0.1] PythonScript Commandlet에 전달할 안전한 인수 목록입니다.
    $EditorArguments = @(
        $ProjectFile,
        '-run=pythonscript',
        "-script=$PythonScript",
        '-unattended',
        '-nop4',
        '-NoSplash',
        '-NullRHI'
    )
}

Write-Host '[CarFight] D1-10B UI Base Widget Tool'
Write-Host ("Mode: {0}" -f $RunMode)

Push-Location $UnrealWorkingDirectory
try
{
    if ($RunMode -eq 'apply')
    {
        # [v1.1.1] GUI 서브시스템 UnrealEditor.exe가 Python Apply와 Editor 종료까지 완료될 때까지 기다릴 Process입니다.
        $EditorProcess = Start-Process -FilePath $EditorProgram -ArgumentList $EditorArguments -WorkingDirectory $UnrealWorkingDirectory -Wait -PassThru
        # [v1.1.1] Full Editor Process가 실제 종료하면서 반환한 ExitCode입니다.
        $EditorExitCode = $EditorProcess.ExitCode
    }
    else
    {
        & $EditorProgram @EditorArguments
        # [v1.0.0] UnrealEditor-Cmd 프로세스가 반환한 실제 종료 코드입니다.
        $EditorExitCode = $LASTEXITCODE
    }
}
finally
{
    Pop-Location

    if ($null -eq $PreviousRunMode)
    {
        Remove-Item Env:CARFIGHT_UI_BASE_MODE -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_UI_BASE_MODE = $PreviousRunMode
    }
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf))
{
    throw "UI Base Widget report was not generated. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

# [v1.0.0] Unreal Python이 UTF-8로 기록한 전체 D1-10B 보고서 텍스트입니다.
$ReportText = [System.IO.File]::ReadAllText(
    $ReportPath,
    [System.Text.UTF8Encoding]::new($false))

# [v1.0.0] 실행 성공 여부와 실제 실행 모드를 검증할 구조화 보고서 객체입니다.
$Report = $ReportText | ConvertFrom-Json
if (-not $Report.success)
{
    throw "UI Base Widget report returned success=false. Editor exit code: $EditorExitCode, Report: $ReportPath"
}
if ($Report.mode -ne $RunMode)
{
    throw "UI Base Widget report mode mismatch. Expected=$RunMode Actual=$($Report.mode)"
}
if ($EditorExitCode -ne 0)
{
    throw "UnrealEditor exited with code $EditorExitCode. Report: $ReportPath"
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host "EDITOR_EXIT_CODE=$EditorExitCode"
Write-Host '[CarFight] D1-10B UI Base Widget Tool completed successfully.'
exit 0
