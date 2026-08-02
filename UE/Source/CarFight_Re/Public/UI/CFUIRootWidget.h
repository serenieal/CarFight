// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.2
// Date: 2026-08-01
// Description: CarFight World별 공통 UI Root Widget
// Scope: Game·HUD·Screen·Panel·Menu·Modal·System·Debug 레이어를 C++ WidgetTree로 제공합니다.
// Changelog:
// - v1.0.2: CreateWidget와 Automation에서 공통으로 사용할 명시적 EnsureLayerTree 계약을 추가.
// - v1.0.1: 현재 엔진에서 명시적 ZOrder를 지원하도록 Overlay 대신 CanvasPanel 레이어를 사용.
// - v1.0.0: UI-P0-01B 표준 8개 레이어와 Widget 추가·제거·정리 API를 최초 추가.
// Migration:
// - 이번 단계에서는 WBP_CFUIRoot 에셋을 만들지 않고 C++ Root를 사용한다.
// - 기존 Pawn 소유 AimReticle과 TargetSelect HUD는 UI-P0-04~05 전까지 Viewport에 그대로 유지한다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CFUITypes.h"
#include "CFUIRootWidget.generated.h"

class UCanvasPanel;
class UWidget;

/**
 * LocalPlayer의 현재 World에서 한 번만 생성되는 공통 UI Root입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] Widget 초기화 시 표준 레이어 트리를 준비합니다.
		virtual void NativeOnInitialized() override;

	// [v1.0.2] 표준 레이어 트리를 중복 없이 보장하고 완성 여부를 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Root", meta=(DisplayName="UI 레이어 트리 보장 (Ensure UI Layer Tree)", ToolTip="C++ UI Root의 표준 8개 CanvasPanel 레이어를 중복 없이 생성하고 모두 준비됐는지 반환합니다."))
	bool EnsureLayerTree();

	// [v1.0.1] 지정한 표준 UI 레이어의 CanvasPanel을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Root", meta=(DisplayName="UI 레이어 반환 (Get UI Layer)", ToolTip="Game, HUD, Screen, Panel, Menu, Modal, System, Debug 중 지정한 표준 CanvasPanel 레이어를 반환합니다."))
	UCanvasPanel* GetLayerWidget(ECFUILayer Layer) const;

	// [v1.0.1] Widget을 지정 레이어에 중복 없이 추가하고 실제 ZOrder를 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Root", meta=(DisplayName="레이어에 Widget 추가 (Add Widget To Layer)", ToolTip="Widget을 기존 부모에서 분리한 뒤 지정 UI 레이어에 추가하고 ZOrder를 적용합니다."))
	bool AddWidgetToLayer(UWidget* WidgetToAdd, ECFUILayer Layer, int32 ZOrder = 0);

	// [v1.0.0] Widget을 현재 UI 부모에서 제거합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Root", meta=(DisplayName="레이어 Widget 제거 (Remove Layer Widget)", ToolTip="지정 Widget을 현재 UI 부모에서 안전하게 제거합니다."))
	bool RemoveLayerWidget(UWidget* WidgetToRemove);

	// [v1.0.0] 지정 레이어의 모든 자식 Widget을 제거합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Root", meta=(DisplayName="UI 레이어 비우기 (Clear UI Layer)", ToolTip="지정한 표준 UI 레이어의 모든 자식 Widget을 제거합니다."))
	void ClearLayer(ECFUILayer Layer);

	// [v1.0.0] 모든 표준 UI 레이어의 자식 Widget을 제거합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Root", meta=(DisplayName="모든 UI 레이어 비우기 (Clear All UI Layers)", ToolTip="Game부터 Debug까지 모든 표준 레이어의 자식 Widget을 제거합니다."))
	void ClearAllLayers();

private:
	// [v1.0.1] C++ 전용 Root Canvas와 표준 레이어를 최초 한 번 생성합니다.
	void BuildLayerTree();

	// [v1.0.1] Root Canvas 아래에 한 표준 레이어 Canvas를 생성합니다.
	UCanvasPanel* CreateLayerCanvas(FName LayerWidgetName, int32 LayerZOrder);

	// [v1.0.1] 모든 표준 레이어를 포함하는 최상위 CanvasPanel입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;

	// [v1.0.1] 월드 위치 기반 마커를 배치할 Game 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> GameLayer = nullptr;

	// [v1.0.1] 상시 인게임 HUD를 배치할 HUD 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> HUDLayer = nullptr;

	// [v1.0.1] 차고·피팅 등 주요 전체 화면을 배치할 Screen 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> ScreenLayer = nullptr;

	// [v1.0.1] 현재 화면 위 상세 패널을 배치할 Panel 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> PanelLayer = nullptr;

	// [v1.0.1] Pause 같은 메뉴를 배치할 Menu 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> MenuLayer = nullptr;

	// [v1.0.1] 확인·경고 팝업을 배치할 Modal 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> ModalLayer = nullptr;

	// [v1.0.1] 저장·입력·오류 알림을 배치할 System 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> SystemLayer = nullptr;

	// [v1.0.1] 개발용 진단 Widget을 배치할 Debug 레이어입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> DebugLayer = nullptr;
};
