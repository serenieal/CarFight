// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-07-01
// Description: CarFight 공통 발사체 Actor 구현
// Scope: ProjectileData 기반 발사체 표시 / 충돌 / 이동 / 수명 처리와 풀링 전환용 생명주기를 제공합니다.
// Changelog:
// - v1.3.0: 마지막 충돌 Transform, ProjectileData, 발사 주체를 보존해 Damage HitContext Debug 생성에 사용.
// - v1.2.0: 충돌 / 수명 / 수동 / 활성화 실패 비활성화 사유와 비행 시간 디버그 기록 구현.
// - v1.1.0: ProjectilePoolComp 소유 반환 경로와 Destroy 정책 setter 구현.
// - v1.0.0: ProjectileData 기반 메시 / 충돌 / 이동 / 수명 적용과 Activate / Deactivate 생명주기 구현.
// Migration:
// - Pool을 쓰지 않는 경우 수명 종료나 충돌 시 Destroy한다.
// - ProjectilePoolComp가 소유자로 지정한 Actor는 비활성화 시 Destroy하지 않고 Pool에 반환한다.
// - 기존 DeactivateProjectile 함수는 유지하고 내부에서 Manual 사유로 기록한다.
// - 마지막 충돌 정보는 Debug 전용이며 실제 Damage 적용은 하지 않는다.

#include "CFProjectileActor.h"

#include "CFProjectileData.h"
#include "CFProjectilePoolComp.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"

// [v1.0.0] 기본 발사체 Actor 컴포넌트와 비활성 상태를 초기화합니다.
ACFProjectileActor::ACFProjectileActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	// [v1.0.0] 발사체 충돌 판정에 사용할 루트 컴포넌트입니다.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(8.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetGenerateOverlapEvents(false);

	// [v1.0.0] ProjectileData의 StaticMesh를 표시할 시각 컴포넌트입니다.
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);

	// [v1.0.0] ProjectileData의 이동값을 적용받을 ProjectileMovement 컴포넌트입니다.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->bAutoActivate = false;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
	ProjectileMovementComponent->InitialSpeed = 6000.0f;
	ProjectileMovementComponent->MaxSpeed = 6000.0f;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

// [v1.0.0] BeginPlay 시 충돌 이벤트 바인딩과 비활성 상태를 보장합니다.
void ACFProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &ACFProjectileActor::HandleProjectileHit);
	}

	if (!bProjectileActive)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

// [v1.0.0] ProjectileData와 발사 방향을 적용해 발사체를 활성화합니다.
void ACFProjectileActor::ActivateProjectile(UCFProjectileData* InProjectileData, const FVector& InLaunchDirection, AActor* InInstigatorActor)
{
	if (!InProjectileData || !CollisionComponent || !ProjectileMovementComponent)
	{
		DeactivateProjectileWithReason(ECFProjectileDeactivateReason::InvalidActivation, TEXT("ActivationInputInvalid"));
		return;
	}

	// [v1.0.0] 이전 발사에서 무시하던 발사 주체입니다.
	AActor* PreviousInstigatorActor = ActiveInstigatorActor.Get();
	if (PreviousInstigatorActor && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(PreviousInstigatorActor, false);
	}

	// [v1.0.0] 이번 발사체에 적용할 ProjectileData입니다.
	ActiveProjectileData = InProjectileData;

	// [v1.2.0] 활성화 시간 기록에 사용할 현재 월드입니다.
	UWorld* World = GetWorld();
	LastActivationTimeSeconds = World ? World->GetTimeSeconds() : -1.0f;
	LastDeactivateTimeSeconds = -1.0f;
	LastFlightDurationSeconds = 0.0f;
	LastDeactivatedProjectileId = NAME_None;
	LastDeactivatedProjectileData = nullptr;
	LastDeactivatedInstigatorActor = nullptr;
	LastDeactivateReason = ECFProjectileDeactivateReason::None;
	LastHitActorName = TEXT("None");
	LastHitActor = nullptr;
	LastImpactLocation = FVector::ZeroVector;
	LastImpactNormal = FVector::UpVector;

	// [v1.0.0] 이번 발사체를 발사한 Actor입니다.
	ActiveInstigatorActor = InInstigatorActor;
	if (ActiveInstigatorActor && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(ActiveInstigatorActor, true);
	}

	// [v1.0.0] NaN과 0 벡터를 방지한 최종 발사 방향입니다.
	FVector SafeLaunchDirection = InLaunchDirection.GetSafeNormal();
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = GetActorForwardVector().GetSafeNormal();
	}
	if (SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = FVector::ForwardVector;
	}
	LastIncomingDirection = SafeLaunchDirection;

	SetActorRotation(SafeLaunchDirection.Rotation());
	ApplyProjectileVisual(*ActiveProjectileData);
	ApplyProjectileCollision(*ActiveProjectileData);
	ApplyProjectileMovement(*ActiveProjectileData, SafeLaunchDirection);
	ScheduleProjectileLifeTimer(*ActiveProjectileData);

	bProjectileActive = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

// [v1.0.0] 발사체 이동과 충돌을 멈추고 비활성화합니다.
void ACFProjectileActor::DeactivateProjectile()
{
	DeactivateProjectileWithReason(ECFProjectileDeactivateReason::Manual, TEXT("None"));
}

// [v1.2.0] 사유와 충돌 대상을 기록한 뒤 발사체 이동과 충돌을 멈춥니다.
void ACFProjectileActor::DeactivateProjectileWithReason(const ECFProjectileDeactivateReason InDeactivateReason, const FString& InHitActorName)
{
	// [v1.0.0] 수명 종료 타이머를 정리할 월드입니다.
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ProjectileLifeTimerHandle);
	}

	// [v1.2.0] 현재 비활성화 월드 시간입니다.
	const float CurrentDeactivateTimeSeconds = World ? World->GetTimeSeconds() : -1.0f;

	// [v1.2.0] 비활성화 직전에 적용되어 있던 ProjectileData ID입니다.
	LastDeactivatedProjectileId = ActiveProjectileData ? ActiveProjectileData->ProjectileId : NAME_None;
	LastDeactivatedProjectileData = ActiveProjectileData;
	LastDeactivatedInstigatorActor = ActiveInstigatorActor;
	LastDeactivateReason = InDeactivateReason;
	LastHitActorName = InHitActorName.IsEmpty() ? TEXT("None") : InHitActorName;
	LastDeactivateTimeSeconds = CurrentDeactivateTimeSeconds;
	LastFlightDurationSeconds = (LastActivationTimeSeconds >= 0.0f && CurrentDeactivateTimeSeconds >= 0.0f)
		? FMath::Max(CurrentDeactivateTimeSeconds - LastActivationTimeSeconds, 0.0f)
		: 0.0f;

	if (InDeactivateReason != ECFProjectileDeactivateReason::Hit)
	{
		LastHitActor = nullptr;
		LastImpactLocation = GetActorLocation();
		LastImpactNormal = FVector::UpVector;
		LastIncomingDirection = GetActorForwardVector().GetSafeNormal();
		if (LastIncomingDirection.IsNearlyZero())
		{
			LastIncomingDirection = FVector::ForwardVector;
		}
	}

	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
	}

	if (CollisionComponent)
	{
		if (ActiveInstigatorActor)
		{
			CollisionComponent->IgnoreActorWhenMoving(ActiveInstigatorActor, false);
		}

		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	bProjectileActive = false;
	ActiveProjectileData = nullptr;
	ActiveInstigatorActor = nullptr;

	FinishDeactivatePolicy();
}

// [v1.1.0] 이 발사체를 재사용 Pool로 반환할 소유 컴포넌트를 지정합니다.
void ACFProjectileActor::SetProjectilePoolOwner(UCFProjectilePoolComp* InProjectilePoolComp)
{
	ProjectilePoolOwnerComp = InProjectilePoolComp;
}

// [v1.1.0] Pool 미사용 경로에서 비활성화 시 Destroy할지 여부를 지정합니다.
void ACFProjectileActor::SetDestroyWhenDeactivated(const bool bInDestroyWhenDeactivated)
{
	bDestroyWhenDeactivated = bInDestroyWhenDeactivated;
}

// [v1.0.0] 충돌 컴포넌트가 Blocking Hit을 감지했을 때 발사체를 종료합니다.
void ACFProjectileActor::HandleProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!bProjectileActive)
	{
		return;
	}

	if (OtherActor && OtherActor == ActiveInstigatorActor)
	{
		return;
	}

	// [v1.2.0] 충돌로 맞은 Actor 이름입니다.
	const FString HitActorName = OtherActor ? OtherActor->GetName() : TEXT("None");

	LastHitActor = OtherActor;
	LastImpactLocation = Hit.ImpactPoint;
	LastImpactNormal = Hit.ImpactNormal.GetSafeNormal();
	if (LastImpactNormal.IsNearlyZero())
	{
		LastImpactNormal = FVector::UpVector;
	}

	LastIncomingDirection = ProjectileMovementComponent ? ProjectileMovementComponent->Velocity.GetSafeNormal() : GetActorForwardVector().GetSafeNormal();
	if (LastIncomingDirection.IsNearlyZero())
	{
		LastIncomingDirection = GetActorForwardVector().GetSafeNormal();
	}
	if (LastIncomingDirection.IsNearlyZero())
	{
		LastIncomingDirection = FVector::ForwardVector;
	}

	DeactivateProjectileWithReason(ECFProjectileDeactivateReason::Hit, HitActorName);
}

// [v1.2.0] 수명 종료 타이머가 끝났을 때 발사체를 비활성화합니다.
void ACFProjectileActor::HandleProjectileLifeExpired()
{
	DeactivateProjectileWithReason(ECFProjectileDeactivateReason::LifeExpired, TEXT("None"));
}

// [v1.0.0] ProjectileData의 표시 메시 설정을 MeshComponent에 적용합니다.
void ACFProjectileActor::ApplyProjectileVisual(const UCFProjectileData& InProjectileData)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetStaticMesh(InProjectileData.ProjectileStaticMesh.Get());
	MeshComponent->SetRelativeLocation(FVector::ZeroVector);
	MeshComponent->SetRelativeRotation(InProjectileData.ProjectileMeshRelativeRotation);
	MeshComponent->SetRelativeScale3D(InProjectileData.ProjectileMeshRelativeScale);
}

// [v1.0.0] ProjectileData의 충돌 반경 설정을 CollisionComponent에 적용합니다.
void ACFProjectileActor::ApplyProjectileCollision(const UCFProjectileData& InProjectileData)
{
	if (!CollisionComponent)
	{
		return;
	}

	// [v1.0.0] 발사체 충돌에 사용할 최소 보정 반경입니다.
	const float SafeCollisionRadius = FMath::Max(InProjectileData.CollisionRadius, 1.0f);

	CollisionComponent->SetSphereRadius(SafeCollisionRadius, true);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

// [v1.0.0] ProjectileData의 이동 설정과 발사 방향을 ProjectileMovementComponent에 적용합니다.
void ACFProjectileActor::ApplyProjectileMovement(const UCFProjectileData& InProjectileData, const FVector& InLaunchDirection)
{
	if (!ProjectileMovementComponent || !CollisionComponent)
	{
		return;
	}

	// [v1.0.0] 발사체 이동에 사용할 최소 보정 속도입니다.
	const float SafeInitialSpeed = FMath::Max(InProjectileData.InitialSpeed, 1.0f);

	// [v1.0.0] 발사체에 적용할 중력 배율입니다.
	const float ProjectileGravityScale = InProjectileData.bAffectedByGravity ? FMath::Max(InProjectileData.GravityScale, 0.0f) : 0.0f;

	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = SafeInitialSpeed;
	ProjectileMovementComponent->MaxSpeed = SafeInitialSpeed;
	ProjectileMovementComponent->ProjectileGravityScale = ProjectileGravityScale;
	ProjectileMovementComponent->Velocity = InLaunchDirection.GetSafeNormal() * SafeInitialSpeed;
	ProjectileMovementComponent->Activate(true);
}

// [v1.0.0] ProjectileData의 수명 설정에 따라 자동 비활성화 타이머를 예약합니다.
void ACFProjectileActor::ScheduleProjectileLifeTimer(const UCFProjectileData& InProjectileData)
{
	// [v1.0.0] 수명 종료 타이머를 예약할 월드입니다.
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(ProjectileLifeTimerHandle);

	// [v1.0.0] 발사체가 월드에 남아 있을 최대 시간입니다.
	const float SafeLifeTimeSeconds = FMath::Max(InProjectileData.LifeTimeSeconds, 0.01f);

	World->GetTimerManager().SetTimer(
		ProjectileLifeTimerHandle,
		this,
		&ACFProjectileActor::HandleProjectileLifeExpired,
		SafeLifeTimeSeconds,
		false);
}

// [v1.1.0] Pool 소유자가 있으면 반환하고, 없으면 기본 비활성화 정책에 따라 Actor를 제거합니다.
void ACFProjectileActor::FinishDeactivatePolicy()
{
	if (IsValid(ProjectilePoolOwnerComp.Get()))
	{
		ProjectilePoolOwnerComp->ReleaseProjectile(this);
		return;
	}

	if (bDestroyWhenDeactivated && !IsActorBeingDestroyed())
	{
		Destroy();
	}
}
