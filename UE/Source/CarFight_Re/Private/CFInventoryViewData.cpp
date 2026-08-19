// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-14
// Description: CF-FQ-035 INV-P0-05 읽기 전용 Inventory ViewData와 의미 ChangeSet 구현
// Scope: 현재 차량 P0 Container·Definition·Reservation을 읽어 결정론적 Snapshot을 만들고 이전 Snapshot 대비 의미 변화를 계산합니다.
// Changelog:
// - v1.0.0: View Snapshot Builder, Definition 강타입 검증, 외부 CompatibilityHint 합성, Item·Container 의미 Diff를 최초 구현.
// Migration:
// - Container·Ledger·Definition은 읽기만 하며 Inventory 소유권 또는 Transaction 상태를 변경하지 않습니다.
// - 현재 차량 Owner의 VehicleCargo·MountedEquipment만 Snapshot에 포함하고 접근 차단 Container는 Container 요약만 보존하며 Item 행은 노출하지 않습니다.
// - Transaction 완료·실패는 Snapshot Diff에서 생성하지 않습니다.

#include "CFInventoryViewData.h"

namespace
{
	// [v1.0.0] 같은 Definition ID와 정확히 일치하는 단일 유효 Inventory Definition을 찾습니다.
	const UCFInventoryItemData* ResolveViewDefinition(
		const TArray<UCFInventoryItemData*>& ItemDefinitions,
		const FCFInventoryItemInstance& ItemInstance,
		ECFInventoryViewBuildStatus& OutFailureStatus)
	{
		OutFailureStatus = ECFInventoryViewBuildStatus::None;

		// [v1.0.0] ItemDefinitionId와 일치한 유효 Definition 개수입니다.
		int32 MatchingDefinitionCount = 0;
		// [v1.0.0] 정확히 하나일 때 반환할 Definition입니다.
		const UCFInventoryItemData* MatchingDefinition = nullptr;
		for (const UCFInventoryItemData* ItemDefinition : ItemDefinitions)
		{
			if (!ItemDefinition || ItemDefinition->GetPrimaryAssetId() != ItemInstance.ItemHandle.ItemDefinitionId)
			{
				continue;
			}

			++MatchingDefinitionCount;
			MatchingDefinition = ItemDefinition;
		}

		if (MatchingDefinitionCount == 0)
		{
			OutFailureStatus = ECFInventoryViewBuildStatus::DefinitionMissing;
			return nullptr;
		}

		if (MatchingDefinitionCount > 1)
		{
			OutFailureStatus = ECFInventoryViewBuildStatus::DefinitionDuplicate;
			return nullptr;
		}

		if (!MatchingDefinition || !ItemInstance.IsValidForDefinition(MatchingDefinition))
		{
			OutFailureStatus = ECFInventoryViewBuildStatus::ItemDefinitionMismatch;
			return nullptr;
		}

		// [v1.0.0] Definition 계약 오류를 받을 임시 목록입니다.
		TArray<FText> DefinitionValidationErrors;
		if (!MatchingDefinition->ValidateItemDefinitionContract(DefinitionValidationErrors))
		{
			OutFailureStatus = ECFInventoryViewBuildStatus::DefinitionContractInvalid;
			return nullptr;
		}

		return MatchingDefinition;
	}

	// [v1.0.0] 특정 Item Instance에 대해 외부 Fitting이 제공한 첫 호환성 힌트를 반환합니다.
	FText FindCompatibilityHint(
		const TArray<FCFInventoryCompatibilityHint>& CompatibilityHints,
		const FCFItemInstanceId& ItemInstanceId)
	{
		for (const FCFInventoryCompatibilityHint& CompatibilityHint : CompatibilityHints)
		{
			if (CompatibilityHint.ItemInstanceId == ItemInstanceId)
			{
				return CompatibilityHint.CompatibilityHint;
			}
		}

		return FText::GetEmpty();
	}

	// [v1.0.0] 고정 P0 Container 표시 순서를 반환합니다.
	int32 GetContainerSortOrder(const ECFInventoryContainerType ContainerType)
	{
		switch (ContainerType)
		{
		case ECFInventoryContainerType::VehicleCargo:
			return 0;
		case ECFInventoryContainerType::MountedEquipment:
			return 1;
		default:
			return 2;
		}
	}

	// [v1.0.0] Item ViewData의 결정론적 정렬 우선순위를 비교합니다.
	bool SortItemViewData(const FCFInventoryItemViewData& LeftItem, const FCFInventoryItemViewData& RightItem)
	{
		// [v1.0.0] 왼쪽 Item이 속한 Container 종류의 고정 표시 순서입니다.
		const int32 LeftContainerOrder = GetContainerSortOrder(LeftItem.ContainerRef.ContainerType);
		// [v1.0.0] 오른쪽 Item이 속한 Container 종류의 고정 표시 순서입니다.
		const int32 RightContainerOrder = GetContainerSortOrder(RightItem.ContainerRef.ContainerType);
		if (LeftContainerOrder != RightContainerOrder)
		{
			return LeftContainerOrder < RightContainerOrder;
		}

		// [v1.0.0] 같은 Container 종류에서 Slot ID 이름 순서를 안정적으로 비교합니다.
		const FString LeftSlotText = LeftItem.ContainerSlotId.ToString();
		// [v1.0.0] 같은 Container 종류에서 비교할 오른쪽 Slot ID 문자열입니다.
		const FString RightSlotText = RightItem.ContainerSlotId.ToString();
		const int32 SlotComparison = LeftSlotText.Compare(RightSlotText, ESearchCase::CaseSensitive);
		if (SlotComparison != 0)
		{
			return SlotComparison < 0;
		}

		return LeftItem.ItemInstanceId.ToString() < RightItem.ItemInstanceId.ToString();
	}

	// [v1.0.0] Snapshot에서 지정 Item ID를 찾습니다.
	const FCFInventoryItemViewData* FindItemViewData(
		const FCFInventoryViewSnapshot& Snapshot,
		const FCFItemInstanceId& ItemInstanceId)
	{
		return Snapshot.Items.FindByPredicate(
			[&ItemInstanceId](const FCFInventoryItemViewData& ItemViewData)
			{
				return ItemViewData.ItemInstanceId == ItemInstanceId;
			});
	}

	// [v1.0.0] Snapshot에서 지정 Container ID를 찾습니다.
	const FCFInventoryContainerViewData* FindContainerViewData(
		const FCFInventoryViewSnapshot& Snapshot,
		const FCFInventoryContainerId& ContainerId)
	{
		return Snapshot.Containers.FindByPredicate(
			[&ContainerId](const FCFInventoryContainerViewData& ContainerViewData)
			{
				return ContainerViewData.ContainerRef.ContainerId == ContainerId;
			});
	}

	// [v1.0.0] ChangeSet에 Item 관련 의미 변화를 한 건 추가합니다.
	void AddItemChange(
		FCFInventoryViewChangeSet& ChangeSet,
		const ECFInventoryViewChangeType ChangeType,
		const FCFInventoryItemViewData& ItemViewData)
	{
		// [v1.0.0] 추가할 Item 의미 변화입니다.
		FCFInventoryViewChange ViewChange;
		ViewChange.ChangeType = ChangeType;
		ViewChange.ItemInstanceId = ItemViewData.ItemInstanceId;
		ViewChange.ContainerId = ItemViewData.ContainerRef.ContainerId;
		ChangeSet.Changes.Add(ViewChange);
	}

	// [v1.0.0] ChangeSet에 Container 요약 변화를 한 건 추가합니다.
	void AddContainerChange(
		FCFInventoryViewChangeSet& ChangeSet,
		const FCFInventoryContainerViewData& ContainerViewData)
	{
		// [v1.0.0] 추가할 Container 의미 변화입니다.
		FCFInventoryViewChange ViewChange;
		ViewChange.ChangeType = ECFInventoryViewChangeType::ContainerChanged;
		ViewChange.ContainerId = ContainerViewData.ContainerRef.ContainerId;
		ChangeSet.Changes.Add(ViewChange);
	}
}

// [v1.0.0] Item ID, Definition, 소유 위치를 포함한 읽기 전용 행 계약이 유효한지 반환합니다.
bool FCFInventoryItemViewData::IsValid() const
{
	return ItemInstanceId.IsValid()
		&& ItemDefinitionId.IsValid()
		&& ItemDomain != ECFInventoryItemDomain::Unknown
		&& Quantity > 0
		&& ContainerRef.IsValid()
		&& !ContainerSlotId.IsNone();
}

// [v1.0.0] Container 참조와 슬롯 수가 읽기 전용 표시 계약으로 유효한지 반환합니다.
bool FCFInventoryContainerViewData::IsValid() const
{
	return ContainerRef.IsValid()
		&& OccupiedSlotCount >= 0
		&& RemainingSlotCount >= 0;
}

// [v1.0.0] 현재 차량 Owner와 모든 Container·Item ViewData가 유효한지 반환합니다.
bool FCFInventoryViewSnapshot::IsValid() const
{
	if (!CurrentVehicleOwnerId.IsValid())
	{
		return false;
	}

	for (const FCFInventoryContainerViewData& ContainerViewData : Containers)
	{
		if (!ContainerViewData.IsValid() || ContainerViewData.ContainerRef.OwnerId != CurrentVehicleOwnerId)
		{
			return false;
		}
	}

	for (const FCFInventoryItemViewData& ItemViewData : Items)
	{
		if (!ItemViewData.IsValid() || ItemViewData.ContainerRef.OwnerId != CurrentVehicleOwnerId)
		{
			return false;
		}
	}

	return true;
}

// [v1.0.0] ViewData Snapshot을 UI에 사용할 수 있는 정상 결과인지 반환합니다.
bool FCFInventoryViewBuildResult::IsSuccessful() const
{
	return BuildStatus == ECFInventoryViewBuildStatus::Success && Snapshot.IsValid();
}

// [v1.0.0] 현재 차량의 접근 가능한 P0 Container와 실제 Item을 결정론적 읽기 전용 Snapshot으로 변환합니다.
FCFInventoryViewBuildResult FCFInventoryViewBuilder::BuildSnapshot(
	const TArray<FCFInventoryContainerState>& Containers,
	const TArray<UCFInventoryItemData*>& ItemDefinitions,
	const FCFInventoryTransferLedger& TransferLedger,
	const FCFInventoryAccessContext& AccessContext,
	const TArray<FCFInventoryCompatibilityHint>& CompatibilityHints)
{
	// [v1.0.0] 호출자에게 반환할 Snapshot 생성 결과입니다.
	FCFInventoryViewBuildResult BuildResult;
	if (!AccessContext.IsValid())
	{
		BuildResult.BuildStatus = ECFInventoryViewBuildStatus::InvalidAccessContext;
		return BuildResult;
	}

	// [v1.0.0] 전체 Container 집합 불변식 오류 목록입니다.
	TArray<FText> ContainerValidationErrors;
	if (!FCFInventoryAccessQuery::ValidateContainerSet(Containers, ContainerValidationErrors))
	{
		BuildResult.BuildStatus = ECFInventoryViewBuildStatus::InvalidContainerSet;
		return BuildResult;
	}

	BuildResult.Snapshot.CurrentVehicleOwnerId = AccessContext.CurrentVehicleOwnerId;

	// [v1.0.0] P0 UI에서 고정 순서로 처리할 Container 종류입니다.
	const TArray<ECFInventoryContainerType> SupportedContainerTypes = {
		ECFInventoryContainerType::VehicleCargo,
		ECFInventoryContainerType::MountedEquipment
	};

	for (const ECFInventoryContainerType ContainerType : SupportedContainerTypes)
	{
		// [v1.0.0] 현재 차량 Owner와 Container 종류를 기준으로 한 접근 결과입니다.
		const FCFInventoryAccessResult AccessResult = FCFInventoryAccessQuery::QueryVehicleContainer(
			Containers,
			AccessContext,
			ContainerType);
		if (AccessResult.AccessState == ECFInventoryAccessState::Missing)
		{
			continue;
		}

		// [v1.0.0] 현재 차량 Owner와 종류가 일치하는 실제 Container입니다.
		const FCFInventoryContainerState* ContainerState = Containers.FindByPredicate(
			[&AccessContext, ContainerType](const FCFInventoryContainerState& CandidateContainer)
			{
				return CandidateContainer.ContainerRef.OwnerId == AccessContext.CurrentVehicleOwnerId
					&& CandidateContainer.ContainerRef.ContainerType == ContainerType;
			});
		if (!ContainerState)
		{
			continue;
		}

		// [v1.0.0] UI에 표시할 현재 Container 요약입니다.
		FCFInventoryContainerViewData ContainerViewData;
		ContainerViewData.ContainerRef = ContainerState->ContainerRef;
		ContainerViewData.bIsAccessible = AccessResult.IsAccessible();
		ContainerViewData.OccupiedSlotCount = ContainerState->Entries.Num();
		ContainerViewData.RemainingSlotCount = FMath::Max(0, ContainerState->Capacity.MaximumSlotCount - ContainerState->Entries.Num());
		BuildResult.Snapshot.Containers.Add(ContainerViewData);

		if (!ContainerViewData.bIsAccessible)
		{
			continue;
		}

		for (const FCFInventoryContainerEntry& ContainerEntry : ContainerState->Entries)
		{
			// [v1.0.0] 이 실제 Item Instance의 단일 강타입 Inventory Definition입니다.
			ECFInventoryViewBuildStatus DefinitionFailureStatus = ECFInventoryViewBuildStatus::None;
			const UCFInventoryItemData* ItemDefinition = ResolveViewDefinition(
				ItemDefinitions,
				ContainerEntry.ItemInstance,
				DefinitionFailureStatus);
			if (!ItemDefinition)
			{
				BuildResult.BuildStatus = DefinitionFailureStatus;
				BuildResult.FailureItemInstanceId = ContainerEntry.ItemInstance.ItemHandle.ItemInstanceId;
				BuildResult.Snapshot = FCFInventoryViewSnapshot();
				return BuildResult;
			}

			// [v1.0.0] Container/Definition/Ledger에서 읽어 생성할 한 UI Item 행입니다.
			FCFInventoryItemViewData ItemViewData;
			ItemViewData.ItemInstanceId = ContainerEntry.ItemInstance.ItemHandle.ItemInstanceId;
			ItemViewData.ItemDefinitionId = ContainerEntry.ItemInstance.ItemHandle.ItemDefinitionId;
			ItemViewData.DisplayName = ItemDefinition->DisplayName;
			ItemViewData.ItemDomain = ItemDefinition->GetItemDomain();
			ItemViewData.Quantity = ContainerEntry.ItemInstance.Quantity;
			ItemViewData.ContainerRef = ContainerState->ContainerRef;
			ItemViewData.ContainerSlotId = ContainerEntry.ContainerSlotId;
			ItemViewData.bIsReserved = TransferLedger.IsItemReserved(ItemViewData.ItemInstanceId);
			ItemViewData.bIsAccessible = true;
			ItemViewData.CompatibilityHint = FindCompatibilityHint(CompatibilityHints, ItemViewData.ItemInstanceId);
			BuildResult.Snapshot.Items.Add(ItemViewData);
		}
	}

	BuildResult.Snapshot.Items.Sort(SortItemViewData);
	BuildResult.BuildStatus = ECFInventoryViewBuildStatus::Success;
	return BuildResult;
}

// [v1.0.0] 이전/현재 Snapshot의 Item 위치·예약·표시와 Container 요약 차이를 의미 ChangeSet으로 계산합니다.
FCFInventoryViewChangeSet FCFInventoryViewBuilder::DiffSnapshots(
	const FCFInventoryViewSnapshot& PreviousSnapshot,
	const FCFInventoryViewSnapshot& CurrentSnapshot)
{
	// [v1.0.0] 호출자에게 반환할 결정론적 의미 변화 목록입니다.
	FCFInventoryViewChangeSet ChangeSet;

	for (const FCFInventoryItemViewData& PreviousItem : PreviousSnapshot.Items)
	{
		// [v1.0.0] 현재 Snapshot에서 같은 실제 Item Instance 행입니다.
		const FCFInventoryItemViewData* CurrentItem = FindItemViewData(CurrentSnapshot, PreviousItem.ItemInstanceId);
		if (!CurrentItem)
		{
			AddItemChange(ChangeSet, ECFInventoryViewChangeType::ItemRemoved, PreviousItem);
			continue;
		}

		if (PreviousItem.ContainerRef.ContainerId != CurrentItem->ContainerRef.ContainerId
			|| PreviousItem.ContainerSlotId != CurrentItem->ContainerSlotId)
		{
			AddItemChange(ChangeSet, ECFInventoryViewChangeType::ItemLocationChanged, *CurrentItem);
		}

		if (PreviousItem.bIsReserved != CurrentItem->bIsReserved)
		{
			AddItemChange(ChangeSet, ECFInventoryViewChangeType::ItemReservationChanged, *CurrentItem);
		}

		if (PreviousItem.ItemDefinitionId != CurrentItem->ItemDefinitionId
			|| !PreviousItem.DisplayName.EqualTo(CurrentItem->DisplayName)
			|| PreviousItem.ItemDomain != CurrentItem->ItemDomain
			|| PreviousItem.Quantity != CurrentItem->Quantity
			|| PreviousItem.bIsAccessible != CurrentItem->bIsAccessible
			|| !PreviousItem.CompatibilityHint.EqualTo(CurrentItem->CompatibilityHint))
		{
			AddItemChange(ChangeSet, ECFInventoryViewChangeType::ItemPresentationChanged, *CurrentItem);
		}
	}

	for (const FCFInventoryItemViewData& CurrentItem : CurrentSnapshot.Items)
	{
		if (!FindItemViewData(PreviousSnapshot, CurrentItem.ItemInstanceId))
		{
			AddItemChange(ChangeSet, ECFInventoryViewChangeType::ItemAdded, CurrentItem);
		}
	}

	for (const FCFInventoryContainerViewData& PreviousContainer : PreviousSnapshot.Containers)
	{
		// [v1.0.0] 현재 Snapshot에서 같은 Container ID의 요약입니다.
		const FCFInventoryContainerViewData* CurrentContainer = FindContainerViewData(
			CurrentSnapshot,
			PreviousContainer.ContainerRef.ContainerId);
		if (!CurrentContainer)
		{
			AddContainerChange(ChangeSet, PreviousContainer);
			continue;
		}

		if (PreviousContainer.bIsAccessible != CurrentContainer->bIsAccessible
			|| PreviousContainer.OccupiedSlotCount != CurrentContainer->OccupiedSlotCount
			|| PreviousContainer.RemainingSlotCount != CurrentContainer->RemainingSlotCount)
		{
			AddContainerChange(ChangeSet, *CurrentContainer);
		}
	}

	for (const FCFInventoryContainerViewData& CurrentContainer : CurrentSnapshot.Containers)
	{
		if (!FindContainerViewData(PreviousSnapshot, CurrentContainer.ContainerRef.ContainerId))
		{
			AddContainerChange(ChangeSet, CurrentContainer);
		}
	}

	ChangeSet.Changes.Sort(
		[](const FCFInventoryViewChange& LeftChange, const FCFInventoryViewChange& RightChange)
		{
			if (LeftChange.ChangeType != RightChange.ChangeType)
			{
				return static_cast<uint8>(LeftChange.ChangeType) < static_cast<uint8>(RightChange.ChangeType);
			}

			// [v1.0.0] 같은 변화 종류에서 Item ID 문자열 기준으로 안정적으로 정렬할 왼쪽 키입니다.
			const FString LeftItemKey = LeftChange.ItemInstanceId.ToString();
			// [v1.0.0] 같은 변화 종류에서 비교할 오른쪽 Item ID 문자열 키입니다.
			const FString RightItemKey = RightChange.ItemInstanceId.ToString();
			const int32 ItemComparison = LeftItemKey.Compare(RightItemKey, ESearchCase::CaseSensitive);
			if (ItemComparison != 0)
			{
				return ItemComparison < 0;
			}

			return LeftChange.ContainerId.Value.ToString() < RightChange.ContainerId.Value.ToString();
		});

	return ChangeSet;
}
