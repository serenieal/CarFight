# CarFight CCAS-P0-02 focused Automation runner.
# Version: v1.1.0
# Date: 2026-09-29
# Description: CF-FQ-058 CCAS-P0-02 Generic Compiler Core exact12 Automation을 기존 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.1.0: Mid-review 교정 회귀 CurrentRetireImpact/FingerprintGuard/SchemaCompatibility/SnapshotConsistency exact4 추가.
# - v1.0.0: WorkbookReader/ProviderValidation/DiffDrift/ReferenceGraph/ImpactAnalysis/BatchNoApply/FailClosed/ScaleBenchmark exact8 최초 추가.
# Migration:
# - Product DataAsset을 저장하거나 변경하지 않습니다.
# - ScaleBenchmark는 disposable in-memory 10k content + 100k child fixture만 사용합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# CCAS-P0-02이 요구하는 exact read-only Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.CCAS.CF_FQ_058.P0_02.WorkbookReader',
    'CarFight.CCAS.CF_FQ_058.P0_02.ProviderValidation',
    'CarFight.CCAS.CF_FQ_058.P0_02.DiffDrift',
    'CarFight.CCAS.CF_FQ_058.P0_02.ReferenceGraph',
    'CarFight.CCAS.CF_FQ_058.P0_02.ImpactAnalysis',
    'CarFight.CCAS.CF_FQ_058.P0_02.BatchNoApply',
    'CarFight.CCAS.CF_FQ_058.P0_02.FailClosed',
    'CarFight.CCAS.CF_FQ_058.P0_02.ScaleBenchmark',
    'CarFight.CCAS.CF_FQ_058.P0_02.CurrentRetireImpact',
    'CarFight.CCAS.CF_FQ_058.P0_02.FingerprintGuard',
    'CarFight.CCAS.CF_FQ_058.P0_02.SchemaCompatibility',
    'CarFight.CCAS.CF_FQ_058.P0_02.SnapshotConsistency'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
