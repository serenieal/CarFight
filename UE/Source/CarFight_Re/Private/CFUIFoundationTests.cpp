// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-08-21
// Description: CF-FQ-032 UI-P0-01A~05 기반 + UI-P0-08 Screen-edge 방향·관계색·가독성 Automation 테스트
// Scope: 기존 UI Root/Target Marker 수명 계약과 Camera View Space 기반 Screen-edge 방향·Safe Region 배치·관계색 매핑·48×36 Bounds를 검증합니다.
// Changelog:
// - v1.5.0: USER 가독성 교정의 Friendly=파랑, Hostile=빨강, Neutral/Unknown=회색 관계색 매핑과 Screen-edge Bounds 48×36 Safe Region 배치 계약을 추가.
// - v1.4.0: UI-P0-08 Selected Target Screen-edge의 Camera View Space 방향 안정화, BehindCamera 특이점 보존, 1920×1080 Safe Region Ray Intersection과 2-Corner Source Art 회전 기준 계약을 추가.
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
	TestFalse(TEXT("Null Rebind 후 선택 Screen-edge Marker 비표시"), TargetMarkerWidget->IsSelectedEdgeMarkerVisible());
	TestNull(TEXT("Null Rebind 후 후보 Actor 캐시 없음"), TargetMarkerWidget->GetCachedCandidateActor());
	TestNull(TEXT("Null Rebind 후 선택 Actor 캐시 없음"), TargetMarkerWidget->GetCachedSelectedActor());
	TestTrue(TEXT("Marker-only 후보 의미 텍스트 비어 있음"), TargetMarkerWidget->GetCandidateInfoText().IsEmpty());
	TestTrue(TEXT("Marker-only 선택 의미 텍스트 비어 있음"), TargetMarkerWidget->GetSelectedInfoText().IsEmpty());
	TestTrue(TEXT("Marker-only Track 의미 텍스트 비어 있음"), TargetMarkerWidget->GetSelectedTrackStateText().IsEmpty());

		RootWidget->ClearLayer(ECFUILayer::Game);
	TestNull(TEXT("Game Layer 정리 뒤 Target Marker 부모 없음"), TargetMarkerWidget->GetParent());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIScreenEdgeTargetMarkerContractTest,
	"CarFight.UI.UI_P0_08.ScreenEdgeTargetMarkerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.5.0] Camera View Space 방향·48×36 Safe Region 배치·관계색이 USER 승인 Screen-edge 계약으로 변환되는지 검증합니다.
bool FCFUIScreenEdgeTargetMarkerContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] 오른쪽 앞 Target의 Camera View Space 방향을 Screen +X로 변환한 결과입니다.
	const FVector2D RightEdgeDirection = UCFTargetSelectWidget::ResolveCameraViewEdgeDirection(FVector(100.0f, 100.0f, 0.0f), FVector2D::ZeroVector);
	TestTrue(TEXT("Camera +Right는 Screen +X 방향"), RightEdgeDirection.Equals(FVector2D(1.0f, 0.0f), KINDA_SMALL_NUMBER));

	// [v1.4.0] 위쪽 앞 Target의 Camera View Space 방향을 Screen -Y로 변환한 결과입니다.
	const FVector2D UpEdgeDirection = UCFTargetSelectWidget::ResolveCameraViewEdgeDirection(FVector(100.0f, 0.0f, 100.0f), FVector2D::ZeroVector);
	TestTrue(TEXT("Camera +Up은 Screen -Y 방향"), UpEdgeDirection.Equals(FVector2D(0.0f, -1.0f), KINDA_SMALL_NUMBER));

	// [v1.4.0] BehindCamera 오른쪽 Target이 화면 중심을 가로질러 반전되지 않고 같은 오른쪽 Edge를 유지하는 방향입니다.
	const FVector2D BehindRightDirection = UCFTargetSelectWidget::ResolveCameraViewEdgeDirection(FVector(-100.0f, 100.0f, 0.0f), FVector2D::ZeroVector);
	TestTrue(TEXT("BehindCamera +Right도 Screen +X Edge 유지"), BehindRightDirection.Equals(FVector2D(1.0f, 0.0f), KINDA_SMALL_NUMBER));

	// [v1.4.0] 정확한 정후방 특이점에서 이전 안정 방향으로 사용할 왼쪽 방향입니다.
	const FVector2D PreviousLeftDirection(-1.0f, 0.0f);
	// [v1.4.0] 정후방 Target이 이전 Edge 방향을 보존한 결과입니다.
	const FVector2D StableBehindDirection = UCFTargetSelectWidget::ResolveCameraViewEdgeDirection(FVector(-100.0f, 0.0f, 0.0f), PreviousLeftDirection);
	TestTrue(TEXT("정후방 특이점은 이전 안정 Edge 방향 보존"), StableBehindDirection.Equals(PreviousLeftDirection, KINDA_SMALL_NUMBER));

	// [v1.4.0] 이전 방향이 없는 최초 정후방 프레임의 결정론적 Screen 하단 fallback입니다.
	const FVector2D InitialBehindDirection = UCFTargetSelectWidget::ResolveCameraViewEdgeDirection(FVector(-100.0f, 0.0f, 0.0f), FVector2D::ZeroVector);
	TestTrue(TEXT("최초 정후방은 결정론적 Screen 하단 fallback"), InitialBehindDirection.Equals(FVector2D(0.0f, 1.0f), KINDA_SMALL_NUMBER));

	// [v1.4.0] StyleSpec 대표 16:9 배치 수학을 검증할 1920×1080 Viewport 크기입니다.
	const FVector2D ViewportSize(1920.0f, 1080.0f);
	// [v1.4.0] 현재 StyleData 기본 Safe Region Inset입니다.
	const float SafeInset = 64.0f;
		// [v1.5.0] USER 가독성 피드백을 반영한 Screen-edge Bracket Bounds입니다.
	const FVector2D MarkerBounds(48.0f, 36.0f);

	// [v1.4.0] 오른쪽 Edge Bracket의 계산된 중심 좌표입니다.
	FVector2D RightEdgePosition = FVector2D::ZeroVector;
	// [v1.4.0] 오른쪽 Edge에서 T_UI_RadarEdge 기준축에 적용할 회전각입니다.
	float RightEdgeAngleDegrees = 0.0f;
	TestTrue(TEXT("오른쪽 Safe Region Edge 배치 계산 성공"), UCFTargetSelectWidget::ResolveScreenEdgePlacement(FVector2D(1.0f, 0.0f), ViewportSize, SafeInset, MarkerBounds, RightEdgePosition, RightEdgeAngleDegrees));
		TestTrue(TEXT("오른쪽 Edge 중심은 48×36 Bracket 전체 Bounds까지 Safe Region 내부"), RightEdgePosition.Equals(FVector2D(1832.0f, 540.0f), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("오른쪽 Edge Source Art 상대 회전 -45도"), FMath::IsNearlyEqual(RightEdgeAngleDegrees, -45.0f, KINDA_SMALL_NUMBER));

	// [v1.4.0] 위쪽 Edge Bracket의 계산된 중심 좌표입니다.
	FVector2D UpEdgePosition = FVector2D::ZeroVector;
	// [v1.4.0] 위쪽 Edge의 Source Art 상대 회전각입니다.
	float UpEdgeAngleDegrees = 0.0f;
	TestTrue(TEXT("위쪽 Safe Region Edge 배치 계산 성공"), UCFTargetSelectWidget::ResolveScreenEdgePlacement(FVector2D(0.0f, -1.0f), ViewportSize, SafeInset, MarkerBounds, UpEdgePosition, UpEdgeAngleDegrees));
		TestTrue(TEXT("위쪽 Edge 중심은 48×36 Bracket 전체 Bounds까지 Safe Region 내부"), UpEdgePosition.Equals(FVector2D(960.0f, 82.0f), KINDA_SMALL_NUMBER));

	// [v1.4.0] 아래쪽 Edge Bracket의 계산된 중심 좌표입니다.
	FVector2D DownEdgePosition = FVector2D::ZeroVector;
	// [v1.4.0] 아래쪽 Edge의 Source Art 상대 회전각입니다.
	float DownEdgeAngleDegrees = 0.0f;
	TestTrue(TEXT("아래쪽 Safe Region Edge 배치 계산 성공"), UCFTargetSelectWidget::ResolveScreenEdgePlacement(FVector2D(0.0f, 1.0f), ViewportSize, SafeInset, MarkerBounds, DownEdgePosition, DownEdgeAngleDegrees));
		TestTrue(TEXT("아래쪽 Edge 중심은 48×36 Bracket 전체 Bounds까지 Safe Region 내부"), DownEdgePosition.Equals(FVector2D(960.0f, 998.0f), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("아래쪽 Edge Source Art 상대 회전 45도"), FMath::IsNearlyEqual(DownEdgeAngleDegrees, 45.0f, KINDA_SMALL_NUMBER));

		// [v1.5.0] 관계색 매핑의 입력과 출력을 명확히 구분하기 위한 Friendly 테스트 색입니다.
	const FLinearColor FriendlyTestColor(0.10f, 0.20f, 0.30f, 1.0f);
	// [v1.5.0] 관계색 매핑의 Hostile 분기를 검증할 테스트 색입니다.
	const FLinearColor HostileTestColor(0.40f, 0.50f, 0.60f, 1.0f);
	// [v1.5.0] Neutral과 Unknown이 Screen-edge에서 공통 사용해야 할 회색 역할의 테스트 색입니다.
	const FLinearColor UnknownTestColor(0.70f, 0.80f, 0.90f, 1.0f);
	TestTrue(TEXT("Friendly Screen-edge는 Friendly 색 사용"), UCFTargetSelectWidget::ResolveScreenEdgeRelationColor(ECFTargetRelation::Friendly, FriendlyTestColor, HostileTestColor, UnknownTestColor).Equals(FriendlyTestColor));
	TestTrue(TEXT("Hostile Screen-edge는 Hostile 색 사용"), UCFTargetSelectWidget::ResolveScreenEdgeRelationColor(ECFTargetRelation::Hostile, FriendlyTestColor, HostileTestColor, UnknownTestColor).Equals(HostileTestColor));
	TestTrue(TEXT("Neutral Screen-edge는 Unknown 회색 공유"), UCFTargetSelectWidget::ResolveScreenEdgeRelationColor(ECFTargetRelation::Neutral, FriendlyTestColor, HostileTestColor, UnknownTestColor).Equals(UnknownTestColor));
	TestTrue(TEXT("Unknown Screen-edge는 Unknown 회색 사용"), UCFTargetSelectWidget::ResolveScreenEdgeRelationColor(ECFTargetRelation::Unknown, FriendlyTestColor, HostileTestColor, UnknownTestColor).Equals(UnknownTestColor));

	// [v1.4.0] 영벡터 방향이 잘못된 화면 가장자리 위치를 만들지 않는지 확인할 출력 좌표입니다.
	FVector2D InvalidEdgePosition = FVector2D::ZeroVector;
	// [v1.4.0] 영벡터 방향 실패 시 변경되지 않을 출력 회전각입니다.
	float InvalidEdgeAngleDegrees = 0.0f;
	TestFalse(TEXT("0 방향은 Screen-edge 배치 실패"), UCFTargetSelectWidget::ResolveScreenEdgePlacement(FVector2D::ZeroVector, ViewportSize, SafeInset, MarkerBounds, InvalidEdgePosition, InvalidEdgeAngleDegrees));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
