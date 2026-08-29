// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.1
// Date: 2026-08-25
// Description: CF-FQ-039 modular Armor Sector Production Visual Widget
// Scope: 공통 Plate + 별도 방향 Icon/Label fallback + 실제 Armor Ratio Bar를 한 의미 단위로 묶고 legacy 방향-baked Plate 호환을 유지합니다.
// Changelog:
// - v1.2.1: 재사용 WBP_CFArmorSector 6개가 같은 클래스여도 각 Designer 인스턴스에서 방향 Icon 회전을 독립 설정할 수 있도록 DirectionIconRotationDegrees를 추가. C++은 Image_DirectionIcon의 Render Transform Angle만 적용하고 BodyMap Slot 위치·크기는 변경하지 않음.
// - v1.2.0: common Plate + 별도 Image_DirectionIcon 구조용 ConfigureModularSector/DirectionIconTexture를 additive 추가. 기존 ConfigureSector는 direction-baked legacy Plate 호환 wrapper로 유지하고 현재 WBP에 DirectionIcon Image가 아직 없어도 Label fallback이 안전하게 동작하도록 분리.
// - v1.1.1: SetArmorPercent가 headless/nested Widget lifecycle에서도 방향 Texture 존재 시 fallback Direction Label을 숨기도록 icon-first visibility를 재보장합니다.
// - v1.1.0: 방향 Texture 우선/Label fallback 계약과 Ratio 기반 Stable/Caution/Critical Presentation tint를 추가. 기존 ConfigureSector/SetArmorPercent API는 유지.
// - v1.0.0: 방향 Label·Plate Texture·Designer Preview 비율 구성과 Runtime Armor Percent 적용 API를 최초 추가.
// Migration:
// - WBP_CFArmorBodyMap은 방향별 Image/Text/ProgressBar를 직접 소유하지 않고 WBP_CFArmorSector 6개를 공간 배치합니다.
// - Armor 값 계산과 6방향 의미는 기존 Defense ViewData/Presenter가 계속 소유하며 이 Widget은 Gameplay를 조회하지 않습니다.
// - P2 방향 Texture가 존재하면 Label은 fallback이므로 숨고, Texture가 없을 때만 DirectionLabel을 표시합니다.
// - v1.2.0 legacy ConfigureSector는 기존 Plate 안에 방향 glyph가 baked된 것으로 간주합니다. 새 ConfigureModularSector는 Image_DirectionIcon이 실제 존재하고 Icon Texture가 설정된 경우에만 Text_Direction을 숨깁니다.

#pragma once

#include "CoreMinimal.h"
#include "UI/CFStyledWidgetBase.h"
#include "CFArmorSectorWidget.generated.h"

class UTexture2D;

/**
 * 차량 한 방향의 Armor 방향 Icon/Label fallback, Plate와 실제 잔여 Armor Bar를 하나로 묶는 재사용 Production Presentation Widget입니다.
 */
UCLASS(Blueprintable)
class CARFIGHT_RE_API UCFArmorSectorWidget : public UCFStyledWidgetBase
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 방향 Label과 Designer Preview Armor 비율을 준비합니다.
	UCFArmorSectorWidget(const FObjectInitializer& ObjectInitializer);

	// [v1.0.0] Legacy Editor Bridge가 방향 glyph가 포함된 완성 Plate Texture와 Designer Preview 비율을 구성합니다.
	void ConfigureSector(const FText& InDirectionLabel, UTexture2D* InArmorPlateTexture, float InDesignerArmorPercent);

	// [v1.2.0] Modular Armor Editor/Runtime bridge가 공통 Plate와 별도 방향 Icon Texture를 구성합니다. Icon 회전은 UMG Designer Render Transform이 소유합니다.
	void ConfigureModularSector(const FText& InDirectionLabel, UTexture2D* InCommonArmorPlateTexture, UTexture2D* InDirectionIconTexture, float InDesignerArmorPercent);

	// [v1.1.0] Presenter가 실제 0~1 Armor 비율과 Bar 표시 여부를 현재 Sector의 Ratio와 Presentation 상태색에 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Armor", meta=(DisplayName="장갑 비율 설정 (Set Armor Percent)", ToolTip="이 Armor Sector의 실제 잔여 장갑 비율을 0~1로 적용합니다. Gameplay 값을 계산하지 않고 Presenter가 전달한 비율만 표시합니다."))
	void SetArmorPercent(float InArmorPercent, bool bVisible = true);

	// [v1.0.0] Editor Validator가 이 Sector 인스턴스에 저장된 방향 Label을 확인할 때 사용합니다.
	const FText& GetConfiguredDirectionLabel() const { return DirectionLabel; }

	// [v1.0.0] Editor Validator가 이 Sector 인스턴스에 저장된 Armor Plate Texture를 확인할 때 사용합니다.
	UTexture2D* GetConfiguredArmorPlateTexture() const { return ArmorPlateTexture.Get(); }

	// [v1.2.0] Editor Validator가 modular Sector의 별도 Direction Icon Texture를 확인할 때 사용합니다.
	UTexture2D* GetConfiguredDirectionIconTexture() const { return DirectionIconTexture.Get(); }

protected:
	// [v1.0.0] Designer와 Runtime 생성 시 저장된 Sector 설정을 내부 Image/Text/ProgressBar에 반영합니다.
	virtual void NativePreConstruct() override;

private:
	// [v1.2.0] 현재 저장된 Label·Plate·Direction Icon·Preview Armor 비율을 고정 의미 자식 Widget에 적용합니다.
	void ApplyConfiguredVisuals();

	// [v1.2.0] 현재 저장 설정과 실제 WBP 자식 상태를 기준으로 Direction Text를 숨길 수 있는 시각 표현이 존재하는지 확인합니다.
	bool HasRenderableDirectionVisual() const;

	// [v1.0.0] 현재 Sector가 표시하는 차량 로컬 방향 Label입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", DisplayName="방향 라벨", ToolTip="이 Sector가 나타내는 차량 로컬 방향입니다. 예: FRONT, RIGHT, REAR, LEFT, TOP, BOTTOM."))
	FText DirectionLabel;

	// [v1.2.0] 현재 Sector의 공통 또는 legacy 방향-baked Armor Plate Texture입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", DisplayName="장갑 Plate Texture", ToolTip="Modular 구조에서는 모든 Sector가 공유하는 공통 mechanical Plate이며 legacy Content에서는 방향 glyph가 포함된 완성 Plate일 수 있습니다."))
	TObjectPtr<UTexture2D> ArmorPlateTexture = nullptr;

	// [v1.2.0] 공통 Plate 위 별도 Image_DirectionIcon에 표시할 Arrow 또는 Chevron2 Texture입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", DisplayName="방향 Icon Texture", ToolTip="공통 Plate와 분리된 방향 Icon Texture입니다. Arrow 또는 Chevron2를 사용합니다."))
	TObjectPtr<UTexture2D> DirectionIconTexture = nullptr;

	// [v1.2.1] 같은 WBP_CFArmorSector 클래스를 재사용하는 각 Designer 인스턴스가 독립적으로 소유할 방향 Icon 회전각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", Units="deg", UIMin="0.0", UIMax="360.0", DisplayName="방향 Icon 회전 각도", ToolTip="이 Armor Sector 인스턴스의 Image_DirectionIcon 회전 각도입니다. Arrow/Chevron2 원본은 오른쪽을 향하며 각 Sector에서 0/90/180/270도처럼 직접 조정합니다. Sector 위치와 크기는 ArmorBodyMap Designer가 계속 소유합니다."))
	float DirectionIconRotationDegrees = 0.0f;

	// [v1.2.0] 기존 ConfigureSector가 방향 glyph가 baked된 legacy 완성 Plate를 사용 중인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", DisplayName="Legacy 방향 Plate 사용", ToolTip="True이면 기존 Plate Texture 자체가 방향 표시를 포함하므로 별도 Image_DirectionIcon이 없어도 Text_Direction fallback을 숨깁니다."))
	bool bLegacyDirectionBakedIntoPlate = false;

	// [v1.0.0] 저장 Asset의 Designer에서 Bar 형태를 확인하기 위한 0~1 Preview Armor 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0", DisplayName="Designer 장갑 비율", ToolTip="Designer 미리보기에서만 기본으로 사용할 장갑 비율입니다. 실제 플레이 중에는 Presenter가 Runtime Armor 비율로 덮어씁니다."))
	float DesignerArmorPercent = 1.0f;
};
