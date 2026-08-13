# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.2
# Date: 2026-08-10
# Description: ApplyUIAssetization.py를 공식 UnrealEditor-Cmd에서 Probe/DryRun/Apply/Readback 모드로 실행합니다.
# Scope: D1-09B Apply는 Slate가 준비된 Full Editor Python 경로를 사용하고 읽기/사전 검증은 빠른 PythonScript Commandlet을 사용합니다.
# Changelog:
# - v1.0.2: UE 5.8 FontFace Import의 SlateApplication 요구에 맞춰 Apply만 -ExecutePythonScript Full Editor 경로로 전환.
# - v1.0.1: Python 보고서 success=true가 확인된 뒤 발생하는 unrelated Editor 플러그인 종료 코드는 경고로 분리해 기존 CarFight Asset Runner 정책과 일치시킴.
# - v1.0.0: 공식 Engine Guard, Probe 모드 실행과 UTF-8 JSON 결과 검증을 최초 추가.
# Migration:
# - 기본 실행은 Probe이며 Content·Config를 수정하지 않습니다.
# - 후속 도구 버전에서 -DryRun, -Apply, -Readback을 단계적으로 활성화합니다.

[CmdletBinding()]
param(
    # [v1.0.0] UE 5.8 Python Reflection만 읽기 전용으로 수집합니다.
    [Parameter(Mandatory = $false)]
    [switch]$Probe,

    # [v1.0.0] 후속 버전에서 실제 변경 계획만 검증할 모드입니다.
    [Parameter(Mandatory = $false)]
    [switch]$DryRun,

    # [v1.0.0] 후속 버전에서 D1-09B 화이트리스트 Asset 생성을 명시적으로 허용할 모드입니다.
    [Parameter(Mandatory = $false)]
    [switch]$Apply,

    # [v1.0.0] 후속 버전에서 저장된 D1-09B Asset/Config를 읽기 전용으로 재검증할 모드입니다.
    [Parameter(Mandatory = $false)]
    [switch]$Readback
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# [v1.0.0] 이 PowerShell 스크립트가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# [v1.0.0] Tools 디렉터리의 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# [v1.0.0] 공용 Engine 경로 검증과 환경 변수를 제공하는 CarFightEnv.bat입니다.
$EnvironmentGuard = Join-Path $ToolsDirectory 'CarFightEnv.bat'

# [v1.0.0] Unreal Python D1-09B 실행 스크립트입니다.
$PythonScript = Join-Path $ToolsDirectory 'ApplyUIAssetization.py'

# [v1.0.0] Python 도구가 기록할 구조화 결과 JSON 경로입니다.
$ReportPath = Join-Path $RepositoryRoot 'UE\Saved\UIAssetization\report.json'

# [v1.0.0] 동시에 두 개 이상의 실행 모드를 요청하지 않았는지 확인할 선택 개수입니다.
$ModeCount = @($Probe, $DryRun, $Apply, $Readback | Where-Object { $_ }).Count
if ($ModeCount -gt 1)
{
    throw 'Choose only one of -Probe, -DryRun, -Apply or -Readback.'
}

# [v1.0.0] 아무 스위치도 없을 때 안전한 Probe를 기본값으로 사용하는 실행 모드입니다.
$RunMode = if ($DryRun) { 'dry_run' } elseif ($Apply) { 'apply' } elseif ($Readback) { 'readback' } else { 'probe' }

foreach ($RequiredFile in @($EnvironmentGuard, $PythonScript))
{
    if (-not (Test-Path -LiteralPath $RequiredFile -PathType Leaf))
    {
        throw "Required file was not found: $RequiredFile"
    }
}

# [v1.0.0] 공용 CarFight 환경 검증을 현재 Process에 반영하기 위한 cmd 출력입니다.
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

# [v1.0.0] 공용 환경 검증에서 확정된 Unreal Editor Commandlet 경로입니다.
$EditorCommand = $env:CARFIGHT_EDITOR_CMD_EXE

# [v1.0.0] 공용 환경 검증에서 확정된 CarFight uproject 경로입니다.
$ProjectFile = $env:CARFIGHT_UPROJECT

# [v1.0.0] UnrealEditor-Cmd가 프로젝트 상대 경로를 올바르게 해석할 작업 디렉터리입니다.
$UnrealWorkingDirectory = $env:CARFIGHT_UE_DIR

foreach ($RequiredFile in @($ProjectFile, $EditorCommand))
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

# [v1.0.0] 실행 후 원래 환경으로 복구할 이전 D1-09B 실행 모드 값입니다.
$PreviousRunMode = $env:CARFIGHT_UI_ASSET_MODE
$env:CARFIGHT_UI_ASSET_MODE = $RunMode

# [v1.0.2] Apply는 FontFace PostEditChange가 요구하는 SlateApplication까지 준비되는 Full Editor Python 경로를 사용합니다.
if ($RunMode -eq 'apply')
{
    # [v1.0.2] Epic 공식 -ExecutePythonScript Full Editor 경로에 전달할 인수 목록입니다.
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
    # [v1.0.2] Probe·DryRun·Readback은 에셋 Import UI 수명이 필요 없으므로 빠른 PythonScript Commandlet을 유지합니다.
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

Write-Host '[CarFight] D1-09B UI Assetization Tool'
Write-Host ("Mode: {0}" -f $RunMode)

Push-Location $UnrealWorkingDirectory
try
{
    & $EditorCommand @EditorArguments

    # [v1.0.0] UnrealEditor-Cmd 프로세스가 반환한 실제 종료 코드입니다.
    $EditorExitCode = $LASTEXITCODE
}
finally
{
    Pop-Location

    if ($null -eq $PreviousRunMode)
    {
        Remove-Item Env:CARFIGHT_UI_ASSET_MODE -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_UI_ASSET_MODE = $PreviousRunMode
    }
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf))
{
    throw "UI Assetization report was not generated. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

# [v1.0.0] Unreal Python이 UTF-8로 기록한 전체 D1-09B 보고서 텍스트입니다.
$ReportText = [System.IO.File]::ReadAllText(
    $ReportPath,
    [System.Text.UTF8Encoding]::new($false))

# [v1.0.0] 실행 성공 여부와 실제 실행 모드를 확인할 구조화 보고서 객체입니다.
$Report = $ReportText | ConvertFrom-Json

if (-not $Report.success)
{
    throw "UI Assetization report returned success=false. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

if ($Report.mode -ne $RunMode)
{
    throw "UI Assetization report mode mismatch. Expected=$RunMode Actual=$($Report.mode)"
}

# [v1.0.1] Python 소유 보고서가 완전한 성공 사후조건을 증명한 뒤의 Editor 플러그인 종료 오류는 별도 경고로 기록합니다.
if ($EditorExitCode -ne 0)
{
    Write-Warning "UnrealEditor-Cmd exited with code $EditorExitCode after a successful UI Assetization report. Review the Editor log for unrelated plugin errors."
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host "EDITOR_EXIT_CODE=$EditorExitCode"
Write-Host '[CarFight] D1-09B UI Assetization Tool completed successfully.'
exit 0
