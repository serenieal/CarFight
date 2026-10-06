// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-09-27
// Description: CF-FQ-056 공통 발사체 비행 물리 합성 구현
// Scope: MotorStep·Guidance request·실제 ProjectileMovement 중력을 합성해 outer frame당 단일 AddForce를 queue합니다.
// Changelog:
// - v1.0.1: Rocket axial governor 활성 시 TVC가 축방향 추진 제한을 우회하지 못하도록 같은 엔진 thrust vector 전체를 0으로 제한.
// - v1.0.0: PFP-P0-02 Ballistic/Rocket/GuidedMissile, Rocket TVC, Missile shared lateral budget, axial governor와 Pool force reset 구현.
// Migration:
// - 중력은 ProjectileMovement에 계속 남으며 Dynamics는 non-gravity acceleration만 AddForce로 전달합니다.
// - MaximumPropelledSpeed는 world-speed clamp가 아니라 추진축 governor로만 사용합니다.

#include "CFProjectileDynamicsComp.h"

#include "CFMissileFlightComp.h"
#include "CFMissileGuideComp.h"
#include "CFProjectileData.h"
#include "CFProjectileMotorComp.h"

#include "GameFramework/ProjectileMovementComponent.h"

namespace
{
	// [v1.0.0] Dynamics 계산에 들어오는 FVector가 세 축 모두 유한한지 확인합니다.
	bool IsFiniteDynamicsVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}

	// [v1.0.0] 지정 기준축에 수직인 벡터 성분만 반환합니다.
	FVector ProjectOntoLateralPlane(const FVector& Value, const FVector& NormalizedAxis)
	{
		return Value - NormalizedAxis * FVector::DotProduct(Value, NormalizedAxis);
	}
}

// [v1.0.0] PrePhysics Tick과 비활성 기본 상태를 구성합니다.
UCFProjectileDynamicsComp::UCFProjectileDynamicsComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetComponentTickEnabled(false);
}

// [v1.0.0] 이번 활성화의 ProjectileData·LaunchContext와 producer/runtime 참조를 복사해 Dynamics를 시작합니다.
void UCFProjectileDynamicsComp::StartProjectileDynamics(
	const UCFProjectileData* InProjectileData,
	const FCFProjectileLaunchContext& InLaunchContext,
	UProjectileMovementComponent* InProjectileMovementComponent,
	UCFProjectileMotorComp* InProjectileMotorComponent,
	UCFMissileFlightComp* InMissileFlightComponent,
	UCFMissileGuideComp* InMissileGuideComponent)
{
	ResetProjectileDynamics();

	if (!IsValid(InProjectileData) || !IsValid(InProjectileMovementComponent))
	{
		return;
	}

	ActivePropulsionConfig = InProjectileData->PropulsionConfig;
	ActiveMissileFlightConfig = InProjectileData->GetEffectiveMissileFlightConfig();
	ActiveMissileGuideConfig = InProjectileData->GetEffectiveMissileGuideConfig();
	ActiveProjectileMovementComponent = InProjectileMovementComponent;
	ActiveProjectileMotorComponent = InProjectileMotorComponent;
	ActiveMissileFlightComponent = InMissileFlightComponent;
	ActiveMissileGuideComponent = InMissileGuideComponent;

	// [v1.0.0] Rocket 안정화와 zero-velocity fallback의 기준이 되는 발사 순간 방향입니다.
	LaunchAxis = InLaunchContext.InitialLaunchDirection.GetSafeNormal();
	if (!IsFiniteDynamicsVector(LaunchAxis) || LaunchAxis.IsNearlyZero())
	{
		LaunchAxis = InLaunchContext.LaunchTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	}
	if (!IsFiniteDynamicsVector(LaunchAxis) || LaunchAxis.IsNearlyZero())
	{
		LaunchAxis = FVector::ForwardVector;
	}

	CurrentDynamicsMode = ResolveDynamicsMode();
	CurrentDynamicsSnapshot = FCFProjectileDynamicsSnapshot();
	CurrentDynamicsSnapshot.DynamicsMode = CurrentDynamicsMode;
	SetComponentTickEnabled(true);
}

// [v1.0.0] pending force와 runtime filter/snapshot을 Pool 재사용 안전 기본값으로 초기화합니다.
void UCFProjectileDynamicsComp::ResetProjectileDynamics()
{
	SetComponentTickEnabled(false);

	if (IsValid(ActiveProjectileMovementComponent.Get()))
	{
		ActiveProjectileMovementComponent->ClearPendingForce(true);
	}

	ActivePropulsionConfig = FCFProjectilePropulsionConfig();
	ActiveMissileFlightConfig = FCFMissileFlightConfig();
	ActiveMissileGuideConfig = FCFMissileGuideConfig();
	LaunchAxis = FVector::ForwardVector;
	ActiveProjectileMovementComponent = nullptr;
	ActiveProjectileMotorComponent = nullptr;
	ActiveMissileFlightComponent = nullptr;
	ActiveMissileGuideComponent = nullptr;
	CurrentDynamicsMode = ECFProjectileDynamicsMode::Ballistic;
	FilteredGuidanceRequest = FVector::ZeroVector;
	CurrentDynamicsSnapshot = FCFProjectileDynamicsSnapshot();
}

// [v1.0.0] Automation에서 월드 프레임 진행 없이 같은 Dynamics 계산을 한 단계 실행합니다.
void UCFProjectileDynamicsComp::AdvanceDynamicsForAutomation(const float DeltaTime)
{
	AdvanceDynamicsSimulation(DeltaTime);
}

// [v1.0.0] 같은 PrePhysics frame의 producer 결과를 읽어 최종 AddForce를 queue합니다.
void UCFProjectileDynamicsComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AdvanceDynamicsSimulation(DeltaTime);
}

// [v1.0.0] Runtime Tick과 Automation이 공유하는 한 outer frame의 Dynamics 계산을 수행합니다.
void UCFProjectileDynamicsComp::AdvanceDynamicsSimulation(const float DeltaTime)
{
	UProjectileMovementComponent* ProjectileMovementComponent = ActiveProjectileMovementComponent.Get();
	if (!IsValid(ProjectileMovementComponent))
	{
		SetComponentTickEnabled(false);
		return;
	}

	// [v1.0.0] NaN과 음수를 제거한 이번 outer frame 제어 시간입니다.
	const float SafeDeltaTime = FMath::IsFinite(DeltaTime)
		? FMath::Max(DeltaTime, 0.0f)
		: 0.0f;
	if (SafeDeltaTime <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// [v1.0.0] Motor가 같은 outer frame에서 생산한 점화·연소 impulse 정보입니다.
	const FCFProjectileMotorStep MotorStep = IsValid(ActiveProjectileMotorComponent.Get())
		? ActiveProjectileMotorComponent->ConsumeLatestMotorStep()
		: FCFProjectileMotorStep();

	// [v1.0.0] 현재 ProjectileMovement가 보유한 실제 월드 Velocity입니다.
	const FVector CurrentVelocity = IsFiniteDynamicsVector(ProjectileMovementComponent->Velocity)
		? ProjectileMovementComponent->Velocity
		: FVector::ZeroVector;

	// [v1.0.0] 중력 배율까지 적용되어 ProjectileMovement가 직접 적분할 실제 중력입니다.
	const FVector WorldGravityAcceleration = ResolveWorldGravityAcceleration();

	CurrentDynamicsSnapshot = FCFProjectileDynamicsSnapshot();
	CurrentDynamicsSnapshot.DynamicsMode = CurrentDynamicsMode;
	CurrentDynamicsSnapshot.WorldGravityAcceleration = WorldGravityAcceleration;
	CurrentDynamicsSnapshot.AppliedBurnFraction = FMath::Clamp(MotorStep.AppliedBurnFraction, 0.0f, 1.0f);

	// [v1.0.0] 이번 frame에 ProjectileMovement로 전달할 추진+비행제어 최종 가속도입니다.
	FVector QueuedNonGravityAcceleration = FVector::ZeroVector;

	if (CurrentDynamicsMode == ECFProjectileDynamicsMode::Rocket)
	{
		QueuedNonGravityAcceleration = ResolveRocketStabilizedThrust(
			MotorStep,
			CurrentVelocity,
			WorldGravityAcceleration);
	}
	else if (CurrentDynamicsMode == ECFProjectileDynamicsMode::GuidedMissile)
	{
		// [v1.0.0] 현재 Velocity가 유효하지 않을 때만 LaunchAxis로 대체할 미사일 비행 접선입니다.
		FVector FlightTangent = CurrentVelocity.GetSafeNormal();
		if (!IsFiniteDynamicsVector(FlightTangent) || FlightTangent.IsNearlyZero())
		{
			FlightTangent = LaunchAxis;
		}

		// [v1.0.0] Burning impulse와 axial governor를 반영한 미사일 축방향 추진 가속도입니다.
		bool bAxialGovernorActive = false;
		const float AppliedAxialAcceleration = ResolveAxialPropulsionAcceleration(
			MotorStep,
			FlightTangent,
			bAxialGovernorActive);

		CurrentDynamicsSnapshot.bAxialGovernorActive = bAxialGovernorActive;
		CurrentDynamicsSnapshot.AppliedAxialAcceleration = AppliedAxialAcceleration;

		// [v1.0.0] Guidance와 횡중력을 같은 budget에서 제한한 미사일 횡제어 가속도입니다.
		const FVector AppliedLateralAcceleration = ResolveMissileLateralControl(
			SafeDeltaTime,
			CurrentVelocity,
			FlightTangent,
			WorldGravityAcceleration);

		QueuedNonGravityAcceleration =
			FlightTangent * AppliedAxialAcceleration
			+ AppliedLateralAcceleration;
	}
	else
	{
		FilteredGuidanceRequest = FVector::ZeroVector;
	}

	if (!IsFiniteDynamicsVector(QueuedNonGravityAcceleration))
	{
		QueuedNonGravityAcceleration = FVector::ZeroVector;
	}

	QueueNonGravityAcceleration(QueuedNonGravityAcceleration);
}

// [v1.0.0] MissileFlight 우선, Propulsion 차순으로 Ballistic/Rocket/GuidedMissile 모드를 결정합니다.
ECFProjectileDynamicsMode UCFProjectileDynamicsComp::ResolveDynamicsMode() const
{
	if (ActiveMissileFlightConfig.bUseMissileFlight)
	{
		return ECFProjectileDynamicsMode::GuidedMissile;
	}

	if (ActivePropulsionConfig.bUsePropulsion)
	{
		return ECFProjectileDynamicsMode::Rocket;
	}

	return ECFProjectileDynamicsMode::Ballistic;
}

// [v1.0.0] ProjectileMovement가 실제 적분할 GravityScale 적용 월드 중력을 반환합니다.
FVector UCFProjectileDynamicsComp::ResolveWorldGravityAcceleration() const
{
	const UProjectileMovementComponent* ProjectileMovementComponent = ActiveProjectileMovementComponent.Get();
	if (!IsValid(ProjectileMovementComponent))
	{
		return FVector::ZeroVector;
	}

	// [v1.0.0] ProjectileMovement가 실제 사용할 GravityScale 적용 Z축 중력입니다.
	const float GravityZ = ProjectileMovementComponent->GetGravityZ();
	return FMath::IsFinite(GravityZ)
		? FVector(0.0f, 0.0f, GravityZ)
		: FVector::ZeroVector;
}

// [v1.0.0] MotorStep의 burn fraction과 axial governor로 축방향 추진 가속도 요구/적용값을 해석합니다.
float UCFProjectileDynamicsComp::ResolveAxialPropulsionAcceleration(
	const FCFProjectileMotorStep& MotorStep,
	const FVector& AxialDirection,
	bool& bOutAxialGovernorActive)
{
	bOutAxialGovernorActive = false;

	// [v1.0.0] 이번 outer frame의 실제 연소시간 비율을 반영한 impulse-equivalent 추진 가속도입니다.
	const float RequestedAxialAcceleration = MotorStep.bHasPropulsionImpulse
		? FMath::Max(MotorStep.ThrustAccelerationCmPerSecSq, 0.0f)
			* FMath::Clamp(MotorStep.AppliedBurnFraction, 0.0f, 1.0f)
		: 0.0f;
	CurrentDynamicsSnapshot.RequestedAxialAcceleration = RequestedAxialAcceleration;

	if (RequestedAxialAcceleration <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	UProjectileMovementComponent* ProjectileMovementComponent = ActiveProjectileMovementComponent.Get();
	if (!IsValid(ProjectileMovementComponent))
	{
		return 0.0f;
	}

	// [v1.0.0] 월드 속도 전체가 아니라 현재 추진축으로 투영한 실제 축방향 속도입니다.
	const float CurrentAxialSpeedCmPerSec = FVector::DotProduct(
		ProjectileMovementComponent->Velocity,
		AxialDirection);

	// [v1.0.0] MotorStep에 보존된 추진축 governor 상한입니다.
	const float MaximumPropelledSpeedCmPerSec = FMath::Max(
		MotorStep.MaximumPropelledSpeedCmPerSec,
		1.0f);

	bOutAxialGovernorActive =
		FMath::IsFinite(CurrentAxialSpeedCmPerSec)
		&& CurrentAxialSpeedCmPerSec + KINDA_SMALL_NUMBER >= MaximumPropelledSpeedCmPerSec;

	return bOutAxialGovernorActive ? 0.0f : RequestedAxialAcceleration;
}

// [v1.0.0] 기존 총 엔진 추력 안에서 Launch-Axis TVC 안정화와 axial governor를 함께 계산합니다.
FVector UCFProjectileDynamicsComp::ResolveRocketStabilizedThrust(
	const FCFProjectileMotorStep& MotorStep,
	const FVector& CurrentVelocity,
	const FVector& WorldGravityAcceleration)
{
	// [v1.0.0] Rocket 순수 axial 추진에 MaximumPropelledSpeed governor를 적용한 기본 가속도입니다.
	bool bAxialGovernorActive = false;
	const float BaseAxialAcceleration = ResolveAxialPropulsionAcceleration(
		MotorStep,
		LaunchAxis,
		bAxialGovernorActive);
	CurrentDynamicsSnapshot.bAxialGovernorActive = bAxialGovernorActive;
	CurrentDynamicsSnapshot.AppliedAxialAcceleration = BaseAxialAcceleration;

	// [v1.0.0] MotorStep burn fraction을 반영한 현재 frame의 총 엔진 가속도 용량입니다.
	const float TotalEngineAcceleration = CurrentDynamicsSnapshot.RequestedAxialAcceleration;
	if (TotalEngineAcceleration <= KINDA_SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}

	if (!ActivePropulsionConfig.bUseLaunchAxisStabilization)
	{
		return LaunchAxis * BaseAxialAcceleration;
	}

	// [v1.0.0] 저장값을 P0 Freeze 범위로 제한한 최대 TVC 각도입니다.
	const float MaximumThrustVectorAngleDeg = FMath::Clamp(
		ActivePropulsionConfig.MaximumThrustVectorAngleDeg,
		0.0f,
		45.0f);

	// [v1.0.0] Launch Axis 횡속도 오차를 줄일 안전한 응답 시간입니다.
	const float StabilizationResponseTimeSeconds = FMath::Clamp(
		ActivePropulsionConfig.LaunchAxisStabilizationResponseTimeSeconds,
		0.01f,
		10.0f);

	if (MaximumThrustVectorAngleDeg <= KINDA_SMALL_NUMBER)
	{
		return LaunchAxis * BaseAxialAcceleration;
	}

	// [v1.0.0] 현재 속도 중 Launch Axis에 수직이라 안정화가 회복해야 할 성분입니다.
	const FVector OffAxisVelocity = ProjectOntoLateralPlane(CurrentVelocity, LaunchAxis);

	// [v1.0.0] 실제 중력 중 Launch Axis에 수직이라 TVC가 같은 총 추력 안에서 대응할 성분입니다.
	const FVector GravityLateral = ProjectOntoLateralPlane(WorldGravityAcceleration, LaunchAxis);

	// [v1.0.0] 설정 응답시간 안에 현재 횡속도 오차를 줄이기 위한 복원 가속도 요구입니다.
	const FVector VelocityRecoveryRequest = -OffAxisVelocity / StabilizationResponseTimeSeconds;

	// [v1.0.0] 횡속도 복원과 실제 횡중력 대응을 합친 Rocket TVC 요구입니다.
	const FVector RocketLateralRequest = VelocityRecoveryRequest - GravityLateral;
	CurrentDynamicsSnapshot.RocketStabilizationRequest = RocketLateralRequest;

	// [v1.0.1] 총 엔진 가속도와 최대 TVC 각도가 허용하는 횡가속도 상한입니다.
	const float MaximumTvcLateralAcceleration =
		TotalEngineAcceleration
		* FMath::Sin(FMath::DegreesToRadians(MaximumThrustVectorAngleDeg));

	// [v1.0.1] MaximumPropelledSpeed governor는 axial thrust를 완전히 중단합니다.
	// Rocket TVC는 같은 엔진 thrust vector를 기울이는 방식이므로 axial 0 상태에서 별도 lateral force를 만들어 governor를 우회하지 않습니다.
	if (bAxialGovernorActive)
	{
		CurrentDynamicsSnapshot.AppliedAxialAcceleration = 0.0f;
		CurrentDynamicsSnapshot.AppliedLateralControlAcceleration = FVector::ZeroVector;
		CurrentDynamicsSnapshot.bRocketStabilizationActive = false;
		CurrentDynamicsSnapshot.bLateralControlSaturated =
			RocketLateralRequest.Size() > KINDA_SMALL_NUMBER;
		return FVector::ZeroVector;
	}

	// [v1.0.1] 기존 총 추력 밖으로 나가지 않도록 각도 한계로 제한한 실제 Rocket 횡가속도입니다.
	const FVector AppliedRocketLateralAcceleration = RocketLateralRequest.GetClampedToMaxSize(
		FMath::Max(MaximumTvcLateralAcceleration, 0.0f));

	CurrentDynamicsSnapshot.AppliedLateralControlAcceleration = AppliedRocketLateralAcceleration;
	CurrentDynamicsSnapshot.bRocketStabilizationActive =
		!AppliedRocketLateralAcceleration.IsNearlyZero();
	CurrentDynamicsSnapshot.bLateralControlSaturated =
		RocketLateralRequest.Size()
		> MaximumTvcLateralAcceleration + KINDA_SMALL_NUMBER;

	// [v1.0.1] 적용 횡가속도 뒤에도 총 엔진 가속도 크기를 정확히 보존하는 axial 성분입니다.
	const float AppliedAxialAcceleration = FMath::Sqrt(FMath::Max(
		FMath::Square(TotalEngineAcceleration)
		- AppliedRocketLateralAcceleration.SizeSquared(),
		0.0f));

	CurrentDynamicsSnapshot.AppliedAxialAcceleration = AppliedAxialAcceleration;
	return LaunchAxis * AppliedAxialAcceleration + AppliedRocketLateralAcceleration;
}

// [v1.0.0] Guidance request와 횡중력 대응을 하나의 최대 횡가속도·선회율 예산에서 합성합니다.
FVector UCFProjectileDynamicsComp::ResolveMissileLateralControl(
	const float DeltaTime,
	const FVector& CurrentVelocity,
	const FVector& FlightTangent,
	const FVector& WorldGravityAcceleration)
{
	// [v1.0.0] shared lateral-control 한도 계산에 사용할 현재 실제 미사일 속력입니다.
	const float CurrentSpeedCmPerSec = CurrentVelocity.Size();

	// [v1.0.0] Flight 활성과 최소 유도 속도를 모두 만족해 aerodynamic lateral authority를 사용할 수 있는지 여부입니다.
	const bool bHasLateralControlAuthority =
		ActiveMissileFlightConfig.bUseMissileFlight
		&& FMath::IsFinite(CurrentSpeedCmPerSec)
		&& CurrentSpeedCmPerSec + KINDA_SMALL_NUMBER
			>= ActiveMissileGuideConfig.GetEffectiveMinimumGuidanceSpeedCmPerSec();

	if (!bHasLateralControlAuthority)
	{
		FilteredGuidanceRequest = FVector::ZeroVector;
		return FVector::ZeroVector;
	}

	// [v1.0.0] GuideComp가 현재 Target/Activation 상태에서 생성한 authoritative request입니다.
	const FCFMissileGuidanceCommand GuidanceCommand = IsValid(ActiveMissileGuideComponent.Get())
		? ActiveMissileGuideComponent->GetGuidanceCommand()
		: FCFMissileGuidanceCommand();

	// [v1.0.0] invalid/closed Guidance에서는 0으로 두고 gravity-lateral stabilization만 남길 raw Guidance request입니다.
	const FVector RawGuidanceRequest = GuidanceCommand.bCommandValid
		&& IsFiniteDynamicsVector(GuidanceCommand.RequestedLateralAccelerationCmPerSecSq)
		? GuidanceCommand.RequestedLateralAccelerationCmPerSecSq
		: FVector::ZeroVector;

	// [v1.0.0] hidden axial energy가 생기지 않도록 현재 FlightTangent 수직면으로 다시 투영한 Guidance request입니다.
	const FVector GuidanceLateral = ProjectOntoLateralPlane(
		RawGuidanceRequest,
		FlightTangent);

	// [v1.0.0] 기존 GuidanceResponseTimeSeconds를 실제 PFP handoff에 적용할 frame 응답 비율입니다.
	const float GuidanceResponseAlpha = FMath::Clamp(
		DeltaTime / ActiveMissileGuideConfig.GetEffectiveGuidanceResponseTimeSeconds(),
		0.0f,
		1.0f);
	FilteredGuidanceRequest = FMath::Lerp(
		FilteredGuidanceRequest,
		GuidanceLateral,
		GuidanceResponseAlpha);

	// [v1.0.0] 현재 비행 접선에 수직인 실제 중력 성분입니다.
	const FVector GravityLateral = ProjectOntoLateralPlane(
		WorldGravityAcceleration,
		FlightTangent);

	// [v1.0.0] 같은 lateral budget 안에서 실제 횡중력을 대응할 반대 방향 요구입니다.
	const FVector GravityLateralCompensationRequest = -GravityLateral;

	// [v1.0.0] Guidance와 횡중력 대응을 합친 포화 전 single shared-control 요구입니다.
	const FVector CombinedLateralControlRequest =
		FilteredGuidanceRequest
		+ GravityLateralCompensationRequest;

	CurrentDynamicsSnapshot.GuidanceLateralRequest = FilteredGuidanceRequest;
	CurrentDynamicsSnapshot.GravityLateralCompensationRequest = GravityLateralCompensationRequest;
	CurrentDynamicsSnapshot.CombinedLateralControlRequest = CombinedLateralControlRequest;

	// [v1.0.0] 현재 속도와 최대 선회율이 허용하는 횡가속도 상한입니다.
	const float MaximumAccelerationByTurnRate =
		CurrentSpeedCmPerSec
		* FMath::DegreesToRadians(
			ActiveMissileGuideConfig.GetEffectiveMaximumTurnRateDegPerSec());

	// [v1.0.0] 최대 횡가속도와 최대 선회율 중 더 엄격한 shared lateral-control 상한입니다.
	const float EffectiveMaximumLateralAcceleration = FMath::Max(
		FMath::Min(
			ActiveMissileGuideConfig.GetEffectiveMaximumLateralAccelerationCmPerSecSq(),
			MaximumAccelerationByTurnRate),
		0.0f);

	// [v1.0.0] Guidance와 gravity 대응을 한 번만 같은 budget으로 제한한 실제 횡가속도입니다.
	const FVector AppliedLateralControlAcceleration =
		CombinedLateralControlRequest.GetClampedToMaxSize(
			EffectiveMaximumLateralAcceleration);

	CurrentDynamicsSnapshot.AppliedLateralControlAcceleration =
		AppliedLateralControlAcceleration;
	CurrentDynamicsSnapshot.bLateralControlSaturated =
		CombinedLateralControlRequest.Size()
		> EffectiveMaximumLateralAcceleration + KINDA_SMALL_NUMBER;

	return AppliedLateralControlAcceleration;
}

// [v1.0.0] 최종 비중력 가속도를 ProjectileMovement AddForce에 정확히 한 번 전달합니다.
void UCFProjectileDynamicsComp::QueueNonGravityAcceleration(
	const FVector& NonGravityAcceleration)
{
	UProjectileMovementComponent* ProjectileMovementComponent =
		ActiveProjectileMovementComponent.Get();
	if (!IsValid(ProjectileMovementComponent))
	{
		return;
	}

	// [v1.0.0] NaN을 제거해 ProjectileMovement pending force에 전달할 최종 가속도입니다.
	const FVector SafeNonGravityAcceleration =
		IsFiniteDynamicsVector(NonGravityAcceleration)
		? NonGravityAcceleration
		: FVector::ZeroVector;

	ProjectileMovementComponent->AddForce(SafeNonGravityAcceleration);
	CurrentDynamicsSnapshot.QueuedNonGravityAcceleration =
		SafeNonGravityAcceleration;
}
