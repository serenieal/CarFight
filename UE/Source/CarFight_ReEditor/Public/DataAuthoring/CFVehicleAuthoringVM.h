// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringVM.h
// Version: v1.7.0
// Date: 2026-08-21
// Description: DAUTH-P0-09~12 single-Vehicle Authoring Workspace transient ViewModel입니다.
// Scope: Selection, Profile binding/impact, Drift review/recovery, Mesh-only creation, typed Recipe intents, Apply/Undo transient UI state만 보관합니다.
// Changelog:
// - v1.7.0: P0-12 UA-06 USER UX remediation을 위해 bound Shared Profile의 Registry allowlisted numeric current value를 read-only로 조회하는 helper를 추가.
// - v1.6.0: P0-12 UA-07 baseline-safe Profile 검증 복구를 위해 explicit Recipe-only Profile unbind orchestration을 추가.
// - v1.5.0: P0-12 UA-06 readiness에서 Workspace Undo를 exact UE TransactionId에 binding해 중간 Editor transaction이 끼면 fail-closed하도록 보강.
// - v1.4.0: P0-12 UA-03 USER feedback에 따라 Frozen 5-domain Profile 목록/현재 binding/explicit Bind/Open orchestration을 normal Workspace에 추가.
// - v1.3.0: Frozen 24.90~24.94 Shared Profile B2, External Drift reviewed decision/Keep token, Mesh-only record creation orchestration 추가.
// - v1.2.0: P0-10 Assets/Layout, Driving Feel preset, Reference Compare, Adoption/Measurement, Mount/Defaults, standard Undo orchestration을 facade-only로 추가.
// - v1.1.0: reviewed Initial Import R2 proposal을 exact transient approval로 보존하고 unsaved loaded Recipe/Target selection을 지원.
// - v1.0.0: Section 24.85~24.87 P0-09 ViewModel foundation 최초 구현.
// Migration:
// - v1.7.0은 Registry descriptor와 bound Profile UObject를 read-only로 결합해 current numeric authored value만 조회하며 Recipe/Profile/VehicleData mutation contract를 변경하지 않습니다.

// - Persistent Authoring Source/Resolver truth/Legacy Pin/Override authority를 저장하지 않습니다.
// - 모든 Authoring read/write는 FCFVehicleAuthoringService facade를 통해 수행합니다.
// - Raw DA Open은 Editor navigation일 뿐 Authoring mutation이 아닙니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"

class UCFVehicleData;
class UCFVehicleRecipeData;

/** Section 24.5 Management 표시용 transient derived state입니다. */
enum class ECFWorkspaceManageView : uint8
{
	Unmanaged,
	LegacyImported,
	PartiallyManaged,
	Managed
};

/** Section 24.5 Sync 표시용 transient derived state입니다. */
enum class ECFWorkspaceSyncView : uint8
{
	NoBaseline,
	InSync,
	EffectiveStale,
	ShadowChanged,
	ExternalDrift,
	PreviewOutOfDate
};

/** Section 24.5 Validation 표시용 transient derived state입니다. */
enum class ECFWorkspaceValidView : uint8
{
	NotEvaluated,
	Valid,
	Warning,
	Blocked
};

/** Section 24.61 Preview freshness 표시 상태입니다. */
enum class ECFWorkspacePreviewView : uint8
{
	NeedsRefresh,
	Fresh,
	Blocked,
	OutOfDate
};

/** P0-09 Slate가 Core를 직접 호출하지 않도록 공통 facade 결과를 presentation state로 묶는 transient ViewModel입니다. */
class CARFIGHT_REEDITOR_API FCFVehicleAuthoringVM
{
public:
	// Asset Registry 기반 Vehicle Browser cache를 facade를 통해 재구성합니다.
	bool RefreshBrowser(const FString& SearchText, FString& OutError);

	// Browser row를 current single-Vehicle selection으로 전환하고 managed target이면 fresh preview를 구성합니다.
	bool SelectVehicle(const FCFVehicleListEntry& Entry, FString& OutError);

	// Current selection의 facade Resolve/Diff/Trace/Validation을 fresh result로 교체합니다.
	bool RefreshPreview(FString& OutError);

	// Persistent mutation 없이 typed VehicleArchetype prospective preview를 계산합니다.
	bool PreviewArchetypeIntent(const FName VehicleArchetypeId, FCFVehicleRecipePreviewResult& OutPreview);

		// Reviewed R1 proposal을 exact approval로 Recipe transaction에 commit하고 preview를 갱신합니다.
	bool CommitArchetypeIntent(const FName VehicleArchetypeId, FCFAuthoringOpResult& OutResult);

	// 임의 raw field가 아닌 typed semantic change를 persistent mutation 없이 prospective facade preview합니다.
	bool PreviewSemanticChange(const FCFVehicleSemanticChange& Change, FCFVehicleRecipePreviewResult& OutPreview);

	// Typed semantic change를 exact R1 preview/approval 뒤 Recipe-only commit하고 fresh preview를 다시 읽습니다.
	bool CommitSemanticChange(const FCFVehicleSemanticChange& Change, FCFAuthoringOpResult& OutResult);

	// Assets & Layout의 typed Asset/Socket Intent를 Recipe-only로 commit합니다.
	bool CommitAssetIntent(const FCFVehicleAssetIntent& AssetIntent, FCFAuthoringOpResult& OutResult);

	// Driving Feel partial 4축 semantic patch를 Recipe-only로 commit합니다.
	bool CommitDrivingFeel(const FCFDrivingFeelPatch& Patch, FCFAuthoringOpResult& OutResult);

	// Frozen migration-parity named preset의 exact 4축 semantic 값을 한 Recipe transaction으로 commit합니다.
	bool CommitDrivingFeelPreset(const FName PresetId, FCFAuthoringOpResult& OutResult);

	// Mounts & Defaults의 typed DefaultData intent를 Recipe-only로 commit합니다.
	bool CommitDefaultDataIntent(const FCFVehicleDefaultIntent& DefaultIntent, FCFAuthoringOpResult& OutResult);

	// Stable-ID Hardpoint intent를 typed Recipe write로 upsert합니다.
	bool UpsertHardpointIntent(const FCFHardpointIntent& HardpointIntent, FCFAuthoringOpResult& OutResult);

	// Stable-ID Mount intent를 typed Recipe write로 upsert합니다.
	bool UpsertMountIntent(const FCFMountIntent& MountIntent, FCFAuthoringOpResult& OutResult);

	// Browser row를 read-only Reference Vehicle로 선택하고 current/reference compare를 갱신합니다.
	bool SelectReferenceVehicle(const FCFVehicleListEntry& Entry, FString& OutError);

	// Current Resolved Preview와 selected Reference를 facade 117-field projection으로 비교합니다.
	bool RefreshReferenceCompare(const bool bChangedOnly, FString& OutError);

	// Current Resolver R6 Wheel measurement proposals를 facade에서 갱신합니다.
	bool RefreshMeasurementProposals(FString& OutError);

	// 선택 measurement decision을 mutation0 prospective R2 proposal로 준비합니다.
	bool PrepareMeasurementDecision(
		const FCFVehicleMeasurementProposal& Proposal,
		ECFVehicleMeasureDecision Decision,
		FCFVehicleMeasurementPreviewResult& OutPreview);

	// UI가 실제 검토한 prepared measurement R2 proposal만 Recipe-only commit합니다.
	bool ExecutePreparedMeasurement(FCFAuthoringOpResult& OutResult);

	// Legacy Pin group Adoption을 mutation0 prospective R2 proposal로 준비합니다.
	bool PrepareGroupAdoption(ECFVehicleAdoptGroup AdoptionGroup, FCFVehicleAdoptionPreviewResult& OutPreview);

		// UI가 실제 검토한 prepared Adoption R2 proposal만 existing Import Core를 통해 Recipe-only commit합니다.
	bool ExecutePreparedAdoption(FCFAuthoringOpResult& OutResult);

	// Current Recipe에 bound된 Shared Profile의 allowlisted numeric edit를 B2 Core로 mutation0 preview합니다.
	bool PrepareProfileNumericEdit(
		ECFVehicleProfileDomain ProfileDomain,
		const FString& ColumnId,
		const FString& CanonicalNumericValue,
		FCFProfileNumericEditPreview& OutPreview);

		// Project Asset Registry의 Frozen Profile 후보를 selected Domain 기준으로 read-only 갱신합니다.
	bool RefreshProfileChoices(ECFVehicleProfileDomain ProfileDomain, FString& OutError);

		// Current Recipe의 exact selected Domain Profile binding path를 반환합니다.
	FSoftObjectPath GetBoundProfilePath(ECFVehicleProfileDomain ProfileDomain) const;

	// Current bound Shared Profile의 Registry allowlisted numeric field current authored 값을 read-only로 반환합니다.
	bool ReadBoundProfileNumericValue(
		ECFVehicleProfileDomain ProfileDomain,
		const FString& ColumnId,
		FString& OutCanonicalValue,
		FString& OutError) const;

		// Existing Profile asset을 reviewed R1 Recipe semantic binding으로 연결하고 Target은 변경하지 않습니다.
	bool CommitProfileBinding(ECFVehicleProfileDomain ProfileDomain, const FSoftObjectPath& ProfilePath, FCFAuthoringOpResult& OutResult);

	// Current selected Domain의 Profile binding을 reviewed R1 Recipe-only semantic write로 명시적으로 해제합니다.
	bool CommitProfileUnbinding(ECFVehicleProfileDomain ProfileDomain, FCFAuthoringOpResult& OutResult);

	// Current Recipe에 bound된 selected Domain Profile을 Unreal 표준 Asset Editor로 엽니다.
	bool OpenBoundProfile(ECFVehicleProfileDomain ProfileDomain, FString& OutError) const;

	// UI가 실제 review한 exact B2 Profile preview/approval만 existing source commit lane으로 실행합니다.
	bool ExecutePreparedProfileEdit(FCFProfileNumericEditResult& OutResult);


	// Current External Drift를 exact Last Applied / Current Raw / Current Authoring rows로 read-only refresh합니다.
	bool RefreshDriftReview(FString& OutError);

	// Selected field subset 또는 전체 Drift field group의 recovery decision을 mutation0 R2 proposal로 준비합니다.
	bool PrepareDriftDecision(
		const TArray<FCFVehicleFieldPath>& FieldPaths,
		ECFVehicleDriftDecision Decision,
		const FString& OverrideReason,
		FCFVehicleDriftDecisionPreview& OutPreview);

	// UI가 실제 review한 exact Drift R2 decision을 commit하고 Keep이면 current evidence token을 보존합니다.
	bool ExecutePreparedDriftDecision(FCFAuthoringOpResult& OutResult);

	// Mesh-only Candidate 또는 explicit New Vehicle의 two-record R2 proposal을 준비합니다.
	bool PrepareVehicleRecordCreate(
		const FCFVehicleRecordCreateRequest& Request,
		FCFVehicleRecordCreatePreview& OutPreview);

	// UI가 실제 review한 exact Definition+Recipe creation proposal만 한 transaction으로 실행하고 새 Vehicle을 선택합니다.
	bool ExecutePreparedVehicleCreate(FCFVehicleRecordCreateResult& OutResult);

	// Profile impact row의 Target Vehicle을 current Browser selection으로 이동합니다.
	bool NavigateToAffectedVehicle(int32 ImpactIndex, FString& OutError);


	// Unmanaged selection을 위한 exact Initial Import R2 proposal을 facade에서 만듭니다.
	bool BuildInitialImportPreview(
		const FString& RecipePackagePath,
		const FName RecipeAssetName,
		FCFVehicleInitialImportPreviewResult& OutPreview);

	// Reviewed Initial Import proposal을 OwnershipWrite로 commit하고 생성된 Recipe를 current selection에 연결합니다.
	bool CommitInitialImport(
		const FString& RecipePackagePath,
		const FName RecipeAssetName,
		FCFVehicleInitialImportResult& OutResult);

	// Current fresh ResolveResult에서 exact R3 ApplyRequest/approval scope를 준비합니다.
	bool PrepareApply(FCFAuthoringOpResult& OutResult);

	// 준비된 exact R3 approval을 facade shared Apply lane으로 한 번 실행하고 current state를 refresh합니다.
	bool ExecutePreparedApply(FCFAuthoringOpResult& OutResult);

		// Current Target VehicleData를 Unreal 표준 Asset Editor에 여는 navigation action입니다.
	bool OpenRawVehicleData(FString& OutError) const;

	// 마지막으로 이 Workspace가 성공시킨 Recipe/Apply transaction을 Unreal 표준 Undo로 되돌리고 fresh preview를 읽습니다.
	bool UndoLastWorkspaceAction(FString& OutError);


	// Current transient selection/cache/approval을 모두 지웁니다.
	void ClearSelection();

	// Current Browser view rows입니다.
	const TArray<FCFVehicleListEntry>& GetBrowserEntries() const { return BrowserEntries; }

	// Current selection row입니다.
	const FCFVehicleListEntry& GetSelectedEntry() const { return SelectedEntry; }

	// Selection이 존재하는지 반환합니다.
	bool HasSelection() const { return bHasSelection; }

		// Current selection이 managed Recipe를 갖는지 반환합니다.
	bool HasRecipe() const { return Recipe.IsValid(); }

	// Current selection이 Definition이 아닌 Mesh-only Candidate인지 반환합니다.
	bool IsMeshOnlyCandidate() const { return bHasSelection && SelectedEntry.bMeshOnlyCandidate; }

	// Current Target UObject입니다.
	UCFVehicleData* GetTargetVehicleData() const { return TargetVehicleData.Get(); }

	// Current Recipe UObject입니다.
	UCFVehicleRecipeData* GetRecipe() const { return Recipe.Get(); }

	// Fresh current context read result입니다.
	const FCFVehicleContextReadResult& GetContextResult() const { return ContextResult; }

	// Fresh current Resolver read result입니다.
	const FCFVehicleResolveReadResult& GetResolveResult() const { return ResolveResult; }

	// Fresh current pending Diff read result입니다.
	const FCFVehicleDiffReadResult& GetDiffResult() const { return DiffResult; }

	// Fresh current Source Trace read result입니다.
	const FCFVehicleTraceReadResult& GetTraceResult() const { return TraceResult; }

		// Fresh current Validation read result입니다.
	const FCFVehicleValidationReadResult& GetValidationResult() const { return ValidationResult; }

	// Current read-only Reference Vehicle selection입니다.
	const FCFVehicleListEntry& GetReferenceEntry() const { return ReferenceEntry; }

	// Reference Vehicle이 선택돼 있는지 반환합니다.
	bool HasReferenceVehicle() const { return bHasReferenceVehicle; }

	// Fresh Reference Compare 결과입니다.
	const FCFVehicleReferenceCompareResult& GetReferenceCompareResult() const { return ReferenceCompareResult; }

		// Fresh Wheel measurement proposal read 결과입니다.
	const FCFVehicleMeasurementReadResult& GetMeasurementResult() const { return MeasurementResult; }

		// Selected Domain에 대해 마지막 read-only refresh로 얻은 existing Shared Profile 후보입니다.
	const TArray<FCFProfileListEntry>& GetProfileChoices() const { return ProfileChoices; }

	// 마지막 Shared Profile preview가 계산한 affected Vehicle navigation rows입니다.
	const FCFProfileNumericEditPreview& GetProfileImpactPreview() const { return LastProfileImpactPreview; }


	// Fresh current External Drift 3-way review rows입니다.
	const FCFVehicleDriftReviewResult& GetDriftReview() const { return DriftReview; }

	// Current External Drift에 exact Keep Authoring review token이 살아있는지 반환합니다.
	bool HasAcceptedDriftKeep() const;


	// Derived management UI state입니다.
	ECFWorkspaceManageView GetManagementView() const { return ManagementView; }

	// Derived sync UI state입니다.
	ECFWorkspaceSyncView GetSyncView() const { return SyncView; }

	// Derived validation UI state입니다.
	ECFWorkspaceValidView GetValidationView() const { return ValidationView; }

	// Current preview freshness UI state입니다.
	ECFWorkspacePreviewView GetPreviewView() const { return PreviewView; }

	// Current preview가 Apply authority로 fresh한지 반환합니다.
	bool IsPreviewFresh() const { return PreviewView == ECFWorkspacePreviewView::Fresh; }

	// Current Fresh Diff row 수입니다.
	int32 GetPendingDiffCount() const { return IsPreviewFresh() ? DiffResult.FieldDiff.Num() : 0; }

	// Current validation warning 수입니다.
	int32 GetWarningCount() const { return ValidationResult.Operation.ValidationSummary.WarningCount; }

	// Current validation Error+Blocked 수입니다.
	int32 GetBlockingIssueCount() const
	{
		return ValidationResult.Operation.ValidationSummary.ErrorCount + ValidationResult.Operation.ValidationSummary.BlockedCount;
	}

	// Current Resolve에서 External Drift가 있는지 반환합니다.
	bool HasExternalDrift() const
	{
		return IsPreviewFresh() && ResolveResult.ResolveResult.StaleReport.bHasExternalDrift;
	}

	// Bottom Action Bar의 normal Apply enable 조건을 반환합니다.
	bool CanApply() const;

		// Current prepared R3 approval이 존재하는지 반환합니다.
	bool HasPreparedApply() const { return bHasPreparedApply; }

		// Last Workspace transaction이 현재 UE Undo stack의 exact top인지 확인해 standard Undo 가능 여부를 반환합니다.
	bool CanUndoLastWorkspaceAction() const;


	// 마지막 facade operation의 persistent UI diagnostic입니다.
	const FString& GetLastMessage() const { return LastMessage; }

private:
	// Current managed Recipe/Target에서 facade read request를 만듭니다.
	FCFVehicleAuthoringReadRequest BuildReadRequest() const;

	// Current ResolveResult를 기존 shared ApplyRequest 구조로 포장합니다.
	bool BuildCurrentApplyRequest(FCFVehicleApplyRequest& OutApplyRequest, FString& OutError) const;

	// Resolver/ImportState 결과에서 Management/Sync/Validation UI 상태만 파생합니다.
	void RefreshDerivedViews();

			// Recipe/Target persistent state가 달라질 수 있는 action 뒤 stale prepared approval을 폐기합니다.
	void InvalidatePreparedApply();

	// Refresh/selection/source mutation 뒤 transient External Drift prepared/Keep evidence를 폐기합니다.
	void InvalidateDriftReviewState();

	// Current Recipe의 exact bound Profile UObject를 Frozen domain으로 resolve합니다.
	UObject* ResolveBoundProfile(ECFVehicleProfileDomain ProfileDomain) const;

	// 성공한 Workspace-owned transaction 뒤 standard Undo action을 transient하게 활성화합니다.
	void MarkWorkspaceTransaction(const FString& ActionDescription);


	// Browser facade cache입니다.
	TArray<FCFVehicleListEntry> BrowserEntries;

	// Current selected Browser row입니다.
	FCFVehicleListEntry SelectedEntry;

	// Current Runtime canonical Target입니다.
	TWeakObjectPtr<UCFVehicleData> TargetVehicleData;

	// Current Editor-only Recipe입니다. Unmanaged target에서는 null입니다.
	TWeakObjectPtr<UCFVehicleRecipeData> Recipe;

	// Selection 존재 여부입니다.
	bool bHasSelection = false;

	// Current facade Vehicle Context입니다.
	FCFVehicleContextReadResult ContextResult;

	// Current facade Resolver Preview입니다.
	FCFVehicleResolveReadResult ResolveResult;

	// Current facade Pending Diff입니다.
	FCFVehicleDiffReadResult DiffResult;

	// Current facade Source Trace입니다.
	FCFVehicleTraceReadResult TraceResult;

		// Current facade Validation result입니다.
	FCFVehicleValidationReadResult ValidationResult;

	// Read-only Reference Vehicle selection row입니다.
	FCFVehicleListEntry ReferenceEntry;

	// Reference selection 존재 여부입니다.
	bool bHasReferenceVehicle = false;

	// Current facade Reference Compare result입니다.
	FCFVehicleReferenceCompareResult ReferenceCompareResult;

	// Current facade Wheel measurement proposals입니다.
	FCFVehicleMeasurementReadResult MeasurementResult;

	// UI가 review한 exact Measurement R2 request입니다.
	FCFVehicleMeasurementRequest PreparedMeasurementRequest;

	// UI가 review한 exact Measurement R2 proposal입니다.
	FCFAuthoringProposal PreparedMeasurementProposal;

	// Prepared Measurement 존재 여부입니다.
	bool bHasPreparedMeasurement = false;

	// UI가 review한 exact Adoption R2 request입니다.
	FCFVehicleAdoptionRequest PreparedAdoptionRequest;

	// UI가 review한 exact Adoption R2 proposal입니다.
	FCFAuthoringProposal PreparedAdoptionProposal;

		// Prepared Adoption 존재 여부입니다.
	bool bHasPreparedAdoption = false;

		// Selected Domain에 대해 facade ListProfiles가 반환한 read-only Shared Profile 후보 cache입니다.
	TArray<FCFProfileListEntry> ProfileChoices;

	// UI가 review할 current exact Shared Profile edit request입니다.
	FCFProfileNumericEditRequest PreparedProfileEditRequest;


	// UI가 review한 existing B2 Session/Approval + impact result입니다.
	FCFProfileNumericEditPreview PreparedProfileEditPreview;

	// Prepared Shared Profile edit 존재 여부입니다.
	bool bHasPreparedProfileEdit = false;

	// Commit 뒤에도 navigation을 유지할 마지막 Profile impact snapshot입니다.
	FCFProfileNumericEditPreview LastProfileImpactPreview;

	// Current fresh External Drift 3-way review입니다.
	FCFVehicleDriftReviewResult DriftReview;

	// UI가 review할 exact Drift decision request입니다.
	FCFVehicleDriftDecisionRequest PreparedDriftDecisionRequest;

	// UI가 review한 exact Drift proposal/prospective result입니다.
	FCFVehicleDriftDecisionPreview PreparedDriftDecisionPreview;

	// Prepared Drift decision 존재 여부입니다.
	bool bHasPreparedDriftDecision = false;

	// Keep Authoring이 review된 current Recipe fingerprint입니다.
	FString AcceptedDriftKeepRecipeFingerprint;

	// Keep Authoring이 review된 current full Target hash입니다.
	FString AcceptedDriftKeepTargetHash;

	// Keep Authoring이 review된 current effective Source signature입니다.
	FString AcceptedDriftKeepSourceSignature;

	// Keep Authoring이 review된 current Resolver revision입니다.
	int32 AcceptedDriftKeepResolverRevision = 0;

	// UI가 review할 exact two-record creation request입니다.
	FCFVehicleRecordCreateRequest PreparedVehicleCreateRequest;

	// UI가 review한 exact R2 two-record proposal입니다.
	FCFVehicleRecordCreatePreview PreparedVehicleCreatePreview;

	// Prepared two-record creation 존재 여부입니다.
	bool bHasPreparedVehicleCreate = false;


		// Reviewed Initial Import에서 UI가 실제 확인한 exact request입니다.
	FCFVehicleInitialImportRequest PreparedInitialImportRequest;

	// Reviewed Initial Import에서 UI가 실제 확인한 exact R2 proposal입니다.
	FCFAuthoringProposal PreparedInitialImportProposal;

	// Reviewed Initial Import approval 존재 여부입니다.
	bool bHasPreparedInitialImport = false;

	// Prepared Apply exact request입니다.
	FCFVehicleApplyOpRequest PreparedApplyRequest;


	// Prepared Apply exact R3 approval proposal입니다.
	FCFAuthoringProposal PreparedApplyProposal;

	// Prepared Apply 존재 여부입니다.
	bool bHasPreparedApply = false;

	// Management presentation state입니다.
	ECFWorkspaceManageView ManagementView = ECFWorkspaceManageView::Unmanaged;

	// Sync presentation state입니다.
	ECFWorkspaceSyncView SyncView = ECFWorkspaceSyncView::NoBaseline;

	// Validation presentation state입니다.
	ECFWorkspaceValidView ValidationView = ECFWorkspaceValidView::NotEvaluated;

	// Preview freshness presentation state입니다.
	ECFWorkspacePreviewView PreviewView = ECFWorkspacePreviewView::NeedsRefresh;

			// 마지막 Workspace-owned transaction을 standard Undo 후보로 추적하는 transient flag입니다.
	bool bCanUndoLastWorkspaceAction = false;

	// Workspace transaction 완료 직후 UE Undo stack top에서 캡처한 exact TransactionId입니다.
	FGuid LastWorkspaceTransactionId;

	// Undo button에 표시할 마지막 Workspace-owned logical action 설명입니다.
	FString LastWorkspaceActionDescription;

	// 중요한 operation result를 toast 없이 persistent하게 보여줄 message입니다.
	FString LastMessage;
};
