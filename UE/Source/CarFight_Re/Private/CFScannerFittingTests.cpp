// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-16
// Description: CF-FQ-037 Scanner 장비/Fitting 정적 계약과 SCAN-P0-04 Runtime 통합 자동화
// Scope: Utility Scanner Snapshot, ResolvedSensorData→Sensor Runtime, scanner-less Fallback과 Field Fitting Commit/Compensation 원자성을 검증합니다.
// Changelog:
// - v1.2.0: SCAN-P0-04 FittingIntegration 테스트를 추가해 Scanner-first Snapshot의 Weapon 선택 분리, 초기 Source 적용, Field hot reapply/scanner-less/compensation/실패 복원을 검증.
// - v1.1.0: DataValidation 단계에서도 Scanner preset을 Utility가 아닌 MountType에 연결하면 거부되는 회귀를 추가.
// - v1.0.0: CarFight.Scanner.SCAN_P0_01.DataContract 최초 추가.
// Migration:
// - 테스트는 Transient UObject만 사용하며 Content Asset, Blueprint, InputAction과 Sensor Runtime을 생성하거나 저장하지 않습니다.
// - P0-01 SensorData는 FCFSensorConfig 기본 0-range Foundation 값을 그대로 사용하며 게임플레이 튜닝값을 새로 확정하지 않습니다.
// - P0-04 테스트의 서로 다른 range 값은 Source 교체·복원 식별만 위한 Transient fixture이며 Content Asset이나 밸런스 수치로 저장하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFEquipmentPresetData.h"
#include "CFFieldFitCoordinator.h"
#include "CFFittingTypes.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehicleSensorComp.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

namespace
{
	// [v1.0.0] Snapshot에 지정한 검증 문제 코드가 존재하는지 반환합니다.
	bool HasScannerFittingIssue(
		const FCFVehicleFittingSnapshot& Snapshot,
		const ECFFittingIssueCode IssueCode,
		const FName MountProfileId = NAME_None)
	{
		for (const FCFFittingValidationIssue& ValidationIssue : Snapshot.ValidationIssues)
		{
			if (ValidationIssue.IssueCode == IssueCode
				&& (MountProfileId.IsNone() || ValidationIssue.MountProfileId == MountProfileId))
			{
				return true;
			}
		}

		return false;
	}

	// [v1.0.0] 테스트 VehicleData에 하드포인트 위치 슬롯을 추가합니다.
	void AddScannerTestHardpoint(UCFVehicleData* VehicleData, const FName LocationSlotId)
	{
		// [v1.0.0] VehicleData에 추가할 테스트 하드포인트 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = LocationSlotId;
		HardpointSlot.LocationCategory = TEXT("ScannerTest");
		VehicleData->HardpointSlots.Add(HardpointSlot);
	}

	// [v1.0.0] 테스트 VehicleData에 장착 프로파일을 추가합니다.
	void AddScannerTestMountProfile(
		UCFVehicleData* VehicleData,
		const FName MountProfileId,
		const FName LocationSlotId,
		const ECFVehicleMountType MountType,
		const ECFVehicleWeaponSize SizeLimit,
		UCFEquipmentPresetData* DefaultEquipmentPresetData)
	{
		// [v1.0.0] VehicleData에 추가할 테스트 장착 프로파일입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = MountProfileId;
		MountProfile.LocationSlotRef = LocationSlotId;
		MountProfile.MountType = MountType;
		MountProfile.SizeLimit = SizeLimit;
		MountProfile.DefaultEquipmentPresetData = DefaultEquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);
	}

	// [v1.0.0] 테스트 FittingData에 명시적 장비 선택을 추가합니다.
	void AddScannerTestSelection(
		UCFVehicleFittingData* FittingData,
		const FName MountProfileId,
		UCFEquipmentPresetData* EquipmentPresetData,
		const bool bEnabled = true)
	{
		// [v1.0.0] VehicleFittingData에 추가할 테스트 장착 선택입니다.
		FCFVehicleMountSelection MountSelection;
		MountSelection.MountProfileId = MountProfileId;
		MountSelection.EquipmentPresetData = EquipmentPresetData;
		MountSelection.bEnabled = bEnabled;
		FittingData->MountSelections.Add(MountSelection);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFScannerFittingDataContractTest,
	"CarFight.Scanner.SCAN_P0_01.DataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Utility Scanner payload, 기존 무장 비회귀, scanner-less와 복수 Scanner Snapshot 계약을 검증합니다.
bool FCFScannerFittingDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Scanner 장비가 재사용할 기본 0-range Foundation SensorData입니다.
	UCFVehicleSensorData* SensorData = NewObject<UCFVehicleSensorData>();
	if (!TestNotNull(TEXT("기본 SensorData 생성"), SensorData))
	{
		return false;
	}
	TestTrue(TEXT("기본 0-range SensorConfig 유효"), SensorData->IsSensorConfigValid());

	// [v1.0.0] Utility Scanner를 표현할 EquipmentPresetData입니다.
	UCFEquipmentPresetData* ScannerPreset = NewObject<UCFEquipmentPresetData>();
	if (!TestNotNull(TEXT("Scanner EquipmentPreset 생성"), ScannerPreset))
	{
		return false;
	}
	ScannerPreset->EquipmentId = TEXT("ScannerPrimary");
	ScannerPreset->RequiredMountType = ECFVehicleMountType::Utility;
	ScannerPreset->RequiredWeaponSize = ECFVehicleWeaponSize::None;
	ScannerPreset->DefaultSensorData = SensorData;

	TestTrue(TEXT("Scanner preset 단일 payload 완성"), ScannerPreset->HasCompleteEquipmentData());
	TestTrue(TEXT("Utility 기준 Scanner preset 완성"), ScannerPreset->HasCompleteEquipmentDataForMount(ECFVehicleMountType::Utility));
	TestFalse(TEXT("Turret 기준 Scanner preset 거부"), ScannerPreset->HasCompleteEquipmentDataForMount(ECFVehicleMountType::Turret));
	TestTrue(TEXT("Utility Scanner 장착 가능"), ScannerPreset->CanUseOnMount(ECFVehicleMountType::Utility, ECFVehicleWeaponSize::None));

	// [v1.0.0] 기존 무장 프리셋 비회귀를 확인할 TurretMountData입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>();
	if (!TestNotNull(TEXT("TurretMountData 생성"), TurretMountData))
	{
		return false;
	}
	TurretMountData->TurretMountWeightKg = 100.0f;

	// [v1.0.0] 기존 무장 프리셋 비회귀를 확인할 WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>();
	if (!TestNotNull(TEXT("WeaponData 생성"), WeaponData))
	{
		return false;
	}
	WeaponData->WeaponSize = ECFVehicleWeaponSize::Medium;
	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	WeaponData->WeaponMassKg = 100.0f;

	// [v1.0.0] 기존 Turret+Weapon 패키지 형태를 검증할 EquipmentPresetData입니다.
	UCFEquipmentPresetData* WeaponPreset = NewObject<UCFEquipmentPresetData>();
	if (!TestNotNull(TEXT("Weapon EquipmentPreset 생성"), WeaponPreset))
	{
		return false;
	}
	WeaponPreset->EquipmentId = TEXT("WeaponPrimary");
	WeaponPreset->RequiredMountType = ECFVehicleMountType::Turret;
	WeaponPreset->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;
	WeaponPreset->DefaultTurretMountData = TurretMountData;
	WeaponPreset->DefaultWeaponData = WeaponData;

	TestTrue(TEXT("기존 무장 preset 단일 payload 완성"), WeaponPreset->HasCompleteEquipmentData());
	TestTrue(TEXT("Turret 기준 기존 무장 preset 완성"), WeaponPreset->HasCompleteEquipmentDataForMount(ECFVehicleMountType::Turret));
	TestTrue(TEXT("기존 Turret 무장 장착 가능"), WeaponPreset->CanUseOnMount(ECFVehicleMountType::Turret, ECFVehicleWeaponSize::Medium));

	// [v1.0.0] 무장과 Scanner MountProfile을 함께 소유할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>();
	if (!TestNotNull(TEXT("Scanner Fitting VehicleData 생성"), VehicleData))
	{
		return false;
	}
	VehicleData->BaseVehicleMassKg = 1000.0f;
	VehicleData->MaximumGrossMassKg = 3000.0f;
	AddScannerTestHardpoint(VehicleData, TEXT("Top_01"));
	AddScannerTestHardpoint(VehicleData, TEXT("Utility_01"));
	AddScannerTestMountProfile(
		VehicleData,
		TEXT("RoofTurret"),
		TEXT("Top_01"),
		ECFVehicleMountType::Turret,
		ECFVehicleWeaponSize::Medium,
		WeaponPreset);
	AddScannerTestMountProfile(
		VehicleData,
		TEXT("ScannerPrimary"),
		TEXT("Utility_01"),
		ECFVehicleMountType::Utility,
		ECFVehicleWeaponSize::None,
		ScannerPreset);

	// [v1.0.0] 명시적 무장과 Scanner를 선택할 Transient FittingData입니다.
	UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>();
	if (!TestNotNull(TEXT("Scanner FittingData 생성"), FittingData))
	{
		return false;
	}
	FittingData->FittingId = TEXT("ScannerDataContract");
	FittingData->VehicleData = VehicleData;
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::ExplicitNone;
	AddScannerTestSelection(FittingData, TEXT("RoofTurret"), WeaponPreset);
	AddScannerTestSelection(FittingData, TEXT("ScannerPrimary"), ScannerPreset);

	// [v1.0.0] DataValidation 수준의 오류를 수집할 배열입니다.
	TArray<FText> ValidationErrors;
		TestTrue(TEXT("Scanner 포함 FittingData 계약 유효"), FittingData->ValidateFittingDataContract(ValidationErrors));
	TestEqual(TEXT("Scanner 포함 FittingData 계약 오류 없음"), ValidationErrors.Num(), 0);

	VehicleData->MountProfiles[1].MountType = ECFVehicleMountType::Turret;
	TestFalse(TEXT("Scanner preset은 Utility가 아닌 MountType에서 DataValidation 거부"), FittingData->ValidateFittingDataContract(ValidationErrors));
	VehicleData->MountProfiles[1].MountType = ECFVehicleMountType::Utility;
	TestTrue(TEXT("Utility MountType 복원 후 DataValidation 유효"), FittingData->ValidateFittingDataContract(ValidationErrors));

	// [v1.0.0] 단 하나의 Scanner Source를 정상 해석한 기준 Snapshot입니다.
	const FCFVehicleFittingSnapshot ValidSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("단일 Scanner Snapshot 유효"), ValidSnapshot.IsValid());
	TestEqual(TEXT("해석된 Mount 2개"), ValidSnapshot.ResolvedMounts.Num(), 2);
	TestTrue(TEXT("Scanner mount SensorData 보존"), ValidSnapshot.ResolvedMounts[1].SensorData == SensorData);
	TestTrue(TEXT("Snapshot 단일 ResolvedSensorData 보존"), ValidSnapshot.ResolvedSensorData == SensorData);
	TestEqual(TEXT("Scanner는 미정 질량을 임의 추가하지 않음"), ValidSnapshot.EquipmentMassKg, 200.0f);

	FittingData->MountSelections[1].bEnabled = false;

	// [v1.0.0] Scanner 장착을 명시적으로 비운 scanner-less 호환 Snapshot입니다.
	const FCFVehicleFittingSnapshot ScannerlessSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("scanner-less Snapshot 유효"), ScannerlessSnapshot.IsValid());
	TestNull(TEXT("scanner-less ResolvedSensorData 없음"), ScannerlessSnapshot.ResolvedSensorData.Get());
	FittingData->MountSelections[1].bEnabled = true;

	// [v1.0.0] SensorConfig 검증 실패 경계를 확인할 별도 Transient SensorData입니다.
	UCFVehicleSensorData* InvalidSensorData = NewObject<UCFVehicleSensorData>();
	if (!TestNotNull(TEXT("무효 SensorData 생성"), InvalidSensorData))
	{
		return false;
	}
	InvalidSensorData->SensorConfig.DetailedScanThreshold = InvalidSensorData->SensorConfig.IdentifiedThreshold;
	ScannerPreset->DefaultSensorData = InvalidSensorData;
	TestFalse(TEXT("무효 SensorData는 Utility 장착 거부"), ScannerPreset->CanUseOnMount(ECFVehicleMountType::Utility, ECFVehicleWeaponSize::None));
	TestFalse(TEXT("무효 SensorData FittingData 계약 거부"), FittingData->ValidateFittingDataContract(ValidationErrors));

	// [v1.0.0] SensorConfig가 무효인 Scanner를 해석한 Snapshot입니다.
	const FCFVehicleFittingSnapshot InvalidSensorSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("무효 Sensor Snapshot 거부"), InvalidSensorSnapshot.IsValid());
	TestTrue(TEXT("InvalidSensorData 문제 보고"), HasScannerFittingIssue(InvalidSensorSnapshot, ECFFittingIssueCode::InvalidSensorData, TEXT("ScannerPrimary")));
	ScannerPreset->DefaultSensorData = SensorData;

	ScannerPreset->DefaultWeaponData = WeaponData;

	// [v1.0.0] 한 프리셋에 Scanner와 Weapon payload를 섞은 모호한 Snapshot입니다.
	const FCFVehicleFittingSnapshot MixedPayloadSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("혼합 Scanner/Weapon payload Snapshot 거부"), MixedPayloadSnapshot.IsValid());
	TestTrue(TEXT("혼합 payload IncompleteEquipmentPreset 보고"), HasScannerFittingIssue(MixedPayloadSnapshot, ECFFittingIssueCode::IncompleteEquipmentPreset, TEXT("ScannerPrimary")));
	ScannerPreset->DefaultWeaponData = nullptr;

	// [v1.0.0] 두 번째 Scanner Source에서 사용할 기본 0-range Foundation SensorData입니다.
	UCFVehicleSensorData* SecondarySensorData = NewObject<UCFVehicleSensorData>();
	if (!TestNotNull(TEXT("두 번째 SensorData 생성"), SecondarySensorData))
	{
		return false;
	}

	// [v1.0.0] 복수 Scanner Source 거부를 검증할 두 번째 Scanner 프리셋입니다.
	UCFEquipmentPresetData* SecondaryScannerPreset = NewObject<UCFEquipmentPresetData>();
	if (!TestNotNull(TEXT("두 번째 Scanner preset 생성"), SecondaryScannerPreset))
	{
		return false;
	}
	SecondaryScannerPreset->EquipmentId = TEXT("ScannerSecondary");
	SecondaryScannerPreset->RequiredMountType = ECFVehicleMountType::Utility;
	SecondaryScannerPreset->RequiredWeaponSize = ECFVehicleWeaponSize::None;
	SecondaryScannerPreset->DefaultSensorData = SecondarySensorData;

	AddScannerTestHardpoint(VehicleData, TEXT("Utility_02"));
	AddScannerTestMountProfile(
		VehicleData,
		TEXT("ScannerSecondary"),
		TEXT("Utility_02"),
		ECFVehicleMountType::Utility,
		ECFVehicleWeaponSize::None,
		SecondaryScannerPreset);
	AddScannerTestSelection(FittingData, TEXT("ScannerSecondary"), SecondaryScannerPreset);

	// [v1.0.0] 두 Scanner 중 하나를 암묵적으로 선택하지 않는 복수 Source Snapshot입니다.
	const FCFVehicleFittingSnapshot MultipleScannerSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("복수 Scanner Snapshot 거부"), MultipleScannerSnapshot.IsValid());
	TestTrue(TEXT("MultipleSensorSources 문제 보고"), HasScannerFittingIssue(MultipleScannerSnapshot, ECFFittingIssueCode::MultipleSensorSources, TEXT("ScannerSecondary")));
	TestNull(TEXT("복수 Scanner에서는 top-level ResolvedSensorData 비움"), MultipleScannerSnapshot.ResolvedSensorData.Get());
	TestTrue(TEXT("첫 Scanner mount payload 자체는 보존"), MultipleScannerSnapshot.ResolvedMounts[1].SensorData == SensorData);
	TestTrue(TEXT("두 번째 Scanner mount payload 자체는 보존"), MultipleScannerSnapshot.ResolvedMounts[2].SensorData == SecondarySensorData);

		return true;
}

namespace
{
	/** P0-04에서 Fitting Runtime Sensor participant를 실제 VehicleSensorComp에 연결하는 Transient 테스트 Adapter입니다. */
	class FCFScannerFittingRuntimeAdapter final : public ICFFittingRuntimeApplyAdapter
	{
	public:
		// [v1.2.0] 실제 ApplySensorData를 호출할 Transient Sensor Component를 연결합니다.
		explicit FCFScannerFittingRuntimeAdapter(UCFVehicleSensorComp* InVehicleSensorComp)
			: VehicleSensorComp(InVehicleSensorComp)
		{
		}

		// [v1.2.0] Snapshot의 Weapon 선택 결과를 기록하고 기존 Weapon participant 성공을 재현합니다.
		virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) override
		{
			LastWeaponInput = WeaponInput;
			++WeaponApplyCallCount;
			bOutWeaponRuntimeReady = WeaponInput.UsesLegacyVehicleConfiguration() || WeaponInput.bHasResolvedMount;
			return true;
		}

		// [v1.2.0] Sensor 통합과 독립적인 Defense participant 성공을 재현합니다.
		virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) override
		{
			LastDefenseInput = DefenseInput;
			++DefenseApplyCallCount;
			bOutDefenseRuntimeReady = DefenseInput.DefenseData != nullptr;
			return true;
		}

		// [v1.2.0] Snapshot Sensor 입력을 실제 P0-02 non-destructive ApplySensorData API로 전달합니다.
		virtual bool ApplySensorRuntime(const FCFFittingSensorRuntimeInput& SensorInput) override
		{
			LastSensorInput = SensorInput;
			++SensorApplyCallCount;
			if (SensorInput.UsesLegacyVehicleConfiguration())
			{
				return true;
			}

			return VehicleSensorComp
				&& VehicleSensorComp->ApplySensorData(SensorInput.SensorData);
		}

		// [v1.2.0] 마지막으로 적용한 Weapon Runtime 입력입니다.
		FCFFittingWeaponRuntimeInput LastWeaponInput;

		// [v1.2.0] 마지막으로 적용한 Defense Runtime 입력입니다.
		FCFFittingDefenseRuntimeInput LastDefenseInput;

		// [v1.2.0] 마지막으로 적용한 Sensor Runtime 입력입니다.
		FCFFittingSensorRuntimeInput LastSensorInput;

		// [v1.2.0] Weapon participant 적용 호출 횟수입니다.
		int32 WeaponApplyCallCount = 0;

		// [v1.2.0] Defense participant 적용 호출 횟수입니다.
		int32 DefenseApplyCallCount = 0;

		// [v1.2.0] Sensor participant 적용 호출 횟수입니다.
		int32 SensorApplyCallCount = 0;

	private:
		// [v1.2.0] 실제 SensorData Source와 Applied Config를 소유하는 Transient Sensor Component입니다.
		TObjectPtr<UCFVehicleSensorComp> VehicleSensorComp = nullptr;
	};

	// [v1.2.0] P0-04 테스트에서 Source 교체를 구분할 유효 Transient SensorData를 생성합니다.
	UCFVehicleSensorData* CreateScannerRuntimeSensorData(UObject* Outer, const float PassiveRangeCm)
	{
		// [v1.2.0] Content Asset 저장 없이 Runtime Source로만 사용할 SensorData입니다.
		UCFVehicleSensorData* SensorData = NewObject<UCFVehicleSensorData>(Outer);
		SensorData->SensorConfig.PassiveDetectionRangeCm = PassiveRangeCm;
		return SensorData;
	}

	// [v1.2.0] 지정 SensorData를 운반할 Utility Scanner EquipmentPreset을 생성합니다.
	UCFEquipmentPresetData* CreateScannerRuntimePreset(UObject* Outer, const FName EquipmentId, UCFVehicleSensorData* SensorData)
	{
		// [v1.2.0] Utility mount에서 SensorData만 운반할 Scanner 프리셋입니다.
		UCFEquipmentPresetData* ScannerPreset = NewObject<UCFEquipmentPresetData>(Outer);
		ScannerPreset->EquipmentId = EquipmentId;
		ScannerPreset->RequiredMountType = ECFVehicleMountType::Utility;
		ScannerPreset->RequiredWeaponSize = ECFVehicleWeaponSize::None;
		ScannerPreset->DefaultSensorData = SensorData;
		return ScannerPreset;
	}

	// [v1.2.0] Scanner-first 순서와 실제 Weapon mount를 함께 가진 P0-04 테스트 VehicleData를 생성합니다.
	UCFVehicleData* CreateScannerRuntimeVehicleData(UObject* Outer, UCFEquipmentPresetData* ScannerPreset, UCFEquipmentPresetData* WeaponPreset)
	{
		// [v1.2.0] Scanner mount를 Weapon보다 먼저 배치해 잘못된 first-mount Weapon 선택 회귀를 검증할 VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);
		VehicleData->BaseVehicleMassKg = 1000.0f;
		VehicleData->MaximumGrossMassKg = 3000.0f;
		AddScannerTestHardpoint(VehicleData, TEXT("Utility_01"));
		AddScannerTestHardpoint(VehicleData, TEXT("Top_01"));
		AddScannerTestMountProfile(VehicleData, TEXT("ScannerPrimary"), TEXT("Utility_01"), ECFVehicleMountType::Utility, ECFVehicleWeaponSize::None, ScannerPreset);
		AddScannerTestMountProfile(VehicleData, TEXT("RoofTurret"), TEXT("Top_01"), ECFVehicleMountType::Turret, ECFVehicleWeaponSize::Medium, WeaponPreset);
		return VehicleData;
	}

	// [v1.2.0] 동일 VehicleData에서 Scanner 선택만 교체하거나 비활성화한 유효 Snapshot을 생성합니다.
	FCFVehicleFittingSnapshot BuildScannerRuntimeSnapshot(
		UObject* Outer,
		UCFVehicleData* VehicleData,
		const FName FittingId,
		UCFEquipmentPresetData* ScannerPreset,
		UCFEquipmentPresetData* WeaponPreset,
		const bool bScannerEnabled)
	{
		// [v1.2.0] 기존 BuildFittingSnapshot 검증 경로를 그대로 사용할 Transient FittingData입니다.
		UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>(Outer);
		FittingData->FittingId = FittingId;
		FittingData->VehicleData = VehicleData;
		FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
		FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::ExplicitNone;
		AddScannerTestSelection(FittingData, TEXT("ScannerPrimary"), ScannerPreset, bScannerEnabled);
		AddScannerTestSelection(FittingData, TEXT("RoofTurret"), WeaponPreset, true);
		return FittingData->BuildFittingSnapshot();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFScannerFittingIntegrationTest,
	"CarFight.Scanner.SCAN_P0_04.FittingIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.2.0] ResolvedSensorData의 초기 적용, Field hot reapply, scanner-less 적용과 compensation 원자성을 검증합니다.
bool FCFScannerFittingIntegrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.2.0] 모든 Transient fixture의 Outer로 사용할 패키지입니다.
	UPackage* TestOuter = GetTransientPackage();

	// [v1.2.0] 초기 출격에서 적용할 첫 Scanner Source입니다.
	UCFVehicleSensorData* SensorDataA = CreateScannerRuntimeSensorData(TestOuter, 1000.0f);
	// [v1.2.0] Field Fitting으로 교체할 두 번째 Scanner Source입니다.
	UCFVehicleSensorData* SensorDataB = CreateScannerRuntimeSensorData(TestOuter, 2000.0f);
	TestTrue(TEXT("SensorData A Config 유효"), SensorDataA && SensorDataA->IsSensorConfigValid());
	TestTrue(TEXT("SensorData B Config 유효"), SensorDataB && SensorDataB->IsSensorConfigValid());

	// [v1.2.0] 첫 Scanner Source를 운반할 Utility 프리셋입니다.
	UCFEquipmentPresetData* ScannerPresetA = CreateScannerRuntimePreset(TestOuter, TEXT("ScannerRuntimeA"), SensorDataA);
	// [v1.2.0] 두 번째 Scanner Source를 운반할 Utility 프리셋입니다.
	UCFEquipmentPresetData* ScannerPresetB = CreateScannerRuntimePreset(TestOuter, TEXT("ScannerRuntimeB"), SensorDataB);

	// [v1.2.0] Scanner와 같은 Snapshot에 존재할 기존 TurretMountData입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(TestOuter);
	TurretMountData->TurretMountWeightKg = 100.0f;

	// [v1.2.0] Scanner-first Snapshot에서 Weapon participant가 정확히 선택해야 할 WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(TestOuter);
	WeaponData->WeaponSize = ECFVehicleWeaponSize::Medium;
	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	WeaponData->WeaponMassKg = 100.0f;

	// [v1.2.0] 기존 Turret+Weapon Runtime 입력을 제공할 무장 프리셋입니다.
	UCFEquipmentPresetData* WeaponPreset = NewObject<UCFEquipmentPresetData>(TestOuter);
	WeaponPreset->EquipmentId = TEXT("ScannerRuntimeWeapon");
	WeaponPreset->RequiredMountType = ECFVehicleMountType::Turret;
	WeaponPreset->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;
	WeaponPreset->DefaultTurretMountData = TurretMountData;
	WeaponPreset->DefaultWeaponData = WeaponData;

	// [v1.2.0] Scanner mount를 ResolvedMounts 첫 위치에 배치한 공통 VehicleData입니다.
	UCFVehicleData* VehicleData = CreateScannerRuntimeVehicleData(TestOuter, ScannerPresetA, WeaponPreset);

	// [v1.2.0] 초기 Scanner A를 선택한 기준 Snapshot입니다.
	const FCFVehicleFittingSnapshot SnapshotA = BuildScannerRuntimeSnapshot(TestOuter, VehicleData, TEXT("ScannerRuntimeA"), ScannerPresetA, WeaponPreset, true);
	// [v1.2.0] 같은 질량에서 Scanner만 B로 교체한 Field Fitting 후보 Snapshot입니다.
	const FCFVehicleFittingSnapshot SnapshotB = BuildScannerRuntimeSnapshot(TestOuter, VehicleData, TEXT("ScannerRuntimeB"), ScannerPresetB, WeaponPreset, true);
	// [v1.2.0] Scanner mount를 명시적으로 비운 scanner-less Field Fitting 후보 Snapshot입니다.
	const FCFVehicleFittingSnapshot ScannerlessSnapshot = BuildScannerRuntimeSnapshot(TestOuter, VehicleData, TEXT("ScannerRuntimeNone"), ScannerPresetA, WeaponPreset, false);
	TestTrue(TEXT("초기 Scanner Snapshot 유효"), SnapshotA.IsValid());
	TestTrue(TEXT("교체 Scanner Snapshot 유효"), SnapshotB.IsValid());
	TestTrue(TEXT("scanner-less Snapshot 유효"), ScannerlessSnapshot.IsValid());
	TestTrue(TEXT("초기 ResolvedSensorData A"), SnapshotA.ResolvedSensorData == SensorDataA);
	TestTrue(TEXT("교체 ResolvedSensorData B"), SnapshotB.ResolvedSensorData == SensorDataB);
	TestNull(TEXT("scanner-less ResolvedSensorData null"), ScannerlessSnapshot.ResolvedSensorData.Get());

	// [v1.2.0] 실제 P0-02 ApplySensorData 상태를 확인할 Transient Sensor Runtime입니다.
	UCFVehicleSensorComp* VehicleSensorComp = NewObject<UCFVehicleSensorComp>(TestOuter);
	// [v1.2.0] Fitting Runtime의 Weapon·Defense·Sensor participant를 기록·적용할 Adapter입니다.
	FCFScannerFittingRuntimeAdapter RuntimeApplyAdapter(VehicleSensorComp);
	// [v1.2.0] Prepared/Applied Snapshot과 compensation checkpoint를 소유할 Fitting Component입니다.
	UCFVehicleFittingComp* VehicleFittingComp = NewObject<UCFVehicleFittingComp>(TestOuter);

	TestTrue(TEXT("초기 Snapshot Runtime Prepare"), VehicleFittingComp->PrepareSortieFittingSnapshot(SnapshotA, NAME_None));
	TestTrue(TEXT("초기 Snapshot Runtime Commit"), VehicleFittingComp->CommitPreparedSortieFitting(RuntimeApplyAdapter));
	TestTrue(TEXT("Scanner-first에서도 Weapon preset 정확히 선택"), RuntimeApplyAdapter.LastWeaponInput.EquipmentPresetData == WeaponPreset);
	TestTrue(TEXT("초기 Fitting이 SensorData A Source 선택"), VehicleSensorComp->SensorData == SensorDataA);
	TestFalse(TEXT("초기 Fitting Source 선택이 Sensor Runtime 암묵 초기화 안 함"), VehicleSensorComp->IsSensorRuntimeReady());
	TestTrue(TEXT("선택된 A Source로 Sensor Runtime 명시 초기화"), VehicleSensorComp->InitializeSensorRuntime());
	TestTrue(TEXT("초기 적용 Config가 SensorData A"), FMath::IsNearlyEqual(VehicleSensorComp->GetResolvedSensorConfig().PassiveDetectionRangeCm, 1000.0f));

	// [v1.2.0] 같은 질량 Scanner B 후보를 기존 Field transaction/Checkpoint 경계에 연결합니다.
	FCFFieldFitRuntimeAdapter ScannerBTransaction(VehicleFittingComp, RuntimeApplyAdapter);
	// [v1.2.0] Field transaction 실패 원인을 받을 진단 문자열입니다.
	FString FieldFailureSummary;
	TestTrue(TEXT("Scanner B Field Runtime Prepare"), ScannerBTransaction.PrepareRuntime(SnapshotB, NAME_None, FieldFailureSummary));
	TestTrue(TEXT("Scanner B Field Runtime Commit"), ScannerBTransaction.CommitRuntime(FieldFailureSummary));
	TestTrue(TEXT("Field hot reapply가 SensorData B 적용"), VehicleSensorComp->SensorData == SensorDataB);
	TestTrue(TEXT("Field hot reapply 뒤 Runtime Ready 보존"), VehicleSensorComp->IsSensorRuntimeReady());
	TestTrue(TEXT("Field hot reapply Config B"), FMath::IsNearlyEqual(VehicleSensorComp->GetResolvedSensorConfig().PassiveDetectionRangeCm, 2000.0f));
	TestTrue(TEXT("Inventory 실패 상당 compensation에서 Scanner A 복원"), ScannerBTransaction.CompensateCommittedRuntime(FieldFailureSummary));
	TestTrue(TEXT("Compensation 뒤 SensorData A 복원"), VehicleSensorComp->SensorData == SensorDataA);
	TestTrue(TEXT("Compensation 뒤 Applied Snapshot A 복원"), VehicleFittingComp->GetAppliedFittingSnapshot().ResolvedSensorData == SensorDataA);

	// [v1.2.0] Scanner 제거를 null/Fallback으로 적용할 같은 질량 Field transaction입니다.
	FCFFieldFitRuntimeAdapter ScannerlessTransaction(VehicleFittingComp, RuntimeApplyAdapter);
	TestTrue(TEXT("scanner-less Field Runtime Prepare"), ScannerlessTransaction.PrepareRuntime(ScannerlessSnapshot, NAME_None, FieldFailureSummary));
	TestTrue(TEXT("scanner-less Field Runtime Commit"), ScannerlessTransaction.CommitRuntime(FieldFailureSummary));
	TestNull(TEXT("scanner-less Commit이 SensorData Source를 null로 전환"), VehicleSensorComp->SensorData.Get());
	TestTrue(TEXT("scanner-less Commit 뒤 Runtime Ready 보존"), VehicleSensorComp->IsSensorRuntimeReady());
	TestTrue(TEXT("scanner-less Compensation에서 Scanner A 복원"), ScannerlessTransaction.CompensateCommittedRuntime(FieldFailureSummary));
	TestTrue(TEXT("scanner-less Compensation 뒤 SensorData A"), VehicleSensorComp->SensorData == SensorDataA);

	// [v1.2.0] null/Fallback 적용 자체가 실패하는 경우 Sensor participant의 원자 복원을 검증할 무효 Fallback 설정입니다.
	VehicleSensorComp->FallbackSensorConfig.DetailedScanThreshold = VehicleSensorComp->FallbackSensorConfig.IdentifiedThreshold;
	TestFalse(TEXT("실패 주입용 Fallback Config 무효"), VehicleSensorComp->FallbackSensorConfig.IsValid());

	// [v1.2.0] Sensor participant 실패 시 Weapon·Defense·Sensor 전체 이전 상태 복원을 확인할 transaction입니다.
	FCFFieldFitRuntimeAdapter FailingSensorTransaction(VehicleFittingComp, RuntimeApplyAdapter);
	TestTrue(TEXT("Sensor 실패 후보 Prepare"), FailingSensorTransaction.PrepareRuntime(ScannerlessSnapshot, NAME_None, FieldFailureSummary));
	TestFalse(TEXT("Sensor participant 실패로 Commit 거부"), FailingSensorTransaction.CommitRuntime(FieldFailureSummary));
	TestTrue(TEXT("Sensor Commit 실패가 이전 Runtime 복구"), FailingSensorTransaction.WasLastCommitFailureRecovered());
	TestTrue(TEXT("Sensor 실패 뒤 SensorData A 보존"), VehicleSensorComp->SensorData == SensorDataA);
	TestTrue(TEXT("Sensor 실패 뒤 Applied Snapshot A 보존"), VehicleFittingComp->GetAppliedFittingSnapshot().ResolvedSensorData == SensorDataA);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
