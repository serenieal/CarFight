// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-14
// Description: CF-FQ-034 FFIT-P0-01 Field Fitting Permission·Blocker Query 구현
// Scope: Provider 상태를 변경하지 않고 구조화 blocker와 최종 시작 Permission을 계산합니다.
// Changelog:
// - v1.0.0: 입력 검증, Provider별 blocker 수집과 정지 hysteresis 입력 계약을 최초 구현.
// Migration:
// - Reservation/Timed Action은 생성하지 않습니다.
// - 이동 중에는 VehicleMoving만 반환하고 StationaryDurationInsufficient를 중복 반환하지 않습니다.

#include "CFFieldFitPermission.h"

namespace
{
	// [v1.0.0] Permission Result에 구조화 blocker 한 건을 추가합니다.
	void AddPermissionBlocker(
		FCFFieldFitPermissionResult& PermissionResult,
		const ECFFieldFitBlockerSource Source,
		const ECFFieldFitBlockReason Reason)
	{
		// [v1.0.0] Provider와 원인 쌍을 보존할 blocker입니다.
		FCFFieldFitBlocker Blocker;
		Blocker.Source = Source;
		Blocker.Reason = Reason;
		PermissionResult.Blockers.Add(Blocker);
	}
}

// [v1.0.0] 수치와 정지 튜닝 값이 유한하고 음수가 아닌지 검증합니다.
bool FCFFieldFitPermissionInput::IsValid() const
{
	return FMath::IsFinite(VehicleSpeedKmh)
		&& FMath::IsFinite(StationaryDurationSeconds)
		&& FMath::IsFinite(StationarySpeedThresholdKmh)
		&& FMath::IsFinite(RequiredStationaryDurationSeconds)
		&& VehicleSpeedKmh >= 0.0f
		&& StationaryDurationSeconds >= 0.0f
		&& StationarySpeedThresholdKmh >= 0.0f
		&& RequiredStationaryDurationSeconds >= 0.0f;
}

// [v1.0.0] 입력을 변경하지 않고 모든 blocker를 고정 순서로 수집해 최종 Permission을 반환합니다.
FCFFieldFitPermissionResult FCFFieldFitPermissionQuery::Evaluate(const FCFFieldFitPermissionInput& PermissionInput)
{
	// [v1.0.0] 모든 Provider blocker와 최종 허용 여부를 반환할 결과입니다.
	FCFFieldFitPermissionResult PermissionResult;
	if (!PermissionInput.IsValid())
	{
		AddPermissionBlocker(
			PermissionResult,
			ECFFieldFitBlockerSource::PermissionInput,
			ECFFieldFitBlockReason::InvalidPermissionInput);
		return PermissionResult;
	}

	if (PermissionInput.bCombatActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Combat, ECFFieldFitBlockReason::CombatActive);
	}

	if (PermissionInput.VehicleSpeedKmh > PermissionInput.StationarySpeedThresholdKmh)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Drive, ECFFieldFitBlockReason::VehicleMoving);
	}
	else if (PermissionInput.StationaryDurationSeconds < PermissionInput.RequiredStationaryDurationSeconds)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Drive, ECFFieldFitBlockReason::StationaryDurationInsufficient);
	}

	if (!PermissionInput.bVehicleRuntimeReady)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::VehicleRuntime, ECFFieldFitBlockReason::VehicleRuntimeNotReady);
	}

	if (PermissionInput.bWeaponCooldownActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Weapon, ECFFieldFitBlockReason::WeaponCooldownActive);
	}

	if (PermissionInput.bLauncherSequenceActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Launcher, ECFFieldFitBlockReason::LauncherSequenceActive);
	}

	if (PermissionInput.bAmmoReloadActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Ammo, ECFFieldFitBlockReason::AmmoReloadActive);
	}

	if (PermissionInput.bAmmoTransferActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Ammo, ECFFieldFitBlockReason::AmmoTransferActive);
	}

	if (PermissionInput.bDefenseCooldownActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Defense, ECFFieldFitBlockReason::DefenseCooldownActive);
	}

	if (PermissionInput.bRepairActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Defense, ECFFieldFitBlockReason::RepairActive);
	}

	if (PermissionInput.bConflictingTimedActionActive)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Action, ECFFieldFitBlockReason::ConflictingTimedActionActive);
	}

	if (!PermissionInput.bInventoryAccessible)
	{
		AddPermissionBlocker(PermissionResult, ECFFieldFitBlockerSource::Inventory, ECFFieldFitBlockReason::InventoryInaccessible);
	}

	PermissionResult.bCanStart = PermissionResult.Blockers.IsEmpty();
	return PermissionResult;
}
