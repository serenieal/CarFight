// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.1
// Date: 2026-08-13
// Description: CF-FQ-032 D1-11 Production HUD 의미 단위 UMG Editor Bridge 구현
// Scope: Root Canvas는 Slot 배치만, Panel/Element는 Image·TextBlock·ProgressBar·Box Layout 중심으로 생성합니다.
// Changelog:
// - v1.5.1: Header가 Root Tree에 붙기 전 제목을 WidgetTree에서 찾던 생성 순서 결함을 제거하고 Header 직접 자식에서 Title Slot을 해석.
// - v1.5.0: WeaponPanel Reserve가 고정 Spacer 때문에 실제 패널 밖으로 클리핑되던 결함을 제거하고 제목 Fill + 우측 Reserve Slot으로 교정. Reserve 0도 같은 슬롯에 표시.
// - v1.4.0: WeaponPanel에 Loaded/Capacity Primary Ammo와 Header의 label-less Reserve 숫자 슬롯을 추가하고 의미 슬롯 Validator를 확장.
// - v1.3.0: 승인 WeaponPanel 설계의 정상 Launcher Sequence Primary Action을 위한 전용 Row/Text/Progress 슬롯과 Validator를 추가.
// - v1.2.0: SpeedArcTrack이 비어 있거나 Load 실패일 때 Image_RPMTrackArt를 Collapsed로 저장하고, 빈 Brush가 가시 상태로 남는 회귀를 Validator에서 차단.
// - v1.1.0: SpeedGauge를 실제 RPM 의미의 정적 21 Tick/85% Red Zone Preview로 교정하고 가짜 Speed/RPM Fallback과 D Gear를 제거했으며 Armor 숫자를 6방향 세로 Bar로 교체하고 회귀 검증을 추가.
// - v1.0.0: 8개 의미 Child Widget과 6개 Panel을 조립하는 Root Tree, Semantic Icon Brush, Border 제한, Graph 0 검증을 최초 구현.
// Migration:
// - Border는 Panel Surface 한 장에만 허용하며 차체·장갑·원호·아이콘·장식선을 작은 Border 조각으로 그리지 않습니다.
// - SpeedGauge, ArmorBodyMap과 Radar의 공간 배치에는 의미 좌표가 필요한 내부 CanvasPanel만 사용합니다.
// - SpeedArcTrack은 실제 HUDVisualData Texture만 사용하며 비어 있거나 Load 실패하면 Image_RPMTrackArt 슬롯은 유지하되 Collapsed로 저장합니다.
// - Vehicle/Armor처럼 의미가 동일한 시각 슬롯은 HUDVisualData가 비어 있을 때 기존 StyleData Semantic Icon Texture를 Editor Preview Fallback으로 사용할 수 있습니다.

#include "UI/CFUIHUDProdEditorBridge.h"

#include "UI/CFHUDLayoutData.h"
#include "UI/CFHUDVisualData.h"
#include "UI/CFStyledWidgetBase.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIStyleData.h"

#if WITH_EDITOR
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Texture2D.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Styling/SlateColor.h"
#include "WidgetBlueprint.h"
#endif

namespace CFUIHUDProdEditorBridge
{
#if WITH_EDITOR
	// [v1.0.0] Production HUD 검증 실패를 공통 로그 형식으로 남기고 false를 반환합니다.
	bool Fail(const FString& FailureReason)
	{
		UE_LOG(LogTemp, Error, TEXT("[CarFight][UIHUDProdEditorBridge] %s"), *FailureReason);
		return false;
	}

	// [v1.0.0] WidgetTree에서 지정 Native Widget을 고유 이름으로 생성합니다.
	template <typename TWidget>
	TWidget* CreateWidget(UWidgetTree* WidgetTree, const TCHAR* WidgetName)
	{
		return WidgetTree ? WidgetTree->ConstructWidget<TWidget>(TWidget::StaticClass(), FName(WidgetName)) : nullptr;
	}

	// [v1.0.0] 읽기 전용 Widget Blueprint의 Generated Class를 Production Child Class로 해석합니다.
	UClass* ResolveGeneratedWidgetClass(UObject* BlueprintObject, const TCHAR* Label, FString& OutFailureReason)
	{
		// [v1.0.0] Python에서 전달된 실제 Widget Blueprint입니다.
		const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(BlueprintObject);
		if (!WidgetBlueprint || !WidgetBlueprint->GeneratedClass || !WidgetBlueprint->GeneratedClass->IsChildOf(UCFStyledWidgetBase::StaticClass()))
		{
			OutFailureReason = FString::Printf(TEXT("%s Widget Blueprint/GeneratedClass is invalid"), Label);
			return nullptr;
		}
		return WidgetBlueprint->GeneratedClass;
	}

	// [v1.0.0] 기존 Designer Tree를 이름 충돌 없이 교체할 새 WidgetTree를 생성합니다.
	UWidgetTree* ReplaceWidgetTree(UWidgetBlueprint* WidgetBlueprint, FString& OutFailureReason)
	{
		if (!WidgetBlueprint)
		{
			OutFailureReason = TEXT("WidgetBlueprint is null while replacing production tree");
			return nullptr;
		}
		WidgetBlueprint->Modify();
		// [v1.0.0] 기존 Tree와 독립된 새 Transactional WidgetTree입니다.
		UWidgetTree* NewWidgetTree = NewObject<UWidgetTree>(WidgetBlueprint, UWidgetTree::StaticClass(), NAME_None, RF_Transactional);
		if (!NewWidgetTree)
		{
			OutFailureReason = TEXT("Production WidgetTree creation failed");
			return nullptr;
		}
		WidgetBlueprint->WidgetTree = NewWidgetTree;
		NewWidgetTree->Modify();
		return NewWidgetTree;
	}

	// [v1.0.0] D1-09B DataAsset과 1080p Profile이 Production D1-11 입력 계약을 만족하는지 검증합니다.
	bool ValidateInputData(
		const UCFHUDLayoutData* LayoutData,
		const UCFUIStyleData* StyleData,
		const UCFUIDensityData* StandardDensityData,
		const UCFUIDensityData* CompactDensityData,
		FString& OutFailureReason)
	{
		if (!LayoutData || !StyleData || !StandardDensityData || !CompactDensityData)
		{
			OutFailureReason = TEXT("Production HUD Layout/Style/Density input is null");
			return false;
		}
		// [v1.0.0] 각 입력 DataAsset Validator가 반환하는 상세 실패 사유입니다.
		FString ValidationFailure;
		if (!LayoutData->ValidateLayoutData(ValidationFailure)
			|| !StyleData->ValidateStyleData(ValidationFailure)
			|| !StandardDensityData->ValidateDensityData(ValidationFailure)
			|| !CompactDensityData->ValidateDensityData(ValidationFailure))
		{
			OutFailureReason = FString::Printf(TEXT("Production HUD input validation failed: %s"), *ValidationFailure);
			return false;
		}
		if (LayoutData->ProfileId != FName(TEXT("HUD_1080_16"))
			|| StandardDensityData->Preset != ECFUIDensityPreset::Standard
			|| CompactDensityData->Preset != ECFUIDensityPreset::Compact)
		{
			OutFailureReason = TEXT("Production D1-11 requires HUD_1080_16 + Standard/Compact density inputs");
			return false;
		}
		return true;
	}

	// [v1.0.0] Style Typography와 1080p Role Floor로 해석한 Font/Color를 TextBlock에 적용합니다.
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
		TextBlock->SetFont(StyleData->ResolveSlateFontInfo(FontFamilyRole, TypographyRole, LayoutData->TypographyScale, LayoutData->ResolveMinimumFontSize(TypographyRole)));
		TextBlock->SetColorAndOpacity(FSlateColor(StyleData->ResolveColor(ColorToken)));
		TextBlock->SetAutoWrapText(false);
		TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// [v1.0.0] 정적 Prototype TextBlock을 생성하고 Style을 적용합니다.
	UTextBlock* CreateText(
		UWidgetTree* WidgetTree,
		const TCHAR* WidgetName,
		const TCHAR* DisplayText,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData,
		const ECFUIFontFamilyRole FontFamilyRole,
		const ECFUITypographyRole TypographyRole,
		const ECFUIColorToken ColorToken)
	{
		// [v1.0.0] 현재 정적 Prototype 문자열을 표시할 TextBlock입니다.
		UTextBlock* TextBlock = CreateWidget<UTextBlock>(WidgetTree, WidgetName);
		if (!TextBlock)
		{
			return nullptr;
		}
		TextBlock->SetText(FText::FromString(DisplayText));
		ApplyTextStyle(TextBlock, StyleData, LayoutData, FontFamilyRole, TypographyRole, ColorToken);
		return TextBlock;
	}

	// [v1.0.0] Semantic ID 또는 명시 Texture Soft Reference를 실제 Editor Preview용 Texture로 해석합니다.
	UTexture2D* ResolveTexture(
		const UCFUIStyleData* StyleData,
		const FName SemanticId,
		const TSoftObjectPtr<UTexture2D>& PreferredTexture)
	{
		if (!PreferredTexture.IsNull())
		{
			return PreferredTexture.LoadSynchronous();
		}
		if (!StyleData)
		{
			return nullptr;
		}
		// [v1.0.0] 기존 D1-09B Semantic Icon Set에서 현재 의미 ID에 연결된 Soft Asset입니다.
		const TSoftObjectPtr<UObject> SemanticAsset = StyleData->ResolveSemanticIconAsset(SemanticId);
		return Cast<UTexture2D>(SemanticAsset.LoadSynchronous());
	}

	// [v1.0.0] 실제 Texture Brush를 사용하는 의미 Image Widget을 생성합니다.
	UImage* CreateImage(
		UWidgetTree* WidgetTree,
		const TCHAR* WidgetName,
		const UCFUIStyleData* StyleData,
		const FName SemanticId,
		const ECFUIColorToken ColorToken,
		const TSoftObjectPtr<UTexture2D>& PreferredTexture = TSoftObjectPtr<UTexture2D>())
	{
		// [v1.0.0] 현재 의미 시각 자산을 표시할 Image Widget입니다.
		UImage* Image = CreateWidget<UImage>(WidgetTree, WidgetName);
		if (!Image || !StyleData)
		{
			return nullptr;
		}
		if (UTexture2D* Texture = ResolveTexture(StyleData, SemanticId, PreferredTexture))
		{
			Image->SetBrushFromTexture(Texture, false);
		}
		Image->SetColorAndOpacity(StyleData->ResolveColor(ColorToken));
		Image->SetVisibility(ESlateVisibility::HitTestInvisible);
		return Image;
	}

	// [v1.0.0] 고정 크기 Spacer를 생성해 Box Layout 간격을 명시합니다.
	USpacer* CreateSpacer(UWidgetTree* WidgetTree, const TCHAR* WidgetName, const FVector2D& Size)
	{
		// [v1.0.0] Box Layout에서 의미 그룹 사이 간격을 확보할 Spacer입니다.
		USpacer* Spacer = CreateWidget<USpacer>(WidgetTree, WidgetName);
		if (Spacer)
		{
			Spacer->SetSize(Size);
		}
		return Spacer;
	}

		// [v1.0.0] 고정 Prototype 비율과 의미 색을 가진 ProgressBar를 생성합니다.
	UProgressBar* CreateProgress(UWidgetTree* WidgetTree, const TCHAR* WidgetName, const float Percent, const UCFUIStyleData* StyleData, const ECFUIColorToken ColorToken)
	{
		// [v1.0.0] 현재 정적 View Data Prototype 비율을 표시할 ProgressBar입니다.
		UProgressBar* ProgressBar = CreateWidget<UProgressBar>(WidgetTree, WidgetName);
		if (!ProgressBar || !StyleData)
		{
			return nullptr;
		}
		ProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		ProgressBar->SetFillColorAndOpacity(StyleData->ResolveColor(ColorToken));
		ProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		return ProgressBar;
	}

	// [v1.1.0] Armor용 ProgressBar를 아래에서 위로 채워지는 세로 비율 Bar로 생성합니다.
	UProgressBar* CreateVerticalProgress(UWidgetTree* WidgetTree, const TCHAR* WidgetName, const float Percent, const UCFUIStyleData* StyleData, const ECFUIColorToken ColorToken)
	{
		// [v1.1.0] 동일 CreateProgress 계약을 재사용하는 실제 세로 Armor Bar입니다.
		UProgressBar* ProgressBar = CreateProgress(WidgetTree, WidgetName, Percent, StyleData, ColorToken);
		if (ProgressBar)
		{
			ProgressBar->SetBarFillType(EProgressBarFillType::BottomToTop);
		}
		return ProgressBar;
	}

		// [v1.2.0] HUDVisualData 전용 Texture만 사용하는 Image를 만들고 Semantic Icon 대체나 빈 Brush 렌더링을 만들지 않습니다.
	UImage* CreatePreferredTextureImage(
		UWidgetTree* WidgetTree,
		const TCHAR* WidgetName,
		const UCFUIStyleData* StyleData,
		const ECFUIColorToken ColorToken,
		const TSoftObjectPtr<UTexture2D>& PreferredTexture)
	{
		// [v1.2.0] 실제 HUDVisualData Texture가 있을 때만 Brush를 채우는 Image입니다.
		UImage* Image = CreateWidget<UImage>(WidgetTree, WidgetName);
		if (!Image || !StyleData)
		{
			return nullptr;
		}

		// [v1.2.0] Soft Reference가 실제 Texture로 해석되어 빈 Brush 표시를 안전하게 허용할 수 있는지 나타냅니다.
		bool bHasResolvedTexture = false;
		if (!PreferredTexture.IsNull())
		{
			// [v1.2.0] Designer Preview에 연결할 실제 HUD 전용 Texture입니다.
			UTexture2D* Texture = PreferredTexture.LoadSynchronous();
			if (Texture)
			{
				Image->SetBrushFromTexture(Texture, false);
				bHasResolvedTexture = true;
			}
		}

		if (!bHasResolvedTexture)
		{
			// [v1.2.0] 전용 Texture가 없을 때 기본 Slate Brush가 Tint 사각형으로 보이지 않도록 슬롯만 보존하고 렌더링은 제거합니다.
			Image->SetVisibility(ESlateVisibility::Collapsed);
			return Image;
		}

		Image->SetColorAndOpacity(StyleData->ResolveColor(ColorToken));
		Image->SetVisibility(ESlateVisibility::HitTestInvisible);
		return Image;
	}

	// [v1.0.0] Production Panel의 유일한 Surface Border와 Content VerticalBox를 생성합니다.
	UVerticalBox* BuildPanelSurface(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData)
	{
		// [v1.0.0] Panel 배경 Surface를 담당하는 유일한 Border입니다.
		UBorder* Surface = CreateWidget<UBorder>(WidgetTree, TEXT("Border_Surface"));
		// [v1.0.0] Panel 의미 콘텐츠를 자동 배치할 VerticalBox입니다.
		UVerticalBox* Content = CreateWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_Content"));
		if (!Surface || !Content || !StyleData)
		{
			return nullptr;
		}
		Surface->SetBrushColor(StyleData->ResolveColor(ECFUIColorToken::SurfaceBase));
		Surface->SetPadding(FMargin(16.0f));
		Surface->SetVisibility(ESlateVisibility::HitTestInvisible);
		Surface->SetContent(Content);
		WidgetTree->RootWidget = Surface;
		return Content;
	}

	// [v1.0.0] HorizontalBox에 작은 Icon + Text 조합을 추가할 공통 Header를 만듭니다.
	UHorizontalBox* BuildHeader(
		UWidgetTree* WidgetTree,
		const TCHAR* HeaderName,
		const TCHAR* IconName,
		const FName SemanticId,
		const TCHAR* TextName,
		const TCHAR* DisplayText,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData,
		const ECFUIColorToken ColorToken)
	{
		// [v1.0.0] Header의 Icon/Text를 같은 행에 배치할 HorizontalBox입니다.
		UHorizontalBox* Header = CreateWidget<UHorizontalBox>(WidgetTree, HeaderName);
		if (!Header)
		{
			return nullptr;
		}
		Header->AddChild(CreateImage(WidgetTree, IconName, StyleData, SemanticId, ColorToken));
		Header->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_HeaderIconGap"), FVector2D(8.0f, 1.0f)));
		Header->AddChild(CreateText(WidgetTree, TextName, DisplayText, StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::HeadingM, ColorToken));
		return Header;
	}

			// [v1.1.0] SpeedGauge에서도 공간 배치 helper를 사용할 수 있도록 구현 앞에 선언합니다.
	bool AddCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FVector2D& Position, const FVector2D& Size, const int32 ZOrder);

	// [v1.1.0] SpeedGauge Element를 실제 RPM 의미의 정적 Track/Tick과 디지털 속도·단일 Gear Slot 구조로 생성합니다.
	bool BuildSpeedGauge(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData, const UCFHUDVisualData* HUDVisualData)
	{
		// [v1.1.0] 비대칭 RPM Tick과 속도/단위를 정확한 의미 위치에 배치할 Root Canvas입니다.
		UCanvasPanel* Root = CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Root"));
		// [v1.1.0] 전용 RPM Track Texture가 실제 연결됐을 때만 표시하고 다른 Semantic Icon을 대체 사용하지 않는 Image입니다.
		UImage* RPMTrackImage = CreatePreferredTextureImage(WidgetTree, TEXT("Image_RPMTrackArt"), StyleData, ECFUIColorToken::AccentTactical, HUDVisualData ? HUDVisualData->SpeedArcTrack : TSoftObjectPtr<UTexture2D>());
		if (!Root || !RPMTrackImage)
		{
			return false;
		}
		WidgetTree->RootWidget = Root;
		if (!AddCanvasChild(Root, RPMTrackImage, FVector2D(8.0f, 14.0f), FVector2D(220.0f, 154.0f), 0))
		{
			return false;
		}

		// [v1.1.0] 21 Tick이 좌측 세로→곡선→상단 수평으로 이어지는 비대칭 RPM 경로를 만들기 위한 위치/회전 설정입니다.
		struct FRPMTickSpec
		{
			// [v1.1.0] 현재 Tick의 SpeedGauge 내부 위치입니다.
			FVector2D Position;
			// [v1.1.0] 세로 구간의 수평 Tick에서 상단 구간의 수직 Tick으로 전환할 회전 각도입니다.
			float AngleDegrees;
		};

		// [v1.1.0] 0%~100%를 5% 간격으로 나타내는 정확한 21개 RPM Tick Preview 위치입니다.
		const FRPMTickSpec TickSpecs[] =
		{
			{FVector2D(18.0f, 158.0f), 0.0f}, {FVector2D(18.0f, 145.0f), 0.0f},
			{FVector2D(18.0f, 132.0f), 0.0f}, {FVector2D(18.0f, 119.0f), 0.0f},
			{FVector2D(18.0f, 106.0f), 0.0f}, {FVector2D(18.0f, 93.0f), 0.0f},
			{FVector2D(18.0f, 80.0f), 0.0f}, {FVector2D(19.0f, 68.0f), 0.0f},
			{FVector2D(22.0f, 56.0f), 15.0f}, {FVector2D(28.0f, 45.0f), 30.0f},
			{FVector2D(38.0f, 36.0f), 45.0f}, {FVector2D(50.0f, 29.0f), 60.0f},
			{FVector2D(64.0f, 25.0f), 75.0f}, {FVector2D(80.0f, 22.0f), 90.0f},
			{FVector2D(99.0f, 22.0f), 90.0f}, {FVector2D(118.0f, 22.0f), 90.0f},
			{FVector2D(137.0f, 22.0f), 90.0f}, {FVector2D(156.0f, 22.0f), 90.0f},
			{FVector2D(175.0f, 22.0f), 90.0f}, {FVector2D(194.0f, 22.0f), 90.0f},
			{FVector2D(213.0f, 22.0f), 90.0f}
		};
		for (int32 TickIndex = 0; TickIndex < UE_ARRAY_COUNT(TickSpecs); ++TickIndex)
		{
			// [v1.1.0] 10% 간격 Tick을 더 길게 표시할 Major Tick 여부입니다.
			const bool bMajorTick = (TickIndex % 2) == 0;
			// [v1.1.0] 85% Tick부터 Gauge 끝까지 고정 Red Zone으로 표시할 여부입니다.
			const bool bRedZoneTick = TickIndex >= 17;
			// [v1.1.0] 85% Red Zone 시작 Tick을 일반 Major보다 한 단계 더 길게 표시할 여부입니다.
			const bool bRedZoneStartTick = TickIndex == 17;
			// [v1.1.0] 현재 Tick의 고유 Widget 이름입니다.
			const FString TickName = FString::Printf(TEXT("ProgressBar_RPMTick%02d"), TickIndex);
			// [v1.1.0] Major/Minor/Red Zone 시작 의미에 따라 결정한 현재 Tick 길이입니다.
			const float TickLength = bRedZoneStartTick ? 15.0f : (bMajorTick ? 12.0f : 8.0f);
			// [v1.1.0] Major/Minor 의미에 따라 결정한 현재 Tick 두께입니다.
			const float TickThickness = bMajorTick || bRedZoneStartTick ? 3.0f : 2.0f;
			// [v1.1.0] Red Zone과 일반 Tick을 분리하는 현재 Tick 색상 Token입니다.
			const ECFUIColorToken TickColor = bRedZoneTick ? ECFUIColorToken::StateDanger : ECFUIColorToken::TextMuted;
			// [v1.1.0] 실제 RPM 값과 무관하게 고정 Scale 눈금만 표시하는 100% 채움 Tick입니다.
			UProgressBar* Tick = CreateProgress(WidgetTree, *TickName, 1.0f, StyleData, TickColor);
			if (!Tick)
			{
				return false;
			}
			Tick->SetRenderTransformAngle(TickSpecs[TickIndex].AngleDegrees);
			if (!AddCanvasChild(Root, Tick, TickSpecs[TickIndex].Position, FVector2D(TickLength, TickThickness), 1))
			{
				return false;
			}
		}

		// [v1.1.0] RPM Gauge 내부에서 가장 먼저 읽히는 3자리 정적 Designer Preview 속도 숫자입니다.
		UTextBlock* SpeedText = CreateText(WidgetTree, TEXT("Text_Speed"), TEXT("076"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::DisplayXL, ECFUIColorToken::TextPrimary);
		// [v1.1.0] 큰 속도 숫자 오른편에 붙는 작은 속도 단위입니다.
		UTextBlock* SpeedUnitText = CreateText(WidgetTree, TEXT("Text_SpeedUnit"), TEXT("km/h"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);
		// [v1.1.0] 실제 전진 기어 데이터가 없는 D1-11에서 가짜 숫자를 만들지 않도록 중립 상태를 사용하는 단일 Gear Slot Preview입니다.
		UTextBlock* GearText = CreateText(WidgetTree, TEXT("Text_Gear"), TEXT("N"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueM, ECFUIColorToken::AccentTactical);
		return AddCanvasChild(Root, SpeedText, FVector2D(44.0f, 88.0f), FVector2D(108.0f, 62.0f), 2)
			&& AddCanvasChild(Root, SpeedUnitText, FVector2D(151.0f, 115.0f), FVector2D(44.0f, 24.0f), 2)
			&& AddCanvasChild(Root, GearText, FVector2D(198.0f, 106.0f), FVector2D(30.0f, 38.0f), 2);
	}

		// [v1.1.0] Canvas Child를 SpeedGauge/Armor/Radar의 의미 공간 배치에만 사용하는 제한된 helper입니다.
	bool AddCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FVector2D& Position, const FVector2D& Size, const int32 ZOrder = 0)
	{
		if (!Canvas || !Child)
		{
			return false;
		}
		// [v1.0.0] 공간 의미 Widget의 실제 Canvas Slot입니다.
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
		if (!Slot)
		{
			return false;
		}
		Slot->SetPosition(Position);
		Slot->SetSize(Size);
		Slot->SetZOrder(ZOrder);
		Slot->SetAutoSize(false);
		return true;
	}

		// [v1.1.0] Armor Body Map을 Vehicle/6방향 Armor Image + 우측 세로 NormalizedArmor Bar 공간 구조로 생성합니다.
	bool BuildArmorBodyMap(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData, const UCFHUDVisualData* HUDVisualData)
	{
		(void)LayoutData;
		// [v1.1.0] 차체와 방향별 Armor를 공간적으로 겹쳐 배치할 허용된 Root Canvas입니다.
		UCanvasPanel* Root = CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Root"));
		if (!Root)
		{
			return false;
		}
		WidgetTree->RootWidget = Root;

		// [v1.1.0] HUDVisualData가 없을 때 동일 의미의 Semantic Vehicle Texture로 대체할 중앙 차량 실루엣 Image입니다.
		UImage* VehicleImage = CreateImage(WidgetTree, TEXT("Image_VehicleSilhouette"), StyleData, FName(TEXT("Vehicle")), ECFUIColorToken::TextSecondary, HUDVisualData ? HUDVisualData->VehicleSilhouette : TSoftObjectPtr<UTexture2D>());
		if (!AddCanvasChild(Root, VehicleImage, FVector2D(108.0f, 62.0f), FVector2D(148.0f, 72.0f), 1))
		{
			return false;
		}

		// [v1.1.0] 6방향 Armor의 Plate/Badge Image와 우측 세로 Bar를 함께 정의하는 Designer Preview 설정입니다.
		struct FArmorVisualSpec
		{
			// [v1.1.0] 현재 방향 Armor Plate/Badge Image Widget 이름입니다.
			const TCHAR* ImageName;
			// [v1.1.0] 현재 방향 NormalizedArmor 세로 ProgressBar Widget 이름입니다.
			const TCHAR* ProgressName;
			// [v1.1.0] D1-11 정적 Visual Prototype에서 방향별 남은 비율을 판독하기 위한 Mock NormalizedArmor입니다.
			float NormalizedArmor;
			// [v1.1.0] 현재 방향 Armor Plate/Badge Image 위치입니다.
			FVector2D ImagePosition;
			// [v1.1.0] Plate/Badge 바로 우측에 붙는 세로 Bar 위치입니다.
			FVector2D BarPosition;
			// [v1.1.0] 실제 HUDVisualData에 연결된 현재 방향 전용 Texture입니다.
			TSoftObjectPtr<UTexture2D> PreferredTexture;
		};

		// [v1.1.0] D1-07 승인 의미 위치 좌=Front·상=Right·우=Rear·하=Left·좌상단=Top·우하단=Bottom입니다.
		const FArmorVisualSpec ArmorSpecs[] =
		{
			{TEXT("Image_ArmorFront"), TEXT("ProgressBar_ArmorFront"), 0.30f, FVector2D(18.0f, 75.0f), FVector2D(68.0f, 75.0f), HUDVisualData ? HUDVisualData->ArmorPlates.FrontPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("Image_ArmorRight"), TEXT("ProgressBar_ArmorRight"), 0.70f, FVector2D(150.0f, 10.0f), FVector2D(200.0f, 10.0f), HUDVisualData ? HUDVisualData->ArmorPlates.RightPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("Image_ArmorRear"), TEXT("ProgressBar_ArmorRear"), 0.75f, FVector2D(300.0f, 75.0f), FVector2D(350.0f, 75.0f), HUDVisualData ? HUDVisualData->ArmorPlates.RearPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("Image_ArmorLeft"), TEXT("ProgressBar_ArmorLeft"), 0.63f, FVector2D(150.0f, 144.0f), FVector2D(200.0f, 144.0f), HUDVisualData ? HUDVisualData->ArmorPlates.LeftPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("Image_ArmorTop"), TEXT("ProgressBar_ArmorTop"), 0.84f, FVector2D(36.0f, 14.0f), FVector2D(86.0f, 14.0f), HUDVisualData ? HUDVisualData->ArmorPlates.TopPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("Image_ArmorBottom"), TEXT("ProgressBar_ArmorBottom"), 0.72f, FVector2D(286.0f, 144.0f), FVector2D(336.0f, 144.0f), HUDVisualData ? HUDVisualData->ArmorPlates.BottomPlate : TSoftObjectPtr<UTexture2D>()}
		};
		for (const FArmorVisualSpec& Spec : ArmorSpecs)
		{
			// [v1.1.0] 실제 방향별 Texture 또는 동일 의미 Semantic Armor Fallback을 표시할 Plate/Badge Image입니다.
			UImage* ArmorImage = CreateImage(WidgetTree, Spec.ImageName, StyleData, FName(TEXT("Armor")), ECFUIColorToken::Armor, Spec.PreferredTexture);
			// [v1.1.0] 숫자 없이 현재 방향의 Mock NormalizedArmor만 아래→위 Fill로 표현하는 세로 Bar입니다.
			UProgressBar* ArmorBar = CreateVerticalProgress(WidgetTree, Spec.ProgressName, Spec.NormalizedArmor, StyleData, ECFUIColorToken::Armor);
			if (!AddCanvasChild(Root, ArmorImage, Spec.ImagePosition, FVector2D(46.0f, 46.0f), 2)
				|| !AddCanvasChild(Root, ArmorBar, Spec.BarPosition, FVector2D(8.0f, 46.0f), 3))
			{
				return false;
			}
		}
		return true;
	}

	// [v1.0.0] Mission Panel을 자동 Box Layout과 실제 Semantic Image로 생성합니다.
	bool BuildMissionPanel(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		if (!Content)
		{
			return false;
		}
		Content->AddChild(BuildHeader(WidgetTree, TEXT("HorizontalBox_Header"), TEXT("Image_MissionIcon"), FName(TEXT("Target")), TEXT("Text_MissionLabel"), TEXT("현재 목표"), StyleData, LayoutData, ECFUIColorToken::AccentTactical));
		Content->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_MissionHeaderGap"), FVector2D(1.0f, 5.0f)));
		Content->AddChild(CreateText(WidgetTree, TEXT("Text_MissionTitle"), TEXT("호송 차량을 보호하십시오"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::HeadingL, ECFUIColorToken::TextPrimary));
		Content->AddChild(CreateText(WidgetTree, TEXT("Text_MissionBody"), TEXT("잔여 적대 차량 3"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextMuted));
		return true;
	}

	// [v1.0.0] Alert Feed를 두 의미 그룹과 실제 Warning/Reload Image로 생성합니다.
	bool BuildAlertFeed(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		// [v1.0.0] 두 Alert 의미 그룹을 가로로 배치할 실제 Content Row입니다.
		UHorizontalBox* Alerts = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Alerts"));
		if (!Content || !Alerts)
		{
			return false;
		}
		// [v1.0.0] 전방 장갑 경고 Icon/Text 그룹입니다.
		UVerticalBox* ArmorAlert = CreateWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_ArmorAlert"));
		// [v1.0.0] Ripple 진행 Icon/Text 그룹입니다.
		UVerticalBox* RippleAlert = CreateWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_RippleAlert"));
		ArmorAlert->AddChild(CreateImage(WidgetTree, TEXT("Image_AlertWarning"), StyleData, FName(TEXT("Warning")), ECFUIColorToken::StateCaution));
		ArmorAlert->AddChild(CreateText(WidgetTree, TEXT("Text_AlertPrimary"), TEXT("전방 장갑 주의"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::StateCaution));
		ArmorAlert->AddChild(CreateText(WidgetTree, TEXT("Text_AlertArmor"), TEXT("장갑 30%"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary));
		RippleAlert->AddChild(CreateImage(WidgetTree, TEXT("Image_AlertReload"), StyleData, FName(TEXT("Reload")), ECFUIColorToken::AccentTactical));
		RippleAlert->AddChild(CreateText(WidgetTree, TEXT("Text_AlertRipple"), TEXT("RIPPLE 진행"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body, ECFUIColorToken::TextPrimary));
		RippleAlert->AddChild(CreateText(WidgetTree, TEXT("Text_AlertSecondary"), TEXT("2 / 4"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary));
		Alerts->AddChild(ArmorAlert);
		Alerts->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_AlertGap"), FVector2D(28.0f, 1.0f)));
		Alerts->AddChild(RippleAlert);
		Content->AddChild(Alerts);
		return true;
	}

	// [v1.0.0] Target Panel을 실제 Target Image, 정보 Row와 Scan ProgressBar로 생성합니다.
	bool BuildTargetPanel(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		if (!Content)
		{
			return false;
		}
		Content->AddChild(BuildHeader(WidgetTree, TEXT("HorizontalBox_Header"), TEXT("Image_TargetIcon"), FName(TEXT("Target")), TEXT("Text_TargetTitle"), TEXT("적대 차량"), StyleData, LayoutData, ECFUIColorToken::Hostile));
		Content->AddChild(CreateText(WidgetTree, TEXT("Text_TargetDistance"), TEXT("거리  842 m"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueM, ECFUIColorToken::TextPrimary));
		Content->AddChild(CreateText(WidgetTree, TEXT("Text_TargetIdentity"), TEXT("식별  ???"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::Unknown));
		Content->AddChild(CreateText(WidgetTree, TEXT("Text_TargetArmor"), TEXT("장갑  ???"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::Unknown));
		Content->AddChild(CreateText(WidgetTree, TEXT("Text_TargetScan"), TEXT("스캔  35%"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextPrimary));
		Content->AddChild(CreateProgress(WidgetTree, TEXT("ProgressBar_TargetScan"), 0.35f, StyleData, ECFUIColorToken::AccentTactical));
		return true;
	}

	// [v1.0.0] Generated Class를 실제 중첩 UserWidget Template으로 생성합니다.
	UUserWidget* CreateNestedWidget(UWidgetTree* WidgetTree, UClass* WidgetClass, const TCHAR* WidgetName)
	{
		return WidgetTree && WidgetClass && WidgetClass->IsChildOf(UUserWidget::StaticClass())
			? WidgetTree->ConstructWidget<UUserWidget>(WidgetClass, FName(WidgetName))
			: nullptr;
	}

	// [v1.0.0] Vehicle Panel을 SpeedGauge + ArmorBodyMap 의미 Element와 Shield/Integrity 상태 Row로 조립합니다.
	bool BuildVehiclePanel(
		UWidgetTree* WidgetTree,
		const UCFUIStyleData* StyleData,
		const UCFHUDLayoutData* LayoutData,
		UClass* SpeedGaugeClass,
		UClass* ArmorBodyMapClass)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		// [v1.0.0] SpeedGauge와 ArmorBodyMap을 한 행에 배치할 주요 Vehicle Row입니다.
		UHorizontalBox* MainRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Main"));
		// [v1.0.0] SpeedGauge 실제 Production Element 인스턴스입니다.
		UUserWidget* SpeedGauge = CreateNestedWidget(WidgetTree, SpeedGaugeClass, TEXT("WBP_CFSpeedGauge"));
		// [v1.0.0] ArmorBodyMap 실제 Production Element 인스턴스입니다.
		UUserWidget* ArmorBodyMap = CreateNestedWidget(WidgetTree, ArmorBodyMapClass, TEXT("WBP_CFArmorBodyMap"));
		if (!Content || !MainRow || !SpeedGauge || !ArmorBodyMap)
		{
			return false;
		}
		// [v1.0.0] 좌측 SpeedGauge의 의도된 Production 폭을 고정할 SizeBox입니다.
		USizeBox* SpeedBox = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_SpeedGauge"));
		SpeedBox->SetWidthOverride(238.0f);
		SpeedBox->SetHeightOverride(190.0f);
		SpeedBox->SetContent(SpeedGauge);
		// [v1.0.0] 우측 ArmorBodyMap의 의도된 Production 폭을 고정할 SizeBox입니다.
		USizeBox* ArmorBox = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_ArmorBodyMap"));
		ArmorBox->SetWidthOverride(382.0f);
		ArmorBox->SetHeightOverride(190.0f);
		ArmorBox->SetContent(ArmorBodyMap);
		MainRow->AddChild(SpeedBox);
		MainRow->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_VehicleMainGap"), FVector2D(14.0f, 1.0f)));
		MainRow->AddChild(ArmorBox);
		Content->AddChild(MainRow);
		Content->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_VehicleStateGap"), FVector2D(1.0f, 8.0f)));

		// [v1.0.0] Shield Icon/Label/Value를 한 행에 표시할 상태 Row입니다.
		UHorizontalBox* ShieldRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Shield"));
		ShieldRow->AddChild(CreateImage(WidgetTree, TEXT("Image_Shield"), StyleData, FName(TEXT("Shield")), ECFUIColorToken::Shield));
		ShieldRow->AddChild(CreateText(WidgetTree, TEXT("Text_ShieldLabel"), TEXT("SHIELD"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary));
		ShieldRow->AddChild(CreateText(WidgetTree, TEXT("Text_ShieldValue"), TEXT("78/100"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::Shield));
		Content->AddChild(ShieldRow);
		Content->AddChild(CreateProgress(WidgetTree, TEXT("ProgressBar_Shield"), 0.78f, StyleData, ECFUIColorToken::Shield));

		// [v1.0.0] Integrity Icon/Label/Value를 한 행에 표시할 상태 Row입니다.
		UHorizontalBox* IntegrityRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Integrity"));
		IntegrityRow->AddChild(CreateImage(WidgetTree, TEXT("Image_Integrity"), StyleData, FName(TEXT("Integrity")), ECFUIColorToken::Integrity));
		IntegrityRow->AddChild(CreateText(WidgetTree, TEXT("Text_IntegrityLabel"), TEXT("INTEGRITY"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Label, ECFUIColorToken::TextSecondary));
		IntegrityRow->AddChild(CreateText(WidgetTree, TEXT("Text_IntegrityValue"), TEXT("82/100"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::Integrity));
		Content->AddChild(IntegrityRow);
		Content->AddChild(CreateProgress(WidgetTree, TEXT("ProgressBar_Integrity"), 0.82f, StyleData, ECFUIColorToken::Integrity));
		return true;
	}

	// [v1.0.0] Radar Panel을 Semantic RadarContact Image의 공간 Field로 생성합니다.
	bool BuildRadarPanel(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData, const UCFHUDVisualData* HUDVisualData)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		if (!Content)
		{
			return false;
		}
		Content->AddChild(BuildHeader(WidgetTree, TEXT("HorizontalBox_Header"), TEXT("Image_RadarHeader"), FName(TEXT("RadarContact")), TEXT("Text_RadarTitle"), TEXT("RADAR"), StyleData, LayoutData, ECFUIColorToken::AccentTactical));
		// [v1.0.0] Radar Contact의 2D 공간 관계만 담당하는 제한된 Canvas Field입니다.
		UCanvasPanel* RadarField = CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_RadarContacts"));
		// [v1.0.0] Radar Field의 고정 Preview 크기를 제공할 SizeBox입니다.
		USizeBox* RadarBox = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_RadarField"));
		if (!RadarField || !RadarBox)
		{
			return false;
		}
		RadarBox->SetHeightOverride(166.0f);
		RadarBox->SetContent(RadarField);
		Content->AddChild(RadarBox);

		// [v1.0.0] Friendly Contact Image입니다.
		UImage* Friendly = CreateImage(WidgetTree, TEXT("Image_RadarFriendly"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Friendly);
		// [v1.0.0] Neutral Contact Image입니다.
		UImage* Neutral = CreateImage(WidgetTree, TEXT("Image_RadarNeutral"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Neutral);
		// [v1.0.0] Hostile Contact Image입니다.
		UImage* Hostile = CreateImage(WidgetTree, TEXT("Image_RadarHostile"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Hostile);
		// [v1.0.0] Unknown Contact Image입니다.
		UImage* Unknown = CreateImage(WidgetTree, TEXT("Image_RadarUnknown"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Unknown);
		// [v1.0.0] 선택 대상 Contact를 더 크게 표시할 Target Image입니다.
		UImage* Selected = CreateImage(WidgetTree, TEXT("Image_RadarSelected"), StyleData, FName(TEXT("Target")), ECFUIColorToken::AccentTactical, HUDVisualData ? HUDVisualData->SelectedTargetBracket : TSoftObjectPtr<UTexture2D>());
		return AddCanvasChild(RadarField, Friendly, FVector2D(54.0f, 86.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Neutral, FVector2D(174.0f, 48.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Hostile, FVector2D(228.0f, 96.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Unknown, FVector2D(112.0f, 122.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Selected, FVector2D(214.0f, 82.0f), FVector2D(48.0f, 48.0f), 2);
	}

		// [v1.5.0] Weapon Panel을 Loaded/Capacity, 우측 Reserve, Heat/Cooldown/Reload 의미 Image와 Compact Rail로 생성합니다.
	bool BuildWeaponPanel(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		if (!Content)
		{
			return false;
		}
						// [v1.4.0] 기존 검증된 Header 제작 경로를 재사용해 무기 아이콘·제목의 Style 계약을 보존합니다.
		UHorizontalBox* WeaponHeader = BuildHeader(
			WidgetTree,
			TEXT("HorizontalBox_Header"),
			TEXT("Image_WeaponIcon"),
			FName(TEXT("Turret")),
			TEXT("Text_WeaponTitle"),
			TEXT("HEAVY CANNON"),
			StyleData,
			LayoutData,
			ECFUIColorToken::AccentTactical);
		if (!WeaponHeader)
		{
			return false;
		}

						// [v1.5.1] Header가 아직 Root Tree에 붙기 전에도 안전하게 가져올 수 있는 BuildHeader의 세 번째 직접 자식인 무기 제목 Widget입니다.
		UTextBlock* WeaponTitleText = WeaponHeader->GetChildrenCount() > 2
			? Cast<UTextBlock>(WeaponHeader->GetChildAt(2))
			: nullptr;

		// [v1.5.0] 제목을 Fill 규칙으로 바꿔 고정 픽셀 Spacer 없이 Header 폭을 안전하게 분배할 Slot입니다.
		UHorizontalBoxSlot* WeaponTitleSlot = WeaponTitleText ? Cast<UHorizontalBoxSlot>(WeaponTitleText->Slot) : nullptr;
		if (!WeaponTitleSlot)
		{
			return false;
		}

		// [v1.5.0] 현재 무기 제목이 Reserve 앞까지 남는 가로 공간을 모두 사용하도록 하는 Fill 크기 규칙입니다.
		FSlateChildSize WeaponTitleSize;
		WeaponTitleSize.SizeRule = ESlateSizeRule::Fill;
		WeaponTitleSlot->SetSize(WeaponTitleSize);
		WeaponTitleSlot->SetVerticalAlignment(VAlign_Center);

		// [v1.5.0] 실제 Reserve가 없을 때도 0을 표시할 수 있도록 기본 Preview도 0으로 생성하는 숫자 Text입니다.
		UTextBlock* ReserveAmmoText = CreateText(
			WidgetTree,
			TEXT("Text_WeaponReserveAmmo"),
			TEXT("0"),
			StyleData,
			LayoutData,
			ECFUIFontFamilyRole::Numeric,
			ECFUITypographyRole::ValueS,
			ECFUIColorToken::TextSecondary);

		// [v1.5.0] Reserve 숫자를 Header의 실제 마지막 자식으로 두고 우측 정렬하는 Slot입니다.
		UHorizontalBoxSlot* ReserveAmmoSlot = ReserveAmmoText ? WeaponHeader->AddChildToHorizontalBox(ReserveAmmoText) : nullptr;
		if (!ReserveAmmoSlot)
		{
			return false;
		}
		ReserveAmmoSlot->SetHorizontalAlignment(HAlign_Right);
		ReserveAmmoSlot->SetVerticalAlignment(VAlign_Center);
		Content->AddChild(WeaponHeader);

		// [v1.3.0] 정상 Ripple/Salvo 진행을 WeaponPanel의 Primary Action으로 표시할 전용 Row입니다.
		UHorizontalBox* LauncherSequenceRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_LauncherSequence"));
		LauncherSequenceRow->AddChild(CreateText(WidgetTree, TEXT("Text_WeaponLauncherSequence"), TEXT("RIPPLE 2 / 4"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueM, ECFUIColorToken::AccentTactical));
		Content->AddChild(LauncherSequenceRow);
		Content->AddChild(CreateProgress(WidgetTree, TEXT("ProgressBar_LauncherSequence"), 0.5f, StyleData, ECFUIColorToken::AccentTactical));

		// [v1.0.0] Ammo 상태 Row입니다.
		UHorizontalBox* AmmoRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Ammo"));
		AmmoRow->AddChild(CreateImage(WidgetTree, TEXT("Image_Ammo"), StyleData, FName(TEXT("Ammo")), ECFUIColorToken::TextPrimary));
				AmmoRow->AddChild(CreateText(WidgetTree, TEXT("Text_WeaponAmmo"), TEXT("5 / 10"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueM, ECFUIColorToken::TextPrimary));
		Content->AddChild(AmmoRow);

		// [v1.0.0] Heat 상태 Row입니다.
		UHorizontalBox* HeatRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Heat"));
		HeatRow->AddChild(CreateImage(WidgetTree, TEXT("Image_Heat"), StyleData, FName(TEXT("Heat")), ECFUIColorToken::StateCaution));
		HeatRow->AddChild(CreateText(WidgetTree, TEXT("Text_WeaponHeat"), TEXT("38%"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::StateCaution));
		Content->AddChild(HeatRow);
		Content->AddChild(CreateProgress(WidgetTree, TEXT("ProgressBar_Heat"), 0.38f, StyleData, ECFUIColorToken::StateCaution));

		// [v1.0.0] Cooldown 상태 Row입니다.
		UHorizontalBox* CooldownRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Cooldown"));
		CooldownRow->AddChild(CreateImage(WidgetTree, TEXT("Image_Cooldown"), StyleData, FName(TEXT("Cooldown")), ECFUIColorToken::AccentTactical));
		CooldownRow->AddChild(CreateText(WidgetTree, TEXT("Text_WeaponCooldown"), TEXT("0.8 s"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextSecondary));
		Content->AddChild(CooldownRow);
		Content->AddChild(CreateProgress(WidgetTree, TEXT("ProgressBar_Cooldown"), 0.42f, StyleData, ECFUIColorToken::AccentTactical));

		// [v1.0.0] 선택/비선택 Weapon Compact Rail을 Image 중심으로 표시할 Row입니다.
		UHorizontalBox* Rail = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_WeaponRail"));
		Rail->AddChild(CreateImage(WidgetTree, TEXT("Image_WeaponRail1"), StyleData, FName(TEXT("Turret")), ECFUIColorToken::AccentTactical));
		Rail->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_WeaponRailGap1"), FVector2D(16.0f, 1.0f)));
		Rail->AddChild(CreateImage(WidgetTree, TEXT("Image_WeaponRail2"), StyleData, FName(TEXT("Ammo")), ECFUIColorToken::TextMuted));
		Rail->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_WeaponRailGap2"), FVector2D(16.0f, 1.0f)));
		Rail->AddChild(CreateImage(WidgetTree, TEXT("Image_WeaponRail3"), StyleData, FName(TEXT("Reload")), ECFUIColorToken::TextMuted));
		Content->AddChild(Rail);
		return true;
	}

	// [v1.0.0] 역할 이름에 따라 정확한 Production Child Tree를 생성합니다.
	bool BuildRoleTree(
		UWidgetTree* WidgetTree,
		const FName WidgetRole,
		const UCFHUDLayoutData* LayoutData,
		const UCFUIStyleData* StyleData,
		const UCFHUDVisualData* HUDVisualData,
		UClass* SpeedGaugeClass,
		UClass* ArmorBodyMapClass,
		FString& OutFailureReason)
	{
		if (WidgetRole == FName(TEXT("SpeedGauge"))) return BuildSpeedGauge(WidgetTree, StyleData, LayoutData, HUDVisualData);
		if (WidgetRole == FName(TEXT("ArmorBodyMap"))) return BuildArmorBodyMap(WidgetTree, StyleData, LayoutData, HUDVisualData);
		if (WidgetRole == FName(TEXT("MissionPanel"))) return BuildMissionPanel(WidgetTree, StyleData, LayoutData);
		if (WidgetRole == FName(TEXT("AlertFeed"))) return BuildAlertFeed(WidgetTree, StyleData, LayoutData);
		if (WidgetRole == FName(TEXT("TargetPanel"))) return BuildTargetPanel(WidgetTree, StyleData, LayoutData);
		if (WidgetRole == FName(TEXT("VehiclePanel")))
		{
			if (!SpeedGaugeClass || !ArmorBodyMapClass)
			{
				OutFailureReason = TEXT("VehiclePanel requires compiled SpeedGauge and ArmorBodyMap classes");
				return false;
			}
			return BuildVehiclePanel(WidgetTree, StyleData, LayoutData, SpeedGaugeClass, ArmorBodyMapClass);
		}
		if (WidgetRole == FName(TEXT("RadarPanel"))) return BuildRadarPanel(WidgetTree, StyleData, LayoutData, HUDVisualData);
		if (WidgetRole == FName(TEXT("WeaponPanel"))) return BuildWeaponPanel(WidgetTree, StyleData, LayoutData);
		OutFailureReason = FString::Printf(TEXT("Unsupported Production HUD widget role: %s"), *WidgetRole.ToString());
		return false;
	}

	// [v1.0.0] UE 기본 미연결 Event Node만 허용하고 실제 Runtime Logic/Binding이 없는지 검증합니다.
	bool ValidateGraphHasNoRuntimeLogic(const UWidgetBlueprint* WidgetBlueprint, FString& OutFailureReason)
	{
		if (!WidgetBlueprint)
		{
			OutFailureReason = TEXT("WidgetBlueprint is null during graph validation");
			return false;
		}
		// [v1.0.0] Event/Function/Macro 전체를 검사할 Graph 목록입니다.
		TArray<UEdGraph*> Graphs;
		Graphs.Append(WidgetBlueprint->UbergraphPages);
		Graphs.Append(WidgetBlueprint->FunctionGraphs);
		Graphs.Append(WidgetBlueprint->MacroGraphs);
		for (const UEdGraph* Graph : Graphs)
		{
			if (!Graph) continue;
			for (const UEdGraphNode* Node : Graph->Nodes)
			{
				if (!Node) continue;
				// [v1.0.0] 신규 Widget Blueprint가 자동 생성하는 미연결 Event Node 외에는 허용하지 않습니다.
				const FString NodeClassName = Node->GetClass()->GetName();
				if (NodeClassName != TEXT("K2Node_Event"))
				{
					OutFailureReason = FString::Printf(TEXT("Production HUD graph node is forbidden: %s"), *NodeClassName);
					return false;
				}
				for (const UEdGraphPin* Pin : Node->Pins)
				{
					if (Pin && Pin->LinkedTo.Num() > 0)
					{
						OutFailureReason = TEXT("Production HUD Runtime Event binding is forbidden in D1-11");
						return false;
					}
				}
			}
		}
		return true;
	}

	// [v1.0.0] Production Child Tree가 Border Mosaic 없이 Image 중심 구조를 유지하는지 검증합니다.
	bool ValidateRoleTree(UWidgetTree* WidgetTree, const FName WidgetRole, UClass* SpeedGaugeClass, UClass* ArmorBodyMapClass, FString& OutFailureReason)
	{
		if (!WidgetTree || !WidgetTree->RootWidget)
		{
			OutFailureReason = TEXT("Production WidgetTree/root is missing");
			return false;
		}
		// [v1.0.0] Production Tree의 Native Widget 구성과 금지 이름을 검사할 전체 Widget 목록입니다.
		TArray<UWidget*> AllWidgets;
		WidgetTree->GetAllWidgets(AllWidgets);
		// [v1.0.0] 작은 Border 조각 회귀를 검출할 실제 Border 개수입니다.
		int32 BorderCount = 0;
		// [v1.0.0] 실제 시각 자산 슬롯을 보장할 Image 개수입니다.
		int32 ImageCount = 0;
		for (const UWidget* Widget : AllWidgets)
		{
			if (!Widget) continue;
			const FString WidgetName = Widget->GetName();
			if (WidgetName.Contains(TEXT("Mock")) || WidgetName.Contains(TEXT("Mosaic")))
			{
				OutFailureReason = FString::Printf(TEXT("Legacy mock/mosaic residue is forbidden: %s"), *WidgetName);
				return false;
			}
			if (Widget->IsA<UBorder>()) ++BorderCount;
			if (Widget->IsA<UImage>()) ++ImageCount;
		}

		const bool bPanelRole = WidgetRole != FName(TEXT("SpeedGauge")) && WidgetRole != FName(TEXT("ArmorBodyMap"));
		if (bPanelRole && BorderCount != 1)
		{
			OutFailureReason = FString::Printf(TEXT("Production Panel must use exactly one surface Border: role=%s count=%d"), *WidgetRole.ToString(), BorderCount);
			return false;
		}
		if (!bPanelRole && BorderCount != 0)
		{
			OutFailureReason = FString::Printf(TEXT("Production Element must not use Border drawing: role=%s count=%d"), *WidgetRole.ToString(), BorderCount);
			return false;
		}

				// [v1.1.0] 역할별 최소 실제 Image 슬롯 개수입니다. SpeedGauge는 전용 Track 1개 + ProgressBar Tick 21개 구조를 사용합니다.
		int32 RequiredImageCount = 1;
		if (WidgetRole == FName(TEXT("AlertFeed"))) RequiredImageCount = 2;
		else if (WidgetRole == FName(TEXT("SpeedGauge"))) RequiredImageCount = 1;
		else if (WidgetRole == FName(TEXT("ArmorBodyMap"))) RequiredImageCount = 7;
		else if (WidgetRole == FName(TEXT("VehiclePanel"))) RequiredImageCount = 2;
		else if (WidgetRole == FName(TEXT("RadarPanel"))) RequiredImageCount = 6;
		else if (WidgetRole == FName(TEXT("WeaponPanel"))) RequiredImageCount = 7;
				if (ImageCount < RequiredImageCount)
		{
			OutFailureReason = FString::Printf(TEXT("Production Image slot contract failed: role=%s actual=%d required=%d"), *WidgetRole.ToString(), ImageCount, RequiredImageCount);
			return false;
		}

				if (WidgetRole == FName(TEXT("WeaponPanel")))
		{
			// [v1.3.0] 정상 Launcher Sequence를 WeaponPanel에 표시하기 위한 의미 슬롯 세 개가 모두 존재해야 합니다.
			const bool bHasLauncherSequenceSlots = WidgetTree->FindWidget(FName(TEXT("HorizontalBox_LauncherSequence")))
				&& WidgetTree->FindWidget(FName(TEXT("Text_WeaponLauncherSequence")))
				&& WidgetTree->FindWidget(FName(TEXT("ProgressBar_LauncherSequence")));
			if (!bHasLauncherSequenceSlots)
			{
				OutFailureReason = TEXT("WeaponPanel Launcher Sequence semantic slots are missing");
				return false;
			}

			// [v1.4.0] Primary Loaded/Capacity와 Header Reserve 숫자를 서로 다른 의미 슬롯으로 유지합니다.
			const bool bHasAmmoDisplaySlots = WidgetTree->FindWidget(FName(TEXT("HorizontalBox_Ammo")))
				&& WidgetTree->FindWidget(FName(TEXT("Text_WeaponAmmo")))
				&& WidgetTree->FindWidget(FName(TEXT("Text_WeaponReserveAmmo")));
						if (!bHasAmmoDisplaySlots
				|| WidgetTree->FindWidget(FName(TEXT("Text_WeaponReserveLabel")))
				|| WidgetTree->FindWidget(FName(TEXT("Spacer_WeaponReserveGap"))))
			{
				OutFailureReason = TEXT("WeaponPanel Loaded/Capacity or label-less Reserve semantic/right-edge slot contract failed");
				return false;
			}
		}

				if (WidgetRole == FName(TEXT("SpeedGauge")))
		{
			// [v1.1.0] SpeedGauge에서 실제 표시해야 하는 단일 Gear Slot Text입니다.
			const UTextBlock* GearText = Cast<UTextBlock>(WidgetTree->FindWidget(FName(TEXT("Text_Gear"))));
			// [v1.2.0] 미래 전용 RPM Track Texture 교체 지점을 유지하는 Image 슬롯입니다.
			const UImage* RPMTrackImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RPMTrackArt"))));
			// [v1.2.0] RPM Track Image에 실제 Texture/Resource가 연결되어 있는지 나타냅니다.
			const bool bRPMTrackHasResource = RPMTrackImage && RPMTrackImage->GetBrush().GetResourceObject() != nullptr;
			// [v1.2.0] Resource 유무와 Image Visibility가 빈 Brush 렌더링 방지 계약에 맞는지 나타냅니다.
			const bool bRPMTrackVisibilityValid = RPMTrackImage
				&& (bRPMTrackHasResource
					? RPMTrackImage->GetVisibility() != ESlateVisibility::Collapsed
					: RPMTrackImage->GetVisibility() == ESlateVisibility::Collapsed);
			// [v1.1.0] 고정 21 Tick 계약이 실제 Tree에 존재하는지 세는 Tick 개수입니다.
			int32 RPMTickCount = 0;
			for (const UWidget* Widget : AllWidgets)
			{
				if (Widget && Widget->GetName().StartsWith(TEXT("ProgressBar_RPMTick")))
				{
					++RPMTickCount;
				}
			}
			if (!GearText || GearText->GetText().ToString() == TEXT("D")
				|| WidgetTree->FindWidget(FName(TEXT("ProgressBar_SpeedFallback")))
				|| WidgetTree->FindWidget(FName(TEXT("Image_SpeedArcArt")))
				|| !RPMTrackImage
				|| !bRPMTrackVisibilityValid
				|| RPMTickCount != 21)
			{
				OutFailureReason = FString::Printf(TEXT("SpeedGauge latest vehicle-panel contract failed: gear=%s rpm_ticks=%d rpm_track_resource=%s rpm_track_visibility_valid=%s"),
					GearText ? *GearText->GetText().ToString() : TEXT("missing"),
					RPMTickCount,
					bRPMTrackHasResource ? TEXT("true") : TEXT("false"),
					bRPMTrackVisibilityValid ? TEXT("true") : TEXT("false"));
				return false;
			}
		}

		if (WidgetRole == FName(TEXT("ArmorBodyMap")))
		{
			// [v1.1.0] 여섯 방향 Armor Bar가 모두 존재하는지 검증할 안정적인 Widget 이름 목록입니다.
			const FName RequiredArmorBars[] =
			{
				FName(TEXT("ProgressBar_ArmorFront")), FName(TEXT("ProgressBar_ArmorRight")),
				FName(TEXT("ProgressBar_ArmorRear")), FName(TEXT("ProgressBar_ArmorLeft")),
				FName(TEXT("ProgressBar_ArmorTop")), FName(TEXT("ProgressBar_ArmorBottom"))
			};
			for (const FName ArmorBarName : RequiredArmorBars)
			{
				if (!Cast<UProgressBar>(WidgetTree->FindWidget(ArmorBarName)))
				{
					OutFailureReason = FString::Printf(TEXT("ArmorBodyMap vertical bar is missing: %s"), *ArmorBarName.ToString());
					return false;
				}
			}
			for (const UWidget* Widget : AllWidgets)
			{
				if (Widget && Widget->GetName().StartsWith(TEXT("Text_Armor")))
				{
					OutFailureReason = FString::Printf(TEXT("ArmorBodyMap numeric/direction text is forbidden: %s"), *Widget->GetName());
					return false;
				}
			}
		}

		if (WidgetRole == FName(TEXT("VehiclePanel")))
		{
			const UUserWidget* SpeedGauge = Cast<UUserWidget>(WidgetTree->FindWidget(FName(TEXT("WBP_CFSpeedGauge"))));
			const UUserWidget* ArmorBodyMap = Cast<UUserWidget>(WidgetTree->FindWidget(FName(TEXT("WBP_CFArmorBodyMap"))));
			if (!SpeedGauge || !ArmorBodyMap || SpeedGauge->GetClass() != SpeedGaugeClass || ArmorBodyMap->GetClass() != ArmorBodyMapClass)
			{
				OutFailureReason = TEXT("VehiclePanel semantic child widget class contract failed");
				return false;
			}
		}
		return true;
	}

	// [v1.0.0] Root Canvas Child Slot에 D1-07 DataAsset Anchor·Offset·Size·Alignment·ZOrder를 적용합니다.
	bool ApplyRootCanvasLayout(UCanvasPanel* RootCanvas, UWidget* ChildWidget, const FCFHUDSlotLayout& SlotLayout)
	{
		if (!RootCanvas || !ChildWidget)
		{
			return false;
		}
		// [v1.0.0] 현재 Root Child가 사용할 실제 Canvas Panel Slot입니다.
		UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(ChildWidget);
		if (!CanvasSlot)
		{
			return false;
		}
		CanvasSlot->SetAnchors(FAnchors(SlotLayout.AnchorMinimum.X, SlotLayout.AnchorMinimum.Y, SlotLayout.AnchorMaximum.X, SlotLayout.AnchorMaximum.Y));
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

	// [v1.0.0] Root Slot 하나의 SizeBox와 실제 Production Panel Widget을 생성합니다.
	bool AddRootPanelSlot(
		UWidgetTree* WidgetTree,
		UCanvasPanel* RootCanvas,
		const FCFHUDSlotLayout& SlotLayout,
		const TCHAR* SizeBoxName,
		const TCHAR* PanelWidgetName,
		UClass* PanelClass)
	{
		// [v1.0.0] D1-07 DesiredSize를 명시적으로 소유할 Root Slot SizeBox입니다.
		USizeBox* SlotBox = CreateWidget<USizeBox>(WidgetTree, SizeBoxName);
		// [v1.0.0] 실제 의미 Panel Generated Class의 Root 인스턴스입니다.
		UUserWidget* PanelWidget = CreateNestedWidget(WidgetTree, PanelClass, PanelWidgetName);
		if (!SlotBox || !PanelWidget)
		{
			return false;
		}
		SlotBox->SetWidthOverride(SlotLayout.DesiredSize.X);
		SlotBox->SetHeightOverride(SlotLayout.DesiredSize.Y);
		SlotBox->SetContent(PanelWidget);
		return ApplyRootCanvasLayout(RootCanvas, SlotBox, SlotLayout);
	}

	// [v1.0.0] Root Layout의 실제 Canvas Slot이 승인 Data와 같은지 검증합니다.
	bool ValidateRootCanvasLayout(const UWidget* Widget, const FCFHUDSlotLayout& Layout, FString& OutFailureReason)
	{
		const UCanvasPanelSlot* Slot = Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
		if (!Slot)
		{
			OutFailureReason = FString::Printf(TEXT("Root Canvas slot missing: %s"), Widget ? *Widget->GetName() : TEXT("null"));
			return false;
		}
		if (!FMath::IsNearlyEqual(Slot->GetAlignment().X, Layout.Alignment.X, 0.01f)
			|| !FMath::IsNearlyEqual(Slot->GetAlignment().Y, Layout.Alignment.Y, 0.01f)
			|| Slot->GetZOrder() != Layout.ZOrder)
		{
			OutFailureReason = FString::Printf(TEXT("Root Canvas alignment/z-order mismatch: %s"), *Widget->GetName());
			return false;
		}
		if (Layout.SlotId != ECFHUDSlotId::ReticleLayer)
		{
			if (!FMath::IsNearlyEqual(Slot->GetPosition().X, Layout.PixelOffset.X, 0.01f)
				|| !FMath::IsNearlyEqual(Slot->GetPosition().Y, Layout.PixelOffset.Y, 0.01f)
				|| !FMath::IsNearlyEqual(Slot->GetSize().X, Layout.DesiredSize.X, 0.01f)
				|| !FMath::IsNearlyEqual(Slot->GetSize().Y, Layout.DesiredSize.Y, 0.01f))
			{
				OutFailureReason = FString::Printf(TEXT("Root Canvas position/size mismatch: %s"), *Widget->GetName());
				return false;
			}
		}
		return true;
	}
#endif
}

// [v1.0.0] 지정 Production Widget 역할의 Designer Tree를 의미 단위 UMG 구조로 생성합니다.
bool UCFUIHUDProdEditorBridge::BuildProductionWidgetResult(
	UObject* WidgetBlueprintObject,
	FName WidgetRole,
	UCFHUDLayoutData* LayoutData,
	UCFUIStyleData* StyleData,
	UCFUIDensityData* StandardDensityData,
	UCFUIDensityData* CompactDensityData,
	UCFHUDVisualData* HUDVisualData,
	UObject* SpeedGaugeBlueprintObject,
	UObject* ArmorBodyMapBlueprintObject)
{
#if WITH_EDITOR
	// [v1.0.0] Production Build 선행 조건과 Class 해석의 상세 실패 사유입니다.
	FString FailureReason;
	if (!CFUIHUDProdEditorBridge::ValidateInputData(LayoutData, StyleData, StandardDensityData, CompactDensityData, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
	// [v1.0.0] Python에서 전달된 실제 Production Child Widget Blueprint입니다.
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	if (!WidgetBlueprint || WidgetBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass())
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production child Widget Blueprint must inherit exactly CFStyledWidgetBase"));
	}

	UClass* SpeedGaugeClass = nullptr;
	UClass* ArmorBodyMapClass = nullptr;
	if (WidgetRole == FName(TEXT("VehiclePanel")))
	{
		SpeedGaugeClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(SpeedGaugeBlueprintObject, TEXT("SpeedGauge"), FailureReason);
		ArmorBodyMapClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(ArmorBodyMapBlueprintObject, TEXT("ArmorBodyMap"), FailureReason);
		if (!SpeedGaugeClass || !ArmorBodyMapClass)
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason);
		}
	}

	UWidgetTree* WidgetTree = CFUIHUDProdEditorBridge::ReplaceWidgetTree(WidgetBlueprint, FailureReason);
	if (!WidgetTree || !CFUIHUDProdEditorBridge::BuildRoleTree(WidgetTree, WidgetRole, LayoutData, StyleData, HUDVisualData, SpeedGaugeClass, ArmorBodyMapClass, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason.IsEmpty() ? TEXT("Production role tree construction failed") : FailureReason);
	}
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	return true;
#else
	return false;
#endif
}

// [v1.0.0] 저장된 Production Widget 역할의 Tree·Image·Border 제한·Graph 0 계약을 검증합니다.
bool UCFUIHUDProdEditorBridge::ValidateProductionWidgetResult(
	UObject* WidgetBlueprintObject,
	FName WidgetRole,
	UCFHUDLayoutData* LayoutData,
	UCFUIStyleData* StyleData,
	UCFUIDensityData* StandardDensityData,
	UCFUIDensityData* CompactDensityData,
	UCFHUDVisualData* HUDVisualData,
	UObject* SpeedGaugeBlueprintObject,
	UObject* ArmorBodyMapBlueprintObject)
{
#if WITH_EDITOR
	(void)HUDVisualData;
	// [v1.0.0] Production Readback 검증의 상세 실패 사유입니다.
	FString FailureReason;
	if (!CFUIHUDProdEditorBridge::ValidateInputData(LayoutData, StyleData, StandardDensityData, CompactDensityData, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
	const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass())
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production child Blueprint/Tree/Parent validation failed"));
	}
	UClass* SpeedGaugeClass = nullptr;
	UClass* ArmorBodyMapClass = nullptr;
	if (WidgetRole == FName(TEXT("VehiclePanel")))
	{
		SpeedGaugeClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(SpeedGaugeBlueprintObject, TEXT("SpeedGauge"), FailureReason);
		ArmorBodyMapClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(ArmorBodyMapBlueprintObject, TEXT("ArmorBodyMap"), FailureReason);
		if (!SpeedGaugeClass || !ArmorBodyMapClass)
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason);
		}
	}
	if (!CFUIHUDProdEditorBridge::ValidateRoleTree(WidgetBlueprint->WidgetTree, WidgetRole, SpeedGaugeClass, ArmorBodyMapClass, FailureReason)
		|| !CFUIHUDProdEditorBridge::ValidateGraphHasNoRuntimeLogic(WidgetBlueprint, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
	return true;
#else
	return false;
#endif
}

// [v1.0.0] 여섯 Production Panel Generated Class를 D1-07 승인 7 Slot Root Canvas에 조립합니다.
bool UCFUIHUDProdEditorBridge::BuildProductionRootResult(
	UObject* RootWidgetBlueprintObject,
	UCFHUDLayoutData* LayoutData,
	UObject* MissionPanelBlueprintObject,
	UObject* AlertFeedBlueprintObject,
	UObject* TargetPanelBlueprintObject,
	UObject* VehiclePanelBlueprintObject,
	UObject* RadarPanelBlueprintObject,
	UObject* WeaponPanelBlueprintObject)
{
#if WITH_EDITOR
	// [v1.0.0] Root Build Class/Layout 해석의 상세 실패 사유입니다.
	FString FailureReason;
	if (!LayoutData || !LayoutData->ValidateLayoutData(FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FString::Printf(TEXT("Production root layout invalid: %s"), *FailureReason));
	}
	UWidgetBlueprint* RootBlueprint = Cast<UWidgetBlueprint>(RootWidgetBlueprintObject);
	if (!RootBlueprint || RootBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass())
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("WBP_CFInGameHUD must inherit exactly CFStyledWidgetBase"));
	}

	// [v1.0.0] Root에 조립할 여섯 Production Panel Generated Class입니다.
	UClass* MissionClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(MissionPanelBlueprintObject, TEXT("MissionPanel"), FailureReason);
	UClass* AlertClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(AlertFeedBlueprintObject, TEXT("AlertFeed"), FailureReason);
	UClass* TargetClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(TargetPanelBlueprintObject, TEXT("TargetPanel"), FailureReason);
	UClass* VehicleClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(VehiclePanelBlueprintObject, TEXT("VehiclePanel"), FailureReason);
	UClass* RadarClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(RadarPanelBlueprintObject, TEXT("RadarPanel"), FailureReason);
	UClass* WeaponClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(WeaponPanelBlueprintObject, TEXT("WeaponPanel"), FailureReason);
	if (!MissionClass || !AlertClass || !TargetClass || !VehicleClass || !RadarClass || !WeaponClass)
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}

	UWidgetTree* WidgetTree = CFUIHUDProdEditorBridge::ReplaceWidgetTree(RootBlueprint, FailureReason);
	// [v1.0.0] D1-07 화면 Slot 배치만 담당하는 Root Canvas입니다.
	UCanvasPanel* RootCanvas = CFUIHUDProdEditorBridge::CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Root"));
	if (!WidgetTree || !RootCanvas)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production root Canvas construction failed"));
	}
	WidgetTree->RootWidget = RootCanvas;

	const FCFHUDSlotLayout* MissionLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::MissionSummary);
	const FCFHUDSlotLayout* AlertLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::AlertFeed);
	const FCFHUDSlotLayout* TargetLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::TargetPanel);
	const FCFHUDSlotLayout* VehicleLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::VehiclePanel);
	const FCFHUDSlotLayout* RadarLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::RadarPanel);
	const FCFHUDSlotLayout* WeaponLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::WeaponPanel);
	const FCFHUDSlotLayout* ReticleLayout = LayoutData->FindSlotLayout(ECFHUDSlotId::ReticleLayer);
	if (!MissionLayout || !AlertLayout || !TargetLayout || !VehicleLayout || !RadarLayout || !WeaponLayout || !ReticleLayout)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production root required 7 layout slots are missing"));
	}

	if (!CFUIHUDProdEditorBridge::AddRootPanelSlot(WidgetTree, RootCanvas, *MissionLayout, TEXT("SizeBox_Slot_MissionSummary"), TEXT("WBP_CFMissionPanel"), MissionClass)
		|| !CFUIHUDProdEditorBridge::AddRootPanelSlot(WidgetTree, RootCanvas, *AlertLayout, TEXT("SizeBox_Slot_AlertFeed"), TEXT("WBP_CFAlertFeed"), AlertClass)
		|| !CFUIHUDProdEditorBridge::AddRootPanelSlot(WidgetTree, RootCanvas, *TargetLayout, TEXT("SizeBox_Slot_TargetPanel"), TEXT("WBP_CFTargetPanel"), TargetClass)
		|| !CFUIHUDProdEditorBridge::AddRootPanelSlot(WidgetTree, RootCanvas, *VehicleLayout, TEXT("SizeBox_Slot_VehiclePanel"), TEXT("WBP_CFVehiclePanel"), VehicleClass)
		|| !CFUIHUDProdEditorBridge::AddRootPanelSlot(WidgetTree, RootCanvas, *RadarLayout, TEXT("SizeBox_Slot_RadarPanel"), TEXT("WBP_CFRadarPanel"), RadarClass)
		|| !CFUIHUDProdEditorBridge::AddRootPanelSlot(WidgetTree, RootCanvas, *WeaponLayout, TEXT("SizeBox_Slot_WeaponPanel"), TEXT("WBP_CFWeaponPanel"), WeaponClass))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production root panel slot composition failed"));
	}

	// [v1.0.0] AimReticle/후속 World Marker 투영을 보존할 빈 Full Stretch Reticle Layer입니다.
	UCanvasPanel* ReticleLayer = CFUIHUDProdEditorBridge::CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Slot_ReticleLayer"));
	if (!CFUIHUDProdEditorBridge::ApplyRootCanvasLayout(RootCanvas, ReticleLayer, *ReticleLayout))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production root Reticle Layer composition failed"));
	}
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(RootBlueprint);
	return true;
#else
	return false;
#endif
}

// [v1.0.0] 저장된 Production HUD Root의 7 Slot·Panel Class·Layout·Graph 0 계약을 검증합니다.
bool UCFUIHUDProdEditorBridge::ValidateProductionRootResult(
	UObject* RootWidgetBlueprintObject,
	UCFHUDLayoutData* LayoutData,
	UObject* MissionPanelBlueprintObject,
	UObject* AlertFeedBlueprintObject,
	UObject* TargetPanelBlueprintObject,
	UObject* VehiclePanelBlueprintObject,
	UObject* RadarPanelBlueprintObject,
	UObject* WeaponPanelBlueprintObject)
{
#if WITH_EDITOR
	// [v1.0.0] Root Readback 검증의 상세 실패 사유입니다.
	FString FailureReason;
	const UWidgetBlueprint* RootBlueprint = Cast<UWidgetBlueprint>(RootWidgetBlueprintObject);
	if (!LayoutData || !RootBlueprint || !RootBlueprint->WidgetTree || RootBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass())
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production root Blueprint/Layout/Tree/Parent validation failed"));
	}
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(RootBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas || RootCanvas->GetFName() != FName(TEXT("CanvasPanel_Root")) || RootCanvas->GetChildrenCount() != 7)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production root must be CanvasPanel_Root with exactly 7 slots"));
	}

	// [v1.0.0] 검증할 여섯 Production Panel Generated Class입니다.
	UClass* MissionClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(MissionPanelBlueprintObject, TEXT("MissionPanel"), FailureReason);
	UClass* AlertClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(AlertFeedBlueprintObject, TEXT("AlertFeed"), FailureReason);
	UClass* TargetClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(TargetPanelBlueprintObject, TEXT("TargetPanel"), FailureReason);
	UClass* VehicleClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(VehiclePanelBlueprintObject, TEXT("VehiclePanel"), FailureReason);
	UClass* RadarClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(RadarPanelBlueprintObject, TEXT("RadarPanel"), FailureReason);
	UClass* WeaponClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(WeaponPanelBlueprintObject, TEXT("WeaponPanel"), FailureReason);
	if (!MissionClass || !AlertClass || !TargetClass || !VehicleClass || !RadarClass || !WeaponClass)
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}

	struct FExpectedRootSlot
	{
		ECFHUDSlotId SlotId;
		const TCHAR* SizeBoxName;
		const TCHAR* PanelWidgetName;
		UClass* PanelClass;
	};
	const FExpectedRootSlot ExpectedSlots[] =
	{
		{ECFHUDSlotId::MissionSummary, TEXT("SizeBox_Slot_MissionSummary"), TEXT("WBP_CFMissionPanel"), MissionClass},
		{ECFHUDSlotId::AlertFeed, TEXT("SizeBox_Slot_AlertFeed"), TEXT("WBP_CFAlertFeed"), AlertClass},
		{ECFHUDSlotId::TargetPanel, TEXT("SizeBox_Slot_TargetPanel"), TEXT("WBP_CFTargetPanel"), TargetClass},
		{ECFHUDSlotId::VehiclePanel, TEXT("SizeBox_Slot_VehiclePanel"), TEXT("WBP_CFVehiclePanel"), VehicleClass},
		{ECFHUDSlotId::RadarPanel, TEXT("SizeBox_Slot_RadarPanel"), TEXT("WBP_CFRadarPanel"), RadarClass},
		{ECFHUDSlotId::WeaponPanel, TEXT("SizeBox_Slot_WeaponPanel"), TEXT("WBP_CFWeaponPanel"), WeaponClass}
	};
	for (const FExpectedRootSlot& Expected : ExpectedSlots)
	{
		USizeBox* SlotBox = Cast<USizeBox>(RootBlueprint->WidgetTree->FindWidget(FName(Expected.SizeBoxName)));
		UUserWidget* PanelWidget = Cast<UUserWidget>(RootBlueprint->WidgetTree->FindWidget(FName(Expected.PanelWidgetName)));
		const FCFHUDSlotLayout* LayoutEntry = LayoutData->FindSlotLayout(Expected.SlotId);
		if (!SlotBox || !PanelWidget || PanelWidget->GetClass() != Expected.PanelClass || !LayoutEntry
			|| !CFUIHUDProdEditorBridge::ValidateRootCanvasLayout(SlotBox, *LayoutEntry, FailureReason))
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason.IsEmpty() ? FString::Printf(TEXT("Production root slot class/layout failed: %s"), Expected.SizeBoxName) : FailureReason);
		}
	}
	if (!RootBlueprint->WidgetTree->FindWidget(FName(TEXT("CanvasPanel_Slot_ReticleLayer"))))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production root Reticle Layer is missing"));
	}
	if (!CFUIHUDProdEditorBridge::ValidateGraphHasNoRuntimeLogic(RootBlueprint, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
	return true;
#else
	return false;
#endif
}
