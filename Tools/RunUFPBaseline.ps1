# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunUFPBaseline.ps1
# Version: v1.0.0
# Date: 2026-09-05
# Description: UFP-01 controlled process-per-test baseline exact1의 existing runner output을 machine-readable evidence로 수집합니다.
# Scope: RunBuilderTransTests.ps1을 정확히 1회 호출하고 그 내부 Stopwatch wall/exact9 PASS marker를 결과 JSON으로 투영합니다.
# Changelog:
# - v1.0.0: Existing baseline runner semantics를 변경하지 않는 exact1 evidence wrapper 최초 추가.
# Migration:
# - Product Source/Asset/Config를 수정하지 않습니다.
# - ROI wall authority는 child RunBuilderTransTests.ps1의 WORKFLOW_EXECUTION_WALL_MS이며 wrapper/process.status wall을 사용하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 현재 CarFight 저장소 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path

# 기존 process-per-test exact9 baseline runner입니다.
$BaselineRunnerPath = Join-Path $PSScriptRoot 'RunBuilderTransTests.ps1'

# baseline machine-readable evidence 결과 경로입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\UFPBaselineResult.json'

# 결과 상위 디렉터리입니다.
$ResultDirectory = Split-Path -Parent $ResultJsonPath

if (-not (Test-Path -LiteralPath $BaselineRunnerPath -PathType Leaf)) {
    throw "UFP baseline runner를 찾을 수 없습니다: $BaselineRunnerPath"
}

[System.IO.Directory]::CreateDirectory($ResultDirectory) | Out-Null
if (Test-Path -LiteralPath $ResultJsonPath -PathType Leaf) {
    Remove-Item -LiteralPath $ResultJsonPath -Force
}

# baseline runner의 bounded textual output 전체입니다.
$BaselineOutputObjects = @(& $BaselineRunnerPath 2>&1)

# baseline runner actual exit code입니다.
$BaselineExitCode = $LASTEXITCODE

# parser용 plain-text output lines입니다.
$BaselineOutputLines = @($BaselineOutputObjects | ForEach-Object { $_.ToString() })

# exact9 success marker에서 full test path를 추출합니다.
$PassedTests = @(
    $BaselineOutputLines |
        Where-Object { $_ -match '^BUILDER_TRANS_TEST_PASS=(?<name>.+)$' } |
        ForEach-Object { [regex]::Match($_, '^BUILDER_TRANS_TEST_PASS=(?<name>.+)$').Groups['name'].Value }
)

# Consumer-local baseline runner가 기록한 authoritative workflow wall marker입니다.
$WorkflowWallMatches = @(
    $BaselineOutputLines |
        Where-Object { $_ -match '^WORKFLOW_EXECUTION_WALL_MS=(?<ms>[0-9]+(?:\.[0-9]+)?)$' }
)

# baseline 내부 timer가 high-resolution이었는지 나타내는 marker입니다.
$HighResolutionMatches = @(
    $BaselineOutputLines |
        Where-Object { $_ -match '^WORKFLOW_TIMER_HIGH_RESOLUTION=(?<value>True|False)$' }
)

# baseline 내부 timer frequency marker입니다.
$TimerFrequencyMatches = @(
    $BaselineOutputLines |
        Where-Object { $_ -match '^WORKFLOW_TIMER_FREQUENCY=(?<value>[0-9]+)$' }
)

# authoritative wall을 정확히 하나만 허용합니다.
$WorkflowWallMs = if ($WorkflowWallMatches.Count -eq 1) {
    [double]::Parse(
        [regex]::Match($WorkflowWallMatches[0], '^WORKFLOW_EXECUTION_WALL_MS=(?<ms>[0-9]+(?:\.[0-9]+)?)$').Groups['ms'].Value,
        [System.Globalization.CultureInfo]::InvariantCulture
    )
}
else {
    $null
}

# high-resolution marker의 exact boolean 값입니다.
$TimerHighResolution = if ($HighResolutionMatches.Count -eq 1) {
    [regex]::Match($HighResolutionMatches[0], '^WORKFLOW_TIMER_HIGH_RESOLUTION=(?<value>True|False)$').Groups['value'].Value -eq 'True'
}
else {
    $false
}

# timer frequency exact integer 값입니다.
$TimerFrequency = if ($TimerFrequencyMatches.Count -eq 1) {
    [int64]::Parse(
        [regex]::Match($TimerFrequencyMatches[0], '^WORKFLOW_TIMER_FREQUENCY=(?<value>[0-9]+)$').Groups['value'].Value,
        [System.Globalization.CultureInfo]::InvariantCulture
    )
}
else {
    [int64]0
}

# exact9가 모두 성공했고 timer evidence도 완전한지 나타냅니다.
$BaselinePassed = (
    $BaselineExitCode -eq 0 -and
    $PassedTests.Count -eq 9 -and
    $null -ne $WorkflowWallMs -and
    $TimerHighResolution -and
    $TimerFrequency -gt 0
)

# controlled baseline exact1 evidence envelope입니다.
$Result = [ordered]@{
    schema_version = 'carfight_ufp_baseline_v1'
    status = if ($BaselinePassed) { 'success' } else { 'failed' }
    topology = 'process_per_test'
    editor_launch_count = 9
    requested_test_count = 9
    executed_pass_count = $PassedTests.Count
    passed_tests = $PassedTests
    baseline_exit_code = $BaselineExitCode
    workflow_execution_wall_ms = $WorkflowWallMs
    workflow_timer_high_resolution = $TimerHighResolution
    workflow_timer_frequency = $TimerFrequency
    log_suppression_mode = 'OFF'
}

# UTF-8 machine-readable baseline result입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 6
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

$BaselineOutputLines | ForEach-Object { Write-Output $_ }
Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "UFP_BASELINE_STATUS=$($Result.status)"
Write-Output "UFP_BASELINE_WALL_MS=$WorkflowWallMs"
Write-Output "UFP_BASELINE_PASS_COUNT=$($PassedTests.Count)"

if (-not $BaselinePassed) {
    exit 1
}

exit 0
