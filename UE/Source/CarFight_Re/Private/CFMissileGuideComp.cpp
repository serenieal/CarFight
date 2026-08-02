// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CarFight 물리 제한형 미사일 유도 컴포넌트 구현
// Scope: Target Actor Snapshot, Seeker 제한, 제한형 비례항법, 목표 상실·오버슈트와 Velocity 방향 적용을 구현합니다.
// Changelog:
// - v1.0.0: MG-P0-03~04 Direct TargetActor Guidance Runtime 최초 구현.
// Migration:
// - 기존 ProjectileData는 bUseGuidance=false 기본값으로 컴포넌트 Tick과 Velocity 변경을 사용하지 않습니다.

#include "CFMissileGuideComp.h"

#include "CFMissileFlightComp.h"
#include "CFMissileGuideMath.h"

#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"

namespace
{
	// [v1.0.0] FVector의 세 축이 모두 유한한 값인지 반환합니다.
	bool IsFiniteMissileVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}
}

// [v1.0.0] Guidance Tick 기본값과 비활성 상태를 초기화합니다.
UCFMissileGuideComp::UCFMissileGuideComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetComponentTickEnabled(false);
	RefreshGuideSnapshot();
}

// [v1.0.0] Guidance 설정과 발사 순간 목표 Snapshot을 복사해 이번 유도 상태를 시작합니다.
void UCFMissileGuideComp::StartMissileGuidance(
	const FCFMissileGuideConfig& InGuideConfig,
	const FCFProjectileLaunchContext& InLaunchContext,
	UProjectileMovementComponent* InProjectileMovementComponent,
	UCFMissileFlightComp* InMissileFlightComponent)
{
	ResetMissileGuidance();

	ActiveGuideConfig = InGuideConfig.GetEffectiveConfig();
	ActiveProjectileMovementComponent = InProjectileMovementComponent;
	ActiveMissileFlightComponent = InMissileFlightComponent;
	GuidanceTargetActor = InLaunchContext.GuidanceTargetActor;
	++GuideActivationCount;

	// [v1.0.0] 발사 순간 유효한 Target Actor에서 복사할 최초 목표 위치입니다.
	FVector InitialTargetLocation = FVector::ZeroVector;

	// [v1.0.0] 최초 관측 API의 출력 형식을 맞추기 위한 목표 속도입니다.
	FVector InitialTargetVelocity = FVector::ZeroVector;

	if (TryResolveTargetObservation(InitialTargetLocation, InitialTargetVelocity))
	{
		LastKnownTargetLocation = InitialTargetLocation;
		bHasLastKnownTargetLocation = true;
		bTargetValidThisStep = true;
	}

	if (!ActiveGuideConfig.IsGuidanceEnabled()
		|| !IsValid(ActiveProjectileMovementComponent.Get())
		|| !IsValid(ActiveMissileFlightComponent.Get()))
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::GuidanceDisabled);
		SetComponentTickEnabled(false);
		RefreshGuideSnapshot();
		return;
	}

	SetComponentTickEnabled(true);
	RefreshGuideSnapshot();
}

// [v1.0.0] 목표 참조, Command와 Runtime 참조를 Pool 재사용 안전 기본값으로 초기화합니다.
void UCFMissileGuideComp::ResetMissileGuidance()
{
	SetComponentTickEnabled(false);
	ActiveGuideConfig = FCFMissileGuideConfig();
	ActiveProjectileMovementComponent = nullptr;
	ActiveMissileFlightComponent = nullptr;
	GuidanceTargetActor.Reset();
	LastKnownTargetLocation = FVector::ZeroVector;
	FilteredLateralAcceleration = FVector::ZeroVector;
	PreviousTargetDistanceCm = 0.0f;
	TargetLostTimeSeconds = 0.0f;
	bHasLastKnownTargetLocation = false;
	bHasPreviousTargetDistance = false;
	bTargetValidThisStep = false;
	CurrentGuidanceCommand = FCFMissileGuidanceCommand();
	RefreshGuideSnapshot();
}

// [v1.0.0] 현재 유도 상태를 한 줄 Debug 문자열로 생성합니다.
FString UCFMissileGuideComp::BuildGuidanceSummary() const
{
	// [v1.0.0] 현재 Guide Mode의 표시 이름을 찾을 Enum 정보입니다.
	const UEnum* GuideModeEnum = StaticEnum<ECFMissileGuideMode>();

	// [v1.0.0] 현재 Miss 사유의 표시 이름을 찾을 Enum 정보입니다.
	const UEnum* MissReasonEnum = StaticEnum<ECFMissileMissReason>();

	// [v1.0.0] Guide Mode를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString GuideModeText = GuideModeEnum
		? GuideModeEnum->GetNameStringByValue(static_cast<int64>(ActiveGuideConfig.GuideMode))
		: TEXT("Unknown");

	// [v1.0.0] 마지막 Miss 사유를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString MissReasonText = MissReasonEnum
		? MissReasonEnum->GetNameStringByValue(static_cast<int64>(CurrentGuidanceCommand.InvalidReason))
		: TEXT("Unknown");

	// [v1.0.0] 현재 Target Actor의 안전한 표시 이름입니다.
	const FString TargetName = GuidanceTargetActor.IsValid()
		? GuidanceTargetActor->GetName()
		: TEXT("None");

	return FString::Printf(
		TEXT("MissileGuidance: Mode=%s, Target=%s, TargetValid=%s, Lost=%.3fs, Seeker=%.1fdeg, RequestedAccel=%.1fcm/s2, AppliedAccel=%.1fcm/s2, TurnRate=%.1fdeg/s, MissReason=%s, Activations=%d"),
		*GuideModeText,
		*TargetName,
		bTargetValidThisStep ? TEXT("Yes") : TEXT("No"),
		TargetLostTimeSeconds,
		CurrentGuidanceCommand.SeekerAngleDeg,
		CurrentGuidanceCommand.RequestedLateralAccelerationCmPerSecSq.Size(),
		CurrentGuidanceCommand.AppliedLateralAccelerationCmPerSecSq.Size(),
		CurrentGuidanceCommand.AppliedTurnRateDegPerSec,
		*MissReasonText,
		GuideActivationCount);
}

// [v1.0.0] Automation에서 월드 Tick 없이 Guidance를 한 단계 진행합니다.
void UCFMissileGuideComp::AdvanceGuidanceForAutomation(const float DeltaTime)
{
	AdvanceGuidanceSimulation(DeltaTime);
}

// [v1.0.0] Flight State와 Target Snapshot을 읽어 제한형 Guidance Velocity를 갱신합니다.
void UCFMissileGuideComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AdvanceGuidanceSimulation(DeltaTime);
}

// [v1.0.0] 한 시간 구간의 목표 관측, 제한형 Guidance와 상실 정책을 처리합니다.
void UCFMissileGuideComp::AdvanceGuidanceSimulation(const float DeltaTime)
{
	if (!ActiveGuideConfig.IsGuidanceEnabled()
		|| !IsValid(ActiveProjectileMovementComponent.Get())
		|| !IsValid(ActiveMissileFlightComponent.Get()))
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::GuidanceDisabled);
		SetComponentTickEnabled(false);
		RefreshGuideSnapshot();
		return;
	}

	if (!ActiveMissileFlightComponent->IsGuidanceWindowOpen())
	{
		bTargetValidThisStep = GuidanceTargetActor.IsValid();
		InvalidateCurrentGuidance(ECFMissileMissReason::GuidanceDisabled);
		RefreshGuideSnapshot();
		return;
	}

	// [v1.0.0] 음수와 비정상 값을 제거한 이번 Guidance 시뮬레이션 시간입니다.
	const float SafeDeltaTime = FMath::IsFinite(DeltaTime)
		? FMath::Max(DeltaTime, 0.0f)
		: 0.0f;
	if (SafeDeltaTime <= KINDA_SMALL_NUMBER)
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::InvalidInput);
		RefreshGuideSnapshot();
		return;
	}

	// [v1.0.0] 현재 ProjectileMovement에서 읽은 미사일 월드 Velocity입니다.
	const FVector MissileVelocity = ActiveProjectileMovementComponent->Velocity;

	// [v1.0.0] 현재 미사일 Owner Actor의 월드 위치입니다.
	const FVector MissileLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	if (!IsFiniteMissileVector(MissileVelocity) || !IsFiniteMissileVector(MissileLocation))
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::InvalidInput);
		RefreshGuideSnapshot();
		return;
	}

	// [v1.0.0] Target Actor 또는 상실 정책에서 해석한 현재 목표 위치입니다.
	FVector TargetLocation = FVector::ZeroVector;

	// [v1.0.0] Target Actor에서 읽거나 상실 정책으로 복구한 목표 Velocity 추정값입니다.
	FVector TargetVelocity = FVector::ZeroVector;

	// [v1.0.0] Target Actor 참조와 월드 값이 현재 유효한지 여부입니다.
	bool bTargetObservationValid = TryResolveTargetObservation(TargetLocation, TargetVelocity);

	// [v1.0.0] Target Actor 관측이 실패했을 때 기록할 기본 Miss 사유입니다.
	ECFMissileMissReason ObservationFailureReason = ECFMissileMissReason::TargetLost;

	// [v1.0.0] 현재 미사일 진행 방향과 목표 시선 사이 각도입니다.
	float SeekerAngleDeg = 0.0f;

	if (bTargetObservationValid)
	{
		bTargetObservationValid = IsTargetInsideSeekerLimits(
			MissileVelocity,
			TargetLocation,
			SeekerAngleDeg,
			ObservationFailureReason);
	}

	bTargetValidThisStep = bTargetObservationValid;
	if (bTargetObservationValid)
	{
		TargetLostTimeSeconds = 0.0f;
		LastKnownTargetLocation = TargetLocation;
		bHasLastKnownTargetLocation = true;
	}
	else if (!ResolveLostTargetFallback(
		SafeDeltaTime,
		ObservationFailureReason,
		TargetLocation,
		TargetVelocity))
	{
		RefreshGuideSnapshot();
		return;
	}

	// [v1.0.0] 오버슈트 진단에 사용할 현재 목표까지의 거리입니다.
	const float CurrentTargetDistanceCm = FVector::Dist(MissileLocation, TargetLocation);
	if (HasOvershotTarget(MissileVelocity, TargetLocation, CurrentTargetDistanceCm))
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::Overshoot);
		PreviousTargetDistanceCm = CurrentTargetDistanceCm;
		bHasPreviousTargetDistance = true;
		RefreshGuideSnapshot();
		return;
	}

	PreviousTargetDistanceCm = CurrentTargetDistanceCm;
	bHasPreviousTargetDistance = true;

	// [v1.0.0] 제한형 비례항법 순수 수학에 전달할 이번 프레임 입력입니다.
	FCFMissileGuidanceInput GuidanceInput;
	GuidanceInput.MissileLocation = MissileLocation;
	GuidanceInput.MissileVelocity = MissileVelocity;
	GuidanceInput.TargetLocation = TargetLocation;
	GuidanceInput.TargetVelocityEstimate = TargetVelocity;
	GuidanceInput.DeltaSeconds = SafeDeltaTime;
	GuidanceInput.bHasTarget = true;

	// [v1.0.0] 최대 횡가속도와 최대 선회율이 이미 적용된 순수 Guidance 결과입니다.
	const FCFMissileGuidanceCommand GuidanceCommand = CFMissileGuideMath::CalculateBoundedProportionalNavigationCommand(
		GuidanceInput,
		ActiveGuideConfig);
	if (!GuidanceCommand.bCommandValid)
	{
		InvalidateCurrentGuidance(GuidanceCommand.InvalidReason);
		RefreshGuideSnapshot();
		return;
	}

	ApplyGuidanceCommand(GuidanceCommand, SafeDeltaTime);
	RefreshGuideSnapshot();
}

// [v1.0.0] 현재 Target Actor에서 유효한 위치와 Velocity를 읽습니다.
bool UCFMissileGuideComp::TryResolveTargetObservation(
	FVector& OutTargetLocation,
	FVector& OutTargetVelocity) const
{
	OutTargetLocation = FVector::ZeroVector;
	OutTargetVelocity = FVector::ZeroVector;

	if (ActiveGuideConfig.GuideMode != ECFMissileGuideMode::TargetActor)
	{
		return false;
	}

	// [v1.0.0] 발사 순간 복사한 현재 유도 목표 Actor입니다.
	AActor* TargetActor = GuidanceTargetActor.Get();
	if (!IsValid(TargetActor) || TargetActor->IsActorBeingDestroyed())
	{
		return false;
	}

	OutTargetLocation = TargetActor->GetActorLocation();
	OutTargetVelocity = TargetActor->GetVelocity();
	if (!IsFiniteMissileVector(OutTargetLocation))
	{
		return false;
	}
	if (!IsFiniteMissileVector(OutTargetVelocity))
	{
		OutTargetVelocity = FVector::ZeroVector;
	}
	return true;
}

// [v1.0.0] 목표 상실 유예와 LostTargetPolicy에 따라 마지막 지점 사용 여부를 결정합니다.
bool UCFMissileGuideComp::ResolveLostTargetFallback(
	const float DeltaTime,
	const ECFMissileMissReason ObservationFailureReason,
	FVector& OutTargetLocation,
	FVector& OutTargetVelocity)
{
	TargetLostTimeSeconds += FMath::Max(DeltaTime, 0.0f);

	// [v1.0.0] Target Lost 정책 확정 전 마지막 위치를 유지할 수 있는 유예 구간인지 여부입니다.
	const bool bWithinTargetLostGrace = TargetLostTimeSeconds
		<= ActiveGuideConfig.GetEffectiveTargetLostGraceTimeSeconds() + KINDA_SMALL_NUMBER;

	// [v1.0.0] 유예 종료 뒤에도 마지막 위치를 계속 추적하는 정책인지 여부입니다.
	const bool bHoldLastKnownPoint = ActiveGuideConfig.LostTargetPolicy
		== ECFMissileLostTargetPolicy::HoldLastKnownPoint;

	if (bHasLastKnownTargetLocation && (bWithinTargetLostGrace || bHoldLastKnownPoint))
	{
		OutTargetLocation = LastKnownTargetLocation;
		OutTargetVelocity = FVector::ZeroVector;
		return true;
	}

	// [v1.0.0] 실제 Projectile 만료 호출은 후속 단계이므로 현재 Runtime에서 기록할 최종 Miss 사유입니다.
	const ECFMissileMissReason FinalMissReason = ActiveGuideConfig.LostTargetPolicy
		== ECFMissileLostTargetPolicy::Expire
		? ECFMissileMissReason::LifeExpired
		: ObservationFailureReason;
	InvalidateCurrentGuidance(FinalMissReason);
	return false;
}

// [v1.0.0] Seeker FOV와 Lock Break 제한을 검사하고 실패 사유를 반환합니다.
bool UCFMissileGuideComp::IsTargetInsideSeekerLimits(
	const FVector& MissileVelocity,
	const FVector& TargetLocation,
	float& OutSeekerAngleDeg,
	ECFMissileMissReason& OutFailureReason) const
{
	OutSeekerAngleDeg = 0.0f;
	OutFailureReason = ECFMissileMissReason::None;

	// [v1.0.0] Seeker 기준 원점으로 사용할 현재 미사일 위치입니다.
	const FVector MissileLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	// [v1.0.0] 현재 미사일에서 Target까지의 정규화 시선 방향입니다.
	const FVector DirectionToTarget = (TargetLocation - MissileLocation).GetSafeNormal();
	if (DirectionToTarget.IsNearlyZero() || MissileVelocity.IsNearlyZero())
	{
		OutFailureReason = ECFMissileMissReason::InvalidInput;
		return false;
	}

	OutSeekerAngleDeg = CFMissileGuideMath::CalculateDirectionAngleDeg(
		MissileVelocity.GetSafeNormal(),
		DirectionToTarget);

	// [v1.0.0] 전체 Seeker FOV를 중심선 기준 반각으로 변환한 허용 각도입니다.
	const float SeekerHalfAngleDeg = ActiveGuideConfig.GetEffectiveSeekerFieldOfViewDeg() * 0.5f;

	// [v1.0.0] 이미 추적 중인 Target이 유지될 수 있는 최대 Lock Break 각도입니다.
	const float LockBreakAngleDeg = ActiveGuideConfig.GetEffectiveLockBreakAngleDeg();

	// [v1.0.0] FOV와 Lock Break 중 더 엄격한 실제 허용 각도입니다.
	const float AllowedSeekerAngleDeg = FMath::Min(SeekerHalfAngleDeg, LockBreakAngleDeg);
	if (OutSeekerAngleDeg <= AllowedSeekerAngleDeg + KINDA_SMALL_NUMBER)
	{
		return true;
	}

	OutFailureReason = OutSeekerAngleDeg > SeekerHalfAngleDeg + KINDA_SMALL_NUMBER
		? ECFMissileMissReason::SeekerFieldOfViewExceeded
		: ECFMissileMissReason::LockBreakAngleExceeded;
	return false;
}

// [v1.0.0] 거리 증가와 진행 방향을 사용해 목표를 지나친 오버슈트인지 판정합니다.
bool UCFMissileGuideComp::HasOvershotTarget(
	const FVector& MissileVelocity,
	const FVector& TargetLocation,
	const float CurrentTargetDistanceCm) const
{
	if (!bHasPreviousTargetDistance || MissileVelocity.IsNearlyZero())
	{
		return false;
	}

	// [v1.0.0] 현재 미사일에서 Target으로 향하는 월드 방향입니다.
	const FVector DirectionToTarget = (TargetLocation - (GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector)).GetSafeNormal();
	if (DirectionToTarget.IsNearlyZero())
	{
		return false;
	}

	// [v1.0.0] 현재 진행 속도가 Target 방향에 얼마나 남아 있는지 나타내는 내적값입니다.
	const float ForwardClosingVelocity = FVector::DotProduct(MissileVelocity, DirectionToTarget);

	// [v1.0.0] 부동소수점 흔들림을 무시할 최소 거리 증가 허용값입니다.
	constexpr float OvershootDistanceToleranceCm = 1.0f;
	return CurrentTargetDistanceCm > PreviousTargetDistanceCm + OvershootDistanceToleranceCm
		&& ForwardClosingVelocity <= 0.0f;
}

// [v1.0.0] 순수 Guidance Command를 응답 시간으로 보간하고 현재 속력 안에서 Velocity 방향에 적용합니다.
void UCFMissileGuideComp::ApplyGuidanceCommand(
	const FCFMissileGuidanceCommand& InGuidanceCommand,
	const float DeltaTime)
{
	UProjectileMovementComponent* ProjectileMovement = ActiveProjectileMovementComponent.Get();
	if (!ProjectileMovement || !InGuidanceCommand.bCommandValid)
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::InvalidInput);
		return;
	}

	// [v1.0.0] Guidance 적용 전 보존할 현재 미사일 속력입니다.
	const float CurrentSpeedCmPerSec = ProjectileMovement->Velocity.Size();
	if (!FMath::IsFinite(CurrentSpeedCmPerSec)
		|| CurrentSpeedCmPerSec + KINDA_SMALL_NUMBER < ActiveGuideConfig.GetEffectiveMinimumGuidanceSpeedCmPerSec())
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::BelowMinimumGuidanceSpeed);
		return;
	}

	// [v1.0.0] 목표 Command를 현재 필터 상태에 반영할 선형 응답 비율입니다.
	const float ResponseAlpha = FMath::Clamp(
		DeltaTime / ActiveGuideConfig.GetEffectiveGuidanceResponseTimeSeconds(),
		0.0f,
		1.0f);
	FilteredLateralAcceleration = FMath::Lerp(
		FilteredLateralAcceleration,
		InGuidanceCommand.AppliedLateralAccelerationCmPerSecSq,
		ResponseAlpha);

	// [v1.0.0] 현재 속력에서 최대 선회율이 허용하는 횡가속도 상한입니다.
	const float MaximumAccelerationByTurnRate = CFMissileGuideMath::CalculateTurnRateAccelerationLimitCmPerSecSq(
		CurrentSpeedCmPerSec,
		ActiveGuideConfig.GetEffectiveMaximumTurnRateDegPerSec());

	// [v1.0.0] 횡가속도와 선회율 제한을 모두 만족하는 최종 가속도 상한입니다.
	const float EffectiveMaximumAcceleration = FMath::Min(
		ActiveGuideConfig.GetEffectiveMaximumLateralAccelerationCmPerSecSq(),
		MaximumAccelerationByTurnRate);
	FilteredLateralAcceleration = FilteredLateralAcceleration.GetClampedToMaxSize(
		FMath::Max(EffectiveMaximumAcceleration, 0.0f));

	// [v1.0.0] 횡가속도를 한 시간 구간 적용한 뒤의 제한 전 방향 후보 Velocity입니다.
	const FVector DirectionCandidateVelocity = ProjectileMovement->Velocity
		+ FilteredLateralAcceleration * DeltaTime;

	// [v1.0.0] 기존 속력을 유지하면서 횡기동만 적용할 새 진행 방향입니다.
	const FVector NewVelocityDirection = DirectionCandidateVelocity.GetSafeNormal();
	if (NewVelocityDirection.IsNearlyZero() || NewVelocityDirection.ContainsNaN())
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::InvalidInput);
		return;
	}

	ProjectileMovement->Velocity = NewVelocityDirection * CurrentSpeedCmPerSec;
	CurrentGuidanceCommand = InGuidanceCommand;
	CurrentGuidanceCommand.AppliedLateralAccelerationCmPerSecSq = FilteredLateralAcceleration;
	CurrentGuidanceCommand.AppliedTurnRateDegPerSec = CurrentSpeedCmPerSec > KINDA_SMALL_NUMBER
		? FMath::RadiansToDegrees(FilteredLateralAcceleration.Size() / CurrentSpeedCmPerSec)
		: 0.0f;
	CurrentGuidanceCommand.InvalidReason = ECFMissileMissReason::None;
}

// [v1.0.0] 이번 프레임 Guidance를 적용하지 않고 지정 Miss 사유를 기록합니다.
void UCFMissileGuideComp::InvalidateCurrentGuidance(const ECFMissileMissReason MissReason)
{
	FilteredLateralAcceleration = FVector::ZeroVector;
	CurrentGuidanceCommand = FCFMissileGuidanceCommand();
	CurrentGuidanceCommand.InvalidReason = MissReason;
}

// [v1.0.0] 현재 내부 값을 Blueprint 읽기용 Guidance 스냅샷에 반영합니다.
void UCFMissileGuideComp::RefreshGuideSnapshot()
{
	CurrentGuideSnapshot.GuideMode = ActiveGuideConfig.GuideMode;
	CurrentGuideSnapshot.bTargetValid = bTargetValidThisStep;
	CurrentGuideSnapshot.TargetLocation = LastKnownTargetLocation;
	CurrentGuideSnapshot.SeekerAngleDeg = CurrentGuidanceCommand.SeekerAngleDeg;
	CurrentGuideSnapshot.RequestedLateralAccelerationCmPerSecSq = CurrentGuidanceCommand.RequestedLateralAccelerationCmPerSecSq;
	CurrentGuideSnapshot.AppliedLateralAccelerationCmPerSecSq = CurrentGuidanceCommand.AppliedLateralAccelerationCmPerSecSq;
	CurrentGuideSnapshot.TargetLostTimeSeconds = TargetLostTimeSeconds;
	CurrentGuideSnapshot.MissReason = CurrentGuidanceCommand.InvalidReason;
}
