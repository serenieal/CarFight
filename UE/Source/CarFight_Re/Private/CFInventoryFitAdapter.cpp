// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CF-FQ-035 INV-P0-04 Inventory Item Instance 기반 Fitting Adapter 구현
// Scope: Container 소유·접근, Reservation, Definition 해석, 중복 선택과 기존 BuildFittingSnapshot 연결을 Pawn 없이 구현합니다.
// Changelog:
// - v1.0.0: FCFInventoryFitAdapter 결정론적 Binding과 Equipment·Defense Domain 해석을 최초 구현.
// Migration:
// - Adapter는 Container, Reservation Ledger, VehicleData와 Domain DataAsset을 읽기만 한다.
// - 호환성·질량은 UCFVehicleFittingData::BuildFittingSnapshot에 위임하며 Runtime Apply 또는 Inventory 이동을 호출하지 않는다.

#include "CFInventoryFitAdapter.h"

#include "CFInventoryItemData.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingData.h"

namespace
{
	/**
	 * 접근·예약·Definition 검증을 통과한 실제 Item Instance 해석 결과입니다.
	 */
	struct FCFResolvedInventoryFitItem
	{
		// [v1.0.0] Container Entry에서 복사한 실제 Quantity 1 Item Instance입니다.
		FCFInventoryItemInstance ItemInstance;

		// [v1.0.0] Item Instance의 검증 시점 실제 Inventory 소유 위치입니다.
		FCFInventoryItemLocation ItemLocation;

		// [v1.0.0] ItemDefinitionId와 정확히 일치한 강타입 Inventory Definition입니다.
		const UCFInventoryItemData* ItemDefinition = nullptr;
	};

	// [v1.0.0] Adapter 실패 상태와 관련 Item·Mount 식별자를 결과에 기록합니다.
	void SetInventoryFitFailure(
		FCFInventoryFitResult& FitResult,
		const ECFInventoryFitStatus FitStatus,
		const FCFItemInstanceId& ItemInstanceId = FCFItemInstanceId(),
		const FName MountProfileId = NAME_None)
	{
		FitResult.AdapterStatus = FitStatus;
		FitResult.FailureItemInstanceId = ItemInstanceId;
		FitResult.FailureMountProfileId = MountProfileId;
	}

	// [v1.0.0] 조회된 Item 위치와 일치하는 실제 Container Entry Item Instance를 복사합니다.
	bool TryFindInventoryItemInstance(
		const TArray<FCFInventoryContainerState>& Containers,
		const FCFInventoryItemLocation& ItemLocation,
		FCFInventoryItemInstance& OutItemInstance)
	{
		OutItemInstance = FCFInventoryItemInstance();

		// [v1.0.0] Item 위치의 Container ID와 일치하는 현재 Container입니다.
		const FCFInventoryContainerState* ItemContainer = Containers.FindByPredicate(
			[&ItemLocation](const FCFInventoryContainerState& Container)
			{
				return Container.ContainerRef.ContainerId == ItemLocation.ContainerRef.ContainerId;
			});
		if (!ItemContainer)
		{
			return false;
		}

		// [v1.0.0] Item ID와 Slot ID가 모두 일치하는 현재 Container Entry입니다.
		const FCFInventoryContainerEntry* ItemEntry = ItemContainer->Entries.FindByPredicate(
			[&ItemLocation](const FCFInventoryContainerEntry& Entry)
			{
				return Entry.IsValid()
					&& Entry.ItemInstance.ItemHandle.ItemInstanceId == ItemLocation.ItemInstanceId
					&& Entry.ContainerSlotId == ItemLocation.ContainerSlotId;
			});
		if (!ItemEntry)
		{
			return false;
		}

		OutItemInstance = ItemEntry->ItemInstance;
		return true;
	}

	// [v1.0.0] ItemDefinitionId와 정확히 일치하는 단일 유효 Definition과 요구 도메인을 해석합니다.
	bool TryResolveInventoryItemDefinition(
		const TArray<UCFInventoryItemData*>& ItemDefinitions,
		const FCFInventoryItemInstance& ItemInstance,
		const ECFInventoryItemDomain RequiredItemDomain,
		const UCFInventoryItemData*& OutItemDefinition,
		ECFInventoryFitStatus& OutFailureStatus)
	{
		OutItemDefinition = nullptr;
		OutFailureStatus = ECFInventoryFitStatus::None;

		// [v1.0.0] ItemDefinitionId와 일치한 Definition의 개수입니다.
		int32 MatchingDefinitionCount = 0;

		for (const UCFInventoryItemData* ItemDefinition : ItemDefinitions)
		{
			if (!ItemDefinition
				|| ItemDefinition->GetPrimaryAssetId() != ItemInstance.ItemHandle.ItemDefinitionId)
			{
				continue;
			}

			++MatchingDefinitionCount;
			OutItemDefinition = ItemDefinition;
		}

		if (MatchingDefinitionCount == 0)
		{
			OutFailureStatus = ECFInventoryFitStatus::DefinitionMissing;
			return false;
		}

		if (MatchingDefinitionCount > 1)
		{
			OutItemDefinition = nullptr;
			OutFailureStatus = ECFInventoryFitStatus::DefinitionDuplicate;
			return false;
		}

		if (!OutItemDefinition || !ItemInstance.IsValidForDefinition(OutItemDefinition))
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemDefinitionMismatch;
			return false;
		}

		// [v1.0.0] 선택된 Inventory Definition 계약에서 발견된 오류 목록입니다.
		TArray<FText> DefinitionValidationErrors;
		if (!OutItemDefinition->ValidateItemDefinitionContract(DefinitionValidationErrors))
		{
			OutFailureStatus = ECFInventoryFitStatus::DefinitionContractInvalid;
			return false;
		}

		if (OutItemDefinition->GetItemDomain() != RequiredItemDomain)
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemDomainMismatch;
			return false;
		}

		return true;
	}

	// [v1.0.0] 실제 Item의 단일 위치, 현재 차량 접근, Reservation과 강타입 Definition을 함께 검증합니다.
	bool TryResolveSelectedInventoryItem(
		const TArray<FCFInventoryContainerState>& Containers,
		const TArray<UCFInventoryItemData*>& ItemDefinitions,
		const FCFInventoryTransferLedger& TransferLedger,
		const FCFInventoryAccessContext& AccessContext,
		const FCFItemInstanceId& ItemInstanceId,
		const ECFInventoryItemDomain RequiredItemDomain,
		FCFResolvedInventoryFitItem& OutResolvedItem,
		ECFInventoryFitStatus& OutFailureStatus)
	{
		OutResolvedItem = FCFResolvedInventoryFitItem();
		OutFailureStatus = ECFInventoryFitStatus::None;

		// [v1.0.0] 전체 Container 집합에서 조회한 Item Instance 단일 소유 위치입니다.
		const FCFInventoryLocationResult ItemLocationResult = FCFInventoryAccessQuery::QueryItemLocation(
			Containers,
			ItemInstanceId);
		if (ItemLocationResult.LocationState == ECFInventoryLocationState::Missing)
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemMissing;
			return false;
		}

		if (ItemLocationResult.LocationState == ECFInventoryLocationState::Duplicate)
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemDuplicate;
			return false;
		}

		if (!ItemLocationResult.IsFound())
		{
			OutFailureStatus = ECFInventoryFitStatus::InvalidRequest;
			return false;
		}

		OutResolvedItem.ItemLocation = ItemLocationResult.ItemLocation;
		if (OutResolvedItem.ItemLocation.ContainerRef.OwnerId != AccessContext.CurrentVehicleOwnerId)
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemInaccessible;
			return false;
		}

		// [v1.0.0] Item이 위치한 Container 종류에 대한 현재 차량 접근 Query 결과입니다.
		const FCFInventoryAccessResult ContainerAccessResult = FCFInventoryAccessQuery::QueryVehicleContainer(
			Containers,
			AccessContext,
			OutResolvedItem.ItemLocation.ContainerRef.ContainerType);
		if (!ContainerAccessResult.IsAccessible()
			|| ContainerAccessResult.ContainerRef.ContainerId != OutResolvedItem.ItemLocation.ContainerRef.ContainerId)
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemInaccessible;
			return false;
		}

		if (TransferLedger.IsItemReserved(ItemInstanceId))
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemReserved;
			return false;
		}

		if (!TryFindInventoryItemInstance(
			Containers,
			OutResolvedItem.ItemLocation,
			OutResolvedItem.ItemInstance))
		{
			OutFailureStatus = ECFInventoryFitStatus::ItemMissing;
			return false;
		}

		if (!TryResolveInventoryItemDefinition(
			ItemDefinitions,
			OutResolvedItem.ItemInstance,
			RequiredItemDomain,
			OutResolvedItem.ItemDefinition,
			OutFailureStatus))
		{
			return false;
		}

		return true;
	}
}

// [v1.0.0] MountProfile ID와 활성·빈 장착에 따른 ItemInstanceId 계약이 유효한지 반환합니다.
bool FCFInventoryFitMountInput::IsValid() const
{
	if (MountProfileId.IsNone())
	{
		return false;
	}

	return bEnabled ? ItemInstanceId.IsValid() : !ItemInstanceId.IsValid();
}

// [v1.0.0] 선택 방식과 Override ItemInstanceId 계약이 유효한지 반환합니다.
bool FCFInventoryFitDefenseInput::IsValid() const
{
	switch (SelectionMode)
	{
	case ECFDefenseSelectionMode::UseVehicleDefault:
	case ECFDefenseSelectionMode::ExplicitNone:
		return !ItemInstanceId.IsValid();
	case ECFDefenseSelectionMode::Override:
		return ItemInstanceId.IsValid();
	default:
		return false;
	}
}

// [v1.0.0] Fitting ID, VehicleData, 접근 Context와 모든 선택 입력의 기본 계약이 유효한지 반환합니다.
bool FCFInventoryFitRequest::IsValid() const
{
	if (FittingId.IsNone() || !VehicleData || !AccessContext.IsValid() || !DefenseSelection.IsValid())
	{
		return false;
	}

	for (const FCFInventoryFitMountInput& MountSelection : MountSelections)
	{
		if (!MountSelection.IsValid())
		{
			return false;
		}
	}

	return true;
}

// [v1.0.0] Inventory Binding과 기존 Fitting Snapshot 검증이 모두 성공했는지 반환합니다.
bool FCFInventoryFitResult::IsSuccessful() const
{
	return AdapterStatus == ECFInventoryFitStatus::Success && FittingSnapshot.IsValid();
}

// [v1.0.0] 실제 Item 소유·접근·예약과 Definition을 검증하고 결정론적 Binding과 기존 Fitting Snapshot을 생성합니다.
FCFInventoryFitResult FCFInventoryFitAdapter::BuildFittingBinding(
	const TArray<FCFInventoryContainerState>& Containers,
	const TArray<UCFInventoryItemData*>& ItemDefinitions,
	const FCFInventoryTransferLedger& TransferLedger,
	const FCFInventoryFitRequest& FitRequest)
{
	// [v1.0.0] 호출자에게 반환할 전체 Adapter Binding과 Fitting Snapshot 결과입니다.
	FCFInventoryFitResult FitResult;

	if (!FitRequest.IsValid())
	{
		SetInventoryFitFailure(FitResult, ECFInventoryFitStatus::InvalidRequest);
		return FitResult;
	}

	// [v1.0.0] Container 집합 불변식에서 발견된 오류 목록입니다.
	TArray<FText> ContainerValidationErrors;
	if (!FCFInventoryAccessQuery::ValidateContainerSet(Containers, ContainerValidationErrors))
	{
		SetInventoryFitFailure(FitResult, ECFInventoryFitStatus::InvalidContainerSet);
		return FitResult;
	}

	// [v1.0.0] VehicleData에 실제 존재하는 유효 MountProfile ID 집합입니다.
	TSet<FName> KnownMountProfileIds;
	for (const FCFVehicleMountProfile& MountProfile : FitRequest.VehicleData->MountProfiles)
	{
		if (!MountProfile.MountProfileId.IsNone())
		{
			KnownMountProfileIds.Add(MountProfile.MountProfileId);
		}
	}

	// [v1.0.0] 요청 안에서 이미 사용된 MountProfile ID 집합입니다.
	TSet<FName> SelectedMountProfileIds;
	for (const FCFInventoryFitMountInput& MountSelection : FitRequest.MountSelections)
	{
		if (SelectedMountProfileIds.Contains(MountSelection.MountProfileId))
		{
			SetInventoryFitFailure(
				FitResult,
				ECFInventoryFitStatus::DuplicateMountSelection,
				MountSelection.ItemInstanceId,
				MountSelection.MountProfileId);
			return FitResult;
		}

		SelectedMountProfileIds.Add(MountSelection.MountProfileId);
		if (!KnownMountProfileIds.Contains(MountSelection.MountProfileId))
		{
			SetInventoryFitFailure(
				FitResult,
				ECFInventoryFitStatus::UnknownMountProfile,
				MountSelection.ItemInstanceId,
				MountSelection.MountProfileId);
			return FitResult;
		}
	}

	// [v1.0.0] Mount와 Defense 전체에서 이미 선택된 실제 Item Instance ID 집합입니다.
	TSet<FCFItemInstanceId> SelectedItemInstanceIds;
	for (const FCFInventoryFitMountInput& MountSelection : FitRequest.MountSelections)
	{
		if (!MountSelection.bEnabled)
		{
			continue;
		}

		if (SelectedItemInstanceIds.Contains(MountSelection.ItemInstanceId))
		{
			SetInventoryFitFailure(
				FitResult,
				ECFInventoryFitStatus::DuplicateItemSelection,
				MountSelection.ItemInstanceId,
				MountSelection.MountProfileId);
			return FitResult;
		}

		SelectedItemInstanceIds.Add(MountSelection.ItemInstanceId);
	}

	if (FitRequest.DefenseSelection.SelectionMode == ECFDefenseSelectionMode::Override)
	{
		if (SelectedItemInstanceIds.Contains(FitRequest.DefenseSelection.ItemInstanceId))
		{
			SetInventoryFitFailure(
				FitResult,
				ECFInventoryFitStatus::DuplicateItemSelection,
				FitRequest.DefenseSelection.ItemInstanceId);
			return FitResult;
		}

		SelectedItemInstanceIds.Add(FitRequest.DefenseSelection.ItemInstanceId);
	}

	// [v1.0.0] 실제 Item Binding을 기존 Fitting 입력으로 변환할 Transient 후보 DataAsset입니다.
	UCFVehicleFittingData* CandidateFittingData = NewObject<UCFVehicleFittingData>(GetTransientPackage());
	if (!CandidateFittingData)
	{
		SetInventoryFitFailure(FitResult, ECFInventoryFitStatus::InvalidRequest);
		return FitResult;
	}

	CandidateFittingData->FittingId = FitRequest.FittingId;
	CandidateFittingData->VehicleData = FitRequest.VehicleData;
	CandidateFittingData->MissingMountSelectionPolicy = FitRequest.MissingMountSelectionPolicy;

	for (const FCFVehicleMountProfile& MountProfile : FitRequest.VehicleData->MountProfiles)
	{
		// [v1.0.0] 현재 VehicleData MountProfile과 일치하는 유일한 Inventory 선택입니다.
		const FCFInventoryFitMountInput* InventoryMountSelection = FitRequest.MountSelections.FindByPredicate(
			[&MountProfile](const FCFInventoryFitMountInput& CandidateSelection)
			{
				return CandidateSelection.MountProfileId == MountProfile.MountProfileId;
			});
		if (!InventoryMountSelection)
		{
			continue;
		}

		// [v1.0.0] 기존 BuildFittingSnapshot에 전달할 한 Mount 선택입니다.
		FCFVehicleMountSelection FittingMountSelection;
		FittingMountSelection.MountProfileId = InventoryMountSelection->MountProfileId;
		FittingMountSelection.bEnabled = InventoryMountSelection->bEnabled;

		// [v1.0.0] VehicleData 순서로 반환할 실제 또는 빈 장착 Binding입니다.
		FCFInventoryFitMountBinding MountBinding;
		MountBinding.MountProfileId = InventoryMountSelection->MountProfileId;
		MountBinding.bEnabled = InventoryMountSelection->bEnabled;

		if (!InventoryMountSelection->bEnabled)
		{
			CandidateFittingData->MountSelections.Add(FittingMountSelection);
			FitResult.MountBindings.Add(MountBinding);
			continue;
		}

		// [v1.0.0] Equipment 도메인으로 소유·접근·예약·Definition 검증을 통과한 Item 해석입니다.
		FCFResolvedInventoryFitItem ResolvedEquipmentItem;

		// [v1.0.0] 현재 Equipment Item 해석 실패 상태입니다.
		ECFInventoryFitStatus ResolveFailureStatus = ECFInventoryFitStatus::None;
		if (!TryResolveSelectedInventoryItem(
			Containers,
			ItemDefinitions,
			TransferLedger,
			FitRequest.AccessContext,
			InventoryMountSelection->ItemInstanceId,
			ECFInventoryItemDomain::Equipment,
			ResolvedEquipmentItem,
			ResolveFailureStatus))
		{
			SetInventoryFitFailure(
				FitResult,
				ResolveFailureStatus,
				InventoryMountSelection->ItemInstanceId,
				InventoryMountSelection->MountProfileId);
			return FitResult;
		}

		// [v1.0.0] Equipment 도메인으로 확인된 강타입 Inventory Definition입니다.
		const UCFEquipmentItemData* EquipmentItemDefinition = Cast<UCFEquipmentItemData>(ResolvedEquipmentItem.ItemDefinition);
		if (!EquipmentItemDefinition || !EquipmentItemDefinition->EquipmentPresetData)
		{
			SetInventoryFitFailure(
				FitResult,
				ECFInventoryFitStatus::DefinitionContractInvalid,
				InventoryMountSelection->ItemInstanceId,
				InventoryMountSelection->MountProfileId);
			return FitResult;
		}

		FittingMountSelection.EquipmentPresetData = EquipmentItemDefinition->EquipmentPresetData;
		CandidateFittingData->MountSelections.Add(FittingMountSelection);

		MountBinding.ItemInstanceId = ResolvedEquipmentItem.ItemInstance.ItemHandle.ItemInstanceId;
		MountBinding.ItemDefinitionId = ResolvedEquipmentItem.ItemInstance.ItemHandle.ItemDefinitionId;
		MountBinding.SourceLocation = ResolvedEquipmentItem.ItemLocation;
		MountBinding.ResolvedEquipmentPresetData = EquipmentItemDefinition->EquipmentPresetData;
		FitResult.MountBindings.Add(MountBinding);
	}

	FitResult.DefenseBinding.SelectionMode = FitRequest.DefenseSelection.SelectionMode;
	CandidateFittingData->DefenseSelection.SelectionMode = FitRequest.DefenseSelection.SelectionMode;
	if (FitRequest.DefenseSelection.SelectionMode == ECFDefenseSelectionMode::Override)
	{
		// [v1.0.0] Defense 도메인으로 소유·접근·예약·Definition 검증을 통과한 Item 해석입니다.
		FCFResolvedInventoryFitItem ResolvedDefenseItem;

		// [v1.0.0] 현재 Defense Item 해석 실패 상태입니다.
		ECFInventoryFitStatus ResolveFailureStatus = ECFInventoryFitStatus::None;
		if (!TryResolveSelectedInventoryItem(
			Containers,
			ItemDefinitions,
			TransferLedger,
			FitRequest.AccessContext,
			FitRequest.DefenseSelection.ItemInstanceId,
			ECFInventoryItemDomain::Defense,
			ResolvedDefenseItem,
			ResolveFailureStatus))
		{
			SetInventoryFitFailure(
				FitResult,
				ResolveFailureStatus,
				FitRequest.DefenseSelection.ItemInstanceId);
			return FitResult;
		}

		// [v1.0.0] Defense 도메인으로 확인된 강타입 Inventory Definition입니다.
		const UCFDefenseItemData* DefenseItemDefinition = Cast<UCFDefenseItemData>(ResolvedDefenseItem.ItemDefinition);
		if (!DefenseItemDefinition || !DefenseItemDefinition->VehicleDefenseData)
		{
			SetInventoryFitFailure(
				FitResult,
				ECFInventoryFitStatus::DefinitionContractInvalid,
				FitRequest.DefenseSelection.ItemInstanceId);
			return FitResult;
		}

		CandidateFittingData->DefenseSelection.DefenseData = DefenseItemDefinition->VehicleDefenseData;
		FitResult.DefenseBinding.bHasItemBinding = true;
		FitResult.DefenseBinding.ItemInstanceId = ResolvedDefenseItem.ItemInstance.ItemHandle.ItemInstanceId;
		FitResult.DefenseBinding.ItemDefinitionId = ResolvedDefenseItem.ItemInstance.ItemHandle.ItemDefinitionId;
		FitResult.DefenseBinding.SourceLocation = ResolvedDefenseItem.ItemLocation;
		FitResult.DefenseBinding.ResolvedDefenseData = DefenseItemDefinition->VehicleDefenseData;
	}
	else
	{
		CandidateFittingData->DefenseSelection.DefenseData = nullptr;
	}

	FitResult.FittingSnapshot = CandidateFittingData->BuildFittingSnapshot();
	if (!FitResult.FittingSnapshot.IsValid())
	{
		SetInventoryFitFailure(FitResult, ECFInventoryFitStatus::FittingSnapshotInvalid);
		return FitResult;
	}

	FitResult.AdapterStatus = ECFInventoryFitStatus::Success;
	return FitResult;
}
