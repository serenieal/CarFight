// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-034 FIT-P0-06 Fitting ViewData·Debug Blueprint 계약 자동화 테스트
// Scope: BlueprintReadOnly reflection, Snapshot 순서·값 보존, 실제 출격 Ammo, 질량 Breakdown, PhysicalMassOnly Mobility와 Invalid Issue 표시를 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Fitting.FIT_P0_06.BlueprintContract 회귀를 최초 추가.
// Migration:
// - World/Pawn/Widget을 생성하지 않고 Transient DataAsset과 순수 Snapshot/ViewData만 사용합니다.
// - 이 테스트는 Fitting 계산을 재검증하는 것이 아니라 ViewData가 기존 Snapshot 계약을 왜곡하지 않는지 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFittingViewData.h"

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

namespace
{
	// [v1.0.0] 지정 Fitting ViewData Property가 Reflection에 존재하고 Blueprint에서 읽기 전용인지 검증합니다.
	bool TestBlueprintReadOnlyProperty(
		FAutomationTestBase& Test,
		UScriptStruct* ViewDataStruct,
		const FName PropertyName)
	{
		// [v1.0.0] 지정 이름으로 찾은 실제 Unreal Reflection Property입니다.
		const FProperty* Property = ViewDataStruct ? ViewDataStruct->FindPropertyByName(PropertyName) : nullptr;
		if (!Test.TestNotNull(*FString::Printf(TEXT("Blueprint Property 존재: %s"), *PropertyName.ToString()), Property))
		{
			return false;
		}

		Test.TestTrue(
			*FString::Printf(TEXT("BlueprintVisible: %s"), *PropertyName.ToString()),
			Property->HasAnyPropertyFlags(CPF_BlueprintVisible));
		Test.TestTrue(
			*FString::Printf(TEXT("BlueprintReadOnly: %s"), *PropertyName.ToString()),
			Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly));
		return true;
	}

	// [v1.0.0] ViewData Builder가 그대로 투영할 정상 Snapshot을 생성합니다.
	FCFVehicleFittingSnapshot BuildValidViewSnapshot()
	{
		// [v1.0.0] Asset 이름 fallback 검증에 사용할 Transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("DA_ViewVehicle"));
		VehicleData->BaseVehicleMassKg = 1000.0f;
		VehicleData->MaximumGrossMassKg = 1500.0f;

		// [v1.0.0] Mount 표시 이름과 참조 보존을 검증할 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(GetTransientPackage(), TEXT("DA_ViewEquipment"));
		EquipmentPresetData->EquipmentId = TEXT("ViewEquipment");
		EquipmentPresetData->DisplayName = FText::FromString(TEXT("View Cannon Kit"));

		// [v1.0.0] Defense 선택 ID와 참조 보존을 검증할 VehicleDefenseData입니다.
		UCFVehicleDefenseData* DefenseData = NewObject<UCFVehicleDefenseData>(GetTransientPackage(), TEXT("DA_ViewDefense"));
		DefenseData->DefenseId = TEXT("ViewDefense");
		DefenseData->DefenseMassKg = 50.0f;

		// [v1.0.0] 실제 출격 Count와 단위 질량 표시를 검증할 AmmoData입니다.
		UCFAmmoData* AmmoData = NewObject<UCFAmmoData>(GetTransientPackage(), TEXT("DA_ViewAmmo"));
		AmmoData->AmmoId = TEXT("ViewAmmo");
		AmmoData->AmmoDisplayName = FText::FromString(TEXT("View Rocket"));
		AmmoData->UnitMassKg = 2.0f;
		AmmoData->MaximumLoadableAmmoCount = 99;

		// [v1.0.0] 정상 ViewData가 복사할 결정론적 Snapshot입니다.
		FCFVehicleFittingSnapshot Snapshot;
		Snapshot.FittingId = TEXT("ViewFitting");
		Snapshot.VehicleData = VehicleData;
		Snapshot.ValidationState = ECFFittingValidationState::ValidWithWarnings;

		// [v1.0.0] 첫 번째 Mount 순서·참조·질량을 검증할 RoofTurret 해석 결과입니다.
		FCFResolvedFittingMount RoofMount;
		RoofMount.MountProfileId = TEXT("RoofTurret");
		RoofMount.LocationSlotId = TEXT("Top_01");
		RoofMount.MountType = ECFVehicleMountType::Turret;
		RoofMount.SizeLimit = ECFVehicleWeaponSize::Large;
		RoofMount.EquipmentPresetData = EquipmentPresetData;
		RoofMount.SelectionSource = ECFFittingSelectionSource::FittingOverride;
		RoofMount.TurretMountMassKg = 20.0f;
		RoofMount.WeaponMassKg = 100.0f;
		Snapshot.ResolvedMounts.Add(RoofMount);

		// [v1.0.0] 두 번째 Mount의 빈 장착 소스와 배열 순서 보존을 검증할 결과입니다.
		FCFResolvedFittingMount EmptyMount;
		EmptyMount.MountProfileId = TEXT("FrontUtility");
		EmptyMount.LocationSlotId = TEXT("Front_01");
		EmptyMount.MountType = ECFVehicleMountType::Utility;
		EmptyMount.SizeLimit = ECFVehicleWeaponSize::Small;
		EmptyMount.SelectionSource = ECFFittingSelectionSource::ExplicitEmpty;
		Snapshot.ResolvedMounts.Add(EmptyMount);

		// [v1.0.0] 실제 출격 수량 7을 최대 적재량 99와 구분할 Ammo load입니다.
		FCFAmmoSortieLoad AmmoSortieLoad;
		AmmoSortieLoad.AmmoData = AmmoData;
		AmmoSortieLoad.InitialSortieAmmoCount = 7;
		Snapshot.InitialSortieAmmoLoads.Add(AmmoSortieLoad);

		Snapshot.ResolvedDefenseSelectionMode = ECFDefenseSelectionMode::Override;
		Snapshot.ResolvedDefenseData = DefenseData;
		Snapshot.BaseVehicleMassKg = 1000.0f;
		Snapshot.EquipmentMassKg = 120.0f;
		Snapshot.AmmoMassKg = 14.0f;
		Snapshot.DefenseMassKg = 50.0f;
		Snapshot.PayloadMassKg = 184.0f;
		Snapshot.TotalVehicleMassKg = 1184.0f;
		Snapshot.MaximumGrossMassKg = 1500.0f;
		Snapshot.PayloadUsageRatio = 184.0f / 500.0f;
		Snapshot.GrossMassUsageRatio = 1184.0f / 1500.0f;

		// [v1.0.0] ValidWithWarnings 상태에서도 UI가 잃지 않고 표시해야 할 Warning 한 건입니다.
		FCFFittingValidationIssue WarningIssue;
		WarningIssue.Severity = ECFFittingIssueSeverity::Warning;
		WarningIssue.IssueCode = ECFFittingIssueCode::MissingMassSource;
		WarningIssue.Message = FText::FromString(TEXT("View warning"));
		WarningIssue.MountProfileId = TEXT("FrontUtility");
		Snapshot.ValidationIssues.Add(WarningIssue);
		return Snapshot;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFittingViewBlueprintContractTest,
	"CarFight.Fitting.FIT_P0_06.BlueprintContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Fitting ViewData의 BlueprintReadOnly 계약과 Snapshot 투영 정확성을 검증합니다.
bool FCFFittingViewBlueprintContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Blueprint/UMG가 직접 읽을 최상위 ViewData Reflection 구조체입니다.
	UScriptStruct* ViewDataStruct = FCFVehicleFittingViewData::StaticStruct();
	if (!TestNotNull(TEXT("CFVehicleFittingViewData Reflection 존재"), ViewDataStruct))
	{
		return false;
	}

	// [v1.0.0] FIT-P0-06 Blueprint 계약에서 반드시 노출되어야 할 핵심 Property 이름입니다.
	const TArray<FName> RequiredBlueprintProperties = {
		TEXT("VehicleData"),
		TEXT("VehicleDisplayName"),
		TEXT("FittingId"),
		TEXT("ValidationState"),
		TEXT("bCanApply"),
		TEXT("MountRows"),
		TEXT("DefenseRow"),
		TEXT("AmmoRows"),
		TEXT("MassRows"),
		TEXT("TotalVehicleMassKg"),
		TEXT("MaximumGrossMassKg"),
		TEXT("PayloadUsageRatio"),
		TEXT("GrossMassUsageRatio"),
		TEXT("MobilityPreview"),
		TEXT("IssueRows"),
		TEXT("DebugSummary")
	};
	for (const FName PropertyName : RequiredBlueprintProperties)
	{
		TestBlueprintReadOnlyProperty(*this, ViewDataStruct, PropertyName);
	}

	// [v1.0.0] 정상 Snapshot → ViewData projection을 검증할 원본입니다.
	const FCFVehicleFittingSnapshot ValidSnapshot = BuildValidViewSnapshot();
	// [v1.0.0] Snapshot을 단 한 번 순수 Builder에 통과시킨 정상 ViewData입니다.
	const FCFVehicleFittingViewData ValidViewData = FCFFittingViewBuilder::Build(ValidSnapshot);

	TestTrue(TEXT("ValidWithWarnings Snapshot 적용 가능"), ValidViewData.bCanApply);
	TestEqual(TEXT("FittingId 그대로"), ValidViewData.FittingId, ValidSnapshot.FittingId);
	TestEqual(TEXT("VehicleData 참조 그대로"), ValidViewData.VehicleData.Get(), ValidSnapshot.VehicleData.Get());
	TestTrue(TEXT("Vehicle 이름 Asset fallback 명시"), ValidViewData.bVehicleDisplayNameUsesAssetName);
	TestEqual(TEXT("Vehicle 이름 Asset 이름 사용"), ValidViewData.VehicleDisplayName.ToString(), FString(TEXT("DA_ViewVehicle")));
	TestEqual(TEXT("ValidationState 그대로"), ValidViewData.ValidationState, ValidSnapshot.ValidationState);

	TestEqual(TEXT("Mount 행 수 그대로"), ValidViewData.MountRows.Num(), ValidSnapshot.ResolvedMounts.Num());
	if (ValidViewData.MountRows.Num() >= 2)
	{
		TestEqual(TEXT("Mount[0] 순서 보존"), ValidViewData.MountRows[0].MountProfileId, FName(TEXT("RoofTurret")));
		TestEqual(TEXT("Mount[0] 장비 표시 이름"), ValidViewData.MountRows[0].EquipmentDisplayName.ToString(), FString(TEXT("View Cannon Kit")));
		TestEqual(TEXT("Mount[0] Turret 질량 그대로"), ValidViewData.MountRows[0].TurretMountMassKg, 20.0f);
		TestEqual(TEXT("Mount[0] Weapon 질량 그대로"), ValidViewData.MountRows[0].WeaponMassKg, 100.0f);
		TestEqual(TEXT("Mount[1] 순서 보존"), ValidViewData.MountRows[1].MountProfileId, FName(TEXT("FrontUtility")));
		TestEqual(TEXT("Mount[1] ExplicitEmpty 보존"), ValidViewData.MountRows[1].SelectionSource, ECFFittingSelectionSource::ExplicitEmpty);
	}

	TestEqual(TEXT("Defense SelectionMode 그대로"), ValidViewData.DefenseRow.SelectionMode, ECFDefenseSelectionMode::Override);
	TestEqual(TEXT("Defense ID 표시"), ValidViewData.DefenseRow.DefenseId, FName(TEXT("ViewDefense")));
	TestEqual(TEXT("Defense 질량 Snapshot 그대로"), ValidViewData.DefenseRow.DefenseMassKg, ValidSnapshot.DefenseMassKg);

	TestEqual(TEXT("Ammo 행 수 그대로"), ValidViewData.AmmoRows.Num(), 1);
	if (ValidViewData.AmmoRows.Num() == 1)
	{
		TestEqual(TEXT("Ammo ID 표시"), ValidViewData.AmmoRows[0].AmmoId, FName(TEXT("ViewAmmo")));
		TestEqual(TEXT("Ammo 표시 이름"), ValidViewData.AmmoRows[0].AmmoDisplayName.ToString(), FString(TEXT("View Rocket")));
		TestEqual(TEXT("실제 출격 Ammo Count 7 유지"), ValidViewData.AmmoRows[0].InitialSortieAmmoCount, 7);
		TestNotEqual(TEXT("MaximumLoadable 99를 현재 수량으로 오용하지 않음"), ValidViewData.AmmoRows[0].InitialSortieAmmoCount, 99);
		TestEqual(TEXT("Ammo UnitMass 표시"), ValidViewData.AmmoRows[0].UnitMassKg, 2.0f);
	}

	TestEqual(TEXT("MassRows 고정 7행"), ValidViewData.MassRows.Num(), 7);
	if (ValidViewData.MassRows.Num() == 7)
	{
		TestEqual(TEXT("Mass Base 그대로"), ValidViewData.MassRows[0].MassKg, ValidSnapshot.BaseVehicleMassKg);
		TestEqual(TEXT("Mass Equipment 그대로"), ValidViewData.MassRows[1].MassKg, ValidSnapshot.EquipmentMassKg);
		TestEqual(TEXT("Mass Ammo 그대로"), ValidViewData.MassRows[2].MassKg, ValidSnapshot.AmmoMassKg);
		TestEqual(TEXT("Mass Defense 그대로"), ValidViewData.MassRows[3].MassKg, ValidSnapshot.DefenseMassKg);
		TestEqual(TEXT("Mass Payload 그대로"), ValidViewData.MassRows[4].MassKg, ValidSnapshot.PayloadMassKg);
		TestEqual(TEXT("Mass Total 그대로"), ValidViewData.MassRows[5].MassKg, ValidSnapshot.TotalVehicleMassKg);
		TestEqual(TEXT("Mass MaximumGross 그대로"), ValidViewData.MassRows[6].MassKg, ValidSnapshot.MaximumGrossMassKg);
	}
	TestEqual(TEXT("TotalVehicleMass 그대로"), ValidViewData.TotalVehicleMassKg, ValidSnapshot.TotalVehicleMassKg);
	TestEqual(TEXT("PayloadUsageRatio 재계산 없이 그대로"), ValidViewData.PayloadUsageRatio, ValidSnapshot.PayloadUsageRatio);
	TestEqual(TEXT("GrossMassUsageRatio 재계산 없이 그대로"), ValidViewData.GrossMassUsageRatio, ValidSnapshot.GrossMassUsageRatio);

	TestEqual(TEXT("Mobility PhysicalMassOnly"), ValidViewData.MobilityPreview.PreviewMode, ECFFittingMobilityPreviewMode::PhysicalMassOnly);
	TestTrue(TEXT("MassRatio Total/Base"), FMath::IsNearlyEqual(ValidViewData.MobilityPreview.MassRatio, 1.184f, 0.0001f));
	TestEqual(TEXT("추가 가속 Scale 기본 1"), ValidViewData.MobilityPreview.AccelerationScale, 1.0f);
	TestEqual(TEXT("추가 제동 Scale 기본 1"), ValidViewData.MobilityPreview.BrakingScale, 1.0f);
	TestEqual(TEXT("추가 조향 Scale 기본 1"), ValidViewData.MobilityPreview.SteeringResponseScale, 1.0f);
	TestFalse(TEXT("추가 Mobility Adapter 비활성"), ValidViewData.MobilityPreview.bAdditionalAdapterApplied);

	TestEqual(TEXT("Issue 행 수 그대로"), ValidViewData.IssueRows.Num(), 1);
	if (ValidViewData.IssueRows.Num() == 1)
	{
		TestEqual(TEXT("Issue severity 그대로"), ValidViewData.IssueRows[0].Severity, ECFFittingIssueSeverity::Warning);
		TestEqual(TEXT("Issue code 그대로"), ValidViewData.IssueRows[0].IssueCode, ECFFittingIssueCode::MissingMassSource);
		TestEqual(TEXT("Issue message 그대로"), ValidViewData.IssueRows[0].Message.ToString(), FString(TEXT("View warning")));
	}
	TestTrue(TEXT("DebugSummary FittingId 포함"), ValidViewData.DebugSummary.Contains(TEXT("ViewFitting")));
	TestTrue(TEXT("DebugSummary PhysicalMassOnly 포함"), ValidViewData.DebugSummary.Contains(TEXT("PhysicalMassOnly")));

	// [v1.0.0] UI가 무효 피팅에서도 문제 원인을 잃지 않는지 검증할 Invalid Snapshot입니다.
	FCFVehicleFittingSnapshot InvalidSnapshot;
	InvalidSnapshot.FittingId = TEXT("InvalidViewFitting");
	InvalidSnapshot.ValidationState = ECFFittingValidationState::Invalid;
	// [v1.0.0] Invalid Snapshot의 실제 표시 원인이 될 Error 한 건입니다.
	FCFFittingValidationIssue InvalidIssue;
	InvalidIssue.Severity = ECFFittingIssueSeverity::Error;
	InvalidIssue.IssueCode = ECFFittingIssueCode::MissingVehicleData;
	InvalidIssue.Message = FText::FromString(TEXT("Vehicle missing"));
	InvalidSnapshot.ValidationIssues.Add(InvalidIssue);
	// [v1.0.0] Invalid Snapshot에서도 Debug 표시를 유지할 ViewData입니다.
	const FCFVehicleFittingViewData InvalidViewData = FCFFittingViewBuilder::Build(InvalidSnapshot);
	TestFalse(TEXT("Invalid Snapshot 적용 불가"), InvalidViewData.bCanApply);
	TestEqual(TEXT("Invalid Mobility Unavailable"), InvalidViewData.MobilityPreview.PreviewMode, ECFFittingMobilityPreviewMode::Unavailable);
	TestEqual(TEXT("Invalid Issue 보존"), InvalidViewData.IssueRows.Num(), 1);
	if (InvalidViewData.IssueRows.Num() == 1)
	{
		TestEqual(TEXT("Invalid Issue code 보존"), InvalidViewData.IssueRows[0].IssueCode, ECFFittingIssueCode::MissingVehicleData);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
