// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchApply.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08M B3 Batch Definition Apply Foundation public contract입니다.
// Scope: Frozen Section 26.54~26.65 fresh R3 evidence plan, exact approval, global preflight, per-Vehicle ApplyService sequence와 partial result를 제공합니다.
// Changelog:
// - v1.0.0: FCFBatchDefinitionApplyPlan/Approval/Result와 FCFBatchApplyService 최초 구현 계약 추가.
// Migration:
// - B1/B2 approval은 B3에 재사용할 수 없습니다.
// - Target write는 FCFVehicleApplyService::Apply만 호출하며 batch global transaction/rollback, auto retry, auto save를 제공하지 않습니다.
// - Section 26.66+ BatchOperationId transient dedupe는 별도 후속 slice이며 이 Foundation은 한 ApplyBatch 호출 내부 exact-once만 보장합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleApplyService.h"

class UCFVehicleData;
class UCFVehicleRecipeData;
class FCFBatchApplyTestAccess;

/** Fresh affected Vehicle이 Frozen B3 Apply 대상인지 분류한 상태입니다. */
enum class ECFBatchApplyEligibility : uint8
{
	Eligible,
	NoChange,
	ShadowOnly,
	ExternalDrift,
	ResolveBlocked,
	ResolveError,
	ValidationBlocked,
	InvalidContext
};

/** B3 Batch Apply 전체 operation의 terminal 상태입니다. */
enum class ECFBatchApplyStatus : uint8
{
	NotRun,
	Succeeded,
	NoChange,
	Blocked,
	Failed,
	PartialFailure
};

/** 문자열 parsing 없이 B3 plan/approval/preflight/execute 실패를 분류하는 stable code입니다. */
enum class ECFBatchApplyErrorCode : uint8
{
	None,
	InvalidRequest,
	ApprovalRequired,
	PlanInvalid,
	ApprovalMismatch,
	OrderedTargetSetMismatch,
	PerTargetEvidenceMismatch,
	GlobalPreflightFailed,
	VehicleApplyFailed,
	InternalError
};

/** Section 26.61 exact target list에서 한 Vehicle의 실행 상태입니다. */
enum class ECFBatchVehicleApplyState : uint8
{
	Ineligible,
	NotStarted,
	Applied,
	Failed
};

/** B1/B2 성공 뒤 fresh Unreal read/resolve할 affected Recipe 집합입니다. */
struct FCFBatchDefinitionApplyPlanRequest
{
	// Affected Vehicle을 소유하는 persistent Recipe들입니다. Target은 각 Recipe binding에서 fresh resolve합니다.
	TArray<UCFVehicleRecipeData*> Recipes;

	// Read-only plan build의 caller diagnostic kind입니다.
	ECFAuthoringCallerKind CallerKind = ECFAuthoringCallerKind::Unknown;
};

/** Section 26.54의 per-target R3 evidence와 reviewed ApplyRequest를 함께 보존합니다. */
struct FCFBatchDefinitionApplyItem
{
	// Canonical Target asset/object path입니다.
	FString TargetPath;

	// Canonical Recipe asset/object path입니다.
	FString RecipePath;

	// Transient plan lifetime 동안 exact persistent Recipe를 가리킵니다.
	UCFVehicleRecipeData* Recipe = nullptr;

	// Transient plan lifetime 동안 exact Runtime canonical Target을 가리킵니다.
	UCFVehicleData* TargetVehicleData = nullptr;

	// Frozen 26.55 eligibility terminal classification입니다.
	ECFBatchApplyEligibility Eligibility = ECFBatchApplyEligibility::InvalidContext;

	// Eligible일 때 true이며 B3 execution target set에 포함됩니다.
	bool bEligible = false;

	// Fresh Recipe semantic fingerprint입니다.
	FString ExpectedRecipeFingerprint;

	// Fresh effective Source set signature입니다.
	FString ExpectedSourceSignature;

	// Fresh current Target full Definition hash입니다.
	FString ExpectedTargetDefinitionHash;

	// Fresh Resolver-owned resolved Definition hash입니다.
	FString ExpectedResolvedDefinitionHash;

	// Fresh FieldDiff exact deterministic hash입니다.
	FString ExpectedDiffHash;

	// Fresh Resolver semantic contract revision입니다.
	int32 ExpectedResolverContractRevision = 0;

	// Recipe/Resolver/Definition validation compact summary입니다.
	FCFAuthoringValidationSummary ValidationSummary;

	// Approval summary에 포함할 Warning 수입니다.
	int32 WarningCount = 0;

	// Fresh FieldDiff row 수입니다.
	int32 FieldDiffCount = 0;

	// Add/Remove/Move array structural diff 수입니다.
	int32 ArrayStructuralDiffCount = 0;

	// R16 current Target external drift 여부입니다.
	bool bExternalDrift = false;

	// Section 26.57 per-target R3 evidence hash입니다.
	FString R3EvidenceHash;

	// Eligible item이 execution 때 FCFVehicleApplyService에 전달할 exact reviewed R3 payload입니다.
	FCFVehicleApplyRequest ApplyRequest;

	// 사람이 읽는 eligibility/diagnostic 설명입니다.
	FString Message;
};

/** Section 26.54의 affected context 전체와 exact eligible target set을 보존하는 transient plan입니다. */
struct FCFBatchDefinitionApplyPlan
{
	// Canonical TargetPath ascending으로 정렬된 affected Vehicle items입니다.
	TArray<FCFBatchDefinitionApplyItem> Items;

	// Execution 대상만 추린 canonical TargetPath ascending exact set입니다.
	TArray<FString> OrderedEligibleTargetPaths;

	// Current fresh evidence와 item eligibility를 모두 binding한 deterministic plan hash입니다.
	FString BatchApplyPlanHash;

	// Plan이 읽은 affected Vehicle 총수입니다.
	int32 TotalAffectedVehicleCount = 0;

	// Frozen 26.55를 통과한 eligible Vehicle 수입니다.
	int32 EligibleVehicleCount = 0;

	// Apply하지 않는 ineligible Vehicle 수입니다.
	int32 IneligibleVehicleCount = 0;

	// Eligible Vehicle들의 FieldDiff row 총수입니다.
	int32 TotalFieldDiffCount = 0;

	// Eligible Vehicle들의 Warning 총수입니다.
	int32 TotalWarningCount = 0;

	// Eligible Vehicle들의 array structural diff 총수입니다.
	int32 TotalArrayStructuralDiffCount = 0;

	// B3는 Package Dirty까지만 허용하고 auto save를 하지 않습니다.
	bool bAutoSave = false;
};

/** B3 approval이 한 exact eligible Target과 R3 evidence에 binding하는 row입니다. */
struct FCFBatchDefinitionApplyApprovalItem
{
	// Canonical Target asset/object path입니다.
	FString TargetPath;

	// Exact per-target R3 evidence hash입니다.
	FString R3EvidenceHash;
};

/** Section 26.57 exact Batch Definition Apply approval입니다. */
struct FCFBatchDefinitionApplyApproval
{
	// Exact reviewed Batch Apply Plan hash입니다.
	FString BatchApplyPlanHash;

	// Canonical TargetPath ascending의 exact execution set입니다.
	TArray<FString> OrderedTargetPaths;

	// OrderedTargetPaths와 같은 순서의 per-target R3 evidence입니다.
	TArray<FCFBatchDefinitionApplyApprovalItem> PerTargetEvidence;

	// Approval 대상 eligible Vehicle 수입니다.
	int32 TotalVehicleCount = 0;

	// Approval 대상 FieldDiff row 총수입니다.
	int32 TotalFieldDiffCount = 0;

	// Approval review Warning 총수입니다.
	int32 WarningCount = 0;

	// Approval review array structural diff 총수입니다.
	int32 ArrayStructuralDiffCount = 0;

	// Frozen 26.65에 따라 항상 false입니다.
	bool bAutoSave = false;
};

/** Reviewed plan/approval을 B3 execution에 전달하는 request입니다. */
struct FCFBatchDefinitionApplyRequest
{
	// Fresh reviewed B3 plan입니다.
	const FCFBatchDefinitionApplyPlan* Plan = nullptr;

	// Section 26.57 exact B3 approval입니다. B1/B2 approval type은 받을 수 없습니다.
	const FCFBatchDefinitionApplyApproval* Approval = nullptr;

	// 사용자가 exact B3 Definition Apply를 명시적으로 승인했는지 여부입니다.
	bool bDefinitionApplyApproved = false;
};

/** Section 26.61 한 Target의 Applied/Failed/NotStarted/Ineligible exact result입니다. */
struct FCFBatchVehicleApplyResult
{
	// Canonical Target identity입니다.
	FString TargetPath;

	// Canonical Recipe identity입니다.
	FString RecipePath;

	// Plan build 당시 eligibility입니다.
	ECFBatchApplyEligibility Eligibility = ECFBatchApplyEligibility::InvalidContext;

	// 이번 Batch execution에서의 exact terminal state입니다.
	ECFBatchVehicleApplyState State = ECFBatchVehicleApplyState::Ineligible;

	// 실제 ApplyService가 호출된 경우의 terminal status입니다.
	ECFVehicleApplyStatus ApplyStatus = ECFVehicleApplyStatus::Blocked;

	// 실제 ApplyService가 호출된 경우의 stable failure code입니다.
	ECFVehicleApplyFailureCode ApplyFailureCode = ECFVehicleApplyFailureCode::None;

	// 한 ApplyBatch 호출 안에서 FCFVehicleApplyService::Apply 호출 횟수이며 eligible item은 0 또는 정확히 1입니다.
	int32 ApplyServiceCallCount = 0;

	// 이 target의 ApplyService 성공으로 persistent Target이 변경됐는지 여부입니다.
	bool bTargetChanged = false;

	// 이 target의 ApplyService 성공으로 Recipe AppliedState가 변경됐는지 여부입니다.
	bool bRecipeAppliedStateChanged = false;

	// 사람이 읽는 per-target terminal diagnostic입니다.
	FString Message;
};

/** B3 global preflight와 sequential Apply의 machine-readable terminal result입니다. */
struct FCFBatchDefinitionApplyResult
{
	// Batch terminal 상태입니다.
	ECFBatchApplyStatus Status = ECFBatchApplyStatus::NotRun;

	// Stable B3 error taxonomy입니다.
	ECFBatchApplyErrorCode ErrorCode = ECFBatchApplyErrorCode::None;

	// Reviewed exact plan hash입니다.
	FString BatchApplyPlanHash;

	// Plan의 affected/ineligible item까지 포함한 exact target result list입니다.
	TArray<FCFBatchVehicleApplyResult> VehicleResults;

	// 실제 ApplyService 성공 Vehicle 수입니다.
	int32 AppliedVehicleCount = 0;

	// 첫 실패로 Failed가 된 Vehicle 수이며 P0 기본값은 최대 1입니다.
	int32 FailedVehicleCount = 0;

	// Eligible이지만 Stop On First Failure 때문에 호출되지 않은 Vehicle 수입니다.
	int32 NotStartedVehicleCount = 0;

	// Plan 단계부터 Apply 대상이 아니었던 Vehicle 수입니다.
	int32 IneligibleVehicleCount = 0;

	// 한 ApplyBatch 호출에서 실제 FCFVehicleApplyService::Apply를 호출한 총횟수입니다.
	int32 ApplyServiceCallCount = 0;

	// 하나 이상 Target이 성공적으로 mutation됐는지 여부입니다.
	bool bAnyTargetChanged = false;

	// successful Apply 때문에 하나 이상 package가 dirty일 수 있음을 나타냅니다.
	bool bPackageDirty = false;

	// Batch-level global rollback을 수행했는지 여부이며 Frozen 26.60~26.63에서 항상 false입니다.
	bool bGlobalRollbackPerformed = false;

	// 동일 write를 service가 automatic retry했는지 여부이며 항상 false입니다.
	bool bAutomaticRetryPerformed = false;

	// Disk save를 수행했는지 여부이며 항상 false입니다.
	bool bSavePerformed = false;

	// 사람이 읽는 batch terminal diagnostic입니다.
	FString Message;
};

/** Frozen B3 plan/approval/preflight/per-Vehicle ApplyService orchestration을 소유하는 Editor-only service입니다. */
class CARFIGHT_REEDITOR_API FCFBatchApplyService
{
public:
	// B1/B2 성공 뒤 affected Recipe를 fresh read/resolve해 Section 26.54 R3 evidence collection을 만듭니다.
	static bool BuildDefinitionApplyPlan(
		const FCFBatchDefinitionApplyPlanRequest& Request,
		FCFBatchDefinitionApplyPlan& OutPlan,
		FString& OutError);

	// Item order와 localized message에 독립적인 exact BatchApplyPlanHash를 계산합니다.
	static FString BuildBatchApplyPlanHash(const FCFBatchDefinitionApplyPlan& Plan);

	// Exact eligible Target set/per-target R3 evidence를 Section 26.57 approval로 binding합니다.
	static bool BuildDefinitionApplyApproval(
		const FCFBatchDefinitionApplyPlan& Plan,
		FCFBatchDefinitionApplyApproval& OutApproval,
		FString& OutError);

	// 모든 eligible item global preflight PASS 뒤 canonical order로 FCFVehicleApplyService::Apply를 각 Target에 최대 한 번 호출합니다.
	static bool ApplyBatch(
		const FCFBatchDefinitionApplyRequest& Request,
		FCFBatchDefinitionApplyResult& OutResult);

private:
	// Production Apply와 private Automation pre-apply hook이 공유하는 실제 B3 execution입니다.
	static bool ApplyBatchInternal(
		const FCFBatchDefinitionApplyRequest& Request,
		FCFBatchDefinitionApplyResult& OutResult,
		const TFunction<void(int32, UCFVehicleData*)>& PreApplyHook);

	friend class FCFBatchApplyTestAccess;
};
