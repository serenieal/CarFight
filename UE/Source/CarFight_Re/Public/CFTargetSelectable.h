// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-22
// Description: CarFight 선택 가능 대상 인터페이스 / Player-facing Identity fail-closed 계약
// Scope: 대상 선택 가능 여부, 표시 정보, 대표 선택 위치와 추적 상태를 제공하는 최소 Blueprint 계약을 정의합니다.
// Changelog:
// - v1.1.0: 기본 TargetDisplayInfo가 Actor UObject 이름을 TargetId/DisplayName으로 자동 공개하지 않는 fail-closed 계약을 명시. 안정 ID/Player-facing 이름은 구현체가 명시적으로 제공해야 함.
// - v1.0.1: BlueprintNativeEvent 기본 구현 함수를 명시적으로 선언하여 생성 코드와의 중복 정의를 방지.
// - v1.0.0: TS-P0-01용 BlueprintNativeEvent 선택 가능 인터페이스를 추가.
// Migration:
// - 선택 대상 Actor 또는 Blueprint는 이 인터페이스를 구현해야 TargetSelectComp에서 사용할 수 있다.
// - v1.1.0부터 기본 표시 정보는 TargetId=None, DisplayName=Empty를 유지합니다. 저장/로그/UI에 사용할 안정 ID와 Player-facing 이름이 필요한 구현체는 명시적으로 제공해야 합니다.
// - 기본 구현은 유효 Actor를 선택 가능 대상으로 보고 TargetPoint/Bounds 중심을 선택 위치 fallback으로 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetSelectTypes.h"
#include "UObject/Interface.h"
#include "CFTargetSelectable.generated.h"

/**
 * Blueprint와 C++ 대상이 구현할 타겟 선택 가능 인터페이스 UObject 타입입니다.
 */
UINTERFACE(BlueprintType, meta=(DisplayName="타겟 선택 가능 (Target Selectable)", ToolTip="대상이 선택 가능 여부, 표시 정보, 대표 위치와 추적 상태를 제공하도록 하는 인터페이스입니다."))
class CARFIGHT_RE_API UCFTargetSelectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 타겟 선택 시스템이 대상 Actor에서 읽을 최소 계약입니다.
 */
class CARFIGHT_RE_API ICFTargetSelectable
{
	GENERATED_BODY()

public:
	// [v1.0.0] 주어진 선택 컨텍스트에서 이 대상을 선택할 수 있는지 반환합니다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="CarFight|TargetSelect|Selectable", meta=(DisplayName="타겟 선택 가능 여부 (Is Target Selectable)", ToolTip="주어진 선택 컨텍스트에서 이 대상을 선택할 수 있는지 반환합니다. 기본 C++ 구현은 유효한 Actor이면 True를 반환합니다."))
	bool IsTargetSelectable(const FCFTargetSelectionContext& SelectionContext) const;

	// [v1.0.1] C++ 또는 Blueprint가 재정의하지 않았을 때 호출할 기본 선택 가능 여부 구현입니다.
	virtual bool IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const;

		// [v1.1.0] UI와 장비 시스템이 사용할 대상 표시 정보를 반환하며 명시 Identity가 없으면 내부 UObject 이름을 공개하지 않습니다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="CarFight|TargetSelect|Selectable", meta=(DisplayName="타겟 표시 정보 반환 (Get Target Display Info)", ToolTip="타겟 ID, 표시 이름, 분류, 관계, 정보 단계와 속성 태그를 반환합니다. 기본 C++ 구현은 TargetId와 표시 이름을 비운 채 미분류/미확인 의미만 제공합니다."))
	FCFTargetDisplayInfo GetTargetDisplayInfo() const;

		// [v1.1.0] C++ 또는 Blueprint가 재정의하지 않았을 때 TargetId/DisplayName을 비워 내부 UObject 이름 노출을 막는 기본 표시 정보 구현입니다.
	virtual FCFTargetDisplayInfo GetTargetDisplayInfo_Implementation() const;

	// [v1.0.0] 후보 평가와 UI 투영에 사용할 대표 선택 월드 위치를 반환합니다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="CarFight|TargetSelect|Selectable", meta=(DisplayName="타겟 선택 위치 반환 (Get Target Selection Location)", ToolTip="후보 평가와 UI 투영에 사용할 대표 월드 위치를 반환합니다. 기본 C++ 구현은 Actor Bounds 중심을 사용합니다."))
	FVector GetTargetSelectionLocation() const;

	// [v1.0.1] C++ 또는 Blueprint가 재정의하지 않았을 때 호출할 기본 선택 위치 구현입니다.
	virtual FVector GetTargetSelectionLocation_Implementation() const;

	// [v1.0.0] 대상의 현재 추적 품질 상태를 반환합니다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="CarFight|TargetSelect|Selectable", meta=(DisplayName="타겟 추적 상태 반환 (Get Target Track State)", ToolTip="대상이 가시, 가림, 추정 추적 또는 신호 손실 상태인지 반환합니다. 기본 C++ 구현은 Visible입니다."))
	ECFTargetTrackState GetTargetTrackState() const;

	// [v1.0.1] C++ 또는 Blueprint가 재정의하지 않았을 때 호출할 기본 추적 상태 구현입니다.
	virtual ECFTargetTrackState GetTargetTrackState_Implementation() const;
};
