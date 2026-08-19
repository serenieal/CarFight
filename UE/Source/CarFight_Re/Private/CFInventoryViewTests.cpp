// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-14
// Description: CF-FQ-035 INV-P0-05 Inventory ViewData·ChangeSet Pawn 없는 자동화 테스트
// Scope: 현재 차량 Container의 Definition 표시·예약·접근·호환 Hint와 Prepare/Commit 전후 의미 Snapshot Diff를 검증합니다.
// Changelog:
// - v1.0.0: ViewData Snapshot과 Reservation/Location/Container ChangeSet 테스트를 최초 추가.
// Migration:
// - Transient Inventory Definition과 순수 Container·TransferLedger만 사용하며 Pawn, World, Runtime Component와 Unreal Asset을 생성하거나 수정하지 않습니다.
// - CompatibilityHint는 테스트가 외부 Fitting 결과처럼 직접 공급하며 Inventory가 Mount 호환성을 계산하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFInventoryViewData.h"

#include "CFEquipmentPresetData.h"
#include "CFVehicleDefenseData.h"

#include "Misc/AutomationTest.h"

namespace
{
	/** INV-P0-05 ViewData와 의미 Diff가 공유할 Transient Fixture입니다. */
	struct FCFInventoryViewFixture
	{
		// [v1.0.0] 현재 차량 Owner ID입니다.
		FCFInventoryOwnerId CurrentOwnerId;

		// [v1.0.0] 현재 차량 Cargo Container ID입니다.
		FCFInventoryContainerId CargoContainerId;

		// [v1.0.0] 현재 차량 MountedEquipment Container ID입니다.
		FCFInventoryContainerId MountedContainerId;

		// [v1.0.0] 다른 차량 Owner ID입니다.
		FCFInventoryOwnerId OtherOwnerId;

		// [v1.0.0] Cargo에서 Mounted로 이동할 실제 Equipment Item입니다.
		FCFInventoryItemInstance EquipmentItem;

		// [v1.0.0] 현재 Cargo에 남아 있을 실제 Defense Item입니다.
		FCFInventoryItemInstance DefenseItem;

		// [v1.0.0] 다른 Owner의 Snapshot에 노출되면 안 되는 실제 Equipment Item입니다.
		FCFInventoryItemInstance OtherOwnerItem;

		// [v1.0.0] UI 표시 이름과 Equipment 도메인을 제공할 Definition입니다.
		UCFEquipmentItemData* EquipmentDefinition = nullptr;

		// [v1.0.0] UI 표시 이름과 Defense 도메인을 제공할 Definition입니다.
		UCFDefenseItemData* DefenseDefinition = nullptr;

		// [v1.0.0] Definition 계약을 완성할 Transient EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] Definition 계약을 완성할 Transient VehicleDefenseData입니다.
		UCFVehicleDefenseData* DefenseData = nullptr;

		// [v1.0.0] 현재·다른 차량의 전체 Container 소유 상태입니다.
		TArray<FCFInventoryContainerState> Containers;

		// [v1.0.0] ItemDefinitionId를 UI Definition으로 해석할 Registry 입력입니다.
		TArray<UCFInventoryItemData*> ItemDefinitions;

		// [v1.0.0] Reservation과 실제 Atomic Commit 수명을 소유할 순수 Ledger입니다.
		FCFInventoryTransferLedger TransferLedger;

		// [v1.0.0] 현재 차량 Cargo·Mounted 접근이 모두 가능한 Context입니다.
		FCFInventoryAccessContext AccessContext;
	};

	// [v1.0.0] Container에 실제 Item Instance 한 건을 지정 Slot으로 추가합니다.
	void AddViewTestEntry(
		FCFInventoryContainerState& Container,
		const FCFInventoryItemInstance& ItemInstance,
		const FName SlotId)
	{
		// [v1.0.0] 추가할 실제 Item 소유 Entry입니다.
		FCFInventoryContainerEntry ContainerEntry;
		ContainerEntry.ItemInstance = ItemInstance;
		ContainerEntry.ContainerSlotId = SlotId;
		Container.Entries.Add(ContainerEntry);
	}

	// [v1.0.0] ViewData/Reservation/Commit 검증에 사용할 정상 Inventory Fixture를 생성합니다.
	bool BuildViewFixture(FAutomationTestBase& Test, FCFInventoryViewFixture& OutFixture)
	{
		OutFixture.CurrentOwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.OtherOwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.CargoContainerId = FCFInventoryContainerId::CreateNew();
		OutFixture.MountedContainerId = FCFInventoryContainerId::CreateNew();

		OutFixture.EquipmentPresetData = NewObject<UCFEquipmentPresetData>();
		OutFixture.EquipmentPresetData->DisplayName = FText::FromString(TEXT("테스트 터렛"));
		OutFixture.DefenseData = NewObject<UCFVehicleDefenseData>();
		OutFixture.DefenseData->DefenseId = TEXT("ViewTestDefense");

		OutFixture.EquipmentDefinition = NewObject<UCFEquipmentItemData>();
		OutFixture.EquipmentDefinition->ItemDefinitionId = TEXT("ViewEquipmentItem");
		OutFixture.EquipmentDefinition->DisplayName = FText::FromString(TEXT("화물 터렛"));
		OutFixture.EquipmentDefinition->EquipmentPresetData = OutFixture.EquipmentPresetData;
		OutFixture.DefenseDefinition = NewObject<UCFDefenseItemData>();
		OutFixture.DefenseDefinition->ItemDefinitionId = TEXT("ViewDefenseItem");
		OutFixture.DefenseDefinition->DisplayName = FText::FromString(TEXT("장갑 패키지"));
		OutFixture.DefenseDefinition->VehicleDefenseData = OutFixture.DefenseData;

		OutFixture.EquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentDefinition);
		OutFixture.DefenseItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.DefenseDefinition);
		OutFixture.OtherOwnerItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentDefinition);
		if (!Test.TestTrue(TEXT("Equipment Item 생성"), OutFixture.EquipmentItem.IsValidForDefinition(OutFixture.EquipmentDefinition))
			|| !Test.TestTrue(TEXT("Defense Item 생성"), OutFixture.DefenseItem.IsValidForDefinition(OutFixture.DefenseDefinition))
			|| !Test.TestTrue(TEXT("Other Owner Item 생성"), OutFixture.OtherOwnerItem.IsValidForDefinition(OutFixture.EquipmentDefinition)))
		{
			return false;
		}

		// [v1.0.0] 입력 배열 순서 독립성 검증을 위해 Item Entry를 Slot 이름 역순으로 넣을 현재 차량 Cargo입니다.
		FCFInventoryContainerState CargoContainer;
		CargoContainer.ContainerRef.OwnerId = OutFixture.CurrentOwnerId;
		CargoContainer.ContainerRef.ContainerId = OutFixture.CargoContainerId;
		CargoContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		CargoContainer.Capacity.MaximumSlotCount = 4;
		AddViewTestEntry(CargoContainer, OutFixture.DefenseItem, TEXT("Cargo_B"));
		AddViewTestEntry(CargoContainer, OutFixture.EquipmentItem, TEXT("Cargo_A"));

		// [v1.0.0] Equipment Item의 이동 목적지로 사용할 현재 차량 MountedEquipment입니다.
		FCFInventoryContainerState MountedContainer;
		MountedContainer.ContainerRef.OwnerId = OutFixture.CurrentOwnerId;
		MountedContainer.ContainerRef.ContainerId = OutFixture.MountedContainerId;
		MountedContainer.ContainerRef.ContainerType = ECFInventoryContainerType::MountedEquipment;
		MountedContainer.Capacity.MaximumSlotCount = 2;

		// [v1.0.0] 현재 차량 Snapshot에 노출되면 안 되는 다른 Owner Cargo입니다.
		FCFInventoryContainerState OtherCargoContainer;
		OtherCargoContainer.ContainerRef.OwnerId = OutFixture.OtherOwnerId;
		OtherCargoContainer.ContainerRef.ContainerId = FCFInventoryContainerId::CreateNew();
		OtherCargoContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		OtherCargoContainer.Capacity.MaximumSlotCount = 2;
		AddViewTestEntry(OtherCargoContainer, OutFixture.OtherOwnerItem, TEXT("Other_A"));

		OutFixture.Containers.Add(OtherCargoContainer);
		OutFixture.Containers.Add(MountedContainer);
		OutFixture.Containers.Add(CargoContainer);
		OutFixture.ItemDefinitions.Add(OutFixture.DefenseDefinition);
		OutFixture.ItemDefinitions.Add(OutFixture.EquipmentDefinition);
		OutFixture.AccessContext.CurrentVehicleOwnerId = OutFixture.CurrentOwnerId;

		// [v1.0.0] Fixture 전체 Container 불변식 오류 목록입니다.
		TArray<FText> ValidationErrors;
		return Test.TestTrue(TEXT("View Fixture Container 집합 유효"), FCFInventoryAccessQuery::ValidateContainerSet(OutFixture.Containers, ValidationErrors));
	}

	// [v1.0.0] 지정 ChangeType과 ItemInstanceId를 가진 의미 변화가 존재하는지 반환합니다.
	bool HasItemChange(
		const FCFInventoryViewChangeSet& ChangeSet,
		const ECFInventoryViewChangeType ChangeType,
		const FCFItemInstanceId& ItemInstanceId)
	{
		return ChangeSet.Changes.ContainsByPredicate(
			[ChangeType, &ItemInstanceId](const FCFInventoryViewChange& ViewChange)
			{
				return ViewChange.ChangeType == ChangeType && ViewChange.ItemInstanceId == ItemInstanceId;
			});
	}

	// [v1.0.0] 지정 Container ID에 대한 ContainerChanged가 존재하는지 반환합니다.
	bool HasContainerChange(
		const FCFInventoryViewChangeSet& ChangeSet,
		const FCFInventoryContainerId& ContainerId)
	{
		return ChangeSet.Changes.ContainsByPredicate(
			[&ContainerId](const FCFInventoryViewChange& ViewChange)
			{
				return ViewChange.ChangeType == ECFInventoryViewChangeType::ContainerChanged
					&& ViewChange.ContainerId == ContainerId;
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryViewDataTest,
	"CarFight.Inventory.INV_P0_05.ViewData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 현재 차량 읽기 전용 Item/Container 표시, Definition, 외부 Compatibility Hint와 Owner 격리를 검증합니다.
bool FCFInventoryViewDataTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 테스트가 공유할 Transient Inventory Fixture입니다.
	FCFInventoryViewFixture Fixture;
	if (!BuildViewFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] Fitting이 Equipment Item에 제공했다고 가정할 외부 호환성 힌트입니다.
	FCFInventoryCompatibilityHint EquipmentCompatibilityHint;
	EquipmentCompatibilityHint.ItemInstanceId = Fixture.EquipmentItem.ItemHandle.ItemInstanceId;
	EquipmentCompatibilityHint.CompatibilityHint = FText::FromString(TEXT("RoofTurret 호환"));
	// [v1.0.0] Snapshot Builder에 공급할 외부 Fitting Hint 목록입니다.
	TArray<FCFInventoryCompatibilityHint> CompatibilityHints;
	CompatibilityHints.Add(EquipmentCompatibilityHint);

	// [v1.0.0] 입력 배열 순서와 무관하게 생성된 초기 읽기 전용 Snapshot 결과입니다.
	const FCFInventoryViewBuildResult InitialResult = FCFInventoryViewBuilder::BuildSnapshot(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		Fixture.AccessContext,
		CompatibilityHints);
	if (!TestTrue(TEXT("초기 ViewData Snapshot 성공"), InitialResult.IsSuccessful()))
	{
		return false;
	}

	TestEqual(TEXT("현재 차량 Container 2개"), InitialResult.Snapshot.Containers.Num(), 2);
	TestEqual(TEXT("Container 첫 순서 VehicleCargo"), InitialResult.Snapshot.Containers[0].ContainerRef.ContainerType, ECFInventoryContainerType::VehicleCargo);
	TestEqual(TEXT("Container 둘째 순서 MountedEquipment"), InitialResult.Snapshot.Containers[1].ContainerRef.ContainerType, ECFInventoryContainerType::MountedEquipment);
	TestEqual(TEXT("현재 차량 Item 2개만 노출"), InitialResult.Snapshot.Items.Num(), 2);
	TestEqual(TEXT("Cargo_A가 첫 Item"), InitialResult.Snapshot.Items[0].ContainerSlotId, FName(TEXT("Cargo_A")));
	TestTrue(TEXT("첫 Item Equipment Instance"), InitialResult.Snapshot.Items[0].ItemInstanceId == Fixture.EquipmentItem.ItemHandle.ItemInstanceId);
	TestEqual(TEXT("Equipment 표시 이름"), InitialResult.Snapshot.Items[0].DisplayName.ToString(), FString(TEXT("화물 터렛")));
	TestEqual(TEXT("Equipment 도메인"), InitialResult.Snapshot.Items[0].ItemDomain, ECFInventoryItemDomain::Equipment);
	TestFalse(TEXT("초기 Equipment 예약 아님"), InitialResult.Snapshot.Items[0].bIsReserved);
	TestTrue(TEXT("Equipment 접근 가능"), InitialResult.Snapshot.Items[0].bIsAccessible);
	TestEqual(TEXT("외부 Fitting CompatibilityHint 합성"), InitialResult.Snapshot.Items[0].CompatibilityHint.ToString(), FString(TEXT("RoofTurret 호환")));
	TestEqual(TEXT("Defense 표시 이름"), InitialResult.Snapshot.Items[1].DisplayName.ToString(), FString(TEXT("장갑 패키지")));
	TestEqual(TEXT("Defense 도메인"), InitialResult.Snapshot.Items[1].ItemDomain, ECFInventoryItemDomain::Defense);
	TestTrue(TEXT("Defense CompatibilityHint 없음"), InitialResult.Snapshot.Items[1].CompatibilityHint.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryViewChangeSetTest,
	"CarFight.Inventory.INV_P0_05.ChangeSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Prepare Reservation과 Atomic Commit이 Container를 직접 읽지 않는 의미 ChangeSet으로 변환되는지 검증합니다.
bool FCFInventoryViewChangeSetTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Reservation과 Commit 전후 Snapshot을 만들 Transient Fixture입니다.
	FCFInventoryViewFixture Fixture;
	if (!BuildViewFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] 어떤 Reservation도 없는 기준 Snapshot 결과입니다.
	const FCFInventoryViewBuildResult InitialResult = FCFInventoryViewBuilder::BuildSnapshot(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		Fixture.AccessContext);
	if (!TestTrue(TEXT("ChangeSet 초기 Snapshot 성공"), InitialResult.IsSuccessful()))
	{
		return false;
	}

	// [v1.0.0] Equipment Item을 Cargo_A에서 Mounted RoofTurret로 이동하기 위한 실제 Inventory Transfer 요청입니다.
	FCFInventoryTransferRequest TransferRequest;
	TransferRequest.TransactionId = FCFInventoryTransactionId::CreateNew();
	TransferRequest.ActionOwnerId = TEXT("INV_P0_05_ViewChange");
	TransferRequest.ItemInstanceId = Fixture.EquipmentItem.ItemHandle.ItemInstanceId;
	TransferRequest.SourceContainerId = Fixture.CargoContainerId;
	TransferRequest.SourceSlotId = TEXT("Cargo_A");
	TransferRequest.DestinationContainerId = Fixture.MountedContainerId;
	TransferRequest.DestinationSlotId = TEXT("RoofTurret");

	// [v1.0.0] Container는 변경하지 않고 Item과 Destination만 예약할 Prepare 결과입니다.
	const FCFInventoryTransferResult PrepareResult = Fixture.TransferLedger.PrepareTransfer(
		Fixture.Containers,
		TArray<FCFInventoryContainerMassSnapshot>(),
		TransferRequest);
	TestEqual(TEXT("ViewChange Prepare 성공"), PrepareResult.TransferStatus, ECFInventoryTransferStatus::Prepared);

	// [v1.0.0] Prepare 직후 Reservation만 달라진 Snapshot 결과입니다.
	const FCFInventoryViewBuildResult ReservedResult = FCFInventoryViewBuilder::BuildSnapshot(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		Fixture.AccessContext);
	if (!TestTrue(TEXT("Reservation Snapshot 성공"), ReservedResult.IsSuccessful()))
	{
		return false;
	}

	// [v1.0.0] Container 무변경 Prepare가 만들어야 할 Reservation 의미 Diff입니다.
	const FCFInventoryViewChangeSet ReservationChangeSet = FCFInventoryViewBuilder::DiffSnapshots(
		InitialResult.Snapshot,
		ReservedResult.Snapshot);
	TestTrue(TEXT("Reservation Change 존재"), ReservationChangeSet.HasChanges());
	TestTrue(
		TEXT("Equipment Item ReservationChanged"),
		HasItemChange(
			ReservationChangeSet,
			ECFInventoryViewChangeType::ItemReservationChanged,
			Fixture.EquipmentItem.ItemHandle.ItemInstanceId));
	TestFalse(
		TEXT("Prepare 중 Item LocationChanged 없음"),
		HasItemChange(
			ReservationChangeSet,
			ECFInventoryViewChangeType::ItemLocationChanged,
			Fixture.EquipmentItem.ItemHandle.ItemInstanceId));
	TestFalse(TEXT("Prepare 중 Cargo ContainerChanged 없음"), HasContainerChange(ReservationChangeSet, Fixture.CargoContainerId));
	TestFalse(TEXT("Prepare 중 Mounted ContainerChanged 없음"), HasContainerChange(ReservationChangeSet, Fixture.MountedContainerId));

	// [v1.0.0] 질량 Capacity를 사용하지 않는 두 Container의 실제 Atomic Commit 결과입니다.
	const FCFInventoryTransferResult CommitResult = Fixture.TransferLedger.CommitTransfer(
		Fixture.Containers,
		TArray<FCFInventoryContainerMassSnapshot>(),
		TransferRequest.TransactionId);
	TestEqual(TEXT("ViewChange Commit 성공"), CommitResult.TransferStatus, ECFInventoryTransferStatus::Committed);

	// [v1.0.0] Commit 후 실제 소유 위치와 Consumed Reservation을 읽은 Snapshot입니다.
	const FCFInventoryViewBuildResult CommittedResult = FCFInventoryViewBuilder::BuildSnapshot(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		Fixture.AccessContext);
	if (!TestTrue(TEXT("Commit Snapshot 성공"), CommittedResult.IsSuccessful()))
	{
		return false;
	}

	// [v1.0.0] Reserved→Committed 전환이 만들어야 할 위치·예약·Container 의미 Diff입니다.
	const FCFInventoryViewChangeSet CommitChangeSet = FCFInventoryViewBuilder::DiffSnapshots(
		ReservedResult.Snapshot,
		CommittedResult.Snapshot);
	TestTrue(
		TEXT("Commit 후 Equipment LocationChanged"),
		HasItemChange(
			CommitChangeSet,
			ECFInventoryViewChangeType::ItemLocationChanged,
			Fixture.EquipmentItem.ItemHandle.ItemInstanceId));
	TestTrue(
		TEXT("Commit 후 Equipment ReservationChanged"),
		HasItemChange(
			CommitChangeSet,
			ECFInventoryViewChangeType::ItemReservationChanged,
			Fixture.EquipmentItem.ItemHandle.ItemInstanceId));
	TestTrue(TEXT("Commit 후 Cargo ContainerChanged"), HasContainerChange(CommitChangeSet, Fixture.CargoContainerId));
	TestTrue(TEXT("Commit 후 Mounted ContainerChanged"), HasContainerChange(CommitChangeSet, Fixture.MountedContainerId));

	// [v1.0.0] Commit 후 Equipment Item의 최종 ViewData 행입니다.
	const FCFInventoryItemViewData* CommittedEquipmentItem = CommittedResult.Snapshot.Items.FindByPredicate(
		[&Fixture](const FCFInventoryItemViewData& ItemViewData)
		{
			return ItemViewData.ItemInstanceId == Fixture.EquipmentItem.ItemHandle.ItemInstanceId;
		});
	if (!TestNotNull(TEXT("Commit 후 Equipment Item ViewData"), CommittedEquipmentItem))
	{
		return false;
	}
	TestEqual(TEXT("Commit 후 Mounted Container"), CommittedEquipmentItem->ContainerRef.ContainerType, ECFInventoryContainerType::MountedEquipment);
	TestEqual(TEXT("Commit 후 RoofTurret Slot"), CommittedEquipmentItem->ContainerSlotId, FName(TEXT("RoofTurret")));
	TestFalse(TEXT("Commit 후 Active Reservation 해제"), CommittedEquipmentItem->bIsReserved);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
