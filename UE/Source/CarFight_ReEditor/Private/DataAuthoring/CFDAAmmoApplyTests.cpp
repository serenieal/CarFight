// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoApplyTests.cpp
// Version: v1.0.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-03 Ammo Reviewed Apply / durable typed writer focused Automation입니다.
// Changelog:
// - v1.0.0: disposable Ammo root에서 Create/Update durable round-trip, stale/dirty guard, SaveStateUnconfirmed을 actual provider path로 검증.
// Migration:
// - 실제 Save/Delete는 /Game/Test/CarFight/DAOAmmoP03 과 AmmoData/__AutomationP03__ exact test-owned root에만 한정합니다.
// - Product Ammo exact0, persisted HeavyFinite/RocketFinite exact2, Missile Product/accepted baseline은 읽거나 저장 대상으로 사용하지 않습니다.

#include "CFDAAmmoProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingApply.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFAmmoData.h"
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

namespace CFDAAmmoApplyTestsPrivate
{
	// DAO-P0-03 disposable Content package root입니다.
	static const FString FixturePackageRoot = TEXT("/Game/Test/CarFight/DAOAmmoP03");

	// DAO-P0-03 disposable Staging source root입니다.
	static const FString FixtureStagingRoot = TEXT("Authoring/DataAssetStaging/AmmoData/__AutomationP03__");

	// Non-null AmmoIcon durable round-trip에 사용할 engine-owned Texture2D입니다.
	static const FString FixtureIconPath = TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture");

	// 한 disposable Ammo fixture의 exact identity/path 묶음입니다.
	struct FFixturePaths
	{
		// AmmoId와 StableLogicalId로 사용할 unique identity입니다.
		FString StableLogicalId;

		// package 안의 exact UObject name입니다.
		FString AssetName;

		// disposable /Game package long name입니다.
		FString PackageName;

		// exact UObject path입니다.
		FString TargetObjectPath;

		// provider-owned main_game-relative Staging JSON path입니다.
		FString StagingRelativePath;
	};

	// GUID suffix를 사용해 collision 없는 disposable Ammo fixture path를 만듭니다.
	FFixturePaths BuildFixturePaths(const FString& Prefix)
	{
		// Collision 방지용 short GUID입니다.
		const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);
		// 반환할 exact path 묶음입니다.
		FFixturePaths Paths;
		Paths.StableLogicalId = FString::Printf(TEXT("%s_%s"), *Prefix, *Suffix);
		Paths.AssetName = FString::Printf(TEXT("DA_%s_%s"), *Prefix, *Suffix);
		Paths.PackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.AssetName);
		Paths.TargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.PackageName, *Paths.AssetName);
		Paths.StagingRelativePath = FString::Printf(TEXT("%s/%s.json"), *FixtureStagingRoot, *Paths.StableLogicalId);
		return Paths;
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
		return FFileHelper::SaveStringToFile(
			JsonText,
			*AbsolutePath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	// Strict Ammo exact8 whole-record JSON을 구성합니다.
	FString BuildAmmoJson(
		const FFixturePaths& Paths,
		const FString* BaseSemanticFingerprint,
		const FString& DisplayName,
		const FString& FamilyId,
		const FString& UnitMassText,
		const FString& TagsJson,
		const FString& AmmoIconJson,
		const FString& MaximumCountText,
		const bool bCanBeResupplied)
	{
		// BaseSemanticFingerprint의 exact JSON representation입니다.
		const FString BaseJson = BaseSemanticFingerprint != nullptr
			? FString::Printf(TEXT("\"%s\""), **BaseSemanticFingerprint)
			: TEXT("null");
		// JSON bool token입니다.
		const TCHAR* ResupplyJson = bCanBeResupplied ? TEXT("true") : TEXT("false");
		return FString::Printf(
			TEXT("{")
			TEXT("\"SchemaId\":\"CarFight.DataAsset.AmmoData\",")
			TEXT("\"SchemaRevision\":1,")
			TEXT("\"AdapterContractRevision\":1,")
			TEXT("\"DataAssetTypeClassPath\":\"/Script/CarFight_Re.CFAmmoData\",")
			TEXT("\"StableLogicalId\":\"%s\",")
			TEXT("\"TargetObjectPath\":\"%s\",")
			TEXT("\"BaseSemanticFingerprint\":%s,")
			TEXT("\"Payload\":{")
			TEXT("\"AmmoId\":\"%s\",")
			TEXT("\"AmmoDisplayName\":{\"Kind\":\"Literal\",\"Text\":\"%s\"},")
			TEXT("\"AmmoFamilyId\":\"%s\",")
			TEXT("\"UnitMassKg\":%s,")
			TEXT("\"AmmoTags\":%s,")
			TEXT("\"AmmoIcon\":%s,")
			TEXT("\"MaximumLoadableAmmoCount\":%s,")
			TEXT("\"bCanBeResupplied\":%s")
			TEXT("}")
			TEXT("}"),
			*Paths.StableLogicalId,
			*Paths.TargetObjectPath,
			*BaseJson,
			*Paths.StableLogicalId,
			*DisplayName,
			*FamilyId,
			*UnitMassText,
			*TagsJson,
			*AmmoIconJson,
			*MaximumCountText,
			ResupplyJson);
	}

	// Actual Ammo provider parse/current resolver/shared Preview를 한 번에 실행합니다.
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
		if (!CFDAAmmoProviderImpl::ParseCommonCandidate(JsonText, StagingRelativePath, Envelope, ParseIssues))
		{
			OutError = ParseIssues.IsEmpty() ? TEXT("Ammo ParseCommonCandidate가 실패했습니다.") : ParseIssues[0].Message;
			return false;
		}

		// Exact current CFAmmoData target/identity truth입니다.
		FCFDACommonCurrentState CurrentState;
		// Current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!CFDAAmmoProviderImpl::ResolveCommonCurrentState(Envelope, CurrentState, CurrentIssues))
		{
			OutError = CurrentIssues.IsEmpty() ? TEXT("Ammo current-state resolver가 실패했습니다.") : CurrentIssues[0].Message;
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

	// Persisted Ammo fixture의 exact typed payload와 semantic fingerprint를 읽습니다.
	bool ReadPersistedAmmo(
		const FString& TargetObjectPath,
		FCFDAAmmoPayload& OutPayload,
		FString& OutFingerprint,
		bool& bOutPackageDirty,
		FString& OutError)
	{
		// Exact persisted CFAmmoData입니다.
		UCFAmmoData* Asset = LoadObject<UCFAmmoData>(nullptr, *TargetObjectPath);
		if (Asset == nullptr)
		{
			OutError = FString::Printf(TEXT("Persisted Ammo fixture를 로드하지 못했습니다: %s"), *TargetObjectPath);
			return false;
		}
		// Exact owning package입니다.
		UPackage* Package = Asset->GetOutermost();
		bOutPackageDirty = Package != nullptr && Package->IsDirty();
		// Production extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!CFDAAmmoProviderImpl::ExtractPayload(*Asset, OutPayload, ExtractIssues))
		{
			OutError = ExtractIssues.IsEmpty() ? TEXT("Persisted Ammo payload 추출에 실패했습니다.") : ExtractIssues[0].Message;
			return false;
		}
		return CFDAAmmoProviderImpl::BuildSemanticFingerprint(OutPayload, OutFingerprint, OutError);
	}

	// Object path가 DAO-P0-03 disposable package root에 속하는지 확인합니다.
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
			OutError = FString::Printf(TEXT("DAO-P0-03 Ammo AssetRegistry residue가 %d개 남았습니다."), RegistryAssets.Num());
			return false;
		}

		for (TObjectIterator<UCFAmmoData> AmmoIterator; AmmoIterator; ++AmmoIterator)
		{
			// Current process의 loaded CFAmmoData 후보입니다.
			const UCFAmmoData* LoadedAmmo = *AmmoIterator;
			if (LoadedAmmo == nullptr || LoadedAmmo->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded Ammo exact object path입니다.
			const FString LoadedPath = FSoftObjectPath(LoadedAmmo).ToString();
			if (IsFixtureObjectPath(LoadedPath))
			{
				OutError = FString::Printf(TEXT("DAO-P0-03 Ammo loaded UObject residue가 남았습니다: %s"), *LoadedPath);
				return false;
			}
		}

		// Disposable Content physical directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DAOAmmoP03")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = TEXT("DAO-P0-03 Ammo physical Content root residue가 남았습니다.");
			return false;
		}

		// Disposable Staging physical directory입니다.
		const FString StagingDirectory = GetStagingAbsolutePath(FixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*StagingDirectory))
		{
			OutError = TEXT("DAO-P0-03 Ammo physical Staging root residue가 남았습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// DAO-P0-03 exact disposable Content/Staging roots만 unload→GC→disk delete→registry refresh 순서로 정리합니다.
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
			const FString PackageFilename = FPackageName::LongPackageNameToFilename(
				FixtureAssetData.PackageName.ToString(),
				FPackageName::GetAssetPackageExtension());
			DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(PackageFilename));
		}

		for (TObjectIterator<UCFAmmoData> AmmoIterator; AmmoIterator; ++AmmoIterator)
		{
			// Registry에 없더라도 resolver가 볼 수 있는 loaded fixture입니다.
			UCFAmmoData* LoadedAmmo = *AmmoIterator;
			if (LoadedAmmo == nullptr || LoadedAmmo->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded exact object path입니다.
			const FString LoadedPath = FSoftObjectPath(LoadedAmmo).ToString();
			if (!IsFixtureObjectPath(LoadedPath))
			{
				continue;
			}
			// Loaded fixture owning package입니다.
			UPackage* LoadedPackage = LoadedAmmo->GetOutermost();
			if (LoadedPackage != nullptr)
			{
				LoadedPackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(LoadedPackage);
				// Loaded-only package도 disk refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(
					LoadedPackage->GetName(),
					FPackageName::GetAssetPackageExtension());
				DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedAmmo->ClearFlags(RF_Standalone);
		}

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("DAO-P0-03 Ammo fixture package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Disposable physical Content root입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DAOAmmoP03")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("DAO-P0-03 Ammo Content root 삭제에 실패했습니다: %s"), *ContentDirectory);
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
			OutError = FString::Printf(TEXT("DAO-P0-03 Ammo Staging root 삭제에 실패했습니다: %s"), *StagingDirectory);
			return false;
		}
		return VerifyFixtureRootResidueFree(OutError);
	}

	// Disposable fixture를 Create로 durable 저장하고 reviewed fingerprint를 반환합니다.
	bool CreateDurableFixture(
		FAutomationTestBase& Test,
		const FFixturePaths& Paths,
		const FString& JsonText,
		FString& OutReviewedFingerprint,
		FString& OutError)
	{
		if (!WriteStagingFile(Paths.StagingRelativePath, JsonText))
		{
			OutError = TEXT("Ammo fixture Staging write에 실패했습니다.");
			return false;
		}
		// Actual provider/common Create Preview입니다.
		FCFDACommonPreviewRow PreviewRow;
		if (!BuildCommonPreviewFromJson(JsonText, Paths.StagingRelativePath, PreviewRow, OutError))
		{
			return false;
		}
		Test.TestEqual(TEXT("Ammo disposable fixture Preview must be Create"), PreviewRow.Kind, ECFDAStagingPreviewKind::Create);
		if (PreviewRow.Kind != ECFDAStagingPreviewKind::Create)
		{
			OutError = TEXT("Ammo disposable fixture가 Create로 분류되지 않았습니다.");
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
			OutError = ApplyReport.Diagnostic.IsEmpty() ? TEXT("Ammo Create durable Apply가 실패했습니다.") : ApplyReport.Diagnostic;
			return false;
		}
		OutError.Reset();
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoDurableRoundTripTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_03.DurableRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoStaleDirtyGuardsTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_03.StaleDirtyGuards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoSaveUncertaintyTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_03.SaveUncertainty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Actual Ammo provider에서 non-empty tags/non-null icon Create→Update durable round-trip을 검증합니다.
bool FCFDAAmmoDurableRoundTripTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAAmmoApplyTestsPrivate;

	// Test 시작 전 stale disposable residue 제거 상세입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("DAO-P0-03 durable pre-clean leaves residue 0"), CleanupFixtureRoot(CleanupError)))
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

	// Unique durable Ammo fixture path입니다.
	const FFixturePaths Paths = BuildFixturePaths(TEXT("DAOP03Dur"));
	// Non-null Texture2D SoftObjectPath JSON token입니다.
	const FString IconJson = FString::Printf(TEXT("\"%s\""), *FixtureIconPath);
	// Initial Create exact8 JSON입니다.
	const FString CreateJson = BuildAmmoJson(
		Paths,
		nullptr,
		TEXT("DAO P0-03 생성 탄약"),
		TEXT("AutomationFamily"),
		TEXT("2.5"),
		TEXT("[\"Practice\",\"AP\"]"),
		IconJson,
		TEXT("24"),
		true);
	// Reviewed Create fingerprint입니다.
	FString CreateFingerprint;
	// Current durable phase error입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("Ammo durable Create succeeds"), CreateDurableFixture(*this, Paths, CreateJson, CreateFingerprint, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Persisted Create exact8 payload입니다.
	FCFDAAmmoPayload PersistedCreatePayload;
	// Persisted Create semantic fingerprint입니다.
	FString PersistedCreateFingerprint;
	// Persisted Create package dirty state입니다.
	bool bPersistedDirty = true;
	if (TestTrue(
		TEXT("Ammo durable Create persisted readback succeeds"),
		ReadPersistedAmmo(Paths.TargetObjectPath, PersistedCreatePayload, PersistedCreateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Create persisted fingerprint equals reviewed staging"), PersistedCreateFingerprint, CreateFingerprint);
		TestFalse(TEXT("Create persisted package remains clean"), bPersistedDirty);
		TestEqual(TEXT("Create non-empty tags count"), PersistedCreatePayload.AmmoTags.Num(), 2);
		TestEqual(TEXT("Create non-null AmmoIcon path"), PersistedCreatePayload.AmmoIcon.ToString(), FixtureIconPath);
		TestEqual(TEXT("Create maximum count"), PersistedCreatePayload.MaximumLoadableAmmoCount, 24);
	}
	else
	{
		AddError(PhaseError);
		return false;
	}

	// Create baseline에 binding된 exact8 Update JSON입니다.
	const FString UpdateJson = BuildAmmoJson(
		Paths,
		&CreateFingerprint,
		TEXT("DAO P0-03 수정 탄약"),
		TEXT("AutomationFamily"),
		TEXT("3.75"),
		TEXT("[\"HE\",\"Practice\",\"AP\"]"),
		IconJson,
		TEXT("18"),
		false);
	if (!TestTrue(TEXT("Ammo Update Staging overwrite succeeds"), WriteStagingFile(Paths.StagingRelativePath, UpdateJson)))
	{
		return false;
	}
	// Actual provider/common Update Preview입니다.
	FCFDACommonPreviewRow UpdatePreview;
	if (!TestTrue(TEXT("Ammo Update Preview builds"), BuildCommonPreviewFromJson(UpdateJson, Paths.StagingRelativePath, UpdatePreview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Ammo Update Preview is Update"), UpdatePreview.Kind, ECFDAStagingPreviewKind::Update);
	// Exact Update Reviewed approval입니다.
	FCFDAStagingReviewedApproval UpdateApproval;
	if (!TestTrue(TEXT("Ammo Update approval builds"), BuildSingleApproval(UpdatePreview, UpdateApproval, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Exact Update durable Apply report입니다.
	FCFDAStagingApplyReport UpdateReport;
	TestTrue(TEXT("Ammo durable Update succeeds"), FCFDAStagingApplyService::ApplyReviewedBatch(UpdateApproval, UpdateReport));
	TestEqual(TEXT("Ammo durable Update result"), UpdateReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("Ammo durable Update count"), UpdateReport.DurableAppliedCount, 1);

	// Persisted Update exact8 payload입니다.
	FCFDAAmmoPayload PersistedUpdatePayload;
	// Persisted Update semantic fingerprint입니다.
	FString PersistedUpdateFingerprint;
	if (TestTrue(
		TEXT("Ammo durable Update persisted readback succeeds"),
		ReadPersistedAmmo(Paths.TargetObjectPath, PersistedUpdatePayload, PersistedUpdateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Update persisted fingerprint equals reviewed staging"), PersistedUpdateFingerprint, UpdatePreview.Envelope.StagingSemanticFingerprint);
		TestFalse(TEXT("Update persisted package remains clean"), bPersistedDirty);
		TestEqual(TEXT("Update tags count"), PersistedUpdatePayload.AmmoTags.Num(), 3);
		TestEqual(TEXT("Update non-null AmmoIcon path"), PersistedUpdatePayload.AmmoIcon.ToString(), FixtureIconPath);
		TestEqual(TEXT("Update authored mass"), PersistedUpdatePayload.UnitMassKg, 3.75f);
		TestEqual(TEXT("Update maximum count"), PersistedUpdatePayload.MaximumLoadableAmmoCount, 18);
		TestFalse(TEXT("Update resupply authored bool"), PersistedUpdatePayload.bCanBeResupplied);
	}
	else
	{
		AddError(PhaseError);
	}
	return !HasAnyErrors();
}

// Actual Ammo path에서 reviewed Staging stale와 pre-existing dirty target을 mutation0으로 차단하는지 검증합니다.
bool FCFDAAmmoStaleDirtyGuardsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAAmmoApplyTestsPrivate;

	// Test 시작 전 stale disposable residue 제거 상세입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("DAO-P0-03 guard pre-clean leaves residue 0"), CleanupFixtureRoot(CleanupError)))
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

	// Stale approval 검증용 unique path입니다.
	const FFixturePaths StalePaths = BuildFixturePaths(TEXT("DAOP03Stale"));
	// Initial Create JSON입니다.
	const FString OriginalJson = BuildAmmoJson(
		StalePaths,
		nullptr,
		TEXT("Stale Original"),
		TEXT("GuardFamily"),
		TEXT("1.5"),
		TEXT("[]"),
		TEXT("null"),
		TEXT("10"),
		true);
	TestTrue(TEXT("Stale fixture staging write"), WriteStagingFile(StalePaths.StagingRelativePath, OriginalJson));
	// Reviewed 전 original Preview입니다.
	FCFDACommonPreviewRow OriginalPreview;
	// Current guard phase error입니다.
	FString PhaseError;
	if (!BuildCommonPreviewFromJson(OriginalJson, StalePaths.StagingRelativePath, OriginalPreview, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Original exact approval입니다.
	FCFDAStagingReviewedApproval StaleApproval;
	if (!BuildSingleApproval(OriginalPreview, StaleApproval, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Review 뒤 disk Staging을 변경한 stale JSON입니다.
	const FString ChangedAfterReviewJson = BuildAmmoJson(
		StalePaths,
		nullptr,
		TEXT("Stale Changed"),
		TEXT("GuardFamily"),
		TEXT("1.5"),
		TEXT("[]"),
		TEXT("null"),
		TEXT("11"),
		true);
	TestTrue(TEXT("Stale fixture post-review rewrite"), WriteStagingFile(StalePaths.StagingRelativePath, ChangedAfterReviewJson));
	// Stale approval apply report입니다.
	FCFDAStagingApplyReport StaleReport;
	TestFalse(TEXT("Changed post-review Ammo Staging must block Apply"), FCFDAStagingApplyService::ApplyReviewedBatch(StaleApproval, StaleReport));
	TestEqual(TEXT("Stale Ammo approval batch is blocked before mutation"), StaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestFalse(TEXT("Stale Ammo target remains absent"), FPackageName::DoesPackageExist(StalePaths.PackageName));

	// Dirty target 검증용 별도 unique path입니다.
	const FFixturePaths DirtyPaths = BuildFixturePaths(TEXT("DAOP03Dirty"));
	// Dirty fixture initial Create JSON입니다.
	const FString DirtyCreateJson = BuildAmmoJson(
		DirtyPaths,
		nullptr,
		TEXT("Dirty Baseline"),
		TEXT("GuardFamily"),
		TEXT("2.0"),
		TEXT("[\"Guard\"]"),
		TEXT("null"),
		TEXT("20"),
		true);
	// Durable baseline fingerprint입니다.
	FString DirtyBaselineFingerprint;
	if (!TestTrue(TEXT("Dirty guard baseline Create succeeds"), CreateDurableFixture(*this, DirtyPaths, DirtyCreateJson, DirtyBaselineFingerprint, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Clean persisted Ammo target입니다.
	UCFAmmoData* DirtyAsset = LoadObject<UCFAmmoData>(nullptr, *DirtyPaths.TargetObjectPath);
	// Dirty guard package입니다.
	UPackage* DirtyPackage = DirtyAsset != nullptr ? DirtyAsset->GetOutermost() : nullptr;
	if (!TestNotNull(TEXT("Dirty guard Ammo target exists"), DirtyAsset)
		|| !TestNotNull(TEXT("Dirty guard package exists"), DirtyPackage))
	{
		return false;
	}
	DirtyPackage->SetDirtyFlag(true);

	// Existing baseline에 binding된 Update JSON입니다.
	const FString DirtyUpdateJson = BuildAmmoJson(
		DirtyPaths,
		&DirtyBaselineFingerprint,
		TEXT("Dirty Desired"),
		TEXT("GuardFamily"),
		TEXT("2.25"),
		TEXT("[\"Guard\"]"),
		TEXT("null"),
		TEXT("19"),
		true);
	// Dirty target current truth를 포함한 Preview입니다.
	FCFDACommonPreviewRow DirtyPreview;
	if (TestTrue(TEXT("Dirty Ammo Preview builds"), BuildCommonPreviewFromJson(DirtyUpdateJson, DirtyPaths.StagingRelativePath, DirtyPreview, PhaseError)))
	{
		TestEqual(TEXT("Pre-existing dirty Ammo target classifies Conflict"), DirtyPreview.Kind, ECFDAStagingPreviewKind::Conflict);
		TestTrue(TEXT("Dirty Ammo target emits TargetDirtyUnowned"), FCFDAStagingService::HasIssueCode(DirtyPreview.Issues, ECFDAStagingIssueCode::TargetDirtyUnowned));
	}
	else
	{
		AddError(PhaseError);
	}
	DirtyPackage->SetDirtyFlag(false);
	return !HasAnyErrors();
}

// Actual Ammo provider Create path에서 SavePackage outcome uncertainty가 durable success로 오인되지 않는지 검증합니다.
bool FCFDAAmmoSaveUncertaintyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAAmmoApplyTestsPrivate;

	// Test 시작 전 stale disposable residue 제거 상세입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("DAO-P0-03 uncertainty pre-clean leaves residue 0"), CleanupFixtureRoot(CleanupError)))
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

	// Save uncertainty unique fixture path입니다.
	const FFixturePaths Paths = BuildFixturePaths(TEXT("DAOP03Save"));
	// Save uncertainty Create JSON입니다.
	const FString JsonText = BuildAmmoJson(
		Paths,
		nullptr,
		TEXT("Forced Save Uncertainty"),
		TEXT("FailureFamily"),
		TEXT("4.0"),
		TEXT("[\"Failure\"]"),
		TEXT("null"),
		TEXT("8"),
		false);
	TestTrue(TEXT("Save uncertainty staging write"), WriteStagingFile(Paths.StagingRelativePath, JsonText));
	// Actual Ammo Create Preview입니다.
	FCFDACommonPreviewRow PreviewRow;
	// Current phase error입니다.
	FString PhaseError;
	if (!BuildCommonPreviewFromJson(JsonText, Paths.StagingRelativePath, PreviewRow, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	// Exact Reviewed approval입니다.
	FCFDAStagingReviewedApproval Approval;
	if (!BuildSingleApproval(PreviewRow, Approval, PhaseError))
	{
		AddError(PhaseError);
		return false;
	}
	FCFDAStagingApplyTestControl::ForceSaveFailure(Paths.TargetObjectPath);
	// Forced SavePackage outcome uncertainty report입니다.
	FCFDAStagingApplyReport ApplyReport;
	TestFalse(TEXT("Forced Ammo SavePackage uncertainty must not report success"), FCFDAStagingApplyService::ApplyReviewedBatch(Approval, ApplyReport));
	TestEqual(TEXT("Forced Ammo SavePackage uncertainty batch result"), ApplyReport.Result, ECFDAStagingBatchApplyResult::SaveStateUnconfirmed);
	TestEqual(TEXT("Forced Ammo SavePackage uncertainty durable count"), ApplyReport.DurableAppliedCount, 0);
	if (!ApplyReport.Targets.IsEmpty())
	{
		TestEqual(TEXT("Forced Ammo SavePackage uncertainty target result"), ApplyReport.Targets[0].Result, ECFDAStagingTargetApplyResult::SaveStateUnconfirmed);
	}
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
