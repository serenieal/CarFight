// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.6.0
// Date: 2026-07-14
// Description: CarFight 공통 발사체 Actor
// Scope: ProjectileData 기반 이동, 풀링, 고속 연속 충돌과 직접 피해 적용 결과 보존을 제공합니다.
// Changelog:
// - v1.6.0: 첫 유효 Impact에서 DamageHitContext와 DamageApplyResult를 생성·저장하는 최소 Damage Runtime 연결 추가.
// - v1.5.0: Sweep/Sub-step 설정 적용, 보조 Sphere Sweep, 활성화별 단일 Impact 처리 상태를 추가.
// - v1.4.0: Projectile Object Channel 적용과 마지막 피격 컴포넌트 이름 Debug 보존 추가.
// - v1.3.0: Pool 반환 뒤 Damage HitContext Debug를 만들 수 있도록 마지막 충돌 Transform과 ProjectileData 참조를 보존.
// - v1.2.0: 충돌 / 수명 / 수동 / 활성화 실패 비활성화 사유와 비행 시간 디버그 값을 추가.
// - v1.1.0: ProjectilePoolComp 소유 반환 경로와 Destroy 정책 setter 추가.
// - v1.0.0: ProjectileData 기반 메시 / 충돌 / 이동 / 수명 적용과 Activate / Deactivate 생명주기 추가.
// Migration:
// - Projectile Actor Blueprint는 이 클래스를 부모로 만들고, 실제 탄종 차이는 ProjectileData에서 조정한다.
// - Pool을 쓰지 않는 경우 기본 동작은 비활성화 시 Destroy이다.
// - ProjectilePoolComp가 소유자로 지정한 Actor는 비활성화 시 Destroy하지 않고 Pool로 반환한다.
// - 기존 DeactivateProjectile 함수는 유지하고 내부에서 Manual 사유로 기록한다.
// - 첫 유효 Impact에서 DamageData.BaseDamage를 대상 VehicleHealthComp에 한 번 적용하고 HitContext / ApplyResult를 Pool 반환 뒤까지 보존한다.
// - 기존 OnComponentHit과 신규 보조 Sweep은 ResolveProjectileImpact를 공유하며 첫 유효 Impact만 처리한다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "CFProjectileActor.generated.h"

class UCFProjectileData;
class UCFProjectilePoolComp;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * 발사체가 비활성화된 원인입니다.
 */
UENUM(BlueprintType)
enum class ECFProjectileDeactivateReason : uint8
{
	None UMETA(DisplayName="None"),
	InvalidActivation UMETA(DisplayName="InvalidActivation"),
	Manual UMETA(DisplayName="Manual"),
	Hit UMETA(DisplayName="Hit"),
	LifeExpired UMETA(DisplayName="LifeExpired")
};

/**
 * ProjectileData를 읽어 이동 / 충돌 / 표시를 구성하는 공통 발사체 Actor입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API ACFProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 발사체 Actor 컴포넌트와 비활성 상태를 초기화합니다.
	ACFProjectileActor();

	// [v1.0.0] ProjectileData와 발사 방향을 적용해 발사체를 활성화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Projectile", meta=(DisplayName="발사체 활성화 (Activate Projectile)", ToolTip="ProjectileData, 발사 방향, 발사 주체를 적용해 발사체를 활성화합니다. Pool 사용 시에도 같은 진입점을 사용합니다."))
	void ActivateProjectile(UCFProjectileData* InProjectileData, const FVector& InLaunchDirection, AActor* InInstigatorActor);

	// [v1.0.0] 발사체 이동과 충돌을 멈추고 비활성화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Projectile", meta=(DisplayName="발사체 비활성화 (Deactivate Projectile)", ToolTip="발사체 이동과 충돌을 멈추고 비활성화합니다. 기본값은 Destroy이며, Pool 사용 시 반환 경로로 바꿀 수 있습니다."))
	void DeactivateProjectile();

	// [v1.0.0] 현재 발사체가 활성 발사 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile", meta=(DisplayName="발사체 활성 여부 (Is Projectile Active)", ToolTip="현재 발사체가 발사되어 이동 중인지 반환합니다."))
	bool IsProjectileActive() const { return bProjectileActive; }

	// [v1.0.0] 현재 발사체에 적용된 ProjectileData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile", meta=(DisplayName="활성 ProjectileData 반환 (Get Active Projectile Data)", ToolTip="현재 발사체에 적용된 ProjectileData입니다. 비활성 상태에서는 None일 수 있습니다."))
	UCFProjectileData* GetActiveProjectileData() const { return ActiveProjectileData; }

	// [v1.2.0] 마지막 비활성화 사유를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Debug", meta=(DisplayName="마지막 비활성화 사유 반환 (Get Last Deactivate Reason)", ToolTip="마지막으로 발사체가 비활성화된 사유입니다. 충돌, 수명 종료, 수동 비활성화, 활성화 실패를 구분합니다."))
	ECFProjectileDeactivateReason GetLastDeactivateReason() const { return LastDeactivateReason; }

	// [v1.2.0] 마지막으로 비활성화된 발사체 ID를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Debug", meta=(DisplayName="마지막 비활성화 발사체 ID 반환 (Get Last Deactivated Projectile Id)", ToolTip="마지막으로 비활성화될 때 적용되어 있던 ProjectileData의 ProjectileId입니다."))
	FName GetLastDeactivatedProjectileId() const { return LastDeactivatedProjectileId; }

	// [v1.2.0] 마지막 비행 시간을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Debug", meta=(DisplayName="마지막 비행 시간 반환 (Get Last Flight Duration Seconds)", ToolTip="마지막 활성화 시점부터 비활성화 시점까지의 비행 시간입니다."))
	float GetLastFlightDurationSeconds() const { return LastFlightDurationSeconds; }

	// [v1.2.0] 마지막 충돌 대상 Actor 이름을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Debug", meta=(DisplayName="마지막 충돌 Actor 이름 반환 (Get Last Hit Actor Name)", ToolTip="마지막 비활성화 사유가 충돌일 때 맞은 Actor 이름입니다. 충돌이 아니면 None입니다."))
	FString GetLastHitActorName() const { return LastHitActorName; }

	// [v1.3.0] 마지막으로 비활성화될 때 적용되어 있던 ProjectileData를 반환합니다.
	UCFProjectileData* GetLastDeactivatedProjectileData() const { return LastDeactivatedProjectileData; }

	// [v1.3.0] 마지막 충돌에서 맞은 Actor를 반환합니다.
	AActor* GetLastHitActor() const { return LastHitActor; }

	// [v1.4.0] 마지막 충돌에서 실제로 맞은 컴포넌트 이름을 반환합니다.
	FName GetLastHitComponentName() const { return LastHitComponentName; }

	// [v1.3.0] 마지막 충돌 월드 위치를 반환합니다.
	FVector GetLastImpactLocation() const { return LastImpactLocation; }

	// [v1.3.0] 마지막 충돌 표면 노멀을 반환합니다.
	FVector GetLastImpactNormal() const { return LastImpactNormal; }

	// [v1.3.0] 마지막 충돌 입사 방향을 반환합니다.
		FVector GetLastIncomingDirection() const { return LastIncomingDirection; }

	// [v1.6.0] 마지막 충돌에서 생성된 Damage HitContext가 존재하는지 반환합니다.
	bool HasLastDamageHitContext() const { return bHasLastDamageHitContext; }

	// [v1.6.0] 마지막 충돌에서 생성된 Damage HitContext를 반환합니다.
	const FCFDamageHitContext& GetLastDamageHitContext() const { return LastDamageHitContext; }

	// [v1.6.0] 마지막 충돌에서 실행한 직접 피해 적용 결과를 반환합니다.
	const FCFDamageApplyResult& GetLastDamageApplyResult() const { return LastDamageApplyResult; }

	// [v1.3.0] 마지막으로 비활성화될 때의 발사 주체를 반환합니다.
	AActor* GetLastInstigatorActor() const { return LastDeactivatedInstigatorActor; }

	// [v1.1.0] 이 발사체를 재사용 Pool로 반환할 소유 컴포넌트를 지정합니다.
	void SetProjectilePoolOwner(UCFProjectilePoolComp* InProjectilePoolComp);

	// [v1.1.0] Pool 미사용 경로에서 비활성화 시 Destroy할지 여부를 지정합니다.
	void SetDestroyWhenDeactivated(bool bInDestroyWhenDeactivated);

protected:
	// [v1.0.0] BeginPlay 시 충돌 이벤트 바인딩과 비활성 상태를 보장합니다.
	virtual void BeginPlay() override;

	// [v1.5.0] ProjectileMovement 이후 보조 연속 Sphere Sweep을 수행합니다.
	virtual void Tick(float DeltaSeconds) override;

private:
	// [v1.0.0] 충돌 컴포넌트가 Blocking Hit을 감지했을 때 단일 Impact 처리 함수로 전달합니다.
	UFUNCTION()
	void HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	// [v1.5.0] OnComponentHit과 보조 Sweep이 공유하는 단일 Impact 처리 진입점입니다.
	bool ResolveProjectileImpact(AActor* InHitActor, UPrimitiveComponent* InHitComponent, const FHitResult& InHitResult);

	// [v1.5.0] 이전 충돌 위치부터 현재 위치까지 첫 Blocking Hit을 Sphere Sweep으로 검사합니다.
	void PerformSupplementalContinuousSweep();

	// [v1.2.0] 수명 종료 타이머가 끝났을 때 발사체를 비활성화합니다.
	void HandleProjectileLifeExpired();

	// [v1.2.0] 사유와 충돌 대상을 기록한 뒤 발사체 이동과 충돌을 멈춥니다.
	void DeactivateProjectileWithReason(ECFProjectileDeactivateReason InDeactivateReason, const FString& InHitActorName);

	// [v1.0.0] ProjectileData의 표시 메시 설정을 MeshComponent에 적용합니다.
	void ApplyProjectileVisual(const UCFProjectileData& InProjectileData);

	// [v1.5.0] ProjectileData의 충돌 반경과 CCD 설정을 CollisionComponent에 적용합니다.
	void ApplyProjectileCollision(const UCFProjectileData& InProjectileData);

	// [v1.5.0] ProjectileData의 이동과 연속 충돌 설정을 ProjectileMovementComponent에 적용합니다.
	void ApplyProjectileMovement(const UCFProjectileData& InProjectileData, const FVector& InLaunchDirection);

	// [v1.0.0] ProjectileData의 수명 설정에 따라 자동 비활성화 타이머를 예약합니다.
	void ScheduleProjectileLifeTimer(const UCFProjectileData& InProjectileData);

	// [v1.0.0] 풀링 전환 전 기본 비활성화 정책에 따라 Actor를 제거합니다.
	void FinishDeactivatePolicy();

	// [v1.0.0] 발사체 충돌 판정을 담당하는 루트 SphereComponent입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile", meta=(AllowPrivateAccess="true", DisplayName="충돌 컴포넌트 (CollisionComponent)", ToolTip="발사체 충돌 판정을 담당하는 루트 SphereComponent입니다."))
	TObjectPtr<USphereComponent> CollisionComponent = nullptr;

	// [v1.0.0] 발사체 시각 메시를 표시하는 StaticMeshComponent입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile", meta=(AllowPrivateAccess="true", DisplayName="메시 컴포넌트 (MeshComponent)", ToolTip="ProjectileData의 메시를 표시하는 StaticMeshComponent입니다. 충돌은 담당하지 않습니다."))
	TObjectPtr<UStaticMeshComponent> MeshComponent = nullptr;

	// [v1.0.0] 발사체 이동을 처리하는 ProjectileMovementComponent입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile", meta=(AllowPrivateAccess="true", DisplayName="Projectile 이동 컴포넌트 (ProjectileMovementComponent)", ToolTip="ProjectileData의 속도와 중력 설정을 적용받아 발사체 이동을 처리합니다."))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	// [v1.0.0] 현재 활성 발사체에 적용된 ProjectileData입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile", meta=(AllowPrivateAccess="true", DisplayName="활성 ProjectileData (ActiveProjectileData)", ToolTip="현재 활성 발사체에 적용된 ProjectileData입니다."))
	TObjectPtr<UCFProjectileData> ActiveProjectileData = nullptr;

	// [v1.0.0] 현재 발사체를 발사한 Actor입니다.
	UPROPERTY(Transient)
	TObjectPtr<AActor> ActiveInstigatorActor = nullptr;

	// [v1.1.0] 비활성화된 발사체를 반환할 Pool 소유 컴포넌트입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFProjectilePoolComp> ProjectilePoolOwnerComp = nullptr;

	// [v1.0.0] 현재 발사체가 활성 이동 중인지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile", meta=(AllowPrivateAccess="true", DisplayName="발사체 활성 여부 (bProjectileActive)", ToolTip="현재 발사체가 발사되어 이동 중인지 여부입니다."))
	bool bProjectileActive = false;

	// [v1.5.0] 이번 활성화에서 Impact 처리가 이미 완료됐는지 여부입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="Impact 처리 완료 여부 (bImpactResolvedThisActivation)", ToolTip="현재 활성화에서 첫 충돌이 이미 처리됐는지 표시합니다. OnComponentHit과 보조 Sweep의 중복 처리를 막습니다."))
	bool bImpactResolvedThisActivation = false;

	// [v1.5.0] 보조 연속 Sphere Sweep의 시작점으로 사용할 이전 충돌 컴포넌트 위치입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="이전 충돌 위치 (PreviousCollisionLocation)", ToolTip="보조 연속 Sphere Sweep이 다음 프레임에 검사할 시작 위치입니다. Pool 재활성화마다 현재 위치로 초기화됩니다."))
	FVector PreviousCollisionLocation = FVector::ZeroVector;

	// [v1.1.0] Pool 소유자가 없을 때 비활성화 시 Actor를 Destroy할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Pooling", meta=(AllowPrivateAccess="true", DisplayName="비활성화 시 Destroy (bDestroyWhenDeactivated)", ToolTip="True이면 수명 종료나 충돌 시 Actor를 Destroy합니다. Pool 소유자가 지정되면 이 값보다 Pool 반환을 우선합니다."))
	bool bDestroyWhenDeactivated = true;

	// [v1.0.0] 발사체 수명 종료를 처리하는 타이머 핸들입니다.
	FTimerHandle ProjectileLifeTimerHandle;

	// [v1.2.0] 마지막 활성화 월드 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 활성화 시간 (LastActivationTimeSeconds)", ToolTip="마지막으로 발사체가 활성화된 월드 시간입니다."))
	float LastActivationTimeSeconds = -1.0f;

	// [v1.2.0] 마지막 비활성화 월드 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 비활성화 시간 (LastDeactivateTimeSeconds)", ToolTip="마지막으로 발사체가 비활성화된 월드 시간입니다."))
	float LastDeactivateTimeSeconds = -1.0f;

	// [v1.2.0] 마지막 비행 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 비행 시간 (LastFlightDurationSeconds)", ToolTip="마지막 활성화 시점부터 비활성화 시점까지의 비행 시간입니다."))
	float LastFlightDurationSeconds = 0.0f;

	// [v1.2.0] 마지막으로 비활성화된 ProjectileData ID입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 비활성화 발사체 ID (LastDeactivatedProjectileId)", ToolTip="마지막으로 비활성화될 때 적용되어 있던 ProjectileData의 ProjectileId입니다."))
	FName LastDeactivatedProjectileId = NAME_None;

	// [v1.2.0] 마지막 비활성화 사유입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 비활성화 사유 (LastDeactivateReason)", ToolTip="마지막으로 발사체가 비활성화된 사유입니다."))
	ECFProjectileDeactivateReason LastDeactivateReason = ECFProjectileDeactivateReason::None;

	// [v1.2.0] 마지막 충돌 대상 Actor 이름입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 충돌 Actor 이름 (LastHitActorName)", ToolTip="마지막 비활성화 사유가 충돌일 때 맞은 Actor 이름입니다."))
	FString LastHitActorName = TEXT("None");

	// [v1.3.0] 마지막으로 비활성화될 때 적용되어 있던 ProjectileData입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 비활성화 ProjectileData (LastDeactivatedProjectileData)", ToolTip="마지막 비활성화 시점의 ProjectileData입니다. Damage HitContext Debug 생성에 사용합니다."))
	TObjectPtr<UCFProjectileData> LastDeactivatedProjectileData = nullptr;

	// [v1.3.0] 마지막으로 비활성화될 때의 발사 주체 Actor입니다.
	UPROPERTY(Transient)
	TObjectPtr<AActor> LastDeactivatedInstigatorActor = nullptr;

	// [v1.3.0] 마지막 충돌에서 맞은 Actor입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 충돌 Actor (LastHitActor)", ToolTip="마지막 Projectile Actor 충돌에서 맞은 Actor입니다."))
	TObjectPtr<AActor> LastHitActor = nullptr;

	// [v1.4.0] 마지막 충돌에서 실제로 맞은 컴포넌트 이름입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 충돌 컴포넌트 이름 (LastHitComponentName)", ToolTip="마지막 Projectile Actor 충돌에서 실제로 맞은 컴포넌트 이름입니다. 차량 차체면 보통 SM_Body입니다."))
	FName LastHitComponentName = NAME_None;

	// [v1.3.0] 마지막 충돌 월드 위치입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 충돌 위치 (LastImpactLocation)", ToolTip="마지막 Projectile Actor 충돌의 월드 위치입니다."))
	FVector LastImpactLocation = FVector::ZeroVector;

	// [v1.3.0] 마지막 충돌 표면 노멀입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 충돌 노멀 (LastImpactNormal)", ToolTip="마지막 Projectile Actor 충돌의 표면 노멀입니다."))
	FVector LastImpactNormal = FVector::UpVector;

	// [v1.3.0] 마지막 충돌 입사 방향입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 입사 방향 (LastIncomingDirection)", ToolTip="마지막 Projectile Actor 충돌에서 발사체가 진행하던 월드 방향입니다."))
		FVector LastIncomingDirection = FVector::ForwardVector;

	// [v1.6.0] 마지막 충돌에서 Damage HitContext를 생성했는지 여부입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Damage", meta=(AllowPrivateAccess="true", DisplayName="마지막 Damage HitContext 존재 여부 (bHasLastDamageHitContext)", ToolTip="마지막 첫 유효 Impact에서 Damage HitContext가 생성됐는지 여부입니다."))
	bool bHasLastDamageHitContext = false;

	// [v1.6.0] 마지막 충돌에서 생성한 공용 Damage HitContext입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Damage", meta=(AllowPrivateAccess="true", DisplayName="마지막 Damage HitContext (LastDamageHitContext)", ToolTip="마지막 첫 유효 Impact에서 생성해 직접 피해와 VehicleDebug가 공유하는 HitContext입니다."))
	FCFDamageHitContext LastDamageHitContext;

	// [v1.6.0] 마지막 충돌에서 실행한 직접 피해 적용 결과입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Damage", meta=(AllowPrivateAccess="true", DisplayName="마지막 피해 적용 결과 (LastDamageApplyResult)", ToolTip="마지막 첫 유효 Impact에서 VehicleHealthComp에 직접 피해 적용을 시도한 결과입니다."))
	FCFDamageApplyResult LastDamageApplyResult;
};
