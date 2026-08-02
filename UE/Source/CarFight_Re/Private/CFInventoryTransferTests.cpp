// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-03 Reservation과 Atomic Transfer Pawn 없는 자동화 테스트
// Scope: 이중 예약 거부, Capacity 선점, 멱등 취소, Prepare 무변경, 원자 Commit·Rollback과 중복 Commit 방지를 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Inventory.INV_P0_03.Reservation과 AtomicTransfer 테스트를 최초 추가.
// Migration:
// - Pawn, World, Fitting Runtime, UI, SaveGame과 Unreal Asset을 생성하거나 수정하지 않는다.
// - 질량 Capacity는 Transient Domain Definition과 별도의 외부 해석 Mass Snapshot으로 검증한다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFInventoryTransfer.h"
#include "CFInventoryItemData.h"
#include "CFEquipmentPresetData.h"

#include "Misc/AutomationTest.h"

namespace
{
	/**
	 * INV-P0-03 테스트에서 공유할 Transient Definition, Item과 Container 집합입니다.
	 */
	struct FCFInventoryTransferFixture
	{
		// [v1.0.0] 테스트 Equipment Definition이 참조할 Transient Domain DataAsset입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] Quantity 1 Item Instance를 생성할 Transient Equipment Definition입니다.
		UCFEquipmentItemData* EquipmentItemDefinition = nullptr;

		// [v1.0.0] 첫 Transfer와 Item Reservation 충돌에 사용할 실제 Item Instance입니다.
		FCFInventoryItemInstance FirstItemInstance;

		// [v1.0.0] Destination Slot·Mass Capacity 충돌과 실패 Commit에 사용할 실제 Item Instance입니다.
		FCFInventoryItemInstance SecondItemInstance;

		// [v1.0.0] Source와 Destination Container가 공유하는 차량 Owner ID입니다.
		FCFInventoryOwnerId VehicleOwnerId;

		// [v1.0.0] VehicleCargo Source Container ID입니다.
		FCFInventoryContainerId SourceContainerId;

		// [v1.0.0] MountedEquipment Destination Container ID입니다.
		FCFInventoryContainerId DestinationContainerId;

		// [v1.0.0] 두 실제 Item을 Source에 소유한 Container 집합입니다.
		TArray<FCFInventoryContainerState> Containers;
	};

	// [v1.0.0] 테스트 Container에 Item Instance와 Slot ID Entry를 추가합니다.
	void AddTransferTestEntry(
		FCFInventoryContainerState& Container,
		const FCFInventoryItemInstance& ItemInstance,
		const FName ContainerSlotId)
	{
		// [v1.0.0] 테스트 Container에 추가할 Quantity 1 Item 소유 Entry입니다.
		FCFInventoryContainerEntry Entry;
		Entry.ItemInstance = ItemInstance;
		Entry.ContainerSlotId = ContainerSlotId;
		Container.Entries.Add(Entry);
	}

	// [v1.0.0] Pawn 없이 유효한 두 Item과 Source·Destination Container 집합을 생성합니다.
	bool BuildTransferFixture(FAutomationTestBase& Test, FCFInventoryTransferFixture& OutFixture)
	{
		OutFixture.EquipmentPresetData = NewObject<UCFEquipmentPresetData>();
		if (!Test.TestNotNull(TEXT("EquipmentPresetData 생성"), OutFixture.EquipmentPresetData))
		{
			return false;
		}

		OutFixture.EquipmentItemDefinition = NewObject<UCFEquipmentItemData>();
		if (!Test.TestNotNull(TEXT("Equipment Item Definition 생성"), OutFixture.EquipmentItemDefinition))
		{
			return false;
		}
		OutFixture.EquipmentItemDefinition->ItemDefinitionId = TEXT("Test_TransferEquipment");
		OutFixture.EquipmentItemDefinition->EquipmentPresetData = OutFixture.EquipmentPresetData;

		OutFixture.FirstItemInstance = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentItemDefinition);
		OutFixture.SecondItemInstance = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentItemDefinition);
		if (!Test.TestTrue(TEXT("첫 Item Instance 유효"), OutFixture.FirstItemInstance.IsValid())
			|| !Test.TestTrue(TEXT("두 번째 Item Instance 유효"), OutFixture.SecondItemInstance.IsValid()))
		{
			return false;
		}

		OutFixture.VehicleOwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.SourceContainerId = FCFInventoryContainerId::CreateNew();
		OutFixture.DestinationContainerId = FCFInventoryContainerId::CreateNew();

		// [v1.0.0] 두 Item Instance를 소유하는 VehicleCargo Source Container입니다.
		FCFInventoryContainerState SourceContainer;
		SourceContainer.ContainerRef.OwnerId = OutFixture.VehicleOwnerId;
		SourceContainer.ContainerRef.ContainerId = OutFixture.SourceContainerId;
		SourceContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		SourceContainer.Capacity.MaximumSlotCount = 4;
		AddTransferTestEntry(SourceContainer, OutFixture.FirstItemInstance, TEXT("CargoSlot_01"));
		AddTransferTestEntry(SourceContainer, OutFixture.SecondItemInstance, TEXT("CargoSlot_02"));

		// [v1.0.0] 슬롯 2개와 100kg 질량 한도를 가진 빈 MountedEquipment Destination Container입니다.
		FCFInventoryContainerState DestinationContainer;
		DestinationContainer.ContainerRef.OwnerId = OutFixture.VehicleOwnerId;
		DestinationContainer.ContainerRef.ContainerId = OutFixture.DestinationContainerId;
		DestinationContainer.ContainerRef.ContainerType = ECFInventoryContainerType::MountedEquipment;
		DestinationContainer.Capacity.MaximumSlotCount = 2;
		DestinationContainer.Capacity.bUseMassLimit = true;
		DestinationContainer.Capacity.MaximumMassKg = 100.0f;

		OutFixture.Containers.Add(SourceContainer);
		OutFixture.Containers.Add(DestinationContainer);

		// [v1.0.0] 생성된 Container 집합 계약 오류를 수집할 배열입니다.
		TArray<FText> ValidationErrors;
		return Test.TestTrue(
			TEXT("Transfer Fixture Container 집합 유효"),
			FCFInventoryAccessQuery::ValidateContainerSet(OutFixture.Containers, ValidationErrors));
	}

	// [v1.0.0] 지정 Item과 Source·Destination Slot을 사용하는 Transfer Request를 생성합니다.
	FCFInventoryTransferRequest CreateTransferRequest(
		const FCFInventoryTransferFixture& Fixture,
		const FCFInventoryItemInstance& ItemInstance,
		const FName SourceSlotId,
		const FName DestinationSlotId,
		const float RequestedMassKg,
		const FName ActionOwnerId)
	{
		// [v1.0.0] 테스트 Prepare·Commit·Rollback에 사용할 Transfer Request입니다.
		FCFInventoryTransferRequest TransferRequest;
		TransferRequest.TransactionId = FCFInventoryTransactionId::CreateNew();
		TransferRequest.ActionOwnerId = ActionOwnerId;
		TransferRequest.ItemInstanceId = ItemInstance.ItemHandle.ItemInstanceId;
		TransferRequest.SourceContainerId = Fixture.SourceContainerId;
		TransferRequest.SourceSlotId = SourceSlotId;
		TransferRequest.DestinationContainerId = Fixture.DestinationContainerId;
		TransferRequest.DestinationSlotId = DestinationSlotId;
		TransferRequest.RequestedMassKg = RequestedMassKg;
		return TransferRequest;
	}

	// [v1.0.0] Destination Container의 외부 해석 현재 질량 Snapshot 배열을 생성합니다.
	TArray<FCFInventoryContainerMassSnapshot> CreateDestinationMassSnapshots(
		const FCFInventoryTransferFixture& Fixture,
		const float DestinationOccupiedMassKg)
	{
		// [v1.0.0] Destination 질량 Capacity Query에 전달할 현재 질량 Snapshot입니다.
		FCFInventoryContainerMassSnapshot DestinationMassSnapshot;
		DestinationMassSnapshot.ContainerId = Fixture.DestinationContainerId;
		DestinationMassSnapshot.OccupiedMassKg = DestinationOccupiedMassKg;

		// [v1.0.0] Ledger Prepare 또는 Commit에 전달할 질량 Snapshot 배열입니다.
		TArray<FCFInventoryContainerMassSnapshot> MassSnapshots;
		MassSnapshots.Add(DestinationMassSnapshot);
		return MassSnapshots;
	}

	// [v1.0.0] 두 Container 집합이 Item·Slot·Capacity까지 같은지 반환합니다.
	bool AreContainerSetsEquivalent(
		const TArray<FCFInventoryContainerState>& FirstContainers,
		const TArray<FCFInventoryContainerState>& SecondContainers)
	{
		if (FirstContainers.Num() != SecondContainers.Num())
		{
			return false;
		}

		for (int32 ContainerIndex = 0; ContainerIndex < FirstContainers.Num(); ++ContainerIndex)
		{
			// [v1.0.0] 첫 Container 집합의 현재 비교 Container입니다.
			const FCFInventoryContainerState& FirstContainer = FirstContainers[ContainerIndex];

			// [v1.0.0] 두 번째 Container 집합의 현재 비교 Container입니다.
			const FCFInventoryContainerState& SecondContainer = SecondContainers[ContainerIndex];
			if (FirstContainer.ContainerRef.OwnerId != SecondContainer.ContainerRef.OwnerId
				|| FirstContainer.ContainerRef.ContainerId != SecondContainer.ContainerRef.ContainerId
				|| FirstContainer.ContainerRef.ContainerType != SecondContainer.ContainerRef.ContainerType
				|| FirstContainer.Capacity.MaximumSlotCount != SecondContainer.Capacity.MaximumSlotCount
				|| FirstContainer.Capacity.bUseMassLimit != SecondContainer.Capacity.bUseMassLimit
				|| !FMath::IsNearlyEqual(FirstContainer.Capacity.MaximumMassKg, SecondContainer.Capacity.MaximumMassKg)
				|| FirstContainer.Entries.Num() != SecondContainer.Entries.Num())
			{
				return false;
			}

			for (int32 EntryIndex = 0; EntryIndex < FirstContainer.Entries.Num(); ++EntryIndex)
			{
				// [v1.0.0] 첫 Container의 현재 비교 Entry입니다.
				const FCFInventoryContainerEntry& FirstEntry = FirstContainer.Entries[EntryIndex];

				// [v1.0.0] 두 번째 Container의 현재 비교 Entry입니다.
				const FCFInventoryContainerEntry& SecondEntry = SecondContainer.Entries[EntryIndex];
				if (FirstEntry.ContainerSlotId != SecondEntry.ContainerSlotId
					|| FirstEntry.ItemInstance.ItemHandle.ItemInstanceId != SecondEntry.ItemInstance.ItemHandle.ItemInstanceId
					|| FirstEntry.ItemInstance.ItemHandle.ItemDefinitionId != SecondEntry.ItemInstance.ItemHandle.ItemDefinitionId
					|| FirstEntry.ItemInstance.Quantity != SecondEntry.ItemInstance.Quantity)
				{
					return false;
				}
			}
		}

		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryReservationTest,
	"CarFight.Inventory.INV_P0_03.Reservation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryAtomicTransferTest,
	"CarFight.Inventory.INV_P0_03.AtomicTransfer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Item·Destination Capacity 이중 예약 거부와 멱등 취소·Rollback 정리를 검증합니다.
bool FCFInventoryReservationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Reservation 테스트에서 사용할 Item과 Container Fixture입니다.
	FCFInventoryTransferFixture Fixture;
	if (!BuildTransferFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] Reservation 수명과 Transaction terminal 상태를 소유할 순수 Ledger입니다.
	FCFInventoryTransferLedger TransferLedger;

	// [v1.0.0] Destination 현재 질량 0kg의 Prepare 입력 Snapshot입니다.
	const TArray<FCFInventoryContainerMassSnapshot> EmptyDestinationMassSnapshots = CreateDestinationMassSnapshots(Fixture, 0.0f);

	// [v1.0.0] 첫 Item 60kg을 RoofLeft Slot으로 예약할 Transaction 요청입니다.
	const FCFInventoryTransferRequest FirstRequest = CreateTransferRequest(
		Fixture,
		Fixture.FirstItemInstance,
		TEXT("CargoSlot_01"),
		TEXT("RoofLeft"),
		60.0f,
		TEXT("EquipAction_A"));

	// [v1.0.0] Prepare 전 Container 무변경을 검증할 Snapshot입니다.
	const TArray<FCFInventoryContainerState> ContainersBeforePrepare = Fixture.Containers;

	// [v1.0.0] Item과 Destination Slot·Capacity Reservation을 생성한 첫 Prepare 결과입니다.
	const FCFInventoryTransferResult FirstPrepareResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		FirstRequest);
	TestTrue(TEXT("첫 Prepare 성공"), FirstPrepareResult.IsSuccessful());
	TestEqual(TEXT("첫 Prepare 상태 Prepared"), FirstPrepareResult.TransferStatus, ECFInventoryTransferStatus::Prepared);
	TestTrue(TEXT("Item Reservation ID 유효"), FirstPrepareResult.ItemReservationId.IsValid());
	TestTrue(TEXT("Destination Reservation ID 유효"), FirstPrepareResult.DestinationReservationId.IsValid());
	TestEqual(TEXT("Active Reservation 2건"), TransferLedger.GetActiveReservationCount(), 2);
	TestTrue(TEXT("첫 Item 예약됨"), TransferLedger.IsItemReserved(FirstRequest.ItemInstanceId));
	TestTrue(TEXT("RoofLeft Slot 예약됨"), TransferLedger.IsDestinationSlotReserved(Fixture.DestinationContainerId, TEXT("RoofLeft")));
	TestTrue(TEXT("Prepare는 Container 무변경"), AreContainerSetsEquivalent(Fixture.Containers, ContainersBeforePrepare));

	// [v1.0.0] 같은 Transaction과 같은 요청을 다시 Prepare한 멱등 결과입니다.
	const FCFInventoryTransferResult RepeatedPrepareResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		FirstRequest);
	TestEqual(TEXT("반복 Prepare 상태 AlreadyPrepared"), RepeatedPrepareResult.TransferStatus, ECFInventoryTransferStatus::AlreadyPrepared);
	TestEqual(TEXT("반복 Prepare 후 Active Reservation 여전히 2건"), TransferLedger.GetActiveReservationCount(), 2);
	TestEqual(TEXT("반복 Prepare Item Reservation ID 동일"), RepeatedPrepareResult.ItemReservationId, FirstPrepareResult.ItemReservationId);
	TestEqual(TEXT("반복 Prepare Destination Reservation ID 동일"), RepeatedPrepareResult.DestinationReservationId, FirstPrepareResult.DestinationReservationId);

	// [v1.0.0] 이미 예약된 같은 Item을 다른 Transaction에서 사용하려는 요청입니다.
	const FCFInventoryTransferRequest DuplicateItemRequest = CreateTransferRequest(
		Fixture,
		Fixture.FirstItemInstance,
		TEXT("CargoSlot_01"),
		TEXT("RoofRight"),
		20.0f,
		TEXT("EquipAction_B"));
	const FCFInventoryTransferResult DuplicateItemResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		DuplicateItemRequest);
	TestEqual(TEXT("같은 Item 이중 예약 거부"), DuplicateItemResult.TransferStatus, ECFInventoryTransferStatus::ItemAlreadyReserved);

	// [v1.0.0] 다른 Item으로 이미 예약된 RoofLeft Slot을 사용하려는 요청입니다.
	const FCFInventoryTransferRequest DuplicateSlotRequest = CreateTransferRequest(
		Fixture,
		Fixture.SecondItemInstance,
		TEXT("CargoSlot_02"),
		TEXT("RoofLeft"),
		20.0f,
		TEXT("EquipAction_C"));
	const FCFInventoryTransferResult DuplicateSlotResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		DuplicateSlotRequest);
	TestEqual(TEXT("같은 Destination Slot 이중 예약 거부"), DuplicateSlotResult.TransferStatus, ECFInventoryTransferStatus::DestinationSlotReserved);

	// [v1.0.0] 기존 60kg 예약과 합쳐 100kg을 초과하는 두 번째 Item 50kg 요청입니다.
	const FCFInventoryTransferRequest OverMassRequest = CreateTransferRequest(
		Fixture,
		Fixture.SecondItemInstance,
		TEXT("CargoSlot_02"),
		TEXT("RoofRight"),
		50.0f,
		TEXT("EquipAction_D"));
	const FCFInventoryTransferResult OverMassResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		OverMassRequest);
	TestEqual(TEXT("Active Reservation 포함 Destination Mass Capacity 초과 거부"), OverMassResult.TransferStatus, ECFInventoryTransferStatus::DestinationCapacityExceeded);

		// [v1.0.0] Item Reservation ID를 취소 Token으로 사용해 같은 Transaction 전체를 취소한 결과입니다.
	const FCFInventoryCancelResult FirstCancelResult = TransferLedger.CancelReservation(FirstPrepareResult.ItemReservationId);
	TestTrue(TEXT("첫 Reservation 취소 성공"), FirstCancelResult.IsSuccessful());
	TestEqual(TEXT("첫 취소 상태 Cancelled"), FirstCancelResult.CancelState, ECFInventoryCancelState::Cancelled);
	TestEqual(TEXT("취소 Token Item Reservation 상태 Cancelled"), TransferLedger.GetReservationState(FirstPrepareResult.ItemReservationId), ECFInventoryReservationState::Cancelled);
	TestEqual(TEXT("짝 Destination Reservation 상태 Cancelled"), TransferLedger.GetReservationState(FirstPrepareResult.DestinationReservationId), ECFInventoryReservationState::Cancelled);
	TestEqual(TEXT("취소 후 Active Reservation 잔류 없음"), TransferLedger.GetActiveReservationCount(), 0);
	TestFalse(TEXT("취소 후 Item 예약 해제"), TransferLedger.IsItemReserved(FirstRequest.ItemInstanceId));
	TestFalse(TEXT("취소 후 RoofLeft Slot 예약 해제"), TransferLedger.IsDestinationSlotReserved(Fixture.DestinationContainerId, TEXT("RoofLeft")));
	TestEqual(TEXT("취소 후 Transaction RolledBack"), TransferLedger.GetTransactionState(FirstRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
	TestTrue(TEXT("Reservation 취소는 Container 무변경"), AreContainerSetsEquivalent(Fixture.Containers, ContainersBeforePrepare));

	// [v1.0.0] 같은 Reservation 취소 Token을 다시 호출한 멱등 결과입니다.
	const FCFInventoryCancelResult RepeatedCancelResult = TransferLedger.CancelReservation(FirstPrepareResult.ItemReservationId);
	TestTrue(TEXT("반복 Reservation 취소 멱등 성공"), RepeatedCancelResult.IsSuccessful());
	TestEqual(TEXT("반복 취소 상태 AlreadyCancelled"), RepeatedCancelResult.CancelState, ECFInventoryCancelState::AlreadyCancelled);
	TestEqual(TEXT("반복 취소 후 Active Reservation 없음"), TransferLedger.GetActiveReservationCount(), 0);

	// [v1.0.0] 취소가 이미 RolledBack으로 전환한 같은 Transaction을 Rollback한 멱등 결과입니다.
	const FCFInventoryTransferResult RollbackAfterCancelResult = TransferLedger.RollbackTransfer(FirstRequest.TransactionId);
	TestEqual(TEXT("취소 후 Rollback 상태 AlreadyRolledBack"), RollbackAfterCancelResult.TransferStatus, ECFInventoryTransferStatus::AlreadyRolledBack);
	TestTrue(TEXT("취소 후 반복 Rollback도 Container 무변경"), AreContainerSetsEquivalent(Fixture.Containers, ContainersBeforePrepare));

	// [v1.0.0] 취소로 Capacity가 해제된 뒤 두 번째 Item 50kg을 예약하는 결과입니다.
	const FCFInventoryTransferResult PrepareAfterCancelResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		OverMassRequest);
	TestEqual(TEXT("취소 후 해제된 Capacity 재예약 가능"), PrepareAfterCancelResult.TransferStatus, ECFInventoryTransferStatus::Prepared);
	TestEqual(TEXT("재예약 후 Active Reservation 2건"), TransferLedger.GetActiveReservationCount(), 2);

	// [v1.0.0] 새 Prepared Transaction을 명시적으로 Rollback한 결과입니다.
	const FCFInventoryTransferResult ExplicitRollbackResult = TransferLedger.RollbackTransfer(OverMassRequest.TransactionId);
	TestTrue(TEXT("명시적 Rollback 성공"), ExplicitRollbackResult.IsSuccessful());
	TestEqual(TEXT("명시적 Rollback 상태 RolledBack"), ExplicitRollbackResult.TransferStatus, ECFInventoryTransferStatus::RolledBack);
	TestEqual(TEXT("명시적 Rollback 후 Active Reservation 없음"), TransferLedger.GetActiveReservationCount(), 0);
	TestTrue(TEXT("명시적 Rollback도 Container 무변경"), AreContainerSetsEquivalent(Fixture.Containers, ContainersBeforePrepare));

	// [v1.0.0] 명시적으로 Rollback한 Transaction을 다시 Rollback한 멱등 결과입니다.
	const FCFInventoryTransferResult RepeatedRollbackResult = TransferLedger.RollbackTransfer(OverMassRequest.TransactionId);
	TestEqual(TEXT("반복 Rollback 상태 AlreadyRolledBack"), RepeatedRollbackResult.TransferStatus, ECFInventoryTransferStatus::AlreadyRolledBack);
	TestEqual(TEXT("반복 Rollback 후 Active Reservation 없음"), TransferLedger.GetActiveReservationCount(), 0);

	return true;

}

// [v1.0.0] 원자 Commit, Commit 실패 무변경, 명시적 Rollback과 중복 Commit 방지를 검증합니다.
bool FCFInventoryAtomicTransferTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Atomic Transfer 테스트에서 사용할 Item과 Container Fixture입니다.
	FCFInventoryTransferFixture Fixture;
	if (!BuildTransferFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] Atomic Transfer 상태를 소유할 순수 Ledger입니다.
	FCFInventoryTransferLedger TransferLedger;

	// [v1.0.0] 첫 Item 40kg을 RoofLeft로 이동할 Transaction 요청입니다.
	const FCFInventoryTransferRequest FirstRequest = CreateTransferRequest(
		Fixture,
		Fixture.FirstItemInstance,
		TEXT("CargoSlot_01"),
		TEXT("RoofLeft"),
		40.0f,
		TEXT("EquipAction_Commit"));

	// [v1.0.0] 첫 Prepare와 Commit에서 사용할 Destination 현재 질량 0kg Snapshot입니다.
	const TArray<FCFInventoryContainerMassSnapshot> EmptyDestinationMassSnapshots = CreateDestinationMassSnapshots(Fixture, 0.0f);
	const FCFInventoryTransferResult FirstPrepareResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		FirstRequest);
	TestEqual(TEXT("Atomic Transfer Prepare 성공"), FirstPrepareResult.TransferStatus, ECFInventoryTransferStatus::Prepared);

	// [v1.0.0] Source 제거와 Destination 추가를 한 번에 확정한 Commit 결과입니다.
	const FCFInventoryTransferResult FirstCommitResult = TransferLedger.CommitTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		FirstRequest.TransactionId);
	TestTrue(TEXT("첫 Commit 성공"), FirstCommitResult.IsSuccessful());
	TestEqual(TEXT("첫 Commit 상태 Committed"), FirstCommitResult.TransferStatus, ECFInventoryTransferStatus::Committed);
	TestEqual(TEXT("Commit 후 Transaction 상태 Committed"), TransferLedger.GetTransactionState(FirstRequest.TransactionId), ECFInventoryTransactionState::Committed);
	TestEqual(TEXT("Commit 후 Item Reservation Consumed"), TransferLedger.GetReservationState(FirstPrepareResult.ItemReservationId), ECFInventoryReservationState::Consumed);
	TestEqual(TEXT("Commit 후 Destination Reservation Consumed"), TransferLedger.GetReservationState(FirstPrepareResult.DestinationReservationId), ECFInventoryReservationState::Consumed);
	TestEqual(TEXT("Commit 후 Active Reservation 없음"), TransferLedger.GetActiveReservationCount(), 0);

	// [v1.0.0] Commit 후 첫 Item의 전체 Container 단일 위치 조회 결과입니다.
	const FCFInventoryLocationResult FirstCommittedLocation = FCFInventoryAccessQuery::QueryItemLocation(
		Fixture.Containers,
		Fixture.FirstItemInstance.ItemHandle.ItemInstanceId);
	TestTrue(TEXT("Commit Item 단일 위치 발견"), FirstCommittedLocation.IsFound());
	TestEqual(TEXT("Commit Item Destination Container 이동"), FirstCommittedLocation.ItemLocation.ContainerRef.ContainerId, Fixture.DestinationContainerId);
	TestEqual(TEXT("Commit Item Destination Slot 이동"), FirstCommittedLocation.ItemLocation.ContainerSlotId, FName(TEXT("RoofLeft")));
	TestEqual(TEXT("Commit 후 Source Entry 1개"), Fixture.Containers[0].Entries.Num(), 1);
	TestEqual(TEXT("Commit 후 Destination Entry 1개"), Fixture.Containers[1].Entries.Num(), 1);

	// [v1.0.0] 중복 Commit 전 Container 상태 Snapshot입니다.
	const TArray<FCFInventoryContainerState> ContainersBeforeDuplicateCommit = Fixture.Containers;

	// [v1.0.0] 같은 Transaction ID를 다시 Commit한 멱등 중복 방지 결과입니다.
	const FCFInventoryTransferResult DuplicateCommitResult = TransferLedger.CommitTransfer(
		Fixture.Containers,
		EmptyDestinationMassSnapshots,
		FirstRequest.TransactionId);
	TestEqual(TEXT("중복 Commit 상태 AlreadyCommitted"), DuplicateCommitResult.TransferStatus, ECFInventoryTransferStatus::AlreadyCommitted);
	TestTrue(TEXT("중복 Commit Container 무변경"), AreContainerSetsEquivalent(Fixture.Containers, ContainersBeforeDuplicateCommit));

	// [v1.0.0] 중복 Commit 후에도 Item Instance가 정확히 한 위치에만 존재하는 조회 결과입니다.
	const FCFInventoryLocationResult DuplicateCommitLocation = FCFInventoryAccessQuery::QueryItemLocation(
		Fixture.Containers,
		Fixture.FirstItemInstance.ItemHandle.ItemInstanceId);
	TestTrue(TEXT("중복 Commit 후 Item 단일 위치 유지"), DuplicateCommitLocation.IsFound());
	TestEqual(TEXT("중복 Commit 후 발견 위치 수 1"), DuplicateCommitLocation.MatchingLocationCount, 1);

	// [v1.0.0] Commit에서 Consumed된 Reservation을 취소하려는 결과입니다.
	const FCFInventoryCancelResult ConsumedCancelResult = TransferLedger.CancelReservation(FirstPrepareResult.ItemReservationId);
	TestEqual(TEXT("Consumed Reservation 취소 거부"), ConsumedCancelResult.CancelState, ECFInventoryCancelState::CannotCancelConsumed);

	// [v1.0.0] 두 번째 Item 50kg을 RoofRight로 Prepare할 Transaction 요청입니다.
	const FCFInventoryTransferRequest SecondRequest = CreateTransferRequest(
		Fixture,
		Fixture.SecondItemInstance,
		TEXT("CargoSlot_02"),
		TEXT("RoofRight"),
		50.0f,
		TEXT("EquipAction_FailedCommit"));

	// [v1.0.0] 첫 Commit Item 40kg이 반영된 Destination 현재 질량 Snapshot입니다.
	const TArray<FCFInventoryContainerMassSnapshot> PreparedDestinationMassSnapshots = CreateDestinationMassSnapshots(Fixture, 40.0f);
	const FCFInventoryTransferResult SecondPrepareResult = TransferLedger.PrepareTransfer(
		Fixture.Containers,
		PreparedDestinationMassSnapshots,
		SecondRequest);
	TestEqual(TEXT("두 번째 Transfer Prepare 성공"), SecondPrepareResult.TransferStatus, ECFInventoryTransferStatus::Prepared);
	TestEqual(TEXT("두 번째 Prepare 후 Active Reservation 2건"), TransferLedger.GetActiveReservationCount(), 2);

	// [v1.0.0] 외부 상태 변화로 Destination 현재 질량이 70kg이 된 Commit 재검증 Snapshot입니다.
	const TArray<FCFInventoryContainerMassSnapshot> OverCapacityCommitMassSnapshots = CreateDestinationMassSnapshots(Fixture, 70.0f);

	// [v1.0.0] 실패 Commit이 Container를 전혀 변경하지 않았는지 비교할 Snapshot입니다.
	const TArray<FCFInventoryContainerState> ContainersBeforeFailedCommit = Fixture.Containers;
	const FCFInventoryTransferResult FailedCommitResult = TransferLedger.CommitTransfer(
		Fixture.Containers,
		OverCapacityCommitMassSnapshots,
		SecondRequest.TransactionId);
	TestEqual(TEXT("Commit 재검증 Mass Capacity 초과 거부"), FailedCommitResult.TransferStatus, ECFInventoryTransferStatus::DestinationCapacityExceeded);
	TestTrue(TEXT("실패 Commit Source·Destination 무변경"), AreContainerSetsEquivalent(Fixture.Containers, ContainersBeforeFailedCommit));
	TestEqual(TEXT("실패 Commit 후 Transaction Prepared 유지"), TransferLedger.GetTransactionState(SecondRequest.TransactionId), ECFInventoryTransactionState::Prepared);
	TestEqual(TEXT("실패 Commit 후 Reservation 유지"), TransferLedger.GetActiveReservationCount(), 2);

	// [v1.0.0] 실패 Commit의 Prepared Transaction을 명시적으로 Rollback한 결과입니다.
	const FCFInventoryTransferResult FailedCommitRollbackResult = TransferLedger.RollbackTransfer(SecondRequest.TransactionId);
	TestEqual(TEXT("실패 Commit Transaction Rollback 성공"), FailedCommitRollbackResult.TransferStatus, ECFInventoryTransferStatus::RolledBack);
	TestEqual(TEXT("Rollback 후 Active Reservation 없음"), TransferLedger.GetActiveReservationCount(), 0);
	TestTrue(TEXT("Rollback 후 Container 계속 무변경"), AreContainerSetsEquivalent(Fixture.Containers, ContainersBeforeFailedCommit));

	// [v1.0.0] 이미 Commit 완료된 첫 Transaction은 Rollback할 수 없는 terminal 결과입니다.
	const FCFInventoryTransferResult CommittedRollbackResult = TransferLedger.RollbackTransfer(FirstRequest.TransactionId);
	TestEqual(TEXT("Committed Transaction Rollback 거부"), CommittedRollbackResult.TransferStatus, ECFInventoryTransferStatus::CannotRollbackCommitted);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
