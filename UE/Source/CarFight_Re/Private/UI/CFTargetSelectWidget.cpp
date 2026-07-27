// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-24
// Description: TS-P0-06 후보 및 선택 타겟 HUD용 C++ 부모 위젯 구현

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
	if (VehiclePawnRef == InVehiclePawnRef)
	{
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
		RefreshFromTargetSelect();
	}
}

void UCFTargetSelectWidget::BindTargetSelectEvents()
{
	UCFTargetSelectComp* TargetSelectComp = IsValid(VehiclePawnRef) ? VehiclePawnRef->GetTargetSelectComp() : nullptr;
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
	UCFTargetSelectComp* TargetSelectComp = IsValid(VehiclePawnRef) ? VehiclePawnRef->GetTargetSelectComp() : nullptr;
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

	float CandidateDistanceCm = CachedCandidateData.WorldDistanceCm;
	if (CandidateDistanceCm <= 0.0f && IsValid(VehiclePawnRef) && IsValid(CandidateActor))
	{
		CandidateDistanceCm = FVector::Distance(VehiclePawnRef->GetActorLocation(), CachedCandidateData.TargetWorldLocation);
	}
	CachedCandidateInfoText = bCandidateMarkerVisible
		? BuildTargetInfoText(TEXT("후보"), CandidateActor, CachedCandidateData.DisplayInfo, CandidateDistanceCm)
		: FText::GetEmpty();

	float SelectedDistanceCm = 0.0f;
	if (IsValid(VehiclePawnRef) && IsValid(SelectedActor))
	{
		SelectedDistanceCm = FVector::Distance(VehiclePawnRef->GetActorLocation(), UCFTargetPointComp::ResolveTargetPoint(SelectedActor).WorldLocation);
	}
	CachedSelectedInfoText = bSelectedMarkerVisible
		? BuildTargetInfoText(TEXT("선택"), SelectedActor, CachedSelectedDisplayInfo, SelectedDistanceCm)
		: FText::GetEmpty();
	CachedSelectedTrackStateText = bSelectedMarkerVisible
		? FText::FromString(FString::Printf(TEXT("%s · %s"), *GetRelationDisplayText(CachedSelectedDisplayInfo.Relation).ToString(), *GetTrackStateDisplayText(CachedSelectedTrackState).ToString()))
		: FText::GetEmpty();
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
		Text_CandidateInfo->SetText(CachedCandidateInfoText);
		Text_CandidateInfo->SetColorAndOpacity(CandidateMarkerColor);
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
		Text_SelectedInfo->SetText(CachedSelectedInfoText);
		Text_SelectedInfo->SetColorAndOpacity(SelectedColor);
	}
	if (Text_SelectedTrackState)
	{
		Text_SelectedTrackState->SetText(CachedSelectedTrackStateText);
		Text_SelectedTrackState->SetColorAndOpacity(SelectedColor);
	}
}

void UCFTargetSelectWidget::RefreshMarkerProjection()
{
	if (bCandidateMarkerVisible && VerticalBox_CandidateRoot)
	{
		bCandidateMarkerVisible = ProjectMarkerRoot(VerticalBox_CandidateRoot, CachedCandidateData.TargetWorldLocation);
		VerticalBox_CandidateRoot->SetVisibility(bCandidateMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	AActor* SelectedActor = CachedSelectedActor.Get();
	if (bSelectedMarkerVisible && VerticalBox_SelectedRoot && IsValid(SelectedActor))
	{
		bSelectedMarkerVisible = ProjectMarkerRoot(VerticalBox_SelectedRoot, UCFTargetPointComp::ResolveTargetPoint(SelectedActor).WorldLocation);
		VerticalBox_SelectedRoot->SetVisibility(bSelectedMarkerVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

bool UCFTargetSelectWidget::ProjectMarkerRoot(UVerticalBox* MarkerRoot, const FVector& WorldLocation) const
{
	if (!MarkerRoot || WorldLocation.ContainsNaN() || !IsValid(VehiclePawnRef))
	{
		return false;
	}
	APlayerController* PlayerController = Cast<APlayerController>(VehiclePawnRef->GetController());
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

FText UCFTargetSelectWidget::BuildTargetInfoText(const TCHAR* Prefix, AActor* TargetActor, const FCFTargetDisplayInfo& DisplayInfo, const float DistanceCm) const
{
	const FString DisplayName = DisplayInfo.DisplayName.IsEmpty() ? (IsValid(TargetActor) ? TargetActor->GetName() : TEXT("Unknown")) : DisplayInfo.DisplayName.ToString();
	return FText::FromString(FString::Printf(TEXT("%s · %s · %.0f m"), Prefix, *DisplayName, FMath::Max(0.0f, DistanceCm) / 100.0f));
}

FText UCFTargetSelectWidget::GetRelationDisplayText(const ECFTargetRelation Relation) const
{
	switch (Relation)
	{
	case ECFTargetRelation::Friendly: return FText::FromString(TEXT("아군"));
	case ECFTargetRelation::Neutral: return FText::FromString(TEXT("중립"));
	case ECFTargetRelation::Hostile: return FText::FromString(TEXT("적대"));
	default: return FText::FromString(TEXT("미확인"));
	}
}

FText UCFTargetSelectWidget::GetTrackStateDisplayText(const ECFTargetTrackState TrackState) const
{
	switch (TrackState)
	{
	case ECFTargetTrackState::Visible: return FText::FromString(TEXT("가시"));
	case ECFTargetTrackState::Occluded: return FText::FromString(TEXT("가림"));
	case ECFTargetTrackState::Estimated: return FText::FromString(TEXT("추정 추적"));
	case ECFTargetTrackState::SignalLost: return FText::FromString(TEXT("신호 손실"));
	default: return FText::FromString(TEXT("무효"));
	}
}

void UCFTargetSelectWidget::HandleTargetCandidateChanged(AActor*, AActor*, FCFTargetCandidate) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetChanged(AActor*, AActor*, FCFTargetDisplayInfo) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetCleared(AActor*, ECFTargetClearReason) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetValidityChanged(AActor*, bool) { RefreshFromTargetSelect(); }
void UCFTargetSelectWidget::HandleSelectedTargetTrackStateChanged(AActor*, ECFTargetTrackState, ECFTargetTrackState) { RefreshFromTargetSelect(); }
