// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-02 Vehicle Ammo Runtime Automation
// Scope: 동일 탄종 공유 예비량, WeaponInstance별 독립 장전량, Snapshot 계산과 반복 초기화 안전성을 검증합니다.
// Changelog:
// - v1.0.0: 최초 Vehicle Ammo Runtime Foundation 테스트 추가.
// Migration:
// - Automation World와 Transient DataAsset만 사용하며 저장 Asset을 수정하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoData.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehiclePawn.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAmmoRuntimeTest,
	"CarFight.Ammo.AMMO_P0_02.Runtime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 공유 Reserve와 독립 Loaded를 가진 두 WeaponInstance의 Snapshot·반복 초기화 계약을 검증합니다.
bool FCFVehicleAmmoRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기본 서브오브젝트 VehicleAmmoComp를 실제 Pawn 수명에서 확인할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Ammo Runtime Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] VehicleAmmoComp 기본 서브오브젝트를 소유할 Transient 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("Ammo Runtime Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] 동일 탄종을 공유하는 두 무기의 실제 차량 예비 풀을 정의할 Transient AmmoData입니다.
	UCFAmmoData* SharedAmmoData = NewObject<UCFAmmoData>(GetTransientPackage());
	SharedAmmoData->AmmoId = TEXT("Test_SharedShell");

	// [v1.0.0] 첫 번째 독립 탄창 설정을 제공하는 Transient WeaponData입니다.
	UCFWeaponData* FirstWeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	FirstWeaponData->DefaultAmmoData = SharedAmmoData;
	FirstWeaponData->bUseInfiniteAmmoForDebug = false;
	FirstWeaponData->MagazineSize = 20;
	FirstWeaponData->InitialLoadedAmmoCount = 20;
	FirstWeaponData->AmmoUnitsPerShot = 1;

	// [v1.0.0] 두 번째 독립 탄창 설정을 제공하는 Transient WeaponData입니다.
	UCFWeaponData* SecondWeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	SecondWeaponData->DefaultAmmoData = SharedAmmoData;
	SecondWeaponData->bUseInfiniteAmmoForDebug = false;
	SecondWeaponData->MagazineSize = 15;
	SecondWeaponData->InitialLoadedAmmoCount = 10;
	SecondWeaponData->AmmoUnitsPerShot = 1;

	// [v1.0.0] Pawn이 C++ 기본 서브오브젝트로 소유해야 하는 실제 VehicleAmmoComp입니다.
	UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();
	if (!TestNotNull(TEXT("VehicleAmmoComp 기본 서브오브젝트"), VehicleAmmoComp))
	{
		return false;
	}

	// [v1.0.0] 같은 탄종을 차량 전체 100발 싣고 출격한 실제 현재 수량 입력입니다.
	FCFAmmoSortieLoad SortieAmmoLoad;
	SortieAmmoLoad.AmmoData = SharedAmmoData;
	SortieAmmoLoad.InitialSortieAmmoCount = 100;

	// [v1.0.0] 첫 번째 무기 인스턴스의 독립 장전 상태 입력입니다.
	FCFWeaponAmmoInitialization FirstWeaponInitialization;
	FirstWeaponInitialization.WeaponInstanceId = TEXT("Mount_A");
	FirstWeaponInitialization.WeaponData = FirstWeaponData;

	// [v1.0.0] 두 번째 무기 인스턴스의 독립 장전 상태 입력입니다.
	FCFWeaponAmmoInitialization SecondWeaponInitialization;
	SecondWeaponInitialization.WeaponInstanceId = TEXT("Mount_B");
	SecondWeaponInitialization.WeaponData = SecondWeaponData;

	TArray<FCFAmmoSortieLoad> SortieAmmoLoads;
	SortieAmmoLoads.Add(SortieAmmoLoad);
	TArray<FCFWeaponAmmoInitialization> WeaponInitializations;
	WeaponInitializations.Add(FirstWeaponInitialization);
	WeaponInitializations.Add(SecondWeaponInitialization);

	TestTrue(TEXT("두 무기 공유 탄종 Runtime 초기화"), VehicleAmmoComp->InitializeAmmoRuntime(VehiclePawn, SortieAmmoLoads, WeaponInitializations));
	TestEqual(TEXT("WeaponInstance Runtime 2개"), VehicleAmmoComp->GetWeaponAmmoRuntimeCount(), 2);
	TestEqual(TEXT("공유 예비량 70"), VehicleAmmoComp->GetReserveAmmoCount(SharedAmmoData->AmmoId), 70);

	// [v1.0.0] 첫 번째 무기의 계산 완료 Snapshot입니다.
	FCFAmmoRuntimeSnapshot FirstSnapshot;
	TestTrue(TEXT("첫 번째 무기 Snapshot 조회"), VehicleAmmoComp->TryGetAmmoSnapshot(TEXT("Mount_A"), FirstSnapshot));
	TestEqual(TEXT("첫 번째 장전량 20"), FirstSnapshot.LoadedAmmoCount, 20);
	TestEqual(TEXT("첫 번째 즉시 사용 가능 20"), FirstSnapshot.ImmediateUsableAmmoCount, 20);
	TestEqual(TEXT("첫 번째 현재 사용 가능 90"), FirstSnapshot.CurrentUsableAmmoCount, 90);
	TestEqual(TEXT("같은 탄종 차량 전체 보유량 100"), FirstSnapshot.CurrentOnboardAmmoCount, 100);

	// [v1.0.0] 두 번째 무기의 계산 완료 Snapshot입니다.
	FCFAmmoRuntimeSnapshot SecondSnapshot;
	TestTrue(TEXT("두 번째 무기 Snapshot 조회"), VehicleAmmoComp->TryGetAmmoSnapshot(TEXT("Mount_B"), SecondSnapshot));
	TestEqual(TEXT("두 번째 장전량 10"), SecondSnapshot.LoadedAmmoCount, 10);
	TestEqual(TEXT("두 번째 현재 사용 가능 80"), SecondSnapshot.CurrentUsableAmmoCount, 80);
	TestEqual(TEXT("두 번째에서도 같은 차량 전체 보유량 100"), SecondSnapshot.CurrentOnboardAmmoCount, 100);

	TestTrue(TEXT("같은 입력 반복 초기화 성공"), VehicleAmmoComp->InitializeAmmoRuntime(VehiclePawn, SortieAmmoLoads, WeaponInitializations));
	TestEqual(TEXT("반복 초기화 후 예비량 중복 생성 없음"), VehicleAmmoComp->GetReserveAmmoCount(SharedAmmoData->AmmoId), 70);
	TestEqual(TEXT("반복 초기화 후 Runtime 수 유지"), VehicleAmmoComp->GetWeaponAmmoRuntimeCount(), 2);

	// [v1.0.0] 출격 총량보다 초기 장전량 합계가 큰 잘못된 입력을 만들기 위한 복사본입니다.
	FCFAmmoSortieLoad InvalidSortieAmmoLoad = SortieAmmoLoad;
	InvalidSortieAmmoLoad.InitialSortieAmmoCount = 25;
	TArray<FCFAmmoSortieLoad> InvalidSortieAmmoLoads;
	InvalidSortieAmmoLoads.Add(InvalidSortieAmmoLoad);
	TestFalse(TEXT("출격 총량보다 장전 합계가 크면 초기화 거부"), VehicleAmmoComp->InitializeAmmoRuntime(VehiclePawn, InvalidSortieAmmoLoads, WeaponInitializations));
	TestEqual(TEXT("실패한 재초기화가 기존 예비 상태를 훼손하지 않음"), VehicleAmmoComp->GetReserveAmmoCount(SharedAmmoData->AmmoId), 70);

	VehicleAmmoComp->ResetAmmoRuntime();
	TestFalse(TEXT("Reset 후 초기화 상태 해제"), VehicleAmmoComp->IsAmmoRuntimeInitialized());
	TestEqual(TEXT("Reset 후 Weapon Runtime 0"), VehicleAmmoComp->GetWeaponAmmoRuntimeCount(), 0);
	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
