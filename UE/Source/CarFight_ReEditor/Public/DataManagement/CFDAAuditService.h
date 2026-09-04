// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAuditService.h
// Version: v1.2.0
// Date: 2026-09-03
// Description: CF-FQ-045 metadata-only inventory service + generation-aware Typed Registry Coverage bridge입니다.
// Changelog:
// - v1.2.0: DAM-P0-02C용 InventoryGeneration stamp overload와 next-generation helper를 추가. 기본 Refresh의 metadata-only 동작은 유지.
// - v1.1.0: FCFDATypeRegistry를 직접 받는 Refresh/BuildInventory overload를 추가해 descriptor coverage를 metadata inventory와 연결.
// - v1.0.0: native CarFight DataAsset auto-discovery, metadata-only persisted asset scan, deterministic merge/dedupe와 Scope/Coverage 계산을 추가.
// Migration:
// - 기존 DA class/Asset 수정 없음. Registry overload도 FAssetData::GetAsset()을 호출하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAManagementTypes.h"

class FCFDATypeRegistry;

// CarFight Data Asset inventory를 mutation 없이 생성하는 Editor-only 서비스입니다.
class CARFIGHT_REEDITOR_API FCFDAAuditService
{
public:
	// caller의 current generation을 stamp하면서 Typed Registry metadata-only inventory를 새로 만듭니다.
	static FCFDAInventoryResult Refresh(
		const FCFDATypeRegistry& TypeRegistry,
		uint64 InventoryGeneration);

	// Typed Registry descriptor coverage를 적용해 metadata-only inventory를 새로 만듭니다. generation은 legacy 0입니다.
	static FCFDAInventoryResult Refresh(const FCFDATypeRegistry& TypeRegistry);

	// 현재 loaded native class universe와 Asset Registry metadata로 inventory를 새로 만듭니다.
	static FCFDAInventoryResult Refresh(const FCFDARefreshOptions& Options = FCFDARefreshOptions());

	// synthetic DTO에 Typed Registry descriptor coverage를 적용해 pure inventory를 만듭니다.
	static FCFDAInventoryResult BuildInventory(
		ECFDARegistryState RegistryState,
		const FString& RegistryMessage,
		const TArray<FCFDANativeTypeMetadata>& NativeTypes,
		const TArray<FCFDAPersistedAssetMetadata>& PersistedAssets,
		const FCFDATypeRegistry& TypeRegistry);

	// synthetic DTO를 deterministic하게 merge/dedupe하여 Automation 가능한 pure inventory를 만듭니다.
	static FCFDAInventoryResult BuildInventory(
		ECFDARegistryState RegistryState,
		const FString& RegistryMessage,
		const TArray<FCFDANativeTypeMetadata>& NativeTypes,
		const TArray<FCFDAPersistedAssetMetadata>& PersistedAssets,
		const FCFDARefreshOptions& Options);

	// session-local owner가 Refresh 직전에 사용할 다음 generation 값을 계산합니다. static cache/state를 소유하지 않습니다.
	static uint64 NextInventoryGeneration(uint64 CurrentInventoryGeneration);

	// Package path를 DAM v0.1.1 Scope 계약으로 분류합니다.
	static ECFDAScope ClassifyScope(const FString& PackagePath);
};
