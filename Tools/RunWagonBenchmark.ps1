# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWagonBenchmark.ps1
# Version: v1.1.0
# Date: 2026-09-02
# Description: CF-FQ-040 actual Wagon ESH-06 Step 8 current saved-target technical driving benchmark wrapper입니다.
# Changelog:
# - v1.1.0: USER-approved fixed-common 5500 DefinitionApply 뒤 current TargetHash 0e5b...에 exact binding하고 ESH-06 USER Driving 전 fresh Step 8 RunId 생성 용도로 갱신.
# - v1.0.0: saved DA_Vehicle_Wagon + exact post-Apply DefinitionHash를 existing RunBuilderBench.ps1에 binding.
# Migration:
# - Product Asset/Map/Config를 수정하거나 저장하지 않습니다.
# - benchmark 결과는 existing VehicleBuilderBenchmarkResult.json 계약을 그대로 사용합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Existing generic Builder benchmark runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunBuilderBench.ps1'

# Persisted actual Wagon VehicleData exact object path입니다.
$VehicleDataPath = '/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon'

# USER-approved ESH-03 fixed-common 5500 DefinitionApply 뒤 current Target DefinitionHash입니다.
$ExpectedTargetDefinitionHash = '0e5b48e8dcd39deba441da9237218be6'

# ESH-06 USER Driving 전 fresh Step 8 result 식별용 bounded label입니다.
$Label = 'Wagon-V60CC-8AT-ESH06-Step8'

if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "Builder benchmark runner를 찾을 수 없습니다: $Runner"
}

# Generic benchmark runner에 전달할 exact bounded parameters입니다.
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
