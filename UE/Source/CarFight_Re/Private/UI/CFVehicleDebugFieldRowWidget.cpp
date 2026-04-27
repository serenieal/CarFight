// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-04-27
// Description: VehicleDebug Panel의 공통 Field Row 렌더링용 C++ 부모 위젯 구현입니다.
// Scope: Field ViewData를 받아 라벨/값 한 줄 표시를 공통 구조로 갱신합니다.

#include "UI/CFVehicleDebugFieldRowWidget.h"

#include "Components/TextBlock.h"

void UCFVehicleDebugFieldRowWidget::SetFieldViewData(const FCFVehicleDebugFieldViewData& InFieldViewData)
{
	// [v1.0.0] 현재 Row 캐시를 최신 Field ViewData로 갱신합니다.
	CachedFieldViewData = InFieldViewData;

	RefreshFromFieldViewData();
}

void UCFVehicleDebugFieldRowWidget::RefreshFromFieldViewData()
{
	// [v1.0.0] 현재 Field를 표시할지 여부입니다.
	const bool bShouldShowField = CachedFieldViewData.bIsVisible;

	SetVisibility(bShouldShowField ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (!bShouldShowField)
	{
		return;
	}

	// [v1.0.0] Row 전체에 적용할 툴팁 텍스트입니다.
	const FText TooltipText = FText::FromString(CachedFieldViewData.TooltipText);

	if (!CachedFieldViewData.TooltipText.IsEmpty())
	{
		SetToolTipText(TooltipText);
	}

	if (Text_Label)
	{
		Text_Label->SetText(FText::FromString(CachedFieldViewData.LabelText));
		Text_Label->SetVisibility(CachedFieldViewData.LabelText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}

	if (Text_Value)
	{
		Text_Value->SetText(FText::FromString(CachedFieldViewData.ValueText));
		Text_Value->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (Text_Combined)
	{
		Text_Combined->SetText(FText::FromString(BuildCombinedFieldText()));
		Text_Combined->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

const FCFVehicleDebugFieldViewData& UCFVehicleDebugFieldRowWidget::GetCachedFieldViewData() const
{
	return CachedFieldViewData;
}

void UCFVehicleDebugFieldRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshFromFieldViewData();
}

FString UCFVehicleDebugFieldRowWidget::BuildCombinedFieldText() const
{
	if (CachedFieldViewData.LabelText.IsEmpty())
	{
		return CachedFieldViewData.ValueText;
	}

	return FString::Printf(TEXT("%s: %s"), *CachedFieldViewData.LabelText, *CachedFieldViewData.ValueText);
}
