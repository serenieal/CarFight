// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-07-01
// Description: CarFight 발사체 Actor Pool 컴포넌트
// Scope: ProjectileData가 지정한 Projectile Actor Class별로 발사체를 재사용해 반복 Spawn / Destroy 부담을 줄입니다.
// Changelog:
// - v1.3.0: Projectile Actor가 Hit으로 반환될 때 소유 Pawn에 Damage HitContext Debug 기록을 전달.
// - v1.2.0: 마지막 Projectile 반환 사유 / 대상 / 비행 시간 디버그 요약 getter 추가.
// - v1.1.0: Pool Debug 표시용 활성 발사체 수 getter와 유효 Actor 기준 카운트 계산 추가.
// - v1.0.0: 클래스별 발사체 Pool 버킷, Acquire / Release 생명주기, 클래스별 최대 생성 수 제한 추가.
// Migration:
// - ACFVehiclePawn은 직접 SpawnActor 대신 이 컴포넌트의 AcquireProjectile을 사용한다.
// - Pool 한도 초과 또는 Pool 확보 실패 시 기존 Dummy HitScan fallback을 유지한다.
// - 마지막 반환 요약은 검증 표시 전용이며 발사 가능 여부를 판정하지 않는다.
// - HitContext 기록은 Debug 전용이며 실제 Damage 적용은 하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFProjectileActor.h"
#include "Components/ActorComponent.h"
#include "CFProjectilePoolComp.generated.h"

class UCFProjectileData;

/**
 * Projectile Actor Class별로 재사용 가능한 발사체 목록을 보관하는 Pool 버킷입니다.
 */
USTRUCT(BlueprintType)
struct FCFProjectilePoolBucket
{
	GENERATED_BODY()

	// [v1.0.0] 이 버킷이 관리하는 Projectile Actor 클래스입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool", meta=(DisplayName="발사체 Actor 클래스 (ProjectileActorClass)", ToolTip="이 Pool 버킷이 관리하는 Projectile Actor 클래스입니다."))
	TSubclassOf<ACFProjectileActor> ProjectileActorClass = nullptr;

	// [v1.0.0] 이 버킷에서 생성해 추적 중인 전체 Projectile Actor 목록입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool", meta=(DisplayName="생성된 발사체 목록 (SpawnedProjectileArray)", ToolTip="Pool이 생성해 추적 중인 전체 발사체 Actor 목록입니다. 활성 / 비활성 Actor를 모두 포함합니다."))
	TArray<TObjectPtr<ACFProjectileActor>> SpawnedProjectileArray;

	// [v1.0.0] 현재 재사용 가능한 비활성 Projectile Actor 목록입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool", meta=(DisplayName="비활성 발사체 목록 (InactiveProjectileArray)", ToolTip="다음 발사에서 재사용할 수 있는 비활성 발사체 Actor 목록입니다."))
	TArray<TObjectPtr<ACFProjectileActor>> InactiveProjectileArray;
};

/**
 * 차량이 반복 발사하는 Projectile Actor를 재사용하기 위한 경량 Pool 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFProjectilePoolComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 발사체 Pool 컴포넌트의 Tick 비활성 기본값과 Pool 제한값을 초기화합니다.
	UCFProjectilePoolComp();

	// [v1.0.0] ProjectileData에 맞는 발사체 Actor를 Pool에서 확보하고 활성화합니다.
	ACFProjectileActor* AcquireProjectile(UCFProjectileData* InProjectileData, const FTransform& InSpawnTransform, const FVector& InLaunchDirection, AActor* InInstigatorActor);

	// [v1.0.0] 비활성화된 발사체 Actor를 Pool에 반환합니다.
	void ReleaseProjectile(ACFProjectileActor* InProjectileActor);

	// [v1.0.0] 현재 Pool이 추적 중인 전체 발사체 Actor 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectilePool", meta=(DisplayName="전체 풀 발사체 수 반환 (Get Total Pooled Projectile Count)", ToolTip="현재 Pool이 추적 중인 전체 발사체 Actor 수를 반환합니다. 활성 / 비활성 Actor를 모두 포함합니다."))
	int32 GetTotalPooledProjectileCount() const;

	// [v1.0.0] 현재 재사용 가능한 비활성 발사체 Actor 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectilePool", meta=(DisplayName="비활성 풀 발사체 수 반환 (Get Inactive Pooled Projectile Count)", ToolTip="현재 재사용 가능한 비활성 발사체 Actor 수를 반환합니다."))
	int32 GetInactivePooledProjectileCount() const;

	// [v1.1.0] 현재 발사되어 이동 중인 활성 발사체 Actor 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectilePool", meta=(DisplayName="활성 풀 발사체 수 반환 (Get Active Pooled Projectile Count)", ToolTip="현재 Pool이 추적 중이고 발사되어 이동 중인 활성 발사체 Actor 수를 반환합니다."))
	int32 GetActivePooledProjectileCount() const;

	// [v1.2.0] 마지막으로 Pool에 반환된 발사체의 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|ProjectilePool", meta=(DisplayName="마지막 발사체 반환 요약 반환 (Get Last Projectile Release Summary)", ToolTip="마지막으로 Pool에 반환된 발사체의 ID, 비활성화 사유, 충돌 대상, 비행 시간을 요약해 반환합니다."))
	FString GetLastProjectileReleaseSummary() const;

protected:
	// [v1.0.0] 컴포넌트 종료 시 Pool이 추적하던 발사체 Actor를 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// [v1.0.0] 지정 클래스에 해당하는 기존 Pool 버킷을 찾습니다.
	FCFProjectilePoolBucket* FindPoolBucket(UClass* InProjectileActorClass);

	// [v1.0.0] 지정 클래스에 해당하는 Pool 버킷을 찾거나 새로 추가합니다.
	FCFProjectilePoolBucket& FindOrAddPoolBucket(UClass* InProjectileActorClass);

	// [v1.0.0] 버킷에 남아 있는 무효 Actor 참조를 제거합니다.
	void CompactPoolBucket(FCFProjectilePoolBucket& InOutPoolBucket);

	// [v1.0.0] 버킷에서 재사용 가능한 비활성 Projectile Actor를 하나 꺼냅니다.
	ACFProjectileActor* PopInactiveProjectile(FCFProjectilePoolBucket& InOutPoolBucket);

	// [v1.0.0] 버킷이 새 Projectile Actor를 더 생성할 수 있는지 반환합니다.
	bool CanSpawnMoreProjectiles(const FCFProjectilePoolBucket& InPoolBucket) const;

	// [v1.0.0] 지정 클래스의 Projectile Actor를 Pool 소유로 새로 생성합니다.
	ACFProjectileActor* SpawnProjectileForPool(UClass* InProjectileActorClass, const FTransform& InSpawnTransform, AActor* InInstigatorActor);

	// [v1.0.0] Pool이 추적하는 모든 Projectile Actor를 Destroy하고 버킷을 비웁니다.
	void DestroyAllPooledProjectiles();

	// [v1.0.0] Projectile Actor 클래스별 Pool 버킷 목록입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool", meta=(AllowPrivateAccess="true", DisplayName="발사체 Pool 버킷 (ProjectilePoolBuckets)", ToolTip="Projectile Actor 클래스별로 나눈 Pool 버킷 목록입니다."))
	TArray<FCFProjectilePoolBucket> ProjectilePoolBuckets;

	// [v1.0.0] 하나의 Projectile Actor 클래스가 Pool에 유지할 수 있는 최대 Actor 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|ProjectilePool", meta=(AllowPrivateAccess="true", ClampMin="0", DisplayName="클래스별 최대 풀 수 (MaxPooledProjectileCountPerClass)", ToolTip="Projectile Actor 클래스 하나가 동시에 유지할 수 있는 최대 Actor 수입니다. 0이면 제한하지 않습니다. 한도 초과 시 Pawn은 Dummy HitScan fallback을 사용할 수 있습니다."))
	int32 MaxPooledProjectileCountPerClass = 128;

	// [v1.2.0] 마지막으로 Pool에 반환된 발사체 ID입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 반환 발사체 ID (LastReleasedProjectileId)", ToolTip="마지막으로 Pool에 반환된 발사체의 ProjectileId입니다."))
	FName LastReleasedProjectileId = NAME_None;

	// [v1.2.0] 마지막으로 Pool에 반환된 발사체 Actor 이름입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 반환 Actor 이름 (LastReleasedProjectileActorName)", ToolTip="마지막으로 Pool에 반환된 발사체 Actor 이름입니다."))
	FString LastReleasedProjectileActorName = TEXT("None");

	// [v1.2.0] 마지막 발사체 반환 사유입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 반환 사유 (LastReleaseReason)", ToolTip="마지막으로 Pool에 반환된 발사체의 비활성화 사유입니다."))
	ECFProjectileDeactivateReason LastReleaseReason = ECFProjectileDeactivateReason::None;

	// [v1.2.0] 마지막 발사체가 충돌한 Actor 이름입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 반환 충돌 Actor 이름 (LastReleaseHitActorName)", ToolTip="마지막 반환 사유가 충돌일 때 맞은 Actor 이름입니다."))
	FString LastReleaseHitActorName = TEXT("None");

	// [v1.2.0] 마지막으로 Pool에 반환된 발사체의 비행 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|ProjectilePool|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 반환 비행 시간 (LastReleaseFlightDurationSeconds)", ToolTip="마지막으로 Pool에 반환된 발사체의 활성화부터 비활성화까지 걸린 시간입니다."))
	float LastReleaseFlightDurationSeconds = 0.0f;
};
