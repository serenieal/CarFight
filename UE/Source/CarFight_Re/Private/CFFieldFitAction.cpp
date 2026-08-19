// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-15
// Description: CF-FQ-034 FFIT-P0-02~03 Field Fitting Timed Action·Reservation·Atomic Coordinator Handoff 구현
// Scope: Permission PASS 시 Reservation을 소유하고 진행·취소하며, Completing 상태에서 같은 Prepared Transaction을 Atomic Coordinator로 넘깁니다.
// Changelog:
// - v1.1.0: CompleteWithCoordinator를 추가해 AlreadyPrepared handoff, 완료 성공→Completed, 보상 완료 실패→Failed, 복구 실패→CompletionRecoveryFailed를 명시적으로 연결.
// - v1.0.0: Start, Advance, Cancel, Completing handoff와 구조화 취소 사유 매핑을 최초 구현.
// Migration:
// - Runtime Apply/Inventory Commit은 수행하지 않으며 Completing 이후 FFIT-P0-03/M6가 같은 Transaction ID를 이어받습니다.

#include "CFFieldFitAction.h"

// [v1.0.0] 단일 Transfer와 양수 유한 Duration이 Timed Action 시작 계약으로 유효한지 반환합니다.
bool FCFFieldFitActionRequest::IsValid() const
{
	return InventoryTransferRequest.IsValid()
		&& FMath::IsFinite(RequiredDurationSeconds)
		&& RequiredDurationSeconds > 0.0f;
}

// [v1.0.0] Permission PASS 뒤 Inventory Prepare Reservation을 만들고 InProgress로 시작합니다.
bool FCFFieldFitTimedAction::Start(
	const FCFFieldFitActionRequest& ActionRequest,
	const FCFFieldFitPermissionResult& PermissionResult,
	const TArray<FCFInventoryContainerState>& Containers,
	const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
	FCFInventoryTransferLedger& TransferLedger)
{
	if (ActionSnapshot.ActionState != ECFFieldFitActionState::Idle)
	{
		return false;
	}

	if (!ActionRequest.IsValid())
	{
		ActionSnapshot.ActionState = ECFFieldFitActionState::Failed;
		ActionSnapshot.EndReason = ECFFieldFitActionEndReason::InvalidActionInput;
		return false;
	}

	CurrentActionRequest = ActionRequest;
	ActionSnapshot.TransactionId = ActionRequest.InventoryTransferRequest.TransactionId;
	ActionSnapshot.RequiredDurationSeconds = ActionRequest.RequiredDurationSeconds;
	if (!PermissionResult.bCanStart || !PermissionResult.Blockers.IsEmpty())
	{
		ActionSnapshot.ActionState = ECFFieldFitActionState::Failed;
		ActionSnapshot.EndReason = ECFFieldFitActionEndReason::PermissionDenied;
		ActionSnapshot.CancellationBlockers = PermissionResult.Blockers;
		return false;
	}

	// [v1.0.0] Timed Action 수명 동안 Item과 Destination을 잠글 실제 Prepare 결과입니다.
	const FCFInventoryTransferResult PrepareResult = TransferLedger.PrepareTransfer(
		Containers,
		ContainerMassSnapshots,
		ActionRequest.InventoryTransferRequest);
	if (PrepareResult.TransferStatus != ECFInventoryTransferStatus::Prepared)
	{
		ActionSnapshot.ActionState = ECFFieldFitActionState::Failed;
		ActionSnapshot.EndReason = ECFFieldFitActionEndReason::InventoryReservationFailed;
		return false;
	}

	ActionSnapshot.ActionState = ECFFieldFitActionState::InProgress;
	ActionSnapshot.EndReason = ECFFieldFitActionEndReason::None;
	ActionSnapshot.ElapsedSeconds = 0.0f;
	ActionSnapshot.ProgressRatio = 0.0f;
	ActionSnapshot.CancellationBlockers.Reset();
	return true;
}

// [v1.0.0] 현재 Permission/외부 신호/Reservation을 확인한 뒤 시간을 누적하고 완료 시 Completing으로 전환합니다.
bool FCFFieldFitTimedAction::Advance(
	const float DeltaSeconds,
	const FCFFieldFitPermissionResult& PermissionResult,
	const FCFFieldFitActionSignals& ActionSignals,
	FCFInventoryTransferLedger& TransferLedger)
{
	if (ActionSnapshot.ActionState != ECFFieldFitActionState::InProgress)
	{
		return false;
	}

	if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds < 0.0f)
	{
		return CancelAndRollback(
			ECFFieldFitActionEndReason::InvalidActionInput,
			TArray<FCFFieldFitBlocker>(),
			TransferLedger);
	}

	// [v1.0.0] 파괴·피해·화면 종료 같은 일회성 사건이 Permission보다 우선하는 취소 이유입니다.
	const ECFFieldFitActionEndReason SignalEndReason = ResolveSignalEndReason(ActionSignals);
	if (SignalEndReason != ECFFieldFitActionEndReason::None)
	{
		return CancelAndRollback(SignalEndReason, TArray<FCFFieldFitBlocker>(), TransferLedger);
	}

	if (!PermissionResult.bCanStart || !PermissionResult.Blockers.IsEmpty())
	{
		return CancelAndRollback(
			ResolvePermissionEndReason(PermissionResult),
			PermissionResult.Blockers,
			TransferLedger);
	}

	if (TransferLedger.GetTransactionState(CurrentActionRequest.InventoryTransferRequest.TransactionId) != ECFInventoryTransactionState::Prepared)
	{
		return CancelAndRollback(
			ECFFieldFitActionEndReason::ReservationLost,
			TArray<FCFFieldFitBlocker>(),
			TransferLedger);
	}

	ActionSnapshot.ElapsedSeconds = FMath::Min(
		ActionSnapshot.ElapsedSeconds + DeltaSeconds,
		ActionSnapshot.RequiredDurationSeconds);
	ActionSnapshot.ProgressRatio = FMath::Clamp(
		ActionSnapshot.ElapsedSeconds / ActionSnapshot.RequiredDurationSeconds,
		0.0f,
		1.0f);
	if (ActionSnapshot.ElapsedSeconds >= ActionSnapshot.RequiredDurationSeconds)
	{
		ActionSnapshot.ActionState = ECFFieldFitActionState::Completing;
	}
	return true;
}

// [v1.0.0] 사용자·화면·상위 시스템의 명시적 종료 사유로 현재 Reservation을 Rollback하고 Cancelled로 전환합니다.
bool FCFFieldFitTimedAction::Cancel(
	const ECFFieldFitActionEndReason CancelReason,
	FCFInventoryTransferLedger& TransferLedger)
{
	if (ActionSnapshot.ActionState != ECFFieldFitActionState::InProgress
		&& ActionSnapshot.ActionState != ECFFieldFitActionState::Completing)
	{
		return false;
	}

	return CancelAndRollback(CancelReason, TArray<FCFFieldFitBlocker>(), TransferLedger);
}

// [v1.0.0] FFIT-P0-03/M6 completion이 성공한 뒤 Action 수명을 Completed로 확정합니다.
bool FCFFieldFitTimedAction::MarkCompleted()
{
	if (ActionSnapshot.ActionState != ECFFieldFitActionState::Completing)
	{
		return false;
	}

	ActionSnapshot.ActionState = ECFFieldFitActionState::Completed;
	ActionSnapshot.EndReason = ECFFieldFitActionEndReason::None;
	ActionSnapshot.ProgressRatio = 1.0f;
	return true;
}

// [v1.1.0] Completing 상태의 Prepared Transaction을 기존 Atomic Coordinator로 넘겨 Runtime·Inventory를 완료하고 Action 최종 상태까지 확정합니다.
FCFFieldFitCompletionResult FCFFieldFitTimedAction::CompleteWithCoordinator(
	const FCFVehicleFittingSnapshot& CandidateFittingSnapshot,
	const FName RequestedActiveMountProfileId,
	TArray<FCFInventoryContainerState>& InOutContainers,
	const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
	FCFInventoryTransferLedger& TransferLedger,
	ICFFieldFitRuntimeTransaction& RuntimeTransaction)
{
	// [v1.1.0] Coordinator 실행 전 Action/Reservation handoff 선행조건 실패를 반환할 결과입니다.
	FCFFieldFitCompletionResult CompletionResult;
	CompletionResult.TransactionId = CurrentActionRequest.InventoryTransferRequest.TransactionId;
	if (ActionSnapshot.ActionState != ECFFieldFitActionState::Completing
		|| !CandidateFittingSnapshot.IsValid()
		|| TransferLedger.GetTransactionState(CurrentActionRequest.InventoryTransferRequest.TransactionId) != ECFInventoryTransactionState::Prepared)
	{
		CompletionResult.CompletionStatus = CandidateFittingSnapshot.IsValid()
			? ECFFieldFitCompletionStatus::InventoryPrepareFailed
			: ECFFieldFitCompletionStatus::InvalidRequest;
		CompletionResult.FailureSummary = TEXT("FieldFitAction: CompletionHandoffPreconditionFailed");
		ActionSnapshot.ActionState = ECFFieldFitActionState::Failed;
		ActionSnapshot.EndReason = CandidateFittingSnapshot.IsValid()
			? ECFFieldFitActionEndReason::ReservationLost
			: ECFFieldFitActionEndReason::CompletionFailed;
		return CompletionResult;
	}

	// [v1.1.0] Action이 시작 때 소유한 동일 Transfer와 검증 후보 Snapshot을 Atomic Coordinator에 넘길 요청입니다.
	FCFFieldFitCompletionRequest CompletionRequest;
	CompletionRequest.InventoryTransferRequest = CurrentActionRequest.InventoryTransferRequest;
	CompletionRequest.CandidateFittingSnapshot = CandidateFittingSnapshot;
	CompletionRequest.RequestedActiveMountProfileId = RequestedActiveMountProfileId;

	CompletionResult = FCFFieldFitCoordinator::Complete(
		InOutContainers,
		ContainerMassSnapshots,
		TransferLedger,
		RuntimeTransaction,
		CompletionRequest);
	if (CompletionResult.IsSuccessful())
	{
		ActionSnapshot.ActionState = ECFFieldFitActionState::Completed;
		ActionSnapshot.EndReason = ECFFieldFitActionEndReason::None;
		ActionSnapshot.ProgressRatio = 1.0f;
		return CompletionResult;
	}

	ActionSnapshot.ActionState = ECFFieldFitActionState::Failed;
	ActionSnapshot.EndReason = CompletionResult.CompletionStatus == ECFFieldFitCompletionStatus::RecoveryFailed
		? ECFFieldFitActionEndReason::CompletionRecoveryFailed
		: ECFFieldFitActionEndReason::CompletionFailed;
	return CompletionResult;
}

// [v1.0.0] Permission blocker 집합을 Timed Action 취소 이유 하나로 결정론적으로 매핑합니다.
ECFFieldFitActionEndReason FCFFieldFitTimedAction::ResolvePermissionEndReason(
	const FCFFieldFitPermissionResult& PermissionResult)
{
	for (const FCFFieldFitBlocker& Blocker : PermissionResult.Blockers)
	{
		switch (Blocker.Reason)
		{
		case ECFFieldFitBlockReason::CombatActive:
			return ECFFieldFitActionEndReason::CombatStarted;
		case ECFFieldFitBlockReason::VehicleMoving:
		case ECFFieldFitBlockReason::StationaryDurationInsufficient:
			return ECFFieldFitActionEndReason::VehicleMoved;
		case ECFFieldFitBlockReason::VehicleRuntimeNotReady:
			return ECFFieldFitActionEndReason::VehicleRuntimeNotReady;
		case ECFFieldFitBlockReason::WeaponCooldownActive:
		case ECFFieldFitBlockReason::LauncherSequenceActive:
		case ECFFieldFitBlockReason::AmmoReloadActive:
		case ECFFieldFitBlockReason::AmmoTransferActive:
		case ECFFieldFitBlockReason::DefenseCooldownActive:
		case ECFFieldFitBlockReason::RepairActive:
		case ECFFieldFitBlockReason::ConflictingTimedActionActive:
			return ECFFieldFitActionEndReason::CooldownOrConflictingActionStarted;
		case ECFFieldFitBlockReason::InventoryInaccessible:
			return ECFFieldFitActionEndReason::InventoryInaccessible;
		case ECFFieldFitBlockReason::InvalidPermissionInput:
		default:
			return ECFFieldFitActionEndReason::InvalidActionInput;
		}
	}

	return ECFFieldFitActionEndReason::PermissionDenied;
}

// [v1.0.0] 일회성 외부 신호 중 가장 우선하는 취소 이유를 결정론적으로 반환합니다.
ECFFieldFitActionEndReason FCFFieldFitTimedAction::ResolveSignalEndReason(
	const FCFFieldFitActionSignals& ActionSignals)
{
	if (ActionSignals.bVehicleDestroyed)
	{
		return ECFFieldFitActionEndReason::VehicleDestroyed;
	}
	if (ActionSignals.bDamageReceived)
	{
		return ECFFieldFitActionEndReason::DamageReceived;
	}
	if (ActionSignals.bSlotStateChanged)
	{
		return ECFFieldFitActionEndReason::SlotStateChanged;
	}
	if (ActionSignals.bInventoryChanged)
	{
		return ECFFieldFitActionEndReason::InventoryChanged;
	}
	if (ActionSignals.bScreenClosed)
	{
		return ECFFieldFitActionEndReason::ScreenClosed;
	}
	if (ActionSignals.bUserCancelled)
	{
		return ECFFieldFitActionEndReason::UserCancelled;
	}
	return ECFFieldFitActionEndReason::None;
}

// [v1.0.0] 현재 Prepared Inventory Transaction을 Rollback하고 Cancelled/Failed 상태를 확정합니다.
bool FCFFieldFitTimedAction::CancelAndRollback(
	const ECFFieldFitActionEndReason CancelReason,
	const TArray<FCFFieldFitBlocker>& CancellationBlockers,
	FCFInventoryTransferLedger& TransferLedger)
{
	// [v1.0.0] 현재 Action이 소유한 Prepared Transaction을 해제할 실제 Ledger 결과입니다.
	const FCFInventoryTransferResult RollbackResult = TransferLedger.RollbackTransfer(
		CurrentActionRequest.InventoryTransferRequest.TransactionId);
	ActionSnapshot.CancellationBlockers = CancellationBlockers;
	if (!RollbackResult.IsSuccessful())
	{
		ActionSnapshot.ActionState = ECFFieldFitActionState::Failed;
		ActionSnapshot.EndReason = ECFFieldFitActionEndReason::ReservationRollbackFailed;
		return false;
	}

	ActionSnapshot.ActionState = ECFFieldFitActionState::Cancelled;
	ActionSnapshot.EndReason = CancelReason;
	return true;
}
