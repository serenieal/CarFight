# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunDAMInventoryTests.ps1
# Version: v1.0.0
# Date: 2026-09-03
# Description: CF-FQ-045 DAM-P0-01 Inventory Core focused Automation runner입니다.
# Changelog:
# - v1.0.0: DAM-P0-01 exact Automation prefix를 existing DataAuthoring unattended runner로 실행합니다.
# Migration:
# - Product Asset/Config mutation과 Save를 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Existing CarFight Editor unattended Automation runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
# DAM-P0-01 focused Automation prefix입니다.
$TestFilter = 'CarFight.DataManagement.CF_FQ_045.DAM_P0_01'

& $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error "DAM-P0-01 focused Automation failed."
    exit $LASTEXITCODE
}

Write-Output 'DAM_P0_01_FOCUSED_PASS=1'
exit 0
