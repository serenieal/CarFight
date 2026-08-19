# CarFight CF-FQ-034 FIT-P0-07C Quantitative Mobility 작업 전용 runner.
# Version: v1.0.0
# Date: 2026-08-17
# Changelog:
# - v1.0.0: QuantitativeMobility Automation을 실행하고 세 MobilityMetric 로그를 machine-readable JSON으로 추출합니다.
# Migration:
# - 이 스크립트는 FIT-P0-07C 작업 전용 실행 수단이며 별도 공용화 결정 전 범용 Automation Tool로 해석하지 않습니다.
# - Product Source/Config/Asset을 저장하지 않으며 결과는 UE/Saved/CarFight/MobilityMeasureResult.json에만 기록합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 현재 저장소의 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build Editor 실행 파일입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'
# 이번 FIT-P0-07C Automation의 전용 Unreal 로그입니다.
$AutomationLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFMobilityMeasure.log'
# Browser가 bounded evidence로 회수할 machine-readable 결과 파일입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\MobilityMeasureResult.json'
# 결과 JSON의 상위 디렉터리입니다.
$ResultDirectory = Split-Path -Parent $ResultJsonPath
# 실행할 정확한 FIT-P0-07C Automation 테스트 경로입니다.
$TestFilter = 'CarFight.Fitting.FIT_P0_07C.QuantitativeMobility'

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    throw "CarFight uproject를 찾을 수 없습니다: $ProjectFile"
}
if (-not (Test-Path -LiteralPath $EditorExecutable -PathType Leaf)) {
    throw "CarFight 공식 UnrealEditor.exe를 찾을 수 없습니다: $EditorExecutable"
}

[System.IO.Directory]::CreateDirectory($ResultDirectory) | Out-Null
if (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf) {
    Remove-Item -LiteralPath $AutomationLogPath -Force
}
if (Test-Path -LiteralPath $ResultJsonPath -PathType Leaf) {
    Remove-Item -LiteralPath $ResultJsonPath -Force
}

# Unreal Automation Controller에 전달할 명령 문자열입니다.
$AutomationCommand = "Automation RunTests $TestFilter"
# 공식 Editor 실행 파일에 전달할 bounded 비대화형 Automation 인자 목록입니다.
$EditorArguments = @(
    $ProjectFile,
    '-unattended',
    '-nop4',
    '-NoSplash',
    '-NullRHI',
    ('-ExecCmds="{0}"' -f $AutomationCommand),
    '-TestExit="Automation Test Queue Empty"',
    "-abslog=$AutomationLogPath",
    '-stdout',
    '-FullStdOutLogOutput'
)

# GUI Editor 프로세스를 실제 종료까지 기다리고 ExitCode를 회수할 프로세스 핸들입니다.
$EditorProcess = Start-Process -FilePath $EditorExecutable -ArgumentList $EditorArguments -WorkingDirectory $RepositoryRoot -Wait -PassThru
# UnrealEditor가 반환한 실제 프로세스 종료 코드입니다.
$EngineExitCode = $EditorProcess.ExitCode

if (-not (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf)) {
    throw "Mobility Automation 로그가 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# UE 로그를 Windows PowerShell 기본 인코딩 추측 없이 UTF-8로 읽은 전체 문자열입니다.
$AutomationLogText = [System.IO.File]::ReadAllText($AutomationLogPath, [System.Text.UTF8Encoding]::new($false))
# 정확한 테스트 성공 완료 marker를 찾는 정규식입니다.
$SuccessRegex = [regex]('Test Completed\. Result=\{Success\}.*?Path=\{' + [regex]::Escape($TestFilter) + '\}')
# 정확한 테스트 실패 완료 marker를 찾는 정규식입니다.
$FailureRegex = [regex]('Test Completed\. Result=\{(?:Fail|Failed)\}.*?Path=\{' + [regex]::Escape($TestFilter) + '\}')
# FIT-P0-07C가 기록한 한 Fixture의 정량 결과를 추출하는 정규식입니다.
$MetricRegex = [regex]('FIT-P0-07C MobilityMetric \| Fixture=(?<fixture>Light|Default|Heavy) \| ConfiguredMassKg=(?<configured>[-0-9.]+) \| ActualMassKg=(?<actual>[-0-9.]+) \| AccelReferenceKmh=(?<accelref>[-0-9.]+) \| AccelTimeSec=(?<acceltime>[-0-9.]+) \| AccelDistanceM=(?<acceldistance>[-0-9.]+) \| BrakeStartKmh=(?<brakestart>[-0-9.]+) \| BrakeStopKmh=(?<brakestop>[-0-9.]+) \| BrakeTimeSec=(?<braketime>[-0-9.]+) \| BrakeDistanceM=(?<brakedistance>[-0-9.]+) \| SteeringStartKmh=(?<steeringstart>[-0-9.]+) \| SteeringInput=(?<steeringinput>[-0-9.]+) \| SteeringDurationSec=(?<steeringduration>[-0-9.]+) \| SteeringYawDeg=(?<steeringyaw>[-0-9.]+) \| SteeringEndKmh=(?<steeringend>[-0-9.]+)')
# 성공 완료 marker 전체 목록입니다.
$SuccessMatches = $SuccessRegex.Matches($AutomationLogText)
# 실패 완료 marker 전체 목록입니다.
$FailureMatches = $FailureRegex.Matches($AutomationLogText)
# 세 Fixture의 정량 결과 marker 목록입니다.
$MetricMatches = $MetricRegex.Matches($AutomationLogText)
# 정량 결과를 Fixture별 구조화 객체로 변환할 배열입니다.
$Metrics = @()

foreach ($MetricMatch in $MetricMatches) {
    # 현재 정규식 Match에서 추출한 한 Fixture의 구조화 결과입니다.
    $Metric = [ordered]@{
        fixture = $MetricMatch.Groups['fixture'].Value
        configured_mass_kg = [double]::Parse($MetricMatch.Groups['configured'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        actual_mass_kg = [double]::Parse($MetricMatch.Groups['actual'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        acceleration_reference_kmh = [double]::Parse($MetricMatch.Groups['accelref'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        acceleration_time_sec = [double]::Parse($MetricMatch.Groups['acceltime'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        acceleration_distance_m = [double]::Parse($MetricMatch.Groups['acceldistance'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        braking_start_kmh = [double]::Parse($MetricMatch.Groups['brakestart'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        braking_stop_kmh = [double]::Parse($MetricMatch.Groups['brakestop'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        braking_time_sec = [double]::Parse($MetricMatch.Groups['braketime'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        braking_distance_m = [double]::Parse($MetricMatch.Groups['brakedistance'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        steering_start_kmh = [double]::Parse($MetricMatch.Groups['steeringstart'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        steering_input = [double]::Parse($MetricMatch.Groups['steeringinput'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        steering_duration_sec = [double]::Parse($MetricMatch.Groups['steeringduration'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        steering_yaw_deg = [double]::Parse($MetricMatch.Groups['steeringyaw'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
        steering_end_kmh = [double]::Parse($MetricMatch.Groups['steeringend'].Value, [System.Globalization.CultureInfo]::InvariantCulture)
    }
    $Metrics += [pscustomobject]$Metric
}

# 세 Fixture가 정확히 한 번씩 측정됐는지 확인할 고유 Fixture 이름 목록입니다.
$MeasuredFixtureNames = @($Metrics | ForEach-Object { $_.fixture } | Sort-Object -Unique)
# 테스트 성공·Engine 정상 종료·세 Metric 완비만 기술 PASS로 판정하며 수치 대소관계는 판정하지 않습니다.
$AutomationPassed = ($EngineExitCode -eq 0) -and ($FailureMatches.Count -eq 0) -and ($SuccessMatches.Count -gt 0) -and ($Metrics.Count -eq 3) -and ($MeasuredFixtureNames.Count -eq 3)

# Browser evidence에 필요한 최소 필드와 세 정량 결과를 가진 결과 객체입니다.
$Result = [ordered]@{
    schema_version = 'carfight_mobility_measure_result_v1'
    status = if ($AutomationPassed) { 'success' } else { 'failed' }
    test_filter = $TestFilter
    engine_exit_code = $EngineExitCode
    success_marker_count = $SuccessMatches.Count
    failure_marker_count = $FailureMatches.Count
    metric_count = $Metrics.Count
    ordering_asserted = $false
    user_driving_feel_asserted = $false
    metrics = $Metrics
    log_path = $AutomationLogPath
}

# PowerShell JSON 직렬화 결과입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 8
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Output "METRIC_COUNT=$($Metrics.Count)"
$Metrics | ForEach-Object {
    Write-Output ("METRIC={0}|Mass={1:F3}|Accel={2:F6}s/{3:F6}m|Brake={4:F6}s/{5:F6}m|Yaw={6:F6}deg" -f $_.fixture, $_.configured_mass_kg, $_.acceleration_time_sec, $_.acceleration_distance_m, $_.braking_time_sec, $_.braking_distance_m, $_.steering_yaw_deg)
}

if (-not $AutomationPassed) {
    # 실패 시 진단에 필요한 FIT-P0-07C와 Automation 관련 마지막 로그만 bounded하게 출력합니다.
    $DiagnosticLines = @($AutomationLogText -split "`r?`n" | Where-Object {
        $_ -match 'FIT-P0-07C|LogAutomation|Test Completed|Error:'
    } | Select-Object -Last 120)
    $DiagnosticLines | ForEach-Object { Write-Output $_ }
    exit 1
}

exit 0
