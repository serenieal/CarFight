// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-18
// Description: UI-P0-05 후보 및 선택 Target World Marker용 C++ 부모 위젯
// Changelog:
// - v1.1.0: UISubsystem Game Layer 수명에 맞춰 VehiclePawnRef를 Weak Reference로 전환하고 이름·거리·관계·TrackState 의미 텍스트를 Marker에서 제거.
// - v1.0.0: TargetSelectComp 이벤트 구독, 후보·선택 캐시, 월드 위치 투영, 거리·관계·추적 상태 표시를 추가.
// Migration:
// - v1.1.0부터 WBP_TargetSelect는 World Marker 표현만 담당하며 Target 의미 정보는 Production TargetPanel의 Provider/Presenter ViewData가 담당한다.
// - 내부 Actor Name을 Player-facing 이름으로 fallback하지 않는다.
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

		UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="타겟 HUD 갱신", ToolTip="현재 TargetSelectComp 상태를 다시 읽고 후보와 선택 World Marker의 상태, 위치와 가시성을 갱신합니다. 이름·거리·관계·Knowledge 텍스트는 Production TargetPanel이 담당합니다."))
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

		// [v1.1.0] Target Marker가 상태를 읽되 이전 Pawn lifetime을 소유하지 않는 현재 차량 Pawn 약한 참조입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="차량 Pawn 약한 참조", ToolTip="현재 Target Marker가 읽을 차량 Pawn의 약한 참조입니다. Pawn 수명을 소유하지 않으며 Rebind 또는 Pawn 소멸 시 안전하게 무효화됩니다."))
	TWeakObjectPtr<ACFVehiclePawn> VehiclePawnRef;

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

		// [v1.1.0] 후보·선택 의미 상태를 재계산하지 않고 World Marker 화면 투영만 매 프레임 갱신할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="Marker 투영 매 프레임 갱신", ToolTip="True이면 후보·선택 의미 상태는 이벤트 캐시를 유지하고 현재 TargetPoint의 화면 좌표 투영만 매 프레임 갱신합니다."))
	bool bAutoRefreshEveryTick = true;

private:
	void BindTargetSelectEvents();
	void UnbindTargetSelectEvents();
	void ClearCachedTargetState();
	void RefreshCachedTargetState();
	void RefreshWidgetTextAndStyle();
	void RefreshMarkerProjection();
		bool ProjectMarkerRoot(UVerticalBox* MarkerRoot, const FVector& WorldLocation) const;

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
