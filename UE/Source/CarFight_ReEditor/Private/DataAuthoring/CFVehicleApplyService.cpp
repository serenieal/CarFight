// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleApplyService.cpp
// Version: v1.1.0
// Date: 2026-08-18
// Description: DAUTH-P0-08H~P0-11 TOCTOU-safe Vehicle Definition Apply Transaction 구현입니다.
// Scope: Fresh precondition, dependency-safe exact diff, transient preflight, Validator/hash readback, AppliedState exact value trace, rollback/no-auto-save를 제공합니다.
// Changelog:
// - v1.1.0: Frozen 24.59 3-way Drift review를 위해 AppliedTrace에 exact typed LastAppliedValue를 추가하되 기존 hash authority를 유지.
// - v1.0.0: Frozen Section 22.33~22.35 A0~A14 Apply lane 최초 구현.
// Migration:
// - Runtime UCFVehicleData schema와 UCFVDAValidator source는 수정하지 않습니다.
// - R14 Foundation의 remove-array 미표현은 Apply plan에서 current-vs-resolved selector 차이로 deterministic 보완하며 Source/Resolver 의미는 변경하지 않습니다.

#include "DataAuthoring/CFVehicleApplyService.h"

#include "CFVDAValidator.h"
#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleApplyPrivate
{
	/** Apply 전에 current UObject에서 다시 계산한 immutable precondition 자료입니다. */
	struct FFreshApplyState
	{
		// Persistent Recipe의 fresh semantic snapshot입니다.
		FCFVehicleRecipeSnapshot RecipeSnapshot;

		// Current Target의 fresh full Definition Snapshot입니다.
		FCFVehicleDefinitionSnapshot TargetSnapshot;

		// Reviewed Profile/Asset context에 fresh Recipe/Target을 주입해 다시 계산한 Resolver result입니다.
		FCFVehicleResolveResult ResolveResult;
	};

	// Apply 결과를 mutation 없는 Blocked 상태로 설정합니다.
	void SetBlocked(
		FCFVehicleApplyResult& OutResult,
		const ECFVehicleApplyFailureCode FailureCode,
		const FString& Message)
	{
		OutResult.Status = ECFVehicleApplyStatus::Blocked;
		OutResult.FailureCode = FailureCode;
		OutResult.Message = Message;
		OutResult.bTargetMutationCommitted = false;
		OutResult.bRecipeAppliedStateUpdated = false;
	}

	// Apply 결과를 계산/transaction Error 상태로 설정합니다.
	void SetError(
		FCFVehicleApplyResult& OutResult,
		const ECFVehicleApplyFailureCode FailureCode,
		const FString& Message)
	{
		OutResult.Status = ECFVehicleApplyStatus::Error;
		OutResult.FailureCode = FailureCode;
		OutResult.Message = Message;
		OutResult.bTargetMutationCommitted = false;
		OutResult.bRecipeAppliedStateUpdated = false;
	}

	// Existing UCFVDAValidator severity를 Authoring Apply validation severity로 변환합니다.
	ECFVehicleValidationSeverity ConvertValidationSeverity(const ECFVDASeverity Severity)
	{
		switch (Severity)
		{
		case ECFVDASeverity::Info:
			return ECFVehicleValidationSeverity::Info;
		case ECFVDASeverity::Warning:
			return ECFVehicleValidationSeverity::Warning;
		case ECFVDASeverity::Blocked:
			return ECFVehicleValidationSeverity::Blocked;
		case ECFVDASeverity::Error:
			return ECFVehicleValidationSeverity::Error;
		case ECFVDASeverity::Pass:
		default:
			return ECFVehicleValidationSeverity::Info;
		}
	}

	// Existing Validator report를 public Apply validation rows로 변환하고 blocking 여부를 반환합니다.
	bool ConvertValidationReport(
		const FCFVDAValidationReport& ValidationReport,
		TArray<FCFVehicleValidationIssue>& OutIssues)
	{
		OutIssues.Reset();
		// Error/Blocked issue가 하나 이상 존재하는지 누적합니다.
		bool bHasBlockingIssue = false;
		for (const FCFVDAValidationItem& ValidationItem : ValidationReport.Items)
		{
			if (ValidationItem.Severity == ECFVDASeverity::Pass)
			{
				continue;
			}

			// 기존 Validator 의미를 보존한 Authoring severity입니다.
			const ECFVehicleValidationSeverity Severity = ConvertValidationSeverity(ValidationItem.Severity);
			// Public Apply result에 추가할 validation issue입니다.
			FCFVehicleValidationIssue& Issue = OutIssues.AddDefaulted_GetRef();
			Issue.Severity = Severity;
			Issue.IssueCode = FName(*FString::Printf(TEXT("Definition.%s"), *ValidationItem.GroupName.ToString()));
			Issue.ValidatorFieldPath = ValidationItem.FieldPath;
			Issue.Message = ValidationItem.Message.ToString();
			if (!ValidationItem.RecommendedAction.IsEmpty())
			{
				Issue.Message += FString::Printf(TEXT(" | 권장 조치: %s"), *ValidationItem.RecommendedAction.ToString());
			}
			bHasBlockingIssue |= Severity == ECFVehicleValidationSeverity::Error || Severity == ECFVehicleValidationSeverity::Blocked;
		}
		return bHasBlockingIssue;
	}

	// Exact field path를 소유하는 Frozen Registry descriptor를 찾습니다.
	const FCFVehicleFieldDescriptor* FindDescriptor(const FCFVehicleFieldPath& ExactPath)
	{
		return FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([&ExactPath](const FCFVehicleFieldDescriptor& Descriptor)
		{
			const FCFVehicleFieldPath& Pattern = Descriptor.StablePathPattern;
			if (ExactPath.CollectionPropertyName != Pattern.CollectionPropertyName
				|| ExactPath.SelectorKeyPropertyName != Pattern.SelectorKeyPropertyName
				|| ExactPath.PropertyChain != Pattern.PropertyChain)
			{
				return false;
			}
			return Pattern.CollectionPropertyName.IsNone() ? ExactPath.SelectorKeyValue.IsNone() : !ExactPath.SelectorKeyValue.IsNone();
		});
	}

	// Stable-ID collection의 identity descriptor path에 exact selector를 채웁니다.
	bool MakeIdentityPath(
		const FName CollectionPropertyName,
		const FName SelectorValue,
		FCFVehicleFieldPath& OutIdentityPath,
		FString& OutError)
	{
		// Requested collection의 identity descriptor입니다.
		const FCFVehicleFieldDescriptor* IdentityDescriptor = FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate(
			[CollectionPropertyName](const FCFVehicleFieldDescriptor& Descriptor)
			{
				return Descriptor.bIdentityField && Descriptor.StablePathPattern.CollectionPropertyName == CollectionPropertyName;
			});
		if (!IdentityDescriptor)
		{
			OutError = FString::Printf(TEXT("Stable-ID collection identity descriptor를 찾을 수 없습니다: %s"), *CollectionPropertyName.ToString());
			return false;
		}

		OutIdentityPath = IdentityDescriptor->StablePathPattern;
		OutIdentityPath.SelectorKeyValue = SelectorValue;
		OutError.Reset();
		return true;
	}

	// Current/Resolved exact field set에서 collection별 selector 집합을 수집합니다.
	template <typename EntryType>
	TMap<FName, TSet<FName>> CollectSelectorSets(const TArray<EntryType>& Entries)
	{
		// Collection 이름별 exact stable selector 집합입니다.
		TMap<FName, TSet<FName>> Result;
		for (const EntryType& Entry : Entries)
		{
			if (!Entry.FieldPath.CollectionPropertyName.IsNone() && !Entry.FieldPath.SelectorKeyValue.IsNone())
			{
				Result.FindOrAdd(Entry.FieldPath.CollectionPropertyName).Add(Entry.FieldPath.SelectorKeyValue);
			}
		}
		return Result;
	}

	// Reviewed R14 diff가 fresh Resolver에서 동일하게 재생성됐는지 exact canonical value로 검사합니다.
	bool AreDiffsEquivalent(
		const TArray<FCFVehicleFieldDiff>& ReviewedDiff,
		const TArray<FCFVehicleFieldDiff>& FreshDiff)
	{
		if (ReviewedDiff.Num() != FreshDiff.Num())
		{
			return false;
		}

		for (int32 DiffIndex = 0; DiffIndex < ReviewedDiff.Num(); ++DiffIndex)
		{
			// 사용자가 검토한 diff row입니다.
			const FCFVehicleFieldDiff& Reviewed = ReviewedDiff[DiffIndex];
			// Apply 직전 fresh Resolver diff row입니다.
			const FCFVehicleFieldDiff& Fresh = FreshDiff[DiffIndex];
			if (Reviewed.Operation != Fresh.Operation
				|| Reviewed.FieldPath.ToCanonicalString(true) != Fresh.FieldPath.ToCanonicalString(true)
				|| Reviewed.bHasBeforeValue != Fresh.bHasBeforeValue
				|| Reviewed.bHasAfterValue != Fresh.bHasAfterValue
				|| Reviewed.SourceTraceIndex != Fresh.SourceTraceIndex)
			{
				return false;
			}

			if (Reviewed.bHasBeforeValue
				&& (Reviewed.BeforeValue.PropertyTypeSignature != Fresh.BeforeValue.PropertyTypeSignature
					|| Reviewed.BeforeValue.CanonicalValueText != Fresh.BeforeValue.CanonicalValueText))
			{
				return false;
			}
			if (Reviewed.bHasAfterValue
				&& (Reviewed.AfterValue.PropertyTypeSignature != Fresh.AfterValue.PropertyTypeSignature
					|| Reviewed.AfterValue.CanonicalValueText != Fresh.AfterValue.CanonicalValueText))
			{
				return false;
			}
		}
		return true;
	}

	// Frozen preconditions와 current UObject truth를 fresh Snapshot/Resolver로 다시 계산합니다.
	bool BuildFreshApplyState(
		const FCFVehicleApplyRequest& Request,
		FFreshApplyState& OutFreshState,
		FCFVehicleApplyResult& OutResult)
	{
		if (!Request.Recipe || !Request.TargetVehicleData)
		{
			SetError(OutResult, ECFVehicleApplyFailureCode::InvalidRequest, TEXT("Recipe 또는 Target VehicleData가 null입니다."));
			return false;
		}
		if (Request.ExpectedRecipeFingerprint.IsEmpty()
			|| Request.ExpectedSourceSignature.IsEmpty()
			|| Request.ExpectedTargetDefinitionHash.IsEmpty()
			|| Request.ExpectedResolvedDefinitionHash.IsEmpty())
		{
			SetError(OutResult, ECFVehicleApplyFailureCode::InvalidRequest, TEXT("Apply TOCTOU expected hash/signature가 비어 있습니다."));
			return false;
		}

		if (Request.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision
			|| Request.ApprovedResolveResult.ResolverContractRevision != Request.ExpectedResolverContractRevision
			|| Request.ResolveRequest.ResolverContractRevision != Request.ExpectedResolverContractRevision)
		{
			SetBlocked(OutResult, ECFVehicleApplyFailureCode::PreviewOutOfDate, TEXT("Resolver contract revision이 Preview 이후 변경됐습니다."));
			return false;
		}

		// Fresh Recipe Snapshot build 실패 이유입니다.
		FString SnapshotError;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Request.Recipe, OutFreshState.RecipeSnapshot, SnapshotError)
			|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Request.TargetVehicleData, OutFreshState.TargetSnapshot, SnapshotError))
		{
			SetError(OutResult, ECFVehicleApplyFailureCode::InternalError, SnapshotError);
			return false;
		}

		// Recipe가 binding한 actual Target object path입니다.
		const FSoftObjectPath ActualTargetPath(Request.TargetVehicleData);
		if (OutFreshState.RecipeSnapshot.TargetVehicleDataPath.IsValid()
			&& OutFreshState.RecipeSnapshot.TargetVehicleDataPath != ActualTargetPath)
		{
			SetBlocked(OutResult, ECFVehicleApplyFailureCode::PreviewOutOfDate, TEXT("Recipe Target binding과 Apply Target이 다릅니다."));
			return false;
		}

		if (OutFreshState.RecipeSnapshot.RecipeFingerprint != Request.ExpectedRecipeFingerprint
			|| OutFreshState.TargetSnapshot.DefinitionHash != Request.ExpectedTargetDefinitionHash)
		{
			SetBlocked(OutResult, ECFVehicleApplyFailureCode::PreviewOutOfDate, TEXT("Recipe 또는 Target Definition이 Preview 이후 변경됐습니다."));
			return false;
		}

		if (Request.ApprovedResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success
			|| Request.ApprovedResolveResult.SourceSignature != Request.ExpectedSourceSignature
			|| Request.ApprovedResolveResult.ResolvedDefinitionHash != Request.ExpectedResolvedDefinitionHash)
		{
			SetBlocked(OutResult, ECFVehicleApplyFailureCode::PreviewOutOfDate, TEXT("Approved Resolve payload가 expected Preview signature/hash와 일치하지 않습니다."));
			return false;
		}

		// Reviewed immutable Profile/Asset context에 current Recipe/Target truth를 주입한 fresh Resolver request입니다.
		FCFVehicleResolveRequest FreshResolveRequest = Request.ResolveRequest;
		FreshResolveRequest.Recipe = OutFreshState.RecipeSnapshot;
		FreshResolveRequest.bHasCurrentDefinition = true;
		FreshResolveRequest.CurrentDefinition = OutFreshState.TargetSnapshot;
		FreshResolveRequest.ResolverContractRevision = Request.ExpectedResolverContractRevision;
		if (!FCFVehicleResolver::Resolve(FreshResolveRequest, OutFreshState.ResolveResult))
		{
			SetError(OutResult, ECFVehicleApplyFailureCode::InternalError, TEXT("Apply 직전 fresh Resolver가 internal Error로 실패했습니다."));
			return false;
		}
		if (OutFreshState.ResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
		{
			SetBlocked(OutResult, ECFVehicleApplyFailureCode::ResolveNotSuccessful, TEXT("Apply 직전 fresh Resolver가 Success가 아니므로 Definition Apply를 차단했습니다."));
			return false;
		}

		if (OutFreshState.ResolveResult.SourceSignature != Request.ExpectedSourceSignature
			|| OutFreshState.ResolveResult.ResolvedDefinitionHash != Request.ExpectedResolvedDefinitionHash
			|| OutFreshState.ResolveResult.ResolverContractRevision != Request.ExpectedResolverContractRevision)
		{
			SetBlocked(OutResult, ECFVehicleApplyFailureCode::PreviewOutOfDate, TEXT("Apply 직전 fresh Resolve signature/hash가 approved Preview와 달라졌습니다."));
			return false;
		}

		if (!AreDiffsEquivalent(Request.ApprovedResolveResult.FieldDiff, OutFreshState.ResolveResult.FieldDiff))
		{
			SetBlocked(OutResult, ECFVehicleApplyFailureCode::ReviewedDiffMismatch, TEXT("Apply 직전 fresh Diff가 사용자가 검토한 Diff와 다릅니다."));
			return false;
		}
		return true;
	}

	// R14 reviewed rows에 current-only stable selector Remove operation을 보완하고 Frozen dependency order로 정렬합니다.
	bool BuildDependencySafeApplyPlan(
		const FCFVehicleDefinitionSnapshot& CurrentTarget,
		const FCFVehicleResolveResult& FreshResolveResult,
		TArray<FCFVehicleFieldDiff>& OutApplyPlan,
		FString& OutError)
	{
		OutApplyPlan.Reset();
		// Current Target collection별 selector 집합입니다.
		const TMap<FName, TSet<FName>> CurrentSelectors = CollectSelectorSets(CurrentTarget.SortedFields);
		// Resolved Definition collection별 desired selector 집합입니다.
		const TMap<FName, TSet<FName>> DesiredSelectors = CollectSelectorSets(FreshResolveResult.SortedResolvedFields);

		for (const TPair<FName, TSet<FName>>& CurrentCollection : CurrentSelectors)
		{
			// 같은 collection의 desired stable selector 집합입니다.
			const TSet<FName>* DesiredSet = DesiredSelectors.Find(CurrentCollection.Key);
			for (const FName CurrentSelector : CurrentCollection.Value)
			{
				if (DesiredSet && DesiredSet->Contains(CurrentSelector))
				{
					continue;
				}

				// Current-only stable selector를 제거할 identity path입니다.
				FCFVehicleFieldPath IdentityPath;
				if (!MakeIdentityPath(CurrentCollection.Key, CurrentSelector, IdentityPath, OutError))
				{
					return false;
				}

				// R14 Foundation이 아직 표현하지 않는 whole-element removal operation입니다.
				FCFVehicleFieldDiff& RemoveDiff = OutApplyPlan.AddDefaulted_GetRef();
				RemoveDiff.Operation = ECFVehicleDiffOp::RemoveArrayElement;
				RemoveDiff.FieldPath = MoveTemp(IdentityPath);
			}
		}

		OutApplyPlan.Append(FreshResolveResult.FieldDiff);

		// Frozen Section 22.31 dependency-safe operation rank를 반환합니다.
		auto GetOperationRank = [](const FCFVehicleFieldDiff& Diff) -> int32
		{
			const FName CollectionName = Diff.FieldPath.CollectionPropertyName;
			if (Diff.Operation == ECFVehicleDiffOp::RemoveArrayElement)
			{
				return CollectionName == TEXT("MountProfiles") ? 0 : 1;
			}
			if (Diff.Operation == ECFVehicleDiffOp::AddArrayElement)
			{
				return CollectionName == TEXT("HardpointSlots") ? 2 : 3;
			}
			if (Diff.Operation == ECFVehicleDiffOp::SetLeaf)
			{
				return CollectionName.IsNone() ? 4 : 5;
			}
			return 6;
		};

		OutApplyPlan.Sort([&GetOperationRank](const FCFVehicleFieldDiff& Left, const FCFVehicleFieldDiff& Right)
		{
			// Section 22.31 dependency rank입니다.
			const int32 LeftRank = GetOperationRank(Left);
			// Section 22.31 dependency rank입니다.
			const int32 RightRank = GetOperationRank(Right);
			if (LeftRank != RightRank)
			{
				return LeftRank < RightRank;
			}
			return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
		});

		if (OutApplyPlan.ContainsByPredicate([](const FCFVehicleFieldDiff& Diff)
		{
			return Diff.Operation == ECFVehicleDiffOp::MoveArrayElement;
		}))
		{
			OutError = TEXT("현재 Frozen Recipe에는 explicit persisted array order intent가 없어 MoveArrayElement Apply를 실행할 수 없습니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Stable-ID collection에서 exact selector element의 physical index를 찾습니다.
	bool FindCollectionElementIndex(
		UCFVehicleData& Target,
		const FCFVehicleFieldPath& FieldPath,
		const FArrayProperty*& OutArrayProperty,
		const FStructProperty*& OutInnerStructProperty,
		int32& OutElementIndex,
		FString& OutError)
	{
		OutArrayProperty = FindFProperty<FArrayProperty>(Target.GetClass(), FieldPath.CollectionPropertyName);
		OutInnerStructProperty = OutArrayProperty ? CastField<FStructProperty>(OutArrayProperty->Inner) : nullptr;
		if (!OutArrayProperty || !OutInnerStructProperty || !OutInnerStructProperty->Struct)
		{
			OutError = FString::Printf(TEXT("Apply stable collection schema가 유효하지 않습니다: %s"), *FieldPath.CollectionPropertyName.ToString());
			return false;
		}

		// Collection element identity를 읽을 selector FName property입니다.
		const FNameProperty* SelectorProperty = FindFProperty<FNameProperty>(OutInnerStructProperty->Struct, FieldPath.SelectorKeyPropertyName);
		if (!SelectorProperty)
		{
			OutError = FString::Printf(TEXT("Apply stable selector property가 유효하지 않습니다: %s"), *FieldPath.SelectorKeyPropertyName.ToString());
			return false;
		}

		// Current collection array storage입니다.
		void* ArrayValueAddress = OutArrayProperty->ContainerPtrToValuePtr<void>(&Target);
		// Stable selector를 physical index로 찾을 reflection helper입니다.
		FScriptArrayHelper ArrayHelper(OutArrayProperty, ArrayValueAddress);
		OutElementIndex = INDEX_NONE;
		for (int32 ElementIndex = 0; ElementIndex < ArrayHelper.Num(); ++ElementIndex)
		{
			// Current physical array element storage입니다.
			const void* ElementAddress = ArrayHelper.GetRawPtr(ElementIndex);
			if (SelectorProperty->GetPropertyValue_InContainer(ElementAddress) == FieldPath.SelectorKeyValue)
			{
				OutElementIndex = ElementIndex;
				break;
			}
		}
		OutError.Reset();
		return true;
	}

	// SetLeaf operation이 가리키는 exact mutable reflected leaf를 찾습니다.
	bool ResolveWritableLeaf(
		UCFVehicleData& Target,
		const FCFVehicleFieldPath& FieldPath,
		const FProperty*& OutLeafProperty,
		void*& OutLeafValueAddress,
		FString& OutError)
	{
		OutLeafProperty = nullptr;
		OutLeafValueAddress = nullptr;

		// Property traversal을 시작할 schema입니다.
		const UStruct* CurrentStruct = Target.GetClass();
		// Current property value의 owning container입니다.
		void* CurrentContainer = &Target;
		if (!FieldPath.CollectionPropertyName.IsNone())
		{
			// Stable collection reflected array property입니다.
			const FArrayProperty* ArrayProperty = nullptr;
			// Stable collection element struct schema입니다.
			const FStructProperty* InnerStructProperty = nullptr;
			// Exact selector의 current physical index입니다.
			int32 ElementIndex = INDEX_NONE;
			if (!FindCollectionElementIndex(Target, FieldPath, ArrayProperty, InnerStructProperty, ElementIndex, OutError)
				|| !ArrayProperty || !InnerStructProperty || ElementIndex == INDEX_NONE)
			{
				if (OutError.IsEmpty())
				{
					OutError = FString::Printf(TEXT("SetLeaf selector element를 찾을 수 없습니다: %s"), *FieldPath.ToCanonicalString(true));
				}
				return false;
			}

			// Current collection array storage입니다.
			void* ArrayValueAddress = ArrayProperty->ContainerPtrToValuePtr<void>(&Target);
			// Exact physical element storage를 찾을 helper입니다.
			FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayValueAddress);
			CurrentStruct = InnerStructProperty->Struct;
			CurrentContainer = ArrayHelper.GetRawPtr(ElementIndex);
		}

		if (FieldPath.PropertyChain.IsEmpty())
		{
			OutError = TEXT("Apply SetLeaf property chain이 비어 있습니다.");
			return false;
		}

		for (int32 ChainIndex = 0; ChainIndex < FieldPath.PropertyChain.Num(); ++ChainIndex)
		{
			// Current traversal segment property입니다.
			const FProperty* Property = FindFProperty<FProperty>(CurrentStruct, FieldPath.PropertyChain[ChainIndex]);
			if (!Property)
			{
				OutError = FString::Printf(TEXT("Apply target property를 찾을 수 없습니다: %s"), *FieldPath.ToCanonicalString(true));
				return false;
			}
			// Current property mutable value address입니다.
			void* PropertyValueAddress = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
			if (ChainIndex == FieldPath.PropertyChain.Num() - 1)
			{
				OutLeafProperty = Property;
				OutLeafValueAddress = PropertyValueAddress;
				OutError.Reset();
				return true;
			}

			// Nested chain 중간 segment의 struct property입니다.
			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty || !StructProperty->Struct)
			{
				OutError = FString::Printf(TEXT("Apply nested path 중간 segment가 struct가 아닙니다: %s"), *FieldPath.ToCanonicalString(true));
				return false;
			}
			CurrentStruct = StructProperty->Struct;
			CurrentContainer = PropertyValueAddress;
		}

		OutError = TEXT("Apply SetLeaf resolution이 leaf 없이 종료됐습니다.");
		return false;
	}

	// AddArrayElement operation으로 exact stable selector element를 default construct합니다.
	bool ApplyAddArrayElement(
		UCFVehicleData& Target,
		const FCFVehicleFieldDiff& Diff,
		FString& OutError)
	{
		// Stable collection reflected array property입니다.
		const FArrayProperty* ArrayProperty = nullptr;
		// Stable collection element struct schema입니다.
		const FStructProperty* InnerStructProperty = nullptr;
		// Existing selector physical index입니다.
		int32 ExistingIndex = INDEX_NONE;
		if (!FindCollectionElementIndex(Target, Diff.FieldPath, ArrayProperty, InnerStructProperty, ExistingIndex, OutError)
			|| !ArrayProperty || !InnerStructProperty)
		{
			return false;
		}
		if (ExistingIndex != INDEX_NONE)
		{
			OutError = FString::Printf(TEXT("AddArrayElement selector가 이미 존재합니다: %s"), *Diff.FieldPath.ToCanonicalString(true));
			return false;
		}

		// Stable selector identity FName property입니다.
		const FNameProperty* SelectorProperty = FindFProperty<FNameProperty>(InnerStructProperty->Struct, Diff.FieldPath.SelectorKeyPropertyName);
		if (!SelectorProperty)
		{
			OutError = TEXT("AddArrayElement selector property를 찾을 수 없습니다.");
			return false;
		}

		// Target stable collection array storage입니다.
		void* ArrayValueAddress = ArrayProperty->ContainerPtrToValuePtr<void>(&Target);
		// 새 element를 생성할 reflection helper입니다.
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayValueAddress);
		// Default-constructed element physical index입니다.
		const int32 NewElementIndex = ArrayHelper.AddValue();
		// 새 element struct storage입니다.
		void* NewElementAddress = ArrayHelper.GetRawPtr(NewElementIndex);
		// 새 element selector storage입니다.
		void* SelectorValueAddress = SelectorProperty->ContainerPtrToValuePtr<void>(NewElementAddress);
		SelectorProperty->SetPropertyValue(SelectorValueAddress, Diff.FieldPath.SelectorKeyValue);

		if (Diff.bHasAfterValue)
		{
			// Reviewed identity value를 exact reflected selector leaf에 재import해 type/value consistency를 검사합니다.
			if (!FCFVehicleFieldCodec::ImportValue(*SelectorProperty, SelectorValueAddress, &Target, Diff.AfterValue, OutError))
			{
				return false;
			}
		}
		return true;
	}

	// RemoveArrayElement operation으로 exact stable selector element를 제거합니다.
	bool ApplyRemoveArrayElement(
		UCFVehicleData& Target,
		const FCFVehicleFieldDiff& Diff,
		FString& OutError)
	{
		// Stable collection reflected array property입니다.
		const FArrayProperty* ArrayProperty = nullptr;
		// Stable collection element struct schema입니다.
		const FStructProperty* InnerStructProperty = nullptr;
		// 제거할 selector의 physical index입니다.
		int32 ElementIndex = INDEX_NONE;
		if (!FindCollectionElementIndex(Target, Diff.FieldPath, ArrayProperty, InnerStructProperty, ElementIndex, OutError)
			|| !ArrayProperty || !InnerStructProperty)
		{
			return false;
		}
		if (ElementIndex == INDEX_NONE)
		{
			OutError = FString::Printf(TEXT("RemoveArrayElement selector가 존재하지 않습니다: %s"), *Diff.FieldPath.ToCanonicalString(true));
			return false;
		}

		// Target stable collection array storage입니다.
		void* ArrayValueAddress = ArrayProperty->ContainerPtrToValuePtr<void>(&Target);
		// Exact element를 제거할 reflection helper입니다.
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayValueAddress);
		ArrayHelper.RemoveValues(ElementIndex, 1);
		OutError.Reset();
		return true;
	}

	// SetLeaf operation으로 reviewed canonical after value를 exact target leaf에 import합니다.
	bool ApplySetLeaf(
		UCFVehicleData& Target,
		const FCFVehicleFieldDiff& Diff,
		FString& OutError)
	{
		if (!Diff.bHasAfterValue)
		{
			OutError = FString::Printf(TEXT("SetLeaf에 AfterValue가 없습니다: %s"), *Diff.FieldPath.ToCanonicalString(true));
			return false;
		}

		// Exact target reflected leaf property입니다.
		const FProperty* LeafProperty = nullptr;
		// Exact target mutable leaf storage입니다.
		void* LeafValueAddress = nullptr;
		if (!ResolveWritableLeaf(Target, Diff.FieldPath, LeafProperty, LeafValueAddress, OutError) || !LeafProperty)
		{
			return false;
		}
		return FCFVehicleFieldCodec::ImportValue(*LeafProperty, LeafValueAddress, &Target, Diff.AfterValue, OutError);
	}

	// Frozen dependency-safe plan을 exact operation 순서대로 하나의 VehicleData object에 적용합니다.
	bool ApplyPlanToVehicleData(
		UCFVehicleData& Target,
		const TArray<FCFVehicleFieldDiff>& ApplyPlan,
		FString& OutError)
	{
		for (const FCFVehicleFieldDiff& Diff : ApplyPlan)
		{
			switch (Diff.Operation)
			{
			case ECFVehicleDiffOp::RemoveArrayElement:
				if (!ApplyRemoveArrayElement(Target, Diff, OutError))
				{
					return false;
				}
				break;
			case ECFVehicleDiffOp::AddArrayElement:
				if (!ApplyAddArrayElement(Target, Diff, OutError))
				{
					return false;
				}
				break;
			case ECFVehicleDiffOp::SetLeaf:
				if (!ApplySetLeaf(Target, Diff, OutError))
				{
					return false;
				}
				break;
			case ECFVehicleDiffOp::MoveArrayElement:
			default:
				OutError = FString::Printf(TEXT("현재 Apply Foundation이 지원하지 않는 Diff operation입니다: %d"), static_cast<int32>(Diff.Operation));
				return false;
			}
		}
		OutError.Reset();
		return true;
	}

	// VehicleData full Snapshot에서 Resolver-owned exact field set만 projection해 expected hash authority와 비교할 Snapshot을 만듭니다.
	bool BuildResolvedProjection(
		const UCFVehicleData& Target,
		const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
		FCFVehicleDefinitionSnapshot& OutProjection,
		FString& OutError)
	{
		// Target 전체 Registry-expanded fresh Snapshot입니다.
		FCFVehicleDefinitionSnapshot FullSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, FullSnapshot, OutError))
		{
			return false;
		}

		// Exact canonical path에서 full snapshot entry를 찾는 lookup입니다.
		TMap<FString, const FCFVehicleFieldEntry*> EntryByPath;
		EntryByPath.Reserve(FullSnapshot.SortedFields.Num());
		for (const FCFVehicleFieldEntry& Entry : FullSnapshot.SortedFields)
		{
			EntryByPath.Add(Entry.FieldPath.ToCanonicalString(true), &Entry);
		}

		OutProjection = FCFVehicleDefinitionSnapshot();
		OutProjection.SortedFields.Reserve(SortedResolvedFields.Num());
		for (const FCFVehicleResolvedField& ResolvedField : SortedResolvedFields)
		{
			// Resolver-owned exact canonical field path입니다.
			const FString CanonicalPath = ResolvedField.FieldPath.ToCanonicalString(true);
			// Target full Snapshot에서 일치하는 actual readback entry입니다.
			const FCFVehicleFieldEntry* const* FoundEntry = EntryByPath.Find(CanonicalPath);
			if (!FoundEntry || !*FoundEntry)
			{
				OutError = FString::Printf(TEXT("Target readback에 Resolver-owned field가 없습니다: %s"), *CanonicalPath);
				return false;
			}
			OutProjection.SortedFields.Add(**FoundEntry);
		}

		if (!FCFVehicleSnapshotBuilder::BuildDefinitionHashFromFields(OutProjection.SortedFields, OutProjection.DefinitionHash, OutError))
		{
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Fresh Resolve Source Trace를 persistent Recipe AppliedState field traces로 변환합니다.
	bool BuildAppliedState(
		const UCFVehicleRecipeData& Recipe,
		const FFreshApplyState& FreshState,
		FCFVehicleAppliedState& OutAppliedState,
		FString& OutError)
	{
		OutAppliedState = FCFVehicleAppliedState();
		OutAppliedState.AppliedRecipeRevision = Recipe.AuthoringRevision;
		OutAppliedState.AppliedRecipeFingerprint = FreshState.RecipeSnapshot.RecipeFingerprint;
		OutAppliedState.AppliedSourceSignature = FreshState.ResolveResult.SourceSignature;
		OutAppliedState.AppliedDefinitionHash = FreshState.ResolveResult.ResolvedDefinitionHash;
		OutAppliedState.ResolverContractRevision = FreshState.ResolveResult.ResolverContractRevision;
		OutAppliedState.FieldTraces.Reserve(FreshState.ResolveResult.SortedResolvedFields.Num());

		for (const FCFVehicleResolvedField& ResolvedField : FreshState.ResolveResult.SortedResolvedFields)
		{
			if (!FreshState.ResolveResult.PreviewSourceTrace.IsValidIndex(ResolvedField.SourceTraceIndex))
			{
				OutError = FString::Printf(TEXT("AppliedState SourceTraceIndex가 유효하지 않습니다: %s"), *ResolvedField.FieldPath.ToCanonicalString(true));
				return false;
			}
			// Resolved field의 exact Source Trace입니다.
			const FCFVehicleSourceTrace& SourceTrace = FreshState.ResolveResult.PreviewSourceTrace[ResolvedField.SourceTraceIndex];
			if (!SourceTrace.Layers.IsValidIndex(SourceTrace.EffectiveLayerIndex))
			{
				OutError = FString::Printf(TEXT("AppliedState effective Source layer가 유효하지 않습니다: %s"), *ResolvedField.FieldPath.ToCanonicalString(true));
				return false;
			}
			// Effective winner source layer입니다.
			const FCFVehicleSourceLayer& EffectiveLayer = SourceTrace.Layers[SourceTrace.EffectiveLayerIndex];
			// Persistent field-level stale/drift authority row입니다.
			FCFVehicleAppliedTrace& AppliedTrace = OutAppliedState.FieldTraces.AddDefaulted_GetRef();
			AppliedTrace.FieldPath = ResolvedField.FieldPath;
			AppliedTrace.EffectiveSourceType = EffectiveLayer.SourceType;
			AppliedTrace.EffectiveSourceId = EffectiveLayer.SourceId;
						AppliedTrace.EffectiveSourceSignature = SourceTrace.EffectiveSourceSignature;
			AppliedTrace.ShadowSourceSignature = SourceTrace.ShadowSourceSignature;
			AppliedTrace.LastAppliedValue = ResolvedField.Value;
			AppliedTrace.LastAppliedValueHash = FCFVehicleFieldCodec::HashValue(ResolvedField.Value);
		}
		OutError.Reset();
		return true;
	}

	// Same-class UPROPERTY storage 전체를 backup object에서 Target object로 복원합니다.
	bool RestoreTargetFromBackup(
		UCFVehicleData& Target,
		const UCFVehicleData& Backup,
		FString& OutError)
	{
		if (Target.GetClass() != Backup.GetClass())
		{
			OutError = TEXT("Rollback backup class가 Target class와 다릅니다.");
			return false;
		}

		for (TFieldIterator<FProperty> PropertyIt(Target.GetClass(), EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			// 같은 class backup에서 복원할 reflected property입니다.
			FProperty* Property = *PropertyIt;
			Property->CopyCompleteValue_InContainer(&Target, &Backup);
		}
		OutError.Reset();
		return true;
	}

	// Transaction 실패 시 Target + Recipe AppliedState + package dirty flags를 pre-apply 상태로 복원하고 검증합니다.
	bool RollbackTransaction(
		UCFVehicleData& Target,
		UCFVehicleRecipeData& Recipe,
		const UCFVehicleData& TargetBackup,
		const FCFVehicleAppliedState& AppliedStateBackup,
		const FCFVehicleDefinitionSnapshot& TargetSnapshotBeforeApply,
		const bool bTargetPackageWasDirty,
		const bool bRecipePackageWasDirty,
		FScopedTransaction& Transaction,
		FString& OutError)
	{
		if (!RestoreTargetFromBackup(Target, TargetBackup, OutError))
		{
			Transaction.Cancel();
			return false;
		}
		Recipe.AppliedState = AppliedStateBackup;
		Transaction.Cancel();

		if (UPackage* TargetPackage = Target.GetOutermost())
		{
			TargetPackage->SetDirtyFlag(bTargetPackageWasDirty);
		}
		if (UPackage* RecipePackage = Recipe.GetOutermost())
		{
			RecipePackage->SetDirtyFlag(bRecipePackageWasDirty);
		}

		// Rollback 뒤 exact full Target Definition Snapshot입니다.
		FCFVehicleDefinitionSnapshot RolledBackSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, RolledBackSnapshot, OutError))
		{
			return false;
		}
		if (RolledBackSnapshot.DefinitionHash != TargetSnapshotBeforeApply.DefinitionHash)
		{
			OutError = FString::Printf(TEXT("Rollback Target hash가 pre-apply hash와 다릅니다. Before=%s After=%s"), *TargetSnapshotBeforeApply.DefinitionHash, *RolledBackSnapshot.DefinitionHash);
			return false;
		}
		OutError.Reset();
		return true;
	}
}

// Reviewed Preview를 TOCTOU 재검증한 뒤 Target + Recipe AppliedState를 atomic transaction으로 적용합니다.
bool FCFVehicleApplyService::Apply(
	const FCFVehicleApplyRequest& Request,
	FCFVehicleApplyResult& OutResult)
{
	return ApplyInternal(Request, OutResult, false);
}

// Production Apply와 Automation rollback probe가 공유하는 실제 transaction implementation입니다.
bool FCFVehicleApplyService::ApplyInternal(
	const FCFVehicleApplyRequest& Request,
	FCFVehicleApplyResult& OutResult,
	const bool bInjectFailureAfterTargetMutation)
{
	OutResult = FCFVehicleApplyResult();

	// A0~A1 current UObject truth를 반영한 fresh precondition state입니다.
	CFVehicleApplyPrivate::FFreshApplyState FreshState;
	if (!CFVehicleApplyPrivate::BuildFreshApplyState(Request, FreshState, OutResult))
	{
		return false;
	}

	// Fresh R14 rows + current-only array removals을 Section 22.31 dependency order로 정렬한 actual Apply plan입니다.
	TArray<FCFVehicleFieldDiff> ApplyPlan;
	// Apply plan 구성 실패 이유입니다.
	FString ApplyError;
	if (!CFVehicleApplyPrivate::BuildDependencySafeApplyPlan(FreshState.TargetSnapshot, FreshState.ResolveResult, ApplyPlan, ApplyError))
	{
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::InternalError, ApplyError);
		return false;
	}
	OutResult.AppliedDiffOperationCount = ApplyPlan.Num();

	// A2 Current Target을 transient duplicate한 preflight candidate입니다.
	TStrongObjectPtr<UCFVehicleData> PreflightCandidate(DuplicateObject<UCFVehicleData>(Request.TargetVehicleData, GetTransientPackage()));
	if (!PreflightCandidate.IsValid())
	{
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::InternalError, TEXT("Preflight Target duplicate 생성에 실패했습니다."));
		return false;
	}
	if (!CFVehicleApplyPrivate::ApplyPlanToVehicleData(*PreflightCandidate, ApplyPlan, ApplyError))
	{
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::PreflightApplyFailed, ApplyError);
		return false;
	}

	// Preflight exact diff 적용 뒤 Resolver-owned projection readback입니다.
	FCFVehicleDefinitionSnapshot PreflightProjection;
	if (!CFVehicleApplyPrivate::BuildResolvedProjection(*PreflightCandidate, FreshState.ResolveResult.SortedResolvedFields, PreflightProjection, ApplyError))
	{
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::PreflightApplyFailed, ApplyError);
		return false;
	}
	if (PreflightProjection.DefinitionHash != Request.ExpectedResolvedDefinitionHash)
	{
		CFVehicleApplyPrivate::SetError(
			OutResult,
			ECFVehicleApplyFailureCode::PreflightHashMismatch,
			FString::Printf(TEXT("Preflight readback hash가 expected resolved hash와 다릅니다. Expected=%s Actual=%s"), *Request.ExpectedResolvedDefinitionHash, *PreflightProjection.DefinitionHash));
		return false;
	}

	// A3 transient candidate의 existing Runtime Definition validation report입니다.
	const FCFVDAValidationReport PreflightValidationReport = UCFVDAValidator::ValidateVehicleData(PreflightCandidate.Get(), nullptr);
	if (CFVehicleApplyPrivate::ConvertValidationReport(PreflightValidationReport, OutResult.DefinitionValidation))
	{
		CFVehicleApplyPrivate::SetBlocked(OutResult, ECFVehicleApplyFailureCode::PreflightValidationFailed, TEXT("Transient preflight Definition Validation이 Error/Blocked를 반환했습니다."));
		return false;
	}

	// A7 transaction rollback에 사용할 full Target backup object입니다.
	TStrongObjectPtr<UCFVehicleData> TargetBackup(DuplicateObject<UCFVehicleData>(Request.TargetVehicleData, GetTransientPackage()));
	if (!TargetBackup.IsValid())
	{
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::InternalError, TEXT("Apply rollback Target backup 생성에 실패했습니다."));
		return false;
	}
	// A7 transaction 전 persistent Recipe AppliedState backup입니다.
	const FCFVehicleAppliedState AppliedStateBackup = Request.Recipe->AppliedState;
	// Apply 전 Target package dirty 상태입니다.
	const bool bTargetPackageWasDirty = Request.TargetVehicleData->GetOutermost() && Request.TargetVehicleData->GetOutermost()->IsDirty();
	// Apply 전 Recipe package dirty 상태입니다.
	const bool bRecipePackageWasDirty = Request.Recipe->GetOutermost() && Request.Recipe->GetOutermost()->IsDirty();
	// A11 success 시 기록할 new AppliedState를 transaction 시작 전에 완전히 검증해 둡니다.
	FCFVehicleAppliedState NewAppliedState;
	if (!CFVehicleApplyPrivate::BuildAppliedState(*Request.Recipe, FreshState, NewAppliedState, ApplyError))
	{
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::InternalError, ApplyError);
		return false;
	}

	// A4 Target + Recipe AppliedState를 하나로 묶는 Editor transaction입니다.
	FScopedTransaction ApplyTransaction(NSLOCTEXT("CarFightDataAuthoring", "ApplyResolvedVehicleDefinition", "해석된 차량 Definition 적용"));
	// A5 Target object transaction snapshot 등록입니다.
	Request.TargetVehicleData->Modify();
	// A6 Recipe object transaction snapshot 등록입니다.
	Request.Recipe->Modify();

	// A8 reviewed dependency-safe exact Field Diff를 actual Target에 적용합니다.
	if (!CFVehicleApplyPrivate::ApplyPlanToVehicleData(*Request.TargetVehicleData, ApplyPlan, ApplyError))
	{
		// Failed actual mutation rollback 이유입니다.
		FString RollbackError;
		if (!CFVehicleApplyPrivate::RollbackTransaction(*Request.TargetVehicleData, *Request.Recipe, *TargetBackup, AppliedStateBackup, FreshState.TargetSnapshot, bTargetPackageWasDirty, bRecipePackageWasDirty, ApplyTransaction, RollbackError))
		{
			CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::RollbackFailed, RollbackError);
			return false;
		}
		OutResult.bRollbackVerified = true;
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::TargetApplyFailed, ApplyError);
		return false;
	}

	if (bInjectFailureAfterTargetMutation)
	{
		// Automation probe가 A8 이후 rollback branch를 실제 실행시키기 위한 injected failure입니다.
		const FString InjectedError = TEXT("Automation injected failure after Target mutation.");
		// Injected failure rollback 이유입니다.
		FString RollbackError;
		if (!CFVehicleApplyPrivate::RollbackTransaction(*Request.TargetVehicleData, *Request.Recipe, *TargetBackup, AppliedStateBackup, FreshState.TargetSnapshot, bTargetPackageWasDirty, bRecipePackageWasDirty, ApplyTransaction, RollbackError))
		{
			CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::RollbackFailed, RollbackError);
			return false;
		}
		OutResult.bRollbackVerified = true;
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::TargetApplyFailed, InjectedError);
		return false;
	}

	// A9 actual Target Resolver-owned readback projection입니다.
	FCFVehicleDefinitionSnapshot TargetProjection;
	if (!CFVehicleApplyPrivate::BuildResolvedProjection(*Request.TargetVehicleData, FreshState.ResolveResult.SortedResolvedFields, TargetProjection, ApplyError)
		|| TargetProjection.DefinitionHash != Request.ExpectedResolvedDefinitionHash)
	{
		if (ApplyError.IsEmpty())
		{
			ApplyError = FString::Printf(TEXT("Target readback hash mismatch. Expected=%s Actual=%s"), *Request.ExpectedResolvedDefinitionHash, *TargetProjection.DefinitionHash);
		}
		// Readback mismatch rollback 이유입니다.
		FString RollbackError;
		if (!CFVehicleApplyPrivate::RollbackTransaction(*Request.TargetVehicleData, *Request.Recipe, *TargetBackup, AppliedStateBackup, FreshState.TargetSnapshot, bTargetPackageWasDirty, bRecipePackageWasDirty, ApplyTransaction, RollbackError))
		{
			CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::RollbackFailed, RollbackError);
			return false;
		}
		OutResult.bRollbackVerified = true;
		CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::TargetReadbackMismatch, ApplyError);
		return false;
	}

	// A10 actual Target의 existing Runtime Definition validation report입니다.
	const FCFVDAValidationReport TargetValidationReport = UCFVDAValidator::ValidateVehicleData(Request.TargetVehicleData, nullptr);
	if (CFVehicleApplyPrivate::ConvertValidationReport(TargetValidationReport, OutResult.DefinitionValidation))
	{
		// Actual Target validation failure rollback 이유입니다.
		FString RollbackError;
		if (!CFVehicleApplyPrivate::RollbackTransaction(*Request.TargetVehicleData, *Request.Recipe, *TargetBackup, AppliedStateBackup, FreshState.TargetSnapshot, bTargetPackageWasDirty, bRecipePackageWasDirty, ApplyTransaction, RollbackError))
		{
			CFVehicleApplyPrivate::SetError(OutResult, ECFVehicleApplyFailureCode::RollbackFailed, RollbackError);
			return false;
		}
		OutResult.bRollbackVerified = true;
		CFVehicleApplyPrivate::SetBlocked(OutResult, ECFVehicleApplyFailureCode::TargetValidationFailed, TEXT("Actual Target Definition Validation이 Error/Blocked를 반환해 transaction을 rollback했습니다."));
		return false;
	}

	// A11 fresh Resolve provenance를 persistent Recipe AppliedState authority로 갱신합니다.
	Request.Recipe->AppliedState = MoveTemp(NewAppliedState);
	// A12 Target package를 dirty로 표시하되 저장하지 않습니다.
	Request.TargetVehicleData->MarkPackageDirty();
	// A12 Recipe package를 dirty로 표시하되 저장하지 않습니다.
	Request.Recipe->MarkPackageDirty();
	// A13 property editor/cache refresh notification입니다.
	Request.TargetVehicleData->PostEditChange();
	// A13 Recipe details/cache refresh notification입니다.
	Request.Recipe->PostEditChange();

	OutResult.Status = ECFVehicleApplyStatus::Success;
	OutResult.FailureCode = ECFVehicleApplyFailureCode::None;
	OutResult.Message = TEXT("Reviewed Vehicle Definition Apply가 atomic transaction으로 완료됐습니다. Package는 저장하지 않았습니다.");
	OutResult.AppliedDefinitionHash = TargetProjection.DefinitionHash;
	OutResult.bTargetMutationCommitted = true;
	OutResult.bRecipeAppliedStateUpdated = true;
	OutResult.bRollbackVerified = false;
	return true;
}
