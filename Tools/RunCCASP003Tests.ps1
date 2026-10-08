# CarFight CCAS-P0-03 focused Automation runner.
# Version: v1.0.0
# Date: 2026-09-29
# Description: CF-FQ-058 CCAS-P0-03 Resource Catalog + Semantic Binding exact8 Automation을 기존 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.0.0: StableResourceId/PickerRegistry/AssetValidation/MeshCapabilities/SemanticRoleBinding/ProfileOverride/RequiredOptionalPolicy/WorkbookResourceValidation exact8 최초 추가.
# Migration:
# - Product DataAsset을 저장하거나 변경하지 않습니다.
# - /Engine/BasicShapes/Cube.Cube와 in-memory registries만 read-only fixture로 사용합니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# CCAS-P0-03이 요구하는 exact read-only Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.CCAS.CF_FQ_058.P0_03.StableResourceId',
    'CarFight.CCAS.CF_FQ_058.P0_03.PickerRegistry',
    'CarFight.CCAS.CF_FQ_058.P0_03.AssetValidation',
    'CarFight.CCAS.CF_FQ_058.P0_03.MeshCapabilities',
    'CarFight.CCAS.CF_FQ_058.P0_03.SemanticRoleBinding',
    'CarFight.CCAS.CF_FQ_058.P0_03.ProfileOverride',
    'CarFight.CCAS.CF_FQ_058.P0_03.RequiredOptionalPolicy',
    'CarFight.CCAS.CF_FQ_058.P0_03.WorkbookResourceValidation'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
