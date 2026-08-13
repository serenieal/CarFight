// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-07 Fitting Ammo Mass Automation
// Scope: 명시적 출격 탄약 선택, 최대 적재 상한, finite 무기 초기 장전 충족, AmmoMassKg와 Snapshot→Ammo Runtime 구성을 검증합니다.
// Changelog:
// - v1.0.1: CreateNewMap을 Transient DataAsset 생성보다 먼저 수행해 Automation map 전환 GC 뒤 오래된 Fixture 포인터를 읽는 Access Violation을 제거.
// - v1.0.0: AMMO-P0-07 FittingMass 통합 Automation 최초 추가.
// Migration:
// - Automation World와 Transient DataAsset만 사용하며 저장 Asset을 생성하거나 수정하지 않습니다.
// - MaximumLoadableAmmoCount는 검증 상한으로만 사용하고 실제 출격 수량은 InitialSortieAmmoLoads에서만 읽습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFFittingTypes.h"
#include "CFTurretMountData.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] Snapshot에 지정 피팅 문제 코드가 존재하는지 반환합니다.
	bool HasAmmoFittingIssue(
		const FCFVehicleFittingSnapshot& Snapshot,
		const ECFFittingIssueCode IssueCode)
	{
		for (const FCFFittingValidationIssue& ValidationIssue : Snapshot.ValidationIssues)
		{
			if (ValidationIssue.IssueCode == IssueCode)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] finite WeaponData를 기본 장비로 사용하는 단일 Turret VehicleData를 구성합니다.
	UCFVehicleData* CreateAmmoFittingVehicleData(
		UObject* Outer,
		UCFWeaponData* WeaponData,
		UCFEquipmentPresetData*& OutEquipmentPresetData,
		UCFTurretMountData*& OutTurretMountData)
	{
		OutEquipmentPresetData = nullptr;
		OutTurretMountData = nullptr;
		if (!Outer || !WeaponData)
		{
			return nullptr;
		}

		// [v1.0.0] 피팅 EquipmentMassKg에 50kg을 기여할 Transient TurretMountData입니다.
		OutTurretMountData = NewObject<UCFTurretMountData>(Outer);
		OutTurretMountData->TurretMountId = TEXT("AmmoP007_Mount");
		OutTurretMountData->TurretMountWeightKg = 50.0f;

		// [v1.0.0] MountProfile 기본 장비가 finite WeaponData와 TurretMountData를 함께 참조하도록 할 Transient EquipmentPresetData입니다.
		OutEquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		OutEquipmentPresetData->EquipmentId = TEXT("AmmoP007_Preset");
		OutEquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		OutEquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		OutEquipmentPresetData->DefaultTurretMountData = OutTurretMountData;
		OutEquipmentPresetData->DefaultWeaponData = WeaponData;

		// [v1.0.0] 기준 질량과 단일 하드포인트·MountProfile을 소유할 Transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);
		VehicleData->BaseVehicleMassKg = 1000.0f;
		VehicleData->MaximumGrossMassKg = 2000.0f;

		// [v1.0.0] 단일 Turret MountProfile이 참조할 최소 Top_01 하드포인트입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Top");
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] finite WeaponData를 기본 장비로 해석할 단일 장착 프로파일입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
		MountProfile.LocationSlotRef = HardpointSlot.LocationSlotId;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
		MountProfile.DefaultEquipmentPresetData = OutEquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);
		return VehicleData;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFAmmoFittingMassTest,
	"CarFight.Ammo.AMMO_P0_07.FittingMass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 명시적 출격 탄약이 피팅 질량과 실제 Ammo Runtime의 단일 수량 원본으로 동작하는지 검증합니다.
bool FCFAmmoFittingMassTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.1] Transient DataAsset 생성 전에 먼저 확정해 이후 map 전환 GC가 Fixture UObject를 무효화하지 않게 할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("AMMO-P0-07 Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 실제 출격 수량·질량·상한을 정의할 Transient AmmoData입니다.
	UCFAmmoData* AmmoData = NewObject<UCFAmmoData>(GetTransientPackage());
	if (!TestNotNull(TEXT("AMMO-P0-07 AmmoData"), AmmoData))
	{
		return false;
	}
	AmmoData->AmmoId = TEXT("AmmoP007_HeavyShell");
	AmmoData->UnitMassKg = 2.0f;
	AmmoData->MaximumLoadableAmmoCount = 20;

	// [v1.0.0] 초기 장전 3발과 100kg 본체 질량을 가진 finite WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	if (!TestNotNull(TEXT("AMMO-P0-07 WeaponData"), WeaponData))
	{
		return false;
	}
	WeaponData->WeaponId = TEXT("AmmoP007_HeavyCannon");
	WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	WeaponData->WeaponMassKg = 100.0f;
	WeaponData->DefaultAmmoData = AmmoData;
	WeaponData->MagazineSize = 5;
	WeaponData->InitialLoadedAmmoCount = 3;
	WeaponData->AmmoUnitsPerShot = 1;
	WeaponData->bUseInfiniteAmmoForDebug = false;

	// [v1.0.0] 테스트 VehicleData가 참조할 장비 프리셋입니다.
	UCFEquipmentPresetData* EquipmentPresetData = nullptr;

	// [v1.0.0] 테스트 VehicleData가 참조할 터렛 마운트 데이터입니다.
	UCFTurretMountData* TurretMountData = nullptr;

	// [v1.0.0] Base 1000kg + Mount 50kg + Weapon 100kg을 가진 단일 장착 차량 데이터입니다.
	UCFVehicleData* VehicleData = CreateAmmoFittingVehicleData(
		GetTransientPackage(),
		WeaponData,
		EquipmentPresetData,
		TurretMountData);
	if (!TestNotNull(TEXT("AMMO-P0-07 VehicleData"), VehicleData)
		|| !TestNotNull(TEXT("AMMO-P0-07 EquipmentPresetData"), EquipmentPresetData)
		|| !TestNotNull(TEXT("AMMO-P0-07 TurretMountData"), TurretMountData))
	{
		return false;
	}

	// [v1.0.0] 실제 10발 출격 수량을 명시해 Snapshot과 Ammo Runtime의 원본으로 사용할 FittingData입니다.
	UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>(GetTransientPackage());
	if (!TestNotNull(TEXT("AMMO-P0-07 FittingData"), FittingData))
	{
		return false;
	}
	FittingData->FittingId = TEXT("AmmoP007_Fitting");
	FittingData->VehicleData = VehicleData;
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::UseVehicleDefault;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::ExplicitNone;

	// [v1.0.0] 이번 출격에 실제 장전+예비 전체 10발을 싣는 명시적 탄약 선택입니다.
	FCFAmmoSortieLoad SortieAmmoLoad;
	SortieAmmoLoad.AmmoData = AmmoData;
	SortieAmmoLoad.InitialSortieAmmoCount = 10;
	FittingData->InitialSortieAmmoLoads.Add(SortieAmmoLoad);

	// [v1.0.0] 정상 출격 탄약 선택을 포함한 기준 피팅 Snapshot입니다.
	const FCFVehicleFittingSnapshot ValidSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("10발 명시 적재 Snapshot 유효"), ValidSnapshot.IsValid());
	TestEqual(TEXT("Snapshot 출격 탄약 항목 1개"), ValidSnapshot.InitialSortieAmmoLoads.Num(), 1);
	TestEqual(TEXT("Snapshot 실제 출격 수량 10"), ValidSnapshot.InitialSortieAmmoLoads[0].InitialSortieAmmoCount, 10);
	TestTrue(TEXT("Snapshot AmmoData 동일"), ValidSnapshot.InitialSortieAmmoLoads[0].AmmoData == AmmoData);
	TestEqual(TEXT("장비 질량 150kg"), ValidSnapshot.EquipmentMassKg, 150.0f);
	TestEqual(TEXT("탄약 질량 10 × 2kg = 20kg"), ValidSnapshot.AmmoMassKg, 20.0f);
	TestEqual(TEXT("탑재 질량 170kg"), ValidSnapshot.PayloadMassKg, 170.0f);
	TestEqual(TEXT("총중량 1170kg"), ValidSnapshot.TotalVehicleMassKg, 1170.0f);

	FittingData->InitialSortieAmmoLoads[0].InitialSortieAmmoCount = 21;

	// [v1.0.0] 최대 적재 가능 20발을 1발 초과한 피팅 Snapshot입니다.
	const FCFVehicleFittingSnapshot OverMaximumSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("최대 적재량 초과 Snapshot 무효"), OverMaximumSnapshot.IsValid());
	TestTrue(TEXT("AmmoCountExceeded 문제 코드"), HasAmmoFittingIssue(OverMaximumSnapshot, ECFFittingIssueCode::AmmoCountExceeded));

	FittingData->InitialSortieAmmoLoads[0].InitialSortieAmmoCount = 2;

	// [v1.0.0] finite 무기 초기 장전 필요량 3발보다 출격 전체 수량이 적은 Snapshot입니다.
	const FCFVehicleFittingSnapshot InsufficientInitialLoadSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("초기 장전 필요량보다 적은 출격 수량 Snapshot 무효"), InsufficientInitialLoadSnapshot.IsValid());
	TestTrue(TEXT("초기 장전 부족 InvalidAmmoSelection"), HasAmmoFittingIssue(InsufficientInitialLoadSnapshot, ECFFittingIssueCode::InvalidAmmoSelection));

	FittingData->InitialSortieAmmoLoads[0].InitialSortieAmmoCount = 10;

	// [v1.0.0] 같은 AmmoId를 중복 기입해 이중 질량·수량 계산을 유도하지 못하게 검증할 두 번째 항목입니다.
	FCFAmmoSortieLoad DuplicateAmmoLoad = SortieAmmoLoad;
	DuplicateAmmoLoad.InitialSortieAmmoCount = 5;
	FittingData->InitialSortieAmmoLoads.Add(DuplicateAmmoLoad);

	// [v1.0.0] 같은 AmmoId를 두 번 선택한 중복 출격 탄약 Snapshot입니다.
	const FCFVehicleFittingSnapshot DuplicateAmmoSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("중복 AmmoId Snapshot 무효"), DuplicateAmmoSnapshot.IsValid());
	TestTrue(TEXT("중복 AmmoId InvalidAmmoSelection"), HasAmmoFittingIssue(DuplicateAmmoSnapshot, ECFFittingIssueCode::InvalidAmmoSelection));
	FittingData->InitialSortieAmmoLoads.SetNum(1);
	FittingData->InitialSortieAmmoLoads[0].InitialSortieAmmoCount = 10;

		// [v1.0.0] 실제 VehicleWeaponComp와 VehicleAmmoComp 기본 서브오브젝트를 소유할 테스트 차량입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("AMMO-P0-07 Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] 유효 Fitting Snapshot의 ResolvedMount와 같은 활성 무기를 구성할 Weapon 컴포넌트입니다.
	UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();

	// [v1.0.0] 유효 Fitting Snapshot의 실제 출격 탄약 목록을 Runtime 상태로 구성할 Ammo 컴포넌트입니다.
	UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();
	if (!TestNotNull(TEXT("AMMO-P0-07 VehicleWeaponComp"), VehicleWeaponComp)
		|| !TestNotNull(TEXT("AMMO-P0-07 VehicleAmmoComp"), VehicleAmmoComp))
	{
		return false;
	}
	TestTrue(TEXT("Snapshot 장비 기준 Weapon Runtime 초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));

	// [v1.0.0] Snapshot의 finite ResolvedMount에서 VehicleAmmoComp용 WeaponInstance 초기화 입력을 생성합니다.
	TArray<FCFWeaponAmmoInitialization> WeaponAmmoInitializations;
	for (const FCFResolvedFittingMount& ResolvedMount : ValidSnapshot.ResolvedMounts)
	{
		if (!IsValid(ResolvedMount.WeaponData) || !ResolvedMount.WeaponData->UsesFiniteAmmoRuntime())
		{
			continue;
		}

		// [v1.0.0] 같은 WeaponData라도 MountProfileId별 Loaded를 독립 소유하게 할 Runtime 초기화 입력입니다.
		FCFWeaponAmmoInitialization WeaponAmmoInitialization;
		WeaponAmmoInitialization.WeaponInstanceId = ResolvedMount.MountProfileId;
		WeaponAmmoInitialization.WeaponData = ResolvedMount.WeaponData;
		WeaponAmmoInitialization.InitialLoadedAmmoCountOverride = INDEX_NONE;
		WeaponAmmoInitializations.Add(WeaponAmmoInitialization);
	}
	TestEqual(TEXT("finite WeaponInstance 초기화 입력 1개"), WeaponAmmoInitializations.Num(), 1);
	TestTrue(
		TEXT("Fitting Snapshot 출격 탄약으로 VehicleAmmoComp 초기화"),
		VehicleAmmoComp->InitializeAmmoRuntime(
			VehiclePawn,
			ValidSnapshot.InitialSortieAmmoLoads,
			WeaponAmmoInitializations));

	// [v1.0.0] Snapshot 10발이 초기 장전 3 + 공유 예비 7로 정확히 분할됐는지 확인할 Runtime Snapshot입니다.
	FCFAmmoRuntimeSnapshot RuntimeAmmoSnapshot;
	TestTrue(
		TEXT("Fitting Snapshot 기반 Ammo Runtime 조회"),
		VehicleAmmoComp->TryGetAmmoSnapshot(TEXT("RoofTurret_MediumOrLarge"), RuntimeAmmoSnapshot));
	TestEqual(TEXT("Fitting Runtime 장전량 3"), RuntimeAmmoSnapshot.LoadedAmmoCount, 3);
	TestEqual(TEXT("Fitting Runtime 예비량 7"), RuntimeAmmoSnapshot.ReserveAmmoCount, 7);
	TestEqual(TEXT("Fitting Runtime 현재 사용 가능량 10"), RuntimeAmmoSnapshot.CurrentUsableAmmoCount, 10);
	TestEqual(TEXT("Fitting Runtime 차량 전체 보유량 10"), RuntimeAmmoSnapshot.CurrentOnboardAmmoCount, 10);

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
