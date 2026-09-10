// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingApply.h
// Version: v1.3.0
// Date: 2026-09-10
// Description: trusted typed provider의 reviewed DataAsset Staging Preview를 exact typed UE Asset materialization으로 승격하는 approval/apply public contract입니다.
// Changelog:
// - v1.3.0: CF-FQ-051 DAO-P0-03에서 trusted TypeKey provider readiness 기반 multi-type reviewed Apply를 Current contract로 명시. Public API signature는 변경하지 않음.
// - v1.2.0: DAS-P0-04 전용 Automation fixture가 save/confirmation uncertainty와 post-durable first-failure를 결정적으로 재현할 수 있도록 WITH_DEV_AUTOMATION_TESTS 전용 fault control을 추가.
// - v1.1.0: DurableApplied가 exact SavePackage 뒤 non-interactive package disk reload와 unified typed semantic readback까지 확인해야 한다는 public result contract를 강화.
// - v1.0.0: one-shot Reviewed approval, global TOCTOU preflight, target/batch result taxonomy와 MissileGuidePreset exact materializer facade를 추가.
// Migration:
// - reviewed write allowlist는 code-owned trusted TypeKey provider registry + ReviewedMutationReady readiness로 제한합니다. 현재 MissileGuidePreset + AmmoData exact2가 mutation-ready입니다.
// - 범용 UObject reflection writer나 Save All 경로는 제공하지 않으며 기존 Public API signature는 유지합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFDAStaging.h"

// Reviewed approval의 one-shot lifecycle입니다.
enum class ECFDAStagingApprovalState : uint8
{
	Previewed,
	Reviewed,
	ApplyAttempted,
	Consumed
};

// 한 target의 durable apply terminal 결과입니다.
enum class ECFDAStagingTargetApplyResult : uint8
{
	NotRun,
	NoChange,
	BlockedBeforeMutation,
	FailedBeforeDurableWrite,
	InMemoryStateUnconfirmed,
	DurableApplied,
	SaveStateUnconfirmed,
	PostCommitWarning
};

// 전체 batch의 fail-safe aggregate terminal 결과입니다.
enum class ECFDAStagingBatchApplyResult : uint8
{
	NoChange,
	DurableApplied,
	BlockedBeforeMutation,
	FailedBeforeDurableWrite,
	InMemoryStateUnconfirmed,
	SaveStateUnconfirmed,
	PartialApplied,
	PostCommitWarning
};

// Approval이 exact Preview target 하나에 대해 보존하는 immutable evidence projection입니다.
struct FCFDAStagingApprovalTarget
{
	// Schema semantic identity입니다.
	FString SchemaId;

	// JSON shape revision입니다.
	int32 SchemaRevision = 0;

	// Typed adapter meaning revision입니다.
	int32 AdapterContractRevision = 0;

	// Exact native DataAsset class path입니다.
	FString DataAssetTypeClassPath;

	// Registry-compatible stable logical identity입니다.
	FName StableLogicalId = NAME_None;

	// Exact Unreal object path입니다.
	FString TargetObjectPath;

	// Exact normalized main_game-relative Staging source path입니다.
	FString StagingRelativePath;

	// Update baseline 존재 여부입니다.
	bool bHasBaseSemanticFingerprint = false;

	// Reviewed Base semantic fingerprint입니다.
	FString BaseSemanticFingerprint;

	// Reviewed Current semantic fingerprint입니다. Create에는 비어 있습니다.
	FString CurrentSemanticFingerprint;

	// Reviewed desired Staging semantic fingerprint입니다.
	FString StagingSemanticFingerprint;

	// Reviewed mutation operation은 Create 또는 Update만 허용합니다.
	ECFDAStagingPreviewKind PlannedOperation = ECFDAStagingPreviewKind::Invalid;
};

// USER/상위 UI가 exact Preview를 검토한 뒤 Apply에 전달하는 one-shot approval evidence입니다.
struct FCFDAStagingReviewedApproval
{
	// 현재 approval lifecycle state입니다.
	ECFDAStagingApprovalState State = ECFDAStagingApprovalState::Previewed;

	// Exact sorted mutation target set을 결합한 deterministic SHA-256 plan hash입니다.
	FString BatchPlanHash;

	// BatchPlanHash와 함께 exact target set을 사람이/도구가 다시 확인할 수 있게 보존하는 evidence입니다.
	TArray<FCFDAStagingApprovalTarget> IncludedTargets;
};

// 한 target의 Apply 결과와 machine-readable diagnostics입니다.
struct FCFDAStagingTargetApplyReport
{
	// Exact target object path입니다.
	FString TargetObjectPath;

	// Stable logical identity입니다.
	FName StableLogicalId = NAME_None;

	// Planned Create/Update operation입니다.
	ECFDAStagingPreviewKind PlannedOperation = ECFDAStagingPreviewKind::Invalid;

	// Target terminal result입니다.
	ECFDAStagingTargetApplyResult Result = ECFDAStagingTargetApplyResult::NotRun;

	// Target-specific stable diagnostics입니다.
	TArray<FCFDAStagingIssue> Issues;

	// USER/debug용 상세 진단입니다.
	FString Diagnostic;
};

// Exact reviewed batch Apply의 전체 terminal report입니다.
struct FCFDAStagingApplyReport
{
	// Batch fail-safe aggregate terminal result입니다.
	ECFDAStagingBatchApplyResult Result = ECFDAStagingBatchApplyResult::NoChange;

	// Apply invocation 시작 시 approval이 one-shot으로 소비되었는지 여부입니다.
	bool bApprovalConsumed = false;

	// Confirmed durable target 수입니다.
	int32 DurableAppliedCount = 0;

	// 실행되지 않은 target 수입니다.
	int32 NotRunCount = 0;

	// Target별 deterministic execution/report 결과입니다.
	TArray<FCFDAStagingTargetApplyReport> Targets;

	// Batch-level diagnostic입니다.
	FString Diagnostic;
};

// Trusted typed provider materializer와 one-shot reviewed Apply를 제공하는 Editor-only service입니다.
#if WITH_DEV_AUTOMATION_TESTS
// DAS-P0-04 test fixture만 사용하는 deterministic fault injection control입니다.
class FCFDAStagingApplyTestControl
{
public:
	// 모든 test-only fault injection state를 초기화합니다.
	static void Reset();

	// exact target의 SavePackage 호출 시점을 outcome-unknown failure로 강제합니다.
	static void ForceSaveFailure(const FString& TargetObjectPath);

	// exact target의 SavePackage 성공 뒤 persisted confirmation 실패를 강제합니다.
	static void ForceConfirmationFailure(const FString& TargetObjectPath);

	// exact target의 global preflight 뒤 mutation 직전 known block을 강제합니다.
	static void ForceBeforeMutationBlock(const FString& TargetObjectPath);
};
#endif

class CARFIGHT_REEDITOR_API FCFDAStagingApplyService
{
public:
	// Conflict/Invalid 없는 exact Create/Update Preview set을 Reviewed approval evidence로 동결합니다.
	static bool BuildReviewedApproval(
		const TArray<FCFDAStagingPreviewRow>& PreviewRows,
		FCFDAStagingReviewedApproval& OutApproval,
		FString& OutError);

	// Reviewed approval을 one-shot 소비하고 disk Staging + current UE truth를 global preflight한 뒤 exact target만 순서대로 materialize/save합니다.
	static bool ApplyReviewedBatch(
		FCFDAStagingReviewedApproval& InOutApproval,
		FCFDAStagingApplyReport& OutReport);
};
