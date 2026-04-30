// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-04-27
// Description: VehicleDebug Panel Navigation 항목용 C++ 부모 위젯 클래스입니다.
// Scope: Navigation 항목 표시, 선택 상태 스타일, 선택 요청 전달을 담당합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CFDebugPanelViewData.h"
#include "CFVehicleDebugNavItemWidget.generated.h"

class UButton;
class UCFVehicleDebugPanelWidget;
class UTextBlock;

/**
 * VehicleDebug Panel Navigation 항목용 C++ 부모 위젯입니다.
 * - Navigation 항목 텍스트/배지를 표시합니다.
 * - 클릭 시 소유 Panel에 Section 선택을 요청합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFVehicleDebugNavItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] 이 NavItem이 선택 요청을 전달할 Panel 위젯 참조를 설정합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|NavItem", meta=(DisplayName="Panel 위젯 참조 설정 (SetOwningPanelWidget)", ToolTip="이 Navigation Item이 클릭되었을 때 선택 요청을 전달할 Panel 위젯 참조를 설정합니다."))
	void SetOwningPanelWidget(UCFVehicleDebugPanelWidget* InOwningPanelWidget);

	// [v1.0.0] 현재 Navigation Item ViewData를 설정하고 표시를 갱신합니다.
	void SetNavItemViewData(const FCFVehicleDebugNavItemViewData& InNavItemViewData);

	// [v1.0.0] 현재 캐시된 Navigation Item ViewData 기준으로 표시를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|NavItem", meta=(DisplayName="Navigation Item 갱신 (RefreshFromNavItemViewData)", ToolTip="현재 캐시된 Navigation Item ViewData 기준으로 표시 텍스트, 배지, 선택 상태를 갱신합니다."))
	void RefreshFromNavItemViewData();

	// [v1.0.0] 이 NavItem이 가리키는 Section 선택을 Panel에 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|NavItem", meta=(DisplayName="이 Navigation Item 선택 요청 (RequestSelectThisNavItem)", ToolTip="이 Navigation Item이 가진 SectionId를 사용해 Panel에 선택 변경을 요청합니다. WBP 이벤트 그래프 fallback에서도 이 함수를 호출합니다."))
	void RequestSelectThisNavItem();

protected:
	// [v1.0.0] 위젯 생성 시 Button 클릭 이벤트를 C++ 경로에 연결합니다.
	virtual void NativeConstruct() override;

	// [v1.0.0] NavItem 클릭 입력을 받을 루트 버튼입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button_Root = nullptr;

	// [v1.0.0] Navigation 항목 이름을 표시할 텍스트입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Display = nullptr;

	// [v1.0.0] Navigation 항목 우측 배지를 표시할 텍스트입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Badge = nullptr;

	// [v1.1.0] 선택되지 않은 Navigation 버튼 배경색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|NavItem|Style", meta=(DisplayName="기본 배경색 (DefaultBackgroundColor)", ToolTip="선택되지 않은 Navigation Item 버튼 배경색입니다."))
	FLinearColor DefaultBackgroundColor = FLinearColor(0.08f, 0.10f, 0.12f, 0.35f);

	// [v1.1.0] 선택된 Navigation 버튼 배경색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|NavItem|Style", meta=(DisplayName="선택 배경색 (SelectedBackgroundColor)", ToolTip="선택된 Navigation Item 버튼 배경색입니다."))
	FLinearColor SelectedBackgroundColor = FLinearColor(0.16f, 0.32f, 0.52f, 0.85f);

	// [v1.1.0] 선택되지 않은 Navigation 텍스트 색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|NavItem|Style", meta=(DisplayName="기본 텍스트 색 (DefaultTextColor)", ToolTip="선택되지 않은 Navigation Item 텍스트 색입니다."))
	FLinearColor DefaultTextColor = FLinearColor(0.90f, 0.92f, 0.95f, 1.0f);

	// [v1.1.0] 선택된 Navigation 텍스트 색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|NavItem|Style", meta=(DisplayName="선택 텍스트 색 (SelectedTextColor)", ToolTip="선택된 Navigation Item 텍스트 색입니다."))
	FLinearColor SelectedTextColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// [v1.1.0] Navigation 배지 텍스트 색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|NavItem|Style", meta=(DisplayName="배지 텍스트 색 (BadgeTextColor)", ToolTip="Navigation Item 배지 텍스트 색입니다."))
	FLinearColor BadgeTextColor = FLinearColor(1.0f, 0.82f, 0.18f, 1.0f);

private:
	// [v1.0.0] 선택 요청을 전달할 Panel 위젯 약참조입니다.
	UPROPERTY(Transient)
	TWeakObjectPtr<UCFVehicleDebugPanelWidget> OwningPanelWidget;

	// [v1.0.0] 현재 Navigation Item 표시와 선택 요청에 사용할 캐시 ViewData입니다.
	FCFVehicleDebugNavItemViewData CachedNavItemViewData;

	// [v1.0.0] 루트 버튼 클릭을 처리해 선택 요청 함수로 전달합니다.
	UFUNCTION()
	void HandleRootButtonClicked();
};
