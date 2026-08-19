// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-17
// Description: CF-FQ-034 FIT-P0-07 공식 Light / Default / Heavy Mobility Fitting Fixture 저장 계약 자동화 테스트
// Scope: 저장된 세 VehicleFittingData가 동일 SUV VehicleData를 사용하고 1000kg / 1570kg / 1600kg Snapshot과 Heavy 15발 계약을 유지하는지 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Fitting.FIT_P0_07.OfficialMobilityFixtures 최초 추가.
// Migration:
// - 이 테스트는 저장된 Fixture를 읽기만 하며 VehicleData, FittingData, WeaponData, AmmoData, DefenseData 또는 Map을 수정·저장하지 않습니다.
// - Mobility USER 주행감 PASS를 대체하지 않으며, 공인 Fixture의 정적 계약과 결정론적 Snapshot만 보호합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoData.h"
#include "CFFittingTypes.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingData.h"

#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMobilityFixtureContractTest,
	"CarFight.Fitting.FIT_P0_07.OfficialMobilityFixtures",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 저장된 공식 Mobility Fixture 3종의 동일 플랫폼, 질량 Breakdown, 순서와 Heavy 탄약 계약을 검증합니다.
bool FCFMobilityFixtureContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 공식 Mobility Fixture가 공통으로 사용해야 할 Fitting-ready SUV VehicleData 경로입니다.
	const TCHAR* ExpectedVehicleDataPath = TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV");
	// [v1.0.0] 장비·방어·탄약을 비운 1000kg Light Fixture 경로입니다.
	const TCHAR* LightFittingPath = TEXT("/Game/CarFight/Tests/Fitting/DA_Fit_MobilityLight.DA_Fit_MobilityLight");
	// [v1.0.0] 기존 Defense 기술 baseline을 보존한 1570kg Default Fixture 경로입니다.
	const TCHAR* DefaultFittingPath = TEXT("/Game/CarFight/Tests/Fitting/DA_Fit_MobilityDefault.DA_Fit_MobilityDefault");
	// [v1.0.0] 기존 HeavyFinite payload를 같은 SUV에 재조합한 1600kg Heavy Fixture 경로입니다.
	const TCHAR* HeavyFittingPath = TEXT("/Game/CarFight/Tests/Fitting/DA_Fit_MobilityHeavy.DA_Fit_MobilityHeavy");

	// [v1.0.0] 세 Fixture가 동일하게 참조해야 할 실제 저장 VehicleData입니다.
	UCFVehicleData* ExpectedVehicleData = LoadObject<UCFVehicleData>(nullptr, ExpectedVehicleDataPath);
	// [v1.0.0] 저장된 Light VehicleFittingData입니다.
	UCFVehicleFittingData* LightFitting = LoadObject<UCFVehicleFittingData>(nullptr, LightFittingPath);
	// [v1.0.0] 저장된 Default VehicleFittingData입니다.
	UCFVehicleFittingData* DefaultFitting = LoadObject<UCFVehicleFittingData>(nullptr, DefaultFittingPath);
	// [v1.0.0] 저장된 Heavy VehicleFittingData입니다.
	UCFVehicleFittingData* HeavyFitting = LoadObject<UCFVehicleFittingData>(nullptr, HeavyFittingPath);

	if (!TestNotNull(TEXT("공식 Mobility VehicleData 로드"), ExpectedVehicleData)
		|| !TestNotNull(TEXT("Light Fixture 로드"), LightFitting)
		|| !TestNotNull(TEXT("Default Fixture 로드"), DefaultFitting)
		|| !TestNotNull(TEXT("Heavy Fixture 로드"), HeavyFitting))
	{
		return false;
	}

	TestEqual(TEXT("공식 VehicleData Base 1000kg"), ExpectedVehicleData->BaseVehicleMassKg, 1000.0f);
	TestEqual(TEXT("공식 VehicleData Gross 2500kg"), ExpectedVehicleData->MaximumGrossMassKg, 2500.0f);
	TestTrue(TEXT("Light 동일 VehicleData"), LightFitting->VehicleData == ExpectedVehicleData);
	TestTrue(TEXT("Default 동일 VehicleData"), DefaultFitting->VehicleData == ExpectedVehicleData);
	TestTrue(TEXT("Heavy 동일 VehicleData"), HeavyFitting->VehicleData == ExpectedVehicleData);
	TestEqual(TEXT("Light FittingId"), LightFitting->FittingId, FName(TEXT("Mobility_Light")));
	TestEqual(TEXT("Default FittingId"), DefaultFitting->FittingId, FName(TEXT("Mobility_Default")));
	TestEqual(TEXT("Heavy FittingId"), HeavyFitting->FittingId, FName(TEXT("Mobility_Heavy")));

	// [v1.0.0] 저장된 Light Fixture에서 직접 생성한 결정론적 질량 Snapshot입니다.
	const FCFVehicleFittingSnapshot LightSnapshot = LightFitting->BuildFittingSnapshot();
	// [v1.0.0] 저장된 Default Fixture에서 직접 생성한 결정론적 질량 Snapshot입니다.
	const FCFVehicleFittingSnapshot DefaultSnapshot = DefaultFitting->BuildFittingSnapshot();
	// [v1.0.0] 저장된 Heavy Fixture에서 직접 생성한 결정론적 질량 Snapshot입니다.
	const FCFVehicleFittingSnapshot HeavySnapshot = HeavyFitting->BuildFittingSnapshot();

	TestTrue(TEXT("Light Snapshot 유효"), LightSnapshot.IsValid());
	TestEqual(TEXT("Light Base 1000kg"), LightSnapshot.BaseVehicleMassKg, 1000.0f);
	TestEqual(TEXT("Light Equipment 0kg"), LightSnapshot.EquipmentMassKg, 0.0f);
	TestEqual(TEXT("Light Ammo 0kg"), LightSnapshot.AmmoMassKg, 0.0f);
	TestEqual(TEXT("Light Defense 0kg"), LightSnapshot.DefenseMassKg, 0.0f);
	TestEqual(TEXT("Light Total 1000kg"), LightSnapshot.TotalVehicleMassKg, 1000.0f);
	TestEqual(TEXT("Light Gross 2500kg"), LightSnapshot.MaximumGrossMassKg, 2500.0f);
	TestEqual(TEXT("Light ResolvedMount 1개"), LightSnapshot.ResolvedMounts.Num(), 1);
	if (LightSnapshot.ResolvedMounts.Num() == 1)
	{
		TestEqual(TEXT("Light 명시적 빈 장착"), LightSnapshot.ResolvedMounts[0].SelectionSource, ECFFittingSelectionSource::ExplicitEmpty);
		TestNull(TEXT("Light EquipmentPreset 없음"), LightSnapshot.ResolvedMounts[0].EquipmentPresetData.Get());
	}
	TestEqual(TEXT("Light 명시적 Defense None"), LightSnapshot.ResolvedDefenseSelectionMode, ECFDefenseSelectionMode::ExplicitNone);
	TestNull(TEXT("Light DefenseData 없음"), LightSnapshot.ResolvedDefenseData.Get());

	TestTrue(TEXT("Default Snapshot 유효"), DefaultSnapshot.IsValid());
	TestEqual(TEXT("Default Base 1000kg"), DefaultSnapshot.BaseVehicleMassKg, 1000.0f);
	TestEqual(TEXT("Default Equipment 470kg"), DefaultSnapshot.EquipmentMassKg, 470.0f);
	TestEqual(TEXT("Default Ammo 0kg"), DefaultSnapshot.AmmoMassKg, 0.0f);
	TestEqual(TEXT("Default Defense 100kg"), DefaultSnapshot.DefenseMassKg, 100.0f);
	TestEqual(TEXT("Default Total 1570kg"), DefaultSnapshot.TotalVehicleMassKg, 1570.0f);
	TestEqual(TEXT("Default Gross 2500kg"), DefaultSnapshot.MaximumGrossMassKg, 2500.0f);
	TestEqual(TEXT("Default Vehicle Defense 사용"), DefaultSnapshot.ResolvedDefenseSelectionMode, ECFDefenseSelectionMode::UseVehicleDefault);
	TestNotNull(TEXT("Default DefenseData 존재"), DefaultSnapshot.ResolvedDefenseData.Get());

	TestTrue(TEXT("Heavy Snapshot 유효"), HeavySnapshot.IsValid());
	TestEqual(TEXT("Heavy Base 1000kg"), HeavySnapshot.BaseVehicleMassKg, 1000.0f);
	TestEqual(TEXT("Heavy Equipment 470kg"), HeavySnapshot.EquipmentMassKg, 470.0f);
	TestEqual(TEXT("Heavy Ammo 30kg"), HeavySnapshot.AmmoMassKg, 30.0f);
	TestEqual(TEXT("Heavy Defense 100kg"), HeavySnapshot.DefenseMassKg, 100.0f);
	TestEqual(TEXT("Heavy Total 1600kg"), HeavySnapshot.TotalVehicleMassKg, 1600.0f);
	TestEqual(TEXT("Heavy Gross 2500kg"), HeavySnapshot.MaximumGrossMassKg, 2500.0f);
	TestEqual(TEXT("Heavy Vehicle Defense 사용"), HeavySnapshot.ResolvedDefenseSelectionMode, ECFDefenseSelectionMode::UseVehicleDefault);
	TestEqual(TEXT("Heavy 출격 탄약 1종"), HeavySnapshot.InitialSortieAmmoLoads.Num(), 1);
	if (HeavySnapshot.InitialSortieAmmoLoads.Num() == 1)
	{
		// [v1.0.0] Heavy Fixture가 실제 적재하는 저장 HeavyShell 탄약 DataAsset입니다.
		const UCFAmmoData* HeavyAmmoData = HeavySnapshot.InitialSortieAmmoLoads[0].AmmoData.Get();
		TestNotNull(TEXT("Heavy AmmoData 존재"), HeavyAmmoData);
		TestEqual(TEXT("Heavy 출격 15발"), HeavySnapshot.InitialSortieAmmoLoads[0].InitialSortieAmmoCount, 15);
		if (HeavyAmmoData)
		{
			TestEqual(TEXT("Heavy Ammo UnitMass 2kg"), HeavyAmmoData->GetEffectiveUnitMassKg(), 2.0f);
		}
	}

	TestTrue(TEXT("Light < Default"), LightSnapshot.TotalVehicleMassKg < DefaultSnapshot.TotalVehicleMassKg);
	TestTrue(TEXT("Default < Heavy"), DefaultSnapshot.TotalVehicleMassKg < HeavySnapshot.TotalVehicleMassKg);
	TestTrue(TEXT("Heavy <= Gross"), HeavySnapshot.TotalVehicleMassKg <= HeavySnapshot.MaximumGrossMassKg);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
