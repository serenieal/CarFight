// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-24
// Description: TS-P0-06 후보 및 선택 타겟 HUD용 C++ 부모 위젯
// Changelog:
// - v1.0.0: TargetSelectComp 이벤트 구독, 후보·선택 캐시, 월드 위치 투영, 거리·관계·추적 상태 표시를 추가.
// Migration:
// - 실제 배치와 스타일은 WBP_TargetSelect가 담당하고 C++는 상태와 투영만 담당한다.
// - 후보와 선택은 각각 ◇와 ▣ 형태로 구분하며 가림 상태는 ▧ 형태로 표시한다.
// - 화면 밖 대상은 P0에서 마커만 숨기고 선택 상태는 유지한다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CFTargetSelectTypes.h"
#include "CFTargetSelectWidget.generated.h"

class AActor;
class ACFVehiclePawn;
class UCFTargetSelectComp;
class UTextBlock;
class UVerticalBox;

UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFTargetSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="차량 Pawn 참조 설정", ToolTip="후보와 선택 상태를 읽을 차량 Pawn을 설정하고 TargetSelectComp 이벤트를 다시 연결합니다."))
	void SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef);

	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="타겟 HUD 갱신", ToolTip="현재 TargetSelectComp 상태를 다시 읽고 후보와 선택 마커의 텍스트, 위치와 가시성을 갱신합니다."))
	void RefreshFromTargetSelect();

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	bool IsCandidateMarkerVisible() const { return bCandidateMarkerVisible; }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	bool IsSelectedMarkerVisible() const { return bSelectedMarkerVisible; }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	AActor* GetCachedCandidateActor() const { return CachedCandidateActor.Get(); }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	AActor* GetCachedSelectedActor() const { return CachedSelectedActor.Get(); }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	FText GetCandidateInfoText() const { return CachedCandidateInfoText; }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	FText GetSelectedInfoText() const { return CachedSelectedInfoText; }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	FText GetSelectedTrackStateText() const { return CachedSelectedTrackStateText; }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	FText GetSelectedMarkerGlyphText() const { return CachedSelectedMarkerGlyphText; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_CandidateRoot = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CandidateMarker = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CandidateInfo = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_SelectedRoot = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SelectedMarker = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SelectedInfo = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SelectedTrackState = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="CarFight|TargetSelect|HUD")
	TObjectPtr<ACFVehiclePawn> VehiclePawnRef = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="CarFight|TargetSelect|HUD")
	FCFTargetCandidate CachedCandidateData;

	UPROPERTY(BlueprintReadOnly, Category="CarFight|TargetSelect|HUD")
	FCFTargetDisplayInfo CachedSelectedDisplayInfo;

	UPROPERTY(BlueprintReadOnly, Category="CarFight|TargetSelect|HUD")
	ECFTargetTrackState CachedSelectedTrackState = ECFTargetTrackState::Invalid;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style", meta=(DisplayName="후보 마커 문자"))
	FText CandidateMarkerGlyph = FText::FromString(TEXT("◇"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style", meta=(DisplayName="선택 마커 문자"))
	FText SelectedMarkerGlyph = FText::FromString(TEXT("▣"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style", meta=(DisplayName="가림 선택 마커 문자"))
	FText OccludedSelectedMarkerGlyph = FText::FromString(TEXT("▧"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style")
	FLinearColor CandidateMarkerColor = FLinearColor(0.25f, 0.90f, 1.0f, 0.90f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style")
	FLinearColor SelectedMarkerColor = FLinearColor(1.0f, 0.85f, 0.20f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style")
	FLinearColor OccludedSelectedMarkerColor = FLinearColor(1.0f, 0.50f, 0.15f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style")
	FLinearColor InvalidSelectedMarkerColor = FLinearColor(1.0f, 0.20f, 0.20f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD")
	bool bAutoRefreshEveryTick = true;

private:
	void BindTargetSelectEvents();
	void UnbindTargetSelectEvents();
	void ClearCachedTargetState();
	void RefreshCachedTargetState();
	void RefreshWidgetTextAndStyle();
	void RefreshMarkerProjection();
	bool ProjectMarkerRoot(UVerticalBox* MarkerRoot, const FVector& WorldLocation) const;
	FText BuildTargetInfoText(const TCHAR* Prefix, AActor* TargetActor, const FCFTargetDisplayInfo& DisplayInfo, float DistanceCm) const;
	FText GetRelationDisplayText(ECFTargetRelation Relation) const;
	FText GetTrackStateDisplayText(ECFTargetTrackState TrackState) const;

	UFUNCTION()
	void HandleTargetCandidateChanged(AActor* PreviousCandidate, AActor* NewCandidate, FCFTargetCandidate CandidateData);

	UFUNCTION()
	void HandleSelectedTargetChanged(AActor* PreviousTarget, AActor* NewTarget, FCFTargetDisplayInfo DisplayInfo);

	UFUNCTION()
	void HandleSelectedTargetCleared(AActor* ClearedTarget, ECFTargetClearReason ClearReason);

	UFUNCTION()
	void HandleSelectedTargetValidityChanged(AActor* TargetActor, bool bIsValidTarget);

	UFUNCTION()
	void HandleSelectedTargetTrackStateChanged(AActor* TargetActor, ECFTargetTrackState PreviousState, ECFTargetTrackState NewState);

	TWeakObjectPtr<UCFTargetSelectComp> BoundTargetSelectComp;
	TWeakObjectPtr<AActor> CachedCandidateActor;
	TWeakObjectPtr<AActor> CachedSelectedActor;
	FText CachedCandidateInfoText;
	FText CachedSelectedInfoText;
	FText CachedSelectedTrackStateText;
	FText CachedSelectedMarkerGlyphText;
	bool bCandidateMarkerVisible = false;
	bool bSelectedMarkerVisible = false;
	bool bCachedSelectedTargetValid = false;
};
