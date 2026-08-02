// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CarFight 미사일 비행 상태 컴포넌트 구현
// Scope: Direct Release의 분리·Clearance·GuidedFlight 전환과 Pool Reset을 구현합니다.
// Changelog:
// - v1.0.0: MG-P0-01 Direct 비행 상태 Runtime 최초 구현.
// Migration:
// - 기존 ProjectileData는 bUseMissileFlight=false 기본값으로 컴포넌트 Tick과 상태 전환을 사용하지 않습니다.

#include "CFMissileFlightComp.h"

#include "GameFramework/ProjectileMovementComponent.h"

// [v1.0.0] 미사일 비행 Tick 기본값과 Inactive 상태를 초기화합니다.
UCFMissileFlightComp::UCFMissileFlightComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetComponentTickEnabled(false);
	RefreshFlightSnapshot();
}

// [v1.0.0] 비행 설정과 발사 순간 Context를 복사해 이번 미사일 비행 상태를 시작합니다.
void UCFMissileFlightComp::StartMissileFlight(
	const FCFMissileFlightConfig& InFlightConfig,
	const FCFProjectileLaunchContext& InLaunchContext,
	UProjectileMovementComponent* InProjectileMovementComponent)
{
	ResetMissileFlight();

	ActiveFlightConfig = InFlightConfig.GetEffectiveConfig();
	ActiveProjectileMovementComponent = InProjectileMovementComponent;
	ReleaseLocation = InLaunchContext.LaunchTransform.GetLocation();
	if (ReleaseLocation.ContainsNaN())
	{
		ReleaseLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	++FlightActivationCount;
	if (!ActiveFlightConfig.bUseMissileFlight || !IsValid(ActiveProjectileMovementComponent.Get()))
	{
		RefreshFlightSnapshot();
		return;
	}

	SetFlightState(ECFMissileFlightState::Released);
	SetComponentTickEnabled(true);
	RefreshFlightSnapshot();
}

// [v1.0.0] 비행 상태와 Runtime 참조를 Pool 재사용 안전 기본값으로 초기화합니다.
void UCFMissileFlightComp::ResetMissileFlight()
{
	SetComponentTickEnabled(false);
	ActiveFlightConfig = FCFMissileFlightConfig();
	ActiveProjectileMovementComponent = nullptr;
	CurrentFlightState = ECFMissileFlightState::Inactive;
	ReleaseLocation = FVector::ZeroVector;
	ElapsedFlightTimeSeconds = 0.0f;
	ElapsedTransitionTimeSeconds = 0.0f;
	DistanceFromReleaseCm = 0.0f;
	RefreshFlightSnapshot();
}

// [v1.0.0] 현재 Flight State가 Guidance Command를 적용할 수 있는 구간인지 반환합니다.
bool UCFMissileFlightComp::IsGuidanceWindowOpen() const
{
	return CurrentFlightState == ECFMissileFlightState::GuidedFlight
		|| CurrentFlightState == ECFMissileFlightState::Terminal;
}

// [v1.0.0] 현재 비행 상태를 한 줄 Debug 문자열로 생성합니다.
FString UCFMissileFlightComp::BuildFlightSummary() const
{
	// [v1.0.0] 현재 비행 상태의 표시 이름을 찾을 Enum 정보입니다.
	const UEnum* FlightStateEnum = StaticEnum<ECFMissileFlightState>();

	// [v1.0.0] 현재 공격 프로파일의 표시 이름을 찾을 Enum 정보입니다.
	const UEnum* AttackProfileEnum = StaticEnum<ECFMissileAttackProfile>();

	// [v1.0.0] 현재 비행 상태를 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString FlightStateText = FlightStateEnum
		? FlightStateEnum->GetNameStringByValue(static_cast<int64>(CurrentFlightState))
		: TEXT("Unknown");

	// [v1.0.0] 현재 공격 프로파일을 사람이 읽을 수 있게 변환한 문자열입니다.
	const FString AttackProfileText = AttackProfileEnum
		? AttackProfileEnum->GetNameStringByValue(static_cast<int64>(ActiveFlightConfig.AttackProfile))
		: TEXT("Unknown");

	return FString::Printf(
		TEXT("MissileFlight: State=%s, Profile=%s, Flight=%.3fs, Transition=%.3fs, Distance=%.1fcm, Clearance=%s, GuidanceWindow=%s, Activations=%d"),
		*FlightStateText,
		*AttackProfileText,
		ElapsedFlightTimeSeconds,
		ElapsedTransitionTimeSeconds,
		DistanceFromReleaseCm,
		CurrentFlightSnapshot.bClearanceSatisfied ? TEXT("Yes") : TEXT("No"),
		IsGuidanceWindowOpen() ? TEXT("Open") : TEXT("Closed"),
		FlightActivationCount);
}

// [v1.0.0] Automation에서 월드 Tick 없이 비행 상태를 한 단계 진행합니다.
void UCFMissileFlightComp::AdvanceFlightForAutomation(const float DeltaTime)
{
	AdvanceFlightSimulation(DeltaTime);
}

// [v1.0.0] 분리 시간·거리와 Flight State 전환을 갱신합니다.
void UCFMissileFlightComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AdvanceFlightSimulation(DeltaTime);
}

// [v1.0.0] 한 시간 구간의 비행 거리와 상태 전환을 처리합니다.
void UCFMissileFlightComp::AdvanceFlightSimulation(const float DeltaTime)
{
	if (!ActiveFlightConfig.bUseMissileFlight
		|| CurrentFlightState == ECFMissileFlightState::Inactive
		|| !IsValid(ActiveProjectileMovementComponent.Get()))
	{
		SetComponentTickEnabled(false);
		RefreshFlightSnapshot();
		return;
	}

	// [v1.0.0] 음수와 비정상 값을 제거한 이번 시뮬레이션 시간입니다.
	const float SafeDeltaTime = FMath::IsFinite(DeltaTime)
		? FMath::Max(DeltaTime, 0.0f)
		: 0.0f;

	ElapsedFlightTimeSeconds += SafeDeltaTime;

	// [v1.0.0] 분리 거리 계산에 사용할 현재 미사일 월드 위치입니다.
	FVector CurrentMissileLocation = GetOwner() ? GetOwner()->GetActorLocation() : ReleaseLocation;
	if (CurrentMissileLocation.ContainsNaN())
	{
		CurrentMissileLocation = ReleaseLocation;
	}

	DistanceFromReleaseCm = FVector::Dist(CurrentMissileLocation, ReleaseLocation);
	AdvanceFlightStateMachine(SafeDeltaTime);
	RefreshFlightSnapshot();
}

// [v1.0.0] 즉시 전환 가능한 상태를 순서대로 진행하고 안정 상태에서 중단합니다.
void UCFMissileFlightComp::AdvanceFlightStateMachine(const float DeltaTime)
{
	// [v1.0.0] Released와 Ejection의 즉시 상태 전환이 무한 반복되지 않게 제한할 최대 횟수입니다.
	constexpr int32 MaximumImmediateTransitions = 4;

	for (int32 TransitionIndex = 0; TransitionIndex < MaximumImmediateTransitions; ++TransitionIndex)
	{
		switch (CurrentFlightState)
		{
		case ECFMissileFlightState::Released:
			SetFlightState(ECFMissileFlightState::Ejection);
			continue;

		case ECFMissileFlightState::Ejection:
			SetFlightState(ECFMissileFlightState::Clearance);
			continue;

		case ECFMissileFlightState::Clearance:
			if (!ActiveFlightConfig.IsClearanceSatisfied(ElapsedFlightTimeSeconds, DistanceFromReleaseCm))
			{
				return;
			}

			if (ActiveFlightConfig.AttackProfile == ECFMissileAttackProfile::Direct)
			{
				SetFlightState(ECFMissileFlightState::GuidedFlight);
				return;
			}

			if (ActiveFlightConfig.GetEffectiveTransitionDurationSeconds() <= KINDA_SMALL_NUMBER)
			{
				SetFlightState(ECFMissileFlightState::GuidedFlight);
				return;
			}

			ElapsedTransitionTimeSeconds = 0.0f;
			SetFlightState(ECFMissileFlightState::Transition);
			return;

		case ECFMissileFlightState::Transition:
			ElapsedTransitionTimeSeconds += DeltaTime;
			if (ElapsedTransitionTimeSeconds + KINDA_SMALL_NUMBER
				>= ActiveFlightConfig.GetEffectiveTransitionDurationSeconds())
			{
				SetFlightState(ECFMissileFlightState::GuidedFlight);
			}
			return;

		default:
			return;
		}
	}
}

// [v1.0.0] 현재 Flight State를 변경합니다.
void UCFMissileFlightComp::SetFlightState(const ECFMissileFlightState NewFlightState)
{
	CurrentFlightState = NewFlightState;
}

// [v1.0.0] 현재 내부 값을 Blueprint 읽기용 비행 스냅샷에 반영합니다.
void UCFMissileFlightComp::RefreshFlightSnapshot()
{
	CurrentFlightSnapshot.CurrentFlightState = CurrentFlightState;
	CurrentFlightSnapshot.AttackProfile = ActiveFlightConfig.AttackProfile;
	CurrentFlightSnapshot.ElapsedFlightTimeSeconds = ElapsedFlightTimeSeconds;
	CurrentFlightSnapshot.DistanceFromReleaseCm = DistanceFromReleaseCm;
	CurrentFlightSnapshot.bClearanceSatisfied = ActiveFlightConfig.IsClearanceSatisfied(
		ElapsedFlightTimeSeconds,
		DistanceFromReleaseCm);
	CurrentFlightSnapshot.bGuidanceWindowOpen = IsGuidanceWindowOpen();
	CurrentFlightSnapshot.bTerminalPhaseActive = CurrentFlightState == ECFMissileFlightState::Terminal;
}
