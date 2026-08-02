// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-29
// Description: CF-FQ-030 MG-P0-00 미사일 비행·유도 Foundation 자동화 테스트
// Scope: 기본 비활성, Config 보정, Clearance와 제한형 비례항법 순수 수학 계약을 검증합니다.
// Changelog:
// - v1.0.0: Missile Foundation Contract 최초 추가.
// Migration:
// - 이 테스트는 Actor·ProjectileMovement 런타임을 실행하지 않으며 기존 Projectile·Rocket 회귀는 기본 비활성 계약으로 보호합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFMissileFlightTypes.h"
#include "CFMissileGuideMath.h"
#include "CFMissileGuideTypes.h"
#include "CFProjectileData.h"

#include "Misc/AutomationTest.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileFoundationContractTest,
	"CarFight.Missile.MG_P0_00.FoundationContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 기본 비활성, Config 보정과 물리 제한형 Guidance Command를 검증합니다.
bool FCFMissileFoundationContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UCFProjectileData* ProjectileData = NewObject<UCFProjectileData>();
	if (!TestNotNull(TEXT("ProjectileData 생성"), ProjectileData))
	{
		return false;
	}

	const FCFMissileFlightConfig DefaultFlightConfig = ProjectileData->GetEffectiveMissileFlightConfig();
	const FCFMissileGuideConfig DefaultGuideConfig = ProjectileData->GetEffectiveMissileGuideConfig();
	TestFalse(TEXT("기본 Missile Flight 비활성"), DefaultFlightConfig.bUseMissileFlight);
	TestEqual(TEXT("비활성 Flight 초기 상태 Inactive"), DefaultFlightConfig.GetInitialFlightState(), ECFMissileFlightState::Inactive);
	TestFalse(TEXT("기본 Guidance 비활성"), DefaultGuideConfig.bUseGuidance);
	TestEqual(TEXT("비활성 Guide Mode None"), DefaultGuideConfig.GuideMode, ECFMissileGuideMode::None);
	TestFalse(TEXT("비활성 Guide 활성 판정 False"), DefaultGuideConfig.IsGuidanceEnabled());
	TestTrue(TEXT("Missile Foundation 요약에 Flight Disabled"), ProjectileData->BuildMissileFoundationSummary().Contains(TEXT("Flight=Disabled")));
	TestTrue(TEXT("Missile Foundation 요약에 Guidance Disabled"), ProjectileData->BuildMissileFoundationSummary().Contains(TEXT("Guidance=Disabled")));
	TestTrue(TEXT("Projectile 통합 요약에 Missile Foundation 포함"), ProjectileData->BuildProjectileSummary().Contains(TEXT("Missile=[MissileFoundation:")));

	FCFMissileFlightConfig InvalidFlightConfig;
	InvalidFlightConfig.bUseMissileFlight = true;
	InvalidFlightConfig.AttackProfile = ECFMissileAttackProfile::PitchOver;
	InvalidFlightConfig.MinimumClearanceTimeSeconds = -1.0f;
	InvalidFlightConfig.MinimumClearanceDistanceCm = std::numeric_limits<float>::quiet_NaN();
	InvalidFlightConfig.TransitionDurationSeconds = 1000.0f;
	InvalidFlightConfig.TerminalPhaseStartDistanceCm = -500.0f;
	const FCFMissileFlightConfig EffectiveFlightConfig = InvalidFlightConfig.GetEffectiveConfig();
	TestEqual(TEXT("음수 Clearance 시간은 0"), EffectiveFlightConfig.MinimumClearanceTimeSeconds, 0.0f);
	TestEqual(TEXT("NaN Clearance 거리는 0"), EffectiveFlightConfig.MinimumClearanceDistanceCm, 0.0f);
	TestEqual(TEXT("전환 시간 최대 30초"), EffectiveFlightConfig.TransitionDurationSeconds, 30.0f);
	TestEqual(TEXT("음수 Terminal 거리는 0"), EffectiveFlightConfig.TerminalPhaseStartDistanceCm, 0.0f);
	TestEqual(TEXT("활성 Flight 초기 상태 Released"), EffectiveFlightConfig.GetInitialFlightState(), ECFMissileFlightState::Released);
	TestEqual(TEXT("원본 Flight 설정은 비파괴"), InvalidFlightConfig.MinimumClearanceTimeSeconds, -1.0f);

	FCFMissileFlightConfig ClearanceConfig;
	ClearanceConfig.bUseMissileFlight = true;
	ClearanceConfig.MinimumClearanceTimeSeconds = 0.2f;
	ClearanceConfig.MinimumClearanceDistanceCm = 300.0f;
	TestFalse(TEXT("시간만 만족하면 Clearance 실패"), ClearanceConfig.IsClearanceSatisfied(0.25f, 299.0f));
	TestFalse(TEXT("거리만 만족하면 Clearance 실패"), ClearanceConfig.IsClearanceSatisfied(0.19f, 400.0f));
	TestTrue(TEXT("시간·거리 모두 만족하면 Clearance 성공"), ClearanceConfig.IsClearanceSatisfied(0.2f, 300.0f));

	FCFMissileGuideConfig InvalidGuideConfig;
	InvalidGuideConfig.bUseGuidance = false;
	InvalidGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
	InvalidGuideConfig.NavigationConstant = -1.0f;
	InvalidGuideConfig.MaximumTurnRateDegPerSec = 1000.0f;
	InvalidGuideConfig.MaximumLateralAccelerationCmPerSecSq = -100.0f;
	InvalidGuideConfig.GuidanceResponseTimeSeconds = 0.0f;
	InvalidGuideConfig.MinimumGuidanceSpeedCmPerSec = -50.0f;
	InvalidGuideConfig.SeekerFieldOfViewDeg = 720.0f;
	InvalidGuideConfig.LockBreakAngleDeg = -10.0f;
	InvalidGuideConfig.TargetLostGraceTimeSeconds = 100.0f;
	const FCFMissileGuideConfig EffectiveGuideConfig = InvalidGuideConfig.GetEffectiveConfig();
	TestEqual(TEXT("비활성 Guidance는 Mode None"), EffectiveGuideConfig.GuideMode, ECFMissileGuideMode::None);
	TestEqual(TEXT("음수 Navigation Constant는 0"), EffectiveGuideConfig.NavigationConstant, 0.0f);
	TestEqual(TEXT("최대 선회율 상한 720"), EffectiveGuideConfig.MaximumTurnRateDegPerSec, 720.0f);
	TestEqual(TEXT("음수 최대 횡가속도는 0"), EffectiveGuideConfig.MaximumLateralAccelerationCmPerSecSq, 0.0f);
	TestTrue(TEXT("응답 시간은 최소 양수"), EffectiveGuideConfig.GuidanceResponseTimeSeconds >= 0.001f);
	TestEqual(TEXT("음수 최소 Guidance 속도는 0"), EffectiveGuideConfig.MinimumGuidanceSpeedCmPerSec, 0.0f);
	TestEqual(TEXT("Seeker FOV 상한 360"), EffectiveGuideConfig.SeekerFieldOfViewDeg, 360.0f);
	TestEqual(TEXT("음수 Lock Break 각도는 0"), EffectiveGuideConfig.LockBreakAngleDeg, 0.0f);
	TestEqual(TEXT("Target Lost 유예 상한 30"), EffectiveGuideConfig.TargetLostGraceTimeSeconds, 30.0f);
	TestEqual(TEXT("원본 Guide Mode는 비파괴"), InvalidGuideConfig.GuideMode, ECFMissileGuideMode::TargetActor);

	FCFMissileGuideConfig GuidanceConfig;
	GuidanceConfig.bUseGuidance = true;
	GuidanceConfig.GuideMode = ECFMissileGuideMode::TargetActor;
	GuidanceConfig.NavigationConstant = 3.0f;
	GuidanceConfig.MaximumTurnRateDegPerSec = 2.0f;
	GuidanceConfig.MaximumLateralAccelerationCmPerSecSq = 50.0f;
	GuidanceConfig.MinimumGuidanceSpeedCmPerSec = 100.0f;

	FCFMissileGuidanceInput GuidanceInput;
	GuidanceInput.bHasTarget = true;
	GuidanceInput.DeltaSeconds = 1.0f / 60.0f;
	GuidanceInput.MissileLocation = FVector::ZeroVector;
	GuidanceInput.MissileVelocity = FVector(1000.0f, 0.0f, 0.0f);
	GuidanceInput.TargetLocation = FVector(10000.0f, 5000.0f, 0.0f);
	GuidanceInput.TargetVelocityEstimate = FVector::ZeroVector;

	const FCFMissileGuidanceCommand GuidanceCommand = CFMissileGuideMath::CalculateBoundedProportionalNavigationCommand(
		GuidanceInput,
		GuidanceConfig);
	TestTrue(TEXT("유효 Guidance Command 생성"), GuidanceCommand.bCommandValid);
	TestEqual(TEXT("유효 Command InvalidReason None"), GuidanceCommand.InvalidReason, ECFMissileMissReason::None);
	TestTrue(TEXT("요구 횡가속도 생성"), GuidanceCommand.RequestedLateralAccelerationCmPerSecSq.Size() > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("적용 횡가속도 최대값 이하"), GuidanceCommand.AppliedLateralAccelerationCmPerSecSq.Size() <= 50.0f + KINDA_SMALL_NUMBER);
	TestTrue(TEXT("적용 선회율 최대값 이하"), GuidanceCommand.AppliedTurnRateDegPerSec <= 2.0f + KINDA_SMALL_NUMBER);
	TestTrue(TEXT("횡가속도는 진행 방향에 수직"), FMath::Abs(FVector::DotProduct(
		GuidanceCommand.AppliedLateralAccelerationCmPerSecSq,
		GuidanceInput.MissileVelocity.GetSafeNormal())) <= KINDA_SMALL_NUMBER);
	TestTrue(TEXT("횡가속도 제한 적용 표시"), GuidanceCommand.bLimitedByLateralAcceleration);
	TestTrue(TEXT("선회율 제한 적용 표시"), GuidanceCommand.bLimitedByTurnRate);
	TestTrue(TEXT("입력 Missile Velocity 비파괴"), GuidanceInput.MissileVelocity.Equals(FVector(1000.0f, 0.0f, 0.0f)));

	FCFMissileGuidanceInput SlowInput = GuidanceInput;
	SlowInput.MissileVelocity = FVector(10.0f, 0.0f, 0.0f);
	const FCFMissileGuidanceCommand SlowCommand = CFMissileGuideMath::CalculateBoundedProportionalNavigationCommand(
		SlowInput,
		GuidanceConfig);
	TestFalse(TEXT("최소 속도 미달 Command 무효"), SlowCommand.bCommandValid);
	TestEqual(TEXT("최소 속도 미달 사유"), SlowCommand.InvalidReason, ECFMissileMissReason::BelowMinimumGuidanceSpeed);

	FCFMissileGuidanceInput BehindTargetInput = GuidanceInput;
	BehindTargetInput.TargetLocation = FVector(-10000.0f, 0.0f, 0.0f);
	const FCFMissileGuidanceCommand BehindTargetCommand = CFMissileGuideMath::CalculateBoundedProportionalNavigationCommand(
		BehindTargetInput,
		GuidanceConfig);
	TestTrue(TEXT("뒤쪽 목표 입력 자체는 유효"), BehindTargetCommand.bCommandValid);
	TestTrue(TEXT("접근 속도 없는 뒤쪽 목표에 강제 U-Turn 횡가속도 없음"), BehindTargetCommand.AppliedLateralAccelerationCmPerSecSq.IsNearlyZero());
	TestTrue(TEXT("뒤쪽 목표 Seeker 각도 약 180도"), BehindTargetCommand.SeekerAngleDeg >= 179.0f);

	FCFMissileGuideConfig DisabledConfig = GuidanceConfig;
	DisabledConfig.bUseGuidance = false;
	const FCFMissileGuidanceCommand DisabledCommand = CFMissileGuideMath::CalculateBoundedProportionalNavigationCommand(
		GuidanceInput,
		DisabledConfig);
	TestFalse(TEXT("비활성 Guidance Command 무효"), DisabledCommand.bCommandValid);
	TestEqual(TEXT("비활성 Guidance 사유"), DisabledCommand.InvalidReason, ECFMissileMissReason::GuidanceDisabled);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
