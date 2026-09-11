// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseApplyTests.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-01 VehicleDefense Reviewed Apply / durable whole-record / TOCTOU focused Automation입니다.
// Changelog:
// - v1.0.0: disposable VehicleDefense root에서 Create→Update durable round-trip, disabled shield raw-value preservation과 source/current stale mutation0 guard를 actual provider path로 검증합니다.
// Migration:
// - 실제 Save/Delete는 /Game/Test/CarFight/VDRDefenseP01 및 VehicleDefenseData/__AutomationP01__ exact test-owned root에만 한정합니다.
// - protected DA_VehicleDefense_Test, Product canonical VehicleDefense staging, DACE history와 mixed operational admission은 mutation 대상으로 사용하지 않습니다.

#include "CFDAVehicleDefenseProvider.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingApply.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
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

namespace CFDAVehicleDefenseApplyTestsPrivate
{
	// VDR-P0-01 disposable Content package root입니다.
	static const FString FixturePackageRoot = TEXT("/Game/Test/CarFight/VDRDefenseP01");

	// VDR-P0-01 disposable Staging source root입니다.
	static const FString FixtureStagingRoot = TEXT("Authoring/DataAssetStaging/VehicleDefenseData/__AutomationP01__");

	// 한 disposable VehicleDefense fixture의 exact identity/path 묶음입니다.
	struct FFixturePaths
	{
		// DefenseId와 StableLogicalId로 사용할 unique identity입니다.
		FName StableLogicalId;

		// Package 안의 exact UObject name입니다.
		FString AssetName;

		// Disposable /Game package long name입니다.
		FString PackageName;

		// Exact UObject path입니다.
		FString TargetObjectPath;

		// Provider-owned main_game-relative Staging JSON path입니다.
		FString StagingRelativePath;
	};

	// 한 directional armor config를 valid authored 값으로 구성합니다.
	FCFDirectionalArmorConfig BuildArmorConfig(const float MaximumArmor, const float DamageMultiplier)
	{
		// 반환할 directional armor config입니다.
		FCFDirectionalArmorConfig Config;
		Config.MaximumArmor = MaximumArmor;
		Config.DamageMultiplier = DamageMultiplier;
		return Config;
	}

	// GUID suffix를 사용해 collision 없는 disposable VehicleDefense fixture path를 만듭니다.
	FFixturePaths BuildFixturePaths(const FString& Prefix)
	{
		// Collision 방지용 short GUID입니다.
		const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);
		// 반환할 exact path 묶음입니다.
		FFixturePaths Paths;
		Paths.StableLogicalId = FName(*FString::Printf(TEXT("%s_%s"), *Prefix, *Suffix));
		Paths.AssetName = FString::Printf(TEXT("DA_%s_%s"), *Prefix, *Suffix);
		Paths.PackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.AssetName);
		Paths.TargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.PackageName, *Paths.AssetName);
		Paths.StagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *FixtureStagingRoot, *Paths.StableLogicalId.ToString());
		return Paths;
	}

	// 지정 identity를 사용하는 valid VehicleDefense whole-record payload를 만듭니다.
	FCFDAVehicleDefensePayload BuildPayload(const FName DefenseId)
	{
		// 반환할 valid VehicleDefense payload입니다.
		FCFDAVehicleDefensePayload Payload;
		Payload.DefenseId = DefenseId;
		Payload.DefenseMassKg = 140.0f;
		Payload.bUseShield = true;
		Payload.MaximumShield = 600.0f;
		Payload.ShieldRegenerationDelaySeconds = 4.0f;
		Payload.ShieldRegenerationPerSecond = 20.0f;
		Payload.ArmorType = ECFArmorType::Heavy;
		Payload.ArmorResistance = 90.0f;
		Payload.FrontArmorConfig = BuildArmorConfig(320.0f, 0.70f);
		Payload.LeftArmorConfig = BuildArmorConfig(230.0f, 0.90f);
		Payload.RightArmorConfig = BuildArmorConfig(230.0f, 0.90f);
		Payload.RearArmorConfig = BuildArmorConfig(170.0f, 1.10f);
		Payload.TopArmorConfig = BuildArmorConfig(110.0f, 1.20f);
		Payload.BottomArmorConfig = BuildArmorConfig(130.0f, 1.15f);
		Payload.ShieldComponentDamageScale = 0.20f;
		Payload.ArmorComponentDamageScale = 0.35f;
		Payload.IntegrityComponentDamageScale = 0.50f;
		return Payload;
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

	// main_game-relative Staging path를 absolute path로 변환합니다.
	FString GetStagingAbsolutePath(const FString& StagingRelativePath)
	{
		// Exact absolute Staging path입니다.
		FString AbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(GetMainGameRoot(), StagingRelativePath));
		FPaths::NormalizeFilename(AbsolutePath);
		return AbsolutePath;
	}

	// Exact test-owned Staging JSON을 UTF-8 without BOM으로 기록합니다.
	bool WriteStagingFile(const FString& StagingRelativePath, const FString& JsonText)
	{
		// Exact staging absolute filename입니다.
		const FString AbsolutePath = GetStagingAbsolutePath(StagingRelativePath);
		// Staging parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(AbsolutePath);
		IFileManager::Get().MakeDirectory(*ParentDirectory, true);
		return FFileHelper::SaveStringToFile(JsonText, *AbsolutePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	// Typed payload와 optional persisted baseline을 production serializer whole-record JSON으로 변환합니다.
	bool BuildJson(
		const FFixturePaths& Paths,
		const FCFDAVehicleDefensePayload& Payload,
		const FString& BaseSemanticFingerprint,
		FString& OutJson,
		FString& OutError)
	{
		return CFDAVehicleDefenseProviderImpl::SerializeStagingJson(
			Payload,
			Paths.TargetObjectPath,
			BaseSemanticFingerprint,
			OutJson,
			OutError);
	}

	// Actual VehicleDefense provider parse/current resolver/shared Preview를 한 번에 실행합니다.
	bool BuildCommonPreviewFromJson(
		const FString& JsonText,
		const FString& StagingRelativePath,
		FCFDACommonPreviewRow& OutPreviewRow,
		FString& OutError)
	{
		// Provider-local parse 결과를 payload-free로 받은 envelope입니다.
		FCFDACommonEnvelope Envelope;
		// Parse/reference validation diagnostics입니다.
		TArray<FCFDAStagingIssue> ParseIssues;
		if (!CFDAVehicleDefenseProviderImpl::ParseCommonCandidate(JsonText, StagingRelativePath, Envelope, ParseIssues))
		{
			OutError = ParseIssues.IsEmpty() ? TEXT("VehicleDefense ParseCommonCandidate가 실패했습니다.") : ParseIssues[0].Message;
			return false;
		}

		// Exact current VehicleDefense target/identity truth입니다.
		FCFDACommonCurrentState CurrentState;
		// Current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!CFDAVehicleDefenseProviderImpl::ResolveCommonCurrentState(Envelope, CurrentState, CurrentIssues))
		{
			OutError = CurrentIssues.IsEmpty() ? TEXT("VehicleDefense current-state resolver가 실패했습니다.") : CurrentIssues[0].Message;
			return false;
		}
		OutPreviewRow = CFDATypeDispatch::BuildCommonPreview(Envelope, CurrentState);
		OutError.Reset();
		return true;
	}

	// 한 common mutation Preview row를 one-shot Reviewed approval로 승격합니다.
	bool BuildSingleApproval(
		const FCFDACommonPreviewRow& PreviewRow,
		FCFDAStagingReviewedApproval& OutApproval,
		FString& OutError)
	{
		// Exact one-row common Preview set입니다.
		const TArray<FCFDACommonPreviewRow> PreviewRows = {PreviewRow};
		return CFDATypeDispatch::BuildCommonReviewedApproval(PreviewRows, OutApproval, OutError);
	}

	// Persisted VehicleDefense fixture의 exact typed payload와 semantic fingerprint를 읽습니다.
	bool ReadPersistedDefense(
		const FString& TargetObjectPath,
		FCFDAVehicleDefensePayload& OutPayload,
		FString& OutFingerprint,
		bool& bOutPackageDirty,
		FString& OutError)
	{
		// Exact persisted UCFVehicleDefenseData입니다.
		UCFVehicleDefenseData* Asset = LoadObject<UCFVehicleDefenseData>(nullptr, *TargetObjectPath);
		if (Asset == nullptr)
		{
			OutError = FString::Printf(TEXT("Persisted VehicleDefense fixture를 로드하지 못했습니다: %s"), *TargetObjectPath);
			return false;
		}
		// Exact owning package입니다.
		UPackage* Package = Asset->GetOutermost();
		bOutPackageDirty = Package != nullptr && Package->IsDirty();
		// Production extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!CFDAVehicleDefenseProviderImpl::ExtractPayload(*Asset, OutPayload, ExtractIssues))
		{
			OutError = ExtractIssues.IsEmpty() ? TEXT("Persisted VehicleDefense payload 추출에 실패했습니다.") : ExtractIssues[0].Message;
			return false;
		}
		return CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(OutPayload, OutFingerprint, OutError);
	}

	// Object path가 VDR-P0-01 disposable package root에 속하는지 확인합니다.
	bool IsFixtureObjectPath(const FString& ObjectPath)
	{
		return ObjectPath.StartsWith(FixturePackageRoot + TEXT("/"), ESearchCase::CaseSensitive);
	}

	// AssetRegistry/loaded UObject/physical Content/Staging 네 authority에서 disposable residue 0을 검증합니다.
	bool VerifyFixtureRootResidueFree(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Disposable root 아래 registry-visible assets입니다.
		TArray<FAssetData> RegistryAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), RegistryAssets, true, true);
		if (!RegistryAssets.IsEmpty())
		{
			OutError = FString::Printf(TEXT("VDR-P0-01 VehicleDefense AssetRegistry residue가 %d개 남았습니다."), RegistryAssets.Num());
			return false;
		}

		for (TObjectIterator<UCFVehicleDefenseData> DefenseIterator; DefenseIterator; ++DefenseIterator)
		{
			// Current process의 loaded UCFVehicleDefenseData 후보입니다.
			const UCFVehicleDefenseData* LoadedDefense = *DefenseIterator;
			if (LoadedDefense == nullptr || LoadedDefense->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded VehicleDefense exact object path입니다.
			const FString LoadedPath = FSoftObjectPath(LoadedDefense).ToString();
			if (IsFixtureObjectPath(LoadedPath))
			{
				OutError = FString::Printf(TEXT("VDR-P0-01 VehicleDefense loaded UObject residue가 남았습니다: %s"), *LoadedPath);
				return false;
			}
		}

		// Disposable Content physical directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/VDRDefenseP01")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = TEXT("VDR-P0-01 VehicleDefense physical Content root residue가 남았습니다.");
			return false;
		}

		// Disposable Staging physical directory입니다.
		const FString StagingDirectory = GetStagingAbsolutePath(FixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*StagingDirectory))
		{
			OutError = TEXT("VDR-P0-01 VehicleDefense physical Staging root residue가 남았습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// VDR-P0-01 exact disposable Content/Staging roots만 unload→GC→disk delete→registry refresh 순서로 정리합니다.
	bool CleanupFixtureRoot(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Registry-visible disposable assets입니다.
		TArray<FAssetData> FixtureAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), FixtureAssets, true, true);
		// Unload할 unique package 목록입니다.
		TArray<UPackage*> PackagesToUnload;
		// Disk delete 뒤 registry refresh에 사용할 exact filenames입니다.
		TArray<FString> DeletedFilenames;

		for (const FAssetData& FixtureAssetData : FixtureAssets)
		{
			// Loaded 또는 persisted fixture object입니다.
			UObject* FixtureObject = FixtureAssetData.GetSoftObjectPath().ResolveObject();
			if (FixtureObject == nullptr)
			{
				FixtureObject = FixtureAssetData.GetAsset();
			}
			// Fixture owning package입니다.
			UPackage* FixturePackage = FixtureObject != nullptr ? FixtureObject->GetOutermost() : nullptr;
			if (FixturePackage != nullptr)
			{
				FixturePackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(FixturePackage);
			}
			// Expected persisted .uasset filename입니다.
			const FString PackageFilename = FPackageName::LongPackageNameToFilename(FixtureAssetData.PackageName.ToString(), FPackageName::GetAssetPackageExtension());
			DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(PackageFilename));
		}

		for (TObjectIterator<UCFVehicleDefenseData> DefenseIterator; DefenseIterator; ++DefenseIterator)
		{
			// Registry에 없더라도 resolver가 볼 수 있는 loaded fixture입니다.
			UCFVehicleDefenseData* LoadedDefense = *DefenseIterator;
			if (LoadedDefense == nullptr || LoadedDefense->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded exact object path입니다.
			const FString LoadedPath = FSoftObjectPath(LoadedDefense).ToString();
			if (!IsFixtureObjectPath(LoadedPath))
			{
				continue;
			}
			// Loaded fixture owning package입니다.
			UPackage* LoadedPackage = LoadedDefense->GetOutermost();
			if (LoadedPackage != nullptr)
			{
				LoadedPackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(LoadedPackage);
				// Loaded-only package도 disk refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(LoadedPackage->GetName(), FPackageName::GetAssetPackageExtension());
				DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedDefense->ClearFlags(RF_Standalone);
		}

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("VDR-P0-01 VehicleDefense fixture package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Disposable physical Content root입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/VDRDefenseP01")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("VDR-P0-01 VehicleDefense Content root 삭제에 실패했습니다: %s"), *ContentDirectory);
			return false;
		}
		if (!DeletedFilenames.IsEmpty())
		{
			AssetRegistry.ScanModifiedAssetFiles(DeletedFilenames);
			AssetRegistry.WaitForCompletion();
		}

		// Disposable physical Staging root입니다.
		const FString StagingDirectory = GetStagingAbsolutePath(FixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*StagingDirectory)
			&& !IFileManager::Get().DeleteDirectory(*StagingDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("VDR-P0-01 VehicleDefense Staging root 삭제에 실패했습니다: %s"), *StagingDirectory);
			return false;
		}
		return VerifyFixtureRootResidueFree(OutError);
	}

	// Disposable fixture를 Create로 durable 저장하고 reviewed fingerprint를 반환합니다.
	bool CreateDurableFixture(
		FAutomationTestBase& Test,
		const FFixturePaths& Paths,
		const FCFDAVehicleDefensePayload& Payload,
		FString& OutReviewedFingerprint,
		FString& OutError)
	{
		// Create whole-record JSON입니다.
		FString JsonText;
		if (!BuildJson(Paths, Payload, FString(), JsonText, OutError))
		{
			return false;
		}
		if (!WriteStagingFile(Paths.StagingRelativePath, JsonText))
		{
			OutError = TEXT("VehicleDefense fixture Staging write에 실패했습니다.");
			return false;
		}
		// Actual provider/common Create Preview입니다.
		FCFDACommonPreviewRow PreviewRow;
		if (!BuildCommonPreviewFromJson(JsonText, Paths.StagingRelativePath, PreviewRow, OutError))
		{
			return false;
		}
		Test.TestEqual(TEXT("VehicleDefense disposable fixture Preview must be Create"), PreviewRow.Kind, ECFDAStagingPreviewKind::Create);
		if (PreviewRow.Kind != ECFDAStagingPreviewKind::Create)
		{
			OutError = TEXT("VehicleDefense disposable fixture가 Create로 분류되지 않았습니다.");
			return false;
		}
		OutReviewedFingerprint = PreviewRow.Envelope.StagingSemanticFingerprint;
		// Exact one-shot reviewed approval입니다.
		FCFDAStagingReviewedApproval Approval;
		if (!BuildSingleApproval(PreviewRow, Approval, OutError))
		{
			return false;
		}
		// Exact durable Apply report입니다.
		FCFDAStagingApplyReport ApplyReport;
		if (!FCFDAStagingApplyService::ApplyReviewedBatch(Approval, ApplyReport)
			|| ApplyReport.Result != ECFDAStagingBatchApplyResult::DurableApplied
			|| ApplyReport.DurableAppliedCount != 1)
		{
			OutError = ApplyReport.Diagnostic.IsEmpty() ? TEXT("VehicleDefense Create durable Apply가 실패했습니다.") : ApplyReport.Diagnostic;
			return false;
		}
		OutError.Reset();
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefDurableTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_01.DurableRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefStaleTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_01.StaleGuards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Actual VehicleDefense provider에서 Create→Update durable round-trip과 disabled shield raw-value preservation을 검증합니다.
bool FCFDAVDefDurableTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseApplyTestsPrivate;

	// Test 시작 전 stale disposable residue 제거 상세입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("VDR-P0-01 durable pre-clean leaves residue0"), CleanupFixtureRoot(CleanupError)))
	{
		AddError(CleanupError);
		return false;
	}
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// Test 종료 disposable root 정리 상세입니다.
		FString TeardownError;
		if (!CleanupFixtureRoot(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Unique durable VehicleDefense fixture path입니다.
	const FFixturePaths Paths = BuildFixturePaths(TEXT("VDRP01Dur"));
	// Initial Create payload입니다.
	const FCFDAVehicleDefensePayload CreatePayload = BuildPayload(Paths.StableLogicalId);
	// Reviewed Create semantic fingerprint입니다.
	FString CreateFingerprint;
	// Current durable phase error입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("VehicleDefense durable Create succeeds"), CreateDurableFixture(*this, Paths, CreatePayload, CreateFingerprint, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Persisted Create exact whole-record payload입니다.
	FCFDAVehicleDefensePayload PersistedCreatePayload;
	// Persisted Create semantic fingerprint입니다.
	FString PersistedCreateFingerprint;
	// Persisted package dirty state입니다.
	bool bPersistedDirty = true;
	if (TestTrue(TEXT("VehicleDefense Create persisted readback succeeds"), ReadPersistedDefense(Paths.TargetObjectPath, PersistedCreatePayload, PersistedCreateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Create persisted fingerprint equals reviewed staging"), PersistedCreateFingerprint, CreateFingerprint);
		TestFalse(TEXT("Create persisted package remains clean"), bPersistedDirty);
		TestEqual(TEXT("Create ArmorType"), PersistedCreatePayload.ArmorType, ECFArmorType::Heavy);
		TestEqual(TEXT("Create Front MaximumArmor"), PersistedCreatePayload.FrontArmorConfig.MaximumArmor, 320.0f);
		TestEqual(TEXT("Create Bottom DamageMultiplier"), PersistedCreatePayload.BottomArmorConfig.DamageMultiplier, 1.15f);
	}
	else
	{
		AddError(PhaseError);
		return false;
	}

	// Update에서 disabled shield raw-value preservation을 직접 검증할 payload입니다.
	FCFDAVehicleDefensePayload UpdatePayload = BuildPayload(Paths.StableLogicalId);
	UpdatePayload.DefenseMassKg = 155.0f;
	UpdatePayload.bUseShield = false;
	UpdatePayload.MaximumShield = 777.0f;
	UpdatePayload.ShieldRegenerationDelaySeconds = 8.0f;
	UpdatePayload.ShieldRegenerationPerSecond = 31.0f;
	UpdatePayload.ArmorType = ECFArmorType::Standard;
	UpdatePayload.ArmorResistance = 65.0f;
	UpdatePayload.FrontArmorConfig.MaximumArmor = 285.0f;
	UpdatePayload.FrontArmorConfig.DamageMultiplier = 0.82f;
	UpdatePayload.ShieldComponentDamageScale = 0.24f;
	UpdatePayload.ArmorComponentDamageScale = 0.38f;
	UpdatePayload.IntegrityComponentDamageScale = 0.56f;

	// Create baseline에 binding된 Update JSON입니다.
	FString UpdateJson;
	if (!TestTrue(TEXT("VehicleDefense Update JSON serializes"), BuildJson(Paths, UpdatePayload, CreateFingerprint, UpdateJson, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("VehicleDefense Update staging overwrite succeeds"), WriteStagingFile(Paths.StagingRelativePath, UpdateJson));
	// Actual provider/common Update Preview입니다.
	FCFDACommonPreviewRow UpdatePreview;
	if (!TestTrue(TEXT("VehicleDefense Update Preview builds"), BuildCommonPreviewFromJson(UpdateJson, Paths.StagingRelativePath, UpdatePreview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("VehicleDefense Update Preview is Update"), UpdatePreview.Kind, ECFDAStagingPreviewKind::Update);
	// Exact Update Reviewed approval입니다.
	FCFDAStagingReviewedApproval UpdateApproval;
	if (!TestTrue(TEXT("VehicleDefense Update approval builds"), BuildSingleApproval(UpdatePreview, UpdateApproval, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Exact Update durable Apply report입니다.
	FCFDAStagingApplyReport UpdateReport;
	TestTrue(TEXT("VehicleDefense durable Update succeeds"), FCFDAStagingApplyService::ApplyReviewedBatch(UpdateApproval, UpdateReport));
	TestEqual(TEXT("VehicleDefense durable Update result"), UpdateReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("VehicleDefense durable Update count"), UpdateReport.DurableAppliedCount, 1);

	// Persisted Update whole-record payload입니다.
	FCFDAVehicleDefensePayload PersistedUpdatePayload;
	// Persisted Update semantic fingerprint입니다.
	FString PersistedUpdateFingerprint;
	if (TestTrue(TEXT("VehicleDefense Update persisted readback succeeds"), ReadPersistedDefense(Paths.TargetObjectPath, PersistedUpdatePayload, PersistedUpdateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Update persisted fingerprint equals reviewed staging"), PersistedUpdateFingerprint, UpdatePreview.Envelope.StagingSemanticFingerprint);
		TestFalse(TEXT("Update persisted package remains clean"), bPersistedDirty);
		TestFalse(TEXT("Update bUseShield false preserved"), PersistedUpdatePayload.bUseShield);
		TestEqual(TEXT("Disabled MaximumShield raw preserved"), PersistedUpdatePayload.MaximumShield, 777.0f);
		TestEqual(TEXT("Disabled regen delay raw preserved"), PersistedUpdatePayload.ShieldRegenerationDelaySeconds, 8.0f);
		TestEqual(TEXT("Disabled regen rate raw preserved"), PersistedUpdatePayload.ShieldRegenerationPerSecond, 31.0f);
		TestEqual(TEXT("Update ArmorType"), PersistedUpdatePayload.ArmorType, ECFArmorType::Standard);
		TestEqual(TEXT("Update ArmorResistance"), PersistedUpdatePayload.ArmorResistance, 65.0f);
		TestEqual(TEXT("Update Front MaximumArmor"), PersistedUpdatePayload.FrontArmorConfig.MaximumArmor, 285.0f);
		TestEqual(TEXT("Update Front DamageMultiplier"), PersistedUpdatePayload.FrontArmorConfig.DamageMultiplier, 0.82f);
		TestEqual(TEXT("Update ShieldComponentDamageScale"), PersistedUpdatePayload.ShieldComponentDamageScale, 0.24f);
		TestEqual(TEXT("Update ArmorComponentDamageScale"), PersistedUpdatePayload.ArmorComponentDamageScale, 0.38f);
		TestEqual(TEXT("Update IntegrityComponentDamageScale"), PersistedUpdatePayload.IntegrityComponentDamageScale, 0.56f);
	}
	else
	{
		AddError(PhaseError);
	}
	return !HasAnyErrors();
}

// Actual VehicleDefense provider path에서 reviewed source stale와 current semantic stale를 mutation0으로 차단하는지 검증합니다.
bool FCFDAVDefStaleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAVehicleDefenseApplyTestsPrivate;

	// Test 시작 전 stale disposable residue 제거 상세입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("VDR-P0-01 stale pre-clean leaves residue0"), CleanupFixtureRoot(CleanupError)))
	{
		AddError(CleanupError);
		return false;
	}
	FCFDAStagingApplyTestControl::Reset();
	ON_SCOPE_EXIT
	{
		FCFDAStagingApplyTestControl::Reset();
		// Test 종료 disposable root 정리 상세입니다.
		FString TeardownError;
		if (!CleanupFixtureRoot(TeardownError))
		{
			AddError(TeardownError);
		}
	};

	// Source stale 검증용 unique path입니다.
	const FFixturePaths SourceStalePaths = BuildFixturePaths(TEXT("VDRP01Src"));
	// Review 전 original Create payload입니다.
	FCFDAVehicleDefensePayload OriginalSourcePayload = BuildPayload(SourceStalePaths.StableLogicalId);
	// Review 전 original Create JSON입니다.
	FString OriginalSourceJson;
	// Current stale guard phase error입니다.
	FString PhaseError;
	if (!BuildJson(SourceStalePaths, OriginalSourcePayload, FString(), OriginalSourceJson, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Source-stale VehicleDefense staging write"), WriteStagingFile(SourceStalePaths.StagingRelativePath, OriginalSourceJson));
	// Reviewed 전 original Preview입니다.
	FCFDACommonPreviewRow OriginalSourcePreview;
	if (!BuildCommonPreviewFromJson(OriginalSourceJson, SourceStalePaths.StagingRelativePath, OriginalSourcePreview, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Original exact approval입니다.
	FCFDAStagingReviewedApproval SourceStaleApproval;
	if (!BuildSingleApproval(OriginalSourcePreview, SourceStaleApproval, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Review 뒤 disk Staging semantic을 바꾼 payload입니다.
	FCFDAVehicleDefensePayload ChangedSourcePayload = OriginalSourcePayload;
	ChangedSourcePayload.ArmorResistance += 1.0f;
	// Review 뒤 변경된 JSON입니다.
	FString ChangedAfterReviewJson;
	if (!BuildJson(SourceStalePaths, ChangedSourcePayload, FString(), ChangedAfterReviewJson, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Source-stale VehicleDefense post-review rewrite"), WriteStagingFile(SourceStalePaths.StagingRelativePath, ChangedAfterReviewJson));
	// Source-stale apply report입니다.
	FCFDAStagingApplyReport SourceStaleReport;
	TestFalse(TEXT("Changed post-review VehicleDefense Staging blocks Apply"), FCFDAStagingApplyService::ApplyReviewedBatch(SourceStaleApproval, SourceStaleReport));
	TestEqual(TEXT("Source-stale VehicleDefense batch blocked before mutation"), SourceStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Source-stale durable count remains0"), SourceStaleReport.DurableAppliedCount, 0);
	TestFalse(TEXT("Source-stale VehicleDefense target remains absent"), FPackageName::DoesPackageExist(SourceStalePaths.PackageName));

	// Current stale 검증용 별도 unique path입니다.
	const FFixturePaths CurrentStalePaths = BuildFixturePaths(TEXT("VDRP01Cur"));
	// Current-stale baseline Create payload입니다.
	const FCFDAVehicleDefensePayload CurrentCreatePayload = BuildPayload(CurrentStalePaths.StableLogicalId);
	// Durable current baseline fingerprint입니다.
	FString CurrentBaselineFingerprint;
	if (!TestTrue(TEXT("Current-stale VehicleDefense baseline Create succeeds"), CreateDurableFixture(*this, CurrentStalePaths, CurrentCreatePayload, CurrentBaselineFingerprint, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Existing baseline에 binding된 Update payload입니다.
	FCFDAVehicleDefensePayload CurrentUpdatePayload = BuildPayload(CurrentStalePaths.StableLogicalId);
	CurrentUpdatePayload.ArmorResistance = 115.0f;
	CurrentUpdatePayload.FrontArmorConfig.MaximumArmor = 360.0f;
	// Existing baseline에 binding된 Update JSON입니다.
	FString CurrentUpdateJson;
	if (!BuildJson(CurrentStalePaths, CurrentUpdatePayload, CurrentBaselineFingerprint, CurrentUpdateJson, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	TestTrue(TEXT("Current-stale VehicleDefense Update staging write"), WriteStagingFile(CurrentStalePaths.StagingRelativePath, CurrentUpdateJson));
	// Review 시점의 clean current baseline을 반영한 Update Preview입니다.
	FCFDACommonPreviewRow CurrentUpdatePreview;
	if (!TestTrue(TEXT("Current-stale VehicleDefense Update Preview builds"), BuildCommonPreviewFromJson(CurrentUpdateJson, CurrentStalePaths.StagingRelativePath, CurrentUpdatePreview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Current-stale baseline Preview is Update"), CurrentUpdatePreview.Kind, ECFDAStagingPreviewKind::Update);
	// Current-stale exact reviewed approval입니다.
	FCFDAStagingReviewedApproval CurrentStaleApproval;
	if (!BuildSingleApproval(CurrentUpdatePreview, CurrentStaleApproval, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}

	// Review 뒤 process-local current semantic을 변경할 disposable VehicleDefense asset입니다.
	UCFVehicleDefenseData* CurrentStaleAsset = LoadObject<UCFVehicleDefenseData>(nullptr, *CurrentStalePaths.TargetObjectPath);
	// Current-stale disposable owning package입니다.
	UPackage* CurrentStalePackage = CurrentStaleAsset != nullptr ? CurrentStaleAsset->GetOutermost() : nullptr;
	if (!TestNotNull(TEXT("Current-stale VehicleDefense target exists"), CurrentStaleAsset)
		|| !TestNotNull(TEXT("Current-stale VehicleDefense package exists"), CurrentStalePackage))
	{
		return false;
	}
	// Review 이후 만든 exact current semantic drift 값입니다.
	const float DriftedArmorResistance = CurrentStaleAsset->ArmorResistance + 7.0f;
	CurrentStaleAsset->ArmorResistance = DriftedArmorResistance;
	CurrentStalePackage->SetDirtyFlag(false);

	// Current-stale apply report입니다.
	FCFDAStagingApplyReport CurrentStaleReport;
	TestFalse(TEXT("Changed post-review VehicleDefense current semantic blocks Apply"), FCFDAStagingApplyService::ApplyReviewedBatch(CurrentStaleApproval, CurrentStaleReport));
	TestEqual(TEXT("Current-stale VehicleDefense batch blocked before mutation"), CurrentStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Current-stale VehicleDefense durable count remains0"), CurrentStaleReport.DurableAppliedCount, 0);
	TestEqual(TEXT("Current-stale mutation0 preserves drifted current value"), CurrentStaleAsset->ArmorResistance, DriftedArmorResistance);
	TestFalse(TEXT("Current-stale disposable package stays clean after block"), CurrentStalePackage->IsDirty());
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
