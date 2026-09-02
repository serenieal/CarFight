// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderEvidenceRefresh.cpp
// Version: v1.0.1
// Date: 2026-08-31
// Description: CF-FQ-040 Guided Vehicle Builder의 Existing Reference Evidence complete research replacement R1 구현입니다.
// Scope: Existing Evidence identity/Recipe/Target binding을 유지한 채 complete FACT/DERIVED research payload만 fresh fingerprint + explicit AuthoringWrite approval로 갱신합니다.
// Changelog:
// - v1.0.1: UCFVehicleRefEvidence::PostEditChangeProperty가 AuthoringRevision을 +1 하는 기존 owner임을 존중해 manual increment를 제거하고, rollback notification 뒤 diagnostic revision을 exact backup으로 재복원.
// - v1.0.0: mutation0 Preview, stale fingerprint/Recipe/Target guard, complete replacement validation, deterministic approval hash, transaction rollback/readback, Save0 commit을 최초 구현.
// Migration:
// - Existing Companion 생성 경로의 overwrite 금지는 유지합니다.
// - EvidenceId/TargetRecipeId/TargetRecipePath/TargetDefinitionPath는 변경하지 않습니다.
// - Profile/Recipe/Target payload와 VehicleData Apply/Save는 수행하지 않습니다.
// - EvidenceFingerprint가 바뀌면 기존 Builder receipt/Reference review는 자연스럽게 stale이므로 후속 Step 1/5 review를 다시 수행해야 합니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/SecureHash.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"

namespace CFBuilderEvidenceRefreshPrivate
{
	// Evidence refresh operation의 stable identity입니다.
	const FName RefreshOperationName(TEXT("RefreshBuilderEvidence"));

	// Common result를 R1 operation 시작 상태로 초기화합니다.
	void InitializeResult(FCFAuthoringOpResult& OutResult, const FString& ClientOperationId)
	{
		OutResult = FCFAuthoringOpResult();
		OutResult.OperationName = RefreshOperationName;
		OutResult.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
		OutResult.ClientOperationId = ClientOperationId;
		OutResult.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		OutResult.Mutation.bSavePerformed = false;
		OutResult.Mutation.bAutomaticRetryPerformed = false;
	}

	// Result를 fail-closed Blocked 상태로 설정합니다.
	bool Block(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
		return false;
	}

	// Result를 concurrent/current-state Conflict 상태로 설정합니다.
	bool Conflict(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Conflict;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
		return false;
	}

	// Result를 mutation 없는 NoChange 상태로 설정합니다.
	void NoChange(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::NoChange;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Result를 성공 상태로 설정합니다.
	void Succeed(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Succeeded;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Delimiter-safe deterministic proposal payload token을 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// Canonical UTF-8 payload를 lowercase MD5 approval digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// Platform-independent UTF-8 byte sequence입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Digest 누적 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());

		// 최종 MD5 byte 배열입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Lowercase hexadecimal 결과입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// Stable lowercase nibble 표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// Current Recipe에 exact binding된 Target VehicleData를 load합니다.
	UCFVehicleData* LoadTarget(UCFVehicleRecipeData& Recipe, FString& OutError)
	{
		// Recipe가 소유하는 exact Target VehicleData입니다.
		UCFVehicleData* Target = Recipe.TargetVehicleData.LoadSynchronous();
		if (!Target)
		{
			OutError = TEXT("Evidence Refresh 대상 Recipe의 Target VehicleData를 load할 수 없습니다.");
			return nullptr;
		}
		OutError.Reset();
		return Target;
	}

	// Existing Evidence를 exact class/Recipe/Target binding과 generated fingerprint까지 검증해 load합니다.
	UCFVehicleRefEvidence* LoadAndValidateEvidence(
		const FCFBuilderEvidenceRefreshRequest& Request,
		UCFVehicleData& Target,
		FString& OutFreshFingerprint,
		FString& OutError)
	{
		if (!Request.Recipe)
		{
			OutError = TEXT("Evidence Refresh에는 managed Recipe가 필요합니다.");
			return nullptr;
		}
		if (!Request.EvidencePath.IsValid())
		{
			OutError = TEXT("Evidence Refresh에는 current existing Evidence path가 필요합니다.");
			return nullptr;
		}

		// Already-loaded Evidence 또는 exact path에서 load한 object입니다.
		UObject* LoadedObject = Request.EvidencePath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = Request.EvidencePath.TryLoad();
		}

		// Exact Reference Evidence object입니다.
		UCFVehicleRefEvidence* Evidence = Cast<UCFVehicleRefEvidence>(LoadedObject);
		if (!Evidence)
		{
			OutError = FString::Printf(TEXT("Evidence Refresh 대상이 UCFVehicleRefEvidence가 아닙니다: %s"), *Request.EvidencePath.ToString());
			return nullptr;
		}

		// Persistent Evidence ownership identity입니다.
		const FSoftObjectPath ExpectedRecipePath(Request.Recipe);
		// Persistent Target identity입니다.
		const FSoftObjectPath ExpectedTargetPath(&Target);
		if (Evidence->TargetRecipeId != Request.Recipe->RecipeId
			|| Evidence->TargetRecipePath != ExpectedRecipePath
			|| Evidence->TargetDefinitionPath != ExpectedTargetPath)
		{
			OutError = TEXT("Evidence Refresh 대상의 Recipe/Target ownership binding이 current selected vehicle과 일치하지 않습니다.");
			return nullptr;
		}

		if (!Evidence->BuildEvidenceFingerprint(OutFreshFingerprint, OutError)
			|| OutFreshFingerprint != Evidence->EvidenceFingerprint)
		{
			if (OutError.IsEmpty())
			{
				OutError = TEXT("Current Evidence generated fingerprint가 semantic payload와 일치하지 않습니다.");
			}
			return nullptr;
		}

		OutError.Reset();
		return Evidence;
	}

	// Current Recipe/Target semantic fingerprint/hash를 fresh 계산합니다.
	bool BuildCurrentOwnerState(
		UCFVehicleRecipeData& Recipe,
		UCFVehicleData& Target,
		FString& OutRecipeFingerprint,
		FString& OutTargetDefinitionHash,
		FString& OutError)
	{
		// Current semantic Recipe snapshot입니다.
		FCFVehicleRecipeSnapshot RecipeSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, RecipeSnapshot, OutError))
		{
			return false;
		}

		// Current full Target Definition snapshot입니다.
		FCFVehicleDefinitionSnapshot DefinitionSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, DefinitionSnapshot, OutError))
		{
			return false;
		}

		OutRecipeFingerprint = RecipeSnapshot.RecipeFingerprint;
		OutTargetDefinitionHash = DefinitionSnapshot.DefinitionHash;
		OutError.Reset();
		return true;
	}

	// Complete replacement payload를 transient Evidence에 적용해 prospective fingerprint를 계산합니다.
	bool BuildProspectiveEvidence(
		const FCFBuilderEvidenceRefreshRequest& Request,
		const UCFVehicleRefEvidence& CurrentEvidence,
		FString& OutProspectiveFingerprint,
		int32& OutProspectiveClaimCount,
		int32& OutProspectiveUnknownFactCount,
		FString& OutError)
	{
		// Persistent object를 건드리지 않는 transient Evidence carrier입니다.
		UCFVehicleRefEvidence* ProspectiveEvidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage());
		if (!ProspectiveEvidence)
		{
			OutError = TEXT("Prospective Evidence carrier를 생성할 수 없습니다.");
			return false;
		}

		ProspectiveEvidence->EvidenceId = CurrentEvidence.EvidenceId;
		ProspectiveEvidence->SchemaRevision = CurrentEvidence.SchemaRevision;
		ProspectiveEvidence->NormalizationPolicyRevision = CurrentEvidence.NormalizationPolicyRevision;
		ProspectiveEvidence->TargetRecipeId = CurrentEvidence.TargetRecipeId;
		ProspectiveEvidence->TargetRecipePath = CurrentEvidence.TargetRecipePath;
		ProspectiveEvidence->TargetDefinitionPath = CurrentEvidence.TargetDefinitionPath;

		if (!ProspectiveEvidence->ApplyInitialResearchPayload(Request.EvidencePayload, OutError))
		{
			return false;
		}

		OutProspectiveFingerprint = ProspectiveEvidence->EvidenceFingerprint;
		OutProspectiveClaimCount = ProspectiveEvidence->Claims.Num();
		OutProspectiveUnknownFactCount = ProspectiveEvidence->UnknownFacts.Num();
		OutError.Reset();
		return true;
	}

	// Preview/Commit이 공유하는 deterministic approval scope hash를 생성합니다.
	FString BuildProposalHash(
		const FCFBuilderEvidenceRefreshRequest& Request,
		const UCFVehicleRefEvidence& Evidence,
		const FString& CurrentEvidenceFingerprint,
		const FString& ProspectiveEvidenceFingerprint,
		const FString& RecipeFingerprint,
		const FString& TargetDefinitionHash)
	{
		// Exact action/current/prospective state를 binding하는 canonical payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), RefreshOperationName.ToString());
		AppendToken(Payload, TEXT("RecipeId"), Request.Recipe ? Request.Recipe->RecipeId.ToString(EGuidFormats::DigitsWithHyphensLower) : FString());
		AppendToken(Payload, TEXT("RecipePath"), Request.Recipe ? FSoftObjectPath(Request.Recipe).ToString() : FString());
		AppendToken(Payload, TEXT("RecipeFingerprint"), RecipeFingerprint);
		AppendToken(Payload, TEXT("TargetPath"), Request.Recipe ? Request.Recipe->TargetVehicleData.ToSoftObjectPath().ToString() : FString());
		AppendToken(Payload, TEXT("TargetDefinitionHash"), TargetDefinitionHash);
		AppendToken(Payload, TEXT("EvidencePath"), Request.EvidencePath.ToString());
		AppendToken(Payload, TEXT("EvidenceId"), Evidence.EvidenceId.ToString(EGuidFormats::DigitsWithHyphensLower));
		AppendToken(Payload, TEXT("CurrentEvidenceFingerprint"), CurrentEvidenceFingerprint);
		AppendToken(Payload, TEXT("ProspectiveEvidenceFingerprint"), ProspectiveEvidenceFingerprint);
		AppendToken(Payload, TEXT("ResolverRevision"), FString::FromInt(FCFVehicleResolver::CurrentResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// UObject package의 current dirty flag를 반환합니다.
	bool IsPackageDirty(const UObject* Object)
	{
		// Object가 속한 package입니다.
		const UPackage* Package = Object ? Object->GetOutermost() : nullptr;
		return Package && Package->IsDirty();
	}

	// Rollback 후 UObject package dirty flag를 정확히 복원합니다.
	void RestorePackageDirty(UObject* Object, const bool bWasDirty)
	{
		// 복원할 owning package입니다.
		UPackage* Package = Object ? Object->GetOutermost() : nullptr;
		if (Package)
		{
			Package->SetDirtyFlag(bWasDirty);
		}
	}
}

// Existing Reference Evidence complete research replacement를 fresh current state에 binding해 mutation0 preview합니다.
bool FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(
	const FCFBuilderEvidenceRefreshRequest& Request,
	FCFBuilderEvidenceRefreshPreview& OutPreview)
{
	using namespace CFBuilderEvidenceRefreshPrivate;

	OutPreview = FCFBuilderEvidenceRefreshPreview();
	InitializeResult(OutPreview.Operation, Request.CallContext.ClientOperationId);

	if (!Request.Recipe)
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("Evidence Refresh에는 current managed Recipe가 필요합니다."));
	}

	// Current Recipe가 소유하는 exact Target입니다.
	FString Error;
	UCFVehicleData* Target = LoadTarget(*Request.Recipe, Error);
	if (!Target)
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::TargetNotFound, Error);
	}

	// Current generated semantic fingerprint까지 검증된 Evidence입니다.
	UCFVehicleRefEvidence* Evidence = LoadAndValidateEvidence(Request, *Target, OutPreview.CurrentEvidenceFingerprint, Error);
	if (!Evidence)
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, Error);
	}
	if (Request.ExpectedCurrentEvidenceFingerprint.IsEmpty()
		|| Request.ExpectedCurrentEvidenceFingerprint != OutPreview.CurrentEvidenceFingerprint)
	{
		return Conflict(
			OutPreview.Operation,
			ECFAuthoringErrorCode::StateChanged,
			TEXT("Evidence fingerprint가 Refresh Draft/Preview baseline과 다릅니다. current Research를 다시 읽고 새 proposal을 만들어야 합니다."));
	}

	// Preview baseline Recipe/Target authority입니다.
	FString RecipeFingerprint;
	FString TargetDefinitionHash;
	if (!BuildCurrentOwnerState(*Request.Recipe, *Target, RecipeFingerprint, TargetDefinitionHash, Error))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::InternalError, Error);
	}

	OutPreview.CurrentClaimCount = Evidence->Claims.Num();
	OutPreview.CurrentUnknownFactCount = Evidence->UnknownFacts.Num();
	if (!BuildProspectiveEvidence(
		Request,
		*Evidence,
		OutPreview.ProspectiveEvidenceFingerprint,
		OutPreview.ProspectiveClaimCount,
		OutPreview.ProspectiveUnknownFactCount,
		Error))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, Error);
	}

	OutPreview.Operation.CurrentRecipeFingerprint = RecipeFingerprint;
	OutPreview.Operation.CurrentTargetDefinitionHash = TargetDefinitionHash;
	OutPreview.Operation.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;

	OutPreview.Proposal.OperationName = RefreshOperationName;
	OutPreview.Proposal.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
	OutPreview.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	OutPreview.Proposal.ExpectedRecipeFingerprint = RecipeFingerprint;
	OutPreview.Proposal.ExpectedTargetDefinitionHash = TargetDefinitionHash;
	OutPreview.Proposal.ProspectiveRecipeFingerprint = RecipeFingerprint;
	OutPreview.Proposal.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	OutPreview.Proposal.bTargetMutation = false;
	OutPreview.Proposal.bSavePerformed = false;
	OutPreview.Proposal.ProposalHash = BuildProposalHash(
		Request,
		*Evidence,
		OutPreview.CurrentEvidenceFingerprint,
		OutPreview.ProspectiveEvidenceFingerprint,
		RecipeFingerprint,
		TargetDefinitionHash);

	if (OutPreview.CurrentEvidenceFingerprint == OutPreview.ProspectiveEvidenceFingerprint)
	{
		NoChange(OutPreview.Operation, TEXT("Existing Reference Evidence research payload가 current semantic state와 동일합니다."));
		return true;
	}

	Succeed(
		OutPreview.Operation,
		TEXT("Existing Reference Evidence complete research replacement preview를 생성했습니다. 아직 Evidence/Profile/Recipe/Target mutation이나 Save는 수행하지 않았습니다."));
	return true;
}

// Fresh preview + AuthoringWrite approval 뒤 exact existing Evidence research payload만 transaction commit합니다.
bool FCFVehicleAuthoringService::CommitBuilderEvidenceRefresh(
	const FCFBuilderEvidenceRefreshRequest& Request,
	const FCFBuilderEvidenceRefreshPreview& ApprovedPreview,
	FCFBuilderEvidenceRefreshResult& OutResult)
{
	using namespace CFBuilderEvidenceRefreshPrivate;

	OutResult = FCFBuilderEvidenceRefreshResult();
	InitializeResult(OutResult.Operation, Request.CallContext.ClientOperationId);

	// Commit 직전 current semantic state를 다시 계산한 fresh mutation0 preview입니다.
	FCFBuilderEvidenceRefreshPreview FreshPreview;
	if (!PreviewBuilderEvidenceRefresh(Request, FreshPreview))
	{
		OutResult.Operation = FreshPreview.Operation;
		return false;
	}
	if (FreshPreview.Operation.Status == ECFAuthoringOpStatus::NoChange)
	{
		OutResult.Operation = FreshPreview.Operation;
		OutResult.EvidenceFingerprint = FreshPreview.CurrentEvidenceFingerprint;
		return true;
	}

	if (ApprovedPreview.Proposal.ProposalHash.IsEmpty()
		|| ApprovedPreview.Proposal.ProposalHash != FreshPreview.Proposal.ProposalHash)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Conflict(OutResult.Operation, ECFAuthoringErrorCode::PreviewOutOfDate, TEXT("승인한 Evidence Refresh preview가 current fresh preview와 다릅니다."));
	}
	if (Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::AuthoringWrite)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Block(OutResult.Operation, ECFAuthoringErrorCode::ApprovalRequired, TEXT("Existing Reference Evidence Refresh에는 explicit AuthoringWrite approval이 필요합니다."));
	}
	if (Request.CallContext.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Block(OutResult.Operation, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("ApprovalScopeHash가 fresh Evidence Refresh proposal과 일치하지 않습니다."));
	}
	if (Request.CallContext.ExpectedRecipeFingerprint != FreshPreview.Proposal.ExpectedRecipeFingerprint)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Conflict(OutResult.Operation, ECFAuthoringErrorCode::RecipeFingerprintMismatch, TEXT("Recipe fingerprint가 Evidence Refresh preview 이후 변경되었습니다."));
	}
	if (Request.CallContext.ExpectedTargetDefinitionHash != FreshPreview.Proposal.ExpectedTargetDefinitionHash)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Conflict(OutResult.Operation, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Target Definition hash가 Evidence Refresh preview 이후 변경되었습니다."));
	}
	if (Request.CallContext.ExpectedResolverContractRevision != FreshPreview.Proposal.ResolverContractRevision
		|| Request.CallContext.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Conflict(OutResult.Operation, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Resolver contract revision이 Evidence Refresh preview와 일치하지 않습니다."));
	}

	// Fresh write target을 다시 load하고 exact current fingerprint를 확인합니다.
	FString Error;
	UCFVehicleData* Target = LoadTarget(*Request.Recipe, Error);
	if (!Target)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Block(OutResult.Operation, ECFAuthoringErrorCode::TargetNotFound, Error);
	}
	// Commit 대상 persistent Evidence입니다.
	FString CurrentEvidenceFingerprint;
	UCFVehicleRefEvidence* Evidence = LoadAndValidateEvidence(Request, *Target, CurrentEvidenceFingerprint, Error);
	if (!Evidence || CurrentEvidenceFingerprint != FreshPreview.CurrentEvidenceFingerprint)
	{
		OutResult.Operation = FreshPreview.Operation;
		return Conflict(
			OutResult.Operation,
			ECFAuthoringErrorCode::StateChanged,
			Error.IsEmpty() ? TEXT("Evidence가 preview 이후 변경되었습니다.") : Error);
	}

	// Identity/binding이 write 동안 절대 바뀌지 않았는지 확인하기 위한 backup입니다.
	const FGuid EvidenceIdBackup = Evidence->EvidenceId;
	// Recipe owner identity backup입니다.
	const FGuid TargetRecipeIdBackup = Evidence->TargetRecipeId;
	// Recipe path binding backup입니다.
	const FSoftObjectPath TargetRecipePathBackup = Evidence->TargetRecipePath;
	// Target path binding backup입니다.
	const FSoftObjectPath TargetDefinitionPathBackup = Evidence->TargetDefinitionPath;
	// Research payload rollback용 backup입니다.
	const TArray<FCFRefVehicleIdentity> ReferenceVehiclesBackup = Evidence->ReferenceVehicles;
	// Source citation rollback용 backup입니다.
	const TArray<FCFRefSourceCitation> SourcesBackup = Evidence->Sources;
	// Claim rollback용 backup입니다.
	const TArray<FCFRefClaim> ClaimsBackup = Evidence->Claims;
	// Conflict rollback용 backup입니다.
	const TArray<FCFRefConflict> ConflictsBackup = Evidence->Conflicts;
	// Unknown Fact rollback용 backup입니다.
	const TArray<FCFRefUnknownFact> UnknownFactsBackup = Evidence->UnknownFacts;
	// Research note rollback용 backup입니다.
	const FString ResearchNotesBackup = Evidence->ResearchNotes;
	// Fingerprint rollback용 backup입니다.
	const FString EvidenceFingerprintBackup = Evidence->EvidenceFingerprint;
	// Diagnostic revision rollback용 backup입니다.
	const int32 AuthoringRevisionBackup = Evidence->AuthoringRevision;
	// Research timestamp rollback용 backup입니다.
	const FDateTime LastResearchAtUtcBackup = Evidence->LastResearchAtUtc;
	// Commit 전 Evidence package dirty 상태입니다.
	const bool bEvidenceDirtyBefore = IsPackageDirty(Evidence);

	// Evidence research replacement를 하나의 Undo/Redo unit으로 묶는 transaction입니다.
	FScopedTransaction Transaction(NSLOCTEXT("CarFight", "RefreshBuilderEvidence", "Guided Vehicle Builder - Refresh Reference Evidence"));
	Evidence->Modify();

	// Complete validated replacement payload를 persistent Evidence에 적용합니다.
	if (!Evidence->ApplyInitialResearchPayload(Request.EvidencePayload, Error))
	{
		Transaction.Cancel();
		RestorePackageDirty(Evidence, bEvidenceDirtyBefore);
		OutResult.Operation = FreshPreview.Operation;
		return Block(OutResult.Operation, ECFAuthoringErrorCode::ValidationBlocked, Error);
	}
	// AuthoringRevision은 아래 PostEditChange() → UCFVehicleRefEvidence::PostEditChangeProperty()가 exactly once 증가시키는 기존 owner를 사용합니다.

	// Persistent assignment/readback 실패 시 payload/identity/dirty를 모두 복원합니다.
	auto RestoreEvidenceState = [&]()
	{
		Evidence->EvidenceId = EvidenceIdBackup;
		Evidence->TargetRecipeId = TargetRecipeIdBackup;
		Evidence->TargetRecipePath = TargetRecipePathBackup;
		Evidence->TargetDefinitionPath = TargetDefinitionPathBackup;
		Evidence->ReferenceVehicles = ReferenceVehiclesBackup;
		Evidence->Sources = SourcesBackup;
		Evidence->Claims = ClaimsBackup;
		Evidence->Conflicts = ConflictsBackup;
		Evidence->UnknownFacts = UnknownFactsBackup;
		Evidence->ResearchNotes = ResearchNotesBackup;
		Evidence->EvidenceFingerprint = EvidenceFingerprintBackup;
		Evidence->AuthoringRevision = AuthoringRevisionBackup;
		Evidence->LastResearchAtUtc = LastResearchAtUtcBackup;
		Evidence->PostEditChange();
		// PostEditChangeProperty의 diagnostic +1까지 rollback 대상이므로 notification 뒤 exact backup revision을 다시 복원합니다.
		Evidence->AuthoringRevision = AuthoringRevisionBackup;
		RestorePackageDirty(Evidence, bEvidenceDirtyBefore);
	};

	// Assignment 직후 semantic/identity readback입니다.
	FString AssignedFingerprint;
	const bool bAssignedStateMatches =
		Evidence->EvidenceId == EvidenceIdBackup
		&& Evidence->TargetRecipeId == TargetRecipeIdBackup
		&& Evidence->TargetRecipePath == TargetRecipePathBackup
		&& Evidence->TargetDefinitionPath == TargetDefinitionPathBackup
		&& Evidence->BuildEvidenceFingerprint(AssignedFingerprint, Error)
		&& AssignedFingerprint == FreshPreview.ProspectiveEvidenceFingerprint
		&& Evidence->EvidenceFingerprint == FreshPreview.ProspectiveEvidenceFingerprint;
	if (!bAssignedStateMatches)
	{
		RestoreEvidenceState();
		Transaction.Cancel();
		OutResult.Operation = FreshPreview.Operation;
		return Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::InternalError,
			Error.IsEmpty() ? TEXT("Evidence Refresh assignment readback이 approved prospective state와 일치하지 않아 rollback했습니다.") : Error);
	}

	Evidence->PostEditChange();

	// PostEditChange 뒤에도 semantic/identity가 정확히 유지되는지 최종 검증합니다.
	FString FinalFingerprint;
	const bool bFinalStateMatches =
		Evidence->EvidenceId == EvidenceIdBackup
		&& Evidence->TargetRecipeId == TargetRecipeIdBackup
		&& Evidence->TargetRecipePath == TargetRecipePathBackup
		&& Evidence->TargetDefinitionPath == TargetDefinitionPathBackup
		&& Evidence->BuildEvidenceFingerprint(FinalFingerprint, Error)
		&& FinalFingerprint == FreshPreview.ProspectiveEvidenceFingerprint
		&& Evidence->EvidenceFingerprint == FreshPreview.ProspectiveEvidenceFingerprint;
	if (!bFinalStateMatches)
	{
		RestoreEvidenceState();
		Transaction.Cancel();
		OutResult.Operation = FreshPreview.Operation;
		return Block(
			OutResult.Operation,
			ECFAuthoringErrorCode::InternalError,
			Error.IsEmpty() ? TEXT("PostEditChange 뒤 Evidence Refresh semantic readback이 달라져 rollback했습니다.") : Error);
	}

	// Final readback 성공 뒤에만 package dirty를 남깁니다.
	const bool bMarkedDirty = Evidence->MarkPackageDirty();

	OutResult.Operation = FreshPreview.Operation;
	OutResult.Operation.OperationName = RefreshOperationName;
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
	OutResult.Operation.ClientOperationId = Request.CallContext.ClientOperationId;
	OutResult.Operation.Mutation.bEvidenceChanged = true;
	OutResult.Operation.Mutation.bRecipeChanged = false;
	OutResult.Operation.Mutation.bTargetChanged = false;
	OutResult.Operation.Mutation.bProfileChanged = false;
	OutResult.Operation.Mutation.bCreatedAssets = false;
	OutResult.Operation.Mutation.bPackageDirty = bMarkedDirty || IsPackageDirty(Evidence);
	OutResult.Operation.Mutation.bSavePerformed = false;
	OutResult.Operation.Mutation.bAutomaticRetryPerformed = false;
	OutResult.Operation.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	OutResult.EvidenceFingerprint = Evidence->EvidenceFingerprint;
	Succeed(
		OutResult.Operation,
		TEXT("Existing Reference Evidence research payload를 한 transaction으로 갱신했습니다. EvidenceId/Recipe/Target binding과 Profile/VehicleData는 변경하지 않았고 Save는 수행하지 않았습니다. 기존 Builder receipt/Reference review는 새 Evidence fingerprint 기준으로 다시 검토해야 합니다."));
	return true;
}
