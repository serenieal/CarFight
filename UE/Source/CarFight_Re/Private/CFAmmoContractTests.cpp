// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-01 Data Contract Automation
// Scope: AmmoData 유효값, WeaponData 기존 무한탄 기본값과 유한탄 설정 보정을 검증합니다.
// Changelog:
// - v1.0.0: 최초 Ammo Data/Weapon Config Contract 테스트 추가.
// Migration:
// - Transient UObject만 사용하며 Unreal Asset을 생성하거나 저장하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoData.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFAmmoDataContractTest,
	"CarFight.Ammo.AMMO_P0_01.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 기존 WeaponData 무한탄 호환 기본값과 신규 탄약 정적 유효값 계약을 검증합니다.
bool FCFAmmoDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 신규 에셋 저장 없이 정적 탄종 계약을 검증할 Transient AmmoData입니다.
	UCFAmmoData* AmmoData = NewObject<UCFAmmoData>(GetTransientPackage());
	// [v1.0.0] 기존 WeaponData 기본값과 신규 탄약 설정을 검증할 Transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient AmmoData"), AmmoData)
		|| !TestNotNull(TEXT("Transient WeaponData"), WeaponData))
	{
		return false;
	}

	TestTrue(TEXT("기존 WeaponData 기본값은 무한탄 호환"), WeaponData->bUseInfiniteAmmoForDebug);
	TestFalse(TEXT("기존 WeaponData는 DefaultAmmoData 없이 유한탄 비활성"), WeaponData->UsesFiniteAmmoRuntime());

	AmmoData->AmmoId = TEXT("Test_30mm_AP");
	AmmoData->UnitMassKg = -5.0f;
	AmmoData->MaximumLoadableAmmoCount = -10;
	TestTrue(TEXT("AmmoId 설정 후 유효"), AmmoData->IsAmmoDataValid());
	TestEqual(TEXT("음수 탄약 질량 0 보정"), AmmoData->GetEffectiveUnitMassKg(), 0.0f);
	TestEqual(TEXT("음수 최대 적재량 0 보정"), AmmoData->GetEffectiveMaximumLoadableAmmoCount(), 0);

	WeaponData->DefaultAmmoData = AmmoData;
	WeaponData->bUseInfiniteAmmoForDebug = false;
	WeaponData->MagazineSize = 20;
	WeaponData->InitialLoadedAmmoCount = 25;
	WeaponData->AmmoUnitsPerShot = 2;
	WeaponData->ReloadTimeSeconds = -3.0f;
	TestTrue(TEXT("명시 설정 후 유한탄 Runtime 사용"), WeaponData->UsesFiniteAmmoRuntime());
	TestEqual(TEXT("탄창 용량 20"), WeaponData->GetEffectiveMagazineCapacity(), 20);
	TestEqual(TEXT("초기 장전량은 탄창 용량 20으로 Clamp"), WeaponData->GetEffectiveInitialLoadedAmmoCount(), 20);
	TestEqual(TEXT("발사당 탄약 단위 2"), WeaponData->GetEffectiveAmmoUnitsPerShot(), 2);
	TestEqual(TEXT("음수 재장전 시간 0 보정"), WeaponData->GetEffectiveReloadTimeSeconds(), 0.0f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
