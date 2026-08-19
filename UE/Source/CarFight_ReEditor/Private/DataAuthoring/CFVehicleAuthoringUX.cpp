// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringUX.cpp
// Version: v1.1.0
// Date: 2026-08-18
// Description: DAUTH-P0-10~11 Workspace UX가 공유하는 Common Authoring facade 확장입니다.
// Scope: Reference/Adoption/Measurement + Shared Profile B2 adapter + External Drift recovery + two-record creation을 제공합니다.
// Changelog:
// - v1.1.0: Frozen 24.91~24.94 Shared Profile impact, 3-way Drift recovery, New Vehicle/Mesh-only record creation 추가.
// - v1.0.0: Frozen Section 24/25 P0-10 Reference Compare / Adoption / Measurement / legacy managed-target guard facade 최초 구현.
// Migration:
// - UI/Legacy Wizard는 SnapshotBuilder/Resolver/ImportService를 직접 호출하지 않고 이 facade만 사용합니다.
// - Target UCFVehicleData write, Raw SetField, auto save, automatic retry를 추가하지 않습니다.
// - Reference Compare는 read-only이며 Adoption/Measurement commit은 Recipe-only transaction입니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/SecureHash.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"
#include "UObject/UObjectIterator.h"

namespace CFVehicleAuthoringUXPrivate
{
	/** Reference Compare side 하나의 exact field/value/source projection입니다. */
	struct FCompareValue
	{
		// Exact stable field identity입니다.
		FCFVehicleFieldPath FieldPath;

		// Canonical typed field value입니다.
		FCFVehicleFieldValue Value;

		// Resolved side이면 effective source를 사용하고 raw side는 SourceId의 Current Definition 표기로 구분합니다.
		ECFVehicleSourceType SourceType = ECFVehicleSourceType::ProjectCompatibilityDefault;

		// Resolved side이면 effective source id입니다.
		FString SourceId;
	};

	// Common facade result envelope를 deterministic default로 초기화합니다.
	void InitializeResult(FCFAuthoringOpResult& OutResult, const FName OperationName, const ECFAuthoringRiskClass RiskClass)
	{
		OutResult = FCFAuthoringOpResult();
		OutResult.OperationName = OperationName;
		OutResult.RiskClass = RiskClass;
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
	}

	// Common facade result를 typed blocked 상태로 설정합니다.
	void SetBlocked(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
	}

	// Common facade result를 success 상태로 설정합니다.
	void SetSucceeded(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Succeeded;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
	}

	// Delimiter 충돌 없는 canonical approval payload token을 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// UTF-8 payload를 P0 Authoring approval hash와 같은 lowercase MD5 digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR와 분리된 canonical UTF-8 bytes입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Deterministic MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// MD5 128-bit digest bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);
		// Lowercase hexadecimal 결과입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// Fixed lowercase nibble table입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// 두 canonical typed field value가 exact same인지 비교합니다.
	bool AreValuesEqual(const FCFVehicleFieldValue& Left, const FCFVehicleFieldValue& Right)
	{
		return Left.PropertyTypeSignature == Right.PropertyTypeSignature
			&& Left.CanonicalValueText == Right.CanonicalValueText;
	}

	// Resolved result의 effective source를 field index 기반으로 compare value에 복사합니다.
	void PopulateResolvedSource(
		const FCFVehicleResolveResult& ResolveResult,
		const FCFVehicleResolvedField& ResolvedField,
		FCompareValue& OutValue)
	{
		if (!ResolveResult.PreviewSourceTrace.IsValidIndex(ResolvedField.SourceTraceIndex))
		{
			return;
		}
		// Field가 가리키는 exact source trace입니다.
		const FCFVehicleSourceTrace& Trace = ResolveResult.PreviewSourceTrace[ResolvedField.SourceTraceIndex];
		if (!Trace.Layers.IsValidIndex(Trace.EffectiveLayerIndex))
		{
			return;
		}
		// Resolver가 결정한 effective source layer입니다.
		const FCFVehicleSourceLayer& Layer = Trace.Layers[Trace.EffectiveLayerIndex];
		OutValue.SourceType = Layer.SourceType;
		OutValue.SourceId = Layer.SourceId;
	}

	// Current raw Definition 또는 managed Resolved Preview를 compare side map으로 projection합니다.
	bool BuildCompareSide(
		UCFVehicleRecipeData* Recipe,
		UCFVehicleData* Target,
		const ECFVehicleCompareValueMode Mode,
		TMap<FString, FCompareValue>& OutValues,
		FCFAuthoringOpResult& OutOperation)
	{
		OutValues.Reset();
		if (!Target)
		{
			SetBlocked(OutOperation, ECFAuthoringErrorCode::TargetNotFound, TEXT("Reference Compare side Target VehicleData가 없습니다."));
			return false;
		}

		if (Mode == ECFVehicleCompareValueMode::CurrentDefinition)
		{
			// Existing 117 Registry-expanded current Definition snapshot입니다.
			FCFVehicleDefinitionSnapshot DefinitionSnapshot;
			// Snapshot build diagnostic입니다.
			FString SnapshotError;
			if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, DefinitionSnapshot, SnapshotError))
			{
				SetBlocked(OutOperation, ECFAuthoringErrorCode::InternalError, SnapshotError);
				return false;
			}
			for (const FCFVehicleFieldEntry& FieldEntry : DefinitionSnapshot.SortedFields)
			{
				// Canonical selector 포함 field key입니다.
				const FString FieldKey = FieldEntry.FieldPath.ToCanonicalString(true);
				// Raw Definition compare value입니다.
				FCompareValue& CompareValue = OutValues.Add(FieldKey);
				CompareValue.FieldPath = FieldEntry.FieldPath;
				CompareValue.Value = FieldEntry.Value;
								CompareValue.SourceType = ECFVehicleSourceType::ProjectCompatibilityDefault;
				CompareValue.SourceId = FString::Printf(TEXT("Current Definition: %s"), *FSoftObjectPath(Target).ToString());
			}
			return true;
		}

		if (!Recipe)
		{
			SetBlocked(OutOperation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("Resolved Preview Reference Compare에는 managed Recipe가 필요합니다."));
			return false;
		}
		// Shared facade Resolve request입니다.
		FCFVehicleAuthoringReadRequest ReadRequest;
		ReadRequest.Recipe = Recipe;
		ReadRequest.TargetVehicleData = Target;
		ReadRequest.CallerKind = ECFAuthoringCallerKind::SlateUI;
		// Existing Pure Resolver의 exact read result입니다.
		FCFVehicleResolveReadResult ResolveRead;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(ReadRequest, ResolveRead))
		{
			OutOperation = ResolveRead.Operation;
			OutOperation.OperationName = TEXT("CompareReferenceVehicles");
			return false;
		}
		for (const FCFVehicleResolvedField& ResolvedField : ResolveRead.ResolveResult.SortedResolvedFields)
		{
			// Canonical selector 포함 field key입니다.
			const FString FieldKey = ResolvedField.FieldPath.ToCanonicalString(true);
			// Resolved compare value입니다.
			FCompareValue& CompareValue = OutValues.Add(FieldKey);
			CompareValue.FieldPath = ResolvedField.FieldPath;
			CompareValue.Value = ResolvedField.Value;
			PopulateResolvedSource(ResolveRead.ResolveResult, ResolvedField, CompareValue);
		}
		return true;
	}

	// Adoption R2 proposal hash를 exact scope/current/prospective state에 binding합니다.
	FString BuildAdoptionProposalHash(
		const FCFVehicleAdoptionRequest& Request,
		const FCFVehicleAdoptionPreview& Preview,
		const FString& TargetDefinitionHash)
	{
		// Canonical R2 approval payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), Preview.Scope == ECFVehicleAdoptionScope::Group ? TEXT("AdoptGroup") : TEXT("AdoptField"));
		AppendToken(Payload, TEXT("RecipePath"), Request.Recipe ? FSoftObjectPath(Request.Recipe).ToString() : FString());
		AppendToken(Payload, TEXT("TargetPath"), Request.TargetVehicleData ? FSoftObjectPath(Request.TargetVehicleData).ToString() : FString());
		AppendToken(Payload, TEXT("BaselineRecipeFingerprint"), Preview.BaselineRecipeFingerprint);
		AppendToken(Payload, TEXT("TargetDefinitionHash"), TargetDefinitionHash);
		AppendToken(Payload, TEXT("Scope"), FString::FromInt(static_cast<int32>(Preview.Scope)));
		AppendToken(Payload, TEXT("Group"), FString::FromInt(static_cast<int32>(Preview.AdoptionGroup)));
		AppendToken(Payload, TEXT("Field"), Preview.AdoptionFieldPath.ToCanonicalString(true));
		AppendToken(Payload, TEXT("ProspectiveRecipeFingerprint"), Preview.ProspectiveRecipeFingerprint);
		AppendToken(Payload, TEXT("ProspectiveSourceSignature"), Preview.ProspectiveResolveResult.SourceSignature);
		AppendToken(Payload, TEXT("ProspectiveResolvedHash"), Preview.ProspectiveResolveResult.ResolvedDefinitionHash);
		AppendToken(Payload, TEXT("ResolverRevision"), FString::FromInt(Preview.ProspectiveResolveResult.ResolverContractRevision));
		for (const FCFVehicleFieldPath& RemovedPath : Preview.LegacyPinPathsToRemove)
		{
			AppendToken(Payload, TEXT("RemovePin"), RemovedPath.ToCanonicalString(true));
		}
		return HashUtf8Payload(Payload);
	}

	// Exact measurement field를 Recipe AssetAdoption state에 reviewed decision으로 적용합니다.
	bool ApplyMeasurementDecision(
		FCFVehicleAssetAdoption& InOutAdoption,
		const FString& CanonicalPath,
		const ECFVehicleMeasureDecision Decision,
		const FString& AssetFingerprint,
		FString& OutError)
	{
		// Accept measured 여부입니다.
		const bool bAcceptMeasured = Decision == ECFVehicleMeasureDecision::AcceptMeasuredValue;
		// Compatibility Default explicit reviewed decision 여부입니다.
		const bool bConfirmDefault = Decision == ECFVehicleMeasureDecision::UseCompatibilityDefault;
		if (CanonicalPath == TEXT("VehicleMovementConfig.FrontWheelRadius"))
		{
			InOutAdoption.bUseMeasuredFrontRadius = bAcceptMeasured;
			InOutAdoption.bConfirmedFrontRadiusCompatibilityDefault = bConfirmDefault;
			InOutAdoption.FrontRadiusAssetFingerprint = bAcceptMeasured ? AssetFingerprint : FString();
		}
		else if (CanonicalPath == TEXT("VehicleMovementConfig.RearWheelRadius"))
		{
			InOutAdoption.bUseMeasuredRearRadius = bAcceptMeasured;
			InOutAdoption.bConfirmedRearRadiusCompatibilityDefault = bConfirmDefault;
			InOutAdoption.RearRadiusAssetFingerprint = bAcceptMeasured ? AssetFingerprint : FString();
		}
		else if (CanonicalPath == TEXT("VehicleMovementConfig.FrontWheelWidth"))
		{
			InOutAdoption.bUseMeasuredFrontWidth = bAcceptMeasured;
			InOutAdoption.bConfirmedFrontWidthCompatibilityDefault = bConfirmDefault;
			InOutAdoption.FrontWidthAssetFingerprint = bAcceptMeasured ? AssetFingerprint : FString();
		}
		else if (CanonicalPath == TEXT("VehicleMovementConfig.RearWheelWidth"))
		{
			InOutAdoption.bUseMeasuredRearWidth = bAcceptMeasured;
			InOutAdoption.bConfirmedRearWidthCompatibilityDefault = bConfirmDefault;
			InOutAdoption.RearWidthAssetFingerprint = bAcceptMeasured ? AssetFingerprint : FString();
		}
		else
		{
			OutError = FString::Printf(TEXT("P0-10 Measurement Adoption이 허용하지 않는 field입니다: %s"), *CanonicalPath);
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Current Resolver proposal에서 request가 review한 exact candidate/rule/fingerprint를 찾아 검증합니다.
	const FCFVehicleMeasurementProposal* FindReviewedMeasurement(
		const FCFVehicleMeasurementRequest& Request,
		const TArray<FCFVehicleMeasurementProposal>& Proposals,
		ECFAuthoringErrorCode& OutErrorCode,
		FString& OutError)
	{
		// Requested exact canonical field path입니다.
		const FString RequestedPath = Request.FieldPath.ToCanonicalString(true);
		// Current exact field proposal입니다.
		const FCFVehicleMeasurementProposal* Proposal = Proposals.FindByPredicate([&RequestedPath](const FCFVehicleMeasurementProposal& Candidate)
		{
			return Candidate.FieldPath.ToCanonicalString(true) == RequestedPath;
		});
		if (!Proposal)
		{
			OutErrorCode = ECFAuthoringErrorCode::MeasurementNotFound;
			OutError = FString::Printf(TEXT("현재 Resolver에 measurement proposal이 없습니다: %s"), *RequestedPath);
			return nullptr;
		}
		if (Proposal->AssetFingerprint != Request.AssetFingerprint)
		{
			OutErrorCode = ECFAuthoringErrorCode::MeasurementFingerprintStale;
			OutError = TEXT("Measurement review 이후 Wheel Asset fingerprint가 변경됐습니다. 새 proposal을 검토하세요.");
			return nullptr;
		}
		if (Proposal->MeasurementRuleId != Request.MeasurementRuleId
			|| !AreValuesEqual(Proposal->MeasuredCandidateValue, Request.MeasuredCandidateValue))
		{
			OutErrorCode = ECFAuthoringErrorCode::StateChanged;
			OutError = TEXT("Measurement review 이후 candidate value 또는 rule이 변경됐습니다.");
			return nullptr;
		}
		OutErrorCode = ECFAuthoringErrorCode::None;
		OutError.Reset();
		return Proposal;
	}

	// Measurement R2 proposal hash를 exact reviewed candidate/current/prospective state에 binding합니다.
	FString BuildMeasurementProposalHash(
		const FCFVehicleMeasurementRequest& Request,
		const FString& BaselineRecipeFingerprint,
		const FString& TargetDefinitionHash,
		const FString& ProspectiveRecipeFingerprint,
		const FCFVehicleResolveResult& ProspectiveResolve)
	{
		// Canonical R2 approval payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), TEXT("CommitMeasurementDecision"));
		AppendToken(Payload, TEXT("RecipePath"), Request.Recipe ? FSoftObjectPath(Request.Recipe).ToString() : FString());
		AppendToken(Payload, TEXT("TargetPath"), Request.TargetVehicleData ? FSoftObjectPath(Request.TargetVehicleData).ToString() : FString());
		AppendToken(Payload, TEXT("Field"), Request.FieldPath.ToCanonicalString(true));
		AppendToken(Payload, TEXT("Rule"), Request.MeasurementRuleId.ToString());
		AppendToken(Payload, TEXT("AssetFingerprint"), Request.AssetFingerprint);
		AppendToken(Payload, TEXT("CandidateType"), Request.MeasuredCandidateValue.PropertyTypeSignature);
		AppendToken(Payload, TEXT("CandidateValue"), Request.MeasuredCandidateValue.CanonicalValueText);
		AppendToken(Payload, TEXT("Decision"), FString::FromInt(static_cast<int32>(Request.Decision)));
		AppendToken(Payload, TEXT("BaselineRecipeFingerprint"), BaselineRecipeFingerprint);
		AppendToken(Payload, TEXT("TargetDefinitionHash"), TargetDefinitionHash);
		AppendToken(Payload, TEXT("ProspectiveRecipeFingerprint"), ProspectiveRecipeFingerprint);
		AppendToken(Payload, TEXT("ProspectiveSourceSignature"), ProspectiveResolve.SourceSignature);
		AppendToken(Payload, TEXT("ProspectiveResolvedHash"), ProspectiveResolve.ResolvedDefinitionHash);
		AppendToken(Payload, TEXT("ResolverRevision"), FString::FromInt(ProspectiveResolve.ResolverContractRevision));
		return HashUtf8Payload(Payload);
	}
}

// Target을 binding한 managed/imported Recipe 존재 여부를 Editor-only Registry에서 read-only로 조회합니다.
bool FCFVehicleAuthoringService::ReadManagedTarget(UCFVehicleData* TargetVehicleData, FCFVehicleManagedReadResult& OutResult)
{
	OutResult = FCFVehicleManagedReadResult();
	CFVehicleAuthoringUXPrivate::InitializeResult(OutResult.Operation, TEXT("ReadManagedTarget"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (!TargetVehicleData)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::TargetNotFound, TEXT("managed-target 조회 대상 VehicleData가 없습니다."));
		return false;
	}
	// Target exact object identity입니다.
	const FSoftObjectPath TargetPath(TargetVehicleData);
	for (TObjectIterator<UCFVehicleRecipeData> Iterator; Iterator; ++Iterator)
	{
		// 현재 loaded Recipe candidate입니다.
		UCFVehicleRecipeData* Recipe = *Iterator;
		if (IsValid(Recipe) && Recipe->TargetVehicleData.ToSoftObjectPath() == TargetPath)
		{
			OutResult.bManaged = true;
			OutResult.RecipePath = FSoftObjectPath(Recipe);
			OutResult.ManageState = Recipe->ImportState.ManageState;
			CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult.Operation, TEXT("Target을 binding한 loaded managed Recipe를 찾았습니다."));
			return true;
		}
	}

	// Project Asset Registry의 Recipe metadata입니다.
	FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	// Registered Recipe assets입니다.
	TArray<FAssetData> RecipeAssets;
	RegistryModule.Get().GetAssetsByClass(UCFVehicleRecipeData::StaticClass()->GetClassPathName(), RecipeAssets, true);
	for (const FAssetData& RecipeAsset : RecipeAssets)
	{
		// Guard 판정에 필요한 existing Editor-only Recipe입니다.
		UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(RecipeAsset.GetAsset());
		if (Recipe && Recipe->TargetVehicleData.ToSoftObjectPath() == TargetPath)
		{
			OutResult.bManaged = true;
			OutResult.RecipePath = RecipeAsset.GetSoftObjectPath();
			OutResult.ManageState = Recipe->ImportState.ManageState;
			CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult.Operation, TEXT("Target을 binding한 registered managed Recipe를 찾았습니다."));
			return true;
		}
	}
	OutResult.bManaged = false;
	OutResult.ManageState = ECFVehicleManageState::Unmanaged;
	CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult.Operation, TEXT("Target을 binding한 Recipe가 없어 unmanaged로 판정했습니다."));
	return true;
}

// 117 Stable Field Registry projection으로 Current/Resolved A-B Reference Compare를 read-only 생성합니다.
bool FCFVehicleAuthoringService::CompareReferenceVehicles(
	const FCFVehicleReferenceCompareRequest& Request,
	FCFVehicleReferenceCompareResult& OutResult)
{
	OutResult = FCFVehicleReferenceCompareResult();
	CFVehicleAuthoringUXPrivate::InitializeResult(OutResult.Operation, TEXT("CompareReferenceVehicles"), ECFAuthoringRiskClass::R0_ReadOnly);
	// A side exact field projection입니다.
	TMap<FString, CFVehicleAuthoringUXPrivate::FCompareValue> CurrentValues;
	// B side exact field projection입니다.
	TMap<FString, CFVehicleAuthoringUXPrivate::FCompareValue> ReferenceValues;
	if (!CFVehicleAuthoringUXPrivate::BuildCompareSide(Request.CurrentRecipe, Request.CurrentTarget, Request.CurrentMode, CurrentValues, OutResult.Operation)
		|| !CFVehicleAuthoringUXPrivate::BuildCompareSide(Request.ReferenceRecipe, Request.ReferenceTarget, Request.ReferenceMode, ReferenceValues, OutResult.Operation))
	{
		return false;
	}

	// Both sides의 stable field identity union입니다.
	TSet<FString> FieldKeySet;
	for (const TPair<FString, CFVehicleAuthoringUXPrivate::FCompareValue>& Pair : CurrentValues)
	{
		FieldKeySet.Add(Pair.Key);
	}
	for (const TPair<FString, CFVehicleAuthoringUXPrivate::FCompareValue>& Pair : ReferenceValues)
	{
		FieldKeySet.Add(Pair.Key);
	}
	// Stable Field Path deterministic order입니다.
	TArray<FString> FieldKeys = FieldKeySet.Array();
	FieldKeys.Sort();
	for (const FString& FieldKey : FieldKeys)
	{
		// A side optional value입니다.
		const CFVehicleAuthoringUXPrivate::FCompareValue* CurrentValue = CurrentValues.Find(FieldKey);
		// B side optional value입니다.
		const CFVehicleAuthoringUXPrivate::FCompareValue* ReferenceValue = ReferenceValues.Find(FieldKey);
		// Both side presence/type/value exact equality입니다.
		const bool bSame = CurrentValue && ReferenceValue && CFVehicleAuthoringUXPrivate::AreValuesEqual(CurrentValue->Value, ReferenceValue->Value);
		if (!Request.bIncludeSameValues && bSame)
		{
			continue;
		}
		// Read-only compare row입니다.
		FCFVehicleReferenceCompareRow& Row = OutResult.Rows.AddDefaulted_GetRef();
		Row.FieldPath = CurrentValue ? CurrentValue->FieldPath : ReferenceValue->FieldPath;
		Row.bHasCurrentValue = CurrentValue != nullptr;
		Row.bHasReferenceValue = ReferenceValue != nullptr;
		Row.bSame = bSame;
		if (CurrentValue)
		{
			Row.CurrentValue = CurrentValue->Value;
			Row.CurrentSourceType = CurrentValue->SourceType;
			Row.CurrentSourceId = CurrentValue->SourceId;
		}
		if (ReferenceValue)
		{
			Row.ReferenceValue = ReferenceValue->Value;
			Row.ReferenceSourceType = ReferenceValue->SourceType;
			Row.ReferenceSourceId = ReferenceValue->SourceId;
		}
		if (!bSame)
		{
			++OutResult.DifferentCount;
		}
	}
	CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult.Operation, TEXT("Reference Vehicle Compare를 existing Snapshot/Resolver projection으로 read-only 생성했습니다."));
	return true;
}

// Existing Import Core를 사용해 group/field Legacy Pin Adoption prospective result와 exact R2 proposal을 만듭니다.
bool FCFVehicleAuthoringService::PreviewAdoption(
	const FCFVehicleAdoptionRequest& Request,
	FCFVehicleAdoptionPreviewResult& OutResult)
{
	OutResult = FCFVehicleAdoptionPreviewResult();
	CFVehicleAuthoringUXPrivate::InitializeResult(OutResult.Operation, TEXT("PreviewAdoption"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (!Request.Recipe || !Request.TargetVehicleData)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::MissingRequiredSource, TEXT("Adoption Preview에는 Recipe와 Target이 필요합니다."));
		return false;
	}
	// Existing facade가 구성한 exact current resolver request입니다.
	FCFVehicleAuthoringReadRequest ReadRequest;
	ReadRequest.Recipe = Request.Recipe;
	ReadRequest.TargetVehicleData = Request.TargetVehicleData;
	ReadRequest.CallerKind = Request.CallContext.CallerKind;
	// Current shared Resolver read입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(ReadRequest, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = TEXT("PreviewAdoption");
		return false;
	}
	// Existing Import Core preview diagnostic입니다.
	FString PreviewError;
	// Existing Import Core success flag입니다.
	bool bPreviewSucceeded = false;
	if (Request.Scope == ECFVehicleAdoptionScope::Group)
	{
		bPreviewSucceeded = FCFVehicleImportService::PreviewGroupAdoption(ResolveRead.ResolveRequest, Request.AdoptionGroup, OutResult.Preview, PreviewError);
	}
	else
	{
		bPreviewSucceeded = FCFVehicleImportService::PreviewFieldAdoption(ResolveRead.ResolveRequest, Request.AdoptionFieldPath, OutResult.Preview, PreviewError);
	}
	if (!bPreviewSucceeded)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::AdoptionBlocked, PreviewError);
		return false;
	}
	if (!OutResult.Preview.bCanCommit)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::AdoptionBlocked, OutResult.Preview.BlockReason);
		return false;
	}
	OutResult.Proposal.OperationName = Request.Scope == ECFVehicleAdoptionScope::Group ? TEXT("AdoptGroup") : TEXT("AdoptField");
	OutResult.Proposal.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
	OutResult.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	OutResult.Proposal.ExpectedRecipeFingerprint = OutResult.Preview.BaselineRecipeFingerprint;
	OutResult.Proposal.ExpectedTargetDefinitionHash = ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash;
	OutResult.Proposal.ProspectiveRecipeFingerprint = OutResult.Preview.ProspectiveRecipeFingerprint;
	OutResult.Proposal.ProspectiveSourceSignature = OutResult.Preview.ProspectiveResolveResult.SourceSignature;
	OutResult.Proposal.ProspectiveResolvedDefinitionHash = OutResult.Preview.ProspectiveResolveResult.ResolvedDefinitionHash;
	OutResult.Proposal.DiffHash = BuildDiffHash(OutResult.Preview.ProspectiveResolveResult.FieldDiff);
	OutResult.Proposal.ResolverContractRevision = OutResult.Preview.ProspectiveResolveResult.ResolverContractRevision;
	OutResult.Proposal.bTargetMutation = false;
	OutResult.Proposal.bSavePerformed = false;
	OutResult.Proposal.ProposalHash = CFVehicleAuthoringUXPrivate::BuildAdoptionProposalHash(Request, OutResult.Preview, OutResult.Proposal.ExpectedTargetDefinitionHash);
	OutResult.Operation.CurrentRecipeFingerprint = OutResult.Preview.BaselineRecipeFingerprint;
	OutResult.Operation.CurrentTargetDefinitionHash = OutResult.Proposal.ExpectedTargetDefinitionHash;
	OutResult.Operation.CurrentSourceSignature = ResolveRead.ResolveResult.SourceSignature;
	OutResult.Operation.CurrentResolvedDefinitionHash = ResolveRead.ResolveResult.ResolvedDefinitionHash;
	OutResult.Operation.ResolverContractRevision = ResolveRead.ResolveResult.ResolverContractRevision;
	CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult.Operation, TEXT("Legacy Pin Adoption prospective result를 Target mutation 없이 생성했습니다."));
	return true;
}

// Fresh adoption preview/approval을 재검사한 뒤 Recipe-only Adoption transaction을 existing Import Core에 위임합니다.
bool FCFVehicleAuthoringService::CommitAdoption(
	const FCFVehicleAdoptionRequest& Request,
	FCFAuthoringOpResult& OutResult)
{
	CFVehicleAuthoringUXPrivate::InitializeResult(OutResult, TEXT("CommitAdoption"), ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite);
	OutResult.ClientOperationId = Request.CallContext.ClientOperationId;
	if (Request.CallContext.ClientOperationId.IsEmpty() || Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::OwnershipWrite)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalRequired, TEXT("Adoption commit에는 ClientOperationId와 exact OwnershipWrite approval이 필요합니다."));
		return false;
	}
	// Commit 직전 fresh Adoption proposal입니다.
	FCFVehicleAdoptionPreviewResult FreshPreview;
	if (!PreviewAdoption(Request, FreshPreview))
	{
		OutResult = FreshPreview.Operation;
		OutResult.OperationName = TEXT("CommitAdoption");
		OutResult.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
		OutResult.ClientOperationId = Request.CallContext.ClientOperationId;
		return false;
	}
	if (Request.CallContext.ExpectedRecipeFingerprint != FreshPreview.Proposal.ExpectedRecipeFingerprint)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::RecipeFingerprintMismatch, TEXT("Adoption review 이후 Recipe fingerprint가 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ExpectedTargetDefinitionHash != FreshPreview.Proposal.ExpectedTargetDefinitionHash)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Adoption review 이후 Target Definition hash가 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ExpectedResolverContractRevision != FreshPreview.Proposal.ResolverContractRevision)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Adoption review 이후 Resolver revision이 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("Adoption approval scope가 fresh prospective result와 일치하지 않습니다."));
		return false;
	}
	// Existing Import Core Recipe-only commit diagnostic입니다.
	FString CommitError;
	if (!FCFVehicleImportService::CommitAdoption(*Request.Recipe, FreshPreview.Preview, CommitError))
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::StateChanged, CommitError);
		return false;
	}
	// Commit readback Recipe Snapshot입니다.
	FCFVehicleRecipeSnapshot RecipeSnapshot;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Request.Recipe, RecipeSnapshot, CommitError))
	{
		OutResult.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Message = CommitError;
		return false;
	}
	OutResult.Mutation.bRecipeChanged = true;
	OutResult.Mutation.bPackageDirty = Request.Recipe->GetOutermost()->IsDirty();
	OutResult.CurrentRecipeFingerprint = RecipeSnapshot.RecipeFingerprint;
	OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
	OutResult.ResolverContractRevision = FreshPreview.Proposal.ResolverContractRevision;
	OutResult.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult, TEXT("Existing Import Core를 통해 Legacy Pin ownership을 Recipe-only로 채택했습니다. Target/Save/Retry는 수행하지 않았습니다."));
	return true;
}

// Shared Resolver가 만든 current Wheel measurement proposals를 read-only 반환합니다.
bool FCFVehicleAuthoringService::ReadMeasurementProposals(
	const FCFVehicleMeasurementReadRequest& Request,
	FCFVehicleMeasurementReadResult& OutResult)
{
	OutResult = FCFVehicleMeasurementReadResult();
	CFVehicleAuthoringUXPrivate::InitializeResult(OutResult.Operation, TEXT("ReadMeasurementProposals"), ECFAuthoringRiskClass::R0_ReadOnly);
	// Existing facade resolve request입니다.
	FCFVehicleAuthoringReadRequest ReadRequest;
	ReadRequest.Recipe = Request.Recipe;
	ReadRequest.TargetVehicleData = Request.TargetVehicleData;
	ReadRequest.CallerKind = Request.CallerKind;
	// Shared Resolver exact result입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(ReadRequest, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = TEXT("ReadMeasurementProposals");
		return false;
	}
	OutResult.Proposals = ResolveRead.ResolveResult.MeasurementProposals;
	OutResult.RecipeFingerprint = ResolveRead.ResolveRequest.Recipe.RecipeFingerprint;
	OutResult.Operation = ResolveRead.Operation;
	OutResult.Operation.OperationName = TEXT("ReadMeasurementProposals");
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Operation.Message = TEXT("Shared Resolver R6 Wheel measurement proposals를 read-only로 읽었습니다.");
	return true;
}

// Measurement decision을 transient Recipe Snapshot에만 적용해 prospective resolve와 exact R2 proposal을 만듭니다.
bool FCFVehicleAuthoringService::PreviewMeasurementDecision(
	const FCFVehicleMeasurementRequest& Request,
	FCFVehicleMeasurementPreviewResult& OutResult)
{
	OutResult = FCFVehicleMeasurementPreviewResult();
	CFVehicleAuthoringUXPrivate::InitializeResult(OutResult.Operation, TEXT("PreviewMeasurementDecision"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (!Request.Recipe || !Request.TargetVehicleData)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::MissingRequiredSource, TEXT("Measurement Preview에는 Recipe와 Target이 필요합니다."));
		return false;
	}
	// Existing facade current resolve request입니다.
	FCFVehicleAuthoringReadRequest ReadRequest;
	ReadRequest.Recipe = Request.Recipe;
	ReadRequest.TargetVehicleData = Request.TargetVehicleData;
	ReadRequest.CallerKind = Request.CallContext.CallerKind;
	// Current shared Resolver exact state입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(ReadRequest, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = TEXT("PreviewMeasurementDecision");
		return false;
	}
	// Reviewed proposal validation taxonomy입니다.
	ECFAuthoringErrorCode ProposalErrorCode = ECFAuthoringErrorCode::None;
	// Reviewed proposal validation diagnostic입니다.
	FString PreviewError;
	if (!CFVehicleAuthoringUXPrivate::FindReviewedMeasurement(Request, ResolveRead.ResolveResult.MeasurementProposals, ProposalErrorCode, PreviewError))
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ProposalErrorCode, PreviewError);
		return false;
	}
	// Persistent Recipe를 수정하지 않는 prospective immutable request입니다.
	FCFVehicleResolveRequest ProspectiveRequest = ResolveRead.ResolveRequest;
	if (!CFVehicleAuthoringUXPrivate::ApplyMeasurementDecision(
		ProspectiveRequest.Recipe.AssetAdoption,
		Request.FieldPath.ToCanonicalString(true),
		Request.Decision,
		Request.AssetFingerprint,
		PreviewError))
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::FieldNotAuthorable, PreviewError);
		return false;
	}
	if (!FCFVehicleSnapshotBuilder::BuildRecipeFingerprintFromSnapshot(ProspectiveRequest.Recipe, ProspectiveRequest.Recipe.RecipeFingerprint, PreviewError))
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::InternalError, PreviewError);
		return false;
	}
	if (!FCFVehicleResolver::Resolve(ProspectiveRequest, OutResult.ProspectiveResolveResult))
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::InternalError, TEXT("Measurement prospective shared Resolver가 internal Error로 실패했습니다."));
		return false;
	}
	OutResult.Proposal.OperationName = TEXT("CommitMeasurementDecision");
	OutResult.Proposal.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
	OutResult.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	OutResult.Proposal.ExpectedRecipeFingerprint = ResolveRead.ResolveRequest.Recipe.RecipeFingerprint;
	OutResult.Proposal.ExpectedTargetDefinitionHash = ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash;
	OutResult.Proposal.ProspectiveRecipeFingerprint = ProspectiveRequest.Recipe.RecipeFingerprint;
	OutResult.Proposal.ProspectiveSourceSignature = OutResult.ProspectiveResolveResult.SourceSignature;
	OutResult.Proposal.ProspectiveResolvedDefinitionHash = OutResult.ProspectiveResolveResult.ResolvedDefinitionHash;
	OutResult.Proposal.DiffHash = BuildDiffHash(OutResult.ProspectiveResolveResult.FieldDiff);
	OutResult.Proposal.ResolverContractRevision = OutResult.ProspectiveResolveResult.ResolverContractRevision;
	OutResult.Proposal.bTargetMutation = false;
	OutResult.Proposal.bSavePerformed = false;
	OutResult.Proposal.ProposalHash = CFVehicleAuthoringUXPrivate::BuildMeasurementProposalHash(
		Request,
		OutResult.Proposal.ExpectedRecipeFingerprint,
		OutResult.Proposal.ExpectedTargetDefinitionHash,
		OutResult.Proposal.ProspectiveRecipeFingerprint,
		OutResult.ProspectiveResolveResult);
	OutResult.Operation.CurrentRecipeFingerprint = OutResult.Proposal.ExpectedRecipeFingerprint;
	OutResult.Operation.CurrentTargetDefinitionHash = OutResult.Proposal.ExpectedTargetDefinitionHash;
	OutResult.Operation.CurrentSourceSignature = ResolveRead.ResolveResult.SourceSignature;
	OutResult.Operation.CurrentResolvedDefinitionHash = ResolveRead.ResolveResult.ResolvedDefinitionHash;
	OutResult.Operation.ResolverContractRevision = ResolveRead.ResolveResult.ResolverContractRevision;
	CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult.Operation, TEXT("Wheel measurement decision prospective Resolve를 Recipe/Target mutation 없이 생성했습니다."));
	return true;
}

// Fresh proposal/asset fingerprint/approval을 재검사한 뒤 Recipe AssetAdoption만 transaction mutation합니다.
bool FCFVehicleAuthoringService::CommitMeasurementDecision(
	const FCFVehicleMeasurementRequest& Request,
	FCFAuthoringOpResult& OutResult)
{
	CFVehicleAuthoringUXPrivate::InitializeResult(OutResult, TEXT("CommitMeasurementDecision"), ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite);
	OutResult.ClientOperationId = Request.CallContext.ClientOperationId;
	if (Request.CallContext.ClientOperationId.IsEmpty() || Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::OwnershipWrite)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalRequired, TEXT("Measurement commit에는 ClientOperationId와 exact OwnershipWrite approval이 필요합니다."));
		return false;
	}
	// Commit 직전 fresh prospective proposal입니다.
	FCFVehicleMeasurementPreviewResult FreshPreview;
	if (!PreviewMeasurementDecision(Request, FreshPreview))
	{
		OutResult = FreshPreview.Operation;
		OutResult.OperationName = TEXT("CommitMeasurementDecision");
		OutResult.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
		OutResult.ClientOperationId = Request.CallContext.ClientOperationId;
		return false;
	}
	if (Request.CallContext.ExpectedRecipeFingerprint != FreshPreview.Proposal.ExpectedRecipeFingerprint)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::RecipeFingerprintMismatch, TEXT("Measurement review 이후 Recipe fingerprint가 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ExpectedTargetDefinitionHash != FreshPreview.Proposal.ExpectedTargetDefinitionHash)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Measurement review 이후 Target Definition hash가 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ExpectedResolverContractRevision != FreshPreview.Proposal.ResolverContractRevision)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Measurement review 이후 Resolver revision이 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("Measurement approval scope가 fresh proposal과 일치하지 않습니다."));
		return false;
	}

	// Reviewed decision을 persistent Recipe에만 반영하기 전 local adoption state입니다.
	FCFVehicleAssetAdoption NewAdoption = Request.Recipe->AssetAdoption;
	// Decision application diagnostic입니다.
	FString CommitError;
	if (!CFVehicleAuthoringUXPrivate::ApplyMeasurementDecision(
		NewAdoption,
		Request.FieldPath.ToCanonicalString(true),
		Request.Decision,
		Request.AssetFingerprint,
		CommitError))
	{
		CFVehicleAuthoringUXPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::FieldNotAuthorable, CommitError);
		return false;
	}
	if (FCFVehicleAssetAdoption::StaticStruct()->CompareScriptStruct(&Request.Recipe->AssetAdoption, &NewAdoption, 0))
	{
		OutResult.Status = ECFAuthoringOpStatus::NoChange;
		OutResult.Message = TEXT("Measurement reviewed decision이 이미 Recipe에 반영되어 mutation을 수행하지 않았습니다.");
		OutResult.CurrentRecipeFingerprint = FreshPreview.Proposal.ExpectedRecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
		OutResult.ResolverContractRevision = FreshPreview.Proposal.ResolverContractRevision;
		return true;
	}
	// Recipe-only measurement decision logical transaction입니다.
	FScopedTransaction MeasurementTransaction(NSLOCTEXT("CarFightDataAuthoring", "CommitWheelMeasurementDecision", "차량 휠 측정값 결정"));
	Request.Recipe->Modify();
	Request.Recipe->AssetAdoption = NewAdoption;
	++Request.Recipe->AuthoringRevision;
	Request.Recipe->MarkPackageDirty();
	Request.Recipe->PostEditChange();
	// Persistent Recipe readback snapshot입니다.
	FCFVehicleRecipeSnapshot RecipeSnapshot;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Request.Recipe, RecipeSnapshot, CommitError))
	{
		OutResult.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Message = CommitError;
		return false;
	}
	OutResult.Mutation.bRecipeChanged = true;
	OutResult.Mutation.bPackageDirty = Request.Recipe->GetOutermost()->IsDirty();
	OutResult.CurrentRecipeFingerprint = RecipeSnapshot.RecipeFingerprint;
	OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
	OutResult.ResolverContractRevision = FreshPreview.Proposal.ResolverContractRevision;
	OutResult.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	CFVehicleAuthoringUXPrivate::SetSucceeded(OutResult, TEXT("Wheel measurement reviewed decision을 Recipe AssetAdoption에 반영했습니다. Target/Save/Retry는 수행하지 않았습니다."));
	return true;
}
