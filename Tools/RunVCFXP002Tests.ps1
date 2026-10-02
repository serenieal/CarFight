# CarFight CF-FQ-057 VCFX-P0-02 focused Automation runner.
# Version: v1.3.0
# Date: 2026-09-29
# Description: Camera FX minimum implementation, P0-03 Lateral/Overspeed/Rear Kick correction과 직접 영향받은 Vehicle Authoring / TargetSelect 계약 exact8을 한 UE 5.8 Editor process에서 검증합니다.
# Changelog:
# - v1.3.0: 기존 exact8 set을 유지하면서 NormalizedMotionMath/FrozenDefaults가 overspeed headroom과 AccelerationRearKickScale까지 검증하도록 범위를 갱신.
# - v1.2.0: P0-03 actual body-motion + free-look Lateral Presentation pure math test를 추가해 exact8 focused set으로 확장.
# - v1.1.0: production normalized motion math/fail-safe direct test를 추가해 exact7 focused set으로 확장.
# - v1.0.0: VCFX frozen defaults, Camera typed Resolver, Registry coverage, 기존 Sensor precedence, Target candidate/search diagnostics exact6 focused set 추가.
# Migration:
# - Product Asset을 저장하지 않습니다.
# - 공용 실행 엔진은 기존 RunDataAuthoringTests.ps1 exact-list mode를 재사용합니다.

$ErrorActionPreference = 'Stop'

# 기존 same-process exact-list Automation runner입니다.
$DataAuthoringRunner = Join-Path $PSScriptRoot 'RunDataAuthoringTests.ps1'

if (-not (Test-Path -LiteralPath $DataAuthoringRunner -PathType Leaf)) {
    throw "공용 Data Authoring Automation runner를 찾을 수 없습니다: $DataAuthoringRunner"
}

# VCFX-P0-02와 직접 영향받은 계약만 포함한 exact focused test 목록입니다.
$FocusedTests = @(
    'CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_02.FrozenDefaults',
    'CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_02.NormalizedMotionMath',
    'CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_03.LateralPresentationMath',
    'CarFight.DataAuthoring.CF_FQ_057.VCFX_P0_02.Resolver.CameraIntentPrecedence',
    'CarFight.DataAuthoring.CF_FQ_040.WSA_P0_01.Foundation.Registry132',
    'CarFight.DataAuthoring.DAUTH_P0_08.Resolver.SensorIntentPrecedence',
    'CarFight.TargetSelect.TS_P0_03.CandidateRanking',
    'CarFight.TargetSelect.TS_P0_08.SearchDiagnostics'
)

& $DataAuthoringRunner -ExactTestNames $FocusedTests -SuppressModelContextProtocolLog
$AutomationExitCode = $LASTEXITCODE

if ($AutomationExitCode -ne 0) {
    Write-Error "VCFX-P0-02 focused Automation failed. ExitCode=$AutomationExitCode"
    exit $AutomationExitCode
}

Write-Output 'VCFX_P0_02_FOCUSED_AUTOMATION=PASS'
exit 0
