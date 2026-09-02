# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunESH04MapAudit.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 ESH-04 dedicated M_VehicleBenchmark persisted geometry authority inventory exact1 runner입니다.
# Changelog:
# - v1.0.0: BenchmarkMapInventory Automation exact1 실행을 추가.
# Migration:
# - Product Asset/Map/Config를 수정하거나 저장하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 공용 Automation runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# ESH-04 dedicated map inventory exact test path입니다.
$TestFilter = 'CarFight.VehicleBuilder.CF_FQ_040.ESH_04.BenchmarkMapInventory'

Write-Output "ESH04_MAP_AUDIT_BEGIN=$TestFilter"
& $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error "ESH-04 benchmark map inventory failed: $TestFilter"
    exit $LASTEXITCODE
}

Write-Output "ESH04_MAP_AUDIT_PASS=$TestFilter"
exit 0
