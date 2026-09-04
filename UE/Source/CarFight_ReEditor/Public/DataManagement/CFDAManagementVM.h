// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAManagementVM.h
// Version: v1.6.0
// Date: 2026-09-04
// Description: CF-FQ-045 DAM-P0-04E 새로고침 후 재검사 필요 상태를 구분하는 Data Asset Manager ViewModel입니다.
// Changelog:
// - v1.6.0: Refresh가 current loaded 결과를 폐기할 때 과거 검사값 자체는 재사용하지 않고, 해당 ObjectPath에 session-local 재검사 필요 표시만 보존하도록 추가.
// - v1.4.0: Type/Asset column sort key/direction과 canonical tie-breaker 기반 deterministic sorting을 추가.
// - v1.3.0: management-universe Overview와 user purpose/usage row/search projection을 추가.
// - v1.2.0: 관리 대상 presentation state, 전체 발견 보기, Type/Asset view-specific filter 경계를 추가.
// - v1.1.0: PolicyUnavailable validation을 non-conclusive로 처리하고 filter/view 전환 시 hidden Asset selection을 자동 정리.
// - v1.0.0: InventoryGeneration owner, validation/reference cache, overview/search/filter/detail/navigation을 추가.
// Migration:
// - Runtime/Content/Product Asset mutation 없음. Refresh는 metadata-only이며 loaded/reference 작업은 explicit action에서만 수행합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAReferenceService.h"
#include "DataManagement/CFDATypeRegistry.h"

// Manager의 current list surface입니다.
enum class ECFDAManagerViewMode : uint8
{
	TypeView,
	AssetView
};

// Generic discovery 결과를 Manager 기본 화면에서 어떤 의미로 보여줄지 나타내는 presentation-only 상태입니다.
enum class ECFDAManagerTypePresentationState : uint8
{
	Managed,
	NeedsManagementRule,
	Framework,
	AuxiliaryDataAsset
};

// Type View에서 사용자가 선택할 수 있는 정렬 기준입니다.
enum class ECFDAManagerTypeSortKey : uint8
{
	DisplayName,
	Domain,
	AssetCount,
	ManagementState
};

// Asset View에서 사용자가 선택할 수 있는 정렬 기준입니다.
enum class ECFDAManagerAssetSortKey : uint8
{
	AssetName,
	TypeDisplayName,
	Domain,
	Scope,
	Health
};

// Manager table의 정렬 방향입니다.
enum class ECFDAManagerSortDirection : uint8
{
	Ascending,
	Descending
};

// Overview에서 표시할 current inventory 요약입니다.
struct FCFDAManagerOverview
{
	int32 TypeCount = 0;
	int32 AssetCount = 0;
	int32 RegisteredTypeCount = 0;
	int32 UnregisteredTypeCount = 0;
	int32 ZeroInstanceTypeCount = 0;
	int32 ErrorAssetCount = 0;
	int32 WarningAssetCount = 0;
	int32 NotValidatedAssetCount = 0;

	// P0-04C 기본 Overview가 사용하는 management-universe 수치입니다.
	int32 ManagedAssetCount = 0;
	int32 ProblemAssetCount = 0;
	int32 ManagedNotValidatedAssetCount = 0;
	int32 NeedsManagementRuleTypeCount = 0;
};

// Type View 한 행입니다.
struct FCFDAManagerTypeRow
{
	FString ClassPath;
	FString TechnicalClassName;
	FString DisplayName;
	FString UserPurposeDescription;
	FString UserUsageDescription;
	FString RoleDescription;
	ECFDADomain Domain = ECFDADomain::Unclassified;
	ECFDACoverageState CoverageState = ECFDACoverageState::Unregistered;
	ECFDAManagerTypePresentationState PresentationState = ECFDAManagerTypePresentationState::AuxiliaryDataAsset;
	ECFDATypeInstanceState TypeInstanceState = ECFDATypeInstanceState::NoAssetInstance;
	bool bCanonicalNative = false;
	bool bAbstract = false;
	int32 AssetCount = 0;
};

// Asset View 한 행입니다.
struct FCFDAManagerAssetRow
{
	FString AssetName;
	FString ObjectPath;
	FString PackagePath;
	FString ClassPath;
	FString TypeDisplayName;
	FString UserPurposeDescription;
	FString UserUsageDescription;
	FString RoleDescription;
	ECFDADomain Domain = ECFDADomain::Unclassified;
	ECFDAScope Scope = ECFDAScope::Unclassified;
	ECFDACoverageState CoverageState = ECFDACoverageState::Unregistered;
	ECFDAManagerTypePresentationState PresentationState = ECFDAManagerTypePresentationState::AuxiliaryDataAsset;
	ECFDAHealthState HealthState = ECFDAHealthState::NotValidated;
	ECFDAStableIdState StableIdState = ECFDAStableIdState::NotResolved;
	ECFDAEvaluationState EvaluationState = ECFDAEvaluationState::NotRequested;
	ECFDADuplicateState DuplicateState = ECFDADuplicateState::NotAnalyzed;

	// 이전 InventoryGeneration에서 검사했지만 현재 snapshot 기준 재검사가 필요한지 나타내는 presentation 상태입니다.
	bool bNeedsRevalidation = false;

	FString StableId;
};

// Manager UI가 설정하는 current filter state입니다.
struct FCFDAManagerFilterState
{
	FString SearchText;
	TOptional<ECFDADomain> Domain;

	// Type View 전용 관리 상태 filter입니다.
	TOptional<ECFDAManagerTypePresentationState> ManagementState;

	// Asset View 전용 filter입니다.
	TOptional<ECFDAScope> Scope;
	TOptional<ECFDAHealthState> Health;

	// false가 기본이며 Framework/AuxiliaryDataAsset을 list/search 대상에서 제외합니다.
	bool bShowAllDiscovered = false;
};

// Data Asset Manager 한 탭 lifetime 동안만 살아 있는 session-local owner입니다.
class CARFIGHT_REEDITOR_API FCFDAManagementVM
{
public:
	// current descriptor Registry를 준비하고 첫 metadata-only Refresh를 실행합니다.
	bool Initialize(FString& OutError);

	// 새 InventoryGeneration을 발급해 metadata-only inventory를 갱신하고 loaded/reference cache를 무효화합니다.
	bool Refresh(FString& OutError);

	// 검색 문자열을 변경합니다.
	void SetSearchText(const FString& SearchText);

	// Domain filter를 변경합니다. unset이면 All입니다.
	void SetDomainFilter(const TOptional<ECFDADomain>& Domain);

	// Type View의 관리 상태 filter를 변경합니다. unset이면 관리 대상 전체입니다.
	void SetManagementStateFilter(const TOptional<ECFDAManagerTypePresentationState>& ManagementState);

	// Asset View Scope filter를 변경합니다. unset이면 All입니다.
	void SetScopeFilter(const TOptional<ECFDAScope>& Scope);

	// Asset View Health filter를 변경합니다. unset이면 All입니다.
	void SetHealthFilter(const TOptional<ECFDAHealthState>& Health);

	// Framework/기타 DataAsset까지 generic discovery 전체를 표시할지 변경합니다.
	void SetShowAllDiscovered(bool bShowAllDiscovered);

	// current list surface를 변경합니다.
	void SetViewMode(ECFDAManagerViewMode ViewMode);

	// Type View의 current sort key/direction을 변경합니다.
	void SetTypeSort(ECFDAManagerTypeSortKey SortKey, ECFDAManagerSortDirection SortDirection);

	// Asset View의 current sort key/direction을 변경합니다.
	void SetAssetSort(ECFDAManagerAssetSortKey SortKey, ECFDAManagerSortDirection SortDirection);

	// current Type View sort key입니다.
	ECFDAManagerTypeSortKey GetTypeSortKey() const { return TypeSortKey; }

	// current Type View sort direction입니다.
	ECFDAManagerSortDirection GetTypeSortDirection() const { return TypeSortDirection; }

	// current Asset View sort key입니다.
	ECFDAManagerAssetSortKey GetAssetSortKey() const { return AssetSortKey; }

	// current Asset View sort direction입니다.
	ECFDAManagerSortDirection GetAssetSortDirection() const { return AssetSortDirection; }

	// Type rows를 selected column + canonical ClassPath tie-breaker로 정렬하는 pure helper입니다.
	static void SortTypeRows(
		TArray<FCFDAManagerTypeRow>& Rows,
		ECFDAManagerTypeSortKey SortKey,
		ECFDAManagerSortDirection SortDirection);

	// Asset rows를 selected column + canonical ObjectPath tie-breaker로 정렬하는 pure helper입니다.
	static void SortAssetRows(
		TArray<FCFDAManagerAssetRow>& Rows,
		ECFDAManagerAssetSortKey SortKey,
		ECFDAManagerSortDirection SortDirection);

	// current inventory와 generation-bound loaded cache를 반영한 Overview입니다.
	FCFDAManagerOverview BuildOverview() const;

	// current Search/Domain/ManagementState/전체 발견 보기 filter를 적용한 Type rows입니다.
	TArray<FCFDAManagerTypeRow> BuildFilteredTypeRows() const;

	// current Search/Domain/Scope/Health/전체 발견 보기 filter를 적용한 Asset rows입니다.
	TArray<FCFDAManagerAssetRow> BuildFilteredAssetRows() const;

	// current inventory에 실제 등장하는 Domain filter 후보를 deterministic 순서로 반환합니다.
	TArray<ECFDADomain> GetAvailableDomains() const;

	// core Coverage/native/abstract truth를 Manager presentation state로 변환하는 pure helper입니다.
	static ECFDAManagerTypePresentationState ResolvePresentationState(const FCFDATypeRecord& TypeRecord);

	// 기본 관리 universe에 포함되는 presentation state인지 반환합니다.
	static bool IsManagementUniverseState(ECFDAManagerTypePresentationState PresentationState);

	// Asset row 하나를 Detail current selection으로 선택합니다.
	bool SelectAsset(const FString& ObjectPath);

	// current selection을 해제합니다.
	void ClearSelection();

	// loaded validation 결과가 사용자에게 'Validate 완료'로 표시 가능한 conclusive evaluation인지 판정합니다.
	static bool IsConclusiveValidationResult(
		const FCFDALoadedAssetResult& Result,
		FString& OutMessage);

	// current selected Asset에 loaded identity/validation/duplicate를 explicit 실행하고 cache합니다.
	bool ValidateSelectedAsset(FString& OutError);

	// current selected Asset의 Asset Registry Reference/Referencer를 explicit query하고 cache합니다.
	bool QuerySelectedReferences(FString& OutError);

	// current selected Asset을 Unreal 표준 Asset Editor로 엽니다.
	bool OpenSelectedAsset(FString& OutError) const;

	// current selected Asset을 Content Browser에 동기화합니다.
	bool SyncSelectedAssetToContentBrowser(FString& OutError) const;

	// current selected Asset metadata row를 반환합니다.
	const FCFDAAssetRecord* GetSelectedAssetRecord() const;

	// current generation에 fresh한 loaded result가 있으면 반환합니다.
	const FCFDALoadedAssetResult* GetSelectedLoadedResult() const;

	// current generation에 fresh한 Reference result가 있으면 반환합니다.
	const FCFDAReferenceResult* GetSelectedReferenceResult() const;

	// current inventory를 반환합니다.
	const FCFDAInventoryResult& GetInventory() const { return Inventory; }

	// current Typed Registry를 반환합니다.
	const FCFDATypeRegistry& GetTypeRegistry() const { return TypeRegistry; }

	// current filter를 반환합니다.
	const FCFDAManagerFilterState& GetFilterState() const { return FilterState; }

	// current list surface를 반환합니다.
	ECFDAManagerViewMode GetViewMode() const { return ViewMode; }

	// current selected object path를 반환합니다.
	const FString& GetSelectedObjectPath() const { return SelectedObjectPath; }

	// current InventoryGeneration을 반환합니다.
	uint64 GetInventoryGeneration() const { return Inventory.InventoryGeneration; }

	// loaded validation cache count입니다. Automation/diagnostic용 read-only seam입니다.
	int32 GetLoadedCacheCount() const { return LoadedResultsByObjectPath.Num(); }

	// Reference cache count입니다. Automation/diagnostic용 read-only seam입니다.
	int32 GetReferenceCacheCount() const { return ReferenceResultsByObjectPath.Num(); }

	// 현재 선택한 에셋이 새 snapshot 기준 재검사가 필요한지 반환합니다.
	bool DoesSelectedAssetNeedRevalidation() const
	{
		return !SelectedObjectPath.IsEmpty()
			&& NeedsRevalidationObjectPaths.Contains(SelectedObjectPath);
	}

	// 재검사가 필요한 session-local 에셋 수입니다. Automation/diagnostic용 read-only seam입니다.
	int32 GetNeedsRevalidationCount() const { return NeedsRevalidationObjectPaths.Num(); }

private:
	// current descriptor Registry를 한 번만 deterministic하게 준비합니다.
	bool EnsureTypeRegistry(FString& OutError);

	// current inventory에서 ObjectPath row를 찾습니다.
	const FCFDAAssetRecord* FindAssetRecord(const FString& ObjectPath) const;

	// current generation loaded cache를 찾습니다.
	const FCFDALoadedAssetResult* FindLoadedResult(const FString& ObjectPath) const;

	// current generation reference cache를 찾습니다.
	const FCFDAReferenceResult* FindReferenceResult(const FString& ObjectPath) const;

	// current inventory에서 exact class path Type record를 찾습니다.
	const FCFDATypeRecord* FindTypeRecord(const FString& ClassPath) const;

	// Asset metadata와 fresh loaded cache를 합친 effective Asset row를 만듭니다.
	FCFDAManagerAssetRow BuildAssetRow(const FCFDAAssetRecord& AssetRecord) const;

	// Asset row가 current Search/Domain/Scope/Health filter에 모두 포함되는지 확인합니다.
	bool AssetMatchesCurrentFilters(const FCFDAManagerAssetRow& Row) const;

	// filter 변경 뒤 current selection이 더 이상 Asset View에 보이지 않으면 해제합니다.
	void ReconcileSelectionWithCurrentAssetFilters();

	// Type row가 current Type View filter에 포함되는지 확인합니다.
	bool TypeMatchesCurrentFilters(const FCFDAManagerTypeRow& Row) const;

	// 공통 Search text가 Type row에 일치하는지 확인합니다.
	bool MatchesTypeSearch(const FCFDAManagerTypeRow& Row) const;

	// 공통 Search text가 Asset row에 일치하는지 확인합니다.
	bool MatchesAssetSearch(const FCFDAManagerAssetRow& Row) const;

	// current descriptor Registry입니다.
	FCFDATypeRegistry TypeRegistry;

	// Registry가 current 27종 descriptor를 준비했는지 나타냅니다.
	bool bTypeRegistryReady = false;

	// current metadata-only snapshot입니다.
	FCFDAInventoryResult Inventory;

	// UI current filter state입니다.
	FCFDAManagerFilterState FilterState;

	// UI current surface입니다.
	ECFDAManagerViewMode ViewMode = ECFDAManagerViewMode::TypeView;

	// Type View current sort key입니다.
	ECFDAManagerTypeSortKey TypeSortKey = ECFDAManagerTypeSortKey::DisplayName;

	// Type View current sort direction입니다.
	ECFDAManagerSortDirection TypeSortDirection = ECFDAManagerSortDirection::Ascending;

	// Asset View current sort key입니다.
	ECFDAManagerAssetSortKey AssetSortKey = ECFDAManagerAssetSortKey::AssetName;

	// Asset View current sort direction입니다.
	ECFDAManagerSortDirection AssetSortDirection = ECFDAManagerSortDirection::Ascending;

	// Detail current selection입니다.
	FString SelectedObjectPath;

	// current generation loaded validation/detail cache입니다.
	TMap<FString, FCFDALoadedAssetResult> LoadedResultsByObjectPath;

	// 과거 generation에서 검사했지만 current snapshot에서는 결과를 다시 신뢰할 수 없는 ObjectPath 집합입니다.
	TSet<FString> NeedsRevalidationObjectPaths;

	// current generation on-demand reference cache입니다.
	TMap<FString, FCFDAReferenceResult> ReferenceResultsByObjectPath;
};
