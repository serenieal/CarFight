// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CarFight 물리 제한형 미사일 유도 컴포넌트
// Scope: 발사 순간 Target Actor Snapshot, 제한형 비례항법, Seeker 제한, 목표 상실·오버슈트와 Pool Reset을 관리합니다.
// Changelog:
// - v1.0.0: MG-P0-03~04 Direct TargetActor Guidance Runtime 최초 구현.
// Migration:
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
	UFUNCTION(BlueprintCallable, Category="CarFight|MissileGuidance", meta=(DisplayName="미사일 유도 시작 (Start Missile Guidance)", ToolTip="ProjectileData의 Guidance 설정과 Launch Context의 Target Actor를 복사하고 FlightComp가 유도 구간을 열면 제한형 유도를 적용합니다."))
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

	// [v1.0.0] 현재 Target Actor에서 유효한 위치와 Velocity를 읽습니다.
	bool TryResolveTargetObservation(FVector& OutTargetLocation, FVector& OutTargetVelocity) const;

	// [v1.0.0] 목표 상실 유예와 LostTargetPolicy에 따라 마지막 지점 사용 여부를 결정합니다.
	bool ResolveLostTargetFallback(
		float DeltaTime,
		ECFMissileMissReason ObservationFailureReason,
		FVector& OutTargetLocation,
		FVector& OutTargetVelocity);

	// [v1.0.0] Seeker FOV와 Lock Break 제한을 검사하고 실패 사유를 반환합니다.
	bool IsTargetInsideSeekerLimits(
		const FVector& MissileVelocity,
		const FVector& TargetLocation,
		float& OutSeekerAngleDeg,
		ECFMissileMissReason& OutFailureReason) const;

	// [v1.0.0] 거리 증가와 진행 방향을 사용해 목표를 지나친 오버슈트인지 판정합니다.
	bool HasOvershotTarget(const FVector& MissileVelocity, const FVector& TargetLocation, float CurrentTargetDistanceCm) const;

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

	// [v1.0.0] 마지막으로 유효하게 관측한 목표 월드 위치입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 목표 위치 (LastKnownTargetLocation)", ToolTip="목표가 사라지거나 Seeker 제한을 벗어나기 직전 마지막으로 유효했던 월드 위치입니다."))
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 응답 시간 필터가 유지하는 현재 횡가속도입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="필터된 횡가속도 (FilteredLateralAcceleration)", ToolTip="GuidanceResponseTimeSeconds에 따라 보간되어 현재 Velocity 방향 변경에 적용되는 횡가속도입니다."))
	FVector FilteredLateralAcceleration = FVector::ZeroVector;

	// [v1.0.0] 마지막으로 계산한 Target까지의 거리입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="이전 목표 거리 cm (PreviousTargetDistanceCm)", ToolTip="목표를 지나쳐 거리가 증가하는 오버슈트를 진단하기 위한 이전 프레임 거리입니다."))
	float PreviousTargetDistanceCm = 0.0f;

	// [v1.0.0] 목표 관측이 연속으로 실패한 시간입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|MissileGuidance|Debug", meta=(AllowPrivateAccess="true", DisplayName="목표 상실 시간 초 (TargetLostTimeSeconds)", ToolTip="Target Actor 무효화 또는 Seeker 제한 이탈이 연속으로 유지된 시간입니다."))
	float TargetLostTimeSeconds = 0.0f;

	// [v1.0.0] 마지막 Target 위치와 속도가 유효했는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasLastKnownTargetLocation = false;

	// [v1.0.0] 오버슈트 판정을 위한 이전 거리 값이 존재하는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasPreviousTargetDistance = false;

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
