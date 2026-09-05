// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderHardpointIntegrity.cpp
// Version: v1.3.1
// Date: 2026-09-04
// Description: CF-FQ-047 Socket/Hardpoint/Mount integrity + typed Physics receipt compatibility/drift boundary 구현입니다.
// Changelog:
// - v1.3.1: Step 5 승인 직후 Step 7 Apply 대기 상태를 정상 Exact로 유지하도록 ReceiptResolvedDefinitionHash == CurrentProspectiveResolvedDefinitionHash를 Exact 기준으로 사용합니다.
// - v1.3.0: Hardpoint/Mount-only structural drift를 single typed evaluator로 분류하고 Step 5 Physics receipt compatibility를 Exact/Equivalent/Stale/Blocked로 projection.
// Migration:
// - v1.3.1부터 Current Target hash 차이는 Step 7 Apply Pending일 수 있으므로 Exact 판정에서 요구하지 않습니다. Receipt != current prospective일 때만 structural drift를 평가합니다.
// - 기존 ValidatePhysicsReceiptRefreshBoundary 호출자는 그대로 유지되며 내부에서 typed structural boundary의 Equivalent만 true로 projection합니다.

#include "DataAuthoring/CFVehicleBuilderHardpointIntegrity.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"

int32 FCFBuilderChassisSocketInventory::CountByClassification(
	const ECFBuilderHardpointSocketClassification Classification) const
{
	int32 Count = 0;
	for (const FCFBuilderChassisSocketInventoryEntry& Entry : Entries)
	{
		if (Entry.Classification == Classification)
		{
			++Count;
		}
	}
	return Count;
}

const FCFBuilderChassisSocketInventoryEntry* FCFBuilderChassisSocketInventory::FindBySocketName(
	const FName SocketName) const
{
	return Entries.FindByPredicate([SocketName](const FCFBuilderChassisSocketInventoryEntry& Entry)
	{
		return Entry.SocketName == SocketName;
	});
}

bool FCFVehicleBuilderHardpointIntegrity::TryParseCanonicalStandardSocketName(
	const FName SocketName,
	const TArray<FName>& StandardCategories,
	FName& OutLocationCategory,
	FName& OutLocationSlotId,
	int32& OutIndex)
{
	OutLocationCategory = NAME_None;
	OutLocationSlotId = NAME_None;
	OutIndex = 0;

	if (SocketName.IsNone())
	{
		return false;
	}

	const FString SocketText = SocketName.ToString();
	const FString HardpointPrefix = TEXT("HP_");
	if (!SocketText.StartsWith(HardpointPrefix, ESearchCase::CaseSensitive))
	{
		return false;
	}

	const FString LocationText = SocketText.Mid(HardpointPrefix.Len());
	for (const FName Category : StandardCategories)
	{
		if (Category.IsNone())
		{
			continue;
		}

		const FString CategoryText = Category.ToString();
		const FString CategoryPrefix = CategoryText + TEXT("_");
		if (!LocationText.StartsWith(CategoryPrefix, ESearchCase::CaseSensitive))
		{
			continue;
		}

		const FString IndexText = LocationText.Mid(CategoryPrefix.Len());
		if (IndexText.IsEmpty() || !IndexText.IsNumeric())
		{
			return false;
		}

		const int32 ParsedIndex = FCString::Atoi(*IndexText);
		if (ParsedIndex <= 0)
		{
			return false;
		}

		const FString CanonicalLocationText = FString::Printf(TEXT("%s_%02d"), *CategoryText, ParsedIndex);
		if (!LocationText.Equals(CanonicalLocationText, ESearchCase::CaseSensitive))
		{
			return false;
		}

		OutLocationCategory = Category;
		OutLocationSlotId = FName(*CanonicalLocationText);
		OutIndex = ParsedIndex;
		return true;
	}

	return false;
}

bool FCFVehicleBuilderHardpointIntegrity::ReadChassisSocketInventory(
	const UStaticMesh* ChassisMesh,
	const UCFVehicleRecipeData* Recipe,
	const UCFVehicleData* TargetVehicleData,
	const TArray<FName>& StandardCategories,
	FCFBuilderChassisSocketInventory& OutInventory,
	FString& OutError)
{
	OutInventory = FCFBuilderChassisSocketInventory();

	if (!ChassisMesh)
	{
		OutError = TEXT("Hardpoint Socket inventory를 읽을 Chassis StaticMesh가 없습니다.");
		return false;
	}
	if (!Recipe)
	{
		OutError = TEXT("Hardpoint Socket inventory를 비교할 current Recipe가 없습니다.");
		return false;
	}

	OutInventory.ChassisMeshPath = FSoftObjectPath(ChassisMesh);
	OutInventory.Entries.Reserve(ChassisMesh->Sockets.Num());

	for (const TObjectPtr<UStaticMeshSocket>& SocketObject : ChassisMesh->Sockets)
	{
		const UStaticMeshSocket* Socket = SocketObject.Get();
		if (!Socket || Socket->SocketName.IsNone())
		{
			continue;
		}

		FCFBuilderChassisSocketInventoryEntry Entry;
		Entry.SocketName = Socket->SocketName;
		Entry.RelativeLocation = Socket->RelativeLocation;
		Entry.RelativeRotation = Socket->RelativeRotation;
		Entry.RelativeScale = Socket->RelativeScale;

		const FString SocketText = Entry.SocketName.ToString();
		if (!SocketText.StartsWith(TEXT("HP_"), ESearchCase::CaseSensitive))
		{
			Entry.Classification = ECFBuilderHardpointSocketClassification::UnrelatedSocket;
			Entry.Diagnostic = TEXT("Hardpoint prefix가 아닌 일반 Chassis Socket입니다.");
			OutInventory.Entries.Add(MoveTemp(Entry));
			continue;
		}

		if (!TryParseCanonicalStandardSocketName(
			Entry.SocketName,
			StandardCategories,
			Entry.ParsedLocationCategory,
			Entry.ParsedLocationSlotId,
			Entry.ParsedStandardIndex))
		{
			Entry.Classification = ECFBuilderHardpointSocketClassification::NonCanonicalHardpointLike;
			Entry.Diagnostic = TEXT("HP_ prefix는 있지만 Standard Guided identity 규칙과 exact 일치하지 않습니다.");
			OutInventory.Entries.Add(MoveTemp(Entry));
			continue;
		}

		const FCFHardpointIntent* RecipeSocketBinding = Recipe->HardpointIntents.FindByPredicate(
			[SocketName = Entry.SocketName](const FCFHardpointIntent& Intent)
			{
				return Intent.SocketName == SocketName;
			});
		if (RecipeSocketBinding)
		{
			Entry.Classification = ECFBuilderHardpointSocketClassification::AlreadyRecipeBound;
			Entry.Diagnostic = FString::Printf(
				TEXT("Recipe가 이미 이 Socket을 사용합니다: %s"),
				*RecipeSocketBinding->LocationSlotId.ToString());
			OutInventory.Entries.Add(MoveTemp(Entry));
			continue;
		}

		const FCFVehicleHardpointSlot* TargetSocketBinding = TargetVehicleData
			? TargetVehicleData->HardpointSlots.FindByPredicate(
				[SocketName = Entry.SocketName](const FCFVehicleHardpointSlot& Slot)
				{
					return Slot.SocketName == SocketName;
				})
			: nullptr;
		if (TargetSocketBinding)
		{
			Entry.Classification = ECFBuilderHardpointSocketClassification::AlreadyTargetBound;
			Entry.Diagnostic = FString::Printf(
				TEXT("Target VehicleData가 이미 이 Socket을 사용합니다: %s"),
				*TargetSocketBinding->LocationSlotId.ToString());
			OutInventory.Entries.Add(MoveTemp(Entry));
			continue;
		}

		const FCFHardpointIntent* RecipeIdentityCollision = Recipe->HardpointIntents.FindByPredicate(
			[LocationSlotId = Entry.ParsedLocationSlotId](const FCFHardpointIntent& Intent)
			{
				return Intent.LocationSlotId == LocationSlotId;
			});
		const FCFVehicleHardpointSlot* TargetIdentityCollision = TargetVehicleData
			? TargetVehicleData->HardpointSlots.FindByPredicate(
				[LocationSlotId = Entry.ParsedLocationSlotId](const FCFVehicleHardpointSlot& Slot)
				{
					return Slot.LocationSlotId == LocationSlotId;
				})
			: nullptr;

		if (RecipeIdentityCollision || TargetIdentityCollision)
		{
			Entry.Classification = ECFBuilderHardpointSocketClassification::IdentityCollision;
			const FName ExistingSocketName = RecipeIdentityCollision
				? RecipeIdentityCollision->SocketName
				: TargetIdentityCollision->SocketName;
			Entry.Diagnostic = FString::Printf(
				TEXT("LocationSlotId %s가 이미 다른 binding을 소유합니다: %s"),
				*Entry.ParsedLocationSlotId.ToString(),
				ExistingSocketName.IsNone() ? TEXT("<SocketName 없음>") : *ExistingSocketName.ToString());
			OutInventory.Entries.Add(MoveTemp(Entry));
			continue;
		}

		Entry.Classification = ECFBuilderHardpointSocketClassification::StandardAdoptable;
		Entry.Diagnostic = FString::Printf(
			TEXT("현재 차량 semantic에 연결되지 않은 canonical Standard Socket입니다: %s"),
			*Entry.ParsedLocationSlotId.ToString());
		OutInventory.Entries.Add(MoveTemp(Entry));
	}

	OutInventory.Entries.Sort([](
		const FCFBuilderChassisSocketInventoryEntry& Left,
		const FCFBuilderChassisSocketInventoryEntry& Right)
	{
		return Left.SocketName.LexicalLess(Right.SocketName);
	});

	OutError.Reset();
	return true;
}

bool FCFVehicleBuilderHardpointIntegrity::BuildHardpointSemanticFacts(
	const UCFVehicleRecipeData* Recipe,
	const UCFVehicleData* TargetVehicleData,
	const FCFBuilderFinalReviewResult* FinalReview,
	FCFBuilderHardpointSemanticFacts& OutFacts,
	FString& OutError)
{
	OutFacts = FCFBuilderHardpointSemanticFacts();
	if (!Recipe)
	{
		OutError = TEXT("Hardpoint/Mount semantic readback에 current Recipe가 없습니다.");
		return false;
	}

	OutFacts.AuthoredHardpointCount = Recipe->HardpointIntents.Num();
	OutFacts.AuthoredMountCount = Recipe->MountIntents.Num();
	OutFacts.AuthoredRelations.Reserve(Recipe->HardpointIntents.Num());
	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		FCFBuilderHardpointMountRelationFact Relation;
		Relation.LocationSlotId = HardpointIntent.LocationSlotId;
		Relation.SocketName = HardpointIntent.SocketName;
		for (const FCFMountIntent& MountIntent : Recipe->MountIntents)
		{
			if (MountIntent.LocationSlotRef == HardpointIntent.LocationSlotId)
			{
				Relation.MountProfileIds.Add(MountIntent.MountProfileId);
			}
		}
		Relation.MountProfileIds.Sort(FNameLexicalLess());
		OutFacts.AuthoredRelations.Add(MoveTemp(Relation));
	}
	OutFacts.AuthoredRelations.Sort([](
		const FCFBuilderHardpointMountRelationFact& Left,
		const FCFBuilderHardpointMountRelationFact& Right)
	{
		return Left.LocationSlotId.LexicalLess(Right.LocationSlotId);
	});

	if (TargetVehicleData)
	{
		OutFacts.bHasCurrentTarget = true;
		OutFacts.CurrentTargetHardpointCount = TargetVehicleData->HardpointSlots.Num();
		OutFacts.CurrentTargetMountCount = TargetVehicleData->MountProfiles.Num();
	}

	if (OutFacts.bHasCurrentTarget
		&& FinalReview
		&& FinalReview->Operation.Status == ECFAuthoringOpStatus::Succeeded)
	{
		int32 ProspectiveHardpointCount = OutFacts.CurrentTargetHardpointCount;
		int32 ProspectiveMountCount = OutFacts.CurrentTargetMountCount;
		TSet<FString> CountedStructuralRows;

		for (const FCFVehicleFieldDiff& Diff : FinalReview->FieldDiff)
		{
			if (Diff.Operation != ECFVehicleDiffOp::AddArrayElement
				&& Diff.Operation != ECFVehicleDiffOp::RemoveArrayElement)
			{
				continue;
			}

			const bool bHardpointCollection = Diff.FieldPath.CollectionPropertyName == TEXT("HardpointSlots");
			const bool bMountCollection = Diff.FieldPath.CollectionPropertyName == TEXT("MountProfiles");
			if (!bHardpointCollection && !bMountCollection)
			{
				continue;
			}

			const FString StructuralKey = Diff.FieldPath.ToCanonicalString();
			if (StructuralKey.IsEmpty() || CountedStructuralRows.Contains(StructuralKey))
			{
				continue;
			}
			CountedStructuralRows.Add(StructuralKey);

			const int32 Delta = Diff.Operation == ECFVehicleDiffOp::AddArrayElement ? 1 : -1;
			if (bHardpointCollection)
			{
				ProspectiveHardpointCount += Delta;
			}
			else
			{
				ProspectiveMountCount += Delta;
			}
		}

		if (ProspectiveHardpointCount >= 0 && ProspectiveMountCount >= 0)
		{
			OutFacts.bHasProspectiveTargetCounts = true;
			OutFacts.ProspectiveTargetHardpointCount = ProspectiveHardpointCount;
			OutFacts.ProspectiveTargetMountCount = ProspectiveMountCount;
		}
		else
		{
			OutFacts.Diagnostic = TEXT("Final Review structural diff에서 prospective Hardpoint/Mount count가 음수가 되어 summary를 표시하지 않습니다.");
		}
	}

	OutFacts.bAvailable = true;
	OutError.Reset();
	return true;
}

// Receipt baseline/current Target/prospective Definition/pending diff를 읽어 Hardpoint/Mount-only structural equivalence를 단일 typed owner에서 판정합니다.
FCFBuilderPhysicsStructuralDriftBoundaryResult FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsStructuralDriftBoundary(
	const FString& ReceiptResolvedDefinitionHash,
	const FString& CurrentTargetDefinitionHash,
	const FString& CurrentProspectiveResolvedDefinitionHash,
	const TArray<FCFVehicleFieldDiff>& CurrentPendingDiff)
{
	// Fail-closed를 기본값으로 갖는 structural boundary 결과입니다.
	FCFBuilderPhysicsStructuralDriftBoundaryResult Result;

	if (ReceiptResolvedDefinitionHash.IsEmpty()
		|| CurrentTargetDefinitionHash.IsEmpty()
		|| CurrentProspectiveResolvedDefinitionHash.IsEmpty())
	{
		Result.Diagnostic = TEXT("Physics receipt 판정에 필요한 resolved/Target hash가 비어 있습니다.");
		return Result;
	}

	// Receipt가 승인했던 baseline이 current Target과 다르면 이미 Target 자체가 drift했으므로 structural-equivalent로 인정하지 않습니다.
	if (ReceiptResolvedDefinitionHash != CurrentTargetDefinitionHash)
	{
		Result.Boundary = ECFBuilderPhysicsStructuralDriftBoundary::NotEquivalent;
		Result.Diagnostic = TEXT("Current Target Definition이 기존 Physics receipt가 승인한 resolved state와 달라 Physics 설정을 다시 확인해야 합니다.");
		return Result;
	}

	// Exact 상태는 compatibility 상위 계층에서 처리하며 structural drift로 분류하지 않습니다.
	if (ReceiptResolvedDefinitionHash == CurrentProspectiveResolvedDefinitionHash)
	{
		Result.Boundary = ECFBuilderPhysicsStructuralDriftBoundary::NotEquivalent;
		Result.Diagnostic = TEXT("Physics receipt가 이미 current prospective Definition과 일치해 재검증이 필요하지 않습니다.");
		return Result;
	}

	if (CurrentPendingDiff.IsEmpty())
	{
		Result.Diagnostic = TEXT("Resolved hash는 달라졌지만 current Target 대비 pending FieldDiff가 없어 Physics 영향 경계를 안전하게 증명할 수 없습니다.");
		return Result;
	}

	for (const FCFVehicleFieldDiff& Diff : CurrentPendingDiff)
	{
		// Current pending row가 속한 top-level collection입니다.
		const FName CollectionName = Diff.FieldPath.CollectionPropertyName;
		if (CollectionName != TEXT("HardpointSlots") && CollectionName != TEXT("MountProfiles"))
		{
			Result.Boundary = ECFBuilderPhysicsStructuralDriftBoundary::NotEquivalent;
			Result.Diagnostic = FString::Printf(
				TEXT("Hardpoint/Mount 이외의 pending Definition 변경이 있어 기존 Physics 설정을 그대로 인정하지 않습니다: %s"),
				*Diff.FieldPath.ToCanonicalString(true));
			return Result;
		}
	}

	Result.Boundary = ECFBuilderPhysicsStructuralDriftBoundary::Equivalent;
	Result.Diagnostic = TEXT("현재 Target 이후 변경은 HardpointSlots/MountProfiles 구조에만 한정되어 기존 Physics 설정과 동등합니다.");
	return Result;
}

// Fresh Physics provenance 판정과 structural boundary를 결합해 Step 5 receipt compatibility를 mutation0으로 반환합니다.
FCFBuilderPhysicsReceiptCompatibilityResult FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
	const bool bPhysicsProvenanceCurrent,
	const FString& ReceiptResolvedDefinitionHash,
	const FString& CurrentTargetDefinitionHash,
	const FString& CurrentProspectiveResolvedDefinitionHash,
	const TArray<FCFVehicleFieldDiff>& CurrentPendingDiff,
	const FString& ProvenanceDiagnostic)
{
	// Fail-closed를 기본값으로 갖는 Step 5 compatibility 결과입니다.
	FCFBuilderPhysicsReceiptCompatibilityResult Result;

	if (!bPhysicsProvenanceCurrent)
	{
		Result.Compatibility = ECFBuilderPhysicsReceiptCompatibility::PhysicsStale;
		Result.Diagnostic = ProvenanceDiagnostic.IsEmpty()
			? TEXT("Physics receipt의 Evidence/Profile/Transmission/Engine/Resolver provenance가 current truth와 달라 다시 검토해야 합니다.")
			: ProvenanceDiagnostic;
		return Result;
	}

	if (ReceiptResolvedDefinitionHash.IsEmpty()
		|| CurrentTargetDefinitionHash.IsEmpty()
		|| CurrentProspectiveResolvedDefinitionHash.IsEmpty())
	{
		Result.Compatibility = ECFBuilderPhysicsReceiptCompatibility::Blocked;
		Result.Diagnostic = TEXT("Physics receipt compatibility를 판정할 Definition identity가 비어 있습니다.");
		return Result;
	}

	// Step 5가 승인한 exact prospective Definition이 그대로 유지되면 Current Target이 아직 다르더라도 Step 7 Apply Pending일 뿐 stale이 아닙니다.
	if (ReceiptResolvedDefinitionHash == CurrentProspectiveResolvedDefinitionHash)
	{
		Result.Compatibility = ECFBuilderPhysicsReceiptCompatibility::Exact;
		Result.Diagnostic = ReceiptResolvedDefinitionHash == CurrentTargetDefinitionHash
			? TEXT("Physics receipt provenance와 current Target/prospective Definition이 정확히 일치합니다.")
			: TEXT("승인된 Physics Proposal이 current prospective Definition과 정확히 일치하며 Target 적용만 남아 있습니다.");
		return Result;
	}

	// 승인된 prospective identity 자체가 달라진 경우에만 single structural boundary owner로 Hardpoint/Mount-only equivalence를 판정합니다.
	const FCFBuilderPhysicsStructuralDriftBoundaryResult StructuralResult = EvaluatePhysicsStructuralDriftBoundary(
		ReceiptResolvedDefinitionHash,
		CurrentTargetDefinitionHash,
		CurrentProspectiveResolvedDefinitionHash,
		CurrentPendingDiff);

	switch (StructuralResult.Boundary)
	{
	case ECFBuilderPhysicsStructuralDriftBoundary::Equivalent:
		Result.Compatibility = ECFBuilderPhysicsReceiptCompatibility::PhysicsEquivalentStructuralDrift;
		Result.Diagnostic = StructuralResult.Diagnostic;
		break;
	case ECFBuilderPhysicsStructuralDriftBoundary::NotEquivalent:
		Result.Compatibility = ECFBuilderPhysicsReceiptCompatibility::PhysicsStale;
		Result.Diagnostic = StructuralResult.Diagnostic;
		break;
	case ECFBuilderPhysicsStructuralDriftBoundary::Blocked:
	default:
		Result.Compatibility = ECFBuilderPhysicsReceiptCompatibility::Blocked;
		Result.Diagnostic = StructuralResult.Diagnostic;
		break;
	}

	return Result;
}

// Legacy receipt-only refresh bool API를 typed structural boundary의 Equivalent projection으로 유지합니다.
bool FCFVehicleBuilderHardpointIntegrity::ValidatePhysicsReceiptRefreshBoundary(
	const FString& ReceiptResolvedDefinitionHash,
	const FString& CurrentTargetDefinitionHash,
	const FString& CurrentProspectiveResolvedDefinitionHash,
	const TArray<FCFVehicleFieldDiff>& CurrentPendingDiff,
	FString& OutError)
{
	// Legacy bool caller가 소비할 single typed structural 판정입니다.
	const FCFBuilderPhysicsStructuralDriftBoundaryResult BoundaryResult = EvaluatePhysicsStructuralDriftBoundary(
		ReceiptResolvedDefinitionHash,
		CurrentTargetDefinitionHash,
		CurrentProspectiveResolvedDefinitionHash,
		CurrentPendingDiff);

	if (BoundaryResult.Boundary != ECFBuilderPhysicsStructuralDriftBoundary::Equivalent)
	{
		OutError = BoundaryResult.Diagnostic;
		return false;
	}

	OutError.Reset();
	return true;
}
