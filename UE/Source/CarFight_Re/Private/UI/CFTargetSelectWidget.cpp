// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-22
// Description: UI-P0-05 World Marker / UI-P0-08 Screen-Off Edge Marker + post-closure ViewData hot-path 복사 제거
// Changelog:
// - v1.4.0: Screen-edge 관계색 갱신에서 전체 FCFInGameUIViewData by-value 복사를 제거하고 HUDDataProvider의 C++ const-reference cache를 직접 읽도록 교정. 표시 의미와 색 계약은 불변.
// - v1.3.0: HUDDataProvider가 이미 만든 Target Relation을 읽어 Friendly=파랑, Hostile=빨강, Neutral/Unknown=회색 Edge Tint를 적용하고 Screen-edge Bounds를 48×36으로 확대.
// - v1.2.0: Selected Target이 Safe Region 밖 또는 BehindCamera일 때 2-Corner Edge Bracket을 런타임 생성하고, Camera View Space 방향→Safe Region Ray Intersection으로 위치·회전을 갱신. Candidate offscreen hide 유지.
// - v1.1.0: VehiclePawnRef Weak Binding, Marker-only 표시, Player-facing 의미 텍스트 제거, 이벤트 기반 의미 캐시 + Projection-only Tick, 화면 재진입 Marker 복구.
// Migration:
// - Target 의미 텍스트는 Production TargetPanel이 Provider/Presenter ViewData로 표시하며 이 Widget은 World Marker만 투영합니다.
// - Screen-edge Texture·Tint·SafeInset은 UISubsystem이 Visual/Style Data에서 해석해 주입하며 이 Widget은 콘텐츠 경로를 직접 Load하지 않습니다.
// - v1.4.0 Screen-edge Tick은 GetCurrentViewDataRef()의 Target 채널만 읽어 Radar/Weapon 배열이 포함된 전체 HUD ViewData 복사를 만들지 않습니다.

#include "UI/CFTargetSelectWidget.h"

#include "CFTargetPointComp.h"
#include "CFTargetSelectComp.h"
#include "CFVehiclePawn.h"
#include "UI/CFHUDDataProvider.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"

void UCFTargetSelectWidget::SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)
{
			if (VehiclePawnRef.Get() == InVehiclePawnRef)
	{
		// [v1.1.0] Weak Pointer가 이미 Null로 해석되는 종료 수명에서도 이전 Delegate가 남지 않도록 명시적으로 정리합니다.
		if (InVehiclePawnRef)
		{
			BindTargetSelectEvents();
		}
		else
		{
			UnbindTargetSelectEvents();
		}
		RefreshFromTargetSelect();
		return;
	}
	UnbindTargetSelectEvents();
	VehiclePawnRef = InVehiclePawnRef;
	BindTargetSelectEvents();
	RefreshFromTargetSelect();
}

// [v1.3.0] UISubsystem이 해석한 Screen-edge Texture·Provider·관계색·SafeInset을 저장하고 런타임 Image를 갱신합니다.
void UCFTargetSelectWidget::ConfigureScreenEdgePresentation(UTexture2D* InEdgeBracketTexture, UCFHUDDataProvider* InHUDDataProvider, const FLinearColor& InFriendlyEdgeColor, const FLinearColor& InHostileEdgeColor, const FLinearColor& InUnknownEdgeColor, const float InSafeRegionInset)
{
	SelectedEdgeBracketTexture = InEdgeBracketTexture;
	ScreenEdgeHUDDataProvider = InHUDDataProvider;
	FriendlyEdgeMarkerColor = InFriendlyEdgeColor;
	HostileEdgeMarkerColor = InHostileEdgeColor;
	UnknownEdgeMarkerColor = InUnknownEdgeColor;
	SelectedEdgeSafeInset = FMath::Max(0.0f, InSafeRegionInset);
	EnsureSelectedEdgeMarkerWidget();
}

void UCFTargetSelectWidget::RefreshFromTargetSelect()
{
	RefreshCachedTargetState();
	RefreshWidgetTextAndStyle();
	RefreshMarkerProjection();
}

void UCFTargetSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureSelectedEdgeMarkerWidget();
	BindTargetSelectEvents();
	RefreshFromTargetSelect();
}

void UCFTargetSelectWidget::NativeDestruct()
{
	UnbindTargetSelectEvents();
	Super::NativeDestruct();
}

void UCFTargetSelectWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	(void)InDeltaTime;
		if (bAutoRefreshEveryTick)
	{
		// [v1.1.0] 의미 상태는 TargetSelect 이벤트에서 갱신하고 Tick에서는 움직이는 TargetPoint의 화면 투영만 갱신합니다.
		RefreshMarkerProjection();
	}
}

void UCFTargetSelectWidget::BindTargetSelectEvents()
{
	// [v1.1.0] Weak Pawn Reference에서 이번 바인딩 동안만 사용할 현재 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = VehiclePawnRef.Get();
	UCFTargetSelectComp* TargetSelectComp = IsValid(CurrentVehiclePawn) ? CurrentVehiclePawn->GetTargetSelectComp() : nullptr;
	if (!TargetSelectComp || BoundTargetSelectComp.Get() == TargetSelectComp)
	{
		return;
	}
	UnbindTargetSelectEvents();
	BoundTargetSelectComp = TargetSelectComp;
	TargetSelectComp->OnTargetCandidateChanged.AddDynamic(this, &UCFTargetSelectWidget::HandleTargetCandidateChanged);
	TargetSelectComp->OnSelectedTargetChanged.AddDynamic(this, &UCFTargetSelectWidget::HandleSelectedTargetChanged);
	TargetSelectComp->OnSelectedTargetCleared.AddDynamic(this, &UCFTargetSelectWidget::HandleSelectedTargetCleared);
	TargetSelectComp->OnSelectedTargetValidityChanged.AddDynamic(this, &UCFTargetSelectWidget::HandleSelectedTargetValidityChanged);
	TargetSelectComp->OnSelectedTargetTrackStateChanged.AddDynamic(this, &UCFTargetSelectWidget::HandleSelectedTargetTrackStateChanged);
}

void UCFTargetSelectWidget::UnbindTargetSelectEvents()
{
	if (UCFTargetSelectComp* TargetSelectComp = BoundTargetSelectComp.Get())
	{
		TargetSelectComp->OnTargetCandidateChanged.RemoveAll(this);
		TargetSelectComp->OnSelectedTargetChanged.RemoveAll(this);
		TargetSelectComp->OnSelectedTargetCleared.RemoveAll(this);
		TargetSelectComp->OnSelectedTargetValidityChanged.RemoveAll(this);
		TargetSelectComp->OnSelectedTargetTrackStateChanged.RemoveAll(this);
	}
	BoundTargetSelectComp.Reset();
}

void UCFTargetSelectWidget::ClearCachedTargetState()
{
	CachedCandidateActor.Reset();
	CachedSelectedActor.Reset();
	CachedCandidateData = FCFTargetCandidate();
	CachedSelectedDisplayInfo = FCFTargetDisplayInfo();
	CachedSelectedTrackState = ECFTargetTrackState::Invalid;
	CachedCandidateInfoText = FText::GetEmpty();
	CachedSelectedInfoText = FText::GetEmpty();
	CachedSelectedTrackStateText = FText::GetEmpty();
	CachedSelectedMarkerGlyphText = SelectedMarkerGlyph;
		bCandidateMarkerVisible = false;
	bSelectedMarkerVisible = false;
	bSelectedEdgeMarkerVisible = false;
	CachedSelectedEdgeDirection = FVector2D::ZeroVector;
	if (Image_SelectedEdgeMarker)
	{
		Image_SelectedEdgeMarker->SetVisibility(ESlateVisibility::Collapsed);
	}
	bCachedSelectedTargetValid = false;
}

void UCFTargetSelectWidget::RefreshCachedTargetState()
{
	// [v1.1.0] Weak Pawn Reference에서 이번 Refresh 동안만 사용할 현재 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = VehiclePawnRef.Get();
	UCFTargetSelectComp* TargetSelectComp = IsValid(CurrentVehiclePawn) ? CurrentVehiclePawn->GetTargetSelectComp() : nullptr;
	if (!TargetSelectComp)
	{
		ClearCachedTargetState();
		return;
	}

	CachedCandidateActor = TargetSelectComp->GetCurrentCandidateActor();
	CachedCandidateData = TargetSelectComp->GetCurrentCandidateData();
	CachedSelectedActor = TargetSelectComp->GetSelectedTargetActor();
	CachedSelectedDisplayInfo = TargetSelectComp->GetSelectedTargetDisplayInfo();
	CachedSelectedTrackState = TargetSelectComp->GetSelectedTargetTrackState();
	bCachedSelectedTargetValid = TargetSelectComp->IsSelectedTargetValid();

	AActor* CandidateActor = CachedCandidateActor.Get();
	AActor* SelectedActor = CachedSelectedActor.Get();
	bCandidateMarkerVisible = IsValid(CandidateActor) && CandidateActor != SelectedActor;
	bSelectedMarkerVisible = IsValid(SelectedActor) && bCachedSelectedTargetValid;

		// [v1.1.0] World Marker는 의미 텍스트를 소유하지 않습니다. 이름·거리·관계·Knowledge는 Production TargetPanel ViewData에서만 표시합니다.
	CachedCandidateInfoText = FText::GetEmpty();
	CachedSelectedInfoText = FText::GetEmpty();
	CachedSelectedTrackStateText = FText::GetEmpty();
	CachedSelectedMarkerGlyphText = CachedSelectedTrackState == ECFTargetTrackState::Occluded ? OccludedSelectedMarkerGlyph : SelectedMarkerGlyph;
}

void UCFTargetSelectWidget::RefreshWidgetTextAndStyle()
{
	if (VerticalBox_CandidateRoot)
	{
		VerticalBox_CandidateRoot->SetVisibility(bCandidateMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (Text_CandidateMarker)
	{
		Text_CandidateMarker->SetText(CandidateMarkerGlyph);
		Text_CandidateMarker->SetColorAndOpacity(CandidateMarkerColor);
	}
		if (Text_CandidateInfo)
	{
		Text_CandidateInfo->SetText(FText::GetEmpty());
		Text_CandidateInfo->SetVisibility(ESlateVisibility::Collapsed);
	}

		if (VerticalBox_SelectedRoot)
	{
		VerticalBox_SelectedRoot->SetVisibility(bSelectedMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (Image_SelectedEdgeMarker)
	{
		Image_SelectedEdgeMarker->SetVisibility(bSelectedEdgeMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	FLinearColor SelectedColor = SelectedMarkerColor;
	if (!bCachedSelectedTargetValid || CachedSelectedTrackState == ECFTargetTrackState::Invalid)
	{
		SelectedColor = InvalidSelectedMarkerColor;
	}
	else if (CachedSelectedTrackState == ECFTargetTrackState::Occluded || CachedSelectedTrackState == ECFTargetTrackState::Estimated || CachedSelectedTrackState == ECFTargetTrackState::SignalLost)
	{
		SelectedColor = OccludedSelectedMarkerColor;
	}
	if (Text_SelectedMarker)
	{
		Text_SelectedMarker->SetText(CachedSelectedMarkerGlyphText);
		Text_SelectedMarker->SetColorAndOpacity(SelectedColor);
	}
		if (Text_SelectedInfo)
	{
		Text_SelectedInfo->SetText(FText::GetEmpty());
		Text_SelectedInfo->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Text_SelectedTrackState)
	{
		Text_SelectedTrackState->SetText(FText::GetEmpty());
		Text_SelectedTrackState->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCFTargetSelectWidget::RefreshMarkerProjection()
{
	// [v1.1.0] 현재 프레임의 후보 World 위치를 TargetPoint에서 다시 해석할 약한 Actor입니다.
	AActor* CandidateActor = CachedCandidateActor.Get();

	// [v1.1.0] 현재 프레임의 선택 World 위치를 TargetPoint에서 다시 해석할 약한 Actor입니다.
	AActor* SelectedActor = CachedSelectedActor.Get();

	// [v1.1.0] 후보의 의미상 표시 가능 여부는 이벤트 캐시 Actor와 현재 선택 Actor 관계에서 매 프레임 복구합니다.
	const bool bCandidateEligible = IsValid(CandidateActor) && CandidateActor != SelectedActor;
	bCandidateMarkerVisible = bCandidateEligible
		&& VerticalBox_CandidateRoot
		&& ProjectMarkerRoot(VerticalBox_CandidateRoot, UCFTargetPointComp::ResolveTargetPoint(CandidateActor).WorldLocation);
	if (VerticalBox_CandidateRoot)
	{
		VerticalBox_CandidateRoot->SetVisibility(bCandidateMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

		// [v1.2.0] 선택 Marker는 Safe Region 내부 4-Corner와 화면 가장자리 2-Corner 표현을 상호 배타적으로 사용합니다.
	const bool bSelectedEligible = IsValid(SelectedActor) && bCachedSelectedTargetValid;
	bSelectedEdgeMarkerVisible = false;
	bSelectedMarkerVisible = bSelectedEligible
		&& VerticalBox_SelectedRoot
		&& ProjectSelectedMarker(UCFTargetPointComp::ResolveTargetPoint(SelectedActor).WorldLocation);
	if (VerticalBox_SelectedRoot)
	{
		VerticalBox_SelectedRoot->SetVisibility(bSelectedMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (Image_SelectedEdgeMarker)
	{
		Image_SelectedEdgeMarker->SetVisibility(bSelectedEdgeMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

// [v1.2.0] persisted WBP를 저장 변경하지 않고 CanvasPanel_Root에 런타임 전용 Edge Image를 정확히 하나 보장합니다.
bool UCFTargetSelectWidget::EnsureSelectedEdgeMarkerWidget()
{
	if (!WidgetTree)
	{
		return false;
	}

	if (!Image_SelectedEdgeMarker)
	{
		Image_SelectedEdgeMarker = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_SelectedEdgeMarker"))));
	}

	// [v1.2.0] 기존 WBP_TargetSelect의 persisted 최상위 Canvas입니다.
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(FName(TEXT("CanvasPanel_Root"))));
	if (!RootCanvas)
	{
		return false;
	}

	if (!Image_SelectedEdgeMarker)
	{
		// [v1.2.0] 저장 Asset을 수정하지 않고 현재 Widget lifetime에만 존재하는 2-Corner Edge Image입니다.
		UImage* CreatedEdgeImage = NewObject<UImage>(RootCanvas, FName(TEXT("Image_SelectedEdgeMarker")));
		if (!CreatedEdgeImage)
		{
			return false;
		}

		// [v1.2.0] Edge Image를 full-screen TargetSelect Canvas에 배치하는 transient Canvas Slot입니다.
		UCanvasPanelSlot* EdgeCanvasSlot = RootCanvas->AddChildToCanvas(CreatedEdgeImage);
		if (!EdgeCanvasSlot)
		{
			return false;
		}
		EdgeCanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		EdgeCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		EdgeCanvasSlot->SetSize(ScreenEdgeMarkerBounds);
		EdgeCanvasSlot->SetAutoSize(false);
		EdgeCanvasSlot->SetZOrder(2);
		CreatedEdgeImage->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		CreatedEdgeImage->SetVisibility(ESlateVisibility::Collapsed);
		Image_SelectedEdgeMarker = CreatedEdgeImage;
	}

	if (!Image_SelectedEdgeMarker)
	{
		return false;
	}

	if (UCanvasPanelSlot* EdgeCanvasSlot = Cast<UCanvasPanelSlot>(Image_SelectedEdgeMarker->Slot))
	{
		EdgeCanvasSlot->SetSize(ScreenEdgeMarkerBounds);
		EdgeCanvasSlot->SetAutoSize(false);
	}

	if (SelectedEdgeBracketTexture)
	{
		Image_SelectedEdgeMarker->SetBrushFromTexture(SelectedEdgeBracketTexture, false);
	}
		Image_SelectedEdgeMarker->SetColorAndOpacity(ResolveCurrentScreenEdgeTint());
	return SelectedEdgeBracketTexture != nullptr;
}

// [v1.2.0] Candidate 또는 일반 On-screen Marker의 기존 월드→화면 투영을 유지합니다.
bool UCFTargetSelectWidget::ProjectMarkerRoot(UVerticalBox* MarkerRoot, const FVector& WorldLocation) const
{
	// [v1.1.0] Projection 시점에 Weak Reference에서 안전하게 해석한 현재 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = VehiclePawnRef.Get();
	if (!MarkerRoot || WorldLocation.ContainsNaN() || !IsValid(CurrentVehiclePawn))
	{
		return false;
	}

	// [v1.1.0] 현재 World Marker를 LocalPlayer Widget 좌표로 투영할 PlayerController입니다.
	APlayerController* PlayerController = Cast<APlayerController>(CurrentVehiclePawn->GetController());
	if (!PlayerController)
	{
		return false;
	}

	// [v1.1.0] ProjectWorldLocationToWidgetPosition이 반환한 LocalPlayer Widget 좌표입니다.
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, WorldLocation, ScreenPosition, false) || ScreenPosition.ContainsNaN())
	{
		return false;
	}

	// [v1.1.0] 현재 TargetSelect Widget이 사용하는 LocalPlayer Viewport Widget 크기입니다.
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();
	if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f || ScreenPosition.X < 0.0f || ScreenPosition.Y < 0.0f || ScreenPosition.X > ViewportSize.X || ScreenPosition.Y > ViewportSize.Y)
	{
		return false;
	}

	ApplyMarkerScreenPosition(MarkerRoot, ScreenPosition);
	return true;
}

// [v1.2.0] Selected Target을 Safe Region 내부 4-Corner Marker 또는 외곽 2-Corner Edge Marker로 분기합니다.
bool UCFTargetSelectWidget::ProjectSelectedMarker(const FVector& WorldLocation)
{
	bSelectedEdgeMarkerVisible = false;
	if (Image_SelectedEdgeMarker)
	{
		Image_SelectedEdgeMarker->SetVisibility(ESlateVisibility::Collapsed);
	}

	// [v1.2.0] Projection 시점에 Weak Reference에서 안전하게 해석한 현재 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = VehiclePawnRef.Get();
	if (!VerticalBox_SelectedRoot || WorldLocation.ContainsNaN() || !IsValid(CurrentVehiclePawn))
	{
		return false;
	}

	// [v1.2.0] Selected Target의 화면 위치와 Camera View Space 방향을 해석할 PlayerController입니다.
	APlayerController* PlayerController = Cast<APlayerController>(CurrentVehiclePawn->GetController());
	if (!PlayerController)
	{
		return false;
	}

	// [v1.2.0] Safe Region과 Edge Ray 교차 계산에 사용할 LocalPlayer Viewport Widget 크기입니다.
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();
	if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f || ViewportSize.ContainsNaN())
	{
		return false;
	}

	// [v1.2.0] 현재 Player View의 Camera World 위치입니다.
	FVector CameraLocation = FVector::ZeroVector;
	// [v1.2.0] 현재 Player View의 Camera World 회전입니다.
	FRotator CameraRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// [v1.2.0] Camera Forward/Right/Up 축에서 Selected Target 방향을 판정할 View Space 벡터입니다.
	const FVector CameraSpaceDirection = CameraRotation.UnrotateVector(WorldLocation - CameraLocation);
	if (CameraSpaceDirection.ContainsNaN())
	{
		return false;
	}

	// [v1.2.0] 앞쪽 Target에 한해 실제 Perspective Projection을 재사용해 정확한 2D 방향을 얻습니다.
	FVector2D ProjectedScreenPosition = FVector2D::ZeroVector;
		// [v1.2.0] 앞쪽 Target의 실제 Perspective Projection이 유효한 Widget 좌표를 제공했는지 여부입니다.
	const bool bProjectionSucceeded = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, WorldLocation, ProjectedScreenPosition, false)
		&& !ProjectedScreenPosition.ContainsNaN();
	// [v1.2.0] Camera Forward 축 기준으로 Target이 현재 View의 앞 반구에 있는지 여부입니다.
	const bool bInFrontOfCamera = CameraSpaceDirection.X > KINDA_SMALL_NUMBER;

	if (bInFrontOfCamera && bProjectionSucceeded && IsInsideSafeRegion(ProjectedScreenPosition, ViewportSize, SelectedEdgeSafeInset))
	{
		ApplyMarkerScreenPosition(VerticalBox_SelectedRoot, ProjectedScreenPosition);
		return true;
	}

	if (!EnsureSelectedEdgeMarkerWidget())
	{
		return false;
	}

	// [v1.2.0] Front-offscreen이면 실제 perspective projection 방향, BehindCamera면 Camera View Space 방향을 사용할 2D Edge Ray입니다.
	FVector2D ScreenEdgeDirection = FVector2D::ZeroVector;
	if (bInFrontOfCamera && bProjectionSucceeded)
	{
		ScreenEdgeDirection = ProjectedScreenPosition - (ViewportSize * 0.5f);
		if (!ScreenEdgeDirection.ContainsNaN() && ScreenEdgeDirection.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			ScreenEdgeDirection.Normalize();
		}
	}
	if (ScreenEdgeDirection.ContainsNaN() || ScreenEdgeDirection.SizeSquared() <= KINDA_SMALL_NUMBER)
	{
		ScreenEdgeDirection = ResolveCameraViewEdgeDirection(CameraSpaceDirection, CachedSelectedEdgeDirection);
	}

	// [v1.2.0] Safe Region 경계 위의 Edge Bracket 중심 위치입니다.
	FVector2D EdgeScreenPosition = FVector2D::ZeroVector;
	// [v1.2.0] Source Art의 TL→BR 45도 기준축을 실제 Target 방향으로 맞출 Render Transform 각도입니다.
	float EdgeRenderAngleDegrees = 0.0f;
	if (!ResolveScreenEdgePlacement(ScreenEdgeDirection, ViewportSize, SelectedEdgeSafeInset, ScreenEdgeMarkerBounds, EdgeScreenPosition, EdgeRenderAngleDegrees))
	{
		return false;
	}

		ApplyMarkerScreenPosition(Image_SelectedEdgeMarker, EdgeScreenPosition);
	Image_SelectedEdgeMarker->SetRenderTransformAngle(EdgeRenderAngleDegrees);
	Image_SelectedEdgeMarker->SetColorAndOpacity(ResolveCurrentScreenEdgeTint());
	Image_SelectedEdgeMarker->SetVisibility(ESlateVisibility::HitTestInvisible);
	CachedSelectedEdgeDirection = ScreenEdgeDirection;
	bSelectedEdgeMarkerVisible = true;
	return false;
}

// [v1.2.0] Widget 종류와 무관하게 Canvas Slot 또는 Render Translation에 검증된 화면 위치를 적용합니다.
void UCFTargetSelectWidget::ApplyMarkerScreenPosition(UWidget* MarkerWidget, const FVector2D& ScreenPosition)
{
	if (!MarkerWidget || ScreenPosition.ContainsNaN())
	{
		return;
	}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MarkerWidget->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		// [v1.2.0] 기존 Candidate/Selected VerticalBox는 TargetPoint 아래 기준 정렬을 유지하고 Edge Image는 자체 중심을 회전축으로 사용합니다.
		CanvasSlot->SetAlignment(Cast<UVerticalBox>(MarkerWidget) ? FVector2D(0.5f, 1.0f) : FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(ScreenPosition);
		return;
	}

	MarkerWidget->SetRenderTranslation(ScreenPosition);
}

// [v1.2.0] Projected Target Point가 Style Safe Region 안에 있는지 판정합니다.
bool UCFTargetSelectWidget::IsInsideSafeRegion(const FVector2D& ScreenPosition, const FVector2D& ViewportSize, const float SafeInset)
{
	if (ScreenPosition.ContainsNaN() || ViewportSize.ContainsNaN() || ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f || !FMath::IsFinite(SafeInset))
	{
		return false;
	}

	// [v1.2.0] Viewport 절반보다 커져 Safe Region이 역전되지 않도록 제한한 실제 Inset입니다.
	const float ClampedSafeInset = FMath::Clamp(SafeInset, 0.0f, FMath::Max(0.0f, (FMath::Min(ViewportSize.X, ViewportSize.Y) * 0.5f) - 1.0f));
	return ScreenPosition.X >= ClampedSafeInset
		&& ScreenPosition.Y >= ClampedSafeInset
		&& ScreenPosition.X <= (ViewportSize.X - ClampedSafeInset)
		&& ScreenPosition.Y <= (ViewportSize.Y - ClampedSafeInset);
}

// [v1.2.0] Camera View Space의 Right/Up 성분을 Edge 2D 방향으로 변환하고 정후방 특이점에서 이전 방향을 보존합니다.
FVector2D UCFTargetSelectWidget::ResolveCameraViewEdgeDirection(const FVector& CameraSpaceDirection, const FVector2D& PreviousStableDirection)
{
	if (CameraSpaceDirection.ContainsNaN())
	{
		return FVector2D::ZeroVector;
	}

	// [v1.2.0] Camera +Y Right를 Screen +X, Camera +Z Up을 Screen -Y로 변환한 원시 Edge 방향입니다.
	FVector2D ResolvedDirection(CameraSpaceDirection.Y, -CameraSpaceDirection.Z);
	if (ResolvedDirection.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		ResolvedDirection.Normalize();
		return ResolvedDirection;
	}

	if (!PreviousStableDirection.ContainsNaN() && PreviousStableDirection.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		ResolvedDirection = PreviousStableDirection;
		ResolvedDirection.Normalize();
		return ResolvedDirection;
	}

	// [v1.2.0] 최초 프레임의 정확한 정후방은 좌우 선택 근거가 없으므로 화면 하단을 결정론적 fallback으로 사용합니다.
	return CameraSpaceDirection.X < 0.0f ? FVector2D(0.0f, 1.0f) : FVector2D(0.0f, -1.0f);
}

// [v1.3.0] Provider Relation을 USER 승인 Screen-edge 의미색으로 변환합니다.
FLinearColor UCFTargetSelectWidget::ResolveScreenEdgeRelationColor(const ECFTargetRelation Relation, const FLinearColor& FriendlyColor, const FLinearColor& HostileColor, const FLinearColor& UnknownColor)
{
	switch (Relation)
	{
	case ECFTargetRelation::Friendly:
		return FriendlyColor;
	case ECFTargetRelation::Hostile:
		return HostileColor;
	case ECFTargetRelation::Neutral:
	case ECFTargetRelation::Unknown:
	default:
		return UnknownColor;
	}
}

// [v1.3.0] 현재 Provider ViewData의 Selected Target Relation을 읽어 Edge Marker Tint를 반환합니다.
FLinearColor UCFTargetSelectWidget::ResolveCurrentScreenEdgeTint() const
{
	if (!ScreenEdgeHUDDataProvider)
	{
		return UnknownEdgeMarkerColor;
	}

	// [v1.3.0] Provider가 Sensor Snapshot과 TargetSelect 상태를 이미 합성한 현재 Target ViewData입니다.
	const FCFTargetHUDData& TargetViewData = ScreenEdgeHUDDataProvider->GetCurrentViewDataRef().Target;
	return ResolveScreenEdgeRelationColor(TargetViewData.Relation, FriendlyEdgeMarkerColor, HostileEdgeMarkerColor, UnknownEdgeMarkerColor);
}

// [v1.2.0] 중심 Ray와 Safe Region 내부 경계를 교차시켜 Edge Bracket 위치와 Source Art 회전각을 계산합니다.
bool UCFTargetSelectWidget::ResolveScreenEdgePlacement(const FVector2D& ScreenDirection, const FVector2D& ViewportSize, const float SafeInset, const FVector2D& MarkerBounds, FVector2D& OutScreenPosition, float& OutRenderAngleDegrees)
{
	OutScreenPosition = FVector2D::ZeroVector;
	OutRenderAngleDegrees = 0.0f;
	if (ScreenDirection.ContainsNaN() || ViewportSize.ContainsNaN() || MarkerBounds.ContainsNaN()
		|| ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f
		|| MarkerBounds.X <= 0.0f || MarkerBounds.Y <= 0.0f
		|| !FMath::IsFinite(SafeInset))
	{
		return false;
	}

	// [v1.2.0] Safe Region Ray 교차에 사용할 정규화 Screen 방향입니다.
	FVector2D NormalizedDirection = ScreenDirection;
	if (NormalizedDirection.SizeSquared() <= KINDA_SMALL_NUMBER)
	{
		return false;
	}
	NormalizedDirection.Normalize();

	// [v1.2.0] Edge Bracket 전체 Bounds가 Safe Region 안에 남도록 Marker 반크기까지 제외한 중심 허용 반경입니다.
	const FVector2D SafeHalfExtent(
		FMath::Max(1.0f, (ViewportSize.X * 0.5f) - FMath::Max(0.0f, SafeInset) - (MarkerBounds.X * 0.5f)),
		FMath::Max(1.0f, (ViewportSize.Y * 0.5f) - FMath::Max(0.0f, SafeInset) - (MarkerBounds.Y * 0.5f)));

	// [v1.2.0] 가로 경계와 교차할 때 필요한 Ray 배율입니다.
	const float HorizontalScale = FMath::Abs(NormalizedDirection.X) > KINDA_SMALL_NUMBER
		? SafeHalfExtent.X / FMath::Abs(NormalizedDirection.X)
		: BIG_NUMBER;
	// [v1.2.0] 세로 경계와 교차할 때 필요한 Ray 배율입니다.
	const float VerticalScale = FMath::Abs(NormalizedDirection.Y) > KINDA_SMALL_NUMBER
		? SafeHalfExtent.Y / FMath::Abs(NormalizedDirection.Y)
		: BIG_NUMBER;
	// [v1.2.0] 첫 번째 Safe Region 경계 교차점까지의 Ray 배율입니다.
	const float EdgeScale = FMath::Min(HorizontalScale, VerticalScale);
	if (!FMath::IsFinite(EdgeScale) || EdgeScale <= 0.0f)
	{
		return false;
	}

	OutScreenPosition = (ViewportSize * 0.5f) + (NormalizedDirection * EdgeScale);
	// [v1.2.0] T_UI_RadarEdge Source Art의 TL→BR 기준축이 Screen +X에서 +45도이므로 실제 Edge 방향에 맞추는 상대 회전입니다.
	OutRenderAngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(NormalizedDirection.Y, NormalizedDirection.X)) - 45.0f;
	return !OutScreenPosition.ContainsNaN() && FMath::IsFinite(OutRenderAngleDegrees);
}



void UCFTargetSelectWidget::HandleTargetCandidateChanged(AActor*, AActor*, FCFTargetCandidate) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetChanged(AActor*, AActor*, FCFTargetDisplayInfo) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetCleared(AActor*, ECFTargetClearReason) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetValidityChanged(AActor*, bool) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetTrackStateChanged(AActor*, ECFTargetTrackState, ECFTargetTrackState) { RefreshFromTargetSelect(); }
