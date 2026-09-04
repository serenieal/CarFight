# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunDAMP004DTests.ps1
# Version: v1.0.0
# Date: 2026-09-04
# Description: CF-FQ-045 DAM-P0-04D presentation correction focused Automation runner입니다.
# Changelog:
# - v1.0.0: Stable ID/Referencer/InventoryGeneration/Slate selection correction exact prefix를 실행합니다.
# Migration:
# - Product Asset mutation/save 없음.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 기존 unattended Automation 공용 runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# DAM-P0-04D presentation correction의 exact Automation prefix입니다.
$TestFilter = 'CarFight.DataManagement.CF_FQ_045.DAM_P0_04D'

& $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
if ($LASTEXITCODE -ne 0) {
    Write-Error "DAM-P0-04D focused Automation failed."
    exit $LASTEXITCODE
}

Write-Output 'DAM_P0_04D_FOCUSED_PASS=1'
exit 0
