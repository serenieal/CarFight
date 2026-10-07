// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCutover.h
// Version: v1.2.0
// Date: 2026-10-03
// Description: CF-FQ-058 CCAS-P0-07 authority cutover manifest, approval binding,
// transaction, provenance, drift guard와 typed Product apply adapter 계약입니다.
// Changelog:
// - v1.2.0: Mid-review 교정. exact Manifest 실행 재검증, fixed execution path, durable recovery finalize 진입점,
//   promotion 재검증/원자 교체 계약을 추가.
// - v1.1.1: Final Technical Review에서 중복 documentation marker를 제거. 동작 의미 변경 없음.
// - v1.1.0: existing ICFContentProvider의 reviewed typed apply seam을 재사용하는 FCFProviderCutoverAdapter를 추가. 신규 Product Create는 별도 writer 없이 fail-closed.
// - v1.0.0: P0-07 frozen contract를 기존 P0-01~06 의미 변경 없이 additive 계층으로 최초 구현.
// Migration:
// - 이 계층의 Verified는 authority activation eligibility만 뜻합니다.
// - 실제 Workbook Current authority 활성화와 Current Systems promotion은 별도 USER acceptance 전에는 수행하지 않습니다.
// - Product physical delete/rename/move API는 의도적으로 제공하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentPlanning.h"
#include "DataAuthoring/CFContentResource.h"

/** P0-07 durable cutover transaction 상태입니다. */
enum class ECFContentCutoverState : uint8
{
	Prepared,
	ProductApplied,
	WorkbookCommitted,
	Verified,
	BlockedBeforeMutation,
	RecoveryRequired
};

/** Cutover target 하나의 실행 상태입니다. */
enum class ECFContentCutoverTargetState : uint8
{
	Pending,
	NoChange,
	Applied,
	Verified,
	Blocked,
	Failed
};

/** P0-07 reviewed Product mutation authority의 exact identity binding입니다. */
struct FCFContentCutoverBinding
{
	// Review 대상 Change Set identity입니다.
	FString ChangeSetId;

	// Review 시점 canonical Workbook semantic hash입니다.
	FString BaseWorkbookSemanticHash;

	// Review 시점 whole-Catalog snapshot fingerprint입니다.
	FString BaseCatalogSnapshotFingerprint;

	// USER가 승인할 immutable Review Package fingerprint입니다.
	FString ReviewPackageFingerprint;

	// Review 시점 registered Provider schema fingerprint입니다.
	FString ProviderSchemaFingerprint;

	// Mutation 대상 logical content identity입니다.
	FCFContentKey ContentKey;

	// Review 시점 exact Product current/absence fingerprint입니다.
	FString PreApplyProductFingerprint;

	// Reviewed desired Product semantic fingerprint입니다.
	FString DesiredProductFingerprint;

	// Reviewed canonical record projection fingerprint입니다.
	FString DesiredCanonicalFingerprint;
};

/** USER가 explicit onboarding한 managed target 하나입니다. */
struct FCFContentCutoverManifestEntry
{
	// P0-07 apply authority를 구성하는 exact identity binding입니다.
	FCFContentCutoverBinding Binding;

	// Reviewed record의 management state입니다.
	ECFContentManagementState ManagementState = ECFContentManagementState::ExternalReadOnly;

	// Reviewed record의 logical lifecycle입니다.
	ECFContentLifecycleState LifecycleState = ECFContentLifecycleState::Active;

	// Product mutation이 필요한 target인지 여부입니다.
	bool bProductMutationRequired = false;
};

/** CCAS authority를 명시적으로 받을 exact ContentKey set입니다. */
struct FCFContentCutoverManifest
{
	// Stable manifest identity입니다.
	FString ManifestId;

	// Manifest semantic payload의 canonical SHA-256 fingerprint입니다.
	FString ManifestFingerprint;

	// Stable ContentKey order의 explicit managed onboarding entries입니다.
	TArray<FCFContentCutoverManifestEntry> Entries;
};

/** USER approval을 ReviewPackage + exact Cutover Manifest에 결속합니다. */
struct FCFContentCutoverApproval
{
	// USER가 승인한 immutable ReviewPackageFingerprint입니다.
	FString ReviewPackageFingerprint;

	// USER가 승인한 exact ManifestFingerprint입니다.
	FString ManifestFingerprint;

	// 이 exact pair를 USER가 승인했는지 여부입니다.
	bool bApproved = false;
};

/** Product target 하나의 transaction evidence입니다. */
struct FCFContentCutoverTargetReport
{
	// Target logical identity입니다.
	FCFContentKey ContentKey;

	// Target 실행 상태입니다.
	ECFContentCutoverTargetState State = ECFContentCutoverTargetState::Pending;

	// Apply 직전 재계산한 Product current/absence fingerprint입니다.
	FString FreshPreApplyProductFingerprint;

	// Reviewed desired Product fingerprint입니다.
	FString DesiredProductFingerprint;

	// Apply 뒤 persisted typed readback fingerprint입니다.
	FString PostApplyReadbackFingerprint;

	// Product persistent mutation이 실제 수행됐는지 여부입니다.
	bool bProductMutationPerformed = false;

	// Fail-closed 또는 recovery 상세입니다.
	FString Diagnostic;
};

/** Durable P0-07 cutover transaction record입니다. */
struct FCFContentCutoverTransaction
{
	// Stable transaction identity입니다.
	FString TransactionId;

	// Transaction 현재 상태입니다.
	ECFContentCutoverState State = ECFContentCutoverState::Prepared;

	// Canonical repository-relative Workbook path입니다.
	FString CanonicalWorkbookPath;

	// Reviewed candidate sibling staged Workbook path입니다.
	FString StagedWorkbookPath;

	// Expected post-change Workbook semantic hash입니다.
	FString ExpectedWorkbookSemanticHash;

	// Exact approved manifest fingerprint입니다.
	FString ManifestFingerprint;

	// Target별 pre/post/apply evidence입니다.
	TArray<FCFContentCutoverTargetReport> Targets;

	// Verified 뒤 별도 USER authority activation gate에 진입 가능한지 여부입니다.
	bool bAuthorityActivationEligible = false;

	// P0-07 implementation은 이 값을 true로 바꾸지 않습니다.
	bool bAuthorityActivated = false;

	// Transaction fail/recovery 상세입니다.
	FString Diagnostic;
};

/** Generated provenance sidecar의 managed record 하나입니다. */
struct FCFContentProvenanceRecord
{
	// Managed logical content identity입니다.
	FCFContentKey ContentKey;

	// 이 provenance를 만든 cutover transaction identity입니다.
	FString CutoverTransactionId;

	// Authority candidate source Workbook semantic hash입니다.
	FString SourceWorkbookSemanticHash;

	// Review 시점 Catalog snapshot fingerprint입니다.
	FString CatalogSnapshotFingerprint;

	// USER-approved Review Package fingerprint입니다.
	FString ReviewPackageFingerprint;

	// Review 시점 Provider schema fingerprint입니다.
	FString ProviderSchemaFingerprint;

	// Reviewed canonical record projection fingerprint입니다.
	FString DesiredCanonicalFingerprint;

	// Product Apply 전 exact current/absence fingerprint입니다.
	FString PreApplyProductFingerprint;

	// Product Apply 뒤 persisted typed readback fingerprint입니다.
	FString PostApplyReadbackFingerprint;

	// Managed/ExternalReadOnly state입니다.
	ECFContentManagementState ManagementState = ECFContentManagementState::ExternalReadOnly;

	// Active/Deprecated/Retired logical lifecycle입니다.
	ECFContentLifecycleState LifecycleState = ECFContentLifecycleState::Active;
};

/** CarFight_Content.cfsnapshot.json durable generated provenance model입니다. */
struct FCFContentProvenanceSnapshot
{
	// Stable snapshot schema identity입니다.
	FString SchemaId = TEXT("carfight-content-snapshot/v1");

	// Snapshot을 생성한 transaction identity입니다.
	FString CutoverTransactionId;

	// Stable ContentKey order의 managed provenance records입니다.
	TArray<FCFContentProvenanceRecord> Records;
};

/** Product 존재/부재까지 포함한 fresh readback 결과입니다. */
struct FCFContentProductState
{
	// Product가 persistent target으로 존재하는지 여부입니다.
	bool bExists = false;

	// 존재하면 provider-managed readback fingerprint, 부재면 canonical absence fingerprint입니다.
	FString ProductFingerprint;
};

/** P0-07이 기존 typed provider/durable writer를 호출하기 위한 content-type local bridge입니다. */
class ICFContentCutoverProductAdapter
{
public:
	// Interface를 polymorphic하게 안전하게 정리합니다.
	virtual ~ICFContentCutoverProductAdapter() = default;

	// 이 adapter가 소유하는 exact ContentTypeId를 반환합니다.
	virtual FCFContentTypeId DescribeContentType() const = 0;

	// Reviewed canonical record의 provider-managed desired Product fingerprint를 계산합니다.
	virtual bool BuildDesiredProductFingerprint(
		const FCFContentRecord& DesiredRecord,
		FString& OutFingerprint,
		FString& OutError) const = 0;

	// Product current truth 또는 explicit absence를 mutation 없이 읽습니다.
	virtual bool ReadCurrentProductState(
		const FCFContentKey& ContentKey,
		FCFContentProductState& OutState,
		FString& OutError) const = 0;

	// P0-07 binding이 검증된 desired record를 기존 typed durable writer로 적용합니다.
	virtual bool ApplyReviewedProduct(
		const FCFContentCutoverBinding& Binding,
		const FCFContentRecord& DesiredRecord,
		FCFContentApplyResult& OutResult) = 0;
};

/** Existing ICFContentProvider의 typed read/diff/reviewed apply 계약을 P0-07 approval binding에 연결합니다. */
class FCFProviderCutoverAdapter final : public ICFContentCutoverProductAdapter
{
public:
	// Provider와 그 exact ContentTypeId를 non-owning으로 결속합니다. Provider lifetime은 Cutover 실행보다 길어야 합니다.
	FCFProviderCutoverAdapter(
		ICFContentProvider& InProvider,
		const FCFContentTypeId& InContentTypeId);

	// Provider가 소유하는 exact ContentTypeId를 반환합니다.
	virtual FCFContentTypeId DescribeContentType() const override;

	// Existing Product current snapshot과 desired record의 provider-local diff에서 desired fingerprint를 계산합니다.
	virtual bool BuildDesiredProductFingerprint(
		const FCFContentRecord& DesiredRecord,
		FString& OutFingerprint,
		FString& OutError) const override;

	// Provider import/readback으로 existing Product 또는 explicit absence를 mutation 없이 확인합니다.
	virtual bool ReadCurrentProductState(
		const FCFContentKey& ContentKey,
		FCFContentProductState& OutState,
		FString& OutError) const override;

	// Cutover binding을 재확인한 뒤 기존 Provider BuildReviewedMutationPlan/ApplyReviewed를 그대로 호출합니다.
	virtual bool ApplyReviewedProduct(
		const FCFContentCutoverBinding& Binding,
		const FCFContentRecord& DesiredRecord,
		FCFContentApplyResult& OutResult) override;

private:
	// Reuse할 existing typed provider입니다. 소유권은 caller/provider registry에 있습니다.
	ICFContentProvider* Provider = nullptr;

	// Adapter가 소유한다고 선언한 exact logical ContentTypeId입니다.
	FCFContentTypeId ContentTypeId;
};

/** ContentTypeId별 P0-07 Product adapter registry입니다. */
class FCFContentCutoverProductRegistry
{
public:
	// Exact ContentTypeId에 adapter를 중복 없이 등록합니다.
	bool RegisterAdapter(
		const TSharedRef<ICFContentCutoverProductAdapter>& Adapter,
		FString& OutError);

	// Exact ContentTypeId의 adapter를 반환하며 없으면 nullptr입니다.
	ICFContentCutoverProductAdapter* FindAdapter(
		const FCFContentTypeId& ContentTypeId) const;

private:
	// Stable ContentTypeId -> typed Product bridge입니다.
	TMap<FString, TSharedRef<ICFContentCutoverProductAdapter>> AdaptersByContentType;
};

/** Durable transaction/provenance와 canonical Workbook promotion의 persistence boundary입니다. */
class ICFContentCutoverStore
{
public:
	// Interface를 polymorphic하게 안전하게 정리합니다.
	virtual ~ICFContentCutoverStore() = default;

	// Current transaction state/evidence를 durable 저장합니다.
	virtual bool SaveTransaction(
		const FCFContentCutoverTransaction& Transaction,
		FString& OutError) = 0;

	// Existing transaction evidence를 durable storage에서 읽습니다.
	virtual bool LoadTransaction(
		const FString& TransactionId,
		FCFContentCutoverTransaction& OutTransaction,
		FString& OutError) const = 0;

	// Validated staged Workbook을 canonical path로 승격합니다.
	virtual bool PromoteStagedWorkbook(
		const FString& StagedWorkbookPath,
		const FString& CanonicalWorkbookPath,
		FString& OutError) = 0;

	// Generated provenance sidecar를 durable 저장합니다.
	virtual bool SaveProvenance(
		const FString& ProvenancePath,
		const FCFContentProvenanceSnapshot& Snapshot,
		FString& OutError) = 0;

	// Generated provenance sidecar를 read-only로 읽습니다.
	virtual bool LoadProvenance(
		const FString& ProvenancePath,
		FCFContentProvenanceSnapshot& OutSnapshot,
		FString& OutError) const = 0;
};

/** UE Editor filesystem을 사용하는 production durable store입니다. */
class FCFContentCutoverFileStore final : public ICFContentCutoverStore
{
public:
	// Transaction JSON을 UE/Saved/CCAS/Cutover 아래에 저장합니다.
	virtual bool SaveTransaction(
		const FCFContentCutoverTransaction& Transaction,
		FString& OutError) override;

	// UE/Saved/CCAS/Cutover의 exact transaction JSON을 읽습니다.
	virtual bool LoadTransaction(
		const FString& TransactionId,
		FCFContentCutoverTransaction& OutTransaction,
		FString& OutError) const override;

	// Validated staged Workbook을 repository-relative canonical path로 승격합니다.
	virtual bool PromoteStagedWorkbook(
		const FString& StagedWorkbookPath,
		const FString& CanonicalWorkbookPath,
		FString& OutError) override;

	// Repository-relative provenance JSON을 저장합니다.
	virtual bool SaveProvenance(
		const FString& ProvenancePath,
		const FCFContentProvenanceSnapshot& Snapshot,
		FString& OutError) override;

	// Repository-relative provenance JSON을 읽습니다.
	virtual bool LoadProvenance(
		const FString& ProvenancePath,
		FCFContentProvenanceSnapshot& OutSnapshot,
		FString& OutError) const override;
};

/** P0-07 cutover 실행 입력입니다. */
struct FCFContentCutoverRequest
{
	// Stable transaction identity입니다.
	FString TransactionId;

	// Canonical repository-relative Workbook path입니다.
	FString CanonicalWorkbookPath = TEXT("Authoring/Content/CarFight_Content.xlsx");

	// Sibling staged Workbook path입니다.
	FString StagedWorkbookPath = TEXT("Authoring/Content/CarFight_Content.staged.xlsx");

	// Generated provenance sidecar path입니다.
	FString ProvenancePath = TEXT("Authoring/Content/CarFight_Content.cfsnapshot.json");

	// P0-06 immutable reviewed artifact입니다.
	FCFContentReviewPackage ReviewPackage;

	// USER approval bound to ReviewPackage + exact manifest입니다.
	FCFContentCutoverApproval Approval;

	// Explicit managed onboarding set입니다.
	FCFContentCutoverManifest Manifest;
};

/** Frozen P0-07 contracts를 fail-closed sequencing으로 조합합니다. */
class FCFContentCutoverCoordinator
{
public:
	// USER가 선택한 exact keys를 approval-bindable managed manifest로 동결합니다.
	static bool BuildManifest(
		const FString& ManifestId,
		const FCFContentReviewPackage& ReviewPackage,
		const FCFContentCatalogSnapshot& BaseSnapshot,
		const TArray<FCFContentKey>& SelectedContentKeys,
		const FCFContentCutoverProductRegistry& ProductRegistry,
		FCFContentCutoverManifest& OutManifest,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError);

	// Provenance readback과 current Product를 비교해 direct Product drift를 fail-visible하게 찾습니다.
	static bool DetectDirectProductDrift(
		const FCFContentProvenanceSnapshot& Snapshot,
		const FCFContentCutoverProductRegistry& ProductRegistry,
		TArray<FCFContentKey>& OutDriftedKeys,
		FString& OutError);

	// Staged Workbook validation -> fresh stale guard -> reviewed Product apply -> Workbook commit -> provenance까지 실행합니다.
	static bool Execute(
		const FCFContentCutoverRequest& Request,
		ICFContentWorkbookAdapter& WorkbookAdapter,
		const FCFContentProviderRegistry& ProviderRegistry,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		const FCFContentCutoverProductRegistry& ProductRegistry,
		ICFContentCutoverStore& Store,
		FCFContentCutoverTransaction& OutTransaction);

	// RecoveryRequired transaction에서 Product를 다시 쓰지 않고 exact staged candidate finalize만 재개합니다.
	static bool ResumeRecovery(
		const FCFContentCutoverRequest& Request,
		ICFContentWorkbookAdapter& WorkbookAdapter,
		const FCFContentCutoverProductRegistry& ProductRegistry,
		ICFContentCutoverStore& Store,
		FCFContentCutoverTransaction& OutTransaction);
};
