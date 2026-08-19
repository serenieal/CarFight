// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-15
// Description: CF-FQ-035 / CF-FQ-034 Field Fitting completion 원자 조율·Chaos Field Mass Reapply 구현
// Scope: Inventory, Weapon·Defense Runtime과 선택적 Vehicle Mass Runtime의 Commit·보상 순서를 구현합니다.
// Changelog:
// - v1.3.0: 실제 CarFight Wheeled Vehicle의 Movement Mass 변경·Chaos Vehicle Physics State 재생성·Configured/Actual/Transform/속도/RuntimeReady 검증 Adapter를 추가.
// - v1.2.0: Commit 실패 뒤 실제 복구 성공 여부를 Coordinator가 판정해 RecoveryFailed를 명시적으로 구분.
// - v1.1.0: mass-changing 후보의 선택적 Mass Runtime Commit·Compensation을 추가.
// - v1.0.0: Coordinator 성공·실패 흐름, Inventory Rollback, Runtime compensation과 same-mass FittingComp Adapter를 최초 구현.
// Migration:
// - Permission/Timed Action/Field Mass Reapply는 구현하지 않습니다.
// - Inventory Ledger가 이미 Committed/RolledBack인 Transaction은 새 completion 호출로 재사용하지 않고 명시 실패로 반환합니다.

#include "CFFieldFitCoordinator.h"

#include "CFVehiclePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"


// [v1.0.0] Inventory Transfer와 후보 Fitting Snapshot의 최소 completion 계약이 유효한지 반환합니다.
bool FCFFieldFitCompletionRequest::IsValid() const
{
	return InventoryTransferRequest.IsValid()
		&& CandidateFittingSnapshot.IsValid()
		&& CandidateFittingSnapshot.VehicleData != nullptr;
}

// [v1.3.0] 실제 Field Fitting 대상 차량 Pawn을 Mass Runtime에 연결합니다.
FCFChaosVehicleMassRuntime::FCFChaosVehicleMassRuntime(ACFVehiclePawn* InVehiclePawn)
	: VehiclePawn(InVehiclePawn)
{
}

// [v1.3.0] Movement Mass를 갱신하고 Chaos Vehicle Physics State를 재생성한 뒤 Configured·Actual 질량과 차량 상태 보존을 검증합니다.
bool FCFChaosVehicleMassRuntime::ReapplyVehicleMassKg(const float TargetMassKg, FString& OutFailureSummary)
{
	OutFailureSummary.Reset();
	if (!IsValid(VehiclePawn) || !FMath::IsFinite(TargetMassKg) || TargetMassKg <= 0.0f)
	{
		OutFailureSummary = TEXT("FieldFitChaosMass: ReapplyFailed, InvalidVehicleOrTarget");
		return false;
	}

	// [v1.3.0] AWheeledVehiclePawn이 소유하는 실제 Chaos Vehicle Movement를 Field Mass SSOT mirror로 사용합니다.
	UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent());
	// [v1.3.0] 실제 chassis Body 질량·Physics State를 readback할 inherited VehicleMesh입니다.
	USkeletalMeshComponent* VehicleMeshComponent = VehiclePawn->GetMesh();
	if (!VehicleMovementComponent || !VehicleMeshComponent)
	{
		OutFailureSummary = TEXT("FieldFitChaosMass: ReapplyFailed, VehicleComponentMissing");
		return false;
	}

	if (!VehiclePawn->HasActorBegunPlay()
		|| !VehicleMovementComponent->HasValidPhysicsState()
		|| !VehicleMeshComponent->IsPhysicsStateCreated()
		|| !VehicleMeshComponent->IsSimulatingPhysics()
		|| !VehicleMeshComponent->GetPhysicsAsset())
	{
		OutFailureSummary = TEXT("FieldFitChaosMass: ReapplyFailed, PhysicsRuntimeNotReady");
		return false;
	}

	// [v1.3.0] Physics State 재생성 전 현재 Chaos 설정 질량입니다.
	const float PreviousConfiguredMassKg = VehicleMovementComponent->Mass;
	// [v1.3.0] PhysicsAsset 보조 Body 오버헤드까지 포함한 현재 실제 VehicleMesh 집계 질량입니다.
	const float PreviousActualMassKg = VehicleMeshComponent->GetMass();
	if (!FMath::IsFinite(PreviousConfiguredMassKg)
		|| PreviousConfiguredMassKg <= 0.0f
		|| !FMath::IsFinite(PreviousActualMassKg)
		|| PreviousActualMassKg <= 0.0f)
	{
		OutFailureSummary = TEXT("FieldFitChaosMass: ReapplyFailed, PreviousMassInvalid");
		return false;
	}

	// [v1.3.0] PhysicsAsset 보조 Body 질량은 유지된다는 전제에서 새 Movement Mass가 반영됐을 때 기대할 실제 집계 질량입니다.
	const float ExpectedActualMassKg = PreviousActualMassKg + (TargetMassKg - PreviousConfiguredMassKg);
	// [v1.3.0] Physics State 재생성 전 보존해야 할 차량 World Transform입니다.
	const FTransform PreviousActorTransform = VehiclePawn->GetActorTransform();
	// [v1.3.0] 정지 임계값 안의 미세 속도까지 재생성 뒤 보존할 chassis 선속도입니다.
	const FVector PreviousLinearVelocity = VehicleMeshComponent->GetPhysicsLinearVelocity();
	// [v1.3.0] 정지 임계값 안의 미세 회전 속도까지 재생성 뒤 보존할 chassis 각속도입니다.
	const FVector PreviousAngularVelocityDegrees = VehicleMeshComponent->GetPhysicsAngularVelocityInDegrees();
	// [v1.3.0] Field Mass Reapply 전 차량 Core Runtime Ready 상태입니다.
	const bool bPreviousCoreRuntimeReady = VehiclePawn->bVehicleCoreRuntimeReady;
	// [v1.3.0] Field Mass Reapply 전 차량 Combat Runtime Ready 상태입니다.
	const bool bPreviousCombatRuntimeReady = VehiclePawn->bVehicleCombatRuntimeReady;

	VehicleMovementComponent->Mass = TargetMassKg;
	VehicleMovementComponent->RecreatePhysicsState();

	if (!VehicleMovementComponent->HasValidPhysicsState()
		|| !VehicleMeshComponent->IsPhysicsStateCreated()
		|| !VehicleMeshComponent->IsSimulatingPhysics()
		|| !VehicleMeshComponent->GetPhysicsAsset())
	{
		OutFailureSummary = TEXT("FieldFitChaosMass: ReapplyFailed, PhysicsStateRecreateFailed");
		return false;
	}

	// [v1.3.0] Field Fitting은 정지 상태에서 수행하지만 threshold 안의 물리 속도도 손실하지 않도록 재생성 직후 복원합니다.
	VehicleMeshComponent->SetPhysicsLinearVelocity(PreviousLinearVelocity, false);
	VehicleMeshComponent->SetPhysicsAngularVelocityInDegrees(PreviousAngularVelocityDegrees, false);

	// [v1.3.0] 재생성 뒤 Chaos Movement에 유지된 설정 질량입니다.
	const float AppliedConfiguredMassKg = VehicleMovementComponent->Mass;
	// [v1.3.0] 재생성 뒤 VehicleMesh BodyInstance가 보고하는 실제 집계 질량입니다.
	const float AppliedActualMassKg = VehicleMeshComponent->GetMass();
	// [v1.3.0] PhysicsAsset 집계 질량 readback에 허용할 기존 FIT-P0-05 기준 오차입니다.
	const float ActualMassToleranceKg = UCFVehicleFittingComp::CalculateInitialMassToleranceKg(TargetMassKg);
	// [v1.3.0] Physics State 재생성으로 Actor Transform이 이동하지 않았는지 확인할 허용 오차입니다.
	constexpr float TransformTolerance = 0.01f;

	if (!FMath::IsNearlyEqual(AppliedConfiguredMassKg, TargetMassKg, 0.01f)
		|| !FMath::IsFinite(AppliedActualMassKg)
		|| !FMath::IsNearlyEqual(AppliedActualMassKg, ExpectedActualMassKg, ActualMassToleranceKg)
		|| !VehiclePawn->GetActorTransform().Equals(PreviousActorTransform, TransformTolerance)
		|| VehiclePawn->bVehicleCoreRuntimeReady != bPreviousCoreRuntimeReady
		|| VehiclePawn->bVehicleCombatRuntimeReady != bPreviousCombatRuntimeReady)
	{
		OutFailureSummary = FString::Printf(
			TEXT("FieldFitChaosMass: ReapplyFailed, VerificationMismatch, Target=%.3f, Configured=%.3f, ExpectedActual=%.3f, Actual=%.3f, Tolerance=%.3f, CoreReady=%s/%s, CombatReady=%s/%s"),
			TargetMassKg,
			AppliedConfiguredMassKg,
			ExpectedActualMassKg,
			AppliedActualMassKg,
			ActualMassToleranceKg,
			bPreviousCoreRuntimeReady ? TEXT("True") : TEXT("False"),
			VehiclePawn->bVehicleCoreRuntimeReady ? TEXT("True") : TEXT("False"),
			bPreviousCombatRuntimeReady ? TEXT("True") : TEXT("False"),
			VehiclePawn->bVehicleCombatRuntimeReady ? TEXT("True") : TEXT("False"));
		return false;
	}

	return true;
}

// [v1.0.0] 실제 FittingComp와 Weapon·Defense Runtime Apply Adapter를 연결합니다. Mass Adapter가 없으므로 same-mass 후보만 허용합니다.
FCFFieldFitRuntimeAdapter::FCFFieldFitRuntimeAdapter(UCFVehicleFittingComp* InFittingComponent, ICFFittingRuntimeApplyAdapter& InRuntimeApplyAdapter)
	: FittingComponent(InFittingComponent)
	, RuntimeApplyAdapter(&InRuntimeApplyAdapter)
{
}

// [v1.1.0] Weapon·Defense Runtime과 Field Mass Runtime을 하나의 transaction에 연결합니다.
FCFFieldFitRuntimeAdapter::FCFFieldFitRuntimeAdapter(
	UCFVehicleFittingComp* InFittingComponent,
	ICFFittingRuntimeApplyAdapter& InRuntimeApplyAdapter,
	ICFFieldFitMassRuntime& InMassRuntimeAdapter)
	: FittingComponent(InFittingComponent)
	, RuntimeApplyAdapter(&InRuntimeApplyAdapter)
	, MassRuntimeAdapter(&InMassRuntimeAdapter)
{
}

// [v1.0.0] 직전 Applied Snapshot과 같은 질량의 후보만 Checkpoint와 함께 준비합니다.
bool FCFFieldFitRuntimeAdapter::PrepareRuntime(
	const FCFVehicleFittingSnapshot& CandidateFittingSnapshot,
	const FName RequestedActiveMountProfileId,
	FString& OutFailureSummary)
{
		OutFailureSummary.Reset();
	PreviousRuntimeCheckpoint = FCFFittingRuntimeCheckpoint();
	bCandidateRuntimeCommitted = false;
	bRequiresMassReapply = false;
	bCandidateMassApplied = false;
	PreviousAppliedMassKg = 0.0f;
	CandidateMassKg = 0.0f;
	bLastCommitFailureRecovered = true;

	if (!FittingComponent || !RuntimeApplyAdapter || !CandidateFittingSnapshot.IsValid())
	{
		OutFailureSummary = TEXT("FieldFitRuntime: PrepareFailed, InvalidAdapterOrSnapshot");
		return false;
	}

	if (!FittingComponent->HasAppliedRuntimeInput() || !FittingComponent->HasAppliedFittingSnapshot())
	{
		OutFailureSummary = TEXT("FieldFitRuntime: PrepareFailed, PreviousAppliedSnapshotRequiredForMassCheck");
		return false;
	}

	// [v1.0.0] Field 질량 변경을 허용하기 전에 비교할 직전 Applied Snapshot입니다.
	const FCFVehicleFittingSnapshot PreviousAppliedSnapshot = FittingComponent->GetAppliedFittingSnapshot();
	if (!PreviousAppliedSnapshot.IsValid() || PreviousAppliedSnapshot.VehicleData != CandidateFittingSnapshot.VehicleData)
	{
		OutFailureSummary = TEXT("FieldFitRuntime: PrepareFailed, AppliedSnapshotVehicleMismatch");
		return false;
	}

		// [v1.1.0] 부동소수점 계산 오차만 허용해 실제 질량 재적용 필요 여부를 판정할 경계입니다.
	constexpr float SameMassToleranceKg = 0.01f;
	PreviousAppliedMassKg = PreviousAppliedSnapshot.TotalVehicleMassKg;
	CandidateMassKg = CandidateFittingSnapshot.TotalVehicleMassKg;
	bRequiresMassReapply = !FMath::IsNearlyEqual(PreviousAppliedMassKg, CandidateMassKg, SameMassToleranceKg);
	if (bRequiresMassReapply && !MassRuntimeAdapter)
	{
		OutFailureSummary = FString::Printf(
			TEXT("FieldFitRuntime: PrepareFailed, RuntimeMassReapplyUnsupported, Previous=%.3f, Candidate=%.3f, Tolerance=%.3f"),
			PreviousAppliedMassKg,
			CandidateMassKg,
			SameMassToleranceKg);
		return false;
	}

	PreviousRuntimeCheckpoint = FittingComponent->CaptureAppliedRuntimeCheckpoint();
	if (!PreviousRuntimeCheckpoint.IsValid())
	{
		OutFailureSummary = TEXT("FieldFitRuntime: PrepareFailed, RuntimeCheckpointInvalid");
		return false;
	}

	if (!FittingComponent->PrepareSortieFittingSnapshot(CandidateFittingSnapshot, RequestedActiveMountProfileId))
	{
		OutFailureSummary = FittingComponent->GetLastFittingRuntimeSummary();
		PreviousRuntimeCheckpoint = FCFFittingRuntimeCheckpoint();
		return false;
	}

	return true;
}

// [v1.1.0] 준비된 후보 Weapon·Defense Runtime과 필요한 경우 후보 질량을 하나의 transaction으로 Commit합니다.
bool FCFFieldFitRuntimeAdapter::CommitRuntime(FString& OutFailureSummary)
{
	OutFailureSummary.Reset();
	bLastCommitFailureRecovered = true;
	if (!FittingComponent || !RuntimeApplyAdapter || !PreviousRuntimeCheckpoint.IsValid())
	{
		OutFailureSummary = TEXT("FieldFitRuntime: CommitFailed, RuntimeTransactionNotPrepared");
		return false;
	}

	if (!FittingComponent->CommitPreparedSortieFitting(*RuntimeApplyAdapter))
	{
		bLastCommitFailureRecovered = FittingComponent->WasLastCommitFailureRecovered();
		OutFailureSummary = FittingComponent->GetLastFittingRuntimeSummary();
		return false;
	}

	bCandidateRuntimeCommitted = true;
	if (!bRequiresMassReapply)
	{
		return true;
	}

	// [v1.1.0] 후보 Weapon·Defense Commit 뒤 같은 transaction에서 적용할 새 차량 질량 실패 요약입니다.
	FString MassFailureSummary;
	if (!MassRuntimeAdapter || !MassRuntimeAdapter->ReapplyVehicleMassKg(CandidateMassKg, MassFailureSummary))
	{
		// [v1.2.0] 후보 Runtime이 이미 적용됐으므로 이전 Weapon·Defense Runtime으로 즉시 복원합니다.
		const bool bRuntimeRestored = FittingComponent->RestoreAppliedRuntimeCheckpoint(*RuntimeApplyAdapter, PreviousRuntimeCheckpoint);
		// [v1.2.0] Mass Apply가 부분 변경됐을 가능성까지 제거하기 위해 직전 질량을 재적용합니다.
		FString MassRestoreFailureSummary;
		const bool bMassRestored = MassRuntimeAdapter
			&& MassRuntimeAdapter->ReapplyVehicleMassKg(PreviousAppliedMassKg, MassRestoreFailureSummary);

		bCandidateRuntimeCommitted = false;
		bCandidateMassApplied = false;
		bLastCommitFailureRecovered = bRuntimeRestored && bMassRestored;
		OutFailureSummary = FString::Printf(
			TEXT("FieldFitRuntime: CommitFailed, MassReapplyFailed, Apply=%s, RuntimeRestore=%s, MassRestore=%s%s%s"),
			*MassFailureSummary,
			bRuntimeRestored ? TEXT("Succeeded") : TEXT("Failed"),
			bMassRestored ? TEXT("Succeeded") : TEXT("Failed"),
			MassRestoreFailureSummary.IsEmpty() ? TEXT("") : TEXT(", RestoreDetail="),
			*MassRestoreFailureSummary);
		return false;
	}

	bCandidateMassApplied = true;
	return true;
}

// [v1.0.0] Commit 전 실패에서 FittingComp의 Prepared 후보만 Rollback합니다.
void FCFFieldFitRuntimeAdapter::RollbackPreparedRuntime()
{
	if (FittingComponent && FittingComponent->HasPreparedRuntimeInput())
	{
		FittingComponent->RollbackPreparedSortieFitting();
	}

		PreviousRuntimeCheckpoint = FCFFittingRuntimeCheckpoint();
	bCandidateRuntimeCommitted = false;
	bRequiresMassReapply = false;
	bCandidateMassApplied = false;
	PreviousAppliedMassKg = 0.0f;
	CandidateMassKg = 0.0f;
	bLastCommitFailureRecovered = true;
}

// [v1.1.0] Inventory Commit 실패 뒤 직전 Applied Runtime과 필요한 경우 직전 질량을 함께 복원합니다.
bool FCFFieldFitRuntimeAdapter::CompensateCommittedRuntime(FString& OutFailureSummary)
{
	OutFailureSummary.Reset();
	if (!FittingComponent || !RuntimeApplyAdapter || !bCandidateRuntimeCommitted || !PreviousRuntimeCheckpoint.IsValid())
	{
		OutFailureSummary = TEXT("FieldFitRuntime: CompensationFailed, RuntimeCheckpointUnavailable");
		return false;
	}

	// [v1.1.0] 후보 Weapon·Defense Runtime을 직전 Applied Checkpoint로 복원한 결과입니다.
	const bool bRuntimeRestored = FittingComponent->RestoreAppliedRuntimeCheckpoint(*RuntimeApplyAdapter, PreviousRuntimeCheckpoint);
	// [v1.1.0] 후보 질량이 실제 적용됐던 경우 직전 총질량으로 되돌린 결과입니다.
	bool bMassRestored = true;
	FString MassRestoreFailureSummary;
	if (bRequiresMassReapply && bCandidateMassApplied)
	{
		bMassRestored = MassRuntimeAdapter
			&& MassRuntimeAdapter->ReapplyVehicleMassKg(PreviousAppliedMassKg, MassRestoreFailureSummary);
	}

	if (!bRuntimeRestored || !bMassRestored)
	{
		OutFailureSummary = FString::Printf(
			TEXT("FieldFitRuntime: CompensationFailed, RuntimeRestore=%s, MassRestore=%s%s%s"),
			bRuntimeRestored ? TEXT("Succeeded") : TEXT("Failed"),
			bMassRestored ? TEXT("Succeeded") : TEXT("Failed"),
			MassRestoreFailureSummary.IsEmpty() ? TEXT("") : TEXT(", MassDetail="),
			*MassRestoreFailureSummary);
		return false;
	}

	bCandidateRuntimeCommitted = false;
	bCandidateMassApplied = false;
	return true;
}

// [v1.0.0] Inventory Prepare → Runtime Prepare·Commit → Inventory Commit을 수행하고 실패 시 필요한 양쪽 보상 Rollback을 실행합니다.
FCFFieldFitCompletionResult FCFFieldFitCoordinator::Complete(
	TArray<FCFInventoryContainerState>& InOutContainers,
	const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
	FCFInventoryTransferLedger& TransferLedger,
	ICFFieldFitRuntimeTransaction& RuntimeTransaction,
	const FCFFieldFitCompletionRequest& CompletionRequest)
{
	// [v1.0.0] 모든 단계와 보상 상태를 호출자에게 반환할 명시적 Operation Result입니다.
	FCFFieldFitCompletionResult CompletionResult;
	CompletionResult.TransactionId = CompletionRequest.InventoryTransferRequest.TransactionId;
	if (!CompletionRequest.IsValid())
	{
		CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::InvalidRequest;
		CompletionResult.FailureSummary = TEXT("FieldFitCoordinator: InvalidCompletionRequest");
		return CompletionResult;
	}

	// [v1.0.0] Container 무변경 Reservation 생성 또는 기존 Prepared 상태 재사용 결과입니다.
	const FCFInventoryTransferResult InventoryPrepareResult = TransferLedger.PrepareTransfer(
		InOutContainers,
		ContainerMassSnapshots,
		CompletionRequest.InventoryTransferRequest);
	CompletionResult.InventoryPrepareStatus = InventoryPrepareResult.TransferStatus;
	if (InventoryPrepareResult.TransferStatus != ECFInventoryTransferStatus::Prepared
		&& InventoryPrepareResult.TransferStatus != ECFInventoryTransferStatus::AlreadyPrepared)
	{
		CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::InventoryPrepareFailed;
		CompletionResult.FailureSummary = FString::Printf(
			TEXT("FieldFitCoordinator: InventoryPrepareFailed, Status=%d"),
			static_cast<int32>(InventoryPrepareResult.TransferStatus));
		return CompletionResult;
	}

	// [v1.0.0] Runtime Adapter가 후보를 아직 적용하지 않고 검증·Checkpoint까지 준비한 실패 요약입니다.
	FString RuntimeFailureSummary;
	if (!RuntimeTransaction.PrepareRuntime(
		CompletionRequest.CandidateFittingSnapshot,
		CompletionRequest.RequestedActiveMountProfileId,
		RuntimeFailureSummary))
	{
		RuntimeTransaction.RollbackPreparedRuntime();
		// [v1.0.0] Runtime Prepare 실패 뒤 Active Reservation을 원상복구할 Inventory Rollback 결과입니다.
		const FCFInventoryTransferResult InventoryRollbackResult = TransferLedger.RollbackTransfer(CompletionRequest.InventoryTransferRequest.TransactionId);
		CompletionResult.InventoryRollbackStatus = InventoryRollbackResult.TransferStatus;
		if (!InventoryRollbackResult.IsSuccessful())
		{
			CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::RecoveryFailed;
			CompletionResult.FailureSummary = FString::Printf(
				TEXT("FieldFitCoordinator: RuntimePrepareFailedAndInventoryRollbackFailed, Runtime=%s, InventoryStatus=%d"),
				*RuntimeFailureSummary,
				static_cast<int32>(InventoryRollbackResult.TransferStatus));
			return CompletionResult;
		}

		CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::RuntimePrepareFailed;
		CompletionResult.FailureSummary = RuntimeFailureSummary;
		return CompletionResult;
	}

		if (!RuntimeTransaction.CommitRuntime(RuntimeFailureSummary))
	{
		// [v1.2.0] Runtime Commit 실패가 이전 상태 복구까지 성공했는지 Runtime Transaction이 명시적으로 보고합니다.
		const bool bRuntimeCommitRecovered = RuntimeTransaction.WasLastCommitFailureRecovered();
		RuntimeTransaction.RollbackPreparedRuntime();
		// [v1.0.0] Runtime Commit 실패 뒤 Prepared Inventory Transaction을 원상복구할 결과입니다.
		const FCFInventoryTransferResult InventoryRollbackResult = TransferLedger.RollbackTransfer(CompletionRequest.InventoryTransferRequest.TransactionId);
		CompletionResult.InventoryRollbackStatus = InventoryRollbackResult.TransferStatus;
		if (!bRuntimeCommitRecovered || !InventoryRollbackResult.IsSuccessful())
		{
			CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::RecoveryFailed;
			CompletionResult.FailureSummary = FString::Printf(
				TEXT("FieldFitCoordinator: RuntimeCommitRecoveryFailed, RuntimeRecovered=%s, Runtime=%s, InventoryRollback=%d"),
				bRuntimeCommitRecovered ? TEXT("Yes") : TEXT("No"),
				*RuntimeFailureSummary,
				static_cast<int32>(InventoryRollbackResult.TransferStatus));
			return CompletionResult;
		}

		CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::RuntimeCommitFailed;
		CompletionResult.FailureSummary = RuntimeFailureSummary;
		return CompletionResult;
	}

	CompletionResult.bRuntimeCommitted = true;

	// [v1.0.0] Runtime 후보가 성공한 뒤에만 실제 Container 소유권을 한 번 교체할 Inventory Commit 결과입니다.
	const FCFInventoryTransferResult InventoryCommitResult = TransferLedger.CommitTransfer(
		InOutContainers,
		ContainerMassSnapshots,
		CompletionRequest.InventoryTransferRequest.TransactionId);
	CompletionResult.InventoryCommitStatus = InventoryCommitResult.TransferStatus;
	if (InventoryCommitResult.TransferStatus == ECFInventoryTransferStatus::Committed)
	{
		CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::Completed;
		return CompletionResult;
	}

	// [v1.0.0] Inventory Commit 실패로 이미 적용된 후보 Runtime을 직전 Applied 상태로 되돌린 결과입니다.
	FString RuntimeCompensationFailureSummary;
	CompletionResult.bRuntimeCompensated = RuntimeTransaction.CompensateCommittedRuntime(RuntimeCompensationFailureSummary);
	// [v1.0.0] Commit 실패 뒤 여전히 Prepared인 Inventory Transaction과 Reservation을 해제할 보상 결과입니다.
	const FCFInventoryTransferResult InventoryRollbackResult = TransferLedger.RollbackTransfer(CompletionRequest.InventoryTransferRequest.TransactionId);
	CompletionResult.InventoryRollbackStatus = InventoryRollbackResult.TransferStatus;
	if (!CompletionResult.bRuntimeCompensated || !InventoryRollbackResult.IsSuccessful())
	{
		CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::RecoveryFailed;
		CompletionResult.FailureSummary = FString::Printf(
			TEXT("FieldFitCoordinator: InventoryCommitFailedRecoveryFailed, CommitStatus=%d, RuntimeCompensation=%s, Runtime=%s, InventoryRollback=%d"),
			static_cast<int32>(InventoryCommitResult.TransferStatus),
			CompletionResult.bRuntimeCompensated ? TEXT("Succeeded") : TEXT("Failed"),
			*RuntimeCompensationFailureSummary,
			static_cast<int32>(InventoryRollbackResult.TransferStatus));
		return CompletionResult;
	}

	CompletionResult.CompletionStatus = ECFFieldFitCompletionStatus::InventoryCommitFailedRolledBack;
	CompletionResult.FailureSummary = FString::Printf(
		TEXT("FieldFitCoordinator: InventoryCommitFailedRolledBack, Status=%d"),
		static_cast<int32>(InventoryCommitResult.TransferStatus));
	return CompletionResult;
}
