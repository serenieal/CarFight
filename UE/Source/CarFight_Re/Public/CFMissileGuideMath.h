// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-29
// Description: CarFight 물리 제한형 미사일 Guidance 순수 수학
// Scope: 시선 각속도, 제한형 비례항법 횡가속도와 선회율 제한을 계산합니다.
// Changelog:
// - v1.0.0: MG-P0-00 Bounded Proportional Navigation 계산 계약 최초 추가.
// Migration:
// - 이 함수는 Command만 반환하며 Actor Transform 또는 ProjectileMovement Velocity를 변경하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileGuideTypes.h"

namespace CFMissileGuideMath
{
	CARFIGHT_RE_API float CalculateDirectionAngleDeg(const FVector& FromDirection, const FVector& ToDirection);

	CARFIGHT_RE_API FVector CalculateLineOfSightAngularVelocityRadPerSec(
		const FVector& RelativeLocation,
		const FVector& RelativeVelocity);

	CARFIGHT_RE_API float CalculateTurnRateAccelerationLimitCmPerSecSq(
		float CurrentSpeedCmPerSec,
		float MaximumTurnRateDegPerSec);

	CARFIGHT_RE_API FCFMissileGuidanceCommand CalculateBoundedProportionalNavigationCommand(
		const FCFMissileGuidanceInput& Input,
		const FCFMissileGuideConfig& Config);
}
