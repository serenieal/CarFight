// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-01 Item Identity와 Definition Pawn 없는 자동화 테스트
// Scope: ItemInstanceId 안정성·직렬화, Definition·Instance 분리, Quantity 1, Equipment·Defense 강타입 참조를 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Inventory.INV_P0_01.Identity와 Definition 테스트를 최초 추가.
// Migration:
// - Container, Reservation, Transfer와 Fitting Adapter 검증은 INV-P0-02~04 테스트로 분리한다.
// - Pawn, World, Blueprint와 Unreal Asset을 생성하거나 수정하지 않는다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFInventoryItemData.h"
#include "CFInventoryTypes.h"
#include "CFEquipmentPresetData.h"
#include "CFVehicleDefenseData.h"

#include "Misc/AutomationTest.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryIdentityTest,
	"CarFight.Inventory.INV_P0_01.Identity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFInventoryDefinitionTest,
	"CarFight.Inventory.INV_P0_01.Definition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] ItemInstanceId 생성, 복사, 서로 다른 인스턴스와 UStruct 직렬화 안정성을 검증합니다.
bool FCFInventoryIdentityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 생성 전 무효 상태를 확인할 기본 ItemInstanceId입니다.
	const FCFItemInstanceId DefaultItemInstanceId;
	TestFalse(TEXT("기본 ItemInstanceId는 무효"), DefaultItemInstanceId.IsValid());

	// [v1.0.0] 안정성과 직렬화를 확인할 첫 번째 고유 ItemInstanceId입니다.
	FCFItemInstanceId FirstItemInstanceId = FCFItemInstanceId::CreateNew();
	TestTrue(TEXT("새 ItemInstanceId는 유효"), FirstItemInstanceId.IsValid());
	TestFalse(TEXT("새 ItemInstanceId 문자열은 비어 있지 않음"), FirstItemInstanceId.ToString().IsEmpty());

	// [v1.0.0] 복사 후 같은 실제 아이템을 계속 식별하는지 확인할 ID입니다.
	const FCFItemInstanceId CopiedItemInstanceId = FirstItemInstanceId;
	TestTrue(TEXT("복사한 ItemInstanceId는 동일"), CopiedItemInstanceId == FirstItemInstanceId);
	TestEqual(TEXT("복사한 ItemInstanceId 문자열 보존"), CopiedItemInstanceId.ToString(), FirstItemInstanceId.ToString());

	// [v1.0.0] 서로 다른 실제 아이템이 다른 ID를 받는지 확인할 두 번째 ID입니다.
	const FCFItemInstanceId SecondItemInstanceId = FCFItemInstanceId::CreateNew();
	TestTrue(TEXT("두 번째 ItemInstanceId는 유효"), SecondItemInstanceId.IsValid());
	TestTrue(TEXT("서로 다른 인스턴스는 다른 ID"), SecondItemInstanceId != FirstItemInstanceId);

	// [v1.0.0] ItemInstanceId UPROPERTY 값을 기록할 메모리 직렬화 버퍼입니다.
	FBufferArchive SerializedItemInstanceId;
	FCFItemInstanceId::StaticStruct()->SerializeItem(SerializedItemInstanceId, &FirstItemInstanceId, nullptr);

	// [v1.0.0] 메모리 직렬화에서 복원할 ItemInstanceId입니다.
	FCFItemInstanceId RestoredItemInstanceId;

	// [v1.0.0] 기록된 ItemInstanceId 바이트를 읽을 메모리 Reader입니다.
	FMemoryReader ItemInstanceIdReader(SerializedItemInstanceId);
	FCFItemInstanceId::StaticStruct()->SerializeItem(ItemInstanceIdReader, &RestoredItemInstanceId, nullptr);

	TestTrue(TEXT("직렬화 복원 ItemInstanceId 유효"), RestoredItemInstanceId.IsValid());
	TestTrue(TEXT("직렬화 후 ItemInstanceId 동일"), RestoredItemInstanceId == FirstItemInstanceId);
	TestEqual(TEXT("직렬화 후 Guid 값 보존"), RestoredItemInstanceId.GetGuid(), FirstItemInstanceId.GetGuid());

	return true;
}

// [v1.0.0] Equipment·Defense Definition 강타입 참조, PrimaryAssetId 분리와 Quantity 1 인스턴스를 검증합니다.
bool FCFInventoryDefinitionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Equipment Definition이 강타입으로 참조할 Transient EquipmentPresetData입니다.
	UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>();
	if (!TestNotNull(TEXT("EquipmentPresetData 생성"), EquipmentPresetData))
	{
		return false;
	}

	// [v1.0.0] 안정적인 Definition ID와 EquipmentPresetData를 연결할 Transient Equipment Item Definition입니다.
	UCFEquipmentItemData* EquipmentItemDefinition = NewObject<UCFEquipmentItemData>();
	if (!TestNotNull(TEXT("Equipment Item Definition 생성"), EquipmentItemDefinition))
	{
		return false;
	}
	EquipmentItemDefinition->ItemDefinitionId = TEXT("Test_SharedDefinition");
	EquipmentItemDefinition->EquipmentPresetData = EquipmentPresetData;

	// [v1.0.0] Equipment Definition 검증 오류를 수집할 배열입니다.
	TArray<FText> ValidationErrors;
	TestTrue(TEXT("Equipment Definition 계약 유효"), EquipmentItemDefinition->ValidateItemDefinitionContract(ValidationErrors));
	TestEqual(TEXT("Equipment Definition 오류 없음"), ValidationErrors.Num(), 0);
	TestEqual(TEXT("Equipment Definition 도메인"), EquipmentItemDefinition->GetItemDomain(), ECFInventoryItemDomain::Equipment);
	TestEqual(TEXT("Equipment Quantity 계약"), EquipmentItemDefinition->GetRequiredQuantity(), 1);
	TestTrue(TEXT("Equipment 고유 인스턴스 계약"), EquipmentItemDefinition->IsUniqueInstanceDefinition());

	// [v1.0.0] Equipment Definition의 명시 ID와 타입으로 기대되는 안정적인 PrimaryAssetId입니다.
	const FPrimaryAssetId ExpectedEquipmentDefinitionId(
		FPrimaryAssetType(TEXT("CFEquipmentItem")),
		TEXT("Test_SharedDefinition"));
	TestEqual(TEXT("Equipment Definition PrimaryAssetId"), EquipmentItemDefinition->GetPrimaryAssetId(), ExpectedEquipmentDefinitionId);

	// [v1.0.0] Defense Definition이 강타입으로 참조할 Transient VehicleDefenseData입니다.
	UCFVehicleDefenseData* VehicleDefenseData = NewObject<UCFVehicleDefenseData>();
	if (!TestNotNull(TEXT("VehicleDefenseData 생성"), VehicleDefenseData))
	{
		return false;
	}

	// [v1.0.0] 안정적인 Definition ID와 VehicleDefenseData를 연결할 Transient Defense Item Definition입니다.
	UCFDefenseItemData* DefenseItemDefinition = NewObject<UCFDefenseItemData>();
	if (!TestNotNull(TEXT("Defense Item Definition 생성"), DefenseItemDefinition))
	{
		return false;
	}
	DefenseItemDefinition->ItemDefinitionId = TEXT("Test_SharedDefinition");
	DefenseItemDefinition->VehicleDefenseData = VehicleDefenseData;

	TestTrue(TEXT("Defense Definition 계약 유효"), DefenseItemDefinition->ValidateItemDefinitionContract(ValidationErrors));
	TestEqual(TEXT("Defense Definition 오류 없음"), ValidationErrors.Num(), 0);
	TestEqual(TEXT("Defense Definition 도메인"), DefenseItemDefinition->GetItemDomain(), ECFInventoryItemDomain::Defense);
	TestEqual(TEXT("Defense Quantity 계약"), DefenseItemDefinition->GetRequiredQuantity(), 1);
	TestTrue(TEXT("Defense 고유 인스턴스 계약"), DefenseItemDefinition->IsUniqueInstanceDefinition());

	// [v1.0.0] Defense Definition의 명시 ID와 타입으로 기대되는 안정적인 PrimaryAssetId입니다.
	const FPrimaryAssetId ExpectedDefenseDefinitionId(
		FPrimaryAssetType(TEXT("CFDefenseItem")),
		TEXT("Test_SharedDefinition"));
	TestEqual(TEXT("Defense Definition PrimaryAssetId"), DefenseItemDefinition->GetPrimaryAssetId(), ExpectedDefenseDefinitionId);
	TestTrue(TEXT("도메인별 같은 이름도 PrimaryAssetType으로 분리"), ExpectedDefenseDefinitionId != ExpectedEquipmentDefinitionId);

	// [v1.0.0] Equipment Definition에서 생성한 Quantity 1 실제 소유 인스턴스입니다.
	FCFInventoryItemInstance EquipmentItemInstance = FCFInventoryItemInstance::CreateUniqueItem(EquipmentItemDefinition);
	TestTrue(TEXT("Equipment ItemInstance 유효"), EquipmentItemInstance.IsValid());
	TestTrue(TEXT("Equipment Definition과 Instance 연결 유효"), EquipmentItemInstance.IsValidForDefinition(EquipmentItemDefinition));
	TestEqual(TEXT("Equipment ItemInstance Quantity 1"), EquipmentItemInstance.Quantity, 1);
	TestEqual(TEXT("Equipment ItemInstance Definition ID 보존"), EquipmentItemInstance.ItemHandle.ItemDefinitionId, ExpectedEquipmentDefinitionId);

	// [v1.0.0] 같은 Equipment Definition에서 생성한 별도의 실제 소유 인스턴스입니다.
	const FCFInventoryItemInstance SecondEquipmentItemInstance = FCFInventoryItemInstance::CreateUniqueItem(EquipmentItemDefinition);
	TestTrue(TEXT("같은 Definition의 두 인스턴스 ID는 다름"), EquipmentItemInstance.ItemHandle.ItemInstanceId != SecondEquipmentItemInstance.ItemHandle.ItemInstanceId);

	// [v1.0.0] Defense Definition에서 생성한 Quantity 1 실제 소유 인스턴스입니다.
	const FCFInventoryItemInstance DefenseItemInstance = FCFInventoryItemInstance::CreateUniqueItem(DefenseItemDefinition);
	TestTrue(TEXT("Defense ItemInstance 유효"), DefenseItemInstance.IsValid());
	TestTrue(TEXT("Defense Definition과 Instance 연결 유효"), DefenseItemInstance.IsValidForDefinition(DefenseItemDefinition));
	TestEqual(TEXT("Defense ItemInstance Quantity 1"), DefenseItemInstance.Quantity, 1);
	TestFalse(TEXT("Equipment Instance를 Defense Definition으로 해석 거부"), EquipmentItemInstance.IsValidForDefinition(DefenseItemDefinition));

	EquipmentItemInstance.Quantity = 2;
	TestFalse(TEXT("Equipment Quantity 2는 고유 인스턴스 계약 위반"), EquipmentItemInstance.IsValid());
	TestFalse(TEXT("Equipment Quantity 2는 Definition 검증 거부"), EquipmentItemInstance.IsValidForDefinition(EquipmentItemDefinition));

	// [v1.0.0] 잘못된 Equipment Domain 참조를 거부하는지 확인할 Definition입니다.
	UCFEquipmentItemData* MissingEquipmentDefinition = NewObject<UCFEquipmentItemData>();
	if (!TestNotNull(TEXT("참조 누락 Equipment Definition 생성"), MissingEquipmentDefinition))
	{
		return false;
	}
	MissingEquipmentDefinition->ItemDefinitionId = TEXT("Test_MissingEquipment");
	TestFalse(TEXT("EquipmentPresetData 누락 Definition 거부"), MissingEquipmentDefinition->ValidateItemDefinitionContract(ValidationErrors));
	TestTrue(TEXT("EquipmentPresetData 누락 오류 존재"), ValidationErrors.Num() > 0);

	// [v1.0.0] 잘못된 Equipment Definition에서 실제 인스턴스 생성이 거부되는지 확인할 결과입니다.
	const FCFInventoryItemInstance InvalidEquipmentItemInstance = FCFInventoryItemInstance::CreateUniqueItem(MissingEquipmentDefinition);
	TestFalse(TEXT("EquipmentPresetData 누락 Definition의 Instance 생성 거부"), InvalidEquipmentItemInstance.IsValid());
	TestEqual(TEXT("잘못된 Equipment Definition Instance Quantity 0"), InvalidEquipmentItemInstance.Quantity, 0);

	// [v1.0.0] 잘못된 Defense Domain 참조를 거부하는지 확인할 Definition입니다.
	UCFDefenseItemData* MissingDefenseDefinition = NewObject<UCFDefenseItemData>();
	if (!TestNotNull(TEXT("참조 누락 Defense Definition 생성"), MissingDefenseDefinition))
	{
		return false;
	}
	MissingDefenseDefinition->ItemDefinitionId = TEXT("Test_MissingDefense");
	TestFalse(TEXT("VehicleDefenseData 누락 Definition 거부"), MissingDefenseDefinition->ValidateItemDefinitionContract(ValidationErrors));
	TestTrue(TEXT("VehicleDefenseData 누락 오류 존재"), ValidationErrors.Num() > 0);

	// [v1.0.0] 잘못된 Defense Definition에서 실제 인스턴스 생성이 거부되는지 확인할 결과입니다.
	const FCFInventoryItemInstance InvalidDefenseItemInstance = FCFInventoryItemInstance::CreateUniqueItem(MissingDefenseDefinition);
	TestFalse(TEXT("VehicleDefenseData 누락 Definition의 Instance 생성 거부"), InvalidDefenseItemInstance.IsValid());
	TestEqual(TEXT("잘못된 Defense Definition Instance Quantity 0"), InvalidDefenseItemInstance.Quantity, 0);

	// [v1.0.0] Definition이 없을 때 생성이 실패하는 안전 기본 인스턴스입니다.
	const FCFInventoryItemInstance MissingDefinitionInstance = FCFInventoryItemInstance::CreateUniqueItem(nullptr);
	TestFalse(TEXT("Definition 없는 ItemInstance 생성 거부"), MissingDefinitionInstance.IsValid());
	TestEqual(TEXT("Definition 없는 ItemInstance Quantity 0"), MissingDefinitionInstance.Quantity, 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
