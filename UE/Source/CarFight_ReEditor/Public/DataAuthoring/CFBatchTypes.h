// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchTypes.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation의 Editor-only value contract입니다.
// Scope: Dataset, column projection, baseline row, immutable-style manifest와 export artifact 모델을 제공합니다.
// Changelog:
// - v1.0.0: Section 26.6, 26.23~26.30 Batch projection/export 타입 최초 구현.
// Migration:
// - Spreadsheet/CSV를 새 Authoring Source Type 또는 SSOT로 추가하지 않습니다.
// - Import/3-way merge/B1·B2/B3 commit/apply 계약은 이 타입에서 열지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"

/** Section 26.6의 P0 Batch Dataset 네 종류입니다. */
enum class ECFBatchDatasetKind : uint8
{
	VehicleSummaryReport,
	ResolvedFieldReport,
	RecipeNumericEdit,
	ProfileNumericEdit
};

/** CSV cell의 canonical machine value type입니다. */
enum class ECFBatchValueType : uint8
{
	String,
	Integer,
	Number,
	Boolean,
	CanonicalText
};

/** Column의 P0 access 의미입니다. */
enum class ECFBatchColumnAccess : uint8
{
	ReadOnly,
	Editable
};

/** Column이 projection하는 Unreal Authoring owner입니다. */
enum class ECFBatchAuthoringOwner : uint8
{
	Metadata,
	ResolvedFieldRegistry,
	Recipe,
	Profile
};

/** Import 구현 전에도 Column이 어떤 typed mutation으로 연결될지 고정하는 metadata입니다. */
enum class ECFBatchMutationKind : uint8
{
	None,
	RecipeDrivingFeelAxis,
	RecipeMassExplicitValue,
	RecipeDurabilityExplicitValue,
	ProfileNumericLeaf
};

/** Section 26.30의 editable blank 의미입니다. */
enum class ECFBatchBlankPolicy : uint8
{
	NotApplicable,
	NoChange
};

/** FCFVehicleFieldRegistry 117 pattern을 read-only report metadata로 projection한 descriptor입니다. */
struct FCFBatchResolvedProjection
{
	// 기존 Field Registry의 canonical wildcard Stable Field Path입니다.
	FString StableFieldPattern;

	// Reflection leaf의 canonical export value type입니다.
	ECFBatchValueType ValueType = ECFBatchValueType::CanonicalText;

	// Reflection metadata에서 읽은 unit text입니다.
	FString Unit;

	// 기존 Field Registry의 Primary Profile Domain입니다.
	ECFVehicleProfileDomain PrimaryProfileDomain = ECFVehicleProfileDomain::None;

	// 기존 Field Registry Resolver rule입니다.
	ECFVehicleResolveRule ResolveRule = ECFVehicleResolveRule::ProjectCompatibilityDefault;

	// 기존 Field Registry Adoption Group입니다.
	ECFVehicleAdoptGroup AdoptionGroup = ECFVehicleAdoptGroup::LegacyTechnical;

	// Stable-ID identity leaf인지 여부입니다.
	bool bIdentityField = false;

	// hidden legacy serialized passthrough leaf인지 여부입니다.
	bool bLegacySerialized = false;
};

/** Section 26.23의 Batch Column Registry descriptor입니다. */
struct FCFBatchColumnDescriptor
{
	// Localized label/order와 독립적인 stable technical CSV identity입니다.
	FString ColumnId;

	// 이 Column이 속하는 exact Dataset Kind입니다.
	ECFBatchDatasetKind DatasetKind = ECFBatchDatasetKind::VehicleSummaryReport;

	// ProfileNumericEdit일 때 exact single-domain identity이며 그 외에는 None입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// 사용자에게 보여줄 label이며 import identity로 사용하지 않습니다.
	FText DisplayLabel;

	// Canonical cell parser/serializer가 사용하는 value type입니다.
	ECFBatchValueType ValueType = ECFBatchValueType::String;

	// 숫자 cell에 섞지 않고 metadata로 보존할 unit입니다.
	FString Unit;

	// ReadOnly 또는 Editable access입니다.
	ECFBatchColumnAccess Access = ECFBatchColumnAccess::ReadOnly;

	// 이 Column이 projection하는 Unreal SSOT owner입니다.
	ECFBatchAuthoringOwner AuthoringOwner = ECFBatchAuthoringOwner::Metadata;

	// 향후 Batch Import가 호출해야 할 typed mutation 종류입니다.
	ECFBatchMutationKind TypedMutationKind = ECFBatchMutationKind::None;

	// Reflection property 또는 semantic target의 stable technical path입니다.
	FString PropertyOrSemanticTarget;

	// Editable blank를 No Change로 해석하기 위한 policy입니다.
	ECFBatchBlankPolicy BlankPolicy = ECFBatchBlankPolicy::NotApplicable;

	// Clamp/효력 조건 등 machine-readable P0 validation 설명입니다.
	FString ValidationMetadata;

	// Recipe source mode처럼 row별 editability를 결정하는 optional condition property path입니다.
	FString EditConditionTarget;

	// EditConditionTarget이 이 enum/name canonical value일 때만 cell을 editable로 export합니다.
	FString RequiredEditConditionValue;

	// __cf_ importer-owned reserved metadata column인지 여부입니다.
	bool bReservedMetadata = false;
};

/** Manifest와 CSV baseline이 공유하는 canonical non-reserved cell입니다. */
struct FCFBatchBaselineCell
{
	// Stable technical ColumnId입니다.
	FString ColumnId;

	// Export 시점 Unreal truth를 canonical text로 보존한 baseline value입니다.
	FString CanonicalValue;

	// Export 시점 source mode까지 고려했을 때 Spreadsheet에서 edit 가능한지 여부입니다.
	bool bEditableAtExport = false;

	// Recipe Explicit/UseProfile/Profile Domain 같은 baseline ownership/source mode입니다.
	FString OwnershipSourceMode;
};

/** Export builder가 받는 한 row의 Unreal export-baseline value copy입니다. */
struct FCFBatchExportRow
{
	// Spreadsheet row order와 무관한 stable row identity입니다.
	FString RowId;

	// Vehicle dataset에서 사용할 Target Definition object path입니다.
	FString TargetPath;

	// Recipe dataset에서 사용할 Recipe object path입니다.
	FString RecipePath;

	// Profile dataset에서 사용할 Profile object path입니다.
	FString ProfilePath;

	// Profile dataset의 exact Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// RecipeFingerprint 또는 ProfileFingerprint입니다.
	FString ObjectFingerprint;

	// Recipe dataset에서 export 시점 Current Target Definition hash입니다.
	FString TargetDefinitionHash;

	// Dataset의 non-reserved baseline cells입니다.
	TArray<FCFBatchBaselineCell> BaselineCells;
};

/** .cfbatch.json에 저장하는 immutable-style baseline row입니다. */
struct FCFBatchManifestRow
{
	// Stable row identity입니다.
	FString RowId;

	// Export 시점 Target path입니다.
	FString TargetPath;

	// Export 시점 Recipe path입니다.
	FString RecipePath;

	// Export 시점 Profile path입니다.
	FString ProfilePath;

	// Profile dataset exact Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// Recipe/Profile object fingerprint입니다.
	FString ObjectFingerprint;

	// Recipe dataset export 시점 Target Definition hash입니다.
	FString TargetDefinitionHash;

	// Export 시점 editable baseline canonical values와 ownership/source mode입니다.
	TArray<FCFBatchBaselineCell> BaselineCells;
};

/** Section 26.26의 immutable export baseline evidence model입니다. */
struct FCFBatchManifest
{
	// 한 export snapshot을 식별하는 caller-provided GUID입니다.
	FGuid BatchExportId;

	// 한 manifest가 정확히 하나만 가지는 Dataset Kind입니다.
	ECFBatchDatasetKind DatasetKind = ECFBatchDatasetKind::VehicleSummaryReport;

	// ProfileNumericEdit일 때 한 file이 가지는 exact Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// Registry가 소유하는 stable schema technical identity입니다.
	FString SchemaId;

	// Registry schema revision입니다.
	int32 SchemaRevision = 0;

	// Export 시점 Frozen Resolver contract revision입니다.
	int32 ResolverContractRevision = 0;

	// CSV header의 canonical stable ColumnId 순서입니다.
	TArray<FString> CsvColumnIds;

	// Section 26.26이 요구하는 editable Column descriptor snapshot입니다.
	TArray<FCFBatchColumnDescriptor> EditableColumns;

	// RowId 기준 canonical 정렬된 baseline rows입니다.
	TArray<FCFBatchManifestRow> Rows;

	// Manifest semantic baseline 전체를 hash한 deterministic export-set fingerprint입니다.
	FString ExportSetHash;
};

/** Disk I/O 전 단계에서 테스트/서비스가 소비하는 canonical export 산출물입니다. */
struct FCFBatchExportArtifact
{
	// RFC-style escaping과 stable ColumnId header를 적용한 canonical CSV text입니다.
	FString CanonicalCsvText;

	// BOM 없는 UTF-8 canonical CSV bytes입니다.
	TArray<uint8> CsvUtf8Bytes;

	// Fixed field order로 직렬화한 .cfbatch.json text입니다.
	FString ManifestJsonText;

	// BOM 없는 UTF-8 manifest bytes입니다.
	TArray<uint8> ManifestUtf8Bytes;

	// JSON과 같은 의미를 가진 immutable-style manifest value copy입니다.
	FCFBatchManifest Manifest;
};
