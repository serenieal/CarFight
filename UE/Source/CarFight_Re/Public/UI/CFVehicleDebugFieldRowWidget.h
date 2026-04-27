// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-04-27
// Description: VehicleDebug Panel의 공통 Field Row 렌더링용 C++ 부모 위젯입니다.
// Scope: Field ViewData를 받아 라벨/값 한 줄 표시를 공통 구조로 갱신합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CFDebugPanelViewData.h"
#include "CFVehicleDebugFieldRowWidget.generated.h"

class UTextBlock;

/**
 * VehicleDebug Panel의 공통 필드 Row 렌더링용 C++ 부모 위젯입니다.
 * - 일반 `라벨: 값` 표시를 섹션 본문 문자열에서 분리합니다.
 * - WBP 자식은 Text_Label / Text_Value 또는 Text_Combined 중 필요한 방식을 선택할 수 있습니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFVehicleDebugFieldRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] 현재 Field ViewData를 설정하고 즉시 Row 표시를 갱신합니다.
	void SetFieldViewData(const FCFVehicleDebugFieldViewData& InFieldViewData);

	// [v1.0.0] 현재 캐시된 Field ViewData 기준으로 라벨/값 표시를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|FieldRow", meta=(DisplayName="Field Row 갱신 (RefreshFromFieldViewData)", ToolTip="현재 캐시된 Field ViewData 기준으로 라벨/값 텍스트를 갱신합니다."))
	void RefreshFromFieldViewData();

	// [v1.0.0] 현재 캐시된 Field ViewData를 반환합니다.
	const FCFVehicleDebugFieldViewData& GetCachedFieldViewData() const;

protected:
	// [v1.0.0] 위젯 생성 직후 현재 캐시 기준으로 첫 Row 갱신을 수행합니다.
	virtual void NativeConstruct() override;

	// [v1.0.0] Field 라벨 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Label = nullptr;

	// [v1.0.0] Field 값 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Value = nullptr;

	// [v1.0.0] 라벨과 값을 한 줄로 합쳐 표시할 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Combined = nullptr;

private:
	// [v1.0.0] 현재 Row에 적용 중인 Field ViewData 캐시입니다.
	FCFVehicleDebugFieldViewData CachedFieldViewData;

	// [v1.0.0] 현재 Field ViewData를 한 줄 문자열로 변환합니다.
	FString BuildCombinedFieldText() const;
};
