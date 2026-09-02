// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonEvidenceCmdlet.cpp
// Version: v1.0.0
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon Reference Evidence persistent refresh one-shot 구현입니다.
// Changelog:
// - v1.0.0: canonical ResearchDraft schema/binding 검증, R1 preview→AuthoringWrite commit, Evidence-only save/rollback, identity/revision/fingerprint/Target/Profile invariant 검증을 최초 구현.
// Migration:
// - Product Target/Recipe/Profile/Transmission payload는 변경하지 않습니다.
// - Evidence identity/ownership binding은 existing owner contract가 보존하며 research payload와 fingerprint/revision만 갱신합니다.

#include "CFBuilderWagonEvidenceCmdlet.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFBuilderWagonEvidence, Log, All);

namespace CFBuilderWagonEvidence
{
	// Actual Wagon persistent object paths입니다.
	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");
	const TCHAR* WagonEvidencePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_Ref_Wagon.DA_Ref_Wagon");
	const TCHAR* WagonBaseProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_VB_Wagon.DA_VB_Wagon");
	const TCHAR* WagonDrivetrainProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_DT_Wagon.DA_DT_Wagon");
	const TCHAR* WagonHandlingProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_HDL_Wagon.DA_HDL_Wagon");
	const TCHAR* WagonPerformanceProfilePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_PRF_Wagon.DA_PRF_Wagon");

	// Process exit code taxonomy입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		AssetLoadFailed = 81,
		DraftInvalid = 82,
		PreviewFailed = 83,
		CommitFailed = 84,
		InvariantFailed = 85,
		SaveFailed = 86
	};

	// Core four Profile payload snapshot을 생성합니다.
	bool BuildCoreProfiles(
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

	// Core four Profile fingerprints가 동일한지 판정합니다.
	bool ProfileFingerprintsEqual(const FCFVehicleProfileSnapshotSet& A, const FCFVehicleProfileSnapshotSet& B)
	{
		return A.BaseSource.ProfileFingerprint == B.BaseSource.ProfileFingerprint
			&& A.DrivetrainSource.ProfileFingerprint == B.DrivetrainSource.ProfileFingerprint
			&& A.HandlingSource.ProfileFingerprint == B.HandlingSource.ProfileFingerprint
			&& A.PerformanceSource.ProfileFingerprint == B.PerformanceSource.ProfileFingerprint;
	}

	// Evidence package의 현재 disk bytes를 backup합니다.
	bool BackupEvidence(UCFVehicleRefEvidence* Evidence, FString& OutFilename, TArray<uint8>& OutBytes, FString& OutError)
	{
		if (!Evidence || !Evidence->GetOutermost())
		{
			OutError = TEXT("Evidence 또는 package가 null입니다.");
			return false;
		}

		OutFilename = FPackageName::LongPackageNameToFilename(
			Evidence->GetOutermost()->GetName(),
			FPackageName::GetAssetPackageExtension());

		if (!FFileHelper::LoadFileToArray(OutBytes, *OutFilename))
		{
			OutError = FString::Printf(TEXT("Evidence backup read 실패: %s"), *OutFilename);
			return false;
		}

		return true;
	}

	// Evidence package 하나만 저장하고 실패 시 pre-refresh bytes로 복원합니다.
	bool SaveEvidenceWithRollback(
		UCFVehicleRefEvidence* Evidence,
		const FString& Filename,
		const TArray<uint8>& OriginalBytes,
		FString& OutError)
	{
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		Evidence->MarkPackageDirty();
		if (UPackage::SavePackage(Evidence->GetOutermost(), Evidence, *Filename, SaveArgs))
		{
			return true;
		}

		if (!FFileHelper::SaveArrayToFile(OriginalBytes, *Filename))
		{
			OutError = FString::Printf(TEXT("Evidence save 실패 후 rollback도 실패했습니다: %s"), *Filename);
			return false;
		}

		OutError = TEXT("Evidence save 실패 / pre-refresh bytes rollback 완료");
		return false;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFBuilderWagonEvidenceCommandlet::UCFBuilderWagonEvidenceCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// Canonical Saved ResearchDraft를 reviewed R1 Evidence Refresh로 commit하고 Evidence package 하나만 저장합니다.
int32 UCFBuilderWagonEvidenceCommandlet::Main(const FString& Params)
{
	(void)Params;
	using namespace CFBuilderWagonEvidence;

	FString Error;

	UCFVehicleRecipeData* Recipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipePath);
	UCFVehicleRefEvidence* Evidence = LoadObject<UCFVehicleRefEvidence>(nullptr, WagonEvidencePath);
	UCFVehicleBaseProfile* Base = LoadObject<UCFVehicleBaseProfile>(nullptr, WagonBaseProfilePath);
	UCFDrivetrainProfile* Drivetrain = LoadObject<UCFDrivetrainProfile>(nullptr, WagonDrivetrainProfilePath);
	UCFHandlingProfile* Handling = LoadObject<UCFHandlingProfile>(nullptr, WagonHandlingProfilePath);
	UCFPerformanceProfile* Performance = LoadObject<UCFPerformanceProfile>(nullptr, WagonPerformanceProfilePath);
	UCFVehicleData* Target = Recipe ? Recipe->TargetVehicleData.LoadSynchronous() : nullptr;

	if (!Recipe || !Evidence || !Base || !Drivetrain || !Handling || !Performance || !Target)
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_LOAD_FAIL"));
		return static_cast<int32>(EExitCode::AssetLoadFailed);
	}

	if (Recipe->BuilderTransmissionPolicy != ECFBuilderTransmissionPolicy::VehicleSpecificRequired)
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_POLICY_FAIL policy=%d"), static_cast<int32>(Recipe->BuilderTransmissionPolicy));
		return static_cast<int32>(EExitCode::DraftInvalid);
	}

	// Canonical AI ResearchDraft path입니다.
	const FString ResearchDraftPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("ResearchDraft.json")));

	FString ResearchDraftJson;
	FCFBuilderResearchDraft ResearchDraft;
	if (!FFileHelper::LoadFileToString(ResearchDraftJson, *ResearchDraftPath)
		|| !FJsonObjectConverter::JsonObjectStringToUStruct(ResearchDraftJson, &ResearchDraft, 0, 0)
		|| ResearchDraft.SchemaRevision != 1
		|| ResearchDraft.RecipeId != Recipe->RecipeId
		|| ResearchDraft.TargetDefinitionPath != FSoftObjectPath(Target))
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_DRAFT_FAIL path=%s"), *ResearchDraftPath);
		return static_cast<int32>(EExitCode::DraftInvalid);
	}

	const FGuid EvidenceIdBefore = Evidence->EvidenceId;
	const FGuid TargetRecipeIdBefore = Evidence->TargetRecipeId;
	const FSoftObjectPath TargetRecipePathBefore = Evidence->TargetRecipePath;
	const FSoftObjectPath TargetDefinitionPathBefore = Evidence->TargetDefinitionPath;
	const FString EvidenceFingerprintBefore = Evidence->EvidenceFingerprint;
	const int32 EvidenceRevisionBefore = Evidence->AuthoringRevision;
	const int32 RecipeRevisionBefore = Recipe->AuthoringRevision;

	FCFVehicleDefinitionSnapshot TargetBefore;
	FCFVehicleProfileSnapshotSet ProfilesBefore;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, Error)
		|| !BuildCoreProfiles(Base, Drivetrain, Handling, Performance, ProfilesBefore, Error))
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_SNAPSHOT_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::DraftInvalid);
	}

	FString EvidenceFilename;
	TArray<uint8> EvidenceBytes;
	if (!BackupEvidence(Evidence, EvidenceFilename, EvidenceBytes, Error))
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_BACKUP_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::DraftInvalid);
	}

	FCFBuilderEvidenceRefreshRequest Request;
	Request.Recipe = Recipe;
	Request.EvidencePath = FSoftObjectPath(Evidence);
	Request.ExpectedCurrentEvidenceFingerprint = Evidence->EvidenceFingerprint;
	Request.EvidencePayload = ResearchDraft.EvidencePayload;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
	Request.CallContext.ClientOperationId = TEXT("VB-P0-09-Wagon-Evidence-Persistent-Preview");

	FCFBuilderEvidenceRefreshPreview Preview;
	if (!FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(Request, Preview))
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_PREVIEW_FAIL %s"), *Preview.Operation.Message);
		return static_cast<int32>(EExitCode::PreviewFailed);
	}

	if (Preview.Operation.Status == ECFAuthoringOpStatus::NoChange)
	{
		UE_LOG(
			LogCFBuilderWagonEvidence,
			Display,
			TEXT("CF_BUILDER_WAGON_EVIDENCE_NOCHANGE fingerprint=%s claims=%d unknowns=%d saved_packages=0"),
			*Preview.CurrentEvidenceFingerprint,
			Preview.CurrentClaimCount,
			Preview.CurrentUnknownFactCount);
		return static_cast<int32>(EExitCode::Success);
	}

	Request.CallContext.ClientOperationId = TEXT("VB-P0-09-Wagon-Evidence-Persistent-Commit");
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	Request.CallContext.ApprovalScopeHash = Preview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = Preview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = Preview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = Preview.Proposal.ResolverContractRevision;

	FCFBuilderEvidenceRefreshResult CommitResult;
	if (!FCFVehicleAuthoringService::CommitBuilderEvidenceRefresh(Request, Preview, CommitResult))
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_COMMIT_FAIL %s"), *CommitResult.Operation.Message);
		return static_cast<int32>(EExitCode::CommitFailed);
	}

	FCFVehicleDefinitionSnapshot TargetAfter;
	FCFVehicleProfileSnapshotSet ProfilesAfter;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, Error)
		|| !BuildCoreProfiles(Base, Drivetrain, Handling, Performance, ProfilesAfter, Error)
		|| TargetAfter.DefinitionHash != TargetBefore.DefinitionHash
		|| !ProfileFingerprintsEqual(ProfilesBefore, ProfilesAfter)
		|| Recipe->AuthoringRevision != RecipeRevisionBefore
		|| Evidence->EvidenceId != EvidenceIdBefore
		|| Evidence->TargetRecipeId != TargetRecipeIdBefore
		|| Evidence->TargetRecipePath != TargetRecipePathBefore
		|| Evidence->TargetDefinitionPath != TargetDefinitionPathBefore
		|| Evidence->EvidenceFingerprint != Preview.ProspectiveEvidenceFingerprint
		|| Evidence->AuthoringRevision != EvidenceRevisionBefore + 1)
	{
		UE_LOG(
			LogCFBuilderWagonEvidence,
			Error,
			TEXT("CF_BUILDER_WAGON_EVIDENCE_INVARIANT_FAIL target_before=%s target_after=%s recipe_revision=%d evidence_revision=%d->%d"),
			*TargetBefore.DefinitionHash,
			*TargetAfter.DefinitionHash,
			Recipe->AuthoringRevision,
			EvidenceRevisionBefore,
			Evidence->AuthoringRevision);
		return static_cast<int32>(EExitCode::InvariantFailed);
	}

	if (!SaveEvidenceWithRollback(Evidence, EvidenceFilename, EvidenceBytes, Error))
	{
		UE_LOG(LogCFBuilderWagonEvidence, Error, TEXT("CF_BUILDER_WAGON_EVIDENCE_SAVE_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFBuilderWagonEvidence,
		Display,
		TEXT("CF_BUILDER_WAGON_EVIDENCE_PASS old=%s new=%s claims=%d->%d unknowns=%d->%d evidence_revision=%d->%d target_hash=%s saved_packages=1"),
		*EvidenceFingerprintBefore,
		*Evidence->EvidenceFingerprint,
		Preview.CurrentClaimCount,
		Preview.ProspectiveClaimCount,
		Preview.CurrentUnknownFactCount,
		Preview.ProspectiveUnknownFactCount,
		EvidenceRevisionBefore,
		Evidence->AuthoringRevision,
		*TargetAfter.DefinitionHash);

	return static_cast<int32>(EExitCode::Success);
}
