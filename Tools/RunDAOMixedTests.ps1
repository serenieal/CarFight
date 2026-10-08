# CarFight CF-FQ-051 DAO-P0-05 predecessor mixed exact4 regression runner.
# Version: v1.0.0
# Date: 2026-09-11
# Description: DDO-P0-04 exact3 activation 뒤 predecessor DAO-P0-05 mixed operational exact4를 한 Unreal Editor process에서 exact-list로 재검증합니다.
# Changelog:
# - v1.0.0: PathOwnerContract/MixedDurable/MixedDuplicate/MixedStale exact4 고정 wrapper를 추가했습니다.
# Migration:
# - 공용 RunDataAuthoringTests.ps1 exact-list mode만 재사용하며 broad filter 실행은 하지 않습니다.
# - predecessor DAO-P0-05 disposable fixture 계약을 그대로 사용하고 Product asset을 저장하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# DDO-P0-04 activation 뒤 보존해야 하는 predecessor DAO-P0-05 mixed regression exact4입니다.
$ExactTestNames = @(
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.PathOwnerContract',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDurable',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDuplicate',
    'CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedStale'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
# 공용 exact-list runner의 terminal exit code입니다.
$AutomationExitCode = $LASTEXITCODE
exit $AutomationExitCode
