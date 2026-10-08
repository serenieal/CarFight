# CarFight CCAS-P0-08 affected PIE Automation runner.
# Version: v1.0.0
# Date: 2026-10-07
# Description: Production Publication Catalog로 전환된 RuntimeApply PIE E2E exact2를 실행합니다.
# Changelog:
# - v1.0.0: RTA-P0-05 PIEE2E + Wagon Mount Equipment PIE exact2 최초 추가.
# Migration:
# - persisted Product/Vehicle/Map/Workbook을 저장·수정하지 않습니다.
# - 공용 Data Authoring exact-list runner를 그대로 재사용합니다.

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

$ExactTestNames = @(
    'CarFight.RuntimeApply.RTA_P0_05.PIEE2E',
    'CarFight.RuntimeApply.CF_FQ_047.WagonMountEquipmentPIE'
)

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

& $DataAuthoringRunner -ExactTestNames $ExactTestNames -SuppressModelContextProtocolLog
exit $LASTEXITCODE
