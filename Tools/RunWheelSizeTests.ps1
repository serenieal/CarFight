# Copyright (c) CarFight. All Rights Reserved.
#
# File: RunWheelSizeTests.ps1
# Version: v1.6.1
# Date: 2026-08-31
# Description: CF-FQ-040 WSA focused Automation과 직접 affected Builder/Validator exact tests만 순차 실행합니다.
# Scope: WSA-P0-01~07 + Migration deferred guard + BuilderShell/Gameplay/ProfileCommit/ValidatorContract exact filters only. Broad regression/PIE/Product Asset Save를 수행하지 않습니다.
# Changelog:
# - v1.6.1: UE Experimental ModelContextProtocol의 비동기 Error 로그가 WSA와 무관한 test를 오염시키지 않도록 generic runner의 opt-in suppression을 WSA suite에서만 사용.
# - v1.6.0: WSA-P0-07 RightFallbackOrientationSpin exact regression을 추가해 Right fallback orientation/spin remediation을 검증.
# - v1.5.0: WSA-P0-05 Recipe-only defer가 exact core Profile/WheelClass blocker에서만 열리는 negative regression을 추가.
# - v1.4.0: WSA-P0-05 SoftObject→Object canonical reference R15 hash roundtrip regression을 추가.
# - v1.3.0: WSA-P0-04 affected BuilderShell, GameplayGuidance, BuilderProfileCommit, VehicleData ValidatorContract exact tests를 추가.
# - v1.2.0: WSA-P0-03 RuntimeVisualFallback focused test를 추가.
# - v1.1.0: WSA-P0-02 SocketScaleDerived Resolver focused test를 추가.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Runner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

if (-not (Test-Path -LiteralPath $Runner -PathType Leaf)) {
    throw "RunDataAuthoringTests.ps1을 찾을 수 없습니다: $Runner"
}

$Tests = @(
    'CarFight.DataAuthoring.CF_FQ_040.WSA_P0_01.Foundation.Registry132',
    'CarFight.Vehicle.WheelSize.WSA_P0_01.SchemaUtilityLegacy',
    'CarFight.DataAuthoring.CF_FQ_040.WSA_P0_02.Resolver.SocketScaleDerived',
    'CarFight.DataAuthoring.CF_FQ_040.WSA_P0_05.Resolver.ObjectReferenceRoundTrip',
    'CarFight.DataAuthoring.CF_FQ_040.WSA_P0_05.Migration.DeferredApplyGuard',
    'CarFight.Vehicle.WheelSize.WSA_P0_03.RuntimeVisualFallback',
    'CarFight.Vehicle.WheelSize.WSA_P0_07.RightFallbackOrientationSpin',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderProfileCommit',
    'CarFight.VehicleData.VD_P0_01.ValidatorContract'
)

foreach ($TestFilter in $Tests) {
    Write-Output "WSA_TEST_BEGIN=$TestFilter"
    & $Runner -TestFilter $TestFilter -SuppressModelContextProtocolLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "WSA focused Automation failed: $TestFilter"
        exit $LASTEXITCODE
    }
    Write-Output "WSA_TEST_PASS=$TestFilter"
}

Write-Output 'WSA_FOCUSED_AUTOMATION=PASS'
exit 0
