// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentDurable.h
// Version: v1.1.0
// Date: 2026-09-17
// Description: CF-FQ-054 EquipmentPreset durable semantic/Review adapter입니다.
// Changelog:
// - v1.1.0: 기존 localized DisplayName을 preserve-only semantic으로 안전하게 유지하는 Contract Revision 2를 추가.
// - v1.0.0: exact7 semantic snapshot/fingerprint, ReviewProposalDigest, child persisted-clean guard와 CFDADurableCore callback adapter를 추가.
// Migration:
// - Editor Private helper이며 Runtime schema/Public API를 변경하지 않습니다.
// - 기존 localized DisplayName은 Equipment Builder가 silent literal 변환하지 않고 그대로 보존합니다.
// - SavePackage/CreatePackage/NewObject sequencing은 이 helper가 소유하지 않고 CFDADurableCore만 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFDAStaging.h"
#include "EquipmentAuthoring/CFEquipmentBuilderTypes.h"

class UCFEquipmentPresetData;

// EquipmentPreset exact1 저작 의미를 shared DataAuthoring durable primitives에 연결하는 Editor-only adapter입니다.
class FCFEquipmentDurable
{
public:
	// EquipmentPreset exact native class path를 반환합니다.
	static FString GetTargetClassPath();

	// Equipment Authoring durable contract revision을 반환합니다.
	static int32 GetContractRevision();

	// Create current-state binding에 사용하는 stable sentinel을 반환합니다.
	static const TCHAR* GetAbsentTargetToken();

	// Reviewed validation terminal state의 stable token을 반환합니다.
	static const TCHAR* GetReadyValidationToken();

	// localized DisplayName preserve-mode fingerprint에 사용하는 stable sentinel을 반환합니다.
	static const TCHAR* GetPreservedDisplayNameToken();

	// 현재 FText를 Equipment Builder가 lossless literal로 직접 편집할 수 있는지 확인합니다.
	static bool CanEditDisplayNameLosslessly(const FText& DisplayName);

	// transient Draft exact7을 lossless/preserve-safe durable semantic snapshot으로 정규화합니다.
	static bool BuildSnapshotFromDraft(
		const FCFEquipmentPresetDraft& Draft,
		FCFEquipmentSemanticSnapshot& OutSnapshot,
		FString& OutError);

	// persisted EquipmentPreset exact7을 lossless/preserve-safe durable semantic snapshot으로 추출합니다.
	static bool ExtractSnapshotFromAsset(
		const UCFEquipmentPresetData& Asset,
		FCFEquipmentSemanticSnapshot& OutSnapshot,
		TArray<FCFDAStagingIssue>& OutIssues);

	// exact7 semantic snapshot을 deterministic SHA-256 fingerprint로 변환합니다.
	static bool BuildSemanticFingerprint(
		const FCFEquipmentSemanticSnapshot& Snapshot,
		FString& OutFingerprint,
		FString& OutError);

	// desired exact7 snapshot을 exact EquipmentPreset UObject에 materialize합니다.
	static void MaterializeSnapshot(
		UCFEquipmentPresetData& Asset,
		const FCFEquipmentSemanticSnapshot& Snapshot);

	// Review/Commit에 사용되는 child dependency가 persisted exact-class clean truth인지 fail-closed 검증합니다.
	static bool ValidatePersistedChildDependencies(
		const FCFEquipmentPresetDraft& Draft,
		const FString& ForcedUnconfirmedObjectPath,
		FString& OutError);

	// current contract ReviewProposalDigest를 deterministic SHA-256으로 계산합니다.
	static bool BuildReviewProposalDigest(
		ECFEquipmentReviewOperation Operation,
		const FString& TargetObjectPath,
		const FString& TargetClassPath,
		const FString& CurrentSemanticFingerprint,
		const FString& ProspectiveSemanticFingerprint,
		int32 ContractRevision,
		const FString& ValidationState,
		FString& OutDigest,
		FString& OutError);
};
