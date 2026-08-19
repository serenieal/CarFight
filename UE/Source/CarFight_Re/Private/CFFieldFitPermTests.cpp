// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-14
// Description: CF-FQ-034 FFIT-P0-01 Permission·Blocker Query Pawn 없는 자동화 테스트
// Scope: 허용 상태, 각 Provider blocker, 이동/정지 유지시간 중복 억제, 다중 blocker 순서와 무효 입력을 검증합니다.
// Changelog:
// - v1.0.0: FFIT-P0-01 Permission Query 회귀를 최초 추가.
// Migration:
// - Pawn, World, Component, Inventory Reservation을 생성하거나 수정하지 않습니다.
// - FFIT-P0-02 Timed Action·Reservation을 완료한 것으로 해석하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFieldFitPermission.h"

#include "Misc/AutomationTest.h"

namespace
{
	// [v1.0.0] 모든 FFIT-P0-01 시작 조건을 만족한 정상 입력을 생성합니다.
	FCFFieldFitPermissionInput BuildAllowedFieldFitPermissionInput()
	{
		// [v1.0.0] 개별 blocker 테스트가 한 필드만 바꿔 사용할 정상 기준 입력입니다.
		FCFFieldFitPermissionInput PermissionInput;
		PermissionInput.bCombatActive = false;
		PermissionInput.VehicleSpeedKmh = 0.2f;
		PermissionInput.StationaryDurationSeconds = 3.0f;
		PermissionInput.StationarySpeedThresholdKmh = 0.5f;
		PermissionInput.RequiredStationaryDurationSeconds = 2.0f;
		PermissionInput.bVehicleRuntimeReady = true;
		PermissionInput.bWeaponCooldownActive = false;
		PermissionInput.bLauncherSequenceActive = false;
		PermissionInput.bAmmoReloadActive = false;
		PermissionInput.bAmmoTransferActive = false;
		PermissionInput.bDefenseCooldownActive = false;
		PermissionInput.bRepairActive = false;
		PermissionInput.bConflictingTimedActionActive = false;
		PermissionInput.bInventoryAccessible = true;
		return PermissionInput;
	}

	// [v1.0.0] 결과에 지정 구조화 차단 사유가 존재하는지 반환합니다.
	bool HasFieldFitBlocker(
		const FCFFieldFitPermissionResult& PermissionResult,
		const ECFFieldFitBlockerSource Source,
		const ECFFieldFitBlockReason Reason)
	{
		return PermissionResult.Blockers.ContainsByPredicate(
			[Source, Reason](const FCFFieldFitBlocker& Blocker)
			{
				return Blocker.Source == Source && Blocker.Reason == Reason;
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitPermissionTest,
	"CarFight.Fitting.FFIT_P0_01.Permission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 정상 허용과 Combat/Runtime/Weapon/Launcher/Ammo/Defense/Action/Inventory blocker 수집을 검증합니다.
bool FCFFieldFitPermissionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] blocker가 없어 시작 가능해야 할 정상 결과입니다.
	const FCFFieldFitPermissionResult AllowedResult = FCFFieldFitPermissionQuery::Evaluate(BuildAllowedFieldFitPermissionInput());
	TestTrue(TEXT("정상 입력 Field Fitting 시작 허용"), AllowedResult.bCanStart);
	TestEqual(TEXT("정상 입력 Blocker 0"), AllowedResult.Blockers.Num(), 0);

	// [v1.0.0] 여러 Provider가 동시에 차단하는 입력입니다.
	FCFFieldFitPermissionInput BlockedInput = BuildAllowedFieldFitPermissionInput();
	BlockedInput.bCombatActive = true;
	BlockedInput.bVehicleRuntimeReady = false;
	BlockedInput.bWeaponCooldownActive = true;
	BlockedInput.bLauncherSequenceActive = true;
	BlockedInput.bAmmoReloadActive = true;
	BlockedInput.bAmmoTransferActive = true;
	BlockedInput.bDefenseCooldownActive = true;
	BlockedInput.bRepairActive = true;
	BlockedInput.bConflictingTimedActionActive = true;
	BlockedInput.bInventoryAccessible = false;

	// [v1.0.0] 모든 비-Drive Provider blocker가 구조화되어야 할 결과입니다.
	const FCFFieldFitPermissionResult BlockedResult = FCFFieldFitPermissionQuery::Evaluate(BlockedInput);
	TestFalse(TEXT("다중 Blocker 시작 거부"), BlockedResult.bCanStart);
	TestEqual(TEXT("다중 Blocker 10개"), BlockedResult.Blockers.Num(), 10);
	TestTrue(TEXT("Combat blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Combat, ECFFieldFitBlockReason::CombatActive));
	TestTrue(TEXT("Runtime blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::VehicleRuntime, ECFFieldFitBlockReason::VehicleRuntimeNotReady));
	TestTrue(TEXT("Weapon blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Weapon, ECFFieldFitBlockReason::WeaponCooldownActive));
	TestTrue(TEXT("Launcher blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Launcher, ECFFieldFitBlockReason::LauncherSequenceActive));
	TestTrue(TEXT("Reload blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Ammo, ECFFieldFitBlockReason::AmmoReloadActive));
	TestTrue(TEXT("Ammo Transfer blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Ammo, ECFFieldFitBlockReason::AmmoTransferActive));
	TestTrue(TEXT("Defense Cooldown blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Defense, ECFFieldFitBlockReason::DefenseCooldownActive));
	TestTrue(TEXT("Repair blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Defense, ECFFieldFitBlockReason::RepairActive));
	TestTrue(TEXT("Action blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Action, ECFFieldFitBlockReason::ConflictingTimedActionActive));
	TestTrue(TEXT("Inventory blocker"), HasFieldFitBlocker(BlockedResult, ECFFieldFitBlockerSource::Inventory, ECFFieldFitBlockReason::InventoryInaccessible));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitDrivePermissionTest,
	"CarFight.Fitting.FFIT_P0_01.DriveAndInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 이동 중/정지 유지시간 부족을 중복 없이 구분하고 무효 수치 입력을 안전 차단하는지 검증합니다.
bool FCFFieldFitDrivePermissionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 정지 임계값보다 빠르게 이동 중인 입력입니다.
	FCFFieldFitPermissionInput MovingInput = BuildAllowedFieldFitPermissionInput();
	MovingInput.VehicleSpeedKmh = 0.6f;
	MovingInput.StationaryDurationSeconds = 0.0f;
	// [v1.0.0] 이동 중에는 정지 지속시간 blocker를 중복하지 않아야 할 결과입니다.
	const FCFFieldFitPermissionResult MovingResult = FCFFieldFitPermissionQuery::Evaluate(MovingInput);
	TestFalse(TEXT("이동 중 시작 거부"), MovingResult.bCanStart);
	TestEqual(TEXT("이동 중 Drive blocker 1개"), MovingResult.Blockers.Num(), 1);
	TestTrue(TEXT("VehicleMoving blocker"), HasFieldFitBlocker(MovingResult, ECFFieldFitBlockerSource::Drive, ECFFieldFitBlockReason::VehicleMoving));
	TestFalse(TEXT("이동 중 StationaryDuration 중복 없음"), HasFieldFitBlocker(MovingResult, ECFFieldFitBlockerSource::Drive, ECFFieldFitBlockReason::StationaryDurationInsufficient));

	// [v1.0.0] 속도는 정지 범위지만 요구 정지시간만 부족한 입력입니다.
	FCFFieldFitPermissionInput SettlingInput = BuildAllowedFieldFitPermissionInput();
	SettlingInput.VehicleSpeedKmh = 0.4f;
	SettlingInput.StationaryDurationSeconds = 1.5f;
	// [v1.0.0] 정지 유지시간 부족만 나타나야 할 결과입니다.
	const FCFFieldFitPermissionResult SettlingResult = FCFFieldFitPermissionQuery::Evaluate(SettlingInput);
	TestFalse(TEXT("정지 유지시간 부족 시작 거부"), SettlingResult.bCanStart);
	TestEqual(TEXT("정지 유지시간 blocker 1개"), SettlingResult.Blockers.Num(), 1);
	TestTrue(TEXT("StationaryDuration blocker"), HasFieldFitBlocker(SettlingResult, ECFFieldFitBlockerSource::Drive, ECFFieldFitBlockReason::StationaryDurationInsufficient));

	// [v1.0.0] 튜닝 데이터 오류를 재현하는 음수 속도 임계값 입력입니다.
	FCFFieldFitPermissionInput InvalidInput = BuildAllowedFieldFitPermissionInput();
	InvalidInput.StationarySpeedThresholdKmh = -1.0f;
	// [v1.0.0] 잘못된 튜닝 값을 다른 gameplay blocker로 오해하지 않을 안전 결과입니다.
	const FCFFieldFitPermissionResult InvalidResult = FCFFieldFitPermissionQuery::Evaluate(InvalidInput);
	TestFalse(TEXT("무효 Permission 입력 시작 거부"), InvalidResult.bCanStart);
	TestEqual(TEXT("무효 입력 blocker 1개"), InvalidResult.Blockers.Num(), 1);
	TestTrue(TEXT("InvalidPermissionInput blocker"), HasFieldFitBlocker(InvalidResult, ECFFieldFitBlockerSource::PermissionInput, ECFFieldFitBlockReason::InvalidPermissionInput));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
