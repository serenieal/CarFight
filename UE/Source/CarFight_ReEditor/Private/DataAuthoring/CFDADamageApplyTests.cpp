// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageApplyTests.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-052 DDO-P0-02 Damage Reviewed Apply / durable exact12 / TOCTOU focused Automation입니다.
// Changelog:
// - v1.0.0: disposable Damage root에서 exact12 Create→Update durable round-trip과 source/current stale mutation0 guard를 actual provider path로 검증합니다.
// Migration:
// - 실제 Save/Delete는 /Game/Test/CarFight/DDODamageP02 와 DamageData/__AutomationP02__ exact test-owned root에만 한정합니다.
// - Product DA_DamageAsset/DA_DamageArmorPenTest, Product canonical Damage Staging, DACE history와 mixed operational admission은 읽기/쓰기 대상으로 사용하지 않습니다.

#include "CFDADamageProvider.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingApply.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFDamageData.h"
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

namespace CFDADamageApplyTestsPrivate
{
	// DDO-P0-02 disposable Content package root입니다.
	static const FString FixturePackageRoot = TEXT("/Game/Test/CarFight/DDODamageP02");

	// DDO-P0-02 disposable Staging source root입니다.
	static const FString FixtureStagingRoot = TEXT("Authoring/DataAssetStaging/DamageData/__AutomationP02__");

	// 한 disposable Damage fixture의 exact identity/path 묶음입니다.
	struct FFixturePaths
	{
		// DamageId와 StableLogicalId로 사용할 unique identity입니다.
		FString StableLogicalId;

		// Package 안의 exact UObject name입니다.
		FString AssetName;

		// Disposable /Game package long name입니다.
		FString PackageName;

		// Exact UObject path입니다.
		FString TargetObjectPath;

		// Provider-owned main_game-relative Staging JSON path입니다.
		FString StagingRelativePath;
	};

	// GUID suffix를 사용해 collision 없는 disposable Damage fixture path를 만듭니다.
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

	// Strict Damage exact12 whole-record JSON을 구성합니다.
	FString BuildDamageJson(
		const FFixturePaths& Paths,
		const FString* BaseSemanticFingerprint,
		const FString& DamageTypeToken,
		const FString& BaseDamageText,
		const bool bCanDamageSelf,
		const FString& ArmorPenetrationText,
		const bool bUseRadialDamage,
		const FString& ExplosionRadiusText,
		const FString& ExplosionInnerRadiusText,
		const FString& ExplosionDamageText,
		const FString& MinExplosionDamageScaleText,
		const FString& ModuleDamageScaleText,
		const FString& ImpulseStrengthText)
	{
		// BaseSemanticFingerprint의 exact JSON representation입니다.
		const FString BaseJson = BaseSemanticFingerprint != nullptr
			? FString::Printf(TEXT("\"%s\""), **BaseSemanticFingerprint)
			: TEXT("null");
		// Self-damage bool JSON token입니다.
		const TCHAR* CanDamageSelfJson = bCanDamageSelf ? TEXT("true") : TEXT("false");
		// Radial enable bool JSON token입니다.
		const TCHAR* UseRadialDamageJson = bUseRadialDamage ? TEXT("true") : TEXT("false");
		return FString::Printf(
			TEXT("{")
			TEXT("\"SchemaId\":\"CarFight.DataAsset.DamageData\",")
			TEXT("\"SchemaRevision\":1,")
			TEXT("\"AdapterContractRevision\":1,")
			TEXT("\"DataAssetTypeClassPath\":\"/Script/CarFight_Re.CFDamageData\",")
			TEXT("\"StableLogicalId\":\"%s\",")
			TEXT("\"TargetObjectPath\":\"%s\",")
			TEXT("\"BaseSemanticFingerprint\":%s,")
			TEXT("\"Payload\":{")
			TEXT("\"DamageId\":\"%s\",")
			TEXT("\"DamageType\":\"%s\",")
			TEXT("\"BaseDamage\":%s,")
			TEXT("\"bCanDamageSelf\":%s,")
			TEXT("\"ArmorPenetration\":%s,")
			TEXT("\"bUseRadialDamage\":%s,")
			TEXT("\"ExplosionRadius\":%s,")
			TEXT("\"ExplosionInnerRadius\":%s,")
			TEXT("\"ExplosionDamage\":%s,")
			TEXT("\"MinExplosionDamageScale\":%s,")
			TEXT("\"ModuleDamageScale\":%s,")
			TEXT("\"ImpulseStrength\":%s")
			TEXT("}")
			TEXT("}"),
			*Paths.StableLogicalId,
			*Paths.TargetObjectPath,
			*BaseJson,
			*Paths.StableLogicalId,
			*DamageTypeToken,
			*BaseDamageText,
			CanDamageSelfJson,
			*ArmorPenetrationText,
			UseRadialDamageJson,
			*ExplosionRadiusText,
			*ExplosionInnerRadiusText,
			*ExplosionDamageText,
			*MinExplosionDamageScaleText,
			*ModuleDamageScaleText,
			*ImpulseStrengthText);
	}

	// Actual Damage provider parse/current resolver/shared Preview를 한 번에 실행합니다.
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
		if (!CFDADamageProviderImpl::ParseCommonCandidate(JsonText, StagingRelativePath, Envelope, ParseIssues))
		{
			OutError = ParseIssues.IsEmpty() ? TEXT("Damage ParseCommonCandidate가 실패했습니다.") : ParseIssues[0].Message;
			return false;
		}

		// Exact current CFDamageData target/identity truth입니다.
		FCFDACommonCurrentState CurrentState;
		// Current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!CFDADamageProviderImpl::ResolveCommonCurrentState(Envelope, CurrentState, CurrentIssues))
		{
			OutError = CurrentIssues.IsEmpty() ? TEXT("Damage current-state resolver가 실패했습니다.") : CurrentIssues[0].Message;
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

	// Persisted Damage fixture의 exact typed payload와 semantic fingerprint를 읽습니다.
	bool ReadPersistedDamage(
		const FString& TargetObjectPath,
		FCFDADamagePayload& OutPayload,
		FString& OutFingerprint,
		bool& bOutPackageDirty,
		FString& OutError)
	{
		// Exact persisted CFDamageData입니다.
		UCFDamageData* Asset = LoadObject<UCFDamageData>(nullptr, *TargetObjectPath);
		if (Asset == nullptr)
		{
			OutError = FString::Printf(TEXT("Persisted Damage fixture를 로드하지 못했습니다: %s"), *TargetObjectPath);
			return false;
		}
		// Exact owning package입니다.
		UPackage* Package = Asset->GetOutermost();
		bOutPackageDirty = Package != nullptr && Package->IsDirty();
		// Production extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!CFDADamageProviderImpl::ExtractPayload(*Asset, OutPayload, ExtractIssues))
		{
			OutError = ExtractIssues.IsEmpty() ? TEXT("Persisted Damage payload 추출에 실패했습니다.") : ExtractIssues[0].Message;
			return false;
		}
		return CFDADamageProviderImpl::BuildSemanticFingerprint(OutPayload, OutFingerprint, OutError);
	}

	// Object path가 DDO-P0-02 disposable package root에 속하는지 확인합니다.
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
			OutError = FString::Printf(TEXT("DDO-P0-02 Damage AssetRegistry residue가 %d개 남았습니다."), RegistryAssets.Num());
			return false;
		}

		for (TObjectIterator<UCFDamageData> DamageIterator; DamageIterator; ++DamageIterator)
		{
			// Current process의 loaded UCFDamageData 후보입니다.
			const UCFDamageData* LoadedDamage = *DamageIterator;
			if (LoadedDamage == nullptr || LoadedDamage->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded Damage exact object path입니다.
			const FString LoadedPath = FSoftObjectPath(LoadedDamage).ToString();
			if (IsFixtureObjectPath(LoadedPath))
			{
				OutError = FString::Printf(TEXT("DDO-P0-02 Damage loaded UObject residue가 남았습니다: %s"), *LoadedPath);
				return false;
			}
		}

		// Disposable Content physical directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DDODamageP02")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = TEXT("DDO-P0-02 Damage physical Content root residue가 남았습니다.");
			return false;
		}

		// Disposable Staging physical directory입니다.
		const FString StagingDirectory = GetStagingAbsolutePath(FixtureStagingRoot);
		if (IFileManager::Get().DirectoryExists(*StagingDirectory))
		{
			OutError = TEXT("DDO-P0-02 Damage physical Staging root residue가 남았습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// DDO-P0-02 exact disposable Content/Staging roots만 unload→GC→disk delete→registry refresh 순서로 정리합니다.
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

		for (TObjectIterator<UCFDamageData> DamageIterator; DamageIterator; ++DamageIterator)
		{
			// Registry에 없더라도 resolver가 볼 수 있는 loaded fixture입니다.
			UCFDamageData* LoadedDamage = *DamageIterator;
			if (LoadedDamage == nullptr || LoadedDamage->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded exact object path입니다.
			const FString LoadedPath = FSoftObjectPath(LoadedDamage).ToString();
			if (!IsFixtureObjectPath(LoadedPath))
			{
				continue;
			}
			// Loaded fixture owning package입니다.
			UPackage* LoadedPackage = LoadedDamage->GetOutermost();
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
			LoadedDamage->ClearFlags(RF_Standalone);
		}

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("DDO-P0-02 Damage fixture package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Disposable physical Content root입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Test/CarFight/DDODamageP02")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("DDO-P0-02 Damage Content root 삭제에 실패했습니다: %s"), *ContentDirectory);
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
			OutError = FString::Printf(TEXT("DDO-P0-02 Damage Staging root 삭제에 실패했습니다: %s"), *StagingDirectory);
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
			OutError = TEXT("Damage fixture Staging write에 실패했습니다.");
			return false;
		}
		// Actual provider/common Create Preview입니다.
		FCFDACommonPreviewRow PreviewRow;
		if (!BuildCommonPreviewFromJson(JsonText, Paths.StagingRelativePath, PreviewRow, OutError))
		{
			return false;
		}
		Test.TestEqual(TEXT("Damage disposable fixture Preview must be Create"), PreviewRow.Kind, ECFDAStagingPreviewKind::Create);
		if (PreviewRow.Kind != ECFDAStagingPreviewKind::Create)
		{
			OutError = TEXT("Damage disposable fixture가 Create로 분류되지 않았습니다.");
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
			OutError = ApplyReport.Diagnostic.IsEmpty() ? TEXT("Damage Create durable Apply가 실패했습니다.") : ApplyReport.Diagnostic;
			return false;
		}
		OutError.Reset();
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageDurableRoundTripTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_02.DurableRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageStaleGuardsTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_02.StaleGuards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Actual Damage provider에서 exact12 Create→Update durable round-trip과 radial false value preservation을 검증합니다.
bool FCFDADamageDurableRoundTripTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDADamageApplyTestsPrivate;

	// Test 시작 전 stale disposable residue 제거 상세입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("DDO-P0-02 durable pre-clean leaves residue 0"), CleanupFixtureRoot(CleanupError)))
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

	// Unique durable Damage fixture path입니다.
	const FFixturePaths Paths = BuildFixturePaths(TEXT("DDOP02Dur"));
	// Initial Create exact12 JSON입니다.
	const FString CreateJson = BuildDamageJson(
		Paths,
		nullptr,
		TEXT("Explosive"),
		TEXT("35.0"),
		false,
		TEXT("12.0"),
		true,
		TEXT("240.0"),
		TEXT("300.0"),
		TEXT("90.0"),
		TEXT("0.25"),
		TEXT("1.2"),
		TEXT("650.0"));
	// Reviewed Create fingerprint입니다.
	FString CreateFingerprint;
	// Current durable phase error입니다.
	FString PhaseError;
	if (!TestTrue(TEXT("Damage durable Create succeeds"), CreateDurableFixture(*this, Paths, CreateJson, CreateFingerprint, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Persisted Create exact12 payload입니다.
	FCFDADamagePayload PersistedCreatePayload;
	// Persisted Create semantic fingerprint입니다.
	FString PersistedCreateFingerprint;
	// Persisted Create package dirty state입니다.
	bool bPersistedDirty = true;
	if (TestTrue(
		TEXT("Damage durable Create persisted readback succeeds"),
		ReadPersistedDamage(Paths.TargetObjectPath, PersistedCreatePayload, PersistedCreateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Create persisted fingerprint equals reviewed staging"), PersistedCreateFingerprint, CreateFingerprint);
		TestFalse(TEXT("Create persisted package remains clean"), bPersistedDirty);
		TestEqual(TEXT("Create DamageType"), PersistedCreatePayload.DamageType, ECFDamageType::Explosive);
		TestEqual(TEXT("Create BaseDamage"), PersistedCreatePayload.BaseDamage, 35.0f);
		TestEqual(TEXT("Create ArmorPenetration"), PersistedCreatePayload.ArmorPenetration, 12.0f);
		TestTrue(TEXT("Create radial authored flag"), PersistedCreatePayload.bUseRadialDamage);
		TestEqual(TEXT("Create ExplosionRadius"), PersistedCreatePayload.ExplosionRadius, 240.0f);
		TestEqual(TEXT("Create ExplosionInnerRadius"), PersistedCreatePayload.ExplosionInnerRadius, 300.0f);
		TestTrue(TEXT("Create preserves allowed InnerRadius > Radius"), PersistedCreatePayload.ExplosionInnerRadius > PersistedCreatePayload.ExplosionRadius);
		TestEqual(TEXT("Create ExplosionDamage"), PersistedCreatePayload.ExplosionDamage, 90.0f);
		TestEqual(TEXT("Create MinExplosionDamageScale"), PersistedCreatePayload.MinExplosionDamageScale, 0.25f);
		TestEqual(TEXT("Create ModuleDamageScale"), PersistedCreatePayload.ModuleDamageScale, 1.2f);
		TestEqual(TEXT("Create ImpulseStrength"), PersistedCreatePayload.ImpulseStrength, 650.0f);
	}
	else
	{
		AddError(PhaseError);
		return false;
	}

	// Create baseline에 binding된 exact12 Update JSON입니다.
	const FString UpdateJson = BuildDamageJson(
		Paths,
		&CreateFingerprint,
		TEXT("Energy"),
		TEXT("42.5"),
		true,
		TEXT("18.0"),
		false,
		TEXT("125.0"),
		TEXT("300.0"),
		TEXT("75.0"),
		TEXT("0.4"),
		TEXT("0.8"),
		TEXT("720.0"));
	if (!TestTrue(TEXT("Damage Update Staging overwrite succeeds"), WriteStagingFile(Paths.StagingRelativePath, UpdateJson)))
	{
		return false;
	}
	// Actual provider/common Update Preview입니다.
	FCFDACommonPreviewRow UpdatePreview;
	if (!TestTrue(TEXT("Damage Update Preview builds"), BuildCommonPreviewFromJson(UpdateJson, Paths.StagingRelativePath, UpdatePreview, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	TestEqual(TEXT("Damage Update Preview is Update"), UpdatePreview.Kind, ECFDAStagingPreviewKind::Update);
	// Exact Update Reviewed approval입니다.
	FCFDAStagingReviewedApproval UpdateApproval;
	if (!TestTrue(TEXT("Damage Update approval builds"), BuildSingleApproval(UpdatePreview, UpdateApproval, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}
	// Exact Update durable Apply report입니다.
	FCFDAStagingApplyReport UpdateReport;
	TestTrue(TEXT("Damage durable Update succeeds"), FCFDAStagingApplyService::ApplyReviewedBatch(UpdateApproval, UpdateReport));
	TestEqual(TEXT("Damage durable Update result"), UpdateReport.Result, ECFDAStagingBatchApplyResult::DurableApplied);
	TestEqual(TEXT("Damage durable Update count"), UpdateReport.DurableAppliedCount, 1);

	// Persisted Update exact12 payload입니다.
	FCFDADamagePayload PersistedUpdatePayload;
	// Persisted Update semantic fingerprint입니다.
	FString PersistedUpdateFingerprint;
	if (TestTrue(
		TEXT("Damage durable Update persisted readback succeeds"),
		ReadPersistedDamage(Paths.TargetObjectPath, PersistedUpdatePayload, PersistedUpdateFingerprint, bPersistedDirty, PhaseError)))
	{
		TestEqual(TEXT("Update persisted fingerprint equals reviewed staging"), PersistedUpdateFingerprint, UpdatePreview.Envelope.StagingSemanticFingerprint);
		TestFalse(TEXT("Update persisted package remains clean"), bPersistedDirty);
		TestEqual(TEXT("Update DamageType"), PersistedUpdatePayload.DamageType, ECFDamageType::Energy);
		TestEqual(TEXT("Update BaseDamage"), PersistedUpdatePayload.BaseDamage, 42.5f);
		TestTrue(TEXT("Update self-damage authored bool"), PersistedUpdatePayload.bCanDamageSelf);
		TestEqual(TEXT("Update ArmorPenetration"), PersistedUpdatePayload.ArmorPenetration, 18.0f);
		TestFalse(TEXT("Update radial authored bool"), PersistedUpdatePayload.bUseRadialDamage);
		TestEqual(TEXT("Radial false preserves nonzero ExplosionRadius"), PersistedUpdatePayload.ExplosionRadius, 125.0f);
		TestEqual(TEXT("Radial false preserves nonzero ExplosionInnerRadius"), PersistedUpdatePayload.ExplosionInnerRadius, 300.0f);
		TestEqual(TEXT("Radial false preserves nonzero ExplosionDamage"), PersistedUpdatePayload.ExplosionDamage, 75.0f);
		TestEqual(TEXT("Update MinExplosionDamageScale"), PersistedUpdatePayload.MinExplosionDamageScale, 0.4f);
		TestEqual(TEXT("Update ModuleDamageScale"), PersistedUpdatePayload.ModuleDamageScale, 0.8f);
		TestEqual(TEXT("Update ImpulseStrength"), PersistedUpdatePayload.ImpulseStrength, 720.0f);
	}
	else
	{
		AddError(PhaseError);
	}
	return !HasAnyErrors();
}

// Actual Damage provider path에서 reviewed source stale와 current semantic stale를 mutation0으로 차단하는지 검증합니다.
bool FCFDADamageStaleGuardsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDADamageApplyTestsPrivate;

	// Test 시작 전 stale disposable residue 제거 상세입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("DDO-P0-02 stale pre-clean leaves residue 0"), CleanupFixtureRoot(CleanupError)))
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
	const FFixturePaths SourceStalePaths = BuildFixturePaths(TEXT("DDOP02Src"));
	// Review 전 original Create JSON입니다.
	const FString OriginalSourceJson = BuildDamageJson(
		SourceStalePaths,
		nullptr,
		TEXT("Kinetic"),
		TEXT("30.0"),
		false,
		TEXT("5.0"),
		false,
		TEXT("0.0"),
		TEXT("0.0"),
		TEXT("0.0"),
		TEXT("0.0"),
		TEXT("1.0"),
		TEXT("100.0"));
	TestTrue(TEXT("Source-stale fixture staging write"), WriteStagingFile(SourceStalePaths.StagingRelativePath, OriginalSourceJson));
	// Reviewed 전 original Preview입니다.
	FCFDACommonPreviewRow OriginalSourcePreview;
	// Current stale guard phase error입니다.
	FString PhaseError;
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
	// Review 뒤 disk Staging semantic을 바꾼 JSON입니다.
	const FString ChangedAfterReviewJson = BuildDamageJson(
		SourceStalePaths,
		nullptr,
		TEXT("Kinetic"),
		TEXT("31.0"),
		false,
		TEXT("5.0"),
		false,
		TEXT("0.0"),
		TEXT("0.0"),
		TEXT("0.0"),
		TEXT("0.0"),
		TEXT("1.0"),
		TEXT("100.0"));
	TestTrue(TEXT("Source-stale post-review rewrite"), WriteStagingFile(SourceStalePaths.StagingRelativePath, ChangedAfterReviewJson));
	// Source-stale apply report입니다.
	FCFDAStagingApplyReport SourceStaleReport;
	TestFalse(TEXT("Changed post-review Damage Staging must block Apply"), FCFDAStagingApplyService::ApplyReviewedBatch(SourceStaleApproval, SourceStaleReport));
	TestEqual(TEXT("Source-stale Damage batch is blocked before mutation"), SourceStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Source-stale Damage durable count remains0"), SourceStaleReport.DurableAppliedCount, 0);
	TestFalse(TEXT("Source-stale Damage target remains absent"), FPackageName::DoesPackageExist(SourceStalePaths.PackageName));

	// Current stale 검증용 별도 unique path입니다.
	const FFixturePaths CurrentStalePaths = BuildFixturePaths(TEXT("DDOP02Cur"));
	// Current-stale baseline Create JSON입니다.
	const FString CurrentCreateJson = BuildDamageJson(
		CurrentStalePaths,
		nullptr,
		TEXT("Kinetic"),
		TEXT("28.0"),
		false,
		TEXT("10.0"),
		false,
		TEXT("40.0"),
		TEXT("20.0"),
		TEXT("15.0"),
		TEXT("0.2"),
		TEXT("1.0"),
		TEXT("250.0"));
	// Durable current baseline fingerprint입니다.
	FString CurrentBaselineFingerprint;
	if (!TestTrue(TEXT("Current-stale baseline Create succeeds"), CreateDurableFixture(*this, CurrentStalePaths, CurrentCreateJson, CurrentBaselineFingerprint, PhaseError)))
	{
		AddError(PhaseError);
		return false;
	}

	// Existing baseline에 binding된 Update JSON입니다.
	const FString CurrentUpdateJson = BuildDamageJson(
		CurrentStalePaths,
		&CurrentBaselineFingerprint,
		TEXT("Energy"),
		TEXT("45.0"),
		true,
		TEXT("20.0"),
		false,
		TEXT("90.0"),
		TEXT("30.0"),
		TEXT("25.0"),
		TEXT("0.3"),
		TEXT("0.9"),
		TEXT("400.0"));
	TestTrue(TEXT("Current-stale Update staging write"), WriteStagingFile(CurrentStalePaths.StagingRelativePath, CurrentUpdateJson));
	// Review 시점의 clean current baseline을 반영한 Update Preview입니다.
	FCFDACommonPreviewRow CurrentUpdatePreview;
	if (!TestTrue(TEXT("Current-stale Update Preview builds"), BuildCommonPreviewFromJson(CurrentUpdateJson, CurrentStalePaths.StagingRelativePath, CurrentUpdatePreview, PhaseError)))
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

	// Review 뒤 process-local current semantic을 변경할 disposable Damage asset입니다.
	UCFDamageData* CurrentStaleAsset = LoadObject<UCFDamageData>(nullptr, *CurrentStalePaths.TargetObjectPath);
	// Current-stale disposable owning package입니다.
	UPackage* CurrentStalePackage = CurrentStaleAsset != nullptr ? CurrentStaleAsset->GetOutermost() : nullptr;
	if (!TestNotNull(TEXT("Current-stale Damage target exists"), CurrentStaleAsset)
		|| !TestNotNull(TEXT("Current-stale Damage package exists"), CurrentStalePackage))
	{
		return false;
	}
	// Review 이후 만든 exact current semantic drift 값입니다.
	const float DriftedArmorPenetration = CurrentStaleAsset->ArmorPenetration + 7.0f;
	CurrentStaleAsset->ArmorPenetration = DriftedArmorPenetration;
	CurrentStalePackage->SetDirtyFlag(false);

	// Current-stale apply report입니다.
	FCFDAStagingApplyReport CurrentStaleReport;
	TestFalse(TEXT("Changed post-review Damage current semantic must block Apply"), FCFDAStagingApplyService::ApplyReviewedBatch(CurrentStaleApproval, CurrentStaleReport));
	TestEqual(TEXT("Current-stale Damage batch is blocked before mutation"), CurrentStaleReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Current-stale Damage durable count remains0"), CurrentStaleReport.DurableAppliedCount, 0);
	TestEqual(TEXT("Current-stale mutation0 preserves drifted current value"), CurrentStaleAsset->ArmorPenetration, DriftedArmorPenetration);
	TestFalse(TEXT("Current-stale disposable package stays clean after blocked Apply"), CurrentStalePackage->IsDirty());
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
