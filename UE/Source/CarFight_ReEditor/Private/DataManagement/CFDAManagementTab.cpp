// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAManagementTab.cpp
// Version: v1.6.0
// Date: 2026-09-04
// Description: CF-FQ-045 DAM-P0-04E 재검사 필요 상태와 사용자 중심 유형 표현 UX 교정 구현입니다.
// Changelog:
// - v1.6.0: Refresh 후 과거 검사값은 재사용하지 않으면서 상태/고유 ID를 재검사 필요/재확인 필요로 구분하고, 추상 유형을 직접 에셋 생성 가능 여부로 교체.
// - v1.5.0: 기술 정보 상태값, Asset View 액션, 참조 관계, 검색/뷰/상태 문구를 한글 우선 표현으로 교정하고 기술 식별값만 원문 유지.
// - v1.4.0: InventoryGeneration 사용자 노출 제거, Stable ID 4상태 표시, Referencer 삭제 안전 경고, list rebuild 후 Slate selection 재결합을 구현.
// - v1.3.0: Type/Asset header click sort, sort indicator와 Purpose ellipsis를 구현.
// - v1.2.0: user-first Overview/Detail, technical expandable section과 Asset-only action visibility를 구현.
// - v1.1.0: 문자열 summary row를 actual Multi-column Type/Asset table로 교체하고 view-specific filter surface를 추가.
// - v1.0.0: Overview, Type/Asset View, dynamic filters, Detail actions와 status surface를 구현.
// Migration:
// - UI mutation/save 없음. 모든 data operation은 FCFDAManagementVM을 경유합니다.

#include "DataManagement/CFDAManagementTab.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

namespace CFDAManagementTabPrivate
{
	FString DomainLabel(const ECFDADomain Domain)
	{
		switch (Domain)
		{
		case ECFDADomain::Vehicle: return TEXT("차량");
		case ECFDADomain::Combat: return TEXT("전투");
		case ECFDADomain::Targeting: return TEXT("타게팅");
		case ECFDADomain::UI: return TEXT("UI");
		case ECFDADomain::Authoring: return TEXT("제작");
		case ECFDADomain::Unclassified:
		default:
			return TEXT("미분류");
		}
	}

	FString ScopeLabel(const ECFDAScope Scope)
	{
		switch (Scope)
		{
		case ECFDAScope::Test: return TEXT("테스트");
		case ECFDAScope::Legacy: return TEXT("레거시");
		case ECFDAScope::Debug: return TEXT("디버그");
		case ECFDAScope::Authoring: return TEXT("제작");
		case ECFDAScope::RuntimeContent: return TEXT("런타임");
		case ECFDAScope::Unclassified:
		default:
			return TEXT("미분류");
		}
	}

	FString HealthLabel(const ECFDAHealthState Health)
	{
		switch (Health)
		{
		case ECFDAHealthState::OK: return TEXT("정상");
		case ECFDAHealthState::Warning: return TEXT("경고");
		case ECFDAHealthState::Error: return TEXT("오류");
		case ECFDAHealthState::NotApplicable: return TEXT("해당 없음");
		case ECFDAHealthState::NotValidated:
		default:
			return TEXT("미검사");
		}
	}

	FString CoverageLabel(const ECFDACoverageState Coverage)
	{
		return Coverage == ECFDACoverageState::Registered
			? TEXT("등록됨")
			: TEXT("미등록");
	}

	FString ManagementStateLabel(const ECFDAManagerTypePresentationState State)
	{
		switch (State)
		{
		case ECFDAManagerTypePresentationState::Managed: return TEXT("관리됨");
		case ECFDAManagerTypePresentationState::NeedsManagementRule: return TEXT("관리 규칙 필요");
		case ECFDAManagerTypePresentationState::Framework: return TEXT("프레임워크");
		case ECFDAManagerTypePresentationState::AuxiliaryDataAsset:
		default:
			return TEXT("기타 데이터 에셋");
		}
	}

	FString StableIdLabel(const ECFDAStableIdState State)
	{
		switch (State)
		{
		case ECFDAStableIdState::Resolved: return TEXT("확인됨");
		case ECFDAStableIdState::NotApplicable: return TEXT("해당 없음");
		case ECFDAStableIdState::MissingRequired: return TEXT("필수 ID 누락");
		case ECFDAStableIdState::NotResolved:
		default:
			return TEXT("미확인");
		}
	}

	FString EvaluationLabel(const ECFDAEvaluationState State)
	{
		switch (State)
		{
		case ECFDAEvaluationState::Succeeded: return TEXT("검사 완료");
		case ECFDAEvaluationState::PolicyUnavailable: return TEXT("검사 규칙 없음");
		case ECFDAEvaluationState::InvalidRequest: return TEXT("검사 요청 오류");
		case ECFDAEvaluationState::LoadFailed: return TEXT("불러오기 실패");
		case ECFDAEvaluationState::ClassMismatch: return TEXT("데이터 유형 불일치");
		case ECFDAEvaluationState::AdapterUnavailable: return TEXT("검사 기능 없음");
		case ECFDAEvaluationState::NotRequested:
		default:
			return TEXT("검사 전");
		}
	}

	FString DuplicateLabel(const ECFDADuplicateState State)
	{
		switch (State)
		{
		case ECFDADuplicateState::NotApplicable: return TEXT("해당 없음");
		case ECFDADuplicateState::Unique: return TEXT("중복 없음");
		case ECFDADuplicateState::Duplicate: return TEXT("중복 있음");
		case ECFDADuplicateState::NotAnalyzed:
		default:
			return TEXT("미검사");
		}
	}

	FString ReferencerLabel(const ECFDAReferencerState State)
	{
		switch (State)
		{
		case ECFDAReferencerState::Referenced: return TEXT("사용처 있음");
		case ECFDAReferencerState::NoKnownReferencer: return TEXT("알려진 사용처 없음");
		case ECFDAReferencerState::QueryFailed: return TEXT("조회 실패");
		case ECFDAReferencerState::NotQueried:
		default:
			return TEXT("조회 전");
		}
	}

	FString JoinPackages(const TArray<FString>& Packages)
	{
		return Packages.IsEmpty()
			? TEXT("(없음)")
			: FString::Join(Packages, TEXT("\n"));
	}

	const FName TypeColumnName(TEXT("Type"));
	const FName TypeDomainColumnName(TEXT("Domain"));
	const FName TypePurposeColumnName(TEXT("Purpose"));
	const FName TypeAssetCountColumnName(TEXT("AssetCount"));
	const FName TypeManagementStateColumnName(TEXT("ManagementState"));

	const FName AssetNameColumnName(TEXT("AssetName"));
	const FName AssetTypeColumnName(TEXT("AssetType"));
	const FName AssetDomainColumnName(TEXT("AssetDomain"));
	const FName AssetScopeColumnName(TEXT("AssetScope"));
	const FName AssetHealthColumnName(TEXT("AssetHealth"));
	const FName AssetStableIdColumnName(TEXT("StableId"));

	class SCFDAManagerTypeTableRow : public SMultiColumnTableRow<TSharedPtr<FCFDAManagerTypeRow>>
	{
	public:
		SLATE_BEGIN_ARGS(SCFDAManagerTypeTableRow) {}
			SLATE_ARGUMENT(TSharedPtr<FCFDAManagerTypeRow>, Item)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
		{
			Item = InArgs._Item;
			SMultiColumnTableRow<TSharedPtr<FCFDAManagerTypeRow>>::Construct(
				FSuperRowType::FArguments().Padding(FMargin(3.0f, 2.0f)),
				OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			if (!Item.IsValid())
			{
				return SNullWidget::NullWidget;
			}
			if (ColumnName == TypeColumnName)
			{
				return SNew(STextBlock).Text(FText::FromString(Item->DisplayName));
			}
			if (ColumnName == TypeDomainColumnName)
			{
				return SNew(STextBlock).Text(FText::FromString(DomainLabel(Item->Domain)));
			}
			if (ColumnName == TypePurposeColumnName)
			{
				const FString Purpose = Item->UserPurposeDescription.IsEmpty()
					? Item->RoleDescription
					: Item->UserPurposeDescription;
				const FString PurposeToolTip = Item->UserUsageDescription.IsEmpty()
					? Purpose
					: FString::Printf(TEXT("%s\n\n어디에 사용되는가\n%s"), *Purpose, *Item->UserUsageDescription);
				return SNew(STextBlock)
					.Text(FText::FromString(Purpose))
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.ToolTipText(FText::FromString(PurposeToolTip));
			}
			if (ColumnName == TypeAssetCountColumnName)
			{
				return SNew(STextBlock).Text(FText::AsNumber(Item->AssetCount));
			}
			if (ColumnName == TypeManagementStateColumnName)
			{
				return SNew(STextBlock).Text(FText::FromString(ManagementStateLabel(Item->PresentationState)));
			}
			return SNullWidget::NullWidget;
		}

	private:
		TSharedPtr<FCFDAManagerTypeRow> Item;
	};

	class SCFDAManagerAssetTableRow : public SMultiColumnTableRow<TSharedPtr<FCFDAManagerAssetRow>>
	{
	public:
		SLATE_BEGIN_ARGS(SCFDAManagerAssetTableRow) {}
			SLATE_ARGUMENT(TSharedPtr<FCFDAManagerAssetRow>, Item)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
		{
			Item = InArgs._Item;
			SMultiColumnTableRow<TSharedPtr<FCFDAManagerAssetRow>>::Construct(
				FSuperRowType::FArguments().Padding(FMargin(3.0f, 2.0f)),
				OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			if (!Item.IsValid())
			{
				return SNullWidget::NullWidget;
			}
			if (ColumnName == AssetNameColumnName)
			{
				return SNew(STextBlock).Text(FText::FromString(Item->AssetName)).ToolTipText(FText::FromString(Item->ObjectPath));
			}
			if (ColumnName == AssetTypeColumnName)
			{
				return SNew(STextBlock).Text(FText::FromString(Item->TypeDisplayName));
			}
			if (ColumnName == AssetDomainColumnName)
			{
				return SNew(STextBlock).Text(FText::FromString(DomainLabel(Item->Domain)));
			}
			if (ColumnName == AssetScopeColumnName)
			{
				return SNew(STextBlock).Text(FText::FromString(ScopeLabel(Item->Scope)));
			}
			if (ColumnName == AssetHealthColumnName)
			{
				return SNew(STextBlock).Text(
					FText::FromString(
						SCFDAManagementTab::BuildHealthUserText(
							Item->HealthState,
							Item->bNeedsRevalidation)));
			}
			if (ColumnName == AssetStableIdColumnName)
			{
				return SNew(STextBlock).Text(
					FText::FromString(
						SCFDAManagementTab::BuildStableIdUserText(
							Item->StableIdState,
							Item->StableId,
							Item->bNeedsRevalidation)));
			}
			return SNullWidget::NullWidget;
		}

	private:
		TSharedPtr<FCFDAManagerAssetRow> Item;
	};
}

// Health 상태와 재검사 필요 여부를 사용자에게 보여줄 문구로 변환합니다.
FString SCFDAManagementTab::BuildHealthUserText(
	const ECFDAHealthState HealthState,
	const bool bNeedsRevalidation)
{
	return bNeedsRevalidation
		? TEXT("재검사 필요")
		: CFDAManagementTabPrivate::HealthLabel(HealthState);
}

// Stable ID state와 실제 값을 사용자에게 보여줄 문구로 변환합니다.
FString SCFDAManagementTab::BuildStableIdUserText(
	const ECFDAStableIdState StableIdState,
	const FString& StableId)
{
	switch (StableIdState)
	{
	case ECFDAStableIdState::Resolved:
		return StableId.IsEmpty() ? TEXT("미확인") : StableId;
	case ECFDAStableIdState::NotApplicable:
		return TEXT("해당 없음");
	case ECFDAStableIdState::MissingRequired:
		return TEXT("필수 ID 누락");
	case ECFDAStableIdState::NotResolved:
	default:
		return TEXT("미확인");
	}
}

// Stable ID state와 재확인 필요 여부를 사용자에게 보여줄 문구로 변환합니다.
FString SCFDAManagementTab::BuildStableIdUserText(
	const ECFDAStableIdState StableIdState,
	const FString& StableId,
	const bool bNeedsRevalidation)
{
	return bNeedsRevalidation
		? TEXT("재확인 필요")
		: BuildStableIdUserText(StableIdState, StableId);
}

// Referencer 결과가 삭제 안전 판정으로 오해되지 않도록 필요한 안내 문구를 만듭니다.
FString SCFDAManagementTab::BuildReferencerSafetyText(
	const ECFDAReferencerState ReferencerState)
{
	return ReferencerState == ECFDAReferencerState::NoKnownReferencer
		? TEXT("안내: 현재 Asset Registry에서 알려진 사용처를 찾지 못했다는 뜻이며, 삭제해도 안전하다는 보장은 아닙니다.")
		: FString();
}

void SCFDAManagementTab::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FCFDAManagementVM>();

	FString InitializeError;
	if (!ViewModel->Initialize(InitializeError))
	{
		LastStatusMessage = InitializeError;
	}
	else
	{
		LastStatusMessage = TEXT("데이터 목록 준비 완료.");
	}

	RebuildFilterOptions();
	RefreshListItems();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 8.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("CarFight 데이터 관리")))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBorder)
				.Padding(8.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						const FCFDAManagerOverview Overview = ViewModel.IsValid() ? ViewModel->BuildOverview() : FCFDAManagerOverview();
						return FText::FromString(FString::Printf(TEXT("관리 대상 에셋\n%d"), Overview.ManagedAssetCount));
					})
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBorder)
				.Padding(8.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						const FCFDAManagerOverview Overview = ViewModel.IsValid() ? ViewModel->BuildOverview() : FCFDAManagerOverview();
						return FText::FromString(FString::Printf(TEXT("문제 발견\n%d"), Overview.ProblemAssetCount));
					})
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBorder)
				.Padding(8.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						const FCFDAManagerOverview Overview = ViewModel.IsValid() ? ViewModel->BuildOverview() : FCFDAManagerOverview();
						return FText::FromString(FString::Printf(TEXT("검사 필요\n%d"), Overview.ManagedNotValidatedAssetCount));
					})
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SBorder)
				.Padding(8.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						const FCFDAManagerOverview Overview = ViewModel.IsValid() ? ViewModel->BuildOverview() : FCFDAManagerOverview();
						return FText::FromString(FString::Printf(TEXT("관리 규칙 필요 유형\n%d"), Overview.NeedsManagementRuleTypeCount));
					})
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 6.0f, 8.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SAssignNew(SearchBox, SSearchBox)
				.HintText(FText::FromString(TEXT("이름, 용도, 사용처 검색")))
				.OnTextChanged(this, &SCFDAManagementTab::HandleSearchChanged)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SAssignNew(DomainComboBox, SComboBox<TSharedPtr<FCFDADomainFilterOption>>)
					.OptionsSource(&DomainOptions)
					.InitiallySelectedItem(SelectedDomainOption)
					.OnGenerateWidget(this, &SCFDAManagementTab::GenerateDomainOptionWidget)
					.OnSelectionChanged(this, &SCFDAManagementTab::HandleDomainChanged)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return FText::FromString(
								SelectedDomainOption.IsValid()
									? SelectedDomainOption->Label
									: TEXT("영역: 전체"));
						})
					]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SAssignNew(ManagementStateComboBox, SComboBox<TSharedPtr<FCFDAManagementStateFilterOption>>)
					.Visibility_Lambda([this]()
					{
						return ViewModel.IsValid() && ViewModel->GetViewMode() == ECFDAManagerViewMode::TypeView
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					.OptionsSource(&ManagementStateOptions)
					.InitiallySelectedItem(SelectedManagementStateOption)
					.OnGenerateWidget(this, &SCFDAManagementTab::GenerateManagementStateOptionWidget)
					.OnSelectionChanged(this, &SCFDAManagementTab::HandleManagementStateChanged)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return FText::FromString(
								SelectedManagementStateOption.IsValid()
									? SelectedManagementStateOption->Label
									: TEXT("관리 상태: 전체"));
						})
					]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SAssignNew(ScopeComboBox, SComboBox<TSharedPtr<FCFDAScopeFilterOption>>)
					.Visibility_Lambda([this]()
					{
						return ViewModel.IsValid() && ViewModel->GetViewMode() == ECFDAManagerViewMode::AssetView
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					.OptionsSource(&ScopeOptions)
					.InitiallySelectedItem(SelectedScopeOption)
					.OnGenerateWidget(this, &SCFDAManagementTab::GenerateScopeOptionWidget)
					.OnSelectionChanged(this, &SCFDAManagementTab::HandleScopeChanged)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return FText::FromString(
								SelectedScopeOption.IsValid()
									? SelectedScopeOption->Label
									: TEXT("사용 범위: 전체"));
						})
					]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SAssignNew(HealthComboBox, SComboBox<TSharedPtr<FCFDAHealthFilterOption>>)
					.Visibility_Lambda([this]()
					{
						return ViewModel.IsValid() && ViewModel->GetViewMode() == ECFDAManagerViewMode::AssetView
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					.OptionsSource(&HealthOptions)
					.InitiallySelectedItem(SelectedHealthOption)
					.OnGenerateWidget(this, &SCFDAManagementTab::GenerateHealthOptionWidget)
					.OnSelectionChanged(this, &SCFDAManagementTab::HandleHealthChanged)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return FText::FromString(
								SelectedHealthOption.IsValid()
									? SelectedHealthOption->Label
									: TEXT("상태: 전체"));
						})
					]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]()
				{
					return ViewModel.IsValid() && ViewModel->GetFilterState().bShowAllDiscovered
						? ECheckBoxState::Checked
						: ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged(this, &SCFDAManagementTab::HandleShowAllDiscoveredChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("전체 발견 보기")))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("새로고침")))
				.ToolTipText(FText::FromString(TEXT("저장된 에셋 목록을 다시 읽습니다. 이전 검사값은 현재 결과로 재사용하지 않고 '재검사 필요 / 재확인 필요'로 표시하며, 참조 관계 조회 결과는 초기화합니다.")))
				.OnClicked(this, &SCFDAManagementTab::HandleRefreshClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f, 8.0f, 6.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("유형 보기")))
				.OnClicked(this, &SCFDAManagementTab::HandleTypeViewClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("에셋 보기")))
				.OnClicked(this, &SCFDAManagementTab::HandleAssetViewClicked)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 0.0f, 8.0f, 6.0f)
		[
			SNew(SSplitter)

			+ SSplitter::Slot()
			.Value(0.60f)
			[
				SAssignNew(ViewSwitcher, SWidgetSwitcher)

				+ SWidgetSwitcher::Slot()
				[
					SAssignNew(TypeListView, SListView<TSharedPtr<FCFDAManagerTypeRow>>)
						.ListItemsSource(&TypeItems)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SCFDAManagementTab::GenerateTypeRow)
						.OnSelectionChanged(this, &SCFDAManagementTab::HandleTypeSelectionChanged)
						.HeaderRow
						(
							SNew(SHeaderRow)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::TypeColumnName)
							.DefaultLabel(FText::FromString(TEXT("데이터 유형")))
							.FillWidth(0.22f)
							.SortMode(this, &SCFDAManagementTab::GetTypeColumnSortMode, CFDAManagementTabPrivate::TypeColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleTypeColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::TypeDomainColumnName)
							.DefaultLabel(FText::FromString(TEXT("영역")))
							.FillWidth(0.11f)
							.SortMode(this, &SCFDAManagementTab::GetTypeColumnSortMode, CFDAManagementTabPrivate::TypeDomainColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleTypeColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::TypePurposeColumnName)
							.DefaultLabel(FText::FromString(TEXT("용도")))
							.FillWidth(0.37f)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::TypeAssetCountColumnName)
							.DefaultLabel(FText::FromString(TEXT("에셋 수")))
							.FillWidth(0.10f)
							.SortMode(this, &SCFDAManagementTab::GetTypeColumnSortMode, CFDAManagementTabPrivate::TypeAssetCountColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleTypeColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::TypeManagementStateColumnName)
							.DefaultLabel(FText::FromString(TEXT("관리 상태")))
							.FillWidth(0.20f)
							.SortMode(this, &SCFDAManagementTab::GetTypeColumnSortMode, CFDAManagementTabPrivate::TypeManagementStateColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleTypeColumnSort)
						)
				]

				+ SWidgetSwitcher::Slot()
				[
					SAssignNew(AssetListView, SListView<TSharedPtr<FCFDAManagerAssetRow>>)
						.ListItemsSource(&AssetItems)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SCFDAManagementTab::GenerateAssetRow)
						.OnSelectionChanged(this, &SCFDAManagementTab::HandleAssetSelectionChanged)
						.HeaderRow
						(
							SNew(SHeaderRow)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::AssetNameColumnName)
							.DefaultLabel(FText::FromString(TEXT("에셋 이름")))
							.FillWidth(0.22f)
							.SortMode(this, &SCFDAManagementTab::GetAssetColumnSortMode, CFDAManagementTabPrivate::AssetNameColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleAssetColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::AssetTypeColumnName)
							.DefaultLabel(FText::FromString(TEXT("데이터 유형")))
							.FillWidth(0.21f)
							.SortMode(this, &SCFDAManagementTab::GetAssetColumnSortMode, CFDAManagementTabPrivate::AssetTypeColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleAssetColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::AssetDomainColumnName)
							.DefaultLabel(FText::FromString(TEXT("영역")))
							.FillWidth(0.10f)
							.SortMode(this, &SCFDAManagementTab::GetAssetColumnSortMode, CFDAManagementTabPrivate::AssetDomainColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleAssetColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::AssetScopeColumnName)
							.DefaultLabel(FText::FromString(TEXT("사용 범위")))
							.FillWidth(0.14f)
							.SortMode(this, &SCFDAManagementTab::GetAssetColumnSortMode, CFDAManagementTabPrivate::AssetScopeColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleAssetColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::AssetHealthColumnName)
							.DefaultLabel(FText::FromString(TEXT("상태")))
							.FillWidth(0.13f)
							.SortMode(this, &SCFDAManagementTab::GetAssetColumnSortMode, CFDAManagementTabPrivate::AssetHealthColumnName)
							.CanManuallySort(true)
							.OnSort(this, &SCFDAManagementTab::HandleAssetColumnSort)
							+ SHeaderRow::Column(CFDAManagementTabPrivate::AssetStableIdColumnName)
							.DefaultLabel(FText::FromString(TEXT("고유 ID")))
							.FillWidth(0.20f)
						)
				]
			]

			+ SSplitter::Slot()
			.Value(0.40f)
			[
				SNew(SBorder)
				.Padding(10.0f)
				[
					SNew(SScrollBox)

					+ SScrollBox::Slot()
					[
						SNew(STextBlock)
							.Text(this, &SCFDAManagementTab::BuildDetailText)
							.AutoWrapText(true)
					]

					+ SScrollBox::Slot()
					.Padding(0.0f, 10.0f, 0.0f, 4.0f)
					[
						SNew(SExpandableArea)
						.InitiallyCollapsed(true)
						.AreaTitle(FText::FromString(TEXT("기술 정보")))
						.Visibility_Lambda([this]()
						{
							return ViewModel.IsValid()
								&& (!SelectedTypeClassPath.IsEmpty() || ViewModel->GetSelectedAssetRecord() != nullptr)
								? EVisibility::Visible
								: EVisibility::Collapsed;
						})
						.BodyContent()
						[
							SNew(STextBlock)
							.Text(this, &SCFDAManagementTab::BuildTechnicalDetailText)
							.AutoWrapText(true)
						]
					]

					+ SScrollBox::Slot()
					.Padding(0.0f, 10.0f, 0.0f, 4.0f)
					[
						SNew(SHorizontalBox)
						.Visibility_Lambda([this]()
						{
							return ViewModel.IsValid()
								&& ViewModel->GetViewMode() == ECFDAManagerViewMode::AssetView
								&& ViewModel->GetSelectedAssetRecord() != nullptr
								? EVisibility::Visible
								: EVisibility::Collapsed;
						})

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("검사")))
								.ToolTipText(FText::FromString(TEXT("선택한 에셋의 상태, 고유 ID, 중복 여부를 확인합니다.")))
								.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetSelectedAssetRecord() != nullptr; })
								.OnClicked(this, &SCFDAManagementTab::HandleValidateClicked)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("참조 관계 조회")))
								.ToolTipText(FText::FromString(TEXT("선택한 에셋이 무엇을 참조하고, 어디에서 사용되는지 확인합니다.")))
								.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetSelectedAssetRecord() != nullptr; })
								.OnClicked(this, &SCFDAManagementTab::HandleReferenceClicked)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("에셋 열기")))
								.ToolTipText(FText::FromString(TEXT("선택한 에셋을 에셋 편집기에서 엽니다.")))
								.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetSelectedAssetRecord() != nullptr; })
								.OnClicked(this, &SCFDAManagementTab::HandleOpenAssetClicked)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("콘텐츠 브라우저에서 찾기")))
								.ToolTipText(FText::FromString(TEXT("콘텐츠 브라우저에서 선택한 에셋의 위치를 표시합니다.")))
								.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetSelectedAssetRecord() != nullptr; })
								.OnClicked(this, &SCFDAManagementTab::HandleSyncBrowserClicked)
						]
					]

					+ SScrollBox::Slot()
					.Padding(0.0f, 10.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
							.Visibility_Lambda([this]()
							{
								return ViewModel.IsValid()
									&& ViewModel->GetViewMode() == ECFDAManagerViewMode::AssetView
									&& ViewModel->GetSelectedAssetRecord() != nullptr
									? EVisibility::Visible
									: EVisibility::Collapsed;
							})
							.Text(this, &SCFDAManagementTab::BuildReferenceText)
							.AutoWrapText(true)
					]
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(STextBlock)
				.Text(this, &SCFDAManagementTab::BuildStatusText)
				.AutoWrapText(true)
		]
	];

	if (ViewSwitcher.IsValid())
	{
		ViewSwitcher->SetActiveWidgetIndex(
			ViewModel->GetViewMode() == ECFDAManagerViewMode::TypeView ? 0 : 1);
	}
}

// Automation에서 실제 Type row selection callback 경로로 한 유형을 선택합니다.
bool SCFDAManagementTab::SelectTypeForTesting(const FString& ClassPath)
{
	if (!TypeListView.IsValid())
	{
		return false;
	}

	for (const TSharedPtr<FCFDAManagerTypeRow>& TypeItem : TypeItems)
	{
		if (TypeItem.IsValid() && TypeItem->ClassPath == ClassPath)
		{
			TypeListView->SetSelection(TypeItem, ESelectInfo::Direct);
			return true;
		}
	}

	return false;
}

// Automation에서 Type ListView가 실제로 highlight 중인 ClassPath를 확인합니다.
FString SCFDAManagementTab::GetSlateSelectedTypePathForTesting() const
{
	if (!TypeListView.IsValid())
	{
		return FString();
	}

	// 현재 Slate Type ListView가 선택한 item snapshot입니다.
	const TArray<TSharedPtr<FCFDAManagerTypeRow>> SelectedItems = TypeListView->GetSelectedItems();
	return SelectedItems.Num() == 1 && SelectedItems[0].IsValid()
		? SelectedItems[0]->ClassPath
		: FString();
}

// Automation에서 Asset ListView가 실제로 highlight 중인 ObjectPath를 확인합니다.
FString SCFDAManagementTab::GetSlateSelectedAssetPathForTesting() const
{
	if (!AssetListView.IsValid())
	{
		return FString();
	}

	// 현재 Slate Asset ListView가 선택한 item snapshot입니다.
	const TArray<TSharedPtr<FCFDAManagerAssetRow>> SelectedItems = AssetListView->GetSelectedItems();
	return SelectedItems.Num() == 1 && SelectedItems[0].IsValid()
		? SelectedItems[0]->ObjectPath
		: FString();
}

void SCFDAManagementTab::RebuildFilterOptions()
{
	const FCFDAManagerFilterState CurrentFilter =
		ViewModel.IsValid()
			? ViewModel->GetFilterState()
			: FCFDAManagerFilterState();

	DomainOptions.Reset();
	SelectedDomainOption.Reset();

	{
		TSharedPtr<FCFDADomainFilterOption> All = MakeShared<FCFDADomainFilterOption>();
		All->Label = TEXT("영역: 전체");
		DomainOptions.Add(All);
		if (!CurrentFilter.Domain.IsSet())
		{
			SelectedDomainOption = All;
		}
	}

	if (ViewModel.IsValid())
	{
		for (const ECFDADomain Domain : ViewModel->GetAvailableDomains())
		{
			TSharedPtr<FCFDADomainFilterOption> Option = MakeShared<FCFDADomainFilterOption>();
			Option->Value = Domain;
			Option->Label = FString::Printf(
				TEXT("영역: %s"),
				*CFDAManagementTabPrivate::DomainLabel(Domain));
			DomainOptions.Add(Option);
			if (CurrentFilter.Domain.IsSet() && CurrentFilter.Domain.GetValue() == Domain)
			{
				SelectedDomainOption = Option;
			}
		}
	}
	if (!SelectedDomainOption.IsValid() && !DomainOptions.IsEmpty())
	{
		SelectedDomainOption = DomainOptions[0];
		if (ViewModel.IsValid())
		{
			ViewModel->SetDomainFilter(TOptional<ECFDADomain>());
		}
	}

	ManagementStateOptions.Reset();
	SelectedManagementStateOption.Reset();
	{
		TSharedPtr<FCFDAManagementStateFilterOption> All = MakeShared<FCFDAManagementStateFilterOption>();
		All->Label = TEXT("관리 상태: 전체");
		ManagementStateOptions.Add(All);
		if (!CurrentFilter.ManagementState.IsSet())
		{
			SelectedManagementStateOption = All;
		}
	}

	const ECFDAManagerTypePresentationState ManagementStateOrder[] =
	{
		ECFDAManagerTypePresentationState::Managed,
		ECFDAManagerTypePresentationState::NeedsManagementRule,
		ECFDAManagerTypePresentationState::Framework,
		ECFDAManagerTypePresentationState::AuxiliaryDataAsset
	};
	for (const ECFDAManagerTypePresentationState State : ManagementStateOrder)
	{
		if (!CurrentFilter.bShowAllDiscovered && !FCFDAManagementVM::IsManagementUniverseState(State))
		{
			continue;
		}

		TSharedPtr<FCFDAManagementStateFilterOption> Option = MakeShared<FCFDAManagementStateFilterOption>();
		Option->Value = State;
		Option->Label = FString::Printf(
			TEXT("관리 상태: %s"),
			*CFDAManagementTabPrivate::ManagementStateLabel(State));
		ManagementStateOptions.Add(Option);
		if (CurrentFilter.ManagementState.IsSet() && CurrentFilter.ManagementState.GetValue() == State)
		{
			SelectedManagementStateOption = Option;
		}
	}
	if (!SelectedManagementStateOption.IsValid())
	{
		SelectedManagementStateOption = ManagementStateOptions[0];
		if (ViewModel.IsValid())
		{
			ViewModel->SetManagementStateFilter(TOptional<ECFDAManagerTypePresentationState>());
		}
	}

	ScopeOptions.Reset();
	SelectedScopeOption.Reset();
	{
		TSharedPtr<FCFDAScopeFilterOption> All = MakeShared<FCFDAScopeFilterOption>();
		All->Label = TEXT("사용 범위: 전체");
		ScopeOptions.Add(All);
		if (!CurrentFilter.Scope.IsSet())
		{
			SelectedScopeOption = All;
		}
	}

	const ECFDAScope ScopeOrder[] =
	{
		ECFDAScope::Test,
		ECFDAScope::Legacy,
		ECFDAScope::Debug,
		ECFDAScope::Authoring,
		ECFDAScope::RuntimeContent,
		ECFDAScope::Unclassified
	};
	for (const ECFDAScope Scope : ScopeOrder)
	{
		TSharedPtr<FCFDAScopeFilterOption> Option = MakeShared<FCFDAScopeFilterOption>();
		Option->Value = Scope;
		Option->Label = FString::Printf(
			TEXT("사용 범위: %s"),
			*CFDAManagementTabPrivate::ScopeLabel(Scope));
		ScopeOptions.Add(Option);
		if (CurrentFilter.Scope.IsSet() && CurrentFilter.Scope.GetValue() == Scope)
		{
			SelectedScopeOption = Option;
		}
	}
	if (!SelectedScopeOption.IsValid())
	{
		SelectedScopeOption = ScopeOptions[0];
		if (ViewModel.IsValid())
		{
			ViewModel->SetScopeFilter(TOptional<ECFDAScope>());
		}
	}

	HealthOptions.Reset();
	SelectedHealthOption.Reset();
	{
		TSharedPtr<FCFDAHealthFilterOption> All = MakeShared<FCFDAHealthFilterOption>();
		All->Label = TEXT("상태: 전체");
		HealthOptions.Add(All);
		if (!CurrentFilter.Health.IsSet())
		{
			SelectedHealthOption = All;
		}
	}

	const ECFDAHealthState HealthOrder[] =
	{
		ECFDAHealthState::OK,
		ECFDAHealthState::Warning,
		ECFDAHealthState::Error,
		ECFDAHealthState::NotValidated,
		ECFDAHealthState::NotApplicable
	};
	for (const ECFDAHealthState Health : HealthOrder)
	{
		TSharedPtr<FCFDAHealthFilterOption> Option = MakeShared<FCFDAHealthFilterOption>();
		Option->Value = Health;
		// NotValidated filter는 최초 미검사와 Refresh 후 재검사 필요를 함께 포함하므로 상위 의미로 표시합니다.
		const FString HealthFilterLabel =
			Health == ECFDAHealthState::NotValidated
				? TEXT("검사 필요")
				: CFDAManagementTabPrivate::HealthLabel(Health);
		Option->Label = FString::Printf(
			TEXT("상태: %s"),
			*HealthFilterLabel);
		HealthOptions.Add(Option);
		if (CurrentFilter.Health.IsSet() && CurrentFilter.Health.GetValue() == Health)
		{
			SelectedHealthOption = Option;
		}
	}
	if (!SelectedHealthOption.IsValid())
	{
		SelectedHealthOption = HealthOptions[0];
		if (ViewModel.IsValid())
		{
			ViewModel->SetHealthFilter(TOptional<ECFDAHealthState>());
		}
	}
}

void SCFDAManagementTab::RefreshListItems()
{
	TypeItems.Reset();
	AssetItems.Reset();

	if (ViewModel.IsValid())
	{
		for (FCFDAManagerTypeRow& Row : ViewModel->BuildFilteredTypeRows())
		{
			TypeItems.Add(MakeShared<FCFDAManagerTypeRow>(MoveTemp(Row)));
		}
		for (FCFDAManagerAssetRow& Row : ViewModel->BuildFilteredAssetRows())
		{
			AssetItems.Add(MakeShared<FCFDAManagerAssetRow>(MoveTemp(Row)));
		}
	}

	if (TypeListView.IsValid())
	{
		TypeListView->RequestListRefresh();
	}
	if (AssetListView.IsValid())
	{
		AssetListView->RequestListRefresh();
	}

	ReconcileVisibleSelections();
}

void SCFDAManagementTab::ReconcileVisibleSelections()
{
	// 재생성된 Type row 중 persistent ClassPath와 일치하는 새 Slate item입니다.
	TSharedPtr<FCFDAManagerTypeRow> VisibleSelectedTypeItem;
	if (!SelectedTypeClassPath.IsEmpty())
	{
		for (const TSharedPtr<FCFDAManagerTypeRow>& TypeItem : TypeItems)
		{
			if (TypeItem.IsValid() && TypeItem->ClassPath == SelectedTypeClassPath)
			{
				VisibleSelectedTypeItem = TypeItem;
				break;
			}
		}
	}

	if (TypeListView.IsValid())
	{
		if (VisibleSelectedTypeItem.IsValid())
		{
			TypeListView->SetSelection(VisibleSelectedTypeItem, ESelectInfo::Direct);
		}
		else
		{
			TypeListView->ClearSelection();
		}
	}
	if (!SelectedTypeClassPath.IsEmpty() && !VisibleSelectedTypeItem.IsValid())
	{
		SelectedTypeClassPath.Reset();
	}

	// VM이 보존한 Asset ObjectPath와 일치하는 재생성 후 새 Slate item입니다.
	const FString SelectedAssetObjectPath =
		ViewModel.IsValid()
			? ViewModel->GetSelectedObjectPath()
			: FString();

	// 재생성된 Asset row 중 persistent ObjectPath와 일치하는 새 Slate item입니다.
	TSharedPtr<FCFDAManagerAssetRow> VisibleSelectedAssetItem;
	if (!SelectedAssetObjectPath.IsEmpty())
	{
		for (const TSharedPtr<FCFDAManagerAssetRow>& AssetItem : AssetItems)
		{
			if (AssetItem.IsValid() && AssetItem->ObjectPath == SelectedAssetObjectPath)
			{
				VisibleSelectedAssetItem = AssetItem;
				break;
			}
		}
	}

	if (AssetListView.IsValid())
	{
		if (VisibleSelectedAssetItem.IsValid())
		{
			AssetListView->SetSelection(VisibleSelectedAssetItem, ESelectInfo::Direct);
		}
		else
		{
			AssetListView->ClearSelection();
		}
	}

	if (ViewModel.IsValid()
		&& !SelectedAssetObjectPath.IsEmpty()
		&& !VisibleSelectedAssetItem.IsValid())
	{
		ViewModel->ClearSelection();
	}
}

void SCFDAManagementTab::RefreshAfterStateChange()
{
	RefreshListItems();
}

FText SCFDAManagementTab::BuildOverviewText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::FromString(TEXT("데이터 관리 화면을 준비하지 못했습니다."));
	}

	const FCFDAManagerOverview Overview = ViewModel->BuildOverview();
	return FText::FromString(FString::Printf(
		TEXT("관리 대상 에셋 %d / 문제 발견 %d / 검사 필요 %d / 관리 규칙 필요 유형 %d"),
		Overview.ManagedAssetCount,
		Overview.ProblemAssetCount,
		Overview.ManagedNotValidatedAssetCount,
		Overview.NeedsManagementRuleTypeCount));
}

FText SCFDAManagementTab::BuildDetailText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::FromString(TEXT("상세 정보를 표시할 수 없습니다."));
	}

	if (const FCFDAAssetRecord* AssetRecord = ViewModel->GetSelectedAssetRecord())
	{
		const FCFDAResolvedSemantic Semantic =
			ViewModel->GetTypeRegistry().ResolveSemantic(AssetRecord->ClassPath, AssetRecord->ClassPath);
		const FCFDALoadedAssetResult* Loaded = ViewModel->GetSelectedLoadedResult();

		// 현재 Detail에 표시할 Health 상태입니다.
		const ECFDAHealthState Health = Loaded ? Loaded->HealthState : AssetRecord->HealthState;

		// 이전 generation에서 검사했지만 current snapshot 기준 재검사가 필요한 상태인지 나타냅니다.
		const bool bNeedsRevalidation = ViewModel->DoesSelectedAssetNeedRevalidation();

		// 현재 Health와 재검사 필요 여부를 합쳐 사용자에게 보여줄 상태 문구입니다.
		const FString HealthUserText = BuildHealthUserText(Health, bNeedsRevalidation);

		// Validate 전 metadata 상태 또는 Validate 후 loaded 결과에서 가져온 Stable ID 상태입니다.
		const ECFDAStableIdState StableIdState =
			Loaded
				? Loaded->StableIdState
				: AssetRecord->StableIdState;

		// 실제 Stable ID 값이 확인된 경우에만 값 자체를 전달합니다.
		const FString StableId =
			Loaded
				? Loaded->StableId
				: AssetRecord->StableId;

		// 사용자에게는 미확인/해당 없음/필수 누락/실제 값과 Refresh 후 재확인 필요를 구분해 표시합니다.
		const FString StableIdUserText = BuildStableIdUserText(StableIdState, StableId, bNeedsRevalidation);

		return FText::FromString(FString::Printf(
			TEXT("%s\n%s · %s\n\n어떤 데이터인가\n%s\n\n어디에 사용되는가\n%s\n\n현재 상태\n상태: %s\n고유 ID: %s"),
			*AssetRecord->AssetName,
			*Semantic.Descriptor.TypeDisplayName,
			*CFDAManagementTabPrivate::DomainLabel(Semantic.Descriptor.Domain),
			*Semantic.Descriptor.UserPurposeDescription,
			*Semantic.Descriptor.UserUsageDescription,
			*HealthUserText,
			*StableIdUserText));
	}

	if (!SelectedTypeClassPath.IsEmpty())
	{
		for (const FCFDATypeRecord& TypeRecord : ViewModel->GetInventory().TypeRecords)
		{
			if (TypeRecord.ClassPath != SelectedTypeClassPath)
			{
				continue;
			}

			const FCFDAResolvedSemantic Semantic =
				ViewModel->GetTypeRegistry().ResolveSemantic(TypeRecord.ClassPath, TypeRecord.TechnicalClassName);
			const ECFDAManagerTypePresentationState PresentationState =
				FCFDAManagementVM::ResolvePresentationState(TypeRecord);
			return FText::FromString(FString::Printf(
				TEXT("%s\n%s · %s\n\n어떤 데이터인가\n%s\n\n어디에 사용되는가\n%s\n\n현재 상태\n에셋 %d개 · %s"),
				*Semantic.Descriptor.TypeDisplayName,
				*CFDAManagementTabPrivate::DomainLabel(Semantic.Descriptor.Domain),
				*CFDAManagementTabPrivate::ManagementStateLabel(PresentationState),
				*Semantic.Descriptor.UserPurposeDescription,
				*Semantic.Descriptor.UserUsageDescription,
				TypeRecord.AssetCount,
				*CFDAManagementTabPrivate::ManagementStateLabel(PresentationState)));
		}
	}

	return FText::FromString(
		TEXT("왼쪽 표에서 데이터 유형 또는 에셋을 선택하세요.\n\n기본 화면은 관리에 필요한 설명을 먼저 보여주고, 세부 클래스 정보는 '기술 정보'에서 확인할 수 있습니다."));
}

FText SCFDAManagementTab::BuildTechnicalDetailText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	if (const FCFDAAssetRecord* AssetRecord = ViewModel->GetSelectedAssetRecord())
	{
		const FCFDAResolvedSemantic Semantic =
			ViewModel->GetTypeRegistry().ResolveSemantic(AssetRecord->ClassPath, AssetRecord->ClassPath);
		const FCFDALoadedAssetResult* Loaded = ViewModel->GetSelectedLoadedResult();
		const ECFDAEvaluationState Evaluation = Loaded ? Loaded->EvaluationState : ECFDAEvaluationState::NotRequested;
		const ECFDAStableIdState StableState = Loaded ? Loaded->StableIdState : AssetRecord->StableIdState;
		const ECFDADuplicateState Duplicate = Loaded ? Loaded->DuplicateState : ECFDADuplicateState::NotAnalyzed;

		// 이전 generation의 검사 결과를 현재값으로 오해하지 않도록 별도 stale 표시를 사용합니다.
		const bool bNeedsRevalidation = ViewModel->DoesSelectedAssetNeedRevalidation();

		// 현재 기술 정보에 표시할 검사 상태 문구입니다.
		const FString EvaluationText = bNeedsRevalidation
			? TEXT("재검사 필요")
			: CFDAManagementTabPrivate::EvaluationLabel(Evaluation);

		// 현재 기술 정보에 표시할 고유 ID 상태 문구입니다.
		const FString StableIdStateText = bNeedsRevalidation
			? TEXT("재확인 필요")
			: CFDAManagementTabPrivate::StableIdLabel(StableState);

		// 중복 여부도 loaded 검사 결과에 포함되므로 stale이면 함께 재검사가 필요합니다.
		const FString DuplicateStateText = bNeedsRevalidation
			? TEXT("재검사 필요")
			: CFDAManagementTabPrivate::DuplicateLabel(Duplicate);

		FString Detail = FString::Printf(
			TEXT("오브젝트 경로 (Object Path): %s\n클래스 경로 (Class Path): %s\n관리 정보 등록: %s\n사용 범위: %s\n검사 상태: %s\n고유 ID 상태: %s\n중복 상태: %s\n\n기술 역할\n%s"),
			*AssetRecord->ObjectPath,
			*AssetRecord->ClassPath,
			*CFDAManagementTabPrivate::CoverageLabel(Semantic.CoverageState),
			*CFDAManagementTabPrivate::ScopeLabel(AssetRecord->Scope),
			*EvaluationText,
			*StableIdStateText,
			*DuplicateStateText,
			*Semantic.Descriptor.RoleDescription);
		if (Loaded && !Loaded->Messages.IsEmpty())
		{
			Detail += TEXT("\n\n검사 / ID 메시지\n");
			Detail += FString::Join(Loaded->Messages, TEXT("\n"));
		}
		return FText::FromString(Detail);
	}

	if (!SelectedTypeClassPath.IsEmpty())
	{
		for (const FCFDATypeRecord& TypeRecord : ViewModel->GetInventory().TypeRecords)
		{
			if (TypeRecord.ClassPath != SelectedTypeClassPath)
			{
				continue;
			}
			const FCFDAResolvedSemantic Semantic =
				ViewModel->GetTypeRegistry().ResolveSemantic(TypeRecord.ClassPath, TypeRecord.TechnicalClassName);
			return FText::FromString(FString::Printf(
				TEXT("클래스 경로 (Class Path): %s\n관리 정보 등록: %s\n직접 에셋 생성: %s\nCarFight 기본 C++ 유형: %s\n저장된 에셋 수: %d\n에셋 존재 상태: %s\n\n기술 역할\n%s"),
				*TypeRecord.ClassPath,
				*CFDAManagementTabPrivate::CoverageLabel(Semantic.CoverageState),
				TypeRecord.bAbstract ? TEXT("불가") : TEXT("가능"),
				TypeRecord.bCanonicalNative ? TEXT("예") : TEXT("아니오"),
				TypeRecord.AssetCount,
				TypeRecord.TypeInstanceState == ECFDATypeInstanceState::NoAssetInstance ? TEXT("저장된 에셋 없음") : TEXT("저장된 에셋 있음"),
				*Semantic.Descriptor.RoleDescription));
		}
	}

	return FText::GetEmpty();
}

FText SCFDAManagementTab::BuildReferenceText() const
{
	if (!ViewModel.IsValid() || !ViewModel->GetSelectedAssetRecord())
	{
		return FText::FromString(TEXT("참조 관계: 에셋을 선택하세요."));
	}

	const FCFDAReferenceResult* ReferenceResult =
		ViewModel->GetSelectedReferenceResult();
	if (!ReferenceResult)
	{
		return FText::FromString(TEXT("참조 관계: 아직 조회하지 않음"));
	}

	if (ReferenceResult->QueryState != ECFDAReferenceQueryState::Succeeded)
	{
		return FText::FromString(FString::Printf(
			TEXT("참조 관계 조회 실패\n%s"),
			*ReferenceResult->Message));
	}

	// No Known Referencer가 삭제 안전 판정으로 오해되지 않도록 필요한 경우 사용자 경고를 덧붙입니다.
	const FString SafetyText = BuildReferencerSafetyText(ReferenceResult->ReferencerState);

	return FText::FromString(FString::Printf(
		TEXT("사용처 상태: %s\n\n이 에셋이 참조하는 대상 (%d)\n%s\n\n이 에셋을 참조하는 대상 / 사용처 (%d)\n%s%s%s"),
		*CFDAManagementTabPrivate::ReferencerLabel(ReferenceResult->ReferencerState),
		ReferenceResult->References.Num(),
		*CFDAManagementTabPrivate::JoinPackages(ReferenceResult->References),
		ReferenceResult->Referencers.Num(),
		*CFDAManagementTabPrivate::JoinPackages(ReferenceResult->Referencers),
		SafetyText.IsEmpty() ? TEXT("") : TEXT("\n\n"),
		*SafetyText));
}

FText SCFDAManagementTab::BuildStatusText() const
{
	return FText::FromString(LastStatusMessage);
}

TSharedRef<SWidget> SCFDAManagementTab::GenerateDomainOptionWidget(
	TSharedPtr<FCFDADomainFilterOption> Option) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Option.IsValid() ? Option->Label : TEXT("영역: 전체")));
}

TSharedRef<SWidget> SCFDAManagementTab::GenerateManagementStateOptionWidget(
	TSharedPtr<FCFDAManagementStateFilterOption> Option) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Option.IsValid() ? Option->Label : TEXT("관리 상태: 전체")));
}

TSharedRef<SWidget> SCFDAManagementTab::GenerateScopeOptionWidget(
	TSharedPtr<FCFDAScopeFilterOption> Option) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Option.IsValid() ? Option->Label : TEXT("사용 범위: 전체")));
}

TSharedRef<SWidget> SCFDAManagementTab::GenerateHealthOptionWidget(
	TSharedPtr<FCFDAHealthFilterOption> Option) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Option.IsValid() ? Option->Label : TEXT("상태: 전체")));
}

TSharedRef<ITableRow> SCFDAManagementTab::GenerateTypeRow(
	TSharedPtr<FCFDAManagerTypeRow> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(CFDAManagementTabPrivate::SCFDAManagerTypeTableRow, OwnerTable)
		.Item(Item);
}

TSharedRef<ITableRow> SCFDAManagementTab::GenerateAssetRow(
	TSharedPtr<FCFDAManagerAssetRow> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(CFDAManagementTabPrivate::SCFDAManagerAssetTableRow, OwnerTable)
		.Item(Item);
}

EColumnSortMode::Type SCFDAManagementTab::GetTypeColumnSortMode(const FName ColumnId) const
{
	if (!ViewModel.IsValid())
	{
		return EColumnSortMode::None;
	}

	bool bMatchesCurrentSort = false;
	switch (ViewModel->GetTypeSortKey())
	{
	case ECFDAManagerTypeSortKey::Domain:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::TypeDomainColumnName;
		break;
	case ECFDAManagerTypeSortKey::AssetCount:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::TypeAssetCountColumnName;
		break;
	case ECFDAManagerTypeSortKey::ManagementState:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::TypeManagementStateColumnName;
		break;
	case ECFDAManagerTypeSortKey::DisplayName:
	default:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::TypeColumnName;
		break;
	}

	if (!bMatchesCurrentSort)
	{
		return EColumnSortMode::None;
	}

	return ViewModel->GetTypeSortDirection() == ECFDAManagerSortDirection::Ascending
		? EColumnSortMode::Ascending
		: EColumnSortMode::Descending;
}

EColumnSortMode::Type SCFDAManagementTab::GetAssetColumnSortMode(const FName ColumnId) const
{
	if (!ViewModel.IsValid())
	{
		return EColumnSortMode::None;
	}

	bool bMatchesCurrentSort = false;
	switch (ViewModel->GetAssetSortKey())
	{
	case ECFDAManagerAssetSortKey::TypeDisplayName:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::AssetTypeColumnName;
		break;
	case ECFDAManagerAssetSortKey::Domain:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::AssetDomainColumnName;
		break;
	case ECFDAManagerAssetSortKey::Scope:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::AssetScopeColumnName;
		break;
	case ECFDAManagerAssetSortKey::Health:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::AssetHealthColumnName;
		break;
	case ECFDAManagerAssetSortKey::AssetName:
	default:
		bMatchesCurrentSort = ColumnId == CFDAManagementTabPrivate::AssetNameColumnName;
		break;
	}

	if (!bMatchesCurrentSort)
	{
		return EColumnSortMode::None;
	}

	return ViewModel->GetAssetSortDirection() == ECFDAManagerSortDirection::Ascending
		? EColumnSortMode::Ascending
		: EColumnSortMode::Descending;
}

void SCFDAManagementTab::HandleTypeColumnSort(
	EColumnSortPriority::Type SortPriority,
	const FName& ColumnId,
	EColumnSortMode::Type NewSortMode)
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	ECFDAManagerTypeSortKey SortKey = ECFDAManagerTypeSortKey::DisplayName;
	if (ColumnId == CFDAManagementTabPrivate::TypeDomainColumnName)
	{
		SortKey = ECFDAManagerTypeSortKey::Domain;
	}
	else if (ColumnId == CFDAManagementTabPrivate::TypeAssetCountColumnName)
	{
		SortKey = ECFDAManagerTypeSortKey::AssetCount;
	}
	else if (ColumnId == CFDAManagementTabPrivate::TypeManagementStateColumnName)
	{
		SortKey = ECFDAManagerTypeSortKey::ManagementState;
	}
	else if (ColumnId != CFDAManagementTabPrivate::TypeColumnName)
	{
		return;
	}

	ViewModel->SetTypeSort(
		SortKey,
		NewSortMode == EColumnSortMode::Descending
			? ECFDAManagerSortDirection::Descending
			: ECFDAManagerSortDirection::Ascending);
	RefreshListItems();
}

void SCFDAManagementTab::HandleAssetColumnSort(
	EColumnSortPriority::Type SortPriority,
	const FName& ColumnId,
	EColumnSortMode::Type NewSortMode)
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	ECFDAManagerAssetSortKey SortKey = ECFDAManagerAssetSortKey::AssetName;
	if (ColumnId == CFDAManagementTabPrivate::AssetTypeColumnName)
	{
		SortKey = ECFDAManagerAssetSortKey::TypeDisplayName;
	}
	else if (ColumnId == CFDAManagementTabPrivate::AssetDomainColumnName)
	{
		SortKey = ECFDAManagerAssetSortKey::Domain;
	}
	else if (ColumnId == CFDAManagementTabPrivate::AssetScopeColumnName)
	{
		SortKey = ECFDAManagerAssetSortKey::Scope;
	}
	else if (ColumnId == CFDAManagementTabPrivate::AssetHealthColumnName)
	{
		SortKey = ECFDAManagerAssetSortKey::Health;
	}
	else if (ColumnId != CFDAManagementTabPrivate::AssetNameColumnName)
	{
		return;
	}

	ViewModel->SetAssetSort(
		SortKey,
		NewSortMode == EColumnSortMode::Descending
			? ECFDAManagerSortDirection::Descending
			: ECFDAManagerSortDirection::Ascending);
	RefreshListItems();
}

void SCFDAManagementTab::HandleSearchChanged(const FText& NewText)
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetSearchText(NewText.ToString());
		if (ViewModel->GetSelectedObjectPath().IsEmpty() && AssetListView.IsValid())
		{
			AssetListView->ClearSelection();
		}
		RefreshListItems();
	}
}

void SCFDAManagementTab::HandleDomainChanged(
	TSharedPtr<FCFDADomainFilterOption> Option,
	ESelectInfo::Type SelectInfo)
{
	SelectedDomainOption = Option;
	if (ViewModel.IsValid())
	{
		ViewModel->SetDomainFilter(
			Option.IsValid()
				? Option->Value
				: TOptional<ECFDADomain>());
		if (ViewModel->GetSelectedObjectPath().IsEmpty() && AssetListView.IsValid())
		{
			AssetListView->ClearSelection();
		}
		RefreshListItems();
	}
}

void SCFDAManagementTab::HandleManagementStateChanged(
	TSharedPtr<FCFDAManagementStateFilterOption> Option,
	ESelectInfo::Type SelectInfo)
{
	SelectedManagementStateOption = Option;
	if (ViewModel.IsValid())
	{
		ViewModel->SetManagementStateFilter(
			Option.IsValid()
				? Option->Value
				: TOptional<ECFDAManagerTypePresentationState>());
		RefreshListItems();
	}
}

void SCFDAManagementTab::HandleScopeChanged(
	TSharedPtr<FCFDAScopeFilterOption> Option,
	ESelectInfo::Type SelectInfo)
{
	SelectedScopeOption = Option;
	if (ViewModel.IsValid())
	{
		ViewModel->SetScopeFilter(
			Option.IsValid()
				? Option->Value
				: TOptional<ECFDAScope>());
		if (ViewModel->GetSelectedObjectPath().IsEmpty() && AssetListView.IsValid())
		{
			AssetListView->ClearSelection();
		}
		RefreshListItems();
	}
}

void SCFDAManagementTab::HandleHealthChanged(
	TSharedPtr<FCFDAHealthFilterOption> Option,
	ESelectInfo::Type SelectInfo)
{
	SelectedHealthOption = Option;
	if (ViewModel.IsValid())
	{
		ViewModel->SetHealthFilter(
			Option.IsValid()
				? Option->Value
				: TOptional<ECFDAHealthState>());
		if (ViewModel->GetSelectedObjectPath().IsEmpty() && AssetListView.IsValid())
		{
			AssetListView->ClearSelection();
		}
		RefreshListItems();
	}
}

void SCFDAManagementTab::HandleShowAllDiscoveredChanged(const ECheckBoxState NewState)
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	ViewModel->SetShowAllDiscovered(NewState == ECheckBoxState::Checked);
	RebuildFilterOptions();
	if (DomainComboBox.IsValid())
	{
		DomainComboBox->RefreshOptions();
		DomainComboBox->SetSelectedItem(SelectedDomainOption);
	}
	if (ManagementStateComboBox.IsValid())
	{
		ManagementStateComboBox->RefreshOptions();
		ManagementStateComboBox->SetSelectedItem(SelectedManagementStateOption);
	}
	RefreshListItems();
}

void SCFDAManagementTab::HandleTypeSelectionChanged(
	TSharedPtr<FCFDAManagerTypeRow> Item,
	ESelectInfo::Type SelectInfo)
{
	if (!Item.IsValid() || !ViewModel.IsValid())
	{
		return;
	}

	SelectedTypeClassPath = Item->ClassPath;
	ViewModel->ClearSelection();
	if (AssetListView.IsValid())
	{
		AssetListView->ClearSelection();
	}
}

void SCFDAManagementTab::HandleAssetSelectionChanged(
	TSharedPtr<FCFDAManagerAssetRow> Item,
	ESelectInfo::Type SelectInfo)
{
	if (!Item.IsValid() || !ViewModel.IsValid())
	{
		return;
	}

	SelectedTypeClassPath.Reset();
	ViewModel->SelectAsset(Item->ObjectPath);
	if (TypeListView.IsValid())
	{
		TypeListView->ClearSelection();
	}
}

FReply SCFDAManagementTab::HandleTypeViewClicked()
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetViewMode(ECFDAManagerViewMode::TypeView);
	}
	if (AssetListView.IsValid())
	{
		AssetListView->ClearSelection();
	}
	if (ViewSwitcher.IsValid())
	{
		ViewSwitcher->SetActiveWidgetIndex(0);
	}
	return FReply::Handled();
}

FReply SCFDAManagementTab::HandleAssetViewClicked()
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetViewMode(ECFDAManagerViewMode::AssetView);
	}
	SelectedTypeClassPath.Reset();
	if (TypeListView.IsValid())
	{
		TypeListView->ClearSelection();
	}
	if (ViewSwitcher.IsValid())
	{
		ViewSwitcher->SetActiveWidgetIndex(1);
	}
	return FReply::Handled();
}

FReply SCFDAManagementTab::HandleRefreshClicked()
{
	if (!ViewModel.IsValid())
	{
		LastStatusMessage = TEXT("데이터 관리 화면을 준비하지 못했습니다.");
		return FReply::Handled();
	}

	FString Error;
	if (ViewModel->Refresh(Error))
	{
		LastStatusMessage = ViewModel->GetNeedsRevalidationCount() > 0
			? FString::Printf(
				TEXT("데이터 목록 새로고침 완료. 이전에 검사한 에셋 %d개는 재검사가 필요한 상태로 표시하며, 사용처 조회 결과는 초기화했습니다."),
				ViewModel->GetNeedsRevalidationCount())
			: TEXT("데이터 목록 새로고침 완료. 사용처 조회 결과를 초기화했습니다.");
	}
	else
	{
		LastStatusMessage = Error;
	}

	RebuildFilterOptions();
	if (DomainComboBox.IsValid())
	{
		DomainComboBox->RefreshOptions();
		DomainComboBox->SetSelectedItem(SelectedDomainOption);
	}
	if (ScopeComboBox.IsValid())
	{
		ScopeComboBox->RefreshOptions();
		ScopeComboBox->SetSelectedItem(SelectedScopeOption);
	}
	if (HealthComboBox.IsValid())
	{
		HealthComboBox->RefreshOptions();
		HealthComboBox->SetSelectedItem(SelectedHealthOption);
	}
	RefreshAfterStateChange();
	return FReply::Handled();
}

FReply SCFDAManagementTab::HandleValidateClicked()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FString Error;
	if (ViewModel->ValidateSelectedAsset(Error))
	{
		LastStatusMessage = TEXT("선택한 에셋 검사 완료. 상태 / 고유 ID / 중복 여부를 갱신했습니다.");
	}
	else
	{
		LastStatusMessage = Error;
	}
	RefreshAfterStateChange();
	return FReply::Handled();
}

FReply SCFDAManagementTab::HandleReferenceClicked()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FString Error;
	if (ViewModel->QuerySelectedReferences(Error))
	{
		LastStatusMessage = TEXT("선택한 에셋의 참조 대상 / 사용처 조회를 완료했습니다.");
	}
	else
	{
		LastStatusMessage = Error;
	}
	return FReply::Handled();
}

FReply SCFDAManagementTab::HandleOpenAssetClicked()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FString Error;
	if (ViewModel->OpenSelectedAsset(Error))
	{
		LastStatusMessage = TEXT("선택한 데이터 에셋을 에셋 편집기로 열었습니다.");
	}
	else
	{
		LastStatusMessage = Error;
	}
	return FReply::Handled();
}

FReply SCFDAManagementTab::HandleSyncBrowserClicked()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FString Error;
	if (ViewModel->SyncSelectedAssetToContentBrowser(Error))
	{
		LastStatusMessage = TEXT("콘텐츠 브라우저에서 선택한 데이터 에셋의 위치를 표시했습니다.");
	}
	else
	{
		LastStatusMessage = Error;
	}
	return FReply::Handled();
}
