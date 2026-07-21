// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.6.0
// Date: 2026-07-14
// Description: CarFight 공통 발사체 Actor 구현
// Scope: ProjectileData 기반 발사체 표시 / 이동 / 풀링 / 고속 연속 충돌과 직접 피해 적용을 제공합니다.
// Changelog:
// - v1.6.0: 첫 유효 Impact에서 DamageHitContext 생성, BaseDamage 적용 결과 저장과 Pool 반환 전달을 구현.
// - v1.5.0: ProjectileMovement Sweep/Sub-step 설정, 보조 Sphere Sweep, 단일 Impact 처리와 Pool 상태 초기화를 구현.
// - v1.4.0: CollisionComponent Object Type을 Projectile 채널로 변경하고 마지막 피격 컴포넌트 이름을 기록.
// - v1.3.0: 마지막 충돌 Transform, ProjectileData, 발사 주체를 보존해 Damage HitContext Debug 생성에 사용.
// - v1.2.0: 충돌 / 수명 / 수동 / 활성화 실패 비활성화 사유와 비행 시간 디버그 기록 구현.
// - v1.1.0: ProjectilePoolComp 소유 반환 경로와 Destroy 정책 setter 구현.
// - v1.0.0: ProjectileData 기반 메시 / 충돌 / 이동 / 수명 적용과 Activate / Deactivate 생명주기 구현.
// Migration:
// - Pool을 쓰지 않는 경우 수명 종료나 충돌 시 Destroy한다.
// - ProjectilePoolComp가 소유자로 지정한 Actor는 비활성화 시 Destroy하지 않고 Pool에 반환한다.
// - 기존 DeactivateProjectile 함수는 유지하고 내부에서 Manual 사유로 기록한다.
// - 마지막 첫 유효 Impact에서 DamageData.BaseDamage를 VehicleHealthComp에 한 번 적용하고 결과를 Pool 반환 뒤까지 보존한다.
// - OnComponentHit과 보조 Sphere Sweep은 ResolveProjectileImpact에서 첫 유효 Impact만 처리한다.

#include "CFProjectileActor.h"

#include "CFCollisionChannels.h"
#include "CFDamageData.h"
#include "CFProjectileData.h"
#include "CFProjectilePoolComp.h"
#include "CFVehicleHealthComp.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"

// [v1.0.0] 기본 발사체 Actor 컴포넌트와 비활성 상태를 초기화합니다.
ACFProjectileActor::ACFProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	bReplicates = false;

	// [v1.0.0] 발사체 충돌 판정에 사용할 루트 컴포넌트입니다.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(8.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComponent->SetCollisionObjectType(CFCollisionChannels::Projectile);
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

	SetActorTickEnabled(false);

	if (!bProjectileActive)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

// [v1.5.0] ProjectileMovement 이후 보조 연속 Sphere Sweep을 수행합니다.
void ACFProjectileActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bProjectileActive || bImpactResolvedThisActivation || !ActiveProjectileData)
	{
		return;
	}

	if (!ActiveProjectileData->bUseSupplementalContinuousSweep)
	{
		return;
	}

	PerformSupplementalContinuousSweep();
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
	LastHitComponentName = NAME_None;
		LastImpactLocation = FVector::ZeroVector;
	LastImpactNormal = FVector::UpVector;
	bHasLastDamageHitContext = false;
	LastDamageHitContext = FCFDamageHitContext();
	LastDamageApplyResult = FCFDamageApplyResult();
	bImpactResolvedThisActivation = false;
	PreviousCollisionLocation = CollisionComponent->GetComponentLocation();

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
	SetActorTickEnabled(ActiveProjectileData->bUseSupplementalContinuousSweep);
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
		LastHitComponentName = NAME_None;
		LastImpactLocation = GetActorLocation();
		LastImpactNormal = FVector::UpVector;
		LastIncomingDirection = GetActorForwardVector().GetSafeNormal();
		if (LastIncomingDirection.IsNearlyZero())
		{
			LastIncomingDirection = FVector::ForwardVector;
		}
	}

	SetActorTickEnabled(false);

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

// [v1.0.0] 충돌 컴포넌트가 Blocking Hit을 감지했을 때 단일 Impact 처리 함수로 전달합니다.
void ACFProjectileActor::HandleProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	ResolveProjectileImpact(OtherActor, OtherComponent, Hit);
}

// [v1.5.0] OnComponentHit과 보조 Sweep이 공유하는 단일 Impact 처리 진입점입니다.
bool ACFProjectileActor::ResolveProjectileImpact(
	AActor* InHitActor,
	UPrimitiveComponent* InHitComponent,
	const FHitResult& InHitResult)
{
	if (!bProjectileActive || bImpactResolvedThisActivation)
	{
		return false;
	}

	// [v1.5.0] 발사체 Pool 또는 Spawn 경로에서 지정된 소유 Actor입니다.
	AActor* OwnerActor = GetOwner();
	if (InHitActor == this || InHitActor == ActiveInstigatorActor || (OwnerActor && InHitActor == OwnerActor))
	{
		return false;
	}

	bImpactResolvedThisActivation = true;

	// [v1.2.0] 충돌로 맞은 Actor 이름입니다.
	const FString HitActorName = InHitActor ? InHitActor->GetName() : TEXT("None");

	// [v1.5.0] 명시 인자가 없을 때 HitResult에서 복구한 실제 피격 컴포넌트입니다.
	UPrimitiveComponent* ResolvedHitComponent = InHitComponent ? InHitComponent : InHitResult.GetComponent();

	LastHitActor = InHitActor;
	LastHitComponentName = ResolvedHitComponent ? ResolvedHitComponent->GetFName() : NAME_None;
	LastImpactLocation = InHitResult.ImpactPoint;
	LastImpactNormal = InHitResult.ImpactNormal.GetSafeNormal();
	if (LastImpactNormal.IsNearlyZero())
	{
		LastImpactNormal = FVector::UpVector;
	}

	LastIncomingDirection = ProjectileMovementComponent ? ProjectileMovementComponent->Velocity.GetSafeNormal() : FVector::ZeroVector;
	if (LastIncomingDirection.IsNearlyZero())
	{
		LastIncomingDirection = (InHitResult.TraceEnd - InHitResult.TraceStart).GetSafeNormal();
	}
	if (LastIncomingDirection.IsNearlyZero())
	{
		LastIncomingDirection = GetActorForwardVector().GetSafeNormal();
	}
		if (LastIncomingDirection.IsNearlyZero())
	{
		LastIncomingDirection = FVector::ForwardVector;
	}

	// [v1.6.0] 이번 Impact에서 실제 직접 피해 데이터로 사용할 DamageData입니다.
	UCFDamageData* ResolvedDamageData = ActiveProjectileData ? ActiveProjectileData->DefaultDamageData.Get() : nullptr;

	// [v1.6.0] Projectile 첫 유효 Impact와 직접 피해 적용이 공유할 공용 명중 컨텍스트입니다.
	FCFDamageHitContext DamageHitContext;
	DamageHitContext.DamageData = ResolvedDamageData;
	DamageHitContext.DamageId = ResolvedDamageData
		? ResolvedDamageData->DamageId
		: (ActiveProjectileData ? ActiveProjectileData->DamageProfileId : NAME_None);
	DamageHitContext.WeaponId = NAME_None;
	DamageHitContext.ProjectileId = ActiveProjectileData ? ActiveProjectileData->ProjectileId : NAME_None;
	DamageHitContext.HitActor = InHitActor;
	DamageHitContext.HitComponentName = LastHitComponentName;
	DamageHitContext.ImpactLocation = LastImpactLocation;
	DamageHitContext.ImpactNormal = LastImpactNormal;
	DamageHitContext.IncomingDirection = LastIncomingDirection;
	DamageHitContext.InstigatorActor = ActiveInstigatorActor;

	// [v1.6.0] 이번 Impact 시점의 비행 시간을 계산할 월드입니다.
	UWorld* ImpactWorld = GetWorld();

	// [v1.6.0] 이번 Impact가 발생한 월드 시간입니다.
	const float ImpactTimeSeconds = ImpactWorld ? ImpactWorld->GetTimeSeconds() : -1.0f;
	DamageHitContext.FlightDurationSeconds = (LastActivationTimeSeconds >= 0.0f && ImpactTimeSeconds >= 0.0f)
		? FMath::Max(ImpactTimeSeconds - LastActivationTimeSeconds, 0.0f)
		: 0.0f;
	DamageHitContext.bFromProjectileActor = true;
	DamageHitContext.bBlockingHit = true;

	bHasLastDamageHitContext = true;
	LastDamageHitContext = DamageHitContext;
	LastDamageApplyResult = FCFDamageApplyResult();
	UCFVehicleHealthComp::TryApplyDamageToActor(LastDamageHitContext, LastDamageApplyResult);

	DeactivateProjectileWithReason(ECFProjectileDeactivateReason::Hit, HitActorName);
	return true;
}

// [v1.5.0] 이전 충돌 위치부터 현재 위치까지 첫 Blocking Hit을 Sphere Sweep으로 검사합니다.
void ACFProjectileActor::PerformSupplementalContinuousSweep()
{
	if (!CollisionComponent)
	{
		return;
	}

	// [v1.5.0] 보조 연속 Sweep을 실행할 현재 월드입니다.
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// [v1.5.0] ProjectileMovement 이후 충돌 컴포넌트의 현재 월드 위치입니다.
	const FVector CurrentCollisionLocation = CollisionComponent->GetComponentLocation();
	if (FVector::DistSquared(PreviousCollisionLocation, CurrentCollisionLocation) <= KINDA_SMALL_NUMBER)
	{
		PreviousCollisionLocation = CurrentCollisionLocation;
		return;
	}

	// [v1.5.0] 발사체 자신과 발사 주체를 보조 Sweep에서 제외하는 Query 파라미터입니다.
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (ActiveInstigatorActor)
	{
		QueryParams.AddIgnoredActor(ActiveInstigatorActor);
	}

	// [v1.5.0] Pool 또는 Spawn 경로에서 지정된 소유 Actor입니다.
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		QueryParams.AddIgnoredActor(OwnerActor);
	}

	// [v1.5.0] 실제 충돌 Sphere의 월드 스케일을 반영한 안전한 Sweep 반경입니다.
	const float SafeSweepRadius = FMath::Max(CollisionComponent->GetScaledSphereRadius(), 1.0f);

	// [v1.5.0] 이전 위치부터 현재 위치까지의 첫 Blocking Hit 결과입니다.
	FHitResult SweepHitResult;

	// [v1.5.0] Projectile 채널 응답표로 Sphere Sweep에서 Blocking Hit을 찾았는지 여부입니다.
	const bool bFoundBlockingHit = World->SweepSingleByChannel(
		SweepHitResult,
		PreviousCollisionLocation,
		CurrentCollisionLocation,
		FQuat::Identity,
		CFCollisionChannels::Projectile,
		FCollisionShape::MakeSphere(SafeSweepRadius),
		QueryParams);

	if (bFoundBlockingHit && SweepHitResult.bBlockingHit)
	{
		ResolveProjectileImpact(SweepHitResult.GetActor(), SweepHitResult.GetComponent(), SweepHitResult);
		if (!bProjectileActive)
		{
			return;
		}
	}

	PreviousCollisionLocation = CurrentCollisionLocation;
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

// [v1.5.0] ProjectileData의 충돌 반경과 CCD 설정을 CollisionComponent에 적용합니다.
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
	CollisionComponent->SetCollisionObjectType(CFCollisionChannels::Projectile);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetUseCCD(InProjectileData.bUseCCD);
}

// [v1.5.0] ProjectileData의 이동과 연속 충돌 설정을 ProjectileMovementComponent에 적용합니다.
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

	// [v1.5.0] 한 번의 ProjectileMovement 시뮬레이션 단계에 사용할 안전한 최대 시간입니다.
	const float SafeMaxSimulationTimeStep = FMath::Clamp(InProjectileData.MaxSimulationTimeStep, 0.001f, 0.05f);

	// [v1.5.0] 한 프레임에서 허용할 안전한 최대 시뮬레이션 반복 횟수입니다.
	const int32 SafeMaxSimulationIterations = FMath::Clamp(InProjectileData.MaxSimulationIterations, 1, 32);

	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->bSweepCollision = InProjectileData.bUseSweepCollision;
	ProjectileMovementComponent->bForceSubStepping = InProjectileData.bForceSubStepping;
	ProjectileMovementComponent->MaxSimulationTimeStep = SafeMaxSimulationTimeStep;
	ProjectileMovementComponent->MaxSimulationIterations = SafeMaxSimulationIterations;
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
