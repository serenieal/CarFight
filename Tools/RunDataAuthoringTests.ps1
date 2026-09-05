# CarFight Data Authoring targeted Automation runner.
# Version: v1.3.1
# Date: 2026-09-05
# Description: DAUTH-P0-08 이후 Data Authoring Automation filter 또는 exact-name 배열을 공식 UE 5.8 Editor 한 프로세스에서 무인 실행하고 JSON 결과를 기록합니다.
# Changelog:
# - v1.3.1: 기존 filter mode의 result schema v1 envelope를 byte-shape 호환으로 복원하고 exact-list mode만 v2 envelope를 사용. Exact name에서 UE command expression 분리 문자(+ ; ")를 fail-closed해 unintended command/test expansion을 차단.
# - v1.3.0: UFP-02 최소 운영 채택을 위해 opt-in ExactTestNames 배열을 추가. '+' multi-test command로 한 Editor process에서 exact set을 실행하고 missing/unexpected/duplicate까지 fail-closed 검증하며 기존 TestFilter 호출은 그대로 보존.
# - v1.2.1: MCP log suppression 사용 여부를 `MODEL_CONTEXT_PROTOCOL_LOG_SUPPRESSED=True/False` marker로 명시해 focused evidence의 실행 조건을 추적 가능하게 함.
# - v1.2.0: opt-in SuppressModelContextProtocolLog switch를 추가해 제품 설정 변경 없이 unattended Automation에서 Experimental MCP 비동기 로그 오염을 격리할 수 있게 함. 기본값은 false.
# - v1.1.0: Start-Process -Wait의 descendant-process 대기를 제거하고 exact UnrealEditor Process 객체 WaitForExit()만 사용해 TestExit 이후 wrapper가 terminal로 회수되도록 교정.
# - v1.0.0: CarFight.DataAuthoring 전용 재실행 가능한 Automation 진입점을 추가.
# Migration:
# - 기존 -TestFilter 호출은 동작 변경 없이 유지됩니다. Same-process exact batching이 필요한 Consumer만 -ExactTestNames를 opt-in합니다.
# - Product Source/Config/Content Asset을 저장하지 않습니다.
# - 결과는 UE/Saved/CarFight/DataAuthoringAutomationResult.json과 전용 로그에만 기록합니다.

[CmdletBinding()]
param(
    # 실행할 Data Authoring Automation 전체 경로 prefix 또는 filter 문자열입니다.
    [string]$TestFilter = 'CarFight.DataAuthoring',

    # 한 UnrealEditor process에서 exact set으로 실행할 full Automation test name 배열입니다.
    [string[]]$ExactTestNames = @(),

    # Experimental ModelContextProtocol의 비동기 로그가 무관한 Automation을 Fail시키는 환경에서 해당 category만 실행 중 숨깁니다.
    [switch]$SuppressModelContextProtocolLog
)

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 현재 CarFight 저장소의 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
# CarFight 공식 Unreal 프로젝트 파일입니다.
$ProjectFile = Join-Path $RepositoryRoot 'UE\CarFight_Re.uproject'
# CarFight 공식 UE 5.8 Source Build Editor 실행 파일입니다.
$EditorExecutable = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'
# 이번 Data Authoring targeted Automation의 전용 Unreal 로그입니다.
$AutomationLogPath = Join-Path $RepositoryRoot 'UE\Saved\Logs\CFDataAuthoringAutomation.log'
# Browser가 bounded evidence로 회수할 machine-readable 결과 파일입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\DataAuthoringAutomationResult.json'
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

# exact-list mode에서 null을 빈 값으로 정규화하고 공백을 제거한 full Automation test name 배열입니다.
$NormalizedExactTestNames = @($ExactTestNames | ForEach-Object { if ($null -eq $_) { '' } else { $_.Trim() } })

if ($NormalizedExactTestNames.Count -gt 0) {
    if ($NormalizedExactTestNames.Count -gt 64) {
        throw "ExactTestNames는 최대 64개까지 허용합니다. actual=$($NormalizedExactTestNames.Count)"
    }

    # exact-list 입력의 빈 이름, control character 또는 UE command-expression 분리 문자 존재 여부입니다.
    $InvalidExactTestName = $NormalizedExactTestNames | Where-Object {
        [string]::IsNullOrWhiteSpace($_) -or
        $_.ToCharArray().Where({ [char]::IsControl($_) }).Count -gt 0 -or
        $_ -match '[+;"]'
    } | Select-Object -First 1
    if ($null -ne $InvalidExactTestName) {
        throw 'ExactTestNames에는 빈 이름, control character 또는 command-expression 분리 문자(+ ; ")를 사용할 수 없습니다.'
    }

    # exact-list 입력의 중복 이름 수입니다.
    $DuplicateRequestedCount = $NormalizedExactTestNames.Count - @($NormalizedExactTestNames | Sort-Object -Unique).Count
    if ($DuplicateRequestedCount -ne 0) {
        throw "ExactTestNames duplicate는 허용하지 않습니다. duplicate_count=$DuplicateRequestedCount"
    }
}

# current invocation의 실행 모드입니다.
$ExecutionMode = if ($NormalizedExactTestNames.Count -gt 0) { 'exact_list_same_process' } else { 'filter' }

# Unreal Automation RunTests에 전달할 filter 또는 '+' 결합 exact test expression입니다.
$AutomationTestExpression = if ($NormalizedExactTestNames.Count -gt 0) {
    $NormalizedExactTestNames -join '+'
}
else {
    $TestFilter
}

# Unreal Automation Controller에 전달할 명령 문자열입니다.
$AutomationCommand = "Automation RunTests $AutomationTestExpression"
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

if ($SuppressModelContextProtocolLog) {
    # WSA 등 MCP 자체를 검증하지 않는 unattended Automation에서 Experimental MCP의 비동기 Error 로그가 현재 test에 귀속되는 것을 방지합니다.
    $EditorArguments += '-LogCmds="LogModelContextProtocol off"'
}

# 현재 targeted run이 Experimental MCP log suppression을 사용했는지 process evidence에 명시합니다.
Write-Output ("MODEL_CONTEXT_PROTOCOL_LOG_SUPPRESSED={0}" -f [bool]$SuppressModelContextProtocolLog)

# actual UnrealEditor Automation exact PID를 시작하며 Process Tree 전체가 아니라 이 Editor PID만 기다립니다.
$EditorProcess = Start-Process -FilePath $EditorExecutable -ArgumentList $EditorArguments -WorkingDirectory $RepositoryRoot -PassThru
$EditorProcess.WaitForExit()
# exact UnrealEditor PID가 반환한 실제 프로세스 종료 코드입니다.
$EngineExitCode = $EditorProcess.ExitCode

if (-not (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf)) {
    throw "Data Authoring Automation 로그가 생성되지 않았습니다. EngineExitCode=$EngineExitCode"
}

# UE 로그를 Windows PowerShell 기본 인코딩 추측 없이 UTF-8로 읽은 전체 문자열입니다.
$AutomationLogText = [System.IO.File]::ReadAllText($AutomationLogPath, [System.Text.UTF8Encoding]::new($false))

# exact-list mode에서 모든 Automation terminal marker를 읽는 정규식입니다.
$TerminalRegex = [regex]('Test Completed\. Result=\{(?<result>Success|Fail|Failed)\}.*?Path=\{(?<path>[^}]+)\}')

# current invocation에서 실제 성공 완료한 unique test path입니다.
$SuccessfulTestPaths = @()

# current invocation에서 실제 실패 완료한 unique test path입니다.
$FailedTestPaths = @()

# exact-list mode에서 요청했지만 terminal marker가 없는 test path입니다.
$MissingTestPaths = @()

# exact-list mode에서 요청하지 않았지만 terminal marker가 나온 test path입니다.
$UnexpectedTestPaths = @()

# exact-list mode에서 동일 test terminal marker가 중복된 수입니다.
$DuplicateTerminalCount = 0

if ($NormalizedExactTestNames.Count -gt 0) {
    # exact-list one-process 실행의 terminal marker 전체입니다.
    $TerminalMatches = @($TerminalRegex.Matches($AutomationLogText))

    # actual terminal marker test path 전체입니다.
    $TerminalTestPaths = @($TerminalMatches | ForEach-Object { $_.Groups['path'].Value })

    # actual terminal marker의 unique test path입니다.
    $UniqueTerminalTestPaths = @($TerminalTestPaths | Sort-Object -Unique)

    $SuccessfulTestPaths = @(
        $TerminalMatches |
            Where-Object { $_.Groups['result'].Value -eq 'Success' } |
            ForEach-Object { $_.Groups['path'].Value } |
            Sort-Object -Unique
    )
    $FailedTestPaths = @(
        $TerminalMatches |
            Where-Object { $_.Groups['result'].Value -ne 'Success' } |
            ForEach-Object { $_.Groups['path'].Value } |
            Sort-Object -Unique
    )
    $MissingTestPaths = @($NormalizedExactTestNames | Where-Object { $_ -notin $UniqueTerminalTestPaths })
    $UnexpectedTestPaths = @($UniqueTerminalTestPaths | Where-Object { $_ -notin $NormalizedExactTestNames })
    $DuplicateTerminalCount = $TerminalTestPaths.Count - $UniqueTerminalTestPaths.Count

    # exact-list는 requested=executed unique equality와 failure/missing/unexpected/duplicate0까지 모두 요구합니다.
    $AutomationPassed = (
        $EngineExitCode -eq 0 -and
        $SuccessfulTestPaths.Count -eq $NormalizedExactTestNames.Count -and
        $FailedTestPaths.Count -eq 0 -and
        $MissingTestPaths.Count -eq 0 -and
        $UnexpectedTestPaths.Count -eq 0 -and
        $DuplicateTerminalCount -eq 0
    )
}
else {
    # 결과 marker에서 허용할 Data Authoring test path prefix의 정규식 안전 문자열입니다.
    $EscapedTestFilter = [regex]::Escape($TestFilter)
    # 성공 완료 marker에서 이번 filter의 실제 테스트 경로를 추출하는 정규식입니다.
    $SuccessRegex = [regex]('Test Completed\. Result=\{Success\}.*?Path=\{(?<path>' + $EscapedTestFilter + '(?:\.[^}]+)?)\}')
    # 실패 완료 marker에서 이번 filter의 실제 테스트 경로를 추출하는 정규식입니다.
    $FailureRegex = [regex]('Test Completed\. Result=\{(?:Fail|Failed)\}.*?Path=\{(?<path>' + $EscapedTestFilter + '(?:\.[^}]+)?)\}')
    $SuccessfulTestPaths = @($SuccessRegex.Matches($AutomationLogText) | ForEach-Object { $_.Groups['path'].Value } | Sort-Object -Unique)
    $FailedTestPaths = @($FailureRegex.Matches($AutomationLogText) | ForEach-Object { $_.Groups['path'].Value } | Sort-Object -Unique)

    # 기존 filter mode의 PASS 의미는 v1.2.1과 동일하게 유지합니다.
    $AutomationPassed = ($EngineExitCode -eq 0) -and ($FailedTestPaths.Count -eq 0) -and ($SuccessfulTestPaths.Count -gt 0)
}

# Browser evidence에 필요한 결과 객체입니다. Legacy filter mode는 기존 v1 envelope를 그대로 보존합니다.
$Result = if ($NormalizedExactTestNames.Count -gt 0) {
    [ordered]@{
        schema_version = 'carfight_data_authoring_automation_result_v2'
        status = if ($AutomationPassed) { 'success' } else { 'failed' }
        execution_mode = $ExecutionMode
        requested_test_count = $NormalizedExactTestNames.Count
        requested_tests = $NormalizedExactTestNames
        engine_exit_code = $EngineExitCode
        success_count = $SuccessfulTestPaths.Count
        failure_count = $FailedTestPaths.Count
        missing_count = $MissingTestPaths.Count
        unexpected_count = $UnexpectedTestPaths.Count
        duplicate_terminal_count = $DuplicateTerminalCount
        successful_tests = $SuccessfulTestPaths
        failed_tests = $FailedTestPaths
        missing_tests = $MissingTestPaths
        unexpected_tests = $UnexpectedTestPaths
        log_path = $AutomationLogPath
    }
}
else {
    [ordered]@{
        schema_version = 'carfight_data_authoring_automation_result_v1'
        status = if ($AutomationPassed) { 'success' } else { 'failed' }
        test_filter = $TestFilter
        engine_exit_code = $EngineExitCode
        success_count = $SuccessfulTestPaths.Count
        failure_count = $FailedTestPaths.Count
        successful_tests = $SuccessfulTestPaths
        failed_tests = $FailedTestPaths
        log_path = $AutomationLogPath
    }
}

# PowerShell JSON 직렬화 결과입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 6
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

Write-Output "RESULT_JSON=$ResultJsonPath"
Write-Output "EXECUTION_MODE=$ExecutionMode"
Write-Output "ENGINE_EXIT_CODE=$EngineExitCode"
Write-Output "SUCCESS_COUNT=$($SuccessfulTestPaths.Count)"
Write-Output "FAILURE_COUNT=$($FailedTestPaths.Count)"
Write-Output "MISSING_COUNT=$($MissingTestPaths.Count)"
Write-Output "UNEXPECTED_COUNT=$($UnexpectedTestPaths.Count)"
Write-Output "DUPLICATE_TERMINAL_COUNT=$DuplicateTerminalCount"
$SuccessfulTestPaths | ForEach-Object { Write-Output "PASS=$_" }
$FailedTestPaths | ForEach-Object { Write-Output "FAIL=$_" }
$MissingTestPaths | ForEach-Object { Write-Output "MISSING=$_" }
$UnexpectedTestPaths | ForEach-Object { Write-Output "UNEXPECTED=$_" }

if (-not $AutomationPassed) {
    # 실패 시 진단에 필요한 Data Authoring Automation 관련 마지막 로그만 bounded하게 출력합니다.
    $DiagnosticLines = @($AutomationLogText -split "`r?`n" | Where-Object {
        $_ -match 'LogAutomation|Test Completed|CarFight\.DataAuthoring|Error:'
    } | Select-Object -Last 120)
    $DiagnosticLines | ForEach-Object { Write-Output $_ }
    exit 1
}

exit 0
