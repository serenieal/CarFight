// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-04-27
// Description: VehicleDebug Panel Navigation 항목용 C++ 부모 위젯 구현입니다.
// Scope: Navigation 항목 표시, 선택 상태 스타일, 선택 요청 전달을 담당합니다.

#include "UI/CFVehicleDebugNavItemWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/CFVehicleDebugPanelWidget.h"

void UCFVehicleDebugNavItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Root)
	{
		Button_Root->OnClicked.RemoveDynamic(this, &UCFVehicleDebugNavItemWidget::HandleRootButtonClicked);
		Button_Root->OnClicked.AddDynamic(this, &UCFVehicleDebugNavItemWidget::HandleRootButtonClicked);
	}

	RefreshFromNavItemViewData();
}

void UCFVehicleDebugNavItemWidget::SetOwningPanelWidget(UCFVehicleDebugPanelWidget* InOwningPanelWidget)
{
	// [v1.0.0] Navigation 선택 요청을 받을 Panel 위젯 약참조를 저장합니다.
	OwningPanelWidget = InOwningPanelWidget;
}

void UCFVehicleDebugNavItemWidget::SetNavItemViewData(const FCFVehicleDebugNavItemViewData& InNavItemViewData)
{
	// [v1.0.0] 표시와 선택 요청에 사용할 Navigation Item ViewData를 캐시합니다.
	CachedNavItemViewData = InNavItemViewData;

	RefreshFromNavItemViewData();
}

void UCFVehicleDebugNavItemWidget::RefreshFromNavItemViewData()
{
	// [v1.1.0] 현재 선택 상태에 따라 사용할 배경색입니다.
	const FLinearColor CurrentBackgroundColor = CachedNavItemViewData.bIsSelected ? SelectedBackgroundColor : DefaultBackgroundColor;

	// [v1.1.0] 현재 선택 상태에 따라 사용할 텍스트 색입니다.
	const FLinearColor CurrentTextColor = CachedNavItemViewData.bIsSelected ? SelectedTextColor : DefaultTextColor;

	if (Button_Root)
	{
		Button_Root->SetBackgroundColor(CurrentBackgroundColor);
		Button_Root->SetColorAndOpacity(CurrentTextColor);
	}

	if (Text_Display)
	{
		Text_Display->SetText(FText::FromString(CachedNavItemViewData.DisplayText));
		Text_Display->SetColorAndOpacity(FSlateColor(CurrentTextColor));
	}

	if (Text_Badge)
	{
		Text_Badge->SetText(FText::FromString(CachedNavItemViewData.BadgeText));
		Text_Badge->SetColorAndOpacity(FSlateColor(BadgeTextColor));
		Text_Badge->SetVisibility(CachedNavItemViewData.BadgeText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UCFVehicleDebugNavItemWidget::RequestSelectThisNavItem()
{
	// [v1.0.0] 유효한 Panel이 없거나 SectionId가 비어 있으면 선택 요청을 중단합니다.
	if (!OwningPanelWidget.IsValid() || CachedNavItemViewData.SectionId.IsEmpty())
	{
		return;
	}

	OwningPanelWidget->SelectSectionById(CachedNavItemViewData.SectionId);
}

void UCFVehicleDebugNavItemWidget::HandleRootButtonClicked()
{
	RequestSelectThisNavItem();
}
