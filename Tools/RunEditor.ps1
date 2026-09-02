# CarFight Unreal Editor Browser 실행 래퍼.
# Version: v1.5.0
# Changelog:
# - v1.5.0: ValidateOnly가 canonical RunEditor.bat의 Unreal MCP start flag, 8100 port variable, port override flag를 직접 검사해 launcher/Ready contract 회귀를 offline에서 탐지하도록 보강했습니다.
# - v1.4.0: Canonical RunEditor.bat가 Unreal MCP StartServer + port 8100을 launcher contract로 보장하므로 Ready wait가 per-user Editor Auto Start 설정에 의존하지 않도록 계약을 명시했습니다.
# - v1.3.0: Ready가 증명된 exact CarFight Editor PID를 `editor_process_id` evidence marker로 출력해 Project Runtime이 동일 Consumer process identity를 저장할 수 있게 했다.
# - v1.2.0: 선택적 Ready 대기를 추가해 exact CarFight Editor 프로세스의 8100 listener ownership까지 확인한 뒤 Browser lifecycle start를 완료할 수 있게 했다.
# - v1.1.0: Browser process.run용 검증 전용 모드와 정확한 CarFight Editor 프로세스 확인을 추가했다.
# - v1.0.0: 기존 Tools/RunEditor.bat를 그대로 사용하는 PowerShell 래퍼를 추가했다.
# Migration:
# - v1.5.0부터 `-ValidateOnly`는 launcher 파일 존재뿐 아니라 MCP 8100 시작 계약까지 검사하며 누락 시 fail-closed합니다.
# - v1.4.0부터 canonical RunEditor.bat가 `-ModelContextProtocolStartServer -ModelContextProtocolPort=8100`을 항상 전달합니다. EditorPerProjectUserSettings의 Auto Start/Port 값은 canonical lifecycle prerequisite가 아닙니다.
# - 공식 수동 실행 진입점과 기본 RunEditor.ps1 호출은 기존처럼 프로세스 출현까지만 기다린다. Browser fixed preset만 -WaitForReady를 server-owned 인자로 사용해 8100 Ready까지 보장한다.
# - `editor_process_id`는 이미 Ready ownership이 증명된 exact Consumer process의 관측 evidence일 뿐 lifecycle authority나 force/Save 권한을 추가하지 않는다.

[CmdletBinding()]
param(
    # CarFight Editor 프로세스가 나타날 때까지 기다리는 최대 초 수.
    [int]$StartupTimeoutSeconds = 60,
    # exact CarFight Editor의 8100 listener ownership까지 기다린다.
    [switch]$WaitForReady,
    # 프로세스 출현 뒤 Ready 상태를 기다리는 최대 초 수.
    [int]$ReadyTimeoutSeconds = 120,
    # Unreal Editor를 실행하지 않고 경로와 실행 계약만 검증한다.
    [switch]$ValidateOnly
)

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책.
$ErrorActionPreference = 'Stop'

# 이 스크립트 위치에서 계산한 CarFight 저장소 루트.
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 저장소 정책이 지정한 공식 CarFight Editor BAT 실행기.
$RunEditorBat = Join-Path $PSScriptRoot 'RunEditor.bat'

# 정확한 CarFight Editor 프로세스를 식별할 공식 프로젝트 경로.
$ProjectPath = Join-Path $RepoRoot 'UE\CarFight_Re.uproject'

# CarFight 프로젝트를 실행 중인 UnrealEditor.exe만 반환한다.
function Get-CarFightEditorProcess {
    # 프로세스 명령줄과 비교할 정규화된 절대 프로젝트 경로.
    $NormalizedProject = [IO.Path]::GetFullPath($ProjectPath)

    return @(Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object {
        $_.CommandLine -and ($_.CommandLine.IndexOf($NormalizedProject, [StringComparison]::OrdinalIgnoreCase) -ge 0)
    })
}

# exact CarFight Editor 하나가 local 8100 listener를 직접 소유하는지 확인한다.
function Test-CarFightEditorReady {
    param(
        # Ready ownership을 확인할 exact CarFight Editor 프로세스 목록.
        [object[]]$EditorProcesses
    )

    if ($EditorProcesses.Count -ne 1) {
        return $false
    }

    # exact CarFight Editor의 단일 process ID.
    $EditorProcessId = [int]$EditorProcesses[0].ProcessId
    # local 8100에서 LISTEN 중인 socket 행 목록.
    $ListenerRows = @(Get-NetTCPConnection -State Listen -LocalPort 8100 -ErrorAction SilentlyContinue)
    # Ready는 8100 listener 중 하나가 exact CarFight Editor process에 귀속될 때만 true다.
    return @($ListenerRows | Where-Object { [int]$_.OwningProcess -eq $EditorProcessId }).Count -gt 0
}

if (-not (Test-Path -LiteralPath $RunEditorBat -PathType Leaf)) {
    throw "공식 Editor 실행기를 찾을 수 없습니다: $RunEditorBat"
}

if (-not (Test-Path -LiteralPath $ProjectPath -PathType Leaf)) {
    throw "CarFight 프로젝트를 찾을 수 없습니다: $ProjectPath"
}

if ($ValidateOnly) {
    # Canonical BAT launcher의 현재 UTF-8 텍스트입니다.
    $RunEditorBatText = [System.IO.File]::ReadAllText($RunEditorBat, [System.Text.UTF8Encoding]::new($false))
    # Unreal MCP 서버를 user setting과 무관하게 시작시키는 필수 command-line flag입니다.
    $RequiredMcpStartFlag = '-ModelContextProtocolStartServer'
    # CarFight canonical Unreal MCP listener port를 고정하는 BAT 변수 선언입니다.
    $RequiredMcpPortVariable = 'set "CARFIGHT_MCP_PORT=8100"'
    # BAT 변수를 Editor command line에 전달하는 필수 port override flag입니다.
    $RequiredMcpPortFlag = '-ModelContextProtocolPort=%CARFIGHT_MCP_PORT%'

    if (-not $RunEditorBatText.Contains($RequiredMcpStartFlag)) {
        throw "Canonical RunEditor.bat에 Unreal MCP start flag가 없습니다: $RequiredMcpStartFlag"
    }

    if (-not $RunEditorBatText.Contains($RequiredMcpPortVariable)) {
        throw "Canonical RunEditor.bat에 CarFight MCP port 8100 계약이 없습니다: $RequiredMcpPortVariable"
    }

    if (-not $RunEditorBatText.Contains($RequiredMcpPortFlag)) {
        throw "Canonical RunEditor.bat에 Unreal MCP port override flag가 없습니다: $RequiredMcpPortFlag"
    }

    Write-Output 'status=validated'
    Write-Output 'canonical_launcher_present=true'
    Write-Output 'project_present=true'
    Write-Output 'mcp_start_flag_present=true'
    Write-Output 'mcp_port_8100_contract_present=true'
    Write-Output 'mcp_port_override_flag_present=true'
    Write-Output "wait_for_ready_enabled=$([bool]$WaitForReady)"
    exit 0
}

# 중복 실행을 막기 위해 시작 전에 확인한 정확한 CarFight Editor 프로세스 목록.
$ExistingEditors = @(Get-CarFightEditorProcess)

if ($ExistingEditors.Count -gt 0) {
    if (-not $WaitForReady) {
        Write-Output 'status=already_running'
        Write-Output "editor_process_count=$($ExistingEditors.Count)"
        exit 0
    }

    # 기존 Editor의 Ready 상태 확인을 제한할 종료 시각.
    $ExistingReadyDeadline = (Get-Date).AddSeconds([Math]::Max(1, $ReadyTimeoutSeconds))

    while ((Get-Date) -lt $ExistingReadyDeadline) {
        # Ready 대기 중 다시 관측한 exact CarFight Editor 프로세스 목록.
        $ExistingRunningEditors = @(Get-CarFightEditorProcess)

        if (Test-CarFightEditorReady -EditorProcesses $ExistingRunningEditors) {
            Write-Output 'status=already_running'
            Write-Output 'editor_ready=true'
            Write-Output 'port_8100_owned_by_editor=true'
            Write-Output "editor_process_count=$($ExistingRunningEditors.Count)"
            Write-Output "editor_process_id=$([int]$ExistingRunningEditors[0].ProcessId)"
            exit 0
        }

        Start-Sleep -Milliseconds 500
    }

    Write-Error "기존 CarFight Unreal Editor가 $ReadyTimeoutSeconds 초 안에 8100 Ready 상태가 되지 않았습니다."
    exit 3
}

# 공식 RunEditor.bat를 호출하는 짧은 cmd.exe 런처 프로세스.
$LauncherProcess = Start-Process -FilePath $env:ComSpec -ArgumentList @('/d', '/c', ('"{0}"' -f $RunEditorBat)) -WorkingDirectory $RepoRoot -PassThru

if (-not $LauncherProcess) {
    throw '공식 RunEditor.bat 실행을 시작하지 못했습니다.'
}

# Editor 시작 확인을 제한할 종료 시각.
$StartupDeadline = (Get-Date).AddSeconds([Math]::Max(1, $StartupTimeoutSeconds))

while ((Get-Date) -lt $StartupDeadline) {
    Start-Sleep -Milliseconds 500

    # 시작 대기 중 관측된 정확한 CarFight Editor 프로세스 목록.
    $RunningEditors = @(Get-CarFightEditorProcess)

    if ($RunningEditors.Count -gt 0) {
        if (-not $WaitForReady) {
            Write-Output 'status=started'
            Write-Output "editor_process_count=$($RunningEditors.Count)"
            Write-Output "launcher_exit_known=$($LauncherProcess.HasExited)"
            exit 0
        }

        # 새로 시작한 Editor의 Ready 상태 확인을 제한할 종료 시각.
        $ReadyDeadline = (Get-Date).AddSeconds([Math]::Max(1, $ReadyTimeoutSeconds))

        while ((Get-Date) -lt $ReadyDeadline) {
            # Ready 대기 중 다시 관측한 exact CarFight Editor 프로세스 목록.
            $ReadyEditors = @(Get-CarFightEditorProcess)

            if (Test-CarFightEditorReady -EditorProcesses $ReadyEditors) {
                Write-Output 'status=started'
                Write-Output 'editor_ready=true'
                Write-Output 'port_8100_owned_by_editor=true'
                Write-Output "editor_process_count=$($ReadyEditors.Count)"
                Write-Output "editor_process_id=$([int]$ReadyEditors[0].ProcessId)"
                Write-Output "launcher_exit_known=$($LauncherProcess.HasExited)"
                exit 0
            }

            Start-Sleep -Milliseconds 500
        }

        Write-Error "CarFight Unreal Editor 프로세스는 시작됐지만 $ReadyTimeoutSeconds 초 안에 8100 Ready 상태가 되지 않았습니다."
        exit 3
    }
}

Write-Error "CarFight Unreal Editor가 $StartupTimeoutSeconds 초 안에 나타나지 않았습니다."
exit 2
