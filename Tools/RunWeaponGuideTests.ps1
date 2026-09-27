# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWeaponGuideTests.ps1
# Version: v1.2.0
# Date: 2026-09-18
# Description: CF-FQ-055 Weapon Equipment Authoring Guide의 WEA-P0-01~04 focused Automation exact8 runner입니다.
# Changelog:
# - v1.2.0: WEA-P0-04 completion summary/fingerprint/navigation-only handoff contract 테스트를 추가해 exact8로 전진했습니다.
# - v1.1.0: WEA-P0-03 disposable exact5 durable graph Create와 partial durable same-session recovery 2종을 추가해 exact7로 전진했습니다.
# - v1.0.0: Stable Step/Template/Naming/CapabilityValidation과 WEA-P0-02 Damage/Ammo provider Draft mapping exact5를 same-process로 실행합니다.
# Migration:
# - Product Asset Save를 수행하지 않습니다. P0-03 durable 테스트는 /Game/CarFight/Tests/WeaponGuide/P003만 저장 후 residue exact0으로 정리합니다.
# - 공용 RunDataAuthoringTests.ps1 exact-list runner를 재사용하며 broad CarFight.DataAuthoring regression은 실행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Existing same-process exact-list Automation runner 경로입니다.
$RunnerPath = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

if (-not (Test-Path -LiteralPath $RunnerPath -PathType Leaf)) {
    throw "공용 Automation runner를 찾을 수 없습니다: $RunnerPath"
}

# CF-FQ-055 WEA-P0-01~04가 현재 소유하는 focused Automation exact8입니다.
$WeaponGuideTests = @(
    'CarFight.WeaponAuthoring.P001.VisibleSteps',
    'CarFight.WeaponAuthoring.P001.TemplateTopology',
    'CarFight.WeaponAuthoring.P001.Naming',
    'CarFight.WeaponAuthoring.P001.CapabilityValidation',
    'CarFight.WeaponAuthoring.P002.ProviderDraftMapping',
    'CarFight.WeaponAuthoring.P003.DurableCreateGraph',
    'CarFight.WeaponAuthoring.P003.PartialRecovery',
    'CarFight.WeaponAuthoring.P004.CompletionHandoff'
)

Write-Output 'WEAPON_GUIDE_AUTOMATION_TOPOLOGY=same_process_exact_list'
Write-Output ("WEAPON_GUIDE_AUTOMATION_REQUESTED={0}" -f $WeaponGuideTests.Count)

# 공용 runner에서 exact8을 하나의 공식 UE 5.8 Editor process로 실행합니다.
& $RunnerPath -ExactTestNames $WeaponGuideTests

# 공용 runner의 terminal exit code입니다.
$RunnerExitCode = $LASTEXITCODE

if ($RunnerExitCode -ne 0) {
    Write-Error "WEA focused Automation failed. exit_code=$RunnerExitCode"
    exit $RunnerExitCode
}

Write-Output 'WEAPON_GUIDE_FOCUSED_AUTOMATION=PASS'
exit 0
