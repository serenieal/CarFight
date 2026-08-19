// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-034 FFIT-P0-03 Timed Action → Atomic Coordinator handoff 자동화 테스트
// Scope: Equip·Unequip Completing handoff 성공, Runtime 실패 Rollback과 Reservation 유실 선행조건을 Pawn 없이 검증합니다.
// Changelog:
// - v1.0.0: 같은 Prepared Transaction을 TimedAction에서 Coordinator로 넘기는 formal FFIT-P0-03 integration 회귀를 최초 추가.
// Migration:
// - Fitting 후보 Snapshot은 외부 Adapter에서 이미 검증된 입력으로 취급하며 이 테스트가 Mount 호환 계산을 복제하지 않습니다.
// - Field Mass Reapply는 FFIT-P0-04 범위이며 이 테스트는 원자 handoff와 Inventory 방향만 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFieldFitAction.h"

#include "CFEquipmentPresetData.h"
#include "CFInventoryItemData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingData.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"

namespace
{
	/** FFIT-P0-03 handoff 테스트의 Inventory 한 건입니다. */
	struct FCFFieldFitCompleteFixture
	{
		// [v1.0.0] 현재 차량 Inventory Owner ID입니다.
		FCFInventoryOwnerId OwnerId;

		// [v1.0.0] VehicleCargo Container ID입니다.
		FCFInventoryContainerId CargoContainerId;

		// [v1.0.0] MountedEquipment Container ID입니다.
		FCFInventoryContainerId MountedContainerId;

		// [v1.0.0] Equip 또는 Unequip에 사용할 실제 Equipment Item입니다.
		FCFInventoryItemInstance EquipmentItem;

		// [v1.0.0] Coordinator가 원자 변경할 Container 배열입니다.
		TArray<FCFInventoryContainerState> Containers;

		// [v1.0.0] TimedAction Reservation과 Coordinator Commit을 함께 소유할 Ledger입니다.
		FCFInventoryTransferLedger TransferLedger;

		// [v1.0.0] TimedAction과 Coordinator가 동일하게 사용할 Inventory Transfer입니다.
		FCFInventoryTransferRequest TransferRequest;
	};

	/** formal handoff에서 성공/실패 Runtime 단계를 주입할 최소 Fake Runtime Transaction입니다. */
	class FCFFieldFitCompleteRuntime final : public ICFFieldFitRuntimeTransaction
	{
	public:
		// [v1.0.0] Runtime Commit을 의도적으로 실패시킬지 여부입니다.
		bool bFailCommit = false;

		// [v1.0.0] Runtime Prepare 호출 횟수입니다.
		int32 PrepareCallCount = 0;

		// [v1.0.0] Runtime Commit 호출 횟수입니다.
		int32 CommitCallCount = 0;

		// [v1.0.0] Prepared Runtime cleanup 호출 횟수입니다.
		int32 RollbackPreparedCallCount = 0;

		// [v1.0.0] Commit 후 보상 호출 횟수입니다.
		int32 CompensationCallCount = 0;

		// [v1.0.0] 유효 후보 Snapshot을 formal completion 입력으로 수용합니다.
		virtual bool PrepareRuntime(const FCFVehicleFittingSnapshot& CandidateFittingSnapshot, FName RequestedActiveMountProfileId, FString& OutFailureSummary) override
		{
			(void)RequestedActiveMountProfileId;
			++PrepareCallCount;
			OutFailureSummary.Reset();
			return CandidateFittingSnapshot.IsValid();
		}

		// [v1.0.0] 테스트 시나리오에 따라 Runtime Commit 성공 또는 실패를 반환합니다.
		virtual bool CommitRuntime(FString& OutFailureSummary) override
		{
			++CommitCallCount;
			OutFailureSummary.Reset();
			if (bFailCommit)
			{
				OutFailureSummary = TEXT("CompleteRuntime: CommitFailed");
				return false;
			}
			return true;
		}

		// [v1.0.0] Runtime Commit 전 실패 cleanup 호출을 기록합니다.
		virtual void RollbackPreparedRuntime() override
		{
			++RollbackPreparedCallCount;
		}

		// [v1.0.0] 이 테스트에서는 Inventory Commit 뒤 compensation이 필요하지 않으므로 호출만 기록합니다.
		virtual bool CompensateCommittedRuntime(FString& OutFailureSummary) override
		{
			++CompensationCallCount;
			OutFailureSummary.Reset();
			return true;
		}
	};

	// [v1.0.0] 테스트 후보 Snapshot에 사용할 유효 EquipmentPresetData를 생성합니다.
	UCFEquipmentPresetData* CreateCompletePreset(UObject* Outer)
	{
		// [v1.0.0] 실제 Mount 질량을 제공할 TurretMountData입니다.
		UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(Outer);
		TurretMountData->TurretMountId = TEXT("CompleteMount");
		TurretMountData->TurretMountWeightKg = 20.0f;

		// [v1.0.0] 실제 Weapon 질량과 Mount 호환을 제공할 WeaponData입니다.
		UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(Outer);
		WeaponData->WeaponId = TEXT("CompleteWeapon");
		WeaponData->WeaponSize = ECFVehicleWeaponSize::Medium;
		WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		WeaponData->WeaponMassKg = 80.0f;

		// [v1.0.0] Fitting Snapshot의 resolved equipment가 사용할 Preset입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		EquipmentPresetData->EquipmentId = TEXT("CompletePreset");
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;
		EquipmentPresetData->DefaultTurretMountData = TurretMountData;
		EquipmentPresetData->DefaultWeaponData = WeaponData;
		return EquipmentPresetData;
	}

	// [v1.0.0] formal handoff 입력으로 사용할 유효 Fitting Snapshot을 생성합니다.
	FCFVehicleFittingSnapshot BuildCompleteSnapshot(UObject* Outer, UCFEquipmentPresetData* EquipmentPresetData)
	{
		// [v1.0.0] 한 개 RoofTurret를 가진 VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);
		VehicleData->BaseVehicleMassKg = 1000.0f;
		VehicleData->MaximumGrossMassKg = 2500.0f;

		// [v1.0.0] RoofTurret의 위치 참조를 만족할 Hardpoint입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Test");
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] 후보 EquipmentPreset을 허용할 MountProfile입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret");
		MountProfile.LocationSlotRef = TEXT("Top_01");
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Medium;
		MountProfile.DefaultEquipmentPresetData = EquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);

		// [v1.0.0] 기존 검증 경로를 통해 Snapshot을 만들 Transient FittingData입니다.
		UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>(Outer);
		FittingData->FittingId = TEXT("CompleteCandidate");
		FittingData->VehicleData = VehicleData;
		FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
		FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

		// [v1.0.0] RoofTurret에 후보 Equipment를 활성화할 Mount 선택입니다.
		FCFVehicleMountSelection MountSelection;
		MountSelection.MountProfileId = TEXT("RoofTurret");
		MountSelection.EquipmentPresetData = EquipmentPresetData;
		MountSelection.bEnabled = true;
		FittingData->MountSelections.Add(MountSelection);
		return FittingData->BuildFittingSnapshot();
	}

	// [v1.0.0] Equip 또는 Unequip 방향의 단일 Inventory Fixture를 생성합니다.
	bool BuildCompleteFixture(FAutomationTestBase& Test, const bool bUnequip, UCFEquipmentPresetData* EquipmentPresetData, FCFFieldFitCompleteFixture& OutFixture)
	{
		OutFixture.OwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.CargoContainerId = FCFInventoryContainerId::CreateNew();
		OutFixture.MountedContainerId = FCFInventoryContainerId::CreateNew();

		// [v1.0.0] 실제 ItemInstance를 생성할 Equipment Definition입니다.
		UCFEquipmentItemData* EquipmentDefinition = NewObject<UCFEquipmentItemData>();
		EquipmentDefinition->ItemDefinitionId = bUnequip ? TEXT("CompleteUnequipItem") : TEXT("CompleteEquipItem");
		EquipmentDefinition->DisplayName = FText::FromString(bUnequip ? TEXT("해제 테스트 장비") : TEXT("장착 테스트 장비"));
		EquipmentDefinition->EquipmentPresetData = EquipmentPresetData;
		OutFixture.EquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(EquipmentDefinition);
		if (!Test.TestTrue(TEXT("Complete Item 유효"), OutFixture.EquipmentItem.IsValidForDefinition(EquipmentDefinition)))
		{
			return false;
		}

		// [v1.0.0] 현재 차량 Cargo 상태입니다.
		FCFInventoryContainerState CargoContainer;
		CargoContainer.ContainerRef.OwnerId = OutFixture.OwnerId;
		CargoContainer.ContainerRef.ContainerId = OutFixture.CargoContainerId;
		CargoContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		CargoContainer.Capacity.MaximumSlotCount = 4;

		// [v1.0.0] 현재 차량 MountedEquipment 상태입니다.
		FCFInventoryContainerState MountedContainer;
		MountedContainer.ContainerRef.OwnerId = OutFixture.OwnerId;
		MountedContainer.ContainerRef.ContainerId = OutFixture.MountedContainerId;
		MountedContainer.ContainerRef.ContainerType = ECFInventoryContainerType::MountedEquipment;
		MountedContainer.Capacity.MaximumSlotCount = 2;

		// [v1.0.0] Action 시작 위치에 실제 Item을 소유할 Entry입니다.
		FCFInventoryContainerEntry ItemEntry;
		ItemEntry.ItemInstance = OutFixture.EquipmentItem;
		ItemEntry.ContainerSlotId = bUnequip ? FName(TEXT("RoofTurret")) : FName(TEXT("Cargo_A"));
		if (bUnequip)
		{
			MountedContainer.Entries.Add(ItemEntry);
		}
		else
		{
			CargoContainer.Entries.Add(ItemEntry);
		}

		OutFixture.Containers.Add(CargoContainer);
		OutFixture.Containers.Add(MountedContainer);
		OutFixture.TransferRequest.TransactionId = FCFInventoryTransactionId::CreateNew();
		OutFixture.TransferRequest.ActionOwnerId = bUnequip ? FName(TEXT("FFIT_P0_03_Unequip")) : FName(TEXT("FFIT_P0_03_Equip"));
		OutFixture.TransferRequest.ItemInstanceId = OutFixture.EquipmentItem.ItemHandle.ItemInstanceId;
		OutFixture.TransferRequest.SourceContainerId = bUnequip ? OutFixture.MountedContainerId : OutFixture.CargoContainerId;
		OutFixture.TransferRequest.SourceSlotId = bUnequip ? FName(TEXT("RoofTurret")) : FName(TEXT("Cargo_A"));
		OutFixture.TransferRequest.DestinationContainerId = bUnequip ? OutFixture.CargoContainerId : OutFixture.MountedContainerId;
		OutFixture.TransferRequest.DestinationSlotId = bUnequip ? FName(TEXT("Cargo_A")) : FName(TEXT("RoofTurret"));
		return Test.TestTrue(TEXT("Complete Transfer 유효"), OutFixture.TransferRequest.IsValid());
	}

	// [v1.0.0] Permission을 이미 통과한 Timed Action 시작 결과를 생성합니다.
	FCFFieldFitPermissionResult BuildCompletePermission()
	{
		// [v1.0.0] FFIT-P0-03가 M1 조건을 재계산하지 않도록 전달할 허용 결과입니다.
		FCFFieldFitPermissionResult PermissionResult;
		PermissionResult.bCanStart = true;
		return PermissionResult;
	}

	// [v1.0.0] Equip 또는 Unequip Timed Action을 Completing까지 진행시킵니다.
	bool AdvanceActionToCompleting(
		FAutomationTestBase& Test,
		const bool bUnequip,
		FCFFieldFitCompleteFixture& Fixture,
		FCFFieldFitTimedAction& TimedAction)
	{
		// [v1.0.0] 같은 Transfer를 소유할 1초 Field Fitting Action 요청입니다.
		FCFFieldFitActionRequest ActionRequest;
		ActionRequest.ActionType = bUnequip ? ECFFieldFitActionType::Unequip : ECFFieldFitActionType::Equip;
		ActionRequest.InventoryTransferRequest = Fixture.TransferRequest;
		ActionRequest.RequiredDurationSeconds = 1.0f;
		if (!Test.TestTrue(
			TEXT("Complete TimedAction Start"),
			TimedAction.Start(
				ActionRequest,
				BuildCompletePermission(),
				Fixture.Containers,
				TArray<FCFInventoryContainerMassSnapshot>(),
				Fixture.TransferLedger)))
		{
			return false;
		}

		if (!Test.TestTrue(
			TEXT("Complete TimedAction Completing 도달"),
			TimedAction.Advance(
				1.0f,
				BuildCompletePermission(),
				FCFFieldFitActionSignals(),
				Fixture.TransferLedger)))
		{
			return false;
		}
		return Test.TestEqual(TEXT("Complete TimedAction 상태 Completing"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Completing);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitActionCoordinatorSuccessTest,
	"CarFight.Fitting.FFIT_P0_03.ActionCoordinatorSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Equip·Unequip 모두 TimedAction의 Prepared Transaction이 Coordinator에 이어져 정확히 한 번 Commit되는지 검증합니다.
bool FCFFieldFitActionCoordinatorSuccessTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Equip/Unequip 양쪽이 공유할 검증 후보 EquipmentPreset입니다.
	UCFEquipmentPresetData* EquipmentPresetData = CreateCompletePreset(GetTransientPackage());
	// [v1.0.0] Atomic Coordinator Runtime 입력으로 사용할 검증 Fitting Snapshot입니다.
	const FCFVehicleFittingSnapshot CandidateSnapshot = BuildCompleteSnapshot(GetTransientPackage(), EquipmentPresetData);
	if (!TestTrue(TEXT("Complete Candidate Snapshot 유효"), CandidateSnapshot.IsValid()))
	{
		return false;
	}

	for (const bool bUnequip : { false, true })
	{
		// [v1.0.0] 현재 방향의 독립 Inventory Fixture입니다.
		FCFFieldFitCompleteFixture Fixture;
		if (!BuildCompleteFixture(*this, bUnequip, EquipmentPresetData, Fixture))
		{
			return false;
		}

		// [v1.0.0] 현재 방향의 Timed Action 상태 머신입니다.
		FCFFieldFitTimedAction TimedAction;
		if (!AdvanceActionToCompleting(*this, bUnequip, Fixture, TimedAction))
		{
			return false;
		}
		TestEqual(TEXT("Coordinator handoff 직전 Prepared"), Fixture.TransferLedger.GetTransactionState(Fixture.TransferRequest.TransactionId), ECFInventoryTransactionState::Prepared);

		// [v1.0.0] 성공 Runtime 단계로 formal Coordinator handoff를 검증할 Adapter입니다.
		FCFFieldFitCompleteRuntime RuntimeTransaction;
		// [v1.0.0] TimedAction이 같은 Prepared Transaction을 Coordinator에 넘긴 실제 결과입니다.
		const FCFFieldFitCompletionResult CompletionResult = TimedAction.CompleteWithCoordinator(
			CandidateSnapshot,
			TEXT("RoofTurret"),
			Fixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			Fixture.TransferLedger,
			RuntimeTransaction);
		TestEqual(TEXT("Formal handoff Completed"), CompletionResult.CompletionStatus, ECFFieldFitCompletionStatus::Completed);
		TestEqual(TEXT("TimedAction Completed"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Completed);
		TestEqual(TEXT("Inventory Transaction Committed"), Fixture.TransferLedger.GetTransactionState(Fixture.TransferRequest.TransactionId), ECFInventoryTransactionState::Committed);
		TestEqual(TEXT("Active Reservation 0"), Fixture.TransferLedger.GetActiveReservationCount(), 0);
		TestEqual(TEXT("Runtime Prepare 1회"), RuntimeTransaction.PrepareCallCount, 1);
		TestEqual(TEXT("Runtime Commit 1회"), RuntimeTransaction.CommitCallCount, 1);

		if (bUnequip)
		{
			TestEqual(TEXT("Unequip 후 Cargo 1"), Fixture.Containers[0].Entries.Num(), 1);
			TestEqual(TEXT("Unequip 후 Mounted 0"), Fixture.Containers[1].Entries.Num(), 0);
		}
		else
		{
			TestEqual(TEXT("Equip 후 Cargo 0"), Fixture.Containers[0].Entries.Num(), 0);
			TestEqual(TEXT("Equip 후 Mounted 1"), Fixture.Containers[1].Entries.Num(), 1);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitActionCoordinatorFailureTest,
	"CarFight.Fitting.FFIT_P0_03.ActionCoordinatorFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] formal completion Runtime 실패가 Inventory를 Rollback하고 TimedAction을 Failed로 종료하는지 검증합니다.
bool FCFFieldFitActionCoordinatorFailureTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실패 시나리오의 검증 후보 EquipmentPreset입니다.
	UCFEquipmentPresetData* EquipmentPresetData = CreateCompletePreset(GetTransientPackage());
	// [v1.0.0] 실패 Runtime에도 유효해야 하는 후보 Snapshot입니다.
	const FCFVehicleFittingSnapshot CandidateSnapshot = BuildCompleteSnapshot(GetTransientPackage(), EquipmentPresetData);
	// [v1.0.0] Runtime 실패 시 Inventory Rollback을 확인할 Equip Fixture입니다.
	FCFFieldFitCompleteFixture Fixture;
	if (!TestTrue(TEXT("Failure Candidate Snapshot 유효"), CandidateSnapshot.IsValid())
		|| !BuildCompleteFixture(*this, false, EquipmentPresetData, Fixture))
	{
		return false;
	}

	// [v1.0.0] Completing 상태에서 formal completion을 시도할 Action입니다.
	FCFFieldFitTimedAction TimedAction;
	if (!AdvanceActionToCompleting(*this, false, Fixture, TimedAction))
	{
		return false;
	}

	// [v1.0.0] Runtime Commit만 실패시켜 Coordinator Rollback 경로를 검증할 Transaction입니다.
	FCFFieldFitCompleteRuntime RuntimeTransaction;
	RuntimeTransaction.bFailCommit = true;
	// [v1.0.0] Runtime 실패 뒤 Coordinator가 보상한 formal completion 결과입니다.
	const FCFFieldFitCompletionResult CompletionResult = TimedAction.CompleteWithCoordinator(
		CandidateSnapshot,
		TEXT("RoofTurret"),
		Fixture.Containers,
		TArray<FCFInventoryContainerMassSnapshot>(),
		Fixture.TransferLedger,
		RuntimeTransaction);
	TestEqual(TEXT("Runtime 실패 CompletionStatus"), CompletionResult.CompletionStatus, ECFFieldFitCompletionStatus::RuntimeCommitFailed);
	TestEqual(TEXT("Runtime 실패 Action Failed"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Failed);
	TestEqual(TEXT("Runtime 실패 Action EndReason"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::CompletionFailed);
	TestEqual(TEXT("Runtime 실패 Inventory RolledBack"), Fixture.TransferLedger.GetTransactionState(Fixture.TransferRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
	TestEqual(TEXT("Runtime 실패 Reservation 0"), Fixture.TransferLedger.GetActiveReservationCount(), 0);
	TestEqual(TEXT("Runtime 실패 Cargo 유지"), Fixture.Containers[0].Entries.Num(), 1);
	TestEqual(TEXT("Runtime 실패 Mounted 비어 있음"), Fixture.Containers[1].Entries.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitActionCoordinatorPreconditionTest,
	"CarFight.Fitting.FFIT_P0_03.ActionCoordinatorPrecondition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Completing handoff 전에 Reservation이 사라지면 Coordinator를 호출하지 않고 ReservationLost로 안전 실패하는지 검증합니다.
bool FCFFieldFitActionCoordinatorPreconditionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Reservation 유실 선행조건 테스트의 후보 EquipmentPreset입니다.
	UCFEquipmentPresetData* EquipmentPresetData = CreateCompletePreset(GetTransientPackage());
	// [v1.0.0] 선행조건 실패 전에도 유효한 후보 Snapshot입니다.
	const FCFVehicleFittingSnapshot CandidateSnapshot = BuildCompleteSnapshot(GetTransientPackage(), EquipmentPresetData);
	// [v1.0.0] Completing 뒤 외부 Rollback을 만들 Equip Fixture입니다.
	FCFFieldFitCompleteFixture Fixture;
	if (!TestTrue(TEXT("Precondition Candidate Snapshot 유효"), CandidateSnapshot.IsValid())
		|| !BuildCompleteFixture(*this, false, EquipmentPresetData, Fixture))
	{
		return false;
	}

	// [v1.0.0] Completing 상태를 만든 뒤 Reservation을 외부에서 제거할 Action입니다.
	FCFFieldFitTimedAction TimedAction;
	if (!AdvanceActionToCompleting(*this, false, Fixture, TimedAction))
	{
		return false;
	}
	TestTrue(TEXT("Precondition 외부 Rollback"), Fixture.TransferLedger.RollbackTransfer(Fixture.TransferRequest.TransactionId).IsSuccessful());

	// [v1.0.0] 선행조건 실패 시 호출되지 않아야 하는 Runtime Transaction입니다.
	FCFFieldFitCompleteRuntime RuntimeTransaction;
	// [v1.0.0] Reservation 유실을 Coordinator 호출 전 차단한 결과입니다.
	const FCFFieldFitCompletionResult CompletionResult = TimedAction.CompleteWithCoordinator(
		CandidateSnapshot,
		TEXT("RoofTurret"),
		Fixture.Containers,
		TArray<FCFInventoryContainerMassSnapshot>(),
		Fixture.TransferLedger,
		RuntimeTransaction);
	TestEqual(TEXT("Reservation 유실 InventoryPrepareFailed"), CompletionResult.CompletionStatus, ECFFieldFitCompletionStatus::InventoryPrepareFailed);
	TestEqual(TEXT("Reservation 유실 Action Failed"), TimedAction.GetSnapshot().ActionState, ECFFieldFitActionState::Failed);
	TestEqual(TEXT("Reservation 유실 EndReason"), TimedAction.GetSnapshot().EndReason, ECFFieldFitActionEndReason::ReservationLost);
	TestEqual(TEXT("Reservation 유실 Runtime Prepare 0"), RuntimeTransaction.PrepareCallCount, 0);
	TestEqual(TEXT("Reservation 유실 Runtime Commit 0"), RuntimeTransaction.CommitCallCount, 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
