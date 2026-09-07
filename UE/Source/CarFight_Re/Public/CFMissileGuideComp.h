// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.1
// Date: 2026-09-07
// Description: CarFight 물리 제한형 미사일 유도 컴포넌트
// Scope: 발사 순간 Target Actor Snapshot, Guidance Activation, Legacy/Stateful Seeker, Direct/Sampled 관측·속도 추정, Guidance Law 전략, rear-aspect·오버슈트와 Pool Reset을 관리합니다.
// Changelog:
// - v1.4.1: MG-P0-12D 최종검수 P1 교정으로 exact rear Launch Right tie-break를 PN Course Capture뿐 아니라 PurePursuit/LeadPursuit의 공통 bounded Pursuit 경로에도 적용.
// - v1.4.0: MG-P0-12D Independent Guidance Activation latch, free Stateful Seeker geometry, approach-armed Overshoot와 Launch Right 기반 exact rear tie-break를 추가.
// - v1.3.0: MG-P0-12C PurePursuit/LeadPursuit/ProportionalNavigation 전략 선택, bounded lead aim point와 rear/non-closing PN Course Capture를 추가.
// - v1.2.0: MG-P0-10 SampledPositionEstimate 관측 주기, 위치 차분 속도 추정, 관측 사이 외삽과 단일 Estimated Target State 소비 경로를 추가.
// - v1.1.0: MG-P0-09 Stateful Seeker의 Acquiring/Tracking/LostGrace/Reacquiring/LostFinal 상태, Acquisition/Tracking/Reacquisition 반각, Hold point와 명시적 Reacquisition 정책을 추가.
// - v1.0.0: MG-P0-03~04 Direct TargetActor Guidance Runtime 최초 구현.
// Migration:
// - GuidanceLaw=ProportionalNavigation, GuidanceActivationMode=FollowFlightGuidanceWindow, SeekerModel=LegacySingleGate와 TargetObservationMode=DirectActorKinematics 기본값에서는 기존 Flight Window·단일 Seeker Gate·PN/Actor 직접 관측 동작을 그대로 유지합니다.
// - Independent Guidance Activation은 FlightComp의 시간/분리거리 Snapshot을 읽어 GuideComp가 AND 조건과 activation latch를 소유하며 FlightComp 상태 머신을 변경하지 않습니다.
// - Stateful Overshoot는 실제 접근이 한 번 성립한 뒤에만 arm하고 LegacySingleGate의 기존 즉시 거리증가 판정은 그대로 보존합니다.
// - PurePursuit와 LeadPursuit는 기존 선회율·횡가속·응답시간 제한을 공유하며 순간이동이나 Velocity 방향 강제 덮어쓰기를 사용하지 않습니다.
// - SampledPositionEstimate는 Target Actor 위치만 설정된 주기로 관측하며 Actor GetVelocity 정답을 Guidance 입력으로 소비하지 않습니다.
// - MissileGuideConfig.bUseGuidance=false인 기존 Projectile·Rocket은 Velocity를 변경하지 않습니다.
// - P0 Runtime은 TargetActor와 ContinueStraight를 우선 지원하며 LaserPoint·DataLink와 실제 Expire 요청은 후속 단계입니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileGuideTypes.h"
#include "CFProjectileLaunchTypes.h"
#include "Components/ActorComponent.h"
#include "CFMissileGuideComp.generated.h"

class AActor;
class UCFMissileFlightComp;
class UProjectileMovementComponent;

/**
 * 발사 순간 복사한 목표를 물리 제한 안에서 추적하는 미사일 유도 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFMissileGuideComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] Guidance Tick 기본값과 비활성 상태를 초기화합니다.
	UCFMissileGuideComp();

	// [v1.0.0] Guidance 설정과 발사 순간 목표 Snapshot을 복사해 이번 유도 상태를 시작합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|MissileGuidance", meta=(DisplayName="미사일 유도 시작 (Start Missile Guidance)", ToolTip="ProjectileData의 Guidance 설정과 Launch Context의 Target Actor를 복사합니다. Follow Flight Guidance Window는 기존 FlightComp 유도 구간을 따르고, Independent는 설정된 시간·분리 거리 조건을 만족하면 독립적으로 제한형 유도를 활성화합니다."))
	void StartMissileGuidance(
		const FCFMissileGuideConfig& InGuideConfig,
		const FCFProjectileLaunchContext& InLaunchContext,
		UProjectileMovementComponent* InProjectileMovementComponent,
		UCFMissileFlightComp* InMissileFlightComponent);

	// [v1.0.0] 목표 참조, Command와 Runtime 참조를 Pool 재사용 안전 기본값으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|MissileGuidance", meta=(DisplayName="미사일 유도 초기화 (Reset Missile Guidance)", ToolTip="현재 목표 Snapshot, 상실 시간, Guidance Command와 이동 참조를 초기화합니다. Projectile Actor 비활성화는 호출자가 별도로 처리합니다."))
	void ResetMissileGuidance();

	// [v1.0.0] 발사 순간 복사되어 현재 미사일이 독립적으로 유지하는 목표 Actor를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileGuidance", meta=(DisplayName="유도 목표 Actor 반환 (Get Guidance Target Actor)", ToolTip="발사 순간 Launch Context에서 복사한 Target Actor입니다. 차량이 이후 다른 타겟을 선택해도 이 참조는 바뀌지 않습니다."))
	AActor* GetGuidanceTargetActor() const { return GuidanceTargetActor.Get(); }

	// [v1.0.0] 현재 유도 목표와 Command 상태를 포함한 스냅샷을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileGuidance", meta=(DisplayName="미사일 유도 스냅샷 반환 (Get Missile Guide Snapshot)", ToolTip="현재 Guide Mode, 목표 유효성, 목표 위치, Seeker 각도, 횡가속도와 상실 사유를 포함한 스냅샷입니다."))
	FCFMissileGuideSnapshot GetGuideSnapshot() const { return CurrentGuideSnapshot; }

	// [v1.0.0] 마지막으로 계산한 제한형 Guidance Command를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileGuidance", meta=(DisplayName="현재 유도 명령 반환 (Get Missile Guidance Command)", ToolTip="마지막 Guidance 계산의 요구·적용 횡가속도, 선회율, Seeker 각도와 무효 사유를 반환합니다."))
	FCFMissileGuidanceCommand GetGuidanceCommand() const { return CurrentGuidanceCommand; }

	// [v1.0.0] 현재 유도 상태를 한 줄 Debug 문자열로 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|MissileGuidance", meta=(DisplayName="미사일 유도 요약 생성 (Build Missile Guidance Summary)", ToolTip="Guide Mode, 목표, 상실 시간, Seeker 각도, 적용 횡가속도와 Miss 사유를 문자열로 반환합니다."))
	FString BuildGuidanceSummary() const;

	// [v1.0.0] Automation에서 월드 Tick 없이 Guidance를 한 단계 진행합니다.
	void AdvanceGuidanceForAutomation(float DeltaTime);

protected:
	// [v1.0.0] Flight State와 Target Snapshot을 읽어 제한형 Guidance Velocity를 갱신합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// [v1.0.0] 한 시간 구간의 목표 관측, 제한형 Guidance와 상실 정책을 처리합니다.
	void AdvanceGuidanceSimulation(float DeltaTime);

	// [v1.4.0] 현재 Activation Mode에 따라 Flight Guidance Window 또는 독립 시간·거리 AND 조건을 판정하고 Independent 만족 상태를 latch합니다.
	bool ResolveGuidanceActivation();

	// [v1.2.0] 발사 순간 Target Snapshot을 최초 관측 seed로 저장하며 Sampled 모드에서는 Actor Velocity를 읽지 않습니다.
	bool InitializeTargetObservationSeed();

	// [v1.2.0] 현재 관측 모델에 따라 Direct Actor 상태 또는 Sampled Estimated Target State를 반환합니다.
	bool TryResolveTargetObservation(
		float DeltaTime,
		FVector& OutTargetLocation,
		FVector& OutTargetVelocity);

	// [v1.2.0] DirectActorKinematics에서 Target Actor의 현재 위치와 Velocity를 직접 읽고 Debug 관측 상태를 동기화합니다.
	bool ResolveDirectActorObservation(
		FVector& OutTargetLocation,
		FVector& OutTargetVelocity);

	// [v1.2.0] SampledPositionEstimate에서 관측 주기마다 위치만 읽고 속도를 위치 차분으로 추정하며 관측 사이 위치를 외삽합니다.
	bool AdvanceSampledTargetObservation(
		float DeltaTime,
		FVector& OutTargetLocation,
		FVector& OutTargetVelocity);

	// [v1.2.0] Launch Snapshot Target Actor 참조가 현재 관측 가능한 수명인지 반환합니다.
	bool IsGuidanceTargetActorValid() const;

	// [v1.0.0] 목표 상실 유예와 LostTargetPolicy에 따라 Legacy 마지막 지점 사용 여부를 결정합니다.
	bool ResolveLostTargetFallback(
		float DeltaTime,
		ECFMissileMissReason ObservationFailureReason,
		FVector& OutTargetLocation,
		FVector& OutTargetVelocity);

	// [v1.1.0] Stateful Seeker 상태를 진행하고 이번 Step에서 Guidance가 소비할 목표 상태가 있으면 반환합니다.
	bool ResolveStatefulGuidanceTarget(
		float DeltaTime,
		const FVector& MissileLocation,
		const FVector& MissileVelocity,
		FVector& OutTargetLocation,
		FVector& OutTargetVelocity);

	// [v1.1.0] Stateful 중심선 기준 반각 안에 목표가 있는지 검사하고 현재 Seeker 각도를 반환합니다.
	bool IsTargetInsideStatefulCone(
		const FVector& MissileVelocity,
		const FVector& TargetLocation,
		float AllowedHalfAngleDeg,
		float& OutSeekerAngleDeg) const;

	// [v1.1.0] Stateful Seeker를 최종 상실 상태로 전환하고 HoldLastKnownPoint가 필요하면 Sensor Truth 지점을 한 번 고정합니다.
	void EnterStatefulLostFinal(
		ECFMissileMissReason FinalMissReason,
		const FVector& SensorTruthLocation,
		bool bHasSensorTruthLocation);

	// [v1.1.0] LostFinal에서 HoldLastKnownPoint만 고정 지점 Guidance를 계속할 수 있게 반환합니다.
	bool ResolveStatefulLostFinalGuidanceTarget(
		FVector& OutTargetLocation,
		FVector& OutTargetVelocity);

	// [v1.0.0] Seeker FOV와 Lock Break 제한을 검사하고 실패 사유를 반환합니다.
	bool IsTargetInsideSeekerLimits(
		const FVector& MissileVelocity,
		const FVector& TargetLocation,
		float& OutSeekerAngleDeg,
		ECFMissileMissReason& OutFailureReason) const;

	// [v1.0.0] LegacySingleGate에서 거리 증가와 진행 방향을 사용해 목표를 지나친 오버슈트인지 판정합니다.
	bool HasOvershotTarget(const FVector& MissileVelocity, const FVector& TargetLocation, float CurrentTargetDistanceCm) const;

	// [v1.4.0] Stateful Seeker에서 실제 접근을 한 번 확인한 뒤에만 Overshoot 판정을 arm하고 armed 상태의 비접근 거리 증가를 종결로 판정합니다.
	bool UpdateStatefulOvershootState(
		const FVector& MissileLocation,
		const FVector& MissileVelocity,
		const FVector& TargetLocation,
		const FVector& TargetVelocity,
		float CurrentTargetDistanceCm);

	// [v1.3.0] 현재 Guidance Law가 허용한 Sensor State로 AimPoint와 물리 제한형 Guidance Command를 생성합니다.
	FCFMissileGuidanceCommand BuildGuidanceCommandForActiveLaw(
		const FVector& MissileLocation,
		const FVector& MissileVelocity,
		const FVector& TargetLocation,
		const FVector& TargetVelocity,
		float DeltaTime);

	// [v1.4.1] 모든 bounded Pursuit 계열에서 exact rear 횡방향 특이점을 Launch Right Vector로 동일하게 해소합니다.
	FCFMissileGuidanceCommand BuildBoundedPursuitCommandWithRearTieBreak(
		const FCFMissileGuidanceInput& GuidanceInput) const;

	// [v1.0.0] 순수 Guidance Command를 응답 시간으로 보간하고 현재 속력 안에서 Velocity 방향에 적용합니다.
	void ApplyGuidanceCommand(const FCFMissileGuidanceCommand& InGuidanceCommand, float DeltaTime);

	// [v1.0.0] 이번 프레임 Guidance를 적용하지 않고 지정 Miss 사유를 기록합니다.
	void InvalidateCurrentGuidance(ECFMissileMissReason MissReason);

	// [v1.0.0] 현재 내부 값을 Blueprint 읽기용 Guidance 스냅샷에 반영합니다.
	void RefreshGuideSnapshot();

	// [v1.0.0] 이번 활성화에 적용된 안전 보정 Guidance 설정입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="활성 미사일 유도 설정 (ActiveGuideConfig)", ToolTip="현재 활성화에 값으로 복사된 안전 보정 MissileGuideConfig입니다."))
	FCFMissileGuideConfig ActiveGuideConfig;

	// [v1.0.0] Guidance가 방향을 변경할 실제 ProjectileMovement입니다.
	UPROPERTY(Transient)
	TObjectPtr<UProjectileMovementComponent> ActiveProjectileMovementComponent = nullptr;

	// [v1.0.0] Guidance 적용 가능 상태를 제공하는 미사일 Flight 컴포넌트입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFMissileFlightComp> ActiveMissileFlightComponent = nullptr;

	// [v1.0.0] 발사 순간 Launch Context에서 복사한 독립 Target Actor 약한 참조입니다.
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> GuidanceTargetActor;

	// [v1.4.0] exact 180도 rear target에서 좌우 선회 특이점을 결정론적으로 해소할 발사 순간 Right Vector입니다.
	UPROPERTY(Transient)
	FVector LaunchRightVector = FVector::ZeroVector;

	// [v1.1.0] Stateful Seeker의 현재 발사 후 Target 추적 상태입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 탐색기 상태 (CurrentSeekerState)", ToolTip="Stateful Seeker의 Inactive, Acquiring, Tracking, LostGrace, Reacquiring 또는 LostFinal 상태입니다. Legacy 모델은 Inactive를 유지합니다."))
	ECFMissileSeekerState CurrentSeekerState = ECFMissileSeekerState::Inactive;

	// [v1.1.0] Stateful LostFinal이 확정된 뒤 유지할 최종 유도 실패 진단 사유입니다.
	UPROPERTY(Transient)
	ECFMissileMissReason StatefulFinalMissReason = ECFMissileMissReason::None;

	// [v1.0.0] 마지막으로 Guidance가 보존한 목표 월드 위치입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 목표 위치 (LastKnownTargetLocation)", ToolTip="목표가 사라지거나 Seeker 제한을 벗어나기 직전 Guidance가 보존한 월드 위치입니다."))
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	// [v1.1.0] Sampled estimator가 다음 관측에서 차분 기준으로 사용할 이전 실제 관측 위치입니다.
	UPROPERTY(Transient)
	FVector PreviousObservedTargetLocation = FVector::ZeroVector;

	// [v1.1.0] 현재 activation에서 Target Actor로부터 마지막으로 직접 취득한 관측 위치입니다.
	UPROPERTY(Transient)
	FVector LastObservedTargetLocation = FVector::ZeroVector;

	// [v1.1.0] 현재 Seeker와 Guidance가 Sensor Truth로 사용할 추정 목표 위치입니다.
	UPROPERTY(Transient)
	FVector EstimatedTargetLocation = FVector::ZeroVector;

	// [v1.1.0] Sampled 위치 차분으로 계산해 필터링한 목표 속도 추정값입니다.
	UPROPERTY(Transient)
	FVector FilteredTargetVelocityEstimate = FVector::ZeroVector;

	// [v1.1.0] HoldLastKnownPoint LostFinal에서 외삽 없이 유지할 고정 월드 지점입니다.
	UPROPERTY(Transient)
	FVector HoldTargetLocation = FVector::ZeroVector;

	// [v1.3.0] 현재 Guidance Law가 이번 Step에서 실제로 향한 월드 Aim Point입니다.
	UPROPERTY(Transient)
	FVector GuidanceAimPoint = FVector::ZeroVector;

	// [v1.3.0] PN이 rear/non-closing 기하에서 물리 제한형 Pursuit Course Capture를 사용 중인지 여부입니다.
	UPROPERTY(Transient)
	bool bCourseCaptureActive = false;

	// [v1.4.0] 현재 Guidance Activation 조건이 실제로 열렸는지 나타내며 Independent에서는 한 번 True가 되면 activation 종료까지 유지됩니다.
	UPROPERTY(Transient)
	bool bGuidanceActivationSatisfied = false;

	// [v1.4.0] Stateful Seeker가 실제 거리 감소와 양의 접근 속도를 한 번 확인해 Overshoot 판정을 허용하는지 나타냅니다.
	UPROPERTY(Transient)
	bool bOvershootArmed = false;

	// [v1.0.0] 응답 시간 필터가 유지하는 현재 횡가속도입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="필터된 횡가속도 (FilteredLateralAcceleration)", ToolTip="GuidanceResponseTimeSeconds에 따라 보간되어 현재 Velocity 방향 변경에 적용되는 횡가속도입니다."))
	FVector FilteredLateralAcceleration = FVector::ZeroVector;

	// [v1.0.0] 마지막으로 계산한 Target까지의 거리입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="이전 목표 거리 cm (PreviousTargetDistanceCm)", ToolTip="목표를 지나쳐 거리가 증가하는 오버슈트를 진단하기 위한 이전 프레임 거리입니다."))
	float PreviousTargetDistanceCm = 0.0f;

	// [v1.0.0] 목표 관측이 연속으로 실패한 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="목표 상실 시간 초 (TargetLostTimeSeconds)", ToolTip="Target Actor 무효화 또는 Seeker 제한 이탈이 연속으로 유지된 시간입니다."))
	float TargetLostTimeSeconds = 0.0f;

	// [v1.1.0] 마지막 실제 Target 위치 관측 이후 누적된 시간입니다.
	UPROPERTY(Transient)
	float ObservationAgeSeconds = 0.0f;

	// [v1.1.0] 현재 Reacquiring 상태에서 소비한 시간입니다.
	UPROPERTY(Transient)
	float ReacquisitionElapsedTimeSeconds = 0.0f;

	// [v1.0.0] 마지막 Target 위치와 속도가 유효했는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasLastKnownTargetLocation = false;

	// [v1.0.0] 오버슈트 판정을 위한 이전 거리 값이 존재하는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasPreviousTargetDistance = false;

	// [v1.1.0] Sampled estimator용 이전 실제 관측 위치가 준비됐는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasPreviousObservedTargetLocation = false;

	// [v1.1.0] 현재 activation에서 유효한 Target 관측 seed 또는 sample을 한 번 이상 확보했는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasValidObservation = false;

	// [v1.1.0] LostFinal HoldLastKnownPoint가 사용할 고정 지점을 확보했는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasHoldTargetLocation = false;

	// [v1.0.0] 이번 Guidance 단계에서 실제 Target Actor 관측이 유효했는지 여부입니다.
	UPROPERTY(Transient)
	bool bTargetValidThisStep = false;

	// [v1.0.0] Pool 재사용을 포함한 누적 Guidance 시작 횟수입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="유도 활성화 횟수 (GuideActivationCount)", ToolTip="StartMissileGuidance가 호출된 누적 횟수입니다. Reset에서는 유지됩니다."))
	int32 GuideActivationCount = 0;

	// [v1.0.0] 마지막으로 계산하거나 무효화한 Guidance Command입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 유도 명령 (CurrentGuidanceCommand)", ToolTip="마지막 제한형 비례항법 계산과 응답 필터 적용 결과입니다."))
	FCFMissileGuidanceCommand CurrentGuidanceCommand;

	// [v1.0.0] Blueprint와 Debug가 한 번에 읽을 현재 Guidance 스냅샷입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="현재 미사일 유도 스냅샷 (CurrentGuideSnapshot)", ToolTip="현재 Guide Mode, 목표 상태, Seeker 각도, 횡가속도, 상실 시간과 Miss 사유를 모은 스냅샷입니다."))
	FCFMissileGuideSnapshot CurrentGuideSnapshot;
};
