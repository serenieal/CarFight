// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-21
// Description: UI-P0-05 World Marker 수명을 보존하면서 UI-P0-08 Screen-Off Selected Target Edge Marker의 관계색·가독성을 개선한 C++ 부모 위젯
// Changelog:
// - v1.3.0: Screen-edge Bracket이 HUDDataProvider Target Relation을 그대로 소비해 Friendly=파랑, Hostile=빨강, Neutral/Unknown=회색으로 표시되고 Bounds를 48×36으로 확대. 관계 재판정·공용 Neutral palette 변경 없음.
// - v1.2.0: 선택 Target이 Safe Region 밖 또는 BehindCamera일 때 Camera View Space 기반 방향을 Safe Region 경계와 교차시켜 2-Corner Open Edge Bracket을 표시. Candidate offscreen hide와 TargetSelect Gameplay ownership은 유지.
// - v1.1.0: UISubsystem Game Layer 수명에 맞춰 VehiclePawnRef를 Weak Reference로 전환하고 이름·거리·관계·TrackState 의미 텍스트를 Marker에서 제거.
// - v1.0.0: TargetSelectComp 이벤트 구독, 후보·선택 캐시, 월드 위치 투영, 거리·관계·추적 상태 표시를 추가.
// Migration:
// - v1.1.0부터 WBP_TargetSelect는 World Marker 표현만 담당하며 Target 의미 정보는 Production TargetPanel의 Provider/Presenter ViewData가 담당한다.
// - 내부 Actor Name을 Player-facing 이름으로 fallback하지 않는다.
// - 후보와 선택은 각각 ◇와 ▣ 형태로 구분하며 가림 상태는 ▧ 형태로 표시한다.
// - v1.2.0부터 Candidate는 화면 밖에서 계속 숨기고, 유효한 Selected Target만 Safe Region 외곽 2-Corner Edge Bracket으로 방향을 표시한다.
// - Screen-edge Texture·Tint·SafeInset은 UISubsystem이 HUD Visual/Style Data에서 주입하며 이 Widget은 콘텐츠 경로를 직접 Load하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CFTargetSelectTypes.h"
#include "CFTargetSelectWidget.generated.h"

class AActor;
class ACFVehiclePawn;
class UCFHUDDataProvider;
class UCFTargetSelectComp;
class UImage;
class UTextBlock;
class UTexture2D;
class UVerticalBox;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFTargetSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="차량 Pawn 참조 설정", ToolTip="후보와 선택 상태를 읽을 차량 Pawn을 설정하고 TargetSelectComp 이벤트를 다시 연결합니다."))
		void SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef);

		// [v1.3.0] UISubsystem이 해석한 Edge Texture·Provider·관계색·Safe Region Inset을 World Marker에 주입합니다.
	void ConfigureScreenEdgePresentation(UTexture2D* InEdgeBracketTexture, UCFHUDDataProvider* InHUDDataProvider, const FLinearColor& InFriendlyEdgeColor, const FLinearColor& InHostileEdgeColor, const FLinearColor& InUnknownEdgeColor, float InSafeRegionInset);

		UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="타겟 HUD 갱신", ToolTip="현재 TargetSelectComp 상태를 다시 읽고 후보와 선택 World Marker의 상태, 위치와 가시성을 갱신합니다. 이름·거리·관계·Knowledge 텍스트는 Production TargetPanel이 담당합니다."))
	void RefreshFromTargetSelect();

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	bool IsCandidateMarkerVisible() const { return bCandidateMarkerVisible; }

		UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	bool IsSelectedMarkerVisible() const { return bSelectedMarkerVisible; }

	// [v1.2.0] 현재 선택 Target이 Screen-edge Bracket으로 표시 중인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	bool IsSelectedEdgeMarkerVisible() const { return bSelectedEdgeMarkerVisible; }

	// [v1.2.0] 마지막으로 안정화된 선택 Target Screen-edge 2D 방향을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|HUD|Debug")
	FVector2D GetSelectedEdgeDirection() const { return CachedSelectedEdgeDirection; }

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

		// [v1.3.0] USER 가독성 피드백을 반영한 Screen-edge 2-Corner Open Bracket 기본 Bounds입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD|Style", meta=(DisplayName="화면 가장자리 Bracket 크기", ToolTip="Selected Target Screen-edge 2-Corner Bracket의 기본 표시 Bounds입니다. 1080p USER 가독성 기준 48×36입니다."))
	FVector2D ScreenEdgeMarkerBounds = FVector2D(48.0f, 36.0f);

		// [v1.1.0] 후보·선택 의미 상태를 재계산하지 않고 World Marker 화면 투영만 매 프레임 갱신할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|HUD", meta=(DisplayName="Marker 투영 매 프레임 갱신", ToolTip="True이면 후보·선택 의미 상태는 이벤트 캐시를 유지하고 현재 TargetPoint의 화면 좌표 투영만 매 프레임 갱신합니다."))
	bool bAutoRefreshEveryTick = true;

private:
	friend class FCFUIScreenEdgeTargetMarkerContractTest;

	void BindTargetSelectEvents();
	void UnbindTargetSelectEvents();
	void ClearCachedTargetState();
	void RefreshCachedTargetState();
	void RefreshWidgetTextAndStyle();
	void RefreshMarkerProjection();

	// [v1.2.0] 저장된 CanvasPanel_Root 아래에 런타임 전용 Selected Edge Image를 정확히 하나 보장합니다.
	bool EnsureSelectedEdgeMarkerWidget();

	// [v1.2.0] Candidate 또는 일반 On-screen Marker의 기존 월드→화면 투영과 Viewport 범위 검사를 수행합니다.
	bool ProjectMarkerRoot(UVerticalBox* MarkerRoot, const FVector& WorldLocation) const;

	// [v1.2.0] 유효한 Selected Target을 On-screen 4-Corner 위치 또는 Screen-edge 2-Corner 방향 표현으로 분기합니다.
	bool ProjectSelectedMarker(const FVector& WorldLocation);

	// [v1.2.0] 이미 검증한 Widget 좌표를 Canvas/Render 위치에 적용합니다.
	static void ApplyMarkerScreenPosition(UWidget* MarkerWidget, const FVector2D& ScreenPosition);

	// [v1.2.0] 주어진 Widget 좌표가 Safe Region 내부인지 판정합니다.
	static bool IsInsideSafeRegion(const FVector2D& ScreenPosition, const FVector2D& ViewportSize, float SafeInset);

	// [v1.2.0] Camera View Space의 Right/Up 성분을 안정적인 Screen-edge 2D 방향으로 변환하고 정후방 특이점에서는 이전 방향을 보존합니다.
	static FVector2D ResolveCameraViewEdgeDirection(const FVector& CameraSpaceDirection, const FVector2D& PreviousStableDirection);

		// [v1.2.0] 화면 중심에서 방향 Ray를 Safe Region 내부 경계와 교차시켜 Bracket 중심 위치와 Source Art 회전각을 계산합니다.
	static bool ResolveScreenEdgePlacement(const FVector2D& ScreenDirection, const FVector2D& ViewportSize, float SafeInset, const FVector2D& MarkerBounds, FVector2D& OutScreenPosition, float& OutRenderAngleDegrees);

	// [v1.3.0] Provider가 이미 판정한 Target Relation을 USER 승인 Screen-edge 색 규칙으로 변환합니다.
	static FLinearColor ResolveScreenEdgeRelationColor(ECFTargetRelation Relation, const FLinearColor& FriendlyColor, const FLinearColor& HostileColor, const FLinearColor& UnknownColor);

	// [v1.3.0] 현재 HUDDataProvider의 Selected Target Relation을 읽어 실제 Edge Bracket Tint를 반환합니다.
	FLinearColor ResolveCurrentScreenEdgeTint() const;

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

	// [v1.2.0] Selected Target의 Screen-edge Bracket이 현재 표시 중인지 나타냅니다.
	bool bSelectedEdgeMarkerVisible = false;

	// [v1.2.0] BehindCamera 정후방 특이점에서도 방향이 화면 중심을 가로질러 튀지 않게 유지할 마지막 안정 2D 방향입니다.
	FVector2D CachedSelectedEdgeDirection = FVector2D::ZeroVector;

	// [v1.2.0] UISubsystem이 HUD VisualData에서 로드해 주입한 2-Corner Edge Bracket Texture입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> SelectedEdgeBracketTexture = nullptr;

		// [v1.3.0] 현재 선택 Target Relation을 중복 판정 없이 읽을 HUD Provider입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFHUDDataProvider> ScreenEdgeHUDDataProvider = nullptr;

	// [v1.3.0] Friendly Screen-edge Marker에 사용할 Style 관계색입니다.
	FLinearColor FriendlyEdgeMarkerColor = FLinearColor(0.325f, 0.663f, 1.0f, 1.0f);

	// [v1.3.0] Hostile Screen-edge Marker에 사용할 Style 관계색입니다.
	FLinearColor HostileEdgeMarkerColor = FLinearColor(1.0f, 0.392f, 0.310f, 1.0f);

	// [v1.3.0] Neutral/Unknown Screen-edge Marker가 공통으로 사용할 회색 Style 관계색입니다.
	FLinearColor UnknownEdgeMarkerColor = FLinearColor(0.655f, 0.678f, 0.702f, 1.0f);

	// [v1.2.0] 현재 UI Style의 Safe Region Token에서 주입된 Screen-edge inset입니다.
	float SelectedEdgeSafeInset = 0.0f;

	// [v1.2.0] persisted WBP에는 저장하지 않고 CanvasPanel_Root 아래 런타임에서만 생성하는 Edge Bracket Image입니다.
	UPROPERTY(Transient)
	TObjectPtr<UImage> Image_SelectedEdgeMarker = nullptr;

	bool bCachedSelectedTargetValid = false;
};
