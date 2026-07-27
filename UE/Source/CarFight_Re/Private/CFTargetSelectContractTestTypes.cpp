// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-23
// Description: TS-P0-01 타겟 선택 런타임 계약 자동화 테스트 전용 타입 구현

#include "CFTargetSelectContractTestTypes.h"

#include "Components/SceneComponent.h"

ACFTargetSelectContractActor::ACFTargetSelectContractActor()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetActorEnableCollision(false);
}

bool ACFTargetSelectContractActor::IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const
{
	(void)SelectionContext;
	return bSelectable;
}

FCFTargetDisplayInfo ACFTargetSelectContractActor::GetTargetDisplayInfo_Implementation() const
{
	return DisplayInfo;
}

FVector ACFTargetSelectContractActor::GetTargetSelectionLocation_Implementation() const
{
	return GetActorLocation() + SelectionLocationOffset;
}

ECFTargetTrackState ACFTargetSelectContractActor::GetTargetTrackState_Implementation() const
{
	return TrackState;
}

void UCFTargetSelectContractProbe::ResetCounts()
{
	CandidateChangedCount = 0;
	SelectedTargetChangedCount = 0;
	SelectedTargetClearedCount = 0;
	SelectedTargetValidityChangedCount = 0;
	SelectedTargetTrackChangedCount = 0;
}

void UCFTargetSelectContractProbe::HandleCandidateChanged(AActor* PreviousCandidate, AActor* NewCandidate, FCFTargetCandidate CandidateData)
{
	(void)PreviousCandidate;
	(void)NewCandidate;
	(void)CandidateData;
	++CandidateChangedCount;
}

void UCFTargetSelectContractProbe::HandleSelectedTargetChanged(AActor* PreviousTarget, AActor* NewTarget, FCFTargetDisplayInfo DisplayInfo)
{
	(void)PreviousTarget;
	(void)NewTarget;
	(void)DisplayInfo;
	++SelectedTargetChangedCount;
}

void UCFTargetSelectContractProbe::HandleSelectedTargetCleared(AActor* ClearedTarget, ECFTargetClearReason ClearReason)
{
	(void)ClearedTarget;
	(void)ClearReason;
	++SelectedTargetClearedCount;
}

void UCFTargetSelectContractProbe::HandleSelectedTargetValidityChanged(AActor* TargetActor, bool bIsValidTarget)
{
	(void)TargetActor;
	(void)bIsValidTarget;
	++SelectedTargetValidityChangedCount;
}

void UCFTargetSelectContractProbe::HandleSelectedTargetTrackChanged(AActor* TargetActor, ECFTargetTrackState PreviousState, ECFTargetTrackState NewState)
{
	(void)TargetActor;
	(void)PreviousState;
	(void)NewState;
	++SelectedTargetTrackChangedCount;
}
