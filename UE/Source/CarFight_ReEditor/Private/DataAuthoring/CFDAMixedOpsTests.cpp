// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAMixedOpsTests.cpp
// Version: v1.0.2
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-05 actual MissileGuidePreset + AmmoData mixed operational discovery/Review/Apply focused Automation입니다.
// Changelog:
// - v1.0.2: Missile Create fixture에서 production serializer 결과의 empty BaseSemanticFingerprint를 parser 계약의 JSON null로 test-local 정규화합니다. Production serializer/parser는 변경하지 않습니다.
// - v1.0.1: focused mixed Preview 실패 시 TypeKey/Kind/IssueCode/Field/Message를 직접 남기는 diagnostic projection을 추가했습니다.
// - v1.0.0: provider-owned path owner exact1, Product Ammo exact0, actual-provider mixed durable Apply, class-scoped identity/global target duplicate, stale source/current TOCTOU와 disposable residue0을 추가했습니다.
// Migration:
// - 실제 durable Save/Delete는 /Game/Test/CarFight/DAOP05 와 각 provider의 __AutomationP05__ Staging root에만 한정합니다.
// - Product Missile Low/Normal/High, Product canonical Ammo exact0, HeavyFinite/RocketFinite와 DACE accepted history는 mutation 대상으로 사용하지 않습니다.

#include "CFDAAmmoProvider.h"
#include "CFDAMissileProvider.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingOps.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFAmmoData.h"
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

namespace CFDAMixedOpsTestsPrivate
{
	// DAO-P0-05 disposable Content package root입니다.
	static const FString FixturePackageRoot = TEXT("/Game/Test/CarFight/DAOP05");

	// DAO-P0-05 disposable Missile Staging root입니다.
	static const FString MissileFixtureStagingRoot = TEXT("Authoring/DataAssetStaging/MissileGuidePreset/__AutomationP05__");

	// DAO-P0-05 disposable Ammo Staging root입니다.
	static const FString AmmoFixtureStagingRoot = TEXT("Authoring/DataAssetStaging/AmmoData/__AutomationP05__");

	// 두 actual DataAsset class가 같은 textual StableLogicalId를 공유하는 disposable path 묶음입니다.
	struct FMixedFixturePaths
	{
		// Missile PresetId와 AmmoId가 공유할 textual identity입니다.
		FString StableLogicalId;

		// Missile fixture UObject name입니다.
		FString MissileAssetName;

		// Missile fixture package long name입니다.
		FString MissilePackageName;

		// Missile fixture exact object path입니다.
		FString MissileTargetObjectPath;

		// Missile provider-owned exact Staging path입니다.
		FString MissileStagingRelativePath;

		// Ammo fixture UObject name입니다.
		FString AmmoAssetName;

		// Ammo fixture package long name입니다.
		FString AmmoPackageName;

		// Ammo fixture exact object path입니다.
		FString AmmoTargetObjectPath;

		// Ammo provider-owned exact Staging path입니다.
		FString AmmoStagingRelativePath;
	};

	// GUID suffix를 사용해 같은 textual identity + 서로 다른 class target을 가진 disposable exact paths를 만듭니다.
	FMixedFixturePaths BuildFixturePaths(const FString& Prefix)
	{
		// Collision 방지용 short GUID suffix입니다.
		const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);
		// 두 actual provider가 공유할 class-scoped identity입니다.
		const FString StableLogicalId = FString::Printf(TEXT("%s_%s"), *Prefix, *Suffix);
		// 반환할 exact fixture path 묶음입니다.
		FMixedFixturePaths Paths;
		Paths.StableLogicalId = StableLogicalId;
		Paths.MissileAssetName = FString::Printf(TEXT("DA_Missile_%s"), *Suffix);
		Paths.MissilePackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.MissileAssetName);
		Paths.MissileTargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.MissilePackageName, *Paths.MissileAssetName);
		Paths.MissileStagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *MissileFixtureStagingRoot, *StableLogicalId);
		Paths.AmmoAssetName = FString::Printf(TEXT("DA_Ammo_%s"), *Suffix);
		Paths.AmmoPackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.AmmoAssetName);
		Paths.AmmoTargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.AmmoPackageName, *Paths.AmmoAssetName);
		Paths.AmmoStagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *AmmoFixtureStagingRoot, *StableLogicalId);
		return Paths;
	}

	// DAO-P0-05 Missile disposable fixture용 valid typed payload를 만듭니다.
	FCFDAMissilePresetPayload BuildMissilePayload(const FString& StableLogicalId, const float NavigationConstant)
	{
		// Exact valid Missile whole-record payload입니다.
		FCFDAMissilePresetPayload Payload;
		Payload.PresetId = FName(*StableLogicalId);
		Payload.PresetDisplayName.Text = TEXT("DAO P0-05 Mixed Missile");
		Payload.PresetDescription.Text = TEXT("DAO P0-05 disposable mixed integration fixture");
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

	// DAO-P0-05 Ammo disposable fixture용 valid exact8 typed payload를 만듭니다.
	FCFDAAmmoPayload BuildAmmoPayload(const FString& StableLogicalId, const int32 MaximumCount)
	{
		// Exact valid Ammo whole-record payload입니다.
		FCFDAAmmoPayload Payload;
		Payload.AmmoId = FName(*StableLogicalId);
		Payload.AmmoDisplayName.Text = TEXT("DAO P0-05 Mixed Ammo");
		Payload.AmmoFamilyId = FName(TEXT("DAOAutomation"));
		Payload.UnitMassKg = 2.5f;
		Payload.AmmoTags = {FName(TEXT("Mixed")), FName(TEXT("Automation"))};
		Payload.AmmoIcon.Reset();
		Payload.MaximumLoadableAmmoCount = MaximumCount;
		Payload.bCanBeResupplied = true;
		return Payload;
	}

	// Production Missile serializer로 disposable Create whole-record JSON을 만듭니다.
	bool BuildMissileJson(
		const FMixedFixturePaths& Paths,
		const float NavigationConstant,
		const FString& TargetObjectPath,
		FString& OutJson,
		FString& OutError)
	{
		// Exact desired Missile payload입니다.
		const FCFDAMissilePresetPayload Payload = BuildMissilePayload(Paths.StableLogicalId, NavigationConstant);
		if (!CFDAMissileProviderImpl::SerializeProductStagingJson(
			Payload,
			TargetObjectPath,
			FString(),
			OutJson,
			OutError))
		{
			return false;
		}

		// Production serializer 결과를 test-local Create envelope로 다시 읽을 JSON reader입니다.
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(OutJson);
		// Production serializer가 만든 exact whole-record root object입니다.
		TSharedPtr<FJsonObject> RootObject;
		if (!FJsonSerializer::Deserialize(JsonReader, RootObject) || !RootObject.IsValid())
		{
			OutJson.Reset();
			OutError = TEXT("Missile production serializer 결과를 DAO-P0-05 Create fixture로 재parse하지 못했습니다.");
			return false;
		}

		// Create fixture는 current target이 없으므로 BaseSemanticFingerprint physical value를 parser 계약의 JSON null로 명시합니다.
		RootObject->SetField(TEXT("BaseSemanticFingerprint"), MakeShared<FJsonValueNull>());
		OutJson.Reset();
		// Test-local null normalization 뒤 deterministic whole-record JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJson);
		if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), JsonWriter))
		{
			OutJson.Reset();
			OutError = TEXT("DAO-P0-05 Missile Create fixture JSON null normalization 직렬화에 실패했습니다.");
			return false;
		}
		JsonWriter->Close();
		OutJson += LINE_TERMINATOR;
		OutError.Reset();
		return true;
	}

	// Production Ammo serializer로 disposable Create whole-record JSON을 만듭니다.
	bool BuildAmmoJson(
		const FMixedFixturePaths& Paths,
		const int32 MaximumCount,
		const FString& TargetObjectPath,
		FString& OutJson,
		FString& OutError)
	{
		// Exact desired Ammo payload입니다.
		const FCFDAAmmoPayload Payload = BuildAmmoPayload(Paths.StableLogicalId, MaximumCount);
		return CFDAAmmoProviderImpl::SerializeStagingJson(
			Payload,
			TargetObjectPath,
			FString(),
			OutJson,
			OutError);
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
		// Exact provider-owned parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(AbsolutePath);
		IFileManager::Get().MakeDirectory(*ParentDirectory, true);
		return FFileHelper::SaveStringToFile(
			JsonText,
			*AbsolutePath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	// UObject path가 DAO-P0-05 disposable package root 아래인지 확인합니다.
	bool IsFixtureObjectPath(const FString& ObjectPath)
	{
		return ObjectPath.StartsWith(FixturePackageRoot + TEXT("/"), ESearchCase::CaseSensitive);
	}

	// Registry/loaded UObject/physical Content/provider Staging roots가 모두 residue0인지 검증합니다.
	bool VerifyFixtureResidueFree(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Disposable package root 아래 persisted registry-visible assets입니다.
		TArray<FAssetData> RegistryAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), RegistryAssets, true, true);
		if (!RegistryAssets.IsEmpty())
		{
			OutError = FString::Printf(TEXT("DAO-P0-05 AssetRegistry residue가 %d개 남았습니다."), RegistryAssets.Num());
			return false;
		}

		for (TObjectIterator<UCFMissileGuidePresetData> MissileIterator; MissileIterator; ++MissileIterator)
		{
			// Current process의 loaded Missile fixture 후보입니다.
			const UCFMissileGuidePresetData* LoadedMissile = *MissileIterator;
			if (LoadedMissile != nullptr
				&& !LoadedMissile->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
				&& IsFixtureObjectPath(FSoftObjectPath(LoadedMissile).ToString()))
			{
				OutError = TEXT("DAO-P0-05 loaded Missile UObject residue가 남았습니다.");
				return false;
			}
		}
		for (TObjectIterator<UCFAmmoData> AmmoIterator; AmmoIterator; ++AmmoIterator)
		{
			// Current process의 loaded Ammo fixture 후보입니다.
			const UCFAmmoData* LoadedAmmo = *AmmoIterator;
			if (LoadedAmmo != nullptr
				&& !LoadedAmmo->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
				&& IsFixtureObjectPath(FSoftObjectPath(LoadedAmmo).ToString()))
			{
				OutError = TEXT("DAO-P0-05 loaded Ammo UObject residue가 남았습니다.");
				return false;
			}
		}

		// Disposable physical Content directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DAOP05")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = TEXT("DAO-P0-05 physical Content root residue가 남았습니다.");
			return false;
		}
		// Disposable Missile Staging directory입니다.
		const FString MissileStagingDirectory = GetStagingAbsolutePath(MissileFixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*MissileStagingDirectory))
		{
			OutError = TEXT("DAO-P0-05 Missile Staging residue가 남았습니다.");
			return false;
		}
		// Disposable Ammo Staging directory입니다.
		const FString AmmoStagingDirectory = GetStagingAbsolutePath(AmmoFixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*AmmoStagingDirectory))
		{
			OutError = TEXT("DAO-P0-05 Ammo Staging residue가 남았습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// DAO-P0-05 disposable Content와 두 provider Staging roots만 unload→GC→delete→registry refresh 순서로 정리합니다.
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
			const FString PackageFilename = FPackageName::LongPackageNameToFilename(
				FixtureAssetData.PackageName.ToString(),
				FPackageName::GetAssetPackageExtension());
			DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(PackageFilename));
		}

		for (TObjectIterator<UCFMissileGuidePresetData> MissileIterator; MissileIterator; ++MissileIterator)
		{
			// Registry에 없어도 current resolver가 볼 수 있는 loaded Missile fixture입니다.
			UCFMissileGuidePresetData* LoadedMissile = *MissileIterator;
			if (LoadedMissile == nullptr
				|| LoadedMissile->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
				|| !IsFixtureObjectPath(FSoftObjectPath(LoadedMissile).ToString()))
			{
				continue;
			}
			// Loaded Missile owning package입니다.
			UPackage* LoadedPackage = LoadedMissile->GetOutermost();
			if (LoadedPackage != nullptr)
			{
				LoadedPackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(LoadedPackage);
				// Loaded-only package도 registry disk refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(LoadedPackage->GetName(), FPackageName::GetAssetPackageExtension());
				DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedMissile->ClearFlags(RF_Standalone);
		}
		for (TObjectIterator<UCFAmmoData> AmmoIterator; AmmoIterator; ++AmmoIterator)
		{
			// Registry에 없어도 current resolver가 볼 수 있는 loaded Ammo fixture입니다.
			UCFAmmoData* LoadedAmmo = *AmmoIterator;
			if (LoadedAmmo == nullptr
				|| LoadedAmmo->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
				|| !IsFixtureObjectPath(FSoftObjectPath(LoadedAmmo).ToString()))
			{
				continue;
			}
			// Loaded Ammo owning package입니다.
			UPackage* LoadedPackage = LoadedAmmo->GetOutermost();
			if (LoadedPackage != nullptr)
			{
				LoadedPackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(LoadedPackage);
				// Loaded-only package도 registry disk refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(LoadedPackage->GetName(), FPackageName::GetAssetPackageExtension());
				DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedAmmo->ClearFlags(RF_Standalone);
		}

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("DAO-P0-05 disposable package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Disposable physical Content directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DAOP05")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = TEXT("DAO-P0-05 physical Content root 삭제에 실패했습니다.");
			return false;
		}
		if (!DeletedFilenames.IsEmpty())
		{
			AssetRegistry.ScanModifiedAssetFiles(DeletedFilenames);
			AssetRegistry.WaitForCompletion();
		}

		// Disposable Missile Staging directory입니다.
		const FString MissileStagingDirectory = GetStagingAbsolutePath(MissileFixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*MissileStagingDirectory)
			&& !IFileManager::Get().DeleteDirectory(*MissileStagingDirectory, false, true))
		{
			OutError = TEXT("DAO-P0-05 Missile Staging root 삭제에 실패했습니다.");
			return false;
		}
		// Disposable Ammo Staging directory입니다.
		const FString AmmoStagingDirectory = GetStagingAbsolutePath(AmmoFixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*AmmoStagingDirectory)
			&& !IFileManager::Get().DeleteDirectory(*AmmoStagingDirectory, false, true))
		{
			OutError = TEXT("DAO-P0-05 Ammo Staging root 삭제에 실패했습니다.");
			return false;
		}
		return VerifyFixtureResidueFree(OutError);
	}

	// Mixed fixture exact2 Staging source를 production serializer로 만들고 disk에 기록합니다.
	bool WriteMixedCreateSources(
		const FMixedFixturePaths& Paths,
		FString& OutMissileJson,
		FString& OutAmmoJson,
		FString& OutError)
	{
		if (!BuildMissileJson(Paths, 3.25f, Paths.MissileTargetObjectPath, OutMissileJson, OutError)
			|| !BuildAmmoJson(Paths, 24, Paths.AmmoTargetObjectPath, OutAmmoJson, OutError))
		{
			return false;
		}
		if (!WriteStagingFile(Paths.MissileStagingRelativePath, OutMissileJson)
			|| !WriteStagingFile(Paths.AmmoStagingRelativePath, OutAmmoJson))
		{
			OutError = TEXT("DAO-P0-05 mixed exact2 Staging source write에 실패했습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Preview rows 안에 특정 issue code가 존재하는지 확인합니다.
	bool PreviewHasIssueCode(const FCFDAStagingOpsPreview& Preview, const ECFDAStagingIssueCode IssueCode)
	{
		for (const FCFDAStagingPreviewRow& Row : Preview.Rows)
		{
			if (FCFDAStagingService::HasIssueCode(Row.Issues, IssueCode))
			{
				return true;
			}
		}
		return false;
	}

	// Focused failure RCA용으로 mixed Preview의 exact row/type/issue projection을 Automation log에 남깁니다.
	void AddPreviewDiagnostics(FAutomationTestBase& Test, const FString& Label, const FCFDAStagingOpsPreview& Preview)
	{
		Test.AddInfo(FString::Printf(
			TEXT("%s summary: rows=%d create=%d update=%d nochange=%d conflict=%d invalid=%d blocked=%s hash=%s"),
			*Label,
			Preview.Rows.Num(),
			Preview.CreateCount,
			Preview.UpdateCount,
			Preview.NoChangeCount,
			Preview.ConflictCount,
			Preview.InvalidCount,
			Preview.bBlocked ? TEXT("true") : TEXT("false"),
			*Preview.BatchPlanHash));
		for (int32 RowIndex = 0; RowIndex < Preview.Rows.Num(); ++RowIndex)
		{
			// Current diagnostic row입니다.
			const FCFDAStagingPreviewRow& Row = Preview.Rows[RowIndex];
			Test.AddInfo(FString::Printf(
				TEXT("%s row[%d]: kind=%d schema=%s class=%s id=%s target=%s source=%s issues=%d"),
				*Label,
				RowIndex,
				static_cast<int32>(Row.Kind),
				*Row.Record.SchemaId,
				*Row.Record.DataAssetTypeClassPath,
				*Row.Record.StableLogicalId.ToString(),
				*Row.Record.TargetObjectPath,
				*Row.Record.StagingRelativePath,
				Row.Issues.Num()));
			for (int32 IssueIndex = 0; IssueIndex < Row.Issues.Num(); ++IssueIndex)
			{
				// Current row의 exact blocker/warning issue입니다.
				const FCFDAStagingIssue& Issue = Row.Issues[IssueIndex];
				Test.AddInfo(FString::Printf(
					TEXT("%s row[%d] issue[%d]: code=%d blocking=%s field=%s message=%s"),
					*Label,
					RowIndex,
					IssueIndex,
					static_cast<int32>(Issue.Code),
					Issue.bBlocking ? TEXT("true") : TEXT("false"),
					*Issue.FieldPath,
					*Issue.Message));
			}
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAMixedPathOwnerTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_05.PathOwnerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAMixedDurableTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDurable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAMixedDuplicateTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedDuplicate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAMixedStaleTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_05.MixedStale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Production exact2 provider root ownership, allowed-scope fail-closed, overlapping-root reject와 Ammo Product exact0을 검증합니다.
bool FCFDAMixedPathOwnerTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAMixedOpsTestsPrivate;

	// Actual production Missile provider entry입니다.
	const FCFDATypeProviderEntry& MissileProvider = CFDATypeDispatch::GetMissilePresetProviderEntry();
	// Actual production Ammo provider entry입니다.
	const FCFDATypeProviderEntry& AmmoProvider = CFDAAmmoProvider::GetProvider();
	// DAO-P0-05 explicit allowed TypeKey exact2입니다.
	const TArray<FCFDATypeKey> AllowedTypeKeys = {MissileProvider.Descriptor.TypeKey, AmmoProvider.Descriptor.TypeKey};
	// Path owner resolver normalized output입니다.
	FString NormalizedPath;
	// Path owner resolver diagnostic입니다.
	FString ResolverError;

	// Actual Missile provider-owned test path입니다.
	const FString MissilePath = MissileFixtureStagingRoot + TEXT("/Owner.json");
	// Actual Missile path owner입니다.
	const FCFDATypeProviderEntry* MissileOwner = CFDATypeDispatch::FindProviderForStagingPath(MissilePath, AllowedTypeKeys, NormalizedPath, &ResolverError);
	TestTrue(TEXT("Missile path owner resolves to actual Missile provider"), MissileOwner == &MissileProvider);
	TestEqual(TEXT("Missile path normalization is exact"), NormalizedPath, MissilePath);

	// Actual Ammo provider-owned test path입니다.
	const FString AmmoPath = AmmoFixtureStagingRoot + TEXT("/Owner.json");
	// Actual Ammo path owner입니다.
	const FCFDATypeProviderEntry* AmmoOwner = CFDATypeDispatch::FindProviderForStagingPath(AmmoPath, AllowedTypeKeys, NormalizedPath, &ResolverError);
	TestTrue(TEXT("Ammo path owner resolves to actual Ammo provider"), AmmoOwner == &AmmoProvider);
	TestEqual(TEXT("Ammo path normalization is exact"), NormalizedPath, AmmoPath);

	// Allowed TypeKey를 Missile 하나로 제한한 explicit scope입니다.
	const TArray<FCFDATypeKey> MissileOnlyScope = {MissileProvider.Descriptor.TypeKey};
	TestNull(TEXT("Ammo owner outside explicit allowed TypeKey scope is rejected"), CFDATypeDispatch::FindProviderForStagingPath(AmmoPath, MissileOnlyScope, NormalizedPath, &ResolverError));
	TestTrue(TEXT("Scope mismatch diagnostic is explicit"), ResolverError.Contains(TEXT("scope"), ESearchCase::IgnoreCase));

	// Registered provider가 소유하지 않는 parent-relative path입니다.
	const FString UnknownPath = TEXT("Authoring/DataAssetStaging/Unknown/Owner.json");
	TestNull(TEXT("Unknown parent-root child path is rejected"), CFDATypeDispatch::FindProviderForStagingPath(UnknownPath, AllowedTypeKeys, NormalizedPath, &ResolverError));

	// Overlapping root negative fixture를 production registry mutation 없이 구성한 parent provider copy입니다.
	FCFDATypeProviderEntry ParentProvider = MissileProvider;
	ParentProvider.Descriptor.TypeKey.SchemaId = TEXT("Test.DAO.P05.Parent");
	ParentProvider.Descriptor.TypeKey.DataAssetTypeClassPath = TEXT("/Script/CarFight_Re.CFDAOTestParent");
	ParentProvider.Descriptor.CanonicalStagingRoot = TEXT("Authoring/DataAssetStaging");
	ParentProvider.Descriptor.DaceContractOwnerName = FName(TEXT("CFDAOTestParentDace"));
	ParentProvider.Descriptor.DaceAcceptedHistoryNamespace = TEXT("DACE-DAOTestParent");
	ParentProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared = true;
	ParentProvider.Descriptor.DaceCanonicalStagingRelativePaths.Reset();
	// Test-owned provider set은 parent root + actual Ammo child root로 의도적인 ambiguity를 만듭니다.
	const TArray<const FCFDATypeProviderEntry*> OverlappingProviders = {&ParentProvider, &AmmoProvider};
	// Overlap test의 explicit allowed exact2입니다.
	const TArray<FCFDATypeKey> OverlapAllowed = {ParentProvider.Descriptor.TypeKey, AmmoProvider.Descriptor.TypeKey};
	TestNull(TEXT("Overlapping provider roots are rejected before payload read"), CFDATypeDispatch::FindProviderForStagingPathInSetForTests(OverlappingProviders, AmmoPath, OverlapAllowed, NormalizedPath, &ResolverError));
	TestTrue(TEXT("Overlapping root diagnostic is explicit"), ResolverError.Contains(TEXT("둘 이상"), ESearchCase::CaseSensitive));

	TestTrue(TEXT("Ammo canonical target set is explicitly declared"), AmmoProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Ammo Product canonical target set remains exact0"), AmmoProvider.Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);
	return !HasAnyErrors();
}

// Actual Missile+Ammo providers가 같은 textual identity를 class-scoped로 허용하고 한 mixed approval에서 exact2 durable Apply되는지 검증합니다.
bool FCFDAMixedDurableTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAMixedOpsTestsPrivate;

	// Pre-existing P0-05 residue cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("Mixed durable pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// Exact P0-05 teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Same textual StableLogicalId를 공유하는 actual-provider exact2 paths입니다.
	const FMixedFixturePaths Paths = BuildFixturePaths(TEXT("DAOP05Dur"));
	// Serialized Missile Create JSON입니다.
	FString MissileJson;
	// Serialized Ammo Create JSON입니다.
	FString AmmoJson;
	if (!TestTrue(TEXT("Mixed exact2 Create sources are written"), WriteMixedCreateSources(Paths, MissileJson, AmmoJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Order A는 Ammo→Missile로 전달합니다.
	const TArray<FString> SelectionA = {Paths.AmmoStagingRelativePath, Paths.MissileStagingRelativePath};
	// Order B는 Missile→Ammo로 전달합니다.
	const TArray<FString> SelectionB = {Paths.MissileStagingRelativePath, Paths.AmmoStagingRelativePath};
	// Order-independent hash 비교용 첫 session입니다.
	FCFDAStagingOpsSession SessionA;
	// Order-independent hash 비교용 두 번째 session입니다.
	FCFDAStagingOpsSession SessionB;
	// First mixed Preview입니다.
	FCFDAStagingOpsPreview PreviewA;
	// Reversed-input mixed Preview입니다.
	FCFDAStagingOpsPreview PreviewB;
	if (!TestTrue(TEXT("Actual-provider mixed Preview A succeeds"), SessionA.Preview(SelectionA, PreviewA, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	if (!TestTrue(TEXT("Actual-provider mixed Preview B succeeds"), SessionB.Preview(SelectionB, PreviewB, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	AddPreviewDiagnostics(*this, TEXT("MixedDurable.PreviewA"), PreviewA);
	TestFalse(TEXT("Mixed Create Preview is not blocked"), PreviewA.bBlocked);
	TestEqual(TEXT("Mixed Create Preview has exact2 Create rows"), PreviewA.CreateCount, 2);
	TestEqual(TEXT("Mixed Create Preview has exact2 rows"), PreviewA.Rows.Num(), 2);
	TestEqual(TEXT("Mixed BatchPlanHash is input-order deterministic"), PreviewA.BatchPlanHash, PreviewB.BatchPlanHash);
	TestFalse(TEXT("Same textual ID across exact classes is not DuplicateStableIdentity"), PreviewHasIssueCode(PreviewA, ECFDAStagingIssueCode::DuplicateStableIdentity));

	if (!TestTrue(TEXT("Actual-provider mixed Review succeeds"), SessionA.Review(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Mixed session owns Reviewed approval"), SessionA.HasReviewedApproval());
	// Fresh mixed exact2 approval입니다.
	const FCFDAStagingReviewedApproval& Approval = SessionA.GetReviewedApproval();
	TestEqual(TEXT("Mixed approval state is Reviewed"), Approval.State, ECFDAStagingApprovalState::Reviewed);
	TestEqual(TEXT("Mixed approval contains exact2 mutation targets"), Approval.IncludedTargets.Num(), 2);

	// Exact mixed Apply terminal report입니다.
	FCFDAStagingApplyReport ApplyReport;
	if (!TestTrue(TEXT("Actual-provider mixed ApplyReviewed succeeds"), SessionA.ApplyReviewed(ApplyReport, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Mixed batch result is DurableApplied"), ApplyReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("Mixed durable applied count is exact2"), ApplyReport.DurableAppliedCount, 2);
	TestEqual(TEXT("Mixed target reports are exact2"), ApplyReport.Targets.Num(), 2);
	for (const FCFDAStagingTargetApplyReport& TargetReport : ApplyReport.Targets)
	{
		TestEqual(TEXT("Every mixed target reaches DurableApplied"), TargetReport.Result, ECFDAStagingTargetApplyResult::DurableApplied);
	}

	// Persisted actual Missile fixture readback입니다.
	UCFMissileGuidePresetData* PersistedMissile = LoadObject<UCFMissileGuidePresetData>(nullptr, *Paths.MissileTargetObjectPath);
	// Persisted actual Ammo fixture readback입니다.
	UCFAmmoData* PersistedAmmo = LoadObject<UCFAmmoData>(nullptr, *Paths.AmmoTargetObjectPath);
	if (TestNotNull(TEXT("Mixed persisted Missile exists"), PersistedMissile))
	{
		TestEqual(TEXT("Persisted Missile class-scoped ID matches"), PersistedMissile->PresetId, FName(*Paths.StableLogicalId));
	}
	if (TestNotNull(TEXT("Mixed persisted Ammo exists"), PersistedAmmo))
	{
		TestEqual(TEXT("Persisted Ammo class-scoped ID matches"), PersistedAmmo->AmmoId, FName(*Paths.StableLogicalId));
	}
	return !HasAnyErrors();
}

// Actual provider exact2가 같은 TargetObjectPath를 요구하면 global duplicate로 Review 전에 fail-closed하는지 검증합니다.
bool FCFDAMixedDuplicateTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAMixedOpsTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("Mixed duplicate pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Exact P0-05 teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Same textual identity but two different class paths를 가진 fixture metadata입니다.
	const FMixedFixturePaths Paths = BuildFixturePaths(TEXT("DAOP05Dup"));
	// 두 TypeKey가 동시에 요구할 intentionally shared target path입니다.
	const FString SharedTargetObjectPath = Paths.MissileTargetObjectPath;
	// Shared target을 요구하는 Missile JSON입니다.
	FString MissileJson;
	// Shared target을 요구하는 Ammo JSON입니다.
	FString AmmoJson;
	if (!BuildMissileJson(Paths, 3.25f, SharedTargetObjectPath, MissileJson, PhaseError)
		|| !BuildAmmoJson(Paths, 24, SharedTargetObjectPath, AmmoJson, PhaseError)
		|| !WriteStagingFile(Paths.MissileStagingRelativePath, MissileJson)
		|| !WriteStagingFile(Paths.AmmoStagingRelativePath, AmmoJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("Mixed duplicate fixture source setup failed") : PhaseError);
		return false;
	}

	// Actual mixed exact2 selection입니다.
	const TArray<FString> Selection = {Paths.MissileStagingRelativePath, Paths.AmmoStagingRelativePath};
	// Duplicate negative session입니다.
	FCFDAStagingOpsSession Session;
	// Global duplicate classification Preview입니다.
	FCFDAStagingOpsPreview Preview;
	if (!TestTrue(TEXT("Mixed duplicate Preview completes with blocker projection"), Session.Preview(Selection, Preview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Cross-TypeKey same TargetObjectPath blocks mixed Preview"), Preview.bBlocked);
	TestTrue(TEXT("Cross-TypeKey target duplicate emits DuplicateTargetPath"), PreviewHasIssueCode(Preview, ECFDAStagingIssueCode::DuplicateTargetPath));
	TestFalse(TEXT("Cross-class same textual ID still does not emit DuplicateStableIdentity"), PreviewHasIssueCode(Preview, ECFDAStagingIssueCode::DuplicateStableIdentity));
	TestFalse(TEXT("Blocked mixed duplicate cannot be Reviewed"), Session.Review(PhaseError));
	TestFalse(TEXT("Duplicate fixture target was not persisted"), FPackageName::DoesPackageExist(Paths.MissilePackageName));
	return !HasAnyErrors();
}

// Mixed approval 뒤 source drift와 global preflight 뒤 current truth drift가 모두 mutation0 fail-closed하는지 검증합니다.
bool FCFDAMixedStaleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAMixedOpsTestsPrivate;

	// Current phase/cleanup diagnostic입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("Mixed stale pre-clean leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// Exact P0-05 teardown residue diagnostic입니다.
		FString TeardownError;
		if (!CleanupFixtureRoots(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Source-stale negative case exact2 paths입니다.
	const FMixedFixturePaths SourceStalePaths = BuildFixturePaths(TEXT("DAOP05Src"));
	// Original Missile JSON입니다.
	FString OriginalMissileJson;
	// Original Ammo JSON입니다.
	FString OriginalAmmoJson;
	if (!TestTrue(TEXT("Source-stale mixed sources are written"), WriteMixedCreateSources(SourceStalePaths, OriginalMissileJson, OriginalAmmoJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Source-stale exact2 selection입니다.
	const TArray<FString> SourceStaleSelection = {SourceStalePaths.MissileStagingRelativePath, SourceStalePaths.AmmoStagingRelativePath};
	// Source-stale mixed session입니다.
	FCFDAStagingOpsSession SourceStaleSession;
	// Source-stale initial Preview입니다.
	FCFDAStagingOpsPreview SourceStalePreview;
	if (!SourceStaleSession.Preview(SourceStaleSelection, SourceStalePreview, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	AddPreviewDiagnostics(*this, TEXT("MixedStale.SourcePreview"), SourceStalePreview);
	if (!SourceStaleSession.Review(PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Review 뒤 semantic을 바꾼 Missile JSON입니다.
	FString ChangedMissileJson;
	if (!BuildMissileJson(SourceStalePaths, 4.25f, SourceStalePaths.MissileTargetObjectPath, ChangedMissileJson, PhaseError)
		|| !WriteStagingFile(SourceStalePaths.MissileStagingRelativePath, ChangedMissileJson))
	{
		AddError(PhaseError.IsEmpty() ? TEXT("Source-stale Missile rewrite failed") : PhaseError);
		return false;
	}
	// Source-stale apply report입니다.
	FCFDAStagingApplyReport SourceStaleReport;
	TestFalse(TEXT("Post-Review Missile source drift blocks mixed Apply"), SourceStaleSession.ApplyReviewed(SourceStaleReport, PhaseError));
	TestEqual(TEXT("Source-stale mixed batch blocks before mutation"), SourceStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Source-stale mixed durable count remains0"), SourceStaleReport.DurableAppliedCount, 0);
	TestFalse(TEXT("Source-stale Missile target remains unpersisted"), FPackageName::DoesPackageExist(SourceStalePaths.MissilePackageName));
	TestFalse(TEXT("Source-stale Ammo target remains unpersisted"), FPackageName::DoesPackageExist(SourceStalePaths.AmmoPackageName));

	if (!TestTrue(TEXT("Source-stale subcase cleanup leaves residue0"), CleanupFixtureRoots(PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Current-stale negative case exact2 paths입니다.
	const FMixedFixturePaths CurrentStalePaths = BuildFixturePaths(TEXT("DAOP05Cur"));
	// Current-stale Missile JSON입니다.
	FString CurrentMissileJson;
	// Current-stale Ammo JSON입니다.
	FString CurrentAmmoJson;
	if (!TestTrue(TEXT("Current-stale mixed sources are written"), WriteMixedCreateSources(CurrentStalePaths, CurrentMissileJson, CurrentAmmoJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Current-stale exact2 selection입니다.
	const TArray<FString> CurrentStaleSelection = {CurrentStalePaths.MissileStagingRelativePath, CurrentStalePaths.AmmoStagingRelativePath};
	// Current-stale mixed session입니다.
	FCFDAStagingOpsSession CurrentStaleSession;
	// Current-stale initial Preview입니다.
	FCFDAStagingOpsPreview CurrentStalePreview;
	if (!CurrentStaleSession.Preview(CurrentStaleSelection, CurrentStalePreview, PhaseError)
		|| !CurrentStaleSession.Review(PhaseError))
	{
		AddError(PhaseError);
		return false;
	}

	// Review 뒤 disk save 없이 current truth에만 등장시킬 Ammo package입니다.
	UPackage* LoadedAmmoPackage = CreatePackage(*CurrentStalePaths.AmmoPackageName);
	// Review 뒤 current resolver가 볼 valid loaded-only Ammo target입니다.
	UCFAmmoData* LoadedAmmo = LoadedAmmoPackage != nullptr
		? NewObject<UCFAmmoData>(LoadedAmmoPackage, *CurrentStalePaths.AmmoAssetName, RF_Public | RF_Standalone)
		: nullptr;
	if (!TestNotNull(TEXT("Current-stale loaded-only Ammo package exists"), LoadedAmmoPackage)
		|| !TestNotNull(TEXT("Current-stale loaded-only Ammo target exists"), LoadedAmmo))
	{
		return false;
	}
	// Current-stale target에 production materializer로 valid exact8 semantic을 부여합니다.
	const FCFDAAmmoPayload LoadedPayload = BuildAmmoPayload(CurrentStalePaths.StableLogicalId, 12);
	CFDAAmmoProviderImpl::MaterializePayload(*LoadedAmmo, LoadedPayload);
	LoadedAmmoPackage->SetDirtyFlag(false);

	// Current-stale global preflight apply report입니다.
	FCFDAStagingApplyReport CurrentStaleReport;
	TestFalse(TEXT("Post-Review Ammo current truth drift blocks mixed Apply"), CurrentStaleSession.ApplyReviewed(CurrentStaleReport, PhaseError));
	TestEqual(TEXT("Current-stale mixed batch blocks before mutation"), CurrentStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Current-stale mixed durable count remains0"), CurrentStaleReport.DurableAppliedCount, 0);
	TestFalse(TEXT("Current-stale Missile target remains unpersisted"), FPackageName::DoesPackageExist(CurrentStalePaths.MissilePackageName));
	TestFalse(TEXT("Current-stale Ammo target remains disk-unpersisted"), FPackageName::DoesPackageExist(CurrentStalePaths.AmmoPackageName));
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
