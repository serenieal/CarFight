// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-15
// Description: CF-FQ-035 / CF-FQ-034 Field Fitting completion 원자 조율·Chaos Field Mass Reapply 계약
// Scope: 단일 Equip·Unequip의 Inventory, Weapon·Defense Runtime과 선택적 Vehicle Mass Runtime을 하나의 completion transaction으로 조율합니다.
// Changelog:
// - v1.3.0: 실제 AWheeledVehiclePawn의 Chaos Movement Mass를 Physics State 재생성으로 적용·검증하는 FCFChaosVehicleMassRuntime을 추가.
// - v1.2.0: Commit 실패 뒤 이전 상태 복구 성공 여부 계약을 추가해 Coordinator가 RuntimeCommitFailed와 RecoveryFailed를 구분.
// - v1.1.0: 선택적 Field Mass Runtime capability와 mass-changing 후보의 Commit·Compensation 경계를 추가.
// - v1.0.0: Completion Request/Result, Runtime Transaction Adapter, same-mass FittingComp Adapter와 Coordinator를 최초 추가.
// Migration:
// - 비전투·정지·쿨타임·시간 액션은 FFIT-P0-01~02가 소유하며 이 Coordinator가 재계산하지 않습니다.
// - Replace는 기존 계약대로 Unequip 완료 후 별도 Equip을 수행하며 한 번의 Coordinator 호출은 단일 Inventory Transfer만 다룹니다.
// - Inventory Commit보다 Runtime Commit을 먼저 수행하고 Inventory Commit 실패 시 직전 Applied Runtime Checkpoint와 Prepared Inventory Transaction을 모두 보상 Rollback합니다.
// - Mass Runtime Adapter가 없으면 기존 안전 경계대로 직전 Applied Snapshot과 후보 Snapshot 총질량이 0.01kg 이내인 경우만 허용합니다.
// - Mass Runtime Adapter가 있으면 질량 변경도 같은 Runtime transaction에 포함하고 실패·Inventory Commit 실패 시 직전 질량까지 보상 복원합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFInventoryTransfer.h"
#include "CFVehicleFittingComp.h"
#include "CFFieldFitCoordinator.generated.h"

class ACFVehiclePawn;


/** Field Fitting completion transaction의 최종 결과 상태입니다. */
UENUM(BlueprintType)
enum class ECFFieldFitCompletionStatus : uint8
{
	None UMETA(DisplayName="결과 없음 (None)"),
	Completed UMETA(DisplayName="완료 (Completed)"),
	InvalidRequest UMETA(DisplayName="요청 무효 (Invalid Request)"),
	InventoryPrepareFailed UMETA(DisplayName="Inventory Prepare 실패 (Inventory Prepare Failed)"),
	RuntimePrepareFailed UMETA(DisplayName="Runtime Prepare 실패 (Runtime Prepare Failed)"),
	RuntimeCommitFailed UMETA(DisplayName="Runtime Commit 실패 (Runtime Commit Failed)"),
	InventoryCommitFailedRolledBack UMETA(DisplayName="Inventory Commit 실패·보상 완료 (Inventory Commit Failed Rolled Back)"),
	RecoveryFailed UMETA(DisplayName="보상 복구 실패 (Recovery Failed)")
};

/** 완료 조건이 충족된 단일 Equip·Unequip을 원자 적용하기 위한 Coordinator 입력입니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitCompletionRequest
{
	GENERATED_BODY()

	// [v1.0.0] Inventory Transfer와 후보 Fitting Snapshot의 최소 completion 계약이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] Prepare부터 Commit·Rollback까지 같은 실제 Item 이동을 식별하는 Inventory 요청입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="Inventory Transfer 요청 (InventoryTransferRequest)", ToolTip="Field Equip 또는 Unequip 완료 순간 원자 적용할 단일 Inventory Transfer 요청입니다."))
	FCFInventoryTransferRequest InventoryTransferRequest;

	// [v1.0.0] Inventory Item Binding과 기존 Fitting 검증을 통과한 최종 후보 Snapshot입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="후보 피팅 Snapshot (CandidateFittingSnapshot)", ToolTip="Inventory Adapter와 BuildFittingSnapshot 검증을 통과한 완료 후보입니다. Coordinator는 Mount 호환성과 질량을 재계산하지 않습니다."))
	FCFVehicleFittingSnapshot CandidateFittingSnapshot;

	// [v1.0.0] 후보 Runtime에서 VehicleWeaponComp가 활성 장착으로 사용할 MountProfile ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="요청 활성 Mount Profile ID (RequestedActiveMountProfileId)", ToolTip="후보 Snapshot을 Weapon Runtime 입력으로 변환할 때 활성 장착으로 사용할 MountProfileId입니다."))
	FName RequestedActiveMountProfileId = NAME_None;
};

/** Field Fitting completion 결과와 보상 상태를 UI·Action 계층에 전달하는 명시적 Operation Result입니다. */
USTRUCT(BlueprintType)
struct FCFFieldFitCompletionResult
{
	GENERATED_BODY()

	// [v1.0.0] Runtime과 Inventory가 모두 새 상태로 Commit된 완전 성공인지 반환합니다.
	bool IsSuccessful() const { return CompletionStatus == ECFFieldFitCompletionStatus::Completed; }

	// [v1.0.0] Field Fitting completion의 최종 원자 결과입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="완료 상태 (CompletionStatus)", ToolTip="정상 완료 또는 Inventory/Runtime 단계와 보상 복구 실패 지점을 나타냅니다."))
	ECFFieldFitCompletionStatus CompletionStatus = ECFFieldFitCompletionStatus::None;

	// [v1.0.0] 결과와 연결된 Inventory Transaction ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="Inventory Transaction ID (TransactionId)", ToolTip="이번 completion의 Prepare·Commit·Rollback을 연결하는 Inventory Transaction ID입니다."))
	FCFInventoryTransactionId TransactionId;

	// [v1.0.0] 첫 Inventory Prepare 단계의 실제 Ledger 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="Inventory Prepare 상태 (InventoryPrepareStatus)", ToolTip="Prepared 또는 구체적인 Prepare 거부 상태를 그대로 보존합니다."))
	ECFInventoryTransferStatus InventoryPrepareStatus = ECFInventoryTransferStatus::None;

	// [v1.0.0] Runtime 성공 뒤 실행한 Inventory Commit의 실제 Ledger 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="Inventory Commit 상태 (InventoryCommitStatus)", ToolTip="Committed 또는 Runtime 보상 Rollback을 유발한 Commit 실패 상태를 보존합니다."))
	ECFInventoryTransferStatus InventoryCommitStatus = ECFInventoryTransferStatus::None;

	// [v1.0.0] 실패 보상에서 실행한 Inventory Rollback의 실제 Ledger 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="Inventory Rollback 상태 (InventoryRollbackStatus)", ToolTip="Prepared Transaction 보상 Rollback의 실제 결과입니다. 보상이 필요 없으면 None입니다."))
	ECFInventoryTransferStatus InventoryRollbackStatus = ECFInventoryTransferStatus::None;

	// [v1.0.0] 후보 Runtime이 실제 Commit 단계까지 성공했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="Runtime Commit 성공 (bRuntimeCommitted)", ToolTip="Inventory Commit 직전 후보 Weapon·Defense Runtime Commit이 성공했으면 True입니다."))
	bool bRuntimeCommitted = false;

	// [v1.0.0] Inventory Commit 실패 뒤 직전 Applied Runtime으로 보상 복원됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="Runtime 보상 복원됨 (bRuntimeCompensated)", ToolTip="후보 Runtime Commit 뒤 Inventory Commit이 실패해 직전 Applied Runtime으로 정상 복원됐으면 True입니다."))
	bool bRuntimeCompensated = false;

	// [v1.0.0] 상위 Action/UI가 진단에 사용할 bounded C++ 실패 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Field|Completion", meta=(DisplayName="실패 요약 (FailureSummary)", ToolTip="Coordinator가 실패한 단계와 보상 결과를 설명하는 진단 문자열입니다. 게임 규칙 판정에는 CompletionStatus를 사용합니다."))
	FString FailureSummary;
};

/** Coordinator가 Inventory와 독립적으로 후보 Runtime을 Prepare·Commit·보상하는 데 사용하는 C++ 계약입니다. */
class CARFIGHT_RE_API ICFFieldFitRuntimeTransaction
{
public:
	// [v1.0.0] 파생 Runtime Transaction Adapter를 안전하게 파괴하기 위한 가상 소멸자입니다.
	virtual ~ICFFieldFitRuntimeTransaction() = default;

	// [v1.0.0] 후보 Snapshot과 활성 Mount를 아직 Runtime에 적용하지 않고 준비합니다.
	virtual bool PrepareRuntime(const FCFVehicleFittingSnapshot& CandidateFittingSnapshot, FName RequestedActiveMountProfileId, FString& OutFailureSummary) = 0;

		// [v1.0.0] 준비된 후보 Runtime을 원자 Commit하고 실패 시 직전 Runtime 복구를 시도합니다.
	virtual bool CommitRuntime(FString& OutFailureSummary) = 0;

	// [v1.2.0] 마지막 Commit 실패가 이전 Runtime 상태를 실제 보존·복구했는지 반환합니다. 기존 구현은 Commit 실패 시 이전 상태 보존 계약을 기본값으로 유지합니다.
	virtual bool WasLastCommitFailureRecovered() const { return true; }

	// [v1.0.0] Runtime Commit 전 실패에서 남은 Prepared 후보 상태만 정리합니다.
	virtual void RollbackPreparedRuntime() = 0;

	// [v1.0.0] Runtime Commit 성공 뒤 Inventory Commit 실패 시 직전 Applied Runtime으로 보상 복원합니다.
	virtual bool CompensateCommittedRuntime(FString& OutFailureSummary) = 0;
};

/** Field Fitting transaction이 실제 차량 질량을 적용·복원할 때 사용하는 선택적 C++ 계약입니다. */
class CARFIGHT_RE_API ICFFieldFitMassRuntime
{
public:
	// [v1.1.0] 파생 Mass Runtime Adapter를 안전하게 파괴하기 위한 가상 소멸자입니다.
	virtual ~ICFFieldFitMassRuntime() = default;

	// [v1.1.0] Physics Runtime에 목표 차량 총질량을 적용하거나 직전 질량을 복원합니다.
	virtual bool ReapplyVehicleMassKg(float TargetMassKg, FString& OutFailureSummary) = 0;
};

/** 실제 CarFight Wheeled Vehicle의 Chaos Movement Mass를 Field Runtime에서 재적용·검증하는 Adapter입니다. */
class CARFIGHT_RE_API FCFChaosVehicleMassRuntime final : public ICFFieldFitMassRuntime
{
public:
	// [v1.3.0] 실제 Field Fitting 대상 차량 Pawn을 Mass Runtime에 연결합니다.
	explicit FCFChaosVehicleMassRuntime(ACFVehiclePawn* InVehiclePawn);

	// [v1.3.0] Movement Mass를 갱신하고 Chaos Vehicle Physics State를 재생성한 뒤 Configured·Actual 질량과 차량 상태 보존을 검증합니다.
	virtual bool ReapplyVehicleMassKg(float TargetMassKg, FString& OutFailureSummary) override;

private:
	// [v1.3.0] Mesh와 Chaos VehicleMovement를 제공하는 실제 Field Fitting 대상 차량입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;
};

/** 현재 UCFVehicleFittingComp를 Coordinator Runtime Transaction 계약에 연결하는 C++ Adapter입니다. */
class CARFIGHT_RE_API FCFFieldFitRuntimeAdapter final : public ICFFieldFitRuntimeTransaction
{
public:
		// [v1.0.0] 실제 FittingComp와 Weapon·Defense Runtime Apply Adapter를 연결합니다. Mass Adapter가 없으므로 same-mass 후보만 허용합니다.
	FCFFieldFitRuntimeAdapter(UCFVehicleFittingComp* InFittingComponent, ICFFittingRuntimeApplyAdapter& InRuntimeApplyAdapter);

	// [v1.1.0] Weapon·Defense Runtime과 Field Mass Runtime을 하나의 transaction에 연결합니다.
	FCFFieldFitRuntimeAdapter(UCFVehicleFittingComp* InFittingComponent, ICFFittingRuntimeApplyAdapter& InRuntimeApplyAdapter, ICFFieldFitMassRuntime& InMassRuntimeAdapter);

	// [v1.0.0] 직전 Applied Snapshot과 같은 질량의 후보만 Checkpoint와 함께 준비합니다.
	virtual bool PrepareRuntime(const FCFVehicleFittingSnapshot& CandidateFittingSnapshot, FName RequestedActiveMountProfileId, FString& OutFailureSummary) override;

		// [v1.1.0] 준비된 후보 Weapon·Defense Runtime과 필요한 경우 후보 질량을 하나의 transaction으로 Commit합니다.
	virtual bool CommitRuntime(FString& OutFailureSummary) override;

	// [v1.2.0] 마지막 Commit 실패에서 Weapon·Defense와 질량이 모두 이전 상태로 복구됐는지 반환합니다.
	virtual bool WasLastCommitFailureRecovered() const override { return bLastCommitFailureRecovered; }

	// [v1.0.0] Commit 전 실패에서 FittingComp의 Prepared 후보만 Rollback합니다.
	virtual void RollbackPreparedRuntime() override;

	// [v1.0.0] Inventory Commit 실패 뒤 캡처한 직전 Applied Runtime Checkpoint를 복원합니다.
	virtual bool CompensateCommittedRuntime(FString& OutFailureSummary) override;

private:
	// [v1.0.0] 실제 Applied/Prepared Fitting Runtime 상태를 소유하는 차량 컴포넌트입니다.
	UCFVehicleFittingComp* FittingComponent = nullptr;

		// [v1.0.0] Weapon·Defense Runtime을 실제 또는 Fake 계층에 적용할 하위 Adapter입니다.
	ICFFittingRuntimeApplyAdapter* RuntimeApplyAdapter = nullptr;

	// [v1.1.0] 질량 변경 후보에서 Vehicle Mass 재적용·복원을 수행할 선택적 Adapter입니다.
	ICFFieldFitMassRuntime* MassRuntimeAdapter = nullptr;

	// [v1.0.0] 후보 Commit 전에 캡처한 직전 Applied Weapon·Defense Runtime Checkpoint입니다.
	FCFFittingRuntimeCheckpoint PreviousRuntimeCheckpoint;

		// [v1.0.0] 이번 Adapter 수명에서 후보 Runtime Commit이 실제 성공했는지 여부입니다.
	bool bCandidateRuntimeCommitted = false;

	// [v1.1.0] 후보 Snapshot이 직전 Applied Snapshot과 다른 총질량을 요구하는지 여부입니다.
	bool bRequiresMassReapply = false;

	// [v1.1.0] 후보 질량이 실제 Vehicle Runtime에 적용 완료됐는지 여부입니다.
	bool bCandidateMassApplied = false;

	// [v1.1.0] 실패 보상에서 되돌릴 직전 Applied Snapshot의 총질량입니다.
	float PreviousAppliedMassKg = 0.0f;

	// [v1.1.0] 현재 후보 Snapshot이 요구하는 새 총질량입니다.
	float CandidateMassKg = 0.0f;

	// [v1.2.0] 마지막 Commit 실패 뒤 Weapon·Defense와 질량이 모두 이전 상태로 복구됐는지 여부입니다.
	bool bLastCommitFailureRecovered = true;
};

/** 이미 완료 조건에 도달한 단일 Field Equip·Unequip의 Inventory와 Runtime Commit을 원자적으로 조율합니다. */
struct CARFIGHT_RE_API FCFFieldFitCoordinator
{
	// [v1.0.0] Inventory Prepare → Runtime Prepare·Commit → Inventory Commit을 수행하고 실패 시 필요한 양쪽 보상 Rollback을 실행합니다.
	static FCFFieldFitCompletionResult Complete(
		TArray<FCFInventoryContainerState>& InOutContainers,
		const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
		FCFInventoryTransferLedger& TransferLedger,
		ICFFieldFitRuntimeTransaction& RuntimeTransaction,
		const FCFFieldFitCompletionRequest& CompletionRequest);
};
