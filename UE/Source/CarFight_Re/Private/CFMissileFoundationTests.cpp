// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-09-07
// Description: CF-FQ-030 미사일 비행·유도 Foundation 자동화 테스트
// Scope: MG-P0-00 기본 비활성·물리 수학 계약과 MG-P0-08~12D Guidance Performance Types·Config Foundation을 검증합니다.
// Changelog:
// - v1.3.0: MG-P0-12D Guidance Activation 기본 호환값·Clamp와 Stateful Seeker 독립 0~180도 반각, Activation/Overshoot Snapshot 기본 계약을 검증.
// - v1.2.0: MG-P0-12C GuidanceLaw 기본 호환값, LeadPursuit 수치 Clamp와 Guidance Law Snapshot 기본 계약을 추가 검증.
// - v1.1.0: Legacy/Stateful Seeker, Direct/Sampled 관측, Reacquisition 기본값·Clamp·각도 invariant·Snapshot 기본 계약을 검증하는 MG-P0-08 ConfigFoundation 테스트 추가.
// - v1.0.0: Missile Foundation Contract 최초 추가.
// Migration:
// - MG-P0-08~12D Foundation 검증은 Runtime 상태 머신을 실행하지 않고 Config/Reflection 값 타입만 검증합니다.
// - v1.3.0부터 Stateful Acquisition/Tracking/Reacquisition 반각은 서로 독립이며, 과거 Tracking 상한 coupling assertion은 Historical contract로 대체합니다.
// - 기존 MG-P0-00 FoundationContract 이름과 검증 범위는 보존합니다.
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileGuideConfigTest,
	"CarFight.Missile.MG_P0_08.ConfigFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] 신규 Guidance Performance 모델의 기본 호환값, 안전 보정과 Snapshot 기본 계약을 검증합니다.
bool FCFMissileGuideConfigTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] UCFProjectileData를 거친 실제 기본 MissileGuideConfig 계약을 확인할 임시 데이터입니다.
	UCFProjectileData* ProjectileData = NewObject<UCFProjectileData>();
	if (!TestNotNull(TEXT("MG-P0-08 ProjectileData 생성"), ProjectileData))
	{
		return false;
	}

	// [v1.1.0] 기존 저장 Asset과 같은 C++ 기본값에서 얻은 유효 Guidance 설정입니다.
	const FCFMissileGuideConfig DefaultGuideConfig = ProjectileData->GetEffectiveMissileGuideConfig();
	TestFalse(TEXT("기존 ProjectileData Guidance 기본 비활성 유지"), DefaultGuideConfig.bUseGuidance);
	TestEqual(TEXT("기존 ProjectileData Guide Mode None 유지"), DefaultGuideConfig.GuideMode, ECFMissileGuideMode::None);
	TestEqual(TEXT("기본 Seeker 모델 LegacySingleGate"), DefaultGuideConfig.SeekerModel, ECFMissileSeekerModel::LegacySingleGate);
	TestEqual(TEXT("기본 관측 모델 DirectActorKinematics"), DefaultGuideConfig.TargetObservationMode, ECFMissileTargetObservationMode::DirectActorKinematics);
	TestEqual(TEXT("기본 Guidance Law ProportionalNavigation"), DefaultGuideConfig.GuidanceLaw, ECFMissileGuidanceLaw::ProportionalNavigation);
	TestEqual(TEXT("기본 Guidance Activation은 Flight Window 호환"), DefaultGuideConfig.GuidanceActivationMode, ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow);
	TestTrue(TEXT("기본 Guidance Activation Delay 0초"), FMath::IsNearlyZero(DefaultGuideConfig.GuidanceActivationDelaySeconds));
	TestTrue(TEXT("기본 Guidance Activation Distance 0cm"), FMath::IsNearlyZero(DefaultGuideConfig.GuidanceActivationDistanceCm));
	TestTrue(TEXT("기본 Lead Time 0.20초"), FMath::IsNearlyEqual(DefaultGuideConfig.LeadTimeSeconds, 0.20f));
	TestTrue(TEXT("기본 Max Lead Distance 2000cm"), FMath::IsNearlyEqual(DefaultGuideConfig.MaxLeadDistanceCm, 2000.0f));
	TestEqual(TEXT("기본 Reacquisition None"), DefaultGuideConfig.ReacquisitionMode, ECFMissileReacquisitionMode::None);
	TestTrue(TEXT("기본 관측 간격 0.08초"), FMath::IsNearlyEqual(DefaultGuideConfig.TargetObservationIntervalSeconds, 0.08f));
	TestTrue(TEXT("기본 속도 추정 응답 0.20초"), FMath::IsNearlyEqual(DefaultGuideConfig.TargetVelocityEstimateResponseTimeSeconds, 0.20f));
	TestTrue(TEXT("기본 Acquisition 반각 30도"), FMath::IsNearlyEqual(DefaultGuideConfig.AcquisitionConeHalfAngleDeg, 30.0f));
	TestTrue(TEXT("기본 Tracking 반각 60도"), FMath::IsNearlyEqual(DefaultGuideConfig.TrackingConeHalfAngleDeg, 60.0f));
	TestTrue(TEXT("기본 Reacquisition 반각 45도"), FMath::IsNearlyEqual(DefaultGuideConfig.ReacquisitionConeHalfAngleDeg, 45.0f));
	TestTrue(TEXT("기본 Reacquisition 시간 0.50초"), FMath::IsNearlyEqual(DefaultGuideConfig.ReacquisitionTimeSeconds, 0.50f));

	// [v1.1.0] 범위를 벗어난 신규 설정값을 안전 보정 계약에 넣을 원본 Config입니다.
	FCFMissileGuideConfig InvalidPerformanceConfig;
	InvalidPerformanceConfig.bUseGuidance = true;
	InvalidPerformanceConfig.GuideMode = ECFMissileGuideMode::TargetActor;
	InvalidPerformanceConfig.SeekerModel = ECFMissileSeekerModel::Stateful;
	InvalidPerformanceConfig.TargetObservationMode = ECFMissileTargetObservationMode::SampledPositionEstimate;
	InvalidPerformanceConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
	InvalidPerformanceConfig.GuidanceLaw = ECFMissileGuidanceLaw::LeadPursuit;
	InvalidPerformanceConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::Independent;
	InvalidPerformanceConfig.GuidanceActivationDelaySeconds = 100.0f;
	InvalidPerformanceConfig.GuidanceActivationDistanceCm = -100.0f;
	InvalidPerformanceConfig.LeadTimeSeconds = 100.0f;
	InvalidPerformanceConfig.MaxLeadDistanceCm = -100.0f;
	InvalidPerformanceConfig.TargetObservationIntervalSeconds = 0.0f;
	InvalidPerformanceConfig.TargetVelocityEstimateResponseTimeSeconds = 100.0f;
	InvalidPerformanceConfig.AcquisitionConeHalfAngleDeg = 170.0f;
	InvalidPerformanceConfig.TrackingConeHalfAngleDeg = 40.0f;
	InvalidPerformanceConfig.ReacquisitionConeHalfAngleDeg = 120.0f;
	InvalidPerformanceConfig.ReacquisitionTimeSeconds = 100.0f;

	// [v1.1.0] 원본을 바꾸지 않고 신규 성능 축 Clamp와 angle invariant가 적용된 Config입니다.
	const FCFMissileGuideConfig EffectivePerformanceConfig = InvalidPerformanceConfig.GetEffectiveConfig();
	TestEqual(TEXT("Stateful Seeker 선택 보존"), EffectivePerformanceConfig.SeekerModel, ECFMissileSeekerModel::Stateful);
	TestEqual(TEXT("Sampled 관측 선택 보존"), EffectivePerformanceConfig.TargetObservationMode, ECFMissileTargetObservationMode::SampledPositionEstimate);
	TestEqual(TEXT("LeadPursuit 선택 보존"), EffectivePerformanceConfig.GuidanceLaw, ECFMissileGuidanceLaw::LeadPursuit);
	TestEqual(TEXT("Independent Guidance Activation 선택 보존"), EffectivePerformanceConfig.GuidanceActivationMode, ECFMissileGuidanceActivationMode::Independent);
	TestTrue(TEXT("Guidance Activation Delay 최대 30초"), FMath::IsNearlyEqual(EffectivePerformanceConfig.GuidanceActivationDelaySeconds, 30.0f));
	TestTrue(TEXT("음수 Guidance Activation Distance는 0cm"), FMath::IsNearlyZero(EffectivePerformanceConfig.GuidanceActivationDistanceCm));
	TestTrue(TEXT("Lead Time 최대 10초"), FMath::IsNearlyEqual(EffectivePerformanceConfig.LeadTimeSeconds, 10.0f));
	TestTrue(TEXT("음수 Max Lead Distance는 0cm"), FMath::IsNearlyZero(EffectivePerformanceConfig.MaxLeadDistanceCm));
	TestEqual(TEXT("ForwardCone 재포착 선택 보존"), EffectivePerformanceConfig.ReacquisitionMode, ECFMissileReacquisitionMode::ForwardCone);
	TestTrue(TEXT("관측 간격 최소 0.001초"), FMath::IsNearlyEqual(EffectivePerformanceConfig.TargetObservationIntervalSeconds, 0.001f));
	TestTrue(TEXT("속도 추정 응답 최대 10초"), FMath::IsNearlyEqual(EffectivePerformanceConfig.TargetVelocityEstimateResponseTimeSeconds, 10.0f));
	TestTrue(TEXT("Tracking 반각 40도 유지"), FMath::IsNearlyEqual(EffectivePerformanceConfig.TrackingConeHalfAngleDeg, 40.0f));
	TestTrue(TEXT("Acquisition 반각은 Tracking과 독립적으로 170도 유지"), FMath::IsNearlyEqual(EffectivePerformanceConfig.AcquisitionConeHalfAngleDeg, 170.0f));
	TestTrue(TEXT("Reacquisition 반각은 Tracking과 독립적으로 120도 유지"), FMath::IsNearlyEqual(EffectivePerformanceConfig.ReacquisitionConeHalfAngleDeg, 120.0f));
	TestTrue(TEXT("Reacquisition 시간 최대 30초"), FMath::IsNearlyEqual(EffectivePerformanceConfig.ReacquisitionTimeSeconds, 30.0f));
	TestTrue(TEXT("원본 Acquisition 반각은 비파괴"), FMath::IsNearlyEqual(InvalidPerformanceConfig.AcquisitionConeHalfAngleDeg, 170.0f));
	TestTrue(TEXT("원본 Reacquisition 반각은 비파괴"), FMath::IsNearlyEqual(InvalidPerformanceConfig.ReacquisitionConeHalfAngleDeg, 120.0f));

	// [v1.1.0] 비정상 부동소수점 입력이 안전한 최솟값/0으로 복구되는지 확인할 Config입니다.
	FCFMissileGuideConfig NonFinitePerformanceConfig;
	NonFinitePerformanceConfig.GuidanceActivationDelaySeconds = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.GuidanceActivationDistanceCm = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.LeadTimeSeconds = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.MaxLeadDistanceCm = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.TargetObservationIntervalSeconds = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.TargetVelocityEstimateResponseTimeSeconds = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.AcquisitionConeHalfAngleDeg = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.TrackingConeHalfAngleDeg = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.ReacquisitionConeHalfAngleDeg = std::numeric_limits<float>::quiet_NaN();
	NonFinitePerformanceConfig.ReacquisitionTimeSeconds = std::numeric_limits<float>::quiet_NaN();

	// [v1.1.0] NaN을 포함한 신규 성능 축에 안전 기본 보정이 적용된 결과입니다.
	const FCFMissileGuideConfig EffectiveNonFiniteConfig = NonFinitePerformanceConfig.GetEffectiveConfig();
	TestTrue(TEXT("NaN Guidance Activation Delay는 0초"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.GuidanceActivationDelaySeconds));
	TestTrue(TEXT("NaN Guidance Activation Distance는 0cm"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.GuidanceActivationDistanceCm));
	TestTrue(TEXT("NaN Lead Time은 0초"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.LeadTimeSeconds));
	TestTrue(TEXT("NaN Max Lead Distance는 0cm"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.MaxLeadDistanceCm));
	TestTrue(TEXT("NaN 관측 간격은 최소 0.001초"), FMath::IsNearlyEqual(EffectiveNonFiniteConfig.TargetObservationIntervalSeconds, 0.001f));
	TestTrue(TEXT("NaN 속도 추정 응답은 최소 0.001초"), FMath::IsNearlyEqual(EffectiveNonFiniteConfig.TargetVelocityEstimateResponseTimeSeconds, 0.001f));
	TestTrue(TEXT("NaN Tracking 반각은 0도"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.TrackingConeHalfAngleDeg));
	TestTrue(TEXT("NaN Acquisition 반각은 0도"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.AcquisitionConeHalfAngleDeg));
	TestTrue(TEXT("NaN Reacquisition 반각은 0도"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.ReacquisitionConeHalfAngleDeg));
	TestTrue(TEXT("NaN Reacquisition 시간은 0초"), FMath::IsNearlyZero(EffectiveNonFiniteConfig.ReacquisitionTimeSeconds));

	// [v1.1.0] 신규 Snapshot 필드가 Pool-safe한 비활성/Legacy 기본값에서 시작하는지 확인할 값입니다.
	const FCFMissileGuideSnapshot DefaultGuideSnapshot;
	TestEqual(TEXT("Snapshot 기본 Seeker 모델 LegacySingleGate"), DefaultGuideSnapshot.SeekerModel, ECFMissileSeekerModel::LegacySingleGate);
	TestEqual(TEXT("Snapshot 기본 Seeker 상태 Inactive"), DefaultGuideSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestEqual(TEXT("Snapshot 기본 관측 모델 DirectActorKinematics"), DefaultGuideSnapshot.TargetObservationMode, ECFMissileTargetObservationMode::DirectActorKinematics);
	TestEqual(TEXT("Snapshot 기본 Guidance Law ProportionalNavigation"), DefaultGuideSnapshot.GuidanceLaw, ECFMissileGuidanceLaw::ProportionalNavigation);
	TestEqual(TEXT("Snapshot 기본 Guidance Activation은 Flight Window 호환"), DefaultGuideSnapshot.GuidanceActivationMode, ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow);
	TestFalse(TEXT("Snapshot 기본 Guidance Activation 미충족"), DefaultGuideSnapshot.bGuidanceActivationSatisfied);
	TestTrue(TEXT("Snapshot 기본 Guidance Aim Point Zero"), DefaultGuideSnapshot.GuidanceAimPoint.IsNearlyZero());
	TestFalse(TEXT("Snapshot 기본 Course Capture 비활성"), DefaultGuideSnapshot.bCourseCaptureActive);
	TestFalse(TEXT("Snapshot 기본 Stateful Overshoot 미준비"), DefaultGuideSnapshot.bOvershootArmed);
	TestTrue(TEXT("Snapshot 기본 LastObserved 위치 Zero"), DefaultGuideSnapshot.LastObservedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("Snapshot 기본 Estimated 위치 Zero"), DefaultGuideSnapshot.EstimatedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("Snapshot 기본 Velocity Estimate Zero"), DefaultGuideSnapshot.FilteredTargetVelocityEstimate.IsNearlyZero());
	TestTrue(TEXT("Snapshot 기본 Observation Age 0"), FMath::IsNearlyZero(DefaultGuideSnapshot.ObservationAgeSeconds));
	TestTrue(TEXT("Snapshot 기본 Reacquisition 경과 0"), FMath::IsNearlyZero(DefaultGuideSnapshot.ReacquisitionElapsedTimeSeconds));
	TestFalse(TEXT("Snapshot 기본 유효 관측 없음"), DefaultGuideSnapshot.bHasValidObservation);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
