// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CarFight 미사일 비행 상태 컴포넌트
// Scope: Direct Release 미사일의 Released·Clearance·GuidedFlight 상태와 Pool 재사용 초기화를 관리합니다.
// Changelog:
// - v1.0.0: MG-P0-01 Direct 비행 상태 Runtime 최초 구현.
// Migration:
// - MissileFlightConfig.bUseMissileFlight=false인 기존 Projectile·Rocket은 Inactive 상태를 유지하며 이동 결과가 바뀌지 않습니다.
// - Direct 프로파일은 Clearance 시간·거리를 모두 만족하면 Transition을 생략하고 GuidedFlight로 진입합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileFlightTypes.h"
#include "CFProjectileLaunchTypes.h"
#include "Components/ActorComponent.h"
#include "CFMissileFlightComp.generated.h"

class UProjectileMovementComponent;

/**
 * 발사 후 런처와 독립된 미사일 비행 상태를 관리하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFMissileFlightComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 미사일 비행 Tick 기본값과 Inactive 상태를 초기화합니다.
	UCFMissileFlightComp();

	// [v1.0.0] 비행 설정과 발사 순간 Context를 복사해 이번 미사일 비행 상태를 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|MissileFlight", meta=(DisplayName="미사일 비행 시작 (Start Missile Flight)", ToolTip="ProjectileData의 미사일 비행 설정과 Launch Context를 값으로 복사하고 Released 상태에서 독립 비행을 시작합니다. 비행 사용이 꺼져 있으면 Inactive 상태를 유지합니다."))
	void StartMissileFlight(
		const FCFMissileFlightConfig& InFlightConfig,
		const FCFProjectileLaunchContext& InLaunchContext,
		UProjectileMovementComponent* InProjectileMovementComponent);

	// [v1.0.0] 비행 상태와 Runtime 참조를 Pool 재사용 안전 기본값으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|MissileFlight", meta=(DisplayName="미사일 비행 초기화 (Reset Missile Flight)", ToolTip="현재 비행 상태, 발사 위치, 경과 시간과 ProjectileMovement 참조를 초기화합니다. Projectile Actor 비활성화는 호출자가 별도로 처리합니다."))
	void ResetMissileFlight();

	// [v1.0.0] 현재 Flight State가 Guidance Command를 적용할 수 있는 구간인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileFlight", meta=(DisplayName="유도 구간 활성 여부 (Is Guidance Window Open)", ToolTip="현재 상태가 GuidedFlight 또는 Terminal이면 True를 반환합니다. Direct 미사일은 Clearance를 만족한 뒤 True가 됩니다."))
	bool IsGuidanceWindowOpen() const;

	// [v1.0.0] 현재 미사일 비행 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileFlight", meta=(DisplayName="현재 미사일 비행 상태 반환 (Get Missile Flight State)", ToolTip="Inactive부터 GuidedFlight, Impact, Expired까지 현재 비행 상태를 반환합니다."))
	ECFMissileFlightState GetCurrentFlightState() const { return CurrentFlightState; }

	// [v1.0.0] 현재 비행 상태와 Clearance 값을 포함한 스냅샷을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileFlight", meta=(DisplayName="미사일 비행 스냅샷 반환 (Get Missile Flight Snapshot)", ToolTip="현재 상태, 공격 프로파일, 경과 시간, 분리 거리와 유도 구간 여부를 포함한 스냅샷입니다."))
	FCFMissileFlightSnapshot GetFlightSnapshot() const { return CurrentFlightSnapshot; }

	// [v1.0.0] 현재 비행 상태를 한 줄 Debug 문자열로 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileFlight", meta=(DisplayName="미사일 비행 요약 생성 (Build Missile Flight Summary)", ToolTip="현재 상태, 공격 프로파일, 경과 시간, 분리 거리, Clearance와 유도 구간 상태를 문자열로 반환합니다."))
	FString BuildFlightSummary() const;

	// [v1.0.0] Automation에서 월드 Tick 없이 비행 상태를 한 단계 진행합니다.
	void AdvanceFlightForAutomation(float DeltaTime);

protected:
	// [v1.0.0] 분리 시간·거리와 Flight State 전환을 갱신합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// [v1.0.0] 한 시간 구간의 비행 거리와 상태 전환을 처리합니다.
	void AdvanceFlightSimulation(float DeltaTime);

	// [v1.0.0] 즉시 전환 가능한 상태를 순서대로 진행하고 안정 상태에서 중단합니다.
	void AdvanceFlightStateMachine(float DeltaTime);

	// [v1.0.0] 현재 Flight State를 변경합니다.
	void SetFlightState(ECFMissileFlightState NewFlightState);

	// [v1.0.0] 현재 내부 값을 Blueprint 읽기용 비행 스냅샷에 반영합니다.
	void RefreshFlightSnapshot();

	// [v1.0.0] 이번 활성화에 적용된 안전 보정 미사일 비행 설정입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="활성 미사일 비행 설정 (ActiveFlightConfig)", ToolTip="현재 활성화에 값으로 복사된 안전 보정 MissileFlightConfig입니다."))
	FCFMissileFlightConfig ActiveFlightConfig;

	// [v1.0.0] 현재 비행 거리 계산과 상태 진행에 사용할 ProjectileMovement입니다.
	UPROPERTY(Transient)
	TObjectPtr<UProjectileMovementComponent> ActiveProjectileMovementComponent = nullptr;

	// [v1.0.0] 현재 미사일 비행 상태입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 미사일 비행 상태 (CurrentFlightState)", ToolTip="현재 미사일의 Released, Clearance, GuidedFlight 등 비행 상태입니다."))
	ECFMissileFlightState CurrentFlightState = ECFMissileFlightState::Inactive;

	// [v1.0.0] Launch Context에서 복사한 분리 순간 월드 위치입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="분리 시작 위치 (ReleaseLocation)", ToolTip="Clearance 거리를 계산할 발사 순간 월드 위치입니다. 발사 뒤 런처 Transform을 다시 조회하지 않습니다."))
	FVector ReleaseLocation = FVector::ZeroVector;

	// [v1.0.0] 이번 활성화의 누적 비행 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="경과 비행 시간 초 (ElapsedFlightTimeSeconds)", ToolTip="Released 상태 이후 누적된 비행 시간입니다."))
	float ElapsedFlightTimeSeconds = 0.0f;

	// [v1.0.0] 비Direct Transition 상태에서 소비한 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="경과 전환 시간 초 (ElapsedTransitionTimeSeconds)", ToolTip="Angled 또는 Vertical 사출의 Transition 상태에서 누적된 시간입니다. Direct 프로파일은 사용하지 않습니다."))
	float ElapsedTransitionTimeSeconds = 0.0f;

	// [v1.0.0] 분리 위치에서 현재 위치까지의 직선 거리입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="분리 거리 cm (DistanceFromReleaseCm)", ToolTip="Launch Context의 분리 위치에서 현재 미사일 위치까지의 직선 거리입니다."))
	float DistanceFromReleaseCm = 0.0f;

	// [v1.0.0] Pool 재사용을 포함한 누적 비행 시작 횟수입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="비행 활성화 횟수 (FlightActivationCount)", ToolTip="StartMissileFlight가 호출된 누적 횟수입니다. Reset에서는 유지됩니다."))
	int32 FlightActivationCount = 0;

	// [v1.0.0] Blueprint와 Debug가 한 번에 읽을 현재 비행 스냅샷입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileFlight|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 미사일 비행 스냅샷 (CurrentFlightSnapshot)", ToolTip="현재 상태, 프로파일, 시간, 거리와 Guidance 구간 상태를 모은 스냅샷입니다."))
	FCFMissileFlightSnapshot CurrentFlightSnapshot;
};
