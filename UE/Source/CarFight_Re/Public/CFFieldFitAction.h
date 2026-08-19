// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-15
// Description: CF-FQ-034 FFIT-P0-02~03 Field Fitting Timed Action·Reservation·Coordinator Handoff 계약
// Scope: FFIT-P0-01 Permission PASS 뒤 단일 Inventory Transfer를 Prepare 예약하고, 진행·취소와 Completing → Atomic Coordinator handoff 수명을 관리합니다.
// Changelog:
// - v1.1.0: Completing 상태가 같은 Prepared Transaction을 FCFFieldFitCoordinator에 넘겨 Atomic Equip/Unequip을 완료하고 completion 실패를 Action 상태에 반영하는 정식 handoff를 추가.
// - v1.0.0: Equip/Unequip Timed Action 상태, progress, Permission 변화·외부 신호 취소, Inventory Reservation Prepare/Rollback과 Completing handoff를 최초 추가.
// Migration:
// - Runtime Apply와 Inventory Commit은 수행하지 않습니다. RequiredDuration 도달 시 Reservation을 Prepared로 유지한 `Completing` 상태에서 FFIT-P0-03/M6 Coordinator로 넘깁니다.
// - Permission은 FFIT-P0-01 결과를 소비하며 Weapon/Ammo/Combat/Drive Provider를 직접 조회하지 않습니다.
// - Replace는 기존 계약대로 Unequip 완료 후 별도 Equip이며 한 Action은 단일 Inventory Transfer만 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFFieldFitCoordinator.h"
#include "CFFieldFitPermission.h"
#include "CFInventoryTransfer.h"
#include "CFFieldFitAction.generated.h"

/** Field Fitting 시간 액션 종류입니다. */
UENUM(BlueprintType)
enum class ECFFieldFitActionType : uint8
{
	Equip UMETA(DisplayName="장착 (Equip)"),
	Unequip UMETA(DisplayName="해제 (Unequip)")
};

/** Field Fitting 시간 액션의 현재 수명 상태입니다. */
UENUM(BlueprintType)
enum class ECFFieldFitActionState : uint8
{
	Idle UMETA(DisplayName="대기 (Idle)"),
	InProgress UMETA(DisplayName="진행 중 (In Progress)"),
	Completing UMETA(DisplayName="완료 적용 대기 (Completing)"),
	Completed UMETA(DisplayName="완료 (Completed)"),
	Cancelled UMETA(DisplayName="취소됨 (Cancelled)"),
	Failed UMETA(DisplayName="실패 (Failed)")
};

/** Field Fitting 시간 액션이 종료되거나 중단된 명시적 이유입니다. */
UENUM(BlueprintType)
enum class ECFFieldFitActionEndReason : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	PermissionDenied UMETA(DisplayName="시작 Permission 거부 (Permission Denied)"),
	InventoryReservationFailed UMETA(DisplayName="Inventory 예약 실패 (Inventory Reservation Failed)"),
	InvalidActionInput UMETA(DisplayName="액션 입력 무효 (Invalid Action Input)"),
	CombatStarted UMETA(DisplayName="전투 시작 (Combat Started)"),
	VehicleMoved UMETA(DisplayName="차량 이동 (Vehicle Moved)"),
	VehicleRuntimeNotReady UMETA(DisplayName="차량 런타임 준비 해제 (Vehicle Runtime Not Ready)"),
	CooldownOrConflictingActionStarted UMETA(DisplayName="쿨타임·충돌 액션 시작 (Cooldown Or Conflicting Action Started)"),
	InventoryInaccessible UMETA(DisplayName="Inventory 접근 불가 (Inventory Inaccessible)"),
	DamageReceived UMETA(DisplayName="피해 받음 (Damage Received)"),
	VehicleDestroyed UMETA(DisplayName="차량 파괴 (Vehicle Destroyed)"),
	SlotStateChanged UMETA(DisplayName="장착 슬롯 상태 변경 (Slot State Changed)"),
	InventoryChanged UMETA(DisplayName="Inventory 상태 변경 (Inventory Changed)"),
	ReservationLost UMETA(DisplayName="Reservation 유실 (Reservation Lost)"),
	ScreenClosed UMETA(DisplayName="화면 닫힘 (Screen Closed)"),
	UserCancelled UMETA(DisplayName="사용자 취소 (User Cancelled)"),
		ReservationRollbackFailed UMETA(DisplayName="Reservation Rollback 실패 (Reservation Rollback Failed)"),
	CompletionFailed UMETA(DisplayName="완료 적용 실패 (Completion Failed)"),
	CompletionRecoveryFailed UMETA(DisplayName="완료 보상 복구 실패 (Completion Recovery Failed)")
};

/** Field Fitting Timed Action 시작 입력입니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitActionRequest
{
	GENERATED_BODY()

	// [v1.0.0] 단일 Transfer와 양수 유한 Duration이 Timed Action 시작 계약으로 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] Cargo→Mounted 또는 Mounted→Cargo 방향을 설명하는 Field Fitting 액션 종류입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="Field Fitting 액션 종류 (ActionType)", ToolTip="빈 Mount 장착 또는 기존 Mount 해제 중 이번 단일 시간 액션의 종류입니다."))
	ECFFieldFitActionType ActionType = ECFFieldFitActionType::Equip;

	// [v1.0.0] 액션 시작 시 Prepare 예약하고 완료 시 Coordinator가 같은 Transaction으로 이어받을 Inventory Transfer입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="Inventory Transfer 요청 (InventoryTransferRequest)", ToolTip="시간 액션 시작 시 Prepare 예약하고 완료 적용 단계가 같은 Transaction ID로 이어받을 단일 Inventory 이동 요청입니다."))
	FCFInventoryTransferRequest InventoryTransferRequest;

	// [v1.0.0] 데이터 기반으로 공급되는 전체 Field Fitting 필요 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="필요 액션 시간 초 (RequiredDurationSeconds)", ToolTip="Field Fitting 완료 전에 연속으로 유지해야 하는 데이터 기반 시간입니다. Query 내부 고정 상수가 아닙니다.", ClampMin="0.001"))
	float RequiredDurationSeconds = 0.0f;
};

/** Timed Action 진행 중 외부 Provider가 즉시 취소해야 하는 일회성 사건을 전달합니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitActionSignals
{
	GENERATED_BODY()

	// [v1.0.0] 액션 진행 중 차량이 피해를 받았는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action|Signals", meta=(DisplayName="피해 받음 (bDamageReceived)", ToolTip="이번 업데이트 구간에 차량이 피해를 받아 Field Fitting을 취소해야 하면 True입니다."))
	bool bDamageReceived = false;

	// [v1.0.0] 액션 진행 중 차량이 파괴 상태가 됐는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action|Signals", meta=(DisplayName="차량 파괴됨 (bVehicleDestroyed)", ToolTip="차량 파괴로 Field Fitting을 즉시 취소해야 하면 True입니다."))
	bool bVehicleDestroyed = false;

	// [v1.0.0] 예약 중인 Mount/Cargo 슬롯 의미가 외부에서 바뀌었는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action|Signals", meta=(DisplayName="슬롯 상태 변경 (bSlotStateChanged)", ToolTip="예약 중인 Source 또는 Destination 슬롯 의미가 외부 변경돼 현재 Action을 취소해야 하면 True입니다."))
	bool bSlotStateChanged = false;

	// [v1.0.0] 현재 Action이 가정한 Inventory 소유 상태가 외부에서 바뀌었는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action|Signals", meta=(DisplayName="Inventory 상태 변경 (bInventoryChanged)", ToolTip="현재 Action의 Inventory 가정이 외부 변경돼 취소해야 하면 True입니다."))
	bool bInventoryChanged = false;

	// [v1.0.0] Field Fitting 화면 또는 상위 Action 소유 화면이 닫혔는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action|Signals", meta=(DisplayName="화면 닫힘 (bScreenClosed)", ToolTip="Field Fitting 화면이 닫혀 진행 중 Action과 Reservation을 취소해야 하면 True입니다."))
	bool bScreenClosed = false;

	// [v1.0.0] 사용자가 현재 Field Fitting Action을 명시적으로 취소했는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Fitting|Field|Action|Signals", meta=(DisplayName="사용자 취소 (bUserCancelled)", ToolTip="사용자가 현재 Field Fitting Action 취소를 요청했으면 True입니다."))
	bool bUserCancelled = false;
};

/** UI와 후속 completion 계층이 읽을 Field Fitting Timed Action Snapshot입니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitActionSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] 현재 Action 수명 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="액션 상태 (ActionState)", ToolTip="Idle, InProgress, Completing, Completed, Cancelled 또는 Failed 중 현재 수명 상태입니다."))
	ECFFieldFitActionState ActionState = ECFFieldFitActionState::Idle;

	// [v1.0.0] Cancelled/Failed 종료 또는 시작 거부의 명시적 이유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="종료 이유 (EndReason)", ToolTip="진행 중 취소, 시작 거부 또는 Reservation Rollback 실패의 명시적 이유입니다."))
	ECFFieldFitActionEndReason EndReason = ECFFieldFitActionEndReason::None;

	// [v1.0.0] 현재 Action이 소유하거나 completion 단계로 넘길 Inventory Transaction ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="Inventory Transaction ID (TransactionId)", ToolTip="Prepare Reservation부터 completion까지 같은 Item 이동을 식별하는 Transaction ID입니다."))
	FCFInventoryTransactionId TransactionId;

	// [v1.0.0] 현재까지 FFIT-P0-01 조건을 유지하며 누적한 진행 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="경과 시간 초 (ElapsedSeconds)", ToolTip="Field Fitting 조건을 유지하며 현재까지 누적된 시간입니다."))
	float ElapsedSeconds = 0.0f;

	// [v1.0.0] 액션 시작 요청의 전체 필요 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="필요 시간 초 (RequiredDurationSeconds)", ToolTip="Completing 상태에 도달하기 위한 전체 필요 시간입니다."))
	float RequiredDurationSeconds = 0.0f;

	// [v1.0.0] Elapsed/Required를 0~1로 정규화한 표시용 진행률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="진행률 (ProgressRatio)", ToolTip="UI가 표시할 0~1 범위의 시간 액션 진행률입니다."))
	float ProgressRatio = 0.0f;

	// [v1.0.0] Permission 변화로 중단됐을 때 당시 구조화 blocker를 보존합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Action", meta=(DisplayName="취소 Blocker 목록 (CancellationBlockers)", ToolTip="FFIT-P0-01 Permission 변화 때문에 취소됐을 때 당시 구조화 blocker 목록입니다."))
	TArray<FCFFieldFitBlocker> CancellationBlockers;
};

/** 단일 Field Fitting Inventory Reservation과 시간 진행 수명을 소유하는 순수 C++ 상태 머신입니다. */
class CARFIGHT_RE_API FCFFieldFitTimedAction
{
public:
	// [v1.0.0] Permission PASS 뒤 Inventory Prepare Reservation을 만들고 InProgress로 시작합니다.
	bool Start(
		const FCFFieldFitActionRequest& ActionRequest,
		const FCFFieldFitPermissionResult& PermissionResult,
		const TArray<FCFInventoryContainerState>& Containers,
		const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
		FCFInventoryTransferLedger& TransferLedger);

	// [v1.0.0] 현재 Permission/외부 신호/Reservation을 확인한 뒤 시간을 누적하고 완료 시 Completing으로 전환합니다.
	bool Advance(
		float DeltaSeconds,
		const FCFFieldFitPermissionResult& PermissionResult,
		const FCFFieldFitActionSignals& ActionSignals,
		FCFInventoryTransferLedger& TransferLedger);

	// [v1.0.0] 사용자·화면·상위 시스템의 명시적 종료 사유로 현재 Reservation을 Rollback하고 Cancelled로 전환합니다.
	bool Cancel(ECFFieldFitActionEndReason CancelReason, FCFInventoryTransferLedger& TransferLedger);

		// [v1.0.0] FFIT-P0-03/M6 completion이 성공한 뒤 Action 수명을 Completed로 확정합니다.
	bool MarkCompleted();

	// [v1.1.0] Completing 상태의 Prepared Transaction을 기존 Atomic Coordinator로 넘겨 Runtime·Inventory를 완료하고 Action 최종 상태까지 확정합니다.
	FCFFieldFitCompletionResult CompleteWithCoordinator(
		const FCFVehicleFittingSnapshot& CandidateFittingSnapshot,
		FName RequestedActiveMountProfileId,
		TArray<FCFInventoryContainerState>& InOutContainers,
		const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
		FCFInventoryTransferLedger& TransferLedger,
		ICFFieldFitRuntimeTransaction& RuntimeTransaction);

	// [v1.0.0] 현재 UI·상위 completion 계층이 읽을 불변 Action Snapshot을 반환합니다.
	const FCFFieldFitActionSnapshot& GetSnapshot() const { return ActionSnapshot; }

	// [v1.0.0] completion 단계가 같은 Inventory Transfer 요청을 재사용할 수 있도록 시작 요청을 반환합니다.
	const FCFFieldFitActionRequest& GetActionRequest() const { return CurrentActionRequest; }

private:
	// [v1.0.0] Permission blocker 집합을 Timed Action 취소 이유 하나로 결정론적으로 매핑합니다.
	static ECFFieldFitActionEndReason ResolvePermissionEndReason(const FCFFieldFitPermissionResult& PermissionResult);

	// [v1.0.0] 일회성 외부 신호 중 가장 우선하는 취소 이유를 결정론적으로 반환합니다.
	static ECFFieldFitActionEndReason ResolveSignalEndReason(const FCFFieldFitActionSignals& ActionSignals);

	// [v1.0.0] 현재 Prepared Inventory Transaction을 Rollback하고 Cancelled/Failed 상태를 확정합니다.
	bool CancelAndRollback(
		ECFFieldFitActionEndReason CancelReason,
		const TArray<FCFFieldFitBlocker>& CancellationBlockers,
		FCFInventoryTransferLedger& TransferLedger);

	// [v1.0.0] 현재 Timed Action이 보존하는 원본 시작 요청입니다.
	FCFFieldFitActionRequest CurrentActionRequest;

	// [v1.0.0] UI와 호출자가 읽는 현재 Timed Action 상태 Snapshot입니다.
	FCFFieldFitActionSnapshot ActionSnapshot;
};
