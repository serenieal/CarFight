// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleUXOps.cpp
// Version: v1.1.0
// Date: 2026-08-18
// Description: DAUTH-P0-11 Frozen 24.91~24.94 normal Workspace completeness facade 구현입니다.
// Scope: existing Batch B2 Profile edit, External Drift 3-way recovery, Definition+Recipe record creation을 Common Authoring facade에 얇게 연결합니다.
// Changelog:
// - v1.1.0: Drift prospective simulation용 UObject duplicate가 PostDuplicate에서 새 RecipeId를 발급해 approval hash가 비결정적이던 문제를 원본 identity 복원으로 교정.
// - v1.0.0: Shared Profile impact/commit, Drift review/decision, New Vehicle/Mesh-only two-record creation 최초 구현.
// Migration:
// - Profile write는 FCFBatchImportService B2만 사용하고 Target Definition Apply는 수행하지 않습니다.
// - Drift Preserve Raw는 FCFVehicleImportService ownership primitive, Advanced는 Registry allowlist를 사용합니다.
// - record creation은 새 Definition+Recipe만 만들며 Apply/Save/Profile/class/physics inference를 수행하지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFBatchColumnRegistry.h"
#include "DataAuthoring/CFBatchExport.h"
#include "DataAuthoring/CFBatchImport.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "Engine/StaticMesh.h"
#include "Misc/PackageName.h"
#include "Misc/SecureHash.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleUXOpsPrivate
{
	// Common typed result를 operation 시작 상태로 초기화합니다.
		void InitializeResult(FCFAuthoringOpResult& OutResult, const FName OperationName, const ECFAuthoringRiskClass RiskClass)
	{
		OutResult = FCFAuthoringOpResult();
		OutResult.OperationName = OperationName;
		OutResult.RiskClass = RiskClass;
	}

	// Common typed result를 fail-closed Blocked 상태로 설정합니다.
	void SetBlocked(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
	}

	// Common typed result를 성공 상태로 설정합니다.
	void SetSucceeded(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Succeeded;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
	}

	// Common typed result를 NoChange 상태로 설정합니다.
	void SetNoChange(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::NoChange;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
	}

	// Delimiter 충돌 없는 canonical hash token을 누적합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("|");
	}

	// UTF-8 payload를 lowercase MD5 hexadecimal digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR payload의 canonical UTF-8 bytes입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Deterministic digest state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// Final MD5 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);
		// Lowercase hexadecimal result입니다.
		FString Result;
		Result.Reserve(32);
		// Fixed hexadecimal lookup table입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			Result.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			Result.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return Result;
	}

	// Exact Stable Field Path가 Registry wildcard descriptor와 일치하는지 확인합니다.
	bool DoesPathMatchDescriptor(const FCFVehicleFieldPath& ExactPath, const FCFVehicleFieldDescriptor& Descriptor)
	{
		// Registry wildcard path pattern입니다.
		const FCFVehicleFieldPath& Pattern = Descriptor.StablePathPattern;
		if (ExactPath.CollectionPropertyName != Pattern.CollectionPropertyName
			|| ExactPath.SelectorKeyPropertyName != Pattern.SelectorKeyPropertyName
			|| ExactPath.PropertyChain != Pattern.PropertyChain)
		{
			return false;
		}
		if (Pattern.CollectionPropertyName.IsNone())
		{
			return ExactPath.SelectorKeyValue.IsNone();
		}
		return !ExactPath.SelectorKeyValue.IsNone();
	}

	// Exact Stable Field Path를 owning Registry descriptor로 resolve합니다.
	const FCFVehicleFieldDescriptor* FindDescriptor(const FCFVehicleFieldPath& FieldPath)
	{
		for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
		{
			if (DoesPathMatchDescriptor(FieldPath, Descriptor))
			{
				return &Descriptor;
			}
		}
		return nullptr;
	}

	// Definition Snapshot에서 exact path의 typed value를 찾습니다.
	const FCFVehicleFieldValue* FindDefinitionValue(const FCFVehicleDefinitionSnapshot& Snapshot, const FCFVehicleFieldPath& FieldPath)
	{
		// Exact canonical identity입니다.
		const FString CanonicalPath = FieldPath.ToCanonicalString(true);
				for (const FCFVehicleFieldEntry& Field : Snapshot.SortedFields)
		{
			if (Field.FieldPath.ToCanonicalString(true) == CanonicalPath)
			{
				return &Field.Value;
			}
		}
		return nullptr;
	}

	// Resolve result에서 exact path의 effective typed value를 찾습니다.
	const FCFVehicleFieldValue* FindResolvedValue(const FCFVehicleResolveResult& ResolveResult, const FCFVehicleFieldPath& FieldPath)
	{
		// Exact canonical identity입니다.
		const FString CanonicalPath = FieldPath.ToCanonicalString(true);
		for (const FCFVehicleResolvedField& Field : ResolveResult.SortedResolvedFields)
		{
			if (Field.FieldPath.ToCanonicalString(true) == CanonicalPath)
			{
				return &Field.Value;
			}
		}
		return nullptr;
	}

	// Recipe AppliedState에서 exact path의 Last Applied trace를 찾습니다.
	const FCFVehicleAppliedTrace* FindAppliedTrace(const UCFVehicleRecipeData& Recipe, const FCFVehicleFieldPath& FieldPath)
	{
		// Exact canonical identity입니다.
		const FString CanonicalPath = FieldPath.ToCanonicalString(true);
		for (const FCFVehicleAppliedTrace& Trace : Recipe.AppliedState.FieldTraces)
		{
			if (Trace.FieldPath.ToCanonicalString(true) == CanonicalPath)
			{
				return &Trace;
			}
		}
		return nullptr;
	}

	// Drift decision path set을 canonical ascending unique order로 정규화합니다.
	TArray<FString> BuildCanonicalPathSet(const TArray<FCFVehicleFieldPath>& FieldPaths)
	{
		// Duplicate 제거와 안정 정렬에 사용할 canonical identities입니다.
		TSet<FString> UniquePaths;
		for (const FCFVehicleFieldPath& FieldPath : FieldPaths)
		{
			UniquePaths.Add(FieldPath.ToCanonicalString(true));
		}
		// Proposal hash용 ascending canonical paths입니다.
		TArray<FString> SortedPaths = UniquePaths.Array();
		SortedPaths.Sort();
		return SortedPaths;
	}

	// Drift review에서 exact canonical path row를 찾습니다.
	const FCFVehicleDriftReviewRow* FindReviewRow(const FCFVehicleDriftReviewResult& Review, const FString& CanonicalPath)
	{
		return Review.Rows.FindByPredicate([&CanonicalPath](const FCFVehicleDriftReviewRow& Row)
		{
			return Row.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// Recipe AdvancedOverrides에 exact path override를 deterministic upsert합니다.
	bool UpsertAdvancedOverride(
		UCFVehicleRecipeData& Recipe,
		const FCFVehicleDriftReviewRow& ReviewRow,
		const FString& Reason,
		FString& OutError)
	{
		// Frozen Registry ownership descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(ReviewRow.FieldPath);
		if (!Descriptor || !Descriptor->bAdvancedOverrideAllowed)
		{
			OutError = FString::Printf(TEXT("Advanced Override가 허용되지 않는 field입니다: %s"), *ReviewRow.FieldPath.ToCanonicalString(true));
			return false;
		}
		if (Reason.TrimStartAndEnd().IsEmpty())
		{
			OutError = TEXT("Advanced Override에는 이유가 필요합니다.");
			return false;
		}

		// Current Raw를 보존하는 exact override value입니다.
		FCFVehicleFieldOverride Override;
		Override.FieldPath = ReviewRow.FieldPath;
		Override.OverrideValue = ReviewRow.CurrentRawValue;
		Override.Reason = Reason.TrimStartAndEnd();
		// Existing exact override index입니다.
		const FString CanonicalPath = ReviewRow.FieldPath.ToCanonicalString(true);
		const int32 ExistingIndex = Recipe.AdvancedOverrides.IndexOfByPredicate([&CanonicalPath](const FCFVehicleFieldOverride& Existing)
		{
			return Existing.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
		if (ExistingIndex != INDEX_NONE)
		{
			Recipe.AdvancedOverrides[ExistingIndex] = Override;
		}
		else
		{
			Recipe.AdvancedOverrides.Add(Override);
		}
		Recipe.AdvancedOverrides.Sort([](const FCFVehicleFieldOverride& Left, const FCFVehicleFieldOverride& Right)
		{
			return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
		});
		OutError.Reset();
		return true;
	}

	// Drift decision과 current review evidence를 exact R2 proposal hash로 binding합니다.
	FString BuildDriftProposalHash(
		const FCFVehicleDriftDecisionRequest& Request,
		const FCFVehicleDriftReviewResult& Review,
		const FCFVehicleResolveResult& ProspectiveResolve)
	{
		// Deterministic proposal payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("op"), TEXT("ResolveExternalDrift"));
		AppendToken(Payload, TEXT("decision"), FString::FromInt(static_cast<int32>(Request.Decision)));
		AppendToken(Payload, TEXT("recipe"), Review.ExpectedRecipeFingerprint);
		AppendToken(Payload, TEXT("target"), Review.ExpectedTargetDefinitionHash);
		AppendToken(Payload, TEXT("source"), Review.ExpectedSourceSignature);
		AppendToken(Payload, TEXT("revision"), FString::FromInt(Review.ResolverContractRevision));
		AppendToken(Payload, TEXT("reason"), Request.OverrideReason.TrimStartAndEnd());
		for (const FString& CanonicalPath : BuildCanonicalPathSet(Request.FieldPaths))
		{
			AppendToken(Payload, TEXT("field"), CanonicalPath);
			// Proposal이 current raw value에도 binding되게 하는 exact row입니다.
			const FCFVehicleDriftReviewRow* Row = FindReviewRow(Review, CanonicalPath);
			AppendToken(Payload, TEXT("raw"), Row ? FCFVehicleFieldCodec::HashValue(Row->CurrentRawValue) : FString());
		}
		AppendToken(Payload, TEXT("prospectiveSource"), ProspectiveResolve.SourceSignature);
		AppendToken(Payload, TEXT("prospectiveResolved"), ProspectiveResolve.ResolvedDefinitionHash);
		return HashUtf8Payload(Payload);
	}

	// Two-record creation request를 exact R2 proposal hash로 binding합니다.
	FString BuildRecordCreateHash(const FCFVehicleRecordCreateRequest& Request, const FSoftObjectPath& DefinitionPath, const FSoftObjectPath& RecipePath)
	{
		// Deterministic creation payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("op"), TEXT("CreateVehicleRecords"));
		AppendToken(Payload, TEXT("definition"), DefinitionPath.ToString());
		AppendToken(Payload, TEXT("recipe"), RecipePath.ToString());
		AppendToken(Payload, TEXT("chassis"), Request.ChassisMesh.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("base"), Request.ProfileBindings.VehicleBaseProfile.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("drive"), Request.ProfileBindings.DrivetrainProfile.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("handling"), Request.ProfileBindings.HandlingProfile.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("performance"), Request.ProfileBindings.PerformanceProfile.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("driveState"), Request.ProfileBindings.DriveStateProfile.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("revision"), FString::FromInt(FCFVehicleResolver::CurrentResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// New asset package/object pair가 현재 memory/disk에 존재하지 않는지 fail-closed 검사합니다.
	bool ValidateNewAssetIdentity(const FString& PackageName, const FName AssetName, FSoftObjectPath& OutObjectPath, FString& OutError)
	{
		if (!FPackageName::IsValidLongPackageName(PackageName) || AssetName.IsNone())
		{
			OutError = TEXT("새 Vehicle record에는 유효한 /Game long package name과 asset name이 필요합니다.");
			return false;
		}
		// Expected object path text입니다.
		const FString ObjectPathText = PackageName + TEXT(".") + AssetName.ToString();
		OutObjectPath = FSoftObjectPath(ObjectPathText);
		if (!OutObjectPath.IsValid())
		{
			OutError = FString::Printf(TEXT("유효하지 않은 object path입니다: %s"), *ObjectPathText);
			return false;
		}
		if (FindPackage(nullptr, *PackageName) || FPackageName::DoesPackageExist(PackageName) || OutObjectPath.ResolveObject())
		{
			OutError = FString::Printf(TEXT("이미 존재하는 package/object에는 Vehicle record를 덮어쓸 수 없습니다: %s"), *ObjectPathText);
			return false;
		}
		OutError.Reset();
		return true;
	}
}

// Existing B2 ProfileNumericEdit pipeline으로 Shared Profile edit와 affected Vehicle impact를 mutation0 preview합니다.
bool FCFVehicleAuthoringService::PreviewProfileNumericEdit(
	const FCFProfileNumericEditRequest& Request,
	FCFProfileNumericEditPreview& OutPreview)
{
	OutPreview = FCFProfileNumericEditPreview();
	CFVehicleUXOpsPrivate::InitializeResult(OutPreview.Operation, TEXT("PreviewProfileNumericEdit"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (!Request.ProfileObject || Request.ProfileDomain == ECFVehicleProfileDomain::None || Request.ColumnId.IsEmpty())
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("Shared Profile preview에는 Profile/Domain/ColumnId가 필요합니다."));
		return false;
	}
	if (Request.CallerKind != ECFAuthoringCallerKind::SlateUI && Request.CallerKind != ECFAuthoringCallerKind::Automation)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::UnsupportedOperation, TEXT("Shared Profile payload edit는 normal SlateUI/Automation route만 허용합니다."));
		return false;
	}

	// Current Profile one-row export baseline입니다.
	FCFBatchExportRow ExportRow;
	// Existing Batch export diagnostics입니다.
	TArray<FString> Errors;
	if (!FCFBatchExportService::BuildProfileNumericRow(*Request.ProfileObject, Request.ProfileDomain, ExportRow, Errors))
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, Errors.IsEmpty() ? TEXT("Profile numeric row projection에 실패했습니다.") : Errors[0]);
		return false;
	}

	// Immutable ProfileNumericEdit export request입니다.
	FCFBatchExportRequest ExportRequest;
	ExportRequest.BatchExportId = FGuid::NewGuid();
	ExportRequest.DatasetKind = ECFBatchDatasetKind::ProfileNumericEdit;
	ExportRequest.ProfileDomain = Request.ProfileDomain;
	ExportRequest.Rows.Add(ExportRow);
	// Existing B2 manifest/CSV artifact입니다.
	FCFBatchExportArtifact Artifact;
	if (!FCFBatchExportService::BuildExport(ExportRequest, Artifact, Errors) || Artifact.Manifest.Rows.Num() != 1)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::InternalError, Errors.IsEmpty() ? TEXT("Profile B2 export baseline 생성에 실패했습니다.") : Errors[0]);
		return false;
	}

	// Exact manifest RowId입니다.
	const FString RowId = Artifact.Manifest.Rows[0].RowId;
	// Immutable baseline에서 exact one cell만 교체한 canonical edited CSV입니다.
	FString EditedCsvText;
	if (!FCFBatchExportService::BuildEditedCellCsv(Artifact, RowId, Request.ColumnId, Request.CanonicalNumericValue, EditedCsvText, Errors))
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::FieldNotAuthorable, Errors.IsEmpty() ? TEXT("Profile edit cell이 current allowlist에 없습니다.") : Errors[0]);
		return false;
	}

	// Existing 3-way import preview request입니다.
	FCFBatchImportRequest ImportRequest;
	ImportRequest.CsvText = EditedCsvText;
	ImportRequest.ManifestJsonText = Artifact.ManifestJsonText;
	if (!FCFBatchImportService::BuildPreview(ImportRequest, OutPreview.Session))
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::PreviewOutOfDate, TEXT("Shared Profile 3-way prospective Preview가 Blocked되었습니다."));
		return false;
	}

	// Existing B2 exact approval build diagnostics입니다.
	TArray<FCFBatchIssue> ApprovalIssues;
	if (!FCFBatchImportService::BuildCommitApproval(OutPreview.Session, OutPreview.Approval, ApprovalIssues))
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, TEXT("Shared Profile B2 approval을 만들 수 없는 conflict/validation 상태입니다."));
		return false;
	}

	// Affected Vehicle navigation duplicate 방지 key입니다.
	TSet<FString> AddedTargetPaths;
	for (const FCFBatchRowPreview& Row : OutPreview.Session.Rows)
	{
		for (const FCFBatchVehiclePreview& VehiclePreview : Row.VehiclePreviews)
		{
			if (AddedTargetPaths.Contains(VehiclePreview.TargetPath))
			{
				continue;
			}
			AddedTargetPaths.Add(VehiclePreview.TargetPath);
			// Normal Profile Editor에 노출할 affected Vehicle row입니다.
			FCFProfileImpactVehicle& Impact = OutPreview.AffectedVehicles.AddDefaulted_GetRef();
			Impact.RecipePath = VehiclePreview.RecipePath;
			Impact.TargetPath = VehiclePreview.TargetPath;
						Impact.PendingDefinitionChangeCount = VehiclePreview.DefinitionDiffCount;
			Impact.ResolveStatus = VehiclePreview.ResolveStatus;
		}
	}
	OutPreview.AffectedVehicles.Sort([](const FCFProfileImpactVehicle& Left, const FCFProfileImpactVehicle& Right)
	{
		return Left.TargetPath < Right.TargetPath;
	});
	OutPreview.AffectedVehicleCount = OutPreview.Session.Summary.AffectedVehicleCount;
		OutPreview.PendingDefinitionChangeCount = OutPreview.Session.Summary.ProspectiveDefinitionChangedFieldCount;
	CFVehicleUXOpsPrivate::SetSucceeded(OutPreview.Operation, TEXT("Shared Profile change를 B2 Core로 preview하고 affected Vehicle impact를 계산했습니다."));
	return true;
}

// UI가 review한 exact B2 Session/Approval만 existing Batch source commit에 위임합니다.
bool FCFVehicleAuthoringService::CommitProfileNumericEdit(
	const FCFProfileNumericEditRequest& Request,
	const FCFProfileNumericEditPreview& ApprovedPreview,
	FCFProfileNumericEditResult& OutResult)
{
	OutResult = FCFProfileNumericEditResult();
	CFVehicleUXOpsPrivate::InitializeResult(OutResult.Operation, TEXT("CommitProfileNumericEdit"), ECFAuthoringRiskClass::R1_AuthoringRecordWrite);
	if (Request.CallerKind != ECFAuthoringCallerKind::SlateUI && Request.CallerKind != ECFAuthoringCallerKind::Automation)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::UnsupportedOperation, TEXT("Shared Profile payload commit은 normal SlateUI/Automation route만 허용합니다."));
		return false;
	}

	// Existing B2 exact Session/Approval을 직접 참조하는 commit request입니다.
	FCFBatchAuthoringCommitRequest CommitRequest;
	CommitRequest.Session = &ApprovedPreview.Session;
	CommitRequest.Approval = &ApprovedPreview.Approval;
	CommitRequest.bAuthoringCommitApproved = true;
	if (!FCFBatchImportService::CommitAuthoringSources(CommitRequest, OutResult.BatchCommitResult))
	{
		switch (OutResult.BatchCommitResult.Status)
		{
		case ECFBatchCommitStatus::FailedRolledBack:
			OutResult.Operation.Status = ECFAuthoringOpStatus::FailedRolledBack;
			OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::InternalError;
			break;
		case ECFBatchCommitStatus::FailedUnknownState:
			OutResult.Operation.Status = ECFAuthoringOpStatus::FailedUnknownState;
			OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::InternalError;
			break;
		default:
			CFVehicleUXOpsPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::StateChanged, OutResult.BatchCommitResult.Message);
			break;
		}
		OutResult.Operation.Message = OutResult.BatchCommitResult.Message;
		return false;
	}

	if (OutResult.BatchCommitResult.Status == ECFBatchCommitStatus::NoChange)
	{
		CFVehicleUXOpsPrivate::SetNoChange(OutResult.Operation, OutResult.BatchCommitResult.Message);
		return true;
	}
	OutResult.Operation.Mutation.bProfileChanged = OutResult.BatchCommitResult.CommittedSourceObjectCount > 0;
	OutResult.Operation.Mutation.bPackageDirty = OutResult.Operation.Mutation.bProfileChanged;
	OutResult.Operation.Mutation.bTargetChanged = false;
	OutResult.Operation.Mutation.bSavePerformed = OutResult.BatchCommitResult.bSavePerformed;
	OutResult.Operation.Mutation.bAutomaticRetryPerformed = false;
	CFVehicleUXOpsPrivate::SetSucceeded(OutResult.Operation, OutResult.BatchCommitResult.Message);
	return true;
}

// Current External Drift를 Last Applied / Current Raw / Current Authoring 3-way rows로 구성합니다.
bool FCFVehicleAuthoringService::BuildDriftReview(
	const FCFVehicleAuthoringReadRequest& Request,
	FCFVehicleDriftReviewResult& OutReview)
{
	OutReview = FCFVehicleDriftReviewResult();
	CFVehicleUXOpsPrivate::InitializeResult(OutReview.Operation, TEXT("BuildDriftReview"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (!Request.Recipe)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutReview.Operation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("External Drift review에는 managed Recipe가 필요합니다."));
		return false;
	}

	// Existing shared Core의 fresh Resolver preview입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(Request, ResolveRead))
	{
		OutReview.Operation = ResolveRead.Operation;
		OutReview.Operation.OperationName = TEXT("BuildDriftReview");
		return false;
	}
	if (!ResolveRead.ResolveResult.StaleReport.bHasExternalDrift || !ResolveRead.ResolveRequest.bHasCurrentDefinition)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutReview.Operation, ECFAuthoringErrorCode::ExternalDriftUnresolved, TEXT("현재 review할 External Drift가 없습니다."));
		return false;
	}

	for (const FCFVehicleStaleField& StaleField : ResolveRead.ResolveResult.StaleReport.Fields)
	{
		if (!StaleField.bExternalDrift)
		{
			continue;
		}
		// Current Raw exact typed value입니다.
		const FCFVehicleFieldValue* RawValue = CFVehicleUXOpsPrivate::FindDefinitionValue(ResolveRead.ResolveRequest.CurrentDefinition, StaleField.FieldPath);
		// Current Authoring exact effective typed value입니다.
		const FCFVehicleFieldValue* AuthoringValue = CFVehicleUXOpsPrivate::FindResolvedValue(ResolveRead.ResolveResult, StaleField.FieldPath);
		if (!RawValue || !AuthoringValue)
		{
			CFVehicleUXOpsPrivate::SetBlocked(OutReview.Operation, ECFAuthoringErrorCode::InternalError, FString::Printf(TEXT("Drift 3-way value를 resolve할 수 없습니다: %s"), *StaleField.FieldPath.ToCanonicalString(true)));
			return false;
		}
		// Recipe-side Last Applied provenance입니다.
		const FCFVehicleAppliedTrace* AppliedTrace = CFVehicleUXOpsPrivate::FindAppliedTrace(*Request.Recipe, StaleField.FieldPath);
		// Frozen Registry capability입니다.
		const FCFVehicleFieldDescriptor* Descriptor = CFVehicleUXOpsPrivate::FindDescriptor(StaleField.FieldPath);
		// Normal Workspace 3-way row입니다.
		FCFVehicleDriftReviewRow& Row = OutReview.Rows.AddDefaulted_GetRef();
		Row.FieldPath = StaleField.FieldPath;
		Row.CurrentRawValue = *RawValue;
		Row.CurrentAuthoringValue = *AuthoringValue;
		Row.LastAppliedValueHash = AppliedTrace ? AppliedTrace->LastAppliedValueHash : StaleField.LastAppliedValueHash;
		Row.bHasLastAppliedValue = AppliedTrace && !AppliedTrace->LastAppliedValue.PropertyTypeSignature.IsEmpty();
		if (Row.bHasLastAppliedValue)
		{
			Row.LastAppliedValue = AppliedTrace->LastAppliedValue;
		}
		Row.bAdvancedOverrideAllowed = Descriptor && Descriptor->bAdvancedOverrideAllowed;
	}
	OutReview.Rows.Sort([](const FCFVehicleDriftReviewRow& Left, const FCFVehicleDriftReviewRow& Right)
	{
		return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
	});
	OutReview.ExpectedRecipeFingerprint = ResolveRead.ResolveRequest.Recipe.RecipeFingerprint;
	OutReview.ExpectedTargetDefinitionHash = ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash;
	OutReview.ExpectedSourceSignature = ResolveRead.ResolveResult.SourceSignature;
	OutReview.ResolverContractRevision = ResolveRead.ResolveResult.ResolverContractRevision;
	CFVehicleUXOpsPrivate::SetSucceeded(OutReview.Operation, TEXT("External Drift 3-way review를 fresh current truth에서 구성했습니다."));
	return true;
}

// Keep/Rebase/Advanced Override decision을 current evidence에 binding하고 mutation0 prospective resolve합니다.
bool FCFVehicleAuthoringService::PreviewDriftDecision(
	const FCFVehicleDriftDecisionRequest& Request,
	FCFVehicleDriftDecisionPreview& OutPreview)
{
	OutPreview = FCFVehicleDriftDecisionPreview();
	if (!BuildDriftReview(Request.ReadRequest, OutPreview.Review))
	{
		return false;
	}
	// Canonical selected field set입니다.
	const TArray<FString> SelectedPaths = CFVehicleUXOpsPrivate::BuildCanonicalPathSet(Request.FieldPaths);
	if (SelectedPaths.IsEmpty())
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Review.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("Drift recovery에는 최소 1개 reviewed field가 필요합니다."));
		return false;
	}
	for (const FString& CanonicalPath : SelectedPaths)
	{
		// Fresh review에 실제 포함된 exact selected row입니다.
		const FCFVehicleDriftReviewRow* Row = CFVehicleUXOpsPrivate::FindReviewRow(OutPreview.Review, CanonicalPath);
		if (!Row)
		{
			CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Review.Operation, ECFAuthoringErrorCode::PreviewOutOfDate, FString::Printf(TEXT("선택한 field가 current External Drift review에 없습니다: %s"), *CanonicalPath));
			return false;
		}
		if (Request.Decision == ECFVehicleDriftDecision::PromoteRawToAdvancedOverride && (!Row->bAdvancedOverrideAllowed || Request.OverrideReason.TrimStartAndEnd().IsEmpty()))
		{
			CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Review.Operation, ECFAuthoringErrorCode::OverrideNotAllowed, FString::Printf(TEXT("Advanced Override 허용/이유 조건을 만족하지 않습니다: %s"), *CanonicalPath));
			return false;
		}
	}

	// Keep Authoring은 current Authoring을 그대로 prospective result로 사용합니다.
	FCFVehicleResolveReadResult CurrentResolve;
	if (!ResolveVehiclePreview(Request.ReadRequest, CurrentResolve))
	{
		OutPreview.Review.Operation = CurrentResolve.Operation;
		return false;
	}
	OutPreview.ProspectiveResolveResult = CurrentResolve.ResolveResult;

	if (Request.Decision != ECFVehicleDriftDecision::KeepAuthoring)
	{
		// Persistent Recipe를 수정하지 않는 transient prospective Recipe입니다.
				UCFVehicleRecipeData* TransientRecipe = DuplicateObject<UCFVehicleRecipeData>(Request.ReadRequest.Recipe, GetTransientPackage());
		if (!TransientRecipe)
		{
			CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Review.Operation, ECFAuthoringErrorCode::InternalError, TEXT("Drift prospective Recipe 복제에 실패했습니다."));
			return false;
		}
		// 이 UObject는 새 asset 복제가 아니라 같은 Recipe의 mutation0 simulation copy이므로 PostDuplicate가 새로 발급한 RecipeId를 원본 identity로 복원합니다.
		TransientRecipe->RecipeId = Request.ReadRequest.Recipe->RecipeId;
		// Existing ImportService ownership primitive에 전달할 raw pins입니다.
		TArray<FCFVehicleFieldOverride> RawPins;
		// Prospective mutation diagnostic입니다.
		FString MutationError;
		for (const FString& CanonicalPath : SelectedPaths)
		{
			// Fresh exact 3-way row입니다.
			const FCFVehicleDriftReviewRow* Row = CFVehicleUXOpsPrivate::FindReviewRow(OutPreview.Review, CanonicalPath);
			if (Request.Decision == ECFVehicleDriftDecision::PreserveRawAsLegacyPin)
			{
				// Existing Legacy Pin shape를 그대로 재사용합니다.
				FCFVehicleFieldOverride& RawPin = RawPins.AddDefaulted_GetRef();
				RawPin.FieldPath = Row->FieldPath;
				RawPin.OverrideValue = Row->CurrentRawValue;
				RawPin.Reason = TEXT("External Drift Preserve Raw");
			}
			else if (!CFVehicleUXOpsPrivate::UpsertAdvancedOverride(*TransientRecipe, *Row, Request.OverrideReason, MutationError))
			{
				CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Review.Operation, ECFAuthoringErrorCode::OverrideNotAllowed, MutationError);
				return false;
			}
		}
		if (Request.Decision == ECFVehicleDriftDecision::PreserveRawAsLegacyPin
			&& !FCFVehicleImportService::ApplyRawPins(*TransientRecipe, RawPins, MutationError))
		{
			CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Review.Operation, ECFAuthoringErrorCode::AdoptionBlocked, MutationError);
			return false;
		}
		TransientRecipe->AuthoringRevision += 1;
		// Transient Recipe prospective Resolve request입니다.
		FCFVehicleAuthoringReadRequest ProspectiveRequest = Request.ReadRequest;
		ProspectiveRequest.Recipe = TransientRecipe;
		// Existing shared Resolver result입니다.
		FCFVehicleResolveReadResult ProspectiveRead;
		if (!ResolveVehiclePreview(ProspectiveRequest, ProspectiveRead))
		{
			OutPreview.Review.Operation = ProspectiveRead.Operation;
			return false;
		}
		OutPreview.ProspectiveResolveResult = ProspectiveRead.ResolveResult;
	}

	OutPreview.Proposal.OperationName = TEXT("ResolveExternalDrift");
	OutPreview.Proposal.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
	OutPreview.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	OutPreview.Proposal.ExpectedRecipeFingerprint = OutPreview.Review.ExpectedRecipeFingerprint;
	OutPreview.Proposal.ExpectedTargetDefinitionHash = OutPreview.Review.ExpectedTargetDefinitionHash;
	OutPreview.Proposal.ProspectiveSourceSignature = OutPreview.ProspectiveResolveResult.SourceSignature;
	OutPreview.Proposal.ProspectiveResolvedDefinitionHash = OutPreview.ProspectiveResolveResult.ResolvedDefinitionHash;
	OutPreview.Proposal.ResolverContractRevision = OutPreview.Review.ResolverContractRevision;
	OutPreview.Proposal.bTargetMutation = false;
	OutPreview.Proposal.bSavePerformed = false;
	OutPreview.Proposal.ProposalHash = CFVehicleUXOpsPrivate::BuildDriftProposalHash(Request, OutPreview.Review, OutPreview.ProspectiveResolveResult);
	return true;
}

// Fresh preview/approval을 재검사하고 Preserve/Override만 Recipe transaction으로 commit합니다.
bool FCFVehicleAuthoringService::CommitDriftDecision(
	const FCFVehicleDriftDecisionRequest& Request,
	FCFAuthoringOpResult& OutResult)
{
	CFVehicleUXOpsPrivate::InitializeResult(OutResult, TEXT("CommitDriftDecision"), ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite);
	// Commit 직전 exact fresh proposal입니다.
	FCFVehicleDriftDecisionPreview FreshPreview;
	if (!PreviewDriftDecision(Request, FreshPreview))
	{
		OutResult = FreshPreview.Review.Operation;
		OutResult.OperationName = TEXT("CommitDriftDecision");
		return false;
	}
	if (!Request.bOwnershipWriteApproved)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalRequired, TEXT("External Drift recovery에는 explicit OwnershipWrite 승인이 필요합니다."));
		return false;
	}
	if (Request.ApprovalScopeHash.IsEmpty() || Request.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("External Drift recovery approval이 current 3-way review와 일치하지 않습니다."));
		return false;
	}
	if (Request.Decision == ECFVehicleDriftDecision::KeepAuthoring)
	{
		OutResult.CurrentRecipeFingerprint = FreshPreview.Review.ExpectedRecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = FreshPreview.Review.ExpectedTargetDefinitionHash;
		OutResult.CurrentSourceSignature = FreshPreview.Review.ExpectedSourceSignature;
		OutResult.CurrentResolvedDefinitionHash = FreshPreview.ProspectiveResolveResult.ResolvedDefinitionHash;
		CFVehicleUXOpsPrivate::SetSucceeded(OutResult, TEXT("Keep Authoring review token을 current External Drift evidence에 binding했습니다. Persistent source mutation은 없습니다."));
		return true;
	}
	if (!Request.ReadRequest.Recipe)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::RecipeNotFound, TEXT("Drift recovery commit에는 Recipe가 필요합니다."));
		return false;
	}

	// Persistent mutation 대상 Recipe입니다.
	UCFVehicleRecipeData& Recipe = *Request.ReadRequest.Recipe;
	// Rollback용 original import state입니다.
	const FCFVehicleImportState OriginalImportState = Recipe.ImportState;
	// Rollback용 original advanced override set입니다.
	const TArray<FCFVehicleFieldOverride> OriginalOverrides = Recipe.AdvancedOverrides;
	// Rollback용 original revision입니다.
	const int32 OriginalRevision = Recipe.AuthoringRevision;
	// Rollback용 original dirty flag입니다.
	const bool bWasDirty = Recipe.GetOutermost()->IsDirty();
	// One logical Recipe ownership transaction입니다.
	FScopedTransaction Transaction(NSLOCTEXT("CarFightDataAuthoring", "ResolveExternalDrift", "CarFight External Drift Recovery"));
	Recipe.Modify();

	// Existing ImportService ownership primitive에 전달할 reviewed raw pins입니다.
	TArray<FCFVehicleFieldOverride> RawPins;
	// Mutation failure diagnostic입니다.
	FString MutationError;
	for (const FString& CanonicalPath : CFVehicleUXOpsPrivate::BuildCanonicalPathSet(Request.FieldPaths))
	{
		// Fresh exact review row입니다.
		const FCFVehicleDriftReviewRow* Row = CFVehicleUXOpsPrivate::FindReviewRow(FreshPreview.Review, CanonicalPath);
		if (!Row)
		{
			MutationError = FString::Printf(TEXT("Commit 직전 Drift row가 사라졌습니다: %s"), *CanonicalPath);
			break;
		}
		if (Request.Decision == ECFVehicleDriftDecision::PreserveRawAsLegacyPin)
		{
			// Existing Legacy Pin shape입니다.
			FCFVehicleFieldOverride& RawPin = RawPins.AddDefaulted_GetRef();
			RawPin.FieldPath = Row->FieldPath;
			RawPin.OverrideValue = Row->CurrentRawValue;
			RawPin.Reason = TEXT("External Drift Preserve Raw");
		}
		else if (!CFVehicleUXOpsPrivate::UpsertAdvancedOverride(Recipe, *Row, Request.OverrideReason, MutationError))
		{
			break;
		}
	}
	if (MutationError.IsEmpty() && Request.Decision == ECFVehicleDriftDecision::PreserveRawAsLegacyPin
		&& !FCFVehicleImportService::ApplyRawPins(Recipe, RawPins, MutationError))
	{
		// Existing ImportService diagnostic를 그대로 사용합니다.
	}
	if (!MutationError.IsEmpty())
	{
		Recipe.ImportState = OriginalImportState;
		Recipe.AdvancedOverrides = OriginalOverrides;
		Recipe.AuthoringRevision = OriginalRevision;
		Recipe.GetOutermost()->SetDirtyFlag(bWasDirty);
		Transaction.Cancel();
		OutResult.Status = ECFAuthoringOpStatus::FailedRolledBack;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Message = MutationError;
		return false;
	}

	Recipe.AuthoringRevision += 1;
	Recipe.MarkPackageDirty();
	Recipe.PostEditChange();
	OutResult.Mutation.bRecipeChanged = true;
	OutResult.Mutation.bPackageDirty = true;
	OutResult.Mutation.bTargetChanged = false;
	OutResult.Mutation.bSavePerformed = false;
	OutResult.Mutation.bAutomaticRetryPerformed = false;
	CFVehicleUXOpsPrivate::SetSucceeded(OutResult, TEXT("External Drift recovery ownership decision을 Recipe transaction으로 commit했습니다. Fresh Resolve/Apply가 필요합니다."));
	return true;
}

// New Vehicle/Mesh-only request의 two-record path/collision/type를 검증하고 exact R2 proposal을 만듭니다.
bool FCFVehicleAuthoringService::PreviewVehicleRecords(
	const FCFVehicleRecordCreateRequest& Request,
	FCFVehicleRecordCreatePreview& OutPreview)
{
	OutPreview = FCFVehicleRecordCreatePreview();
	CFVehicleUXOpsPrivate::InitializeResult(OutPreview.Operation, TEXT("PreviewVehicleRecords"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (Request.CallerKind != ECFAuthoringCallerKind::SlateUI && Request.CallerKind != ECFAuthoringCallerKind::Automation)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::UnsupportedOperation, TEXT("New Vehicle/Mesh-only record creation은 SlateUI/Automation route만 허용합니다."));
		return false;
	}
	// Definition identity validation diagnostic입니다.
	FString Error;
	if (!CFVehicleUXOpsPrivate::ValidateNewAssetIdentity(Request.DefinitionPackageName, Request.DefinitionAssetName, OutPreview.ProspectiveDefinitionPath, Error)
		|| !CFVehicleUXOpsPrivate::ValidateNewAssetIdentity(Request.RecipePackageName, Request.RecipeAssetName, OutPreview.ProspectiveRecipePath, Error))
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
		return false;
	}
	if (OutPreview.ProspectiveDefinitionPath == OutPreview.ProspectiveRecipePath || Request.DefinitionPackageName == Request.RecipePackageName)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("Definition과 Recipe는 서로 다른 package/object identity가 필요합니다."));
		return false;
	}
	if (!Request.ChassisMesh.IsNull())
	{
		// Explicit Mesh candidate의 actual type입니다.
		UStaticMesh* ChassisMesh = Request.ChassisMesh.LoadSynchronous();
		if (!ChassisMesh)
		{
			CFVehicleUXOpsPrivate::SetBlocked(OutPreview.Operation, ECFAuthoringErrorCode::WrongAssetType, TEXT("Create Vehicle From Mesh의 Chassis asset을 StaticMesh로 load할 수 없습니다."));
			return false;
		}
	}

	OutPreview.Proposal.OperationName = TEXT("CreateVehicleRecords");
	OutPreview.Proposal.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
	OutPreview.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	OutPreview.Proposal.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	OutPreview.Proposal.bTargetMutation = true;
	OutPreview.Proposal.bSavePerformed = false;
	OutPreview.Proposal.ProposalHash = CFVehicleUXOpsPrivate::BuildRecordCreateHash(Request, OutPreview.ProspectiveDefinitionPath, OutPreview.ProspectiveRecipePath);
	CFVehicleUXOpsPrivate::SetSucceeded(OutPreview.Operation, TEXT("Definition+Recipe two-record creation을 mutation 없이 preview했습니다."));
	return true;
}

// Fresh R2 proposal을 재검사한 뒤 Definition+Recipe만 한 transaction으로 생성합니다.
bool FCFVehicleAuthoringService::CreateVehicleRecords(
	const FCFVehicleRecordCreateRequest& Request,
	FCFVehicleRecordCreateResult& OutResult)
{
	OutResult = FCFVehicleRecordCreateResult();
	CFVehicleUXOpsPrivate::InitializeResult(OutResult.Operation, TEXT("CreateVehicleRecords"), ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite);
	// Commit 직전 fresh collision/type proposal입니다.
	FCFVehicleRecordCreatePreview FreshPreview;
	if (!PreviewVehicleRecords(Request, FreshPreview))
	{
		OutResult.Operation = FreshPreview.Operation;
		OutResult.Operation.OperationName = TEXT("CreateVehicleRecords");
		return false;
	}
	if (!Request.bOwnershipWriteApproved)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::ApprovalRequired, TEXT("Definition+Recipe 생성에는 explicit OwnershipWrite 승인이 필요합니다."));
		return false;
	}
	if (Request.ApprovalScopeHash.IsEmpty() || Request.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		CFVehicleUXOpsPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("Vehicle record creation approval이 current path/profile/mesh input과 일치하지 않습니다."));
		return false;
	}

	// Two-record creation을 하나의 Undo 단위로 묶는 transaction입니다.
	FScopedTransaction Transaction(NSLOCTEXT("CarFightDataAuthoring", "CreateVehicleRecords", "CarFight Create Vehicle Records"));
	// New Definition package입니다.
	UPackage* DefinitionPackage = CreatePackage(*Request.DefinitionPackageName);
	// New Recipe package입니다.
	UPackage* RecipePackage = CreatePackage(*Request.RecipePackageName);
	if (!DefinitionPackage || !RecipePackage)
	{
		Transaction.Cancel();
		CFVehicleUXOpsPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::InternalError, TEXT("Definition/Recipe package 생성에 실패했습니다."));
		return false;
	}
	// C++ defaults만 가진 canonical Runtime Definition record입니다.
	UCFVehicleData* Definition = NewObject<UCFVehicleData>(DefinitionPackage, Request.DefinitionAssetName, RF_Public | RF_Standalone | RF_Transactional);
	// Editor-only semantic Recipe record입니다.
	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(RecipePackage, Request.RecipeAssetName, RF_Public | RF_Standalone | RF_Transactional);
	if (!Definition || !Recipe)
	{
		Transaction.Cancel();
		CFVehicleUXOpsPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::InternalError, TEXT("Definition/Recipe UObject 생성에 실패했습니다."));
		return false;
	}
	Definition->Modify();
	Recipe->Modify();
		Recipe->TargetVehicleData = Definition;
	Recipe->AssetIntent.ChassisMesh = Request.ChassisMesh;
	Recipe->ProfileBindings = Request.ProfileBindings;
	Recipe->ImportState.ManageState = ECFVehicleManageState::Managed;
	FAssetRegistryModule::AssetCreated(Definition);
	FAssetRegistryModule::AssetCreated(Recipe);
	Definition->MarkPackageDirty();
	Recipe->MarkPackageDirty();
	Definition->PostEditChange();
	Recipe->PostEditChange();

	OutResult.CreatedDefinition = Definition;
	OutResult.CreatedRecipe = Recipe;
	OutResult.Operation.Mutation.bRecipeChanged = true;
	OutResult.Operation.Mutation.bTargetChanged = true;
	OutResult.Operation.Mutation.bPackageDirty = true;
		OutResult.Operation.Mutation.bCreatedAssets = true;
	OutResult.Operation.Mutation.bSavePerformed = false;
	OutResult.Operation.Mutation.bAutomaticRetryPerformed = false;
	CFVehicleUXOpsPrivate::SetSucceeded(OutResult.Operation, TEXT("Definition+Recipe record를 생성했습니다. Apply/Save/Profile/class/physics inference는 수행하지 않았습니다."));
	return true;
}
