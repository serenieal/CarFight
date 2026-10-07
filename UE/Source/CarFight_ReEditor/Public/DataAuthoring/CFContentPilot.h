// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentPilot.h
// Version: v1.1.0
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-04 Weapon + Vehicle Dual Consumer Pilot의 provider-local read-only bridge 계약입니다.
// Changelog:
// - v1.1.0: Fresh review 교정으로 Profile capability도 fixture 문자열이 아니라 기존 typed backend의 실제 socket intent를 사용하도록 고정.
// - v1.0.0: WeaponData/TurretMountData와 VehicleRecipe/AssetReader typed backend를 P0-03 Resource Core에 연결하는
//   shared context, Weapon pilot, Vehicle pilot result/entry를 최초 구현.
// Migration:
// - CFContentResource Generic Core는 변경하지 않습니다.
// - Product Apply, Product writer activation, Workbook persistent write, P0-07 authority cutover를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentResource.h"

class UCFTurretMountData;
class UCFVehicleRecipeData;
class UCFWeaponData;

/** Weapon/Vehicle pilot이 동일한 Resource Core registry instances를 공유하기 위한 Editor-only context입니다. */
struct FCFContentPilotContext
{
	// Stable picker type registration을 공유합니다.
	FCFResourcePickerRegistry PickerRegistry;

	// Weapon/Vehicle가 등록한 stable ResourceId를 같은 catalog에서 관리합니다.
	FCFResourceCatalog ResourceCatalog;

	// 두 consumer의 namespaced semantic role contract를 같은 registry에서 관리합니다.
	FCFSemanticRoleRegistry RoleRegistry;

	// 두 consumer의 typed profile projection을 같은 registry에서 관리합니다.
	FCFResourceProfileRegistry ProfileRegistry;

	// Static/Skeletal Mesh picker와 fixed shared role schema를 exact1로 초기화합니다.
	bool Initialize(FString& OutError);
};

/** CF-FQ-055 typed Weapon backend에서 Resource Core로 projection한 pilot 결과입니다. */
struct FCFWeaponPilotResult
{
	// Existing WeaponData의 domain identity입니다.
	FName WeaponId = NAME_None;

	// Existing TurretMountData의 domain identity입니다.
	FName TurretMountId = NAME_None;

	// Existing WeaponData 정적 contract validation을 실제 통과했는지 나타냅니다.
	bool bWeaponContractValidated = false;

	// Weapon profile + typed socket override를 shared Core가 resolve한 결과입니다.
	FCFResourceBindingResult ResourceBindings;

	// Provider-local projection/identity 오류와 shared Core diagnostics를 합친 결과입니다.
	TArray<FCFContentValidationIssue> Issues;
};

/** Existing Vehicle Authoring/Builder backend를 Resource Core로 projection한 pilot 결과입니다. */
struct FCFVehiclePilotResult
{
	// Existing Recipe의 persistent authoring identity입니다.
	FGuid RecipeId;

	// Existing VehicleSnapshotBuilder가 계산한 typed Recipe fingerprint입니다.
	FString RecipeFingerprint;

	// Existing VehicleAssetReader가 계산한 Chassis/socket layout fingerprint입니다.
	FString ChassisLayoutFingerprint;

	// Existing VehicleSnapshotBuilder read-only seam을 실제 통과했는지 나타냅니다.
	bool bRecipeSnapshotBuilt = false;

	// Existing VehicleAssetReader read-only seam을 실제 통과했는지 나타냅니다.
	bool bAssetSnapshotBuilt = false;

	// Vehicle profile + typed socket override를 shared Core가 resolve한 결과입니다.
	FCFResourceBindingResult ResourceBindings;

	// Provider-local projection/identity 오류와 shared Core diagnostics를 합친 결과입니다.
	TArray<FCFContentValidationIssue> Issues;
};

/** Existing CF-FQ-055 WeaponData/TurretMountData typed semantics를 shared Resource Core로 연결합니다. */
class FCFWeaponResourcePilot
{
public:
	// Product mutation 없이 WeaponData contract와 Turret resource/socket binding을 resolve합니다.
	static bool Build(
		const UCFWeaponData& WeaponData,
		const UCFTurretMountData& TurretMountData,
		FCFContentPilotContext& SharedContext,
		FCFWeaponPilotResult& OutResult);
};

/** Existing Vehicle Recipe/Snapshot/AssetReader typed semantics를 shared Resource Core로 연결합니다. */
class FCFVehicleResourcePilot
{
public:
	// Product mutation 없이 Recipe Snapshot/Asset Snapshot과 resource/socket binding을 resolve합니다.
	static bool Build(
		const UCFVehicleRecipeData& Recipe,
		FCFContentPilotContext& SharedContext,
		FCFVehiclePilotResult& OutResult);
};
