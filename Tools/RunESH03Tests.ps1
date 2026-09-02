# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunESH03Tests.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 ESH-03 WheelTorqueCrossoverShift generic + actual Wagon mutation0 focused runner입니다.
# Changelog:
# - v1.0.0: generic deterministic crossover와 actual Wagon persisted Engine Curve/8AT/WSA diagnostic exact2 실행.
# Migration:
# - Product Asset/Profile/Recipe/Target mutation과 Save를 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
$TestFilters = @(
    'CarFight.DataAuthoring.CF_FQ_040.ESH_03.WheelTorqueCrossoverGeneric',
    'CarFight.DataAuthoring.CF_FQ_040.ESH_03.WagonShiftDiagnostic'
)

foreach ($TestFilter in $TestFilters) {
    Write-Output "ESH03_TEST_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "ESH-03 focused Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "ESH03_TEST_PASS=$TestFilter"
}

Write-Output "ESH03_EXACT_PASS_COUNT=$($TestFilters.Count)"
exit 0
