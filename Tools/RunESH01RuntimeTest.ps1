# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunESH01RuntimeTest.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: ESH-01 same-Pawn Engine TorqueCurve runtime apply/restore exact1 focused runner입니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'
& $Runner -TestFilter 'CarFight.VehicleData.VD_P0_03.RuntimeApplyContract' -SuppressModelContextProtocolLog
exit $LASTEXITCODE
