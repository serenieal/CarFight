# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonHighSpeed.ps1
# Version: v1.1.0
# Date: 2026-09-02
# Description: CF-FQ-040 actual Wagon ESH-05 post-5500 DefinitionApply persisted high-speed exact wrapper입니다.
# Changelog:
# - v1.1.0: USER-approved ChangeUpRPM 5500 Target Apply 뒤 exact TargetHash 0e5b...로 binding을 갱신하고 ESH-05 persisted retest 역할로 승격.
# - v1.0.0: current Wagon VehicleData + exact post-Engine-Curve Target hash를 RunHighSpeedBench.ps1에 binding.
# Migration:
# - Product Asset/Map/Config를 수정하거나 저장하지 않습니다.
# - transient override 없이 persisted Target의 ChangeUpRPM/ChangeDownRPM을 그대로 관측합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Generic ESH-04 high-speed runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunHighSpeedBench.ps1'

# Persisted actual Wagon VehicleData exact object path입니다.
$VehicleDataPath = '/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon'

# USER-approved ESH-03 ChangeUpRPM 5500 DefinitionApply 뒤 exact Target DefinitionHash입니다.
$ExpectedTargetDefinitionHash = '0e5b48e8dcd39deba441da9237218be6'

# Wagon ESH-05 persisted retest evidence 식별 label입니다.
$Label = 'Wagon-V60CC-8AT-ESH05-Persisted5500'

if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "ESH-04 high-speed runner를 찾을 수 없습니다: $Runner"
}

# Generic runner에 전달할 exact bounded parameters입니다.
$RunnerParams = @{
    VehicleDataPath = $VehicleDataPath
    Label = $Label
    ExpectedTargetDefinitionHash = $ExpectedTargetDefinitionHash
}

& $Runner @RunnerParams
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

exit 0
