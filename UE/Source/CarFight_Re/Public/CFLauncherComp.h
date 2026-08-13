// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-13
// Description: CarFight 모듈형 런처 발사 시퀀스 컴포넌트
// Scope: 첫 승인 발사 이후 Ripple·Salvo Dispatch, Volley 명령 목표 Snapshot, 유한탄 Sequence 예약 인계, 장비·터렛·차량 상태 취소, Volley 단위 쿨다운과 Debug 상태를 소유합니다.
// Changelog:
// - v1.4.0: CF-FQ-031 AMMO-P0-04에서 Pawn이 첫 발 전에 확보한 Launcher 탄약 예약을 인수하고 후속 성공·실패 발사 및 Terminal에서 Commit·Release하도록 연결.
// - v1.3.0: Completed/Cancelled terminal 이벤트를 SequenceCompleted Cooldown 적용 뒤에 Broadcast해 HUD의 Sequence→Cooldown 전이를 원자화.
// - v1.2.0: 시퀀스 시작·진행·완료·취소·Reset 상태를 HUD 같은 읽기 전용 소비자가 즉시 구독할 수 있는 상태 변경 이벤트를 추가.
// - v1.1.0: 첫 발사 순간 CommandTargetLocation과 GuidanceTargetActor를 함께 Snapshot하고 모든 후속 발사에 같은 값을 전달.
// - v1.0.0: LM-P0-03B Ripple·Salvo Runtime Scheduler 최초 추가.
// Migration:
// - GuidanceTargetActor는 약한 참조이며 목표가 파괴되면 후속 발사에는 None이 전달되어 미사일 목표 소실 정책을 사용합니다.
// - SingleCycle 또는 입력당 발사체 수 1은 기존 Pawn 단발 경로를 사용하고 이 컴포넌트의 Active 시퀀스를 시작하지 않습니다.
// - 진행 중 추가 발사 입력은 기존 시퀀스를 교체하지 않으며 Pawn에서 WeaponCooldown 호환 피드백으로 거부합니다.
// - Projectile 실행, FireOrigin, Damage와 FX는 Pawn의 기존 단발 실행 함수를 재사용하며 이 컴포넌트가 직접 생성하지 않습니다.
// - Completed/Cancelled 상태 구독자는 SequenceCompleted Cooldown이 적용된 뒤 terminal 이벤트를 받습니다.
// - InAmmoWeaponInstanceId가 지정된 유한탄 Sequence는 Pawn이 첫 발 실행 전에 전체 유효 발수를 예약한 상태여야 시작하며, Terminal에서 남은 예약과 Action Lock을 정리합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFLauncherTypes.h"
#include "Components/ActorComponent.h"
#include "CFLauncherComp.generated.h"

class AActor;
class ACFVehiclePawn;
class UCFTurretMountData;
class UCFVehicleWeaponComp;
class UCFWeaponData;

/**
 * 런처 시퀀스의 결정적 상태가 변경됐음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFLauncherSequenceChangedSignature, FCFLauncherSequenceRuntime, SequenceRuntime);

/**
 * 차량 런처의 Ripple·Salvo 예약 발사와 취소·완료 상태를 관리합니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFLauncherComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] Tick 기반 발사 시퀀스 갱신을 사용할 기본 컴포넌트 값을 초기화합니다.
	UCFLauncherComp();

	// [v1.0.0] EndPlay에서 예약 발사와 런타임 참조를 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// [v1.0.0] Ripple 시간 진행과 Salvo 후속 처리 묶음을 갱신합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [v1.0.0] 런처 시퀀스가 사용할 Owner Pawn과 상위 WeaponComp를 연결합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Launcher", meta=(DisplayName="런처 런타임 초기화 (Initialize Launcher Runtime)", ToolTip="Owner 차량 Pawn과 WeaponComp를 연결하고 진행 중 예약 발사를 초기화합니다."))
	bool InitializeLauncherRuntime(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleWeaponComp* InVehicleWeaponComp);

		// [v1.4.0] 첫 발이 이미 승인·실행된 발사 패턴의 목표 Snapshot과 남은 Ripple·Salvo 및 선택적 유한탄 예약을 인수해 시퀀스를 시작합니다.
	bool StartFireSequenceAfterFirstAcceptedShot(
		const FCFLauncherFirePatternConfig& InFirePatternConfig,
		const FVector& InCommandTargetLocation,
		AActor* InGuidanceTargetActor,
		float FirstAcceptedFireTimeSeconds,
		FName InAmmoWeaponInstanceId = NAME_None);

	// [v1.0.0] 진행 중인 발사 시퀀스를 지정한 사유로 취소합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Launcher", meta=(DisplayName="런처 발사 시퀀스 취소 (Cancel Fire Sequence)", ToolTip="진행 중인 Ripple 또는 Salvo 시퀀스를 취소합니다. 이미 발사된 Projectile은 회수하거나 제거하지 않습니다."))
	void CancelFireSequence(ECFLauncherSequenceCancelReason CancelReason = ECFLauncherSequenceCancelReason::Manual);

	// [v1.1.0] 시퀀스 상태와 위치·Actor 목표 Snapshot을 신규 입력 대기 상태로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Launcher", meta=(DisplayName="런처 런타임 초기화 (Reset Launcher Runtime)", ToolTip="예약 발사 상태, 장비 스냅샷과 Command Target 위치·유도 목표 Actor를 초기화합니다."))
	void ResetLauncherRuntime();

	// [v1.0.0] 후속 Ripple·Salvo Projectile이 남은 Active 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Launcher", meta=(DisplayName="발사 시퀀스 진행 여부 (Is Fire Sequence Active)", ToolTip="후속 Projectile Dispatch가 남아 있는 Ripple 또는 Salvo 시퀀스가 진행 중이면 True입니다."))
	bool IsFireSequenceActive() const { return SequenceRuntime.IsActive(); }

	// [v1.0.0] 마지막 또는 현재 발사 시퀀스 런타임 스냅샷을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Launcher", meta=(DisplayName="런처 시퀀스 Runtime 반환 (Get Launcher Sequence Runtime)", ToolTip="Volley ID, 전체·시도·승인·실패·남은 수량과 상태를 포함한 런타임 스냅샷입니다."))
	FCFLauncherSequenceRuntime GetLauncherSequenceRuntime() const { return SequenceRuntime; }

	// [v1.1.0] 진행 중인 시퀀스가 첫 발사 순간 고정한 Command Target 위치를 반환합니다.
	bool TryGetActiveCommandTargetLocation(FVector& OutCommandTargetLocation) const
	{
		if (!SequenceRuntime.IsActive() || !SequenceCommandTargetSnapshot.HasValidCommandTargetLocation())
		{
			OutCommandTargetLocation = FVector::ZeroVector;
			return false;
		}

		OutCommandTargetLocation = SequenceCommandTargetSnapshot.CommandTargetLocation;
		return true;
	}

	// [v1.1.0] 진행 중인 시퀀스가 첫 발사 순간 고정한 유도 목표 Actor를 반환합니다.
	AActor* GetActiveGuidanceTargetActor() const
	{
		return SequenceRuntime.IsActive()
			? SequenceCommandTargetSnapshot.GetGuidanceTargetActor()
			: nullptr;
	}

	// [v1.0.0] 현재 또는 마지막 발사 시퀀스의 한 줄 Debug 요약을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Launcher", meta=(DisplayName="런처 시퀀스 요약 반환 (Get Launcher Sequence Summary)", ToolTip="발사 패턴, Volley ID, 시도·승인·실패·남은 수량, 상태와 취소 사유를 표시합니다."))
	FString GetLauncherSequenceSummary() const { return LastLauncherSequenceSummary; }

	// [v1.2.0] 시퀀스 시작·진행·완료·취소·Reset의 실제 Runtime 변경을 읽기 전용 소비자에게 통지합니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|Launcher|Events", meta=(DisplayName="런처 시퀀스 상태 변경 이벤트", ToolTip="Ripple 또는 Salvo 시퀀스의 시작, 발사 수 진행, 완료, 취소 또는 Reset으로 결정적 Runtime 상태가 바뀔 때 현재 스냅샷을 전달합니다."))
	FCFLauncherSequenceChangedSignature OnLauncherSequenceChanged;

private:
	// [v1.0.0] Owner·Health·Weapon·Turret 스냅샷이 현재 시퀀스를 계속 실행할 수 있는지 검사합니다.
	ECFLauncherSequenceCancelReason ValidateActiveSequenceRuntime() const;

	// [v1.0.0] 이번 Tick에서 허용된 수량만큼 Pawn의 기존 단발 실행 경로를 호출합니다.
	void DispatchDueShots(float DeltaSeconds);

		// [v1.3.0] Completed 또는 Cancelled 상태에서 SequenceCompleted 쿨다운과 최종 요약을 적용한 뒤 terminal 상태 이벤트를 한 번 Broadcast합니다.
	void FinalizeTerminalSequence(float TerminalTimeSeconds);

	// [v1.4.0] 현재 Launcher Sequence가 인수한 남은 탄약 예약과 LauncherSequenceActive Action Lock을 안전하게 반환합니다.
	void ReleaseActiveAmmoReservation();

	// [v1.0.0] 현재 SequenceRuntime을 읽기 쉬운 한 줄 문자열로 다시 만듭니다.
	void RefreshLauncherSequenceSummary();

	// [v1.3.0] 현재 결정적 SequenceRuntime 스냅샷을 상태 변경 구독자에게 전달합니다. Terminal 상태는 Cooldown 적용 이후에만 호출합니다.
	void BroadcastLauncherSequenceChanged();

	// [v1.0.0] 실제 Projectile·HitScan 단발 실행을 제공하는 Owner 차량 Pawn입니다.
	UPROPERTY(Transient)
	TObjectPtr<ACFVehiclePawn> OwnerVehiclePawn = nullptr;

	// [v1.0.0] 활성 장비와 쿨다운을 소유하는 상위 WeaponComp입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleWeaponComp> VehicleWeaponComp = nullptr;

	// [v1.0.0] 시퀀스 시작 시 복사한 WeaponData 참조입니다. 변경되면 시퀀스를 취소합니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFWeaponData> SequenceWeaponData = nullptr;

	// [v1.0.0] 시퀀스 시작 시 복사한 TurretMountData 참조입니다. 변경되면 시퀀스를 취소합니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFTurretMountData> SequenceTurretMountData = nullptr;

	// [v1.1.0] 첫 발사 순간 함께 고정한 Volley 공통 Command Target 위치와 약한 Guidance Target Actor입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Launcher|Runtime", meta=(AllowPrivateAccess="true", DisplayName="시퀀스 명령 목표 Snapshot (SequenceCommandTargetSnapshot)", ToolTip="Ripple·Salvo 첫 발과 모든 후속 발사가 공통으로 사용하는 첫 발사 순간 Command Target 위치와 유도 목표 Actor입니다."))
	FCFLauncherCommandTargetSnapshot SequenceCommandTargetSnapshot;

	// [v1.0.0] 첫 승인 발사의 월드 시간입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Launcher|Runtime", meta=(AllowPrivateAccess="true", DisplayName="첫 승인 발사 시간 (FirstAcceptedFireTimeSeconds)", ToolTip="현재 Volley의 첫 Projectile이 승인된 월드 시간입니다."))
	float FirstAcceptedFireTimeSeconds = -1.0f;

	// [v1.0.0] 다음 새 발사 입력에 부여할 증가형 Volley ID입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Launcher|Runtime", meta=(AllowPrivateAccess="true", DisplayName="다음 발사 묶음 ID (NextVolleyId)", ToolTip="다음 Ripple·Salvo 시퀀스에 부여할 증가형 Volley ID입니다."))
	int32 NextVolleyId = 1;

		// [v1.0.0] 이번 terminal 상태에서 SequenceCompleted 쿨다운을 이미 적용했는지 여부입니다.
	UPROPERTY(Transient)
	bool bTerminalCooldownApplied = false;

	// [v1.4.0] 현재 유한탄 Launcher Sequence 예약을 식별하는 Ammo Runtime WeaponInstanceId입니다.
	UPROPERTY(Transient)
	FName SequenceAmmoWeaponInstanceId = NAME_None;

	// [v1.4.0] 현재 Sequence가 VehicleAmmoComp의 Launcher 예약과 Action Lock을 인수한 상태인지 여부입니다.
	UPROPERTY(Transient)
	bool bSequenceAmmoReservationActive = false;

	// [v1.0.0] 현재 또는 마지막 발사 시퀀스의 결정적 상태입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Launcher|Runtime", meta=(AllowPrivateAccess="true", DisplayName="런처 시퀀스 Runtime (SequenceRuntime)", ToolTip="발사 패턴, Volley ID, 수량, 시간과 완료·취소 상태를 보존합니다."))
	FCFLauncherSequenceRuntime SequenceRuntime;

	// [v1.0.0] VehicleDebug와 로그에서 사용할 시퀀스 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Launcher|Runtime", meta=(AllowPrivateAccess="true", DisplayName="런처 시퀀스 요약 (LastLauncherSequenceSummary)", ToolTip="현재 또는 마지막 발사 시퀀스 상태를 한 줄로 표시합니다."))
	FString LastLauncherSequenceSummary = TEXT("LauncherSequence: NotInitialized");
};
