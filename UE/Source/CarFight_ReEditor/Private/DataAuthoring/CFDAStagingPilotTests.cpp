// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingPilotTests.cpp
// Version: v1.2.3
// Date: 2026-09-08
// Description: CF-FQ-049 DAS-P0-04 MissileGuidePreset Product mutation0 Preview와 test-owned durable materializer fixture Automation입니다.
// Changelog:
// - v1.2.3: AssetRegistry residue 검사를 on-disk-only로 고정해 persisted registry truth와 loaded-but-unregistered UObject truth를 분리하고 loaded-only negative regression의 진단 authority를 정확히 검증.
// - v1.2.2: UObject AssetDeleted ordering 의존을 제거하고 package unload→GC→disk delete→ScanModifiedAssetFiles disk refresh 순서로 AssetRegistry를 동기화해 loaded handle과 watcher resurrection을 동시에 차단.
// - v1.2.1: AssetDeleted 전에 exact test-owned physical Content root를 먼저 삭제해 directory watcher의 deleted-asset resurrection을 차단하고, unload/GC 뒤 defensive second delete + 4중 residue 검증을 유지.
// - v1.2.0: fixture cleanup을 AssetRegistry-visible + loaded-but-unregistered UObject union으로 강화하고, teardown 완료 시 AssetRegistry/loaded UObject/Content/Staging 4중 residue 0을 검증. loaded-only residue detection 회귀를 durable fixture에 추가.
// - v1.1.0: DAS-P0-04 fixture teardown을 exact test-owned root 단위로 강화해 AssetRegistry delete notification → package unload → GC → Content/Staging root recursive delete → residue 0 검증을 수행하고 cleanup failure 자체를 Automation failure로 승격.
// - v1.0.1: first-run persisted FText stable-key finding에 맞춰 Product/extractor failure diagnostics를 강화하고, test-owned cleanup은 disk file 제거를 Asset Registry delete notification보다 먼저 수행해 watcher resurrection warning을 방지.
// - v1.0.0: Product Low/Normal/High NoChange/Update Preview, test-owned Create/Update durable readback, drift/dirty guards, save uncertainty와 PartialApplied exact fixture를 추가.
// Migration:
// - Product Low/Normal/High는 read-only Preview만 수행하며 BuildReviewedApproval/ApplyReviewedBatch를 호출하지 않습니다.
// - 실제 Save/Delete는 /Game/Test/CarFight/DAStagingP04 test-owned package와 __AutomationP04__ Staging 파일에만 한정하고 test 종료 시 정리합니다.
// - cleanup PASS는 AssetRegistry-visible, resolver-visible loaded UObject, physical Content, Staging 네 authority 모두 residue 0일 때만 성립합니다.

#include "DataAuthoring/CFDAStagingApply.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
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
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAStagingPilotTestsPrivate
{
	// Product Pilot 한 건의 immutable identity/path projection입니다.
	struct FProductPresetCase
	{
		// 기대 stable PresetId입니다.
		const TCHAR* StableLogicalId = TEXT("");

		// persisted Product DataAsset exact object path입니다.
		const TCHAR* TargetObjectPath = TEXT("");
	};

	// test-owned package/Staging cleanup에 필요한 exact 경로 묶음입니다.
	struct FFixturePaths
	{
		// unique stable identity입니다.
		FString StableLogicalId;

		// package 안의 asset object name입니다.
		FString AssetName;

		// test-owned /Game package long name입니다.
		FString PackageName;

		// exact object path입니다.
		FString TargetObjectPath;

		// main_game-relative canonical Staging JSON path입니다.
		FString StagingRelativePath;
	};

	// enum authored value를 strict JSON source token으로 변환합니다.
	template <typename TEnum>
	FString GetEnumToken(const TEnum Value)
	{
		// enum source token authority입니다.
		const UEnum* Enum = StaticEnum<TEnum>();
		return Enum != nullptr
			? Enum->GetNameStringByValue(static_cast<int64>(Value))
			: FString();
	}

	// Literal FText JSON object를 만듭니다.
	TSharedRef<FJsonObject> BuildLiteralTextObject(const FString& Text)
	{
		// strict Literal FText JSON object입니다.
		TSharedRef<FJsonObject> TextObject = MakeShared<FJsonObject>();
		TextObject->SetStringField(TEXT("Kind"), TEXT("Literal"));
		TextObject->SetStringField(TEXT("Text"), Text);
		return TextObject;
	}

	// exact typed MissileGuideConfig를 strict 26-field JSON object로 직렬화합니다.
	TSharedRef<FJsonObject> BuildGuideConfigObject(const FCFMissileGuideConfig& Config)
	{
		// exact writable GuideConfig JSON object입니다.
		TSharedRef<FJsonObject> ConfigObject = MakeShared<FJsonObject>();
		ConfigObject->SetBoolField(TEXT("bUseGuidance"), Config.bUseGuidance);
		ConfigObject->SetStringField(TEXT("GuideMode"), GetEnumToken(Config.GuideMode));
		ConfigObject->SetStringField(TEXT("LostTargetPolicy"), GetEnumToken(Config.LostTargetPolicy));
		ConfigObject->SetNumberField(TEXT("NavigationConstant"), Config.NavigationConstant);
		ConfigObject->SetNumberField(TEXT("MaximumTurnRateDegPerSec"), Config.MaximumTurnRateDegPerSec);
		ConfigObject->SetNumberField(TEXT("MaximumLateralAccelerationCmPerSecSq"), Config.MaximumLateralAccelerationCmPerSecSq);
		ConfigObject->SetNumberField(TEXT("GuidanceResponseTimeSeconds"), Config.GuidanceResponseTimeSeconds);
		ConfigObject->SetNumberField(TEXT("MinimumGuidanceSpeedCmPerSec"), Config.MinimumGuidanceSpeedCmPerSec);
		ConfigObject->SetNumberField(TEXT("SeekerFieldOfViewDeg"), Config.SeekerFieldOfViewDeg);
		ConfigObject->SetNumberField(TEXT("LockBreakAngleDeg"), Config.LockBreakAngleDeg);
		ConfigObject->SetNumberField(TEXT("TargetLostGraceTimeSeconds"), Config.TargetLostGraceTimeSeconds);
		ConfigObject->SetStringField(TEXT("SeekerModel"), GetEnumToken(Config.SeekerModel));
		ConfigObject->SetStringField(TEXT("TargetObservationMode"), GetEnumToken(Config.TargetObservationMode));
		ConfigObject->SetStringField(TEXT("GuidanceLaw"), GetEnumToken(Config.GuidanceLaw));
		ConfigObject->SetStringField(TEXT("GuidanceActivationMode"), GetEnumToken(Config.GuidanceActivationMode));
		ConfigObject->SetNumberField(TEXT("GuidanceActivationDelaySeconds"), Config.GuidanceActivationDelaySeconds);
		ConfigObject->SetNumberField(TEXT("GuidanceActivationDistanceCm"), Config.GuidanceActivationDistanceCm);
		ConfigObject->SetNumberField(TEXT("LeadTimeSeconds"), Config.LeadTimeSeconds);
		ConfigObject->SetNumberField(TEXT("MaxLeadDistanceCm"), Config.MaxLeadDistanceCm);
		ConfigObject->SetStringField(TEXT("ReacquisitionMode"), GetEnumToken(Config.ReacquisitionMode));
		ConfigObject->SetNumberField(TEXT("TargetObservationIntervalSeconds"), Config.TargetObservationIntervalSeconds);
		ConfigObject->SetNumberField(TEXT("TargetVelocityEstimateResponseTimeSeconds"), Config.TargetVelocityEstimateResponseTimeSeconds);
		ConfigObject->SetNumberField(TEXT("AcquisitionConeHalfAngleDeg"), Config.AcquisitionConeHalfAngleDeg);
		ConfigObject->SetNumberField(TEXT("TrackingConeHalfAngleDeg"), Config.TrackingConeHalfAngleDeg);
		ConfigObject->SetNumberField(TEXT("ReacquisitionConeHalfAngleDeg"), Config.ReacquisitionConeHalfAngleDeg);
		ConfigObject->SetNumberField(TEXT("ReacquisitionTimeSeconds"), Config.ReacquisitionTimeSeconds);
		return ConfigObject;
	}

	// typed Pilot payload를 strict whole-record JSON으로 직렬화합니다.
	FString BuildStagingJson(
		const FCFDAMissilePresetPayload& Payload,
		const FString& TargetObjectPath,
		const FString* BaseSemanticFingerprint)
	{
		// exact writable Payload JSON object입니다.
		TSharedRef<FJsonObject> PayloadObject = MakeShared<FJsonObject>();
		PayloadObject->SetStringField(TEXT("PresetId"), Payload.PresetId.ToString());
		PayloadObject->SetObjectField(TEXT("PresetDisplayName"), BuildLiteralTextObject(Payload.PresetDisplayName.Text));
		PayloadObject->SetObjectField(TEXT("PresetDescription"), BuildLiteralTextObject(Payload.PresetDescription.Text));
		PayloadObject->SetObjectField(TEXT("MissileGuideConfig"), BuildGuideConfigObject(Payload.MissileGuideConfig));

		// strict 8-field Staging root object입니다.
		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("SchemaId"), FCFDAStagingService::GetMissilePresetSchemaId());
		RootObject->SetNumberField(TEXT("SchemaRevision"), FCFDAStagingService::GetMissilePresetSchemaRevision());
		RootObject->SetNumberField(TEXT("AdapterContractRevision"), FCFDAStagingService::GetMissilePresetAdapterRevision());
		RootObject->SetStringField(TEXT("DataAssetTypeClassPath"), FCFDAStagingService::GetMissilePresetClassPath());
		RootObject->SetStringField(TEXT("StableLogicalId"), Payload.PresetId.ToString());
		RootObject->SetStringField(TEXT("TargetObjectPath"), TargetObjectPath);
		if (BaseSemanticFingerprint != nullptr)
		{
			RootObject->SetStringField(TEXT("BaseSemanticFingerprint"), *BaseSemanticFingerprint);
		}
		else
		{
			RootObject->SetField(TEXT("BaseSemanticFingerprint"), MakeShared<FJsonValueNull>());
		}
		RootObject->SetObjectField(TEXT("Payload"), PayloadObject);

		// canonical parser input text입니다.
		FString JsonText;
		// compact JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonText);
		FJsonSerializer::Serialize(RootObject, JsonWriter);
		JsonWriter->Close();
		return JsonText;
	}

	// DAS-P0-04 fixture에 사용할 valid typed payload를 만듭니다.
	FCFDAMissilePresetPayload BuildFixturePayload(const FString& StableLogicalId, const FString& Description, const float NavigationConstant)
	{
		// 반환할 whole-record payload입니다.
		FCFDAMissilePresetPayload Payload;
		Payload.PresetId = FName(*StableLogicalId);
		Payload.PresetDisplayName.Text = TEXT("DAS-P0-04 Automation Fixture");
		Payload.PresetDescription.Text = Description;
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

	// short GUID suffix로 collision 없는 test-owned package/Staging 경로를 만듭니다.
	FFixturePaths BuildFixturePaths(const FString& Prefix)
	{
		// collision 방지용 8자리 GUID suffix입니다.
		const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);
		// 반환할 exact fixture paths입니다.
		FFixturePaths Paths;
		Paths.StableLogicalId = FString::Printf(TEXT("%s_%s"), *Prefix, *Suffix);
		Paths.AssetName = FString::Printf(TEXT("DA_%s_%s"), *Prefix, *Suffix);
		Paths.PackageName = FString::Printf(TEXT("/Game/Test/CarFight/DAStagingP04/%s"), *Paths.AssetName);
		Paths.TargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.PackageName, *Paths.AssetName);
		Paths.StagingRelativePath = FString::Printf(TEXT("Authoring/DataAssetStaging/__AutomationP04__/%s.json"), *Paths.StableLogicalId);
		return Paths;
	}

	// CarFight main_game root를 ProjectDir 부모로 resolve합니다.
	FString GetMainGameRoot()
	{
		// `<main_game>/UE/` absolute project root입니다.
		const FString UnrealProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		// `<main_game>/` absolute root입니다.
		FString MainGameRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(UnrealProjectRoot, TEXT("..")));
		FPaths::NormalizeDirectoryName(MainGameRoot);
		return MainGameRoot;
	}

	// main_game-relative Staging path를 exact absolute path로 변환합니다.
	FString GetStagingAbsolutePath(const FString& StagingRelativePath)
	{
		// exact staging absolute filename입니다.
		FString StagingAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(GetMainGameRoot(), StagingRelativePath));
		FPaths::NormalizeFilename(StagingAbsolutePath);
		return StagingAbsolutePath;
	}

	// test-owned Staging JSON을 UTF-8 without BOM으로 저장합니다.
	bool WriteStagingFile(const FString& StagingRelativePath, const FString& JsonText)
	{
		// exact staging absolute filename입니다.
		const FString StagingAbsolutePath = GetStagingAbsolutePath(StagingRelativePath);
		// staging parent directory입니다.
		const FString StagingDirectory = FPaths::GetPath(StagingAbsolutePath);
		IFileManager::Get().MakeDirectory(*StagingDirectory, true);
		return FFileHelper::SaveStringToFile(
			JsonText,
			*StagingAbsolutePath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	// test-owned Staging JSON을 삭제합니다.
	void DeleteStagingFile(const FString& StagingRelativePath)
	{
		// exact staging absolute filename입니다.
		const FString StagingAbsolutePath = GetStagingAbsolutePath(StagingRelativePath);
		IFileManager::Get().Delete(*StagingAbsolutePath, false, true, true);
	}

	// parse + current resolver + mutation0 Preview를 하나의 exact helper로 수행합니다.
	bool BuildPreviewFromJson(
		const FString& JsonText,
		const FString& StagingRelativePath,
		FCFDAStagingPreviewRow& OutPreviewRow,
		FString& OutError)
	{
		// strict typed parse result입니다.
		const FCFDAStagingParseResult ParseResult = FCFDAStagingService::ParseMissilePresetJson(JsonText, StagingRelativePath);
		if (!ParseResult.bValid)
		{
			OutError = ParseResult.Issues.IsEmpty()
				? TEXT("Staging parse가 실패했습니다.")
				: ParseResult.Issues[0].Message;
			return false;
		}

		// exact current target/identity truth입니다.
		FCFDAStagingCurrentState CurrentState;
		// current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> ResolveIssues;
		if (!FCFDAStagingService::ResolveMissilePresetCurrentState(ParseResult.Record, CurrentState, ResolveIssues))
		{
			OutError = ResolveIssues.IsEmpty()
				? TEXT("Current resolver가 실패했습니다.")
				: ResolveIssues[0].Message;
			return false;
		}
		OutPreviewRow = FCFDAStagingService::BuildPreview(ParseResult.Record, CurrentState);
		OutError.Reset();
		return true;
	}

	// Product/test persisted asset에서 exact semantic fingerprint를 read-only로 계산합니다.
	bool ReadPersistedFingerprint(
		const FString& TargetObjectPath,
		FString& OutFingerprint,
		bool& bOutPackageDirty,
		FString& OutError)
	{
		// exact persisted Pilot asset입니다.
		UCFMissileGuidePresetData* Asset = LoadObject<UCFMissileGuidePresetData>(nullptr, *TargetObjectPath);
		if (Asset == nullptr)
		{
			OutError = FString::Printf(TEXT("Persisted Pilot asset을 로드하지 못했습니다: %s"), *TargetObjectPath);
			return false;
		}
		// exact owning package입니다.
		UPackage* Package = Asset->GetOutermost();
		bOutPackageDirty = Package != nullptr && Package->IsDirty();
		// lossless typed payload입니다.
		FCFDAMissilePresetPayload Payload;
		// extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!FCFDAStagingService::ExtractMissilePresetPayload(*Asset, Payload, ExtractIssues))
		{
			OutError = ExtractIssues.IsEmpty()
				? TEXT("Persisted Pilot typed payload 추출에 실패했습니다.")
				: ExtractIssues[0].Message;
			return false;
		}
		return FCFDAStagingService::BuildSemanticFingerprint(Payload, OutFingerprint, OutError);
	}

	// exact package를 disk state에서 다시 reload합니다.
	bool ReloadFixturePackage(const FString& TargetObjectPath, FString& OutError)
	{
		// exact current object입니다.
		UObject* TargetObject = FSoftObjectPath(TargetObjectPath).ResolveObject();
		if (TargetObject == nullptr)
		{
			TargetObject = LoadObject<UObject>(nullptr, *TargetObjectPath);
		}
		// exact owning package입니다.
		UPackage* Package = TargetObject != nullptr ? TargetObject->GetOutermost() : nullptr;
		if (Package == nullptr)
		{
			OutError = TEXT("Reload할 fixture package를 찾지 못했습니다.");
			return false;
		}
		// exact one-package reload list입니다.
		TArray<UPackage*> PackagesToReload;
		PackagesToReload.Add(Package);
		// reload failure text입니다.
		FText ReloadError;
		if (!UPackageTools::ReloadPackages(PackagesToReload, ReloadError, EReloadPackagesInteractionMode::AssumePositive))
		{
			OutError = ReloadError.ToString();
			return false;
		}
		OutError.Reset();
		return true;
	}

	// object path가 DAS-P0-04 exact test-owned package root 아래인지 확인합니다.
	bool IsFixtureObjectPath(const FString& ObjectPath)
	{
		return ObjectPath.StartsWith(TEXT("/Game/Test/CarFight/DAStagingP04/"), ESearchCase::CaseSensitive);
	}

	// AssetRegistry/loaded UObject/Content/Staging 네 authority 모두에서 DAS-P0-04 residue가 0인지 검증합니다.
	bool VerifyFixtureRootResidueFree(FString& OutError)
	{
		// test-owned AssetRegistry package root입니다.
		const FName FixturePackageRoot(TEXT("/Game/Test/CarFight/DAStagingP04"));
		// test-owned physical Content directory입니다.
		const FString FixtureContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DAStagingP04")));
		// test-owned physical Staging directory입니다.
		const FString FixtureStagingDirectory = GetStagingAbsolutePath(TEXT("Authoring/DataAssetStaging/__AutomationP04__"));
		// current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// exact root 아래 registry-visible test assets입니다.
		TArray<FAssetData> RemainingRegistryAssets;
		AssetRegistry.GetAssetsByPath(FixturePackageRoot, RemainingRegistryAssets, true, true);
		if (!RemainingRegistryAssets.IsEmpty())
		{
			OutError = FString::Printf(TEXT("DAS-P0-04 fixture AssetRegistry residue가 %d개 남았습니다."), RemainingRegistryAssets.Num());
			return false;
		}

		for (TObjectIterator<UCFMissileGuidePresetData> LoadedPresetIterator; LoadedPresetIterator; ++LoadedPresetIterator)
		{
			// current process에서 resolver가 볼 수 있는 loaded MissileGuidePreset 후보입니다.
			const UCFMissileGuidePresetData* LoadedPreset = *LoadedPresetIterator;
			if (LoadedPreset == nullptr || LoadedPreset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}

			// loaded object의 exact current object path입니다.
			const FString LoadedObjectPath = FSoftObjectPath(LoadedPreset).ToString();
			if (IsFixtureObjectPath(LoadedObjectPath))
			{
				OutError = FString::Printf(TEXT("DAS-P0-04 fixture loaded UObject residue가 남았습니다: %s"), *LoadedObjectPath);
				return false;
			}
		}

		// Content root의 remaining physical files입니다.
		TArray<FString> RemainingContentFiles;
		IFileManager::Get().FindFilesRecursive(RemainingContentFiles, *FixtureContentDirectory, TEXT("*"), true, false, false);
		if (IFileManager::Get().DirectoryExists(*FixtureContentDirectory) || !RemainingContentFiles.IsEmpty())
		{
			OutError = TEXT("DAS-P0-04 fixture Content root residue가 남았습니다.");
			return false;
		}

		// Staging root의 remaining physical files입니다.
		TArray<FString> RemainingStagingFiles;
		IFileManager::Get().FindFilesRecursive(RemainingStagingFiles, *FixtureStagingDirectory, TEXT("*"), true, false, false);
		if (IFileManager::Get().DirectoryExists(*FixtureStagingDirectory) || !RemainingStagingFiles.IsEmpty())
		{
			OutError = TEXT("DAS-P0-04 fixture Staging root residue가 남았습니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// DAS-P0-04 test-owned Content/Staging root와 resolver-visible loaded/registered residue를 unload-first 순서로 제거합니다.
	bool CleanupFixtureRoot(FString& OutError)
	{
		// test-owned AssetRegistry package root입니다.
		const FName FixturePackageRoot(TEXT("/Game/Test/CarFight/DAStagingP04"));
		// test-owned physical Content directory입니다.
		const FString FixtureContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DAStagingP04")));
		// test-owned physical Staging directory입니다.
		const FString FixtureStagingDirectory = GetStagingAbsolutePath(TEXT("Authoring/DataAssetStaging/__AutomationP04__"));
		// current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// exact root 아래 registry-visible test assets입니다.
		TArray<FAssetData> FixtureAssets;
		AssetRegistry.GetAssetsByPath(FixturePackageRoot, FixtureAssets, true, true);
		// unload할 unique fixture packages입니다.
		TArray<UPackage*> PackagesToUnload;

		// disk delete 뒤 AssetRegistry disk refresh에 사용할 exact persisted fixture filename 목록입니다.
		TArray<FString> RegistryFixtureFilenames;
		for (const FAssetData& FixtureAssetData : FixtureAssets)
		{
			// loaded 또는 persisted fixture UObject입니다.
			UObject* FixtureAsset = FixtureAssetData.GetSoftObjectPath().ResolveObject();
			if (FixtureAsset == nullptr)
			{
				FixtureAsset = FixtureAssetData.GetAsset();
			}
			if (FixtureAsset == nullptr)
			{
				continue;
			}

			// fixture owning package입니다.
			UPackage* FixturePackage = FixtureAsset->GetOutermost();
			if (FixturePackage != nullptr)
			{
				FixturePackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(FixturePackage);
			}
			// registry-visible fixture package의 expected physical uasset filename입니다.
			const FString FixturePackageFilename = FPackageName::LongPackageNameToFilename(
				FixtureAssetData.PackageName.ToString(),
				FPackageName::GetAssetPackageExtension());
			RegistryFixtureFilenames.AddUnique(FPaths::ConvertRelativePathToFull(FixturePackageFilename));
		}

		for (TObjectIterator<UCFMissileGuidePresetData> LoadedPresetIterator; LoadedPresetIterator; ++LoadedPresetIterator)
		{
			// AssetRegistry에 없더라도 resolver가 current truth로 볼 수 있는 loaded fixture입니다.
			UCFMissileGuidePresetData* LoadedPreset = *LoadedPresetIterator;
			if (LoadedPreset == nullptr || LoadedPreset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}

			// loaded fixture의 exact object path입니다.
			const FString LoadedObjectPath = FSoftObjectPath(LoadedPreset).ToString();
			if (!IsFixtureObjectPath(LoadedObjectPath))
			{
				continue;
			}

			// loaded-but-unregistered fixture를 포함한 exact owning package입니다.
			UPackage* LoadedFixturePackage = LoadedPreset->GetOutermost();
			if (LoadedFixturePackage != nullptr)
			{
				LoadedFixturePackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(LoadedFixturePackage);
			}
			// loaded-only RF_Standalone object도 unload 대상 package와 함께 회수될 수 있도록 standalone 유지권을 제거합니다.
			LoadedPreset->ClearFlags(RF_Standalone);
		}

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("DAS-P0-04 fixture package unload에 실패했습니다.");
			return false;
		}

		CollectGarbage(RF_NoFlags);

		// package handle과 loaded UObject를 회수한 뒤 exact test-owned physical Content root를 재귀 삭제합니다.
		if (IFileManager::Get().DirectoryExists(*FixtureContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*FixtureContentDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("DAS-P0-04 fixture Content root 삭제에 실패했습니다: %s"), *FixtureContentDirectory);
			return false;
		}

		if (!RegistryFixtureFilenames.IsEmpty())
		{
			// deleted filenames의 current disk truth를 강제 재스캔해 stale AssetRegistry entry를 제거합니다.
			AssetRegistry.ScanModifiedAssetFiles(RegistryFixtureFilenames);
			AssetRegistry.WaitForCompletion();
		}

		// canonical Staging fixture root도 함께 재귀 삭제합니다.
		if (IFileManager::Get().DirectoryExists(*FixtureStagingDirectory)
			&& !IFileManager::Get().DeleteDirectory(*FixtureStagingDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("DAS-P0-04 fixture Staging root 삭제에 실패했습니다: %s"), *FixtureStagingDirectory);
			return false;
		}

		return VerifyFixtureRootResidueFree(OutError);
	}

	// one-row Preview를 Reviewed approval로 동결합니다.
	bool BuildSingleApproval(
		const FCFDAStagingPreviewRow& PreviewRow,
		FCFDAStagingReviewedApproval& OutApproval,
		FString& OutError)
	{
		// exact single mutation Preview set입니다.
		const TArray<FCFDAStagingPreviewRow> PreviewRows = {PreviewRow};
		return FCFDAStagingApplyService::BuildReviewedApproval(PreviewRows, OutApproval, OutError);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingProductPreviewTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_04.ProductPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingDurableFixtureTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_04.DurableFixture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingConflictGuardsTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_04.ConflictGuards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingSaveUncertaintyTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_04.SaveUncertainty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingPartialAppliedTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_04.PartialApplied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Product Low/Normal/High를 whole-record JSON으로 표현해 NoChange/Update Preview만 검증합니다.
bool FCFDAStagingProductPreviewTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingPilotTestsPrivate;

	// Product Pilot exact 3종입니다.
	const FProductPresetCase ProductCases[] = {
		{TEXT("MissileFeel_Low"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low")},
		{TEXT("MissileFeel_Normal"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Normal.DA_MissileFeel_Normal")},
		{TEXT("MissileFeel_High"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_High.DA_MissileFeel_High")}
	};

	for (const FProductPresetCase& ProductCase : ProductCases)
	{
		// exact persisted Product asset입니다.
		UCFMissileGuidePresetData* ProductAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, ProductCase.TargetObjectPath);
		if (!TestNotNull(TEXT("Product MissileGuidePreset exists"), ProductAsset))
		{
			continue;
		}
		// Product owning package입니다.
		UPackage* ProductPackage = ProductAsset->GetOutermost();
		TestNotNull(TEXT("Product package exists"), ProductPackage);
		TestFalse(TEXT("Product package is clean before mutation0 Preview"), ProductPackage != nullptr && ProductPackage->IsDirty());
		TestEqual(TEXT("Product StableIdentity matches expected"), ProductAsset->PresetId, FName(ProductCase.StableLogicalId));

		// current Product whole-record typed payload입니다.
		FCFDAMissilePresetPayload CurrentPayload;
		// current Product extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		// exact Product whole-record extraction 결과입니다.
		const bool bExtracted = FCFDAStagingService::ExtractMissilePresetPayload(*ProductAsset, CurrentPayload, ExtractIssues);
		TestTrue(TEXT("Product typed whole-record extraction succeeds"), bExtracted);
		if (!bExtracted)
		{
			for (const FCFDAStagingIssue& Issue : ExtractIssues)
			{
				AddError(FString::Printf(TEXT("%s: %s"), ProductCase.TargetObjectPath, *Issue.Message));
			}
			continue;
		}
		// Product current semantic baseline입니다.
		FString CurrentFingerprint;
		// Product fingerprint error입니다.
		FString FingerprintError;
		if (!TestTrue(TEXT("Product current semantic fingerprint succeeds"), FCFDAStagingService::BuildSemanticFingerprint(CurrentPayload, CurrentFingerprint, FingerprintError)))
		{
			AddError(FingerprintError);
			continue;
		}
		// canonical Product Staging projection path입니다. 파일 저장은 수행하지 않습니다.
		const FString ProductStagingPath = FString::Printf(TEXT("Authoring/DataAssetStaging/MissileGuidePreset/%s.json"), ProductCase.StableLogicalId);
		// exact current state를 desired로 표현한 whole-record JSON입니다.
		const FString NoChangeJson = BuildStagingJson(CurrentPayload, ProductCase.TargetObjectPath, &CurrentFingerprint);
		// mutation0 NoChange Preview입니다.
		FCFDAStagingPreviewRow NoChangePreview;
		// mutation0 Preview error입니다.
		FString PreviewError;
		if (TestTrue(TEXT("Product NoChange Preview builds"), BuildPreviewFromJson(NoChangeJson, ProductStagingPath, NoChangePreview, PreviewError)))
		{
			TestEqual(TEXT("Product exact current JSON classifies NoChange"), NoChangePreview.Kind, ECFDAStagingPreviewKind::NoChange);
		}
		else
		{
			AddError(PreviewError);
		}

		// Product를 실제 수정하지 않고 intended Update JSON을 만들기 위한 payload copy입니다.
		FCFDAMissilePresetPayload IntendedUpdatePayload = CurrentPayload;
		IntendedUpdatePayload.MissileGuideConfig.NavigationConstant = CurrentPayload.MissileGuideConfig.NavigationConstant <= 9.5f
			? CurrentPayload.MissileGuideConfig.NavigationConstant + 0.25f
			: CurrentPayload.MissileGuideConfig.NavigationConstant - 0.25f;
		// intended Update whole-record JSON입니다.
		const FString UpdateJson = BuildStagingJson(IntendedUpdatePayload, ProductCase.TargetObjectPath, &CurrentFingerprint);
		// mutation0 Update Preview입니다.
		FCFDAStagingPreviewRow UpdatePreview;
		if (TestTrue(TEXT("Product Update Preview builds"), BuildPreviewFromJson(UpdateJson, ProductStagingPath, UpdatePreview, PreviewError)))
		{
			TestEqual(TEXT("Product intended changed JSON classifies Update"), UpdatePreview.Kind, ECFDAStagingPreviewKind::Update);
		}
		else
		{
			AddError(PreviewError);
		}

		// mutation0 검증 뒤 Product persisted/in-memory semantic fingerprint입니다.
		FString FinalFingerprint;
		// mutation0 뒤 package dirty 여부입니다.
		bool bFinalPackageDirty = true;
		if (TestTrue(TEXT("Product fingerprint readback after mutation0 Preview succeeds"), ReadPersistedFingerprint(ProductCase.TargetObjectPath, FinalFingerprint, bFinalPackageDirty, FingerprintError)))
		{
			TestEqual(TEXT("Product semantic state unchanged after Preview"), FinalFingerprint, CurrentFingerprint);
			TestFalse(TEXT("Product package remains clean after Preview"), bFinalPackageDirty);
		}
	}
	return !HasAnyErrors();
}

// test-owned fixture에서 Create→Save→reload→readback과 Update→Save→reload→readback을 실제 검증합니다.
bool FCFDAStagingDurableFixtureTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingPilotTestsPrivate;

	// write fixture 시작 전 이전 run residue를 제거할 오류입니다.
	FString InitialCleanupError;
	if (!TestTrue(TEXT("Durable fixture pre-clean leaves residue 0"), CleanupFixtureRoot(InitialCleanupError)))
	{
		AddError(InitialCleanupError);
		return false;
	}
	// resolver가 볼 수 있지만 AssetRegistry에는 등록하지 않은 loaded-only fixture package입니다.
	UPackage* LoadedOnlyFixturePackage = CreatePackage(TEXT("/Game/Test/CarFight/DAStagingP04/DA_DASP04LoadedOnly"));
	// loaded-only residue detector를 검증할 test-owned in-memory DataAsset입니다. 저장하거나 AssetRegistry에 등록하지 않습니다.
	UCFMissileGuidePresetData* LoadedOnlyFixtureAsset = LoadedOnlyFixturePackage != nullptr
		? NewObject<UCFMissileGuidePresetData>(LoadedOnlyFixturePackage, TEXT("DA_DASP04LoadedOnly"), RF_Public | RF_Standalone)
		: nullptr;
	if (!TestNotNull(TEXT("Loaded-only fixture package exists"), LoadedOnlyFixturePackage)
		|| !TestNotNull(TEXT("Loaded-only fixture asset exists"), LoadedOnlyFixtureAsset))
	{
		return false;
	}
	LoadedOnlyFixtureAsset->PresetId = FName(TEXT("DASP04LoadedOnly"));
	LoadedOnlyFixturePackage->SetDirtyFlag(false);
	// loaded-only residue detection diagnostic입니다.
	FString LoadedOnlyResidueError;
	TestFalse(TEXT("Loaded-only resolver residue is detected"), VerifyFixtureRootResidueFree(LoadedOnlyResidueError));
	TestTrue(TEXT("Loaded-only residue diagnostic is explicit"), LoadedOnlyResidueError.Contains(TEXT("loaded UObject residue"), ESearchCase::CaseSensitive));
	LoadedOnlyFixtureAsset = nullptr;
	LoadedOnlyFixturePackage = nullptr;
	// loaded-but-unregistered residue도 cleanup authority가 제거할 수 있어야 합니다.
	FString LoadedOnlyCleanupError;
	if (!TestTrue(TEXT("Loaded-only resolver residue cleanup succeeds"), CleanupFixtureRoot(LoadedOnlyCleanupError)))
	{
		AddError(LoadedOnlyCleanupError);
		return false;
	}

	// unique durable fixture paths입니다.
	const FFixturePaths Paths = BuildFixturePaths(TEXT("DASP04Dur"));
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// durable fixture teardown residue error입니다.
		FString TeardownError;
		if (!CleanupFixtureRoot(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// initial Create desired payload입니다.
	const FCFDAMissilePresetPayload CreatePayload = BuildFixturePayload(Paths.StableLogicalId, TEXT("Create durable fixture"), 3.25f);
	// initial Create Staging JSON입니다.
	const FString CreateJson = BuildStagingJson(CreatePayload, Paths.TargetObjectPath, nullptr);
	if (!TestTrue(TEXT("Create fixture Staging file write succeeds"), WriteStagingFile(Paths.StagingRelativePath, CreateJson)))
	{
		return false;
	}
	// Create mutation Preview입니다.
	FCFDAStagingPreviewRow CreatePreview;
	// current phase error입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("Create fixture Preview builds"), BuildPreviewFromJson(CreateJson, Paths.StagingRelativePath, CreatePreview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Create fixture Preview is Create"), CreatePreview.Kind, ECFDAStagingPreviewKind::Create);
	// Create reviewed approval입니다.
	FCFDAStagingReviewedApproval CreateApproval;
	if (!TestTrue(TEXT("Create fixture approval builds"), BuildSingleApproval(CreatePreview, CreateApproval, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Create durable Apply report입니다.
	FCFDAStagingApplyReport CreateReport;
	TestTrue(TEXT("Create fixture Apply reaches durable success"), FCFDAStagingApplyService::ApplyReviewedBatch(CreateApproval, CreateReport));
	TestEqual(TEXT("Create fixture batch result DurableApplied"), CreateReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("Create fixture durable count 1"), CreateReport.DurableAppliedCount, 1);

	// Create desired semantic fingerprint입니다.
	const FString CreateFingerprint = CreatePreview.Record.StagingSemanticFingerprint;
	// persisted Create readback fingerprint입니다.
	FString PersistedCreateFingerprint;
	// persisted package dirty state입니다.
	bool bPersistedDirty = true;
	if (TestTrue(TEXT("Create persisted semantic readback succeeds"), ReadPersistedFingerprint(Paths.TargetObjectPath, PersistedCreateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Create persisted fingerprint matches reviewed staging"), PersistedCreateFingerprint, CreateFingerprint);
		TestFalse(TEXT("Create persisted package is clean"), bPersistedDirty);
	}

	// Update desired payload입니다.
	const FCFDAMissilePresetPayload UpdatePayload = BuildFixturePayload(Paths.StableLogicalId, TEXT("Update durable fixture"), 4.5f);
	// Update whole-record Staging JSON with exact Create baseline입니다.
	const FString UpdateJson = BuildStagingJson(UpdatePayload, Paths.TargetObjectPath, &CreateFingerprint);
	if (!TestTrue(TEXT("Update fixture Staging overwrite succeeds"), WriteStagingFile(Paths.StagingRelativePath, UpdateJson)))
	{
		return false;
	}
	// Update mutation Preview입니다.
	FCFDAStagingPreviewRow UpdatePreview;
	if (!TestTrue(TEXT("Update fixture Preview builds"), BuildPreviewFromJson(UpdateJson, Paths.StagingRelativePath, UpdatePreview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Update fixture Preview is Update"), UpdatePreview.Kind, ECFDAStagingPreviewKind::Update);
	// Update reviewed approval입니다.
	FCFDAStagingReviewedApproval UpdateApproval;
	if (!TestTrue(TEXT("Update fixture approval builds"), BuildSingleApproval(UpdatePreview, UpdateApproval, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Update durable Apply report입니다.
	FCFDAStagingApplyReport UpdateReport;
	TestTrue(TEXT("Update fixture Apply reaches durable success"), FCFDAStagingApplyService::ApplyReviewedBatch(UpdateApproval, UpdateReport));
	TestEqual(TEXT("Update fixture batch result DurableApplied"), UpdateReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("Update fixture durable count 1"), UpdateReport.DurableAppliedCount, 1);

	// Update desired semantic fingerprint입니다.
	const FString UpdateFingerprint = UpdatePreview.Record.StagingSemanticFingerprint;
	// persisted Update readback fingerprint입니다.
	FString PersistedUpdateFingerprint;
	if (TestTrue(TEXT("Update persisted semantic readback succeeds"), ReadPersistedFingerprint(Paths.TargetObjectPath, PersistedUpdateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Update persisted fingerprint matches reviewed staging"), PersistedUpdateFingerprint, UpdateFingerprint);
		TestFalse(TEXT("Update persisted package is clean"), bPersistedDirty);
	}
	return !HasAnyErrors();
}

// test-owned persisted fixture에서 baseline drift와 pre-existing dirty를 mutation0으로 차단하는지 검증합니다.
bool FCFDAStagingConflictGuardsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingPilotTestsPrivate;

	// guard fixture 시작 전 이전 run residue를 제거할 오류입니다.
	FString InitialCleanupError;
	if (!TestTrue(TEXT("Conflict guard fixture pre-clean leaves residue 0"), CleanupFixtureRoot(InitialCleanupError)))
	{
		AddError(InitialCleanupError);
		return false;
	}
	// unique guard fixture paths입니다.
	const FFixturePaths Paths = BuildFixturePaths(TEXT("DASP04Guard"));
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// conflict guard teardown residue error입니다.
		FString TeardownError;
		if (!CleanupFixtureRoot(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// persisted baseline payload입니다.
	const FCFDAMissilePresetPayload BaselinePayload = BuildFixturePayload(Paths.StableLogicalId, TEXT("Guard baseline"), 3.0f);
	// baseline Create JSON입니다.
	const FString CreateJson = BuildStagingJson(BaselinePayload, Paths.TargetObjectPath, nullptr);
	TestTrue(TEXT("Guard fixture staging write"), WriteStagingFile(Paths.StagingRelativePath, CreateJson));
	// baseline Create Preview입니다.
	FCFDAStagingPreviewRow CreatePreview;
	// guard phase error입니다.
	FString PhaseError;
	if (!BuildPreviewFromJson(CreateJson, Paths.StagingRelativePath, CreatePreview, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// baseline Create approval입니다.
	FCFDAStagingReviewedApproval CreateApproval;
	if (!BuildSingleApproval(CreatePreview, CreateApproval, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// baseline durable Create report입니다.
	FCFDAStagingApplyReport CreateReport;
	if (!TestTrue(TEXT("Guard fixture baseline durable Create"), FCFDAStagingApplyService::ApplyReviewedBatch(CreateApproval, CreateReport)))
	{
		return false;
	}
	// exact persisted baseline fingerprint입니다.
	const FString BaselineFingerprint = CreatePreview.Record.StagingSemanticFingerprint;

	// baseline에서 다른 desired state를 만드는 Update payload입니다.
	const FCFDAMissilePresetPayload DesiredUpdatePayload = BuildFixturePayload(Paths.StableLogicalId, TEXT("Guard desired update"), 4.0f);
	// baseline-bound Update JSON입니다.
	const FString UpdateJson = BuildStagingJson(DesiredUpdatePayload, Paths.TargetObjectPath, &BaselineFingerprint);

	// persisted fixture를 current memory에서 drift시킬 exact asset입니다.
	UCFMissileGuidePresetData* DriftAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, *Paths.TargetObjectPath);
	if (!TestNotNull(TEXT("Drift fixture asset exists"), DriftAsset))
	{
		return false;
	}
	DriftAsset->MissileGuideConfig.NavigationConstant = 5.0f;
	// drift injection은 dirty ownership 검증과 분리하기 위해 package dirty를 만들지 않습니다.
	UPackage* DriftPackage = DriftAsset->GetOutermost();
	TestFalse(TEXT("Memory-only drift injection remains package-clean"), DriftPackage != nullptr && DriftPackage->IsDirty());

	// drift 상태의 mutation0 Preview입니다.
	FCFDAStagingPreviewRow DriftPreview;
	if (TestTrue(TEXT("Drift Preview builds"), BuildPreviewFromJson(UpdateJson, Paths.StagingRelativePath, DriftPreview, PhaseError)))
	{
		TestEqual(TEXT("Baseline drift classifies Conflict"), DriftPreview.Kind, ECFDAStagingPreviewKind::Conflict);
		TestTrue(TEXT("Baseline drift emits BaselineMismatch"), FCFDAStagingService::HasIssueCode(DriftPreview.Issues, ECFDAStagingIssueCode::BaselineMismatch));
	}
	else
	{
		AddError(PhaseError);
	}

	DriftAsset = nullptr;
	DriftPackage = nullptr;
	if (!TestTrue(TEXT("Drift fixture reload restores persisted baseline"), ReloadFixturePackage(Paths.TargetObjectPath, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// clean persisted asset를 dirty guard용으로 다시 acquire합니다.
	UCFMissileGuidePresetData* DirtyAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, *Paths.TargetObjectPath);
	// dirty guard exact package입니다.
	UPackage* DirtyPackage = DirtyAsset != nullptr ? DirtyAsset->GetOutermost() : nullptr;
	if (!TestNotNull(TEXT("Dirty fixture asset exists"), DirtyAsset) || !TestNotNull(TEXT("Dirty fixture package exists"), DirtyPackage))
	{
		return false;
	}
	DirtyPackage->SetDirtyFlag(true);
	// pre-existing dirty 상태의 mutation0 Preview입니다.
	FCFDAStagingPreviewRow DirtyPreview;
	if (TestTrue(TEXT("Dirty Preview builds"), BuildPreviewFromJson(UpdateJson, Paths.StagingRelativePath, DirtyPreview, PhaseError)))
	{
		TestEqual(TEXT("Pre-existing dirty classifies Conflict"), DirtyPreview.Kind, ECFDAStagingPreviewKind::Conflict);
		TestTrue(TEXT("Pre-existing dirty emits TargetDirtyUnowned"), FCFDAStagingService::HasIssueCode(DirtyPreview.Issues, ECFDAStagingIssueCode::TargetDirtyUnowned));
	}
	else
	{
		AddError(PhaseError);
	}
	DirtyPackage->SetDirtyFlag(false);
	return !HasAnyErrors();
}

// test-only deterministic fault를 사용해 SavePackage outcome/confirmation uncertainty가 fail-closed되는지 검증합니다.
bool FCFDAStagingSaveUncertaintyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingPilotTestsPrivate;

	// save uncertainty fixture 시작 전 이전 run residue를 제거할 오류입니다.
	FString InitialCleanupError;
	if (!TestTrue(TEXT("Save uncertainty fixture pre-clean leaves residue 0"), CleanupFixtureRoot(InitialCleanupError)))
	{
		AddError(InitialCleanupError);
		return false;
	}
	// SavePackage outcome uncertainty fixture입니다.
	const FFixturePaths SaveFailurePaths = BuildFixturePaths(TEXT("DASP04Save"));
	// post-save confirmation uncertainty fixture입니다.
	const FFixturePaths ConfirmFailurePaths = BuildFixturePaths(TEXT("DASP04Conf"));
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// save uncertainty teardown residue error입니다.
		FString TeardownError;
		if (!CleanupFixtureRoot(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// SavePackage failure fixture payload입니다.
	const FCFDAMissilePresetPayload SavePayload = BuildFixturePayload(SaveFailurePaths.StableLogicalId, TEXT("Forced save uncertainty"), 3.2f);
	// SavePackage failure Create JSON입니다.
	const FString SaveJson = BuildStagingJson(SavePayload, SaveFailurePaths.TargetObjectPath, nullptr);
	TestTrue(TEXT("Save uncertainty staging write"), WriteStagingFile(SaveFailurePaths.StagingRelativePath, SaveJson));
	// Save uncertainty Create Preview입니다.
	FCFDAStagingPreviewRow SavePreview;
	// uncertainty phase error입니다.
	FString PhaseError;
	if (!BuildPreviewFromJson(SaveJson, SaveFailurePaths.StagingRelativePath, SavePreview, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Save uncertainty Reviewed approval입니다.
	FCFDAStagingReviewedApproval SaveApproval;
	if (!BuildSingleApproval(SavePreview, SaveApproval, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	FCFDAStagingApplyTestControl::ForceSaveFailure(SaveFailurePaths.TargetObjectPath);
	// forced SavePackage outcome uncertainty report입니다.
	FCFDAStagingApplyReport SaveReport;
	TestFalse(TEXT("Forced SavePackage uncertainty does not report success"), FCFDAStagingApplyService::ApplyReviewedBatch(SaveApproval, SaveReport));
	TestEqual(TEXT("Forced SavePackage uncertainty batch is SaveStateUnconfirmed"), SaveReport.Result, ECFDAStagingBatchApplyResult::SaveStateUnconfirmed);
	TestEqual(TEXT("Forced SavePackage uncertainty durable count 0"), SaveReport.DurableAppliedCount, 0);
	if (!SaveReport.Targets.IsEmpty())
	{
		TestEqual(TEXT("Forced SavePackage uncertainty target is SaveStateUnconfirmed"), SaveReport.Targets[0].Result, ECFDAStagingTargetApplyResult::SaveStateUnconfirmed);
	}
	FCFDAStagingApplyTestControl::Reset();

	// confirmation failure fixture payload입니다.
	const FCFDAMissilePresetPayload ConfirmPayload = BuildFixturePayload(ConfirmFailurePaths.StableLogicalId, TEXT("Forced confirmation uncertainty"), 3.4f);
	// confirmation failure Create JSON입니다.
	const FString ConfirmJson = BuildStagingJson(ConfirmPayload, ConfirmFailurePaths.TargetObjectPath, nullptr);
	TestTrue(TEXT("Confirmation uncertainty staging write"), WriteStagingFile(ConfirmFailurePaths.StagingRelativePath, ConfirmJson));
	// confirmation uncertainty Create Preview입니다.
	FCFDAStagingPreviewRow ConfirmPreview;
	if (!BuildPreviewFromJson(ConfirmJson, ConfirmFailurePaths.StagingRelativePath, ConfirmPreview, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// confirmation uncertainty Reviewed approval입니다.
	FCFDAStagingReviewedApproval ConfirmApproval;
	if (!BuildSingleApproval(ConfirmPreview, ConfirmApproval, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	FCFDAStagingApplyTestControl::ForceConfirmationFailure(ConfirmFailurePaths.TargetObjectPath);
	// forced post-save confirmation uncertainty report입니다.
	FCFDAStagingApplyReport ConfirmReport;
	TestFalse(TEXT("Forced confirmation uncertainty does not report success"), FCFDAStagingApplyService::ApplyReviewedBatch(ConfirmApproval, ConfirmReport));
	TestEqual(TEXT("Forced confirmation uncertainty batch is SaveStateUnconfirmed"), ConfirmReport.Result, ECFDAStagingBatchApplyResult::SaveStateUnconfirmed);
	TestEqual(TEXT("Forced confirmation uncertainty durable count 0"), ConfirmReport.DurableAppliedCount, 0);
	// confirmation fault occurs after actual SavePackage so disk existence is expected but must not be promoted to durable success.
	TestTrue(TEXT("Confirmation uncertainty leaves persisted file requiring triage"), FPackageName::DoesPackageExist(ConfirmFailurePaths.PackageName));
	return !HasAnyErrors();
}

// 첫 target durable 성공 뒤 두 번째 target이 known pre-mutation failure면 PartialApplied로 집계되는지 검증합니다.
bool FCFDAStagingPartialAppliedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingPilotTestsPrivate;

	// partial fixture 시작 전 이전 run residue를 제거할 오류입니다.
	FString InitialCleanupError;
	if (!TestTrue(TEXT("PartialApplied fixture pre-clean leaves residue 0"), CleanupFixtureRoot(InitialCleanupError)))
	{
		AddError(InitialCleanupError);
		return false;
	}
	// deterministic sort에서 먼저 실행될 A fixture입니다.
	const FFixturePaths FirstPaths = BuildFixturePaths(TEXT("DASP04A"));
	// deterministic sort에서 뒤에 실행될 Z fixture입니다.
	const FFixturePaths SecondPaths = BuildFixturePaths(TEXT("DASP04Z"));
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// PartialApplied teardown residue error입니다.
		FString TeardownError;
		if (!CleanupFixtureRoot(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// first durable payload입니다.
	const FCFDAMissilePresetPayload FirstPayload = BuildFixturePayload(FirstPaths.StableLogicalId, TEXT("Partial first durable"), 3.1f);
	// second blocked payload입니다.
	const FCFDAMissilePresetPayload SecondPayload = BuildFixturePayload(SecondPaths.StableLogicalId, TEXT("Partial second blocked"), 3.3f);
	// first staging JSON입니다.
	const FString FirstJson = BuildStagingJson(FirstPayload, FirstPaths.TargetObjectPath, nullptr);
	// second staging JSON입니다.
	const FString SecondJson = BuildStagingJson(SecondPayload, SecondPaths.TargetObjectPath, nullptr);
	TestTrue(TEXT("Partial first staging write"), WriteStagingFile(FirstPaths.StagingRelativePath, FirstJson));
	TestTrue(TEXT("Partial second staging write"), WriteStagingFile(SecondPaths.StagingRelativePath, SecondJson));

	// first Create Preview입니다.
	FCFDAStagingPreviewRow FirstPreview;
	// second Create Preview입니다.
	FCFDAStagingPreviewRow SecondPreview;
	// partial phase error입니다.
	FString PhaseError;
	if (!BuildPreviewFromJson(FirstJson, FirstPaths.StagingRelativePath, FirstPreview, PhaseError)
		|| !BuildPreviewFromJson(SecondJson, SecondPaths.StagingRelativePath, SecondPreview, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Partial first Preview Create"), FirstPreview.Kind, ECFDAStagingPreviewKind::Create);
	TestEqual(TEXT("Partial second Preview Create"), SecondPreview.Kind, ECFDAStagingPreviewKind::Create);

	// exact two-row reviewed mutation set입니다.
	const TArray<FCFDAStagingPreviewRow> PreviewRows = {SecondPreview, FirstPreview};
	// deterministic sorted batch approval입니다.
	FCFDAStagingReviewedApproval Approval;
	if (!TestTrue(TEXT("Partial batch approval builds"), FCFDAStagingApplyService::BuildReviewedApproval(PreviewRows, Approval, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Partial batch target count 2"), Approval.IncludedTargets.Num(), 2);
	if (Approval.IncludedTargets.Num() == 2)
	{
		TestEqual(TEXT("Partial deterministic first target is A"), Approval.IncludedTargets[0].TargetObjectPath, FirstPaths.TargetObjectPath);
		TestEqual(TEXT("Partial deterministic second target is Z"), Approval.IncludedTargets[1].TargetObjectPath, SecondPaths.TargetObjectPath);
	}

	FCFDAStagingApplyTestControl::ForceBeforeMutationBlock(SecondPaths.TargetObjectPath);
	// partial batch terminal report입니다.
	FCFDAStagingApplyReport ApplyReport;
	TestFalse(TEXT("Partial batch does not report full success"), FCFDAStagingApplyService::ApplyReviewedBatch(Approval, ApplyReport));
	TestEqual(TEXT("Partial batch aggregate is PartialApplied"), ApplyReport.Result, ECFDAStagingBatchApplyResult::PartialApplied);
	TestEqual(TEXT("Partial batch durable count 1"), ApplyReport.DurableAppliedCount, 1);
	TestEqual(TEXT("Partial batch NotRun count 0"), ApplyReport.NotRunCount, 0);
	if (ApplyReport.Targets.Num() == 2)
	{
		TestEqual(TEXT("Partial first target is DurableApplied"), ApplyReport.Targets[0].Result, ECFDAStagingTargetApplyResult::DurableApplied);
		TestEqual(TEXT("Partial second target is known BlockedBeforeMutation"), ApplyReport.Targets[1].Result, ECFDAStagingTargetApplyResult::BlockedBeforeMutation);
	}
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
