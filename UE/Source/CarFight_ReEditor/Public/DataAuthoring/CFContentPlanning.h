// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentPlanning.h
// Version: v1.1.0
// Date: 2026-10-03
// Description: CF-FQ-058 CCAS-P0-06 immutable Catalog Snapshot, typed Change Proposal,
// semantic preflight, Review Package와 two-layer stale guard의 Editor-only contract입니다.
// Changelog:
// - v1.1.0: Production-specific approved review → typed execution exact binding을 위해 optional ExecutionBindingFingerprint를 immutable Review Package payload에 추가.
// - v1.0.1: P0-07 Mid-review 연계용 read-only ReviewPackage fingerprint 계산 helper를 공개.
// - v1.0.0: P0-06 frozen design exact 범위를 최초 구현.
// Migration:
// - P0-01~05 Core/Compiler/Provider seam을 재사용하며 Product Apply/Save를 호출하지 않습니다.
// - P0-06의 mutable target은 transient in-memory Workbook candidate뿐입니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentCompiler.h"
#include "DataAuthoring/CFContentResource.h"

/** AI planning/review가 결속되는 whole registered Catalog immutable snapshot입니다. */
struct FCFContentCatalogSnapshot
{
	// Snapshot 전체 machine-semantic payload의 canonical SHA-256 identity입니다.
	FString SnapshotFingerprint;

	// Canonical Workbook logical source identity입니다.
	FString WorkbookSourceId;

	// Snapshot의 authoritative Workbook semantic concurrency token입니다.
	FString WorkbookSemanticHash;

	// Snapshot의 Workbook machine schema revision입니다.
	int32 WorkbookSchemaVersion = 0;

	// Registered Provider schema/ownership 전체 fingerprint입니다.
	FString ProviderSchemaFingerprint;

	// Current Product + ExternalReadOnly semantic state fingerprint입니다.
	FString ProductStateFingerprint;

	// Registered Resource existence/type/capability fingerprint입니다.
	FString ResourceCatalogFingerprint;

	// Snapshot이 보존하는 canonical desired Workbook model입니다.
	FCFContentWorkbookModel Workbook;

	// Desired Workbook provider dependency graph입니다.
	TArray<FCFContentDependencyEdge> DependencyEdges;

	// Current Product provider dependency graph입니다.
	TArray<FCFContentDependencyEdge> CurrentDependencyEdges;

	// Snapshot 생성 시의 typed diagnostics입니다.
	TArray<FCFContentValidationIssue> ValidationSummary;

	// Workbook/Product drift와 provider fingerprint 연결 증거입니다.
	TArray<FCFContentGeneratedFingerprint> GeneratedDriftSummary;
};

/** AI proposal에서 touched record exact1에 대응하는 review rationale입니다. */
struct FCFContentRecordRationale
{
	// Rationale 대상 canonical content key입니다.
	FCFContentKey ContentKey;

	// 사용자가 이해할 수 있는 변경 이유입니다.
	FString Reason;

	// 해당 record의 design intent가 어떻게 달라지는지 설명합니다.
	FString DesignIntentDelta;

	// 이 rationale이 설명하는 ChangeSet operation indexes입니다.
	TArray<int32> RelatedOperationIndexes;
};

/** AI가 immutable Catalog Snapshot에 대해 제안하는 typed P0-06 change proposal입니다. */
struct FCFContentChangeProposal
{
	// 실제 deterministic mutation intent를 담는 typed Change Set입니다.
	FCFContentChangeSet ChangeSet;

	// Proposal 생성 기준 whole-Catalog SnapshotFingerprint입니다.
	FString BaseCatalogSnapshotFingerprint;

	// Touched record별 exact1 rationale 집합입니다.
	TArray<FCFContentRecordRationale> RecordRationales;
};

/** Product payload diff와 별개로 Planning/Lifecycle operation을 typed review projection으로 보존합니다. */
struct FCFContentReviewChange
{
	// 원본 ChangeSet operation index입니다.
	int32 OperationIndex = INDEX_NONE;

	// 변경 대상 canonical content key입니다.
	FCFContentKey ContentKey;

	// 변경 종류를 보존하는 typed operation enum입니다.
	ECFContentChangeOperationType OperationType = ECFContentChangeOperationType::UpdateField;

	// Field/child/planning/lifecycle을 구분하는 stable semantic target입니다.
	FString SemanticTarget;
};

/** USER가 whole Change Set exact1 단위로 검토하는 immutable review artifact입니다. */
struct FCFContentReviewPackage
{
	// 전체 review artifact의 canonical SHA-256 identity입니다.
	FString ReviewPackageFingerprint;

	// Review 대상 Change Set stable identity입니다.
	FString ChangeSetId;

	// Change Set machine-semantic payload fingerprint입니다.
	FString ChangeSetFingerprint;

	// Review가 결속된 whole-Catalog SnapshotFingerprint입니다.
	FString BaseCatalogSnapshotFingerprint;

	// Review가 결속된 Workbook semantic concurrency token입니다.
	FString BaseWorkbookSemanticHash;

	// 모든 operation 적용 후 expected Workbook semantic hash입니다.
	FString ExpectedPostSemanticHash;

	// Domain-specific reviewed execution manifest를 결속하는 optional canonical SHA-256입니다. Generic review에서는 empty를 허용합니다.
	FString ExecutionBindingFingerprint;

	// Touched record별 review rationale입니다.
	TArray<FCFContentRecordRationale> RecordRationales;

	// Planning/Lifecycle을 포함한 ChangeSet operation의 deterministic typed review projection입니다.
	TArray<FCFContentReviewChange> PlanningChanges;

	// Existing Generic Compiler가 다시 계산한 authoritative Product diff/impact/validation preview입니다.
	FCFContentCompileResult Preview;

	// P0-06 review path에서 staged/persistent Workbook write를 호출하지 않았는지 나타냅니다.
	bool bNoPersistentWorkbookWriteGuardPassed = false;

	// P0-06 review path가 Product Apply authority를 열지 않는 계약임을 나타냅니다.
	bool bNoDirectDataAssetWriteGuardPassed = false;
};

/** USER approval이 exact immutable ReviewPackage에 결속됐는지 표현합니다. */
struct FCFContentReviewApproval
{
	// USER가 승인한 exact ReviewPackageFingerprint입니다.
	FString ReviewPackageFingerprint;

	// USER가 이 exact package를 승인했는지 여부입니다.
	bool bApproved = false;
};

/** Frozen P0-06 planning/review layer를 Product writer 없이 조합합니다. */
class FCFContentPlanningService
{
public:
	// Successful Compiler Preview와 provider/resource fingerprints를 whole-Catalog immutable snapshot으로 고정합니다.
	static bool BuildCatalogSnapshot(
		const FCFContentCompileResult& CompileResult,
		const FCFContentProviderRegistry& ProviderRegistry,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		FCFContentCatalogSnapshot& OutSnapshot,
		FString& OutError);

	// Snapshot-aware semantic preflight 뒤 transient Workbook candidate를 기존 Compiler로 preview하고 immutable ReviewPackage를 생성합니다.
	static bool BuildReviewPackage(
		const FCFContentCatalogSnapshot& BaseSnapshot,
		const FCFContentChangeProposal& Proposal,
		const FCFContentProviderRegistry& ProviderRegistry,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		FCFContentReviewPackage& OutReviewPackage,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError);

	// Review 뒤 fresh Catalog/Workbook이 바뀌지 않았는지 two-layer stale guard로 검증합니다.
	static bool ValidateReviewFreshness(
		const FCFContentReviewPackage& ReviewPackage,
		const FCFContentCatalogSnapshot& FreshSnapshot,
		TArray<FCFContentValidationIssue>& OutIssues);

	// ReviewPackage current payload의 deterministic canonical fingerprint를 read-only로 계산합니다.
	static bool ComputeReviewPackageFingerprint(
		const FCFContentReviewPackage& ReviewPackage,
		FString& OutFingerprint,
		FString& OutError);

	// USER approval이 exact immutable ReviewPackageFingerprint와 fresh payload fingerprint에 결속됐는지 검증합니다.
	static bool ValidateApproval(
		const FCFContentReviewPackage& ReviewPackage,
		const FCFContentReviewApproval& Approval,
		TArray<FCFContentValidationIssue>& OutIssues);
};
