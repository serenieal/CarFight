// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAManagementTab.h
// Version: v1.6.0
// Date: 2026-09-04
// Description: CF-FQ-045 DAM-P0-04E 재검사 필요 상태와 사용자 중심 유형 표현을 제공하는 surface입니다.
// Changelog:
// - v1.6.0: 새로고침 후 재검사/재확인 필요 문구를 만드는 helper를 추가하고, 과거 검사값 비재사용 정책을 UI에서 구분 가능하게 함.
// - v1.5.0: 기술 정보/참조 관계 한글 우선 표현을 Automation에서 확인할 read-only seam을 추가.
// - v1.4.0: Stable ID 사용자 상태, Referencer 삭제 안전 경고, list rebuild 후 Slate selection 동기화와 P0-04D Automation seam을 추가.
// - v1.3.0: Type/Asset sortable header state/callback을 추가.
// - v1.2.0: user-first Overview/Detail, technical expandable section과 Asset-only action visibility를 추가.
// - v1.1.0: Type/Asset actual column table, 관리 상태 filter, 전체 발견 보기와 view-specific filter surface를 추가.
// - v1.0.0: Overview, Type/Asset View, search/filter, Detail, Validate/Reference/Open/Sync action을 추가.
// Migration:
// - UI는 session-local FCFDAManagementVM만 호출하며 Product Asset mutation/save를 소유하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAManagementVM.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"

class SSearchBox;
class SWidgetSwitcher;

struct FCFDADomainFilterOption
{
	FString Label;
	TOptional<ECFDADomain> Value;
};

struct FCFDAManagementStateFilterOption
{
	FString Label;
	TOptional<ECFDAManagerTypePresentationState> Value;
};

struct FCFDAScopeFilterOption
{
	FString Label;
	TOptional<ECFDAScope> Value;
};

struct FCFDAHealthFilterOption
{
	FString Label;
	TOptional<ECFDAHealthState> Value;
};

class CARFIGHT_REEDITOR_API SCFDAManagementTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCFDAManagementTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Automation에서 current VM/UI foundation을 read-only 확인하는 seam입니다.
	TSharedPtr<FCFDAManagementVM> GetViewModelForTesting() const { return ViewModel; }

	// Health 상태와 재검사 필요 여부를 사용자에게 보여줄 문구로 변환합니다.
	static FString BuildHealthUserText(ECFDAHealthState HealthState, bool bNeedsRevalidation);

	// Stable ID state와 실제 값을 사용자에게 보여줄 문구로 변환합니다.
	static FString BuildStableIdUserText(ECFDAStableIdState StableIdState, const FString& StableId);

	// Stable ID state와 재확인 필요 여부를 사용자에게 보여줄 문구로 변환합니다.
	static FString BuildStableIdUserText(ECFDAStableIdState StableIdState, const FString& StableId, bool bNeedsRevalidation);

	// Referencer 결과가 삭제 안전 판정으로 오해되지 않도록 필요한 안내 문구를 만듭니다.
	static FString BuildReferencerSafetyText(ECFDAReferencerState ReferencerState);

	// Automation에서 현재 사용자용 상태 문구를 확인합니다.
	FString GetStatusMessageForTesting() const { return LastStatusMessage; }

	// Automation에서 현재 Detail 사용자 문구를 확인합니다.
	FText GetDetailTextForTesting() const { return BuildDetailText(); }

	// Automation에서 현재 기술 정보 사용자 문구를 확인합니다.
	FText GetTechnicalDetailTextForTesting() const { return BuildTechnicalDetailText(); }

	// Automation에서 현재 참조 관계 사용자 문구를 확인합니다.
	FText GetReferenceTextForTesting() const { return BuildReferenceText(); }

	// Automation에서 현재 row source를 다시 만들고 selection reconciliation을 실행합니다.
	void RefreshListItemsForTesting() { RefreshListItems(); }

	// Automation에서 실제 Refresh 버튼과 같은 경로를 실행합니다.
	void ExecuteRefreshForTesting() { HandleRefreshClicked(); }

	// Automation에서 실제 Type row selection callback 경로로 한 유형을 선택합니다.
	bool SelectTypeForTesting(const FString& ClassPath);

	// Automation에서 Type ListView가 실제로 highlight 중인 ClassPath를 확인합니다.
	FString GetSlateSelectedTypePathForTesting() const;

	// Automation에서 Asset ListView가 실제로 highlight 중인 ObjectPath를 확인합니다.
	FString GetSlateSelectedAssetPathForTesting() const;

private:
	void RebuildFilterOptions();
	void RefreshListItems();
	void ReconcileVisibleSelections();
	void RefreshAfterStateChange();

	FText BuildOverviewText() const;
	FText BuildDetailText() const;
	FText BuildTechnicalDetailText() const;
	FText BuildReferenceText() const;
	FText BuildStatusText() const;

	TSharedRef<SWidget> GenerateDomainOptionWidget(TSharedPtr<FCFDADomainFilterOption> Option) const;
	TSharedRef<SWidget> GenerateManagementStateOptionWidget(TSharedPtr<FCFDAManagementStateFilterOption> Option) const;
	TSharedRef<SWidget> GenerateScopeOptionWidget(TSharedPtr<FCFDAScopeFilterOption> Option) const;
	TSharedRef<SWidget> GenerateHealthOptionWidget(TSharedPtr<FCFDAHealthFilterOption> Option) const;

	TSharedRef<ITableRow> GenerateTypeRow(
		TSharedPtr<FCFDAManagerTypeRow> Item,
		const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateAssetRow(
		TSharedPtr<FCFDAManagerAssetRow> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// Type header 한 column의 current sort indicator를 반환합니다.
	EColumnSortMode::Type GetTypeColumnSortMode(FName ColumnId) const;

	// Asset header 한 column의 current sort indicator를 반환합니다.
	EColumnSortMode::Type GetAssetColumnSortMode(FName ColumnId) const;

	// Type header 클릭을 VM sort state로 변환합니다.
	void HandleTypeColumnSort(
		EColumnSortPriority::Type SortPriority,
		const FName& ColumnId,
		EColumnSortMode::Type NewSortMode);

	// Asset header 클릭을 VM sort state로 변환합니다.
	void HandleAssetColumnSort(
		EColumnSortPriority::Type SortPriority,
		const FName& ColumnId,
		EColumnSortMode::Type NewSortMode);

	void HandleSearchChanged(const FText& NewText);
	void HandleDomainChanged(TSharedPtr<FCFDADomainFilterOption> Option, ESelectInfo::Type SelectInfo);
	void HandleManagementStateChanged(TSharedPtr<FCFDAManagementStateFilterOption> Option, ESelectInfo::Type SelectInfo);
	void HandleScopeChanged(TSharedPtr<FCFDAScopeFilterOption> Option, ESelectInfo::Type SelectInfo);
	void HandleHealthChanged(TSharedPtr<FCFDAHealthFilterOption> Option, ESelectInfo::Type SelectInfo);
	void HandleShowAllDiscoveredChanged(ECheckBoxState NewState);
	void HandleTypeSelectionChanged(TSharedPtr<FCFDAManagerTypeRow> Item, ESelectInfo::Type SelectInfo);
	void HandleAssetSelectionChanged(TSharedPtr<FCFDAManagerAssetRow> Item, ESelectInfo::Type SelectInfo);

	FReply HandleTypeViewClicked();
	FReply HandleAssetViewClicked();
	FReply HandleRefreshClicked();
	FReply HandleValidateClicked();
	FReply HandleReferenceClicked();
	FReply HandleOpenAssetClicked();
	FReply HandleSyncBrowserClicked();

	TSharedPtr<FCFDAManagementVM> ViewModel;

	TArray<TSharedPtr<FCFDAManagerTypeRow>> TypeItems;
	TArray<TSharedPtr<FCFDAManagerAssetRow>> AssetItems;

	TArray<TSharedPtr<FCFDADomainFilterOption>> DomainOptions;
	TArray<TSharedPtr<FCFDAManagementStateFilterOption>> ManagementStateOptions;
	TArray<TSharedPtr<FCFDAScopeFilterOption>> ScopeOptions;
	TArray<TSharedPtr<FCFDAHealthFilterOption>> HealthOptions;

	TSharedPtr<FCFDADomainFilterOption> SelectedDomainOption;
	TSharedPtr<FCFDAManagementStateFilterOption> SelectedManagementStateOption;
	TSharedPtr<FCFDAScopeFilterOption> SelectedScopeOption;
	TSharedPtr<FCFDAHealthFilterOption> SelectedHealthOption;

	TSharedPtr<SListView<TSharedPtr<FCFDAManagerTypeRow>>> TypeListView;
	TSharedPtr<SListView<TSharedPtr<FCFDAManagerAssetRow>>> AssetListView;
	TSharedPtr<SComboBox<TSharedPtr<FCFDADomainFilterOption>>> DomainComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FCFDAManagementStateFilterOption>>> ManagementStateComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FCFDAScopeFilterOption>>> ScopeComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FCFDAHealthFilterOption>>> HealthComboBox;
	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<SWidgetSwitcher> ViewSwitcher;

	FString SelectedTypeClassPath;
	FString LastStatusMessage;
};
