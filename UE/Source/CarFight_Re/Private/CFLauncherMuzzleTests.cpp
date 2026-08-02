// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-28
// Description: CF-FQ-029 LM-P0-02 가변 Muzzle 순환 자동화 테스트
// Scope: 기존 단일 Muzzle 기본값과 승인 발사 기반 SingleCycle 인덱스 진행·순환·Reset 계약을 검증합니다.
// Changelog:
// - v1.0.0: Muzzle 데이터 기본값과 VehicleWeaponComp 순환 상태 RuntimeContract 최초 추가.
// Migration:
// - 실제 StaticMesh 소켓 존재와 시각 발사 순서는 Editor PIE에서 별도로 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFTurretMountData.h"
#include "CFProjectileLaunchTypes.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleWeaponComp.h"
#include "CFVehicleWeaponTypes.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFLauncherMuzzleSequenceTest,
	"CarFight.Launcher.LM_P0_02.MuzzleSequence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Legacy 기본값과 승인 발사 기반 SingleCycle 순환·Reset 계약을 검증합니다.
bool FCFLauncherMuzzleSequenceTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기존 DataAsset 재저장 없이 단일 Muzzle fallback을 유지할 기본 터렛 데이터입니다.
	const UCFTurretMountData* DefaultTurretMountData = GetDefault<UCFTurretMountData>();
	TestNotNull(TEXT("기본 TurretMountData CDO 존재"), DefaultTurretMountData);
	if (DefaultTurretMountData)
	{
		TestTrue(TEXT("기본 Muzzle 배열은 비어 있음"), DefaultTurretMountData->MuzzleSocketNames.IsEmpty());
		TestEqual(TEXT("기존 단일 Muzzle 이름 유지"), DefaultTurretMountData->MuzzleSocketName, FName(TEXT("Muzzle")));
		TestFalse(TEXT("기본값은 누락 Muzzle을 건너뛸 수 있음"), DefaultTurretMountData->bRequireAllMuzzles);
	}

	// [v1.0.0] 선택 Muzzle가 아직 해결되지 않은 FireOrigin 기본값입니다.
	const FCFVehicleFireOrigin DefaultFireOrigin;
	TestEqual(TEXT("FireOrigin 기본 Muzzle 이름은 None"), DefaultFireOrigin.MuzzleSocketName, NAME_None);
	TestEqual(TEXT("FireOrigin 기본 Muzzle 인덱스는 INDEX_NONE"), DefaultFireOrigin.MuzzleSocketIndex, INDEX_NONE);
	TestEqual(TEXT("FireOrigin 기본 Muzzle 슬롯 수는 0"), DefaultFireOrigin.MuzzleSocketCount, 0);

	// [v1.0.0] 기존 Aim 경로가 안전하게 생성할 FireRequest 기본값입니다.
	const FCFVehicleFireRequest DefaultFireRequest;
	TestEqual(TEXT("FireRequest 기본 Muzzle 이름은 None"), DefaultFireRequest.MuzzleSocketName, NAME_None);
	TestEqual(TEXT("FireRequest 기본 Muzzle 인덱스는 INDEX_NONE"), DefaultFireRequest.MuzzleSocketIndex, INDEX_NONE);
	TestEqual(TEXT("FireRequest 기본 Muzzle 슬롯 수는 0"), DefaultFireRequest.MuzzleSocketCount, 0);

	// [v1.0.0] 기존 직접 Projectile API가 안전하게 생성할 Launch Context 기본값입니다.
	const FCFProjectileLaunchContext DefaultLaunchContext;
	TestEqual(TEXT("Launch Context 기본 Muzzle 이름은 None"), DefaultLaunchContext.MuzzleSocketName, NAME_None);
	TestEqual(TEXT("Launch Context 기본 Muzzle 인덱스는 INDEX_NONE"), DefaultLaunchContext.MuzzleSocketIndex, INDEX_NONE);
	TestEqual(TEXT("Launch Context 기본 Muzzle 슬롯 수는 0"), DefaultLaunchContext.MuzzleSocketCount, 0);

	// [v1.0.0] 순수 런타임 Muzzle 상태 계약을 검증할 임시 WeaponComp입니다.
	UCFVehicleWeaponComp* WeaponComp = NewObject<UCFVehicleWeaponComp>();
	if (!TestNotNull(TEXT("테스트 WeaponComp 생성"), WeaponComp))
	{
		return false;
	}

	TestEqual(TEXT("초기 다음 Muzzle 인덱스는 0"), WeaponComp->GetNextMuzzleSocketIndex(), 0);

	WeaponComp->RecordResolvedMuzzleSelection(TEXT("Muzzle_1"), 0, 4);
	TestEqual(TEXT("해결만으로 다음 인덱스를 진행하지 않음"), WeaponComp->GetNextMuzzleSocketIndex(), 0);

	WeaponComp->AdvanceMuzzleSequenceAfterAcceptedFire(TEXT("Muzzle_1"), 0, 4);
	TestEqual(TEXT("첫 승인 발사 뒤 다음 인덱스는 1"), WeaponComp->GetNextMuzzleSocketIndex(), 1);
	TestTrue(TEXT("요약에 마지막 발사 Muzzle 기록"), WeaponComp->BuildMuzzleSequenceSummary().Contains(TEXT("LastFired=Muzzle_1")));
	TestTrue(TEXT("요약에 진행 횟수 1 기록"), WeaponComp->BuildMuzzleSequenceSummary().Contains(TEXT("AdvanceCount=1")));

	WeaponComp->AdvanceMuzzleSequenceAfterAcceptedFire(TEXT("Muzzle_4"), 3, 4);
	TestEqual(TEXT("마지막 배열 인덱스 승인 뒤 0으로 순환"), WeaponComp->GetNextMuzzleSocketIndex(), 0);

	WeaponComp->AdvanceMuzzleSequenceAfterAcceptedFire(NAME_None, INDEX_NONE, 0);
	TestEqual(TEXT("무효 입력은 현재 순서를 변경하지 않음"), WeaponComp->GetNextMuzzleSocketIndex(), 0);

	WeaponComp->AdvanceMuzzleSequenceAfterAcceptedFire(TEXT("Muzzle"), 0, 1);
	TestEqual(TEXT("기존 단일 Muzzle는 항상 0으로 순환"), WeaponComp->GetNextMuzzleSocketIndex(), 0);

	WeaponComp->ResetMuzzleSequence();
	TestEqual(TEXT("Reset 뒤 다음 인덱스는 0"), WeaponComp->GetNextMuzzleSocketIndex(), 0);
	TestTrue(TEXT("Reset 뒤 마지막 발사 Muzzle은 None"), WeaponComp->BuildMuzzleSequenceSummary().Contains(TEXT("LastFired=None")));
	TestTrue(TEXT("Reset 뒤 진행 횟수는 0"), WeaponComp->BuildMuzzleSequenceSummary().Contains(TEXT("AdvanceCount=0")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
