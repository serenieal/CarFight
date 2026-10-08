# CarFight CCAS-P0-06 focused Automation runner.
# Version: v1.0.0
# Date: 2026-09-30
# Description: CF-FQ-058 CCAS-P0-06 planning/review exact7 Automation을 기존 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.0.0: SnapshotFingerprint/PlanningLifecycleReview/BulkSemanticPreflight/RetireReferenceGuard/TwoLayerStaleGuard/ApprovalBinding/NoDirectDAWrite exact7 최초 추가.
# Migration:
# - Product Apply/Save, persistent Workbook authoring, P0-07 authority cutover를 수행하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리합니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# CCAS-P0-06이 요구하는 exact planning/review Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.CCAS.CF_FQ_058.P0_06.SnapshotFingerprint',
    'CarFight.CCAS.CF_FQ_058.P0_06.PlanningLifecycleReview',
    'CarFight.CCAS.CF_FQ_058.P0_06.BulkSemanticPreflight',
    'CarFight.CCAS.CF_FQ_058.P0_06.RetireReferenceGuard',
    'CarFight.CCAS.CF_FQ_058.P0_06.TwoLayerStaleGuard',
    'CarFight.CCAS.CF_FQ_058.P0_06.ApprovalBinding',
    'CarFight.CCAS.CF_FQ_058.P0_06.NoDirectDAWrite'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
