// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAHealthService.cpp
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-02C inventory-bound explicit lazy load + complete namespace duplicate 분석 구현입니다.
// Changelog:
// - v1.1.0: generation/row provenance를 Inventory snapshot으로 결합하고 complete duplicate closure + typed canonical equality를 추가.
// - v1.0.0: Registered semantic만 load하고 generation-bound typed health + deterministic duplicate marking을 구현.
// Migration:
// - Product Asset을 수정/저장하지 않으며 Reference/Referencer와 Manager UI를 열지 않습니다.

#include "DataManagement/CFDAHealthService.h"
#include "DataManagement/CFDATypeAdapter.h"
#include "DataManagement/CFDATypeRegistry.h"

#include "UObject/SoftObjectPath.h"

namespace CFDAHealthServicePrivate
{
	constexpr int32 MaxDiagnosticMessages = 64;

	void AddMessage(
		const FString& Message,
		FCFDALoadedAssetResult& InOutResult)
	{
		if (!Message.IsEmpty() && InOutResult.Messages.Num() < MaxDiagnosticMessages)
		{
			InOutResult.Messages.Add(Message);
		}
	}

	const FCFDAAssetRecord* FindAssetRecord(
		const FCFDAInventoryResult& Inventory,
		const FString& ObjectPath)
	{
		for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
		{
			if (AssetRecord.ObjectPath == ObjectPath)
			{
				return &AssetRecord;
			}
		}

		return nullptr;
	}

	FCFDALoadedAssetResult MakeInvalidRequestResult(
		const FCFDAInventoryResult& Inventory,
		const FString& ObjectPath,
		const FString& Message)
	{
		FCFDALoadedAssetResult Result;
		Result.InventoryGeneration = Inventory.InventoryGeneration;
		Result.ObjectPath = ObjectPath;
		Result.EvaluationState = ECFDAEvaluationState::InvalidRequest;
		Result.HealthState = ECFDAHealthState::NotValidated;
		Result.StableIdState = ECFDAStableIdState::NotResolved;
		Result.DuplicateState = ECFDADuplicateState::NotAnalyzed;
		AddMessage(Message, Result);
		return Result;
	}

	FCFDALoadedAssetResult EvaluateRecord(
		const FCFDAAssetRecord& AssetRecord,
		const FCFDATypeRegistry& TypeRegistry,
		const uint64 InventoryGeneration)
	{
		const FCFDAResolvedSemantic Semantic =
			TypeRegistry.ResolveSemantic(AssetRecord.ClassPath, AssetRecord.ClassPath);
		if (!Semantic.HasAuthoritativePolicy())
		{
			return FCFDATypeAdapter::EvaluateLoadedAsset(
				AssetRecord,
				nullptr,
				TypeRegistry,
				InventoryGeneration);
		}

		if (AssetRecord.ObjectPath.IsEmpty())
		{
			FCFDALoadedAssetResult Result;
			Result.InventoryGeneration = InventoryGeneration;
			Result.ObjectPath = AssetRecord.ObjectPath;
			Result.ClassPath = AssetRecord.ClassPath;
			Result.CoverageState = AssetRecord.CoverageState;
			Result.EvaluationState = ECFDAEvaluationState::InvalidRequest;
			Result.HealthState = ECFDAHealthState::NotValidated;
			Result.StableIdState = ECFDAStableIdState::NotResolved;
			Result.DuplicateState = ECFDADuplicateState::NotAnalyzed;
			AddMessage(TEXT("Inventory AssetRecord의 ObjectPath가 비어 있어 loaded validation을 실행할 수 없습니다."), Result);
			return Result;
		}

		const FSoftObjectPath ObjectPath(AssetRecord.ObjectPath);

		// DAM-P0-02C DataManagement의 유일한 actual UObject lazy load 지점입니다.
		UObject* LoadedObject = ObjectPath.TryLoad();

		return FCFDATypeAdapter::EvaluateLoadedAsset(
			AssetRecord,
			LoadedObject,
			TypeRegistry,
			InventoryGeneration);
	}

	bool IsPrimaryAssetDuplicateCandidate(
		const FCFDAAssetRecord& AssetRecord,
		const FCFDATypeRegistry& TypeRegistry)
	{
		const FCFDAResolvedSemantic Semantic =
			TypeRegistry.ResolveSemantic(AssetRecord.ClassPath, AssetRecord.ClassPath);
		return Semantic.HasAuthoritativePolicy()
			&& FCFDATypeRegistry::ResolveDuplicateNamespacePolicy(Semantic.Descriptor)
				== ECFDADuplicateNamespacePolicy::PrimaryAssetType;
	}

	void AddDuplicateClosureForRecord(
		const FCFDAInventoryResult& Inventory,
		const FCFDAAssetRecord& RequestedRecord,
		const FCFDATypeRegistry& TypeRegistry,
		TSet<FString>& InOutClosureObjectPaths)
	{
		InOutClosureObjectPaths.Add(RequestedRecord.ObjectPath);

		const FCFDAResolvedSemantic Semantic =
			TypeRegistry.ResolveSemantic(RequestedRecord.ClassPath, RequestedRecord.ClassPath);
		if (!Semantic.HasAuthoritativePolicy()
			|| Semantic.Descriptor.IdentityPolicy == ECFDAIdentityPolicy::NotApplicable)
		{
			return;
		}

		const ECFDADuplicateNamespacePolicy NamespacePolicy =
			FCFDATypeRegistry::ResolveDuplicateNamespacePolicy(Semantic.Descriptor);

		if (NamespacePolicy == ECFDADuplicateNamespacePolicy::ExactClassPath)
		{
			for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
			{
				if (AssetRecord.ClassPath == RequestedRecord.ClassPath)
				{
					InOutClosureObjectPaths.Add(AssetRecord.ObjectPath);
				}
			}
			return;
		}

		if (NamespacePolicy == ECFDADuplicateNamespacePolicy::PrimaryAssetType)
		{
			// PrimaryAssetType은 UObject resolve 뒤에 확정되므로, registered PrimaryAssetId 후보 전체를 bounded closure로 평가합니다.
			for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
			{
				if (IsPrimaryAssetDuplicateCandidate(AssetRecord, TypeRegistry))
				{
					InOutClosureObjectPaths.Add(AssetRecord.ObjectPath);
				}
			}
		}
	}

	const FCFDALoadedAssetResult* FindLoadedResult(
		const TArray<FCFDALoadedAssetResult>& Results,
		const FString& ObjectPath)
	{
		for (const FCFDALoadedAssetResult& Result : Results)
		{
			if (Result.ObjectPath == ObjectPath)
			{
				return &Result;
			}
		}

		return nullptr;
	}

	bool IsDuplicateCoverageCompleteForResult(
		const FCFDAInventoryResult& Inventory,
		const FCFDALoadedAssetResult& TargetResult,
		const FCFDATypeRegistry& TypeRegistry,
		const TArray<FCFDALoadedAssetResult>& ClosureResults)
	{
		if (TargetResult.EvaluationState != ECFDAEvaluationState::Succeeded
			|| TargetResult.StableIdState != ECFDAStableIdState::Resolved
			|| !TargetResult.CanonicalStableIdentity.IsValid())
		{
			return false;
		}

		const FCFDAResolvedSemantic Semantic =
			TypeRegistry.ResolveSemantic(TargetResult.ClassPath, TargetResult.ClassPath);
		if (!Semantic.HasAuthoritativePolicy())
		{
			return false;
		}

		const ECFDADuplicateNamespacePolicy NamespacePolicy =
			FCFDATypeRegistry::ResolveDuplicateNamespacePolicy(Semantic.Descriptor);

		for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
		{
			bool bPotentialCandidate = false;
			if (NamespacePolicy == ECFDADuplicateNamespacePolicy::ExactClassPath)
			{
				bPotentialCandidate = AssetRecord.ClassPath == TargetResult.ClassPath;
			}
			else if (NamespacePolicy == ECFDADuplicateNamespacePolicy::PrimaryAssetType)
			{
				bPotentialCandidate = IsPrimaryAssetDuplicateCandidate(AssetRecord, TypeRegistry);
			}

			if (!bPotentialCandidate)
			{
				continue;
			}

			const FCFDALoadedAssetResult* CandidateResult =
				FindLoadedResult(ClosureResults, AssetRecord.ObjectPath);
			if (!CandidateResult
				|| CandidateResult->EvaluationState != ECFDAEvaluationState::Succeeded)
			{
				// 운영/adapter 실패 후보가 하나라도 있으면 그 후보의 실제 ID를 모르므로 Unique를 결론내리지 않습니다.
				return false;
			}
		}

		return true;
	}
}

// current Inventory snapshot의 exact object path 한 건을 explicit validate합니다.
FCFDALoadedAssetResult FCFDAHealthService::ValidateAsset(
	const FCFDAInventoryResult& Inventory,
	const FString& ObjectPath,
	const FCFDATypeRegistry& TypeRegistry)
{
	const TArray<FCFDALoadedAssetResult> Results =
		ValidateAssets(
			Inventory,
			TArray<FString>{ObjectPath},
			TypeRegistry);

	if (Results.Num() == 1)
	{
		return Results[0];
	}

	return CFDAHealthServicePrivate::MakeInvalidRequestResult(
		Inventory,
		ObjectPath,
		TEXT("요청한 Asset의 loaded validation 결과를 만들지 못했습니다."));
}

// current Inventory snapshot에서 요청된 object path의 결과만 반환하고 duplicate namespace 후보는 내부적으로 bounded 확장합니다.
TArray<FCFDALoadedAssetResult> FCFDAHealthService::ValidateAssets(
	const FCFDAInventoryResult& Inventory,
	const TArray<FString>& ObjectPaths,
	const FCFDATypeRegistry& TypeRegistry)
{
	TArray<FString> RequestedObjectPaths = ObjectPaths;
	RequestedObjectPaths.Sort([](const FString& Left, const FString& Right)
	{
		return Left.Compare(Right, ESearchCase::CaseSensitive) < 0;
	});

	TSet<FString> SeenRequestedObjectPaths;
	TArray<FString> UniqueRequestedObjectPaths;
	UniqueRequestedObjectPaths.Reserve(RequestedObjectPaths.Num());
	for (const FString& ObjectPath : RequestedObjectPaths)
	{
		if (!SeenRequestedObjectPaths.Contains(ObjectPath))
		{
			SeenRequestedObjectPaths.Add(ObjectPath);
			UniqueRequestedObjectPaths.Add(ObjectPath);
		}
	}

	TArray<FCFDALoadedAssetResult> ImmediateInvalidResults;
	TSet<FString> ClosureObjectPaths;

	if (Inventory.InventoryGeneration == 0 || !Inventory.IsInventoryComplete())
	{
		for (const FString& ObjectPath : UniqueRequestedObjectPaths)
		{
			ImmediateInvalidResults.Add(
				CFDAHealthServicePrivate::MakeInvalidRequestResult(
					Inventory,
					ObjectPath,
					TEXT("Loaded validation에는 complete inventory와 0이 아닌 InventoryGeneration이 필요합니다.")));
		}
		return ImmediateInvalidResults;
	}

	for (const FString& ObjectPath : UniqueRequestedObjectPaths)
	{
		const FCFDAAssetRecord* RequestedRecord =
			CFDAHealthServicePrivate::FindAssetRecord(Inventory, ObjectPath);
		if (!RequestedRecord)
		{
			ImmediateInvalidResults.Add(
				CFDAHealthServicePrivate::MakeInvalidRequestResult(
					Inventory,
					ObjectPath,
					TEXT("요청한 ObjectPath가 현재 Inventory snapshot에 존재하지 않습니다.")));
			continue;
		}

		CFDAHealthServicePrivate::AddDuplicateClosureForRecord(
			Inventory,
			*RequestedRecord,
			TypeRegistry,
			ClosureObjectPaths);
	}

	TArray<FString> SortedClosureObjectPaths = ClosureObjectPaths.Array();
	SortedClosureObjectPaths.Sort([](const FString& Left, const FString& Right)
	{
		return Left.Compare(Right, ESearchCase::CaseSensitive) < 0;
	});

	TArray<FCFDALoadedAssetResult> ClosureResults;
	ClosureResults.Reserve(SortedClosureObjectPaths.Num());
	for (const FString& ClosureObjectPath : SortedClosureObjectPaths)
	{
		const FCFDAAssetRecord* AssetRecord =
			CFDAHealthServicePrivate::FindAssetRecord(Inventory, ClosureObjectPath);
		if (!AssetRecord)
		{
			continue;
		}

		ClosureResults.Add(
			CFDAHealthServicePrivate::EvaluateRecord(
				*AssetRecord,
				TypeRegistry,
				Inventory.InventoryGeneration));
	}

	for (FCFDALoadedAssetResult& Result : ClosureResults)
	{
		if (Result.StableIdState == ECFDAStableIdState::NotApplicable)
		{
			Result.DuplicateState = ECFDADuplicateState::NotApplicable;
			continue;
		}

		Result.bDuplicateNamespaceCoverageComplete =
			CFDAHealthServicePrivate::IsDuplicateCoverageCompleteForResult(
				Inventory,
				Result,
				TypeRegistry,
				ClosureResults);
	}

	AnalyzeDuplicateStableIds(ClosureResults);

	TArray<FCFDALoadedAssetResult> RequestedResults = MoveTemp(ImmediateInvalidResults);
	RequestedResults.Reserve(RequestedResults.Num() + UniqueRequestedObjectPaths.Num());
	for (const FString& ObjectPath : UniqueRequestedObjectPaths)
	{
		if (const FCFDALoadedAssetResult* LoadedResult =
			CFDAHealthServicePrivate::FindLoadedResult(ClosureResults, ObjectPath))
		{
			RequestedResults.Add(*LoadedResult);
		}
	}

	RequestedResults.Sort([](
		const FCFDALoadedAssetResult& Left,
		const FCFDALoadedAssetResult& Right)
	{
		return Left.ObjectPath.Compare(Right.ObjectPath, ESearchCase::CaseSensitive) < 0;
	});
	return RequestedResults;
}

// complete namespace coverage 표시가 있는 loaded 결과 집합에서 typed/canonical duplicate를 pure하게 표시합니다.
void FCFDAHealthService::AnalyzeDuplicateStableIds(
	TArray<FCFDALoadedAssetResult>& InOutResults)
{
	for (FCFDALoadedAssetResult& Result : InOutResults)
	{
		if (Result.StableIdState == ECFDAStableIdState::NotApplicable)
		{
			Result.DuplicateState = ECFDADuplicateState::NotApplicable;
		}
		else
		{
			Result.DuplicateState = ECFDADuplicateState::NotAnalyzed;
		}
	}

	for (int32 LeftIndex = 0; LeftIndex < InOutResults.Num(); ++LeftIndex)
	{
		FCFDALoadedAssetResult& Left = InOutResults[LeftIndex];
		if (Left.InventoryGeneration == 0
			|| Left.EvaluationState != ECFDAEvaluationState::Succeeded
			|| Left.StableIdState != ECFDAStableIdState::Resolved
			|| !Left.CanonicalStableIdentity.IsValid())
		{
			continue;
		}

		for (int32 RightIndex = LeftIndex + 1; RightIndex < InOutResults.Num(); ++RightIndex)
		{
			FCFDALoadedAssetResult& Right = InOutResults[RightIndex];
			if (Left.InventoryGeneration != Right.InventoryGeneration
				|| Right.EvaluationState != ECFDAEvaluationState::Succeeded
				|| Right.StableIdState != ECFDAStableIdState::Resolved
				|| !Right.CanonicalStableIdentity.IsValid()
				|| !Left.CanonicalStableIdentity.Equals(Right.CanonicalStableIdentity))
			{
				continue;
			}

			Left.DuplicateState = ECFDADuplicateState::Duplicate;
			Right.DuplicateState = ECFDADuplicateState::Duplicate;
			Left.HealthState = ECFDAHealthState::Error;
			Right.HealthState = ECFDAHealthState::Error;

			CFDAHealthServicePrivate::AddMessage(
				FString::Printf(
					TEXT("동일 namespace '%s' 안에 Stable ID '%s'가 중복되어 있습니다."),
					*Left.DuplicateNamespace,
					*Left.StableId),
				Left);
			CFDAHealthServicePrivate::AddMessage(
				FString::Printf(
					TEXT("동일 namespace '%s' 안에 Stable ID '%s'가 중복되어 있습니다."),
					*Right.DuplicateNamespace,
					*Right.StableId),
				Right);
		}
	}

	for (FCFDALoadedAssetResult& Result : InOutResults)
	{
		if (Result.DuplicateState == ECFDADuplicateState::Duplicate
			|| Result.DuplicateState == ECFDADuplicateState::NotApplicable)
		{
			continue;
		}

		if (Result.EvaluationState == ECFDAEvaluationState::Succeeded
			&& Result.StableIdState == ECFDAStableIdState::Resolved
			&& Result.CanonicalStableIdentity.IsValid()
			&& Result.bDuplicateNamespaceCoverageComplete)
		{
			Result.DuplicateState = ECFDADuplicateState::Unique;
		}
	}
}
