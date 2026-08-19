// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-19
// Description: UI-P0-06 무기 내부 Charge P0 Runtime 순수 상태 구현
// Scope: explicit 최대/초기/발사당 소비/초당 회복 값만으로 상태를 관리하며 VehicleBattery나 다른 Resource를 참조하지 않습니다.
// Changelog:
// - v1.0.0: fail-closed 구성, 승인 한 발 소비, Game-Time 회복과 정규화 조회를 구현.
// Migration:
// - 잘못되거나 불완전한 Charge 입력은 Disabled로 초기화되어 기존 발사를 제한하지 않습니다.

#include "CFWeaponChargeRuntime.h"

// [v1.0.0] explicit 입력으로 상태를 구성하고 InitialCharge에서 시작합니다.
void FCFWeaponChargeRuntime::Configure(
	const float InMaximumCharge,
	const float InInitialCharge,
	const float InChargePerShot,
	const float InRecoveryPerSecond)
{
	Reset();

	// [v1.0.0] 네 입력이 모두 유한한 숫자인지 확인하는 조건입니다.
	const bool bAllValuesFinite = FMath::IsFinite(InMaximumCharge)
		&& FMath::IsFinite(InInitialCharge)
		&& FMath::IsFinite(InChargePerShot)
		&& FMath::IsFinite(InRecoveryPerSecond);
	// [v1.0.0] 초기 Charge가 유효한 최대 범위 안에 있는지 확인합니다.
	const bool bInitialChargeValid = InInitialCharge >= 0.0f
		&& InInitialCharge <= InMaximumCharge + KINDA_SMALL_NUMBER;
	// [v1.0.0] 한 발 소비량이 실제로 양수이며 최대 Charge를 넘지 않는지 확인합니다.
	const bool bChargePerShotValid = InChargePerShot > KINDA_SMALL_NUMBER
		&& InChargePerShot <= InMaximumCharge + KINDA_SMALL_NUMBER;
	if (!bAllValuesFinite
		|| InMaximumCharge <= KINDA_SMALL_NUMBER
		|| !bInitialChargeValid
		|| !bChargePerShotValid
		|| InRecoveryPerSecond <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	MaximumCharge = InMaximumCharge;
	InitialCharge = FMath::Clamp(InInitialCharge, 0.0f, MaximumCharge);
	ChargePerShot = InChargePerShot;
	RecoveryPerSecond = InRecoveryPerSecond;
	CurrentCharge = InitialCharge;
	bEnabled = true;
}

// [v1.0.0] Charge Runtime을 완전 비활성 상태로 초기화합니다.
void FCFWeaponChargeRuntime::Reset()
{
	bEnabled = false;
	CurrentCharge = 0.0f;
	MaximumCharge = 0.0f;
	InitialCharge = 0.0f;
	ChargePerShot = 0.0f;
	RecoveryPerSecond = 0.0f;
}

// [v1.0.0] 실제 Game-Time Delta만큼 현재 Charge를 최대값까지 회복합니다.
void FCFWeaponChargeRuntime::AdvanceRecovery(const float DeltaSeconds)
{
	if (!bEnabled
		|| !FMath::IsFinite(DeltaSeconds)
		|| DeltaSeconds <= 0.0f
		|| CurrentCharge >= MaximumCharge)
	{
		return;
	}

	// [v1.0.0] 이번 Game-Time 구간에서 회복할 Charge 양입니다.
	const float RecoveredCharge = RecoveryPerSecond * DeltaSeconds;
	CurrentCharge = FMath::Clamp(CurrentCharge + RecoveredCharge, 0.0f, MaximumCharge);
}

// [v1.0.0] 현재 Runtime이 동일한 explicit Charge 구성인지 비교합니다.
bool FCFWeaponChargeRuntime::MatchesConfiguration(
	const float InMaximumCharge,
	const float InInitialCharge,
	const float InChargePerShot,
	const float InRecoveryPerSecond) const
{
	return bEnabled
		&& FMath::IsNearlyEqual(MaximumCharge, InMaximumCharge)
		&& FMath::IsNearlyEqual(InitialCharge, InInitialCharge)
		&& FMath::IsNearlyEqual(ChargePerShot, InChargePerShot)
		&& FMath::IsNearlyEqual(RecoveryPerSecond, InRecoveryPerSecond);
}

// [v1.0.0] 다음 표준 한 발의 Charge 소비를 감당할 수 있는지 반환합니다. Disabled는 발사를 제한하지 않습니다.
bool FCFWeaponChargeRuntime::CanAcceptShot() const
{
	return !bEnabled || CurrentCharge + KINDA_SMALL_NUMBER >= ChargePerShot;
}

// [v1.0.0] 실제 승인된 한 발의 Charge를 정확히 한 번 소비합니다.
void FCFWeaponChargeRuntime::RecordAcceptedShot()
{
	if (!bEnabled || !CanAcceptShot())
	{
		return;
	}

	CurrentCharge = FMath::Clamp(CurrentCharge - ChargePerShot, 0.0f, MaximumCharge);
}

// [v1.0.0] 현재 Charge를 0~1로 정규화해 반환합니다.
float FCFWeaponChargeRuntime::GetChargeRatio() const
{
	if (!bEnabled || MaximumCharge <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentCharge / MaximumCharge, 0.0f, 1.0f);
}
