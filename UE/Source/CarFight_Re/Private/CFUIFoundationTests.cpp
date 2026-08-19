// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-18
// Description: CF-FQ-032 UI-P0-01A~05 기반 Automation 테스트
// Scope: PlayerController 기본 소유 상태, C++ UI Root 레이어 계약, AimReticle HUD Layer와 TargetSelect Game Layer Marker 계약을 검증합니다.
// Changelog:
// - v1.3.0: UI-P0-05 TargetSelect Marker의 Game Layer ZOrder, 동일 Widget 중복 추가 방지, Null Rebind 시 Marker-only 캐시 정리 계약을 추가.
// - v1.2.0: UI-P0-04 AimReticle이 Production HUD와 같은 HUD Layer에서 더 높은 로컬 ZOrder를 사용하는 계약을 추가.
// - v1.1.0: UI Root가 Legacy Aim·Target HUD보다 높은 Viewport ZOrder를 사용하는 계약을 추가.
// - v1.0.0: ControllerContract와 RootLayerContract 테스트를 최초 추가.
// Migration:
// - 테스트는 Transient UObject만 사용하며 Blueprint, Map, DataAsset과 피팅 파일을 생성하거나 수정하지 않는다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFPlayerController.h"
#include "UI/CFUIRootWidget.h"
#include "UI/CFUISubsystem.h"
#include "UI/CFTargetSelectWidget.h"
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

	// [v1.1.0] Legacy Pawn HUD보다 높은 Viewport 우선순위를 검증할 UI Subsystem CDO입니다.
	const UCFUISubsystem* UISubsystemDefaults = GetDefault<UCFUISubsystem>();
	if (!TestNotNull(TEXT("CFUISubsystem CDO"), UISubsystemDefaults))
	{
		return false;
	}
			TestTrue(TEXT("UI Root Viewport ZOrder는 Gameplay HUD 계층보다 높음"), UISubsystemDefaults->GetRootViewportZOrder() > 20);
	TestTrue(TEXT("Aim Reticle HUD Layer ZOrder는 Production HUD 0보다 높음"), UISubsystemDefaults->GetAimReticleHUDLayerZOrder() > 0);
	TestEqual(TEXT("Target Marker는 Game Layer 기본 ZOrder 사용"), UISubsystemDefaults->GetTargetSelectGameLayerZOrder(), 0);

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUITargetMarkerLayerContractTest,
	"CarFight.UI.UI_P0_05.TargetMarkerLayerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.3.0] TargetSelect Marker가 Game Layer의 단일 자식으로 유지되고 Null Rebind에서 Player-facing 의미 캐시를 남기지 않는지 검증합니다.
bool FCFUITargetMarkerLayerContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.3.0] Target Marker의 Game Layer 부모와 중복 추가 방지를 검증할 Transient UI Root입니다.
	UCFUIRootWidget* RootWidget = NewObject<UCFUIRootWidget>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient UI Root"), RootWidget))
	{
		return false;
	}

	TestTrue(TEXT("Transient UI Root 초기화"), RootWidget->Initialize());
	TestTrue(TEXT("Transient UI Root 레이어 트리 보장"), RootWidget->EnsureLayerTree());

	// [v1.3.0] 실제 WBP 없이 Marker-only C++ 수명 계약을 검증할 Transient TargetSelect Widget입니다.
	UCFTargetSelectWidget* TargetMarkerWidget = NewObject<UCFTargetSelectWidget>(RootWidget);
	if (!TestNotNull(TEXT("Transient Target Marker Widget"), TargetMarkerWidget))
	{
		return false;
	}

	// [v1.3.0] Target Marker가 배치될 표준 World Marker 전용 Game Layer입니다.
	UCanvasPanel* GameLayer = RootWidget->GetLayerWidget(ECFUILayer::Game);
	if (!TestNotNull(TEXT("Game Layer"), GameLayer))
	{
		return false;
	}

	TestTrue(TEXT("Target Marker Game Layer 최초 추가"), RootWidget->AddWidgetToLayer(TargetMarkerWidget, ECFUILayer::Game, 0));
	TestTrue(TEXT("Target Marker 부모는 Game Layer"), TargetMarkerWidget->GetParent() == GameLayer);
	TestTrue(TEXT("동일 Target Marker 재추가는 성공하되 중복 생성하지 않음"), RootWidget->AddWidgetToLayer(TargetMarkerWidget, ECFUILayer::Game, 0));
	TestEqual(TEXT("Game Layer Target Marker 자식 수는 1"), GameLayer->GetChildrenCount(), 1);

	TargetMarkerWidget->SetVehiclePawnRef(nullptr);
	TargetMarkerWidget->RefreshFromTargetSelect();
	TestFalse(TEXT("Null Rebind 후 후보 Marker 비표시"), TargetMarkerWidget->IsCandidateMarkerVisible());
	TestFalse(TEXT("Null Rebind 후 선택 Marker 비표시"), TargetMarkerWidget->IsSelectedMarkerVisible());
	TestNull(TEXT("Null Rebind 후 후보 Actor 캐시 없음"), TargetMarkerWidget->GetCachedCandidateActor());
	TestNull(TEXT("Null Rebind 후 선택 Actor 캐시 없음"), TargetMarkerWidget->GetCachedSelectedActor());
	TestTrue(TEXT("Marker-only 후보 의미 텍스트 비어 있음"), TargetMarkerWidget->GetCandidateInfoText().IsEmpty());
	TestTrue(TEXT("Marker-only 선택 의미 텍스트 비어 있음"), TargetMarkerWidget->GetSelectedInfoText().IsEmpty());
	TestTrue(TEXT("Marker-only Track 의미 텍스트 비어 있음"), TargetMarkerWidget->GetSelectedTrackStateText().IsEmpty());

	RootWidget->ClearLayer(ECFUILayer::Game);
	TestNull(TEXT("Game Layer 정리 뒤 Target Marker 부모 없음"), TargetMarkerWidget->GetParent());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
