# CarFight Unreal Editor Browser 종료 래퍼.
# Version: v1.1.0
# Changelog:
# - v1.1.0: 검증 전용 모드와 명시적 미저장 폐기 모드를 추가했다. 기본 종료는 저장 0 / 강제 종료 0이다.
# - v1.0.0: CarFight 프로젝트만 대상으로 하는 bounded graceful close를 추가했다.
# Migration:
# - 기본 호출은 저장 명령이나 강제 종료를 사용하지 않는다. -DiscardUnsaved는 사용자가 미저장 변경 폐기를 명시적으로 허용한 경우에만 사용한다.

[CmdletBinding()]
param(
    # 정상 종료 요청 뒤 프로세스 종료를 기다리는 최대 초 수.
    [int]$ShutdownTimeoutSeconds = 30,
    # Unreal Editor를 종료하지 않고 경로와 실행 계약만 검증한다.
    [switch]$ValidateOnly,
    # 정확한 CarFight Editor를 강제 종료해 미저장 상태를 저장하지 않고 폐기한다.
    [switch]$DiscardUnsaved,
    # 실수로 미저장 폐기 모드를 호출하지 못하게 하는 고정 승인 토큰.
    [string]$DiscardApproval = ''
)

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책.
$ErrorActionPreference = 'Stop'

# 이 스크립트 위치에서 계산한 CarFight 저장소 루트.
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 정확한 CarFight Editor 프로세스를 식별할 공식 프로젝트 경로.
$ProjectPath = Join-Path $RepoRoot 'UE\CarFight_Re.uproject'

# 명시적 미저장 폐기 호출에서만 허용하는 고정 승인 토큰.
$RequiredDiscardApproval = 'DISCARD_UNSAVED_CONFIRMED'

# CarFight 프로젝트를 실행 중인 UnrealEditor.exe만 반환한다.
function Get-CarFightEditorProcess {
    # 프로세스 명령줄과 비교할 정규화된 절대 프로젝트 경로.
    $NormalizedProject = [IO.Path]::GetFullPath($ProjectPath)

    return @(Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object {
        $_.CommandLine -and ($_.CommandLine.IndexOf($NormalizedProject, [StringComparison]::OrdinalIgnoreCase) -ge 0)
    })
}

if (-not (Test-Path -LiteralPath $ProjectPath -PathType Leaf)) {
    throw "CarFight 프로젝트를 찾을 수 없습니다: $ProjectPath"
}

if ($DiscardUnsaved -and ($DiscardApproval -ne $RequiredDiscardApproval)) {
    Write-Error '미저장 폐기는 명시적 승인 토큰이 필요합니다. 강제 종료를 수행하지 않았습니다.'
    exit 2
}

if ($ValidateOnly) {
    Write-Output 'status=validated'
    Write-Output 'project_present=true'
    Write-Output "discard_unsaved_enabled=$([bool]$DiscardUnsaved)"
    exit 0
}

# 이번 종료 요청의 대상이 되는 정확한 CarFight Editor 프로세스 목록.
$TargetEditors = @(Get-CarFightEditorProcess)

if ($TargetEditors.Count -eq 0) {
    Write-Output 'status=already_stopped'
    Write-Output 'editor_process_count=0'
    exit 0
}

if ($TargetEditors.Count -ne 1) {
    Write-Error "CarFight Editor 프로세스가 $($TargetEditors.Count)개라 종료를 거부합니다. 정확히 1개여야 합니다."
    exit 3
}

# 종료할 단일 CarFight Editor의 프로세스 ID.
$TargetPid = [int]$TargetEditors[0].ProcessId

# 정상 창 닫기 또는 명시적 폐기에 사용할 프로세스 핸들.
$EditorProcess = Get-Process -Id $TargetPid -ErrorAction Stop

if ($DiscardUnsaved) {
    Stop-Process -Id $TargetPid -Force -ErrorAction Stop
    Start-Sleep -Milliseconds 500

    # 명시적 강제 폐기 뒤 남은 정확한 CarFight Editor 프로세스 목록.
    $EditorsAfterDiscard = @(Get-CarFightEditorProcess)

    if ($EditorsAfterDiscard.Count -eq 0) {
        Write-Output 'status=stopped_discarded_unsaved'
        Write-Output 'save_requested=false'
        Write-Output 'force_kill_used=true'
        exit 0
    }

    Write-Error '미저장 폐기를 요청했지만 정확한 CarFight Editor 프로세스가 아직 남아 있습니다.'
    exit 6
}

# 저장 명령 없이 정상 WM_CLOSE를 요청한 결과.
$CloseRequested = $EditorProcess.CloseMainWindow()

if (-not $CloseRequested) {
    Write-Error 'Unreal Editor가 정상 종료 요청을 받지 않았습니다. 저장 또는 강제 종료는 수행하지 않았습니다.'
    exit 4
}

# 정상 종료 확인을 제한할 종료 시각.
$ShutdownDeadline = (Get-Date).AddSeconds([Math]::Max(1, $ShutdownTimeoutSeconds))

while ((Get-Date) -lt $ShutdownDeadline) {
    Start-Sleep -Milliseconds 500

    # 정상 종료 대기 중 남아 있는 정확한 CarFight Editor 프로세스 목록.
    $RemainingEditors = @(Get-CarFightEditorProcess)

    if ($RemainingEditors.Count -eq 0) {
        Write-Output 'status=stopped'
        Write-Output 'save_requested=false'
        Write-Output 'force_kill_used=false'
        exit 0
    }
}

Write-Error '정상 종료 제한 시간 뒤에도 Editor가 실행 중입니다. 저장/폐기 모달 또는 사용자 미저장 작업 가능성이 있으므로 저장이나 강제 종료 없이 중단합니다.'
exit 5
