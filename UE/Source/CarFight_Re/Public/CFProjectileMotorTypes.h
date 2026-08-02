// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-02
// Description: CarFight 추진 발사체 모터 공용 타입
// Scope: 비유도 로켓과 미사일의 추진 설정, 방향 모드, 모터 상태와 런타임 스냅샷을 제공합니다.
// Changelog:
// - v1.1.0: 기존 Rocket 고정 방향과 Missile 현재 Velocity 방향을 분리하는 ThrustDirectionMode 추가.
// - v1.0.0: 점화 지연, 연소 시간, 추진 가속도와 최대 추진 속도를 포함한 P0 추진 계약 추가.
// Migration:
// - 기존 ProjectileData는 bUsePropulsion=false 기본값으로 기존 초기 속도 비행을 유지합니다.
// - 기존 StartMotor 호출은 FixedLaunchDirection을 사용해 비유도 Rocket 결과를 유지합니다.
// - 미사일 Flight Runtime만 CurrentVelocityDirection을 명시적으로 선택합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFProjectileMotorTypes.generated.h"

/**
 * 추진 발사체 모터의 현재 실행 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFProjectileMotorState : uint8
{
	Inactive UMETA(DisplayName="Inactive"),
	Disabled UMETA(DisplayName="Disabled"),
	IgnitionDelay UMETA(DisplayName="Ignition Delay"),
	Burning UMETA(DisplayName="Burning"),
	BurnedOut UMETA(DisplayName="Burned Out")
};

/**
 * 추진 가속을 계산할 월드 방향의 해석 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFProjectileThrustDirectionMode : uint8
{
	FixedLaunchDirection UMETA(DisplayName="고정 발사 방향 (Fixed Launch Direction)"),
	CurrentVelocityDirection UMETA(DisplayName="현재 속도 방향 (Current Velocity Direction)")
};

/**
 * ProjectileData가 소유하는 비유도 로켓 P0 추진 설정입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFProjectilePropulsionConfig
{
	GENERATED_BODY()

	// [v1.0.0] 실제 자체 추진 가속을 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Propulsion", meta=(DisplayName="자체 추진 사용 (bUsePropulsion)", ToolTip="True이면 InitialSpeed로 발사된 뒤 점화 지연과 연소 시간 동안 고정 발사 방향으로 가속합니다. False이면 기존 Projectile처럼 InitialSpeed를 유지합니다."))
	bool bUsePropulsion = false;

	// [v1.0.0] 발사 후 실제 추진 연소가 시작되기 전까지의 대기 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Propulsion", meta=(ClampMin="0.0", EditCondition="bUsePropulsion", DisplayName="점화 지연 초 (IgnitionDelaySeconds)", ToolTip="발사대에서 분리된 뒤 로켓 모터가 Burning 상태로 진입하기 전까지 기다릴 시간입니다. 0이면 발사 즉시 점화합니다."))
	float IgnitionDelaySeconds = 0.05f;

	// [v1.0.0] 로켓 모터가 실제 가속을 발생시키는 총 연소 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Propulsion", meta=(ClampMin="0.0", EditCondition="bUsePropulsion", DisplayName="연소 시간 초 (BurnDurationSeconds)", ToolTip="Burning 상태에서 추진 가속을 적용할 총 시간입니다. 연소가 끝나면 BurnedOut 상태로 전환하고 기존 속도와 중력으로 관성 비행합니다."))
	float BurnDurationSeconds = 1.0f;

	// [v1.0.0] 연소 중 발사 방향으로 더할 가속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Propulsion", meta=(ClampMin="0.0", EditCondition="bUsePropulsion", DisplayName="추진 가속도 cm/s² (ThrustAccelerationCmPerSecSq)", ToolTip="로켓 모터가 Burning 상태일 때 발사 방향으로 적용할 가속도입니다. 단위는 cm/s²입니다."))
	float ThrustAccelerationCmPerSecSq = 9000.0f;

	// [v1.0.0] 추진 가속으로 도달할 수 있는 최대 발사체 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileData|Propulsion", meta=(ClampMin="1.0", EditCondition="bUsePropulsion", DisplayName="최대 추진 속도 cm/s (MaximumPropelledSpeed)", ToolTip="Burning 상태에서 추진 가속을 적용한 뒤 제한할 최대 속도입니다. InitialSpeed보다 작게 설정해도 발사 직후 속도를 강제로 낮추지는 않으며, 추진 가속 적용 시점부터 상한으로 사용합니다."))
	float MaximumPropelledSpeed = 10000.0f;
};

/**
 * 현재 추진 모터 상태를 Debug와 Blueprint에서 읽기 위한 스냅샷입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFProjectileMotorSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] 현재 추진 모터 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="현재 모터 상태 (CurrentMotorState)", ToolTip="Inactive, Disabled, IgnitionDelay, Burning 또는 BurnedOut 중 현재 상태입니다."))
	ECFProjectileMotorState CurrentMotorState = ECFProjectileMotorState::Inactive;

	// [v1.0.0] 모터 시작 이후 누적된 시뮬레이션 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="경과 비행 시간 초 (ElapsedFlightTimeSeconds)", ToolTip="이번 활성화에서 추진 모터가 누적한 총 시뮬레이션 시간입니다."))
	float ElapsedFlightTimeSeconds = 0.0f;

	// [v1.0.0] 실제 Burning 상태에서 누적된 연소 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="경과 연소 시간 초 (ElapsedBurnTimeSeconds)", ToolTip="이번 활성화에서 실제 추진 가속을 적용한 누적 시간입니다."))
	float ElapsedBurnTimeSeconds = 0.0f;

	// [v1.0.0] 현재 설정 기준 남은 연소 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="남은 연소 시간 초 (RemainingBurnTimeSeconds)", ToolTip="BurnDurationSeconds에서 현재 경과 연소 시간을 뺀 값입니다."))
	float RemainingBurnTimeSeconds = 0.0f;

	// [v1.0.0] 현재 ProjectileMovement 속력입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="현재 속도 cm/s (CurrentSpeedCmPerSec)", ToolTip="현재 ProjectileMovement Velocity의 크기입니다."))
	float CurrentSpeedCmPerSec = 0.0f;

	// [v1.0.0] P0에서 고정 사용하는 발사 시 추진 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="현재 추진 방향 (CurrentThrustDirection)", ToolTip="P0 비유도 로켓이 발사 시 저장해 연소 동안 고정 사용하는 월드 추진 방향입니다."))
	FVector CurrentThrustDirection = FVector::ForwardVector;

	// [v1.0.0] 현재 프레임에 추진 가속을 생산할 수 있는 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="추진 발생 여부 (bIsProducingThrust)", ToolTip="현재 모터 상태가 Burning이고 유효한 ProjectileMovement가 연결되어 실제 추진 가속을 적용할 수 있으면 True입니다."))
	bool bIsProducingThrust = false;

	// [v1.0.0] 추진 화염 FX가 활성 상태여야 하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="추진 FX 활성 요청 (bThrusterFxShouldBeActive)", ToolTip="실제 모터 상태가 Burning이면 True입니다. Niagara 자산 존재 여부와는 독립적인 게임플레이 상태입니다."))
	bool bThrusterFxShouldBeActive = false;

	// [v1.0.0] 이 컴포넌트가 StartMotor로 활성화된 누적 횟수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|ProjectileMotor", meta=(DisplayName="모터 활성화 횟수 (MotorActivationCount)", ToolTip="Projectile Pool 재사용 시 이전 활성화와 현재 활성화를 구분하기 위한 누적 시작 횟수입니다."))
	int32 MotorActivationCount = 0;
};
