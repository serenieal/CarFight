// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchApply.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08M Frozen Section 26.54~26.65 B3 Batch Definition Apply Foundation 구현입니다.
// Scope: fresh R3 evidence plan, exact approval, all-target global preflight, canonical per-Vehicle ApplyService sequence와 partial result를 제공합니다.
// Changelog:
// - v1.0.0: B3 plan/approval/preflight/Stop-On-First-Failure/per-Vehicle atomic ApplyService orchestration 최초 구현.
// Migration:
// - Batch global transaction/rollback을 만들지 않으며 이미 성공한 Vehicle을 자동 rollback하지 않습니다.
// - FCFVehicleApplyService::Apply 외 Target writer를 만들지 않습니다.
// - Auto retry/save와 Section 26.66 BatchOperationId dedupe는 수행하지 않습니다.

#include "DataAuthoring/CFBatchApply.h"

#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Misc/SecureHash.h"

namespace CFBatchApplyPrivate
{
		// UTF-8 canonical payload를 lowercase MD5 hexadecimal digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR 표현과 분리된 canonical UTF-8 payload입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// UTF-8 bytes의 deterministic digest를 계산할 MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// MD5 128-bit 결과 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Platform-independent lowercase hexadecimal 결과 문자열입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// 한 nibble을 lowercase hex로 바꾸는 고정 문자표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// Delimiter 충돌 없이 canonical payload 조각을 label/문자수/value 형태로 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// B3 eligibility enum을 hash용 stable token으로 변환합니다.
	FString EligibilityToToken(const ECFBatchApplyEligibility Eligibility)
	{
		return FString::FromInt(static_cast<int32>(Eligibility));
	}

	// Resolver FieldDiff에서 Add/Remove/Move structural operation 수를 계산합니다.
	int32 CountArrayStructuralDiffs(const TArray<FCFVehicleFieldDiff>& FieldDiff)
	{
		// Array structural operation 누적값입니다.
		int32 StructuralCount = 0;
		for (const FCFVehicleFieldDiff& Diff : FieldDiff)
		{
			if (Diff.Operation != ECFVehicleDiffOp::SetLeaf)
			{
				++StructuralCount;
			}
		}
		return StructuralCount;
	}

	// Section 26.54 한 item의 immutable R3 evidence hash를 계산합니다.
	FString BuildR3EvidenceHash(const FCFBatchDefinitionApplyItem& Item)
	{
		// Localized message/UObject pointer를 제외한 exact R3 evidence payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Kind"), TEXT("CarFightBatchR3Evidence"));
		AppendToken(Payload, TEXT("FormatRevision"), TEXT("1"));
		AppendToken(Payload, TEXT("TargetPath"), Item.TargetPath);
		AppendToken(Payload, TEXT("RecipePath"), Item.RecipePath);
		AppendToken(Payload, TEXT("Eligibility"), EligibilityToToken(Item.Eligibility));
		AppendToken(Payload, TEXT("Eligible"), Item.bEligible ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("RecipeFingerprint"), Item.ExpectedRecipeFingerprint);
		AppendToken(Payload, TEXT("SourceSignature"), Item.ExpectedSourceSignature);
		AppendToken(Payload, TEXT("TargetDefinitionHash"), Item.ExpectedTargetDefinitionHash);
		AppendToken(Payload, TEXT("ResolvedDefinitionHash"), Item.ExpectedResolvedDefinitionHash);
		AppendToken(Payload, TEXT("DiffHash"), Item.ExpectedDiffHash);
		AppendToken(Payload, TEXT("ResolverRevision"), FString::FromInt(Item.ExpectedResolverContractRevision));
		AppendToken(Payload, TEXT("ValidationInfo"), FString::FromInt(Item.ValidationSummary.InfoCount));
		AppendToken(Payload, TEXT("ValidationWarning"), FString::FromInt(Item.ValidationSummary.WarningCount));
		AppendToken(Payload, TEXT("ValidationBlocked"), FString::FromInt(Item.ValidationSummary.BlockedCount));
		AppendToken(Payload, TEXT("ValidationError"), FString::FromInt(Item.ValidationSummary.ErrorCount));
		AppendToken(Payload, TEXT("ApplyBlocking"), Item.ValidationSummary.bApplyBlocking ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("WarningCount"), FString::FromInt(Item.WarningCount));
		AppendToken(Payload, TEXT("FieldDiffCount"), FString::FromInt(Item.FieldDiffCount));
		AppendToken(Payload, TEXT("ArrayStructuralDiffCount"), FString::FromInt(Item.ArrayStructuralDiffCount));
		AppendToken(Payload, TEXT("ExternalDrift"), Item.bExternalDrift ? TEXT("1") : TEXT("0"));
		return HashUtf8Payload(Payload);
	}

	// Fresh Resolve 결과를 Frozen 26.55 eligibility로 분류합니다.
	ECFBatchApplyEligibility ClassifyEligibility(const FCFVehicleResolveReadResult& ResolveRead)
	{
		if (ResolveRead.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Error)
		{
			return ECFBatchApplyEligibility::ResolveError;
		}
		if (ResolveRead.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Blocked)
		{
			return ECFBatchApplyEligibility::ResolveBlocked;
		}
		if (ResolveRead.Operation.ValidationSummary.bApplyBlocking)
		{
			return ECFBatchApplyEligibility::ValidationBlocked;
		}
		if (ResolveRead.ResolveResult.StaleReport.bHasExternalDrift)
		{
			return ECFBatchApplyEligibility::ExternalDrift;
		}
		if (ResolveRead.ResolveResult.FieldDiff.IsEmpty())
		{
			return ResolveRead.ResolveResult.StaleReport.bHasShadowSourceChange
				? ECFBatchApplyEligibility::ShadowOnly
				: ECFBatchApplyEligibility::NoChange;
		}
		return ECFBatchApplyEligibility::Eligible;
	}

	// Eligibility를 사람이 읽는 deterministic diagnostic으로 변환합니다.
	FString BuildEligibilityMessage(const ECFBatchApplyEligibility Eligibility)
	{
		switch (Eligibility)
		{
		case ECFBatchApplyEligibility::Eligible:
			return TEXT("Fresh Preview가 B3 Definition Apply eligible입니다.");
		case ECFBatchApplyEligibility::NoChange:
			return TEXT("Fresh Preview Diff가 0이므로 B3 Apply 대상이 아닙니다.");
		case ECFBatchApplyEligibility::ShadowOnly:
			return TEXT("Shadow-only source change이며 effective Diff가 0이므로 B3 Apply 대상이 아닙니다.");
		case ECFBatchApplyEligibility::ExternalDrift:
			return TEXT("External Drift가 있어 per-Vehicle Drift Review 전에는 B3 Apply할 수 없습니다.");
		case ECFBatchApplyEligibility::ResolveBlocked:
			return TEXT("Fresh Resolver가 Blocked이므로 B3 Apply 대상이 아닙니다.");
		case ECFBatchApplyEligibility::ResolveError:
			return TEXT("Fresh Resolver가 Error이므로 B3 Apply 대상이 아닙니다.");
		case ECFBatchApplyEligibility::ValidationBlocked:
			return TEXT("Fresh validation에 Apply blocker가 있어 B3 Apply 대상이 아닙니다.");
		case ECFBatchApplyEligibility::InvalidContext:
		default:
			return TEXT("Recipe/Target context가 유효하지 않아 B3 Apply 대상이 아닙니다.");
		}
	}

		// Persistent Recipe/Target truth를 Authoring facade로 fresh read/resolve해 한 B3 item을 구성합니다.
	bool BuildFreshItem(
		UCFVehicleRecipeData& Recipe,
		const ECFAuthoringCallerKind CallerKind,
		FCFBatchDefinitionApplyItem& OutItem,
		FString& OutError)
	{
		OutItem = FCFBatchDefinitionApplyItem();
		OutItem.Recipe = &Recipe;
		OutItem.RecipePath = FSoftObjectPath(&Recipe).ToString();

		// Recipe가 현재 binding한 Target soft path입니다.
		const FSoftObjectPath TargetPath = Recipe.TargetVehicleData.ToSoftObjectPath();
		OutItem.TargetPath = TargetPath.ToString();
		if (!TargetPath.IsValid())
		{
			OutItem.Eligibility = ECFBatchApplyEligibility::InvalidContext;
			OutItem.Message = TEXT("Recipe Target binding이 없습니다.");
			OutItem.R3EvidenceHash = BuildR3EvidenceHash(OutItem);
			OutError.Reset();
			return true;
		}

		// Current Recipe binding의 persistent Target object입니다.
		UCFVehicleData* TargetVehicleData = Recipe.TargetVehicleData.LoadSynchronous();
		if (!TargetVehicleData)
		{
			OutItem.Eligibility = ECFBatchApplyEligibility::InvalidContext;
			OutItem.Message = FString::Printf(TEXT("Recipe Target VehicleData를 load할 수 없습니다: %s"), *OutItem.TargetPath);
			OutItem.R3EvidenceHash = BuildR3EvidenceHash(OutItem);
			OutError.Reset();
			return true;
		}
		OutItem.TargetVehicleData = TargetVehicleData;
		OutItem.TargetPath = FSoftObjectPath(TargetVehicleData).ToString();

		// Existing Authoring facade가 shared Snapshot/AssetReader/Pure Resolver를 호출하는 fresh R0 request입니다.
		FCFVehicleAuthoringReadRequest ReadRequest;
		ReadRequest.Recipe = &Recipe;
		ReadRequest.TargetVehicleData = TargetVehicleData;
		ReadRequest.CallerKind = CallerKind;
		// Exact fresh Resolver request/result입니다.
		FCFVehicleResolveReadResult ResolveRead;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(ReadRequest, ResolveRead))
		{
			OutItem.Eligibility = ResolveRead.Operation.Status == ECFAuthoringOpStatus::FailedUnknownState
				? ECFBatchApplyEligibility::ResolveError
				: ECFBatchApplyEligibility::ResolveBlocked;
			OutItem.Message = ResolveRead.Operation.Message;
			OutItem.R3EvidenceHash = BuildR3EvidenceHash(OutItem);
			OutError.Reset();
			return true;
		}

		OutItem.ExpectedRecipeFingerprint = ResolveRead.ResolveRequest.Recipe.RecipeFingerprint;
		OutItem.ExpectedSourceSignature = ResolveRead.ResolveResult.SourceSignature;
		OutItem.ExpectedTargetDefinitionHash = ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash;
		OutItem.ExpectedResolvedDefinitionHash = ResolveRead.ResolveResult.ResolvedDefinitionHash;
		OutItem.ExpectedDiffHash = FCFVehicleAuthoringService::BuildDiffHash(ResolveRead.ResolveResult.FieldDiff);
		OutItem.ExpectedResolverContractRevision = ResolveRead.ResolveResult.ResolverContractRevision;
		OutItem.ValidationSummary = ResolveRead.Operation.ValidationSummary;
		OutItem.WarningCount = ResolveRead.Operation.ValidationSummary.WarningCount;
		OutItem.FieldDiffCount = ResolveRead.ResolveResult.FieldDiff.Num();
		OutItem.ArrayStructuralDiffCount = CountArrayStructuralDiffs(ResolveRead.ResolveResult.FieldDiff);
		OutItem.bExternalDrift = ResolveRead.ResolveResult.StaleReport.bHasExternalDrift;
		OutItem.Eligibility = ClassifyEligibility(ResolveRead);
		OutItem.bEligible = OutItem.Eligibility == ECFBatchApplyEligibility::Eligible;
		OutItem.Message = BuildEligibilityMessage(OutItem.Eligibility);

		if (OutItem.bEligible)
		{
			OutItem.ApplyRequest.Recipe = &Recipe;
			OutItem.ApplyRequest.TargetVehicleData = TargetVehicleData;
			OutItem.ApplyRequest.ResolveRequest = ResolveRead.ResolveRequest;
			OutItem.ApplyRequest.ApprovedResolveResult = ResolveRead.ResolveResult;
			OutItem.ApplyRequest.ExpectedRecipeFingerprint = OutItem.ExpectedRecipeFingerprint;
			OutItem.ApplyRequest.ExpectedSourceSignature = OutItem.ExpectedSourceSignature;
			OutItem.ApplyRequest.ExpectedTargetDefinitionHash = OutItem.ExpectedTargetDefinitionHash;
			OutItem.ApplyRequest.ExpectedResolvedDefinitionHash = OutItem.ExpectedResolvedDefinitionHash;
			OutItem.ApplyRequest.ExpectedResolverContractRevision = OutItem.ExpectedResolverContractRevision;
		}

		OutItem.R3EvidenceHash = BuildR3EvidenceHash(OutItem);
		OutError.Reset();
		return true;
	}

	// Plan item들에서 expected summary와 ordered eligible set을 재계산해 stored plan consistency를 검사합니다.
	bool ValidatePlanSummary(const FCFBatchDefinitionApplyPlan& Plan, FString& OutError)
	{
		// Recomputed eligible Target path set입니다.
		TArray<FString> EligibleTargets;
		// Recomputed eligible count입니다.
		int32 EligibleCount = 0;
		// Recomputed ineligible count입니다.
		int32 IneligibleCount = 0;
		// Recomputed diff row total입니다.
		int32 FieldDiffCount = 0;
		// Recomputed warning total입니다.
		int32 WarningCount = 0;
		// Recomputed structural diff total입니다.
		int32 StructuralCount = 0;

		// Duplicate target identity를 금지할 set입니다.
		TSet<FString> SeenTargets;
		// Duplicate recipe identity를 금지할 set입니다.
		TSet<FString> SeenRecipes;
		// Canonical order 검사용 직전 Target path입니다.
		FString LastTargetPath;
		for (const FCFBatchDefinitionApplyItem& Item : Plan.Items)
		{
			if (Item.TargetPath.IsEmpty() || Item.RecipePath.IsEmpty())
			{
				OutError = TEXT("B3 plan item TargetPath/RecipePath가 비어 있습니다.");
				return false;
			}
			if (SeenTargets.Contains(Item.TargetPath) || SeenRecipes.Contains(Item.RecipePath))
			{
				OutError = TEXT("B3 plan에는 duplicate Target 또는 Recipe identity가 있을 수 없습니다.");
				return false;
			}
			SeenTargets.Add(Item.TargetPath);
			SeenRecipes.Add(Item.RecipePath);
			if (!LastTargetPath.IsEmpty() && LastTargetPath > Item.TargetPath)
			{
				OutError = TEXT("B3 plan item이 canonical TargetPath ascending 순서가 아닙니다.");
				return false;
			}
			LastTargetPath = Item.TargetPath;

			if (Item.R3EvidenceHash != BuildR3EvidenceHash(Item))
			{
				OutError = FString::Printf(TEXT("B3 per-target R3 evidence hash가 item content와 일치하지 않습니다: %s"), *Item.TargetPath);
				return false;
			}
			if (Item.bEligible != (Item.Eligibility == ECFBatchApplyEligibility::Eligible))
			{
				OutError = FString::Printf(TEXT("B3 eligibility flag가 terminal classification과 일치하지 않습니다: %s"), *Item.TargetPath);
				return false;
			}
			if (Item.bEligible)
			{
				++EligibleCount;
				EligibleTargets.Add(Item.TargetPath);
				FieldDiffCount += Item.FieldDiffCount;
				WarningCount += Item.WarningCount;
				StructuralCount += Item.ArrayStructuralDiffCount;
			}
			else
			{
				++IneligibleCount;
			}
		}

		if (Plan.TotalAffectedVehicleCount != Plan.Items.Num()
			|| Plan.EligibleVehicleCount != EligibleCount
			|| Plan.IneligibleVehicleCount != IneligibleCount
			|| Plan.TotalFieldDiffCount != FieldDiffCount
			|| Plan.TotalWarningCount != WarningCount
			|| Plan.TotalArrayStructuralDiffCount != StructuralCount
			|| Plan.OrderedEligibleTargetPaths != EligibleTargets
			|| Plan.bAutoSave)
		{
			OutError = TEXT("B3 stored plan summary/ordered target set이 item evidence에서 재계산한 값과 다릅니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

		// Plan/approval exact equality를 mutation 전에 검사하고 mismatch taxonomy를 반환합니다.
	ECFBatchApplyErrorCode CompareApproval(
		const FCFBatchDefinitionApplyApproval& Actual,
		const FCFBatchDefinitionApplyApproval& Expected)
	{
		if (Actual.OrderedTargetPaths != Expected.OrderedTargetPaths)
		{
			return ECFBatchApplyErrorCode::OrderedTargetSetMismatch;
		}
		if (Actual.PerTargetEvidence.Num() != Expected.PerTargetEvidence.Num())
		{
			return ECFBatchApplyErrorCode::PerTargetEvidenceMismatch;
		}
		for (int32 ItemIndex = 0; ItemIndex < Actual.PerTargetEvidence.Num(); ++ItemIndex)
		{
			// Caller approval row입니다.
			const FCFBatchDefinitionApplyApprovalItem& ActualItem = Actual.PerTargetEvidence[ItemIndex];
			// Rebuilt expected approval row입니다.
			const FCFBatchDefinitionApplyApprovalItem& ExpectedItem = Expected.PerTargetEvidence[ItemIndex];
			if (ActualItem.TargetPath != ExpectedItem.TargetPath || ActualItem.R3EvidenceHash != ExpectedItem.R3EvidenceHash)
			{
				return ECFBatchApplyErrorCode::PerTargetEvidenceMismatch;
			}
		}
		if (Actual.BatchApplyPlanHash != Expected.BatchApplyPlanHash
			|| Actual.TotalVehicleCount != Expected.TotalVehicleCount
			|| Actual.TotalFieldDiffCount != Expected.TotalFieldDiffCount
			|| Actual.WarningCount != Expected.WarningCount
			|| Actual.ArrayStructuralDiffCount != Expected.ArrayStructuralDiffCount
			|| Actual.bAutoSave != Expected.bAutoSave)
		{
			return ECFBatchApplyErrorCode::ApprovalMismatch;
		}
		return ECFBatchApplyErrorCode::None;
	}

	// Result list를 plan item 순서로 초기화하고 eligible item은 NotStarted로 설정합니다.
	void InitializeVehicleResults(
		const FCFBatchDefinitionApplyPlan& Plan,
		FCFBatchDefinitionApplyResult& OutResult)
	{
		OutResult.VehicleResults.Reset();
		OutResult.VehicleResults.Reserve(Plan.Items.Num());
		for (const FCFBatchDefinitionApplyItem& Item : Plan.Items)
		{
			// Plan item 하나의 initial result row입니다.
			FCFBatchVehicleApplyResult& VehicleResult = OutResult.VehicleResults.AddDefaulted_GetRef();
			VehicleResult.TargetPath = Item.TargetPath;
			VehicleResult.RecipePath = Item.RecipePath;
			VehicleResult.Eligibility = Item.Eligibility;
			VehicleResult.State = Item.bEligible ? ECFBatchVehicleApplyState::NotStarted : ECFBatchVehicleApplyState::Ineligible;
			VehicleResult.Message = Item.Message;
			if (!Item.bEligible)
			{
				++OutResult.IneligibleVehicleCount;
			}
		}
		OutResult.NotStartedVehicleCount = Plan.EligibleVehicleCount;
	}

	// Exact TargetPath의 mutable result row를 찾습니다.
	FCFBatchVehicleApplyResult* FindVehicleResult(
		FCFBatchDefinitionApplyResult& Result,
		const FString& TargetPath)
	{
		return Result.VehicleResults.FindByPredicate([&TargetPath](const FCFBatchVehicleApplyResult& VehicleResult)
		{
			return VehicleResult.TargetPath == TargetPath;
		});
	}



}

// B1/B2 성공 뒤 affected Recipe를 fresh read/resolve해 Section 26.54 R3 evidence collection을 만듭니다.
bool FCFBatchApplyService::BuildDefinitionApplyPlan(
	const FCFBatchDefinitionApplyPlanRequest& Request,
	FCFBatchDefinitionApplyPlan& OutPlan,
	FString& OutError)
{
	OutPlan = FCFBatchDefinitionApplyPlan();
	if (Request.Recipes.IsEmpty())
	{
		OutError = TEXT("B3 Apply Plan에는 최소 하나의 affected Recipe가 필요합니다.");
		return false;
	}

	// Request에 duplicate Recipe pointer가 있는지 확인할 identity set입니다.
	TSet<FString> RequestedRecipePaths;
	for (UCFVehicleRecipeData* Recipe : Request.Recipes)
	{
		if (!Recipe)
		{
			OutError = TEXT("B3 Apply Plan affected Recipe에 null이 포함되어 있습니다.");
			return false;
		}
		// Persistent Recipe canonical identity입니다.
		const FString RecipePath = FSoftObjectPath(Recipe).ToString();
		if (RecipePath.IsEmpty() || RequestedRecipePaths.Contains(RecipePath))
		{
			OutError = TEXT("B3 Apply Plan affected Recipe identity가 비어 있거나 중복되었습니다.");
			return false;
		}
		RequestedRecipePaths.Add(RecipePath);

		// One affected Recipe의 fresh R3 evidence item입니다.
		FCFBatchDefinitionApplyItem Item;
		if (!CFBatchApplyPrivate::BuildFreshItem(*Recipe, Request.CallerKind, Item, OutError))
		{
			return false;
		}
		OutPlan.Items.Add(MoveTemp(Item));
	}

	OutPlan.Items.Sort([](const FCFBatchDefinitionApplyItem& Left, const FCFBatchDefinitionApplyItem& Right)
	{
		if (Left.TargetPath != Right.TargetPath)
		{
			return Left.TargetPath < Right.TargetPath;
		}
		return Left.RecipePath < Right.RecipePath;
	});

	// Canonical sorted plan에서 duplicate Target identity를 금지합니다.
	TSet<FString> SeenTargetPaths;
	for (const FCFBatchDefinitionApplyItem& Item : OutPlan.Items)
	{
		if (Item.TargetPath.IsEmpty() || SeenTargetPaths.Contains(Item.TargetPath))
		{
			OutError = TEXT("B3 Apply Plan에는 empty/duplicate Target identity가 있을 수 없습니다.");
			return false;
		}
		SeenTargetPaths.Add(Item.TargetPath);

		if (Item.bEligible)
		{
			OutPlan.OrderedEligibleTargetPaths.Add(Item.TargetPath);
			++OutPlan.EligibleVehicleCount;
			OutPlan.TotalFieldDiffCount += Item.FieldDiffCount;
			OutPlan.TotalWarningCount += Item.WarningCount;
			OutPlan.TotalArrayStructuralDiffCount += Item.ArrayStructuralDiffCount;
		}
		else
		{
			++OutPlan.IneligibleVehicleCount;
		}
	}
	OutPlan.TotalAffectedVehicleCount = OutPlan.Items.Num();
	OutPlan.bAutoSave = false;
	OutPlan.BatchApplyPlanHash = BuildBatchApplyPlanHash(OutPlan);
	if (OutPlan.BatchApplyPlanHash.IsEmpty() || !CFBatchApplyPrivate::ValidatePlanSummary(OutPlan, OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("B3 BatchApplyPlanHash 생성 또는 plan consistency 검증에 실패했습니다.");
		}
		return false;
	}
	OutError.Reset();
	return true;
}

// Item order와 localized message에 독립적인 exact BatchApplyPlanHash를 계산합니다.
FString FCFBatchApplyService::BuildBatchApplyPlanHash(const FCFBatchDefinitionApplyPlan& Plan)
{
	// Target/Recipe identity 기준으로 재정렬할 item pointers입니다.
	TArray<const FCFBatchDefinitionApplyItem*> SortedItems;
	SortedItems.Reserve(Plan.Items.Num());
	for (const FCFBatchDefinitionApplyItem& Item : Plan.Items)
	{
		SortedItems.Add(&Item);
	}
		SortedItems.Sort([](const FCFBatchDefinitionApplyItem& Left, const FCFBatchDefinitionApplyItem& Right)
	{
		if (Left.TargetPath != Right.TargetPath)
		{
			return Left.TargetPath < Right.TargetPath;
		}
		return Left.RecipePath < Right.RecipePath;
	});

	// Exact plan semantic payload입니다.
	FString Payload;
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("Kind"), TEXT("CarFightBatchDefinitionApplyPlan"));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("FormatRevision"), TEXT("1"));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("ItemCount"), FString::FromInt(SortedItems.Num()));
	for (const FCFBatchDefinitionApplyItem* Item : SortedItems)
	{
		if (!Item)
		{
			continue;
		}
		CFBatchApplyPrivate::AppendToken(Payload, TEXT("TargetPath"), Item->TargetPath);
		CFBatchApplyPrivate::AppendToken(Payload, TEXT("RecipePath"), Item->RecipePath);
		CFBatchApplyPrivate::AppendToken(Payload, TEXT("R3EvidenceHash"), Item->R3EvidenceHash);
	}
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("AffectedVehicleCount"), FString::FromInt(Plan.TotalAffectedVehicleCount));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("EligibleVehicleCount"), FString::FromInt(Plan.EligibleVehicleCount));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("IneligibleVehicleCount"), FString::FromInt(Plan.IneligibleVehicleCount));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("TotalFieldDiffCount"), FString::FromInt(Plan.TotalFieldDiffCount));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("TotalWarningCount"), FString::FromInt(Plan.TotalWarningCount));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("TotalArrayStructuralDiffCount"), FString::FromInt(Plan.TotalArrayStructuralDiffCount));
	CFBatchApplyPrivate::AppendToken(Payload, TEXT("AutoSave"), Plan.bAutoSave ? TEXT("1") : TEXT("0"));
	return CFBatchApplyPrivate::HashUtf8Payload(Payload);
}

// Exact eligible Target set/per-target R3 evidence를 Section 26.57 approval로 binding합니다.
bool FCFBatchApplyService::BuildDefinitionApplyApproval(
	const FCFBatchDefinitionApplyPlan& Plan,
	FCFBatchDefinitionApplyApproval& OutApproval,
	FString& OutError)
{
	OutApproval = FCFBatchDefinitionApplyApproval();
	if (!CFBatchApplyPrivate::ValidatePlanSummary(Plan, OutError))
	{
		return false;
	}
	if (Plan.BatchApplyPlanHash.IsEmpty() || Plan.BatchApplyPlanHash != BuildBatchApplyPlanHash(Plan))
	{
		OutError = TEXT("B3 stored BatchApplyPlanHash가 current plan evidence와 일치하지 않습니다.");
		return false;
	}
	if (Plan.EligibleVehicleCount <= 0)
	{
		OutError = TEXT("B3 Apply eligible Vehicle이 없습니다. NoChange/ShadowOnly/Drift/Blocked item은 approval 대상이 아닙니다.");
		return false;
	}

	OutApproval.BatchApplyPlanHash = Plan.BatchApplyPlanHash;
	OutApproval.OrderedTargetPaths = Plan.OrderedEligibleTargetPaths;
	OutApproval.TotalVehicleCount = Plan.EligibleVehicleCount;
	OutApproval.TotalFieldDiffCount = Plan.TotalFieldDiffCount;
	OutApproval.WarningCount = Plan.TotalWarningCount;
	OutApproval.ArrayStructuralDiffCount = Plan.TotalArrayStructuralDiffCount;
	OutApproval.bAutoSave = false;
	for (const FCFBatchDefinitionApplyItem& Item : Plan.Items)
	{
		if (!Item.bEligible)
		{
			continue;
		}
		// Exact ordered Target/R3 evidence approval row입니다.
		FCFBatchDefinitionApplyApprovalItem& ApprovalItem = OutApproval.PerTargetEvidence.AddDefaulted_GetRef();
		ApprovalItem.TargetPath = Item.TargetPath;
		ApprovalItem.R3EvidenceHash = Item.R3EvidenceHash;
	}
	OutError.Reset();
	return true;
}

// 모든 eligible item global preflight PASS 뒤 canonical order로 FCFVehicleApplyService::Apply를 각 Target에 최대 한 번 호출합니다.
bool FCFBatchApplyService::ApplyBatch(
	const FCFBatchDefinitionApplyRequest& Request,
	FCFBatchDefinitionApplyResult& OutResult)
{
	// Production path에는 mutation 사이에 외부 state를 바꾸는 hook이 없습니다.
	const TFunction<void(int32, UCFVehicleData*)> NoHook;
	return ApplyBatchInternal(Request, OutResult, NoHook);
}

// Production Apply와 private Automation pre-apply hook이 공유하는 실제 B3 execution입니다.
bool FCFBatchApplyService::ApplyBatchInternal(
	const FCFBatchDefinitionApplyRequest& Request,
	FCFBatchDefinitionApplyResult& OutResult,
	const TFunction<void(int32, UCFVehicleData*)>& PreApplyHook)
{
	OutResult = FCFBatchDefinitionApplyResult();
	OutResult.bGlobalRollbackPerformed = false;
	OutResult.bAutomaticRetryPerformed = false;
	OutResult.bSavePerformed = false;
	if (!Request.Plan || !Request.Approval)
	{
		OutResult.Status = ECFBatchApplyStatus::Blocked;
		OutResult.ErrorCode = ECFBatchApplyErrorCode::InvalidRequest;
		OutResult.Message = TEXT("B3 Apply에는 exact Definition Apply Plan과 B3 approval이 필요합니다.");
		return false;
	}
	if (!Request.bDefinitionApplyApproved)
	{
		OutResult.Status = ECFBatchApplyStatus::Blocked;
		OutResult.ErrorCode = ECFBatchApplyErrorCode::ApprovalRequired;
		OutResult.Message = TEXT("B3 Batch Definition Apply는 exact B3 approval이 명시적으로 필요합니다.");
		return false;
	}

	// Reviewed exact B3 plan입니다.
	const FCFBatchDefinitionApplyPlan& Plan = *Request.Plan;
	// Caller가 제출한 exact B3 approval입니다.
	const FCFBatchDefinitionApplyApproval& Approval = *Request.Approval;
	OutResult.BatchApplyPlanHash = Plan.BatchApplyPlanHash;
	CFBatchApplyPrivate::InitializeVehicleResults(Plan, OutResult);

	// Plan self-consistency diagnostic입니다.
	FString ValidationError;
	if (!CFBatchApplyPrivate::ValidatePlanSummary(Plan, ValidationError)
		|| Plan.BatchApplyPlanHash.IsEmpty()
		|| Plan.BatchApplyPlanHash != BuildBatchApplyPlanHash(Plan))
	{
		OutResult.Status = ECFBatchApplyStatus::Blocked;
		OutResult.ErrorCode = ECFBatchApplyErrorCode::PlanInvalid;
		OutResult.Message = ValidationError.IsEmpty() ? TEXT("B3 plan hash가 current plan evidence와 일치하지 않습니다.") : ValidationError;
		return false;
	}

	// Plan에서 server-side로 다시 만든 exact approval입니다.
	FCFBatchDefinitionApplyApproval ExpectedApproval;
	if (!BuildDefinitionApplyApproval(Plan, ExpectedApproval, ValidationError))
	{
		OutResult.Status = Plan.EligibleVehicleCount == 0 ? ECFBatchApplyStatus::NoChange : ECFBatchApplyStatus::Blocked;
		OutResult.ErrorCode = Plan.EligibleVehicleCount == 0 ? ECFBatchApplyErrorCode::None : ECFBatchApplyErrorCode::PlanInvalid;
		OutResult.Message = ValidationError;
		return false;
	}
	// Approval의 exact mismatch taxonomy입니다.
	const ECFBatchApplyErrorCode ApprovalError = CFBatchApplyPrivate::CompareApproval(Approval, ExpectedApproval);
	if (ApprovalError != ECFBatchApplyErrorCode::None)
	{
		OutResult.Status = ECFBatchApplyStatus::Blocked;
		OutResult.ErrorCode = ApprovalError;
		OutResult.Message = TEXT("B3 approval이 exact BatchApplyPlanHash/ordered Target set/per-target R3 evidence와 일치하지 않습니다.");
		return false;
	}

	// Frozen 26.58: 첫 Target mutation 전에 모든 eligible item을 fresh read/resolve합니다.
	for (const FCFBatchDefinitionApplyItem& PlannedItem : Plan.Items)
	{
		if (!PlannedItem.bEligible)
		{
			continue;
		}
		if (!PlannedItem.Recipe || !PlannedItem.TargetVehicleData)
		{
			OutResult.Status = ECFBatchApplyStatus::Blocked;
			OutResult.ErrorCode = ECFBatchApplyErrorCode::GlobalPreflightFailed;
			OutResult.Message = FString::Printf(TEXT("B3 global preflight source pointer가 유효하지 않습니다: %s"), *PlannedItem.TargetPath);
			return false;
		}

		// Commit/approval 이후 current Unreal truth로 다시 만든 exact item입니다.
		FCFBatchDefinitionApplyItem FreshItem;
		if (!CFBatchApplyPrivate::BuildFreshItem(*PlannedItem.Recipe, ECFAuthoringCallerKind::Automation, FreshItem, ValidationError))
		{
			OutResult.Status = ECFBatchApplyStatus::Blocked;
			OutResult.ErrorCode = ECFBatchApplyErrorCode::GlobalPreflightFailed;
			OutResult.Message = ValidationError;
			return false;
		}
		if (!FreshItem.bEligible
			|| FreshItem.TargetVehicleData != PlannedItem.TargetVehicleData
			|| FreshItem.TargetPath != PlannedItem.TargetPath
			|| FreshItem.RecipePath != PlannedItem.RecipePath
			|| FreshItem.R3EvidenceHash != PlannedItem.R3EvidenceHash)
		{
			OutResult.Status = ECFBatchApplyStatus::Blocked;
			OutResult.ErrorCode = ECFBatchApplyErrorCode::GlobalPreflightFailed;
			OutResult.Message = FString::Printf(TEXT("B3 global preflight에서 R3 evidence가 변경됐습니다. Applied Vehicle 0: %s"), *PlannedItem.TargetPath);
			return false;
		}
	}

	// Frozen 26.59 canonical order는 plan Items의 TargetPath ascending과 동일합니다.
	int32 EligibleExecutionIndex = 0;
	for (const FCFBatchDefinitionApplyItem& Item : Plan.Items)
	{
		if (!Item.bEligible)
		{
			continue;
		}
		// Exact target result row입니다.
		FCFBatchVehicleApplyResult* VehicleResult = CFBatchApplyPrivate::FindVehicleResult(OutResult, Item.TargetPath);
		if (!VehicleResult)
		{
			OutResult.Status = OutResult.AppliedVehicleCount > 0 ? ECFBatchApplyStatus::PartialFailure : ECFBatchApplyStatus::Failed;
			OutResult.ErrorCode = ECFBatchApplyErrorCode::InternalError;
			OutResult.Message = TEXT("B3 result target row를 찾지 못했습니다. 이미 Applied Vehicle은 자동 rollback하지 않습니다.");
			return false;
		}

		if (PreApplyHook)
		{
			PreApplyHook(EligibleExecutionIndex, Item.TargetVehicleData);
		}

		// Frozen 26.59 Target writer 단일 lane: 이 호출은 해당 item에 대해 정확히 한 번만 수행됩니다.
		FCFVehicleApplyResult ApplyResult;
		const bool bApplySucceeded = FCFVehicleApplyService::Apply(Item.ApplyRequest, ApplyResult);
		++VehicleResult->ApplyServiceCallCount;
		++OutResult.ApplyServiceCallCount;
		VehicleResult->ApplyStatus = ApplyResult.Status;
		VehicleResult->ApplyFailureCode = ApplyResult.FailureCode;
		VehicleResult->Message = ApplyResult.Message;
		++EligibleExecutionIndex;

		if (!bApplySucceeded || ApplyResult.Status != ECFVehicleApplyStatus::Success)
		{
			VehicleResult->State = ECFBatchVehicleApplyState::Failed;
			VehicleResult->bTargetChanged = ApplyResult.bTargetMutationCommitted;
			VehicleResult->bRecipeAppliedStateChanged = ApplyResult.bRecipeAppliedStateUpdated;
			++OutResult.FailedVehicleCount;
			--OutResult.NotStartedVehicleCount;
			OutResult.Status = OutResult.AppliedVehicleCount > 0 ? ECFBatchApplyStatus::PartialFailure : ECFBatchApplyStatus::Failed;
			OutResult.ErrorCode = ECFBatchApplyErrorCode::VehicleApplyFailed;
			OutResult.Message = FString::Printf(
				TEXT("B3 Stop On First Failure: Target=%s / 이미 성공한 Vehicle=%d / 남은 eligible Vehicle=%d. 자동 rollback/continue/retry는 수행하지 않았습니다."),
				*Item.TargetPath,
				OutResult.AppliedVehicleCount,
				OutResult.NotStartedVehicleCount);
			return false;
		}

		VehicleResult->State = ECFBatchVehicleApplyState::Applied;
		VehicleResult->bTargetChanged = ApplyResult.bTargetMutationCommitted;
		VehicleResult->bRecipeAppliedStateChanged = ApplyResult.bRecipeAppliedStateUpdated;
		++OutResult.AppliedVehicleCount;
		--OutResult.NotStartedVehicleCount;
		OutResult.bAnyTargetChanged |= ApplyResult.bTargetMutationCommitted;
		OutResult.bPackageDirty |= ApplyResult.bTargetMutationCommitted || ApplyResult.bRecipeAppliedStateUpdated;
	}

	OutResult.Status = ECFBatchApplyStatus::Succeeded;
	OutResult.ErrorCode = ECFBatchApplyErrorCode::None;
	OutResult.Message = FString::Printf(
		TEXT("B3 Batch Definition Apply 완료: Applied=%d / Ineligible=%d / ApplyServiceCalls=%d / AutoSave=0."),
		OutResult.AppliedVehicleCount,
		OutResult.IneligibleVehicleCount,
		OutResult.ApplyServiceCallCount);
	return true;
}


