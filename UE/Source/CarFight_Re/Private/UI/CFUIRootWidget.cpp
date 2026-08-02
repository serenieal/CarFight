// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.2
// Date: 2026-08-01
// Description: CarFight World별 공통 UI Root Widget 구현
// Scope: 에셋 저장 없이 표준 CanvasPanel 레이어를 결정론적 순서와 ZOrder로 구성합니다.
// Changelog:
// - v1.0.2: 명시적 EnsureLayerTree 계약으로 CreateWidget와 Automation 초기화를 동일하게 보장.
// - v1.0.1: UOverlaySlot의 ZOrder 미지원 문제를 제거하고 CanvasPanelSlot 기반 명시적 ZOrder로 교체.
// - v1.0.0: UI-P0-01B C++ Root와 8개 레이어 수명 API를 최초 구현.
// Migration:
// - Blueprint Root를 추가할 때도 레이어 의미와 순서는 이 계약을 유지한다.

#include "UI/CFUIRootWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Widget.h"

// [v1.0.0] Widget 초기화 시 표준 레이어 트리를 준비합니다.
void UCFUIRootWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureLayerTree();
}

// [v1.0.2] 표준 레이어 트리를 중복 없이 보장하고 완성 여부를 반환합니다.
bool UCFUIRootWidget::EnsureLayerTree()
{
	BuildLayerTree();
	return RootCanvas && GameLayer && HUDLayer && ScreenLayer && PanelLayer && MenuLayer && ModalLayer && SystemLayer && DebugLayer;
}

// [v1.0.1] 지정한 표준 UI 레이어의 CanvasPanel을 반환합니다.
UCanvasPanel* UCFUIRootWidget::GetLayerWidget(const ECFUILayer Layer) const
{
	switch (Layer)
	{
	case ECFUILayer::Game:
		return GameLayer;
	case ECFUILayer::HUD:
		return HUDLayer;
	case ECFUILayer::Screen:
		return ScreenLayer;
	case ECFUILayer::Panel:
		return PanelLayer;
	case ECFUILayer::Menu:
		return MenuLayer;
	case ECFUILayer::Modal:
		return ModalLayer;
	case ECFUILayer::System:
		return SystemLayer;
	case ECFUILayer::Debug:
		return DebugLayer;
	default:
		return nullptr;
	}
}

// [v1.0.1] Widget을 지정 레이어에 중복 없이 추가하고 실제 ZOrder를 적용합니다.
bool UCFUIRootWidget::AddWidgetToLayer(UWidget* WidgetToAdd, const ECFUILayer Layer, const int32 ZOrder)
{
	if (!WidgetToAdd)
	{
		return false;
	}

	UCanvasPanel* TargetLayer = GetLayerWidget(Layer);
	if (!TargetLayer)
	{
		return false;
	}

	if (WidgetToAdd->GetParent() == TargetLayer)
	{
		if (UCanvasPanelSlot* ExistingCanvasSlot = Cast<UCanvasPanelSlot>(WidgetToAdd->Slot))
		{
			ExistingCanvasSlot->SetZOrder(ZOrder);
		}
		return true;
	}

	WidgetToAdd->RemoveFromParent();

	UCanvasPanelSlot* AddedCanvasSlot = TargetLayer->AddChildToCanvas(WidgetToAdd);
	if (!AddedCanvasSlot)
	{
		return false;
	}

	AddedCanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	AddedCanvasSlot->SetOffsets(FMargin(0.0f));
	AddedCanvasSlot->SetAlignment(FVector2D::ZeroVector);
	AddedCanvasSlot->SetZOrder(ZOrder);
	return true;
}

// [v1.0.0] Widget을 현재 UI 부모에서 제거합니다.
bool UCFUIRootWidget::RemoveLayerWidget(UWidget* WidgetToRemove)
{
	if (!WidgetToRemove || !WidgetToRemove->GetParent())
	{
		return false;
	}

	WidgetToRemove->RemoveFromParent();
	return true;
}

// [v1.0.0] 지정 레이어의 모든 자식 Widget을 제거합니다.
void UCFUIRootWidget::ClearLayer(const ECFUILayer Layer)
{
	if (UCanvasPanel* TargetLayer = GetLayerWidget(Layer))
	{
		TargetLayer->ClearChildren();
	}
}

// [v1.0.0] 모든 표준 UI 레이어의 자식 Widget을 제거합니다.
void UCFUIRootWidget::ClearAllLayers()
{
	ClearLayer(ECFUILayer::Game);
	ClearLayer(ECFUILayer::HUD);
	ClearLayer(ECFUILayer::Screen);
	ClearLayer(ECFUILayer::Panel);
	ClearLayer(ECFUILayer::Menu);
	ClearLayer(ECFUILayer::Modal);
	ClearLayer(ECFUILayer::System);
	ClearLayer(ECFUILayer::Debug);
}

// [v1.0.1] C++ 전용 Root Canvas와 표준 레이어를 최초 한 번 생성합니다.
void UCFUIRootWidget::BuildLayerTree()
{
	if (RootCanvas && GameLayer && HUDLayer && ScreenLayer && PanelLayer && MenuLayer && ModalLayer && SystemLayer && DebugLayer)
	{
		return;
	}

	if (!WidgetTree)
	{
		return;
	}

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Canvas_CFUIRoot"));
	if (!RootCanvas)
	{
		return;
	}

	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	GameLayer = CreateLayerCanvas(TEXT("Canvas_GameLayer"), 0);
	HUDLayer = CreateLayerCanvas(TEXT("Canvas_HUDLayer"), 100);
	ScreenLayer = CreateLayerCanvas(TEXT("Canvas_ScreenLayer"), 200);
	PanelLayer = CreateLayerCanvas(TEXT("Canvas_PanelLayer"), 300);
	MenuLayer = CreateLayerCanvas(TEXT("Canvas_MenuLayer"), 400);
	ModalLayer = CreateLayerCanvas(TEXT("Canvas_ModalLayer"), 500);
	SystemLayer = CreateLayerCanvas(TEXT("Canvas_SystemLayer"), 600);
	DebugLayer = CreateLayerCanvas(TEXT("Canvas_DebugLayer"), 700);
}

// [v1.0.1] Root Canvas 아래에 한 표준 레이어 Canvas를 생성합니다.
UCanvasPanel* UCFUIRootWidget::CreateLayerCanvas(const FName LayerWidgetName, const int32 LayerZOrder)
{
	if (!WidgetTree || !RootCanvas)
	{
		return nullptr;
	}

	UCanvasPanel* CreatedLayer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), LayerWidgetName);
	if (!CreatedLayer)
	{
		return nullptr;
	}

	CreatedLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UCanvasPanelSlot* CreatedLayerSlot = RootCanvas->AddChildToCanvas(CreatedLayer);
	if (!CreatedLayerSlot)
	{
		return nullptr;
	}

	CreatedLayerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	CreatedLayerSlot->SetOffsets(FMargin(0.0f));
	CreatedLayerSlot->SetAlignment(FVector2D::ZeroVector);
	CreatedLayerSlot->SetZOrder(LayerZOrder);
	return CreatedLayer;
}
