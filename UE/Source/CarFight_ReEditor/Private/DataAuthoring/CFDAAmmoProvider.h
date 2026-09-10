// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoProvider.h
// Version: v1.2.0
// Date: 2026-09-10
// Description: CF-FQ-051 AmmoData typed authoring + DACE production probe provider 계약입니다.
// Changelog:
// - v1.2.0: DAO-P0-04 DACE가 실제 production path를 관측할 수 있도록 deterministic Ammo whole-record serializer를 추가했습니다. 파일 write 권한은 추가하지 않습니다.
// - v1.1.0: Ammo exact8 typed materializer와 Reviewed Apply provider callback을 추가하고 shared durable core 진입 계약을 정의.
// - v1.0.0: Ammo exact8 typed payload/record, strict parser, semantic fingerprint, extractor, current resolver와 ReadOnlyPreviewReady provider seam을 추가.
// Migration:
// - Editor Private internal contract입니다. Public DataAuthoring API, UCFAmmoData runtime contract와 Missile compatibility facade는 변경하지 않습니다.
// - v1.1.0부터 Ammo provider는 ReviewedMutationReady이며 durable sequencing은 provider-neutral CFDADurableCore를 사용합니다.
// - v1.2.0 serializer는 memory JSON 생성만 수행하며 Product exact0, persisted Ammo asset과 canonical Staging 파일을 수정하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDATypeDispatch.h"
#include "UObject/SoftObjectPath.h"

class UCFAmmoData;

// CFAmmoData exact8 authored whole-record semantic payload입니다.
struct FCFDAAmmoPayload
{
	// CFAmmoData의 required stable identity입니다.
	FName AmmoId = NAME_None;

	// 사용자 표시 이름의 Literal FText semantic입니다.
	FCFDAStagingLiteralText AmmoDisplayName;

	// 선택적 탄약 계열 분류 FName입니다.
	FName AmmoFamilyId = NAME_None;

	// clamp하지 않은 authored 탄약 단위 질량 kg입니다.
	float UnitMassKg = 0.0f;

	// set-like semantic으로 canonical 정렬되는 탄약 분류 태그입니다.
	TArray<FName> AmmoTags;

	// null 또는 canonical top-level SoftObjectPath인 선택적 아이콘 reference입니다.
	FSoftObjectPath AmmoIcon;

	// clamp하지 않은 authored 최대 적재 가능 탄약량입니다.
	int32 MaximumLoadableAmmoCount = 0;

	// authored 재보급 가능 정책입니다.
	bool bCanBeResupplied = true;
};

// Ammo typed parser가 provider-local stack에서만 보유하는 whole-record입니다.
struct FCFDAAmmoRecord
{
	// Ammo schema family identity입니다.
	FString SchemaId;

	// Ammo JSON shape revision입니다.
	int32 SchemaRevision = 0;

	// Ammo typed semantic adapter revision입니다.
	int32 AdapterContractRevision = 0;

	// Exact CFAmmoData native class path입니다.
	FString DataAssetTypeClassPath;

	// AmmoId와 FName semantic으로 일치해야 하는 stable logical identity입니다.
	FName StableLogicalId = NAME_None;

	// Exact `/Game/.../Asset.Asset` target object path입니다.
	FString TargetObjectPath;

	// Update baseline 존재 여부입니다.
	bool bHasBaseSemanticFingerprint = false;

	// Update record가 캡처한 persisted semantic baseline입니다.
	FString BaseSemanticFingerprint;

	// Ammo exact8 typed authored payload입니다.
	FCFDAAmmoPayload Payload;

	// typed payload에서 계산한 desired-state semantic fingerprint입니다.
	FString StagingSemanticFingerprint;

	// Provider-owned canonical Staging JSON source path입니다.
	FString StagingRelativePath;
};

// Ammo strict JSON parse 결과와 stable diagnostics입니다.
struct FCFDAAmmoParseResult
{
	// whole-record schema/value validation이 모두 통과했는지 나타냅니다.
	bool bValid = false;

	// parse 성공 시 사용할 provider-local typed record입니다.
	FCFDAAmmoRecord Record;

	// parse/validation diagnostic 목록입니다.
	TArray<FCFDAStagingIssue> Issues;
};

// Reviewed mutation-ready common provider entry와 Ammo 전용 typed contract authority를 함께 소유합니다.
struct FCFDAAmmoTypeProvider : public FCFDATypeProviderEntry
{
};

namespace CFDAAmmoProvider
{
	// Current AmmoData ReviewedMutationReady trusted provider authority를 반환합니다.
	const FCFDAAmmoTypeProvider& GetProvider();
}

namespace CFDAAmmoProviderImpl
{
	// Ammo typed record를 payload-free shared envelope로 투영합니다.
	FCFDACommonEnvelope BuildCommonEnvelope(
		const FCFDAAmmoRecord& Record,
		const FString& CurrentSemanticFingerprint = FString(),
		ECFDAStagingPreviewKind PlannedOperation = ECFDAStagingPreviewKind::Invalid);

	// Raw JSON을 strict Ammo typed parser + reference validation까지 통과시킨 뒤 common candidate만 반환합니다.
	bool ParseCommonCandidate(
		const FString& JsonText,
		const FString& StagingRelativePath,
		FCFDACommonEnvelope& OutEnvelope,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Payload-free Ammo candidate를 exact CFAmmoData current truth로 read-only 해석합니다.
	bool ResolveCommonCurrentState(
		const FCFDACommonEnvelope& Envelope,
		FCFDACommonCurrentState& OutCurrentState,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Strict whole-record Ammo JSON을 provider-local typed record로 parse/canonicalize합니다.
	FCFDAAmmoParseResult ParseJson(
		const FString& JsonText,
		const FString& StagingRelativePath = FString());

	// Ammo exact8 typed payload를 current schema whole-record JSON으로 deterministic 직렬화합니다. 빈 BaseSemanticFingerprint는 JSON null을 뜻합니다.
	bool SerializeStagingJson(
		const FCFDAAmmoPayload& Payload,
		const FString& TargetObjectPath,
		const FString& BaseSemanticFingerprint,
		FString& OutJsonText,
		FString& OutError);

	// Ammo exact8 typed payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
	bool BuildSemanticFingerprint(
		const FCFDAAmmoPayload& Payload,
		FString& OutFingerprint,
		FString& OutError);

	// Exact CFAmmoData UObject를 authored whole-record payload로 read-only 추출합니다.
	bool ExtractPayload(
		const UCFAmmoData& AmmoAsset,
		FCFDAAmmoPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Mutable provider-local record의 payload와 cached fingerprint가 exact 일치하는지 검증합니다.
	bool ValidateRecordIntegrity(
		const FCFDAAmmoRecord& Record,
		FString& OutError);

	// Ammo exact8 typed payload를 CFAmmoData UObject에 deterministic whole-record로 materialize합니다.
	void MaterializePayload(
		UCFAmmoData& TargetAsset,
		const FCFDAAmmoPayload& Payload);

	// Fresh reviewed JSON을 재parse/rebind한 뒤 shared durable core로 exact Ammo Create/Update를 실행합니다.
	void ApplyReviewedMutation(
		const FString& JsonText,
		const FCFDACommonPreviewRow& FreshRow,
		FCFDAStagingTargetApplyReport& OutTargetReport);
}
