// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-05-21
// Description: Aim Reticle UI용 C++ 부모 위젯 클래스입니다.
// Scope: VehicleAimComp의 Reticle 상태를 읽어 선택적 TextBlock과 위젯 가시성을 갱신합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CFVehicleAimTypes.h"
#include "CFAimReticleWidget.generated.h"

class ACFVehiclePawn;
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

	// [v1.0.0] Reticle이 조준 상태를 읽어올 차량 Pawn 참조입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle", meta=(DisplayName="차량 Pawn 참조 (VehiclePawnRef)", ToolTip="현재 Reticle UI가 조준 상태를 읽어올 차량 Pawn 참조입니다."))
	TObjectPtr<ACFVehiclePawn> VehiclePawnRef = nullptr;

	// [v1.0.0] 현재 Reticle에 적용 중인 상태 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle", meta=(DisplayName="Reticle 상태 캐시 (CachedReticleState)", ToolTip="현재 Reticle UI에 적용 중인 조준점 상태 캐시입니다."))
	ECFVehicleReticleState CachedReticleState = ECFVehicleReticleState::Hidden;

	// [v1.0.0] 현재 Reticle에 표시할 발사 가능 여부 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle", meta=(DisplayName="발사 가능 캐시 (bCachedCanFire)", ToolTip="현재 Reticle UI에 표시할 발사 가능 여부 캐시입니다. 서버 최종 판정이 아닙니다."))
	bool bCachedCanFire = false;

	// [v1.0.0] True이면 NativeTick에서 Reticle을 자동 갱신합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Aim|Reticle", meta=(DisplayName="매 프레임 자동 갱신 (bAutoRefreshEveryTick)", ToolTip="True이면 NativeTick에서 현재 Pawn 기준 Reticle UI를 자동 갱신합니다."))
	bool bAutoRefreshEveryTick = true;

private:
	// [v1.0.0] 현재 캐시값을 선택적 TextBlock들에 반영합니다.
	void RefreshTextBlocks();

	// [v1.0.0] Reticle 상태별 보조 설명 텍스트를 반환합니다.
	FText GetReticleHintDisplayText() const;
};
