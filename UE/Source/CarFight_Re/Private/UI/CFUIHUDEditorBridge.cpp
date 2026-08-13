// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-10
// Description: CF-FQ-032 D1-11 WBP_CFInGameHUD 1920x1080 Visual Prototype Editor Bridge 구현
// Scope: D1-07 USER PASS 1080p Slot·Mock Visual을 실제 UMG Designer에서 재현하고 정적 Visual Fidelity 계약을 검증합니다.
// Changelog:
// - v1.3.0: 세 번째 사용자 Preview FAIL을 반영해 Vehicle Armor 의미별 위치를 D1-07 계약(좌 Front·상 Right·우 Rear·하 Left·좌상단 Top·우하단 Bottom)과 정확히 일치시키고, 속도 원호와 왼쪽 전방 차량 실루엣의 판독성을 강화하며 의미별 위치 Validator를 추가.
// - v1.2.0: 두 번째 사용자 Preview FAIL을 반영해 Vehicle 좌측 원호형 속도계·왼쪽 전방 차체 실루엣·승인 Armor 위치 의미를 복원하고 중첩 D1-10 Base Widget Designer Placeholder를 화면 밖 보관하도록 교정.
// - v1.1.0: 사용자 1920x1080 Preview FAIL을 반영해 Slot 내부 Stretch, Vehicle Armor Map, Radar Contact Field, Weapon Compact Rail, Target 정렬을 Canvas 기반 Visual Mock으로 재구축하고 Visual Fidelity Validator를 추가.
// - v1.0.3: CarFight CodeWorkGate에 맞춰 지역 변수 역할 주석과 검증 블록 들여쓰기를 보강.
// - v1.0.2: UE가 신규 Widget Blueprint에 자동 생성하는 미연결 K2Node_Event만 허용하고 연결된 Pin 또는 Event 외 Graph Node를 금지.
// - v1.0.1: UE 5.8 FAnchors 4-float 생성자와 float Font Size 검증 형식에 맞게 Compile 호환성을 보정.
// - v1.0.0: 정확한 7-Slot, D1-07 Mock 문자열, Typography Floor, Weapon Compact, Graph 0 검증을 최초 구현.
// Migration:
// - 기존 WBP_CFInGameHUD는 같은 Asset 안에서 WidgetTree를 교체해 D1-11 v1.3.0 Visual Prototype으로 재구축합니다.
// - Gameplay Actor/Component 조회, DataAsset Path Load, Runtime Event Binding을 생성하지 않습니다.
// - D1-09B/D1-10 Asset은 입력 Class/Data로만 읽고 변경하지 않습니다.

#include "UI/CFUIHUDEditorBridge.h"

#include "UI/CFHUDLayoutData.h"
#include "UI/CFStyledWidgetBase.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIStyleData.h"

#if WITH_EDITOR
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Styling/SlateColor.h"
#include "WidgetBlueprint.h"
#endif

namespace CFUIHUDEditorBridge
{
#if WITH_EDITOR
	// [v1.1.0] D1-11 검증 실패를 공통 로그 형식으로 남기고 false를 반환합니다.
	bool Fail(const FString& FailureReason)
	{
		UE_LOG(LogTemp, Error, TEXT("[CarFight][UIHUDEditorBridge] %s"), *FailureReason);
		return false;
	}

	// [v1.1.0] 두 Vector 값이 Pixel/Anchor 정밀도 범위에서 같은지 검사합니다.
	bool IsNearlyEqualVector(const FVector2D& Left, const FVector2D& Right, const double Tolerance = 0.01)
	{
		return FMath::IsNearlyEqual(Left.X, Right.X, Tolerance)
			&& FMath::IsNearlyEqual(Left.Y, Right.Y, Tolerance);
	}

	// [v1.1.0] WidgetTree에서 지정 Native Widget을 고유 이름으로 생성합니다.
	template <typename TWidget>
	TWidget* CreateWidget(UWidgetTree* WidgetTree, const TCHAR* WidgetName)
	{
		return WidgetTree
			? WidgetTree->ConstructWidget<TWidget>(TWidget::StaticClass(), FName(WidgetName))
			: nullptr;
	}

	// [v1.1.0] 기존 D1-10 Widget Blueprint Generated Class로 중첩 UserWidget Template을 생성합니다.
	UUserWidget* CreateNestedUserWidget(UWidgetTree* WidgetTree, UClass* WidgetClass, const TCHAR* WidgetName)
	{
		if (!WidgetTree || !WidgetClass || !WidgetClass->IsChildOf(UUserWidget::StaticClass()))
		{
			return nullptr;
		}
		return WidgetTree->ConstructWidget<UUserWidget>(WidgetClass, FName(WidgetName));
	}

	// [v1.1.0] 읽기 전용 Base Widget Blueprint에서 사용 가능한 Generated Class를 반환합니다.
	UClass* ResolveGeneratedWidgetClass(UObject* BlueprintObject, const TCHAR* Label, FString& OutFailureReason)
	{
		// [v1.1.0] Python에서 전달된 읽기 전용 D1-10 Widget Blueprint입니다.
		const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(BlueprintObject);
		if (!WidgetBlueprint || !WidgetBlueprint->GeneratedClass)
		{
			OutFailureReason = FString::Printf(TEXT("%s Widget Blueprint or GeneratedClass is invalid"), Label);
			return nullptr;
		}
		if (!WidgetBlueprint->GeneratedClass->IsChildOf(UCFStyledWidgetBase::StaticClass()))
		{
			OutFailureReason = FString::Printf(TEXT("%s GeneratedClass is not a CFStyledWidgetBase child: %s"), Label, *WidgetBlueprint->GeneratedClass->GetName());
			return nullptr;
		}
		return WidgetBlueprint->GeneratedClass;
	}

	// [v1.1.0] D1-09B DataAsset과 1080p Profile이 D1-11 고정 입력 계약을 만족하는지 검증합니다.
	bool ValidateInputData(
		const UCFHUDLayoutData* LayoutData,
		const UCFUIStyleData* StyleData,
		const UCFUIDensityData* StandardDensityData,
		const UCFUIDensityData* CompactDensityData,
		FString& OutFailureReason)
	{
		if (!LayoutData || !StyleData || !StandardDensityData || !CompactDensityData)
		{
			OutFailureReason = TEXT("Layout/Style/StandardDensity/CompactDensity input is null");
			return false;
		}

		// [v1.1.0] 각 D1-09B 입력 DataAsset Validator가 반환하는 상세 실패 사유입니다.
		FString ValidationFailure;
		if (!LayoutData->ValidateLayoutData(ValidationFailure))
		{
			OutFailureReason = FString::Printf(TEXT("Layout Data invalid: %s"), *ValidationFailure);
			return false;
		}
		if (!StyleData->ValidateStyleData(ValidationFailure))
		{
			OutFailureReason = FString::Printf(TEXT("Style Data invalid: %s"), *ValidationFailure);
			return false;
		}
		if (!StandardDensityData->ValidateDensityData(ValidationFailure))
		{
			OutFailureReason = FString::Printf(TEXT("Standard Density invalid: %s"), *ValidationFailure);
			return false;
		}
		if (!CompactDensityData->ValidateDensityData(ValidationFailure))
		{
			OutFailureReason = FString::Printf(TEXT("Compact Density invalid: %s"), *ValidationFailure);
			return false;
		}

		if (LayoutData->ProfileId != FName(TEXT("HUD_1080_16"))
			|| !IsNearlyEqualVector(LayoutData->ReferenceViewportSize, FVector2D(1920.0, 1080.0))
			|| !FMath::IsNearlyEqual(LayoutData->GeometryScale, 0.75f)
			|| !FMath::IsNearlyEqual(LayoutData->TypographyScale, 0.75f))
		{
			OutFailureReason = TEXT("D1-11 requires HUD_1080_16 / 1920x1080 / Geometry 0.75 / Typography 0.75");
			return false;
		}
		if (StandardDensityData->Preset != ECFUIDensityPreset::Standard || CompactDensityData->Preset != ECFUIDensityPreset::Compact)
		{
			OutFailureReason = TEXT("D1-11 requires Standard HUD Density and Compact Weapon Density");
			return false;
		}
		return true;
	}

	// [v1.1.0] D1-11 Target Blueprint가 기존 Widget 오브젝트 이름과 충돌하지 않도록 새 WidgetTree로 교체합니다.
	UWidgetTree* ReplaceWidgetTree(UWidgetBlueprint* WidgetBlueprint, FString& OutFailureReason)
	{
		if (!WidgetBlueprint)
		{
			OutFailureReason = TEXT("WidgetBlueprint is null while replacing WidgetTree");
			return nullptr;
		}

		WidgetBlueprint->Modify();
		// [v1.1.0] 기존 Tree와 독립된 Outer를 사용해 동일 Widget 이름을 안전하게 재사용할 새 WidgetTree입니다.
		UWidgetTree* NewWidgetTree = NewObject<UWidgetTree>(WidgetBlueprint, UWidgetTree::StaticClass(), NAME_None, RF_Transactional);
		if (!NewWidgetTree)
		{
			OutFailureReason = TEXT("New D1-11 WidgetTree creation failed");
			return nullptr;
		}
		WidgetBlueprint->WidgetTree = NewWidgetTree;
		NewWidgetTree->Modify();
		return NewWidgetTree;
	}

	// [v1.1.0] Root Canvas Child Slot에 DataAsset의 Anchor·Offset·Size·Alignment·ZOrder를 직접 적용합니다.
	bool ApplyRootCanvasLayout(UCanvasPanel* RootCanvas, UWidget* ChildWidget, const FCFHUDSlotLayout& SlotLayout)
	{
		if (!RootCanvas || !ChildWidget)
		{
			return false;
		}

		// [v1.1.0] Root Canvas에 Child를 추가하면서 생성된 실제 Canvas Panel Slot입니다.
		UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(ChildWidget);
		if (!CanvasSlot)
		{
			return false;
		}

		CanvasSlot->SetAnchors(FAnchors(
			SlotLayout.AnchorMinimum.X,
			SlotLayout.AnchorMinimum.Y,
			SlotLayout.AnchorMaximum.X,
			SlotLayout.AnchorMaximum.Y));
		CanvasSlot->SetAlignment(SlotLayout.Alignment);
		CanvasSlot->SetZOrder(SlotLayout.ZOrder);
		CanvasSlot->SetAutoSize(false);

		if (SlotLayout.SlotId == ECFHUDSlotId::ReticleLayer)
		{
			CanvasSlot->SetOffsets(FMargin(0.0f));
		}
		else
		{
			CanvasSlot->SetPosition(SlotLayout.PixelOffset);
			CanvasSlot->SetSize(SlotLayout.DesiredSize);
		}
		return true;
	}

	// [v1.1.0] Overlay Child가 부모 Slot 전체를 사용하도록 명시적으로 Fill 정렬을 적용합니다.
	bool ApplyOverlayFill(UOverlaySlot* OverlaySlot)
	{
		if (!OverlaySlot)
		{
			return false;
		}
		OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		OverlaySlot->SetPadding(FMargin(0.0f));
		return true;
	}

	// [v1.1.0] Panel 내부 Canvas에 Child를 절대 Screen-space 크기로 배치합니다.
	UCanvasPanelSlot* AddCanvasChild(
		UCanvasPanel* ParentCanvas,
		UWidget* ChildWidget,
		const FVector2D& Position,
		const FVector2D& Size,
		const int32 ZOrder = 0)
	{
		if (!ParentCanvas || !ChildWidget)
		{
			return nullptr;
		}

		// [v1.1.0] 현재 Panel 내부 Child가 사용할 실제 Canvas Slot입니다.
		UCanvasPanelSlot* CanvasSlot = ParentCanvas->AddChildToCanvas(ChildWidget);
		if (!CanvasSlot)
		{
			return nullptr;
		}
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D::ZeroVector);
		CanvasSlot->SetPosition(Position);
		CanvasSlot->SetSize(Size);
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetZOrder(ZOrder);
		return CanvasSlot;
	}

	// [v1.1.0] Style Typography와 1080p Role Floor로 해석한 Font를 TextBlock에 적용합니다.
	void ApplyTextStyle(
		UTextBlock* TextBlock,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData,
		const ECFUIFontFamilyRole FontFamilyRole,
		const ECFUITypographyRole TypographyRole,
		const ECFUIColorToken ColorToken)
	{
		if (!TextBlock || !StyleData || !LayoutData)
		{
			return;
		}
		TextBlock->SetFont(StyleData->ResolveSlateFontInfo(
			FontFamilyRole,
			TypographyRole,
			LayoutData->TypographyScale,
			LayoutData->ResolveMinimumFontSize(TypographyRole)));
		TextBlock->SetColorAndOpacity(FSlateColor(StyleData->ResolveColor(ColorToken)));
		TextBlock->SetAutoWrapText(false);
		TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// [v1.1.0] Panel 내부 Canvas에 정적 Mock Text를 생성하고 위치·크기·Style을 적용합니다.
	UTextBlock* AddCanvasText(
		UWidgetTree* WidgetTree,
		UCanvasPanel* ParentCanvas,
		const TCHAR* WidgetName,
		const TCHAR* DisplayText,
		const FVector2D& Position,
		const FVector2D& Size,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData,
		const ECFUIFontFamilyRole FontFamilyRole,
		const ECFUITypographyRole TypographyRole,
		const ECFUIColorToken ColorToken,
		const int32 ZOrder = 2)
	{
		// [v1.1.0] 현재 Mock 문자열을 표시할 TextBlock입니다.
		UTextBlock* TextBlock = CreateWidget<UTextBlock>(WidgetTree, WidgetName);
		if (!TextBlock)
		{
			return nullptr;
		}
		TextBlock->SetText(FText::FromString(DisplayText));
		ApplyTextStyle(TextBlock, StyleData, LayoutData, FontFamilyRole, TypographyRole, ColorToken);
		return AddCanvasChild(ParentCanvas, TextBlock, Position, Size, ZOrder) ? TextBlock : nullptr;
	}

	// [v1.1.0] Panel 내부 Canvas에 Style Token 색을 사용하는 사각 시각 요소를 생성합니다.
	UBorder* AddCanvasBorder(
		UWidgetTree* WidgetTree,
		UCanvasPanel* ParentCanvas,
		const TCHAR* WidgetName,
		const FVector2D& Position,
		const FVector2D& Size,
		const UCFUIStyleData* StyleData,
		const ECFUIColorToken ColorToken,
		const int32 ZOrder = 1)
	{
		// [v1.1.0] 현재 Background·Line·Badge·Tile을 표현할 Border입니다.
		UBorder* Border = CreateWidget<UBorder>(WidgetTree, WidgetName);
		if (!Border || !StyleData)
		{
			return nullptr;
		}
		Border->SetBrushColor(StyleData->ResolveColor(ColorToken));
		Border->SetPadding(FMargin(0.0f));
		Border->SetVisibility(ESlateVisibility::HitTestInvisible);
		return AddCanvasChild(ParentCanvas, Border, Position, Size, ZOrder) ? Border : nullptr;
	}

		// [v1.2.0] 속도계 원호를 저비용 정적 Segment로 구성할 한 개의 Track/Fill Border를 생성합니다.
	UBorder* AddSpeedArcSegment(
		UWidgetTree* WidgetTree,
		UCanvasPanel* ParentCanvas,
		const TCHAR* WidgetName,
		const FVector2D& Position,
		const FVector2D& Size,
		const UCFUIStyleData* StyleData,
		const ECFUIColorToken ColorToken,
		const int32 ZOrder = 3)
	{
		return AddCanvasBorder(WidgetTree, ParentCanvas, WidgetName, Position, Size, StyleData, ColorToken, ZOrder);
	}

	// [v1.1.0] Panel 내부에 1px 선 네 개로 사각 Outline을 만듭니다.
	bool AddCanvasOutline(
		UWidgetTree* WidgetTree,
		UCanvasPanel* ParentCanvas,
		const TCHAR* Prefix,
		const FVector2D& Position,
		const FVector2D& Size,
		const UCFUIStyleData* StyleData,
		const ECFUIColorToken ColorToken,
		const float Thickness = 1.0f,
		const int32 ZOrder = 3)
	{
		// [v1.1.0] Outline 상단 선 Widget 이름입니다.
		const FString TopName = FString::Printf(TEXT("%s_Top"), Prefix);
		// [v1.1.0] Outline 하단 선 Widget 이름입니다.
		const FString BottomName = FString::Printf(TEXT("%s_Bottom"), Prefix);
		// [v1.1.0] Outline 좌측 선 Widget 이름입니다.
		const FString LeftName = FString::Printf(TEXT("%s_Left"), Prefix);
		// [v1.1.0] Outline 우측 선 Widget 이름입니다.
		const FString RightName = FString::Printf(TEXT("%s_Right"), Prefix);

		return AddCanvasBorder(WidgetTree, ParentCanvas, *TopName, Position, FVector2D(Size.X, Thickness), StyleData, ColorToken, ZOrder)
			&& AddCanvasBorder(WidgetTree, ParentCanvas, *BottomName, FVector2D(Position.X, Position.Y + Size.Y - Thickness), FVector2D(Size.X, Thickness), StyleData, ColorToken, ZOrder)
			&& AddCanvasBorder(WidgetTree, ParentCanvas, *LeftName, Position, FVector2D(Thickness, Size.Y), StyleData, ColorToken, ZOrder)
			&& AddCanvasBorder(WidgetTree, ParentCanvas, *RightName, FVector2D(Position.X + Size.X - Thickness, Position.Y), FVector2D(Thickness, Size.Y), StyleData, ColorToken, ZOrder);
	}

	// [v1.1.0] Panel 내부 Canvas에 정적 Percent Progress Bar를 생성합니다.
	UProgressBar* AddCanvasProgress(
		UWidgetTree* WidgetTree,
		UCanvasPanel* ParentCanvas,
		const TCHAR* WidgetName,
		const FVector2D& Position,
		const FVector2D& Size,
		const float Percent,
		const UCFUIStyleData* StyleData,
		const ECFUIColorToken ColorToken,
		const int32 ZOrder = 2)
	{
		// [v1.1.0] 현재 Mock 비율을 표시할 ProgressBar입니다.
		UProgressBar* ProgressBar = CreateWidget<UProgressBar>(WidgetTree, WidgetName);
		if (!ProgressBar || !StyleData)
		{
			return nullptr;
		}
		ProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		ProgressBar->SetFillColorAndOpacity(StyleData->ResolveColor(ColorToken));
		ProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		return AddCanvasChild(ParentCanvas, ProgressBar, Position, Size, ZOrder) ? ProgressBar : nullptr;
	}

	/**
	 * 한 고정 HUD Slot의 SizeBox + Overlay + D1-10 Base Widget + Mock Surface + 절대좌표 Canvas를 보관합니다.
	 */
	struct FHUDPrototypeLayer
	{
		// [v1.1.0] Layout Data의 DesiredSize를 직접 받는 Slot Root입니다.
		USizeBox* SlotBox = nullptr;
		// [v1.1.0] D1-10 Base Widget과 D1-11 Mock Surface를 겹칠 Overlay입니다.
		UOverlay* Overlay = nullptr;
		// [v1.1.0] 읽기 전용 D1-10 Generated Class로 생성된 Base Widget Template입니다.
		UUserWidget* BaseWidget = nullptr;
		// [v1.1.0] 현재 Style의 Panel Surface를 Slot 전체에 채울 Mock Border입니다.
		UBorder* MockSurface = nullptr;
		// [v1.1.0] 승인된 D1-07 공간 구성을 고정 좌표로 재현할 내부 Canvas입니다.
		UCanvasPanel* ContentCanvas = nullptr;
	};

	// [v1.1.0] D1-11 고정 Slot의 공통 Tree와 전체 Fill Style Surface를 생성합니다.
	FHUDPrototypeLayer BuildPrototypeLayer(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		const TCHAR* SlotWidgetName,
		const TCHAR* OverlayWidgetName,
		const TCHAR* BaseWidgetName,
		const TCHAR* SurfaceWidgetName,
		const TCHAR* ContentWidgetName,
		UClass* BaseWidgetClass,
		const UCFUIStyleData* StyleData,
		const ECFUIColorToken SurfaceColorToken)
	{
		// [v1.1.0] 호출자에게 반환할 현재 Slot의 생성 결과입니다.
		FHUDPrototypeLayer Layer;
		Layer.SlotBox = CreateWidget<USizeBox>(WidgetTree, SlotWidgetName);
		Layer.Overlay = CreateWidget<UOverlay>(WidgetTree, OverlayWidgetName);
		Layer.BaseWidget = CreateNestedUserWidget(WidgetTree, BaseWidgetClass, BaseWidgetName);
		Layer.MockSurface = CreateWidget<UBorder>(WidgetTree, SurfaceWidgetName);
		Layer.ContentCanvas = CreateWidget<UCanvasPanel>(WidgetTree, ContentWidgetName);
		if (!Layer.SlotBox || !Layer.Overlay || !Layer.BaseWidget || !Layer.MockSurface || !Layer.ContentCanvas || !StyleData)
		{
			return FHUDPrototypeLayer();
		}

		Layer.SlotBox->SetWidthOverride(SlotLayout.DesiredSize.X);
		Layer.SlotBox->SetHeightOverride(SlotLayout.DesiredSize.Y);
		Layer.SlotBox->SetContent(Layer.Overlay);

				// [v1.2.0] D1-10 Base Widget Class 재사용 계약은 유지하되 Designer NamedSlot Placeholder가 Mock 위에 겹치지 않도록 완전히 숨깁니다.
		Layer.BaseWidget->SetVisibility(ESlateVisibility::Collapsed);
		Layer.BaseWidget->SetRenderOpacity(0.0f);
		Layer.MockSurface->SetVisibility(ESlateVisibility::HitTestInvisible);
		Layer.MockSurface->SetBrushColor(StyleData->ResolveColor(SurfaceColorToken));
		Layer.MockSurface->SetPadding(FMargin(0.0f));
		Layer.MockSurface->SetContent(Layer.ContentCanvas);

		// [v1.2.0] 실제 D1-11 Mock Surface가 Slot 전체를 채울 Overlay Slot입니다.
		UOverlaySlot* SurfaceOverlaySlot = Layer.Overlay->AddChildToOverlay(Layer.MockSurface);
		// [v1.2.0] D1-10 Base Widget Template을 Class 재사용 증거로 보존하면서 Designer 화면 밖에 저장할 Canvas Slot입니다.
		UCanvasPanelSlot* BaseStorageSlot = AddCanvasChild(Layer.ContentCanvas, Layer.BaseWidget, FVector2D(-4096.0f, -4096.0f), FVector2D(1.0f, 1.0f), -100);
		if (!ApplyOverlayFill(SurfaceOverlaySlot) || !BaseStorageSlot)
		{
			return FHUDPrototypeLayer();
		}
		if (!ApplyRootCanvasLayout(RootCanvas, Layer.SlotBox, SlotLayout))
		{
			return FHUDPrototypeLayer();
		}
		return Layer;
	}

	// [v1.1.0] 공통 Panel 가장자리 Outline과 좌측 Accent Line을 추가합니다.
	bool AddPanelChrome(
		UWidgetTree* WidgetTree,
		UCanvasPanel* ContentCanvas,
		const FVector2D& PanelSize,
		const UCFUIStyleData* StyleData,
		const ECFUIColorToken AccentColorToken)
	{
		return AddCanvasOutline(WidgetTree, ContentCanvas, TEXT("Border_PanelOutline"), FVector2D::ZeroVector, PanelSize, StyleData, ECFUIColorToken::LineDefault, 1.0f, 10)
			&& AddCanvasBorder(WidgetTree, ContentCanvas, TEXT("Border_PanelAccent"), FVector2D::ZeroVector, FVector2D(4.0f, PanelSize.Y), StyleData, AccentColorToken, 11) != nullptr;
	}

	// [v1.1.0] Mission Summary의 D1-07 승인 시선 흐름을 375x87에 재현합니다.
	bool BuildMissionMock(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		UClass* PanelClass,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData)
	{
		// [v1.1.0] Mission Summary Slot의 공통 Base Widget + Mock Surface 생성 결과입니다.
		FHUDPrototypeLayer Layer = BuildPrototypeLayer(
			WidgetTree, RootCanvas, SlotLayout,
			TEXT("SizeBox_Slot_MissionSummary"), TEXT("Overlay_MissionSummary"), TEXT("WBP_CFPanelBase_MissionSummary"),
			TEXT("Border_Mock_MissionSummary"), TEXT("Canvas_Mock_MissionSummary"), PanelClass, StyleData, ECFUIColorToken::SurfaceBase);
		if (!Layer.ContentCanvas || !AddPanelChrome(WidgetTree, Layer.ContentCanvas, SlotLayout.DesiredSize, StyleData, ECFUIColorToken::AccentTactical))
		{
			return false;
		}

		return AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_MissionLabel"), TEXT("현재 목표"), FVector2D(18.0f, 7.0f), FVector2D(330.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_MissionTitle"), TEXT("호송 차량을 보호하십시오"), FVector2D(18.0f, 27.0f), FVector2D(340.0f, 28.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::HeadingL, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_MissionBody"), TEXT("잔여 적대 차량 3"), FVector2D(18.0f, 60.0f), FVector2D(330.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextMuted);
	}

	// [v1.1.0] Alert Feed를 좌·우 두 경고 블록으로 분리해 D1-07 승인 구도를 재현합니다.
	bool BuildAlertMock(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		UClass* AlertClass,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData)
	{
		// [v1.1.0] Alert Feed Slot의 공통 Base Widget + Mock Surface 생성 결과입니다.
		FHUDPrototypeLayer Layer = BuildPrototypeLayer(
			WidgetTree, RootCanvas, SlotLayout,
			TEXT("SizeBox_Slot_AlertFeed"), TEXT("Overlay_AlertFeed"), TEXT("WBP_CFAlertItem_AlertFeed"),
			TEXT("Border_Mock_AlertFeed"), TEXT("Canvas_Mock_AlertFeed"), AlertClass, StyleData, ECFUIColorToken::SurfaceBase);
		if (!Layer.ContentCanvas || !AddPanelChrome(WidgetTree, Layer.ContentCanvas, SlotLayout.DesiredSize, StyleData, ECFUIColorToken::StateCaution))
		{
			return false;
		}

		// [v1.1.0] Alert 두 의미 블록 사이의 수직 구분선입니다.
		UBorder* Separator = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_AlertSeparator"), FVector2D(276.0f, 14.0f), FVector2D(1.0f, 53.0f), StyleData, ECFUIColorToken::LineSubtle, 4);
		return Separator
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_AlertWarningIcon"), TEXT("▲"), FVector2D(18.0f, 15.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::StateCaution)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_AlertPrimary"), TEXT("전방 장갑 주의"), FVector2D(48.0f, 12.0f), FVector2D(210.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::StateCaution)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_AlertArmor"), TEXT("장갑 30%"), FVector2D(48.0f, 43.0f), FVector2D(210.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_AlertRipple"), TEXT("RIPPLE 진행"), FVector2D(298.0f, 12.0f), FVector2D(225.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_AlertSecondary"), TEXT("2 / 4"), FVector2D(298.0f, 43.0f), FVector2D(225.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);
	}

	// [v1.1.0] Target Panel의 제목·정보열·스캔바를 312x232.5 전체 폭으로 재현합니다.
	bool BuildTargetMock(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		UClass* PanelClass,
		UClass* InfoRowClass,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData)
	{
		// [v1.1.0] Target Panel Slot의 공통 Base Widget + Mock Surface 생성 결과입니다.
		FHUDPrototypeLayer Layer = BuildPrototypeLayer(
			WidgetTree, RootCanvas, SlotLayout,
			TEXT("SizeBox_Slot_TargetPanel"), TEXT("Overlay_TargetPanel"), TEXT("WBP_CFPanelBase_TargetPanel"),
			TEXT("Border_Mock_TargetPanel"), TEXT("Canvas_Mock_TargetPanel"), PanelClass, StyleData, ECFUIColorToken::SurfaceBase);
		if (!Layer.ContentCanvas || !AddPanelChrome(WidgetTree, Layer.ContentCanvas, SlotLayout.DesiredSize, StyleData, ECFUIColorToken::Hostile))
		{
			return false;
		}

		// [v1.1.0] D1-10 InfoRow 재사용 계약을 보존하되 D1-11 고정 Mock이 직접 표시하므로 숨기는 Identity Row입니다.
		UUserWidget* IdentityInfoRow = CreateNestedUserWidget(WidgetTree, InfoRowClass, TEXT("WBP_CFInfoRow_TargetIdentity"));
		// [v1.1.0] D1-10 InfoRow 재사용 계약을 보존하되 D1-11 고정 Mock이 직접 표시하므로 숨기는 Armor Row입니다.
		UUserWidget* ArmorInfoRow = CreateNestedUserWidget(WidgetTree, InfoRowClass, TEXT("WBP_CFInfoRow_TargetArmor"));
		if (!IdentityInfoRow || !ArmorInfoRow)
		{
			return false;
		}
		IdentityInfoRow->SetVisibility(ESlateVisibility::Collapsed);
		ArmorInfoRow->SetVisibility(ESlateVisibility::Collapsed);
				IdentityInfoRow->SetRenderOpacity(0.0f);
		ArmorInfoRow->SetRenderOpacity(0.0f);
		if (!AddCanvasChild(Layer.ContentCanvas, IdentityInfoRow, FVector2D(-4096.0f, -4096.0f), FVector2D(1.0f, 1.0f), -100)
			|| !AddCanvasChild(Layer.ContentCanvas, ArmorInfoRow, FVector2D(-4096.0f, -4096.0f), FVector2D(1.0f, 1.0f), -100))
		{
			return false;
		}

		return AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetMarker"), TEXT("◆"), FVector2D(18.0f, 14.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::Hostile)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetTitle"), TEXT("적대 차량"), FVector2D(48.0f, 10.0f), FVector2D(240.0f, 30.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::HeadingL, ECFUIColorToken::Hostile)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetDistanceLabel"), TEXT("거리"), FVector2D(18.0f, 52.0f), FVector2D(70.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetDistance"), TEXT("842 m"), FVector2D(190.0f, 48.0f), FVector2D(102.0f, 30.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueM, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetIdentityLabel"), TEXT("식별"), FVector2D(18.0f, 88.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetIdentity"), TEXT("???"), FVector2D(222.0f, 87.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::Unknown)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetArmorLabel"), TEXT("장갑"), FVector2D(18.0f, 122.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetArmor"), TEXT("???"), FVector2D(222.0f, 121.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::Unknown)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetScanLabel"), TEXT("스캔"), FVector2D(18.0f, 156.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_TargetScan"), TEXT("35%"), FVector2D(222.0f, 155.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextPrimary)
			&& AddCanvasProgress(WidgetTree, Layer.ContentCanvas, TEXT("ProgressBar_TargetScan"), FVector2D(18.0f, 194.0f), FVector2D(274.0f, 8.0f), 0.35f, StyleData, ECFUIColorToken::AccentTactical);
	}

	// [v1.1.0] Vehicle Panel의 속도계·6방향 Armor Map·Shield·Integrity를 672x312 전체 공간에 재현합니다.
	bool BuildVehicleMock(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		UClass* PanelClass,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData)
	{
		// [v1.1.0] Vehicle Panel Slot의 공통 Base Widget + Mock Surface 생성 결과입니다.
		FHUDPrototypeLayer Layer = BuildPrototypeLayer(
			WidgetTree, RootCanvas, SlotLayout,
			TEXT("SizeBox_Slot_VehiclePanel"), TEXT("Overlay_VehiclePanel"), TEXT("WBP_CFPanelBase_VehiclePanel"),
			TEXT("Border_Mock_VehiclePanel"), TEXT("Canvas_Mock_VehiclePanel"), PanelClass, StyleData, ECFUIColorToken::SurfaceBase);
		if (!Layer.ContentCanvas || !AddPanelChrome(WidgetTree, Layer.ContentCanvas, SlotLayout.DesiredSize, StyleData, ECFUIColorToken::AccentTactical))
		{
			return false;
		}

		// [v1.1.0] 좌측 속도계 영역의 Raised Surface입니다.
		UBorder* SpeedSurface = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleSpeedSurface"), FVector2D(18.0f, 18.0f), FVector2D(240.0f, 188.0f), StyleData, ECFUIColorToken::SurfaceRaised, 1);
		// [v1.1.0] 우측 6방향 Armor Map 영역의 Raised Surface입니다.
		UBorder* ArmorSurface = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleArmorSurface"), FVector2D(270.0f, 18.0f), FVector2D(384.0f, 188.0f), StyleData, ECFUIColorToken::SurfaceRaised, 1);
						// [v1.3.0] Armor Map 중앙의 왼쪽 전방 차량 본체를 승인 Mockup 비율에 가깝게 확장한 Surface입니다.
		UBorder* BodySilhouette = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleBodySilhouette"), FVector2D(390.0f, 82.0f), FVector2D(142.0f, 64.0f), StyleData, ECFUIColorToken::SurfaceSoft, 2);
		// [v1.3.0] 왼쪽 전방 Nose의 중앙부를 좁게 만들어 차체 진행 방향을 즉시 읽게 하는 Surface입니다.
		UBorder* BodyNoseCenter = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleBodyNoseCenter"), FVector2D(370.0f, 96.0f), FVector2D(20.0f, 36.0f), StyleData, ECFUIColorToken::SurfaceSoft, 3);
		// [v1.3.0] 왼쪽 전방 Nose의 상단 Shoulder를 만들어 사각형이 아닌 단계형 전면 윤곽을 구성합니다.
		UBorder* BodyNoseUpper = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleBodyNoseUpper"), FVector2D(380.0f, 87.0f), FVector2D(20.0f, 16.0f), StyleData, ECFUIColorToken::SurfaceSoft, 3);
		// [v1.3.0] 왼쪽 전방 Nose의 하단 Shoulder를 만들어 상하 대칭 차량 윤곽을 구성합니다.
		UBorder* BodyNoseLower = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleBodyNoseLower"), FVector2D(380.0f, 131.0f), FVector2D(20.0f, 16.0f), StyleData, ECFUIColorToken::SurfaceSoft, 3);
		// [v1.3.0] 후미 폭을 줄여 왼쪽이 전방임을 실루엣 자체에서 구분하게 하는 Tail Surface입니다.
		UBorder* BodyRear = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleBodyRear"), FVector2D(532.0f, 96.0f), FVector2D(18.0f, 36.0f), StyleData, ECFUIColorToken::SurfaceSoft, 3);
		// [v1.3.0] Cabin은 채운 사각형 대신 얇은 Outline으로 표시해 본체 형상을 가리지 않습니다.
		const bool bCabinOutlineBuilt = AddCanvasOutline(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleCabin"), FVector2D(430.0f, 90.0f), FVector2D(58.0f, 48.0f), StyleData, ECFUIColorToken::LineDefault, 1.0f, 4);
		// [v1.3.0] 차체 상단 앞 Wheel을 표현하는 짧은 Surface입니다.
		UBorder* WheelFrontTop = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleWheelFrontTop"), FVector2D(404.0f, 74.0f), FVector2D(22.0f, 6.0f), StyleData, ECFUIColorToken::LineDefault, 4);
		// [v1.3.0] 차체 하단 앞 Wheel을 표현하는 짧은 Surface입니다.
		UBorder* WheelFrontBottom = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleWheelFrontBottom"), FVector2D(404.0f, 148.0f), FVector2D(22.0f, 6.0f), StyleData, ECFUIColorToken::LineDefault, 4);
		// [v1.3.0] 차체 상단 뒤 Wheel을 표현하는 짧은 Surface입니다.
		UBorder* WheelRearTop = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleWheelRearTop"), FVector2D(504.0f, 74.0f), FVector2D(22.0f, 6.0f), StyleData, ECFUIColorToken::LineDefault, 4);
		// [v1.3.0] 차체 하단 뒤 Wheel을 표현하는 짧은 Surface입니다.
		UBorder* WheelRearBottom = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_VehicleWheelRearBottom"), FVector2D(504.0f, 148.0f), FVector2D(22.0f, 6.0f), StyleData, ECFUIColorToken::LineDefault, 4);
		if (!SpeedSurface || !ArmorSurface || !BodySilhouette || !BodyNoseCenter || !BodyNoseUpper || !BodyNoseLower || !BodyRear
			|| !bCabinOutlineBuilt || !WheelFrontTop || !WheelFrontBottom || !WheelRearTop || !WheelRearBottom)
		{
			return false;
		}

				// [v1.3.0] 좌측 Front Armor Plate입니다.
		UBorder* FrontPlate = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_ArmorFront"), FVector2D(286.0f, 78.0f), FVector2D(86.0f, 52.0f), StyleData, ECFUIColorToken::SurfaceSoft, 2);
		// [v1.3.0] 상단 Right Armor Plate입니다. 차량이 왼쪽을 바라보므로 화면 상단이 차량 Right입니다.
		UBorder* RightPlate = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_ArmorRight"), FVector2D(404.0f, 33.0f), FVector2D(116.0f, 30.0f), StyleData, ECFUIColorToken::SurfaceSoft, 2);
		// [v1.3.0] 우측 Rear Armor Plate입니다.
		UBorder* RearPlate = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_ArmorRear"), FVector2D(552.0f, 78.0f), FVector2D(86.0f, 52.0f), StyleData, ECFUIColorToken::SurfaceSoft, 2);
		// [v1.3.0] 하단 Left Armor Plate입니다. 차량이 왼쪽을 바라보므로 화면 하단이 차량 Left입니다.
		UBorder* LeftPlate = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_ArmorLeft"), FVector2D(404.0f, 151.0f), FVector2D(116.0f, 34.0f), StyleData, ECFUIColorToken::SurfaceSoft, 2);
		// [v1.3.0] D1-07 승인 위치인 Armor Cluster 좌상단 Top Badge입니다.
		UBorder* TopBadge = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_ArmorTopBadge"), FVector2D(286.0f, 33.0f), FVector2D(100.0f, 30.0f), StyleData, ECFUIColorToken::SurfaceSoft, 2);
		// [v1.3.0] D1-07 승인 위치인 Armor Cluster 우하단 Bottom Badge입니다.
		UBorder* BottomBadge = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_ArmorBottomBadge"), FVector2D(538.0f, 151.0f), FVector2D(100.0f, 30.0f), StyleData, ECFUIColorToken::SurfaceSoft, 2);
		if (!FrontPlate || !RightPlate || !RearPlate || !LeftPlate || !TopBadge || !BottomBadge)
		{
			return false;
		}

				// [v1.3.0] 승인 Mockup의 원호를 계단 블록이 아니라 조밀한 짧은 Segment 열로 근사합니다.
		if (!AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack01"), FVector2D(44.0f, 158.0f), FVector2D(5.0f, 20.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack02"), FVector2D(48.0f, 143.0f), FVector2D(5.0f, 14.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack03"), FVector2D(53.0f, 130.0f), FVector2D(6.0f, 12.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack04"), FVector2D(60.0f, 117.0f), FVector2D(6.0f, 11.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack05"), FVector2D(68.0f, 105.0f), FVector2D(7.0f, 10.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack06"), FVector2D(78.0f, 94.0f), FVector2D(8.0f, 9.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack07"), FVector2D(89.0f, 84.0f), FVector2D(9.0f, 8.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack08"), FVector2D(101.0f, 76.0f), FVector2D(10.0f, 7.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack09"), FVector2D(114.0f, 69.0f), FVector2D(11.0f, 6.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack10"), FVector2D(128.0f, 64.0f), FVector2D(12.0f, 5.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack11"), FVector2D(143.0f, 60.0f), FVector2D(13.0f, 4.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcTrack12"), FVector2D(159.0f, 58.0f), FVector2D(16.0f, 4.0f), StyleData, ECFUIColorToken::LineSubtle)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill01"), FVector2D(44.0f, 158.0f), FVector2D(5.0f, 20.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill02"), FVector2D(48.0f, 143.0f), FVector2D(5.0f, 14.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill03"), FVector2D(53.0f, 130.0f), FVector2D(6.0f, 12.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill04"), FVector2D(60.0f, 117.0f), FVector2D(6.0f, 11.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill05"), FVector2D(68.0f, 105.0f), FVector2D(7.0f, 10.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill06"), FVector2D(78.0f, 94.0f), FVector2D(8.0f, 9.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill07"), FVector2D(89.0f, 84.0f), FVector2D(9.0f, 8.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill08"), FVector2D(101.0f, 76.0f), FVector2D(10.0f, 7.0f), StyleData, ECFUIColorToken::AccentTactical, 4)
			|| !AddSpeedArcSegment(WidgetTree, Layer.ContentCanvas, TEXT("Border_SpeedArcFill09"), FVector2D(114.0f, 69.0f), FVector2D(11.0f, 6.0f), StyleData, ECFUIColorToken::AccentTactical, 4))
		{
			return false;
		}

		return AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleLabel"), TEXT("SPEED"), FVector2D(32.0f, 28.0f), FVector2D(120.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextMuted)
						&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleSpeed"), TEXT("076"), FVector2D(69.0f, 116.0f), FVector2D(145.0f, 52.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::DisplayXL, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleSpeedUnit"), TEXT("km/h"), FVector2D(165.0f, 150.0f), FVector2D(72.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleFrontDirection"), TEXT("◀"), FVector2D(340.0f, 101.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::AccentTactical)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleGearD"), TEXT("D"), FVector2D(207.0f, 45.0f), FVector2D(30.0f, 32.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::HeadingL, ECFUIColorToken::AccentTactical)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleGearN"), TEXT("N"), FVector2D(207.0f, 90.0f), FVector2D(30.0f, 26.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::HeadingM, ECFUIColorToken::TextMuted)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleGearR"), TEXT("R"), FVector2D(207.0f, 128.0f), FVector2D(30.0f, 26.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::HeadingM, ECFUIColorToken::TextMuted)
						&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_ArmorFront"), TEXT("30/100"), FVector2D(294.0f, 94.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::StateCaution)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_ArmorRight"), TEXT("70/100"), FVector2D(421.0f, 38.0f), FVector2D(86.0f, 20.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::Armor)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_ArmorRear"), TEXT("60/80"), FVector2D(560.0f, 94.0f), FVector2D(70.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::Armor)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_ArmorLeft"), TEXT("63/100"), FVector2D(421.0f, 158.0f), FVector2D(86.0f, 20.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::StateDanger)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_ArmorTop"), TEXT("84/100"), FVector2D(294.0f, 38.0f), FVector2D(76.0f, 20.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::Armor)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_ArmorBottom"), TEXT("72/100"), FVector2D(546.0f, 158.0f), FVector2D(82.0f, 20.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::Armor)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleShield"), TEXT("쉴드"), FVector2D(24.0f, 222.0f), FVector2D(78.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::Shield)
			&& AddCanvasProgress(WidgetTree, Layer.ContentCanvas, TEXT("ProgressBar_VehicleShield"), FVector2D(116.0f, 227.0f), FVector2D(444.0f, 10.0f), 0.78f, StyleData, ECFUIColorToken::Shield)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleShieldValue"), TEXT("78/100"), FVector2D(574.0f, 220.0f), FVector2D(78.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleIntegrity"), TEXT("차량 내구도"), FVector2D(24.0f, 264.0f), FVector2D(105.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::Integrity)
			&& AddCanvasProgress(WidgetTree, Layer.ContentCanvas, TEXT("ProgressBar_VehicleIntegrity"), FVector2D(138.0f, 268.0f), FVector2D(422.0f, 14.0f), 0.82f, StyleData, ECFUIColorToken::Integrity)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_VehicleIntegrityValue"), TEXT("82/100"), FVector2D(574.0f, 262.0f), FVector2D(78.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextPrimary);
	}

	// [v1.1.0] Radar Panel을 텍스트 목록이 아닌 공간형 Contact Field로 재현합니다.
	bool BuildRadarMock(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		UClass* PanelClass,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData)
	{
		// [v1.1.0] Radar Panel Slot의 공통 Base Widget + Mock Surface 생성 결과입니다.
		FHUDPrototypeLayer Layer = BuildPrototypeLayer(
			WidgetTree, RootCanvas, SlotLayout,
			TEXT("SizeBox_Slot_RadarPanel"), TEXT("Overlay_RadarPanel"), TEXT("WBP_CFPanelBase_RadarPanel"),
			TEXT("Border_Mock_RadarPanel"), TEXT("Canvas_Mock_RadarPanel"), PanelClass, StyleData, ECFUIColorToken::SurfaceSoft);
		if (!Layer.ContentCanvas || !AddPanelChrome(WidgetTree, Layer.ContentCanvas, SlotLayout.DesiredSize, StyleData, ECFUIColorToken::AccentTactical))
		{
			return false;
		}

		// [v1.1.0] Radar 중심 수평 축입니다.
		UBorder* HorizontalAxis = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_RadarAxisH"), FVector2D(30.0f, 145.0f), FVector2D(225.0f, 1.0f), StyleData, ECFUIColorToken::LineSubtle, 2);
		// [v1.1.0] Radar 중심 수직 축입니다.
		UBorder* VerticalAxis = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_RadarAxisV"), FVector2D(142.0f, 48.0f), FVector2D(1.0f, 190.0f), StyleData, ECFUIColorToken::LineSubtle, 2);
		if (!HorizontalAxis || !VerticalAxis
			|| !AddCanvasOutline(WidgetTree, Layer.ContentCanvas, TEXT("Border_RadarOuterRing"), FVector2D(47.0f, 52.0f), FVector2D(190.0f, 190.0f), StyleData, ECFUIColorToken::LineDefault, 1.0f, 2)
			|| !AddCanvasOutline(WidgetTree, Layer.ContentCanvas, TEXT("Border_RadarInnerRing"), FVector2D(83.0f, 88.0f), FVector2D(118.0f, 118.0f), StyleData, ECFUIColorToken::LineSubtle, 1.0f, 2))
		{
			return false;
		}

		// [v1.1.0] 선택 Hostile Contact를 감싸는 Cyan Outer Bracket의 상단 선입니다.
		UBorder* SelectedTop = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_RadarSelectedTop"), FVector2D(196.0f, 72.0f), FVector2D(30.0f, 2.0f), StyleData, ECFUIColorToken::AccentTactical, 5);
		// [v1.1.0] 선택 Hostile Contact를 감싸는 Cyan Outer Bracket의 하단 선입니다.
		UBorder* SelectedBottom = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_RadarSelectedBottom"), FVector2D(196.0f, 102.0f), FVector2D(30.0f, 2.0f), StyleData, ECFUIColorToken::AccentTactical, 5);
		if (!SelectedTop || !SelectedBottom)
		{
			return false;
		}

		return AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_RadarTitle"), TEXT("RADAR"), FVector2D(14.0f, 10.0f), FVector2D(120.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::HeadingM, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_RadarPlayer"), TEXT("▲"), FVector2D(132.0f, 133.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::AccentTactical)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_RadarFriendly"), TEXT("●"), FVector2D(78.0f, 178.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::Friendly)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_RadarNeutral"), TEXT("■"), FVector2D(181.0f, 194.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::Neutral)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_RadarHostile"), TEXT("◆"), FVector2D(200.0f, 76.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::Hostile)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_RadarUnknown"), TEXT("◇"), FVector2D(65.0f, 82.0f), FVector2D(24.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::Unknown);
	}

	// [v1.1.0] Weapon Panel을 선택 무기·Ammo·Heat/Cooldown·Fire State·Rail 3개가 모두 보이는 Compact 구조로 재현합니다.
	bool BuildWeaponMock(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		UClass* PanelClass,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData)
	{
		// [v1.1.0] Weapon Panel Slot의 Compact Base Widget + Mock Surface 생성 결과입니다.
		FHUDPrototypeLayer Layer = BuildPrototypeLayer(
			WidgetTree, RootCanvas, SlotLayout,
			TEXT("SizeBox_Slot_WeaponPanel"), TEXT("Overlay_WeaponPanel"), TEXT("WBP_CFPanelBase_WeaponPanel"),
			TEXT("Border_Mock_WeaponPanel"), TEXT("Canvas_Mock_WeaponPanel"), PanelClass, StyleData, ECFUIColorToken::SurfaceRaised);
		if (!Layer.ContentCanvas
			|| !AddCanvasOutline(WidgetTree, Layer.ContentCanvas, TEXT("Border_WeaponSelectedOutline"), FVector2D::ZeroVector, SlotLayout.DesiredSize, StyleData, ECFUIColorToken::AccentTactical, 2.0f, 10))
		{
			return false;
		}

		// [v1.1.0] Fire State의 Caution Surface입니다.
		UBorder* FireStateSurface = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_WeaponFireState"), FVector2D(14.0f, 101.0f), FVector2D(242.0f, 28.0f), StyleData, ECFUIColorToken::SurfaceSoft, 1);
		// [v1.1.0] 첫 번째 Rocket Launcher Rail Tile입니다.
		UBorder* RailOne = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_WeaponRail1"), FVector2D(14.0f, 138.0f), FVector2D(76.0f, 42.0f), StyleData, ECFUIColorToken::SurfaceSoft, 1);
		// [v1.1.0] 두 번째 Pulse Laser Rail Tile입니다.
		UBorder* RailTwo = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_WeaponRail2"), FVector2D(97.0f, 138.0f), FVector2D(76.0f, 42.0f), StyleData, ECFUIColorToken::SurfaceSoft, 1);
		// [v1.1.0] 나머지 Weapon Rail 개수를 요약하는 세 번째 Tile입니다.
		UBorder* RailThree = AddCanvasBorder(WidgetTree, Layer.ContentCanvas, TEXT("Border_WeaponRail3"), FVector2D(180.0f, 138.0f), FVector2D(76.0f, 42.0f), StyleData, ECFUIColorToken::SurfaceSoft, 1);
		if (!FireStateSurface || !RailOne || !RailTwo || !RailThree)
		{
			return false;
		}

		return AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponIndex"), TEXT("1"), FVector2D(14.0f, 8.0f), FVector2D(20.0f, 22.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponTitle"), TEXT("HEAVY CANNON"), FVector2D(40.0f, 7.0f), FVector2D(210.0f, 24.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponAmmoLabel"), TEXT("탄약"), FVector2D(14.0f, 37.0f), FVector2D(60.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponAmmo"), TEXT("20 | 120"), FVector2D(14.0f, 53.0f), FVector2D(148.0f, 36.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueM, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponHeatLabel"), TEXT("열"), FVector2D(172.0f, 38.0f), FVector2D(30.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponHeat"), TEXT("38%"), FVector2D(213.0f, 37.0f), FVector2D(45.0f, 20.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::StateCaution)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponCooldownLabel"), TEXT("쿨다운"), FVector2D(172.0f, 65.0f), FVector2D(52.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponCooldown"), TEXT("0.8s"), FVector2D(220.0f, 64.0f), FVector2D(40.0f, 20.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponState"), TEXT("재사용 대기"), FVector2D(28.0f, 106.0f), FVector2D(110.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::StateCaution)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponRipple"), TEXT("RIPPLE 2 / 4"), FVector2D(146.0f, 106.0f), FVector2D(102.0f, 18.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::StateCaution)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponRail1"), TEXT("로켓\n2/4"), FVector2D(20.0f, 142.0f), FVector2D(64.0f, 34.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponRail2"), TEXT("레이저\n74%"), FVector2D(103.0f, 142.0f), FVector2D(64.0f, 34.0f), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextPrimary)
			&& AddCanvasText(WidgetTree, Layer.ContentCanvas, TEXT("Text_WeaponRail3"), TEXT("+2"), FVector2D(198.0f, 146.0f), FVector2D(42.0f, 26.0f), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::HeadingM, ECFUIColorToken::TextPrimary);
	}

	// [v1.1.0] Full-screen Reticle Projection Layer를 DataAsset Stretch Slot으로 생성합니다.
	bool BuildReticleLayer(UWidgetTree* WidgetTree, UCanvasPanel* RootCanvas, const FCFHUDSlotLayout& SlotLayout)
	{
		// [v1.1.0] Runtime 연결 없이 Reticle/World Marker 공간만 예약하는 Full-screen Canvas입니다.
		UCanvasPanel* ReticleLayer = CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Slot_ReticleLayer"));
		return ReticleLayer && ApplyRootCanvasLayout(RootCanvas, ReticleLayer, SlotLayout);
	}

	// [v1.1.0] D1-11에서 요구하는 7개 Layout Slot을 승인 순서대로 생성합니다.
	bool BuildAllSlots(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const UCFHUDLayoutData* LayoutData,
		const UCFUIStyleData* StyleData,
		UClass* PanelClass,
		UClass* AlertClass,
		UClass* InfoRowClass)
	{
		// [v1.1.0] Mission Summary의 승인된 1080p Slot Layout입니다.
		const FCFHUDSlotLayout* Mission = LayoutData->FindSlotLayout(ECFHUDSlotId::MissionSummary);
		// [v1.1.0] Alert Feed의 승인된 1080p Slot Layout입니다.
		const FCFHUDSlotLayout* Alert = LayoutData->FindSlotLayout(ECFHUDSlotId::AlertFeed);
		// [v1.1.0] Target Panel의 승인된 1080p Slot Layout입니다.
		const FCFHUDSlotLayout* Target = LayoutData->FindSlotLayout(ECFHUDSlotId::TargetPanel);
		// [v1.1.0] Vehicle Panel의 승인된 1080p Slot Layout입니다.
		const FCFHUDSlotLayout* Vehicle = LayoutData->FindSlotLayout(ECFHUDSlotId::VehiclePanel);
		// [v1.1.0] Radar Panel의 승인된 1080p Slot Layout입니다.
		const FCFHUDSlotLayout* Radar = LayoutData->FindSlotLayout(ECFHUDSlotId::RadarPanel);
		// [v1.1.0] Weapon Panel의 승인된 1080p Slot Layout입니다.
		const FCFHUDSlotLayout* Weapon = LayoutData->FindSlotLayout(ECFHUDSlotId::WeaponPanel);
		// [v1.1.0] Full-screen Reticle Projection Layer의 승인된 Layout입니다.
		const FCFHUDSlotLayout* Reticle = LayoutData->FindSlotLayout(ECFHUDSlotId::ReticleLayer);
		if (!Mission || !Alert || !Target || !Vehicle || !Radar || !Weapon || !Reticle)
		{
			return false;
		}

		return BuildMissionMock(WidgetTree, RootCanvas, *Mission, PanelClass, StyleData, LayoutData)
			&& BuildAlertMock(WidgetTree, RootCanvas, *Alert, AlertClass, StyleData, LayoutData)
			&& BuildTargetMock(WidgetTree, RootCanvas, *Target, PanelClass, InfoRowClass, StyleData, LayoutData)
			&& BuildVehicleMock(WidgetTree, RootCanvas, *Vehicle, PanelClass, StyleData, LayoutData)
			&& BuildRadarMock(WidgetTree, RootCanvas, *Radar, PanelClass, StyleData, LayoutData)
			&& BuildWeaponMock(WidgetTree, RootCanvas, *Weapon, PanelClass, StyleData, LayoutData)
			&& BuildReticleLayer(WidgetTree, RootCanvas, *Reticle);
	}

	// [v1.1.0] Root Canvas Slot이 Layout Data 값을 GeometryScale 재적용 없이 정확히 보존하는지 검증합니다.
	bool ValidateRootCanvasLayout(const UWidget* Widget, const FCFHUDSlotLayout& ExpectedLayout, FString& OutFailureReason)
	{
		// [v1.1.0] 현재 고정 HUD Widget이 실제로 사용하는 Root Canvas Slot입니다.
		const UCanvasPanelSlot* CanvasSlot = Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
		if (!CanvasSlot)
		{
			OutFailureReason = FString::Printf(TEXT("Canvas slot missing: %s"), Widget ? *Widget->GetName() : TEXT("None"));
			return false;
		}

		// [v1.1.0] 저장된 Root Canvas Slot에서 읽은 실제 Anchor 범위입니다.
		const FAnchors Anchors = CanvasSlot->GetAnchors();
		if (!IsNearlyEqualVector(Anchors.Minimum, ExpectedLayout.AnchorMinimum)
			|| !IsNearlyEqualVector(Anchors.Maximum, ExpectedLayout.AnchorMaximum)
			|| !IsNearlyEqualVector(CanvasSlot->GetAlignment(), ExpectedLayout.Alignment)
			|| CanvasSlot->GetZOrder() != ExpectedLayout.ZOrder)
		{
			OutFailureReason = FString::Printf(TEXT("Canvas anchor/alignment/z mismatch: %s"), *Widget->GetName());
			return false;
		}

		if (ExpectedLayout.SlotId == ECFHUDSlotId::ReticleLayer)
		{
			// [v1.1.0] Full-screen Stretch Reticle Layer의 실제 Canvas Offsets입니다.
			const FMargin Offsets = CanvasSlot->GetOffsets();
			if (!FMath::IsNearlyZero(Offsets.Left) || !FMath::IsNearlyZero(Offsets.Top) || !FMath::IsNearlyZero(Offsets.Right) || !FMath::IsNearlyZero(Offsets.Bottom))
			{
				OutFailureReason = TEXT("ReticleLayer stretch offsets are not zero");
				return false;
			}
			return true;
		}

		if (!IsNearlyEqualVector(CanvasSlot->GetPosition(), ExpectedLayout.PixelOffset)
			|| !IsNearlyEqualVector(CanvasSlot->GetSize(), ExpectedLayout.DesiredSize))
		{
			OutFailureReason = FString::Printf(TEXT("Canvas position/size mismatch: %s"), *Widget->GetName());
			return false;
		}
		return true;
	}

	// [v1.1.0] 지정 Panel 내부 Widget의 Canvas 위치와 크기가 Visual Contract와 일치하는지 검증합니다.
	bool ValidateCanvasRect(
		UWidgetTree* WidgetTree,
		const TCHAR* WidgetName,
		const FVector2D& ExpectedPosition,
		const FVector2D& ExpectedSize,
		FString& OutFailureReason)
	{
		// [v1.1.0] 이름으로 찾은 실제 D1-11 Visual Widget입니다.
		const UWidget* Widget = WidgetTree ? WidgetTree->FindWidget(FName(WidgetName)) : nullptr;
		// [v1.1.0] Visual Widget이 실제로 사용하는 내부 Canvas Slot입니다.
		const UCanvasPanelSlot* CanvasSlot = Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
		if (!CanvasSlot)
		{
			OutFailureReason = FString::Printf(TEXT("D1-11 visual canvas slot missing: %s"), WidgetName);
			return false;
		}
		if (!IsNearlyEqualVector(CanvasSlot->GetPosition(), ExpectedPosition)
			|| !IsNearlyEqualVector(CanvasSlot->GetSize(), ExpectedSize))
		{
			OutFailureReason = FString::Printf(TEXT("D1-11 visual rect mismatch: %s"), WidgetName);
			return false;
		}
		return true;
	}

	// [v1.1.0] 각 Mock Surface가 Overlay 부모 전체를 Fill하고 내부 Content가 Canvas인지 검증합니다.
	bool ValidatePrototypeSurface(UWidgetTree* WidgetTree, const TCHAR* SurfaceName, const TCHAR* CanvasName, FString& OutFailureReason)
	{
		// [v1.1.0] 현재 Slot 전체를 채워야 하는 Mock Surface Border입니다.
		const UBorder* Surface = WidgetTree ? Cast<UBorder>(WidgetTree->FindWidget(FName(SurfaceName))) : nullptr;
		// [v1.1.0] Mock Surface 아래의 절대좌표 Content Canvas입니다.
		const UCanvasPanel* ContentCanvas = WidgetTree ? Cast<UCanvasPanel>(WidgetTree->FindWidget(FName(CanvasName))) : nullptr;
		// [v1.1.0] Mock Surface의 실제 Overlay Slot입니다.
		const UOverlaySlot* OverlaySlot = Surface ? Cast<UOverlaySlot>(Surface->Slot) : nullptr;
		if (!Surface || !ContentCanvas || Surface->GetContent() != ContentCanvas || !OverlaySlot)
		{
			OutFailureReason = FString::Printf(TEXT("D1-11 prototype surface structure mismatch: %s"), SurfaceName);
			return false;
		}
		if (OverlaySlot->GetHorizontalAlignment() != HAlign_Fill || OverlaySlot->GetVerticalAlignment() != VAlign_Fill)
		{
			OutFailureReason = FString::Printf(TEXT("D1-11 prototype surface is not Fill aligned: %s"), SurfaceName);
			return false;
		}
		return true;
	}

		// [v1.2.0] D1-10 Base Widget Template이 Designer 시야 밖의 Collapsed Canvas Storage에 있는지 검증합니다.
	bool ValidateHiddenBaseWidget(UWidgetTree* WidgetTree, const TCHAR* WidgetName, FString& OutFailureReason)
	{
		// [v1.2.0] Designer Placeholder를 숨겨야 하는 실제 D1-10 Base Widget Template입니다.
		const UUserWidget* BaseWidget = WidgetTree ? Cast<UUserWidget>(WidgetTree->FindWidget(FName(WidgetName))) : nullptr;
		// [v1.2.0] Base Widget Template이 보관된 실제 Canvas Slot입니다.
		const UCanvasPanelSlot* StorageSlot = BaseWidget ? Cast<UCanvasPanelSlot>(BaseWidget->Slot) : nullptr;
		if (!BaseWidget || !StorageSlot)
		{
			OutFailureReason = FString::Printf(TEXT("D1-10 Base Widget Designer storage missing: %s"), WidgetName);
			return false;
		}
		if (BaseWidget->GetVisibility() != ESlateVisibility::Collapsed
			|| !FMath::IsNearlyZero(BaseWidget->GetRenderOpacity())
			|| StorageSlot->GetPosition().X > -1000.0f
			|| StorageSlot->GetPosition().Y > -1000.0f)
		{
			OutFailureReason = FString::Printf(TEXT("D1-10 Base Widget Designer placeholder is not fully hidden: %s"), WidgetName);
			return false;
		}
		return true;
	}

	// [v1.1.0] 지정 이름 Text가 Mock 문자열과 Typography Role 최소 크기를 보존하는지 검증합니다.
	bool ValidateText(
		UWidgetTree* WidgetTree,
		const TCHAR* WidgetName,
		const TCHAR* ExpectedText,
		const UCFHUDLayoutData* LayoutData,
		const ECFUITypographyRole TypographyRole,
		FString& OutFailureReason)
	{
		// [v1.1.0] 이름으로 찾은 실제 D1-11 Mock TextBlock입니다.
		const UTextBlock* TextBlock = WidgetTree ? Cast<UTextBlock>(WidgetTree->FindWidget(FName(WidgetName))) : nullptr;
		if (!TextBlock)
		{
			OutFailureReason = FString::Printf(TEXT("Mock text missing: %s"), WidgetName);
			return false;
		}
		if (TextBlock->GetText().ToString() != ExpectedText)
		{
			OutFailureReason = FString::Printf(TEXT("Mock text mismatch: %s actual=%s expected=%s"), WidgetName, *TextBlock->GetText().ToString(), ExpectedText);
			return false;
		}
		if (TextBlock->GetFont().Size < LayoutData->ResolveMinimumFontSize(TypographyRole))
		{
			OutFailureReason = FString::Printf(TEXT("Typography floor violated: %s actual=%.1f floor=%d"), WidgetName, TextBlock->GetFont().Size, LayoutData->ResolveMinimumFontSize(TypographyRole));
			return false;
		}
		return true;
	}

	// [v1.1.0] UE 기본 미연결 Event Node만 허용하고 실제 Runtime Logic/Binding이 없는지 검증합니다.
	bool ValidateGraphHasNoRuntimeLogic(const UWidgetBlueprint* WidgetBlueprint, FString& OutFailureReason)
	{
		if (!WidgetBlueprint)
		{
			OutFailureReason = TEXT("WidgetBlueprint is null");
			return false;
		}

		// [v1.1.0] EventGraph·FunctionGraph·MacroGraph 전체를 검사할 Graph 목록입니다.
		TArray<UEdGraph*> Graphs;
		Graphs.Append(WidgetBlueprint->UbergraphPages);
		Graphs.Append(WidgetBlueprint->FunctionGraphs);
		Graphs.Append(WidgetBlueprint->MacroGraphs);
		for (const UEdGraph* Graph : Graphs)
		{
			if (!Graph)
			{
				continue;
			}

			for (const UEdGraphNode* Node : Graph->Nodes)
			{
				if (!Node)
				{
					continue;
				}

				// [v1.1.0] UE가 새 Widget Blueprint에 자동 배치하는 기본 Event Node인지 판별하는 실제 Class 이름입니다.
				const FString NodeClassName = Node->GetClass()->GetName();
				if (NodeClassName != TEXT("K2Node_Event"))
				{
					OutFailureReason = FString::Printf(TEXT("D1-11 non-event Blueprint Graph node is forbidden: graph=%s node=%s class=%s"), *Graph->GetName(), *Node->GetName(), *NodeClassName);
					return false;
				}

				for (const UEdGraphPin* Pin : Node->Pins)
				{
					if (Pin && Pin->LinkedTo.Num() > 0)
					{
						OutFailureReason = FString::Printf(TEXT("D1-11 Runtime Event binding is forbidden: graph=%s node=%s pin=%s links=%d"), *Graph->GetName(), *Node->GetName(), *Pin->PinName.ToString(), Pin->LinkedTo.Num());
						return false;
					}
				}
			}
		}
		return true;
	}

	// [v1.1.0] 한 Slot의 1920x1080 Screen Rectangle을 Layout Anchor·Offset·Alignment에서 계산합니다.
	FBox2D ResolveFixedSlotRect(const FCFHUDSlotLayout& SlotLayout, const FVector2D& ReferenceViewport)
	{
		// [v1.1.0] 고정 Slot Anchor가 1920x1080 화면에서 가리키는 실제 Pixel 위치입니다.
		const FVector2D AnchorPosition(ReferenceViewport.X * SlotLayout.AnchorMinimum.X, ReferenceViewport.Y * SlotLayout.AnchorMinimum.Y);
		// [v1.1.0] Alignment와 PixelOffset을 반영한 고정 Slot의 실제 좌상단 좌표입니다.
		const FVector2D TopLeft = AnchorPosition + SlotLayout.PixelOffset - FVector2D(SlotLayout.Alignment.X * SlotLayout.DesiredSize.X, SlotLayout.Alignment.Y * SlotLayout.DesiredSize.Y);
		return FBox2D(TopLeft, TopLeft + SlotLayout.DesiredSize);
	}

	// [v1.1.0] 승인된 고정 HUD Slot이 화면 중심 전투 시야 점을 침범하지 않는지 검증합니다.
	bool ValidateCenterProtection(const UCFHUDLayoutData* LayoutData, FString& OutFailureReason)
	{
		// [v1.1.0] 중앙 전투 시야 보호 여부를 판정할 1920x1080 Viewport 중심점입니다.
		const FVector2D ViewCenter(LayoutData->ReferenceViewportSize.X * 0.5, LayoutData->ReferenceViewportSize.Y * 0.5);
		for (uint8 SlotIndex = static_cast<uint8>(ECFHUDSlotId::MissionSummary); SlotIndex <= static_cast<uint8>(ECFHUDSlotId::WeaponPanel); ++SlotIndex)
		{
			// [v1.1.0] 현재 중앙 침범 여부를 검사할 고정 HUD Slot Layout입니다.
			const FCFHUDSlotLayout* SlotLayout = LayoutData->FindSlotLayout(static_cast<ECFHUDSlotId>(SlotIndex));
			if (SlotLayout && ResolveFixedSlotRect(*SlotLayout, LayoutData->ReferenceViewportSize).IsInside(ViewCenter))
			{
				OutFailureReason = FString::Printf(TEXT("Center combat view is covered by fixed HUD slot id=%d"), SlotIndex);
				return false;
			}
		}
		return true;
	}

	// [v1.1.0] 사용자 Preview에서 발견된 내부 축소·목록형 회귀를 자동 검출할 Visual Fidelity 계약을 검증합니다.
	bool ValidateVisualFidelity(UWidgetTree* WidgetTree, FString& OutFailureReason)
	{
		if (!WidgetTree)
		{
			OutFailureReason = TEXT("WidgetTree is null during visual fidelity validation");
			return false;
		}

		if (!ValidatePrototypeSurface(WidgetTree, TEXT("Border_Mock_MissionSummary"), TEXT("Canvas_Mock_MissionSummary"), OutFailureReason)
			|| !ValidatePrototypeSurface(WidgetTree, TEXT("Border_Mock_AlertFeed"), TEXT("Canvas_Mock_AlertFeed"), OutFailureReason)
			|| !ValidatePrototypeSurface(WidgetTree, TEXT("Border_Mock_TargetPanel"), TEXT("Canvas_Mock_TargetPanel"), OutFailureReason)
			|| !ValidatePrototypeSurface(WidgetTree, TEXT("Border_Mock_VehiclePanel"), TEXT("Canvas_Mock_VehiclePanel"), OutFailureReason)
			|| !ValidatePrototypeSurface(WidgetTree, TEXT("Border_Mock_RadarPanel"), TEXT("Canvas_Mock_RadarPanel"), OutFailureReason)
			|| !ValidatePrototypeSurface(WidgetTree, TEXT("Border_Mock_WeaponPanel"), TEXT("Canvas_Mock_WeaponPanel"), OutFailureReason))
		{
			return false;
		}

						if (!ValidateCanvasRect(WidgetTree, TEXT("Border_VehicleSpeedSurface"), FVector2D(18.0f, 18.0f), FVector2D(240.0f, 188.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_VehicleArmorSurface"), FVector2D(270.0f, 18.0f), FVector2D(384.0f, 188.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_VehicleBodySilhouette"), FVector2D(390.0f, 82.0f), FVector2D(142.0f, 64.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_VehicleBodyNoseCenter"), FVector2D(370.0f, 96.0f), FVector2D(20.0f, 36.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_VehicleCabin_Top"), FVector2D(430.0f, 90.0f), FVector2D(58.0f, 1.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_SpeedArcFill01"), FVector2D(44.0f, 158.0f), FVector2D(5.0f, 20.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_SpeedArcFill09"), FVector2D(114.0f, 69.0f), FVector2D(11.0f, 6.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_ArmorFront"), FVector2D(286.0f, 78.0f), FVector2D(86.0f, 52.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_ArmorRight"), FVector2D(404.0f, 33.0f), FVector2D(116.0f, 30.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_ArmorRear"), FVector2D(552.0f, 78.0f), FVector2D(86.0f, 52.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_ArmorLeft"), FVector2D(404.0f, 151.0f), FVector2D(116.0f, 34.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_ArmorTopBadge"), FVector2D(286.0f, 33.0f), FVector2D(100.0f, 30.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_ArmorBottomBadge"), FVector2D(538.0f, 151.0f), FVector2D(100.0f, 30.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("ProgressBar_VehicleShield"), FVector2D(116.0f, 227.0f), FVector2D(444.0f, 10.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("ProgressBar_VehicleIntegrity"), FVector2D(138.0f, 268.0f), FVector2D(422.0f, 14.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("ProgressBar_TargetScan"), FVector2D(18.0f, 194.0f), FVector2D(274.0f, 8.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_WeaponRail1"), FVector2D(14.0f, 138.0f), FVector2D(76.0f, 42.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_WeaponRail2"), FVector2D(97.0f, 138.0f), FVector2D(76.0f, 42.0f), OutFailureReason)
			|| !ValidateCanvasRect(WidgetTree, TEXT("Border_WeaponRail3"), FVector2D(180.0f, 138.0f), FVector2D(76.0f, 42.0f), OutFailureReason))
		{
			return false;
		}

				if (!ValidateHiddenBaseWidget(WidgetTree, TEXT("WBP_CFPanelBase_MissionSummary"), OutFailureReason)
			|| !ValidateHiddenBaseWidget(WidgetTree, TEXT("WBP_CFAlertItem_AlertFeed"), OutFailureReason)
			|| !ValidateHiddenBaseWidget(WidgetTree, TEXT("WBP_CFPanelBase_TargetPanel"), OutFailureReason)
			|| !ValidateHiddenBaseWidget(WidgetTree, TEXT("WBP_CFPanelBase_VehiclePanel"), OutFailureReason)
			|| !ValidateHiddenBaseWidget(WidgetTree, TEXT("WBP_CFPanelBase_RadarPanel"), OutFailureReason)
			|| !ValidateHiddenBaseWidget(WidgetTree, TEXT("WBP_CFPanelBase_WeaponPanel"), OutFailureReason))
		{
			return false;
		}

				if (!WidgetTree->FindWidget(FName(TEXT("Text_VehicleFrontDirection")))
			|| !WidgetTree->FindWidget(FName(TEXT("Border_VehicleBodyNoseUpper")))
			|| !WidgetTree->FindWidget(FName(TEXT("Border_VehicleBodyNoseLower")))
			|| !WidgetTree->FindWidget(FName(TEXT("Border_VehicleBodyRear")))
			|| !WidgetTree->FindWidget(FName(TEXT("Border_VehicleWheelFrontTop"))))
		{
			OutFailureReason = TEXT("Vehicle approved arc/silhouette visual contract is incomplete");
			return false;
		}

		if (!WidgetTree->FindWidget(FName(TEXT("Text_RadarFriendly")))
			|| !WidgetTree->FindWidget(FName(TEXT("Text_RadarNeutral")))
			|| !WidgetTree->FindWidget(FName(TEXT("Text_RadarHostile")))
			|| !WidgetTree->FindWidget(FName(TEXT("Text_RadarUnknown")))
			|| !WidgetTree->FindWidget(FName(TEXT("Border_RadarSelectedTop"))))
		{
			OutFailureReason = TEXT("Radar spatial contact field contract is incomplete");
			return false;
		}

		if (WidgetTree->FindWidget(FName(TEXT("VerticalBox_Mock_VehiclePanel")))
			|| WidgetTree->FindWidget(FName(TEXT("VerticalBox_Mock_RadarPanel")))
			|| WidgetTree->FindWidget(FName(TEXT("VerticalBox_Mock_WeaponPanel"))))
		{
			OutFailureReason = TEXT("Legacy D1-11 VerticalBox mock residue is forbidden after Preview FAIL fix");
			return false;
		}
		return true;
	}
#endif
}

// [v1.1.0] D1-11 Layout·Style·Density와 기존 Base Widget을 사용해 WBP_CFInGameHUD Designer Tree를 생성 또는 재구축합니다.
bool UCFUIHUDEditorBridge::BuildHUDPrototypeResult(
	UObject* WidgetBlueprintObject,
	UCFHUDLayoutData* LayoutData,
	UCFUIStyleData* StyleData,
	UCFUIDensityData* StandardDensityData,
	UCFUIDensityData* CompactDensityData,
	UObject* PanelBlueprintObject,
	UObject* AlertBlueprintObject,
	UObject* InfoRowBlueprintObject)
{
#if WITH_EDITOR
	// [v1.1.0] D1-11 Build 선행 조건과 Base Widget Class 해석의 상세 실패 사유입니다.
	FString FailureReason;
	if (!CFUIHUDEditorBridge::ValidateInputData(LayoutData, StyleData, StandardDensityData, CompactDensityData, FailureReason))
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	// [v1.1.0] Python에서 전달된 D1-11 Target Widget Blueprint입니다.
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	if (!WidgetBlueprint || WidgetBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass())
	{
		return CFUIHUDEditorBridge::Fail(TEXT("WBP_CFInGameHUD must exist and inherit exactly CFStyledWidgetBase"));
	}

	// [v1.1.0] 기존 D1-10 Panel Base Generated Class입니다.
	UClass* PanelClass = CFUIHUDEditorBridge::ResolveGeneratedWidgetClass(PanelBlueprintObject, TEXT("PanelBase"), FailureReason);
	// [v1.1.0] 기존 D1-10 Alert Base Generated Class입니다.
	UClass* AlertClass = CFUIHUDEditorBridge::ResolveGeneratedWidgetClass(AlertBlueprintObject, TEXT("AlertItem"), FailureReason);
	// [v1.1.0] 기존 D1-10 InfoRow Base Generated Class입니다.
	UClass* InfoRowClass = CFUIHUDEditorBridge::ResolveGeneratedWidgetClass(InfoRowBlueprintObject, TEXT("InfoRow"), FailureReason);
	if (!PanelClass || !AlertClass || !InfoRowClass)
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	// [v1.1.0] 기존 Preview FAIL Tree와 이름 충돌 없이 교체할 새 Designer WidgetTree입니다.
	UWidgetTree* WidgetTree = CFUIHUDEditorBridge::ReplaceWidgetTree(WidgetBlueprint, FailureReason);
	if (!WidgetTree)
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	// [v1.1.0] D1-11의 정확한 Root Canvas입니다.
	UCanvasPanel* RootCanvas = CFUIHUDEditorBridge::CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Root"));
	if (!RootCanvas)
	{
		return CFUIHUDEditorBridge::Fail(TEXT("CanvasPanel_Root construction failed"));
	}
	WidgetTree->RootWidget = RootCanvas;

	if (!CFUIHUDEditorBridge::BuildAllSlots(WidgetTree, RootCanvas, LayoutData, StyleData, PanelClass, AlertClass, InfoRowClass))
	{
		return CFUIHUDEditorBridge::Fail(TEXT("D1-11 7-Slot Visual Fidelity Designer Tree construction failed"));
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	return true;
#else
	return false;
#endif
}

// [v1.1.0] 저장된 WBP_CFInGameHUD의 Parent·7 Slot·Visual Fidelity·Typography·Graph 0 계약을 읽기 전용으로 검증합니다.
bool UCFUIHUDEditorBridge::ValidateHUDPrototypeResult(
	UObject* WidgetBlueprintObject,
	UCFHUDLayoutData* LayoutData,
	UCFUIStyleData* StyleData,
	UCFUIDensityData* StandardDensityData,
	UCFUIDensityData* CompactDensityData,
	UObject* PanelBlueprintObject,
	UObject* AlertBlueprintObject,
	UObject* InfoRowBlueprintObject)
{
#if WITH_EDITOR
	// [v1.1.0] D1-11 Readback 검증 중 발생한 상세 실패 사유입니다.
	FString FailureReason;
	if (!CFUIHUDEditorBridge::ValidateInputData(LayoutData, StyleData, StandardDensityData, CompactDensityData, FailureReason))
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	// [v1.1.0] 저장된 D1-11 Target을 읽기 전용으로 해석한 Widget Blueprint입니다.
	const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass())
	{
		return CFUIHUDEditorBridge::Fail(TEXT("WBP_CFInGameHUD Blueprint/Tree/Native Parent validation failed"));
	}

	// [v1.1.0] D1-10 Panel Base의 저장된 Generated Class입니다.
	UClass* PanelClass = CFUIHUDEditorBridge::ResolveGeneratedWidgetClass(PanelBlueprintObject, TEXT("PanelBase"), FailureReason);
	// [v1.1.0] D1-10 Alert Base의 저장된 Generated Class입니다.
	UClass* AlertClass = CFUIHUDEditorBridge::ResolveGeneratedWidgetClass(AlertBlueprintObject, TEXT("AlertItem"), FailureReason);
	// [v1.1.0] D1-10 InfoRow Base의 저장된 Generated Class입니다.
	UClass* InfoRowClass = CFUIHUDEditorBridge::ResolveGeneratedWidgetClass(InfoRowBlueprintObject, TEXT("InfoRow"), FailureReason);
	if (!PanelClass || !AlertClass || !InfoRowClass)
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	// [v1.1.0] 저장된 WBP_CFInGameHUD Designer Tree의 실제 Root Canvas입니다.
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas || RootCanvas->GetFName() != FName(TEXT("CanvasPanel_Root")) || RootCanvas->GetChildrenCount() != 7)
	{
		return CFUIHUDEditorBridge::Fail(TEXT("CanvasPanel_Root must contain exactly 7 D1-11 slots"));
	}

	/**
	 * Root Canvas의 한 승인 Slot ID·이름·Native Widget Class 계약입니다.
	 */
	struct FExpectedSlot
	{
		// [v1.1.0] Layout Data에서 대응시킬 HUD Slot ID입니다.
		ECFHUDSlotId SlotId;
		// [v1.1.0] Designer Tree에서 요구하는 정확한 Root Child 이름입니다.
		const TCHAR* WidgetName;
		// [v1.1.0] Designer Tree Root Child가 가져야 할 정확한 Native Widget Class입니다.
		UClass* WidgetClass;
	};

	// [v1.1.0] D1-11 Root Canvas가 정확한 순서로 가져야 할 7개 Slot 계약입니다.
	const FExpectedSlot ExpectedSlots[] =
	{
		{ECFHUDSlotId::MissionSummary, TEXT("SizeBox_Slot_MissionSummary"), USizeBox::StaticClass()},
		{ECFHUDSlotId::AlertFeed, TEXT("SizeBox_Slot_AlertFeed"), USizeBox::StaticClass()},
		{ECFHUDSlotId::TargetPanel, TEXT("SizeBox_Slot_TargetPanel"), USizeBox::StaticClass()},
		{ECFHUDSlotId::VehiclePanel, TEXT("SizeBox_Slot_VehiclePanel"), USizeBox::StaticClass()},
		{ECFHUDSlotId::RadarPanel, TEXT("SizeBox_Slot_RadarPanel"), USizeBox::StaticClass()},
		{ECFHUDSlotId::WeaponPanel, TEXT("SizeBox_Slot_WeaponPanel"), USizeBox::StaticClass()},
		{ECFHUDSlotId::ReticleLayer, TEXT("CanvasPanel_Slot_ReticleLayer"), UCanvasPanel::StaticClass()}
	};

	for (int32 SlotIndex = 0; SlotIndex < UE_ARRAY_COUNT(ExpectedSlots); ++SlotIndex)
	{
		// [v1.1.0] 현재 순서에서 요구되는 Slot ID·이름·Class 계약입니다.
		const FExpectedSlot& ExpectedSlot = ExpectedSlots[SlotIndex];
		// [v1.1.0] 저장된 Root Canvas의 현재 순서 실제 Child Widget입니다.
		UWidget* ActualWidget = RootCanvas->GetChildAt(SlotIndex);
		// [v1.1.0] 현재 Slot ID에 대응하는 승인 1080p Layout Data 항목입니다.
		const FCFHUDSlotLayout* LayoutEntry = LayoutData->FindSlotLayout(ExpectedSlot.SlotId);
		if (!ActualWidget || !LayoutEntry || ActualWidget->GetFName() != FName(ExpectedSlot.WidgetName) || !ActualWidget->IsA(ExpectedSlot.WidgetClass))
		{
			return CFUIHUDEditorBridge::Fail(FString::Printf(TEXT("D1-11 slot identity mismatch at index %d"), SlotIndex));
		}
		if (!CFUIHUDEditorBridge::ValidateRootCanvasLayout(ActualWidget, *LayoutEntry, FailureReason))
		{
			return CFUIHUDEditorBridge::Fail(FailureReason);
		}
	}

	// [v1.1.0] Mission Summary가 재사용하는 실제 D1-10 Panel Base 인스턴스입니다.
	const UUserWidget* MissionPanel = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFPanelBase_MissionSummary"))));
	// [v1.1.0] Alert Feed가 재사용하는 실제 D1-10 Alert Item 인스턴스입니다.
	const UUserWidget* AlertItem = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFAlertItem_AlertFeed"))));
	// [v1.1.0] Target Panel이 재사용하는 실제 D1-10 Panel Base 인스턴스입니다.
	const UUserWidget* TargetPanel = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFPanelBase_TargetPanel"))));
	// [v1.1.0] Vehicle Panel이 재사용하는 실제 D1-10 Panel Base 인스턴스입니다.
	const UUserWidget* VehiclePanel = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFPanelBase_VehiclePanel"))));
	// [v1.1.0] Radar Panel이 재사용하는 실제 D1-10 Panel Base 인스턴스입니다.
	const UUserWidget* RadarPanel = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFPanelBase_RadarPanel"))));
	// [v1.1.0] Weapon Panel이 재사용하는 실제 D1-10 Panel Base 인스턴스입니다.
	const UUserWidget* WeaponPanel = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFPanelBase_WeaponPanel"))));
	// [v1.1.0] Target Identity Mock 구조가 재사용하는 D1-10 InfoRow 인스턴스입니다.
	const UUserWidget* IdentityInfoRow = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFInfoRow_TargetIdentity"))));
	// [v1.1.0] Target Armor Mock 구조가 재사용하는 D1-10 InfoRow 인스턴스입니다.
	const UUserWidget* ArmorInfoRow = Cast<UUserWidget>(WidgetBlueprint->WidgetTree->FindWidget(FName(TEXT("WBP_CFInfoRow_TargetArmor"))));
	if (!MissionPanel || MissionPanel->GetClass() != PanelClass
		|| !AlertItem || AlertItem->GetClass() != AlertClass
		|| !TargetPanel || TargetPanel->GetClass() != PanelClass
		|| !VehiclePanel || VehiclePanel->GetClass() != PanelClass
		|| !RadarPanel || RadarPanel->GetClass() != PanelClass
		|| !WeaponPanel || WeaponPanel->GetClass() != PanelClass
		|| !IdentityInfoRow || IdentityInfoRow->GetClass() != InfoRowClass
		|| !ArmorInfoRow || ArmorInfoRow->GetClass() != InfoRowClass)
	{
		return CFUIHUDEditorBridge::Fail(TEXT("D1-10 Base Widget class reuse contract failed"));
	}

	if (!CFUIHUDEditorBridge::ValidateVisualFidelity(WidgetBlueprint->WidgetTree, FailureReason))
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	if (!CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_MissionTitle"), TEXT("호송 차량을 보호하십시오"), LayoutData, ECFUITypographyRole::HeadingL, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_AlertSecondary"), TEXT("2 / 4"), LayoutData, ECFUITypographyRole::Caption, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_TargetDistance"), TEXT("842 m"), LayoutData, ECFUITypographyRole::ValueM, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_TargetIdentity"), TEXT("???"), LayoutData, ECFUITypographyRole::Label, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_TargetArmor"), TEXT("???"), LayoutData, ECFUITypographyRole::Label, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_TargetScan"), TEXT("35%"), LayoutData, ECFUITypographyRole::ValueS, FailureReason)
				|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_VehicleSpeed"), TEXT("076"), LayoutData, ECFUITypographyRole::DisplayXL, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_ArmorFront"), TEXT("30/100"), LayoutData, ECFUITypographyRole::ValueS, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_ArmorRight"), TEXT("70/100"), LayoutData, ECFUITypographyRole::Caption, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_ArmorRear"), TEXT("60/80"), LayoutData, ECFUITypographyRole::ValueS, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_ArmorLeft"), TEXT("63/100"), LayoutData, ECFUITypographyRole::Caption, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_ArmorTop"), TEXT("84/100"), LayoutData, ECFUITypographyRole::Caption, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_ArmorBottom"), TEXT("72/100"), LayoutData, ECFUITypographyRole::Caption, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_VehicleShieldValue"), TEXT("78/100"), LayoutData, ECFUITypographyRole::ValueS, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_VehicleIntegrityValue"), TEXT("82/100"), LayoutData, ECFUITypographyRole::ValueS, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_WeaponTitle"), TEXT("HEAVY CANNON"), LayoutData, ECFUITypographyRole::Body, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_WeaponAmmo"), TEXT("20 | 120"), LayoutData, ECFUITypographyRole::ValueM, FailureReason)
		|| !CFUIHUDEditorBridge::ValidateText(WidgetBlueprint->WidgetTree, TEXT("Text_WeaponHeat"), TEXT("38%"), LayoutData, ECFUITypographyRole::ValueS, FailureReason))
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	// [v1.1.0] Weapon Compact 회귀를 막기 위해 직접 비교할 승인 1080p Weapon Slot입니다.
	const FCFHUDSlotLayout* WeaponLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::WeaponPanel);
	if (!WeaponLayout || !CFUIHUDEditorBridge::IsNearlyEqualVector(WeaponLayout->DesiredSize, FVector2D(270.0, 190.5)))
	{
		return CFUIHUDEditorBridge::Fail(TEXT("Weapon Compact regression: D1-11 requires exact 270x190.5 1080p slot"));
	}
	if (!CFUIHUDEditorBridge::ValidateCenterProtection(LayoutData, FailureReason))
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}
	if (!CFUIHUDEditorBridge::ValidateGraphHasNoRuntimeLogic(WidgetBlueprint, FailureReason))
	{
		return CFUIHUDEditorBridge::Fail(FailureReason);
	}

	return true;
#else
	return false;
#endif
}
