// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-31
// Description: TargetSelect 후보 공급용 월드 단위 Targetable Actor Registry 구현
// Changelog:
// - v1.0.0: 초기 1회 scan + spawn/level-add 증분 등록 + weak reference prune를 구현.
// Migration:
// - 런타임 후보 갱신은 전체 World Actor를 반복 순회하지 않고 이 Registry의 Targetable Actor만 소비합니다.

#include "CFTargetRegistrySubsystem.h"

#include "CFTargetSelectable.h"

#include "Engine/Level.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// Registry delegate를 연결하고 World 수명에 맞는 후보 공급 준비를 시작합니다.
void UCFTargetRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 이 Subsystem이 소유하는 현재 World입니다.
	UWorld* RegistryWorld = GetWorld();
	if (!RegistryWorld)
	{
		return;
	}

	// 이후 spawn되는 Actor를 증분 등록하기 위한 World delegate handle입니다.
	ActorSpawnedDelegateHandle = RegistryWorld->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(this, &UCFTargetRegistrySubsystem::HandleActorSpawned));

	// streaming 등으로 Level이 추가될 때 해당 Level Actor만 검사하기 위한 global delegate handle입니다.
	LevelAddedToWorldDelegateHandle = FWorldDelegates::LevelAddedToWorld.AddUObject(
		this,
		&UCFTargetRegistrySubsystem::HandleLevelAddedToWorld);
}

// Registry delegate와 약한 Actor 캐시를 정리합니다.
void UCFTargetRegistrySubsystem::Deinitialize()
{
	// 이 Subsystem이 소유했던 현재 World입니다.
	UWorld* RegistryWorld = GetWorld();
	if (RegistryWorld && ActorSpawnedDelegateHandle.IsValid())
	{
		RegistryWorld->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);
	}

	if (LevelAddedToWorldDelegateHandle.IsValid())
	{
		FWorldDelegates::LevelAddedToWorld.Remove(LevelAddedToWorldDelegateHandle);
	}

	RegisteredTargetActors.Reset();
	ActorSpawnedDelegateHandle.Reset();
	LevelAddedToWorldDelegateHandle.Reset();
	bInitialScanCompleted = false;

	Super::Deinitialize();
}

// BeginPlay 시점에 기존 World Actor를 정확히 한 번 bootstrap합니다.
void UCFTargetRegistrySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (&InWorld == GetWorld())
	{
		EnsureInitialScan();
	}
}

// 현재 유효한 Targetable Actor snapshot을 반환하고 무효 weak reference를 함께 정리합니다.
void UCFTargetRegistrySubsystem::CollectTargetableActors(TArray<AActor*>& OutTargetableActors)
{
	EnsureInitialScan();
	PruneInvalidActors();

	OutTargetableActors.Reset();
	OutTargetableActors.Reserve(RegisteredTargetActors.Num());

	for (const TWeakObjectPtr<AActor>& WeakTargetActor : RegisteredTargetActors)
	{
		// Registry snapshot에 반환할 현재 유효 Actor입니다.
		AActor* TargetActor = WeakTargetActor.Get();
		if (IsValid(TargetActor))
		{
			OutTargetableActors.Add(TargetActor);
		}
	}
}

// 기존 World Actor를 한 번만 순회해 초기 Targetable 집합을 채웁니다.
void UCFTargetRegistrySubsystem::EnsureInitialScan()
{
	if (bInitialScanCompleted)
	{
		return;
	}

	// 초기 scan의 대상이 되는 현재 World입니다.
	UWorld* RegistryWorld = GetWorld();
	if (!RegistryWorld)
	{
		return;
	}

	bInitialScanCompleted = true;

	for (TActorIterator<AActor> ActorIterator(RegistryWorld); ActorIterator; ++ActorIterator)
	{
		RegisterActorIfTargetable(*ActorIterator);
	}
}

// 단일 Actor가 현재 World의 TargetSelectable 계약을 구현하면 Registry에 추가합니다.
void UCFTargetRegistrySubsystem::RegisterActorIfTargetable(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || TargetActor->GetWorld() != GetWorld())
	{
		return;
	}

	// Actor class가 Native 또는 Blueprint로 TargetSelectable 인터페이스를 구현하는지 여부입니다.
	const bool bImplementsTargetSelectable = TargetActor->GetClass()->ImplementsInterface(UCFTargetSelectable::StaticClass());
	if (!bImplementsTargetSelectable)
	{
		return;
	}

	RegisteredTargetActors.Add(TargetActor);
}

// 현재 World에 새 Actor가 spawn되면 TargetSelectable 여부를 검사해 등록합니다.
void UCFTargetRegistrySubsystem::HandleActorSpawned(AActor* SpawnedActor)
{
	RegisterActorIfTargetable(SpawnedActor);
}

// 현재 World에 Level이 추가되면 해당 Level Actor만 검사해 streaming 대상 누락을 방지합니다.
void UCFTargetRegistrySubsystem::HandleLevelAddedToWorld(ULevel* AddedLevel, UWorld* AddedWorld)
{
	if (!AddedLevel || AddedWorld != GetWorld())
	{
		return;
	}

	for (AActor* LevelActor : AddedLevel->Actors)
	{
		RegisterActorIfTargetable(LevelActor);
	}
}

// 파괴·제거된 Actor의 무효 weak reference를 Registry에서 제거합니다.
void UCFTargetRegistrySubsystem::PruneInvalidActors()
{
	for (auto TargetActorIterator = RegisteredTargetActors.CreateIterator(); TargetActorIterator; ++TargetActorIterator)
	{
		if (!TargetActorIterator->IsValid())
		{
			TargetActorIterator.RemoveCurrent();
		}
	}
}
