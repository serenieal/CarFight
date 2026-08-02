// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.0
// Date: 2026-08-02
// Description: CarFight 공통 발사체 Actor 구현
// Scope: ProjectileData 기반 추진 / 미사일 비행·유도 / 표시 / 지속형 비행 FX / 이동 / 풀링 / 동일 발사 차량 격리, Projectile 요격과 정식 차량 방어 결과를 제공합니다.
// Changelog:
// - v1.13.0: MissileFlightComp·MissileGuideComp 생성, Target Snapshot Guidance, 현재 방향 추진과 비활성화 Reset을 연결.
// - v1.12.0: 첫 유효 Impact를 VehicleDefenseComp 진입점으로 전환하고 전체 방어 결과와 기존 Integrity 결과를 함께 보존.
// - v1.11.0: Projectile 기본 Block, 동일 발사 차량 양방향 Ignore·정리, 다른 차량 Projectile·Hitscan Intercepted 처리를 구현.
// - v1.10.0: Projectile 채널 상호 Ignore로 Salvo 자폭을 막은 임시 구현. v1.11.0에서 차량별 격리로 교체.
// - v1.9.0: Launch Context 기반 활성화, Legacy Adapter, 초기 월드 Velocity 적용과 비활성화 Context Reset 구현.
// - v1.8.1: RelativeTransform Scale을 소켓·Fallback 공통 독립 FX Scale로 적용하고 Debug 요약에 실제 Scale 추가.
// - v1.8.0: ProjectileMotorComp 생성·활성화·Reset, 추진 최대 속도와 실제 Burning 상태 기반 Thruster FX 동기화 구현.
// - v1.7.0: Trail·Thruster NiagaraComponent, 메시 소켓/Fallback 부착과 Pool 반환 전 FX Reset 구현.
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
// - Trail과 추진 화염이 비활성 또는 미연결이어도 기존 Projectile 활성화, 이동, 충돌과 피해는 유지한다.
// - Hit, LifeExpired, Manual, InvalidActivation 비활성화는 Pool 반환 전에 두 Niagara를 Reset하고 Asset 참조를 비운다.
// - 비행 FX RelativeTransform의 Scale은 소켓 경로에서도 적용하고 위치·회전만 소켓 Transform을 우선한다.
// - PropulsionConfig가 비활성이면 기존 InitialSpeed 고정 비행을 유지하고, 활성화되면 MotorComp가 ProjectileMovement보다 먼저 추진 가속을 적용한다.
// - Thruster Niagara는 모터 Burning 상태에서만 재생하고 IgnitionDelay·BurnedOut·Disabled에서는 정지한다.
// - 기존 ActivateProjectile 호출은 Direct Launch Context로 변환되며 기존 InitialSpeed 결과를 유지한다.
// - Context 초기 Velocity가 유효하면 월드 Velocity로 적용하고, 무효하면 InitialLaunchDirection * InitialSpeed로 안전하게 복구한다.
// - 비활성화 시 ActiveLaunchContext와 존재 여부를 초기화해 Pool 재사용 오염을 막는다.
// - Projectile 채널은 기본 Block이며 서로 다른 발사 차량의 Projectile은 실제 충돌과 요격 대상이다.
// - 같은 ActiveInstigatorActor의 모든 탄종·Volley만 Pool이 양방향 Ignore로 연결하고 비활성화 전에 관계를 정리한다.
// - 요격 불가 Projectile은 적중 뒤 기존 Velocity를 복구하고, 양쪽 모두 요격 불가면 해당 Actor 쌍만 추가 충돌에서 제외한다.

#include "CFProjectileActor.h"

#include "CFCollisionChannels.h"
#include "CFDamageData.h"
#include "CFMissileFlightComp.h"
#include "CFMissileGuideComp.h"
#include "CFProjectileData.h"
#include "CFProjectileMotorComp.h"
#include "CFProjectilePoolComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehicleDefenseComp.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
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
	CollisionComponent->SetCollisionResponseToChannel(CFCollisionChannels::Projectile, ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetGenerateOverlapEvents(false);

	// [v1.0.0] ProjectileData의 StaticMesh를 표시할 시각 컴포넌트입니다.
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);

	// [v1.7.0] Trail FX의 소켓 또는 Projectile Relative Fallback을 적용할 독립 원점입니다.
	TrailOriginComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TrailOriginComponent"));
	TrailOriginComponent->SetupAttachment(CollisionComponent);
	TrailOriginComponent->SetAbsolute(false, false, true);

	// [v1.7.0] Projectile 활성 시간 동안 Trail을 재생하고 Pool과 함께 재사용할 NiagaraComponent입니다.
	TrailNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailNiagaraComponent"));
	TrailNiagaraComponent->SetupAttachment(TrailOriginComponent);
	TrailNiagaraComponent->SetAutoActivate(false);
	TrailNiagaraComponent->SetAutoDestroy(false);
	TrailNiagaraComponent->SetRelativeTransform(FTransform::Identity);

	// [v1.7.0] 추진 화염 FX의 소켓 또는 Projectile Relative Fallback을 적용할 독립 원점입니다.
	ThrusterOriginComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ThrusterOriginComponent"));
	ThrusterOriginComponent->SetupAttachment(CollisionComponent);
	ThrusterOriginComponent->SetAbsolute(false, false, true);

	// [v1.7.0] Projectile 활성 시간 동안 추진 화염을 재생하고 Pool과 함께 재사용할 NiagaraComponent입니다.
	ThrusterNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ThrusterNiagaraComponent"));
	ThrusterNiagaraComponent->SetupAttachment(ThrusterOriginComponent);
	ThrusterNiagaraComponent->SetAutoActivate(false);
	ThrusterNiagaraComponent->SetAutoDestroy(false);
	ThrusterNiagaraComponent->SetRelativeTransform(FTransform::Identity);

	// [v1.0.0] ProjectileData의 이동값을 적용받을 ProjectileMovement 컴포넌트입니다.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->bAutoActivate = false;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
	ProjectileMovementComponent->InitialSpeed = 6000.0f;
	ProjectileMovementComponent->MaxSpeed = 6000.0f;

		// [v1.8.0] ProjectileMovement Velocity에 점화·연소 추진을 적용할 독립 모터 컴포넌트입니다.
	ProjectileMotorComponent = CreateDefaultSubobject<UCFProjectileMotorComp>(TEXT("ProjectileMotorComponent"));
	ProjectileMovementComponent->AddTickPrerequisiteComponent(ProjectileMotorComponent);

	// [v1.13.0] 미사일의 Released·Clearance·GuidedFlight 상태를 ProjectileMovement 이전에 진행할 컴포넌트입니다.
	MissileFlightComponent = CreateDefaultSubobject<UCFMissileFlightComp>(TEXT("MissileFlightComponent"));

	// [v1.13.0] Motor와 Flight 이후 제한형 Guidance로 Velocity 방향을 갱신할 컴포넌트입니다.
	MissileGuideComponent = CreateDefaultSubobject<UCFMissileGuideComp>(TEXT("MissileGuideComponent"));
	MissileGuideComponent->AddTickPrerequisiteComponent(ProjectileMotorComponent);
	MissileGuideComponent->AddTickPrerequisiteComponent(MissileFlightComponent);
	ProjectileMovementComponent->AddTickPrerequisiteComponent(MissileGuideComponent);

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

	if (ProjectileMotorComponent)
	{
		ProjectileMotorComponent->OnMotorStateChanged.RemoveDynamic(this, &ACFProjectileActor::HandleProjectileMotorStateChanged);
		ProjectileMotorComponent->OnMotorStateChanged.AddDynamic(this, &ACFProjectileActor::HandleProjectileMotorStateChanged);
	}

	SetActorTickEnabled(false);

	if (!bProjectileActive)
	{
		DeactivateProjectileFlightFx();
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

// [v1.9.0] 기존 호출 호환을 위해 발사 방향에서 Direct Launch Context를 생성해 발사체를 활성화합니다.
void ACFProjectileActor::ActivateProjectile(UCFProjectileData* InProjectileData, const FVector& InLaunchDirection, AActor* InInstigatorActor)
{
	// [v1.9.0] 기존 발사 방향 입력에서 NaN과 0 벡터를 제거한 Direct 발사 방향입니다.
	FVector SafeLaunchDirection = InLaunchDirection.GetSafeNormal();
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = GetActorForwardVector().GetSafeNormal();
	}
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = FVector::ForwardVector;
	}

	// [v1.9.0] 기존 ProjectileData InitialSpeed를 보존할 안전한 초기 속력입니다.
	const float SafeInitialSpeed = InProjectileData
		? FMath::Max(InProjectileData->InitialSpeed, 1.0f)
		: 1.0f;

	// [v1.9.0] 기존 ActivateProjectile 호출을 신규 Context 경로로 전달할 Direct 발사 데이터입니다.
	FCFProjectileLaunchContext LegacyLaunchContext;
	LegacyLaunchContext.LaunchTransform = FTransform(SafeLaunchDirection.Rotation(), GetActorLocation());
	LegacyLaunchContext.InitialLaunchDirection = SafeLaunchDirection;
	LegacyLaunchContext.InitialLaunchVelocity = SafeLaunchDirection * SafeInitialSpeed;
	LegacyLaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;

	ActivateProjectileWithContext(InProjectileData, LegacyLaunchContext, InInstigatorActor);
}

// [v1.9.0] 발사 위치·초기 방향·초기 월드 속도를 포함한 Launch Context를 복사해 발사체를 활성화합니다.
void ACFProjectileActor::ActivateProjectileWithContext(
	UCFProjectileData* InProjectileData,
	const FCFProjectileLaunchContext& InLaunchContext,
	AActor* InInstigatorActor)
{
	if (!InProjectileData || !CollisionComponent || !ProjectileMovementComponent)
	{
		DeactivateProjectileWithReason(ECFProjectileDeactivateReason::InvalidActivation, TEXT("ActivationInputInvalid"));
		return;
	}

	// [v1.9.0] Context 초기 Velocity가 유효한지 검사할 원본 월드 Velocity입니다.
	FVector SafeInitialLaunchVelocity = InLaunchContext.InitialLaunchVelocity;
	if (SafeInitialLaunchVelocity.ContainsNaN())
	{
		SafeInitialLaunchVelocity = FVector::ZeroVector;
	}

	// [v1.9.0] Context 방향에서 NaN과 0 벡터를 제거한 초기 발사 방향입니다.
	FVector SafeLaunchDirection = InLaunchContext.InitialLaunchDirection.GetSafeNormal();
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = SafeInitialLaunchVelocity.GetSafeNormal();
	}
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = InLaunchContext.LaunchTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	}
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = GetActorForwardVector().GetSafeNormal();
	}
	if (SafeLaunchDirection.ContainsNaN() || SafeLaunchDirection.IsNearlyZero())
	{
		SafeLaunchDirection = FVector::ForwardVector;
	}

	// [v1.9.0] Context Velocity가 비어 있을 때 기존 InitialSpeed 결과를 복구할 안전 속력입니다.
	const float SafeFallbackInitialSpeed = FMath::Max(InProjectileData->InitialSpeed, 1.0f);
	if (SafeInitialLaunchVelocity.IsNearlyZero())
	{
		SafeInitialLaunchVelocity = SafeLaunchDirection * SafeFallbackInitialSpeed;
	}

	// [v1.9.0] NaN Transform 입력을 현재 Actor 위치 기반 Direct Transform으로 교체한 발사 Transform입니다.
	FTransform SafeLaunchTransform = InLaunchContext.LaunchTransform;
	if (SafeLaunchTransform.ContainsNaN())
	{
		SafeLaunchTransform = FTransform(SafeLaunchDirection.Rotation(), GetActorLocation());
	}
	SafeLaunchTransform.SetRotation(SafeLaunchDirection.Rotation().Quaternion());

		// [v1.11.0] Pool 재사용 Actor에 남을 수 있는 이전 동일 발사 차량 Ignore 관계를 먼저 정리합니다.
	ClearSameSourceProjectileIgnores();

	// [v1.0.0] 이전 발사에서 무시하던 발사 주체입니다.
	AActor* PreviousInstigatorActor = ActiveInstigatorActor.Get();
	if (PreviousInstigatorActor && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(PreviousInstigatorActor, false);
	}

	// [v1.0.0] 이번 발사체에 적용할 ProjectileData입니다.
	ActiveProjectileData = InProjectileData;

	// [v1.9.0] 런처와 독립적으로 유지할 이번 활성화의 안전한 Launch Context 복사본입니다.
	ActiveLaunchContext = InLaunchContext;
	ActiveLaunchContext.LaunchTransform = SafeLaunchTransform;
	ActiveLaunchContext.InitialLaunchDirection = SafeLaunchDirection;
	ActiveLaunchContext.InitialLaunchVelocity = SafeInitialLaunchVelocity;
	bHasActiveLaunchContext = true;

	SetActorTransform(SafeLaunchTransform, false, nullptr, ETeleportType::TeleportPhysics);

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
	bHasLastVehicleDamageResult = false;
	LastVehicleDamageResult = FCFVehicleDamageResult();
	LastDamageApplyResult = FCFDamageApplyResult();
	bImpactResolvedThisActivation = false;
	PreviousCollisionLocation = CollisionComponent->GetComponentLocation();

	// [v1.0.0] 이번 발사체를 발사한 Actor입니다.
	ActiveInstigatorActor = InInstigatorActor;
		if (ActiveInstigatorActor && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(ActiveInstigatorActor, true);
	}

	if (ProjectilePoolOwnerComp && ActiveInstigatorActor)
	{
		ProjectilePoolOwnerComp->RegisterSameSourceProjectileIsolation(this, ActiveInstigatorActor);
	}

	LastIncomingDirection = SafeLaunchDirection;

	ApplyProjectileVisual(*ActiveProjectileData);
	ResetProjectileFlightFx();
	ApplyProjectileFlightFx(*ActiveProjectileData);
	ApplyProjectileCollision(*ActiveProjectileData);
	ApplyProjectileMovement(*ActiveProjectileData, SafeLaunchDirection, SafeInitialLaunchVelocity);

		// [v1.13.0] 기존 Rocket 고정 방향과 미사일 현재 Velocity 방향을 분리할 안전 보정 Flight 설정입니다.
	const FCFMissileFlightConfig EffectiveMissileFlightConfig = ActiveProjectileData->GetEffectiveMissileFlightConfig();

	// [v1.13.0] Missile Flight 활성 여부에 따라 선택한 이번 모터 추진 방향 모드입니다.
	const ECFProjectileThrustDirectionMode ThrustDirectionMode = EffectiveMissileFlightConfig.bUseMissileFlight
		? ECFProjectileThrustDirectionMode::CurrentVelocityDirection
		: ECFProjectileThrustDirectionMode::FixedLaunchDirection;

	if (ProjectileMotorComponent)
	{
		ProjectileMotorComponent->StartMotorWithDirectionMode(
			ActiveProjectileData->PropulsionConfig,
			ProjectileMovementComponent,
			SafeLaunchDirection,
			ThrustDirectionMode);
	}

	if (MissileFlightComponent)
	{
		MissileFlightComponent->StartMissileFlight(
			EffectiveMissileFlightConfig,
			ActiveLaunchContext,
			ProjectileMovementComponent);
	}

	if (MissileGuideComponent)
	{
		MissileGuideComponent->StartMissileGuidance(
			ActiveProjectileData->GetEffectiveMissileGuideConfig(),
			ActiveLaunchContext,
			ProjectileMovementComponent,
			MissileFlightComponent);
	}
	RefreshThrusterFxFromMotorState();
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

		if (InDeactivateReason != ECFProjectileDeactivateReason::Hit
		&& InDeactivateReason != ECFProjectileDeactivateReason::Intercepted)
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
	ClearSameSourceProjectileIgnores();

	if (MissileGuideComponent)
	{
		MissileGuideComponent->ResetMissileGuidance();
	}

	if (MissileFlightComponent)
	{
		MissileFlightComponent->ResetMissileFlight();
	}

	if (ProjectileMotorComponent)
	{
		ProjectileMotorComponent->ResetMotor();
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
	DeactivateProjectileFlightFx();
	SetActorHiddenInGame(true);
		bProjectileActive = false;
	ActiveProjectileData = nullptr;
	ActiveInstigatorActor = nullptr;
	ActiveLaunchContext = FCFProjectileLaunchContext();
	bHasActiveLaunchContext = false;

	FinishDeactivatePolicy();
}

// [v1.8.1] 현재 Trail과 추진 화염의 활성 상태, 실제 부착 출처와 적용 Scale을 반환합니다.
FString ACFProjectileActor::BuildProjectileFlightFxSummary() const
{
	// [v1.8.1] 현재 Trail 원점에 적용된 독립 FX 월드 Scale입니다.
	const FVector TrailFxScale = TrailOriginComponent
		? TrailOriginComponent->GetComponentScale()
		: FVector::OneVector;

	// [v1.8.1] 현재 추진 원점에 적용된 독립 FX 월드 Scale입니다.
	const FVector ThrusterFxScale = ThrusterOriginComponent
		? ThrusterOriginComponent->GetComponentScale()
		: FVector::OneVector;

	return FString::Printf(
		TEXT("ProjectileFlightFx: TrailStatus=%s, TrailAttachment=%s, TrailScale=(%.3f, %.3f, %.3f), ThrusterStatus=%s, ThrusterAttachment=%s, ThrusterScale=(%.3f, %.3f, %.3f)"),
		*TrailFlightFxStatus,
		*TrailFlightFxAttachmentSource,
		TrailFxScale.X,
		TrailFxScale.Y,
		TrailFxScale.Z,
		*ThrusterFlightFxStatus,
		*ThrusterFlightFxAttachmentSource,
		ThrusterFxScale.X,
		ThrusterFxScale.Y,
		ThrusterFxScale.Z);
}

// [v1.9.0] 현재 Launch Context의 모드, 요청 ID, 방향과 속도를 표시하는 요약을 생성합니다.
FString ACFProjectileActor::BuildProjectileLaunchSummary() const
{
	// [v1.9.0] 현재 Release Mode를 사람이 읽을 수 있게 표시할 문자열입니다.
	const UEnum* ReleaseModeEnum = StaticEnum<ECFProjectileReleaseMode>();
	const FString ReleaseModeText = ReleaseModeEnum
		? ReleaseModeEnum->GetNameStringByValue(static_cast<int64>(ActiveLaunchContext.ReleaseMode))
		: TEXT("Unknown");

	return FString::Printf(
		TEXT("ProjectileLaunch: HasContext=%s, Mode=%s, Request=%d, WeaponGroup=%s, Direction=(%.3f, %.3f, %.3f), Velocity=(%.1f, %.1f, %.1f), CarrierVelocity=(%.1f, %.1f, %.1f), Target=(%.1f, %.1f, %.1f)"),
		bHasActiveLaunchContext ? TEXT("Yes") : TEXT("No"),
		*ReleaseModeText,
		ActiveLaunchContext.FireRequestId,
		*ActiveLaunchContext.WeaponGroupId.ToString(),
		ActiveLaunchContext.InitialLaunchDirection.X,
		ActiveLaunchContext.InitialLaunchDirection.Y,
		ActiveLaunchContext.InitialLaunchDirection.Z,
		ActiveLaunchContext.InitialLaunchVelocity.X,
		ActiveLaunchContext.InitialLaunchVelocity.Y,
		ActiveLaunchContext.InitialLaunchVelocity.Z,
		ActiveLaunchContext.InheritedCarrierVelocity.X,
		ActiveLaunchContext.InheritedCarrierVelocity.Y,
		ActiveLaunchContext.InheritedCarrierVelocity.Z,
		ActiveLaunchContext.CommandTargetLocation.X,
		ActiveLaunchContext.CommandTargetLocation.Y,
		ActiveLaunchContext.CommandTargetLocation.Z);
}

// [v1.8.0] 현재 점화·연소·관성 비행 상태와 추진 속도를 반환합니다.
FString ACFProjectileActor::BuildProjectileMotorSummary() const
{
	return ProjectileMotorComponent
		? ProjectileMotorComponent->BuildMotorSummary()
		: TEXT("ProjectileMotor: MissingComponent");
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

// [v1.11.0] 지정 Projectile이 현재 동일 발사 차량 Ignore 목록에 등록됐는지 반환합니다.
bool ACFProjectileActor::IsIgnoringSameSourceProjectile(const ACFProjectileActor* InProjectileActor) const
{
	if (!InProjectileActor)
	{
		return false;
	}

	return IgnoredSameSourceProjectileArray.ContainsByPredicate(
		[InProjectileActor](const TWeakObjectPtr<ACFProjectileActor>& IgnoredProjectileActor)
		{
			return IgnoredProjectileActor.Get() == InProjectileActor;
		});
}

// [v1.11.0] 현재 동일 발사 차량 Ignore 목록의 유효 Projectile 수를 반환합니다.
int32 ACFProjectileActor::GetIgnoredSameSourceProjectileCount() const
{
	int32 ValidIgnoredProjectileCount = 0;
	for (const TWeakObjectPtr<ACFProjectileActor>& IgnoredProjectileActor : IgnoredSameSourceProjectileArray)
	{
		if (IgnoredProjectileActor.IsValid())
		{
			++ValidIgnoredProjectileCount;
		}
	}
	return ValidIgnoredProjectileCount;
}

// [v1.11.0] 같은 발사 차량 Projectile인지 확인해 충돌 무시 정책을 결정합니다.
bool ACFProjectileActor::ShouldIgnoreProjectileCollision(const ACFProjectileActor* InOtherProjectileActor) const
{
	if (!InOtherProjectileActor || InOtherProjectileActor == this)
	{
		return true;
	}

	return ActiveInstigatorActor
		&& InOtherProjectileActor->ActiveInstigatorActor
		&& ActiveInstigatorActor == InOtherProjectileActor->ActiveInstigatorActor;
}

// [v1.11.0] 같은 발사 차량 Projectile 한 개를 현재 이동·보조 Sweep Ignore 목록에 추가합니다.
void ACFProjectileActor::AddSameSourceProjectileIgnore(ACFProjectileActor* InProjectileActor)
{
	if (!IsValid(InProjectileActor) || InProjectileActor == this)
	{
		return;
	}

	IgnoredSameSourceProjectileArray.RemoveAll(
		[](const TWeakObjectPtr<ACFProjectileActor>& IgnoredProjectileActor)
		{
			return !IgnoredProjectileActor.IsValid();
		});

	if (!IsIgnoringSameSourceProjectile(InProjectileActor))
	{
		IgnoredSameSourceProjectileArray.Add(InProjectileActor);
	}

	if (CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(InProjectileActor, true);
	}
}

// [v1.11.0] 같은 발사 차량 Projectile 한 개를 현재 Ignore 목록에서 제거합니다.
void ACFProjectileActor::RemoveSameSourceProjectileIgnore(ACFProjectileActor* InProjectileActor)
{
	if (CollisionComponent && IsValid(InProjectileActor))
	{
		CollisionComponent->IgnoreActorWhenMoving(InProjectileActor, false);
	}

	IgnoredSameSourceProjectileArray.RemoveAll(
		[InProjectileActor](const TWeakObjectPtr<ACFProjectileActor>& IgnoredProjectileActor)
		{
			return !IgnoredProjectileActor.IsValid() || IgnoredProjectileActor.Get() == InProjectileActor;
		});
}

// [v1.11.0] Pool 반환 전에 동일 발사 차량 Projectile Ignore 관계를 양방향으로 모두 해제합니다.
void ACFProjectileActor::ClearSameSourceProjectileIgnores()
{
	TArray<TWeakObjectPtr<ACFProjectileActor>> IgnoredProjectileSnapshot = IgnoredSameSourceProjectileArray;
	IgnoredSameSourceProjectileArray.Reset();

	for (const TWeakObjectPtr<ACFProjectileActor>& IgnoredProjectileActorPtr : IgnoredProjectileSnapshot)
	{
		ACFProjectileActor* IgnoredProjectileActor = IgnoredProjectileActorPtr.Get();
		if (!IsValid(IgnoredProjectileActor))
		{
			continue;
		}

		if (CollisionComponent)
		{
			CollisionComponent->IgnoreActorWhenMoving(IgnoredProjectileActor, false);
		}
		IgnoredProjectileActor->RemoveSameSourceProjectileIgnore(this);
	}
}

// [v1.11.0] 요격 불가 Projectile이 충돌 이벤트 뒤에도 기존 Velocity로 계속 비행하도록 Movement를 복구합니다.
void ACFProjectileActor::RestoreMovementAfterProjectileCollision(const FVector& InPreCollisionVelocity)
{
	if (!bProjectileActive || !ProjectileMovementComponent || InPreCollisionVelocity.ContainsNaN())
	{
		return;
	}

	ProjectileMovementComponent->Velocity = InPreCollisionVelocity;
	ProjectileMovementComponent->Activate(true);
}

// [v1.11.0] 다른 차량 Projectile 또는 Hitscan 적중으로 현재 Projectile의 요격 종료를 시도합니다.
bool ACFProjectileActor::TryResolveProjectileInterception(
	AActor* InInterceptorActor,
	AActor* InInterceptorSourceActor,
	const FHitResult& InHitResult)
{
	if (!bProjectileActive || bImpactResolvedThisActivation || !ActiveProjectileData || !ActiveProjectileData->bCanBeIntercepted)
	{
		return false;
	}

	if (ActiveInstigatorActor && InInterceptorSourceActor && ActiveInstigatorActor == InInterceptorSourceActor)
	{
		return false;
	}

	bImpactResolvedThisActivation = true;
	LastHitActor = InInterceptorActor;
	LastHitComponentName = CollisionComponent ? CollisionComponent->GetFName() : NAME_None;
		LastImpactLocation = FVector(InHitResult.ImpactPoint).ContainsNaN()
		? GetActorLocation()
		: FVector(InHitResult.ImpactPoint);
	LastImpactNormal = InHitResult.ImpactNormal.GetSafeNormal();
	if (LastImpactNormal.IsNearlyZero())
	{
		LastImpactNormal = FVector::UpVector;
	}

	LastIncomingDirection = FVector::ZeroVector;
	if (const ACFProjectileActor* InterceptorProjectileActor = Cast<ACFProjectileActor>(InInterceptorActor))
	{
		if (const UProjectileMovementComponent* InterceptorMovementComponent = InterceptorProjectileActor->FindComponentByClass<UProjectileMovementComponent>())
		{
			LastIncomingDirection = InterceptorMovementComponent->Velocity.GetSafeNormal();
		}
	}
	if (LastIncomingDirection.IsNearlyZero())
	{
		LastIncomingDirection = (InHitResult.TraceEnd - InHitResult.TraceStart).GetSafeNormal();
	}
	if (LastIncomingDirection.IsNearlyZero() && InInterceptorSourceActor)
	{
		LastIncomingDirection = (GetActorLocation() - InInterceptorSourceActor->GetActorLocation()).GetSafeNormal();
	}
	if (LastIncomingDirection.IsNearlyZero())
	{
		LastIncomingDirection = FVector::ForwardVector;
	}

	FCFDamageHitContext InterceptionHitContext;
	InterceptionHitContext.DamageData = nullptr;
	InterceptionHitContext.DamageId = TEXT("ProjectileIntercept");
	InterceptionHitContext.WeaponId = NAME_None;
	InterceptionHitContext.ProjectileId = ActiveProjectileData->ProjectileId;
	InterceptionHitContext.HitActor = this;
	InterceptionHitContext.HitComponentName = LastHitComponentName;
	InterceptionHitContext.ImpactLocation = LastImpactLocation;
	InterceptionHitContext.ImpactNormal = LastImpactNormal;
	InterceptionHitContext.IncomingDirection = LastIncomingDirection;
	InterceptionHitContext.InstigatorActor = InInterceptorSourceActor;

	UWorld* ImpactWorld = GetWorld();
	const float ImpactTimeSeconds = ImpactWorld ? ImpactWorld->GetTimeSeconds() : -1.0f;
	InterceptionHitContext.FlightDurationSeconds = (LastActivationTimeSeconds >= 0.0f && ImpactTimeSeconds >= 0.0f)
		? FMath::Max(ImpactTimeSeconds - LastActivationTimeSeconds, 0.0f)
		: 0.0f;
	InterceptionHitContext.bFromProjectileActor = Cast<ACFProjectileActor>(InInterceptorActor) != nullptr;
	InterceptionHitContext.bBlockingHit = true;

		bHasLastDamageHitContext = true;
	LastDamageHitContext = InterceptionHitContext;
	bHasLastVehicleDamageResult = false;
	LastVehicleDamageResult = FCFVehicleDamageResult();
	LastDamageApplyResult = FCFDamageApplyResult();

	const FString InterceptorActorName = InInterceptorActor ? InInterceptorActor->GetName() : TEXT("None");
	DeactivateProjectileWithReason(ECFProjectileDeactivateReason::Intercepted, InterceptorActorName);
	return true;
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

		ACFProjectileActor* HitProjectileActor = Cast<ACFProjectileActor>(InHitActor);
	if (HitProjectileActor)
	{
		if (ShouldIgnoreProjectileCollision(HitProjectileActor))
		{
			return false;
		}

		const FVector ThisProjectilePreCollisionVelocity = ProjectileMovementComponent
			? ProjectileMovementComponent->Velocity
			: FVector::ZeroVector;
		UProjectileMovementComponent* HitProjectileMovementComponent = HitProjectileActor->FindComponentByClass<UProjectileMovementComponent>();
		const FVector HitProjectilePreCollisionVelocity = HitProjectileMovementComponent
			? HitProjectileMovementComponent->Velocity
			: FVector::ZeroVector;
		AActor* ThisProjectileSourceActor = ActiveInstigatorActor;
		AActor* HitProjectileSourceActor = HitProjectileActor->GetActiveInstigatorActor();

		const bool bHitProjectileIntercepted = HitProjectileActor->TryResolveProjectileInterception(
			this,
			ThisProjectileSourceActor,
			InHitResult);

		FHitResult ReverseHitResult = InHitResult;
		ReverseHitResult.ImpactNormal *= -1.0f;
		ReverseHitResult.Normal *= -1.0f;
		const bool bThisProjectileIntercepted = TryResolveProjectileInterception(
			HitProjectileActor,
			HitProjectileSourceActor,
			ReverseHitResult);

		if (!bThisProjectileIntercepted)
		{
			RestoreMovementAfterProjectileCollision(ThisProjectilePreCollisionVelocity);
		}
		if (!bHitProjectileIntercepted)
		{
			HitProjectileActor->RestoreMovementAfterProjectileCollision(HitProjectilePreCollisionVelocity);
		}

		if (!bThisProjectileIntercepted && !bHitProjectileIntercepted)
		{
			AddSameSourceProjectileIgnore(HitProjectileActor);
			HitProjectileActor->AddSameSourceProjectileIgnore(this);
		}

		return bThisProjectileIntercepted || bHitProjectileIntercepted;
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
	bHasLastVehicleDamageResult = true;
	LastVehicleDamageResult = FCFVehicleDamageResult();
	UCFVehicleDefenseComp::TryApplyDamageToActor(LastDamageHitContext, LastVehicleDamageResult);
	LastDamageApplyResult = UCFVehicleDefenseComp::BuildIntegrityCompatibilityResult(LastVehicleDamageResult);

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

	for (const TWeakObjectPtr<ACFProjectileActor>& IgnoredProjectileActor : IgnoredSameSourceProjectileArray)
	{
		if (IgnoredProjectileActor.IsValid())
		{
			QueryParams.AddIgnoredActor(IgnoredProjectileActor.Get());
		}
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

// [v1.8.0] 추진 모터 상태가 변경됐을 때 실제 Thruster Niagara 활성 상태를 동기화합니다.
void ACFProjectileActor::HandleProjectileMotorStateChanged(
	const ECFProjectileMotorState PreviousMotorState,
	const ECFProjectileMotorState NewMotorState)
{
	(void)PreviousMotorState;
	(void)NewMotorState;
	RefreshThrusterFxFromMotorState();
}

// [v1.8.0] 현재 모터 상태를 기준으로 추진 화염 Niagara를 시작·정지하고 Debug 상태를 갱신합니다.
void ACFProjectileActor::RefreshThrusterFxFromMotorState()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		ThrusterFlightFxStatus = TEXT("DedicatedServerSkipped");
		return;
	}

	if (!ActiveProjectileData)
	{
		DeactivateThrusterFxForMotorState(TEXT("Inactive"));
		return;
	}

	const FCFProjectileAttachedFxSettings& ThrusterFxSettings = ActiveProjectileData->ThrusterFxSettings;
	if (!ThrusterFxSettings.bEnabled)
	{
		DeactivateThrusterFxForMotorState(TEXT("Disabled"));
		return;
	}

	if (!ThrusterFxSettings.NiagaraSystem)
	{
		DeactivateThrusterFxForMotorState(TEXT("MissingSystem"));
		return;
	}

	if (!ProjectileMotorComponent)
	{
		DeactivateThrusterFxForMotorState(TEXT("MotorMissing"));
		return;
	}

	switch (ProjectileMotorComponent->GetMotorState())
	{
	case ECFProjectileMotorState::Burning:
		ActivatePreparedThrusterFx();
		break;

	case ECFProjectileMotorState::IgnitionDelay:
		DeactivateThrusterFxForMotorState(TEXT("IgnitionPending"));
		break;

	case ECFProjectileMotorState::BurnedOut:
		DeactivateThrusterFxForMotorState(TEXT("BurnedOut"));
		break;

	case ECFProjectileMotorState::Disabled:
		DeactivateThrusterFxForMotorState(TEXT("MotorDisabled"));
		break;

	case ECFProjectileMotorState::Inactive:
	default:
		DeactivateThrusterFxForMotorState(TEXT("Inactive"));
		break;
	}
}

// [v1.8.0] 준비된 Thruster Niagara 자산을 실제 연소 표현으로 활성화합니다.
void ACFProjectileActor::ActivatePreparedThrusterFx()
{
	if (!ThrusterNiagaraComponent || !ThrusterNiagaraComponent->GetAsset())
	{
		ThrusterFlightFxStatus = TEXT("MissingSystem");
		return;
	}

	if (!ThrusterNiagaraComponent->IsActive())
	{
		ThrusterNiagaraComponent->ReinitializeSystem();
		ThrusterNiagaraComponent->Activate(true);
	}

	ThrusterFlightFxStatus = ThrusterNiagaraComponent->IsActive()
		? TEXT("Active")
		: TEXT("ActivationRequested");
}

// [v1.8.0] 추진 화염을 정지하되 Pool 전체 Reset 전까지 Asset과 부착 위치는 유지합니다.
void ACFProjectileActor::DeactivateThrusterFxForMotorState(const FString& InMotorFxStatus)
{
	if (ThrusterNiagaraComponent && ThrusterNiagaraComponent->IsActive())
	{
		ThrusterNiagaraComponent->DeactivateImmediate();
		ThrusterNiagaraComponent->ResetSystem();
	}

	ThrusterFlightFxStatus = InMotorFxStatus;
}

// [v1.7.0] 이전 활성화의 Trail·추진 화염 상태와 Asset 참조를 안전하게 초기화합니다.
void ACFProjectileActor::ResetProjectileFlightFx()
{
	// [v1.7.0] 한 NiagaraComponent의 실행 상태와 이전 Asset 참조를 정리하는 함수입니다.
	const auto ResetNiagaraComponent = [](UNiagaraComponent* InNiagaraComponent)
	{
		if (!InNiagaraComponent)
		{
			return;
		}

		InNiagaraComponent->DeactivateImmediate();
		InNiagaraComponent->ResetSystem();
		InNiagaraComponent->SetAsset(nullptr);
		InNiagaraComponent->SetRelativeTransform(FTransform::Identity);
	};

	// [v1.7.0] FX 원점을 기본 Projectile Relative 위치로 되돌리는 함수입니다.
	const auto ResetFxOrigin = [this](USceneComponent* InFxOriginComponent)
	{
		if (!InFxOriginComponent || !CollisionComponent)
		{
			return;
		}

		InFxOriginComponent->AttachToComponent(CollisionComponent, FAttachmentTransformRules::KeepRelativeTransform);
		InFxOriginComponent->SetAbsolute(false, false, true);
		InFxOriginComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		InFxOriginComponent->SetWorldScale3D(FVector::OneVector);
	};

	ResetNiagaraComponent(TrailNiagaraComponent);
	ResetNiagaraComponent(ThrusterNiagaraComponent);
	ResetFxOrigin(TrailOriginComponent);
	ResetFxOrigin(ThrusterOriginComponent);

	TrailFlightFxStatus = TEXT("Inactive");
	TrailFlightFxAttachmentSource = TEXT("None");
	ThrusterFlightFxStatus = TEXT("Inactive");
	ThrusterFlightFxAttachmentSource = TEXT("None");
}

// [v1.7.0] ProjectileData의 Trail·추진 화염 설정을 독립 NiagaraComponent에 적용합니다.
void ACFProjectileActor::ApplyProjectileFlightFx(const UCFProjectileData& InProjectileData)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		TrailFlightFxStatus = TEXT("DedicatedServerSkipped");
		TrailFlightFxAttachmentSource = TEXT("None");
		ThrusterFlightFxStatus = TEXT("DedicatedServerSkipped");
		ThrusterFlightFxAttachmentSource = TEXT("None");
		return;
	}

	ApplyProjectileAttachedFx(
		InProjectileData.TrailFxSettings,
		TrailOriginComponent,
		TrailNiagaraComponent,
		TrailFlightFxStatus,
		TrailFlightFxAttachmentSource,
		true);

	// [v1.8.0] Thruster는 Asset과 부착 위치만 준비하고 실제 활성화는 Motor Burning 상태가 결정합니다.
	ApplyProjectileAttachedFx(
		InProjectileData.ThrusterFxSettings,
		ThrusterOriginComponent,
		ThrusterNiagaraComponent,
		ThrusterFlightFxStatus,
		ThrusterFlightFxAttachmentSource,
		false);
}

// [v1.7.0] 한 비행 FX 슬롯의 부착 기준, Niagara와 Debug 상태를 적용합니다.
void ACFProjectileActor::ApplyProjectileAttachedFx(
	const FCFProjectileAttachedFxSettings& InFxSettings,
	USceneComponent* InFxOriginComponent,
	UNiagaraComponent* InNiagaraComponent,
	FString& OutFxStatus,
	FString& OutAttachmentSource,
	const bool bActivateImmediately)
{
	OutFxStatus = TEXT("Disabled");
	OutAttachmentSource = TEXT("None");

	if (!InFxOriginComponent || !InNiagaraComponent || !CollisionComponent)
	{
		OutFxStatus = TEXT("MissingComponent");
		return;
	}

	if (!InFxSettings.bEnabled)
	{
		return;
	}

	// [v1.8.1] 소켓·Fallback 경로에 공통 적용할 NaN 방지 독립 FX Scale입니다.
	FVector SafeIndependentFxScale = InFxSettings.RelativeTransform.GetScale3D();
	if (SafeIndependentFxScale.ContainsNaN())
	{
		SafeIndependentFxScale = FVector::OneVector;
	}

	SafeIndependentFxScale.X = FMath::Max(0.001f, SafeIndependentFxScale.X);
	SafeIndependentFxScale.Y = FMath::Max(0.001f, SafeIndependentFxScale.Y);
	SafeIndependentFxScale.Z = FMath::Max(0.001f, SafeIndependentFxScale.Z);

	// [v1.7.0] 메시 소켓을 실제로 사용할 수 있는지 여부입니다.
	const bool bCanUseMeshSocket =
		InFxSettings.AttachMode == ECFProjectileFxAttachMode::MeshSocketWithFallback
		&& MeshComponent
		&& !InFxSettings.AttachSocketName.IsNone()
		&& MeshComponent->DoesSocketExist(InFxSettings.AttachSocketName);

	if (bCanUseMeshSocket)
	{
		InFxOriginComponent->AttachToComponent(
			MeshComponent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			InFxSettings.AttachSocketName);
		InFxOriginComponent->SetAbsolute(false, false, true);
		InFxOriginComponent->SetWorldScale3D(SafeIndependentFxScale);
		OutAttachmentSource = FString::Printf(TEXT("MeshSocket:%s"), *InFxSettings.AttachSocketName.ToString());
	}
	else
	{
		InFxOriginComponent->AttachToComponent(CollisionComponent, FAttachmentTransformRules::KeepRelativeTransform);
		InFxOriginComponent->SetAbsolute(false, false, true);
		InFxOriginComponent->SetRelativeLocationAndRotation(
			InFxSettings.RelativeTransform.GetLocation(),
			InFxSettings.RelativeTransform.Rotator());
		InFxOriginComponent->SetWorldScale3D(SafeIndependentFxScale);

		const bool bMissingRequestedSocket =
			InFxSettings.AttachMode == ECFProjectileFxAttachMode::MeshSocketWithFallback
			&& !InFxSettings.AttachSocketName.IsNone();
		OutAttachmentSource = bMissingRequestedSocket
			? FString::Printf(TEXT("MissingSocketFallback:%s"), *InFxSettings.AttachSocketName.ToString())
			: TEXT("ProjectileRelative");
	}

	InNiagaraComponent->SetRelativeTransform(FTransform::Identity);

	UNiagaraSystem* SelectedNiagaraSystem = InFxSettings.NiagaraSystem.Get();
	if (!SelectedNiagaraSystem)
	{
		InNiagaraComponent->SetAsset(nullptr);
		OutFxStatus = TEXT("MissingSystem");
		return;
	}

	InNiagaraComponent->SetAsset(SelectedNiagaraSystem);

	if (!bActivateImmediately)
	{
		InNiagaraComponent->DeactivateImmediate();
		InNiagaraComponent->ResetSystem();
		OutFxStatus = TEXT("Prepared");
		return;
	}

	InNiagaraComponent->ReinitializeSystem();
	InNiagaraComponent->Activate(true);
	OutFxStatus = InNiagaraComponent->IsActive() ? TEXT("Active") : TEXT("ActivationRequested");
}

// [v1.7.0] Pool 반환 또는 Destroy 전에 Trail·추진 화염을 즉시 정지하고 초기화합니다.
void ACFProjectileActor::DeactivateProjectileFlightFx()
{
	ResetProjectileFlightFx();
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

	// [v1.10.0] 충돌을 다시 켜기 전에 응답표를 완성해 같은 프레임 Salvo Projectile과의 일시적인 Blocking Hit을 방지합니다.
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComponent->SetCollisionObjectType(CFCollisionChannels::Projectile);
		CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(CFCollisionChannels::Projectile, ECR_Block);
	CollisionComponent->SetUseCCD(InProjectileData.bUseCCD);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

// [v1.9.0] ProjectileData의 이동 설정과 Launch Context의 초기 월드 Velocity를 ProjectileMovementComponent에 적용합니다.
void ACFProjectileActor::ApplyProjectileMovement(
	const UCFProjectileData& InProjectileData,
	const FVector& InLaunchDirection,
	const FVector& InInitialLaunchVelocity)
{
	if (!ProjectileMovementComponent || !CollisionComponent)
	{
		return;
	}

	// [v1.9.0] Context Velocity가 무효일 때 기존 결과를 복구할 ProjectileData 기반 최소 속력입니다.
	const float SafeFallbackInitialSpeed = FMath::Max(InProjectileData.InitialSpeed, 1.0f);

	// [v1.9.0] 실제 ProjectileMovement에 적용할 안전한 초기 월드 Velocity입니다.
	FVector SafeInitialLaunchVelocity = InInitialLaunchVelocity;
	if (SafeInitialLaunchVelocity.ContainsNaN() || SafeInitialLaunchVelocity.IsNearlyZero())
	{
		SafeInitialLaunchVelocity = InLaunchDirection.GetSafeNormal() * SafeFallbackInitialSpeed;
	}

	// [v1.9.0] InitialSpeed와 MaxSpeed 계산에 사용할 실제 초기 월드 속력입니다.
	const float SafeInitialLaunchSpeed = FMath::Max(SafeInitialLaunchVelocity.Size(), 1.0f);

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

	// [v1.9.0] 추진 발사체는 설정 상한과 실제 초기 월드 속력 중 큰 값을 사용해 발사 직후 강제 감속을 막습니다.
	const float SafeMaximumSpeed = InProjectileData.PropulsionConfig.bUsePropulsion
		? FMath::Max(InProjectileData.PropulsionConfig.MaximumPropelledSpeed, SafeInitialLaunchSpeed)
		: SafeInitialLaunchSpeed;

	ProjectileMovementComponent->InitialSpeed = SafeInitialLaunchSpeed;
	ProjectileMovementComponent->MaxSpeed = SafeMaximumSpeed;
	ProjectileMovementComponent->ProjectileGravityScale = ProjectileGravityScale;
	ProjectileMovementComponent->Velocity = SafeInitialLaunchVelocity;
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
