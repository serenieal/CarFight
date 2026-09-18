// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.3
// Date: 2026-09-18
// Description: Phase 6 HUD Provider 독립 채널 Automation 접근 보강
// Scope: Runtime/Blueprint 계약은 변경하지 않고 Phase 6 HUD Provider 회귀가 기존 private deterministic Sensor test seam을 사용할 exact friend만 추가합니다.
// Changelog:
// - v1.13.3: FCFPhase6HUDChannelsTest exact test friend를 추가해 Selection B / Lock A / Scan A Provider 투영을 기존 private Sensor fixture 경로로 검증. Runtime/Blueprint 계약 변경 없음.
// - v1.13.2: CancelTargetScan을 추가해 Target Scan Attempt만 종료하고 broad Active Detection Pulse/Selection/Lock에는 영향을 주지 않는 Phase 5 command boundary를 제공.
// - v1.13.1: FCFTargetingRequestLockTest가 실제 Actor→ContactId bridge와 actor-free Sensor Snapshot을 함께 구성해 RequestLock public 경로를 검증할 수 있도록 exact test friend를 추가. Runtime/Blueprint 계약 변경 없음.
// - v1.13.0: UCFVehicleSensorComp private actor-free Persistent Knowledge Record/Store와 Phase 3B-1 helper 계약을 추가. Identified/DetailedScan/Terminal Knowledge를 Contact lifetime 밖에서 보존하고 Dynamic Knowledge/Rescan은 추가하지 않음.
// - v1.12.0: StartActiveDetectionPulse/StartTargetScan/CancelSensorOperations와 역할 명확한 상태 Getter를 canonical API로 추가하고 기존 ActiveScan 계열 API는 compatibility wrapper로 축소. 내부 전체 update canonical 이름을 RunSensorUpdate로 정리하고 Contact Legacy AnalysisProgress 제거 의미를 반영.
// - v1.11.0: bActiveScanRunning/ActiveScanRemainingSeconds를 broad Active Detection 전용 state로 고정하고 Target Scan에 독립 duration state를 추가해 두 Operation의 동시 실행과 독립 만료를 허용.
// - v1.10.0: 지정 Target 하나만 분석하는 StartTargetedScan, 독립 Scan Attempt 진행 상태와 완료 전이 allocator를 추가하고 Contact Knowledge와 Scanner Runtime을 분리.
// - v1.9.0: 같은 Component 수명에서 DetailedScan 최초 완료 이벤트에 단조 증가 AnalysisCompletionRevision을 발급하는 allocator를 추가.
// - v1.8.1: canonical Basic Sensor 단발 완료 Automation이 기존 SEN-P0-04 deterministic private update seam을 사용할 수 있도록 exact test friend를 추가. Runtime/Blueprint API 노출은 변경하지 않음.
// - v1.8.0: VehicleData 기본 SensorData와 장착 Scanner SensorData를 분리하고 Scanner override > VehicleBase > zero-range Fallback 우선순위를 추가.
// - v1.7.0: Runtime Ready 상태에 RadarDisplayRangePresetsCm/Default index를 Applied copy로 고정하고 read-only Getter를 추가. invalid Radar Profile SensorData는 Config와 함께 fail-closed 거부.
// - v1.6.0: ApplySensorData non-destructive config apply, 적용 Config 사본, range 감소 Contact reconcile과 Active Scan remaining clamp를 추가.
// - v1.5.0: HUD/Target adapter가 Actor truth를 읽지 않고 Snapshot Contact를 찾을 수 있도록 기존 Actor→ContactId read-only bridge를 추가.
// - v1.4.0: VehicleHealthComp 파괴 확정 이벤트/상태를 독립 소비하는 DestroyedHold, 보존 타이머와 안전한 이벤트 해제를 추가.
// - v1.3.0: 입력과 분리된 Start/Stop Active Scan API, ActiveScanRange 기반 장거리 전방향 탐지, Analysis 증가·감소와 Identified/DetailedScan Knowledge 승격을 추가.
// - v1.2.1: SEN-P0-03 의미 변경 없이 누적된 선언·주석 들여쓰기만 정규화.
// - v1.2.0: Sensor update 독립 Contact freshness, Live→LastKnown→Lost→Removed, Lost 최소 1 Snapshot 게시와 Lost 전 동일 Actor 재획득을 추가.
// - v1.1.0: Level Actor cursor 기반 bounded Passive Detection, private weak Actor Contact 저장소, Passive/Visual 탐지, Visibility LOS와 중복 Contact 방지를 추가.
// - v1.0.0: SEN-P0-01 Sensor 독립 소유권, Fallback Config, Snapshot 수명과 ContactId allocator를 최초 추가.
// Migration:
// - v1.13.3은 Automation 접근 허용만 보강하며 Sensor Runtime 동작, persisted schema, Blueprint surface와 Product Asset에는 변경이 없습니다.
// - v1.13.2부터 Target Scan 전용 gameplay 취소는 CancelTargetScan을 사용합니다. CancelSensorOperations는 Active Detection Pulse와 Target Scan을 함께 끄는 broad system/equipment/compatibility cancel 의미를 유지합니다.
// - v1.13.0부터 valid TargetEntityId의 Identified/DetailedScan Knowledge와 authoritative Terminal 이력은 Component private Store가 Contact 제거 뒤에도 보존합니다. ResetSensorRuntime/InitializeSensorRuntime 재호출/EndPlay은 Store를 비우고 ApplySensorData/ApplyVehicleBaseSensorData hot reapply는 Store를 보존합니다.
// - v1.13.0 Persistent Knowledge Store는 Actor/UObject 참조를 저장하지 않으며 Dynamic HP/Loadout/Freshness와 Rescan/Refresh API는 Phase 3B-2까지 추가하지 않습니다.
// - v1.12.0부터 새 코드의 canonical command는 StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations입니다. 기존 StartActiveScan / StartTargetedScan / StopActiveScan은 저장된 Blueprint·기존 C++ caller 호환 wrapper로만 유지합니다.
// - v1.12.0부터 broad Active Detection 상태는 IsActiveDetectionPulseRunning/GetActiveDetectionPulseRemainingSeconds, Target Scan 상태는 IsTargetScanRunning/GetCurrentScanAttemptProgress01을 사용합니다. 기존 상태 Getter는 호환 wrapper입니다.
// - v1.12.0부터 FCFSensorContact에는 AnalysisProgress01이 없습니다. 현재 Scan 진행률은 Snapshot.ScanAttempt.Progress01만 소유합니다.
// - v1.11.0부터 bActiveScanRunning과 ActiveScanRemainingSeconds는 broad Active Detection Pulse만 의미합니다. Target Scan Attempt는 bCurrentScanAttemptActive + CurrentScanAttemptRemainingSeconds를 독립 Authority로 사용합니다.
// - v1.11.0 StartTargetedScan은 broad Active Detection을 켜지 않으며 지정 Target 하나만 ActiveScanRange 안에서 명시적으로 관측·분석합니다. Active Detection Pulse와 Target Scan은 동시에 실행할 수 있습니다.
// - v1.10.0부터 플레이어 분석은 StartTargetedScan(TargetActor)로 단 하나의 Contact만 진행합니다. StartActiveScan은 장거리 Contact Detection 호환 pulse이며 자체로 Contact 분석 진행률을 올리지 않습니다.
// - v1.10.0에서 현재 Scan Attempt 진행률은 Contact가 아니라 Component의 독립 transient 상태가 소유합니다. DetailedScan 완료 즉시 Scanner Runtime은 Idle로 돌아가고 Attempt는 0/inactive로 종료되지만 Contact Knowledge는 유지됩니다.
// - v1.10.0 기본 정책은 DetailedScan 완료 Contact의 즉시 재스캔 거부입니다. 향후 동적 정보 재분석은 별도 Rescan 계약으로 추가합니다.
// - v1.9.0부터 DetailedScan 최초 완료는 Component 수명 단조 증가 AnalysisCompletionRevision으로 식별되며 ResetSensorRuntime이 allocator를 재사용하지 않습니다.
// - v1.3.0에서 Sensor는 InputAction을 소유하지 않습니다. 차량 입력 owner 또는 후속 Adapter가 명시 command API를 호출합니다.
// - ActiveScanRangeCm은 Active Scan 실행 중 장거리 전방향 Contact 탐지에 사용하며, 지정 Scan Attempt 분석 진척은 Live + Active 범위 + ECC_Visibility 직접 가시 조건에서만 증가합니다.
// - Scan Attempt가 LOS/거리 조건을 잃으면 AnalysisDecayPerSec로 현재 Attempt 진행률이 감소하며, 이미 획득한 InformationLevel과 Known Target Knowledge는 강등하지 않습니다.
// - Identified 이상으로 승격될 때만 private SourceDisplayInfo의 TargetId/DisplayName을 KnownTargetId/KnownDisplayName으로 공개합니다. Source InformationLevel은 복사하지 않습니다.
// - P0-03 ContactId, Live/LastKnown/Lost와 Lost 전 same-ID reacquire 계약을 유지합니다.
// - v1.4.0부터 차량 파괴 확정은 UCFVehicleHealthComp::OnVehicleDestroyed/IsDestroyed만 소비하며 Actor Destroy 또는 weak invalid를 파괴 확정으로 해석하지 않습니다.
// - DestroyedHold는 기존 ContactId, Knowledge와 마지막 신뢰 위치를 보존하고 DestroyedHoldTimeSec 만료 뒤 제거됩니다.
// - TargetSelect의 Destroyed 즉시 clear와 Sensor DestroyedHold는 서로 독립입니다.
// - v1.5.0의 Actor→ContactId bridge는 이미 존재하는 Sensor Contact의 ID만 반환하며 Actor metadata/위치/Knowledge를 공개하지 않습니다. 외부 표시 데이터는 반드시 FCFSensorSnapshot에서 읽습니다.
// - v1.6.0부터 Runtime Ready 상태의 SensorData 교체는 ApplySensorData를 사용합니다. 직접 Source 변경은 현재 적용 Config를 즉시 바꾸지 않습니다.
// - ApplySensorData는 ContactId와 획득 Knowledge를 지우지 않습니다. 진행 중 Target Scan Attempt는 Source 변경 시 종료하고, 탐지 능력 감소는 Live Contact를 LastKnown으로 넘겨 기존 lifecycle에서 재획득 또는 Lost 처리합니다.
// - 실행 중 Active Scan은 새 장비 적용으로 남은 시간이 늘어나지 않으며 새 Config가 Active Scan을 지원하지 않으면 기존 Stop 의미로 안전 종료됩니다.
// - v1.7.0 Radar 표시 Range는 Sensor 탐지 판정을 바꾸지 않는 Scanner Profile 데이터입니다. Runtime Ready 이후에는 Source UObject 후속 편집이 아니라 Applied Radar Profile 사본만 UI Provider에 공개합니다.
// - v1.8.0부터 VehicleBaseSensorData는 차량 자체의 기본 Sensor Source이고 SensorData는 장착 Scanner override입니다. Scanner를 제거하면 VehicleBaseSensorData로 복귀하며 둘 다 없을 때만 기존 zero-range Fallback을 사용합니다.
// - v1.8.1은 Automation 접근 허용만 보강하며 Sensor Runtime 동작, persisted schema, Blueprint surface와 Product Asset에는 변경이 없습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "CFSensorTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleSensorComp.generated.h"

class AActor;
class UCFVehicleHealthComp;
class UCFVehicleSensorData;

/**
 * 차량별 Sensor Contact와 Player Knowledge를 TargetSelect와 독립적으로 소유하는 Runtime 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, meta=(BlueprintSpawnableComponent, DisplayName="차량 센서 컴포넌트 (Vehicle Sensor Component)"))
class CARFIGHT_RE_API UCFVehicleSensorComp : public UActorComponent
{
	GENERATED_BODY()

	friend class FCFSensorRuntimeContractTest;
	friend class FCFSensorPassiveRangeTest;
	friend class FCFSensorVisualFallbackTest;
	friend class FCFSensorBoundedScanTest;
	friend class FCFSensorContactLifetimeTest;
	friend class FCFSensorContactReacquireTest;
	friend class FCFSensorPersistentKnowledgeTest;
	friend class FCFSensorActiveRangeTest;
	friend class FCFSensorAnalysisProgressTest;
	friend class FCFSensorAnalysisDecayTest;
	friend class FCFSensorBasicSingleScanCompletionTest;
	friend class FCFSensorOpSplitTest;
	friend class FCFSensorDestroyedHoldTest;
		friend class FCFSensorDestroyedInvalidActorTest;
					friend class FCFSensorHUDSnapshotTest;
	friend class FCFPhase6HUDChannelsTest;
	friend class FCFRadarRangeFoundationTest;
	friend class FCFScannerRuntimeConfigTest;
	friend class FCFTargetingRequestLockTest;



public:
	// [v1.1.0] Sensor update가 필요한 동안만 Tick을 활성화하는 차량 Sensor Component를 생성합니다.
	UCFVehicleSensorComp();

	// [v1.3.0] UpdateIntervalSec마다 Contact 수명, bounded Detection과 Tactical Analysis를 진행합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [v1.0.0] Actor 수명이 종료될 때 Sensor Snapshot과 준비 상태를 안전하게 비웁니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// [v1.13.0] 유효한 Config로 Runtime을 Hard Reinitialize하며 기존 Contact와 Persistent Knowledge Store를 비우고 새 Sensor lifetime을 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Runtime", meta=(DisplayName="센서 런타임 초기화", ToolTip="SensorData 또는 Fallback 설정으로 Sensor Runtime을 새로 초기화합니다. 재호출하면 기존 Contact와 획득 Persistent Knowledge를 모두 비우는 Hard Reinitialize이며, Passive/Visual 탐지 또는 Active Scan 요청이 있을 때 Sensor update가 실행됩니다."))
	bool InitializeSensorRuntime();

	// [v1.12.0] 장착 Scanner SensorData를 적용하거나 None이면 차량 기본 Sensor Source로 복귀합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Config", meta=(DisplayName="장착 스캐너 센서 데이터 적용", ToolTip="장착 Scanner의 VehicleSensorData를 적용합니다. None이면 차량 기본 SensorData로 복귀하고, 기본 SensorData도 없을 때만 zero-range Fallback을 사용합니다. Runtime Ready 상태에서는 Contact ID와 획득 Knowledge를 보존하고 진행 중 Target Scan Attempt는 안전하게 종료합니다."))
	bool ApplySensorData(UCFVehicleSensorData* NewSensorData);

	// [v1.13.0] VehicleData의 기본 Sensor Source를 설정하고 장착 Scanner가 없으면 즉시 적용하며 기존 Contact/Knowledge를 보존합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Config", meta=(DisplayName="차량 기본 센서 데이터 적용", ToolTip="VehicleData가 제공하는 기본 VehicleSensorData를 설정합니다. 장착 Scanner가 있으면 Scanner가 우선하고, Scanner가 없거나 제거되면 이 기본 SensorData를 사용합니다. Runtime Ready 상태에서는 Contact ID와 획득 Persistent Knowledge를 보존하며, None이면 기존 zero-range 안전 Fallback으로 돌아갑니다."))
	bool ApplyVehicleBaseSensorData(UCFVehicleSensorData* NewVehicleBaseSensorData);

	// [v1.13.0] Runtime Contact, Sensor Operation, Persistent Knowledge Store, scan cursor와 공개 Snapshot을 모두 비우는 Hard Reset입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Runtime", meta=(DisplayName="센서 런타임 초기화 해제", ToolTip="현재 Sensor Contact, Sensor Operation, 획득 Persistent Knowledge, scan cursor와 Runtime 준비 상태를 모두 비웁니다. 같은 Component 수명에서 ContactId serial은 재사용하지 않습니다."))
	void ResetSensorRuntime();

	// [v1.12.0] 주변 Contact 탐지 강화를 위한 broad Active Detection Pulse를 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Detection", meta=(DisplayName="능동 탐지 펄스 시작", ToolTip="현재 Sensor Config의 Active Scan 거리와 지속 시간으로 주변 Contact를 장거리 전방향 탐지하는 Active Detection Pulse를 시작합니다. Target Scan이나 Knowledge 분석은 시작하지 않습니다."))
	bool StartActiveDetectionPulse();

	// [v1.12.0] 지정 Target 하나의 Knowledge 분석용 Target Scan Attempt를 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|TargetScan", meta=(DisplayName="타겟 스캔 시작", ToolTip="지정 Actor 하나만 Target Scan Attempt 대상으로 시작합니다. broad Active Detection Pulse를 자동 시작하지 않으며 이미 DetailedScan 완료된 대상은 기본 정책상 재스캔하지 않습니다."))
	bool StartTargetScan(AActor* TargetActor);

	// [v1.13.2] 현재 Target Scan Attempt만 명시적으로 취소하고 broad Active Detection Pulse는 유지합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|TargetScan", meta=(DisplayName="타겟 스캔 취소", ToolTip="현재 진행 중인 Target Scan Attempt만 즉시 취소합니다. Active Detection Pulse, Contact와 획득 Knowledge는 유지됩니다."))
	bool CancelTargetScan();

	// [v1.12.0] 현재 실행 중인 Sensor Operation들을 명시적으로 취소합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Runtime", meta=(DisplayName="센서 작업 취소", ToolTip="현재 실행 중인 Active Detection Pulse와 Target Scan Attempt를 모두 즉시 취소합니다. Contact와 획득 Knowledge는 유지됩니다."))
	bool CancelSensorOperations();

	// [v1.12.0] 기존 Blueprint/C++ caller 호환용 Active Scan 시작 wrapper입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Legacy", meta=(DisplayName="Legacy Active Scan 시작", ToolTip="호환용 API입니다. 새 코드에서는 능동 탐지 펄스 시작(StartActiveDetectionPulse)을 사용합니다.", DeprecatedFunction, DeprecationMessage="StartActiveDetectionPulse를 사용하세요."))
	bool StartActiveScan();

	// [v1.12.0] 기존 Blueprint/C++ caller 호환용 Targeted Scan 시작 wrapper입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Legacy", meta=(DisplayName="Legacy Targeted Scan 시작", ToolTip="호환용 API입니다. 새 코드에서는 타겟 스캔 시작(StartTargetScan)을 사용합니다.", DeprecatedFunction, DeprecationMessage="StartTargetScan을 사용하세요."))
	bool StartTargetedScan(AActor* TargetActor);

	// [v1.12.0] 기존 Blueprint/C++ caller 호환용 전체 Sensor Operation 취소 wrapper입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Legacy", meta=(DisplayName="Legacy Active Scan 중단", ToolTip="호환용 API입니다. 새 코드에서는 센서 작업 취소(CancelSensorOperations)를 사용합니다.", DeprecatedFunction, DeprecationMessage="CancelSensorOperations를 사용하세요."))
	bool StopActiveScan();

					// [v1.6.0] Runtime Ready이면 실제 적용된 Sensor Config를, 초기화 전이면 현재 Source 또는 Fallback 설정을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Config", meta=(DisplayName="해석된 센서 설정 반환", ToolTip="Runtime Ready 상태에서는 ApplySensorData 또는 초기화로 실제 적용된 SensorConfig를 반환합니다. 초기화 전에는 현재 전체 SensorData 계약이 유효하면 해당 설정을, 아니면 FallbackSensorConfig를 반환합니다."))
	FCFSensorConfig GetResolvedSensorConfig() const;

	// [v1.7.0] Runtime에 실제 적용된 Scanner Radar 표시 범위 Preset 사본을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Radar", meta=(DisplayName="해석된 Radar 표시 범위 프리셋 반환", ToolTip="Runtime Ready 상태에서는 실제 적용된 Scanner Profile의 Radar 표시 범위 Preset 사본을 반환합니다. Scanner-less Fallback 또는 미설정 Profile이면 빈 배열입니다."))
	TArray<float> GetResolvedRadarDisplayRangePresetsCm() const;

	// [v1.7.0] Runtime에 실제 적용된 Scanner Radar 기본 표시 범위 Preset 인덱스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Radar", meta=(DisplayName="해석된 기본 Radar 범위 인덱스 반환", ToolTip="Runtime Ready 상태에서 실제 적용된 Scanner Profile의 기본 Radar 표시 범위 0-based 인덱스를 반환합니다. Profile이 없으면 -1입니다."))
	int32 GetResolvedDefaultRadarDisplayRangePresetIndex() const;

		// [v1.0.0] 현재 Actor-free Sensor Snapshot 사본을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="센서 Snapshot 반환", ToolTip="현재 Sensor Runtime이 게시한 Actor 포인터 없는 Snapshot 사본을 반환합니다."))
	FCFSensorSnapshot GetSensorSnapshot() const;

	// [v1.5.0] 이미 존재하는 Sensor Contact와 지정 Actor의 연결을 ContactId만으로 읽고 Actor truth나 private Runtime data는 공개하지 않습니다.
	bool TryGetContactIdForActor(const AActor* TargetActor, FName& OutContactId) const;


	// [v1.0.0] 현재 Sensor Runtime이 유효한 Config로 초기화됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Runtime", meta=(DisplayName="센서 런타임 준비 여부", ToolTip="유효한 SensorData 또는 Fallback Config로 Sensor Runtime이 초기화됐으면 True입니다."))
	bool IsSensorRuntimeReady() const { return bSensorRuntimeReady; }

	// [v1.12.0] broad Active Detection Pulse가 현재 실행 중인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Detection", meta=(DisplayName="능동 탐지 펄스 실행 여부", ToolTip="주변 Contact를 장거리 전방향 탐지하는 Active Detection Pulse가 실행 중이면 True입니다. Target Scan Attempt는 별도 상태입니다."))
	bool IsActiveDetectionPulseRunning() const { return bActiveScanRunning; }

	// [v1.12.0] Target Scan Attempt가 실제 진행 중인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|TargetScan", meta=(DisplayName="타겟 스캔 진행 여부", ToolTip="현재 단 하나의 Target Scan Attempt가 진행 중이면 True입니다. DetailedScan 완료 또는 명시 취소 즉시 False로 돌아갑니다."))
	bool IsTargetScanRunning() const { return bCurrentScanAttemptActive; }

	// [v1.12.0] 기존 broad Active Scan 상태 Getter 호환 wrapper입니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Legacy", meta=(DisplayName="Legacy Active Scan 실행 여부", ToolTip="호환용 Getter입니다. 새 코드에서는 능동 탐지 펄스 실행 여부를 사용합니다.", DeprecatedFunction, DeprecationMessage="IsActiveDetectionPulseRunning을 사용하세요."))
	bool IsActiveScanRunning() const { return IsActiveDetectionPulseRunning(); }

	// [v1.12.0] 기존 Targeted Scan 상태 Getter 호환 wrapper입니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Legacy", meta=(DisplayName="Legacy Targeted Scan 진행 여부", ToolTip="호환용 Getter입니다. 새 코드에서는 타겟 스캔 진행 여부를 사용합니다.", DeprecatedFunction, DeprecationMessage="IsTargetScanRunning을 사용하세요."))
	bool IsTargetedScanRunning() const { return IsTargetScanRunning(); }

	// [v1.10.0] 현재 지정 Target Scan Attempt의 0~1 진행률을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|ScanAttempt", meta=(DisplayName="현재 타겟 스캔 진행률", ToolTip="현재 지정 Target Scan Attempt의 0~1 진행률입니다. Attempt가 비활성이면 0입니다."))
	float GetCurrentScanAttemptProgress01() const { return bCurrentScanAttemptActive ? CurrentScanAttemptProgress01 : 0.0f; }

	// [v1.12.0] broad Active Detection Pulse에 남은 실행 시간을 초 단위로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Detection", meta=(DisplayName="능동 탐지 펄스 남은 시간", ToolTip="현재 Active Detection Pulse에 남은 실행 시간입니다. Target Scan Attempt의 남은 시간은 포함하지 않으며 Pulse가 실행 중이 아니면 0입니다."))
	float GetActiveDetectionPulseRemainingSeconds() const { return ActiveScanRemainingSeconds; }

	// [v1.12.0] 기존 Active Scan 남은 시간 Getter 호환 wrapper입니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Legacy", meta=(DisplayName="Legacy Active Scan 남은 시간", ToolTip="호환용 Getter입니다. 새 코드에서는 능동 탐지 펄스 남은 시간을 사용합니다.", DeprecatedFunction, DeprecationMessage="GetActiveDetectionPulseRemainingSeconds를 사용하세요."))
	float GetActiveScanRemainingSeconds() const { return GetActiveDetectionPulseRemainingSeconds(); }

	// [v1.0.0] 마지막 Sensor Runtime 초기화·Reset 결과 요약을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Debug", meta=(DisplayName="센서 런타임 요약 반환", ToolTip="마지막 Sensor Runtime 초기화 또는 Reset 결과와 사용한 Config 출처를 한 줄 문자열로 반환합니다."))
	FString GetLastSensorRuntimeSummary() const { return LastSensorRuntimeSummary; }

	// [v1.1.0] 가장 최근 bounded Sensor update에서 검사한 Actor 슬롯 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Debug", meta=(DisplayName="최근 Sensor Actor 검사 수", ToolTip="가장 최근 bounded Sensor Detection update에서 실제 검사한 Level Actor 슬롯 수입니다. MaxActorScansPerUpdate를 넘지 않습니다."))
	int32 GetLastPassiveScanActorCount() const { return LastPassiveScanActorCount; }

	// [v1.1.0] 가장 최근 Sensor update에서 Visual fallback을 위해 실행한 ECC_Visibility Trace 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Debug", meta=(DisplayName="최근 Passive Visibility Trace 수", ToolTip="Passive 거리 밖이지만 VisualDetectionRangeCm 안인 대상의 직접 가시성을 확인하기 위해 실행한 ECC_Visibility Trace 수입니다."))
	int32 GetLastPassiveVisibilityTraceCount() const { return LastPassiveVisibilityTraceCount; }

	// [v1.3.0] 가장 최근 Sensor update에서 Tactical Analysis 직접 가시성을 확인한 ECC_Visibility Trace 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Debug", meta=(DisplayName="최근 Active Analysis Trace 수", ToolTip="Active Scan Tactical Analysis 진행 가능 여부를 확인하기 위해 실행한 ECC_Visibility Trace 수입니다. TargetSelect Trace Channel을 사용하지 않습니다."))
	int32 GetLastActiveAnalysisTraceCount() const { return LastActiveAnalysisTraceCount; }

	// [v1.1.0] bounded cursor가 전체 Level Actor 배열을 한 바퀴 완료한 누적 횟수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Debug", meta=(DisplayName="Sensor 전체 순회 횟수", ToolTip="bounded Sensor cursor가 현재 World의 Level Actor 배열 끝까지 진행해 첫 위치로 돌아온 횟수입니다."))
	int32 GetPassiveScanCycleSerial() const { return PassiveScanCycleSerial; }

	// [v1.8.0] 장착 Scanner가 제공하는 override Source이며 None이면 차량 기본 Sensor Source로 복귀합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Config", meta=(DisplayName="장착 스캐너 센서 데이터", ToolTip="장착 Scanner가 제공하는 VehicleSensorData override입니다. 유효하면 차량 기본 SensorData보다 우선하며, None이면 VehicleData의 기본 SensorData 또는 zero-range Fallback을 사용합니다."))
	TObjectPtr<UCFVehicleSensorData> SensorData = nullptr;

	// [v1.1.0] SensorData가 없거나 유효하지 않을 때 사용할 안전한 Runtime 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Config", meta=(DisplayName="기본 센서 설정", ToolTip="SensorData가 없거나 유효하지 않을 때 사용합니다. 거리와 Active Scan 기본값 0은 실제 게임 Sensor 튜닝 수치를 아직 확정하지 않은 비활성 상태입니다."))
	FCFSensorConfig FallbackSensorConfig;

private:
	/**
	 * Actor 참조와 Source Metadata를 외부 Snapshot에서 분리해 보관하는 Sensor 전용 Runtime Contact입니다.
	 */
	struct FCFSensorContactRuntime
	{
		// [v1.1.0] 같은 Actor 중복 Contact를 막고 후속 lifecycle 이벤트를 연결할 내부 약한 참조입니다.
		TWeakObjectPtr<AActor> TargetActor;

		// [v1.1.0] ICFTargetSelectable에서 읽은 원본 Metadata이며 Player Knowledge로 자동 공개하지 않습니다.
		FCFTargetDisplayInfo SourceDisplayInfo;

		// [v1.1.0] Actor/UObject pointer 없이 외부에 게시할 현재 공개 Contact 데이터입니다.
		FCFSensorContact PublicContact;

		// [v1.2.0] Lost 상태가 최소 한 번 Public Snapshot에 포함됐는지 기록해 다음 update cleanup 시점을 결정합니다.
		bool bLostSnapshotPublished = false;

		// [v1.3.0] 마지막 유효 관측이 Active Scan이 없어도 Passive/Visual 계약으로 유지될 수 있었는지 기록합니다.
		bool bBaselineDetectionValidAtLastObservation = false;

		// [v1.4.0] 이 Contact의 authoritative 파괴 확정을 구독한 VehicleHealthComp입니다.
		TWeakObjectPtr<UCFVehicleHealthComp> BoundVehicleHealthComponent;

		// [v1.4.0] DestroyedHold 수명 시작점으로 사용할 파괴 확정 월드 시간입니다.
		double DestroyedConfirmedWorldTimeSeconds = 0.0;
	};

	/**
	 * [v1.13.0] Contact/Actor lifetime과 독립적으로 TargetEntityId에 귀속된 Persistent Knowledge 값만 보존하는 actor-free Record입니다.
	 */
	struct FCFSensorKnowledgeRecord
	{
		// [v1.13.0] 이 Knowledge Record를 유일하게 식별하는 Gameplay Entity Identity입니다.
		FGuid TargetEntityId;

		// [v1.13.0] 같은 Entity 재등장 시 concrete mismatch를 검출할 private Source TargetId입니다.
		FName StableTargetId = NAME_None;

		// [v1.13.0] 같은 Entity lifetime에서 안정적이라고 확정된 Target 큰 분류입니다.
		ECFTargetCategory StableTargetCategory = ECFTargetCategory::Unknown;

		// [v1.13.0] Identified 이상에서 플레이어에게 공개된 TargetId Knowledge입니다.
		FName KnownTargetId = NAME_None;

		// [v1.13.0] Identified 이상에서 플레이어에게 공개된 표시 이름이며 identity consistency key로 사용하지 않습니다.
		FText KnownDisplayName;

		// [v1.13.0] 동일 Entity에서 획득한 최고 단계형 Knowledge입니다.
		ECFTargetInfoLevel HighestKnowledgeTier = ECFTargetInfoLevel::Detected;

		// [v1.13.0] 최초 DetailedScan 획득 때 발급된 양수 AnalysisCompletionRevision을 exact 보존합니다.
		int32 DetailedScanAnalysisRevision = 0;

		// [v1.13.0] authoritative destruction으로 해당 Gameplay Entity lifetime이 종료됐는지 기록합니다.
		bool bTerminalDestroyed = false;
	};

	// [v1.0.0] 현재 Sensor Runtime이 유효한 Config로 준비됐는지 여부입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|Runtime", meta=(AllowPrivateAccess="true", DisplayName="센서 런타임 준비 상태"))
	bool bSensorRuntimeReady = false;

	// [v1.11.0] broad Active Detection Pulse가 현재 실행 중인지 나타냅니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|ActiveScan", meta=(AllowPrivateAccess="true", DisplayName="Active Scan 실행 상태"))
	bool bActiveScanRunning = false;

	// [v1.11.0] broad Active Detection Pulse에 남은 실행 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|ActiveScan", meta=(AllowPrivateAccess="true", Units="s", DisplayName="Active Scan 남은 시간"))
	float ActiveScanRemainingSeconds = 0.0f;

	// [v1.10.0] 현재 지정 Target Scan Attempt가 실제 진행 중인지 나타냅니다.
	bool bCurrentScanAttemptActive = false;

	// [v1.11.0] broad Active Detection Pulse와 독립적으로 현재 Target Scan Attempt에 남은 실행 시간입니다.
	float CurrentScanAttemptRemainingSeconds = 0.0f;

	// [v1.10.0] 현재 Scan Attempt가 분석하는 private Target Actor 약한 참조입니다.
	TWeakObjectPtr<AActor> CurrentScanTargetActor;

	// [v1.10.0] 현재 Scan Attempt가 연결된 안정 ContactId입니다.
	FName CurrentScanTargetContactId = NAME_None;

	// [v1.10.0] 현재 Scan Attempt 자체의 0~1 진행률입니다.
	float CurrentScanAttemptProgress01 = 0.0f;

	// [v1.10.0] 가장 최근 완료 전이가 발생한 ContactId입니다.
	FName LastCompletedScanContactId = NAME_None;

	// [v1.10.0] 가장 최근 지정 Scan Attempt 완료 전이 Revision입니다.
	int32 LastScanCompletionTransitionRevision = 0;

	// [v1.0.0] 외부 소비자에게 공개할 현재 Actor-free Snapshot입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(AllowPrivateAccess="true", DisplayName="현재 센서 Snapshot"))
	FCFSensorSnapshot CurrentSensorSnapshot;

		// [v1.0.0] 마지막 Sensor Runtime 초기화 또는 Reset 결과 요약입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 센서 런타임 요약"))
	FString LastSensorRuntimeSummary = TEXT("SensorRuntime: NotInitialized");

	// [v1.8.0] VehicleData가 제공하는 차량 자체의 기본 Sensor Source입니다. 장착 Scanner가 있으면 우선순위에서 밀립니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|Config", meta=(AllowPrivateAccess="true", DisplayName="차량 기본 센서 데이터", ToolTip="현재 VehicleData가 제공한 기본 VehicleSensorData입니다. 장착 Scanner가 없을 때 사용하며, 둘 다 없으면 zero-range Fallback을 사용합니다."))
	TObjectPtr<UCFVehicleSensorData> VehicleBaseSensorData = nullptr;

	// [v1.6.0] Runtime Ready 상태에서 Source UObject의 후속 변경과 분리해 실제 Sensor가 소비하는 적용 Config 사본입니다.
	FCFSensorConfig AppliedSensorConfig;

	// [v1.7.0] Runtime Ready 상태에서 Source UObject 후속 편집과 분리한 Radar 표시 Range Preset 적용 사본입니다.
	TArray<float> AppliedRadarDisplayRangePresetsCm;

	// [v1.7.0] AppliedRadarDisplayRangePresetsCm에서 초기 UI 표시 범위를 지시하는 적용 기본 인덱스입니다.
	int32 AppliedDefaultRadarDisplayRangePresetIndex = INDEX_NONE;

	// [v1.6.0] AppliedSensorConfig가 현재 Runtime에 실제 적용된 유효 설정인지 여부입니다.
	bool bHasAppliedSensorConfig = false;

	// [v1.1.0] Actor가 포함될 수 있어 공개 Snapshot과 분리하는 Sensor private Runtime Contact 목록입니다.
	TArray<FCFSensorContactRuntime> RuntimeContacts;

	// [v1.13.0] valid TargetEntityId를 exact key로 사용하고 Actor/UObject 참조 없이 획득 Knowledge를 Contact lifetime 밖에서 보존합니다.
	TMap<FGuid, FCFSensorKnowledgeRecord> PersistentKnowledgeStore;

	// [v1.0.0] 같은 Component 수명에서 새 Contact에 사용할 증가형 serial입니다.
	int32 NextContactSerial = 1;

	// [v1.9.0] 같은 Component 수명에서 DetailedScan 최초 Knowledge 획득 이벤트에 사용할 증가형 Revision입니다.
	int32 NextAnalysisCompletionRevision = 1;

	// [v1.10.0] 같은 Component 수명에서 지정 Scan Attempt 완료 전이에 사용할 증가형 Revision입니다.
	int32 NextScanCompletionTransitionRevision = 1;

	// [v1.0.0] 같은 Component 수명에서 새 Snapshot에 사용할 증가형 Revision입니다.
	int32 NextSnapshotRevision = 1;

	// [v1.1.0] UpdateIntervalSec까지 누적할 Sensor Detection 경과 시간입니다.
	float PassiveUpdateElapsedSeconds = 0.0f;

	// [v1.1.0] 다음 bounded update가 이어서 읽을 World Level 배열 인덱스입니다.
	int32 PassiveScanLevelIndex = 0;

	// [v1.1.0] 현재 Level에서 다음 bounded update가 이어서 읽을 Actor 슬롯 인덱스입니다.
	int32 PassiveScanActorIndex = 0;

	// [v1.1.0] cursor가 전체 World Level Actor 배열을 끝까지 순회한 누적 횟수입니다.
	int32 PassiveScanCycleSerial = 0;

	// [v1.1.0] 가장 최근 update에서 예산을 소비해 검사한 Actor 슬롯 수입니다.
	int32 LastPassiveScanActorCount = 0;

	// [v1.1.0] 가장 최근 update에서 Visual fallback 때문에 실행한 Visibility Trace 수입니다.
	int32 LastPassiveVisibilityTraceCount = 0;

	// [v1.3.0] 가장 최근 update에서 Active Tactical Analysis 때문에 실행한 Visibility Trace 수입니다.
	int32 LastActiveAnalysisTraceCount = 0;

		// [v1.0.0] TargetId와 독립된 새 ContactId를 증가형 serial로 발급합니다.
	FName AllocateContactId();

	// [v1.8.0] Scanner override > VehicleBase 순서로 현재 유효한 SensorData Source를 반환하고 없으면 nullptr을 반환합니다.
	const UCFVehicleSensorData* ResolveConfiguredSensorData() const;

	// [v1.8.0] Scanner override > VehicleBase > Fallback 순서로 실제 Sensor Config를 반환합니다.
	FCFSensorConfig ResolveConfiguredSensorConfig() const;

	// [v1.1.0] Passive 또는 Visual 탐지 거리가 실제로 설정돼 기본 bounded update가 필요한지 반환합니다.
	bool HasConfiguredPassiveWork(const FCFSensorConfig& SensorConfig) const;

	// [v1.3.0] 현재 Active Scan 실행 상태까지 포함해 이번 update에 bounded World Detection이 필요한지 반환합니다.
	bool HasConfiguredDetectionWork(const FCFSensorConfig& SensorConfig) const;

	// [v1.12.0] Contact lifecycle, bounded Detection, Target Scan progression, Operation duration과 Snapshot을 한 번 갱신하는 전체 Sensor update입니다.
	void RunSensorUpdate(float SensorUpdateDeltaSeconds = -1.0f);

	// [v1.12.0] 기존 Automation/private caller 호환용 update wrapper입니다.
	void RunPassiveDetectionUpdate(float SensorUpdateDeltaSeconds = -1.0f);

	// [v1.11.0] 하나의 Actor를 Sensor 자격·대표 위치·Passive/Visual/Active 거리 순서로 평가하고 필요 시 지정 Target Scan의 단일 Active-range 관측만 허용해 Runtime Contact를 갱신합니다.
	bool ProcessPassiveScanActor(AActor* CandidateActor, const FCFSensorConfig& SensorConfig, bool bAllowTargetScanActiveRange = false);

	// [v1.1.0] ICFTargetSelectable 대상 자격만 재사용해 Sensor 후보로 사용할 수 있는지 반환합니다.
	bool IsSensorCandidateEligible(AActor* CandidateActor) const;

	// [v1.1.0] ICFTargetSelectable의 BlueprintNativeEvent 안전 디스패치로 대표 위치를 읽습니다.
	FVector ResolveSensorTargetLocation(AActor* CandidateActor) const;

	// [v1.1.0] ICFTargetSelectable의 Source Metadata를 읽되 InformationLevel을 Player Knowledge로 자동 승격하지 않습니다.
	FCFTargetDisplayInfo ResolveSensorSourceDisplayInfo(AActor* CandidateActor) const;

	// [v1.3.0] Owner→Target 대표 위치를 ECC_Visibility로 확인하고 호출 목적에 맞는 Trace 진단값을 증가시킵니다.
	bool HasDirectSensorVisibility(AActor* CandidateActor, const FVector& TargetWorldLocation, bool bActiveAnalysisTrace = false);

		// [v1.3.0] 같은 Actor Runtime Contact를 갱신하거나 새 ContactId를 한 번만 발급하고 마지막 관측이 baseline 탐지로 유지 가능한지도 기록합니다.
	bool UpsertLiveContact(AActor* CandidateActor, const FCFTargetDisplayInfo& SourceDisplayInfo, const FVector& TargetWorldLocation, bool bBaselineDetectionValidAtObservation);

	// [v1.4.0] 기존 Runtime Contact의 VehicleHealth 파괴 이벤트 구독을 현재 Actor 컴포넌트에 맞춰 연결합니다.
	void BindContactDestroyedEvent(FCFSensorContactRuntime& RuntimeContact);

	// [v1.4.0] Runtime Contact가 구독 중인 VehicleHealth 파괴 이벤트를 안전하게 해제합니다.
	void UnbindContactDestroyedEvent(FCFSensorContactRuntime& RuntimeContact);

	// [v1.4.0] Reset 또는 재초기화 전에 모든 Runtime Contact의 VehicleHealth 파괴 이벤트를 안전하게 해제합니다.
	void UnbindAllContactDestroyedEvents();

	// [v1.4.0] authoritative VehicleHealth가 파괴 상태인 기존 Contact만 DestroyedHold로 전환합니다.
	bool ConfirmDestroyedContactForActor(AActor* CandidateActor);

	// [v1.4.0] VehicleHealthComp의 최초 파괴 이벤트를 받아 기존 Sensor Contact를 즉시 DestroyedHold로 전환합니다.
	UFUNCTION()
	void HandleObservedVehicleDestroyed(FCFDamageHitContext DamageHitContext);


	// [v1.2.0] 이번 후보 평가에서 탐지 조건을 잃은 기존 Live Contact를 LastKnown으로 전환하되 마지막 신뢰 위치·관측 시각은 변경하지 않습니다.
	bool MarkContactNotObservedForActor(const AActor* CandidateActor);

		// [v1.6.0] Passive/Visual 탐지 능력 감소 시 기존 Live Contact를 삭제하지 않고 LastKnown lifecycle로 넘깁니다.
	void MarkAllLiveContactsLastKnown();

	// [v1.3.0] Active Scan 종료 시 마지막 관측이 Active-only였던 Live Contact를 LastKnown으로 전환합니다.
	void MarkActiveOnlyContactsLastKnown();

		// [v1.4.0] 모든 Runtime Contact의 Freshness, LastKnown→Lost→Removed와 DestroyedHold 만료 수명을 bounded cursor와 독립적으로 현재 Sensor update 시각까지 진행합니다.
	void AdvanceContactLifetimes(double CurrentWorldTimeSeconds, const FCFSensorConfig& SensorConfig);

	// [v1.11.0] 현재 지정 Target Scan Attempt만 Target Scan 유효 시간에는 증가시키고 LOS/거리 조건 상실에서는 설정된 감소율로 감소시킵니다.
	void AdvanceCurrentScanAttempt(float SensorUpdateDeltaSeconds, float TargetScanTimeThisUpdate, const FCFSensorConfig& SensorConfig);

	// [v1.10.0] 현재 지정 Scan Attempt의 Target Contact가 분석 증가 조건을 만족하는지 판정합니다.
	bool IsCurrentScanAttemptValidForAnalysis(FCFSensorContactRuntime& RuntimeContact, const FCFSensorConfig& SensorConfig);

	// [v1.12.0] Target Scan Attempt 진행률로 Contact Knowledge를 단방향 승격하고 Identified 이상에서만 TargetId/Name을 공개합니다.
	void PromoteContactKnowledgeFromScanProgress(FCFSensorContactRuntime& RuntimeContact, float ScanAttemptProgress01, const FCFSensorConfig& SensorConfig);

	// [v1.13.0] 기존 Persistent Knowledge Record와 현재 Source stable identity가 함께 concrete일 때 서로 모순되지 않는지 판정합니다.
	bool IsPersistentKnowledgeIdentityCompatible(const FCFSensorKnowledgeRecord& KnowledgeRecord, const FCFTargetDisplayInfo& SourceDisplayInfo) const;

	// [v1.13.0] None/Unknown이던 Stable Identity를 현재 concrete Source Metadata로 최초 보강하되 이미 concrete인 값은 바꾸지 않습니다.
	void FillPersistentKnowledgeIdentity(FCFSensorKnowledgeRecord& KnowledgeRecord, const FCFTargetDisplayInfo& SourceDisplayInfo);

	// [v1.13.0] 기존 Store Knowledge를 새 Contact Projection에 복원하되 Scan Attempt/완료 전이와 allocator는 변경하지 않습니다.
	void RestorePersistentKnowledgeToContact(FCFSensorContactRuntime& RuntimeContact, const FCFSensorKnowledgeRecord& KnowledgeRecord);

	// [v1.13.0] Identified/DetailedScan 또는 Terminal 사건을 valid TargetEntityId Record에 즉시 반영하고 identity conflict에서는 fail-closed합니다.
	bool UpsertPersistentKnowledgeRecord(const FGuid& TargetEntityId, const FCFTargetDisplayInfo& SourceDisplayInfo, const FCFSensorContact* PublicContact, bool bTerminalDestroyed);

	// [v1.11.0] Target Scan만 유지하던 현재 Contact가 다른 Detection으로 유지되지 않는 경우 LastKnown으로 전환합니다.
	void MarkCurrentScanTargetLastKnownIfNeeded();

	// [v1.11.0] 현재 지정 Scan Attempt를 Idle/0 상태로 되돌리되 Contact Knowledge와 완료 전이 이력은 유지합니다.
	void ResetCurrentScanAttempt();

	// [v1.11.0] DetailedScan 완료 전이를 정확히 한 번 기록하고 Target Scan Attempt만 즉시 Idle로 복귀시킵니다.
	void CompleteCurrentScanAttempt(FCFSensorContactRuntime& RuntimeContact, const FCFSensorConfig& SensorConfig);

	// [v1.11.0] 이번 Sensor update가 소비한 Target Scan 시간을 반영하고 만료되면 Target Scan Attempt만 종료합니다.
	void AdvanceCurrentScanDuration(float TargetScanTimeThisUpdate, const FCFSensorConfig& SensorConfig);

	// [v1.11.0] 이번 Sensor update가 소비한 broad Active Detection 시간을 반영하고 만료되면 active-only Contact를 LastKnown으로 전환합니다.
	void AdvanceActiveScanDuration(float ActiveScanTimeThisUpdate, const FCFSensorConfig& SensorConfig);

	// [v1.1.0] 같은 Actor에 이미 연결된 Runtime Contact 인덱스를 찾고 없으면 INDEX_NONE을 반환합니다.
	int32 FindRuntimeContactIndexByActor(const AActor* CandidateActor) const;

	// [v1.3.0] Runtime Contact의 Actor-free PublicContact만 복사해 결정 정렬된 Snapshot을 게시하고 현재 Active Scan 상태를 함께 공개합니다.
	void PublishRuntimeSnapshot();

	// [v1.1.0] bounded world scan cursor와 진단값을 초기 위치로 되돌립니다.
	void ResetPassiveScanCursor();

	// [v1.1.0] 다음 World Level로 cursor를 이동하고 끝에서 처음으로 돌아오면 cycle serial을 증가시킵니다.
	void AdvancePassiveScanLevel(int32 LevelCount);

	// [v1.3.0] 현재 Owner Transform과 Runtime 준비·Active Scan 상태로 Contact가 없는 Foundation Snapshot을 다시 만듭니다.
	void RebuildFoundationSnapshot(bool bRuntimeReady);
};