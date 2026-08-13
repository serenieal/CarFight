// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-10
// Description: CF-FQ-032 D1-11 WBP_CFInGameHUD 1920x1080 Visual Prototype Editor Bridge
// Scope: D1-07 승인 1080p Slot·Mock Visual을 Widget Blueprint Designer Tree에 생성하고 Visual Fidelity 정적 계약까지 검증합니다.
// Changelog:
// - v1.3.0: 세 번째 사용자 Preview FAIL의 Vehicle Armor 의미별 위치·값 계약과 Speed Arc·왼쪽 전방 Silhouette 판독성 회귀 검증을 추가. 공개 함수 시그니처는 유지.
// - v1.2.0: 두 번째 사용자 Preview FAIL의 Vehicle 원호형 Speed/차체 Silhouette와 Designer Placeholder 차단 계약을 추가. 공개 함수 시그니처는 유지.
// - v1.1.0: 사용자 Preview FAIL을 반영한 Canvas 기반 내부 Visual Fidelity 재구축·검증 계약으로 확장. 공개 함수 시그니처는 유지.
// - v1.0.0: D1-11 HUD Prototype Build/Validate bool-only Python Bridge를 최초 추가.
// Migration:
// - Editor 자동화 전용이며 실제 HUD Runtime, Gameplay View Data, Event Binding을 추가하지 않습니다.
// - D1-09B Style/Density/Layout와 D1-10 Base Widget Asset을 읽기 전용 입력으로 사용하고 수정하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CFUIHUDEditorBridge.generated.h"

class UCFHUDLayoutData;
class UCFUIDensityData;
class UCFUIStyleData;

/**
 * D1-11 기능 없는 1920x1080 HUD Visual Prototype의 Editor 전용 Designer Tree Bridge입니다.
 */
UCLASS()
class CARFIGHT_RE_API UCFUIHUDEditorBridge : public UObject
{
	GENERATED_BODY()

public:
			// [v1.3.0] D1-11 Layout·Style·Density와 기존 Base Widget을 사용해 WBP_CFInGameHUD Designer Tree를 생성 또는 재구축합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="HUD Prototype 생성 결과 (Build HUD Prototype Result)", ToolTip="D1-11 1920x1080 기능 없는 HUD Visual Prototype Tree를 생성합니다. Gameplay 조회와 Runtime Event를 만들지 않습니다."))
	static bool BuildHUDPrototypeResult(
		UObject* WidgetBlueprintObject,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UObject* PanelBlueprintObject,
		UObject* AlertBlueprintObject,
		UObject* InfoRowBlueprintObject);

			// [v1.3.0] 저장된 WBP_CFInGameHUD의 Parent·7 Slot·Vehicle 의미별 위치·Visual Fidelity·Typography·Graph 0 계약을 읽기 전용으로 검증합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="HUD Prototype 검증 결과 (Validate HUD Prototype Result)", ToolTip="D1-11 HUD Prototype의 1080p Slot Bounds, Mock 문자열, Typography 하한, Base Widget Class와 Graph 0 계약을 검증합니다."))
	static bool ValidateHUDPrototypeResult(
		UObject* WidgetBlueprintObject,
		UCFHUDLayoutData* LayoutData,
		UCFUIStyleData* StyleData,
		UCFUIDensityData* StandardDensityData,
		UCFUIDensityData* CompactDensityData,
		UObject* PanelBlueprintObject,
		UObject* AlertBlueprintObject,
		UObject* InfoRowBlueprintObject);
};
