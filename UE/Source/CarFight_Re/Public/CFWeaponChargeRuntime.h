// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-19
// Description: UI-P0-06 무기 내부 Charge P0 Runtime 순수 상태 계약
// Scope: explicit WeaponData Charge 설정을 current/max/accepted-shot 소비/Game-Time 회복으로 관리합니다. VehicleBattery와 독립입니다.
// Changelog:
// - v1.0.0: explicit Charge 구성, 초기값, 승인 한 발 소비, Game-Time 회복과 fail-closed 비활성 계약을 추가.
// Migration:
// - 기존 WeaponData의 Charge 네 값 all-zero는 Disabled이며 발사를 제한하지 않습니다.
// - VehicleBattery, Cooldown, Heat를 Charge 값으로 대체하거나 추정하지 않습니다.

#pragma once

#include "CoreMinimal.h"

/**
 * 무기 자체 내부 축전기의 최소 P0 Runtime 상태입니다.
 */
struct CARFIGHT_RE_API FCFWeaponChargeRuntime
{
	// [v1.0.0] explicit 입력으로 상태를 구성하고 InitialCharge에서 시작합니다.
	void Configure(float InMaximumCharge, float InInitialCharge, float InChargePerShot, float InRecoveryPerSecond);

	// [v1.0.0] Charge Runtime을 완전 비활성 상태로 초기화합니다.
	void Reset();

	// [v1.0.0] 실제 Game-Time Delta만큼 현재 Charge를 최대값까지 회복합니다.
	void AdvanceRecovery(float DeltaSeconds);

	// [v1.0.0] 현재 Runtime이 동일한 explicit Charge 구성인지 비교합니다.
	bool MatchesConfiguration(float InMaximumCharge, float InInitialCharge, float InChargePerShot, float InRecoveryPerSecond) const;

	// [v1.0.0] 현재 Charge Runtime이 explicit 유효 구성으로 활성화됐는지 반환합니다.
	bool IsEnabled() const { return bEnabled; }

	// [v1.0.0] 다음 표준 한 발의 Charge 소비를 감당할 수 있는지 반환합니다. Disabled는 발사를 제한하지 않습니다.
	bool CanAcceptShot() const;

	// [v1.0.0] 실제 승인된 한 발의 Charge를 정확히 한 번 소비합니다.
	void RecordAcceptedShot();

	// [v1.0.0] 현재 무기 내부 Charge를 반환합니다.
	float GetCurrentCharge() const { return CurrentCharge; }

	// [v1.0.0] 현재 무기 내부 최대 Charge를 반환합니다.
	float GetMaximumCharge() const { return MaximumCharge; }

	// [v1.0.0] 현재 Charge를 0~1로 정규화해 반환합니다.
	float GetChargeRatio() const;

private:
	// [v1.0.0] explicit Charge 구성의 활성 상태입니다.
	bool bEnabled = false;

	// [v1.0.0] 현재 무기 내부 Charge입니다.
	float CurrentCharge = 0.0f;

	// [v1.0.0] 최대 무기 내부 Charge입니다.
	float MaximumCharge = 0.0f;

	// [v1.0.0] Runtime 초기화 시 시작 Charge입니다.
	float InitialCharge = 0.0f;

	// [v1.0.0] 승인된 실제 한 발당 소비 Charge입니다.
	float ChargePerShot = 0.0f;

	// [v1.0.0] Game-Time 1초당 회복 Charge입니다.
	float RecoveryPerSecond = 0.0f;
};
