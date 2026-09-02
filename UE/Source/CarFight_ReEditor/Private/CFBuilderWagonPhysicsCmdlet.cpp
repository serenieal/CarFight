// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonPhysicsCmdlet.cpp
// Version: v1.1.0
// Date: 2026-09-02
// Description: CF-FQ-040 actual Wagon PhysicsDraft v3 ESH-03 fixed-common ChangeUpRPM Profile persistent commit one-shot 구현입니다.
// Changelog:
// - v1.1.0: post-EngineCurve Target/PhysicsDraft v3 기준으로 교정하고 USER-approved ChangeUpRPM 4500→5500만 새 Transmission proposal로 commit하도록 exact pre/post hash와 8AT/EngineCurve/ChangeDown invariant를 갱신.
// - v1.0.1: UCLASS default constructor 구현 누락을 교정하고 실제 snapshot/transmission struct member 이름을 current source와 일치시킴.
// - v1.0.0: exact Wagon selection, current Reference reacceptance, PhysicsDraft v2 validation/preview, VehicleSpecificRequired diagnostic 검증, existing Step 5 commit, 5-package save/rollback을 최초 구현.
// Migration:
// - Builder-private 4 Profile + Recipe receipt만 persistent 변경합니다.
// - DA_Vehicle_Wagon Target, Reference Evidence, WSA/Chassis는 변경하거나 저장하지 않습니다.
// - current approved Engine Curve와 8AT/Final3.20/ChangeDown2000을 유지하고 Drivetrain ChangeUpRPM만 5500 proposal로 갱신합니다.

#include "CFBuilderWagonPhysicsCmdlet.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleBuilderVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFBuilderWagonPhysics, Log, All);

namespace CFBuilderWagonPhysics
{
	// actual Wagon exact managed object paths입니다.
	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");
	const TCHAR* WagonTargetPath = TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon");
	const TCHAR* WagonEvidencePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_Ref_Wagon.DA_Ref_Wagon");

	// 이번 5500 proposal 직전 persisted Transmission proposal identity입니다.
	const TCHAR* ExpectedPreviousTransmissionProposalHash = TEXT("14ec8925b761f16111251c1abac8c218");
	// USER가 승인한 5500 fixed-common exact Transmission proposal identity입니다.
	const TCHAR* ExpectedTransmissionProposalHash = TEXT("e1be2562d77fb1f5a993d6e7f5d17962");
	// 기존 USER-approved Engine Curve proposal identity입니다.
	const TCHAR* ExpectedEngineCurveProposalHash = TEXT("773221966499c6295f2a652a21b270a5");
	// Engine Curve DefinitionApply 뒤 현재 persisted Target exact hash입니다.
	const TCHAR* ExpectedCurrentTargetHash = TEXT("83c69e52522dc72649c32477b5aad220");

	// Process exit code taxonomy입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		SelectionFailed = 91,
		ReferenceReviewFailed = 92,
		DraftFailed = 93,
		PreviewFailed = 94,
		CommitFailed = 95,
		InvariantFailed = 96,
		SaveFailed = 97
	};

	// 하나의 existing package disk bytes backup입니다.
	struct FPackageBackup
	{
		// Save 대상 Asset object입니다.
		UObject* Asset = nullptr;

		// Existing .uasset exact filename입니다.
		FString Filename;

		// Commit 전 disk bytes입니다.
		TArray<uint8> Bytes;
	};

	// Asset package를 save 전 bytes로 backup합니다.
	bool BackupPackage(UObject* Asset, FPackageBackup& OutBackup, FString& OutError)
	{
		if (!Asset || !Asset->GetOutermost())
		{
			OutError = TEXT("Package backup 대상 Asset 또는 package가 null입니다.");
			return false;
		}

		OutBackup.Asset = Asset;
		OutBackup.Filename = FPackageName::LongPackageNameToFilename(
			Asset->GetOutermost()->GetName(),
			FPackageName::GetAssetPackageExtension());

		if (!FFileHelper::LoadFileToArray(OutBackup.Bytes, *OutBackup.Filename))
		{
			OutError = FString::Printf(TEXT("Package backup read 실패: %s"), *OutBackup.Filename);
			return false;
		}

		return true;
	}

	// Existing Asset을 자기 package exact .uasset에 저장합니다.
	bool SaveExistingAsset(UObject* Asset, FString& OutError)
	{
		if (!Asset || !Asset->GetOutermost())
		{
			OutError = TEXT("Save 대상 Asset 또는 package가 null입니다.");
			return false;
		}

		UPackage* Package = Asset->GetOutermost();
		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(),
			FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		Asset->MarkPackageDirty();
		if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArgs))
		{
			OutError = FString::Printf(TEXT("Package save 실패: %s"), *Filename);
			return false;
		}

		return true;
	}

	// Save 도중 하나라도 실패하면 5개 package disk bytes를 전부 pre-commit 상태로 복원합니다.
	bool RestoreBackups(const TArray<FPackageBackup>& Backups, FString& OutError)
	{
		bool bAllRestored = true;
		for (const FPackageBackup& Backup : Backups)
		{
			if (!FFileHelper::SaveArrayToFile(Backup.Bytes, *Backup.Filename))
			{
				bAllRestored = false;
				OutError += FString::Printf(TEXT("Rollback failed: %s | "), *Backup.Filename);
			}
		}
		return bAllRestored;
	}

	// 5개 authoring package를 모두 저장하고 실패 시 disk bytes 전체를 원복합니다.
	bool SaveAllWithRollback(const TArray<FPackageBackup>& Backups, FString& OutError)
	{
		for (const FPackageBackup& Backup : Backups)
		{
			FString SaveError;
			if (!SaveExistingAsset(Backup.Asset, SaveError))
			{
				FString RollbackError;
				const bool bRollbackOk = RestoreBackups(Backups, RollbackError);
				OutError = bRollbackOk
					? FString::Printf(TEXT("%s / 5-package rollback 완료"), *SaveError)
					: FString::Printf(TEXT("%s / rollback failure: %s"), *SaveError, *RollbackError);
				return false;
			}
		}
		return true;
	}

	// Browser에서 exact actual Wagon managed row를 찾습니다.
	const FCFVehicleListEntry* FindWagonEntry(const TArray<FCFVehicleListEntry>& Entries)
	{
		return Entries.FindByPredicate([](const FCFVehicleListEntry& Entry)
		{
			return Entry.RecipePath.ToString() == WagonRecipePath
				&& Entry.DefinitionPath.ToString() == WagonTargetPath;
		});
	}

	// Core four Profile fingerprints를 snapshot합니다.
	bool BuildProfileSnapshot(
		UCFVehicleBaseProfile* Base,
		UCFDrivetrainProfile* Drivetrain,
		UCFHandlingProfile* Handling,
		UCFPerformanceProfile* Performance,
		FCFVehicleProfileSnapshotSet& OutSnapshot,
		FString& OutError)
	{
		return FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
			Base,
			Drivetrain,
			Handling,
			Performance,
			nullptr,
			OutSnapshot,
			OutError);
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFBuilderWagonPhysicsCommandlet::UCFBuilderWagonPhysicsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// Current Reference를 USER-reviewed token으로 재승인한 뒤 PhysicsDraft v3 5500 proposal을 Preview/Commit하고 5개 authoring package만 저장합니다.
int32 UCFBuilderWagonPhysicsCommandlet::Main(const FString& Params)
{
	(void)Params;
	using namespace CFBuilderWagonPhysics;

	FString Error;

	// Existing Builder Shell ViewModel입니다. Selection/Reference token/Physics Preview+Commit contract를 그대로 사용합니다.
	FCFVehicleBuilderVM Builder;
	if (!Builder.RefreshVehicles(Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_BROWSER_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	const FCFVehicleListEntry* WagonEntry = FindWagonEntry(Builder.GetVehicleEntries());
	if (!WagonEntry || !Builder.SelectVehicle(*WagonEntry, Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_SELECTION_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	UCFVehicleRecipeData* Recipe = Builder.GetRecipe();
	UCFVehicleData* Target = Recipe ? Recipe->TargetVehicleData.LoadSynchronous() : nullptr;
	UCFVehicleRefEvidence* Evidence = Builder.GetCurrentReferenceEvidence();
	if (!Recipe || !Target || !Evidence
		|| FSoftObjectPath(Recipe).ToString() != WagonRecipePath
		|| FSoftObjectPath(Target).ToString() != WagonTargetPath
		|| FSoftObjectPath(Evidence).ToString() != WagonEvidencePath)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_BINDING_FAIL"));
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	if (Recipe->BuilderTransmissionPolicy != ECFBuilderTransmissionPolicy::VehicleSpecificRequired)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_POLICY_FAIL current=%d"), static_cast<int32>(Recipe->BuilderTransmissionPolicy));
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	// Evidence Refresh 뒤 current exact fingerprint를 USER가 이번 Review에서 승인한 Step 1 token으로 기록합니다.
	if (!Builder.AcceptCurrentReferenceSet(Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_REFERENCE_REVIEW_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::ReferenceReviewFailed);
	}

	// USER-approved PhysicsDraft v3를 current exact Evidence/Recipe/Profile authority에 binding해 읽습니다.
	if (!Builder.LoadPhysicsProposalDraft(Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_DRAFT_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::DraftFailed);
	}

	const FCFBuilderPhysicsDraft& Draft = Builder.GetLoadedPhysicsProposalDraft();
	if (Draft.SchemaRevision != 3
		|| Draft.ProfilePayload.DrivetrainData.TransmissionRatios.ForwardGearRatios.Num() != 8
		|| Draft.ProfilePayload.DrivetrainData.TransmissionRatios.ReverseGearRatios.Num() != 1
		|| !FMath::IsNearlyEqual(Draft.ProfilePayload.DrivetrainData.FinalRatio, 3.20f, KINDA_SMALL_NUMBER)
		|| Draft.ProfilePayload.DrivetrainData.ChangeUpRPM != 5500
		|| Draft.ProfilePayload.DrivetrainData.ChangeDownRPM != 2000
		|| !Draft.ProfilePayload.PerformanceData.bUseEngineTorqueCurve
		|| Draft.ProfilePayload.PerformanceData.EngineTorqueCurve.Points.Num() != 6)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_DRAFT_VALUE_FAIL"));
		return static_cast<int32>(EExitCode::DraftFailed);
	}

	// Existing Step 5 mutation0 preview입니다.
	FCFBuilderProfileCommitPreview Preview;
	if (!Builder.PreparePhysicsProposal(Preview, Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_PREVIEW_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::PreviewFailed);
	}

	if (Preview.Operation.Status == ECFAuthoringOpStatus::NoChange)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_UNEXPECTED_NOCHANGE"));
		return static_cast<int32>(EExitCode::PreviewFailed);
	}
	if (Preview.TransmissionProposalHash != ExpectedTransmissionProposalHash
		|| Preview.TransmissionDiagnostic.Blockers.Num() != 0
		|| Preview.TransmissionDiagnostic.Gears.Num() != 8
		|| !Preview.TransmissionDiagnostic.Gears[0].bShiftSpeedAvailable
		|| Preview.TransmissionDiagnostic.Gears[0].ShiftSpeedMaxKmh < 50.0f
		|| Preview.TransmissionDiagnostic.Gears[0].ShiftSpeedMaxKmh > 53.0f)
	{
		UE_LOG(
			LogCFBuilderWagonPhysics,
			Error,
			TEXT("CF_BUILDER_WAGON_PHYSICS_PREVIEW_CONTRACT_FAIL hash=%s blockers=%d gears=%d first_shift=%.3f"),
			*Preview.TransmissionProposalHash,
			Preview.TransmissionDiagnostic.Blockers.Num(),
			Preview.TransmissionDiagnostic.Gears.Num(),
			Preview.TransmissionDiagnostic.Gears.IsEmpty() ? -1.0f : Preview.TransmissionDiagnostic.Gears[0].ShiftSpeedMaxKmh);
		return static_cast<int32>(EExitCode::PreviewFailed);
	}

	// Current persistent private 4 Profile입니다.
	UCFVehicleBaseProfile* Base = Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
	UCFDrivetrainProfile* Drivetrain = Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	UCFHandlingProfile* Handling = Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
	UCFPerformanceProfile* Performance = Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	if (!Base || !Drivetrain || !Handling || !Performance)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_PROFILE_LOAD_FAIL"));
		return static_cast<int32>(EExitCode::PreviewFailed);
	}

	// Persistent mutation 전 Target/Evidence/Profile/Recipe baseline입니다.
	FCFVehicleDefinitionSnapshot TargetBefore;
	FCFVehicleProfileSnapshotSet ProfilesBefore;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, Error)
		|| !BuildProfileSnapshot(Base, Drivetrain, Handling, Performance, ProfilesBefore, Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_BASELINE_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::PreviewFailed);
	}
	const FString EvidenceFingerprintBefore = Evidence->EvidenceFingerprint;
	const int32 EvidenceRevisionBefore = Evidence->AuthoringRevision;
	const int32 RecipeRevisionBefore = Recipe->AuthoringRevision;
	const int32 BaseRevisionBefore = Base->Meta.AuthoringRevision;
	const int32 DrivetrainRevisionBefore = Drivetrain->Meta.AuthoringRevision;
	const int32 HandlingRevisionBefore = Handling->Meta.AuthoringRevision;
	const int32 PerformanceRevisionBefore = Performance->Meta.AuthoringRevision;

	// 이번 transition은 현재 Engine Curve 적용 Target + 이전 4500 private Drivetrain receipt에서만 허용합니다.
	if (TargetBefore.DefinitionHash != ExpectedCurrentTargetHash
		|| Target->VehicleMovementConfig.ChangeUpRPM != 4500
		|| Target->VehicleMovementConfig.ChangeDownRPM != 2000
		|| Target->VehicleMovementConfig.TransmissionRatios.ForwardGearRatios.Num() != 8
		|| !FMath::IsNearlyEqual(Target->VehicleMovementConfig.FinalRatio, 3.20f, KINDA_SMALL_NUMBER)
		|| !Target->VehicleMovementConfig.bUseEngineTorqueCurve
		|| Drivetrain->Data.ChangeUpRPM != 4500
		|| Drivetrain->Data.ChangeDownRPM != 2000
		|| Recipe->BuilderCommitReceipt.TransmissionProposalHash != ExpectedPreviousTransmissionProposalHash
		|| Recipe->BuilderCommitReceipt.EngineCurveProposalHash != ExpectedEngineCurveProposalHash)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_PRESTATE_FAIL target=%s target_up=%.1f profile_up=%.1f transmission_hash=%s engine_hash=%s"),
			*TargetBefore.DefinitionHash,
			Target->VehicleMovementConfig.ChangeUpRPM,
			Drivetrain->Data.ChangeUpRPM,
			*Recipe->BuilderCommitReceipt.TransmissionProposalHash,
			*Recipe->BuilderCommitReceipt.EngineCurveProposalHash);
		return static_cast<int32>(EExitCode::PreviewFailed);
	}

	// Save failure rollback용 exact five package disk bytes입니다.
	TArray<FPackageBackup> Backups;
	Backups.SetNum(5);
	if (!BackupPackage(Base, Backups[0], Error)
		|| !BackupPackage(Drivetrain, Backups[1], Error)
		|| !BackupPackage(Handling, Backups[2], Error)
		|| !BackupPackage(Performance, Backups[3], Error)
		|| !BackupPackage(Recipe, Backups[4], Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_BACKUP_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::PreviewFailed);
	}

	// USER가 직전 대화에서 승인한 exact Preview에 AuthoringWrite approval을 붙여 existing Builder facade로 commit합니다.
	FCFAuthoringOpResult CommitResult;
	if (!Builder.ExecutePreparedPhysicsProposal(CommitResult, Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_COMMIT_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::CommitFailed);
	}

	// Commit 후 Target/Evidence는 불변이고 4 Profile/Recipe revision과 receipt는 새 proposal에 맞아야 합니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	FCFVehicleProfileSnapshotSet ProfilesAfter;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, Error)
		|| !BuildProfileSnapshot(Base, Drivetrain, Handling, Performance, ProfilesAfter, Error)
		|| TargetAfter.DefinitionHash != TargetBefore.DefinitionHash
		|| Evidence->EvidenceFingerprint != EvidenceFingerprintBefore
		|| Evidence->AuthoringRevision != EvidenceRevisionBefore
		|| Recipe->AuthoringRevision != RecipeRevisionBefore + 1
		|| Base->Meta.AuthoringRevision != BaseRevisionBefore + 1
		|| Drivetrain->Meta.AuthoringRevision != DrivetrainRevisionBefore + 1
		|| Handling->Meta.AuthoringRevision != HandlingRevisionBefore + 1
		|| Performance->Meta.AuthoringRevision != PerformanceRevisionBefore + 1
		|| !Recipe->BuilderCommitReceipt.IsValid()
		|| Recipe->BuilderCommitReceipt.TransmissionPolicy != ECFBuilderTransmissionPolicy::VehicleSpecificRequired
		|| Recipe->BuilderCommitReceipt.TransmissionProposalHash != ExpectedTransmissionProposalHash
		|| Recipe->BuilderCommitReceipt.EngineCurveProposalHash != ExpectedEngineCurveProposalHash
		|| Recipe->BuilderCommitReceipt.EvidenceFingerprint != EvidenceFingerprintBefore
		|| ProfilesAfter.BaseSource.ProfileFingerprint != Preview.ProspectiveFingerprints.VehicleBaseFingerprint
		|| ProfilesAfter.DrivetrainSource.ProfileFingerprint != Preview.ProspectiveFingerprints.DrivetrainFingerprint
		|| ProfilesAfter.HandlingSource.ProfileFingerprint != Preview.ProspectiveFingerprints.HandlingFingerprint
		|| ProfilesAfter.PerformanceSource.ProfileFingerprint != Preview.ProspectiveFingerprints.PerformanceFingerprint)
	{
		UE_LOG(
			LogCFBuilderWagonPhysics,
			Error,
			TEXT("CF_BUILDER_WAGON_PHYSICS_INVARIANT_FAIL target=%s/%s recipe_rev=%d->%d evidence=%s"),
			*TargetBefore.DefinitionHash,
			*TargetAfter.DefinitionHash,
			RecipeRevisionBefore,
			Recipe->AuthoringRevision,
			*Evidence->EvidenceFingerprint);
		return static_cast<int32>(EExitCode::InvariantFailed);
	}

	// Profile commit은 Target Apply가 아니므로 current post-EngineCurve Target이 byte-semantic상 그대로여야 합니다.
	if (TargetAfter.DefinitionHash != ExpectedCurrentTargetHash
		|| Target->VehicleMovementConfig.TransmissionRatios.ForwardGearRatios.Num() != 8
		|| !FMath::IsNearlyEqual(Target->VehicleMovementConfig.FinalRatio, 3.20f, KINDA_SMALL_NUMBER)
		|| Target->VehicleMovementConfig.ChangeUpRPM != 4500
		|| Target->VehicleMovementConfig.ChangeDownRPM != 2000
		|| !Target->VehicleMovementConfig.bUseEngineTorqueCurve
		|| Target->VehicleMovementConfig.EngineTorqueCurve.Points.Num() != 6)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_TARGET_MUTATED_EARLY target=%s up=%.1f down=%.1f"),
			*TargetAfter.DefinitionHash,
			Target->VehicleMovementConfig.ChangeUpRPM,
			Target->VehicleMovementConfig.ChangeDownRPM);
		return static_cast<int32>(EExitCode::InvariantFailed);
	}

	// Commit된 Drivetrain private Profile은 exact 8-speed proposal이어야 합니다.
	if (!Drivetrain->Data.bUseTransmissionConfig
		|| Drivetrain->Data.TransmissionRatios.ForwardGearRatios.Num() != 8
		|| !FMath::IsNearlyEqual(Drivetrain->Data.FinalRatio, 3.20f, KINDA_SMALL_NUMBER)
		|| Drivetrain->Data.ChangeUpRPM != 5500
		|| Drivetrain->Data.ChangeDownRPM != 2000)
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_DRIVETRAIN_READBACK_FAIL"));
		return static_cast<int32>(EExitCode::InvariantFailed);
	}

	if (!SaveAllWithRollback(Backups, Error))
	{
		UE_LOG(LogCFBuilderWagonPhysics, Error, TEXT("CF_BUILDER_WAGON_PHYSICS_SAVE_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFBuilderWagonPhysics,
		Display,
		TEXT("CF_BUILDER_WAGON_PHYSICS_PASS proposal=%s first_shift=%.3f target_hash=%s recipe_revision=%d->%d drivetrain_revision=%d->%d saved_packages=5 target_saved=false"),
		*Preview.TransmissionProposalHash,
		Preview.TransmissionDiagnostic.Gears[0].ShiftSpeedMaxKmh,
		*TargetAfter.DefinitionHash,
		RecipeRevisionBefore,
		Recipe->AuthoringRevision,
		DrivetrainRevisionBefore,
		Drivetrain->Meta.AuthoringRevision);

	return static_cast<int32>(EExitCode::Success);
}
