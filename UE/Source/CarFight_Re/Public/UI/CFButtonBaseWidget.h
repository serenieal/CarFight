// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight 공통 Button Base의 Interaction·Focus Bridge
// Scope: D1-10A Click/Hover/Press/Release와 Keyboard/Gamepad Focus를 Gameplay 의미 없이 Blueprint Visual State로 변환합니다.
// Changelog:
// - v1.0.0: Button_Interaction BindWidget, OnActivated, Focus 요청, Enabled 제어와 독립 Hover/Focus/Pressed 상태 Bridge를 최초 추가.
// Migration:
// - Pause 판정과 World Pause는 이 Widget이 소유하지 않습니다.
// - D1-10B WBP_CFButtonBase는 반드시 Button_Interaction 이름의 UButton을 포함해야 합니다.

#pragma once

#include "CoreMinimal.h"
#include "UI/CFStyledWidgetBase.h"
#include "CFButtonBaseWidget.generated.h"

class UButton;

/**
 * Button Visual Layer가 사용할 대표 상태입니다. Hover와 Focus 동시 여부는 별도 Boolean으로 Event에 함께 전달합니다.
 */
UENUM(BlueprintType)
enum class ECFUIButtonVisualState : uint8
{
	Normal UMETA(DisplayName="일반 (Normal)"),
	Hover UMETA(DisplayName="호버 (Hover)"),
	Focused UMETA(DisplayName="포커스 (Focused)"),
	Pressed UMETA(DisplayName="눌림 (Pressed)"),
	Disabled UMETA(DisplayName="비활성 (Disabled)")
};

// [v1.0.0] 공통 Button이 실제 Click Activation을 외부 소유자에게 전달하는 Delegate입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCFButtonActivated);

/**
 * 공통 Button Blueprint가 UButton 입력과 Focus를 안정적으로 소비하도록 중계하는 C++ Base입니다.
 */
UCLASS(Blueprintable)
class CARFIGHT_RE_API UCFButtonBaseWidget : public UCFStyledWidgetBase
{
	GENERATED_BODY()

public:
	// [v1.0.0] Blueprint Tree에 Bind된 실제 Interaction UButton을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Button", meta=(DisplayName="상호작용 버튼 가져오기 (Get Interaction Button)", ToolTip="WBP_CFButtonBase의 Button_Interaction UButton을 반환합니다. Bind 실패 시 Null입니다."))
	UButton* GetInteractionButton() const;

	// [v1.0.0] Keyboard/Gamepad Navigation이 사용할 실제 Interaction UButton으로 Focus를 이동합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Button", meta=(DisplayName="상호작용 버튼 포커스 (Focus Interaction Button)", ToolTip="실제 Button_Interaction에 사용자 Focus를 요청합니다. Bind된 버튼이 없거나 비활성이면 False를 반환합니다."))
	bool FocusInteractionButton();

	// [v1.0.0] 공통 Button의 입력 가능 상태를 변경하고 Disabled Visual State를 즉시 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Button", meta=(DisplayName="버튼 활성 상태 설정 (Set Button Enabled)", ToolTip="Button_Interaction과 Widget의 입력 가능 상태를 함께 변경하고 Visual State를 갱신합니다."))
	void SetButtonEnabled(bool bEnabled);

	// [v1.0.0] 현재 대표 Button Visual State를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Button", meta=(DisplayName="버튼 시각 상태 가져오기 (Get Button Visual State)", ToolTip="Disabled > Pressed > Focused > Hover > Normal 우선순위의 대표 Visual State를 반환합니다. Hover와 Focus 동시 여부는 별도 Getter로 확인할 수 있습니다."))
	ECFUIButtonVisualState GetButtonVisualState() const;

	// [v1.0.0] 현재 마우스 Hover 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Button", meta=(DisplayName="버튼 호버 여부 (Is Button Hovered)", ToolTip="Button_Interaction의 현재 Hover 상태입니다. Focus 상태와 독립적으로 유지됩니다."))
	bool IsButtonHovered() const;

	// [v1.0.0] 현재 Keyboard/Gamepad Focus Path 포함 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Button", meta=(DisplayName="버튼 포커스 여부 (Is Button Focused)", ToolTip="Button_Interaction이 현재 Keyboard/Gamepad Focus Path에 포함되어 있는지 반환합니다. Hover와 독립 상태입니다."))
	bool IsButtonFocused() const;

	// [v1.0.0] 현재 Pressed 상태 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Button", meta=(DisplayName="버튼 눌림 여부 (Is Button Pressed)", ToolTip="Button_Interaction이 현재 눌린 상태인지 반환합니다."))
	bool IsButtonPressed() const;

	// [v1.0.0] 실제 UButton Click이 발생했을 때 외부 메뉴·화면 소유자에게 전달하는 Activation Event입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|Button", meta=(DisplayName="활성화됨 (On Activated)", ToolTip="실제 Button_Interaction Click이 발생했을 때 Broadcast됩니다. Pause 해제 같은 Gameplay 판정은 외부 소유자가 처리합니다."))
	FOnCFButtonActivated OnActivated;

protected:
	// [v1.0.0] BindWidget 연결과 UButton Delegate를 초기화하고 현재 Visual State를 동기화합니다.
	virtual void NativeOnInitialized() override;

	// [v1.0.0] Keyboard/Gamepad Focus Path에 진입하면 Hover와 별개인 Focus 상태를 갱신합니다.
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;

	// [v1.0.0] Keyboard/Gamepad Focus Path에서 이탈하면 Focus 상태만 해제합니다.
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	// [v1.0.0] 대표 상태와 Hover·Focus·Pressed·Enabled 독립 값을 Blueprint Visual Layer에 전달합니다.
	UFUNCTION(BlueprintImplementableEvent, Category="CarFight|UI|Button", meta=(DisplayName="버튼 시각 상태 변경됨 (On Button Visual State Changed)", ToolTip="Button 입력/Focus 상태가 바뀔 때 호출됩니다. Hover와 Focus는 별도 Boolean이므로 하나로 합치지 마세요."))
	void OnButtonVisualStateChanged(ECFUIButtonVisualState VisualState, bool bHovered, bool bFocused, bool bPressed, bool bEnabled);

	// [v1.0.0] D1-10B WBP_CFButtonBase가 반드시 제공해야 하는 실제 Interaction Button입니다.
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="CarFight|UI|Button")
	TObjectPtr<UButton> Button_Interaction = nullptr;

private:
	// [v1.0.0] UButton Click을 외부 OnActivated Delegate로 변환합니다.
	UFUNCTION()
	void HandleInteractionClicked();

	// [v1.0.0] UButton Hover 진입을 독립 Hover 상태로 기록합니다.
	UFUNCTION()
	void HandleInteractionHovered();

	// [v1.0.0] UButton Hover 이탈을 독립 Hover 상태로 기록합니다.
	UFUNCTION()
	void HandleInteractionUnhovered();

	// [v1.0.0] UButton Press 입력을 독립 Pressed 상태로 기록합니다.
	UFUNCTION()
	void HandleInteractionPressed();

	// [v1.0.0] UButton Release 입력을 독립 Pressed 상태에서 해제합니다.
	UFUNCTION()
	void HandleInteractionReleased();

	// [v1.0.0] 현재 독립 입력 상태에서 대표 Visual State를 계산하고 Blueprint Event를 호출합니다.
	void RefreshButtonVisualState();

	// [v1.0.0] 현재 UButton에 마우스가 Hover 중인지 보존하는 독립 상태입니다.
	UPROPERTY(Transient)
	bool bButtonHovered = false;

	// [v1.0.0] 현재 UButton이 Keyboard/Gamepad Focus Path에 있는지 보존하는 독립 상태입니다.
	UPROPERTY(Transient)
	bool bButtonFocused = false;

	// [v1.0.0] 현재 UButton이 Pressed 상태인지 보존하는 독립 상태입니다.
	UPROPERTY(Transient)
	bool bButtonPressed = false;
};
