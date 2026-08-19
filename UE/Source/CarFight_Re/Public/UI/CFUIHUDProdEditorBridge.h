// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-08-19
// Description: CF-FQ-032 UI-P0-06 truthful Weapon Rail + Dynamic Resource Visual Stage B + RPM Gauge Runtime Sink Production HUD Editor Bridge
// Scope: Root 1 + Panel 6 + Element 2 구조를 유지하면서 WeaponPanel Rail을 가짜 semantic icon 3개에서 이름 기반 fixed Text Tile 3개로 교체하고 기존 Compact Resource/RPM 구조를 보존합니다.
// Changelog:
// - v1.5.0: WeaponPanel Rail의 Turret/Ammo/Reload placeholder Image를 제거하고 112x68 fixed Text Tile 3개 + 8px gap으로 교체. Designer default는 Collapsed, Runtime Presenter만 실제 비선택 Weapon Selection이 있을 때 표시.
// - v1.4.0: SpeedGauge의 기존 ProgressBar_RPMTick00~20을 Presenter가 runtime Percent sink로 재사용하는 계약을 명시. Designer Preview 구조/개수/배치와 Asset 구조는 변경하지 않음.
// - v1.3.0: WeaponPanel Stage B. Launcher/Ammo/Heat/Cooldown 구형 고정 Row를 새 Compact Resource Presentation 의미 슬롯으로 교체하고 Validator가 중복 legacy slot을 차단하도록 계약 갱신.
// - v1.2.0: 전용 RPM Track Texture가 없는 Image 슬롯을 Collapsed로 보존하고 빈 Brush 가시 회귀를 Production Validator에서 차단.
// - v1.1.0: 최신 VehiclePanel 계약에 맞춘 SpeedGauge RPM Preview와 Armor 세로 Bar 회귀 검증을 Production Widget Validate 계약에 반영.
// - v1.0.0: Production Child Widget Build/Validate와 Root Composition Build/Validate bool-only Python Bridge를 최초 추가.
// Migration:
// - 기존 CFUIHUDEditorBridge v1.x Border/Canvas Mock 경로는 Historical Preview evidence로 보존합니다.
// - Production Bridge는 Gameplay 조회, Runtime Event Binding, 직접 콘텐츠 경로 하드코딩을 만들지 않습니다.
// - D1-11 Designer Preview는 실제 Runtime RPM/Gear Provider를 선행 구현하지 않으며 가짜 전진 기어와 RPM 비율을 생성하지 않습니다. 저장 Tick Percent=1은 Scale Preview이며 Runtime Presenter가 explicit Redline 계약에 따라 덮어씁니다.
// - HUD 전용 RPM Track 자산이 비어 있어도 Image_RPMTrackArt 슬롯은 유지되지만 렌더링되지 않습니다.
// - WeaponPanel은 raw ResourceChannels 개수대로 Widget을 생성하지 않으며 Primary/SecondaryA/SecondaryB/FireState 고정 의미 슬롯만 생성합니다.
// - ReserveAmmo는 Header 우측 label-less owner를 유지하고 VehicleBattery·WeaponCharge의 현재 Runtime 부재를 가짜 Preview Row로 보완하지 않습니다. Heat는 기존 actual Runtime Projection만 사용합니다.
// - v1.5.0 Rail은 실제 weapon icon source가 생기기 전 Image를 만들지 않습니다. 비선택 weapon resource summary도 현재 ViewData에 없으므로 Designer Tile은 Text slot만 소유합니다.

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
			// [v1.5.0] 지정 Production Widget 역할의 Designer Tree를 의미 단위 UMG 구조로 생성하며 WeaponPanel은 Compact Resource와 truthful Text Rail 슬롯 계약을 사용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Widget 생성 (Build Production HUD Widget)", ToolTip="Production Widget Tree를 의미 단위로 생성합니다. WeaponPanel은 Header Reserve, Primary 1, Secondary 최대 2, FireState 1과 비선택 무기용 Text Rail 3슬롯을 만들며 가짜 weapon icon을 생성하지 않습니다."))
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

			// [v1.5.0] 저장 Production Widget의 Tree·Image·Border·Graph 0과 WeaponPanel Compact Resource + truthful Text Rail 계약을 검증합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Widget 검증 (Validate Production HUD Widget)", ToolTip="WeaponPanel은 Compact Resource 슬롯과 이름 기반 Rail 3슬롯 존재, 구형 Resource Row 및 Turret/Ammo/Reload Rail Image 부재, Rail Designer 기본 Collapsed를 확인합니다."))
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
