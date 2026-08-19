// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-15
// Description: CF-FQ-036 Sensor Contact 공개 데이터 계약 구현 / SEN-P0-02 bounded 설정 검증
// Scope: Sensor Config 검증, Actor-free Contact 검증과 Snapshot 결정 정렬을 구현합니다.
// Changelog:
// - v1.1.0: ActiveScanRangeCm=0 비활성 계약과 MaxActorScansPerUpdate 범위 검증을 추가.
// - v1.0.0: SEN-P0-01 Config/Contact/Snapshot 기본 검증과 결정 정렬을 최초 구현.
// Migration:
// - 이 파일은 월드 검색, LOS Trace, Contact 생성·수명 갱신을 수행하지 않습니다.

#include "CFSensorTypes.h"

namespace
{
	// [v1.0.0] 부동소수점 값이 유한하고 0 이상인지 반환합니다.
	bool IsFiniteNonNegative(const float Value)
	{
		return FMath::IsFinite(Value) && Value >= 0.0f;
	}

	// [v1.0.0] 월드 위치 벡터의 모든 축이 유한한 값인지 반환합니다.
	bool IsFiniteVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}

	// [v1.0.0] InformationLevel이 대상 정체 공개가 가능한 Identified 이상인지 반환합니다.
	bool IsIdentifiedOrHigher(const ECFTargetInfoLevel InformationLevel)
	{
		return InformationLevel == ECFTargetInfoLevel::Identified
			|| InformationLevel == ECFTargetInfoLevel::DetailedScan;
	}
}

// [v1.0.0] Sensor 설정이 유한하고 P0 의미 범위를 만족하는지 반환합니다.
bool FCFSensorConfig::IsValid() const
{
	// [v1.0.0] Passive 탐지 거리가 유한하고 0 이상인지 여부입니다.
	const bool bPassiveRangeValid = IsFiniteNonNegative(PassiveDetectionRangeCm);

		// [v1.1.0] Active Scan 거리가 0으로 비활성이거나, 활성 시 Passive 거리 이상인지 여부입니다.
	const bool bActiveRangeValid = IsFiniteNonNegative(ActiveScanRangeCm)
		&& (ActiveScanRangeCm <= KINDA_SMALL_NUMBER || ActiveScanRangeCm >= PassiveDetectionRangeCm);

	// [v1.0.0] Visual 탐지 후보 거리가 유한하고 0 이상인지 여부입니다.
	const bool bVisualRangeValid = IsFiniteNonNegative(VisualDetectionRangeCm);

	// [v1.0.0] Sensor 갱신 간격이 유한하고 0보다 큰지 여부입니다.
	const bool bUpdateIntervalValid = FMath::IsFinite(UpdateIntervalSec)
		&& UpdateIntervalSec > 0.0f;

		// [v1.1.0] 한 update의 bounded Actor 검사 예산이 허용 범위인지 여부입니다.
	const bool bActorScanBudgetValid = MaxActorScansPerUpdate >= 1
		&& MaxActorScansPerUpdate <= 4096;

	// [v1.0.0] Contact 기억 시간이 유한하고 0 이상인지 여부입니다.
	const bool bContactMemoryValid = IsFiniteNonNegative(ContactMemoryTimeSec);

	// [v1.0.0] 파괴 Contact 보존 시간이 유한하고 0 이상인지 여부입니다.
	const bool bDestroyedHoldValid = IsFiniteNonNegative(DestroyedHoldTimeSec);

	// [v1.0.0] Active Scan 지속 시간이 유한하고 0 이상인지 여부입니다.
	const bool bActiveScanDurationValid = IsFiniteNonNegative(ActiveScanDurationSec);

	// [v1.0.0] Tactical Analysis 증가율이 유한하고 0 이상인지 여부입니다.
	const bool bAnalysisGainValid = IsFiniteNonNegative(AnalysisGainPerSec);

	// [v1.0.0] Tactical Analysis 감소율이 유한하고 0 이상인지 여부입니다.
	const bool bAnalysisDecayValid = IsFiniteNonNegative(AnalysisDecayPerSec);

	// [v1.0.0] Identified 임계값이 0~1 범위인지 여부입니다.
	const bool bIdentifiedThresholdValid = FMath::IsFinite(IdentifiedThreshold)
		&& IdentifiedThreshold >= 0.0f
		&& IdentifiedThreshold <= 1.0f;

	// [v1.0.0] DetailedScan 임계값이 Identified보다 크고 1 이하인지 여부입니다.
	const bool bDetailedThresholdValid = FMath::IsFinite(DetailedScanThreshold)
		&& DetailedScanThreshold > IdentifiedThreshold
		&& DetailedScanThreshold <= 1.0f;

	return bPassiveRangeValid
		&& bActiveRangeValid
		&& bVisualRangeValid
				&& bUpdateIntervalValid
		&& bActorScanBudgetValid
		&& bContactMemoryValid
		&& bDestroyedHoldValid
		&& bActiveScanDurationValid
		&& bAnalysisGainValid
		&& bAnalysisDecayValid
		&& bIdentifiedThresholdValid
		&& bDetailedThresholdValid;
}

// [v1.0.0] 현재 Sensor 설정을 한 줄 디버그 문자열로 생성합니다.
FString FCFSensorConfig::BuildDebugSummary() const
{
	return FString::Printf(
				TEXT("SensorConfig: Valid=%s Passive=%.1fcm Active=%.1fcm Visual=%.1fcm Update=%.3fs ActorBudget=%d Memory=%.2fs DestroyedHold=%.2fs ActiveDuration=%.2fs Gain=%.3f/s Decay=%.3f/s Identified=%.2f Detailed=%.2f"),
		IsValid() ? TEXT("True") : TEXT("False"),
		PassiveDetectionRangeCm,
		ActiveScanRangeCm,
		VisualDetectionRangeCm,
				UpdateIntervalSec,
		MaxActorScansPerUpdate,
		ContactMemoryTimeSec,
		DestroyedHoldTimeSec,
		ActiveScanDurationSec,
		AnalysisGainPerSec,
		AnalysisDecayPerSec,
		IdentifiedThreshold,
		DetailedScanThreshold);
}

// [v1.0.0] 공개 Contact가 Actor-free Snapshot에 들어갈 수 있는 최소 의미 계약을 만족하는지 반환합니다.
bool FCFSensorContact::IsPublicContractValid() const
{
	if (ContactId.IsNone() || ContactState == ECFSensorContactState::Invalid)
	{
		return false;
	}

	if (InformationLevel == ECFTargetInfoLevel::None)
	{
		return false;
	}

	if (IsIdentifiedOrHigher(InformationLevel) && KnownTargetId.IsNone())
	{
		return false;
	}

	if (!IsFiniteVector(LastKnownWorldLocation)
		|| !FMath::IsFinite(LastObservedWorldTimeSeconds)
		|| LastObservedWorldTimeSeconds < 0.0
		|| !IsFiniteNonNegative(FreshnessSeconds)
		|| !FMath::IsFinite(AnalysisProgress01)
		|| AnalysisProgress01 < 0.0f
		|| AnalysisProgress01 > 1.0f)
	{
		return false;
	}

	if (bDestroyedConfirmed != (ContactState == ECFSensorContactState::DestroyedHold))
	{
		return false;
	}

	return true;
}

// [v1.0.0] ContactId를 기준으로 공개 Contact 목록을 결정적인 순서로 정렬합니다.
void FCFSensorSnapshot::SortContactsDeterministically()
{
	Contacts.Sort([](const FCFSensorContact& LeftContact, const FCFSensorContact& RightContact)
	{
		// [v1.0.0] 같은 입력에서 플랫폼과 프레임에 관계없이 동일한 순서를 만드는 문자열 ContactId 키입니다.
		const FString LeftContactId = LeftContact.ContactId.ToString();
		// [v1.0.0] 오른쪽 Contact의 결정 정렬용 문자열 ContactId 키입니다.
		const FString RightContactId = RightContact.ContactId.ToString();
		if (LeftContactId != RightContactId)
		{
			return LeftContactId < RightContactId;
		}

		return LeftContact.KnownTargetId.ToString() < RightContact.KnownTargetId.ToString();
	});
}

// [v1.0.0] Snapshot ContactId가 모두 유효하고 중복되지 않는지 반환합니다.
bool FCFSensorSnapshot::HasUniqueContactIds() const
{
	// [v1.0.0] 이미 확인한 ContactId를 기록해 중복을 검출하는 임시 집합입니다.
	TSet<FName> SeenContactIds;
	for (const FCFSensorContact& Contact : Contacts)
	{
		if (Contact.ContactId.IsNone() || SeenContactIds.Contains(Contact.ContactId))
		{
			return false;
		}

		SeenContactIds.Add(Contact.ContactId);
	}

	return true;
}

// [v1.0.0] Snapshot과 모든 Contact가 공개 데이터 최소 계약을 만족하는지 반환합니다.
bool FCFSensorSnapshot::IsPublicContractValid() const
{
	if (Revision < 0
		|| !FMath::IsFinite(SnapshotWorldTimeSeconds)
		|| SnapshotWorldTimeSeconds < 0.0
		|| !IsFiniteVector(SensorOriginWorldLocation)
		|| !IsFiniteVector(SensorForwardWorldDirection)
		|| !HasUniqueContactIds())
	{
		return false;
	}

	for (const FCFSensorContact& Contact : Contacts)
	{
		if (!Contact.IsPublicContractValid())
		{
			return false;
		}
	}

	return true;
}
