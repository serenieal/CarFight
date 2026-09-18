// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-18
// Description: Phase 4 Targeting Config / Snapshot 계약 검증 구현
// Changelog:
// - v1.0.0: 독립 Targeting tuning validation과 상태별 actor-free Snapshot validation을 최초 구현.
// Migration:
// - 기존 Sensor/TargetSelect 타입의 validation 의미는 변경하지 않습니다.

#include "CFTargetingTypes.h"

// 모든 P0 Targeting tuning 값이 유한하고 상태 전이를 완료할 수 있는 범위인지 반환합니다.
bool FCFTargetingConfig::IsValid() const
{
	return FMath::IsFinite(LockAcquireGainPerSec)
		&& FMath::IsFinite(LockAcquireDecayPerSec)
		&& FMath::IsFinite(LockQualityRecoveryPerSec)
		&& FMath::IsFinite(LockQualityDecayPerSec)
		&& LockAcquireGainPerSec > KINDA_SMALL_NUMBER
		&& LockAcquireDecayPerSec >= 0.0f
		&& LockQualityRecoveryPerSec >= 0.0f
		&& LockQualityDecayPerSec > KINDA_SMALL_NUMBER;
}

// 로그와 테스트에서 사용할 간단한 Targeting 설정 요약을 만듭니다.
FString FCFTargetingConfig::BuildDebugSummary() const
{
	return FString::Printf(
		TEXT("TargetingConfig AcquireGain=%.3f AcquireDecay=%.3f QualityRecovery=%.3f QualityDecay=%.3f"),
		LockAcquireGainPerSec,
		LockAcquireDecayPerSec,
		LockQualityRecoveryPerSec,
		LockQualityDecayPerSec);
}

// Snapshot의 상태별 필드 조합과 0~1 범위가 공개 계약을 만족하는지 반환합니다.
bool FCFTargetingSnapshot::IsPublicContractValid() const
{
	// 공개 진행률 두 값이 유한한지 여부입니다.
	const bool bFiniteProgress = FMath::IsFinite(LockProgress01) && FMath::IsFinite(LockQuality01);
	if (!bFiniteProgress
		|| LockProgress01 < 0.0f
		|| LockProgress01 > 1.0f
		|| LockQuality01 < 0.0f
		|| LockQuality01 > 1.0f
		|| BreakTransitionRevision < 0)
	{
		return false;
	}

	// Break Revision과 사유가 서로 일치하는지 여부입니다.
	const bool bBreakEvidenceValid = BreakTransitionRevision == 0
		? LastBreakReason == ECFTargetLockBreakReason::None
		: LastBreakReason != ECFTargetLockBreakReason::None;
	if (!bBreakEvidenceValid)
	{
		return false;
	}

	switch (State)
	{
	case ECFTargetLockState::Idle:
		return TargetContactId.IsNone()
			&& FMath::IsNearlyZero(LockProgress01)
			&& FMath::IsNearlyZero(LockQuality01);

	case ECFTargetLockState::Acquiring:
		return !TargetContactId.IsNone()
			&& LockProgress01 < 1.0f
			&& FMath::IsNearlyZero(LockQuality01);

	case ECFTargetLockState::Locked:
		return !TargetContactId.IsNone()
			&& FMath::IsNearlyEqual(LockProgress01, 1.0f)
			&& LockQuality01 > 0.0f;

	default:
		return false;
	}
}
