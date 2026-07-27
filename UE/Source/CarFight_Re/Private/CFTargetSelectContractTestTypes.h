// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-23
// Description: TS-P0-01 타겟 선택 런타임 계약 자동화 테스트 전용 타입
// Scope: 선택 가능 Actor와 Dynamic Multicast Delegate 호출 횟수 Probe를 제공합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetSelectable.h"
#include "GameFramework/Actor.h"
#include "UObject/Object.h"
#include "CFTargetSelectContractTestTypes.generated.h"

/**
 * TS-P0-01 필터와 상태 전이 검증에 사용할 최소 선택 가능 Actor입니다.
 */
UCLASS(Transient, NotBlueprintable)
class ACFTargetSelectContractActor : public AActor, public ICFTargetSelectable
{
	GENERATED_BODY()

public:
	ACFTargetSelectContractActor();

	virtual bool IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const override;
	virtual FCFTargetDisplayInfo GetTargetDisplayInfo_Implementation() const override;
	virtual FVector GetTargetSelectionLocation_Implementation() const override;
	virtual ECFTargetTrackState GetTargetTrackState_Implementation() const override;

	bool bSelectable = true;
	FCFTargetDisplayInfo DisplayInfo;
	FVector SelectionLocationOffset = FVector::ZeroVector;
	ECFTargetTrackState TrackState = ECFTargetTrackState::Visible;
};

/**
 * TargetSelectComp Dynamic Multicast Delegate의 실제 Broadcast 횟수를 기록하는 테스트 Probe입니다.
 */
UCLASS(Transient, NotBlueprintable)
class UCFTargetSelectContractProbe : public UObject
{
	GENERATED_BODY()

public:
	void ResetCounts();

	UFUNCTION()
	void HandleCandidateChanged(AActor* PreviousCandidate, AActor* NewCandidate, FCFTargetCandidate CandidateData);

	UFUNCTION()
	void HandleSelectedTargetChanged(AActor* PreviousTarget, AActor* NewTarget, FCFTargetDisplayInfo DisplayInfo);

	UFUNCTION()
	void HandleSelectedTargetCleared(AActor* ClearedTarget, ECFTargetClearReason ClearReason);

	UFUNCTION()
	void HandleSelectedTargetValidityChanged(AActor* TargetActor, bool bIsValidTarget);

	UFUNCTION()
	void HandleSelectedTargetTrackChanged(AActor* TargetActor, ECFTargetTrackState PreviousState, ECFTargetTrackState NewState);

	int32 CandidateChangedCount = 0;
	int32 SelectedTargetChangedCount = 0;
	int32 SelectedTargetClearedCount = 0;
	int32 SelectedTargetValidityChangedCount = 0;
	int32 SelectedTargetTrackChangedCount = 0;
};
