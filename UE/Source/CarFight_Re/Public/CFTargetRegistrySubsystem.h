// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-31
// Description: TargetSelect 후보 공급용 월드 단위 Targetable Actor Registry Subsystem
// Changelog:
// - v1.0.0: ICFTargetSelectable Actor를 월드 시작 1회 bootstrap, 이후 Actor spawn 및 level 추가 이벤트로 등록하고 weak reference로 수명 관리.
// Migration:
// - TargetSelect의 직접 조준, LOS, 후보 평가와 Sensor 책임은 변경하지 않습니다.
// - 기존 매 후보 갱신 TActorIterator 전체 월드 순회는 이 Registry snapshot 조회로 대체합니다.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CFTargetRegistrySubsystem.generated.h"

class AActor;
class ULevel;
class UWorld;

/**
 * 현재 World의 ICFTargetSelectable Actor만 약한 참조로 보관해 TargetSelect 후보 수집 비용을 제한합니다.
 */
UCLASS()
class CARFIGHT_RE_API UCFTargetRegistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Registry delegate를 연결하고 World 수명에 맞는 후보 공급 준비를 시작합니다.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Registry delegate와 약한 Actor 캐시를 정리합니다.
	virtual void Deinitialize() override;

	// BeginPlay 시점에 기존 World Actor를 정확히 한 번 bootstrap합니다.
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// 현재 유효한 Targetable Actor snapshot을 반환하고 무효 weak reference를 함께 정리합니다.
	void CollectTargetableActors(TArray<AActor*>& OutTargetableActors);

private:
	// 기존 World Actor를 한 번만 순회해 초기 Targetable 집합을 채웁니다.
	void EnsureInitialScan();

	// 단일 Actor가 현재 World의 TargetSelectable 계약을 구현하면 Registry에 추가합니다.
	void RegisterActorIfTargetable(AActor* TargetActor);

	// 현재 World에 새 Actor가 spawn되면 TargetSelectable 여부를 검사해 등록합니다.
	void HandleActorSpawned(AActor* SpawnedActor);

	// 현재 World에 Level이 추가되면 해당 Level Actor만 검사해 streaming 대상 누락을 방지합니다.
	void HandleLevelAddedToWorld(ULevel* AddedLevel, UWorld* AddedWorld);

	// 파괴·제거된 Actor의 무효 weak reference를 Registry에서 제거합니다.
	void PruneInvalidActors();

	// 현재 World의 TargetSelectable Actor를 GC 강한 참조 없이 보관하는 집합입니다.
	TSet<TWeakObjectPtr<AActor>> RegisteredTargetActors;

	// 현재 World의 Actor spawn 알림 구독을 해제하기 위한 delegate handle입니다.
	FDelegateHandle ActorSpawnedDelegateHandle;

	// 현재 World의 Level 추가 알림 구독을 해제하기 위한 delegate handle입니다.
	FDelegateHandle LevelAddedToWorldDelegateHandle;

	// 기존 World Actor bootstrap 전체 순회를 이미 수행했는지 나타냅니다.
	bool bInitialScanCompleted = false;
};
