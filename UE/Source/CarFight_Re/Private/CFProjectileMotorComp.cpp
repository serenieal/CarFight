// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-27
// Description: CarFight 추진 발사체 모터 컴포넌트 구현
// Scope: 점화 지연·연소 상태를 진행하고 Dynamics가 소비할 impulse 보존형 propulsion step을 생산합니다.
// Changelog:
// - v1.2.0: PFP-P0-02 직접 Velocity write와 world-speed clamp 제거, outer-frame MotorStep producer 및 consume handoff 추가.
// - v1.1.0: 현재 ProjectileMovement Velocity 방향을 따라가는 미사일 추진 모드와 기존 StartMotor 호환 Adapter 추가.
// - v1.0.0: CF-FQ-028 비유도 로켓 P0 추진 모터 최초 구현.
// Migration:
// - bUsePropulsion=false인 기존 ProjectileData는 Disabled 상태로 유지하며 Velocity를 변경하지 않습니다.
// - 기존 StartMotor는 FixedLaunchDirection으로 동작하고 미사일만 CurrentVelocityDirection을 명시적으로 선택합니다.
// - BurnedOut 전환은 Projectile을 제거하거나 정지하지 않고 propulsion step 생산만 종료합니다.
// - 실제 추진 방향, TVC, MaximumPropelledSpeed axial governor와 AddForce 적용은 ProjectileDynamicsComp가 소유합니다.

#include "CFProjectileMotorComp.h"

#include "GameFramework/ProjectileMovementComponent.h"

// [v1.0.0] 모터 Tick 기본값과 비활성 상태를 초기화합니다.
UCFProjectileMotorComp::UCFProjectileMotorComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetComponentTickEnabled(false);
	RefreshMotorSnapshot();
}

// [v1.0.0] 추진 설정, ProjectileMovement와 발사 방향을 적용해 고정 방향 모터 상태를 시작합니다.
void UCFProjectileMotorComp::StartMotor(
	const FCFProjectilePropulsionConfig& InPropulsionConfig,
	UProjectileMovementComponent* InProjectileMovementComponent,
	const FVector& InLaunchDirection)
{
	StartMotorWithDirectionMode(
		InPropulsionConfig,
		InProjectileMovementComponent,
		InLaunchDirection,
		ECFProjectileThrustDirectionMode::FixedLaunchDirection);
}

// [v1.1.0] 추진 방향 모드를 명시해 Rocket 또는 Missile 모터 상태를 시작합니다.
void UCFProjectileMotorComp::StartMotorWithDirectionMode(
	const FCFProjectilePropulsionConfig& InPropulsionConfig,
	UProjectileMovementComponent* InProjectileMovementComponent,
	const FVector& InLaunchDirection,
	const ECFProjectileThrustDirectionMode InThrustDirectionMode)
{
	ResetMotor();

	ActivePropulsionConfig = InPropulsionConfig;
	ActiveProjectileMovementComponent = InProjectileMovementComponent;
	ThrustDirectionMode = InThrustDirectionMode;
	++MotorActivationCount;

	FixedThrustDirection = InLaunchDirection.GetSafeNormal();
	if (FixedThrustDirection.ContainsNaN() || FixedThrustDirection.IsNearlyZero())
	{
		FixedThrustDirection = FVector::ForwardVector;
	}

	if (!ActivePropulsionConfig.bUsePropulsion || !ActiveProjectileMovementComponent)
	{
		SetMotorState(ECFProjectileMotorState::Disabled);
		SetComponentTickEnabled(false);
		RefreshMotorSnapshot();
		return;
	}

	if (FMath::Max(ActivePropulsionConfig.IgnitionDelaySeconds, 0.0f) > KINDA_SMALL_NUMBER)
	{
		SetMotorState(ECFProjectileMotorState::IgnitionDelay);
		SetComponentTickEnabled(true);
	}
	else
	{
		EnterBurningState();
	}

	RefreshMotorSnapshot();
}

// [v1.0.0] 현재 추진 상태와 참조를 Pool 재사용 안전 기본값으로 초기화합니다.
void UCFProjectileMotorComp::ResetMotor()
{
	SetComponentTickEnabled(false);
	ActiveProjectileMovementComponent = nullptr;
	ActivePropulsionConfig = FCFProjectilePropulsionConfig();
	FixedThrustDirection = FVector::ForwardVector;
	ThrustDirectionMode = ECFProjectileThrustDirectionMode::FixedLaunchDirection;
	ElapsedFlightTimeSeconds = 0.0f;
	ElapsedIgnitionDelaySeconds = 0.0f;
	ElapsedBurnTimeSeconds = 0.0f;
	LatestMotorStep = FCFProjectileMotorStep();
	SetMotorState(ECFProjectileMotorState::Inactive);
	RefreshMotorSnapshot();
}

// [v1.0.0] 현재 Burning 상태에서 실제 추진 가속을 생산 중인지 반환합니다.
bool UCFProjectileMotorComp::IsProducingThrust() const
{
	return CurrentMotorState == ECFProjectileMotorState::Burning
		&& IsValid(ActiveProjectileMovementComponent.Get());
}

// [v1.2.0] 같은 outer frame의 Dynamics가 최신 propulsion step을 정확히 한 번 소비하고 pending step을 비웁니다.
FCFProjectileMotorStep UCFProjectileMotorComp::ConsumeLatestMotorStep()
{
	// [v1.2.0] Dynamics에 값 복사로 전달할 이번 frame의 생산 결과입니다.
	const FCFProjectileMotorStep ConsumedMotorStep = LatestMotorStep;
	LatestMotorStep = FCFProjectileMotorStep();
	return ConsumedMotorStep;
}

// [v1.0.0] Debug 패널과 Automation에서 사용할 모터 요약 문자열을 생성합니다.
FString UCFProjectileMotorComp::BuildMotorSummary() const
{
	// [v1.0.0] 현재 모터 상태의 표시 이름을 찾을 Enum 정보입니다.
	const UEnum* MotorStateEnum = StaticEnum<ECFProjectileMotorState>();

	// [v1.0.0] 현재 모터 상태를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString MotorStateText = MotorStateEnum
		? MotorStateEnum->GetNameStringByValue(static_cast<int64>(CurrentMotorState))
		: TEXT("Unknown");

	// [v1.1.0] 현재 추진 방향 모드를 사람이 읽을 수 있게 표시할 문자열입니다.
	const FString ThrustDirectionModeText = UEnum::GetValueAsString(ThrustDirectionMode);

	return FString::Printf(
		TEXT("ProjectileMotor: State=%s, DirectionMode=%s, Flight=%.3fs, Ignition=%.3f/%.3fs, Burn=%.3f/%.3fs, RemainingBurn=%.3fs, Speed=%.1fcm/s, ThrustAccel=%.1fcm/s2, MaxSpeed=%.1fcm/s, Direction=(%.3f, %.3f, %.3f), Producing=%s, FxActive=%s, Activations=%d"),
		*MotorStateText,
		*ThrustDirectionModeText,
		CurrentMotorSnapshot.ElapsedFlightTimeSeconds,
		ElapsedIgnitionDelaySeconds,
		FMath::Max(ActivePropulsionConfig.IgnitionDelaySeconds, 0.0f),
		CurrentMotorSnapshot.ElapsedBurnTimeSeconds,
		FMath::Max(ActivePropulsionConfig.BurnDurationSeconds, 0.0f),
		CurrentMotorSnapshot.RemainingBurnTimeSeconds,
		CurrentMotorSnapshot.CurrentSpeedCmPerSec,
		FMath::Max(ActivePropulsionConfig.ThrustAccelerationCmPerSecSq, 0.0f),
		FMath::Max(ActivePropulsionConfig.MaximumPropelledSpeed, 1.0f),
		CurrentMotorSnapshot.CurrentThrustDirection.X,
		CurrentMotorSnapshot.CurrentThrustDirection.Y,
		CurrentMotorSnapshot.CurrentThrustDirection.Z,
		CurrentMotorSnapshot.bIsProducingThrust ? TEXT("Yes") : TEXT("No"),
		CurrentMotorSnapshot.bThrusterFxShouldBeActive ? TEXT("Yes") : TEXT("No"),
		CurrentMotorSnapshot.MotorActivationCount);
}

// [v1.2.0] Automation과 결정적 진단에서 월드 프레임 진행 없이 모터 시간 구간을 한 단계 진행합니다.
void UCFProjectileMotorComp::AdvanceMotorForAutomation(const float DeltaTime)
{
	if (CurrentMotorState != ECFProjectileMotorState::IgnitionDelay
		&& CurrentMotorState != ECFProjectileMotorState::Burning)
	{
		LatestMotorStep = FCFProjectileMotorStep();

		// [v1.2.0] 비연소 상태에서도 같은 outer frame 길이를 진단할 안전한 Step Delta입니다.
		const float SafeStepDeltaSeconds = FMath::IsFinite(DeltaTime)
			? FMath::Max(DeltaTime, 0.0f)
			: 0.0f;
		LatestMotorStep.StepDeltaSeconds = SafeStepDeltaSeconds;
		LatestMotorStep.MotorStateBeforeStep = CurrentMotorState;
		LatestMotorStep.MotorStateAfterStep = CurrentMotorState;
		RefreshMotorSnapshot();
		return;
	}

	AdvanceMotorSimulation(DeltaTime);
	RefreshMotorSnapshot();
}

// [v1.2.0] 점화 지연과 연소 시간을 진행하고 이번 outer frame의 propulsion step을 생산합니다.
void UCFProjectileMotorComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentMotorState != ECFProjectileMotorState::IgnitionDelay
		&& CurrentMotorState != ECFProjectileMotorState::Burning)
	{
		SetComponentTickEnabled(false);
		RefreshMotorSnapshot();
		return;
	}

	if (!IsValid(ActiveProjectileMovementComponent.Get()))
	{
		LatestMotorStep = FCFProjectileMotorStep();

		// [v1.2.0] Movement 참조 상실 frame을 진단할 안전한 Step Delta입니다.
		const float SafeStepDeltaSeconds = FMath::IsFinite(DeltaTime)
			? FMath::Max(DeltaTime, 0.0f)
			: 0.0f;
		LatestMotorStep.StepDeltaSeconds = SafeStepDeltaSeconds;
		LatestMotorStep.MotorStateBeforeStep = CurrentMotorState;
		SetMotorState(ECFProjectileMotorState::Disabled);
		LatestMotorStep.MotorStateAfterStep = CurrentMotorState;
		SetComponentTickEnabled(false);
		RefreshMotorSnapshot();
		return;
	}

	AdvanceMotorSimulation(DeltaTime);
	RefreshMotorSnapshot();
}

// [v1.0.0] 상태 변경을 기록하고 이벤트를 한 번 발생시킵니다.
void UCFProjectileMotorComp::SetMotorState(const ECFProjectileMotorState NewMotorState)
{
	if (CurrentMotorState == NewMotorState)
	{
		return;
	}

	// [v1.0.0] 상태 변경 이벤트에 전달할 이전 모터 상태입니다.
	const ECFProjectileMotorState PreviousMotorState = CurrentMotorState;
	CurrentMotorState = NewMotorState;
	CurrentMotorSnapshot.CurrentMotorState = CurrentMotorState;
	OnMotorStateChanged.Broadcast(PreviousMotorState, CurrentMotorState);
}

// [v1.0.0] 점화 지연이 끝난 뒤 실제 연소 상태로 진입합니다.
void UCFProjectileMotorComp::EnterBurningState()
{
	if (FMath::Max(ActivePropulsionConfig.BurnDurationSeconds, 0.0f) <= KINDA_SMALL_NUMBER)
	{
		CompleteBurn();
		return;
	}

	SetMotorState(ECFProjectileMotorState::Burning);
	SetComponentTickEnabled(true);
}

// [v1.0.0] 남은 연소 시간이 끝난 뒤 관성 비행 상태로 전환합니다.
void UCFProjectileMotorComp::CompleteBurn()
{
	ElapsedBurnTimeSeconds = FMath::Max(ActivePropulsionConfig.BurnDurationSeconds, 0.0f);
	SetMotorState(ECFProjectileMotorState::BurnedOut);
	SetComponentTickEnabled(false);
}

// [v1.2.0] 한 Tick의 시간 구간을 점화 지연과 연소 구간으로 나눠 impulse 보존형 MotorStep을 생산합니다.
void UCFProjectileMotorComp::AdvanceMotorSimulation(const float DeltaTime)
{
	// [v1.2.0] NaN과 음수를 제거한 이번 outer frame 전체 시간입니다.
	const float SafeStepDeltaSeconds = FMath::IsFinite(DeltaTime)
		? FMath::Max(DeltaTime, 0.0f)
		: 0.0f;

	LatestMotorStep = FCFProjectileMotorStep();
	LatestMotorStep.StepDeltaSeconds = SafeStepDeltaSeconds;
	LatestMotorStep.MotorStateBeforeStep = CurrentMotorState;
	LatestMotorStep.ThrustAccelerationCmPerSecSq =
		FMath::Max(ActivePropulsionConfig.ThrustAccelerationCmPerSecSq, 0.0f);
	LatestMotorStep.MaximumPropelledSpeedCmPerSec =
		FMath::Max(ActivePropulsionConfig.MaximumPropelledSpeed, 1.0f);

	// [v1.2.0] 점화와 연소 구간에 분배할 남은 outer frame 시간입니다.
	float RemainingFrameTime = SafeStepDeltaSeconds;
	ElapsedFlightTimeSeconds += SafeStepDeltaSeconds;

	if (CurrentMotorState == ECFProjectileMotorState::IgnitionDelay)
	{
		// [v1.2.0] 음수 입력을 제거한 총 점화 지연 시간입니다.
		const float SafeIgnitionDelaySeconds = FMath::Max(
			ActivePropulsionConfig.IgnitionDelaySeconds,
			0.0f);

		// [v1.2.0] 현재 시점에 남아 있는 점화 지연 시간입니다.
		const float RemainingIgnitionDelaySeconds = FMath::Max(
			SafeIgnitionDelaySeconds - ElapsedIgnitionDelaySeconds,
			0.0f);

		// [v1.2.0] 이번 frame에서 실제 소비할 점화 지연 시간입니다.
		const float ConsumedIgnitionDelaySeconds = FMath::Min(
			RemainingFrameTime,
			RemainingIgnitionDelaySeconds);

		ElapsedIgnitionDelaySeconds += ConsumedIgnitionDelaySeconds;
		RemainingFrameTime -= ConsumedIgnitionDelaySeconds;

		if (ElapsedIgnitionDelaySeconds + KINDA_SMALL_NUMBER >= SafeIgnitionDelaySeconds)
		{
			ElapsedIgnitionDelaySeconds = SafeIgnitionDelaySeconds;
			EnterBurningState();
		}
	}

	if (CurrentMotorState == ECFProjectileMotorState::Burning
		&& RemainingFrameTime > KINDA_SMALL_NUMBER)
	{
		// [v1.2.0] 음수 입력을 제거한 총 연소 시간입니다.
		const float SafeBurnDurationSeconds = FMath::Max(
			ActivePropulsionConfig.BurnDurationSeconds,
			0.0f);

		// [v1.2.0] 현재 시점에 남아 있는 연소 시간입니다.
		const float RemainingBurnDurationSeconds = FMath::Max(
			SafeBurnDurationSeconds - ElapsedBurnTimeSeconds,
			0.0f);

		// [v1.2.0] 이번 outer frame에서 실제 Burning 상태였던 시간입니다.
		const float AppliedBurnDurationSeconds = FMath::Min(
			RemainingFrameTime,
			RemainingBurnDurationSeconds);

		LatestMotorStep.AppliedBurnDurationSeconds = AppliedBurnDurationSeconds;
		ElapsedBurnTimeSeconds += AppliedBurnDurationSeconds;

		if (ElapsedBurnTimeSeconds + KINDA_SMALL_NUMBER >= SafeBurnDurationSeconds)
		{
			CompleteBurn();
		}
	}

	LatestMotorStep.AppliedBurnFraction =
		SafeStepDeltaSeconds > KINDA_SMALL_NUMBER
		? FMath::Clamp(
			LatestMotorStep.AppliedBurnDurationSeconds / SafeStepDeltaSeconds,
			0.0f,
			1.0f)
		: 0.0f;
	LatestMotorStep.bHasPropulsionImpulse =
		LatestMotorStep.AppliedBurnDurationSeconds > KINDA_SMALL_NUMBER
		&& LatestMotorStep.ThrustAccelerationCmPerSecSq > KINDA_SMALL_NUMBER;
	LatestMotorStep.MotorStateAfterStep = CurrentMotorState;
}

// [v1.2.0] 현재 추진 방향 모드와 ProjectileMovement Velocity에서 Debug용 source-intent 방향을 해석합니다.
FVector UCFProjectileMotorComp::ResolveCurrentThrustDirection() const
{
	if (ThrustDirectionMode == ECFProjectileThrustDirectionMode::CurrentVelocityDirection
		&& IsValid(ActiveProjectileMovementComponent.Get()))
	{
		// [v1.1.0] Guidance가 변경한 현재 Velocity에서 계산한 미사일 추진 방향입니다.
		const FVector CurrentVelocityDirection = ActiveProjectileMovementComponent->Velocity.GetSafeNormal();
		if (!CurrentVelocityDirection.ContainsNaN() && !CurrentVelocityDirection.IsNearlyZero())
		{
			return CurrentVelocityDirection;
		}
	}

	return FixedThrustDirection;
}

// [v1.0.0] 현재 내부 상태를 Blueprint 읽기용 스냅샷에 반영합니다.
void UCFProjectileMotorComp::RefreshMotorSnapshot()
{
	CurrentMotorSnapshot.CurrentMotorState = CurrentMotorState;
	CurrentMotorSnapshot.ElapsedFlightTimeSeconds = ElapsedFlightTimeSeconds;
	CurrentMotorSnapshot.ElapsedBurnTimeSeconds = ElapsedBurnTimeSeconds;
	CurrentMotorSnapshot.RemainingBurnTimeSeconds = FMath::Max(
		FMath::Max(ActivePropulsionConfig.BurnDurationSeconds, 0.0f) - ElapsedBurnTimeSeconds,
		0.0f);
	CurrentMotorSnapshot.CurrentSpeedCmPerSec = ActiveProjectileMovementComponent
		? ActiveProjectileMovementComponent->Velocity.Size()
		: 0.0f;
	CurrentMotorSnapshot.CurrentThrustDirection = ResolveCurrentThrustDirection();
	CurrentMotorSnapshot.bIsProducingThrust = IsProducingThrust();
	CurrentMotorSnapshot.bThrusterFxShouldBeActive = CurrentMotorState == ECFProjectileMotorState::Burning;
	CurrentMotorSnapshot.MotorActivationCount = MotorActivationCount;
}
