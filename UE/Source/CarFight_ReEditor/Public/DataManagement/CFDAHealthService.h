// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAHealthService.h
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-02C inventory-bound explicit loaded health/duplicate service입니다.
// Changelog:
// - v1.1.0: Inventory snapshot authority, complete namespace duplicate closure, typed canonical equality와 evaluation/Health 분리를 추가.
// - v1.0.0: explicit Asset lazy load, generation-bound result와 namespace-local duplicate 분석을 추가.
// Migration:
// - 기본 metadata Refresh 경로와 분리되며 Save/Reference/Manager UI를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAManagementTypes.h"

class FCFDATypeRegistry;

// explicit Validate/Detail 요청에서만 필요한 DataAsset을 load하고 loaded health 결과를 만드는 Editor-only 서비스입니다.
class CARFIGHT_REEDITOR_API FCFDAHealthService
{
public:
	// current Inventory snapshot의 exact object path 한 건을 explicit validate합니다.
	// generation은 caller가 따로 전달하지 않고 Inventory snapshot 자체에서만 가져옵니다.
	static FCFDALoadedAssetResult ValidateAsset(
		const FCFDAInventoryResult& Inventory,
		const FString& ObjectPath,
		const FCFDATypeRegistry& TypeRegistry);

	// current Inventory snapshot에서 명시적으로 요청된 object path 집합만 결과로 반환합니다.
	// duplicate 결론을 위해 필요한 같은 namespace 후보는 내부적으로 bounded lazy load할 수 있습니다.
	static TArray<FCFDALoadedAssetResult> ValidateAssets(
		const FCFDAInventoryResult& Inventory,
		const TArray<FString>& ObjectPaths,
		const FCFDATypeRegistry& TypeRegistry);

	// complete namespace coverage 표시가 있는 loaded 결과 집합에서 typed/canonical duplicate를 pure하게 표시합니다.
	// incomplete coverage에서도 이미 확인된 duplicate는 Error로 표시하지만 Unique는 complete일 때만 부여합니다.
	static void AnalyzeDuplicateStableIds(
		TArray<FCFDALoadedAssetResult>& InOutResults);
};
