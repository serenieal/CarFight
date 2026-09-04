# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunDAMManagerInfoTests.ps1
# Version: v1.0.0
# Date: 2026-09-03
# Description: CF-FQ-045 DAM-P0-04C Information Hierarchy + User Semantics focused Automation runner입니다.
# Changelog:
# - v1.0.0: DAM-P0-04C exact Automation prefix를 existing unattended runner로 실행합니다.
# Migration:
# - Product Asset mutation/save 없음.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
$TestFilter = 'CarFight.DataManagement.CF_FQ_045.DAM_P0_04C'

& $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error "DAM-P0-04C focused Automation failed."
    exit $LASTEXITCODE
}

Write-Output 'DAM_P0_04C_FOCUSED_PASS=1'
exit 0
