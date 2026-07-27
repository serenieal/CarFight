// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-23
// Description: CarFight 타겟 선택 설정 DataAsset
// Scope: 타겟 후보 검색과 선택 안정화에 사용할 설정, 유효성 검사와 디버그 요약을 제공합니다.
// Changelog:
// - v1.0.0: TS-P0-01용 UPrimaryDataAsset 설정 클래스와 검증 API를 추가.
// Migration:
// - 이번 단계에서는 DataAsset 클래스만 추가하며 .uasset 인스턴스는 생성하지 않는다.
// - TargetSelectComp는 유효한 DataAsset이 없으면 자체 Fallback 설정을 사용한다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetSelectTypes.h"
#include "Engine/DataAsset.h"
#include "CFTargetSelectData.generated.h"

/**
 * 타겟 선택 시스템의 공통 튜닝값을 제공하는 DataAsset입니다.
 */
UCLASS(BlueprintType, meta=(DisplayName="타겟 선택 데이터 (Target Select Data)", ToolTip="직접 선택 거리, 근접 후보 범위, 갱신 주기와 선택 안정화 설정을 제공합니다."))
class CARFIGHT_RE_API UCFTargetSelectData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 타겟 선택 설정값이 유한하고 허용 범위 안에 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Data", meta=(DisplayName="타겟 선택 설정 유효 여부 (Is Target Select Config Valid)", ToolTip="거리, 각도, 갱신 간격, 가림 시간과 전환 비율이 유한하고 허용 범위 안에 있는지 반환합니다."))
	bool IsTargetSelectConfigValid() const;

	// [v1.0.0] 디버그 패널과 로그에 사용할 타겟 선택 설정 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Data", meta=(DisplayName="타겟 선택 설정 요약 생성 (Build Target Select Summary)", ToolTip="현재 타겟 선택 설정값과 유효성 검사 결과를 한 줄 요약 문자열로 생성합니다."))
	FString BuildTargetSelectSummary() const;

	// [v1.0.0] 후보 검색과 선택 안정화에 사용할 공통 타겟 선택 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Data", meta=(DisplayName="타겟 선택 설정 (TargetSelectConfig)", ToolTip="직접 선택 거리, 근접 후보 범위, 갱신 주기, 가림 유예와 후보 전환 우위 비율을 정의합니다."))
	FCFTargetSelectConfig TargetSelectConfig;
};
