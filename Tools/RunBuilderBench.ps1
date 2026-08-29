# CarFight CF-FQ-040 VB-P0-08 Technical Driving Benchmark runner.
# Version: v1.3.0
# Date: 2026-08-27
# Changelog:
# - v1.3.0: VB-P0-09 Step 8 Guided Shell이 current saved Target과 결과를 exact binding하도록 optional RunId/ExpectedTargetDefinitionHash/completed UTC provenance를 result envelope에 additive 추가.
# - v1.2.0: UE 5.8 Source-confirmed -UseFixedTimeStep -FPS=60을 benchmark process에 적용해 Chaos vehicle 계측을 고정 60Hz simulation step으로 실행하고 반복 편차를 줄임.
# - v1.1.0: Start-Process -Wait의 descendant-process 대기를 제거하고 exact UnrealEditor Process 객체 WaitForExit()만 사용해 TestExit 이후 wrapper가 terminal로 회수되도록 교정.
# - v1.0.0: saved VehicleData + optional FittingData를 fresh PIE Builder benchmark Automation에 전달하고 machine-readable JSON으로 추출.
# Migration:
# - Product Asset/Map/Config를 저장하지 않습니다.
# - Reference 성능과의 PASS/FAIL threshold를 이 runner가 임의 생성하지 않습니다.

[CmdletBinding()]
param(
    # 계측할 saved UCFVehicleData exact object path입니다.
    [Parameter(Mandatory = $true)]
    [string]$VehicleDataPath,

    # 장비 포함 질량을 계측할 optional UCFVehicleFittingData exact object path입니다.
    [string]$FittingDataPath = '',

    # 사람이 결과를 식별할 bounded label입니다.
    [string]$Label = 'BuilderVehicle',

    # Guided Step 8이 한 benchmark launch와 USER acceptance를 exact binding할 optional run identity입니다.
    [string]$RunId = '',

    # Guided Step 8 launch 당시 current Target semantic DefinitionHash입니다. 기존 standalone P0-08 호출에서는 비어 있을 수 있습니다.
    [string]$ExpectedTargetDefinitionHash = ''
)

# PowerShell 오류를 즉시 실패로 처리합니다.
$ErrorActionPreference = 'Stop'

# 현재 CarFight 저장소 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build Editor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'
# 이번 benchmark 전용 Unreal log입니다.
$AutomationLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFVehicleBuilderBench.log'
# Browser/AI가 회수할 machine-readable 결과 JSON입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\VehicleBuilderBenchmarkResult.json'
# 결과 JSON 상위 폴더입니다.
$ResultDirectory = Split-Path -Parent $ResultJsonPath
# 실행할 exact Automation test path입니다.
$TestFilter = 'CarFight.VehicleBuilder.CF_FQ_040.VB_P0_08.TechnicalDrivingBenchmark'

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
if ([string]::IsNullOrWhiteSpace($RunId)) {
    # Standalone P0-08 caller도 결과 identity를 갖도록 fresh GUID를 생성합니다.
    $RunId = [guid]::NewGuid().ToString('D')
}
# RunId가 정상 GUID 형식인지 검증할 parsed 값입니다.
$ParsedRunId = [guid]::Empty
if (-not [guid]::TryParse($RunId, [ref]$ParsedRunId)) {
    throw 'RunId는 GUID 형식이어야 합니다.'
}

[System.IO.Directory]::CreateDirectory($ResultDirectory) | Out-Null
if (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf) {
    Remove-Item -LiteralPath $AutomationLogPath -Force
}
if (Test-Path -LiteralPath $ResultJsonPath -PathType Leaf) {
    Remove-Item -LiteralPath $ResultJsonPath -Force
}

# exact Automation Controller command입니다.
$AutomationCommand = "Automation RunTests $TestFilter"
# VehicleData command-line argument입니다.
$VehicleDataArgument = "-CFBuilderBenchmarkVehicleData=$VehicleDataPath"
# Label command-line argument입니다.
$LabelArgument = "-CFBuilderBenchmarkLabel=$Label"
# optional Fitting command-line argument입니다.
$FittingArgument = if ([string]::IsNullOrWhiteSpace($FittingDataPath)) { $null } else { "-CFBuilderBenchmarkFittingData=$FittingDataPath" }

# 공식 non-interactive Automation 실행 인자입니다.
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
if ($null -ne $FittingArgument) {
    $EditorArguments += $FittingArgument
}

# actual UnrealEditor benchmark exact PID를 시작하며 Process Tree 전체가 아니라 이 Editor PID만 기다립니다.
$EditorProcess = Start-Process -FilePath $EditorExecutable -ArgumentList $EditorArguments -WorkingDirectory $RepositoryRoot -PassThru
$EditorProcess.WaitForExit()
# exact UnrealEditor PID의 terminal exit code입니다.
$EngineExitCode = $EditorProcess.ExitCode

if (-not (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf)) {
    throw "Builder Benchmark 로그가 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# UTF-8로 읽은 전체 Automation log입니다.
$AutomationLogText = [System.IO.File]::ReadAllText($AutomationLogPath, [System.Text.UTF8Encoding]::new($false))
# exact success marker regex입니다.
$SuccessRegex = [regex]('Test Completed\. Result=\{Success\}.*?Path=\{' + [regex]::Escape($TestFilter) + '\}')
# exact failure marker regex입니다.
$FailureRegex = [regex]('Test Completed\. Result=\{(?:Fail|Failed)\}.*?Path=\{' + [regex]::Escape($TestFilter) + '\}')
# VB-P0-08 machine-readable metric line regex입니다.
$MetricRegex = [regex]('VB-P0-08 DrivingMetric \| Label=(?<label>[^|]+?) \| VehicleData=(?<vehicle>[^|]+?) \| FittingData=(?<fitting>[^|]+?) \| ConfiguredMassKg=(?<configured>[-0-9.]+) \| ActualMassKg=(?<actual>[-0-9.]+) \| Accel0To50Sec=(?<a50>[-0-9.]+) \| Accel0To100Sec=(?<a100>[-0-9.]+) \| PeakSpeedKmh=(?<peak>[-0-9.]+) \| TopSpeedStable=(?<stable>True|False) \| PeakRPM=(?<rpm>[-0-9.]+) \| PeakGear=(?<gear>-?[0-9]+) \| Braking100Available=(?<brakeavailable>True|False) \| BrakeStartKmh=(?<brakestart>[-0-9.]+) \| Brake100ToIdleSec=(?<braketime>[-0-9.]+) \| Brake100ToIdleDistanceM=(?<brakedistance>[-0-9.]+) \| SteadyYawDeg=(?<yaw>[-0-9.]+) \| TurningRadiusM=(?<radius>[-0-9.]+) \| TurningAverageSpeedKmh=(?<turnspeed>[-0-9.]+)')
# success marker matches입니다.
$SuccessMatches = $SuccessRegex.Matches($AutomationLogText)
# failure marker matches입니다.
$FailureMatches = $FailureRegex.Matches($AutomationLogText)
# benchmark metric matches입니다.
$MetricMatches = $MetricRegex.Matches($AutomationLogText)

# exact one metric을 요구하는 benchmark pass 여부입니다.
$AutomationPassed = ($EngineExitCode -eq 0) -and ($FailureMatches.Count -eq 0) -and ($SuccessMatches.Count -gt 0) -and ($MetricMatches.Count -eq 1)

# 결과 JSON에 넣을 metric object입니다.
$Metric = $null
if ($MetricMatches.Count -eq 1) {
    # exact benchmark metric regex match입니다.
    $Match = $MetricMatches[0]
    $Metric = [ordered]@{
        label = $Match.Groups['label'].Value.Trim()
        vehicle_data_path = $Match.Groups['vehicle'].Value.Trim()
        fitting_data_path = $Match.Groups['fitting'].Value.Trim()
        configured_mass_kg = [double]::Parse($Match.Groups['configured'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        actual_mass_kg = [double]::Parse($Match.Groups['actual'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        acceleration_0_to_50_sec = [double]::Parse($Match.Groups['a50'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        acceleration_0_to_100_sec = [double]::Parse($Match.Groups['a100'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        peak_speed_kmh = [double]::Parse($Match.Groups['peak'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        top_speed_stable = ($Match.Groups['stable'].Value -eq 'True')
        peak_engine_rpm = [double]::Parse($Match.Groups['rpm'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        peak_speed_gear = [int]::Parse($Match.Groups['gear'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        braking_100_available = ($Match.Groups['brakeavailable'].Value -eq 'True')
        braking_start_kmh = [double]::Parse($Match.Groups['brakestart'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        braking_100_to_idle_sec = [double]::Parse($Match.Groups['braketime'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        braking_100_to_idle_distance_m = [double]::Parse($Match.Groups['brakedistance'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        steady_yaw_deg = [double]::Parse($Match.Groups['yaw'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        effective_turning_radius_m = [double]::Parse($Match.Groups['radius'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        turning_average_speed_kmh = [double]::Parse($Match.Groups['turnspeed'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
    }
}

# Benchmark process terminal timestamp를 UTC ISO 8601로 기록합니다.
$CompletedUtc = [DateTime]::UtcNow.ToString('o', [System.Globalization.CultureInfo]::InvariantCulture)

# Browser evidence result envelope입니다.
$Result = [ordered]@{
    schema_version = 'carfight_vehicle_builder_benchmark_v1'
    status = if ($AutomationPassed) { 'success' } else { 'failed' }
    run_id = $ParsedRunId.ToString('D')
    expected_target_definition_hash = $ExpectedTargetDefinitionHash
    completed_utc = $CompletedUtc
    test_filter = $TestFilter
    engine_exit_code = $EngineExitCode
    success_marker_count = $SuccessMatches.Count
    failure_marker_count = $FailureMatches.Count
    metric_count = $MetricMatches.Count
    reference_threshold_asserted = $false
    user_driving_feel_asserted = $false
    metric = $Metric
    log_path = $AutomationLogPath
}

# UTF-8 JSON 문자열입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 8
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "RUN_ID=$($ParsedRunId.ToString('D'))"
Write-Output "EXPECTED_TARGET_DEFINITION_HASH=$ExpectedTargetDefinitionHash"
Write-Output "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Output "METRIC_COUNT=$($MetricMatches.Count)"
if ($null -ne $Metric) {
    # 사람이 빠르게 읽을 bounded one-line summary입니다.
    $Summary = "BENCHMARK={0}|Mass={1:F3}|0-50={2:F6}s|0-100={3:F6}s|Peak={4:F6}kmh|Brake={5:F6}s/{6:F6}m|Yaw={7:F6}deg|Radius={8:F6}m" -f $Metric.label, $Metric.configured_mass_kg, $Metric.acceleration_0_to_50_sec, $Metric.acceleration_0_to_100_sec, $Metric.peak_speed_kmh, $Metric.braking_100_to_idle_sec, $Metric.braking_100_to_idle_distance_m, $Metric.steady_yaw_deg, $Metric.effective_turning_radius_m
    Write-Output $Summary
}

if (-not $AutomationPassed) {
    # 실패 RCA에 필요한 bounded log lines입니다.
    $DiagnosticLines = @([regex]::Split($AutomationLogText, '\r?\n') | Where-Object {
        $_ -match 'VB-P0-08|LogAutomation|Test Completed|Error:'
    } | Select-Object -Last 160)
    $DiagnosticLines | ForEach-Object { Write-Output $_ }
    exit 1
}

exit 0
