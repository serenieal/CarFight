// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.1
// Date: 2026-08-25
// Description: UCFArmorSectorWidget modular Armor Sector Production Visual 구현
// Scope: 공통 Plate + 별도 Direction Icon/Label fallback, legacy direction-baked Plate와 실제 Armor Percent 기반 상태 Tint를 안전하게 공존시킵니다.
// Changelog:
// - v1.2.1: 각 WBP_CFArmorSector Designer 인스턴스의 DirectionIconRotationDegrees를 Image_DirectionIcon Render Transform Angle에 적용. ConfigureModularSector는 회전을 덮어쓰지 않아 사용자 배치/회전 ownership을 보존.
// - v1.2.0: ConfigureModularSector와 Image_DirectionIcon 소비를 추가. legacy ConfigureSector는 기존 방향-baked Plate 호환을 유지하고 modular Content는 실제 Icon Image+Texture가 있을 때만 Label을 숨김. Plate/Icon/ProgressBar에 같은 Armor semantic tint 적용.
// - v1.1.1: headless/nested Widget lifecycle에서도 icon-first 계약이 Presenter 적용 직후 확정되도록 SetArmorPercent가 Direction Label visibility를 Texture 존재 여부로 재보장합니다.
// - v1.1.0: 방향별 P2 Texture가 있으면 중복 Direction Label을 숨기고 Texture 누락 시에만 Label fallback을 표시. Armor Ratio를 Stable/Caution/Critical Presentation 색으로 변환해 Plate와 실제 세로 Bar에 함께 적용.
// - v1.0.0: ConfigureSector, NativePreConstruct, SetArmorPercent와 고정 자식 이름 기반 Visual 적용을 최초 구현.
// Migration:
// - Gameplay/Defense 계산은 추가하지 않으며 WBP_CFArmorSector의 Image_ArmorPlate, Text_Direction, ProgressBar_Armor 이름을 고정 소비합니다.
// - Ratio 임계값은 VehiclePanel Spec의 Presentation 상태 계약(Stable >0.60 / Caution >0.30 / Critical <=0.30)을 사용하며 Gameplay 상태를 생성하지 않습니다.
// - v1.2.0 기존 WBP에 Image_DirectionIcon이 아직 없으면 modular 설정은 Text_Direction fallback을 유지합니다. Asset migration 뒤 Icon Image가 생기면 같은 C++ 계약으로 자동 전환됩니다.

#include "UI/CFArmorSectorWidget.h"

#include "UI/CFUIStyleData.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

namespace
{
	// [v1.1.0] 실제 Armor Ratio를 Gameplay 판정 없이 VehiclePanel Presentation 상태색으로 변환합니다.
	FLinearColor ResolveArmorVisualColor(const UCFUIStyleData* StyleData, const float ArmorRatio, const bool bVisible)
	{
		if (!StyleData)
		{
			return FLinearColor::White;
		}

		if (!bVisible)
		{
			return StyleData->ResolveColor(ECFUIColorToken::TextDisabled);
		}

		// [v1.1.0] Presentation 상태 판정에 사용할 0~1 범위 Armor Ratio입니다.
		const float ClampedArmorRatio = FMath::Clamp(ArmorRatio, 0.0f, 1.0f);
		if (ClampedArmorRatio > 0.60f)
		{
			return StyleData->ResolveColor(ECFUIColorToken::Armor);
		}
		if (ClampedArmorRatio > 0.30f)
		{
			return StyleData->ResolveColor(ECFUIColorToken::StateCaution);
		}
		return StyleData->ResolveColor(ECFUIColorToken::StateCritical);
	}
}

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
	DirectionIconTexture = nullptr;
	bLegacyDirectionBakedIntoPlate = InArmorPlateTexture != nullptr;
	DesignerArmorPercent = FMath::Clamp(InDesignerArmorPercent, 0.0f, 1.0f);
	ApplyConfiguredVisuals();
}

// [v1.2.0] Modular Sector의 공통 Plate와 별도 Direction Icon을 구성하고 Icon 회전은 Designer에 남깁니다.
void UCFArmorSectorWidget::ConfigureModularSector(
	const FText& InDirectionLabel,
	UTexture2D* InCommonArmorPlateTexture,
	UTexture2D* InDirectionIconTexture,
	const float InDesignerArmorPercent)
{
#if WITH_EDITOR
	Modify();
#endif
	DirectionLabel = InDirectionLabel;
	ArmorPlateTexture = InCommonArmorPlateTexture;
	DirectionIconTexture = InDirectionIconTexture;
	bLegacyDirectionBakedIntoPlate = false;
	DesignerArmorPercent = FMath::Clamp(InDesignerArmorPercent, 0.0f, 1.0f);
	ApplyConfiguredVisuals();
}

// [v1.1.0] Presenter가 실제 0~1 Armor 비율과 Bar 표시 여부를 현재 Sector의 비율·상태색에 함께 적용합니다.
void UCFArmorSectorWidget::SetArmorPercent(const float InArmorPercent, const bool bVisible)
{
	// [v1.1.0] ProgressBar와 Plate가 함께 사용할 안전한 0~1 Armor Ratio입니다.
	const float ClampedArmorPercent = FMath::Clamp(InArmorPercent, 0.0f, 1.0f);
	// [v1.1.0] 현재 Visual Context와 VehiclePanel Presentation 임계값으로 해석한 Armor 상태색입니다.
	const FLinearColor ArmorVisualColor = ResolveArmorVisualColor(GetUIStyleData(), ClampedArmorPercent, bVisible);

	// [v1.0.0] 이 Sector의 실제 Armor 잔량을 표시하는 고정 ProgressBar입니다.
	UProgressBar* ArmorProgressBar = Cast<UProgressBar>(GetWidgetFromName(FName(TEXT("ProgressBar_Armor"))));
	if (ArmorProgressBar)
	{
		ArmorProgressBar->SetPercent(ClampedArmorPercent);
		ArmorProgressBar->SetFillColorAndOpacity(ArmorVisualColor);
		ArmorProgressBar->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.2.0] 방향과 분리된 공통 mechanical Plate Image입니다. legacy Content에서는 방향 glyph가 포함된 Plate일 수 있습니다.
	UImage* ArmorPlateImage = Cast<UImage>(GetWidgetFromName(FName(TEXT("Image_ArmorPlate"))));
	if (ArmorPlateImage)
	{
		ArmorPlateImage->SetColorAndOpacity(ArmorVisualColor);
	}

	// [v1.2.0] 공통 Plate 위에서 Arrow/Chevron2를 표시하는 별도 Direction Icon Image입니다.
	UImage* DirectionIconImage = Cast<UImage>(GetWidgetFromName(FName(TEXT("Image_DirectionIcon"))));
	if (DirectionIconImage)
	{
		DirectionIconImage->SetColorAndOpacity(ArmorVisualColor);
		DirectionIconImage->SetVisibility(DirectionIconTexture != nullptr && bVisible
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	}

	// [v1.2.0] 실제 Direction visual이 렌더링되지 않을 때만 보이는 fallback Label입니다.
	UTextBlock* DirectionText = Cast<UTextBlock>(GetWidgetFromName(FName(TEXT("Text_Direction"))));
	if (DirectionText)
	{
		DirectionText->SetColorAndOpacity(FSlateColor(ArmorVisualColor));
		DirectionText->SetVisibility(HasRenderableDirectionVisual() && bVisible
			? ESlateVisibility::Collapsed
			: (bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed));
	}
}

// [v1.0.0] Designer와 Runtime 생성 시 저장된 Sector 설정을 내부 Image/Text/ProgressBar에 반영합니다.
void UCFArmorSectorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyConfiguredVisuals();
}

// [v1.2.0] 현재 저장된 Label·Plate·Direction Icon·Preview Armor 비율을 Production Visual에 적용합니다.
void UCFArmorSectorWidget::ApplyConfiguredVisuals()
{
	// [v1.0.0] 현재 Sector의 차량 로컬 방향 문자열을 표시하는 fallback TextBlock입니다.
	UTextBlock* DirectionText = Cast<UTextBlock>(GetWidgetFromName(FName(TEXT("Text_Direction"))));
	if (DirectionText)
	{
		DirectionText->SetText(DirectionLabel);
	}

	// [v1.2.0] 모든 modular Sector가 공유하거나 legacy Content가 그대로 사용할 Armor Plate Image입니다.
	UImage* ArmorPlateImage = Cast<UImage>(GetWidgetFromName(FName(TEXT("Image_ArmorPlate"))));
	if (ArmorPlateImage && ArmorPlateTexture)
	{
		ArmorPlateImage->SetBrushFromTexture(ArmorPlateTexture, false);
	}

	// [v1.2.1] Arrow/Chevron2 Source와 이 Sector 인스턴스의 Designer-owned 회전을 적용할 Direction Icon Image입니다.
	UImage* DirectionIconImage = Cast<UImage>(GetWidgetFromName(FName(TEXT("Image_DirectionIcon"))));
	if (DirectionIconImage)
	{
		DirectionIconImage->SetRenderTransformAngle(DirectionIconRotationDegrees);
		if (DirectionIconTexture)
		{
			DirectionIconImage->SetBrushFromTexture(DirectionIconTexture, false);
		}
	}

	SetArmorPercent(DesignerArmorPercent, true);
}

// [v1.2.0] legacy 방향-baked Plate 또는 실제 별도 Direction Icon Image가 존재해 Label을 숨길 수 있는지 확인합니다.
bool UCFArmorSectorWidget::HasRenderableDirectionVisual() const
{
	if (bLegacyDirectionBakedIntoPlate && ArmorPlateTexture)
	{
		return true;
	}

	UImage* DirectionIconImage = Cast<UImage>(GetWidgetFromName(FName(TEXT("Image_DirectionIcon"))));
	return DirectionIconImage != nullptr && DirectionIconTexture != nullptr;
}
