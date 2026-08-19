// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchImport.h
// Version: v1.1.0
// Date: 2026-08-17
// Description: DAUTH-P0-08K~L Batch Import Session / 3-way Preview / B1·B2 Authoring Source Commit public contract입니다.
// Scope: canonical CSV/manifest parse, 3-way prospective preview, exact approval binding과 atomic Recipe/Profile numeric source commit을 제공합니다.
// Changelog:
// - v1.1.0: Section 26.44~26.53 B1/B2 approval binding, global TOCTOU preflight, all-or-nothing source commit contract를 추가.
// - v1.0.0: Section 26.31~26.43 transient Batch Import Session Foundation 최초 구현.
// Migration:
// - B1/B2는 reviewed Authoring Source만 변경하며 B3 Definition Apply, UI, disk file save를 제공하지 않습니다.
// - successful B1/B2 뒤 old Preview/Approval은 source fingerprint mismatch로 재사용할 수 없고 fresh read/resolve가 필요합니다.
// - CSV/manifest는 Unreal Source Truth가 아니며 current Recipe/Profile/Target을 import preview 때 다시 읽습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFBatchTypes.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"

/** Section 26.35의 editable cell 3-way merge 결과입니다. */
enum class ECFBatchCellMergeState : uint8
{
	Unchanged,
	SafeCandidate,
	NoSpreadsheetChange,
	ConvergedNoChange,
	ConcurrentEditConflict,
	OwnershipChangedSinceExport,
	BlockedInvalid
};

/** Section 26.39의 최소 Batch row status입니다. */
enum class ECFBatchRowStatus : uint8
{
	Unchanged,
	Candidate,
	ShadowOnly,
	Warning,
	Blocked,
	Conflict,
	ExternalDrift
};

/** Batch parse/3-way/prospective preview가 구조적으로 반환하는 issue 수준입니다. */
enum class ECFBatchIssueSeverity : uint8
{
	Info,
	Warning,
	Blocked,
	Conflict
};

/** 문자열 메시지 parsing 없이 Batch issue를 분류하는 stable P0 code입니다. */
enum class ECFBatchIssueCode : uint8
{
	None,
	ManifestParseError,
	ManifestInvalid,
	UnsupportedDataset,
	CsvParseError,
	CsvManifestColumnMismatch,
	MissingRowIdentity,
	UnknownRowIdentity,
	MissingExportedRow,
	DuplicateRowIdentity,
	ReadOnlyColumnModified,
	CellNotEditableAtExport,
	InvalidNumericValue,
	OwnershipChangedSinceExport,
	ConcurrentEditConflict,
	CurrentObjectMissing,
	CurrentTypeMismatch,
	ProspectivePatchFailed,
	ProspectiveResolveFailed,
	ValidationBlocked,
	NoAffectedRecipe,
	UnexpectedArrayStructuralChange
};

/** Canonical CSV parser가 보존하는 stable ColumnId + raw text cell입니다. */
struct FCFBatchParsedCell
{
	// Header에서 resolve한 stable technical ColumnId입니다.
	FString ColumnId;

	// CSV escaping을 해제한 원본 cell text입니다.
	FString RawValue;
};

/** Row order와 무관하게 identity로 다시 정렬되는 parsed CSV row입니다. */
struct FCFBatchParsedRow
{
	// Parsed reserved metadata에서 읽은 row identity입니다.
	FString RowId;

	// RecipePath 또는 ProfilePath인 authoring source identity입니다.
	FString SourceIdentity;

	// Header mapping을 적용한 parsed cells입니다.
	TArray<FCFBatchParsedCell> Cells;
};

/** Export/Edited/Current 세 값을 보존하는 한 editable cell의 3-way review입니다. */
struct FCFBatchCellReview
{
	// Stable technical ColumnId입니다.
	FString ColumnId;

	// Export manifest가 보존한 canonical baseline value입니다.
	FString ExportBaselineValue;

	// Spreadsheet에서 읽어 property type에 맞게 normalize한 canonical value입니다.
	FString EditedCanonicalValue;

	// Import 시 다시 읽은 current Unreal authoring canonical value입니다.
	FString CurrentUnrealValue;

	// Export 시점 ownership/source mode입니다.
	FString ExportOwnershipSourceMode;

	// Import 시점 current ownership/source mode입니다.
	FString CurrentOwnershipSourceMode;

	// Export 시점 cell editability입니다.
	bool bEditableAtExport = false;

	// Import 시점 current cell editability입니다.
	bool bEditableNow = false;

	// Blank NoChange를 포함해 Spreadsheet가 semantic edit를 실제 요청했는지 여부입니다.
	bool bSpreadsheetChanged = false;

	// Section 26.35/26.36의 3-way terminal classification입니다.
	ECFBatchCellMergeState MergeState = ECFBatchCellMergeState::Unchanged;
};

/** Batch row 또는 file-level 문제를 구조적으로 보존합니다. */
struct FCFBatchIssue
{
	// Stable issue taxonomy입니다.
	ECFBatchIssueCode Code = ECFBatchIssueCode::None;

	// Info/Warning/Blocked/Conflict 수준입니다.
	ECFBatchIssueSeverity Severity = ECFBatchIssueSeverity::Info;

	// Issue가 특정 row에 귀속될 때의 stable RowId입니다.
	FString RowId;

	// Issue가 특정 column에 귀속될 때의 stable ColumnId입니다.
	FString ColumnId;

	// 사람이 읽는 한국어 diagnostic입니다.
	FString Message;
};

/** Prospective Recipe/Profile payload가 한 Vehicle에 주는 기존 Resolver 결과 요약입니다. */
struct FCFBatchVehiclePreview
{
	// Affected persistent Recipe identity입니다.
	FString RecipePath;

	// Affected Runtime canonical Target Definition identity입니다.
	FString TargetPath;

	// Prospective 계산 전 current effective resolved hash입니다.
	FString CurrentResolvedDefinitionHash;

	// Candidate patch를 반영한 prospective resolved hash입니다.
	FString ProspectiveResolvedDefinitionHash;

	// Prospective effective source stack aggregate signature입니다.
	FString ProspectiveSourceSignature;

	// Prospective Target-vs-resolved Definition leaf/array Diff row 수입니다.
	int32 DefinitionDiffCount = 0;

	// Add/Remove/Move array operation 수이며 P0 numeric edit에서는 정상값 0입니다.
	int32 ArrayStructuralChangeCount = 0;

	// Prospective Recipe validation Info 수입니다.
	int32 ValidationInfoCount = 0;

	// Prospective Validation Warning 수입니다.
	int32 ValidationWarningCount = 0;

	// Prospective Validation Blocked 수입니다.
	int32 ValidationBlockedCount = 0;

	// Prospective Validation Error 수입니다.
	int32 ValidationErrorCount = 0;

	// R16이 current Target external drift를 보고하는지 여부입니다.
	bool bExternalDrift = false;

	// 기존 Pure Resolver의 prospective terminal status입니다.
	ECFVehicleResolveStatus ResolveStatus = ECFVehicleResolveStatus::Error;
};

/** Section 26.43 한 CSV row의 full review foundation입니다. */
struct FCFBatchRowPreview
{
	// Stable row identity입니다.
	FString RowId;

	// RecipeNumericEdit일 때 Target identity입니다.
	FString TargetPath;

	// RecipeNumericEdit일 때 Recipe identity입니다.
	FString RecipePath;

	// ProfileNumericEdit일 때 Profile identity입니다.
	FString ProfilePath;

	// ProfileNumericEdit exact Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// Manifest export baseline Recipe/Profile fingerprint입니다.
	FString ExportObjectFingerprint;

	// Import preview 때 current Unreal에서 다시 읽은 Recipe/Profile fingerprint입니다.
	FString CurrentObjectFingerprint;

	// Fingerprint mismatch는 fast signal이며 단독 Block 근거가 아닙니다.
	bool bFingerprintChangedSinceExport = false;

	// Recipe dataset에서 export 이후 Target Definition hash가 달라졌는지 여부입니다.
	bool bTargetHashChangedSinceExport = false;

	// Spreadsheet가 실제 변경 요청한 editable cell 수입니다.
	int32 SpreadsheetChangedCellCount = 0;

	// 3-way SafeCandidate가 된 editable cell 수입니다.
	int32 CandidateCellCount = 0;

	// Edited==Current로 수렴해 mutation 후보가 아닌 cell 수입니다.
	int32 ConvergedCellCount = 0;

	// Row의 canonical ColumnId 순서 cell review입니다.
	TArray<FCFBatchCellReview> CellReviews;

	// Recipe row는 최대 1개, Profile row는 shared affected Vehicle N개의 prospective preview입니다.
	TArray<FCFBatchVehiclePreview> VehiclePreviews;

	// 하나의 row가 여러 문제를 가질 수 있으므로 별도 issue list를 보존합니다.
	TArray<FCFBatchIssue> Issues;

	// Section 26.39 aggregate row status입니다.
	ECFBatchRowStatus Status = ECFBatchRowStatus::Unchanged;
};

/** Section 26.42 Batch Preview Summary의 P0 machine-readable 집계입니다. */
struct FCFBatchPreviewSummary
{
	// Parsed/validated Dataset Kind입니다.
	ECFBatchDatasetKind DatasetKind = ECFBatchDatasetKind::VehicleSummaryReport;

	// Spreadsheet가 하나 이상 editable cell을 바꾼 row 수입니다.
	int32 EditedRowCount = 0;

	// Candidate row 수입니다.
	int32 CandidateRowCount = 0;

	// Unchanged row 수입니다.
	int32 UnchangedRowCount = 0;

	// ShadowOnly row 수입니다.
	int32 ShadowOnlyRowCount = 0;

	// Warning row 수입니다.
	int32 WarningRowCount = 0;

	// Blocked row 수입니다.
	int32 BlockedRowCount = 0;

	// Conflict row 수입니다.
	int32 ConflictRowCount = 0;

	// ExternalDrift row 수입니다.
	int32 ExternalDriftRowCount = 0;

	// Preview에 포함된 distinct affected Recipe 수입니다.
	int32 AffectedRecipeCount = 0;

	// Preview에 포함된 distinct affected Profile 수입니다.
	int32 AffectedProfileCount = 0;

	// Preview에 포함된 distinct affected Vehicle Target 수입니다.
	int32 AffectedVehicleCount = 0;

	// Prospective Target-vs-resolved Definition changed field row 총수입니다.
	int32 ProspectiveDefinitionChangedFieldCount = 0;

	// Numeric batch에서 정상 0인 prospective array structural operation 총수입니다.
	int32 ArrayStructuralChangeCount = 0;

	// Affected Vehicle preview 중 R16 external drift를 가진 수입니다.
	int32 ExternalDriftCount = 0;

	// Prospective validation Warning 총수입니다.
	int32 ValidationWarningCount = 0;

	// Prospective validation Blocked 총수입니다.
	int32 ValidationBlockedCount = 0;

	// Prospective validation Error 총수입니다.
	int32 ValidationErrorCount = 0;
};

/** CSV text + companion manifest text로 transient preview Session을 만드는 request입니다. */
struct FCFBatchImportRequest
{
	// Disk I/O 없이 caller가 전달하는 canonical CSV text입니다.
	FString CsvText;

	// Disk I/O 없이 caller가 전달하는 companion .cfbatch.json text입니다.
	FString ManifestJsonText;
};

/** Section 26.38의 restart-persistent가 아닌 transient Batch Import Session value model입니다. */
struct FCFBatchImportSession
{
	// Manifest가 선언한 exact Dataset Kind입니다.
	ECFBatchDatasetKind DatasetKind = ECFBatchDatasetKind::VehicleSummaryReport;

	// ProfileNumericEdit일 때 exact one Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// 검증된 immutable export baseline evidence입니다.
	FCFBatchManifest Manifest;

	// CSV row order와 무관하게 RowId 기준 canonical 정렬된 parsed rows입니다.
	TArray<FCFBatchParsedRow> ParsedRows;

	// RowId 기준 canonical 정렬된 3-way/prospective review rows입니다.
	TArray<FCFBatchRowPreview> Rows;

	// File-level schema/duplicate/missing-row issue입니다.
	TArray<FCFBatchIssue> Issues;

	// Section 26.42 aggregate preview summary입니다.
	FCFBatchPreviewSummary Summary;

	// Current Unreal truth + canonical edited intent + prospective outputs를 binding한 deterministic plan hash입니다.
	FString BatchPlanHash;

		// File-level issue 때문에 commit 가능한 Plan으로 사용할 수 없는지 여부입니다.
	bool bFileBlocked = false;
};

/** B1/B2 Authoring Source Commit의 terminal 상태입니다. */
enum class ECFBatchCommitStatus : uint8
{
	NotRun,
	Succeeded,
	NoChange,
	Blocked,
	FailedRolledBack,
	FailedUnknownState
};

/** 문자열 메시지 parsing 없이 B1/B2 commit 실패 원인을 분류하는 stable code입니다. */
enum class ECFBatchCommitErrorCode : uint8
{
	None,
	InvalidRequest,
	ApprovalRequired,
	UnsupportedDataset,
	SessionBlocked,
	ApprovalMismatch,
	IncludedRowSetMismatch,
	ProposedPatchMismatch,
	SourceObjectMissing,
	SourceTypeMismatch,
	SourceFingerprintMismatch,
	AffectedRecipeInventoryChanged,
	ProspectiveValidationBlocked,
	PatchApplyFailed,
	PostCheckFailed,
	RollbackFailed,
	AutomationInjectedFailure
};

/** Section 26.51 approval이 한 included row에 binding하는 exact source/patch evidence입니다. */
struct FCFBatchCommitApprovalRow
{
	// Stable Batch RowId입니다.
	FString RowId;

	// SafeCandidate ColumnId/typed value set만 canonicalize한 proposed patch hash입니다.
	FString ProposedPatchHash;

	// Approval 시점 current Recipe/Profile fingerprint입니다.
	FString CurrentFingerprint;
};

/** Section 26.51의 B1/B2 exact Batch Authoring Commit approval evidence입니다. */
struct FCFBatchCommitApproval
{
	// Approval 대상 exact Dataset Kind입니다.
	ECFBatchDatasetKind DatasetKind = ECFBatchDatasetKind::VehicleSummaryReport;

	// ProfileNumericEdit일 때 exact one Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// Export schema technical identity입니다.
	FString SchemaId;

	// Export schema revision입니다.
	int32 SchemaRevision = 0;

	// Immutable export set identity입니다.
	FGuid BatchExportId;

	// Current Unreal truth와 prospective evidence를 binding한 exact Batch Plan hash입니다.
	FString BatchPlanHash;

	// Canonical RowId ascending order의 exact included row set입니다.
	TArray<FCFBatchCommitApprovalRow> IncludedRows;

	// Approval 시점 Conflict row 수이며 B1/B2에서는 0이어야 합니다.
	int32 ConflictRowCount = 0;

	// Approval 시점 explicit Warning row 수입니다.
	int32 WarningRowCount = 0;

	// Source commit을 막지 않는 External Drift row 수입니다.
	int32 ExternalDriftRowCount = 0;

	// Source commit을 막지 않는 ShadowOnly row 수입니다.
	int32 ShadowOnlyRowCount = 0;

	// Prospective affected-Vehicle validation warning 총수입니다.
	int32 ValidationWarningCount = 0;
};

/** Reviewed B1/B2 approval을 실제 source transaction에 전달하는 request입니다. */
struct FCFBatchAuthoringCommitRequest
{
	// P0-08K에서 생성한 exact transient Batch Plan입니다.
	const FCFBatchImportSession* Session = nullptr;

	// Section 26.51 exact approval evidence입니다.
	const FCFBatchCommitApproval* Approval = nullptr;

	// 사용자가 B1/B2 Authoring Source Commit을 명시적으로 승인했는지 여부입니다.
	bool bAuthoringCommitApproved = false;

#if WITH_DEV_AUTOMATION_TESTS
	// Automation rollback 검증용으로 N번째 object patch 직후 failure를 주입하며 production caller는 INDEX_NONE을 유지합니다.
	int32 AutomationFailureAfterPatchedObjectCount = INDEX_NONE;
#endif
};

/** B1/B2 source transaction의 machine-readable terminal 결과입니다. */
struct FCFBatchAuthoringCommitResult
{
	// Commit terminal 상태입니다.
	ECFBatchCommitStatus Status = ECFBatchCommitStatus::NotRun;

	// Stable error taxonomy입니다.
	ECFBatchCommitErrorCode ErrorCode = ECFBatchCommitErrorCode::None;

	// 실제 source mutation이 성공한 included row 수입니다.
	int32 CommittedRowCount = 0;

	// 실제 Modify/typed patch 대상이 된 distinct Recipe/Profile object 수입니다.
	int32 CommittedSourceObjectCount = 0;

	// 성공 후 old approval이 source fingerprint 변화로 소비됐음을 명시합니다.
	bool bApprovalConsumed = false;

	// B1/B2 성공 후 Section 26.44~26.45 fresh read/resolve가 필요한지 여부입니다.
	bool bRequiresFreshPreview = false;

	// B1/B2가 Target VehicleData를 변경했는지 여부이며 이 slice에서는 항상 false여야 합니다.
	bool bTargetMutationPerformed = false;

	// Service가 package save를 수행했는지 여부이며 이 slice에서는 항상 false여야 합니다.
	bool bSavePerformed = false;

	// Successful commit source identity를 canonical ascending order로 보존합니다.
	TArray<FString> CommittedSourceIdentities;

	// Global preflight에서 stale로 판정한 RowId를 canonical ascending order로 보존합니다.
	TArray<FString> StaleRowIds;

	// 사람이 읽는 한국어 terminal diagnostic입니다.
	FString Message;
};

/** Spreadsheet를 direct source-of-truth로 사용하지 않고 Preview→Approval→B1/B2 source transaction 경계를 제공하는 Editor service입니다. */
class CARFIGHT_REEDITOR_API FCFBatchImportService
{
public:
	// Companion JSON을 FCFBatchManifest로 parse하고 current Registry/schema/hash contract까지 검증합니다.
	static bool ParseAndValidateManifest(
		const FString& ManifestJsonText,
		FCFBatchManifest& OutManifest,
		TArray<FCFBatchIssue>& OutIssues);

	// Standard quoted canonical CSV text를 stable ColumnId/header 기반 parsed rows로 변환합니다.
	static bool ParseCanonicalCsv(
		const FString& CsvText,
		const FCFBatchManifest& Manifest,
		TArray<FCFBatchParsedRow>& OutRows,
		TArray<FCFBatchIssue>& OutIssues);

	// Current Unreal truth를 다시 읽어 3-way merge와 Recipe/Profile prospective Pure Resolver preview Session을 만듭니다.
	static bool BuildPreview(
		const FCFBatchImportRequest& Request,
		FCFBatchImportSession& OutSession);

		// Row order와 localized message에 독립적인 deterministic Batch plan hash를 계산합니다.
	static FString BuildBatchPlanHash(const FCFBatchImportSession& Session);

	// SafeCandidate ColumnId/value set만 canonicalize해 Section 26.51 per-row Proposed Patch Hash를 계산합니다.
	static FString BuildRowProposedPatchHash(const FCFBatchRowPreview& Row);

	// Current Session의 모든 commit-candidate row를 exact BatchPlanHash/current fingerprint에 binding한 B1/B2 approval evidence로 만듭니다.
	static bool BuildCommitApproval(
		const FCFBatchImportSession& Session,
		FCFBatchCommitApproval& OutApproval,
		TArray<FCFBatchIssue>& OutIssues);

	// Global preflight를 모두 통과한 뒤 B1 Recipe 또는 B2 Profile source를 one logical transaction으로 all-or-nothing commit합니다.
	static bool CommitAuthoringSources(
		const FCFBatchAuthoringCommitRequest& Request,
		FCFBatchAuthoringCommitResult& OutResult);
};
