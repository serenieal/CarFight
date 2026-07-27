// Copyright (c) CarFight. All Rights Reserved.
// Version: 1.2.0
// Date: 2026-07-25
// Changelog:
// - v1.2.0: Niagara User Vector Scale 적용과 차량별 FX_Destroyed 소켓 우선 파괴 위치 해석을 추가.
// - v1.1.0: CombatFxData 최대 수명 이후 Niagara를 강제 제거하는 Loop 안전 퓨즈를 추가.
// - v1.0.0: 데이터 기반 일회성 Niagara 생성과 최초 파괴 이벤트 수명을 구현.

#include "CFCombatFxComp.h"

#include "CFCombatFxData.h"
#include "CFVehicleData.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

UCFCombatFxComp::UCFCombatFxComp()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCFCombatFxComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindVehicleHealthEvents();
	OwnerVehiclePawn = nullptr;
	ActiveDestroyedFxData = nullptr;
	ActiveDestroyedFxSocketName = TEXT("FX_Destroyed");
	bCombatFxRuntimeReady = false;
	Super::EndPlay(EndPlayReason);
}

bool UCFCombatFxComp::InitializeCombatFxRuntime(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData, UCFVehicleHealthComp* InVehicleHealthComp)
{
	UnbindVehicleHealthEvents();
	OwnerVehiclePawn = InOwnerVehiclePawn;
	BoundVehicleHealthComp = InVehicleHealthComp;
	ActiveDestroyedFxData = InVehicleData ? InVehicleData->DefaultDestroyedFxData : nullptr;
	ActiveDestroyedFxSocketName = InVehicleData && !InVehicleData->DestroyedFxSocketName.IsNone()
		? InVehicleData->DestroyedFxSocketName
		: FName(TEXT("FX_Destroyed"));
	bDestroyedFxHandled = false;
	FireFxSpawnCount = 0;
	ImpactFxSpawnCount = 0;
	DestroyedFxSpawnCount = 0;
	LastDestroyedFxSpawnSource = TEXT("DestroyedFxSource: NotResolved");

	bCombatFxRuntimeReady = IsValid(OwnerVehiclePawn.Get());
	if (IsValid(BoundVehicleHealthComp.Get()))
	{
		BoundVehicleHealthComp->OnVehicleDestroyed.AddUniqueDynamic(this, &UCFCombatFxComp::HandleOwnerVehicleDestroyed);
	}

	LastCombatFxSummary = FString::Printf(
		TEXT("CombatFx: Owner=%s, Health=%s, DestroyedFx=%s, DestroyedSocket=%s, Ready=%s"),
		OwnerVehiclePawn ? *OwnerVehiclePawn->GetName() : TEXT("Missing"),
		BoundVehicleHealthComp ? TEXT("Bound") : TEXT("MissingOptional"),
		ActiveDestroyedFxData ? *ActiveDestroyedFxData->GetName() : TEXT("MissingOptional"),
		*ActiveDestroyedFxSocketName.ToString(),
		bCombatFxRuntimeReady ? TEXT("True") : TEXT("False"));
	return bCombatFxRuntimeReady;
}

bool UCFCombatFxComp::PlayFireFx(UCFCombatFxData* InCombatFxData, FVector FireLocation, FVector FireDirection)
{
	FVector SafeDirection = FireDirection.GetSafeNormal();
	if (SafeDirection.ContainsNaN() || SafeDirection.IsNearlyZero())
	{
		LastCombatFxSummary = TEXT("CombatFx Fire: InvalidDirection");
		return false;
	}

	const bool bSpawned = SpawnOneShotFx(InCombatFxData, FireLocation, SafeDirection.Rotation(), TEXT("Fire"));
	if (bSpawned)
	{
		++FireFxSpawnCount;
	}
	return bSpawned;
}

bool UCFCombatFxComp::PlayImpactFx(UCFCombatFxData* InCombatFxData, const FCFDamageHitContext& DamageHitContext)
{
	if (!DamageHitContext.bBlockingHit)
	{
		LastCombatFxSummary = TEXT("CombatFx Impact: NoBlockingHit");
		return false;
	}

	FVector SafeNormal = DamageHitContext.ImpactNormal.GetSafeNormal();
	if (SafeNormal.ContainsNaN() || SafeNormal.IsNearlyZero())
	{
		SafeNormal = FVector::UpVector;
	}

	const bool bSpawned = SpawnOneShotFx(InCombatFxData, DamageHitContext.ImpactLocation, SafeNormal.Rotation(), TEXT("Impact"));
	if (bSpawned)
	{
		++ImpactFxSpawnCount;
	}
	return bSpawned;
}

bool UCFCombatFxComp::PlayDestroyedFx(UCFCombatFxData* InCombatFxData, const FCFDamageHitContext& DamageHitContext)
{
	if (bDestroyedFxHandled)
	{
		LastCombatFxSummary = TEXT("CombatFx Destroyed: DuplicateIgnored");
		return false;
	}

	bDestroyedFxHandled = true;

	FTransform DestroyedFxTransform = FTransform::Identity;
	FString DestroyedFxSpawnSource;
	ResolveDestroyedFxTransform(DamageHitContext, DestroyedFxTransform, DestroyedFxSpawnSource);
	LastDestroyedFxSpawnSource = DestroyedFxSpawnSource;

	const bool bSpawned = SpawnOneShotFx(
		InCombatFxData,
		DestroyedFxTransform.GetLocation(),
		DestroyedFxTransform.Rotator(),
		TEXT("Destroyed"));

	LastCombatFxSummary += FString::Printf(TEXT(", SpawnSource=%s"), *DestroyedFxSpawnSource);
	if (bSpawned)
	{
		++DestroyedFxSpawnCount;
	}
	return bSpawned;
}

void UCFCombatFxComp::HandleOwnerVehicleDestroyed(FCFDamageHitContext DamageHitContext)
{
	PlayDestroyedFx(ActiveDestroyedFxData.Get(), DamageHitContext);
}

bool UCFCombatFxComp::ResolveDestroyedFxTransform(const FCFDamageHitContext& DamageHitContext, FTransform& OutDestroyedFxTransform, FString& OutSpawnSource) const
{
	const ACFVehiclePawn* VehiclePawn = OwnerVehiclePawn.Get();
	const UStaticMeshComponent* BodyMeshComponent = VehiclePawn ? VehiclePawn->GetVehicleBodyMeshComponent() : nullptr;

	if (BodyMeshComponent)
	{
		if (!ActiveDestroyedFxSocketName.IsNone() && BodyMeshComponent->DoesSocketExist(ActiveDestroyedFxSocketName))
		{
			OutDestroyedFxTransform = BodyMeshComponent->GetSocketTransform(ActiveDestroyedFxSocketName, RTS_World);
			OutDestroyedFxTransform.SetScale3D(FVector::OneVector);
			OutSpawnSource = FString::Printf(TEXT("SM_Body.Socket:%s"), *ActiveDestroyedFxSocketName.ToString());
			return true;
		}

		OutDestroyedFxTransform = FTransform(
			BodyMeshComponent->GetComponentQuat(),
			BodyMeshComponent->Bounds.Origin,
			FVector::OneVector);
		OutSpawnSource = FString::Printf(TEXT("SM_Body.BoundsFallback:MissingSocket=%s"), *ActiveDestroyedFxSocketName.ToString());
		return true;
	}

	const AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		OutDestroyedFxTransform = FTransform(OwnerActor->GetActorQuat(), OwnerActor->GetActorLocation(), FVector::OneVector);
		OutSpawnSource = TEXT("ActorTransformFallback:SM_BodyMissing");
		return true;
	}

	FVector SafeImpactNormal = DamageHitContext.ImpactNormal.GetSafeNormal();
	if (SafeImpactNormal.ContainsNaN() || SafeImpactNormal.IsNearlyZero())
	{
		SafeImpactNormal = FVector::UpVector;
	}
	OutDestroyedFxTransform = FTransform(SafeImpactNormal.Rotation(), DamageHitContext.ImpactLocation, FVector::OneVector);
	OutSpawnSource = TEXT("DamageHitContextFallback:OwnerMissing");
	return false;
}

bool UCFCombatFxComp::SpawnOneShotFx(UCFCombatFxData* InCombatFxData, const FVector& SpawnLocation, const FRotator& BaseRotation, const TCHAR* FxRoleText)
{
	if (!bCombatFxRuntimeReady)
	{
		LastCombatFxSummary = FString::Printf(TEXT("CombatFx %s: RuntimeNotReady"), FxRoleText);
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (OwnerActor && OwnerActor->GetNetMode() == NM_DedicatedServer)
	{
		LastCombatFxSummary = FString::Printf(TEXT("CombatFx %s: DedicatedServerSkipped"), FxRoleText);
		return false;
	}

	if (!InCombatFxData || !InCombatFxData->IsCombatFxConfigured())
	{
		LastCombatFxSummary = FString::Printf(TEXT("CombatFx %s: MissingOptionalData"), FxRoleText);
		return false;
	}

	UWorld* World = GetWorld();
	if (!World || SpawnLocation.ContainsNaN())
	{
		LastCombatFxSummary = FString::Printf(TEXT("CombatFx %s: InvalidWorldOrLocation"), FxRoleText);
		return false;
	}

	FVector SafeScale = InCombatFxData->FxScale;
	if (SafeScale.ContainsNaN())
	{
		SafeScale = FVector::OneVector;
	}

	const bool bUseNiagaraUserScale = InCombatFxData->FxScaleMode == ECFCombatFxScaleMode::NiagaraUserVector
		&& !InCombatFxData->NiagaraUserScaleParameterName.IsNone();
	const FVector SpawnComponentScale = bUseNiagaraUserScale ? FVector::OneVector : SafeScale;
	const FRotator FinalRotation = (BaseRotation.Quaternion() * InCombatFxData->RotationOffset.Quaternion()).Rotator();

	UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		InCombatFxData->NiagaraSystem,
		SpawnLocation,
		FinalRotation,
		SpawnComponentScale,
		true,
		false,
		ENCPoolMethod::None,
		true);

	const bool bSpawned = IsValid(SpawnedComponent);
	if (bSpawned)
	{
		SpawnedComponent->SetAbsolute(false, false, false);
		if (bUseNiagaraUserScale)
		{
			SpawnedComponent->SetWorldScale3D(FVector::OneVector);
			SpawnedComponent->SetVariableVec3(InCombatFxData->NiagaraUserScaleParameterName, SafeScale);
		}
		else
		{
			SpawnedComponent->SetWorldScale3D(SafeScale);
		}
		SpawnedComponent->Activate(true);
	}

	if (bSpawned && InCombatFxData->MaximumLifetimeSeconds > 0.0f)
	{
		const TWeakObjectPtr<UNiagaraComponent> WeakSpawnedComponent(SpawnedComponent);
		FTimerHandle MaximumLifetimeTimerHandle;
		World->GetTimerManager().SetTimer(
			MaximumLifetimeTimerHandle,
			FTimerDelegate::CreateLambda([WeakSpawnedComponent]()
			{
				if (UNiagaraComponent* NiagaraComponent = WeakSpawnedComponent.Get())
				{
					NiagaraComponent->DeactivateImmediate();
					NiagaraComponent->DestroyComponent();
				}
			}),
			InCombatFxData->MaximumLifetimeSeconds,
			false);
	}

	const TCHAR* ScaleModeText = bUseNiagaraUserScale ? TEXT("NiagaraUserVector") : TEXT("ComponentTransform");
	LastCombatFxSummary = FString::Printf(
		TEXT("CombatFx %s: Spawned=%s, Data=%s, Niagara=%s, ScaleMode=%s, Scale=(%.3f, %.3f, %.3f), UserParameter=%s, MaximumLifetime=%.2fs, Location=(%.1f, %.1f, %.1f)"),
		FxRoleText,
		bSpawned ? TEXT("True") : TEXT("False"),
		*InCombatFxData->GetName(),
		InCombatFxData->NiagaraSystem ? *InCombatFxData->NiagaraSystem->GetName() : TEXT("Missing"),
		ScaleModeText,
		SafeScale.X,
		SafeScale.Y,
		SafeScale.Z,
		*InCombatFxData->NiagaraUserScaleParameterName.ToString(),
		InCombatFxData->MaximumLifetimeSeconds,
		SpawnLocation.X,
		SpawnLocation.Y,
		SpawnLocation.Z);
	return bSpawned;
}

void UCFCombatFxComp::UnbindVehicleHealthEvents()
{
	if (IsValid(BoundVehicleHealthComp.Get()))
	{
		BoundVehicleHealthComp->OnVehicleDestroyed.RemoveDynamic(this, &UCFCombatFxComp::HandleOwnerVehicleDestroyed);
	}
	BoundVehicleHealthComp = nullptr;
}
