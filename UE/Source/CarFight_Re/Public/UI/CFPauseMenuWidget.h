// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CarFight 싱글플레이 Pause Menu C++ Widget
// Scope: Unreal Asset 없이 Menu 레이어에 표시할 최소 Pause 화면과 Continue 요청을 제공합니다.
// Changelog:
// - v1.0.0: UI-P0-02 Pause 제목, Continue 버튼, 기본 Focus와 Continue 이벤트를 최초 추가.
// Migration:
// - 향후 WBP_CFPauseMenu를 추가해도 Continue 요청과 Menu 레이어 수명 계약은 유지한다.
// - 이 Widget은 게임 Pause 판정이나 입력 중립화를 직접 수행하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CFPauseMenuWidget.generated.h"

class UButton;
class UCanvasPanel;
class UTextBlock;
class UVerticalBox;

/**
 * Pause Menu의 Continue 버튼이 선택됐음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCFPauseContinueSignature);

/**
 * UI-P0-02에서 Unreal Asset 없이 사용할 수 있는 최소 Pause Menu입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] Widget 초기화 시 최소 Pause Menu 트리와 버튼 이벤트를 준비합니다.
	virtual void NativeOnInitialized() override;

	// [v1.0.0] Widget 파괴 시 Continue 버튼 이벤트 바인딩을 해제합니다.
	virtual void NativeDestruct() override;

	// [v1.0.0] C++ Pause Menu 트리를 중복 없이 보장하고 완성 여부를 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Pause", meta=(DisplayName="Pause Menu 트리 보장 (Ensure Pause Menu Tree)", ToolTip="Pause 제목과 Continue 버튼을 포함하는 C++ WidgetTree를 중복 없이 생성하고 준비 여부를 반환합니다."))
	bool EnsurePauseMenuTree();

	// [v1.0.0] 키보드·게임패드 기본 Focus를 Continue 버튼으로 이동합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Pause", meta=(DisplayName="Pause 기본 Focus 적용 (Focus Pause Default Button)", ToolTip="Pause Menu가 열린 뒤 Continue 버튼에 키보드와 게임패드 Focus를 적용합니다."))
	void FocusDefaultButton();

	// [v1.0.0] Continue 버튼 인스턴스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Pause", meta=(DisplayName="Continue 버튼 반환 (Get Continue Button)", ToolTip="현재 C++ Pause Menu의 Continue 버튼을 반환합니다."))
	UButton* GetContinueButton() const { return ContinueButton; }

	// [v1.0.0] Continue 버튼 요청 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|Pause")
	FCFPauseContinueSignature OnContinueRequested;

private:
	// [v1.0.0] Continue 버튼 클릭을 Pause 해제 요청 이벤트로 변환합니다.
	UFUNCTION()
	void HandleContinueButtonClicked();

	// [v1.0.0] 화면 전체를 채우는 Pause Menu 최상위 Canvas입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;

	// [v1.0.0] Pause 제목과 Continue 버튼을 세로로 배치하는 컨테이너입니다.
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> MenuBox = nullptr;

	// [v1.0.0] Pause 상태를 표시하는 제목 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PauseTitleText = nullptr;

	// [v1.0.0] Pause를 해제하고 게임으로 돌아가는 기본 버튼입니다.
	UPROPERTY(Transient)
	TObjectPtr<UButton> ContinueButton = nullptr;

	// [v1.0.0] Continue 버튼 안에 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContinueButtonText = nullptr;
};
