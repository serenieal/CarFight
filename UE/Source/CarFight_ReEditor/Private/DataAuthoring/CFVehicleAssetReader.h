// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAssetReader.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08D Asset Snapshot Reader public-in-module 계약입니다.
// Scope: Recipe Snapshot의 StaticMesh refs를 immutable FCFVehicleAssetSnapshot으로 읽습니다.
// Changelog:
// - v1.0.0: Chassis socket facts / Wheel local bounds / resolver-relevant fingerprint reader 최초 구현.
// Migration:
// - Target VehicleData, StaticMesh, Content Asset을 수정하거나 저장하지 않습니다.
// - Resolver / Measurement Proposal 계산은 이 Reader의 책임이 아닙니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleSnapshotTypes.h"

/** Editor UObject/StaticMesh read와 향후 Pure Resolver 사이를 분리하는 read-only Asset Snapshot Reader입니다. */
class FCFVehicleAssetReader
{
public:
	// Recipe Snapshot이 요청한 Chassis socket facts와 4개 Wheel bounds를 resolver-safe value copy로 읽습니다.
	static bool BuildAssetSnapshot(
		const FCFVehicleRecipeSnapshot& RecipeSnapshot,
		FCFVehicleAssetSnapshot& OutSnapshot,
		FString& OutError);
};
