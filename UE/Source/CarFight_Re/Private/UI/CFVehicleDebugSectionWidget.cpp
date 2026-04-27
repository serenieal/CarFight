// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-04-27
// Description: VehicleDebug Panel의 공통 Section 렌더링용 C++ 부모 위젯 구현입니다.
// Scope: Section ViewData를 받아 제목/본문/하위 섹션 표시를 공통 구조로 갱신하는 기반 위젯입니다.

#include "UI/CFVehicleDebugSectionWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "UI/CFVehicleDebugFieldRowWidget.h"

void UCFVehicleDebugSectionWidget::SetSectionViewData(const FCFVehicleDebugSectionViewData& InSectionViewData)
{
	// [v1.0.1] 이번에 적용할 섹션이 이전 섹션과 같은지 여부입니다.
	const bool bIsSameSectionId = LastAppliedSectionId.Equals(InSectionViewData.SectionId, ESearchCase::CaseSensitive);

	// [v1.0.0] 현재 섹션 캐시를 최신 ViewData로 갱신합니다.
	CachedSectionViewData = InSectionViewData;

	// [v1.0.1] 다른 섹션이 들어온 경우에만 기본 펼침 상태를 다시 반영합니다.
	if (!bIsSameSectionId)
	{
		bIsSectionExpanded = CachedSectionViewData.bDefaultExpanded;
	}

	// [v1.0.1] 마지막 적용 섹션 식별자를 현재 섹션으로 갱신합니다.
	LastAppliedSectionId = CachedSectionViewData.SectionId;

	RefreshFromSectionViewData();
}

void UCFVehicleDebugSectionWidget::RefreshFromSectionViewData()
{
	// [v1.0.0] 현재 섹션 본문 문자열입니다.
	const FString SectionBodyText = BuildSectionBodyText();

	// [v1.0.0] Section 제목 텍스트가 있으면 현재 헤더 문자열을 반영합니다.
	UpdateSectionHeaderText();

	// [v1.0.0] Section 본문 텍스트가 있으면 현재 본문 문자열을 반영합니다.
	if (Text_Body)
	{
		Text_Body->SetText(FText::FromString(SectionBodyText));
		Text_Body->SetVisibility(SectionBodyText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}

	EnsureFieldRowWidgets();
	RefreshFieldRowWidgets();
	EnsureChildSectionWidgets();
	RefreshChildSectionWidgets();
	UpdateSectionBodyVisibility();
	SetVisibility(CachedSectionViewData.bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCFVehicleDebugSectionWidget::ToggleSectionExpanded()
{
	// [v1.0.0] 현재 섹션 펼침 상태를 반전합니다.
	bIsSectionExpanded = !bIsSectionExpanded;

	UpdateSectionHeaderText();
	UpdateSectionBodyVisibility();
}

const FCFVehicleDebugSectionViewData& UCFVehicleDebugSectionWidget::GetCachedSectionViewData() const
{
	return CachedSectionViewData;
}

void UCFVehicleDebugSectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshFromSectionViewData();
}

FString UCFVehicleDebugSectionWidget::BuildSectionBodyText() const
{
	// [v1.0.0] 조합 중인 최종 본문 문자열입니다.
	FString SectionBodyText = CachedSectionViewData.BodyText;

	if (IsFieldRowLayoutReady())
	{
		return SectionBodyText;
	}

	for (const FCFVehicleDebugFieldViewData& FieldViewData : CachedSectionViewData.FieldArray)
	{
		if (!FieldViewData.bIsVisible)
		{
			continue;
		}

		// [v1.0.0] 현재 필드의 한 줄 문자열입니다.
		const FString FieldLineText = FieldViewData.LabelText.IsEmpty()
			? FieldViewData.ValueText
			: FString::Printf(TEXT("%s: %s"), *FieldViewData.LabelText, *FieldViewData.ValueText);

		if (!SectionBodyText.IsEmpty())
		{
			SectionBodyText.Append(TEXT("\n"));
		}

		SectionBodyText.Append(FieldLineText);
	}

	return SectionBodyText;
}

void UCFVehicleDebugSectionWidget::UpdateSectionHeaderText()
{
	// [v1.0.0] 현재 펼침 상태를 반영한 헤더 문자열입니다.
	const FString HeaderText = FString::Printf(TEXT("%s %s"), bIsSectionExpanded ? TEXT("[-]") : TEXT("[+]"), *CachedSectionViewData.TitleText);

	if (Text_Title)
	{
		Text_Title->SetText(FText::FromString(HeaderText));
	}
}

void UCFVehicleDebugSectionWidget::UpdateSectionBodyVisibility()
{
	// [v1.0.0] 현재 섹션 본문을 표시할지 여부입니다.
	const bool bShouldShowBody = bIsSectionExpanded && CachedSectionViewData.bIsVisible;

	if (Container_Body)
	{
		Container_Body->SetVisibility(bShouldShowBody ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (VerticalBox_FieldHost)
	{
		VerticalBox_FieldHost->SetVisibility((bShouldShowBody && IsFieldRowLayoutReady() && CachedSectionViewData.FieldArray.Num() > 0) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (VerticalBox_ChildSectionHost)
	{
		VerticalBox_ChildSectionHost->SetVisibility(bShouldShowBody ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

bool UCFVehicleDebugSectionWidget::IsFieldRowLayoutReady() const
{
	return VerticalBox_FieldHost != nullptr && FieldRowWidgetClass != nullptr;
}

void UCFVehicleDebugSectionWidget::EnsureFieldRowWidgets()
{
	// [v1.1.0] Field Row 레이아웃 준비가 안 되었으면 생성을 중단합니다.
	if (!IsFieldRowLayoutReady())
	{
		return;
	}

	// [v1.1.0] 현재 필요한 Field Row 개수입니다.
	const int32 RequiredFieldRowCount = CachedSectionViewData.FieldArray.Num();

	if (FieldRowWidgetArray.Num() == RequiredFieldRowCount)
	{
		return;
	}

	VerticalBox_FieldHost->ClearChildren();
	FieldRowWidgetArray.Reset();

	for (int32 FieldIndex = 0; FieldIndex < RequiredFieldRowCount; ++FieldIndex)
	{
		// [v1.1.0] 새로 생성할 Field Row 위젯 인스턴스입니다.
		UCFVehicleDebugFieldRowWidget* FieldRowWidget = nullptr;

		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			FieldRowWidget = CreateWidget<UCFVehicleDebugFieldRowWidget>(OwningPlayerController, FieldRowWidgetClass);
		}
		else if (WidgetTree)
		{
			FieldRowWidget = CreateWidget<UCFVehicleDebugFieldRowWidget>(WidgetTree, FieldRowWidgetClass);
		}

		if (!FieldRowWidget)
		{
			continue;
		}

		FieldRowWidgetArray.Add(FieldRowWidget);
		VerticalBox_FieldHost->AddChildToVerticalBox(FieldRowWidget);
	}
}

void UCFVehicleDebugSectionWidget::RefreshFieldRowWidgets()
{
	if (!IsFieldRowLayoutReady())
	{
		return;
	}

	for (int32 FieldIndex = 0; FieldIndex < FieldRowWidgetArray.Num(); ++FieldIndex)
	{
		// [v1.1.0] 현재 갱신할 Field Row 위젯 인스턴스입니다.
		UCFVehicleDebugFieldRowWidget* FieldRowWidget = FieldRowWidgetArray[FieldIndex];

		if (!FieldRowWidget || !CachedSectionViewData.FieldArray.IsValidIndex(FieldIndex))
		{
			continue;
		}

		FieldRowWidget->SetFieldViewData(CachedSectionViewData.FieldArray[FieldIndex]);
	}
}

void UCFVehicleDebugSectionWidget::EnsureChildSectionWidgets()
{
	// [v1.0.0] 하위 섹션 호스트가 없으면 준비를 생략합니다.
	if (!VerticalBox_ChildSectionHost)
	{
		return;
	}

	// [v1.1.1] 하위 섹션 생성에 사용할 위젯 클래스입니다.
	const TSubclassOf<UCFVehicleDebugSectionWidget> ResolvedChildSectionWidgetClass = ResolveChildSectionWidgetClass();

	// [v1.1.1] 하위 섹션 위젯 클래스가 없으면 준비를 생략합니다.
	if (!ResolvedChildSectionWidgetClass)
	{
		return;
	}

	// [v1.0.0] 현재 필요한 하위 섹션 개수입니다.
	const int32 RequiredChildSectionCount = CachedSectionViewData.ChildSectionArray.Num();

	if (ChildSectionWidgetArray.Num() == RequiredChildSectionCount)
	{
		return;
	}

	VerticalBox_ChildSectionHost->ClearChildren();
	ChildSectionWidgetArray.Reset();

	for (int32 ChildSectionIndex = 0; ChildSectionIndex < RequiredChildSectionCount; ++ChildSectionIndex)
	{
		// [v1.0.0] 새로 생성할 하위 섹션 위젯 인스턴스입니다.
		UCFVehicleDebugSectionWidget* ChildSectionWidget = nullptr;

		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			ChildSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(OwningPlayerController, ResolvedChildSectionWidgetClass);
		}
		else if (WidgetTree)
		{
			ChildSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(WidgetTree, ResolvedChildSectionWidgetClass);
		}

		if (!ChildSectionWidget)
		{
			continue;
		}

		ChildSectionWidgetArray.Add(ChildSectionWidget);

		// [v1.2.0] 하위 섹션임을 시각적으로 구분하기 위한 슬롯 여백입니다.
		UVerticalBoxSlot* ChildSectionSlot = VerticalBox_ChildSectionHost->AddChildToVerticalBox(ChildSectionWidget);
		if (ChildSectionSlot)
		{
			ChildSectionSlot->SetPadding(FMargin(ChildSectionIndentLeft, ChildSectionTopPadding, 0.0f, 0.0f));
		}
	}
}

TSubclassOf<UCFVehicleDebugSectionWidget> UCFVehicleDebugSectionWidget::ResolveChildSectionWidgetClass() const
{
	if (ChildSectionWidgetClass)
	{
		return ChildSectionWidgetClass;
	}

	return GetClass();
}

void UCFVehicleDebugSectionWidget::RefreshChildSectionWidgets()
{
	for (int32 ChildSectionIndex = 0; ChildSectionIndex < ChildSectionWidgetArray.Num(); ++ChildSectionIndex)
	{
		// [v1.0.0] 현재 갱신할 하위 섹션 위젯 인스턴스입니다.
		UCFVehicleDebugSectionWidget* ChildSectionWidget = ChildSectionWidgetArray[ChildSectionIndex];

		// [v1.0.0] 현재 위젯에 대응하는 하위 섹션 ViewData입니다.
		const TSharedPtr<FCFVehicleDebugSectionViewData> ChildSectionViewData =
			CachedSectionViewData.ChildSectionArray.IsValidIndex(ChildSectionIndex)
			? CachedSectionViewData.ChildSectionArray[ChildSectionIndex]
			: nullptr;

		if (!ChildSectionWidget || !ChildSectionViewData.IsValid())
		{
			continue;
		}

		ChildSectionWidget->SetSectionViewData(*ChildSectionViewData);
	}
}
