// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-02
// Description: CarFight 모듈형 런처 발사 시퀀스 컴포넌트 구현
// Scope: 첫 승인 발사 이후 Ripple·Salvo Dispatch, Volley 명령 목표 Snapshot, 런타임 취소, Volley 단위 쿨다운과 Debug 집계를 구현합니다.
// Changelog:
// - v1.1.0: 첫 발사 순간 CommandTargetLocation과 GuidanceTargetActor를 원자적으로 보존하고 후속 Projectile에 같은 Snapshot을 전달.
// - v1.0.0: LM-P0-03B Ripple·Salvo Runtime Scheduler 최초 구현.
// Migration:
// - 목표 Actor는 약한 참조로 보존하며 파괴된 목표의 수명을 연장하지 않습니다.
// - 첫 Projectile의 검증·실행·FX·Muzzle 진행은 Pawn 기존 단발 경로가 수행하고 이 컴포넌트는 남은 Projectile만 Dispatch합니다.
// - SequenceCompleted 쿨다운은 완료 또는 부분 발사 후 취소 시 한 번 기록하며 이미 발사된 Projectile을 되돌리지 않습니다.

#include "CFLauncherComp.h"

#include "CFTurretMountData.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

// [v1.0.0] Tick 기반 발사 시퀀스 갱신을 사용할 기본 컴포넌트 값을 초기화합니다.
UCFLauncherComp::UCFLauncherComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

// [v1.0.0] EndPlay에서 예약 발사와 런타임 참조를 정리합니다.
void UCFLauncherComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetLauncherRuntime();
	OwnerVehiclePawn = nullptr;
	VehicleWeaponComp = nullptr;
	Super::EndPlay(EndPlayReason);
}

// [v1.0.0] Ripple 시간 진행과 Salvo 후속 처리 묶음을 갱신합니다.
void UCFLauncherComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!SequenceRuntime.IsActive())
	{
		return;
	}

	// [v1.0.0] Tick 진행 전에 Owner·Health·장비 스냅샷이 유지되는지 검사한 결과입니다.
	const ECFLauncherSequenceCancelReason InvalidRuntimeReason = ValidateActiveSequenceRuntime();
	if (InvalidRuntimeReason != ECFLauncherSequenceCancelReason::None)
	{
		CancelFireSequence(InvalidRuntimeReason);
		return;
	}

	DispatchDueShots(DeltaTime);
}

// [v1.0.0] 런처 시퀀스가 사용할 Owner Pawn과 상위 WeaponComp를 연결합니다.
bool UCFLauncherComp::InitializeLauncherRuntime(
	ACFVehiclePawn* InOwnerVehiclePawn,
	UCFVehicleWeaponComp* InVehicleWeaponComp)
{
	ResetLauncherRuntime();
	OwnerVehiclePawn = InOwnerVehiclePawn;
	VehicleWeaponComp = InVehicleWeaponComp;

	// [v1.0.0] Owner Pawn과 WeaponComp가 모두 유효한 런처 런타임 준비 상태입니다.
	const bool bRuntimeReady = IsValid(OwnerVehiclePawn) && IsValid(VehicleWeaponComp);
	LastLauncherSequenceSummary = bRuntimeReady
		? TEXT("LauncherSequence: RuntimeReady, State=Idle")
		: TEXT("LauncherSequence: RuntimeMissing, State=Idle");
	return bRuntimeReady;
}

// [v1.1.0] 첫 발이 이미 승인·실행된 발사 패턴의 위치·Actor 목표 Snapshot과 남은 Ripple·Salvo 시퀀스를 시작합니다.
bool UCFLauncherComp::StartFireSequenceAfterFirstAcceptedShot(
	const FCFLauncherFirePatternConfig& InFirePatternConfig,
	const FVector& InCommandTargetLocation,
	AActor* InGuidanceTargetActor,
	const float InFirstAcceptedFireTimeSeconds)
{
	if (SequenceRuntime.IsActive()
		|| !IsValid(OwnerVehiclePawn)
		|| !IsValid(VehicleWeaponComp)
		|| InCommandTargetLocation.ContainsNaN())
	{
		return false;
	}

	// [v1.0.0] 시퀀스 도중 장비 교체를 감지하기 위해 시작 순간 복사한 WeaponData입니다.
	SequenceWeaponData = VehicleWeaponComp->GetActiveWeaponData();

	// [v1.0.0] 시퀀스 도중 마운트 교체를 감지하기 위해 시작 순간 복사한 TurretMountData입니다.
	SequenceTurretMountData = VehicleWeaponComp->GetActiveTurretMountData();

	SequenceCommandTargetSnapshot.Capture(InCommandTargetLocation, InGuidanceTargetActor);
	FirstAcceptedFireTimeSeconds = FMath::Max(InFirstAcceptedFireTimeSeconds, 0.0f);
	SequenceRuntime.Start(InFirePatternConfig, NextVolleyId++);

	// [v1.0.0] 첫 승인 발사에서 쿨다운을 시작하는 정책은 Pawn이 이미 기록했음을 표시합니다.
	bTerminalCooldownApplied = SequenceRuntime.ActiveConfig.CooldownStartPolicy
		== ECFLauncherCooldownStartPolicy::FirstAcceptedProjectile;

	RefreshLauncherSequenceSummary();

	if (SequenceRuntime.IsActive())
	{
		// [v1.0.0] Salvo의 첫 발과 같은 처리 묶음에 들어갈 추가 발사를 즉시 Dispatch합니다.
		DispatchDueShots(0.0f);
	}
	else
	{
		FinalizeTerminalSequence(FirstAcceptedFireTimeSeconds);
	}

	return true;
}

// [v1.0.0] 진행 중인 발사 시퀀스를 지정한 사유로 취소합니다.
void UCFLauncherComp::CancelFireSequence(const ECFLauncherSequenceCancelReason CancelReason)
{
	if (!SequenceRuntime.IsActive())
	{
		return;
	}

	SequenceRuntime.Cancel(CancelReason);

	// [v1.0.0] 취소 시점에 SequenceCompleted 정책 쿨다운을 기록할 현재 월드 시간입니다.
	const float TerminalTimeSeconds = GetWorld()
		? GetWorld()->GetTimeSeconds()
		: FMath::Max(FirstAcceptedFireTimeSeconds, 0.0f);
	FinalizeTerminalSequence(TerminalTimeSeconds);
}

// [v1.1.0] 시퀀스 상태와 위치·Actor 목표 Snapshot을 신규 입력 대기 상태로 초기화합니다.
void UCFLauncherComp::ResetLauncherRuntime()
{
	SequenceRuntime.Reset();
	SequenceWeaponData = nullptr;
	SequenceTurretMountData = nullptr;
	SequenceCommandTargetSnapshot.Reset();
	FirstAcceptedFireTimeSeconds = -1.0f;
	bTerminalCooldownApplied = false;
	LastLauncherSequenceSummary = TEXT("LauncherSequence: Reset, State=Idle");
}

// [v1.0.0] Owner·Health·Weapon·Turret 스냅샷이 현재 시퀀스를 계속 실행할 수 있는지 검사합니다.
ECFLauncherSequenceCancelReason UCFLauncherComp::ValidateActiveSequenceRuntime() const
{
	if (!IsValid(OwnerVehiclePawn)
		|| OwnerVehiclePawn->IsActorBeingDestroyed()
		|| !IsValid(VehicleWeaponComp))
	{
		return ECFLauncherSequenceCancelReason::OwnerInvalid;
	}

	// [v1.0.0] 파괴 상태에서 남은 Ripple·Salvo 발사를 취소하기 위한 차량 체력 컴포넌트입니다.
	const UCFVehicleHealthComp* VehicleHealthComp = OwnerVehiclePawn->GetVehicleHealthComp();
	if (VehicleHealthComp && VehicleHealthComp->IsDestroyed())
	{
		return ECFLauncherSequenceCancelReason::OwnerDestroyed;
	}

	if (VehicleWeaponComp->GetActiveWeaponData() != SequenceWeaponData)
	{
		return ECFLauncherSequenceCancelReason::WeaponChanged;
	}

	if (VehicleWeaponComp->GetActiveTurretMountData() != SequenceTurretMountData)
	{
		return ECFLauncherSequenceCancelReason::TurretMountChanged;
	}

	return ECFLauncherSequenceCancelReason::None;
}

// [v1.0.0] 이번 Tick에서 허용된 수량만큼 Pawn의 기존 단발 실행 경로를 호출합니다.
void UCFLauncherComp::DispatchDueShots(const float DeltaSeconds)
{
	if (!SequenceRuntime.IsActive())
	{
		return;
	}

	// [v1.0.0] 이번 호출에서 실행할 Ripple 한 발 또는 Salvo 처리 묶음 수량입니다.
	const int32 DispatchBudget = SequenceRuntime.GetDispatchBudget(DeltaSeconds);
	for (int32 DispatchIndex = 0; DispatchIndex < DispatchBudget; ++DispatchIndex)
	{
		if (!SequenceRuntime.IsActive())
		{
			break;
		}

		if (!IsValid(OwnerVehiclePawn) || !SequenceRuntime.MarkShotDispatched())
		{
			SequenceRuntime.Cancel(ECFLauncherSequenceCancelReason::RuntimeUnavailable);
			break;
		}

		// [v1.0.0] 첫 발을 0으로 하는 현재 Volley 내부 발사 순번입니다.
		const int32 SequenceShotIndex = SequenceRuntime.AttemptedProjectileCount - 1;

		// [v1.1.0] Pawn이 첫 발사 순간 위치·Actor Snapshot과 현재 Muzzle을 사용해 기존 Projectile·FX·Damage 경로를 재사용한 결과입니다.
		const bool bShotAccepted = OwnerVehiclePawn->ExecuteScheduledLauncherShot(
			SequenceRuntime.VolleyId,
			SequenceShotIndex,
			SequenceCommandTargetSnapshot.CommandTargetLocation,
			SequenceCommandTargetSnapshot.GetGuidanceTargetActor());

		SequenceRuntime.RecordShotResult(bShotAccepted);
	}

	if (!SequenceRuntime.IsActive())
	{
		// [v1.0.0] 완료 또는 실패 정책 취소 시점에 사용할 현재 월드 시간입니다.
		const float TerminalTimeSeconds = GetWorld()
			? GetWorld()->GetTimeSeconds()
			: FMath::Max(FirstAcceptedFireTimeSeconds, 0.0f);
		FinalizeTerminalSequence(TerminalTimeSeconds);
	}
	else
	{
		RefreshLauncherSequenceSummary();
	}
}

// [v1.0.0] Completed 또는 Cancelled 상태에서 SequenceCompleted 쿨다운과 최종 요약을 한 번 적용합니다.
void UCFLauncherComp::FinalizeTerminalSequence(const float TerminalTimeSeconds)
{
	// [v1.0.0] 현재 시퀀스가 완료 또는 취소된 terminal 상태인지 여부입니다.
	const bool bTerminalState = SequenceRuntime.State == ECFLauncherSequenceState::Completed
		|| SequenceRuntime.State == ECFLauncherSequenceState::Cancelled;
	if (!bTerminalState)
	{
		return;
	}

	// [v1.0.0] SequenceCompleted 정책에 따라 완료 시점 쿨다운을 한 번 기록해야 하는지 여부입니다.
	const bool bShouldApplyCompletionCooldown = !bTerminalCooldownApplied
		&& SequenceRuntime.AcceptedProjectileCount > 0
		&& SequenceRuntime.ActiveConfig.CooldownStartPolicy == ECFLauncherCooldownStartPolicy::SequenceCompleted
		&& IsValid(VehicleWeaponComp);
	if (bShouldApplyCompletionCooldown)
	{
		VehicleWeaponComp->RecordAcceptedFire(FMath::Max(TerminalTimeSeconds, 0.0f));
		bTerminalCooldownApplied = true;
	}

	RefreshLauncherSequenceSummary();
}

// [v1.1.0] 현재 SequenceRuntime과 위치·Actor 목표 Snapshot을 읽기 쉬운 한 줄 문자열로 다시 만듭니다.
void UCFLauncherComp::RefreshLauncherSequenceSummary()
{
	// [v1.1.0] 아직 유효한 첫 발사 순간 Guidance Target Actor입니다.
	AActor* GuidanceTargetActor = SequenceCommandTargetSnapshot.GetGuidanceTargetActor();

	// [v1.1.0] Debug 요약에 표시할 유도 목표 Actor 이름입니다.
	const FString GuidanceTargetActorName = IsValid(GuidanceTargetActor)
		? GuidanceTargetActor->GetName()
		: TEXT("None");

	LastLauncherSequenceSummary = FString::Printf(
		TEXT("LauncherSequence: State=%s, Cancel=%s, Pattern=%s, VolleyId=%d, Total=%d, Attempted=%d, Accepted=%d, Failed=%d, Remaining=%d, Pending=%d, NextDelay=%.3fs, Target=(%.1f, %.1f, %.1f), TargetActor=%s, CooldownApplied=%s"),
		*UEnum::GetValueAsString(SequenceRuntime.State),
		*UEnum::GetValueAsString(SequenceRuntime.CancelReason),
		*UEnum::GetValueAsString(SequenceRuntime.ActiveConfig.FirePattern),
		SequenceRuntime.VolleyId,
		SequenceRuntime.TotalProjectileCount,
		SequenceRuntime.AttemptedProjectileCount,
		SequenceRuntime.AcceptedProjectileCount,
		SequenceRuntime.FailedProjectileCount,
		SequenceRuntime.RemainingProjectileCount,
		SequenceRuntime.PendingResultCount,
		SequenceRuntime.TimeUntilNextDispatchSeconds,
		SequenceCommandTargetSnapshot.CommandTargetLocation.X,
		SequenceCommandTargetSnapshot.CommandTargetLocation.Y,
		SequenceCommandTargetSnapshot.CommandTargetLocation.Z,
		*GuidanceTargetActorName,
		bTerminalCooldownApplied ? TEXT("True") : TEXT("False"));
}
