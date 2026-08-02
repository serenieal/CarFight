// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-034 FIT-P0-02 차량 피팅 데이터 계약 자동화 테스트
// Scope: 공용 타입 기본값, 질량 필드 호환, VehicleFittingData 계약, 중복 선택과 방어 Override 오류를 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Fitting.FIT_P0_02.DataContract 최초 추가.
// Migration:
// - 실제 장착 호환성·질량 합산 Snapshot은 FIT-P0-03 테스트로 확장한다.
// - Chaos Vehicle 질량 적용과 PhysicsAsset 비교는 FIT-P0-05 테스트 범위다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFittingTypes.h"
#include "CFEquipmentPresetData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingData.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFittingDataContractTest,
	"CarFight.Fitting.FIT_P0_02.DataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 피팅 공용 타입, 질량 기본값, 유효 계약과 대표 오류를 검증합니다.
bool FCFFittingDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 런타임 해석 전 안전한 기본 상태를 확인할 피팅 스냅샷입니다.
	const FCFVehicleFittingSnapshot DefaultSnapshot;
	TestEqual(TEXT("기본 Snapshot 상태는 NotEvaluated"), DefaultSnapshot.ValidationState, ECFFittingValidationState::NotEvaluated);
	TestFalse(TEXT("기본 Snapshot은 적용 불가"), DefaultSnapshot.IsValid());
	TestEqual(TEXT("기본 Snapshot 총중량 0"), DefaultSnapshot.TotalVehicleMassKg, 0.0f);

	// [v1.0.0] 피팅 기준 질량과 총중량 한도를 설정할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>();
	if (!TestNotNull(TEXT("VehicleData 생성"), VehicleData))
	{
		return false;
	}

	TestEqual(TEXT("기존 VehicleData 기준 질량 기본값 0"), VehicleData->BaseVehicleMassKg, 0.0f);
	TestEqual(TEXT("기존 VehicleData 최대 총중량 기본값 0"), VehicleData->MaximumGrossMassKg, 0.0f);
	VehicleData->BaseVehicleMassKg = 1500.0f;
	VehicleData->MaximumGrossMassKg = 2500.0f;

	// [v1.0.0] 피팅 장비 프리셋에 연결할 Transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>();
	if (!TestNotNull(TEXT("WeaponData 생성"), WeaponData))
	{
		return false;
	}

	TestEqual(TEXT("기존 WeaponData 질량 기본값 0"), WeaponData->GetEffectiveWeaponMassKg(), 0.0f);
	WeaponData->WeaponMassKg = 200.0f;
	TestEqual(TEXT("WeaponData 유효 질량"), WeaponData->GetEffectiveWeaponMassKg(), 200.0f);
	WeaponData->WeaponMassKg = -10.0f;
	TestEqual(TEXT("음수 WeaponData 질량은 0으로 보정"), WeaponData->GetEffectiveWeaponMassKg(), 0.0f);
	WeaponData->WeaponMassKg = 200.0f;

	// [v1.0.0] 피팅 장비 프리셋에 연결할 Transient TurretMountData입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>();
	if (!TestNotNull(TEXT("TurretMountData 생성"), TurretMountData))
	{
		return false;
	}
	TurretMountData->TurretMountWeightKg = 300.0f;

	// [v1.0.0] 완성된 장비 조합을 제공할 Transient EquipmentPresetData입니다.
	UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>();
	if (!TestNotNull(TEXT("EquipmentPresetData 생성"), EquipmentPresetData))
	{
		return false;
	}
	EquipmentPresetData->DefaultTurretMountData = TurretMountData;
	EquipmentPresetData->DefaultWeaponData = WeaponData;
	TestTrue(TEXT("테스트 장비 프리셋 완성"), EquipmentPresetData->HasCompleteEquipmentData());

	// [v1.0.0] 피팅 방어 선택과 질량 검증에 사용할 Transient VehicleDefenseData입니다.
	UCFVehicleDefenseData* DefenseData = NewObject<UCFVehicleDefenseData>();
	if (!TestNotNull(TEXT("VehicleDefenseData 생성"), DefenseData))
	{
		return false;
	}

	TestEqual(TEXT("기존 VehicleDefenseData 질량 기본값 0"), DefenseData->GetEffectiveDefenseMassKg(), 0.0f);
	DefenseData->DefenseMassKg = 250.0f;
	TestEqual(TEXT("VehicleDefenseData 유효 질량"), DefenseData->GetEffectiveDefenseMassKg(), 250.0f);

	// [v1.0.0] 정상 계약을 구성할 Transient VehicleFittingData입니다.
	UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>();
	if (!TestNotNull(TEXT("VehicleFittingData 생성"), FittingData))
	{
		return false;
	}

	// [v1.0.0] 기본 피팅 계약 오류를 수집할 배열입니다.
	TArray<FText> ValidationErrors;
	TestFalse(TEXT("VehicleData 없는 기본 피팅은 무효"), FittingData->ValidateFittingDataContract(ValidationErrors));
	TestTrue(TEXT("기본 피팅 오류 존재"), ValidationErrors.Num() > 0);

	FittingData->FittingId = TEXT("TestSUV_Default");
	FittingData->VehicleData = VehicleData;
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::UseVehicleDefault;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::Override;
	FittingData->DefenseSelection.DefenseData = DefenseData;

	// [v1.0.0] Top_01 계열 장착 프로파일에 적용할 유효 피팅 선택입니다.
	FCFVehicleMountSelection MountSelection;
	MountSelection.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
	MountSelection.EquipmentPresetData = EquipmentPresetData;
	MountSelection.bEnabled = true;
	FittingData->MountSelections.Add(MountSelection);

	TestTrue(TEXT("완성된 피팅 데이터 계약 유효"), FittingData->ValidateFittingDataContract(ValidationErrors));
	TestEqual(TEXT("완성된 피팅 계약 오류 없음"), ValidationErrors.Num(), 0);
	TestTrue(TEXT("피팅 요약에 ID 포함"), FittingData->BuildVehicleFittingSummary().Contains(TEXT("TestSUV_Default")));

	// [v1.0.0] 공용 Snapshot 구조의 질량 항목이 결정론적으로 값을 보존하는지 확인할 복사본입니다.
	FCFVehicleFittingSnapshot Snapshot;
	Snapshot.FittingId = FittingData->FittingId;
	Snapshot.VehicleData = VehicleData;
	Snapshot.ValidationState = ECFFittingValidationState::Valid;
	Snapshot.BaseVehicleMassKg = VehicleData->BaseVehicleMassKg;
	Snapshot.EquipmentMassKg = TurretMountData->TurretMountWeightKg + WeaponData->GetEffectiveWeaponMassKg();
	Snapshot.DefenseMassKg = DefenseData->GetEffectiveDefenseMassKg();
	Snapshot.PayloadMassKg = Snapshot.EquipmentMassKg + Snapshot.AmmoMassKg + Snapshot.DefenseMassKg;
	Snapshot.TotalVehicleMassKg = Snapshot.BaseVehicleMassKg + Snapshot.PayloadMassKg;
	Snapshot.MaximumGrossMassKg = VehicleData->MaximumGrossMassKg;
	TestTrue(TEXT("유효 Snapshot은 적용 가능"), Snapshot.IsValid());
	TestEqual(TEXT("장비 질량 합산"), Snapshot.EquipmentMassKg, 500.0f);
	TestEqual(TEXT("탑재 질량 합산"), Snapshot.PayloadMassKg, 750.0f);
	TestEqual(TEXT("차량 총중량 합산"), Snapshot.TotalVehicleMassKg, 2250.0f);

	// [v1.0.0] 중복 MountProfileId 계약 오류를 확인할 두 번째 선택입니다.
	FittingData->MountSelections.Add(MountSelection);
	TestFalse(TEXT("중복 MountProfileId 피팅은 무효"), FittingData->ValidateFittingDataContract(ValidationErrors));
	TestTrue(TEXT("중복 선택 오류 존재"), ValidationErrors.Num() > 0);
	FittingData->MountSelections.SetNum(1);

	FittingData->DefenseSelection.DefenseData = nullptr;
	TestFalse(TEXT("Defense Override 데이터 누락은 무효"), FittingData->ValidateFittingDataContract(ValidationErrors));
	TestTrue(TEXT("방어 Override 오류 존재"), ValidationErrors.Num() > 0);

	DefenseData->DefenseMassKg = -1.0f;
	TestEqual(TEXT("음수 방어 질량은 0으로 보정"), DefenseData->GetEffectiveDefenseMassKg(), 0.0f);
	TestFalse(TEXT("음수 방어 질량 계약 거부"), DefenseData->ValidateDefenseDataContract(ValidationErrors));
	TestTrue(TEXT("음수 방어 질량 오류 존재"), ValidationErrors.Num() > 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
