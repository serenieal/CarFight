// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.1
// Date: 2026-09-07
// Description: CarFight 물리 제한형 미사일 유도 컴포넌트 구현
// Scope: Target Actor Snapshot, Guidance Activation, Legacy/Stateful Seeker, Direct/Sampled 관측·속도 추정, Guidance Law, rear-aspect·오버슈트와 Velocity 방향 적용을 구현합니다.
// Changelog:
// - v1.4.1: MG-P0-12D 최종검수 P1 교정으로 exact rear Launch Right tie-break를 모든 bounded Pursuit 계열(Pure/Lead/PN Course Capture)의 공통 helper로 통합.
// - v1.4.0: MG-P0-12D Independent Guidance Activation latch, free Stateful Seeker geometry, approach-armed Overshoot와 Launch Right 기반 exact rear tie-break를 추가.
// - v1.3.0: MG-P0-12C PurePursuit/LeadPursuit/ProportionalNavigation 전략, bounded lead aim point와 rear/non-closing PN Course Capture를 추가.
// - v1.2.0: MG-P0-10 SampledPositionEstimate의 관측 주기, 위치 차분 속도 추정, 응답 필터, 관측 사이 외삽과 Estimated Target State 단일 소비 경로를 추가.
// - v1.1.0: MG-P0-09 Stateful Seeker 상태 전이, Acquisition/Tracking/Reacquisition 반각, LostGrace, LostFinal Hold point와 명시적 재포착을 추가.
// - v1.0.0: MG-P0-03~04 Direct TargetActor Guidance Runtime 최초 구현.
// Migration:
// - ProportionalNavigation + FollowFlightGuidanceWindow + LegacySingleGate + DirectActorKinematics 기본값은 기존 Flight Guidance Window, 단일 Gate와 기존 PN/Target Actor 직접 관측 동작을 보존합니다.
// - Independent Guidance Activation은 GuideComp 내부 latch만 추가하며 FlightComp 상태 머신과 Clearance 의미를 변경하지 않습니다.
// - Stateful Overshoot만 approach-armed 의미를 사용하고 LegacySingleGate는 기존 즉시 Overshoot 의미를 유지합니다.
// - PurePursuit와 LeadPursuit도 기존 GuidanceResponseTime, MaximumTurnRate, MaximumLateralAcceleration 제한을 동일하게 거칩니다.
// - SampledPositionEstimate는 Target Actor 위치만 설정 주기로 읽고 Actor GetVelocity를 Guidance 정답으로 소비하지 않습니다.
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
	LaunchRightVector = InLaunchContext.LaunchTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();
	++GuideActivationCount;

	// [v1.2.0] 발사 순간 Target 위치를 최초 Sensor seed로 저장합니다. Sampled 모드에서는 Actor Velocity를 읽지 않습니다.
	InitializeTargetObservationSeed();

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
	LaunchRightVector = FVector::ZeroVector;
	CurrentSeekerState = ECFMissileSeekerState::Inactive;
	StatefulFinalMissReason = ECFMissileMissReason::None;
	LastKnownTargetLocation = FVector::ZeroVector;
	PreviousObservedTargetLocation = FVector::ZeroVector;
	LastObservedTargetLocation = FVector::ZeroVector;
	EstimatedTargetLocation = FVector::ZeroVector;
	FilteredTargetVelocityEstimate = FVector::ZeroVector;
	HoldTargetLocation = FVector::ZeroVector;
	GuidanceAimPoint = FVector::ZeroVector;
	bCourseCaptureActive = false;
	bGuidanceActivationSatisfied = false;
	bOvershootArmed = false;
	FilteredLateralAcceleration = FVector::ZeroVector;
	PreviousTargetDistanceCm = 0.0f;
	TargetLostTimeSeconds = 0.0f;
	ObservationAgeSeconds = 0.0f;
	ReacquisitionElapsedTimeSeconds = 0.0f;
	bHasLastKnownTargetLocation = false;
	bHasPreviousTargetDistance = false;
	bHasPreviousObservedTargetLocation = false;
	bHasValidObservation = false;
	bHasHoldTargetLocation = false;
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

	// [v1.1.0] 현재 Seeker 상태의 표시 이름을 찾을 Enum 정보입니다.
	const UEnum* SeekerStateEnum = StaticEnum<ECFMissileSeekerState>();

	// [v1.3.0] 현재 Guidance Law의 표시 이름을 찾을 Enum 정보입니다.
	const UEnum* GuidanceLawEnum = StaticEnum<ECFMissileGuidanceLaw>();

	// [v1.0.0] Guide Mode를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString GuideModeText = GuideModeEnum
		? GuideModeEnum->GetNameStringByValue(static_cast<int64>(ActiveGuideConfig.GuideMode))
		: TEXT("Unknown");

	// [v1.1.0] 현재 Stateful Seeker 상태를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString SeekerStateText = SeekerStateEnum
		? SeekerStateEnum->GetNameStringByValue(static_cast<int64>(CurrentSeekerState))
		: TEXT("Unknown");

	// [v1.3.0] 현재 Guidance Law를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString GuidanceLawText = GuidanceLawEnum
		? GuidanceLawEnum->GetNameStringByValue(static_cast<int64>(ActiveGuideConfig.GuidanceLaw))
		: TEXT("Unknown");

	// [v1.1.0] Stateful LostFinal에서는 Command 유효성과 독립적으로 보존할 최종 Miss 사유입니다.
	const ECFMissileMissReason SummaryMissReason = ActiveGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful
		&& StatefulFinalMissReason != ECFMissileMissReason::None
		? StatefulFinalMissReason
		: CurrentGuidanceCommand.InvalidReason;

	// [v1.0.0] 마지막 Miss 사유를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString MissReasonText = MissReasonEnum
		? MissReasonEnum->GetNameStringByValue(static_cast<int64>(SummaryMissReason))
		: TEXT("Unknown");

	// [v1.0.0] 현재 Target Actor의 안전한 표시 이름입니다.
	const FString TargetName = GuidanceTargetActor.IsValid()
		? GuidanceTargetActor->GetName()
		: TEXT("None");

	return FString::Printf(
		TEXT("MissileGuidance: Mode=%s, Law=%s, SeekerState=%s, CourseCapture=%s, Target=%s, TargetValid=%s, Lost=%.3fs, Reacquire=%.3fs, Seeker=%.1fdeg, RequestedAccel=%.1fcm/s2, AppliedAccel=%.1fcm/s2, TurnRate=%.1fdeg/s, MissReason=%s, Activations=%d"),
		*GuideModeText,
		*GuidanceLawText,
		*SeekerStateText,
		bCourseCaptureActive ? TEXT("Yes") : TEXT("No"),
		*TargetName,
		bTargetValidThisStep ? TEXT("Yes") : TEXT("No"),
		TargetLostTimeSeconds,
		ReacquisitionElapsedTimeSeconds,
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

// [v1.1.0] 한 시간 구간의 목표 관측, Legacy/Stateful Seeker, 제한형 Guidance와 상실 정책을 처리합니다.
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

	// [v1.1.0] 음수와 비정상 값을 제거한 이번 Guidance 시뮬레이션 시간입니다.
	const float SafeDeltaTime = FMath::IsFinite(DeltaTime)
		? FMath::Max(DeltaTime, 0.0f)
		: 0.0f;
	if (SafeDeltaTime <= KINDA_SMALL_NUMBER)
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::InvalidInput);
		RefreshGuideSnapshot();
		return;
	}

	if (!ResolveGuidanceActivation())
	{
		bTargetValidThisStep = ActiveGuideConfig.SeekerModel == ECFMissileSeekerModel::LegacySingleGate
			&& GuidanceTargetActor.IsValid();
		if (ActiveGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate)
		{
			ObservationAgeSeconds += SafeDeltaTime;
		}
		InvalidateCurrentGuidance(ECFMissileMissReason::GuidanceDisabled);
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

	if (ActiveGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful)
	{
		if (!ResolveStatefulGuidanceTarget(
			SafeDeltaTime,
			MissileLocation,
			MissileVelocity,
			TargetLocation,
			TargetVelocity))
		{
			RefreshGuideSnapshot();
			return;
		}
	}
	else
	{
		// [v1.0.0] Target Actor 참조와 월드 값이 현재 유효한지 여부입니다.
		bool bTargetObservationValid = TryResolveTargetObservation(SafeDeltaTime, TargetLocation, TargetVelocity);

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
	}

	// [v1.0.0] 오버슈트 진단에 사용할 현재 목표까지의 거리입니다.
	const float CurrentTargetDistanceCm = FVector::Dist(MissileLocation, TargetLocation);
	if (ActiveGuideConfig.SeekerModel == ECFMissileSeekerModel::LegacySingleGate
		&& HasOvershotTarget(MissileVelocity, TargetLocation, CurrentTargetDistanceCm))
	{
		InvalidateCurrentGuidance(ECFMissileMissReason::Overshoot);
		PreviousTargetDistanceCm = CurrentTargetDistanceCm;
		bHasPreviousTargetDistance = true;
		RefreshGuideSnapshot();
		return;
	}

	PreviousTargetDistanceCm = CurrentTargetDistanceCm;
	bHasPreviousTargetDistance = true;

	// [v1.3.0] 현재 Guidance Law가 허용한 Sensor State만 소비해 생성한 물리 제한형 Guidance 결과입니다.
	const FCFMissileGuidanceCommand GuidanceCommand = BuildGuidanceCommandForActiveLaw(
		MissileLocation,
		MissileVelocity,
		TargetLocation,
		TargetVelocity,
		SafeDeltaTime);
	if (!GuidanceCommand.bCommandValid)
	{
		InvalidateCurrentGuidance(GuidanceCommand.InvalidReason);
		RefreshGuideSnapshot();
		return;
	}

	ApplyGuidanceCommand(GuidanceCommand, SafeDeltaTime);
	RefreshGuideSnapshot();
}

// [v1.4.0] 현재 Activation Mode에 따라 Flight Guidance Window 또는 독립 시간·거리 AND 조건을 판정하고 Independent 만족 상태를 latch합니다.
bool UCFMissileGuideComp::ResolveGuidanceActivation()
{
	UCFMissileFlightComp* MissileFlightComponent = ActiveMissileFlightComponent.Get();
	if (!IsValid(MissileFlightComponent))
	{
		bGuidanceActivationSatisfied = false;
		return false;
	}

	if (ActiveGuideConfig.GuidanceActivationMode == ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow)
	{
		bGuidanceActivationSatisfied = MissileFlightComponent->IsGuidanceWindowOpen();
		return bGuidanceActivationSatisfied;
	}

	if (bGuidanceActivationSatisfied)
	{
		return true;
	}

	// [v1.4.0] FlightComp가 소유하는 현재 발사 후 경과 시간과 분리 거리 Snapshot입니다.
	const FCFMissileFlightSnapshot FlightSnapshot = MissileFlightComponent->GetFlightSnapshot();

	// [v1.4.0] 0이면 즉시 만족하고 양수이면 실제 경과 비행 시간이 설정값 이상인지 확인하는 시간 조건입니다.
	const float ActivationDelaySeconds = ActiveGuideConfig.GetEffectiveGuidanceActivationDelaySeconds();
	const bool bDelaySatisfied = ActivationDelaySeconds <= KINDA_SMALL_NUMBER
		|| FlightSnapshot.ElapsedFlightTimeSeconds + KINDA_SMALL_NUMBER >= ActivationDelaySeconds;

	// [v1.4.0] 0이면 즉시 만족하고 양수이면 실제 분리 거리가 설정값 이상인지 확인하는 거리 조건입니다.
	const float ActivationDistanceCm = ActiveGuideConfig.GetEffectiveGuidanceActivationDistanceCm();
	const bool bDistanceSatisfied = ActivationDistanceCm <= KINDA_SMALL_NUMBER
		|| FlightSnapshot.DistanceFromReleaseCm + KINDA_SMALL_NUMBER >= ActivationDistanceCm;

	bGuidanceActivationSatisfied = bDelaySatisfied && bDistanceSatisfied;
	return bGuidanceActivationSatisfied;
}

// [v1.2.0] 발사 순간 Target Snapshot을 최초 관측 seed로 저장하며 Sampled 모드에서는 Actor Velocity를 읽지 않습니다.
bool UCFMissileGuideComp::InitializeTargetObservationSeed()
{
	if (!IsGuidanceTargetActorValid())
	{
		return false;
	}

	// [v1.2.0] 발사 순간 최초 Sensor seed로 사용할 실제 Target Actor입니다.
	AActor* TargetActor = GuidanceTargetActor.Get();

	// [v1.2.0] Direct/Sampled 모두 발사 순간 한 번만 취득하는 최초 Target 위치 seed입니다.
	const FVector InitialTargetLocation = TargetActor->GetActorLocation();
	if (!IsFiniteMissileVector(InitialTargetLocation))
	{
		return false;
	}

	// [v1.2.0] Direct 모드에서만 기존 Actor kinematics 호환을 위해 읽는 최초 Target Velocity입니다.
	FVector InitialTargetVelocity = FVector::ZeroVector;
	if (ActiveGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::DirectActorKinematics)
	{
		InitialTargetVelocity = TargetActor->GetVelocity();
		if (!IsFiniteMissileVector(InitialTargetVelocity))
		{
			InitialTargetVelocity = FVector::ZeroVector;
		}
	}

	LastKnownTargetLocation = InitialTargetLocation;
	PreviousObservedTargetLocation = InitialTargetLocation;
	LastObservedTargetLocation = InitialTargetLocation;
	EstimatedTargetLocation = InitialTargetLocation;
	FilteredTargetVelocityEstimate = InitialTargetVelocity;
	ObservationAgeSeconds = 0.0f;
	bHasLastKnownTargetLocation = true;
	bHasPreviousObservedTargetLocation = true;
	bHasValidObservation = true;
	bTargetValidThisStep = ActiveGuideConfig.SeekerModel == ECFMissileSeekerModel::LegacySingleGate;
	return true;
}

// [v1.2.0] 현재 관측 모델에 따라 Direct Actor 상태 또는 Sampled Estimated Target State를 반환합니다.
bool UCFMissileGuideComp::TryResolveTargetObservation(
	const float DeltaTime,
	FVector& OutTargetLocation,
	FVector& OutTargetVelocity)
{
	if (ActiveGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate)
	{
		return AdvanceSampledTargetObservation(DeltaTime, OutTargetLocation, OutTargetVelocity);
	}

	return ResolveDirectActorObservation(OutTargetLocation, OutTargetVelocity);
}

// [v1.2.0] DirectActorKinematics에서 Target Actor의 현재 위치와 Velocity를 직접 읽고 Debug 관측 상태를 동기화합니다.
bool UCFMissileGuideComp::ResolveDirectActorObservation(
	FVector& OutTargetLocation,
	FVector& OutTargetVelocity)
{
	OutTargetLocation = FVector::ZeroVector;
	OutTargetVelocity = FVector::ZeroVector;

	if (!IsGuidanceTargetActorValid())
	{
		return false;
	}

	// [v1.2.0] DirectActorKinematics가 매 Guidance Tick 직접 읽는 Target Actor입니다.
	AActor* TargetActor = GuidanceTargetActor.Get();
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

	PreviousObservedTargetLocation = LastObservedTargetLocation;
	LastObservedTargetLocation = OutTargetLocation;
	EstimatedTargetLocation = OutTargetLocation;
	FilteredTargetVelocityEstimate = OutTargetVelocity;
	ObservationAgeSeconds = 0.0f;
	bHasPreviousObservedTargetLocation = bHasValidObservation;
	bHasValidObservation = true;
	return true;
}

// [v1.2.0] SampledPositionEstimate에서 관측 주기마다 위치만 읽고 속도를 위치 차분으로 추정하며 관측 사이 위치를 외삽합니다.
bool UCFMissileGuideComp::AdvanceSampledTargetObservation(
	const float DeltaTime,
	FVector& OutTargetLocation,
	FVector& OutTargetVelocity)
{
	OutTargetLocation = EstimatedTargetLocation;
	OutTargetVelocity = FilteredTargetVelocityEstimate;

	// [v1.2.0] 비정상 DeltaTime을 배제하고 마지막 실제 위치 Sample 이후 실제 경과 시간을 누적합니다.
	const float SafeObservationDeltaTime = FMath::IsFinite(DeltaTime)
		? FMath::Max(DeltaTime, 0.0f)
		: 0.0f;
	ObservationAgeSeconds += SafeObservationDeltaTime;

	if (bHasValidObservation)
	{
		EstimatedTargetLocation = LastObservedTargetLocation
			+ FilteredTargetVelocityEstimate * ObservationAgeSeconds;
		OutTargetLocation = EstimatedTargetLocation;
		OutTargetVelocity = FilteredTargetVelocityEstimate;
	}

	// [v1.2.0] Actor 수명 무효는 숨은 위치를 새로 읽지 않고 즉시 Target observation invalid로 반환합니다.
	if (!IsGuidanceTargetActorValid())
	{
		return false;
	}

	// [v1.2.0] Launch seed가 없던 예외 activation에서 최초 위치 Sample을 한 번 취득할지 여부입니다.
	const bool bNeedsInitialSample = !bHasValidObservation;

	// [v1.2.0] 마지막 실제 위치 Sample 이후 설정된 관측 간격 이상이 지났는지 여부입니다.
	const bool bObservationIntervalElapsed = ObservationAgeSeconds + KINDA_SMALL_NUMBER
		>= ActiveGuideConfig.GetEffectiveTargetObservationIntervalSeconds();
	if (!bNeedsInitialSample && !bObservationIntervalElapsed)
	{
		return IsFiniteMissileVector(OutTargetLocation)
			&& IsFiniteMissileVector(OutTargetVelocity);
	}

	// [v1.2.0] 이번 Guidance Tick에서 최대 한 번만 실제 위치를 읽을 Target Actor입니다.
	AActor* TargetActor = GuidanceTargetActor.Get();

	// [v1.2.0] Sampled 모드가 이번 관측 시점에 취득하는 유일한 실제 Target 운동 정보인 위치입니다.
	const FVector NewObservedTargetLocation = TargetActor->GetActorLocation();
	if (!IsFiniteMissileVector(NewObservedTargetLocation))
	{
		return false;
	}

	if (bHasValidObservation && ObservationAgeSeconds > KINDA_SMALL_NUMBER)
	{
		// [v1.2.0] 이전 실제 Sample과 이번 Sample 사이 누적 실제 시간으로 계산한 위치 차분 속도입니다.
		const FVector RawTargetVelocityEstimate =
			(NewObservedTargetLocation - LastObservedTargetLocation) / ObservationAgeSeconds;

		// [v1.2.0] 속도 추정값이 설정된 응답 시간에 따라 새 위치 차분을 따라갈 비율입니다.
		const float VelocityEstimateResponseAlpha = FMath::Clamp(
			ObservationAgeSeconds / ActiveGuideConfig.GetEffectiveTargetVelocityEstimateResponseTimeSeconds(),
			0.0f,
			1.0f);
		FilteredTargetVelocityEstimate = FMath::Lerp(
			FilteredTargetVelocityEstimate,
			IsFiniteMissileVector(RawTargetVelocityEstimate) ? RawTargetVelocityEstimate : FVector::ZeroVector,
			VelocityEstimateResponseAlpha);
	}
	else
	{
		FilteredTargetVelocityEstimate = FVector::ZeroVector;
	}

	PreviousObservedTargetLocation = LastObservedTargetLocation;
	LastObservedTargetLocation = NewObservedTargetLocation;
	EstimatedTargetLocation = NewObservedTargetLocation;
	ObservationAgeSeconds = 0.0f;
	bHasPreviousObservedTargetLocation = bHasValidObservation;
	bHasValidObservation = true;

	OutTargetLocation = EstimatedTargetLocation;
	OutTargetVelocity = FilteredTargetVelocityEstimate;
	return IsFiniteMissileVector(OutTargetLocation)
		&& IsFiniteMissileVector(OutTargetVelocity);
}

// [v1.2.0] Launch Snapshot Target Actor 참조가 현재 관측 가능한 수명인지 반환합니다.
bool UCFMissileGuideComp::IsGuidanceTargetActorValid() const
{
	if (ActiveGuideConfig.GuideMode != ECFMissileGuideMode::TargetActor)
	{
		return false;
	}

	// [v1.2.0] 발사 순간 복사한 현재 유도 목표 Actor입니다.
	AActor* TargetActor = GuidanceTargetActor.Get();
	return IsValid(TargetActor) && !TargetActor->IsActorBeingDestroyed();
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

// [v1.1.0] Stateful Seeker 상태를 진행하고 이번 Step에서 Guidance가 소비할 목표 상태가 있으면 반환합니다.
bool UCFMissileGuideComp::ResolveStatefulGuidanceTarget(
	const float DeltaTime,
	const FVector& MissileLocation,
	const FVector& MissileVelocity,
	FVector& OutTargetLocation,
	FVector& OutTargetVelocity)
{
	OutTargetLocation = FVector::ZeroVector;
	OutTargetVelocity = FVector::ZeroVector;
	bTargetValidThisStep = false;

	if (CurrentSeekerState == ECFMissileSeekerState::LostFinal)
	{
		return ResolveStatefulLostFinalGuidanceTarget(OutTargetLocation, OutTargetVelocity);
	}

	if (CurrentSeekerState == ECFMissileSeekerState::Inactive)
	{
		CurrentSeekerState = ECFMissileSeekerState::Acquiring;
	}

	// [v1.2.0] 현재 관측 모델이 반환한 단일 Sensor Truth Target 위치입니다. Sampled에서는 EstimatedTargetLocation입니다.
	FVector ObservedTargetLocation = EstimatedTargetLocation;

	// [v1.2.0] 현재 관측 모델이 반환한 Target Velocity입니다. Sampled에서는 위치 차분 기반 Filtered estimate입니다.
	FVector ObservedTargetVelocity = FilteredTargetVelocityEstimate;

	// [v1.2.0] 같은 Launch Snapshot Target을 현재 관측 모델로 해석할 수 있는지 여부입니다.
	const bool bObservationValid = TryResolveTargetObservation(
		DeltaTime,
		ObservedTargetLocation,
		ObservedTargetVelocity);

	// [v1.1.0] 현재 Stateful cone 판정에서 계산한 중심선 기준 Target 각도입니다.
	float StatefulSeekerAngleDeg = 0.0f;

	if (bObservationValid
		&& (CurrentSeekerState == ECFMissileSeekerState::Tracking
			|| CurrentSeekerState == ECFMissileSeekerState::LostGrace
			|| CurrentSeekerState == ECFMissileSeekerState::Reacquiring))
	{
		// [v1.4.0] Stateful approach-armed Overshoot가 현재 Sensor Truth로 판정할 목표 거리입니다.
		const float StatefulTargetDistanceCm = FVector::Dist(MissileLocation, ObservedTargetLocation);
		if (UpdateStatefulOvershootState(
			MissileLocation,
			MissileVelocity,
			ObservedTargetLocation,
			ObservedTargetVelocity,
			StatefulTargetDistanceCm))
		{
			EnterStatefulLostFinal(ECFMissileMissReason::Overshoot, ObservedTargetLocation, true);
			return false;
		}
	}

	if (CurrentSeekerState == ECFMissileSeekerState::Acquiring)
	{
		if (!bObservationValid)
		{
			// [v1.2.0] Sampled는 Actor가 사라져도 마지막 estimator 외삽 지점을 Sensor Truth로 보존합니다.
			const FVector LostSensorTruthLocation = ActiveGuideConfig.TargetObservationMode
				== ECFMissileTargetObservationMode::SampledPositionEstimate
				? EstimatedTargetLocation
				: LastObservedTargetLocation;
			EnterStatefulLostFinal(
				ECFMissileMissReason::TargetLost,
				LostSensorTruthLocation,
				bHasValidObservation);
			return ResolveStatefulLostFinalGuidanceTarget(OutTargetLocation, OutTargetVelocity);
		}

		if (!IsTargetInsideStatefulCone(
			MissileVelocity,
			ObservedTargetLocation,
			ActiveGuideConfig.GetEffectiveAcquisitionConeHalfAngleDeg(),
			StatefulSeekerAngleDeg))
		{
			InvalidateCurrentGuidance(ECFMissileMissReason::None);
			CurrentGuidanceCommand.SeekerAngleDeg = StatefulSeekerAngleDeg;
			return false;
		}

		CurrentSeekerState = ECFMissileSeekerState::Tracking;
		TargetLostTimeSeconds = 0.0f;
		ReacquisitionElapsedTimeSeconds = 0.0f;
		LastKnownTargetLocation = ObservedTargetLocation;
		bHasLastKnownTargetLocation = true;
		bTargetValidThisStep = true;
		OutTargetLocation = ObservedTargetLocation;
		OutTargetVelocity = ObservedTargetVelocity;
		return true;
	}

	if (CurrentSeekerState == ECFMissileSeekerState::Tracking)
	{
		if (bObservationValid
			&& IsTargetInsideStatefulCone(
				MissileVelocity,
				ObservedTargetLocation,
				ActiveGuideConfig.GetEffectiveTrackingConeHalfAngleDeg(),
				StatefulSeekerAngleDeg))
		{
			TargetLostTimeSeconds = 0.0f;
			ReacquisitionElapsedTimeSeconds = 0.0f;
			LastKnownTargetLocation = ObservedTargetLocation;
			bHasLastKnownTargetLocation = true;
			bTargetValidThisStep = true;
			OutTargetLocation = ObservedTargetLocation;
			OutTargetVelocity = ObservedTargetVelocity;
			return true;
		}

		CurrentSeekerState = ECFMissileSeekerState::LostGrace;
		TargetLostTimeSeconds = 0.0f;
		ReacquisitionElapsedTimeSeconds = 0.0f;
	}

	if (CurrentSeekerState == ECFMissileSeekerState::LostGrace)
	{
		if (bObservationValid
			&& IsTargetInsideStatefulCone(
				MissileVelocity,
				ObservedTargetLocation,
				ActiveGuideConfig.GetEffectiveTrackingConeHalfAngleDeg(),
				StatefulSeekerAngleDeg))
		{
			CurrentSeekerState = ECFMissileSeekerState::Tracking;
			TargetLostTimeSeconds = 0.0f;
			ReacquisitionElapsedTimeSeconds = 0.0f;
			LastKnownTargetLocation = ObservedTargetLocation;
			bHasLastKnownTargetLocation = true;
			bTargetValidThisStep = true;
			OutTargetLocation = ObservedTargetLocation;
			OutTargetVelocity = ObservedTargetVelocity;
			return true;
		}

		TargetLostTimeSeconds += FMath::Max(DeltaTime, 0.0f);

		// [v1.1.0] 한 번 Tracking이 성립한 뒤 마지막 보존 지점을 제한형 Guidance에 사용할 수 있는 유예 시간입니다.
		const float TargetLostGraceTimeSeconds = ActiveGuideConfig.GetEffectiveTargetLostGraceTimeSeconds();
		if (TargetLostGraceTimeSeconds > KINDA_SMALL_NUMBER
			&& TargetLostTimeSeconds <= TargetLostGraceTimeSeconds + KINDA_SMALL_NUMBER)
		{
			if (ActiveGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate
				&& bHasValidObservation)
			{
				// [v1.2.0] Sampled LostGrace도 같은 Estimated Target State를 PN 입력으로 사용해 숨은 Actor truth와 경로가 갈라지지 않게 합니다.
				LastKnownTargetLocation = EstimatedTargetLocation;
				bHasLastKnownTargetLocation = true;
				OutTargetLocation = EstimatedTargetLocation;
				OutTargetVelocity = FilteredTargetVelocityEstimate;
				return true;
			}

			if (bHasLastKnownTargetLocation)
			{
				OutTargetLocation = LastKnownTargetLocation;
				OutTargetVelocity = FVector::ZeroVector;
				return true;
			}

			InvalidateCurrentGuidance(ECFMissileMissReason::None);
			CurrentGuidanceCommand.SeekerAngleDeg = StatefulSeekerAngleDeg;
			return false;
		}

		if (!bObservationValid)
		{
			// [v1.2.0] LostGrace 종료 시 Sampled Hold 후보는 fresh Actor truth가 아니라 현재 Estimated Sensor Truth입니다.
			const FVector LostSensorTruthLocation = ActiveGuideConfig.TargetObservationMode
				== ECFMissileTargetObservationMode::SampledPositionEstimate
				? EstimatedTargetLocation
				: LastObservedTargetLocation;
			EnterStatefulLostFinal(
				ECFMissileMissReason::TargetLost,
				LostSensorTruthLocation,
				bHasValidObservation);
			return ResolveStatefulLostFinalGuidanceTarget(OutTargetLocation, OutTargetVelocity);
		}

		if (ActiveGuideConfig.ReacquisitionMode == ECFMissileReacquisitionMode::ForwardCone
			&& ActiveGuideConfig.GetEffectiveReacquisitionTimeSeconds() > KINDA_SMALL_NUMBER)
		{
			CurrentSeekerState = ECFMissileSeekerState::Reacquiring;
			ReacquisitionElapsedTimeSeconds = 0.0f;
			InvalidateCurrentGuidance(ECFMissileMissReason::None);
			CurrentGuidanceCommand.SeekerAngleDeg = StatefulSeekerAngleDeg;
			return false;
		}

		EnterStatefulLostFinal(ECFMissileMissReason::TargetLost, ObservedTargetLocation, true);
		return ResolveStatefulLostFinalGuidanceTarget(OutTargetLocation, OutTargetVelocity);
	}

	if (CurrentSeekerState == ECFMissileSeekerState::Reacquiring)
	{
		if (!bObservationValid)
		{
			// [v1.2.0] Reacquisition 중 Target 소실도 Sampled estimator의 마지막 Sensor Truth에서 종결합니다.
			const FVector LostSensorTruthLocation = ActiveGuideConfig.TargetObservationMode
				== ECFMissileTargetObservationMode::SampledPositionEstimate
				? EstimatedTargetLocation
				: LastObservedTargetLocation;
			EnterStatefulLostFinal(
				ECFMissileMissReason::TargetLost,
				LostSensorTruthLocation,
				bHasValidObservation);
			return ResolveStatefulLostFinalGuidanceTarget(OutTargetLocation, OutTargetVelocity);
		}

		if (IsTargetInsideStatefulCone(
			MissileVelocity,
			ObservedTargetLocation,
			ActiveGuideConfig.GetEffectiveReacquisitionConeHalfAngleDeg(),
			StatefulSeekerAngleDeg))
		{
			CurrentSeekerState = ECFMissileSeekerState::Tracking;
			TargetLostTimeSeconds = 0.0f;
			ReacquisitionElapsedTimeSeconds = 0.0f;
			LastKnownTargetLocation = ObservedTargetLocation;
			bHasLastKnownTargetLocation = true;
			bTargetValidThisStep = true;
			OutTargetLocation = ObservedTargetLocation;
			OutTargetVelocity = ObservedTargetVelocity;
			return true;
		}

		ReacquisitionElapsedTimeSeconds += FMath::Max(DeltaTime, 0.0f);
		if (ReacquisitionElapsedTimeSeconds + KINDA_SMALL_NUMBER
			>= ActiveGuideConfig.GetEffectiveReacquisitionTimeSeconds())
		{
			EnterStatefulLostFinal(ECFMissileMissReason::TargetLost, ObservedTargetLocation, true);
			return ResolveStatefulLostFinalGuidanceTarget(OutTargetLocation, OutTargetVelocity);
		}

		InvalidateCurrentGuidance(ECFMissileMissReason::None);
		CurrentGuidanceCommand.SeekerAngleDeg = StatefulSeekerAngleDeg;
		return false;
	}

	InvalidateCurrentGuidance(ECFMissileMissReason::InvalidInput);
	return false;
}

// [v1.1.0] Stateful 중심선 기준 반각 안에 목표가 있는지 검사하고 현재 Seeker 각도를 반환합니다.
bool UCFMissileGuideComp::IsTargetInsideStatefulCone(
	const FVector& MissileVelocity,
	const FVector& TargetLocation,
	const float AllowedHalfAngleDeg,
	float& OutSeekerAngleDeg) const
{
	OutSeekerAngleDeg = 0.0f;

	// [v1.1.0] Stateful Seeker 기준 원점으로 사용할 현재 미사일 위치입니다.
	const FVector MissileLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	// [v1.1.0] Stateful Seeker가 판정할 현재 Target 시선 방향입니다.
	const FVector DirectionToTarget = (TargetLocation - MissileLocation).GetSafeNormal();
	if (DirectionToTarget.IsNearlyZero() || MissileVelocity.IsNearlyZero())
	{
		return false;
	}

	OutSeekerAngleDeg = CFMissileGuideMath::CalculateDirectionAngleDeg(
		MissileVelocity.GetSafeNormal(),
		DirectionToTarget);

	// [v1.1.0] 비정상 입력을 제거한 중심선 기준 허용 반각입니다.
	const float SafeAllowedHalfAngleDeg = FMath::IsFinite(AllowedHalfAngleDeg)
		? FMath::Clamp(AllowedHalfAngleDeg, 0.0f, 180.0f)
		: 0.0f;
	return OutSeekerAngleDeg <= SafeAllowedHalfAngleDeg + KINDA_SMALL_NUMBER;
}

// [v1.1.0] Stateful Seeker를 최종 상실 상태로 전환하고 HoldLastKnownPoint가 필요하면 Sensor Truth 지점을 한 번 고정합니다.
void UCFMissileGuideComp::EnterStatefulLostFinal(
	const ECFMissileMissReason FinalMissReason,
	const FVector& SensorTruthLocation,
	const bool bHasSensorTruthLocation)
{
	CurrentSeekerState = ECFMissileSeekerState::LostFinal;
	bTargetValidThisStep = false;
	bHasHoldTargetLocation = false;
	HoldTargetLocation = FVector::ZeroVector;

	StatefulFinalMissReason = FinalMissReason == ECFMissileMissReason::TargetLost
		&& ActiveGuideConfig.LostTargetPolicy == ECFMissileLostTargetPolicy::Expire
		? ECFMissileMissReason::LifeExpired
		: FinalMissReason;

	if (StatefulFinalMissReason == ECFMissileMissReason::TargetLost
		&& ActiveGuideConfig.LostTargetPolicy == ECFMissileLostTargetPolicy::HoldLastKnownPoint)
	{
		if (bHasSensorTruthLocation && IsFiniteMissileVector(SensorTruthLocation))
		{
			HoldTargetLocation = SensorTruthLocation;
			bHasHoldTargetLocation = true;
		}
		else if (bHasLastKnownTargetLocation)
		{
			HoldTargetLocation = LastKnownTargetLocation;
			bHasHoldTargetLocation = true;
		}

		if (bHasHoldTargetLocation)
		{
			LastKnownTargetLocation = HoldTargetLocation;
			bHasLastKnownTargetLocation = true;
		}
	}

	InvalidateCurrentGuidance(StatefulFinalMissReason);
	if (!bHasHoldTargetLocation)
	{
		SetComponentTickEnabled(false);
	}
}

// [v1.1.0] LostFinal에서 HoldLastKnownPoint만 고정 지점 Guidance를 계속할 수 있게 반환합니다.
bool UCFMissileGuideComp::ResolveStatefulLostFinalGuidanceTarget(
	FVector& OutTargetLocation,
	FVector& OutTargetVelocity)
{
	OutTargetLocation = FVector::ZeroVector;
	OutTargetVelocity = FVector::ZeroVector;
	bTargetValidThisStep = false;

	if (StatefulFinalMissReason == ECFMissileMissReason::TargetLost
		&& ActiveGuideConfig.LostTargetPolicy == ECFMissileLostTargetPolicy::HoldLastKnownPoint
		&& bHasHoldTargetLocation)
	{
		OutTargetLocation = HoldTargetLocation;
		OutTargetVelocity = FVector::ZeroVector;
		return true;
	}

	InvalidateCurrentGuidance(StatefulFinalMissReason);
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

// [v1.4.0] Stateful Seeker에서 실제 접근을 한 번 확인한 뒤에만 Overshoot 판정을 arm하고 armed 상태의 비접근 거리 증가를 종결로 판정합니다.
bool UCFMissileGuideComp::UpdateStatefulOvershootState(
	const FVector& MissileLocation,
	const FVector& MissileVelocity,
	const FVector& TargetLocation,
	const FVector& TargetVelocity,
	const float CurrentTargetDistanceCm)
{
	if (!bHasPreviousTargetDistance || MissileVelocity.IsNearlyZero())
	{
		return false;
	}

	// [v1.4.0] 현재 미사일에서 Sensor Truth Target으로 향하는 정규화 방향입니다.
	const FVector DirectionToTarget = (TargetLocation - MissileLocation).GetSafeNormal();
	if (DirectionToTarget.IsNearlyZero())
	{
		return false;
	}

	// [v1.4.0] PN과 같은 상대운동 정의로 계산한 Target 방향의 실제 접근 속도입니다.
	const FVector RelativeVelocity = TargetVelocity - MissileVelocity;
	const float ForwardClosingVelocityCmPerSec = -FVector::DotProduct(RelativeVelocity, DirectionToTarget);

	// [v1.4.0] 부동소수점 흔들림으로 approach/overshoot가 반복 전환되지 않게 할 거리 허용값입니다.
	constexpr float OvershootDistanceToleranceCm = 1.0f;

	// [v1.4.0] 이전 Step보다 실제 거리가 줄었고 상대 접근 속도도 양수인지 여부입니다.
	const bool bDistanceDecreasing = CurrentTargetDistanceCm
		< PreviousTargetDistanceCm - OvershootDistanceToleranceCm;
	if (!bOvershootArmed
		&& CurrentSeekerState == ECFMissileSeekerState::Tracking
		&& bDistanceDecreasing
		&& ForwardClosingVelocityCmPerSec > KINDA_SMALL_NUMBER)
	{
		bOvershootArmed = true;
	}

	// [v1.4.0] 실제 접근이 한 번 성립한 뒤 거리가 증가하면서 더 이상 접근하지 않을 때만 Stateful Overshoot로 종결합니다.
	const bool bDistanceIncreasing = CurrentTargetDistanceCm
		> PreviousTargetDistanceCm + OvershootDistanceToleranceCm;
	return bOvershootArmed
		&& bDistanceIncreasing
		&& ForwardClosingVelocityCmPerSec <= 0.0f;
}

// [v1.3.0] 현재 Guidance Law의 정보 소비 계약에 맞춰 AimPoint와 물리 제한형 Guidance Command를 생성합니다.
FCFMissileGuidanceCommand UCFMissileGuideComp::BuildGuidanceCommandForActiveLaw(
	const FVector& MissileLocation,
	const FVector& MissileVelocity,
	const FVector& TargetLocation,
	const FVector& TargetVelocity,
	const float DeltaTime)
{
	GuidanceAimPoint = TargetLocation;
	bCourseCaptureActive = false;

	// [v1.3.0] 선택된 Law와 무관하게 공통으로 전달할 현재 미사일 상태와 유효 Target 입력입니다.
	FCFMissileGuidanceInput GuidanceInput;
	GuidanceInput.MissileLocation = MissileLocation;
	GuidanceInput.MissileVelocity = MissileVelocity;
	GuidanceInput.TargetLocation = TargetLocation;
	GuidanceInput.TargetVelocityEstimate = TargetVelocity;
	GuidanceInput.DeltaSeconds = DeltaTime;
	GuidanceInput.bHasTarget = true;

	if (ActiveGuideConfig.GuidanceLaw == ECFMissileGuidanceLaw::PurePursuit)
	{
		// [v1.3.0] LostFinal Hold는 기존 고정 지점 계약을 우선하고 일반 추적에서는 마지막 실제 관측 위치만 사용하는 PurePursuit AimPoint입니다.
		GuidanceAimPoint = CurrentSeekerState == ECFMissileSeekerState::LostFinal && bHasHoldTargetLocation
			? HoldTargetLocation
			: (bHasValidObservation ? LastObservedTargetLocation : TargetLocation);
		GuidanceInput.TargetLocation = GuidanceAimPoint;
		GuidanceInput.TargetVelocityEstimate = FVector::ZeroVector;
		return BuildBoundedPursuitCommandWithRearTieBreak(GuidanceInput);
	}

	if (ActiveGuideConfig.GuidanceLaw == ECFMissileGuidanceLaw::LeadPursuit)
	{
		// [v1.3.0] LeadPursuit가 미래로 투영할 안전 보정 선행 시간입니다.
		const float LeadTimeSeconds = ActiveGuideConfig.GetEffectiveLeadTimeSeconds();

		// [v1.3.0] 현재 관측 계층이 제공한 Target 속도 추정으로 계산한 원본 선행 오프셋입니다.
		const FVector RawLeadOffset = TargetVelocity * LeadTimeSeconds;

		// [v1.3.0] ProjectileData가 허용한 최대 선행 거리 안으로 제한한 실제 선행 오프셋입니다.
		const FVector BoundedLeadOffset = RawLeadOffset.GetClampedToMaxSize(
			ActiveGuideConfig.GetEffectiveMaxLeadDistanceCm());

		// [v1.3.0] EstimatedTargetLocation을 다시 더하지 않고 마지막 실제 관측 위치에 bounded lead만 더한 LeadPursuit AimPoint입니다.
		const FVector LeadBaseLocation = CurrentSeekerState == ECFMissileSeekerState::LostFinal && bHasHoldTargetLocation
			? HoldTargetLocation
			: (bHasValidObservation ? LastObservedTargetLocation : TargetLocation);
		GuidanceAimPoint = LeadBaseLocation + BoundedLeadOffset;
		GuidanceInput.TargetLocation = GuidanceAimPoint;
		return BuildBoundedPursuitCommandWithRearTieBreak(GuidanceInput);
	}

	// [v1.3.0] PN이 현재 Sensor Truth 방향으로 실제 접근 기하를 이미 만들었는지 판단할 정규화 Target 방향입니다.
	const FVector DirectionToTarget = (TargetLocation - MissileLocation).GetSafeNormal();

	// [v1.3.0] PN과 같은 상대운동 기준으로 ClosingSpeed를 판정할 Target-Missile 상대 속도입니다.
	const FVector RelativeVelocity = TargetVelocity - MissileVelocity;

	// [v1.3.0] 현재 Sensor Truth Target과의 실제 상대 거리가 줄어드는 방향의 PN-compatible 접근 속도입니다.
	const float ForwardClosingVelocityCmPerSec = DirectionToTarget.IsNearlyZero()
		? 0.0f
		: FMath::Max(-FVector::DotProduct(RelativeVelocity, DirectionToTarget), 0.0f);

	if (ForwardClosingVelocityCmPerSec <= KINDA_SMALL_NUMBER)
	{
		bCourseCaptureActive = true;
		GuidanceAimPoint = TargetLocation;
		GuidanceInput.TargetLocation = GuidanceAimPoint;
		GuidanceInput.TargetVelocityEstimate = FVector::ZeroVector;
		return BuildBoundedPursuitCommandWithRearTieBreak(GuidanceInput);
	}

	GuidanceAimPoint = TargetLocation;
	return CFMissileGuideMath::CalculateBoundedProportionalNavigationCommand(GuidanceInput, ActiveGuideConfig);
}

// [v1.4.1] 모든 bounded Pursuit 계열에서 exact rear 횡방향 특이점을 Launch Right Vector로 동일하게 해소합니다.
FCFMissileGuidanceCommand UCFMissileGuideComp::BuildBoundedPursuitCommandWithRearTieBreak(
	const FCFMissileGuidanceInput& GuidanceInput) const
{
	// [v1.4.1] 원본 AimPoint에 대한 실제 Seeker 각도와 rear 정렬을 계산할 현재 진행 방향입니다.
	const FVector MissileForwardDirection = GuidanceInput.MissileVelocity.GetSafeNormal();

	// [v1.4.1] 원본 Guidance AimPoint까지의 실제 시선 방향입니다.
	const FVector DirectionToGuidanceAimPoint = (GuidanceInput.TargetLocation - GuidanceInput.MissileLocation).GetSafeNormal();
	if (MissileForwardDirection.IsNearlyZero() || DirectionToGuidanceAimPoint.IsNearlyZero())
	{
		return CFMissileGuideMath::CalculateBoundedPursuitCommand(GuidanceInput, ActiveGuideConfig);
	}

	// [v1.4.1] 정확한 후방 특이점 여부를 판단할 현재 진행 방향과 원본 AimPoint 시선 방향의 정렬값입니다.
	const float GuidanceAimPointForwardAlignment = FVector::DotProduct(
		MissileForwardDirection,
		DirectionToGuidanceAimPoint);

	// [v1.4.1] 원본 AimPoint 시선에서 현재 진행 방향 성분을 제거한 실제 횡방향 성분입니다.
	const FVector GuidanceAimPointLateralDirection = DirectionToGuidanceAimPoint
		- MissileForwardDirection * GuidanceAimPointForwardAlignment;

	// [v1.4.1] 수치상 정반대로 간주할 정렬 허용값입니다.
	constexpr float ExactRearAlignmentTolerance = 0.0001f;
	if (GuidanceAimPointForwardAlignment > -1.0f + ExactRearAlignmentTolerance
		|| !GuidanceAimPointLateralDirection.IsNearlyZero())
	{
		return CFMissileGuideMath::CalculateBoundedPursuitCommand(GuidanceInput, ActiveGuideConfig);
	}

	// [v1.4.1] 발사 순간 Right Vector를 현재 진행 방향의 횡평면에 투영한 결정론적 선회 방향입니다.
	const FVector ProjectedLaunchRightVector = LaunchRightVector
		- MissileForwardDirection * FVector::DotProduct(LaunchRightVector, MissileForwardDirection);

	// [v1.4.1] exact rear에서 bounded Pursuit가 사용할 안정적인 좌우 선회 방향입니다.
	const FVector RearTieBreakDirection = ProjectedLaunchRightVector.GetSafeNormal();
	if (RearTieBreakDirection.IsNearlyZero())
	{
		return CFMissileGuideMath::CalculateBoundedPursuitCommand(GuidanceInput, ActiveGuideConfig);
	}

	// [v1.4.1] 실제 AimPoint까지의 거리를 보존하면서 순수 Pursuit 계산에만 횡방향을 제공할 임시 입력입니다.
	FCFMissileGuidanceInput RearTieBreakGuidanceInput = GuidanceInput;

	// [v1.4.1] 지나치게 짧은 가상 AimPoint가 수치적으로 불안정하지 않게 할 최소 tie-break 거리입니다.
	const float RearTieBreakAimDistanceCm = FMath::Max(
		FVector::Dist(GuidanceInput.MissileLocation, GuidanceInput.TargetLocation),
		100.0f);

	// [v1.4.1] 180도 방향오차 크기는 사실상 유지하면서 좌우 방향만 결정하기 위한 아주 작은 횡방향 bias입니다.
	constexpr float RearTieBreakSteeringBias = 0.001f;

	// [v1.4.1] 원래 정반대 방향에 Launch Right를 미세하게 더해 bounded Pursuit가 안정적인 횡방향만 얻도록 만든 방향입니다.
	const FVector RearBiasedGuidanceDirection = (
		-MissileForwardDirection
		+ RearTieBreakDirection * RearTieBreakSteeringBias).GetSafeNormal();
	RearTieBreakGuidanceInput.TargetLocation = GuidanceInput.MissileLocation
		+ RearBiasedGuidanceDirection * RearTieBreakAimDistanceCm;

	// [v1.4.1] 기존 Pursuit 물리 제한을 그대로 거친 deterministic exact-rear 선회 명령입니다.
	FCFMissileGuidanceCommand RearTieBreakCommand = CFMissileGuideMath::CalculateBoundedPursuitCommand(
		RearTieBreakGuidanceInput,
		ActiveGuideConfig);

	// [v1.4.1] 가상 횡방향 AimPoint가 아니라 실제 원본 rear AimPoint 기준 Seeker 각도를 진단에 보존합니다.
	RearTieBreakCommand.SeekerAngleDeg = CFMissileGuideMath::CalculateDirectionAngleDeg(
		MissileForwardDirection,
		DirectionToGuidanceAimPoint);
	return RearTieBreakCommand;
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
	GuidanceAimPoint = FVector::ZeroVector;
	bCourseCaptureActive = false;
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
	CurrentGuideSnapshot.MissReason = ActiveGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful
		&& StatefulFinalMissReason != ECFMissileMissReason::None
		? StatefulFinalMissReason
		: CurrentGuidanceCommand.InvalidReason;
	CurrentGuideSnapshot.SeekerModel = ActiveGuideConfig.SeekerModel;
	CurrentGuideSnapshot.SeekerState = CurrentSeekerState;
	CurrentGuideSnapshot.TargetObservationMode = ActiveGuideConfig.TargetObservationMode;
	CurrentGuideSnapshot.GuidanceLaw = ActiveGuideConfig.GuidanceLaw;
	CurrentGuideSnapshot.GuidanceActivationMode = ActiveGuideConfig.GuidanceActivationMode;
	CurrentGuideSnapshot.bGuidanceActivationSatisfied = bGuidanceActivationSatisfied;
	CurrentGuideSnapshot.GuidanceAimPoint = GuidanceAimPoint;
	CurrentGuideSnapshot.bCourseCaptureActive = bCourseCaptureActive;
	CurrentGuideSnapshot.bOvershootArmed = bOvershootArmed;
	CurrentGuideSnapshot.LastObservedTargetLocation = LastObservedTargetLocation;
	CurrentGuideSnapshot.EstimatedTargetLocation = EstimatedTargetLocation;
	CurrentGuideSnapshot.FilteredTargetVelocityEstimate = FilteredTargetVelocityEstimate;
	CurrentGuideSnapshot.ObservationAgeSeconds = ObservationAgeSeconds;
	CurrentGuideSnapshot.ReacquisitionElapsedTimeSeconds = ReacquisitionElapsedTimeSeconds;
	CurrentGuideSnapshot.bHasValidObservation = bHasValidObservation;
}
