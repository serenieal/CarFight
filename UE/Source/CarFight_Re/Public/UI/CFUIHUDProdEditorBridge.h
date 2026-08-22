// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.11.0
// Date: 2026-08-21
// Description: CF-FQ-032 Production HUD 의미 구조 검증 + UI-P0-09B View Mode additive migration Editor Bridge
// Scope: 기존 Production Designer Layout을 보존하면서 Radar Visual과 Root ReticleLayer의 Vehicle Direction 의미 Widget만 명시적으로 additive 갱신합니다.
// Changelog:
// - v1.11.0: UI-P0-09B용 View Mode migration을 추가. 기존 Root 7-child 구조와 모든 Panel Slot Layout을 보존하고 빈 ReticleLayer 내부에 Designer-owned Direction Track + Vehicle Semantic Image만 누락 시 추가합니다.
// - v1.10.0: UI-P0-08B용 Radar Visual layout-preserving migration을 추가. 기존 RadarPanel Tree를 교체하지 않고 Frame/Range/Player/SelectedEdge 의미 Widget만 누락 시 추가하며 기존 Contact/Selected Image는 Brush/Color만 교체합니다.
// - v1.9.0: Build 경로를 신규/빈 Widget Blueprint의 최초 Scaffold 전용으로 제한하고 기존 Designer Tree는 Validate-only로 보호. 기존 Asset의 Position/Size/Anchor/Alignment/Padding/AutoSize를 Bridge 재적용 계약에서 제거.

// - v1.8.0: SpeedGauge를 `Image_RPMGauge` 단일 UI Material 구조로 전환하고 구형 `Image_RPMTrackArt` / `ProgressBar_RPMTick00~20` 잔존을 Validator에서 금지. Runtime은 `RPMRatio` 스칼라 하나만 소비.
// - v1.7.0: WBP_CFArmorSector를 세 번째 Production Element로 추가. ArmorBodyMap은 방향별 Image/Text/ProgressBar를 직접 그리지 않고 ArmorSector Generated Class 6개를 공간 배치하도록 Bridge 입력·검증 계약을 확장.
// - v1.6.0: VehiclePanelFrame 9-Slice 소비와 FRONT/RIGHT/REAR/LEFT/TOP/BOTTOM Armor Badge Label을 additive 추가. 차량 좌향 기준 Front←/Right↑/Rear→/Left↓, Top 좌상단, Bottom 우하단 배치를 Validator 계약으로 승격.
// - v1.5.0: WeaponPanel Rail의 Turret/Ammo/Reload placeholder Image를 제거하고 112x68 fixed Text Tile 3개 + 8px gap으로 교체. Designer default는 Collapsed, Runtime Presenter만 실제 비선택 Weapon Selection이 있을 때 표시.
// - v1.4.0: SpeedGauge의 기존 ProgressBar_RPMTick00~20을 Presenter가 runtime Percent sink로 재사용하는 계약을 명시. Designer Preview 구조/개수/배치와 Asset 구조는 변경하지 않음.
// - v1.3.0: WeaponPanel Stage B. Launcher/Ammo/Heat/Cooldown 구형 고정 Row를 새 Compact Resource Presentation 의미 슬롯으로 교체하고 Validator가 중복 legacy slot을 차단하도록 계약 갱신.
// - v1.2.0: 전용 RPM Track Texture가 없는 Image 슬롯을 Collapsed로 보존하고 빈 Brush 가시 회귀를 Production Validator에서 차단.
// - v1.1.0: 최신 VehiclePanel 계약에 맞춘 SpeedGauge RPM Preview와 Armor 세로 Bar 회귀 검증을 Production Widget Validate 계약에 반영.
// - v1.0.0: Production Child Widget Build/Validate와 Root Composition Build/Validate bool-only Python Bridge를 최초 추가.
// Migration:
// - 기존 CFUIHUDEditorBridge v1.x Border/Canvas Mock 경로는 Historical Preview evidence로 보존합니다.
// - Production Bridge는 Gameplay 조회, Runtime Event Binding, 직접 콘텐츠 경로 하드코딩을 만들지 않습니다.
// - WBP_CFArmorSector는 방향 Label + Plate Image + 실제 Armor Ratio ProgressBar의 재사용 Presentation 단위이며 WBP_CFArmorBodyMap의 Canvas는 여섯 Sector 위치와 차량 실루엣 배치만 소유합니다.
// - WBP_CFSpeedGauge는 Track·21 Tick·Redline 표현을 `Image_RPMGauge`의 UI Material 한 장에서 처리하며 UCFHUDPresenter는 explicit Redline/Maximum 계약으로 계산한 `RPMRatio` 하나만 전달합니다.
// - HUDVisualData.SpeedArcMaterial이 없으면 `Image_RPMGauge`는 표시하지 않으며 Production Validator가 fail-closed합니다. 구형 `Image_RPMTrackArt`와 `ProgressBar_RPMTick*`는 현재 구조에서 금지됩니다.
// - WeaponPanel은 raw ResourceChannels 개수대로 Widget을 생성하지 않으며 Primary/SecondaryA/SecondaryB/FireState 고정 의미 슬롯만 생성합니다.
// - ReserveAmmo는 Header 우측 label-less owner를 유지하고 VehicleBattery의 현재 Runtime 부재를 가짜 Preview Row로 보완하지 않습니다. WeaponCharge/Heat는 기존 actual Runtime Projection만 사용합니다.
// - v1.5.0 Rail은 실제 weapon icon source가 생기기 전 Image를 만들지 않습니다. 비선택 weapon resource summary도 현재 ViewData에 없으므로 Designer Tile은 Text slot만 소유합니다.
// - v1.9.0부터 BuildProductionWidgetResult/BuildProductionRootResult는 최초 Scaffold 전용입니다. 이미 RootWidget이 존재하는 저장 Designer Asset은 ValidateProduction*Result로만 검사하며 Tree를 교체하지 않습니다.
// - Scaffold에 들어가는 초기 픽셀값은 생성 편의를 위한 시작값일 뿐 장기 Layout 계약이 아닙니다. 저장 후 Position/Size/Anchor/Alignment/Padding/AutoSize는 UMG Designer Asset이 소유합니다.
// - v1.10.0 Radar migration은 새로 추가되는 Widget에만 최초 Anchor/Size를 부여합니다. 이미 존재하는 Radar Widget의 Slot Layout은 읽거나 다시 쓰지 않습니다.
// - v1.11.0 ViewMode migration은 기존 `CanvasPanel_Slot_ReticleLayer`의 Root Slot을 수정하지 않습니다. 새 Direction Track/Image의 최초 Scaffold Layout만 설정하며 이후 Position/Size/Anchor는 UMG Designer Asset이 소유합니다.



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
			// [v1.9.0] RootWidget이 아직 없는 신규 Production Widget에만 최초 의미 구조 Scaffold를 생성합니다. 기존 Designer Tree는 이 경로로 재구축하지 않습니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Widget 최초 Scaffold (Scaffold Production HUD Widget)", ToolTip="신규/빈 Production Widget에만 최초 의미 구조를 생성합니다. 이미 Designer Tree가 있는 Asset은 실패하며 Validate 경로를 사용해야 합니다."))

	static bool BuildProductionWidgetResult(
		UObject* WidgetBlueprintObject,
		FName WidgetRole,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UCFHUDVisualData* HUDVisualData,
		UObject* SpeedGaugeBlueprintObject,
		UObject* ArmorBodyMapBlueprintObject,
		UObject* ArmorSectorBlueprintObject);

			// [v1.9.0] 저장 Production Widget의 이름·타입·의미 구조를 검증하되 Position/Size/Anchor/Alignment/Padding/AutoSize는 검사하거나 덮어쓰지 않습니다.

	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Widget 검증 (Validate Production HUD Widget)", ToolTip="ArmorSector의 Image/Text/실제 Armor Progress 슬롯과 ArmorBodyMap의 6개 Sector Class, 그리고 기존 Production 역할별 구조 계약을 검증합니다."))
		static bool ValidateProductionWidgetResult(
		UObject* WidgetBlueprintObject,
		FName WidgetRole,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UCFHUDVisualData* HUDVisualData,
		UObject* SpeedGaugeBlueprintObject,
		UObject* ArmorBodyMapBlueprintObject,
		UObject* ArmorSectorBlueprintObject);

	// [v1.10.0] 저장 RadarPanel의 기존 Slot Layout을 보존하면서 UI-P0-08B에 필요한 정적 Visual 의미 Widget과 Brush만 additive 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Radar Visual 보존 마이그레이션 (Apply Radar Visual Migration)", ToolTip="기존 WBP_CFRadarPanel Tree와 기존 Widget의 위치·크기·Anchor를 유지하고 Radar Frame, Range Text, Player Marker, Selected Edge Bracket이 없을 때만 추가합니다. 기존 Contact/Selected Image는 전용 Texture Brush와 Style Color만 갱신합니다."))
		static bool ApplyRadarVisualMigrationResult(
		UObject* RadarPanelBlueprintObject,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UCFHUDVisualData* HUDVisualData);

	// [v1.11.0] 저장 Root의 기존 7-child Layout을 보존하면서 ReticleLayer 내부에 Vehicle Direction Track/Image만 additive 추가합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="View Mode Visual 보존 마이그레이션 (Apply View Mode Visual Migration)", ToolTip="기존 WBP_CFInGameHUD Root와 Panel Slot을 유지하고 CanvasPanel_Slot_ReticleLayer 안에 Vehicle Direction Track과 Vehicle Semantic Image가 없을 때만 추가합니다."))
	static bool ApplyViewModeVisualMigrationResult(
		UObject* RootWidgetBlueprintObject,
		UCFUIStyleData* StyleData);

		// [v1.9.0] RootWidget이 아직 없는 신규 Production Root에만 D1-07 초기 Slot Scaffold를 생성합니다. 저장된 Root Designer Layout은 재구축하지 않습니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Root 최초 Scaffold (Scaffold Production HUD Root)", ToolTip="신규/빈 Root에만 초기 Panel Slot을 생성합니다. 저장된 Root는 Validate-only이며 Designer Layout을 덮어쓰지 않습니다."))

	static bool BuildProductionRootResult(
		UObject* RootWidgetBlueprintObject,
		UCFHUDLayoutData* LayoutData,
		UObject* MissionPanelBlueprintObject,
		UObject* AlertFeedBlueprintObject,
		UObject* TargetPanelBlueprintObject,
		UObject* VehiclePanelBlueprintObject,
		UObject* RadarPanelBlueprintObject,
		UObject* WeaponPanelBlueprintObject);

		// [v1.9.0] 저장된 Production HUD Root의 7 Slot 이름·Panel Class·Graph 0 의미 계약을 검증하고 Canvas Layout 값은 Designer 소유로 남깁니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Production HUD Root 검증 (Validate Production HUD Root)", ToolTip="WBP_CFInGameHUD Root의 Slot 이름·Panel Class·Runtime Graph 부재를 검증합니다. 위치·크기·Anchor·Alignment는 검증하지 않습니다."))

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
