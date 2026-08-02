// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-29
// Description: CarFight 물리 제한형 미사일 Guidance 순수 수학 구현
// Scope: 입력 유효성 검사, 시선 각속도, 비례항법 요구 가속도와 두 물리 제한을 계산합니다.
// Changelog:
// - v1.0.0: MG-P0-00 Bounded Proportional Navigation 계산 계약 최초 구현.
// Migration:
// - 결과는 후속 GuideComp의 입력일 뿐이며 이 파일은 ProjectileMovement를 직접 수정하지 않습니다.

#include "CFMissileGuideMath.h"

namespace
{
	bool IsFiniteVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}
}

float CFMissileGuideMath::CalculateDirectionAngleDeg(
	const FVector& FromDirection,
	const FVector& ToDirection)
{
	const FVector SafeFromDirection = FromDirection.GetSafeNormal();
	const FVector SafeToDirection = ToDirection.GetSafeNormal();
	if (!IsFiniteVector(SafeFromDirection)
		|| !IsFiniteVector(SafeToDirection)
		|| SafeFromDirection.IsNearlyZero()
		|| SafeToDirection.IsNearlyZero())
	{
		return 0.0f;
	}

	const float ClampedDot = FMath::Clamp(
		FVector::DotProduct(SafeFromDirection, SafeToDirection),
		-1.0f,
		1.0f);
	return FMath::RadiansToDegrees(FMath::Acos(ClampedDot));
}

FVector CFMissileGuideMath::CalculateLineOfSightAngularVelocityRadPerSec(
	const FVector& RelativeLocation,
	const FVector& RelativeVelocity)
{
	if (!IsFiniteVector(RelativeLocation) || !IsFiniteVector(RelativeVelocity))
	{
		return FVector::ZeroVector;
	}

	const double DistanceSquared = RelativeLocation.SizeSquared();
	if (!FMath::IsFinite(DistanceSquared) || DistanceSquared <= UE_DOUBLE_SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}

	const FVector AngularVelocity = FVector::CrossProduct(RelativeLocation, RelativeVelocity) / DistanceSquared;
	return IsFiniteVector(AngularVelocity)
		? AngularVelocity
		: FVector::ZeroVector;
}

float CFMissileGuideMath::CalculateTurnRateAccelerationLimitCmPerSecSq(
	const float CurrentSpeedCmPerSec,
	const float MaximumTurnRateDegPerSec)
{
	if (!FMath::IsFinite(CurrentSpeedCmPerSec)
		|| !FMath::IsFinite(MaximumTurnRateDegPerSec)
		|| CurrentSpeedCmPerSec <= 0.0f
		|| MaximumTurnRateDegPerSec <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentSpeedCmPerSec * FMath::DegreesToRadians(MaximumTurnRateDegPerSec);
}

FCFMissileGuidanceCommand CFMissileGuideMath::CalculateBoundedProportionalNavigationCommand(
	const FCFMissileGuidanceInput& Input,
	const FCFMissileGuideConfig& Config)
{
	FCFMissileGuidanceCommand Command;
	const FCFMissileGuideConfig EffectiveConfig = Config.GetEffectiveConfig();

	if (!EffectiveConfig.IsGuidanceEnabled())
	{
		Command.InvalidReason = ECFMissileMissReason::GuidanceDisabled;
		return Command;
	}

	if (!Input.bHasTarget
		|| !FMath::IsFinite(Input.DeltaSeconds)
		|| Input.DeltaSeconds <= 0.0f
		|| !IsFiniteVector(Input.MissileLocation)
		|| !IsFiniteVector(Input.MissileVelocity)
		|| !IsFiniteVector(Input.TargetLocation)
		|| !IsFiniteVector(Input.TargetVelocityEstimate))
	{
		Command.InvalidReason = ECFMissileMissReason::InvalidInput;
		return Command;
	}

	const float MissileSpeedCmPerSec = Input.MissileVelocity.Size();
	if (!FMath::IsFinite(MissileSpeedCmPerSec)
		|| MissileSpeedCmPerSec + KINDA_SMALL_NUMBER < EffectiveConfig.MinimumGuidanceSpeedCmPerSec)
	{
		Command.InvalidReason = ECFMissileMissReason::BelowMinimumGuidanceSpeed;
		return Command;
	}

	const FVector RelativeLocation = Input.TargetLocation - Input.MissileLocation;
	const double DistanceSquared = RelativeLocation.SizeSquared();
	if (!FMath::IsFinite(DistanceSquared) || DistanceSquared <= UE_DOUBLE_SMALL_NUMBER)
	{
		Command.InvalidReason = ECFMissileMissReason::InvalidInput;
		return Command;
	}

	const FVector MissileDirection = Input.MissileVelocity / MissileSpeedCmPerSec;
	const FVector LineOfSightDirection = RelativeLocation.GetSafeNormal();
	const FVector RelativeVelocity = Input.TargetVelocityEstimate - Input.MissileVelocity;
	const FVector LineOfSightAngularVelocity = CalculateLineOfSightAngularVelocityRadPerSec(
		RelativeLocation,
		RelativeVelocity);
	const float ClosingSpeedCmPerSec = FMath::Max(
		-FVector::DotProduct(RelativeVelocity, LineOfSightDirection),
		0.0f);

	FVector RequestedAcceleration = EffectiveConfig.NavigationConstant
		* ClosingSpeedCmPerSec
		* FVector::CrossProduct(LineOfSightAngularVelocity, MissileDirection);
	RequestedAcceleration = FVector::VectorPlaneProject(RequestedAcceleration, MissileDirection);
	if (!IsFiniteVector(RequestedAcceleration))
	{
		Command.InvalidReason = ECFMissileMissReason::InvalidInput;
		return Command;
	}

	const float RequestedAccelerationMagnitude = RequestedAcceleration.Size();
	const float MaximumLateralAcceleration = EffectiveConfig.MaximumLateralAccelerationCmPerSecSq;
	const float MaximumAccelerationByTurnRate = CalculateTurnRateAccelerationLimitCmPerSecSq(
		MissileSpeedCmPerSec,
		EffectiveConfig.MaximumTurnRateDegPerSec);
	const float EffectiveMaximumAcceleration = FMath::Min(
		MaximumLateralAcceleration,
		MaximumAccelerationByTurnRate);

	const FVector AppliedAcceleration = RequestedAcceleration.GetClampedToMaxSize(
		FMath::Max(EffectiveMaximumAcceleration, 0.0f));
	const float AppliedAccelerationMagnitude = AppliedAcceleration.Size();

	Command.bCommandValid = true;
	Command.RequestedLateralAccelerationCmPerSecSq = RequestedAcceleration;
	Command.AppliedLateralAccelerationCmPerSecSq = AppliedAcceleration;
	Command.LineOfSightAngularVelocityRadPerSec = LineOfSightAngularVelocity;
	Command.ClosingSpeedCmPerSec = ClosingSpeedCmPerSec;
	Command.SeekerAngleDeg = CalculateDirectionAngleDeg(MissileDirection, LineOfSightDirection);
	Command.RequestedTurnRateDegPerSec = MissileSpeedCmPerSec > KINDA_SMALL_NUMBER
		? FMath::RadiansToDegrees(RequestedAccelerationMagnitude / MissileSpeedCmPerSec)
		: 0.0f;
	Command.AppliedTurnRateDegPerSec = MissileSpeedCmPerSec > KINDA_SMALL_NUMBER
		? FMath::RadiansToDegrees(AppliedAccelerationMagnitude / MissileSpeedCmPerSec)
		: 0.0f;
	Command.bLimitedByLateralAcceleration = RequestedAccelerationMagnitude > MaximumLateralAcceleration + KINDA_SMALL_NUMBER;
	Command.bLimitedByTurnRate = RequestedAccelerationMagnitude > MaximumAccelerationByTurnRate + KINDA_SMALL_NUMBER;
	Command.InvalidReason = ECFMissileMissReason::None;
	return Command;
}
