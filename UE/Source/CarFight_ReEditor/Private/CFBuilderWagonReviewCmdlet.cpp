// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonReviewCmdlet.cpp
// Version: v1.5.2
// Date: 2026-09-02
// Description: CF-FQ-040 actual Wagon ESH-03 ChangeUpRPM 5500 exact Final Review/DefinitionApply persistent lane을 current Builder VM contract로 실행합니다.
// Changelog:
// - v1.5.2: pre-Apply TargetHash 83c...가 Engine Curve Apply 완료 후 5500 Shift Apply 직전 상태라는 실제 의미에 맞춰 stale 주석을 교정. guard/실행 동작은 변경 없음.
// - v1.5.1: discovery에서 확정한 DiffHash 79f3..., DefinitionApply ProposalHash 226e..., prospective TargetHash 0e5b...를 exact guard로 무장. pre 4500→5500 Diff1 / post Diff0를 모두 검증하고 existing Builder R3 Apply만 허용.
// - v1.5.0: current Transmission receipt를 5500 proposal hash로 갱신하고, pre-Target 83c...에서 exact ChangeUpRPM 4500→5500 Diff1만 허용하는 review-only discovery gate를 추가. 새 Diff/Apply/Post hash 확정 전 persistent Apply mode는 명시적으로 차단.
// - v1.4.1: Builder VM의 정상 post-Apply 계약(PrepareFinalReviewApply=false + bCanCompleteFinalReview=true)을 terminal PASS로 해석하고 pre-Apply에서만 prepared Apply 성공을 강제.
// - v1.4.0: review-only R0가 exact pre-Apply readiness와 exact post-Apply completion 상태를 모두 fail-closed로 판정하도록 확장. post-Apply에서는 Diff0/ExternalDrift0/AppliedState/6-point Target Curve를 검증.
// - v1.3.0: historical 8AT Apply guard를 current ESH-02 Engine Curve exact2 proposal/diff/pre/post hash로 교체하고 post-Apply 6-point Curve exact invariant를 추가.
// - v1.2.0: review-only current receipt guard에 ESH-02 EngineCurveProposalHash exact binding과 EngineCurveDiagnostic blocker 검증을 추가.
// - v1.1.0: 승인 hash가 명시된 경우에만 exact two-diff Final Review를 재검사하고 existing ExecutePreparedFinalReviewApply를 호출한 뒤 Target+Recipe 2-package save/rollback 및 post-Apply Diff0 검증을 추가.
// - v1.0.0: existing Builder VM PrepareFinalReviewApply R0 경로, Target/Recipe invariant, FieldDiff/TransmissionDiagnostic JSON export를 최초 구현.
// Migration:
// - 승인 hash가 없으면 exact pre-Apply Review Ready 또는 exact post-Apply Complete를 mutation0으로 검증합니다.
// - 승인 hash가 `226ea77d192e1af864270ae1687bdbda`와 exact 일치할 때만 existing Builder R3 Apply를 실행하고 Target+Recipe 두 package만 저장합니다.

#include "CFBuilderWagonReviewCmdlet.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleBuilderVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFBuilderWagonReview, Log, All);

namespace CFBuilderWagonReview
{
	// Actual Wagon managed Recipe exact object path입니다.
	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");

	// Actual Wagon Target exact object path입니다.
	const TCHAR* WagonTargetPath = TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon");

	// Expected current vehicle-specific Transmission proposal identity입니다.
	const TCHAR* ExpectedTransmissionProposalHash = TEXT("e1be2562d77fb1f5a993d6e7f5d17962");

	// USER-approved ESH-02 Engine Curve proposal identity입니다.
	const TCHAR* ExpectedEngineCurveProposalHash = TEXT("773221966499c6295f2a652a21b270a5");

	// USER가 fresh Final Review에서 승인한 ChangeUpRPM 5500 DefinitionApply proposal identity입니다.
	const TCHAR* ExpectedDefinitionApplyProposalHash = TEXT("226ea77d192e1af864270ae1687bdbda");

	// USER가 승인한 ChangeUpRPM 4500→5500 exact one-row Diff identity입니다.
	const TCHAR* ExpectedDiffHash = TEXT("79f3a25eb4a50202f8a284c26db941e8");

	// Engine Curve Apply는 이미 완료됐고, ChangeUpRPM 4500→5500 Shift Apply 직전인 exact Target Definition hash입니다.
	const TCHAR* ExpectedPreApplyTargetHash = TEXT("83c69e52522dc72649c32477b5aad220");

	// 승인된 ChangeUpRPM 5500 DefinitionApply 뒤 prospective Target Definition hash입니다.
	const TCHAR* ExpectedPostApplyTargetHash = TEXT("0e5b48e8dcd39deba441da9237218be6");

	// Process exit code taxonomy입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		SelectionFailed = 101,
		ReviewFailed = 102,
		InvariantFailed = 103,
		OutputFailed = 104,
		ApplyFailed = 105,
		SaveFailed = 106
	};

	// 하나의 existing package disk bytes backup입니다.
	struct FPackageBackup
	{
		// Save 대상 Asset object입니다.
		UObject* Asset = nullptr;

		// Existing .uasset exact filename입니다.
		FString Filename;

		// Apply 전 exact disk bytes입니다.
		TArray<uint8> Bytes;
	};

	// Asset package를 Apply 전 exact disk bytes로 backup합니다.
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

	// Save 도중 하나라도 실패하면 Target+Recipe disk bytes를 둘 다 pre-Apply 상태로 복원합니다.
	bool SaveBothWithRollback(const TArray<FPackageBackup>& Backups, FString& OutError)
	{
		for (const FPackageBackup& Backup : Backups)
		{
			FString SaveError;
			if (SaveExistingAsset(Backup.Asset, SaveError))
			{
				continue;
			}

			bool bRollbackOk = true;
			FString RollbackError;
			for (const FPackageBackup& Restore : Backups)
			{
				if (!FFileHelper::SaveArrayToFile(Restore.Bytes, *Restore.Filename))
				{
					bRollbackOk = false;
					RollbackError += FString::Printf(TEXT("%s | "), *Restore.Filename);
				}
			}
			OutError = bRollbackOk
				? FString::Printf(TEXT("%s / Target+Recipe rollback 완료"), *SaveError)
				: FString::Printf(TEXT("%s / rollback failure: %s"), *SaveError, *RollbackError);
			return false;
		}
		return true;
	}

	// Discovery Final Review가 ChangeUpRPM 4500→5500 exact one-row Diff인지 검사합니다.
	bool IsExpectedChangeUpDiscoveryDiff(const FCFBuilderFinalReviewResult& Review)
	{
		if (Review.FieldDiff.Num() != 1)
		{
			return false;
		}

		// 이번 USER-approved mutation의 유일한 canonical diff row입니다.
		const FCFVehicleFieldDiff& Diff = Review.FieldDiff[0];
		if (Diff.Operation != ECFVehicleDiffOp::SetLeaf
			|| Diff.FieldPath.ToCanonicalString(true) != TEXT("VehicleMovementConfig.ChangeUpRPM")
			|| !Diff.bHasBeforeValue
			|| !Diff.bHasAfterValue)
		{
			return false;
		}

		// Canonical float text에서 읽은 current 4500 값입니다.
		float BeforeChangeUpRpm = 0.0f;
		// Canonical float text에서 읽은 proposed 5500 값입니다.
		float AfterChangeUpRpm = 0.0f;
		return LexTryParseString(BeforeChangeUpRpm, *Diff.BeforeValue.CanonicalValueText)
			&& LexTryParseString(AfterChangeUpRpm, *Diff.AfterValue.CanonicalValueText)
			&& FMath::IsNearlyEqual(BeforeChangeUpRpm, 4500.0f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(AfterChangeUpRpm, 5500.0f, KINDA_SMALL_NUMBER);
	}

	// USER-approved ChangeUpRPM 5500 exact Diff/proposal/post-target identity를 검사합니다.
	bool IsExactApprovedDiff(const FCFBuilderFinalReviewResult& Review)
	{
		return IsExpectedChangeUpDiscoveryDiff(Review)
			&& Review.DiffHash == ExpectedDiffHash
			&& Review.ApplyProposal.ProposalHash == ExpectedDefinitionApplyProposalHash
			&& Review.ApplyProposal.ProspectiveResolvedDefinitionHash == ExpectedPostApplyTargetHash;
	}

	// Apply 뒤 Target이 USER-approved 6-point Engine Curve를 exact하게 소유하는지 검사합니다.
	bool HasExpectedEngineCurve(const FCFVehicleMovementConfig& MovementConfig)
	{
		// Apply 뒤 Target에 저장된 Engine Curve point 배열입니다.
		const TArray<FCFVehicleEngineTorquePoint>& Points = MovementConfig.EngineTorqueCurve.Points;
		if (!MovementConfig.bUseEngineTorqueCurve || Points.Num() != 6)
		{
			return false;
		}

		return FMath::IsNearlyEqual(Points[0].EngineRPM, 900.0f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[0].TorqueMultiplier, 0.60f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[1].EngineRPM, 1800.0f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[1].TorqueMultiplier, 1.00f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[2].EngineRPM, 4800.0f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[2].TorqueMultiplier, 1.00f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[3].EngineRPM, 5400.0f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[3].TorqueMultiplier, 0.929033824f, 0.000001f)
			&& FMath::IsNearlyEqual(Points[4].EngineRPM, 5700.0f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[4].TorqueMultiplier, 0.880137307f, 0.000001f)
			&& FMath::IsNearlyEqual(Points[5].EngineRPM, 6500.0f, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Points[5].TorqueMultiplier, 0.65f, KINDA_SMALL_NUMBER);
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
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFBuilderWagonReviewCommandlet::UCFBuilderWagonReviewCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// Current Builder authority에서 exact Wagon Final Review를 읽고 Saved diagnostic JSON만 기록합니다.
int32 UCFBuilderWagonReviewCommandlet::Main(const FString& Params)
{
	using namespace CFBuilderWagonReview;

	// 승인 hash가 비어 있으면 기존 R0 review-only, 값이 있으면 exact approved Apply mode입니다.
	FString ApprovedApplyHash;
	FParse::Value(*Params, TEXT("CFWagonApplyApprovedHash="), ApprovedApplyHash);
	const bool bApplyRequested = !ApprovedApplyHash.IsEmpty();

	FString Error;

	// Existing Guided Builder ViewModel입니다.
	FCFVehicleBuilderVM Builder;
	if (!Builder.RefreshVehicles(Error))
	{
		UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_REVIEW_BROWSER_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	// Exact actual Wagon managed row입니다.
	const FCFVehicleListEntry* WagonEntry = FindWagonEntry(Builder.GetVehicleEntries());
	if (!WagonEntry || !Builder.SelectVehicle(*WagonEntry, Error))
	{
		UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_REVIEW_SELECTION_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	// Review owner Recipe입니다.
	UCFVehicleRecipeData* Recipe = Builder.GetRecipe();
	// Review Target입니다.
	UCFVehicleData* Target = Recipe ? Recipe->TargetVehicleData.LoadSynchronous() : nullptr;
	if (!Recipe || !Target
		|| FSoftObjectPath(Recipe).ToString() != WagonRecipePath
		|| FSoftObjectPath(Target).ToString() != WagonTargetPath)
	{
		UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_REVIEW_BINDING_FAIL"));
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	if (Recipe->BuilderTransmissionPolicy != ECFBuilderTransmissionPolicy::VehicleSpecificRequired
		|| !Recipe->BuilderCommitReceipt.IsValid()
		|| Recipe->BuilderCommitReceipt.TransmissionProposalHash != ExpectedTransmissionProposalHash
		|| Recipe->BuilderCommitReceipt.EngineCurveProposalHash != ExpectedEngineCurveProposalHash)
	{
		UE_LOG(
			LogCFBuilderWagonReview,
			Error,
			TEXT("CF_BUILDER_WAGON_REVIEW_RECEIPT_FAIL policy=%d receipt_valid=%s transmission_hash=%s engine_hash=%s"),
			static_cast<int32>(Recipe->BuilderTransmissionPolicy),
			Recipe->BuilderCommitReceipt.IsValid() ? TEXT("true") : TEXT("false"),
			*Recipe->BuilderCommitReceipt.TransmissionProposalHash,
			*Recipe->BuilderCommitReceipt.EngineCurveProposalHash);
		return static_cast<int32>(EExitCode::ReviewFailed);
	}

	// Review 전 full Target Definition snapshot입니다.
	FCFVehicleDefinitionSnapshot TargetBefore;
	// Review 전 semantic Recipe snapshot입니다.
	FCFVehicleRecipeSnapshot RecipeBefore;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, Error)
		|| !FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, RecipeBefore, Error))
	{
		UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_REVIEW_BASELINE_FAIL %s"), *Error);
		return static_cast<int32>(EExitCode::ReviewFailed);
	}

	// 현재 Target이 exact pre/post Apply state인지 구분합니다.
	const bool bIsExpectedPreApplyState = TargetBefore.DefinitionHash == ExpectedPreApplyTargetHash;
	const bool bIsExpectedPostApplyState = TargetBefore.DefinitionHash == ExpectedPostApplyTargetHash;
	// pre에서는 4500, post에서는 5500이어야 하며 나머지 drivetrain/Engine Curve는 동일해야 합니다.
	const bool bExpectedShiftValue = (bIsExpectedPreApplyState && Target->VehicleMovementConfig.ChangeUpRPM == 4500)
		|| (bIsExpectedPostApplyState && Target->VehicleMovementConfig.ChangeUpRPM == 5500);
	if ((!bIsExpectedPreApplyState && !bIsExpectedPostApplyState)
		|| !bExpectedShiftValue
		|| Target->VehicleMovementConfig.ChangeDownRPM != 2000
		|| Target->VehicleMovementConfig.TransmissionRatios.ForwardGearRatios.Num() != 8
		|| Target->VehicleMovementConfig.TransmissionRatios.ReverseGearRatios.Num() != 1
		|| !FMath::IsNearlyEqual(Target->VehicleMovementConfig.FinalRatio, 3.20f, KINDA_SMALL_NUMBER)
		|| !HasExpectedEngineCurve(Target->VehicleMovementConfig))
	{
		UE_LOG(
			LogCFBuilderWagonReview,
			Error,
			TEXT("CF_BUILDER_WAGON_REVIEW_TARGET_STATE_FAIL target=%s expected_pre=%s expected_post=%s up=%.1f down=%.1f curve=%s"),
			*TargetBefore.DefinitionHash,
			ExpectedPreApplyTargetHash,
			ExpectedPostApplyTargetHash,
			Target->VehicleMovementConfig.ChangeUpRPM,
			Target->VehicleMovementConfig.ChangeDownRPM,
			HasExpectedEngineCurve(Target->VehicleMovementConfig) ? TEXT("true") : TEXT("false"));
		return static_cast<int32>(EExitCode::ReviewFailed);
	}

	// Existing Builder Step 7 fresh mutation0 review입니다. pre에서는 prepared Apply가 성공하고 post Complete에서는 false+complete가 정상입니다.
	FCFBuilderFinalReviewResult Review;
	// Builder VM이 실제 DefinitionApply prepared state를 만들었는지 여부입니다.
	const bool bPrepareSucceeded = Builder.PrepareFinalReviewApply(Review, Error);
	if (!bPrepareSucceeded
		&& (!bIsExpectedPostApplyState || !Review.bCanCompleteFinalReview))
	{
		UE_LOG(
			LogCFBuilderWagonReview,
			Error,
			TEXT("CF_BUILDER_WAGON_REVIEW_PREPARE_FAIL blockers=%d diff=%d complete=%s message=%s"),
			Review.BlockingIssueCount,
			Review.FieldDiff.Num(),
			Review.bCanCompleteFinalReview ? TEXT("true") : TEXT("false"),
			*Error);
		return static_cast<int32>(EExitCode::ReviewFailed);
	}

	if (bIsExpectedPreApplyState)
	{
		// pre-state는 USER-approved ChangeUpRPM 4500→5500 exact Diff1/proposal을 제공해야 합니다.
		if (!Review.bCanApply
			|| !Review.bApplyRequired
			|| Review.bCanCompleteFinalReview
			|| Review.BlockingIssueCount != 0
			|| Review.bHasExternalDrift
			|| Review.ApplyProposal.bSavePerformed
			|| !IsExactApprovedDiff(Review))
		{
			UE_LOG(
				LogCFBuilderWagonReview,
				Error,
				TEXT("CF_BUILDER_WAGON_REVIEW_PREAPPLY_FAIL can_apply=%s apply_required=%s complete=%s blockers=%d drift=%s diff=%d apply_hash=%s diff_hash=%s prospective=%s auto_save=%s"),
				Review.bCanApply ? TEXT("true") : TEXT("false"),
				Review.bApplyRequired ? TEXT("true") : TEXT("false"),
				Review.bCanCompleteFinalReview ? TEXT("true") : TEXT("false"),
				Review.BlockingIssueCount,
				Review.bHasExternalDrift ? TEXT("true") : TEXT("false"),
				Review.FieldDiff.Num(),
				*Review.ApplyProposal.ProposalHash,
				*Review.DiffHash,
				*Review.ApplyProposal.ProspectiveResolvedDefinitionHash,
				Review.ApplyProposal.bSavePerformed ? TEXT("true") : TEXT("false"));
			return static_cast<int32>(EExitCode::ReviewFailed);
		}
	}
	else
	{
		// post-state는 Diff0 / blocker0 / drift0 / exact AppliedState이며 Final Review Complete여야 합니다.
		if (Review.bCanApply
			|| Review.bApplyRequired
			|| !Review.bCanCompleteFinalReview
			|| Review.BlockingIssueCount != 0
			|| Review.bHasExternalDrift
			|| !Review.FieldDiff.IsEmpty()
			|| Review.ApplyProposal.bSavePerformed
			|| Recipe->AppliedState.AppliedDefinitionHash != ExpectedPostApplyTargetHash
			|| Recipe->AppliedState.ResolverContractRevision != 5)
		{
			UE_LOG(
				LogCFBuilderWagonReview,
				Error,
				TEXT("CF_BUILDER_WAGON_REVIEW_POSTAPPLY_FAIL can_apply=%s apply_required=%s complete=%s blockers=%d drift=%s diff=%d applied_hash=%s applied_revision=%d auto_save=%s"),
				Review.bCanApply ? TEXT("true") : TEXT("false"),
				Review.bApplyRequired ? TEXT("true") : TEXT("false"),
				Review.bCanCompleteFinalReview ? TEXT("true") : TEXT("false"),
				Review.BlockingIssueCount,
				Review.bHasExternalDrift ? TEXT("true") : TEXT("false"),
				Review.FieldDiff.Num(),
				*Recipe->AppliedState.AppliedDefinitionHash,
				Recipe->AppliedState.ResolverContractRevision,
				Review.ApplyProposal.bSavePerformed ? TEXT("true") : TEXT("false"));
			return static_cast<int32>(EExitCode::ReviewFailed);
		}
	}

	if (!Review.TransmissionDiagnostic.bEvaluated
		|| !Review.TransmissionDiagnostic.bVehicleSpecificRequired
		|| !Review.TransmissionDiagnostic.Blockers.IsEmpty()
		|| Review.TransmissionDiagnostic.Gears.Num() != 8)
	{
		UE_LOG(
			LogCFBuilderWagonReview,
			Error,
			TEXT("CF_BUILDER_WAGON_REVIEW_TRANSMISSION_FAIL evaluated=%s vehicle_specific=%s blockers=%d gears=%d"),
			Review.TransmissionDiagnostic.bEvaluated ? TEXT("true") : TEXT("false"),
			Review.TransmissionDiagnostic.bVehicleSpecificRequired ? TEXT("true") : TEXT("false"),
			Review.TransmissionDiagnostic.Blockers.Num(),
			Review.TransmissionDiagnostic.Gears.Num());
		return static_cast<int32>(EExitCode::ReviewFailed);
	}

	// ESH-02 persistent Engine Curve receipt도 current Performance/Evidence에 fresh binding되어 blocker 0이어야 합니다.
	if (!Review.EngineCurveDiagnostic.bEvaluated
		|| !Review.EngineCurveDiagnostic.bVehicleSpecificCurveEnabled
		|| Review.EngineCurveDiagnostic.EngineCurveProposalHash != ExpectedEngineCurveProposalHash
		|| !Review.EngineCurveDiagnostic.Blockers.IsEmpty())
	{
		UE_LOG(
			LogCFBuilderWagonReview,
			Error,
			TEXT("CF_BUILDER_WAGON_REVIEW_ENGINE_FAIL evaluated=%s enabled=%s hash=%s blockers=%d"),
			Review.EngineCurveDiagnostic.bEvaluated ? TEXT("true") : TEXT("false"),
			Review.EngineCurveDiagnostic.bVehicleSpecificCurveEnabled ? TEXT("true") : TEXT("false"),
			*Review.EngineCurveDiagnostic.EngineCurveProposalHash,
			Review.EngineCurveDiagnostic.Blockers.Num());
		return static_cast<int32>(EExitCode::ReviewFailed);
	}

	// Review 뒤 Target/Recipe snapshots입니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	FCFVehicleRecipeSnapshot RecipeAfter;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, Error)
		|| !FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, RecipeAfter, Error)
		|| TargetAfter.DefinitionHash != TargetBefore.DefinitionHash
		|| RecipeAfter.RecipeFingerprint != RecipeBefore.RecipeFingerprint)
	{
		UE_LOG(
			LogCFBuilderWagonReview,
			Error,
			TEXT("CF_BUILDER_WAGON_REVIEW_MUTATION_FAIL target=%s/%s recipe=%s/%s"),
			*TargetBefore.DefinitionHash,
			*TargetAfter.DefinitionHash,
			*RecipeBefore.RecipeFingerprint,
			*RecipeAfter.RecipeFingerprint);
		return static_cast<int32>(EExitCode::InvariantFailed);
	}

	if (bApplyRequested)
	{
		// USER approval은 current exact proposal/diff/pre-state와 모두 일치해야 합니다.
		if (ApprovedApplyHash != ExpectedDefinitionApplyProposalHash
			|| TargetBefore.DefinitionHash != ExpectedPreApplyTargetHash
			|| !IsExactApprovedDiff(Review))
		{
			UE_LOG(
				LogCFBuilderWagonReview,
				Error,
				TEXT("CF_BUILDER_WAGON_APPLY_SCOPE_FAIL approved=%s current_apply=%s current_diff=%s target=%s"),
				*ApprovedApplyHash,
				*Review.ApplyProposal.ProposalHash,
				*Review.DiffHash,
				*TargetBefore.DefinitionHash);
			return static_cast<int32>(EExitCode::ApplyFailed);
		}

		// Apply 전 Target/Recipe exact disk bytes입니다.
		TArray<FPackageBackup> Backups;
		Backups.SetNum(2);
		if (!BackupPackage(Target, Backups[0], Error)
			|| !BackupPackage(Recipe, Backups[1], Error))
		{
			UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_APPLY_BACKUP_FAIL %s"), *Error);
			return static_cast<int32>(EExitCode::ApplyFailed);
		}

		// Existing Builder R3 DefinitionApply facade를 exact prepared approval에 대해 실행합니다.
		FCFBuilderFinalApplyResult ApplyResult;
		if (!Builder.ExecutePreparedFinalReviewApply(ApplyResult, Error))
		{
			UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_APPLY_EXEC_FAIL %s"), *Error);
			return static_cast<int32>(EExitCode::ApplyFailed);
		}

		// Apply 뒤 exact Target/semantic Recipe snapshots입니다.
		FCFVehicleDefinitionSnapshot AppliedTarget;
		FCFVehicleRecipeSnapshot AppliedRecipe;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, AppliedTarget, Error)
			|| !FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, AppliedRecipe, Error)
			|| AppliedTarget.DefinitionHash != ExpectedPostApplyTargetHash
			|| AppliedRecipe.RecipeFingerprint != RecipeBefore.RecipeFingerprint
			|| !ApplyResult.Operation.Mutation.bTargetChanged
			|| !ApplyResult.Operation.Mutation.bRecipeChanged
			|| !HasExpectedEngineCurve(Target->VehicleMovementConfig)
			|| !Target->VehicleMovementConfig.TransmissionRatios.ForwardGearRatios.IsValidIndex(7)
			|| Target->VehicleMovementConfig.TransmissionRatios.ForwardGearRatios.Num() != 8
			|| Target->VehicleMovementConfig.TransmissionRatios.ReverseGearRatios.Num() != 1
			|| !FMath::IsNearlyEqual(Target->VehicleMovementConfig.FinalRatio, 3.20f, KINDA_SMALL_NUMBER)
			|| !FMath::IsNearlyEqual(Target->VehicleMovementConfig.TransmissionRatios.ReverseGearRatios[0], 3.992f, KINDA_SMALL_NUMBER)
			|| Target->VehicleMovementConfig.ChangeUpRPM != 5500
			|| Target->VehicleMovementConfig.ChangeDownRPM != 2000)
		{
			UE_LOG(
				LogCFBuilderWagonReview,
				Error,
				TEXT("CF_BUILDER_WAGON_APPLY_POSTSTATE_FAIL target=%s expected=%s recipe=%s/%s target_changed=%s recipe_changed=%s"),
				*AppliedTarget.DefinitionHash,
				ExpectedPostApplyTargetHash,
				*RecipeBefore.RecipeFingerprint,
				*AppliedRecipe.RecipeFingerprint,
				ApplyResult.Operation.Mutation.bTargetChanged ? TEXT("true") : TEXT("false"),
				ApplyResult.Operation.Mutation.bRecipeChanged ? TEXT("true") : TEXT("false"));
			return static_cast<int32>(EExitCode::InvariantFailed);
		}

		// ExecutePreparedFinalReviewApply 내부 fresh refresh가 Diff0 / Complete를 만들었는지 확인합니다.
		const FCFBuilderFinalReviewResult& PostReview = Builder.GetFinalReviewResult();
		if (!PostReview.bCanCompleteFinalReview
			|| PostReview.bApplyRequired
			|| PostReview.bCanApply
			|| PostReview.BlockingIssueCount != 0
			|| PostReview.bHasExternalDrift
			|| !PostReview.FieldDiff.IsEmpty())
		{
			UE_LOG(
				LogCFBuilderWagonReview,
				Error,
				TEXT("CF_BUILDER_WAGON_APPLY_POSTREVIEW_FAIL complete=%s apply_required=%s can_apply=%s blockers=%d drift=%s diff=%d"),
				PostReview.bCanCompleteFinalReview ? TEXT("true") : TEXT("false"),
				PostReview.bApplyRequired ? TEXT("true") : TEXT("false"),
				PostReview.bCanApply ? TEXT("true") : TEXT("false"),
				PostReview.BlockingIssueCount,
				PostReview.bHasExternalDrift ? TEXT("true") : TEXT("false"),
				PostReview.FieldDiff.Num());
			return static_cast<int32>(EExitCode::InvariantFailed);
		}

		// Exact post-state 검증 뒤 Target+Recipe 두 package만 persistent 저장합니다.
		if (!SaveBothWithRollback(Backups, Error))
		{
			UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_APPLY_SAVE_FAIL %s"), *Error);
			return static_cast<int32>(EExitCode::SaveFailed);
		}

		// Persistent Apply evidence는 pre-Apply review artifact와 분리해 기록합니다.
		FString ApplyJson;
		FJsonObjectConverter::UStructToJsonObjectString(ApplyResult, ApplyJson);
		const FString ApplyResultPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
			FPaths::ProjectSavedDir(), TEXT("CarFight"), TEXT("VehicleBuilder"), TEXT("WagonFinalApply.json")));
		const FString ApplySummaryPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
			FPaths::ProjectSavedDir(), TEXT("CarFight"), TEXT("VehicleBuilder"), TEXT("WagonFinalApply.txt")));
		const FString ApplySummary = Builder.BuildFinalReviewSummary();
		if (!FFileHelper::SaveStringToFile(ApplyJson, *ApplyResultPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)
			|| !FFileHelper::SaveStringToFile(ApplySummary, *ApplySummaryPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			UE_LOG(LogCFBuilderWagonReview, Warning, TEXT("CF_BUILDER_WAGON_APPLY_DIAGNOSTIC_WRITE_WARN"));
		}

		UE_LOG(
			LogCFBuilderWagonReview,
			Display,
			TEXT("CF_BUILDER_WAGON_APPLY_PASS apply_hash=%s pre_target=%s post_target=%s post_diff=%d saved_target=true saved_recipe=true undo_available=%s"),
			*ApprovedApplyHash,
			*TargetBefore.DefinitionHash,
			*AppliedTarget.DefinitionHash,
			PostReview.FieldDiff.Num(),
			ApplyResult.bUndoAvailable ? TEXT("true") : TEXT("false"));
		return static_cast<int32>(EExitCode::Success);
	}

	// Machine-readable complete Final Review result입니다.
	FString ReviewJson;
	if (!FJsonObjectConverter::UStructToJsonObjectString(Review, ReviewJson))
	{
		UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_REVIEW_JSON_FAIL"));
		return static_cast<int32>(EExitCode::OutputFailed);
	}

	// USER-readable Builder summary도 함께 남깁니다.
	const FString ReviewSummary = Builder.BuildFinalReviewSummary();

	// Project Saved diagnostic path입니다.
	const FString ResultPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("WagonFinalReview.json")));

	// JSON string 안에 summary를 이중 escape하는 대신 separate companion txt를 씁니다.
	const FString SummaryPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("WagonFinalReview.txt")));

	if (!FFileHelper::SaveStringToFile(
			ReviewJson,
			*ResultPath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)
		|| !FFileHelper::SaveStringToFile(
			ReviewSummary,
			*SummaryPath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogCFBuilderWagonReview, Error, TEXT("CF_BUILDER_WAGON_REVIEW_OUTPUT_FAIL"));
		return static_cast<int32>(EExitCode::OutputFailed);
	}

	UE_LOG(
		LogCFBuilderWagonReview,
		Display,
		TEXT("CF_BUILDER_WAGON_REVIEW_PASS diff=%d warnings=%d blockers=%d diff_hash=%s apply_hash=%s target_hash=%s mutation=false save=false"),
		Review.FieldDiff.Num(),
		Review.WarningCount,
		Review.BlockingIssueCount,
		*Review.DiffHash,
		*Review.ApplyProposal.ProposalHash,
		*TargetAfter.DefinitionHash);

	return static_cast<int32>(EExitCode::Success);
}
