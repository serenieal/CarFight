# CarFight CCAS-P0-01 focused Automation runner.
# Version: v1.0.0
# Date: 2026-09-29
# Description: CF-FQ-058 CCAS-P0-01 canonical Content Core exact8 Automation을 기존 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.0.0: CoreSchema/ValueStates/Collections/SemanticHash/FailClosed/ExternalReadOnly/Migration/ProviderRegistry exact8 최초 추가.
# Migration:
# - Product DataAsset을 저장하거나 변경하지 않습니다.
# - OpenXLSX concrete adapter/Workbook physical I/O를 실행하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# CCAS-P0-01이 요구하는 exact in-memory Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.CCAS.CF_FQ_058.P0_01.CoreSchema',
    'CarFight.CCAS.CF_FQ_058.P0_01.ValueStates',
    'CarFight.CCAS.CF_FQ_058.P0_01.Collections',
    'CarFight.CCAS.CF_FQ_058.P0_01.SemanticHash',
    'CarFight.CCAS.CF_FQ_058.P0_01.FailClosed',
    'CarFight.CCAS.CF_FQ_058.P0_01.ExternalReadOnly',
    'CarFight.CCAS.CF_FQ_058.P0_01.Migration',
    'CarFight.CCAS.CF_FQ_058.P0_01.ProviderRegistry'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
