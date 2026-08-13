# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-10
# Description: ApplyUIHUDPrototype.py를 공식 UE 5.8 Probe/DryRun/Apply/Readback 경로에서 실행합니다.
# Scope: D1-11 WBP_CFInGameHUD 한 개만 Full Editor Apply하고 나머지 모드는 읽기 전용 Commandlet로 실행합니다.
# Changelog:
# - v1.0.0: CarFightEnv 검증, 단일 HUD Prototype allowlist, Full Editor Wait와 UTF-8 Report 검증을 최초 추가.
# Migration:
# - 기본 실행은 Probe입니다.
# - -DryRun/-Readback은 읽기 전용이고 -Apply만 /Game/CarFight/UI/HUD/WBP_CFInGameHUD 한 개를 생성할 수 있습니다.

[CmdletBinding()]
param(
    # [v1.0.0] D1-11 필수 Python/C++ 타입과 읽기 전용 Dependency만 확인합니다.
    [Parameter(Mandatory = $false)]
    [switch]$Probe,

    # [v1.0.0] WBP_CFInGameHUD 생성 전 단일 Target allowlist와 보호 상태를 검증합니다.
    [Parameter(Mandatory = $false)]
    [switch]$DryRun,

    # [v1.0.0] 정확한 WBP_CFInGameHUD 한 개 생성을 명시적으로 허용합니다.
    [Parameter(Mandatory = $false)]
    [switch]$Apply,

    # [v1.0.0] 저장된 WBP_CFInGameHUD를 새 프로세스에서 변경 없이 검증합니다.
    [Parameter(Mandatory = $false)]
    [switch]$Readback
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# [v1.0.0] 현재 Runner가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# [v1.0.0] Tools 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# [v1.0.0] 공식 UE 5.8 Engine/Project 경로를 검증할 공용 환경 스크립트입니다.
$EnvironmentGuard = Join-Path $ToolsDirectory 'CarFightEnv.bat'

# [v1.0.0] D1-11 Unreal Python 제작/검증 도구 경로입니다.
$PythonScript = Join-Path $ToolsDirectory 'ApplyUIHUDPrototype.py'

# [v1.0.0] Python 도구가 기록할 D1-11 구조화 결과 JSON 경로입니다.
$ReportPath = Join-Path $RepositoryRoot 'UE\Saved\UIHUDPrototype\report.json'

# [v1.0.0] 동시에 여러 실행 모드를 선택하지 않았는지 확인할 선택 개수입니다.
$ModeCount = @($Probe, $DryRun, $Apply, $Readback | Where-Object { $_ }).Count
if ($ModeCount -gt 1)
{
    throw 'Choose only one of -Probe, -DryRun, -Apply or -Readback.'
}

# [v1.0.0] 아무 스위치도 없으면 가장 안전한 Probe를 사용하는 실제 실행 모드입니다.
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

# [v1.0.0] 실제 Widget Blueprint Apply에 사용할 공식 UnrealEditor.exe 경로입니다.
$EditorExecutable = $env:CARFIGHT_EDITOR_EXE

# [v1.0.0] Probe·DryRun·Readback에 사용할 공식 UnrealEditor-Cmd.exe 경로입니다.
$EditorCommand = $env:CARFIGHT_EDITOR_CMD_EXE

# [v1.0.0] 공용 환경 검증에서 확정된 CarFight uproject 경로입니다.
$ProjectFile = $env:CARFIGHT_UPROJECT

# [v1.0.0] Unreal 상대 경로 해석에 사용할 UE 프로젝트 작업 디렉터리입니다.
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

# [v1.0.0] 실행 뒤 원래 환경으로 복구할 이전 D1-11 실행 모드 값입니다.
$PreviousRunMode = $env:CARFIGHT_UI_HUD_MODE
$env:CARFIGHT_UI_HUD_MODE = $RunMode

if ($RunMode -eq 'apply')
{
    # [v1.0.0] 실제 UMG Widget Blueprint 생성에 사용할 Full Editor 프로그램입니다.
    $EditorProgram = $EditorExecutable
    # [v1.0.0] Full Editor Python Apply 인수입니다.
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
    # [v1.0.0] 읽기 전용 모드에 사용할 Commandlet 프로그램입니다.
    $EditorProgram = $EditorCommand
    # [v1.0.0] PythonScript Commandlet 읽기 전용 실행 인수입니다.
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

Write-Host '[CarFight] D1-11 UI HUD Prototype Tool'
Write-Host ("Mode: {0}" -f $RunMode)

Push-Location $UnrealWorkingDirectory
try
{
    if ($RunMode -eq 'apply')
    {
        # [v1.0.0] Full Editor가 Python Apply와 종료를 모두 마칠 때까지 대기할 Process입니다.
        $EditorProcess = Start-Process -FilePath $EditorProgram -ArgumentList $EditorArguments -WorkingDirectory $UnrealWorkingDirectory -Wait -PassThru
        # [v1.0.0] Full Editor 실제 Exit Code입니다.
        $EditorExitCode = $EditorProcess.ExitCode
    }
    else
    {
        & $EditorProgram @EditorArguments
        # [v1.0.0] UnrealEditor-Cmd 실제 Exit Code입니다.
        $EditorExitCode = $LASTEXITCODE
    }
}
finally
{
    Pop-Location
    if ($null -eq $PreviousRunMode)
    {
        Remove-Item Env:CARFIGHT_UI_HUD_MODE -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_UI_HUD_MODE = $PreviousRunMode
    }
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf))
{
    throw "UI HUD Prototype report was not generated. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

# [v1.0.0] Unreal Python이 UTF-8로 기록한 전체 D1-11 보고서 텍스트입니다.
$ReportText = [System.IO.File]::ReadAllText(
    $ReportPath,
    [System.Text.UTF8Encoding]::new($false))

# [v1.0.0] 실행 성공 여부와 실제 Mode를 검증할 구조화 보고서 객체입니다.
$Report = $ReportText | ConvertFrom-Json
if (-not $Report.success)
{
    throw "UI HUD Prototype report returned success=false. Editor exit code: $EditorExitCode, Report: $ReportPath"
}
if ($Report.mode -ne $RunMode)
{
    throw "UI HUD Prototype report mode mismatch. Expected=$RunMode Actual=$($Report.mode)"
}
if ($EditorExitCode -ne 0)
{
    throw "UnrealEditor exited with code $EditorExitCode. Report: $ReportPath"
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host "EDITOR_EXIT_CODE=$EditorExitCode"
Write-Host '[CarFight] D1-11 UI HUD Prototype Tool completed successfully.'
exit 0
