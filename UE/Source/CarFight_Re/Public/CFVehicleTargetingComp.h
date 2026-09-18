// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.2
// Date: 2026-09-18
// Description: Phase 4 단일 Vehicle Target Lock Runtime Authority / Phase 7 Guided Weapon actor bridge fail-closed 보강
// Changelog:
// - v1.0.2: GetLockedTargetActor가 Targeting Runtime Ready + Locked + valid public snapshot + valid non-destroying Actor를 모두 만족할 때만 내부 Gameplay Actor bridge를 반환하도록 fail-closed 강화하고 Phase 7 consumer focused test exact friend를 추가.
// - v1.0.1: FCFTargetingRequestLockTest가 public RequestLock 이후 내부 상태 진행을 결정적으로 검증할 수 있도록 exact test friend를 추가. Runtime/Blueprint 계약 변경 없음.
// - v1.0.0: Idle/Acquiring/Locked 상태, Live Contact 기반 RequestLock, LastKnown 진행/품질 감쇠, Lost/Destroyed Break와 actor-free Snapshot을 최초 구현.
// Migration:
// - TargetSelect는 선택 상태를 계속 독립 소유합니다.
// - Phase 7부터 TargetActor Guided Weapon의 내부 Guidance source는 VehicleFireComp가 GetLockedTargetActor를 통해 Vehicle Locked Target만 사용합니다.
// - HUD와 일반 Consumer는 GetLockedTargetActor가 아니라 GetTargetingSnapshot을 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFTargetingTypes.h"
#include "CFVehicleTargetingComp.generated.h"

class AActor;
class UCFVehicleSensorComp;
struct FCFSensorContact;
struct FCFSensorSnapshot;

/** 차량 단위 단일 Target Lock의 상태와 Sensor Contact 기반 유지 정책을 소유합니다. */
UCLASS(ClassGroup=(CarFight), meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleTargetingComp : public UActorComponent
{
	GENERATED_BODY()

	// Phase 4 focused Automation이 public API 확대 없이 결정적 상태 전이를 검증할 수 있게 허용합니다.
	friend class FCFTargetingRuntimeTest;
	friend class FCFTargetingRequestLockTest;
	friend class FCFPhase7GuidedWeaponTargetSourceTest;

public:
	// Targeting Runtime 기본 Tick 정책과 P0 fallback 설정을 준비합니다.
	UCFVehicleTargetingComp();

protected:
	// Component 시작 시 독립 Targeting Runtime을 초기화합니다.
	virtual void BeginPlay() override;

	// Component 수명 종료 시 Actor bridge와 Targeting 상태를 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Acquiring/Locked 동안 현재 Sensor Snapshot을 소비해 Lock 상태를 진행합니다.
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// 현재 fallback Config를 검증하고 새로운 Targeting Runtime lifetime을 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Targeting", meta=(DisplayName="타겟팅 런타임 초기화", ToolTip="Vehicle Target Lock Runtime을 초기화합니다. 기존 락 상태와 Break 전이 이력은 새 Runtime lifetime으로 초기화됩니다."))
	bool InitializeTargetingRuntime();

	// 현재 Lock/Acquire와 Break 이력을 모두 비우고 Runtime을 미준비 상태로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Targeting", meta=(DisplayName="타겟팅 런타임 리셋", ToolTip="현재 Vehicle Target Lock 상태와 Break 전이 이력을 모두 초기화합니다."))
	void ResetTargetingRuntime();

	// Live Sensor Contact로 관측 중인 Actor를 단일 Vehicle Target Lock 대상으로 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Targeting", meta=(DisplayName="타겟 락 요청", ToolTip="대상이 현재 Sensor의 Live Contact일 때만 락 획득을 시작합니다. 같은 대상의 중복 요청은 명시적 Already 결과를 반환하며 진행률을 재시작하지 않습니다."))
	ECFTargetLockRequestResult RequestLock(AActor* TargetActor);

	// 현재 Acquiring 또는 Locked 상태를 명시적으로 해제하며 Break 이벤트는 만들지 않습니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Targeting", meta=(DisplayName="타겟 락 해제", ToolTip="현재 락 획득/락 완료 상태를 Idle로 되돌립니다. 수동 해제는 Lock Break 이벤트가 아니므로 BreakTransitionRevision을 증가시키지 않습니다."))
	bool ClearLock();

	// HUD와 일반 Consumer가 Actor 없이 사용할 현재 Targeting Snapshot 사본을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Targeting", meta=(DisplayName="타겟팅 스냅샷 반환", ToolTip="현재 Vehicle Target Lock 상태, ContactId, 진행률, 품질과 Break 전이를 Actor 참조 없이 반환합니다."))
	FCFTargetingSnapshot GetTargetingSnapshot() const { return CurrentTargetingSnapshot; }

	// Targeting Runtime이 유효한 독립 Config로 초기화됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Targeting", meta=(DisplayName="타겟팅 런타임 준비 여부", ToolTip="Vehicle Target Lock Runtime이 유효한 Config로 초기화되어 명령을 받을 수 있는지 반환합니다."))
	bool IsTargetingRuntimeReady() const { return bTargetingRuntimeReady; }

	// 가장 최근 RequestLock 호출의 명시적 결과를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Targeting", meta=(DisplayName="마지막 타겟 락 요청 결과", ToolTip="가장 최근 RequestLock 요청이 수락 또는 거부된 이유를 반환합니다."))
	ECFTargetLockRequestResult GetLastLockRequestResult() const { return LastLockRequestResult; }

	// Phase 7 Guided Weapon 같은 내부 Gameplay Consumer가 Runtime Ready + valid Locked Snapshot + valid Actor를 모두 만족하는 대상 Actor bridge를 읽습니다. HUD truth로 사용하지 않습니다.
	AActor* GetLockedTargetActor() const;

	// Sensor tuning과 분리된 P0 Targeting fallback 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Targeting", meta=(DisplayName="기본 타겟팅 설정", ToolTip="별도 Targeting Profile 도입 전 사용하는 Vehicle Target Lock 독립 fallback 설정입니다. Sensor Analysis tuning과 공유하지 않습니다."))
	FCFTargetingConfig FallbackTargetingConfig;

private:
	// 현재 소유 Actor의 Sensor Component를 찾아 반환합니다.
	UCFVehicleSensorComp* ResolveSensorComponent() const;

	// Sensor Snapshot에서 exact ContactId와 일치하는 Contact를 찾아 반환합니다.
	const FCFSensorContact* FindContactById(const FCFSensorSnapshot& SensorSnapshot, const FName ContactId) const;

	// 현재 Sensor Snapshot 한 번을 소비해 Acquiring/Locked 상태를 진행합니다.
	void AdvanceTargetingState(float DeltaSeconds, const FCFSensorSnapshot& SensorSnapshot);

	// Acquiring 상태의 Contact 상태와 설정을 적용합니다.
	void UpdateAcquiringState(float DeltaSeconds, const FCFSensorContact& SensorContact);

	// Locked 상태의 Contact 상태와 설정을 적용합니다.
	void UpdateLockedState(float DeltaSeconds, const FCFSensorContact& SensorContact);

	// Break 전이 증거는 보존하면서 현재 대상/진행/품질만 Idle로 비웁니다.
	void ClearActiveTargetState();

	// 실제 Locked Break를 exact-once 기록하고 즉시 Idle로 전환합니다.
	void BreakCurrentLock(ECFTargetLockBreakReason BreakReason);

	// 현재 Runtime이 Lock 명령을 받을 준비가 됐는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, Category="CarFight|Targeting")
	bool bTargetingRuntimeReady = false;

	// HUD와 일반 Consumer에 공개할 actor-free Targeting 상태입니다.
	UPROPERTY(VisibleInstanceOnly, Category="CarFight|Targeting")
	FCFTargetingSnapshot CurrentTargetingSnapshot;

	// 현재 Acquiring/Locked 대상의 내부 Gameplay Actor bridge입니다.
	TWeakObjectPtr<AActor> CurrentTargetActor;

	// 가장 최근 RequestLock 호출의 명시적 결과입니다.
	UPROPERTY(VisibleInstanceOnly, Category="CarFight|Targeting")
	ECFTargetLockRequestResult LastLockRequestResult = ECFTargetLockRequestResult::None;
};
