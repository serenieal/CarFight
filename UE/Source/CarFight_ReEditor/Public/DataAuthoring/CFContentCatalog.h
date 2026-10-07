// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCatalog.h
// Version: v1.0.0
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-05 Catalog / Compare + USER Workflow의 read-only Presentation ViewModel 계약입니다.
// Changelog:
// - v1.0.0: Weapon/Vehicle 공용 catalog entry, Family/Variant grouping, selected compare, value source,
//   dependency/resource/diff/workbook status와 기존 authoring tab navigation 계약을 최초 구현.
// Migration:
// - P0-01~04 Core의 의미/권한은 변경하지 않습니다.
// - 이 계층은 Product Apply, Product writer, Workbook persistent write, P0-07 authority cutover를 수행하지 않습니다.
// - ObjectPath/Stable ID는 내부 navigation/identity에만 유지하고 USER-facing 기본 표시에서는 노출하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentCompiler.h"
#include "DataAuthoring/CFContentResource.h"

/** USER-facing Catalog의 현재 consumer 분류입니다. */
enum class ECFContentCatalogType : uint8
{
	All,
	Weapon,
	Vehicle,
	Other
};

/** Catalog의 값 하나를 absolute/relative compare와 source 표시가 가능하도록 투영합니다. */
struct FCFContentCatalogValue
{
	// USER-facing 비교 항목 이름입니다.
	FString DisplayName;

	// Presentation 내부에서 같은 의미의 값을 matching하기 위한 stable field key입니다.
	FString FieldKey;

	// USER에게 표시할 absolute value입니다.
	FString ValueText;

	// VALUE/NONE/INHERIT, Recipe/Profile/WeaponData 등 실제 값 출처를 사람이 읽게 표시합니다.
	FString SourceText;

	// Numeric relative compare가 가능한 값인지 나타냅니다.
	bool bNumeric = false;

	// Relative compare 계산용 canonical numeric value입니다.
	double NumericValue = 0.0;

	// Numeric value의 USER-facing 단위입니다.
	FString UnitText;
};

/** Resource/Socket 한 항목의 USER-facing projection입니다. */
struct FCFContentCatalogResource
{
	// Semantic Role을 사람이 읽는 이름으로 표시합니다.
	FString RoleDisplayName;

	// ResourceId/ObjectPath 대신 USER에게 표시할 Asset 이름입니다.
	FString ResourceDisplayName;

	// Socket/Bone/MaterialSlot이 있으면 사람이 읽게 표시합니다.
	FString CapabilityDisplayName;

	// Profile/Override 등 resolved binding source입니다.
	FString SourceText;

	// USER가 explicit edit/navigation을 선택했을 때만 사용할 내부 ObjectPath입니다.
	FSoftObjectPath ResourceObjectPath;
};

/** Content dependency/referencer 한 항목의 USER-facing projection입니다. */
struct FCFContentCatalogDependency
{
	// 의존 관계의 의미를 사람이 읽게 표시합니다.
	FString RelationDisplayName;

	// Stable ContentId/ObjectPath 대신 USER에게 표시할 대상 이름입니다.
	FString TargetDisplayName;
};

/** Catalog row 하나의 read-only Presentation 모델입니다. */
struct FCFContentCatalogEntry
{
	// Presentation 내부 selection/compare matching에 사용할 canonical logical key입니다.
	FCFContentKey Key;

	// Weapon/Vehicle 등 USER-facing consumer 분류입니다.
	ECFContentCatalogType CatalogType = ECFContentCatalogType::Other;

	// USER에게 가장 먼저 보여줄 사람 읽기용 이름입니다.
	FString DisplayName;

	// Type/authoring state 같은 보조 설명입니다.
	FString SecondaryText;

	// Canonical FamilyId가 있으면 사람이 읽게 표시하고 없으면 미지정 상태를 명시합니다.
	FString FamilyDisplayName;

	// Canonical Base/Variant 관계가 있으면 사람이 읽게 표시합니다.
	FString VariantDisplayName;

	// AI/USER가 보존한 Design Intent가 있으면 표시합니다.
	FString DesignIntent;

	// Absolute value + source rows입니다.
	TArray<FCFContentCatalogValue> Values;

	// P0-04 shared Resource Core에서 resolve된 resource/socket rows입니다.
	TArray<FCFContentCatalogResource> Resources;

	// Provider/typed backend에서 관측한 dependency rows입니다.
	TArray<FCFContentCatalogDependency> Dependencies;

	// Compiler Preview가 존재할 때 record-level Diff를 보유합니다.
	FCFContentCompileDiff Diff;

	// Diff projection이 실제 존재하는지 나타냅니다.
	bool bHasDiff = false;

	// 현재 row와 관련된 blocking/non-blocking validation issue 수입니다.
	int32 ValidationIssueCount = 0;

	// 기존 전문 authoring/editor로 이동할 Nomad Tab identity입니다.
	FName EditTabId = NAME_None;

	// USER가 기존 전문 asset editor를 명시적으로 열 때 사용할 내부 source path입니다.
	FSoftObjectPath SourceAssetPath;
};

/** Family/Variant tree의 한 그룹입니다. */
struct FCFContentCatalogGroup
{
	// 같은 consumer/family를 안정적으로 묶는 Presentation 내부 identity입니다.
	FString GroupKey;

	// USER-facing Family 또는 미지정 group 이름입니다.
	FString DisplayName;

	// Group 안에 포함되는 filtered catalog entry key 목록입니다.
	TArray<FCFContentKey> EntryKeys;
};

/** Selected vs Compare 값 한 줄의 absolute/relative projection입니다. */
struct FCFContentCompareRow
{
	// USER-facing 비교 항목 이름입니다.
	FString DisplayName;

	// Selected 쪽 absolute value입니다.
	FString SelectedValue;

	// Compare 쪽 absolute value입니다.
	FString CompareValue;

	// Numeric이면 Compare 기준 selected delta를 절대값/비율로 표시합니다.
	FString RelativeDelta;

	// Selected value의 resolved/authored source입니다.
	FString SelectedSource;

	// Compare value의 resolved/authored source입니다.
	FString CompareSource;
};

/** Workbook/Validation/Diff 상태를 USER가 기술 ID 없이 이해하도록 묶습니다. */
struct FCFContentCatalogStatus
{
	// 현재 Catalog가 읽고 있는 authority source 설명입니다.
	FString CatalogSourceText;

	// Workbook authority/cutover 상태입니다.
	FString WorkbookAuthorityText;

	// Workbook Preview가 존재하면 schema revision을 사람이 읽게 표시합니다.
	FString WorkbookRevisionText;

	// Workbook Preview validation 상태입니다.
	FString ValidationText;

	// Compile Diff Preview 요약입니다.
	FString DiffPreviewText;

	// P0-07 전에는 Apply가 닫혀 있음을 명시하는 안전 상태입니다.
	FString ApplyStateText;
};

/** P0-05 Weapon/Vehicle 공용 read-only Catalog / Compare ViewModel입니다. */
class CARFIGHT_REEDITOR_API FCFContentCatalogVM
{
public:
	// Persisted Vehicle Recipe + Weapon EquipmentPreset을 읽고 P0-04 shared typed bridge로 live Catalog를 구성합니다.
	bool RefreshLiveCatalog(FString& OutError);

	// P0-01 Canonical Workbook + P0-02 Compile Preview를 USER-facing Catalog/Diff/Family projection에 반영합니다.
	void ApplyCompilePreview(const FCFContentCompileResult& CompileResult);

	// Automation 또는 후속 provider adapter가 이미 만든 entries를 read-only Presentation state로 주입합니다.
	void SetEntriesForPresentation(
		const TArray<FCFContentCatalogEntry>& InEntries,
		const FCFContentCatalogStatus& InStatus);

	// Search text를 갱신하고 filtered rows/family groups를 다시 계산합니다.
	void SetSearchText(const FString& InSearchText);

	// All/Weapon/Vehicle type filter를 갱신하고 filtered rows/family groups를 다시 계산합니다.
	void SetTypeFilter(ECFContentCatalogType InTypeFilter);

	// Exact logical key를 current selected row로 설정합니다.
	bool SelectEntry(const FCFContentKey& Key);

	// Exact logical key를 current compare row로 설정합니다.
	bool SelectCompareEntry(const FCFContentKey& Key);

	// Current selected/compare row를 함께 지웁니다.
	void ClearSelection();

	// Compare row만 지웁니다.
	void ClearCompareEntry();

	// USER-facing filtered list입니다.
	const TArray<FCFContentCatalogEntry>& GetFilteredEntries() const { return FilteredEntries; }

	// USER-facing Family/Variant grouping입니다.
	const TArray<FCFContentCatalogGroup>& GetGroups() const { return Groups; }

	// Current selected row입니다. 선택이 없으면 nullptr입니다.
	const FCFContentCatalogEntry* GetSelectedEntry() const;

	// Current compare row입니다. 선택이 없으면 nullptr입니다.
	const FCFContentCatalogEntry* GetCompareEntry() const;

	// Current Selected vs Compare absolute/relative/source rows를 deterministic 순서로 만듭니다.
	TArray<FCFContentCompareRow> BuildCompareRows() const;

	// Current selected row의 dependency view를 USER-facing multi-line text로 만듭니다.
	FString BuildDependencyText() const;

	// Current selected row의 resource/socket view를 USER-facing multi-line text로 만듭니다.
	FString BuildResourceText() const;

	// Current selected row의 Diff Preview를 USER-facing multi-line text로 만듭니다.
	FString BuildSelectedDiffText() const;

	// Workbook/validation/apply 상태를 반환합니다.
	const FCFContentCatalogStatus& GetStatus() const { return Status; }

	// 현재 type filter를 반환합니다.
	ECFContentCatalogType GetTypeFilter() const { return TypeFilter; }

	// USER가 technical ObjectPath/Stable ID를 보지 않고 기존 전문 authoring UI로 이동할 TabId를 반환합니다.
	FName GetSelectedEditTabId() const;

private:
	// Current raw entries와 search/type filter에서 filtered list를 재구성합니다.
	void RebuildFilteredEntries();

	// Current filtered entries를 consumer + Family 기준 group으로 재구성합니다.
	void RebuildGroups();

	// Canonical workbook record를 generic Presentation entry로 투영합니다.
	FCFContentCatalogEntry BuildCanonicalEntry(
		const FCFContentRecord& Record,
		const FCFContentWorkbookModel& Workbook) const;

	// Exact key의 raw entry index를 반환하며 없으면 INDEX_NONE입니다.
	int32 FindEntryIndex(const FCFContentKey& Key) const;

	// Catalog 전체 read-only source entries입니다.
	TArray<FCFContentCatalogEntry> Entries;

	// Search/type filter 적용 뒤 USER list에 노출할 entries입니다.
	TArray<FCFContentCatalogEntry> FilteredEntries;

	// Filtered entries 기준 Family/Variant groups입니다.
	TArray<FCFContentCatalogGroup> Groups;

	// Current search text입니다.
	FString SearchText;

	// Current consumer type filter입니다.
	ECFContentCatalogType TypeFilter = ECFContentCatalogType::All;

	// Selected entry의 raw Entries index입니다.
	int32 SelectedEntryIndex = INDEX_NONE;

	// Compare entry의 raw Entries index입니다.
	int32 CompareEntryIndex = INDEX_NONE;

	// Workbook/validation/diff/apply USER status입니다.
	FCFContentCatalogStatus Status;
};
