// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonPolicyCmdlet.cpp
// Version: v1.0.0
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon의 BuilderTransmissionPolicy one-time migration 구현입니다.
// Changelog:
// - v1.0.0: exact Recipe/Target/Profile preflight, policy-only write, AuthoringRevision +1, receipt unchanged, Target/Profile invariant, rollback save를 최초 구현.
// Migration:
// - Runtime VehicleData / WSA / Drivetrain payload를 수정하지 않습니다.
// - Receipt의 old LegacyCompatible state를 보존해 fresh VehicleSpecificRequired commit을 강제합니다.

#include "CFBuilderWagonPolicyCmdlet.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFBuilderWagonPolicy, Log, All);

namespace CFBuilderWagonPolicy
{
	// actual Wagon Recipe exact object path입니다.
	const TCHAR* WagonRecipeObjectPath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");

	// actual Wagon Target exact object path입니다.
	const TCHAR* WagonTargetObjectPath = TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon");

	// Current bound Wagon private Profile exact object paths입니다.
	const TCHAR* WagonBaseProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_VB_Wagon.DA_VB_Wagon");
	const TCHAR* WagonDrivetrainProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_DT_Wagon.DA_DT_Wagon");
	const TCHAR* WagonHandlingProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_HDL_Wagon.DA_HDL_Wagon");
	const TCHAR* WagonPerformanceProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_PRF_Wagon.DA_PRF_Wagon");

	// Process exit code taxonomy입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		AssetLoadFailed = 71,
		PreflightFailed = 72,
		InvariantFailed = 73,
		SaveFailed = 74
	};

	// Recipe package를 disk bytes로 backup합니다.
	bool BackupRecipe(UCFVehicleRecipeData* Recipe, FString& OutFilename, TArray<uint8>& OutBytes, FString& OutError)
	{
		if (!Recipe || !Recipe->GetOutermost())
		{
			OutError = TEXT("Recipe 또는 package가 null입니다.");
			return false;
		}

		OutFilename = FPackageName::LongPackageNameToFilename(
			Recipe->GetOutermost()->GetName(),
			FPackageName::GetAssetPackageExtension());

		if (!FFileHelper::LoadFileToArray(OutBytes, *OutFilename))
		{
			OutError = FString::Printf(TEXT("Recipe backup read 실패: %s"), *OutFilename);
			return false;
		}

		return true;
	}

	// Recipe 한 package만 저장하고 실패 시 pre-migration bytes로 복원합니다.
	bool SaveRecipeWithRollback(
		UCFVehicleRecipeData* Recipe,
		const FString& Filename,
		const TArray<uint8>& OriginalBytes,
		FString& OutError)
	{
		if (!Recipe || !Recipe->GetOutermost())
		{
			OutError = TEXT("Recipe save 대상이 null입니다.");
			return false;
		}

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		Recipe->MarkPackageDirty();
		if (UPackage::SavePackage(Recipe->GetOutermost(), Recipe, *Filename, SaveArgs))
		{
			return true;
		}

		if (!FFileHelper::SaveArrayToFile(OriginalBytes, *Filename))
		{
			OutError = FString::Printf(TEXT("Recipe save 실패 후 rollback도 실패했습니다: %s"), *Filename);
			return false;
		}

		OutError = TEXT("Recipe save 실패 / pre-migration bytes rollback 완료");
		return false;
	}

	// Four core private Profiles를 resolver payload fingerprint set으로 snapshot합니다.
	bool BuildCoreProfileSnapshot(
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

	// Profile payload fingerprints가 policy migration 전후 완전히 동일한지 판정합니다.
	bool ProfileFingerprintsEqual(
		const FCFVehicleProfileSnapshotSet& A,
		const FCFVehicleProfileSnapshotSet& B)
	{
		return A.BaseSource.ProfileFingerprint == B.BaseSource.ProfileFingerprint
			&& A.DrivetrainSource.ProfileFingerprint == B.DrivetrainSource.ProfileFingerprint
			&& A.HandlingSource.ProfileFingerprint == B.HandlingSource.ProfileFingerprint
			&& A.PerformanceSource.ProfileFingerprint == B.PerformanceSource.ProfileFingerprint;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFBuilderWagonPolicyCommandlet::UCFBuilderWagonPolicyCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// LegacyCompatible actual Wagon Recipe를 VehicleSpecificRequired로 전환하고 non-policy authority가 불변인지 검증합니다.
int32 UCFBuilderWagonPolicyCommandlet::Main(const FString& Params)
{
	(void)Params;
	using namespace CFBuilderWagonPolicy;

	FString Error;

	UCFVehicleRecipeData* Recipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipeObjectPath);
	UCFVehicleData* Target = LoadObject<UCFVehicleData>(nullptr, WagonTargetObjectPath);
	UCFVehicleBaseProfile* Base = LoadObject<UCFVehicleBaseProfile>(nullptr, WagonBaseProfilePath);
	UCFDrivetrainProfile* Drivetrain = LoadObject<UCFDrivetrainProfile>(nullptr, WagonDrivetrainProfilePath);
	UCFHandlingProfile* Handling = LoadObject<UCFHandlingProfile>(nullptr, WagonHandlingProfilePath);
	UCFPerformanceProfile* Performance = LoadObject<UCFPerformanceProfile>(nullptr, WagonPerformanceProfilePath);

	if (!Recipe || !Target || !Base || !Drivetrain || !Handling || !Performance)
	{
		UE_LOG(LogCFBuilderWagonPolicy, Error, TEXT("CF_BUILDER_WAGON_POLICY_LOAD_FAIL"));
		return static_cast<int32>(EExitCode::AssetLoadFailed);
	}

	if (Recipe->BuilderTransmissionPolicy != ECFBuilderTransmissionPolicy::LegacyCompatible)
	{
		UE_LOG(
			LogCFBuilderWagonPolicy,
			Error,
			TEXT("CF_BUILDER_WAGON_POLICY_PREFLIGHT_FAIL current_policy=%d expected=LegacyCompatible"),
			static_cast<int32>(Recipe->BuilderTransmissionPolicy));
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	if (Recipe->BuilderCommitReceipt.TransmissionPolicy != ECFBuilderTransmissionPolicy::LegacyCompatible
		|| !Recipe->BuilderCommitReceipt.TransmissionProposalHash.IsEmpty()
		|| Recipe->BuilderCommitReceipt.TransmissionReview.IsPresent())
	{
		UE_LOG(LogCFBuilderWagonPolicy, Error, TEXT("CF_BUILDER_WAGON_POLICY_PREFLIGHT_FAIL existing receipt is already transmission-specific"));
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	const int32 RevisionBefore = Recipe->AuthoringRevision;
	const FCFVehicleBuilderCommitReceipt ReceiptBefore = Recipe->BuilderCommitReceipt;

	FCFVehicleDefinitionSnapshot TargetBefore;
	FCFVehicleProfileSnapshotSet ProfilesBefore;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, Error)
		|| !BuildCoreProfileSnapshot(Base, Drivetrain, Handling, Performance, ProfilesBefore, Error))
	{
		UE_LOG(LogCFBuilderWagonPolicy, Error, TEXT("CF_BUILDER_WAGON_POLICY_SNAPSHOT_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	FString RecipeFilename;
	TArray<uint8> RecipeBytes;
	if (!BackupRecipe(Recipe, RecipeFilename, RecipeBytes, Error))
	{
		UE_LOG(LogCFBuilderWagonPolicy, Error, TEXT("CF_BUILDER_WAGON_POLICY_BACKUP_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	Recipe->Modify();
	Recipe->BuilderTransmissionPolicy = ECFBuilderTransmissionPolicy::VehicleSpecificRequired;
	++Recipe->AuthoringRevision;
	Recipe->MarkPackageDirty();
	Recipe->PostEditChange();

	FCFVehicleDefinitionSnapshot TargetAfter;
	FCFVehicleProfileSnapshotSet ProfilesAfter;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, Error)
		|| !BuildCoreProfileSnapshot(Base, Drivetrain, Handling, Performance, ProfilesAfter, Error))
	{
		UE_LOG(LogCFBuilderWagonPolicy, Error, TEXT("CF_BUILDER_WAGON_POLICY_POST_SNAPSHOT_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::InvariantFailed);
	}

	if (TargetAfter.DefinitionHash != TargetBefore.DefinitionHash
		|| !ProfileFingerprintsEqual(ProfilesBefore, ProfilesAfter)
		|| Recipe->AuthoringRevision != RevisionBefore + 1
		|| Recipe->BuilderCommitReceipt.ProposalHash != ReceiptBefore.ProposalHash
		|| Recipe->BuilderCommitReceipt.TransmissionPolicy != ReceiptBefore.TransmissionPolicy
		|| Recipe->BuilderCommitReceipt.TransmissionProposalHash != ReceiptBefore.TransmissionProposalHash)
	{
		UE_LOG(
			LogCFBuilderWagonPolicy,
			Error,
			TEXT("CF_BUILDER_WAGON_POLICY_INVARIANT_FAIL target_before=%s target_after=%s revision_before=%d revision_after=%d"),
			*TargetBefore.DefinitionHash,
			*TargetAfter.DefinitionHash,
			RevisionBefore,
			Recipe->AuthoringRevision);
		return static_cast<int32>(EExitCode::InvariantFailed);
	}

	if (!SaveRecipeWithRollback(Recipe, RecipeFilename, RecipeBytes, Error))
	{
		UE_LOG(LogCFBuilderWagonPolicy, Error, TEXT("CF_BUILDER_WAGON_POLICY_SAVE_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFBuilderWagonPolicy,
		Display,
		TEXT("CF_BUILDER_WAGON_POLICY_PASS revision=%d->%d target_hash=%s drivetrain_fingerprint=%s receipt_policy_preserved=%d saved_packages=1"),
		RevisionBefore,
		Recipe->AuthoringRevision,
		*TargetAfter.DefinitionHash,
		*ProfilesAfter.DrivetrainSource.ProfileFingerprint,
		static_cast<int32>(Recipe->BuilderCommitReceipt.TransmissionPolicy));

	return static_cast<int32>(EExitCode::Success);
}
