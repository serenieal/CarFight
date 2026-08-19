// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-18
// Description: CF-FQ-032 UI-P0-06 무기 Heat P0 런타임 구현
// Scope: WeaponData에서 전달된 explicit Heat 설정만 사용하며 Cooldown·FireRate·Ammo에서 Heat 값을 추정하지 않습니다.
// Changelog:
// - v1.0.0: Heat configure/reset, 승인 발사 누적, 자연 냉각, 과열과 next-shot-headroom 회복을 구현.
// Migration:
// - Disabled Runtime은 RecordAcceptedShot/AdvanceCooling이 무효 동작이므로 기존 WeaponData 결과를 보존합니다.

#include "CFWeaponHeatRuntime.h"

// [v1.0.0] 명시적인 WeaponData Heat 설정으로 런타임을 구성하며 유효하지 않으면 Disabled 상태로 초기화합니다.
void FCFWeaponHeatRuntime::Configure(
	const float InHeatPerShot,
	const float InMaximumHeat,
	const float InHeatDissipationPerSecond)
{
	Reset();

	if (!FMath::IsFinite(InHeatPerShot)
		|| !FMath::IsFinite(InMaximumHeat)
		|| !FMath::IsFinite(InHeatDissipationPerSecond)
		|| InHeatPerShot <= KINDA_SMALL_NUMBER
		|| InMaximumHeat <= KINDA_SMALL_NUMBER
		|| InHeatDissipationPerSecond <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	bEnabled = true;
	HeatPerShot = InHeatPerShot;
	MaximumHeat = InMaximumHeat;
	HeatDissipationPerSecond = InHeatDissipationPerSecond;
}

// [v1.0.0] 현재 Heat와 설정을 모두 지우고 기존 무기와 같은 Disabled 상태로 되돌립니다.
void FCFWeaponHeatRuntime::Reset()
{
	bEnabled = false;
	HeatPerShot = 0.0f;
	MaximumHeat = 0.0f;
	HeatDissipationPerSecond = 0.0f;
	CurrentHeat = 0.0f;
	bOverheated = false;
}

// [v1.0.0] 비발사 시간의 자연 냉각을 적용하고 충분히 냉각되면 과열 차단을 해제합니다.
void FCFWeaponHeatRuntime::AdvanceCooling(const float DeltaSeconds)
{
	if (!bEnabled
		|| !FMath::IsFinite(DeltaSeconds)
		|| DeltaSeconds <= 0.0f
		|| CurrentHeat <= 0.0f)
	{
		return;
	}

	// [v1.0.0] 이번 Tick에서 자연 냉각으로 제거할 Heat 양입니다.
	const float DissipatedHeat = HeatDissipationPerSecond * DeltaSeconds;
	CurrentHeat = FMath::Clamp(CurrentHeat - DissipatedHeat, 0.0f, MaximumHeat);

	if (bOverheated && CurrentHeat <= GetRecoveryHeatThreshold() + KINDA_SMALL_NUMBER)
	{
		bOverheated = false;
	}
}

// [v1.0.0] 현재 Heat 설정이 지정 값과 같아 재구성 없이 상태를 보존할 수 있는지 반환합니다.
bool FCFWeaponHeatRuntime::MatchesConfiguration(
	const float InHeatPerShot,
	const float InMaximumHeat,
	const float InHeatDissipationPerSecond) const
{
	if (!bEnabled)
	{
		return false;
	}

	return FMath::IsNearlyEqual(HeatPerShot, InHeatPerShot)
		&& FMath::IsNearlyEqual(MaximumHeat, InMaximumHeat)
		&& FMath::IsNearlyEqual(HeatDissipationPerSecond, InHeatDissipationPerSecond);
}

// [v1.0.0] 실제 승인된 한 발의 Heat를 정확히 한 번 누적하고 MaxHeat 도달 시 과열로 전환합니다.
void FCFWeaponHeatRuntime::RecordAcceptedShot()
{
	if (!bEnabled || bOverheated)
	{
		return;
	}

	CurrentHeat = FMath::Clamp(CurrentHeat + HeatPerShot, 0.0f, MaximumHeat);
	if (CurrentHeat + KINDA_SMALL_NUMBER >= MaximumHeat)
	{
		CurrentHeat = MaximumHeat;
		bOverheated = true;
	}
}

// [v1.0.0] 현재 Heat를 0~1 범위로 정규화해 반환합니다.
float FCFWeaponHeatRuntime::GetHeatRatio() const
{
	if (!bEnabled || MaximumHeat <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentHeat / MaximumHeat, 0.0f, 1.0f);
}

// [v1.0.0] 다음 표준 한 발이 MaxHeat를 넘지 않도록 과열에서 회복해야 하는 Heat 임계값을 반환합니다.
float FCFWeaponHeatRuntime::GetRecoveryHeatThreshold() const
{
	if (!bEnabled)
	{
		return 0.0f;
	}

	return FMath::Max(MaximumHeat - HeatPerShot, 0.0f);
}
