// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAHealthTests.cpp
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-02C Loaded Health Lane correction focused Automation입니다.
// Changelog:
// - v1.1.0: inventory-bound provenance, evaluation/Health 분리, complete namespace duplicate, typed canonical identity와 operational failure를 검증.
// - v1.0.1: current mapping에 없는 Optional identity의 missing → NotResolved/non-Error 계약을 synthetic descriptor로 고정.
// - v1.0.0: generation freshness, Unregistered no-policy, typed identity/validation, explicit lazy load와 namespace-local duplicate를 검증.
// Migration:
// - transient existing DA classes와 read-only persisted Asset load만 사용하며 Product Asset mutation/Save/Reference/UI를 수행하지 않습니다.

#include "DataManagement/CFDAAuditService.h"
#include "DataManagement/CFDAHealthService.h"
#include "DataManagement/CFDATypeAdapter.h"
#include "DataManagement/CFDATypeRegistry.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFAmmoData.h"
#include "CFInventoryItemData.h"
#include "CFVehicleCameraData.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

namespace CFDAHealthTestsPrivate
{
	FCFDAAssetRecord MakeRecord(
		const UClass* AssetClass,
		const FString& ObjectPath)
	{
		FCFDAAssetRecord Record;
		Record.AssetName = TEXT("DAM_Test");
		Record.ObjectPath = ObjectPath;
		Record.PackagePath = TEXT("/Game/CarFight/Tests/DataManagement");
		Record.ClassPath = AssetClass ? AssetClass->GetClassPathName().ToString() : FString();
		Record.Scope = ECFDAScope::Test;
		Record.CoverageState = ECFDACoverageState::Registered;
		Record.HealthState = ECFDAHealthState::NotValidated;
		Record.StableIdState = ECFDAStableIdState::NotResolved;
		return Record;
	}

	const FCFDAAssetRecord* FindFirstAssetByClass(
		const FCFDAInventoryResult& Inventory,
		const FString& ClassPath)
	{
		for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
		{
			if (AssetRecord.ClassPath == ClassPath)
			{
				return &AssetRecord;
			}
		}
		return nullptr;
	}

	FCFDAInventoryResult MakeSyntheticInventory(
		const uint64 InventoryGeneration,
		const FCFDAAssetRecord& AssetRecord)
	{
		FCFDAInventoryResult Inventory;
		Inventory.InventoryGeneration = InventoryGeneration;
		Inventory.RegistryState = ECFDARegistryState::Ready;
		Inventory.AssetRecords.Add(AssetRecord);
		return Inventory;
	}

	FCFDALoadedAssetResult MakeResolvedNameResult(
		const uint64 InventoryGeneration,
		const FString& ObjectPath,
		const FString& Namespace,
		const FName StableName,
		const bool bCoverageComplete)
	{
		FCFDALoadedAssetResult Result;
		Result.InventoryGeneration = InventoryGeneration;
		Result.ObjectPath = ObjectPath;
		Result.ClassPath = Namespace;
		Result.EvaluationState = ECFDAEvaluationState::Succeeded;
		Result.bAssetLoaded = true;
		Result.StableIdState = ECFDAStableIdState::Resolved;
		Result.StableId = StableName.ToString();
		Result.DuplicateNamespace = Namespace;
		Result.CanonicalStableIdentity.Namespace = Namespace;
		Result.CanonicalStableIdentity.ValueKind = ECFDAStableIdentityValueKind::Name;
		Result.CanonicalStableIdentity.NameValue = StableName;
		Result.bDuplicateNamespaceCoverageComplete = bCoverageComplete;
		Result.HealthState = ECFDAHealthState::OK;
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAGenerationLoadedTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_02C.GenerationAndExplicitLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDATypedHealthTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_02C.TypedIdentityValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADuplicateHealthTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_02C.NamespaceDuplicate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Inventory snapshot provenance, explicit load와 operational failure 분리를 current persisted Asset으로 검증합니다.
bool FCFDAGenerationLoadedTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("First inventory generation"), FCFDAAuditService::NextInventoryGeneration(0), static_cast<uint64>(1));
	TestEqual(TEXT("Inventory generation increments"), FCFDAAuditService::NextInventoryGeneration(41), static_cast<uint64>(42));

	FCFDATypeRegistry TypeRegistry;
	FString RegistrationError;
	if (!TestTrue(
		TEXT("Current descriptors register for P0-02C correction"),
		TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistrationError)))
	{
		AddError(RegistrationError);
		return false;
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.SearchAllAssets(true);
	TestFalse(TEXT("Asset Registry completion prerequisite"), AssetRegistry.IsLoadingAssets());

	const uint64 InventoryGeneration = 7;
	const FCFDAInventoryResult Inventory =
		FCFDAAuditService::Refresh(TypeRegistry, InventoryGeneration);
	TestEqual(TEXT("Inventory generation stamped"), Inventory.InventoryGeneration, InventoryGeneration);
	TestTrue(TEXT("Inventory generation reports current"), Inventory.IsCurrentGeneration(InventoryGeneration));
	TestFalse(TEXT("Inventory generation rejects stale"), Inventory.IsCurrentGeneration(InventoryGeneration + 1));

	for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
	{
		TestEqual(
			FString::Printf(TEXT("Metadata Health remains NotValidated: %s"), *AssetRecord.ObjectPath),
			AssetRecord.HealthState,
			ECFDAHealthState::NotValidated);
		TestEqual(
			FString::Printf(TEXT("Metadata StableId remains NotResolved: %s"), *AssetRecord.ObjectPath),
			AssetRecord.StableIdState,
			ECFDAStableIdState::NotResolved);
	}

	const FCFDAAssetRecord* CatalogRecord =
		CFDAHealthTestsPrivate::FindFirstAssetByClass(
			Inventory,
			TEXT("/Script/CarFight_Re.CFRuntimeTestCatalogData"));
	TestNotNull(TEXT("Current RuntimeTestCatalog asset exists"), CatalogRecord);
	if (CatalogRecord)
	{
		const FCFDALoadedAssetResult LoadedCatalog =
			FCFDAHealthService::ValidateAsset(
				Inventory,
				CatalogRecord->ObjectPath,
				TypeRegistry);
		TestEqual(TEXT("Catalog evaluation succeeds"), LoadedCatalog.EvaluationState, ECFDAEvaluationState::Succeeded);
		TestTrue(TEXT("Explicit catalog request loads asset"), LoadedCatalog.bAssetLoaded);
		TestEqual(TEXT("Catalog loaded generation comes from inventory"), LoadedCatalog.InventoryGeneration, Inventory.InventoryGeneration);
		TestTrue(TEXT("Catalog loaded result is fresh"), LoadedCatalog.IsFreshForInventoryGeneration(InventoryGeneration));
		TestFalse(TEXT("Catalog loaded result becomes stale"), LoadedCatalog.IsFreshForInventoryGeneration(InventoryGeneration + 1));
		TestEqual(TEXT("Catalog stable identity N/A"), LoadedCatalog.StableIdState, ECFDAStableIdState::NotApplicable);
		TestEqual(TEXT("Catalog duplicate N/A"), LoadedCatalog.DuplicateState, ECFDADuplicateState::NotApplicable);
		TestTrue(TEXT("Catalog explicit validation leaves NotValidated state"), LoadedCatalog.HealthState != ECFDAHealthState::NotValidated);
	}

	// single Validate도 exact-class duplicate namespace 후보를 내부 평가하고 요청 Asset 한 건만 반환해야 합니다.
	const FCFDAAssetRecord* WeaponRecord =
		CFDAHealthTestsPrivate::FindFirstAssetByClass(
			Inventory,
			TEXT("/Script/CarFight_Re.CFWeaponData"));
	TestNotNull(TEXT("Current WeaponData asset exists"), WeaponRecord);
	if (WeaponRecord)
	{
		const TArray<FCFDALoadedAssetResult> WeaponResults =
			FCFDAHealthService::ValidateAssets(
				Inventory,
				TArray<FString>{WeaponRecord->ObjectPath},
				TypeRegistry);
		TestEqual(TEXT("Single Weapon request returns one result"), WeaponResults.Num(), 1);
		if (WeaponResults.Num() == 1)
		{
			TestEqual(TEXT("Weapon evaluation succeeds"), WeaponResults[0].EvaluationState, ECFDAEvaluationState::Succeeded);
			TestTrue(TEXT("Weapon duplicate namespace coverage complete"), WeaponResults[0].bDuplicateNamespaceCoverageComplete);
			TestTrue(
				TEXT("Weapon duplicate state is conclusive"),
				WeaponResults[0].DuplicateState == ECFDADuplicateState::Unique
					|| WeaponResults[0].DuplicateState == ECFDADuplicateState::Duplicate);
		}
	}

	// PrimaryAssetId namespace도 single Validate에서 bounded closure를 거쳐 conclusive duplicate state가 되어야 합니다.
	const FCFDAAssetRecord* VehicleRecord =
		CFDAHealthTestsPrivate::FindFirstAssetByClass(
			Inventory,
			TEXT("/Script/CarFight_Re.CFVehicleData"));
	TestNotNull(TEXT("Current VehicleData asset exists"), VehicleRecord);
	if (VehicleRecord)
	{
		const FCFDALoadedAssetResult VehicleResult =
			FCFDAHealthService::ValidateAsset(
				Inventory,
				VehicleRecord->ObjectPath,
				TypeRegistry);
		TestEqual(TEXT("Vehicle evaluation succeeds"), VehicleResult.EvaluationState, ECFDAEvaluationState::Succeeded);
		TestEqual(TEXT("Vehicle PrimaryAssetId resolves"), VehicleResult.StableIdState, ECFDAStableIdState::Resolved);
		TestTrue(TEXT("Vehicle duplicate namespace coverage complete"), VehicleResult.bDuplicateNamespaceCoverageComplete);
		TestTrue(
			TEXT("Vehicle duplicate state is conclusive"),
			VehicleResult.DuplicateState == ECFDADuplicateState::Unique
				|| VehicleResult.DuplicateState == ECFDADuplicateState::Duplicate);
	}

	// current snapshot에 없는 요청은 load를 시도하지 않는 InvalidRequest입니다.
	const FCFDALoadedAssetResult UnknownRequest =
		FCFDAHealthService::ValidateAsset(
			Inventory,
			TEXT("/Game/CarFight/Tests/DoesNotExist.DoesNotExist"),
			TypeRegistry);
	TestEqual(TEXT("Unknown path is InvalidRequest"), UnknownRequest.EvaluationState, ECFDAEvaluationState::InvalidRequest);
	TestFalse(TEXT("Unknown path is not loaded"), UnknownRequest.bAssetLoaded);
	TestEqual(TEXT("Unknown path Health remains NotValidated"), UnknownRequest.HealthState, ECFDAHealthState::NotValidated);

	// registered row가 snapshot에 있지만 실제 object가 없으면 DA Error가 아니라 LoadFailed입니다.
	FCFDAAssetRecord MissingObjectRecord =
		CFDAHealthTestsPrivate::MakeRecord(
			UCFAmmoData::StaticClass(),
			TEXT("/Game/CarFight/Tests/DoesNotExist.DA_MissingAmmo"));
	const FCFDAInventoryResult MissingObjectInventory =
		CFDAHealthTestsPrivate::MakeSyntheticInventory(33, MissingObjectRecord);
	const FCFDALoadedAssetResult MissingObjectResult =
		FCFDAHealthService::ValidateAsset(
			MissingObjectInventory,
			MissingObjectRecord.ObjectPath,
			TypeRegistry);
	TestEqual(TEXT("Missing object is LoadFailed"), MissingObjectResult.EvaluationState, ECFDAEvaluationState::LoadFailed);
	TestEqual(TEXT("Missing object Health remains NotValidated"), MissingObjectResult.HealthState, ECFDAHealthState::NotValidated);
	TestFalse(TEXT("Missing object is not loaded"), MissingObjectResult.bAssetLoaded);

	// actual Catalog object path를 Ammo metadata로 위장하면 tool metadata mismatch이며 DA contract Error가 아닙니다.
	if (CatalogRecord)
	{
		FCFDAAssetRecord MismatchedRecord = *CatalogRecord;
		MismatchedRecord.ClassPath = UCFAmmoData::StaticClass()->GetClassPathName().ToString();
		MismatchedRecord.CoverageState = ECFDACoverageState::Registered;
		const FCFDAInventoryResult MismatchInventory =
			CFDAHealthTestsPrivate::MakeSyntheticInventory(34, MismatchedRecord);
		const FCFDALoadedAssetResult MismatchResult =
			FCFDAHealthService::ValidateAsset(
				MismatchInventory,
				MismatchedRecord.ObjectPath,
				TypeRegistry);
		TestEqual(TEXT("Metadata class mismatch is ClassMismatch"), MismatchResult.EvaluationState, ECFDAEvaluationState::ClassMismatch);
		TestEqual(TEXT("Class mismatch Health remains NotValidated"), MismatchResult.HealthState, ECFDAHealthState::NotValidated);
	}
	return true;
}

// Required/Optional/N-A identity, typed validation, canonical key와 adapter-unavailable 분리를 transient existing DA로 검증합니다.
bool FCFDATypedHealthTest::RunTest(const FString& Parameters)
{
	FCFDATypeRegistry TypeRegistry;
	FString RegistrationError;
	if (!TestTrue(
		TEXT("Current descriptors register for typed health"),
		TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistrationError)))
	{
		AddError(RegistrationError);
		return false;
	}

	UCFAmmoData* ValidAmmo = NewObject<UCFAmmoData>();
	ValidAmmo->AmmoId = TEXT("DAM_Test_Ammo");
	const FCFDAAssetRecord ValidAmmoRecord =
		CFDAHealthTestsPrivate::MakeRecord(
			UCFAmmoData::StaticClass(),
			TEXT("/Transient/DAM_Test_Ammo"));

	const FCFDALoadedAssetResult ValidAmmoResult =
		FCFDATypeAdapter::EvaluateLoadedAsset(
			ValidAmmoRecord,
			ValidAmmo,
			TypeRegistry,
			3);
	TestEqual(TEXT("Valid Ammo evaluation succeeds"), ValidAmmoResult.EvaluationState, ECFDAEvaluationState::Succeeded);
	TestTrue(TEXT("Valid Ammo loaded"), ValidAmmoResult.bAssetLoaded);
	TestEqual(TEXT("Valid Ammo stable ID resolved"), ValidAmmoResult.StableIdState, ECFDAStableIdState::Resolved);
	TestEqual(TEXT("Valid Ammo stable ID display"), ValidAmmoResult.StableId, FString(TEXT("DAM_Test_Ammo")));
	TestEqual(TEXT("Valid Ammo namespace exact class"), ValidAmmoResult.DuplicateNamespace, ValidAmmoRecord.ClassPath);
	TestEqual(TEXT("Valid Ammo canonical kind Name"), ValidAmmoResult.CanonicalStableIdentity.ValueKind, ECFDAStableIdentityValueKind::Name);
	TestEqual(TEXT("Valid Ammo canonical FName"), ValidAmmoResult.CanonicalStableIdentity.NameValue, FName(TEXT("DAM_Test_Ammo")));
	TestEqual(TEXT("Valid Ammo validation OK"), ValidAmmoResult.HealthState, ECFDAHealthState::OK);
	TestEqual(TEXT("Adapter alone has not analyzed duplicate"), ValidAmmoResult.DuplicateState, ECFDADuplicateState::NotAnalyzed);

	UCFAmmoData* MissingAmmo = NewObject<UCFAmmoData>();
	const FCFDALoadedAssetResult MissingAmmoResult =
		FCFDATypeAdapter::EvaluateLoadedAsset(
			ValidAmmoRecord,
			MissingAmmo,
			TypeRegistry,
			3);
	TestEqual(TEXT("Missing Ammo evaluation still succeeds"), MissingAmmoResult.EvaluationState, ECFDAEvaluationState::Succeeded);
	TestEqual(TEXT("Missing Ammo ID is MissingRequired"), MissingAmmoResult.StableIdState, ECFDAStableIdState::MissingRequired);
	TestEqual(TEXT("Missing Ammo ID raises DA Error"), MissingAmmoResult.HealthState, ECFDAHealthState::Error);

	FCFDATypeRegistry OptionalIdentityRegistry;
	FCFDASemanticDescriptor OptionalAmmoDescriptor;
	OptionalAmmoDescriptor.ClassPath = UCFAmmoData::StaticClass()->GetClassPathName().ToString();
	OptionalAmmoDescriptor.Domain = ECFDADomain::Combat;
	OptionalAmmoDescriptor.TypeDisplayName = TEXT("Optional 탄약 테스트 데이터");
	OptionalAmmoDescriptor.RoleDescription = TEXT("Optional identity missing semantics를 검증하는 synthetic descriptor입니다.");
	OptionalAmmoDescriptor.IdentityPolicy = ECFDAIdentityPolicy::Optional;
	OptionalAmmoDescriptor.IdentityResolverKind = ECFDAIdentityResolverKind::ExplicitFName;
	OptionalAmmoDescriptor.IdentitySourceName = TEXT("AmmoId");
	OptionalAmmoDescriptor.ValidationPolicy = ECFDAValidationPolicy::None;
	TestTrue(
		TEXT("Synthetic Optional Ammo descriptor registers"),
		OptionalIdentityRegistry.RegisterDescriptor(OptionalAmmoDescriptor, &RegistrationError));

	const FCFDALoadedAssetResult OptionalMissingAmmoResult =
		FCFDATypeAdapter::EvaluateLoadedAsset(
			ValidAmmoRecord,
			MissingAmmo,
			OptionalIdentityRegistry,
			3);
	TestEqual(TEXT("Optional missing evaluation succeeds"), OptionalMissingAmmoResult.EvaluationState, ECFDAEvaluationState::Succeeded);
	TestEqual(TEXT("Optional missing Ammo ID remains NotResolved"), OptionalMissingAmmoResult.StableIdState, ECFDAStableIdState::NotResolved);
	TestTrue(TEXT("Optional missing Ammo ID is not Error"), OptionalMissingAmmoResult.HealthState != ECFDAHealthState::Error);

	UCFEquipmentItemData* EquipmentItem = NewObject<UCFEquipmentItemData>();
	EquipmentItem->ItemDefinitionId = TEXT("DAM_Test_EquipmentItem");
	const FCFDAAssetRecord EquipmentItemRecord =
		CFDAHealthTestsPrivate::MakeRecord(
			UCFEquipmentItemData::StaticClass(),
			TEXT("/Transient/DAM_Test_EquipmentItem"));
	const FCFDALoadedAssetResult EquipmentItemResult =
		FCFDATypeAdapter::EvaluateLoadedAsset(
			EquipmentItemRecord,
			EquipmentItem,
			TypeRegistry,
			3);
	TestEqual(TEXT("Equipment item evaluation succeeds"), EquipmentItemResult.EvaluationState, ECFDAEvaluationState::Succeeded);
	TestEqual(TEXT("Equipment item PrimaryAssetId resolved"), EquipmentItemResult.StableIdState, ECFDAStableIdState::Resolved);
	TestEqual(TEXT("Equipment item namespace is PrimaryAssetType"), EquipmentItemResult.DuplicateNamespace, FString(TEXT("CFEquipmentItem")));
	TestEqual(TEXT("Equipment item canonical name is original ItemDefinitionId"), EquipmentItemResult.CanonicalStableIdentity.NameValue, FName(TEXT("DAM_Test_EquipmentItem")));
	TestEqual(TEXT("Equipment item native validation detects missing reference"), EquipmentItemResult.HealthState, ECFDAHealthState::Error);

	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>();
	const FCFDAAssetRecord RecipeRecord =
		CFDAHealthTestsPrivate::MakeRecord(
			UCFVehicleRecipeData::StaticClass(),
			TEXT("/Transient/DAM_Test_Recipe"));
	const FCFDALoadedAssetResult RecipeResult =
		FCFDATypeAdapter::EvaluateLoadedAsset(
			RecipeRecord,
			Recipe,
			TypeRegistry,
			3);
	TestEqual(TEXT("Recipe evaluation succeeds"), RecipeResult.EvaluationState, ECFDAEvaluationState::Succeeded);
	TestEqual(TEXT("Recipe Guid resolved"), RecipeResult.StableIdState, ECFDAStableIdState::Resolved);
	TestEqual(TEXT("Recipe canonical kind Guid"), RecipeResult.CanonicalStableIdentity.ValueKind, ECFDAStableIdentityValueKind::Guid);
	TestEqual(TEXT("Recipe canonical Guid preserves source value"), RecipeResult.CanonicalStableIdentity.GuidValue, Recipe->RecipeId);
	TestEqual(TEXT("Recipe has no validation rule"), RecipeResult.HealthState, ECFDAHealthState::NotApplicable);

	UCFVehicleCameraData* CameraData = NewObject<UCFVehicleCameraData>();
	const FCFDAAssetRecord CameraRecord =
		CFDAHealthTestsPrivate::MakeRecord(
			UCFVehicleCameraData::StaticClass(),
			TEXT("/Transient/DAM_Test_Camera"));
	const FCFDALoadedAssetResult CameraResult =
		FCFDATypeAdapter::EvaluateLoadedAsset(
			CameraRecord,
			CameraData,
			TypeRegistry,
			3);
	TestEqual(TEXT("Camera evaluation succeeds"), CameraResult.EvaluationState, ECFDAEvaluationState::Succeeded);
	TestEqual(TEXT("Camera stable identity N/A"), CameraResult.StableIdState, ECFDAStableIdState::NotApplicable);
	TestEqual(TEXT("Camera duplicate N/A"), CameraResult.DuplicateState, ECFDADuplicateState::NotApplicable);
	TestEqual(TEXT("Camera validation N/A"), CameraResult.HealthState, ECFDAHealthState::NotApplicable);

	// descriptor/source drift는 DA contract Error가 아니라 AdapterUnavailable입니다.
	FCFDATypeRegistry BrokenAdapterRegistry;
	FCFDASemanticDescriptor BrokenAmmoDescriptor;
	BrokenAmmoDescriptor.ClassPath = UCFAmmoData::StaticClass()->GetClassPathName().ToString();
	BrokenAmmoDescriptor.Domain = ECFDADomain::Combat;
	BrokenAmmoDescriptor.TypeDisplayName = TEXT("깨진 탄약 Adapter 테스트");
	BrokenAmmoDescriptor.RoleDescription = TEXT("운영 adapter failure와 DA Health 분리를 검증합니다.");
	BrokenAmmoDescriptor.IdentityPolicy = ECFDAIdentityPolicy::Required;
	BrokenAmmoDescriptor.IdentityResolverKind = ECFDAIdentityResolverKind::ExplicitFName;
	BrokenAmmoDescriptor.IdentitySourceName = TEXT("MissingAmmoId");
	BrokenAmmoDescriptor.ValidationPolicy = ECFDAValidationPolicy::None;
	TestTrue(
		TEXT("Broken adapter descriptor structurally registers"),
		BrokenAdapterRegistry.RegisterDescriptor(BrokenAmmoDescriptor, &RegistrationError));

	const FCFDALoadedAssetResult BrokenAdapterResult =
		FCFDATypeAdapter::EvaluateLoadedAsset(
			ValidAmmoRecord,
			ValidAmmo,
			BrokenAdapterRegistry,
			3);
	TestEqual(TEXT("Broken source reports AdapterUnavailable"), BrokenAdapterResult.EvaluationState, ECFDAEvaluationState::AdapterUnavailable);
	TestEqual(TEXT("Broken source does not accuse DA Health"), BrokenAdapterResult.HealthState, ECFDAHealthState::NotValidated);
	TestEqual(TEXT("Broken source stable ID stays unresolved"), BrokenAdapterResult.StableIdState, ECFDAStableIdState::NotResolved);
	return true;
}

// typed canonical equality, namespace/generation 분리와 incomplete coverage semantics를 pure seam에서 검증합니다.
bool FCFDADuplicateHealthTest::RunTest(const FString& Parameters)
{
	FCFDALoadedAssetResult First =
		CFDAHealthTestsPrivate::MakeResolvedNameResult(
			10,
			TEXT("/Game/A.A"),
			TEXT("/Script/CarFight_Re.CFWeaponData"),
			FName(TEXT("SharedId")),
			true);

	// 표시 문자열의 case가 달라도 FName identity equality를 그대로 사용해야 합니다.
	FCFDALoadedAssetResult Second =
		CFDAHealthTestsPrivate::MakeResolvedNameResult(
			10,
			TEXT("/Game/B.B"),
			TEXT("/Script/CarFight_Re.CFWeaponData"),
			FName(TEXT("sharedid")),
			true);

	FCFDALoadedAssetResult DifferentNamespace =
		CFDAHealthTestsPrivate::MakeResolvedNameResult(
			10,
			TEXT("/Game/C.C"),
			TEXT("/Script/CarFight_Re.CFAmmoData"),
			FName(TEXT("SharedId")),
			true);

	FCFDALoadedAssetResult DifferentGeneration =
		CFDAHealthTestsPrivate::MakeResolvedNameResult(
			11,
			TEXT("/Game/D.D"),
			TEXT("/Script/CarFight_Re.CFWeaponData"),
			FName(TEXT("SharedId")),
			true);

	FCFDALoadedAssetResult IncompleteCoverage =
		CFDAHealthTestsPrivate::MakeResolvedNameResult(
			10,
			TEXT("/Game/E.E"),
			TEXT("/Script/CarFight_Re.CFProjectileData"),
			FName(TEXT("ApparentlyUnique")),
			false);

	FCFDALoadedAssetResult NotApplicable;
	NotApplicable.InventoryGeneration = 10;
	NotApplicable.ObjectPath = TEXT("/Game/F.F");
	NotApplicable.EvaluationState = ECFDAEvaluationState::Succeeded;
	NotApplicable.StableIdState = ECFDAStableIdState::NotApplicable;
	NotApplicable.HealthState = ECFDAHealthState::NotApplicable;

	TArray<FCFDALoadedAssetResult> Results =
	{
		First,
		Second,
		DifferentNamespace,
		DifferentGeneration,
		IncompleteCoverage,
		NotApplicable
	};

	FCFDAHealthService::AnalyzeDuplicateStableIds(Results);

	TestEqual(TEXT("First duplicate marked"), Results[0].DuplicateState, ECFDADuplicateState::Duplicate);
	TestEqual(TEXT("Second duplicate marked through FName equality"), Results[1].DuplicateState, ECFDADuplicateState::Duplicate);
	TestEqual(TEXT("First duplicate raises Error"), Results[0].HealthState, ECFDAHealthState::Error);
	TestEqual(TEXT("Second duplicate raises Error"), Results[1].HealthState, ECFDAHealthState::Error);
	TestEqual(TEXT("Different namespace is Unique"), Results[2].DuplicateState, ECFDADuplicateState::Unique);
	TestEqual(TEXT("Different generation is Unique in its generation"), Results[3].DuplicateState, ECFDADuplicateState::Unique);
	TestEqual(TEXT("Incomplete namespace cannot claim Unique"), Results[4].DuplicateState, ECFDADuplicateState::NotAnalyzed);
	TestEqual(TEXT("Identity N/A duplicate state is N/A"), Results[5].DuplicateState, ECFDADuplicateState::NotApplicable);
	TestEqual(TEXT("Different namespace Health preserved"), Results[2].HealthState, ECFDAHealthState::OK);
	TestEqual(TEXT("Different generation Health preserved"), Results[3].HealthState, ECFDAHealthState::OK);
	return true;
}

#endif
