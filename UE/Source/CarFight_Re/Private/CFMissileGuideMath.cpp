// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-09-07
// Description: CarFight 물리 제한형 미사일 Guidance 순수 수학 구현
// Scope: 입력 유효성 검사, 시선 각속도, 비례항법·Pursuit 요구 가속도와 두 물리 제한을 계산합니다.
// Changelog:
// - v1.1.0: MG-P0-12C PurePursuit/LeadPursuit/PN Course Capture용 Bounded Pursuit Command 계산을 추가.
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

// [v1.1.0] 지정 Target 지점을 직접 향하는 데 필요한 횡가속도를 계산하고 기존 선회율·횡가속 상한을 동일하게 적용합니다.
FCFMissileGuidanceCommand CFMissileGuideMath::CalculateBoundedPursuitCommand(
	const FCFMissileGuidanceInput& Input,
	const FCFMissileGuideConfig& Config)
{
	// [v1.1.0] Pursuit 계산 결과와 유효성·물리 제한 진단을 반환할 Command입니다.
	FCFMissileGuidanceCommand Command;

	// [v1.1.0] 비정상 Config 수치를 제거하고 계산에 사용할 안전 보정 설정입니다.
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
		|| !IsFiniteVector(Input.TargetLocation))
	{
		Command.InvalidReason = ECFMissileMissReason::InvalidInput;
		return Command;
	}

	// [v1.1.0] 현재 진행 방향을 보존하면서 횡기동 크기를 계산할 미사일 속력입니다.
	const float MissileSpeedCmPerSec = Input.MissileVelocity.Size();
	if (!FMath::IsFinite(MissileSpeedCmPerSec)
		|| MissileSpeedCmPerSec + KINDA_SMALL_NUMBER < EffectiveConfig.MinimumGuidanceSpeedCmPerSec)
	{
		Command.InvalidReason = ECFMissileMissReason::BelowMinimumGuidanceSpeed;
		return Command;
	}

	// [v1.1.0] 미사일에서 이번 GuidanceAimPoint까지의 상대 위치입니다.
	const FVector RelativeLocation = Input.TargetLocation - Input.MissileLocation;
	if (!IsFiniteVector(RelativeLocation) || RelativeLocation.IsNearlyZero())
	{
		Command.InvalidReason = ECFMissileMissReason::InvalidInput;
		return Command;
	}

	// [v1.1.0] 현재 Missile Velocity의 정규화 진행 방향입니다.
	const FVector MissileDirection = Input.MissileVelocity / MissileSpeedCmPerSec;

	// [v1.1.0] 이번 Pursuit가 직접 향해야 할 GuidanceAimPoint 시선 방향입니다.
	const FVector LineOfSightDirection = RelativeLocation.GetSafeNormal();

	// [v1.1.0] 현재 진행 방향에서 시선 방향으로 꺾기 위해 사용할 진행 방향 수직 성분입니다.
	const FVector LateralDirection = FVector::VectorPlaneProject(LineOfSightDirection, MissileDirection).GetSafeNormal();

	// [v1.1.0] 현재 진행 방향과 GuidanceAimPoint 사이의 전체 방향 오차입니다.
	const float DirectionErrorDeg = CalculateDirectionAngleDeg(MissileDirection, LineOfSightDirection);

	// [v1.1.0] 현재 DeltaTime 안에 방향 오차를 모두 줄이려 할 때 필요한 이상적 선회율입니다.
	const float RequestedTurnRateRadPerSec = FMath::DegreesToRadians(DirectionErrorDeg) / Input.DeltaSeconds;

	// [v1.1.0] 이상적 선회율을 현재 속력에서 횡가속도로 환산한 크기입니다.
	const float RequestedAccelerationMagnitude = MissileSpeedCmPerSec * RequestedTurnRateRadPerSec;

	// [v1.1.0] 정확히 정반대처럼 횡방향이 정의되지 않는 경우 0이 되며 MG-P0-12D의 deterministic tie-break가 별도로 책임질 요구 횡가속도입니다.
	const FVector RequestedAcceleration = LateralDirection.IsNearlyZero()
		? FVector::ZeroVector
		: LateralDirection * RequestedAccelerationMagnitude;

	// [v1.1.0] ProjectileData가 직접 허용한 최대 횡가속도입니다.
	const float MaximumLateralAcceleration = EffectiveConfig.MaximumLateralAccelerationCmPerSecSq;

	// [v1.1.0] 현재 속력과 최대 선회율이 허용하는 횡가속도 상한입니다.
	const float MaximumAccelerationByTurnRate = CalculateTurnRateAccelerationLimitCmPerSecSq(
		MissileSpeedCmPerSec,
		EffectiveConfig.MaximumTurnRateDegPerSec);

	// [v1.1.0] 횡가속도와 선회율 두 제한 중 더 엄격한 최종 상한입니다.
	const float EffectiveMaximumAcceleration = FMath::Min(
		MaximumLateralAcceleration,
		MaximumAccelerationByTurnRate);

	// [v1.1.0] 두 물리 상한을 모두 적용한 실제 Pursuit 횡가속도입니다.
	const FVector AppliedAcceleration = RequestedAcceleration.GetClampedToMaxSize(
		FMath::Max(EffectiveMaximumAcceleration, 0.0f));

	// [v1.1.0] 실제 적용 횡가속도의 크기입니다.
	const float AppliedAccelerationMagnitude = AppliedAcceleration.Size();

	// [v1.1.0] 현재 Missile 진행 속도 중 GuidanceAimPoint 방향으로 향하는 전방 성분입니다.
	const float ForwardClosingVelocityCmPerSec = FVector::DotProduct(Input.MissileVelocity, LineOfSightDirection);

	Command.bCommandValid = true;
	Command.RequestedLateralAccelerationCmPerSecSq = RequestedAcceleration;
	Command.AppliedLateralAccelerationCmPerSecSq = AppliedAcceleration;
	Command.LineOfSightAngularVelocityRadPerSec = FVector::ZeroVector;
	Command.ClosingSpeedCmPerSec = FMath::Max(ForwardClosingVelocityCmPerSec, 0.0f);
	Command.SeekerAngleDeg = DirectionErrorDeg;
	Command.RequestedTurnRateDegPerSec = FMath::RadiansToDegrees(RequestedTurnRateRadPerSec);
	Command.AppliedTurnRateDegPerSec = MissileSpeedCmPerSec > KINDA_SMALL_NUMBER
		? FMath::RadiansToDegrees(AppliedAccelerationMagnitude / MissileSpeedCmPerSec)
		: 0.0f;
	Command.bLimitedByLateralAcceleration = RequestedAccelerationMagnitude > MaximumLateralAcceleration + KINDA_SMALL_NUMBER;
	Command.bLimitedByTurnRate = RequestedAccelerationMagnitude > MaximumAccelerationByTurnRate + KINDA_SMALL_NUMBER;
	Command.InvalidReason = ECFMissileMissReason::None;
	return Command;
}
