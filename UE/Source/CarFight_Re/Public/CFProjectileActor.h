// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.0
// Date: 2026-08-02
// Description: CarFight 공통 발사체 Actor
// Scope: ProjectileData 기반 추진·미사일 비행·유도·이동·풀링·지속형 비행 FX·고속 연속 충돌, 동일 발사 차량 격리, Projectile 요격과 정식 차량 방어 결과를 보존합니다.
// Changelog:
// - v1.13.0: MissileFlightComp·MissileGuideComp, Direct Target Snapshot Guidance, 현재 방향 추진과 Pool Reset 연결을 추가.
// - v1.12.0: 첫 유효 Impact를 VehicleDefenseComp 정식 진입점으로 전환하고 전체 방어 결과와 기존 Integrity 결과를 함께 보존.
// - v1.11.0: Projectile 채널 Block을 복구하고 동일 발사 차량의 모든 탄종·Volley만 양방향 Ignore하며 다른 차량 Projectile 요격과 Intercepted 생명주기를 추가.
// - v1.10.0: Projectile 상호 충돌을 Collision Response와 Impact Guard 양쪽에서 무시해 Salvo 사출 직후 자폭을 방지한 임시 구현. v1.11.0에서 차량별 격리로 교체.
// - v1.9.0: Launch Context 기반 활성화, Legacy Adapter, 초기 월드 Velocity 적용과 Pool Reset용 Launch Context 상태 추가.
// - v1.8.1: 비행 FX RelativeTransform Scale을 유효 소켓과 Fallback 경로에 동일 적용하고 실제 Scale Debug 추가.
// - v1.8.0: ProjectileMotorComp, 실제 점화·연소 상태와 Thruster FX 동기화, 추진 최대 속도 적용 추가.
// - v1.7.0: Trail·Thruster NiagaraComponent, 메시 소켓/Fallback 부착과 Pool 반환 전 FX Reset 생명주기 추가.
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
// - Trail과 추진 화염은 ProjectileData에서 선택적으로 지정하며, 비어 있거나 비활성이어도 이동·충돌·피해는 유지된다.
// - Pool 반환 전에 두 NiagaraComponent를 즉시 정지·Reset하고 Asset 참조를 비워 다음 활성화의 상태 오염을 막는다.
// - 비행 FX RelativeTransform의 Scale은 소켓 위치를 사용할 때도 독립 FX Scale로 적용되며 위치·회전만 소켓이 우선한다.
// - 추진 화염은 Projectile 활성 시간 전체가 아니라 ProjectileMotorComp의 Burning 상태에서만 재생한다.
// - 추진이 비활성인 기존 ProjectileData는 InitialSpeed와 기존 MaxSpeed 동작을 그대로 유지한다.
// - 기존 ActivateProjectile API는 Direct Launch Context를 생성하는 호환 Adapter로 유지한다.
// - Context 기반 활성화는 발사 위치·초기 방향·초기 월드 Velocity를 복사하며 비활성화 시 Context를 초기화한다.
// - Projectile 채널은 기본 Block이며 서로 다른 발사 차량의 Projectile은 실제 Sweep·Hit으로 충돌할 수 있다.
// - 같은 차량이 발사한 모든 탄종과 모든 Volley는 ActiveInstigatorActor 동일성을 기준으로 양방향 Ignore한다.
// - 동일 차량 Ignore 관계는 Projectile Actor Class와 FireRequestId에 제한되지 않으며 Pool 반환 전에 양쪽 Actor에서 해제한다.
// - 요격 가능한 Projectile은 다른 차량 Projectile 또는 Hitscan 적중 한 번으로 Intercepted 처리하며 선택적으로 Impact FX를 요청한다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "CFDamageRuntimeTypes.h"
#include "CFProjectileMotorTypes.h"
#include "CFProjectileLaunchTypes.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "CFProjectileActor.generated.h"

class UCFMissileFlightComp;
class UCFMissileGuideComp;
class UCFProjectileData;
class UCFProjectileMotorComp;
class UCFProjectilePoolComp;
class UNiagaraComponent;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
struct FCFProjectileAttachedFxSettings;

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
	Intercepted UMETA(DisplayName="Intercepted"),
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

			// [v1.0.0] 기존 호출 호환을 위해 발사 방향에서 Direct Launch Context를 생성해 발사체를 활성화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Projectile", meta=(DisplayName="발사체 활성화 (Activate Projectile)", ToolTip="기존 호출 호환용 함수입니다. ProjectileData와 발사 방향으로 Direct Launch Context를 만든 뒤 발사체를 활성화합니다."))
	void ActivateProjectile(UCFProjectileData* InProjectileData, const FVector& InLaunchDirection, AActor* InInstigatorActor);

	// [v1.9.0] 발사 위치·초기 방향·초기 월드 속도를 포함한 Launch Context를 복사해 발사체를 활성화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Projectile", meta=(DisplayName="발사 컨텍스트로 발사체 활성화 (Activate Projectile With Context)", ToolTip="발사 순간의 월드 Transform, 초기 발사 방향, 초기 월드 속도와 명령 목표를 복사해 발사체를 활성화합니다. 활성화 뒤 런처 Transform을 다시 조회하지 않습니다."))
	void ActivateProjectileWithContext(UCFProjectileData* InProjectileData, const FCFProjectileLaunchContext& InLaunchContext, AActor* InInstigatorActor);

	// [v1.0.0] 발사체 이동과 충돌을 멈추고 비활성화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Projectile", meta=(DisplayName="발사체 비활성화 (Deactivate Projectile)", ToolTip="발사체 이동과 충돌을 멈추고 비활성화합니다. 기본값은 Destroy이며, Pool 사용 시 반환 경로로 바꿀 수 있습니다."))
	void DeactivateProjectile();

	// [v1.0.0] 현재 발사체가 활성 발사 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile", meta=(DisplayName="발사체 활성 여부 (Is Projectile Active)", ToolTip="현재 발사체가 발사되어 이동 중인지 반환합니다."))
	bool IsProjectileActive() const { return bProjectileActive; }

		// [v1.0.0] 현재 발사체에 적용된 ProjectileData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile", meta=(DisplayName="활성 ProjectileData 반환 (Get Active Projectile Data)", ToolTip="현재 발사체에 적용된 ProjectileData입니다. 비활성 상태에서는 None일 수 있습니다."))
		UCFProjectileData* GetActiveProjectileData() const { return ActiveProjectileData; }

	// [v1.11.0] 현재 발사체를 발사한 차량 또는 전투 Actor를 반환합니다.
	AActor* GetActiveInstigatorActor() const { return ActiveInstigatorActor; }

	// [v1.11.0] 지정 Projectile이 현재 동일 발사 차량 Ignore 목록에 등록됐는지 반환합니다.
	bool IsIgnoringSameSourceProjectile(const ACFProjectileActor* InProjectileActor) const;

	// [v1.11.0] 현재 동일 발사 차량 Ignore 목록의 유효 Projectile 수를 반환합니다.
	int32 GetIgnoredSameSourceProjectileCount() const;

	// [v1.11.0] 다른 차량 Projectile 또는 Hitscan 적중으로 현재 Projectile의 요격 종료를 시도합니다.
	bool TryResolveProjectileInterception(AActor* InInterceptorActor, AActor* InInterceptorSourceActor, const FHitResult& InHitResult);

	// [v1.9.0] 현재 활성화에 복사된 Launch Context가 존재하는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Launch", meta=(DisplayName="활성 발사 컨텍스트 존재 여부 (Has Active Launch Context)", ToolTip="현재 발사체가 유효한 Launch Context를 복사해 활성화된 상태인지 반환합니다. Pool 반환 뒤에는 False입니다."))
	bool HasActiveLaunchContext() const { return bHasActiveLaunchContext; }

	// [v1.9.0] 현재 활성화에 복사된 Launch Context를 값으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Launch", meta=(DisplayName="활성 발사 컨텍스트 반환 (Get Active Launch Context)", ToolTip="현재 발사체가 독립적으로 보유하는 발사 순간 Transform, 방향, 속도, 명령 목표와 요청 식별값을 반환합니다."))
	FCFProjectileLaunchContext GetActiveLaunchContext() const { return ActiveLaunchContext; }

	// [v1.9.0] 현재 Launch Context의 모드, 요청 ID, 방향과 속도를 표시하는 요약을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Launch", meta=(DisplayName="발사 컨텍스트 요약 생성 (Build Projectile Launch Summary)", ToolTip="현재 Launch Context 존재 여부, Release Mode, FireRequestId, WeaponGroupId, 초기 방향·속도와 명령 목표를 문자열로 반환합니다."))
	FString BuildProjectileLaunchSummary() const;

				// [v1.8.1] 현재 Trail과 추진 화염의 활성 상태, 실제 부착 출처와 적용 Scale을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|FlightFx", meta=(DisplayName="비행 FX 요약 생성 (Build Projectile Flight FX Summary)", ToolTip="현재 Trail과 추진 화염의 활성 상태, Niagara 연결 상태, 메시 소켓 또는 Fallback 부착 출처와 실제 적용 Scale을 표시합니다."))
	FString BuildProjectileFlightFxSummary() const;

	// [v1.8.0] 현재 점화·연소·관성 비행 상태와 추진 속도를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Propulsion", meta=(DisplayName="추진 모터 요약 생성 (Build Projectile Motor Summary)", ToolTip="현재 발사체의 모터 상태, 점화·연소 시간, 속도, 추진 방향과 활성화 횟수를 표시합니다."))
	FString BuildProjectileMotorSummary() const;

		// [v1.8.0] 이 발사체가 소유하는 추진 모터 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Propulsion", meta=(DisplayName="추진 모터 컴포넌트 반환 (Get Projectile Motor Component)", ToolTip="점화 지연과 실제 추진 가속을 소유하는 ProjectileMotorComp입니다."))
	UCFProjectileMotorComp* GetProjectileMotorComponent() const { return ProjectileMotorComponent; }

	// [v1.13.0] 이 발사체가 소유하는 미사일 비행 상태 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Missile", meta=(DisplayName="미사일 비행 컴포넌트 반환 (Get Missile Flight Component)", ToolTip="Released, Clearance와 GuidedFlight 상태를 소유하는 MissileFlightComp입니다. 기존 Projectile에서도 컴포넌트는 존재하지만 설정 기본값은 비활성입니다."))
	UCFMissileFlightComp* GetMissileFlightComponent() const { return MissileFlightComponent; }

	// [v1.13.0] 이 발사체가 소유하는 물리 제한형 유도 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Projectile|Missile", meta=(DisplayName="미사일 유도 컴포넌트 반환 (Get Missile Guide Component)", ToolTip="발사 순간 Target Snapshot과 제한형 Guidance Command를 소유하는 MissileGuideComp입니다. 기존 Projectile에서는 설정 기본값이 비활성입니다."))
	UCFMissileGuideComp* GetMissileGuideComponent() const { return MissileGuideComponent; }


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

		// [v1.12.0] 마지막 충돌에서 정식 차량 방어 피해 결과가 생성됐는지 반환합니다.
	bool HasLastVehicleDamageResult() const { return bHasLastVehicleDamageResult; }

	// [v1.12.0] 마지막 충돌에서 생성한 쉴드·장갑·내구도 전체 피해 결과를 반환합니다.
	const FCFVehicleDamageResult& GetLastVehicleDamageResult() const { return LastVehicleDamageResult; }

	// [v1.6.0] 마지막 충돌에서 실행한 기존 차량 내구도 호환 결과를 반환합니다.
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
	friend class UCFProjectilePoolComp;

	// [v1.0.0] 충돌 컴포넌트가 Blocking Hit을 감지했을 때 단일 Impact 처리 함수로 전달합니다.
	UFUNCTION()
	void HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

		// [v1.5.0] OnComponentHit과 보조 Sweep이 공유하는 단일 Impact 처리 진입점입니다.
	bool ResolveProjectileImpact(AActor* InHitActor, UPrimitiveComponent* InHitComponent, const FHitResult& InHitResult);

	// [v1.11.0] 같은 발사 차량 Projectile인지 확인해 충돌 무시 정책을 결정합니다.
	bool ShouldIgnoreProjectileCollision(const ACFProjectileActor* InOtherProjectileActor) const;

	// [v1.11.0] 같은 발사 차량 Projectile 한 개를 현재 이동·보조 Sweep Ignore 목록에 추가합니다.
	void AddSameSourceProjectileIgnore(ACFProjectileActor* InProjectileActor);

	// [v1.11.0] 같은 발사 차량 Projectile 한 개를 현재 Ignore 목록에서 제거합니다.
	void RemoveSameSourceProjectileIgnore(ACFProjectileActor* InProjectileActor);

	// [v1.11.0] Pool 반환 전에 동일 발사 차량 Projectile Ignore 관계를 양방향으로 모두 해제합니다.
	void ClearSameSourceProjectileIgnores();

	// [v1.11.0] 요격 불가 Projectile이 충돌 이벤트 뒤에도 기존 Velocity로 계속 비행하도록 Movement를 복구합니다.
	void RestoreMovementAfterProjectileCollision(const FVector& InPreCollisionVelocity);

	// [v1.5.0] 이전 충돌 위치부터 현재 위치까지 첫 Blocking Hit을 Sphere Sweep으로 검사합니다.
	void PerformSupplementalContinuousSweep();

	// [v1.2.0] 수명 종료 타이머가 끝났을 때 발사체를 비활성화합니다.
	void HandleProjectileLifeExpired();

	// [v1.2.0] 사유와 충돌 대상을 기록한 뒤 발사체 이동과 충돌을 멈춥니다.
	void DeactivateProjectileWithReason(ECFProjectileDeactivateReason InDeactivateReason, const FString& InHitActorName);

	// [v1.8.0] 추진 모터 상태가 변경됐을 때 실제 Thruster Niagara 활성 상태를 동기화합니다.
	UFUNCTION()
	void HandleProjectileMotorStateChanged(ECFProjectileMotorState PreviousMotorState, ECFProjectileMotorState NewMotorState);

	// [v1.0.0] ProjectileData의 표시 메시 설정을 MeshComponent에 적용합니다.
	void ApplyProjectileVisual(const UCFProjectileData& InProjectileData);

	// [v1.7.0] 이전 활성화의 Trail·추진 화염 상태와 Asset 참조를 안전하게 초기화합니다.
	void ResetProjectileFlightFx();

	// [v1.7.0] ProjectileData의 Trail·추진 화염 설정을 독립 NiagaraComponent에 적용합니다.
	void ApplyProjectileFlightFx(const UCFProjectileData& InProjectileData);

	// [v1.7.0] 한 비행 FX 슬롯의 부착 기준, Niagara와 Debug 상태를 적용합니다.
	void ApplyProjectileAttachedFx(
		const FCFProjectileAttachedFxSettings& InFxSettings,
		USceneComponent* InFxOriginComponent,
		UNiagaraComponent* InNiagaraComponent,
		FString& OutFxStatus,
		FString& OutAttachmentSource,
		bool bActivateImmediately);

	// [v1.8.0] 현재 모터 상태를 기준으로 추진 화염 Niagara를 시작·정지하고 Debug 상태를 갱신합니다.
	void RefreshThrusterFxFromMotorState();

	// [v1.8.0] 준비된 Thruster Niagara 자산을 실제 연소 표현으로 활성화합니다.
	void ActivatePreparedThrusterFx();

	// [v1.8.0] 추진 화염을 정지하되 Pool 전체 Reset 전까지 Asset과 부착 위치는 유지합니다.
	void DeactivateThrusterFxForMotorState(const FString& InMotorFxStatus);

	// [v1.7.0] Pool 반환 또는 Destroy 전에 Trail·추진 화염을 즉시 정지하고 초기화합니다.
	void DeactivateProjectileFlightFx();

	// [v1.5.0] ProjectileData의 충돌 반경과 CCD 설정을 CollisionComponent에 적용합니다.
	void ApplyProjectileCollision(const UCFProjectileData& InProjectileData);

		// [v1.9.0] ProjectileData의 이동 설정과 Launch Context의 초기 월드 Velocity를 ProjectileMovementComponent에 적용합니다.
	void ApplyProjectileMovement(const UCFProjectileData& InProjectileData, const FVector& InLaunchDirection, const FVector& InInitialLaunchVelocity);

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

		// [v1.7.0] Trail의 메시 소켓 또는 Projectile Fallback Transform을 적용할 독립 원점입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx", meta=(AllowPrivateAccess="true", DisplayName="Trail 원점 컴포넌트 (TrailOriginComponent)", ToolTip="Trail Niagara가 사용할 메시 소켓 또는 Projectile Relative Fallback 위치·회전과 메시 Scale에서 분리된 독립 FX Scale을 소유합니다."))
	TObjectPtr<USceneComponent> TrailOriginComponent = nullptr;

	// [v1.7.0] Projectile 활성 시간 동안 Trail을 재생하고 Pool 반환 전에 Reset되는 NiagaraComponent입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx", meta=(AllowPrivateAccess="true", DisplayName="Trail Niagara 컴포넌트 (TrailNiagaraComponent)", ToolTip="ProjectileData의 Trail FX를 재생합니다. 발사마다 생성하지 않고 Projectile Actor와 함께 Pool에서 재사용합니다."))
	TObjectPtr<UNiagaraComponent> TrailNiagaraComponent = nullptr;

		// [v1.7.0] 추진 화염의 메시 소켓 또는 Projectile Fallback Transform을 적용할 독립 원점입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx", meta=(AllowPrivateAccess="true", DisplayName="추진 FX 원점 컴포넌트 (ThrusterOriginComponent)", ToolTip="추진 화염 Niagara가 사용할 메시 소켓 또는 Projectile Relative Fallback 위치·회전과 메시 Scale에서 분리된 독립 FX Scale을 소유합니다."))
	TObjectPtr<USceneComponent> ThrusterOriginComponent = nullptr;

	// [v1.8.0] 실제 모터 Burning 상태에서 추진 화염을 재생하고 Pool 반환 전에 Reset되는 NiagaraComponent입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx", meta=(AllowPrivateAccess="true", DisplayName="추진 Niagara 컴포넌트 (ThrusterNiagaraComponent)", ToolTip="ProjectileData의 추진 화염 FX를 모터 Burning 상태에서 재생합니다. 발사마다 생성하지 않고 Projectile Actor와 함께 Pool에서 재사용합니다."))
	TObjectPtr<UNiagaraComponent> ThrusterNiagaraComponent = nullptr;

	// [v1.0.0] 발사체 이동을 처리하는 ProjectileMovementComponent입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile", meta=(AllowPrivateAccess="true", DisplayName="Projectile 이동 컴포넌트 (ProjectileMovementComponent)", ToolTip="ProjectileData의 초기 속도, 최대 속도와 중력 설정을 적용받아 발사체 이동을 처리합니다."))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

		// [v1.8.0] 점화 지연, 실제 추진 가속과 연소 종료 상태를 관리하는 모터 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Propulsion", meta=(AllowPrivateAccess="true", DisplayName="Projectile 모터 컴포넌트 (ProjectileMotorComponent)", ToolTip="ProjectileData.PropulsionConfig를 읽어 Rocket 고정 방향 또는 Missile 현재 방향 추진과 BurnedOut 관성 비행을 관리합니다."))
	TObjectPtr<UCFProjectileMotorComp> ProjectileMotorComponent = nullptr;

	// [v1.13.0] MissileFlightConfig의 상태 진행과 Guidance 활성 시점을 관리하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Missile", meta=(AllowPrivateAccess="true", DisplayName="미사일 비행 컴포넌트 (MissileFlightComponent)", ToolTip="발사 순간 Context를 복사하고 Released, Clearance, GuidedFlight 상태를 관리합니다. bUseMissileFlight=false이면 Inactive 상태입니다."))
	TObjectPtr<UCFMissileFlightComp> MissileFlightComponent = nullptr;

	// [v1.13.0] MissileGuideConfig와 발사 순간 Target Snapshot으로 물리 제한형 유도를 적용하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Projectile|Missile", meta=(AllowPrivateAccess="true", DisplayName="미사일 유도 컴포넌트 (MissileGuideComponent)", ToolTip="FlightComp의 유도 구간에서 제한형 비례항법을 계산하고 ProjectileMovement Velocity 방향을 갱신합니다. bUseGuidance=false이면 비활성입니다."))
	TObjectPtr<UCFMissileGuideComp> MissileGuideComponent = nullptr;

		// [v1.0.0] 현재 활성 발사체에 적용된 ProjectileData입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile", meta=(AllowPrivateAccess="true", DisplayName="활성 ProjectileData (ActiveProjectileData)", ToolTip="현재 활성 발사체에 적용된 ProjectileData입니다."))
	TObjectPtr<UCFProjectileData> ActiveProjectileData = nullptr;

	// [v1.9.0] 현재 활성화에서 발사체가 독립적으로 복사해 보유하는 발사 순간 데이터입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(AllowPrivateAccess="true", DisplayName="활성 발사 컨텍스트 (ActiveLaunchContext)", ToolTip="발사 순간의 월드 Transform, 초기 방향·속도, 명령 목표와 요청 식별값의 복사본입니다. 발사 후 런처 상태 변경과 독립적입니다."))
	FCFProjectileLaunchContext ActiveLaunchContext;

	// [v1.9.0] 현재 활성 발사체에 유효한 Launch Context가 복사됐는지 여부입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Launch", meta=(AllowPrivateAccess="true", DisplayName="활성 발사 컨텍스트 존재 여부 (bHasActiveLaunchContext)", ToolTip="True이면 현재 활성화가 Launch Context를 보유합니다. 비활성화와 Pool 반환 전에 False로 초기화됩니다."))
	bool bHasActiveLaunchContext = false;

	// [v1.7.0] 현재 Trail 슬롯의 비활성, 누락 또는 활성 상태입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx|Debug", meta=(AllowPrivateAccess="true", DisplayName="Trail FX 상태 (TrailFlightFxStatus)", ToolTip="현재 Trail 슬롯이 Inactive, Disabled, MissingSystem, Active 또는 DedicatedServerSkipped 중 어떤 상태인지 표시합니다."))
	FString TrailFlightFxStatus = TEXT("Inactive");

	// [v1.7.0] 현재 Trail 원점이 사용한 메시 소켓 또는 Projectile Fallback 출처입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx|Debug", meta=(AllowPrivateAccess="true", DisplayName="Trail 부착 출처 (TrailFlightFxAttachmentSource)", ToolTip="Trail이 MeshSocket, ProjectileRelative 또는 MissingSocketFallback 중 어느 기준으로 배치됐는지 표시합니다."))
	FString TrailFlightFxAttachmentSource = TEXT("None");

	// [v1.7.0] 현재 추진 화염 슬롯의 비활성, 누락 또는 활성 상태입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx|Debug", meta=(AllowPrivateAccess="true", DisplayName="추진 FX 상태 (ThrusterFlightFxStatus)", ToolTip="현재 추진 화염 슬롯이 Inactive, Disabled, MissingSystem, Active 또는 DedicatedServerSkipped 중 어떤 상태인지 표시합니다."))
	FString ThrusterFlightFxStatus = TEXT("Inactive");

	// [v1.7.0] 현재 추진 화염 원점이 사용한 메시 소켓 또는 Projectile Fallback 출처입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|FlightFx|Debug", meta=(AllowPrivateAccess="true", DisplayName="추진 FX 부착 출처 (ThrusterFlightFxAttachmentSource)", ToolTip="추진 화염이 MeshSocket, ProjectileRelative 또는 MissingSocketFallback 중 어느 기준으로 배치됐는지 표시합니다."))
	FString ThrusterFlightFxAttachmentSource = TEXT("None");

		// [v1.0.0] 현재 발사체를 발사한 Actor입니다.
	UPROPERTY(Transient)
	TObjectPtr<AActor> ActiveInstigatorActor = nullptr;

	// [v1.11.0] 같은 발사 차량에서 나온 모든 탄종·Volley 중 현재 상호 충돌을 무시할 Projectile 목록입니다.
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ACFProjectileActor>> IgnoredSameSourceProjectileArray;

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

		// [v1.12.0] 마지막 충돌에서 정식 차량 방어 피해 결과가 생성됐는지 여부입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Damage", meta=(AllowPrivateAccess="true", DisplayName="마지막 차량 방어 결과 존재 여부 (bHasLastVehicleDamageResult)", ToolTip="마지막 첫 유효 Impact에서 VehicleDefenseComp 정식 진입점이 전체 피해 결과를 생성했는지 여부입니다."))
	bool bHasLastVehicleDamageResult = false;

	// [v1.12.0] 마지막 충돌에서 생성한 쉴드·장갑·내구도 전체 피해 결과입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Damage", meta=(AllowPrivateAccess="true", DisplayName="마지막 차량 방어 결과 (LastVehicleDamageResult)", ToolTip="마지막 첫 유효 Impact에서 VehicleDefenseComp가 계산한 쉴드, 장갑, 관통과 차량 내구도 전체 결과입니다."))
	FCFVehicleDamageResult LastVehicleDamageResult;

	// [v1.6.0] 마지막 충돌에서 실행한 기존 차량 내구도 호환 결과입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Projectile|Damage", meta=(AllowPrivateAccess="true", DisplayName="마지막 내구도 적용 결과 (LastDamageApplyResult)", ToolTip="기존 VehicleDebug와 Projectile Pool 복사 호환을 위해 전체 방어 결과에서 변환한 차량 내구도 적용 결과입니다."))
	FCFDamageApplyResult LastDamageApplyResult;
};
