// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentBuilderTab.cpp
// Version: v1.5.3
// Date: 2026-09-17
// Description: CF-FQ-054 EBA-P0-05 Guided Equipment Builder USER UX correction + child DataAsset 제작 진입 UX 구현입니다.
// Changelog:
// - v1.5.3: Scanner child-data SBox를 감싸는 outer Slate slot의 누락된 닫힘을 복구.
// - v1.5.2: Scanner child-data 행과 제작 안내를 SVerticalBox로 묶어 잘못된 Slate slot 중첩을 교정.
// - v1.5.1: USER UX correction patch 중 include 영역에 잘못 삽입된 Weapon 버튼 조각을 제거하고 DataAsset 생성에 필요한 UE Editor header를 명시적으로 복구.
// - v1.5.0: 기존 장비 종류 변경 금지, 신규 에셋 이름 기반 자동 ID/경로, 게임 표시 이름 설명, compatibility 결과 인접 배치와 Turret/Weapon/Sensor DataAsset 생성 진입을 추가.
// - v1.4.0: 사용자 노출 용어를 한글 중심으로 통일하고 localized DisplayName 편집 보호, Scanner Utility 고정 UX를 추가.
// - v1.3.0: VehicleData picker, current MountProfile combo, exact3 advisory summary/refresh와 Vehicle Builder navigation-only action을 추가.
// - v1.2.0: 신규 Create target path, Review state, explicit `검토`와 one-shot `적용 및 저장` action을 추가.
// - v1.1.0: Weapon TurretMountData/WeaponData, Scanner VehicleSensorData transient picker와 validation hierarchy presentation을 추가.
// - v1.0.0: 0.27/0.73 shell, metadata-only EquipmentPreset browser, new/existing transient draft, identity/basic mount editor, common scroll과 Product mutation0 안내를 추가.
// Migration:
// - Product durable mutation은 `검토`를 통과한 exact EquipmentPreset target에서 explicit `적용 및 저장`을 눌렀을 때만 발생합니다.
// - 기존 localized 표시 이름은 손실 없는 편집이 불가능하면 읽기 전용으로 표시하고 저장 시 그대로 보존합니다.
// - 기존 EquipmentPreset의 무장/스캐너 종류는 고정하고 신규 Draft에서만 종류를 선택합니다.
// - 신규 EquipmentPreset은 에셋 이름으로 내부 EquipmentId와 저장 위치를 자동 결정합니다.
// - compatibility probe는 durable Review/Apply readiness와 분리되고 결과를 새로고침 action 바로 아래에 표시합니다.
// - child DataAsset 생성은 Content Browser 표준 신규 생성 흐름만 시작하며 EquipmentPreset durable 저장과 결합하지 않습니다.

#include "EquipmentAuthoring/CFEquipmentBuilderTab.h"

#include "EquipmentAuthoring/CFEquipmentBuilderVM.h"

#include "AssetRegistry/AssetData.h"
#include "ContentBrowserModule.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"
#include "Factories/DataAssetFactory.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SCFEquipmentBuilderTab"

// Slate widget tree와 transient Equipment Builder ViewModel을 초기화합니다.
void SCFEquipmentBuilderTab::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FCFEquipmentBuilderVM>();
	MountTypeOptions = {
		MakeShared<ECFVehicleMountType>(ECFVehicleMountType::None),
		MakeShared<ECFVehicleMountType>(ECFVehicleMountType::Fixed),
		MakeShared<ECFVehicleMountType>(ECFVehicleMountType::Gimbal),
		MakeShared<ECFVehicleMountType>(ECFVehicleMountType::Turret),
		MakeShared<ECFVehicleMountType>(ECFVehicleMountType::Launcher),
		MakeShared<ECFVehicleMountType>(ECFVehicleMountType::Utility)};
	WeaponSizeOptions = {
		MakeShared<ECFVehicleWeaponSize>(ECFVehicleWeaponSize::None),
		MakeShared<ECFVehicleWeaponSize>(ECFVehicleWeaponSize::Small),
		MakeShared<ECFVehicleWeaponSize>(ECFVehicleWeaponSize::Medium),
		MakeShared<ECFVehicleWeaponSize>(ECFVehicleWeaponSize::Large)};

	// 초기 metadata-only browser 준비 오류입니다.
	FString InitializeError;
	if (!ViewModel->Initialize(InitializeError))
	{
		LastStatusMessage = InitializeError;
	}
	else
	{
		LastStatusMessage = FString::Printf(TEXT("장비 프리셋 목록 준비 완료: %d개"), ViewModel->GetPresetEntries().Num());
	}
	RefreshPresetItemsFromViewModel();
	RefreshCompatibilityMountItemsFromViewModel();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("Title", "CarFight 장비 제작 가이드"))
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SSplitter)
				+ SSplitter::Slot().Value(0.27f)[BuildBrowserPanel()]
				+ SSplitter::Slot().Value(0.73f)[BuildDraftPanel()]
			]
		]
	];
}

// 왼쪽 작업 대상 browser와 제작 단계 영역을 구성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::BuildBrowserPanel()
{
	return SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("TargetSection", "작업 대상"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("NewPreset", "+ 새 장비 프리셋"))
				.ToolTipText(LOCTEXT("NewPresetTip", "저장된 Asset을 만들지 않고 새 transient 장비 프리셋 초안을 시작합니다."))
				.OnClicked(this, &SCFEquipmentBuilderTab::HandleBeginNewPreset)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SAssignNew(PresetListView, SListView<FPresetRowPtr>)
				.ListItemsSource(&PresetItems)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SCFEquipmentBuilderTab::HandleGeneratePresetRow)
				.OnSelectionChanged(this, &SCFEquipmentBuilderTab::HandlePresetSelectionChanged)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "목록 새로고침"))
				.ToolTipText(LOCTEXT("RefreshTip", "기존 Data Asset Manager의 metadata-only inventory를 fresh 읽습니다."))
				.OnClicked(this, &SCFEquipmentBuilderTab::HandleRefreshBrowser)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("Steps", "제작 단계"))
			]
			+ SVerticalBox::Slot().AutoHeight()[BuildStepList()]
		];
}

// 오른쪽 고정 상태 header + common scroll draft 영역을 구성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::BuildDraftPanel()
{
	return SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetTargetText).AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("CurrentStep", "현재 단계: 5~6. 최종 검토 / 적용 및 저장"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetDraftStateText).AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SBorder).Padding(8.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(LOCTEXT("ActionTitle", "지금 할 일"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetCurrentActionText).AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SAssignNew(DraftScrollBox, SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[BuildDraftFields()]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)[BuildReferenceSummary()]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)[BuildCompatibilityPanel()]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetReviewSummaryText).AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetMutationSafetyText).AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("ResetDraft", "초안 되돌리기"))
					.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasActiveDraft() && ViewModel->HasDraftChanges(); })
					.OnClicked(this, &SCFEquipmentBuilderTab::HandleResetDraft)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("ReviewDraft", "검토"))
					.ToolTipText(LOCTEXT("ReviewDraftTip", "현재 저장 대상, 장비 프리셋 내용, ID 중복과 하위 데이터 상태를 다시 확인해 한 번 사용할 수 있는 검토 결과를 만듭니다."))
					.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasActiveDraft(); })
					.OnClicked(this, &SCFEquipmentBuilderTab::HandleReviewDraft)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ApplyDraft", "적용 및 저장"))
					.ToolTipText(LOCTEXT("ApplyDraftTip", "검토 결과를 한 번 사용해 현재 장비 프리셋 하나만 저장합니다. 저장 직전에 상태를 다시 확인합니다."))
					.IsEnabled(this, &SCFEquipmentBuilderTab::CanApplyReviewedDraft)
					.OnClicked(this, &SCFEquipmentBuilderTab::HandleApplyDraft)
				]
			]
		];
}

// EBA P0 전체 흐름을 보여주는 단계 안내 영역을 구성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::BuildStepList() const
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SNew(STextBlock).Text(LOCTEXT("Step1", "✓ 1. 작업 대상 / 초안"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SNew(STextBlock).Text(LOCTEXT("Step2", "● 2. 장비 구성 / 하위 데이터 검증"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SNew(STextBlock).Text(LOCTEXT("Step3", "○ 3. 장착 규격"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SNew(STextBlock).Text(LOCTEXT("Step4", "○ 4. 데이터 검증"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SNew(STextBlock).Text(LOCTEXT("Step5", "○ 5. 최종 검토"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SNew(STextBlock).Text(LOCTEXT("Step6", "○ 6. 적용 및 저장"))];
}

// 현재 Gate에서 편집 가능한 transient identity/basic mount 필드를 구성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::BuildDraftFields()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)[SNew(STextBlock).Text(LOCTEXT("DraftTitle", "장비 프리셋 초안"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SButton)
				.Text_Lambda([this]() { return GetDraftModeButtonText(ECFEquipmentDraftMode::Weapon); })
				.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->CanChangeDraftMode(); })
				.ToolTipText(LOCTEXT("WeaponModeTip", "새 장비 프리셋을 만들 때만 무장 장비 종류를 선택할 수 있습니다. 기존 장비의 종류는 바꾸지 않습니다."))
				.OnClicked_Lambda([this]() { return HandleSelectDraftMode(ECFEquipmentDraftMode::Weapon); })
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text_Lambda([this]() { return GetDraftModeButtonText(ECFEquipmentDraftMode::Scanner); })
				.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->CanChangeDraftMode(); })
				.ToolTipText(LOCTEXT("ScannerModeTip", "새 장비 프리셋을 만들 때만 스캐너 장비 종류를 선택할 수 있습니다. 기존 장비의 종류는 바꾸지 않습니다."))
				.OnClicked_Lambda([this]() { return HandleSelectDraftMode(ECFEquipmentDraftMode::Scanner); })
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Visibility_Lambda([this]()
			{
				return ViewModel.IsValid() && ViewModel->HasActiveDraft() && !ViewModel->IsNewDraft()
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			.Text(LOCTEXT("ExistingModeLockedNote", "기존 장비의 종류는 고정됩니다. 무장 장비를 스캐너로, 스캐너를 무장 장비로 바꾸려면 기존 프리셋을 변환하지 말고 새 장비 프리셋을 만드세요."))
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.26f)[SNew(STextBlock).Text(LOCTEXT("IdLabel", "내부 장비 ID (자동)"))]
			+ SHorizontalBox::Slot().FillWidth(0.74f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasActiveDraft() ? FText::FromName(ViewModel->GetDraft().EquipmentId) : FText::GetEmpty(); })
				.ToolTipText(LOCTEXT("IdTip", "전투 로그, 저장 데이터, 디버그에서 장비를 식별하는 내부 값입니다. 신규 장비에서는 아래 에셋 이름에서 자동으로 정하므로 직접 관리할 필요가 없습니다."))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.26f)[SNew(STextBlock).Text(LOCTEXT("DisplayLabel", "게임 표시 이름"))]
			+ SHorizontalBox::Slot().FillWidth(0.74f)
			[
				SNew(SEditableTextBox)
				.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->CanEditDisplayName(); })
				.Text_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasActiveDraft() ? ViewModel->GetDraft().DisplayName : FText::GetEmpty(); })
				.HintText(LOCTEXT("DisplayHint", "예: 중형 헤비 캐논"))
				.ToolTipText(LOCTEXT("DisplayTip", "플레이어가 Fitting(피팅)과 HUD에서 보게 되는 이름입니다. 에셋 파일 이름이나 내부 장비 ID와는 별개입니다."))
				.OnTextChanged(this, &SCFEquipmentBuilderTab::HandleDisplayNameChanged)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DisplayMeaningNote", "게임 표시 이름은 플레이어에게 보여주는 이름입니다. 에셋 이름(콘텐츠 브라우저에서 보이는 이름)과 내부 장비 ID는 아래에서 별도로 관리됩니다."))
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Visibility_Lambda([this]()
			{
				return ViewModel.IsValid() && ViewModel->HasActiveDraft() && !ViewModel->CanEditDisplayName()
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			.Text(LOCTEXT("DisplayPreservedNote", "이 기존 장비의 표시 이름은 현지화 정보가 포함되어 있어 여기서는 안전하게 변경하지 않습니다. 다른 항목은 검토·수정할 수 있으며 저장해도 기존 표시 이름은 그대로 보존됩니다."))
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SBox)
			.Visibility_Lambda([this]()
			{
				return ViewModel.IsValid() && ViewModel->HasActiveDraft() && ViewModel->IsNewDraft()
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.26f)[SNew(STextBlock).Text(LOCTEXT("CreateAssetNameLabel", "에셋 이름"))]
					+ SHorizontalBox::Slot().FillWidth(0.74f)
					[
						SNew(SEditableTextBox)
						.Text_Lambda([this]() { return ViewModel.IsValid() ? FText::FromString(ViewModel->GetCreateAssetName()) : FText::GetEmpty(); })
						.HintText(LOCTEXT("CreateAssetNameHint", "예: HeavyCannon_Mk2"))
						.ToolTipText(LOCTEXT("CreateAssetNameTip", "콘텐츠 브라우저에서 보일 새 장비 프리셋 에셋 이름입니다. 입력하면 내부 장비 ID와 저장 위치를 자동으로 결정합니다."))
						.OnTextChanged(this, &SCFEquipmentBuilderTab::HandleCreateAssetNameChanged)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.26f)[SNew(STextBlock).Text(LOCTEXT("AutoTargetLabel", "저장 위치 (자동)"))]
					+ SHorizontalBox::Slot().FillWidth(0.74f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return ViewModel.IsValid() && !ViewModel->GetCreateTargetObjectPath().IsEmpty()
								? FText::FromString(ViewModel->GetCreateTargetObjectPath())
								: LOCTEXT("AutoTargetPending", "에셋 이름을 입력하면 자동으로 정해집니다.");
						})
						.AutoWrapText(true)
					]
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.26f)[SNew(STextBlock).Text(LOCTEXT("MountLabel", "요구 장착 타입"))]
			+ SHorizontalBox::Slot().FillWidth(0.74f)
			[
				SNew(SComboBox<TSharedPtr<ECFVehicleMountType>>)
				.OptionsSource(&MountTypeOptions)
				.IsEnabled_Lambda([this]()
				{
					return ViewModel.IsValid() && ViewModel->HasActiveDraft()
						&& ViewModel->GetDraft().DraftMode == ECFEquipmentDraftMode::Weapon;
				})
				.OnGenerateWidget(this, &SCFEquipmentBuilderTab::GenerateMountTypeOption)
				.OnSelectionChanged(this, &SCFEquipmentBuilderTab::HandleMountTypeChanged)
				[SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetCurrentMountTypeText)]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.26f)[SNew(STextBlock).Text(LOCTEXT("SizeLabel", "요구 장착 크기"))]
			+ SHorizontalBox::Slot().FillWidth(0.74f)
			[
				SNew(SComboBox<TSharedPtr<ECFVehicleWeaponSize>>)
				.OptionsSource(&WeaponSizeOptions)
				.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasActiveDraft() && ViewModel->GetDraft().DraftMode == ECFEquipmentDraftMode::Weapon; })
				.OnGenerateWidget(this, &SCFEquipmentBuilderTab::GenerateWeaponSizeOption)
				.OnSelectionChanged(this, &SCFEquipmentBuilderTab::HandleWeaponSizeChanged)
				[SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetCurrentWeaponSizeText)]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MountNote", "무장 장비는 필요한 장착 타입과 크기를 선택합니다. 스캐너 장비는 장착 타입이 유틸리티, 장착 크기가 해당 없음으로 고정됩니다."))
			.AutoWrapText(true)
		];
}

// current Draft의 Weapon/Scanner child reference selector와 bounded validation 결과를 구성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::BuildReferenceSummary()
{
	return SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text(LOCTEXT("RefsTitle", "장비 구성 데이터 제작 / 선택"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 3.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RefsGuideIntro", "이미 만든 데이터를 선택할 수도 있고, 필요한 하위 데이터가 없으면 여기서 새 DataAsset(데이터 에셋) 생성을 시작할 수 있습니다. 하위 데이터는 각 데이터 에디터에서 내용을 작성·저장한 뒤 이 화면에서 선택해 검증합니다."))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]()
				{
					return ViewModel.IsValid() && ViewModel->HasActiveDraft() && ViewModel->GetDraft().DraftMode == ECFEquipmentDraftMode::Weapon
						? EVisibility::Visible
						: EVisibility::Collapsed;
				})
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(0.26f)
						[
							SNew(STextBlock).Text(LOCTEXT("TurretMountLabel", "터렛 마운트 데이터"))
						]
						+ SHorizontalBox::Slot().FillWidth(0.74f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.0f)
							[
								SNew(SObjectPropertyEntryBox)
								.AllowedClass(UCFTurretMountData::StaticClass())
								.ObjectPath(this, &SCFEquipmentBuilderTab::GetTurretMountObjectPath)
								.OnObjectChanged(this, &SCFEquipmentBuilderTab::HandleTurretMountChanged)
								.AllowClear(true)
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CreateTurretMountData", "새로 만들기…"))
								.ToolTipText(LOCTEXT("CreateTurretMountDataTip", "새 터렛 마운트 DataAsset 생성을 시작합니다. 만든 뒤 Base/Yaw/Pitch 메쉬, 피벗/총구 소켓, 회전 제한과 속도를 설정하고 저장하세요."))
								.OnClicked(this, &SCFEquipmentBuilderTab::HandleCreateTurretMountData)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TurretMountAuthoringGuide", "터렛 마운트 제작 순서: Base/Yaw/Pitch 메쉬 → Yaw/Pitch 피벗 소켓 → Muzzle(총구) 소켓 → Yaw/Pitch 회전 범위·속도 → 발사 정렬/총구 안전 설정 → 저장 → 위 선택칸에서 선택."))
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(0.26f)
						[
							SNew(STextBlock).Text(LOCTEXT("WeaponDataLabel", "무기 데이터"))
						]
						+ SHorizontalBox::Slot().FillWidth(0.74f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.0f)
							[
								SNew(SObjectPropertyEntryBox)
								.AllowedClass(UCFWeaponData::StaticClass())
								.ObjectPath(this, &SCFEquipmentBuilderTab::GetWeaponDataObjectPath)
								.OnObjectChanged(this, &SCFEquipmentBuilderTab::HandleWeaponDataChanged)
								.AllowClear(true)
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CreateWeaponData", "새로 만들기…"))
								.ToolTipText(LOCTEXT("CreateWeaponDataTip", "새 무기 DataAsset 생성을 시작합니다. 만든 뒤 장착 규격, 발사 방식·속도·사거리, 발사체/탄약과 필요 시 Heat/Charge를 설정하고 저장하세요."))
								.OnClicked(this, &SCFEquipmentBuilderTab::HandleCreateWeaponData)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WeaponAuthoringGuide", "무기 데이터 제작 순서: 무기 크기·호환 장착 타입 → HitScan/Projectile 발사 방식 → 발사속도·사거리·탄퍼짐 → Projectile/Ammo 연결 → 필요 시 런처 패턴·Heat·Charge → 저장 → 위 선택칸에서 선택."))
						.AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]()
				{
					return ViewModel.IsValid() && ViewModel->HasActiveDraft() && ViewModel->GetDraft().DraftMode == ECFEquipmentDraftMode::Scanner
						? EVisibility::Visible
						: EVisibility::Collapsed;
				})
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(0.26f)
						[
							SNew(STextBlock).Text(LOCTEXT("SensorDataLabel", "센서 데이터"))
						]
						+ SHorizontalBox::Slot().FillWidth(0.74f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.0f)
							[
								SNew(SObjectPropertyEntryBox)
								.AllowedClass(UCFVehicleSensorData::StaticClass())
								.ObjectPath(this, &SCFEquipmentBuilderTab::GetSensorDataObjectPath)
								.OnObjectChanged(this, &SCFEquipmentBuilderTab::HandleSensorDataChanged)
								.AllowClear(true)
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CreateSensorData", "새로 만들기…"))
								.ToolTipText(LOCTEXT("CreateSensorDataTip", "새 센서 DataAsset 생성을 시작합니다. 만든 뒤 Passive/Active/Visual 탐지, Contact, Tactical Analysis와 Radar 표시 범위를 설정하고 저장하세요."))
								.OnClicked(this, &SCFEquipmentBuilderTab::HandleCreateSensorData)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SensorAuthoringGuide", "센서 데이터 제작 순서: Passive/Active/Visual 탐지 범위 → 갱신 주기·Contact 유지 → Tactical Analysis 진행 설정 → Radar 표시 범위 프리셋 → 전체 데이터 검증 → 저장 → 위 선택칸에서 선택."))
						.AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("ValidationTitle", "하위 데이터 검증"))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(this, &SCFEquipmentBuilderTab::GetValidationSummaryText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ChildBoundary", "하위 데이터의 생성·편집·저장은 장비 프리셋 저장과 분리됩니다. 이 가이드의 '새로 만들기…'는 표준 DataAsset 생성 흐름을 시작하고, 실제 내용은 해당 데이터 에디터에서 작성·저장합니다. '제한 검증'은 전체 유효성 검사가 아니라 현재 확인 가능한 기본 구조만 확인했다는 뜻입니다."))
				.AutoWrapText(true)
			]
		];
}

// transient VehicleData + MountProfile compatibility probe와 navigation-only Cross-Builder UX를 구성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::BuildCompatibilityPanel()
{
	return SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text(LOCTEXT("CompatibilityTitle", "차량 장착 호환성 — 저장과 독립"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.26f)
				[
					SNew(STextBlock).Text(LOCTEXT("CompatibilityVehicleLabel", "차량 데이터"))
				]
				+ SHorizontalBox::Slot().FillWidth(0.74f)
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UCFVehicleData::StaticClass())
					.ObjectPath(this, &SCFEquipmentBuilderTab::GetCompatibilityVehicleObjectPath)
					.OnObjectChanged(this, &SCFEquipmentBuilderTab::HandleCompatibilityVehicleChanged)
					.AllowClear(true)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.26f)
				[
					SNew(STextBlock).Text(LOCTEXT("CompatibilityMountLabel", "장착 프로파일"))
				]
				+ SHorizontalBox::Slot().FillWidth(0.74f)
				[
					SAssignNew(CompatibilityMountCombo, SComboBox<FMountRowPtr>)
					.OptionsSource(&CompatibilityMountItems)
					.OnGenerateWidget(this, &SCFEquipmentBuilderTab::GenerateCompatibilityMountOption)
					.OnSelectionChanged(this, &SCFEquipmentBuilderTab::HandleCompatibilityMountChanged)
					[
						SNew(STextBlock).Text(this, &SCFEquipmentBuilderTab::GetCurrentCompatibilityMountText)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("RefreshCompatibility", "호환성 새로고침"))
					.ToolTipText(LOCTEXT("RefreshCompatibilityTip", "선택한 차량 데이터와 현재 장착 지점 목록을 다시 읽어 호환성 결과를 갱신합니다."))
					.OnClicked(this, &SCFEquipmentBuilderTab::HandleRefreshCompatibility)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenVehicleBuilder", "차량 제작 가이드 열기"))
					.ToolTipText(LOCTEXT("OpenVehicleBuilderTip", "차량 제작 가이드 탭만 엽니다. 현재 차량이나 장착 지점 선택은 전달하지 않습니다."))
					.OnClicked(this, &SCFEquipmentBuilderTab::HandleOpenVehicleBuilder)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.Padding(6.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(LOCTEXT("CompatibilityResultTitle", "호환성 결과"))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(this, &SCFEquipmentBuilderTab::GetCompatibilitySummaryText)
						.AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CompatibilityBoundary", "호환됨 / 호환되지 않음 / 판정할 수 없음은 선택한 차량의 장착 지점에 대한 참고 결과입니다. 이 결과는 장비 프리셋의 검토·저장 가능 여부를 바꾸지 않으며 차량 데이터나 실제 장착 상태를 수정하지 않습니다."))
				.AutoWrapText(true)
			]
		];
}

// Browser row 하나의 Slate 표현을 생성합니다.
TSharedRef<ITableRow> SCFEquipmentBuilderTab::HandleGeneratePresetRow(FPresetRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FPresetRowPtr>, OwnerTable)
		.Padding(4.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? Item->AssetName : TEXT("<invalid>")))]
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? Item->ObjectPath : FString()))]
		];
}

// Browser에서 existing EquipmentPreset을 선택해 read-only source를 transient draft로 load합니다.
void SCFEquipmentBuilderTab::HandlePresetSelectionChanged(FPresetRowPtr SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (bSynchronizingSelection || !SelectedItem.IsValid() || !ViewModel.IsValid())
	{
		return;
	}
	// existing source read 오류입니다.
	FString LoadError;
	if (!ViewModel->LoadExistingPreset(SelectedItem->ObjectPath, LoadError))
	{
		LastStatusMessage = LoadError;
		return;
	}
	LastStatusMessage = FString::Printf(TEXT("기존 장비 프리셋을 임시 초안으로 읽었습니다: %s"), *SelectedItem->AssetName);
	ResetDraftScrollToStart();
}

// Product Asset을 만들지 않고 새 EquipmentPreset transient draft를 시작합니다.
FReply SCFEquipmentBuilderTab::HandleBeginNewPreset()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	ViewModel->BeginNewPreset();
	if (PresetListView.IsValid())
	{
		bSynchronizingSelection = true;
		PresetListView->ClearSelection();
		bSynchronizingSelection = false;
	}
	LastStatusMessage = TEXT("새 장비 프리셋 임시 초안을 시작했습니다. 장비 종류와 에셋 이름을 정하면 내부 ID와 저장 위치는 자동으로 결정됩니다.");
	ResetDraftScrollToStart();
	return FReply::Handled();
}

// Data Asset Manager metadata inventory를 fresh 읽어 EquipmentPreset browser만 갱신합니다.
FReply SCFEquipmentBuilderTab::HandleRefreshBrowser()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	// metadata-only refresh 오류입니다.
	FString RefreshError;
	if (!ViewModel->RefreshBrowser(RefreshError))
	{
		LastStatusMessage = RefreshError;
	}
	else
	{
		LastStatusMessage = FString::Printf(TEXT("장비 프리셋 목록을 새로고침했습니다: %d개"), ViewModel->GetPresetEntries().Num());
	}
	RefreshPresetItemsFromViewModel();
	return FReply::Handled();
}

// current transient draft를 source snapshot으로 되돌립니다.
FReply SCFEquipmentBuilderTab::HandleResetDraft()
{
	if (ViewModel.IsValid())
	{
		ViewModel->ResetDraftToSource();
		LastStatusMessage = TEXT("임시 초안을 시작 상태로 되돌렸습니다. 저장된 장비 프리셋에는 변경이 없습니다.");
		ResetDraftScrollToStart();
	}
	return FReply::Handled();
}

// 신규 Draft에서만 Weapon/Scanner 장비 종류를 변경합니다.
FReply SCFEquipmentBuilderTab::HandleSelectDraftMode(ECFEquipmentDraftMode DraftMode)
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	if (!ViewModel->CanChangeDraftMode())
	{
		LastStatusMessage = TEXT("기존 장비의 종류는 변경할 수 없습니다. 다른 종류의 장비가 필요하면 '+ 새 장비 프리셋'으로 새로 만드세요.");
		return FReply::Handled();
	}
	ViewModel->SetDraftMode(DraftMode);
	LastStatusMessage = DraftMode == ECFEquipmentDraftMode::Scanner
		? TEXT("새 프리셋을 스캐너 장비로 설정했습니다. 장착 타입은 유틸리티, 장착 크기는 해당 없음으로 고정됩니다.")
		: TEXT("새 프리셋을 무장 장비로 설정했습니다. 터렛 마운트와 무기 데이터를 제작하거나 선택하세요.");
	return FReply::Handled();
}

// DisplayName 입력을 transient draft에 반영합니다.
void SCFEquipmentBuilderTab::HandleDisplayNameChanged(const FText& NewText)
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetDisplayName(NewText);
	}
}

// 신규 Draft의 사용자용 에셋 이름을 자동 EquipmentId/Product target state에 반영합니다.
void SCFEquipmentBuilderTab::HandleCreateAssetNameChanged(const FText& NewText)
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetCreateAssetName(NewText.ToString());
	}
}

// current Draft를 다시 검증하고 current-contract Reviewed approval을 생성합니다.
FReply SCFEquipmentBuilderTab::HandleReviewDraft()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	// fresh Review 실패 상세입니다.
	FString ReviewError;
	if (!ViewModel->ReviewCurrentDraft(ReviewError))
	{
		LastStatusMessage = FString::Printf(TEXT("검토 실패: %s"), *ReviewError);
		return FReply::Handled();
	}
	LastStatusMessage = TEXT("검토 완료: 저장 대상, 장비 프리셋 내용, ID 중복과 하위 데이터 상태를 다시 확인했습니다. 초안을 바꾸면 다시 검토해야 합니다.");
	return FReply::Handled();
}

// current one-shot Reviewed approval을 소비해 exact EquipmentPreset을 적용 및 저장합니다.
FReply SCFEquipmentBuilderTab::HandleApplyDraft()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	// one-shot durable apply terminal report입니다.
	FCFEquipmentApplyReport ApplyReport;
	if (!ViewModel->ApplyReviewedDraft(ApplyReport))
	{
		LastStatusMessage = FString::Printf(TEXT("적용 및 저장 실패: %s"), *ApplyReport.Diagnostic);
		return FReply::Handled();
	}
	LastStatusMessage = FString::Printf(TEXT("적용 및 저장 완료: 현재 장비 프리셋 하나를 저장했고 디스크에서 다시 읽어 같은 내용인지 확인했습니다. %s"), *ApplyReport.TargetObjectPath);
	RefreshPresetItemsFromViewModel();
	return FReply::Handled();
}

// RequiredMountType 선택을 transient draft에 반영합니다.
void SCFEquipmentBuilderTab::HandleMountTypeChanged(TSharedPtr<ECFVehicleMountType> NewValue, ESelectInfo::Type SelectInfo)
{
	if (ViewModel.IsValid() && NewValue.IsValid())
	{
		ViewModel->SetRequiredMountType(*NewValue);
	}
}

// RequiredWeaponSize 선택을 transient draft에 반영합니다.
void SCFEquipmentBuilderTab::HandleWeaponSizeChanged(TSharedPtr<ECFVehicleWeaponSize> NewValue, ESelectInfo::Type SelectInfo)
{
	if (ViewModel.IsValid() && NewValue.IsValid())
	{
		ViewModel->SetRequiredWeaponSize(*NewValue);
	}
}

// Weapon Draft의 TurretMountData picker 변경을 transient reference에만 반영합니다.
void SCFEquipmentBuilderTab::HandleTurretMountChanged(const FAssetData& AssetData)
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	// picker 변경을 VM에 전달한 결과 메시지입니다.
	FString SelectionError;
	// clear를 포함한 exact soft object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	if (!ViewModel->SetTurretMountDataPath(SelectedPath, SelectionError))
	{
		LastStatusMessage = SelectionError;
		return;
	}
	LastStatusMessage = TEXT("터렛 마운트 데이터 선택을 임시 초안에만 반영했습니다. 저장된 하위 데이터는 수정하지 않았습니다.");
}

// Weapon Draft의 WeaponData picker 변경을 transient reference에만 반영합니다.
void SCFEquipmentBuilderTab::HandleWeaponDataChanged(const FAssetData& AssetData)
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	// picker 변경을 VM에 전달한 결과 메시지입니다.
	FString SelectionError;
	// clear를 포함한 exact soft object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	if (!ViewModel->SetWeaponDataPath(SelectedPath, SelectionError))
	{
		LastStatusMessage = SelectionError;
		return;
	}
	LastStatusMessage = TEXT("무기 데이터 선택을 임시 초안에 반영하고 관련 하위 데이터 검증을 갱신했습니다.");
}

// Scanner Draft의 VehicleSensorData picker 변경을 transient reference에만 반영합니다.
void SCFEquipmentBuilderTab::HandleSensorDataChanged(const FAssetData& AssetData)
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	// picker 변경을 VM에 전달한 결과 메시지입니다.
	FString SelectionError;
	// clear를 포함한 exact soft object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	if (!ViewModel->SetSensorDataPath(SelectedPath, SelectionError))
	{
		LastStatusMessage = SelectionError;
		return;
	}
	LastStatusMessage = TEXT("센서 데이터 선택을 임시 초안에 반영하고 내장 검증을 갱신했습니다.");
}

// TurretMountData 신규 DataAsset 생성 흐름을 Content Browser에서 시작합니다.
FReply SCFEquipmentBuilderTab::HandleCreateTurretMountData()
{
	return BeginChildDataAssetCreation(
		UCFTurretMountData::StaticClass(),
		TEXT("DA_NewTurretMount"),
		TEXT("/Game/CarFight/Weapons/Data/TurretMounts"),
		TEXT("터렛 마운트 데이터"));
}

// WeaponData 신규 DataAsset 생성 흐름을 Content Browser에서 시작합니다.
FReply SCFEquipmentBuilderTab::HandleCreateWeaponData()
{
	return BeginChildDataAssetCreation(
		UCFWeaponData::StaticClass(),
		TEXT("DA_NewWeapon"),
		TEXT("/Game/CarFight/Weapons/Data/WeaponDefs"),
		TEXT("무기 데이터"));
}

// VehicleSensorData 신규 DataAsset 생성 흐름을 Content Browser에서 시작합니다.
FReply SCFEquipmentBuilderTab::HandleCreateSensorData()
{
	return BeginChildDataAssetCreation(
		UCFVehicleSensorData::StaticClass(),
		TEXT("DA_NewSensor"),
		TEXT("/Game/CarFight/Vehicles/Data/Sensor"),
		TEXT("센서 데이터"));
}

// 지정한 child DataAsset class의 표준 Content Browser 신규 생성 흐름을 시작합니다.
FReply SCFEquipmentBuilderTab::BeginChildDataAssetCreation(
	UClass* DataAssetClass,
	const FString& DefaultAssetName,
	const FString& PackagePath,
	const FString& UserLabel)
{
	if (DataAssetClass == nullptr)
	{
		LastStatusMessage = FString::Printf(TEXT("%s 생성 준비에 실패했습니다: DataAsset class가 없습니다."), *UserLabel);
		return FReply::Handled();
	}

	// 선택한 강타입 DataAsset을 생성할 Unreal 표준 DataAsset factory입니다.
	UDataAssetFactory* DataAssetFactory = NewObject<UDataAssetFactory>();
	DataAssetFactory->DataAssetClass = DataAssetClass;

	// 표준 Content Browser 신규 Asset 생성 UI에 접근할 Editor 모듈입니다.
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.Get().CreateNewAsset(DefaultAssetName, PackagePath, DataAssetClass, DataAssetFactory);

	LastStatusMessage = FString::Printf(
		TEXT("%s 새 DataAsset 생성을 시작했습니다. 콘텐츠 브라우저에서 이름을 확정하고 데이터 에디터에서 값을 작성·저장한 뒤 이 가이드의 선택칸에서 선택하세요."),
		*UserLabel);
	return FReply::Handled();
}

// compatibility VehicleData exact picker 변경을 transient identity에만 반영합니다.
void SCFEquipmentBuilderTab::HandleCompatibilityVehicleChanged(const FAssetData& AssetData)
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	// clear를 포함한 exact VehicleData object path입니다.
	const FSoftObjectPath VehicleDataPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	ViewModel->SetCompatibilityVehicleDataPath(VehicleDataPath);
	RefreshCompatibilityMountItemsFromViewModel();
	LastStatusMessage = VehicleDataPath.IsNull()
		? TEXT("호환성을 확인할 차량 선택을 비웠습니다. 장비 검토/저장 상태는 변경되지 않습니다.")
		: TEXT("차량 데이터를 호환성 확인 대상으로 선택했습니다. 장착 프로파일을 선택하세요.");
}

// compatibility MountProfile combo 변경을 transient identity에만 반영합니다.
void SCFEquipmentBuilderTab::HandleCompatibilityMountChanged(FMountRowPtr NewValue, ESelectInfo::Type SelectInfo)
{
	if (!ViewModel.IsValid() || !NewValue.IsValid())
	{
		return;
	}
	ViewModel->SetCompatibilityMountProfileId(NewValue->MountProfileId);
	LastStatusMessage = FString::Printf(TEXT("장착 프로파일 '%s'의 호환성을 다시 계산했습니다."), *NewValue->MountProfileId.ToString());
}

// current Vehicle/Mount/Draft source를 fresh read-only compatibility로 다시 평가합니다.
FReply SCFEquipmentBuilderTab::HandleRefreshCompatibility()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	ViewModel->RefreshCompatibilityProbe();
	RefreshCompatibilityMountItemsFromViewModel();
	LastStatusMessage = TEXT("선택한 차량과 현재 장착 프로파일 목록을 다시 읽었습니다. 장비 검토/저장 상태는 변경되지 않습니다.");
	return FReply::Handled();
}

// registered Vehicle Builder Nomad Tab만 열고 context/business state는 전달하지 않습니다.
FReply SCFEquipmentBuilderTab::HandleOpenVehicleBuilder()
{
	// Cross-Builder P0가 허용하는 registered Vehicle Builder tab identity입니다.
	static const FName VehicleBuilderTabName(TEXT("CarFight.VehicleBuilder"));
	FGlobalTabmanager::Get()->TryInvokeTab(VehicleBuilderTabName);
	LastStatusMessage = TEXT("차량 제작 가이드를 열었습니다. 현재 차량/장착 지점/장비 선택은 전달하지 않았습니다.");
	return FReply::Handled();
}

// MountType combo option 한 행을 생성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::GenerateMountTypeOption(TSharedPtr<ECFVehicleMountType> Value) const
{
	return SNew(STextBlock).Text(Value.IsValid() ? GetMountTypeLabel(*Value) : FText::GetEmpty());
}

// WeaponSize combo option 한 행을 생성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::GenerateWeaponSizeOption(TSharedPtr<ECFVehicleWeaponSize> Value) const
{
	return SNew(STextBlock).Text(Value.IsValid() ? GetWeaponSizeLabel(*Value) : FText::GetEmpty());
}

// compatibility MountProfile combo option 한 행을 생성합니다.
TSharedRef<SWidget> SCFEquipmentBuilderTab::GenerateCompatibilityMountOption(FMountRowPtr Value) const
{
	if (!Value.IsValid())
	{
		return SNew(STextBlock).Text(FText::GetEmpty());
	}
	return SNew(STextBlock).Text(FText::FromString(FString::Printf(
		TEXT("%s / %s / %s"),
		*Value->MountProfileId.ToString(),
		*GetMountTypeLabel(Value->MountType).ToString(),
		*GetWeaponSizeLabel(Value->SizeLimit).ToString())));
}

// browser shared rows를 ViewModel metadata projection에서 다시 만듭니다.
void SCFEquipmentBuilderTab::RefreshPresetItemsFromViewModel()
{
	PresetItems.Reset();
	if (ViewModel.IsValid())
	{
		for (const FCFEquipmentPresetListEntry& Entry : ViewModel->GetPresetEntries())
		{
			PresetItems.Add(MakeShared<FCFEquipmentPresetListEntry>(Entry));
		}
	}
	if (!PresetListView.IsValid())
	{
		return;
	}
	PresetListView->RequestListRefresh();
	bSynchronizingSelection = true;
	PresetListView->ClearSelection();
	if (ViewModel.IsValid() && !ViewModel->GetSelectedObjectPath().IsEmpty())
	{
		for (const FPresetRowPtr& Item : PresetItems)
		{
			if (Item.IsValid() && Item->ObjectPath == ViewModel->GetSelectedObjectPath())
			{
				PresetListView->SetSelection(Item, ESelectInfo::Direct);
				break;
			}
		}
	}
	bSynchronizingSelection = false;
}

// compatibility MountProfile shared rows를 ViewModel read-only projection에서 다시 만듭니다.
void SCFEquipmentBuilderTab::RefreshCompatibilityMountItemsFromViewModel()
{
	CompatibilityMountItems.Reset();
	if (ViewModel.IsValid())
	{
		for (const FCFEquipmentMountOption& MountOption : ViewModel->GetCompatibilityMountOptions())
		{
			CompatibilityMountItems.Add(MakeShared<FCFEquipmentMountOption>(MountOption));
		}
	}
	if (!CompatibilityMountCombo.IsValid())
	{
		return;
	}
	CompatibilityMountCombo->RefreshOptions();
	CompatibilityMountCombo->ClearSelection();
	if (!ViewModel.IsValid() || ViewModel->GetCompatibilityMountProfileId().IsNone())
	{
		return;
	}
	for (const FMountRowPtr& MountItem : CompatibilityMountItems)
	{
		if (MountItem.IsValid() && MountItem->MountProfileId == ViewModel->GetCompatibilityMountProfileId())
		{
			CompatibilityMountCombo->SetSelectedItem(MountItem);
			break;
		}
	}
}

// current target/draft 전환 시 오른쪽 common scroll을 상단으로 되돌립니다.
void SCFEquipmentBuilderTab::ResetDraftScrollToStart()
{
	if (DraftScrollBox.IsValid())
	{
		DraftScrollBox->ScrollToStart();
	}
}

// current 작업 대상을 사용자용 문장으로 반환합니다.
FText SCFEquipmentBuilderTab::GetTargetText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasActiveDraft())
	{
		return LOCTEXT("NoTarget", "작업 대상: 선택 없음");
	}
	if (ViewModel->IsNewDraft())
	{
		return ViewModel->GetCreateTargetObjectPath().IsEmpty()
			? LOCTEXT("NewTarget", "작업 대상: 새 장비 프리셋 초안 — 아래에서 에셋 이름을 정하세요. 내부 ID와 저장 위치는 자동으로 결정됩니다.")
			: FText::FromString(FString::Printf(TEXT("작업 대상: 새 장비 프리셋 '%s' → %s"), *ViewModel->GetCreateAssetName(), *ViewModel->GetCreateTargetObjectPath()));
	}
	return FText::FromString(FString::Printf(TEXT("작업 대상: %s"), *ViewModel->GetSelectedObjectPath()));
}

// current transient draft 상태를 사용자용 문장으로 반환합니다.
FText SCFEquipmentBuilderTab::GetDraftStateText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::FromString(LastStatusMessage);
	}
	return FText::FromString(FString::Printf(TEXT("%s\n%s"), *ViewModel->BuildDraftStateSummary(), *LastStatusMessage));
}

// current Gate에서 사용자가 할 일을 반환합니다.
FText SCFEquipmentBuilderTab::GetCurrentActionText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasActiveDraft())
	{
		return LOCTEXT("ActionSelect", "왼쪽에서 기존 장비 프리셋을 선택하거나 '+ 새 장비 프리셋'을 눌러 초안을 시작하세요.");
	}
	if (ViewModel->GetReviewState().ApprovalState == ECFEquipmentApprovalState::Reviewed)
	{
		return LOCTEXT("ActionApply", "검토가 완료됐습니다. 초안과 저장 대상을 바꾸지 않았다면 '적용 및 저장'으로 현재 장비 프리셋 하나만 저장할 수 있습니다.");
	}
	return ViewModel->GetDraft().DraftMode == ECFEquipmentDraftMode::Weapon
		? LOCTEXT("ActionWeapon", "터렛 마운트 데이터와 무기 데이터가 없으면 '새로 만들기…'로 제작을 시작하고, 각 DataAsset을 저장한 뒤 이 화면에서 선택해 검증하고 '검토'를 누르세요.")
		: LOCTEXT("ActionScanner", "센서 데이터가 없으면 '새로 만들기…'로 제작을 시작하고, Sensor DataAsset을 저장한 뒤 이 화면에서 선택해 검증하고 '검토'를 누르세요.");
}

// Product Asset mutation 여부와 draft/review 상태를 명확히 표시합니다.
FText SCFEquipmentBuilderTab::GetMutationSafetyText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasActiveDraft())
	{
		return LOCTEXT("MutationIdle", "장비 프리셋 변경/저장: 없음 — 작업 대상을 선택하지 않았습니다. 하위 DataAsset 제작은 별도 생성·편집·저장 흐름입니다.");
	}
	if (ViewModel->GetReviewState().ApprovalState == ECFEquipmentApprovalState::Reviewed)
	{
		return LOCTEXT("MutationReviewed", "검토 완료 / 아직 장비 프리셋 저장 전 — '적용 및 저장'은 현재 장비 프리셋 하나만 저장합니다. 새로 만든 하위 DataAsset은 각 데이터 에디터에서 따로 저장해야 합니다.");
	}
	return ViewModel->HasDraftChanges()
		? LOCTEXT("MutationDirty", "장비 프리셋 초안 변경 있음 / 아직 저장 전 — 변경 내용은 임시 초안에만 있습니다. 하위 DataAsset의 생성·편집·저장은 이 저장과 분리됩니다.")
		: LOCTEXT("MutationClean", "장비 프리셋 초안 변경 없음 / 아직 저장 전 / 하위 DataAsset 저장은 별도 흐름");
}

// current Review/approval 상태를 사용자용 문장으로 반환합니다.
FText SCFEquipmentBuilderTab::GetReviewSummaryText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasActiveDraft())
	{
		return LOCTEXT("ReviewIdle", "검토 상태: 대상 없음");
	}
	// Current one-shot Review state입니다.
	const FCFEquipmentReviewState& Review = ViewModel->GetReviewState();
	if (Review.ApprovalState == ECFEquipmentApprovalState::Reviewed)
	{
		return LOCTEXT("ReviewReady", "검토 상태: 검토 완료 — 현재 초안과 저장 대상이 승인되었습니다. 초안을 바꾸면 다시 검토해야 합니다.");
	}
	if (Review.ApprovalState == ECFEquipmentApprovalState::Consumed)
	{
		return LOCTEXT("ReviewConsumed", "검토 상태: 사용 완료 — 이전 검토 결과는 재사용할 수 없습니다. 현재 상태를 확인한 뒤 다시 검토하세요.");
	}
	return LOCTEXT("ReviewUnreviewed", "검토 상태: 미검토 — 현재 초안과 저장 대상을 검토해야 '적용 및 저장'할 수 있습니다.");
}

// current Draft가 Apply 가능한 Reviewed 상태인지 반환합니다.
bool SCFEquipmentBuilderTab::CanApplyReviewedDraft() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasActiveDraft()
		&& ViewModel->GetReviewState().ApprovalState == ECFEquipmentApprovalState::Reviewed;
}

// UI-only Draft Mode 버튼 label을 반환합니다.
FText SCFEquipmentBuilderTab::GetDraftModeButtonText(ECFEquipmentDraftMode DraftMode) const
{
	// current selected UI-only mode입니다.
	const bool bSelected = ViewModel.IsValid() && ViewModel->HasActiveDraft() && ViewModel->GetDraft().DraftMode == DraftMode;
	if (DraftMode == ECFEquipmentDraftMode::Scanner)
	{
		return FText::FromString(bSelected ? TEXT("● 스캐너 장비") : TEXT("○ 스캐너 장비"));
	}
	return FText::FromString(bSelected ? TEXT("● 무장 장비") : TEXT("○ 무장 장비"));
}

// current MountType label을 반환합니다.
FText SCFEquipmentBuilderTab::GetCurrentMountTypeText() const
{
	return ViewModel.IsValid() && ViewModel->HasActiveDraft() ? GetMountTypeLabel(ViewModel->GetDraft().RequiredMountType) : LOCTEXT("MountEmpty", "선택 없음");
}

// current WeaponSize label을 반환합니다.
FText SCFEquipmentBuilderTab::GetCurrentWeaponSizeText() const
{
	return ViewModel.IsValid() && ViewModel->HasActiveDraft() ? GetWeaponSizeLabel(ViewModel->GetDraft().RequiredWeaponSize) : LOCTEXT("SizeEmpty", "선택 없음");
}

// current Draft의 TurretMountData object path를 picker attribute용 문자열로 반환합니다.
FString SCFEquipmentBuilderTab::GetTurretMountObjectPath() const
{
	return ViewModel.IsValid() && ViewModel->HasActiveDraft()
		? ViewModel->GetDraft().DefaultTurretMountData.ToSoftObjectPath().ToString()
		: FString();
}

// current Draft의 WeaponData object path를 picker attribute용 문자열로 반환합니다.
FString SCFEquipmentBuilderTab::GetWeaponDataObjectPath() const
{
	return ViewModel.IsValid() && ViewModel->HasActiveDraft()
		? ViewModel->GetDraft().DefaultWeaponData.ToSoftObjectPath().ToString()
		: FString();
}

// current Draft의 VehicleSensorData object path를 picker attribute용 문자열로 반환합니다.
FString SCFEquipmentBuilderTab::GetSensorDataObjectPath() const
{
	return ViewModel.IsValid() && ViewModel->HasActiveDraft()
		? ViewModel->GetDraft().DefaultSensorData.ToSoftObjectPath().ToString()
		: FString();
}

// current Draft child validation 전체를 사용자용 다중 행 텍스트로 반환합니다.
FText SCFEquipmentBuilderTab::GetValidationSummaryText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasActiveDraft())
	{
		return LOCTEXT("ValidationNoDraft", "초안을 먼저 선택하세요.");
	}

	// current transient Draft의 fresh bounded validation 결과입니다.
	const FCFEquipmentDraftValidation Validation = ViewModel->BuildCurrentValidation();
	// validation header와 각 항목을 누적할 사용자용 문자열입니다.
	FString Summary = FString::Printf(
		TEXT("패키지 구성: %s / 차단 오류: %s"),
		Validation.bPackageComplete ? TEXT("완료") : TEXT("미완료"),
		Validation.bHasBlockingErrors ? TEXT("있음") : TEXT("없음"));
	for (const FCFEquipmentValidationItem& Item : Validation.Items)
	{
		Summary += FString::Printf(TEXT("\n%s %s — %s"), *GetValidationSeverityPrefix(Item.Severity), *Item.Title, *Item.Message);
	}
	return FText::FromString(Summary);
}

// compatibility VehicleData object path를 picker attribute용 문자열로 반환합니다.
FString SCFEquipmentBuilderTab::GetCompatibilityVehicleObjectPath() const
{
	return ViewModel.IsValid() ? ViewModel->GetCompatibilityVehicleDataPath().ToString() : FString();
}

// current compatibility MountProfile combo label을 반환합니다.
FText SCFEquipmentBuilderTab::GetCurrentCompatibilityMountText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetCompatibilityMountProfileId().IsNone())
	{
		return LOCTEXT("CompatibilityMountEmpty", "선택 없음");
	}
	for (const FMountRowPtr& MountItem : CompatibilityMountItems)
	{
		if (MountItem.IsValid() && MountItem->MountProfileId == ViewModel->GetCompatibilityMountProfileId())
		{
			return FText::FromString(FString::Printf(
				TEXT("%s / %s / %s"),
				*MountItem->MountProfileId.ToString(),
				*GetMountTypeLabel(MountItem->MountType).ToString(),
				*GetWeaponSizeLabel(MountItem->SizeLimit).ToString()));
		}
	}
	return FText::FromName(ViewModel->GetCompatibilityMountProfileId());
}

// exact3 compatibility state/reason을 사용자용 문장으로 반환합니다.
FText SCFEquipmentBuilderTab::GetCompatibilitySummaryText() const
{
	if (!ViewModel.IsValid())
	{
		return LOCTEXT("CompatibilityUnavailable", "호환성: 판정할 수 없음 — 내부 상태를 준비하지 못했습니다.");
	}
	// current exact3 advisory result입니다.
	const FCFEquipmentCompatibilityResult& Result = ViewModel->GetCompatibilityResult();
	// current exact3 state의 사용자 표시 문자열입니다.
	const TCHAR* StateText = Result.State == ECFEquipmentCompatibilityState::Compatible
		? TEXT("호환됨")
		: Result.State == ECFEquipmentCompatibilityState::Incompatible
			? TEXT("호환되지 않음")
			: TEXT("판정할 수 없음");
	// bounded reason의 사용자 표시 문자열입니다.
	FString ReasonText;
	switch (Result.Reason)
	{
	case ECFEquipmentCompatibilityReason::None: ReasonText = TEXT("이상 없음"); break;
	case ECFEquipmentCompatibilityReason::NoVehicleSelected: ReasonText = TEXT("차량 미선택"); break;
	case ECFEquipmentCompatibilityReason::VehicleSourceUnresolved: ReasonText = TEXT("차량 데이터 확인 실패"); break;
	case ECFEquipmentCompatibilityReason::VehicleSourceUnpersisted: ReasonText = TEXT("저장되지 않은 차량 데이터"); break;
	case ECFEquipmentCompatibilityReason::VehicleSourceDirty: ReasonText = TEXT("차량 데이터에 저장되지 않은 변경 있음"); break;
	case ECFEquipmentCompatibilityReason::MountProfileMissing: ReasonText = TEXT("장착 프로파일 없음"); break;
	case ECFEquipmentCompatibilityReason::MountProfileIdNone: ReasonText = TEXT("장착 프로파일 ID 없음"); break;
	case ECFEquipmentCompatibilityReason::MountProfileIdDuplicate: ReasonText = TEXT("장착 프로파일 ID 중복"); break;
	case ECFEquipmentCompatibilityReason::MountTypeNone: ReasonText = TEXT("장착 타입 없음"); break;
	case ECFEquipmentCompatibilityReason::EquipmentIntrinsicInvalid: ReasonText = TEXT("장비 자체 검증 실패"); break;
	case ECFEquipmentCompatibilityReason::RequiredMountTypeMismatch: ReasonText = TEXT("요구 장착 타입 불일치"); break;
	case ECFEquipmentCompatibilityReason::RequiredWeaponSizeTooLarge: ReasonText = TEXT("요구 장착 크기 초과"); break;
	case ECFEquipmentCompatibilityReason::WeaponMountUnsupported: ReasonText = TEXT("무기 데이터가 장착 타입을 지원하지 않음"); break;
	case ECFEquipmentCompatibilityReason::WeaponSizeUnsupported: ReasonText = TEXT("무기 데이터가 장착 크기를 지원하지 않음"); break;
	case ECFEquipmentCompatibilityReason::RuntimeContractRejected: ReasonText = TEXT("장비 장착 계약 불일치"); break;
	default: ReasonText = TEXT("알 수 없는 이유"); break;
	}
	return FText::FromString(FString::Printf(TEXT("호환성: %s / 이유: %s"), StateText, *ReasonText));
}

// validation severity를 사용자용 prefix로 변환합니다.
FString SCFEquipmentBuilderTab::GetValidationSeverityPrefix(ECFEquipmentValidationSeverity Severity)
{
	switch (Severity)
	{
	case ECFEquipmentValidationSeverity::Pass:
		return TEXT("[통과]");
	case ECFEquipmentValidationSeverity::Limited:
		return TEXT("[제한 검증]");
	case ECFEquipmentValidationSeverity::Warning:
		return TEXT("[확인 필요]");
	case ECFEquipmentValidationSeverity::Error:
	default:
		return TEXT("[오류]");
	}
}

// MountType enum을 사용자용 한글 label로 변환합니다.
FText SCFEquipmentBuilderTab::GetMountTypeLabel(ECFVehicleMountType MountType)
{
	switch (MountType)
	{
	case ECFVehicleMountType::Fixed: return LOCTEXT("MountFixed", "고정");
	case ECFVehicleMountType::Gimbal: return LOCTEXT("MountGimbal", "짐벌");
	case ECFVehicleMountType::Turret: return LOCTEXT("MountTurret", "터렛");
	case ECFVehicleMountType::Launcher: return LOCTEXT("MountLauncher", "런처");
	case ECFVehicleMountType::Utility: return LOCTEXT("MountUtility", "유틸리티");
	case ECFVehicleMountType::None:
	default: return LOCTEXT("MountNone", "제한 없음");
	}
}

// WeaponSize enum을 사용자용 한글 label로 변환합니다.
FText SCFEquipmentBuilderTab::GetWeaponSizeLabel(ECFVehicleWeaponSize WeaponSize)
{
	switch (WeaponSize)
	{
	case ECFVehicleWeaponSize::Small: return LOCTEXT("SizeSmall", "소형");
	case ECFVehicleWeaponSize::Medium: return LOCTEXT("SizeMedium", "중형");
	case ECFVehicleWeaponSize::Large: return LOCTEXT("SizeLarge", "대형");
	case ECFVehicleWeaponSize::None:
	default: return LOCTEXT("SizeNone", "해당 없음");
	}
}

// soft object path를 비어 있음/실제 path 사용자 표시로 변환합니다.
FString SCFEquipmentBuilderTab::FormatReferencePath(const FSoftObjectPath& ObjectPath)
{
	return ObjectPath.IsNull() ? TEXT("지정 안 됨") : ObjectPath.ToString();
}

#undef LOCTEXT_NAMESPACE
