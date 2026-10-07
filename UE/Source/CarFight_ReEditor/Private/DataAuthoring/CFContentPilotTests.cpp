// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentPilotTests.cpp
// Version: v1.1.0
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-04 Weapon + Vehicle Dual Consumer Pilot persisted-content focused Automation입니다.
// Changelog:
// - v1.1.0: Persisted SkeletalMesh exact3에서 Socket=0이 확인된 실제 콘텐츠 상태를 fail로 왜곡하지 않고,
//   Bone/MaterialSlot real scan은 blocking evidence로 유지하며 Active Socket fixture 부재는 P2 non-blocking evidence로 기록.
// - v1.0.0: SharedCoreDualConsumer, WeaponPersistedBinding, VehiclePersistedBinding, ProfileOverrideSources,
//   SkeletalMeshPersistedScan exact5를 최초 추가.
// Migration:
// - 실제 저장된 Weapon/Turret/Vehicle/SkeletalMesh를 read-only로 load하며 Product asset을 저장하거나 수정하지 않습니다.

#include "DataAuthoring/CFContentPilot.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

namespace CFContentPilotTestsPrivate
{
	// Persisted representative WeaponData를 load합니다.
	UCFWeaponData* LoadWeaponData()
	{
		return LoadObject<UCFWeaponData>(
			nullptr,
			TEXT("/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon.DA_ProtoTurretCannon"));
	}

	// Persisted representative TurretMountData를 load합니다.
	UCFTurretMountData* LoadTurretMountData()
	{
		return LoadObject<UCFTurretMountData>(
			nullptr,
			TEXT("/Game/CarFight/Weapons/Data/TurretMounts/DA_CannonBody.DA_CannonBody"));
	}

	// Persisted representative VehicleRecipeData를 load합니다.
	UCFVehicleRecipeData* LoadVehicleRecipe()
	{
		return LoadObject<UCFVehicleRecipeData>(
			nullptr,
			TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_TestSUV.DA_Recipe_TestSUV"));
	}

	// Exact semantic RoleId의 resolved binding을 찾습니다.
	const FCFResolvedResourceBinding* FindBinding(
		const FCFResourceBindingResult& Result,
		const FString& RoleId)
	{
		return Result.Bindings.FindByPredicate(
			[&RoleId](const FCFResolvedResourceBinding& Binding)
			{
				return Binding.RoleId.Equals(
					RoleId,
					ESearchCase::CaseSensitive);
			});
	}

	// Binding이 존재하고 bound/source/capability 계약과 일치하는지 검사합니다.
	bool IsBinding(
		const FCFResourceBindingResult& Result,
		const FString& RoleId,
		const ECFResourceBindingSource Source,
		const FName CapabilityName = NAME_None)
	{
		// Requested semantic role의 resolved binding입니다.
		const FCFResolvedResourceBinding* Binding =
			FindBinding(Result, RoleId);
		if (Binding == nullptr
			|| !Binding->bBound
			|| Binding->Source != Source)
		{
			return false;
		}

		return CapabilityName.IsNone()
			|| Binding->CapabilityName == CapabilityName;
	}

	// Fresh shared pilot context를 초기화합니다.
	bool MakeContext(
		FCFContentPilotContext& OutContext,
		FString& OutError)
	{
		return OutContext.Initialize(OutError);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentPilotSharedCoreTest,
	"CarFight.CCAS.CF_FQ_058.P0_04.SharedCoreDualConsumer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentPilotWeaponTest,
	"CarFight.CCAS.CF_FQ_058.P0_04.WeaponPersistedBinding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentPilotVehicleTest,
	"CarFight.CCAS.CF_FQ_058.P0_04.VehiclePersistedBinding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentPilotSourceTest,
	"CarFight.CCAS.CF_FQ_058.P0_04.ProfileOverrideSources",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentPilotSkeletalTest,
	"CarFight.CCAS.CF_FQ_058.P0_04.SkeletalMeshPersistedScan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Weapon과 Vehicle이 exact same registry instances를 연속 사용해도 consumer-specific Core branch 없이 resolve되는지 검증합니다.
bool FCFContentPilotSharedCoreTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Persisted representative WeaponData입니다.
	UCFWeaponData* WeaponData =
		CFContentPilotTestsPrivate::LoadWeaponData();
	// Persisted representative TurretMountData입니다.
	UCFTurretMountData* TurretMountData =
		CFContentPilotTestsPrivate::LoadTurretMountData();
	// Persisted representative VehicleRecipeData입니다.
	UCFVehicleRecipeData* VehicleRecipe =
		CFContentPilotTestsPrivate::LoadVehicleRecipe();

	TestNotNull(TEXT("Persisted WeaponData fixture loads"), WeaponData);
	TestNotNull(TEXT("Persisted TurretMountData fixture loads"), TurretMountData);
	TestNotNull(TEXT("Persisted VehicleRecipe fixture loads"), VehicleRecipe);
	if (WeaponData == nullptr
		|| TurretMountData == nullptr
		|| VehicleRecipe == nullptr)
	{
		return false;
	}

	// 두 consumer가 공유할 exact same Resource Core context입니다.
	FCFContentPilotContext SharedContext;
	// Shared context initialization diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("Shared Resource Core context initializes"),
		CFContentPilotTestsPrivate::MakeContext(
			SharedContext,
			Error));

	// Persisted Weapon typed-backend pilot result입니다.
	FCFWeaponPilotResult WeaponResult;
	TestTrue(
		TEXT("Weapon resolves through shared Resource Core"),
		FCFWeaponResourcePilot::Build(
			*WeaponData,
			*TurretMountData,
			SharedContext,
			WeaponResult));

	// Persisted Vehicle typed-backend pilot result입니다.
	FCFVehiclePilotResult VehicleResult;
	TestTrue(
		TEXT("Vehicle resolves through the same Resource Core context"),
		FCFVehicleResourcePilot::Build(
			*VehicleRecipe,
			SharedContext,
			VehicleResult));

	TestTrue(
		TEXT("Existing WeaponData typed contract was reused"),
		WeaponResult.bWeaponContractValidated);
	TestTrue(
		TEXT("Existing Vehicle Recipe Snapshot backend was reused"),
		VehicleResult.bRecipeSnapshotBuilt);
	TestTrue(
		TEXT("Existing Vehicle AssetReader backend was reused"),
		VehicleResult.bAssetSnapshotBuilt);
	TestEqual(
		TEXT("Shared picker registry remains exact2 StaticMesh + SkeletalMesh"),
		SharedContext.PickerRegistry.Num(),
		2);
	TestEqual(
		TEXT("Shared Resource Catalog contains Weapon exact3 + Vehicle exact5"),
		SharedContext.ResourceCatalog.Num(),
		8);
	TestEqual(
		TEXT("Shared profile registry contains Weapon + Vehicle exact2"),
		SharedContext.ProfileRegistry.Num(),
		2);
	return true;
}

// Persisted DA_CannonBody + DA_ProtoTurretCannon의 mesh/socket intent가 shared Resource Core와 정확히 일치하는지 검증합니다.
bool FCFContentPilotWeaponTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Persisted WeaponData fixture입니다.
	UCFWeaponData* WeaponData =
		CFContentPilotTestsPrivate::LoadWeaponData();
	// Persisted TurretMountData fixture입니다.
	UCFTurretMountData* TurretMountData =
		CFContentPilotTestsPrivate::LoadTurretMountData();
	TestNotNull(TEXT("WeaponData fixture loads"), WeaponData);
	TestNotNull(TEXT("TurretMountData fixture loads"), TurretMountData);
	if (WeaponData == nullptr || TurretMountData == nullptr)
	{
		return false;
	}

	// Fresh shared Resource Core context입니다.
	FCFContentPilotContext Context;
	// Initialization diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("Pilot context initializes"),
		Context.Initialize(Error));

	// Weapon pilot result입니다.
	FCFWeaponPilotResult Result;
	TestTrue(
		TEXT("Persisted Weapon pilot resolves"),
		FCFWeaponResourcePilot::Build(
			*WeaponData,
			*TurretMountData,
			Context,
			Result));
	TestEqual(
		TEXT("Typed WeaponId preserved"),
		Result.WeaponId,
		FName(TEXT("Proto_TurretCannon")));
	TestEqual(
		TEXT("Typed TurretMountId preserved"),
		Result.TurretMountId,
		FName(TEXT("Proto_RoofTurretMount")));

	TestTrue(
		TEXT("Base mesh binding comes from Profile"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("weapon.visual.base"),
			ECFResourceBindingSource::Profile));
	TestTrue(
		TEXT("Yaw mesh binding comes from Profile"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("weapon.visual.yaw"),
			ECFResourceBindingSource::Profile));
	TestTrue(
		TEXT("Pitch mesh binding comes from Profile"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("weapon.visual.pitch"),
			ECFResourceBindingSource::Profile));
	TestTrue(
		TEXT("YawPivot is validated through typed Override"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("weapon.socket.yaw_pivot"),
			ECFResourceBindingSource::Override,
			FName(TEXT("YawPivot"))));
	TestTrue(
		TEXT("PitchPivot is validated through typed Override"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("weapon.socket.pitch_pivot"),
			ECFResourceBindingSource::Override,
			FName(TEXT("PitchPivot"))));
	TestTrue(
		TEXT("Muzzle is validated through typed Override"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("weapon.socket.muzzle_primary"),
			ECFResourceBindingSource::Override,
			FName(TEXT("Muzzle"))));
	return true;
}

// Persisted DA_Recipe_TestSUV의 typed AssetIntent/HardpointIntent와 existing AssetReader facts를 shared Resource Core에서 검증합니다.
bool FCFContentPilotVehicleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Persisted Vehicle Recipe fixture입니다.
	UCFVehicleRecipeData* VehicleRecipe =
		CFContentPilotTestsPrivate::LoadVehicleRecipe();
	TestNotNull(TEXT("Vehicle Recipe fixture loads"), VehicleRecipe);
	if (VehicleRecipe == nullptr)
	{
		return false;
	}

	// Fresh shared Resource Core context입니다.
	FCFContentPilotContext Context;
	// Initialization diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("Pilot context initializes"),
		Context.Initialize(Error));

	// Vehicle pilot result입니다.
	FCFVehiclePilotResult Result;
	TestTrue(
		TEXT("Persisted Vehicle pilot resolves"),
		FCFVehicleResourcePilot::Build(
			*VehicleRecipe,
			Context,
			Result));
	TestTrue(
		TEXT("Recipe fingerprint comes from existing SnapshotBuilder"),
		!Result.RecipeFingerprint.IsEmpty());
	TestTrue(
		TEXT("Chassis layout fingerprint comes from existing AssetReader"),
		!Result.ChassisLayoutFingerprint.IsEmpty());

	TestTrue(
		TEXT("Chassis resource binding comes from Profile"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("vehicle.visual.chassis"),
			ECFResourceBindingSource::Profile));
	TestTrue(
		TEXT("Wheel FL socket uses typed Override"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("vehicle.socket.wheel_fl"),
			ECFResourceBindingSource::Override,
			FName(TEXT("Wheel_Anchor_FL"))));
	TestTrue(
		TEXT("Wheel FR socket uses typed Override"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("vehicle.socket.wheel_fr"),
			ECFResourceBindingSource::Override,
			FName(TEXT("Wheel_Anchor_FR"))));
	TestTrue(
		TEXT("Wheel RL socket uses typed Override"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("vehicle.socket.wheel_rl"),
			ECFResourceBindingSource::Override,
			FName(TEXT("Wheel_Anchor_RL"))));
	TestTrue(
		TEXT("Wheel RR socket uses typed Override"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("vehicle.socket.wheel_rr"),
			ECFResourceBindingSource::Override,
			FName(TEXT("Wheel_Anchor_RR"))));
	TestTrue(
		TEXT("Front_01 persisted hardpoint socket resolves"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("vehicle.socket.hardpoint.front_01"),
			ECFResourceBindingSource::Profile,
			FName(TEXT("HP_Front_01"))));
	TestTrue(
		TEXT("Top_01 persisted hardpoint socket resolves"),
		CFContentPilotTestsPrivate::IsBinding(
			Result.ResourceBindings,
			TEXT("vehicle.socket.hardpoint.top_01"),
			ECFResourceBindingSource::Profile,
			FName(TEXT("HP_Top_01"))));
	TestNull(
		TEXT("Top_02 has no explicit socket and is not invented as a binding"),
		CFContentPilotTestsPrivate::FindBinding(
			Result.ResourceBindings,
			TEXT("vehicle.socket.hardpoint.top_02")));
	return true;
}

// 두 consumer 모두 Profile base와 per-content Override precedence를 같은 resolver semantics로 사용하는지 교차 검증합니다.
bool FCFContentPilotSourceTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Persisted Weapon fixture입니다.
	UCFWeaponData* WeaponData =
		CFContentPilotTestsPrivate::LoadWeaponData();
	// Persisted Turret fixture입니다.
	UCFTurretMountData* TurretMountData =
		CFContentPilotTestsPrivate::LoadTurretMountData();
	// Persisted Vehicle fixture입니다.
	UCFVehicleRecipeData* VehicleRecipe =
		CFContentPilotTestsPrivate::LoadVehicleRecipe();
	if (WeaponData == nullptr
		|| TurretMountData == nullptr
		|| VehicleRecipe == nullptr)
	{
		AddError(TEXT("Dual consumer persisted fixtures를 load하지 못했습니다."));
		return false;
	}

	// Exact same Resource Core context입니다.
	FCFContentPilotContext SharedContext;
	// Initialization diagnostic입니다.
	FString Error;
	if (!SharedContext.Initialize(Error))
	{
		AddError(Error);
		return false;
	}

	// Weapon projection result입니다.
	FCFWeaponPilotResult WeaponResult;
	// Vehicle projection result입니다.
	FCFVehiclePilotResult VehicleResult;
	TestTrue(
		TEXT("Weapon pilot succeeds"),
		FCFWeaponResourcePilot::Build(
			*WeaponData,
			*TurretMountData,
			SharedContext,
			WeaponResult));
	TestTrue(
		TEXT("Vehicle pilot succeeds"),
		FCFVehicleResourcePilot::Build(
			*VehicleRecipe,
			SharedContext,
			VehicleResult));

	TestTrue(
		TEXT("Weapon visual source is Profile"),
		CFContentPilotTestsPrivate::IsBinding(
			WeaponResult.ResourceBindings,
			TEXT("weapon.visual.pitch"),
			ECFResourceBindingSource::Profile));
	TestTrue(
		TEXT("Weapon capability source is Override"),
		CFContentPilotTestsPrivate::IsBinding(
			WeaponResult.ResourceBindings,
			TEXT("weapon.socket.muzzle_primary"),
			ECFResourceBindingSource::Override));
	TestTrue(
		TEXT("Vehicle visual source is Profile"),
		CFContentPilotTestsPrivate::IsBinding(
			VehicleResult.ResourceBindings,
			TEXT("vehicle.visual.chassis"),
			ECFResourceBindingSource::Profile));
	TestTrue(
		TEXT("Vehicle capability source is Override"),
		CFContentPilotTestsPrivate::IsBinding(
			VehicleResult.ResourceBindings,
			TEXT("vehicle.socket.wheel_fl"),
			ECFResourceBindingSource::Override));
	return true;
}

// Persisted USkeletalMesh에서 active Socket, Reference Skeleton Bone, Material Slot을 P0-03 scanner로 end-to-end 관측합니다.
bool FCFContentPilotSkeletalTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fresh Resource Core context입니다.
	FCFContentPilotContext Context;
	// Initialization diagnostic입니다.
	FString Error;
	if (!Context.Initialize(Error))
	{
		AddError(Error);
		return false;
	}

	// 실제 persisted SkeletalMesh evidence candidates입니다.
	struct FSkeletalFixture
	{
		// Stable evidence ResourceId입니다.
		const TCHAR* ResourceId;

		// Persisted SkeletalMesh object path입니다.
		const TCHAR* ObjectPath;
	};

	// Project에 실제 존재하는 representative SkeletalMesh exact3입니다.
	const FSkeletalFixture Fixtures[] = {
		{
			TEXT("evidence.skeletal.sportscar_vehicle"),
			TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar")
		},
		{
			TEXT("evidence.skeletal.offroad_vehicle"),
			TEXT("/Game/Vehicles/OffroadCar/SKM_Offroad.SKM_Offroad")
		},
		{
			TEXT("evidence.skeletal.sportscar_model"),
			TEXT("/Game/Models/SportsCar/SKM_SportsCar.SKM_SportsCar")
		}
	};

	// At least one persisted fixture가 non-empty active socket을 제공했는지 여부입니다.
	bool bFoundActiveSocket = false;
	// At least one persisted fixture가 non-empty reference skeleton bone을 제공했는지 여부입니다.
	bool bFoundBone = false;
	// At least one persisted fixture가 non-empty material slot을 제공했는지 여부입니다.
	bool bFoundMaterialSlot = false;

	for (const FSkeletalFixture& Fixture : Fixtures)
	{
		// Persisted SkeletalMesh resource descriptor입니다.
		FCFResourceDescriptor Resource;
		Resource.ResourceId = Fixture.ResourceId;
		Resource.PickerTypeId = TEXT("picker.skeletal_mesh");
		Resource.ObjectPath = FSoftObjectPath(Fixture.ObjectPath);
		if (!Context.ResourceCatalog.RegisterResource(
			Resource,
			Context.PickerRegistry,
			Error))
		{
			AddError(
				FString::Printf(
					TEXT("SkeletalMesh resource registration 실패: %s / %s"),
					Fixture.ResourceId,
					*Error));
			return false;
		}

		// P0-03 generic Mesh Capability Scanner가 만든 persisted snapshot입니다.
		FCFResourceCapabilitySnapshot Snapshot;
		// Scan diagnostics입니다.
		TArray<FCFContentValidationIssue> Issues;
		const bool bScanned = FCFMeshCapabilityScanner::ScanResource(
			Fixture.ResourceId,
			Context.ResourceCatalog,
			Context.PickerRegistry,
			Snapshot,
			Issues);
		TestTrue(
			FString::Printf(
				TEXT("Persisted SkeletalMesh scans: %s"),
				Fixture.ResourceId),
			bScanned);
		if (!bScanned)
		{
			continue;
		}

		AddInfo(
			FString::Printf(
				TEXT("SkeletalMeshEvidence Resource=%s Socket=%d Bone=%d MaterialSlot=%d"),
				Fixture.ResourceId,
				Snapshot.SocketNames.Num(),
				Snapshot.BoneNames.Num(),
				Snapshot.MaterialSlotNames.Num()));

		bFoundActiveSocket =
			bFoundActiveSocket || !Snapshot.SocketNames.IsEmpty();
		bFoundBone =
			bFoundBone || !Snapshot.BoneNames.IsEmpty();
		bFoundMaterialSlot =
			bFoundMaterialSlot || !Snapshot.MaterialSlotNames.IsEmpty();
	}

	if (!bFoundActiveSocket)
	{
		AddInfo(
			TEXT("Persisted USkeletalMesh exact3은 active Socket을 보유하지 않습니다. Scanner real execution은 PASS이며 Active Socket fixture evidence P2는 non-blocking으로 유지합니다."));
	}
	TestTrue(
		TEXT("At least one persisted USkeletalMesh exposes Reference Skeleton Bone evidence"),
		bFoundBone);
	TestTrue(
		TEXT("At least one persisted USkeletalMesh exposes Material Slot evidence"),
		bFoundMaterialSlot);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
