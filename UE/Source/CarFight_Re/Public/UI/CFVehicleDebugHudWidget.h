// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-04-23
// Description: VehicleDebug HUD용 C++ 부모 위젯 클래스입니다.
// Scope: VehicleDebug Overview를 읽어 HUD 텍스트를 갱신하고 HUD 가시성을 제어합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CFVehiclePawn.h"
#include "CFVehicleDebugHudWidget.generated.h"

class UTextBlock;

/**
 * VehicleDebug HUD용 C++ 부모 위젯 클래스입니다.
 * - 데이터 읽기와 갱신은 C++가 담당합니다.
 * - 레이아웃과 스타일은 WBP 자식이 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFVehicleDebugHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] HUD가 읽을 차량 Pawn 참조를 설정하고 즉시 HUD를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Hud", meta=(DisplayName="차량 Pawn 참조 설정 (SetVehiclePawnRef)", ToolTip="HUD가 디버그 정보를 읽어올 차량 Pawn 참조를 설정하고 즉시 갱신을 수행합니다."))
	void SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef);

	// [v1.0.0] 현재 Pawn에서 최신 VehicleDebug Overview를 읽어 HUD를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Hud", meta=(DisplayName="Pawn에서 HUD 갱신 (RefreshFromPawn)", ToolTip="현재 차량 Pawn에서 최신 VehicleDebug Overview를 읽어 HUD 표시값을 갱신합니다."))
	void RefreshFromPawn();

	// [v1.0.0] 전달받은 Overview 스냅샷 값을 HUD 텍스트 위젯에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Hud", meta=(DisplayName="Overview 적용 (ApplyVehicleDebugOverview)", ToolTip="전달받은 VehicleDebug Overview 스냅샷 값을 HUD 텍스트 위젯에 반영합니다."))
	void ApplyVehicleDebugOverview(const FCFVehicleDebugOverview& InOverview);

	// [v1.0.0] 현재 Pawn의 HUD 표시 조건에 따라 위젯 가시성을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Hud", meta=(DisplayName="HUD 가시성 갱신 (UpdateHudVisibility)", ToolTip="현재 차량 Pawn의 HUD 표시 조건에 따라 HUD 위젯 가시성을 갱신합니다."))
	void UpdateHudVisibility();

protected:
	// [v1.0.0] 위젯 생성 직후 현재 참조 기준으로 첫 HUD 갱신을 수행합니다.
	virtual void NativeConstruct() override;

	// [v1.0.0] 옵션이 켜져 있으면 매 프레임 Pawn 기준 HUD 갱신을 수행합니다.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// [v1.0.0] HUD 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Title = nullptr;

	// [v1.0.0] 런타임 준비 상태 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeReady = nullptr;

	// [v1.0.0] 현재 Drive 상태 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DriveState = nullptr;

	// [v1.0.0] 현재 속도 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Speed = nullptr;

	// [v1.0.0] 현재 전후 방향 속도 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ForwardSpeed = nullptr;

	// [v1.0.0] 현재 입력 장치 모드 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DeviceMode = nullptr;

	// [v1.0.0] 현재 입력 주도권 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_InputOwner = nullptr;

	// [v1.0.0] 마지막 전이 요약 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_LastTransition = nullptr;

	// [v1.0.0] HUD가 디버그 정보를 읽어올 차량 Pawn 참조입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Hud", meta=(DisplayName="차량 Pawn 참조 (VehiclePawnRef)", ToolTip="현재 HUD가 디버그 정보를 읽어올 차량 Pawn 참조입니다."))
	TObjectPtr<ACFVehiclePawn> VehiclePawnRef = nullptr;

	// [v1.0.0] 현재 HUD에 적용 중인 최신 Overview 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Hud", meta=(DisplayName="Overview 캐시 (CachedOverview)", ToolTip="현재 HUD에 표시 중인 최신 VehicleDebug Overview 캐시입니다."))
	FCFVehicleDebugOverview CachedOverview;

	// [v1.0.0] True이면 NativeTick에서 HUD를 자동 갱신합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Hud", meta=(DisplayName="매 프레임 자동 갱신 (bAutoRefreshEveryTick)", ToolTip="True이면 NativeTick에서 현재 Pawn 기준 HUD를 자동 갱신합니다."))
	bool bAutoRefreshEveryTick = true;

private:
	// [v1.0.0] enum 값을 HUD 표시용 문자열로 변환합니다.
	FString ConvertEnumValueToDisplayString(const TCHAR* EnumValueString) const;
};
