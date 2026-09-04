// Copyright (c) CarFight. All Rights Reserved.
// File: CFDATypeAdapter.h
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-02C loaded UObject identity/validation typed adapter입니다.
// Changelog:
// - v1.1.0: operational evaluation failure를 DA Health와 분리하고 typed canonical duplicate identity를 생성.
// - v1.0.0: Registered semantic의 Stable ID resolve와 Native/Custom validation을 read-only로 연결.
// Migration:
// - Asset load/save/mutation은 소유하지 않습니다. Unregistered semantic은 policy non-authoritative로 처리합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAManagementTypes.h"

class FCFDATypeRegistry;
class UObject;

// 이미 load된 DataAsset UObject를 current Typed Semantic 계약으로 해석하는 Editor-only adapter입니다.
class CARFIGHT_REEDITOR_API FCFDATypeAdapter
{
public:
	// 이미 load된 UObject 한 건의 stable identity와 typed validation을 계산합니다.
	static FCFDALoadedAssetResult EvaluateLoadedAsset(
		const FCFDAAssetRecord& AssetRecord,
		UObject* LoadedObject,
		const FCFDATypeRegistry& TypeRegistry,
		uint64 InventoryGeneration);

private:
	// descriptor의 Identity Policy/Resolver에 따라 stable identity를 read-only로 해석합니다. adapter 계약 실패면 false입니다.
	static bool ResolveStableIdentity(
		UObject* LoadedObject,
		const FCFDASemanticDescriptor& Descriptor,
		FCFDALoadedAssetResult& InOutResult);

	// descriptor의 Validation Policy에 따라 existing validator를 read-only로 실행합니다. adapter 계약 실패면 false입니다.
	static bool RunTypedValidation(
		UObject* LoadedObject,
		const FCFDASemanticDescriptor& Descriptor,
		FCFDALoadedAssetResult& InOutResult);

	// Health 우선순위를 낮추지 않으면서 새 상태를 반영합니다.
	static void RaiseHealthState(
		ECFDAHealthState NewHealthState,
		FCFDALoadedAssetResult& InOutResult);

	// FText validation error 목록을 문자열 메시지로 추가합니다.
	static void AppendTextMessages(
		const TArray<FText>& ValidationMessages,
		FCFDALoadedAssetResult& InOutResult);
};
