// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.14.0
// Date: 2026-08-21
// Description: CF-FQ-032 Designer Layout Ownership + UI-P0-09B View Mode additive migration 구현
// Scope: 저장된 Production UMG의 기존 Slot Layout을 보존하면서 Radar Visual과 Root ReticleLayer의 Vehicle Direction 의미 Widget만 누락 시 추가합니다.
// Changelog:
// - v1.14.0: 기존 WBP_CFInGameHUD Root 7-child 구조와 ReticleLayer Slot을 보존하고 ReticleLayer 내부에 `CanvasPanel_ViewDirection` + `Image_ViewVehicleDirection`만 additive 추가하는 ApplyViewModeVisualMigrationResult를 구현. Vehicle Semantic Icon·TextSecondary·Style IconLarge를 재사용.
// - v1.13.0: RadarPanel Scaffold/Validator를 Frame+Range+Player+SelectedEdge 구조로 확장하고 기존 Designer Tree용 ApplyRadarVisualMigrationResult를 추가. 기존 Radar Widget Slot은 수정하지 않고 Brush/Color만 갱신.
// - v1.12.0: Production Child/Root Build를 RootWidget 없는 최초 Scaffold 전용으로 fail-closed하고 기존 Designer Tree 교체를 차단. Root Validator의 Position/Size/Alignment/ZOrder 비교를 제거해 persisted Designer Layout ownership을 보존.

// - v1.11.0: SpeedGauge의 ProgressBar_RPMTick00~20과 Image_RPMTrackArt를 제거하고 `Image_RPMGauge` 단일 UI Material Image로 교체. Validator가 Material Brush를 요구하고 구형 21 Tick 잔존을 명시적으로 거부.
// - v1.10.0: WBP_CFArmorSector를 방향 Label + Plate Image + 실제 Armor ProgressBar 재사용 Element로 추가. ArmorBodyMap의 18개 직접 방향 자식을 제거하고 6개 Sector 인스턴스 배치로 교체.
// - v1.9.1: 512x256 P2 Frame의 48x24px 내부 경계와 일치하도록 9-Slice UV Margin을 0.09375로 교정. Runtime 의미 변경 없음.
// - v1.9.0: VehiclePanelFrame을 단일 Surface Border의 9-Slice Brush로 소비. ArmorBodyMap에 FRONT/RIGHT/REAR/LEFT/TOP/BOTTOM Text Label을 additive 추가하고 차량 좌향 기준 Front←/Right↑/Rear→/Left↓ + Top 좌상단 + Bottom 우하단 공간 계약으로 배치/Validator 갱신.
// - v1.8.0: WeaponRail Turret/Ammo/Reload Image 3개 제거, 112x68 Text Tile 3개 + 8px gap으로 교체. Header icon 1개만 Image 계약에 남기고 old Rail Image 잔존을 Validator가 거부.
// - v1.7.0: SpeedGauge ProgressBar_RPMTick00~20의 저장 100% 값은 Designer Scale Preview이고 Runtime Presenter가 Percent를 덮어쓰는 의미를 명시. 구조/배치/색/Validator count는 그대로 유지.
// - v1.6.0: WeaponPanel의 Launcher/Ammo/Heat/Cooldown 전용 고정 Row를 Compact Resource Presentation 의미 슬롯으로 원자 교체. ReserveAmmo Header owner와 Compact Rail 구조는 유지하고 Rail은 다중 무기 Runtime 전 기본 Collapsed. Validator가 새 의미 슬롯을 요구하고 구형 전용 Row 잔존을 거부하도록 갱신.
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
// - SpeedArcTrack Texture는 RPM UI Material의 데이터/형상 입력으로 사용합니다. WBP_CFSpeedGauge는 Texture를 직접 그리지 않고 HUDVisualData.SpeedArcMaterial을 `Image_RPMGauge` Brush로 사용합니다.
// - Vehicle/Armor처럼 의미가 동일한 시각 슬롯은 HUDVisualData가 비어 있을 때 기존 StyleData Semantic Icon Texture를 Editor Preview Fallback으로 사용할 수 있습니다.
// - WeaponPanel Resource Presentation은 raw ResourceChannels 개수에 따라 Widget을 생성하지 않습니다. Primary/SecondaryA/SecondaryB/FireState의 고정 의미 슬롯만 만들고 Runtime Presenter가 역할별 Visibility/Text/Progress를 적용합니다.
// - VehicleBattery·WeaponCharge·Heat Runtime 부재를 Designer Preview 값으로 위조하지 않으며 구형 Heat 전용 Row는 Stage B에서 제거합니다.
// - v1.11.0부터 RPM 눈금/Track은 Texture+UI Material 한 장이 담당하고 Runtime UCFHUDPresenter는 `RPMRatio` 스칼라 하나만 갱신합니다. ProgressBar_RPMTick00~20 구조는 금지합니다.
// - v1.8.0 Weapon Rail은 실제 weapon icon source가 없으므로 Image를 만들지 않고 Text Tile만 소유합니다. 비선택 무기의 자원 상태도 현재 HUD ViewData에 없으므로 Designer에서 summary 값을 만들지 않습니다.
// - v1.9.1 VehiclePanel Frame 9-Slice는 SourceArt P2 Frame의 48/512 = 24/256 = 0.09375 UV 경계를 사용합니다.
// - v1.10.0 ArmorBodyMap은 Vehicle Silhouette + 재사용 ArmorSector 6개만 직접 배치합니다. Sector 내부 ProgressBar는 장식이 아니라 실제 방향 Armor Ratio를 표시합니다.
// - v1.12.0부터 아래 Position/Size/SizeBox/Padding 값은 신규 Asset 최초 Scaffold 기본값일 뿐 저장 Asset의 재적용 계약이 아닙니다. 기존 Asset은 Validate-only 경로를 사용합니다.
// - v1.13.0 Radar Visual migration은 기존 Widget의 Position/Size/Anchor/Alignment/AutoSize를 변경하지 않습니다. 누락된 새 의미 Widget에만 최초 배치값을 적용하며 이후 Layout SSOT는 Designer Asset입니다.
// - v1.14.0 ViewMode migration도 같은 ownership을 따릅니다. `CanvasPanel_Slot_ReticleLayer` 자체는 수정하지 않고 새 Direction Track/Image의 최초 Scaffold Slot만 설정합니다.



#include "UI/CFUIHUDProdEditorBridge.h"

#include "UI/CFArmorSectorWidget.h"
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
#include "Components/OverlaySlot.h"
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
#include "Materials/MaterialInterface.h"
#include "Styling/SlateBrush.h"
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

					// [v1.11.0] HUDVisualData의 UI Material만 Brush로 사용하는 Image를 만들고 Material 누락 시 빈 사각형을 렌더링하지 않습니다.
	UImage* CreatePreferredMaterialImage(
		UWidgetTree* WidgetTree,
		const TCHAR* WidgetName,
		const TSoftObjectPtr<UMaterialInterface>& PreferredMaterial)
	{
		// [v1.11.0] RPM Gauge Material을 표시할 단일 의미 Image Widget입니다.
		UImage* Image = CreateWidget<UImage>(WidgetTree, WidgetName);
		if (!Image)
		{
			return nullptr;
		}

		// [v1.11.0] HUDVisualData에 연결된 실제 UI Domain Material입니다.
		UMaterialInterface* Material = PreferredMaterial.IsNull() ? nullptr : PreferredMaterial.LoadSynchronous();
		if (!Material)
		{
			Image->SetVisibility(ESlateVisibility::Collapsed);
			return Image;
		}

		Image->SetBrushFromMaterial(Material);
		Image->SetColorAndOpacity(FLinearColor::White);
		Image->SetVisibility(ESlateVisibility::HitTestInvisible);
		return Image;
	}

	// [v1.9.1] P2 VehiclePanelFrame의 48px horizontal / 24px vertical 내부 경계에 대응하는 normalized 9-Slice UV Margin입니다.
	constexpr float VehiclePanelFrameSliceMargin = 0.09375f;

	// [v1.9.0] Production Panel의 유일한 Surface Border와 Content VerticalBox를 생성하고 선택적 9-Slice Frame을 적용합니다.
	UVerticalBox* BuildPanelSurface(
		UWidgetTree* WidgetTree,
		const UCFUIStyleData* StyleData,
		const TSoftObjectPtr<UTexture2D>& PreferredFrame = TSoftObjectPtr<UTexture2D>())
	{
		// [v1.0.0] Panel 배경 Surface를 담당하는 유일한 Border입니다.
		UBorder* Surface = CreateWidget<UBorder>(WidgetTree, TEXT("Border_Surface"));
		// [v1.0.0] Panel 의미 콘텐츠를 자동 배치할 VerticalBox입니다.
		UVerticalBox* Content = CreateWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_Content"));
		if (!Surface || !Content || !StyleData)
		{
			return nullptr;
		}

		// [v1.9.0] VehiclePanel처럼 전용 Frame이 연결됐을 때 단일 Border를 그대로 9-Slice Brush로 사용하는 실제 Texture입니다.
		UTexture2D* FrameTexture = PreferredFrame.IsNull() ? nullptr : PreferredFrame.LoadSynchronous();
		if (FrameTexture)
		{
			// [v1.9.0] Corner 두께를 유지하며 중앙/Edge만 늘어나도록 설정할 9-Slice Slate Brush입니다.
			FSlateBrush FrameBrush;
			FrameBrush.SetResourceObject(FrameTexture);
			FrameBrush.DrawAs = ESlateBrushDrawType::Box;
									FrameBrush.Margin = FMargin(VehiclePanelFrameSliceMargin);
			FrameBrush.TintColor = FSlateColor(FLinearColor::White);
			Surface->SetBrush(FrameBrush);
			Surface->SetBrushColor(FLinearColor::White);
		}
		else
		{
			Surface->SetBrushColor(StyleData->ResolveColor(ECFUIColorToken::SurfaceBase));
		}

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

			// [v1.11.0] SpeedGauge Element를 단일 동적 RPM UI Material Image와 디지털 속도·단일 Gear Slot 구조로 생성합니다.
	bool BuildSpeedGauge(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData, const UCFHUDVisualData* HUDVisualData)
	{
		// [v1.11.0] 동적 RPM Gauge와 속도/단위/Gear의 의미 위치만 담당하는 Root Canvas입니다.
		UCanvasPanel* Root = CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Root"));
		// [v1.11.0] Track·21 Tick·Red Zone 표현을 Texture/Material 한 장에서 처리하는 단일 RPM Gauge Image입니다.
		UImage* RPMGaugeImage = CreatePreferredMaterialImage(
			WidgetTree,
			TEXT("Image_RPMGauge"),
			HUDVisualData ? HUDVisualData->SpeedArcMaterial : TSoftObjectPtr<UMaterialInterface>());
		if (!Root || !RPMGaugeImage)
		{
			return false;
		}
		WidgetTree->RootWidget = Root;
		if (!AddCanvasChild(Root, RPMGaugeImage, FVector2D(8.0f, 14.0f), FVector2D(220.0f, 154.0f), 0))
		{
			return false;
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

			// [v1.10.0] 한 방향 Armor의 Label, Plate Image와 실제 Armor Ratio Bar를 재사용 가능한 의미 Element로 생성합니다.
	bool BuildArmorSector(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData)
	{
		// [v1.10.0] 한 Sector의 고정 68x58 Designer 크기를 소유하는 Root SizeBox입니다.
		USizeBox* Root = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_Root"));
		// [v1.10.0] Plate 묶음과 실제 Armor Bar를 좌우로 배치하는 Content Row입니다.
		UHorizontalBox* Content = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_Content"));
		// [v1.10.0] 방향별 Plate Image와 작은 방향 Label을 겹치는 58x58 영역입니다.
		USizeBox* PlateBox = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_Plate"));
		// [v1.10.0] Plate Image와 방향 Label만 겹치는 시각 Overlay입니다.
		UOverlay* PlateOverlay = CreateWidget<UOverlay>(WidgetTree, TEXT("Overlay_Plate"));
		// [v1.10.0] 실제 방향별 Armor Art를 표시하며 Sector 인스턴스 설정이 있으면 Texture가 교체되는 Image입니다.
		UImage* ArmorImage = CreateImage(WidgetTree, TEXT("Image_ArmorPlate"), StyleData, FName(TEXT("Armor")), ECFUIColorToken::Armor);
		// [v1.10.0] 방향 Art를 가리지 않도록 Caption 크기로 낮춘 보조 방향 Label입니다.
		UTextBlock* DirectionText = CreateText(WidgetTree, TEXT("Text_Direction"), TEXT("ARMOR"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);
		// [v1.10.0] Plate와 실제 Armor Bar 사이의 작은 구조적 간격입니다.
		USpacer* ArmorGap = CreateSpacer(WidgetTree, TEXT("Spacer_ArmorGap"), FVector2D(2.0f, 1.0f));
		// [v1.10.0] 실제 Armor Bar의 8x58 크기를 고정하는 SizeBox입니다.
		USizeBox* ProgressBox = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_ArmorProgress"));
		// [v1.10.0] 실제 방향 Armor Ratio를 아래에서 위로 채우는 단일 ProgressBar입니다.
		UProgressBar* ArmorBar = CreateVerticalProgress(WidgetTree, TEXT("ProgressBar_Armor"), 1.0f, StyleData, ECFUIColorToken::Armor);
		if (!Root || !Content || !PlateBox || !PlateOverlay || !ArmorImage || !DirectionText || !ArmorGap || !ProgressBox || !ArmorBar)
		{
			return false;
		}

		Root->SetWidthOverride(68.0f);
		Root->SetHeightOverride(58.0f);
		PlateBox->SetWidthOverride(58.0f);
		PlateBox->SetHeightOverride(58.0f);
		ProgressBox->SetWidthOverride(8.0f);
		ProgressBox->SetHeightOverride(58.0f);
		DirectionText->SetJustification(ETextJustify::Center);

		// [v1.10.0] Plate Image를 58x58 Overlay 전체에 채우는 Slot입니다.
		UOverlaySlot* ArmorImageSlot = PlateOverlay->AddChildToOverlay(ArmorImage);
		// [v1.10.0] 작은 방향 Label을 Plate 상단 중앙에 두는 Slot입니다.
		UOverlaySlot* DirectionTextSlot = PlateOverlay->AddChildToOverlay(DirectionText);
		if (!ArmorImageSlot || !DirectionTextSlot)
		{
			return false;
		}
		ArmorImageSlot->SetHorizontalAlignment(HAlign_Fill);
		ArmorImageSlot->SetVerticalAlignment(VAlign_Fill);
		DirectionTextSlot->SetHorizontalAlignment(HAlign_Center);
		DirectionTextSlot->SetVerticalAlignment(VAlign_Top);
		DirectionTextSlot->SetPadding(FMargin(0.0f, 1.0f, 0.0f, 0.0f));

		PlateBox->SetContent(PlateOverlay);
		ProgressBox->SetContent(ArmorBar);
		Content->AddChild(PlateBox);
		Content->AddChild(ArmorGap);
		Content->AddChild(ProgressBox);
		Root->SetContent(Content);
		WidgetTree->RootWidget = Root;
		return true;
	}

	// [v1.10.0] Armor Body Map을 중앙 Vehicle + 완성된 재사용 ArmorSector 6개의 공간 배치 구조로 생성합니다.
	bool BuildArmorBodyMap(
		UWidgetTree* WidgetTree,
		const UCFUIStyleData* StyleData,
		const UCFHUDVisualData* HUDVisualData,
		UClass* ArmorSectorClass)
	{
		// [v1.10.0] 차체와 완성된 방향 Sector의 공간 관계만 담당하는 Root Canvas입니다.
		UCanvasPanel* Root = CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_Root"));
		if (!Root || !ArmorSectorClass || !ArmorSectorClass->IsChildOf(UCFArmorSectorWidget::StaticClass()))
		{
			return false;
		}
		WidgetTree->RootWidget = Root;

		// [v1.10.0] HUDVisualData가 없을 때 동일 의미 Semantic Vehicle Texture로 대체할 중앙 좌향 차량 실루엣 Image입니다.
		UImage* VehicleImage = CreateImage(WidgetTree, TEXT("Image_VehicleSilhouette"), StyleData, FName(TEXT("Vehicle")), ECFUIColorToken::TextSecondary, HUDVisualData ? HUDVisualData->VehicleSilhouette : TSoftObjectPtr<UTexture2D>());
		if (!AddCanvasChild(Root, VehicleImage, FVector2D(112.0f, 57.0f), FVector2D(154.0f, 76.0f), 1))
		{
			return false;
		}

		// [v1.10.0] 한 방향 Sector 인스턴스의 이름, 정적 방향 정보, Preview 비율과 BodyMap 위치를 묶습니다.
		struct FArmorSectorSpec
		{
			// [v1.10.0] ArmorBodyMap에서 Presenter와 Validator가 찾는 재사용 Sector 인스턴스 이름입니다.
			const TCHAR* WidgetName;
			// [v1.10.0] 차량 로컬 방향을 즉시 읽을 수 있게 표시할 짧은 방향 문자열입니다.
			const TCHAR* DirectionText;
			// [v1.10.0] 저장 Asset Designer에서 방향별 Bar 형태를 구분하기 위한 Preview 비율입니다.
			float DesignerArmorPercent;
			// [v1.10.0] ArmorBodyMap Canvas 안에서 완성된 Sector 전체의 위치입니다.
			FVector2D Position;
			// [v1.10.0] 실제 HUDVisualData에 연결된 현재 방향 전용 Texture입니다.
			TSoftObjectPtr<UTexture2D> PreferredTexture;
		};

		// [v1.10.0] 사용자 확정 공간 계약: Front=좌←, Right=상↑, Rear=우→, Left=하↓, Top=좌상단, Bottom=우하단입니다.
		const FArmorSectorSpec ArmorSpecs[] =
		{
			{TEXT("WBP_ArmorFront"), TEXT("FRONT"), 0.30f, FVector2D(8.0f, 66.0f), HUDVisualData ? HUDVisualData->ArmorPlates.FrontPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("WBP_ArmorRight"), TEXT("RIGHT"), 0.70f, FVector2D(184.0f, 2.0f), HUDVisualData ? HUDVisualData->ArmorPlates.RightPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("WBP_ArmorRear"), TEXT("REAR"), 0.75f, FVector2D(308.0f, 66.0f), HUDVisualData ? HUDVisualData->ArmorPlates.RearPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("WBP_ArmorLeft"), TEXT("LEFT"), 0.63f, FVector2D(184.0f, 130.0f), HUDVisualData ? HUDVisualData->ArmorPlates.LeftPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("WBP_ArmorTop"), TEXT("TOP"), 0.84f, FVector2D(78.0f, 2.0f), HUDVisualData ? HUDVisualData->ArmorPlates.TopPlate : TSoftObjectPtr<UTexture2D>()},
			{TEXT("WBP_ArmorBottom"), TEXT("BOTTOM"), 0.72f, FVector2D(286.0f, 130.0f), HUDVisualData ? HUDVisualData->ArmorPlates.BottomPlate : TSoftObjectPtr<UTexture2D>()}
		};

		for (const FArmorSectorSpec& Spec : ArmorSpecs)
		{
			// [v1.10.0] 동일 WBP_CFArmorSector Generated Class를 재사용하는 현재 방향 Sector 인스턴스입니다.
			UCFArmorSectorWidget* ArmorSector = WidgetTree->ConstructWidget<UCFArmorSectorWidget>(ArmorSectorClass, FName(Spec.WidgetName));
			if (!ArmorSector)
			{
				return false;
			}

			// [v1.10.0] 현재 방향에 연결된 P2 Plate Texture이며 없으면 ArmorSector 기본 Semantic Image를 그대로 사용합니다.
			UTexture2D* ArmorTexture = Spec.PreferredTexture.IsNull() ? nullptr : Spec.PreferredTexture.LoadSynchronous();
			ArmorSector->ConfigureSector(FText::FromString(Spec.DirectionText), ArmorTexture, Spec.DesignerArmorPercent);
			if (!AddCanvasChild(Root, ArmorSector, Spec.Position, FVector2D(68.0f, 58.0f), 2))
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
		const UCFHUDVisualData* HUDVisualData,
		UClass* SpeedGaugeClass,
		UClass* ArmorBodyMapClass)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData, HUDVisualData ? HUDVisualData->VehiclePanelFrame : TSoftObjectPtr<UTexture2D>());
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

		// [v1.13.0] Radar Panel을 전용 Frame/Blip/Player/Selection Image와 Range Text의 공간 Field로 생성합니다. 이 위치값은 신규 Asset 최초 Scaffold 시작값입니다.
	bool BuildRadarPanel(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData, const UCFHUDVisualData* HUDVisualData)
	{
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		if (!Content)
		{
			return false;
		}
		Content->AddChild(BuildHeader(WidgetTree, TEXT("HorizontalBox_Header"), TEXT("Image_RadarHeader"), FName(TEXT("RadarContact")), TEXT("Text_RadarTitle"), TEXT("RADAR"), StyleData, LayoutData, ECFUIColorToken::AccentTactical));

		// [v1.13.0] Radar Contact의 Heading-Up 공간 관계와 runtime Blip을 담당하는 제한된 Canvas Field입니다.
		UCanvasPanel* RadarField = CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_RadarContacts"));
		// [v1.13.0] 신규 Scaffold에서 Radar Field의 시작 높이만 제공하는 SizeBox입니다.
		USizeBox* RadarBox = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_RadarField"));
		if (!RadarField || !RadarBox)
		{
			return false;
		}
		RadarBox->SetHeightOverride(166.0f);
		RadarBox->SetContent(RadarField);
		Content->AddChild(RadarBox);

		// [v1.13.0] Grid·3 Range Ring·외곽 Frame을 한 장으로 표시하는 Radar 배경 Image입니다.
		UImage* RadarFrame = CreateImage(WidgetTree, TEXT("Image_RadarFrame"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::AccentTactical, HUDVisualData ? HUDVisualData->RadarFrame : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] Friendly 원형 Blip의 숨겨진 Designer Brush/Size Template입니다.
		UImage* Friendly = CreateImage(WidgetTree, TEXT("Image_RadarFriendly"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Friendly, HUDVisualData ? HUDVisualData->RadarFriendlyBlip : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] Neutral은 Unknown 마름모 Texture를 재사용하고 관계색만 Neutral로 유지하는 Template입니다.
		UImage* Neutral = CreateImage(WidgetTree, TEXT("Image_RadarNeutral"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Neutral, HUDVisualData ? HUDVisualData->RadarUnknownBlip : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] Hostile 하향 삼각형 Blip의 숨겨진 Designer Brush/Size Template입니다.
		UImage* Hostile = CreateImage(WidgetTree, TEXT("Image_RadarHostile"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Hostile, HUDVisualData ? HUDVisualData->RadarHostileBlip : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] Unknown 마름모 Blip의 숨겨진 Designer Brush/Size Template입니다.
		UImage* Unknown = CreateImage(WidgetTree, TEXT("Image_RadarUnknown"), StyleData, FName(TEXT("RadarContact")), ECFUIColorToken::Unknown, HUDVisualData ? HUDVisualData->RadarUnknownBlip : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] Heading-Up Radar 중앙에 고정되는 Player 상향 삼각형 Image입니다.
		UImage* Player = CreateImage(WidgetTree, TEXT("Image_RadarPlayer"), StyleData, FName(TEXT("Vehicle")), ECFUIColorToken::AccentTactical, HUDVisualData ? HUDVisualData->RadarPlayerMarker : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] 표시 Range 안에서 선택 Contact를 감싸는 4-Corner Bracket입니다.
		UImage* Selected = CreateImage(WidgetTree, TEXT("Image_RadarSelected"), StyleData, FName(TEXT("Target")), ECFUIColorToken::AccentTactical, HUDVisualData ? HUDVisualData->SelectedTargetBracket : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] 표시 Range 밖 선택 Contact 방향에만 표시하는 2-Corner Open Edge Bracket입니다.
		UImage* SelectedEdge = CreateImage(WidgetTree, TEXT("Image_RadarSelectedEdge"), StyleData, FName(TEXT("Target")), ECFUIColorToken::AccentTactical, HUDVisualData ? HUDVisualData->RadarSelectedEdgeBracket : TSoftObjectPtr<UTexture2D>());
		// [v1.13.0] 현재 단계식 Radar Display Range를 실제 Runtime Text로 표시할 의미 슬롯입니다.
		UTextBlock* RangeText = CreateText(WidgetTree, TEXT("Text_RadarRange"), TEXT("RANGE —"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);
		if (!RadarFrame || !Friendly || !Neutral || !Hostile || !Unknown || !Player || !Selected || !SelectedEdge || !RangeText)
		{
			return false;
		}

		SelectedEdge->SetVisibility(ESlateVisibility::Collapsed);
		return AddCanvasChild(RadarField, RadarFrame, FVector2D(0.0f, 0.0f), FVector2D(288.0f, 166.0f), 0)
			&& AddCanvasChild(RadarField, Friendly, FVector2D(54.0f, 86.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Neutral, FVector2D(174.0f, 48.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Hostile, FVector2D(228.0f, 96.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Unknown, FVector2D(112.0f, 122.0f), FVector2D(20.0f, 20.0f), 1)
			&& AddCanvasChild(RadarField, Player, FVector2D(134.0f, 73.0f), FVector2D(20.0f, 20.0f), 2)
			&& AddCanvasChild(RadarField, Selected, FVector2D(214.0f, 82.0f), FVector2D(48.0f, 48.0f), 3)
			&& AddCanvasChild(RadarField, SelectedEdge, FVector2D(124.0f, 63.0f), FVector2D(40.0f, 40.0f), 3)
			&& AddCanvasChild(RadarField, RangeText, FVector2D(196.0f, 6.0f), FVector2D(84.0f, 20.0f), 4);
	}

			// [v1.6.0] Weapon Panel을 Header Reserve와 Compact Primary/Secondary/FireState 의미 슬롯으로 생성합니다.
	bool BuildWeaponPanel(UWidgetTree* WidgetTree, const UCFUIStyleData* StyleData, const UCFHUDLayoutData* LayoutData)
	{
		// [v1.6.0] WeaponPanel의 단일 Surface 안에 Header, Resource Presentation, Rail을 세로 배치할 Content입니다.
		UVerticalBox* Content = BuildPanelSurface(WidgetTree, StyleData);
		if (!Content)
		{
			return false;
		}

		// [v1.6.0] 기존 검증된 Header 제작 경로를 재사용해 무기 아이콘·제목의 Style 계약을 보존합니다.
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

		// [v1.6.0] Header가 아직 Root Tree에 붙기 전에도 안전하게 가져올 수 있는 BuildHeader의 세 번째 직접 자식인 무기 제목 Widget입니다.
		UTextBlock* WeaponTitleText = WeaponHeader->GetChildrenCount() > 2
			? Cast<UTextBlock>(WeaponHeader->GetChildAt(2))
			: nullptr;

		// [v1.6.0] 제목을 Fill 규칙으로 바꿔 고정 픽셀 Spacer 없이 Header 폭을 안전하게 분배할 Slot입니다.
		UHorizontalBoxSlot* WeaponTitleSlot = WeaponTitleText ? Cast<UHorizontalBoxSlot>(WeaponTitleText->Slot) : nullptr;
		if (!WeaponTitleSlot)
		{
			return false;
		}

		// [v1.6.0] 현재 무기 제목이 Reserve 앞까지 남는 가로 공간을 모두 사용하도록 하는 Fill 크기 규칙입니다.
		FSlateChildSize WeaponTitleSize;
		WeaponTitleSize.SizeRule = ESlateSizeRule::Fill;
		WeaponTitleSlot->SetSize(WeaponTitleSize);
		WeaponTitleSlot->SetVerticalAlignment(VAlign_Center);

		// [v1.6.0] 실제 Reserve가 없을 때도 KnownZero를 같은 슬롯에 표시할 수 있는 Header 숫자 Text입니다.
		UTextBlock* ReserveAmmoText = CreateText(
			WidgetTree,
			TEXT("Text_WeaponReserveAmmo"),
			TEXT("0"),
			StyleData,
			LayoutData,
			ECFUIFontFamilyRole::Numeric,
			ECFUITypographyRole::ValueS,
			ECFUIColorToken::TextSecondary);

		// [v1.6.0] Reserve 숫자를 Header의 실제 마지막 자식으로 두고 우측 정렬하는 Slot입니다.
		UHorizontalBoxSlot* ReserveAmmoSlot = ReserveAmmoText ? WeaponHeader->AddChildToHorizontalBox(ReserveAmmoText) : nullptr;
		if (!ReserveAmmoSlot)
		{
			return false;
		}
		ReserveAmmoSlot->SetHorizontalAlignment(HAlign_Right);
		ReserveAmmoSlot->SetVerticalAlignment(VAlign_Center);
		Content->AddChild(WeaponHeader);

		// [v1.6.0] Stage A Projection의 최대 cardinality만 소유하고 raw ResourceChannels 개수와 독립적인 Resource Presentation Container입니다.
		UVerticalBox* ResourcePresentation = CreateWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_ResourcePresentation"));
		if (!ResourcePresentation)
		{
			return false;
		}
		Content->AddChild(ResourcePresentation);

		// [v1.6.0] Launcher 또는 Ammo가 차지하는 Compact Primary 최대 한 개의 의미 Row입니다.
		UHorizontalBox* PrimaryResourceRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_PrimaryResource"));
		// [v1.6.0] Primary Player-facing 문자열을 크게 표시할 Text입니다.
		UTextBlock* PrimaryResourceText = CreateText(WidgetTree, TEXT("Text_WeaponPrimaryResource"), TEXT("SALVO 2 / 4"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueM, ECFUIColorToken::AccentTactical);
		// [v1.6.0] Sequence 같은 Primary Entry가 실제 진행률을 가질 때만 Runtime에서 노출할 ProgressBar입니다.
		UProgressBar* PrimaryResourceProgress = CreateProgress(WidgetTree, TEXT("ProgressBar_PrimaryResource"), 0.5f, StyleData, ECFUIColorToken::AccentTactical);
		if (!PrimaryResourceRow || !PrimaryResourceText || !PrimaryResourceProgress)
		{
			return false;
		}
		PrimaryResourceRow->AddChild(PrimaryResourceText);
		ResourcePresentation->AddChild(PrimaryResourceRow);
		ResourcePresentation->AddChild(PrimaryResourceProgress);

		// [v1.6.0] Secondary 최대 두 개를 0개/1개/2개 계약으로 배치할 의미 Row입니다.
		UHorizontalBox* SecondaryResourcesRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_SecondaryResources"));
		// [v1.6.0] 첫 번째 Secondary Entry의 Text와 선택적 Progress를 함께 소유하는 의미 Container입니다.
		UVerticalBox* SecondaryResourceA = CreateWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_SecondaryResourceA"));
		// [v1.6.0] 두 번째 Secondary Entry의 Text와 선택적 Progress를 함께 소유하는 의미 Container입니다.
		UVerticalBox* SecondaryResourceB = CreateWidget<UVerticalBox>(WidgetTree, TEXT("VerticalBox_SecondaryResourceB"));
		// [v1.6.0] Launcher Primary 중에도 보존되는 Ammo 같은 첫 번째 보조 문자열입니다.
		UTextBlock* SecondaryResourceTextA = CreateText(WidgetTree, TEXT("Text_WeaponSecondaryResourceA"), TEXT("5 / 10"), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextPrimary);
		// [v1.6.0] 향후 실제 Secondary Entry가 진행률을 가질 때만 Runtime에서 노출할 첫 번째 보조 ProgressBar입니다.
		UProgressBar* SecondaryResourceProgressA = CreateProgress(WidgetTree, TEXT("ProgressBar_SecondaryResourceA"), 0.0f, StyleData, ECFUIColorToken::TextSecondary);
		// [v1.6.0] 두 번째 Secondary Entry의 Player-facing 문자열 슬롯입니다.
		UTextBlock* SecondaryResourceTextB = CreateText(WidgetTree, TEXT("Text_WeaponSecondaryResourceB"), TEXT(""), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextSecondary);
		// [v1.6.0] 향후 실제 Secondary Entry가 진행률을 가질 때만 Runtime에서 노출할 두 번째 보조 ProgressBar입니다.
		UProgressBar* SecondaryResourceProgressB = CreateProgress(WidgetTree, TEXT("ProgressBar_SecondaryResourceB"), 0.0f, StyleData, ECFUIColorToken::TextSecondary);
		if (!SecondaryResourcesRow || !SecondaryResourceA || !SecondaryResourceB
			|| !SecondaryResourceTextA || !SecondaryResourceProgressA
			|| !SecondaryResourceTextB || !SecondaryResourceProgressB)
		{
			return false;
		}
		SecondaryResourceA->AddChild(SecondaryResourceTextA);
		SecondaryResourceA->AddChild(SecondaryResourceProgressA);
		SecondaryResourceB->AddChild(SecondaryResourceTextB);
		SecondaryResourceB->AddChild(SecondaryResourceProgressB);
		// [v1.6.0] Secondary A가 혼자일 때 전체 폭, A/B 둘일 때 동일 비율로 분배되게 할 첫 번째 Fill Slot입니다.
		UHorizontalBoxSlot* SecondarySlotA = SecondaryResourcesRow->AddChildToHorizontalBox(SecondaryResourceA);
		// [v1.6.0] 두 번째 Secondary가 실제 표시될 때 A와 동일 비율로 공간을 나눌 Fill Slot입니다.
		UHorizontalBoxSlot* SecondarySlotB = SecondaryResourcesRow->AddChildToHorizontalBox(SecondaryResourceB);
		if (!SecondarySlotA || !SecondarySlotB)
		{
			return false;
		}
		// [v1.6.0] Secondary A/B가 같은 가중치로 남는 가로 공간을 나눠 쓰는 Fill 크기 규칙입니다.
		FSlateChildSize SecondaryFillSize;
		SecondaryFillSize.SizeRule = ESlateSizeRule::Fill;
		SecondarySlotA->SetSize(SecondaryFillSize);
		SecondarySlotB->SetSize(SecondaryFillSize);
		SecondaryResourceProgressA->SetVisibility(ESlateVisibility::Collapsed);
		SecondaryResourceB->SetVisibility(ESlateVisibility::Collapsed);
		SecondaryResourceProgressB->SetVisibility(ESlateVisibility::Collapsed);
		ResourcePresentation->AddChild(SecondaryResourcesRow);

		// [v1.6.0] Reload > NoAmmo > Cooldown/READY 중 정확히 한 상태만 표시할 FireState 의미 Row입니다.
		UHorizontalBox* FireStateRow = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_FireState"));
		// [v1.6.0] FireState Resolver가 완성한 Player-facing 상태 문자열을 표시할 Text입니다.
		UTextBlock* FireStateText = CreateText(WidgetTree, TEXT("Text_WeaponFireState"), TEXT(""), StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::ValueS, ECFUIColorToken::TextSecondary);
		// [v1.6.0] Reload/Cooldown처럼 실제 진행률이 있는 FireState에서만 Runtime이 노출할 ProgressBar입니다.
		UProgressBar* FireStateProgress = CreateProgress(WidgetTree, TEXT("ProgressBar_FireState"), 0.0f, StyleData, ECFUIColorToken::AccentTactical);
		if (!FireStateRow || !FireStateText || !FireStateProgress)
		{
			return false;
		}
		FireStateRow->AddChild(FireStateText);
		FireStateRow->SetVisibility(ESlateVisibility::Collapsed);
		FireStateProgress->SetVisibility(ESlateVisibility::Collapsed);
		ResourcePresentation->AddChild(FireStateRow);
		ResourcePresentation->AddChild(FireStateProgress);

				// [v1.8.0] 실제 비선택 Weapon Selection만 최대 3개 표시할 fixed-cardinality Compact Rail입니다.
		UHorizontalBox* Rail = CreateWidget<UHorizontalBox>(WidgetTree, TEXT("HorizontalBox_WeaponRail"));
		if (!Rail)
		{
			return false;
		}

		// [v1.8.0] Rail 첫 번째 fixed 112x68 Text Tile입니다. 실제 문자열은 Runtime Presenter가 순번+DisplayName으로 덮어씁니다.
		USizeBox* RailTile1 = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_WeaponRail1"));
		// [v1.8.0] Rail 두 번째 fixed 112x68 Text Tile입니다.
		USizeBox* RailTile2 = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_WeaponRail2"));
		// [v1.8.0] Rail 세 번째 fixed 112x68 Text Tile 또는 +N overflow 슬롯입니다.
		USizeBox* RailTile3 = CreateWidget<USizeBox>(WidgetTree, TEXT("SizeBox_WeaponRail3"));
		// [v1.8.0] 첫 Rail Tile의 비선택 무기 순번+DisplayName 문자열입니다.
		UTextBlock* RailText1 = CreateText(WidgetTree, TEXT("Text_WeaponRail1"), TEXT("02  WEAPON"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);
		// [v1.8.0] 두 번째 Rail Tile의 비선택 무기 순번+DisplayName 문자열입니다.
		UTextBlock* RailText2 = CreateText(WidgetTree, TEXT("Text_WeaponRail2"), TEXT("03  WEAPON"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);
		// [v1.8.0] 세 번째 Rail Tile의 비선택 무기 문자열 또는 overflow +N 문자열입니다.
		UTextBlock* RailText3 = CreateText(WidgetTree, TEXT("Text_WeaponRail3"), TEXT("+2"), StyleData, LayoutData, ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);
		if (!RailTile1 || !RailTile2 || !RailTile3 || !RailText1 || !RailText2 || !RailText3)
		{
			return false;
		}

		RailTile1->SetWidthOverride(112.0f);
		RailTile1->SetHeightOverride(68.0f);
		RailTile1->SetContent(RailText1);
		RailTile2->SetWidthOverride(112.0f);
		RailTile2->SetHeightOverride(68.0f);
		RailTile2->SetContent(RailText2);
		RailTile3->SetWidthOverride(112.0f);
		RailTile3->SetHeightOverride(68.0f);
		RailTile3->SetContent(RailText3);

		// [v1.8.0] Saved Designer 상태에서 fake selectable weapon을 노출하지 않도록 Tile과 Rail 모두 기본 Collapsed입니다.
		RailTile1->SetVisibility(ESlateVisibility::Collapsed);
		RailTile2->SetVisibility(ESlateVisibility::Collapsed);
		RailTile3->SetVisibility(ESlateVisibility::Collapsed);
		Rail->AddChild(RailTile1);
		Rail->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_WeaponRailGap1"), FVector2D(8.0f, 1.0f)));
		Rail->AddChild(RailTile2);
		Rail->AddChild(CreateSpacer(WidgetTree, TEXT("Spacer_WeaponRailGap2"), FVector2D(8.0f, 1.0f)));
		Rail->AddChild(RailTile3);
		Rail->SetVisibility(ESlateVisibility::Collapsed);
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
		UClass* ArmorSectorClass,
		FString& OutFailureReason)
	{
		if (WidgetRole == FName(TEXT("SpeedGauge"))) return BuildSpeedGauge(WidgetTree, StyleData, LayoutData, HUDVisualData);
		if (WidgetRole == FName(TEXT("ArmorSector"))) return BuildArmorSector(WidgetTree, StyleData, LayoutData);
		if (WidgetRole == FName(TEXT("ArmorBodyMap")))
		{
			if (!ArmorSectorClass)
			{
				OutFailureReason = TEXT("ArmorBodyMap requires compiled ArmorSector class");
				return false;
			}
			return BuildArmorBodyMap(WidgetTree, StyleData, HUDVisualData, ArmorSectorClass);
		}
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
									return BuildVehiclePanel(WidgetTree, StyleData, LayoutData, HUDVisualData, SpeedGaugeClass, ArmorBodyMapClass);
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
		bool ValidateRoleTree(UWidgetTree* WidgetTree, const FName WidgetRole, UClass* SpeedGaugeClass, UClass* ArmorBodyMapClass, UClass* ArmorSectorClass, FString& OutFailureReason)
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

				const bool bPanelRole = WidgetRole != FName(TEXT("SpeedGauge"))
			&& WidgetRole != FName(TEXT("ArmorBodyMap"))
			&& WidgetRole != FName(TEXT("ArmorSector"));
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

						// [v1.11.0] 역할별 최소 실제 Image 슬롯 개수입니다. SpeedGauge는 Track/Tick을 포함한 단일 UI Material Image를 사용합니다.
		int32 RequiredImageCount = 1;
		if (WidgetRole == FName(TEXT("AlertFeed"))) RequiredImageCount = 2;
				else if (WidgetRole == FName(TEXT("SpeedGauge"))) RequiredImageCount = 1;
		else if (WidgetRole == FName(TEXT("ArmorSector"))) RequiredImageCount = 1;
		else if (WidgetRole == FName(TEXT("ArmorBodyMap"))) RequiredImageCount = 1;
		else if (WidgetRole == FName(TEXT("VehiclePanel"))) RequiredImageCount = 2;
				else if (WidgetRole == FName(TEXT("RadarPanel"))) RequiredImageCount = 9;
								else if (WidgetRole == FName(TEXT("WeaponPanel"))) RequiredImageCount = 1;
				if (ImageCount < RequiredImageCount)
		{
			OutFailureReason = FString::Printf(TEXT("Production Image slot contract failed: role=%s actual=%d required=%d"), *WidgetRole.ToString(), ImageCount, RequiredImageCount);
			return false;
		}

						if (WidgetRole == FName(TEXT("WeaponPanel")))
		{
			// [v1.6.0] Stage B Compact Resource Presentation이 반드시 소유해야 하는 고정 의미 슬롯 이름입니다.
			const FName RequiredWeaponResourceSlots[] =
			{
				FName(TEXT("VerticalBox_ResourcePresentation")),
				FName(TEXT("HorizontalBox_PrimaryResource")),
				FName(TEXT("Text_WeaponPrimaryResource")),
				FName(TEXT("ProgressBar_PrimaryResource")),
				FName(TEXT("HorizontalBox_SecondaryResources")),
				FName(TEXT("VerticalBox_SecondaryResourceA")),
				FName(TEXT("Text_WeaponSecondaryResourceA")),
				FName(TEXT("ProgressBar_SecondaryResourceA")),
				FName(TEXT("VerticalBox_SecondaryResourceB")),
				FName(TEXT("Text_WeaponSecondaryResourceB")),
				FName(TEXT("ProgressBar_SecondaryResourceB")),
				FName(TEXT("HorizontalBox_FireState")),
				FName(TEXT("Text_WeaponFireState")),
				FName(TEXT("ProgressBar_FireState")),
								FName(TEXT("Text_WeaponReserveAmmo")),
				FName(TEXT("HorizontalBox_WeaponRail")),
				FName(TEXT("SizeBox_WeaponRail1")),
				FName(TEXT("Text_WeaponRail1")),
				FName(TEXT("Spacer_WeaponRailGap1")),
				FName(TEXT("SizeBox_WeaponRail2")),
				FName(TEXT("Text_WeaponRail2")),
				FName(TEXT("Spacer_WeaponRailGap2")),
				FName(TEXT("SizeBox_WeaponRail3")),
				FName(TEXT("Text_WeaponRail3"))
			};
			for (const FName RequiredWeaponResourceSlot : RequiredWeaponResourceSlots)
			{
				if (!WidgetTree->FindWidget(RequiredWeaponResourceSlot))
				{
					OutFailureReason = FString::Printf(TEXT("WeaponPanel Stage B semantic slot is missing: %s"), *RequiredWeaponResourceSlot.ToString());
					return false;
				}
			}

			// [v1.6.0] Stage B에서 새 의미 슬롯과 동시에 존재하면 중복 렌더링이 되는 구형 전용 Row/Widget 이름입니다.
			const FName ForbiddenLegacyWeaponResourceSlots[] =
			{
				FName(TEXT("HorizontalBox_LauncherSequence")),
				FName(TEXT("Text_WeaponLauncherSequence")),
				FName(TEXT("ProgressBar_LauncherSequence")),
				FName(TEXT("HorizontalBox_Ammo")),
				FName(TEXT("Image_Ammo")),
				FName(TEXT("Text_WeaponAmmo")),
				FName(TEXT("HorizontalBox_Heat")),
				FName(TEXT("Image_Heat")),
				FName(TEXT("Text_WeaponHeat")),
				FName(TEXT("ProgressBar_Heat")),
				FName(TEXT("HorizontalBox_Cooldown")),
				FName(TEXT("Image_Cooldown")),
				FName(TEXT("Text_WeaponCooldown")),
				FName(TEXT("ProgressBar_Cooldown"))
			};
			for (const FName ForbiddenLegacyWeaponResourceSlot : ForbiddenLegacyWeaponResourceSlots)
			{
				if (WidgetTree->FindWidget(ForbiddenLegacyWeaponResourceSlot))
				{
					OutFailureReason = FString::Printf(TEXT("WeaponPanel legacy fixed resource slot is forbidden after Stage B: %s"), *ForbiddenLegacyWeaponResourceSlot.ToString());
					return false;
				}
			}

			// [v1.6.0] Header Reserve는 label-less 우측 owner 하나만 유지하고 별도 Label/고정 Gap을 만들지 않습니다.
			if (WidgetTree->FindWidget(FName(TEXT("Text_WeaponReserveLabel")))
				|| WidgetTree->FindWidget(FName(TEXT("Spacer_WeaponReserveGap"))))
			{
				OutFailureReason = TEXT("WeaponPanel label-less Reserve semantic/right-edge slot contract failed");
				return false;
			}

						// [v1.8.0] 구형 Rail의 Turret/Ammo/Reload semantic Image는 실제 weapon identity가 아니므로 새 Text Rail과 공존할 수 없습니다.
			const FName ForbiddenLegacyWeaponRailImages[] =
			{
				FName(TEXT("Image_WeaponRail1")),
				FName(TEXT("Image_WeaponRail2")),
				FName(TEXT("Image_WeaponRail3"))
			};
			for (const FName ForbiddenLegacyWeaponRailImage : ForbiddenLegacyWeaponRailImages)
			{
				if (WidgetTree->FindWidget(ForbiddenLegacyWeaponRailImage))
				{
					OutFailureReason = FString::Printf(TEXT("WeaponPanel fake semantic Rail image is forbidden: %s"), *ForbiddenLegacyWeaponRailImage.ToString());
					return false;
				}
			}

			// [v1.8.0] Saved Designer는 Runtime 무기 목록을 가정하지 않으므로 Rail은 기본 Collapsed이고 Presenter만 actual nonselected selection에서 엽니다.
			const UWidget* WeaponRail = WidgetTree->FindWidget(FName(TEXT("HorizontalBox_WeaponRail")));
			if (!WeaponRail || WeaponRail->GetVisibility() != ESlateVisibility::Collapsed)
			{
				OutFailureReason = TEXT("WeaponPanel truthful Rail must be Designer-default Collapsed and runtime-presented only");
				return false;
			}
		}

						if (WidgetRole == FName(TEXT("SpeedGauge")))
		{
			// [v1.11.0] SpeedGauge에서 실제 표시해야 하는 단일 Gear Slot Text입니다.
			const UTextBlock* GearText = Cast<UTextBlock>(WidgetTree->FindWidget(FName(TEXT("Text_Gear"))));
			// [v1.11.0] Track·Tick·Red Zone을 모두 Material이 표현하는 단일 RPM Gauge Image입니다.
			const UImage* RPMGaugeImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RPMGauge"))));
			// [v1.11.0] RPM Gauge Brush에 실제 Material Resource가 연결됐는지 나타냅니다.
			const UMaterialInterface* RPMGaugeMaterial = RPMGaugeImage
				? Cast<UMaterialInterface>(RPMGaugeImage->GetBrush().GetResourceObject())
				: nullptr;
			// [v1.11.0] 구형 ProgressBar RPM Tick이 한 개라도 남아 있는지 세는 회귀 검출 값입니다.
			int32 LegacyRPMTickCount = 0;
			for (const UWidget* Widget : AllWidgets)
			{
				if (Widget && Widget->GetName().StartsWith(TEXT("ProgressBar_RPMTick")))
				{
					++LegacyRPMTickCount;
				}
			}

			if (!GearText || GearText->GetText().ToString() == TEXT("D")
				|| WidgetTree->FindWidget(FName(TEXT("ProgressBar_SpeedFallback")))
				|| WidgetTree->FindWidget(FName(TEXT("Image_SpeedArcArt")))
				|| WidgetTree->FindWidget(FName(TEXT("Image_RPMTrackArt")))
				|| !RPMGaugeImage
				|| !RPMGaugeMaterial
				|| RPMGaugeImage->GetVisibility() == ESlateVisibility::Collapsed
				|| LegacyRPMTickCount != 0)
			{
				OutFailureReason = FString::Printf(
					TEXT("SpeedGauge dynamic material contract failed: gear=%s legacy_rpm_ticks=%d material=%s"),
					GearText ? *GearText->GetText().ToString() : TEXT("missing"),
					LegacyRPMTickCount,
					RPMGaugeMaterial ? *RPMGaugeMaterial->GetPathName() : TEXT("missing"));
				return false;
			}
		}

				if (WidgetRole == FName(TEXT("ArmorSector")))
		{
			// [v1.10.0] 재사용 ArmorSector가 반드시 소유하는 실제 그림, 보조 방향 Label과 실제 Armor Ratio ProgressBar입니다.
			const UImage* ArmorImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_ArmorPlate"))));
			const UTextBlock* DirectionText = Cast<UTextBlock>(WidgetTree->FindWidget(FName(TEXT("Text_Direction"))));
			const UProgressBar* ArmorProgress = Cast<UProgressBar>(WidgetTree->FindWidget(FName(TEXT("ProgressBar_Armor"))));
			if (!ArmorImage || !DirectionText || !ArmorProgress)
			{
				OutFailureReason = TEXT("ArmorSector Image/Text/real Armor Progress contract failed");
				return false;
			}
		}

		if (WidgetRole == FName(TEXT("ArmorBodyMap")))
		{
			if (!ArmorSectorClass || !ArmorSectorClass->IsChildOf(UCFArmorSectorWidget::StaticClass()))
			{
				OutFailureReason = TEXT("ArmorBodyMap ArmorSector class contract failed");
				return false;
			}

			// [v1.10.0] BodyMap이 직접 소유하지 않아야 하는 구형 방향별 Image/Text/ProgressBar 이름입니다.
			const FName ForbiddenLegacyArmorWidgets[] =
			{
				FName(TEXT("Image_ArmorFront")), FName(TEXT("Image_ArmorRight")), FName(TEXT("Image_ArmorRear")),
				FName(TEXT("Image_ArmorLeft")), FName(TEXT("Image_ArmorTop")), FName(TEXT("Image_ArmorBottom")),
				FName(TEXT("Text_ArmorFrontLabel")), FName(TEXT("Text_ArmorRightLabel")), FName(TEXT("Text_ArmorRearLabel")),
				FName(TEXT("Text_ArmorLeftLabel")), FName(TEXT("Text_ArmorTopLabel")), FName(TEXT("Text_ArmorBottomLabel")),
				FName(TEXT("ProgressBar_ArmorFront")), FName(TEXT("ProgressBar_ArmorRight")), FName(TEXT("ProgressBar_ArmorRear")),
				FName(TEXT("ProgressBar_ArmorLeft")), FName(TEXT("ProgressBar_ArmorTop")), FName(TEXT("ProgressBar_ArmorBottom"))
			};
			for (const FName ForbiddenLegacyArmorWidget : ForbiddenLegacyArmorWidgets)
			{
				if (WidgetTree->FindWidget(ForbiddenLegacyArmorWidget))
				{
					OutFailureReason = FString::Printf(TEXT("ArmorBodyMap direct legacy direction widget is forbidden: %s"), *ForbiddenLegacyArmorWidget.ToString());
					return false;
				}
			}

			// [v1.10.0] 여섯 재사용 Sector 인스턴스와 각 인스턴스가 보존해야 하는 차량 로컬 방향 Label입니다.
			const TPair<FName, FString> RequiredArmorSectors[] =
			{
				{FName(TEXT("WBP_ArmorFront")), TEXT("FRONT")},
				{FName(TEXT("WBP_ArmorRight")), TEXT("RIGHT")},
				{FName(TEXT("WBP_ArmorRear")), TEXT("REAR")},
				{FName(TEXT("WBP_ArmorLeft")), TEXT("LEFT")},
				{FName(TEXT("WBP_ArmorTop")), TEXT("TOP")},
				{FName(TEXT("WBP_ArmorBottom")), TEXT("BOTTOM")}
			};
			for (const TPair<FName, FString>& RequiredArmorSector : RequiredArmorSectors)
			{
				// [v1.10.0] 현재 방향에 배치된 실제 재사용 ArmorSector 인스턴스입니다.
				const UCFArmorSectorWidget* ArmorSector = Cast<UCFArmorSectorWidget>(WidgetTree->FindWidget(RequiredArmorSector.Key));
				if (!ArmorSector
					|| ArmorSector->GetClass() != ArmorSectorClass
					|| ArmorSector->GetConfiguredDirectionLabel().ToString() != RequiredArmorSector.Value)
				{
					OutFailureReason = FString::Printf(TEXT("ArmorBodyMap reusable Sector contract failed: %s"), *RequiredArmorSector.Key.ToString());
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

		if (WidgetRole == FName(TEXT("RadarPanel")))
		{
			// [v1.13.0] UI-P0-08B Radar presentation이 저장 Designer Tree에 반드시 소유해야 하는 의미 Widget입니다.
			const FName RequiredRadarImageNames[] =
			{
				FName(TEXT("Image_RadarHeader")),
				FName(TEXT("Image_RadarFrame")),
				FName(TEXT("Image_RadarFriendly")),
				FName(TEXT("Image_RadarNeutral")),
				FName(TEXT("Image_RadarHostile")),
				FName(TEXT("Image_RadarUnknown")),
				FName(TEXT("Image_RadarPlayer")),
				FName(TEXT("Image_RadarSelected")),
				FName(TEXT("Image_RadarSelectedEdge"))
			};
			for (const FName RequiredRadarImageName : RequiredRadarImageNames)
			{
				if (!Cast<UImage>(WidgetTree->FindWidget(RequiredRadarImageName)))
				{
					OutFailureReason = FString::Printf(TEXT("RadarPanel required Image is missing: %s"), *RequiredRadarImageName.ToString());
					return false;
				}
			}

			if (!Cast<UCanvasPanel>(WidgetTree->FindWidget(FName(TEXT("CanvasPanel_RadarContacts"))))
				|| !Cast<UTextBlock>(WidgetTree->FindWidget(FName(TEXT("Text_RadarRange")))))
			{
				OutFailureReason = TEXT("RadarPanel Contact Canvas or Range Text semantic slot is missing");
				return false;
			}

			for (const UWidget* Widget : AllWidgets)
			{
				if (Widget && Widget->GetName().StartsWith(TEXT("Text_RadarRuntimeContact_")))
				{
					OutFailureReason = TEXT("RadarPanel Text glyph Contact is forbidden; runtime Contact must use Image visual art");
					return false;
				}
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

		// [v1.12.0] Root 의미 Widget이 실제 Canvas Slot에 속하는지만 검증합니다. Position/Size/Anchor/Alignment/ZOrder/AutoSize는 persisted Designer Layout이 소유합니다.
	bool ValidateRootCanvasSlot(const UWidget* Widget, FString& OutFailureReason)
	{
		// [v1.12.0] 의미 Widget이 Root Canvas의 실제 Child인지 확인할 Slot입니다.
		const UCanvasPanelSlot* CanvasSlot = Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
		if (!CanvasSlot)
		{
			OutFailureReason = FString::Printf(TEXT("Root Canvas slot missing: %s"), Widget ? *Widget->GetName() : TEXT("null"));
			return false;
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
	UObject* ArmorBodyMapBlueprintObject,
	UObject* ArmorSectorBlueprintObject)
{
#if WITH_EDITOR
	// [v1.0.0] Production Build 선행 조건과 Class 해석의 상세 실패 사유입니다.
	FString FailureReason;
	if (!CFUIHUDProdEditorBridge::ValidateInputData(LayoutData, StyleData, StandardDensityData, CompactDensityData, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
		// [v1.10.0] Python에서 전달된 실제 Production Child Widget Blueprint입니다.
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	// [v1.10.0] 재사용 ArmorSector만 전용 Native Presentation Base를 사용하고 다른 Production 역할은 기존 CFStyledWidgetBase를 그대로 사용합니다.
	UClass* ExpectedParentClass = WidgetRole == FName(TEXT("ArmorSector"))
		? UCFArmorSectorWidget::StaticClass()
		: UCFStyledWidgetBase::StaticClass();
	if (!WidgetBlueprint || WidgetBlueprint->ParentClass != ExpectedParentClass)
	{
		return CFUIHUDProdEditorBridge::Fail(FString::Printf(TEXT("Production child Widget Blueprint parent mismatch: role=%s"), *WidgetRole.ToString()));
	}

	UClass* SpeedGaugeClass = nullptr;
	UClass* ArmorBodyMapClass = nullptr;
	UClass* ArmorSectorClass = nullptr;
	if (WidgetRole == FName(TEXT("ArmorBodyMap")))
	{
		ArmorSectorClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(ArmorSectorBlueprintObject, TEXT("ArmorSector"), FailureReason);
		if (!ArmorSectorClass || !ArmorSectorClass->IsChildOf(UCFArmorSectorWidget::StaticClass()))
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason.IsEmpty() ? TEXT("ArmorBodyMap requires ArmorSector generated class") : FailureReason);
		}
	}
	if (WidgetRole == FName(TEXT("VehiclePanel")))
	{
		SpeedGaugeClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(SpeedGaugeBlueprintObject, TEXT("SpeedGauge"), FailureReason);
		ArmorBodyMapClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(ArmorBodyMapBlueprintObject, TEXT("ArmorBodyMap"), FailureReason);
		if (!SpeedGaugeClass || !ArmorBodyMapClass)
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason);
		}
	}

		// [v1.12.0] 저장된 Designer Tree는 Layout SSOT이므로 Build 경로가 기존 RootWidget을 교체하지 못하게 fail-closed합니다.
	if (WidgetBlueprint->WidgetTree && WidgetBlueprint->WidgetTree->RootWidget)
	{
		return CFUIHUDProdEditorBridge::Fail(FString::Printf(TEXT("Production Widget Build is scaffold-only; existing Designer Tree must use Validate: role=%s"), *WidgetRole.ToString()));
	}

	UWidgetTree* WidgetTree = CFUIHUDProdEditorBridge::ReplaceWidgetTree(WidgetBlueprint, FailureReason);
	if (!WidgetTree || !CFUIHUDProdEditorBridge::BuildRoleTree(WidgetTree, WidgetRole, LayoutData, StyleData, HUDVisualData, SpeedGaugeClass, ArmorBodyMapClass, ArmorSectorClass, FailureReason))

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
	UObject* ArmorBodyMapBlueprintObject,
	UObject* ArmorSectorBlueprintObject)
{
#if WITH_EDITOR
	// [v1.0.0] Production Readback 검증의 상세 실패 사유입니다.
	FString FailureReason;
	if (!CFUIHUDProdEditorBridge::ValidateInputData(LayoutData, StyleData, StandardDensityData, CompactDensityData, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
		const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	// [v1.10.0] 재사용 ArmorSector만 전용 Native Presentation Base를 사용하고 나머지는 기존 CFStyledWidgetBase Parent 계약을 유지합니다.
	UClass* ExpectedParentClass = WidgetRole == FName(TEXT("ArmorSector"))
		? UCFArmorSectorWidget::StaticClass()
		: UCFStyledWidgetBase::StaticClass();
		if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->ParentClass != ExpectedParentClass)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production child Blueprint/Tree/Parent validation failed"));
	}
	if (WidgetRole == FName(TEXT("RadarPanel")) && (!HUDVisualData || !HUDVisualData->HasCompleteRadarPresentationArt()))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("RadarPanel requires complete UI-P0-08B Radar presentation art before validation"));
	}
	UClass* SpeedGaugeClass = nullptr;
	UClass* ArmorBodyMapClass = nullptr;
	UClass* ArmorSectorClass = nullptr;
	if (WidgetRole == FName(TEXT("ArmorBodyMap")))
	{
		ArmorSectorClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(ArmorSectorBlueprintObject, TEXT("ArmorSector"), FailureReason);
		if (!ArmorSectorClass || !ArmorSectorClass->IsChildOf(UCFArmorSectorWidget::StaticClass()))
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason.IsEmpty() ? TEXT("ArmorBodyMap requires ArmorSector generated class") : FailureReason);
		}
	}
	if (WidgetRole == FName(TEXT("VehiclePanel")))
	{
		SpeedGaugeClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(SpeedGaugeBlueprintObject, TEXT("SpeedGauge"), FailureReason);
		ArmorBodyMapClass = CFUIHUDProdEditorBridge::ResolveGeneratedWidgetClass(ArmorBodyMapBlueprintObject, TEXT("ArmorBodyMap"), FailureReason);
		if (!SpeedGaugeClass || !ArmorBodyMapClass)
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason);
		}
	}
		if (!CFUIHUDProdEditorBridge::ValidateRoleTree(WidgetBlueprint->WidgetTree, WidgetRole, SpeedGaugeClass, ArmorBodyMapClass, ArmorSectorClass, FailureReason)
		|| !CFUIHUDProdEditorBridge::ValidateGraphHasNoRuntimeLogic(WidgetBlueprint, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
		return true;
#else
	return false;
#endif
}

// [v1.13.0] 저장 RadarPanel의 기존 Designer Layout을 보존하면서 UI-P0-08B 정적 Visual Widget과 Texture Brush만 additive 갱신합니다.
bool UCFUIHUDProdEditorBridge::ApplyRadarVisualMigrationResult(
	UObject* RadarPanelBlueprintObject,
	UCFHUDLayoutData* LayoutData,
	UCFUIStyleData* StyleData,
	UCFUIDensityData* StandardDensityData,
	UCFUIDensityData* CompactDensityData,
	UCFHUDVisualData* HUDVisualData)
{
#if WITH_EDITOR
	// [v1.13.0] Radar targeted migration의 입력/구조 실패 사유입니다.
	FString FailureReason;
	if (!CFUIHUDProdEditorBridge::ValidateInputData(LayoutData, StyleData, StandardDensityData, CompactDensityData, FailureReason))
	{
		return CFUIHUDProdEditorBridge::Fail(FailureReason);
	}
	if (!HUDVisualData || !HUDVisualData->HasCompleteRadarPresentationArt())
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar visual migration requires complete Radar presentation art"));
	}

	// [v1.13.0] 기존 persisted Designer Tree를 직접 보존 갱신할 정확한 RadarPanel Widget Blueprint입니다.
	UWidgetBlueprint* RadarPanelBlueprint = Cast<UWidgetBlueprint>(RadarPanelBlueprintObject);
	if (!RadarPanelBlueprint || RadarPanelBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass()
		|| !RadarPanelBlueprint->WidgetTree || !RadarPanelBlueprint->WidgetTree->RootWidget)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar visual migration requires an existing CFStyledWidgetBase RadarPanel Designer Tree"));
	}

	// [v1.13.0] 기존 RadarPanel WidgetTree이며 이 객체 자체를 교체하지 않습니다.
	UWidgetTree* WidgetTree = RadarPanelBlueprint->WidgetTree;
	// [v1.13.0] 기존 Contact Preview와 runtime Contact가 같은 좌표계를 공유하는 Designer-owned Canvas입니다.
	UCanvasPanel* RadarCanvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(FName(TEXT("CanvasPanel_RadarContacts"))));
	if (!RadarCanvas)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar visual migration requires existing CanvasPanel_RadarContacts"));
	}

	RadarPanelBlueprint->Modify();
	WidgetTree->Modify();
	RadarCanvas->Modify();

	// [v1.13.0] Soft Radar Texture를 실제 Image Brush에 연결하고 Style Token 색을 적용하는 layout-neutral helper입니다.
	auto ApplyRadarTexture = [StyleData](UImage* Image, const TSoftObjectPtr<UTexture2D>& TextureReference, const ECFUIColorToken ColorToken) -> bool
	{
		if (!Image || TextureReference.IsNull())
		{
			return false;
		}
		// [v1.13.0] 현재 Radar VisualData Soft Reference에서 동기 로드한 전용 UI Texture입니다.
		UTexture2D* Texture = TextureReference.LoadSynchronous();
		if (!Texture)
		{
			return false;
		}
		Image->Modify();
		Image->SetBrushFromTexture(Texture, false);
		Image->SetColorAndOpacity(StyleData->ResolveColor(ColorToken));
		return true;
	};

	// [v1.13.0] 기존 Designer Preview의 Friendly 원형 Blip Template입니다.
	UImage* FriendlyImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarFriendly"))));
	// [v1.13.0] 기존 Designer Preview의 Neutral 마름모 Blip Template입니다.
	UImage* NeutralImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarNeutral"))));
	// [v1.13.0] 기존 Designer Preview의 Hostile 하향 삼각형 Blip Template입니다.
	UImage* HostileImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarHostile"))));
	// [v1.13.0] 기존 Designer Preview의 Unknown 마름모 Blip Template입니다.
	UImage* UnknownImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarUnknown"))));
	// [v1.13.0] 기존 in-range 선택 Contact의 4-Corner Bracket Template입니다.
	UImage* SelectedImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarSelected"))));
	if (!FriendlyImage || !NeutralImage || !HostileImage || !UnknownImage || !SelectedImage)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar visual migration requires existing Friendly/Neutral/Hostile/Unknown/Selected Image templates"));
	}

	if (!ApplyRadarTexture(FriendlyImage, HUDVisualData->RadarFriendlyBlip, ECFUIColorToken::Friendly)
		|| !ApplyRadarTexture(NeutralImage, HUDVisualData->RadarUnknownBlip, ECFUIColorToken::Neutral)
		|| !ApplyRadarTexture(HostileImage, HUDVisualData->RadarHostileBlip, ECFUIColorToken::Hostile)
		|| !ApplyRadarTexture(UnknownImage, HUDVisualData->RadarUnknownBlip, ECFUIColorToken::Unknown)
		|| !ApplyRadarTexture(SelectedImage, HUDVisualData->SelectedTargetBracket, ECFUIColorToken::AccentTactical))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar visual migration failed to apply existing Contact/Selected Texture brushes"));
	}

	// [v1.13.0] 이번 실행에서 새 Designer 의미 Widget을 실제로 추가했는지 나타냅니다.
	bool bAddedNewRadarWidget = false;

	// [v1.13.0] Grid·3 Ring·Frame을 표시하는 Radar 배경 Image입니다. 기존 Widget이면 Slot을 절대 수정하지 않습니다.
	UImage* RadarFrameImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarFrame"))));
	if (!RadarFrameImage)
	{
		RadarFrameImage = CFUIHUDProdEditorBridge::CreateWidget<UImage>(WidgetTree, TEXT("Image_RadarFrame"));
		// [v1.13.0] 새 Frame이 현재 Radar Canvas 전체를 최초로 채우도록 설정할 Canvas Slot입니다.
		UCanvasPanelSlot* RadarFrameSlot = RadarFrameImage ? RadarCanvas->AddChildToCanvas(RadarFrameImage) : nullptr;
		if (!RadarFrameSlot)
		{
			return CFUIHUDProdEditorBridge::Fail(TEXT("Radar Frame additive Widget creation failed"));
		}
		RadarFrameSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		RadarFrameSlot->SetOffsets(FMargin(0.0f));
		RadarFrameSlot->SetAlignment(FVector2D::ZeroVector);
		RadarFrameSlot->SetZOrder(0);
		RadarFrameSlot->SetAutoSize(false);
		bAddedNewRadarWidget = true;
	}
	if (!ApplyRadarTexture(RadarFrameImage, HUDVisualData->RadarFrame, ECFUIColorToken::AccentTactical))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar Frame Texture apply failed"));
	}

	// [v1.13.0] Heading-Up Radar 중앙에 고정되는 Player 상향 삼각형 Image입니다. 기존 Widget이면 Slot을 보존합니다.
	UImage* RadarPlayerImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarPlayer"))));
	if (!RadarPlayerImage)
	{
		RadarPlayerImage = CFUIHUDProdEditorBridge::CreateWidget<UImage>(WidgetTree, TEXT("Image_RadarPlayer"));
		// [v1.13.0] 새 Player Marker의 최초 중앙 배치만 정의하는 Canvas Slot입니다.
		UCanvasPanelSlot* RadarPlayerSlot = RadarPlayerImage ? RadarCanvas->AddChildToCanvas(RadarPlayerImage) : nullptr;
		if (!RadarPlayerSlot)
		{
			return CFUIHUDProdEditorBridge::Fail(TEXT("Radar Player additive Widget creation failed"));
		}
		RadarPlayerSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		RadarPlayerSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		RadarPlayerSlot->SetPosition(FVector2D::ZeroVector);
		RadarPlayerSlot->SetSize(FVector2D(20.0f, 20.0f));
		RadarPlayerSlot->SetZOrder(2);
		RadarPlayerSlot->SetAutoSize(false);
		bAddedNewRadarWidget = true;
	}
	if (!ApplyRadarTexture(RadarPlayerImage, HUDVisualData->RadarPlayerMarker, ECFUIColorToken::AccentTactical))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar Player Texture apply failed"));
	}

	// [v1.13.0] 표시 Range 밖 선택 Contact의 방향을 나타내는 2-Corner Open Edge Bracket Image입니다. 기존 Widget이면 Slot을 보존합니다.
	UImage* SelectedEdgeImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_RadarSelectedEdge"))));
	if (!SelectedEdgeImage)
	{
		SelectedEdgeImage = CFUIHUDProdEditorBridge::CreateWidget<UImage>(WidgetTree, TEXT("Image_RadarSelectedEdge"));
		// [v1.13.0] 새 Edge Bracket의 최초 중앙 대기 위치와 크기만 정의하는 Canvas Slot입니다.
		UCanvasPanelSlot* SelectedEdgeSlot = SelectedEdgeImage ? RadarCanvas->AddChildToCanvas(SelectedEdgeImage) : nullptr;
		if (!SelectedEdgeSlot)
		{
			return CFUIHUDProdEditorBridge::Fail(TEXT("Radar Selected Edge additive Widget creation failed"));
		}
		SelectedEdgeSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		SelectedEdgeSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		SelectedEdgeSlot->SetPosition(FVector2D::ZeroVector);
		SelectedEdgeSlot->SetSize(FVector2D(40.0f, 40.0f));
		SelectedEdgeSlot->SetZOrder(3);
		SelectedEdgeSlot->SetAutoSize(false);
		SelectedEdgeImage->SetVisibility(ESlateVisibility::Collapsed);
		bAddedNewRadarWidget = true;
	}
	if (!ApplyRadarTexture(SelectedEdgeImage, HUDVisualData->RadarSelectedEdgeBracket, ECFUIColorToken::AccentTactical))
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Radar Selected Edge Texture apply failed"));
	}

	// [v1.13.0] 현재 단계식 Display Range를 표시하는 Text 슬롯입니다. 기존 Widget이면 Designer Layout을 보존합니다.
	UTextBlock* RadarRangeText = Cast<UTextBlock>(WidgetTree->FindWidget(FName(TEXT("Text_RadarRange"))));
	if (!RadarRangeText)
	{
		RadarRangeText = CFUIHUDProdEditorBridge::CreateText(
			WidgetTree,
			TEXT("Text_RadarRange"),
			TEXT("RANGE —"),
			StyleData,
			LayoutData,
			ECFUIFontFamilyRole::Numeric,
			ECFUITypographyRole::Caption,
			ECFUIColorToken::TextSecondary);
		// [v1.13.0] 새 Range Text의 최초 우상단 배치만 정의하는 Canvas Slot입니다.
		UCanvasPanelSlot* RadarRangeSlot = RadarRangeText ? RadarCanvas->AddChildToCanvas(RadarRangeText) : nullptr;
		if (!RadarRangeSlot)
		{
			return CFUIHUDProdEditorBridge::Fail(TEXT("Radar Range Text additive Widget creation failed"));
		}
		RadarRangeSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		RadarRangeSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		RadarRangeSlot->SetPosition(FVector2D(-8.0f, 8.0f));
		RadarRangeSlot->SetZOrder(4);
		RadarRangeSlot->SetAutoSize(true);
		bAddedNewRadarWidget = true;
	}
	CFUIHUDProdEditorBridge::ApplyTextStyle(RadarRangeText, StyleData, LayoutData, ECFUIFontFamilyRole::Numeric, ECFUITypographyRole::Caption, ECFUIColorToken::TextSecondary);

	if (bAddedNewRadarWidget)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(RadarPanelBlueprint);
	}
	else
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(RadarPanelBlueprint);
	}
	return true;
#else
	return false;
#endif
}

// [v1.14.0] 저장 Production Root의 기존 7-child Layout을 보존하면서 ReticleLayer 내부에 Vehicle Direction 의미 Widget만 additive 추가합니다.
bool UCFUIHUDProdEditorBridge::ApplyViewModeVisualMigrationResult(
	UObject* RootWidgetBlueprintObject,
	UCFUIStyleData* StyleData)
{
#if WITH_EDITOR
	if (!StyleData)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("ViewMode visual migration requires UI Style Data"));
	}

	// [v1.14.0] 기존 persisted Designer Tree를 직접 보존 갱신할 정확한 Production Root Widget Blueprint입니다.
	UWidgetBlueprint* RootWidgetBlueprint = Cast<UWidgetBlueprint>(RootWidgetBlueprintObject);
	if (!RootWidgetBlueprint || RootWidgetBlueprint->ParentClass != UCFStyledWidgetBase::StaticClass()
		|| !RootWidgetBlueprint->WidgetTree || !RootWidgetBlueprint->WidgetTree->RootWidget)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("ViewMode visual migration requires an existing CFStyledWidgetBase Production Root Designer Tree"));
	}

	// [v1.14.0] 기존 Root WidgetTree이며 이 객체 자체를 교체하지 않습니다.
	UWidgetTree* WidgetTree = RootWidgetBlueprint->WidgetTree;
	// [v1.14.0] D1-07부터 예약돼 있던 기존 full-screen Reticle presentation Canvas입니다.
	UCanvasPanel* ReticleLayer = Cast<UCanvasPanel>(WidgetTree->FindWidget(FName(TEXT("CanvasPanel_Slot_ReticleLayer"))));
	if (!ReticleLayer)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("ViewMode visual migration requires existing CanvasPanel_Slot_ReticleLayer"));
	}

	// [v1.14.0] 차체 방향 아이콘의 Designer-owned 이동 범위를 정의하는 비시각 Canvas Track입니다.
	UCanvasPanel* ViewDirectionCanvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(FName(TEXT("CanvasPanel_ViewDirection"))));
	// [v1.14.0] 차체 진행 방향을 카메라 기준 좌우 위치로 표시할 기존 Vehicle Semantic Image입니다.
	UImage* VehicleDirectionImage = Cast<UImage>(WidgetTree->FindWidget(FName(TEXT("Image_ViewVehicleDirection"))));
	// [v1.14.0] 이번 실행에서 실제로 새 Designer 의미 Widget을 추가했는지 나타냅니다.
	bool bAddedViewModeWidget = false;

	if (!ViewDirectionCanvas)
	{
		ViewDirectionCanvas = CFUIHUDProdEditorBridge::CreateWidget<UCanvasPanel>(WidgetTree, TEXT("CanvasPanel_ViewDirection"));
		// [v1.14.0] 화면 중앙 하단 쪽의 20% 폭을 최초 Track으로 제안하되 이후 UMG Designer가 소유할 ReticleLayer Slot입니다.
		UCanvasPanelSlot* ViewDirectionCanvasSlot = ViewDirectionCanvas ? ReticleLayer->AddChildToCanvas(ViewDirectionCanvas) : nullptr;
		if (!ViewDirectionCanvasSlot)
		{
			return CFUIHUDProdEditorBridge::Fail(TEXT("ViewMode Direction Track additive Widget creation failed"));
		}
		ViewDirectionCanvasSlot->SetAnchors(FAnchors(0.40f, 0.66f, 0.60f, 0.66f));
		ViewDirectionCanvasSlot->SetOffsets(FMargin(0.0f, -18.0f, 0.0f, 36.0f));
		ViewDirectionCanvasSlot->SetAlignment(FVector2D::ZeroVector);
		ViewDirectionCanvasSlot->SetZOrder(1);
		ViewDirectionCanvasSlot->SetAutoSize(false);
		bAddedViewModeWidget = true;
	}
	else if (ViewDirectionCanvas->GetParent() != ReticleLayer)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("CanvasPanel_ViewDirection must remain a direct child of CanvasPanel_Slot_ReticleLayer"));
	}

	if (!VehicleDirectionImage)
	{
		VehicleDirectionImage = CFUIHUDProdEditorBridge::CreateImage(
			WidgetTree,
			TEXT("Image_ViewVehicleDirection"),
			StyleData,
			FName(TEXT("Vehicle")),
			ECFUIColorToken::TextSecondary);
		// [v1.14.0] Style IconLarge를 사용해 Track 중앙에 최초 대기시킬 Vehicle Direction Image Slot입니다.
		UCanvasPanelSlot* VehicleDirectionSlot = VehicleDirectionImage ? ViewDirectionCanvas->AddChildToCanvas(VehicleDirectionImage) : nullptr;
		if (!VehicleDirectionSlot)
		{
			return CFUIHUDProdEditorBridge::Fail(TEXT("ViewMode Vehicle Direction Image additive Widget creation failed"));
		}
		// [v1.14.0] Designer에서 편집 가능한 Style IconLarge 값으로 초기 정사각 표시 크기를 결정합니다.
		const float VehicleDirectionIconSize = FMath::Max(StyleData->IconSet.IconLarge, 1.0f);
		VehicleDirectionSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		VehicleDirectionSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		VehicleDirectionSlot->SetPosition(FVector2D::ZeroVector);
		VehicleDirectionSlot->SetSize(FVector2D(VehicleDirectionIconSize, VehicleDirectionIconSize));
		VehicleDirectionSlot->SetZOrder(1);
		VehicleDirectionSlot->SetAutoSize(false);
		VehicleDirectionImage->SetVisibility(ESlateVisibility::Collapsed);
		bAddedViewModeWidget = true;
	}
	else if (VehicleDirectionImage->GetParent() != ViewDirectionCanvas)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Image_ViewVehicleDirection must remain a direct child of CanvasPanel_ViewDirection"));
	}

	if (!VehicleDirectionImage->GetBrush().GetResourceObject())
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("ViewMode Vehicle Direction Image requires resolved Vehicle Semantic Icon brush"));
	}

	if (bAddedViewModeWidget)
	{
		RootWidgetBlueprint->Modify();
		WidgetTree->Modify();
		ReticleLayer->Modify();
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(RootWidgetBlueprint);
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

		// [v1.12.0] 저장된 Root Designer Tree는 Layout SSOT이므로 기존 RootWidget이 있으면 Scaffold Build를 차단합니다.
	if (RootBlueprint->WidgetTree && RootBlueprint->WidgetTree->RootWidget)
	{
		return CFUIHUDProdEditorBridge::Fail(TEXT("Production Root Build is scaffold-only; existing Designer Tree must use Validate"));
	}

	UWidgetTree* WidgetTree = CFUIHUDProdEditorBridge::ReplaceWidgetTree(RootBlueprint, FailureReason);
	// [v1.12.0] D1-07 값은 신규 Root의 최초 Scaffold 시작 배치에만 사용합니다.

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
			|| !CFUIHUDProdEditorBridge::ValidateRootCanvasSlot(SlotBox, FailureReason))
		{
			return CFUIHUDProdEditorBridge::Fail(FailureReason.IsEmpty() ? FString::Printf(TEXT("Production root semantic slot/class failed: %s"), Expected.SizeBoxName) : FailureReason);
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
