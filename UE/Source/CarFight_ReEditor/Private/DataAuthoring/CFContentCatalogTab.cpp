// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCatalogTab.cpp
// Version: v1.0.1
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-05 CarFight Content Manager Native Slate 탭 구현입니다.
// Changelog:
// - v1.0.1: search/type filter가 current selection을 숨길 때 stale detail/compare가 남지 않도록 visible-selection guard를 공통화.
// - v1.0.0: 상태 → Family/Variant → Catalog → 선택/비교/Resource/Diff 정보 계층,
//   기존 전문 authoring tab 진입과 explicit resource asset edit 진입을 최초 구현.
// Migration:
// - Content Manager는 read-only hub입니다. Product Apply/Save/Workbook persistent write를 수행하지 않습니다.
// - Resource asset editor는 USER가 명시적으로 버튼을 누른 경우에만 엽니다.

#include "DataAuthoring/CFContentCatalogTab.h"

#include "Editor.h"
#include "Framework/Docking/TabManager.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace CFContentCatalogTabPrivate
{
	// 섹션 헤더용 공통 TextBlock을 만듭니다.
	TSharedRef<SWidget> MakeSectionTitle(const FString& Title)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Title))
			.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 12));
	}

	// USER-facing type label을 반환합니다.
	FString GetTypeLabel(const ECFContentCatalogType CatalogType)
	{
		switch (CatalogType)
		{
		case ECFContentCatalogType::Weapon:
			return TEXT("무장");
		case ECFContentCatalogType::Vehicle:
			return TEXT("차량");
		case ECFContentCatalogType::Other:
			return TEXT("기타");
		case ECFContentCatalogType::All:
		default:
			return TEXT("전체");
		}
	}

	// 두 ContentKey의 exact logical equality를 비교합니다.
	bool AreKeysEqual(
		const FCFContentKey& Left,
		const FCFContentKey& Right)
	{
		return Left == Right;
	}
}

// Read-only Catalog / Compare UI를 구성하고 persisted Weapon/Vehicle catalog를 최초 로드합니다.
void SCFContentCatalogTab::Construct(const FArguments& InArgs)
{
	(void)InArgs;

	// USER-facing read-only ViewModel입니다.
	ViewModel = MakeShared<FCFContentCatalogVM>();

	// Initial persisted catalog refresh diagnostic입니다.
	FString RefreshError;
	if (ViewModel->RefreshLiveCatalog(RefreshError))
	{
		LastRefreshMessage = TEXT("현재 저장된 차량/무장 콘텐츠를 읽었습니다.");
	}
	else
	{
		LastRefreshMessage = RefreshError.IsEmpty()
			? TEXT("표시할 차량/무장 콘텐츠를 찾지 못했습니다.")
			: RefreshError;
	}

	RebuildCatalogItems();
	RebuildTreeItems();
	RebuildCompareItems();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(10.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("CarFight 콘텐츠 관리")))
				.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("전체 목록 → Family / Variant → 선택 → 비교 → Resource / Diff 순서로 확인합니다. 기술 ID와 ObjectPath는 기본 화면에 표시하지 않습니다.")))
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SBorder)
				.Padding(8.0f)
				[
					SAssignNew(StatusBox, SVerticalBox)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SSearchBox)
						.HintText(FText::FromString(
							TEXT("이름, Family, Variant, 값 또는 출처 검색")))
						.OnTextChanged(this, &SCFContentCatalogTab::HandleSearchChanged)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
						.Text(this, &SCFContentCatalogTab::GetFilterLabel,
							ECFContentCatalogType::All,
							FString(TEXT("전체")))
						.OnClicked(this, &SCFContentCatalogTab::HandleAllFilterClicked)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
						.Text(this, &SCFContentCatalogTab::GetFilterLabel,
							ECFContentCatalogType::Vehicle,
							FString(TEXT("차량")))
						.OnClicked(this, &SCFContentCatalogTab::HandleVehicleFilterClicked)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
						.Text(this, &SCFContentCatalogTab::GetFilterLabel,
							ECFContentCatalogType::Weapon,
							FString(TEXT("무장")))
						.OnClicked(this, &SCFContentCatalogTab::HandleWeaponFilterClicked)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("새로고침")))
						.ToolTipText(FText::FromString(
							TEXT("저장된 Product를 다시 읽습니다. 저장하거나 수정하지 않습니다.")))
						.OnClicked(this, &SCFContentCatalogTab::HandleRefreshClicked)
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SSplitter)

				+ SSplitter::Slot()
				.Value(0.22f)
				[
					SNew(SBorder)
					.Padding(6.0f)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 6.0f)
						[
							CFContentCatalogTabPrivate::MakeSectionTitle(
								TEXT("Family / Variant"))
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SAssignNew(
								FamilyTreeView,
								STreeView<TSharedPtr<FCFContentCatalogTreeNode>>)
								.TreeItemsSource(&TreeRootItems)
								.OnGenerateRow(
									this,
									&SCFContentCatalogTab::GenerateTreeRow)
								.OnGetChildren(
									this,
									&SCFContentCatalogTab::GetTreeChildren)
								.OnSelectionChanged(
									this,
									&SCFContentCatalogTab::HandleTreeSelectionChanged)
						]
					]
				]

				+ SSplitter::Slot()
				.Value(0.30f)
				[
					SNew(SBorder)
					.Padding(6.0f)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 6.0f)
						[
							CFContentCatalogTabPrivate::MakeSectionTitle(
								TEXT("콘텐츠 목록"))
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SAssignNew(
								CatalogListView,
								SListView<TSharedPtr<FCFContentCatalogEntry>>)
								.ListItemsSource(&CatalogItems)
								.SelectionMode(ESelectionMode::Single)
								.OnGenerateRow(
									this,
									&SCFContentCatalogTab::GenerateCatalogRow)
								.OnSelectionChanged(
									this,
									&SCFContentCatalogTab::HandleCatalogSelectionChanged)
						]
					]
				]

				+ SSplitter::Slot()
				.Value(0.48f)
				[
					SNew(SBorder)
					.Padding(8.0f)
					[
						SNew(SScrollBox)

						+ SScrollBox::Slot()
						[
							SAssignNew(DetailBox, SVerticalBox)
						]
					]
				]
			]
		]
	];

	RebuildStatusPanel();
	RebuildDetailPanel();

	if (FamilyTreeView.IsValid())
	{
		for (const TSharedPtr<FCFContentCatalogTreeNode>& RootNode : TreeRootItems)
		{
			FamilyTreeView->SetItemExpansion(RootNode, true);
		}
	}
}

// Persisted Product read-only catalog를 다시 읽고 모든 presentation widget을 갱신합니다.
FReply SCFContentCatalogTab::HandleRefreshClicked()
{
	// Live catalog refresh diagnostic입니다.
	FString RefreshError;
	if (ViewModel.IsValid()
		&& ViewModel->RefreshLiveCatalog(RefreshError))
	{
		LastRefreshMessage =
			TEXT("현재 저장된 차량/무장 콘텐츠를 다시 읽었습니다.");
	}
	else
	{
		LastRefreshMessage = RefreshError.IsEmpty()
			? TEXT("표시할 차량/무장 콘텐츠를 찾지 못했습니다.")
			: RefreshError;
	}

	RebuildCatalogItems();
	RebuildTreeItems();
	RebuildCompareItems();
	RebuildStatusPanel();
	RebuildDetailPanel();
	return FReply::Handled();
}

// Search box 변경을 ViewModel filter에 반영합니다.
void SCFContentCatalogTab::HandleSearchChanged(
	const FText& SearchText)
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	ViewModel->SetSearchText(SearchText.ToString());
	RebuildCatalogItems();
	RebuildTreeItems();

	ClearSelectionIfFilteredOut();
	RebuildCompareItems();
	RebuildDetailPanel();
}

// 전체 consumer filter를 선택합니다.
FReply SCFContentCatalogTab::HandleAllFilterClicked()
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetTypeFilter(ECFContentCatalogType::All);
		RebuildCatalogItems();
		RebuildTreeItems();
		ClearSelectionIfFilteredOut();
		RebuildCompareItems();
		RebuildDetailPanel();
	}
	return FReply::Handled();
}

// Vehicle consumer filter를 선택합니다.
FReply SCFContentCatalogTab::HandleVehicleFilterClicked()
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetTypeFilter(ECFContentCatalogType::Vehicle);
		RebuildCatalogItems();
		RebuildTreeItems();
		ClearSelectionIfFilteredOut();
		RebuildCompareItems();
		RebuildDetailPanel();
	}
	return FReply::Handled();
}

// Weapon consumer filter를 선택합니다.
FReply SCFContentCatalogTab::HandleWeaponFilterClicked()
{
	if (ViewModel.IsValid())
	{
		ViewModel->SetTypeFilter(ECFContentCatalogType::Weapon);
		RebuildCatalogItems();
		RebuildTreeItems();
		ClearSelectionIfFilteredOut();
		RebuildCompareItems();
		RebuildDetailPanel();
	}
	return FReply::Handled();
}

// Filtered catalog entry 한 줄을 만듭니다.
TSharedRef<ITableRow> SCFContentCatalogTab::GenerateCatalogRow(
	TSharedPtr<FCFContentCatalogEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// USER-facing primary row label입니다.
	const FString PrimaryText = Item.IsValid()
		? Item->DisplayName
		: TEXT("유효하지 않은 콘텐츠");

	// USER-facing secondary row label입니다.
	const FString SecondaryText = Item.IsValid()
		? FString::Printf(
			TEXT("%s · %s · %s"),
			*Item->SecondaryText,
			*Item->FamilyDisplayName,
			*Item->VariantDisplayName)
		: FString();

	return SNew(STableRow<TSharedPtr<FCFContentCatalogEntry>>, OwnerTable)
		.Padding(4.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
					.Text(FText::FromString(PrimaryText))
					.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
					.Text(FText::FromString(SecondaryText))
					.AutoWrapText(true)
			]
		];
}

// Catalog row 선택을 ViewModel과 Detail panel에 반영합니다.
void SCFContentCatalogTab::HandleCatalogSelectionChanged(
	TSharedPtr<FCFContentCatalogEntry> Item,
	ESelectInfo::Type SelectInfo)
{
	(void)SelectInfo;

	if (!ViewModel.IsValid())
	{
		return;
	}

	if (!Item.IsValid())
	{
		ViewModel->ClearSelection();
	}
	else
	{
		ViewModel->SelectEntry(Item->Key);
	}

	RebuildCompareItems();
	RebuildDetailPanel();
}

// Family/Variant tree node 한 줄을 만듭니다.
TSharedRef<ITableRow> SCFContentCatalogTab::GenerateTreeRow(
	TSharedPtr<FCFContentCatalogTreeNode> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// USER-facing tree row label입니다.
	const FString DisplayName = Item.IsValid()
		? Item->DisplayName
		: TEXT("유효하지 않은 항목");

	return SNew(STableRow<TSharedPtr<FCFContentCatalogTreeNode>>, OwnerTable)
		.Padding(3.0f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(DisplayName))
				.Font(
					Item.IsValid() && !Item->bEntryNode
						? FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10)
						: FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10))
		];
}

// Family/Variant tree node의 child 목록을 제공합니다.
void SCFContentCatalogTab::GetTreeChildren(
	TSharedPtr<FCFContentCatalogTreeNode> Item,
	TArray<TSharedPtr<FCFContentCatalogTreeNode>>& OutChildren) const
{
	OutChildren.Reset();
	if (Item.IsValid())
	{
		OutChildren = Item->Children;
	}
}

// Tree의 Variant entry를 선택하면 같은 Catalog row를 선택합니다.
void SCFContentCatalogTab::HandleTreeSelectionChanged(
	TSharedPtr<FCFContentCatalogTreeNode> Item,
	ESelectInfo::Type SelectInfo)
{
	(void)SelectInfo;

	if (!Item.IsValid()
		|| !Item->bEntryNode
		|| !ViewModel.IsValid())
	{
		return;
	}

	if (!ViewModel->SelectEntry(Item->EntryKey))
	{
		return;
	}

	if (CatalogListView.IsValid())
	{
		for (const TSharedPtr<FCFContentCatalogEntry>& CatalogItem : CatalogItems)
		{
			if (CatalogItem.IsValid()
				&& CFContentCatalogTabPrivate::AreKeysEqual(
					CatalogItem->Key,
					Item->EntryKey))
			{
				CatalogListView->SetSelection(CatalogItem);
				CatalogListView->RequestScrollIntoView(CatalogItem);
				break;
			}
		}
	}

	RebuildCompareItems();
	RebuildDetailPanel();
}

// Compare combo item의 USER-facing widget을 만듭니다.
TSharedRef<SWidget> SCFContentCatalogTab::GenerateCompareItemWidget(
	TSharedPtr<FCFContentCatalogEntry> Item) const
{
	// USER-facing compare candidate label입니다.
	const FString Label = Item.IsValid()
		? FString::Printf(
			TEXT("%s · %s"),
			*Item->DisplayName,
			*Item->VariantDisplayName)
		: TEXT("유효하지 않은 항목");

	return SNew(STextBlock)
		.Text(FText::FromString(Label));
}

// Compare target 선택을 ViewModel과 Detail panel에 반영합니다.
void SCFContentCatalogTab::HandleCompareSelectionChanged(
	TSharedPtr<FCFContentCatalogEntry> Item,
	ESelectInfo::Type SelectInfo)
{
	(void)SelectInfo;

	if (!ViewModel.IsValid())
	{
		return;
	}

	if (!Item.IsValid())
	{
		ViewModel->ClearCompareEntry();
	}
	else
	{
		ViewModel->SelectCompareEntry(Item->Key);
	}

	RebuildDetailPanel();
}

// Current Compare target의 USER-facing label을 반환합니다.
FText SCFContentCatalogTab::GetCompareSelectionText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::FromString(TEXT("비교 대상 선택"));
	}

	// Current compare row입니다.
	const FCFContentCatalogEntry* CompareEntry =
		ViewModel->GetCompareEntry();
	return CompareEntry != nullptr
		? FText::FromString(CompareEntry->DisplayName)
		: FText::FromString(TEXT("비교 대상 선택"));
}

// 선택된 content의 기존 전문 authoring tab을 엽니다.
FReply SCFContentCatalogTab::HandleOpenSpecializedEditor()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Existing specialized authoring tab identity입니다.
	const FName EditTabId =
		ViewModel->GetSelectedEditTabId();
	if (!EditTabId.IsNone())
	{
		FGlobalTabmanager::Get()->TryInvokeTab(EditTabId);
	}
	return FReply::Handled();
}

// 선택한 resource asset을 해당 Unreal Asset Editor로 엽니다.
FReply SCFContentCatalogTab::HandleOpenResourceAsset(
	FSoftObjectPath ResourceObjectPath)
{
	if (!ResourceObjectPath.IsValid()
		|| GEditor == nullptr)
	{
		return FReply::Handled();
	}

	// USER가 explicit edit 진입을 선택한 Resource object입니다.
	UObject* ResourceObject = ResourceObjectPath.TryLoad();
	if (ResourceObject == nullptr)
	{
		return FReply::Handled();
	}

	// Unreal의 표준 Asset Editor subsystem입니다.
	UAssetEditorSubsystem* AssetEditorSubsystem =
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem != nullptr)
	{
		AssetEditorSubsystem->OpenEditorForAsset(ResourceObject);
	}
	return FReply::Handled();
}

// ViewModel filtered entries를 Slate list item으로 재구성합니다.
void SCFContentCatalogTab::RebuildCatalogItems()
{
	CatalogItems.Reset();

	if (ViewModel.IsValid())
	{
		for (const FCFContentCatalogEntry& Entry :
			ViewModel->GetFilteredEntries())
		{
			CatalogItems.Add(
				MakeShared<FCFContentCatalogEntry>(Entry));
		}
	}

	if (CatalogListView.IsValid())
	{
		CatalogListView->RequestListRefresh();
	}
}

// ViewModel groups를 Family/Variant tree nodes로 재구성합니다.
void SCFContentCatalogTab::RebuildTreeItems()
{
	TreeRootItems.Reset();

	if (ViewModel.IsValid())
	{
		for (const FCFContentCatalogGroup& Group :
			ViewModel->GetGroups())
		{
			// USER-facing family root node입니다.
			TSharedPtr<FCFContentCatalogTreeNode> RootNode =
				MakeShared<FCFContentCatalogTreeNode>();
			RootNode->DisplayName = Group.DisplayName;
			RootNode->bEntryNode = false;

			for (const FCFContentKey& EntryKey : Group.EntryKeys)
			{
				// Group key와 일치하는 filtered catalog entry입니다.
				const FCFContentCatalogEntry* Entry =
					ViewModel->GetFilteredEntries().FindByPredicate(
						[&EntryKey](const FCFContentCatalogEntry& Candidate)
						{
							return CFContentCatalogTabPrivate::AreKeysEqual(
								Candidate.Key,
								EntryKey);
						});
				if (Entry == nullptr)
				{
					continue;
				}

				// USER-facing variant child node입니다.
				TSharedPtr<FCFContentCatalogTreeNode> ChildNode =
					MakeShared<FCFContentCatalogTreeNode>();
				ChildNode->DisplayName =
					FString::Printf(
						TEXT("%s · %s"),
						*Entry->DisplayName,
						*Entry->VariantDisplayName);
				ChildNode->EntryKey = Entry->Key;
				ChildNode->bEntryNode = true;
				RootNode->Children.Add(ChildNode);
			}

			TreeRootItems.Add(RootNode);
		}
	}

	if (FamilyTreeView.IsValid())
	{
		FamilyTreeView->RequestTreeRefresh();
		for (const TSharedPtr<FCFContentCatalogTreeNode>& RootNode : TreeRootItems)
		{
			FamilyTreeView->SetItemExpansion(RootNode, true);
		}
	}
}

// Current filter 결과에서 선택 row가 사라졌으면 selected/compare 상태를 함께 지웁니다.
void SCFContentCatalogTab::ClearSelectionIfFilteredOut()
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	// Current selected row입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		ViewModel->GetSelectedEntry();
	if (SelectedEntry == nullptr)
	{
		return;
	}

	// Current selected row가 filtered list에 계속 보이는지 여부입니다.
	const bool bStillVisible = CatalogItems.ContainsByPredicate(
		[SelectedEntry](const TSharedPtr<FCFContentCatalogEntry>& Candidate)
		{
			return Candidate.IsValid()
				&& CFContentCatalogTabPrivate::AreKeysEqual(
					Candidate->Key,
					SelectedEntry->Key);
		});
	if (!bStillVisible)
	{
		ViewModel->ClearSelection();
		if (CatalogListView.IsValid())
		{
			CatalogListView->ClearSelection();
		}
	}
}

// Current selected entry와 같은 consumer의 compare candidates를 재구성합니다.
void SCFContentCatalogTab::RebuildCompareItems()
{
	CompareItems.Reset();

	if (ViewModel.IsValid())
	{
		// Current selected row입니다.
		const FCFContentCatalogEntry* SelectedEntry =
			ViewModel->GetSelectedEntry();
		if (SelectedEntry != nullptr)
		{
			for (const FCFContentCatalogEntry& Candidate :
				ViewModel->GetFilteredEntries())
			{
				if (Candidate.CatalogType != SelectedEntry->CatalogType
					|| CFContentCatalogTabPrivate::AreKeysEqual(
						Candidate.Key,
						SelectedEntry->Key))
				{
					continue;
				}

				CompareItems.Add(
					MakeShared<FCFContentCatalogEntry>(Candidate));
			}
		}
	}

	if (CompareComboBox.IsValid())
	{
		CompareComboBox->RefreshOptions();
	}
}

// 선택/비교/Resource/Dependency/Diff 상세 패널을 다시 구성합니다.
void SCFContentCatalogTab::RebuildDetailPanel()
{
	if (!DetailBox.IsValid())
	{
		return;
	}

	DetailBox->ClearChildren();
	CompareComboBox.Reset();

	if (!ViewModel.IsValid())
	{
		DetailBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("Content Catalog ViewModel을 사용할 수 없습니다.")))
		];
		return;
	}

	// Current selected entry입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		ViewModel->GetSelectedEntry();
	if (SelectedEntry == nullptr)
	{
		DetailBox->AddSlot()
		.AutoHeight()
		[
			CFContentCatalogTabPrivate::MakeSectionTitle(
				TEXT("콘텐츠를 선택하세요"))
		];

		DetailBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 5.0f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("왼쪽 Family / Variant 또는 가운데 콘텐츠 목록에서 항목을 선택하면 값의 출처, 비교, 의존성, Resource / Socket, Diff Preview를 한 곳에서 확인할 수 있습니다.")))
				.AutoWrapText(true)
		];
		return;
	}

	DetailBox->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
			.Text(FText::FromString(SelectedEntry->DisplayName))
			.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 16))
	];

	DetailBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 2.0f, 0.0f, 6.0f)
	[
		SNew(STextBlock)
			.Text(FText::FromString(SelectedEntry->SecondaryText))
	];

	DetailBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
			.Text(FText::FromString(
				FString::Printf(
					TEXT("%s · %s"),
					*SelectedEntry->FamilyDisplayName,
					*SelectedEntry->VariantDisplayName)))
			.AutoWrapText(true)
	];

	if (!SelectedEntry->DesignIntent.IsEmpty())
	{
		DetailBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.Padding(6.0f)
			[
				SNew(STextBlock)
					.Text(FText::FromString(
						TEXT("설계 의도\n")
						+ SelectedEntry->DesignIntent))
					.AutoWrapText(true)
			]
		];
	}

	if (!SelectedEntry->EditTabId.IsNone())
	{
		DetailBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SButton)
				.Text(FText::FromString(
					TEXT("전문 제작 화면 열기")))
				.ToolTipText(FText::FromString(
					TEXT("현재 콘텐츠 종류에 맞는 기존 Vehicle Authoring 또는 Weapon Guide를 엽니다.")))
				.OnClicked(
					this,
					&SCFContentCatalogTab::HandleOpenSpecializedEditor)
		];
	}

	DetailBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 4.0f)
	[
		CFContentCatalogTabPrivate::MakeSectionTitle(
			TEXT("값과 출처"))
	];

	if (SelectedEntry->Values.IsEmpty())
	{
		DetailBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("표시할 대표 값이 없습니다.")))
		];
	}
	else
	{
		for (const FCFContentCatalogValue& Value :
			SelectedEntry->Values)
		{
			DetailBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.38f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Value.DisplayName))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.32f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Value.ValueText))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.30f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(
							TEXT("출처: ") + Value.SourceText))
				]
			];
		}
	}

	DetailBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 10.0f, 0.0f, 4.0f)
	[
		CFContentCatalogTabPrivate::MakeSectionTitle(
			TEXT("선택 비교"))
	];

	if (CompareItems.IsEmpty())
	{
		DetailBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("현재 필터 안에 비교할 같은 종류의 콘텐츠가 없습니다.")))
		];
	}
	else
	{
		DetailBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 5.0f)
		[
			SAssignNew(
				CompareComboBox,
				SComboBox<TSharedPtr<FCFContentCatalogEntry>>)
				.OptionsSource(&CompareItems)
				.OnGenerateWidget(
					this,
					&SCFContentCatalogTab::GenerateCompareItemWidget)
				.OnSelectionChanged(
					this,
					&SCFContentCatalogTab::HandleCompareSelectionChanged)
				[
					SNew(STextBlock)
						.Text(this, &SCFContentCatalogTab::GetCompareSelectionText)
				]
		];

		// Current selected/compare value rows입니다.
		const TArray<FCFContentCompareRow> CompareRows =
			ViewModel->BuildCompareRows();

		for (const FCFContentCompareRow& Row : CompareRows)
		{
			DetailBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(
							FString::Printf(
								TEXT("%s  |  %s ↔ %s  |  %s"),
								*Row.DisplayName,
								*Row.SelectedValue,
								*Row.CompareValue,
								*Row.RelativeDelta)))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(
							FString::Printf(
								TEXT("출처: %s ↔ %s"),
								*Row.SelectedSource,
								*Row.CompareSource)))
				]
			];
		}
	}

	DetailBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 10.0f, 0.0f, 4.0f)
	[
		CFContentCatalogTabPrivate::MakeSectionTitle(
			TEXT("의존성"))
	];

	DetailBox->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
			.Text(FText::FromString(
				ViewModel->BuildDependencyText()))
			.AutoWrapText(true)
	];

	DetailBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 10.0f, 0.0f, 4.0f)
	[
		CFContentCatalogTabPrivate::MakeSectionTitle(
			TEXT("Resource / Socket"))
	];

	if (SelectedEntry->Resources.IsEmpty())
	{
		DetailBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(
					ViewModel->BuildResourceText()))
				.AutoWrapText(true)
		];
	}
	else
	{
		for (const FCFContentCatalogResource& Resource :
			SelectedEntry->Resources)
		{
			// USER-facing resource description입니다.
			const FString ResourceDescription =
				Resource.CapabilityDisplayName.IsEmpty()
					? FString::Printf(
						TEXT("%s: %s · 출처 %s"),
						*Resource.RoleDisplayName,
						*Resource.ResourceDisplayName,
						*Resource.SourceText)
					: FString::Printf(
						TEXT("%s: %s · %s · 출처 %s"),
						*Resource.RoleDisplayName,
						*Resource.ResourceDisplayName,
						*Resource.CapabilityDisplayName,
						*Resource.SourceText);

			DetailBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(ResourceDescription))
						.AutoWrapText(true)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(6.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
						.IsEnabled(Resource.ResourceObjectPath.IsValid())
						.Text(FText::FromString(TEXT("자산 열기")))
						.ToolTipText(FText::FromString(
							TEXT("선택한 Mesh/Resource를 Unreal Asset Editor에서 엽니다. Socket 등 전문 편집은 해당 Asset Editor가 담당합니다.")))
						.OnClicked_Lambda(
							[this, ResourcePath = Resource.ResourceObjectPath]()
							{
								return HandleOpenResourceAsset(ResourcePath);
							})
				]
			];
		}
	}

	DetailBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 10.0f, 0.0f, 4.0f)
	[
		CFContentCatalogTabPrivate::MakeSectionTitle(
			TEXT("Diff Preview"))
	];

	DetailBox->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
			.Text(FText::FromString(
				ViewModel->BuildSelectedDiffText()))
			.AutoWrapText(true)
	];
}

// 상단 authority/workbook/validation/apply 상태 패널을 다시 구성합니다.
void SCFContentCatalogTab::RebuildStatusPanel()
{
	if (!StatusBox.IsValid()
		|| !ViewModel.IsValid())
	{
		return;
	}

	StatusBox->ClearChildren();

	// USER-facing Catalog authority/workbook status입니다.
	const FCFContentCatalogStatus& Status =
		ViewModel->GetStatus();

	StatusBox->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
			.Text(FText::FromString(
				FString::Printf(
					TEXT("%s  |  %s"),
					*Status.CatalogSourceText,
					*Status.WorkbookAuthorityText)))
			.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
			.AutoWrapText(true)
	];

	StatusBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 3.0f)
	[
		SNew(STextBlock)
			.Text(FText::FromString(
				FString::Printf(
					TEXT("%s  |  %s  |  %s"),
					*Status.WorkbookRevisionText,
					*Status.ValidationText,
					*Status.DiffPreviewText)))
			.AutoWrapText(true)
	];

	StatusBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 3.0f, 0.0f, 0.0f)
	[
		SNew(STextBlock)
			.Text(FText::FromString(
				Status.ApplyStateText
				+ TEXT("\n")
				+ LastRefreshMessage))
			.AutoWrapText(true)
	];
}

// Current filter 버튼 label에 선택 상태를 표시합니다.
FText SCFContentCatalogTab::GetFilterLabel(
	const ECFContentCatalogType FilterType,
	FString Label) const
{
	const bool bSelected =
		ViewModel.IsValid()
		&& ViewModel->GetTypeFilter() == FilterType;
	return FText::FromString(
		bSelected
			? TEXT("● ") + Label
			: Label);
}
