// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.2
// Date: 2026-09-02
// Description: CF-FQ-041 RTA-P0-01 Runtime Test Catalog 계약 회귀
// Scope: Empty/Valid/Null/Duplicate 목록과 Config locator의 기본 경로 계약을 검증합니다.
// Changelog:
// - v1.0.2: RTA-P0-05 fitting-ready E2E 차량 추가를 반영해 기본 Catalog 차량 4 / 장비 2와 DA_VehicleDefense_TestSUV exact membership을 검증.
// - v1.0.1: persisted 기본 Catalog를 Runtime Settings 경로로 실제 로드해 차량 3 / 장비 2와 runtime validation을 검증.
// - v1.0.0: Catalog runtime validation과 DefaultGame Config path 회귀를 추가.
// Migration:
// - 기본 Catalog 차량 수는 RTA-P0-05 이후 4개이며 DA_VehicleDefense_TestSUV를 포함해야 합니다. persisted .uasset은 Config locator runtime load Automation과 AssetDump evidence를 함께 사용합니다.

#include "CFRuntimeTestCatalogData.h"
#include "CFRuntimeTestSettings.h"

#include "CFEquipmentPresetData.h"
#include "CFVehicleData.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeTestCatalogContractTest,
	"CarFight.RuntimeApply.RTA_P0_01.CatalogContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Runtime Test Catalog의 최소 등록 계약과 fail-closed validation을 검증합니다.
bool FCFRuntimeTestCatalogContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 독립 검증에 사용할 transient Catalog입니다.
	UCFRuntimeTestCatalogData* Catalog = NewObject<UCFRuntimeTestCatalogData>(GetTransientPackage());
	// [v1.0.0] 정상 차량 항목으로 사용할 transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	// [v1.0.0] 정상 장비 항목으로 사용할 transient EquipmentPresetData입니다.
	UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(GetTransientPackage());

	if (!TestNotNull(TEXT("Catalog 생성"), Catalog)
		|| !TestNotNull(TEXT("VehicleData 생성"), VehicleData)
		|| !TestNotNull(TEXT("EquipmentPresetData 생성"), EquipmentPresetData))
	{
		return false;
	}

	TestFalse(TEXT("빈 Catalog는 사용 불가"), Catalog->IsRuntimeTestCatalogUsable());

	Catalog->AllowedVehicleData.Add(VehicleData);
	Catalog->AllowedEquipmentPresetData.Add(EquipmentPresetData);
	TestTrue(TEXT("차량/장비 각 1개 Catalog는 사용 가능"), Catalog->IsRuntimeTestCatalogUsable());

	Catalog->AllowedVehicleData.Add(VehicleData);
	TestFalse(TEXT("중복 VehicleData는 거부"), Catalog->IsRuntimeTestCatalogUsable());
	Catalog->AllowedVehicleData.SetNum(1);

	Catalog->AllowedEquipmentPresetData.Add(nullptr);
	TestFalse(TEXT("Null EquipmentPresetData는 거부"), Catalog->IsRuntimeTestCatalogUsable());
	Catalog->AllowedEquipmentPresetData.SetNum(1);

	TestTrue(TEXT("복구된 Catalog는 다시 사용 가능"), Catalog->IsRuntimeTestCatalogUsable());
	TestTrue(TEXT("요약에 차량 수 포함"), Catalog->BuildRuntimeTestCatalogSummary().Contains(TEXT("Vehicles=1")));
	TestTrue(TEXT("요약에 장비 수 포함"), Catalog->BuildRuntimeTestCatalogSummary().Contains(TEXT("Equipment=1")));

	// [v1.0.0] DefaultGame.ini에서 로드되는 기본 Runtime Test Catalog 설정 CDO입니다.
	const UCFRuntimeTestSettings* RuntimeTestSettings = GetDefault<UCFRuntimeTestSettings>();
	TestNotNull(TEXT("Runtime Test Settings CDO"), RuntimeTestSettings);
	if (RuntimeTestSettings)
	{
		TestEqual(
			TEXT("기본 Catalog Config 경로"),
			RuntimeTestSettings->DefaultCatalog.ToSoftObjectPath().ToString(),
			FString(TEXT("/Game/CarFight/Debug/Data/DA_CFRuntimeTestCatalog_Default.DA_CFRuntimeTestCatalog_Default")));

		// [v1.0.1] Editor Asset Registry enumeration 없이 Config soft reference로 실제 저장 Catalog를 로드한 결과입니다.
		UCFRuntimeTestCatalogData* LoadedDefaultCatalog = RuntimeTestSettings->LoadDefaultCatalog();
		if (TestNotNull(TEXT("기본 Catalog Runtime load"), LoadedDefaultCatalog))
		{
			TestEqual(TEXT("기본 Catalog 차량 수"), LoadedDefaultCatalog->AllowedVehicleData.Num(), 4);
			TestEqual(TEXT("기본 Catalog 장비 수"), LoadedDefaultCatalog->AllowedEquipmentPresetData.Num(), 2);

			// [v1.0.2] RTA-P0-05 성공형 Equipment E2E의 기준 차량으로 Catalog에 등록되어야 하는 persisted VehicleData입니다.
			UCFVehicleData* FittingReadyVehicleData = LoadObject<UCFVehicleData>(
				nullptr,
				TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV"));
			if (TestNotNull(TEXT("기본 Catalog fitting-ready 차량 load"), FittingReadyVehicleData))
			{
				TestTrue(
					TEXT("기본 Catalog에 DA_VehicleDefense_TestSUV exact membership"),
					LoadedDefaultCatalog->AllowedVehicleData.Contains(FittingReadyVehicleData));
			}

			TestTrue(TEXT("기본 Catalog Runtime validation"), LoadedDefaultCatalog->IsRuntimeTestCatalogUsable());
		}
	}

	return true;
}

#endif
