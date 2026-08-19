// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-034 FFIT-P0-02 Timed Action·Reservation Pawn 없는 자동화 테스트
// Scope: Start Reservation, 진행률, Completing handoff, Permission/외부 취소, Reservation 유실과 중복 Action 소유 거부를 검증합니다.
// Changelog:
// - v1.0.0: FFIT-P0-02 Timed Action·Reservation 회귀를 최초 추가.
// Migration:
// - Runtime Apply·Inventory Commit은 호출하지 않습니다. Completing 상태에서도 Item 위치는 변하지 않고 Prepared Transaction만 유지해야 합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFieldFitAction.h"

#include "CFEquipmentPresetData.h"
#include "CFInventoryItemData.h"
#include "Misc/AutomationTest.h"

namespace
{
	/** Timed Action 테스트가 사용할 단일 Cargo→Mounted Inventory 상태입니다. */
	struct FCFFieldFitActionFixture
	{
		// [v1.0.0] 현재 차량 Inventory Owner ID입니다.
		FCFInventoryOwnerId OwnerId;

		// [v1.0.0] Source VehicleCargo ID입니다.
		FCFInventoryContainerId CargoContainerId;

		// [v1.0.0] Destination MountedEquipment ID입니다.
		FCFInventoryContainerId MountedContainerId;

		// [v1.0.0] 실제 Cargo Item Instance입니다.
		FCFInventoryItemInstance EquipmentItem;

		// [v1.0.0] Source/Destination Container 배열입니다.
		TArray<FCFInventoryContainerState> Containers;

		// [v1.0.0] Timed Action Reservation을 소유할 Ledger입니다.
		FCFInventoryTransferLedger TransferLedger;

		// [v1.0.0] Timed Action이 Prepare할 Inventory Transfer입니다.
		FCFInventoryTransferRequest TransferRequest;
	};

	// [v1.0.0] 모든 Permission blocker가 없는 정상 FFIT-P0-01 결과를 생성합니다.
	FCFFieldFitPermissionResult BuildAllowedActionPermission()
	{
		// [v1.0.0] Action 상태 머신이 M1을 재계산하지 않고 소비할 허용 결과입니다.
		FCFFieldFitPermissionResult PermissionResult;
		PermissionResult.bCanStart = true;
		return PermissionResult;
	}

	// [v1.0.0] 단일 구조화 blocker를 가진 Permission 거부 결과를 생성합니다.
	FCFFieldFitPermissionResult BuildBlockedActionPermission(
		const ECFFieldFitBlockerSource Source,
		const ECFFieldFitBlockReason Reason)
	{
		// [v1.0.0] 진행 중 조건 변화 취소를 재현할 Permission 결과입니다.
		FCFFieldFitPermissionResult PermissionResult;
		PermissionResult.bCanStart = false;
		// [v1.0.0] 취소 원인으로 보존할 단일 blocker입니다.
		FCFFieldFitBlocker Blocker;
		Blocker.Source = Source;
		Blocker.Reason = Reason;
		PermissionResult.Blockers.Add(Blocker);
		return PermissionResult;
	}

	// [v1.0.0] Timed Action 테스트용 정상 Inventory Fixture를 생성합니다.
	bool BuildActionFixture(FAutomationTestBase& Test, FCFFieldFitActionFixture& OutFixture)
	{
		OutFixture.OwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.CargoContainerId = FCFInventoryContainerId::CreateNew();
		OutFixture.MountedContainerId = FCFInventoryContainerId::CreateNew();

		// [v1.0.0] 실제 ItemInstance 생성을 위한 강타입 Equipment Definition입니다.
		UCFEquipmentItemData* EquipmentDefinition = NewObject<UCFEquipmentItemData>();
		EquipmentDefinition->ItemDefinitionId = TEXT("FieldFitActionItem");
		EquipmentDefinition->DisplayName = FText::FromString(TEXT("시간 장착 테스트 아이템"));
		// [v1.0.0] Definition 강타입 계약을 만족할 최소 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(EquipmentDefinition);
		EquipmentPresetData->EquipmentId = TEXT("FieldFitActionPreset");
		EquipmentDefinition->EquipmentPresetData = EquipmentPresetData;
		OutFixture.EquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(EquipmentDefinition);
		if (!Test.TestTrue(TEXT("Timed Action Item 유효"), OutFixture.EquipmentItem.IsValidForDefinition(EquipmentDefinition)))
		{
			return false;
		}

		// [v1.0.0] Item이 Action 완료 전까지 계속 머물러야 할 VehicleCargo입니다.
		FCFInventoryContainerState CargoContainer;
		CargoContainer.ContainerRef.OwnerId = OutFixture.OwnerId;
		CargoContainer.ContainerRef.ContainerId = OutFixture.CargoContainerId;
		CargoContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		CargoContainer.Capacity.MaximumSlotCount = 4;
		// [v1.0.0] Cargo_A에 실제 Item을 소유할 Entry입니다.
		FCFInventoryContainerEntry CargoEntry;
		CargoEntry.ItemInstance = OutFixture.EquipmentItem;
		CargoEntry.ContainerSlotId = TEXT("Cargo_A");
		CargoContainer.Entries.Add(CargoEntry);

		// [v1.0.0] Action 완료 전에는 비어 있어야 할 MountedEquipment 목적지입니다.
		FCFInventoryContainerState MountedContainer;
		MountedContainer.ContainerRef.OwnerId = OutFixture.OwnerId;
		MountedContainer.ContainerRef.ContainerId = OutFixture.MountedContainerId;
		MountedContainer.ContainerRef.ContainerType = ECFInventoryContainerType::MountedEquipment;
		MountedContainer.Capacity.MaximumSlotCount = 2;

		OutFixture.Containers.Add(CargoContainer);
		OutFixture.Containers.Add(MountedContainer);
		OutFixture.TransferRequest.TransactionId = FCFInventoryTransactionId::CreateNew();
		OutFixture.TransferRequest.ActionOwnerId = TEXT("FFIT_P0_02_Action");
		OutFixture.TransferRequest.ItemInstanceId = OutFixture.EquipmentItem.ItemHandle.ItemInstanceId;
		OutFixture.TransferRequest.SourceContainerId = OutFixture.CargoContainerId;
		OutFixture.TransferRequest.SourceSlotId = TEXT("Cargo_A");
		OutFixture.TransferRequest.DestinationContainerId = OutFixture.MountedContainerId;
		OutFixture.TransferRequest.DestinationSlotId = TEXT("RoofTurret");
		return Test.TestTrue(TEXT("Timed Action Transfer 유효"), OutFixture.TransferRequest.IsValid());
	}

	// [v1.0.0] 2초 Equip Action 요청을 생성합니다.
	FCFFieldFitActionRequest BuildActionRequest(const FCFFieldFitActionFixture& Fixture)
	{
		// [v1.0.0] 모든 Timed Action 테스트가 공유할 정상 요청입니다.
		FCFFieldFitActionRequest ActionRequest;
		ActionRequest.ActionType = ECFFieldFitActionType::Equip;
		ActionRequest.InventoryTransferRequest = Fixture.TransferRequest;
		ActionRequest.RequiredDurationSeconds = 2.0f;
		return ActionRequest;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitTimedActionLifecycleTest,
	"CarFight.Fitting.FFIT_P0_02.TimedActionLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Start에서 Reservation만 생성하고 시간 완료까지 Item 위치 무변경·Completing Prepared handoff가 유지되는지 검증합니다.
bool FCFFieldFitTimedActionLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 정상 Timed Action의 Inventory Fixture입니다.
	FCFFieldFitActionFixture Fixture;
	if (!BuildActionFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] 테스트할 순수 Field Fitting Timed Action 상태 머신입니다.
	FCFFieldFitTimedAction TimedAction;
	TestTrue(
		TEXT("Timed Action Start 성공"),
		TimedAction.Start(
			BuildActionRequest(Fixture),
			BuildAllowedActionPermission(),
			Fixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			Fixture.TransferLedger));
	TestEqual(TEXT("Start 후 InProgress"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::InProgress);
	TestEqual(TEXT("Start 후 Transaction Prepared"), Fixture.TransferLedger.GetTransactionState(Fixture.TransferRequest.TransactionId), ECFInventoryTransactionState::Prepared);
	TestTrue(TEXT("Start 후 Item Reserved"), Fixture.TransferLedger.IsItemReserved(Fixture.EquipmentItem.ItemHandle.ItemInstanceId));
	TestEqual(TEXT("Start 후 Cargo Item 유지"), Fixture.Containers[0].Entries.Num(), 1);
	TestEqual(TEXT("Start 후 Mounted 비어 있음"), Fixture.Containers[1].Entries.Num(), 0);

	// [v1.0.0] 절반 진행 후에도 실제 Inventory 위치가 그대로여야 합니다.
	TestTrue(TEXT("Timed Action 1초 진행"), TimedAction.Advance(1.0f, BuildAllowedActionPermission(), FCFFieldFitActionSignals(), Fixture.TransferLedger));
	TestEqual(TEXT("1초 후 InProgress"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::InProgress);
	TestTrue(TEXT("1초 후 Progress 0.5"), FMath::IsNearlyEqual(TimedAction.GetSnapshot().ProgressRatio, 0.5f));
	TestEqual(TEXT("1초 후 Cargo Item 유지"), Fixture.Containers[0].Entries.Num(), 1);
	TestEqual(TEXT("1초 후 Mounted 비어 있음"), Fixture.Containers[1].Entries.Num(), 0);

	// [v1.0.0] RequiredDuration 도달 시 Runtime/Inventory Commit 없이 Completing으로만 전환합니다.
	TestTrue(TEXT("Timed Action 완료 시간 도달"), TimedAction.Advance(1.0f, BuildAllowedActionPermission(), FCFFieldFitActionSignals(), Fixture.TransferLedger));
	TestEqual(TEXT("완료 시간 후 Completing"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Completing);
	TestTrue(TEXT("완료 시간 Progress 1"), FMath::IsNearlyEqual(TimedAction.GetSnapshot().ProgressRatio, 1.0f));
	TestEqual(TEXT("Completing 중 Transaction Prepared 유지"), Fixture.TransferLedger.GetTransactionState(Fixture.TransferRequest.TransactionId), ECFInventoryTransactionState::Prepared);
	TestEqual(TEXT("Completing 중 Cargo Item 유지"), Fixture.Containers[0].Entries.Num(), 1);
	TestEqual(TEXT("Completing 중 Mounted 비어 있음"), Fixture.Containers[1].Entries.Num(), 0);

	TestTrue(TEXT("후속 Coordinator 성공 뒤 MarkCompleted"), TimedAction.MarkCompleted());
	TestEqual(TEXT("MarkCompleted 후 Completed"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Completed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitTimedActionCancelTest,
	"CarFight.Fitting.FFIT_P0_02.CancelAndReservation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Permission 변화·외부 취소·Reservation 유실이 Prepared Transaction을 Rollback하고 부분 Inventory 변경을 남기지 않는지 검증합니다.
bool FCFFieldFitTimedActionCancelTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	{
		// [v1.0.0] Combat 시작으로 자동 취소할 독립 Fixture입니다.
		FCFFieldFitActionFixture Fixture;
		if (!BuildActionFixture(*this, Fixture))
		{
			return false;
		}
		// [v1.0.0] Combat blocker 취소를 검증할 Action입니다.
		FCFFieldFitTimedAction TimedAction;
		TestTrue(TEXT("Combat Cancel Action Start"), TimedAction.Start(BuildActionRequest(Fixture), BuildAllowedActionPermission(), Fixture.Containers, TArray<FCFInventoryContainerMassSnapshot>(), Fixture.TransferLedger));
		TestTrue(TEXT("Combat blocker Advance가 Rollback 성공"), TimedAction.Advance(0.1f, BuildBlockedActionPermission(ECFFieldFitBlockerSource::Combat, ECFFieldFitBlockReason::CombatActive), FCFFieldFitActionSignals(), Fixture.TransferLedger));
		TestEqual(TEXT("Combat blocker Cancelled"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Cancelled);
		TestEqual(TEXT("Combat blocker EndReason"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::CombatStarted);
		TestEqual(TEXT("Combat cancel Transaction RolledBack"), Fixture.TransferLedger.GetTransactionState(Fixture.TransferRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
		TestEqual(TEXT("Combat cancel Reservation 0"), Fixture.TransferLedger.GetActiveReservationCount(), 0);
		TestEqual(TEXT("Combat cancel Cargo 유지"), Fixture.Containers[0].Entries.Num(), 1);
		TestEqual(TEXT("Combat cancel Mounted 비어 있음"), Fixture.Containers[1].Entries.Num(), 0);
	}

	{
		// [v1.0.0] 사용자 취소 신호를 검증할 독립 Fixture입니다.
		FCFFieldFitActionFixture Fixture;
		if (!BuildActionFixture(*this, Fixture))
		{
			return false;
		}
		// [v1.0.0] UserCancelled 신호를 받을 Action입니다.
		FCFFieldFitTimedAction TimedAction;
		TestTrue(TEXT("User Cancel Action Start"), TimedAction.Start(BuildActionRequest(Fixture), BuildAllowedActionPermission(), Fixture.Containers, TArray<FCFInventoryContainerMassSnapshot>(), Fixture.TransferLedger));
		// [v1.0.0] 이번 업데이트에 사용자 취소가 발생했음을 나타낼 신호입니다.
		FCFFieldFitActionSignals CancelSignals;
		CancelSignals.bUserCancelled = true;
		TestTrue(TEXT("User Cancel Rollback 성공"), TimedAction.Advance(0.1f, BuildAllowedActionPermission(), CancelSignals, Fixture.TransferLedger));
		TestEqual(TEXT("User Cancelled 상태"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Cancelled);
		TestEqual(TEXT("User Cancelled EndReason"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::UserCancelled);
		TestEqual(TEXT("User Cancel Reservation 0"), Fixture.TransferLedger.GetActiveReservationCount(), 0);
	}

	{
		// [v1.0.0] 외부 Reservation 유실을 검증할 독립 Fixture입니다.
		FCFFieldFitActionFixture Fixture;
		if (!BuildActionFixture(*this, Fixture))
		{
			return false;
		}
		// [v1.0.0] ReservationLost를 감지할 Action입니다.
		FCFFieldFitTimedAction TimedAction;
		TestTrue(TEXT("Reservation Lost Action Start"), TimedAction.Start(BuildActionRequest(Fixture), BuildAllowedActionPermission(), Fixture.Containers, TArray<FCFInventoryContainerMassSnapshot>(), Fixture.TransferLedger));
		TestTrue(TEXT("외부 Rollback 성공"), Fixture.TransferLedger.RollbackTransfer(Fixture.TransferRequest.TransactionId).IsSuccessful());
		TestTrue(TEXT("Reservation Lost 처리 성공"), TimedAction.Advance(0.1f, BuildAllowedActionPermission(), FCFFieldFitActionSignals(), Fixture.TransferLedger));
		TestEqual(TEXT("Reservation Lost Cancelled"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Cancelled);
		TestEqual(TEXT("Reservation Lost EndReason"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::ReservationLost);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitTimedActionStartFailureTest,
	"CarFight.Fitting.FFIT_P0_02.StartFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Permission 거부, 무효 Duration과 이미 다른 Action이 소유한 Prepared Transaction을 안전하게 시작 거부하는지 검증합니다.
bool FCFFieldFitTimedActionStartFailureTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	{
		// [v1.0.0] Permission 거부 시작을 검증할 독립 Fixture입니다.
		FCFFieldFitActionFixture Fixture;
		if (!BuildActionFixture(*this, Fixture))
		{
			return false;
		}
		// [v1.0.0] 시작 Permission이 거부될 Action입니다.
		FCFFieldFitTimedAction TimedAction;
		TestFalse(TEXT("Permission 거부 Start 실패"), TimedAction.Start(BuildActionRequest(Fixture), BuildBlockedActionPermission(ECFFieldFitBlockerSource::Inventory, ECFFieldFitBlockReason::InventoryInaccessible), Fixture.Containers, TArray<FCFInventoryContainerMassSnapshot>(), Fixture.TransferLedger));
		TestEqual(TEXT("Permission 거부 Failed"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Failed);
		TestEqual(TEXT("Permission 거부 EndReason"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::PermissionDenied);
		TestEqual(TEXT("Permission 거부 Reservation 0"), Fixture.TransferLedger.GetActiveReservationCount(), 0);
	}

	{
		// [v1.0.0] 무효 Duration 시작을 검증할 독립 Fixture입니다.
		FCFFieldFitActionFixture Fixture;
		if (!BuildActionFixture(*this, Fixture))
		{
			return false;
		}
		// [v1.0.0] Duration 0으로 실패해야 할 요청입니다.
		FCFFieldFitActionRequest InvalidRequest = BuildActionRequest(Fixture);
		InvalidRequest.RequiredDurationSeconds = 0.0f;
		// [v1.0.0] 무효 요청을 받을 Action입니다.
		FCFFieldFitTimedAction TimedAction;
		TestFalse(TEXT("Duration 0 Start 실패"), TimedAction.Start(InvalidRequest, BuildAllowedActionPermission(), Fixture.Containers, TArray<FCFInventoryContainerMassSnapshot>(), Fixture.TransferLedger));
		TestEqual(TEXT("Duration 0 InvalidActionInput"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::InvalidActionInput);
	}

	{
		// [v1.0.0] 이미 다른 Action이 같은 Transaction을 Prepared한 소유 충돌 Fixture입니다.
		FCFFieldFitActionFixture Fixture;
		if (!BuildActionFixture(*this, Fixture))
		{
			return false;
		}
		TestEqual(TEXT("외부 Prepare 성공"), Fixture.TransferLedger.PrepareTransfer(Fixture.Containers, TArray<FCFInventoryContainerMassSnapshot>(), Fixture.TransferRequest).TransferStatus, ECFInventoryTransferStatus::Prepared);
		// [v1.0.0] AlreadyPrepared Transaction을 소유권 없이 탈취하면 안 되는 Action입니다.
		FCFFieldFitTimedAction TimedAction;
		TestFalse(TEXT("AlreadyPrepared Start 거부"), TimedAction.Start(BuildActionRequest(Fixture), BuildAllowedActionPermission(), Fixture.Containers, TArray<FCFInventoryContainerMassSnapshot>(), Fixture.TransferLedger));
		TestEqual(TEXT("AlreadyPrepared ReservationFailed"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::InventoryReservationFailed);
		TestEqual(TEXT("외부 Prepared 상태 보존"), Fixture.TransferLedger.GetTransactionState(Fixture.TransferRequest.TransactionId), ECFInventoryTransactionState::Prepared);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
