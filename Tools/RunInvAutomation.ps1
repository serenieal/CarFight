# CarFight CF-FQ-035 targeted Inventory Automation 작업 전용 runner.
# Version: v1.0.1
# Date: 2026-08-14
# Changelog:
# - v1.0.1: 중괄호가 포함된 Automation 결과 정규식을 PowerShell -f 포맷이 아닌 문자열 연결 방식으로 생성해 결과 파싱 오류를 교정합니다.
# - v1.0.0: 검증된 UnrealEditor.exe + -abslog + Start-Process -Wait 방식으로 지정 Inventory filter를 실행하고 실제 Success/Fail marker를 JSON으로 기록합니다.
# Migration:
# - 이 스크립트는 CF-FQ-035 작업 전용 실행 수단이며 별도 공용화 결정 전 공식 범용 Tool로 해석하지 않습니다.
# - Product Source/Config/Asset을 저장하지 않으며 결과는 UE/Saved/CarFight/InvAutomationResult.json에만 기록합니다.

[CmdletBinding()]
param(
    # 실행할 Inventory Automation 전체 경로 prefix 또는 filter 문자열입니다.
    [string]$TestFilter = 'CarFight.Inventory.INV_P0_05'
)

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 현재 저장소의 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build Editor 실행 파일입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'
# 이번 targeted Inventory Automation의 전용 Unreal 로그입니다.
$AutomationLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFInvAutomation.log'
# Browser가 bounded evidence로 회수할 machine-readable 결과 파일입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\InvAutomationResult.json'
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
    throw "Inventory Automation 로그가 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# UE 로그를 Windows PowerShell 기본 인코딩 추측 없이 UTF-8로 읽은 전체 문자열입니다.
$AutomationLogText = [System.IO.File]::ReadAllText($AutomationLogPath, [System.Text.UTF8Encoding]::new($false))
# 결과 marker에서 허용할 CarFight Inventory test path prefix의 정규식 안전 문자열입니다.
$EscapedTestFilter = [regex]::Escape($TestFilter)
# 성공 완료 marker에서 이번 filter의 실제 테스트 경로를 추출하는 정규식입니다.
$SuccessRegex = [regex]('Test Completed\. Result=\{Success\}.*?Path=\{(?<path>' + $EscapedTestFilter + '(?:\.[^}]+)?)\}')
# 실패 완료 marker에서 이번 filter의 실제 테스트 경로를 추출하는 정규식입니다.
$FailureRegex = [regex]('Test Completed\. Result=\{(?:Fail|Failed)\}.*?Path=\{(?<path>' + $EscapedTestFilter + '(?:\.[^}]+)?)\}')
# 실제 성공 완료 marker 전체 목록입니다.
$SuccessMatches = $SuccessRegex.Matches($AutomationLogText)
# 실제 실패 완료 marker 전체 목록입니다.
$FailureMatches = $FailureRegex.Matches($AutomationLogText)
# 성공한 테스트의 전체 Path 목록입니다.
$SuccessfulTestPaths = @($SuccessMatches | ForEach-Object { $_.Groups['path'].Value } | Sort-Object -Unique)
# 실패한 테스트의 전체 Path 목록입니다.
$FailedTestPaths = @($FailureMatches | ForEach-Object { $_.Groups['path'].Value } | Sort-Object -Unique)
# 실패 marker가 없고 엔진도 정상 종료했으며 성공 marker가 하나 이상 존재하는지 여부입니다.
$AutomationPassed = ($EngineExitCode -eq 0) -and ($FailedTestPaths.Count -eq 0) -and ($SuccessfulTestPaths.Count -gt 0)

# Browser evidence에 필요한 최소 필드만 가진 결과 객체입니다.
$Result = [ordered]@{
    schema_version = 'carfight_inventory_automation_result_v1'
    status = if ($AutomationPassed) { 'success' } else { 'failed' }
    test_filter = $TestFilter
    engine_exit_code = $EngineExitCode
    success_count = $SuccessfulTestPaths.Count
    failure_count = $FailedTestPaths.Count
    successful_tests = $SuccessfulTestPaths
    failed_tests = $FailedTestPaths
    log_path = $AutomationLogPath
}

# PowerShell JSON 직렬화 결과입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 6
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Output "SUCCESS_COUNT=$($SuccessfulTestPaths.Count)"
Write-Output "FAILURE_COUNT=$($FailedTestPaths.Count)"
$SuccessfulTestPaths | ForEach-Object { Write-Output "PASS=$_" }
$FailedTestPaths | ForEach-Object { Write-Output "FAIL=$_" }

if (-not $AutomationPassed) {
    # 실패 시 진단에 필요한 Automation 관련 마지막 로그만 bounded하게 출력합니다.
    $DiagnosticLines = @($AutomationLogText -split "`r?`n" | Where-Object {
        $_ -match 'LogAutomation|Test Completed|CarFight\.Inventory|Error:'
    } | Select-Object -Last 100)
    $DiagnosticLines | ForEach-Object { Write-Output $_ }
    exit 1
}

exit 0
