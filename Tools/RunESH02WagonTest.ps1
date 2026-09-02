# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunESH02WagonTest.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 ESH-02 actual Wagon Engine Curve PhysicsDraft v3 mutation0 dry-run runner입니다.
# Changelog:
# - v1.0.0: actual persistent Wagon Recipe/Evidence/Profile/Target + Saved PhysicsDraft v3 exact single Automation 실행.
# Migration:
# - Product Asset/Config mutation과 Save를 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
$TestFilter = 'CarFight.DataAuthoring.CF_FQ_040.ESH_02.WagonEngineCurveDraft'

& $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error "ESH-02 actual Wagon Engine Curve dry-run failed: $TestFilter"
    exit $LASTEXITCODE
}

Write-Output "ESH02_WAGON_ENGINE_CURVE_PASS=$TestFilter"
exit 0
