// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-27
// Description: CF-FQ-056 공통 발사체 비행 물리 합성 컴포넌트
// Scope: MotorStep·Guidance request·실제 중력을 읽어 Rocket TVC와 Missile shared lateral control을 합성하고 ProjectileMovement AddForce로 전달합니다.
// Changelog:
// - v1.0.0: PFP-P0-02 단일 non-gravity acceleration authority 최초 구현.
// Migration:
// - ProjectileMovement는 중력·Velocity integration·Sub-step·Sweep/Collision authority로 유지합니다.
// - BlueprintSpawnableComponent로 노출하지 않으며 ACFProjectileActor가 정확히 하나의 기본 subobject를 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileFlightTypes.h"
#include "CFMissileGuideTypes.h"
#include "CFProjectileDynamicsTypes.h"
#include "CFProjectileLaunchTypes.h"
#include "CFProjectileMotorTypes.h"
#include "Components/ActorComponent.h"
#include "CFProjectileDynamicsComp.generated.h"

class UCFMissileFlightComp;
class UCFMissileGuideComp;
class UCFProjectileData;
class UCFProjectileMotorComp;
class UProjectileMovementComponent;

/**
 * ProjectileMovement 앞에서 모든 CarFight 비중력 발사체 가속도를 한 번 합성하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType)
class CARFIGHT_RE_API UCFProjectileDynamicsComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] PrePhysics Tick과 비활성 기본 상태를 구성합니다.
	UCFProjectileDynamicsComp();

	// [v1.0.0] 이번 활성화의 ProjectileData·LaunchContext와 producer/runtime 참조를 복사해 Dynamics를 시작합니다.
	void StartProjectileDynamics(
		const UCFProjectileData* InProjectileData,
		const FCFProjectileLaunchContext& InLaunchContext,
		UProjectileMovementComponent* InProjectileMovementComponent,
		UCFProjectileMotorComp* InProjectileMotorComponent,
		UCFMissileFlightComp* InMissileFlightComponent,
		UCFMissileGuideComp* InMissileGuideComponent);

	// [v1.0.0] pending force와 runtime filter/snapshot을 Pool 재사용 안전 기본값으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|ProjectileDynamics", meta=(DisplayName="발사체 Dynamics 초기화 (Reset Projectile Dynamics)", ToolTip="현재 Dynamics 상태와 ProjectileMovement pending force를 초기화합니다. 발사체 Actor의 이동 정지나 Pool 반환은 호출자가 별도로 처리합니다."))
	void ResetProjectileDynamics();

	// [v1.0.0] Debug와 Automation에서 읽을 현재 Dynamics 결과를 값으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectileDynamics", meta=(DisplayName="발사체 Dynamics 스냅샷 반환 (Get Projectile Dynamics Snapshot)", ToolTip="현재 모드, 실제 중력, 추진·TVC·Guidance 요구, 포화 결과와 AddForce에 queue한 최종 비중력 가속도를 반환합니다."))
	FCFProjectileDynamicsSnapshot GetProjectileDynamicsSnapshot() const { return CurrentDynamicsSnapshot; }

	// [v1.0.0] Automation에서 월드 프레임 진행 없이 같은 Dynamics 계산을 한 단계 실행합니다.
	void AdvanceDynamicsForAutomation(float DeltaTime);

protected:
	// [v1.0.0] 같은 PrePhysics frame의 producer 결과를 읽어 최종 AddForce를 queue합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// [v1.0.0] Runtime Tick과 Automation이 공유하는 한 outer frame의 Dynamics 계산을 수행합니다.
	void AdvanceDynamicsSimulation(float DeltaTime);

	// [v1.0.0] MissileFlight 우선, Propulsion 차순으로 Ballistic/Rocket/GuidedMissile 모드를 결정합니다.
	ECFProjectileDynamicsMode ResolveDynamicsMode() const;

	// [v1.0.0] ProjectileMovement가 실제 적분할 GravityScale 적용 월드 중력을 반환합니다.
	FVector ResolveWorldGravityAcceleration() const;

	// [v1.0.0] MotorStep의 burn fraction과 axial governor로 축방향 추진 가속도 요구/적용값을 해석합니다.
	float ResolveAxialPropulsionAcceleration(
		const FCFProjectileMotorStep& MotorStep,
		const FVector& AxialDirection,
		bool& bOutAxialGovernorActive);

	// [v1.0.0] 기존 총 엔진 추력 안에서 Launch-Axis TVC 안정화와 axial governor를 함께 계산합니다.
	FVector ResolveRocketStabilizedThrust(
		const FCFProjectileMotorStep& MotorStep,
		const FVector& CurrentVelocity,
		const FVector& WorldGravityAcceleration);

	// [v1.0.0] Guidance request와 횡중력 대응을 하나의 최대 횡가속도·선회율 예산에서 합성합니다.
	FVector ResolveMissileLateralControl(
		float DeltaTime,
		const FVector& CurrentVelocity,
		const FVector& FlightTangent,
		const FVector& WorldGravityAcceleration);

	// [v1.0.0] 최종 비중력 가속도를 ProjectileMovement AddForce에 정확히 한 번 전달합니다.
	void QueueNonGravityAcceleration(const FVector& NonGravityAcceleration);

	// [v1.0.0] 이번 활성화에서 복사한 추진 설정입니다.
	UPROPERTY(Transient)
	FCFProjectilePropulsionConfig ActivePropulsionConfig;

	// [v1.0.0] 이번 활성화에서 복사한 유효 Missile Flight 설정입니다.
	UPROPERTY(Transient)
	FCFMissileFlightConfig ActiveMissileFlightConfig;

	// [v1.0.0] 이번 활성화에서 복사한 유효 Missile Guidance 성능 설정입니다.
	UPROPERTY(Transient)
	FCFMissileGuideConfig ActiveMissileGuideConfig;

	// [v1.0.0] Rocket 안정화와 invalid/zero Velocity fallback에 사용할 발사 순간 기준축입니다.
	UPROPERTY(Transient)
	FVector LaunchAxis = FVector::ForwardVector;

	// [v1.0.0] 실제 중력과 최종 비중력 force를 소유하는 ProjectileMovement입니다.
	UPROPERTY(Transient)
	TObjectPtr<UProjectileMovementComponent> ActiveProjectileMovementComponent = nullptr;

	// [v1.0.0] 점화·연소 시간과 frame burn fraction을 생산하는 MotorComp입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFProjectileMotorComp> ActiveProjectileMotorComponent = nullptr;

	// [v1.0.0] GuidedMissile mode와 비행 활성 상태를 제공하는 MissileFlightComp입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFMissileFlightComp> ActiveMissileFlightComponent = nullptr;

	// [v1.0.0] Target/Seeker/Law가 계산한 authoritative Guidance request를 제공하는 MissileGuideComp입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFMissileGuideComp> ActiveMissileGuideComponent = nullptr;

	// [v1.0.0] 현재 활성화에서 고정된 Dynamics 모드입니다.
	UPROPERTY(Transient)
	ECFProjectileDynamicsMode CurrentDynamicsMode = ECFProjectileDynamicsMode::Ballistic;

	// [v1.0.0] Missile GuidanceResponseTimeSeconds를 적용해 frame 사이 유지하는 실제 Guidance request filter 상태입니다.
	UPROPERTY(Transient)
	FVector FilteredGuidanceRequest = FVector::ZeroVector;

	// [v1.0.0] Blueprint/Debug/Automation이 읽을 현재 합성 결과입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectileDynamics|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 Dynamics 스냅샷 (CurrentDynamicsSnapshot)", ToolTip="이번 outer frame에 계산한 중력, 추진, TVC, shared lateral control과 최종 AddForce 결과입니다."))
	FCFProjectileDynamicsSnapshot CurrentDynamicsSnapshot;
};
