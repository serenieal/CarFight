// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-07-10
// Description: Aim Reticle UI용 C++ 부모 위젯 클래스입니다.
// Changelog:
// - v1.4.0: 선택적 Reticle 이미지와 FireFeedback TextBlock에 상태별 색상을 안전하게 적용.
// - v1.3.0: Pawn FireFeedback ViewData를 읽어 Reticle 상태와 선택적 피드백 TextBlock에 반영.
// Migration:
// - 기존 Reticle TextBlock은 유지한다.
// - 신규 Text_FireFeedbackState / Text_FireFeedbackHint / Text_Cooldown은 선택 사항이며 WBP에 없어도 동작한다.
// - Image_CenterDot / Image_LeftBracket / Image_RightBracket / Image_TopBracket / Image_BottomBracket과 Text_OutOfArcWarning은 선택 사항이며, 추가 시 Is Variable을 활성화한다.
// Scope: VehicleAimComp의 Reticle 상태를 읽어 선택적 TextBlock과 위젯 가시성을 갱신합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleFireFeedbackTypes.h"
#include "CFAimReticleWidget.generated.h"

class ACFVehiclePawn;
class UImage;
class UTextBlock;

/**
 * Aim Reticle UI용 C++ 부모 위젯 클래스입니다.
 * - 실제 배치와 스타일은 WBP 자식이 담당합니다.
 * - C++는 Pawn/AimComp 상태 읽기와 안전한 표시값 갱신만 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFAimReticleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] Reticle이 읽을 차량 Pawn 참조를 설정하고 즉시 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim|Reticle", meta=(DisplayName="차량 Pawn 참조 설정 (SetVehiclePawnRef)", ToolTip="Reticle UI가 조준 상태를 읽어올 차량 Pawn 참조를 설정하고 즉시 갱신합니다."))
	void SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef);

	// [v1.0.0] 현재 Pawn의 VehicleAimComp에서 최신 Reticle 상태를 읽어 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim|Reticle", meta=(DisplayName="Pawn에서 Reticle 갱신 (RefreshFromPawn)", ToolTip="현재 차량 Pawn의 VehicleAimComp에서 최신 Reticle 상태를 읽어 UI 표시값을 갱신합니다."))
	void RefreshFromPawn();

	// [v1.0.0] 전달받은 Reticle 상태를 캐시와 선택적 텍스트 위젯에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim|Reticle", meta=(DisplayName="Reticle 상태 적용 (ApplyReticleState)", ToolTip="전달받은 Reticle 상태를 캐시와 선택적 텍스트 위젯에 반영합니다."))
	void ApplyReticleState(ECFVehicleReticleState InReticleState);

	// [v1.0.0] 현재 Reticle 상태에 따라 위젯 가시성을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim|Reticle", meta=(DisplayName="Reticle 가시성 갱신 (UpdateReticleVisibility)", ToolTip="현재 Reticle 상태에 따라 위젯 가시성을 갱신합니다. Hidden 상태에서는 숨깁니다."))
	void UpdateReticleVisibility();

	// [v1.0.0] 현재 Reticle 상태를 UI 표시용 텍스트로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim|Reticle", meta=(DisplayName="Reticle 상태 표시 텍스트 반환 (GetReticleStateDisplayText)", ToolTip="현재 Reticle 상태를 UI에 표시하기 쉬운 텍스트로 반환합니다."))
	FText GetReticleStateDisplayText() const;

	// [v1.0.0] 현재 발사 가능 캐시를 UI 표시용 텍스트로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim|Reticle", meta=(DisplayName="발사 가능 표시 텍스트 반환 (GetCanFireDisplayText)", ToolTip="현재 발사 가능 캐시를 UI에 표시하기 쉬운 텍스트로 반환합니다."))
	FText GetCanFireDisplayText() const;

protected:
	// [v1.0.0] 위젯 생성 직후 현재 참조 기준으로 첫 Reticle 갱신을 수행합니다.
	virtual void NativeConstruct() override;

	// [v1.0.0] 옵션이 켜져 있으면 매 프레임 Pawn 기준 Reticle 갱신을 수행합니다.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// [v1.0.0] Reticle 상태를 표시할 선택적 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ReticleState = nullptr;

	// [v1.0.0] 발사 가능 여부를 표시할 선택적 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CanFire = nullptr;

	// [v1.0.0] Reticle 보조 설명을 표시할 선택적 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ReticleHint = nullptr;

	// [v1.3.0] 현재 발사 피드백 상태를 표시할 선택적 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_FireFeedbackState = nullptr;

	// [v1.3.0] 현재 발사 피드백 보조 설명을 표시할 선택적 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_FireFeedbackHint = nullptr;

	// [v1.3.0] 현재 남은 쿨다운 시간을 표시할 선택적 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Cooldown = nullptr;

	// [v1.4.0] Reticle 중앙점을 표시할 선택적 이미지 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_CenterDot = nullptr;

	// [v1.4.0] Reticle 좌측 브라켓을 표시할 선택적 이미지 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_LeftBracket = nullptr;

	// [v1.4.0] Reticle 우측 브라켓을 표시할 선택적 이미지 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_RightBracket = nullptr;

	// [v1.4.0] Reticle 상단 브라켓을 표시할 선택적 이미지 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_TopBracket = nullptr;

	// [v1.4.0] Reticle 하단 브라켓을 표시할 선택적 이미지 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_BottomBracket = nullptr;

	// [v1.4.0] 조준각 경고를 보조 표시할 선택적 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_OutOfArcWarning = nullptr;

	// [v1.0.0] Reticle이 조준 상태를 읽어올 차량 Pawn 참조입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle", meta=(DisplayName="차량 Pawn 참조 (VehiclePawnRef)", ToolTip="현재 Reticle UI가 조준 상태를 읽어올 차량 Pawn 참조입니다."))
	TObjectPtr<ACFVehiclePawn> VehiclePawnRef = nullptr;

	// [v1.0.0] 현재 Reticle에 적용 중인 상태 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle", meta=(DisplayName="Reticle 상태 캐시 (CachedReticleState)", ToolTip="현재 Reticle UI에 적용 중인 조준점 상태 캐시입니다."))
	ECFVehicleReticleState CachedReticleState = ECFVehicleReticleState::Hidden;

	// [v1.0.0] 현재 Reticle에 표시할 발사 가능 여부 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle", meta=(DisplayName="발사 가능 캐시 (bCachedCanFire)", ToolTip="현재 Reticle UI에 표시할 발사 가능 여부 캐시입니다. 서버 최종 판정이 아닙니다."))
	bool bCachedCanFire = false;

	// [v1.3.0] 마지막으로 Pawn에서 읽은 FireFeedback 표시 데이터입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle|FireFeedback", meta=(DisplayName="FireFeedback 표시 데이터 캐시 (CachedFireFeedbackViewData)", ToolTip="현재 Reticle UI가 표시 중인 로컬 발사 피드백 표시 데이터입니다."))
	FCFVehicleFireFeedbackViewData CachedFireFeedbackViewData;

	// [v1.4.0] Ready 상태 주 Reticle과 기본 텍스트에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="준비 Reticle 색상 (ReadyReticleColor)", ToolTip="Ready 상태일 때 주 Reticle과 기본 피드백 텍스트에 적용할 색상입니다."))
	FLinearColor ReadyReticleColor = FLinearColor::White;

	// [v1.4.0] 정상 발사 성공을 짧게 강조할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="발사 성공 Reticle 색상 (FireSuccessReticleColor)", ToolTip="활성 FireSuccess 피드백에 적용할 짧은 성공 강조 색상입니다."))
	FLinearColor FireSuccessReticleColor = FLinearColor(0.35f, 1.0f, 0.55f, 1.0f);

	// [v1.4.0] 쿨다운 상태에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="쿨다운 Reticle 색상 (CooldownReticleColor)", ToolTip="Cooldown 상태와 쿨다운 시간 텍스트에 적용할 색상입니다."))
	FLinearColor CooldownReticleColor = FLinearColor(0.25f, 0.60f, 1.0f, 1.0f);

	// [v1.4.0] 발사 처리 중 상태에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="발사 처리 Reticle 색상 (FirePendingReticleColor)", ToolTip="FirePending 상태에 적용할 청록색 계열의 색상입니다."))
	FLinearColor FirePendingReticleColor = FLinearColor(0.10f, 0.85f, 0.90f, 1.0f);

	// [v1.4.0] 발사 거부 상태에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="발사 거부 Reticle 색상 (FireRejectedReticleColor)", ToolTip="FireRejected 상태에 적용할 빨간색 계열의 색상입니다."))
	FLinearColor FireRejectedReticleColor = FLinearColor(1.0f, 0.25f, 0.25f, 1.0f);

	// [v1.4.0] 무기 없음 상태에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="무기 없음 Reticle 색상 (NoWeaponReticleColor)", ToolTip="NoWeapon 상태에 적용할 회색 계열의 색상입니다."))
	FLinearColor NoWeaponReticleColor = FLinearColor(0.55f, 0.55f, 0.58f, 1.0f);

	// [v1.4.0] 조준선 가림 상태에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="조준 가림 Reticle 색상 (AimBlockedReticleColor)", ToolTip="Blocked 또는 AimBlocked 상태에 적용할 주황색 계열의 색상입니다."))
	FLinearColor AimBlockedReticleColor = FLinearColor(1.0f, 0.55f, 0.12f, 1.0f);

	// [v1.4.0] 조준각 보조 경고에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="각도 경고 색상 (OutOfArcWarningColor)", ToolTip="Text_OutOfArcWarning에 적용할 노란색 계열의 보조 경고 색상입니다."))
	FLinearColor OutOfArcWarningColor = FLinearColor(1.0f, 0.85f, 0.10f, 1.0f);

	// [v1.4.0] 재장전 상태에 사용할 색상입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="CarFight|Aim|Reticle|Style", meta=(DisplayName="재장전 Reticle 색상 (ReloadingReticleColor)", ToolTip="Reloading 상태에 적용할 청록색 계열의 색상입니다."))
	FLinearColor ReloadingReticleColor = FLinearColor(0.20f, 0.70f, 0.90f, 1.0f);

	// [v1.0.0] True이면 NativeTick에서 Reticle을 자동 갱신합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Aim|Reticle", meta=(DisplayName="매 프레임 자동 갱신 (bAutoRefreshEveryTick)", ToolTip="True이면 NativeTick에서 현재 Pawn 기준 Reticle UI를 자동 갱신합니다."))
	bool bAutoRefreshEveryTick = true;

private:
	// [v1.3.0] FireFeedback 표시 데이터를 캐시에 저장하고 선택적 TextBlock에 반영합니다.
	void ApplyFireFeedbackViewData(const FCFVehicleFireFeedbackViewData& InViewData);

	// [v1.0.0] 현재 캐시값을 선택적 TextBlock들에 반영합니다.
	void RefreshTextBlocks();

	// [v1.4.0] 현재 캐시값에 맞는 이미지와 피드백 텍스트 색상을 안전하게 갱신합니다.
	void RefreshVisualStyle();

	// [v1.4.0] 현재 Reticle 상태와 활성 FireFeedback 기준의 주 Reticle 색상을 반환합니다.
	FLinearColor GetMainReticleColor() const;

	// [v1.4.0] 현재 활성 FireFeedback 기준의 피드백 텍스트 색상을 반환합니다.
	FLinearColor GetFireFeedbackTextColor() const;

	// [v1.4.0] 전달된 Reticle 상태에 대응하는 기본 색상을 반환합니다.
	FLinearColor GetReticleStateColor(ECFVehicleReticleState InReticleState) const;

	// [v1.0.0] Reticle 상태별 보조 설명 텍스트를 반환합니다.
	FText GetReticleHintDisplayText() const;

	// [v1.3.0] FireFeedback 상태를 UI 표시용 한국어 텍스트로 변환합니다.
	FText GetFireFeedbackStateDisplayText(ECFVehicleFireFeedbackState InFeedbackState) const;

	// [v1.3.0] FireFeedback 상태를 UI 보조 설명 텍스트로 변환합니다.
	FText GetFireFeedbackHintDisplayText(ECFVehicleFireFeedbackState InFeedbackState) const;

	// [v1.3.0] 기본 Reticle 상태와 FireFeedback 표시 데이터를 합쳐 최종 Reticle 상태를 반환합니다.
	ECFVehicleReticleState ResolveReticleStateFromFireFeedback(const FCFVehicleFireFeedbackViewData& InViewData, ECFVehicleReticleState BaseReticleState) const;
};
