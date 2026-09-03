# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunCatalogPromoTests.ps1
# Version: v1.0.0
# Date: 2026-09-02
# Description: CF-FQ-044 VRCP-P0-02 Editor Promotion Service focused Automation runner입니다.
# Changelog:
# - v1.0.0: Promotion Service exact test prefix를 existing DataAuthoring runner로 실행합니다.
# Migration:
# - Product Asset/Config mutation과 Save를 수행하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Existing DataAuthoring unattended Automation runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
# VRCP-P0-02 focused Automation prefix입니다.
$TestFilter = 'CarFight.DataAuthoring.CF_FQ_044.VRCP_P0_02'

& $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error "VRCP-P0-02 focused Automation failed."
    exit $LASTEXITCODE
}

Write-Output 'VRCP_P0_02_FOCUSED_PASS=1'
exit 0
