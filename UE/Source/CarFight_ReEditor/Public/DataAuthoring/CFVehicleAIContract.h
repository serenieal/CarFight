// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAIContract.h
// Version: v1.20.0
// Date: 2026-09-02
// Description: DAUTH Common Authoring Service + CF-FQ-040 ESH-03 WheelTorqueCrossoverShift diagnostic contract입니다.
// Scope: Section 25 R0~R3 risk, approval, read/query와 normal Workspace Mesh-only candidate projection을 제공합니다.
// Changelog:
// - v1.20.0: CF-FQ-043 VMG-P0-04 Gameplay Guidance R0 request에 persistent Recipe HardpointPlanMode의 transient projection을 추가. Resolver RecipeSnapshot/fingerprint에는 넣지 않고 Step 6 Mode authority만 전달.
// - v1.19.0: CF-FQ-043 stable identity 삭제를 위해 append-only RemoveMountIntent/RemoveHardpointIntent semantic op와 exact deletion identity payload를 추가. 기존 enum ordinal은 유지.
// - v1.18.0: vehicle-specific Engine Curve + gear ratios로 adjacent gear Wheel-Torque crossover와 fixed common ChangeUpRPM recommendation을 계산하는 typed diagnostic을 TransmissionDiagnostic에 추가.
// - v1.17.0: PhysicsDraft schema v3에 EngineCurveReview를 추가하고 Profile Preview/Final Review에 EngineCurveProposalHash/Diagnostic projection을 additive 추가.
// - v1.16.0: Existing Reference Evidence complete research replacement를 fresh fingerprint/AuthoringWrite approval에 binding하는 Builder Evidence Refresh R1 request/preview/result와 mutation footprint를 추가.
// - v1.15.0: Builder-wide fixed-shift diagnostic row에 RpmRetention을 명시적으로 추가해 문서/Step5/FinalReview projection contract를 일치시킴.
// - v1.14.0: Guided 신규 차량 Transmission remediation을 위해 PhysicsDraft/commit/final-review contract에 field-level Transmission review hash와 fixed-shift diagnostic projection을 추가.
// - v1.13.0: VB-P0-09 Step 5에서 current accepted Evidence + complete Builder-private 4 Profile proposal을 AI→Guided Shell에 넘기는 transient FCFBuilderPhysicsDraft contract를 추가.
// - v1.12.0: VB-P0-09 Step 1에서 initial Reference Research payload/EvidenceId를 Companion approval에 binding하고 AI→Shell ResearchDraft typed handoff를 추가.
// - v1.11.0: Final Review provenance를 persistent BuilderCommitReceipt/current 4 Profile fingerprint에 binding하고, Undo token에 semantic Recipe + post-Apply Target/AppliedState stale guard를 추가.
// - v1.10.0: VB-P0-07 Final Review의 validation/drift/gameplay/diff/provenance summary, exact R3 Apply approval과 Builder-owned UE transaction guarded Undo token contract를 추가.
// - v1.9.0: VB-P0-06 Durability/Defense/DestroyedFx/Hardpoint/Mount/DriveState/WheelVisual/FittingMass를 current Recipe/Resolver/Target에서 읽어 Complete/Optional/NeedsReview/Blocked로 분류하는 R0 Builder guidance contract를 추가.
// - v1.8.0: Existing Vehicle Completion이 missing private Profile 기본값 binding으로 주행 특성을 바꾸지 않도록 New/CompleteExisting mode, complete initial Profile seed payload와 current→prospective Resolver hash preservation evidence를 추가.
// - v1.7.0: VB-P0-05 최소 7-Asset 경로를 닫기 위해 existing Definition+Recipe 뒤 Evidence + private 4 Profile을 생성·owner/binding하는 R2 Builder companion typed request/preview/result를 추가.
// - v1.6.0: VB-P0-05 설계 검수 교정으로 Builder commit에 typed Evidence path/id/fingerprint/consumed claims와 prospective Resolver signature/hash binding을 추가. opaque upstream hash는 보조 correlation 값으로 격하.
// - v1.5.0: VB-P0-05 Builder-private VehicleBase/Drivetrain/Handling/Performance complete payload를 4개 current/prospective fingerprint와 OwnerRecipeId에 binding하는 atomic typed commit request/preview를 추가.
// - v1.4.0: P0-12 UA-07 baseline-safe Profile 복구를 위해 explicit R1 UnbindVehicleProfile semantic operation을 additive 추가. Bind의 non-empty path 계약은 유지.
// - v1.3.0: P0-11 ListVehicles에 bounded Mesh-only Candidate projection metadata를 추가하되 AI raw writer/create operation은 추가하지 않음.
// - v1.2.0: P0-10 Reference Compare, Legacy Adoption, Wheel Measurement, managed-target lookup typed R0/R2 contract를 추가.
// - v1.1.0: P0-09 reviewed Initial Import proposal/commit을 UI와 공용 facade에서 사용할 typed R2 contract로 추가.
// - v1.0.0: AI/UI 공용 typed request/result foundation 최초 구현.
// Migration:
// - Raw SetField/UObject property patch/direct VehicleData write API를 제공하지 않습니다.
// - v1.5.0의 Profile write는 Meta.OwnerRecipeId가 exact RecipeId인 Builder-private 4 Profile만 허용합니다. Invalid/mismatch owner인 shared/legacy Profile은 fail-closed이며 기존 Shared Profile 편집 경로를 우회하지 않습니다.
// - Builder-private typed commit은 fresh Reference Evidence + current Recipe/Target/private Profile + prospective Resolver result에 binding한 4 Profile complete payload만 한 transaction으로 갱신하며 Save/SaveAll, force/skip-validation, automatic retry를 제공하지 않습니다.
// - 일반 Shared Profile payload writer, Batch/CSV, Save/SaveAll, force/skip-validation 옵션을 제공하지 않습니다.
// - v1.4.0의 UnbindVehicleProfile은 Profile Asset을 삭제하거나 수정하지 않고 Recipe의 선택 Domain binding만 명시적으로 비웁니다.
// - v1.9.0 Builder Gameplay Guidance는 R0 read-only입니다. Hardpoint Socket을 생성·이동하지 않고, Existing Vehicle Completion에서는 current Target의 기존 LocalTransform을 baseline-safe하게 보존할 수 있습니다.
// - v1.10.0 Builder Final Review는 기존 Validator/Resolver/Drift/ApplyService를 재사용합니다. Apply는 exact DefinitionApply approval 뒤에만 수행되고 Undo는 이 Builder가 생성한 exact UE transaction token이 현재 Undo stack top일 때만 허용합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleApplyService.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "CFVehicleAIContract.generated.h"

class UCFVehicleData;
class UCFVehicleRecipeData;
class UCFVehicleRefEvidence;
class UCFVehicleBaseProfile;
class UCFDrivetrainProfile;
class UCFHandlingProfile;
class UCFPerformanceProfile;

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
	UnbindVehicleProfile,
	SetVehicleArchetype,
	SetVehicleAssetIntent,
	SetDrivingFeel,
	SetMassIntent,
	SetDurabilityIntent,
	SetDefaultDataIntent,
	SetWheelVisualIntent,
	SetDriveStateMode,
	UpsertHardpointIntent,
	UpsertMountIntent,
	RemoveMountIntent,
	RemoveHardpointIntent
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

	// Shared/Profile payload가 persistent mutation됐는지 여부입니다.
	UPROPERTY()
	bool bProfileChanged = false;

	// Existing Reference Evidence research payload가 persistent mutation됐는지 여부입니다.
	UPROPERTY()
	bool bEvidenceChanged = false;

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

	// RemoveMountIntent에서만 사용하는 exact MountProfile stable identity입니다.
	UPROPERTY()
	FName RemoveMountProfileId = NAME_None;

	// RemoveHardpointIntent에서만 사용하는 exact LocationSlot stable identity입니다.
	UPROPERTY()
	FName RemoveHardpointLocationSlotId = NAME_None;
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

/** Guided Vehicle Builder가 한 차량에 대해 제안하는 private 4 Profile complete payload입니다. */
USTRUCT()
struct FCFBuilderPrivateProfilePayload
{
	GENERATED_BODY()

	// Builder-private VehicleBase Profile asset identity입니다.
	UPROPERTY()
	FSoftObjectPath VehicleBaseProfilePath;

	// Builder-private Drivetrain Profile asset identity입니다.
	UPROPERTY()
	FSoftObjectPath DrivetrainProfilePath;

	// Builder-private Handling Profile asset identity입니다.
	UPROPERTY()
	FSoftObjectPath HandlingProfilePath;

	// Builder-private Performance Profile asset identity입니다.
	UPROPERTY()
	FSoftObjectPath PerformanceProfilePath;

	// VehicleBase의 complete prospective typed payload입니다.
	UPROPERTY()
	FCFVehicleBaseProfileData VehicleBaseData;

	// Drivetrain의 complete prospective typed payload입니다.
	UPROPERTY()
	FCFDrivetrainProfileData DrivetrainData;

	// Handling의 complete prospective typed payload입니다.
	UPROPERTY()
	FCFHandlingProfileData HandlingData;

	// Performance의 complete prospective typed payload입니다.
	UPROPERTY()
	FCFPerformanceProfileData PerformanceData;
};

/** Builder-private 4 Profile current fingerprint를 exact domain별로 보관합니다. */
USTRUCT()
struct FCFBuilderProfileFingerprints
{
	GENERATED_BODY()

	// VehicleBase Profile fingerprint입니다.
	UPROPERTY()
	FString VehicleBaseFingerprint;

	// Drivetrain Profile fingerprint입니다.
	UPROPERTY()
	FString DrivetrainFingerprint;

	// Handling Profile fingerprint입니다.
	UPROPERTY()
	FString HandlingFingerprint;

	// Performance Profile fingerprint입니다.
	UPROPERTY()
	FString PerformanceFingerprint;
};

/** Builder가 새 companion asset 하나를 만들 exact package/object identity입니다. */
USTRUCT()
struct FCFBuilderAssetIdentity
{
	GENERATED_BODY()

	// 새 asset을 생성할 Unreal long package name입니다.
	UPROPERTY()
	FString PackageName;

	// 새 asset의 object name입니다.
	UPROPERTY()
	FName AssetName = NAME_None;
};

/** Builder companion completeness가 신규 차량인지 기존 차량 보완인지 구분합니다. */
UENUM()
enum class ECFBuilderCompanionMode : uint8
{
	NewVehicle,
	CompleteExisting
};

/** AI가 조사한 Reference와 complete private Profile seed를 Guided Shell에 넘기는 transient typed draft입니다. Persistent SSOT나 approval token이 아닙니다. */
USTRUCT()
struct FCFBuilderResearchDraft
{
	GENERATED_BODY()

	// Research Draft JSON schema revision입니다.
	UPROPERTY()
	int32 SchemaRevision = 1;

	// 이 Draft가 exact binding된 managed Recipe identity입니다.
	UPROPERTY()
	FGuid RecipeId;

	// 이 Draft가 exact binding된 Target VehicleData path입니다.
	UPROPERTY()
	FSoftObjectPath TargetDefinitionPath;

	// 새 Reference Evidence에 넣을 normalized FACT/DERIVED research payload입니다.
	UPROPERTY()
	FCFVehicleRefEvidencePayload EvidencePayload;

	// Missing Builder-private Profile을 current effective/reference proposal에서 완전하게 seed할 typed payload입니다.
	UPROPERTY()
	FCFBuilderPrivateProfilePayload InitialProfilePayload;

	// 사람이 어떤 Research Draft인지 식별할 diagnostic label입니다. Persistent authority가 아닙니다.
	UPROPERTY()
	FString ResearchLabel;
};

/** Existing Reference Evidence research payload를 complete replacement하는 reviewed R1 request입니다. Evidence identity/ownership binding은 변경하지 않습니다. */
USTRUCT()
struct FCFBuilderEvidenceRefreshRequest
{
	GENERATED_BODY()

	// Refresh 대상 Evidence를 소유하는 exact managed Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// Current Recipe에 exact binding된 existing Reference Evidence object path입니다.
	UPROPERTY()
	FSoftObjectPath EvidencePath;

	// Draft 작성 시점에 읽은 current Evidence fingerprint입니다. Preview/Commit stale guard입니다.
	UPROPERTY()
	FString ExpectedCurrentEvidenceFingerprint;

	// 기존 research payload 전체를 대체할 normalized FACT/DERIVED complete payload입니다. GAME_BIAS는 Initial Research와 동일하게 금지합니다.
	UPROPERTY()
	FCFVehicleRefEvidencePayload EvidencePayload;

	// Preview caller identity와 Commit AuthoringWrite approval/current Recipe/Target state를 전달합니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};

/** Existing Reference Evidence refresh의 mutation0 R1 preview입니다. */
USTRUCT()
struct FCFBuilderEvidenceRefreshPreview
{
	GENERATED_BODY()

	// Common typed preview result입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Commit에 binding할 exact AuthoringWrite proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal Proposal;

	// Preview에서 fresh 재계산한 current Evidence fingerprint입니다.
	UPROPERTY()
	FString CurrentEvidenceFingerprint;

	// Complete replacement payload를 transient Evidence에 적용해 계산한 prospective fingerprint입니다.
	UPROPERTY()
	FString ProspectiveEvidenceFingerprint;

	// USER review를 위한 current canonical Claim 수입니다.
	UPROPERTY()
	int32 CurrentClaimCount = 0;

	// USER review를 위한 prospective canonical/candidate Claim 전체 수입니다.
	UPROPERTY()
	int32 ProspectiveClaimCount = 0;

	// USER review를 위한 current Unknown Fact 수입니다.
	UPROPERTY()
	int32 CurrentUnknownFactCount = 0;

	// USER review를 위한 prospective Unknown Fact 수입니다.
	UPROPERTY()
	int32 ProspectiveUnknownFactCount = 0;
};

/** Existing Reference Evidence refresh terminal result입니다. */
USTRUCT()
struct FCFBuilderEvidenceRefreshResult
{
	GENERATED_BODY()

	// Common typed terminal result입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Commit 뒤 fresh readback한 Evidence fingerprint입니다.
	UPROPERTY()
	FString EvidenceFingerprint;
};

/** Builder Step 5 / Final Review가 공유하는 vehicle-specific Engine Curve provenance/fidelity diagnostic입니다. */
USTRUCT()
struct FCFBuilderEngineCurveDiagnostic
{
	GENERATED_BODY()

	// Engine Curve provenance/fidelity 검증을 수행했는지 여부입니다.
	UPROPERTY()
	bool bEvaluated = false;

	// Current Performance payload가 vehicle-specific Engine Curve를 실제 사용하도록 opt-in했는지 여부입니다.
	UPROPERTY()
	bool bVehicleSpecificCurveEnabled = false;

	// Current reviewed Engine Curve proposal hash입니다. BaselineInherited/legacy에서는 비어 있을 수 있습니다.
	UPROPERTY()
	FString EngineCurveProposalHash;

	// USER가 확인해야 하지만 Profile commit/Final Review를 즉시 막지는 않는 diagnostic입니다.
	UPROPERTY()
	TArray<FString> Warnings;

	// Invalid/stale provenance 또는 enabled curve payload contradiction으로 commit/Final Review를 막는 diagnostic입니다.
	UPROPERTY()
	TArray<FString> Blockers;
};

/** AI가 current accepted Reference Evidence를 근거로 작성한 Step 5 complete 4-Profile Physics Proposal transient draft입니다. Persistent SSOT나 approval token이 아닙니다. */
USTRUCT()
struct FCFBuilderPhysicsDraft
{
	GENERATED_BODY()

	// Physics Draft JSON schema revision입니다. v2부터 Transmission review, v3부터 Engine Curve review가 포함됩니다.
	UPROPERTY()
	int32 SchemaRevision = 3;

	// 이 Physics Draft가 exact binding된 managed Recipe identity입니다.
	UPROPERTY()
	FGuid RecipeId;

	// 이 Physics Draft가 exact binding된 Target VehicleData path입니다.
	UPROPERTY()
	FSoftObjectPath TargetDefinitionPath;

	// 이 Proposal이 소비한 exact accepted Reference Evidence identity입니다.
	UPROPERTY()
	FGuid EvidenceId;

	// Proposal 작성 시점에 소비한 exact Reference Evidence semantic fingerprint입니다.
	UPROPERTY()
	FString ExpectedEvidenceFingerprint;

	// Current Builder-private 4 Profile에 반영할 complete prospective typed payload입니다.
	UPROPERTY()
	FCFBuilderPrivateProfilePayload ProfilePayload;

	// Profile Proposal이 실제로 소비한 canonical Evidence Claim ID 목록입니다.
	UPROPERTY()
	TArray<FName> ConsumedClaimIds;

	// Vehicle-specific Transmission이 필요한 Guided 차량에서 field-level FACT/DERIVED/GAME_BIAS 근거를 보존하는 review metadata입니다.
	UPROPERTY()
	FCFBuilderTransmissionReview TransmissionReview;

	// Vehicle-specific Engine Curve가 FACT/DERIVED/GAME_BIAS 중 어떤 근거로 작성됐는지 보존하는 review metadata입니다.
	UPROPERTY()
	FCFBuilderEngineCurveReview EngineCurveReview;

	// Upstream AI proposal correlation을 stable하게 구분하는 opaque deterministic hash입니다.
	UPROPERTY()
	FString ProposalCorrelationHash;

	// USER가 어떤 제안인지 식별할 짧은 제목입니다. Persistent authority가 아닙니다.
	UPROPERTY()
	FString ProposalLabel;

	// USER가 Chaos 세부 숫자를 몰라도 차량 특성 변화 방향을 이해할 수 있게 AI가 작성한 설명입니다.
	UPROPERTY()
	FString UserFacingSummary;
};

/** 한 forward gear의 고정 ChangeUpRPM 기반 kinematic sanity diagnostic입니다. */
USTRUCT()
struct FCFBuilderTransmissionGearDiagnostic
{
	GENERATED_BODY()

	// 1부터 시작하는 forward gear 번호입니다.
	UPROPERTY()
	int32 GearNumber = 0;

	// 현재 proposed forward gear ratio입니다.
	UPROPERTY()
	float GearRatio = 0.0f;

	// GearRatio * FinalRatio 전체 감속비입니다.
	UPROPERTY()
	float OverallRatio = 0.0f;

	// Current effective powered-wheel radius로 이론 shift speed를 계산할 수 있는지 여부입니다.
	UPROPERTY()
	bool bShiftSpeedAvailable = false;

	// Powered wheel radius 범위 중 작은 반경에서 계산한 ChangeUpRPM 기준 예상 차속입니다.
	UPROPERTY()
	float ShiftSpeedMinKmh = 0.0f;

	// Powered wheel radius 범위 중 큰 반경에서 계산한 ChangeUpRPM 기준 예상 차속입니다.
	UPROPERTY()
	float ShiftSpeedMaxKmh = 0.0f;

	// 다음 forward gear가 존재해 post-shift RPM을 계산할 수 있는지 여부입니다.
	UPROPERTY()
	bool bPostShiftAvailable = false;

	// 동일 차속에서 다음 기어로 상향 변속 직후의 이론 RPM입니다.
	UPROPERTY()
	float PostShiftRPM = 0.0f;

	// NextGearRatio / CurrentGearRatio로 표현한 상향변속 RPM 유지 비율입니다.
	UPROPERTY()
	float RpmRetention = 0.0f;

	// PostShiftRPM - ChangeDownRPM 여유입니다.
	UPROPERTY()
	float DownshiftMarginRPM = 0.0f;
};

/** 한 adjacent forward gear pair의 Engine Curve 기반 Wheel-Torque crossover diagnostic입니다. */
USTRUCT()
struct FCFBuilderWheelTorquePairDiagnostic
{
	GENERATED_BODY()

	// 현재 forward gear 번호입니다.
	UPROPERTY()
	int32 FromGearNumber = 0;

	// 다음 forward gear 번호입니다.
	UPROPERTY()
	int32 ToGearNumber = 0;

	// 실제 wheel-torque crossover를 EngineMaxRPM 이내에서 찾았는지 여부입니다.
	UPROPERTY()
	bool bCrossoverFound = false;

	// Crossover가 EngineMaxRPM까지 나타나지 않아 EngineMaxRPM을 usable shift limit으로 사용했는지 여부입니다.
	UPROPERTY()
	bool bLimitedByEngineMaxRPM = false;

	// CurrentWheelTorque와 NextWheelTorque가 같아지거나 EngineMaxRPM limit에 도달한 current-gear RPM입니다.
	UPROPERTY()
	float CrossoverRPM = 0.0f;

	// CrossoverRPM에서 다음 gear로 바뀌었을 때의 동일 차속 post-shift RPM입니다.
	UPROPERTY()
	float CrossoverPostShiftRPM = 0.0f;

	// CrossoverRPM 시점 current gear theoretical wheel torque Nm입니다.
	UPROPERTY()
	float CurrentWheelTorqueNm = 0.0f;

	// CrossoverRPM 시점 next gear theoretical wheel torque Nm입니다.
	UPROPERTY()
	float NextWheelTorqueNm = 0.0f;

	// WSA-authoritative powered-wheel radius로 crossover road speed를 계산할 수 있는지 여부입니다.
	UPROPERTY()
	bool bCrossoverSpeedAvailable = false;

	// Powered-wheel radius 범위의 작은 반경에서 계산한 crossover road speed입니다.
	UPROPERTY()
	float CrossoverSpeedMinKmh = 0.0f;

	// Powered-wheel radius 범위의 큰 반경에서 계산한 crossover road speed입니다.
	UPROPERTY()
	float CrossoverSpeedMaxKmh = 0.0f;

	// Builder가 제안한 single common ChangeUpRPM에서 current gear wheel torque입니다.
	UPROPERTY()
	float RecommendedCurrentWheelTorqueNm = 0.0f;

	// Builder가 제안한 single common ChangeUpRPM에서 next gear wheel torque입니다.
	UPROPERTY()
	float RecommendedNextWheelTorqueNm = 0.0f;

	// common threshold에서 더 큰 wheel torque 대비 current/next 차이 비율입니다. 0에 가까울수록 crossover에 가깝습니다.
	UPROPERTY()
	float RecommendedRelativeTorqueGap = 0.0f;

	// common threshold에서 다음 gear로 바뀐 직후 RPM입니다.
	UPROPERTY()
	float RecommendedPostShiftRPM = 0.0f;

	// WSA-authoritative powered-wheel radius로 common threshold road speed를 계산할 수 있는지 여부입니다.
	UPROPERTY()
	bool bRecommendedSpeedAvailable = false;

	// 작은 powered-wheel radius에서 계산한 common threshold road speed입니다.
	UPROPERTY()
	float RecommendedSpeedMinKmh = 0.0f;

	// 큰 powered-wheel radius에서 계산한 common threshold road speed입니다.
	UPROPERTY()
	float RecommendedSpeedMaxKmh = 0.0f;
};

/** P0 single ChangeUpRPM을 위한 Engine Curve 기반 Wheel-Torque crossover 전체 diagnostic입니다. */
USTRUCT()
struct FCFBuilderWheelTorqueShiftDiagnostic
{
	GENERATED_BODY()

	// Engine Curve/gear ratio 기반 계산을 수행했는지 여부입니다.
	UPROPERTY()
	bool bEvaluated = false;

	// 차량별 Engine Curve가 실제로 활성화되어 이 diagnostic의 authority가 있는지 여부입니다.
	UPROPERTY()
	bool bVehicleSpecificEngineCurveAvailable = false;

	// 공통 threshold recommendation 계산 방법의 stable ID입니다.
	UPROPERTY()
	FName MethodId = NAME_None;

	// deterministic 계산 방법 revision입니다.
	UPROPERTY()
	int32 MethodRevision = 0;

	// 탐색을 시작한 current-gear RPM입니다.
	UPROPERTY()
	int32 SearchStartRPM = 0;

	// 탐색 상한인 EngineMaxRPM입니다.
	UPROPERTY()
	int32 SearchEndRPM = 0;

	// 모든 gear pair의 relative torque gap squared mean이 최소가 되는 single common ChangeUpRPM recommendation입니다.
	UPROPERTY()
	int32 RecommendedChangeUpRPM = 0;

	// RecommendedChangeUpRPM에서 adjacent gear pair relative torque gap squared 평균입니다.
	UPROPERTY()
	float MeanSquaredRelativeTorqueGap = 0.0f;

	// RecommendedChangeUpRPM에서 adjacent gear pair 중 가장 큰 relative torque gap입니다.
	UPROPERTY()
	float MaxRelativeTorqueGap = 0.0f;

	// Adjacent forward gear pair별 crossover/result입니다.
	UPROPERTY()
	TArray<FCFBuilderWheelTorquePairDiagnostic> Pairs;

	// 계산은 가능하지만 USER가 확인해야 하는 제약/근사입니다.
	UPROPERTY()
	TArray<FString> Warnings;

	// 계산 authority가 없어 recommendation을 사용할 수 없는 이유입니다.
	UPROPERTY()
	TArray<FString> Blockers;
};

/** Builder Step 5 / Final Review가 공유하는 vehicle-specific Transmission sanity diagnostic입니다. */
USTRUCT()
struct FCFBuilderTransmissionDiagnostic
{
	GENERATED_BODY()

	// Diagnostic 계산을 수행했는지 여부입니다.
	UPROPERTY()
	bool bEvaluated = false;

	// Current Recipe가 vehicle-specific Transmission을 의무화하는지 여부입니다.
	UPROPERTY()
	bool bVehicleSpecificRequired = false;

	// Current accepted Transmission proposal hash입니다. LegacyCompatible에서는 비어 있을 수 있습니다.
	UPROPERTY()
	FString TransmissionProposalHash;

	// Forward gear별 fixed ChangeUpRPM kinematic result입니다.
	UPROPERTY()
	TArray<FCFBuilderTransmissionGearDiagnostic> Gears;

	// Vehicle-specific Engine Curve가 준비됐을 때 계산하는 adjacent-gear Wheel Torque crossover + common ChangeUpRPM recommendation입니다.
	UPROPERTY()
	FCFBuilderWheelTorqueShiftDiagnostic WheelTorqueShift;

	// USER가 확인해야 하지만 Apply를 막지는 않는 diagnostic입니다.
	UPROPERTY()
	TArray<FString> Warnings;

	// Step 5 commit 또는 Final Review completion을 막는 diagnostic입니다.
	UPROPERTY()
	TArray<FString> Blockers;
};

/** Existing Definition+Recipe에 Evidence + private 4 Profile을 붙이는 R2 Builder request입니다. */
USTRUCT()
struct FCFBuilderCompanionRequest
{
	GENERATED_BODY()

	// Companion owner가 될 persistent managed Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// 신규 차량 생성과 기존 차량 보완의 baseline-preservation 규칙을 선택합니다.
	UPROPERTY()
	ECFBuilderCompanionMode Mode = ECFBuilderCompanionMode::NewVehicle;

	// 하나 이상의 private Profile이 Missing일 때 complete initial seed payload를 제공했는지 여부입니다.
	UPROPERTY()
	bool bHasInitialProfilePayload = false;

	// Missing private Profile을 default constructor 값이 아니라 complete reviewed/effective baseline으로 초기화할 typed payload입니다.
	UPROPERTY()
	FCFBuilderPrivateProfilePayload InitialProfilePayload;

	// 기존 정상 Reference Evidence가 있으면 그 exact path입니다. 비어 있으면 EvidenceAsset identity로 새 Evidence를 생성합니다.
	UPROPERTY()
	FSoftObjectPath ExistingEvidencePath;

	// 새 Evidence를 생성할 때 complete normalized Research payload가 제공됐는지 여부입니다.
	UPROPERTY()
	bool bHasInitialEvidencePayload = false;

	// Preview와 Commit이 같은 새 Evidence semantic identity/fingerprint를 사용하도록 caller가 한 번 생성해 고정하는 GUID입니다.
	UPROPERTY()
	FGuid NewEvidenceId;

	// 새 Evidence를 빈 record로 만들지 않고 같은 Companion transaction에서 채울 normalized initial Research payload입니다.
	UPROPERTY()
	FCFVehicleRefEvidencePayload InitialEvidencePayload;

	// ExistingEvidencePath가 비어 있을 때 생성할 새 Reference Evidence asset identity입니다.
	UPROPERTY()
	FCFBuilderAssetIdentity EvidenceAsset;

	// Recipe VehicleBase binding이 비어 있을 때 생성할 새 Builder-private VehicleBase Profile asset identity입니다.
	UPROPERTY()
	FCFBuilderAssetIdentity VehicleBaseAsset;

	// Recipe Drivetrain binding이 비어 있을 때 생성할 새 Builder-private Drivetrain Profile asset identity입니다.
	UPROPERTY()
	FCFBuilderAssetIdentity DrivetrainAsset;

	// Recipe Handling binding이 비어 있을 때 생성할 새 Builder-private Handling Profile asset identity입니다.
	UPROPERTY()
	FCFBuilderAssetIdentity HandlingAsset;

	// Recipe Performance binding이 비어 있을 때 생성할 새 Builder-private Performance Profile asset identity입니다.
	UPROPERTY()
	FCFBuilderAssetIdentity PerformanceAsset;

	// Preview는 caller identity를 사용하고 Commit은 exact R2 expected state + OwnershipWrite approval을 요구합니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};

/** Builder companion creation의 mutation0 R2 preview입니다. */
USTRUCT()
struct FCFBuilderCompanionPreview
{
	GENERATED_BODY()

	// Common typed preview result입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Commit에 binding할 exact OwnershipWrite proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal Proposal;

	// Preview 뒤 사용할 existing 또는 prospective Reference Evidence object path입니다.
	UPROPERTY()
	FSoftObjectPath EvidencePath;

	// Existing 또는 prospective initial Research payload에서 fresh 계산한 exact Evidence fingerprint입니다.
	UPROPERTY()
	FString ProspectiveEvidenceFingerprint;

	// Preview 뒤 사용할 existing 또는 prospective VehicleBase private Profile object path입니다.
	UPROPERTY()
	FSoftObjectPath VehicleBasePath;

	// Preview 뒤 사용할 existing 또는 prospective Drivetrain private Profile object path입니다.
	UPROPERTY()
	FSoftObjectPath DrivetrainPath;

	// Preview 뒤 사용할 existing 또는 prospective Handling private Profile object path입니다.
	UPROPERTY()
	FSoftObjectPath HandlingPath;

	// Preview 뒤 사용할 existing 또는 prospective Performance private Profile object path입니다.
	UPROPERTY()
	FSoftObjectPath PerformancePath;

	// Companion 보완 전 current Recipe의 resolved Definition hash입니다. Current resolve가 성공한 경우에만 채웁니다.
	UPROPERTY()
	FString CurrentResolvedDefinitionHash;

	// Missing private Profile complete seed를 prospective binding한 resolved Definition hash입니다.
	UPROPERTY()
	FString ProspectiveResolvedDefinitionHash;

	// Prospective persistent binding identity + typed payload의 effective Source signature입니다.
	UPROPERTY()
	FString ProspectiveSourceSignature;
};

/** Builder companion creation terminal result입니다. */
USTRUCT()
struct FCFBuilderCompanionResult
{
	GENERATED_BODY()

	// Common typed terminal result입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// 성공 시 생성된 Reference Evidence입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRefEvidence> CreatedEvidence = nullptr;

	// 성공 시 생성된 private VehicleBase Profile입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleBaseProfile> CreatedVehicleBase = nullptr;

	// 성공 시 생성된 private Drivetrain Profile입니다.
	UPROPERTY()
	TObjectPtr<UCFDrivetrainProfile> CreatedDrivetrain = nullptr;

	// 성공 시 생성된 private Handling Profile입니다.
	UPROPERTY()
	TObjectPtr<UCFHandlingProfile> CreatedHandling = nullptr;

	// 성공 시 생성된 private Performance Profile입니다.
	UPROPERTY()
	TObjectPtr<UCFPerformanceProfile> CreatedPerformance = nullptr;
};

/** Builder Gameplay Setup에서 판정할 의미 영역입니다. */
UENUM()
enum class ECFBuilderGameplayArea : uint8
{
	Durability,
	Defense,
	DestroyedFx,
	Hardpoints,
	MountProfiles,
	DriveState,
	WheelVisual,
	FittingMass
};

/** Builder Gameplay Setup 영역 하나의 read-only completeness 상태입니다. */
UENUM()
enum class ECFBuilderGuidanceState : uint8
{
	Complete,
	Optional,
	NeedsReview,
	Blocked
};

/** Builder Gameplay Setup R0 guidance가 읽을 current Recipe/Target context입니다. */
USTRUCT()
struct FCFBuilderGameplayGuidanceRequest
{
	GENERATED_BODY()

	// Existing Authoring R0 read contract를 그대로 재사용합니다.
	UPROPERTY()
	FCFVehicleAuthoringReadRequest ReadRequest;

	// 신규 차량과 Existing Vehicle Completion의 Companion lifecycle을 구분합니다. Hardpoint/Mount 정책 authority로 사용하지 않습니다.
	UPROPERTY()
	ECFBuilderCompanionMode Mode = ECFBuilderCompanionMode::NewVehicle;

	// Recipe-owned Hardpoint/Mount Guidance lifecycle을 R0 request에 transient projection합니다. Resolver fingerprint/hash에는 포함하지 않습니다.
	UPROPERTY()
	ECFBuilderHardpointPlanMode HardpointPlanMode = ECFBuilderHardpointPlanMode::LegacyCompatible;
};

/** Gameplay Setup 영역 하나의 사용자 판단/조치 안내입니다. */
USTRUCT()
struct FCFBuilderGameplayGuidanceItem
{
	GENERATED_BODY()

	// 이 guidance가 설명하는 Gameplay 영역입니다.
	UPROPERTY()
	ECFBuilderGameplayArea Area = ECFBuilderGameplayArea::Durability;

	// 현재 authoritative truth에서 fresh derive한 completeness 상태입니다.
	UPROPERTY()
	ECFBuilderGuidanceState State = ECFBuilderGuidanceState::Blocked;

	// 사람이 현재 상태를 빠르게 이해할 한국어 요약입니다.
	UPROPERTY()
	FString Summary;

	// USER가 해야 할 일이 있을 때의 구체적인 해결 안내입니다.
	UPROPERTY()
	FString ResolutionText;

	// 관련 Recipe/VehicleData semantic field를 사람이 추적할 문자열입니다.
	UPROPERTY()
	FString RelatedFieldPath;

	// True이면 USER 판단 또는 Editor 작업 없이는 이 영역을 완료할 수 없습니다.
	UPROPERTY()
	bool bUserActionRequired = false;
};

/** USER authority인 Chassis Socket 하나에 대한 read-only Builder 안내입니다. */
USTRUCT()
struct FCFBuilderManualSocketGuidance
{
	GENERATED_BODY()

	// Hardpoint 또는 Destroyed FX 중 이 Socket이 속한 영역입니다.
	UPROPERTY()
	ECFBuilderGameplayArea Area = ECFBuilderGameplayArea::Hardpoints;

	// Top_01 같은 semantic slot identity입니다. Destroyed FX는 FX_Destroyed를 사용합니다.
	UPROPERTY()
	FName SemanticId = NAME_None;

	// 현재 Recipe가 binding했거나 Builder가 권장하는 Socket 이름입니다.
	UPROPERTY()
	FName SocketName = NAME_None;

	// 현재 Chassis StaticMesh에 이 Socket이 실제 존재하는지 여부입니다.
	UPROPERTY()
	bool bFoundOnChassis = false;

	// Existing Vehicle Completion에서 current VehicleData LocalTransform을 그대로 보존해도 되는지 여부입니다.
	UPROPERTY()
	bool bExistingStoredTransformAccepted = false;

	// Static Mesh Editor에서 USER가 직접 수행할 작업 안내입니다.
	UPROPERTY()
	FString Instruction;
};

/** Builder Gameplay Setup Step이 소비할 mutation0 completeness/read-only guidance 결과입니다. */
USTRUCT()
struct FCFBuilderGameplayGuidanceResult
{
	GENERATED_BODY()

	// Common R0 typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// 이 평가에 적용한 NewVehicle/CompleteExisting mode입니다.
	UPROPERTY()
	ECFBuilderCompanionMode Mode = ECFBuilderCompanionMode::NewVehicle;

	// 고정 영역 순서의 fresh completeness 결과입니다.
	UPROPERTY()
	TArray<FCFBuilderGameplayGuidanceItem> Items;

	// USER authority인 Hardpoint/Destroyed FX Socket의 현재 존재 상태와 수동 작업 안내입니다.
	UPROPERTY()
	TArray<FCFBuilderManualSocketGuidance> SocketGuidance;

	// USER review/action이 필요한 영역 수입니다.
	UPROPERTY()
	int32 NeedsReviewCount = 0;

	// current contract 위반으로 막힌 영역 수입니다.
	UPROPERTY()
	int32 BlockedCount = 0;

	// Step 6을 기술적으로 완료할 수 있는지 여부입니다. Optional은 blocker가 아닙니다.
	UPROPERTY()
	bool bCanCompleteGameplayStep = false;

	// current Target과 resolved authoring candidate 사이 Gameplay 관련 pending diff 수입니다.
	UPROPERTY()
	int32 PendingGameplayDiffCount = 0;

	// Existing Vehicle Completion에서 기존 값을 보존하는지, 후속 Apply review가 필요한지 설명합니다.
	UPROPERTY()
	FString ExistingCompletionSummary;
};

/** Builder proposal이 실제 current Reference Evidence에 binding할 typed stale-precondition입니다. */
USTRUCT()
struct FCFBuilderEvidenceBinding
{
	GENERATED_BODY()

	// Preview/Commit에서 다시 load할 exact Reference Evidence asset path입니다.
	UPROPERTY()
	FSoftObjectPath EvidencePath;

	// Preview가 사용한 exact persistent Evidence identity입니다.
	UPROPERTY()
	FGuid ExpectedEvidenceId;

	// Preview가 사용한 exact generated semantic Evidence fingerprint입니다.
	UPROPERTY()
	FString ExpectedEvidenceFingerprint;

	// 이번 complete Profile proposal이 실제 소비한 canonical Claim ID 목록입니다.
	UPROPERTY()
	TArray<FName> ConsumedClaimIds;
};

/** Preview/Commit이 공유하는 Builder-private 4 Profile atomic typed request입니다. */
USTRUCT()
struct FCFBuilderProfileCommitRequest
{
	GENERATED_BODY()

	// 4 private Profile의 owner authority인 persistent Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// RecipeId와 exact equality가 필요한 expected Builder owner identity입니다.
	UPROPERTY()
	FGuid ExpectedOwnerRecipeId;

	// 4 private Profile path + complete prospective payload입니다.
	UPROPERTY()
	FCFBuilderPrivateProfilePayload Payload;

	// Preview 당시 current 4 Profile fingerprints입니다.
	UPROPERTY()
	FCFBuilderProfileFingerprints ExpectedCurrentFingerprints;

	// Fresh Reference Evidence stale-precondition과 consumed canonical Claim set입니다.
	UPROPERTY()
	FCFBuilderEvidenceBinding EvidenceBinding;

	// Vehicle-specific Transmission 완료가 필요한 경우 field-level reviewed provenance입니다.
	UPROPERTY()
	FCFBuilderTransmissionReview TransmissionReview;

	// Vehicle-specific Engine Curve provenance/fidelity를 fresh Evidence에 binding할 reviewed metadata입니다.
	UPROPERTY()
	FCFBuilderEngineCurveReview EngineCurveReview;

	// 상위 Builder proposal과의 correlation/binding hash입니다.
	UPROPERTY()
	FString UpstreamBuilderProposalHash;

	// Preview/Commit 공용 caller + exact approval/current-state contract입니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};

/** Builder-private 4 Profile complete payload의 mutation0 preview입니다. */
USTRUCT()
struct FCFBuilderProfileCommitPreview
{
	GENERATED_BODY()

	// Common typed preview result입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Commit에 binding할 exact AuthoringWrite proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal Proposal;

	// Preview 당시 current 4 Profile fingerprints입니다.
	UPROPERTY()
	FCFBuilderProfileFingerprints CurrentFingerprints;

	// Complete typed payload를 materialize했을 prospective 4 Profile fingerprints입니다.
	UPROPERTY()
	FCFBuilderProfileFingerprints ProspectiveFingerprints;

	// Preview가 fresh 검증한 Reference Evidence fingerprint입니다.
	UPROPERTY()
	FString EvidenceFingerprint;

	// Prospective complete payload의 effective Resolver Source signature입니다.
	UPROPERTY()
	FString ProspectiveSourceSignature;

	// Prospective complete payload의 resolved Definition hash입니다.
	UPROPERTY()
	FString ProspectiveResolvedDefinitionHash;

	// Prospective Drivetrain payload + field-level review의 deterministic hash입니다.
	UPROPERTY()
	FString TransmissionProposalHash;

	// Prospective Performance Engine Curve payload + reviewed provenance의 deterministic hash입니다.
	UPROPERTY()
	FString EngineCurveProposalHash;

	// USER가 Step 5에서 함께 검토할 Engine Curve provenance/fidelity diagnostic입니다.
	UPROPERTY()
	FCFBuilderEngineCurveDiagnostic EngineCurveDiagnostic;

	// USER가 Step 5에서 함께 검토할 fixed-shift kinematic diagnostic입니다.
	UPROPERTY()
	FCFBuilderTransmissionDiagnostic TransmissionDiagnostic;
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

/** Final Review가 표시할 accepted Reference Evidence consumed Claim provenance 요약입니다. */
USTRUCT()
struct FCFBuilderProvenanceSummary
{
	GENERATED_BODY()

	// Persistent receipt + current Evidence/Profile binding이 fresh해서 provenance를 표시할 수 있는지 여부입니다.
	UPROPERTY()
	bool bAvailable = false;

	// 이번 accepted proposal이 소비한 canonical Claim 수입니다.
	UPROPERTY()
	int32 ConsumedClaimCount = 0;

	// Consumed canonical Claim 중 FACT 수입니다.
	UPROPERTY()
	int32 FactClaimCount = 0;

	// Consumed canonical Claim 중 DERIVED 수입니다.
	UPROPERTY()
	int32 DerivedClaimCount = 0;

	// Consumed canonical Claim 중 GAME_BIAS 수입니다.
	UPROPERTY()
	int32 GameBiasClaimCount = 0;

	// Provenance 판정에 사용한 fresh Evidence fingerprint입니다.
	UPROPERTY()
	FString EvidenceFingerprint;

	// Provenance를 사용할 수 없을 때의 blocker/diagnostic 설명입니다.
	UPROPERTY()
	FString IssueText;
};

/** Builder Final Review R0 aggregate request입니다. */
USTRUCT()
struct FCFBuilderFinalReviewRequest
{
	GENERATED_BODY()

	// Existing Vehicle Authoring read + Builder gameplay guidance context입니다.
	UPROPERTY()
	FCFBuilderGameplayGuidanceRequest GameplayRequest;

	// accepted Evidence binding을 제공했는지 여부입니다.
	UPROPERTY()
	bool bHasEvidenceBinding = false;

	// persistent receipt와 대조할 accepted Evidence/Claim binding입니다.
	UPROPERTY()
	FCFBuilderEvidenceBinding EvidenceBinding;
};

/** Builder Final Review Step의 validation/drift/gameplay/diff/provenance + Apply readiness 결과입니다. */
USTRUCT()
struct FCFBuilderFinalReviewResult
{
	GENERATED_BODY()

	// Common R0 typed result envelope입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// P0-06 gameplay completeness/read-only guidance projection입니다.
	UPROPERTY()
	FCFBuilderGameplayGuidanceResult GameplayGuidance;

	// Shared Resolver의 Recipe/Resolver/Definition/Stale validation projection입니다.
	UPROPERTY()
	FCFVehicleValidationReadResult Validation;

	// Shared external drift projection입니다.
	UPROPERTY()
	FCFVehicleDriftReadResult Drift;

	// current Target과 resolved candidate의 exact Shared Resolver field diff입니다.
	UPROPERTY()
	TArray<FCFVehicleFieldDiff> FieldDiff;

	// FieldDiff exact rows의 deterministic approval hash입니다.
	UPROPERTY()
	FString DiffHash;

	// accepted Evidence consumed canonical Claim provenance입니다.
	UPROPERTY()
	FCFBuilderProvenanceSummary Provenance;

	// Current Recipe policy/receipt/Drivetrain/fixed-shift kinematics를 fresh 재검사한 Transmission diagnostic입니다.
	UPROPERTY()
	FCFBuilderTransmissionDiagnostic TransmissionDiagnostic;

	// Current persistent receipt/Performance Profile/Reference Evidence를 fresh 재검사한 Engine Curve diagnostic입니다.
	UPROPERTY()
	FCFBuilderEngineCurveDiagnostic EngineCurveDiagnostic;

	// Final Review에서 표시할 warning 수입니다.
	UPROPERTY()
	int32 WarningCount = 0;

	// Apply/Complete를 막는 blocker 수입니다.
	UPROPERTY()
	int32 BlockingIssueCount = 0;

	// current AppliedState 기준 External Drift가 존재하는지 여부입니다.
	UPROPERTY()
	bool bHasExternalDrift = false;

	// current Target에 pending resolved diff가 있어 explicit Apply가 필요한지 여부입니다.
	UPROPERTY()
	bool bApplyRequired = false;

	// fresh review에서 explicit DefinitionApply proposal을 승인할 수 있는지 여부입니다.
	UPROPERTY()
	bool bCanApply = false;

	// pending Apply 없이 Final Review Step을 기술적으로 완료할 수 있는지 여부입니다.
	UPROPERTY()
	bool bCanCompleteFinalReview = false;

	// bCanApply=true일 때 exact DefinitionApply approval proposal입니다.
	UPROPERTY()
	FCFAuthoringProposal ApplyProposal;

	// ApplyBuilderFinalReview가 fresh 재검사 뒤 사용할 exact ApplyService request입니다.
	UPROPERTY()
	FCFVehicleApplyRequest PreparedApplyRequest;
};

/** Builder Final Review의 explicit Definition Apply request입니다. */
USTRUCT()
struct FCFBuilderFinalApplyRequest
{
	GENERATED_BODY()

	// Apply 직전 fresh 재검사할 exact Final Review input입니다.
	UPROPERTY()
	FCFBuilderFinalReviewRequest ReviewRequest;

	// exact DefinitionApply ProposalHash와 expected state를 담는 explicit approval입니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};

/** Builder가 성공한 Apply transaction 하나를 안전하게 되돌리기 위해 보관하는 transient token입니다. */
USTRUCT()
struct FCFBuilderUndoToken
{
	GENERATED_BODY()

	// Apply 성공 직후 Undo stack top에서 캡처한 exact UE transaction identity입니다.
	UPROPERTY()
	FGuid TransactionId;

	// transaction owner Recipe persistent identity입니다.
	UPROPERTY()
	FGuid RecipeId;

	// transaction owner Recipe object path입니다.
	UPROPERTY()
	FSoftObjectPath RecipePath;

	// transaction Target VehicleData object path입니다.
	UPROPERTY()
	FSoftObjectPath TargetDefinitionPath;

	// explicit Undo approval이 binding할 exact token hash입니다.
	UPROPERTY()
	FString UndoScopeHash;

	// Apply 전후 동일해야 하는 semantic Recipe fingerprint입니다. AppliedState/Builder receipt 같은 non-semantic metadata는 제외됩니다.
	UPROPERTY()
	FString ApplyRecipeFingerprint;

	// Undo 뒤 복원돼야 하는 pre-Apply full Target Definition hash입니다.
	UPROPERTY()
	FString PreApplyTargetDefinitionHash;

	// Undo 뒤 복원돼야 하는 pre-Apply AppliedState recipe fingerprint입니다.
	UPROPERTY()
	FString PreApplyAppliedRecipeFingerprint;

	// Undo 뒤 복원돼야 하는 pre-Apply AppliedState source signature입니다.
	UPROPERTY()
	FString PreApplyAppliedSourceSignature;

	// Undo 뒤 복원돼야 하는 pre-Apply AppliedState Definition hash입니다.
	UPROPERTY()
	FString PreApplyAppliedDefinitionHash;

	// Undo 뒤 복원돼야 하는 pre-Apply AppliedState Resolver revision입니다.
	UPROPERTY()
	int32 PreApplyAppliedResolverRevision = 0;

	// Undo 직전까지 exact 유지돼야 하는 post-Apply full Target Definition hash입니다.
	UPROPERTY()
	FString PostApplyTargetDefinitionHash;

	// Undo 직전까지 exact 유지돼야 하는 post-Apply AppliedState recipe fingerprint입니다.
	UPROPERTY()
	FString PostApplyAppliedRecipeFingerprint;

	// Undo 직전까지 exact 유지돼야 하는 post-Apply AppliedState source signature입니다.
	UPROPERTY()
	FString PostApplyAppliedSourceSignature;

	// Undo 직전까지 exact 유지돼야 하는 post-Apply AppliedState Definition hash입니다.
	UPROPERTY()
	FString PostApplyAppliedDefinitionHash;

	// Undo 직전까지 exact 유지돼야 하는 post-Apply AppliedState Resolver revision입니다.
	UPROPERTY()
	int32 PostApplyAppliedResolverRevision = 0;
};

/** Builder Final Review explicit Apply terminal result입니다. */
USTRUCT()
struct FCFBuilderFinalApplyResult
{
	GENERATED_BODY()

	// Existing ApplyResolvedVehicle result를 그대로 전달하는 common result입니다.
	UPROPERTY()
	FCFAuthoringOpResult Operation;

	// Apply가 실제 UE transaction을 만들었고 guarded Undo token을 발급했는지 여부입니다.
	UPROPERTY()
	bool bUndoAvailable = false;

	// bUndoAvailable=true일 때만 유효한 one-transaction Undo capability입니다.
	UPROPERTY()
	FCFBuilderUndoToken UndoToken;
};

/** Builder가 자신이 만든 마지막 exact Apply transaction만 되돌리는 request입니다. */
USTRUCT()
struct FCFBuilderUndoRequest
{
	GENERATED_BODY()

	// ApplyBuilderFinalReview가 발급한 exact transient Undo token입니다.
	UPROPERTY()
	FCFBuilderUndoToken UndoToken;

	// ClientOperationId + DefinitionApply approval class + UndoScopeHash를 요구하는 explicit Undo approval입니다.
	UPROPERTY()
	FCFAuthoringCallContext CallContext;
};
