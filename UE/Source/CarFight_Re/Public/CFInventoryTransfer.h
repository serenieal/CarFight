// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-03 Reservation과 Atomic Transfer 공용 계약
// Scope: Item·Destination Capacity 예약, 멱등 취소, Prepare·Commit·Rollback과 중복 Commit 방지를 Pawn 없이 제공합니다.
// Changelog:
// - v1.0.0: ReservationId, TransactionId, Reservation·Transaction 상태와 원자 Transfer Ledger를 최초 추가.
// Migration:
// - INV-P0-01 Item Identity와 INV-P0-02 Container·Capacity·Access Query 타입은 변경하지 않는다.
// - Container 배열은 Prepare에서 변경하지 않고 Commit의 후보 복사본 검증이 성공한 경우에만 한 번 교체한다.
// - 질량은 Domain DataAsset에서 호출자가 해석한 Container Mass Snapshot과 Item 질량만 입력받는다.
// - Fitting Runtime, UI, SaveGame, 네트워크, 만료 Timer와 Unreal Asset은 이 타입이 소유하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFInventoryContainer.h"
#include "CFInventoryTransfer.generated.h"

/**
 * Inventory Reservation이 잠그는 자원의 종류입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryReservationKind : uint8
{
	Unknown UMETA(DisplayName="알 수 없음 (Unknown)"),
	Item UMETA(DisplayName="아이템 (Item)"),
	DestinationCapacity UMETA(DisplayName="목적지 용량 (Destination Capacity)")
};

/**
 * Inventory Reservation의 수명 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryReservationState : uint8
{
	Unknown UMETA(DisplayName="알 수 없음 (Unknown)"),
	Active UMETA(DisplayName="활성 (Active)"),
	Cancelled UMETA(DisplayName="취소됨 (Cancelled)"),
	Consumed UMETA(DisplayName="Commit 소비됨 (Consumed)")
};

/**
 * Inventory Transaction의 수명 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryTransactionState : uint8
{
	Unknown UMETA(DisplayName="알 수 없음 (Unknown)"),
	Prepared UMETA(DisplayName="준비됨 (Prepared)"),
	Committed UMETA(DisplayName="Commit 완료 (Committed)"),
	RolledBack UMETA(DisplayName="Rollback 완료 (Rolled Back)")
};

/**
 * Prepare·Commit·Rollback 요청의 결정론적 결과 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryTransferStatus : uint8
{
	None UMETA(DisplayName="결과 없음 (None)"),
	Prepared UMETA(DisplayName="Prepare 완료 (Prepared)"),
	AlreadyPrepared UMETA(DisplayName="이미 Prepare됨 (Already Prepared)"),
	Committed UMETA(DisplayName="Commit 완료 (Committed)"),
	AlreadyCommitted UMETA(DisplayName="이미 Commit됨 (Already Committed)"),
	RolledBack UMETA(DisplayName="Rollback 완료 (Rolled Back)"),
	AlreadyRolledBack UMETA(DisplayName="이미 Rollback됨 (Already Rolled Back)"),
	InvalidRequest UMETA(DisplayName="요청 무효 (Invalid Request)"),
	InvalidContainerSet UMETA(DisplayName="Container 집합 무효 (Invalid Container Set)"),
	TransactionIdConflict UMETA(DisplayName="Transaction ID 충돌 (Transaction ID Conflict)"),
	TransactionMissing UMETA(DisplayName="Transaction 없음 (Transaction Missing)"),
	SourceItemMissing UMETA(DisplayName="Source Item 없음 (Source Item Missing)"),
	SourceLocationChanged UMETA(DisplayName="Source 위치 변경됨 (Source Location Changed)"),
	DestinationContainerMissing UMETA(DisplayName="Destination Container 없음 (Destination Container Missing)"),
	DestinationSlotOccupied UMETA(DisplayName="Destination Slot 사용 중 (Destination Slot Occupied)"),
	ItemAlreadyReserved UMETA(DisplayName="Item 이미 예약됨 (Item Already Reserved)"),
	DestinationSlotReserved UMETA(DisplayName="Destination Slot 이미 예약됨 (Destination Slot Reserved)"),
	DestinationCapacityExceeded UMETA(DisplayName="Destination Capacity 초과 (Destination Capacity Exceeded)"),
	MissingMassSnapshot UMETA(DisplayName="질량 Snapshot 없음 (Missing Mass Snapshot)"),
	ReservationUnavailable UMETA(DisplayName="Reservation 사용 불가 (Reservation Unavailable)"),
	ContainerStateChanged UMETA(DisplayName="Container 상태 변경됨 (Container State Changed)"),
	CannotRollbackCommitted UMETA(DisplayName="Commit 완료 Transaction Rollback 불가 (Cannot Rollback Committed)"),
	CommitValidationFailed UMETA(DisplayName="Commit 후보 검증 실패 (Commit Validation Failed)")
};

/**
 * Reservation 취소 요청의 멱등 결과 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryCancelState : uint8
{
	None UMETA(DisplayName="결과 없음 (None)"),
	Cancelled UMETA(DisplayName="취소 완료 (Cancelled)"),
	AlreadyCancelled UMETA(DisplayName="이미 취소됨 (Already Cancelled)"),
	CannotCancelConsumed UMETA(DisplayName="Commit 소비 Reservation 취소 불가 (Cannot Cancel Consumed)"),
	ReservationMissing UMETA(DisplayName="Reservation 없음 (Reservation Missing)"),
	InvalidReservationId UMETA(DisplayName="Reservation ID 무효 (Invalid Reservation ID)")
};

/**
 * Inventory Reservation 한 건을 안정적으로 식별하는 직렬화 가능한 ID입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryReservationId
{
	GENERATED_BODY()

	// [v1.0.0] 새로운 고유 Reservation ID를 생성합니다.
	static FCFInventoryReservationId CreateNew();

	// [v1.0.0] 기존 Guid를 보존하는 Reservation ID를 생성합니다.
	static FCFInventoryReservationId FromGuid(const FGuid& InGuid);

	// [v1.0.0] 유효한 Reservation ID인지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 두 Reservation ID가 같은 예약을 가리키는지 비교합니다.
	bool operator==(const FCFInventoryReservationId& Other) const;

	// [v1.0.0] 두 Reservation ID가 다른 예약을 가리키는지 비교합니다.
	bool operator!=(const FCFInventoryReservationId& Other) const;

	// [v1.0.0] TSet·TMap에서 사용할 Reservation ID 해시 값을 반환합니다.
	friend uint32 GetTypeHash(const FCFInventoryReservationId& ReservationId)
	{
		return GetTypeHash(ReservationId.Value);
	}

	// [v1.0.0] Reservation 한 건을 식별하는 Guid입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation|Identity", meta=(DisplayName="Inventory Reservation Guid (Value)", ToolTip="Item 또는 Destination Capacity Reservation 한 건을 취소·조회하는 안정적인 Guid입니다."))
	FGuid Value;
};

/**
 * Inventory Transaction 한 건을 안정적으로 식별하는 직렬화 가능한 ID입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryTransactionId
{
	GENERATED_BODY()

	// [v1.0.0] 새로운 고유 Transaction ID를 생성합니다.
	static FCFInventoryTransactionId CreateNew();

	// [v1.0.0] 기존 Guid를 보존하는 Transaction ID를 생성합니다.
	static FCFInventoryTransactionId FromGuid(const FGuid& InGuid);

	// [v1.0.0] 유효한 Transaction ID인지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 두 Transaction ID가 같은 Transaction을 가리키는지 비교합니다.
	bool operator==(const FCFInventoryTransactionId& Other) const;

	// [v1.0.0] 두 Transaction ID가 다른 Transaction을 가리키는지 비교합니다.
	bool operator!=(const FCFInventoryTransactionId& Other) const;

	// [v1.0.0] TSet·TMap에서 사용할 Transaction ID 해시 값을 반환합니다.
	friend uint32 GetTypeHash(const FCFInventoryTransactionId& TransactionId)
	{
		return GetTypeHash(TransactionId.Value);
	}

	// [v1.0.0] Prepare·Commit·Rollback Transaction 한 건을 식별하는 Guid입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer|Identity", meta=(DisplayName="Inventory Transaction Guid (Value)", ToolTip="같은 Transfer의 Prepare, Commit과 Rollback을 연결하고 중복 Commit을 방지하는 안정적인 Guid입니다."))
	FGuid Value;
};

/**
 * Container 질량 한도 검증에만 사용하는 외부 해석 현재 질량 Snapshot입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryContainerMassSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] Container ID와 외부 해석 질량이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 이 질량 Snapshot이 설명하는 Container ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer|Capacity", meta=(DisplayName="Inventory Container ID (ContainerId)", ToolTip="OccupiedMassKg가 현재 설명하는 Inventory Container ID입니다."))
	FCFInventoryContainerId ContainerId;

	// [v1.0.0] Domain DataAsset에서 호출자가 해석한 현재 Container 총질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer|Capacity", meta=(ClampMin="0.0", Units="kg", DisplayName="현재 해석 질량 kg (OccupiedMassKg)", ToolTip="Inventory가 저장하거나 재계산하지 않는 현재 Container의 외부 해석 총질량입니다."))
	float OccupiedMassKg = 0.0f;
};

/**
 * Item 한 개를 Source에서 Destination으로 이동하기 위한 Prepare 입력입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryTransferRequest
{
	GENERATED_BODY()

	// [v1.0.0] Transaction, Action Owner, Item, Source·Destination과 질량 입력이 모두 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 같은 Transfer의 중복 Prepare·Commit을 식별하는 Transaction ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Inventory Transaction ID (TransactionId)", ToolTip="Prepare부터 Commit 또는 Rollback까지 같은 Transfer를 식별하는 ID입니다."))
	FCFInventoryTransactionId TransactionId;

	// [v1.0.0] Reservation을 소유하는 시간 액션 또는 Coordinator의 불투명 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Action 소유자 ID (ActionOwnerId)", ToolTip="Reservation을 생성한 시간 액션 또는 상위 Coordinator를 식별합니다. Inventory는 이 값의 게임 규칙을 해석하지 않습니다."))
	FName ActionOwnerId = NAME_None;

	// [v1.0.0] 이동하고 예약할 실제 Item Instance ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="아이템 인스턴스 ID (ItemInstanceId)", ToolTip="Source에서 정확히 한 위치에 존재해야 하며 Prepared 동안 다른 Transaction이 예약할 수 없는 Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] Prepare 시 Item이 존재해야 하는 Source Container ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Source Container ID (SourceContainerId)", ToolTip="Prepare와 Commit에서 Item의 원래 소유 위치를 검증할 Source Container ID입니다."))
	FCFInventoryContainerId SourceContainerId;

	// [v1.0.0] Prepare 시 Item이 존재해야 하는 Source Slot ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Source Slot ID (SourceSlotId)", ToolTip="Prepare와 Commit에서 Item의 원래 슬롯 위치를 검증하는 안정적인 Slot ID입니다."))
	FName SourceSlotId = NAME_None;

	// [v1.0.0] Item을 Commit할 Destination Container ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Destination Container ID (DestinationContainerId)", ToolTip="Item Reservation과 함께 슬롯·질량 Capacity를 예약할 Destination Container ID입니다."))
	FCFInventoryContainerId DestinationContainerId;

	// [v1.0.0] Item을 Commit할 Destination Slot 또는 MountProfile 위치 키입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Destination Slot ID (DestinationSlotId)", ToolTip="Prepared 동안 다른 Transaction이 사용할 수 없고 Commit 시 Item이 배치될 Destination Slot ID입니다."))
	FName DestinationSlotId = NAME_None;

	// [v1.0.0] Destination Mass Capacity에서 예약할 외부 해석 Item 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(ClampMin="0.0", Units="kg", DisplayName="예약 Item 질량 kg (RequestedMassKg)", ToolTip="Domain DataAsset에서 호출자가 해석한 Item 질량입니다. Inventory Definition이나 Instance에 복제되지 않습니다."))
	float RequestedMassKg = 0.0f;
};

/**
 * Item 또는 Destination Capacity를 잠그는 Reservation 한 건입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryReservation
{
	GENERATED_BODY()

	// [v1.0.0] Reservation ID, Transaction ID, 종류와 자원 식별자가 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 현재 다른 Transaction을 차단하는 Active Reservation인지 반환합니다.
	bool IsActive() const;

	// [v1.0.0] 이 Reservation 한 건의 고유 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="Reservation ID (ReservationId)", ToolTip="이 Reservation을 조회하거나 멱등 취소할 때 사용하는 고유 ID입니다."))
	FCFInventoryReservationId ReservationId;

	// [v1.0.0] 이 Reservation을 소유하는 Transaction ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="Transaction ID (TransactionId)", ToolTip="Item Reservation과 Destination Capacity Reservation을 하나의 Transfer로 묶는 Transaction ID입니다."))
	FCFInventoryTransactionId TransactionId;

	// [v1.0.0] 이 Reservation을 생성한 Action 또는 Coordinator 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="Action 소유자 ID (ActionOwnerId)", ToolTip="Reservation 수명의 외부 소유자를 식별하는 불투명 Action ID입니다."))
	FName ActionOwnerId = NAME_None;

	// [v1.0.0] Item 잠금 또는 Destination Capacity 잠금 중 Reservation 종류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="Reservation 종류 (ReservationKind)", ToolTip="Item 자체를 잠그는지 Destination Slot·Capacity를 잠그는지 나타냅니다."))
	ECFInventoryReservationKind ReservationKind = ECFInventoryReservationKind::Unknown;

	// [v1.0.0] Active, Cancelled 또는 Commit에서 Consumed된 수명 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="Reservation 상태 (ReservationState)", ToolTip="Active만 다른 Transfer를 차단하며 Cancelled와 Consumed는 다시 Capacity를 점유하지 않습니다."))
	ECFInventoryReservationState ReservationState = ECFInventoryReservationState::Unknown;

	// [v1.0.0] Item Reservation이 잠그는 실제 Item Instance ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="예약 Item Instance ID (ItemInstanceId)", ToolTip="Item Reservation에서 잠그는 실제 Item Instance ID입니다. Capacity Reservation에서는 같은 Transfer Item 확인용으로 보존됩니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] Item의 Source 또는 Capacity의 Destination Container ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="예약 Container ID (ContainerId)", ToolTip="Item Reservation에서는 Source, Capacity Reservation에서는 Destination Container를 가리킵니다."))
	FCFInventoryContainerId ContainerId;

	// [v1.0.0] Item의 Source 또는 Capacity의 Destination Slot ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(DisplayName="예약 Slot ID (ContainerSlotId)", ToolTip="Item Reservation에서는 Source Slot, Capacity Reservation에서는 선점한 Destination Slot입니다."))
	FName ContainerSlotId = NAME_None;

	// [v1.0.0] Destination Capacity Reservation이 선점하는 슬롯 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(ClampMin="0", DisplayName="예약 슬롯 수 (ReservedSlotCount)", ToolTip="P0 Quantity 1 Transfer의 Destination Capacity Reservation은 1, Item Reservation은 0입니다."))
	int32 ReservedSlotCount = 0;

	// [v1.0.0] Destination Capacity Reservation이 선점하는 외부 해석 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation", meta=(ClampMin="0.0", Units="kg", DisplayName="예약 질량 kg (ReservedMassKg)", ToolTip="Destination Capacity Reservation이 다른 Prepared Transfer로부터 보호하는 외부 해석 Item 질량입니다."))
	float ReservedMassKg = 0.0f;
};

/**
 * Prepare에서 고정한 Source·Destination 조건과 Reservation을 보존하는 Transaction입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryTransaction
{
	GENERATED_BODY()

	// [v1.0.0] Transaction ID, 요청, Prepared Item과 Reservation ID가 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] Prepare에서 확정한 Transfer 요청 Snapshot입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Transfer 요청 (TransferRequest)", ToolTip="Commit 재검증과 중복 Prepare 판정에 사용하는 원본 Transfer 요청입니다."))
	FCFInventoryTransferRequest TransferRequest;

	// [v1.0.0] Prepare 시 Source에서 확인한 정확한 Quantity 1 Item Instance Snapshot입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Prepare Item Instance (PreparedItemInstance)", ToolTip="Commit 시 같은 ItemInstanceId뿐 아니라 Definition과 Quantity도 Prepare 상태와 같은지 재검증합니다."))
	FCFInventoryItemInstance PreparedItemInstance;

	// [v1.0.0] Prepare 시 확인한 Source Container 참조 Snapshot입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Prepare Source Container 참조 (PreparedSourceContainerRef)", ToolTip="Commit 시 Source Container의 Owner와 종류가 바뀌지 않았는지 재검증합니다."))
	FCFInventoryContainerRef PreparedSourceContainerRef;

	// [v1.0.0] Prepare 시 확인한 Destination Container 참조 Snapshot입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Prepare Destination Container 참조 (PreparedDestinationContainerRef)", ToolTip="Commit 시 Destination Container의 Owner와 종류가 바뀌지 않았는지 재검증합니다."))
	FCFInventoryContainerRef PreparedDestinationContainerRef;

	// [v1.0.0] 이 Transaction이 소유하는 Item Reservation ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Item Reservation ID (ItemReservationId)", ToolTip="Prepared 동안 ItemInstanceId를 다른 Transaction으로부터 잠그는 Reservation ID입니다."))
	FCFInventoryReservationId ItemReservationId;

	// [v1.0.0] 이 Transaction이 소유하는 Destination Capacity Reservation ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Destination Reservation ID (DestinationReservationId)", ToolTip="Prepared 동안 Destination Slot과 슬롯·질량 Capacity를 다른 Transaction으로부터 잠그는 Reservation ID입니다."))
	FCFInventoryReservationId DestinationReservationId;

	// [v1.0.0] Prepared, Committed 또는 RolledBack Transaction 수명 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer", meta=(DisplayName="Transaction 상태 (TransactionState)", ToolTip="Committed와 RolledBack은 terminal 상태이며 같은 Transaction ID의 중복 Commit을 차단합니다."))
	ECFInventoryTransactionState TransactionState = ECFInventoryTransactionState::Unknown;
};

/**
 * Prepare·Commit·Rollback의 Transaction과 Reservation 식별 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryTransferResult
{
	GENERATED_BODY()

	// [v1.0.0] Prepare, Commit 또는 Rollback이 성공 또는 멱등 성공 결과인지 반환합니다.
	bool IsSuccessful() const;

	// [v1.0.0] Transfer 요청의 최종 결과 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer|Result", meta=(DisplayName="Transfer 결과 상태 (TransferStatus)", ToolTip="Prepare·Commit·Rollback 성공, 멱등 재호출 또는 구체적인 거부 사유를 나타냅니다."))
	ECFInventoryTransferStatus TransferStatus = ECFInventoryTransferStatus::None;

	// [v1.0.0] 결과와 연결된 Transaction ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer|Result", meta=(DisplayName="Transaction ID (TransactionId)", ToolTip="결과가 설명하는 Inventory Transaction ID입니다."))
	FCFInventoryTransactionId TransactionId;

	// [v1.0.0] Prepare 성공 시 생성되거나 기존 Transaction에서 조회된 Item Reservation ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer|Result", meta=(DisplayName="Item Reservation ID (ItemReservationId)", ToolTip="Item 잠금 Reservation의 ID입니다."))
	FCFInventoryReservationId ItemReservationId;

	// [v1.0.0] Prepare 성공 시 생성되거나 기존 Transaction에서 조회된 Destination Capacity Reservation ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Transfer|Result", meta=(DisplayName="Destination Reservation ID (DestinationReservationId)", ToolTip="Destination Slot과 Capacity 잠금 Reservation의 ID입니다."))
	FCFInventoryReservationId DestinationReservationId;
};

/**
 * Reservation 멱등 취소 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryCancelResult
{
	GENERATED_BODY()

	// [v1.0.0] 첫 취소 또는 반복 취소가 멱등 성공인지 반환합니다.
	bool IsSuccessful() const;

	// [v1.0.0] 취소 요청 결과 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation|Result", meta=(DisplayName="Reservation 취소 상태 (CancelState)", ToolTip="Cancelled와 AlreadyCancelled는 같은 최종 상태를 보장하는 멱등 성공입니다."))
	ECFInventoryCancelState CancelState = ECFInventoryCancelState::None;

	// [v1.0.0] 취소 결과가 설명하는 Reservation ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Reservation|Result", meta=(DisplayName="Reservation ID (ReservationId)", ToolTip="취소 또는 반복 취소 요청 대상 Reservation ID입니다."))
	FCFInventoryReservationId ReservationId;
};

/**
 * Pawn과 World 없이 Reservation과 Atomic Transfer 수명을 소유하는 순수 Ledger입니다.
 */
struct CARFIGHT_RE_API FCFInventoryTransferLedger
{
public:
	// [v1.0.0] Source Item과 Destination Slot·Capacity를 예약하고 Container를 변경하지 않은 Prepared Transaction을 생성합니다.
	FCFInventoryTransferResult PrepareTransfer(
		const TArray<FCFInventoryContainerState>& Containers,
		const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
		const FCFInventoryTransferRequest& TransferRequest);

	// [v1.0.0] Prepared 조건을 재검증하고 후보 Container 복사본이 유효할 때만 Source 제거와 Destination 추가를 함께 확정합니다.
	FCFInventoryTransferResult CommitTransfer(
		TArray<FCFInventoryContainerState>& InOutContainers,
		const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
		const FCFInventoryTransactionId& TransactionId);

	// [v1.0.0] Prepared Transaction의 Active Reservation을 모두 취소하고 Container를 변경하지 않은 RolledBack 상태로 전환합니다.
	FCFInventoryTransferResult RollbackTransfer(const FCFInventoryTransactionId& TransactionId);

		// [v1.0.0] 하나의 Reservation ID로 같은 Transaction의 모든 Active Reservation을 취소·Rollback하며 반복 호출은 AlreadyCancelled를 반환합니다.
	FCFInventoryCancelResult CancelReservation(const FCFInventoryReservationId& ReservationId);


	// [v1.0.0] 지정 ItemInstanceId를 잠그는 Active Item Reservation이 있는지 반환합니다.
	bool IsItemReserved(const FCFItemInstanceId& ItemInstanceId) const;

	// [v1.0.0] 지정 Destination Container와 Slot을 잠그는 Active Capacity Reservation이 있는지 반환합니다.
	bool IsDestinationSlotReserved(const FCFInventoryContainerId& ContainerId, FName ContainerSlotId) const;

	// [v1.0.0] 현재 다른 Transfer를 차단하는 Active Reservation 수를 반환합니다.
	int32 GetActiveReservationCount() const;

	// [v1.0.0] 지정 Reservation의 현재 상태를 반환하며 없으면 Unknown을 반환합니다.
	ECFInventoryReservationState GetReservationState(const FCFInventoryReservationId& ReservationId) const;

	// [v1.0.0] 지정 Transaction의 현재 상태를 반환하며 없으면 Unknown을 반환합니다.
	ECFInventoryTransactionState GetTransactionState(const FCFInventoryTransactionId& TransactionId) const;

private:
	// [v1.0.0] 전체 Reservation 수명을 보존하는 Ledger 내부 배열입니다.
	TArray<FCFInventoryReservation> Reservations;

	// [v1.0.0] 전체 Transaction terminal 상태와 Prepare Snapshot을 보존하는 Ledger 내부 배열입니다.
	TArray<FCFInventoryTransaction> Transactions;
};
