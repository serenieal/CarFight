// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-14
// Description: CF-FQ-034 FFIT-P0-01 Field Fitting Permission·Blocker Query 계약
// Scope: Combat/Drive/Runtime/Weapon/Launcher/Ammo/Defense/Action/Inventory Provider 상태를 읽어 시작 허용 여부와 구조화 blocker를 생성합니다.
// Changelog:
// - v1.0.0: Provider별 입력, 구조화 BlockReason과 순수 Permission Query를 최초 추가.
// Migration:
// - Reservation 생성·Timed Action 시작은 FFIT-P0-02가 소유하며 이 Query는 상태를 변경하지 않습니다.
// - 전역 CombatState가 아직 없으므로 Combat Provider 결과를 입력으로 받고 새로운 전투 상태 시스템을 만들지 않습니다.
// - 속도 임계값과 정지 유지시간은 호출자가 데이터 기반 값으로 공급하며 Query 내부에 게임 튜닝 상수를 고정하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFFieldFitPermission.generated.h"

/** Field Fitting 시작을 차단한 상태를 제공한 논리 Provider 종류입니다. */
UENUM(BlueprintType)
enum class ECFFieldFitBlockerSource : uint8
{
	PermissionInput UMETA(DisplayName="Permission 입력 (Permission Input)"),
	Combat UMETA(DisplayName="전투 (Combat)"),
	Drive UMETA(DisplayName="주행 (Drive)"),
	VehicleRuntime UMETA(DisplayName="차량 런타임 (Vehicle Runtime)"),
	Weapon UMETA(DisplayName="무기 (Weapon)"),
	Launcher UMETA(DisplayName="런처 (Launcher)"),
	Ammo UMETA(DisplayName="탄약 (Ammo)"),
	Defense UMETA(DisplayName="방어·유틸리티 (Defense Utility)"),
	Action UMETA(DisplayName="시간 액션 (Action)"),
	Inventory UMETA(DisplayName="인벤토리 (Inventory)")
};

/** Field Fitting 시작을 차단하는 구조화 사유입니다. */
UENUM(BlueprintType)
enum class ECFFieldFitBlockReason : uint8
{
	InvalidPermissionInput UMETA(DisplayName="Permission 입력 무효 (Invalid Permission Input)"),
	CombatActive UMETA(DisplayName="전투 중 (Combat Active)"),
	VehicleMoving UMETA(DisplayName="차량 이동 중 (Vehicle Moving)"),
	StationaryDurationInsufficient UMETA(DisplayName="정지 유지시간 부족 (Stationary Duration Insufficient)"),
	VehicleRuntimeNotReady UMETA(DisplayName="차량 런타임 준비 안 됨 (Vehicle Runtime Not Ready)"),
	WeaponCooldownActive UMETA(DisplayName="무기 쿨타임 진행 중 (Weapon Cooldown Active)"),
	LauncherSequenceActive UMETA(DisplayName="런처 시퀀스 진행 중 (Launcher Sequence Active)"),
	AmmoReloadActive UMETA(DisplayName="재장전 진행 중 (Ammo Reload Active)"),
	AmmoTransferActive UMETA(DisplayName="탄약 이동 진행 중 (Ammo Transfer Active)"),
	DefenseCooldownActive UMETA(DisplayName="방어·유틸리티 쿨타임 진행 중 (Defense Cooldown Active)"),
	RepairActive UMETA(DisplayName="수리 진행 중 (Repair Active)"),
	ConflictingTimedActionActive UMETA(DisplayName="충돌 시간 액션 진행 중 (Conflicting Timed Action Active)"),
	InventoryInaccessible UMETA(DisplayName="인벤토리 접근 불가 (Inventory Inaccessible)")
};

/** 하나의 Field Fitting 시작 blocker입니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitBlocker
{
	GENERATED_BODY()

	// [v1.0.0] blocker 상태를 제공한 논리 Provider입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Permission", meta=(DisplayName="Blocker 제공자 (Source)", ToolTip="이 Field Fitting 시작 차단 사유를 제공한 Combat, Drive, Weapon 등의 논리 Provider입니다."))
	ECFFieldFitBlockerSource Source = ECFFieldFitBlockerSource::PermissionInput;

	// [v1.0.0] UI와 Action 계층이 분기할 구조화 차단 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Permission", meta=(DisplayName="차단 사유 (Reason)", ToolTip="Field Fitting 시작을 차단한 구체적인 구조화 사유입니다."))
	ECFFieldFitBlockReason Reason = ECFFieldFitBlockReason::InvalidPermissionInput;
};

/** 각 소유 Provider가 계산한 현재 Field Fitting 시작 상태를 모은 읽기 전용 입력입니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitPermissionInput
{
	GENERATED_BODY()

	// [v1.0.0] 수치와 정지 튜닝 값이 유한하고 음수가 아닌지 검증합니다.
	bool IsValid() const;

	// [v1.0.0] Combat Provider가 현재 전투 상태라고 판정했는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Combat", meta=(DisplayName="전투 중 (bCombatActive)", ToolTip="Combat Provider가 현재 차량을 전투 상태로 판정했으면 True입니다."))
	bool bCombatActive = false;

	// [v1.0.0] Drive Provider가 제공한 현재 차량 절대 속도 km/h입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Drive", meta=(DisplayName="현재 차량 속도 km/h (VehicleSpeedKmh)", ToolTip="Drive Provider가 제공한 현재 차량 절대 속도입니다. Query는 데이터 기반 정지 임계값과 비교합니다.", ClampMin="0.0"))
	float VehicleSpeedKmh = 0.0f;

	// [v1.0.0] 정지 임계값 이하 상태가 연속 유지된 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Drive", meta=(DisplayName="정지 유지 시간 초 (StationaryDurationSeconds)", ToolTip="차량 속도가 정지 임계값 이하로 연속 유지된 시간입니다.", ClampMin="0.0"))
	float StationaryDurationSeconds = 0.0f;

	// [v1.0.0] 호출자가 데이터에서 공급하는 정지 판정 최대 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Drive", meta=(DisplayName="정지 속도 임계값 km/h (StationarySpeedThresholdKmh)", ToolTip="Field Fitting을 정지 상태로 인정할 데이터 기반 최대 속도입니다.", ClampMin="0.0"))
	float StationarySpeedThresholdKmh = 0.0f;

	// [v1.0.0] 호출자가 데이터에서 공급하는 최소 연속 정지 유지시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Drive", meta=(DisplayName="필요 정지 유지 시간 초 (RequiredStationaryDurationSeconds)", ToolTip="Field Fitting 시작 전에 요구되는 데이터 기반 최소 연속 정지 시간입니다.", ClampMin="0.0"))
	float RequiredStationaryDurationSeconds = 0.0f;

	// [v1.0.0] Vehicle Runtime Provider가 Field Fitting에 필요한 차량 런타임을 준비했다고 판정했는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Runtime", meta=(DisplayName="차량 런타임 준비 완료 (bVehicleRuntimeReady)", ToolTip="Field Fitting Runtime 적용에 필요한 차량 런타임이 준비됐으면 True입니다."))
	bool bVehicleRuntimeReady = false;

	// [v1.0.0] Weapon Provider에서 관련 무기 쿨타임이 하나라도 진행 중인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Weapon", meta=(DisplayName="무기 쿨타임 진행 중 (bWeaponCooldownActive)", ToolTip="Field Fitting과 관련된 무기 쿨타임이 아직 남아 있으면 True입니다."))
	bool bWeaponCooldownActive = false;

	// [v1.0.0] Launcher Provider에서 발사 시퀀스가 진행 중인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Launcher", meta=(DisplayName="런처 시퀀스 진행 중 (bLauncherSequenceActive)", ToolTip="Ripple 또는 Salvo 같은 Launcher 발사 시퀀스가 진행 중이면 True입니다."))
	bool bLauncherSequenceActive = false;

	// [v1.0.0] Ammo Provider에서 재장전이 진행 중인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Ammo", meta=(DisplayName="재장전 진행 중 (bAmmoReloadActive)", ToolTip="관련 무기의 Ammo Reload가 진행 중이면 True입니다."))
	bool bAmmoReloadActive = false;

	// [v1.0.0] Ammo Provider에서 별도 탄약 이동 액션이 진행 중인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Ammo", meta=(DisplayName="탄약 이동 진행 중 (bAmmoTransferActive)", ToolTip="향후 탄약 이동 또는 보급 액션이 진행 중이면 True입니다."))
	bool bAmmoTransferActive = false;

	// [v1.0.0] Defense/Utility Provider에서 관련 쿨타임이 진행 중인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Defense", meta=(DisplayName="방어·유틸리티 쿨타임 진행 중 (bDefenseCooldownActive)", ToolTip="Field Fitting과 충돌하는 Defense 또는 Utility 쿨타임이 진행 중이면 True입니다."))
	bool bDefenseCooldownActive = false;

	// [v1.0.0] Defense/Utility Provider에서 수리 액션이 진행 중인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Defense", meta=(DisplayName="수리 진행 중 (bRepairActive)", ToolTip="차량 또는 장비 수리 액션이 진행 중이면 True입니다."))
	bool bRepairActive = false;

	// [v1.0.0] Action Provider에서 Field Fitting과 동시에 수행할 수 없는 다른 시간 액션이 진행 중인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Action", meta=(DisplayName="충돌 시간 액션 진행 중 (bConflictingTimedActionActive)", ToolTip="다른 Field Fitting, 보급, 수리 등 Field Fitting과 충돌하는 시간 액션이 진행 중이면 True입니다."))
	bool bConflictingTimedActionActive = false;

	// [v1.0.0] Inventory Access Provider에서 현재 차량의 필요한 Container에 접근 가능하다고 판정했는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Permission|Inventory", meta=(DisplayName="인벤토리 접근 가능 (bInventoryAccessible)", ToolTip="현재 차량의 Field Fitting에 필요한 Inventory Container에 접근 가능하면 True입니다. Reservation 성공 여부가 아닙니다."))
	bool bInventoryAccessible = false;
};

/** Field Fitting 시작 Permission과 모든 구조화 blocker 결과입니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitPermissionResult
{
	GENERATED_BODY()

	// [v1.0.0] blocker가 하나도 없어 Field Fitting Timed Action 시작 단계로 넘어갈 수 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Permission", meta=(DisplayName="Field Fitting 시작 가능 (bCanStart)", ToolTip="True이면 FFIT-P0-01 조건을 모두 만족해 다음 Timed Action·Reservation 단계로 넘어갈 수 있습니다."))
	bool bCanStart = false;

	// [v1.0.0] Provider 순서대로 결정론적으로 수집된 현재 시작 차단 사유 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Permission", meta=(DisplayName="Field Fitting 차단 사유 목록 (Blockers)", ToolTip="현재 Field Fitting 시작을 차단하는 모든 구조화 사유입니다. 비어 있으면 시작 가능합니다."))
	TArray<FCFFieldFitBlocker> Blockers;
};

/** Provider별 읽기 전용 상태를 Field Fitting 시작 Permission으로 합성하는 순수 Query입니다. */
struct CARFIGHT_RE_API FCFFieldFitPermissionQuery
{
	// [v1.0.0] 입력을 변경하지 않고 모든 blocker를 고정 순서로 수집해 최종 Permission을 반환합니다.
	static FCFFieldFitPermissionResult Evaluate(const FCFFieldFitPermissionInput& PermissionInput);
};
