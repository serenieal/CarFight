// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-27
// Description: CarFight 추진 발사체 모터 컴포넌트
// Scope: 점화 지연·연소 상태와 outer-frame propulsion step을 생산하고 기존 방향 API/Debug 호환을 유지합니다.
// Changelog:
// - v1.2.0: PFP-P0-02에서 직접 Projectile Velocity write와 최종 world-speed clamp를 제거하고 impulse 보존형 MotorStep producer로 전환.
// - v1.1.0: 현재 ProjectileMovement Velocity 방향을 따라가는 미사일 추진 모드와 호환 Start API 추가.
// - v1.0.0: CF-FQ-028 비유도 로켓 P0 추진 모터 최초 구현.
// Migration:
// - 기존 StartMotor는 FixedLaunchDirection을 선택해 Rocket 결과를 그대로 유지합니다.
// - 미사일은 StartMotorWithDirectionMode에서 CurrentVelocityDirection을 명시적으로 선택합니다.
// - ResetMotor는 Projectile 이동·충돌·피해를 종료하지 않고 모터 상태만 초기화합니다.
// - PFP 이후 실제 추진 방향·가속도·MaximumPropelledSpeed governor 적용은 ProjectileDynamicsComp가 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFProjectileMotorTypes.h"
#include "Components/ActorComponent.h"
#include "CFProjectileMotorComp.generated.h"

class UProjectileMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCFProjectileMotorStateChangedSignature,
	ECFProjectileMotorState,
	PreviousMotorState,
	ECFProjectileMotorState,
	NewMotorState);

/**
 * 점화·연소 상태와 propulsion step을 생산하는 재사용 가능 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFProjectileMotorComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 모터 Tick 기본값과 비활성 상태를 초기화합니다.
	UCFProjectileMotorComp();

		// [v1.0.0] 추진 설정, ProjectileMovement와 발사 방향을 적용해 고정 방향 모터 상태를 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|ProjectileMotor", meta=(DisplayName="발사체 모터 시작 (Start Projectile Motor)", ToolTip="기존 Rocket 호환 함수입니다. ProjectileData 추진 설정과 발사 방향을 적용하고 FixedLaunchDirection 모드로 시작합니다."))
	void StartMotor(
		const FCFProjectilePropulsionConfig& InPropulsionConfig,
		UProjectileMovementComponent* InProjectileMovementComponent,
		const FVector& InLaunchDirection);

	// [v1.1.0] 추진 방향 모드를 명시해 Rocket 또는 Missile 모터 상태를 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|ProjectileMotor", meta=(DisplayName="방향 모드로 발사체 모터 시작 (Start Projectile Motor With Direction Mode)", ToolTip="FixedLaunchDirection은 기존 Rocket처럼 발사 방향을 유지하고, CurrentVelocityDirection은 미사일 Guidance가 변경한 현재 Velocity 방향으로 추진합니다."))
	void StartMotorWithDirectionMode(
		const FCFProjectilePropulsionConfig& InPropulsionConfig,
		UProjectileMovementComponent* InProjectileMovementComponent,
		const FVector& InLaunchDirection,
		ECFProjectileThrustDirectionMode InThrustDirectionMode);

	// [v1.0.0] 현재 추진 상태와 참조를 Pool 재사용 안전 기본값으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|ProjectileMotor", meta=(DisplayName="발사체 모터 초기화 (Reset Projectile Motor)", ToolTip="현재 점화·연소 상태와 ProjectileMovement 참조를 초기화합니다. 발사체 Actor의 이동 정지나 Pool 반환은 호출자가 별도로 처리합니다."))
	void ResetMotor();

	// [v1.0.0] 현재 추진 모터 상태와 속도, 연소 시간을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileMotor", meta=(DisplayName="발사체 모터 스냅샷 반환 (Get Projectile Motor Snapshot)", ToolTip="현재 모터 상태, 경과·남은 연소 시간, 추진 방향과 속도를 포함한 스냅샷입니다."))
	FCFProjectileMotorSnapshot GetMotorSnapshot() const { return CurrentMotorSnapshot; }

	// [v1.0.0] 현재 모터 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileMotor", meta=(DisplayName="현재 발사체 모터 상태 반환 (Get Projectile Motor State)", ToolTip="Inactive, Disabled, IgnitionDelay, Burning 또는 BurnedOut 중 현재 상태를 반환합니다."))
	ECFProjectileMotorState GetMotorState() const { return CurrentMotorState; }

	// [v1.2.0] 현재 Burning 상태에서 Dynamics가 소비할 propulsion step을 생산 중인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileMotor", meta=(DisplayName="발사체 추진 발생 여부 (Is Producing Projectile Thrust)", ToolTip="현재 모터가 Burning 상태이고 유효한 ProjectileMovement가 연결되어 Dynamics에 propulsion step을 생산할 수 있으면 True입니다. MotorComp는 Velocity를 직접 변경하지 않습니다."))
	bool IsProducingThrust() const;

	// [v1.2.0] 같은 outer frame의 Dynamics가 정확히 한 번 소비할 최신 MotorStep을 반환하고 pending step을 비웁니다.
	FCFProjectileMotorStep ConsumeLatestMotorStep();

	// [v1.0.0] Debug 패널과 Automation에서 사용할 모터 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileMotor", meta=(DisplayName="발사체 모터 요약 생성 (Build Projectile Motor Summary)", ToolTip="현재 모터 상태, 점화·연소 시간, 속도, 추진 방향과 활성화 횟수를 문자열로 반환합니다."))
	FString BuildMotorSummary() const;

		// [v1.0.0] 모터 상태가 변경됐을 때 발생하는 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|ProjectileMotor", meta=(DisplayName="발사체 모터 상태 변경 (OnProjectileMotorStateChanged)", ToolTip="Inactive, Disabled, IgnitionDelay, Burning 또는 BurnedOut 상태가 변경될 때 이전 상태와 새 상태를 전달합니다."))
	FCFProjectileMotorStateChangedSignature OnMotorStateChanged;

	// [v1.0.0] Automation과 결정적 진단에서 월드 프레임 진행 없이 모터 시간 구간을 한 단계 진행합니다.
	void AdvanceMotorForAutomation(float DeltaTime);

protected:
	// [v1.2.0] 점화 지연과 연소 시간을 진행하고 이번 outer frame의 propulsion step을 생산합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// [v1.0.0] 상태 변경을 기록하고 이벤트를 한 번 발생시킵니다.
	void SetMotorState(ECFProjectileMotorState NewMotorState);

	// [v1.0.0] 점화 지연이 끝난 뒤 실제 연소 상태로 진입합니다.
	void EnterBurningState();

	// [v1.0.0] 남은 연소 시간이 끝난 뒤 관성 비행 상태로 전환합니다.
	void CompleteBurn();

	// [v1.2.0] 한 Tick의 시간 구간을 점화 지연과 연소 구간으로 나눠 impulse 보존형 MotorStep을 생산합니다.
	void AdvanceMotorSimulation(float DeltaTime);

	// [v1.1.0] 현재 추진 방향 모드와 ProjectileMovement Velocity에서 Debug용 source-intent 방향을 해석합니다.
	FVector ResolveCurrentThrustDirection() const;

	// [v1.0.0] 현재 내부 상태를 Blueprint 읽기용 스냅샷에 반영합니다.
	void RefreshMotorSnapshot();

	// [v1.0.0] 이번 활성화에 적용된 추진 설정입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="활성 추진 설정 (ActivePropulsionConfig)", ToolTip="현재 활성화에 적용된 ProjectileData 추진 설정의 복사본입니다."))
	FCFProjectilePropulsionConfig ActivePropulsionConfig;

	// [v1.2.0] 현재 속도 Debug와 source-intent 방향 fallback을 읽을 ProjectileMovement입니다.
	UPROPERTY(Transient)
	TObjectPtr<UProjectileMovementComponent> ActiveProjectileMovementComponent = nullptr;

		// [v1.0.0] 발사 시 확정되어 FixedLaunchDirection 모드에서 사용하는 월드 추진 방향입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="고정 추진 방향 (FixedThrustDirection)", ToolTip="비유도 Rocket이 발사 시 저장하고 연소 동안 고정 사용하는 월드 방향입니다. 미사일 모드는 현재 Velocity가 무효할 때만 Fallback으로 사용합니다."))
	FVector FixedThrustDirection = FVector::ForwardVector;

	// [v1.1.0] 이번 활성화에서 실제 추진 방향을 해석할 모드입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="추진 방향 모드 (ThrustDirectionMode)", ToolTip="FixedLaunchDirection은 기존 Rocket 고정 방향이고 CurrentVelocityDirection은 Guidance가 변경한 현재 속도 방향입니다."))
	ECFProjectileThrustDirectionMode ThrustDirectionMode = ECFProjectileThrustDirectionMode::FixedLaunchDirection;

	// [v1.0.0] 현재 모터 상태입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 모터 상태 (CurrentMotorState)", ToolTip="현재 추진 모터의 실행 상태입니다."))
	ECFProjectileMotorState CurrentMotorState = ECFProjectileMotorState::Inactive;

	// [v1.0.0] 이번 활성화의 누적 모터 시뮬레이션 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="경과 비행 시간 초 (ElapsedFlightTimeSeconds)", ToolTip="이번 모터 활성화에서 누적된 총 시뮬레이션 시간입니다."))
	float ElapsedFlightTimeSeconds = 0.0f;

	// [v1.0.0] 현재까지 소비한 점화 지연 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="경과 점화 지연 초 (ElapsedIgnitionDelaySeconds)", ToolTip="이번 활성화에서 이미 소비한 점화 지연 시간입니다."))
	float ElapsedIgnitionDelaySeconds = 0.0f;

	// [v1.0.0] 현재까지 실제 추진 가속을 적용한 연소 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="경과 연소 시간 초 (ElapsedBurnTimeSeconds)", ToolTip="이번 활성화에서 실제 추진 가속을 적용한 누적 시간입니다."))
	float ElapsedBurnTimeSeconds = 0.0f;

	// [v1.0.0] Pool 재사용을 포함한 누적 모터 시작 횟수입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="모터 활성화 횟수 (MotorActivationCount)", ToolTip="StartMotor가 호출된 누적 횟수입니다. ResetMotor에서는 초기화하지 않습니다."))
	int32 MotorActivationCount = 0;

	// [v1.2.0] 같은 outer frame의 Dynamics가 한 번 소비할 최신 propulsion step입니다.
	UPROPERTY(Transient)
	FCFProjectileMotorStep LatestMotorStep;

	// [v1.0.0] Blueprint와 Debug가 한 번에 읽을 현재 모터 스냅샷입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileMotor|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 모터 스냅샷 (CurrentMotorSnapshot)", ToolTip="현재 상태와 시간, 속도, source-intent 방향, 추진 FX 요청을 모은 런타임 스냅샷입니다."))
	FCFProjectileMotorSnapshot CurrentMotorSnapshot;
};
