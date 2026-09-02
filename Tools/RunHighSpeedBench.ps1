# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunHighSpeedBench.ps1
# Version: v1.1.0
# Date: 2026-09-02
# Description: CF-FQ-040 ESH-04 dedicated M_VehicleBenchmark high-speed authority runner입니다.
# Changelog:
# - v1.1.0: ExpectedTargetDefinitionHash가 제공되면 benchmark 시작 전 CFVehicleHashCheck commandlet으로 current saved VehicleData semantic hash를 공용 Snapshot authority와 exact 비교하고 mismatch를 fail-closed. 결과 envelope에 실제 observed hash와 verification 여부를 기록.
# - v1.0.8: 0→100/150/200 km/h first-reach time telemetry parse/output을 추가해 ChangeUpRPM A/B/C acceleration 비교를 지원.
# - v1.0.7: Windows PowerShell 5.1 호환을 위해 ChangeUpRPMOverride finite 검사를 Double.IsNaN/IsInfinity 조합으로 교정.
# - v1.0.6: optional transient ChangeUpRPMOverride 전달과 EffectiveChangeUp/Down/AutomaticGears/ChangeUpOverride telemetry parse를 추가.
# - v1.0.5: v1.15.0 production no-fitting Legacy ConfiguredMassKg / ActualMassKg telemetry parse를 추가.
# - v1.0.4: v1.14.0 YawRateLock / MaxSuppressedYawRateDegPerSec longitudinal-fixture telemetry parse를 추가.
# - v1.0.3: v1.13.0 Floor-centerline derived start의 CenterlineShiftM authority telemetry parse를 추가.
# - v1.0.2: v1.12.0 bounded heading-hold의 MaxSteeringInput intervention telemetry parse를 추가.
# - v1.0.1: v1.11.1 benchmark summary의 MaxHeadingDeg authority telemetry parse를 추가.
# - v1.0.0: fixed-60Hz HighSpeedBenchmark 실행, authority summary/gear telemetry exact parse, bounded JSON evidence 출력을 추가.
# Migration:
# - Product Asset/Map/Config를 수정하거나 저장하지 않습니다.
# - ExpectedTargetDefinitionHash가 비어 있지 않으면 read-only preflight에서 current saved VehicleData DefinitionHash와 실제 비교하며, mismatch 시 high-speed benchmark를 시작하지 않습니다.
# - ExpectedTargetDefinitionHash가 비어 있는 legacy standalone 호출은 hash verification 없이 기존 benchmark 동작을 유지합니다.

[CmdletBinding()]
param(
    # 계측할 saved UCFVehicleData exact object path입니다.
    [Parameter(Mandatory = $true)]
    [string]$VehicleDataPath,

    # 사람이 결과를 식별할 bounded label입니다.
    [string]$Label = 'HighSpeedVehicle',

    # 실행 직전 current saved Target과 exact 일치해야 하는 semantic DefinitionHash입니다. 비어 있으면 legacy no-verification 동작입니다.
    [string]$ExpectedTargetDefinitionHash = '',

    # persisted Asset을 저장 변경하지 않고 transient duplicate에만 적용할 optional ChangeUpRPM입니다. 0이면 baseline입니다.
    [double]$ChangeUpRPMOverride = 0.0
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 현재 CarFight 저장소 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path

# CarFight 공식 Unreal project file입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# CarFight 공식 UE 5.8 Source Build Editor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# ESH-04 전용 Automation log입니다.
$AutomationLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFVehicleHighSpeedBench.log'

# expected TargetHash가 제공된 경우 benchmark 전에 current saved VehicleData를 검증하는 read-only preflight log입니다.
$HashPreflightLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFVehicleHighSpeedHashPreflight.log'

# ESH-04 machine-readable result JSON입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\VehicleHighSpeedBenchmarkResult.json'

# 결과 JSON 상위 폴더입니다.
$ResultDirectory = Split-Path -Parent $ResultJsonPath

# 실행할 exact ESH-04 Automation test path입니다.
$TestFilter = 'CarFight.VehicleBuilder.CF_FQ_040.ESH_04.HighSpeedBenchmark'

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    throw "CarFight uproject를 찾을 수 없습니다: $ProjectFile"
}
if (-not (Test-Path -LiteralPath $EditorExecutable -PathType Leaf)) {
    throw "CarFight 공식 UnrealEditor.exe를 찾을 수 없습니다: $EditorExecutable"
}
if ([string]::IsNullOrWhiteSpace($VehicleDataPath)) {
    throw 'VehicleDataPath는 비어 있을 수 없습니다.'
}
if ($Label.Length -gt 64) {
    throw 'Label은 64자를 넘을 수 없습니다.'
}
if ([double]::IsNaN($ChangeUpRPMOverride) -or [double]::IsInfinity($ChangeUpRPMOverride) -or $ChangeUpRPMOverride -lt 0.0) {
    throw 'ChangeUpRPMOverride는 finite 0 이상 값이어야 합니다.'
}
if (-not [string]::IsNullOrWhiteSpace($ExpectedTargetDefinitionHash) -and $ExpectedTargetDefinitionHash -notmatch '^[0-9a-fA-F]{32}$') {
    throw 'ExpectedTargetDefinitionHash는 비어 있거나 32자리 hexadecimal DefinitionHash여야 합니다.'
}

# caller가 expected Target identity를 제공한 경우 actual saved VehicleData를 공용 Definition Snapshot hash authority로 선검증합니다.
$TargetHashVerified = $false
$ObservedTargetDefinitionHash = ''
if (-not [string]::IsNullOrWhiteSpace($ExpectedTargetDefinitionHash)) {
    if (Test-Path -LiteralPath $HashPreflightLogPath -PathType Leaf) {
        Remove-Item -LiteralPath $HashPreflightLogPath -Force
    }

    # Product Asset을 저장 변경하지 않는 Editor-only semantic hash preflight commandlet 인자입니다.
    $HashPreflightArguments = @(
        $ProjectFile,
        '-run=CFVehicleHashCheck',
        "-CFVehicleData=$VehicleDataPath",
        "-CFExpectedDefinitionHash=$ExpectedTargetDefinitionHash",
        '-unattended',
        '-nop4',
        '-nosplash',
        '-nullrhi',
        "-abslog=$HashPreflightLogPath",
        '-stdout',
        '-FullStdOutLogOutput'
    )

    $HashPreflightProcess = Start-Process -FilePath $EditorExecutable -ArgumentList $HashPreflightArguments -WorkingDirectory $RepositoryRoot -Wait -PassThru -NoNewWindow
    $HashPreflightExitCode = $HashPreflightProcess.ExitCode
    if (-not (Test-Path -LiteralPath $HashPreflightLogPath -PathType Leaf)) {
        throw "Target DefinitionHash preflight log가 생성되지 않았습니다. ExitCode=$HashPreflightExitCode"
    }

    $HashPreflightLogText = [System.IO.File]::ReadAllText(
        $HashPreflightLogPath,
        [System.Text.UTF8Encoding]::new($false))
    $HashPassMatches = [regex]::Matches($HashPreflightLogText, 'CF_VEHICLE_HASH_CHECK_PASS path=.*? definition_hash=(?<hash>[0-9a-fA-F]{32}) field_count=[0-9]+')
    if ($HashPreflightExitCode -ne 0 -or $HashPassMatches.Count -ne 1) {
        $HashDiagnosticLines = @([regex]::Split($HashPreflightLogText, '\r?\n') | Where-Object {
            $_ -match 'CF_VEHICLE_HASH_CHECK_|Error:'
        } | Select-Object -Last 80)
        $HashDiagnosticLines | ForEach-Object { Write-Output $_ }
        throw "Target DefinitionHash preflight가 실패했습니다. ExitCode=$HashPreflightExitCode PassCount=$($HashPassMatches.Count)"
    }

    $ObservedTargetDefinitionHash = $HashPassMatches[0].Groups['hash'].Value.ToLowerInvariant()
    $TargetHashVerified = $ObservedTargetDefinitionHash -eq $ExpectedTargetDefinitionHash.ToLowerInvariant()
    if (-not $TargetHashVerified) {
        throw "Target DefinitionHash preflight 결과가 expected와 일치하지 않습니다. Expected=$ExpectedTargetDefinitionHash Observed=$ObservedTargetDefinitionHash"
    }
    Write-Output "TARGET_HASH_VERIFIED=$TargetHashVerified"
    Write-Output "OBSERVED_TARGET_DEFINITION_HASH=$ObservedTargetDefinitionHash"
}

[System.IO.Directory]::CreateDirectory($ResultDirectory) | Out-Null
if (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf) {
    Remove-Item -LiteralPath $AutomationLogPath -Force
}
if (Test-Path -LiteralPath $ResultJsonPath -PathType Leaf) {
    Remove-Item -LiteralPath $ResultJsonPath -Force
}

# Automation Controller exact command입니다.
$AutomationCommand = "Automation RunTests $TestFilter"

# saved VehicleData command-line argument입니다.
$VehicleDataArgument = "-CFHighSpeedVehicleData=$VehicleDataPath"

# human-readable label command-line argument입니다.
$LabelArgument = "-CFHighSpeedLabel=$Label"

# fixed-60Hz ESH-04 fresh PIE process arguments입니다.
$EditorArguments = @(
    $ProjectFile,
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI',
    '-UseFixedTimeStep',
    '-FPS=60',
    $VehicleDataArgument,
    $LabelArgument,
    ('-ExecCmds="{0}"' -f $AutomationCommand),
    '-TestExit="Automation Test Queue Empty"',
    "-abslog=$AutomationLogPath",
    '-stdout',
    '-FullStdOutLogOutput'
)

if ($ChangeUpRPMOverride -gt 0.0) {
    # invariant-culture transient ChangeUpRPM command-line argument입니다.
    $ChangeUpRpmOverrideArgument = '-CFHighSpeedChangeUpRPMOverride=' + $ChangeUpRPMOverride.ToString('0.###', [System.Globalization.CultureInfo]::InvariantCulture)
    $EditorArguments += $ChangeUpRpmOverrideArgument
}

# actual UnrealEditor benchmark process입니다.
$EditorProcess = Start-Process -FilePath $EditorExecutable -ArgumentList $EditorArguments -WorkingDirectory $RepositoryRoot -PassThru
$EditorProcess.WaitForExit()

# exact UnrealEditor terminal exit code입니다.
$EngineExitCode = $EditorProcess.ExitCode

if (-not (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf)) {
    throw "ESH-04 high-speed benchmark 로그가 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# UTF-8로 읽은 전체 Automation log입니다.
$AutomationLogText = [System.IO.File]::ReadAllText(
    $AutomationLogPath,
    [System.Text.UTF8Encoding]::new($false))

# exact Automation success marker regex입니다.
$SuccessRegex = [regex]('Test Completed\. Result=\{Success\}.*?Path=\{' + [regex]::Escape($TestFilter) + '\}')

# exact Automation failure marker regex입니다.
$FailureRegex = [regex]('Test Completed\. Result=\{(?:Fail|Failed)\}.*?Path=\{' + [regex]::Escape($TestFilter) + '\}')

# ESH-04 authority summary line regex입니다.
$SummaryRegex = [regex]('ESH04_HIGH_SPEED_SUMMARY \| Label=(?<label>[^|]+?) \| VehicleData=(?<vehicle>[^|]+?) \| DurationSec=(?<duration>[-0-9.]+) \| ConfiguredMassKg=(?<configuredmass>[-0-9.]+) \| ActualMassKg=(?<actualmass>[-0-9.]+) \| EffectiveChangeUpRPM=(?<changeup>[-0-9.]+) \| EffectiveChangeDownRPM=(?<changedown>[-0-9.]+) \| AutomaticGears=(?<automatic>True|False) \| ChangeUpOverride=(?<override>True|False) \| TimeTo100KmhSec=(?<t100>[-0-9.]+) \| TimeTo150KmhSec=(?<t150>[-0-9.]+) \| TimeTo200KmhSec=(?<t200>[-0-9.]+) \| ForwardAvailableM=(?<available>[-0-9.]+) \| CenterlineShiftM=(?<centershift>[-0-9.]+) \| TravelM=(?<travel>[-0-9.]+) \| MaxLateralM=(?<lateral>[-0-9.]+) \| MaxHeadingDeg=(?<heading>[-0-9.]+) \| MaxSteeringInput=(?<steering>[-0-9.]+) \| YawRateLock=(?<yawlock>True|False) \| MaxSuppressedYawRateDegPerSec=(?<yawrate>[-0-9.]+) \| PeakSpeedKmh=(?<peak>[-0-9.]+) \| FinalSpeedKmh=(?<finalspeed>[-0-9.]+) \| PeakRPM=(?<rpm>[-0-9.]+) \| FinalGear=(?<finalgear>-?[0-9]+) \| TopSpeedStable=(?<stable>True|False) \| LastStableWindowGainKmh=(?<windowgain>[-0-9.]+) \| RoadEndReached=(?<roadend>True|False) \| CollisionObstacles=(?<obstacles>[0-9]+)')

# ESH-04 one-gear telemetry regex입니다.
$GearRegex = [regex]('ESH04_HIGH_SPEED_GEAR \| Gear=(?<gear>[0-9]+) \| EntryTimeSec=(?<entrytime>[-0-9.]+) \| EntrySpeedKmh=(?<entryspeed>[-0-9.]+) \| EntryRPM=(?<entryrpm>[-0-9.]+) \| DwellSec=(?<dwell>[-0-9.]+) \| PeakSpeedKmh=(?<peakspeed>[-0-9.]+) \| SpeedGainKmh=(?<speedgain>[-0-9.]+) \| PeakRPM=(?<peakrpm>[-0-9.]+)')

# exact Automation success matches입니다.
$SuccessMatches = $SuccessRegex.Matches($AutomationLogText)

# exact Automation failure matches입니다.
$FailureMatches = $FailureRegex.Matches($AutomationLogText)

# exact ESH-04 summary matches입니다.
$SummaryMatches = $SummaryRegex.Matches($AutomationLogText)

# all observed gear telemetry matches입니다.
$GearMatches = $GearRegex.Matches($AutomationLogText)

# exact one ESH-04 summary를 요구하는 process PASS 여부입니다.
$AutomationPassed = ($EngineExitCode -eq 0) -and
    ($FailureMatches.Count -eq 0) -and
    ($SuccessMatches.Count -gt 0) -and
    ($SummaryMatches.Count -eq 1)

# parsed authority summary object입니다.
$Summary = $null
if ($SummaryMatches.Count -eq 1) {
    # exact one summary regex match입니다.
    $Match = $SummaryMatches[0]
    $Summary = [ordered]@{
        label = $Match.Groups['label'].Value.Trim()
        vehicle_data_path = $Match.Groups['vehicle'].Value.Trim()
        duration_sec = [double]::Parse($Match.Groups['duration'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        configured_mass_kg = [double]::Parse($Match.Groups['configuredmass'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        actual_mass_kg = [double]::Parse($Match.Groups['actualmass'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        effective_change_up_rpm = [double]::Parse($Match.Groups['changeup'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        effective_change_down_rpm = [double]::Parse($Match.Groups['changedown'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        automatic_gears = ($Match.Groups['automatic'].Value -eq 'True')
        change_up_override = ($Match.Groups['override'].Value -eq 'True')
        time_to_100_kmh_sec = [double]::Parse($Match.Groups['t100'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        time_to_150_kmh_sec = [double]::Parse($Match.Groups['t150'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        time_to_200_kmh_sec = [double]::Parse($Match.Groups['t200'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        forward_available_m = [double]::Parse($Match.Groups['available'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        centerline_shift_m = [double]::Parse($Match.Groups['centershift'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        travelled_forward_m = [double]::Parse($Match.Groups['travel'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        max_lateral_m = [double]::Parse($Match.Groups['lateral'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        max_heading_deg = [double]::Parse($Match.Groups['heading'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        max_steering_input = [double]::Parse($Match.Groups['steering'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        yaw_rate_lock = ($Match.Groups['yawlock'].Value -eq 'True')
        max_suppressed_yaw_rate_deg_per_sec = [double]::Parse($Match.Groups['yawrate'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        peak_speed_kmh = [double]::Parse($Match.Groups['peak'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        final_speed_kmh = [double]::Parse($Match.Groups['finalspeed'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        peak_engine_rpm = [double]::Parse($Match.Groups['rpm'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        final_gear = [int]::Parse($Match.Groups['finalgear'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        top_speed_stable = ($Match.Groups['stable'].Value -eq 'True')
        last_stability_window_gain_kmh = [double]::Parse($Match.Groups['windowgain'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        road_end_reached = ($Match.Groups['roadend'].Value -eq 'True')
        collision_obstacles = [int]::Parse($Match.Groups['obstacles'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
    }
}

# parsed per-gear telemetry list입니다.
$Gears = @()
foreach ($GearMatch in $GearMatches) {
    # current parsed gear telemetry row입니다.
    $GearRow = [ordered]@{
        gear = [int]::Parse($GearMatch.Groups['gear'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        entry_time_sec = [double]::Parse($GearMatch.Groups['entrytime'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        entry_speed_kmh = [double]::Parse($GearMatch.Groups['entryspeed'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        entry_engine_rpm = [double]::Parse($GearMatch.Groups['entryrpm'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        dwell_sec = [double]::Parse($GearMatch.Groups['dwell'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        peak_speed_kmh = [double]::Parse($GearMatch.Groups['peakspeed'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        speed_gain_kmh = [double]::Parse($GearMatch.Groups['speedgain'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        peak_engine_rpm = [double]::Parse($GearMatch.Groups['peakrpm'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
    }
    $Gears += $GearRow
}

# process terminal UTC timestamp입니다.
$CompletedUtc = [DateTime]::UtcNow.ToString('o', [System.Globalization.CultureInfo]::InvariantCulture)

# ESH-04 bounded machine-readable evidence envelope입니다.
$Result = [ordered]@{
    schema_version = 'carfight_vehicle_high_speed_benchmark_v1'
    status = if ($AutomationPassed) { 'success' } else { 'failed' }
    test_filter = $TestFilter
    expected_target_definition_hash = $ExpectedTargetDefinitionHash
    observed_target_definition_hash = $ObservedTargetDefinitionHash
    target_hash_verified = $TargetHashVerified
    completed_utc = $CompletedUtc
    engine_exit_code = $EngineExitCode
    success_marker_count = $SuccessMatches.Count
    failure_marker_count = $FailureMatches.Count
    summary_count = $SummaryMatches.Count
    gear_count = $GearMatches.Count
    product_mutation_asserted = $false
    reference_threshold_asserted = $false
    summary = $Summary
    gears = $Gears
    log_path = $AutomationLogPath
}

# UTF-8 JSON evidence입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 8
[System.IO.File]::WriteAllText(
    $ResultJsonPath,
    $ResultJson,
    [System.Text.UTF8Encoding]::new($false))

Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "EXPECTED_TARGET_DEFINITION_HASH=$ExpectedTargetDefinitionHash"
Write-Output "OBSERVED_TARGET_DEFINITION_HASH=$ObservedTargetDefinitionHash"
Write-Output "TARGET_HASH_VERIFIED=$TargetHashVerified"
Write-Output "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Output "SUMMARY_COUNT=$($SummaryMatches.Count)"
Write-Output "GEAR_COUNT=$($GearMatches.Count)"

if ($null -ne $Summary) {
    Write-Output ("ESH04_RESULT=Peak={0:F3}kmh|Final={1:F3}kmh|Gear={2}|ShiftUp={3:F0}rpm|T100={4:F3}s|T150={5:F3}s|T200={6:F3}s|Mass={7:F1}/{8:F1}kg|Travel={9:F1}m|Heading={10:F2}deg|SteerMax={11:F4}|YawLock={12}|Stable={13}|RoadEnd={14}" -f
        $Summary.peak_speed_kmh,
        $Summary.final_speed_kmh,
        $Summary.final_gear,
        $Summary.effective_change_up_rpm,
        $Summary.time_to_100_kmh_sec,
        $Summary.time_to_150_kmh_sec,
        $Summary.time_to_200_kmh_sec,
        $Summary.configured_mass_kg,
        $Summary.actual_mass_kg,
        $Summary.travelled_forward_m,
        $Summary.max_heading_deg,
        $Summary.max_steering_input,
        $Summary.yaw_rate_lock,
        $Summary.top_speed_stable,
        $Summary.road_end_reached)
}
foreach ($GearRow in $Gears) {
    Write-Output ("ESH04_GEAR=G{0}|Entry={1:F3}kmh@{2:F3}s|Dwell={3:F3}s|Gain={4:F3}kmh|Peak={5:F3}kmh" -f
        $GearRow.gear,
        $GearRow.entry_speed_kmh,
        $GearRow.entry_time_sec,
        $GearRow.dwell_sec,
        $GearRow.speed_gain_kmh,
        $GearRow.peak_speed_kmh)
}

if (-not $AutomationPassed) {
    # failure RCA에 필요한 bounded ESH-04/Automation log lines입니다.
    $DiagnosticLines = @([regex]::Split($AutomationLogText, '\r?\n') | Where-Object {
        $_ -match 'ESH-04|ESH04_|LogAutomation|Test Completed|Error:'
    } | Select-Object -Last 200)
    $DiagnosticLines | ForEach-Object { Write-Output $_ }
    exit 1
}

exit 0
