// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.2
// Date: 2026-09-18
// Description: Phase 4 Vehicle Target Lock Runtime 구현 / Phase 7 Guided Weapon actor bridge fail-closed 보강
// Changelog:
// - v1.0.2: GetLockedTargetActor가 Runtime Ready, Locked, valid public snapshot, valid non-destroying Actor를 모두 확인한 뒤에만 Phase 7 내부 Gameplay Consumer에 Actor bridge를 반환.
// - v1.0.1: 같은 Contact의 AlreadyLocked/AlreadyAcquiring 판정보다 fresh Sensor Snapshot의 Live Contact 검증을 먼저 수행해 RequestLock의 Live-only 계약을 stale window에서도 유지.
// - v1.0.0: Live Contact RequestLock, 단일 교체, Acquire 진행/감쇠, Locked Quality 회복/감쇠, Lost/Destroyed Break와 Snapshot을 최초 구현.
// Migration:
// - Selection 변경은 이 Component 상태를 자동 변경하지 않습니다.
// - Lost 뒤 새 ContactId는 과거 Lock에 자동 재연결하지 않습니다.

#include "CFVehicleTargetingComp.h"

#include "CFVehicleSensorComp.h"
#include "CFSensorTypes.h"
#include "GameFramework/Actor.h"

// Targeting Runtime 기본 Tick 정책과 P0 fallback 설정을 준비합니다.
UCFVehicleTargetingComp::UCFVehicleTargetingComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Component 시작 시 독립 Targeting Runtime을 초기화합니다.
void UCFVehicleTargetingComp::BeginPlay()
{
	Super::BeginPlay();
	InitializeTargetingRuntime();
}

// Component 수명 종료 시 Actor bridge와 Targeting 상태를 정리합니다.
void UCFVehicleTargetingComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetTargetingRuntime();
	Super::EndPlay(EndPlayReason);
}

// Acquiring/Locked 동안 현재 Sensor Snapshot을 소비해 Lock 상태를 진행합니다.
void UCFVehicleTargetingComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bTargetingRuntimeReady || CurrentTargetingSnapshot.State == ECFTargetLockState::Idle)
	{
		SetComponentTickEnabled(false);
		return;
	}

	// 현재 Lock 상태의 Contact Authority를 제공할 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = ResolveSensorComponent();
	if (!SensorComponent || !SensorComponent->IsSensorRuntimeReady())
	{
		if (CurrentTargetingSnapshot.State == ECFTargetLockState::Locked)
		{
			BreakCurrentLock(ECFTargetLockBreakReason::SensorUnavailable);
		}
		else
		{
			ClearActiveTargetState();
		}
		return;
	}

	// 이번 Tick에서 Lock 상태 전이를 계산할 actor-free Sensor Snapshot입니다.
	const FCFSensorSnapshot SensorSnapshot = SensorComponent->GetSensorSnapshot();
	AdvanceTargetingState(FMath::Max(DeltaTime, 0.0f), SensorSnapshot);
}

// 현재 fallback Config를 검증하고 새로운 Targeting Runtime lifetime을 시작합니다.
bool UCFVehicleTargetingComp::InitializeTargetingRuntime()
{
	CurrentTargetActor.Reset();
	CurrentTargetingSnapshot = FCFTargetingSnapshot();
	LastLockRequestResult = ECFTargetLockRequestResult::None;
	SetComponentTickEnabled(false);

	bTargetingRuntimeReady = FallbackTargetingConfig.IsValid();
	return bTargetingRuntimeReady;
}

// 현재 Lock/Acquire와 Break 이력을 모두 비우고 Runtime을 미준비 상태로 되돌립니다.
void UCFVehicleTargetingComp::ResetTargetingRuntime()
{
	bTargetingRuntimeReady = false;
	CurrentTargetActor.Reset();
	CurrentTargetingSnapshot = FCFTargetingSnapshot();
	LastLockRequestResult = ECFTargetLockRequestResult::None;
	SetComponentTickEnabled(false);
}

// Live Sensor Contact로 관측 중인 Actor를 단일 Vehicle Target Lock 대상으로 요청합니다.
ECFTargetLockRequestResult UCFVehicleTargetingComp::RequestLock(AActor* TargetActor)
{
	if (!bTargetingRuntimeReady)
	{
		LastLockRequestResult = ECFTargetLockRequestResult::RuntimeNotReady;
		return LastLockRequestResult;
	}

	if (!IsValid(TargetActor))
	{
		LastLockRequestResult = ECFTargetLockRequestResult::InvalidTarget;
		return LastLockRequestResult;
	}

	// Lock 요청 대상의 Contact Authority를 조회할 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = ResolveSensorComponent();
	if (!SensorComponent || !SensorComponent->IsSensorRuntimeReady())
	{
		LastLockRequestResult = ECFTargetLockRequestResult::SensorUnavailable;
		return LastLockRequestResult;
	}

	// Actor를 현재 Sensor Contact lifetime의 exact ContactId로 연결할 출력값입니다.
	FName TargetContactId = NAME_None;
	if (!SensorComponent->TryGetContactIdForActor(TargetActor, TargetContactId) || TargetContactId.IsNone())
	{
		LastLockRequestResult = ECFTargetLockRequestResult::NoSensorContact;
		return LastLockRequestResult;
	}

	// 요청 시점 Contact가 반드시 Live인지 확인할 actor-free Sensor Snapshot입니다.
	const FCFSensorSnapshot SensorSnapshot = SensorComponent->GetSensorSnapshot();

	// 요청한 ContactId에 대응하는 현재 Contact입니다.
	const FCFSensorContact* SensorContact = FindContactById(SensorSnapshot, TargetContactId);
	if (!SensorContact)
	{
		LastLockRequestResult = ECFTargetLockRequestResult::NoSensorContact;
		return LastLockRequestResult;
	}

	if (SensorContact->ContactState != ECFSensorContactState::Live)
	{
		LastLockRequestResult = ECFTargetLockRequestResult::ContactNotLive;
		return LastLockRequestResult;
	}

	// fresh Live Contact 검증 뒤에만 같은 ContactId의 중복 Lock 요청 결과를 판정합니다.
	if (CurrentTargetingSnapshot.TargetContactId == TargetContactId)
	{
		if (CurrentTargetingSnapshot.State == ECFTargetLockState::Locked)
		{
			LastLockRequestResult = ECFTargetLockRequestResult::AlreadyLocked;
			return LastLockRequestResult;
		}

		if (CurrentTargetingSnapshot.State == ECFTargetLockState::Acquiring)
		{
			LastLockRequestResult = ECFTargetLockRequestResult::AlreadyAcquiring;
			return LastLockRequestResult;
		}
	}

	ClearActiveTargetState();
	CurrentTargetActor = TargetActor;
	CurrentTargetingSnapshot.State = ECFTargetLockState::Acquiring;
	CurrentTargetingSnapshot.TargetContactId = TargetContactId;
	CurrentTargetingSnapshot.LockProgress01 = 0.0f;
	CurrentTargetingSnapshot.LockQuality01 = 0.0f;
	LastLockRequestResult = ECFTargetLockRequestResult::Accepted;
	SetComponentTickEnabled(true);
	return LastLockRequestResult;
}

// 현재 Acquiring 또는 Locked 상태를 명시적으로 해제하며 Break 이벤트는 만들지 않습니다.
bool UCFVehicleTargetingComp::ClearLock()
{
	if (CurrentTargetingSnapshot.State == ECFTargetLockState::Idle)
	{
		return false;
	}

	ClearActiveTargetState();
	return true;
}

// Phase 7 Guided Weapon 같은 내부 Gameplay Consumer가 fail-closed Locked 대상 Actor bridge를 읽습니다. HUD truth로 사용하지 않습니다.
AActor* UCFVehicleTargetingComp::GetLockedTargetActor() const
{
	if (!bTargetingRuntimeReady
		|| CurrentTargetingSnapshot.State != ECFTargetLockState::Locked
		|| !CurrentTargetingSnapshot.IsPublicContractValid())
	{
		return nullptr;
	}

	// 현재 Locked Snapshot과 함께 보존 중인 내부 Gameplay Actor bridge입니다.
	AActor* LockedTargetActor = CurrentTargetActor.Get();
	return IsValid(LockedTargetActor) && !LockedTargetActor->IsActorBeingDestroyed()
		? LockedTargetActor
		: nullptr;
}

// 현재 소유 Actor의 Sensor Component를 찾아 반환합니다.
UCFVehicleSensorComp* UCFVehicleTargetingComp::ResolveSensorComponent() const
{
	// Targeting Component를 소유하는 Actor입니다.
	AActor* OwnerActor = GetOwner();
	return IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UCFVehicleSensorComp>()
		: nullptr;
}

// Sensor Snapshot에서 exact ContactId와 일치하는 Contact를 찾아 반환합니다.
const FCFSensorContact* UCFVehicleTargetingComp::FindContactById(
	const FCFSensorSnapshot& SensorSnapshot,
	const FName ContactId) const
{
	if (ContactId.IsNone())
	{
		return nullptr;
	}

	return SensorSnapshot.Contacts.FindByPredicate(
		[ContactId](const FCFSensorContact& Contact)
		{
			return Contact.ContactId == ContactId;
		});
}

// 현재 Sensor Snapshot 한 번을 소비해 Acquiring/Locked 상태를 진행합니다.
void UCFVehicleTargetingComp::AdvanceTargetingState(
	const float DeltaSeconds,
	const FCFSensorSnapshot& SensorSnapshot)
{
	// 현재 Acquire/Lock 대상 ContactId와 정확히 일치하는 Sensor Contact입니다.
	const FCFSensorContact* SensorContact = FindContactById(
		SensorSnapshot,
		CurrentTargetingSnapshot.TargetContactId);
	if (!SensorContact)
	{
		if (CurrentTargetingSnapshot.State == ECFTargetLockState::Locked)
		{
			BreakCurrentLock(ECFTargetLockBreakReason::ContactLost);
		}
		else
		{
			ClearActiveTargetState();
		}
		return;
	}

	if (CurrentTargetingSnapshot.State == ECFTargetLockState::Acquiring)
	{
		UpdateAcquiringState(DeltaSeconds, *SensorContact);
		return;
	}

	if (CurrentTargetingSnapshot.State == ECFTargetLockState::Locked)
	{
		UpdateLockedState(DeltaSeconds, *SensorContact);
	}
}

// Acquiring 상태의 Contact 상태와 설정을 적용합니다.
void UCFVehicleTargetingComp::UpdateAcquiringState(
	const float DeltaSeconds,
	const FCFSensorContact& SensorContact)
{
	switch (SensorContact.ContactState)
	{
	case ECFSensorContactState::Live:
		CurrentTargetingSnapshot.LockProgress01 = FMath::Clamp(
			CurrentTargetingSnapshot.LockProgress01
				+ FallbackTargetingConfig.LockAcquireGainPerSec * DeltaSeconds,
			0.0f,
			1.0f);
		if (CurrentTargetingSnapshot.LockProgress01 >= 1.0f - KINDA_SMALL_NUMBER)
		{
			CurrentTargetingSnapshot.State = ECFTargetLockState::Locked;
			CurrentTargetingSnapshot.LockProgress01 = 1.0f;
			CurrentTargetingSnapshot.LockQuality01 = 1.0f;
		}
		break;

	case ECFSensorContactState::LastKnown:
		CurrentTargetingSnapshot.LockProgress01 = FMath::Clamp(
			CurrentTargetingSnapshot.LockProgress01
				- FallbackTargetingConfig.LockAcquireDecayPerSec * DeltaSeconds,
			0.0f,
			1.0f);
		break;

	case ECFSensorContactState::Lost:
	case ECFSensorContactState::DestroyedHold:
	case ECFSensorContactState::Invalid:
	default:
		ClearActiveTargetState();
		break;
	}
}

// Locked 상태의 Contact 상태와 설정을 적용합니다.
void UCFVehicleTargetingComp::UpdateLockedState(
	const float DeltaSeconds,
	const FCFSensorContact& SensorContact)
{
	switch (SensorContact.ContactState)
	{
	case ECFSensorContactState::Live:
		CurrentTargetingSnapshot.LockQuality01 = FMath::Clamp(
			CurrentTargetingSnapshot.LockQuality01
				+ FallbackTargetingConfig.LockQualityRecoveryPerSec * DeltaSeconds,
			0.0f,
			1.0f);
		break;

	case ECFSensorContactState::LastKnown:
		CurrentTargetingSnapshot.LockQuality01 = FMath::Clamp(
			CurrentTargetingSnapshot.LockQuality01
				- FallbackTargetingConfig.LockQualityDecayPerSec * DeltaSeconds,
			0.0f,
			1.0f);
		if (CurrentTargetingSnapshot.LockQuality01 <= KINDA_SMALL_NUMBER)
		{
			BreakCurrentLock(ECFTargetLockBreakReason::QualityDepleted);
		}
		break;

	case ECFSensorContactState::DestroyedHold:
		BreakCurrentLock(ECFTargetLockBreakReason::TargetDestroyed);
		break;

	case ECFSensorContactState::Lost:
	case ECFSensorContactState::Invalid:
	default:
		BreakCurrentLock(ECFTargetLockBreakReason::ContactLost);
		break;
	}
}

// Break 전이 증거는 보존하면서 현재 대상/진행/품질만 Idle로 비웁니다.
void UCFVehicleTargetingComp::ClearActiveTargetState()
{
	CurrentTargetActor.Reset();
	CurrentTargetingSnapshot.State = ECFTargetLockState::Idle;
	CurrentTargetingSnapshot.TargetContactId = NAME_None;
	CurrentTargetingSnapshot.LockProgress01 = 0.0f;
	CurrentTargetingSnapshot.LockQuality01 = 0.0f;
	SetComponentTickEnabled(false);
}

// 실제 Locked Break를 exact-once 기록하고 즉시 Idle로 전환합니다.
void UCFVehicleTargetingComp::BreakCurrentLock(const ECFTargetLockBreakReason BreakReason)
{
	// 이번 Break에 발급할 양수 단조 증가 Revision입니다.
	const int32 NextBreakRevision = FMath::Max(CurrentTargetingSnapshot.BreakTransitionRevision + 1, 1);
	CurrentTargetingSnapshot.BreakTransitionRevision = NextBreakRevision;
	CurrentTargetingSnapshot.LastBreakReason = BreakReason;
	ClearActiveTargetState();
}
