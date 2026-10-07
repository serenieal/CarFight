// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentTypes.h
// Version: v1.2.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS Canonical Content Model과 P0-06 planning/review typed contract입니다.
// Changelog:
// - v1.2.0: P0-06 PlanningMetadata, explicit Active/Deprecated/Retired lifecycle, BaseCatalogSnapshotFingerprint와 SetPlanningMetadata operation을 추가.
// - v1.1.0: WorkbookSourceId, canonical unit/derived/FText mode, Family/Profile/DesignIntent metadata, typed ChangeSet contract를 frozen P0-01 기준으로 보강.
// - v1.0.0: ContentKey, stable sheet/column/row identity, field ownership, tri-state value, collection, dependency, migration model을 최초 구현.
// Migration:
// - Product DataAsset authority는 변경하지 않습니다.
// - 미분류 field ownership은 Unspecified로 유지하며 CCASManaged로 자동 승격하지 않습니다.
// - ExternalReadOnly record는 Catalog graph에 존재할 수 있지만 mutation 대상이 아닙니다.

#pragma once

#include "CoreMinimal.h"

/** Persistent field의 CCAS ownership exact1 상태입니다. */
enum class ECFContentFieldOwnership : uint8
{
	Unspecified,
	CCASManaged,
	ExternalManaged,
	LegacyPreserve,
	Derived,
	ReadOnly,
	Deprecated
};

/** Workbook cell이 표현하는 null/inheritance/value 상태입니다. */
enum class ECFContentValueState : uint8
{
	Inherit,
	None,
	Value
};

/** Catalog record의 CCAS 관리 상태입니다. */
enum class ECFContentManagementState : uint8
{
	Managed,
	ExternalReadOnly
};

/** Canonical content record의 authoring lifecycle 상태입니다. */
enum class ECFContentLifecycleState : uint8
{
	Active,
	Deprecated,
	Retired
};

/** Variant가 Family/Base 대비 어떤 방향으로 달라지는지 나타냅니다. */
enum class ECFContentRelativeDirection : uint8
{
	Decrease,
	Equal,
	Increase
};

/** Planning 단계의 authoring readiness 상태입니다. */
enum class ECFContentPlanningReadiness : uint8
{
	Planned,
	AuthoringReady,
	Conditional,
	Blocked
};

/** Planning technology dependency의 현재 충족 상태입니다. */
enum class ECFContentDependencyState : uint8
{
	Satisfied,
	Pending,
	Blocked
};

/** Canonical scalar/reference 값의 논리 타입입니다. */
enum class ECFContentValueType : uint8
{
	Boolean,
	SignedInteger,
	UnsignedInteger,
	Float,
	Double,
	Enum,
	NameId,
	String,
	Text,
	ContentReference,
	ResourceReference
};

/** FText semantic representation의 종류입니다. */
enum class ECFContentTextKind : uint8
{
	Literal,
	Localized
};

/** FText field descriptor가 소유하는 authoring identity policy입니다. */
enum class ECFContentTextMode : uint8
{
	NotText,
	PreserveIdentity,
	StableLocalized,
	LiteralInvariant
};

/** Child collection의 의미적 ordering 계약입니다. */
enum class ECFContentCollectionKind : uint8
{
	OrderedList,
	KeyedCollection,
	UnorderedSet
};

/** Reviewable typed Change Set에서 허용하는 P0 operation 종류입니다. */
enum class ECFContentChangeOperationType : uint8
{
	AddRecord,
	UpdateField,
	SetNone,
	SetInherit,
	UpsertChild,
	RemoveChildIntent,
	AssignProfile,
	SetPlanningMetadata,
	BindResource,
	RetireRecord,
	ReactivateRecord
};

/** Content provider가 소유하는 stable logical type identity입니다. */
struct FCFContentTypeId
{
	// Stable machine type identifier입니다.
	FString Value;

	// Type identifier가 canonical stable ID인지 확인합니다.
	bool IsValid() const;

	// 비교/정렬용 canonical string을 반환합니다.
	FString ToStableString() const;

	// Exact logical equality를 비교합니다.
	bool operator==(const FCFContentTypeId& Other) const
	{
		return Value.Equals(Other.Value, ESearchCase::CaseSensitive);
	}
};

/** ContentTypeId + 기존 domain ContentId로 구성되는 core logical identity입니다. */
struct FCFContentKey
{
	// Content가 속하는 stable logical type입니다.
	FCFContentTypeId ContentTypeId;

	// 기존 runtime identity와 mapping되는 stable content identifier입니다.
	FString ContentId;

	// ContentKey가 canonical machine identity인지 확인합니다.
	bool IsValid() const;

	// Deterministic sort/hash에 사용할 stable representation을 반환합니다.
	FString ToStableString() const;

	// Exact logical equality를 비교합니다.
	bool operator==(const FCFContentKey& Other) const
	{
		return ContentTypeId == Other.ContentTypeId
			&& ContentId.Equals(Other.ContentId, ESearchCase::CaseSensitive);
	}
};

/** FText를 display serialization이 아니라 semantic source로 표현합니다. */
struct FCFContentTextValue
{
	// Literal 또는 Localized semantic kind입니다.
	ECFContentTextKind Kind = ECFContentTextKind::Literal;

	// Localized text namespace이며 Literal에서는 비어 있어야 합니다.
	FString Namespace;

	// Localized text key이며 Literal에서는 비어 있어야 합니다.
	FString Key;

	// Literal source 또는 localized source string입니다.
	FString SourceString;
};

/** Canonical Workbook cell 하나의 typed value입니다. */
struct FCFContentValue
{
	// 이 값의 schema-declared logical type입니다.
	ECFContentValueType Type = ECFContentValueType::String;

	// INHERIT/NONE/VALUE 상태입니다.
	ECFContentValueState State = ECFContentValueState::Inherit;

	// Boolean VALUE storage입니다.
	bool bBooleanValue = false;

	// Signed integer VALUE storage입니다.
	int64 SignedIntegerValue = 0;

	// Unsigned integer VALUE storage입니다.
	uint64 UnsignedIntegerValue = 0;

	// Float/Double VALUE storage입니다. Float 타입은 float precision으로 canonicalize한 값을 저장합니다.
	double FloatingPointValue = 0.0;

	// Enum/NameId/String/ResourceReference VALUE storage입니다.
	FString StringValue;

	// FText semantic VALUE storage입니다.
	FCFContentTextValue TextValue;

	// ContentReference VALUE storage입니다.
	FCFContentKey ContentReference;
};

/** Persistent field 하나의 stable schema descriptor입니다. */
struct FCFContentFieldDescriptor
{
	// Physical column 위치/label과 독립적인 stable machine identity입니다.
	FString ColumnId;

	// 사용자에게 표시할 label이며 semantic identity/hash에서 제외됩니다.
	FString DisplayLabel;

	// 이 field의 canonical value type입니다.
	ECFContentValueType ValueType = ECFContentValueType::String;

	// 이 field의 persistent ownership exact1 값입니다. Unspecified는 invalid schema입니다.
	ECFContentFieldOwnership Ownership = ECFContentFieldOwnership::Unspecified;

	// ExternalManaged일 때 exact owner identity입니다.
	FString ExternalOwnerId;

	// Schema가 소유하는 canonical gameplay/unit identity입니다. Unitless field는 비워 둡니다.
	FString CanonicalUnitId;

	// Profile inheritance를 묶는 stable field-domain identity입니다. Profile 미사용 field는 비워 둡니다.
	FString FieldDomainId;

	// FText field의 identity policy exact1입니다. 비-Text field에서는 NotText여야 합니다.
	ECFContentTextMode TextMode = ECFContentTextMode::NotText;

	// NONE 상태를 허용하는지 여부입니다.
	bool bAllowNone = false;

	// VALUE가 반드시 존재해야 하는 provider-level requirement hint입니다.
	bool bRequired = false;
};

/** Workbook logical sheet의 stable schema descriptor입니다. */
struct FCFContentSheetDescriptor
{
	// Worksheet tab name과 독립적인 stable logical sheet identity입니다.
	FString SheetId;

	// 이 sheet schema의 positive revision입니다.
	int32 SchemaRevision = 0;

	// 이 sheet가 소유하는 ContentType identity입니다.
	FCFContentTypeId ContentTypeId;

	// Child sheet일 때 parent logical SheetId이며 primary sheet에서는 비어 있습니다.
	FString ParentSheetId;

	// Child sheet일 때 parent record 안의 stable collection identity입니다.
	FString CollectionId;

	// Child sheet collection의 semantic kind입니다.
	ECFContentCollectionKind CollectionKind = ECFContentCollectionKind::OrderedList;

	// Stable ColumnId 기반 field descriptor 집합입니다.
	TArray<FCFContentFieldDescriptor> Fields;

	// 이 descriptor가 child collection sheet인지 확인합니다.
	bool IsChildSheet() const
	{
		return !ParentSheetId.IsEmpty();
	}
};

/** Child collection item 하나의 canonical representation입니다. */
struct FCFContentCollectionItem
{
	// Row number와 독립적인 stable child identity입니다.
	FString ChildItemId;

	// KeyedCollection에서 사용하는 provider/domain semantic key입니다.
	FString DomainKey;

	// OrderedList에서 사용하는 explicit semantic order입니다.
	int32 Order = INDEX_NONE;

	// Stable ColumnId -> canonical typed value map입니다.
	TMap<FString, FCFContentValue> Fields;
};

/** Content record 내부의 canonical child collection입니다. */
struct FCFContentCollection
{
	// Parent record 안에서 stable한 collection identity입니다.
	FString CollectionId;

	// OrderedList/KeyedCollection/UnorderedSet semantic kind입니다.
	ECFContentCollectionKind Kind = ECFContentCollectionKind::OrderedList;

	// Canonical child item 집합입니다. Physical worksheet order는 authority가 아닙니다.
	TArray<FCFContentCollectionItem> Items;
};

/** Family/Base 대비 Variant의 한 design dimension 방향을 보존합니다. */
struct FCFContentRelativeIntent
{
	// Damage, FireRate 같은 stable design dimension identity입니다.
	FString DimensionId;

	// Base/Family 대비 감소/동일/증가 방향입니다.
	ECFContentRelativeDirection Direction = ECFContentRelativeDirection::Equal;
};

/** Planning을 차단하거나 조건부로 만드는 technology dependency 하나입니다. */
struct FCFContentTechnologyDependency
{
	// PFP.P0.03.UserTrajectory 같은 stable dependency identity입니다.
	FString DependencyId;

	// Dependency의 current planning 상태입니다.
	ECFContentDependencyState State = ECFContentDependencyState::Pending;

	// USER review에서 이유를 이해하기 위한 설명입니다.
	FString Reason;
};

/** Runtime payload와 분리된 Family/Variant planning metadata입니다. */
struct FCFContentPlanningMetadata
{
	// Family 안의 stable Variant identity입니다. Variant 미사용 record는 비워 둘 수 있습니다.
	FString VariantId;

	// Standard/LongRange/Salvo 같은 planning role identity입니다. Role 미사용 record는 비워 둘 수 있습니다.
	FString RoleId;

	// Base/Family 대비 design dimension 방향 집합입니다.
	TArray<FCFContentRelativeIntent> RelativeIntents;

	// Wave1A/Wave1B 같은 stable production wave identity입니다.
	FString ProductionWaveId;

	// Roster 포함 여부와 Product Apply readiness를 분리하는 planning 상태입니다.
	ECFContentPlanningReadiness Readiness = ECFContentPlanningReadiness::Planned;

	// 이 content의 authoring/Product acceptance를 조건화하는 technology dependency 집합입니다.
	TArray<FCFContentTechnologyDependency> TechnologyDependencies;

	// 여러 Variant가 공유하도록 계획된 Provider-owned DataRole IDs입니다.
	TArray<FString> SharedDataRoleIds;

	// 이 Variant가 별도로 소유하도록 계획된 Provider-owned DataRole IDs입니다.
	TArray<FString> VariantOwnedDataRoleIds;
};

/** Family/Profile/DesignIntent를 Product payload와 분리해 보존하는 authoring metadata입니다. */
struct FCFContentAuthoringMetadata
{
	// Variant concrete record가 소속되는 stable Family identity입니다. Family 미사용 record는 비워 둡니다.
	FString FamilyId;

	// 비교/디자인 기준으로만 사용하는 stable base ContentId입니다. implicit inheritance owner가 아닙니다.
	FString BaseContentId;

	// AI와 USER가 보존해야 하는 human-authored design intent입니다.
	FString DesignIntent;

	// Stable field-domain ID -> assigned Profile ID mapping입니다.
	TMap<FString, FString> AssignedProfileIdsByDomain;

	// Variant/Role/Wave/Readiness 등 P0-06 typed planning metadata입니다.
	FCFContentPlanningMetadata Planning;
};

/** Catalog의 canonical content record입니다. */
struct FCFContentRecord
{
	// ContentTypeId + existing domain ContentId logical identity입니다.
	FCFContentKey Key;

	// Top-level row에서 canonical FCFContentKey token과 exact 일치하는 stable machine RowId입니다.
	FString RowId;

	// CCAS Managed 또는 ExternalReadOnly 관리 상태입니다.
	ECFContentManagementState ManagementState = ECFContentManagementState::ExternalReadOnly;

	// Active/Deprecated/Retired authoring lifecycle exact1 상태입니다.
	ECFContentLifecycleState LifecycleState = ECFContentLifecycleState::Active;

	// Family/Profile/DesignIntent 등 canonical authoring metadata입니다.
	FCFContentAuthoringMetadata AuthoringMetadata;

	// Stable ColumnId -> canonical typed scalar/reference value map입니다.
	TMap<FString, FCFContentValue> Fields;

	// Stable CollectionId -> canonical child collection map입니다.
	TMap<FString, FCFContentCollection> Collections;
};

/** Workbook 전체의 canonical semantic model입니다. */
struct FCFContentWorkbookModel
{
	// Physical filename과 독립적인 stable Workbook source identity입니다.
	FString WorkbookSourceId;

	// Workbook machine schema의 positive revision입니다.
	int32 SchemaRevision = 0;

	// Physical tab position과 독립적인 logical sheet descriptor 집합입니다.
	TArray<FCFContentSheetDescriptor> Sheets;

	// Physical row order와 독립적인 canonical content record 집합입니다.
	TArray<FCFContentRecord> Records;
};

/** Reviewable typed Change Set operation 하나의 stable target/value contract입니다. */
struct FCFContentChangeOperation
{
	// P0에서 허용된 typed operation 종류입니다.
	ECFContentChangeOperationType OperationType = ECFContentChangeOperationType::UpdateField;

	// Worksheet tab name과 독립적인 stable logical SheetId입니다.
	FString SheetId;

	// Target top-level logical ContentKey입니다.
	FCFContentKey ContentKey;

	// Top-level operation에서 ContentKey token과 일치하는 stable RowId입니다.
	FString RowId;

	// Field operation의 stable ColumnId입니다. Field 비대상 operation에서는 비워 둘 수 있습니다.
	FString ColumnId;

	// Child operation의 stable ChildItemId입니다. Child 비대상 operation에서는 비워 둘 수 있습니다.
	FString ChildItemId;

	// KeyedCollection child 신규 생성 시 사용할 stable DomainKey입니다.
	FString ChildDomainKey;

	// OrderedList child 신규 생성 시 사용할 explicit semantic Order입니다.
	int32 ChildOrder = INDEX_NONE;

	// AssignProfile operation의 stable ProfileId입니다.
	FString ProfileId;

	// BindResource operation의 stable ResourceId입니다.
	FString ResourceId;

	// SetPlanningMetadata operation이 record에 설정할 Family identity입니다.
	FString PlanningFamilyId;

	// SetPlanningMetadata operation이 record에 설정할 optional BaseContent identity입니다.
	FString PlanningBaseContentId;

	// SetPlanningMetadata operation이 record에 설정할 authored design intent 설명입니다.
	FString PlanningDesignIntent;

	// SetPlanningMetadata operation이 record에 설정할 Variant/Role/Wave/readiness typed planning metadata입니다.
	FCFContentPlanningMetadata PlanningMetadata;

	// Update/Set/Add/Upsert operation이 전달하는 canonical typed value입니다.
	FCFContentValue Value;
};

/** AI/Tool이 xlsx binary 대신 생성하는 reviewable typed Change Set입니다. */
struct FCFContentChangeSet
{
	// Change Set schema identity입니다.
	FString SchemaId = TEXT("cfcontent-change/v1");

	// Idempotency/review를 위한 stable ChangeSet identity입니다.
	FString ChangeSetId;

	// Review가 결속된 authoritative Base WorkbookSemanticHash입니다.
	FString BaseWorkbookSemanticHash;

	// P0-06 AI planning/review가 결속된 immutable whole-Catalog snapshot fingerprint입니다.
	FString BaseCatalogSnapshotFingerprint;

	// Change Set이 대상으로 하는 Workbook machine schema revision입니다.
	int32 WorkbookSchemaVersion = 0;

	// USER/AI에게 변경 이유를 설명하는 review metadata입니다.
	FString Reason;

	// 이번 변경 묶음의 상위 design intent입니다.
	FString DesignIntent;

	// Stable target identity를 사용하는 ordered typed operation 집합입니다.
	TArray<FCFContentChangeOperation> Operations;

	// Deterministic preview 뒤 계산되는 expected post WorkbookSemanticHash입니다.
	FString ExpectedPostSemanticHash;
};

/** Content reference graph의 directed dependency edge입니다. */
struct FCFContentDependencyEdge
{
	// Referencing source ContentKey입니다.
	FCFContentKey From;

	// Referenced target ContentKey입니다.
	FCFContentKey To;

	// Reference를 만든 stable field/collection path입니다.
	FString FieldPath;
};

/** Schema/content validation에서 사용하는 stable diagnostic입니다. */
struct FCFContentValidationIssue
{
	// Stable machine diagnostic code입니다.
	FString Code;

	// 문제 위치를 나타내는 stable machine path입니다.
	FString Path;

	// 사용자-facing 한국어 진단 메시지입니다.
	FString Message;

	// true이면 validation 전체를 fail-closed 합니다.
	bool bBlocking = true;
};

/** Disposable old->new schema migration 규칙입니다. */
struct FCFContentMigrationRule
{
	// Migration 입력 workbook revision입니다.
	int32 FromRevision = 0;

	// Migration 출력 workbook revision입니다.
	int32 ToRevision = 0;

	// Stable old ColumnId -> new ColumnId rename map입니다.
	TMap<FString, FString> ColumnRenames;
};

/** Deterministic schema migration preview 결과입니다. */
struct FCFContentMigrationPreview
{
	// Migration 전 canonical semantic hash입니다.
	FString SourceSemanticHash;

	// Migration 후 canonical semantic hash입니다.
	FString TargetSemanticHash;

	// Product mutation 없이 memory 안에서 생성된 migrated model입니다.
	FCFContentWorkbookModel MigratedModel;

	// 사람이 검토할 deterministic summary입니다.
	TArray<FString> Summary;
};

/** Stable machine identifier가 허용되는 ASCII canonical grammar인지 확인합니다. */
bool CFIsStableContentId(const FString& Value);

/** Stable resource identifier가 허용되는 ASCII canonical grammar인지 확인합니다. */
bool CFIsStableResourceId(const FString& Value);
