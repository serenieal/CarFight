// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAMissileProvider.h
// Version: v1.2.0
// Date: 2026-09-09
// Description: CF-FQ-051 DAO-P0-01 MissileGuidePreset complete provider entry와 provider-local typed callback ownership을 Editor Private 경계에 고정합니다.
// Changelog:
// - v1.2.0: 기존 Public Missile Preview row의 typed mutable integrity를 provider-local에서 검증하고 payload-free common Review row로 투영하는 compatibility helper를 추가.
// - v1.1.0: FCFDATypeProviderEntry를 상속해 shared parse/current/apply operation을 등록하고 typed record→common projection을 provider-local 경계로 이동.
// - v1.0.0: 기존 Missile parse/fingerprint/extract/current-state/serialize/materialize production 구현을 exact typed provider callback table로 묶는 Private 계약을 추가.
// Migration:
// - Public FCFDAStagingRecord/FCFDAStagingService/FCFDAStagingOps/FCFDAStagingApplyService 시그니처는 유지합니다. Shared core는 common row만 보유하며 Missile typed payload는 이 provider 내부에서만 해석합니다. Ammo는 P0-02 이후 별도 provider로 추가합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDATypeDispatch.h"

class UCFMissileGuidePresetData;

// MissileGuidePreset typed behavior가 소유하는 production callback table입니다.
struct FCFDAMissileProviderCallbacks
{
	// Strict whole-record Missile JSON parser callback입니다.
	FCFDAStagingParseResult (*ParseJson)(const FString&, const FString&) = nullptr;

	// Missile typed payload semantic fingerprint callback입니다.
	bool (*BuildSemanticFingerprint)(const FCFDAMissilePresetPayload&, FString&, FString&) = nullptr;

	// Missile UObject → typed payload extractor callback입니다.
	bool (*ExtractPayload)(const UCFMissileGuidePresetData&, FCFDAMissilePresetPayload&, TArray<FCFDAStagingIssue>&) = nullptr;

	// Missile exact-class current-state resolver callback입니다.
	bool (*ResolveCurrentState)(const FCFDAStagingRecord&, FCFDAStagingCurrentState&, TArray<FCFDAStagingIssue>&) = nullptr;

	// Missile Product UObject → canonical Staging JSON serializer callback입니다.
	bool (*SerializeProductStagingJson)(const FCFDAMissilePresetPayload&, const FString&, const FString&, FString&, FString&) = nullptr;

	// Missile typed payload → exact UObject materializer callback입니다.
	void (*MaterializePayload)(UCFMissileGuidePresetData&, const FCFDAMissilePresetPayload&) = nullptr;
};

// Complete common provider entry와 Missile 전용 typed callbacks를 함께 소유하는 first provider입니다.
struct FCFDAMissileTypeProvider : public FCFDATypeProviderEntry
{
	// Missile typed implementation만 해석하는 callback table입니다.
	FCFDAMissileProviderCallbacks Callbacks;
};

namespace CFDAMissileProvider
{
	// Current MissileGuidePreset typed provider authority를 반환합니다.
	const FCFDAMissileTypeProvider& GetProvider();
}

namespace CFDAMissileProviderImpl
{
	// Existing Missile typed record를 payload-free common envelope로 투영하는 provider-local adapter입니다.
	FCFDACommonEnvelope BuildCommonEnvelope(
		const FCFDAStagingRecord& Record,
		const FString& CurrentSemanticFingerprint = FString(),
		ECFDAStagingPreviewKind PlannedOperation = ECFDAStagingPreviewKind::Invalid);

	// Existing Public Missile Preview row의 typed mutable integrity를 검증한 뒤 payload-free common row로 투영합니다.
	bool ProjectCompatibilityPreviewRow(
		const FCFDAStagingPreviewRow& PreviewRow,
		FCFDACommonPreviewRow& OutCommonRow,
		FString& OutError);

	// Raw JSON을 typed Missile parser로 검증한 뒤 payload-free common candidate만 shared core에 반환합니다.
	bool ParseCommonCandidate(
		const FString& JsonText,
		const FString& StagingRelativePath,
		FCFDACommonEnvelope& OutEnvelope,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Payload-free common candidate를 exact Missile current-state resolver로 해석합니다.
	bool ResolveCommonCurrentState(
		const FCFDACommonEnvelope& Envelope,
		FCFDACommonCurrentState& OutCurrentState,
		TArray<FCFDAStagingIssue>& OutIssues);

	// Fresh JSON을 provider-local typed payload로 재parse한 뒤 exact reviewed mutation을 수행합니다.
	void ApplyReviewedMutation(
		const FString& JsonText,
		const FCFDACommonPreviewRow& FreshRow,
		FCFDAStagingTargetApplyReport& OutTargetReport);

	// Existing strict Missile parser production body입니다.
	FCFDAStagingParseResult ParseJson(const FString& JsonText, const FString& StagingRelativePath);

	// Existing Missile semantic fingerprint production body입니다.
	bool BuildSemanticFingerprint(const FCFDAMissilePresetPayload& Payload, FString& OutFingerprint, FString& OutError);

	// Existing Missile UObject extractor production body입니다.
	bool ExtractPayload(const UCFMissileGuidePresetData& PresetAsset, FCFDAMissilePresetPayload& OutPayload, TArray<FCFDAStagingIssue>& OutIssues);

	// Existing Missile exact-class current-state resolver production body입니다.
	bool ResolveCurrentState(const FCFDAStagingRecord& Record, FCFDAStagingCurrentState& OutCurrentState, TArray<FCFDAStagingIssue>& OutIssues);

	// Existing Product→Staging serializer production body로 연결하는 typed bridge입니다.
	bool SerializeProductStagingJson(const FCFDAMissilePresetPayload& Payload, const FString& TargetObjectPath, const FString& BaseSemanticFingerprint, FString& OutJsonText, FString& OutError);

	// Existing Missile typed materializer production body로 연결하는 typed bridge입니다.
	void MaterializePayload(UCFMissileGuidePresetData& TargetAsset, const FCFDAMissilePresetPayload& Payload);
}
