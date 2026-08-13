// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-13
// Description: CF-FQ-032 D1-11 Production HUD 의미 단위 UMG Editor Bridge
// Scope: Root 1 + Panel 6 + Element 2 Widget Blueprint를 Image/Text/ProgressBar 중심 구조로 생성·검증합니다.
// Changelog:
// - v1.2.0: 전용 RPM Track Texture가 없는 Image 슬롯을 Collapsed로 보존하고 빈 Brush 가시 회귀를 Production Validator에서 차단.
// - v1.1.0: 최신 VehiclePanel 계약에 맞춘 SpeedGauge RPM Preview와 Armor 세로 Bar 회귀 검증을 Production Widget Validate 계약에 반영.
// - v1.0.0: Production Child Widget Build/Validate와 Root Composition Build/Validate bool-only Python Bridge를 최초 추가.
// Migration:
// - 기존 CFUIHUDEditorBridge v1.x Border/Canvas Mock 경로는 Historical Preview evidence로 보존합니다.
// - Production Bridge는 Gameplay 조회, Runtime Event Binding, 직접 콘텐츠 경로 하드코딩을 만들지 않습니다.
// - D1-11 Designer Preview는 실제 Runtime RPM/Gear Provider를 선행 구현하지 않으며 가짜 전진 기어와 RPM 비율을 생성하지 않습니다.
// - HUD 전용 RPM Track 자산이 비어 있어도 Image_RPMTrackArt 슬롯은 유지되지만 렌더링되지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CFUIHUDProdEditorBridge.generated.h"

class UCFHUDLayoutData;
class UCFHUDVisualData;
class UCFUIDensityData;
class UCFUIStyleData;

/**
 * D1-11 Production UMG 구조를 역할별 Widget Blueprint와 Root Composition으로 만드는 Editor 전용 Bridge입니다.
 */
UCLASS()
class CARFIGHT_RE_API UCFUIHUDProdEditorBridge : public UObject
{
	GENERATED_BODY()

public:
	// [v1.0.0] 지정 Production Widget 역할의 Designer Tree를 의미 단위 UMG 구조로 생성합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Widget 생성 (Build Production HUD Widget)", ToolTip="SpeedGauge, ArmorBodyMap, MissionPanel, AlertFeed, TargetPanel, VehiclePanel, RadarPanel, WeaponPanel 역할의 Production Widget Tree를 생성합니다."))
	static bool BuildProductionWidgetResult(
		UObject* WidgetBlueprintObject,
		FName WidgetRole,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UCFHUDVisualData* HUDVisualData,
		UObject* SpeedGaugeBlueprintObject,
		UObject* ArmorBodyMapBlueprintObject);

	// [v1.0.0] 저장된 Production Widget 역할의 Tree·Image·Border 제한·Graph 0 계약을 검증합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Widget 검증 (Validate Production HUD Widget)", ToolTip="Production Widget의 역할별 Tree, Image 기반 시각 슬롯, Border Mosaic 금지와 Runtime Graph 0 계약을 검증합니다."))
	static bool ValidateProductionWidgetResult(
		UObject* WidgetBlueprintObject,
		FName WidgetRole,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UCFHUDVisualData* HUDVisualData,
		UObject* SpeedGaugeBlueprintObject,
		UObject* ArmorBodyMapBlueprintObject);

	// [v1.0.0] 여섯 Production Panel Generated Class를 D1-07 승인 7 Slot Root Canvas에 조립합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Root 생성 (Build Production HUD Root)", ToolTip="여섯 의미 Panel Widget을 D1-07 승인 1920x1080 Slot Layout에 배치하고 Reticle Layer를 유지하는 WBP_CFInGameHUD Root를 생성합니다."))
	static bool BuildProductionRootResult(
		UObject* RootWidgetBlueprintObject,
		UCFHUDLayoutData* LayoutData,
		UObject* MissionPanelBlueprintObject,
		UObject* AlertFeedBlueprintObject,
		UObject* TargetPanelBlueprintObject,
		UObject* VehiclePanelBlueprintObject,
		UObject* RadarPanelBlueprintObject,
		UObject* WeaponPanelBlueprintObject);

	// [v1.0.0] 저장된 Production HUD Root의 7 Slot·Panel Class·Layout·Graph 0 계약을 검증합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Root 검증 (Validate Production HUD Root)", ToolTip="WBP_CFInGameHUD Root가 정확한 7 Slot과 여섯 Production Panel Class를 사용하고 Runtime Graph를 만들지 않았는지 검증합니다."))
	static bool ValidateProductionRootResult(
		UObject* RootWidgetBlueprintObject,
		UCFHUDLayoutData* LayoutData,
		UObject* MissionPanelBlueprintObject,
		UObject* AlertFeedBlueprintObject,
		UObject* TargetPanelBlueprintObject,
		UObject* VehiclePanelBlueprintObject,
		UObject* RadarPanelBlueprintObject,
		UObject* WeaponPanelBlueprintObject);
};
