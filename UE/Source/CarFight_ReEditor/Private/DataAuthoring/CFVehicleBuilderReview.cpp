// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBuilderReview.cpp
// Version: v1.2.0
// Date: 2026-08-27
// Description: CF-FQ-040 VB-P0-07 Final Review provenance receipt / one-resolve / post-state guarded Undo hardening 구현입니다.
// Scope: Existing Validation/External Drift/Gameplay/Diff/Reference Evidence를 aggregate하고 기존 R3 Apply lane과 Unreal standard Undo를 재사용합니다.
// Changelog:
// - v1.2.0: Final Review caller가 ConsumedClaimIds를 비운 resume read에서는 persistent BuilderCommitReceipt의 canonical Claim ID 목록을 사용하고, caller가 명시한 목록은 기존 hash exact-match로 계속 tamper 차단.
// - v1.1.0: provenance를 persistent BuilderCommitReceipt/current 4 Profile fingerprint에 binding하고, Final Review one-resolve projection과 Undo post-Apply state guard를 추가.
// - v1.0.0: ReadBuilderFinalReview / ApplyBuilderFinalReview / UndoBuilderFinalApply 최초 구현.
// Migration:
// - Target write는 기존 ApplyResolvedVehicle -> FCFVehicleApplyService만 사용합니다.
// - Hardpoint Socket 생성/이동, raw VehicleData write, auto Save, automatic retry를 추가하지 않습니다.
// - Provenance 수치는 consumed canonical Evidence Claim count이며 field count로 위장하지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "Editor.h"
#include "Editor/Transactor.h"
#include "Misc/SecureHash.h"

namespace CFVehicleBuilderReviewPrivate
{
	// Final Review R0 operation의 stable identity입니다.
	const FName ReviewOperationName(TEXT("ReadBuilderFinalReview"));

	// Final Review explicit Apply operation의 stable identity입니다.
	const FName ApplyOperationName(TEXT("ApplyBuilderFinalReview"));

	// Final Review guarded Undo operation의 stable identity입니다.
	const FName UndoOperationName(TEXT("UndoBuilderFinalApply"));

	// 이 Builder facade가 현재 Editor lifetime에서 실제 발급한 Undo transaction token record입니다.
	struct FTrackedUndoRecord
	{
		// Transaction owner Recipe identity입니다.
		FGuid RecipeId;

		// Transaction owner Recipe path입니다.
		FSoftObjectPath RecipePath;

		// Transaction owner Target path입니다.
		FSoftObjectPath TargetPath;

		// Token과 explicit approval이 공유하는 exact Undo scope hash입니다.
		FString UndoScopeHash;
	};

	// Builder Apply가 현재 lifetime에서 생성한 guarded Undo capability를 exact TransactionId로 보관합니다.
	TMap<FGuid, FTrackedUndoRecord> TrackedUndoRecords;

	// Common result를 지정한 operation/risk의 mutation0 기본값으로 초기화합니다.
	void InitializeOperation(
		FCFAuthoringOpResult& OutOperation,
		const FName OperationName,
		const ECFAuthoringRiskClass RiskClass,
		const FString& ClientOperationId = FString())
	{
		OutOperation = FCFAuthoringOpResult();
		OutOperation.OperationName = OperationName;
		OutOperation.RiskClass = RiskClass;
		OutOperation.Status = ECFAuthoringOpStatus::Blocked;
		OutOperation.ErrorCode = ECFAuthoringErrorCode::None;
		OutOperation.ClientOperationId = ClientOperationId;
		OutOperation.Mutation.bSavePerformed = false;
		OutOperation.Mutation.bAutomaticRetryPerformed = false;
	}

	// Common result를 mutation 없는 typed blocker로 종료합니다.
	bool Block(
		FCFAuthoringOpResult& OutOperation,
		const ECFAuthoringErrorCode ErrorCode,
		const FString& Message)
	{
		OutOperation.Status = ECFAuthoringOpStatus::Blocked;
		OutOperation.ErrorCode = ErrorCode;
		OutOperation.Message = Message;
		OutOperation.bRetryAllowed = false;
		return false;
	}

	// Common result를 success로 종료합니다.
	void Succeed(FCFAuthoringOpResult& OutOperation, const FString& Message)
	{
		OutOperation.Status = ECFAuthoringOpStatus::Succeeded;
		OutOperation.ErrorCode = ECFAuthoringErrorCode::None;
		OutOperation.Message = Message;
		OutOperation.bRetryAllowed = false;
	}

	// Hash payload에 delimiter-safe token을 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// Canonical UTF-8 payload를 existing Authoring approval convention과 같은 lowercase MD5로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR payload의 canonical UTF-8 byte representation입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Stable MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());

		// 최종 MD5 digest bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Lowercase hexadecimal digest입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// Lowercase hexadecimal digit table입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// Consumed Claim ID set을 order-independent deterministic hash로 만듭니다.
	FString BuildConsumedClaimIdHash(const TArray<FName>& ConsumedClaimIds)
	{
		// Deterministic lexical order를 만들 Claim ID 복사본입니다.
		TArray<FName> SortedClaimIds = ConsumedClaimIds;
		SortedClaimIds.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});

		// Delimiter-safe canonical Claim payload입니다.
		FString ClaimPayload;
		for (const FName ClaimId : SortedClaimIds)
		{
			AppendToken(ClaimPayload, TEXT("ClaimId"), ClaimId.ToString());
		}
		return HashUtf8Payload(ClaimPayload);
	}

	// Soft object path에서 exact Reference Evidence를 resolve/load합니다.
	UCFVehicleRefEvidence* LoadEvidence(const FSoftObjectPath& EvidencePath)
	{
		if (!EvidencePath.IsValid())
		{
			return nullptr;
		}

		// 이미 로드된 object 또는 path에서 load한 object입니다.
		UObject* LoadedObject = EvidencePath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = EvidencePath.TryLoad();
		}
		return Cast<UCFVehicleRefEvidence>(LoadedObject);
	}

	// Final Review provenance binding을 fresh Evidence에 대조하고 consumed canonical Claim provenance를 집계합니다.
	void BuildProvenanceSummary(
		const FCFBuilderFinalReviewRequest& Request,
		UCFVehicleRecipeData* Recipe,
		UCFVehicleData* TargetVehicleData,
		FCFBuilderProvenanceSummary& OutSummary,
		int32& OutWarningCount,
		int32& OutBlockingCount)
	{
		OutSummary = FCFBuilderProvenanceSummary();
		if (!Request.bHasEvidenceBinding)
		{
			OutSummary.IssueText = TEXT("Final Review에 latest accepted Builder Evidence binding이 없습니다. provenance를 추정하지 않고 새 Proposal/Review를 요구합니다.");
			++OutBlockingCount;
			return;
		}

		// Request가 가리키는 current Reference Evidence입니다.
		UCFVehicleRefEvidence* Evidence = LoadEvidence(Request.EvidenceBinding.EvidencePath);
		if (!Evidence)
		{
			OutSummary.IssueText = TEXT("Final Review Reference Evidence를 exact path에서 읽을 수 없습니다.");
			++OutBlockingCount;
			return;
		}

		if (!Recipe
			|| !TargetVehicleData
			|| !Request.EvidenceBinding.ExpectedEvidenceId.IsValid()
			|| Evidence->EvidenceId != Request.EvidenceBinding.ExpectedEvidenceId
			|| Evidence->TargetRecipeId != Recipe->RecipeId
			|| Evidence->TargetRecipePath != FSoftObjectPath(Recipe)
			|| Evidence->TargetDefinitionPath != FSoftObjectPath(TargetVehicleData))
		{
			OutSummary.IssueText = TEXT("Reference Evidence identity/Recipe/Definition binding이 current Final Review target과 일치하지 않습니다.");
			++OutBlockingCount;
			return;
		}

		// Current semantic Evidence fingerprint입니다.
		FString FreshEvidenceFingerprint;
		// Evidence fingerprint 계산 실패 diagnostic입니다.
		FString EvidenceError;
		if (!Evidence->BuildEvidenceFingerprint(FreshEvidenceFingerprint, EvidenceError)
			|| FreshEvidenceFingerprint.IsEmpty()
			|| FreshEvidenceFingerprint != Evidence->EvidenceFingerprint
			|| FreshEvidenceFingerprint != Request.EvidenceBinding.ExpectedEvidenceFingerprint)
		{
			OutSummary.IssueText = EvidenceError.IsEmpty()
				? TEXT("Reference Evidence fingerprint가 latest accepted Builder binding과 달라졌습니다. 새 Research/Proposal review가 필요합니다.")
				: EvidenceError;
			++OutBlockingCount;
			return;
		}

		// Current Recipe가 보유한 persistent Builder profile provenance receipt입니다.
		const FCFVehicleBuilderCommitReceipt& Receipt = Recipe->BuilderCommitReceipt;
		// Caller가 explicit Claim set을 제공하면 그것을 사용하고, resume read에서 비어 있으면 receipt의 persistent canonical 목록을 사용합니다.
		const TArray<FName>& EffectiveConsumedClaimIds = Request.EvidenceBinding.ConsumedClaimIds.IsEmpty()
			? Receipt.ConsumedClaimIds
			: Request.EvidenceBinding.ConsumedClaimIds;
		if (!Receipt.IsValid()
			|| Receipt.EvidencePath != Request.EvidenceBinding.EvidencePath
			|| Receipt.EvidenceId != Request.EvidenceBinding.ExpectedEvidenceId
			|| Receipt.EvidenceFingerprint != FreshEvidenceFingerprint
			|| Receipt.ConsumedClaimIdsHash != BuildConsumedClaimIdHash(EffectiveConsumedClaimIds)
			|| Receipt.ResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
		{
			OutSummary.IssueText = TEXT("Current Recipe의 Builder commit receipt가 이 Evidence/Claim set/Resolver revision을 증명하지 않습니다. Builder Profile proposal을 다시 preview/commit해야 합니다.");
			++OutBlockingCount;
			return;
		}

		// Current Recipe binding에서 load한 Builder-private VehicleBase Profile입니다.
		UCFVehicleBaseProfile* VehicleBaseProfile = Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
		// Current Recipe binding에서 load한 Builder-private Drivetrain Profile입니다.
		UCFDrivetrainProfile* DrivetrainProfile = Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
		// Current Recipe binding에서 load한 Builder-private Handling Profile입니다.
		UCFHandlingProfile* HandlingProfile = Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
		// Current Recipe binding에서 load한 Builder-private Performance Profile입니다.
		UCFPerformanceProfile* PerformanceProfile = Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
		if (!VehicleBaseProfile || !DrivetrainProfile || !HandlingProfile || !PerformanceProfile)
		{
			OutSummary.IssueText = TEXT("Builder commit receipt를 검증할 current private 4 Profile binding을 읽을 수 없습니다.");
			++OutBlockingCount;
			return;
		}

		// Current 4 Profile의 exact typed fingerprint snapshot입니다.
		FCFVehicleProfileSnapshotSet CurrentProfiles;
		// Current Profile fingerprint build diagnostic입니다.
		FString ProfileSnapshotError;
		if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
			VehicleBaseProfile,
			DrivetrainProfile,
			HandlingProfile,
			PerformanceProfile,
			nullptr,
			CurrentProfiles,
			ProfileSnapshotError)
			|| CurrentProfiles.BaseSource.OwnerRecipeId != Recipe->RecipeId
			|| CurrentProfiles.DrivetrainSource.OwnerRecipeId != Recipe->RecipeId
			|| CurrentProfiles.HandlingSource.OwnerRecipeId != Recipe->RecipeId
			|| CurrentProfiles.PerformanceSource.OwnerRecipeId != Recipe->RecipeId
			|| CurrentProfiles.BaseSource.ProfileFingerprint != Receipt.VehicleBaseFingerprint
			|| CurrentProfiles.DrivetrainSource.ProfileFingerprint != Receipt.DrivetrainFingerprint
			|| CurrentProfiles.HandlingSource.ProfileFingerprint != Receipt.HandlingFingerprint
			|| CurrentProfiles.PerformanceSource.ProfileFingerprint != Receipt.PerformanceFingerprint)
		{
			OutSummary.IssueText = ProfileSnapshotError.IsEmpty()
				? TEXT("Current private 4 Profile payload가 persistent Builder commit receipt와 달라졌습니다. provenance를 현재 값에 재사용할 수 없습니다.")
				: ProfileSnapshotError;
			++OutBlockingCount;
			return;
		}

		// Duplicate consumed Claim ID를 막는 stable set입니다.
		TSet<FName> ConsumedClaimIds;
		for (const FName ClaimId : EffectiveConsumedClaimIds)
		{
			if (ClaimId.IsNone() || ConsumedClaimIds.Contains(ClaimId))
			{
				OutSummary.IssueText = TEXT("ConsumedClaimIds에 None 또는 중복 ID가 있어 provenance를 신뢰할 수 없습니다.");
				++OutBlockingCount;
				return;
			}
			ConsumedClaimIds.Add(ClaimId);

			// Current Evidence의 exact consumed Claim입니다.
			const FCFRefClaim* Claim = Evidence->Claims.FindByPredicate([ClaimId](const FCFRefClaim& Candidate)
			{
				return Candidate.ClaimId == ClaimId;
			});
			if (!Claim || Claim->ResolutionState != ECFRefClaimResolution::Canonical)
			{
				OutSummary.IssueText = FString::Printf(TEXT("Consumed Claim이 current Evidence에서 Canonical이 아닙니다: %s"), *ClaimId.ToString());
				++OutBlockingCount;
				return;
			}

			++OutSummary.ConsumedClaimCount;
			switch (Claim->Provenance)
			{
			case ECFRefProvenance::FACT:
				++OutSummary.FactClaimCount;
				break;
			case ECFRefProvenance::DERIVED:
				++OutSummary.DerivedClaimCount;
				break;
			case ECFRefProvenance::GAME_BIAS:
				++OutSummary.GameBiasClaimCount;
				break;
			default:
				break;
			}
		}

		for (const FCFRefConflict& Conflict : Evidence->Conflicts)
		{
			if (Conflict.Severity == ECFRefConflictSeverity::Block)
			{
				++OutBlockingCount;
			}
			else if (Conflict.Severity == ECFRefConflictSeverity::Warning)
			{
				++OutWarningCount;
			}
		}
		for (const FCFRefUnknownFact& UnknownFact : Evidence->UnknownFacts)
		{
			if (UnknownFact.BlockingUse == ECFRefUnknownBlockingUse::ProposalBlock)
			{
				++OutBlockingCount;
			}
			else if (UnknownFact.BlockingUse == ECFRefUnknownBlockingUse::ProposalWarning)
			{
				++OutWarningCount;
			}
		}

		OutSummary.bAvailable = true;
		OutSummary.EvidenceFingerprint = FreshEvidenceFingerprint;
		OutSummary.IssueText.Reset();
	}

	// Current fresh Resolve result를 existing ApplyService request로 포장합니다.
	void BuildPreparedApplyRequest(
		const FCFBuilderFinalReviewRequest& Request,
		const FCFVehicleResolveReadResult& ResolveRead,
		UCFVehicleData* TargetVehicleData,
		FCFVehicleApplyRequest& OutApplyRequest)
	{
		OutApplyRequest = FCFVehicleApplyRequest();
		OutApplyRequest.Recipe = Request.GameplayRequest.ReadRequest.Recipe;
		OutApplyRequest.TargetVehicleData = TargetVehicleData;
		OutApplyRequest.ResolveRequest = ResolveRead.ResolveRequest;
		OutApplyRequest.ApprovedResolveResult = ResolveRead.ResolveResult;
		OutApplyRequest.ExpectedRecipeFingerprint = ResolveRead.ResolveRequest.Recipe.RecipeFingerprint;
		OutApplyRequest.ExpectedSourceSignature = ResolveRead.ResolveResult.SourceSignature;
		OutApplyRequest.ExpectedTargetDefinitionHash = ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash;
		OutApplyRequest.ExpectedResolvedDefinitionHash = ResolveRead.ResolveResult.ResolvedDefinitionHash;
		OutApplyRequest.ExpectedResolverContractRevision = ResolveRead.ResolveResult.ResolverContractRevision;
	}

	// Builder Apply가 발급할 guarded Undo token의 approval scope hash를 생성합니다.
	FString BuildUndoScopeHash(const FCFBuilderUndoToken& Token)
	{
		// Exact transaction/owner/pre-state에 binding할 canonical payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), UndoOperationName.ToString());
		AppendToken(Payload, TEXT("TransactionId"), Token.TransactionId.ToString(EGuidFormats::DigitsWithHyphensLower));
		AppendToken(Payload, TEXT("RecipeId"), Token.RecipeId.ToString(EGuidFormats::DigitsWithHyphensLower));
		AppendToken(Payload, TEXT("RecipePath"), Token.RecipePath.ToString());
		AppendToken(Payload, TEXT("TargetPath"), Token.TargetDefinitionPath.ToString());
		AppendToken(Payload, TEXT("ApplyRecipeFingerprint"), Token.ApplyRecipeFingerprint);
		AppendToken(Payload, TEXT("PreTargetHash"), Token.PreApplyTargetDefinitionHash);
		AppendToken(Payload, TEXT("PreAppliedRecipe"), Token.PreApplyAppliedRecipeFingerprint);
		AppendToken(Payload, TEXT("PreAppliedSource"), Token.PreApplyAppliedSourceSignature);
		AppendToken(Payload, TEXT("PreAppliedDefinition"), Token.PreApplyAppliedDefinitionHash);
		AppendToken(Payload, TEXT("PreAppliedRevision"), FString::FromInt(Token.PreApplyAppliedResolverRevision));
		AppendToken(Payload, TEXT("PostTargetHash"), Token.PostApplyTargetDefinitionHash);
		AppendToken(Payload, TEXT("PostAppliedRecipe"), Token.PostApplyAppliedRecipeFingerprint);
		AppendToken(Payload, TEXT("PostAppliedSource"), Token.PostApplyAppliedSourceSignature);
		AppendToken(Payload, TEXT("PostAppliedDefinition"), Token.PostApplyAppliedDefinitionHash);
		AppendToken(Payload, TEXT("PostAppliedRevision"), FString::FromInt(Token.PostApplyAppliedResolverRevision));
		return HashUtf8Payload(Payload);
	}

	// Builder Apply가 실제 생성한 transaction token을 bounded lifetime tracker에 등록합니다.
	void TrackUndoToken(const FCFBuilderUndoToken& Token)
	{
		if (TrackedUndoRecords.Num() >= 64)
		{
			// Stale capability가 무한 누적되지 않도록 bounded cache를 폐기합니다.
			TrackedUndoRecords.Reset();
		}

		// Exact transaction capability record입니다.
		FTrackedUndoRecord& Record = TrackedUndoRecords.Add(Token.TransactionId);
		Record.RecipeId = Token.RecipeId;
		Record.RecipePath = Token.RecipePath;
		Record.TargetPath = Token.TargetDefinitionPath;
		Record.UndoScopeHash = Token.UndoScopeHash;
	}
}

// Existing Validation/Drift/Gameplay/Diff/Evidence truth를 하나의 mutation0 Final Review로 집계합니다.
bool FCFVehicleAuthoringService::ReadBuilderFinalReview(
	const FCFBuilderFinalReviewRequest& Request,
	FCFBuilderFinalReviewResult& OutResult)
{
	OutResult = FCFBuilderFinalReviewResult();
	CFVehicleBuilderReviewPrivate::InitializeOperation(
		OutResult.Operation,
		CFVehicleBuilderReviewPrivate::ReviewOperationName,
		ECFAuthoringRiskClass::R0_ReadOnly);

	// Final Review의 authoritative Recipe입니다.
	UCFVehicleRecipeData* Recipe = Request.GameplayRequest.ReadRequest.Recipe;
	// Recipe binding 또는 explicit request의 authoritative Target입니다.
	UCFVehicleData* TargetVehicleData = Request.GameplayRequest.ReadRequest.TargetVehicleData;
	if (!Recipe)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::RecipeNotFound,
			TEXT("Builder Final Review에는 persistent managed Recipe가 필요합니다."));
	}
	if (!TargetVehicleData)
	{
		TargetVehicleData = Recipe->TargetVehicleData.LoadSynchronous();
	}
	if (!TargetVehicleData)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::TargetNotFound,
			TEXT("Builder Final Review의 Target VehicleData를 확인할 수 없습니다."));
	}

	// Shared Pure Resolver의 fresh current result입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(Request.GameplayRequest.ReadRequest, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = CFVehicleBuilderReviewPrivate::ReviewOperationName;
		OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
		return false;
	}

	// One fresh Resolve에서 기존 Validation projection을 그대로 구성합니다.
	OutResult.Validation.RecipeValidation = ResolveRead.ResolveResult.RecipeValidation;
	OutResult.Validation.ResolverValidation = ResolveRead.ResolveResult.ResolverValidation;
	OutResult.Validation.DefinitionValidation = ResolveRead.ResolveResult.DefinitionValidation;
	OutResult.Validation.StaleReport = ResolveRead.ResolveResult.StaleReport;
	OutResult.Validation.Operation = ResolveRead.Operation;
	OutResult.Validation.Operation.OperationName = TEXT("ReadValidation");
	OutResult.Validation.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Validation.Operation.Message = TEXT("Final Review가 one fresh Resolve에서 Shared Resolver validation layers를 projection했습니다.");

	// One fresh Resolve에서 기존 External Drift projection을 그대로 구성합니다.
	OutResult.Drift.StaleReport = ResolveRead.ResolveResult.StaleReport;
	OutResult.Drift.Operation = ResolveRead.Operation;
	OutResult.Drift.Operation.OperationName = TEXT("ReviewExternalDrift");
	OutResult.Drift.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Drift.Operation.Message = TEXT("Final Review가 one fresh Resolve에서 Shared R16 Stale/External Drift report를 projection했습니다.");

	if (!BuildBuilderGameplayGuidanceFromResolve(Request.GameplayRequest, ResolveRead, OutResult.GameplayGuidance))
	{
		OutResult.Operation = OutResult.GameplayGuidance.Operation;
		OutResult.Operation.OperationName = CFVehicleBuilderReviewPrivate::ReviewOperationName;
		OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
		return false;
	}

	OutResult.FieldDiff = ResolveRead.ResolveResult.FieldDiff;
	OutResult.DiffHash = BuildDiffHash(OutResult.FieldDiff);
	OutResult.WarningCount = OutResult.Validation.Operation.ValidationSummary.WarningCount;
	OutResult.BlockingIssueCount = OutResult.Validation.Operation.ValidationSummary.BlockedCount
		+ OutResult.Validation.Operation.ValidationSummary.ErrorCount
		+ OutResult.GameplayGuidance.NeedsReviewCount
		+ OutResult.GameplayGuidance.BlockedCount;

	CFVehicleBuilderReviewPrivate::BuildProvenanceSummary(
		Request,
		Recipe,
		TargetVehicleData,
		OutResult.Provenance,
		OutResult.WarningCount,
		OutResult.BlockingIssueCount);

	OutResult.bHasExternalDrift = OutResult.Drift.StaleReport.bHasExternalDrift;
	if (OutResult.bHasExternalDrift)
	{
		++OutResult.BlockingIssueCount;
	}

	OutResult.bApplyRequired = !OutResult.FieldDiff.IsEmpty();
	OutResult.bCanApply = OutResult.bApplyRequired
		&& OutResult.BlockingIssueCount == 0
		&& ResolveRead.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success;
	OutResult.bCanCompleteFinalReview = !OutResult.bApplyRequired
		&& OutResult.BlockingIssueCount == 0
		&& ResolveRead.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success;

	OutResult.Operation = ResolveRead.Operation;
	OutResult.Operation.OperationName = CFVehicleBuilderReviewPrivate::ReviewOperationName;
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Operation.CurrentDiffHash = OutResult.DiffHash;
	OutResult.Operation.ValidationSummary = OutResult.Validation.Operation.ValidationSummary;
	OutResult.Operation.Mutation = FCFAuthoringMutationFootprint();

	if (OutResult.bCanApply)
	{
		CFVehicleBuilderReviewPrivate::BuildPreparedApplyRequest(Request, ResolveRead, TargetVehicleData, OutResult.PreparedApplyRequest);
		// Existing R3 facade가 만드는 exact Apply approval proposal입니다.
		FCFAuthoringOpResult ProposalOperation;
		if (!BuildApplyApprovalProposal(OutResult.PreparedApplyRequest, OutResult.ApplyProposal, ProposalOperation))
		{
			OutResult.Operation = ProposalOperation;
			OutResult.Operation.OperationName = CFVehicleBuilderReviewPrivate::ReviewOperationName;
			OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
			return false;
		}
	}

	CFVehicleBuilderReviewPrivate::Succeed(
		OutResult.Operation,
		OutResult.bCanCompleteFinalReview
			? TEXT("Builder Final Review PASS입니다. Target Diff가 없어 다음 Driving Test로 진행할 수 있습니다.")
			: OutResult.bCanApply
				? FString::Printf(TEXT("Builder Final Review가 Apply 준비됐습니다. Warning %d / Blocker 0 / Target Diff %d이며 자동 저장은 하지 않습니다."), OutResult.WarningCount, OutResult.FieldDiff.Num())
				: FString::Printf(TEXT("Builder Final Review를 읽었습니다. Warning %d / Blocker %d / Target Diff %d입니다."), OutResult.WarningCount, OutResult.BlockingIssueCount, OutResult.FieldDiff.Num()));
	return true;
}

// Final Review exact approval을 fresh 재검사하고 existing ApplyResolvedVehicle lane으로 explicit Apply합니다.
bool FCFVehicleAuthoringService::ApplyBuilderFinalReview(
	const FCFBuilderFinalApplyRequest& Request,
	FCFBuilderFinalApplyResult& OutResult)
{
	OutResult = FCFBuilderFinalApplyResult();
	CFVehicleBuilderReviewPrivate::InitializeOperation(
		OutResult.Operation,
		CFVehicleBuilderReviewPrivate::ApplyOperationName,
		ECFAuthoringRiskClass::R3_DefinitionApply,
		Request.CallContext.ClientOperationId);

	if (Request.CallContext.ClientOperationId.IsEmpty()
		|| Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::DefinitionApply
		|| Request.CallContext.ApprovalScopeHash.IsEmpty())
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::ApprovalRequired,
			TEXT("Builder Apply에는 explicit ClientOperationId와 DefinitionApply ApprovalScopeHash가 필요합니다."));
	}

	// Apply 직전 fresh Final Review result입니다.
	FCFBuilderFinalReviewResult FreshReview;
	if (!ReadBuilderFinalReview(Request.ReviewRequest, FreshReview))
	{
		OutResult.Operation = FreshReview.Operation;
		OutResult.Operation.OperationName = CFVehicleBuilderReviewPrivate::ApplyOperationName;
		OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R3_DefinitionApply;
		OutResult.Operation.ClientOperationId = Request.CallContext.ClientOperationId;
		return false;
	}
	if (!FreshReview.bCanApply)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::ValidationBlocked,
			TEXT("Current Final Review가 Apply 가능한 상태가 아닙니다. blocker/drift/provenance/diff를 fresh 확인하세요."));
	}
	if (Request.CallContext.ApprovalScopeHash != FreshReview.ApplyProposal.ProposalHash)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::ApprovalScopeMismatch,
			TEXT("Builder Apply approval scope가 current fresh Final Review proposal과 일치하지 않습니다."));
	}

	// Apply 전 Undo 검증을 위해 보존할 Recipe입니다.
	UCFVehicleRecipeData* Recipe = FreshReview.PreparedApplyRequest.Recipe;
	// Apply 전 Undo 검증을 위해 보존할 Target입니다.
	UCFVehicleData* TargetVehicleData = FreshReview.PreparedApplyRequest.TargetVehicleData;
	if (!Recipe || !TargetVehicleData)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::ApplyPreconditionFailed,
			TEXT("Builder Apply의 current Recipe/Target identity를 확인할 수 없습니다."));
	}

	// Apply 전후 동일해야 하는 semantic Recipe fingerprint입니다.
	const FString ApplyRecipeFingerprint = FreshReview.PreparedApplyRequest.ExpectedRecipeFingerprint;
	// Apply 전 full Target hash입니다.
	const FString PreApplyTargetHash = FreshReview.PreparedApplyRequest.ExpectedTargetDefinitionHash;
	// Apply 전 AppliedState recipe fingerprint입니다.
	const FString PreAppliedRecipeFingerprint = Recipe->AppliedState.AppliedRecipeFingerprint;
	// Apply 전 AppliedState source signature입니다.
	const FString PreAppliedSourceSignature = Recipe->AppliedState.AppliedSourceSignature;
	// Apply 전 AppliedState definition hash입니다.
	const FString PreAppliedDefinitionHash = Recipe->AppliedState.AppliedDefinitionHash;
	// Apply 전 AppliedState resolver revision입니다.
	const int32 PreAppliedResolverRevision = Recipe->AppliedState.ResolverContractRevision;

	// Existing R3 facade에 전달할 exact reviewed request입니다.
	FCFVehicleApplyOpRequest ApplyRequest;
	ApplyRequest.ApplyRequest = FreshReview.PreparedApplyRequest;
	ApplyRequest.ExpectedDiffHash = FreshReview.DiffHash;
	ApplyRequest.CallContext = Request.CallContext;
	ApplyRequest.CallContext.ExpectedRecipeFingerprint = FreshReview.ApplyProposal.ExpectedRecipeFingerprint;
	ApplyRequest.CallContext.ExpectedTargetDefinitionHash = FreshReview.ApplyProposal.ExpectedTargetDefinitionHash;
	ApplyRequest.CallContext.ExpectedResolverContractRevision = FreshReview.ApplyProposal.ResolverContractRevision;

	// Existing R3 Apply facade terminal result입니다.
	FCFAuthoringOpResult ApplyOperation;
	const bool bApplied = ApplyResolvedVehicle(ApplyRequest, ApplyOperation);
	OutResult.Operation = ApplyOperation;
	OutResult.Operation.OperationName = CFVehicleBuilderReviewPrivate::ApplyOperationName;
	if (!bApplied)
	{
		return false;
	}

	// NoChange에는 새 transaction이 없으므로 Undo token을 발급하지 않습니다.
	if (ApplyOperation.Status == ECFAuthoringOpStatus::NoChange || !ApplyOperation.Mutation.bTargetChanged)
	{
		OutResult.bUndoAvailable = false;
		return true;
	}

	if (!GEditor || !GEditor->Trans)
	{
		OutResult.bUndoAvailable = false;
		OutResult.Operation.Message += TEXT(" 적용은 성공했지만 Builder guarded Undo token을 캡처할 Editor transaction buffer가 없습니다. 자동 저장은 수행되지 않았습니다.");
		return true;
	}

	// Apply 직후 current UE Undo stack top transaction입니다.
	const FTransactionContext UndoContext = GEditor->Trans->GetUndoContext(false);
	if (!UndoContext.TransactionId.IsValid())
	{
		OutResult.bUndoAvailable = false;
		OutResult.Operation.Message += TEXT(" 적용은 성공했지만 current UE Undo transaction identity를 캡처하지 못했습니다. 자동 저장은 수행되지 않았습니다.");
		return true;
	}

	// Apply 직후 exact Target Definition snapshot입니다.
	FCFVehicleDefinitionSnapshot PostApplyTargetSnapshot;
	// Apply 직후 exact semantic Recipe snapshot입니다.
	FCFVehicleRecipeSnapshot PostApplyRecipeSnapshot;
	// Guarded Undo token post-state capture diagnostic입니다.
	FString PostApplySnapshotError;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*TargetVehicleData, PostApplyTargetSnapshot, PostApplySnapshotError)
		|| !FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, PostApplyRecipeSnapshot, PostApplySnapshotError)
		|| PostApplyRecipeSnapshot.RecipeFingerprint != ApplyRecipeFingerprint)
	{
		OutResult.bUndoAvailable = false;
		OutResult.Operation.Message += PostApplySnapshotError.IsEmpty()
			? TEXT(" 적용은 성공했지만 post-Apply Target/Recipe evidence를 exact 캡처하지 못해 Builder guarded Undo token을 발급하지 않았습니다. 자동 저장은 수행되지 않았습니다.")
			: FString::Printf(TEXT(" 적용은 성공했지만 guarded Undo post-state 캡처에 실패했습니다: %s"), *PostApplySnapshotError);
		return true;
	}

	OutResult.UndoToken.TransactionId = UndoContext.TransactionId;
	OutResult.UndoToken.RecipeId = Recipe->RecipeId;
	OutResult.UndoToken.RecipePath = FSoftObjectPath(Recipe);
	OutResult.UndoToken.TargetDefinitionPath = FSoftObjectPath(TargetVehicleData);
	OutResult.UndoToken.ApplyRecipeFingerprint = ApplyRecipeFingerprint;
	OutResult.UndoToken.PreApplyTargetDefinitionHash = PreApplyTargetHash;
	OutResult.UndoToken.PreApplyAppliedRecipeFingerprint = PreAppliedRecipeFingerprint;
	OutResult.UndoToken.PreApplyAppliedSourceSignature = PreAppliedSourceSignature;
	OutResult.UndoToken.PreApplyAppliedDefinitionHash = PreAppliedDefinitionHash;
	OutResult.UndoToken.PreApplyAppliedResolverRevision = PreAppliedResolverRevision;
	OutResult.UndoToken.PostApplyTargetDefinitionHash = PostApplyTargetSnapshot.DefinitionHash;
	OutResult.UndoToken.PostApplyAppliedRecipeFingerprint = Recipe->AppliedState.AppliedRecipeFingerprint;
	OutResult.UndoToken.PostApplyAppliedSourceSignature = Recipe->AppliedState.AppliedSourceSignature;
	OutResult.UndoToken.PostApplyAppliedDefinitionHash = Recipe->AppliedState.AppliedDefinitionHash;
	OutResult.UndoToken.PostApplyAppliedResolverRevision = Recipe->AppliedState.ResolverContractRevision;
	OutResult.UndoToken.UndoScopeHash = CFVehicleBuilderReviewPrivate::BuildUndoScopeHash(OutResult.UndoToken);
	CFVehicleBuilderReviewPrivate::TrackUndoToken(OutResult.UndoToken);
	OutResult.bUndoAvailable = true;
	return true;
}

// Exact Builder-owned top transaction token과 explicit scope가 모두 일치할 때만 Unreal standard Undo를 수행합니다.
bool FCFVehicleAuthoringService::UndoBuilderFinalApply(
	const FCFBuilderUndoRequest& Request,
	FCFAuthoringOpResult& OutResult)
{
	CFVehicleBuilderReviewPrivate::InitializeOperation(
		OutResult,
		CFVehicleBuilderReviewPrivate::UndoOperationName,
		ECFAuthoringRiskClass::R3_DefinitionApply,
		Request.CallContext.ClientOperationId);

	if (Request.CallContext.ClientOperationId.IsEmpty()
		|| Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::DefinitionApply
		|| Request.CallContext.ApprovalScopeHash.IsEmpty()
		|| Request.CallContext.ApprovalScopeHash != Request.UndoToken.UndoScopeHash)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::ApprovalRequired,
			TEXT("Builder Undo에는 explicit ClientOperationId와 exact UndoScopeHash DefinitionApply approval이 필요합니다."));
	}
	if (!Request.UndoToken.TransactionId.IsValid()
		|| !Request.UndoToken.RecipeId.IsValid()
		|| !Request.UndoToken.RecipePath.IsValid()
		|| !Request.UndoToken.TargetDefinitionPath.IsValid())
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::InvalidSemanticInput,
			TEXT("Builder Undo token identity가 불완전합니다."));
	}

	// Current lifetime에서 이 Builder Apply가 실제 발급한 transaction record입니다.
	const CFVehicleBuilderReviewPrivate::FTrackedUndoRecord* TrackedRecord =
		CFVehicleBuilderReviewPrivate::TrackedUndoRecords.Find(Request.UndoToken.TransactionId);
	if (!TrackedRecord
		|| TrackedRecord->RecipeId != Request.UndoToken.RecipeId
		|| TrackedRecord->RecipePath != Request.UndoToken.RecipePath
		|| TrackedRecord->TargetPath != Request.UndoToken.TargetDefinitionPath
		|| TrackedRecord->UndoScopeHash != Request.UndoToken.UndoScopeHash)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::ApprovalScopeMismatch,
			TEXT("이 Undo token은 current Editor lifetime에서 Builder가 생성한 exact Apply transaction이 아닙니다."));
	}
	if (!GEditor || !GEditor->Trans)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::UnsupportedOperation,
			TEXT("Unreal transaction buffer가 없어 guarded Undo를 수행할 수 없습니다."));
	}

	// 다른 Editor transaction을 실수로 Undo하지 않기 위한 current exact stack top입니다.
	const FTransactionContext UndoContext = GEditor->Trans->GetUndoContext(false);
	if (!UndoContext.TransactionId.IsValid() || UndoContext.TransactionId != Request.UndoToken.TransactionId)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::StateChanged,
			TEXT("Builder Apply 이후 다른 Editor transaction이 current Undo stack top에 있어 안전한 Undo를 차단했습니다."));
	}

	// Undo 검증에 사용할 current Recipe object입니다.
	UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(Request.UndoToken.RecipePath.ResolveObject());
	if (!Recipe)
	{
		Recipe = Cast<UCFVehicleRecipeData>(Request.UndoToken.RecipePath.TryLoad());
	}
	// Undo 검증에 사용할 current Target object입니다.
	UCFVehicleData* TargetVehicleData = Cast<UCFVehicleData>(Request.UndoToken.TargetDefinitionPath.ResolveObject());
	if (!TargetVehicleData)
	{
		TargetVehicleData = Cast<UCFVehicleData>(Request.UndoToken.TargetDefinitionPath.TryLoad());
	}
	if (!Recipe || !TargetVehicleData || Recipe->RecipeId != Request.UndoToken.RecipeId)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::StateChanged,
			TEXT("Builder Undo owner Recipe/Target identity가 current state에서 달라졌습니다."));
	}

	// Undo 직전 current Target Definition snapshot입니다.
	FCFVehicleDefinitionSnapshot CurrentTargetSnapshot;
	// Undo 직전 current semantic Recipe snapshot입니다.
	FCFVehicleRecipeSnapshot CurrentRecipeSnapshot;
	// Undo precondition snapshot diagnostic입니다.
	FString CurrentSnapshotError;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*TargetVehicleData, CurrentTargetSnapshot, CurrentSnapshotError)
		|| !FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, CurrentRecipeSnapshot, CurrentSnapshotError)
		|| CurrentRecipeSnapshot.RecipeFingerprint != Request.UndoToken.ApplyRecipeFingerprint
		|| CurrentTargetSnapshot.DefinitionHash != Request.UndoToken.PostApplyTargetDefinitionHash
		|| Recipe->AppliedState.AppliedRecipeFingerprint != Request.UndoToken.PostApplyAppliedRecipeFingerprint
		|| Recipe->AppliedState.AppliedSourceSignature != Request.UndoToken.PostApplyAppliedSourceSignature
		|| Recipe->AppliedState.AppliedDefinitionHash != Request.UndoToken.PostApplyAppliedDefinitionHash
		|| Recipe->AppliedState.ResolverContractRevision != Request.UndoToken.PostApplyAppliedResolverRevision)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::StateChanged,
			CurrentSnapshotError.IsEmpty()
				? TEXT("Builder Apply 이후 Target/Recipe/AppliedState가 transaction 밖에서 변경되어 안전한 Undo를 차단했습니다.")
				: CurrentSnapshotError);
	}

	// Exact Builder transaction이 top이고 post-Apply state도 그대로일 때만 실행하는 Unreal standard Undo 결과입니다.
	const bool bUndoSucceeded = GEditor->UndoTransaction();
	if (!bUndoSucceeded)
	{
		return CFVehicleBuilderReviewPrivate::Block(
			OutResult,
			ECFAuthoringErrorCode::InternalError,
			TEXT("Unreal standard Undo가 Builder Apply transaction을 되돌리지 못했습니다."));
	}

	CFVehicleBuilderReviewPrivate::TrackedUndoRecords.Remove(Request.UndoToken.TransactionId);

	// Undo 뒤 full Target Definition hash readback입니다.
	FCFVehicleDefinitionSnapshot TargetSnapshot;
	// Undo 뒤 semantic Recipe fingerprint readback입니다.
	FCFVehicleRecipeSnapshot RecipeSnapshot;
	// Snapshot readback diagnostic입니다.
	FString SnapshotError;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*TargetVehicleData, TargetSnapshot, SnapshotError)
		|| !FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, RecipeSnapshot, SnapshotError)
		|| RecipeSnapshot.RecipeFingerprint != Request.UndoToken.ApplyRecipeFingerprint
		|| TargetSnapshot.DefinitionHash != Request.UndoToken.PreApplyTargetDefinitionHash
		|| Recipe->AppliedState.AppliedRecipeFingerprint != Request.UndoToken.PreApplyAppliedRecipeFingerprint
		|| Recipe->AppliedState.AppliedSourceSignature != Request.UndoToken.PreApplyAppliedSourceSignature
		|| Recipe->AppliedState.AppliedDefinitionHash != Request.UndoToken.PreApplyAppliedDefinitionHash
		|| Recipe->AppliedState.ResolverContractRevision != Request.UndoToken.PreApplyAppliedResolverRevision)
	{
		OutResult.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApplyRollbackFailed;
		OutResult.Message = SnapshotError.IsEmpty()
			? TEXT("Builder Undo는 실행됐지만 Target/AppliedState가 pre-Apply evidence와 정확히 일치하지 않습니다. current state를 다시 검토해야 합니다.")
			: SnapshotError;
		OutResult.Mutation.bTargetChanged = true;
		OutResult.Mutation.bRecipeChanged = true;
		OutResult.Mutation.bSavePerformed = false;
		return false;
	}

	OutResult.Mutation.bTargetChanged = true;
	OutResult.Mutation.bRecipeChanged = true;
	OutResult.Mutation.bProfileChanged = false;
	OutResult.Mutation.bCreatedAssets = false;
	OutResult.Mutation.bPackageDirty = true;
	OutResult.Mutation.bSavePerformed = false;
	OutResult.Mutation.bAutomaticRetryPerformed = false;
	OutResult.CurrentTargetDefinitionHash = TargetSnapshot.DefinitionHash;
	CFVehicleBuilderReviewPrivate::Succeed(
		OutResult,
		TEXT("Builder가 만든 exact Apply transaction을 Unreal standard Undo로 되돌렸습니다. 자동 저장은 수행하지 않았습니다."));
	return true;
}
