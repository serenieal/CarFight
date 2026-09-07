// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.1
// Date: 2026-09-07
// Description: CF-FQ-030 MG-P0-09~12D Stateful Seeker, Sampled Target Observation, Guidance Law·Activation과 Rear Aspect 자동화 테스트
// Scope: Stateful 상태 전이, Sampled 관측/추정, Guidance Variant/Law, Independent Activation, free Seeker geometry, law-independent rear tie-break, approach-armed Overshoot와 Reset 계약을 검증합니다.
// Changelog:
// - v1.4.1: MG-P0-12D 최종검수 P1 교정으로 PurePursuit/LeadPursuit exact 180deg rear에서도 Launch Right tie-break가 실제 횡가속을 생성하고 방향오차·물리 상한을 보존하는 회귀를 추가.
// - v1.4.0: MG-P0-12D GuidanceActivationRearAspectContract를 추가해 Independent AND+latch, 독립 Stateful 반각, exact rear Launch Right tie-break, approach-armed Overshoot와 Reset을 검증.
// - v1.3.1: MG-P0-12C 동일 activation에서 rear Course Capture가 실제 Guidance 결과 Velocity를 유지해 접근 기하를 형성한 뒤 PN으로 복귀하는 전환 검증 추가.
// - v1.3.0: MG-P0-12C GuidanceLawContract를 추가해 PurePursuit LastObserved-only, LeadPursuit bounded lead, PN rear/non-closing Course Capture, 접근 기하 PN 복귀와 Reset을 검증.
// - v1.2.1: MG-P0-11 최종검수 P1 교정으로 동일 측면 이동→반전 Target에서 Low/Normal/High 관측 시점·속도 추정·Guidance 반응 차이를 직접 비교하는 결정론적 행렬을 추가.
// - v1.2.0: MG-P0-11 GuidanceVariantMatrix를 추가해 세 transient config의 결정론적 반응 차이, Tracking 상실, Reacquisition 차이, 물리 한계, 쉬운 정면 실제 Blocking contact와 Reset을 검증.
// - v1.1.0: MG-P0-10 ObservationEstimatorContract와 Sampled 전용 transient 테스트 장치를 추가.
// - v1.0.0: MG-P0-09 Stateful Seeker Contract 최초 추가.
// Migration:
// - v1.4.1은 exact rear tie-break를 Guidance Law 비종속 계약으로 검증하며 Pure/Lead/PN 모두 같은 Launch Right turn-side와 기존 물리 한계를 공유합니다.
// - v1.4.0은 MG-P0-12D의 Current Runtime 계약을 검증하며 과거 Stateful Acquisition/Reacquisition <= Tracking coupling은 더 이상 Current assertion으로 사용하지 않습니다.
// - MG-P0-12D 테스트는 transient Actor/Component/Config만 사용하며 persisted MissileDirectTest Content와 12E USER Feel Setup은 변경하지 않습니다.
// - 모든 테스트는 transient Actor/Component/Data만 사용하며 저장 DataAsset, Blueprint, Map을 수정하지 않습니다.
// - Low/Normal/High라는 이름은 Automation fixture 구분용일 뿐 Product Runtime enum이나 품질 분기로 추가하지 않습니다.
// - v1.2.1의 moving/reversal 행렬은 세 Variant에 동일한 Target 위치 변화와 동일 DeltaTime을 적용하며 Product Runtime 또는 저장 Asset을 추가 수정하지 않습니다.
// - 기존 CFMissileRuntimeTests.cpp v1.3.0과 Product Missile Runtime source는 MG-P0-11에서 수정하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFMissileFlightComp.h"
#include "CFMissileGuideComp.h"
#include "CFMissileTestTarget.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileGuideStateContractTest,
	"CarFight.Missile.MG_P0_09.StatefulSeekerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileObservationEstimatorContractTest,
	"CarFight.Missile.MG_P0_10.ObservationEstimatorContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileGuidanceVariantMatrixTest,
	"CarFight.Missile.MG_P0_11.GuidanceVariantMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileGuidanceLawContractTest,
	"CarFight.Missile.MG_P0_12C.GuidanceLawContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileGuidanceActivationRearAspectContractTest,
	"CarFight.Missile.MG_P0_12D.GuidanceActivationRearAspectContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
	/**
	 * Stateful Seeker 상태 전이를 저장 Asset 없이 실행하기 위한 transient 미사일 테스트 장치입니다.
	 */
	struct FCFStatefulMissileTestRig
	{
		// [v1.0.0] transient Missile과 Target Actor를 소유하는 Automation World입니다.
		UWorld* TestWorld = nullptr;

		// [v1.0.0] MissileGuideComp와 FlightComp를 소유하는 transient Missile Actor입니다.
		AActor* MissileActor = nullptr;

		// [v1.0.0] Launch Context에 Snapshot으로 전달할 transient Target Actor입니다.
		AActor* TargetActor = nullptr;

		// [v1.0.0] ProjectileMovement의 UpdatedComponent로 사용할 Missile Root입니다.
		USceneComponent* MissileRootComponent = nullptr;

		// [v1.0.0] Stateful Guidance가 방향을 변경할 transient ProjectileMovement입니다.
		UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

		// [v1.0.0] Guidance Window를 Direct GuidedFlight까지 진행할 transient FlightComp입니다.
		UCFMissileFlightComp* MissileFlightComponent = nullptr;

		// [v1.0.0] MG-P0-09 상태 전이를 실제로 실행할 transient GuideComp입니다.
		UCFMissileGuideComp* MissileGuideComponent = nullptr;

		// [v1.0.0] World 안에 Missile/Target과 필요한 Component를 생성하고 연결합니다.
		bool Initialize(UWorld* InTestWorld)
		{
			TestWorld = InTestWorld;
			if (!TestWorld)
			{
				return false;
			}

			MissileActor = TestWorld->SpawnActor<AActor>();
			TargetActor = TestWorld->SpawnActor<AActor>();
			if (!MissileActor || !TargetActor)
			{
				return false;
			}

			// [v1.0.0] Missile Actor의 Transform 이동과 ProjectileMovement 연결을 위한 Scene Root입니다.
			MissileRootComponent = NewObject<USceneComponent>(MissileActor, TEXT("StatefulMissileRoot"));
			MissileActor->AddInstanceComponent(MissileRootComponent);
			MissileActor->SetRootComponent(MissileRootComponent);
			MissileRootComponent->RegisterComponent();

			// [v1.0.0] Target Actor의 월드 위치를 안정적으로 이동하기 위한 Scene Root입니다.
			USceneComponent* TargetRootComponent = NewObject<USceneComponent>(TargetActor, TEXT("StatefulTargetRoot"));
			TargetActor->AddInstanceComponent(TargetRootComponent);
			TargetActor->SetRootComponent(TargetRootComponent);
			TargetRootComponent->RegisterComponent();

			ProjectileMovementComponent = NewObject<UProjectileMovementComponent>(MissileActor, TEXT("StatefulProjectileMovement"));
			MissileActor->AddInstanceComponent(ProjectileMovementComponent);
			ProjectileMovementComponent->RegisterComponent();
			ProjectileMovementComponent->SetUpdatedComponent(MissileRootComponent);
			ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

			MissileFlightComponent = NewObject<UCFMissileFlightComp>(MissileActor, TEXT("StatefulMissileFlight"));
			MissileActor->AddInstanceComponent(MissileFlightComponent);
			MissileFlightComponent->RegisterComponent();

			MissileGuideComponent = NewObject<UCFMissileGuideComp>(MissileActor, TEXT("StatefulMissileGuide"));
			MissileActor->AddInstanceComponent(MissileGuideComponent);
			MissileGuideComponent->RegisterComponent();

			return MissileRootComponent
				&& ProjectileMovementComponent
				&& MissileFlightComponent
				&& MissileGuideComponent;
		}

		// [v1.0.0] Target을 Missile의 현재 위치에서 지정 반각과 거리로 배치합니다.
		void SetTargetAngleDeg(const float TargetAngleDeg, const float TargetDistanceCm = 10000.0f) const
		{
			// [v1.0.0] 중심선 +X 기준 Target 반각을 라디안으로 변환한 값입니다.
			const float TargetAngleRadians = FMath::DegreesToRadians(TargetAngleDeg);

			// [v1.0.0] 요청 반각과 거리로 계산한 Missile 기준 Target 상대 위치입니다.
			const FVector TargetOffset(
				FMath::Cos(TargetAngleRadians) * TargetDistanceCm,
				FMath::Sin(TargetAngleRadians) * TargetDistanceCm,
				0.0f);
			TargetActor->SetActorLocation(MissileActor->GetActorLocation() + TargetOffset);
		}

		// [v1.0.0] Seeker 각도 테스트가 이전 PN 조향 영향을 받지 않게 Missile 진행 방향을 +X로 복원합니다.
		void ResetForwardVelocity(const float SpeedCmPerSec = 1000.0f) const
		{
			ProjectileMovementComponent->Velocity = FVector(SpeedCmPerSec, 0.0f, 0.0f);
		}

		// [v1.0.0] Direct GuidedFlight를 열고 지정 Stateful Config와 Target Snapshot으로 Guidance를 시작합니다.
		void StartGuidance(const FCFMissileGuideConfig& GuideConfig, const FVector& InitialTargetLocation) const
		{
			MissileActor->SetActorLocation(FVector::ZeroVector);
			TargetActor->SetActorLocation(InitialTargetLocation);
			ResetForwardVelocity();

			// [v1.0.0] 상태 테스트에서 즉시 GuidedFlight를 열기 위한 최소 Direct Flight 설정입니다.
			FCFMissileFlightConfig FlightConfig;
			FlightConfig.bUseMissileFlight = true;
			FlightConfig.AttackProfile = ECFMissileAttackProfile::Direct;
			FlightConfig.MinimumClearanceTimeSeconds = 0.0f;
			FlightConfig.MinimumClearanceDistanceCm = 0.0f;

			// [v1.0.0] 이번 transient 발사의 Target Snapshot과 초기 상태를 전달할 Launch Context입니다.
			FCFProjectileLaunchContext LaunchContext;
			LaunchContext.LaunchTransform = MissileActor->GetActorTransform();
			LaunchContext.InitialLaunchDirection = FVector::ForwardVector;
			LaunchContext.InitialLaunchVelocity = ProjectileMovementComponent->Velocity;
			LaunchContext.CommandTargetLocation = InitialTargetLocation;
			LaunchContext.GuidanceTargetActor = TargetActor;
			LaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;

			MissileFlightComponent->StartMissileFlight(
				FlightConfig,
				LaunchContext,
				ProjectileMovementComponent);
			MissileFlightComponent->AdvanceFlightForAutomation(0.01f);

			MissileGuideComponent->StartMissileGuidance(
				GuideConfig,
				LaunchContext,
				ProjectileMovementComponent,
				MissileFlightComponent);
		}

		// [v1.4.0] Flight Guidance Window를 의도적으로 닫은 상태에서 Guidance Activation 계약을 검증하도록 발사를 시작합니다.
		void StartGuidanceWithClosedFlightWindow(
			const FCFMissileGuideConfig& GuideConfig,
			const FVector& InitialTargetLocation,
			const float MinimumClearanceTimeSeconds = 10.0f,
			const float MinimumClearanceDistanceCm = 100000.0f) const
		{
			MissileActor->SetActorLocation(FVector::ZeroVector);
			TargetActor->SetActorLocation(InitialTargetLocation);
			ResetForwardVelocity();

			// [v1.4.0] 이번 테스트 동안 Flight Guidance Window가 열리지 않도록 충분히 큰 Clearance 조건을 가진 Direct Flight 설정입니다.
			FCFMissileFlightConfig FlightConfig;
			FlightConfig.bUseMissileFlight = true;
			FlightConfig.AttackProfile = ECFMissileAttackProfile::Direct;
			FlightConfig.MinimumClearanceTimeSeconds = MinimumClearanceTimeSeconds;
			FlightConfig.MinimumClearanceDistanceCm = MinimumClearanceDistanceCm;

			// [v1.4.0] 발사 Transform Right Vector와 동일 Launch Target Snapshot을 12D Runtime에 전달할 Launch Context입니다.
			FCFProjectileLaunchContext LaunchContext;
			LaunchContext.LaunchTransform = MissileActor->GetActorTransform();
			LaunchContext.InitialLaunchDirection = FVector::ForwardVector;
			LaunchContext.InitialLaunchVelocity = ProjectileMovementComponent->Velocity;
			LaunchContext.CommandTargetLocation = InitialTargetLocation;
			LaunchContext.GuidanceTargetActor = TargetActor;
			LaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;

			MissileFlightComponent->StartMissileFlight(
				FlightConfig,
				LaunchContext,
				ProjectileMovementComponent);

			MissileGuideComponent->StartMissileGuidance(
				GuideConfig,
				LaunchContext,
				ProjectileMovementComponent,
				MissileFlightComponent);
		}

		// [v1.4.0] Guidance Activation 테스트가 FlightComp의 실제 경과 시간·분리 거리 Snapshot을 갱신하도록 Flight를 한 단계 진행합니다.
		void AdvanceFlight(const float DeltaTime) const
		{
			MissileFlightComponent->AdvanceFlightForAutomation(DeltaTime);
		}

		// [v1.0.0] 다음 상태 전이 전에 Missile 진행 방향을 +X로 복원하고 Guidance를 한 단계 진행합니다.
		void AdvanceGuidance(const float DeltaTime)
		{
			ResetForwardVelocity();
			MissileGuideComponent->AdvanceGuidanceForAutomation(DeltaTime);
		}

		// [v1.3.1] 같은 activation의 실제 Guidance 결과 Velocity를 유지한 채 다음 Guidance 단계를 진행합니다.
		void AdvanceGuidanceKeepingVelocity(const float DeltaTime) const
		{
			MissileGuideComponent->AdvanceGuidanceForAutomation(DeltaTime);
		}
	};

	// [v1.0.0] MG-P0-09 공통 상태 테스트가 사용할 Stateful + DirectActorKinematics Config를 생성합니다.
	FCFMissileGuideConfig BuildStatefulGuideConfig()
	{
		// [v1.0.0] Stateful 상태 전이만 분명하게 관측하도록 물리 제한을 충분히 열어 둔 Guidance 설정입니다.
		FCFMissileGuideConfig GuideConfig;
		GuideConfig.bUseGuidance = true;
		GuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
		GuideConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;
		GuideConfig.NavigationConstant = 3.0f;
		GuideConfig.MaximumTurnRateDegPerSec = 180.0f;
		GuideConfig.MaximumLateralAccelerationCmPerSecSq = 100000.0f;
		GuideConfig.GuidanceResponseTimeSeconds = 0.01f;
		GuideConfig.MinimumGuidanceSpeedCmPerSec = 1.0f;
		GuideConfig.SeekerModel = ECFMissileSeekerModel::Stateful;
		GuideConfig.TargetObservationMode = ECFMissileTargetObservationMode::DirectActorKinematics;
		GuideConfig.AcquisitionConeHalfAngleDeg = 30.0f;
		GuideConfig.TrackingConeHalfAngleDeg = 60.0f;
		GuideConfig.TargetLostGraceTimeSeconds = 0.20f;
		GuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
		GuideConfig.ReacquisitionConeHalfAngleDeg = 45.0f;
		GuideConfig.ReacquisitionTimeSeconds = 0.50f;
		return GuideConfig;
	}

	// [v1.0.0] +X 중심선 기준 지정 반각과 거리의 절대 Target 위치를 생성합니다.
	FVector MakeTargetLocationAtAngleDeg(const float TargetAngleDeg, const float TargetDistanceCm = 10000.0f)
	{
		// [v1.0.0] 반각 계산에 사용할 라디안 값입니다.
		const float TargetAngleRadians = FMath::DegreesToRadians(TargetAngleDeg);
		return FVector(
			FMath::Cos(TargetAngleRadians) * TargetDistanceCm,
			FMath::Sin(TargetAngleRadians) * TargetDistanceCm,
			0.0f);
	}

	/**
	 * MG-P0-10 SampledPositionEstimate를 저장 Asset 없이 검증하는 transient 미사일 테스트 장치입니다.
	 */
	struct FCFSampledMissileTestRig
	{
		// [v1.1.0] transient Missile과 Test Target을 소유하는 Automation World입니다.
		UWorld* TestWorld = nullptr;

		// [v1.1.0] MissileGuideComp와 FlightComp를 소유하는 transient Missile Actor입니다.
		AActor* MissileActor = nullptr;

		// [v1.1.0] 위치 샘플과 의도적으로 다른 GetVelocity 값을 제공할 Missile Test Target입니다.
		ACFMissileTestTarget* TargetActor = nullptr;

		// [v1.1.0] ProjectileMovement의 UpdatedComponent로 사용할 Missile Root입니다.
		USceneComponent* MissileRootComponent = nullptr;

		// [v1.1.0] Sampled Guidance가 방향을 변경할 transient ProjectileMovement입니다.
		UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

		// [v1.1.0] Guidance Window open/closed 계약을 제어할 transient FlightComp입니다.
		UCFMissileFlightComp* MissileFlightComponent = nullptr;

		// [v1.1.0] MG-P0-10 observer와 estimator를 실제로 실행할 transient GuideComp입니다.
		UCFMissileGuideComp* MissileGuideComponent = nullptr;

		// [v1.1.0] World 안에 Sampled observer 검증용 Missile/Target과 Component를 생성합니다.
		bool Initialize(UWorld* InTestWorld)
		{
			TestWorld = InTestWorld;
			if (!TestWorld)
			{
				return false;
			}

			MissileActor = TestWorld->SpawnActor<AActor>();
			TargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>();
			if (!MissileActor || !TargetActor)
			{
				return false;
			}

			// [v1.1.0] Missile Actor Transform과 ProjectileMovement 연결을 위한 Scene Root입니다.
			MissileRootComponent = NewObject<USceneComponent>(MissileActor, TEXT("SampledMissileRoot"));
			MissileActor->AddInstanceComponent(MissileRootComponent);
			MissileActor->SetRootComponent(MissileRootComponent);
			MissileRootComponent->RegisterComponent();

			ProjectileMovementComponent = NewObject<UProjectileMovementComponent>(MissileActor, TEXT("SampledProjectileMovement"));
			MissileActor->AddInstanceComponent(ProjectileMovementComponent);
			ProjectileMovementComponent->RegisterComponent();
			ProjectileMovementComponent->SetUpdatedComponent(MissileRootComponent);
			ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

			MissileFlightComponent = NewObject<UCFMissileFlightComp>(MissileActor, TEXT("SampledMissileFlight"));
			MissileActor->AddInstanceComponent(MissileFlightComponent);
			MissileFlightComponent->RegisterComponent();

			MissileGuideComponent = NewObject<UCFMissileGuideComp>(MissileActor, TEXT("SampledMissileGuide"));
			MissileActor->AddInstanceComponent(MissileGuideComponent);
			MissileGuideComponent->RegisterComponent();

			return MissileRootComponent
				&& ProjectileMovementComponent
				&& MissileFlightComponent
				&& MissileGuideComponent;
		}

		// [v1.1.0] Seeker 각도 검증이 이전 PN 조향에 영향받지 않도록 Missile 진행 방향을 +X로 복원합니다.
		void ResetForwardVelocity(const float SpeedCmPerSec = 1000.0f) const
		{
			ProjectileMovementComponent->Velocity = FVector(SpeedCmPerSec, 0.0f, 0.0f);
		}

		// [v1.1.0] 지정 Clearance 시간과 Guide Config로 Sampled observer activation을 시작합니다.
		void StartGuidance(
			const FCFMissileGuideConfig& GuideConfig,
			const FVector& InitialTargetLocation,
			const float MinimumClearanceTimeSeconds = 0.0f,
			const bool bOpenGuidanceWindowImmediately = true) const
		{
			MissileActor->SetActorLocation(FVector::ZeroVector);
			TargetActor->SetActorLocation(InitialTargetLocation);
			ResetForwardVelocity();

			// [v1.1.0] Guidance Window timing을 제어할 Direct Flight 설정입니다.
			FCFMissileFlightConfig FlightConfig;
			FlightConfig.bUseMissileFlight = true;
			FlightConfig.AttackProfile = ECFMissileAttackProfile::Direct;
			FlightConfig.MinimumClearanceTimeSeconds = MinimumClearanceTimeSeconds;
			FlightConfig.MinimumClearanceDistanceCm = 0.0f;

			// [v1.1.0] Sampled observer가 같은 Launch Snapshot Target을 받게 할 Launch Context입니다.
			FCFProjectileLaunchContext LaunchContext;
			LaunchContext.LaunchTransform = MissileActor->GetActorTransform();
			LaunchContext.InitialLaunchDirection = FVector::ForwardVector;
			LaunchContext.InitialLaunchVelocity = ProjectileMovementComponent->Velocity;
			LaunchContext.CommandTargetLocation = InitialTargetLocation;
			LaunchContext.GuidanceTargetActor = TargetActor;
			LaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;

			MissileFlightComponent->StartMissileFlight(
				FlightConfig,
				LaunchContext,
				ProjectileMovementComponent);
			if (bOpenGuidanceWindowImmediately)
			{
				MissileFlightComponent->AdvanceFlightForAutomation(
					FMath::Max(MinimumClearanceTimeSeconds, 0.0f) + 0.01f);
			}

			MissileGuideComponent->StartMissileGuidance(
				GuideConfig,
				LaunchContext,
				ProjectileMovementComponent,
				MissileFlightComponent);
		}

		// [v1.1.0] 실제 월드 Tick 없이 Guide observer와 Guidance를 한 단계 진행합니다.
		void AdvanceGuidance(const float DeltaTime)
		{
			ResetForwardVelocity();
			MissileGuideComponent->AdvanceGuidanceForAutomation(DeltaTime);
		}

		// [v1.1.0] 현재 Flight 상태만 진행해 닫혀 있던 Guidance Window를 엽니다.
		void AdvanceFlight(const float DeltaTime) const
		{
			MissileFlightComponent->AdvanceFlightForAutomation(DeltaTime);
		}
	};

	// [v1.1.0] MG-P0-10 공통 SampledPositionEstimate 설정을 생성합니다.
	FCFMissileGuideConfig BuildSampledGuideConfig(
		const float ObservationIntervalSeconds = 0.10f,
		const float VelocityEstimateResponseTimeSeconds = 0.20f)
	{
		// [v1.1.0] Stateful 상태 계약을 유지하면서 Target observation만 Sampled로 전환할 설정입니다.
		FCFMissileGuideConfig GuideConfig = BuildStatefulGuideConfig();
		GuideConfig.TargetObservationMode = ECFMissileTargetObservationMode::SampledPositionEstimate;
		GuideConfig.AcquisitionConeHalfAngleDeg = 90.0f;
		GuideConfig.TrackingConeHalfAngleDeg = 90.0f;
		GuideConfig.TargetObservationIntervalSeconds = ObservationIntervalSeconds;
		GuideConfig.TargetVelocityEstimateResponseTimeSeconds = VelocityEstimateResponseTimeSeconds;
		GuideConfig.TargetLostGraceTimeSeconds = 0.20f;
		return GuideConfig;
	}

	// [v1.2.0] MG-P0-11 저성능 Automation fixture용 데이터 기반 Guidance 설정을 생성합니다.
	FCFMissileGuideConfig BuildLowPerformanceGuideConfig()
	{
		// [v1.2.0] 느린 관측·추정·조향과 좁은 Tracking, 재포착 없음으로 구성한 저성능 설정입니다.
		FCFMissileGuideConfig GuideConfig = BuildSampledGuideConfig(0.15f, 0.35f);
		GuideConfig.GuidanceResponseTimeSeconds = 0.30f;
		GuideConfig.MaximumTurnRateDegPerSec = 25.0f;
		GuideConfig.MaximumLateralAccelerationCmPerSecSq = 1200.0f;
		GuideConfig.AcquisitionConeHalfAngleDeg = 25.0f;
		GuideConfig.TrackingConeHalfAngleDeg = 35.0f;
		GuideConfig.TargetLostGraceTimeSeconds = 0.10f;
		GuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
		GuideConfig.ReacquisitionConeHalfAngleDeg = 25.0f;
		GuideConfig.ReacquisitionTimeSeconds = 0.0f;
		return GuideConfig;
	}

	// [v1.2.0] MG-P0-11 기준형 Automation fixture용 데이터 기반 Guidance 설정을 생성합니다.
	FCFMissileGuideConfig BuildNormalPerformanceGuideConfig()
	{
		// [v1.2.0] 중간 관측·추정·조향과 제한된 ForwardCone 재포착으로 구성한 기준형 설정입니다.
		FCFMissileGuideConfig GuideConfig = BuildSampledGuideConfig(0.08f, 0.20f);
		GuideConfig.GuidanceResponseTimeSeconds = 0.20f;
		GuideConfig.MaximumTurnRateDegPerSec = 50.0f;
		GuideConfig.MaximumLateralAccelerationCmPerSecSq = 6000.0f;
		GuideConfig.AcquisitionConeHalfAngleDeg = 35.0f;
		GuideConfig.TrackingConeHalfAngleDeg = 65.0f;
		GuideConfig.TargetLostGraceTimeSeconds = 0.40f;
		GuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
		GuideConfig.ReacquisitionConeHalfAngleDeg = 45.0f;
		GuideConfig.ReacquisitionTimeSeconds = 0.30f;
		return GuideConfig;
	}

	// [v1.2.0] MG-P0-11 고성능 Automation fixture용 데이터 기반 Guidance 설정을 생성합니다.
	FCFMissileGuideConfig BuildHighPerformanceGuideConfig()
	{
		// [v1.2.0] 빠른 관측·추정·조향과 넓은 Tracking/ForwardCone 재포착으로 구성한 고성능 설정입니다.
		FCFMissileGuideConfig GuideConfig = BuildSampledGuideConfig(0.03f, 0.10f);
		GuideConfig.GuidanceResponseTimeSeconds = 0.10f;
		GuideConfig.MaximumTurnRateDegPerSec = 80.0f;
		GuideConfig.MaximumLateralAccelerationCmPerSecSq = 15000.0f;
		GuideConfig.AcquisitionConeHalfAngleDeg = 50.0f;
		GuideConfig.TrackingConeHalfAngleDeg = 85.0f;
		GuideConfig.TargetLostGraceTimeSeconds = 0.80f;
		GuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
		GuideConfig.ReacquisitionConeHalfAngleDeg = 65.0f;
		GuideConfig.ReacquisitionTimeSeconds = 0.60f;
		return GuideConfig;
	}

	// [v1.2.0] 한 Guidance Command가 지정 config의 선회율·횡가속 물리 상한을 모두 지키는지 반환합니다.
	bool IsGuidanceCommandInsidePhysicalLimits(
		const FCFMissileGuidanceCommand& GuidanceCommand,
		const FCFMissileGuideConfig& GuideConfig)
	{
		return GuidanceCommand.AppliedTurnRateDegPerSec
			<= GuideConfig.GetEffectiveMaximumTurnRateDegPerSec() + 0.01f
			&& GuidanceCommand.AppliedLateralAccelerationCmPerSecSq.Size()
			<= GuideConfig.GetEffectiveMaximumLateralAccelerationCmPerSecSq() + 0.01f;
	}
}

// [v1.0.0] Stateful Seeker의 핵심 상태 전이와 Legacy 비침범 Reset 계약을 검증합니다.
bool FCFMissileGuideStateContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 모든 transient Stateful 시나리오를 격리해 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("MG-P0-09 Automation World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Acquisition 30도와 Tracking 60도의 서로 다른 허용 범위를 검증할 첫 장치입니다.
	FCFStatefulMissileTestRig AcquisitionTrackingRig;
	if (!TestTrue(TEXT("Acquisition/Tracking 장치 초기화"), AcquisitionTrackingRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] Acquisition/Tracking 분리 시나리오에 사용할 기본 Stateful 설정입니다.
	FCFMissileGuideConfig AcquisitionTrackingConfig = BuildStatefulGuideConfig();
	AcquisitionTrackingRig.StartGuidance(
		AcquisitionTrackingConfig,
		MakeTargetLocationAtAngleDeg(45.0f));

	// [v1.0.0] Start 직후 아직 발사 후 Seeker 판정을 시작하지 않은 Snapshot입니다.
	FCFMissileGuideSnapshot GuideSnapshot = AcquisitionTrackingRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Stateful Start는 Inactive"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestFalse(TEXT("Stateful Start는 아직 TargetValid 아님"), GuideSnapshot.bTargetValid);

	AcquisitionTrackingRig.AdvanceGuidance(0.01f);
	GuideSnapshot = AcquisitionTrackingRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("45도 Target은 Acquisition 30도 밖에서 Acquiring 유지"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Acquiring);
	TestFalse(TEXT("Acquiring 대기는 TargetValid False"), GuideSnapshot.bTargetValid);
	TestEqual(TEXT("Acquiring 대기는 최종 Miss 아님"), GuideSnapshot.MissReason, ECFMissileMissReason::None);
	TestFalse(TEXT("Acquiring 대기는 Guidance Command 없음"), AcquisitionTrackingRig.MissileGuideComponent->GetGuidanceCommand().bCommandValid);

	AcquisitionTrackingRig.SetTargetAngleDeg(20.0f);
	AcquisitionTrackingRig.AdvanceGuidance(0.01f);
	GuideSnapshot = AcquisitionTrackingRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("20도 Target은 Acquisition 성공 후 Tracking"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("Tracking은 TargetValid True"), GuideSnapshot.bTargetValid);

	AcquisitionTrackingRig.SetTargetAngleDeg(50.0f);
	AcquisitionTrackingRig.AdvanceGuidance(0.01f);
	GuideSnapshot = AcquisitionTrackingRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("50도 Target은 Acquisition보다 넓은 Tracking 60도 안에서 유지"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("Tracking 유지 구간은 TargetValid True"), GuideSnapshot.bTargetValid);

	// [v1.0.0] Tracking 상실 뒤 Grace 안에서 같은 Snapshot Target이 복귀하는 시나리오 장치입니다.
	FCFStatefulMissileTestRig LostGraceRig;
	if (!TestTrue(TEXT("LostGrace 장치 초기화"), LostGraceRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] 0.20초 LostGrace를 허용하고 Reacquisition은 사용하지 않는 설정입니다.
	FCFMissileGuideConfig LostGraceConfig = BuildStatefulGuideConfig();
	LostGraceConfig.TargetLostGraceTimeSeconds = 0.20f;
	LostGraceRig.StartGuidance(LostGraceConfig, MakeTargetLocationAtAngleDeg(20.0f));
	LostGraceRig.AdvanceGuidance(0.01f);
	LostGraceRig.SetTargetAngleDeg(80.0f);
	LostGraceRig.AdvanceGuidance(0.05f);
	GuideSnapshot = LostGraceRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Tracking 60도 이탈 후 LostGrace 진입"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostGrace);
	TestFalse(TEXT("LostGrace fallback은 TargetValid False"), GuideSnapshot.bTargetValid);
	TestEqual(TEXT("LostGrace 진행 중 최종 Miss 없음"), GuideSnapshot.MissReason, ECFMissileMissReason::None);

	LostGraceRig.SetTargetAngleDeg(20.0f);
	LostGraceRig.AdvanceGuidance(0.05f);
	GuideSnapshot = LostGraceRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Grace 안에 Tracking cone 복귀 시 Tracking 회복"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("Grace 복귀 후 TargetValid True"), GuideSnapshot.bTargetValid);

	// [v1.0.0] ReacquisitionMode=None에서 LostFinal 뒤 암묵적 재추적이 금지되는 시나리오 장치입니다.
	FCFStatefulMissileTestRig NoReacquisitionRig;
	if (!TestTrue(TEXT("Reacquisition None 장치 초기화"), NoReacquisitionRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] LostGrace를 즉시 종료하고 재포착을 금지하는 Stateful 설정입니다.
	FCFMissileGuideConfig NoReacquisitionConfig = BuildStatefulGuideConfig();
	NoReacquisitionConfig.TargetLostGraceTimeSeconds = 0.0f;
	NoReacquisitionConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
	NoReacquisitionRig.StartGuidance(NoReacquisitionConfig, MakeTargetLocationAtAngleDeg(20.0f));
	NoReacquisitionRig.AdvanceGuidance(0.01f);
	NoReacquisitionRig.SetTargetAngleDeg(80.0f);
	NoReacquisitionRig.AdvanceGuidance(0.01f);
	GuideSnapshot = NoReacquisitionRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Grace 0 + Reacquisition None은 같은 Step에 LostFinal"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("최종 추적 상실은 TargetLost"), GuideSnapshot.MissReason, ECFMissileMissReason::TargetLost);

	NoReacquisitionRig.SetTargetAngleDeg(20.0f);
	NoReacquisitionRig.AdvanceGuidance(0.01f);
	GuideSnapshot = NoReacquisitionRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("LostFinal 뒤 Target이 돌아와도 암묵적 재추적 금지"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestFalse(TEXT("LostFinal은 TargetValid False 유지"), GuideSnapshot.bTargetValid);

	// [v1.0.0] ForwardCone이 Acquisition보다 넓은 각도에서 같은 Snapshot Target을 재포착하는 시나리오 장치입니다.
	FCFStatefulMissileTestRig ForwardReacquisitionRig;
	if (!TestTrue(TEXT("ForwardCone Reacquisition 장치 초기화"), ForwardReacquisitionRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] Grace 종료 뒤 45도 ForwardCone에서 최대 0.5초 재포착을 허용하는 설정입니다.
	FCFMissileGuideConfig ForwardReacquisitionConfig = BuildStatefulGuideConfig();
	ForwardReacquisitionConfig.TargetLostGraceTimeSeconds = 0.05f;
	ForwardReacquisitionConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
	ForwardReacquisitionConfig.ReacquisitionConeHalfAngleDeg = 45.0f;
	ForwardReacquisitionConfig.ReacquisitionTimeSeconds = 0.50f;
	ForwardReacquisitionRig.StartGuidance(ForwardReacquisitionConfig, MakeTargetLocationAtAngleDeg(20.0f));
	ForwardReacquisitionRig.AdvanceGuidance(0.01f);
	ForwardReacquisitionRig.SetTargetAngleDeg(80.0f);
	ForwardReacquisitionRig.AdvanceGuidance(0.10f);
	GuideSnapshot = ForwardReacquisitionRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Grace 종료 뒤 ForwardCone은 Reacquiring 진입"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Reacquiring);
	TestEqual(TEXT("Reacquiring 대기는 최종 Miss 아님"), GuideSnapshot.MissReason, ECFMissileMissReason::None);

	ForwardReacquisitionRig.SetTargetAngleDeg(40.0f);
	ForwardReacquisitionRig.AdvanceGuidance(0.01f);
	GuideSnapshot = ForwardReacquisitionRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Acquisition 30도 밖이지만 Reacquisition 45도 안이면 Tracking 회복"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("ForwardCone 재포착 성공 후 TargetValid True"), GuideSnapshot.bTargetValid);

	// [v1.0.0] Destroyed Target이 Grace 뒤 Reacquisition으로 넘어가지 않는 계약을 검증할 장치입니다.
	FCFStatefulMissileTestRig DestroyedTargetRig;
	if (!TestTrue(TEXT("Destroyed Target 장치 초기화"), DestroyedTargetRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] 재포착이 켜져 있어도 파괴된 Actor에는 재포착을 허용하지 않을 설정입니다.
	FCFMissileGuideConfig DestroyedTargetConfig = BuildStatefulGuideConfig();
	DestroyedTargetConfig.TargetLostGraceTimeSeconds = 0.05f;
	DestroyedTargetConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
	DestroyedTargetConfig.ReacquisitionTimeSeconds = 0.50f;
	DestroyedTargetRig.StartGuidance(DestroyedTargetConfig, MakeTargetLocationAtAngleDeg(20.0f));
	DestroyedTargetRig.AdvanceGuidance(0.01f);
	DestroyedTargetRig.TargetActor->Destroy();
	DestroyedTargetRig.AdvanceGuidance(0.10f);
	GuideSnapshot = DestroyedTargetRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Destroyed Target은 Grace 종료 뒤 LostFinal"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("Destroyed Target 최종 사유 TargetLost"), GuideSnapshot.MissReason, ECFMissileMissReason::TargetLost);

	// [v1.0.0] ReacquisitionTimeSeconds=0이 Reacquiring 상태를 한 Step 유지하지 않는지 검증할 장치입니다.
	FCFStatefulMissileTestRig ZeroReacquisitionTimeRig;
	if (!TestTrue(TEXT("Zero Reacquisition Time 장치 초기화"), ZeroReacquisitionTimeRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] ForwardCone을 선택했지만 재포착 시간은 0으로 둔 설정입니다.
	FCFMissileGuideConfig ZeroReacquisitionTimeConfig = BuildStatefulGuideConfig();
	ZeroReacquisitionTimeConfig.TargetLostGraceTimeSeconds = 0.0f;
	ZeroReacquisitionTimeConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
	ZeroReacquisitionTimeConfig.ReacquisitionTimeSeconds = 0.0f;
	ZeroReacquisitionTimeRig.StartGuidance(ZeroReacquisitionTimeConfig, MakeTargetLocationAtAngleDeg(20.0f));
	ZeroReacquisitionTimeRig.AdvanceGuidance(0.01f);
	ZeroReacquisitionTimeRig.SetTargetAngleDeg(80.0f);
	ZeroReacquisitionTimeRig.AdvanceGuidance(0.01f);
	GuideSnapshot = ZeroReacquisitionTimeRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Reacquisition 시간 0은 같은 Step에 LostFinal"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);

	// [v1.0.0] LostFinal HoldLastKnownPoint가 Actor 추적을 재개하지 않고 고정점을 유지하는지 검증할 장치입니다.
	FCFStatefulMissileTestRig HoldPointRig;
	if (!TestTrue(TEXT("HoldLastKnownPoint 장치 초기화"), HoldPointRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] 즉시 LostFinal 뒤 Sensor Truth 지점을 고정해서 유도할 설정입니다.
	FCFMissileGuideConfig HoldPointConfig = BuildStatefulGuideConfig();
	HoldPointConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::HoldLastKnownPoint;
	HoldPointConfig.TargetLostGraceTimeSeconds = 0.0f;
	HoldPointConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
	HoldPointRig.StartGuidance(HoldPointConfig, MakeTargetLocationAtAngleDeg(20.0f));
	HoldPointRig.AdvanceGuidance(0.01f);
	HoldPointRig.SetTargetAngleDeg(80.0f);
	HoldPointRig.AdvanceGuidance(0.01f);
	GuideSnapshot = HoldPointRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Hold 정책도 Seeker 상태는 LostFinal"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("Hold LostFinal 진단은 TargetLost"), GuideSnapshot.MissReason, ECFMissileMissReason::TargetLost);

	// [v1.0.0] LostFinal 진입 순간 한 번 고정된 Hold Target 위치입니다.
	const FVector FrozenHoldTargetLocation = GuideSnapshot.TargetLocation;
	HoldPointRig.SetTargetAngleDeg(10.0f);
	HoldPointRig.AdvanceGuidance(0.01f);
	GuideSnapshot = HoldPointRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Hold 정책은 Target Actor가 이동해도 LostFinal 유지"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestTrue(TEXT("Hold 정책은 LostFinal 진입 지점을 고정"), GuideSnapshot.TargetLocation.Equals(FrozenHoldTargetLocation, 0.01f));
	TestFalse(TEXT("Hold 고정점 Guidance는 Actor Tracking TargetValid가 아님"), GuideSnapshot.bTargetValid);

	HoldPointRig.MissileGuideComponent->ResetMissileGuidance();
	GuideSnapshot = HoldPointRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Pool Reset 후 SeekerState Inactive"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestTrue(TEXT("Pool Reset 후 LastObserved Zero"), GuideSnapshot.LastObservedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("Pool Reset 후 Estimated Zero"), GuideSnapshot.EstimatedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("Pool Reset 후 Velocity Estimate Zero"), GuideSnapshot.FilteredTargetVelocityEstimate.IsNearlyZero());
	TestTrue(TEXT("Pool Reset 후 ObservationAge 0"), FMath::IsNearlyZero(GuideSnapshot.ObservationAgeSeconds));
	TestTrue(TEXT("Pool Reset 후 ReacquisitionElapsed 0"), FMath::IsNearlyZero(GuideSnapshot.ReacquisitionElapsedTimeSeconds));
	TestFalse(TEXT("Pool Reset 후 유효 관측 없음"), GuideSnapshot.bHasValidObservation);
	TestNull(TEXT("Pool Reset 후 Guidance Target Actor 없음"), HoldPointRig.MissileGuideComponent->GetGuidanceTargetActor());
	TestTrue(TEXT("Pool Reset 후 기존 TargetLocation Zero"), GuideSnapshot.TargetLocation.IsNearlyZero());

	// [v1.0.0] Target을 실제로 지나친 뒤 Reacquisition보다 Overshoot가 우선하는지 검증할 장치입니다.
	FCFStatefulMissileTestRig OvershootRig;
	if (!TestTrue(TEXT("Overshoot 우선순위 장치 초기화"), OvershootRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.0.0] Grace 0과 ForwardCone 재포착을 사용해 Overshoot 우선순위를 명확히 드러낼 설정입니다.
	FCFMissileGuideConfig OvershootConfig = BuildStatefulGuideConfig();
	OvershootConfig.AcquisitionConeHalfAngleDeg = 30.0f;
	OvershootConfig.TrackingConeHalfAngleDeg = 60.0f;
	OvershootConfig.TargetLostGraceTimeSeconds = 0.0f;
	OvershootConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
	OvershootConfig.ReacquisitionConeHalfAngleDeg = 45.0f;
	OvershootConfig.ReacquisitionTimeSeconds = 1.0f;
	OvershootRig.StartGuidance(OvershootConfig, FVector(1000.0f, 0.0f, 0.0f));
	OvershootRig.AdvanceGuidance(0.01f);

	OvershootRig.MissileActor->SetActorLocation(FVector(900.0f, 0.0f, 0.0f));
	OvershootRig.AdvanceGuidance(0.01f);
	GuideSnapshot = OvershootRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Target 직전까지 Tracking 유지"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Tracking);

	OvershootRig.MissileActor->SetActorLocation(FVector(1200.0f, 0.0f, 0.0f));
	OvershootRig.AdvanceGuidance(0.01f);
	GuideSnapshot = OvershootRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Target 통과 뒤 Reacquiring보다 LostFinal Overshoot 우선"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("Target 통과 최종 사유 Overshoot"), GuideSnapshot.MissReason, ECFMissileMissReason::Overshoot);

	return true;
}

// [v1.1.0] Sampled observer의 위치 관측 주기, 속도 추정, 외삽, Window 대기와 Sensor Truth 일관성을 검증합니다.
bool FCFMissileObservationEstimatorContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] 모든 MG-P0-10 transient 시나리오를 격리해 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("MG-P0-10 Automation World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.1.0] 위치 샘플과 의도적으로 반대인 Actor GetVelocity를 사용해 raw velocity 비소비를 검증할 장치입니다.
	FCFSampledMissileTestRig EstimatorRig;
	if (!TestTrue(TEXT("Sampled estimator 장치 초기화"), EstimatorRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.1.0] 0.10초 관측 주기와 0.20초 속도 추정 응답 시간을 가진 기본 Sampled 설정입니다.
	FCFMissileGuideConfig SampledConfig = BuildSampledGuideConfig(0.10f, 0.20f);

	// [v1.1.0] 위치 차분 +Y와 정반대가 되도록 Target Actor가 직접 보고할 가짜 Velocity입니다.
	const FVector ContradictoryActorVelocity(0.0f, -9000.0f, 0.0f);
	EstimatorRig.TargetActor->SetTestVelocityForAutomation(ContradictoryActorVelocity);
	EstimatorRig.StartGuidance(SampledConfig, FVector(10000.0f, 0.0f, 0.0f));

	// [v1.1.0] 발사 순간 seed 직후 Sampled observer의 초기 상태입니다.
	FCFMissileGuideSnapshot GuideSnapshot = EstimatorRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Sampled observer mode 활성"), GuideSnapshot.TargetObservationMode, ECFMissileTargetObservationMode::SampledPositionEstimate);
	TestTrue(TEXT("발사 순간 위치 seed 확보"), GuideSnapshot.LastObservedTargetLocation.Equals(FVector(10000.0f, 0.0f, 0.0f), 0.01f));
	TestTrue(TEXT("Sampled seed는 Actor GetVelocity를 소비하지 않아 VelocityEstimate Zero"), GuideSnapshot.FilteredTargetVelocityEstimate.IsNearlyZero());
	TestTrue(TEXT("Sampled seed ObservationAge 0"), FMath::IsNearlyZero(GuideSnapshot.ObservationAgeSeconds));

	EstimatorRig.TargetActor->SetActorLocation(FVector(10000.0f, 100.0f, 0.0f));
	EstimatorRig.AdvanceGuidance(0.05f);
	GuideSnapshot = EstimatorRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("첫 0.05초에서 Stateful은 Tracking 성립"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("관측 주기 전에는 LastObserved가 seed에 머묾"), GuideSnapshot.LastObservedTargetLocation.Equals(FVector(10000.0f, 0.0f, 0.0f), 0.01f));
	TestTrue(TEXT("관측 주기 전 ObservationAge 0.05"), FMath::IsNearlyEqual(GuideSnapshot.ObservationAgeSeconds, 0.05f, 0.0001f));
	TestTrue(TEXT("초기 VelocityEstimate 0이면 관측 전 Estimated도 seed 유지"), GuideSnapshot.EstimatedTargetLocation.Equals(FVector(10000.0f, 0.0f, 0.0f), 0.01f));

	EstimatorRig.TargetActor->SetActorLocation(FVector(10000.0f, 200.0f, 0.0f));
	EstimatorRig.AdvanceGuidance(0.04f);
	GuideSnapshot = EstimatorRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("0.09초에서도 새 위치 Sample 없음"), GuideSnapshot.LastObservedTargetLocation.Equals(FVector(10000.0f, 0.0f, 0.0f), 0.01f));
	TestTrue(TEXT("단일 ObservationAge clock 0.09"), FMath::IsNearlyEqual(GuideSnapshot.ObservationAgeSeconds, 0.09f, 0.0001f));

	EstimatorRig.AdvanceGuidance(0.01f);
	GuideSnapshot = EstimatorRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("0.10초 도달 시 실제 위치 Sample 1회 취득"), GuideSnapshot.LastObservedTargetLocation.Equals(FVector(10000.0f, 200.0f, 0.0f), 0.01f));
	TestTrue(TEXT("새 Sample 뒤 ObservationAge 0"), FMath::IsNearlyZero(GuideSnapshot.ObservationAgeSeconds, 0.0001f));
	TestTrue(TEXT("위치 차분 +2000cm/s와 응답 alpha 0.5로 +1000cm/s 추정"), FMath::IsNearlyEqual(GuideSnapshot.FilteredTargetVelocityEstimate.Y, 1000.0f, 0.5f));
	TestTrue(TEXT("Actor GetVelocity -9000Y 대신 위치 차분 기반 +Y 추정"), GuideSnapshot.FilteredTargetVelocityEstimate.Y > 0.0f);

	EstimatorRig.TargetActor->SetActorLocation(FVector(10000.0f, 1000.0f, 0.0f));
	EstimatorRig.AdvanceGuidance(0.05f);
	GuideSnapshot = EstimatorRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("관측 사이 LastObserved는 이전 실제 Sample 유지"), GuideSnapshot.LastObservedTargetLocation.Equals(FVector(10000.0f, 200.0f, 0.0f), 0.01f));
	TestTrue(TEXT("관측 사이 Estimated는 200 + 1000*0.05 = 250Y로 외삽"), FMath::IsNearlyEqual(GuideSnapshot.EstimatedTargetLocation.Y, 250.0f, 0.5f));
	TestTrue(TEXT("실제 Target 1000Y를 관측 주기 전에 몰래 따라가지 않음"), !GuideSnapshot.EstimatedTargetLocation.Equals(EstimatorRig.TargetActor->GetActorLocation(), 1.0f));
	TestTrue(TEXT("Stateful Tracking TargetLocation도 같은 Estimated Sensor Truth 사용"), GuideSnapshot.TargetLocation.Equals(GuideSnapshot.EstimatedTargetLocation, 0.01f));

	EstimatorRig.TargetActor->SetActorLocation(FVector(10000.0f, 150.0f, 0.0f));
	EstimatorRig.AdvanceGuidance(0.05f);
	GuideSnapshot = EstimatorRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("방향 반전 Sample은 LastObserved 150Y로 갱신"), FMath::IsNearlyEqual(GuideSnapshot.LastObservedTargetLocation.Y, 150.0f, 0.01f));
	TestTrue(TEXT("Raw -500cm/s 반전 뒤에도 필터 때문에 +250cm/s 추정이 남음"), FMath::IsNearlyEqual(GuideSnapshot.FilteredTargetVelocityEstimate.Y, 250.0f, 0.5f));
	TestTrue(TEXT("방향 반전 직후 초능력처럼 즉시 음수 속도로 뒤집히지 않음"), GuideSnapshot.FilteredTargetVelocityEstimate.Y > 0.0f);

	EstimatorRig.TargetActor->SetActorLocation(FVector(10000.0f, 450.0f, 0.0f));
	EstimatorRig.AdvanceGuidance(0.35f);
	GuideSnapshot = EstimatorRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("낮은 FPS 한 Tick에서도 실제 Sample은 현재 위치 한 번만 반영"), FMath::IsNearlyEqual(GuideSnapshot.LastObservedTargetLocation.Y, 450.0f, 0.01f));
	TestTrue(TEXT("0.35초 실제 경과 시간으로 300/0.35 약 857.14cm/s 추정"), FMath::IsNearlyEqual(GuideSnapshot.FilteredTargetVelocityEstimate.Y, 857.1428f, 1.0f));
	TestTrue(TEXT("낮은 FPS Sample 뒤 ObservationAge는 0으로 단일 reset"), FMath::IsNearlyZero(GuideSnapshot.ObservationAgeSeconds, 0.0001f));

	// [v1.1.0] Guidance Window가 닫힌 동안 Sample 없이 ObservationAge만 누적되는지 검증할 장치입니다.
	FCFSampledMissileTestRig ClosedWindowRig;
	if (!TestTrue(TEXT("Closed Window 장치 초기화"), ClosedWindowRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.1.0] 닫힌 Window에서 Sample을 지연시킬 같은 Sampled 설정입니다.
	FCFMissileGuideConfig ClosedWindowConfig = BuildSampledGuideConfig(0.10f, 0.20f);
	ClosedWindowRig.StartGuidance(
		ClosedWindowConfig,
		FVector(10000.0f, 0.0f, 0.0f),
		1.0f,
		false);
	ClosedWindowRig.TargetActor->SetActorLocation(FVector(10000.0f, 5000.0f, 0.0f));
	ClosedWindowRig.AdvanceGuidance(0.25f);
	GuideSnapshot = ClosedWindowRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Guidance Window 닫힘 동안 SeekerState Inactive 유지"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestTrue(TEXT("Guidance Window 닫힘 동안 새 Actor 위치 Sample 없음"), GuideSnapshot.LastObservedTargetLocation.Equals(FVector(10000.0f, 0.0f, 0.0f), 0.01f));
	TestTrue(TEXT("Guidance Window 닫힘 동안 ObservationAge만 0.25 누적"), FMath::IsNearlyEqual(GuideSnapshot.ObservationAgeSeconds, 0.25f, 0.0001f));

	ClosedWindowRig.AdvanceFlight(1.01f);
	ClosedWindowRig.AdvanceGuidance(0.01f);
	GuideSnapshot = ClosedWindowRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Window open 뒤 즉시 Stateful Tracking 진입"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("누적 interval 충족으로 Window open 첫 Guidance Step에서 현재 위치 Sample"), GuideSnapshot.LastObservedTargetLocation.Equals(FVector(10000.0f, 5000.0f, 0.0f), 0.01f));
	TestTrue(TEXT("Window open 즉시 Sample 뒤 ObservationAge 0"), FMath::IsNearlyZero(GuideSnapshot.ObservationAgeSeconds, 0.0001f));

	// [v1.1.0] 서로 다른 관측 주기가 같은 Target motion의 반응 시점을 실제로 갈라놓는지 검증할 빠른 observer입니다.
	FCFSampledMissileTestRig FastObservationRig;
	if (!TestTrue(TEXT("Fast Observation 장치 초기화"), FastObservationRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.1.0] 느린 관측 성능과 비교할 두 번째 observer입니다.
	FCFSampledMissileTestRig SlowObservationRig;
	if (!TestTrue(TEXT("Slow Observation 장치 초기화"), SlowObservationRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.1.0] 0.05초 관측 주기를 가진 고응답 비교 설정입니다.
	FCFMissileGuideConfig FastObservationConfig = BuildSampledGuideConfig(0.05f, 0.20f);

	// [v1.1.0] 0.20초 관측 주기를 가진 저응답 비교 설정입니다.
	FCFMissileGuideConfig SlowObservationConfig = BuildSampledGuideConfig(0.20f, 0.20f);
	FastObservationRig.StartGuidance(FastObservationConfig, FVector(10000.0f, 0.0f, 0.0f));
	SlowObservationRig.StartGuidance(SlowObservationConfig, FVector(10000.0f, 0.0f, 0.0f));
	FastObservationRig.TargetActor->SetActorLocation(FVector(10000.0f, 400.0f, 0.0f));
	SlowObservationRig.TargetActor->SetActorLocation(FVector(10000.0f, 400.0f, 0.0f));
	FastObservationRig.AdvanceGuidance(0.06f);
	SlowObservationRig.AdvanceGuidance(0.06f);

	// [v1.1.0] 같은 0.06초 뒤 빠른 observer가 공개한 관측 Snapshot입니다.
	const FCFMissileGuideSnapshot FastObservationSnapshot = FastObservationRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.1.0] 같은 0.06초 뒤 느린 observer가 공개한 관측 Snapshot입니다.
	const FCFMissileGuideSnapshot SlowObservationSnapshot = SlowObservationRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("0.05초 observer는 0.06초 뒤 새 위치를 관측"), FMath::IsNearlyEqual(FastObservationSnapshot.LastObservedTargetLocation.Y, 400.0f, 0.01f));
	TestTrue(TEXT("0.20초 observer는 0.06초 뒤 seed 위치 유지"), FMath::IsNearlyZero(SlowObservationSnapshot.LastObservedTargetLocation.Y, 0.01f));
	TestTrue(TEXT("관측 주기가 실제 반응 지연 차이를 생성"), !FastObservationSnapshot.LastObservedTargetLocation.Equals(SlowObservationSnapshot.LastObservedTargetLocation, 1.0f));

	// [v1.1.0] ObservationMode가 SeekerModel과 독립적으로 Legacy Single Gate에도 적용되는지 검증할 장치입니다.
	FCFSampledMissileTestRig LegacySampledRig;
	if (!TestTrue(TEXT("Legacy + Sampled 장치 초기화"), LegacySampledRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.1.0] Legacy Single Gate를 유지하면서 Target observation만 Sampled로 전환한 설정입니다.
	FCFMissileGuideConfig LegacySampledConfig = BuildSampledGuideConfig(0.10f, 0.20f);
	LegacySampledConfig.SeekerModel = ECFMissileSeekerModel::LegacySingleGate;
	LegacySampledConfig.SeekerFieldOfViewDeg = 180.0f;
	LegacySampledConfig.LockBreakAngleDeg = 180.0f;
	LegacySampledRig.TargetActor->SetTestVelocityForAutomation(ContradictoryActorVelocity);
	LegacySampledRig.StartGuidance(LegacySampledConfig, FVector(10000.0f, 0.0f, 0.0f));
	LegacySampledRig.TargetActor->SetActorLocation(FVector(10000.0f, 300.0f, 0.0f));
	LegacySampledRig.AdvanceGuidance(0.10f);
	GuideSnapshot = LegacySampledRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Legacy Seeker에서도 ObservationMode Sampled 유지"), GuideSnapshot.TargetObservationMode, ECFMissileTargetObservationMode::SampledPositionEstimate);
	TestTrue(TEXT("Legacy + Sampled도 위치 차분으로 +Y 속도 추정"), GuideSnapshot.FilteredTargetVelocityEstimate.Y > 0.0f);
	TestTrue(TEXT("Legacy + Sampled도 Actor GetVelocity -Y를 Guidance 정답으로 사용하지 않음"), !FMath::IsNearlyEqual(GuideSnapshot.FilteredTargetVelocityEstimate.Y, ContradictoryActorVelocity.Y, 1.0f));

	// [v1.1.0] LostFinal HoldLastKnownPoint가 현재 Estimated Sensor Truth를 한 번 고정하는지 검증할 장치입니다.
	FCFSampledMissileTestRig SampledHoldRig;
	if (!TestTrue(TEXT("Sampled Hold 장치 초기화"), SampledHoldRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.1.0] Tracking cone 이탈 즉시 LostFinal Hold로 전환할 Sampled 설정입니다.
	FCFMissileGuideConfig SampledHoldConfig = BuildSampledGuideConfig(0.10f, 0.20f);
	SampledHoldConfig.TrackingConeHalfAngleDeg = 30.0f;
	SampledHoldConfig.AcquisitionConeHalfAngleDeg = 30.0f;
	SampledHoldConfig.TargetLostGraceTimeSeconds = 0.0f;
	SampledHoldConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
	SampledHoldConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::HoldLastKnownPoint;
	SampledHoldRig.StartGuidance(SampledHoldConfig, FVector(10000.0f, 0.0f, 0.0f));
	SampledHoldRig.AdvanceGuidance(0.01f);
	SampledHoldRig.TargetActor->SetActorLocation(MakeTargetLocationAtAngleDeg(70.0f));
	SampledHoldRig.AdvanceGuidance(0.10f);
	GuideSnapshot = SampledHoldRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Sampled Tracking cone 이탈 + grace0은 LostFinal"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("Sampled Hold LostFinal 진단 TargetLost"), GuideSnapshot.MissReason, ECFMissileMissReason::TargetLost);

	// [v1.1.0] LostFinal 진입 시 Sampled Estimated Sensor Truth에서 고정된 Hold 지점입니다.
	const FVector FrozenSampledHoldLocation = GuideSnapshot.TargetLocation;
	TestTrue(TEXT("Sampled Hold 지점은 LostFinal 순간 Estimated Target과 일치"), FrozenSampledHoldLocation.Equals(GuideSnapshot.EstimatedTargetLocation, 0.01f));
	SampledHoldRig.TargetActor->SetActorLocation(FVector(10000.0f, -9000.0f, 0.0f));
	SampledHoldRig.TargetActor->SetTestVelocityForAutomation(FVector(0.0f, 20000.0f, 0.0f));
	SampledHoldRig.AdvanceGuidance(0.20f);
	GuideSnapshot = SampledHoldRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Sampled Hold는 이후에도 LostFinal 유지"), GuideSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestTrue(TEXT("Sampled Hold는 Actor 이동 뒤에도 고정점 불변"), GuideSnapshot.TargetLocation.Equals(FrozenSampledHoldLocation, 0.01f));
	TestTrue(TEXT("LostFinal 뒤 Sampled observer도 더 이상 새 Actor 위치를 읽지 않음"), GuideSnapshot.EstimatedTargetLocation.Equals(FrozenSampledHoldLocation, 0.01f));

	SampledHoldRig.MissileGuideComponent->ResetMissileGuidance();
	GuideSnapshot = SampledHoldRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Sampled Pool Reset 후 SeekerState Inactive"), GuideSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestTrue(TEXT("Sampled Pool Reset 후 LastObserved Zero"), GuideSnapshot.LastObservedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("Sampled Pool Reset 후 Estimated Zero"), GuideSnapshot.EstimatedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("Sampled Pool Reset 후 VelocityEstimate Zero"), GuideSnapshot.FilteredTargetVelocityEstimate.IsNearlyZero());
	TestTrue(TEXT("Sampled Pool Reset 후 ObservationAge 0"), FMath::IsNearlyZero(GuideSnapshot.ObservationAgeSeconds));
	TestFalse(TEXT("Sampled Pool Reset 후 유효 관측 없음"), GuideSnapshot.bHasValidObservation);
	TestNull(TEXT("Sampled Pool Reset 후 Guidance Target Actor 없음"), SampledHoldRig.MissileGuideComponent->GetGuidanceTargetActor());

	return true;
}

// [v1.2.0] 같은 Runtime에 서로 다른 데이터 수치를 넣었을 때 성능 Variant가 결정론적으로 갈리는지 검증합니다.
bool FCFMissileGuidanceVariantMatrixTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.2.0] Variant 비교와 실제 Blocking contact를 한 월드에서 격리해 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("MG-P0-11 Automation World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.2.0] 저성능/기준형/고성능이 동일 Target motion을 각각 독립적으로 처리할 transient 장치입니다.
	FCFSampledMissileTestRig LowPerformanceRig;
	FCFSampledMissileTestRig NormalPerformanceRig;
	FCFSampledMissileTestRig HighPerformanceRig;
	if (!TestTrue(TEXT("저성능 Variant 장치 초기화"), LowPerformanceRig.Initialize(TestWorld))
		|| !TestTrue(TEXT("기준형 Variant 장치 초기화"), NormalPerformanceRig.Initialize(TestWorld))
		|| !TestTrue(TEXT("고성능 Variant 장치 초기화"), HighPerformanceRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.2.0] Runtime 분기 없이 개별 수치만 다른 저성능 테스트 설정입니다.
	const FCFMissileGuideConfig LowPerformanceConfig = BuildLowPerformanceGuideConfig();

	// [v1.2.0] Runtime 분기 없이 개별 수치만 다른 기준형 테스트 설정입니다.
	const FCFMissileGuideConfig NormalPerformanceConfig = BuildNormalPerformanceGuideConfig();

	// [v1.2.0] Runtime 분기 없이 개별 수치만 다른 고성능 테스트 설정입니다.
	const FCFMissileGuideConfig HighPerformanceConfig = BuildHighPerformanceGuideConfig();

	// [v1.2.0] 세 Variant가 모두 Acquisition 가능하면서 횡 Guidance가 필요한 동일 20도 정지 Target 위치입니다.
	const FVector SharedInitialTargetLocation = MakeTargetLocationAtAngleDeg(20.0f);
	LowPerformanceRig.StartGuidance(LowPerformanceConfig, SharedInitialTargetLocation);
	NormalPerformanceRig.StartGuidance(NormalPerformanceConfig, SharedInitialTargetLocation);
	HighPerformanceRig.StartGuidance(HighPerformanceConfig, SharedInitialTargetLocation);
	LowPerformanceRig.AdvanceGuidance(0.01f);
	NormalPerformanceRig.AdvanceGuidance(0.01f);
	HighPerformanceRig.AdvanceGuidance(0.01f);

	TestEqual(TEXT("저성능 Variant 최초 20도 Target 획득 Tracking"), LowPerformanceRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);
	TestEqual(TEXT("기준형 Variant 최초 20도 Target 획득 Tracking"), NormalPerformanceRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);
	TestEqual(TEXT("고성능 Variant 최초 20도 Target 획득 Tracking"), HighPerformanceRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);

	// [v1.2.0] 위치 변화에 따른 추정 속도 상쇄 없이 같은 오프축 정지 engagement에서 물리/Guidance response 차이를 누적할 시간입니다.
	constexpr float SharedVariantResponseStepSeconds = 0.16f;
	LowPerformanceRig.AdvanceGuidance(SharedVariantResponseStepSeconds);
	NormalPerformanceRig.AdvanceGuidance(SharedVariantResponseStepSeconds);
	HighPerformanceRig.AdvanceGuidance(SharedVariantResponseStepSeconds);

	// [v1.2.0] 동일 engagement에서 저성능 config가 실제 적용한 Guidance Command입니다.
	const FCFMissileGuidanceCommand LowPerformanceCommand = LowPerformanceRig.MissileGuideComponent->GetGuidanceCommand();

	// [v1.2.0] 동일 engagement에서 기준형 config가 실제 적용한 Guidance Command입니다.
	const FCFMissileGuidanceCommand NormalPerformanceCommand = NormalPerformanceRig.MissileGuideComponent->GetGuidanceCommand();

	// [v1.2.0] 동일 engagement에서 고성능 config가 실제 적용한 Guidance Command입니다.
	const FCFMissileGuidanceCommand HighPerformanceCommand = HighPerformanceRig.MissileGuideComponent->GetGuidanceCommand();

	TestTrue(TEXT("저성능 동일 engagement Guidance Command 유효"), LowPerformanceCommand.bCommandValid);
	TestTrue(TEXT("기준형 동일 engagement Guidance Command 유효"), NormalPerformanceCommand.bCommandValid);
	TestTrue(TEXT("고성능 동일 engagement Guidance Command 유효"), HighPerformanceCommand.bCommandValid);
	TestTrue(TEXT("저성능 적용 Guidance가 기준형보다 약함"), LowPerformanceCommand.AppliedLateralAccelerationCmPerSecSq.Size() < NormalPerformanceCommand.AppliedLateralAccelerationCmPerSecSq.Size());
	TestTrue(TEXT("기준형 적용 Guidance가 고성능보다 약함"), NormalPerformanceCommand.AppliedLateralAccelerationCmPerSecSq.Size() < HighPerformanceCommand.AppliedLateralAccelerationCmPerSecSq.Size());
	TestTrue(TEXT("저성능 선회 반응이 기준형보다 작음"), LowPerformanceCommand.AppliedTurnRateDegPerSec < NormalPerformanceCommand.AppliedTurnRateDegPerSec);
	TestTrue(TEXT("기준형 선회 반응이 고성능보다 작음"), NormalPerformanceCommand.AppliedTurnRateDegPerSec < HighPerformanceCommand.AppliedTurnRateDegPerSec);
	TestTrue(TEXT("저성능 Guidance가 자체 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(LowPerformanceCommand, LowPerformanceConfig));
	TestTrue(TEXT("기준형 Guidance가 자체 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(NormalPerformanceCommand, NormalPerformanceConfig));
	TestTrue(TEXT("고성능 Guidance가 자체 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(HighPerformanceCommand, HighPerformanceConfig));

	AddInfo(FString::Printf(
		TEXT("MG-P0-11 Same Engagement Response: Low Accel=%.1f Turn=%.2f, Normal Accel=%.1f Turn=%.2f, High Accel=%.1f Turn=%.2f"),
		LowPerformanceCommand.AppliedLateralAccelerationCmPerSecSq.Size(),
		LowPerformanceCommand.AppliedTurnRateDegPerSec,
		NormalPerformanceCommand.AppliedLateralAccelerationCmPerSecSq.Size(),
		NormalPerformanceCommand.AppliedTurnRateDegPerSec,
		HighPerformanceCommand.AppliedLateralAccelerationCmPerSecSq.Size(),
		HighPerformanceCommand.AppliedTurnRateDegPerSec));

	// [v1.2.1] 동일 이동·반전 Target에서 관측 주기와 속도 추정 응답 차이를 직접 비교할 저성능 장치입니다.
	FCFSampledMissileTestRig LowMovingTargetRig;

	// [v1.2.1] 동일 이동·반전 Target에서 관측 주기와 속도 추정 응답 차이를 직접 비교할 기준형 장치입니다.
	FCFSampledMissileTestRig NormalMovingTargetRig;

	// [v1.2.1] 동일 이동·반전 Target에서 관측 주기와 속도 추정 응답 차이를 직접 비교할 고성능 장치입니다.
	FCFSampledMissileTestRig HighMovingTargetRig;
	if (!TestTrue(TEXT("저성능 Moving Target 장치 초기화"), LowMovingTargetRig.Initialize(TestWorld))
		|| !TestTrue(TEXT("기준형 Moving Target 장치 초기화"), NormalMovingTargetRig.Initialize(TestWorld))
		|| !TestTrue(TEXT("고성능 Moving Target 장치 초기화"), HighMovingTargetRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.2.1] 세 Variant가 동일 시각축에서 출발할 정면 Target seed입니다.
	const FVector SharedMovingTargetSeedLocation(10000.0f, 0.0f, 0.0f);
	LowMovingTargetRig.StartGuidance(LowPerformanceConfig, SharedMovingTargetSeedLocation);
	NormalMovingTargetRig.StartGuidance(NormalPerformanceConfig, SharedMovingTargetSeedLocation);
	HighMovingTargetRig.StartGuidance(HighPerformanceConfig, SharedMovingTargetSeedLocation);
	LowMovingTargetRig.AdvanceGuidance(0.01f);
	NormalMovingTargetRig.AdvanceGuidance(0.01f);
	HighMovingTargetRig.AdvanceGuidance(0.01f);

	// [v1.2.1] 세 Variant에 동일하게 적용할 첫 +Y 측면 이동 위치입니다.
	const FVector SharedMovingTargetPositiveLocation(10000.0f, 600.0f, 0.0f);
	LowMovingTargetRig.TargetActor->SetActorLocation(SharedMovingTargetPositiveLocation);
	NormalMovingTargetRig.TargetActor->SetActorLocation(SharedMovingTargetPositiveLocation);
	HighMovingTargetRig.TargetActor->SetActorLocation(SharedMovingTargetPositiveLocation);
	LowMovingTargetRig.AdvanceGuidance(0.04f);
	NormalMovingTargetRig.AdvanceGuidance(0.04f);
	HighMovingTargetRig.AdvanceGuidance(0.04f);

	// [v1.2.1] 발사 후 누적 0.05초 시점에 각 Variant가 실제로 관측한 Target 상태입니다.
	const FCFMissileGuideSnapshot LowPositiveMotionSnapshot = LowMovingTargetRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.2.1] 발사 후 누적 0.05초 시점에 기준형 Variant가 실제로 관측한 Target 상태입니다.
	const FCFMissileGuideSnapshot NormalPositiveMotionSnapshot = NormalMovingTargetRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.2.1] 발사 후 누적 0.05초 시점에 고성능 Variant가 실제로 관측한 Target 상태입니다.
	const FCFMissileGuideSnapshot HighPositiveMotionSnapshot = HighMovingTargetRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("0.05초 시점 저성능은 0.15초 interval 때문에 seed 위치 유지"), FMath::IsNearlyZero(LowPositiveMotionSnapshot.LastObservedTargetLocation.Y, 0.01f));
	TestTrue(TEXT("0.05초 시점 기준형은 0.08초 interval 때문에 seed 위치 유지"), FMath::IsNearlyZero(NormalPositiveMotionSnapshot.LastObservedTargetLocation.Y, 0.01f));
	TestTrue(TEXT("0.05초 시점 고성능은 0.03초 interval로 +Y 이동을 먼저 관측"), FMath::IsNearlyEqual(HighPositiveMotionSnapshot.LastObservedTargetLocation.Y, 600.0f, 0.01f));
	TestTrue(TEXT("0.05초 시점 고성능은 +Y 위치 차분 속도를 먼저 추정"), HighPositiveMotionSnapshot.FilteredTargetVelocityEstimate.Y > 0.0f);
	TestTrue(TEXT("0.05초 시점 저성능 속도 추정은 아직 Zero"), FMath::IsNearlyZero(LowPositiveMotionSnapshot.FilteredTargetVelocityEstimate.Y, 0.01f));
	TestTrue(TEXT("0.05초 시점 기준형 속도 추정은 아직 Zero"), FMath::IsNearlyZero(NormalPositiveMotionSnapshot.FilteredTargetVelocityEstimate.Y, 0.01f));

	// [v1.2.1] 같은 Target이 +Y에서 -Y로 방향을 반전한 동일 두 번째 위치입니다.
	const FVector SharedMovingTargetReversalLocation(10000.0f, -600.0f, 0.0f);
	LowMovingTargetRig.TargetActor->SetActorLocation(SharedMovingTargetReversalLocation);
	NormalMovingTargetRig.TargetActor->SetActorLocation(SharedMovingTargetReversalLocation);
	HighMovingTargetRig.TargetActor->SetActorLocation(SharedMovingTargetReversalLocation);
	LowMovingTargetRig.AdvanceGuidance(0.04f);
	NormalMovingTargetRig.AdvanceGuidance(0.04f);
	HighMovingTargetRig.AdvanceGuidance(0.04f);

	// [v1.2.1] 누적 0.09초 반전 시점의 저성능 Sensor Truth입니다.
	const FCFMissileGuideSnapshot LowReversalSnapshot = LowMovingTargetRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.2.1] 누적 0.09초 반전 시점의 기준형 Sensor Truth입니다.
	const FCFMissileGuideSnapshot NormalReversalSnapshot = NormalMovingTargetRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.2.1] 누적 0.09초 반전 시점의 고성능 Sensor Truth입니다.
	const FCFMissileGuideSnapshot HighReversalSnapshot = HighMovingTargetRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("0.09초 반전 시점 저성능은 아직 launch seed 위치 유지"), FMath::IsNearlyZero(LowReversalSnapshot.LastObservedTargetLocation.Y, 0.01f));
	TestTrue(TEXT("0.09초 반전 시점 기준형은 -Y 반전 위치를 첫 관측"), FMath::IsNearlyEqual(NormalReversalSnapshot.LastObservedTargetLocation.Y, -600.0f, 0.01f));
	TestTrue(TEXT("0.09초 반전 시점 고성능은 -Y 반전 위치를 재관측"), FMath::IsNearlyEqual(HighReversalSnapshot.LastObservedTargetLocation.Y, -600.0f, 0.01f));
	TestTrue(TEXT("반전 직후 고성능 속도 추정은 기준형보다 더 빠르게 -Y로 전환"), HighReversalSnapshot.FilteredTargetVelocityEstimate.Y < NormalReversalSnapshot.FilteredTargetVelocityEstimate.Y);
	TestTrue(TEXT("반전 직후 기준형 속도 추정은 아직 미관측 저성능보다 -Y 반응이 빠름"), NormalReversalSnapshot.FilteredTargetVelocityEstimate.Y < LowReversalSnapshot.FilteredTargetVelocityEstimate.Y);

	// [v1.2.1] 같은 반전 자극에서 각 Variant Runtime이 실제 적용한 Guidance Command입니다.
	const FCFMissileGuidanceCommand LowReversalCommand = LowMovingTargetRig.MissileGuideComponent->GetGuidanceCommand();

	// [v1.2.1] 같은 반전 자극에서 기준형 Runtime이 실제 적용한 Guidance Command입니다.
	const FCFMissileGuidanceCommand NormalReversalCommand = NormalMovingTargetRig.MissileGuideComponent->GetGuidanceCommand();

	// [v1.2.1] 같은 반전 자극에서 고성능 Runtime이 실제 적용한 Guidance Command입니다.
	const FCFMissileGuidanceCommand HighReversalCommand = HighMovingTargetRig.MissileGuideComponent->GetGuidanceCommand();
	TestTrue(TEXT("이동·반전 저성능 Guidance Command 유효"), LowReversalCommand.bCommandValid);
	TestTrue(TEXT("이동·반전 기준형 Guidance Command 유효"), NormalReversalCommand.bCommandValid);
	TestTrue(TEXT("이동·반전 고성능 Guidance Command 유효"), HighReversalCommand.bCommandValid);
	TestTrue(TEXT("이동·반전에서 저성능 적용 Guidance가 기준형보다 약함"), LowReversalCommand.AppliedLateralAccelerationCmPerSecSq.Size() < NormalReversalCommand.AppliedLateralAccelerationCmPerSecSq.Size());
	TestTrue(TEXT("이동·반전에서 기준형 적용 Guidance가 고성능보다 약함"), NormalReversalCommand.AppliedLateralAccelerationCmPerSecSq.Size() < HighReversalCommand.AppliedLateralAccelerationCmPerSecSq.Size());
	TestTrue(TEXT("이동·반전 저성능도 자체 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(LowReversalCommand, LowPerformanceConfig));
	TestTrue(TEXT("이동·반전 기준형도 자체 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(NormalReversalCommand, NormalPerformanceConfig));
	TestTrue(TEXT("이동·반전 고성능도 자체 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(HighReversalCommand, HighPerformanceConfig));

	AddInfo(FString::Printf(
		TEXT("MG-P0-11 Moving Reversal: Low ObsY=%.1f VelY=%.1f Accel=%.1f, Normal ObsY=%.1f VelY=%.1f Accel=%.1f, High ObsY=%.1f VelY=%.1f Accel=%.1f"),
		LowReversalSnapshot.LastObservedTargetLocation.Y,
		LowReversalSnapshot.FilteredTargetVelocityEstimate.Y,
		LowReversalCommand.AppliedLateralAccelerationCmPerSecSq.Size(),
		NormalReversalSnapshot.LastObservedTargetLocation.Y,
		NormalReversalSnapshot.FilteredTargetVelocityEstimate.Y,
		NormalReversalCommand.AppliedLateralAccelerationCmPerSecSq.Size(),
		HighReversalSnapshot.LastObservedTargetLocation.Y,
		HighReversalSnapshot.FilteredTargetVelocityEstimate.Y,
		HighReversalCommand.AppliedLateralAccelerationCmPerSecSq.Size()));

	// [v1.2.0] 좁은 저성능 Tracking cone만 상실하고 기준형/고성능은 유지해야 하는 동일 50도 Target 위치입니다.
	const FVector SharedFiftyDegreeTargetLocation = MakeTargetLocationAtAngleDeg(50.0f);
	LowPerformanceRig.TargetActor->SetActorLocation(SharedFiftyDegreeTargetLocation);
	NormalPerformanceRig.TargetActor->SetActorLocation(SharedFiftyDegreeTargetLocation);
	HighPerformanceRig.TargetActor->SetActorLocation(SharedFiftyDegreeTargetLocation);
	LowPerformanceRig.AdvanceGuidance(0.16f);
	NormalPerformanceRig.AdvanceGuidance(0.16f);
	HighPerformanceRig.AdvanceGuidance(0.16f);

	TestEqual(TEXT("저성능은 50도 Tracking 이탈 후 Reacquisition None으로 LostFinal"), LowPerformanceRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("저성능 Tracking 상실은 정상 TargetLost"), LowPerformanceRig.MissileGuideComponent->GetGuideSnapshot().MissReason, ECFMissileMissReason::TargetLost);
	TestEqual(TEXT("기준형은 동일 50도 기하에서 Tracking 유지"), NormalPerformanceRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);
	TestEqual(TEXT("고성능은 동일 50도 기하에서 Tracking 유지"), HighPerformanceRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);

	// [v1.2.0] Reacquisition=None과 ForwardCone 차이를 같은 Target 기하로 비교할 저성능 장치입니다.
	FCFSampledMissileTestRig NoReacquisitionVariantRig;

	// [v1.2.0] Reacquisition=None과 ForwardCone 차이를 같은 Target 기하로 비교할 기준형 장치입니다.
	FCFSampledMissileTestRig ForwardReacquisitionVariantRig;
	if (!TestTrue(TEXT("Variant Reacquisition None 장치 초기화"), NoReacquisitionVariantRig.Initialize(TestWorld))
		|| !TestTrue(TEXT("Variant ForwardCone 장치 초기화"), ForwardReacquisitionVariantRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.2.0] 두 Variant가 최초 Tracking을 성립시킬 동일 20도 Target 위치입니다.
	const FVector SharedTwentyDegreeTargetLocation = MakeTargetLocationAtAngleDeg(20.0f);
	NoReacquisitionVariantRig.StartGuidance(LowPerformanceConfig, SharedTwentyDegreeTargetLocation);
	ForwardReacquisitionVariantRig.StartGuidance(NormalPerformanceConfig, SharedTwentyDegreeTargetLocation);
	NoReacquisitionVariantRig.AdvanceGuidance(0.01f);
	ForwardReacquisitionVariantRig.AdvanceGuidance(0.01f);

	// [v1.2.0] 두 Variant 모두 Tracking cone을 확실히 벗어나게 할 동일 80도 Target 위치입니다.
	const FVector SharedEightyDegreeTargetLocation = MakeTargetLocationAtAngleDeg(80.0f);
	NoReacquisitionVariantRig.TargetActor->SetActorLocation(SharedEightyDegreeTargetLocation);
	ForwardReacquisitionVariantRig.TargetActor->SetActorLocation(SharedEightyDegreeTargetLocation);
	NoReacquisitionVariantRig.AdvanceGuidance(0.41f);
	ForwardReacquisitionVariantRig.AdvanceGuidance(0.41f);

	TestEqual(TEXT("저성능 None은 grace 종료 후 LostFinal"), NoReacquisitionVariantRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("기준형 ForwardCone은 grace 종료 후 Reacquiring"), ForwardReacquisitionVariantRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Reacquiring);

	// [v1.2.0] 기준형 ForwardCone 45도 안으로 같은 Launch Snapshot Target을 되돌릴 위치입니다.
	const FVector SharedFortyDegreeTargetLocation = MakeTargetLocationAtAngleDeg(40.0f);
	NoReacquisitionVariantRig.TargetActor->SetActorLocation(SharedFortyDegreeTargetLocation);
	ForwardReacquisitionVariantRig.TargetActor->SetActorLocation(SharedFortyDegreeTargetLocation);
	NoReacquisitionVariantRig.AdvanceGuidance(0.08f);
	ForwardReacquisitionVariantRig.AdvanceGuidance(0.08f);

	TestEqual(TEXT("Reacquisition None은 Target 복귀 뒤에도 LostFinal 유지"), NoReacquisitionVariantRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("ForwardCone은 같은 Snapshot Target이 40도로 복귀하면 Tracking 회복"), ForwardReacquisitionVariantRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);

	// [v1.2.0] Variant 자극 뒤 Pool-style Reset이 상태 오염을 남기지 않는지 세 대표 장치에서 확인합니다.
	LowPerformanceRig.MissileGuideComponent->ResetMissileGuidance();
	NormalPerformanceRig.MissileGuideComponent->ResetMissileGuidance();
	HighPerformanceRig.MissileGuideComponent->ResetMissileGuidance();

	// [v1.2.0] Reset 직후 저성능 observer/seeker 상태입니다.
	const FCFMissileGuideSnapshot LowResetSnapshot = LowPerformanceRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.2.0] Reset 직후 기준형 observer/seeker 상태입니다.
	const FCFMissileGuideSnapshot NormalResetSnapshot = NormalPerformanceRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.2.0] Reset 직후 고성능 observer/seeker 상태입니다.
	const FCFMissileGuideSnapshot HighResetSnapshot = HighPerformanceRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("저성능 Reset Seeker Inactive"), LowResetSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestEqual(TEXT("기준형 Reset Seeker Inactive"), NormalResetSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestEqual(TEXT("고성능 Reset Seeker Inactive"), HighResetSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestFalse(TEXT("저성능 Reset observation valid 제거"), LowResetSnapshot.bHasValidObservation);
	TestFalse(TEXT("기준형 Reset observation valid 제거"), NormalResetSnapshot.bHasValidObservation);
	TestFalse(TEXT("고성능 Reset observation valid 제거"), HighResetSnapshot.bHasValidObservation);
	TestTrue(TEXT("저성능 Reset EstimatedTarget Zero"), LowResetSnapshot.EstimatedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("기준형 Reset EstimatedTarget Zero"), NormalResetSnapshot.EstimatedTargetLocation.IsNearlyZero());
	TestTrue(TEXT("고성능 Reset EstimatedTarget Zero"), HighResetSnapshot.EstimatedTargetLocation.IsNearlyZero());
	TestNull(TEXT("저성능 Reset Guidance Target 제거"), LowPerformanceRig.MissileGuideComponent->GetGuidanceTargetActor());
	TestNull(TEXT("기준형 Reset Guidance Target 제거"), NormalPerformanceRig.MissileGuideComponent->GetGuidanceTargetActor());
	TestNull(TEXT("고성능 Reset Guidance Target 제거"), HighPerformanceRig.MissileGuideComponent->GetGuidanceTargetActor());

	// [v1.2.0] 쉬운 정면 정지 Target에서 실제 ProjectileMovement Blocking 접촉을 검증할 transient 타겟입니다.
	ACFMissileTestTarget* StraightBlockingTargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>();
	if (!TestNotNull(TEXT("정면 Blocking Target 생성"), StraightBlockingTargetActor))
	{
		return false;
	}
	StraightBlockingTargetActor->SetActorLocation(FVector(1000.0f, 0.0f, 2000.0f));
	StraightBlockingTargetActor->bBlockProjectileHits = true;
	StraightBlockingTargetActor->RefreshTestTargetConfig();

	// [v1.2.0] 저장 Asset 없이 실제 ACFProjectileActor 충돌 경로를 실행할 transient Missile Actor입니다.
	ACFProjectileActor* StraightBlockingMissileActor = TestWorld->SpawnActor<ACFProjectileActor>();
	if (!TestNotNull(TEXT("정면 Blocking Missile Actor 생성"), StraightBlockingMissileActor))
	{
		StraightBlockingTargetActor->Destroy();
		return false;
	}
	StraightBlockingMissileActor->SetDestroyWhenDeactivated(false);

	// [v1.2.0] 쉬운 정면 교전에서 기준형 Guidance config를 실제 Projectile Actor에 연결할 transient ProjectileData입니다.
	UCFProjectileData* StraightBlockingProjectileData = NewObject<UCFProjectileData>(StraightBlockingMissileActor, TEXT("VariantStraightBlockingProjectileData"));
	StraightBlockingProjectileData->InitialSpeed = 1500.0f;
	StraightBlockingProjectileData->LifeTimeSeconds = 5.0f;
	StraightBlockingProjectileData->bAffectedByGravity = false;
	StraightBlockingProjectileData->bUseSupplementalContinuousSweep = false;
	StraightBlockingProjectileData->PropulsionConfig.bUsePropulsion = false;
	StraightBlockingProjectileData->MissileFlightConfig.bUseMissileFlight = true;
	StraightBlockingProjectileData->MissileFlightConfig.AttackProfile = ECFMissileAttackProfile::Direct;
	StraightBlockingProjectileData->MissileFlightConfig.MinimumClearanceTimeSeconds = 0.0f;
	StraightBlockingProjectileData->MissileFlightConfig.MinimumClearanceDistanceCm = 0.0f;
	StraightBlockingProjectileData->MissileGuideConfig = NormalPerformanceConfig;

	// [v1.2.0] 정면 Target Snapshot과 정확한 +X 초기 속도를 전달할 실제 Projectile Launch Context입니다.
	FCFProjectileLaunchContext StraightBlockingLaunchContext;
	StraightBlockingLaunchContext.LaunchTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 2000.0f));
	StraightBlockingLaunchContext.InitialLaunchDirection = FVector::ForwardVector;
	StraightBlockingLaunchContext.InitialLaunchVelocity = FVector(1500.0f, 0.0f, 0.0f);
	StraightBlockingLaunchContext.CommandTargetLocation = StraightBlockingTargetActor->GetActorLocation();
	StraightBlockingLaunchContext.GuidanceTargetActor = StraightBlockingTargetActor;
	StraightBlockingLaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;
	StraightBlockingMissileActor->ActivateProjectileWithContext(
		StraightBlockingProjectileData,
		StraightBlockingLaunchContext,
		nullptr);

	// [v1.2.0] 실제 ACFProjectileActor에 내장된 production ProjectileMovement 컴포넌트입니다.
	UProjectileMovementComponent* StraightBlockingProjectileMovement = StraightBlockingMissileActor->FindComponentByClass<UProjectileMovementComponent>();
	if (!TestNotNull(TEXT("정면 Blocking Missile ProjectileMovement 존재"), StraightBlockingProjectileMovement))
	{
		StraightBlockingMissileActor->Destroy();
		StraightBlockingTargetActor->Destroy();
		return false;
	}

	// [v1.2.0] 순간이동 없이 production ProjectileMovement를 직접 진행할 고정 60Hz 간격입니다.
	constexpr float StraightBlockingSimulationStepSeconds = 1.0f / 60.0f;

	// [v1.2.0] 1000cm 정면 Target에 도달하기 충분한 최대 시뮬레이션 프레임 수입니다.
	constexpr int32 MaximumStraightBlockingSimulationSteps = 120;

	// [v1.2.0] 실제 Blocking으로 Velocity가 0이 될 때까지 실행한 movement frame 수입니다.
	int32 StraightBlockingExecutedSteps = 0;
	while (StraightBlockingMissileActor->IsProjectileActive()
		&& StraightBlockingExecutedSteps < MaximumStraightBlockingSimulationSteps)
	{
		StraightBlockingProjectileMovement->TickComponent(
			StraightBlockingSimulationStepSeconds,
			LEVELTICK_All,
			nullptr);
		++StraightBlockingExecutedSteps;
		if (StraightBlockingProjectileMovement->Velocity.IsNearlyZero())
		{
			break;
		}
	}

	// [v1.2.0] 실제 production ProjectileMovement가 정지한 정면 Blocking 접촉 위치입니다.
	const FVector StraightBlockingContactLocation = StraightBlockingMissileActor->GetActorLocation();

	// [v1.2.0] Target 중심에서 실제 Blocking 접촉 위치까지의 거리입니다.
	const float StraightBlockingCenterDistance = FVector::Dist(
		StraightBlockingContactLocation,
		StraightBlockingTargetActor->GetActorLocation());
	TestTrue(TEXT("쉬운 정면 Target 실제 이동 최소 1 frame"), StraightBlockingExecutedSteps > 0);
	TestTrue(TEXT("쉬운 정면 Target production ProjectileMovement 실제 전진"), StraightBlockingContactLocation.X > 1.0f);
	TestTrue(TEXT("쉬운 정면 Target은 Blocking으로 Projectile Velocity 0"), StraightBlockingProjectileMovement->Velocity.IsNearlyZero());
	TestTrue(TEXT("쉬운 정면 Target은 collision volume 경계에서 Blocking"), StraightBlockingCenterDistance <= 150.0f);

	AddInfo(FString::Printf(
		TEXT("MG-P0-11 Straight Blocking Contact: Steps=%d, Contact=(%.1f, %.1f, %.1f), Target=(%.1f, %.1f, %.1f), CenterDistance=%.1f"),
		StraightBlockingExecutedSteps,
		StraightBlockingContactLocation.X,
		StraightBlockingContactLocation.Y,
		StraightBlockingContactLocation.Z,
		StraightBlockingTargetActor->GetActorLocation().X,
		StraightBlockingTargetActor->GetActorLocation().Y,
		StraightBlockingTargetActor->GetActorLocation().Z,
		StraightBlockingCenterDistance));

	if (StraightBlockingMissileActor->IsProjectileActive())
	{
		StraightBlockingMissileActor->DeactivateProjectile();
	}
	StraightBlockingMissileActor->Destroy();
	StraightBlockingTargetActor->Destroy();
	return true;
}

// [v1.3.0] Guidance Law별 정보 소비, bounded aim point, PN Course Capture와 공통 물리 제한·Reset 계약을 검증합니다.
bool FCFMissileGuidanceLawContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.3.0] MG-P0-12C의 모든 transient Guidance Law 시나리오를 격리해 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("MG-P0-12C Automation World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.3.0] PurePursuit가 predictive EstimatedTargetLocation 대신 마지막 실제 관측 위치만 향하는지 검증할 장치입니다.
	FCFSampledMissileTestRig PurePursuitRig;
	if (!TestTrue(TEXT("PurePursuit 장치 초기화"), PurePursuitRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.3.0] 관측 사이 외삽을 의도적으로 만들면서 Guidance Law만 PurePursuit로 바꾼 설정입니다.
	FCFMissileGuideConfig PurePursuitConfig = BuildSampledGuideConfig(0.10f, 0.10f);
	PurePursuitConfig.GuidanceLaw = ECFMissileGuidanceLaw::PurePursuit;
	PurePursuitConfig.MaximumTurnRateDegPerSec = 90.0f;
	PurePursuitConfig.MaximumLateralAccelerationCmPerSecSq = 10000.0f;
	PurePursuitRig.StartGuidance(PurePursuitConfig, FVector(10000.0f, 0.0f, 0.0f));
	PurePursuitRig.AdvanceGuidance(0.01f);
	PurePursuitRig.TargetActor->SetActorLocation(FVector(10000.0f, 200.0f, 0.0f));
	PurePursuitRig.AdvanceGuidance(0.10f);
	PurePursuitRig.TargetActor->SetActorLocation(FVector(10000.0f, 800.0f, 0.0f));
	PurePursuitRig.AdvanceGuidance(0.05f);

	// [v1.3.0] 관측 사이 외삽이 존재하는 시점의 PurePursuit Sensor/Guidance Snapshot입니다.
	const FCFMissileGuideSnapshot PurePursuitSnapshot = PurePursuitRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("PurePursuit Snapshot Law"), PurePursuitSnapshot.GuidanceLaw, ECFMissileGuidanceLaw::PurePursuit);
	TestTrue(TEXT("PurePursuit는 마지막 실제 관측 위치를 AimPoint로 사용"), PurePursuitSnapshot.GuidanceAimPoint.Equals(PurePursuitSnapshot.LastObservedTargetLocation, 0.01f));
	TestTrue(TEXT("PurePursuit는 외삽 EstimatedTargetLocation을 AimPoint로 사용하지 않음"), !PurePursuitSnapshot.GuidanceAimPoint.Equals(PurePursuitSnapshot.EstimatedTargetLocation, 0.5f));
	TestTrue(TEXT("PurePursuit Guidance Command 유효"), PurePursuitRig.MissileGuideComponent->GetGuidanceCommand().bCommandValid);
	TestTrue(TEXT("PurePursuit 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(PurePursuitRig.MissileGuideComponent->GetGuidanceCommand(), PurePursuitConfig));

	// [v1.3.0] LeadPursuit가 LastObserved 위치에 제한된 Target velocity lead만 더하는지 검증할 장치입니다.
	FCFSampledMissileTestRig LeadPursuitRig;
	if (!TestTrue(TEXT("LeadPursuit 장치 초기화"), LeadPursuitRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.3.0] 계산된 선행 오프셋이 100cm 상한에 걸리도록 설정한 LeadPursuit Config입니다.
	FCFMissileGuideConfig LeadPursuitConfig = BuildSampledGuideConfig(0.10f, 0.10f);
	LeadPursuitConfig.GuidanceLaw = ECFMissileGuidanceLaw::LeadPursuit;
	LeadPursuitConfig.LeadTimeSeconds = 1.0f;
	LeadPursuitConfig.MaxLeadDistanceCm = 100.0f;
	LeadPursuitConfig.MaximumTurnRateDegPerSec = 90.0f;
	LeadPursuitConfig.MaximumLateralAccelerationCmPerSecSq = 10000.0f;
	LeadPursuitRig.StartGuidance(LeadPursuitConfig, FVector(10000.0f, 0.0f, 0.0f));
	LeadPursuitRig.AdvanceGuidance(0.01f);
	LeadPursuitRig.TargetActor->SetActorLocation(FVector(10000.0f, 400.0f, 0.0f));
	LeadPursuitRig.AdvanceGuidance(0.10f);

	// [v1.3.0] 새 위치 Sample에서 실제 위치 차분 속도가 생성된 LeadPursuit Snapshot입니다.
	const FCFMissileGuideSnapshot LeadPursuitSnapshot = LeadPursuitRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.3.0] Snapshot의 FilteredTargetVelocityEstimate와 Config로 독립 계산한 bounded lead 오프셋입니다.
	const FVector ExpectedLeadOffset = (LeadPursuitSnapshot.FilteredTargetVelocityEstimate * LeadPursuitConfig.LeadTimeSeconds)
		.GetClampedToMaxSize(LeadPursuitConfig.MaxLeadDistanceCm);

	// [v1.3.0] 설계 계약인 LastObserved + bounded lead로 계산한 기대 Guidance AimPoint입니다.
	const FVector ExpectedLeadAimPoint = LeadPursuitSnapshot.LastObservedTargetLocation + ExpectedLeadOffset;
	TestEqual(TEXT("LeadPursuit Snapshot Law"), LeadPursuitSnapshot.GuidanceLaw, ECFMissileGuidanceLaw::LeadPursuit);
	TestTrue(TEXT("LeadPursuit AimPoint = LastObserved + bounded lead"), LeadPursuitSnapshot.GuidanceAimPoint.Equals(ExpectedLeadAimPoint, 0.1f));
	TestTrue(TEXT("LeadPursuit 선행 오프셋은 MaxLeadDistance 이하"), FVector::Dist(LeadPursuitSnapshot.GuidanceAimPoint, LeadPursuitSnapshot.LastObservedTargetLocation) <= LeadPursuitConfig.MaxLeadDistanceCm + 0.1f);
	TestTrue(TEXT("LeadPursuit 실제 선행 오프셋 생성"), FVector::Dist(LeadPursuitSnapshot.GuidanceAimPoint, LeadPursuitSnapshot.LastObservedTargetLocation) > 1.0f);
	TestTrue(TEXT("LeadPursuit Guidance Command 유효"), LeadPursuitRig.MissileGuideComponent->GetGuidanceCommand().bCommandValid);
	TestTrue(TEXT("LeadPursuit 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(LeadPursuitRig.MissileGuideComponent->GetGuidanceCommand(), LeadPursuitConfig));

	// [v1.3.0] 기존 호환 기본 Law인 PN이 정상 접근 기하에서는 기존 PN 경로를 계속 쓰는지 검증할 장치입니다.
	FCFStatefulMissileTestRig ProportionalNavigationRig;
	if (!TestTrue(TEXT("PN 접근 기하 장치 초기화"), ProportionalNavigationRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.3.0] 20도 Target을 충분히 획득하면서 기존 PN을 명시적으로 사용하는 설정입니다.
	FCFMissileGuideConfig ProportionalNavigationConfig = BuildStatefulGuideConfig();
	ProportionalNavigationConfig.GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;
	ProportionalNavigationConfig.AcquisitionConeHalfAngleDeg = 180.0f;
	ProportionalNavigationConfig.TrackingConeHalfAngleDeg = 180.0f;
	ProportionalNavigationRig.StartGuidance(ProportionalNavigationConfig, MakeTargetLocationAtAngleDeg(20.0f));
	ProportionalNavigationRig.AdvanceGuidance(0.01f);

	// [v1.3.0] Target 방향 전방 속도 성분이 양수인 정상 접근 기하의 PN Snapshot입니다.
	const FCFMissileGuideSnapshot ProportionalNavigationSnapshot = ProportionalNavigationRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("기존 호환 Guidance Law는 PN"), ProportionalNavigationSnapshot.GuidanceLaw, ECFMissileGuidanceLaw::ProportionalNavigation);
	TestFalse(TEXT("정상 접근 기하에서는 Course Capture 비활성"), ProportionalNavigationSnapshot.bCourseCaptureActive);
	TestTrue(TEXT("정상 접근 기하 PN Command 유효"), ProportionalNavigationRig.MissileGuideComponent->GetGuidanceCommand().bCommandValid);

	// [v1.3.0] PN의 기존 ClosingSpeed=0 기하에서도 bounded Course Capture command가 생성되는지 검증할 rear-aspect 장치입니다.
	FCFStatefulMissileTestRig CourseCaptureRig;
	if (!TestTrue(TEXT("PN Course Capture 장치 초기화"), CourseCaptureRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.3.0] 150도 rear target을 Stateful Seeker가 획득할 수 있게 열고 실제 선회 물리 상한은 유한하게 유지한 PN 설정입니다.
	FCFMissileGuideConfig CourseCaptureConfig = BuildStatefulGuideConfig();
	CourseCaptureConfig.GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;
	CourseCaptureConfig.AcquisitionConeHalfAngleDeg = 180.0f;
	CourseCaptureConfig.TrackingConeHalfAngleDeg = 180.0f;
	CourseCaptureConfig.MaximumTurnRateDegPerSec = 45.0f;
	CourseCaptureConfig.MaximumLateralAccelerationCmPerSecSq = 5000.0f;
	CourseCaptureRig.StartGuidance(CourseCaptureConfig, MakeTargetLocationAtAngleDeg(150.0f));
	CourseCaptureRig.AdvanceGuidance(0.01f);

	// [v1.3.0] 첫 rear-aspect Tracking Step에서 Overshoot 이전 접근 이력이 없어 Course Capture를 직접 관측할 Snapshot입니다.
	const FCFMissileGuideSnapshot CourseCaptureSnapshot = CourseCaptureRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.3.0] 같은 Step에 실제 Runtime이 만든 bounded Course Capture Guidance Command입니다.
	const FCFMissileGuidanceCommand CourseCaptureCommand = CourseCaptureRig.MissileGuideComponent->GetGuidanceCommand();
	TestEqual(TEXT("rear-aspect도 Guidance Law는 PN 유지"), CourseCaptureSnapshot.GuidanceLaw, ECFMissileGuidanceLaw::ProportionalNavigation);
	TestTrue(TEXT("rear/non-closing PN은 Course Capture 활성"), CourseCaptureSnapshot.bCourseCaptureActive);
	TestTrue(TEXT("rear/non-closing Course Capture Command 유효"), CourseCaptureCommand.bCommandValid);
	TestTrue(TEXT("150도 rear target Course Capture는 실제 횡가속 생성"), CourseCaptureCommand.AppliedLateralAccelerationCmPerSecSq.Size() > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Course Capture도 기존 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(CourseCaptureCommand, CourseCaptureConfig));
	TestTrue(TEXT("Course Capture AimPoint는 현재 Sensor Truth Target"), CourseCaptureSnapshot.GuidanceAimPoint.Equals(CourseCaptureSnapshot.TargetLocation, 0.01f));

	// [v1.3.1] 같은 activation 안에서 실제 Course Capture 선회가 접근 기하를 만들고 PN으로 복귀하는지 확인할 최대 simulation step 수입니다.
	constexpr int32 MaximumCourseCaptureTransitionSteps = 600;

	// [v1.3.1] Course Capture에서 정상 PN으로 실제 전환할 때까지 실행된 guidance step 수입니다.
	int32 CourseCaptureTransitionExecutedSteps = 0;

	// [v1.3.1] 동일 activation 안에서 Course Capture가 끝나고 PN이 다시 선택됐는지 여부입니다.
	bool bReturnedToProportionalNavigation = false;
	while (CourseCaptureTransitionExecutedSteps < MaximumCourseCaptureTransitionSteps)
	{
		CourseCaptureRig.AdvanceGuidanceKeepingVelocity(0.01f);
		++CourseCaptureTransitionExecutedSteps;

		// [v1.3.1] 현재 step의 Course Capture/PN 전환 상태를 판정할 Runtime Snapshot입니다.
		const FCFMissileGuideSnapshot TransitionSnapshot = CourseCaptureRig.MissileGuideComponent->GetGuideSnapshot();
		if (!TransitionSnapshot.bCourseCaptureActive
			&& TransitionSnapshot.GuidanceLaw == ECFMissileGuidanceLaw::ProportionalNavigation
			&& CourseCaptureRig.MissileGuideComponent->GetGuidanceCommand().bCommandValid)
		{
			bReturnedToProportionalNavigation = true;
			break;
		}
	}

	TestTrue(TEXT("동일 activation에서 Course Capture 이후 PN으로 복귀"), bReturnedToProportionalNavigation);
	TestTrue(TEXT("Course Capture -> PN 전환은 bounded simulation 안에서 발생"), CourseCaptureTransitionExecutedSteps < MaximumCourseCaptureTransitionSteps);

	CourseCaptureRig.MissileGuideComponent->ResetMissileGuidance();

	// [v1.3.0] Pool-style Reset 뒤 신규 Guidance Law transient 상태가 제거됐는지 확인할 Snapshot입니다.
	const FCFMissileGuideSnapshot ResetSnapshot = CourseCaptureRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("MG-P0-12C Reset 후 GuidanceAimPoint Zero"), ResetSnapshot.GuidanceAimPoint.IsNearlyZero());
	TestFalse(TEXT("MG-P0-12C Reset 후 Course Capture 비활성"), ResetSnapshot.bCourseCaptureActive);
	TestEqual(TEXT("MG-P0-12C Reset 후 GuidanceLaw 기본 PN"), ResetSnapshot.GuidanceLaw, ECFMissileGuidanceLaw::ProportionalNavigation);

	return true;
}

// [v1.4.1] Guidance Activation, free Stateful Seeker geometry, law-independent exact rear tie-break, approach-armed Overshoot와 Reset을 검증합니다.
bool FCFMissileGuidanceActivationRearAspectContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] MG-P0-12D transient Runtime 시나리오를 격리해 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("MG-P0-12D Automation World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.4.0] 기존 FollowFlightGuidanceWindow가 닫힌 Flight Window를 그대로 따르는지 검증할 장치입니다.
	FCFStatefulMissileTestRig FollowWindowRig;
	if (!TestTrue(TEXT("Follow Flight Window 장치 초기화"), FollowWindowRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.0] 기존 Flight Guidance Window 호환 모드를 명시한 Stateful Guidance 설정입니다.
	FCFMissileGuideConfig FollowWindowConfig = BuildStatefulGuideConfig();
	FollowWindowConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow;
	FollowWindowRig.StartGuidanceWithClosedFlightWindow(FollowWindowConfig, FVector(10000.0f, 0.0f, 0.0f));
	FollowWindowRig.AdvanceGuidance(0.01f);

	// [v1.4.0] 닫힌 Flight Window를 그대로 따라 Guidance가 비활성인 호환 Snapshot입니다.
	const FCFMissileGuideSnapshot FollowWindowSnapshot = FollowWindowRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Follow 모드는 기존 Flight Window 호환"), FollowWindowSnapshot.GuidanceActivationMode, ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow);
	TestFalse(TEXT("닫힌 Flight Window에서는 Follow Guidance 미활성"), FollowWindowSnapshot.bGuidanceActivationSatisfied);
	TestEqual(TEXT("닫힌 Flight Window에서는 Stateful Seeker Inactive"), FollowWindowSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestEqual(TEXT("닫힌 Flight Window Follow 사유 GuidanceDisabled"), FollowWindowSnapshot.MissReason, ECFMissileMissReason::GuidanceDisabled);

	// [v1.4.0] Independent 0/0이 Flight Window가 닫혀 있어도 첫 유효 Guidance tick부터 열리는지 검증할 장치입니다.
	FCFStatefulMissileTestRig ImmediateIndependentRig;
	if (!TestTrue(TEXT("Independent 즉시 활성화 장치 초기화"), ImmediateIndependentRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.0] 시간·거리 조건을 모두 0으로 두어 즉시 독립 활성화를 요구하는 설정입니다.
	FCFMissileGuideConfig ImmediateIndependentConfig = BuildStatefulGuideConfig();
	ImmediateIndependentConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::Independent;
	ImmediateIndependentConfig.GuidanceActivationDelaySeconds = 0.0f;
	ImmediateIndependentConfig.GuidanceActivationDistanceCm = 0.0f;
	ImmediateIndependentRig.StartGuidanceWithClosedFlightWindow(ImmediateIndependentConfig, FVector(10000.0f, 0.0f, 0.0f));
	ImmediateIndependentRig.AdvanceGuidance(0.01f);

	// [v1.4.0] Flight Window와 독립적으로 즉시 열린 Guidance Activation Snapshot입니다.
	const FCFMissileGuideSnapshot ImmediateIndependentSnapshot = ImmediateIndependentRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("Independent Activation Mode Snapshot"), ImmediateIndependentSnapshot.GuidanceActivationMode, ECFMissileGuidanceActivationMode::Independent);
	TestTrue(TEXT("Independent 0/0은 닫힌 Flight Window에서도 즉시 활성"), ImmediateIndependentSnapshot.bGuidanceActivationSatisfied);
	TestEqual(TEXT("Independent 0/0은 즉시 Target Tracking"), ImmediateIndependentSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("Independent 0/0 Guidance Command 유효"), ImmediateIndependentRig.MissileGuideComponent->GetGuidanceCommand().bCommandValid);

	// [v1.4.0] Independent 시간·거리 AND 조건과 한 번 만족 후 latch를 검증할 장치입니다.
	FCFStatefulMissileTestRig ThresholdIndependentRig;
	if (!TestTrue(TEXT("Independent AND + latch 장치 초기화"), ThresholdIndependentRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.0] 0.05초와 100cm를 모두 만족해야 Guidance가 열리는 Independent 설정입니다.
	FCFMissileGuideConfig ThresholdIndependentConfig = BuildStatefulGuideConfig();
	ThresholdIndependentConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::Independent;
	ThresholdIndependentConfig.GuidanceActivationDelaySeconds = 0.05f;
	ThresholdIndependentConfig.GuidanceActivationDistanceCm = 100.0f;
	ThresholdIndependentRig.StartGuidanceWithClosedFlightWindow(ThresholdIndependentConfig, FVector(10000.0f, 0.0f, 0.0f));
	ThresholdIndependentRig.AdvanceGuidance(0.01f);
	TestFalse(TEXT("Independent AND 조건 시작 시 미충족"), ThresholdIndependentRig.MissileGuideComponent->GetGuideSnapshot().bGuidanceActivationSatisfied);

	ThresholdIndependentRig.MissileActor->SetActorLocation(FVector(120.0f, 0.0f, 0.0f));
	ThresholdIndependentRig.AdvanceFlight(0.01f);
	ThresholdIndependentRig.AdvanceGuidance(0.01f);
	TestFalse(TEXT("Independent는 거리만 만족하면 열리지 않음"), ThresholdIndependentRig.MissileGuideComponent->GetGuideSnapshot().bGuidanceActivationSatisfied);

	ThresholdIndependentRig.AdvanceFlight(0.05f);
	ThresholdIndependentRig.AdvanceGuidance(0.01f);

	// [v1.4.0] 시간·거리 AND 조건을 모두 만족해 latch된 Independent Snapshot입니다.
	const FCFMissileGuideSnapshot LatchedIndependentSnapshot = ThresholdIndependentRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("Independent 시간·거리 AND 만족 후 활성"), LatchedIndependentSnapshot.bGuidanceActivationSatisfied);
	TestEqual(TEXT("Independent AND 만족 후 Tracking"), LatchedIndependentSnapshot.SeekerState, ECFMissileSeekerState::Tracking);

	ThresholdIndependentRig.MissileActor->SetActorLocation(FVector::ZeroVector);
	ThresholdIndependentRig.AdvanceFlight(0.01f);
	ThresholdIndependentRig.AdvanceGuidance(0.01f);
	TestTrue(TEXT("Independent 활성화는 분리 거리가 다시 줄어도 latch 유지"), ThresholdIndependentRig.MissileGuideComponent->GetGuideSnapshot().bGuidanceActivationSatisfied);

	ThresholdIndependentRig.MissileGuideComponent->ResetMissileGuidance();

	// [v1.4.0] Pool-style Reset 뒤 activation-local latch가 제거됐는지 확인할 Snapshot입니다.
	const FCFMissileGuideSnapshot IndependentResetSnapshot = ThresholdIndependentRig.MissileGuideComponent->GetGuideSnapshot();
	TestFalse(TEXT("Reset 후 Independent activation latch 제거"), IndependentResetSnapshot.bGuidanceActivationSatisfied);
	TestEqual(TEXT("Reset 후 Activation Mode 기본 Follow"), IndependentResetSnapshot.GuidanceActivationMode, ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow);

	// [v1.4.0] Acquisition 반각이 Tracking보다 넓어도 독립적으로 최초 획득할 수 있는지 검증할 장치입니다.
	FCFStatefulMissileTestRig WideAcquisitionRig;
	if (!TestTrue(TEXT("Wide Acquisition 장치 초기화"), WideAcquisitionRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.0] Tracking은 35도지만 최초 Acquisition은 150도까지 허용하는 free geometry 설정입니다.
	FCFMissileGuideConfig WideAcquisitionConfig = BuildStatefulGuideConfig();
	WideAcquisitionConfig.AcquisitionConeHalfAngleDeg = 150.0f;
	WideAcquisitionConfig.TrackingConeHalfAngleDeg = 35.0f;
	WideAcquisitionConfig.ReacquisitionConeHalfAngleDeg = 120.0f;
	WideAcquisitionConfig.GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;
	WideAcquisitionRig.StartGuidance(WideAcquisitionConfig, MakeTargetLocationAtAngleDeg(120.0f));
	WideAcquisitionRig.AdvanceGuidance(0.01f);
	TestTrue(TEXT("Acquisition 유효 반각은 Tracking과 독립 150도"), FMath::IsNearlyEqual(WideAcquisitionConfig.GetEffectiveAcquisitionConeHalfAngleDeg(), 150.0f));
	TestTrue(TEXT("Tracking 유효 반각은 35도"), FMath::IsNearlyEqual(WideAcquisitionConfig.GetEffectiveTrackingConeHalfAngleDeg(), 35.0f));
	TestTrue(TEXT("Reacquisition 유효 반각은 Tracking과 독립 120도"), FMath::IsNearlyEqual(WideAcquisitionConfig.GetEffectiveReacquisitionConeHalfAngleDeg(), 120.0f));
	TestEqual(TEXT("Tracking보다 넓은 120도 Target도 Acquisition으로 최초 획득"), WideAcquisitionRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);

	// [v1.4.0] Reacquisition 반각이 Tracking보다 넓은 상태에서 같은 Launch Target Snapshot을 다시 획득하는지 검증할 장치입니다.
	FCFStatefulMissileTestRig WideReacquisitionRig;
	if (!TestTrue(TEXT("Wide Reacquisition 장치 초기화"), WideReacquisitionRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.0] Tracking 35도 이탈 뒤 120도 Reacquisition cone으로 같은 Target을 다시 받을 설정입니다.
	FCFMissileGuideConfig WideReacquisitionConfig = BuildStatefulGuideConfig();
	WideReacquisitionConfig.AcquisitionConeHalfAngleDeg = 30.0f;
	WideReacquisitionConfig.TrackingConeHalfAngleDeg = 35.0f;
	WideReacquisitionConfig.TargetLostGraceTimeSeconds = 0.0f;
	WideReacquisitionConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
	WideReacquisitionConfig.ReacquisitionConeHalfAngleDeg = 120.0f;
	WideReacquisitionConfig.ReacquisitionTimeSeconds = 0.50f;
	WideReacquisitionRig.StartGuidance(WideReacquisitionConfig, MakeTargetLocationAtAngleDeg(20.0f));
	WideReacquisitionRig.AdvanceGuidance(0.01f);
	WideReacquisitionRig.SetTargetAngleDeg(80.0f);
	WideReacquisitionRig.AdvanceGuidance(0.01f);
	TestEqual(TEXT("Tracking 이탈 뒤 Wide Reacquisition은 Reacquiring 진입"), WideReacquisitionRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Reacquiring);
	WideReacquisitionRig.AdvanceGuidance(0.01f);
	TestEqual(TEXT("Tracking 35도 밖의 80도 Target을 Reacquisition 120도로 재포착"), WideReacquisitionRig.MissileGuideComponent->GetGuideSnapshot().SeekerState, ECFMissileSeekerState::Tracking);

	// [v1.4.1] PurePursuit exact 180도 rear target이 공통 Launch Right tie-break로 실제 선회를 시작하는지 검증할 장치입니다.
	FCFStatefulMissileTestRig ExactRearPurePursuitRig;
	if (!TestTrue(TEXT("Exact Rear PurePursuit 장치 초기화"), ExactRearPurePursuitRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.1] exact rear를 직접 향하면서 기존 선회/횡가속 물리 한계를 유지할 PurePursuit 설정입니다.
	FCFMissileGuideConfig ExactRearPurePursuitConfig = BuildStatefulGuideConfig();
	ExactRearPurePursuitConfig.GuidanceLaw = ECFMissileGuidanceLaw::PurePursuit;
	ExactRearPurePursuitConfig.AcquisitionConeHalfAngleDeg = 180.0f;
	ExactRearPurePursuitConfig.TrackingConeHalfAngleDeg = 180.0f;
	ExactRearPurePursuitConfig.MaximumTurnRateDegPerSec = 45.0f;
	ExactRearPurePursuitConfig.MaximumLateralAccelerationCmPerSecSq = 5000.0f;
	ExactRearPurePursuitConfig.GuidanceResponseTimeSeconds = 0.01f;
	ExactRearPurePursuitRig.StartGuidance(ExactRearPurePursuitConfig, FVector(-10000.0f, 0.0f, 0.0f));
	ExactRearPurePursuitRig.AdvanceGuidance(0.01f);

	// [v1.4.1] PurePursuit exact rear 첫 Guidance tick의 실제 Runtime Snapshot입니다.
	const FCFMissileGuideSnapshot ExactRearPurePursuitSnapshot = ExactRearPurePursuitRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.4.1] PurePursuit가 공통 helper로 만든 deterministic Launch Right bounded command입니다.
	const FCFMissileGuidanceCommand ExactRearPurePursuitCommand = ExactRearPurePursuitRig.MissileGuideComponent->GetGuidanceCommand();
	TestEqual(TEXT("Exact Rear PurePursuit Target 획득 후 Tracking"), ExactRearPurePursuitSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestFalse(TEXT("Exact Rear PurePursuit는 PN Course Capture가 아님"), ExactRearPurePursuitSnapshot.bCourseCaptureActive);
	TestTrue(TEXT("Exact Rear PurePursuit Command 유효"), ExactRearPurePursuitCommand.bCommandValid);
	TestTrue(TEXT("Exact Rear PurePursuit Launch Right +Y 선회"), ExactRearPurePursuitCommand.AppliedLateralAccelerationCmPerSecSq.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Exact Rear PurePursuit Seeker 각도 180도 유지"), FMath::IsNearlyEqual(ExactRearPurePursuitCommand.SeekerAngleDeg, 180.0f, 0.01f));
	TestTrue(TEXT("Exact Rear PurePursuit tie-break는 180도 방향오차 크기를 사실상 유지"), ExactRearPurePursuitCommand.RequestedTurnRateDegPerSec > 17000.0f);
	TestTrue(TEXT("Exact Rear PurePursuit 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(ExactRearPurePursuitCommand, ExactRearPurePursuitConfig));
	TestTrue(TEXT("Exact Rear PurePursuit Debug AimPoint는 실제 rear Target 유지"), ExactRearPurePursuitSnapshot.GuidanceAimPoint.Equals(FVector(-10000.0f, 0.0f, 0.0f), 0.01f));

	// [v1.4.1] LeadPursuit exact rear에서 Target velocity가 0일 때 동일 공통 tie-break를 사용하는지 검증할 장치입니다.
	FCFStatefulMissileTestRig ExactRearLeadPursuitRig;
	if (!TestTrue(TEXT("Exact Rear LeadPursuit 장치 초기화"), ExactRearLeadPursuitRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.1] 정지 Target의 bounded lead가 0이어서 실제 Guidance AimPoint가 exact rear에 남는 LeadPursuit 설정입니다.
	FCFMissileGuideConfig ExactRearLeadPursuitConfig = BuildStatefulGuideConfig();
	ExactRearLeadPursuitConfig.GuidanceLaw = ECFMissileGuidanceLaw::LeadPursuit;
	ExactRearLeadPursuitConfig.LeadTimeSeconds = 0.20f;
	ExactRearLeadPursuitConfig.MaxLeadDistanceCm = 2000.0f;
	ExactRearLeadPursuitConfig.AcquisitionConeHalfAngleDeg = 180.0f;
	ExactRearLeadPursuitConfig.TrackingConeHalfAngleDeg = 180.0f;
	ExactRearLeadPursuitConfig.MaximumTurnRateDegPerSec = 45.0f;
	ExactRearLeadPursuitConfig.MaximumLateralAccelerationCmPerSecSq = 5000.0f;
	ExactRearLeadPursuitConfig.GuidanceResponseTimeSeconds = 0.01f;
	ExactRearLeadPursuitRig.StartGuidance(ExactRearLeadPursuitConfig, FVector(-10000.0f, 0.0f, 0.0f));
	ExactRearLeadPursuitRig.AdvanceGuidance(0.01f);

	// [v1.4.1] LeadPursuit exact rear 첫 Guidance tick의 실제 Runtime Snapshot입니다.
	const FCFMissileGuideSnapshot ExactRearLeadPursuitSnapshot = ExactRearLeadPursuitRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.4.1] LeadPursuit가 공통 helper로 만든 deterministic Launch Right bounded command입니다.
	const FCFMissileGuidanceCommand ExactRearLeadPursuitCommand = ExactRearLeadPursuitRig.MissileGuideComponent->GetGuidanceCommand();
	TestEqual(TEXT("Exact Rear LeadPursuit Target 획득 후 Tracking"), ExactRearLeadPursuitSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestFalse(TEXT("Exact Rear LeadPursuit는 PN Course Capture가 아님"), ExactRearLeadPursuitSnapshot.bCourseCaptureActive);
	TestTrue(TEXT("Exact Rear LeadPursuit Command 유효"), ExactRearLeadPursuitCommand.bCommandValid);
	TestTrue(TEXT("Exact Rear LeadPursuit Launch Right +Y 선회"), ExactRearLeadPursuitCommand.AppliedLateralAccelerationCmPerSecSq.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Exact Rear LeadPursuit Seeker 각도 180도 유지"), FMath::IsNearlyEqual(ExactRearLeadPursuitCommand.SeekerAngleDeg, 180.0f, 0.01f));
	TestTrue(TEXT("Exact Rear LeadPursuit tie-break는 180도 방향오차 크기를 사실상 유지"), ExactRearLeadPursuitCommand.RequestedTurnRateDegPerSec > 17000.0f);
	TestTrue(TEXT("Exact Rear LeadPursuit 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(ExactRearLeadPursuitCommand, ExactRearLeadPursuitConfig));
	TestTrue(TEXT("Exact Rear LeadPursuit Debug AimPoint는 실제 rear Target 유지"), ExactRearLeadPursuitSnapshot.GuidanceAimPoint.Equals(FVector(-10000.0f, 0.0f, 0.0f), 0.01f));

	// [v1.4.0] exact 180도 rear target이 LaunchTransform Right Vector 기준으로 결정론적 Course Capture를 시작하는지 검증할 장치입니다.
	FCFStatefulMissileTestRig ExactRearRig;
	if (!TestTrue(TEXT("Exact Rear tie-break 장치 초기화"), ExactRearRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.0] 180도 Target을 획득할 수 있으면서 선회/횡가속 물리 한계는 유한하게 유지한 PN 설정입니다.
	FCFMissileGuideConfig ExactRearConfig = BuildStatefulGuideConfig();
	ExactRearConfig.GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;
	ExactRearConfig.AcquisitionConeHalfAngleDeg = 180.0f;
	ExactRearConfig.TrackingConeHalfAngleDeg = 180.0f;
	ExactRearConfig.MaximumTurnRateDegPerSec = 45.0f;
	ExactRearConfig.MaximumLateralAccelerationCmPerSecSq = 5000.0f;
	ExactRearConfig.GuidanceResponseTimeSeconds = 0.01f;
	ExactRearRig.StartGuidance(ExactRearConfig, FVector(-10000.0f, 0.0f, 0.0f));
	ExactRearRig.AdvanceGuidance(0.01f);

	// [v1.4.0] exact rear 첫 Guidance tick의 activation-local Runtime Snapshot입니다.
	const FCFMissileGuideSnapshot ExactRearSnapshot = ExactRearRig.MissileGuideComponent->GetGuideSnapshot();

	// [v1.4.0] LaunchTransform Right Vector인 +Y 방향으로 결정론적으로 생성된 bounded Course Capture Command입니다.
	const FCFMissileGuidanceCommand ExactRearCommand = ExactRearRig.MissileGuideComponent->GetGuidanceCommand();
	TestEqual(TEXT("Exact Rear Target 획득 후 Tracking"), ExactRearSnapshot.SeekerState, ECFMissileSeekerState::Tracking);
	TestTrue(TEXT("Exact Rear PN은 Course Capture 활성"), ExactRearSnapshot.bCourseCaptureActive);
	TestFalse(TEXT("초기 Exact Rear U-turn은 Overshoot 미준비"), ExactRearSnapshot.bOvershootArmed);
	TestTrue(TEXT("Exact Rear Course Capture Command 유효"), ExactRearCommand.bCommandValid);
	TestTrue(TEXT("Exact Rear Launch Right +Y 방향으로 결정론적 선회"), ExactRearCommand.AppliedLateralAccelerationCmPerSecSq.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Exact Rear Seeker 각도 180도 유지"), FMath::IsNearlyEqual(ExactRearCommand.SeekerAngleDeg, 180.0f, 0.01f));
	TestTrue(TEXT("Exact Rear tie-break도 기존 물리 상한 준수"), IsGuidanceCommandInsidePhysicalLimits(ExactRearCommand, ExactRearConfig));

	ExactRearRig.TargetActor->SetActorLocation(FVector(-10100.0f, 0.0f, 0.0f));
	ExactRearRig.AdvanceGuidanceKeepingVelocity(0.01f);

	// [v1.4.0] rear U-turn 도중 Target 거리가 증가해도 접근 이력 전에는 Overshoot로 종결되지 않는 Snapshot입니다.
	const FCFMissileGuideSnapshot RearDistanceIncreaseSnapshot = ExactRearRig.MissileGuideComponent->GetGuideSnapshot();
	TestTrue(TEXT("접근 전 rear 거리 증가도 LostFinal 아님"), RearDistanceIncreaseSnapshot.SeekerState != ECFMissileSeekerState::LostFinal);
	TestFalse(TEXT("접근 전 rear 거리 증가에도 Overshoot 미준비"), RearDistanceIncreaseSnapshot.bOvershootArmed);
	TestTrue(TEXT("접근 전 rear 거리 증가 MissReason은 Overshoot 아님"), RearDistanceIncreaseSnapshot.MissReason != ECFMissileMissReason::Overshoot);

	// [v1.4.0] 실제 접근 성립 뒤에만 Overshoot가 arm되고 Target 통과 뒤 종결되는지 검증할 장치입니다.
	FCFStatefulMissileTestRig ArmedOvershootRig;
	if (!TestTrue(TEXT("Approach-Armed Overshoot 장치 초기화"), ArmedOvershootRig.Initialize(TestWorld)))
	{
		return false;
	}

	// [v1.4.0] 전후방 기하를 모두 관측할 수 있게 Seeker를 180도로 연 Stateful PN 설정입니다.
	FCFMissileGuideConfig ArmedOvershootConfig = BuildStatefulGuideConfig();
	ArmedOvershootConfig.GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;
	ArmedOvershootConfig.AcquisitionConeHalfAngleDeg = 180.0f;
	ArmedOvershootConfig.TrackingConeHalfAngleDeg = 180.0f;
	ArmedOvershootRig.StartGuidance(ArmedOvershootConfig, FVector(1000.0f, 0.0f, 0.0f));
	ArmedOvershootRig.AdvanceGuidance(0.01f);

	ArmedOvershootRig.MissileActor->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));
	ArmedOvershootRig.AdvanceGuidance(0.01f);
	TestTrue(TEXT("거리 감소 + 양의 접근 속도 후 Stateful Overshoot arm"), ArmedOvershootRig.MissileGuideComponent->GetGuideSnapshot().bOvershootArmed);

	ArmedOvershootRig.MissileActor->SetActorLocation(FVector(1050.0f, 0.0f, 0.0f));
	ArmedOvershootRig.AdvanceGuidance(0.01f);
	TestTrue(TEXT("Target을 막 지난 Step은 거리 감소 중이면 즉시 Overshoot 아님"), ArmedOvershootRig.MissileGuideComponent->GetGuideSnapshot().SeekerState != ECFMissileSeekerState::LostFinal);

	ArmedOvershootRig.MissileActor->SetActorLocation(FVector(1200.0f, 0.0f, 0.0f));
	ArmedOvershootRig.AdvanceGuidance(0.01f);

	// [v1.4.0] approach-armed 뒤 거리 증가와 비접근이 함께 성립해 정상 Overshoot로 종결된 Snapshot입니다.
	const FCFMissileGuideSnapshot ArmedOvershootSnapshot = ArmedOvershootRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("접근 arm 이후 거리 증가 + 비접근은 LostFinal"), ArmedOvershootSnapshot.SeekerState, ECFMissileSeekerState::LostFinal);
	TestEqual(TEXT("접근 arm 이후 최종 MissReason Overshoot"), ArmedOvershootSnapshot.MissReason, ECFMissileMissReason::Overshoot);
	TestTrue(TEXT("Overshoot 종결 시 arm 진단 유지"), ArmedOvershootSnapshot.bOvershootArmed);

	ArmedOvershootRig.MissileGuideComponent->ResetMissileGuidance();

	// [v1.4.0] Pool-style Reset이 12D activation-local 상태를 모두 제거했는지 확인할 최종 Snapshot입니다.
	const FCFMissileGuideSnapshot RearAspectResetSnapshot = ArmedOvershootRig.MissileGuideComponent->GetGuideSnapshot();
	TestEqual(TEXT("MG-P0-12D Reset 후 SeekerState Inactive"), RearAspectResetSnapshot.SeekerState, ECFMissileSeekerState::Inactive);
	TestFalse(TEXT("MG-P0-12D Reset 후 activation latch 제거"), RearAspectResetSnapshot.bGuidanceActivationSatisfied);
	TestFalse(TEXT("MG-P0-12D Reset 후 Overshoot arm 제거"), RearAspectResetSnapshot.bOvershootArmed);
	TestFalse(TEXT("MG-P0-12D Reset 후 Course Capture 비활성"), RearAspectResetSnapshot.bCourseCaptureActive);
	TestEqual(TEXT("MG-P0-12D Reset 후 Activation Mode 기본 Follow"), RearAspectResetSnapshot.GuidanceActivationMode, ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
