// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-09-27
// Description: CF-FQ-056 Projectile Flight Physics gravity-on focused Automation
// Scope: 실제 UProjectileMovementComponent의 중력·AddForce·Sub-step 적분과 Rocket TVC, axial governor, Missile shared lateral control, Pool reset 계약을 검증합니다.
// Changelog:
// - v1.0.1: Rocket MaximumPropelledSpeed axial governor가 TVC로 우회되지 않는 focused test 추가.
// - v1.0.0: PFP-P0-02 Cannon/Rocket/Missile gravity-on, saturation, ignition/burnout, frame/sub-step, Pool residue focused tests 최초 추가.
// Migration:
// - 저장 Product ProjectileData를 변경하지 않고 transient 데이터만 사용합니다.
// - DA_Missile_DirectTest는 이 파일에서 사용하거나 변경하지 않습니다.
// - 별도 custom integrator를 만들지 않고 Motor/Flight→Guide→Dynamics→ProjectileMovement 순서로 실제 UE ProjectileMovement를 직접 진행합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFMissileFlightComp.h"
#include "CFMissileGuideComp.h"
#include "CFMissileTestTarget.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectileDynamicsComp.h"
#include "CFProjectileMotorComp.h"

#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	/**
	 * PFP focused test에서 한 실제 ProjectileActor의 producer→Dynamics→ProjectileMovement 체인을 보존하는 transient rig입니다.
	 */
	struct FCFProjectileDynamicsTestRig
	{
		// [v1.0.0] transient Actor가 존재할 Automation World입니다.
		UWorld* TestWorld = nullptr;

		// [v1.0.0] 실제 production ACFProjectileActor입니다.
		ACFProjectileActor* ProjectileActor = nullptr;

		// [v1.0.0] 저장 에셋을 건드리지 않고 시나리오별 물리를 구성할 transient ProjectileData입니다.
		UCFProjectileData* ProjectileData = nullptr;

		// [v1.0.0] ignition/burn state와 MotorStep을 생산하는 컴포넌트입니다.
		UCFProjectileMotorComp* ProjectileMotorComponent = nullptr;

		// [v1.0.0] missile flight state/window를 생산하는 컴포넌트입니다.
		UCFMissileFlightComp* MissileFlightComponent = nullptr;

		// [v1.0.0] seeker/guidance request를 생산하는 컴포넌트입니다.
		UCFMissileGuideComp* MissileGuideComponent = nullptr;

		// [v1.0.0] 비중력 가속도를 단일 합성하고 AddForce를 queue하는 컴포넌트입니다.
		UCFProjectileDynamicsComp* ProjectileDynamicsComponent = nullptr;

		// [v1.0.0] 실제 중력·Velocity 적분·Sub-step을 수행하는 UE ProjectileMovement입니다.
		UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

		// [v1.0.0] 이번 활성화의 시작 위치입니다.
		FVector ActivationStartLocation = FVector::ZeroVector;

		// [v1.0.0] 지정 World/위치에 production ProjectileActor와 transient data를 준비합니다.
		bool Initialize(UWorld* InTestWorld, const FVector& SpawnLocation)
		{
			TestWorld = InTestWorld;
			if (!IsValid(TestWorld))
			{
				return false;
			}

			// [v1.0.0] empty-map 충돌 간섭을 피하도록 지정 고도/위치에 생성할 Actor transform입니다.
			const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

			ProjectileActor = TestWorld->SpawnActorDeferred<ACFProjectileActor>(
				ACFProjectileActor::StaticClass(),
				SpawnTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (!IsValid(ProjectileActor))
			{
				return false;
			}

			ProjectileActor->FinishSpawning(SpawnTransform);
			ProjectileActor->SetDestroyWhenDeactivated(false);

			ProjectileMotorComponent = ProjectileActor->FindComponentByClass<UCFProjectileMotorComp>();
			MissileFlightComponent = ProjectileActor->FindComponentByClass<UCFMissileFlightComp>();
			MissileGuideComponent = ProjectileActor->FindComponentByClass<UCFMissileGuideComp>();
			ProjectileDynamicsComponent = ProjectileActor->FindComponentByClass<UCFProjectileDynamicsComp>();
			ProjectileMovementComponent = ProjectileActor->FindComponentByClass<UProjectileMovementComponent>();

			if (!IsValid(ProjectileMotorComponent)
				|| !IsValid(MissileFlightComponent)
				|| !IsValid(MissileGuideComponent)
				|| !IsValid(ProjectileDynamicsComponent)
				|| !IsValid(ProjectileMovementComponent))
			{
				return false;
			}

			ProjectileData = NewObject<UCFProjectileData>(ProjectileActor);
			if (!IsValid(ProjectileData))
			{
				return false;
			}

			// [v1.0.0] 모든 PFP focused scenario의 기본 실제 초기 속력입니다.
			ProjectileData->InitialSpeed = 2000.0f;

			// [v1.0.0] 수명 타이머가 focused simulation을 조기에 종료하지 않을 충분한 수명입니다.
			ProjectileData->LifeTimeSeconds = 20.0f;

			// [v1.0.0] PFP acceptance의 기본 조건인 실제 중력 활성입니다.
			ProjectileData->bAffectedByGravity = true;

			// [v1.0.0] 월드 중력을 1배로 적분할 기본 중력 배율입니다.
			ProjectileData->GravityScale = 1.0f;

			// [v1.0.0] production ProjectileMovement의 sweep 경로를 유지합니다.
			ProjectileData->bUseSweepCollision = true;

			// [v1.0.0] production ProjectileMovement의 sub-step 경로를 기본 활성화합니다.
			ProjectileData->bForceSubStepping = true;

			// [v1.0.0] 120Hz 이하 내부 simulation step을 사용하도록 지정합니다.
			ProjectileData->MaxSimulationTimeStep = 1.0f / 120.0f;

			// [v1.0.0] 큰 outer delta에서도 필요한 sub-step 반복을 허용합니다.
			ProjectileData->MaxSimulationIterations = 16;

			// [v1.0.0] PFP 물리 검증과 무관한 Actor 보조 sweep은 끕니다.
			ProjectileData->bUseSupplementalContinuousSweep = false;

			// [v1.0.0] 기본 시나리오는 순수 탄도이므로 추진을 끕니다.
			ProjectileData->PropulsionConfig.bUsePropulsion = false;

			// [v1.0.0] 기본 시나리오는 순수 탄도이므로 Missile Flight를 끕니다.
			ProjectileData->MissileFlightConfig.bUseMissileFlight = false;

			// [v1.0.0] 기본 시나리오는 순수 탄도이므로 Guidance를 끕니다.
			ProjectileData->MissileGuideConfig.bUseGuidance = false;

			return true;
		}

		// [v1.0.0] 지정 초기 Velocity와 선택 Target으로 LaunchContext 기반 실제 Projectile 활성화를 수행합니다.
		void Activate(const FVector& InitialVelocity, AActor* GuidanceTargetActor = nullptr)
		{
			if (!IsValid(ProjectileActor) || !IsValid(ProjectileData))
			{
				return;
			}

			// [v1.0.0] 유효한 Launch Axis를 만들기 위한 초기 속도 정규화 방향입니다.
			FVector InitialLaunchDirection = InitialVelocity.GetSafeNormal();
			if (InitialLaunchDirection.ContainsNaN() || InitialLaunchDirection.IsNearlyZero())
			{
				InitialLaunchDirection = FVector::ForwardVector;
			}

			ActivationStartLocation = ProjectileActor->GetActorLocation();

			// [v1.0.0] Runtime과 동일하게 InitialLaunchDirection/Velocity/Target snapshot을 전달할 Context입니다.
			FCFProjectileLaunchContext LaunchContext;
			LaunchContext.LaunchTransform = FTransform(
				InitialLaunchDirection.Rotation(),
				ActivationStartLocation);
			LaunchContext.InitialLaunchDirection = InitialLaunchDirection;
			LaunchContext.InitialLaunchVelocity = InitialVelocity;
			LaunchContext.CommandTargetLocation = IsValid(GuidanceTargetActor)
				? GuidanceTargetActor->GetActorLocation()
				: ActivationStartLocation + InitialLaunchDirection * 10000.0f;
			LaunchContext.GuidanceTargetActor = GuidanceTargetActor;
			LaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;

			ProjectileActor->ActivateProjectileWithContext(
				ProjectileData,
				LaunchContext,
				nullptr);
		}

		// [v1.0.0] frozen outer-frame 계약 순서로 producer와 Dynamics를 진행한 뒤 실제 ProjectileMovement를 한 번 적분합니다.
		void AdvanceOuterFrame(const float DeltaTime)
		{
			if (!IsValid(ProjectileActor)
				|| !ProjectileActor->IsProjectileActive()
				|| !IsValid(ProjectileMotorComponent)
				|| !IsValid(MissileFlightComponent)
				|| !IsValid(MissileGuideComponent)
				|| !IsValid(ProjectileDynamicsComponent)
				|| !IsValid(ProjectileMovementComponent))
			{
				return;
			}

			ProjectileMotorComponent->AdvanceMotorForAutomation(DeltaTime);
			MissileFlightComponent->AdvanceFlightForAutomation(DeltaTime);
			MissileGuideComponent->AdvanceGuidanceForAutomation(DeltaTime);
			ProjectileDynamicsComponent->AdvanceDynamicsForAutomation(DeltaTime);

			// [v1.0.0] Dynamics가 queue한 AddForce와 실제 GravityScale을 함께 적분할 production ProjectileMovement입니다.
			ProjectileMovementComponent->TickComponent(
				DeltaTime,
				LEVELTICK_All,
				nullptr);
		}

		// [v1.0.0] 지정 총 시간만큼 같은 outer delta로 production chain을 반복 진행합니다.
		void AdvanceForDuration(const float TotalDurationSeconds, const float OuterDeltaTime)
		{
			// [v1.0.0] 음수·0 입력을 제거한 안전한 outer delta입니다.
			const float SafeOuterDeltaTime = FMath::Max(OuterDeltaTime, KINDA_SMALL_NUMBER);

			// [v1.0.0] 목표 총 시간을 넘지 않도록 계산한 정수 outer frame 수입니다.
			const int32 StepCount = FMath::Max(
				FMath::RoundToInt(TotalDurationSeconds / SafeOuterDeltaTime),
				0);

			// [v1.0.0] 지정 총 시간을 진행할 deterministic outer-frame 반복 인덱스입니다.
			for (int32 StepIndex = 0; StepIndex < StepCount; ++StepIndex)
			{
				AdvanceOuterFrame(SafeOuterDeltaTime);
			}
		}

		// [v1.0.0] 현재 Actor를 안전하게 비활성화하고 제거합니다.
		void Cleanup()
		{
			if (IsValid(ProjectileActor))
			{
				if (ProjectileActor->IsProjectileActive())
				{
					ProjectileActor->DeactivateProjectile();
				}

				ProjectileActor->Destroy();
			}
		}
	};

	// [v1.0.0] GuidedMissile gravity-control 단독 검증에 사용할 최소 Flight/Guide 성능을 구성합니다.
	void ConfigureGravityControlledMissile(UCFProjectileData* ProjectileData)
	{
		if (!IsValid(ProjectileData))
		{
			return;
		}

		ProjectileData->MissileFlightConfig.bUseMissileFlight = true;
		ProjectileData->MissileFlightConfig.AttackProfile = ECFMissileAttackProfile::Direct;
		ProjectileData->MissileFlightConfig.MinimumClearanceTimeSeconds = 0.0f;
		ProjectileData->MissileFlightConfig.MinimumClearanceDistanceCm = 0.0f;
		ProjectileData->MissileGuideConfig.bUseGuidance = false;
		ProjectileData->MissileGuideConfig.MaximumTurnRateDegPerSec = 180.0f;
		ProjectileData->MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 5000.0f;
		ProjectileData->MissileGuideConfig.GuidanceResponseTimeSeconds = 0.01f;
		ProjectileData->MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = 100.0f;
	}

	// [v1.0.0] 실제 TargetActor Guidance와 즉시 Direct guidance window를 사용하는 최소 미사일 설정을 구성합니다.
	void ConfigureTargetGuidedMissile(UCFProjectileData* ProjectileData)
	{
		if (!IsValid(ProjectileData))
		{
			return;
		}

		ConfigureGravityControlledMissile(ProjectileData);
		ProjectileData->MissileGuideConfig.bUseGuidance = true;
		ProjectileData->MissileGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
		ProjectileData->MissileGuideConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;
		ProjectileData->MissileGuideConfig.MaximumTurnRateDegPerSec = 90.0f;
		ProjectileData->MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 4000.0f;
		ProjectileData->MissileGuideConfig.GuidanceResponseTimeSeconds = 0.01f;
		ProjectileData->MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = 100.0f;
		ProjectileData->MissileGuideConfig.SeekerFieldOfViewDeg = 170.0f;
		ProjectileData->MissileGuideConfig.LockBreakAngleDeg = 120.0f;
		ProjectileData->MissileGuideConfig.TargetLostGraceTimeSeconds = 0.10f;
	}

	// [v1.0.0] gravity-on Rocket TVC focused 검증에 사용할 공통 추진 설정을 구성합니다.
	void ConfigureStabilizedRocket(
		UCFProjectileData* ProjectileData,
		const float ThrustAccelerationCmPerSecSq,
		const float MaximumThrustVectorAngleDeg)
	{
		if (!IsValid(ProjectileData))
		{
			return;
		}

		ProjectileData->PropulsionConfig.bUsePropulsion = true;
		ProjectileData->PropulsionConfig.IgnitionDelaySeconds = 0.0f;
		ProjectileData->PropulsionConfig.BurnDurationSeconds = 2.0f;
		ProjectileData->PropulsionConfig.ThrustAccelerationCmPerSecSq = ThrustAccelerationCmPerSecSq;
		ProjectileData->PropulsionConfig.MaximumPropelledSpeed = 10000.0f;
		ProjectileData->PropulsionConfig.bUseLaunchAxisStabilization = true;
		ProjectileData->PropulsionConfig.MaximumThrustVectorAngleDeg = MaximumThrustVectorAngleDeg;
		ProjectileData->PropulsionConfig.LaunchAxisStabilizationResponseTimeSeconds = 0.25f;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpCannonGravityOnBallisticTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.CannonGravityOnBallistic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Cannon이 non-gravity force 없이 실제 ProjectileMovement 중력으로 하강하는지 검증합니다.
bool FCFPfpCannonGravityOnBallisticTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] gravity-on Cannon을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Cannon gravity-on 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 실제 Cannon 비행 체인을 구성할 transient rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Cannon PFP rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Cannon은 중력 영향을 받는 ProjectileData로 활성화"), Rig.ProjectileData->bAffectedByGravity);
	TestTrue(TEXT("Cannon GravityScale은 0보다 큼"), Rig.ProjectileData->GravityScale > 0.0f);

	// [v1.0.0] 중력 적분 전 Cannon 시작 고도입니다.
	const float StartHeight = Rig.ProjectileActor->GetActorLocation().Z;

	Rig.AdvanceOuterFrame(0.10f);

	// [v1.0.0] Ballistic frame에서 Dynamics가 기록한 최종 물리 스냅샷입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestEqual(TEXT("Cannon Dynamics mode는 Ballistic"), Snapshot.DynamicsMode, ECFProjectileDynamicsMode::Ballistic);
	TestTrue(TEXT("실제 월드 중력 Z는 음수"), Snapshot.WorldGravityAcceleration.Z < -KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Ballistic Cannon은 non-gravity AddForce를 queue하지 않음"), Snapshot.QueuedNonGravityAcceleration.IsNearlyZero());
	TestTrue(TEXT("실제 ProjectileMovement 적분 뒤 Cannon Z Velocity가 하강 방향"), Rig.ProjectileMovementComponent->Velocity.Z < -KINDA_SMALL_NUMBER);
	TestTrue(TEXT("실제 ProjectileMovement 적분 뒤 Cannon 고도가 감소"), Rig.ProjectileActor->GetActorLocation().Z < StartHeight);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpRocketGravityOnStabilizationTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.RocketGravityOnStabilization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] gravity-on Rocket이 중력을 끄지 않고 기존 총 추력 안의 TVC로 Launch Axis를 안정화하는지 검증합니다.
bool FCFPfpRocketGravityOnStabilizationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] stabilization Rocket을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Rocket stabilization 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 충분한 TVC authority를 가진 Rocket rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Rocket stabilization rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureStabilizedRocket(Rig.ProjectileData, 6000.0f, 30.0f);
	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f));

	Rig.AdvanceForDuration(0.10f, 1.0f / 60.0f);

	// [v1.0.0] 충분한 TVC authority가 횡중력과 off-axis velocity를 대응한 최종 Rocket 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestEqual(TEXT("Rocket Dynamics mode"), Snapshot.DynamicsMode, ECFProjectileDynamicsMode::Rocket);
	TestTrue(TEXT("Rocket에서도 실제 중력은 계속 음수"), Snapshot.WorldGravityAcceleration.Z < -KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Rocket stabilization이 실제 활성"), Snapshot.bRocketStabilizationActive);
	TestTrue(TEXT("Rocket stabilization은 위쪽 TVC 가속도를 적용"), Snapshot.AppliedLateralControlAcceleration.Z > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("충분한 TVC에서는 0.1초 뒤 수직 속도 편차가 제한됨"), FMath::Abs(Rig.ProjectileMovementComponent->Velocity.Z) < 75.0f);

	// [v1.0.0] 기존 총 엔진 가속도 밖의 무료 가속도가 없는지 비교할 queue 크기 상한입니다.
	const float MaximumQueuedAcceleration =
		Rig.ProjectileData->PropulsionConfig.ThrustAccelerationCmPerSecSq + 1.0f;
	TestTrue(TEXT("Rocket queue 가속도는 기존 총 추력 크기를 넘지 않음"), Snapshot.QueuedNonGravityAcceleration.Size() <= MaximumQueuedAcceleration);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpRocketSaturationTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.RocketSaturation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 부족한 TVC 각도에서는 횡중력을 완전히 지우지 못하고 saturation과 실제 하강이 남는지 검증합니다.
bool FCFPfpRocketSaturationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 낮은 TVC authority Rocket을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Rocket saturation 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] TVC 한도를 의도적으로 작게 둔 Rocket rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Rocket saturation rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureStabilizedRocket(Rig.ProjectileData, 1000.0f, 1.0f);
	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	Rig.AdvanceOuterFrame(1.0f / 60.0f);

	// [v1.0.0] 1도 TVC 제한이 적용된 첫 Rocket frame 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	// [v1.0.0] 총 추력 1000에서 1도 TVC가 허용하는 이론상 최대 횡가속도입니다.
	const float MaximumTvcLateralAcceleration =
		1000.0f * FMath::Sin(FMath::DegreesToRadians(1.0f));

	TestTrue(TEXT("부족한 Rocket TVC는 saturation"), Snapshot.bLateralControlSaturated);
	TestTrue(TEXT("Rocket saturation에서도 횡제어는 0보다 큼"), Snapshot.AppliedLateralControlAcceleration.Size() > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Rocket TVC 횡가속도는 각도 한도를 넘지 않음"), Snapshot.AppliedLateralControlAcceleration.Size() <= MaximumTvcLateralAcceleration + 0.1f);
	TestTrue(TEXT("부족한 TVC에서는 실제 중력 하강 Velocity가 남음"), Rig.ProjectileMovementComponent->Velocity.Z < -KINDA_SMALL_NUMBER);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpRocketAxialGovernorTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.RocketAxialGovernor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.1] MaximumPropelledSpeed에 도달한 Rocket이 TVC로 axial governor를 우회하지 않고 전체 엔진 thrust를 중단하는지 검증합니다.
bool FCFPfpRocketAxialGovernorTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.1] axial governor Rocket을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Rocket axial governor 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.1] Launch Axis 속도가 MaximumPropelledSpeed에 정확히 도달한 Rocket rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Rocket axial governor rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureStabilizedRocket(Rig.ProjectileData, 6000.0f, 30.0f);
	Rig.ProjectileData->InitialSpeed = 10000.0f;
	Rig.ProjectileData->PropulsionConfig.MaximumPropelledSpeed = 10000.0f;
	Rig.Activate(FVector(10000.0f, 0.0f, 0.0f));
	Rig.AdvanceOuterFrame(1.0f / 60.0f);

	// [v1.0.1] axial governor가 활성화된 첫 Rocket frame 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestTrue(TEXT("Rocket axial governor 활성"), Snapshot.bAxialGovernorActive);
	TestTrue(TEXT("Burning 중 추진 요구 자체는 존재"), Snapshot.RequestedAxialAcceleration > KINDA_SMALL_NUMBER);
	TestEqual(TEXT("axial governor 적용 뒤 축방향 가속도 0"), Snapshot.AppliedAxialAcceleration, 0.0f);
	TestTrue(TEXT("TVC가 axial governor를 우회해 lateral thrust를 만들지 않음"), Snapshot.AppliedLateralControlAcceleration.IsNearlyZero());
	TestTrue(TEXT("axial governor에서는 non-gravity queue 전체가 0"), Snapshot.QueuedNonGravityAcceleration.IsNearlyZero());
	TestTrue(TEXT("world-speed clamp가 아니라 실제 중력 하강은 계속 발생"), Rig.ProjectileMovementComponent->Velocity.Z < -KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Launch Axis 속도는 hard clamp로 잘리지 않음"), Rig.ProjectileMovementComponent->Velocity.X >= 9999.0f);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpIgnitionDelayTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.IgnitionDelay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] IgnitionDelay 동안 TVC/추진 force가 없고 중력만 실제 적분되는지 검증합니다.
bool FCFPfpIgnitionDelayTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] ignition-delay Rocket을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("IgnitionDelay 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 0.2초 점화 지연을 가진 Rocket rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("IgnitionDelay rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureStabilizedRocket(Rig.ProjectileData, 6000.0f, 30.0f);
	Rig.ProjectileData->PropulsionConfig.IgnitionDelaySeconds = 0.20f;
	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	Rig.AdvanceOuterFrame(0.05f);

	// [v1.0.0] 점화 지연 중 Dynamics가 읽은 zero-impulse 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestEqual(TEXT("0.05초에는 아직 IgnitionDelay"), Rig.ProjectileMotorComponent->GetMotorState(), ECFProjectileMotorState::IgnitionDelay);
	TestEqual(TEXT("IgnitionDelay BurnFraction은 0"), Snapshot.AppliedBurnFraction, 0.0f);
	TestEqual(TEXT("IgnitionDelay axial acceleration은 0"), Snapshot.AppliedAxialAcceleration, 0.0f);
	TestFalse(TEXT("IgnitionDelay에서는 Rocket stabilization 비활성"), Snapshot.bRocketStabilizationActive);
	TestTrue(TEXT("IgnitionDelay non-gravity queue는 0"), Snapshot.QueuedNonGravityAcceleration.IsNearlyZero());
	TestTrue(TEXT("IgnitionDelay에도 중력은 실제 하강을 만듦"), Rig.ProjectileMovementComponent->Velocity.Z < -KINDA_SMALL_NUMBER);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpBurnedOutBallisticTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.BurnedOutBallistic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Rocket 연료가 끝난 다음 frame부터 추진/TVC가 0이고 중력 탄도 비행이 계속되는지 검증합니다.
bool FCFPfpBurnedOutBallisticTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 짧은 burn Rocket을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("BurnedOut ballistic 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 0.05초 뒤 BurnedOut으로 전환될 Rocket rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("BurnedOut rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureStabilizedRocket(Rig.ProjectileData, 6000.0f, 30.0f);
	Rig.ProjectileData->PropulsionConfig.BurnDurationSeconds = 0.05f;
	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f));

	Rig.AdvanceOuterFrame(0.10f);
	TestEqual(TEXT("첫 0.10초 frame 안에서 BurnedOut 도달"), Rig.ProjectileMotorComponent->GetMotorState(), ECFProjectileMotorState::BurnedOut);

	// [v1.0.0] BurnedOut 다음 순수 탄도 frame 전의 수직 Velocity입니다.
	const float VerticalVelocityBeforeBallisticFrame = Rig.ProjectileMovementComponent->Velocity.Z;

	Rig.AdvanceOuterFrame(0.10f);

	// [v1.0.0] BurnedOut 다음 frame의 zero-thrust Dynamics 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestEqual(TEXT("BurnedOut 다음 frame BurnFraction 0"), Snapshot.AppliedBurnFraction, 0.0f);
	TestEqual(TEXT("BurnedOut axial acceleration 0"), Snapshot.AppliedAxialAcceleration, 0.0f);
	TestFalse(TEXT("BurnedOut Rocket TVC 비활성"), Snapshot.bRocketStabilizationActive);
	TestTrue(TEXT("BurnedOut non-gravity queue 0"), Snapshot.QueuedNonGravityAcceleration.IsNearlyZero());
	TestTrue(TEXT("BurnedOut 뒤에도 중력으로 수직 Velocity가 더 감소"), Rig.ProjectileMovementComponent->Velocity.Z < VerticalVelocityBeforeBallisticFrame);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpMissileLevelClimbDescentTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.MissileLevelClimbDescent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Missile의 횡중력 대응은 접선 수직 성분만 다루고 상승/하강 GravityParallel은 제거하지 않는지 검증합니다.
bool FCFPfpMissileLevelClimbDescentTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 세 비행 방향의 독립 missile rig를 배치할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Missile level/climb/descent 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 수평 비행 Missile rig입니다.
	FCFProjectileDynamicsTestRig LevelRig;

	// [v1.0.0] 상승 비행 Missile rig입니다.
	FCFProjectileDynamicsTestRig ClimbRig;

	// [v1.0.0] 하강 비행 Missile rig입니다.
	FCFProjectileDynamicsTestRig DescentRig;

	if (!TestTrue(TEXT("Level missile rig 초기화"), LevelRig.Initialize(TestWorld, FVector(0.0f, -100000.0f, 50000.0f)))
		|| !TestTrue(TEXT("Climb missile rig 초기화"), ClimbRig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f)))
		|| !TestTrue(TEXT("Descent missile rig 초기화"), DescentRig.Initialize(TestWorld, FVector(0.0f, 100000.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureGravityControlledMissile(LevelRig.ProjectileData);
	ConfigureGravityControlledMissile(ClimbRig.ProjectileData);
	ConfigureGravityControlledMissile(DescentRig.ProjectileData);

	LevelRig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	ClimbRig.Activate(FVector(2000.0f, 0.0f, 1000.0f));
	DescentRig.Activate(FVector(2000.0f, 0.0f, -1000.0f));

	LevelRig.AdvanceOuterFrame(0.01f);
	ClimbRig.AdvanceOuterFrame(0.01f);
	DescentRig.AdvanceOuterFrame(0.01f);

	// [v1.0.0] 수평 Missile의 Dynamics 결과입니다.
	const FCFProjectileDynamicsSnapshot LevelSnapshot =
		LevelRig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	// [v1.0.0] 상승 Missile의 Dynamics 결과입니다.
	const FCFProjectileDynamicsSnapshot ClimbSnapshot =
		ClimbRig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	// [v1.0.0] 하강 Missile의 Dynamics 결과입니다.
	const FCFProjectileDynamicsSnapshot DescentSnapshot =
		DescentRig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestEqual(TEXT("Level mode는 GuidedMissile"), LevelSnapshot.DynamicsMode, ECFProjectileDynamicsMode::GuidedMissile);
	TestEqual(TEXT("Climb mode는 GuidedMissile"), ClimbSnapshot.DynamicsMode, ECFProjectileDynamicsMode::GuidedMissile);
	TestEqual(TEXT("Descent mode는 GuidedMissile"), DescentSnapshot.DynamicsMode, ECFProjectileDynamicsMode::GuidedMissile);
	TestTrue(TEXT("Level에서 횡중력 대응 요구가 위쪽"), LevelSnapshot.GravityLateralCompensationRequest.Z > KINDA_SMALL_NUMBER);

	// [v1.0.0] 각 초기 Velocity에서 비교할 정규화 비행 접선입니다.
	const FVector LevelTangent = FVector(2000.0f, 0.0f, 0.0f).GetSafeNormal();

	// [v1.0.0] 상승 초기 Velocity의 정규화 비행 접선입니다.
	const FVector ClimbTangent = FVector(2000.0f, 0.0f, 1000.0f).GetSafeNormal();

	// [v1.0.0] 하강 초기 Velocity의 정규화 비행 접선입니다.
	const FVector DescentTangent = FVector(2000.0f, 0.0f, -1000.0f).GetSafeNormal();

	TestTrue(TEXT("Level applied lateral은 FlightTangent에 수직"),
		FMath::Abs(FVector::DotProduct(LevelSnapshot.AppliedLateralControlAcceleration, LevelTangent)) < 0.1f);
	TestTrue(TEXT("Climb applied lateral은 FlightTangent에 수직"),
		FMath::Abs(FVector::DotProduct(ClimbSnapshot.AppliedLateralControlAcceleration, ClimbTangent)) < 0.1f);
	TestTrue(TEXT("Descent applied lateral은 FlightTangent에 수직"),
		FMath::Abs(FVector::DotProduct(DescentSnapshot.AppliedLateralControlAcceleration, DescentTangent)) < 0.1f);

	// [v1.0.0] 상승 방향의 실제 중력 접선 성분입니다.
	const float ClimbGravityParallel =
		FVector::DotProduct(ClimbSnapshot.WorldGravityAcceleration, ClimbTangent);

	// [v1.0.0] 하강 방향의 실제 중력 접선 성분입니다.
	const float DescentGravityParallel =
		FVector::DotProduct(DescentSnapshot.WorldGravityAcceleration, DescentTangent);

	TestTrue(TEXT("상승 시 GravityParallel은 감속 방향으로 남음"), ClimbGravityParallel < -KINDA_SMALL_NUMBER);
	TestTrue(TEXT("하강 시 GravityParallel은 진행 방향 가속으로 남음"), DescentGravityParallel > KINDA_SMALL_NUMBER);

	LevelRig.Cleanup();
	ClimbRig.Cleanup();
	DescentRig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpMissileSharedLateralSaturationTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.MissileSharedLateralSaturation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Missile 횡중력 대응도 Guidance와 같은 shared lateral budget에서 포화되는지 검증합니다.
bool FCFPfpMissileSharedLateralSaturationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 낮은 lateral authority Missile을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Missile shared saturation 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 최대 횡가속도를 의도적으로 50cm/s²로 제한할 Missile rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Missile shared saturation rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureGravityControlledMissile(Rig.ProjectileData);
	Rig.ProjectileData->MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 50.0f;
	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	Rig.AdvanceOuterFrame(0.05f);

	// [v1.0.0] 실제 중력 대응 요구가 shared 50cm/s² budget에서 잘린 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestTrue(TEXT("횡중력 요구가 shared lateral budget을 초과해 saturation"), Snapshot.bLateralControlSaturated);
	TestTrue(TEXT("적용 횡제어 가속도는 50cm/s² 이하"), Snapshot.AppliedLateralControlAcceleration.Size() <= 50.1f);
	TestTrue(TEXT("제한된 lateral authority 때문에 실제 하강 Velocity가 남음"), Rig.ProjectileMovementComponent->Velocity.Z < -KINDA_SMALL_NUMBER);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpMissileBurnedOutGuidanceTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.MissileBurnedOutGuidance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Missile Motor가 BurnedOut이어도 axial thrust 없이 bounded aerodynamic Guidance가 계속 가능한지 검증합니다.
bool FCFPfpMissileBurnedOutGuidanceTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] BurnedOut guidance Missile과 Target을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Missile BurnedOut guidance 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 측면 Guidance request를 만들 실제 테스트 Target입니다.
	ACFMissileTestTarget* TargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>();
	if (!TestNotNull(TEXT("BurnedOut guidance Target 생성"), TargetActor))
	{
		return false;
	}

	TargetActor->SetActorLocation(FVector(10000.0f, 3000.0f, 50000.0f));
	TargetActor->bBlockProjectileHits = false;
	TargetActor->RefreshTestTargetConfig();

	// [v1.0.0] 짧은 burn 뒤에도 TargetActor guidance를 계속할 Missile rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Missile BurnedOut guidance rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		TargetActor->Destroy();
		return false;
	}

	ConfigureTargetGuidedMissile(Rig.ProjectileData);
	Rig.ProjectileData->PropulsionConfig.bUsePropulsion = true;
	Rig.ProjectileData->PropulsionConfig.IgnitionDelaySeconds = 0.0f;
	Rig.ProjectileData->PropulsionConfig.BurnDurationSeconds = 0.05f;
	Rig.ProjectileData->PropulsionConfig.ThrustAccelerationCmPerSecSq = 3000.0f;
	Rig.ProjectileData->PropulsionConfig.MaximumPropelledSpeed = 10000.0f;
	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f), TargetActor);

	Rig.AdvanceOuterFrame(0.05f);
	Rig.AdvanceOuterFrame(0.05f);

	// [v1.0.0] 두 번째 frame에서 BurnedOut axial=0과 continued guidance를 함께 확인할 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestEqual(TEXT("Missile Motor는 BurnedOut"), Rig.ProjectileMotorComponent->GetMotorState(), ECFProjectileMotorState::BurnedOut);
	TestEqual(TEXT("BurnedOut 다음 frame axial acceleration 0"), Snapshot.AppliedAxialAcceleration, 0.0f);
	TestTrue(TEXT("BurnedOut이어도 측면 Target Guidance request가 존재"), Snapshot.GuidanceLateralRequest.Size() > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("BurnedOut이어도 bounded lateral control은 적용 가능"), Snapshot.AppliedLateralControlAcceleration.Size() > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("BurnedOut lateral은 설정 최대 4000cm/s² 이하"), Snapshot.AppliedLateralControlAcceleration.Size() <= 4000.1f);

	Rig.Cleanup();
	TargetActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpMinimumGuidanceSpeedTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.MinimumGuidanceSpeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 실제 속력이 MinimumGuidanceSpeed 아래면 Missile lateral authority가 0이고 중력만 남는지 검증합니다.
bool FCFPfpMinimumGuidanceSpeedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 저속 Missile을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Minimum guidance speed 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] lateral authority 최소 속도보다 느리게 발사할 Missile rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Minimum guidance speed rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureGravityControlledMissile(Rig.ProjectileData);
	Rig.ProjectileData->MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = 500.0f;
	Rig.Activate(FVector(100.0f, 0.0f, 0.0f));
	Rig.AdvanceOuterFrame(0.05f);

	// [v1.0.0] 최소 유도 속도 미만에서 Dynamics가 lateral authority를 제거한 결과입니다.
	const FCFProjectileDynamicsSnapshot Snapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestTrue(TEXT("최소 유도 속도 미만에서는 lateral control 0"), Snapshot.AppliedLateralControlAcceleration.IsNearlyZero());
	TestTrue(TEXT("추진도 없으므로 non-gravity queue 0"), Snapshot.QueuedNonGravityAcceleration.IsNearlyZero());
	TestTrue(TEXT("최소 유도 속도 미만에서도 실제 중력은 유지"), Snapshot.WorldGravityAcceleration.Z < -KINDA_SMALL_NUMBER);
	TestTrue(TEXT("최소 유도 속도 미만에서 실제 하강 Velocity 발생"), Rig.ProjectileMovementComponent->Velocity.Z < -KINDA_SMALL_NUMBER);

	Rig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpFrameDeltaSubStepConsistencyTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.FrameDeltaSubStepConsistency",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] outer frame delta와 ProjectileMovement sub-step 설정 변화가 Rocket 궤적을 허용오차 안에 유지하는지 검증합니다.
bool FCFPfpFrameDeltaSubStepConsistencyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 세 integration profile을 같은 World의 멀리 떨어진 위치에서 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Frame/sub-step consistency 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 30Hz outer frame + 120Hz Movement sub-step Rocket입니다.
	FCFProjectileDynamicsTestRig CoarseSubStepRig;

	// [v1.0.0] 60Hz outer frame + 120Hz Movement sub-step Rocket입니다.
	FCFProjectileDynamicsTestRig FineSubStepRig;

	// [v1.0.0] 30Hz outer frame + Movement sub-step 비활성 Rocket입니다.
	FCFProjectileDynamicsTestRig CoarseNoSubStepRig;

	if (!TestTrue(TEXT("Coarse sub-step rig 초기화"), CoarseSubStepRig.Initialize(TestWorld, FVector(0.0f, -100000.0f, 50000.0f)))
		|| !TestTrue(TEXT("Fine sub-step rig 초기화"), FineSubStepRig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f)))
		|| !TestTrue(TEXT("Coarse no-sub-step rig 초기화"), CoarseNoSubStepRig.Initialize(TestWorld, FVector(0.0f, 100000.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureStabilizedRocket(CoarseSubStepRig.ProjectileData, 4000.0f, 20.0f);
	ConfigureStabilizedRocket(FineSubStepRig.ProjectileData, 4000.0f, 20.0f);
	ConfigureStabilizedRocket(CoarseNoSubStepRig.ProjectileData, 4000.0f, 20.0f);
	CoarseNoSubStepRig.ProjectileData->bForceSubStepping = false;

	CoarseSubStepRig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	FineSubStepRig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	CoarseNoSubStepRig.Activate(FVector(2000.0f, 0.0f, 0.0f));

	CoarseSubStepRig.AdvanceForDuration(0.50f, 1.0f / 30.0f);
	FineSubStepRig.AdvanceForDuration(0.50f, 1.0f / 60.0f);
	CoarseNoSubStepRig.AdvanceForDuration(0.50f, 1.0f / 30.0f);

	// [v1.0.0] 30Hz+substep의 시작점 기준 최종 변위입니다.
	const FVector CoarseSubStepDisplacement =
		CoarseSubStepRig.ProjectileActor->GetActorLocation() - CoarseSubStepRig.ActivationStartLocation;

	// [v1.0.0] 60Hz+substep의 시작점 기준 최종 변위입니다.
	const FVector FineSubStepDisplacement =
		FineSubStepRig.ProjectileActor->GetActorLocation() - FineSubStepRig.ActivationStartLocation;

	// [v1.0.0] 30Hz+no-substep의 시작점 기준 최종 변위입니다.
	const FVector CoarseNoSubStepDisplacement =
		CoarseNoSubStepRig.ProjectileActor->GetActorLocation() - CoarseNoSubStepRig.ActivationStartLocation;

	// [v1.0.0] outer-frame delta 변화에 따른 최종 위치 오차입니다.
	const float FrameDeltaPositionErrorCm =
		FVector::Dist(CoarseSubStepDisplacement, FineSubStepDisplacement);

	// [v1.0.0] 같은 30Hz에서 Movement sub-step on/off에 따른 최종 위치 오차입니다.
	const float SubStepPositionErrorCm =
		FVector::Dist(CoarseSubStepDisplacement, CoarseNoSubStepDisplacement);

	// [v1.0.0] outer-frame delta 변화에 따른 최종 Velocity 오차입니다.
	const float FrameDeltaVelocityErrorCmPerSec = FVector::Dist(
		CoarseSubStepRig.ProjectileMovementComponent->Velocity,
		FineSubStepRig.ProjectileMovementComponent->Velocity);

	// [v1.0.0] 같은 30Hz에서 Movement sub-step on/off에 따른 최종 Velocity 오차입니다.
	const float SubStepVelocityErrorCmPerSec = FVector::Dist(
		CoarseSubStepRig.ProjectileMovementComponent->Velocity,
		CoarseNoSubStepRig.ProjectileMovementComponent->Velocity);

	AddInfo(FString::Printf(
		TEXT("PFP trajectory consistency: FramePos=%.3fcm SubStepPos=%.3fcm FrameVel=%.3fcm/s SubStepVel=%.3fcm/s"),
		FrameDeltaPositionErrorCm,
		SubStepPositionErrorCm,
		FrameDeltaVelocityErrorCmPerSec,
		SubStepVelocityErrorCmPerSec));

	TestTrue(TEXT("30Hz vs 60Hz outer-frame 최종 위치 오차 100cm 이하"), FrameDeltaPositionErrorCm <= 100.0f);
	TestTrue(TEXT("sub-step on/off 최종 위치 오차 100cm 이하"), SubStepPositionErrorCm <= 100.0f);
	TestTrue(TEXT("30Hz vs 60Hz outer-frame 최종 Velocity 오차 150cm/s 이하"), FrameDeltaVelocityErrorCmPerSec <= 150.0f);
	TestTrue(TEXT("sub-step on/off 최종 Velocity 오차 150cm/s 이하"), SubStepVelocityErrorCmPerSec <= 150.0f);

	CoarseSubStepRig.Cleanup();
	FineSubStepRig.Cleanup();
	CoarseNoSubStepRig.Cleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFPfpPoolResidueZeroTest,
	"CarFight.ProjectileFlightPhysics.PFP_P0_02.PoolResidueZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Pool 비활성화가 Dynamics와 ProjectileMovement pending force를 지워 다음 Ballistic 활성화에 propulsion residue를 남기지 않는지 검증합니다.
bool FCFPfpPoolResidueZeroTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 같은 ProjectileActor를 두 번 활성화할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Pool residue 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 첫 Rocket 활성화와 두 번째 Ballistic 활성화를 공유할 rig입니다.
	FCFProjectileDynamicsTestRig Rig;
	if (!TestTrue(TEXT("Pool residue rig 초기화"), Rig.Initialize(TestWorld, FVector(0.0f, 0.0f, 50000.0f))))
	{
		return false;
	}

	ConfigureStabilizedRocket(Rig.ProjectileData, 6000.0f, 30.0f);
	Rig.Activate(FVector(2000.0f, 0.0f, 0.0f));
	Rig.AdvanceOuterFrame(0.05f);

	// [v1.0.0] Pool reset 전에 non-gravity acceleration이 실제로 존재했음을 확인할 Rocket 결과입니다.
	const FCFProjectileDynamicsSnapshot RocketSnapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();
	TestTrue(TEXT("첫 Rocket 활성화는 non-gravity force를 실제 queue"), RocketSnapshot.QueuedNonGravityAcceleration.Size() > KINDA_SMALL_NUMBER);

	Rig.ProjectileActor->DeactivateProjectile();

	// [v1.0.0] Deactivation 직후 Dynamics reset 결과입니다.
	const FCFProjectileDynamicsSnapshot ResetSnapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();
	TestTrue(TEXT("Pool Deactivation 뒤 Dynamics queue residue 0"), ResetSnapshot.QueuedNonGravityAcceleration.IsNearlyZero());
	TestTrue(TEXT("Pool Deactivation 뒤 lateral residue 0"), ResetSnapshot.AppliedLateralControlAcceleration.IsNearlyZero());

	// [v1.0.0] 같은 Actor의 두 번째 활성화를 순수 gravity-on Ballistic으로 바꿉니다.
	Rig.ProjectileData->PropulsionConfig = FCFProjectilePropulsionConfig();
	Rig.ProjectileData->MissileFlightConfig = FCFMissileFlightConfig();
	Rig.ProjectileData->MissileGuideConfig = FCFMissileGuideConfig();

	// [v1.0.0] 재활성화 직전 원래 시험 고도로 되돌릴 transform입니다.
	const FTransform ReuseTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 50000.0f));
	Rig.ProjectileActor->SetActorTransform(ReuseTransform, false, nullptr, ETeleportType::TeleportPhysics);
	Rig.Activate(FVector(1000.0f, 0.0f, 0.0f));
	Rig.AdvanceOuterFrame(0.05f);

	// [v1.0.0] 두 번째 Ballistic 활성화에 이전 Rocket force가 재생되지 않았는지 확인할 결과입니다.
	const FCFProjectileDynamicsSnapshot ReuseSnapshot =
		Rig.ProjectileDynamicsComponent->GetProjectileDynamicsSnapshot();

	TestEqual(TEXT("Pool 재사용 두 번째 mode는 Ballistic"), ReuseSnapshot.DynamicsMode, ECFProjectileDynamicsMode::Ballistic);
	TestTrue(TEXT("Pool 재사용 Ballistic non-gravity queue 0"), ReuseSnapshot.QueuedNonGravityAcceleration.IsNearlyZero());
	TestTrue(TEXT("Pool 재사용 Ballistic X 속도는 이전 Rocket pending force 없이 초기값 유지"), FMath::IsNearlyEqual(Rig.ProjectileMovementComponent->Velocity.X, 1000.0f, 1.0f));
	TestTrue(TEXT("Pool 재사용 Ballistic에는 새 frame 중력 하강만 발생"), Rig.ProjectileMovementComponent->Velocity.Z < -KINDA_SMALL_NUMBER);

	Rig.Cleanup();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
