# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunDAMManagerTests.ps1
# Version: v1.0.0
# Date: 2026-09-03
# Description: CF-FQ-045 DAM-P0-03 Manager UI + On-demand Detail focused Automation runner입니다.
# Changelog:
# - v1.0.0: DAM-P0-03 exact Automation prefix를 existing unattended runner로 실행합니다.
# Migration:
# - Product Asset mutation/save 없음.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
$TestFilter = 'CarFight.DataManagement.CF_FQ_045.DAM_P0_03'

& $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error "DAM-P0-03 focused Automation failed."
    exit $LASTEXITCODE
}

Write-Output 'DAM_P0_03_FOCUSED_PASS=1'
exit 0
