# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunESH01Tests.ps1
# Version: v1.0.0
# Date: 2026-09-01
# Description: CF-FQ-040 ESH-01 Engine TorqueCurve schema/Resolver/Runtime focused Automation exact3 runner입니다.
# Changelog:
# - v1.0.0: Registry134, EngineTorqueCurve Resolver, RuntimeApplyContract 세 테스트를 기존 RunDataAuthoringTests.ps1로 순차 실행.
# Migration:
# - Product Asset/Config를 저장하지 않습니다.
# - Experimental MCP 비동기 로그는 focused test와 무관하므로 기존 runner opt-in suppression을 사용합니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 기존 검증된 targeted Automation runner입니다.
$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

# ESH-01 exact focused test 목록입니다.
$TestFilters = @(
    'CarFight.DataAuthoring.CF_FQ_040.WSA_P0_01.Foundation.Registry132',
    'CarFight.DataAuthoring.CF_FQ_040.ESH_01.Resolver.EngineTorqueCurve',
    'CarFight.VehicleData.VD_P0_03.RuntimeApplyContract'
)

foreach ($TestFilter in $TestFilters) {
    Write-Output "ESH01_TEST_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "ESH-01 focused Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "ESH01_TEST_PASS=$TestFilter"
}

Write-Output "ESH01_EXACT_PASS_COUNT=$($TestFilters.Count)"
exit 0
