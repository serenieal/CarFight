// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleSnapshotBuilder.h
// Version: v1.2.0
// Date: 2026-08-17
// Description: DAUTH-P0-08C/F/G immutable Snapshot Builder와 공용 Recipe/Definition hash 계약입니다.
// Scope: Recipe/Profile/Definition/Project Compatibility Default Snapshot과 canonical fingerprint/hash 생성을 담당합니다.
// Changelog:
// - v1.2.0: DAUTH-P0-08G Adoption Preview가 UObject mutation 없이 prospective Recipe fingerprint를 계산할 수 있도록 BuildRecipeFingerprintFromSnapshot을 공개.
// - v1.1.0: DAUTH-P0-08F Resolver materialized readback과 같은 Definition hash authority를 공유하도록 BuildDefinitionHashFromFields를 공개.
// - v1.0.0: Section 22.13/22.27 Snapshot Builder 최초 구현.
// Migration:
// - Asset Snapshot Reader / Apply / Content Asset mutation 기능을 포함하지 않습니다.
// - Recipe/Definition hash 의미와 format revision은 기존 canonical 계약을 그대로 재사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleSnapshotTypes.h"

class UCFDriveStateProfile;
class UCFDrivetrainProfile;
class UCFHandlingProfile;
class UCFPerformanceProfile;
class UCFVehicleBaseProfile;
class UCFVehicleData;
class UCFVehicleRecipeData;

/** Live UObject read와 Pure Resolve 사이를 끊는 Editor-only immutable-style Snapshot Builder입니다. */
class FCFVehicleSnapshotBuilder
{
public:
	// Persistent Recipe를 Resolve-safe value copy와 deterministic Recipe fingerprint로 변환합니다.
	static bool BuildRecipeSnapshot(
		const UCFVehicleRecipeData& Recipe,
		FCFVehicleRecipeSnapshot& OutSnapshot,
		FString& OutError);

	// 이미 resolve된 5개 typed Profile UObject를 source+payload Snapshot Set으로 복사합니다.
	static bool BuildProfileSnapshotSet(
		const UCFVehicleBaseProfile* BaseProfile,
		const UCFDrivetrainProfile* DrivetrainProfile,
		const UCFHandlingProfile* HandlingProfile,
		const UCFPerformanceProfile* PerformanceProfile,
		const UCFDriveStateProfile* DriveStateProfile,
		FCFVehicleProfileSnapshotSet& OutSnapshot,
		FString& OutError);

	// Current UCFVehicleData를 Registry descriptor로 실제 Stable-ID element까지 확장한 exact Snapshot으로 읽습니다.
	static bool BuildDefinitionSnapshot(
		const UCFVehicleData& Definition,
		FCFVehicleDefinitionSnapshot& OutSnapshot,
		FString& OutError);

		// UCFVehicleData C++ defaults와 default-constructed array element를 사용해 exact 117-pattern compatibility baseline을 만듭니다.
	static bool BuildProjectCompatibilityDefaultSnapshot(
		FCFVehicleDefinitionSnapshot& OutSnapshot,
		FString& OutError);

		// UObject를 다시 읽지 않고 Recipe Snapshot semantic payload의 deterministic fingerprint를 다시 계산합니다.
	static bool BuildRecipeFingerprintFromSnapshot(
		const FCFVehicleRecipeSnapshot& RecipeSnapshot,
		FString& OutRecipeFingerprint,
		FString& OutError);

	// Stable field entry set을 path 순으로 정규화한 뒤 Snapshot과 Resolver가 공유하는 deterministic Definition hash를 만듭니다.
	static bool BuildDefinitionHashFromFields(
		const TArray<FCFVehicleFieldEntry>& FieldEntries,
		FString& OutDefinitionHash,
		FString& OutError);
};
