// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-13
// Description: CF-FQ-031 차량 탄약·재장전 공용 타입
// Scope: 탄약 출격 구성, 무기별 장전 상태, 단발·Launcher 예약, 재장전·행동 잠금 enum과 UI/Debug Snapshot 계약을 제공합니다.
// Changelog:
// - v1.3.0: AMMO-P0-06 HUD가 Reload 진행률을 추정하지 않도록 FCFAmmoRuntimeSnapshot에 전체 Reload 시간을 명시적으로 추가.
// - v1.2.0: AMMO-P0-04에서 기존 ReservedSequenceAmmoCount를 실제 Ripple·Salvo 전체 예약·Action Lock 계약으로 활성화.
// - v1.1.0: AMMO-P0-03 SingleCycle 실행 전 임시 ReservedFireTransactionAmmoCount를 추가.
// - v1.0.0: AMMO-P0-01 최초 공용 enum, 초기화 입력, Runtime과 Snapshot 구조체 추가.
// Migration:
// - 기존 WeaponData만 사용하는 차량은 이 타입을 참조하지 않아도 기존 무한탄 발사 결과를 유지합니다.
// - 실제 탄약 소비·Launcher 예약·Reload 전이는 AMMO-P0-03~05에서 이 계약 위에 연결합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.generated.h"

class UCFAmmoData;
class UCFWeaponData;

/**
 * 한 탄창을 어떤 방식으로 재장전할지 구분합니다.
 */
UENUM(BlueprintType, meta=(DisplayName="무기 재장전 방식 (Weapon Reload Mode)", ToolTip="FullMagazine은 한 번에 탄창을 채우고 PerRound는 한 발씩 장전하는 후속 확장 모드입니다."))
enum class ECFWeaponReloadMode : uint8
{
	FullMagazine UMETA(DisplayName="탄창 전체 재장전 (Full Magazine)"),
	PerRound UMETA(DisplayName="한 발씩 재장전 (Per Round)")
};

/**
 * 현재 무기의 탄약·재장전 Runtime 상태입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="무기 재장전 상태 (Weapon Reload State)", ToolTip="탄약 Runtime 초기화, 준비, 재장전, 탄창 비움과 예비 탄약 없음 상태를 구분합니다."))
enum class ECFWeaponReloadState : uint8
{
	NotInitialized UMETA(DisplayName="초기화 안 됨 (Not Initialized)"),
	Ready UMETA(DisplayName="준비 (Ready)"),
	Reloading UMETA(DisplayName="재장전 중 (Reloading)"),
	Empty UMETA(DisplayName="탄창 비움 (Empty)"),
	NoReserveAmmo UMETA(DisplayName="예비 탄약 없음 (No Reserve Ammo)"),
	Disabled UMETA(DisplayName="사용 불가 (Disabled)")
};

/**
 * 탄약·재장전·런처 상태 때문에 무기 행동이 잠긴 대표 사유입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="무기 행동 잠금 사유 (Weapon Action Lock Reason)", ToolTip="현재 무기 교환, 재장전 또는 신규 발사 행동을 막는 대표 사유입니다."))
enum class ECFWeaponActionLockReason : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	LauncherSequenceActive UMETA(DisplayName="런처 시퀀스 진행 중 (Launcher Sequence Active)"),
	Reloading UMETA(DisplayName="재장전 중 (Reloading)"),
	WeaponDisabled UMETA(DisplayName="무기 사용 불가 (Weapon Disabled)"),
	VehicleDestroyed UMETA(DisplayName="차량 파괴 (Vehicle Destroyed)"),
	EquipmentChanging UMETA(DisplayName="장비 교환 중 (Equipment Changing)")
};

/**
 * 탄약 예약·소비·반환 요청의 결과를 구분합니다.
 */
UENUM(BlueprintType, meta=(DisplayName="탄약 처리 결과 (Ammo Transaction Result)", ToolTip="탄약 예약, 소비, 반환 또는 재장전 요청이 성공하거나 거부된 이유입니다."))
enum class ECFAmmoTransactionResult : uint8
{
	Accepted UMETA(DisplayName="승인 (Accepted)"),
	MissingAmmoData UMETA(DisplayName="탄약 데이터 없음 (Missing Ammo Data)"),
	MissingWeaponRuntime UMETA(DisplayName="무기 Runtime 없음 (Missing Weapon Runtime)"),
	InvalidAmmoAmount UMETA(DisplayName="잘못된 탄약 수량 (Invalid Ammo Amount)"),
	NotEnoughLoadedAmmo UMETA(DisplayName="장전 탄약 부족 (Not Enough Loaded Ammo)"),
	SequenceAlreadyActive UMETA(DisplayName="시퀀스 이미 진행 중 (Sequence Already Active)"),
	Reloading UMETA(DisplayName="재장전 중 (Reloading)"),
	ActionLocked UMETA(DisplayName="행동 잠김 (Action Locked)"),
	ExecutionFailed UMETA(DisplayName="발사 실행 실패 (Execution Failed)")
};

/**
 * 한 탄종의 출격 시작 총 적재량을 Ammo Runtime에 전달합니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="출격 탄약 적재 입력 (Ammo Sortie Load)", ToolTip="특정 AmmoData 탄종을 이번 출격에 실제 몇 단위 싣고 나왔는지 전달합니다. 최대 적재 한도가 아니라 실제 현재 수량입니다."))
struct CARFIGHT_RE_API FCFAmmoSortieLoad
{
	GENERATED_BODY()

	// [v1.0.0] 이번 출격에 적재할 탄종 데이터입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Ammo|Initialization", meta=(DisplayName="탄약 데이터 (AmmoData)", ToolTip="이번 출격에 실제 적재되는 탄종 DataAsset입니다."))
	TObjectPtr<UCFAmmoData> AmmoData = nullptr;

	// [v1.0.0] 이번 출격 시작 시 차량 전체에 실제 존재하는 해당 탄종 수량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Ammo|Initialization", meta=(ClampMin="0", DisplayName="출격 시작 탄약량 (InitialSortieAmmoCount)", ToolTip="피팅 최대 한도가 아니라 이번 출격 시작 시 실제 차량에 실린 장전+예비 전체 탄약 수량입니다."))
	int32 InitialSortieAmmoCount = 0;
};

/**
 * 한 무기 인스턴스의 출격 시작 장전 상태를 Ammo Runtime에 전달합니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 탄약 초기화 입력 (Weapon Ammo Initialization)", ToolTip="WeaponInstanceId별 독립 장전 상태를 만들기 위한 무기 데이터와 선택적 초기 장전량 Override입니다."))
struct CARFIGHT_RE_API FCFWeaponAmmoInitialization
{
	GENERATED_BODY()

	// [v1.0.0] 동일 WeaponData를 여러 장착 위치에서 구분할 안정 인스턴스 ID입니다. P0 기본값은 MountProfileId입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Ammo|Initialization", meta=(DisplayName="무기 인스턴스 ID (WeaponInstanceId)", ToolTip="동일 WeaponData를 여러 개 장착해도 장전량을 독립적으로 소유하도록 구분하는 ID입니다. P0에서는 MountProfileId를 권장합니다."))
	FName WeaponInstanceId = NAME_None;

	// [v1.0.0] 탄창 용량, 기본 탄종과 초기 장전 정책을 제공하는 무기 데이터입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Ammo|Initialization", meta=(DisplayName="무기 데이터 (WeaponData)", ToolTip="탄창 용량, 기본 탄종과 탄약 사용 정책을 제공하는 WeaponData입니다."))
	TObjectPtr<UCFWeaponData> WeaponData = nullptr;

	// [v1.0.0] 0 이상이면 WeaponData.InitialLoadedAmmoCount 대신 사용할 출격 초기 장전량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Ammo|Initialization", meta=(ClampMin="-1", DisplayName="초기 장전량 Override", ToolTip="-1이면 WeaponData.InitialLoadedAmmoCount를 사용하고, 0 이상이면 이 값을 출격 초기 장전량으로 사용합니다."))
	int32 InitialLoadedAmmoCountOverride = INDEX_NONE;
};

/**
 * WeaponInstanceId 하나가 소유하는 실제 장전·예약·재장전 Runtime 상태입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 탄약 Runtime (Weapon Ammo Runtime)", ToolTip="WeaponInstanceId별 장전, 시퀀스 예약, 재장전과 출격 통계를 보존하는 실제 Runtime 상태입니다."))
struct CARFIGHT_RE_API FCFWeaponAmmoRuntime
{
	GENERATED_BODY()

	// [v1.0.0] 이 Runtime을 식별하는 무기 인스턴스 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="무기 인스턴스 ID"))
	FName WeaponInstanceId = NAME_None;

	// [v1.0.0] 이 Runtime을 구성한 WeaponData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="무기 데이터"))
	TObjectPtr<UCFWeaponData> WeaponData = nullptr;

	// [v1.0.0] 현재 무기가 사용하는 실제 AmmoData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="현재 탄약 데이터"))
	TObjectPtr<UCFAmmoData> CurrentAmmoData = nullptr;

	// [v1.0.0] 현재 무기 탄창의 최대 탄약 단위 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="탄창 용량"))
	int32 MagazineCapacity = 0;

	// [v1.0.0] 현재 무기에 실제 장전되어 있는 탄약 단위 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="현재 장전량"))
	int32 LoadedAmmoCount = 0;

		// [v1.0.0] 진행 중 Launcher Sequence에 예약되어 신규 행동에 사용할 수 없는 장전 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="시퀀스 예약량"))
	int32 ReservedSequenceAmmoCount = 0;

	// [v1.1.0] 검증 승인 뒤 실제 SingleCycle 실행 결과가 확정되기 전까지 임시 예약한 장전 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="단발 Transaction 예약량", ToolTip="SingleCycle 발사 실행 전에 임시 예약되고 성공 시 소비, 실행 실패 시 반환되는 장전 탄약 수입니다."))
	int32 ReservedFireTransactionAmmoCount = 0;

	// [v1.0.0] 현재 재장전 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="재장전 상태"))
	ECFWeaponReloadState ReloadState = ECFWeaponReloadState::NotInitialized;

	// [v1.0.0] 현재 재장전에서 이미 경과한 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(Units="s", DisplayName="재장전 경과 시간"))
	float ReloadElapsedSeconds = 0.0f;

	// [v1.0.0] 현재 재장전의 전체 소요 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(Units="s", DisplayName="재장전 전체 시간"))
	float ReloadDurationSeconds = 0.0f;

	// [v1.0.0] 재장전 완료 순간 예비 탄약에서 탄창으로 이동할 예정 수량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="대기 재장전 수량"))
	int32 PendingReloadAmount = 0;

	// [v1.0.0] 이번 출격에서 이 무기가 실제 소비한 탄약 단위 누계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="출격 발사 탄약량"))
	int32 FiredAmmoCountThisSortie = 0;

	// [v1.0.0] 현재 또는 마지막 Launcher Sequence에서 실제 소비한 탄약 단위 누계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="현재 시퀀스 발사 탄약량"))
	int32 FiredAmmoCountThisSequence = 0;

	// [v1.0.0] 이번 출격에서 예비 탄약에서 탄창으로 이동한 누계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="출격 재장전 탄약량"))
	int32 ReloadedAmmoCountThisSortie = 0;

	// [v1.0.0] 장전 탄약 부족으로 발사를 거부한 누계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="탄약 부족 거부 횟수"))
	int32 NoAmmoRejectCount = 0;

	// [v1.0.0] 현재 무기 행동이 잠겨 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="무기 행동 잠김"))
	bool bWeaponActionLocked = false;

	// [v1.0.0] 현재 무기 행동을 잠근 대표 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Runtime", meta=(DisplayName="무기 행동 잠금 사유"))
	ECFWeaponActionLockReason WeaponActionLockReason = ECFWeaponActionLockReason::None;
};

/**
 * HUD와 Debug가 내부 Map을 직접 읽지 않고 소비할 계산 완료 탄약 Snapshot입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="탄약 Runtime Snapshot (Ammo Runtime Snapshot)", ToolTip="현재 선택 무기의 장전, 예약, 예비, 사용 가능 총량, 재장전과 행동 잠금 상태를 계산 완료 값으로 제공합니다."))
struct CARFIGHT_RE_API FCFAmmoRuntimeSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] Snapshot 대상 무기 인스턴스 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="무기 인스턴스 ID"))
	FName WeaponInstanceId = NAME_None;

	// [v1.0.0] 현재 탄종의 안정 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="탄약 ID"))
	FName AmmoId = NAME_None;

	// [v1.0.0] 유한 탄약 Runtime Snapshot이 유효한지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="유한 탄약 Runtime 사용"))
	bool bFiniteAmmoRuntimeActive = false;

	// [v1.0.0] 무기의 최대 장전량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="탄창 용량"))
	int32 MagazineCapacity = 0;

	// [v1.0.0] 무기에 실제 장전된 현재 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="현재 장전량"))
	int32 LoadedAmmoCount = 0;

	// [v1.0.0] Launcher Sequence에 예약된 현재 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="시퀀스 예약량"))
	int32 ReservedSequenceAmmoCount = 0;

	// [v1.0.0] 신규 발사 행동에서 즉시 자유롭게 사용할 수 있는 현재 장전 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="즉시 사용 가능 탄약량"))
	int32 ImmediateUsableAmmoCount = 0;

	// [v1.0.0] 차량 탄약고에 남은 같은 탄종 예비 수량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="예비 탄약량"))
	int32 ReserveAmmoCount = 0;

	// [v1.0.0] 현재 무기 기준 신규 행동으로 자유롭게 사용할 수 있는 장전+예비 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="현재 사용 가능 탄약량"))
	int32 CurrentUsableAmmoCount = 0;

	// [v1.0.0] 같은 탄종의 모든 무기 장전량과 차량 예비량을 합친 실제 차량 보유량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="현재 차량 보유 탄약량"))
	int32 CurrentOnboardAmmoCount = 0;

	// [v1.0.0] 현재 장전·예약 상태에서 신규 발사로 즉시 실행 가능한 횟수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="즉시 발사 가능 횟수"))
	int32 ImmediateFireableShotCount = 0;

	// [v1.0.0] 현재 무기와 차량 예비량으로 앞으로 자유롭게 실행 가능한 총 발사 횟수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="남은 자유 발사 가능 횟수"))
	int32 RemainingFreeFireableShotCount = 0;

	// [v1.0.0] 진행 중 Launcher Sequence에서 앞으로 실행할 예약 발사 횟수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="시퀀스 남은 발사 수"))
	int32 PendingSequenceShotCount = 0;

	// [v1.0.0] 현재 또는 마지막 Launcher Sequence에서 실제 소비한 탄약량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="현재 시퀀스 발사 탄약량"))
	int32 FiredAmmoCountThisSequence = 0;

	// [v1.0.0] 이번 출격에서 실제 소비한 탄약량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="출격 발사 탄약량"))
	int32 FiredAmmoCountThisSortie = 0;

	// [v1.0.0] 현재 재장전 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="재장전 상태"))
	ECFWeaponReloadState ReloadState = ECFWeaponReloadState::NotInitialized;

		// [v1.3.0] 현재 Reload 한 사이클의 전체 소요 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(Units="s", DisplayName="전체 재장전 시간", ToolTip="HUD가 진행률을 추정하지 않고 Runtime의 실제 Reload 전체 시간을 사용할 수 있게 제공합니다."))
	float ReloadDurationSeconds = 0.0f;

	// [v1.0.0] 현재 재장전 완료까지 남은 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(Units="s", DisplayName="남은 재장전 시간"))
	float RemainingReloadTimeSeconds = 0.0f;

	// [v1.0.0] 현재 무기 행동이 잠긴 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="무기 행동 잠김"))
	bool bWeaponActionLocked = false;

	// [v1.0.0] 현재 행동 잠금의 대표 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Ammo|Snapshot", meta=(DisplayName="무기 행동 잠금 사유"))
	ECFWeaponActionLockReason WeaponActionLockReason = ECFWeaponActionLockReason::None;
};
