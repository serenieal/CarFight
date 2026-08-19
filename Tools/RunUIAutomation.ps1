# CarFight CF-FQ-032 UI targeted Automation 작업 전용 runner.
# Version: v1.1.0
# Date: 2026-08-18
# Changelog:
# - v1.1.0: 결과 파서를 UI_P0_03 고정 경로에서 호출자가 지정한 CarFight.UI TestFilter prefix 기준으로 일반화해 UI-P0-04 이후 focused 검증도 같은 runner에서 정확히 판독합니다.
# - v1.0.3: 절대 Automation 로그 경로가 실제 파일로 생성되도록 Unreal의 -abslog 인자를 사용합니다.
# - v1.0.2: GUI UnrealEditor.exe를 Start-Process -Wait -PassThru로 실행해 실제 종료 코드를 회수하고 공백이 있는 ExecCmds/TestExit 인자를 명시적으로 인용하도록 교정했습니다.
# - v1.0.1: CarFight 직접 실행 강제 규칙에 맞춰 실행 파일을 공식 D:\UnrealEngine_Source의 UnrealEditor.exe로 교정했습니다.
# - v1.0.0: Unreal Automation RunTests + TestExit를 사용해 지정 filter를 실행하고 실제 Test Completed Success/Fail 로그를 JSON으로 기록합니다.
# Migration:
# - 이 스크립트는 CF-FQ-032 UI 작업 전용 실행 수단이며 별도 공용화 결정 전 프로젝트 범용 Automation Tool로 해석하지 않습니다.
# - Product Source/Config/Asset을 저장하지 않으며 결과는 UE/Saved/CarFight/UIAutomationResult.json에만 기록합니다.

[CmdletBinding()]
param(
    # 실행할 Automation 전체 경로 prefix 또는 필터 문자열입니다.
    [string]$TestFilter = 'CarFight.UI.UI_P0_03'
)

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 현재 저장소의 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build Editor 실행 파일입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'
# 이번 targeted Automation의 전용 Unreal 로그입니다.
$AutomationLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFUIAutomation.log'
# Browser가 bounded evidence로 회수할 machine-readable 결과 파일입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\UIAutomationResult.json'
# 결과 JSON의 상위 디렉터리입니다.
$ResultDirectory = Split-Path -Parent $ResultJsonPath

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
    throw "Automation 로그가 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# UE 로그를 Windows PowerShell 기본 인코딩 추측 없이 UTF-8로 읽은 전체 문자열입니다.
$AutomationLogText = [System.IO.File]::ReadAllText($AutomationLogPath, [System.Text.UTF8Encoding]::new($false))
# Test Completed 라인에서 성공 상태와 CarFight UI 테스트 전체 경로를 추출하는 정규식입니다.
$SuccessRegex = [regex]'Test Completed\. Result=\{Success\}.*?Path=\{(?<path>CarFight\.UI\.[^}]+)\}'
# Test Completed 라인에서 실패 상태와 CarFight UI 테스트 전체 경로를 추출하는 정규식입니다.
$FailureRegex = [regex]'Test Completed\. Result=\{(?:Fail|Failed)\}.*?Path=\{(?<path>CarFight\.UI\.[^}]+)\}'
# 실제 성공 완료 marker 전체 목록입니다.
$SuccessMatches = $SuccessRegex.Matches($AutomationLogText)
# 실제 실패 완료 marker 전체 목록입니다.
$FailureMatches = $FailureRegex.Matches($AutomationLogText)
# 호출자가 요청한 TestFilter prefix에 실제로 포함되는 성공 테스트 Path 목록입니다.
$SuccessfulTestPaths = @($SuccessMatches | ForEach-Object { $_.Groups['path'].Value } | Where-Object { $_.StartsWith($TestFilter, [System.StringComparison]::OrdinalIgnoreCase) })
# 호출자가 요청한 TestFilter prefix에 실제로 포함되는 실패 테스트 Path 목록입니다.
$FailedTestPaths = @($FailureMatches | ForEach-Object { $_.Groups['path'].Value } | Where-Object { $_.StartsWith($TestFilter, [System.StringComparison]::OrdinalIgnoreCase) })
# 같은 테스트의 중복 로그가 있을 때 path 단위로 중복을 제거한 성공 목록입니다.
$UniqueSuccessfulTestPaths = @($SuccessfulTestPaths | Sort-Object -Unique)
# 같은 테스트의 중복 로그가 있을 때 path 단위로 중복을 제거한 실패 목록입니다.
$UniqueFailedTestPaths = @($FailedTestPaths | Sort-Object -Unique)
# 실패 marker가 없고 엔진도 정상 종료했으며 성공 marker가 하나 이상 존재하는지 여부입니다.
$AutomationPassed = ($EngineExitCode -eq 0) -and ($UniqueFailedTestPaths.Count -eq 0) -and ($UniqueSuccessfulTestPaths.Count -gt 0)

# Browser evidence에 필요한 최소 필드만 가진 결과 객체입니다.
$Result = [ordered]@{
    schema_version = 'carfight_ui_automation_result_v1'
    status = if ($AutomationPassed) { 'success' } else { 'failed' }
    test_filter = $TestFilter
    engine_exit_code = $EngineExitCode
    success_count = $UniqueSuccessfulTestPaths.Count
    failure_count = $UniqueFailedTestPaths.Count
    successful_tests = $UniqueSuccessfulTestPaths
    failed_tests = $UniqueFailedTestPaths
    log_path = $AutomationLogPath
}

# PowerShell JSON 직렬화 결과입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 6
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Output "SUCCESS_COUNT=$($UniqueSuccessfulTestPaths.Count)"
Write-Output "FAILURE_COUNT=$($UniqueFailedTestPaths.Count)"
$UniqueSuccessfulTestPaths | ForEach-Object { Write-Output "PASS=$_" }
$UniqueFailedTestPaths | ForEach-Object { Write-Output "FAIL=$_" }

if (-not $AutomationPassed) {
    # 실패 시 진단에 필요한 Automation 관련 마지막 로그만 bounded하게 출력합니다.
    $DiagnosticLines = @($AutomationLogText -split "`r?`n" | Where-Object {
                $_ -match 'LogAutomation|Test Completed|CarFight\.UI\.|Error:'
    } | Select-Object -Last 80)
    $DiagnosticLines | ForEach-Object { Write-Output $_ }
    exit 1
}

exit 0
