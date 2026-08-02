// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CF-FQ-035 INV-P0-04 Inventory Fitting Adapter Pawn 없는 자동화 테스트
// Scope: 실제 Item Binding, 소유·접근·예약·Definition·중복 선택과 기존 BuildFittingSnapshot 연결을 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Inventory.INV_P0_04.FittingAdapter 종합 테스트를 최초 추가.
// Migration:
// - Transient UObject와 순수 Container·Ledger만 사용하며 Pawn, World, Runtime Component와 Unreal Asset을 생성하거나 수정하지 않는다.
// - Inventory 이동 Commit, Runtime Apply와 Field Timed Action은 호출하지 않는다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFInventoryFitAdapter.h"

#include "CFEquipmentPresetData.h"
#include "CFInventoryItemData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

namespace
{
	/** 테스트 EquipmentPresetData와 하위 DataAsset 묶음입니다. */
	struct FCFInventoryFitTestEquipment
	{
		// [v1.0.0] Inventory Definition에서 해석할 장비 프리셋입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] 장비 프리셋이 참조할 터렛 마운트입니다.
		UCFTurretMountData* TurretMountData = nullptr;

		// [v1.0.0] 장비 프리셋이 참조할 무기 데이터입니다.
		UCFWeaponData* WeaponData = nullptr;
	};

	/** 정상·거부 경로가 공유할 Transient Fixture입니다. */
	struct FCFInventoryFitFixture
	{
		// [v1.0.0] 두 MountProfile과 질량 기준을 소유할 차량 데이터입니다.
		UCFVehicleData* VehicleData = nullptr;

		// [v1.0.0] 두 실제 Item이 공유할 정상 장비 묶음입니다.
		FCFInventoryFitTestEquipment SharedEquipment;

		// [v1.0.0] 기존 Fitting 호환 실패를 만들 Launcher 장비 묶음입니다.
		FCFInventoryFitTestEquipment IncompatibleEquipment;

		// [v1.0.0] 정상 Equipment Item Definition입니다.
		UCFEquipmentItemData* EquipmentDefinition = nullptr;

		// [v1.0.0] 비호환 Equipment Item Definition입니다.
		UCFEquipmentItemData* IncompatibleDefinition = nullptr;

		// [v1.0.0] Defense Item Definition입니다.
		UCFDefenseItemData* DefenseDefinition = nullptr;

		// [v1.0.0] 정상 Defense Override 데이터입니다.
		UCFVehicleDefenseData* DefenseData = nullptr;

		// [v1.0.0] Mount_A에 선택할 Equipment Item입니다.
		FCFInventoryItemInstance FirstEquipmentItem;

		// [v1.0.0] Mount_B에 선택할 같은 Definition의 다른 Equipment Item입니다.
		FCFInventoryItemInstance SecondEquipmentItem;

		// [v1.0.0] Reservation 선택 거부를 검증할 Equipment Item입니다.
		FCFInventoryItemInstance ReservedEquipmentItem;

		// [v1.0.0] 기존 Fitting 호환 실패에 사용할 Equipment Item입니다.
		FCFInventoryItemInstance IncompatibleEquipmentItem;

		// [v1.0.0] Defense Override에 선택할 실제 Item입니다.
		FCFInventoryItemInstance DefenseItem;

		// [v1.0.0] 다른 차량 소유권 접근 거부를 검증할 Item입니다.
		FCFInventoryItemInstance OtherVehicleItem;

		// [v1.0.0] 현재 차량 Inventory Owner ID입니다.
		FCFInventoryOwnerId CurrentOwnerId;

		// [v1.0.0] 다른 차량 Inventory Owner ID입니다.
		FCFInventoryOwnerId OtherOwnerId;

		// [v1.0.0] 현재 차량 Cargo Container ID입니다.
		FCFInventoryContainerId CargoContainerId;

		// [v1.0.0] 현재 차량 Mounted Container ID입니다.
		FCFInventoryContainerId MountedContainerId;

		// [v1.0.0] 현재·다른 차량의 Item 소유 상태입니다.
		TArray<FCFInventoryContainerState> Containers;

		// [v1.0.0] ItemDefinitionId 해석에 사용할 Definition 목록입니다.
		TArray<UCFInventoryItemData*> ItemDefinitions;

		// [v1.0.0] ReservedEquipmentItem의 Active Reservation을 소유합니다.
		FCFInventoryTransferLedger TransferLedger;
	};

	// [v1.0.0] 지정 호환과 질량을 가진 완성된 EquipmentPresetData 묶음을 생성합니다.
	FCFInventoryFitTestEquipment CreateTestEquipment(
		UObject* Outer,
		const FName EquipmentId,
		const ECFVehicleMountType MountType,
		const float MountMassKg,
		const float WeaponMassKg)
	{
		// [v1.0.0] 생성된 장비 DataAsset 묶음입니다.
		FCFInventoryFitTestEquipment TestEquipment;
		TestEquipment.TurretMountData = NewObject<UCFTurretMountData>(Outer);
		TestEquipment.TurretMountData->TurretMountId = FName(*FString::Printf(TEXT("%s_Mount"), *EquipmentId.ToString()));
		TestEquipment.TurretMountData->TurretMountWeightKg = MountMassKg;
		TestEquipment.WeaponData = NewObject<UCFWeaponData>(Outer);
		TestEquipment.WeaponData->WeaponId = FName(*FString::Printf(TEXT("%s_Weapon"), *EquipmentId.ToString()));
		TestEquipment.WeaponData->WeaponSize = ECFVehicleWeaponSize::Medium;
		TestEquipment.WeaponData->CompatibleMountTypes.Add(MountType);
		TestEquipment.WeaponData->WeaponMassKg = WeaponMassKg;
		TestEquipment.EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		TestEquipment.EquipmentPresetData->EquipmentId = EquipmentId;
		TestEquipment.EquipmentPresetData->RequiredMountType = MountType;
		TestEquipment.EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;
		TestEquipment.EquipmentPresetData->DefaultTurretMountData = TestEquipment.TurretMountData;
		TestEquipment.EquipmentPresetData->DefaultWeaponData = TestEquipment.WeaponData;
		return TestEquipment;
	}

	// [v1.0.0] VehicleData에 한 하드포인트와 Turret MountProfile을 추가합니다.
	void AddTestMount(UCFVehicleData* VehicleData, const FName MountProfileId, const FName LocationSlotId)
	{
		// [v1.0.0] 추가할 하드포인트 위치입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = LocationSlotId;
		HardpointSlot.LocationCategory = TEXT("InventoryFitTest");
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] 추가할 Turret·Medium MountProfile입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = MountProfileId;
		MountProfile.LocationSlotRef = LocationSlotId;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Medium;
		VehicleData->MountProfiles.Add(MountProfile);
	}

	// [v1.0.0] Container에 실제 Item Instance Entry를 추가합니다.
	void AddTestEntry(FCFInventoryContainerState& Container, const FCFInventoryItemInstance& ItemInstance, const FName SlotId)
	{
		// [v1.0.0] 추가할 Item 소유 Entry입니다.
		FCFInventoryContainerEntry Entry;
		Entry.ItemInstance = ItemInstance;
		Entry.ContainerSlotId = SlotId;
		Container.Entries.Add(Entry);
	}

	// [v1.0.0] 정상 Binding과 주요 거부 경로를 검증할 Fixture를 생성합니다.
	bool BuildFixture(FAutomationTestBase& Test, FCFInventoryFitFixture& OutFixture)
	{
		OutFixture.VehicleData = NewObject<UCFVehicleData>();
		if (!Test.TestNotNull(TEXT("VehicleData 생성"), OutFixture.VehicleData))
		{
			return false;
		}
		OutFixture.VehicleData->BaseVehicleMassKg = 1000.0f;
		OutFixture.VehicleData->MaximumGrossMassKg = 3000.0f;
		AddTestMount(OutFixture.VehicleData, TEXT("Mount_A"), TEXT("Top_A"));
		AddTestMount(OutFixture.VehicleData, TEXT("Mount_B"), TEXT("Top_B"));

		OutFixture.SharedEquipment = CreateTestEquipment(OutFixture.VehicleData, TEXT("SharedTurret"), ECFVehicleMountType::Turret, 100.0f, 150.0f);
		OutFixture.IncompatibleEquipment = CreateTestEquipment(OutFixture.VehicleData, TEXT("LauncherOnly"), ECFVehicleMountType::Launcher, 80.0f, 120.0f);

		OutFixture.EquipmentDefinition = NewObject<UCFEquipmentItemData>();
		OutFixture.EquipmentDefinition->ItemDefinitionId = TEXT("SharedTurretItem");
		OutFixture.EquipmentDefinition->EquipmentPresetData = OutFixture.SharedEquipment.EquipmentPresetData;
		OutFixture.IncompatibleDefinition = NewObject<UCFEquipmentItemData>();
		OutFixture.IncompatibleDefinition->ItemDefinitionId = TEXT("LauncherOnlyItem");
		OutFixture.IncompatibleDefinition->EquipmentPresetData = OutFixture.IncompatibleEquipment.EquipmentPresetData;
		OutFixture.DefenseData = NewObject<UCFVehicleDefenseData>();
		OutFixture.DefenseData->DefenseId = TEXT("InventoryDefense");
		OutFixture.DefenseData->DefenseMassKg = 100.0f;
		OutFixture.DefenseDefinition = NewObject<UCFDefenseItemData>();
		OutFixture.DefenseDefinition->ItemDefinitionId = TEXT("InventoryDefenseItem");
		OutFixture.DefenseDefinition->VehicleDefenseData = OutFixture.DefenseData;

		OutFixture.FirstEquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentDefinition);
		OutFixture.SecondEquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentDefinition);
		OutFixture.ReservedEquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentDefinition);
		OutFixture.IncompatibleEquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.IncompatibleDefinition);
		OutFixture.DefenseItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.DefenseDefinition);
		OutFixture.OtherVehicleItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentDefinition);

		OutFixture.CurrentOwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.OtherOwnerId = FCFInventoryOwnerId::CreateNew();
		OutFixture.CargoContainerId = FCFInventoryContainerId::CreateNew();
		OutFixture.MountedContainerId = FCFInventoryContainerId::CreateNew();

		// [v1.0.0] 현재 차량에서 선택 가능한 Item들을 소유할 Cargo입니다.
		FCFInventoryContainerState CargoContainer;
		CargoContainer.ContainerRef.OwnerId = OutFixture.CurrentOwnerId;
		CargoContainer.ContainerRef.ContainerId = OutFixture.CargoContainerId;
		CargoContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		CargoContainer.Capacity.MaximumSlotCount = 8;
		AddTestEntry(CargoContainer, OutFixture.FirstEquipmentItem, TEXT("Cargo_First"));
		AddTestEntry(CargoContainer, OutFixture.SecondEquipmentItem, TEXT("Cargo_Second"));
		AddTestEntry(CargoContainer, OutFixture.ReservedEquipmentItem, TEXT("Cargo_Reserved"));
		AddTestEntry(CargoContainer, OutFixture.IncompatibleEquipmentItem, TEXT("Cargo_Incompatible"));
		AddTestEntry(CargoContainer, OutFixture.DefenseItem, TEXT("Cargo_Defense"));

		// [v1.0.0] Reservation 목적지로 사용할 현재 차량 MountedEquipment입니다.
		FCFInventoryContainerState MountedContainer;
		MountedContainer.ContainerRef.OwnerId = OutFixture.CurrentOwnerId;
		MountedContainer.ContainerRef.ContainerId = OutFixture.MountedContainerId;
		MountedContainer.ContainerRef.ContainerType = ECFInventoryContainerType::MountedEquipment;
		MountedContainer.Capacity.MaximumSlotCount = 4;

		// [v1.0.0] 현재 차량 AccessContext에서 거부할 다른 차량 Cargo입니다.
		FCFInventoryContainerState OtherCargoContainer;
		OtherCargoContainer.ContainerRef.OwnerId = OutFixture.OtherOwnerId;
		OtherCargoContainer.ContainerRef.ContainerId = FCFInventoryContainerId::CreateNew();
		OtherCargoContainer.ContainerRef.ContainerType = ECFInventoryContainerType::VehicleCargo;
		OtherCargoContainer.Capacity.MaximumSlotCount = 2;
		AddTestEntry(OtherCargoContainer, OutFixture.OtherVehicleItem, TEXT("Other_First"));

		OutFixture.Containers.Add(CargoContainer);
		OutFixture.Containers.Add(MountedContainer);
		OutFixture.Containers.Add(OtherCargoContainer);
		OutFixture.ItemDefinitions.Add(OutFixture.EquipmentDefinition);
		OutFixture.ItemDefinitions.Add(OutFixture.IncompatibleDefinition);
		OutFixture.ItemDefinitions.Add(OutFixture.DefenseDefinition);

		// [v1.0.0] Fixture Container 불변식 오류 목록입니다.
		TArray<FText> ValidationErrors;
		if (!Test.TestTrue(TEXT("Container 집합 유효"), FCFInventoryAccessQuery::ValidateContainerSet(OutFixture.Containers, ValidationErrors)))
		{
			return false;
		}

		// [v1.0.0] ReservedEquipmentItem을 잠글 Prepare 요청입니다.
		FCFInventoryTransferRequest ReservationRequest;
		ReservationRequest.TransactionId = FCFInventoryTransactionId::CreateNew();
		ReservationRequest.ActionOwnerId = TEXT("FitAdapterReservation");
		ReservationRequest.ItemInstanceId = OutFixture.ReservedEquipmentItem.ItemHandle.ItemInstanceId;
		ReservationRequest.SourceContainerId = OutFixture.CargoContainerId;
		ReservationRequest.SourceSlotId = TEXT("Cargo_Reserved");
		ReservationRequest.DestinationContainerId = OutFixture.MountedContainerId;
		ReservationRequest.DestinationSlotId = TEXT("ReservedMount");

		// [v1.0.0] Adapter가 거부해야 할 Active Item Reservation 생성 결과입니다.
		const FCFInventoryTransferResult ReservationResult = OutFixture.TransferLedger.PrepareTransfer(
			OutFixture.Containers,
			TArray<FCFInventoryContainerMassSnapshot>(),
			ReservationRequest);
		return Test.TestEqual(TEXT("Reservation Prepare 성공"), ReservationResult.TransferStatus, ECFInventoryTransferStatus::Prepared);
	}

	// [v1.0.0] 같은 Definition의 서로 다른 Equipment Item 두 개와 Defense Item을 선택하는 정상 요청을 생성합니다.
	FCFInventoryFitRequest CreateValidRequest(const FCFInventoryFitFixture& Fixture)
	{
		// [v1.0.0] 입력 배열 순서 독립성을 검증할 정상 요청입니다.
		FCFInventoryFitRequest Request;
		Request.FittingId = TEXT("InventoryBoundFitting");
		Request.VehicleData = Fixture.VehicleData;
		Request.MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
		Request.AccessContext.CurrentVehicleOwnerId = Fixture.CurrentOwnerId;

		// [v1.0.0] VehicleData와 반대 순서로 넣을 Mount_B 선택입니다.
		FCFInventoryFitMountInput MountBInput;
		MountBInput.MountProfileId = TEXT("Mount_B");
		MountBInput.ItemInstanceId = Fixture.SecondEquipmentItem.ItemHandle.ItemInstanceId;
		Request.MountSelections.Add(MountBInput);

		// [v1.0.0] VehicleData와 반대 순서로 넣을 Mount_A 선택입니다.
		FCFInventoryFitMountInput MountAInput;
		MountAInput.MountProfileId = TEXT("Mount_A");
		MountAInput.ItemInstanceId = Fixture.FirstEquipmentItem.ItemHandle.ItemInstanceId;
		Request.MountSelections.Add(MountAInput);

		Request.DefenseSelection.SelectionMode = ECFDefenseSelectionMode::Override;
		Request.DefenseSelection.ItemInstanceId = Fixture.DefenseItem.ItemHandle.ItemInstanceId;
		return Request;
	}

	// [v1.0.0] Mount Binding의 결정론적 순서와 실제 Item ID 서명을 생성합니다.
	FString BuildBindingSignature(const FCFInventoryFitResult& Result)
	{
		// [v1.0.0] 순서대로 결합할 Binding 문자열입니다.
		TArray<FString> BindingTexts;
		for (const FCFInventoryFitMountBinding& Binding : Result.MountBindings)
		{
			BindingTexts.Add(FString::Printf(TEXT("%s:%s"), *Binding.MountProfileId.ToString(), *Binding.ItemInstanceId.ToString()));
		}
		return FString::Join(BindingTexts, TEXT("|"));
	}

	// [v1.0.0] 기존 Fitting Snapshot에 지정 문제 코드가 존재하는지 반환합니다.
	bool HasSnapshotIssue(const FCFVehicleFittingSnapshot& Snapshot, const ECFFittingIssueCode IssueCode, const FName MountProfileId)
	{
		for (const FCFFittingValidationIssue& Issue : Snapshot.ValidationIssues)
		{
			if (Issue.IssueCode == IssueCode && Issue.MountProfileId == MountProfileId)
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryFittingAdapterTest,
	"CarFight.Inventory.INV_P0_04.FittingAdapter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Item Binding, 소유·접근·예약·Definition·중복 선택과 기존 Snapshot 연결을 종합 검증합니다.
bool FCFInventoryFittingAdapterTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 테스트 전체가 공유할 Transient Fixture입니다.
	FCFInventoryFitFixture Fixture;
	if (!BuildFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] 정상 Adapter 요청입니다.
	FCFInventoryFitRequest ValidRequest = CreateValidRequest(Fixture);

	// [v1.0.0] Adapter 호출 전 Cargo Entry 수입니다.
	const int32 CargoEntryCount = Fixture.Containers[0].Entries.Num();

	// [v1.0.0] Adapter 호출 전 Mounted Entry 수입니다.
	const int32 MountedEntryCount = Fixture.Containers[1].Entries.Num();

	// [v1.0.0] 정상 Item Binding과 기존 Snapshot 결과입니다.
	const FCFInventoryFitResult ValidResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		ValidRequest);
	TestTrue(TEXT("정상 Adapter 성공"), ValidResult.IsSuccessful());
	TestEqual(TEXT("정상 상태 Success"), ValidResult.AdapterStatus, ECFInventoryFitStatus::Success);
	TestEqual(TEXT("Mount Binding 2개"), ValidResult.MountBindings.Num(), 2);
	TestEqual(TEXT("VehicleData 순서 첫 Mount_A"), ValidResult.MountBindings[0].MountProfileId, FName(TEXT("Mount_A")));
	TestEqual(TEXT("VehicleData 순서 둘째 Mount_B"), ValidResult.MountBindings[1].MountProfileId, FName(TEXT("Mount_B")));
	TestTrue(TEXT("같은 Definition ID 보존"), ValidResult.MountBindings[0].ItemDefinitionId == ValidResult.MountBindings[1].ItemDefinitionId);
	TestTrue(TEXT("같은 Definition의 Instance ID 구분"), ValidResult.MountBindings[0].ItemInstanceId != ValidResult.MountBindings[1].ItemInstanceId);
	TestTrue(TEXT("EquipmentPresetData 해석"), ValidResult.MountBindings[0].ResolvedEquipmentPresetData == Fixture.SharedEquipment.EquipmentPresetData);
	TestTrue(TEXT("Defense Item Binding"), ValidResult.DefenseBinding.bHasItemBinding);
	TestTrue(TEXT("VehicleDefenseData 해석"), ValidResult.DefenseBinding.ResolvedDefenseData == Fixture.DefenseData);
	TestTrue(TEXT("기존 BuildFittingSnapshot 유효"), ValidResult.FittingSnapshot.IsValid());
	TestEqual(TEXT("장비 질량 500kg"), ValidResult.FittingSnapshot.EquipmentMassKg, 500.0f);
	TestEqual(TEXT("방어 질량 100kg"), ValidResult.FittingSnapshot.DefenseMassKg, 100.0f);
	TestEqual(TEXT("총중량 1600kg"), ValidResult.FittingSnapshot.TotalVehicleMassKg, 1600.0f);
	TestEqual(TEXT("Cargo 무변경"), Fixture.Containers[0].Entries.Num(), CargoEntryCount);
	TestEqual(TEXT("Mounted 무변경"), Fixture.Containers[1].Entries.Num(), MountedEntryCount);

	// [v1.0.0] Mount 입력 배열 순서를 바꾼 결정론 비교 요청입니다.
	FCFInventoryFitRequest ReorderedRequest = ValidRequest;
	ReorderedRequest.MountSelections.Swap(0, 1);

	// [v1.0.0] 입력 순서 변경 뒤 생성한 Adapter 결과입니다.
	const FCFInventoryFitResult ReorderedResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		ReorderedRequest);
	TestTrue(TEXT("순서 변경 Adapter 성공"), ReorderedResult.IsSuccessful());
	TestEqual(TEXT("결정론적 Binding 서명"), BuildBindingSignature(ReorderedResult), BuildBindingSignature(ValidResult));
	TestEqual(TEXT("결정론적 총중량"), ReorderedResult.FittingSnapshot.TotalVehicleMassKg, ValidResult.FittingSnapshot.TotalVehicleMassKg);

	// [v1.0.0] 같은 실제 Item을 두 Mount에 선택한 요청입니다.
	FCFInventoryFitRequest DuplicateItemRequest = ValidRequest;
	DuplicateItemRequest.MountSelections[0].ItemInstanceId = Fixture.FirstEquipmentItem.ItemHandle.ItemInstanceId;
	DuplicateItemRequest.MountSelections[1].ItemInstanceId = Fixture.FirstEquipmentItem.ItemHandle.ItemInstanceId;

	// [v1.0.0] 동일 ItemInstanceId 복수 Mount 거부 결과입니다.
	const FCFInventoryFitResult DuplicateItemResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		DuplicateItemRequest);
	TestEqual(TEXT("복수 Mount 중복 Item 거부"), DuplicateItemResult.AdapterStatus, ECFInventoryFitStatus::DuplicateItemSelection);

	// [v1.0.0] Active Reservation Item을 Mount_A에 선택한 요청입니다.
	FCFInventoryFitRequest ReservedItemRequest = ValidRequest;
	ReservedItemRequest.MountSelections[1].ItemInstanceId = Fixture.ReservedEquipmentItem.ItemHandle.ItemInstanceId;

	// [v1.0.0] 예약 중 Item 선택 거부 결과입니다.
	const FCFInventoryFitResult ReservedItemResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		ReservedItemRequest);
	TestEqual(TEXT("예약 Item 선택 거부"), ReservedItemResult.AdapterStatus, ECFInventoryFitStatus::ItemReserved);
	TestEqual(TEXT("예약 실패 Mount_A"), ReservedItemResult.FailureMountProfileId, FName(TEXT("Mount_A")));

	// [v1.0.0] 다른 차량 Owner의 Item을 Mount_A에 선택한 요청입니다.
	FCFInventoryFitRequest OtherOwnerRequest = ValidRequest;
	OtherOwnerRequest.MountSelections[1].ItemInstanceId = Fixture.OtherVehicleItem.ItemHandle.ItemInstanceId;

	// [v1.0.0] 다른 차량 Item 접근 거부 결과입니다.
	const FCFInventoryFitResult OtherOwnerResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		OtherOwnerRequest);
	TestEqual(TEXT("다른 차량 소유 Item 거부"), OtherOwnerResult.AdapterStatus, ECFInventoryFitStatus::ItemInaccessible);

	// [v1.0.0] 현재 차량 Cargo 접근을 차단한 요청입니다.
	FCFInventoryFitRequest AccessBlockedRequest = ValidRequest;
	AccessBlockedRequest.AccessContext.bAllowVehicleCargo = false;

	// [v1.0.0] 접근 불가 Container Item 거부 결과입니다.
	const FCFInventoryFitResult AccessBlockedResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		AccessBlockedRequest);
	TestEqual(TEXT("Cargo 접근 차단 Item 거부"), AccessBlockedResult.AdapterStatus, ECFInventoryFitStatus::ItemInaccessible);

	// [v1.0.0] 정상 Equipment Definition을 제거한 Registry입니다.
	TArray<UCFInventoryItemData*> MissingDefinitionRegistry;
	MissingDefinitionRegistry.Add(Fixture.IncompatibleDefinition);
	MissingDefinitionRegistry.Add(Fixture.DefenseDefinition);

	// [v1.0.0] ItemDefinitionId 해석 실패 결과입니다.
	const FCFInventoryFitResult MissingDefinitionResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		MissingDefinitionRegistry,
		Fixture.TransferLedger,
		ValidRequest);
	TestEqual(TEXT("ItemDefinitionId 누락 거부"), MissingDefinitionResult.AdapterStatus, ECFInventoryFitStatus::DefinitionMissing);

	// [v1.0.0] Inventory 검증은 통과하지만 Launcher 장비를 Turret Mount_A에 선택한 요청입니다.
	FCFInventoryFitRequest IncompatibleRequest = ValidRequest;
	IncompatibleRequest.MountSelections[1].ItemInstanceId = Fixture.IncompatibleEquipmentItem.ItemHandle.ItemInstanceId;

	// [v1.0.0] 기존 BuildFittingSnapshot 호환 검증 실패 결과입니다.
	const FCFInventoryFitResult IncompatibleResult = FCFInventoryFitAdapter::BuildFittingBinding(
		Fixture.Containers,
		Fixture.ItemDefinitions,
		Fixture.TransferLedger,
		IncompatibleRequest);
	TestEqual(TEXT("기존 Fitting 검증 실패 전달"), IncompatibleResult.AdapterStatus, ECFInventoryFitStatus::FittingSnapshotInvalid);
	TestFalse(TEXT("비호환 Snapshot 무효"), IncompatibleResult.FittingSnapshot.IsValid());
	TestTrue(TEXT("MountTypeMismatch 문제 보존"), HasSnapshotIssue(IncompatibleResult.FittingSnapshot, ECFFittingIssueCode::MountTypeMismatch, TEXT("Mount_A")));
	TestEqual(TEXT("호환 실패도 Binding 2개 보존"), IncompatibleResult.MountBindings.Num(), 2);
	TestEqual(TEXT("호환 실패 후 Cargo 무변경"), Fixture.Containers[0].Entries.Num(), CargoEntryCount);
	TestEqual(TEXT("호환 실패 후 Mounted 무변경"), Fixture.Containers[1].Entries.Num(), MountedEntryCount);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
