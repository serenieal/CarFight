// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCatalogTab.h
// Version: v1.0.1
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-05 CarFight Content Manager Native Slate 탭 계약입니다.
// Changelog:
// - v1.0.1: search/type filter 전환으로 선택 row가 숨겨질 때 stale selection을 지우는 USER workflow consistency guard를 추가.
// - v1.0.0: 상태 → Family/Variant → Catalog → 선택/비교/Resource/Diff의 USER information hierarchy를 최초 구현.
// Migration:
// - 기존 Vehicle Authoring / Weapon Guide는 전문 편집 화면으로 유지합니다.
// - Content Manager는 read-only navigation/compare hub이며 Product Apply/Workbook persistent write를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentCatalog.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SComboBox.h"

class SVerticalBox;

/** Family/Variant tree에서 사용하는 USER-facing node입니다. */
struct FCFContentCatalogTreeNode
{
	// USER에게 표시하는 group/entry 이름입니다.
	FString DisplayName;

	// Entry node이면 exact catalog key를 보유하고 group node이면 invalid key입니다.
	FCFContentKey EntryKey;

	// Group node 아래의 child nodes입니다.
	TArray<TSharedPtr<FCFContentCatalogTreeNode>> Children;

	// 이 node가 실제 catalog entry를 가리키는지 나타냅니다.
	bool bEntryNode = false;
};

/** CF-FQ-058 CCAS-P0-05 통합 Content Catalog / Compare Native Slate 탭입니다. */
class SCFContentCatalogTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCFContentCatalogTab)
	{
	}
	SLATE_END_ARGS()

	// Read-only Catalog / Compare UI를 구성하고 persisted Weapon/Vehicle catalog를 최초 로드합니다.
	void Construct(const FArguments& InArgs);

private:
	// Persisted Product read-only catalog를 다시 읽고 모든 presentation widget을 갱신합니다.
	FReply HandleRefreshClicked();

	// Search box 변경을 ViewModel filter에 반영합니다.
	void HandleSearchChanged(const FText& SearchText);

	// 전체 consumer filter를 선택합니다.
	FReply HandleAllFilterClicked();

	// Vehicle consumer filter를 선택합니다.
	FReply HandleVehicleFilterClicked();

	// Weapon consumer filter를 선택합니다.
	FReply HandleWeaponFilterClicked();

	// Filtered catalog entry 한 줄을 만듭니다.
	TSharedRef<ITableRow> GenerateCatalogRow(
		TSharedPtr<FCFContentCatalogEntry> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// Catalog row 선택을 ViewModel과 Detail panel에 반영합니다.
	void HandleCatalogSelectionChanged(
		TSharedPtr<FCFContentCatalogEntry> Item,
		ESelectInfo::Type SelectInfo);

	// Family/Variant tree node 한 줄을 만듭니다.
	TSharedRef<ITableRow> GenerateTreeRow(
		TSharedPtr<FCFContentCatalogTreeNode> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// Family/Variant tree node의 child 목록을 제공합니다.
	void GetTreeChildren(
		TSharedPtr<FCFContentCatalogTreeNode> Item,
		TArray<TSharedPtr<FCFContentCatalogTreeNode>>& OutChildren) const;

	// Tree의 Variant entry를 선택하면 같은 Catalog row를 선택합니다.
	void HandleTreeSelectionChanged(
		TSharedPtr<FCFContentCatalogTreeNode> Item,
		ESelectInfo::Type SelectInfo);

	// Compare combo item의 USER-facing widget을 만듭니다.
	TSharedRef<SWidget> GenerateCompareItemWidget(
		TSharedPtr<FCFContentCatalogEntry> Item) const;

	// Compare target 선택을 ViewModel과 Detail panel에 반영합니다.
	void HandleCompareSelectionChanged(
		TSharedPtr<FCFContentCatalogEntry> Item,
		ESelectInfo::Type SelectInfo);

	// Current Compare target의 USER-facing label을 반환합니다.
	FText GetCompareSelectionText() const;

	// 선택된 content의 기존 전문 authoring tab을 엽니다.
	FReply HandleOpenSpecializedEditor();

	// 선택한 resource asset을 해당 Unreal Asset Editor로 엽니다.
	FReply HandleOpenResourceAsset(FSoftObjectPath ResourceObjectPath);

	// ViewModel filtered entries를 Slate list item으로 재구성합니다.
	void RebuildCatalogItems();

	// ViewModel groups를 Family/Variant tree nodes로 재구성합니다.
	void RebuildTreeItems();

	// Current filter 결과에서 선택 row가 사라졌으면 selected/compare 상태를 함께 지웁니다.
	void ClearSelectionIfFilteredOut();

	// Current selected entry와 같은 consumer의 compare candidates를 재구성합니다.
	void RebuildCompareItems();

	// 선택/비교/Resource/Dependency/Diff 상세 패널을 다시 구성합니다.
	void RebuildDetailPanel();

	// 상단 authority/workbook/validation/apply 상태 패널을 다시 구성합니다.
	void RebuildStatusPanel();

	// Current filter 버튼 label에 선택 상태를 표시합니다.
	FText GetFilterLabel(
		ECFContentCatalogType FilterType,
		FString Label) const;

	// USER-facing read-only Catalog / Compare 상태를 소유합니다.
	TSharedPtr<FCFContentCatalogVM> ViewModel;

	// Filtered entries를 Slate ListView에서 소유하는 shared rows입니다.
	TArray<TSharedPtr<FCFContentCatalogEntry>> CatalogItems;

	// Family/Variant tree root nodes입니다.
	TArray<TSharedPtr<FCFContentCatalogTreeNode>> TreeRootItems;

	// Current selected entry와 비교 가능한 rows입니다.
	TArray<TSharedPtr<FCFContentCatalogEntry>> CompareItems;

	// Main catalog list widget입니다.
	TSharedPtr<SListView<TSharedPtr<FCFContentCatalogEntry>>> CatalogListView;

	// Family/Variant navigation tree widget입니다.
	TSharedPtr<STreeView<TSharedPtr<FCFContentCatalogTreeNode>>> FamilyTreeView;

	// Compare target 선택 widget입니다.
	TSharedPtr<SComboBox<TSharedPtr<FCFContentCatalogEntry>>> CompareComboBox;

	// 상단 authority/workbook/validation summary를 재구성할 container입니다.
	TSharedPtr<SVerticalBox> StatusBox;

	// 우측 selected/compare/resource/diff detail을 재구성할 container입니다.
	TSharedPtr<SVerticalBox> DetailBox;

	// Refresh 또는 live catalog read 실패 시 USER에게 보여줄 message입니다.
	FString LastRefreshMessage;
};
