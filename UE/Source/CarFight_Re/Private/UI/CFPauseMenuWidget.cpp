// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-06
// Description: CarFight 싱글플레이 Pause Menu C++ Widget 구현
// Scope: 반투명 배경, Pause 제목, Continue 버튼과 기본 Focus를 C++ WidgetTree로 구성합니다.
// Changelog:
// - v1.1.0: Continue 버튼의 최소 클릭 영역, 고대비 스타일, 입력 안내와 Player 직접 Focus를 추가.
// - v1.0.0: UI-P0-02 에셋 없는 Pause Menu와 Continue 이벤트를 최초 구현.
// Migration:
// - Blueprint 파생 Widget을 사용할 때도 OnContinueRequested 이벤트를 Pause 해제 진입점으로 유지한다.

#include "UI/CFPauseMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"

// [v1.0.0] Widget 초기화 시 최소 Pause Menu 트리와 버튼 이벤트를 준비합니다.
void UCFPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsurePauseMenuTree();
}

// [v1.0.0] Widget 파괴 시 Continue 버튼 이벤트 바인딩을 해제합니다.
void UCFPauseMenuWidget::NativeDestruct()
{
	if (ContinueButton)
	{
		ContinueButton->OnClicked.RemoveDynamic(this, &UCFPauseMenuWidget::HandleContinueButtonClicked);
	}

	Super::NativeDestruct();
}

// [v1.0.0] C++ Pause Menu 트리를 중복 없이 보장하고 완성 여부를 반환합니다.
bool UCFPauseMenuWidget::EnsurePauseMenuTree()
{
		if (RootCanvas && MenuBox && PauseTitleText && ContinueButtonSizeBox && ContinueButton && ContinueButtonText && ContinueHintText)
	{
		return true;
	}

	if (!WidgetTree)
	{
		return false;
	}

		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Canvas_CFPauseMenu"));
	if (!RootCanvas)
	{
		return false;
	}
	WidgetTree->RootWidget = RootCanvas;

	// [v1.0.0] 게임 화면 위를 덮는 반투명 배경입니다.
	UBorder* PauseBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Border_PauseBackground"));
	if (!PauseBackground)
	{
		return false;
	}
	PauseBackground->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));
	PauseBackground->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(PauseBackground);
	if (!BackgroundSlot)
	{
		return false;
	}
	BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	BackgroundSlot->SetOffsets(FMargin(0.0f));
	BackgroundSlot->SetAlignment(FVector2D::ZeroVector);
	BackgroundSlot->SetZOrder(0);

	MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VerticalBox_PauseMenu"));
	if (!MenuBox)
	{
		return false;
	}

	UCanvasPanelSlot* MenuBoxSlot = RootCanvas->AddChildToCanvas(MenuBox);
	if (!MenuBoxSlot)
	{
		return false;
	}
	MenuBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	MenuBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	MenuBoxSlot->SetPosition(FVector2D::ZeroVector);
	MenuBoxSlot->SetSize(FVector2D(460.0f, 220.0f));
	MenuBoxSlot->SetZOrder(10);

	PauseTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_PauseTitle"));
	if (!PauseTitleText)
	{
		return false;
	}
		PauseTitleText->SetText(FText::FromString(TEXT("일시정지")));
	PauseTitleText->SetJustification(ETextJustify::Center);
	PauseTitleText->SetAutoWrapText(true);
	PauseTitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	PauseTitleText->SetVisibility(ESlateVisibility::HitTestInvisible);

	// [v1.1.0] Pause 제목을 메뉴 제목으로 명확하게 보이게 할 글꼴 설정입니다.
	FSlateFontInfo PauseTitleFont = PauseTitleText->GetFont();
	PauseTitleFont.Size = 34;
	PauseTitleText->SetFont(PauseTitleFont);

	UVerticalBoxSlot* PauseTitleSlot = MenuBox->AddChildToVerticalBox(PauseTitleText);
	if (!PauseTitleSlot)
	{
		return false;
	}
	PauseTitleSlot->SetHorizontalAlignment(HAlign_Fill);
	PauseTitleSlot->SetPadding(FMargin(12.0f, 12.0f, 12.0f, 32.0f));

		ContinueButtonSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SizeBox_ContinueButton"));
	if (!ContinueButtonSizeBox)
	{
		return false;
	}
	ContinueButtonSizeBox->SetWidthOverride(360.0f);
	ContinueButtonSizeBox->SetHeightOverride(72.0f);

	ContinueButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_Continue"));
	if (!ContinueButton)
	{
		return false;
	}
			ContinueButton->SetIsEnabled(true);
	ContinueButton->SetVisibility(ESlateVisibility::Visible);
	ContinueButton->SetBackgroundColor(FLinearColor(0.05f, 0.32f, 0.62f, 1.0f));
	ContinueButton->SetColorAndOpacity(FLinearColor::White);
	ContinueButton->OnClicked.RemoveDynamic(this, &UCFPauseMenuWidget::HandleContinueButtonClicked);
	ContinueButton->OnClicked.AddDynamic(this, &UCFPauseMenuWidget::HandleContinueButtonClicked);

	ContinueButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Continue"));
	if (!ContinueButtonText)
	{
		return false;
	}
	ContinueButtonText->SetText(FText::FromString(TEXT("계속하기")));
	ContinueButtonText->SetJustification(ETextJustify::Center);
	ContinueButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	ContinueButtonText->SetVisibility(ESlateVisibility::HitTestInvisible);

	// [v1.1.0] 버튼의 주 동작 문구를 명확하게 보이게 할 글꼴 설정입니다.
	FSlateFontInfo ContinueButtonFont = ContinueButtonText->GetFont();
	ContinueButtonFont.Size = 26;
	ContinueButtonText->SetFont(ContinueButtonFont);
	ContinueButton->AddChild(ContinueButtonText);
	ContinueButtonSizeBox->AddChild(ContinueButton);

	UVerticalBoxSlot* ContinueButtonSlot = MenuBox->AddChildToVerticalBox(ContinueButtonSizeBox);
	if (!ContinueButtonSlot)
	{
		return false;
	}
	ContinueButtonSlot->SetHorizontalAlignment(HAlign_Center);
	ContinueButtonSlot->SetPadding(FMargin(48.0f, 8.0f, 48.0f, 8.0f));

	ContinueHintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_ContinueHint"));
	if (!ContinueHintText)
	{
		return false;
	}
	ContinueHintText->SetText(FText::FromString(TEXT("마우스 클릭 또는 Enter")));
	ContinueHintText->SetJustification(ETextJustify::Center);
	ContinueHintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.86f, 0.92f, 1.0f)));
	ContinueHintText->SetVisibility(ESlateVisibility::HitTestInvisible);

	// [v1.1.0] 보조 입력 안내를 본문보다 작게 보이게 할 글꼴 설정입니다.
	FSlateFontInfo ContinueHintFont = ContinueHintText->GetFont();
	ContinueHintFont.Size = 16;
	ContinueHintText->SetFont(ContinueHintFont);

	UVerticalBoxSlot* ContinueHintSlot = MenuBox->AddChildToVerticalBox(ContinueHintText);
	if (!ContinueHintSlot)
	{
		return false;
	}
	ContinueHintSlot->SetHorizontalAlignment(HAlign_Fill);
	ContinueHintSlot->SetPadding(FMargin(12.0f, 8.0f, 12.0f, 4.0f));

	return true;
}

// [v1.0.0] 키보드·게임패드 기본 Focus를 Continue 버튼으로 이동합니다.
void UCFPauseMenuWidget::FocusDefaultButton()
{
		if (EnsurePauseMenuTree() && ContinueButton)
	{
		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			ContinueButton->SetUserFocus(OwningPlayerController);
		}
		ContinueButton->SetKeyboardFocus();
	}
}

// [v1.0.0] Continue 버튼 클릭을 Pause 해제 요청 이벤트로 변환합니다.
void UCFPauseMenuWidget::HandleContinueButtonClicked()
{
	OnContinueRequested.Broadcast();
}
