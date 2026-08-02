// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-034 FIT-P0-03 차량 피팅 호환 검증과 질량 Snapshot 자동화 테스트
// Scope: MountProfile·Hardpoint 해석, 선택 정책, 장비·무기 호환, 방어 3상태, 결정론적 순서와 질량·총중량 검증을 수행합니다.
// Changelog:
// - v1.0.0: CarFight.Fitting.FIT_P0_03.Compatibility와 MassSnapshot 테스트를 최초 추가.
// Migration:
// - 테스트는 Transient UObject만 사용하며 Pawn, VehicleMovement, Blueprint, DataAsset, PhysicsAsset과 Map을 생성하거나 수정하지 않는다.
// - AmmoMassKg는 CF-FQ-031 연동 전까지 0으로 검증한다.
// - Chaos Vehicle 질량 적용은 FIT-P0-05 테스트 범위다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFEquipmentPresetData.h"
#include "CFFittingTypes.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingData.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

namespace
{
	/**
	 * 한 테스트 EquipmentPreset을 구성하는 세 Transient DataAsset 포인터입니다.
	 */
	struct FCFFittingTestEquipment
	{
		// [v1.0.0] 테스트 장착 프리셋 DataAsset입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] 테스트 터렛 마운트 DataAsset입니다.
		UCFTurretMountData* TurretMountData = nullptr;

		// [v1.0.0] 테스트 무기 DataAsset입니다.
		UCFWeaponData* WeaponData = nullptr;
	};

	// [v1.0.0] 지정 타입·크기·질량을 가진 완성된 Transient 장비 프리셋을 생성합니다.
	FCFFittingTestEquipment CreateFittingTestEquipment(
		UObject* Outer,
		const FName EquipmentId,
		const ECFVehicleMountType MountType,
		const ECFVehicleWeaponSize WeaponSize,
		const float TurretMountMassKg,
		const float WeaponMassKg)
	{
		// [v1.0.0] 호출자에게 반환할 테스트 장비 묶음입니다.
		FCFFittingTestEquipment TestEquipment;

		TestEquipment.TurretMountData = NewObject<UCFTurretMountData>(Outer);
		TestEquipment.TurretMountData->TurretMountId = FName(*FString::Printf(TEXT("%s_Mount"), *EquipmentId.ToString()));
		TestEquipment.TurretMountData->TurretMountWeightKg = TurretMountMassKg;

		TestEquipment.WeaponData = NewObject<UCFWeaponData>(Outer);
		TestEquipment.WeaponData->WeaponId = FName(*FString::Printf(TEXT("%s_Weapon"), *EquipmentId.ToString()));
		TestEquipment.WeaponData->WeaponSize = WeaponSize;
		TestEquipment.WeaponData->CompatibleMountTypes.Reset();
		TestEquipment.WeaponData->CompatibleMountTypes.Add(MountType);
		TestEquipment.WeaponData->WeaponMassKg = WeaponMassKg;

		TestEquipment.EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		TestEquipment.EquipmentPresetData->EquipmentId = EquipmentId;
		TestEquipment.EquipmentPresetData->RequiredMountType = MountType;
		TestEquipment.EquipmentPresetData->RequiredWeaponSize = WeaponSize;
		TestEquipment.EquipmentPresetData->DefaultTurretMountData = TestEquipment.TurretMountData;
		TestEquipment.EquipmentPresetData->DefaultWeaponData = TestEquipment.WeaponData;
		return TestEquipment;
	}

	// [v1.0.0] VehicleData에 테스트 하드포인트 위치 슬롯을 추가합니다.
	void AddFittingTestHardpoint(UCFVehicleData* VehicleData, const FName LocationSlotId)
	{
		// [v1.0.0] VehicleData에 추가할 테스트 하드포인트 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = LocationSlotId;
		HardpointSlot.LocationCategory = TEXT("Test");
		VehicleData->HardpointSlots.Add(HardpointSlot);
	}

	// [v1.0.0] VehicleData에 테스트 장착 프로파일을 추가합니다.
	void AddFittingTestMountProfile(
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

	// [v1.0.0] 피팅 선택 배열에 활성 또는 명시적 빈 장착 선택을 추가합니다.
	void AddFittingTestSelection(
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

	// [v1.0.0] Snapshot에 지정 문제 코드와 선택적 MountProfileId가 존재하는지 반환합니다.
	bool HasFittingIssue(
		const FCFVehicleFittingSnapshot& Snapshot,
		const ECFFittingIssueCode IssueCode,
		const FName MountProfileId = NAME_None)
	{
		// [v1.0.0] 문제 코드와 장착 프로파일을 비교할 한 검증 문제입니다.
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

	// [v1.0.0] Snapshot 검증 문제의 결정론적 순서를 비교할 서명 문자열을 생성합니다.
	FString BuildFittingIssueSignature(const FCFVehicleFittingSnapshot& Snapshot)
	{
		// [v1.0.0] 순서대로 결합할 구조화된 검증 문제 문자열 목록입니다.
		TArray<FString> IssueTexts;
		for (const FCFFittingValidationIssue& ValidationIssue : Snapshot.ValidationIssues)
		{
			IssueTexts.Add(FString::Printf(
				TEXT("%s:%s"),
				*UEnum::GetValueAsString(ValidationIssue.IssueCode),
				*ValidationIssue.MountProfileId.ToString()));
		}
		return FString::Join(IssueTexts, TEXT("|"));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFittingCompatibilityTest,
	"CarFight.Fitting.FIT_P0_03.Compatibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] MountProfile·Hardpoint·장비 호환, 선택 정책, 방어 3상태와 결정론적 순서를 검증합니다.
bool FCFFittingCompatibilityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 두 하드포인트와 장착 프로파일을 소유할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>();
	if (!TestNotNull(TEXT("Compatibility VehicleData 생성"), VehicleData))
	{
		return false;
	}
	VehicleData->BaseVehicleMassKg = 1200.0f;
	VehicleData->MaximumGrossMassKg = 3000.0f;

	AddFittingTestHardpoint(VehicleData, TEXT("Top_01"));
	AddFittingTestHardpoint(VehicleData, TEXT("Front_01"));

	// [v1.0.0] Roof Turret 프로파일에서 사용할 정상 장비 묶음입니다.
	FCFFittingTestEquipment RoofEquipment = CreateFittingTestEquipment(
		VehicleData,
		TEXT("RoofCannon"),
		ECFVehicleMountType::Turret,
		ECFVehicleWeaponSize::Large,
		100.0f,
		200.0f);

	// [v1.0.0] Front Launcher 프로파일에서 사용할 정상 장비 묶음입니다.
	FCFFittingTestEquipment FrontEquipment = CreateFittingTestEquipment(
		VehicleData,
		TEXT("FrontLauncher"),
		ECFVehicleMountType::Launcher,
		ECFVehicleWeaponSize::Medium,
		80.0f,
		120.0f);

	AddFittingTestMountProfile(
		VehicleData,
		TEXT("RoofTurret"),
		TEXT("Top_01"),
		ECFVehicleMountType::Turret,
		ECFVehicleWeaponSize::Large,
		RoofEquipment.EquipmentPresetData);
	AddFittingTestMountProfile(
		VehicleData,
		TEXT("FrontLauncher"),
		TEXT("Front_01"),
		ECFVehicleMountType::Launcher,
		ECFVehicleWeaponSize::Medium,
		FrontEquipment.EquipmentPresetData);

	// [v1.0.0] UseVehicleDefault 방어 선택에서 해석할 기본 방어 DataAsset입니다.
	UCFVehicleDefenseData* DefaultDefenseData = NewObject<UCFVehicleDefenseData>(VehicleData);
	DefaultDefenseData->DefenseId = TEXT("DefaultDefense");
	DefaultDefenseData->DefenseMassKg = 150.0f;
	VehicleData->DefaultDefenseData = DefaultDefenseData;

	// [v1.0.0] 장착 선택 순서 독립성과 정상 해석을 검증할 Transient FittingData입니다.
	UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>();
	if (!TestNotNull(TEXT("Compatibility FittingData 생성"), FittingData))
	{
		return false;
	}
	FittingData->FittingId = TEXT("CompatibilityFitting");
	FittingData->VehicleData = VehicleData;
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::UseVehicleDefault;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	AddFittingTestSelection(FittingData, TEXT("FrontLauncher"), FrontEquipment.EquipmentPresetData);
	AddFittingTestSelection(FittingData, TEXT("RoofTurret"), RoofEquipment.EquipmentPresetData);

	// [v1.0.0] VehicleData 순서로 해석되는 첫 정상 Snapshot입니다.
	const FCFVehicleFittingSnapshot FirstSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("정상 Compatibility Snapshot 유효"), FirstSnapshot.IsValid());
	TestEqual(TEXT("정상 Compatibility 문제 없음"), FirstSnapshot.ValidationIssues.Num(), 0);
	TestEqual(TEXT("장착 결과 2개"), FirstSnapshot.ResolvedMounts.Num(), 2);
	TestEqual(TEXT("첫 결과는 VehicleData 첫 RoofTurret"), FirstSnapshot.ResolvedMounts[0].MountProfileId, FName(TEXT("RoofTurret")));
	TestEqual(TEXT("두 번째 결과는 FrontLauncher"), FirstSnapshot.ResolvedMounts[1].MountProfileId, FName(TEXT("FrontLauncher")));
	TestEqual(TEXT("Roof 선택 소스 Override"), FirstSnapshot.ResolvedMounts[0].SelectionSource, ECFFittingSelectionSource::FittingOverride);
	TestEqual(TEXT("Front 선택 소스 Override"), FirstSnapshot.ResolvedMounts[1].SelectionSource, ECFFittingSelectionSource::FittingOverride);
	TestEqual(TEXT("방어 선택 방식 UseVehicleDefault"), FirstSnapshot.ResolvedDefenseSelectionMode, ECFDefenseSelectionMode::UseVehicleDefault);
	TestTrue(TEXT("기본 방어 DataAsset 해석"), FirstSnapshot.ResolvedDefenseData == DefaultDefenseData);

	FittingData->MountSelections.Swap(0, 1);

	// [v1.0.0] 선택 배열 순서를 바꾼 뒤 생성한 두 번째 정상 Snapshot입니다.
	const FCFVehicleFittingSnapshot ReorderedSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("순서 변경 Snapshot 유효"), ReorderedSnapshot.IsValid());
	TestEqual(TEXT("순서 변경 후 첫 결과 동일"), ReorderedSnapshot.ResolvedMounts[0].MountProfileId, FirstSnapshot.ResolvedMounts[0].MountProfileId);
	TestEqual(TEXT("순서 변경 후 두 번째 결과 동일"), ReorderedSnapshot.ResolvedMounts[1].MountProfileId, FirstSnapshot.ResolvedMounts[1].MountProfileId);
	TestEqual(TEXT("순서 변경 후 장비 질량 동일"), ReorderedSnapshot.EquipmentMassKg, FirstSnapshot.EquipmentMassKg);
	TestEqual(TEXT("순서 변경 후 문제 서명 동일"), BuildFittingIssueSignature(ReorderedSnapshot), BuildFittingIssueSignature(FirstSnapshot));

	FittingData->MountSelections.Reset();
	AddFittingTestSelection(FittingData, TEXT("FrontLauncher"), FrontEquipment.EquipmentPresetData);

	// [v1.0.0] Roof 선택 누락을 차량 기본 장비로 해석한 Snapshot입니다.
	const FCFVehicleFittingSnapshot VehicleDefaultSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("UseVehicleDefault 누락 정책 유효"), VehicleDefaultSnapshot.IsValid());
	TestEqual(TEXT("누락 Roof는 VehicleDefault"), VehicleDefaultSnapshot.ResolvedMounts[0].SelectionSource, ECFFittingSelectionSource::VehicleDefault);
	TestTrue(TEXT("누락 Roof 기본 프리셋 해석"), VehicleDefaultSnapshot.ResolvedMounts[0].EquipmentPresetData == RoofEquipment.EquipmentPresetData);

	FittingData->MountSelections.Reset();
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsEmpty;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::ExplicitNone;

	// [v1.0.0] 모든 누락 선택을 빈 장착으로 해석한 Snapshot입니다.
	const FCFVehicleFittingSnapshot PolicyEmptySnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("TreatAsEmpty Snapshot 유효"), PolicyEmptySnapshot.IsValid());
	TestEqual(TEXT("Roof 누락 정책 빈 장착"), PolicyEmptySnapshot.ResolvedMounts[0].SelectionSource, ECFFittingSelectionSource::MissingPolicyEmpty);
	TestEqual(TEXT("Front 누락 정책 빈 장착"), PolicyEmptySnapshot.ResolvedMounts[1].SelectionSource, ECFFittingSelectionSource::MissingPolicyEmpty);
	TestEqual(TEXT("명시적 방어 없음 방식 보존"), PolicyEmptySnapshot.ResolvedDefenseSelectionMode, ECFDefenseSelectionMode::ExplicitNone);
	TestNull(TEXT("명시적 방어 없음 DataAsset"), PolicyEmptySnapshot.ResolvedDefenseData.Get());
	TestEqual(TEXT("빈 장착 장비 질량 0"), PolicyEmptySnapshot.EquipmentMassKg, 0.0f);
	TestEqual(TEXT("방어 없음 질량 0"), PolicyEmptySnapshot.DefenseMassKg, 0.0f);

	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;

	// [v1.0.0] 누락 선택을 오류로 처리한 Snapshot입니다.
	const FCFVehicleFittingSnapshot MissingSelectionErrorSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("TreatAsError Snapshot 무효"), MissingSelectionErrorSnapshot.IsValid());
	TestTrue(TEXT("Roof 누락 선택 오류"), HasFittingIssue(MissingSelectionErrorSnapshot, ECFFittingIssueCode::MissingMountSelection, TEXT("RoofTurret")));
	TestTrue(TEXT("Front 누락 선택 오류"), HasFittingIssue(MissingSelectionErrorSnapshot, ECFFittingIssueCode::MissingMountSelection, TEXT("FrontLauncher")));

	AddFittingTestSelection(FittingData, TEXT("RoofTurret"), nullptr, false);
	AddFittingTestSelection(FittingData, TEXT("FrontLauncher"), nullptr, false);

	// [v1.0.0] TreatAsError에서도 두 프로파일을 명시적으로 비운 Snapshot입니다.
	const FCFVehicleFittingSnapshot ExplicitEmptySnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("명시적 빈 장착 Snapshot 유효"), ExplicitEmptySnapshot.IsValid());
	TestEqual(TEXT("Roof 명시적 빈 장착"), ExplicitEmptySnapshot.ResolvedMounts[0].SelectionSource, ECFFittingSelectionSource::ExplicitEmpty);
	TestEqual(TEXT("Front 명시적 빈 장착"), ExplicitEmptySnapshot.ResolvedMounts[1].SelectionSource, ECFFittingSelectionSource::ExplicitEmpty);

	FittingData->MountSelections.Reset();
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::UseVehicleDefault;
	AddFittingTestSelection(FittingData, TEXT("UnknownMount"), RoofEquipment.EquipmentPresetData);

	// [v1.0.0] VehicleData에 없는 선택 ID를 포함한 Snapshot입니다.
	const FCFVehicleFittingSnapshot UnknownProfileSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("알 수 없는 MountProfile Snapshot 무효"), UnknownProfileSnapshot.IsValid());
	TestTrue(TEXT("알 수 없는 MountProfile 오류"), HasFittingIssue(UnknownProfileSnapshot, ECFFittingIssueCode::UnknownMountProfile, TEXT("UnknownMount")));

	FittingData->MountSelections.Reset();
	AddFittingTestSelection(FittingData, TEXT("RoofTurret"), RoofEquipment.EquipmentPresetData);
	AddFittingTestSelection(FittingData, TEXT("RoofTurret"), FrontEquipment.EquipmentPresetData);

	// [v1.0.0] 같은 장착 프로파일 선택이 중복된 Snapshot입니다.
	const FCFVehicleFittingSnapshot DuplicateSelectionSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("중복 MountSelection Snapshot 무효"), DuplicateSelectionSnapshot.IsValid());
	TestTrue(TEXT("중복 MountSelection 오류"), HasFittingIssue(DuplicateSelectionSnapshot, ECFFittingIssueCode::DuplicateMountSelection, TEXT("RoofTurret")));

	FittingData->MountSelections.Reset();
	AddFittingTestSelection(FittingData, TEXT("RoofTurret"), RoofEquipment.EquipmentPresetData);
	AddFittingTestSelection(FittingData, TEXT("FrontLauncher"), FrontEquipment.EquipmentPresetData);
	VehicleData->MountProfiles[1].LocationSlotRef = TEXT("MissingSlot");

	// [v1.0.0] 존재하지 않는 하드포인트를 참조하는 Snapshot입니다.
	const FCFVehicleFittingSnapshot MissingHardpointSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("하드포인트 누락 Snapshot 무효"), MissingHardpointSnapshot.IsValid());
	TestTrue(TEXT("하드포인트 누락 오류"), HasFittingIssue(MissingHardpointSnapshot, ECFFittingIssueCode::MissingHardpointSlot, TEXT("FrontLauncher")));
	VehicleData->MountProfiles[1].LocationSlotRef = TEXT("Front_01");

	FrontEquipment.EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;

	// [v1.0.0] 프리셋 요구 MountType이 프로파일과 다른 Snapshot입니다.
	const FCFVehicleFittingSnapshot MountTypeMismatchSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("MountType 불일치 Snapshot 무효"), MountTypeMismatchSnapshot.IsValid());
	TestTrue(TEXT("MountType 불일치 오류"), HasFittingIssue(MountTypeMismatchSnapshot, ECFFittingIssueCode::MountTypeMismatch, TEXT("FrontLauncher")));
	FrontEquipment.EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Launcher;

	FrontEquipment.EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// [v1.0.0] 프리셋 요구 크기가 프로파일 제한을 초과한 Snapshot입니다.
	const FCFVehicleFittingSnapshot WeaponSizeExceededSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("WeaponSize 초과 Snapshot 무효"), WeaponSizeExceededSnapshot.IsValid());
	TestTrue(TEXT("WeaponSize 초과 오류"), HasFittingIssue(WeaponSizeExceededSnapshot, ECFFittingIssueCode::WeaponSizeExceeded, TEXT("FrontLauncher")));
	FrontEquipment.EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;

	FrontEquipment.WeaponData->CompatibleMountTypes.Reset();
	FrontEquipment.WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);

	// [v1.0.0] WeaponData가 프로파일 MountType을 지원하지 않는 Snapshot입니다.
	const FCFVehicleFittingSnapshot WeaponMountMismatchSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("Weapon Mount 비호환 Snapshot 무효"), WeaponMountMismatchSnapshot.IsValid());
	TestTrue(TEXT("Weapon Mount 비호환 오류"), HasFittingIssue(WeaponMountMismatchSnapshot, ECFFittingIssueCode::WeaponMountIncompatible, TEXT("FrontLauncher")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFittingMassSnapshotTest,
	"CarFight.Fitting.FIT_P0_03.MassSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Base·Mount·Weapon·Defense 질량 합산, 사용률, 질량 오류와 GrossMass 초과를 검증합니다.
bool FCFFittingMassSnapshotTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 질량 합산과 최대 총중량을 검증할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>();
	if (!TestNotNull(TEXT("Mass VehicleData 생성"), VehicleData))
	{
		return false;
	}
	VehicleData->BaseVehicleMassKg = 1000.0f;
	VehicleData->MaximumGrossMassKg = 2000.0f;

	AddFittingTestHardpoint(VehicleData, TEXT("Top_01"));
	AddFittingTestHardpoint(VehicleData, TEXT("Top_02"));

	// [v1.0.0] 첫 장착 프로파일에서 250kg을 기여할 테스트 장비입니다.
	FCFFittingTestEquipment FirstEquipment = CreateFittingTestEquipment(
		VehicleData,
		TEXT("FirstEquipment"),
		ECFVehicleMountType::Turret,
		ECFVehicleWeaponSize::Medium,
		100.0f,
		150.0f);

	// [v1.0.0] 두 번째 장착 프로파일에서 450kg을 기여할 테스트 장비입니다.
	FCFFittingTestEquipment SecondEquipment = CreateFittingTestEquipment(
		VehicleData,
		TEXT("SecondEquipment"),
		ECFVehicleMountType::Turret,
		ECFVehicleWeaponSize::Large,
		200.0f,
		250.0f);

	AddFittingTestMountProfile(
		VehicleData,
		TEXT("Mount_A"),
		TEXT("Top_01"),
		ECFVehicleMountType::Turret,
		ECFVehicleWeaponSize::Medium,
		FirstEquipment.EquipmentPresetData);
	AddFittingTestMountProfile(
		VehicleData,
		TEXT("Mount_B"),
		TEXT("Top_02"),
		ECFVehicleMountType::Turret,
		ECFVehicleWeaponSize::Large,
		SecondEquipment.EquipmentPresetData);

	// [v1.0.0] 총중량에 200kg을 기여할 Override 방어 DataAsset입니다.
	UCFVehicleDefenseData* OverrideDefenseData = NewObject<UCFVehicleDefenseData>(VehicleData);
	OverrideDefenseData->DefenseId = TEXT("HeavyDefense");
	OverrideDefenseData->DefenseMassKg = 200.0f;

	// [v1.0.0] 두 장비와 Override 방어를 선택할 Transient FittingData입니다.
	UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>();
	if (!TestNotNull(TEXT("Mass FittingData 생성"), FittingData))
	{
		return false;
	}
	FittingData->FittingId = TEXT("MassFitting");
	FittingData->VehicleData = VehicleData;
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::Override;
	FittingData->DefenseSelection.DefenseData = OverrideDefenseData;
	AddFittingTestSelection(FittingData, TEXT("Mount_A"), FirstEquipment.EquipmentPresetData);
	AddFittingTestSelection(FittingData, TEXT("Mount_B"), SecondEquipment.EquipmentPresetData);

	// [v1.0.0] 모든 정상 질량을 합산한 기준 Snapshot입니다.
	const FCFVehicleFittingSnapshot ValidMassSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("정상 Mass Snapshot 유효"), ValidMassSnapshot.IsValid());
	TestEqual(TEXT("기준 차량 질량 1000kg"), ValidMassSnapshot.BaseVehicleMassKg, 1000.0f);
	TestEqual(TEXT("장비 질량 700kg"), ValidMassSnapshot.EquipmentMassKg, 700.0f);
	TestEqual(TEXT("Ammo 연동 전 질량 0kg"), ValidMassSnapshot.AmmoMassKg, 0.0f);
	TestEqual(TEXT("방어 질량 200kg"), ValidMassSnapshot.DefenseMassKg, 200.0f);
	TestEqual(TEXT("탑재 질량 900kg"), ValidMassSnapshot.PayloadMassKg, 900.0f);
	TestEqual(TEXT("차량 총중량 1900kg"), ValidMassSnapshot.TotalVehicleMassKg, 1900.0f);
	TestEqual(TEXT("최대 총중량 2000kg"), ValidMassSnapshot.MaximumGrossMassKg, 2000.0f);
	TestTrue(TEXT("탑재 사용률 0.9"), FMath::IsNearlyEqual(ValidMassSnapshot.PayloadUsageRatio, 0.9f));
	TestTrue(TEXT("총중량 사용률 0.95"), FMath::IsNearlyEqual(ValidMassSnapshot.GrossMassUsageRatio, 0.95f));

	VehicleData->MaximumGrossMassKg = 1800.0f;

	// [v1.0.0] 동일 장비가 최대 허용 총중량을 초과한 Snapshot입니다.
	const FCFVehicleFittingSnapshot OverGrossMassSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("과중량 Snapshot 무효"), OverGrossMassSnapshot.IsValid());
	TestTrue(TEXT("GrossMassExceeded 오류"), HasFittingIssue(OverGrossMassSnapshot, ECFFittingIssueCode::GrossMassExceeded));
	VehicleData->MaximumGrossMassKg = 2000.0f;

	FirstEquipment.WeaponData->WeaponMassKg = 0.0f;

	// [v1.0.0] 선택된 WeaponData의 질량 소스가 미설정인 Snapshot입니다.
	const FCFVehicleFittingSnapshot MissingWeaponMassSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("무기 질량 누락 Snapshot 무효"), MissingWeaponMassSnapshot.IsValid());
	TestTrue(TEXT("무기 질량 MissingMassSource"), HasFittingIssue(MissingWeaponMassSnapshot, ECFFittingIssueCode::MissingMassSource, TEXT("Mount_A")));
	FirstEquipment.WeaponData->WeaponMassKg = 150.0f;

	SecondEquipment.TurretMountData->TurretMountWeightKg = -1.0f;

	// [v1.0.0] 선택된 TurretMountData의 질량이 음수인 Snapshot입니다.
	const FCFVehicleFittingSnapshot InvalidTurretMassSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("터렛 질량 오류 Snapshot 무효"), InvalidTurretMassSnapshot.IsValid());
	TestTrue(TEXT("터렛 질량 InvalidMassValue"), HasFittingIssue(InvalidTurretMassSnapshot, ECFFittingIssueCode::InvalidMassValue, TEXT("Mount_B")));
	SecondEquipment.TurretMountData->TurretMountWeightKg = 200.0f;

	OverrideDefenseData->DefenseMassKg = 0.0f;

	// [v1.0.0] 선택된 방어 패키지 질량이 미설정인 Snapshot입니다.
	const FCFVehicleFittingSnapshot MissingDefenseMassSnapshot = FittingData->BuildFittingSnapshot();
	TestFalse(TEXT("방어 질량 누락 Snapshot 무효"), MissingDefenseMassSnapshot.IsValid());
	TestTrue(TEXT("방어 질량 MissingMassSource"), HasFittingIssue(MissingDefenseMassSnapshot, ECFFittingIssueCode::MissingMassSource));
	OverrideDefenseData->DefenseMassKg = 200.0f;

	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::ExplicitNone;
	FittingData->DefenseSelection.DefenseData = nullptr;

	// [v1.0.0] 방어 패키지 없음을 명시한 정상 Snapshot입니다.
	const FCFVehicleFittingSnapshot ExplicitNoDefenseSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("ExplicitNone 방어 Snapshot 유효"), ExplicitNoDefenseSnapshot.IsValid());
	TestEqual(TEXT("ExplicitNone 방어 질량 0"), ExplicitNoDefenseSnapshot.DefenseMassKg, 0.0f);
	TestEqual(TEXT("ExplicitNone 총중량 1700"), ExplicitNoDefenseSnapshot.TotalVehicleMassKg, 1700.0f);
	TestNull(TEXT("ExplicitNone 방어 참조 없음"), ExplicitNoDefenseSnapshot.ResolvedDefenseData.Get());

	VehicleData->DefaultDefenseData = nullptr;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	// [v1.0.0] 차량 기본 방어가 None인 Legacy 호환 Snapshot입니다.
	const FCFVehicleFittingSnapshot LegacyNoDefenseSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("기본 방어 None Legacy Snapshot 유효"), LegacyNoDefenseSnapshot.IsValid());
	TestEqual(TEXT("기본 방어 None 질량 0"), LegacyNoDefenseSnapshot.DefenseMassKg, 0.0f);
	TestEqual(TEXT("기본 방어 선택 방식 보존"), LegacyNoDefenseSnapshot.ResolvedDefenseSelectionMode, ECFDefenseSelectionMode::UseVehicleDefault);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
