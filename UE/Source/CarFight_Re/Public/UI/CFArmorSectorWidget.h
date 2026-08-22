// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-20
// Description: CF-FQ-032 VehiclePanel의 재사용 가능한 한 방향 Armor Sector 표시 Widget
// Scope: 방향 Label, Armor Plate Image와 실제 Armor Ratio ProgressBar를 한 의미 단위로 묶고 Gameplay 조회 없이 Presentation 값만 적용합니다.
// Changelog:
// - v1.0.0: 방향 Label·Plate Texture·Designer Preview 비율 구성과 Runtime Armor Percent 적용 API를 최초 추가.
// Migration:
// - WBP_CFArmorBodyMap은 방향별 Image/Text/ProgressBar를 직접 소유하지 않고 WBP_CFArmorSector 6개를 공간 배치합니다.
// - Armor 값 계산과 6방향 의미는 기존 Defense ViewData/Presenter가 계속 소유하며 이 Widget은 Gameplay를 조회하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "UI/CFStyledWidgetBase.h"
#include "CFArmorSectorWidget.generated.h"

class UTexture2D;

/**
 * 차량 한 방향의 Armor Plate, 방향 Label과 실제 잔여 Armor Bar를 하나로 묶는 재사용 Presentation Widget입니다.
 */
UCLASS(Blueprintable)
class CARFIGHT_RE_API UCFArmorSectorWidget : public UCFStyledWidgetBase
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 방향 Label과 Designer Preview Armor 비율을 준비합니다.
	UCFArmorSectorWidget(const FObjectInitializer& ObjectInitializer);

	// [v1.0.0] Editor Bridge가 한 방향 Sector 인스턴스의 정적 Label·Plate Texture·Designer Preview 비율을 구성합니다.
	void ConfigureSector(const FText& InDirectionLabel, UTexture2D* InArmorPlateTexture, float InDesignerArmorPercent);

	// [v1.0.0] Presenter가 실제 0~1 Armor 비율과 Bar 표시 여부를 현재 Sector에 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Armor", meta=(DisplayName="장갑 비율 설정 (Set Armor Percent)", ToolTip="이 Armor Sector의 실제 잔여 장갑 비율을 0~1로 적용합니다. Gameplay 값을 계산하지 않고 Presenter가 전달한 비율만 표시합니다."))
	void SetArmorPercent(float InArmorPercent, bool bVisible = true);

	// [v1.0.0] Editor Validator가 이 Sector 인스턴스에 저장된 방향 Label을 확인할 때 사용합니다.
	const FText& GetConfiguredDirectionLabel() const { return DirectionLabel; }

	// [v1.0.0] Editor Validator가 이 Sector 인스턴스에 저장된 Armor Plate Texture를 확인할 때 사용합니다.
	UTexture2D* GetConfiguredArmorPlateTexture() const { return ArmorPlateTexture.Get(); }

protected:
	// [v1.0.0] Designer와 Runtime 생성 시 저장된 Sector 설정을 내부 Image/Text/ProgressBar에 반영합니다.
	virtual void NativePreConstruct() override;

private:
	// [v1.0.0] 현재 저장된 Label·Texture·Preview Armor 비율을 고정 의미 자식 Widget에 적용합니다.
	void ApplyConfiguredVisuals();

	// [v1.0.0] 현재 Sector가 표시하는 차량 로컬 방향 Label입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", DisplayName="방향 라벨", ToolTip="이 Sector가 나타내는 차량 로컬 방향입니다. 예: FRONT, RIGHT, REAR, LEFT, TOP, BOTTOM."))
	FText DirectionLabel;

	// [v1.0.0] 현재 Sector의 방향별 Armor Plate 전용 Texture입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", DisplayName="장갑 Plate Texture", ToolTip="이 Sector의 방향별 Armor Plate Image에 표시할 HUD 전용 Texture입니다."))
	TObjectPtr<UTexture2D> ArmorPlateTexture = nullptr;

	// [v1.0.0] 저장 Asset의 Designer에서 Bar 형태를 확인하기 위한 0~1 Preview Armor 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Armor", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0", DisplayName="Designer 장갑 비율", ToolTip="Designer 미리보기에서만 기본으로 사용할 장갑 비율입니다. 실제 플레이 중에는 Presenter가 Runtime Armor 비율로 덮어씁니다."))
	float DesignerArmorPercent = 1.0f;
};
