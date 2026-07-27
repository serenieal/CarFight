// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-24
// Description: 선택 대상을 장비가 사용할 수 있는지 평가하는 공용 계약
// Scope: 장비별 대상 정책, 평가 요청, 사용 가능 결과와 실패 사유를 제공합니다.
// Changelog:
// - v1.0.0: TS-P0-07 선택 대상 조회와 장비 호환성·거리·실패 사유 계약을 추가.
// Migration:
// - 선택 상태는 UCFTargetSelectComp가 계속 소유하며 장비는 FCFTargetUseRequest로 읽기 전용 평가만 요청한다.
// - 빈 허용 목록은 제한 없음으로 해석하고 MaxUseDistanceCm가 0 이하이면 거리 제한을 사용하지 않는다.
// - 선택 대상 존재와 장비 사용 가능 상태는 서로 다른 bool 값으로 유지한다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetSelectTypes.h"
#include "CFTargetUseTypes.generated.h"

class AActor;

/**
 * 선택 대상이 장비 사용 조건을 만족하지 못한 구체적인 사유입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="타겟 사용 실패 사유 (Target Use Failure Reason)", ToolTip="장비가 현재 선택 대상을 사용할 수 없는 이유를 나타냅니다."))
enum class ECFTargetUseFailureReason : uint8
{
	None UMETA(DisplayName="사용 가능 (None)"),
	TargetSystemUnavailable UMETA(DisplayName="타겟 시스템 없음 (Target System Unavailable)"),
	EquipmentUnavailable UMETA(DisplayName="장비 사용 불가 (Equipment Unavailable)"),
	NoSelectedTarget UMETA(DisplayName="선택 대상 없음 (No Selected Target)"),
	SelectedTargetInvalid UMETA(DisplayName="선택 대상 무효 (Selected Target Invalid)"),
	CategoryNotAllowed UMETA(DisplayName="대상 분류 비호환 (Category Not Allowed)"),
	RelationNotAllowed UMETA(DisplayName="대상 관계 비호환 (Relation Not Allowed)"),
	MissingRequiredTag UMETA(DisplayName="필수 속성 없음 (Missing Required Tag)"),
	ExcludedByTag UMETA(DisplayName="제외 속성 대상 (Excluded By Tag)"),
	TrackStateNotAllowed UMETA(DisplayName="추적 상태 비호환 (Track State Not Allowed)"),
	OutOfRange UMETA(DisplayName="사용 거리 초과 (Out Of Range)")
};

/**
 * 장비가 사용할 수 있는 대상의 분류·관계·속성·추적 상태 정책입니다.
 * 각 배열이 비어 있으면 해당 항목은 제한하지 않습니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="장비 타겟 사용 정책 (Target Use Policy)", ToolTip="장비별 허용 대상 분류, 관계, 속성 태그와 추적 상태 조건을 정의합니다."))
struct FCFTargetUsePolicy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Policy", meta=(DisplayName="허용 대상 분류 (AllowedCategories)", ToolTip="장비가 사용할 수 있는 대상 분류 목록입니다. 비어 있으면 모든 분류를 허용합니다."))
	TArray<ECFTargetCategory> AllowedCategories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Policy", meta=(DisplayName="허용 대상 관계 (AllowedRelations)", ToolTip="장비가 사용할 수 있는 대상 관계 목록입니다. 비어 있으면 모든 관계를 허용합니다."))
	TArray<ECFTargetRelation> AllowedRelations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Policy", meta=(DisplayName="필수 대상 속성 태그 (RequiredAttributeTags)", ToolTip="선택 대상이 모두 가지고 있어야 하는 속성 태그입니다."))
	TArray<FName> RequiredAttributeTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Policy", meta=(DisplayName="제외 대상 속성 태그 (ExcludedAttributeTags)", ToolTip="선택 대상이 하나라도 가지고 있으면 장비 사용을 거부할 속성 태그입니다."))
	TArray<FName> ExcludedAttributeTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Policy", meta=(DisplayName="허용 추적 상태 (AllowedTrackStates)", ToolTip="장비가 사용할 수 있는 선택 대상 추적 상태입니다. 비어 있으면 Invalid를 제외한 모든 상태를 허용합니다."))
	TArray<ECFTargetTrackState> AllowedTrackStates;
};

/**
 * 장비가 현재 선택 대상을 평가하기 위해 TargetSelectComp에 전달하는 읽기 전용 요청입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="장비 타겟 사용 요청 (Target Use Request)", ToolTip="장비 ID, 준비 상태, 사용 거리와 대상 정책을 전달합니다."))
struct FCFTargetUseRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Request", meta=(DisplayName="장비 ID (EquipmentId)", ToolTip="평가를 요청한 무기 또는 유틸리티 장비의 안정적인 식별자입니다."))
	FName EquipmentId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Request", meta=(DisplayName="장비 준비 완료 여부 (bEquipmentReady)", ToolTip="False이면 선택 대상 상태와 관계없이 EquipmentUnavailable 결과를 반환합니다."))
	bool bEquipmentReady = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Request", meta=(ClampMin="0.0", Units="cm", DisplayName="최대 사용 거리 (MaxUseDistanceCm)", ToolTip="장비가 선택 대상에 사용할 수 있는 최대 거리입니다. 0이면 거리 제한을 사용하지 않습니다."))
	float MaxUseDistanceCm = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Request", meta=(DisplayName="명시적 사용 원점 사용 (bUseExplicitOrigin)", ToolTip="True이면 Owner Actor 위치 대신 ExplicitUseOrigin을 거리 계산 원점으로 사용합니다."))
	bool bUseExplicitOrigin = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Request", meta=(DisplayName="명시적 사용 원점 (ExplicitUseOrigin)", ToolTip="무기 총구나 유틸리티 장비 위치처럼 거리 계산에 사용할 명시적 월드 원점입니다."))
	FVector ExplicitUseOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetUse|Request", meta=(DisplayName="대상 사용 정책 (TargetPolicy)", ToolTip="대상 분류, 관계, 속성 태그와 추적 상태 호환 정책입니다."))
	FCFTargetUsePolicy TargetPolicy;
};

/**
 * 장비가 현재 선택 대상을 사용할 수 있는지 단계별로 분리한 평가 결과입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="장비 타겟 사용 결과 (Target Use Result)", ToolTip="선택 존재, 유효성, 호환성, 거리와 최종 사용 가능 여부를 개별 상태로 제공합니다."))
struct FCFTargetUseResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="장비 ID (EquipmentId)"))
	FName EquipmentId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="평가 대상 Actor (TargetActor)"))
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="대상 표시 정보 (DisplayInfo)"))
	FCFTargetDisplayInfo DisplayInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="대상 추적 상태 (TrackState)"))
	ECFTargetTrackState TrackState = ECFTargetTrackState::Invalid;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(Units="cm", DisplayName="대상 거리 (DistanceCm)"))
	float DistanceCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(Units="cm", DisplayName="최대 사용 거리 (MaxUseDistanceCm)"))
	float MaxUseDistanceCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="장비 준비 완료 여부 (bEquipmentReady)"))
	bool bEquipmentReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="선택 기록 존재 여부 (bHasSelectedTarget)"))
	bool bHasSelectedTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="선택 대상 유효 여부 (bSelectedTargetValid)"))
	bool bSelectedTargetValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="대상 호환 여부 (bTargetCompatible)"))
	bool bTargetCompatible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="사용 거리 충족 여부 (bWithinUseDistance)"))
	bool bWithinUseDistance = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="선택 대상 사용 가능 여부 (bCanUseTarget)"))
	bool bCanUseTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="사용 실패 사유 (FailureReason)"))
	ECFTargetUseFailureReason FailureReason = ECFTargetUseFailureReason::TargetSystemUnavailable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="실패 관련 속성 태그 (FailureAttributeTag)", ToolTip="MissingRequiredTag 또는 ExcludedByTag 원인이 된 속성 태그입니다."))
	FName FailureAttributeTag = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetUse|Result", meta=(DisplayName="사용 결과 메시지 (ResultMessage)"))
	FText ResultMessage;
};
