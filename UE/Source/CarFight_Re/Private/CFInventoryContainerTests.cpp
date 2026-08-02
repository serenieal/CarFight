// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-02 Container와 Access Query Pawn 없는 자동화 테스트
// Scope: VehicleCargo·MountedEquipment 위치, 슬롯·질량 Capacity, 현재 차량 접근과 중복 소유 거부를 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Inventory.INV_P0_02.Container와 AccessQuery 테스트를 최초 추가.
// Migration:
// - Pawn, World, Blueprint와 Unreal Asset을 생성하거나 수정하지 않는다.
// - Reservation, Atomic Transfer와 Fitting Runtime 검증은 INV-P0-03~04 테스트로 분리한다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFInventoryContainer.h"
#include "CFInventoryItemData.h"
#include "CFEquipmentPresetData.h"
#include "CFVehicleDefenseData.h"

#include "Misc/AutomationTest.h"

namespace
{
	/**
	 * INV-P0-02 테스트에서 공유할 Transient Definition과 실제 Item Instance 묶음입니다.
	 */
	struct FCFInventoryContainerFixture
	{
		// [v1.0.0] Equipment Item Definition이 참조할 Transient EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] Defense Item Definition이 참조할 Transient VehicleDefenseData입니다.
		UCFVehicleDefenseData* VehicleDefenseData = nullptr;

		// [v1.0.0] Equipment 실제 Item Instance 생성에 사용할 Transient Definition입니다.
		UCFEquipmentItemData* EquipmentItemDefinition = nullptr;

		// [v1.0.0] Defense 실제 Item Instance 생성에 사용할 Transient Definition입니다.
		UCFDefenseItemData* DefenseItemDefinition = nullptr;

		// [v1.0.0] 첫 VehicleCargo 슬롯에 배치할 Equipment 실제 인스턴스입니다.
		FCFInventoryItemInstance CargoEquipmentItem;

		// [v1.0.0] MountedEquipment 슬롯에 배치할 Defense 실제 인스턴스입니다.
		FCFInventoryItemInstance MountedDefenseItem;

		// [v1.0.0] Capacity와 미소유 위치 조회에 사용할 추가 Equipment 실제 인스턴스입니다.
		FCFInventoryItemInstance SpareEquipmentItem;
	};

	// [v1.0.0] Pawn 없이 유효한 Equipment·Defense Definition과 Quantity 1 Item Instance를 생성합니다.
	bool BuildContainerFixture(FAutomationTestBase& Test, FCFInventoryContainerFixture& OutFixture)
	{
		OutFixture.EquipmentPresetData = NewObject<UCFEquipmentPresetData>();
		if (!Test.TestNotNull(TEXT("EquipmentPresetData 생성"), OutFixture.EquipmentPresetData))
		{
			return false;
		}

		OutFixture.VehicleDefenseData = NewObject<UCFVehicleDefenseData>();
		if (!Test.TestNotNull(TEXT("VehicleDefenseData 생성"), OutFixture.VehicleDefenseData))
		{
			return false;
		}

		OutFixture.EquipmentItemDefinition = NewObject<UCFEquipmentItemData>();
		if (!Test.TestNotNull(TEXT("Equipment Item Definition 생성"), OutFixture.EquipmentItemDefinition))
		{
			return false;
		}
		OutFixture.EquipmentItemDefinition->ItemDefinitionId = TEXT("Test_ContainerEquipment");
		OutFixture.EquipmentItemDefinition->EquipmentPresetData = OutFixture.EquipmentPresetData;

		OutFixture.DefenseItemDefinition = NewObject<UCFDefenseItemData>();
		if (!Test.TestNotNull(TEXT("Defense Item Definition 생성"), OutFixture.DefenseItemDefinition))
		{
			return false;
		}
		OutFixture.DefenseItemDefinition->ItemDefinitionId = TEXT("Test_ContainerDefense");
		OutFixture.DefenseItemDefinition->VehicleDefenseData = OutFixture.VehicleDefenseData;

		OutFixture.CargoEquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentItemDefinition);
		OutFixture.MountedDefenseItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.DefenseItemDefinition);
		OutFixture.SpareEquipmentItem = FCFInventoryItemInstance::CreateUniqueItem(OutFixture.EquipmentItemDefinition);

		return Test.TestTrue(TEXT("Cargo Equipment Item 유효"), OutFixture.CargoEquipmentItem.IsValid())
			&& Test.TestTrue(TEXT("Mounted Defense Item 유효"), OutFixture.MountedDefenseItem.IsValid())
			&& Test.TestTrue(TEXT("Spare Equipment Item 유효"), OutFixture.SpareEquipmentItem.IsValid());
	}

	// [v1.0.0] 지정 Owner와 종류를 가진 빈 테스트 Container를 생성합니다.
	FCFInventoryContainerState CreateTestContainer(
		const FCFInventoryOwnerId& OwnerId,
		const ECFInventoryContainerType ContainerType,
		const int32 MaximumSlotCount,
		const bool bUseMassLimit = false,
		const float MaximumMassKg = 0.0f)
	{
		// [v1.0.0] 호출자가 지정한 P0 소유 위치와 Capacity를 가진 테스트 Container입니다.
		FCFInventoryContainerState Container;
		Container.ContainerRef.OwnerId = OwnerId;
		Container.ContainerRef.ContainerId = FCFInventoryContainerId::CreateNew();
		Container.ContainerRef.ContainerType = ContainerType;
		Container.Capacity.MaximumSlotCount = MaximumSlotCount;
		Container.Capacity.bUseMassLimit = bUseMassLimit;
		Container.Capacity.MaximumMassKg = MaximumMassKg;
		return Container;
	}

	// [v1.0.0] Container에 Item Instance와 안정적인 슬롯 위치를 나타내는 Entry를 추가합니다.
	void AddTestContainerEntry(
		FCFInventoryContainerState& Container,
		const FCFInventoryItemInstance& ItemInstance,
		const FName ContainerSlotId)
	{
		// [v1.0.0] 테스트 Container에 추가할 Item 소유 위치 Entry입니다.
		FCFInventoryContainerEntry Entry;
		Entry.ItemInstance = ItemInstance;
		Entry.ContainerSlotId = ContainerSlotId;
		Container.Entries.Add(Entry);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryContainerTest,
	"CarFight.Inventory.INV_P0_02.Container",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryAccessQueryTest,
	"CarFight.Inventory.INV_P0_02.AccessQuery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] VehicleCargo·MountedEquipment 소유 위치, 슬롯·질량 Capacity와 Container 불변식을 검증합니다.
bool FCFInventoryContainerTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Container 테스트에서 공유할 Definition과 실제 Item Instance 묶음입니다.
	FCFInventoryContainerFixture Fixture;
	if (!BuildContainerFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] VehicleCargo와 MountedEquipment가 공유할 현재 차량 Owner ID입니다.
	const FCFInventoryOwnerId CurrentVehicleOwnerId = FCFInventoryOwnerId::CreateNew();
	TestTrue(TEXT("현재 차량 Owner ID 유효"), CurrentVehicleOwnerId.IsValid());

	// [v1.0.0] 슬롯 2개와 100kg 선택적 질량 한도를 가진 VehicleCargo입니다.
	FCFInventoryContainerState VehicleCargo = CreateTestContainer(
		CurrentVehicleOwnerId,
		ECFInventoryContainerType::VehicleCargo,
		2,
		true,
		100.0f);
	AddTestContainerEntry(VehicleCargo, Fixture.CargoEquipmentItem, TEXT("CargoSlot_01"));

	// [v1.0.0] MountProfileId를 슬롯 키로 사용하는 MountedEquipment Container입니다.
	FCFInventoryContainerState MountedEquipment = CreateTestContainer(
		CurrentVehicleOwnerId,
		ECFInventoryContainerType::MountedEquipment,
		2);
	AddTestContainerEntry(MountedEquipment, Fixture.MountedDefenseItem, TEXT("RoofDefense"));

	// [v1.0.0] 개별 Container 계약 오류를 수집할 배열입니다.
	TArray<FText> ValidationErrors;
	TestTrue(TEXT("VehicleCargo 계약 유효"), VehicleCargo.ValidateContainerContract(ValidationErrors));
	TestEqual(TEXT("VehicleCargo 계약 오류 없음"), ValidationErrors.Num(), 0);
	TestTrue(TEXT("MountedEquipment 계약 유효"), MountedEquipment.ValidateContainerContract(ValidationErrors));
	TestEqual(TEXT("MountedEquipment 계약 오류 없음"), ValidationErrors.Num(), 0);
	TestEqual(TEXT("VehicleCargo 사용 슬롯 1"), VehicleCargo.GetOccupiedSlotCount(), 1);
	TestEqual(TEXT("MountedEquipment 사용 슬롯 1"), MountedEquipment.GetOccupiedSlotCount(), 1);

	// [v1.0.0] VehicleCargo에서 기존 Equipment Item의 완전한 위치를 조회할 결과입니다.
	FCFInventoryItemLocation CargoItemLocation;
	TestTrue(TEXT("Cargo Equipment 위치 조회"), VehicleCargo.FindItemLocation(Fixture.CargoEquipmentItem.ItemHandle.ItemInstanceId, CargoItemLocation));
	TestTrue(TEXT("Cargo Equipment 위치 유효"), CargoItemLocation.IsValid());
	TestEqual(TEXT("Cargo 위치 Owner 보존"), CargoItemLocation.ContainerRef.OwnerId, CurrentVehicleOwnerId);
	TestEqual(TEXT("Cargo 위치 Container 종류"), CargoItemLocation.ContainerRef.ContainerType, ECFInventoryContainerType::VehicleCargo);
	TestEqual(TEXT("Cargo 위치 Slot ID"), CargoItemLocation.ContainerSlotId, FName(TEXT("CargoSlot_01")));

	// [v1.0.0] 현재 1슬롯·60kg에서 1슬롯·30kg 추가를 평가한 수용 가능 결과입니다.
	const FCFInventoryCapacityResult AvailableCapacity = VehicleCargo.EvaluateCapacity(60.0f, 1, 30.0f);
	TestTrue(TEXT("VehicleCargo 1슬롯 30kg 추가 가능"), AvailableCapacity.CanAccept());
	TestEqual(TEXT("Capacity 상태 Available"), AvailableCapacity.CapacityState, ECFInventoryCapacityState::Available);
	TestEqual(TEXT("요청 전 남은 슬롯 1"), AvailableCapacity.RemainingSlotCount, 1);
	TestEqual(TEXT("요청 후 남은 슬롯 0"), AvailableCapacity.ProjectedRemainingSlotCount, 0);
	TestTrue(TEXT("요청 전 남은 질량 40kg"), FMath::IsNearlyEqual(AvailableCapacity.RemainingMassKg, 40.0f));
	TestTrue(TEXT("요청 후 남은 질량 10kg"), FMath::IsNearlyEqual(AvailableCapacity.ProjectedRemainingMassKg, 10.0f));

	// [v1.0.0] 현재 1슬롯에서 2슬롯 추가를 요청한 슬롯 한도 초과 결과입니다.
	const FCFInventoryCapacityResult SlotExceededCapacity = VehicleCargo.EvaluateCapacity(60.0f, 2, 0.0f);
	TestFalse(TEXT("VehicleCargo 2슬롯 추가 거부"), SlotExceededCapacity.CanAccept());
	TestEqual(TEXT("Capacity 상태 SlotLimitExceeded"), SlotExceededCapacity.CapacityState, ECFInventoryCapacityState::SlotLimitExceeded);

	// [v1.0.0] 현재 60kg에서 50kg 추가를 요청한 질량 한도 초과 결과입니다.
	const FCFInventoryCapacityResult MassExceededCapacity = VehicleCargo.EvaluateCapacity(60.0f, 1, 50.0f);
	TestFalse(TEXT("VehicleCargo 50kg 추가 거부"), MassExceededCapacity.CanAccept());
	TestEqual(TEXT("Capacity 상태 MassLimitExceeded"), MassExceededCapacity.CapacityState, ECFInventoryCapacityState::MassLimitExceeded);

	// [v1.0.0] 음수 외부 해석 질량을 전달한 무효 Capacity 요청 결과입니다.
	const FCFInventoryCapacityResult InvalidCapacityRequest = VehicleCargo.EvaluateCapacity(-1.0f, 1, 0.0f);
	TestFalse(TEXT("음수 현재 질량 요청 거부"), InvalidCapacityRequest.CanAccept());
	TestEqual(TEXT("Capacity 상태 InvalidRequest"), InvalidCapacityRequest.CapacityState, ECFInventoryCapacityState::InvalidRequest);

	// [v1.0.0] 같은 ItemInstanceId를 한 Container에 중복 추가한 무효 VehicleCargo입니다.
	FCFInventoryContainerState DuplicateItemContainer = VehicleCargo;
	AddTestContainerEntry(DuplicateItemContainer, Fixture.CargoEquipmentItem, TEXT("CargoSlot_02"));
	TestFalse(TEXT("Container 내부 ItemInstanceId 중복 거부"), DuplicateItemContainer.ValidateContainerContract(ValidationErrors));
	TestTrue(TEXT("Container 내부 중복 Item 오류 존재"), ValidationErrors.Num() > 0);

	// [v1.0.0] 한 Container 내부 중복 ItemInstanceId의 위치 조회를 검증할 단일 Container 집합입니다.
	TArray<FCFInventoryContainerState> DuplicateItemLocationContainers;
	DuplicateItemLocationContainers.Add(DuplicateItemContainer);

	// [v1.0.0] 같은 Container 내부 두 슬롯에 존재하는 ItemInstanceId 위치 조회 결과입니다.
	const FCFInventoryLocationResult DuplicateItemLocationResult = FCFInventoryAccessQuery::QueryItemLocation(
		DuplicateItemLocationContainers,
		Fixture.CargoEquipmentItem.ItemHandle.ItemInstanceId);
	TestEqual(TEXT("Container 내부 중복 Item 위치 상태 Duplicate"), DuplicateItemLocationResult.LocationState, ECFInventoryLocationState::Duplicate);
	TestEqual(TEXT("Container 내부 중복 Item 발견 위치 수 2"), DuplicateItemLocationResult.MatchingLocationCount, 2);

	// [v1.0.0] 같은 MountProfileId 슬롯을 중복 사용한 무효 MountedEquipment입니다.
	FCFInventoryContainerState DuplicateSlotContainer = MountedEquipment;
	AddTestContainerEntry(DuplicateSlotContainer, Fixture.SpareEquipmentItem, TEXT("RoofDefense"));
	TestFalse(TEXT("MountedEquipment Slot ID 중복 거부"), DuplicateSlotContainer.ValidateContainerContract(ValidationErrors));
	TestTrue(TEXT("MountedEquipment 중복 Slot 오류 존재"), ValidationErrors.Num() > 0);

	// [v1.0.0] MaximumSlotCount보다 많은 Entry를 가진 무효 VehicleCargo입니다.
	FCFInventoryContainerState OverCapacityContainer = VehicleCargo;
	OverCapacityContainer.Capacity.MaximumSlotCount = 1;
	AddTestContainerEntry(OverCapacityContainer, Fixture.SpareEquipmentItem, TEXT("CargoSlot_02"));
	TestFalse(TEXT("Container 현재 Entry 슬롯 초과 거부"), OverCapacityContainer.ValidateContainerContract(ValidationErrors));
	TestTrue(TEXT("Container 현재 슬롯 초과 오류 존재"), ValidationErrors.Num() > 0);

	// [v1.0.0] 현재 상태 자체가 슬롯 한도를 초과한 Container의 Capacity Query 결과입니다.
	const FCFInventoryCapacityResult InvalidCurrentContainerCapacity = OverCapacityContainer.EvaluateCapacity(0.0f, 1, 0.0f);
	TestFalse(TEXT("현재 상태 무효 Container Capacity Query 거부"), InvalidCurrentContainerCapacity.CanAccept());
	TestEqual(TEXT("현재 상태 무효 Capacity 상태 InvalidContainer"), InvalidCurrentContainerCapacity.CapacityState, ECFInventoryCapacityState::InvalidContainer);

	return true;
}

// [v1.0.0] 현재 차량 접근 범위, 다른 차량 격리, 단일 Item 위치와 전체 중복 소유 거부를 검증합니다.
bool FCFInventoryAccessQueryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Access Query 테스트에서 공유할 Definition과 실제 Item Instance 묶음입니다.
	FCFInventoryContainerFixture Fixture;
	if (!BuildContainerFixture(*this, Fixture))
	{
		return false;
	}

	// [v1.0.0] 현재 차량 Container 두 개가 공유할 Owner ID입니다.
	const FCFInventoryOwnerId CurrentVehicleOwnerId = FCFInventoryOwnerId::CreateNew();

	// [v1.0.0] 접근 범위 밖 다른 차량 Container가 사용할 Owner ID입니다.
	const FCFInventoryOwnerId OtherVehicleOwnerId = FCFInventoryOwnerId::CreateNew();

	// [v1.0.0] 현재 차량의 VehicleCargo입니다.
	FCFInventoryContainerState VehicleCargo = CreateTestContainer(
		CurrentVehicleOwnerId,
		ECFInventoryContainerType::VehicleCargo,
		3);
	AddTestContainerEntry(VehicleCargo, Fixture.CargoEquipmentItem, TEXT("CargoSlot_01"));

	// [v1.0.0] 현재 차량의 MountedEquipment 소유 위치입니다.
	FCFInventoryContainerState MountedEquipment = CreateTestContainer(
		CurrentVehicleOwnerId,
		ECFInventoryContainerType::MountedEquipment,
		2);
	AddTestContainerEntry(MountedEquipment, Fixture.MountedDefenseItem, TEXT("RoofDefense"));

	// [v1.0.0] 현재 차량 Access Query에서 제외돼야 하는 다른 차량 VehicleCargo입니다.
	FCFInventoryContainerState OtherVehicleCargo = CreateTestContainer(
		OtherVehicleOwnerId,
		ECFInventoryContainerType::VehicleCargo,
		2);
	AddTestContainerEntry(OtherVehicleCargo, Fixture.SpareEquipmentItem, TEXT("CargoSlot_01"));

	// [v1.0.0] 현재 차량과 다른 차량 Container를 함께 보유한 전체 조회 집합입니다.
	TArray<FCFInventoryContainerState> Containers;
	Containers.Add(VehicleCargo);
	Containers.Add(MountedEquipment);
	Containers.Add(OtherVehicleCargo);

	// [v1.0.0] 전체 Container 집합 불변식 오류를 수집할 배열입니다.
	TArray<FText> ValidationErrors;
	TestTrue(TEXT("Container 집합 계약 유효"), FCFInventoryAccessQuery::ValidateContainerSet(Containers, ValidationErrors));
	TestEqual(TEXT("Container 집합 오류 없음"), ValidationErrors.Num(), 0);

	// [v1.0.0] 현재 차량 VehicleCargo와 MountedEquipment 접근을 모두 허용하는 Context입니다.
	FCFInventoryAccessContext AccessContext;
	AccessContext.CurrentVehicleOwnerId = CurrentVehicleOwnerId;
	AccessContext.bAllowVehicleCargo = true;
	AccessContext.bAllowMountedEquipment = true;
	TestTrue(TEXT("현재 차량 Access Context 유효"), AccessContext.IsValid());

	// [v1.0.0] 현재 차량 VehicleCargo 접근 조회 결과입니다.
	const FCFInventoryAccessResult CargoAccessResult = FCFInventoryAccessQuery::QueryVehicleContainer(
		Containers,
		AccessContext,
		ECFInventoryContainerType::VehicleCargo);
	TestTrue(TEXT("현재 차량 VehicleCargo 접근 가능"), CargoAccessResult.IsAccessible());
	TestEqual(TEXT("VehicleCargo 접근 상태"), CargoAccessResult.AccessState, ECFInventoryAccessState::Accessible);
	TestEqual(TEXT("VehicleCargo Owner 일치"), CargoAccessResult.ContainerRef.OwnerId, CurrentVehicleOwnerId);
	TestEqual(TEXT("VehicleCargo 사용 슬롯 1"), CargoAccessResult.OccupiedSlotCount, 1);
	TestEqual(TEXT("VehicleCargo 남은 슬롯 2"), CargoAccessResult.RemainingSlotCount, 2);

	// [v1.0.0] 현재 차량 MountedEquipment 접근 조회 결과입니다.
	const FCFInventoryAccessResult MountedAccessResult = FCFInventoryAccessQuery::QueryVehicleContainer(
		Containers,
		AccessContext,
		ECFInventoryContainerType::MountedEquipment);
	TestTrue(TEXT("현재 차량 MountedEquipment 접근 가능"), MountedAccessResult.IsAccessible());
	TestEqual(TEXT("MountedEquipment 접근 상태"), MountedAccessResult.AccessState, ECFInventoryAccessState::Accessible);
	TestEqual(TEXT("MountedEquipment Owner 일치"), MountedAccessResult.ContainerRef.OwnerId, CurrentVehicleOwnerId);

	AccessContext.bAllowMountedEquipment = false;

	// [v1.0.0] 현재 Context에서 MountedEquipment 접근을 비활성화한 조회 결과입니다.
	const FCFInventoryAccessResult DisabledMountedAccessResult = FCFInventoryAccessQuery::QueryVehicleContainer(
		Containers,
		AccessContext,
		ECFInventoryContainerType::MountedEquipment);
	TestFalse(TEXT("비활성 MountedEquipment 접근 거부"), DisabledMountedAccessResult.IsAccessible());
	TestEqual(TEXT("비활성 MountedEquipment 상태 Unavailable"), DisabledMountedAccessResult.AccessState, ECFInventoryAccessState::Unavailable);

	// [v1.0.0] Container가 없는 차량 Owner ID를 사용하는 Access Context입니다.
	FCFInventoryAccessContext MissingOwnerContext;
	MissingOwnerContext.CurrentVehicleOwnerId = FCFInventoryOwnerId::CreateNew();

	// [v1.0.0] 현재 집합에 없는 차량의 VehicleCargo 조회 결과입니다.
	const FCFInventoryAccessResult MissingCargoResult = FCFInventoryAccessQuery::QueryVehicleContainer(
		Containers,
		MissingOwnerContext,
		ECFInventoryContainerType::VehicleCargo);
	TestEqual(TEXT("없는 차량 VehicleCargo 상태 Missing"), MissingCargoResult.AccessState, ECFInventoryAccessState::Missing);

	// [v1.0.0] Cargo Equipment Item의 전체 Container 단일 위치 조회 결과입니다.
	const FCFInventoryLocationResult CargoLocationResult = FCFInventoryAccessQuery::QueryItemLocation(
		Containers,
		Fixture.CargoEquipmentItem.ItemHandle.ItemInstanceId);
	TestTrue(TEXT("Cargo Equipment 단일 위치 발견"), CargoLocationResult.IsFound());
	TestEqual(TEXT("Cargo Equipment 위치 상태 Found"), CargoLocationResult.LocationState, ECFInventoryLocationState::Found);
	TestEqual(TEXT("Cargo Equipment 위치 Owner"), CargoLocationResult.ItemLocation.ContainerRef.OwnerId, CurrentVehicleOwnerId);
	TestEqual(TEXT("Cargo Equipment 위치 종류"), CargoLocationResult.ItemLocation.ContainerRef.ContainerType, ECFInventoryContainerType::VehicleCargo);
	TestEqual(TEXT("Cargo Equipment 위치 Slot"), CargoLocationResult.ItemLocation.ContainerSlotId, FName(TEXT("CargoSlot_01")));

	// [v1.0.0] Container 집합에 넣지 않은 새 Item Instance 위치 조회 결과입니다.
	const FCFInventoryItemInstance MissingItemInstance = FCFInventoryItemInstance::CreateUniqueItem(Fixture.EquipmentItemDefinition);
	const FCFInventoryLocationResult MissingLocationResult = FCFInventoryAccessQuery::QueryItemLocation(
		Containers,
		MissingItemInstance.ItemHandle.ItemInstanceId);
	TestFalse(TEXT("미소유 Item 위치 발견 안 됨"), MissingLocationResult.IsFound());
	TestEqual(TEXT("미소유 Item 위치 상태 Missing"), MissingLocationResult.LocationState, ECFInventoryLocationState::Missing);

	// [v1.0.0] 같은 ItemInstanceId를 다른 Container에도 배치해 전체 중복 소유를 만든 집합입니다.
	TArray<FCFInventoryContainerState> DuplicateOwnershipContainers = Containers;
	AddTestContainerEntry(DuplicateOwnershipContainers[2], Fixture.CargoEquipmentItem, TEXT("CargoSlot_02"));
	TestFalse(TEXT("전체 Container ItemInstanceId 중복 소유 거부"), FCFInventoryAccessQuery::ValidateContainerSet(DuplicateOwnershipContainers, ValidationErrors));
	TestTrue(TEXT("전체 중복 소유 오류 존재"), ValidationErrors.Num() > 0);

	// [v1.0.0] 전체 중복 소유 상태에서 Item 위치를 조회한 결과입니다.
	const FCFInventoryLocationResult DuplicateLocationResult = FCFInventoryAccessQuery::QueryItemLocation(
		DuplicateOwnershipContainers,
		Fixture.CargoEquipmentItem.ItemHandle.ItemInstanceId);
	TestFalse(TEXT("중복 소유 Item은 단일 위치로 판정하지 않음"), DuplicateLocationResult.IsFound());
	TestEqual(TEXT("중복 소유 Item 위치 상태 Duplicate"), DuplicateLocationResult.LocationState, ECFInventoryLocationState::Duplicate);
	TestEqual(TEXT("중복 소유 발견 위치 수 2"), DuplicateLocationResult.MatchingLocationCount, 2);

	// [v1.0.0] 같은 Owner의 두 번째 VehicleCargo를 추가한 무효 Container 집합입니다.
	TArray<FCFInventoryContainerState> DuplicateOwnerTypeContainers = Containers;
	DuplicateOwnerTypeContainers.Add(CreateTestContainer(
		CurrentVehicleOwnerId,
		ECFInventoryContainerType::VehicleCargo,
		1));
	TestFalse(TEXT("한 차량의 VehicleCargo 두 개 거부"), FCFInventoryAccessQuery::ValidateContainerSet(DuplicateOwnerTypeContainers, ValidationErrors));
	TestTrue(TEXT("Owner별 ContainerType 중복 오류 존재"), ValidationErrors.Num() > 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
