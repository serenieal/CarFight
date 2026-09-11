// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseProvider.h
// Version: v1.1.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-01 VehicleDefenseData reviewed durable authoring provider 계약입니다.
// Changelog:
// - v1.1.0: VDR-P0-02 independent DACE bootstrap acceptance 뒤 VehicleDefense provider의 DACE readiness를 ContractReady로 승격했습니다. typed authoring API는 변경하지 않습니다.
// - v1.0.0: VehicleDefenseData exact17/top-level + nested armor exact12 typed payload, strict parse/serialize/fingerprint, current resolver와 Reviewed mutation provider seam을 추가했습니다.
// Migration:
// - Editor Private internal contract입니다. UCFVehicleDefenseData runtime/Blueprint/DataAsset 계약과 persisted Product asset은 변경하지 않습니다.
// - v1.1.0부터 DACE는 ContractReady입니다. mixed operational admission은 CFDAStagingOps의 explicit allowlist가 별도 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageRuntimeTypes.h"
#include "CFDATypeDispatch.h"

class UCFVehicleDefenseData;

// UCFVehicleDefenseData authored whole-record semantic payload입니다.
struct FCFDAVehicleDefensePayload
{
	// UCFVehicleDefenseData의 required stable identity입니다.
	FName DefenseId = NAME_None;

	// Clamp하지 않은 authored defense package mass입니다.
	float DefenseMassKg = 0.0f;

	// Authored shield enable flag입니다.
	bool bUseShield = false;

	// Clamp하지 않은 authored maximum shield입니다.
	float MaximumShield = 0.0f;

	// Clamp하지 않은 authored shield regeneration delay입니다.
	float ShieldRegenerationDelaySeconds = 0.0f;

	// Clamp하지 않은 authored shield regeneration rate입니다.
	float ShieldRegenerationPerSecond = 0.0f;

	// Authored armor display/category type입니다.
	ECFArmorType ArmorType = ECFArmorType::Standard;

	// Clamp하지 않은 authored armor resistance입니다.
	float ArmorResistance = 0.0f;

	// Clamp하지 않은 authored front armor config입니다.
	FCFDirectionalArmorConfig FrontArmorConfig;

	// Clamp하지 않은 authored left armor config입니다.
	FCFDirectionalArmorConfig LeftArmorConfig;

	// Clamp하지 않은 authored right armor config입니다.
	FCFDirectionalArmorConfig RightArmorConfig;

	// Clamp하지 않은 authored rear armor config입니다.
	FCFDirectionalArmorConfig RearArmorConfig;

	// Clamp하지 않은 authored top armor config입니다.
	FCFDirectionalArmorConfig TopArmorConfig;

	// Clamp하지 않은 authored bottom armor config입니다.
	FCFDirectionalArmorConfig BottomArmorConfig;

	// Authored shield-layer component damage scale입니다.
	float ShieldComponentDamageScale = 0.0f;

	// Authored armor-layer component damage scale입니다.
	float ArmorComponentDamageScale = 0.0f;

	// Authored integrity-layer component damage scale입니다.
	float IntegrityComponentDamageScale = 0.0f;
};

// VehicleDefense typed parser가 provider-local stack에서만 보유하는 whole-record입니다.
struct FCFDAVehicleDefenseRecord
{
	// VehicleDefense schema family identity입니다.
	FString SchemaId;

	// VehicleDefense JSON shape revision입니다.
	int32 SchemaRevision = 0;

	// VehicleDefense typed semantic adapter revision입니다.
	int32 AdapterContractRevision = 0;

	// Exact UCFVehicleDefenseData native class path입니다.
	FString DataAssetTypeClassPath;

	// DefenseId와 FName semantic으로 일치해야 하는 stable logical identity입니다.
	FName StableLogicalId = NAME_None;

	// Exact `/Game/.../Asset.Asset` target object path입니다.
	FString TargetObjectPath;

	// Update baseline 존재 여부입니다.
	bool bHasBaseSemanticFingerprint = false;

	// Update record가 캡처한 persisted semantic baseline입니다.
	FString BaseSemanticFingerprint;

	// VehicleDefense typed authored payload입니다.
	FCFDAVehicleDefensePayload Payload;

	// Typed payload에서 계산한 desired-state semantic fingerprint입니다.
	FString StagingSemanticFingerprint;

	// Provider-owned canonical Staging JSON source path입니다.
	FString StagingRelativePath;
};

// VehicleDefense strict JSON parse 결과와 stable diagnostics입니다.
struct FCFDAVehicleDefenseParseResult
{
	// Whole-record schema/value validation이 모두 통과했는지 나타냅니다.
	bool bValid = false;

	// Parse 성공 시 사용할 provider-local typed record입니다.
	FCFDAVehicleDefenseRecord Record;

	// Parse/validation diagnostic 목록입니다.
	TArray<FCFDAStagingIssue> Issues;
};

// Reviewed mutation-capable common provider entry와 VehicleDefense typed authority를 함께 소유합니다.
struct FCFDAVehicleDefenseProvider : public FCFDATypeProviderEntry
{
};

namespace CFDAVehicleDefenseProvider
{
	// Current VehicleDefenseData ReviewedMutationReady trusted provider authority를 반환합니다.
	const FCFDAVehicleDefenseProvider& GetProvider();
}

namespace CFDAVehicleDefenseProviderImpl
{
	// VehicleDefense typed record를 payload-free shared envelope로 투영합니다.
	FCFDACommonEnvelope BuildCommonEnvelope(
		const FCFDAVehicleDefenseRecord& Record,
		const FString& CurrentSemanticFingerprint = FString(),
		ECFDAStagingPreviewKind PlannedOperation = ECFDAStagingPreviewKind::Invalid);

	// Raw JSON을 strict VehicleDefense typed parser까지 통과시킨 뒤 common candidate만 반환합니다.
	bool ParseCommonCandidate(
		const FString& JsonText,
		const FString& StagingRelativePath,
		FCFDACommonEnvelope& OutEnvelope,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Payload-free VehicleDefense candidate를 exact UCFVehicleDefenseData current truth로 read-only 해석합니다.
	bool ResolveCommonCurrentState(
		const FCFDACommonEnvelope& Envelope,
		FCFDACommonCurrentState& OutCurrentState,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Strict whole-record VehicleDefense JSON을 provider-local typed record로 parse합니다.
	FCFDAVehicleDefenseParseResult ParseJson(
		const FString& JsonText,
		const FString& StagingRelativePath = FString());

	// VehicleDefense typed payload를 current schema whole-record JSON으로 deterministic 직렬화합니다. 빈 BaseSemanticFingerprint는 JSON null을 뜻합니다.
	bool SerializeStagingJson(
		const FCFDAVehicleDefensePayload& Payload,
		const FString& TargetObjectPath,
		const FString& BaseSemanticFingerprint,
		FString& OutJsonText,
		FString& OutError);

	// VehicleDefense typed payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
	bool BuildSemanticFingerprint(
		const FCFDAVehicleDefensePayload& Payload,
		FString& OutFingerprint,
		FString& OutError);

	// Exact UCFVehicleDefenseData UObject를 authored whole-record payload로 read-only 추출합니다.
	bool ExtractPayload(
		const UCFVehicleDefenseData& DefenseAsset,
		FCFDAVehicleDefensePayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Mutable provider-local record의 payload와 cached fingerprint가 exact 일치하는지 검증합니다.
	bool ValidateRecordIntegrity(
		const FCFDAVehicleDefenseRecord& Record,
		FString& OutError);

	// VehicleDefense typed payload를 exact UCFVehicleDefenseData UObject에 deterministic whole-record로 materialize합니다.
	void MaterializePayload(
		UCFVehicleDefenseData& TargetAsset,
		const FCFDAVehicleDefensePayload& Payload);

	// Fresh reviewed JSON을 재parse/rebind한 뒤 shared durable core로 exact VehicleDefense Create/Update를 실행합니다.
	void ApplyReviewedMutation(
		const FString& JsonText,
		const FCFDACommonPreviewRow& FreshRow,
		FCFDAStagingTargetApplyReport& OutTargetReport);
}
