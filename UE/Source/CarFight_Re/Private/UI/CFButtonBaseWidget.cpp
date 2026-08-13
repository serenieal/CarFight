// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: UCFButtonBaseWidget Interaction·Focus Bridge 구현
// Scope: D1-10A UButton Delegate와 Focus Path를 독립 Visual State로 변환합니다.
// Changelog:
// - v1.0.0: BindWidget Delegate, Activation, Focus, Enabled와 Visual State 갱신 구현.
// Migration:
// - Pause, World 상태, Gameplay Actor 또는 DataAsset Load를 참조하지 않습니다.

#include "UI/CFButtonBaseWidget.h"

#include "Components/Button.h"

// [v1.0.0] Blueprint Tree에 Bind된 실제 Interaction UButton을 반환합니다.
UButton* UCFButtonBaseWidget::GetInteractionButton() const
{
	return Button_Interaction;
}

// [v1.0.0] Keyboard/Gamepad Navigation이 사용할 실제 Interaction UButton으로 Focus를 이동합니다.
bool UCFButtonBaseWidget::FocusInteractionButton()
{
	if (!Button_Interaction || !Button_Interaction->GetIsEnabled())
	{
		return false;
	}

	// [v1.0.0] LocalPlayer의 User Focus를 우선 적용할 현재 Owning Player입니다.
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer)
	{
		Button_Interaction->SetUserFocus(OwningPlayer);
	}
	else
	{
		Button_Interaction->SetKeyboardFocus();
	}

	return true;
}

// [v1.0.0] 공통 Button의 입력 가능 상태를 변경하고 Disabled Visual State를 즉시 갱신합니다.
void UCFButtonBaseWidget::SetButtonEnabled(bool bEnabled)
{
	SetIsEnabled(bEnabled);
	if (Button_Interaction)
	{
		Button_Interaction->SetIsEnabled(bEnabled);
	}

	if (!bEnabled)
	{
		bButtonPressed = false;
	}
	RefreshButtonVisualState();
}

// [v1.0.0] 현재 대표 Button Visual State를 반환합니다.
ECFUIButtonVisualState UCFButtonBaseWidget::GetButtonVisualState() const
{
	// [v1.0.0] Bind된 버튼이 있으면 실제 UButton Enabled 상태까지 반영한 활성 여부입니다.
	const bool bEnabled = GetIsEnabled() && (!Button_Interaction || Button_Interaction->GetIsEnabled());
	if (!bEnabled)
	{
		return ECFUIButtonVisualState::Disabled;
	}
	if (bButtonPressed)
	{
		return ECFUIButtonVisualState::Pressed;
	}
	if (bButtonFocused)
	{
		return ECFUIButtonVisualState::Focused;
	}
	if (bButtonHovered)
	{
		return ECFUIButtonVisualState::Hover;
	}
	return ECFUIButtonVisualState::Normal;
}

// [v1.0.0] 현재 마우스 Hover 여부를 반환합니다.
bool UCFButtonBaseWidget::IsButtonHovered() const
{
	return bButtonHovered;
}

// [v1.0.0] 현재 Keyboard/Gamepad Focus Path 포함 여부를 반환합니다.
bool UCFButtonBaseWidget::IsButtonFocused() const
{
	return bButtonFocused;
}

// [v1.0.0] 현재 Pressed 상태 여부를 반환합니다.
bool UCFButtonBaseWidget::IsButtonPressed() const
{
	return bButtonPressed;
}

// [v1.0.0] BindWidget 연결과 UButton Delegate를 초기화하고 현재 Visual State를 동기화합니다.
void UCFButtonBaseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Button_Interaction)
	{
		Button_Interaction->OnClicked.RemoveDynamic(this, &UCFButtonBaseWidget::HandleInteractionClicked);
		Button_Interaction->OnHovered.RemoveDynamic(this, &UCFButtonBaseWidget::HandleInteractionHovered);
		Button_Interaction->OnUnhovered.RemoveDynamic(this, &UCFButtonBaseWidget::HandleInteractionUnhovered);
		Button_Interaction->OnPressed.RemoveDynamic(this, &UCFButtonBaseWidget::HandleInteractionPressed);
		Button_Interaction->OnReleased.RemoveDynamic(this, &UCFButtonBaseWidget::HandleInteractionReleased);

		Button_Interaction->OnClicked.AddDynamic(this, &UCFButtonBaseWidget::HandleInteractionClicked);
		Button_Interaction->OnHovered.AddDynamic(this, &UCFButtonBaseWidget::HandleInteractionHovered);
		Button_Interaction->OnUnhovered.AddDynamic(this, &UCFButtonBaseWidget::HandleInteractionUnhovered);
		Button_Interaction->OnPressed.AddDynamic(this, &UCFButtonBaseWidget::HandleInteractionPressed);
		Button_Interaction->OnReleased.AddDynamic(this, &UCFButtonBaseWidget::HandleInteractionReleased);
	}

	RefreshButtonVisualState();
}

// [v1.0.0] Keyboard/Gamepad Focus Path에 진입하면 Hover와 별개인 Focus 상태를 갱신합니다.
void UCFButtonBaseWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	bButtonFocused = true;
	RefreshButtonVisualState();
}

// [v1.0.0] Keyboard/Gamepad Focus Path에서 이탈하면 Focus 상태만 해제합니다.
void UCFButtonBaseWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	bButtonFocused = false;
	bButtonPressed = false;
	RefreshButtonVisualState();
}

// [v1.0.0] UButton Click을 외부 OnActivated Delegate로 변환합니다.
void UCFButtonBaseWidget::HandleInteractionClicked()
{
	OnActivated.Broadcast();
}

// [v1.0.0] UButton Hover 진입을 독립 Hover 상태로 기록합니다.
void UCFButtonBaseWidget::HandleInteractionHovered()
{
	bButtonHovered = true;
	RefreshButtonVisualState();
}

// [v1.0.0] UButton Hover 이탈을 독립 Hover 상태로 기록합니다.
void UCFButtonBaseWidget::HandleInteractionUnhovered()
{
	bButtonHovered = false;
	RefreshButtonVisualState();
}

// [v1.0.0] UButton Press 입력을 독립 Pressed 상태로 기록합니다.
void UCFButtonBaseWidget::HandleInteractionPressed()
{
	bButtonPressed = true;
	RefreshButtonVisualState();
}

// [v1.0.0] UButton Release 입력을 독립 Pressed 상태에서 해제합니다.
void UCFButtonBaseWidget::HandleInteractionReleased()
{
	bButtonPressed = false;
	RefreshButtonVisualState();
}

// [v1.0.0] 현재 독립 입력 상태에서 대표 Visual State를 계산하고 Blueprint Event를 호출합니다.
void UCFButtonBaseWidget::RefreshButtonVisualState()
{
	// [v1.0.0] 현재 Widget과 Bind된 UButton을 함께 고려한 실효 활성 여부입니다.
	const bool bEnabled = GetIsEnabled() && (!Button_Interaction || Button_Interaction->GetIsEnabled());
	OnButtonVisualStateChanged(GetButtonVisualState(), bButtonHovered, bButtonFocused, bButtonPressed, bEnabled);
}
