// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-18
// Description: Phase 4 Vehicle Target Lock Runtime의 독립 Config / State / actor-free Snapshot 계약
// Changelog:
// - v1.0.0: Vehicle Target Lock의 P0 상태, 요청 결과, Break 사유, 독립 Targeting tuning과 actor-free Snapshot을 최초 정의.
// Migration:
// - Sensor Analysis/Contact tuning을 Lock tuning으로 재사용하지 않습니다.
// - HUD/Input/Guided Weapon source migration은 Phase 5~7 후속 범위이며 이 타입만으로 기존 Consumer 의미를 변경하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetingTypes.generated.h"

/** Vehicle Target Lock Runtime의 지속 상태입니다. */
UENUM(BlueprintType)
enum class ECFTargetLockState : uint8
{
	Idle UMETA(DisplayName="대기"),
	Acquiring UMETA(DisplayName="락 획득 중"),
	Locked UMETA(DisplayName="락 완료")
};

/** RequestLock 요청의 명시적 결과입니다. */
UENUM(BlueprintType)
enum class ECFTargetLockRequestResult : uint8
{
	None UMETA(DisplayName="없음"),
	Accepted UMETA(DisplayName="요청 수락"),
	InvalidTarget UMETA(DisplayName="대상 무효"),
	RuntimeNotReady UMETA(DisplayName="타겟팅 런타임 미준비"),
	SensorUnavailable UMETA(DisplayName="센서 사용 불가"),
	NoSensorContact UMETA(DisplayName="센서 Contact 없음"),
	ContactNotLive UMETA(DisplayName="Live Contact 아님"),
	AlreadyAcquiring UMETA(DisplayName="이미 같은 대상 락 획득 중"),
	AlreadyLocked UMETA(DisplayName="이미 같은 대상 락 완료")
};

/** Locked 상태가 끊어진 이유를 exact-once transition과 함께 공개합니다. */
UENUM(BlueprintType)
enum class ECFTargetLockBreakReason : uint8
{
	None UMETA(DisplayName="없음"),
	ContactLost UMETA(DisplayName="Contact Lost"),
	TargetDestroyed UMETA(DisplayName="대상 파괴"),
	SensorUnavailable UMETA(DisplayName="센서 사용 불가"),
	QualityDepleted UMETA(DisplayName="락 품질 소진")
};

/** Sensor tuning과 독립적인 P0 Vehicle Target Lock 변화율 설정입니다. */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFTargetingConfig
{
	GENERATED_BODY()

	// Live Contact에서 Acquiring 진행률을 초당 증가시킬 양입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Targeting|Config", meta=(ClampMin="0.001", DisplayName="락 획득 증가율", ToolTip="Live Contact를 유지할 때 LockProgress01이 초당 증가하는 양입니다. Sensor AnalysisGain과 독립입니다."))
	float LockAcquireGainPerSec = 1.0f;

	// Acquiring 중 Contact가 LastKnown으로 바뀌었을 때 진행률을 초당 감소시킬 양입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Targeting|Config", meta=(ClampMin="0.0", DisplayName="락 획득 감쇠율", ToolTip="락 획득 중 Contact가 LastKnown이면 LockProgress01이 초당 감소하는 양입니다."))
	float LockAcquireDecayPerSec = 0.5f;

	// Locked 대상이 다시 Live가 됐을 때 LockQuality를 초당 회복할 양입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Targeting|Config", meta=(ClampMin="0.0", DisplayName="락 품질 회복률", ToolTip="Locked 대상의 Contact가 Live이면 LockQuality01을 초당 회복하는 양입니다."))
	float LockQualityRecoveryPerSec = 1.0f;

	// Locked 대상이 LastKnown일 때 LockQuality를 초당 감소시킬 양입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Targeting|Config", meta=(ClampMin="0.001", DisplayName="락 품질 감쇠율", ToolTip="Locked 대상의 Contact가 LastKnown이면 LockQuality01이 초당 감소하는 양입니다. 0이 되면 Lock Break가 발생합니다."))
	float LockQualityDecayPerSec = 0.5f;

	// 모든 P0 Targeting tuning 값이 유한하고 상태 전이를 완료할 수 있는 범위인지 반환합니다.
	bool IsValid() const;

	// 로그와 테스트에서 사용할 간단한 Targeting 설정 요약을 만듭니다.
	FString BuildDebugSummary() const;
};

/** HUD와 일반 Consumer가 Actor 없이 읽을 Vehicle Target Lock 공개 Snapshot입니다. */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFTargetingSnapshot
{
	GENERATED_BODY()

	// 현재 Vehicle Target Lock의 지속 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Targeting|Snapshot", meta=(DisplayName="타겟 락 상태", ToolTip="Idle, Acquiring, Locked 중 현재 Vehicle Target Lock 상태입니다."))
	ECFTargetLockState State = ECFTargetLockState::Idle;

	// 현재 Acquiring 또는 Locked 대상의 Sensor ContactId입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Targeting|Snapshot", meta=(DisplayName="타겟 Contact ID", ToolTip="현재 락 획득 또는 락 완료 대상의 Sensor ContactId입니다. Actor 참조를 포함하지 않습니다."))
	FName TargetContactId = NAME_None;

	// Acquiring 상태의 0~1 락 획득 진행률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Targeting|Snapshot", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="락 획득 진행률", ToolTip="Acquiring 상태에서 증가·감소하는 락 획득 진행률입니다. Locked 상태에서는 1입니다."))
	float LockProgress01 = 0.0f;

	// Locked 상태의 0~1 추적 품질입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Targeting|Snapshot", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="락 품질", ToolTip="Locked 상태에서 Contact 상태에 따라 회복·감소하는 추적 품질입니다."))
	float LockQuality01 = 0.0f;

	// 실제 Locked 상태 Break가 발생할 때마다 1씩 증가하는 전이 Revision입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Targeting|Snapshot", meta=(DisplayName="락 브레이크 전이 리비전", ToolTip="Locked 상태가 실제 Break될 때마다 증가합니다. HUD는 이 값을 이용해 같은 Break 피드백을 중복 표시하지 않을 수 있습니다."))
	int32 BreakTransitionRevision = 0;

	// 가장 최근 BreakTransitionRevision에 대응하는 Break 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Targeting|Snapshot", meta=(DisplayName="마지막 락 브레이크 사유", ToolTip="가장 최근 Vehicle Target Lock Break의 원인입니다. 수동 ClearLock은 Break 이벤트가 아니므로 이 값을 갱신하지 않습니다."))
	ECFTargetLockBreakReason LastBreakReason = ECFTargetLockBreakReason::None;

	// Snapshot의 상태별 필드 조합과 0~1 범위가 공개 계약을 만족하는지 반환합니다.
	bool IsPublicContractValid() const;
};
