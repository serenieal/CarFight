# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunUFPBatch.ps1
# Version: v1.0.0
# Date: 2026-09-05
# Description: UFP-01 Builder Transmission exact9을 한 UnrealEditor process에서 실행하고 comparable workflow wall/equivalence evidence를 기록합니다.
# Scope: Existing exact9 names, UE 5.8 executable, unattended options, NullRHI, log suppression OFF를 유지하고 Automation RunTests의 '+' multi-test syntax만 사용합니다.
# Changelog:
# - v1.0.0: exact9 same-process batch + high-resolution Stopwatch + exact requested/executed/missing/unexpected/duplicate result 검증을 최초 구현.
# Migration:
# - Product Source/Asset/Config를 저장하지 않습니다.
# - GoPyMCP public/policy/runtime surface를 사용하거나 변경하지 않습니다.
# - 결과는 UE/Saved/CarFight/UFPBatchResult.json과 기존 Data Authoring Automation log에만 기록합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 현재 CarFight 저장소 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path

# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'

# CarFight 공식 UE 5.8 Source Build Editor입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# baseline child runner와 동일한 Automation log 경로입니다.
$AutomationLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFDataAuthoringAutomation.log'

# same-process batch machine-readable 결과 경로입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\UFPBatchResult.json'

# 결과 상위 디렉터리입니다.
$ResultDirectory = Split-Path -Parent $ResultJsonPath

# UFP-01에서 frozen된 exact9 test names/order입니다.
$Tests = @(
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderEvidenceRefresh',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderTransmissionContract',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderProfileCommit',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep1Reference',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderEvidenceRefreshVM',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep5Physics',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep7FinalReview',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep8Driving'
)

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

# Unreal Automation multi-test filter는 frozen exact9 full paths를 '+'로 결합합니다.
$CombinedTestFilter = $Tests -join '+'

# baseline child runner와 같은 RunTests command family를 사용한 one-process command입니다.
$AutomationCommand = "Automation RunTests $CombinedTestFilter"

# baseline child runner와 동일한 non-interactive Editor options입니다.
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

# same-process exact9 Consumer workflow의 high-resolution monotonic wall timer입니다.
$WorkflowStopwatch = [System.Diagnostics.Stopwatch]::StartNew()

# exact one UnrealEditor process를 시작합니다.
$EditorProcess = Start-Process -FilePath $EditorExecutable -ArgumentList $EditorArguments -WorkingDirectory $RepositoryRoot -PassThru
$EditorProcess.WaitForExit()

# exact UnrealEditor process terminal exit code입니다.
$EngineExitCode = $EditorProcess.ExitCode

if (-not (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf)) {
    $WorkflowStopwatch.Stop()
    throw "UFP same-process Automation 로그가 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# UTF-8로 읽은 exact same-process Automation log입니다.
$AutomationLogText = [System.IO.File]::ReadAllText($AutomationLogPath, [System.Text.UTF8Encoding]::new($false))

# 모든 Automation terminal marker의 result/path를 추출하는 regex입니다.
$TerminalRegex = [regex]('Test Completed\. Result=\{(?<result>Success|Fail|Failed)\}.*?Path=\{(?<path>[^}]+)\}')

# current one-process run의 terminal marker 전체입니다.
$TerminalMatches = @($TerminalRegex.Matches($AutomationLogText))

# terminal marker에 실제로 나타난 test path 목록입니다.
$ExecutedTests = @($TerminalMatches | ForEach-Object { $_.Groups['path'].Value })

# 중복 제거된 actual test path 목록입니다.
$UniqueExecutedTests = @($ExecutedTests | Sort-Object -Unique)

# failure result marker가 있는 test path 목록입니다.
$FailedTests = @(
    $TerminalMatches |
        Where-Object { $_.Groups['result'].Value -ne 'Success' } |
        ForEach-Object { $_.Groups['path'].Value }
)

# frozen exact9 중 actual terminal marker가 없는 tests입니다.
$MissingTests = @($Tests | Where-Object { $_ -notin $UniqueExecutedTests })

# frozen exact9 외 actual terminal marker가 나타난 tests입니다.
$UnexpectedTests = @($UniqueExecutedTests | Where-Object { $_ -notin $Tests })

# 동일 test가 둘 이상 terminal marker로 기록된 duplicate count입니다.
$DuplicateTerminalCount = $ExecutedTests.Count - $UniqueExecutedTests.Count

# same-process workflow wall은 result parsing까지 포함한 Consumer-local aggregate입니다.
$WorkflowStopwatch.Stop()

# authoritative same-process workflow wall milliseconds입니다.
$WorkflowWallMs = $WorkflowStopwatch.Elapsed.TotalMilliseconds

# current .NET Stopwatch high-resolution 여부입니다.
$TimerHighResolution = [System.Diagnostics.Stopwatch]::IsHighResolution

# current .NET Stopwatch frequency입니다.
$TimerFrequency = [System.Diagnostics.Stopwatch]::Frequency

# exact9 semantic/equivalence envelope가 모두 충족됐는지 여부입니다.
$BatchPassed = (
    $EngineExitCode -eq 0 -and
    $TimerHighResolution -and
    $Tests.Count -eq 9 -and
    $ExecutedTests.Count -eq 9 -and
    $UniqueExecutedTests.Count -eq 9 -and
    $FailedTests.Count -eq 0 -and
    $MissingTests.Count -eq 0 -and
    $UnexpectedTests.Count -eq 0 -and
    $DuplicateTerminalCount -eq 0
)

# UFP-01 same-process batch evidence envelope입니다.
$Result = [ordered]@{
    schema_version = 'carfight_ufp_batch_v1'
    status = if ($BatchPassed) { 'success' } else { 'failed' }
    topology = 'same_process_exact_list'
    editor_launch_count = 1
    requested_test_count = $Tests.Count
    requested_tests = $Tests
    terminal_marker_count = $ExecutedTests.Count
    executed_unique_count = $UniqueExecutedTests.Count
    executed_tests = $UniqueExecutedTests
    failed_tests = $FailedTests
    missing_tests = $MissingTests
    unexpected_tests = $UnexpectedTests
    duplicate_terminal_count = $DuplicateTerminalCount
    engine_exit_code = $EngineExitCode
    workflow_execution_wall_ms = $WorkflowWallMs
    workflow_timer_high_resolution = $TimerHighResolution
    workflow_timer_frequency = $TimerFrequency
    log_suppression_mode = 'OFF'
    automation_command_mode = 'RunTests plus-separated exact full paths'
    log_path = $AutomationLogPath
}

# UTF-8 machine-readable batch result입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 7
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

Write-Output 'MODEL_CONTEXT_PROTOCOL_LOG_SUPPRESSED=False'
Write-Output ("WORKFLOW_TIMER_HIGH_RESOLUTION={0}" -f $TimerHighResolution)
Write-Output ("WORKFLOW_TIMER_FREQUENCY={0}" -f $TimerFrequency)
Write-Output ("WORKFLOW_EXECUTION_WALL_MS={0:F3}" -f $WorkflowWallMs)
Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "UFP_BATCH_STATUS=$($Result.status)"
Write-Output "UFP_BATCH_EXECUTED_UNIQUE=$($UniqueExecutedTests.Count)"
Write-Output "UFP_BATCH_MISSING=$($MissingTests.Count)"
Write-Output "UFP_BATCH_UNEXPECTED=$($UnexpectedTests.Count)"
Write-Output "UFP_BATCH_FAILED=$($FailedTests.Count)"
Write-Output "UFP_BATCH_DUPLICATE_TERMINAL=$DuplicateTerminalCount"

if (-not $BatchPassed) {
    # failure RCA에 필요한 bounded Automation lines입니다.
    $DiagnosticLines = @($AutomationLogText -split "`r?`n" | Where-Object {
        $_ -match 'LogAutomation|Test Completed|CarFight\.DataAuthoring|Error:'
    } | Select-Object -Last 180)
    $DiagnosticLines | ForEach-Object { Write-Output $_ }
    exit 1
}

exit 0
