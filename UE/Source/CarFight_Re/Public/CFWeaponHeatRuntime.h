// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-18
// Description: CF-FQ-032 UI-P0-06 무기 Heat P0 런타임 상태 계약
// Scope: 명시적인 발사 열량·최대 열량·초당 자연 냉각량만 사용해 누적, 과열 차단, 자연 냉각과 재사용 가능 전이를 결정적으로 처리합니다.
// Changelog:
// - v1.0.0: 발사당 Heat 누적, MaxHeat 과열, 자연 냉각, 다음 표준 1발이 들어갈 만큼 냉각되면 재사용하는 P0 Runtime을 추가.
// Migration:
// - 세 정적 입력 중 하나라도 0/비유한이면 Runtime은 Disabled이며 기존 무기의 발사 결과를 변경하지 않습니다.
// - 과열 회복 임계값은 별도 임의 수치가 아니라 max(MaximumHeat - HeatPerShot, 0)으로 계산해 다음 표준 1발이 MaxHeat를 넘지 않는 시점에만 재사용합니다.

#pragma once

#include "CoreMinimal.h"

/**
 * 활성 무기 한 개의 발사 열 누적과 자연 냉각을 소유하는 순수 C++ 런타임 상태입니다.
 */
struct CARFIGHT_RE_API FCFWeaponHeatRuntime
{
public:
	// [v1.0.0] 명시적인 WeaponData Heat 설정으로 런타임을 구성하며 유효하지 않으면 Disabled 상태로 초기화합니다.
	void Configure(float InHeatPerShot, float InMaximumHeat, float InHeatDissipationPerSecond);

	// [v1.0.0] 현재 Heat와 설정을 모두 지우고 기존 무기와 같은 Disabled 상태로 되돌립니다.
	void Reset();

	// [v1.0.0] 비발사 시간의 자연 냉각을 적용하고 충분히 냉각되면 과열 차단을 해제합니다.
	void AdvanceCooling(float DeltaSeconds);

	// [v1.0.0] 현재 Heat 설정이 지정 값과 같아 재구성 없이 상태를 보존할 수 있는지 반환합니다.
	bool MatchesConfiguration(float InHeatPerShot, float InMaximumHeat, float InHeatDissipationPerSecond) const;

	// [v1.0.0] 현재 무기가 Heat Runtime을 실제 사용하도록 구성됐는지 반환합니다.
	bool IsEnabled() const { return bEnabled; }

	// [v1.0.0] 과열 차단을 포함해 현재 Heat 관점에서 한 발을 추가로 허용할 수 있는지 반환합니다.
	bool CanAcceptShot() const { return !bEnabled || !bOverheated; }

	// [v1.0.0] 실제 승인된 한 발의 Heat를 정확히 한 번 누적하고 MaxHeat 도달 시 과열로 전환합니다.
	void RecordAcceptedShot();

	// [v1.0.0] 현재 누적 Heat를 반환합니다.
	float GetCurrentHeat() const { return CurrentHeat; }

	// [v1.0.0] 현재 활성 설정의 최대 Heat를 반환합니다.
	float GetMaximumHeat() const { return MaximumHeat; }

	// [v1.0.0] 현재 Heat를 0~1 범위로 정규화해 반환합니다.
	float GetHeatRatio() const;

	// [v1.0.0] 현재 활성 무기가 과열 차단 상태인지 반환합니다.
	bool IsOverheated() const { return bOverheated; }

	// [v1.0.0] 다음 표준 한 발이 MaxHeat를 넘지 않도록 과열에서 회복해야 하는 Heat 임계값을 반환합니다.
	float GetRecoveryHeatThreshold() const;

private:
	// [v1.0.0] 세 authored Heat 입력이 모두 유효해 Heat Runtime이 활성화됐는지 여부입니다.
	bool bEnabled = false;

	// [v1.0.0] 승인 발사 한 번마다 증가할 Heat 양입니다.
	float HeatPerShot = 0.0f;

	// [v1.0.0] 과열 상태로 전환되는 최대 Heat입니다.
	float MaximumHeat = 0.0f;

	// [v1.0.0] 비발사 시간에 1초마다 감소할 Heat 양입니다.
	float HeatDissipationPerSecond = 0.0f;

	// [v1.0.0] 현재 활성 무기에 누적된 Heat입니다.
	float CurrentHeat = 0.0f;

	// [v1.0.0] MaxHeat 도달 후 회복 임계값까지 냉각되기 전 발사를 막는 상태입니다.
	bool bOverheated = false;
};
