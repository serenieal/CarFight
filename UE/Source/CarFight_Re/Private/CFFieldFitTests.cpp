// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-14
// Description: CF-FQ-035 INV-P0-06 Field Fitting Coordinator Pawn 없는 원자성·실제 계층 통합 자동화 테스트
// Scope: 정상 Commit, Runtime 실패, Inventory Commit 실패 보상, 실제 FittingComp same-mass·질량 변경 거부와 Coordinator 조합을 검증합니다.
// Changelog:
// - v1.3.0: Mass 복원 실패와 FittingComp 내부 Rollback 실패를 Coordinator RecoveryFailed로 분류하는 회귀를 추가.
// - v1.2.0: FFIT-P0-04 mass-changing Runtime Transaction의 후보 질량 Commit·직전 질량 Compensation과 Mass Apply 실패 즉시 원복 회귀를 추가.
// - v1.1.0: 실제 FCFFieldFitCoordinator → FCFFieldFitRuntimeAdapter → UCFVehicleFittingComp 조합의 same-mass 성공과 mass-change Prepare 실패·Inventory Rollback 회귀를 추가.
// - v1.0.0: Coordinator Atomic Success/Failure와 FittingComp Runtime Checkpoint/Compensation·Mass Gate 테스트를 최초 추가.
// Migration:
// - World/Pawn/Asset 저장 없이 Transient DataAsset, 순수 Inventory Container/Ledger와 Fake Runtime Adapter만 사용합니다.
// - FFIT-P0-01 Permission, FFIT-P0-02 Timed Action과 FFIT-P0-04 Field Mass Reapply를 완료한 것으로 해석하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFieldFitCoordinator.h"

#include "CFEquipmentPresetData.h"
#include "CFInventoryItemData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingData.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

namespace
{
	/** Coordinator 테스트가 사용할 실제 Inventory 한 건과 Transfer 상태입니다. */
	struct FCFFieldFitInventoryFixture
	{
		// [v1.0.0] 현재 차량 Inventory Owner ID입니다.
		FCFInventoryOwnerId OwnerId;

		// [v1.0.0] Equipment Item이 시작하는 VehicleCargo ID입니다.
		FCFInventoryContainerId CargoContainerId;

		// [v1.0.0] Equipment Item이 이동할 MountedEquipment ID입니다.
		FCFInventoryContainerId MountedContainerId;

		// [v1.0.0] Cargo에서 Mounted로 이동할 실제 Equipment Item입니다.
		FCFInventoryItemInstance EquipmentItem;

		// [v1.0.0] Atomic Transfer가 변경할 실제 Container 배열입니다.
		TArray<FCFInventoryContainerState> Containers;

		// [v1.0.0] Reservation과 Transaction 수명을 소유할 Ledger입니다.
		FCFInventoryTransferLedger TransferLedger;

		// [v1.0.0] Coordinator가 Prepare·Commit할 단일 Item Transfer 요청입니다.
		FCFInventoryTransferRequest TransferRequest;
	};

	/** Fitting Snapshot 생성에 필요한 동일 구조 장비 세트입니다. */
	struct FCFFieldFitEquipment
	{
		// [v1.0.0] 후보 Snapshot의 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] EquipmentPreset이 참조할 TurretMountData입니다.
		UCFTurretMountData* TurretMountData = nullptr;

		// [v1.0.0] EquipmentPreset이 참조할 WeaponData입니다.
		UCFWeaponData* WeaponData = nullptr;
	};

	/** Coordinator 단계 호출과 실패 주입을 기록하는 Fake Runtime Transaction입니다. */
	class FCFFieldFitFakeRuntimeTransaction final : public ICFFieldFitRuntimeTransaction
	{
	public:
		// [v1.0.0] Runtime Prepare 단계만 실패시킬지 여부입니다.
		bool bFailPrepare = false;

		// [v1.0.0] Runtime Commit 단계만 실패시킬지 여부입니다.
		bool bFailCommit = false;

		// [v1.0.0] Inventory Commit 직전 외부 상태 변경을 재현할지 여부입니다.
		bool bMutateInventoryDuringCommit = false;

		// [v1.0.0] Commit 단계에서 외부 변경을 주입할 실제 Container 배열입니다.
		TArray<FCFInventoryContainerState>* MutableContainers = nullptr;

		// [v1.0.0] Runtime Prepare 호출 횟수입니다.
		int32 PrepareCallCount = 0;

		// [v1.0.0] Runtime Commit 호출 횟수입니다.
		int32 CommitCallCount = 0;

		// [v1.0.0] Prepared 후보 Rollback 호출 횟수입니다.
		int32 RollbackPreparedCallCount = 0;

		// [v1.0.0] 성공 Runtime의 보상 복원 호출 횟수입니다.
		int32 CompensationCallCount = 0;

		// [v1.0.0] 후보 Snapshot을 아직 적용하지 않고 Prepare 결과를 반환합니다.
		virtual bool PrepareRuntime(const FCFVehicleFittingSnapshot& CandidateFittingSnapshot, FName RequestedActiveMountProfileId, FString& OutFailureSummary) override
		{
			(void)RequestedActiveMountProfileId;
			++PrepareCallCount;
			OutFailureSummary.Reset();
			if (bFailPrepare || !CandidateFittingSnapshot.IsValid())
			{
				OutFailureSummary = TEXT("FakeRuntime: PrepareFailed");
				return false;
			}
			return true;
		}

		// [v1.0.0] 후보 Runtime Commit과 선택적 Inventory 동시 변경 주입 결과를 반환합니다.
		virtual bool CommitRuntime(FString& OutFailureSummary) override
		{
			++CommitCallCount;
			OutFailureSummary.Reset();
			if (bFailCommit)
			{
				OutFailureSummary = TEXT("FakeRuntime: CommitFailed");
				return false;
			}

			if (bMutateInventoryDuringCommit && MutableContainers && MutableContainers->Num() > 0)
			{
				// [v1.0.0] Prepare 이후 Source 위치가 바뀐 외부 경쟁 상태를 만들어 Inventory Commit을 실패시킬 Cargo Entry입니다.
				FCFInventoryContainerEntry* SourceEntry = (*MutableContainers)[0].Entries.Num() > 0 ? &(*MutableContainers)[0].Entries[0] : nullptr;
				if (SourceEntry)
				{
					SourceEntry->ContainerSlotId = TEXT("ExternallyChangedSlot");
				}
			}
			return true;
		}

		// [v1.0.0] Commit 전 실패에서 Prepared 후보 정리 호출만 기록합니다.
		virtual void RollbackPreparedRuntime() override
		{
			++RollbackPreparedCallCount;
		}

		// [v1.0.0] Inventory Commit 실패 뒤 직전 Runtime 보상 복원 호출을 기록합니다.
		virtual bool CompensateCommittedRuntime(FString& OutFailureSummary) override
		{
			++CompensationCallCount;
			OutFailureSummary.Reset();
			return true;
		}
	};

	/** 실제 FittingComp에 적용된 Weapon·Defense 입력을 기록하는 하위 Fake Adapter입니다. */
	class FCFFieldFitApplyAdapter final : public ICFFittingRuntimeApplyAdapter
	{
	public:
		// [v1.0.0] Weapon Runtime 적용 횟수입니다.
		int32 WeaponApplyCallCount = 0;

		// [v1.0.0] Defense Runtime 적용 횟수입니다.
		int32 DefenseApplyCallCount = 0;

		// [v1.0.0] 마지막 Weapon Runtime 입력입니다.
		FCFFittingWeaponRuntimeInput LastWeaponInput;

		// [v1.0.0] 마지막 Defense Runtime 입력입니다.
		FCFFittingDefenseRuntimeInput LastDefenseInput;

		// [v1.0.0] Weapon 입력을 기록하고 Snapshot Mount 또는 Legacy 입력을 성공 처리합니다.
		virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) override
		{
			++WeaponApplyCallCount;
			LastWeaponInput = WeaponInput;
			bOutWeaponRuntimeReady = WeaponInput.UsesLegacyVehicleConfiguration() || WeaponInput.bHasResolvedMount;
			return true;
		}

		// [v1.0.0] Defense 입력을 기록하고 Data 유무와 무관하게 적용 경계 성공을 반환합니다.
		virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) override
		{
			++DefenseApplyCallCount;
			LastDefenseInput = DefenseInput;
			bOutDefenseRuntimeReady = DefenseInput.DefenseData != nullptr;
			return true;
		}
	};

	// [v1.0.0] 질량을 지정한 유효 EquipmentPreset 묶음을 생성합니다.
	FCFFieldFitEquipment CreateFieldFitEquipment(UObject* Outer, const FName EquipmentId, const float WeaponMassKg)
	{
		// [v1.0.0] 호출자에게 반환할 장비·Mount·Weapon 묶음입니다.
		FCFFieldFitEquipment Equipment;
		Equipment.TurretMountData = NewObject<UCFTurretMountData>(Outer);
		Equipment.TurretMountData->TurretMountId = FName(*FString::Printf(TEXT("%s_Mount"), *EquipmentId.ToString()));
		Equipment.TurretMountData->TurretMountWeightKg = 20.0f;
		Equipment.WeaponData = NewObject<UCFWeaponData>(Outer);
		Equipment.WeaponData->WeaponId = FName(*FString::Printf(TEXT("%s_Weapon"), *EquipmentId.ToString()));
		Equipment.WeaponData->WeaponSize = ECFVehicleWeaponSize::Medium;
		Equipment.WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		Equipment.WeaponData->WeaponMassKg = WeaponMassKg;
		Equipment.EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		Equipment.EquipmentPresetData->EquipmentId = EquipmentId;
		Equipment.EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		Equipment.EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;
		Equipment.EquipmentPresetData->DefaultTurretMountData = Equipment.TurretMountData;
		Equipment.EquipmentPresetData->DefaultWeaponData = Equipment.WeaponData;
		return Equipment;
	}

	// [v1.0.0] 한 개 RoofTurret MountProfile을 가진 테스트 VehicleData를 생성합니다.
	UCFVehicleData* CreateFieldFitVehicleData(UObject* Outer, UCFEquipmentPresetData* DefaultEquipmentPresetData)
	{
		// [v1.0.0] 후보 Snapshot의 플랫폼·질량·Mount 규칙을 제공할 VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);
		VehicleData->BaseVehicleMassKg = 1000.0f;
		VehicleData->MaximumGrossMassKg = 2500.0f;

		// [v1.0.0] RoofTurret가 참조할 단일 Hardpoint 위치입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Test");
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] Medium Turret 장비를 허용할 단일 MountProfile입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret");
		MountProfile.LocationSlotRef = TEXT("Top_01");
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Medium;
		MountProfile.DefaultEquipmentPresetData = DefaultEquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);
		return VehicleData;
	}

	// [v1.0.0] 한 Mount를 지정 EquipmentPreset으로 Override하는 유효 Snapshot을 생성합니다.
	FCFVehicleFittingSnapshot BuildFieldFitSnapshot(UObject* Outer, UCFVehicleData* VehicleData, const FName FittingId, UCFEquipmentPresetData* EquipmentPresetData)
	{
		// [v1.0.0] 기존 BuildFittingSnapshot 계약을 그대로 사용할 Transient 후보 FittingData입니다.
		UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>(Outer);
		FittingData->FittingId = FittingId;
		FittingData->VehicleData = VehicleData;
		FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
		FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

		// [v1.0.0] RoofTurret에 실제 EquipmentPreset을 선택할 Mount 입력입니다.
		FCFVehicleMountSelection MountSelection;
		MountSelection.MountProfileId = TEXT("RoofTurret");
		MountSelection.EquipmentPresetData = EquipmentPresetData;
		MountSelection.bEnabled = true;
		FittingData->MountSelections.Add(MountSelection);
		return FittingData->BuildFittingSnapshot();
	}

	// [v1.0.0] 단일 Equipment Item의 Cargo→Mounted Transfer Fixture를 생성합니다.
	bool BuildFieldFitInventoryFixture(FAutomationTestBase& Test, FCFFieldFitInventoryFixture& OutFixture)
	{
		OutFixture.OwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.CargoContainerId = FCFInventoryContainerId::CreateNew();
		OutFixture.MountedContainerId = FCFInventoryContainerId::CreateNew();

		// [v1.0.0] 실제 Inventory ItemInstance 생성을 위한 강타입 Equipment Definition입니다.
		UCFEquipmentItemData* EquipmentDefinition = NewObject<UCFEquipmentItemData>();
		EquipmentDefinition->ItemDefinitionId = TEXT("FieldFitEquipmentItem");
		EquipmentDefinition->DisplayName = FText::FromString(TEXT("필드 장착 아이템"));
		// [v1.0.0] Inventory Definition 계약을 완성할 Transient EquipmentPreset입니다.
		const FCFFieldFitEquipment InventoryEquipment = CreateFieldFitEquipment(EquipmentDefinition, TEXT("InventoryEquipment"), 80.0f);
		EquipmentDefinition->EquipmentPresetData = InventoryEquipment.EquipmentPresetData;
		OutFixture.EquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(EquipmentDefinition);
		if (!Test.TestTrue(TEXT("Field Fit Inventory Item 유효"), OutFixture.EquipmentItem.IsValidForDefinition(EquipmentDefinition)))
		{
			return false;
		}

		// [v1.0.0] Item이 시작하는 현재 차량 VehicleCargo입니다.
		FCFInventoryContainerState CargoContainer;
		CargoContainer.ContainerRef.OwnerId = OutFixture.OwnerId;
		CargoContainer.ContainerRef.ContainerId = OutFixture.CargoContainerId;
		CargoContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		CargoContainer.Capacity.MaximumSlotCount = 4;
		// [v1.0.0] Cargo_A에 실제 Equipment Item을 소유할 Entry입니다.
		FCFInventoryContainerEntry CargoEntry;
		CargoEntry.ItemInstance = OutFixture.EquipmentItem;
		CargoEntry.ContainerSlotId = TEXT("Cargo_A");
		CargoContainer.Entries.Add(CargoEntry);

		// [v1.0.0] 빈 RoofTurret 목적지를 가진 현재 차량 MountedEquipment입니다.
		FCFInventoryContainerState MountedContainer;
		MountedContainer.ContainerRef.OwnerId = OutFixture.OwnerId;
		MountedContainer.ContainerRef.ContainerId = OutFixture.MountedContainerId;
		MountedContainer.ContainerRef.ContainerType = ECFInventoryContainerType::MountedEquipment;
		MountedContainer.Capacity.MaximumSlotCount = 2;

		OutFixture.Containers.Add(CargoContainer);
		OutFixture.Containers.Add(MountedContainer);
		OutFixture.TransferRequest.TransactionId = FCFInventoryTransactionId::CreateNew();
		OutFixture.TransferRequest.ActionOwnerId = TEXT("INV_P0_06_FieldFit");
		OutFixture.TransferRequest.ItemInstanceId = OutFixture.EquipmentItem.ItemHandle.ItemInstanceId;
		OutFixture.TransferRequest.SourceContainerId = OutFixture.CargoContainerId;
		OutFixture.TransferRequest.SourceSlotId = TEXT("Cargo_A");
		OutFixture.TransferRequest.DestinationContainerId = OutFixture.MountedContainerId;
		OutFixture.TransferRequest.DestinationSlotId = TEXT("RoofTurret");
		return Test.TestTrue(TEXT("Field Fit Transfer Request 유효"), OutFixture.TransferRequest.IsValid());
	}

	// [v1.0.0] Coordinator 테스트의 유효 Completion Request를 생성합니다.
	FCFFieldFitCompletionRequest BuildCompletionRequest(const FCFFieldFitInventoryFixture& InventoryFixture, const FCFVehicleFittingSnapshot& CandidateSnapshot)
	{
		// [v1.0.0] Inventory Transfer와 Fitting 후보를 함께 전달할 Completion Request입니다.
		FCFFieldFitCompletionRequest CompletionRequest;
		CompletionRequest.InventoryTransferRequest = InventoryFixture.TransferRequest;
		CompletionRequest.CandidateFittingSnapshot = CandidateSnapshot;
		CompletionRequest.RequestedActiveMountProfileId = TEXT("RoofTurret");
		return CompletionRequest;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitCoordinatorAtomicTest,
	"CarFight.Inventory.INV_P0_06.FieldFitCoordinatorAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 정상 Commit, Runtime 실패와 Inventory Commit 실패가 부분 상태 없이 종료되는지 검증합니다.
bool FCFFieldFitCoordinatorAtomicTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Coordinator에 전달할 유효 후보 Snapshot을 만들 장비입니다.
	const FCFFieldFitEquipment CandidateEquipment = CreateFieldFitEquipment(GetTransientPackage(), TEXT("CoordinatorCandidate"), 80.0f);
	// [v1.0.0] 후보 Snapshot의 VehicleData입니다.
	UCFVehicleData* VehicleData = CreateFieldFitVehicleData(GetTransientPackage(), CandidateEquipment.EquipmentPresetData);
	// [v1.0.0] Runtime Fake가 유효성만 검사할 후보 Snapshot입니다.
	const FCFVehicleFittingSnapshot CandidateSnapshot = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("CoordinatorCandidateFit"), CandidateEquipment.EquipmentPresetData);
	if (!TestTrue(TEXT("Coordinator 후보 Snapshot 유효"), CandidateSnapshot.IsValid()))
	{
		return false;
	}

	{
		// [v1.0.0] 정상 Commit 경로의 독립 Inventory Fixture입니다.
		FCFFieldFitInventoryFixture SuccessFixture;
		if (!BuildFieldFitInventoryFixture(*this, SuccessFixture))
		{
			return false;
		}

		// [v1.0.0] 모든 Runtime 단계를 성공시키는 Fake Transaction입니다.
		FCFFieldFitFakeRuntimeTransaction SuccessRuntime;
		// [v1.0.0] 정상 Coordinator 완료 결과입니다.
		const FCFFieldFitCompletionResult SuccessResult = FCFFieldFitCoordinator::Complete(
			SuccessFixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			SuccessFixture.TransferLedger,
			SuccessRuntime,
			BuildCompletionRequest(SuccessFixture, CandidateSnapshot));
		TestEqual(TEXT("정상 Completion Completed"), SuccessResult.CompletionStatus, ECFFieldFitCompletionStatus::Completed);
		TestTrue(TEXT("정상 Completion 성공"), SuccessResult.IsSuccessful());
		TestEqual(TEXT("정상 Inventory Committed"), SuccessFixture.TransferLedger.GetTransactionState(SuccessFixture.TransferRequest.TransactionId), ECFInventoryTransactionState::Committed);
		TestEqual(TEXT("정상 Runtime Prepare 1회"), SuccessRuntime.PrepareCallCount, 1);
		TestEqual(TEXT("정상 Runtime Commit 1회"), SuccessRuntime.CommitCallCount, 1);
		TestEqual(TEXT("정상 Runtime 보상 0회"), SuccessRuntime.CompensationCallCount, 0);
	}

	{
		// [v1.0.0] Runtime Commit 실패 경로의 독립 Inventory Fixture입니다.
		FCFFieldFitInventoryFixture RuntimeFailureFixture;
		if (!BuildFieldFitInventoryFixture(*this, RuntimeFailureFixture))
		{
			return false;
		}

		// [v1.0.0] Runtime Commit만 실패시킬 Fake Transaction입니다.
		FCFFieldFitFakeRuntimeTransaction RuntimeFailure;
		RuntimeFailure.bFailCommit = true;
		// [v1.0.0] Runtime Commit 실패 Coordinator 결과입니다.
		const FCFFieldFitCompletionResult RuntimeFailureResult = FCFFieldFitCoordinator::Complete(
			RuntimeFailureFixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			RuntimeFailureFixture.TransferLedger,
			RuntimeFailure,
			BuildCompletionRequest(RuntimeFailureFixture, CandidateSnapshot));
		TestEqual(TEXT("Runtime 실패 상태"), RuntimeFailureResult.CompletionStatus, ECFFieldFitCompletionStatus::RuntimeCommitFailed);
		TestEqual(TEXT("Runtime 실패 Inventory RolledBack"), RuntimeFailureFixture.TransferLedger.GetTransactionState(RuntimeFailureFixture.TransferRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
		TestEqual(TEXT("Runtime 실패 Prepared Rollback 1회"), RuntimeFailure.RollbackPreparedCallCount, 1);
		TestEqual(TEXT("Runtime 실패 Compensation 0회"), RuntimeFailure.CompensationCallCount, 0);
	}

	{
		// [v1.0.0] Inventory Commit 경쟁 실패 경로의 독립 Fixture입니다.
		FCFFieldFitInventoryFixture InventoryFailureFixture;
		if (!BuildFieldFitInventoryFixture(*this, InventoryFailureFixture))
		{
			return false;
		}

		// [v1.0.0] Runtime Commit 성공 직후 Source 위치를 외부 변경해 Inventory Commit을 실패시킬 Fake Transaction입니다.
		FCFFieldFitFakeRuntimeTransaction InventoryFailureRuntime;
		InventoryFailureRuntime.bMutateInventoryDuringCommit = true;
		InventoryFailureRuntime.MutableContainers = &InventoryFailureFixture.Containers;
		// [v1.0.0] Inventory Commit 실패 뒤 양쪽 보상이 성공해야 할 Coordinator 결과입니다.
		const FCFFieldFitCompletionResult InventoryFailureResult = FCFFieldFitCoordinator::Complete(
			InventoryFailureFixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			InventoryFailureFixture.TransferLedger,
			InventoryFailureRuntime,
			BuildCompletionRequest(InventoryFailureFixture, CandidateSnapshot));
		TestEqual(TEXT("Inventory Commit 실패 보상 상태"), InventoryFailureResult.CompletionStatus, ECFFieldFitCompletionStatus::InventoryCommitFailedRolledBack);
		TestTrue(TEXT("Inventory Commit 실패 Runtime 보상"), InventoryFailureResult.bRuntimeCompensated);
		TestEqual(TEXT("Inventory Commit 실패 Compensation 1회"), InventoryFailureRuntime.CompensationCallCount, 1);
		TestEqual(TEXT("Inventory Commit 실패 Transaction RolledBack"), InventoryFailureFixture.TransferLedger.GetTransactionState(InventoryFailureFixture.TransferRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
		TestEqual(TEXT("Inventory Commit 실패 Active Reservation 0"), InventoryFailureFixture.TransferLedger.GetActiveReservationCount(), 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitRuntimeAdapterTest,
	"CarFight.Inventory.INV_P0_06.FieldFitRuntimeAdapter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 실제 FittingComp의 검증 Snapshot 직접 Prepare, same-mass Commit·보상과 질량 변경 후보 거부를 검증합니다.
bool FCFFieldFitRuntimeAdapterTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 초기 Applied와 same-mass 후보에 사용할 80kg Weapon 장비 A입니다.
	const FCFFieldFitEquipment EquipmentA = CreateFieldFitEquipment(GetTransientPackage(), TEXT("FieldRuntimeA"), 80.0f);
	// [v1.0.0] 동일 총질량의 후보 장비 B입니다.
	const FCFFieldFitEquipment EquipmentB = CreateFieldFitEquipment(GetTransientPackage(), TEXT("FieldRuntimeB"), 80.0f);
	// [v1.0.0] Field Mass Reapply 거부를 확인할 더 무거운 후보 장비 C입니다.
	const FCFFieldFitEquipment EquipmentC = CreateFieldFitEquipment(GetTransientPackage(), TEXT("FieldRuntimeHeavy"), 180.0f);
	// [v1.0.0] 세 Snapshot이 공유할 VehicleData입니다.
	UCFVehicleData* VehicleData = CreateFieldFitVehicleData(GetTransientPackage(), EquipmentA.EquipmentPresetData);
	// [v1.0.0] 직전 Applied 상태가 될 Snapshot A입니다.
	const FCFVehicleFittingSnapshot SnapshotA = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("AppliedA"), EquipmentA.EquipmentPresetData);
	// [v1.0.0] 같은 총질량으로 Runtime Commit할 후보 Snapshot B입니다.
	const FCFVehicleFittingSnapshot SnapshotB = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("CandidateB"), EquipmentB.EquipmentPresetData);
	// [v1.0.0] 총질량 변화 때문에 현재 Field Runtime Adapter가 거부해야 할 후보 Snapshot C입니다.
	const FCFVehicleFittingSnapshot SnapshotC = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("CandidateHeavy"), EquipmentC.EquipmentPresetData);
	if (!TestTrue(TEXT("Snapshot A 유효"), SnapshotA.IsValid())
		|| !TestTrue(TEXT("Snapshot B 유효"), SnapshotB.IsValid())
		|| !TestTrue(TEXT("Snapshot C 유효"), SnapshotC.IsValid()))
	{
		return false;
	}
	TestTrue(TEXT("A/B same-mass"), FMath::IsNearlyEqual(SnapshotA.TotalVehicleMassKg, SnapshotB.TotalVehicleMassKg, 0.01f));
	TestFalse(TEXT("A/C mass differs"), FMath::IsNearlyEqual(SnapshotA.TotalVehicleMassKg, SnapshotC.TotalVehicleMassKg, 0.01f));

	// [v1.0.0] 실제 FittingComp 상태 머신을 Transient로 생성합니다.
	UCFVehicleFittingComp* FittingComponent = NewObject<UCFVehicleFittingComp>();
	// [v1.0.0] Weapon·Defense 입력 적용과 Checkpoint 복원을 기록할 하위 Fake Adapter입니다.
	FCFFieldFitApplyAdapter ApplyAdapter;
	if (!TestNotNull(TEXT("FittingComponent 생성"), FittingComponent))
	{
		return false;
	}

	TestTrue(TEXT("초기 Snapshot A 직접 Prepare"), FittingComponent->PrepareSortieFittingSnapshot(SnapshotA, TEXT("RoofTurret")));
	TestTrue(TEXT("초기 Snapshot A Commit"), FittingComponent->CommitPreparedSortieFitting(ApplyAdapter));
	TestEqual(TEXT("초기 Applied Fitting A"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("AppliedA")));

	// [v1.0.0] 실제 FittingComp를 Field completion Runtime 계약에 연결할 same-mass Adapter입니다.
	FCFFieldFitRuntimeAdapter FieldRuntimeAdapter(FittingComponent, ApplyAdapter);
	// [v1.0.0] Runtime Adapter 실패 진단을 받을 문자열입니다.
	FString RuntimeFailureSummary;
	TestTrue(TEXT("same-mass 후보 B Prepare"), FieldRuntimeAdapter.PrepareRuntime(SnapshotB, TEXT("RoofTurret"), RuntimeFailureSummary));
	TestTrue(TEXT("same-mass 후보 B Commit"), FieldRuntimeAdapter.CommitRuntime(RuntimeFailureSummary));
	TestEqual(TEXT("후보 B Applied"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("CandidateB")));
	TestTrue(TEXT("후보 B 보상 Restore"), FieldRuntimeAdapter.CompensateCommittedRuntime(RuntimeFailureSummary));
	TestEqual(TEXT("보상 후 Applied A 복원"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("AppliedA")));

	// [v1.0.0] 새 Adapter 수명으로 무거운 후보를 Prepare 단계에서 거부할 계약입니다.
	FCFFieldFitRuntimeAdapter HeavyRuntimeAdapter(FittingComponent, ApplyAdapter);
	TestFalse(TEXT("질량 변경 후보 Prepare 거부"), HeavyRuntimeAdapter.PrepareRuntime(SnapshotC, TEXT("RoofTurret"), RuntimeFailureSummary));
	TestTrue(TEXT("질량 변경 거부 요약"), RuntimeFailureSummary.Contains(TEXT("RuntimeMassReapplyUnsupported")));
		TestEqual(TEXT("질량 변경 거부 후 Applied A 유지"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("AppliedA")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitCoordinatorIntegrationTest,
	"CarFight.Inventory.INV_P0_06.FieldFitCoordinatorFittingComp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] 실제 Coordinator와 FittingComp Adapter 조합에서 same-mass 완료와 mass-change Prepare 실패가 Inventory까지 원자적으로 끝나는지 검증합니다.
bool FCFFieldFitCoordinatorIntegrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] 초기 Applied 상태와 same-mass 후보에 사용할 80kg 장비 A입니다.
	const FCFFieldFitEquipment EquipmentA = CreateFieldFitEquipment(GetTransientPackage(), TEXT("IntegratedA"), 80.0f);
	// [v1.1.0] 동일 총질량으로 정상 완료할 80kg 장비 B입니다.
	const FCFFieldFitEquipment EquipmentB = CreateFieldFitEquipment(GetTransientPackage(), TEXT("IntegratedB"), 80.0f);
	// [v1.1.0] Field Mass Reapply 부재로 Coordinator Prepare에서 거부할 180kg 장비 C입니다.
	const FCFFieldFitEquipment EquipmentC = CreateFieldFitEquipment(GetTransientPackage(), TEXT("IntegratedHeavy"), 180.0f);
	// [v1.1.0] 세 Snapshot이 공유할 VehicleData입니다.
	UCFVehicleData* VehicleData = CreateFieldFitVehicleData(GetTransientPackage(), EquipmentA.EquipmentPresetData);
	// [v1.1.0] 각 시나리오의 직전 Applied 상태가 될 Snapshot A입니다.
	const FCFVehicleFittingSnapshot SnapshotA = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("IntegratedAppliedA"), EquipmentA.EquipmentPresetData);
	// [v1.1.0] same-mass 정상 completion 후보 Snapshot B입니다.
	const FCFVehicleFittingSnapshot SnapshotB = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("IntegratedCandidateB"), EquipmentB.EquipmentPresetData);
	// [v1.1.0] mass-change 명시 거부 후보 Snapshot C입니다.
	const FCFVehicleFittingSnapshot SnapshotC = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("IntegratedHeavyC"), EquipmentC.EquipmentPresetData);
	if (!TestTrue(TEXT("Integration Snapshot A 유효"), SnapshotA.IsValid())
		|| !TestTrue(TEXT("Integration Snapshot B 유효"), SnapshotB.IsValid())
		|| !TestTrue(TEXT("Integration Snapshot C 유효"), SnapshotC.IsValid()))
	{
		return false;
	}

	{
		// [v1.1.0] same-mass 실제 계층 성공 시나리오의 독립 FittingComp입니다.
		UCFVehicleFittingComp* FittingComponent = NewObject<UCFVehicleFittingComp>();
		// [v1.1.0] 실제 FittingComp Weapon·Defense 적용을 기록할 하위 Fake Adapter입니다.
		FCFFieldFitApplyAdapter ApplyAdapter;
		// [v1.1.0] 실제 Inventory Prepare·Commit을 수행할 독립 Fixture입니다.
		FCFFieldFitInventoryFixture InventoryFixture;
		if (!TestNotNull(TEXT("Integration Success FittingComp"), FittingComponent)
			|| !BuildFieldFitInventoryFixture(*this, InventoryFixture))
		{
			return false;
		}

		TestTrue(TEXT("Integration Success 초기 A Prepare"), FittingComponent->PrepareSortieFittingSnapshot(SnapshotA, TEXT("RoofTurret")));
		TestTrue(TEXT("Integration Success 초기 A Commit"), FittingComponent->CommitPreparedSortieFitting(ApplyAdapter));

		// [v1.1.0] 실제 FittingComp를 Coordinator의 Runtime Transaction 계약에 연결할 Adapter입니다.
		FCFFieldFitRuntimeAdapter RuntimeTransaction(FittingComponent, ApplyAdapter);
		// [v1.1.0] Coordinator가 Inventory와 실제 FittingComp를 함께 완료한 결과입니다.
		const FCFFieldFitCompletionResult CompletionResult = FCFFieldFitCoordinator::Complete(
			InventoryFixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			InventoryFixture.TransferLedger,
			RuntimeTransaction,
			BuildCompletionRequest(InventoryFixture, SnapshotB));
		TestEqual(TEXT("Integration same-mass Completed"), CompletionResult.CompletionStatus, ECFFieldFitCompletionStatus::Completed);
		TestEqual(TEXT("Integration same-mass Inventory Committed"), InventoryFixture.TransferLedger.GetTransactionState(InventoryFixture.TransferRequest.TransactionId), ECFInventoryTransactionState::Committed);
		TestEqual(TEXT("Integration same-mass Applied B"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("IntegratedCandidateB")));
		TestEqual(TEXT("Integration same-mass Active Reservation 0"), InventoryFixture.TransferLedger.GetActiveReservationCount(), 0);
	}

	{
		// [v1.1.0] mass-change 거부 시나리오의 독립 FittingComp입니다.
		UCFVehicleFittingComp* FittingComponent = NewObject<UCFVehicleFittingComp>();
		// [v1.1.0] mass-change 시나리오의 Weapon·Defense 적용 기록 Adapter입니다.
		FCFFieldFitApplyAdapter ApplyAdapter;
		// [v1.1.0] Runtime Prepare 실패 시 Reservation Rollback을 확인할 독립 Inventory Fixture입니다.
		FCFFieldFitInventoryFixture InventoryFixture;
		if (!TestNotNull(TEXT("Integration MassGate FittingComp"), FittingComponent)
			|| !BuildFieldFitInventoryFixture(*this, InventoryFixture))
		{
			return false;
		}

		TestTrue(TEXT("Integration MassGate 초기 A Prepare"), FittingComponent->PrepareSortieFittingSnapshot(SnapshotA, TEXT("RoofTurret")));
		TestTrue(TEXT("Integration MassGate 초기 A Commit"), FittingComponent->CommitPreparedSortieFitting(ApplyAdapter));

		// [v1.1.0] mass-change 후보를 실제 FittingComp와 연결할 Runtime Transaction Adapter입니다.
		FCFFieldFitRuntimeAdapter RuntimeTransaction(FittingComponent, ApplyAdapter);
		// [v1.1.0] 질량 변경 후보가 Runtime Prepare에서 거부되고 Inventory가 보상 Rollback돼야 할 결과입니다.
		const FCFFieldFitCompletionResult CompletionResult = FCFFieldFitCoordinator::Complete(
			InventoryFixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			InventoryFixture.TransferLedger,
			RuntimeTransaction,
			BuildCompletionRequest(InventoryFixture, SnapshotC));
		TestEqual(TEXT("Integration mass-change RuntimePrepareFailed"), CompletionResult.CompletionStatus, ECFFieldFitCompletionStatus::RuntimePrepareFailed);
		TestEqual(TEXT("Integration mass-change Inventory RolledBack"), InventoryFixture.TransferLedger.GetTransactionState(InventoryFixture.TransferRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
		TestEqual(TEXT("Integration mass-change Active Reservation 0"), InventoryFixture.TransferLedger.GetActiveReservationCount(), 0);
		TestEqual(TEXT("Integration mass-change Applied A 유지"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("IntegratedAppliedA")));
		TestTrue(TEXT("Integration mass-change 실패 요약"), CompletionResult.FailureSummary.Contains(TEXT("RuntimeMassReapplyUnsupported")));
	}

	return true;
}

namespace
{
	/** FFIT-P0-04 질량 적용 요청을 기록하고 지정 호출만 실패시킬 Pawn 없는 Mass Runtime입니다. */
	class FCFFieldFitMassTestRuntime final : public ICFFieldFitMassRuntime
	{
	public:
				// [v1.2.0] 1부터 시작하는 질량 적용 호출 번호 중 실패시킬 번호입니다. 0이면 지정 호출 실패를 사용하지 않습니다.
		int32 FailOnCallNumber = 0;

		// [v1.3.0] 후보 적용과 보상 복원을 포함해 모든 질량 적용 요청을 실패시킬지 여부입니다.
		bool bFailEveryCall = false;

		// [v1.2.0] transaction이 실제 요청한 질량을 호출 순서대로 보존합니다.
		TArray<float> RequestedMassesKg;

		// [v1.2.0] 후보 또는 보상 질량 요청을 기록하고 지정 호출에서만 실패합니다.
		virtual bool ReapplyVehicleMassKg(const float TargetMassKg, FString& OutFailureSummary) override
		{
			RequestedMassesKg.Add(TargetMassKg);
			OutFailureSummary.Reset();
						if (bFailEveryCall || (FailOnCallNumber > 0 && RequestedMassesKg.Num() == FailOnCallNumber))
			{
				OutFailureSummary = FString::Printf(TEXT("MassTestRuntime: InjectedFailure, Target=%.3f"), TargetMassKg);
				return false;
			}
						return FMath::IsFinite(TargetMassKg) && TargetMassKg > 0.0f;
		}
	};

	/** 후보 Weapon 적용과 직후 이전 Runtime Rollback의 Weapon 적용을 모두 실패시켜 내부 복구 실패를 재현합니다. */
	class FCFFieldFitRollbackFailApplyAdapter final : public ICFFittingRuntimeApplyAdapter
	{
	public:
		// [v1.3.0] 후보 적용과 Rollback 복원 시도 횟수를 확인할 Weapon 호출 수입니다.
		int32 WeaponApplyCallCount = 0;

		// [v1.3.0] 후보 Weapon 적용과 이전 Runtime 복원을 모두 실패시켜 FittingComp의 Rollback 실패를 만듭니다.
		virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) override
		{
			(void)WeaponInput;
			++WeaponApplyCallCount;
			bOutWeaponRuntimeReady = false;
			return false;
		}

		// [v1.3.0] Weapon 단계에서 이미 실패하므로 Defense 호출이 오면 실패로 유지합니다.
		virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) override
		{
			(void)DefenseInput;
			bOutDefenseRuntimeReady = false;
			return false;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitMassTransactionTest,
	"CarFight.Fitting.FFIT_P0_04.FieldRuntimeMassTransaction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.2.0] 질량 변경 후보가 같은 Runtime Transaction에서 Commit·Compensation되고 Mass Apply 실패 시 직전 Fitting과 질량으로 즉시 복원되는지 검증합니다.
bool FCFFieldFitMassTransactionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.2.0] 직전 Applied 상태에 사용할 80kg Weapon 장비입니다.
	const FCFFieldFitEquipment EquipmentA = CreateFieldFitEquipment(GetTransientPackage(), TEXT("MassTxnAppliedA"), 80.0f);
	// [v1.2.0] 실제 질량 변경을 요구할 180kg Weapon 후보 장비입니다.
	const FCFFieldFitEquipment EquipmentHeavy = CreateFieldFitEquipment(GetTransientPackage(), TEXT("MassTxnHeavy"), 180.0f);
	// [v1.2.0] 두 Snapshot이 공유할 차량 플랫폼 데이터입니다.
	UCFVehicleData* VehicleData = CreateFieldFitVehicleData(GetTransientPackage(), EquipmentA.EquipmentPresetData);
	// [v1.2.0] 보상 복원 기준이 될 기존 Applied Snapshot입니다.
	const FCFVehicleFittingSnapshot SnapshotA = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("MassTxnAppliedA"), EquipmentA.EquipmentPresetData);
	// [v1.2.0] 총질량이 달라 Mass Runtime capability가 필요한 후보 Snapshot입니다.
	const FCFVehicleFittingSnapshot SnapshotHeavy = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("MassTxnHeavy"), EquipmentHeavy.EquipmentPresetData);
	if (!TestTrue(TEXT("Mass Transaction Snapshot A 유효"), SnapshotA.IsValid())
		|| !TestTrue(TEXT("Mass Transaction Heavy Snapshot 유효"), SnapshotHeavy.IsValid()))
	{
		return false;
	}
	TestFalse(TEXT("Mass Transaction 후보 질량 실제 변경"), FMath::IsNearlyEqual(SnapshotA.TotalVehicleMassKg, SnapshotHeavy.TotalVehicleMassKg, 0.01f));

	{
		// [v1.2.0] 성공 Commit 뒤 compensation까지 검증할 실제 FittingComp 상태 머신입니다.
		UCFVehicleFittingComp* FittingComponent = NewObject<UCFVehicleFittingComp>();
		// [v1.2.0] Weapon·Defense Runtime 적용과 복원을 기록할 기존 Fake Apply Adapter입니다.
		FCFFieldFitApplyAdapter ApplyAdapter;
		// [v1.2.0] 후보와 이전 질량 요청을 순서대로 기록할 성공 Mass Runtime입니다.
		FCFFieldFitMassTestRuntime MassRuntime;
		if (!TestNotNull(TEXT("Mass Transaction Success FittingComp"), FittingComponent))
		{
			return false;
		}

		TestTrue(TEXT("Mass Transaction 초기 A Prepare"), FittingComponent->PrepareSortieFittingSnapshot(SnapshotA, TEXT("RoofTurret")));
		TestTrue(TEXT("Mass Transaction 초기 A Commit"), FittingComponent->CommitPreparedSortieFitting(ApplyAdapter));

		// [v1.2.0] Weapon·Defense와 질량을 하나의 Field Runtime Transaction으로 묶을 Adapter입니다.
		FCFFieldFitRuntimeAdapter RuntimeTransaction(FittingComponent, ApplyAdapter, MassRuntime);
		// [v1.2.0] Runtime Transaction 실패 진단을 받을 문자열입니다.
		FString RuntimeFailureSummary;
		TestTrue(TEXT("Mass Transaction Heavy Prepare"), RuntimeTransaction.PrepareRuntime(SnapshotHeavy, TEXT("RoofTurret"), RuntimeFailureSummary));
		TestTrue(TEXT("Mass Transaction Heavy Commit"), RuntimeTransaction.CommitRuntime(RuntimeFailureSummary));
		TestEqual(TEXT("Mass Transaction 후보 Fitting Applied"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("MassTxnHeavy")));
		TestEqual(TEXT("Mass Transaction 후보 질량 적용 1회"), MassRuntime.RequestedMassesKg.Num(), 1);
		if (MassRuntime.RequestedMassesKg.Num() >= 1)
		{
			TestTrue(TEXT("Mass Transaction 후보 질량 값"), FMath::IsNearlyEqual(MassRuntime.RequestedMassesKg[0], SnapshotHeavy.TotalVehicleMassKg, 0.01f));
		}

		TestTrue(TEXT("Mass Transaction Compensation 성공"), RuntimeTransaction.CompensateCommittedRuntime(RuntimeFailureSummary));
		TestEqual(TEXT("Mass Transaction 이전 Fitting 복원"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("MassTxnAppliedA")));
		TestEqual(TEXT("Mass Transaction 후보+보상 질량 2회"), MassRuntime.RequestedMassesKg.Num(), 2);
		if (MassRuntime.RequestedMassesKg.Num() >= 2)
		{
			TestTrue(TEXT("Mass Transaction 이전 질량 복원 값"), FMath::IsNearlyEqual(MassRuntime.RequestedMassesKg[1], SnapshotA.TotalVehicleMassKg, 0.01f));
		}
	}

	{
		// [v1.2.0] 후보 질량 Apply 실패 즉시 원복을 검증할 독립 FittingComp입니다.
		UCFVehicleFittingComp* FittingComponent = NewObject<UCFVehicleFittingComp>();
		// [v1.2.0] 후보 Weapon·Defense 적용 후 Checkpoint 복원을 확인할 Fake Apply Adapter입니다.
		FCFFieldFitApplyAdapter ApplyAdapter;
		// [v1.2.0] 첫 후보 질량 Apply만 실패하고 두 번째 이전 질량 복원은 성공할 Mass Runtime입니다.
		FCFFieldFitMassTestRuntime FailingMassRuntime;
		FailingMassRuntime.FailOnCallNumber = 1;
		if (!TestNotNull(TEXT("Mass Failure FittingComp"), FittingComponent))
		{
			return false;
		}

		TestTrue(TEXT("Mass Failure 초기 A Prepare"), FittingComponent->PrepareSortieFittingSnapshot(SnapshotA, TEXT("RoofTurret")));
		TestTrue(TEXT("Mass Failure 초기 A Commit"), FittingComponent->CommitPreparedSortieFitting(ApplyAdapter));

		// [v1.2.0] 실패 주입 Mass Runtime을 포함한 실제 Field Runtime Transaction입니다.
		FCFFieldFitRuntimeAdapter RuntimeTransaction(FittingComponent, ApplyAdapter, FailingMassRuntime);
		// [v1.2.0] 후보 질량 실패 상세를 확인할 진단 문자열입니다.
		FString RuntimeFailureSummary;
		TestTrue(TEXT("Mass Failure Heavy Prepare"), RuntimeTransaction.PrepareRuntime(SnapshotHeavy, TEXT("RoofTurret"), RuntimeFailureSummary));
		TestFalse(TEXT("Mass Failure Heavy Commit 실패"), RuntimeTransaction.CommitRuntime(RuntimeFailureSummary));
		TestTrue(TEXT("Mass Failure 요약 포함"), RuntimeFailureSummary.Contains(TEXT("MassReapplyFailed")));
		TestEqual(TEXT("Mass Failure 직전 Fitting 즉시 복원"), FittingComponent->GetAppliedFittingSnapshot().FittingId, FName(TEXT("MassTxnAppliedA")));
		TestEqual(TEXT("Mass Failure 후보+이전 질량 요청 2회"), FailingMassRuntime.RequestedMassesKg.Num(), 2);
		if (FailingMassRuntime.RequestedMassesKg.Num() >= 2)
		{
			TestTrue(TEXT("Mass Failure 첫 요청 후보 질량"), FMath::IsNearlyEqual(FailingMassRuntime.RequestedMassesKg[0], SnapshotHeavy.TotalVehicleMassKg, 0.01f));
			TestTrue(TEXT("Mass Failure 두 번째 요청 이전 질량"), FMath::IsNearlyEqual(FailingMassRuntime.RequestedMassesKg[1], SnapshotA.TotalVehicleMassKg, 0.01f));
		}
	}

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitMassRecoveryClassificationTest,
	"CarFight.Fitting.FFIT_P0_04.MassRecoveryClassification",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.3.0] 후보 질량 적용과 이전 질량 복원이 모두 실패하면 Coordinator가 RecoveryFailed로 승격하는지 검증합니다.
bool FCFFieldFitMassRecoveryClassificationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.3.0] 직전 Applied 상태에 사용할 정상 장비입니다.
	const FCFFieldFitEquipment EquipmentA = CreateFieldFitEquipment(GetTransientPackage(), TEXT("RecoveryMassA"), 80.0f);
	// [v1.3.0] 질량 변경을 요구할 후보 장비입니다.
	const FCFFieldFitEquipment EquipmentHeavy = CreateFieldFitEquipment(GetTransientPackage(), TEXT("RecoveryMassHeavy"), 180.0f);
	// [v1.3.0] 두 Snapshot이 공유할 VehicleData입니다.
	UCFVehicleData* VehicleData = CreateFieldFitVehicleData(GetTransientPackage(), EquipmentA.EquipmentPresetData);
	// [v1.3.0] 직전 Applied Snapshot입니다.
	const FCFVehicleFittingSnapshot SnapshotA = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("RecoveryMassA"), EquipmentA.EquipmentPresetData);
	// [v1.3.0] 후보 mass-changing Snapshot입니다.
	const FCFVehicleFittingSnapshot SnapshotHeavy = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("RecoveryMassHeavy"), EquipmentHeavy.EquipmentPresetData);
	if (!TestTrue(TEXT("Mass Recovery Snapshot A 유효"), SnapshotA.IsValid())
		|| !TestTrue(TEXT("Mass Recovery Heavy Snapshot 유효"), SnapshotHeavy.IsValid()))
	{
		return false;
	}

	// [v1.3.0] Coordinator가 Rollback할 실제 Inventory Fixture입니다.
	FCFFieldFitInventoryFixture InventoryFixture;
	if (!BuildFieldFitInventoryFixture(*this, InventoryFixture))
	{
		return false;
	}

	// [v1.3.0] 직전 Applied Weapon·Defense 상태를 먼저 구성할 FittingComp입니다.
	UCFVehicleFittingComp* FittingComponent = NewObject<UCFVehicleFittingComp>();
	// [v1.3.0] 정상 Weapon·Defense Apply Adapter입니다.
	FCFFieldFitApplyAdapter ApplyAdapter;
	// [v1.3.0] 후보 Mass와 이전 Mass 복원을 모두 실패시킬 Mass Runtime입니다.
	FCFFieldFitMassTestRuntime MassRuntime;
	MassRuntime.bFailEveryCall = true;
	if (!TestNotNull(TEXT("Mass Recovery FittingComp"), FittingComponent))
	{
		return false;
	}
	TestTrue(TEXT("Mass Recovery 초기 A Prepare"), FittingComponent->PrepareSortieFittingSnapshot(SnapshotA, TEXT("RoofTurret")));
	TestTrue(TEXT("Mass Recovery 초기 A Commit"), FittingComponent->CommitPreparedSortieFitting(ApplyAdapter));

	// [v1.3.0] Weapon·Defense와 Mass를 하나의 Runtime Transaction에 연결합니다.
	FCFFieldFitRuntimeAdapter RuntimeTransaction(FittingComponent, ApplyAdapter, MassRuntime);
	// [v1.3.0] 복구 실패를 Coordinator가 RecoveryFailed로 승격해야 하는 결과입니다.
	const FCFFieldFitCompletionResult CompletionResult = FCFFieldFitCoordinator::Complete(
		InventoryFixture.Containers,
		TArray<FCFInventoryContainerMassSnapshot>(),
		InventoryFixture.TransferLedger,
		RuntimeTransaction,
		BuildCompletionRequest(InventoryFixture, SnapshotHeavy));

	TestEqual(TEXT("Mass 복구 실패 RecoveryFailed"), CompletionResult.CompletionStatus, ECFFieldFitCompletionStatus::RecoveryFailed);
	TestEqual(TEXT("Mass 복구 실패 Inventory RolledBack"), InventoryFixture.TransferLedger.GetTransactionState(InventoryFixture.TransferRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
	TestEqual(TEXT("Mass 복구 실패 Reservation 0"), InventoryFixture.TransferLedger.GetActiveReservationCount(), 0);
	TestEqual(TEXT("Mass 적용+복원 실패 2회"), MassRuntime.RequestedMassesKg.Num(), 2);
	TestTrue(TEXT("Mass 복구 실패 요약"), CompletionResult.FailureSummary.Contains(TEXT("RuntimeRecovered=No")));
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitRuntimeRecoveryClassificationTest,
	"CarFight.Fitting.FFIT_P0_04.RuntimeRecoveryClassification",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.3.0] FittingComp 내부 Weapon·Defense Rollback 실패가 Coordinator RecoveryFailed로 승격되는지 검증합니다.
bool FCFFieldFitRuntimeRecoveryClassificationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.3.0] 직전 Applied 상태와 same-mass 후보를 만들 장비 두 개입니다.
	const FCFFieldFitEquipment EquipmentA = CreateFieldFitEquipment(GetTransientPackage(), TEXT("RecoveryRuntimeA"), 80.0f);
	const FCFFieldFitEquipment EquipmentB = CreateFieldFitEquipment(GetTransientPackage(), TEXT("RecoveryRuntimeB"), 80.0f);
	// [v1.3.0] 두 Snapshot이 공유할 VehicleData입니다.
	UCFVehicleData* VehicleData = CreateFieldFitVehicleData(GetTransientPackage(), EquipmentA.EquipmentPresetData);
	const FCFVehicleFittingSnapshot SnapshotA = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("RecoveryRuntimeA"), EquipmentA.EquipmentPresetData);
	const FCFVehicleFittingSnapshot SnapshotB = BuildFieldFitSnapshot(GetTransientPackage(), VehicleData, TEXT("RecoveryRuntimeB"), EquipmentB.EquipmentPresetData);
	if (!TestTrue(TEXT("Runtime Recovery Snapshot A 유효"), SnapshotA.IsValid())
		|| !TestTrue(TEXT("Runtime Recovery Snapshot B 유효"), SnapshotB.IsValid()))
	{
		return false;
	}

	// [v1.3.0] Runtime 실패 뒤 Inventory Rollback을 확인할 독립 Fixture입니다.
	FCFFieldFitInventoryFixture InventoryFixture;
	if (!BuildFieldFitInventoryFixture(*this, InventoryFixture))
	{
		return false;
	}

	// [v1.3.0] 이전 Applied 상태는 정상 Adapter로 구성합니다.
	UCFVehicleFittingComp* FittingComponent = NewObject<UCFVehicleFittingComp>();
	FCFFieldFitApplyAdapter InitialApplyAdapter;
	FCFFieldFitRollbackFailApplyAdapter FailingApplyAdapter;
	if (!TestNotNull(TEXT("Runtime Recovery FittingComp"), FittingComponent))
	{
		return false;
	}
	TestTrue(TEXT("Runtime Recovery 초기 A Prepare"), FittingComponent->PrepareSortieFittingSnapshot(SnapshotA, TEXT("RoofTurret")));
	TestTrue(TEXT("Runtime Recovery 초기 A Commit"), FittingComponent->CommitPreparedSortieFitting(InitialApplyAdapter));

	// [v1.3.0] 후보 Weapon 적용과 내부 Rollback을 모두 실패시키는 Runtime Transaction입니다.
	FCFFieldFitRuntimeAdapter RuntimeTransaction(FittingComponent, FailingApplyAdapter);
	const FCFFieldFitCompletionResult CompletionResult = FCFFieldFitCoordinator::Complete(
		InventoryFixture.Containers,
		TArray<FCFInventoryContainerMassSnapshot>(),
		InventoryFixture.TransferLedger,
		RuntimeTransaction,
		BuildCompletionRequest(InventoryFixture, SnapshotB));

	TestEqual(TEXT("Runtime 내부 복구 실패 RecoveryFailed"), CompletionResult.CompletionStatus, ECFFieldFitCompletionStatus::RecoveryFailed);
	TestEqual(TEXT("Runtime 내부 복구 실패 Inventory RolledBack"), InventoryFixture.TransferLedger.GetTransactionState(InventoryFixture.TransferRequest.TransactionId), ECFInventoryTransactionState::RolledBack);
	TestTrue(TEXT("Runtime 내부 복구 실패 Weapon 재시도"), FailingApplyAdapter.WeaponApplyCallCount >= 2);
	TestFalse(TEXT("FittingComp 내부 복구 실패 상태"), FittingComponent->WasLastCommitFailureRecovered());
	TestTrue(TEXT("Runtime 내부 복구 실패 요약"), CompletionResult.FailureSummary.Contains(TEXT("RuntimeRecovered=No")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
