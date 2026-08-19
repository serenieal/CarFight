// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.1
// Date: 2026-08-15
// Description: CF-FQ-036 Sensor Contact 공개 데이터 계약 / SEN-P0-04 Active Scan Analysis 의미 동기화
// Scope: Sensor Contact 수명, Passive/Active 조정 설정, Sensor Knowledge와 Actor-free Snapshot 타입을 제공합니다.
// Changelog:
// - v1.2.1: SEN-P0-04 의미 변경 없이 주석 들여쓰기만 정규화.
// - v1.2.0: Active Scan Runtime, Tactical Analysis gain/decay, Identified/DetailedScan Knowledge와 Snapshot 실행 상태의 현재 의미를 ToolTip/Migration에 반영.
// - v1.1.0: 한 Sensor update에서 검사할 Actor 슬롯 상한 MaxActorScansPerUpdate를 추가하고 ActiveScanRangeCm=0을 Active Scan 비활성 값으로 허용.
// - v1.0.0: SEN-P0-01 ContactId, ContactState, Knowledge, Config와 Actor-free Snapshot 계약을 최초 추가.
// Migration:
// - v1.2.1은 formatting-only이며 Sensor Config/Data Contract Migration이 없습니다.
// - v1.2.0에서 ActiveScanRangeCm은 StartActiveScan으로 Runtime이 실행 중일 때만 장거리 전방향 Contact Detection에 사용합니다.
// - Tactical Analysis progress는 Active range + 직접 ECC_Visibility가 유효할 때 증가하며 비유효 상태에서는 AnalysisDecayPerSec로 감소합니다.
// - Identified/DetailedScan Knowledge는 Sensor Analysis threshold가 직접 승격하며 Source InformationLevel을 복사하지 않습니다.
// - v1.1.0부터 ActiveScanRangeCm=0은 Passive Detection만 사용하는 유효 설정입니다. 0이 아니면 PassiveDetectionRangeCm 이상이어야 합니다.
// - MaxActorScansPerUpdate는 탐지 성능 수치가 아니라 한 번의 bounded world scan 작업량 상한입니다.
// - ECFTargetRelation, ECFTargetCategory, ECFTargetInfoLevel은 의미가 같은 범위에서만 재사용합니다.
// - ECFTargetTrackState는 TargetSelect 추적 품질이므로 Sensor Contact lifecycle에 재사용하지 않습니다.
// - 공개 Contact/Snapshot에는 Actor 또는 UObject 포인터를 넣지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetSelectTypes.h"
#include "CFSensorTypes.generated.h"

/**
 * Sensor가 기억하는 Contact 자체의 생명주기입니다.
 * Removed는 별도 상태로 보관하지 않고 Snapshot에서 Contact가 사라진 것으로 표현합니다.
 */
UENUM(BlueprintType, meta=(DisplayName="센서 Contact 상태 (Sensor Contact State)", ToolTip="Sensor Contact가 현재 탐지 중인지, 마지막 관측만 남았는지, 신호를 잃었는지 또는 파괴 확인 보존 중인지 나타냅니다."))
enum class ECFSensorContactState : uint8
{
	// 유효한 Contact가 아닌 초기 상태입니다.
	Invalid UMETA(DisplayName="무효 (Invalid)"),

	// 현재 Sensor가 유효하게 관측하고 있는 Contact입니다.
	Live UMETA(DisplayName="실시간 Contact (Live)"),

	// 현재 관측은 끊겼지만 마지막 신뢰 위치와 정보를 기억하는 Contact입니다.
	LastKnown UMETA(DisplayName="마지막 확인 (Last Known)"),

	// Contact Memory가 만료되어 추적 신호를 잃은 Contact입니다.
	Lost UMETA(DisplayName="Contact 손실 (Lost)"),

	// 파괴가 확인되어 짧은 표시 보존 수명만 남은 Contact입니다.
	DestroyedHold UMETA(DisplayName="파괴 확인 보존 (Destroyed Hold)")
};

/**
 * 차량 Sensor Runtime의 P0 조정값입니다.
 * 거리·수명·Active Scan 기본값 0은 실제 게임 튜닝 수치가 아직 확정되지 않은 비활성 Foundation 값으로 사용할 수 있습니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="센서 설정 (Sensor Config)", ToolTip="Passive/Active/Visual 탐지 거리, Contact 기억, 분석 증가·감소와 정보 승격 임계값을 정의합니다."))
struct CARFIGHT_RE_API FCFSensorConfig
{
	GENERATED_BODY()

	// [v1.0.0] 상시 Passive Detection이 사용할 최대 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", Units="cm", DisplayName="Passive 탐지 거리 (PassiveDetectionRangeCm)", ToolTip="상시 Passive Detection의 최대 거리입니다. 0이면 Passive 거리 탐지가 비활성입니다."))
	float PassiveDetectionRangeCm = 0.0f;

	// [v1.0.0] Active Scan이 사용할 최대 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", Units="cm", DisplayName="Active Scan 거리 (ActiveScanRangeCm)", ToolTip="Active Scan 실행 중에만 적용되는 장거리 전방향 Contact 탐지 최대 거리입니다. 0이면 Active Scan 시작이 비활성입니다."))
	float ActiveScanRangeCm = 0.0f;

	// [v1.0.0] 명확하게 직접 보이는 대상을 최소 탐지 계약으로 다룰 후보 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", Units="cm", DisplayName="Visual 탐지 거리 (VisualDetectionRangeCm)", ToolTip="Passive 범위 밖에서 ECC_Visibility 직접 가시 대상의 최소 탐지 계약에 사용할 후보 거리입니다."))
	float VisualDetectionRangeCm = 0.0f;

				// [v1.0.0] Sensor Detection, Contact lifecycle과 Tactical Analysis 갱신을 수행할 시간 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.001", Units="s", DisplayName="센서 갱신 간격 (UpdateIntervalSec)", ToolTip="Sensor Detection, Contact lifecycle과 Tactical Analysis를 다시 계산할 기본 시간 간격입니다."))
	float UpdateIntervalSec = 0.1f;

	// [v1.1.0] 한 번의 bounded Sensor Detection update에서 검사할 월드 Actor 슬롯의 최대 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="1", ClampMax="4096", DisplayName="Update당 Actor 검사 상한 (MaxActorScansPerUpdate)", ToolTip="한 번의 bounded Passive/Active Detection update에서 월드 Level Actor 배열을 최대 몇 슬롯까지 검사할지 제한합니다. Sensor 탐지 거리와 별개의 성능 예산입니다."))
	int32 MaxActorScansPerUpdate = 64;

	// [v1.0.0] Live 관측이 끊긴 뒤 LastKnown 상태를 기억할 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", Units="s", DisplayName="Contact 기억 시간 (ContactMemoryTimeSec)", ToolTip="현재 관측이 끊긴 뒤 마지막 신뢰 위치와 정보를 LastKnown으로 유지할 시간입니다."))
	float ContactMemoryTimeSec = 0.0f;

	// [v1.0.0] 파괴 확인 Contact를 Snapshot에 보존할 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", Units="s", DisplayName="파괴 Contact 보존 시간 (DestroyedHoldTimeSec)", ToolTip="파괴가 확인된 Contact를 Radar/Target 소비자가 잠시 표현할 수 있도록 보존할 시간입니다."))
	float DestroyedHoldTimeSec = 0.0f;

	// [v1.0.0] 한 번의 Active Scan이 지속될 기본 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", Units="s", DisplayName="Active Scan 지속 시간 (ActiveScanDurationSec)", ToolTip="한 번 시작한 Active Scan이 유효한 상태로 유지될 기본 시간입니다."))
	float ActiveScanDurationSec = 0.0f;

	// [v1.0.0] 유효한 Tactical Analysis 동안 초당 증가할 정규화 진행률입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", DisplayName="분석 증가율 (AnalysisGainPerSec)", ToolTip="유효한 Active Scan/Tactical Analysis 동안 매초 증가할 0~1 진행률의 증가량입니다."))
	float AnalysisGainPerSec = 0.0f;

	// [v1.0.0] 분석 대상이 일시 가림 또는 범위 이탈했을 때 초당 감소할 정규화 진행률입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", DisplayName="분석 감소율 (AnalysisDecayPerSec)", ToolTip="Tactical Analysis가 일시 중단됐을 때 즉시 초기화하지 않고 매초 감소시킬 0~1 진행률의 감소량입니다."))
	float AnalysisDecayPerSec = 0.0f;

	// [v1.0.0] Detected에서 Identified로 승격할 정규화 분석 진행률입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="Identified 임계값 (IdentifiedThreshold)", ToolTip="Tactical Analysis 진행률이 이 값 이상이면 Identified 정보 단계로 승격할 수 있습니다."))
	float IdentifiedThreshold = 0.5f;

	// [v1.0.0] Identified에서 DetailedScan으로 승격할 정규화 분석 진행률입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Sensor|Config", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="Detailed Scan 임계값 (DetailedScanThreshold)", ToolTip="Tactical Analysis 진행률이 이 값 이상이면 DetailedScan 정보 단계로 승격할 수 있습니다."))
	float DetailedScanThreshold = 1.0f;

	// [v1.0.0] Sensor 설정이 유한하고 P0 의미 범위를 만족하는지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 현재 Sensor 설정을 한 줄 디버그 문자열로 생성합니다.
	FString BuildDebugSummary() const;
};

/**
 * UI, TargetSelect Adapter와 향후 Network가 읽을 수 있는 Actor-free Sensor Contact 한 건입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="센서 Contact (Sensor Contact)", ToolTip="Contact ID, 공개 가능한 Target Knowledge, Contact 수명과 마지막 신뢰 위치를 Actor 포인터 없이 전달합니다."))
struct CARFIGHT_RE_API FCFSensorContact
{
	GENERATED_BODY()

	// [v1.0.0] Sensor Runtime이 Contact 수명 동안 유지하는 독립 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="Contact ID", ToolTip="TargetId와 별개로 Sensor Runtime이 발급하며 Live→LastKnown→재획득 동안 같은 Contact를 식별합니다."))
	FName ContactId = NAME_None;

	// [v1.0.0] 플레이어가 식별 단계에 도달했을 때만 공개할 TargetId입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="확인된 Target ID (KnownTargetId)", ToolTip="대상이 Identified 이상으로 식별됐을 때 공개할 TargetId입니다. Detected 단계에서는 None일 수 있으며 ContactId와 같은 식별자가 아닙니다."))
	FName KnownTargetId = NAME_None;

	// [v1.0.0] 현재 공개 가능한 대상 표시 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="확인된 표시 이름 (KnownDisplayName)", ToolTip="InformationLevel이 허용하는 범위에서만 채우는 Player-facing 대상 이름입니다."))
	FText KnownDisplayName;

	// [v1.0.0] 현재 플레이어에게 공개 가능한 대상의 큰 기능 분류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="대상 분류", ToolTip="기존 TargetSelect 공용 의미 타입을 재사용하는 차량, 장치, 환경 또는 미분류 상태입니다."))
	ECFTargetCategory TargetCategory = ECFTargetCategory::Unknown;

	// [v1.0.0] 현재 플레이어에게 공개 가능한 관계 정보입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="대상 관계", ToolTip="기존 TargetSelect 공용 의미 타입을 재사용하는 미확인, 아군, 중립 또는 적대 관계입니다."))
	ECFTargetRelation Relation = ECFTargetRelation::Unknown;

	// [v1.0.0] Sensor가 현재 플레이어에게 공개하는 정보 상세 단계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="정보 단계", ToolTip="Sensor Knowledge가 소유하는 Detected, Identified, DetailedScan 정보 단계입니다."))
	ECFTargetInfoLevel InformationLevel = ECFTargetInfoLevel::Detected;

	// [v1.0.0] Sensor Contact 자체의 현재 생명주기 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="Contact 상태", ToolTip="Live, LastKnown, Lost 또는 DestroyedHold 상태입니다. TargetSelect TrackState와 별개입니다."))
	ECFSensorContactState ContactState = ECFSensorContactState::Invalid;

	// [v1.0.0] 마지막으로 신뢰할 수 있었던 대상의 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="마지막 확인 위치", ToolTip="Live 또는 마지막 유효 관측 시점에 신뢰할 수 있었던 위치입니다. LastKnown에서 현재 실제 위치로 추정해 갱신하지 않습니다."))
	FVector LastKnownWorldLocation = FVector::ZeroVector;

	// [v1.0.0] 마지막 유효 관측의 월드 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(Units="s", DisplayName="마지막 관측 시각", ToolTip="마지막으로 Sensor가 유효 관측을 확정한 월드 시간(초)입니다."))
	double LastObservedWorldTimeSeconds = 0.0;

	// [v1.0.0] 마지막 유효 관측 이후 경과한 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(Units="s", DisplayName="정보 경과 시간", ToolTip="마지막 신뢰 관측 이후 경과한 시간입니다. LastKnown freshness 표시와 수명 판정에 사용합니다."))
	float FreshnessSeconds = 0.0f;

	// [v1.0.0] Tactical Analysis의 0~1 정규화 진행률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="분석 진행률", ToolTip="Active Scan/Tactical Analysis의 0~1 진행률입니다. 유효 분석에서 증가하고 가림·범위 이탈·스캔 중단에서는 설정된 감소율로 서서히 감소하며 즉시 초기화하지 않습니다."))
	float AnalysisProgress01 = 0.0f;

	// [v1.0.0] 대상 파괴가 Sensor Runtime에 의해 확인됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Contact", meta=(DisplayName="파괴 확인", ToolTip="True이면 대상 파괴가 확인되어 Contact가 DestroyedHold 수명으로 관리됩니다."))
	bool bDestroyedConfirmed = false;

	// [v1.0.0] 공개 Contact가 Actor-free Snapshot에 들어갈 수 있는 최소 의미 계약을 만족하는지 반환합니다.
	bool IsPublicContractValid() const;
};

/**
 * Sensor Runtime 한 시점의 읽기 전용 공개 Snapshot입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="센서 Snapshot (Sensor Snapshot)", ToolTip="Sensor Runtime 준비 상태, 관측 기준과 Actor-free Contact 목록을 한 번에 전달합니다."))
struct CARFIGHT_RE_API FCFSensorSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] Snapshot이 새로 게시될 때 증가하는 Revision입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="Snapshot Revision", ToolTip="같은 Sensor Runtime에서 Snapshot 내용이 새로 게시될 때 증가하는 Revision입니다."))
	int32 Revision = 0;

	// [v1.0.0] Snapshot을 만든 월드 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(Units="s", DisplayName="Snapshot 시각", ToolTip="이 Snapshot이 만들어진 월드 시간(초)입니다."))
	double SnapshotWorldTimeSeconds = 0.0;

	// [v1.0.0] Snapshot을 만든 Sensor 소유 차량의 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="Sensor 기준 위치", ToolTip="HUD Adapter와 향후 Network 소비자가 Contact의 상대 위치를 계산할 수 있는 Sensor 소유 차량 위치입니다."))
	FVector SensorOriginWorldLocation = FVector::ZeroVector;

	// [v1.0.0] Snapshot을 만든 Sensor 소유 차량의 전방 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="Sensor 전방 방향", ToolTip="Radar 정규화와 방향 계산에 사용할 Sensor 소유 차량의 정규화 전방 방향입니다."))
	FVector SensorForwardWorldDirection = FVector::ForwardVector;

	// [v1.0.0] Sensor Runtime이 현재 유효한 Config로 초기화됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="Sensor Runtime 준비", ToolTip="현재 Sensor Component가 유효한 Config로 초기화되어 Snapshot을 게시할 준비가 됐는지 나타냅니다."))
	bool bRuntimeReady = false;

	// [v1.0.0] Active Scan이 현재 실행 중인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="Active Scan 실행 중", ToolTip="현재 Sensor Runtime에서 Active Scan이 실행 중이면 True입니다. StartActiveScan/StopActiveScan과 같은 Runtime 상태를 게시합니다."))
	bool bActiveScanRunning = false;

	// [v1.0.0] 현재 Sensor가 공개하는 Actor-free Contact 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="Contact 목록", ToolTip="ContactId 순으로 결정적으로 정렬되는 Actor-free 공개 Contact 목록입니다."))
	TArray<FCFSensorContact> Contacts;

	// [v1.0.0] ContactId를 기준으로 공개 Contact 목록을 결정적인 순서로 정렬합니다.
	void SortContactsDeterministically();

	// [v1.0.0] Snapshot ContactId가 모두 유효하고 중복되지 않는지 반환합니다.
	bool HasUniqueContactIds() const;

	// [v1.0.0] Snapshot과 모든 Contact가 공개 데이터 최소 계약을 만족하는지 반환합니다.
	bool IsPublicContractValid() const;
};
