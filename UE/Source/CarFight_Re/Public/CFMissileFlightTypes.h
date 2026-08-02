// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-29
// Description: CarFight 미사일 비행 상태와 비활성 Foundation 설정
// Scope: 발사 후 독립 비행 상태, 공격 프로파일, Clearance 조건과 Debug Snapshot 타입을 제공합니다.
// Changelog:
// - v1.0.0: MG-P0-00 Flight State·Attack Profile·Flight Config·Snapshot 최초 추가.
// Migration:
// - bUseMissileFlight=false가 기본값이므로 기존 포탄과 비유도 Rocket의 이동·추진·충돌 결과를 변경하지 않습니다.
// - 이 파일은 상태와 데이터 계약만 제공하며 Projectile Actor 또는 ProjectileMovement에 연결하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileFlightTypes.generated.h"

/**
 * 런처에서 분리된 미사일의 독립 비행 진행 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileFlightState : uint8
{
	Inactive UMETA(DisplayName="Inactive"),
	Released UMETA(DisplayName="Released"),
	Ejection UMETA(DisplayName="Ejection"),
	Clearance UMETA(DisplayName="Clearance"),
	Ignition UMETA(DisplayName="Ignition"),
	Boost UMETA(DisplayName="Boost"),
	Transition UMETA(DisplayName="Transition"),
	GuidedFlight UMETA(DisplayName="Guided Flight"),
	Terminal UMETA(DisplayName="Terminal"),
	Impact UMETA(DisplayName="Impact"),
	Expired UMETA(DisplayName="Expired")
};

/**
 * 비행 상태가 제공할 중간 목표와 전환 형태입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileAttackProfile : uint8
{
	Direct UMETA(DisplayName="Direct"),
	Loft UMETA(DisplayName="Loft"),
	PitchOver UMETA(DisplayName="Pitch Over"),
	TopAttack UMETA(DisplayName="Top Attack")
};

/**
 * ProjectileData가 소유할 미사일 비행 상태 설정입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileFlightConfig
{
	GENERATED_BODY()

	// [v1.0.0] 미사일 전용 Flight State를 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileFlight", meta=(DisplayName="미사일 비행 사용 (bUseMissileFlight)", ToolTip="True이면 후속 MissileFlightComp가 Released부터 GuidedFlight까지의 상태를 관리합니다. 기본값 False에서는 기존 Projectile·Rocket 동작을 그대로 유지합니다."))
	bool bUseMissileFlight = false;

	// [v1.0.0] Direct, Loft, PitchOver 또는 TopAttack 중 사용할 비행 프로파일입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileFlight", meta=(EditCondition="bUseMissileFlight", DisplayName="공격 프로파일 (AttackProfile)", ToolTip="비행 상태가 사용할 중간 목표와 전환 형태입니다. Guide는 이 프로파일을 직접 소유하지 않습니다."))
	ECFMissileAttackProfile AttackProfile = ECFMissileAttackProfile::Direct;

	// [v1.0.0] 분리 후 점화·전환을 허용하기 전 필요한 최소 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileFlight", meta=(ClampMin="0.0", ClampMax="30.0", Units="s", EditCondition="bUseMissileFlight", DisplayName="최소 Clearance 시간 초 (MinimumClearanceTimeSeconds)", ToolTip="런처 분리 후 점화 또는 전환을 허용하기 전에 기다릴 최소 시간입니다."))
	float MinimumClearanceTimeSeconds = 0.15f;

	// [v1.0.0] 분리 위치로부터 점화·전환을 허용하기 전 필요한 최소 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileFlight", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm", EditCondition="bUseMissileFlight", DisplayName="최소 Clearance 거리 cm (MinimumClearanceDistanceCm)", ToolTip="런처 분리 위치로부터 이 거리 이상 이동해야 Clearance 조건을 만족합니다."))
	float MinimumClearanceDistanceCm = 300.0f;

	// [v1.0.0] 사출 방향에서 비행 프로파일 방향으로 전환하는 최소 계획 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileFlight", meta=(ClampMin="0.0", ClampMax="30.0", Units="s", EditCondition="bUseMissileFlight", DisplayName="전환 시간 초 (TransitionDurationSeconds)", ToolTip="Angled 또는 Vertical 사출 뒤 목표 비행 방향으로 전환할 계획 시간입니다. 실제 선회는 Guidance 제한을 따라야 합니다."))
	float TransitionDurationSeconds = 0.35f;

	// [v1.0.0] 목표 접근 중 별도 Terminal 상태를 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileFlight", meta=(EditCondition="bUseMissileFlight", DisplayName="종말 비행 사용 (bUseTerminalPhase)", ToolTip="True이면 목표 접근 거리에서 Terminal 상태를 사용할 수 있습니다. MG-P0-00에서는 데이터만 제공하고 런타임 전환은 하지 않습니다."))
	bool bUseTerminalPhase = false;

	// [v1.0.0] Terminal 상태 진입 후보로 사용할 목표까지의 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|MissileFlight", meta=(ClampMin="0.0", ClampMax="1000000.0", Units="cm", EditCondition="bUseMissileFlight && bUseTerminalPhase", DisplayName="종말 비행 시작 거리 cm (TerminalPhaseStartDistanceCm)", ToolTip="목표까지의 거리가 이 값 이하일 때 Terminal 상태 진입을 검토합니다."))
	float TerminalPhaseStartDistanceCm = 1500.0f;

	float GetEffectiveMinimumClearanceTimeSeconds() const
	{
		return FMath::IsFinite(MinimumClearanceTimeSeconds)
			? FMath::Clamp(MinimumClearanceTimeSeconds, 0.0f, 30.0f)
			: 0.0f;
	}

	float GetEffectiveMinimumClearanceDistanceCm() const
	{
		return FMath::IsFinite(MinimumClearanceDistanceCm)
			? FMath::Clamp(MinimumClearanceDistanceCm, 0.0f, 1000000.0f)
			: 0.0f;
	}

	float GetEffectiveTransitionDurationSeconds() const
	{
		return FMath::IsFinite(TransitionDurationSeconds)
			? FMath::Clamp(TransitionDurationSeconds, 0.0f, 30.0f)
			: 0.0f;
	}

	float GetEffectiveTerminalPhaseStartDistanceCm() const
	{
		return FMath::IsFinite(TerminalPhaseStartDistanceCm)
			? FMath::Clamp(TerminalPhaseStartDistanceCm, 0.0f, 1000000.0f)
			: 0.0f;
	}

	ECFMissileFlightState GetInitialFlightState() const
	{
		return bUseMissileFlight
			? ECFMissileFlightState::Released
			: ECFMissileFlightState::Inactive;
	}

	bool IsClearanceSatisfied(const float ElapsedFlightTimeSeconds, const float DistanceFromReleaseCm) const
	{
		if (!bUseMissileFlight
			|| !FMath::IsFinite(ElapsedFlightTimeSeconds)
			|| !FMath::IsFinite(DistanceFromReleaseCm))
		{
			return false;
		}

		return ElapsedFlightTimeSeconds + KINDA_SMALL_NUMBER >= GetEffectiveMinimumClearanceTimeSeconds()
			&& DistanceFromReleaseCm + KINDA_SMALL_NUMBER >= GetEffectiveMinimumClearanceDistanceCm();
	}

	FCFMissileFlightConfig GetEffectiveConfig() const
	{
		FCFMissileFlightConfig EffectiveConfig = *this;
		EffectiveConfig.MinimumClearanceTimeSeconds = GetEffectiveMinimumClearanceTimeSeconds();
		EffectiveConfig.MinimumClearanceDistanceCm = GetEffectiveMinimumClearanceDistanceCm();
		EffectiveConfig.TransitionDurationSeconds = GetEffectiveTransitionDurationSeconds();
		EffectiveConfig.TerminalPhaseStartDistanceCm = GetEffectiveTerminalPhaseStartDistanceCm();
		return EffectiveConfig;
	}
};

/**
 * 후속 MissileFlightComp가 Debug와 Pool Reset 검증에 사용할 비행 스냅샷입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFMissileFlightSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileFlight")
	ECFMissileFlightState CurrentFlightState = ECFMissileFlightState::Inactive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileFlight")
	ECFMissileAttackProfile AttackProfile = ECFMissileAttackProfile::Direct;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileFlight")
	float ElapsedFlightTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileFlight")
	float DistanceFromReleaseCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileFlight")
	bool bClearanceSatisfied = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileFlight")
	bool bGuidanceWindowOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileFlight")
	bool bTerminalPhaseActive = false;
};
