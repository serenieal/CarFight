// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-036 SEN-P0-01 Sensor Contact Data Contract Automation
// Scope: Config, Actor-free Contact/Snapshot, ContactId allocator와 Pawn 기본 소유권을 asset-free로 검증합니다.
// Changelog:
// - v1.0.0: SEN-P0-01 DataContract, RuntimeContract, PawnOwnership 3개 Automation을 최초 추가.
// Migration:
// - Transient UObject/CDO와 C++ Struct만 사용하며 Content Asset을 생성하거나 저장하지 않습니다.
// - TargetSelect 후보·선택, HUD, Collision Config와 World 탐지 로직을 실행하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFSensorTypes.h"
#include "CFTargetSelectComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleSensorComp.h"
#include "CFVehicleSensorData.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

namespace
{
	// [v1.0.0] 공개 Struct가 직접 또는 중첩 Struct/Array를 통해 UObject 참조 Property를 포함하는지 재귀 검사합니다.
	bool ContainsObjectReferenceProperty(const UStruct* StructType)
	{
		if (!StructType)
		{
			return false;
		}

		for (TFieldIterator<FProperty> PropertyIterator(StructType); PropertyIterator; ++PropertyIterator)
		{
			// [v1.0.0] 현재 재귀 검사 중인 Reflection Property입니다.
			const FProperty* Property = *PropertyIterator;
			if (CastField<const FObjectPropertyBase>(Property))
			{
				return true;
			}

			if (const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(Property))
			{
				if (CastField<const FObjectPropertyBase>(ArrayProperty->Inner))
				{
					return true;
				}

				if (const FStructProperty* InnerStructProperty = CastField<const FStructProperty>(ArrayProperty->Inner))
				{
					if (ContainsObjectReferenceProperty(InnerStructProperty->Struct))
					{
						return true;
					}
				}
			}

			if (const FStructProperty* StructProperty = CastField<const FStructProperty>(Property))
			{
				if (ContainsObjectReferenceProperty(StructProperty->Struct))
				{
					return true;
				}
			}
		}

		return false;
	}

	// [v1.0.0] 테스트용 유효 Identified Contact를 생성합니다.
	FCFSensorContact MakeValidContact(const FName ContactId, const FName TargetId)
	{
		// [v1.0.0] 기본 공개 계약을 만족하도록 채울 테스트 Contact입니다.
		FCFSensorContact Contact;
		Contact.ContactId = ContactId;
		Contact.KnownTargetId = TargetId;
		Contact.KnownDisplayName = FText::FromString(TargetId.ToString());
		Contact.TargetCategory = ECFTargetCategory::Vehicle;
		Contact.Relation = ECFTargetRelation::Hostile;
		Contact.InformationLevel = ECFTargetInfoLevel::Identified;
		Contact.ContactState = ECFSensorContactState::Live;
		Contact.LastKnownWorldLocation = FVector(100.0, 200.0, 50.0);
		Contact.LastObservedWorldTimeSeconds = 1.0;
		Contact.FreshnessSeconds = 0.0f;
		Contact.AnalysisProgress01 = 0.5f;
		Contact.bDestroyedConfirmed = false;
		return Contact;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorDataContractTest,
	"CarFight.Sensor.SEN_P0_01.DataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Config 검증, ContactId/TargetId 분리, Actor-free Snapshot과 결정 정렬 계약을 검증합니다.
bool FCFSensorDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Content Asset 저장 없이 DataValidation 계약을 확인할 Transient SensorData입니다.
	UCFVehicleSensorData* SensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient VehicleSensorData"), SensorData))
	{
		return false;
	}

	TestTrue(TEXT("Foundation 기본 SensorConfig 유효"), SensorData->IsSensorConfigValid());
	SensorData->SensorConfig.UpdateIntervalSec = 0.0f;
	TestFalse(TEXT("0초 UpdateInterval은 무효"), SensorData->IsSensorConfigValid());
	SensorData->SensorConfig.UpdateIntervalSec = 0.1f;
	SensorData->SensorConfig.IdentifiedThreshold = 0.8f;
	SensorData->SensorConfig.DetailedScanThreshold = 0.7f;
	TestFalse(TEXT("DetailedScanThreshold가 Identified 이하이면 무효"), SensorData->IsSensorConfigValid());

	// [v1.0.0] ContactId와 TargetId 분리 계약을 검증할 첫 Contact입니다.
	FCFSensorContact ContactB = MakeValidContact(TEXT("Contact_000002"), TEXT("Vehicle_B"));
	// [v1.0.0] 결정 정렬에서 ContactB보다 앞에 와야 할 둘째 Contact입니다.
	FCFSensorContact ContactA = MakeValidContact(TEXT("Contact_000001"), TEXT("Vehicle_A"));
	TestTrue(TEXT("유효 Contact 계약"), ContactA.IsPublicContractValid());
	TestNotEqual(TEXT("ContactId는 KnownTargetId와 별도 식별자"), ContactA.ContactId, ContactA.KnownTargetId);

	TestFalse(TEXT("CF Sensor Contact는 UObject/Actor 참조 Property 없음"), ContainsObjectReferenceProperty(FCFSensorContact::StaticStruct()));
	TestFalse(TEXT("CF Sensor Snapshot은 중첩 Contact 포함 UObject/Actor 참조 Property 없음"), ContainsObjectReferenceProperty(FCFSensorSnapshot::StaticStruct()));
	TestNull(TEXT("Sensor Contact에는 TargetSelect TrackState Property 없음"), FCFSensorContact::StaticStruct()->FindPropertyByName(TEXT("TrackState")));

	// [v1.0.0] 입력 순서와 무관한 ContactId 결정 정렬을 검증할 Snapshot입니다.
	FCFSensorSnapshot Snapshot;
	Snapshot.Revision = 1;
	Snapshot.SnapshotWorldTimeSeconds = 1.0;
	Snapshot.Contacts = {ContactB, ContactA};
	Snapshot.SortContactsDeterministically();
	TestEqual(TEXT("첫 Contact 결정 정렬"), Snapshot.Contacts[0].ContactId, FName(TEXT("Contact_000001")));
	TestEqual(TEXT("둘째 Contact 결정 정렬"), Snapshot.Contacts[1].ContactId, FName(TEXT("Contact_000002")));
	TestTrue(TEXT("중복 없는 Snapshot 공개 계약"), Snapshot.IsPublicContractValid());

	Snapshot.Contacts.Add(ContactA);
	TestFalse(TEXT("중복 ContactId 검출"), Snapshot.HasUniqueContactIds());
	TestFalse(TEXT("중복 ContactId Snapshot은 공개 계약 무효"), Snapshot.IsPublicContractValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorRuntimeContractTest,
	"CarFight.Sensor.SEN_P0_01.RuntimeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] SensorComp의 Fallback 초기화, 빈 Snapshot과 같은 수명 ContactId 비재사용 계약을 검증합니다.
bool FCFSensorRuntimeContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] World/Content Asset 없이 Runtime Foundation을 검증할 Transient SensorComp입니다.
	UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient VehicleSensorComp"), SensorComponent))
	{
		return false;
	}

	TestTrue(TEXT("Fallback SensorConfig로 Runtime 초기화"), SensorComponent->InitializeSensorRuntime());
	TestTrue(TEXT("Sensor Runtime Ready"), SensorComponent->IsSensorRuntimeReady());

	// [v1.0.0] 초기화 직후 외부 소비자가 읽을 Foundation Snapshot입니다.
	const FCFSensorSnapshot InitialSnapshot = SensorComponent->GetSensorSnapshot();
	TestTrue(TEXT("초기 Snapshot Runtime Ready"), InitialSnapshot.bRuntimeReady);
	TestEqual(TEXT("SEN-P0-01 초기 Contact 0"), InitialSnapshot.Contacts.Num(), 0);
	TestTrue(TEXT("빈 Foundation Snapshot 공개 계약 유효"), InitialSnapshot.IsPublicContractValid());

	// [v1.0.0] TargetId와 무관하게 Component가 독립 발급하는 첫 ContactId입니다.
	const FName FirstContactId = SensorComponent->AllocateContactId();
	// [v1.0.0] 같은 Runtime 수명에서 중복 없이 발급되는 둘째 ContactId입니다.
	const FName SecondContactId = SensorComponent->AllocateContactId();
	TestEqual(TEXT("첫 ContactId"), FirstContactId, FName(TEXT("Contact_000001")));
	TestEqual(TEXT("둘째 ContactId"), SecondContactId, FName(TEXT("Contact_000002")));

	SensorComponent->ResetSensorRuntime();
	TestFalse(TEXT("Reset 후 Sensor Runtime Not Ready"), SensorComponent->IsSensorRuntimeReady());
	TestFalse(TEXT("Reset Snapshot Runtime Not Ready"), SensorComponent->GetSensorSnapshot().bRuntimeReady);
	TestTrue(TEXT("Reset 후 재초기화"), SensorComponent->InitializeSensorRuntime());

	// [v1.0.0] Reset/Reinitialize 후에도 같은 Component 수명에서 재사용되지 않아야 할 셋째 ContactId입니다.
	const FName ThirdContactId = SensorComponent->AllocateContactId();
	TestEqual(TEXT("재초기화 후 ContactId serial 유지"), ThirdContactId, FName(TEXT("Contact_000003")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorPawnOwnershipTest,
	"CarFight.Sensor.SEN_P0_01.PawnOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] ACFVehiclePawn CDO가 SensorComp를 기본 서브오브젝트로 소유하고 TargetSelect와 별도 인스턴스로 유지하는지 검증합니다.
bool FCFSensorPawnOwnershipTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Content Asset을 변경하지 않고 기본 서브오브젝트 구성을 확인할 VehiclePawn CDO입니다.
	const ACFVehiclePawn* VehiclePawnCDO = GetDefault<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("CFVehiclePawn CDO"), VehiclePawnCDO))
	{
		return false;
	}

	TestNotNull(TEXT("VehicleSensorComp 기본 서브오브젝트"), VehiclePawnCDO->GetVehicleSensorComp());
	TestNotNull(TEXT("기존 TargetSelectComp 보존"), VehiclePawnCDO->GetTargetSelectComp());
	TestTrue(
		TEXT("SensorComp와 TargetSelectComp는 독립 UObject"),
		static_cast<const UObject*>(VehiclePawnCDO->GetVehicleSensorComp()) != static_cast<const UObject*>(VehiclePawnCDO->GetTargetSelectComp()));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
