// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageProvider.h
// Version: v1.3.0
// Date: 2026-09-11
// Description: CF-FQ-052 DamageData exact12 reviewed durable authoring + ContractReady DACE provider 계약입니다.
// Changelog:
// - v1.3.0: DDO-P0-04에서 provider-local capability/DACE/canonical exact0은 그대로 유지한 채 production mixed operational admission이 외부 CFDAStagingOps explicit exact3 policy로 활성화됐음을 문서 projection에 반영했습니다.
// - v1.2.0: dedicated CFDADamageDace descriptor/history candidate와 함께 DACE readiness를 ContractReady로 전환했습니다. Reviewed mutation capability와 mixed operational admission exact2 경계는 유지합니다.
// - v1.1.0: exact12 materializer와 Reviewed mutation callback을 추가해 shared durable core를 사용하는 Damage ReviewedMutationReady seam을 개방했습니다.
// - v1.0.0: Damage exact12 typed payload/record, strict parse/serialize, semantic fingerprint, read-only extractor/current resolver와 ReadOnlyPreviewReady provider seam을 추가.
// Migration:
// - Editor Private internal contract입니다. UCFDamageData runtime/Blueprint/DataAsset 계약과 persisted Product Damage asset은 변경하지 않습니다.
// - DDO-P0-04부터 production mixed operational admission은 CFDAStagingOps explicit MissileGuidePreset+AmmoData+DamageData exact3 policy가 소유합니다. 이 provider의 canonical Product Damage DACE target은 계속 explicit exact0입니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "CFDATypeDispatch.h"

class UCFDamageData;

// UCFDamageData exact12 authored whole-record semantic payload입니다.
struct FCFDADamagePayload
{
	// UCFDamageData의 required stable identity입니다.
	FName DamageId = NAME_None;

	// Authored damage category입니다.
	ECFDamageType DamageType = ECFDamageType::Kinetic;

	// Clamp하지 않은 authored direct base damage입니다.
	float BaseDamage = 0.0f;

	// Authored self-damage permission입니다.
	bool bCanDamageSelf = false;

	// Runtime armor calculation이 소비하는 authored armor penetration입니다.
	float ArmorPenetration = 0.0f;

	// Authored radial-damage enable flag입니다.
	bool bUseRadialDamage = false;

	// Authored explosion outer radius입니다.
	float ExplosionRadius = 0.0f;

	// Authored full-damage inner radius입니다.
	float ExplosionInnerRadius = 0.0f;

	// Authored explosion damage입니다.
	float ExplosionDamage = 0.0f;

	// Authored minimum explosion damage scale입니다.
	float MinExplosionDamageScale = 0.0f;

	// Authored module damage multiplier입니다.
	float ModuleDamageScale = 0.0f;

	// Authored physical impulse strength입니다.
	float ImpulseStrength = 0.0f;
};

// Damage typed parser가 provider-local stack에서만 보유하는 whole-record입니다.
struct FCFDADamageRecord
{
	// Damage schema family identity입니다.
	FString SchemaId;

	// Damage JSON shape revision입니다.
	int32 SchemaRevision = 0;

	// Damage typed semantic adapter revision입니다.
	int32 AdapterContractRevision = 0;

	// Exact UCFDamageData native class path입니다.
	FString DataAssetTypeClassPath;

	// DamageId와 FName semantic으로 일치해야 하는 stable logical identity입니다.
	FName StableLogicalId = NAME_None;

	// Exact `/Game/.../Asset.Asset` target object path입니다.
	FString TargetObjectPath;

	// Update baseline 존재 여부입니다.
	bool bHasBaseSemanticFingerprint = false;

	// Update record가 캡처한 persisted semantic baseline입니다.
	FString BaseSemanticFingerprint;

	// Damage exact12 typed authored payload입니다.
	FCFDADamagePayload Payload;

	// Typed payload에서 계산한 desired-state semantic fingerprint입니다.
	FString StagingSemanticFingerprint;

	// Provider-owned canonical Staging JSON source path입니다.
	FString StagingRelativePath;
};

// Damage strict JSON parse 결과와 stable diagnostics입니다.
struct FCFDADamageParseResult
{
	// Whole-record schema/value validation이 모두 통과했는지 나타냅니다.
	bool bValid = false;

	// Parse 성공 시 사용할 provider-local typed record입니다.
	FCFDADamageRecord Record;

	// Parse/validation diagnostic 목록입니다.
	TArray<FCFDAStagingIssue> Issues;
};

// Reviewed mutation-capable common provider entry와 Damage 전용 typed contract authority를 함께 소유합니다.
struct FCFDADamageTypeProvider : public FCFDATypeProviderEntry
{
};

namespace CFDADamageProvider
{
	// Current DamageData ReviewedMutationReady trusted provider authority를 반환합니다.
	const FCFDADamageTypeProvider& GetProvider();
}

namespace CFDADamageProviderImpl
{
	// Damage typed record를 payload-free shared envelope로 투영합니다.
	FCFDACommonEnvelope BuildCommonEnvelope(
		const FCFDADamageRecord& Record,
		const FString& CurrentSemanticFingerprint = FString(),
		ECFDAStagingPreviewKind PlannedOperation = ECFDAStagingPreviewKind::Invalid);

	// Raw JSON을 strict Damage typed parser까지 통과시킨 뒤 common candidate만 반환합니다.
	bool ParseCommonCandidate(
		const FString& JsonText,
		const FString& StagingRelativePath,
		FCFDACommonEnvelope& OutEnvelope,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Payload-free Damage candidate를 exact UCFDamageData current truth로 read-only 해석합니다.
	bool ResolveCommonCurrentState(
		const FCFDACommonEnvelope& Envelope,
		FCFDACommonCurrentState& OutCurrentState,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Strict whole-record Damage JSON을 provider-local typed record로 parse합니다.
	FCFDADamageParseResult ParseJson(
		const FString& JsonText,
		const FString& StagingRelativePath = FString());

	// Damage exact12 typed payload를 current schema whole-record JSON으로 deterministic 직렬화합니다. 빈 BaseSemanticFingerprint는 JSON null을 뜻합니다.
	bool SerializeStagingJson(
		const FCFDADamagePayload& Payload,
		const FString& TargetObjectPath,
		const FString& BaseSemanticFingerprint,
		FString& OutJsonText,
		FString& OutError);

	// Damage exact12 typed payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
	bool BuildSemanticFingerprint(
		const FCFDADamagePayload& Payload,
		FString& OutFingerprint,
		FString& OutError);

	// Exact UCFDamageData UObject를 authored whole-record payload로 read-only 추출합니다.
	bool ExtractPayload(
		const UCFDamageData& DamageAsset,
		FCFDADamagePayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Mutable provider-local record의 payload와 cached fingerprint가 exact 일치하는지 검증합니다.
	bool ValidateRecordIntegrity(
		const FCFDADamageRecord& Record,
		FString& OutError);

	// Damage exact12 typed payload를 exact UCFDamageData UObject에 deterministic whole-record로 materialize합니다.
	void MaterializePayload(
		UCFDamageData& TargetAsset,
		const FCFDADamagePayload& Payload);

	// Fresh reviewed JSON을 재parse/rebind한 뒤 shared durable core로 exact Damage Create/Update를 실행합니다.
	void ApplyReviewedMutation(
		const FString& JsonText,
		const FCFDACommonPreviewRow& FreshRow,
		FCFDAStagingTargetApplyReport& OutTargetReport);
}
