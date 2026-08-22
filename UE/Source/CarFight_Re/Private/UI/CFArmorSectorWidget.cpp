// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-20
// Description: UCFArmorSectorWidget 재사용 Armor Sector Presentation 구현
// Scope: 저장된 방향 Label·Armor Plate Texture·Designer Preview와 Presenter의 실제 Armor Percent를 내부 의미 Widget에 적용합니다.
// Changelog:
// - v1.0.0: ConfigureSector, NativePreConstruct, SetArmorPercent와 고정 자식 이름 기반 Visual 적용을 최초 구현.
// Migration:
// - Gameplay/Defense 계산은 추가하지 않으며 WBP_CFArmorSector의 Image_ArmorPlate, Text_Direction, ProgressBar_Armor 이름을 고정 소비합니다.

#include "UI/CFArmorSectorWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

// [v1.0.0] 기본 방향 Label과 Designer Preview Armor 비율을 준비합니다.
UCFArmorSectorWidget::UCFArmorSectorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DirectionLabel(FText::FromString(TEXT("ARMOR")))
{
}

// [v1.0.0] Editor Bridge가 한 방향 Sector 인스턴스의 정적 Label·Plate Texture·Designer Preview 비율을 구성합니다.
void UCFArmorSectorWidget::ConfigureSector(
	const FText& InDirectionLabel,
	UTexture2D* InArmorPlateTexture,
	const float InDesignerArmorPercent)
{
#if WITH_EDITOR
	Modify();
#endif
	DirectionLabel = InDirectionLabel;
	ArmorPlateTexture = InArmorPlateTexture;
	DesignerArmorPercent = FMath::Clamp(InDesignerArmorPercent, 0.0f, 1.0f);
	ApplyConfiguredVisuals();
}

// [v1.0.0] Presenter가 실제 0~1 Armor 비율과 Bar 표시 여부를 현재 Sector에 적용합니다.
void UCFArmorSectorWidget::SetArmorPercent(const float InArmorPercent, const bool bVisible)
{
	// [v1.0.0] 이 Sector의 실제 Armor 잔량을 표시하는 고정 ProgressBar입니다.
	UProgressBar* ArmorProgressBar = Cast<UProgressBar>(GetWidgetFromName(FName(TEXT("ProgressBar_Armor"))));
	if (!ArmorProgressBar)
	{
		return;
	}

	ArmorProgressBar->SetPercent(FMath::Clamp(InArmorPercent, 0.0f, 1.0f));
	ArmorProgressBar->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

// [v1.0.0] Designer와 Runtime 생성 시 저장된 Sector 설정을 내부 Image/Text/ProgressBar에 반영합니다.
void UCFArmorSectorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyConfiguredVisuals();
}

// [v1.0.0] 현재 저장된 Label·Texture·Preview Armor 비율을 고정 의미 자식 Widget에 적용합니다.
void UCFArmorSectorWidget::ApplyConfiguredVisuals()
{
	// [v1.0.0] 현재 Sector의 차량 로컬 방향 문자열을 표시하는 고정 TextBlock입니다.
	UTextBlock* DirectionText = Cast<UTextBlock>(GetWidgetFromName(FName(TEXT("Text_Direction"))));
	if (DirectionText)
	{
		DirectionText->SetText(DirectionLabel);
	}

	// [v1.0.0] 현재 Sector의 방향별 Armor Plate Texture를 표시하는 고정 Image입니다.
	UImage* ArmorPlateImage = Cast<UImage>(GetWidgetFromName(FName(TEXT("Image_ArmorPlate"))));
	if (ArmorPlateImage && ArmorPlateTexture)
	{
		ArmorPlateImage->SetBrushFromTexture(ArmorPlateTexture, false);
	}

	SetArmorPercent(DesignerArmorPercent, true);
}
