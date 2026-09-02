# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonHighSpeed5500.ps1
# Version: v1.1.1
# Date: 2026-09-02
# Description: CF-FQ-040 ESH-03 historical A/B/C의 5500 lane을 current post-Apply Target에서도 재현할 수 있는 transient verification wrapper입니다.
# Changelog:
# - v1.1.1: 5500이 이미 USER-approved persisted Product 값인 current 상태에 맞춰 stale Migration 설명을 교정. 실행 동작은 변경 없음.
# - v1.1.0: persisted Target이 5500으로 승인 적용된 뒤 exact TargetHash 0e5b...에 재binding. override=5500은 current persisted value와 동일한 verification lane입니다.
# - v1.0.0: current persisted Wagon TargetHash를 유지한 채 generic ESH-04 runner의 transient ChangeUpRPMOverride=5500만 활성화.
# Migration:
# - DA_Vehicle_Wagon, Recipe, Profile, Map, Config를 저장하거나 수정하지 않습니다.
# - 5500 RPM은 현재 USER-approved persisted Product 값이며, 이 wrapper의 override는 같은 값을 transient verification으로 재현할 뿐 Product를 다시 수정하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Generic ESH-04 high-speed runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunHighSpeedBench.ps1'

# Persisted actual Wagon VehicleData exact object path입니다.
$VehicleDataPath = '/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon'

# USER-approved ESH-03 ChangeUpRPM 5500 DefinitionApply 뒤 exact Target DefinitionHash입니다.
$ExpectedTargetDefinitionHash = '0e5b48e8dcd39deba441da9237218be6'

# Historical A/B/C 5500 lane을 재현하는 transient override입니다. current persisted 값과 동일합니다.
$ChangeUpRPMOverride = 5500.0

# transient A/B/C evidence 식별 label입니다.
$Label = 'Wagon-V60CC-8AT-ESH03-5500ABC'

if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "ESH-04 high-speed runner를 찾을 수 없습니다: $Runner"
}

# Generic runner에 전달할 exact bounded parameters입니다.
$RunnerParams = @{
    VehicleDataPath = $VehicleDataPath
    Label = $Label
    ExpectedTargetDefinitionHash = $ExpectedTargetDefinitionHash
    ChangeUpRPMOverride = $ChangeUpRPMOverride
}

& $Runner @RunnerParams
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

exit 0
