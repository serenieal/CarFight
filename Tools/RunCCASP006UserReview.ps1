# CarFight CCAS-P0-06 USER Acceptance review runner.
# Version: v1.0.0
# Date: 2026-09-30
# Description: USER Accepted Weapon Roster exact8을 disposable AI Change Proposal -> Review Package 경로로 실행하고 review marker를 출력합니다.
# Changelog:
# - v1.0.0: P0-06 USER review exact1 runner 최초 추가.
# Migration:
# - Product Apply/Save, persistent Workbook authoring, DataAsset mutation, P0-07 authority cutover를 수행하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리합니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# USER Acceptance용 exact test name입니다.
$ExactTestName = 'CarFight.CCAS.CF_FQ_058.P0_06_USER.WeaponRosterProposalReview'

# 공용 runner가 생성하는 Automation log 경로입니다.
$AutomationLogPath = Join-Path (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path 'UE\Saved\Logs\CFDataAuthoringAutomation.log'

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames @($ExactTestName) -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

if (-not (Test-Path -LiteralPath $AutomationLogPath -PathType Leaf)) {
    throw "USER review marker를 읽을 Automation log를 찾을 수 없습니다: $AutomationLogPath"
}

# UE log를 UTF-8로 읽습니다.
$AutomationLogText = [System.IO.File]::ReadAllText(
    $AutomationLogPath,
    [System.Text.UTF8Encoding]::new($false))

# 사람이 검수할 P0-06 USER review marker만 bounded하게 추출합니다.
$ReviewLines = @(
    $AutomationLogText -split "`r?`n" |
        Where-Object { $_ -match 'CCAS_USER_REVIEW\|' } |
        ForEach-Object {
            $MarkerIndex = $_.IndexOf('CCAS_USER_REVIEW|')
            if ($MarkerIndex -ge 0) {
                $_.Substring($MarkerIndex)
            }
        }
)

if ($ReviewLines.Count -eq 0) {
    throw 'CCAS USER review marker가 생성되지 않았습니다.'
}

Write-Output "USER_REVIEW_MARKER_COUNT=$($ReviewLines.Count)"
$ReviewLines | ForEach-Object { Write-Output $_ }
exit 0
