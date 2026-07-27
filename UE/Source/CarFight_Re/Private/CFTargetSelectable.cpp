// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-24
// Description: CarFight 선택 가능 대상 인터페이스 기본 구현
// Scope: 유효 Actor 판정, Actor 이름 기반 표시 정보, TargetPoint 공용 Fallback과 기본 가시 추적 상태를 제공합니다.
// Changelog:
// - v1.1.0: 선택 위치 기본 구현을 UCFTargetPointComp의 TargetPoint, Actor Bounds, Actor Location 공용 해석으로 전환.
// - v1.0.1: deprecated _getUObject 호출을 인터페이스 지원 Cast로 교체하고 const 일관성을 적용.
// - v1.0.0: TS-P0-01 선택 가능 인터페이스의 안전 기본 구현을 추가.
// Migration:
// - Blueprint 또는 C++ 구현체가 개별 함수를 재정의하지 않으면 이 기본값을 사용한다.

#include "CFTargetSelectable.h"

#include "CFTargetPointComp.h"
#include "GameFramework/Actor.h"

// [v1.0.0] 주어진 선택 컨텍스트에서 이 대상을 선택할 수 있는지 반환합니다.
bool ICFTargetSelectable::IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const
{
	// [v1.0.0] 기본 구현에서 아직 직접 해석하지 않는 선택 컨텍스트입니다.
	(void)SelectionContext;

	// [v1.0.1] 인터페이스를 구현한 실제 Actor입니다.
	const AActor* TargetActor = Cast<const AActor>(this);

	return IsValid(TargetActor);
}

// [v1.0.0] UI와 장비 시스템이 사용할 대상 표시 정보를 반환합니다.
FCFTargetDisplayInfo ICFTargetSelectable::GetTargetDisplayInfo_Implementation() const
{
	// [v1.0.0] 기본값으로 반환할 대상 표시 정보입니다.
	FCFTargetDisplayInfo DisplayInfo;

	// [v1.0.1] 인터페이스를 구현한 실제 Actor입니다.
	const AActor* TargetActor = Cast<const AActor>(this);
	if (!IsValid(TargetActor))
	{
		DisplayInfo.InformationLevel = ECFTargetInfoLevel::None;
		return DisplayInfo;
	}

	DisplayInfo.TargetId = TargetActor->GetFName();
	DisplayInfo.DisplayName = FText::FromString(TargetActor->GetName());
	DisplayInfo.TargetCategory = ECFTargetCategory::Unknown;
	DisplayInfo.Relation = ECFTargetRelation::Unknown;
	DisplayInfo.InformationLevel = ECFTargetInfoLevel::Detected;
	return DisplayInfo;
}

// [v1.0.0] 후보 평가와 UI 투영에 사용할 대표 선택 월드 위치를 반환합니다.
FVector ICFTargetSelectable::GetTargetSelectionLocation_Implementation() const
{
	// [v1.0.1] 인터페이스를 구현한 실제 Actor입니다.
	const AActor* TargetActor = Cast<const AActor>(this);
	if (!IsValid(TargetActor))
	{
		return FVector::ZeroVector;
	}

	// [v1.0.0] Actor Bounds 중심 월드 위치입니다.
		return UCFTargetPointComp::ResolveTargetPoint(TargetActor).WorldLocation;
}

// [v1.0.0] 대상의 현재 추적 품질 상태를 반환합니다.
ECFTargetTrackState ICFTargetSelectable::GetTargetTrackState_Implementation() const
{
	// [v1.0.1] 인터페이스를 구현한 실제 Actor입니다.
	const AActor* TargetActor = Cast<const AActor>(this);

	return IsValid(TargetActor) ? ECFTargetTrackState::Visible : ECFTargetTrackState::Invalid;
}
