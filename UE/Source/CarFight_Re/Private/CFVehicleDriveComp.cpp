// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.1
// Date: 2026-04-01
// Description: CarFight 차량 Drive 상태/입력 해석 컴포넌트 구현 (입력 적용/속도 계산 중복 정리)

#include "CFVehicleDriveComp.h"

#include "CarFightVehicleUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

namespace
{
	// Owner Actor 기준 현재 속도를 km/h로 계산합니다.
	float GetOwnerSpeedKmh(const UCFVehicleDriveComp& VehicleDriveComp, const bool bProjectToForwardAxis)
	{
		// 현재 DriveComp의 Owner Actor 포인터입니다.
		const AActor* OwnerActor = VehicleDriveComp.GetOwner();
		if (!OwnerActor)
		{
			return 0.0f;
		}

		// 현재 Owner Actor의 선형 속도 벡터입니다.
		const FVector VehicleVelocity = OwnerActor->GetVelocity();
		if (!bProjectToForwardAxis)
		{
			return VehicleVelocity.Size() * 0.036f;
		}

		// 현재 Owner Actor의 전방 방향 벡터입니다.
		const FVector VehicleForwardVector = OwnerActor->GetActorForwardVector();

		// 현재 전방 축 기준 선속도(cm/s)입니다.
		const float ForwardSpeedCmPerSec = FVector::DotProduct(VehicleVelocity, VehicleForwardVector);
		return ForwardSpeedCmPerSec * 0.036f;
	}

	// VehicleMovement가 준비되면 지정 입력 적용 함수를 실행하고 DriveState를 갱신합니다.
	template<typename ApplyMovementInputFunc>
	void ApplyMovementInputAndRefreshState(UCFVehicleDriveComp& VehicleDriveComp, ApplyMovementInputFunc&& ApplyMovementInput)
	{
		// 현재 사용할 VehicleMovement 컴포넌트 포인터입니다.
		UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = VehicleDriveComp.GetVehicleMovementComponent();
		if (!VehicleMovementComponent && !VehicleDriveComp.CacheVehicleMovementComponent())
		{
			VehicleDriveComp.UpdateDriveState();
			return;
		}

		// 캐시 재시도 이후 확정된 VehicleMovement 컴포넌트 포인터입니다.
		VehicleMovementComponent = VehicleDriveComp.GetVehicleMovementComponent();
		if (VehicleMovementComponent)
		{
			ApplyMovementInput(*VehicleMovementComponent);
		}

		VehicleDriveComp.UpdateDriveState();
	}
}

UCFVehicleDriveComp::UCFVehicleDriveComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bEnableDriveStateUpdates = true;
	bEnableDriveStateHysteresis = true;
	bUsePerStateHoldTimes = true;
	DriveStateMinimumHoldTimeSeconds = 0.12f;
	IdleStateMinimumHoldTimeSeconds = 0.10f;
	ReversingStateMinimumHoldTimeSeconds = 0.18f;
	AirborneStateMinimumHoldTimeSeconds = 0.08f;
	IdleEnterSpeedThresholdKmh = 0.75f;
	IdleExitSpeedThresholdKmh = 1.5f;
	ReverseEnterSpeedThresholdKmh = 1.25f;
	ReverseExitSpeedThresholdKmh = 0.75f;
	AirborneMinSpeedThresholdKmh = 3.0f;
	ActiveInputThreshold = 0.05f;
	AirborneVerticalSpeedThresholdCmPerSec = 100.0f;
	bUseWheelContactSurfaceHint = true;
	bTreatOppositeThrottleAsBrake = true;
	PreviousDriveState = ECFVehicleDriveState::Disabled;
	CurrentDriveState = ECFVehicleDriveState::Disabled;
	LastDriveStateChangeTimeSeconds = -1.0f;
	bDriveStateChangedThisFrame = false;
	DriveStateChangeCount = 0;
	LastDriveStateTransitionSummary = TEXT("DriveStateTransition: None");
}

void UCFVehicleDriveComp::BeginPlay()
{
	Super::BeginPlay();
	CacheVehicleMovementComponent();
	UpdateDriveState();
}

void UCFVehicleDriveComp::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bEnableDriveStateUpdates)
	{
		UpdateDriveState();
	}
}

bool UCFVehicleDriveComp::CacheVehicleMovementComponent()
{
	CachedVehicleMovementComponent = nullptr;
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		CurrentDriveState = ECFVehicleDriveState::Disabled;
		return false;
	}
	CachedVehicleMovementComponent = OwnerActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	if (!CachedVehicleMovementComponent)
	{
		CurrentDriveState = ECFVehicleDriveState::Disabled;
		return false;
	}
	return true;
}

bool UCFVehicleDriveComp::IsDriveReady() const
{
	return (CachedVehicleMovementComponent != nullptr);
}

UChaosWheeledVehicleMovementComponent* UCFVehicleDriveComp::GetVehicleMovementComponent() const
{
	return CachedVehicleMovementComponent;
}

float UCFVehicleDriveComp::GetCurrentSpeedKmh() const
{
	return GetOwnerSpeedKmh(*this, false);
}

float UCFVehicleDriveComp::GetForwardSpeedKmh() const
{
	return GetOwnerSpeedKmh(*this, true);
}

FString UCFVehicleDriveComp::GetDriveStateDebugString(bool bIncludeTransitionSummary) const
{
	FString DriveDebugSummary = FString::Printf(
		TEXT("Prev=%s | Changed=%s | %s"),
		*UCarFightVehicleUtils::ConvDriveStateToDisplayString(PreviousDriveState),
		bDriveStateChangedThisFrame ? TEXT("True") : TEXT("False"),
		*UCarFightVehicleUtils::MakeDriveStateSnapshotDebugString(LastDriveStateSnapshot));

	if (bIncludeTransitionSummary)
	{
		DriveDebugSummary += FString::Printf(TEXT(" | Transition=%s"), *LastDriveStateTransitionSummary);
	}

	return DriveDebugSummary;
}

void UCFVehicleDriveComp::UpdateDriveState()
{
	LastDriveStateSnapshot = BuildDriveStateSnapshot();
	UpdateDriveStateTransitionSummary(LastDriveStateSnapshot);
}

void UCFVehicleDriveComp::ApplyDriveStateConfig(const FCFVehicleDriveStateConfig& InDriveStateConfig)
{
	if (!InDriveStateConfig.bUseDriveStateOverrides)
	{
		return;
	}

	bEnableDriveStateHysteresis = InDriveStateConfig.bEnableDriveStateHysteresis;
	bUsePerStateHoldTimes = InDriveStateConfig.bUsePerStateHoldTimes;
	DriveStateMinimumHoldTimeSeconds = InDriveStateConfig.DriveStateMinimumHoldTimeSeconds;
	IdleStateMinimumHoldTimeSeconds = InDriveStateConfig.IdleStateMinimumHoldTimeSeconds;
	ReversingStateMinimumHoldTimeSeconds = InDriveStateConfig.ReversingStateMinimumHoldTimeSeconds;
	AirborneStateMinimumHoldTimeSeconds = InDriveStateConfig.AirborneStateMinimumHoldTimeSeconds;
	IdleEnterSpeedThresholdKmh = InDriveStateConfig.IdleEnterSpeedThresholdKmh;
	IdleExitSpeedThresholdKmh = InDriveStateConfig.IdleExitSpeedThresholdKmh;
	ReverseEnterSpeedThresholdKmh = InDriveStateConfig.ReverseEnterSpeedThresholdKmh;
	ReverseExitSpeedThresholdKmh = InDriveStateConfig.ReverseExitSpeedThresholdKmh;
	AirborneMinSpeedThresholdKmh = InDriveStateConfig.AirborneMinSpeedThresholdKmh;
	AirborneVerticalSpeedThresholdCmPerSec = InDriveStateConfig.AirborneVerticalSpeedThresholdCmPerSec;
	ActiveInputThreshold = InDriveStateConfig.ActiveInputThreshold;
	bTreatOppositeThrottleAsBrake = InDriveStateConfig.bTreatOppositeThrottleAsBrake;
}

FCFVehicleDriveStateSnapshot UCFVehicleDriveComp::BuildDriveStateSnapshot()
{
	FCFVehicleDriveStateSnapshot NewDriveStateSnapshot;
	NewDriveStateSnapshot.CurrentInputState = CurrentInputState;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		NewDriveStateSnapshot.CurrentDriveState = ECFVehicleDriveState::Disabled;
		NewDriveStateSnapshot.bIsGrounded = false;
		return NewDriveStateSnapshot;
	}

	const FVector VehicleVelocity = OwnerActor->GetVelocity();
	const FVector VehicleForwardVector = OwnerActor->GetActorForwardVector();
	const float CurrentSpeedCmPerSec = VehicleVelocity.Size();
	const float ForwardSpeedCmPerSec = FVector::DotProduct(VehicleVelocity, VehicleForwardVector);
	const float AbsoluteVerticalSpeedCmPerSec = FMath::Abs(VehicleVelocity.Z);

	NewDriveStateSnapshot.CurrentSpeedKmh = ConvertCmPerSecToKmh(CurrentSpeedCmPerSec);
	NewDriveStateSnapshot.ForwardSpeedKmh = ConvertCmPerSecToKmh(ForwardSpeedCmPerSec);
	NewDriveStateSnapshot.bIsGrounded = !ShouldEnterAirborneState(NewDriveStateSnapshot, AbsoluteVerticalSpeedCmPerSec);

	if (!bEnableDriveStateUpdates)
	{
		NewDriveStateSnapshot.CurrentDriveState = ECFVehicleDriveState::Disabled;
		return NewDriveStateSnapshot;
	}

	NewDriveStateSnapshot.CurrentDriveState = EvaluateDriveState(NewDriveStateSnapshot);
	return NewDriveStateSnapshot;
}

void UCFVehicleDriveComp::ApplyThrottleInput(float InThrottleValue)
{
	CurrentInputState.ThrottleInput = InThrottleValue;
	ApplyMovementInputAndRefreshState(*this, [InThrottleValue](UChaosWheeledVehicleMovementComponent& VehicleMovementComponent)
	{
		VehicleMovementComponent.SetThrottleInput(InThrottleValue);
	});
}

void UCFVehicleDriveComp::ApplySteeringInput(float InSteeringValue)
{
	CurrentInputState.SteeringInput = InSteeringValue;
	ApplyMovementInputAndRefreshState(*this, [InSteeringValue](UChaosWheeledVehicleMovementComponent& VehicleMovementComponent)
	{
		VehicleMovementComponent.SetSteeringInput(InSteeringValue);
	});
}

void UCFVehicleDriveComp::ApplyBrakeInput(float InBrakeValue)
{
	CurrentInputState.BrakeInput = InBrakeValue;
	ApplyMovementInputAndRefreshState(*this, [InBrakeValue](UChaosWheeledVehicleMovementComponent& VehicleMovementComponent)
	{
		VehicleMovementComponent.SetBrakeInput(InBrakeValue);
	});
}

void UCFVehicleDriveComp::ApplyHandbrakeInput(bool bInHandbrakePressed)
{
	CurrentInputState.bHandbrakePressed = bInHandbrakePressed;
	ApplyMovementInputAndRefreshState(*this, [bInHandbrakePressed](UChaosWheeledVehicleMovementComponent& VehicleMovementComponent)
	{
		VehicleMovementComponent.SetHandbrakeInput(bInHandbrakePressed);
	});
}

ECFVehicleDriveState UCFVehicleDriveComp::EvaluateDriveState(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot) const
{
	const float CurrentSpeedKmh = InDriveStateSnapshot.CurrentSpeedKmh;
	const float ForwardSpeedKmh = InDriveStateSnapshot.ForwardSpeedKmh;
	const bool bHasThrottleIntent = (FMath::Abs(InDriveStateSnapshot.CurrentInputState.ThrottleInput) > ActiveInputThreshold);
	const bool bHasBrakeIntent = (InDriveStateSnapshot.CurrentInputState.BrakeInput > ActiveInputThreshold) || InDriveStateSnapshot.CurrentInputState.bHandbrakePressed;
	const bool bThrottleIntentForward = (InDriveStateSnapshot.CurrentInputState.ThrottleInput > ActiveInputThreshold);
	const bool bThrottleIntentBackward = (InDriveStateSnapshot.CurrentInputState.ThrottleInput < -ActiveInputThreshold);

	const float IdleEnterThreshold = bEnableDriveStateHysteresis ? IdleEnterSpeedThresholdKmh : IdleExitSpeedThresholdKmh;
	const float IdleExitThreshold = bEnableDriveStateHysteresis ? IdleExitSpeedThresholdKmh : IdleEnterSpeedThresholdKmh;
	const float ReverseEnterThreshold = bEnableDriveStateHysteresis ? ReverseEnterSpeedThresholdKmh : ReverseExitSpeedThresholdKmh;
	const float ReverseExitThreshold = bEnableDriveStateHysteresis ? ReverseExitSpeedThresholdKmh : ReverseEnterThreshold;

	const bool bIsNearlyStopped = (CurrentDriveState == ECFVehicleDriveState::Idle)
		? (CurrentSpeedKmh <= IdleExitThreshold)
		: (CurrentSpeedKmh <= IdleEnterThreshold);

	const bool bIsMovingForward = (ForwardSpeedKmh > IdleExitThreshold);
	const bool bIsMovingBackward = (CurrentDriveState == ECFVehicleDriveState::Reversing)
		? (ForwardSpeedKmh < -ReverseExitThreshold)
		: (ForwardSpeedKmh < -ReverseEnterThreshold);

	const bool bHasOppositeThrottleBrakeIntent = bTreatOppositeThrottleAsBrake && ((bIsMovingForward && bThrottleIntentBackward) || (bIsMovingBackward && bThrottleIntentForward));

	if (!InDriveStateSnapshot.bIsGrounded)
	{
		return ECFVehicleDriveState::Airborne;
	}
	if (bIsNearlyStopped)
	{
		if (bThrottleIntentBackward)
		{
			return ECFVehicleDriveState::Reversing;
		}
		if (bThrottleIntentForward)
		{
			return ECFVehicleDriveState::Accelerating;
		}
		return ECFVehicleDriveState::Idle;
	}
	if (bHasBrakeIntent || bHasOppositeThrottleBrakeIntent)
	{
		return ECFVehicleDriveState::Braking;
	}
	if (bIsMovingBackward)
	{
		if (bThrottleIntentBackward || !bHasThrottleIntent)
		{
			return ECFVehicleDriveState::Reversing;
		}
		return ECFVehicleDriveState::Braking;
	}
	if (bThrottleIntentForward)
	{
		return ECFVehicleDriveState::Accelerating;
	}
	if (bHasThrottleIntent)
	{
		return ECFVehicleDriveState::Braking;
	}
	return ECFVehicleDriveState::Coasting;
}

bool UCFVehicleDriveComp::ShouldEnterAirborneState(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot, float InVerticalSpeedCmPerSec) const
{
	if (bUseWheelContactSurfaceHint && HasAnyWheelContactSurfaceHint())
	{
		return false;
	}
	const bool bHasEnoughVerticalSpeed = (InVerticalSpeedCmPerSec > AirborneVerticalSpeedThresholdCmPerSec);
	const bool bHasEnoughOverallSpeed = (InDriveStateSnapshot.CurrentSpeedKmh > AirborneMinSpeedThresholdKmh);
	return bHasEnoughVerticalSpeed && bHasEnoughOverallSpeed;
}

bool UCFVehicleDriveComp::HasAnyWheelContactSurfaceHint() const
{
	if (!CachedVehicleMovementComponent)
	{
		return false;
	}

	const int32 WheelCount = CachedVehicleMovementComponent->GetNumWheels();
	for (int32 WheelIndex = 0; WheelIndex < WheelCount; ++WheelIndex)
	{
		if (CachedVehicleMovementComponent->GetPhysMaterial(WheelIndex) != nullptr)
		{
			return true;
		}
	}
	return false;
}

float UCFVehicleDriveComp::GetCurrentStateMinimumHoldTimeSeconds() const
{
	if (!bUsePerStateHoldTimes)
	{
		return DriveStateMinimumHoldTimeSeconds;
	}
	switch (CurrentDriveState)
	{
	case ECFVehicleDriveState::Idle:
		return IdleStateMinimumHoldTimeSeconds;
	case ECFVehicleDriveState::Reversing:
		return ReversingStateMinimumHoldTimeSeconds;
	case ECFVehicleDriveState::Airborne:
		return AirborneStateMinimumHoldTimeSeconds;
	case ECFVehicleDriveState::Disabled:
		return 0.0f;
	case ECFVehicleDriveState::Accelerating:
	case ECFVehicleDriveState::Coasting:
	case ECFVehicleDriveState::Braking:
	default:
		return DriveStateMinimumHoldTimeSeconds;
	}
}

bool UCFVehicleDriveComp::CanApplyStateTransition(ECFVehicleDriveState CandidateDriveState, float CurrentWorldTimeSeconds) const
{
	if (!bEnableDriveStateHysteresis)
	{
		return true;
	}
	if (CandidateDriveState == CurrentDriveState)
	{
		return true;
	}
	if (LastDriveStateChangeTimeSeconds < 0.0f)
	{
		return true;
	}
	const float HeldDurationSeconds = CurrentWorldTimeSeconds - LastDriveStateChangeTimeSeconds;
	return (HeldDurationSeconds >= GetCurrentStateMinimumHoldTimeSeconds());
}

void UCFVehicleDriveComp::UpdateDriveStateTransitionSummary(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot)
{
	const float CurrentWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const ECFVehicleDriveState CandidateDriveState = InDriveStateSnapshot.CurrentDriveState;
	const ECFVehicleDriveState ApprovedDriveState = CanApplyStateTransition(CandidateDriveState, CurrentWorldTimeSeconds) ? CandidateDriveState : CurrentDriveState;

	PreviousDriveState = CurrentDriveState;
	CurrentDriveState = ApprovedDriveState;
	bDriveStateChangedThisFrame = (PreviousDriveState != CurrentDriveState);
	LastDriveStateSnapshot = InDriveStateSnapshot;
	LastDriveStateSnapshot.CurrentDriveState = CurrentDriveState;

	if (bDriveStateChangedThisFrame)
	{
		LastDriveStateChangeTimeSeconds = CurrentWorldTimeSeconds;
		++DriveStateChangeCount;
		LastDriveStateTransitionSummary = FString::Printf(
			TEXT("DriveStateTransition: %s -> %s | Candidate=%s | Hold=%.2f | Speed=%.2f km/h | Forward=%.2f km/h | Grounded=%s | Count=%d"),
			*UCarFightVehicleUtils::ConvDriveStateToDisplayString(PreviousDriveState),
			*UCarFightVehicleUtils::ConvDriveStateToDisplayString(CurrentDriveState),
			*UCarFightVehicleUtils::ConvDriveStateToDisplayString(CandidateDriveState),
			GetCurrentStateMinimumHoldTimeSeconds(),
			InDriveStateSnapshot.CurrentSpeedKmh,
			InDriveStateSnapshot.ForwardSpeedKmh,
			InDriveStateSnapshot.bIsGrounded ? TEXT("True") : TEXT("False"),
			DriveStateChangeCount);
		return;
	}

	LastDriveStateTransitionSummary = FString::Printf(
		TEXT("DriveStateTransition: %s (unchanged) | Candidate=%s | Hold=%.2f | Speed=%.2f km/h | Forward=%.2f km/h | Grounded=%s | Count=%d"),
		*UCarFightVehicleUtils::ConvDriveStateToDisplayString(CurrentDriveState),
		*UCarFightVehicleUtils::ConvDriveStateToDisplayString(CandidateDriveState),
		GetCurrentStateMinimumHoldTimeSeconds(),
		InDriveStateSnapshot.CurrentSpeedKmh,
		InDriveStateSnapshot.ForwardSpeedKmh,
		InDriveStateSnapshot.bIsGrounded ? TEXT("True") : TEXT("False"),
		DriveStateChangeCount);
}

float UCFVehicleDriveComp::ConvertCmPerSecToKmh(float InSpeedCmPerSec) const
{
	return InSpeedCmPerSec * 0.036f;
}
