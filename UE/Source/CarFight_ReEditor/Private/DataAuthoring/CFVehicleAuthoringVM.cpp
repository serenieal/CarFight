// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringVM.cpp
// Version: v1.5.0
// Date: 2026-08-18
// Description: DAUTH-P0-09~12 Vehicle Authoring Workspace transient ViewModel 구현입니다.
// Changelog:
// - v1.5.0: P0-12 UA-06 readiness에서 UE top Undo TransactionId exact binding을 추가해 intervening Editor transaction을 잘못 Undo하지 않도록 fail-closed 보강.
// - v1.4.0: P0-12 UA-03 Profile choice cache를 selection lifecycle에 포함해 stale 후보를 보존하지 않도록 교정.
// - v1.3.0: Mesh-only Candidate selection과 reviewed Keep Authoring token 기반 External Drift Apply gate를 연결.
// - v1.2.0: P0-10 typed Assets/Layout, Driving Feel preset, Reference Compare, Adoption/Measurement review, Mount/Defaults, standard Undo orchestration 추가.
// - v1.1.0: reviewed Initial Import exact R2 proposal을 transient approval로 보존하고 unsaved loaded Recipe/Target selection을 지원.
// - v1.0.0: Facade-only Browser/Selection/Preview/Diff/Trace/Validation/InitialImport/RecipeIntent/Apply orchestration과 Raw DA navigation 추가.
// Migration:
// - ViewModel은 SnapshotBuilder/Resolver/ImportService/ApplyService/Validator를 직접 호출하지 않습니다.
// - Definition Apply mutation은 FCFVehicleAuthoringService::ApplyResolvedVehicle만 사용합니다.
// - 별도 memory Revert/Undo 구현을 만들지 않고 Unreal standard transaction Undo를 유지합니다.

#include "DataAuthoring/CFVehicleAuthoringVM.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Editor.h"
#include "Editor/Transactor.h"
#include "Subsystems/AssetEditorSubsystem.h"

namespace CFVehicleAuthoringVMPrivate
{
	// Persistent manage state를 Section 24.5 transient presentation state로 변환합니다.
	ECFWorkspaceManageView ToManageView(const ECFVehicleManageState ManageState)
	{
		switch (ManageState)
		{
		case ECFVehicleManageState::LegacyImported:
			return ECFWorkspaceManageView::LegacyImported;
		case ECFVehicleManageState::PartiallyManaged:
			return ECFWorkspaceManageView::PartiallyManaged;
		case ECFVehicleManageState::Managed:
			return ECFWorkspaceManageView::Managed;
		default:
			return ECFWorkspaceManageView::Unmanaged;
		}
	}
}

// Asset Registry 기반 Vehicle Browser cache를 facade를 통해 재구성합니다.
bool FCFVehicleAuthoringVM::RefreshBrowser(const FString& SearchText, FString& OutError)
{
	// Facade-only Vehicle list request입니다.
	FCFVehicleListRequest Request;
		Request.SearchText = SearchText;

	// Facade가 반환할 fresh Browser rows입니다.
	FCFVehicleListResult Result;
	if (!FCFVehicleAuthoringService::ListVehicles(Request, Result))
	{
		OutError = Result.Operation.Message;
		LastMessage = OutError;
		return false;
	}
		BrowserEntries = MoveTemp(Result.Vehicles);
	LastMessage = Result.Operation.Message;
	OutError.Reset();
	return true;
}

// Browser row를 current single-Vehicle selection으로 전환하고 managed target이면 fresh preview를 구성합니다.
bool FCFVehicleAuthoringVM::SelectVehicle(const FCFVehicleListEntry& Entry, FString& OutError)
{
	ClearSelection();
	if (Entry.bMeshOnlyCandidate && Entry.ChassisMeshPath.IsValid())
	{
		SelectedEntry = Entry;
		bHasSelection = true;
		ManagementView = ECFWorkspaceManageView::Unmanaged;
		SyncView = ECFWorkspaceSyncView::NoBaseline;
		ValidationView = ECFWorkspaceValidView::NotEvaluated;
		PreviewView = ECFWorkspacePreviewView::Blocked;
		LastMessage = TEXT("Mesh-only Candidate입니다. Definition이 아니므로 Create Vehicle From Mesh review 전 Validator/Apply/Import를 실행하지 않습니다.");
		OutError.Reset();
		return true;
	}
	if (!Entry.DefinitionPath.IsValid())
	{
		OutError = TEXT("Vehicle Browser row에 valid DefinitionPath가 없습니다.");
		LastMessage = OutError;
		return false;
	}

		// Browser row의 Runtime canonical Target입니다. Unsaved loaded object를 먼저 사용하고 필요할 때만 disk load합니다.
	UObject* TargetObject = Entry.DefinitionPath.ResolveObject();
	if (!TargetObject)
	{
		TargetObject = Entry.DefinitionPath.TryLoad();
	}
	// Resolved/loaded object를 exact VehicleData type으로 제한합니다.
	UCFVehicleData* LoadedTarget = Cast<UCFVehicleData>(TargetObject);

	if (!LoadedTarget)
	{
		OutError = FString::Printf(TEXT("VehicleData를 열 수 없습니다: %s"), *Entry.DefinitionPath.ToString());
		LastMessage = OutError;
		return false;
	}

	SelectedEntry = Entry;
	TargetVehicleData = LoadedTarget;
	bHasSelection = true;
	if (Entry.RecipePath.IsValid())
	{
				// Managed row의 Editor-only Recipe입니다. Unsaved newly imported Recipe도 즉시 선택할 수 있게 loaded object를 우선합니다.
		UObject* RecipeObject = Entry.RecipePath.ResolveObject();
		if (!RecipeObject)
		{
			RecipeObject = Entry.RecipePath.TryLoad();
		}
		// Resolved/loaded object를 exact Recipe type으로 제한합니다.
		UCFVehicleRecipeData* LoadedRecipe = Cast<UCFVehicleRecipeData>(RecipeObject);

		if (!LoadedRecipe)
		{
			OutError = FString::Printf(TEXT("Recipe를 열 수 없습니다: %s"), *Entry.RecipePath.ToString());
			LastMessage = OutError;
			ClearSelection();
			return false;
		}
		Recipe = LoadedRecipe;
		ManagementView = CFVehicleAuthoringVMPrivate::ToManageView(LoadedRecipe->ImportState.ManageState);
		return RefreshPreview(OutError);
	}

	ManagementView = ECFWorkspaceManageView::Unmanaged;
	SyncView = ECFWorkspaceSyncView::NoBaseline;
	ValidationView = ECFWorkspaceValidView::NotEvaluated;
	PreviewView = ECFWorkspacePreviewView::Blocked;
	LastMessage = TEXT("Unmanaged Existing Definition입니다. Validate/Raw Open 또는 reviewed Initial Import를 사용하세요.");
	OutError.Reset();
	return true;
}

// Current selection의 facade Resolve/Diff/Trace/Validation을 fresh result로 교체합니다.
bool FCFVehicleAuthoringVM::RefreshPreview(FString& OutError)
{
	InvalidatePreparedApply();
	InvalidateDriftReviewState();
	if (!bHasSelection || !TargetVehicleData.IsValid())
	{
		PreviewView = ECFWorkspacePreviewView::Blocked;
		OutError = TEXT("Preview할 Vehicle selection이 없습니다.");
		LastMessage = OutError;
		return false;
	}
	if (!Recipe.IsValid())
	{
		PreviewView = ECFWorkspacePreviewView::Blocked;
		ManagementView = ECFWorkspaceManageView::Unmanaged;
		OutError = TEXT("Unmanaged Vehicle은 Recipe Initial Import 전 Resolve Preview를 만들 수 없습니다.");
		LastMessage = OutError;
		return false;
	}

	// 모든 read operation이 공유하는 current Recipe/Target request입니다.
	const FCFVehicleAuthoringReadRequest ReadRequest = BuildReadRequest();
	if (!FCFVehicleAuthoringService::ReadVehicleContext(ReadRequest, ContextResult))
	{
		PreviewView = ECFWorkspacePreviewView::Blocked;
		OutError = ContextResult.Operation.Message;
		LastMessage = OutError;
		RefreshDerivedViews();
		return false;
	}
	if (!FCFVehicleAuthoringService::ResolveVehiclePreview(ReadRequest, ResolveResult))
	{
		PreviewView = ECFWorkspacePreviewView::Blocked;
		OutError = ResolveResult.Operation.Message;
		LastMessage = OutError;
		RefreshDerivedViews();
		return false;
	}
		// P0-09 기본 Workspace는 field filter 없이 전체 Source Trace를 읽습니다.
	const TArray<FCFVehicleFieldPath> AllTraceFields;
	if (!FCFVehicleAuthoringService::ReadPendingDiff(ReadRequest, DiffResult)
		|| !FCFVehicleAuthoringService::ReadSourceTrace(ReadRequest, AllTraceFields, TraceResult)
		|| !FCFVehicleAuthoringService::ReadValidation(ReadRequest, ValidationResult))
	{
		PreviewView = ECFWorkspacePreviewView::Blocked;
		OutError = !DiffResult.Operation.Message.IsEmpty()
			? DiffResult.Operation.Message
			: (!TraceResult.Operation.Message.IsEmpty() ? TraceResult.Operation.Message : ValidationResult.Operation.Message);
		LastMessage = OutError;
		RefreshDerivedViews();
		return false;
	}

	PreviewView = ResolveResult.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success
		? ECFWorkspacePreviewView::Fresh
		: ECFWorkspacePreviewView::Blocked;
		LastMessage = ResolveResult.Operation.Message;
	RefreshDerivedViews();
	// Fresh preview와 같은 Resolver state의 Wheel measurement proposals를 facade에서 갱신합니다.
	FCFVehicleMeasurementReadRequest MeasurementRequest;
	MeasurementRequest.Recipe = Recipe.Get();
	MeasurementRequest.TargetVehicleData = TargetVehicleData.Get();
	MeasurementRequest.CallerKind = ECFAuthoringCallerKind::SlateUI;
	FCFVehicleAuthoringService::ReadMeasurementProposals(MeasurementRequest, MeasurementResult);
	OutError.Reset();
	return PreviewView == ECFWorkspacePreviewView::Fresh;
}

// Persistent mutation 없이 typed VehicleArchetype prospective preview를 계산합니다.
bool FCFVehicleAuthoringVM::PreviewArchetypeIntent(const FName VehicleArchetypeId, FCFVehicleRecipePreviewResult& OutPreview)
{
	OutPreview = FCFVehicleRecipePreviewResult();
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid())
	{
		LastMessage = TEXT("Recipe Intent Preview에는 managed Recipe/Target selection이 필요합니다.");
		return false;
	}

	// Facade R1 typed semantic preview request입니다.
	FCFVehicleRecipeChangeRequest Request;
	Request.Recipe = Recipe.Get();
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.Change.Operation = ECFVehicleSemanticOp::SetVehicleArchetype;
	Request.Change.VehicleArchetypeId = VehicleArchetypeId;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	const bool bSucceeded = FCFVehicleAuthoringService::PreviewRecipeChange(Request, OutPreview);
	LastMessage = OutPreview.Operation.Message;
	return bSucceeded;
}

// Reviewed R1 proposal을 공통 typed semantic transaction 경로로 commit하고 preview/Undo 상태를 갱신합니다.
bool FCFVehicleAuthoringVM::CommitArchetypeIntent(const FName VehicleArchetypeId, FCFAuthoringOpResult& OutResult)
{
	// Archetype도 다른 P0-10 Recipe edit와 동일한 typed semantic transaction/Undo 경로를 사용합니다.
	FCFVehicleSemanticChange Change;
	Change.Operation = ECFVehicleSemanticOp::SetVehicleArchetype;
	Change.VehicleArchetypeId = VehicleArchetypeId;
	return CommitSemanticChange(Change, OutResult);
}

// 임의 raw field가 아닌 typed semantic change를 persistent mutation 없이 prospective facade preview합니다.
bool FCFVehicleAuthoringVM::PreviewSemanticChange(const FCFVehicleSemanticChange& Change, FCFVehicleRecipePreviewResult& OutPreview)
{
	OutPreview = FCFVehicleRecipePreviewResult();
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid())
	{
		LastMessage = TEXT("Typed semantic preview에는 managed Recipe/Target selection이 필요합니다.");
		return false;
	}
	// Common R1 typed semantic prospective request입니다.
	FCFVehicleRecipeChangeRequest Request;
	Request.Recipe = Recipe.Get();
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.Change = Change;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	// Common facade prospective result입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::PreviewRecipeChange(Request, OutPreview);
	LastMessage = OutPreview.Operation.Message;
	return bSucceeded;
}

// Typed semantic change를 exact R1 preview/approval 뒤 Recipe-only commit하고 fresh preview를 다시 읽습니다.
bool FCFVehicleAuthoringVM::CommitSemanticChange(const FCFVehicleSemanticChange& Change, FCFAuthoringOpResult& OutResult)
{
	OutResult = FCFAuthoringOpResult();
	// UI가 commit할 exact mutation0 prospective proposal입니다.
	FCFVehicleRecipePreviewResult Preview;
	if (!PreviewSemanticChange(Change, Preview))
	{
		OutResult = Preview.Operation;
		return false;
	}
	// Exact reviewed R1 semantic commit request입니다.
	FCFVehicleRecipeChangeRequest Request;
	Request.Recipe = Recipe.Get();
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.Change = Change;
	Request.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	Request.CallContext.ApprovalScopeHash = Preview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = Preview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = Preview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = Preview.Proposal.ResolverContractRevision;
	// Shared facade Recipe-only commit 결과입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::CommitRecipeChange(Request, OutResult);
	LastMessage = OutResult.Message;
	InvalidatePreparedApply();
	bHasPreparedMeasurement = false;
	bHasPreparedAdoption = false;
	if (bSucceeded && OutResult.Mutation.bRecipeChanged)
	{
		MarkWorkspaceTransaction(TEXT("Recipe 의미 변경"));
	}
	if (bSucceeded)
	{
		// Discrete Recipe commit 뒤 Frozen UX에 따라 full preview를 자동 refresh합니다.
		FString RefreshError;
		RefreshPreview(RefreshError);
	}
	return bSucceeded;
}

// Assets & Layout의 typed Asset/Socket Intent를 Recipe-only로 commit합니다.
bool FCFVehicleAuthoringVM::CommitAssetIntent(const FCFVehicleAssetIntent& AssetIntent, FCFAuthoringOpResult& OutResult)
{
	// Raw Target field가 아닌 Recipe AssetIntent semantic command입니다.
	FCFVehicleSemanticChange Change;
	Change.Operation = ECFVehicleSemanticOp::SetVehicleAssetIntent;
	Change.AssetIntent = AssetIntent;
	return CommitSemanticChange(Change, OutResult);
}

// Driving Feel partial 4축 semantic patch를 Recipe-only로 commit합니다.
bool FCFVehicleAuthoringVM::CommitDrivingFeel(const FCFDrivingFeelPatch& Patch, FCFAuthoringOpResult& OutResult)
{
	// Frozen 4-axis typed semantic command입니다.
	FCFVehicleSemanticChange Change;
	Change.Operation = ECFVehicleSemanticOp::SetDrivingFeel;
	Change.DrivingFeelPatch = Patch;
	return CommitSemanticChange(Change, OutResult);
}

// Frozen migration-parity named preset의 exact 4축 semantic 값을 한 Recipe transaction으로 commit합니다.
bool FCFVehicleAuthoringVM::CommitDrivingFeelPreset(const FName PresetId, FCFAuthoringOpResult& OutResult)
{
	// Preset이 확장될 exact all-axis semantic patch입니다.
	FCFDrivingFeelPatch Patch;
	Patch.bSetAccelerationFeel = true;
	Patch.bSetSteeringAgility = true;
	Patch.bSetGripFeel = true;
	Patch.bSetSuspensionFirmness = true;
	if (PresetId == TEXT("Sedan"))
	{
		Patch.AccelerationFeel = 0.50f;
		Patch.SteeringAgility = 0.50f;
		Patch.GripFeel = 0.55f;
		Patch.SuspensionFirmness = 0.45f;
	}
	else if (PresetId == TEXT("SUV"))
	{
		Patch.AccelerationFeel = 0.38f;
		Patch.SteeringAgility = 0.34f;
		Patch.GripFeel = 0.50f;
		Patch.SuspensionFirmness = 0.38f;
	}
	else if (PresetId == TEXT("Sports"))
	{
		Patch.AccelerationFeel = 0.85f;
		Patch.SteeringAgility = 0.78f;
		Patch.GripFeel = 0.82f;
		Patch.SuspensionFirmness = 0.78f;
	}
	else if (PresetId == TEXT("Heavy"))
	{
		Patch.AccelerationFeel = 0.28f;
		Patch.SteeringAgility = 0.25f;
		Patch.GripFeel = 0.45f;
		Patch.SuspensionFirmness = 0.55f;
	}
	else
	{
		OutResult = FCFAuthoringOpResult();
		OutResult.OperationName = TEXT("SetDrivingFeelPreset");
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
		OutResult.Message = FString::Printf(TEXT("지원하지 않는 Driving Feel preset입니다: %s"), *PresetId.ToString());
		LastMessage = OutResult.Message;
		return false;
	}
	return CommitDrivingFeel(Patch, OutResult);
}

// Mounts & Defaults의 typed DefaultData intent를 Recipe-only로 commit합니다.
bool FCFVehicleAuthoringVM::CommitDefaultDataIntent(const FCFVehicleDefaultIntent& DefaultIntent, FCFAuthoringOpResult& OutResult)
{
	// DefaultData typed semantic command입니다.
	FCFVehicleSemanticChange Change;
	Change.Operation = ECFVehicleSemanticOp::SetDefaultDataIntent;
	Change.DefaultDataIntent = DefaultIntent;
	return CommitSemanticChange(Change, OutResult);
}

// Stable-ID Hardpoint intent를 typed Recipe write로 upsert합니다.
bool FCFVehicleAuthoringVM::UpsertHardpointIntent(const FCFHardpointIntent& HardpointIntent, FCFAuthoringOpResult& OutResult)
{
	// Stable-ID Hardpoint typed semantic command입니다.
	FCFVehicleSemanticChange Change;
	Change.Operation = ECFVehicleSemanticOp::UpsertHardpointIntent;
	Change.HardpointIntent = HardpointIntent;
	return CommitSemanticChange(Change, OutResult);
}

// Stable-ID Mount intent를 typed Recipe write로 upsert합니다.
bool FCFVehicleAuthoringVM::UpsertMountIntent(const FCFMountIntent& MountIntent, FCFAuthoringOpResult& OutResult)
{
	// Stable-ID Mount typed semantic command입니다.
	FCFVehicleSemanticChange Change;
	Change.Operation = ECFVehicleSemanticOp::UpsertMountIntent;
	Change.MountIntent = MountIntent;
	return CommitSemanticChange(Change, OutResult);
}

// Browser row를 read-only Reference Vehicle로 선택하고 current/reference compare를 갱신합니다.
bool FCFVehicleAuthoringVM::SelectReferenceVehicle(const FCFVehicleListEntry& Entry, FString& OutError)
{
	if (!Entry.DefinitionPath.IsValid())
	{
		OutError = TEXT("Reference Vehicle row에 valid DefinitionPath가 없습니다.");
		LastMessage = OutError;
		return false;
	}
	ReferenceEntry = Entry;
	bHasReferenceVehicle = true;
	return RefreshReferenceCompare(false, OutError);
}

// Current Resolved Preview와 selected Reference를 facade 117-field projection으로 비교합니다.
bool FCFVehicleAuthoringVM::RefreshReferenceCompare(const bool bChangedOnly, FString& OutError)
{
	ReferenceCompareResult = FCFVehicleReferenceCompareResult();
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid() || !bHasReferenceVehicle)
	{
		OutError = TEXT("Reference Compare에는 current managed Vehicle과 Reference Vehicle selection이 필요합니다.");
		LastMessage = OutError;
		return false;
	}
	// Reference Target loaded object입니다.
	UObject* ReferenceTargetObject = ReferenceEntry.DefinitionPath.ResolveObject();
	if (!ReferenceTargetObject)
	{
		ReferenceTargetObject = ReferenceEntry.DefinitionPath.TryLoad();
	}
	// Exact Reference VehicleData입니다.
	UCFVehicleData* ReferenceTarget = Cast<UCFVehicleData>(ReferenceTargetObject);
	if (!ReferenceTarget)
	{
		OutError = TEXT("Reference VehicleData를 resolve할 수 없습니다.");
		LastMessage = OutError;
		return false;
	}
	// Optional managed Reference Recipe입니다.
	UCFVehicleRecipeData* ReferenceRecipe = nullptr;
	if (ReferenceEntry.RecipePath.IsValid())
	{
		// Loaded object를 우선하는 Reference Recipe UObject입니다.
		UObject* ReferenceRecipeObject = ReferenceEntry.RecipePath.ResolveObject();
		if (!ReferenceRecipeObject)
		{
			ReferenceRecipeObject = ReferenceEntry.RecipePath.TryLoad();
		}
		ReferenceRecipe = Cast<UCFVehicleRecipeData>(ReferenceRecipeObject);
	}
	// Frozen Reference Compare A=current resolved, B=managed resolved 또는 unmanaged current Definition request입니다.
	FCFVehicleReferenceCompareRequest CompareRequest;
	CompareRequest.CurrentRecipe = Recipe.Get();
	CompareRequest.CurrentTarget = TargetVehicleData.Get();
	CompareRequest.CurrentMode = ECFVehicleCompareValueMode::ResolvedPreview;
	CompareRequest.ReferenceRecipe = ReferenceRecipe;
	CompareRequest.ReferenceTarget = ReferenceTarget;
	CompareRequest.ReferenceMode = ReferenceRecipe ? ECFVehicleCompareValueMode::ResolvedPreview : ECFVehicleCompareValueMode::CurrentDefinition;
	CompareRequest.bIncludeSameValues = !bChangedOnly;
	// Facade read-only compare result입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::CompareReferenceVehicles(CompareRequest, ReferenceCompareResult);
	LastMessage = ReferenceCompareResult.Operation.Message;
	if (!bSucceeded)
	{
		OutError = LastMessage;
		return false;
	}
	OutError.Reset();
	return true;
}

// Current Resolver R6 Wheel measurement proposals를 facade에서 갱신합니다.
bool FCFVehicleAuthoringVM::RefreshMeasurementProposals(FString& OutError)
{
	MeasurementResult = FCFVehicleMeasurementReadResult();
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid())
	{
		OutError = TEXT("Measurement proposals에는 managed Recipe/Target selection이 필요합니다.");
		return false;
	}
	// Facade R0 measurement request입니다.
	FCFVehicleMeasurementReadRequest Request;
	Request.Recipe = Recipe.Get();
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.CallerKind = ECFAuthoringCallerKind::SlateUI;
	// Shared Resolver R6 proposal read result입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::ReadMeasurementProposals(Request, MeasurementResult);
	LastMessage = MeasurementResult.Operation.Message;
	if (!bSucceeded)
	{
		OutError = LastMessage;
		return false;
	}
	OutError.Reset();
	return true;
}

// 선택 measurement decision을 mutation0 prospective R2 proposal로 준비합니다.
bool FCFVehicleAuthoringVM::PrepareMeasurementDecision(
	const FCFVehicleMeasurementProposal& Proposal,
	const ECFVehicleMeasureDecision Decision,
	FCFVehicleMeasurementPreviewResult& OutPreview)
{
	PreparedMeasurementRequest = FCFVehicleMeasurementRequest();
	PreparedMeasurementProposal = FCFAuthoringProposal();
	bHasPreparedMeasurement = false;
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid())
	{
		LastMessage = TEXT("Measurement decision에는 managed Recipe/Target selection이 필요합니다.");
		return false;
	}
	// UI가 실제 검토할 exact proposal identity/value를 담는 R0 preview request입니다.
	FCFVehicleMeasurementRequest Request;
	Request.Recipe = Recipe.Get();
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.FieldPath = Proposal.FieldPath;
	Request.MeasurementRuleId = Proposal.MeasurementRuleId;
	Request.AssetFingerprint = Proposal.AssetFingerprint;
	Request.MeasuredCandidateValue = Proposal.MeasuredCandidateValue;
	Request.Decision = Decision;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	// Mutation0 prospective decision result입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::PreviewMeasurementDecision(Request, OutPreview);
	LastMessage = OutPreview.Operation.Message;
	if (bSucceeded)
	{
		PreparedMeasurementRequest = Request;
		PreparedMeasurementProposal = OutPreview.Proposal;
		bHasPreparedMeasurement = true;
	}
	return bSucceeded;
}

// UI가 실제 검토한 prepared measurement R2 proposal만 Recipe-only commit합니다.
bool FCFVehicleAuthoringVM::ExecutePreparedMeasurement(FCFAuthoringOpResult& OutResult)
{
	OutResult = FCFAuthoringOpResult();
	if (!bHasPreparedMeasurement)
	{
		OutResult.OperationName = TEXT("CommitMeasurementDecision");
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApprovalRequired;
		OutResult.Message = TEXT("Prepared Measurement approval이 없습니다. 다시 검토하세요.");
		LastMessage = OutResult.Message;
		return false;
	}
	// UI가 review한 exact R2 request를 복사한 commit request입니다.
	FCFVehicleMeasurementRequest Request = PreparedMeasurementRequest;
	Request.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	Request.CallContext.ApprovalScopeHash = PreparedMeasurementProposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = PreparedMeasurementProposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = PreparedMeasurementProposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = PreparedMeasurementProposal.ResolverContractRevision;
	bHasPreparedMeasurement = false;
	PreparedMeasurementRequest = FCFVehicleMeasurementRequest();
	PreparedMeasurementProposal = FCFAuthoringProposal();
	// Common facade R2 Recipe-only mutation입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::CommitMeasurementDecision(Request, OutResult);
	LastMessage = OutResult.Message;
	InvalidatePreparedApply();
	if (bSucceeded && OutResult.Mutation.bRecipeChanged)
	{
		MarkWorkspaceTransaction(TEXT("Wheel Measurement 결정"));
	}
	if (bSucceeded)
	{
		// Measurement decision commit 뒤 full preview/measurement을 fresh read합니다.
		FString RefreshError;
		RefreshPreview(RefreshError);
	}
	return bSucceeded;
}

// Legacy Pin group Adoption을 mutation0 prospective R2 proposal로 준비합니다.
bool FCFVehicleAuthoringVM::PrepareGroupAdoption(const ECFVehicleAdoptGroup AdoptionGroup, FCFVehicleAdoptionPreviewResult& OutPreview)
{
	PreparedAdoptionRequest = FCFVehicleAdoptionRequest();
	PreparedAdoptionProposal = FCFAuthoringProposal();
	bHasPreparedAdoption = false;
	if (!Recipe.IsValid() || !TargetVehicleData.IsValid())
	{
		LastMessage = TEXT("Adoption에는 managed Recipe/Target selection이 필요합니다.");
		return false;
	}
	// Group scope mutation0 Adoption preview request입니다.
	FCFVehicleAdoptionRequest Request;
	Request.Recipe = Recipe.Get();
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.Scope = ECFVehicleAdoptionScope::Group;
	Request.AdoptionGroup = AdoptionGroup;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	// Existing Import Core를 facade로 감싼 prospective result입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::PreviewAdoption(Request, OutPreview);
	LastMessage = OutPreview.Operation.Message;
	if (bSucceeded)
	{
		PreparedAdoptionRequest = Request;
		PreparedAdoptionProposal = OutPreview.Proposal;
		bHasPreparedAdoption = true;
	}
	return bSucceeded;
}

// UI가 실제 검토한 prepared Adoption R2 proposal만 existing Import Core를 통해 Recipe-only commit합니다.
bool FCFVehicleAuthoringVM::ExecutePreparedAdoption(FCFAuthoringOpResult& OutResult)
{
	OutResult = FCFAuthoringOpResult();
	if (!bHasPreparedAdoption)
	{
		OutResult.OperationName = TEXT("CommitAdoption");
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApprovalRequired;
		OutResult.Message = TEXT("Prepared Adoption approval이 없습니다. 다시 검토하세요.");
		LastMessage = OutResult.Message;
		return false;
	}
	// UI가 review한 exact R2 Adoption commit request입니다.
	FCFVehicleAdoptionRequest Request = PreparedAdoptionRequest;
	Request.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	Request.CallContext.ApprovalScopeHash = PreparedAdoptionProposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = PreparedAdoptionProposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = PreparedAdoptionProposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = PreparedAdoptionProposal.ResolverContractRevision;
	bHasPreparedAdoption = false;
	PreparedAdoptionRequest = FCFVehicleAdoptionRequest();
	PreparedAdoptionProposal = FCFAuthoringProposal();
	// Existing Import Core shared facade commit result입니다.
	const bool bSucceeded = FCFVehicleAuthoringService::CommitAdoption(Request, OutResult);
	LastMessage = OutResult.Message;
	InvalidatePreparedApply();
	if (bSucceeded && OutResult.Mutation.bRecipeChanged)
	{
		MarkWorkspaceTransaction(TEXT("Legacy Source Adoption"));
	}
	if (bSucceeded)
	{
		// Ownership transition 뒤 fresh effective winner/diff를 다시 읽습니다.
		FString RefreshError;
		RefreshPreview(RefreshError);
	}
	return bSucceeded;
}

// Unmanaged selection을 위한 exact Initial Import R2 proposal을 facade에서 만듭니다.
bool FCFVehicleAuthoringVM::BuildInitialImportPreview(
	const FString& RecipePackagePath,
	const FName RecipeAssetName,
	FCFVehicleInitialImportPreviewResult& OutPreview)
{
	OutPreview = FCFVehicleInitialImportPreviewResult();
	PreparedInitialImportRequest = FCFVehicleInitialImportRequest();
	PreparedInitialImportProposal = FCFAuthoringProposal();
	bHasPreparedInitialImport = false;
	if (!bHasSelection || !TargetVehicleData.IsValid() || Recipe.IsValid())
	{
		LastMessage = TEXT("Initial Import Preview는 unmanaged Vehicle selection에서만 가능합니다.");
		return false;
	}

	// Facade reviewed Initial Import request입니다.
	FCFVehicleInitialImportRequest Request;
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.RecipePackagePath = RecipePackagePath;
	Request.RecipeAssetName = RecipeAssetName;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
		const bool bSucceeded = FCFVehicleAuthoringService::BuildInitialImportProposal(Request, OutPreview);
	LastMessage = OutPreview.Operation.Message;
	if (bSucceeded)
	{
		PreparedInitialImportRequest = Request;
		PreparedInitialImportProposal = OutPreview.Proposal;
		bHasPreparedInitialImport = true;
	}
	return bSucceeded;
}

// Reviewed Initial Import proposal을 OwnershipWrite로 commit하고 생성된 Recipe를 current selection에 연결합니다.
bool FCFVehicleAuthoringVM::CommitInitialImport(
	const FString& RecipePackagePath,
	const FName RecipeAssetName,
	FCFVehicleInitialImportResult& OutResult)
{
		OutResult = FCFVehicleInitialImportResult();
	if (!bHasPreparedInitialImport
		|| PreparedInitialImportRequest.TargetVehicleData != TargetVehicleData.Get()
		|| PreparedInitialImportRequest.RecipePackagePath != RecipePackagePath
		|| PreparedInitialImportRequest.RecipeAssetName != RecipeAssetName)
	{
		OutResult.Operation.OperationName = TEXT("ImportExistingDefinition");
		OutResult.Operation.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::ApprovalRequired;
		OutResult.Operation.Message = TEXT("현재 입력과 exact match하는 reviewed Initial Import proposal이 없습니다. 다시 검토하세요.");
		LastMessage = OutResult.Operation.Message;
		return false;
	}

	// UI가 dialog에서 실제 검토한 exact R2 request를 복사하고 approval metadata만 붙입니다.
	FCFVehicleInitialImportRequest Request = PreparedInitialImportRequest;
	Request.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	Request.CallContext.ApprovalScopeHash = PreparedInitialImportProposal.ProposalHash;
	Request.CallContext.ExpectedTargetDefinitionHash = PreparedInitialImportProposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = PreparedInitialImportProposal.ResolverContractRevision;
	PreparedInitialImportRequest = FCFVehicleInitialImportRequest();
	PreparedInitialImportProposal = FCFAuthoringProposal();
	bHasPreparedInitialImport = false;
	const bool bSucceeded = FCFVehicleAuthoringService::ImportExistingDefinition(Request, OutResult);
	LastMessage = OutResult.Operation.Message;
	if (!bSucceeded || !OutResult.Recipe)
	{
		return false;
	}

	Recipe = OutResult.Recipe;
	SelectedEntry.RecipePath = OutResult.RecipePath;
	SelectedEntry.RecipeId = OutResult.Recipe->RecipeId;
	SelectedEntry.ManageState = OutResult.Recipe->ImportState.ManageState;
	ManagementView = CFVehicleAuthoringVMPrivate::ToManageView(OutResult.Recipe->ImportState.ManageState);
	InvalidatePreparedApply();
	// Initial Import 뒤 Target은 그대로이고 newly managed Recipe truth를 fresh resolve합니다.
	FString RefreshError;
	RefreshPreview(RefreshError);
	return true;
}

// Current fresh ResolveResult에서 exact R3 ApplyRequest/approval scope를 준비합니다.
bool FCFVehicleAuthoringVM::PrepareApply(FCFAuthoringOpResult& OutResult)
{
	OutResult = FCFAuthoringOpResult();
	InvalidatePreparedApply();
	if (!CanApply())
	{
		OutResult.OperationName = TEXT("PrepareVehicleApply");
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ValidationBlocked;
		OutResult.Message = TEXT("Apply 조건을 만족하지 않습니다. Fresh Preview/Diff/Validation/External Drift 상태를 확인하세요.");
		LastMessage = OutResult.Message;
		return false;
	}

	// Cached fresh Resolve authority를 exact shared ApplyRequest로 변환한 값입니다.
	FCFVehicleApplyRequest ApplyRequest;
	// ApplyRequest 구성 diagnostic입니다.
	FString ApplyError;
	if (!BuildCurrentApplyRequest(ApplyRequest, ApplyError))
	{
		OutResult.OperationName = TEXT("PrepareVehicleApply");
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApplyPreconditionFailed;
		OutResult.Message = ApplyError;
		LastMessage = ApplyError;
		return false;
	}

	if (!FCFVehicleAuthoringService::BuildApplyApprovalProposal(ApplyRequest, PreparedApplyProposal, OutResult))
	{
		LastMessage = OutResult.Message;
		return false;
	}

	PreparedApplyRequest = FCFVehicleApplyOpRequest();
	PreparedApplyRequest.ApplyRequest = ApplyRequest;
	PreparedApplyRequest.ExpectedDiffHash = PreparedApplyProposal.DiffHash;
	PreparedApplyRequest.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	PreparedApplyRequest.CallContext.CallerKind = ECFAuthoringCallerKind::SlateUI;
	PreparedApplyRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	PreparedApplyRequest.CallContext.ApprovalScopeHash = PreparedApplyProposal.ProposalHash;
	PreparedApplyRequest.CallContext.ExpectedRecipeFingerprint = PreparedApplyProposal.ExpectedRecipeFingerprint;
	PreparedApplyRequest.CallContext.ExpectedTargetDefinitionHash = PreparedApplyProposal.ExpectedTargetDefinitionHash;
	PreparedApplyRequest.CallContext.ExpectedResolverContractRevision = PreparedApplyProposal.ResolverContractRevision;
	bHasPreparedApply = true;
	LastMessage = TEXT("Fresh Preview exact Diff에 binding된 Definition Apply approval을 준비했습니다.");
	return true;
}

// 준비된 exact R3 approval을 facade shared Apply lane으로 한 번 실행하고 current state를 refresh합니다.
bool FCFVehicleAuthoringVM::ExecutePreparedApply(FCFAuthoringOpResult& OutResult)
{
	OutResult = FCFAuthoringOpResult();
	if (!bHasPreparedApply)
	{
		OutResult.OperationName = TEXT("ApplyResolvedVehicle");
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApprovalRequired;
		OutResult.Message = TEXT("Prepared Definition Apply approval이 없습니다. Fresh Preview에서 다시 준비하세요.");
		LastMessage = OutResult.Message;
		return false;
	}

	// stale이어도 자동 재준비하지 않고 exact prepared request를 한 번 전달합니다.
	const bool bSucceeded = FCFVehicleAuthoringService::ApplyResolvedVehicle(PreparedApplyRequest, OutResult);
	LastMessage = OutResult.Message;
	InvalidatePreparedApply();
		if (bSucceeded)
	{
		if (OutResult.Mutation.bTargetChanged || OutResult.Mutation.bRecipeChanged)
		{
			MarkWorkspaceTransaction(TEXT("Definition Apply"));
		}
		// Apply transaction readback을 UI에 반영하는 fresh read입니다.
		FString RefreshError;
		RefreshPreview(RefreshError);
	}
	else
	{
		PreviewView = ECFWorkspacePreviewView::OutOfDate;
		SyncView = ECFWorkspaceSyncView::PreviewOutOfDate;
	}
	return bSucceeded;
}

// Current Target VehicleData를 Unreal 표준 Asset Editor에 여는 navigation action입니다.
bool FCFVehicleAuthoringVM::OpenRawVehicleData(FString& OutError) const
{
	if (!TargetVehicleData.IsValid() || !GEditor)
	{
		OutError = TEXT("Raw VehicleData를 열 current Target 또는 Editor subsystem이 없습니다.");
		return false;
	}
	// Unreal 표준 Asset Editor navigation subsystem입니다.
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		OutError = TEXT("AssetEditorSubsystem을 가져올 수 없습니다.");
		return false;
	}
	AssetEditorSubsystem->OpenEditorForAsset(TargetVehicleData.Get());
	OutError.Reset();
	return true;
}

// Last Workspace transaction이 현재 UE Undo stack의 exact top인지 확인해 standard Undo 가능 여부를 반환합니다.
bool FCFVehicleAuthoringVM::CanUndoLastWorkspaceAction() const
{
	if (!bCanUndoLastWorkspaceAction || !LastWorkspaceTransactionId.IsValid() || !GEditor || !GEditor->Trans)
	{
		return false;
	}
	// 사용자가 지금 Undo하면 실제로 되돌릴 top transaction identity입니다. Undo 가능 여부 자체는 최종 UndoTransaction()이 판정합니다.
		const FTransactionContext UndoContext = GEditor->Trans->GetUndoContext(false);
	return UndoContext.TransactionId.IsValid() && UndoContext.TransactionId == LastWorkspaceTransactionId;
}

// 마지막으로 이 Workspace가 성공시킨 Recipe/Apply transaction을 exact UE TransactionId 확인 뒤 표준 Undo하고 fresh preview를 읽습니다.
bool FCFVehicleAuthoringVM::UndoLastWorkspaceAction(FString& OutError)
{
	if (!bCanUndoLastWorkspaceAction || !LastWorkspaceTransactionId.IsValid() || !GEditor || !GEditor->Trans)
	{
		OutError = TEXT("현재 Workspace가 확실히 소유한 마지막 transaction이 없습니다. 일반 Ctrl+Z는 Unreal 표준 Undo 기록을 따릅니다.");
		LastMessage = OutError;
		return false;
	}
	// 사용자가 지금 Undo하면 실제로 되돌릴 top transaction identity입니다. 다른 transaction을 Undo하지 않기 위한 exact guard입니다.
		const FTransactionContext UndoContext = GEditor->Trans->GetUndoContext(false);
	if (!UndoContext.TransactionId.IsValid() || UndoContext.TransactionId != LastWorkspaceTransactionId)
	{
		OutError = TEXT("Workspace 작업 이후 다른 Editor 변경이 있어 안전한 '마지막 작업 되돌리기'를 차단했습니다. 일반 Ctrl+Z/Redo 기록을 먼저 확인하세요.");
		LastMessage = OutError;
		return false;
	}
	// Exact Workspace TransactionId가 top일 때만 실행하는 Unreal 표준 Undo 결과입니다.
	const bool bUndoSucceeded = GEditor->UndoTransaction();
	bCanUndoLastWorkspaceAction = false;
	LastWorkspaceTransactionId.Invalidate();
	if (!bUndoSucceeded)
	{
		OutError = TEXT("Unreal 표준 Undo가 마지막 Workspace transaction을 되돌리지 못했습니다.");
		LastMessage = OutError;
		return false;
	}
	// Undo 뒤 cached approval/reference output을 무효화하고 current truth를 fresh read합니다.
	InvalidatePreparedApply();
	bHasPreparedMeasurement = false;
	bHasPreparedAdoption = false;
	ReferenceCompareResult = FCFVehicleReferenceCompareResult();
	LastMessage = FString::Printf(TEXT("Unreal 표준 Undo 완료: %s"), *LastWorkspaceActionDescription);
	LastWorkspaceActionDescription.Reset();
	if (Recipe.IsValid())
	{
		// Undo 결과에 맞춘 fresh full preview diagnostic입니다.
		FString RefreshError;
		if (!RefreshPreview(RefreshError))
		{
			OutError = RefreshError;
			return false;
		}
	}
	OutError.Reset();
	return true;
}

// Current transient selection/cache/approval을 모두 지웁니다.
void FCFVehicleAuthoringVM::ClearSelection()
{
	SelectedEntry = FCFVehicleListEntry();
	TargetVehicleData.Reset();
	Recipe.Reset();
	bHasSelection = false;
	ContextResult = FCFVehicleContextReadResult();
	ResolveResult = FCFVehicleResolveReadResult();
	DiffResult = FCFVehicleDiffReadResult();
	TraceResult = FCFVehicleTraceReadResult();
		ValidationResult = FCFVehicleValidationReadResult();
	ReferenceEntry = FCFVehicleListEntry();
	bHasReferenceVehicle = false;
	ReferenceCompareResult = FCFVehicleReferenceCompareResult();
	MeasurementResult = FCFVehicleMeasurementReadResult();
	PreparedMeasurementRequest = FCFVehicleMeasurementRequest();
	PreparedMeasurementProposal = FCFAuthoringProposal();
	bHasPreparedMeasurement = false;
		PreparedAdoptionRequest = FCFVehicleAdoptionRequest();
	PreparedAdoptionProposal = FCFAuthoringProposal();
		bHasPreparedAdoption = false;
	ProfileChoices.Reset();
	PreparedProfileEditRequest = FCFProfileNumericEditRequest();
	PreparedProfileEditPreview = FCFProfileNumericEditPreview();
	bHasPreparedProfileEdit = false;
	PreparedVehicleCreateRequest = FCFVehicleRecordCreateRequest();
	PreparedVehicleCreatePreview = FCFVehicleRecordCreatePreview();
	bHasPreparedVehicleCreate = false;
	InvalidateDriftReviewState();
		bCanUndoLastWorkspaceAction = false;
	LastWorkspaceTransactionId.Invalidate();
	LastWorkspaceActionDescription.Reset();
	ManagementView = ECFWorkspaceManageView::Unmanaged;
	SyncView = ECFWorkspaceSyncView::NoBaseline;
	ValidationView = ECFWorkspaceValidView::NotEvaluated;
	PreviewView = ECFWorkspacePreviewView::NeedsRefresh;
		LastMessage.Reset();
	PreparedInitialImportRequest = FCFVehicleInitialImportRequest();
	PreparedInitialImportProposal = FCFAuthoringProposal();
	bHasPreparedInitialImport = false;
	InvalidatePreparedApply();
}

// Bottom Action Bar의 normal Apply enable 조건을 반환합니다.
bool FCFVehicleAuthoringVM::CanApply() const
{
	// External Drift가 없거나 exact current 3-way evidence에 binding된 Keep Authoring review가 있어야 합니다.
	const bool bExternalDriftReviewed = !ResolveResult.ResolveResult.StaleReport.bHasExternalDrift || HasAcceptedDriftKeep();
	return bHasSelection
		&& TargetVehicleData.IsValid()
		&& Recipe.IsValid()
		&& IsPreviewFresh()
		&& !DiffResult.FieldDiff.IsEmpty()
		&& !ValidationResult.Operation.ValidationSummary.bApplyBlocking
		&& bExternalDriftReviewed
		&& ResolveResult.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success;
}

// Current managed Recipe/Target에서 facade read request를 만듭니다.
FCFVehicleAuthoringReadRequest FCFVehicleAuthoringVM::BuildReadRequest() const
{
	// 모든 UI read operation이 공유할 facade request입니다.
	FCFVehicleAuthoringReadRequest Request;
	Request.Recipe = Recipe.Get();
	Request.TargetVehicleData = TargetVehicleData.Get();
	Request.CallerKind = ECFAuthoringCallerKind::SlateUI;
	return Request;
}

// Current ResolveResult를 기존 shared ApplyRequest 구조로 포장합니다.
bool FCFVehicleAuthoringVM::BuildCurrentApplyRequest(FCFVehicleApplyRequest& OutApplyRequest, FString& OutError) const
{
	OutApplyRequest = FCFVehicleApplyRequest();
	if (!CanApply())
	{
		OutError = TEXT("Current ViewModel state가 exact ApplyRequest를 만들 수 있는 Fresh Apply state가 아닙니다.");
		return false;
	}
	OutApplyRequest.Recipe = Recipe.Get();
	OutApplyRequest.TargetVehicleData = TargetVehicleData.Get();
	OutApplyRequest.ResolveRequest = ResolveResult.ResolveRequest;
	OutApplyRequest.ApprovedResolveResult = ResolveResult.ResolveResult;
	OutApplyRequest.ExpectedRecipeFingerprint = ResolveResult.ResolveRequest.Recipe.RecipeFingerprint;
	OutApplyRequest.ExpectedSourceSignature = ResolveResult.ResolveResult.SourceSignature;
	OutApplyRequest.ExpectedTargetDefinitionHash = ResolveResult.ResolveRequest.CurrentDefinition.DefinitionHash;
	OutApplyRequest.ExpectedResolvedDefinitionHash = ResolveResult.ResolveResult.ResolvedDefinitionHash;
	OutApplyRequest.ExpectedResolverContractRevision = ResolveResult.ResolveResult.ResolverContractRevision;
	OutError.Reset();
	return true;
}

// Resolver/ImportState 결과에서 Management/Sync/Validation UI 상태만 파생합니다.
void FCFVehicleAuthoringVM::RefreshDerivedViews()
{
	if (!Recipe.IsValid())
	{
		ManagementView = ECFWorkspaceManageView::Unmanaged;
		SyncView = ECFWorkspaceSyncView::NoBaseline;
		ValidationView = ECFWorkspaceValidView::NotEvaluated;
		return;
	}
	ManagementView = CFVehicleAuthoringVMPrivate::ToManageView(Recipe->ImportState.ManageState);
	if (PreviewView == ECFWorkspacePreviewView::OutOfDate)
	{
		SyncView = ECFWorkspaceSyncView::PreviewOutOfDate;
	}
	else if (ResolveResult.ResolveResult.StaleReport.bHasExternalDrift)
	{
		SyncView = ECFWorkspaceSyncView::ExternalDrift;
	}
	else if (ResolveResult.ResolveResult.StaleReport.bHasEffectiveStale)
	{
		SyncView = ECFWorkspaceSyncView::EffectiveStale;
	}
	else if (ResolveResult.ResolveResult.StaleReport.bHasShadowSourceChange)
	{
		SyncView = ECFWorkspaceSyncView::ShadowChanged;
	}
	else
	{
		SyncView = ECFWorkspaceSyncView::InSync;
	}

	// Facade가 이미 계산한 validation compact summary입니다.
	const FCFAuthoringValidationSummary& Summary = ValidationResult.Operation.ValidationSummary;
	if (Summary.ErrorCount > 0 || Summary.BlockedCount > 0)
	{
		ValidationView = ECFWorkspaceValidView::Blocked;
	}
	else if (Summary.WarningCount > 0)
	{
		ValidationView = ECFWorkspaceValidView::Warning;
	}
	else
	{
		ValidationView = ECFWorkspaceValidView::Valid;
	}
}

// Recipe/Target persistent state가 달라질 수 있는 action 뒤 stale prepared approval을 폐기합니다.
void FCFVehicleAuthoringVM::InvalidatePreparedApply()
{
	PreparedApplyRequest = FCFVehicleApplyOpRequest();
	PreparedApplyProposal = FCFAuthoringProposal();
	bHasPreparedApply = false;
}

// 성공한 Workspace-owned transaction 뒤 UE Undo stack top의 exact TransactionId를 캡처해 안전한 Undo 후보로 기록합니다.
void FCFVehicleAuthoringVM::MarkWorkspaceTransaction(const FString& ActionDescription)
{
	bCanUndoLastWorkspaceAction = false;
	LastWorkspaceTransactionId.Invalidate();
	LastWorkspaceActionDescription.Reset();
	if (!GEditor || !GEditor->Trans)
	{
		return;
	}
	// 방금 완료된 Workspace mutation 뒤 현재 UE Undo stack top transaction identity입니다.
		const FTransactionContext UndoContext = GEditor->Trans->GetUndoContext(false);
	if (!UndoContext.TransactionId.IsValid())
	{
		return;
	}
	LastWorkspaceTransactionId = UndoContext.TransactionId;
	LastWorkspaceActionDescription = ActionDescription;
	bCanUndoLastWorkspaceAction = LastWorkspaceTransactionId.IsValid();
}
