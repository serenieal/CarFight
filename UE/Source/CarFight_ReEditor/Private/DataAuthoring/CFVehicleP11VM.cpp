// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleP11VM.cpp
// Version: v1.1.0
// Date: 2026-08-18
// Description: DAUTH-P0-11~12 Frozen 24.90~24.94 Workspace completeness ViewModel orchestration입니다.
// Scope: 5-domain Profile binding/open, Shared Profile B2 edit/impact, External Drift reviewed recovery/Keep token, Mesh-only two-record creation과 affected Vehicle navigation을 제공합니다.
// Changelog:
// - v1.1.0: P0-12 UA-03 USER feedback에 따라 ListProfiles 기반 후보 조회와 existing BindVehicleProfile/Open Profile normal Workspace route를 추가.
// - v1.0.0: Frozen UX completeness ViewModel route 최초 구현.
// Migration:
// - 모든 persistent Authoring operation은 FCFVehicleAuthoringService facade만 사용합니다.
// - Profile write는 existing B2, Drift ownership은 existing Import/Registry Core, Target Apply는 existing Apply lane을 유지합니다.

#include "DataAuthoring/CFVehicleAuthoringVM.h"

#include "DataAuthoring/CFDriveStateProfile.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"


// Current Recipe의 exact bound Profile UObject를 Frozen domain으로 resolve합니다.
UObject* FCFVehicleAuthoringVM::ResolveBoundProfile(const ECFVehicleProfileDomain ProfileDomain) const
{
	if (!Recipe.IsValid())
	{
		return nullptr;
	}

	switch (ProfileDomain)
	{
	case ECFVehicleProfileDomain::VehicleBase:
		return Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
	case ECFVehicleProfileDomain::Drivetrain:
		return Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	case ECFVehicleProfileDomain::Handling:
		return Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
	case ECFVehicleProfileDomain::Performance:
		return Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	case ECFVehicleProfileDomain::DriveState:
		return Recipe->ProfileBindings.DriveStateProfile.LoadSynchronous();
	default:
		return nullptr;
	}
}

// Project Asset Registry의 Frozen Profile 후보를 selected Domain 기준으로 facade에서 read-only 갱신합니다.
bool FCFVehicleAuthoringVM::RefreshProfileChoices(const ECFVehicleProfileDomain ProfileDomain, FString& OutError)
{
	ProfileChoices.Reset();
	if (!Recipe.IsValid())
	{
		OutError = TEXT("Profile 후보 조회에는 managed Recipe selection이 필요합니다.");
		LastMessage = OutError;
		return false;
	}

	// Selected Domain 하나만 조회하는 bounded facade request입니다.
	FCFProfileListRequest Request;
	Request.Domain = ProfileDomain;
	// Asset Registry에서 반환할 read-only Profile 목록입니다.
	FCFProfileListResult Result;
	if (!FCFVehicleAuthoringService::ListProfiles(Request, Result))
	{
		OutError = Result.Operation.Message;
		LastMessage = OutError;
		return false;
	}

	ProfileChoices = MoveTemp(Result.Profiles);
	LastMessage = ProfileChoices.IsEmpty()
		? TEXT("선택한 종류에 사용할 수 있는 Shared Profile Asset이 없습니다.")
		: Result.Operation.Message;
	OutError.Reset();
	return true;
}

// Current Recipe의 exact selected Domain Profile binding path를 반환합니다.
FSoftObjectPath FCFVehicleAuthoringVM::GetBoundProfilePath(const ECFVehicleProfileDomain ProfileDomain) const
{
	if (!Recipe.IsValid())
	{
		return FSoftObjectPath();
	}

	switch (ProfileDomain)
	{
	case ECFVehicleProfileDomain::VehicleBase:
		return Recipe->ProfileBindings.VehicleBaseProfile.ToSoftObjectPath();
	case ECFVehicleProfileDomain::Drivetrain:
		return Recipe->ProfileBindings.DrivetrainProfile.ToSoftObjectPath();
	case ECFVehicleProfileDomain::Handling:
		return Recipe->ProfileBindings.HandlingProfile.ToSoftObjectPath();
	case ECFVehicleProfileDomain::Performance:
		return Recipe->ProfileBindings.PerformanceProfile.ToSoftObjectPath();
	case ECFVehicleProfileDomain::DriveState:
		return Recipe->ProfileBindings.DriveStateProfile.ToSoftObjectPath();
	default:
		return FSoftObjectPath();
	}
}

// Existing Profile asset을 reviewed R1 BindVehicleProfile semantic write로 Recipe에만 연결합니다.
bool FCFVehicleAuthoringVM::CommitProfileBinding(
	const ECFVehicleProfileDomain ProfileDomain,
	const FSoftObjectPath& ProfilePath,
	FCFAuthoringOpResult& OutResult)
{
	// Raw property write 대신 existing typed semantic binding command를 사용합니다.
	FCFVehicleSemanticChange Change;
	Change.Operation = ECFVehicleSemanticOp::BindVehicleProfile;
	Change.ProfileDomain = ProfileDomain;
	Change.ProfileAssetPath = ProfilePath;
	return CommitSemanticChange(Change, OutResult);
}

// Current Recipe에 bound된 selected Domain Profile을 Unreal 표준 Asset Editor로 엽니다.
bool FCFVehicleAuthoringVM::OpenBoundProfile(const ECFVehicleProfileDomain ProfileDomain, FString& OutError) const
{
	// Current binding에서 exact typed Profile UObject를 resolve합니다.
	UObject* BoundProfile = ResolveBoundProfile(ProfileDomain);
	if (!BoundProfile || !GEditor)
	{
		OutError = TEXT("선택한 종류에 연결된 Shared Profile 또는 Editor subsystem이 없습니다.");
		return false;
	}

	// Unreal 표준 Asset Editor navigation subsystem입니다.
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		OutError = TEXT("AssetEditorSubsystem을 가져올 수 없습니다.");
		return false;
	}
	AssetEditorSubsystem->OpenEditorForAsset(BoundProfile);
	OutError.Reset();
	return true;
}

// Current Recipe에 bound된 Shared Profile의 allowlisted numeric edit를 B2 Core로 mutation0 preview합니다.
bool FCFVehicleAuthoringVM::PrepareProfileNumericEdit(
	const ECFVehicleProfileDomain ProfileDomain,
	const FString& ColumnId,
	const FString& CanonicalNumericValue,
	FCFProfileNumericEditPreview& OutPreview)
{
	PreparedProfileEditRequest = FCFProfileNumericEditRequest();
	PreparedProfileEditPreview = FCFProfileNumericEditPreview();
	bHasPreparedProfileEdit = false;
	OutPreview = FCFProfileNumericEditPreview();
	if (!Recipe.IsValid())
	{
		LastMessage = TEXT("Shared Profile edit에는 managed Recipe가 필요합니다.");
		return false;
	}

	// Current Recipe가 exact domain에 bind한 Shared Profile입니다.
	UObject* BoundProfile = ResolveBoundProfile(ProfileDomain);
	if (!BoundProfile)
	{
		LastMessage = TEXT("선택한 Profile Domain에 bound Shared Profile이 없습니다.");
		return false;
	}

	// Existing B2 ProfileNumericEdit pipeline에 전달할 exact normal Workspace request입니다.
	FCFProfileNumericEditRequest Request;
	Request.ProfileObject = BoundProfile;
	Request.ProfileDomain = ProfileDomain;
	Request.ColumnId = ColumnId.TrimStartAndEnd();
	Request.CanonicalNumericValue = CanonicalNumericValue.TrimStartAndEnd();
	Request.CallerKind = ECFAuthoringCallerKind::SlateUI;
	if (!FCFVehicleAuthoringService::PreviewProfileNumericEdit(Request, OutPreview))
	{
		LastMessage = OutPreview.Operation.Message;
		return false;
	}

	PreparedProfileEditRequest = Request;
	PreparedProfileEditPreview = OutPreview;
	LastProfileImpactPreview = OutPreview;
	bHasPreparedProfileEdit = true;
	LastMessage = OutPreview.Operation.Message;
	return true;
}

// UI가 실제 review한 exact B2 Profile preview/approval만 existing source commit lane으로 실행합니다.
bool FCFVehicleAuthoringVM::ExecutePreparedProfileEdit(FCFProfileNumericEditResult& OutResult)
{
	OutResult = FCFProfileNumericEditResult();
	if (!bHasPreparedProfileEdit)
	{
		OutResult.Operation.OperationName = TEXT("CommitProfileNumericEdit");
		OutResult.Operation.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::ApprovalRequired;
		OutResult.Operation.Message = TEXT("Prepared Shared Profile edit approval이 없습니다. 다시 preview하세요.");
		LastMessage = OutResult.Operation.Message;
		return false;
	}

	// Commit 전 UI가 review한 exact request입니다.
	const FCFProfileNumericEditRequest Request = PreparedProfileEditRequest;
	// Commit 전 UI가 review한 exact B2 Session/Approval입니다.
	const FCFProfileNumericEditPreview ApprovedPreview = PreparedProfileEditPreview;
	bHasPreparedProfileEdit = false;
	PreparedProfileEditRequest = FCFProfileNumericEditRequest();
	PreparedProfileEditPreview = FCFProfileNumericEditPreview();
	InvalidatePreparedApply();
	InvalidateDriftReviewState();
	const bool bSucceeded = FCFVehicleAuthoringService::CommitProfileNumericEdit(Request, ApprovedPreview, OutResult);
	LastMessage = OutResult.Operation.Message;
	if (bSucceeded && OutResult.Operation.Mutation.bProfileChanged)
	{
		MarkWorkspaceTransaction(TEXT("Shared Profile 변경"));
	}
	if (bSucceeded && Recipe.IsValid())
	{
		// Shared Profile source commit 후 old preview를 폐기하고 fresh Resolve가 필요하다는 Frozen 계약을 바로 반영합니다.
		FString RefreshError;
		RefreshPreview(RefreshError);
	}
	return bSucceeded;
}

// Current External Drift를 exact Last Applied / Current Raw / Current Authoring rows로 read-only refresh합니다.
bool FCFVehicleAuthoringVM::RefreshDriftReview(FString& OutError)
{
	DriftReview = FCFVehicleDriftReviewResult();
	PreparedDriftDecisionRequest = FCFVehicleDriftDecisionRequest();
	PreparedDriftDecisionPreview = FCFVehicleDriftDecisionPreview();
	bHasPreparedDriftDecision = false;
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid())
	{
		OutError = TEXT("External Drift review에는 managed Recipe/Target selection이 필요합니다.");
		LastMessage = OutError;
		return false;
	}

	if (!FCFVehicleAuthoringService::BuildDriftReview(BuildReadRequest(), DriftReview))
	{
		OutError = DriftReview.Operation.Message;
		LastMessage = OutError;
		return false;
	}
	LastMessage = DriftReview.Operation.Message;
	OutError.Reset();
	return true;
}

// Selected field subset 또는 전체 Drift field group의 recovery decision을 mutation0 R2 proposal로 준비합니다.
bool FCFVehicleAuthoringVM::PrepareDriftDecision(
	const TArray<FCFVehicleFieldPath>& FieldPaths,
	const ECFVehicleDriftDecision Decision,
	const FString& OverrideReason,
	FCFVehicleDriftDecisionPreview& OutPreview)
{
	PreparedDriftDecisionRequest = FCFVehicleDriftDecisionRequest();
	PreparedDriftDecisionPreview = FCFVehicleDriftDecisionPreview();
	bHasPreparedDriftDecision = false;
	OutPreview = FCFVehicleDriftDecisionPreview();

	// Caller가 빈 subset을 주면 current External Drift 전체 group을 review 대상으로 사용합니다.
	TArray<FCFVehicleFieldPath> ReviewedPaths = FieldPaths;
	if (ReviewedPaths.IsEmpty())
	{
		// Fresh review diagnostic입니다.
		FString ReviewError;
		if (!RefreshDriftReview(ReviewError))
		{
			return false;
		}
		for (const FCFVehicleDriftReviewRow& Row : DriftReview.Rows)
		{
			ReviewedPaths.Add(Row.FieldPath);
		}
	}

	// Exact current 3-way state에 binding될 R2 decision request입니다.
	FCFVehicleDriftDecisionRequest Request;
	Request.ReadRequest = BuildReadRequest();
	Request.FieldPaths = ReviewedPaths;
	Request.Decision = Decision;
	Request.OverrideReason = OverrideReason.TrimStartAndEnd();
	if (!FCFVehicleAuthoringService::PreviewDriftDecision(Request, OutPreview))
	{
		LastMessage = OutPreview.Review.Operation.Message;
		return false;
	}

	DriftReview = OutPreview.Review;
	PreparedDriftDecisionRequest = Request;
	PreparedDriftDecisionPreview = OutPreview;
	bHasPreparedDriftDecision = true;
	LastMessage = OutPreview.Review.Operation.Message;
	return true;
}

// UI가 실제 review한 exact Drift R2 decision을 commit하고 Keep이면 current evidence token을 보존합니다.
bool FCFVehicleAuthoringVM::ExecutePreparedDriftDecision(FCFAuthoringOpResult& OutResult)
{
	OutResult = FCFAuthoringOpResult();
	if (!bHasPreparedDriftDecision)
	{
		OutResult.OperationName = TEXT("CommitDriftDecision");
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApprovalRequired;
		OutResult.Message = TEXT("Prepared External Drift decision approval이 없습니다. 다시 review하세요.");
		LastMessage = OutResult.Message;
		return false;
	}

	// UI가 review한 exact R2 request입니다.
	FCFVehicleDriftDecisionRequest Request = PreparedDriftDecisionRequest;
	Request.bOwnershipWriteApproved = true;
	Request.ApprovalScopeHash = PreparedDriftDecisionPreview.Proposal.ProposalHash;
	// Keep token과 persistent recovery를 구분하기 위한 reviewed decision입니다.
	const ECFVehicleDriftDecision ReviewedDecision = Request.Decision;
	// Keep token에 binding할 exact fresh review evidence입니다.
	const FCFVehicleDriftReviewResult ReviewedState = PreparedDriftDecisionPreview.Review;
	bHasPreparedDriftDecision = false;
	PreparedDriftDecisionRequest = FCFVehicleDriftDecisionRequest();
	PreparedDriftDecisionPreview = FCFVehicleDriftDecisionPreview();
	InvalidatePreparedApply();
	const bool bSucceeded = FCFVehicleAuthoringService::CommitDriftDecision(Request, OutResult);
	LastMessage = OutResult.Message;
	if (!bSucceeded)
	{
		return false;
	}

	if (ReviewedDecision == ECFVehicleDriftDecision::KeepAuthoring)
	{
		AcceptedDriftKeepRecipeFingerprint = ReviewedState.ExpectedRecipeFingerprint;
		AcceptedDriftKeepTargetHash = ReviewedState.ExpectedTargetDefinitionHash;
		AcceptedDriftKeepSourceSignature = ReviewedState.ExpectedSourceSignature;
		AcceptedDriftKeepResolverRevision = ReviewedState.ResolverContractRevision;
		DriftReview = ReviewedState;
		return true;
	}

	InvalidateDriftReviewState();
	if (OutResult.Mutation.bRecipeChanged)
	{
		MarkWorkspaceTransaction(TEXT("External Drift Recovery"));
	}
	// Persistent Recipe ownership decision 뒤 fresh Resolve/Diff/Validation을 다시 읽습니다.
	FString RefreshError;
	RefreshPreview(RefreshError);
	return true;
}

// Mesh-only Candidate 또는 explicit New Vehicle의 two-record R2 proposal을 준비합니다.
bool FCFVehicleAuthoringVM::PrepareVehicleRecordCreate(
	const FCFVehicleRecordCreateRequest& Request,
	FCFVehicleRecordCreatePreview& OutPreview)
{
	PreparedVehicleCreateRequest = FCFVehicleRecordCreateRequest();
	PreparedVehicleCreatePreview = FCFVehicleRecordCreatePreview();
	bHasPreparedVehicleCreate = false;
	OutPreview = FCFVehicleRecordCreatePreview();

	// UI caller와 mutation0 approval state를 강제한 exact preview request입니다.
	FCFVehicleRecordCreateRequest PreviewRequest = Request;
	PreviewRequest.CallerKind = ECFAuthoringCallerKind::SlateUI;
	PreviewRequest.bOwnershipWriteApproved = false;
	PreviewRequest.ApprovalScopeHash.Reset();
	if (!FCFVehicleAuthoringService::PreviewVehicleRecords(PreviewRequest, OutPreview))
	{
		LastMessage = OutPreview.Operation.Message;
		return false;
	}

	PreparedVehicleCreateRequest = PreviewRequest;
	PreparedVehicleCreatePreview = OutPreview;
	bHasPreparedVehicleCreate = true;
	LastMessage = OutPreview.Operation.Message;
	return true;
}

// UI가 실제 review한 exact Definition+Recipe creation proposal만 한 transaction으로 실행하고 새 Vehicle을 선택합니다.
bool FCFVehicleAuthoringVM::ExecutePreparedVehicleCreate(FCFVehicleRecordCreateResult& OutResult)
{
	OutResult = FCFVehicleRecordCreateResult();
	if (!bHasPreparedVehicleCreate)
	{
		OutResult.Operation.OperationName = TEXT("CreateVehicleRecords");
		OutResult.Operation.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::ApprovalRequired;
		OutResult.Operation.Message = TEXT("Prepared Vehicle record creation approval이 없습니다. 다시 preview하세요.");
		LastMessage = OutResult.Operation.Message;
		return false;
	}

	// UI가 review한 exact R2 creation request입니다.
	FCFVehicleRecordCreateRequest Request = PreparedVehicleCreateRequest;
	Request.bOwnershipWriteApproved = true;
	Request.ApprovalScopeHash = PreparedVehicleCreatePreview.Proposal.ProposalHash;
	bHasPreparedVehicleCreate = false;
	PreparedVehicleCreateRequest = FCFVehicleRecordCreateRequest();
	PreparedVehicleCreatePreview = FCFVehicleRecordCreatePreview();
	const bool bSucceeded = FCFVehicleAuthoringService::CreateVehicleRecords(Request, OutResult);
	LastMessage = OutResult.Operation.Message;
	if (!bSucceeded || !OutResult.CreatedDefinition || !OutResult.CreatedRecipe)
	{
		return false;
	}

	// Newly created unsaved objects를 current Workspace selection으로 연결할 exact row입니다.
	FCFVehicleListEntry CreatedEntry;
	CreatedEntry.DefinitionPath = FSoftObjectPath(OutResult.CreatedDefinition);
	CreatedEntry.RecipePath = FSoftObjectPath(OutResult.CreatedRecipe);
	CreatedEntry.RecipeId = OutResult.CreatedRecipe->RecipeId;
	CreatedEntry.ManageState = OutResult.CreatedRecipe->ImportState.ManageState;
	// Selection transition diagnostic입니다.
	FString SelectError;
	if (!SelectVehicle(CreatedEntry, SelectError))
	{
		LastMessage = FString::Printf(TEXT("Vehicle records는 생성됐지만 새 selection refresh에 실패했습니다: %s"), *SelectError);
	}
	MarkWorkspaceTransaction(TEXT("Vehicle Definition + Recipe 생성"));
	return true;
}

// Profile impact row의 Target Vehicle을 current Browser selection으로 이동합니다.
bool FCFVehicleAuthoringVM::NavigateToAffectedVehicle(const int32 ImpactIndex, FString& OutError)
{
	if (!LastProfileImpactPreview.AffectedVehicles.IsValidIndex(ImpactIndex))
	{
		OutError = TEXT("선택한 affected Vehicle index가 current impact preview에 없습니다.");
		return false;
	}
	// Navigation 대상 exact Target path입니다.
	const FString TargetPath = LastProfileImpactPreview.AffectedVehicles[ImpactIndex].TargetPath;
	// Current Browser cache에서 exact target row를 찾습니다.
	const FCFVehicleListEntry* MatchingEntry = BrowserEntries.FindByPredicate([&TargetPath](const FCFVehicleListEntry& Entry)
	{
		return !Entry.bMeshOnlyCandidate && Entry.DefinitionPath.ToString() == TargetPath;
	});
	if (!MatchingEntry)
	{
		// Profile commit 뒤 Browser cache가 오래됐을 수 있으므로 read-only refresh 한 번만 수행합니다.
		FString RefreshError;
		if (!RefreshBrowser(FString(), RefreshError))
		{
			OutError = RefreshError;
			return false;
		}
		MatchingEntry = BrowserEntries.FindByPredicate([&TargetPath](const FCFVehicleListEntry& Entry)
		{
			return !Entry.bMeshOnlyCandidate && Entry.DefinitionPath.ToString() == TargetPath;
		});
	}
	if (!MatchingEntry)
	{
		OutError = FString::Printf(TEXT("Affected Vehicle을 Browser에서 찾을 수 없습니다: %s"), *TargetPath);
		return false;
	}
	return SelectVehicle(*MatchingEntry, OutError);
}

// Current External Drift에 exact Keep Authoring review token이 살아있는지 반환합니다.
bool FCFVehicleAuthoringVM::HasAcceptedDriftKeep() const
{
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid() || !IsPreviewFresh() || !ResolveResult.ResolveResult.StaleReport.bHasExternalDrift)
	{
		return false;
	}
	return !AcceptedDriftKeepRecipeFingerprint.IsEmpty()
		&& AcceptedDriftKeepRecipeFingerprint == ResolveResult.ResolveRequest.Recipe.RecipeFingerprint
		&& AcceptedDriftKeepTargetHash == ResolveResult.ResolveRequest.CurrentDefinition.DefinitionHash
		&& AcceptedDriftKeepSourceSignature == ResolveResult.ResolveResult.SourceSignature
		&& AcceptedDriftKeepResolverRevision == ResolveResult.ResolveResult.ResolverContractRevision;
}

// Refresh/selection/source mutation 뒤 transient External Drift prepared/Keep evidence를 폐기합니다.
void FCFVehicleAuthoringVM::InvalidateDriftReviewState()
{
	DriftReview = FCFVehicleDriftReviewResult();
	PreparedDriftDecisionRequest = FCFVehicleDriftDecisionRequest();
	PreparedDriftDecisionPreview = FCFVehicleDriftDecisionPreview();
	bHasPreparedDriftDecision = false;
	AcceptedDriftKeepRecipeFingerprint.Reset();
	AcceptedDriftKeepTargetHash.Reset();
	AcceptedDriftKeepSourceSignature.Reset();
	AcceptedDriftKeepResolverRevision = 0;
}
