// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-18
// Description: UI-P0-05 후보 및 선택 Target World Marker용 C++ 부모 위젯 구현
// Changelog:
// - v1.1.0: VehiclePawnRef Weak Binding, Marker-only 표시, Player-facing 의미 텍스트 제거, 이벤트 기반 의미 캐시 + Projection-only Tick, 화면 재진입 Marker 복구.
// Migration:
// - Target 의미 텍스트는 Production TargetPanel이 Provider/Presenter ViewData로 표시하며 이 Widget은 World Marker만 투영합니다.

#include "UI/CFTargetSelectWidget.h"

#include "CFTargetPointComp.h"
#include "CFTargetSelectComp.h"
#include "CFVehiclePawn.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
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

void UCFTargetSelectWidget::RefreshFromTargetSelect()
{
	RefreshCachedTargetState();
	RefreshWidgetTextAndStyle();
	RefreshMarkerProjection();
}

void UCFTargetSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
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

	// [v1.1.0] 선택 Marker의 의미상 표시 가능 여부는 이벤트로 캐시된 유효성만 사용하고 현재 위치만 프레임 단위로 다시 투영합니다.
	const bool bSelectedEligible = IsValid(SelectedActor) && bCachedSelectedTargetValid;
	bSelectedMarkerVisible = bSelectedEligible
		&& VerticalBox_SelectedRoot
		&& ProjectMarkerRoot(VerticalBox_SelectedRoot, UCFTargetPointComp::ResolveTargetPoint(SelectedActor).WorldLocation);
	if (VerticalBox_SelectedRoot)
	{
		VerticalBox_SelectedRoot->SetVisibility(bSelectedMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

bool UCFTargetSelectWidget::ProjectMarkerRoot(UVerticalBox* MarkerRoot, const FVector& WorldLocation) const
{
	// [v1.1.0] Projection 시점에 Weak Reference에서 안전하게 해석한 현재 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = VehiclePawnRef.Get();
	if (!MarkerRoot || WorldLocation.ContainsNaN() || !IsValid(CurrentVehiclePawn))
	{
		return false;
	}
	APlayerController* PlayerController = Cast<APlayerController>(CurrentVehiclePawn->GetController());
	if (!PlayerController)
	{
		return false;
	}
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, WorldLocation, ScreenPosition, false) || ScreenPosition.ContainsNaN())
	{
		return false;
	}
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();
	if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f || ScreenPosition.X < 0.0f || ScreenPosition.Y < 0.0f || ScreenPosition.X > ViewportSize.X || ScreenPosition.Y > ViewportSize.Y)
	{
		return false;
	}
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MarkerRoot->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		CanvasSlot->SetPosition(ScreenPosition);
	}
	else
	{
		MarkerRoot->SetRenderTranslation(ScreenPosition);
	}
	return true;
}



void UCFTargetSelectWidget::HandleTargetCandidateChanged(AActor*, AActor*, FCFTargetCandidate) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetChanged(AActor*, AActor*, FCFTargetDisplayInfo) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetCleared(AActor*, ECFTargetClearReason) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetValidityChanged(AActor*, bool) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetTrackStateChanged(AActor*, ECFTargetTrackState, ECFTargetTrackState) { RefreshFromTargetSelect(); }
