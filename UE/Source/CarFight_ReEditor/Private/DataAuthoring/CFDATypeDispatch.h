// Copyright (c) CarFight. All Rights Reserved.
// File: CFDATypeDispatch.h
// Version: v1.10.0
// Date: 2026-09-11
// Description: CF-FQ-053 payload-free shared orchestration, exact4 provider registry readiness와 per-TypeKey DACE boundary 계약입니다.
// Changelog:
// - v1.10.0: VDR-P0-02 VehicleDefenseData independent DACE bootstrap acceptance를 반영해 exact4 provider가 모두 ContractReady이며 mixed operational admission도 explicit exact4로 전진했습니다. shared DTO/API는 변경하지 않습니다.
// - v1.9.0: VDR-P0-01 VehicleDefenseData ReviewedMutationReady fourth provider를 production registry에 추가했습니다. DACE는 ContractNotReady이며 mixed operational admission은 기존 exact3를 유지했습니다.
// - v1.8.0: DamageData provider의 ReviewedMutationReady 전진을 반영했습니다. shared DTO/API와 당시 mixed operational admission exact2 계약은 변경하지 않습니다.
// - v1.7.0: DamageData ReadOnlyPreviewReady third provider registration을 반영했습니다. shared DTO/API와 mixed operational admission 계약은 변경하지 않습니다.
// - v1.6.0: DAO-P0-05에서 JSON read 전 provider-owned CanonicalStagingRoot containment로 exact Staging path owner를 하나만 결정하는 provider-neutral resolver와 test-owned provider-set seam을 추가했습니다.
// - v1.5.0: DAO-P0-04 correction에서 authoring readiness와 독립된 DACE readiness, explicit canonical Staging target-set authority를 exact TypeKey provider descriptor에 추가했습니다.
// - v1.4.0: DAO-P0-03에서 AmmoData durable writer가 등록되며 production exact2 provider가 ReviewedMutationReady로 공존하는 current contract를 반영.
// - v1.3.0: ReadOnlyPreviewReady / ReviewedMutationReady capability를 분리하고 mutation readiness fail-closed validation seam을 추가.
// - v1.2.0: payload-free common Preview set을 deterministic Reviewed approval evidence로 동결하는 shared Review contract를 추가.
// - v1.1.0: Public Missile record 의존을 shared DTO에서 제거하고 common current/preview row, complete provider operation entry, stable identity policy/resolver, DACE owner/history namespace, duplicate TypeKey fail-closed registry 계약을 추가.
// - v1.0.0: MissileGuidePreset trusted provider, common envelope, provider-owned StagingRoot, class-scoped StableLogicalId key와 exact contract validation을 최초 추가.
// Migration:
// - 기존 FCFDAStagingRecord/FCFDAStagingService Public Missile API는 변경하지 않습니다. typed payload는 provider-local stack에서만 해석하고 shared Review/TOCTOU/Batch lifecycle에는 payload-free common row만 전달합니다.
// - read-only provider는 Parse/Current operation만으로 registry에 등록할 수 있지만 Reviewed Apply 진입은 capability 검증에서 차단됩니다. 현재 MissileGuidePreset, AmmoData, DamageData, VehicleDefenseData exact4 provider는 ReviewedMutationReady + DACE ContractReady이며 mixed operational admission은 네 TypeKey explicit exact4입니다.
// - v1.5.0부터 DACE readiness는 authoring readiness와 별도입니다. canonical target set은 empty라도 explicit declared 상태여야 하며, empty set은 현재 canonical Product target exact0을 의미합니다.
// - v1.6.0부터 mixed operational selection은 payload를 읽기 전에 exact path owner를 먼저 확정해야 하며 owner0/owner>1/allowed TypeKey scope mismatch는 fail-closed합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFDAStaging.h"

struct FCFDAStagingTargetApplyReport;
struct FCFDAStagingReviewedApproval;

// SchemaId + exact native DataAsset class path로 구성되는 trusted provider lookup key입니다.
struct FCFDATypeKey
{
	// Provider가 소유하는 exact schema identity입니다.
	FString SchemaId;

	// Provider가 소유하는 exact native DataAsset class path입니다.
	FString DataAssetTypeClassPath;
};

// Provider가 StableLogicalId를 필수로 요구하는지 나타내는 내부 정책입니다.
enum class ECFDAStableIdentityPolicy : uint8
{
	Required
};

// Provider가 exact FName source field를 StableLogicalId authority로 사용하는 내부 resolver 종류입니다.
enum class ECFDAStableIdentityResolver : uint8
{
	ExplicitFName
};

// Provider가 현재 production registry에서 허용하는 operation readiness 단계입니다.
enum class ECFDAProviderReadiness : uint8
{
	// Parse + current-state + Preview까지만 허용하며 mutation callback을 보유하지 않습니다.
	ReadOnlyPreviewReady,

	// Reviewed Apply까지 허용하며 mutation callback을 반드시 보유합니다.
	ReviewedMutationReady
};

// Authoring readiness와 독립된 per-TypeKey DACE contract readiness입니다.
enum class ECFDADaceReadiness : uint8
{
	// TypeKey/history namespace/target-set 경계만 예약됐고 accepted DACE contract는 아직 없습니다.
	ContractNotReady,

	// 해당 TypeKey의 accepted DACE contract/history가 production authority로 준비됐습니다.
	ContractReady
};

// Payload를 보유하지 않는 shared orchestration envelope입니다.
struct FCFDACommonEnvelope
{
	// Staging schema identity입니다.
	FString SchemaId;

	// Staging schema shape revision입니다.
	int32 SchemaRevision = 0;

	// Typed adapter semantic contract revision입니다.
	int32 AdapterContractRevision = 0;

	// Exact native DataAsset class path입니다.
	FString DataAssetTypeClassPath;

	// Type-local stable logical identity입니다.
	FName StableLogicalId = NAME_None;

	// Desired Unreal object path입니다.
	FString TargetObjectPath;

	// Provider-owned canonical Staging source path입니다.
	FString StagingRelativePath;

	// Update baseline 존재 여부입니다.
	bool bHasBaseSemanticFingerprint = false;

	// Update baseline semantic fingerprint입니다.
	FString BaseSemanticFingerprint;

	// Preview 시점 current semantic fingerprint입니다.
	FString CurrentSemanticFingerprint;

	// Desired Staging semantic fingerprint입니다.
	FString StagingSemanticFingerprint;

	// Approval에 결합되는 planned operation입니다.
	ECFDAStagingPreviewKind PlannedOperation = ECFDAStagingPreviewKind::Invalid;
};

// Provider current-state resolver가 shared orchestration에 반환하는 payload-free current truth입니다.
struct FCFDACommonCurrentState
{
	// Requested exact target이 존재하는지 나타냅니다.
	bool bRequestedTargetExists = false;

	// Requested target package에 pre-existing unowned dirty가 있는지 나타냅니다.
	bool bRequestedTargetDirty = false;

	// Requested target의 exact concrete class path입니다.
	FString RequestedTargetClassPath;

	// Requested target의 provider-owned stable logical identity입니다.
	FName RequestedTargetStableLogicalId = NAME_None;

	// Requested target whole-record current semantic fingerprint입니다.
	FString CurrentSemanticFingerprint;

	// Same class-scoped StableLogicalId가 current truth에 존재하는지 나타냅니다.
	bool bStableIdentityExists = false;

	// Same class-scoped StableLogicalId의 current match count입니다.
	int32 StableIdentityMatchCount = 0;

	// Same class-scoped StableLogicalId가 exact1일 때 canonical current object path입니다.
	FString StableIdentityObjectPath;
};

// Shared Preview/Review/TOCTOU가 보유하는 payload-free common row입니다.
struct FCFDACommonPreviewRow
{
	// Shared Preview 최종 분류입니다.
	ECFDAStagingPreviewKind Kind = ECFDAStagingPreviewKind::Invalid;

	// Provider typed payload를 제외한 immutable candidate envelope입니다.
	FCFDACommonEnvelope Envelope;

	// Blocking 또는 informational shared diagnostic입니다.
	TArray<FCFDAStagingIssue> Issues;
};

// 한 DataAsset type의 trusted structural authority입니다. Typed payload 자체는 보유하지 않습니다.
struct FCFDATypeProvider
{
	// Exact provider lookup key입니다.
	FCFDATypeKey TypeKey;

	// Provider가 허용하는 exact schema revision입니다.
	int32 SchemaRevision = 0;

	// Provider가 허용하는 exact typed adapter revision입니다.
	int32 AdapterContractRevision = 0;

	// Provider가 단독 소유하는 main_game-relative canonical Staging root입니다.
	FString CanonicalStagingRoot;

	// StableLogicalId required/optional 의미를 소유하는 provider policy입니다.
	ECFDAStableIdentityPolicy StableIdentityPolicy = ECFDAStableIdentityPolicy::Required;

	// StableLogicalId를 읽는 exact resolver 의미입니다.
	ECFDAStableIdentityResolver StableIdentityResolver = ECFDAStableIdentityResolver::ExplicitFName;

	// Provider exact class 안에서 StableLogicalId를 읽는 source field 이름입니다.
	FName StableIdentitySourceName = NAME_None;

	// DACE contract descriptor/probe authority owner 이름입니다.
	FName DaceContractOwnerName = NAME_None;

	// DACE accepted snapshot chain이 사용하는 provider-local history namespace입니다.
	FString DaceAcceptedHistoryNamespace;

	// Authoring readiness와 독립된 이 exact TypeKey의 DACE contract 준비 상태입니다.
	ECFDADaceReadiness DaceReadiness = ECFDADaceReadiness::ContractNotReady;

	// Empty target set과 미선언 상태를 구분하는 explicit canonical Staging target-set declaration입니다.
	bool bDaceCanonicalStagingTargetSetDeclared = false;

	// 이 exact TypeKey가 migration compatibility 대상으로 소유하는 main_game-relative canonical Staging JSON 집합입니다.
	TArray<FString> DaceCanonicalStagingRelativePaths;
};

// Shared core가 exact TypeKey lookup 뒤 호출할 수 있는 payload-free provider operation table입니다.
struct FCFDATypeProviderOperations
{
	// Raw Staging JSON을 provider-local typed parser/integrity/reference validation까지 통과시킨 뒤 payload-free common candidate만 반환합니다.
	bool (*ParseCommonCandidate)(const FString&, const FString&, FCFDACommonEnvelope&, TArray<FCFDAStagingIssue>&) = nullptr;

	// Common candidate identity/target을 provider exact-class current truth로 해석합니다.
	bool (*ResolveCommonCurrentState)(const FCFDACommonEnvelope&, FCFDACommonCurrentState&, TArray<FCFDAStagingIssue>&) = nullptr;

	// Mutation 직전 전달된 fresh JSON을 provider-local typed payload로 다시 parse한 뒤 exact materialize/save를 수행합니다.
	void (*ApplyReviewedMutation)(const FString&, const FCFDACommonPreviewRow&, FCFDAStagingTargetApplyReport&) = nullptr;
};

// Descriptor와 shared-core operation entry를 한 authority로 묶는 exact provider registration입니다.
struct FCFDATypeProviderEntry
{
	// Provider structural/identity/DACE authority입니다.
	FCFDATypeProvider Descriptor;

	// Provider가 read-only Preview 또는 Reviewed mutation 중 어느 단계까지 준비됐는지 나타냅니다.
	ECFDAProviderReadiness Readiness = ECFDAProviderReadiness::ReadOnlyPreviewReady;

	// Shared orchestration이 사용할 provider operation table입니다.
	FCFDATypeProviderOperations Operations;
};

namespace CFDATypeDispatch
{
	// Current MissileGuidePreset complete trusted provider entry를 반환합니다.
	const FCFDATypeProviderEntry& GetMissilePresetProviderEntry();

	// Current MissileGuidePreset trusted provider descriptor compatibility view를 반환합니다.
	const FCFDATypeProvider& GetMissilePresetProvider();

	// Production provider registry가 duplicate TypeKey와 readiness-incomplete authority를 포함하지 않는지 fail-closed 검증합니다.
	bool ValidateProviderRegistry(FString& OutError);

	// Exact SchemaId + DataAssetTypeClassPath TypeKey에 등록된 readiness-valid provider entry만 반환합니다.
	const FCFDATypeProviderEntry* FindExactProviderEntry(
		const FString& SchemaId,
		const FString& DataAssetTypeClassPath,
		FString* OutError = nullptr);

	// Reviewed Apply 진입 전 selected provider가 mutation-ready인지 fail-closed 확인합니다.
	bool ValidateProviderMutationReady(
		const FCFDATypeProviderEntry& ProviderEntry,
		FString& OutError);

	// Common envelope의 exact TypeKey/revision/StagingRoot가 trusted provider 계약과 일치하는지 검사합니다.
	bool ValidateProviderContract(
		const FCFDACommonEnvelope& Envelope,
		const FCFDATypeProvider& Provider,
		FString& OutError);

	// Provider-owned StagingRoot 아래의 canonical lowercase .json source path로 정규화합니다.
	bool NormalizeProviderStagingPath(
		const FCFDATypeProvider& Provider,
		const FString& InputPath,
		FString& OutPath);

	// Explicit allowed TypeKey scope 안에서 exact Staging path owner를 JSON read 전에 production registry 기준으로 하나만 확정합니다.
	const FCFDATypeProviderEntry* FindProviderForStagingPath(
		const FString& InputPath,
		const TArray<FCFDATypeKey>& AllowedTypeKeys,
		FString& OutNormalizedPath,
		FString* OutError = nullptr);

	// StableLogicalId duplicate scope를 exact DataAsset class + canonical FName semantic으로 만듭니다.
	FString BuildClassScopedStableIdentityKey(
		const FString& DataAssetTypeClassPath,
		FName StableLogicalId);

	// Payload-free common candidate/current truth를 exact 3-way Preview state machine으로 분류합니다.
	FCFDACommonPreviewRow BuildCommonPreview(
		const FCFDACommonEnvelope& Envelope,
		const FCFDACommonCurrentState& CurrentState);

	// Same-class StableLogicalId와 global TargetObjectPath duplicate를 payload-free rows에서 fail-closed합니다.
	void ApplyCommonBatchDuplicateValidation(TArray<FCFDACommonPreviewRow>& InOutRows);

	// Create/Update payload-free common rows의 deterministic BatchPlanHash를 계산합니다.
	bool BuildCommonBatchPlanHash(
		const TArray<FCFDACommonPreviewRow>& PreviewRows,
		FString& OutBatchPlanHash,
		FString& OutError);

	// Payload-free common Preview set을 deterministic Reviewed approval evidence로 동결합니다.
	bool BuildCommonReviewedApproval(
		const TArray<FCFDACommonPreviewRow>& PreviewRows,
		FCFDAStagingReviewedApproval& OutApproval,
		FString& OutError);

	// Approval/TOCTOU가 두 common envelope의 exact immutable binding을 비교합니다.
	bool AreCommonEnvelopesEquivalent(
		const FCFDACommonEnvelope& Left,
		const FCFDACommonEnvelope& Right);

#if WITH_DEV_AUTOMATION_TESTS
	// Test-owned explicit provider set에서 duplicate TypeKey/readiness-incomplete provider를 production registry mutation 없이 검증합니다.
	bool ValidateProviderSetForTests(
		const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
		FString& OutError);

	// Test-owned explicit provider set에서 exact TypeKey lookup seam을 production registry mutation 없이 검증합니다.
	const FCFDATypeProviderEntry* FindExactProviderEntryInSetForTests(
		const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
		const FString& SchemaId,
		const FString& DataAssetTypeClassPath,
		FString* OutError = nullptr);

	// Test-owned explicit provider set에서 overlapping root를 production registry mutation 없이 검증하는 exact path-owner seam입니다.
	const FCFDATypeProviderEntry* FindProviderForStagingPathInSetForTests(
		const TArray<const FCFDATypeProviderEntry*>& ProviderEntries,
		const FString& InputPath,
		const TArray<FCFDATypeKey>& AllowedTypeKeys,
		FString& OutNormalizedPath,
		FString* OutError = nullptr);
#endif
}
