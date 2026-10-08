# CarFight CCAS-P0-04 focused Automation runner.
# Version: v1.0.0
# Date: 2026-09-29
# Description: CF-FQ-058 CCAS-P0-04 Weapon + Vehicle Dual Consumer Pilot exact5 Automation을 기존 Data Authoring runner로 실행합니다.
# Changelog:
# - v1.0.0: SharedCoreDualConsumer/WeaponPersistedBinding/VehiclePersistedBinding/ProfileOverrideSources/SkeletalMeshPersistedScan exact5 최초 추가.
# Migration:
# - 실제 저장된 대표 Weapon/Turret/Vehicle/SkeletalMesh를 read-only로 검증합니다.
# - Product Apply/Save, Workbook persistent write, writer activation, P0-07 cutover를 수행하지 않습니다.

[CmdletBinding()]
param()

# PowerShell 오류를 즉시 실패로 처리하는 실행 정책입니다.
$ErrorActionPreference = 'Stop'

# 공용 Data Authoring exact-list Automation runner 경로입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# CCAS-P0-04이 요구하는 exact persisted-content Automation 목록입니다.
$ExactTestNames = @(
    'CarFight.CCAS.CF_FQ_058.P0_04.SharedCoreDualConsumer',
    'CarFight.CCAS.CF_FQ_058.P0_04.WeaponPersistedBinding',
    'CarFight.CCAS.CF_FQ_058.P0_04.VehiclePersistedBinding',
    'CarFight.CCAS.CF_FQ_058.P0_04.ProfileOverrideSources',
    'CarFight.CCAS.CF_FQ_058.P0_04.SkeletalMeshPersistedScan'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
