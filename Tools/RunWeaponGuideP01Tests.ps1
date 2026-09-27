# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWeaponGuideP01Tests.ps1
# Version: v1.0.0
# Date: 2026-09-18
# Description: CF-FQ-055 WEA-P0-01 Weapon Guide exact5 focused Automation runner입니다.
# Changelog:
# - v1.0.0: Stable Step, Template, Naming, Capability, ProviderBoundary exact5를 existing DataAuthoring runner의 same-process exact-list mode로 실행합니다.
# Migration:
# - Product Asset/Config mutation과 Save를 수행하지 않습니다.
# - 공용 Automation 실행 계약은 RunDataAuthoringTests.ps1을 재사용합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Existing same-process exact-list Automation runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# WEA-P0-01이 소유하는 exact focused Automation 목록입니다.
$ExactTests = @(
    'CarFight.WeaponAuthoring.P001.VisibleSteps',
    'CarFight.WeaponAuthoring.P001.TemplateTopology',
    'CarFight.WeaponAuthoring.P001.Naming',
    'CarFight.WeaponAuthoring.P001.CapabilityValidation',
    'CarFight.WeaponAuthoring.P001.ProviderBoundary'
)

& $Runner -ExactTestNames $ExactTests -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error 'WEA-P0-01 Weapon Guide focused Automation failed.'
    exit $LASTEXITCODE
}

Write-Output "WEAPON_GUIDE_P001_EXACT_PASS_COUNT=$($ExactTests.Count)"
exit 0
