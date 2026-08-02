// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-07-30
// Description: CarFight 발사체 Actor Pool 컴포넌트 구현
// Scope: ProjectileData 기반 발사체 Actor 재사용과 동일 발사 차량의 모든 탄종·Volley 충돌 격리를 관리합니다.
// Changelog:
// - v1.5.0: 모든 Actor Class 버킷의 동일 발사 차량 활성 Projectile을 양방향 Ignore로 등록하고 Hitscan Query 제외를 추가.
// - v1.4.0: Launch Context 기반 Acquire 경로와 기존 SpawnTransform·LaunchDirection 호출용 Direct Adapter 구현.
// - v1.3.0: Hit으로 반환된 Projectile Actor의 Damage HitContext Debug 기록을 소유 Pawn에 전달.
// - v1.2.0: 마지막 Projectile 반환 사유 / 대상 / 비행 시간 디버그 요약 구현.
// - v1.1.0: Pool Debug 표시용 활성 발사체 수 getter와 유효 Actor 기준 카운트 계산 구현.
// - v1.0.0: 클래스별 발사체 Pool 버킷, Acquire / Release 생명주기, 클래스별 최대 생성 수 제한 구현.
// Migration:
// - ACFVehiclePawn은 직접 SpawnActor 대신 이 컴포넌트의 AcquireProjectile을 사용한다.
// - Pool 한도 초과 또는 Pool 확보 실패 시 기존 Dummy HitScan fallback을 유지한다.
// - 마지막 반환 요약은 검증 표시 전용이며 발사 가능 여부를 판정하지 않는다.
// - HitContext 기록은 Debug 전용이며 실제 Damage 적용은 하지 않는다.
// - 기존 AcquireProjectile API는 Direct Launch Context를 만들어 신규 Context 경로로 전달한다.
// - Context의 LaunchTransform은 Actor 생성·재사용 배치와 활성화에 같은 값으로 사용한다.

#include "CFProjectilePoolComp.h"

#include "CFProjectileData.h"
#include "CFVehiclePawn.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"

namespace
{
	// [v1.2.0] 발사체 비활성화 사유 enum 값을 표시 문자열로 변환합니다.
	FString BuildProjectileDeactivateReasonText(const ECFProjectileDeactivateReason InDeactivateReason)
	{
		// [v1.2.0] 발사체 비활성화 사유 enum 타입 정보입니다.
		const UEnum* DeactivateReasonEnum = StaticEnum<ECFProjectileDeactivateReason>();
		if (!DeactivateReasonEnum)
		{
			return TEXT("Unknown");
		}

		return DeactivateReasonEnum->GetNameStringByValue(static_cast<int64>(InDeactivateReason));
	}
}

// [v1.0.0] 발사체 Pool 컴포넌트의 Tick 비활성 기본값과 Pool 제한값을 초기화합니다.
UCFProjectilePoolComp::UCFProjectilePoolComp()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxPooledProjectileCountPerClass = 128;
}

// [v1.4.0] 기존 호출 호환을 위해 SpawnTransform과 발사 방향에서 Direct Context를 만들어 발사체를 확보합니다.
ACFProjectileActor* UCFProjectilePoolComp::AcquireProjectile(
	UCFProjectileData* InProjectileData,
	const FTransform& InSpawnTransform,
	const FVector& InLaunchDirection,
	AActor* InInstigatorActor)
{
	// [v1.4.0] 기존 발사 방향에서 NaN과 0 벡터를 제거한 Direct 발사 방향입니다.
	FVector SafeLaunchDirection = InLaunchDirection.GetSafeNormal();
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = InSpawnTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	}
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = FVector::ForwardVector;
	}

	// [v1.4.0] 기존 ProjectileData InitialSpeed를 유지할 안전한 초기 속력입니다.
	const float SafeInitialSpeed = InProjectileData
		? FMath::Max(InProjectileData->InitialSpeed, 1.0f)
		: 1.0f;

	// [v1.4.0] 기존 Acquire 호출을 신규 Context 경로로 전달할 Direct 발사 데이터입니다.
	FCFProjectileLaunchContext LegacyLaunchContext;
	LegacyLaunchContext.LaunchTransform = InSpawnTransform.ContainsNaN()
		? FTransform(SafeLaunchDirection.Rotation(), FVector::ZeroVector)
		: InSpawnTransform;
	LegacyLaunchContext.LaunchTransform.SetRotation(SafeLaunchDirection.Rotation().Quaternion());
	LegacyLaunchContext.InitialLaunchDirection = SafeLaunchDirection;
	LegacyLaunchContext.InitialLaunchVelocity = SafeLaunchDirection * SafeInitialSpeed;
	LegacyLaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;

	return AcquireProjectileWithContext(InProjectileData, LegacyLaunchContext, InInstigatorActor);
}

// [v1.4.0] Launch Context의 Transform과 초기 월드 Velocity를 사용해 발사체를 Pool에서 확보하고 활성화합니다.
ACFProjectileActor* UCFProjectilePoolComp::AcquireProjectileWithContext(
	UCFProjectileData* InProjectileData,
	const FCFProjectileLaunchContext& InLaunchContext,
	AActor* InInstigatorActor)
{
	if (!InProjectileData || !InProjectileData->ProjectileActorClass)
	{
		return nullptr;
	}

	// [v1.0.0] 이번 발사에서 사용할 Projectile Actor 클래스입니다.
	UClass* ProjectileActorClass = InProjectileData->ProjectileActorClass.Get();
	if (!ProjectileActorClass)
	{
		return nullptr;
	}

	// [v1.4.0] Spawn과 Actor 활성화에 공통으로 사용할 NaN 방지 Launch Transform입니다.
	const FTransform SafeLaunchTransform = InLaunchContext.LaunchTransform.ContainsNaN()
		? FTransform::Identity
		: InLaunchContext.LaunchTransform;

	// [v1.4.0] 안전한 Transform을 반영해 Actor에 전달할 Launch Context 복사본입니다.
	FCFProjectileLaunchContext SafeLaunchContext = InLaunchContext;
	SafeLaunchContext.LaunchTransform = SafeLaunchTransform;

	// [v1.0.0] 이번 Projectile Actor 클래스에 대응하는 Pool 버킷입니다.
	FCFProjectilePoolBucket& PoolBucket = FindOrAddPoolBucket(ProjectileActorClass);
	CompactPoolBucket(PoolBucket);

	// [v1.0.0] Pool에서 재사용할 수 있는 비활성 Projectile Actor입니다.
	ACFProjectileActor* ProjectileActor = PopInactiveProjectile(PoolBucket);
	if (!ProjectileActor)
	{
		if (!CanSpawnMoreProjectiles(PoolBucket))
		{
			return nullptr;
		}

		ProjectileActor = SpawnProjectileForPool(ProjectileActorClass, SafeLaunchTransform, InInstigatorActor);
		if (!ProjectileActor)
		{
			return nullptr;
		}

		PoolBucket.SpawnedProjectileArray.Add(ProjectileActor);
	}

	ProjectileActor->SetProjectilePoolOwner(this);
	ProjectileActor->SetDestroyWhenDeactivated(false);
	ProjectileActor->SetActorTransform(SafeLaunchTransform, false, nullptr, ETeleportType::TeleportPhysics);
	ProjectileActor->ActivateProjectileWithContext(InProjectileData, SafeLaunchContext, InInstigatorActor);

	return ProjectileActor->IsProjectileActive() ? ProjectileActor : nullptr;
}

// [v1.0.0] 비활성화된 발사체 Actor를 Pool에 반환합니다.
void UCFProjectilePoolComp::ReleaseProjectile(ACFProjectileActor* InProjectileActor)
{
	if (!IsValid(InProjectileActor))
	{
		return;
	}

	// [v1.0.0] 반환되는 발사체의 실제 Actor 클래스입니다.
	UClass* ProjectileActorClass = InProjectileActor->GetClass();
	if (!ProjectileActorClass)
	{
		return;
	}

	// [v1.0.0] 반환 대상 Projectile Actor 클래스의 Pool 버킷입니다.
	FCFProjectilePoolBucket& PoolBucket = FindOrAddPoolBucket(ProjectileActorClass);
	CompactPoolBucket(PoolBucket);

	LastReleasedProjectileId = InProjectileActor->GetLastDeactivatedProjectileId();
	LastReleasedProjectileActorName = InProjectileActor->GetName();
	LastReleaseReason = InProjectileActor->GetLastDeactivateReason();
	LastReleaseHitActorName = InProjectileActor->GetLastHitActorName();
	LastReleaseFlightDurationSeconds = InProjectileActor->GetLastFlightDurationSeconds();

		if (LastReleaseReason == ECFProjectileDeactivateReason::Hit
		|| LastReleaseReason == ECFProjectileDeactivateReason::Intercepted)
	{
		// [v1.3.0] Projectile Pool을 소유한 차량 Pawn입니다.
		ACFVehiclePawn* OwnerVehiclePawn = Cast<ACFVehiclePawn>(GetOwner());
		if (OwnerVehiclePawn)
		{
			OwnerVehiclePawn->RecordProjectileDamageHitContextFromPool(InProjectileActor);
		}
	}

	if (!PoolBucket.SpawnedProjectileArray.Contains(InProjectileActor))
	{
		PoolBucket.SpawnedProjectileArray.Add(InProjectileActor);
	}

	if (!PoolBucket.InactiveProjectileArray.Contains(InProjectileActor))
	{
		PoolBucket.InactiveProjectileArray.Add(InProjectileActor);
	}
}

// [v1.5.0] 새 Projectile을 같은 발사 차량의 모든 활성 탄종·Volley와 양방향 Ignore로 등록합니다.
void UCFProjectilePoolComp::RegisterSameSourceProjectileIsolation(
	ACFProjectileActor* InProjectileActor,
	AActor* InSourceActor)
{
	if (!IsValid(InProjectileActor) || !IsValid(InSourceActor))
	{
		return;
	}

	for (FCFProjectilePoolBucket& PoolBucket : ProjectilePoolBuckets)
	{
		CompactPoolBucket(PoolBucket);
		for (const TObjectPtr<ACFProjectileActor>& ExistingProjectileActorPtr : PoolBucket.SpawnedProjectileArray)
		{
			ACFProjectileActor* ExistingProjectileActor = ExistingProjectileActorPtr.Get();
			if (!IsValid(ExistingProjectileActor)
				|| ExistingProjectileActor == InProjectileActor
				|| !ExistingProjectileActor->IsProjectileActive()
				|| ExistingProjectileActor->GetActiveInstigatorActor() != InSourceActor)
			{
				continue;
			}

			InProjectileActor->AddSameSourceProjectileIgnore(ExistingProjectileActor);
			ExistingProjectileActor->AddSameSourceProjectileIgnore(InProjectileActor);
		}
	}
}

// [v1.5.0] Hitscan Trace가 같은 발사 차량의 현재 활성 Projectile을 건너뛰도록 QueryParams에 추가합니다.
void UCFProjectilePoolComp::AddActiveSourceProjectilesToQueryParams(
	AActor* InSourceActor,
	FCollisionQueryParams& InOutQueryParams) const
{
	if (!IsValid(InSourceActor))
	{
		return;
	}

	for (const FCFProjectilePoolBucket& PoolBucket : ProjectilePoolBuckets)
	{
		for (const TObjectPtr<ACFProjectileActor>& ProjectileActorPtr : PoolBucket.SpawnedProjectileArray)
		{
			ACFProjectileActor* ProjectileActor = ProjectileActorPtr.Get();
			if (IsValid(ProjectileActor)
				&& ProjectileActor->IsProjectileActive()
				&& ProjectileActor->GetActiveInstigatorActor() == InSourceActor)
			{
				InOutQueryParams.AddIgnoredActor(ProjectileActor);
			}
		}
	}
}

// [v1.0.0] 현재 Pool이 추적 중인 전체 발사체 Actor 수를 반환합니다.
int32 UCFProjectilePoolComp::GetTotalPooledProjectileCount() const
{
	// [v1.0.0] 전체 Pool 버킷에서 합산한 추적 Actor 수입니다.
	int32 TotalPooledProjectileCount = 0;

	// [v1.0.0] 현재 검사 중인 Pool 버킷 인덱스입니다.
	for (int32 BucketIndex = 0; BucketIndex < ProjectilePoolBuckets.Num(); ++BucketIndex)
	{
		// [v1.1.0] 현재 검사 중인 Pool 버킷입니다.
		const FCFProjectilePoolBucket& PoolBucket = ProjectilePoolBuckets[BucketIndex];

		// [v1.1.0] 현재 검사 중인 생성 발사체 인덱스입니다.
		for (int32 ProjectileIndex = 0; ProjectileIndex < PoolBucket.SpawnedProjectileArray.Num(); ++ProjectileIndex)
		{
			if (IsValid(PoolBucket.SpawnedProjectileArray[ProjectileIndex].Get()))
			{
				++TotalPooledProjectileCount;
			}
		}
	}

	return TotalPooledProjectileCount;
}

// [v1.0.0] 현재 재사용 가능한 비활성 발사체 Actor 수를 반환합니다.
int32 UCFProjectilePoolComp::GetInactivePooledProjectileCount() const
{
	// [v1.0.0] 전체 Pool 버킷에서 합산한 비활성 Actor 수입니다.
	int32 InactivePooledProjectileCount = 0;

	// [v1.0.0] 현재 검사 중인 Pool 버킷 인덱스입니다.
	for (int32 BucketIndex = 0; BucketIndex < ProjectilePoolBuckets.Num(); ++BucketIndex)
	{
		// [v1.1.0] 현재 검사 중인 Pool 버킷입니다.
		const FCFProjectilePoolBucket& PoolBucket = ProjectilePoolBuckets[BucketIndex];

		// [v1.1.0] 현재 검사 중인 비활성 발사체 인덱스입니다.
		for (int32 ProjectileIndex = 0; ProjectileIndex < PoolBucket.InactiveProjectileArray.Num(); ++ProjectileIndex)
		{
			if (IsValid(PoolBucket.InactiveProjectileArray[ProjectileIndex].Get()))
			{
				++InactivePooledProjectileCount;
			}
		}
	}

	return InactivePooledProjectileCount;
}

// [v1.1.0] 현재 발사되어 이동 중인 활성 발사체 Actor 수를 반환합니다.
int32 UCFProjectilePoolComp::GetActivePooledProjectileCount() const
{
	// [v1.1.0] 전체 Pool 버킷에서 합산한 활성 Actor 수입니다.
	int32 ActivePooledProjectileCount = 0;

	// [v1.1.0] 현재 검사 중인 Pool 버킷 인덱스입니다.
	for (int32 BucketIndex = 0; BucketIndex < ProjectilePoolBuckets.Num(); ++BucketIndex)
	{
		// [v1.1.0] 현재 검사 중인 Pool 버킷입니다.
		const FCFProjectilePoolBucket& PoolBucket = ProjectilePoolBuckets[BucketIndex];

		// [v1.1.0] 현재 검사 중인 생성 발사체 인덱스입니다.
		for (int32 ProjectileIndex = 0; ProjectileIndex < PoolBucket.SpawnedProjectileArray.Num(); ++ProjectileIndex)
		{
			// [v1.1.0] 현재 검사 중인 Projectile Actor입니다.
			const ACFProjectileActor* ProjectileActor = PoolBucket.SpawnedProjectileArray[ProjectileIndex].Get();
			if (IsValid(ProjectileActor) && ProjectileActor->IsProjectileActive())
			{
				++ActivePooledProjectileCount;
			}
		}
	}

	return ActivePooledProjectileCount;
}

// [v1.2.0] 마지막으로 Pool에 반환된 발사체의 요약 문자열을 반환합니다.
FString UCFProjectilePoolComp::GetLastProjectileReleaseSummary() const
{
	// [v1.2.0] 마지막 반환 사유를 표시할 문자열입니다.
	const FString ReleaseReasonText = BuildProjectileDeactivateReasonText(LastReleaseReason);

	return FString::Printf(
		TEXT("ProjectileRelease: Projectile=%s, Reason=%s, Actor=%s, HitActor=%s, Flight=%.2fs"),
		*LastReleasedProjectileId.ToString(),
		*ReleaseReasonText,
		*LastReleasedProjectileActorName,
		*LastReleaseHitActorName,
		LastReleaseFlightDurationSeconds);
}

// [v1.0.0] 컴포넌트 종료 시 Pool이 추적하던 발사체 Actor를 정리합니다.
void UCFProjectilePoolComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyAllPooledProjectiles();
	Super::EndPlay(EndPlayReason);
}

// [v1.0.0] 지정 클래스에 해당하는 기존 Pool 버킷을 찾습니다.
FCFProjectilePoolBucket* UCFProjectilePoolComp::FindPoolBucket(UClass* InProjectileActorClass)
{
	if (!InProjectileActorClass)
	{
		return nullptr;
	}

	// [v1.0.0] 현재 검사 중인 Pool 버킷 인덱스입니다.
	for (int32 BucketIndex = 0; BucketIndex < ProjectilePoolBuckets.Num(); ++BucketIndex)
	{
		// [v1.0.0] 현재 검사 중인 Pool 버킷입니다.
		FCFProjectilePoolBucket& PoolBucket = ProjectilePoolBuckets[BucketIndex];
		if (PoolBucket.ProjectileActorClass.Get() == InProjectileActorClass)
		{
			return &PoolBucket;
		}
	}

	return nullptr;
}

// [v1.0.0] 지정 클래스에 해당하는 Pool 버킷을 찾거나 새로 추가합니다.
FCFProjectilePoolBucket& UCFProjectilePoolComp::FindOrAddPoolBucket(UClass* InProjectileActorClass)
{
	// [v1.0.0] 이미 존재하는 Projectile Actor 클래스별 Pool 버킷입니다.
	FCFProjectilePoolBucket* ExistingPoolBucket = FindPoolBucket(InProjectileActorClass);
	if (ExistingPoolBucket)
	{
		return *ExistingPoolBucket;
	}

	// [v1.0.0] 새로 추가된 Pool 버킷의 배열 인덱스입니다.
	const int32 AddedBucketIndex = ProjectilePoolBuckets.AddDefaulted();

	// [v1.0.0] 새로 추가된 Projectile Actor 클래스별 Pool 버킷입니다.
	FCFProjectilePoolBucket& AddedPoolBucket = ProjectilePoolBuckets[AddedBucketIndex];
	AddedPoolBucket.ProjectileActorClass = InProjectileActorClass;
	return AddedPoolBucket;
}

// [v1.0.0] 버킷에 남아 있는 무효 Actor 참조를 제거합니다.
void UCFProjectilePoolComp::CompactPoolBucket(FCFProjectilePoolBucket& InOutPoolBucket)
{
	InOutPoolBucket.SpawnedProjectileArray.RemoveAll(
		[](const TObjectPtr<ACFProjectileActor>& ProjectileActor)
		{
			return !IsValid(ProjectileActor.Get());
		});

	InOutPoolBucket.InactiveProjectileArray.RemoveAll(
		[](const TObjectPtr<ACFProjectileActor>& ProjectileActor)
		{
			return !IsValid(ProjectileActor.Get());
		});
}

// [v1.0.0] 버킷에서 재사용 가능한 비활성 Projectile Actor를 하나 꺼냅니다.
ACFProjectileActor* UCFProjectilePoolComp::PopInactiveProjectile(FCFProjectilePoolBucket& InOutPoolBucket)
{
	while (InOutPoolBucket.InactiveProjectileArray.Num() > 0)
	{
		// [v1.0.0] 마지막 비활성 Actor 항목의 인덱스입니다.
		const int32 LastInactiveIndex = InOutPoolBucket.InactiveProjectileArray.Num() - 1;

		// [v1.0.0] 이번에 재사용 후보로 꺼낼 Projectile Actor입니다.
		ACFProjectileActor* ProjectileActor = InOutPoolBucket.InactiveProjectileArray[LastInactiveIndex].Get();
		InOutPoolBucket.InactiveProjectileArray.RemoveAtSwap(LastInactiveIndex, 1, EAllowShrinking::No);

		if (IsValid(ProjectileActor))
		{
			return ProjectileActor;
		}
	}

	return nullptr;
}

// [v1.0.0] 버킷이 새 Projectile Actor를 더 생성할 수 있는지 반환합니다.
bool UCFProjectilePoolComp::CanSpawnMoreProjectiles(const FCFProjectilePoolBucket& InPoolBucket) const
{
	if (MaxPooledProjectileCountPerClass <= 0)
	{
		return true;
	}

	return InPoolBucket.SpawnedProjectileArray.Num() < MaxPooledProjectileCountPerClass;
}

// [v1.0.0] 지정 클래스의 Projectile Actor를 Pool 소유로 새로 생성합니다.
ACFProjectileActor* UCFProjectilePoolComp::SpawnProjectileForPool(UClass* InProjectileActorClass, const FTransform& InSpawnTransform, AActor* InInstigatorActor)
{
	if (!InProjectileActorClass)
	{
		return nullptr;
	}

	// [v1.0.0] Projectile Actor를 생성할 현재 월드입니다.
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// [v1.0.0] Pool 소유 Actor를 생성할 때 지정할 Spawn 파라미터입니다.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(InInstigatorActor);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// [v1.0.0] Pool이 새로 생성한 Projectile Actor입니다.
	ACFProjectileActor* SpawnedProjectileActor = World->SpawnActor<ACFProjectileActor>(
		InProjectileActorClass,
		InSpawnTransform,
		SpawnParameters);
	if (!SpawnedProjectileActor)
	{
		return nullptr;
	}

	SpawnedProjectileActor->SetProjectilePoolOwner(this);
	SpawnedProjectileActor->SetDestroyWhenDeactivated(false);
	return SpawnedProjectileActor;
}

// [v1.0.0] Pool이 추적하는 모든 Projectile Actor를 Destroy하고 버킷을 비웁니다.
void UCFProjectilePoolComp::DestroyAllPooledProjectiles()
{
	// [v1.0.0] 현재 정리 중인 Pool 버킷 인덱스입니다.
	for (int32 BucketIndex = 0; BucketIndex < ProjectilePoolBuckets.Num(); ++BucketIndex)
	{
		// [v1.0.0] 현재 정리 중인 Pool 버킷입니다.
		FCFProjectilePoolBucket& PoolBucket = ProjectilePoolBuckets[BucketIndex];

		// [v1.0.0] 현재 정리 중인 Projectile Actor 인덱스입니다.
		for (int32 ProjectileIndex = 0; ProjectileIndex < PoolBucket.SpawnedProjectileArray.Num(); ++ProjectileIndex)
		{
			// [v1.0.0] 현재 정리 중인 Projectile Actor입니다.
			ACFProjectileActor* ProjectileActor = PoolBucket.SpawnedProjectileArray[ProjectileIndex].Get();
			if (!IsValid(ProjectileActor))
			{
				continue;
			}

			ProjectileActor->SetProjectilePoolOwner(nullptr);
			ProjectileActor->SetDestroyWhenDeactivated(true);
			ProjectileActor->Destroy();
		}
	}

	ProjectilePoolBuckets.Reset();
}
