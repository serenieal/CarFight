# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.6.0
# Date: 2026-08-21
# Description: ApplyUIHUDProduction.py의 신규 Scaffold / Designer Layout 보존 Targeted Apply 경로를 공식 UE 5.8에서 실행합니다.
# Scope: 기존 Production Widget은 기본 Validate-only로 보존하며 -RadarOnly와 -ViewModeOnly만 명시적 additive Visual migration을 허용합니다.
# Changelog:
# - v1.6.0: -ViewModeOnly를 추가해 기존 WBP_CFInGameHUD 정확히 1개만 저장하는 view_mode_visual_apply를 연결. Root 7-child/ReticleLayer 기존 Slot Layout 재적용과 다른 Production Asset 저장 금지.
# - v1.5.0: -RadarOnly를 추가해 Radar Texture 7 + VisualData + RadarPanel 정확히 9개만 저장하는 radar_visual_apply를 연결. RadarPanel 기존 Slot Layout/Tree rebuild 금지.
# - v1.4.0: Apply/WeaponPanelOnly/ArmorMapOnly에서 기존 Widget Tree 재구축을 중단하고 Validate-only로 전환. RpmGaugeOnly는 Texture+Material+VisualData 3개만 저장하고 SpeedGauge는 Layout 보존 검증 전용으로 변경.

# - v1.3.0: -RpmGaugeOnly 스위치를 추가해 T_UI_RPMTrack + M_UI_RPMGauge + DA_CFHUDVisual_Default + WBP_CFSpeedGauge 정확히 4개 Asset만 변경하는 targeted apply mode를 연결.
# - v1.2.0: -ArmorMapOnly 스위치를 추가해 WBP_CFArmorSector 생성/재구축 + WBP_CFArmorBodyMap 재구축 정확히 2개 Asset만 변경하는 targeted apply mode를 연결.
# - v1.1.0: -WeaponPanelOnly 스위치를 추가해 전체 Production 재작성을 우회하고 exact WBP_CFWeaponPanel 단일 Asset만 변경하는 targeted apply mode를 연결.
# - v1.0.0: Production Root 1 + Panel 6 + Element 2 + Visual DataAsset 1의 Full Editor Wait와 UTF-8 Report 검증을 최초 추가.
# Migration:
# - 기본 실행은 Probe입니다.
# - -DryRun/-Readback은 읽기 전용입니다. -Apply는 기존 Widget을 Validate-only로 보호하고 누락돼 이번 실행에서 생성한 Widget만 최초 Scaffold/저장합니다.
# - -WeaponPanelOnly는 기존 WBP_CFWeaponPanel의 의미 구조만 검증하며 Designer Layout을 저장하거나 재구축하지 않습니다.
# - -ArmorMapOnly는 기존 ArmorSector/ArmorBodyMap을 Validate-only로 보호하고 ArmorSector가 누락된 경우에만 최초 Scaffold를 허용합니다.
# - -RpmGaugeOnly는 RPM Texture/Material/VisualData 3개만 저장하고 기존 SpeedGauge는 의미 구조 Validate-only로 유지합니다.
# - -RadarOnly는 SourceArt/UI/HUD/P2의 Radar PNG 7종을 import/reimport하고 VisualData + 기존 WBP_CFRadarPanel만 저장합니다. 기존 Radar Widget Slot Layout은 다시 쓰지 않습니다.
# - -ViewModeOnly는 기존 WBP_CFInGameHUD의 빈 ReticleLayer에 Vehicle Direction Track/Image 누락분만 추가하고 Root/Panel 기존 Slot Layout은 다시 쓰지 않습니다.



[CmdletBinding()]
param(
    # [v1.0.0] Production 필수 Python/C++ 타입과 읽기 전용 Dependency만 확인합니다.
    [Parameter(Mandatory = $false)]
    [switch]$Probe,

        # [v1.4.0] Production Target의 기존 Validate-preserve-layout / 누락 create-scaffold 판정을 검증합니다.

    [Parameter(Mandatory = $false)]
    [switch]$DryRun,

        # [v1.4.0] 누락 Production Asset의 최초 Scaffold를 허용하며 기존 Widget은 Validate-only로 유지합니다.

    [Parameter(Mandatory = $false)]
    [switch]$Apply,

            # [v1.0.0] 저장된 Production 10 Asset을 새 프로세스에서 변경 없이 검증합니다.
    [Parameter(Mandatory = $false)]
    [switch]$Readback,

            # [v1.4.0] 기존 WBP_CFWeaponPanel의 의미 구조를 Validate-only로 검증합니다.

    [Parameter(Mandatory = $false)]
    [switch]$WeaponPanelOnly,

            # [v1.4.0] 기존 ArmorSector/ArmorBodyMap을 Validate-only로 검증하고 누락 ArmorSector만 최초 Scaffold합니다.

    [Parameter(Mandatory = $false)]
    [switch]$ArmorMapOnly,

        # [v1.4.0] 동적 RPM Texture + Material + VisualData 3개만 저장하고 SpeedGauge Designer Tree는 Validate-only로 보호합니다.

        [Parameter(Mandatory = $false)]
    [switch]$RpmGaugeOnly,

        # [v1.5.0] Radar Texture 7 + VisualData + 기존 RadarPanel additive Visual migration만 수행합니다.
    [Parameter(Mandatory = $false)]
    [switch]$RadarOnly,

    # [v1.6.0] 기존 Production Root 한 개에 Vehicle Direction Track/Image additive migration만 수행합니다.
    [Parameter(Mandatory = $false)]
    [switch]$ViewModeOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# [v1.0.0] 현재 Runner가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# [v1.0.0] Tools 상위 CarFight 저장소 루트입니다.
$RepositoryRoot = Split-Path -Parent $ToolsDirectory

# [v1.0.0] 공식 UE 5.8 Engine/Project 경로를 검증할 공용 환경 스크립트입니다.
$EnvironmentGuard = Join-Path $ToolsDirectory 'CarFightEnv.bat'

# [v1.0.0] Production Unreal Python 제작/검증 도구 경로입니다.
$PythonScript = Join-Path $ToolsDirectory 'ApplyUIHUDProduction.py'

# [v1.0.0] Python 도구가 기록할 Production 구조화 결과 JSON 경로입니다.
$ReportPath = Join-Path $RepositoryRoot 'UE\Saved\UIHUDProduction\report.json'

# [v1.1.0] 동시에 여러 실행 모드를 선택하지 않았는지 확인할 선택 개수입니다.
$ModeCount = @(@($Probe, $DryRun, $Apply, $Readback, $WeaponPanelOnly, $ArmorMapOnly, $RpmGaugeOnly, $RadarOnly, $ViewModeOnly) | Where-Object { $_ }).Count
if ($ModeCount -gt 1)
{
        throw 'Choose only one of -Probe, -DryRun, -Apply, -Readback, -WeaponPanelOnly, -ArmorMapOnly, -RpmGaugeOnly, -RadarOnly or -ViewModeOnly.'
}

# [v1.1.0] 아무 스위치도 없으면 가장 안전한 Probe를 사용하고 WeaponPanel-only는 별도 targeted mutation 모드로 분리합니다.
$RunMode = if ($DryRun) { 'dry_run' } elseif ($Apply) { 'apply' } elseif ($Readback) { 'readback' } elseif ($WeaponPanelOnly) { 'weapon_panel_apply' } elseif ($ArmorMapOnly) { 'armor_map_apply' } elseif ($RpmGaugeOnly) { 'rpm_gauge_apply' } elseif ($RadarOnly) { 'radar_visual_apply' } elseif ($ViewModeOnly) { 'view_mode_visual_apply' } else { 'probe' }

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

# [v1.0.0] 실제 Production Widget Blueprint Apply에 사용할 공식 UnrealEditor.exe 경로입니다.
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

# [v1.0.0] 실행 뒤 원래 환경으로 복구할 이전 Production 실행 모드 값입니다.
$PreviousRunMode = $env:CARFIGHT_UI_HUD_PROD_MODE
$env:CARFIGHT_UI_HUD_PROD_MODE = $RunMode

if ($RunMode -eq 'apply' -or $RunMode -eq 'weapon_panel_apply' -or $RunMode -eq 'armor_map_apply' -or $RunMode -eq 'rpm_gauge_apply' -or $RunMode -eq 'radar_visual_apply' -or $RunMode -eq 'view_mode_visual_apply')
{
        # [v1.4.0] 신규 UMG Scaffold 또는 Material/Visual targeted mutation에 사용할 Full Editor 프로그램입니다.

    $EditorProgram = $EditorExecutable
    # [v1.0.0] Full Editor Production Python Apply 인수입니다.
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
    # [v1.0.0] 읽기 전용 Production 모드에 사용할 Commandlet 프로그램입니다.
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

Write-Host '[CarFight] Production UI HUD Tool'
Write-Host ("Mode: {0}" -f $RunMode)

Push-Location $UnrealWorkingDirectory
try
{
    if ($RunMode -eq 'apply' -or $RunMode -eq 'weapon_panel_apply' -or $RunMode -eq 'armor_map_apply' -or $RunMode -eq 'rpm_gauge_apply' -or $RunMode -eq 'radar_visual_apply' -or $RunMode -eq 'view_mode_visual_apply')
    {
        # [v1.3.0] Full Editor가 Production Python Apply와 종료를 모두 마칠 때까지 대기할 Process입니다.
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
        Remove-Item Env:CARFIGHT_UI_HUD_PROD_MODE -ErrorAction SilentlyContinue
    }
    else
    {
        $env:CARFIGHT_UI_HUD_PROD_MODE = $PreviousRunMode
    }
}

if (-not (Test-Path -LiteralPath $ReportPath -PathType Leaf))
{
    throw "Production UI HUD report was not generated. Editor exit code: $EditorExitCode, Report: $ReportPath"
}

# [v1.0.0] Unreal Python이 UTF-8로 기록한 전체 Production 보고서 텍스트입니다.
$ReportText = [System.IO.File]::ReadAllText($ReportPath, [System.Text.UTF8Encoding]::new($false))

# [v1.0.0] 실행 성공 여부와 실제 Mode를 검증할 구조화 보고서 객체입니다.
$Report = $ReportText | ConvertFrom-Json
if (-not $Report.success)
{
    throw "Production UI HUD report returned success=false. Editor exit code: $EditorExitCode, Report: $ReportPath"
}
if ($Report.mode -ne $RunMode)
{
    throw "Production UI HUD report mode mismatch. Expected=$RunMode Actual=$($Report.mode)"
}
if ($EditorExitCode -ne 0)
{
    throw "UnrealEditor exited with code $EditorExitCode. Report: $ReportPath"
}

Write-Host "RESULT_JSON_PATH=$ReportPath"
Write-Host "EDITOR_EXIT_CODE=$EditorExitCode"
Write-Host '[CarFight] D1-11 Production UI HUD Tool completed successfully.'
exit 0
