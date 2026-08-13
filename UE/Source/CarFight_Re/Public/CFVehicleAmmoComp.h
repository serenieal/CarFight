// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-13
// Description: CF-FQ-031 차량 탄약 Runtime 단일 소유 컴포넌트
// Scope: 탄종별 차량 예비량, WeaponInstanceId별 장전 상태, SingleCycle Transaction, Launcher Sequence 예약·Action Lock, FullMagazine Reload와 계산 완료 Snapshot을 소유합니다.
// Changelog:
// - v1.3.0: AMMO-P0-05 Pause-safe FullMagazine 수동·자동 Reload, 완료 시점 Reserve→Loaded 이동, Reloading Action Lock과 취소 API를 추가.
// - v1.2.0: AMMO-P0-04 Ripple·Salvo 전체 유효 발수 예약, 성공 발사 Commit, 실패 발사 Release, Terminal 전체 반환과 LauncherSequenceActive Action Lock을 추가.
// - v1.1.0: AMMO-P0-03 SingleCycle 발사 전 Validate·Reserve, 성공 Commit, 실패 Rollback과 단발 Transaction 예약량을 추가.
// - v1.0.0: AMMO-P0-02 초기 출격 구성, 장전·예비 분리, 공유 예비량, Snapshot과 Reset 수명 계약 추가.
// Migration:
// - 기존 WeaponData는 유한탄 설정이 명시되지 않으면 이 컴포넌트가 발사 동작을 변경하지 않습니다.
// - SingleCycle과 Launcher Sequence는 기존 예약·소비 계약을 유지합니다.
// - P0 실제 Reload는 FullMagazine만 실행하며 PerRound는 후속 확장용 데이터 계약으로 유지합니다.
// - Reload 진행은 일반 Component Tick을 사용하고 TickEvenWhenPaused=false이므로 게임 Pause 동안 진행되지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleAmmoComp.generated.h"

class ACFVehiclePawn;
class UCFAmmoData;
class UCFVehicleWeaponComp;

/**
 * 실제 탄약 Snapshot이 바뀌었음을 HUD·Debug 소비자에게 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFAmmoRuntimeChangedSignature, FCFAmmoRuntimeSnapshot, AmmoSnapshot);

/**
 * 차량 한 대의 출격 탄약 상태를 단일 소유합니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleAmmoComp : public UActorComponent
{
	GENERATED_BODY()

public:
		// [v1.3.0] Reload 중에만 일반 게임 Tick을 사용하도록 탄약 컴포넌트 기본값을 설정합니다.
	UCFVehicleAmmoComp();

	// [v1.3.0] 활성 Reload의 경과 시간을 게임 DeltaSeconds로 진행하고 완료 시 Reserve→Loaded 이동을 확정합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [v1.0.0] 월드 종료에서 출격 탄약 상태가 다음 수명으로 남지 않도록 초기화합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// [v1.0.0] 탄종별 실제 출격 적재량과 무기 인스턴스별 초기 장전량으로 Runtime을 원자적으로 구성합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo", meta=(DisplayName="탄약 Runtime 초기화", ToolTip="이번 출격의 실제 탄종별 총 적재량과 무기 인스턴스별 초기 장전량으로 장전·예비 Runtime을 구성합니다. 피팅 최대 적재 한도를 현재 수량으로 사용하지 않습니다."))
	bool InitializeAmmoRuntime(
		ACFVehiclePawn* InOwnerVehiclePawn,
		const TArray<FCFAmmoSortieLoad>& SortieAmmoLoads,
		const TArray<FCFWeaponAmmoInitialization>& WeaponInitializations);

	// [v1.0.0] 현재 WeaponComp의 활성 무기 하나를 명시적 출격 총 탄수로 초기화하는 단일 무기 편의 경로입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo", meta=(DisplayName="활성 무기 탄약 Runtime 초기화", ToolTip="현재 활성 WeaponData와 MountProfileId를 사용해 단일 무기 탄약 Runtime을 구성합니다. InitialSortieAmmoCount는 실제 출격 적재량이어야 합니다."))
	bool InitializeActiveWeaponAmmoRuntime(
		ACFVehiclePawn* InOwnerVehiclePawn,
		UCFVehicleWeaponComp* InVehicleWeaponComp,
		int32 InitialSortieAmmoCount);

		// [v1.0.0] 모든 장전·예비·예약·통계 상태를 비우고 NotInitialized 상태로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo", meta=(DisplayName="탄약 Runtime 초기화 상태 해제", ToolTip="현재 출격의 장전, 예비, 예약, 재장전과 통계를 모두 비웁니다."))
	void ResetAmmoRuntime();

	// [v1.1.0] SingleCycle 한 발을 실행하기 전에 현재 장전량·Reload·Action Lock 상태를 검증합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Transaction", meta=(DisplayName="단발 탄약 검증", ToolTip="현재 WeaponInstance가 한 발에 필요한 탄약을 자유 장전량에서 사용할 수 있는지 검증합니다. 탄약 부족 거부 통계도 기록합니다."))
	ECFAmmoTransactionResult ValidateSingleFireAmmo(FName WeaponInstanceId);

	// [v1.1.0] 실제 SingleCycle 실행 전에 한 발에 필요한 탄약을 장전량에서 임시 예약합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Transaction", meta=(DisplayName="단발 탄약 예약", ToolTip="발사 실행 전 필요한 장전 탄약을 임시 예약합니다. 아직 LoadedAmmoCount에서는 차감하지 않으며 성공 Commit 또는 실패 Rollback이 반드시 뒤따라야 합니다."))
	ECFAmmoTransactionResult ReserveSingleFireAmmo(FName WeaponInstanceId);

	// [v1.1.0] 성공한 SingleCycle 실행의 임시 예약량을 실제 장전 탄약에서 정확히 한 번 소비합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Transaction", meta=(DisplayName="단발 탄약 소비 확정", ToolTip="성공한 발사 실행 뒤 임시 예약량을 LoadedAmmoCount에서 소비하고 출격 발사 통계를 갱신합니다."))
	ECFAmmoTransactionResult CommitSingleFireAmmo(FName WeaponInstanceId);

		// [v1.1.0] 실패한 SingleCycle 실행의 임시 예약량을 소비 없이 전부 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Transaction", meta=(DisplayName="단발 탄약 예약 반환", ToolTip="Projectile 실행 실패나 사출 경로 거부처럼 실제 발사가 성립하지 않았을 때 임시 예약량만 해제하고 LoadedAmmoCount는 유지합니다."))
	ECFAmmoTransactionResult RollbackSingleFireAmmo(FName WeaponInstanceId);

	// [v1.2.0] Ripple·Salvo 시작 전에 현재 자유 장전량으로 실행 가능한 전체 유효 발수를 한 번에 예약하고 Action Lock을 겁니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Launcher", meta=(DisplayName="런처 시퀀스 탄약 예약", ToolTip="첫 발 실행 전에 요청한 Ripple·Salvo 전체 발수를 예약합니다. 부분 시퀀스 허용 시 현재 장전량으로 가능한 발수만 예약하며 최대 적재량이나 예비 탄약을 자동 장전하지 않습니다."))
	ECFAmmoTransactionResult ReserveLauncherSequenceAmmo(
		FName WeaponInstanceId,
		int32 RequestedShotCount,
		bool bAllowPartialSequence,
		int32& OutReservedShotCount);

	// [v1.2.0] 실제 성공한 Launcher 내부 발사 한 발의 예약량을 Loaded에서 소비로 확정합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Launcher", meta=(DisplayName="런처 예약 발사 소비 확정", ToolTip="이미 전체 예약된 Launcher Sequence에서 실제 성공한 한 발에 필요한 탄약 단위를 LoadedAmmoCount에서 소비합니다."))
	ECFAmmoTransactionResult CommitReservedLauncherShot(FName WeaponInstanceId);

	// [v1.2.0] 실행 실패한 Launcher 내부 발사 한 발의 예약량을 Loaded 소비 없이 자유 장전량으로 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Launcher", meta=(DisplayName="런처 실패 발사 예약 반환", ToolTip="예약된 Launcher 내부 발사가 실패했을 때 해당 한 발 분량의 예약만 해제하고 LoadedAmmoCount는 유지합니다."))
	ECFAmmoTransactionResult ReleaseReservedLauncherShot(FName WeaponInstanceId);

	// [v1.2.0] 완료·취소·시작 실패 시 아직 남은 Launcher 예약 전체를 반환하고 Action Lock을 해제합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Launcher", meta=(DisplayName="런처 시퀀스 예약 전체 반환", ToolTip="Launcher Sequence가 완료, 취소되거나 시작에 실패했을 때 미실행 예약을 모두 해제하고 LauncherSequenceActive 행동 잠금을 제거합니다."))
	ECFAmmoTransactionResult ReleaseLauncherSequenceReservation(FName WeaponInstanceId);

		// [v1.2.0] 지정 무기가 현재 Launcher Sequence 예약 또는 LauncherSequenceActive Action Lock을 소유하는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo|Launcher", meta=(DisplayName="런처 시퀀스 탄약 예약 여부", ToolTip="지정 WeaponInstanceId에 진행 중 Launcher 예약 또는 LauncherSequenceActive 행동 잠금이 남아 있으면 True입니다."))
	bool HasActiveLauncherSequenceReservation(FName WeaponInstanceId) const;

	// [v1.3.0] 지정 WeaponInstanceId의 FullMagazine 재장전을 수동으로 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Reload", meta=(DisplayName="무기 재장전 요청", ToolTip="Launcher Sequence와 다른 Action Lock이 없을 때 현재 장전량과 예비량으로 FullMagazine 재장전을 시작합니다. 완료 전에는 탄약 수량을 이동하지 않습니다."))
	ECFAmmoTransactionResult RequestReload(FName WeaponInstanceId);

	// [v1.3.0] 현재 VehicleWeaponComp 활성 MountProfileId의 FullMagazine 재장전을 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Reload", meta=(DisplayName="활성 무기 재장전 요청", ToolTip="현재 활성 MountProfileId를 WeaponInstanceId로 사용해 FullMagazine 재장전을 요청합니다."))
	ECFAmmoTransactionResult RequestActiveWeaponReload(const UCFVehicleWeaponComp* VehicleWeaponComp);

	// [v1.3.0] 진행 중인 지정 무기 Reload를 수량 이동 없이 취소합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Ammo|Reload", meta=(DisplayName="무기 재장전 취소", ToolTip="진행 중인 재장전을 취소하고 PendingReloadAmount를 버립니다. 장전량과 예비량은 변경하지 않습니다."))
	ECFAmmoTransactionResult CancelReload(FName WeaponInstanceId);

	// [v1.3.0] 지정 WeaponInstanceId가 현재 Reloading 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo|Reload", meta=(DisplayName="무기 재장전 중 여부", ToolTip="지정 WeaponInstanceId의 ReloadState가 Reloading이면 True입니다."))
	bool IsReloadActive(FName WeaponInstanceId) const;

	// [v1.0.0] 지정 WeaponInstanceId의 계산 완료 Snapshot을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo", meta=(DisplayName="무기 탄약 Snapshot 조회", ToolTip="지정 WeaponInstanceId의 장전, 예비, 사용 가능량과 재장전 상태를 계산 완료 Snapshot으로 반환합니다."))
	bool TryGetAmmoSnapshot(FName WeaponInstanceId, FCFAmmoRuntimeSnapshot& OutAmmoSnapshot) const;

	// [v1.0.0] 현재 VehicleWeaponComp 활성 MountProfileId의 탄약 Snapshot을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo", meta=(DisplayName="활성 무기 탄약 Snapshot 조회", ToolTip="현재 활성 MountProfileId를 WeaponInstanceId로 사용해 탄약 Snapshot을 반환합니다."))
	bool TryGetActiveWeaponAmmoSnapshot(const UCFVehicleWeaponComp* VehicleWeaponComp, FCFAmmoRuntimeSnapshot& OutAmmoSnapshot) const;

	// [v1.0.0] 지정 탄종 ID의 차량 예비 탄약 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo", meta=(DisplayName="예비 탄약량 반환", ToolTip="차량 탄약고에 남아 있는 지정 AmmoId의 실제 예비 탄약 수량을 반환합니다."))
	int32 GetReserveAmmoCount(FName AmmoId) const;

	// [v1.0.0] 현재 생성된 유한탄 WeaponInstance Runtime 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo", meta=(DisplayName="무기 탄약 Runtime 수 반환", ToolTip="현재 출격에서 유한 탄약 상태를 소유하는 WeaponInstanceId 개수를 반환합니다."))
	int32 GetWeaponAmmoRuntimeCount() const { return WeaponAmmoRuntimeByInstanceId.Num(); }

	// [v1.0.0] 현재 출격 탄약 Runtime 초기화가 성공했는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo", meta=(DisplayName="탄약 Runtime 초기화 여부", ToolTip="InitializeAmmoRuntime 또는 무한탄 호환 초기화가 성공했으면 True입니다."))
	bool IsAmmoRuntimeInitialized() const { return bAmmoRuntimeInitialized; }

	// [v1.0.0] 마지막 초기화·Reset 결과 요약을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Ammo", meta=(DisplayName="탄약 Runtime 요약 반환", ToolTip="현재 출격 탄약 Runtime의 초기화 결과와 무기·탄종 개수를 한 줄로 반환합니다."))
	FString GetLastAmmoRuntimeSummary() const { return LastAmmoRuntimeSummary; }

	// [v1.0.0] 실제 유한 탄약 Snapshot 변경을 UI·Debug에 전달합니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|Ammo|Events", meta=(DisplayName="탄약 Runtime 변경 이벤트", ToolTip="장전, 예비, 예약 또는 재장전 상태가 바뀔 때 계산 완료 Snapshot을 전달합니다."))
	FCFAmmoRuntimeChangedSignature OnAmmoRuntimeChanged;

private:
	// [v1.3.0] 수동·자동 공통 FullMagazine Reload 시작 검증과 Pending 상태 설정을 수행합니다.
	ECFAmmoTransactionResult StartReloadInternal(FName WeaponInstanceId, bool bAutomaticRequest);

	// [v1.3.0] Empty 상태와 WeaponData 자동 재장전 정책을 만족하면 Action Lock을 확인한 뒤 Reload를 시작합니다.
	bool TryStartAutoReloadInternal(FCFWeaponAmmoRuntime& WeaponAmmoRuntime);

	// [v1.3.0] Reload 완료 시점의 실제 공유 Reserve와 탄창 부족량을 다시 확인해 수량 이동을 한 번만 확정합니다.
	void CompleteReloadInternal(FCFWeaponAmmoRuntime& WeaponAmmoRuntime);

	// [v1.3.0] 진행 중 Reload를 수량 이동 없이 정리하고 지정 후속 상태·Action Lock을 적용합니다.
	void CancelReloadInternal(FCFWeaponAmmoRuntime& WeaponAmmoRuntime, ECFWeaponReloadState ResultState, ECFWeaponActionLockReason ResultLockReason);

	// [v1.3.0] 현재 하나라도 Reloading 상태가 있으면 Tick을 켜고 없으면 다시 끕니다.
	void RefreshReloadTickEnabled();

	// [v1.1.0] 상태를 변경하지 않고 SingleCycle 한 발의 현재 탄약 Transaction 가능 여부를 계산합니다.
	ECFAmmoTransactionResult EvaluateSingleFireAmmo(FName WeaponInstanceId) const;

	// [v1.1.0] 장전·예약·예비량 변화 뒤 현재 무기가 Ready·Empty·NoReserveAmmo 중 어느 상태인지 다시 계산합니다.
	void UpdateWeaponReloadStateAfterAmmoChange(FCFWeaponAmmoRuntime& WeaponAmmoRuntime);

	// [v1.0.0] 내부 Weapon Runtime 한 건에서 UI·Debug용 계산 완료 Snapshot을 생성합니다.
	FCFAmmoRuntimeSnapshot BuildAmmoSnapshot(const FCFWeaponAmmoRuntime& WeaponAmmoRuntime) const;

	// [v1.0.0] 같은 AmmoId를 사용하는 모든 무기 장전량과 차량 예비량을 합친 실제 보유량을 계산합니다.
	int32 CalculateCurrentOnboardAmmoCount(FName AmmoId) const;

	// [v1.0.0] 현재 모든 유한탄 WeaponInstance Snapshot을 상태 변경 이벤트로 전달합니다.
	void BroadcastAllAmmoSnapshots();

	// [v1.0.0] 현재 출격 탄약 상태를 소유하는 차량 Pawn입니다.
	UPROPERTY(Transient)
	TObjectPtr<ACFVehiclePawn> OwnerVehiclePawn = nullptr;

	// [v1.0.0] 탄종 ID별 차량 공용 예비 탄약 수량입니다.
	UPROPERTY(Transient)
	TMap<FName, int32> ReserveAmmoCountByAmmoId;

	// [v1.0.0] WeaponInstanceId별 독립 장전·예약·재장전 Runtime입니다.
	UPROPERTY(Transient)
	TMap<FName, FCFWeaponAmmoRuntime> WeaponAmmoRuntimeByInstanceId;

	// [v1.0.0] 현재 출격 탄약 Runtime이 성공적으로 초기화됐는지 여부입니다.
	UPROPERTY(Transient)
	bool bAmmoRuntimeInitialized = false;

	// [v1.0.0] 마지막 초기화·Reset 결과를 Debug에서 확인할 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(AllowPrivateAccess="true", DisplayName="탄약 Runtime 요약", ToolTip="마지막 탄약 Runtime 초기화 또는 Reset 결과를 한 줄로 표시합니다."))
	FString LastAmmoRuntimeSummary = TEXT("AmmoRuntime: NotInitialized");
};
