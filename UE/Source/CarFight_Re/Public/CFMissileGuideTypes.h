// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-29
// Description: CarFight 물리 제한형 미사일 유도 공용 타입
// Scope: Guidance Mode, 목표 상실 정책, 제한값, 순수 수학 입력·출력과 Debug Snapshot을 제공합니다.
// Changelog:
// - v1.0.0: MG-P0-00 Guidance Config·Command·Snapshot 최초 추가.
// Migration:
// - bUseGuidance=false와 GuideMode=None이 기본값이므로 기존 Projectile·Rocket Velocity를 변경하지 않습니다.
// - 이 파일의 타입은 이동력을 직접 소유하지 않으며 후속 GuideComp가 계산 입력과 결과로 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileGuideTypes.generated.h"

/**
 * 미사일이 관측할 외부 목표 정보의 출처입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileGuideMode : uint8
{
	None UMETA(DisplayName="None"),
	TargetActor UMETA(DisplayName="Target Actor"),
	LaserPoint UMETA(DisplayName="Laser Point"),
	InertialPoint UMETA(DisplayName="Inertial Point"),
	DataLink UMETA(DisplayName="Data Link")
};

/**
 * 기존 목표 정보를 더 이상 사용할 수 없을 때의 비행 정책입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileLostTargetPolicy : uint8
{
	ContinueStraight UMETA(DisplayName="Continue Straight"),
	HoldLastKnownPoint UMETA(DisplayName="Hold Last Known Point"),
	Expire UMETA(DisplayName="Expire")
};

/**
 * 명중 판정과 분리된 유도 종료·실패 진단 사유입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileMissReason : uint8
{
	None UMETA(DisplayName="None"),
	GuidanceDisabled UMETA(DisplayName="Guidance Disabled"),
	InvalidInput UMETA(DisplayName="Invalid Input"),
	BelowMinimumGuidanceSpeed UMETA(DisplayName="Below Minimum Guidance Speed"),
	TargetLost UMETA(DisplayName="Target Lost"),
	SeekerFieldOfViewExceeded UMETA(DisplayName="Seeker Field Of View Exceeded"),
	LockBreakAngleExceeded UMETA(DisplayName="Lock Break Angle Exceeded"),
	Overshoot UMETA(DisplayName="Overshoot"),
	LifeExpired UMETA(DisplayName="Life Expired")
};

/**
 * ProjectileData가 소유할 물리 제한형 Guidance 설정입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuideConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(DisplayName="미사일 유도 사용 (bUseGuidance)", ToolTip="True이고 GuideMode가 None이 아닐 때 후속 MissileGuideComp가 Guidance Command를 계산할 수 있습니다. 기본값 False에서는 기존 Velocity를 변경하지 않습니다."))
	bool bUseGuidance = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(EditCondition="bUseGuidance", DisplayName="유도 모드 (GuideMode)", ToolTip="TargetActor, LaserPoint, InertialPoint 또는 DataLink 중 관측 정보 출처입니다."))
	ECFMissileGuideMode GuideMode = ECFMissileGuideMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(EditCondition="bUseGuidance", DisplayName="목표 상실 정책 (LostTargetPolicy)", ToolTip="목표 참조 또는 신호가 무효화됐을 때 직진, 마지막 지점 유지 또는 만료 중 어떤 정책을 사용할지 정의합니다."))
	ECFMissileLostTargetPolicy LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="10.0", EditCondition="bUseGuidance", DisplayName="비례항법 계수 (NavigationConstant)", ToolTip="시선 각속도와 접근 속도로 요구 횡가속도를 계산할 때 사용할 비례항법 계수입니다."))
	float NavigationConstant = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="720.0", Units="deg/s", EditCondition="bUseGuidance", DisplayName="최대 선회율 deg/s (MaximumTurnRateDegPerSec)", ToolTip="현재 속도 방향이 초당 회전할 수 있는 최대 각도입니다. 위치나 Velocity를 목표 방향으로 즉시 덮어쓰지 않습니다."))
	float MaximumTurnRateDegPerSec = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm/s^2", EditCondition="bUseGuidance", DisplayName="최대 횡가속도 cm/s² (MaximumLateralAccelerationCmPerSecSq)", ToolTip="진행 방향에 수직으로 적용할 수 있는 Guidance 가속도의 최대 크기입니다."))
	float MaximumLateralAccelerationCmPerSecSq = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.001", ClampMax="10.0", Units="s", EditCondition="bUseGuidance", DisplayName="유도 응답 시간 초 (GuidanceResponseTimeSeconds)", ToolTip="후속 GuideComp가 Command 필터링에 사용할 응답 시간입니다. MG-P0-00 수학 함수는 제한값 계약만 제공하고 시간 필터를 적용하지 않습니다."))
	float GuidanceResponseTimeSeconds = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm/s", EditCondition="bUseGuidance", DisplayName="최소 유도 속도 cm/s (MinimumGuidanceSpeedCmPerSec)", ToolTip="이 속도보다 느리면 선회율과 횡가속도 계산을 유효 Guidance Command로 처리하지 않습니다."))
	float MinimumGuidanceSpeedCmPerSec = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="360.0", Units="deg", EditCondition="bUseGuidance", DisplayName="탐색기 시야각 deg (SeekerFieldOfViewDeg)", ToolTip="미사일 전방을 기준으로 탐색기가 관측 가능한 전체 원뿔 각도입니다. 실제 Lock 판정은 MG-P0-04에서 연결합니다."))
	float SeekerFieldOfViewDeg = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg", EditCondition="bUseGuidance", DisplayName="Lock 해제 각도 deg (LockBreakAngleDeg)", ToolTip="이미 추적 중인 목표가 이 각도보다 멀어졌을 때 Lock 상실을 검토합니다. 실제 수명 판정은 MG-P0-04에서 연결합니다."))
	float LockBreakAngleDeg = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileGuidance", meta=(ClampMin="0.0", ClampMax="30.0", Units="s", EditCondition="bUseGuidance", DisplayName="목표 상실 유예 시간 초 (TargetLostGraceTimeSeconds)", ToolTip="목표 관측이 끊긴 뒤 LostTargetPolicy를 확정하기 전까지 유지할 수 있는 유예 시간입니다."))
	float TargetLostGraceTimeSeconds = 0.25f;

	bool IsGuidanceEnabled() const
	{
		return bUseGuidance && GuideMode != ECFMissileGuideMode::None;
	}

	float GetEffectiveNavigationConstant() const
	{
		return FMath::IsFinite(NavigationConstant)
			? FMath::Clamp(NavigationConstant, 0.0f, 10.0f)
			: 0.0f;
	}

	float GetEffectiveMaximumTurnRateDegPerSec() const
	{
		return FMath::IsFinite(MaximumTurnRateDegPerSec)
			? FMath::Clamp(MaximumTurnRateDegPerSec, 0.0f, 720.0f)
			: 0.0f;
	}

	float GetEffectiveMaximumLateralAccelerationCmPerSecSq() const
	{
		return FMath::IsFinite(MaximumLateralAccelerationCmPerSecSq)
			? FMath::Clamp(MaximumLateralAccelerationCmPerSecSq, 0.0f, 1000000.0f)
			: 0.0f;
	}

	float GetEffectiveGuidanceResponseTimeSeconds() const
	{
		return FMath::IsFinite(GuidanceResponseTimeSeconds)
			? FMath::Clamp(GuidanceResponseTimeSeconds, 0.001f, 10.0f)
			: 0.001f;
	}

	float GetEffectiveMinimumGuidanceSpeedCmPerSec() const
	{
		return FMath::IsFinite(MinimumGuidanceSpeedCmPerSec)
			? FMath::Clamp(MinimumGuidanceSpeedCmPerSec, 0.0f, 1000000.0f)
			: 0.0f;
	}

	float GetEffectiveSeekerFieldOfViewDeg() const
	{
		return FMath::IsFinite(SeekerFieldOfViewDeg)
			? FMath::Clamp(SeekerFieldOfViewDeg, 0.0f, 360.0f)
			: 0.0f;
	}

	float GetEffectiveLockBreakAngleDeg() const
	{
		return FMath::IsFinite(LockBreakAngleDeg)
			? FMath::Clamp(LockBreakAngleDeg, 0.0f, 180.0f)
			: 0.0f;
	}

	float GetEffectiveTargetLostGraceTimeSeconds() const
	{
		return FMath::IsFinite(TargetLostGraceTimeSeconds)
			? FMath::Clamp(TargetLostGraceTimeSeconds, 0.0f, 30.0f)
			: 0.0f;
	}

	FCFMissileGuideConfig GetEffectiveConfig() const
	{
		FCFMissileGuideConfig EffectiveConfig = *this;
		if (!EffectiveConfig.bUseGuidance)
		{
			EffectiveConfig.GuideMode = ECFMissileGuideMode::None;
		}
		EffectiveConfig.NavigationConstant = GetEffectiveNavigationConstant();
		EffectiveConfig.MaximumTurnRateDegPerSec = GetEffectiveMaximumTurnRateDegPerSec();
		EffectiveConfig.MaximumLateralAccelerationCmPerSecSq = GetEffectiveMaximumLateralAccelerationCmPerSecSq();
		EffectiveConfig.GuidanceResponseTimeSeconds = GetEffectiveGuidanceResponseTimeSeconds();
		EffectiveConfig.MinimumGuidanceSpeedCmPerSec = GetEffectiveMinimumGuidanceSpeedCmPerSec();
		EffectiveConfig.SeekerFieldOfViewDeg = GetEffectiveSeekerFieldOfViewDeg();
		EffectiveConfig.LockBreakAngleDeg = GetEffectiveLockBreakAngleDeg();
		EffectiveConfig.TargetLostGraceTimeSeconds = GetEffectiveTargetLostGraceTimeSeconds();
		return EffectiveConfig;
	}
};

/**
 * 순수 Guidance 수학 함수가 한 단계 계산에 사용할 값 타입 입력입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuidanceInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector MissileLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector MissileVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	FVector TargetVelocityEstimate = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	float DeltaSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileGuidance")
	bool bHasTarget = false;
};

/**
 * 물리 제한을 적용하기 전·후 횡가속도와 선회율을 보존하는 순수 Guidance 결과입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuidanceCommand
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bCommandValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector RequestedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector AppliedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector LineOfSightAngularVelocityRadPerSec = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float ClosingSpeedCmPerSec = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float SeekerAngleDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float RequestedTurnRateDegPerSec = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float AppliedTurnRateDegPerSec = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bLimitedByLateralAcceleration = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bLimitedByTurnRate = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	ECFMissileMissReason InvalidReason = ECFMissileMissReason::None;
};

/**
 * 후속 MissileGuideComp가 Target·Command·상실 상태를 한 번에 노출할 Debug Snapshot입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileGuideSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	ECFMissileGuideMode GuideMode = ECFMissileGuideMode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	bool bTargetValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float SeekerAngleDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector RequestedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	FVector AppliedLateralAccelerationCmPerSecSq = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	float TargetLostTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileGuidance")
	ECFMissileMissReason MissReason = ECFMissileMissReason::None;
};
