// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCompiler.h
// Version: v1.2.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS Generic Compiler Core의 read/validate/diff/impact/fingerprint/batch preview 계약입니다.
// Changelog:
// - v1.2.0: P0-06 Catalog Snapshot이 재사용할 whole-current ProductSemanticHash를 CompileResult에 노출.
// - v1.1.0: Mid-review 교정으로 current/desired reference graph 분리, retire impact 안전성, provider fingerprint/schema fail-closed와 current snapshot consistency guard를 추가.
// - v1.0.0: Workbook adapter reader, provider-backed validation/diff, reference graph, impact analysis, generated drift와 preview batch result를 최초 구현.
// Migration:
// - P0-01 Canonical Content Model/Provider Registry/Workbook Adapter 계약을 그대로 재사용합니다.
// - 이 API는 Product Apply, DataAsset Save, CCAS-P0-07 authority cutover를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentProvider.h"
#include "DataAuthoring/CFWorkbookAdapter.h"

/** Desired Workbook과 current Product 사이의 record-level diff 종류입니다. */
enum class ECFContentCompileDiffKind : uint8
{
	Added,
	Removed,
	Modified,
	Unchanged
};

/** Provider-managed semantic fingerprint 기준의 generated drift 상태입니다. */
enum class ECFContentDriftState : uint8
{
	InSync,
	MissingCurrent,
	Changed,
	RetiredCurrent,
	ExternalReadOnly
};

/** Apply 전 preview batch item의 결과 상태입니다. */
enum class ECFContentBatchItemState : uint8
{
	ReadyToReview,
	RetireCandidate,
	AlreadyCurrent,
	ExternalReadOnly,
	Blocked
};

/** Generic Compiler가 만든 record-level typed diff입니다. */
struct FCFContentCompileDiff
{
	// Diff 대상 canonical logical key입니다.
	FCFContentKey Key;

	// Added/Removed/Modified/Unchanged record-level 상태입니다.
	ECFContentCompileDiffKind Kind = ECFContentCompileDiffKind::Unchanged;

	// Desired/current record의 CCAS management state입니다.
	ECFContentManagementState ManagementState = ECFContentManagementState::ExternalReadOnly;

	// Provider가 보고한 current Product semantic fingerprint입니다.
	FString CurrentProviderFingerprint;

	// Provider가 보고한 desired semantic fingerprint입니다.
	FString DesiredProviderFingerprint;

	// Provider typed diff가 보고한 stable field/collection path 목록입니다.
	TArray<FString> ChangedPaths;
};

/** Workbook source와 Product readback을 연결하는 generated fingerprint/drift record입니다. */
struct FCFContentGeneratedFingerprint
{
	// Fingerprint 대상 canonical logical key입니다.
	FCFContentKey Key;

	// 이번 preview를 생성한 authoritative WorkbookSemanticHash입니다.
	FString SourceWorkbookSemanticHash;

	// P0-01 canonical semantic hasher를 재사용해 만든 desired record projection fingerprint입니다.
	FString DesiredCanonicalFingerprint;

	// Provider가 읽은 current Product fingerprint입니다.
	FString CurrentProviderFingerprint;

	// Provider typed diff가 계산한 desired Product fingerprint입니다.
	FString DesiredProviderFingerprint;

	// Provider-managed semantic scope에서의 drift 상태입니다.
	ECFContentDriftState DriftState = ECFContentDriftState::InSync;

	// Drift를 만든 provider-managed stable path 목록입니다.
	TArray<FString> ChangedPaths;
};

/** 변경 root 하나가 영향을 주는 referencer 집합입니다. */
struct FCFContentImpactEntry
{
	// Added/Removed/Modified root content key입니다.
	FCFContentKey ChangedKey;

	// ChangedKey를 직접 참조하는 desired Workbook records입니다.
	TArray<FCFContentKey> DirectDependents;

	// Direct + transitive referencer 전체 집합입니다.
	TArray<FCFContentKey> TransitiveDependents;
};

/** Apply를 수행하지 않는 preview batch item 결과입니다. */
struct FCFContentBatchItemResult
{
	// Batch item canonical logical key입니다.
	FCFContentKey Key;

	// Review/retire/current/read-only/blocked preview 상태입니다.
	ECFContentBatchItemState State = ECFContentBatchItemState::Blocked;

	// UI/AI review layer가 표시할 deterministic 한국어 summary입니다.
	FString Summary;
};

/** Generic Compiler preview batch의 집계 결과입니다. */
struct FCFContentBatchResult
{
	// Desired Workbook record 수입니다.
	int32 DesiredRecordCount = 0;

	// Provider ImportExisting에서 관측한 current record 수입니다.
	int32 CurrentRecordCount = 0;

	// Added/Modified로 USER review가 필요한 item 수입니다.
	int32 ReadyToReviewCount = 0;

	// Desired Workbook에서 사라져 retire review가 필요한 item 수입니다.
	int32 RetireCandidateCount = 0;

	// Provider typed diff상 이미 동일한 item 수입니다.
	int32 AlreadyCurrentCount = 0;

	// ExternalReadOnly라 CCAS Product mutation 대상이 아닌 item 수입니다.
	int32 ExternalReadOnlyCount = 0;

	// Compiler/Provider blocking issue로 preview가 막힌 item 수입니다.
	int32 BlockedCount = 0;

	// Stable ContentKey 순서의 per-item result입니다.
	TArray<FCFContentBatchItemResult> Items;
};

/** CCAS-P0-02 Generic Compiler의 read-only preview 전체 결과입니다. */
struct FCFContentCompileResult
{
	// 전체 read/validation/diff/impact pipeline이 blocking issue 없이 완료됐는지 여부입니다.
	bool bSucceeded = false;

	// 실패 시 어느 stage에서 중단됐는지 나타내는 stable stage ID입니다.
	FString FailureStage;

	// 실패 시 사용자-facing 상세 원인입니다.
	FString Error;

	// 실제 Workbook read에 사용된 adapter identity/capability 정보입니다.
	FCFWorkbookAdapterInfo AdapterInfo;

	// Adapter가 읽은 canonical Workbook model입니다.
	FCFContentWorkbookModel Workbook;

	// P0-01 canonical semantic hasher가 만든 source Workbook hash입니다.
	FString WorkbookSemanticHash;

	// Preview 시작/종료 exact consistency를 통과한 whole-current Product canonical semantic hash입니다.
	FString CurrentProductSemanticHash;

	// Common + provider validation diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;

	// Provider가 desired records에서 구축한 validated desired content dependency graph입니다.
	TArray<FCFContentDependencyEdge> DependencyEdges;

	// Provider가 imported current Product records에서 구축한 validated current dependency graph입니다.
	TArray<FCFContentDependencyEdge> CurrentDependencyEdges;

	// Current Product와 desired Workbook의 deterministic record-level diff입니다.
	TArray<FCFContentCompileDiff> Diffs;

	// Workbook source hash와 Product readback을 연결하는 generated fingerprint/drift 결과입니다.
	TArray<FCFContentGeneratedFingerprint> GeneratedFingerprints;

	// Changed roots의 direct/transitive referencer impact입니다.
	TArray<FCFContentImpactEntry> Impacts;

	// Apply 없이 생성한 preview batch result입니다.
	FCFContentBatchResult BatchResult;
};

/** P0-01 typed contracts를 조합해 read-only Generic Compiler preview를 수행합니다. */
class FCFContentCompiler
{
public:
	// Workbook adapter read부터 provider diff/impact/batch preview까지 한 번에 수행하며 Product mutation은 호출하지 않습니다.
	static bool CompilePreview(
		const FString& WorkbookPath,
		ICFContentWorkbookAdapter& WorkbookAdapter,
		const FCFContentProviderRegistry& ProviderRegistry,
		FCFContentCompileResult& OutResult);
};
