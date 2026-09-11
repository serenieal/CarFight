// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseMixedTests.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-02 VehicleDefenseData fourth-type mixed operational admission focused Automation입니다.
// Changelog:
// - v1.0.0: explicit exact4 admission, VehicleDefense+Damage class-scoped StableLogicalId durable coexistence, global TargetObjectPath duplicate와 post-Review source stale mutation0를 actual provider path로 검증합니다.
// Migration:
// - 실제 Save/Delete는 /Game/Test/CarFight/VDRDefenseP02와 provider __AutomationP02__ roots에만 한정합니다.
// - Product assets/canonical Staging, protected DA_VehicleDefense_Test와 predecessor accepted DACE histories는 mutation하지 않습니다.

#include "CFDADamageProvider.h"
#include "CFDAVehicleDefenseDace.h"
#include "CFDAVehicleDefenseProvider.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingOps.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFDamageData.h"
#include "CFVehicleDefenseData.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAVehicleDefenseMixedTestsPrivate
{
	// VDR-P0-02 disposable Content package root입니다.
	static const FString FixturePackageRoot = TEXT("/Game/Test/CarFight/VDRDefenseP02");

	// VDR-P0-02 VehicleDefense provider-owned disposable Staging root입니다.
	static const FString VehicleDefenseFixtureStagingRoot = TEXT("Authoring/DataAssetStaging/VehicleDefenseData/__AutomationP02__");

	// VDR-P0-02 Damage provider-owned disposable Staging root입니다.
	static const FString DamageFixtureStagingRoot = TEXT("Authoring/DataAssetStaging/DamageData/__AutomationVDRP02__");

	// Same textual identity를 두 exact classes에서 공유하는 mixed fixture path 묶음입니다.
	struct FTwoTypeFixturePaths
	{
		// 두 exact class가 공유해도 되는 textual StableLogicalId입니다.
		FString StableLogicalId;

		// VehicleDefense UObject name입니다.
		FString VehicleDefenseAssetName;

		// VehicleDefense package long name입니다.
		FString VehicleDefensePackageName;

		// VehicleDefense exact target object path입니다.
		FString VehicleDefenseTargetObjectPath;

		// VehicleDefense provider-owned Staging source path입니다.
		FString VehicleDefenseStagingRelativePath;

		// Damage UObject name입니다.
		FString DamageAssetName;

		// Damage package long name입니다.
		FString DamagePackageName;

		// Damage exact target object path입니다.
		FString DamageTargetObjectPath;

		// Damage provider-owned Staging source path입니다.
		FString DamageStagingRelativePath;
	};

	// GUID suffix로 collision 없는 same-text identity + two-class target/source paths를 생성합니다.
	FTwoTypeFixturePaths BuildFixturePaths(const FString& Prefix)
	{
		// Collision 방지용 short GUID suffix입니다.
		const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);
		// 두 provider가 공유할 textual identity입니다.
		const FString StableLogicalId = FString::Printf(TEXT("%s_%s"), *Prefix, *Suffix);
		// 반환할 exact two-type fixture paths입니다.
		FTwoTypeFixturePaths Paths;
		Paths.StableLogicalId = StableLogicalId;
		Paths.VehicleDefenseAssetName = FString::Printf(TEXT("DA_Defense_%s"), *Suffix);
		Paths.VehicleDefensePackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.VehicleDefenseAssetName);
		Paths.VehicleDefenseTargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.VehicleDefensePackageName, *Paths.VehicleDefenseAssetName);
		Paths.VehicleDefenseStagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *VehicleDefenseFixtureStagingRoot, *StableLogicalId);
		Paths.DamageAssetName = FString::Printf(TEXT("DA_Damage_%s"), *Suffix);
		Paths.DamagePackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.DamageAssetName);
		Paths.DamageTargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.DamagePackageName, *Paths.DamageAssetName);
		Paths.DamageStagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *DamageFixtureStagingRoot, *StableLogicalId);
		return Paths;
	}

	// 한 directional armor config의 valid exact2 authored 값을 생성합니다.
	FCFDirectionalArmorConfig BuildArmorConfig(const float MaximumArmor, const float DamageMultiplier)
	{
		// 반환할 exact2 directional armor config입니다.
		FCFDirectionalArmorConfig Config;
		Config.MaximumArmor = MaximumArmor;
		Config.DamageMultiplier = DamageMultiplier;
		return Config;
	}

	// Mixed durable fixture용 valid VehicleDefense typed payload를 생성합니다.
	FCFDAVehicleDefensePayload BuildVehicleDefensePayload(const FString& StableLogicalId, const float ArmorResistance)
	{
		// 반환할 valid VehicleDefense whole-record payload입니다.
		FCFDAVehicleDefensePayload Payload;
		Payload.DefenseId = FName(*StableLogicalId);
		Payload.DefenseMassKg = 135.0f;
		Payload.bUseShield = true;
		Payload.MaximumShield = 520.0f;
		Payload.ShieldRegenerationDelaySeconds = 3.0f;
		Payload.ShieldRegenerationPerSecond = 18.0f;
		Payload.ArmorType = ECFArmorType::Heavy;
		Payload.ArmorResistance = ArmorResistance;
		Payload.FrontArmorConfig = BuildArmorConfig(310.0f, 0.75f);
		Payload.LeftArmorConfig = BuildArmorConfig(210.0f, 0.92f);
		Payload.RightArmorConfig = BuildArmorConfig(211.0f, 0.93f);
		Payload.RearArmorConfig = BuildArmorConfig(165.0f, 1.08f);
		Payload.TopArmorConfig = BuildArmorConfig(105.0f, 1.18f);
		Payload.BottomArmorConfig = BuildArmorConfig(125.0f, 1.12f);
		Payload.ShieldComponentDamageScale = 0.20f;
		Payload.ArmorComponentDamageScale = 0.35f;
		Payload.IntegrityComponentDamageScale = 0.50f;
		return Payload;
	}

	// Mixed durable fixture용 valid Damage typed payload를 생성합니다.
	FCFDADamagePayload BuildDamagePayload(const FString& StableLogicalId, const float BaseDamage)
	{
		// 반환할 valid Damage whole-record payload입니다.
		FCFDADamagePayload Payload;
		Payload.DamageId = FName(*StableLogicalId);
		Payload.DamageType = ECFDamageType::Explosive;
		Payload.BaseDamage = BaseDamage;
		Payload.bCanDamageSelf = false;
		Payload.ArmorPenetration = 15.0f;
		Payload.bUseRadialDamage = false;
		Payload.ExplosionRadius = 120.0f;
		Payload.ExplosionInnerRadius = 250.0f;
		Payload.ExplosionDamage = 70.0f;
		Payload.MinExplosionDamageScale = 0.4f;
		Payload.ModuleDamageScale = 0.8f;
		Payload.ImpulseStrength = 700.0f;
		return Payload;
	}

	// Production VehicleDefense serializer로 disposable Create JSON을 생성합니다.
	bool BuildVehicleDefenseJson(
		const FTwoTypeFixturePaths& Paths,
		const float ArmorResistance,
		const FString& TargetObjectPath,
		FString& OutJson,
		FString& OutError)
	{
		// Exact desired VehicleDefense payload입니다.
		const FCFDAVehicleDefensePayload Payload = BuildVehicleDefensePayload(Paths.StableLogicalId, ArmorResistance);
		return CFDAVehicleDefenseProviderImpl::SerializeStagingJson(Payload, TargetObjectPath, FString(), OutJson, OutError);
	}

	// Production Damage serializer로 disposable Create JSON을 생성합니다.
	bool BuildDamageJson(
		const FTwoTypeFixturePaths& Paths,
		const float BaseDamage,
		const FString& TargetObjectPath,
		FString& OutJson,
		FString& OutError)
	{
		// Exact desired Damage payload입니다.
		const FCFDADamagePayload Payload = BuildDamagePayload(Paths.StableLogicalId, BaseDamage);
		return CFDADamageProviderImpl::SerializeStagingJson(Payload, TargetObjectPath, FString(), OutJson, OutError);
	}

	// CarFight main_game root를 Unreal ProjectDir의 부모로 계산합니다.
	FString GetMainGameRoot()
	{
		// `<main_game>/UE/` absolute project root입니다.
		const FString UnrealProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		// `<main_game>/` absolute repository root입니다.
		FString MainGameRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(UnrealProjectRoot, TEXT("..")));
		FPaths::NormalizeDirectoryName(MainGameRoot);
		return MainGameRoot;
	}

	// main_game-relative Staging source를 exact absolute path로 변환합니다.
	FString GetStagingAbsolutePath(const FString& StagingRelativePath)
	{
		// Exact staging absolute filename입니다.
		FString AbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(GetMainGameRoot(), StagingRelativePath));
		FPaths::NormalizeFilename(AbsolutePath);
		return AbsolutePath;
	}

	// Test-owned provider Staging JSON을 UTF-8 without BOM으로 기록합니다.
	bool WriteStagingFile(const FString& StagingRelativePath, const FString& JsonText)
	{
		// Exact staging absolute filename입니다.
		const FString AbsolutePath = GetStagingAbsolutePath(StagingRelativePath);
		// Provider-owned disposable parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(AbsolutePath);
		IFileManager::Get().MakeDirectory(*ParentDirectory, true);
		return FFileHelper::SaveStringToFile(JsonText, *AbsolutePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	// UObject path가 VDR-P0-02 disposable Content root에 속하는지 확인합니다.
	bool IsFixtureObjectPath(const FString& ObjectPath)
	{
		return ObjectPath.StartsWith(FixturePackageRoot + TEXT("/"), ESearchCase::CaseSensitive);
	}

	// Exact class의 loaded disposable UObject package를 teardown 목록에 수집합니다.
	template <typename AssetType>
	void CollectLoadedFixturePackages(TArray<UPackage*>& InOutPackagesToUnload, TArray<FString>& InOutDeletedFilenames)
	{
		for (TObjectIterator<AssetType> AssetIterator; AssetIterator; ++AssetIterator)
		{
			// Current process loaded exact-class fixture 후보입니다.
			AssetType* LoadedAsset = *AssetIterator;
			if (LoadedAsset == nullptr
				|| LoadedAsset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
				|| !IsFixtureObjectPath(FSoftObjectPath(LoadedAsset).ToString()))
			{
				continue;
			}
			// Loaded fixture owning package입니다.
			UPackage* LoadedPackage = LoadedAsset->GetOutermost();
			if (LoadedPackage != nullptr)
			{
				LoadedPackage->SetDirtyFlag(false);
				InOutPackagesToUnload.AddUnique(LoadedPackage);
				// Loaded-only package도 registry refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(LoadedPackage->GetName(), FPackageName::GetAssetPackageExtension());
				InOutDeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedAsset->ClearFlags(RF_Standalone);
		}
	}

	// Exact class의 loaded disposable UObject가 남았는지 확인합니다.
	template <typename AssetType>
	bool HasLoadedFixtureObject()
	{
		for (TObjectIterator<AssetType> AssetIterator; AssetIterator; ++AssetIterator)
		{
			// Current process loaded exact-class fixture 후보입니다.
			const AssetType* LoadedAsset = *AssetIterator;
			if (LoadedAsset != nullptr
				&& !LoadedAsset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
				&& IsFixtureObjectPath(FSoftObjectPath(LoadedAsset).ToString()))
			{
				return true;
			}
		}
		return false;
	}

	// AssetRegistry/loaded UObject/physical Content/two provider Staging roots가 residue0인지 검증합니다.
	bool VerifyFixtureResidueFree(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Disposable root 아래 registry-visible persisted assets입니다.
		TArray<FAssetData> RegistryAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), RegistryAssets, true, true);
		if (!RegistryAssets.IsEmpty())
		{
			OutError = FString::Printf(TEXT("VDR-P0-02 AssetRegistry residue가 %d개 남았습니다."), RegistryAssets.Num());
			return false;
		}
		if (HasLoadedFixtureObject<UCFVehicleDefenseData>())
		{
			OutError = TEXT("VDR-P0-02 loaded VehicleDefense UObject residue가 남았습니다.");
			return false;
		}
		if (HasLoadedFixtureObject<UCFDamageData>())
		{
			OutError = TEXT("VDR-P0-02 loaded Damage UObject residue가 남았습니다.");
			return false;
		}

		// Disposable physical Content directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/VDRDefenseP02")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = TEXT("VDR-P0-02 physical Content root residue가 남았습니다.");
			return false;
		}
		// Disposable provider Staging roots입니다.
		const TArray<FString> StagingDirectories =
		{
			GetStagingAbsolutePath(VehicleDefenseFixtureStagingRoot),
			GetStagingAbsolutePath(DamageFixtureStagingRoot)
		};
		for (const FString& StagingDirectory : StagingDirectories)
		{
			if (IFileManager::Get().DirectoryExists(*StagingDirectory))
			{
				OutError = FString::Printf(TEXT("VDR-P0-02 Staging residue가 남았습니다: %s"), *StagingDirectory);
				return false;
			}
		}
		OutError.Reset();
		return true;
	}

	// VDR-P0-02 disposable Content와 provider Staging roots만 unload→GC→delete→registry refresh 순서로 정리합니다.
	bool CleanupFixtureRoots(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Registry-visible disposable assets입니다.
		TArray<FAssetData> FixtureAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), FixtureAssets, true, true);
		// Unload할 unique disposable packages입니다.
		TArray<UPackage*> PackagesToUnload;
		// Disk delete 뒤 registry stale removal에 사용할 filenames입니다.
		TArray<FString> DeletedFilenames;
		for (const FAssetData& FixtureAssetData : FixtureAssets)
		{
			// Registry-visible fixture package입니다.
			UPackage* FixturePackage = FindPackage(nullptr, *FixtureAssetData.PackageName.ToString());
			if (FixturePackage == nullptr)
			{
				// Persisted fixture를 load해 owning package handle을 확보합니다.
				UObject* FixtureObject = FixtureAssetData.GetAsset();
				FixturePackage = FixtureObject != nullptr ? FixtureObject->GetOutermost() : nullptr;
			}
			if (FixturePackage != nullptr)
			{
				FixturePackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(FixturePackage);
			}
			// Persisted fixture의 expected physical package filename입니다.
			const FString PackageFilename = FPackageName::LongPackageNameToFilename(FixtureAssetData.PackageName.ToString(), FPackageName::GetAssetPackageExtension());
			DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(PackageFilename));
		}

		CollectLoadedFixturePackages<UCFVehicleDefenseData>(PackagesToUnload, DeletedFilenames);
		CollectLoadedFixturePackages<UCFDamageData>(PackagesToUnload, DeletedFilenames);
		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("VDR-P0-02 disposable package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Disposable physical Content directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/VDRDefenseP02")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = TEXT("VDR-P0-02 physical Content root 삭제에 실패했습니다.");
			return false;
		}
		if (!DeletedFilenames.IsEmpty())
		{
			AssetRegistry.ScanModifiedAssetFiles(DeletedFilenames);
			AssetRegistry.WaitForCompletion();
		}

		// Disposable provider Staging roots입니다.
		const TArray<FString> StagingDirectories =
		{
			GetStagingAbsolutePath(VehicleDefenseFixtureStagingRoot),
			GetStagingAbsolutePath(DamageFixtureStagingRoot)
		};
		for (const FString& StagingDirectory : StagingDirectories)
		{
			if (IFileManager::Get().DirectoryExists(*StagingDirectory)
				&& !IFileManager::Get().DeleteDirectory(*StagingDirectory, false, true))
			{
				OutError = FString::Printf(TEXT("VDR-P0-02 Staging root 삭제에 실패했습니다: %s"), *StagingDirectory);
				return false;
			}
		}
		return VerifyFixtureResidueFree(OutError);
	}

	// VehicleDefense+Damage exact2 Create sources를 production serializers로 생성해 disk에 기록합니다.
	bool WriteTwoTypeCreateSources(
		const FTwoTypeFixturePaths& Paths,
		FString& OutVehicleDefenseJson,
		FString& OutDamageJson,
		FString& OutError)
	{
		if (!BuildVehicleDefenseJson(Paths, 88.0f, Paths.VehicleDefenseTargetObjectPath, OutVehicleDefenseJson, OutError)
			|| !BuildDamageJson(Paths, 46.0f, Paths.DamageTargetObjectPath, OutDamageJson, OutError))
		{
			return false;
		}
		if (!WriteStagingFile(Paths.VehicleDefenseStagingRelativePath, OutVehicleDefenseJson)
			|| !WriteStagingFile(Paths.DamageStagingRelativePath, OutDamageJson))
		{
			OutError = TEXT("VDR-P0-02 two-type Staging source write에 실패했습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Preview 전체에 특정 issue code가 존재하는지 검사합니다.
	bool PreviewHasIssueCode(const FCFDAStagingOpsPreview& Preview, const ECFDAStagingIssueCode IssueCode)
	{
		for (const FCFDAStagingPreviewRow& Row : Preview.Rows)
		{
			for (const FCFDAStagingIssue& Issue : Row.Issues)
			{
				if (Issue.Code == IssueCode)
				{
					return true;
				}
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefMixedAdmissionTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_02.OperationalAdmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefMixedDurableTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_02.CrossTypeDurable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefMixedDuplicateTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_02.TargetDuplicate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefMixedStaleTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_02.SourceStale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Actual production backing authority exact4와 VehicleDefense operational qualification을 직접 검증합니다.
bool FCFDAVDefMixedAdmissionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseMixedTestsPrivate;

	// Normalize/Discover가 실제 사용하는 production operational backing SchemaIds입니다.
	TArray<FString> SchemaIds;
	// Normalize/Discover가 실제 사용하는 production operational backing class paths입니다.
	TArray<FString> ClassPaths;
	FCFDAStagingOpsTestControl::GetMixedOperationalAllowedTypeKeys(SchemaIds, ClassPaths);
	TestEqual(TEXT("Production mixed backing SchemaId count is exact4"), SchemaIds.Num(), 4);
	TestEqual(TEXT("Production mixed backing ClassPath count is exact4"), ClassPaths.Num(), 4);
	if (SchemaIds.Num() == 4 && ClassPaths.Num() == 4)
	{
		TestEqual(TEXT("Operational TypeKey[0] remains MissileGuidePreset"), SchemaIds[0], FString(TEXT("CarFight.DataAsset.MissileGuidePreset")));
		TestEqual(TEXT("Operational TypeKey[1] remains AmmoData"), SchemaIds[1], FString(TEXT("CarFight.DataAsset.AmmoData")));
		TestEqual(TEXT("Operational TypeKey[2] remains DamageData"), SchemaIds[2], FString(TEXT("CarFight.DataAsset.DamageData")));
		TestEqual(TEXT("Operational TypeKey[3] is VehicleDefenseData"), SchemaIds[3], FString(TEXT("CarFight.DataAsset.VehicleDefenseData")));
		TestEqual(TEXT("Operational VehicleDefense class path exact"), ClassPaths[3], FString(TEXT("/Script/CarFight_Re.CFVehicleDefenseData")));
	}

	// Current VehicleDefense production provider authority입니다.
	const FCFDAVehicleDefenseProvider& VehicleDefenseProvider = CFDAVehicleDefenseProvider::GetProvider();
	TestEqual(TEXT("VehicleDefense provider readiness is ReviewedMutationReady"), VehicleDefenseProvider.Readiness, ECFDAProviderReadiness::ReviewedMutationReady);
	TestEqual(TEXT("VehicleDefense DACE readiness is ContractReady"), VehicleDefenseProvider.Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestTrue(TEXT("VehicleDefense DACE migration gate remains valid before admission use"), FCFDAVehicleDefenseDace::EvaluateCurrentMigrationGate().Validation.bPassed);

	// Fixture setup/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("OperationalAdmission pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Actual VehicleDefense admission fixture paths입니다.
	const FTwoTypeFixturePaths Paths = BuildFixturePaths(TEXT("VDRP02Adm"));
	// Serialized VehicleDefense Create JSON입니다.
	FString VehicleDefenseJson;
	if (!BuildVehicleDefenseJson(Paths, 88.0f, Paths.VehicleDefenseTargetObjectPath, VehicleDefenseJson, PhaseError)
		|| !WriteStagingFile(Paths.VehicleDefenseStagingRelativePath, VehicleDefenseJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("VehicleDefense operational admission source setup failed") : PhaseError);
		return false;
	}

	// Actual production mixed session입니다.
	FCFDAStagingOpsSession Session;
	// VehicleDefense-only operational Preview projection입니다.
	FCFDAStagingOpsPreview Preview;
	if (!TestTrue(TEXT("Production mixed session admits explicit VehicleDefense path"), Session.Preview({Paths.VehicleDefenseStagingRelativePath}, Preview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestFalse(TEXT("VehicleDefense-only operational Preview is not blocked"), Preview.bBlocked);
	TestEqual(TEXT("VehicleDefense-only operational Preview has one row"), Preview.Rows.Num(), 1);
	TestEqual(TEXT("VehicleDefense-only operational Preview classifies Create1"), Preview.CreateCount, 1);
	return !HasAnyErrors();
}

// VehicleDefense+Damage가 같은 textual StableLogicalId를 class-scoped로 공유하면서 durable batch를 완주하는지 검증합니다.
bool FCFDAVDefMixedDurableTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseMixedTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("CrossTypeDurable pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Same textual ID를 공유하는 actual provider two-type paths입니다.
	const FTwoTypeFixturePaths Paths = BuildFixturePaths(TEXT("VDRP02Dur"));
	// VehicleDefense Create JSON입니다.
	FString VehicleDefenseJson;
	// Damage Create JSON입니다.
	FString DamageJson;
	if (!TestTrue(TEXT("VehicleDefense+Damage Create sources are written"), WriteTwoTypeCreateSources(Paths, VehicleDefenseJson, DamageJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Input order A는 VehicleDefense→Damage입니다.
	const TArray<FString> SelectionA = {Paths.VehicleDefenseStagingRelativePath, Paths.DamageStagingRelativePath};
	// Input order B는 Damage→VehicleDefense입니다.
	const TArray<FString> SelectionB = {Paths.DamageStagingRelativePath, Paths.VehicleDefenseStagingRelativePath};
	// Primary reviewed session입니다.
	FCFDAStagingOpsSession SessionA;
	// Reversed-order hash comparison session입니다.
	FCFDAStagingOpsSession SessionB;
	// Primary Preview입니다.
	FCFDAStagingOpsPreview PreviewA;
	// Reversed-order Preview입니다.
	FCFDAStagingOpsPreview PreviewB;
	if (!SessionA.Preview(SelectionA, PreviewA, PhaseError)
		|| !SessionB.Preview(SelectionB, PreviewB, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	TestFalse(TEXT("VehicleDefense+Damage Preview is not blocked"), PreviewA.bBlocked);
	TestEqual(TEXT("VehicleDefense+Damage Preview rows exact2"), PreviewA.Rows.Num(), 2);
	TestEqual(TEXT("VehicleDefense+Damage Create count exact2"), PreviewA.CreateCount, 2);
	TestEqual(TEXT("Cross-type input order keeps deterministic BatchPlanHash"), PreviewA.BatchPlanHash, PreviewB.BatchPlanHash);
	TestFalse(TEXT("Same textual ID across VehicleDefense/Damage is not DuplicateStableIdentity"), PreviewHasIssueCode(PreviewA, ECFDAStagingIssueCode::DuplicateStableIdentity));

	if (!TestTrue(TEXT("VehicleDefense+Damage Review succeeds"), SessionA.Review(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Exact two-type Apply terminal report입니다.
	FCFDAStagingApplyReport ApplyReport;
	if (!TestTrue(TEXT("VehicleDefense+Damage ApplyReviewed succeeds"), SessionA.ApplyReviewed(ApplyReport, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("VehicleDefense+Damage batch result is DurableApplied"), ApplyReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("VehicleDefense+Damage durable applied count exact2"), ApplyReport.DurableAppliedCount, 2);
	TestEqual(TEXT("VehicleDefense+Damage NotRun count0"), ApplyReport.NotRunCount, 0);

	// Persisted VehicleDefense fixture readback입니다.
	UCFVehicleDefenseData* PersistedVehicleDefense = LoadObject<UCFVehicleDefenseData>(nullptr, *Paths.VehicleDefenseTargetObjectPath);
	// Persisted Damage fixture readback입니다.
	UCFDamageData* PersistedDamage = LoadObject<UCFDamageData>(nullptr, *Paths.DamageTargetObjectPath);
	if (TestNotNull(TEXT("Persisted VehicleDefense exists"), PersistedVehicleDefense))
	{
		// Production extractor readback payload입니다.
		FCFDAVehicleDefensePayload VehicleDefenseReadback;
		// Production extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> VehicleDefenseIssues;
		TestTrue(TEXT("Persisted VehicleDefense extractor succeeds"), CFDAVehicleDefenseProviderImpl::ExtractPayload(*PersistedVehicleDefense, VehicleDefenseReadback, VehicleDefenseIssues));
		TestEqual(TEXT("Persisted VehicleDefense class-scoped ID matches"), VehicleDefenseReadback.DefenseId, FName(*Paths.StableLogicalId));
		TestEqual(TEXT("Persisted VehicleDefense ArmorResistance matches"), VehicleDefenseReadback.ArmorResistance, 88.0f);
	}
	if (TestNotNull(TEXT("Persisted Damage exists"), PersistedDamage))
	{
		// Production Damage extractor readback payload입니다.
		FCFDADamagePayload DamageReadback;
		// Damage extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> DamageIssues;
		TestTrue(TEXT("Persisted Damage extractor succeeds"), CFDADamageProviderImpl::ExtractPayload(*PersistedDamage, DamageReadback, DamageIssues));
		TestEqual(TEXT("Persisted Damage class-scoped ID matches"), DamageReadback.DamageId, FName(*Paths.StableLogicalId));
		TestEqual(TEXT("Persisted Damage BaseDamage matches"), DamageReadback.BaseDamage, 46.0f);
	}
	return !HasAnyErrors();
}

// VehicleDefense와 Damage가 같은 TargetObjectPath를 요구하면 TypeKey와 무관하게 global duplicate로 차단되는지 검증합니다.
bool FCFDAVDefMixedDuplicateTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseMixedTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("TargetDuplicate pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Same textual identity를 공유하는 actual provider paths입니다.
	const FTwoTypeFixturePaths Paths = BuildFixturePaths(TEXT("VDRP02Dup"));
	// VehicleDefense와 Damage가 동시에 요구할 intentionally shared target path입니다.
	const FString SharedTargetObjectPath = Paths.VehicleDefenseTargetObjectPath;
	// Shared target을 요구하는 VehicleDefense JSON입니다.
	FString VehicleDefenseJson;
	// Shared target을 요구하는 Damage JSON입니다.
	FString DamageJson;
	if (!BuildVehicleDefenseJson(Paths, 88.0f, SharedTargetObjectPath, VehicleDefenseJson, PhaseError)
		|| !BuildDamageJson(Paths, 46.0f, SharedTargetObjectPath, DamageJson, PhaseError)
		|| !WriteStagingFile(Paths.VehicleDefenseStagingRelativePath, VehicleDefenseJson)
		|| !WriteStagingFile(Paths.DamageStagingRelativePath, DamageJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("VehicleDefense+Damage duplicate fixture setup failed") : PhaseError);
		return false;
	}

	// Duplicate negative production session입니다.
	FCFDAStagingOpsSession Session;
	// Global duplicate classification Preview입니다.
	FCFDAStagingOpsPreview Preview;
	if (!TestTrue(TEXT("VehicleDefense+Damage duplicate Preview completes with blocker projection"), Session.Preview({Paths.VehicleDefenseStagingRelativePath, Paths.DamageStagingRelativePath}, Preview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Same TargetObjectPath blocks VehicleDefense+Damage Preview"), Preview.bBlocked);
	TestTrue(TEXT("Cross-TypeKey target duplicate emits DuplicateTargetPath"), PreviewHasIssueCode(Preview, ECFDAStagingIssueCode::DuplicateTargetPath));
	TestFalse(TEXT("Cross-class same textual ID still does not emit DuplicateStableIdentity"), PreviewHasIssueCode(Preview, ECFDAStagingIssueCode::DuplicateStableIdentity));
	TestFalse(TEXT("Blocked duplicate cannot be Reviewed"), Session.Review(PhaseError));
	TestFalse(TEXT("Shared target was not persisted"), FPackageName::DoesPackageExist(Paths.VehicleDefensePackageName));
	TestFalse(TEXT("Damage own package path was not persisted"), FPackageName::DoesPackageExist(Paths.DamagePackageName));
	return !HasAnyErrors();
}

// Review 뒤 VehicleDefense source semantic이 바뀌면 mixed batch 전체가 all-before-mutation으로 차단되는지 검증합니다.
bool FCFDAVDefMixedStaleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseMixedTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("SourceStale pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Source-stale actual provider paths입니다.
	const FTwoTypeFixturePaths Paths = BuildFixturePaths(TEXT("VDRP02Src"));
	// Original VehicleDefense JSON입니다.
	FString VehicleDefenseJson;
	// Original Damage JSON입니다.
	FString DamageJson;
	if (!TestTrue(TEXT("Source-stale two-type sources are written"), WriteTwoTypeCreateSources(Paths, VehicleDefenseJson, DamageJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Source-stale production session입니다.
	FCFDAStagingOpsSession Session;
	// Source-stale initial Preview입니다.
	FCFDAStagingOpsPreview Preview;
	if (!Session.Preview({Paths.VehicleDefenseStagingRelativePath, Paths.DamageStagingRelativePath}, Preview, PhaseError)
		|| !Session.Review(PhaseError))
	{
		AddError(PhaseError);
		return false;
	}

	// Review 뒤 semantic drift를 가진 VehicleDefense JSON입니다.
	FString ChangedVehicleDefenseJson;
	if (!BuildVehicleDefenseJson(Paths, 89.0f, Paths.VehicleDefenseTargetObjectPath, ChangedVehicleDefenseJson, PhaseError)
		|| !WriteStagingFile(Paths.VehicleDefenseStagingRelativePath, ChangedVehicleDefenseJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("Post-Review VehicleDefense source rewrite failed") : PhaseError);
		return false;
	}

	// Source-stale global preflight report입니다.
	FCFDAStagingApplyReport ApplyReport;
	TestFalse(TEXT("Post-Review VehicleDefense source drift blocks mixed Apply"), Session.ApplyReviewed(ApplyReport, PhaseError));
	TestEqual(TEXT("Source-stale mixed batch blocks before mutation"), ApplyReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Source-stale mixed durable count remains0"), ApplyReport.DurableAppliedCount, 0);
	TestTrue(TEXT("Source-stale failed Apply consumes approval"), ApplyReport.bApprovalConsumed);
	TestFalse(TEXT("Source-stale VehicleDefense target remains unpersisted"), FPackageName::DoesPackageExist(Paths.VehicleDefensePackageName));
	TestFalse(TEXT("Source-stale Damage target remains unpersisted"), FPackageName::DoesPackageExist(Paths.DamagePackageName));
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
