// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-032 UI-P0-01A~01B 기반 Automation 테스트
// Scope: PlayerController 기본 소유 상태와 C++ UI Root 표준 레이어·추가·정리 계약을 검증합니다.
// Changelog:
// - v1.0.0: ControllerContract와 RootLayerContract 테스트를 최초 추가.
// Migration:
// - 테스트는 Transient UObject만 사용하며 Blueprint, Map, DataAsset과 피팅 파일을 생성하거나 수정하지 않는다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFPlayerController.h"
#include "UI/CFUIRootWidget.h"
#include "UI/CFUITypes.h"

#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIControllerContractTest,
	"CarFight.UI.UI_P0_01A.ControllerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] PlayerController CDO가 Gameplay 입력 기본 상태와 Context 무소유 상태로 시작하는지 검증합니다.
bool FCFUIControllerContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] C++ 기본 입력·소유 상태를 검증할 PlayerController CDO입니다.
	const ACFPlayerController* PlayerControllerDefaults = GetDefault<ACFPlayerController>();
	if (!TestNotNull(TEXT("CFPlayerController CDO"), PlayerControllerDefaults))
	{
		return false;
	}

	TestFalse(TEXT("기본 상태는 UI 입력 비활성"), PlayerControllerDefaults->IsUIInputEnabled());
	TestEqual(TEXT("CDO는 런타임 Mapping Context 무소유"), PlayerControllerDefaults->GetOwnedMappingContextCount(), 0);
	TestEqual(TEXT("Game 레이어 enum 순서"), static_cast<uint8>(ECFUILayer::Game), static_cast<uint8>(0));
	TestEqual(TEXT("Debug 레이어 enum 순서"), static_cast<uint8>(ECFUILayer::Debug), static_cast<uint8>(7));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIRootLayerContractTest,
	"CarFight.UI.UI_P0_01B.RootLayerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] C++ UI Root가 8개 레이어를 고유하게 생성하고 Widget 추가·정리를 수행하는지 검증합니다.
bool FCFUIRootLayerContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 에셋 없이 표준 레이어를 생성할 Transient UI Root입니다.
	UCFUIRootWidget* RootWidget = NewObject<UCFUIRootWidget>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient UI Root"), RootWidget))
	{
		return false;
	}

		TestTrue(TEXT("Transient UI Root 초기화"), RootWidget->Initialize());
	TestTrue(TEXT("Transient UI Root 레이어 트리 보장"), RootWidget->EnsureLayerTree());

	// [v1.0.0] 모든 표준 레이어가 서로 다른 CanvasPanel인지 확인할 집합입니다.
	TSet<const UCanvasPanel*> UniqueLayers;
	for (uint8 LayerIndex = static_cast<uint8>(ECFUILayer::Game); LayerIndex <= static_cast<uint8>(ECFUILayer::Debug); ++LayerIndex)
	{
		// [v1.0.0] 현재 enum 인덱스에 해당하는 표준 UI 레이어입니다.
		UCanvasPanel* LayerWidget = RootWidget->GetLayerWidget(static_cast<ECFUILayer>(LayerIndex));
		TestNotNull(*FString::Printf(TEXT("UI Layer %d"), LayerIndex), LayerWidget);
		if (LayerWidget)
		{
			UniqueLayers.Add(LayerWidget);
		}
	}
	TestEqual(TEXT("표준 UI 레이어 8개 고유 생성"), UniqueLayers.Num(), 8);

	// [v1.0.0] Screen 레이어 추가·제거를 검증할 Transient TextBlock입니다.
	UTextBlock* TestScreenWidget = NewObject<UTextBlock>(RootWidget);
	if (!TestNotNull(TEXT("Transient Screen Widget"), TestScreenWidget))
	{
		return false;
	}

	TestTrue(TEXT("Screen 레이어 추가"), RootWidget->AddWidgetToLayer(TestScreenWidget, ECFUILayer::Screen, 25));
	TestTrue(TEXT("Screen 레이어 부모 연결"), TestScreenWidget->GetParent() == RootWidget->GetLayerWidget(ECFUILayer::Screen));
	RootWidget->ClearLayer(ECFUILayer::Screen);
	TestNull(TEXT("Screen 레이어 정리 뒤 부모 없음"), TestScreenWidget->GetParent());

	RootWidget->ClearAllLayers();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
