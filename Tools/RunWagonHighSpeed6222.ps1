# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonHighSpeed6222.ps1
# Version: v1.0.2
# Date: 2026-09-02
# Description: CF-FQ-040 ESH-03 raw ChangeUpRPM=6222 이론값을 current persisted 5500 Target과 Product mutation 없이 비교하는 transient wrapper입니다.
# Changelog:
# - v1.0.2: USER-approved 5500 Target Apply 뒤 exact TargetHash 0e5b...에 재binding하고 label을 post-Apply raw6222 comparison으로 교정.
# - v1.0.1: 5500 comparison lane 추가 후 역할 설명을 A/B/C raw diagnostic comparison으로 교정. 실행 동작은 변경 없음.
# - v1.0.0: current persisted Wagon TargetHash를 유지한 채 generic ESH-04 runner의 transient ChangeUpRPMOverride=6222만 활성화.
# Migration:
# - DA_Vehicle_Wagon, Recipe, Profile, Map, Config를 저장하거나 수정하지 않습니다.
# - 6222 RPM은 후보 검증값일 뿐 Product 승인값이 아닙니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Generic ESH-04 high-speed runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunHighSpeedBench.ps1'

# Persisted actual Wagon VehicleData exact object path입니다.
$VehicleDataPath = '/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon'

# USER-approved ESH-03 ChangeUpRPM 5500 DefinitionApply 뒤 exact Target DefinitionHash입니다.
$ExpectedTargetDefinitionHash = '0e5b48e8dcd39deba441da9237218be6'

# ESH-03 fixed-common candidate ChangeUpRPM입니다.
$ChangeUpRPMOverride = 6222.0

# post-Apply raw 6222 comparison evidence 식별 label입니다.
$Label = 'Wagon-V60CC-8AT-ESH03-PostApply-Raw6222'

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
