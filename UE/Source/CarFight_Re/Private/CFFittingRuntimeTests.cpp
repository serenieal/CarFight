// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.1
// Date: 2026-08-02
// Description: CF-FQ-034 FIT-P0-04~05 출격 피팅 Runtime Apply·Initial Mass Pawn 없는 자동화 테스트
// Scope: Legacy, 유효 Snapshot Commit, 무효 fallback, 부분 실패 Rollback, 초기 질량 기록·검증·재적용 거부와 Reset 수명을 검증합니다.
// Changelog:
// - v1.1.1: 13.6kg 부동소수점 표현 오차를 허용하도록 허용 오차 단언의 비교 정밀도를 0.001kg로 명시.
// - v1.1.0: FIT_P0_05 InitialMass 상태 계약, 1%·1kg 허용 오차, Verify Only, 다른 질량 거부와 Invalid Legacy fallback 검증을 추가.
// - v1.0.0: RuntimeApply와 AtomicBoundary 테스트를 최초 추가.
// Migration:
// - Transient UObject와 Fake Adapter만 사용하며 Pawn, World, Blueprint, Asset과 Map을 생성하거나 수정하지 않는다.
// - 엔진 Mass 필드와 실제 Physics State는 공식 Editor Build와 차량 Runtime 검증이 별도로 증명한다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFEquipmentPresetData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

namespace
{
	/** 테스트용 완성 장비 프리셋 묶음입니다. */
	struct FCFFittingRuntimeTestEquipment
	{
		// [v1.0.0] Snapshot에서 비교할 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = nullptr;

		// [v1.0.0] 장비 프리셋이 참조할 TurretMountData입니다.
		UCFTurretMountData* TurretMountData = nullptr;

		// [v1.0.0] 장비 프리셋이 참조할 WeaponData입니다.
		UCFWeaponData* WeaponData = nullptr;
	};

	/** Pawn 없이 적용 호출과 실패 Rollback을 기록하는 Fake Adapter입니다. */
	class FCFFittingRuntimeTestAdapter final : public ICFFittingRuntimeApplyAdapter
	{
	public:
		// [v1.0.0] 다음 Snapshot Defense 적용만 실패시킬지 여부입니다.
		bool bFailNextSnapshotDefenseApply = false;

		// [v1.0.0] Weapon 입력 적용 호출 횟수입니다.
		int32 WeaponApplyCallCount = 0;

		// [v1.0.0] Defense 입력 적용 호출 횟수입니다.
		int32 DefenseApplyCallCount = 0;

		// [v1.0.0] 마지막 Weapon 입력입니다.
		FCFFittingWeaponRuntimeInput LastWeaponInput;

		// [v1.0.0] 마지막 Defense 입력입니다.
		FCFFittingDefenseRuntimeInput LastDefenseInput;

		// [v1.0.0] Weapon 입력을 기록하고 적용 성공을 반환합니다.
		virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) override
		{
			++WeaponApplyCallCount;
			LastWeaponInput = WeaponInput;
			bOutWeaponRuntimeReady = WeaponInput.UsesLegacyVehicleConfiguration() || WeaponInput.bHasResolvedMount;
			return true;
		}

		// [v1.0.0] Defense 입력을 기록하고 요청된 실패를 재현합니다.
		virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) override
		{
			++DefenseApplyCallCount;
			LastDefenseInput = DefenseInput;
			if (bFailNextSnapshotDefenseApply && !DefenseInput.UsesLegacyVehicleConfiguration())
			{
				bFailNextSnapshotDefenseApply = false;
				bOutDefenseRuntimeReady = false;
				return false;
			}

			bOutDefenseRuntimeReady = DefenseInput.DefenseData != nullptr;
			return true;
		}
	};

	// [v1.0.0] 지정 ID와 질량을 가진 완성 장비 프리셋을 생성합니다.
	FCFFittingRuntimeTestEquipment CreateRuntimeTestEquipment(UObject* Outer, const FName EquipmentId, const float TurretMountMassKg, const float WeaponMassKg)
	{
		FCFFittingRuntimeTestEquipment TestEquipment;
		TestEquipment.TurretMountData = NewObject<UCFTurretMountData>(Outer);
		TestEquipment.TurretMountData->TurretMountId = FName(*FString::Printf(TEXT("%s_Mount"), *EquipmentId.ToString()));
		TestEquipment.TurretMountData->TurretMountWeightKg = TurretMountMassKg;
		TestEquipment.WeaponData = NewObject<UCFWeaponData>(Outer);
		TestEquipment.WeaponData->WeaponId = FName(*FString::Printf(TEXT("%s_Weapon"), *EquipmentId.ToString()));
		TestEquipment.WeaponData->WeaponSize = ECFVehicleWeaponSize::Medium;
		TestEquipment.WeaponData->CompatibleMountTypes.Reset();
		TestEquipment.WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		TestEquipment.WeaponData->WeaponMassKg = WeaponMassKg;
		TestEquipment.EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		TestEquipment.EquipmentPresetData->EquipmentId = EquipmentId;
		TestEquipment.EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		TestEquipment.EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Medium;
		TestEquipment.EquipmentPresetData->DefaultTurretMountData = TestEquipment.TurretMountData;
		TestEquipment.EquipmentPresetData->DefaultWeaponData = TestEquipment.WeaponData;
		return TestEquipment;
	}

	// [v1.0.0] 한 하드포인트와 터렛 MountProfile을 가진 VehicleData를 생성합니다.
	UCFVehicleData* CreateRuntimeTestVehicleData(UObject* Outer, UCFEquipmentPresetData* DefaultEquipmentPresetData, UCFVehicleDefenseData* DefaultDefenseData)
	{
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);
		VehicleData->BaseVehicleMassKg = 1000.0f;
		VehicleData->MaximumGrossMassKg = 2500.0f;
		VehicleData->DefaultDefenseData = DefaultDefenseData;

		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Test");
		VehicleData->HardpointSlots.Add(HardpointSlot);

		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret");
		MountProfile.LocationSlotRef = TEXT("Top_01");
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Medium;
		MountProfile.DefaultEquipmentPresetData = DefaultEquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);
		return VehicleData;
	}

	// [v1.0.0] 지정 장비와 방어를 Override하는 유효 FittingData를 생성합니다.
	UCFVehicleFittingData* CreateRuntimeTestFittingData(UObject* Outer, UCFVehicleData* VehicleData, const FName FittingId, UCFEquipmentPresetData* EquipmentPresetData, UCFVehicleDefenseData* DefenseData)
	{
		UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>(Outer);
		FittingData->FittingId = FittingId;
		FittingData->VehicleData = VehicleData;
		FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsError;
		FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::Override;
		FittingData->DefenseSelection.DefenseData = DefenseData;

		FCFVehicleMountSelection MountSelection;
		MountSelection.MountProfileId = TEXT("RoofTurret");
		MountSelection.EquipmentPresetData = EquipmentPresetData;
		MountSelection.bEnabled = true;
		FittingData->MountSelections.Add(MountSelection);
		return FittingData;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingRuntimeApplyTest, "CarFight.Fitting.FIT_P0_04.RuntimeApply", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingAtomicBoundaryTest, "CarFight.Fitting.FIT_P0_04.AtomicBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFFittingInitialMassTest, "CarFight.Fitting.FIT_P0_05.InitialMass", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Legacy, Snapshot Commit, 재초기화와 Reset 수명을 검증합니다.
bool FCFFittingRuntimeApplyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	FCFFittingRuntimeTestEquipment LegacyEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("LegacyEquipment"), 80.0f, 120.0f);
	FCFFittingRuntimeTestEquipment OverrideEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("OverrideEquipment"), 90.0f, 140.0f);
	UCFVehicleDefenseData* LegacyDefenseData = NewObject<UCFVehicleDefenseData>();
	LegacyDefenseData->DefenseId = TEXT("LegacyDefense");
	LegacyDefenseData->DefenseMassKg = 100.0f;
	UCFVehicleDefenseData* OverrideDefenseData = NewObject<UCFVehicleDefenseData>();
	OverrideDefenseData->DefenseId = TEXT("OverrideDefense");
	OverrideDefenseData->DefenseMassKg = 130.0f;
	UCFVehicleData* VehicleData = CreateRuntimeTestVehicleData(GetTransientPackage(), LegacyEquipment.EquipmentPresetData, LegacyDefenseData);
	UCFVehicleFittingComp* FittingComp = NewObject<UCFVehicleFittingComp>();
	FCFFittingRuntimeTestAdapter RuntimeAdapter;

	TestTrue(TEXT("Legacy Prepare 성공"), FittingComp->PrepareSortieFitting(nullptr, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("Legacy Prepared 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::PreparedLegacy);
	TestEqual(TEXT("Prepare Weapon 무호출"), RuntimeAdapter.WeaponApplyCallCount, 0);
	TestTrue(TEXT("Legacy Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("Legacy Committed 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::CommittedLegacy);
	TestFalse(TEXT("Legacy Applied Snapshot 없음"), FittingComp->HasAppliedFittingSnapshot());

	UCFVehicleFittingData* FittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("RuntimeOverride"), OverrideEquipment.EquipmentPresetData, OverrideDefenseData);
	TestTrue(TEXT("Snapshot Prepare 성공"), FittingComp->PrepareSortieFitting(FittingData, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("Snapshot Prepare Weapon 무변경"), RuntimeAdapter.WeaponApplyCallCount, 1);
	TestTrue(TEXT("Snapshot Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestTrue(TEXT("Applied Snapshot 존재"), FittingComp->HasAppliedFittingSnapshot());
	TestEqual(TEXT("Applied Fitting ID"), FittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("RuntimeOverride")));
	TestEqual(TEXT("장비 Override 전달"), RuntimeAdapter.LastWeaponInput.EquipmentPresetData.Get(), OverrideEquipment.EquipmentPresetData);
	TestEqual(TEXT("방어 Override 전달"), RuntimeAdapter.LastDefenseInput.DefenseData.Get(), OverrideDefenseData);

	TestTrue(TEXT("재Prepare 성공"), FittingComp->PrepareSortieFitting(FittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("재Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("재초기화 Mount 수 1"), FittingComp->GetAppliedFittingSnapshot().ResolvedMounts.Num(), 1);

	FittingComp->ResetFittingRuntimeState();
	TestEqual(TEXT("Reset 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::Uninitialized);
	TestFalse(TEXT("Reset Applied 없음"), FittingComp->HasAppliedRuntimeInput());
	TestFalse(TEXT("Reset Snapshot 없음"), FittingComp->HasAppliedFittingSnapshot());
	return true;
}

// [v1.0.0] 무효 무변경과 부분 실패 Rollback을 검증합니다.
bool FCFFittingAtomicBoundaryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	FCFFittingRuntimeTestEquipment FirstEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("FirstEquipment"), 70.0f, 110.0f);
	FCFFittingRuntimeTestEquipment SecondEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("SecondEquipment"), 75.0f, 115.0f);
	UCFVehicleDefenseData* FirstDefenseData = NewObject<UCFVehicleDefenseData>();
	FirstDefenseData->DefenseId = TEXT("FirstDefense");
	FirstDefenseData->DefenseMassKg = 90.0f;
	UCFVehicleDefenseData* SecondDefenseData = NewObject<UCFVehicleDefenseData>();
	SecondDefenseData->DefenseId = TEXT("SecondDefense");
	SecondDefenseData->DefenseMassKg = 95.0f;
	UCFVehicleData* VehicleData = CreateRuntimeTestVehicleData(GetTransientPackage(), FirstEquipment.EquipmentPresetData, FirstDefenseData);
	UCFVehicleFittingData* FirstFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("FirstApplied"), FirstEquipment.EquipmentPresetData, FirstDefenseData);
	UCFVehicleFittingData* SecondFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("SecondCandidate"), SecondEquipment.EquipmentPresetData, SecondDefenseData);
	UCFVehicleFittingComp* FittingComp = NewObject<UCFVehicleFittingComp>();
	FCFFittingRuntimeTestAdapter RuntimeAdapter;

	TestTrue(TEXT("첫 Prepare 성공"), FittingComp->PrepareSortieFitting(FirstFittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("첫 Commit 성공"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	FirstFittingData->MountSelections.Reset();
	const int32 WeaponCallsBeforeInvalid = RuntimeAdapter.WeaponApplyCallCount;
	const int32 DefenseCallsBeforeInvalid = RuntimeAdapter.DefenseApplyCallCount;
	TestFalse(TEXT("무효 Snapshot Prepare 거부"), FittingComp->PrepareSortieFitting(FirstFittingData, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("무효 Prepare Weapon 무호출"), RuntimeAdapter.WeaponApplyCallCount, WeaponCallsBeforeInvalid);
	TestEqual(TEXT("무효 Prepare Defense 무호출"), RuntimeAdapter.DefenseApplyCallCount, DefenseCallsBeforeInvalid);
	TestEqual(TEXT("기존 Applied 유지"), FittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("FirstApplied")));

	TestTrue(TEXT("두 번째 Prepare 성공"), FittingComp->PrepareSortieFitting(SecondFittingData, VehicleData, TEXT("RoofTurret")));
	RuntimeAdapter.bFailNextSnapshotDefenseApply = true;
	TestFalse(TEXT("Defense 실패 Commit 거부"), FittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("실패 뒤 Applied 유지"), FittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("FirstApplied")));
	TestEqual(TEXT("Rollback Weapon 첫 장비"), RuntimeAdapter.LastWeaponInput.EquipmentPresetData.Get(), FirstEquipment.EquipmentPresetData);
	TestEqual(TEXT("Rollback Defense 첫 방어"), RuntimeAdapter.LastDefenseInput.DefenseData.Get(), FirstDefenseData);
	TestFalse(TEXT("실패 뒤 Prepared 없음"), FittingComp->HasPreparedRuntimeInput());

	TestTrue(TEXT("명시 Rollback Prepare"), FittingComp->PrepareSortieFitting(SecondFittingData, VehicleData, TEXT("RoofTurret")));
	const int32 WeaponCallsBeforeRollback = RuntimeAdapter.WeaponApplyCallCount;
	FittingComp->RollbackPreparedSortieFitting();
	TestEqual(TEXT("명시 Rollback 상태"), FittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::RolledBack);
		TestEqual(TEXT("명시 Rollback Adapter 무호출"), RuntimeAdapter.WeaponApplyCallCount, WeaponCallsBeforeRollback);
	return true;
}

// [v1.1.0] Legacy 유지, Snapshot 초기 질량, 허용 오차, Verify Only와 다른 질량 재적용 거부를 검증합니다.
bool FCFFittingInitialMassTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] Legacy와 Snapshot 경로가 공유할 기본 장비 프리셋입니다.
	FCFFittingRuntimeTestEquipment LegacyEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("MassLegacyEquipment"), 80.0f, 120.0f);
	// [v1.1.0] 초기 출격 Snapshot Target을 만들 Override 장비 프리셋입니다.
	FCFFittingRuntimeTestEquipment OverrideEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("MassOverrideEquipment"), 90.0f, 140.0f);
	// [v1.1.0] 다른 Runtime Mass 요청을 재현할 중량 장비 프리셋입니다.
	FCFFittingRuntimeTestEquipment HeavyEquipment = CreateRuntimeTestEquipment(GetTransientPackage(), TEXT("MassHeavyEquipment"), 180.0f, 260.0f);
	// [v1.1.0] VehicleData Legacy 방어 패키지입니다.
	UCFVehicleDefenseData* LegacyDefenseData = NewObject<UCFVehicleDefenseData>();
	LegacyDefenseData->DefenseId = TEXT("MassLegacyDefense");
	LegacyDefenseData->DefenseMassKg = 100.0f;
	// [v1.1.0] 유효 Snapshot의 방어 패키지입니다.
	UCFVehicleDefenseData* OverrideDefenseData = NewObject<UCFVehicleDefenseData>();
	OverrideDefenseData->DefenseId = TEXT("MassOverrideDefense");
	OverrideDefenseData->DefenseMassKg = 130.0f;
	// [v1.1.0] 다른 Target Mass를 만들 중량 방어 패키지입니다.
	UCFVehicleDefenseData* HeavyDefenseData = NewObject<UCFVehicleDefenseData>();
	HeavyDefenseData->DefenseId = TEXT("MassHeavyDefense");
	HeavyDefenseData->DefenseMassKg = 220.0f;
	// [v1.1.0] 모든 Initial Mass 시나리오가 공유할 차량 플랫폼입니다.
	UCFVehicleData* VehicleData = CreateRuntimeTestVehicleData(GetTransientPackage(), LegacyEquipment.EquipmentPresetData, LegacyDefenseData);
	// [v1.1.0] 1000 + 90 + 140 + 130으로 1360kg Target을 만들 피팅입니다.
	UCFVehicleFittingData* OverrideFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("MassOverride"), OverrideEquipment.EquipmentPresetData, OverrideDefenseData);
	// [v1.1.0] 이미 Physics State가 생성된 뒤 거부할 더 무거운 피팅입니다.
	UCFVehicleFittingData* HeavyFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("MassHeavy"), HeavyEquipment.EquipmentPresetData, HeavyDefenseData);

	// [v1.1.0] FittingData 미지정 시 기존 Chaos 질량과 Legacy Weapon·Defense 입력을 유지할 컴포넌트입니다.
	UCFVehicleFittingComp* LegacyFittingComp = NewObject<UCFVehicleFittingComp>();
	TestTrue(TEXT("Legacy Initial Prepare 성공"), LegacyFittingComp->PrepareInitialSortieFitting(nullptr, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("Legacy Initial Mass 사용"), LegacyFittingComp->UsesLegacyInitialMass());
	TestFalse(TEXT("Legacy Mass 기록 불필요"), LegacyFittingComp->ShouldApplyPreparedInitialMass());
	TestTrue(TEXT("Legacy Mass 검증은 기존 질량 유지로 통과"), LegacyFittingComp->VerifyInitialMassAfterPhysics(0.0f, 0.0f, false, false, false));
	TestEqual(TEXT("Legacy Mass 상태"), LegacyFittingComp->GetInitialMassState(), ECFInitialMassState::LegacyPreserved);

	// [v1.1.0] 유효 Snapshot의 초기 질량 상태와 Runtime Commit을 검증할 컴포넌트입니다.
	UCFVehicleFittingComp* SnapshotFittingComp = NewObject<UCFVehicleFittingComp>();
	FCFFittingRuntimeTestAdapter RuntimeAdapter;
	TestTrue(TEXT("Snapshot Initial Prepare 성공"), SnapshotFittingComp->PrepareInitialSortieFitting(OverrideFittingData, VehicleData, TEXT("RoofTurret")));
	TestFalse(TEXT("Snapshot은 Legacy Mass 아님"), SnapshotFittingComp->UsesLegacyInitialMass());
	TestTrue(TEXT("첫 Snapshot Mass 기록 필요"), SnapshotFittingComp->ShouldApplyPreparedInitialMass());
	// [v1.1.0] Snapshot 공식으로 계산된 예상 초기 출격 질량입니다.
	const float ExpectedTargetMassKg = 1360.0f;
	TestTrue(TEXT("Snapshot Target 1360kg"), FMath::IsNearlyEqual(SnapshotFittingComp->GetPreparedInitialMassKg(), ExpectedTargetMassKg));
	// [v1.1.0] 1360kg Target의 1% 허용 오차입니다.
	const float ExpectedToleranceKg = 13.6f;
		TestTrue(TEXT("Initial Mass 허용 오차 13.6kg"), FMath::IsNearlyEqual(UCFVehicleFittingComp::CalculateInitialMassToleranceKg(ExpectedTargetMassKg), ExpectedToleranceKg, 0.001f));
	TestTrue(TEXT("물리 생성 전 Movement Mass 기록"), SnapshotFittingComp->RecordInitialMassBeforePhysics(1500.0f, ExpectedTargetMassKg));
	TestTrue(TEXT("Configured Initial Mass 존재"), SnapshotFittingComp->HasConfiguredInitialMass());
	TestTrue(TEXT("실제 질량 허용 오차 안 검증 성공"), SnapshotFittingComp->VerifyInitialMassAfterPhysics(ExpectedTargetMassKg, ExpectedTargetMassKg + 5.0f, true, true, true));
	TestTrue(TEXT("Initial Mass 검증 완료"), SnapshotFittingComp->HasVerifiedInitialMass());
	TestTrue(TEXT("검증된 Cached Snapshot Commit 성공"), SnapshotFittingComp->CommitPreparedSortieFitting(RuntimeAdapter));
	TestEqual(TEXT("검증 뒤 Applied Fitting ID"), SnapshotFittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("MassOverride")));

	TestTrue(TEXT("같은 Snapshot 재초기화 Prepare 성공"), SnapshotFittingComp->PrepareInitialSortieFitting(OverrideFittingData, VehicleData, TEXT("RoofTurret")));
	TestFalse(TEXT("같은 질량 재초기화는 Mass 재기록 없음"), SnapshotFittingComp->ShouldApplyPreparedInitialMass());
	TestTrue(TEXT("같은 질량 재초기화 Verify Only 성공"), SnapshotFittingComp->VerifyInitialMassAfterPhysics(ExpectedTargetMassKg, ExpectedTargetMassKg, true, true, true));
	SnapshotFittingComp->RollbackPreparedSortieFitting();

	TestFalse(TEXT("다른 Target Mass 재적용 거부"), SnapshotFittingComp->PrepareInitialSortieFitting(HeavyFittingData, VehicleData, TEXT("RoofTurret")));
	TestEqual(TEXT("다른 질량 거부 상태"), SnapshotFittingComp->GetInitialMassState(), ECFInitialMassState::ReapplyRejected);
	TestFalse(TEXT("다른 질량 거부 뒤 Prepared 입력 없음"), SnapshotFittingComp->HasPreparedRuntimeInput());
	TestEqual(TEXT("다른 질량 거부 뒤 기존 Applied 유지"), SnapshotFittingComp->GetAppliedFittingSnapshot().FittingId, FName(TEXT("MassOverride")));

	// [v1.1.0] 실제 VehicleMesh 질량 불일치 실패를 독립적으로 검증할 컴포넌트입니다.
	UCFVehicleFittingComp* FailedMassFittingComp = NewObject<UCFVehicleFittingComp>();
	TestTrue(TEXT("실패 시나리오 Snapshot Prepare"), FailedMassFittingComp->PrepareInitialSortieFitting(OverrideFittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("실패 시나리오 PrePhysics 기록"), FailedMassFittingComp->RecordInitialMassBeforePhysics(1500.0f, ExpectedTargetMassKg));
	TestFalse(TEXT("실제 질량 허용 오차 밖 검증 실패"), FailedMassFittingComp->VerifyInitialMassAfterPhysics(ExpectedTargetMassKg, ExpectedTargetMassKg + 50.0f, true, true, true));
	TestEqual(TEXT("실제 질량 실패 상태"), FailedMassFittingComp->GetInitialMassState(), ECFInitialMassState::VerificationFailed);

	// [v1.1.0] 초기 Invalid Snapshot이 Legacy 입력으로 fallback하는지 확인할 독립 피팅입니다.
	UCFVehicleFittingData* InvalidFittingData = CreateRuntimeTestFittingData(GetTransientPackage(), VehicleData, TEXT("MassInvalid"), OverrideEquipment.EquipmentPresetData, OverrideDefenseData);
	InvalidFittingData->MountSelections.Reset();
	UCFVehicleFittingComp* InvalidFittingComp = NewObject<UCFVehicleFittingComp>();
	TestTrue(TEXT("초기 Invalid Snapshot Legacy fallback 성공"), InvalidFittingComp->PrepareInitialSortieFitting(InvalidFittingData, VehicleData, TEXT("RoofTurret")));
	TestTrue(TEXT("Invalid Snapshot Legacy 질량 유지"), InvalidFittingComp->UsesLegacyInitialMass());
	TestEqual(TEXT("Invalid Snapshot Legacy Prepared 상태"), InvalidFittingComp->GetRuntimeApplyState(), ECFFittingRuntimeApplyState::PreparedLegacy);

	SnapshotFittingComp->ResetFittingRuntimeState();
	TestFalse(TEXT("Reset Initial Mass 구성 없음"), SnapshotFittingComp->HasConfiguredInitialMass());
	TestFalse(TEXT("Reset Initial Mass 검증 없음"), SnapshotFittingComp->HasVerifiedInitialMass());
	TestEqual(TEXT("Reset Initial Mass 상태"), SnapshotFittingComp->GetInitialMassState(), ECFInitialMassState::Uninitialized);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
