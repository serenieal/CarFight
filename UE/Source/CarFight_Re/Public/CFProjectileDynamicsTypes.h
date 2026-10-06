// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-27
// Description: CF-FQ-056 발사체 공통 비행 물리 진단 타입
// Scope: Ballistic/Rocket/GuidedMissile Dynamics 모드와 최종 비중력 가속도 합성 스냅샷을 제공합니다.
// Changelog:
// - v1.0.0: PFP-P0-02 단일 Projectile Dynamics authority용 모드와 읽기 전용 Snapshot 최초 추가.
// Migration:
// - 기존 ProjectileData와 Blueprint 저장값을 변경하지 않습니다.
// - Snapshot은 Debug/Automation 출력 전용이며 게임 로직의 물리 입력으로 다시 사용하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFProjectileDynamicsTypes.generated.h"

/**
 * 공통 발사체 Dynamics가 현재 적용하는 물리 합성 모드입니다.
 */
UENUM(BlueprintType)
enum class ECFProjectileDynamicsMode : uint8
{
	Ballistic UMETA(DisplayName="탄도 비행 (Ballistic)"),
	Rocket UMETA(DisplayName="비유도 로켓 (Rocket)"),
	GuidedMissile UMETA(DisplayName="유도 미사일 (Guided Missile)")
};

/**
 * 한 outer frame에서 Dynamics가 계산·적용한 비중력 물리 결과입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFProjectileDynamicsSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] 현재 ProjectileData에서 해석한 Dynamics 모드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="Dynamics 모드 (DynamicsMode)", ToolTip="Ballistic, Rocket, Guided Missile 중 현재 발사체에 적용된 공통 비행 물리 모드입니다."))
	ECFProjectileDynamicsMode DynamicsMode = ECFProjectileDynamicsMode::Ballistic;

	// [v1.0.0] ProjectileMovement가 실제 적분할 현재 월드 중력 가속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="월드 중력 가속도 (WorldGravityAcceleration)", ToolTip="ProjectileMovement.GetGravityZ를 사용해 읽은 실제 중력 가속도입니다. Dynamics가 이 값을 제거하거나 별도로 적분하지 않습니다."))
	FVector WorldGravityAcceleration = FVector::ZeroVector;

	// [v1.0.0] 이번 outer frame 중 실제 Burning 시간이 차지한 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="적용 연소 비율 (AppliedBurnFraction)", ToolTip="MotorStep의 실제 Burning 시간을 outer frame DeltaTime으로 나눈 값입니다. 점화·연소 종료 경계의 총 impulse를 보존합니다."))
	float AppliedBurnFraction = 0.0f;

	// [v1.0.0] axial governor 적용 전 추진축 방향 요구 가속도 크기입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="요구 축방향 가속도 (RequestedAxialAcceleration)", ToolTip="MotorStep 연소 비율을 반영한 축방향 추진 요구 가속도 크기입니다."))
	float RequestedAxialAcceleration = 0.0f;

	// [v1.0.0] axial governor와 Rocket TVC 분배 뒤 실제 추진축 방향으로 queue한 가속도 크기입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="적용 축방향 가속도 (AppliedAxialAcceleration)", ToolTip="MaximumPropelledSpeed axial governor와 Rocket TVC 분배를 거친 최종 추진축 가속도 크기입니다."))
	float AppliedAxialAcceleration = 0.0f;

	// [v1.0.0] Rocket 발사축의 횡속도 오차와 횡중력을 줄이기 위해 요구한 TVC 횡가속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="Rocket 안정화 요구 (RocketStabilizationRequest)", ToolTip="Launch Axis 수직 속도 오차 회복과 횡중력 대응을 합친 Rocket TVC 요구입니다. 기존 총 추력 밖의 무료 가속도가 아닙니다."))
	FVector RocketStabilizationRequest = FVector::ZeroVector;

	// [v1.0.0] Guidance 응답 필터와 FlightTangent 수직 투영을 거친 미사일 유도 횡가속도 요구입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="Guidance 횡제어 요구 (GuidanceLateralRequest)", ToolTip="MissileGuideComp의 RequestedLateralAcceleration을 FlightTangent 수직면으로 투영하고 응답 필터를 적용한 Guidance 요구입니다."))
	FVector GuidanceLateralRequest = FVector::ZeroVector;

	// [v1.0.0] 현재 FlightTangent에 수직인 중력을 상쇄하기 위한 같은 lateral budget의 요구입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="횡중력 대응 요구 (GravityLateralCompensationRequest)", ToolTip="실제 횡중력의 반대 방향 요구입니다. Guidance와 같은 최대 횡가속도·선회율 예산을 소비합니다."))
	FVector GravityLateralCompensationRequest = FVector::ZeroVector;

	// [v1.0.0] Guidance와 횡중력 대응을 합친 포화 전 최종 미사일 횡제어 요구입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="합성 횡제어 요구 (CombinedLateralControlRequest)", ToolTip="필터된 Guidance 요구와 횡중력 대응 요구를 합친 뒤 shared lateral budget을 적용하기 전 값입니다."))
	FVector CombinedLateralControlRequest = FVector::ZeroVector;

	// [v1.0.0] shared lateral budget 또는 Rocket TVC 한도 안에서 실제 적용한 횡가속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="적용 횡제어 가속도 (AppliedLateralControlAcceleration)", ToolTip="Rocket TVC 또는 Missile shared lateral budget에서 최종 허용된 횡가속도입니다."))
	FVector AppliedLateralControlAcceleration = FVector::ZeroVector;

	// [v1.0.0] 이번 outer frame에 ProjectileMovement AddForce로 정확히 한 번 전달한 비중력 가속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="Queue된 비중력 가속도 (QueuedNonGravityAcceleration)", ToolTip="ProjectileMovement가 중력과 함께 적분하도록 AddForce로 한 번 queue한 추진+비행제어 가속도입니다."))
	FVector QueuedNonGravityAcceleration = FVector::ZeroVector;

	// [v1.0.0] 추진축 속도가 MaximumPropelledSpeed에 도달해 추가 axial thrust가 제한됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="축방향 Governor 활성 (bAxialGovernorActive)", ToolTip="현재 추진축 방향 속도가 MaximumPropelledSpeed 이상이라 순수 axial thrust 요구를 중단하면 True입니다. 월드 Velocity를 clamp하지 않습니다."))
	bool bAxialGovernorActive = false;

	// [v1.0.0] Rocket이 이번 frame에 실제 TVC 횡가속도를 사용했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="Rocket 안정화 활성 (bRocketStabilizationActive)", ToolTip="Burning 중 Launch-Axis Stabilization이 실제 추력 벡터를 기울여 횡가속도를 적용하면 True입니다."))
	bool bRocketStabilizationActive = false;

	// [v1.0.0] Rocket TVC 또는 Missile shared lateral request가 물리 한도를 초과해 잘렸는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileDynamics", meta=(DisplayName="횡제어 포화 (bLateralControlSaturated)", ToolTip="요구 횡가속도가 Rocket TVC 각도 또는 Missile 최대 횡가속도/선회율 한도를 넘어 포화되면 True입니다."))
	bool bLateralControlSaturated = false;
};
