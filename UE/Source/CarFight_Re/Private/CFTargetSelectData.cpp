// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-23
// Description: CarFight 타겟 선택 설정 DataAsset 구현
// Scope: 설정값의 NaN, 무한대와 허용 범위를 검사하고 디버그 요약 문자열을 생성합니다.
// Changelog:
// - v1.0.0: TS-P0-01 설정 검증과 요약 생성을 추가.
// Migration:
// - 유효하지 않은 DataAsset 설정은 TargetSelectComp에서 Fallback 설정으로 대체한다.

#include "CFTargetSelectData.h"

namespace
{
	// [v1.0.0] 부동소수점 값이 유한하고 0 이상인지 반환합니다.
	bool IsFiniteNonNegative(const float Value)
	{
		return FMath::IsFinite(Value) && Value >= 0.0f;
	}
}

// [v1.0.0] 타겟 선택 설정값이 유한하고 허용 범위 안에 있는지 반환합니다.
bool UCFTargetSelectData::IsTargetSelectConfigValid() const
{
	// [v1.0.0] 직접 선택 최대 거리가 유효한지 여부입니다.
	const bool bDirectDistanceValid = IsFiniteNonNegative(TargetSelectConfig.DirectSelectMaxDistanceCm);

	// [v1.0.0] 근접 선택 최대 거리가 유효한지 여부입니다.
	const bool bProximityDistanceValid = IsFiniteNonNegative(TargetSelectConfig.ProximitySelectMaxDistanceCm);

	// [v1.0.0] 근접 후보 반각이 0 초과 180 이하인지 여부입니다.
	const bool bProximityAngleValid = FMath::IsFinite(TargetSelectConfig.ProximityHalfAngleDeg)
		&& TargetSelectConfig.ProximityHalfAngleDeg > 0.0f
		&& TargetSelectConfig.ProximityHalfAngleDeg <= 180.0f;

	// [v1.0.0] 후보 갱신 간격이 유한하고 0보다 큰지 여부입니다.
	const bool bRefreshIntervalValid = FMath::IsFinite(TargetSelectConfig.CandidateRefreshIntervalSec)
		&& TargetSelectConfig.CandidateRefreshIntervalSec > 0.0f;

	// [v1.0.0] 가림 유예 시간이 유한하고 0 이상인지 여부입니다.
	const bool bOcclusionGraceValid = IsFiniteNonNegative(TargetSelectConfig.OcclusionGracePeriodSec);

	// [v1.0.0] 후보 전환 우위 비율이 유한하고 0~1 범위인지 여부입니다.
	const bool bSwitchRatioValid = FMath::IsFinite(TargetSelectConfig.CandidateSwitchAdvantageRatio)
		&& TargetSelectConfig.CandidateSwitchAdvantageRatio >= 0.0f
		&& TargetSelectConfig.CandidateSwitchAdvantageRatio <= 1.0f;

	return bDirectDistanceValid
		&& bProximityDistanceValid
		&& bProximityAngleValid
		&& bRefreshIntervalValid
		&& bOcclusionGraceValid
		&& bSwitchRatioValid;
}

// [v1.0.0] 디버그 패널과 로그에 사용할 타겟 선택 설정 요약 문자열을 생성합니다.
FString UCFTargetSelectData::BuildTargetSelectSummary() const
{
	// [v1.0.0] 현재 설정 유효성 검사 결과를 표시할 문자열입니다.
	const FString ValidationText = IsTargetSelectConfigValid() ? TEXT("Valid") : TEXT("Invalid");

	return FString::Printf(
		TEXT("TargetSelectData: Validation=%s, Direct=%.1fcm, Proximity=%.1fcm, HalfAngle=%.2fdeg, Refresh=%.3fs, OcclusionGrace=%.2fs, SwitchAdvantage=%.2f"),
		*ValidationText,
		TargetSelectConfig.DirectSelectMaxDistanceCm,
		TargetSelectConfig.ProximitySelectMaxDistanceCm,
		TargetSelectConfig.ProximityHalfAngleDeg,
		TargetSelectConfig.CandidateRefreshIntervalSec,
		TargetSelectConfig.OcclusionGracePeriodSec,
		TargetSelectConfig.CandidateSwitchAdvantageRatio);
}
