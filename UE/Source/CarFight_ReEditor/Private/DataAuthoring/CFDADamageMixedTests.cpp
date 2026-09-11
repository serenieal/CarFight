// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageMixedTests.cpp
// Version: v1.2.0
// Date: 2026-09-11
// Description: CF-FQ-052 DDO-P0-04 DamageData operational admission + Missile/Ammo/Damage exact3 mixed integration focused Automation입니다.
// Changelog:
// - v1.2.0: VDR-P0-02 fourth-type admission 뒤 current production operational backing exact4를 반영하되 기존 Missile/Ammo/Damage first-three order와 exact3 lifecycle 회귀 의미를 그대로 보존합니다.
// - v1.1.0: OperationalAdmission에 unknown/out-of-scope production full-path fail-closed와 test-owned registry-only synthetic provider owner-capable / operational auto-admission0 direct regression을 추가합니다.
// - v1.0.0: production mixed backing authority exact3 direct regression, three-type durable lifecycle, global TargetObjectPath duplicate, Damage source/current TOCTOU mutation0를 actual-provider disposable fixture로 검증합니다.
// Migration:
// - v1.2.0부터 OperationalAdmission current-authority projection은 exact4를 기대하지만 ThreeTypeDurable/Duplicate/Stale는 predecessor Missile+Ammo+Damage exact3 subset regression으로 유지합니다.
// - v1.1.0은 production registry/allowlist/Normalize/Discover를 수정하지 않고 기존 test-owned provider-set owner seam과 actual production session만 사용합니다.
// - 실제 Save/Delete는 /Game/Test/CarFight/DDODamageP04 및 각 provider __AutomationP04__ Staging root에만 한정합니다.
// - Product Missile/Ammo/Damage asset, canonical Product Staging과 Missile/Ammo/Damage accepted DACE history는 mutation하지 않습니다.

#include "CFDAAmmoProvider.h"
#include "CFDADamageDace.h"
#include "CFDADamageProvider.h"
#include "CFDAMissileProvider.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingOps.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFAmmoData.h"
#include "CFDamageData.h"
#include "CFMissileGuidePresetData.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDADamageMixedTestsPrivate
{
	// DDO-P0-04 disposable Content package root입니다.
	static const FString FixturePackageRoot = TEXT("/Game/Test/CarFight/DDODamageP04");

	// DDO-P0-04 disposable Missile Staging source root입니다.
	static const FString MissileFixtureStagingRoot = TEXT("Authoring/DataAssetStaging/MissileGuidePreset/__AutomationP04__");

	// DDO-P0-04 disposable Ammo Staging source root입니다.
	static const FString AmmoFixtureStagingRoot = TEXT("Authoring/DataAssetStaging/AmmoData/__AutomationP04__");

	// DDO-P0-04 disposable Damage Staging source root입니다.
	static const FString DamageFixtureStagingRoot = TEXT("Authoring/DataAssetStaging/DamageData/__AutomationP04__");

	// 한 exact3 mixed fixture의 class-scoped identity와 provider별 target/source paths입니다.
	struct FThreeTypeFixturePaths
	{
		// 세 exact class가 공유해도 되는 textual StableLogicalId입니다.
		FString StableLogicalId;

		// Missile UObject name입니다.
		FString MissileAssetName;

		// Missile package long name입니다.
		FString MissilePackageName;

		// Missile exact target object path입니다.
		FString MissileTargetObjectPath;

		// Missile provider-owned Staging source path입니다.
		FString MissileStagingRelativePath;

		// Ammo UObject name입니다.
		FString AmmoAssetName;

		// Ammo package long name입니다.
		FString AmmoPackageName;

		// Ammo exact target object path입니다.
		FString AmmoTargetObjectPath;

		// Ammo provider-owned Staging source path입니다.
		FString AmmoStagingRelativePath;

		// Damage UObject name입니다.
		FString DamageAssetName;

		// Damage package long name입니다.
		FString DamagePackageName;

		// Damage exact target object path입니다.
		FString DamageTargetObjectPath;

		// Damage provider-owned Staging source path입니다.
		FString DamageStagingRelativePath;
	};

	// GUID suffix를 사용해 same textual identity + exact3 distinct-class target path 묶음을 만듭니다.
	FThreeTypeFixturePaths BuildFixturePaths(const FString& Prefix)
	{
		// Collision 방지용 short GUID suffix입니다.
		const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);
		// 세 provider가 공유할 class-scoped textual identity입니다.
		const FString StableLogicalId = FString::Printf(TEXT("%s_%s"), *Prefix, *Suffix);
		// 반환할 exact3 path 묶음입니다.
		FThreeTypeFixturePaths Paths;
		Paths.StableLogicalId = StableLogicalId;
		Paths.MissileAssetName = FString::Printf(TEXT("DA_Missile_%s"), *Suffix);
		Paths.MissilePackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.MissileAssetName);
		Paths.MissileTargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.MissilePackageName, *Paths.MissileAssetName);
		Paths.MissileStagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *MissileFixtureStagingRoot, *StableLogicalId);
		Paths.AmmoAssetName = FString::Printf(TEXT("DA_Ammo_%s"), *Suffix);
		Paths.AmmoPackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.AmmoAssetName);
		Paths.AmmoTargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.AmmoPackageName, *Paths.AmmoAssetName);
		Paths.AmmoStagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *AmmoFixtureStagingRoot, *StableLogicalId);
		Paths.DamageAssetName = FString::Printf(TEXT("DA_Damage_%s"), *Suffix);
		Paths.DamagePackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.DamageAssetName);
		Paths.DamageTargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.DamagePackageName, *Paths.DamageAssetName);
		Paths.DamageStagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *DamageFixtureStagingRoot, *StableLogicalId);
		return Paths;
	}

	// DDO-P0-04 Missile disposable fixture용 valid typed payload를 만듭니다.
	FCFDAMissilePresetPayload BuildMissilePayload(const FString& StableLogicalId, const float NavigationConstant)
	{
		// Exact valid Missile whole-record payload입니다.
		FCFDAMissilePresetPayload Payload;
		Payload.PresetId = FName(*StableLogicalId);
		Payload.PresetDisplayName.Text = TEXT("DDO P0-04 Mixed Missile");
		Payload.PresetDescription.Text = TEXT("DDO P0-04 disposable three-type integration fixture");
		Payload.MissileGuideConfig.bUseGuidance = true;
		Payload.MissileGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
		Payload.MissileGuideConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;
		Payload.MissileGuideConfig.NavigationConstant = NavigationConstant;
		Payload.MissileGuideConfig.MaximumTurnRateDegPerSec = 35.0f;
		Payload.MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 2000.0f;
		Payload.MissileGuideConfig.GuidanceResponseTimeSeconds = 0.18f;
		Payload.MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = 500.0f;
		Payload.MissileGuideConfig.SeekerFieldOfViewDeg = 60.0f;
		Payload.MissileGuideConfig.LockBreakAngleDeg = 85.0f;
		Payload.MissileGuideConfig.TargetLostGraceTimeSeconds = 0.2f;
		Payload.MissileGuideConfig.SeekerModel = ECFMissileSeekerModel::Stateful;
		Payload.MissileGuideConfig.TargetObservationMode = ECFMissileTargetObservationMode::SampledPositionEstimate;
		Payload.MissileGuideConfig.GuidanceLaw = ECFMissileGuidanceLaw::PurePursuit;
		Payload.MissileGuideConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::Independent;
		Payload.MissileGuideConfig.GuidanceActivationDelaySeconds = 0.25f;
		Payload.MissileGuideConfig.GuidanceActivationDistanceCm = 500.0f;
		Payload.MissileGuideConfig.LeadTimeSeconds = 0.0f;
		Payload.MissileGuideConfig.MaxLeadDistanceCm = 0.0f;
		Payload.MissileGuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
		Payload.MissileGuideConfig.TargetObservationIntervalSeconds = 0.08f;
		Payload.MissileGuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.25f;
		Payload.MissileGuideConfig.AcquisitionConeHalfAngleDeg = 45.0f;
		Payload.MissileGuideConfig.TrackingConeHalfAngleDeg = 60.0f;
		Payload.MissileGuideConfig.ReacquisitionConeHalfAngleDeg = 60.0f;
		Payload.MissileGuideConfig.ReacquisitionTimeSeconds = 0.0f;
		return Payload;
	}

	// DDO-P0-04 Ammo disposable fixture용 valid exact8 typed payload를 만듭니다.
	FCFDAAmmoPayload BuildAmmoPayload(const FString& StableLogicalId, const int32 MaximumCount)
	{
		// Exact valid Ammo whole-record payload입니다.
		FCFDAAmmoPayload Payload;
		Payload.AmmoId = FName(*StableLogicalId);
		Payload.AmmoDisplayName.Text = TEXT("DDO P0-04 Mixed Ammo");
		Payload.AmmoFamilyId = FName(TEXT("DDOAutomation"));
		Payload.UnitMassKg = 2.5f;
		Payload.AmmoTags = {FName(TEXT("Mixed")), FName(TEXT("Automation"))};
		Payload.AmmoIcon.Reset();
		Payload.MaximumLoadableAmmoCount = MaximumCount;
		Payload.bCanBeResupplied = true;
		return Payload;
	}

	// DDO-P0-04 Damage disposable fixture용 valid exact12 typed payload를 만듭니다.
	FCFDADamagePayload BuildDamagePayload(const FString& StableLogicalId, const float BaseDamage)
	{
		// Exact valid Damage whole-record payload입니다.
		FCFDADamagePayload Payload;
		Payload.DamageId = FName(*StableLogicalId);
		Payload.DamageType = ECFDamageType::Explosive;
		Payload.BaseDamage = BaseDamage;
		Payload.bCanDamageSelf = false;
		Payload.ArmorPenetration = 12.0f;
		Payload.bUseRadialDamage = false;
		Payload.ExplosionRadius = 125.0f;
		Payload.ExplosionInnerRadius = 300.0f;
		Payload.ExplosionDamage = 75.0f;
		Payload.MinExplosionDamageScale = 0.4f;
		Payload.ModuleDamageScale = 0.8f;
		Payload.ImpulseStrength = 720.0f;
		return Payload;
	}

	// Production Missile serializer로 disposable Create JSON을 만들고 BaseSemanticFingerprint를 JSON null로 정규화합니다.
	bool BuildMissileJson(
		const FThreeTypeFixturePaths& Paths,
		const float NavigationConstant,
		const FString& TargetObjectPath,
		FString& OutJson,
		FString& OutError)
	{
		// Exact desired Missile payload입니다.
		const FCFDAMissilePresetPayload Payload = BuildMissilePayload(Paths.StableLogicalId, NavigationConstant);
		if (!CFDAMissileProviderImpl::SerializeProductStagingJson(Payload, TargetObjectPath, FString(), OutJson, OutError))
		{
			return false;
		}

		// Production serializer 결과를 Create envelope로 다시 읽을 JSON reader입니다.
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(OutJson);
		// Production serializer가 만든 exact root object입니다.
		TSharedPtr<FJsonObject> RootObject;
		if (!FJsonSerializer::Deserialize(JsonReader, RootObject) || !RootObject.IsValid())
		{
			OutJson.Reset();
			OutError = TEXT("Missile production serializer 결과를 DDO-P0-04 Create fixture로 재parse하지 못했습니다.");
			return false;
		}
		RootObject->SetField(TEXT("BaseSemanticFingerprint"), MakeShared<FJsonValueNull>());
		OutJson.Reset();
		// Test-local null normalization 뒤 deterministic JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJson);
		if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), JsonWriter))
		{
			OutJson.Reset();
			OutError = TEXT("DDO-P0-04 Missile Create fixture JSON null normalization 직렬화에 실패했습니다.");
			return false;
		}
		JsonWriter->Close();
		OutJson += LINE_TERMINATOR;
		OutError.Reset();
		return true;
	}

	// Production Ammo serializer로 disposable Create JSON을 만듭니다.
	bool BuildAmmoJson(
		const FThreeTypeFixturePaths& Paths,
		const int32 MaximumCount,
		const FString& TargetObjectPath,
		FString& OutJson,
		FString& OutError)
	{
		// Exact desired Ammo payload입니다.
		const FCFDAAmmoPayload Payload = BuildAmmoPayload(Paths.StableLogicalId, MaximumCount);
		return CFDAAmmoProviderImpl::SerializeStagingJson(Payload, TargetObjectPath, FString(), OutJson, OutError);
	}

	// Production Damage serializer로 disposable Create JSON을 만듭니다.
	bool BuildDamageJson(
		const FThreeTypeFixturePaths& Paths,
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

	// main_game-relative Staging path를 exact absolute path로 변환합니다.
	FString GetStagingAbsolutePath(const FString& StagingRelativePath)
	{
		// Exact staging absolute filename입니다.
		FString AbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(GetMainGameRoot(), StagingRelativePath));
		FPaths::NormalizeFilename(AbsolutePath);
		return AbsolutePath;
	}

	// Exact disposable Staging JSON을 UTF-8 without BOM으로 기록합니다.
	bool WriteStagingFile(const FString& StagingRelativePath, const FString& JsonText)
	{
		// Exact staging absolute filename입니다.
		const FString AbsolutePath = GetStagingAbsolutePath(StagingRelativePath);
		// Provider-owned disposable parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(AbsolutePath);
		IFileManager::Get().MakeDirectory(*ParentDirectory, true);
		return FFileHelper::SaveStringToFile(JsonText, *AbsolutePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	// UObject path가 DDO-P0-04 disposable package root 아래인지 확인합니다.
	bool IsFixtureObjectPath(const FString& ObjectPath)
	{
		return ObjectPath.StartsWith(FixturePackageRoot + TEXT("/"), ESearchCase::CaseSensitive);
	}

	// Current process의 loaded fixture UObject 하나를 cleanup package 목록에 추가합니다.
	template <typename AssetType>
	void CollectLoadedFixturePackages(TArray<UPackage*>& InOutPackagesToUnload, TArray<FString>& InOutDeletedFilenames)
	{
		for (TObjectIterator<AssetType> AssetIterator; AssetIterator; ++AssetIterator)
		{
			// Current process의 loaded exact-class fixture 후보입니다.
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
				// Loaded-only package도 registry disk refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(LoadedPackage->GetName(), FPackageName::GetAssetPackageExtension());
				InOutDeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedAsset->ClearFlags(RF_Standalone);
		}
	}

	// Exact class의 loaded fixture UObject가 남았는지 확인합니다.
	template <typename AssetType>
	bool HasLoadedFixtureObject()
	{
		for (TObjectIterator<AssetType> AssetIterator; AssetIterator; ++AssetIterator)
		{
			// Current process의 loaded exact-class fixture 후보입니다.
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

	// Registry/loaded UObject/physical Content/provider Staging roots가 모두 residue0인지 검증합니다.
	bool VerifyFixtureResidueFree(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Disposable root 아래 persisted registry-visible assets입니다.
		TArray<FAssetData> RegistryAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), RegistryAssets, true, true);
		if (!RegistryAssets.IsEmpty())
		{
			OutError = FString::Printf(TEXT("DDO-P0-04 AssetRegistry residue가 %d개 남았습니다."), RegistryAssets.Num());
			return false;
		}
		if (HasLoadedFixtureObject<UCFMissileGuidePresetData>())
		{
			OutError = TEXT("DDO-P0-04 loaded Missile UObject residue가 남았습니다.");
			return false;
		}
		if (HasLoadedFixtureObject<UCFAmmoData>())
		{
			OutError = TEXT("DDO-P0-04 loaded Ammo UObject residue가 남았습니다.");
			return false;
		}
		if (HasLoadedFixtureObject<UCFDamageData>())
		{
			OutError = TEXT("DDO-P0-04 loaded Damage UObject residue가 남았습니다.");
			return false;
		}

		// Disposable physical Content directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DDODamageP04")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = TEXT("DDO-P0-04 physical Content root residue가 남았습니다.");
			return false;
		}
		// Disposable provider Staging directories입니다.
		const TArray<FString> StagingDirectories =
		{
			GetStagingAbsolutePath(MissileFixtureStagingRoot),
			GetStagingAbsolutePath(AmmoFixtureStagingRoot),
			GetStagingAbsolutePath(DamageFixtureStagingRoot)
		};
		for (const FString& StagingDirectory : StagingDirectories)
		{
			if (IFileManager::Get().DirectoryExists(*StagingDirectory))
			{
				OutError = FString::Printf(TEXT("DDO-P0-04 Staging residue가 남았습니다: %s"), *StagingDirectory);
				return false;
			}
		}
		OutError.Reset();
		return true;
	}

	// DDO-P0-04 disposable Content와 세 provider Staging roots만 unload→GC→delete→registry refresh 순서로 정리합니다.
	bool CleanupFixtureRoots(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Registry-visible disposable assets입니다.
		TArray<FAssetData> FixtureAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), FixtureAssets, true, true);
		// Unload할 unique disposable packages입니다.
		TArray<UPackage*> PackagesToUnload;
		// Disk delete 뒤 stale registry removal에 사용할 expected filenames입니다.
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

		CollectLoadedFixturePackages<UCFMissileGuidePresetData>(PackagesToUnload, DeletedFilenames);
		CollectLoadedFixturePackages<UCFAmmoData>(PackagesToUnload, DeletedFilenames);
		CollectLoadedFixturePackages<UCFDamageData>(PackagesToUnload, DeletedFilenames);

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("DDO-P0-04 disposable package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Disposable physical Content directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DDODamageP04")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = TEXT("DDO-P0-04 physical Content root 삭제에 실패했습니다.");
			return false;
		}
		if (!DeletedFilenames.IsEmpty())
		{
			AssetRegistry.ScanModifiedAssetFiles(DeletedFilenames);
			AssetRegistry.WaitForCompletion();
		}

		// 세 provider disposable Staging roots입니다.
		const TArray<FString> StagingDirectories =
		{
			GetStagingAbsolutePath(MissileFixtureStagingRoot),
			GetStagingAbsolutePath(AmmoFixtureStagingRoot),
			GetStagingAbsolutePath(DamageFixtureStagingRoot)
		};
		for (const FString& StagingDirectory : StagingDirectories)
		{
			if (IFileManager::Get().DirectoryExists(*StagingDirectory)
				&& !IFileManager::Get().DeleteDirectory(*StagingDirectory, false, true))
			{
				OutError = FString::Printf(TEXT("DDO-P0-04 Staging root 삭제에 실패했습니다: %s"), *StagingDirectory);
				return false;
			}
		}
		return VerifyFixtureResidueFree(OutError);
	}

	// Exact3 provider disposable Create sources를 production serializer로 만들고 disk에 기록합니다.
	bool WriteThreeTypeCreateSources(
		const FThreeTypeFixturePaths& Paths,
		FString& OutMissileJson,
		FString& OutAmmoJson,
		FString& OutDamageJson,
		FString& OutError)
	{
		if (!BuildMissileJson(Paths, 3.5f, Paths.MissileTargetObjectPath, OutMissileJson, OutError)
			|| !BuildAmmoJson(Paths, 24, Paths.AmmoTargetObjectPath, OutAmmoJson, OutError)
			|| !BuildDamageJson(Paths, 42.0f, Paths.DamageTargetObjectPath, OutDamageJson, OutError))
		{
			return false;
		}
		if (!WriteStagingFile(Paths.MissileStagingRelativePath, OutMissileJson)
			|| !WriteStagingFile(Paths.AmmoStagingRelativePath, OutAmmoJson)
			|| !WriteStagingFile(Paths.DamageStagingRelativePath, OutDamageJson))
		{
			OutError = TEXT("DDO-P0-04 exact3 Staging source write에 실패했습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Preview 전체에서 특정 issue code가 존재하는지 검사합니다.
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
	FCFDADamageOperationalAdmissionTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_04.OperationalAdmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageThreeTypeDurableTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_04.ThreeTypeDurable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageThreeTypeDuplicateTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_04.ThreeTypeDuplicate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageThreeTypeStaleTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_04.ThreeTypeStale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Actual production backing authority exact3와 Damage admission qualification을 직접 검증합니다.
bool FCFDADamageOperationalAdmissionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDADamageMixedTestsPrivate;

	// Normalize/Discover가 실제 사용하는 production backing authority의 SchemaId projection입니다.
	TArray<FString> SchemaIds;
	// Normalize/Discover가 실제 사용하는 production backing authority의 class-path projection입니다.
	TArray<FString> ClassPaths;
	FCFDAStagingOpsTestControl::GetMixedOperationalAllowedTypeKeys(SchemaIds, ClassPaths);
	TestEqual(TEXT("Production mixed backing SchemaId count is exact4"), SchemaIds.Num(), 4);
	TestEqual(TEXT("Production mixed backing ClassPath count is exact4"), ClassPaths.Num(), 4);
	if (SchemaIds.Num() == 4 && ClassPaths.Num() == 4)
	{
		TestEqual(TEXT("Operational TypeKey[0] SchemaId remains MissileGuidePreset"), SchemaIds[0], FString(TEXT("CarFight.DataAsset.MissileGuidePreset")));
		TestEqual(TEXT("Operational TypeKey[0] ClassPath remains MissileGuidePreset"), ClassPaths[0], FString(TEXT("/Script/CarFight_Re.CFMissileGuidePresetData")));
		TestEqual(TEXT("Operational TypeKey[1] SchemaId remains AmmoData"), SchemaIds[1], FString(TEXT("CarFight.DataAsset.AmmoData")));
		TestEqual(TEXT("Operational TypeKey[1] ClassPath remains AmmoData"), ClassPaths[1], FString(TEXT("/Script/CarFight_Re.CFAmmoData")));
		TestEqual(TEXT("Operational TypeKey[2] SchemaId remains DamageData"), SchemaIds[2], FString(TEXT("CarFight.DataAsset.DamageData")));
		TestEqual(TEXT("Operational TypeKey[2] ClassPath remains DamageData"), ClassPaths[2], FString(TEXT("/Script/CarFight_Re.CFDamageData")));
		TestEqual(TEXT("Operational TypeKey[3] SchemaId is VehicleDefenseData"), SchemaIds[3], FString(TEXT("CarFight.DataAsset.VehicleDefenseData")));
		TestEqual(TEXT("Operational TypeKey[3] ClassPath is VehicleDefenseData"), ClassPaths[3], FString(TEXT("/Script/CarFight_Re.CFVehicleDefenseData")));
	}
	TestFalse(TEXT("Future synthetic provider SchemaId is not auto-admitted"), SchemaIds.Contains(TEXT("CarFight.DataAsset.FutureSynthetic")));

	// Current Damage provider authority입니다.
	const FCFDADamageTypeProvider& DamageProvider = CFDADamageProvider::GetProvider();

	// Production registry를 mutation하지 않고 registry-only future provider shape를 재현하는 test-owned provider entry입니다.
	FCFDATypeProviderEntry SyntheticProvider = DamageProvider;
	SyntheticProvider.Descriptor.TypeKey.SchemaId = TEXT("CarFight.DataAsset.FutureSynthetic");
	SyntheticProvider.Descriptor.TypeKey.DataAssetTypeClassPath = TEXT("/Script/CarFight_Re.CFFutureSyntheticData");
	SyntheticProvider.Descriptor.CanonicalStagingRoot = TEXT("Authoring/DataAssetStaging/FutureSynthetic");
	// Synthetic provider 하나만 가진 test-owned registry projection입니다.
	const TArray<const FCFDATypeProviderEntry*> SyntheticProviderSet = {&SyntheticProvider};
	// Synthetic provider가 owner가 될 수 있음을 검증할 exact allowed TypeKey scope입니다.
	const TArray<FCFDATypeKey> SyntheticAllowedTypeKeys = {SyntheticProvider.Descriptor.TypeKey};
	// Synthetic provider-owned full Staging path입니다.
	const FString SyntheticOwnedPath = TEXT("Authoring/DataAssetStaging/FutureSynthetic/OwnerCapable.json");
	// Test-owned registry owner resolution의 normalized path readback입니다.
	FString SyntheticNormalizedPath;
	// Test-owned registry owner resolution diagnostics입니다.
	FString SyntheticOwnerError;
	// Production registry mutation 없이 synthetic provider가 실제 path owner로 판정 가능한지 확인합니다.
	const FCFDATypeProviderEntry* SyntheticOwner = CFDATypeDispatch::FindProviderForStagingPathInSetForTests(
		SyntheticProviderSet,
		SyntheticOwnedPath,
		SyntheticAllowedTypeKeys,
		SyntheticNormalizedPath,
		&SyntheticOwnerError);
	TestTrue(TEXT("Registry-only synthetic provider is owner-capable in isolated registry evidence"), SyntheticOwner == &SyntheticProvider);
	TestEqual(TEXT("Registry-only synthetic provider path normalizes exactly"), SyntheticNormalizedPath, SyntheticOwnedPath);
	TestFalse(TEXT("Owner-capable registry-only synthetic provider is still absent from production operational backing"), SchemaIds.Contains(SyntheticProvider.Descriptor.TypeKey.SchemaId));

	// Actual production mixed session must reject the same synthetic provider-owned full path before JSON read because production operational admission is explicit exact4 only.
	FCFDAStagingOpsSession SyntheticProductionSession;
	// Unknown/out-of-scope production Preview projection입니다.
	FCFDAStagingOpsPreview SyntheticProductionPreview;
	// Unknown/out-of-scope production Preview diagnostics입니다.
	FString SyntheticProductionError;
	TestFalse(
		TEXT("Production mixed session fail-closes registry-only synthetic full path"),
		SyntheticProductionSession.Preview({SyntheticOwnedPath}, SyntheticProductionPreview, SyntheticProductionError));
	TestFalse(TEXT("Production synthetic rejection returns diagnostic"), SyntheticProductionError.IsEmpty());

	// Production registry exact lookup diagnostic입니다.
	FString LookupError;
	// Exact Damage TypeKey production registry entry입니다.
	const FCFDATypeProviderEntry* RegisteredDamageProvider = CFDATypeDispatch::FindExactProviderEntry(
		DamageProvider.Descriptor.TypeKey.SchemaId,
		DamageProvider.Descriptor.TypeKey.DataAssetTypeClassPath,
		&LookupError);
	TestTrue(TEXT("Damage production registry lookup returns exact provider authority"), RegisteredDamageProvider == &DamageProvider);
	TestEqual(TEXT("Damage provider readiness is ReviewedMutationReady"), DamageProvider.Readiness, ECFDAProviderReadiness::ReviewedMutationReady);
	TestTrue(TEXT("Damage ParseCommonCandidate callback exists"), DamageProvider.Operations.ParseCommonCandidate != nullptr);
	TestTrue(TEXT("Damage ResolveCommonCurrentState callback exists"), DamageProvider.Operations.ResolveCommonCurrentState != nullptr);
	TestTrue(TEXT("Damage ApplyReviewedMutation callback exists"), DamageProvider.Operations.ApplyReviewedMutation != nullptr);
	// Mutation readiness validation diagnostic입니다.
	FString MutationReadyError;
	TestTrue(TEXT("Damage provider mutation-ready validation passes"), CFDATypeDispatch::ValidateProviderMutationReady(DamageProvider, MutationReadyError));
	TestEqual(TEXT("Damage DACE readiness is ContractReady"), DamageProvider.Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestEqual(TEXT("Damage DACE owner is CFDADamageDace"), DamageProvider.Descriptor.DaceContractOwnerName, FName(TEXT("CFDADamageDace")));
	TestEqual(TEXT("Damage DACE history namespace is exact"), DamageProvider.Descriptor.DaceAcceptedHistoryNamespace, FString(TEXT("DACE-DamageData")));
	TestTrue(TEXT("Damage canonical DACE target set is explicitly declared"), DamageProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Damage canonical DACE target set stays exact0"), DamageProvider.Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);
	TestTrue(TEXT("Damage current change declaration stays null"), FCFDADamageDace::GetCurrentChangeDeclaration() == nullptr);
	// Current Damage DACE no-delta migration gate입니다.
	const FCFDAMigrationGateResult MigrationGate = FCFDADamageDace::EvaluateCurrentMigrationGate();
	TestTrue(TEXT("Damage current migration gate validation passes"), MigrationGate.Validation.bPassed);
	TestFalse(TEXT("Damage duplicate accepted snapshot append remains forbidden"), MigrationGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("Damage Current System promotion remains allowed"), MigrationGate.bCurrentSystemPromotionAllowed);

	// Fixture cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("OperationalAdmission pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact test teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Actual Damage provider admission fixture paths입니다.
	const FThreeTypeFixturePaths Paths = BuildFixturePaths(TEXT("DDOP04Adm"));
	// Serialized Damage Create JSON입니다.
	FString DamageJson;
	if (!BuildDamageJson(Paths, 42.0f, Paths.DamageTargetObjectPath, DamageJson, PhaseError)
		|| !WriteStagingFile(Paths.DamageStagingRelativePath, DamageJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("OperationalAdmission Damage source setup failed") : PhaseError);
		return false;
	}
	// Production session에 전달할 Damage-only explicit path입니다.
	const TArray<FString> Selection = {Paths.DamageStagingRelativePath};
	// Actual production mixed operational session입니다.
	FCFDAStagingOpsSession Session;
	// Damage admission Preview result입니다.
	FCFDAStagingOpsPreview Preview;
	if (!TestTrue(TEXT("Production mixed session admits explicit Damage provider path"), Session.Preview(Selection, Preview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestFalse(TEXT("Damage-only explicit operational Preview is not blocked"), Preview.bBlocked);
	TestEqual(TEXT("Damage-only explicit operational Preview has one row"), Preview.Rows.Num(), 1);
	TestEqual(TEXT("Damage-only explicit operational Preview classifies Create1"), Preview.CreateCount, 1);
	return !HasAnyErrors();
}

// Actual Missile+Ammo+Damage exact3가 같은 Reviewed batch에서 deterministic durable lifecycle을 완주하는지 검증합니다.
bool FCFDADamageThreeTypeDurableTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDADamageMixedTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("ThreeTypeDurable pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact P0-04 teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Same textual StableLogicalId를 공유하는 actual-provider exact3 paths입니다.
	const FThreeTypeFixturePaths Paths = BuildFixturePaths(TEXT("DDOP04Dur"));
	// Serialized Missile Create JSON입니다.
	FString MissileJson;
	// Serialized Ammo Create JSON입니다.
	FString AmmoJson;
	// Serialized Damage Create JSON입니다.
	FString DamageJson;
	if (!TestTrue(TEXT("Three-type exact3 Create sources are written"), WriteThreeTypeCreateSources(Paths, MissileJson, AmmoJson, DamageJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Order A는 Damage→Ammo→Missile로 전달합니다.
	const TArray<FString> SelectionA = {Paths.DamageStagingRelativePath, Paths.AmmoStagingRelativePath, Paths.MissileStagingRelativePath};
	// Order B는 Missile→Ammo→Damage로 전달합니다.
	const TArray<FString> SelectionB = {Paths.MissileStagingRelativePath, Paths.AmmoStagingRelativePath, Paths.DamageStagingRelativePath};
	// First three-type session입니다.
	FCFDAStagingOpsSession SessionA;
	// Reversed-input comparison session입니다.
	FCFDAStagingOpsSession SessionB;
	// First exact3 Preview입니다.
	FCFDAStagingOpsPreview PreviewA;
	// Reversed-input exact3 Preview입니다.
	FCFDAStagingOpsPreview PreviewB;
	if (!TestTrue(TEXT("Three-type Preview A succeeds"), SessionA.Preview(SelectionA, PreviewA, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	if (!TestTrue(TEXT("Three-type Preview B succeeds"), SessionB.Preview(SelectionB, PreviewB, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestFalse(TEXT("Three-type Create Preview is not blocked"), PreviewA.bBlocked);
	TestEqual(TEXT("Three-type Preview has exact3 rows"), PreviewA.Rows.Num(), 3);
	TestEqual(TEXT("Three-type Preview has Create exact3"), PreviewA.CreateCount, 3);
	TestEqual(TEXT("Three-type Preview has Conflict0"), PreviewA.ConflictCount, 0);
	TestEqual(TEXT("Three-type Preview has Invalid0"), PreviewA.InvalidCount, 0);
	TestFalse(TEXT("Three-type BatchPlanHash is non-empty"), PreviewA.BatchPlanHash.IsEmpty());
	TestEqual(TEXT("Three-type BatchPlanHash is input-order deterministic"), PreviewA.BatchPlanHash, PreviewB.BatchPlanHash);
	TestFalse(TEXT("Same textual ID across exact3 classes is not DuplicateStableIdentity"), PreviewHasIssueCode(PreviewA, ECFDAStagingIssueCode::DuplicateStableIdentity));

	if (!TestTrue(TEXT("Three-type Review succeeds"), SessionA.Review(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Three-type session owns Reviewed approval"), SessionA.HasReviewedApproval());
	// Fresh exact3 approval입니다.
	const FCFDAStagingReviewedApproval& Approval = SessionA.GetReviewedApproval();
	TestEqual(TEXT("Three-type approval state is Reviewed"), Approval.State, ECFDAStagingApprovalState::Reviewed);
	TestEqual(TEXT("Three-type approval contains exact3 mutation targets"), Approval.IncludedTargets.Num(), 3);
	TestEqual(TEXT("Three-type approval hash matches Preview hash"), Approval.BatchPlanHash, PreviewA.BatchPlanHash);

	// Exact3 Apply terminal report입니다.
	FCFDAStagingApplyReport ApplyReport;
	if (!TestTrue(TEXT("Three-type ApplyReviewed succeeds"), SessionA.ApplyReviewed(ApplyReport, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Three-type batch result is DurableApplied"), ApplyReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("Three-type durable applied count is exact3"), ApplyReport.DurableAppliedCount, 3);
	TestEqual(TEXT("Three-type NotRun count is0"), ApplyReport.NotRunCount, 0);
	TestEqual(TEXT("Three-type target reports are exact3"), ApplyReport.Targets.Num(), 3);
	TestTrue(TEXT("Three-type successful Apply consumes approval"), ApplyReport.bApprovalConsumed);
	TestFalse(TEXT("Three-type session no longer owns reusable Reviewed approval"), SessionA.HasReviewedApproval());
	for (const FCFDAStagingTargetApplyReport& TargetReport : ApplyReport.Targets)
	{
		TestEqual(TEXT("Every three-type target reaches DurableApplied"), TargetReport.Result, ECFDAStagingTargetApplyResult::DurableApplied);
	}

	// Persisted actual Missile fixture readback입니다.
	UCFMissileGuidePresetData* PersistedMissile = LoadObject<UCFMissileGuidePresetData>(nullptr, *Paths.MissileTargetObjectPath);
	// Persisted actual Ammo fixture readback입니다.
	UCFAmmoData* PersistedAmmo = LoadObject<UCFAmmoData>(nullptr, *Paths.AmmoTargetObjectPath);
	// Persisted actual Damage fixture readback입니다.
	UCFDamageData* PersistedDamage = LoadObject<UCFDamageData>(nullptr, *Paths.DamageTargetObjectPath);
	if (TestNotNull(TEXT("Three-type persisted Missile exists"), PersistedMissile))
	{
		TestEqual(TEXT("Persisted Missile class-scoped ID matches"), PersistedMissile->PresetId, FName(*Paths.StableLogicalId));
	}
	if (TestNotNull(TEXT("Three-type persisted Ammo exists"), PersistedAmmo))
	{
		TestEqual(TEXT("Persisted Ammo class-scoped ID matches"), PersistedAmmo->AmmoId, FName(*Paths.StableLogicalId));
	}
	if (TestNotNull(TEXT("Three-type persisted Damage exists"), PersistedDamage))
	{
		// Production extractor readback payload입니다.
		FCFDADamagePayload DamageReadback;
		// Production extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> DamageReadbackIssues;
		TestTrue(TEXT("Persisted Damage production extractor succeeds"), CFDADamageProviderImpl::ExtractPayload(*PersistedDamage, DamageReadback, DamageReadbackIssues));
		TestEqual(TEXT("Persisted Damage class-scoped ID matches"), DamageReadback.DamageId, FName(*Paths.StableLogicalId));
		TestEqual(TEXT("Persisted Damage BaseDamage matches reviewed semantic"), DamageReadback.BaseDamage, 42.0f);
		TestFalse(TEXT("Persisted Damage radial disabled semantic preserved"), DamageReadback.bUseRadialDamage);
		TestEqual(TEXT("Persisted Damage disabled radial radius preserved"), DamageReadback.ExplosionRadius, 125.0f);
		TestEqual(TEXT("Persisted Damage inner radius larger than outer remains valid"), DamageReadback.ExplosionInnerRadius, 300.0f);
		TestEqual(TEXT("Persisted Damage disabled radial damage preserved"), DamageReadback.ExplosionDamage, 75.0f);
	}
	return !HasAnyErrors();
}

// Damage가 참여한 exact3에서 TargetObjectPath duplicate는 TypeKey와 무관한 global blocker인지 검증합니다.
bool FCFDADamageThreeTypeDuplicateTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDADamageMixedTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("ThreeTypeDuplicate pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact P0-04 teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Same textual identity + exact3 class metadata입니다.
	const FThreeTypeFixturePaths Paths = BuildFixturePaths(TEXT("DDOP04Dup"));
	// Missile과 Damage가 동시에 요구할 intentionally shared target path입니다.
	const FString SharedTargetObjectPath = Paths.MissileTargetObjectPath;
	// Shared target을 요구하는 Missile JSON입니다.
	FString MissileJson;
	// Unique Ammo target JSON입니다.
	FString AmmoJson;
	// Shared target을 요구하는 Damage JSON입니다.
	FString DamageJson;
	if (!BuildMissileJson(Paths, 3.25f, SharedTargetObjectPath, MissileJson, PhaseError)
		|| !BuildAmmoJson(Paths, 24, Paths.AmmoTargetObjectPath, AmmoJson, PhaseError)
		|| !BuildDamageJson(Paths, 42.0f, SharedTargetObjectPath, DamageJson, PhaseError)
		|| !WriteStagingFile(Paths.MissileStagingRelativePath, MissileJson)
		|| !WriteStagingFile(Paths.AmmoStagingRelativePath, AmmoJson)
		|| !WriteStagingFile(Paths.DamageStagingRelativePath, DamageJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("Three-type duplicate fixture source setup failed") : PhaseError);
		return false;
	}

	// Actual exact3 selection입니다.
	const TArray<FString> Selection = {Paths.MissileStagingRelativePath, Paths.AmmoStagingRelativePath, Paths.DamageStagingRelativePath};
	// Duplicate negative production session입니다.
	FCFDAStagingOpsSession Session;
	// Global duplicate classification Preview입니다.
	FCFDAStagingOpsPreview Preview;
	if (!TestTrue(TEXT("Three-type duplicate Preview completes with blocker projection"), Session.Preview(Selection, Preview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Damage-involved same TargetObjectPath blocks exact3 Preview"), Preview.bBlocked);
	TestTrue(TEXT("Damage-involved target duplicate emits DuplicateTargetPath"), PreviewHasIssueCode(Preview, ECFDAStagingIssueCode::DuplicateTargetPath));
	TestFalse(TEXT("Cross-class same textual ID still does not emit DuplicateStableIdentity"), PreviewHasIssueCode(Preview, ECFDAStagingIssueCode::DuplicateStableIdentity));
	TestFalse(TEXT("Blocked three-type duplicate cannot be Reviewed"), Session.Review(PhaseError));
	TestFalse(TEXT("Duplicate shared target was not persisted"), FPackageName::DoesPackageExist(Paths.MissilePackageName));
	TestFalse(TEXT("Duplicate Ammo target was not persisted"), FPackageName::DoesPackageExist(Paths.AmmoPackageName));
	TestFalse(TEXT("Duplicate Damage own package path was not persisted"), FPackageName::DoesPackageExist(Paths.DamagePackageName));
	return !HasAnyErrors();
}

// Damage source/current stale가 exact3 global preflight에서 all-before-mutation으로 차단되는지 검증합니다.
bool FCFDADamageThreeTypeStaleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDADamageMixedTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("ThreeTypeStale pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact P0-04 teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Source-stale exact3 paths입니다.
	const FThreeTypeFixturePaths SourceStalePaths = BuildFixturePaths(TEXT("DDOP04Src"));
	// Source-stale original Missile JSON입니다.
	FString SourceMissileJson;
	// Source-stale original Ammo JSON입니다.
	FString SourceAmmoJson;
	// Source-stale original Damage JSON입니다.
	FString SourceDamageJson;
	if (!TestTrue(TEXT("Source-stale three-type sources are written"), WriteThreeTypeCreateSources(SourceStalePaths, SourceMissileJson, SourceAmmoJson, SourceDamageJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Source-stale exact3 selection입니다.
	const TArray<FString> SourceStaleSelection = {SourceStalePaths.MissileStagingRelativePath, SourceStalePaths.AmmoStagingRelativePath, SourceStalePaths.DamageStagingRelativePath};
	// Source-stale production session입니다.
	FCFDAStagingOpsSession SourceStaleSession;
	// Source-stale initial Preview입니다.
	FCFDAStagingOpsPreview SourceStalePreview;
	if (!SourceStaleSession.Preview(SourceStaleSelection, SourceStalePreview, PhaseError)
		|| !SourceStaleSession.Review(PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Review 뒤 semantic drift를 가진 Damage JSON입니다.
	FString ChangedDamageJson;
	if (!BuildDamageJson(SourceStalePaths, 43.0f, SourceStalePaths.DamageTargetObjectPath, ChangedDamageJson, PhaseError)
		|| !WriteStagingFile(SourceStalePaths.DamageStagingRelativePath, ChangedDamageJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("Source-stale Damage rewrite failed") : PhaseError);
		return false;
	}
	// Source-stale global preflight report입니다.
	FCFDAStagingApplyReport SourceStaleReport;
	TestFalse(TEXT("Post-Review Damage source drift blocks exact3 Apply"), SourceStaleSession.ApplyReviewed(SourceStaleReport, PhaseError));
	TestEqual(TEXT("Source-stale exact3 batch blocks before mutation"), SourceStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Source-stale exact3 durable count remains0"), SourceStaleReport.DurableAppliedCount, 0);
	TestTrue(TEXT("Source-stale failed Apply consumes approval"), SourceStaleReport.bApprovalConsumed);
	TestFalse(TEXT("Source-stale session cannot reuse Reviewed approval"), SourceStaleSession.HasReviewedApproval());
	TestFalse(TEXT("Source-stale Missile target remains unpersisted"), FPackageName::DoesPackageExist(SourceStalePaths.MissilePackageName));
	TestFalse(TEXT("Source-stale Ammo target remains unpersisted"), FPackageName::DoesPackageExist(SourceStalePaths.AmmoPackageName));
	TestFalse(TEXT("Source-stale Damage target remains unpersisted"), FPackageName::DoesPackageExist(SourceStalePaths.DamagePackageName));

	if (!TestTrue(TEXT("Source-stale subcase cleanup leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Current-stale exact3 paths입니다.
	const FThreeTypeFixturePaths CurrentStalePaths = BuildFixturePaths(TEXT("DDOP04Cur"));
	// Current-stale Missile JSON입니다.
	FString CurrentMissileJson;
	// Current-stale Ammo JSON입니다.
	FString CurrentAmmoJson;
	// Current-stale Damage JSON입니다.
	FString CurrentDamageJson;
	if (!TestTrue(TEXT("Current-stale three-type sources are written"), WriteThreeTypeCreateSources(CurrentStalePaths, CurrentMissileJson, CurrentAmmoJson, CurrentDamageJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Current-stale exact3 selection입니다.
	const TArray<FString> CurrentStaleSelection = {CurrentStalePaths.MissileStagingRelativePath, CurrentStalePaths.AmmoStagingRelativePath, CurrentStalePaths.DamageStagingRelativePath};
	// Current-stale production session입니다.
	FCFDAStagingOpsSession CurrentStaleSession;
	// Current-stale initial Preview입니다.
	FCFDAStagingOpsPreview CurrentStalePreview;
	if (!CurrentStaleSession.Preview(CurrentStaleSelection, CurrentStalePreview, PhaseError)
		|| !CurrentStaleSession.Review(PhaseError))
	{
		AddError(PhaseError);
		return false;
	}

	// Review 뒤 disk save 없이 current truth에만 등장시킬 Damage package입니다.
	UPackage* LoadedDamagePackage = CreatePackage(*CurrentStalePaths.DamagePackageName);
	// Review 뒤 current resolver가 볼 valid loaded-only Damage target입니다.
	UCFDamageData* LoadedDamage = LoadedDamagePackage != nullptr
		? NewObject<UCFDamageData>(LoadedDamagePackage, *CurrentStalePaths.DamageAssetName, RF_Public | RF_Standalone)
		: nullptr;
	if (!TestNotNull(TEXT("Current-stale loaded-only Damage package exists"), LoadedDamagePackage)
		|| !TestNotNull(TEXT("Current-stale loaded-only Damage target exists"), LoadedDamage))
	{
		return false;
	}
	// Reviewed desired state와 다른 valid current Damage semantic입니다.
	const FCFDADamagePayload DriftedDamagePayload = BuildDamagePayload(CurrentStalePaths.StableLogicalId, 99.0f);
	CFDADamageProviderImpl::MaterializePayload(*LoadedDamage, DriftedDamagePayload);
	LoadedDamagePackage->SetDirtyFlag(false);

	// Current-stale global preflight report입니다.
	FCFDAStagingApplyReport CurrentStaleReport;
	TestFalse(TEXT("Post-Review Damage current truth drift blocks exact3 Apply"), CurrentStaleSession.ApplyReviewed(CurrentStaleReport, PhaseError));
	TestEqual(TEXT("Current-stale exact3 batch blocks before mutation"), CurrentStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Current-stale exact3 durable count remains0"), CurrentStaleReport.DurableAppliedCount, 0);
	TestTrue(TEXT("Current-stale failed Apply consumes approval"), CurrentStaleReport.bApprovalConsumed);
	TestFalse(TEXT("Current-stale session cannot reuse Reviewed approval"), CurrentStaleSession.HasReviewedApproval());
	TestFalse(TEXT("Current-stale Missile target remains disk-unpersisted"), FPackageName::DoesPackageExist(CurrentStalePaths.MissilePackageName));
	TestFalse(TEXT("Current-stale Ammo target remains disk-unpersisted"), FPackageName::DoesPackageExist(CurrentStalePaths.AmmoPackageName));
	TestFalse(TEXT("Current-stale Damage target remains disk-unpersisted"), FPackageName::DoesPackageExist(CurrentStalePaths.DamagePackageName));
	// Loaded-only current truth가 failed Apply로 변형되지 않았는지 읽는 payload입니다.
	FCFDADamagePayload LoadedDamageReadback;
	// Loaded-only readback diagnostics입니다.
	TArray<FCFDAStagingIssue> LoadedDamageReadbackIssues;
	TestTrue(TEXT("Current-stale loaded-only Damage extractor succeeds"), CFDADamageProviderImpl::ExtractPayload(*LoadedDamage, LoadedDamageReadback, LoadedDamageReadbackIssues));
	TestEqual(TEXT("Current-stale loaded-only Damage semantic is preserved"), LoadedDamageReadback.BaseDamage, 99.0f);
	TestFalse(TEXT("Current-stale loaded-only package remains clean"), LoadedDamagePackage->IsDirty());
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
