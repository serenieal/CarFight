// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.1
// Date: 2026-09-03
// Description: CF-FQ-041 Runtime Apply UI + CF-FQ-044 Runtime Catalog change-aware option synchronization 구현
// Scope: Asset 없는 최소 UI, Catalog 선택 상태, Current/Selected 분리, Runtime Apply service 위임과 Catalog cache sync를 구현합니다.
// Changelog:
// - v1.1.1: Mid-review P1. Catalog raw array에 null/invalid entry가 있어도 rebuild가 사용하는 valid-entry filtered sequence와 cache를 비교해 매 Tick 불필요 ClearOptions 반복을 방지.
// - v1.1.0: VRCP-P0-03. AllowedVehicleData/AllowedEquipmentPresetData exact sequence가 cached option과 실제 다를 때만 해당 options를 rebuild하고, Pawn VehicleData 변경의 Mount→Equipment rebuild를 먼저 처리해 same-refresh Equipment 중복 ClearOptions까지 방지.
// - v1.0.2: Applied Snapshot이 없는 Legacy Runtime에서도 VehicleWeaponComp의 실제 활성 장비 readback을 Current Equipment에 표시하도록 교정.
// - v1.0.1: 동일 VehicleData의 매 프레임 Refresh에서 Equipment Combo options를 재구성하지 않도록 교정해 드롭다운이 즉시 닫히는 UI lifecycle 버그를 수정.
// - v1.0.0: Vehicle/Mount/Equipment ComboBox, Explicit Apply 버튼, 호환성/Last Result readback과 C++ WidgetTree를 최초 구현.
// Migration:
// - v1.1.0부터 RefreshRuntimeApplyState는 Vehicle/Equipment Catalog exact sequence 변화만 감지해 선택을 보존한 bounded rebuild를 수행합니다. 변화가 없으면 ComboBox options를 건드리지 않습니다.
// - VehicleData에는 Player-facing DisplayName 계약이 없으므로 Debug UI에서 Asset UObject 이름을 사용합니다.
// - EquipmentPresetData는 기존 DisplayName 계약을 보존하며 빈 DisplayName을 EquipmentId/AssetName으로 대체하지 않습니다.
// - Runtime domain mutation은 FCFRuntimeVehicleApplyService / FCFRuntimeEquipApplyService만 수행합니다.

#include "UI/CFRuntimeApplyWidget.h"

#include "CFEquipmentPresetData.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFRuntimeTestSettings.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFVehicleWeaponTypes.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
	// [v1.0.0] Runtime Apply root에 공통 TextBlock 한 줄을 만들고 반환합니다.
	UTextBlock* AddRuntimeApplyText(
		UWidgetTree* WidgetTree,
		UVerticalBox* RootVerticalBox,
		const FName WidgetName,
		const FString& InitialText,
		const int32 FontSize,
		const FMargin& Padding)
	{
		if (!WidgetTree || !RootVerticalBox)
		{
			return nullptr;
		}

		// [v1.0.0] 현재 한 줄 상태/제목을 표시할 TextBlock입니다.
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			WidgetName);
		if (!TextBlock)
		{
			return nullptr;
		}

		TextBlock->SetText(FText::FromString(InitialText));
		TextBlock->SetAutoWrapText(true);
		TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);

		// [v1.0.0] 이 TextBlock의 용도에 맞춰 적용할 글꼴 설정입니다.
		FSlateFontInfo TextFont = TextBlock->GetFont();
		TextFont.Size = FontSize;
		TextBlock->SetFont(TextFont);

		// [v1.0.0] Runtime Apply root 안에서 이 TextBlock의 배치와 여백을 소유할 slot입니다.
		UVerticalBoxSlot* TextSlot = RootVerticalBox->AddChildToVerticalBox(TextBlock);
		if (TextSlot)
		{
			TextSlot->SetHorizontalAlignment(HAlign_Fill);
			TextSlot->SetPadding(Padding);
		}

		return TextBlock;
	}

	// [v1.0.0] Runtime Apply root에 공통 ComboBoxString 하나를 만들고 반환합니다.
	UComboBoxString* AddRuntimeApplyComboBox(
		UWidgetTree* WidgetTree,
		UVerticalBox* RootVerticalBox,
		const FName WidgetName)
	{
		if (!WidgetTree || !RootVerticalBox)
		{
			return nullptr;
		}

		// [v1.0.0] 현재 Vehicle/Mount/Equipment 후보를 선택할 ComboBox입니다.
		UComboBoxString* ComboBox = WidgetTree->ConstructWidget<UComboBoxString>(
			UComboBoxString::StaticClass(),
			WidgetName);
		if (!ComboBox)
		{
			return nullptr;
		}

		ComboBox->SetVisibility(ESlateVisibility::Visible);

		// [v1.0.0] ComboBox를 root 폭에 맞추고 위아래 여백을 줄 slot입니다.
		UVerticalBoxSlot* ComboBoxSlot = RootVerticalBox->AddChildToVerticalBox(ComboBox);
		if (ComboBoxSlot)
		{
			ComboBoxSlot->SetHorizontalAlignment(HAlign_Fill);
			ComboBoxSlot->SetPadding(FMargin(12.0f, 2.0f, 12.0f, 8.0f));
		}

		return ComboBox;
	}

	// [v1.0.0] Runtime Apply root에 명시 Apply 버튼과 버튼 문구를 만들고 반환합니다.
	UButton* AddRuntimeApplyButton(
		UWidgetTree* WidgetTree,
		UVerticalBox* RootVerticalBox,
		const FName ButtonName,
		const FName TextName,
		const FString& ButtonLabel)
	{
		if (!WidgetTree || !RootVerticalBox)
		{
			return nullptr;
		}

		// [v1.0.0] 사용자가 선택과 실제 적용을 분리해 명시적으로 누를 Apply 버튼입니다.
		UButton* ApplyButton = WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(),
			ButtonName);
		if (!ApplyButton)
		{
			return nullptr;
		}

		ApplyButton->SetVisibility(ESlateVisibility::Visible);

		// [v1.0.0] Apply 버튼 안에 표시할 동작 문구 TextBlock입니다.
		UTextBlock* ApplyButtonText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			TextName);
		if (!ApplyButtonText)
		{
			return nullptr;
		}

		ApplyButtonText->SetText(FText::FromString(ButtonLabel));
		ApplyButtonText->SetJustification(ETextJustify::Center);
		ApplyButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		ApplyButtonText->SetVisibility(ESlateVisibility::HitTestInvisible);

		// [v1.0.0] 명시 Apply 버튼 문구를 본문보다 조금 크게 보이게 할 글꼴 설정입니다.
		FSlateFontInfo ButtonFont = ApplyButtonText->GetFont();
		ButtonFont.Size = 18;
		ApplyButtonText->SetFont(ButtonFont);
		ApplyButton->AddChild(ApplyButtonText);

		// [v1.0.0] Apply 버튼 클릭 영역과 주변 간격을 확보할 root slot입니다.
		UVerticalBoxSlot* ButtonSlot = RootVerticalBox->AddChildToVerticalBox(ApplyButton);
		if (ButtonSlot)
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
			ButtonSlot->SetPadding(FMargin(12.0f, 2.0f, 12.0f, 14.0f));
		}

		return ApplyButton;
	}

	// [v1.1.1] RebuildVehicleOptions()가 실제 cache에 넣는 valid-entry sequence와 current Catalog를 동일 규칙으로 비교합니다.
	bool AreVehicleCatalogOptionsCurrent(
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		const TArray<TObjectPtr<UCFVehicleData>>& CachedVehicleOptions)
	{
		if (!IsValid(RuntimeCatalog))
		{
			return CachedVehicleOptions.IsEmpty();
		}

		// Raw Catalog의 null/invalid entry는 RebuildVehicleOptions()도 cache에서 제외하므로 비교에서도 제외합니다.
		int32 CachedVehicleIndex = 0;
		for (UCFVehicleData* VehicleData : RuntimeCatalog->AllowedVehicleData)
		{
			if (!IsValid(VehicleData))
			{
				continue;
			}

			if (!CachedVehicleOptions.IsValidIndex(CachedVehicleIndex)
				|| CachedVehicleOptions[CachedVehicleIndex].Get() != VehicleData)
			{
				return false;
			}
			++CachedVehicleIndex;
		}

		// Cached 쪽에 Catalog의 valid entry보다 남는 항목이 있으면 stale cache입니다.
		return CachedVehicleIndex == CachedVehicleOptions.Num();
	}

	// [v1.1.1] RebuildEquipmentOptions()가 실제 cache에 넣는 valid-entry sequence와 current Catalog를 동일 규칙으로 비교합니다.
	bool AreEquipmentCatalogOptionsCurrent(
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		const TArray<TObjectPtr<UCFEquipmentPresetData>>& CachedEquipmentOptions)
	{
		if (!IsValid(RuntimeCatalog))
		{
			return CachedEquipmentOptions.IsEmpty();
		}

		// Raw Catalog의 null/invalid entry는 RebuildEquipmentOptions()도 cache에서 제외하므로 비교에서도 제외합니다.
		int32 CachedEquipmentIndex = 0;
		for (UCFEquipmentPresetData* EquipmentPresetData : RuntimeCatalog->AllowedEquipmentPresetData)
		{
			if (!IsValid(EquipmentPresetData))
			{
				continue;
			}

			if (!CachedEquipmentOptions.IsValidIndex(CachedEquipmentIndex)
				|| CachedEquipmentOptions[CachedEquipmentIndex].Get() != EquipmentPresetData)
			{
				return false;
			}
			++CachedEquipmentIndex;
		}

		// Cached 쪽에 Catalog의 valid entry보다 남는 항목이 있으면 stale cache입니다.
		return CachedEquipmentIndex == CachedEquipmentOptions.Num();
	}
}

// [v1.0.0] Widget 초기화 시 C++ WidgetTree와 기본 Runtime Catalog를 준비합니다.
void UCFRuntimeApplyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureRuntimeApplyTree();
	LoadDefaultRuntimeCatalog();
	RefreshRuntimeApplyState();
}

// [v1.0.0] Widget 파괴 시 ComboBox/Button delegate 바인딩을 정리합니다.
void UCFRuntimeApplyWidget::NativeDestruct()
{
	if (VehicleComboBox)
	{
		VehicleComboBox->OnSelectionChanged.RemoveDynamic(
			this,
			&UCFRuntimeApplyWidget::HandleVehicleSelectionChanged);
	}

	if (MountComboBox)
	{
		MountComboBox->OnSelectionChanged.RemoveDynamic(
			this,
			&UCFRuntimeApplyWidget::HandleMountSelectionChanged);
	}

	if (EquipmentComboBox)
	{
		EquipmentComboBox->OnSelectionChanged.RemoveDynamic(
			this,
			&UCFRuntimeApplyWidget::HandleEquipmentSelectionChanged);
	}

	if (ApplyVehicleButton)
	{
		ApplyVehicleButton->OnClicked.RemoveDynamic(
			this,
			&UCFRuntimeApplyWidget::HandleApplyVehicleClicked);
	}

	if (ApplyEquipmentButton)
	{
		ApplyEquipmentButton->OnClicked.RemoveDynamic(
			this,
			&UCFRuntimeApplyWidget::HandleApplyEquipmentClicked);
	}

	Super::NativeDestruct();
}

// [v1.0.0] Runtime Apply 대상 차량 Pawn을 설정하고 Current 상태와 Mount 선택 목록을 갱신합니다.
void UCFRuntimeApplyWidget::SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)
{
	if (VehiclePawnRef == InVehiclePawnRef)
	{
		RefreshRuntimeApplyState();
		return;
	}

	VehiclePawnRef = InVehiclePawnRef;

	// [v1.0.0] 새 Pawn의 VehicleData를 기준으로 Mount option을 강제 재구성하기 위한 관측 캐시 초기화입니다.
	LastObservedVehicleData = nullptr;

	// [v1.0.0] 다른 Pawn에 이전 UI Apply의 transient Vehicle 표시 힌트가 잘못 남지 않도록 초기화합니다.
	LastAppliedVehicleSourceData = nullptr;
	LastAppliedVehicleRuntimeData = nullptr;

	RefreshRuntimeApplyState();
}

// [v1.0.0] 현재 Pawn/Catalog 상태를 읽어 표시 텍스트와 버튼 가능 상태를 갱신합니다.
void UCFRuntimeApplyWidget::RefreshRuntimeApplyState()
{
	EnsureRuntimeApplyTree();

	if (!RuntimeCatalog)
	{
		LoadDefaultRuntimeCatalog();
	}

	// [v1.1.0] Builder Catalog promotion/Undo/Redo 등으로 same object의 Vehicle hard-reference sequence가 달라졌을 때만 Vehicle options를 재구성합니다.
	if (!AreVehicleCatalogOptionsCurrent(RuntimeCatalog, VehicleOptionDataArray))
	{
		RebuildVehicleOptions();
	}

	// [v1.0.0] 현재 Pawn이 실제 사용하는 VehicleData이며 Mount option 변경 감지 기준입니다.
	UCFVehicleData* CurrentVehicleData =
		IsValid(VehiclePawnRef) ? VehiclePawnRef->VehicleData.Get() : nullptr;

	if (LastObservedVehicleData != CurrentVehicleData)
	{
		LastObservedVehicleData = CurrentVehicleData;
		// RebuildMountOptions()는 mount compatibility 표시를 위해 Equipment options도 한 번 재구성합니다.
		RebuildMountOptions();
	}

	// [v1.1.0] Mount rebuild가 이미 Equipment cache를 current Catalog로 맞춘 뒤에도 exact sequence가 다를 때만 추가 재구성합니다.
	// 이 순서로 Pawn VehicleData 변경과 Catalog Equipment 변경이 같은 refresh에 겹쳐도 Equipment ClearOptions가 중복 실행되지 않습니다.
	if (!AreEquipmentCatalogOptionsCurrent(RuntimeCatalog, EquipmentOptionDataArray))
	{
		RebuildEquipmentOptions();
	}

	// [v1.0.1] 같은 VehicleData의 일반 Tick Refresh에서는 Equipment options를 다시 만들지 않습니다.
	// Mount 선택 변경은 SelectMountByIndex()/HandleMountSelectionChanged()에서, VehicleData 변경은 RebuildMountOptions()에서만 재구성합니다.
	// UComboBoxString::ClearOptions()는 열린 popup을 닫으므로 매 프레임 rebuild하면 Equipment 드롭다운을 클릭해도 즉시 닫힙니다.
	RefreshDisplayTexts();
	RefreshApplyButtonStates();
}

// [v1.0.0] Asset 없이 사용할 최소 Runtime Apply WidgetTree를 중복 없이 생성하고 준비 여부를 반환합니다.
bool UCFRuntimeApplyWidget::EnsureRuntimeApplyTree()
{
	if (RootVerticalBox
		&& CatalogSummaryText
		&& CurrentVehicleText
		&& SelectedVehicleText
		&& VehicleComboBox
		&& ApplyVehicleButton
		&& CurrentMountText
		&& MountComboBox
		&& CurrentEquipmentText
		&& SelectedEquipmentText
		&& EquipmentComboBox
		&& EquipmentCompatibilityText
		&& ApplyEquipmentButton
		&& LastResultTextBlock)
	{
		return true;
	}

	if (!WidgetTree)
	{
		return false;
	}

	RootVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(),
		TEXT("VerticalBox_RuntimeApplyRoot"));
	if (!RootVerticalBox)
	{
		return false;
	}
	WidgetTree->RootWidget = RootVerticalBox;

	// [v1.0.0] Runtime Apply 전용 화면임을 표시하는 제목 Text입니다.
	UTextBlock* TitleText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_RuntimeApplyTitle"),
		TEXT("런타임 콘텐츠 적용"),
		24,
		FMargin(12.0f, 8.0f, 12.0f, 10.0f));

	CatalogSummaryText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_RuntimeCatalogSummary"),
		TEXT("Catalog: 로드 전"),
		14,
		FMargin(12.0f, 0.0f, 12.0f, 12.0f));

	CurrentVehicleText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_CurrentVehicle"),
		TEXT("현재 차량: 없음"),
		16,
		FMargin(12.0f, 2.0f, 12.0f, 2.0f));

	SelectedVehicleText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_SelectedVehicle"),
		TEXT("선택 차량: 없음"),
		16,
		FMargin(12.0f, 2.0f, 12.0f, 4.0f));

	VehicleComboBox = AddRuntimeApplyComboBox(
		WidgetTree,
		RootVerticalBox,
		TEXT("ComboBox_RuntimeVehicle"));

	ApplyVehicleButton = AddRuntimeApplyButton(
		WidgetTree,
		RootVerticalBox,
		TEXT("Button_ApplyRuntimeVehicle"),
		TEXT("Text_ApplyRuntimeVehicle"),
		TEXT("선택 차량 적용"));

	// [v1.0.0] 차량 적용 영역과 장비 적용 영역을 시각적으로 구분하는 단순 구분 Text입니다.
	UTextBlock* EquipmentHeaderText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_RuntimeEquipmentHeader"),
		TEXT("장비 슬롯 적용"),
		20,
		FMargin(12.0f, 8.0f, 12.0f, 8.0f));

	CurrentMountText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_CurrentMount"),
		TEXT("대상 Mount/Profile: 없음"),
		16,
		FMargin(12.0f, 2.0f, 12.0f, 4.0f));

	MountComboBox = AddRuntimeApplyComboBox(
		WidgetTree,
		RootVerticalBox,
		TEXT("ComboBox_RuntimeMount"));

	CurrentEquipmentText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_CurrentEquipment"),
		TEXT("현재 장비: 없음"),
		16,
		FMargin(12.0f, 2.0f, 12.0f, 2.0f));

	SelectedEquipmentText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_SelectedEquipment"),
		TEXT("선택 장비: 없음"),
		16,
		FMargin(12.0f, 2.0f, 12.0f, 4.0f));

	EquipmentComboBox = AddRuntimeApplyComboBox(
		WidgetTree,
		RootVerticalBox,
		TEXT("ComboBox_RuntimeEquipment"));

	EquipmentCompatibilityText = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_EquipmentCompatibility"),
		TEXT("호환성: 확인할 Mount/장비 없음"),
		14,
		FMargin(12.0f, 0.0f, 12.0f, 6.0f));

	ApplyEquipmentButton = AddRuntimeApplyButton(
		WidgetTree,
		RootVerticalBox,
		TEXT("Button_ApplyRuntimeEquipment"),
		TEXT("Text_ApplyRuntimeEquipment"),
		TEXT("선택 장비 적용"));

	LastResultTextBlock = AddRuntimeApplyText(
		WidgetTree,
		RootVerticalBox,
		TEXT("Text_RuntimeApplyLastResult"),
		TEXT("마지막 결과: 아직 적용하지 않음"),
		15,
		FMargin(12.0f, 10.0f, 12.0f, 12.0f));

	if (!TitleText
		|| !EquipmentHeaderText
		|| !CatalogSummaryText
		|| !CurrentVehicleText
		|| !SelectedVehicleText
		|| !VehicleComboBox
		|| !ApplyVehicleButton
		|| !CurrentMountText
		|| !MountComboBox
		|| !CurrentEquipmentText
		|| !SelectedEquipmentText
		|| !EquipmentComboBox
		|| !EquipmentCompatibilityText
		|| !ApplyEquipmentButton
		|| !LastResultTextBlock)
	{
		return false;
	}

	VehicleComboBox->OnSelectionChanged.RemoveDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleVehicleSelectionChanged);
	VehicleComboBox->OnSelectionChanged.AddDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleVehicleSelectionChanged);

	MountComboBox->OnSelectionChanged.RemoveDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleMountSelectionChanged);
	MountComboBox->OnSelectionChanged.AddDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleMountSelectionChanged);

	EquipmentComboBox->OnSelectionChanged.RemoveDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleEquipmentSelectionChanged);
	EquipmentComboBox->OnSelectionChanged.AddDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleEquipmentSelectionChanged);

	ApplyVehicleButton->OnClicked.RemoveDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleApplyVehicleClicked);
	ApplyVehicleButton->OnClicked.AddDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleApplyVehicleClicked);

	ApplyEquipmentButton->OnClicked.RemoveDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleApplyEquipmentClicked);
	ApplyEquipmentButton->OnClicked.AddDynamic(
		this,
		&UCFRuntimeApplyWidget::HandleApplyEquipmentClicked);

	return true;
}

// [v1.0.0] Catalog Vehicle 목록에서 index 항목을 Selected Vehicle로 지정합니다.
bool UCFRuntimeApplyWidget::SelectVehicleByIndex(const int32 VehicleIndex)
{
	if (!VehicleOptionDataArray.IsValidIndex(VehicleIndex)
		|| !IsValid(VehicleOptionDataArray[VehicleIndex]))
	{
		return false;
	}

	SelectedVehicleData = VehicleOptionDataArray[VehicleIndex];

	if (VehicleComboBox && VehicleOptionLabelArray.IsValidIndex(VehicleIndex))
	{
		VehicleComboBox->SetSelectedOption(VehicleOptionLabelArray[VehicleIndex]);
	}

	RefreshDisplayTexts();
	RefreshApplyButtonStates();
	return true;
}

// [v1.0.0] 현재 차량 MountProfiles에서 index 항목을 Target Mount/Profile로 지정합니다.
bool UCFRuntimeApplyWidget::SelectMountByIndex(const int32 MountIndex)
{
	if (!MountOptionIdArray.IsValidIndex(MountIndex)
		|| MountOptionIdArray[MountIndex].IsNone())
	{
		return false;
	}

	SelectedMountProfileId = MountOptionIdArray[MountIndex];

	if (MountComboBox && MountOptionLabelArray.IsValidIndex(MountIndex))
	{
		MountComboBox->SetSelectedOption(MountOptionLabelArray[MountIndex]);
	}

	RebuildEquipmentOptions();
	RefreshDisplayTexts();
	RefreshApplyButtonStates();
	return true;
}

// [v1.0.0] Catalog Equipment 목록에서 index 항목을 Selected Equipment로 지정합니다.
bool UCFRuntimeApplyWidget::SelectEquipmentByIndex(const int32 EquipmentIndex)
{
	if (!EquipmentOptionDataArray.IsValidIndex(EquipmentIndex)
		|| !IsValid(EquipmentOptionDataArray[EquipmentIndex]))
	{
		return false;
	}

	SelectedEquipmentData = EquipmentOptionDataArray[EquipmentIndex];

	if (EquipmentComboBox && EquipmentOptionLabelArray.IsValidIndex(EquipmentIndex))
	{
		EquipmentComboBox->SetSelectedOption(EquipmentOptionLabelArray[EquipmentIndex]);
	}

	RefreshDisplayTexts();
	RefreshApplyButtonStates();
	return true;
}

// [v1.0.0] Selected Vehicle을 현재 Pawn에 명시적으로 Runtime Apply합니다.
ECFRuntimeVehicleApplyStatus UCFRuntimeApplyWidget::ApplySelectedVehicle()
{
	if (!IsValid(VehiclePawnRef)
		|| !IsValid(RuntimeCatalog)
		|| !IsValid(SelectedVehicleData))
	{
		LastVehicleApplyResult = FCFRuntimeVehicleApplyResult();
		LastVehicleApplyResult.Status = ECFRuntimeVehicleApplyStatus::ValidationFailed;
		LastVehicleApplyResult.Message =
			TEXT("Runtime Apply UI에 유효한 Pawn, Catalog 또는 Selected Vehicle이 없습니다.");
		LastResultText = FString::Printf(
			TEXT("차량 [검증 실패] %s"),
			*LastVehicleApplyResult.Message);
		RefreshRuntimeApplyState();
		return LastVehicleApplyResult.Status;
	}

	// [v1.0.0] UI가 Runtime mutation을 직접 구현하지 않고 P0-02 service에 위임한 Vehicle 적용 결과입니다.
	LastVehicleApplyResult = FCFRuntimeVehicleApplyService::ApplyCatalogVehicle(
		VehiclePawnRef,
		RuntimeCatalog,
		SelectedVehicleData);

	if (LastVehicleApplyResult.Status == ECFRuntimeVehicleApplyStatus::Succeeded)
	{
		LastAppliedVehicleSourceData = SelectedVehicleData;
		LastAppliedVehicleRuntimeData = VehiclePawnRef->VehicleData.Get();
	}

	LastResultText = FString::Printf(
		TEXT("차량 [%s] %s"),
		*BuildResultStatusText(true),
		*LastVehicleApplyResult.Message);

	// [v1.0.0] Vehicle Apply 성공/복구 뒤 current VehicleData가 바뀌었을 수 있으므로 Mount option을 강제 재구성합니다.
	LastObservedVehicleData = nullptr;
	RefreshRuntimeApplyState();
	return LastVehicleApplyResult.Status;
}

// [v1.0.0] Selected Equipment를 Target Mount/Profile에 명시적으로 Runtime Apply합니다.
ECFRuntimeEquipApplyStatus UCFRuntimeApplyWidget::ApplySelectedEquipment()
{
	if (!IsValid(VehiclePawnRef)
		|| !IsValid(RuntimeCatalog)
		|| SelectedMountProfileId.IsNone()
		|| !IsValid(SelectedEquipmentData))
	{
		LastEquipmentApplyResult = FCFRuntimeEquipApplyResult();
		LastEquipmentApplyResult.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
		LastEquipmentApplyResult.Message =
			TEXT("Runtime Apply UI에 유효한 Pawn, Catalog, Target Mount 또는 Selected Equipment가 없습니다.");
		LastResultText = FString::Printf(
			TEXT("장비 [검증 실패] %s"),
			*LastEquipmentApplyResult.Message);
		RefreshRuntimeApplyState();
		return LastEquipmentApplyResult.Status;
	}

	// [v1.0.0] UI가 Fitting/Mass mutation을 직접 구현하지 않고 P0-03 service에 위임한 Equipment 적용 결과입니다.
	LastEquipmentApplyResult = FCFRuntimeEquipApplyService::ApplyCatalogEquipment(
		VehiclePawnRef,
		RuntimeCatalog,
		SelectedMountProfileId,
		SelectedEquipmentData);

	LastResultText = FString::Printf(
		TEXT("장비 [%s] %s"),
		*BuildResultStatusText(false),
		*LastEquipmentApplyResult.Message);

	RefreshRuntimeApplyState();
	return LastEquipmentApplyResult.Status;
}

// [v1.0.0] Config soft reference에서 기본 Runtime Test Catalog를 로드하고 선택 옵션 캐시를 준비합니다.
bool UCFRuntimeApplyWidget::LoadDefaultRuntimeCatalog()
{
	// [v1.0.0] Packaged-safe soft reference loader를 제공하는 Runtime Test Settings CDO입니다.
	const UCFRuntimeTestSettings* RuntimeTestSettings = GetDefault<UCFRuntimeTestSettings>();
	if (!RuntimeTestSettings)
	{
		RuntimeCatalog = nullptr;
		RebuildVehicleOptions();
		RebuildEquipmentOptions();
		return false;
	}

	// [v1.0.0] Config에 지정된 hard-reference Catalog Asset을 실제 runtime object로 로드한 결과입니다.
	UCFRuntimeTestCatalogData* LoadedRuntimeCatalog =
		RuntimeTestSettings->LoadDefaultCatalog();

	if (RuntimeCatalog == LoadedRuntimeCatalog && IsValid(RuntimeCatalog))
	{
		return true;
	}

	RuntimeCatalog = LoadedRuntimeCatalog;
	RebuildVehicleOptions();
	RebuildEquipmentOptions();
	return IsValid(RuntimeCatalog);
}

// [v1.0.0] 현재 Catalog AllowedVehicleData를 Vehicle ComboBox와 내부 option 배열에 반영합니다.
void UCFRuntimeApplyWidget::RebuildVehicleOptions()
{
	// [v1.0.0] option rebuild 전 사용자가 선택했던 exact VehicleData identity입니다.
	UCFVehicleData* PreviousSelectedVehicleData = SelectedVehicleData.Get();

	VehicleOptionDataArray.Reset();
	VehicleOptionLabelArray.Reset();

	if (VehicleComboBox)
	{
		VehicleComboBox->ClearOptions();
	}

	if (IsValid(RuntimeCatalog))
	{
		for (UCFVehicleData* VehicleData : RuntimeCatalog->AllowedVehicleData)
		{
			if (!IsValid(VehicleData))
			{
				continue;
			}

			VehicleOptionDataArray.Add(VehicleData);

			// [v1.0.0] 중복 AssetName이어도 ComboBox 문자열 identity가 충돌하지 않게 ordinal을 붙인 표시 문자열입니다.
			const FString VehicleOptionLabel = FString::Printf(
				TEXT("%d. %s"),
				VehicleOptionDataArray.Num(),
				*BuildVehicleDisplayText(VehicleData));
			VehicleOptionLabelArray.Add(VehicleOptionLabel);

			if (VehicleComboBox)
			{
				VehicleComboBox->AddOption(VehicleOptionLabel);
			}
		}
	}

	// [v1.0.0] rebuild 후에도 이전 exact 선택이 Catalog에 남아 있는지 찾은 index입니다.
	const int32 PreviousSelectionIndex =
		VehicleOptionDataArray.IndexOfByKey(PreviousSelectedVehicleData);

	if (VehicleOptionDataArray.IsValidIndex(PreviousSelectionIndex))
	{
		SelectVehicleByIndex(PreviousSelectionIndex);
	}
	else if (!VehicleOptionDataArray.IsEmpty())
	{
		SelectVehicleByIndex(0);
	}
	else
	{
		SelectedVehicleData = nullptr;
	}
}

// [v1.0.0] 현재 Pawn.VehicleData의 MountProfiles를 Target Mount ComboBox와 내부 option 배열에 반영합니다.
void UCFRuntimeApplyWidget::RebuildMountOptions()
{
	// [v1.0.0] option rebuild 전 사용자가 선택했던 MountProfileId입니다.
	const FName PreviousSelectedMountProfileId = SelectedMountProfileId;

	MountOptionIdArray.Reset();
	MountOptionLabelArray.Reset();

	if (MountComboBox)
	{
		MountComboBox->ClearOptions();
	}

	// [v1.0.0] 현재 실제 Pawn에서 Equipment Apply 대상 MountProfiles를 제공할 VehicleData입니다.
	UCFVehicleData* CurrentVehicleData =
		IsValid(VehiclePawnRef) ? VehiclePawnRef->VehicleData.Get() : nullptr;

	if (IsValid(CurrentVehicleData))
	{
		for (const FCFVehicleMountProfile& MountProfile : CurrentVehicleData->MountProfiles)
		{
			if (MountProfile.MountProfileId.IsNone())
			{
				continue;
			}

			MountOptionIdArray.Add(MountProfile.MountProfileId);

			// [v1.0.0] 같은 표시 문자열 충돌을 방지하기 위해 ordinal을 붙인 Mount option label입니다.
			const FString MountOptionLabel = FString::Printf(
				TEXT("%d. %s"),
				MountOptionIdArray.Num(),
				*BuildMountDisplayText(MountProfile));
			MountOptionLabelArray.Add(MountOptionLabel);

			if (MountComboBox)
			{
				MountComboBox->AddOption(MountOptionLabel);
			}
		}
	}

	// [v1.0.0] 같은 current VehicleData에서 이전 Mount 선택을 보존할 index입니다.
	const int32 PreviousMountIndex =
		MountOptionIdArray.IndexOfByKey(PreviousSelectedMountProfileId);

	if (MountOptionIdArray.IsValidIndex(PreviousMountIndex))
	{
		SelectedMountProfileId = MountOptionIdArray[PreviousMountIndex];
		if (MountComboBox && MountOptionLabelArray.IsValidIndex(PreviousMountIndex))
		{
			MountComboBox->SetSelectedOption(MountOptionLabelArray[PreviousMountIndex]);
		}
	}
	else if (!MountOptionIdArray.IsEmpty())
	{
		SelectedMountProfileId = MountOptionIdArray[0];
		if (MountComboBox && !MountOptionLabelArray.IsEmpty())
		{
			MountComboBox->SetSelectedOption(MountOptionLabelArray[0]);
		}
	}
	else
	{
		SelectedMountProfileId = NAME_None;
	}

	RebuildEquipmentOptions();
}

// [v1.0.0] 현재 Catalog AllowedEquipmentPresetData를 Equipment ComboBox와 내부 option 배열에 반영합니다.
void UCFRuntimeApplyWidget::RebuildEquipmentOptions()
{
	// [v1.0.0] option rebuild 전 사용자가 선택했던 exact EquipmentPresetData identity입니다.
	UCFEquipmentPresetData* PreviousSelectedEquipmentData = SelectedEquipmentData.Get();

	EquipmentOptionDataArray.Reset();
	EquipmentOptionLabelArray.Reset();

	if (EquipmentComboBox)
	{
		EquipmentComboBox->ClearOptions();
	}

	// [v1.0.0] 각 Equipment option에 현재 Mount 기준 호환/비호환 보조 표시를 붙일 MountProfile입니다.
	const FCFVehicleMountProfile* SelectedMountProfile = FindSelectedMountProfile();

	if (IsValid(RuntimeCatalog))
	{
		for (UCFEquipmentPresetData* EquipmentPresetData : RuntimeCatalog->AllowedEquipmentPresetData)
		{
			if (!IsValid(EquipmentPresetData))
			{
				continue;
			}

			EquipmentOptionDataArray.Add(EquipmentPresetData);

			// [v1.0.0] 현재 Mount 규칙과 이 Catalog 장비가 사전 호환되는지 표시할 결과입니다.
			const bool bEquipmentCompatible =
				SelectedMountProfile
				&& EquipmentPresetData->CanUseOnMount(
					SelectedMountProfile->MountType,
					SelectedMountProfile->SizeLimit);

			// [v1.0.0] DisplayName 계약을 유지하면서 ordinal/호환성만 보조 정보로 붙인 Equipment option label입니다.
			const FString EquipmentOptionLabel = FString::Printf(
				TEXT("%d. [%s] %s"),
				EquipmentOptionDataArray.Num(),
				bEquipmentCompatible ? TEXT("호환") : TEXT("비호환"),
				*BuildEquipmentDisplayText(EquipmentPresetData));
			EquipmentOptionLabelArray.Add(EquipmentOptionLabel);

			if (EquipmentComboBox)
			{
				EquipmentComboBox->AddOption(EquipmentOptionLabel);
			}
		}
	}

	// [v1.0.0] rebuild 뒤에도 이전 exact Equipment 선택이 Catalog에 남아 있는지 찾은 index입니다.
	const int32 PreviousSelectionIndex =
		EquipmentOptionDataArray.IndexOfByKey(PreviousSelectedEquipmentData);

	if (EquipmentOptionDataArray.IsValidIndex(PreviousSelectionIndex))
	{
		SelectedEquipmentData = EquipmentOptionDataArray[PreviousSelectionIndex];
		if (EquipmentComboBox && EquipmentOptionLabelArray.IsValidIndex(PreviousSelectionIndex))
		{
			EquipmentComboBox->SetSelectedOption(EquipmentOptionLabelArray[PreviousSelectionIndex]);
		}
	}
	else if (!EquipmentOptionDataArray.IsEmpty())
	{
		SelectedEquipmentData = EquipmentOptionDataArray[0];
		if (EquipmentComboBox && !EquipmentOptionLabelArray.IsEmpty())
		{
			EquipmentComboBox->SetSelectedOption(EquipmentOptionLabelArray[0]);
		}
	}
	else
	{
		SelectedEquipmentData = nullptr;
	}
}

// [v1.0.0] 현재 선택 MountProfileId에 해당하는 VehicleData MountProfile을 찾습니다.
const FCFVehicleMountProfile* UCFRuntimeApplyWidget::FindSelectedMountProfile() const
{
	if (!IsValid(VehiclePawnRef)
		|| !IsValid(VehiclePawnRef->VehicleData)
		|| SelectedMountProfileId.IsNone())
	{
		return nullptr;
	}

	for (const FCFVehicleMountProfile& MountProfile : VehiclePawnRef->VehicleData->MountProfiles)
	{
		if (MountProfile.MountProfileId == SelectedMountProfileId)
		{
			return &MountProfile;
		}
	}

	return nullptr;
}

// [v1.0.2] 선택 Mount의 실제 현재 Equipment를 Snapshot 우선, Legacy Weapon Runtime 차선으로 readback합니다.
UCFEquipmentPresetData* UCFRuntimeApplyWidget::FindCurrentAppliedEquipment() const
{
	if (!IsValid(VehiclePawnRef)
		|| SelectedMountProfileId.IsNone())
	{
		return nullptr;
	}

	// [v1.0.2] Applied Snapshot이 있으면 exact Mount 장비의 authoritative readback을 제공할 Fitting component입니다.
	const UCFVehicleFittingComp* VehicleFittingComp =
		VehiclePawnRef->GetVehicleFittingComp();
	if (IsValid(VehicleFittingComp)
		&& VehicleFittingComp->HasAppliedFittingSnapshot())
	{
		// [v1.0.2] 현재 출격/Runtime에 실제 Commit된 Fitting Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot =
			VehicleFittingComp->GetAppliedFittingSnapshot();

		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			if (ResolvedMount.MountProfileId == SelectedMountProfileId)
			{
				return ResolvedMount.EquipmentPresetData;
			}
		}

		return nullptr;
	}

	// [v1.0.2] Snapshot이 없는 Legacy Runtime에서 실제 활성 Mount/장비를 소유하는 Weapon component입니다.
	const UCFVehicleWeaponComp* VehicleWeaponComp =
		VehiclePawnRef->GetVehicleWeaponComp();
	if (!IsValid(VehicleWeaponComp)
		|| VehicleWeaponComp->GetActiveMountProfileId() != SelectedMountProfileId)
	{
		return nullptr;
	}

	return VehicleWeaponComp->GetActiveEquipmentPresetData();
}

// [v1.0.0] VehicleData를 Runtime Apply UI의 사람이 읽을 표시 문자열로 변환합니다.
FString UCFRuntimeApplyWidget::BuildVehicleDisplayText(const UCFVehicleData* VehicleData) const
{
	if (!IsValid(VehicleData))
	{
		return TEXT("없음");
	}

	return VehicleData->GetName();
}

// [v1.0.0] EquipmentPresetData의 Player-facing DisplayName 계약을 지키는 표시 문자열을 생성합니다.
FString UCFRuntimeApplyWidget::BuildEquipmentDisplayText(
	const UCFEquipmentPresetData* EquipmentPresetData) const
{
	if (!IsValid(EquipmentPresetData))
	{
		return TEXT("없음");
	}

	if (!EquipmentPresetData->DisplayName.IsEmpty())
	{
		return EquipmentPresetData->DisplayName.ToString();
	}

	return TEXT("(공개 이름 미지정)");
}

// [v1.0.0] 현재 Target Mount의 ID와 LocationSlot을 함께 보여주는 표시 문자열을 생성합니다.
FString UCFRuntimeApplyWidget::BuildMountDisplayText(
	const FCFVehicleMountProfile& MountProfile) const
{
	return FString::Printf(
		TEXT("%s / 위치 %s"),
		*MountProfile.MountProfileId.ToString(),
		*MountProfile.LocationSlotRef.ToString());
}

// [v1.0.0] 마지막 Vehicle/Equipment 결과 상태를 짧은 한국어 문자열로 변환합니다.
FString UCFRuntimeApplyWidget::BuildResultStatusText(const bool bVehicleResult) const
{
	if (bVehicleResult)
	{
		switch (LastVehicleApplyResult.Status)
		{
		case ECFRuntimeVehicleApplyStatus::Succeeded:
			return TEXT("성공");
		case ECFRuntimeVehicleApplyStatus::ValidationFailed:
			return TEXT("검증 실패");
		case ECFRuntimeVehicleApplyStatus::ApplyFailed:
			return LastVehicleApplyResult.bRecoveryAttempted && LastVehicleApplyResult.bRecoverySucceeded
				? TEXT("적용 실패 / 복구 성공")
				: TEXT("적용 실패");
		case ECFRuntimeVehicleApplyStatus::RecoveryFailed:
			return TEXT("복구 실패");
		default:
			return TEXT("시도 전");
		}
	}

	switch (LastEquipmentApplyResult.Status)
	{
	case ECFRuntimeEquipApplyStatus::Succeeded:
		return TEXT("성공");
	case ECFRuntimeEquipApplyStatus::ValidationFailed:
		return TEXT("검증 실패");
	case ECFRuntimeEquipApplyStatus::ApplyFailed:
		return LastEquipmentApplyResult.bRecoveryAttempted && LastEquipmentApplyResult.bRecoverySucceeded
			? TEXT("적용 실패 / 복구 성공")
			: TEXT("적용 실패");
	case ECFRuntimeEquipApplyStatus::RecoveryFailed:
		return TEXT("복구 실패");
	default:
		return TEXT("시도 전");
	}
}

// [v1.0.0] 현재 Current/Selected/Catalog/Compatibility/Last Result TextBlock 값을 갱신합니다.
void UCFRuntimeApplyWidget::RefreshDisplayTexts()
{
	if (CatalogSummaryText)
	{
		CatalogSummaryText->SetText(FText::FromString(
			IsValid(RuntimeCatalog)
				? RuntimeCatalog->BuildRuntimeTestCatalogSummary()
				: TEXT("Catalog: 로드 실패")));
	}

	// [v1.0.0] Current Vehicle 표시 기준이 되는 실제 Pawn.VehicleData입니다.
	UCFVehicleData* CurrentVehicleData =
		IsValid(VehiclePawnRef) ? VehiclePawnRef->VehicleData.Get() : nullptr;

	// [v1.0.0] transient Runtime copy를 마지막 UI Apply 원본 이름으로 설명할 수 있는지 여부입니다.
	const bool bCurrentIsLastUiRuntimeCopy =
		IsValid(CurrentVehicleData)
		&& CurrentVehicleData == LastAppliedVehicleRuntimeData
		&& IsValid(LastAppliedVehicleSourceData);

	// [v1.0.0] Current와 Selected를 혼동하지 않도록 별도 줄에 표시할 Current Vehicle 문자열입니다.
	FString CurrentVehicleDisplayText;
	if (bCurrentIsLastUiRuntimeCopy)
	{
		CurrentVehicleDisplayText = FString::Printf(
			TEXT("%s (Runtime Copy)"),
			*BuildVehicleDisplayText(LastAppliedVehicleSourceData));
	}
	else if (IsValid(CurrentVehicleData))
	{
		CurrentVehicleDisplayText = BuildVehicleDisplayText(CurrentVehicleData);
		if (CurrentVehicleData->HasAnyFlags(RF_Transient))
		{
			CurrentVehicleDisplayText.Append(TEXT(" (Transient)"));
		}
	}
	else
	{
		CurrentVehicleDisplayText = TEXT("없음");
	}

	if (CurrentVehicleText)
	{
		CurrentVehicleText->SetText(FText::FromString(
			FString::Printf(TEXT("현재 차량: %s"), *CurrentVehicleDisplayText)));
	}

	if (SelectedVehicleText)
	{
		SelectedVehicleText->SetText(FText::FromString(
			FString::Printf(
				TEXT("선택 차량: %s"),
				*BuildVehicleDisplayText(SelectedVehicleData))));
	}

	const FCFVehicleMountProfile* SelectedMountProfile =
		FindSelectedMountProfile();

	if (CurrentMountText)
	{
		CurrentMountText->SetText(FText::FromString(
			FString::Printf(
				TEXT("대상 Mount/Profile: %s"),
				SelectedMountProfile
					? *BuildMountDisplayText(*SelectedMountProfile)
					: TEXT("없음"))));
	}

	// [v1.0.2] Snapshot 또는 Legacy Weapon Runtime에서 선택 Mount에 실제 사용 중인 EquipmentPresetData입니다.
	UCFEquipmentPresetData* CurrentAppliedEquipment =
		FindCurrentAppliedEquipment();

	// [v1.0.2] Fitting Snapshot 자체가 없는 Legacy/default runtime인지 구분할 Fitting component입니다.
	const UCFVehicleFittingComp* VehicleFittingComp =
		IsValid(VehiclePawnRef) ? VehiclePawnRef->GetVehicleFittingComp() : nullptr;

	if (CurrentEquipmentText)
	{
		// [v1.0.2] Current Equipment를 실제 Snapshot/Legacy Runtime readback 또는 명시적 없음 상태로 표현한 문자열입니다.
		const FString CurrentEquipmentDisplayText =
			IsValid(CurrentAppliedEquipment)
				? BuildEquipmentDisplayText(CurrentAppliedEquipment)
				: (IsValid(VehicleFittingComp) && VehicleFittingComp->HasAppliedFittingSnapshot()
					? TEXT("없음")
					: TEXT("없음 (Legacy/기본 Runtime)"));

		CurrentEquipmentText->SetText(FText::FromString(
			FString::Printf(
				TEXT("현재 장비: %s"),
				*CurrentEquipmentDisplayText)));
	}

	if (SelectedEquipmentText)
	{
		SelectedEquipmentText->SetText(FText::FromString(
			FString::Printf(
				TEXT("선택 장비: %s"),
				*BuildEquipmentDisplayText(SelectedEquipmentData))));
	}

	// [v1.0.0] 현재 선택 Mount와 Equipment가 실제 Fitting 호환 규칙을 사전 통과하는지 여부입니다.
	const bool bSelectedEquipmentCompatible =
		SelectedMountProfile
		&& IsValid(SelectedEquipmentData)
		&& SelectedEquipmentData->CanUseOnMount(
			SelectedMountProfile->MountType,
			SelectedMountProfile->SizeLimit);

	if (EquipmentCompatibilityText)
	{
		EquipmentCompatibilityText->SetText(FText::FromString(
			SelectedMountProfile && IsValid(SelectedEquipmentData)
				? (bSelectedEquipmentCompatible
					? TEXT("호환성: 현재 Mount에 장착 가능")
					: TEXT("호환성: 현재 Mount에 장착 불가 — Apply 시 검증 실패 예상"))
				: TEXT("호환성: 확인할 Mount/장비 없음")));
	}

	if (LastResultTextBlock)
	{
		LastResultTextBlock->SetText(FText::FromString(
			FString::Printf(TEXT("마지막 결과: %s"), *LastResultText)));
	}
}

// [v1.0.0] 현재 선택과 Catalog/Pawn 유효성에 따라 Explicit Apply 버튼 활성 상태를 갱신합니다.
void UCFRuntimeApplyWidget::RefreshApplyButtonStates()
{
	// [v1.0.0] Catalog 전체 runtime selection 계약이 현재 유효한지 여부입니다.
	const bool bCatalogUsable =
		IsValid(RuntimeCatalog)
		&& RuntimeCatalog->IsRuntimeTestCatalogUsable();

	if (ApplyVehicleButton)
	{
		ApplyVehicleButton->SetIsEnabled(
			bCatalogUsable
			&& IsValid(VehiclePawnRef)
			&& IsValid(SelectedVehicleData));
	}

	if (ApplyEquipmentButton)
	{
		ApplyEquipmentButton->SetIsEnabled(
			bCatalogUsable
			&& IsValid(VehiclePawnRef)
			&& !SelectedMountProfileId.IsNone()
			&& IsValid(SelectedEquipmentData));
	}
}

// [v1.0.0] Vehicle ComboBox 선택 변경을 SelectedVehicleData 변경으로만 반영합니다.
void UCFRuntimeApplyWidget::HandleVehicleSelectionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	(void)SelectionType;

	// [v1.0.0] ComboBox의 unique label을 내부 Catalog Vehicle option index로 되돌린 값입니다.
	const int32 SelectedVehicleIndex =
		VehicleOptionLabelArray.IndexOfByKey(SelectedItem);

	if (VehicleOptionDataArray.IsValidIndex(SelectedVehicleIndex))
	{
		SelectedVehicleData = VehicleOptionDataArray[SelectedVehicleIndex];
		RefreshDisplayTexts();
		RefreshApplyButtonStates();
	}
}

// [v1.0.0] Mount ComboBox 선택 변경을 SelectedMountProfileId 변경으로만 반영합니다.
void UCFRuntimeApplyWidget::HandleMountSelectionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	(void)SelectionType;

	// [v1.0.0] ComboBox의 unique label을 current VehicleData Mount option index로 되돌린 값입니다.
	const int32 SelectedMountIndex =
		MountOptionLabelArray.IndexOfByKey(SelectedItem);

	if (MountOptionIdArray.IsValidIndex(SelectedMountIndex))
	{
		SelectedMountProfileId = MountOptionIdArray[SelectedMountIndex];
		RebuildEquipmentOptions();
		RefreshDisplayTexts();
		RefreshApplyButtonStates();
	}
}

// [v1.0.0] Equipment ComboBox 선택 변경을 SelectedEquipmentData 변경으로만 반영합니다.
void UCFRuntimeApplyWidget::HandleEquipmentSelectionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	(void)SelectionType;

	// [v1.0.0] ComboBox의 unique label을 Catalog Equipment option index로 되돌린 값입니다.
	const int32 SelectedEquipmentIndex =
		EquipmentOptionLabelArray.IndexOfByKey(SelectedItem);

	if (EquipmentOptionDataArray.IsValidIndex(SelectedEquipmentIndex))
	{
		SelectedEquipmentData =
			EquipmentOptionDataArray[SelectedEquipmentIndex];
		RefreshDisplayTexts();
		RefreshApplyButtonStates();
	}
}

// [v1.0.0] Vehicle Apply 버튼 클릭을 ApplySelectedVehicle로 전달합니다.
void UCFRuntimeApplyWidget::HandleApplyVehicleClicked()
{
	ApplySelectedVehicle();
}

// [v1.0.0] Equipment Apply 버튼 클릭을 ApplySelectedEquipment로 전달합니다.
void UCFRuntimeApplyWidget::HandleApplyEquipmentClicked()
{
	ApplySelectedEquipment();
}
