// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentDurableTests.cpp
// Version: v1.2.2
// Date: 2026-09-16
// Description: CF-FQ-054 EBA-P0-03 durable 회귀와 EBA-P0-05 end-to-end technical acceptance Automation입니다.
// Changelog:
// - v1.2.2: USER 한글화된 EquipmentId 중복 진단에 맞춰 LoadedDuplicate regression 기대값을 동기화.
// - v1.2.1: UE 5.8 FTextInspector namespace/key의 TOptional 반환 시그니처에 맞춰 HeavyCannon localization identity 회귀 fixture를 교정.
// - v1.2.0: HeavyCannon localized DisplayName preserve-only Review 성공/identity 불변 회귀와 USER UX 한글 validation 기대값을 P005에 추가.
// - v1.1.1: 첫 P005 acceptance에서 드러난 representative fixture 가정을 교정. Weapon Product source는 authored FText를 lossless 변환하지 않고 read-only 불변성만 확인하며, Scanner 신규 flow는 유효한 persisted VehicleSensorData를 직접 선택해 검증.
// - v1.1.0: representative Product Weapon/Scanner source를 read-only로 재사용한 P005 Weapon 신규, Scanner 오류→해결, fresh existing→update, residue0 acceptance를 추가.
// - v1.0.0: Revision 1 digest, Create/Update, stale one-shot, loaded-only duplicate, child state, dirty target, SaveStateUnconfirmed/retry0와 residue0를 추가.
// Migration:
// - 실제 EquipmentPreset mutation은 `/Game/CarFight/Tests/EquipmentAuthoring/` disposable namespace에만 한정합니다.
// - P005 representative Product EquipmentPreset과 모든 child DataAsset은 read-only source이며 Product write/save는 수행하지 않습니다.
// - USER Editor UX PASS는 Automation으로 대체하지 않으며 별도 P0-05 USER 확인으로 남깁니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "EquipmentAuthoring/CFEquipmentBuilderVM.h"
#include "EquipmentAuthoring/CFEquipmentDurable.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CFEquipmentPresetData.h"
#include "CFTurretMountData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFDADurableCore.h"
#include "HAL/FileManager.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

namespace CFEquipmentDurableTestsPrivate
{
	// EBA-P0-03 exact disposable package root입니다.
	static const FString FixturePackageRoot = TEXT("/Game/CarFight/Tests/EquipmentAuthoring");

	// 한 disposable EquipmentPreset fixture의 exact path/identity 묶음입니다.
	struct FFixturePaths
	{
		// EquipmentId로 사용할 unique identity입니다.
		FName EquipmentId = NAME_None;

		// package 안 exact UObject name입니다.
		FString AssetName;

		// disposable package long name입니다.
		FString PackageName;

		// exact UObject path입니다.
		FString TargetObjectPath;
	};

	// GUID suffix로 collision 없는 disposable EquipmentPreset path를 만듭니다.
	FFixturePaths BuildFixturePaths(const FString& Prefix)
	{
		// Collision 방지용 short GUID입니다.
		const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);
		// 반환할 exact fixture path 묶음입니다.
		FFixturePaths Paths;
		Paths.EquipmentId = FName(*FString::Printf(TEXT("EBA_%s_%s"), *Prefix, *Suffix));
		Paths.AssetName = FString::Printf(TEXT("EQ_%s_%s"), *Prefix, *Suffix);
		Paths.PackageName = FString::Printf(TEXT("%s/%s"), *FixturePackageRoot, *Paths.AssetName);
		Paths.TargetObjectPath = FString::Printf(TEXT("%s.%s"), *Paths.PackageName, *Paths.AssetName);
		return Paths;
	}

	// object path가 exact EBA disposable namespace에 속하는지 확인합니다.
	bool IsFixtureObjectPath(const FString& ObjectPath)
	{
		return ObjectPath.StartsWith(FixturePackageRoot + TEXT("/"), ESearchCase::CaseSensitive);
	}

	// EBA disposable root의 registry/loaded UObject/physical directory residue가 exact0인지 검증합니다.
	bool VerifyFixtureRootResidueFree(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Disposable root 아래 registry-visible assets입니다.
		TArray<FAssetData> RegistryAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), RegistryAssets, true, true);
		if (!RegistryAssets.IsEmpty())
		{
			OutError = FString::Printf(TEXT("EBA-P0-03 AssetRegistry residue가 %d개 남았습니다."), RegistryAssets.Num());
			return false;
		}

		for (TObjectIterator<UCFEquipmentPresetData> PresetIterator; PresetIterator; ++PresetIterator)
		{
			// Current process의 loaded EquipmentPreset 후보입니다.
			const UCFEquipmentPresetData* LoadedPreset = *PresetIterator;
			if (LoadedPreset == nullptr || LoadedPreset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded exact object path입니다.
			const FString LoadedPath = LoadedPreset->GetPathName();
			if (IsFixtureObjectPath(LoadedPath))
			{
				OutError = FString::Printf(TEXT("EBA-P0-03 loaded UObject residue가 남았습니다: %s"), *LoadedPath);
				return false;
			}
		}

		// Disposable physical Content directory입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("CarFight/Tests/EquipmentAuthoring")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = TEXT("EBA-P0-03 physical Content root residue가 남았습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// exact EBA disposable Content root만 unload→GC→disk delete→registry refresh 순서로 정리합니다.
	bool CleanupFixtureRoot(FString& OutError)
	{
		CFDADurableCore::ResetTestFaults();

		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Registry-visible disposable EquipmentPreset assets입니다.
		TArray<FAssetData> FixtureAssets;
		AssetRegistry.GetAssetsByPath(FName(*FixturePackageRoot), FixtureAssets, true, true);
		// Unload할 unique package 목록입니다.
		TArray<UPackage*> PackagesToUnload;
		// Disk delete 뒤 registry refresh에 사용할 exact filenames입니다.
		TArray<FString> DeletedFilenames;

		for (const FAssetData& FixtureAssetData : FixtureAssets)
		{
			// Loaded 또는 persisted disposable object입니다.
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

		for (TObjectIterator<UCFEquipmentPresetData> PresetIterator; PresetIterator; ++PresetIterator)
		{
			// Registry에 없더라도 loaded-only identity closure가 볼 수 있는 disposable preset입니다.
			UCFEquipmentPresetData* LoadedPreset = *PresetIterator;
			if (LoadedPreset == nullptr || LoadedPreset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			// Loaded exact object path입니다.
			const FString LoadedPath = LoadedPreset->GetPathName();
			if (!IsFixtureObjectPath(LoadedPath))
			{
				continue;
			}
			// Loaded fixture owning package입니다.
			UPackage* LoadedPackage = LoadedPreset->GetOutermost();
			if (LoadedPackage != nullptr)
			{
				LoadedPackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(LoadedPackage);
				// Loaded-only package도 registry refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(
					LoadedPackage->GetName(),
					FPackageName::GetAssetPackageExtension());
				DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedPreset->ClearFlags(RF_Standalone);
		}

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("EBA-P0-03 fixture package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Exact disposable physical Content root입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("CarFight/Tests/EquipmentAuthoring")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("EBA-P0-03 Content root 삭제에 실패했습니다: %s"), *ContentDirectory);
			return false;
		}
		if (!DeletedFilenames.IsEmpty())
		{
			AssetRegistry.ScanModifiedAssetFiles(DeletedFilenames);
			AssetRegistry.WaitForCompletion();
		}
		return VerifyFixtureRootResidueFree(OutError);
	}

	// Existing Product EquipmentPreset 중 strict child persisted/clean validation까지 통과하는 source Draft 하나를 read-only로 찾습니다.
	bool FindReusablePersistedSourceDraft(FCFEquipmentPresetDraft& OutDraft, FString& OutError)
	{
		// Existing Product source를 탐색할 read-only VM입니다.
		FCFEquipmentBuilderVM SourceViewModel;
		if (!SourceViewModel.Initialize(OutError))
		{
			return false;
		}

		for (const FCFEquipmentPresetListEntry& Entry : SourceViewModel.GetPresetEntries())
		{
			// Current persisted preset read-only load 진단입니다.
			FString LoadError;
			if (!SourceViewModel.LoadExistingPreset(Entry.ObjectPath, LoadError))
			{
				continue;
			}
			// Existing bounded package/native validation 결과입니다.
			const FCFEquipmentDraftValidation Validation = SourceViewModel.BuildCurrentValidation();
			if (!Validation.bPackageComplete || Validation.bHasBlockingErrors)
			{
				continue;
			}
			// Child persisted/clean truth 진단입니다.
			FString ChildError;
			if (!FCFEquipmentDurable::ValidatePersistedChildDependencies(SourceViewModel.GetDraft(), FString(), ChildError))
			{
				continue;
			}
			OutDraft = SourceViewModel.GetDraft();
			OutError.Reset();
			return true;
		}

		OutError = TEXT("EBA-P0-03에서 read-only 재사용 가능한 persisted EquipmentPreset child graph를 찾지 못했습니다.");
		return false;
	}

	// EBA-P0-05가 요구하는 exact Weapon 또는 Scanner Product source 하나를 Test namespace 밖에서 read-only로 찾습니다.
	bool FindRepresentativeProductSourceDraft(
		const ECFEquipmentDraftMode RequiredMode,
		FCFEquipmentPresetDraft& OutDraft,
		FString& OutObjectPath,
		FString& OutError)
	{
		// Representative Product source를 탐색할 read-only ViewModel입니다.
		FCFEquipmentBuilderVM SourceViewModel;
		if (!SourceViewModel.Initialize(OutError))
		{
			return false;
		}

		for (const FCFEquipmentPresetListEntry& Entry : SourceViewModel.GetPresetEntries())
		{
			if (Entry.ObjectPath.StartsWith(TEXT("/Game/CarFight/Tests/"), ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Representative candidate read-only load 진단입니다.
			FString LoadError;
			if (!SourceViewModel.LoadExistingPreset(Entry.ObjectPath, LoadError))
			{
				continue;
			}
			if (SourceViewModel.GetDraft().DraftMode != RequiredMode)
			{
				continue;
			}

			// Representative candidate의 exact Product EquipmentPreset입니다.
			UCFEquipmentPresetData* CandidatePreset = LoadObject<UCFEquipmentPresetData>(nullptr, *Entry.ObjectPath);
			// Representative candidate의 owning package입니다.
			UPackage* CandidatePackage = CandidatePreset != nullptr ? CandidatePreset->GetOutermost() : nullptr;
			if (CandidatePreset == nullptr
				|| CandidatePreset->GetClass() != UCFEquipmentPresetData::StaticClass()
				|| CandidatePackage == nullptr
				|| CandidatePackage->IsDirty()
				|| !FPackageName::DoesPackageExist(CandidatePackage->GetName()))
			{
				continue;
			}

			// Representative candidate의 bounded package/native validation 결과입니다.
			const FCFEquipmentDraftValidation Validation = SourceViewModel.BuildCurrentValidation();
			if (!Validation.bPackageComplete || Validation.bHasBlockingErrors)
			{
				continue;
			}

			// Representative candidate child graph의 persisted/clean truth 진단입니다.
			FString ChildError;
			if (!FCFEquipmentDurable::ValidatePersistedChildDependencies(SourceViewModel.GetDraft(), FString(), ChildError))
			{
				continue;
			}

			OutDraft = SourceViewModel.GetDraft();
			OutObjectPath = Entry.ObjectPath;
			OutError.Reset();
			return true;
		}

		OutError = FString::Printf(
			TEXT("EBA-P0-05 representative Product %s EquipmentPreset을 read-only로 찾지 못했습니다."),
			RequiredMode == ECFEquipmentDraftMode::Weapon ? TEXT("Weapon") : TEXT("Scanner"));
		return false;
	}

	// EBA-P0-05 Scanner 신규 flow가 선택할 persisted clean + native-valid VehicleSensorData 하나를 read-only로 찾습니다.
	bool FindRepresentativePersistedSensorPath(FSoftObjectPath& OutSensorDataPath, FString& OutError)
	{
		// Current Editor Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Exact VehicleSensorData만 `/Game/CarFight` 아래에서 찾는 read-only filter입니다.
		FARFilter Filter;
		Filter.ClassPaths.Add(UCFVehicleSensorData::StaticClass()->GetClassPathName());
		Filter.PackagePaths.Add(FName(TEXT("/Game/CarFight")));
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = false;
		// Persisted VehicleSensorData metadata 후보입니다.
		TArray<FAssetData> SensorAssets;
		AssetRegistry.GetAssets(Filter, SensorAssets);

		// 각 persisted SensorData 후보를 exact class/persisted-clean/native contract 순서로 확인합니다.
		for (const FAssetData& SensorAssetData : SensorAssets)
		{
			// Current candidate exact VehicleSensorData입니다.
			UCFVehicleSensorData* SensorData = Cast<UCFVehicleSensorData>(SensorAssetData.GetAsset());
			if (SensorData == nullptr || SensorData->GetClass() != UCFVehicleSensorData::StaticClass())
			{
				continue;
			}
			// Current candidate owning package입니다.
			UPackage* SensorPackage = SensorData->GetOutermost();
			if (SensorPackage == nullptr
				|| SensorPackage->IsDirty()
				|| !FPackageName::DoesPackageExist(SensorPackage->GetName())
				|| !SensorData->IsSensorDataContractValid())
			{
				continue;
			}

			OutSensorDataPath = SensorAssetData.GetSoftObjectPath();
			OutError.Reset();
			return true;
		}

		OutError = TEXT("EBA-P0-05 representative persisted clean/native-valid VehicleSensorData를 찾지 못했습니다.");
		return false;
	}

	// Validation 결과에 지정 severity/title/message 조합이 존재하는지 확인합니다.
	bool HasValidationDiagnostic(
		const FCFEquipmentDraftValidation& Validation,
		const ECFEquipmentValidationSeverity Severity,
		const FString& Title,
		const FString& MessageFragment)
	{
		// 현재 validation에서 severity/title/message를 비교할 한 항목입니다.
		for (const FCFEquipmentValidationItem& Item : Validation.Items)
		{
			if (Item.Severity == Severity
				&& Item.Title == Title
				&& Item.Message.Contains(MessageFragment))
			{
				return true;
			}
		}
		return false;
	}

	// persisted source child graph를 재사용해 disposable 신규 Draft를 구성합니다.
	bool ConfigureNewDisposableDraft(
		FCFEquipmentBuilderVM& ViewModel,
		const FCFEquipmentPresetDraft& SourceDraft,
		const FFixturePaths& Paths,
		FString& OutError)
	{
		ViewModel.BeginNewPreset();
		ViewModel.EnableDisposableTestTargetsForAutomation(true);
		ViewModel.SetCreateTargetObjectPath(Paths.TargetObjectPath);
		ViewModel.SetEquipmentId(Paths.EquipmentId);
		ViewModel.SetDisplayName(FText::FromString(FString::Printf(TEXT("EBA P003 %s"), *Paths.EquipmentId.ToString())));
		ViewModel.SetDraftMode(SourceDraft.DraftMode);
		ViewModel.SetRequiredMountType(SourceDraft.RequiredMountType);
		ViewModel.SetRequiredWeaponSize(SourceDraft.RequiredWeaponSize);

		if (SourceDraft.DraftMode == ECFEquipmentDraftMode::Scanner)
		{
			return ViewModel.SetSensorDataPath(SourceDraft.DefaultSensorData.ToSoftObjectPath(), OutError);
		}
		if (!ViewModel.SetTurretMountDataPath(SourceDraft.DefaultTurretMountData.ToSoftObjectPath(), OutError))
		{
			return false;
		}
		return ViewModel.SetWeaponDataPath(SourceDraft.DefaultWeaponData.ToSoftObjectPath(), OutError);
	}

	// exact disposable target persisted snapshot/fingerprint를 읽습니다.
	bool ReadPersistedSnapshot(
		const FString& TargetObjectPath,
		FCFEquipmentSemanticSnapshot& OutSnapshot,
		bool& bOutPackageDirty,
		FString& OutError)
	{
		// Exact persisted EquipmentPreset target입니다.
		UCFEquipmentPresetData* PersistedPreset = LoadObject<UCFEquipmentPresetData>(nullptr, *TargetObjectPath);
		if (PersistedPreset == nullptr || PersistedPreset->GetClass() != UCFEquipmentPresetData::StaticClass())
		{
			OutError = FString::Printf(TEXT("Persisted EquipmentPreset fixture를 exact class로 로드하지 못했습니다: %s"), *TargetObjectPath);
			return false;
		}
		// Persisted target owning package입니다.
		UPackage* Package = PersistedPreset->GetOutermost();
		bOutPackageDirty = Package != nullptr && Package->IsDirty();
		// exact7 extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> Issues;
		if (!FCFEquipmentDurable::ExtractSnapshotFromAsset(*PersistedPreset, OutSnapshot, Issues))
		{
			OutError = Issues.IsEmpty() ? TEXT("Persisted EquipmentPreset exact7 extraction에 실패했습니다.") : Issues[0].Message;
			return false;
		}
		OutError.Reset();
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentDigestTest,
	"CarFight.EquipmentAuthoring.P003.SemanticDigest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Revision 1 exact7 ReviewProposalDigest가 stable canonical tokens와 exact TargetClassPath에만 bind되는지 검증합니다.
bool FCFEquipmentDigestTest::RunTest(const FString& Parameters)
{
	// Deterministic exact7 semantic snapshot입니다.
	FCFEquipmentSemanticSnapshot Snapshot;
	Snapshot.EquipmentId = TEXT("EBA_Digest");
	Snapshot.DisplayNameSource = TEXT("Digest Fixture");
	Snapshot.RequiredMountType = ECFVehicleMountType::Turret;
	Snapshot.RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// Prospective semantic fingerprint입니다.
	FString ProspectiveFingerprint;
	// Semantic fingerprint diagnostic입니다.
	FString FingerprintError;
	TestTrue(TEXT("Revision 1 semantic fingerprint builds"), FCFEquipmentDurable::BuildSemanticFingerprint(Snapshot, ProspectiveFingerprint, FingerprintError));

	// First canonical ReviewProposalDigest입니다.
	FString FirstDigest;
	// Digest build diagnostic입니다.
	FString DigestError;
	const bool bFirstDigest = FCFEquipmentDurable::BuildReviewProposalDigest(
		ECFEquipmentReviewOperation::Create,
		TEXT("/Game/CarFight/Tests/EquipmentAuthoring/EQ_Digest.EQ_Digest"),
		FCFEquipmentDurable::GetTargetClassPath(),
		FCFEquipmentDurable::GetAbsentTargetToken(),
		ProspectiveFingerprint,
		FCFEquipmentDurable::GetContractRevision(),
		FCFEquipmentDurable::GetReadyValidationToken(),
		FirstDigest,
		DigestError);
	TestTrue(FString::Printf(TEXT("Canonical ReviewProposalDigest builds: %s"), *DigestError), bFirstDigest);

	// Same stable inputs에서 다시 만든 digest입니다.
	FString SecondDigest;
	const bool bSecondDigest = FCFEquipmentDurable::BuildReviewProposalDigest(
		ECFEquipmentReviewOperation::Create,
		TEXT("/Game/CarFight/Tests/EquipmentAuthoring/EQ_Digest.EQ_Digest"),
		FCFEquipmentDurable::GetTargetClassPath(),
		FCFEquipmentDurable::GetAbsentTargetToken(),
		ProspectiveFingerprint,
		FCFEquipmentDurable::GetContractRevision(),
		FCFEquipmentDurable::GetReadyValidationToken(),
		SecondDigest,
		DigestError);
	TestTrue(TEXT("Repeated canonical ReviewProposalDigest builds"), bSecondDigest);
	TestEqual(TEXT("Repeated canonical ReviewProposalDigest is stable"), FirstDigest, SecondDigest);

	// Wrong target class binding을 검증할 output digest입니다.
	FString WrongClassDigest;
	const bool bWrongClassAccepted = FCFEquipmentDurable::BuildReviewProposalDigest(
		ECFEquipmentReviewOperation::Create,
		TEXT("/Game/CarFight/Tests/EquipmentAuthoring/EQ_Digest.EQ_Digest"),
		TEXT("/Script/CarFight_Re.CFWeaponData"),
		FCFEquipmentDurable::GetAbsentTargetToken(),
		ProspectiveFingerprint,
		FCFEquipmentDurable::GetContractRevision(),
		FCFEquipmentDurable::GetReadyValidationToken(),
		WrongClassDigest,
		DigestError);
	TestFalse(TEXT("Wrong TargetClassPath is rejected before approval"), bWrongClassAccepted);
	return bFirstDigest && bSecondDigest && FirstDigest == SecondDigest && !bWrongClassAccepted;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentCreateUpdateTest,
	"CarFight.EquipmentAuthoring.P003.CreateUpdateDurable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// disposable EquipmentPreset의 Create→disk readback→Update→disk readback durable round-trip을 검증합니다.
bool FCFEquipmentCreateUpdateTest::RunTest(const FString& Parameters)
{
	// Pre-test disposable cleanup diagnostic입니다.
	FString CleanupError;
	if (!CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError))
	{
		AddError(CleanupError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Failure path에서도 exact test root만 best-effort cleanup합니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Product child graph를 read-only로 재사용할 source Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Source discovery diagnostic입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindReusablePersistedSourceDraft(SourceDraft, SourceError))
	{
		AddError(SourceError);
		return false;
	}

	// Disposable Create fixture path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("RoundTrip"));
	// Create/Update orchestration VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Draft configuration diagnostic입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(ViewModel, SourceDraft, Paths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}

	// Create Review diagnostic입니다.
	FString ReviewError;
	const bool bCreateReviewed = ViewModel.ReviewCurrentDraft(ReviewError);
	TestTrue(FString::Printf(TEXT("Disposable Create Review succeeds: %s"), *ReviewError), bCreateReviewed);
	if (!bCreateReviewed)
	{
		return false;
	}
	// Reviewed prospective Create fingerprint입니다.
	const FString ReviewedCreateFingerprint = ViewModel.GetReviewState().ProspectiveSemanticFingerprint;
	// Create durable terminal report입니다.
	FCFEquipmentApplyReport CreateReport;
	const bool bCreateApplied = ViewModel.ApplyReviewedDraft(CreateReport);
	TestTrue(FString::Printf(TEXT("Disposable Create durable Apply succeeds: %s"), *CreateReport.Diagnostic), bCreateApplied);
	TestTrue(TEXT("Create approval is consumed"), CreateReport.bApprovalConsumed);
	TestEqual(TEXT("Create terminal result is DurableApplied"), CreateReport.Result, ECFEquipmentApplyResult::DurableApplied);
	if (!bCreateApplied)
	{
		return false;
	}

	// Persisted Create snapshot입니다.
	FCFEquipmentSemanticSnapshot PersistedCreateSnapshot;
	// Persisted Create package dirty state입니다.
	bool bCreatePackageDirty = true;
	// Persisted Create readback diagnostic입니다.
	FString ReadbackError;
	const bool bCreateReadback = CFEquipmentDurableTestsPrivate::ReadPersistedSnapshot(Paths.TargetObjectPath, PersistedCreateSnapshot, bCreatePackageDirty, ReadbackError);
	TestTrue(FString::Printf(TEXT("Create persisted exact7 readback succeeds: %s"), *ReadbackError), bCreateReadback);
	TestFalse(TEXT("Create persisted package is clean"), bCreatePackageDirty);
	TestEqual(TEXT("Create persisted fingerprint matches Reviewed fingerprint"), PersistedCreateSnapshot.SemanticFingerprint, ReviewedCreateFingerprint);

	// Durable Create 성공 뒤 같은 VM은 persisted target을 source로 삼는 Update state로 전환됩니다.
	TestFalse(TEXT("Create success transitions VM out of new-draft state"), ViewModel.IsNewDraft());
	TestEqual(TEXT("Create success binds exact persisted target"), ViewModel.GetTargetObjectPathForReview(), Paths.TargetObjectPath);
	ViewModel.SetDisplayName(FText::FromString(TEXT("EBA P003 Updated")));

	// Update Review diagnostic입니다.
	FString UpdateReviewError;
	const bool bUpdateReviewed = ViewModel.ReviewCurrentDraft(UpdateReviewError);
	TestTrue(FString::Printf(TEXT("Disposable Update Review succeeds: %s"), *UpdateReviewError), bUpdateReviewed);
	if (!bUpdateReviewed)
	{
		return false;
	}
	// Reviewed prospective Update fingerprint입니다.
	const FString ReviewedUpdateFingerprint = ViewModel.GetReviewState().ProspectiveSemanticFingerprint;
	// Update durable terminal report입니다.
	FCFEquipmentApplyReport UpdateReport;
	const bool bUpdateApplied = ViewModel.ApplyReviewedDraft(UpdateReport);
	TestTrue(FString::Printf(TEXT("Disposable Update durable Apply succeeds: %s"), *UpdateReport.Diagnostic), bUpdateApplied);
	TestTrue(TEXT("Update approval is consumed"), UpdateReport.bApprovalConsumed);
	if (!bUpdateApplied)
	{
		return false;
	}

	// Persisted Update snapshot입니다.
	FCFEquipmentSemanticSnapshot PersistedUpdateSnapshot;
	// Persisted Update package dirty state입니다.
	bool bUpdatePackageDirty = true;
	const bool bUpdateReadback = CFEquipmentDurableTestsPrivate::ReadPersistedSnapshot(Paths.TargetObjectPath, PersistedUpdateSnapshot, bUpdatePackageDirty, ReadbackError);
	TestTrue(FString::Printf(TEXT("Update persisted exact7 readback succeeds: %s"), *ReadbackError), bUpdateReadback);
	TestFalse(TEXT("Update persisted package is clean"), bUpdatePackageDirty);
	TestEqual(TEXT("Update persisted fingerprint matches Reviewed fingerprint"), PersistedUpdateSnapshot.SemanticFingerprint, ReviewedUpdateFingerprint);
	return bCreateReadback && bUpdateReadback && !bCreatePackageDirty && !bUpdatePackageDirty;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentStaleOneShotTest,
	"CarFight.EquipmentAuthoring.P003.StaleOneShot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// final fresh digest mismatch가 mutation0으로 차단되고 같은 approval이 즉시 Consumed되어 재사용되지 않는지 검증합니다.
bool FCFEquipmentStaleOneShotTest::RunTest(const FString& Parameters)
{
	// Pre-test cleanup diagnostic입니다.
	FString CleanupError;
	CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError);
	ON_SCOPE_EXIT
	{
		// Failure path cleanup diagnostic입니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Read-only reusable source Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Source discovery diagnostic입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindReusablePersistedSourceDraft(SourceDraft, SourceError))
	{
		AddError(SourceError);
		return false;
	}
	// Disposable stale fixture path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("Stale"));
	// One-shot stale 검증 VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Draft configuration diagnostic입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(ViewModel, SourceDraft, Paths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	// Review diagnostic입니다.
	FString ReviewError;
	if (!ViewModel.ReviewCurrentDraft(ReviewError))
	{
		AddError(ReviewError);
		return false;
	}
	ViewModel.CorruptReviewedDigestForAutomation();

	// Stale Apply terminal report입니다.
	FCFEquipmentApplyReport FirstReport;
	const bool bFirstApply = ViewModel.ApplyReviewedDraft(FirstReport);
	TestFalse(TEXT("Corrupted reviewed digest blocks Apply"), bFirstApply);
	TestTrue(TEXT("Stale Apply consumes approval"), FirstReport.bApprovalConsumed);
	TestEqual(TEXT("Stale Apply is blocked before mutation"), FirstReport.Result, ECFEquipmentApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("Review state is Consumed after stale attempt"), ViewModel.GetReviewState().ApprovalState, ECFEquipmentApprovalState::Consumed);
	TestFalse(TEXT("Stale target was not persisted"), FPackageName::DoesPackageExist(Paths.PackageName));

	// Same approval second Apply terminal report입니다.
	FCFEquipmentApplyReport SecondReport;
	const bool bSecondApply = ViewModel.ApplyReviewedDraft(SecondReport);
	TestFalse(TEXT("Consumed stale approval cannot be retried"), bSecondApply);
	TestEqual(TEXT("Consumed retry stays blocked before mutation"), SecondReport.Result, ECFEquipmentApplyResult::BlockedBeforeMutation);
	return !bFirstApply && !bSecondApply && !FPackageName::DoesPackageExist(Paths.PackageName);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentLoadedDuplicateTest,
	"CarFight.EquipmentAuthoring.P003.LoadedDuplicate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// AssetRegistry에 등록되지 않은 loaded-only exact EquipmentPreset duplicate identity도 Review에서 fail-closed하는지 검증합니다.
bool FCFEquipmentLoadedDuplicateTest::RunTest(const FString& Parameters)
{
	// Pre-test cleanup diagnostic입니다.
	FString CleanupError;
	CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError);
	ON_SCOPE_EXIT
	{
		// Loaded-only fixture cleanup diagnostic입니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Read-only reusable source Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Source discovery diagnostic입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindReusablePersistedSourceDraft(SourceDraft, SourceError))
	{
		AddError(SourceError);
		return false;
	}
	// Reviewed candidate path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths CandidatePaths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("DuplicateCandidate"));
	// Loaded-only duplicate object path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths DuplicatePaths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("DuplicateLoaded"));

	// Registry notification/save 없이 loaded-only package를 만듭니다.
	UPackage* DuplicatePackage = CreatePackage(*DuplicatePaths.PackageName);
	// Registry notification/save 없이 exact EquipmentPreset UObject를 만듭니다.
	UCFEquipmentPresetData* LoadedOnlyDuplicate = NewObject<UCFEquipmentPresetData>(
		DuplicatePackage,
		*DuplicatePaths.AssetName,
		RF_Public | RF_Standalone);
	LoadedOnlyDuplicate->EquipmentId = CandidatePaths.EquipmentId;

	// Candidate Review VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Candidate Draft configuration diagnostic입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(ViewModel, SourceDraft, CandidatePaths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	// Duplicate Review diagnostic입니다.
	FString ReviewError;
	const bool bReviewed = ViewModel.ReviewCurrentDraft(ReviewError);
	TestFalse(TEXT("Loaded-only duplicate EquipmentId blocks Review"), bReviewed);
	TestTrue(TEXT("Loaded-only duplicate emits localized duplicate diagnostic"), ReviewError.Contains(TEXT("장비 프리셋 ID 중복")));
	TestEqual(TEXT("Duplicate failure keeps approval Unreviewed"), ViewModel.GetReviewState().ApprovalState, ECFEquipmentApprovalState::Unreviewed);
	TestFalse(TEXT("Duplicate candidate target is not persisted"), FPackageName::DoesPackageExist(CandidatePaths.PackageName));
	return !bReviewed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentChildStateTest,
	"CarFight.EquipmentAuthoring.P003.ChildStateUnconfirmed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// persisted Product child를 수정하지 않고 exact child-state fault를 주입해 Review가 mutation0 fail-closed하는지 검증합니다.
bool FCFEquipmentChildStateTest::RunTest(const FString& Parameters)
{
	// Read-only reusable source Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Source discovery diagnostic입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindReusablePersistedSourceDraft(SourceDraft, SourceError))
	{
		AddError(SourceError);
		return false;
	}
	// Disposable child-state fixture path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("ChildState"));
	// Child-state fail-closed 검증 VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Draft configuration diagnostic입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(ViewModel, SourceDraft, Paths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}

	// Fault injection할 actual persisted child object path입니다.
	const FString ChildObjectPath = SourceDraft.DraftMode == ECFEquipmentDraftMode::Scanner
		? SourceDraft.DefaultSensorData.ToSoftObjectPath().ToString()
		: SourceDraft.DefaultWeaponData.ToSoftObjectPath().ToString();
	ViewModel.SetForcedChildStateUnconfirmedForAutomation(ChildObjectPath);

	// Child-state Review diagnostic입니다.
	FString ReviewError;
	const bool bReviewed = ViewModel.ReviewCurrentDraft(ReviewError);
	TestFalse(TEXT("ChildStateUnconfirmed blocks Review"), bReviewed);
	TestTrue(TEXT("Child-state failure is explicit"), ReviewError.Contains(TEXT("ChildStateUnconfirmed")));
	TestEqual(TEXT("Child-state failure keeps approval Unreviewed"), ViewModel.GetReviewState().ApprovalState, ECFEquipmentApprovalState::Unreviewed);
	TestFalse(TEXT("Child-state failure does not persist target"), FPackageName::DoesPackageExist(Paths.PackageName));
	return !bReviewed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentDirtyTargetTest,
	"CarFight.EquipmentAuthoring.P003.DirtyTarget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// disposable persisted Update target package가 dirty이면 Review 단계에서 mutation0으로 차단되는지 검증합니다.
bool FCFEquipmentDirtyTargetTest::RunTest(const FString& Parameters)
{
	// Pre-test cleanup diagnostic입니다.
	FString CleanupError;
	CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError);
	ON_SCOPE_EXIT
	{
		// Dirty fixture cleanup diagnostic입니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Read-only reusable source Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Source discovery diagnostic입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindReusablePersistedSourceDraft(SourceDraft, SourceError))
	{
		AddError(SourceError);
		return false;
	}
	// Disposable dirty-target fixture path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("Dirty"));
	// Initial Create VM입니다.
	FCFEquipmentBuilderVM CreateViewModel;
	// Draft configuration diagnostic입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(CreateViewModel, SourceDraft, Paths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	// Initial Review diagnostic입니다.
	FString ReviewError;
	if (!CreateViewModel.ReviewCurrentDraft(ReviewError))
	{
		AddError(ReviewError);
		return false;
	}
	// Initial durable Create report입니다.
	FCFEquipmentApplyReport CreateReport;
	if (!CreateViewModel.ApplyReviewedDraft(CreateReport))
	{
		AddError(CreateReport.Diagnostic);
		return false;
	}

	// Persisted disposable target입니다.
	UCFEquipmentPresetData* PersistedPreset = LoadObject<UCFEquipmentPresetData>(nullptr, *Paths.TargetObjectPath);
	// Persisted disposable target package입니다.
	UPackage* PersistedPackage = PersistedPreset != nullptr ? PersistedPreset->GetOutermost() : nullptr;
	if (PersistedPackage == nullptr)
	{
		AddError(TEXT("Dirty-target fixture package를 찾지 못했습니다."));
		return false;
	}
	PersistedPackage->SetDirtyFlag(true);

	// Durable Create 성공 뒤 same VM의 exact persisted Update target만 dirty로 만든 상태에서 Draft를 변경합니다.
	CreateViewModel.SetDisplayName(FText::FromString(TEXT("Dirty target must block")));
	// Dirty target Review diagnostic입니다.
	FString DirtyReviewError;
	const bool bReviewed = CreateViewModel.ReviewCurrentDraft(DirtyReviewError);
	TestFalse(TEXT("Dirty Update target blocks Review"), bReviewed);
	TestTrue(TEXT("Dirty Update target emits TargetDirtyUnowned"), DirtyReviewError.Contains(TEXT("TargetDirtyUnowned")));
	TestEqual(TEXT("Dirty target failure keeps approval Unreviewed"), CreateViewModel.GetReviewState().ApprovalState, ECFEquipmentApprovalState::Unreviewed);
	PersistedPackage->SetDirtyFlag(false);
	return !bReviewed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentSaveRetryTest,
	"CarFight.EquipmentAuthoring.P003.SaveUncertaintyRetry0",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// shared core SaveStateUnconfirmed 뒤 approval이 Consumed되고 same-approval retry0인지 검증합니다.
bool FCFEquipmentSaveRetryTest::RunTest(const FString& Parameters)
{
	// Pre-test cleanup diagnostic입니다.
	FString CleanupError;
	CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError);
	ON_SCOPE_EXIT
	{
		CFDADurableCore::ResetTestFaults();
		// Save uncertainty fixture cleanup diagnostic입니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Read-only reusable source Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Source discovery diagnostic입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindReusablePersistedSourceDraft(SourceDraft, SourceError))
	{
		AddError(SourceError);
		return false;
	}
	// Disposable save-uncertainty fixture path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("SaveUncertain"));
	// Save uncertainty 검증 VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Draft configuration diagnostic입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(ViewModel, SourceDraft, Paths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	// Review diagnostic입니다.
	FString ReviewError;
	if (!ViewModel.ReviewCurrentDraft(ReviewError))
	{
		AddError(ReviewError);
		return false;
	}

	CFDADurableCore::SetForceSaveFailureTarget(Paths.TargetObjectPath);
	// Forced SaveStateUnconfirmed terminal report입니다.
	FCFEquipmentApplyReport FirstReport;
	const bool bFirstApply = ViewModel.ApplyReviewedDraft(FirstReport);
	TestFalse(TEXT("Forced Save uncertainty is not reported as success"), bFirstApply);
	TestEqual(TEXT("Forced Save uncertainty maps to SaveStateUnconfirmed"), FirstReport.Result, ECFEquipmentApplyResult::SaveStateUnconfirmed);
	TestTrue(TEXT("SaveStateUnconfirmed consumes approval"), FirstReport.bApprovalConsumed);
	TestEqual(TEXT("Review state is Consumed after SaveStateUnconfirmed"), ViewModel.GetReviewState().ApprovalState, ECFEquipmentApprovalState::Consumed);

	CFDADurableCore::ResetTestFaults();
	// Same-approval retry terminal report입니다.
	FCFEquipmentApplyReport SecondReport;
	const bool bSecondApply = ViewModel.ApplyReviewedDraft(SecondReport);
	TestFalse(TEXT("Same approval retry is forbidden after SaveStateUnconfirmed"), bSecondApply);
	TestEqual(TEXT("Same approval retry is blocked before mutation"), SecondReport.Result, ECFEquipmentApplyResult::BlockedBeforeMutation);
	return !bFirstApply && !bSecondApply && FirstReport.Result == ECFEquipmentApplyResult::SaveStateUnconfirmed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005HeavyCannonLocalizationTest,
	"CarFight.EquipmentAuthoring.P005.HeavyCannonLocalizedDisplayName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 실제 HeavyCannon의 localized DisplayName을 다시 쓰지 않고 기존 장비 Review가 성공하며 localization identity가 불변인지 검증합니다.
bool FCFEquipmentP005HeavyCannonLocalizationTest::RunTest(const FString& Parameters)
{
	// USER가 재현한 실제 HeavyCannon Product object path입니다.
	const FString HeavyCannonObjectPath = TEXT("/Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon.HeavyCannon");
	// Review 전 exact HeavyCannon Product source입니다.
	UCFEquipmentPresetData* HeavyCannonPreset = LoadObject<UCFEquipmentPresetData>(nullptr, *HeavyCannonObjectPath);
	TestNotNull(TEXT("P005 HeavyCannon exact Product source resolves"), HeavyCannonPreset);
	if (HeavyCannonPreset == nullptr || HeavyCannonPreset->GetClass() != UCFEquipmentPresetData::StaticClass())
	{
		return false;
	}

	// Review 전 Product package입니다.
	UPackage* HeavyCannonPackage = HeavyCannonPreset->GetOutermost();
	TestNotNull(TEXT("P005 HeavyCannon has owning package"), HeavyCannonPackage);
	if (HeavyCannonPackage == nullptr)
	{
		return false;
	}
	TestFalse(TEXT("P005 HeavyCannon package starts clean"), HeavyCannonPackage->IsDirty());

	// Review 전 localized DisplayName 전체 값입니다.
	const FText DisplayNameBeforeReview = HeavyCannonPreset->DisplayName;
	// Review 전 localization namespace optional 값입니다.
	const TOptional<FString> NamespaceBeforeOption = FTextInspector::GetNamespace(DisplayNameBeforeReview);
	// Review 전 localization key optional 값입니다.
	const TOptional<FString> KeyBeforeOption = FTextInspector::GetKey(DisplayNameBeforeReview);
	// Review 전 localization source string입니다.
	const FString* SourceBeforePtr = FTextInspector::GetSourceString(DisplayNameBeforeReview);
	// optional lifetime과 무관하게 비교할 namespace 값입니다.
	const FString NamespaceBefore = NamespaceBeforeOption.IsSet() ? NamespaceBeforeOption.GetValue() : FString();
	// optional lifetime과 무관하게 비교할 key 값입니다.
	const FString KeyBefore = KeyBeforeOption.IsSet() ? KeyBeforeOption.GetValue() : FString();
	// 포인터 lifetime과 무관하게 비교할 source 값입니다.
	const FString SourceBefore = SourceBeforePtr != nullptr ? *SourceBeforePtr : FString();
	TestFalse(TEXT("P005 HeavyCannon DisplayName is not literal-lossless editable"), FCFEquipmentDurable::CanEditDisplayNameLosslessly(DisplayNameBeforeReview));
	TestFalse(TEXT("P005 HeavyCannon localized namespace exists"), NamespaceBefore.IsEmpty());
	TestFalse(TEXT("P005 HeavyCannon localized key exists"), KeyBefore.IsEmpty());

	// HeavyCannon existing Review를 실행할 fresh ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Equipment browser 초기화 진단입니다.
	FString InitializeError;
	const bool bInitialized = ViewModel.Initialize(InitializeError);
	TestTrue(FString::Printf(TEXT("P005 HeavyCannon browser initializes: %s"), *InitializeError), bInitialized);
	if (!bInitialized)
	{
		return false;
	}

	// HeavyCannon existing read-only load 진단입니다.
	FString LoadError;
	const bool bLoaded = ViewModel.LoadExistingPreset(HeavyCannonObjectPath, LoadError);
	TestTrue(FString::Printf(TEXT("P005 HeavyCannon existing load succeeds: %s"), *LoadError), bLoaded);
	if (!bLoaded)
	{
		return false;
	}
	TestTrue(TEXT("P005 HeavyCannon Draft preserves localized DisplayName"), ViewModel.GetDraft().bPreserveExistingDisplayName);
	TestFalse(TEXT("P005 HeavyCannon localized DisplayName editor is locked"), ViewModel.CanEditDisplayName());

	// HeavyCannon fresh Review 진단입니다.
	FString ReviewError;
	const bool bReviewed = ViewModel.ReviewCurrentDraft(ReviewError);
	TestTrue(FString::Printf(TEXT("P005 HeavyCannon localized Review succeeds: %s"), *ReviewError), bReviewed);
	if (!bReviewed)
	{
		return false;
	}
	TestEqual(TEXT("P005 HeavyCannon Review uses Contract Revision 2"), ViewModel.GetReviewState().ContractRevision, 2);
	TestTrue(TEXT("P005 HeavyCannon reviewed snapshot keeps preserve mode"), ViewModel.GetReviewState().AfterSnapshot.bPreserveExistingDisplayName);
	TestEqual(TEXT("P005 HeavyCannon reviewed snapshot uses preserve sentinel"), ViewModel.GetReviewState().AfterSnapshot.DisplayNameSource, FString(FCFEquipmentDurable::GetPreservedDisplayNameToken()));

	// Review 뒤 동일 Product source를 다시 읽어 localization identity가 변하지 않았는지 확인합니다.
	UCFEquipmentPresetData* HeavyCannonAfterReview = LoadObject<UCFEquipmentPresetData>(nullptr, *HeavyCannonObjectPath);
	TestNotNull(TEXT("P005 HeavyCannon remains readable after Review"), HeavyCannonAfterReview);
	if (HeavyCannonAfterReview == nullptr)
	{
		return false;
	}
	// Review 뒤 localization namespace optional 값입니다.
	const TOptional<FString> NamespaceAfterOption = FTextInspector::GetNamespace(HeavyCannonAfterReview->DisplayName);
	// Review 뒤 localization key optional 값입니다.
	const TOptional<FString> KeyAfterOption = FTextInspector::GetKey(HeavyCannonAfterReview->DisplayName);
	// Review 뒤 localization source string입니다.
	const FString* SourceAfterPtr = FTextInspector::GetSourceString(HeavyCannonAfterReview->DisplayName);
	// 비교용 namespace 값입니다.
	const FString NamespaceAfter = NamespaceAfterOption.IsSet() ? NamespaceAfterOption.GetValue() : FString();
	// 비교용 key 값입니다.
	const FString KeyAfter = KeyAfterOption.IsSet() ? KeyAfterOption.GetValue() : FString();
	// 비교용 source 값입니다.
	const FString SourceAfter = SourceAfterPtr != nullptr ? *SourceAfterPtr : FString();
	TestEqual(TEXT("P005 HeavyCannon localization namespace is unchanged"), NamespaceAfter, NamespaceBefore);
	TestEqual(TEXT("P005 HeavyCannon localization key is unchanged"), KeyAfter, KeyBefore);
	TestEqual(TEXT("P005 HeavyCannon localization source is unchanged"), SourceAfter, SourceBefore);
	TestTrue(TEXT("P005 HeavyCannon display text is unchanged"), HeavyCannonAfterReview->DisplayName.EqualTo(DisplayNameBeforeReview));
	TestFalse(TEXT("P005 HeavyCannon Review leaves Product package clean"), HeavyCannonPackage->IsDirty());
	return bReviewed
		&& NamespaceAfter == NamespaceBefore
		&& KeyAfter == KeyBefore
		&& SourceAfter == SourceBefore
		&& !HeavyCannonPackage->IsDirty();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005WeaponNewFlowTest,
	"CarFight.EquipmentAuthoring.P005.WeaponNewFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Representative Product Weapon source를 read-only로 재사용해 신규 EquipmentPreset 검토→durable 저장→persisted readback을 end-to-end 검증합니다.
bool FCFEquipmentP005WeaponNewFlowTest::RunTest(const FString& Parameters)
{
	// Pre-test disposable cleanup 진단입니다.
	FString CleanupError;
	if (!CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError))
	{
		AddError(CleanupError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Failure path에서도 exact disposable root만 정리합니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Representative Product Weapon source Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Representative Product Weapon source exact object path입니다.
	FString SourceObjectPath;
	// Representative source discovery 진단입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindRepresentativeProductSourceDraft(
		ECFEquipmentDraftMode::Weapon,
		SourceDraft,
		SourceObjectPath,
		SourceError))
	{
		AddError(SourceError);
		return false;
	}

	// Representative Weapon Product source exact UObject입니다.
	UCFEquipmentPresetData* ProductSourcePreset = LoadObject<UCFEquipmentPresetData>(nullptr, *SourceObjectPath);
	// Representative Weapon Product source owning package입니다.
	UPackage* ProductSourcePackage = ProductSourcePreset != nullptr ? ProductSourcePreset->GetOutermost() : nullptr;
	TestNotNull(TEXT("Representative Weapon Product source resolves exact UObject"), ProductSourcePreset);
	TestNotNull(TEXT("Representative Weapon Product source has owning package"), ProductSourcePackage);
	if (ProductSourcePreset == nullptr || ProductSourcePackage == nullptr)
	{
		return false;
	}
	TestFalse(TEXT("Representative Weapon Product source starts clean"), ProductSourcePackage->IsDirty());

	// Disposable persisted readback에서 재사용할 진단 문자열입니다.
	FString ReadbackError;

	// P005 Weapon 신규 disposable target path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("P005Weapon"));
	// Weapon 신규 flow를 실행할 ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Source child graph를 신규 Draft에 선택하는 진단입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(ViewModel, SourceDraft, Paths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}

	// 신규 Weapon Draft의 fresh Review 진단입니다.
	FString ReviewError;
	// 신규 Weapon Draft의 fresh Review 성공 여부입니다.
	const bool bReviewed = ViewModel.ReviewCurrentDraft(ReviewError);
	TestTrue(FString::Printf(TEXT("P005 Weapon new Review succeeds: %s"), *ReviewError), bReviewed);
	if (!bReviewed)
	{
		return false;
	}

	// 신규 Weapon durable terminal report입니다.
	FCFEquipmentApplyReport ApplyReport;
	// 신규 Weapon durable Apply 성공 여부입니다.
	const bool bApplied = ViewModel.ApplyReviewedDraft(ApplyReport);
	TestTrue(FString::Printf(TEXT("P005 Weapon new Apply succeeds: %s"), *ApplyReport.Diagnostic), bApplied);
	TestEqual(TEXT("P005 Weapon new terminal result is DurableApplied"), ApplyReport.Result, ECFEquipmentApplyResult::DurableApplied);
	if (!bApplied)
	{
		return false;
	}

	// Persisted Weapon 신규 snapshot입니다.
	FCFEquipmentSemanticSnapshot PersistedSnapshot;
	// Persisted Weapon 신규 package dirty state입니다.
	bool bPersistedDirty = true;
	// Persisted Weapon 신규 exact7 readback 성공 여부입니다.
	const bool bReadback = CFEquipmentDurableTestsPrivate::ReadPersistedSnapshot(Paths.TargetObjectPath, PersistedSnapshot, bPersistedDirty, ReadbackError);
	TestTrue(FString::Printf(TEXT("P005 Weapon new persisted readback succeeds: %s"), *ReadbackError), bReadback);
	TestFalse(TEXT("P005 Weapon new persisted package is clean"), bPersistedDirty);
	TestFalse(TEXT("P005 Weapon new keeps TurretMountData"), PersistedSnapshot.DefaultTurretMountDataPath.IsNull());
	TestFalse(TEXT("P005 Weapon new keeps WeaponData"), PersistedSnapshot.DefaultWeaponDataPath.IsNull());
	TestTrue(TEXT("P005 Weapon new keeps SensorData empty"), PersistedSnapshot.DefaultSensorDataPath.IsNull());

	// Flow 뒤 다시 resolve한 exact Product EquipmentPreset입니다.
	UCFEquipmentPresetData* ProductAfterPreset = LoadObject<UCFEquipmentPresetData>(nullptr, *SourceObjectPath);
	// Flow 뒤 Product source owning package입니다.
	UPackage* ProductAfterPackage = ProductAfterPreset != nullptr ? ProductAfterPreset->GetOutermost() : nullptr;
	TestNotNull(TEXT("Representative Weapon Product source remains readable"), ProductAfterPreset);
	TestNotNull(TEXT("Representative Weapon Product source package remains available"), ProductAfterPackage);
	if (ProductAfterPreset == nullptr || ProductAfterPackage == nullptr)
	{
		return false;
	}
	TestFalse(TEXT("Representative Weapon Product source remains clean"), ProductAfterPackage->IsDirty());
	// Representative Product source exact7 authored values가 flow 전 read-only Draft와 동일한지 확인합니다.
	const bool bProductUnchanged = ProductAfterPreset->EquipmentId == SourceDraft.EquipmentId
		&& ProductAfterPreset->DisplayName.EqualTo(SourceDraft.DisplayName)
		&& ProductAfterPreset->RequiredMountType == SourceDraft.RequiredMountType
		&& ProductAfterPreset->RequiredWeaponSize == SourceDraft.RequiredWeaponSize
		&& FSoftObjectPath(ProductAfterPreset->DefaultTurretMountData.Get()) == SourceDraft.DefaultTurretMountData.ToSoftObjectPath()
		&& FSoftObjectPath(ProductAfterPreset->DefaultWeaponData.Get()) == SourceDraft.DefaultWeaponData.ToSoftObjectPath()
		&& FSoftObjectPath(ProductAfterPreset->DefaultSensorData.Get()) == SourceDraft.DefaultSensorData.ToSoftObjectPath();
	TestTrue(TEXT("Representative Weapon Product source authored exact7 remains unchanged"), bProductUnchanged);
	return bReadback && !bPersistedDirty && !ProductAfterPackage->IsDirty() && bProductUnchanged;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005ScannerRecoveryTest,
	"CarFight.EquipmentAuthoring.P005.ScannerErrorRecoveryFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Representative Product Scanner source로 missing Sensor 오류 안내→사용자 교정→Review→durable 저장 흐름을 end-to-end 검증합니다.
bool FCFEquipmentP005ScannerRecoveryTest::RunTest(const FString& Parameters)
{
	// Pre-test disposable cleanup 진단입니다.
	FString CleanupError;
	if (!CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError))
	{
		AddError(CleanupError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Failure path에서도 exact disposable root만 정리합니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Representative Scanner 신규 flow가 사용할 persisted clean/native-valid SensorData path입니다.
	FSoftObjectPath SensorDataPath;
	// Representative SensorData discovery 진단입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindRepresentativePersistedSensorPath(SensorDataPath, SourceError))
	{
		AddError(SourceError);
		return false;
	}

	// P005 Scanner 오류→해결 disposable target path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("P005Scanner"));
	// Scanner 오류→해결 flow를 실행할 ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.EnableDisposableTestTargetsForAutomation(true);
	ViewModel.SetCreateTargetObjectPath(Paths.TargetObjectPath);
	ViewModel.SetEquipmentId(Paths.EquipmentId);
	ViewModel.SetDisplayName(FText::FromString(TEXT("EBA P005 Scanner Recovery")));
	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Utility);

	// Sensor를 일부러 선택하지 않은 오류 상태 validation입니다.
	const FCFEquipmentDraftValidation BrokenValidation = ViewModel.BuildCurrentValidation();
	TestFalse(TEXT("P005 Scanner missing Sensor is incomplete"), BrokenValidation.bPackageComplete);
	TestTrue(TEXT("P005 Scanner missing Sensor is blocking"), BrokenValidation.bHasBlockingErrors);
	TestTrue(
		TEXT("P005 Scanner missing Sensor exposes actionable validation guidance"),
		CFEquipmentDurableTestsPrivate::HasValidationDiagnostic(
			BrokenValidation,
			ECFEquipmentValidationSeverity::Error,
			TEXT("센서 데이터"),
			TEXT("센서 데이터가 필요")));

	// 오류 상태에서 Review가 mutation 전 차단되는 진단입니다.
	FString BrokenReviewError;
	// 오류 상태 Draft가 잘못 Reviewed되지 않았는지 확인할 결과입니다.
	const bool bBrokenReviewed = ViewModel.ReviewCurrentDraft(BrokenReviewError);
	TestFalse(TEXT("P005 Scanner broken Draft cannot be Reviewed"), bBrokenReviewed);
	TestEqual(TEXT("P005 Scanner broken Review remains Unreviewed"), ViewModel.GetReviewState().ApprovalState, ECFEquipmentApprovalState::Unreviewed);
	TestFalse(TEXT("P005 Scanner broken Draft does not persist target"), FPackageName::DoesPackageExist(Paths.PackageName));

	// 사용자 오류 해결 동작과 동일하게 representative SensorData를 선택하는 진단입니다.
	FString SelectionError;
	// representative SensorData 선택 성공 여부입니다.
	const bool bSensorSelected = ViewModel.SetSensorDataPath(SensorDataPath, SelectionError);
	TestTrue(FString::Printf(TEXT("P005 Scanner recovery selects SensorData: %s"), *SelectionError), bSensorSelected);
	if (!bSensorSelected)
	{
		return false;
	}

	// Sensor 선택 뒤 복구된 Scanner validation입니다.
	const FCFEquipmentDraftValidation RecoveredValidation = ViewModel.BuildCurrentValidation();
	TestTrue(TEXT("P005 Scanner recovery becomes package-complete"), RecoveredValidation.bPackageComplete);
	TestFalse(TEXT("P005 Scanner recovery clears blocking errors"), RecoveredValidation.bHasBlockingErrors);

	// 복구 상태의 fresh Review 진단입니다.
	FString ReviewError;
	// 복구된 Scanner Draft의 fresh Review 성공 여부입니다.
	const bool bReviewed = ViewModel.ReviewCurrentDraft(ReviewError);
	TestTrue(FString::Printf(TEXT("P005 Scanner recovered Review succeeds: %s"), *ReviewError), bReviewed);
	if (!bReviewed)
	{
		return false;
	}

	// 복구된 Scanner durable terminal report입니다.
	FCFEquipmentApplyReport ApplyReport;
	// 복구된 Scanner durable Apply 성공 여부입니다.
	const bool bApplied = ViewModel.ApplyReviewedDraft(ApplyReport);
	TestTrue(FString::Printf(TEXT("P005 Scanner recovered Apply succeeds: %s"), *ApplyReport.Diagnostic), bApplied);
	if (!bApplied)
	{
		return false;
	}

	// Persisted Scanner recovery snapshot입니다.
	FCFEquipmentSemanticSnapshot PersistedSnapshot;
	// Persisted Scanner recovery package dirty state입니다.
	bool bPersistedDirty = true;
	// Persisted Scanner recovery readback 진단입니다.
	FString ReadbackError;
	// Persisted Scanner recovery exact7 readback 성공 여부입니다.
	const bool bReadback = CFEquipmentDurableTestsPrivate::ReadPersistedSnapshot(Paths.TargetObjectPath, PersistedSnapshot, bPersistedDirty, ReadbackError);
	TestTrue(FString::Printf(TEXT("P005 Scanner recovery persisted readback succeeds: %s"), *ReadbackError), bReadback);
	TestFalse(TEXT("P005 Scanner recovery persisted package is clean"), bPersistedDirty);
	TestTrue(TEXT("P005 Scanner recovery keeps TurretMountData empty"), PersistedSnapshot.DefaultTurretMountDataPath.IsNull());
	TestTrue(TEXT("P005 Scanner recovery keeps WeaponData empty"), PersistedSnapshot.DefaultWeaponDataPath.IsNull());
	TestFalse(TEXT("P005 Scanner recovery keeps SensorData"), PersistedSnapshot.DefaultSensorDataPath.IsNull());
	TestEqual(TEXT("P005 Scanner recovery keeps RequiredWeaponSize None"), PersistedSnapshot.RequiredWeaponSize, ECFVehicleWeaponSize::None);
	return bReadback && !bPersistedDirty;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005ExistingUpdateTest,
	"CarFight.EquipmentAuthoring.P005.ExistingUpdateFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Disposable target을 먼저 신규 생성한 뒤 fresh browser/VM에서 existing으로 다시 열어 Update durable flow를 검증합니다.
bool FCFEquipmentP005ExistingUpdateTest::RunTest(const FString& Parameters)
{
	// Pre-test disposable cleanup 진단입니다.
	FString CleanupError;
	if (!CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError))
	{
		AddError(CleanupError);
		return false;
	}
	ON_SCOPE_EXIT
	{
		// Failure path에서도 exact disposable root만 정리합니다.
		FString ScopeCleanupError;
		CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(ScopeCleanupError);
	};

	// Existing/Update fixture의 source로 쓸 representative Product Weapon Draft입니다.
	FCFEquipmentPresetDraft SourceDraft;
	// Representative Product source exact object path입니다.
	FString SourceObjectPath;
	// Representative source discovery 진단입니다.
	FString SourceError;
	if (!CFEquipmentDurableTestsPrivate::FindRepresentativeProductSourceDraft(
		ECFEquipmentDraftMode::Weapon,
		SourceDraft,
		SourceObjectPath,
		SourceError))
	{
		AddError(SourceError);
		return false;
	}

	// P005 existing/update disposable target path입니다.
	const CFEquipmentDurableTestsPrivate::FFixturePaths Paths = CFEquipmentDurableTestsPrivate::BuildFixturePaths(TEXT("P005Existing"));
	// Initial Create용 ViewModel입니다.
	FCFEquipmentBuilderVM CreateViewModel;
	// Initial disposable Draft 구성 진단입니다.
	FString ConfigureError;
	if (!CFEquipmentDurableTestsPrivate::ConfigureNewDisposableDraft(CreateViewModel, SourceDraft, Paths, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	// Initial Create Review 진단입니다.
	FString CreateReviewError;
	if (!CreateViewModel.ReviewCurrentDraft(CreateReviewError))
	{
		AddError(CreateReviewError);
		return false;
	}
	// Initial Create durable terminal report입니다.
	FCFEquipmentApplyReport CreateReport;
	if (!CreateViewModel.ApplyReviewedDraft(CreateReport))
	{
		AddError(CreateReport.Diagnostic);
		return false;
	}

	// Fresh existing browser/update flow용 새 ViewModel입니다.
	FCFEquipmentBuilderVM ExistingViewModel;
	ExistingViewModel.EnableDisposableTestTargetsForAutomation(true);
	// Fresh browser inventory 초기화 진단입니다.
	FString InitializeError;
	// Fresh existing browser inventory 초기화 성공 여부입니다.
	const bool bInitialized = ExistingViewModel.Initialize(InitializeError);
	TestTrue(FString::Printf(TEXT("P005 fresh existing browser initializes: %s"), *InitializeError), bInitialized);
	if (!bInitialized)
	{
		return false;
	}

	// Fresh browser에서 persisted disposable target을 existing source로 다시 여는 진단입니다.
	FString LoadError;
	// Persisted disposable target을 existing source로 다시 연 성공 여부입니다.
	const bool bLoaded = ExistingViewModel.LoadExistingPreset(Paths.TargetObjectPath, LoadError);
	TestTrue(FString::Printf(TEXT("P005 existing target reloads through browser: %s"), *LoadError), bLoaded);
	if (!bLoaded)
	{
		return false;
	}
	TestFalse(TEXT("P005 existing reload is not a new Draft"), ExistingViewModel.IsNewDraft());
	TestFalse(TEXT("P005 existing reload starts presentation-clean"), ExistingViewModel.HasDraftChanges());

	ExistingViewModel.SetDisplayName(FText::FromString(TEXT("EBA P005 Existing Updated")));
	TestTrue(TEXT("P005 existing edit marks transient Draft changed"), ExistingViewModel.HasDraftChanges());

	// Fresh existing Update Review 진단입니다.
	FString UpdateReviewError;
	// Fresh existing Update Review 성공 여부입니다.
	const bool bUpdatedReviewed = ExistingViewModel.ReviewCurrentDraft(UpdateReviewError);
	TestTrue(FString::Printf(TEXT("P005 existing Update Review succeeds: %s"), *UpdateReviewError), bUpdatedReviewed);
	if (!bUpdatedReviewed)
	{
		return false;
	}
	// Reviewed Update prospective fingerprint입니다.
	const FString ReviewedUpdateFingerprint = ExistingViewModel.GetReviewState().ProspectiveSemanticFingerprint;

	// Fresh existing Update durable terminal report입니다.
	FCFEquipmentApplyReport UpdateReport;
	// Fresh existing Update durable Apply 성공 여부입니다.
	const bool bUpdatedApplied = ExistingViewModel.ApplyReviewedDraft(UpdateReport);
	TestTrue(FString::Printf(TEXT("P005 existing Update Apply succeeds: %s"), *UpdateReport.Diagnostic), bUpdatedApplied);
	TestEqual(TEXT("P005 existing Update terminal result is DurableApplied"), UpdateReport.Result, ECFEquipmentApplyResult::DurableApplied);
	if (!bUpdatedApplied)
	{
		return false;
	}

	// Persisted existing Update snapshot입니다.
	FCFEquipmentSemanticSnapshot PersistedUpdateSnapshot;
	// Persisted existing Update package dirty state입니다.
	bool bPersistedDirty = true;
	// Persisted existing Update readback 진단입니다.
	FString ReadbackError;
	// Persisted existing Update exact7 readback 성공 여부입니다.
	const bool bReadback = CFEquipmentDurableTestsPrivate::ReadPersistedSnapshot(Paths.TargetObjectPath, PersistedUpdateSnapshot, bPersistedDirty, ReadbackError);
	TestTrue(FString::Printf(TEXT("P005 existing Update persisted readback succeeds: %s"), *ReadbackError), bReadback);
	TestFalse(TEXT("P005 existing Update persisted package is clean"), bPersistedDirty);
	TestEqual(TEXT("P005 existing Update persisted fingerprint matches Review"), PersistedUpdateSnapshot.SemanticFingerprint, ReviewedUpdateFingerprint);
	TestEqual(TEXT("P005 existing Update persisted DisplayName source"), PersistedUpdateSnapshot.DisplayNameSource, FString(TEXT("EBA P005 Existing Updated")));
	return bReadback && !bPersistedDirty;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005ResidueTest,
	"CarFight.EquipmentAuthoring.P005.ZDisposableResidue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// EBA-P0-05 end-to-end acceptance 뒤 disposable namespace residue가 exact0인지 검증합니다.
bool FCFEquipmentP005ResidueTest::RunTest(const FString& Parameters)
{
	// Final P005 disposable cleanup/residue 진단입니다.
	FString CleanupError;
	// P005 disposable root cleanup/residue exact0 성공 여부입니다.
	const bool bClean = CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError);
	TestTrue(FString::Printf(TEXT("EBA-P0-05 disposable residue exact0: %s"), *CleanupError), bClean);
	return bClean;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentResidueTest,
	"CarFight.EquipmentAuthoring.P003.ZDisposableResidue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// EBA-P0-03 disposable namespace가 registry/loaded UObject/disk 기준 exact0 residue인지 최종 검증합니다.
bool FCFEquipmentResidueTest::RunTest(const FString& Parameters)
{
	// Final disposable cleanup/residue diagnostic입니다.
	FString CleanupError;
	const bool bClean = CFEquipmentDurableTestsPrivate::CleanupFixtureRoot(CleanupError);
	TestTrue(FString::Printf(TEXT("EBA-P0-03 disposable residue exact0: %s"), *CleanupError), bClean);
	return bClean;
}

#endif
