// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.6.0
// Date: 2026-08-16
// Description: CF-FQ-037 차량 Scanner Runtime Config Apply / SCAN-P0-02
// Scope: Sensor 설정 해석, non-destructive SensorData 적용, private Runtime Contact, bounded Passive/Active Detection, Contact 수명, DestroyedHold, Tactical Analysis와 Actor-free Snapshot 소유권을 제공합니다.
// Changelog:
// - v1.6.0: ApplySensorData non-destructive config apply, 적용 Config 사본, range 감소 Contact reconcile과 Active Scan remaining clamp를 추가.
// - v1.5.0: HUD/Target adapter가 Actor truth를 읽지 않고 Snapshot Contact를 찾을 수 있도록 기존 Actor→ContactId read-only bridge를 추가.
// - v1.4.0: VehicleHealthComp 파괴 확정 이벤트/상태를 독립 소비하는 DestroyedHold, 보존 타이머와 안전한 이벤트 해제를 추가.
// - v1.3.0: 입력과 분리된 Start/Stop Active Scan API, ActiveScanRange 기반 장거리 전방향 탐지, Analysis 증가·감소와 Identified/DetailedScan Knowledge 승격을 추가.
// - v1.2.1: SEN-P0-03 의미 변경 없이 누적된 선언·주석 들여쓰기만 정규화.
// - v1.2.0: Sensor update 독립 Contact freshness, Live→LastKnown→Lost→Removed, Lost 최소 1 Snapshot 게시와 Lost 전 동일 Actor 재획득을 추가.
// - v1.1.0: Level Actor cursor 기반 bounded Passive Detection, private weak Actor Contact 저장소, Passive/Visual 탐지, Visibility LOS와 중복 Contact 방지를 추가.
// - v1.0.0: SEN-P0-01 Sensor 독립 소유권, Fallback Config, Snapshot 수명과 ContactId allocator를 최초 추가.
// Migration:
// - v1.3.0에서 Sensor는 InputAction을 소유하지 않습니다. 차량 입력 owner 또는 후속 Adapter가 StartActiveScan/StopActiveScan만 호출합니다.
// - ActiveScanRangeCm은 Active Scan 실행 중에만 장거리 전방향 Contact 탐지에 사용하며, 분석 진척은 Live + Active 범위 + ECC_Visibility 직접 가시 조건에서만 증가합니다.
// - Active Scan/Analysis가 유효하지 않으면 AnalysisProgress01은 AnalysisDecayPerSec로 감소하지만 즉시 초기화하지 않고, 획득한 InformationLevel과 Known Target Knowledge는 강등하지 않습니다.
// - Identified 이상으로 승격될 때만 private SourceDisplayInfo의 TargetId/DisplayName을 KnownTargetId/KnownDisplayName으로 공개합니다. Source InformationLevel은 복사하지 않습니다.
// - P0-03 ContactId, Live/LastKnown/Lost와 Lost 전 same-ID reacquire 계약을 유지합니다.
// - v1.4.0부터 차량 파괴 확정은 UCFVehicleHealthComp::OnVehicleDestroyed/IsDestroyed만 소비하며 Actor Destroy 또는 weak invalid를 파괴 확정으로 해석하지 않습니다.
// - DestroyedHold는 기존 ContactId, Knowledge, AnalysisProgress와 마지막 신뢰 위치를 보존하고 DestroyedHoldTimeSec 만료 뒤 제거됩니다.
// - TargetSelect의 Destroyed 즉시 clear와 Sensor DestroyedHold는 서로 독립입니다.
// - v1.5.0의 Actor→ContactId bridge는 이미 존재하는 Sensor Contact의 ID만 반환하며 Actor metadata/위치/Knowledge를 공개하지 않습니다. 외부 표시 데이터는 반드시 FCFSensorSnapshot에서 읽습니다.
// - v1.6.0부터 Runtime Ready 상태의 SensorData 교체는 ApplySensorData를 사용합니다. 직접 Source 변경은 현재 적용 Config를 즉시 바꾸지 않습니다.
// - ApplySensorData는 ContactId, AnalysisProgress와 획득 Knowledge를 지우지 않습니다. 탐지 능력 감소는 Live Contact를 LastKnown으로 넘겨 기존 lifecycle에서 재획득 또는 Lost 처리합니다.
// - 실행 중 Active Scan은 새 장비 적용으로 남은 시간이 늘어나지 않으며 새 Config가 Active Scan을 지원하지 않으면 기존 Stop 의미로 안전 종료됩니다.

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
	friend class FCFSensorActiveRangeTest;
	friend class FCFSensorAnalysisProgressTest;
		friend class FCFSensorAnalysisDecayTest;
	friend class FCFSensorDestroyedHoldTest;
		friend class FCFSensorDestroyedInvalidActorTest;
		friend class FCFSensorHUDSnapshotTest;
	friend class FCFScannerRuntimeConfigTest;



public:
	// [v1.1.0] Sensor update가 필요한 동안만 Tick을 활성화하는 차량 Sensor Component를 생성합니다.
	UCFVehicleSensorComp();

	// [v1.3.0] UpdateIntervalSec마다 Contact 수명, bounded Detection과 Tactical Analysis를 진행합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [v1.0.0] Actor 수명이 종료될 때 Sensor Snapshot과 준비 상태를 안전하게 비웁니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// [v1.3.0] 유효한 Config로 Runtime을 초기화하고 Passive/Visual 작업이 있으면 기본 Sensor update를 활성화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Runtime", meta=(DisplayName="센서 런타임 초기화", ToolTip="SensorData 또는 Fallback 설정으로 Sensor Runtime을 초기화합니다. Passive/Visual 탐지 또는 Active Scan 요청이 있을 때 Sensor update가 실행됩니다."))
	bool InitializeSensorRuntime();

		// [v1.6.0] 새 SensorData 또는 scanner-less Fallback Source를 기존 Contact/Knowledge를 지우지 않고 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Config", meta=(DisplayName="센서 데이터 적용", ToolTip="유효한 VehicleSensorData 또는 None의 Fallback 설정을 적용합니다. Runtime Ready 상태에서는 Contact ID, 분석 진행률과 획득 Knowledge를 보존하며 탐지 능력 감소와 Active Scan 상태를 안전하게 조정합니다. 명시적으로 잘못된 SensorData는 현재 상태를 바꾸지 않고 거부합니다."))
	bool ApplySensorData(UCFVehicleSensorData* NewSensorData);

	// [v1.3.0] Runtime Contact, Active Scan, scan cursor와 공개 Snapshot을 초기화하고 Tick을 비활성화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|Runtime", meta=(DisplayName="센서 런타임 초기화 해제", ToolTip="현재 Sensor Contact와 Active Scan 상태, scan cursor와 Runtime 준비 상태를 비웁니다. 같은 Component 수명에서 ContactId serial은 재사용하지 않습니다."))
	void ResetSensorRuntime();

	// [v1.3.0] 현재 Config의 Active Scan을 한 번 시작하고 성공 여부를 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|ActiveScan", meta=(DisplayName="Active Scan 시작", ToolTip="현재 Sensor Config의 Active Scan 거리와 지속 시간이 유효하면 스캔을 시작합니다. 입력 바인딩은 Sensor가 소유하지 않으며 호출자가 이 명령 API를 사용합니다."))
	bool StartActiveScan();

	// [v1.3.0] 실행 중인 Active Scan을 중단하고 실제 중단이 발생했는지 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Sensor|ActiveScan", meta=(DisplayName="Active Scan 중단", ToolTip="현재 실행 중인 Active Scan을 즉시 중단합니다. 분석 진행률은 초기화하지 않고 이후 Sensor update에서 설정된 감소율로 서서히 감소합니다."))
	bool StopActiveScan();

		// [v1.6.0] Runtime Ready이면 실제 적용된 Sensor Config를, 초기화 전이면 현재 Source 또는 Fallback 설정을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Config", meta=(DisplayName="해석된 센서 설정 반환", ToolTip="Runtime Ready 상태에서는 ApplySensorData 또는 초기화로 실제 적용된 SensorConfig를 반환합니다. 초기화 전에는 현재 SensorData가 유효하면 해당 설정을, 아니면 FallbackSensorConfig를 반환합니다."))
	FCFSensorConfig GetResolvedSensorConfig() const;

		// [v1.0.0] 현재 Actor-free Sensor Snapshot 사본을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Snapshot", meta=(DisplayName="센서 Snapshot 반환", ToolTip="현재 Sensor Runtime이 게시한 Actor 포인터 없는 Snapshot 사본을 반환합니다."))
	FCFSensorSnapshot GetSensorSnapshot() const;

	// [v1.5.0] 이미 존재하는 Sensor Contact와 지정 Actor의 연결을 ContactId만으로 읽고 Actor truth나 private Runtime data는 공개하지 않습니다.
	bool TryGetContactIdForActor(const AActor* TargetActor, FName& OutContactId) const;


	// [v1.0.0] 현재 Sensor Runtime이 유효한 Config로 초기화됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Runtime", meta=(DisplayName="센서 런타임 준비 여부", ToolTip="유효한 SensorData 또는 Fallback Config로 Sensor Runtime이 초기화됐으면 True입니다."))
	bool IsSensorRuntimeReady() const { return bSensorRuntimeReady; }

	// [v1.3.0] Active Scan Runtime이 현재 실행 중인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|ActiveScan", meta=(DisplayName="Active Scan 실행 여부", ToolTip="Active Scan이 현재 실행 중이면 True입니다. 공개 Sensor Snapshot의 Active Scan 상태와 같은 Runtime owner를 사용합니다."))
	bool IsActiveScanRunning() const { return bActiveScanRunning; }

	// [v1.3.0] 현재 Active Scan에 남은 실행 시간을 초 단위로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|ActiveScan", meta=(DisplayName="Active Scan 남은 시간", ToolTip="현재 Active Scan에 남은 실행 시간입니다. 실행 중이 아니면 0입니다."))
	float GetActiveScanRemainingSeconds() const { return ActiveScanRemainingSeconds; }

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

		// [v1.6.0] 초기화 전 사용할 기본 Source이며 Runtime Ready 상태의 변경은 ApplySensorData를 통해 명시적으로 적용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Config", meta=(DisplayName="차량 센서 데이터", ToolTip="유효하면 초기화 시 FallbackSensorConfig보다 우선 사용할 VehicleSensorData입니다. Runtime Ready 상태에서 장비 Source를 바꿀 때는 센서 데이터 적용(ApplySensorData)을 사용하며 Content Asset을 자동 생성하거나 저장하지 않습니다."))
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

	// [v1.0.0] 현재 Sensor Runtime이 유효한 Config로 준비됐는지 여부입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|Runtime", meta=(AllowPrivateAccess="true", DisplayName="센서 런타임 준비 상태"))
	bool bSensorRuntimeReady = false;

	// [v1.3.0] 현재 Active Scan 실행 상태입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|ActiveScan", meta=(AllowPrivateAccess="true", DisplayName="Active Scan 실행 상태"))
	bool bActiveScanRunning = false;

	// [v1.3.0] 현재 Active Scan에 남은 실행 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|ActiveScan", meta=(AllowPrivateAccess="true", Units="s", DisplayName="Active Scan 남은 시간"))
	float ActiveScanRemainingSeconds = 0.0f;

	// [v1.0.0] 외부 소비자에게 공개할 현재 Actor-free Snapshot입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|Snapshot", meta=(AllowPrivateAccess="true", DisplayName="현재 센서 Snapshot"))
	FCFSensorSnapshot CurrentSensorSnapshot;

		// [v1.0.0] 마지막 Sensor Runtime 초기화 또는 Reset 결과 요약입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Sensor|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 센서 런타임 요약"))
	FString LastSensorRuntimeSummary = TEXT("SensorRuntime: NotInitialized");

	// [v1.6.0] Runtime Ready 상태에서 Source UObject의 후속 변경과 분리해 실제 Sensor가 소비하는 적용 Config 사본입니다.
	FCFSensorConfig AppliedSensorConfig;

	// [v1.6.0] AppliedSensorConfig가 현재 Runtime에 실제 적용된 유효 설정인지 여부입니다.
	bool bHasAppliedSensorConfig = false;

	// [v1.1.0] Actor가 포함될 수 있어 공개 Snapshot과 분리하는 Sensor private Runtime Contact 목록입니다.
	TArray<FCFSensorContactRuntime> RuntimeContacts;

	// [v1.0.0] 같은 Component 수명에서 새 Contact에 사용할 증가형 serial입니다.
	int32 NextContactSerial = 1;

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

	// [v1.6.0] 현재 SensorData가 유효하면 해당 설정을, 아니면 FallbackSensorConfig를 반환하는 Source 해석 전용 함수입니다.
	FCFSensorConfig ResolveConfiguredSensorConfig() const;

	// [v1.1.0] Passive 또는 Visual 탐지 거리가 실제로 설정돼 기본 bounded update가 필요한지 반환합니다.
	bool HasConfiguredPassiveWork(const FCFSensorConfig& SensorConfig) const;

	// [v1.3.0] 현재 Active Scan 실행 상태까지 포함해 이번 update에 bounded World Detection이 필요한지 반환합니다.
	bool HasConfiguredDetectionWork(const FCFSensorConfig& SensorConfig) const;

	// [v1.3.0] 현재 cursor부터 bounded Actor 슬롯을 검사하고 Contact lifetime, Detection, Analysis와 Snapshot을 한 번 갱신합니다.
	void RunPassiveDetectionUpdate(float SensorUpdateDeltaSeconds = -1.0f);

	// [v1.3.0] 하나의 Actor를 Sensor 자격·대표 위치·Passive/Visual/Active 거리 순서로 평가하고 Runtime Contact를 갱신합니다.
	bool ProcessPassiveScanActor(AActor* CandidateActor, const FCFSensorConfig& SensorConfig);

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

	// [v1.3.0] 모든 Runtime Contact의 Tactical Analysis를 active-valid 시간에는 증가시키고 그 외 시간에는 설정된 감소율로 서서히 감소시킵니다.
	void AdvanceContactAnalysis(float SensorUpdateDeltaSeconds, float ActiveScanTimeThisUpdate, const FCFSensorConfig& SensorConfig);

	// [v1.3.0] 현재 Runtime Contact가 Active Tactical Analysis 증가 조건을 만족하는지 판정합니다.
	bool IsContactValidForActiveAnalysis(FCFSensorContactRuntime& RuntimeContact, const FCFSensorConfig& SensorConfig);

	// [v1.3.0] 분석 임계값을 통과한 Contact의 Sensor Knowledge를 단방향으로 승격하고 Identified 이상에서만 TargetId/Name을 공개합니다.
	void PromoteContactKnowledgeFromAnalysis(FCFSensorContactRuntime& RuntimeContact, const FCFSensorConfig& SensorConfig);

	// [v1.3.0] 이번 Sensor update가 소비한 Active Scan 시간을 반영하고 만료되면 Active-only Contact를 LastKnown으로 전환합니다.
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