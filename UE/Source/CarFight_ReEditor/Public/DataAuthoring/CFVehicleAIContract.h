// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAIContract.h
// Version: v1.3.0
// Date: 2026-08-18
// Description: DAUTH-P0-08I~P0-11 Common Authoring Service / AI Typed Contract public value contract입니다.
// Scope: Section 25 R0~R3 risk, approval, read/query와 normal Workspace Mesh-only candidate projection을 제공합니다.
// Changelog:
// - v1.3.0: P0-11 ListVehicles에 bounded Mesh-only Candidate projection metadata를 추가하되 AI raw writer/create operation은 추가하지 않음.
// - v1.2.0: P0-10 Reference Compare, Legacy Adoption, Wheel Measurement, managed-target lookup typed R0/R2 contract를 추가.
// - v1.1.0: P0-09 reviewed Initial Import proposal/commit을 UI와 공용 facade에서 사용할 typed R2 contract로 추가.
// - v1.0.0: AI/UI 공용 typed request/result foundation 최초 구현.
// Migration:
// - Raw SetField/UObject property patch/direct VehicleData write API를 제공하지 않습니다.
// - Shared Profile payload writer, Batch/CSV, Save/SaveAll, force/skip-validation 옵션을 제공하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleApplyService.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"
#include "CFVehicleAIContract.generated.h"

class UCFVehicleData;
class UCFVehicleRecipeData;

/** Section 25의 operation risk class입니다. */
UENUM()
enum class ECFAuthoringRiskClass : uint8
{
	R0_ReadOnly,
	R1_AuthoringRecordWrite,
	R2_OwnershipExceptionalWrite,
	R3_DefinitionApply
};

/** Caller identity는 권한과 분리된 diagnostic identity입니다. */
UENUM()
enum class ECFAuthoringCallerKind : uint8
{
	Unknown,
	SlateUI,
	AI,
	Automation
};

/** Exact reviewed action에 binding되는 transient approval class입니다. */
UENUM()
enum class ECFAuthoringApprovalClass : uint8
{
	None,
	AuthoringWrite,
	OwnershipWrite,
	DefinitionApply
};

/** 모든 facade operation이 공유하는 terminal status입니다. */
UENUM()
enum class ECFAuthoringOpStatus : uint8
{
	Succeeded,
	NoChange,
	Blocked,
	Conflict,
	FailedRolledBack,
	FailedUnknownState
};

/** Section 25 typed error taxonomy의 P0 foundation입니다. */
UENUM()
enum class ECFAuthoringErrorCode : uint8
{
	None,
	TargetNotFound,
	RecipeNotFound,
	ProfileNotFound,
	WrongAssetType,
	UnmanagedRequired,
	ManagedRequired,
	StateChanged,
	RecipeFingerprintMismatch,
	TargetHashMismatch,
	ResolverRevisionMismatch,
	ApprovalRequired,
	ApprovalScopeMismatch,
	OperationIdConflict,
	InvalidSemanticInput,
	MissingRequiredSource,
	FieldNotAuthorable,
	OverrideNotAllowed,
	StableIdConflict,
	DependencyConflict,
	MeasurementNotFound,
	MeasurementFingerprintStale,
	AdoptionBlocked,
	ExternalDriftUnresolved,
	PreviewOutOfDate,
	ValidationBlocked,
	ApplyPreconditionFailed,
	ApplyValidationFailed,
	ApplyRollbackFailed,
	UnsupportedOperation,
	InternalError
};

/** Reference Compare에서 current raw Definition과 resolved Authoring preview를 구분합니다. */
UENUM()
enum class ECFVehicleCompareValueMode : uint8
{
	CurrentDefinition,
	ResolvedPreview
};

/** Wheel measurement review에서 허용하는 explicit decision입니다. */
UENUM()
enum class ECFVehicleMeasureDecision : uint8
{
	AcceptMeasuredValue,
	UseCompatibilityDefault
};

/** P0-08I에서 허용하는 normal R1 Recipe semantic operation 종류입니다. */
UENUM()
enum class ECFVehicleSemanticOp : uint8
{
	BindVehicleProfile,
	SetVehicleArchetype,
	SetVehicleAssetIntent,
	SetDrivingFeel,
	SetMassIntent,
	SetDurabilityIntent,
	SetDefaultDataIntent,
	SetWheelVisualIntent,
	SetDriveStateMode,
	UpsertHardpointIntent,
	UpsertMountIntent
};

/** R1/R2/R3 request가 공유하는 read-before-write / approval context입니다. */
USTRUCT()
struct FCFAuthoringCallContext
{
	GENERATED_BODY()

	// 한 client mutation attempt를 식별하는 transient idempotency key입니다.
	UPROPERTY()
	FString ClientOperationId;

	// Slate/AI/Automation caller diagnostic identity입니다.
	UPROPERTY()
	ECFAuthoringCallerKind CallerKind = ECFAuthoringCallerKind::Unknown;

	// Exact operation risk에 맞아야 하는 transient approval class입니다.
	UPROPERTY()
	ECFAuthoringApprovalClass ApprovalClass = ECFAuthoringApprovalClass::None;

	// Preview가 만든 exact proposal/action hash입니다.
	UPROPERTY()
	FString ApprovalScopeHash;

	// Write 직전 다시 비교할 Recipe semantic fingerprint입니다.
	UPROPERTY()
	FString ExpectedRecipeFingerprint;

	// Target 관련 write에서 다시 비교할 Current Definition hash입니다.
	UPROPERTY()
	FString ExpectedTargetDefinitionHash;

	// Request가 기대하는 Frozen Resolver semantic contract revision입니다.
	UPROPERTY()
	int32 ExpectedResolverContractRevision = 0;
};

/** 모든 operation result가 명시하는 mutation footprint입니다. */
USTRUCT()
struct FCFAuthoringMutationFootprint
{
	GENERATED_BODY()

	// Recipe UDataAsset가 persistent mutation됐는지 여부입니다.
	UPROPERTY()
	bool bRecipeChanged = false;

	// Target UCFVehicleData가 persistent mutation됐는지 여부입니다.
	UPROPERTY()
	bool bTargetChanged = false;

	// Shared Profile payload가 persistent mutation됐는지 여부입니다. P0-08I에서는 항상 false입니다.
	UPROPERTY()
	bool bProfileChanged = false;

	// 새 persistent asset이 생성됐는지 여부입니다. P0-08I에서는 항상 false입니다.
	UPROPERTY()
	bool bCreatedAssets = false;

	// 이번 operation으로 관련 package가 Dirty가 되었는지 여부입니다.
	UPROPERTY()
	bool bPackageDirty = false;

	// Disk save가 실제 수행됐는지 여부입니다. P0 Authoring contract에서는 항상 false입니다.
	UPROPERTY()
	bool bSavePerformed = false;

	// Service가 동일 write를 automatic retry했는지 여부입니다. P0에서는 항상 false입니다.
	UPROPERTY()
	bool bAutomaticRetryPerformed = false;
};

/** Recipe/Resolver/Definition validation을 compact count로 요약합니다. */
USTRUCT()
struct FCFAuthoringValidationSummary
{
	GENERATED_BODY()

	// Info issue 수입니다.
	UPROPERTY()
	int32 InfoCount = 0;

	// Warning issue 수입니다.
	UPROPERTY()
	int32 WarningCount = 0;

	// Blocked issue 수입니다.
	UPROPERTY()
	int32 BlockedCount = 0;

	// Error issue 수입니다.
	UPROPERTY()
	int32 ErrorCount = 0;

	// 현재 validation layer가 Apply를 차단하는 issue를 포함하는지 여부입니다.
	UPROPERTY()
	bool bApplyBlocking = false;
};

/** AI/UI facade가 공통으로 반환하는 typed result envelope입니다. */
USTRUCT()
struct FCFAuthoringOpResult
{
	GENERATED_BODY()

	// 실제 수행한 typed operation 이름입니다.
	UPROPERTY()
	FName OperationName = NAME_None;

	// Operation의 Frozen risk class입니다.
	UPROPERTY()
	ECFAuthoringRiskClass RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;

	// Terminal operation status입니다.
	UPROPERTY()
	ECFAuthoringOpStatus Status = ECFAuthoringOpStatus::Blocked;

	// 문자열 parsing 없이 처리할 stable error code입니다.
	UPROPERTY()
	ECFAuthoringErrorCode ErrorCode = ECFAuthoringErrorCode::None;

	// 사람이 읽는 한국어 결과 메시지입니다.
	UPROPERTY()
	FString Message;

	// Caller가 전달한 ClientOperationId입니다.
	UPROPERTY()
	FString ClientOperationId;

	// 이번 operation이 바꾼 object 범위입니다.
	UPROPERTY()
	FCFAuthoringMutationFootprint Mutation;

	// Fresh request로 다시 시도할 수 있는 성격인지 여부입니다. Write blind retry 허용 의미가 아닙니다.
	UPROPERTY()
	bool bRetryAllowed = false;

	// Editor lifetime dedupe cache에서 terminal result를 재사용했는지 여부입니다.
	UPROPERTY()
	bool bResultReplayedFromDedupe = false;

	// 성공한 persistent authoring action diagnostic identity입니다.
	UPROPERTY()
	FString AuthoringActionId;

	// Operation 종료 시 current Recipe fingerprint입니다.
	UPROPERTY()
	FString CurrentRecipeFingerprint;

	// Operation 종료 시 current full Target Definition hash입니다.
	UPROPERTY()
	FString CurrentTargetDefinitionHash;

	// Operation 종료 시 current effective Source signature입니다.
	UPROPERTY()
	FString CurrentSourceSignature;

	// Operation 종료 시 current resolved Definition hash입니다.
	UPROPERTY()
	FString CurrentResolvedDefinitionHash;

	// Operation이 다룬 deterministic Diff hash입니다.
	UPROPERTY()
	FString CurrentDiffHash;

	// Operation이 사용한 Resolver semantic contract revision입니다.
	UPROPERTY()
	int32 ResolverContractRevision = 0;

	// Current validation compact summary입니다.
	UPROPERTY()
	FCFAuthoringValidationSummary ValidationSummary;

	// R3 actual Apply가 수행한 dependency-safe operation 수입니다.
	UPROPERTY()
	int32 AppliedFieldCount = 0;
};

/** Preview 뒤 exact approval을 binding할 typed proposal envelope입니다. */
USTRUCT()
struct FCFAuthoringProposal
{
	GENERATED_BODY()

	// Proposal이 승인하려는 exact operation 이름입니다.
	UPROPERTY()
	FName OperationName = NAME_None;

	// Proposal operation의 risk class입니다.
	UPROPERTY()
	ECFAuthoringRiskClass RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;

	// Commit에 필요한 exact approval class입니다.
	UPROPERTY()
	ECFAuthoringApprovalClass RequiredApprovalClass = ECFAuthoringApprovalClass::None;

	// Operation/Target/Recipe/prospective state를 canonical hash한 approval scope입니다.
	UPROPERTY()
	FString ProposalHash;

	// Preview baseline Recipe fingerprint입니다.
	UPROPERTY()
	FString ExpectedRecipeFingerprint;

	// Preview baseline full Target Definition hash입니다.
	UPROPERTY()
	FString ExpectedTargetDefinitionHash;

	// Prospective Recipe semantic fingerprint입니다.
	UPROPERTY()
	FString ProspectiveRecipeFingerprint;

	// Prospective effective Source signature입니다.
	UPROPERTY()
	FString ProspectiveSourceSignature;

	// Prospective resolved Definition hash입니다.
	UPROPERTY()
	FString ProspectiveResolvedDefinitionHash;

	// Prospective exact FieldDiff deterministic hash입니다.
	UPROPERTY()
	FString DiffHash;

	// Proposal이 사용한 Resolver semantic contract revision입니다.
	UPROPERTY()
	int32 ResolverContractRevision = 0;

	// Proposal commit 자체가 Target Definition을 수정하는지 여부입니다.
	UPROPERTY()
	bool bTargetMutation = false;

	// P0 Authoring operation이 disk save를 수행하는지 여부입니다. 항상 false입니다.
	UPROPERTY()
	bool bSavePerformed = false;
};

/** SetDrivingFeel의 omitted-axis 보존을 위한 typed partial patch입니다. */
USTRUCT()
struct FCFDrivingFeelPatch
{
	GENERATED_BODY()

	// AccelerationFeel을 exact desired value로 바꿀지 여부입니다.
	UPROPERTY()
	bool bSetAccelerationFeel = false;

	// bSetAccelerationFeel=true일 때의 0..1 desired value입니다.
	UPROPERTY()
	float AccelerationFeel = 0.5f;

	// SteeringAgility를 exact desired value로 바꿀지 여부입니다.
	UPROPERTY()
	bool bSetSteeringAgility = false;

	// bSetSteeringAgility=true일 때의 0..1 desired value입니다.
	UPROPERTY()
	float SteeringAgility = 0.5f;

	// GripFeel을 exact desired value로 바꿀지 여부입니다.
	UPROPERTY()
	bool bSetGripFeel = false;

	// bSetGripFeel=true일 때의 0..1 desired value입니다.
	UPROPERTY()
	float GripFeel = 0.5f;

	// SuspensionFirmness를 exact desired value로 바꿀지 여부입니다.
	UPROPERTY()
	bool bSetSuspensionFirmness = false;

	// bSetSuspensionFirmness=true일 때의 0..1 desired value입니다.
	UPROPERTY()
	float SuspensionFirmness = 0.5f;
};

/** Preview/Commit이 공유하는 discriminated typed semantic command입니다. Raw field path/value를 받지 않습니다. */
USTRUCT()
struct FCFVehicleSemanticChange
{
	GENERATED_BODY()

	// 이 command가 수행할 normal R1 semantic operation입니다.
	UPROPERTY()
	ECFVehicleSemanticOp Operation = ECFVehicleSemanticOp::SetDrivingFeel;

	// BindVehicleProfile에서만 사용하는 Profile Domain입니다.
	UPROPERTY()
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// BindVehicleProfile에서만 사용하는 existing Profile asset identity입니다.
	UPROPERTY()
	FSoftObjectPath ProfileAssetPath;

	// SetVehicleArchetype에서만 사용하는 exact desired archetype id입니다.
	UPROPERTY()
	FName VehicleArchetypeId = NAME_None;

	// SetVehicleAssetIntent에서만 사용하는 typed desired asset/socket intent입니다.
	UPROPERTY()
	FCFVehicleAssetIntent AssetIntent;

	// SetDrivingFeel에서만 사용하는 typed partial semantic patch입니다.
	UPROPERTY()
	FCFDrivingFeelPatch DrivingFeelPatch;

	// SetMassIntent에서만 사용하는 typed desired state입니다.
	UPROPERTY()
	FCFVehicleMassIntent MassIntent;

	// SetDurabilityIntent에서만 사용하는 typed desired state입니다.
	UPROPERTY()
	FCFVehicleDurabilityIntent DurabilityIntent;

	// SetDefaultDataIntent에서만 사용하는 typed desired state입니다.
	UPROPERTY()
	FCFVehicleDefaultIntent DefaultDataIntent;

	// SetWheelVisualIntent에서만 사용하는 typed desired state입니다.
	UPROPERTY()
	FCFWheelVisualIntent WheelVisualIntent;

	// SetDriveStateMode에서만 사용하는 typed desired state입니다.
	UPROPERTY()
	ECFVehicleDriveStateMode DriveStateMode = ECFVehicleDriveStateMode::ProjectDefault;

	// UpsertHardpointIntent에서만 사용하는 Stable-ID semantic desired element입니다.
	UPROPERTY()
	FCFHardpointIntent HardpointIntent;

	// UpsertMountIntent에서만 사용하는 Stable-ID semantic desired element입니다.
	UPROPERTY()
	FCFMountIntent MountIntent;
};

/** Recipe/Target 기준의 common read/resolve request입니다. */
USTRUCT()
struct FCFVehicleAuthoringReadRequest
{
	GENERATED_BODY()

	// Authoring truth를 읽을 Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// null이면 Recipe.TargetVehicleData를 사용하고, 지정하면 binding과 동일 identity인지 검증합니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> TargetVehicleData = nullptr;

	// Optional Fitting preview context 사용 여부입니다.
	UPROPERTY()
	bool bHasPreviewContext = false;

	// Resolver source winner가 될 수 없는 optional preview context입니다.
	UPROPERTY()
	FCFVehicleResolverPreviewContext PreviewContext;

	// Caller diagnostic identity입니다. R0는 approval이 필요하지 않습니다.
	UPROPERTY()
	ECFAuthoringCallerKind CallerKind = ECFAuthoringCallerKind::Unknown;
};

/** ListVehicles의 bounded foundation filter입니다. */
USTRUCT()
struct FCFVehicleListRequest
{
	GENERATED_BODY()

	// AssetName/ObjectPath에 case-insensitive로 적용할 optional 검색 문자열입니다.
	UPROPERTY()
	FString SearchText;

	// Recipe가 연결된 managed/imported record를 포함할지 여부입니다.
	UPROPERTY()
	bool bIncludeRecipeRecords = true;

		// Recipe가 없는 raw Definition record를 포함할지 여부입니다.
	UPROPERTY()
	bool bIncludeUnmanagedDefinitions = true;

	// Existing VehicleData가 실제 사용하는 ChassisMesh sibling directory에서 아직 Definition에 연결되지 않은 Mesh Candidate를 포함할지 여부입니다.
	UPROPERTY()
	bool bIncludeMeshCandidates = true;
};

/** ListVehicles가 반환하는 lightweight record입니다. */
USTRUCT()
struct FCFVehicleListEntry
{
	GENERATED_BODY()

	// Vehicle Definition soft object path입니다.
	UPROPERTY()
	FSoftObjectPath DefinitionPath;

	// 연결된 Recipe soft object path입니다.
	UPROPERTY()
	FSoftObjectPath RecipePath;

	// Recipe가 로드된 경우의 persistent identity입니다.
	UPROPERTY()
	FGuid RecipeId;

	// Recipe 기준 management state이며 Recipe가 없으면 Unmanaged입니다.
	UPROPERTY()
	ECFVehicleManageState ManageState = ECFVehicleManageState::Unmanaged;

		// Recipe가 존재하는 경우 Advanced Override 수입니다.
	UPROPERTY()
	int32 AdvancedOverrideCount = 0;

	// 이 row가 Definition이 아니라 아직 Vehicle record로 생성되지 않은 StaticMesh candidate인지 여부입니다.
	UPROPERTY()
	bool bMeshOnlyCandidate = false;

	// Mesh-only Candidate일 때 explicit Create Vehicle From Mesh 입력으로 사용할 StaticMesh path입니다.
	UPROPERTY()
	FSoftObjectPath ChassisMeshPath;
};

/** ListVehicles R0 결과입니다. */
USTRUCT()
struct FCFVehicleListResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Definition path 기준 deterministic 정렬된 records입니다.
	UPROPERTY()
	TArray<FCFVehicleListEntry> Vehicles;
};

/** ListProfiles의 optional Domain filter입니다. */
USTRUCT()
struct FCFProfileListRequest
{
	GENERATED_BODY()

	// None이면 5개 Domain 전체를 반환합니다.
	UPROPERTY()
	ECFVehicleProfileDomain Domain = ECFVehicleProfileDomain::None;
};

/** Profile lightweight list item입니다. */
USTRUCT()
struct FCFProfileListEntry
{
	GENERATED_BODY()

	// 5개 Frozen Profile Domain 중 하나입니다.
	UPROPERTY()
	ECFVehicleProfileDomain Domain = ECFVehicleProfileDomain::None;

	// Profile asset soft object path입니다.
	UPROPERTY()
	FSoftObjectPath ProfilePath;

	// Profile metadata의 표시 이름입니다.
	UPROPERTY()
	FText DisplayName;

	// Profile diagnostic revision입니다.
	UPROPERTY()
	int32 AuthoringRevision = 0;

	// Resolver payload deterministic fingerprint입니다.
	UPROPERTY()
	FString ProfileFingerprint;
};

/** ListProfiles R0 결과입니다. */
USTRUCT()
struct FCFProfileListResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Domain/path 기준 deterministic 정렬된 profile 목록입니다.
	UPROPERTY()
	TArray<FCFProfileListEntry> Profiles;
};

/** ReadProfile의 exact typed source request입니다. */
USTRUCT()
struct FCFProfileReadRequest
{
	GENERATED_BODY()

	// 기대하는 exact Profile Domain입니다.
	UPROPERTY()
	ECFVehicleProfileDomain Domain = ECFVehicleProfileDomain::None;

	// 읽을 existing Profile asset path입니다.
	UPROPERTY()
	FSoftObjectPath ProfilePath;
};

/** ReadProfile은 기존 SnapshotBuilder output을 얇게 노출합니다. */
USTRUCT()
struct FCFProfileReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// 읽은 exact Domain입니다.
	UPROPERTY()
	ECFVehicleProfileDomain Domain = ECFVehicleProfileDomain::None;

	// 기존 5-domain Snapshot type을 재사용하고 요청 Domain만 source가 present합니다.
	UPROPERTY()
	FCFVehicleProfileSnapshotSet Snapshot;
};

/** ReadVehicleContext의 bounded default context입니다. */
USTRUCT()
struct FCFVehicleContextReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Persistent Recipe identity입니다.
	UPROPERTY()
	FGuid RecipeId;

	// Recipe asset/object identity입니다.
	UPROPERTY()
	FSoftObjectPath RecipePath;

	// Bound Target Definition identity입니다.
	UPROPERTY()
	FSoftObjectPath DefinitionPath;

	// Current migration management state입니다.
	UPROPERTY()
	ECFVehicleManageState ManageState = ECFVehicleManageState::Unmanaged;

	// 5개 Profile binding의 typed current state입니다.
	UPROPERTY()
	FCFVehicleProfileBindings ProfileBindings;

	// Normal Legacy Pin 수입니다.
	UPROPERTY()
	int32 LegacyPinnedFieldCount = 0;

	// Hidden Legacy Serialized passthrough 수입니다.
	UPROPERTY()
	int32 LegacySerializedFieldCount = 0;

	// Current Advanced Override 수입니다.
	UPROPERTY()
	int32 AdvancedOverrideCount = 0;

	// Last successful Apply의 Definition hash입니다.
	UPROPERTY()
	FString AppliedDefinitionHash;

	// Fresh Resolve의 stale/drift summary입니다.
	UPROPERTY()
	FCFVehicleStaleReport StaleReport;
};

/** ResolveVehiclePreview R0 결과입니다. */
USTRUCT()
struct FCFVehicleResolveReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Shared SnapshotBuilder/AssetReader로 구성한 exact immutable request입니다.
	UPROPERTY()
	FCFVehicleResolveRequest ResolveRequest;

	// Shared Pure Resolver의 exact result입니다.
	UPROPERTY()
	FCFVehicleResolveResult ResolveResult;
};

/** ReadPendingDiff R0 결과입니다. */
USTRUCT()
struct FCFVehicleDiffReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Shared Resolver FieldDiff authority를 그대로 복사합니다.
	UPROPERTY()
	TArray<FCFVehicleFieldDiff> FieldDiff;

	// FieldDiff exact deterministic hash입니다.
	UPROPERTY()
	FString DiffHash;
};

/** ReadSourceTrace R0 결과입니다. */
USTRUCT()
struct FCFVehicleTraceReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Shared Resolver PreviewSourceTrace를 optional field filter로 얇게 projection한 결과입니다.
	UPROPERTY()
	TArray<FCFVehicleSourceTrace> SourceTrace;
};

/** ReadValidation R0 결과입니다. */
USTRUCT()
struct FCFVehicleValidationReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Shared Resolver Recipe layer validation입니다.
	UPROPERTY()
	TArray<FCFVehicleValidationIssue> RecipeValidation;

	// Shared Resolver candidate/cross-field validation입니다.
	UPROPERTY()
	TArray<FCFVehicleValidationIssue> ResolverValidation;

	// Shared R15 existing UCFVDAValidator result입니다.
	UPROPERTY()
	TArray<FCFVehicleValidationIssue> DefinitionValidation;

	// Shared R16 stale/drift report입니다.
	UPROPERTY()
	FCFVehicleStaleReport StaleReport;
};

/** ReviewExternalDrift R0 결과입니다. */
USTRUCT()
struct FCFVehicleDriftReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Shared R16 stale/drift authority입니다.
	UPROPERTY()
	FCFVehicleStaleReport StaleReport;
};

/** Legacy Wizard guard와 UI가 Target의 managed Recipe 존재 여부를 facade로 조회하는 R0 결과입니다. */
USTRUCT()
struct FCFVehicleManagedReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Target을 binding한 Recipe가 존재하는지 여부입니다.
	UPROPERTY()
	bool bManaged = false;

	// 존재할 때 exact Recipe object path입니다.
	UPROPERTY()
	FSoftObjectPath RecipePath;

	// 존재할 때 Recipe management state입니다.
	UPROPERTY()
	ECFVehicleManageState ManageState = ECFVehicleManageState::Unmanaged;
};

/** Reference Vehicle Compare의 A/B source request입니다. */
USTRUCT()
struct FCFVehicleReferenceCompareRequest
{
	GENERATED_BODY()

	// A side current Workspace Recipe입니다. ResolvedPreview 모드에서는 필수입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> CurrentRecipe = nullptr;

	// A side current Workspace Target입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> CurrentTarget = nullptr;

	// A side value mode입니다.
	UPROPERTY()
	ECFVehicleCompareValueMode CurrentMode = ECFVehicleCompareValueMode::ResolvedPreview;

	// B side Reference Recipe입니다. ResolvedPreview 모드일 때만 필요하며 unmanaged reference에서는 null일 수 있습니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> ReferenceRecipe = nullptr;

	// B side Reference Target입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> ReferenceTarget = nullptr;

	// B side value mode입니다.
	UPROPERTY()
	ECFVehicleCompareValueMode ReferenceMode = ECFVehicleCompareValueMode::CurrentDefinition;

	// 동일 값 row도 결과에 포함할지 여부입니다.
	UPROPERTY()
	bool bIncludeSameValues = true;
};

/** 117 Registry 기반 Reference Compare row입니다. */
USTRUCT()
struct FCFVehicleReferenceCompareRow
{
	GENERATED_BODY()

	// 비교 대상 exact stable field입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// A side에 exact value가 존재하는지 여부입니다.
	UPROPERTY()
	bool bHasCurrentValue = false;

	// A side canonical typed value입니다.
	UPROPERTY()
	FCFVehicleFieldValue CurrentValue;

	// B side에 exact value가 존재하는지 여부입니다.
	UPROPERTY()
	bool bHasReferenceValue = false;

	// B side canonical typed value입니다.
	UPROPERTY()
	FCFVehicleFieldValue ReferenceValue;

	// 존재 여부/type/value가 모두 같은지 여부입니다.
	UPROPERTY()
	bool bSame = false;

	// A가 resolved preview일 때의 effective source type입니다.
	UPROPERTY()
	ECFVehicleSourceType CurrentSourceType = ECFVehicleSourceType::ProjectCompatibilityDefault;

	// A가 resolved preview일 때의 effective source id입니다.
	UPROPERTY()
	FString CurrentSourceId;

	// B가 resolved preview일 때의 effective source type입니다.
	UPROPERTY()
	ECFVehicleSourceType ReferenceSourceType = ECFVehicleSourceType::ProjectCompatibilityDefault;

	// B가 resolved preview일 때의 effective source id입니다.
	UPROPERTY()
	FString ReferenceSourceId;
};

/** Reference Vehicle Compare R0 결과입니다. */
USTRUCT()
struct FCFVehicleReferenceCompareResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Stable Field Path 순서의 read-only compare rows입니다.
	UPROPERTY()
	TArray<FCFVehicleReferenceCompareRow> Rows;

	// Different row 수입니다.
	UPROPERTY()
	int32 DifferentCount = 0;
};

/** Legacy Pin Adoption preview/commit이 공유하는 typed scope request입니다. */
USTRUCT()
struct FCFVehicleAdoptionRequest
{
	GENERATED_BODY()

	// Persistent managed Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// Recipe binding과 동일한 Target입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> TargetVehicleData = nullptr;

	// Group 또는 exact Field adoption scope입니다.
	UPROPERTY()
	ECFVehicleAdoptionScope Scope = ECFVehicleAdoptionScope::Group;

	// Group scope의 adoption group입니다.
	UPROPERTY()
	ECFVehicleAdoptGroup AdoptionGroup = ECFVehicleAdoptGroup::VisualAssets;

	// Field scope의 stable field path입니다.
	UPROPERTY()
	FCFVehicleFieldPath AdoptionFieldPath;

	// Preview는 CallerKind만 사용하고 Commit은 exact R2 approval/current fingerprint를 요구합니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};

/** Existing Import Core Adoption preview를 approval proposal과 함께 노출하는 R0 결과입니다. */
USTRUCT()
struct FCFVehicleAdoptionPreviewResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// R2 commit에 binding할 exact proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal Proposal;

	// Existing Import Core의 exact prospective adoption 결과입니다.
	UPROPERTY()
	FCFVehicleAdoptionPreview Preview;
};

/** Wheel measurement proposal read request입니다. */
USTRUCT()
struct FCFVehicleMeasurementReadRequest
{
	GENERATED_BODY()

	// Persistent managed Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// Recipe binding과 동일한 Target입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> TargetVehicleData = nullptr;

	// Caller diagnostic identity입니다.
	UPROPERTY()
	ECFAuthoringCallerKind CallerKind = ECFAuthoringCallerKind::Unknown;
};

/** Pure Resolver가 만든 Wheel measurement proposals를 그대로 projection하는 R0 결과입니다. */
USTRUCT()
struct FCFVehicleMeasurementReadResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// R6 exact measurement proposals입니다.
	UPROPERTY()
	TArray<FCFVehicleMeasurementProposal> Proposals;

	// Proposal baseline Recipe fingerprint입니다.
	UPROPERTY()
	FString RecipeFingerprint;
};

/** Measurement decision preview/commit request입니다. */
USTRUCT()
struct FCFVehicleMeasurementRequest
{
	GENERATED_BODY()

	// Persistent managed Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// Recipe binding과 동일한 Target입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> TargetVehicleData = nullptr;

	// exact measurement target field입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// UI가 검토한 exact measurement rule입니다.
	UPROPERTY()
	FName MeasurementRuleId = NAME_None;

	// UI가 검토한 exact asset fingerprint입니다.
	UPROPERTY()
	FString AssetFingerprint;

	// UI가 검토한 exact candidate typed value입니다.
	UPROPERTY()
	FCFVehicleFieldValue MeasuredCandidateValue;

	// Accept measured 또는 compatibility default 유지 decision입니다.
	UPROPERTY()
	ECFVehicleMeasureDecision Decision = ECFVehicleMeasureDecision::AcceptMeasuredValue;

	// Preview는 CallerKind만 사용하고 Commit은 exact R2 approval/current state를 요구합니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};

/** Measurement decision prospective resolve와 exact R2 proposal입니다. */
USTRUCT()
struct FCFVehicleMeasurementPreviewResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// R2 commit에 binding할 exact proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal Proposal;

	// Decision을 transient Recipe Snapshot에 반영한 prospective Resolver result입니다.
	UPROPERTY()
	FCFVehicleResolveResult ProspectiveResolveResult;
};

/** PreviewRecipeChange/CommitRecipeChange가 공유하는 single-Recipe typed request입니다. */
USTRUCT()
struct FCFVehicleRecipeChangeRequest
{
	GENERATED_BODY()

	// Persistent Recipe입니다. Preview에서는 읽기만 하고 Commit에서만 transaction mutation합니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// null이면 Recipe binding Target을 사용합니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> TargetVehicleData = nullptr;

	// Raw field path가 아닌 discriminated semantic desired-state command입니다.
	UPROPERTY()
	FCFVehicleSemanticChange Change;

	// Preview에서는 CallerKind만 사용하고 Commit에서는 expected state + approval을 요구합니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;

	// Optional Fitting preview context 사용 여부입니다.
	UPROPERTY()
	bool bHasPreviewContext = false;

	// Source winner가 될 수 없는 optional Fitting preview context입니다.
	UPROPERTY()
	FCFVehicleResolverPreviewContext PreviewContext;
};

/** PreviewRecipeChange R0 result입니다. */
USTRUCT()
struct FCFVehicleRecipePreviewResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Commit approval에 binding할 exact R1 proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal Proposal;

	// Shared Pure Resolver의 prospective exact result입니다.
	UPROPERTY()
	FCFVehicleResolveResult ProspectiveResolveResult;

	// Prospective Recipe의 migration/adoption state입니다.
	UPROPERTY()
	FCFVehicleImportState ProspectiveImportState;
};

/** Existing unmanaged Definition을 Authoring으로 가져오기 위한 reviewed R2 request입니다. */
USTRUCT()
struct FCFVehicleInitialImportRequest
{
	GENERATED_BODY()

	// Recipe를 새로 만들 unmanaged Runtime canonical VehicleData입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> TargetVehicleData = nullptr;

	// 새 Recipe package를 생성할 long package folder입니다. 일반 UI는 /Game 이하만 허용합니다.
	UPROPERTY()
	FString RecipePackagePath;

	// 새 Recipe UObject/asset에 사용할 이름입니다.
	UPROPERTY()
	FName RecipeAssetName = NAME_None;

	// Preview에서는 CallerKind만 사용하고 Commit에서는 exact R2 approval/current state를 요구합니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};

/** Initial Import가 Target mutation 없이 보여줄 exact reviewed proposal입니다. */
USTRUCT()
struct FCFVehicleInitialImportPreviewResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// OwnershipWrite commit에 binding할 exact R2 proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal Proposal;

	// Commit 성공 시 만들어질 exact Recipe object path입니다.
	UPROPERTY()
	FSoftObjectPath ProspectiveRecipePath;

	// Existing Import Core가 계산한 exact Legacy/direct semantic summary입니다.
	UPROPERTY()
	FCFVehicleImportResult ImportSummary;
};

/** Initial Import commit의 new Recipe identity와 mutation footprint입니다. */
USTRUCT()
struct FCFVehicleInitialImportResult
{
	GENERATED_BODY()

	// Common typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// 성공 시 생성된 persistent Editor-only Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// 성공 시 생성된 Recipe exact object path입니다.
	UPROPERTY()
	FSoftObjectPath RecipePath;

	// Commit에 실제 사용된 Existing Import summary입니다.
	UPROPERTY()
	FCFVehicleImportResult ImportSummary;
};

/** BuildApplyApprovalProposal / ApplyResolvedVehicle가 공유하는 R3 typed request입니다. */
USTRUCT()
struct FCFVehicleApplyOpRequest
{
	GENERATED_BODY()

	// Section 22.33~22.35 shared ApplyService request를 그대로 포함합니다.
	UPROPERTY()
	FCFVehicleApplyRequest ApplyRequest;

	// Approval scope가 binding할 reviewed FieldDiff hash입니다.
	UPROPERTY()
	FString ExpectedDiffHash;

	// DefinitionApply approval + common expected state입니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};
