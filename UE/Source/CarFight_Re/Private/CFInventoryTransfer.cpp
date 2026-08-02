// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-03 Reservation과 Atomic Transfer 구현
// Scope: Item·Destination Capacity 잠금, 멱등 취소와 후보 복사본 기반 Prepare·Commit·Rollback을 구현합니다.
// Changelog:
// - v1.0.0: FCFInventoryTransferLedger와 Reservation·Transaction 상태 전이를 최초 추가.
// Migration:
// - Prepare와 Rollback은 Container를 변경하지 않는다.
// - Commit은 모든 재검증과 후보 Container 집합 Validation이 성공한 경우에만 원본 배열을 교체한다.
// - Commit 실패 후 Transaction은 Prepared 상태와 Reservation을 유지하며 호출자가 Rollback할 수 있다.

#include "CFInventoryTransfer.h"

namespace
{
	// [v1.0.0] 두 Container 참조가 같은 Owner, Container ID와 종류를 보존하는지 반환합니다.
	bool AreContainerRefsEquivalent(
		const FCFInventoryContainerRef& FirstContainerRef,
		const FCFInventoryContainerRef& SecondContainerRef)
	{
		return FirstContainerRef.OwnerId == SecondContainerRef.OwnerId
			&& FirstContainerRef.ContainerId == SecondContainerRef.ContainerId
			&& FirstContainerRef.ContainerType == SecondContainerRef.ContainerType;
	}

	// [v1.0.0] 두 Item Instance가 같은 ID, Definition과 Quantity를 보존하는지 반환합니다.
	bool AreItemInstancesEquivalent(
		const FCFInventoryItemInstance& FirstItemInstance,
		const FCFInventoryItemInstance& SecondItemInstance)
	{
		return FirstItemInstance.ItemHandle.ItemInstanceId == SecondItemInstance.ItemHandle.ItemInstanceId
			&& FirstItemInstance.ItemHandle.ItemDefinitionId == SecondItemInstance.ItemHandle.ItemDefinitionId
			&& FirstItemInstance.Quantity == SecondItemInstance.Quantity;
	}

	// [v1.0.0] 두 Transfer Request가 같은 Transaction 입력을 설명하는지 반환합니다.
	bool AreTransferRequestsEquivalent(
		const FCFInventoryTransferRequest& FirstRequest,
		const FCFInventoryTransferRequest& SecondRequest)
	{
		return FirstRequest.TransactionId == SecondRequest.TransactionId
			&& FirstRequest.ActionOwnerId == SecondRequest.ActionOwnerId
			&& FirstRequest.ItemInstanceId == SecondRequest.ItemInstanceId
			&& FirstRequest.SourceContainerId == SecondRequest.SourceContainerId
			&& FirstRequest.SourceSlotId == SecondRequest.SourceSlotId
			&& FirstRequest.DestinationContainerId == SecondRequest.DestinationContainerId
			&& FirstRequest.DestinationSlotId == SecondRequest.DestinationSlotId
			&& FMath::IsNearlyEqual(FirstRequest.RequestedMassKg, SecondRequest.RequestedMassKg);
	}

	// [v1.0.0] Container ID와 일치하는 유일한 Container의 배열 인덱스를 반환합니다.
	int32 FindContainerIndex(
		const TArray<FCFInventoryContainerState>& Containers,
		const FCFInventoryContainerId& ContainerId)
	{
		return Containers.IndexOfByPredicate(
			[&ContainerId](const FCFInventoryContainerState& Container)
			{
				return Container.ContainerRef.ContainerId == ContainerId;
			});
	}

	// [v1.0.0] Container에서 ItemInstanceId와 Slot ID가 모두 일치하는 Entry 인덱스를 반환합니다.
	int32 FindContainerEntryIndex(
		const FCFInventoryContainerState& Container,
		const FCFItemInstanceId& ItemInstanceId,
		const FName ContainerSlotId)
	{
		return Container.Entries.IndexOfByPredicate(
			[&ItemInstanceId, ContainerSlotId](const FCFInventoryContainerEntry& Entry)
			{
				return Entry.IsValid()
					&& Entry.ItemInstance.ItemHandle.ItemInstanceId == ItemInstanceId
					&& Entry.ContainerSlotId == ContainerSlotId;
			});
	}

	// [v1.0.0] Container에서 지정 Slot ID를 사용하는 Entry가 있는지 반환합니다.
	bool IsContainerSlotOccupied(
		const FCFInventoryContainerState& Container,
		const FName ContainerSlotId)
	{
		return Container.Entries.ContainsByPredicate(
			[ContainerSlotId](const FCFInventoryContainerEntry& Entry)
			{
				return Entry.IsValid() && Entry.ContainerSlotId == ContainerSlotId;
			});
	}

	// [v1.0.0] 질량 Snapshot 배열에서 Container ID와 정확히 한 번 일치하는 유효 질량을 반환합니다.
	bool TryResolveContainerMass(
		const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
		const FCFInventoryContainerId& ContainerId,
		float& OutOccupiedMassKg)
	{
		OutOccupiedMassKg = 0.0f;

		// [v1.0.0] 같은 Container ID와 일치한 유효 질량 Snapshot 수입니다.
		int32 MatchingSnapshotCount = 0;

		for (const FCFInventoryContainerMassSnapshot& MassSnapshot : ContainerMassSnapshots)
		{
			if (MassSnapshot.ContainerId != ContainerId)
			{
				continue;
			}

			if (!MassSnapshot.IsValid())
			{
				return false;
			}

			++MatchingSnapshotCount;
			OutOccupiedMassKg = MassSnapshot.OccupiedMassKg;
		}

		return MatchingSnapshotCount == 1;
	}

	// [v1.0.0] Transfer Result에 Transaction과 Reservation ID를 채워 반환합니다.
	FCFInventoryTransferResult MakeTransferResult(
		const ECFInventoryTransferStatus TransferStatus,
		const FCFInventoryTransactionId& TransactionId,
		const FCFInventoryReservationId& ItemReservationId = FCFInventoryReservationId(),
		const FCFInventoryReservationId& DestinationReservationId = FCFInventoryReservationId())
	{
		// [v1.0.0] 호출자에게 반환할 Transfer 결과입니다.
		FCFInventoryTransferResult TransferResult;
		TransferResult.TransferStatus = TransferStatus;
		TransferResult.TransactionId = TransactionId;
		TransferResult.ItemReservationId = ItemReservationId;
		TransferResult.DestinationReservationId = DestinationReservationId;
		return TransferResult;
	}
}

// [v1.0.0] 새로운 고유 Reservation ID를 생성합니다.
FCFInventoryReservationId FCFInventoryReservationId::CreateNew()
{
	return FromGuid(FGuid::NewGuid());
}

// [v1.0.0] 기존 Guid를 보존하는 Reservation ID를 생성합니다.
FCFInventoryReservationId FCFInventoryReservationId::FromGuid(const FGuid& InGuid)
{
	// [v1.0.0] 입력 Guid를 보존할 Reservation ID입니다.
	FCFInventoryReservationId ReservationId;
	ReservationId.Value = InGuid;
	return ReservationId;
}

// [v1.0.0] 유효한 Reservation ID인지 반환합니다.
bool FCFInventoryReservationId::IsValid() const
{
	return Value.IsValid();
}

// [v1.0.0] 두 Reservation ID가 같은 예약을 가리키는지 비교합니다.
bool FCFInventoryReservationId::operator==(const FCFInventoryReservationId& Other) const
{
	return Value == Other.Value;
}

// [v1.0.0] 두 Reservation ID가 다른 예약을 가리키는지 비교합니다.
bool FCFInventoryReservationId::operator!=(const FCFInventoryReservationId& Other) const
{
	return !(*this == Other);
}

// [v1.0.0] 새로운 고유 Transaction ID를 생성합니다.
FCFInventoryTransactionId FCFInventoryTransactionId::CreateNew()
{
	return FromGuid(FGuid::NewGuid());
}

// [v1.0.0] 기존 Guid를 보존하는 Transaction ID를 생성합니다.
FCFInventoryTransactionId FCFInventoryTransactionId::FromGuid(const FGuid& InGuid)
{
	// [v1.0.0] 입력 Guid를 보존할 Transaction ID입니다.
	FCFInventoryTransactionId TransactionId;
	TransactionId.Value = InGuid;
	return TransactionId;
}

// [v1.0.0] 유효한 Transaction ID인지 반환합니다.
bool FCFInventoryTransactionId::IsValid() const
{
	return Value.IsValid();
}

// [v1.0.0] 두 Transaction ID가 같은 Transaction을 가리키는지 비교합니다.
bool FCFInventoryTransactionId::operator==(const FCFInventoryTransactionId& Other) const
{
	return Value == Other.Value;
}

// [v1.0.0] 두 Transaction ID가 다른 Transaction을 가리키는지 비교합니다.
bool FCFInventoryTransactionId::operator!=(const FCFInventoryTransactionId& Other) const
{
	return !(*this == Other);
}

// [v1.0.0] Container ID와 외부 해석 질량이 유효한지 반환합니다.
bool FCFInventoryContainerMassSnapshot::IsValid() const
{
	return ContainerId.IsValid()
		&& FMath::IsFinite(OccupiedMassKg)
		&& OccupiedMassKg >= 0.0f;
}

// [v1.0.0] Transaction, Action Owner, Item, Source·Destination과 질량 입력이 모두 유효한지 반환합니다.
bool FCFInventoryTransferRequest::IsValid() const
{
	return TransactionId.IsValid()
		&& !ActionOwnerId.IsNone()
		&& ItemInstanceId.IsValid()
		&& SourceContainerId.IsValid()
		&& !SourceSlotId.IsNone()
		&& DestinationContainerId.IsValid()
		&& !DestinationSlotId.IsNone()
		&& SourceContainerId != DestinationContainerId
		&& FMath::IsFinite(RequestedMassKg)
		&& RequestedMassKg >= 0.0f;
}

// [v1.0.0] Reservation ID, Transaction ID, 종류와 자원 식별자가 유효한지 반환합니다.
bool FCFInventoryReservation::IsValid() const
{
	if (!ReservationId.IsValid()
		|| !TransactionId.IsValid()
		|| ActionOwnerId.IsNone()
		|| !ItemInstanceId.IsValid()
		|| !ContainerId.IsValid()
		|| ContainerSlotId.IsNone()
		|| ReservationState == ECFInventoryReservationState::Unknown
		|| !FMath::IsFinite(ReservedMassKg)
		|| ReservedMassKg < 0.0f)
	{
		return false;
	}

	if (ReservationKind == ECFInventoryReservationKind::Item)
	{
		return ReservedSlotCount == 0 && FMath::IsNearlyZero(ReservedMassKg);
	}

	if (ReservationKind == ECFInventoryReservationKind::DestinationCapacity)
	{
		return ReservedSlotCount == 1;
	}

	return false;
}

// [v1.0.0] 현재 다른 Transaction을 차단하는 Active Reservation인지 반환합니다.
bool FCFInventoryReservation::IsActive() const
{
	return IsValid() && ReservationState == ECFInventoryReservationState::Active;
}

// [v1.0.0] Transaction ID, 요청, Prepared Item과 Reservation ID가 유효한지 반환합니다.
bool FCFInventoryTransaction::IsValid() const
{
	return TransferRequest.IsValid()
		&& PreparedItemInstance.IsValid()
		&& PreparedSourceContainerRef.IsValid()
		&& PreparedDestinationContainerRef.IsValid()
		&& ItemReservationId.IsValid()
		&& DestinationReservationId.IsValid()
		&& TransactionState != ECFInventoryTransactionState::Unknown;
}

// [v1.0.0] Prepare, Commit 또는 Rollback이 성공 또는 멱등 성공 결과인지 반환합니다.
bool FCFInventoryTransferResult::IsSuccessful() const
{
	return TransferStatus == ECFInventoryTransferStatus::Prepared
		|| TransferStatus == ECFInventoryTransferStatus::AlreadyPrepared
		|| TransferStatus == ECFInventoryTransferStatus::Committed
		|| TransferStatus == ECFInventoryTransferStatus::AlreadyCommitted
		|| TransferStatus == ECFInventoryTransferStatus::RolledBack
		|| TransferStatus == ECFInventoryTransferStatus::AlreadyRolledBack;
}

// [v1.0.0] 첫 취소 또는 반복 취소가 멱등 성공인지 반환합니다.
bool FCFInventoryCancelResult::IsSuccessful() const
{
	return CancelState == ECFInventoryCancelState::Cancelled
		|| CancelState == ECFInventoryCancelState::AlreadyCancelled;
}

// [v1.0.0] Source Item과 Destination Slot·Capacity를 예약하고 Container를 변경하지 않은 Prepared Transaction을 생성합니다.
FCFInventoryTransferResult FCFInventoryTransferLedger::PrepareTransfer(
	const TArray<FCFInventoryContainerState>& Containers,
	const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
	const FCFInventoryTransferRequest& TransferRequest)
{
	if (!TransferRequest.IsValid())
	{
		return MakeTransferResult(ECFInventoryTransferStatus::InvalidRequest, TransferRequest.TransactionId);
	}

	// [v1.0.0] 같은 Transaction ID로 이미 생성된 Transaction입니다.
	const FCFInventoryTransaction* ExistingTransaction = Transactions.FindByPredicate(
		[&TransferRequest](const FCFInventoryTransaction& Transaction)
		{
			return Transaction.TransferRequest.TransactionId == TransferRequest.TransactionId;
		});

	if (ExistingTransaction)
	{
		if (!AreTransferRequestsEquivalent(ExistingTransaction->TransferRequest, TransferRequest))
		{
			return MakeTransferResult(ECFInventoryTransferStatus::TransactionIdConflict, TransferRequest.TransactionId);
		}

		if (ExistingTransaction->TransactionState == ECFInventoryTransactionState::Prepared)
		{
			return MakeTransferResult(
				ECFInventoryTransferStatus::AlreadyPrepared,
				TransferRequest.TransactionId,
				ExistingTransaction->ItemReservationId,
				ExistingTransaction->DestinationReservationId);
		}

		if (ExistingTransaction->TransactionState == ECFInventoryTransactionState::Committed)
		{
			return MakeTransferResult(
				ECFInventoryTransferStatus::AlreadyCommitted,
				TransferRequest.TransactionId,
				ExistingTransaction->ItemReservationId,
				ExistingTransaction->DestinationReservationId);
		}

		return MakeTransferResult(
			ECFInventoryTransferStatus::AlreadyRolledBack,
			TransferRequest.TransactionId,
			ExistingTransaction->ItemReservationId,
			ExistingTransaction->DestinationReservationId);
	}

	// [v1.0.0] 현재 Container 집합 불변식 오류를 수집할 배열입니다.
	TArray<FText> ContainerValidationErrors;
	if (!FCFInventoryAccessQuery::ValidateContainerSet(Containers, ContainerValidationErrors))
	{
		return MakeTransferResult(ECFInventoryTransferStatus::InvalidContainerSet, TransferRequest.TransactionId);
	}

	// [v1.0.0] 이동 Item의 현재 단일 소유 위치 조회 결과입니다.
	const FCFInventoryLocationResult SourceLocationResult = FCFInventoryAccessQuery::QueryItemLocation(
		Containers,
		TransferRequest.ItemInstanceId);
	if (!SourceLocationResult.IsFound())
	{
		return MakeTransferResult(ECFInventoryTransferStatus::SourceItemMissing, TransferRequest.TransactionId);
	}

	if (SourceLocationResult.ItemLocation.ContainerRef.ContainerId != TransferRequest.SourceContainerId
		|| SourceLocationResult.ItemLocation.ContainerSlotId != TransferRequest.SourceSlotId)
	{
		return MakeTransferResult(ECFInventoryTransferStatus::SourceLocationChanged, TransferRequest.TransactionId);
	}

	// [v1.0.0] Source Container 배열 인덱스입니다.
	const int32 SourceContainerIndex = FindContainerIndex(Containers, TransferRequest.SourceContainerId);

	// [v1.0.0] Destination Container 배열 인덱스입니다.
	const int32 DestinationContainerIndex = FindContainerIndex(Containers, TransferRequest.DestinationContainerId);
	if (SourceContainerIndex == INDEX_NONE)
	{
		return MakeTransferResult(ECFInventoryTransferStatus::SourceItemMissing, TransferRequest.TransactionId);
	}

	if (DestinationContainerIndex == INDEX_NONE)
	{
		return MakeTransferResult(ECFInventoryTransferStatus::DestinationContainerMissing, TransferRequest.TransactionId);
	}

	// [v1.0.0] Prepare 시 Item Instance Snapshot을 가져올 Source Container입니다.
	const FCFInventoryContainerState& SourceContainer = Containers[SourceContainerIndex];

	// [v1.0.0] Slot과 Capacity를 예약할 Destination Container입니다.
	const FCFInventoryContainerState& DestinationContainer = Containers[DestinationContainerIndex];

	// [v1.0.0] Source Item과 Slot이 함께 일치하는 Entry 인덱스입니다.
	const int32 SourceEntryIndex = FindContainerEntryIndex(
		SourceContainer,
		TransferRequest.ItemInstanceId,
		TransferRequest.SourceSlotId);
	if (SourceEntryIndex == INDEX_NONE)
	{
		return MakeTransferResult(ECFInventoryTransferStatus::SourceLocationChanged, TransferRequest.TransactionId);
	}

	if (IsContainerSlotOccupied(DestinationContainer, TransferRequest.DestinationSlotId))
	{
		return MakeTransferResult(ECFInventoryTransferStatus::DestinationSlotOccupied, TransferRequest.TransactionId);
	}

	if (IsItemReserved(TransferRequest.ItemInstanceId))
	{
		return MakeTransferResult(ECFInventoryTransferStatus::ItemAlreadyReserved, TransferRequest.TransactionId);
	}

	if (IsDestinationSlotReserved(TransferRequest.DestinationContainerId, TransferRequest.DestinationSlotId))
	{
		return MakeTransferResult(ECFInventoryTransferStatus::DestinationSlotReserved, TransferRequest.TransactionId);
	}

	// [v1.0.0] Destination Container에 이미 Active로 예약된 추가 슬롯 수입니다.
	int32 ActiveReservedSlotCount = 0;

	// [v1.0.0] Destination Container에 이미 Active로 예약된 추가 질량입니다.
	float ActiveReservedMassKg = 0.0f;

	for (const FCFInventoryReservation& Reservation : Reservations)
	{
		if (!Reservation.IsActive()
			|| Reservation.ReservationKind != ECFInventoryReservationKind::DestinationCapacity
			|| Reservation.ContainerId != TransferRequest.DestinationContainerId)
		{
			continue;
		}

		ActiveReservedSlotCount += Reservation.ReservedSlotCount;
		ActiveReservedMassKg += Reservation.ReservedMassKg;
	}

	// [v1.0.0] 현재 Entry, 기존 Reservation과 새 요청을 합친 예상 사용 슬롯 수입니다.
	const int32 ProjectedOccupiedSlotCount = DestinationContainer.GetOccupiedSlotCount()
		+ ActiveReservedSlotCount
		+ 1;
	if (ProjectedOccupiedSlotCount > DestinationContainer.Capacity.MaximumSlotCount)
	{
		return MakeTransferResult(ECFInventoryTransferStatus::DestinationCapacityExceeded, TransferRequest.TransactionId);
	}

	if (DestinationContainer.Capacity.bUseMassLimit)
	{
		// [v1.0.0] 호출자가 Domain DataAsset에서 해석한 Destination 현재 총질량입니다.
		float DestinationOccupiedMassKg = 0.0f;
		if (!TryResolveContainerMass(
			ContainerMassSnapshots,
			TransferRequest.DestinationContainerId,
			DestinationOccupiedMassKg))
		{
			return MakeTransferResult(ECFInventoryTransferStatus::MissingMassSnapshot, TransferRequest.TransactionId);
		}

		// [v1.0.0] 현재 질량, 기존 Reservation과 새 요청을 합친 예상 Destination 질량입니다.
		const float ProjectedOccupiedMassKg = DestinationOccupiedMassKg
			+ ActiveReservedMassKg
			+ TransferRequest.RequestedMassKg;
		if (ProjectedOccupiedMassKg > DestinationContainer.Capacity.MaximumMassKg)
		{
			return MakeTransferResult(ECFInventoryTransferStatus::DestinationCapacityExceeded, TransferRequest.TransactionId);
		}
	}

	// [v1.0.0] Source Item을 다른 Transaction으로부터 잠글 Reservation입니다.
	FCFInventoryReservation ItemReservation;
	ItemReservation.ReservationId = FCFInventoryReservationId::CreateNew();
	ItemReservation.TransactionId = TransferRequest.TransactionId;
	ItemReservation.ActionOwnerId = TransferRequest.ActionOwnerId;
	ItemReservation.ReservationKind = ECFInventoryReservationKind::Item;
	ItemReservation.ReservationState = ECFInventoryReservationState::Active;
	ItemReservation.ItemInstanceId = TransferRequest.ItemInstanceId;
	ItemReservation.ContainerId = TransferRequest.SourceContainerId;
	ItemReservation.ContainerSlotId = TransferRequest.SourceSlotId;

	// [v1.0.0] Destination Slot과 Capacity를 다른 Transaction으로부터 잠글 Reservation입니다.
	FCFInventoryReservation DestinationReservation;
	DestinationReservation.ReservationId = FCFInventoryReservationId::CreateNew();
	DestinationReservation.TransactionId = TransferRequest.TransactionId;
	DestinationReservation.ActionOwnerId = TransferRequest.ActionOwnerId;
	DestinationReservation.ReservationKind = ECFInventoryReservationKind::DestinationCapacity;
	DestinationReservation.ReservationState = ECFInventoryReservationState::Active;
	DestinationReservation.ItemInstanceId = TransferRequest.ItemInstanceId;
	DestinationReservation.ContainerId = TransferRequest.DestinationContainerId;
	DestinationReservation.ContainerSlotId = TransferRequest.DestinationSlotId;
	DestinationReservation.ReservedSlotCount = 1;
	DestinationReservation.ReservedMassKg = TransferRequest.RequestedMassKg;

	if (!ItemReservation.IsValid() || !DestinationReservation.IsValid())
	{
		return MakeTransferResult(ECFInventoryTransferStatus::InvalidRequest, TransferRequest.TransactionId);
	}

	// [v1.0.0] Prepare Snapshot과 두 Reservation ID를 보존할 Transaction입니다.
	FCFInventoryTransaction Transaction;
	Transaction.TransferRequest = TransferRequest;
	Transaction.PreparedItemInstance = SourceContainer.Entries[SourceEntryIndex].ItemInstance;
	Transaction.PreparedSourceContainerRef = SourceContainer.ContainerRef;
	Transaction.PreparedDestinationContainerRef = DestinationContainer.ContainerRef;
	Transaction.ItemReservationId = ItemReservation.ReservationId;
	Transaction.DestinationReservationId = DestinationReservation.ReservationId;
	Transaction.TransactionState = ECFInventoryTransactionState::Prepared;

	if (!Transaction.IsValid())
	{
		return MakeTransferResult(ECFInventoryTransferStatus::InvalidRequest, TransferRequest.TransactionId);
	}

	Reservations.Add(ItemReservation);
	Reservations.Add(DestinationReservation);
	Transactions.Add(Transaction);

	return MakeTransferResult(
		ECFInventoryTransferStatus::Prepared,
		TransferRequest.TransactionId,
		ItemReservation.ReservationId,
		DestinationReservation.ReservationId);
}

// [v1.0.0] Prepared 조건을 재검증하고 후보 Container 복사본이 유효할 때만 Source 제거와 Destination 추가를 함께 확정합니다.
FCFInventoryTransferResult FCFInventoryTransferLedger::CommitTransfer(
	TArray<FCFInventoryContainerState>& InOutContainers,
	const TArray<FCFInventoryContainerMassSnapshot>& ContainerMassSnapshots,
	const FCFInventoryTransactionId& TransactionId)
{
	if (!TransactionId.IsValid())
	{
		return MakeTransferResult(ECFInventoryTransferStatus::InvalidRequest, TransactionId);
	}

	// [v1.0.0] Commit할 Transaction의 Ledger 배열 인덱스입니다.
	const int32 TransactionIndex = Transactions.IndexOfByPredicate(
		[&TransactionId](const FCFInventoryTransaction& Transaction)
		{
			return Transaction.TransferRequest.TransactionId == TransactionId;
		});
	if (TransactionIndex == INDEX_NONE)
	{
		return MakeTransferResult(ECFInventoryTransferStatus::TransactionMissing, TransactionId);
	}

	// [v1.0.0] Commit 상태를 전환할 Transaction입니다.
	FCFInventoryTransaction& Transaction = Transactions[TransactionIndex];
	if (Transaction.TransactionState == ECFInventoryTransactionState::Committed)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::AlreadyCommitted,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	if (Transaction.TransactionState == ECFInventoryTransactionState::RolledBack)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::AlreadyRolledBack,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	// [v1.0.0] Item Reservation의 Ledger 배열 인덱스입니다.
	const int32 ItemReservationIndex = Reservations.IndexOfByPredicate(
		[&Transaction](const FCFInventoryReservation& Reservation)
		{
			return Reservation.ReservationId == Transaction.ItemReservationId;
		});

	// [v1.0.0] Destination Capacity Reservation의 Ledger 배열 인덱스입니다.
	const int32 DestinationReservationIndex = Reservations.IndexOfByPredicate(
		[&Transaction](const FCFInventoryReservation& Reservation)
		{
			return Reservation.ReservationId == Transaction.DestinationReservationId;
		});
	if (ItemReservationIndex == INDEX_NONE || DestinationReservationIndex == INDEX_NONE)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::ReservationUnavailable,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	// [v1.0.0] Commit 전까지 Active여야 하는 Item Reservation입니다.
	FCFInventoryReservation& ItemReservation = Reservations[ItemReservationIndex];

	// [v1.0.0] Commit 전까지 Active여야 하는 Destination Capacity Reservation입니다.
	FCFInventoryReservation& DestinationReservation = Reservations[DestinationReservationIndex];
	if (!ItemReservation.IsActive()
		|| !DestinationReservation.IsActive()
		|| ItemReservation.TransactionId != TransactionId
		|| DestinationReservation.TransactionId != TransactionId
		|| ItemReservation.ReservationKind != ECFInventoryReservationKind::Item
		|| DestinationReservation.ReservationKind != ECFInventoryReservationKind::DestinationCapacity)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::ReservationUnavailable,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	// [v1.0.0] Commit 직전 Container 집합 불변식 오류를 수집할 배열입니다.
	TArray<FText> ContainerValidationErrors;
	if (!FCFInventoryAccessQuery::ValidateContainerSet(InOutContainers, ContainerValidationErrors))
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::InvalidContainerSet,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	// [v1.0.0] Commit 직전 Source Container 배열 인덱스입니다.
	const int32 SourceContainerIndex = FindContainerIndex(
		InOutContainers,
		Transaction.TransferRequest.SourceContainerId);

	// [v1.0.0] Commit 직전 Destination Container 배열 인덱스입니다.
	const int32 DestinationContainerIndex = FindContainerIndex(
		InOutContainers,
		Transaction.TransferRequest.DestinationContainerId);
	if (SourceContainerIndex == INDEX_NONE)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::SourceItemMissing,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	if (DestinationContainerIndex == INDEX_NONE)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::DestinationContainerMissing,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	// [v1.0.0] Commit 직전 Source Container 상태입니다.
	const FCFInventoryContainerState& SourceContainer = InOutContainers[SourceContainerIndex];

	// [v1.0.0] Commit 직전 Destination Container 상태입니다.
	const FCFInventoryContainerState& DestinationContainer = InOutContainers[DestinationContainerIndex];
	if (!AreContainerRefsEquivalent(SourceContainer.ContainerRef, Transaction.PreparedSourceContainerRef)
		|| !AreContainerRefsEquivalent(DestinationContainer.ContainerRef, Transaction.PreparedDestinationContainerRef))
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::ContainerStateChanged,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	// [v1.0.0] Commit 직전 Source Item과 Slot이 모두 일치하는 Entry 인덱스입니다.
	const int32 SourceEntryIndex = FindContainerEntryIndex(
		SourceContainer,
		Transaction.TransferRequest.ItemInstanceId,
		Transaction.TransferRequest.SourceSlotId);
	if (SourceEntryIndex == INDEX_NONE
		|| !AreItemInstancesEquivalent(
			SourceContainer.Entries[SourceEntryIndex].ItemInstance,
			Transaction.PreparedItemInstance))
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::SourceLocationChanged,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	if (IsContainerSlotOccupied(DestinationContainer, Transaction.TransferRequest.DestinationSlotId))
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::DestinationSlotOccupied,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	// [v1.0.0] Commit 후 실제 Entry와 아직 Active인 다른 Transaction을 모두 보호할 예약 슬롯 수입니다.
	int32 ActiveReservedSlotCount = 0;

	// [v1.0.0] Commit 후 실제 질량과 아직 Active인 다른 Transaction을 모두 보호할 예약 질량입니다.
	float ActiveReservedMassKg = 0.0f;

	for (const FCFInventoryReservation& Reservation : Reservations)
	{
		if (!Reservation.IsActive()
			|| Reservation.ReservationKind != ECFInventoryReservationKind::DestinationCapacity
			|| Reservation.ContainerId != Transaction.TransferRequest.DestinationContainerId)
		{
			continue;
		}

		ActiveReservedSlotCount += Reservation.ReservedSlotCount;
		ActiveReservedMassKg += Reservation.ReservedMassKg;
	}

	// [v1.0.0] 현재 Entry와 모든 Active Reservation을 합친 Commit 직전 보호 슬롯 수입니다.
	const int32 ProtectedOccupiedSlotCount = DestinationContainer.GetOccupiedSlotCount() + ActiveReservedSlotCount;
	if (ProtectedOccupiedSlotCount > DestinationContainer.Capacity.MaximumSlotCount)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::DestinationCapacityExceeded,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	if (DestinationContainer.Capacity.bUseMassLimit)
	{
		// [v1.0.0] Commit 직전 호출자가 다시 해석한 Destination 현재 총질량입니다.
		float DestinationOccupiedMassKg = 0.0f;
		if (!TryResolveContainerMass(
			ContainerMassSnapshots,
			Transaction.TransferRequest.DestinationContainerId,
			DestinationOccupiedMassKg))
		{
			return MakeTransferResult(
				ECFInventoryTransferStatus::MissingMassSnapshot,
				TransactionId,
				Transaction.ItemReservationId,
				Transaction.DestinationReservationId);
		}

		// [v1.0.0] 현재 질량과 모든 Active Reservation을 합친 Commit 직전 보호 질량입니다.
		const float ProtectedOccupiedMassKg = DestinationOccupiedMassKg + ActiveReservedMassKg;
		if (ProtectedOccupiedMassKg > DestinationContainer.Capacity.MaximumMassKg)
		{
			return MakeTransferResult(
				ECFInventoryTransferStatus::DestinationCapacityExceeded,
				TransactionId,
				Transaction.ItemReservationId,
				Transaction.DestinationReservationId);
		}
	}

	// [v1.0.0] 원본을 변경하지 않고 전체 이동과 불변식을 검증할 후보 Container 복사본입니다.
	TArray<FCFInventoryContainerState> CandidateContainers = InOutContainers;

	// [v1.0.0] 후보 Source Container입니다.
	FCFInventoryContainerState& CandidateSourceContainer = CandidateContainers[SourceContainerIndex];

	// [v1.0.0] 후보 Destination Container입니다.
	FCFInventoryContainerState& CandidateDestinationContainer = CandidateContainers[DestinationContainerIndex];
	CandidateSourceContainer.Entries.RemoveAt(SourceEntryIndex);

	// [v1.0.0] 후보 Destination에 추가할 Prepare Item과 Destination Slot Entry입니다.
	FCFInventoryContainerEntry DestinationEntry;
	DestinationEntry.ItemInstance = Transaction.PreparedItemInstance;
	DestinationEntry.ContainerSlotId = Transaction.TransferRequest.DestinationSlotId;
	CandidateDestinationContainer.Entries.Add(DestinationEntry);

	ContainerValidationErrors.Reset();
	if (!FCFInventoryAccessQuery::ValidateContainerSet(CandidateContainers, ContainerValidationErrors))
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::CommitValidationFailed,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	InOutContainers = MoveTemp(CandidateContainers);
	ItemReservation.ReservationState = ECFInventoryReservationState::Consumed;
	DestinationReservation.ReservationState = ECFInventoryReservationState::Consumed;
	Transaction.TransactionState = ECFInventoryTransactionState::Committed;

	return MakeTransferResult(
		ECFInventoryTransferStatus::Committed,
		TransactionId,
		Transaction.ItemReservationId,
		Transaction.DestinationReservationId);
}

// [v1.0.0] Prepared Transaction의 Active Reservation을 모두 취소하고 Container를 변경하지 않은 RolledBack 상태로 전환합니다.
FCFInventoryTransferResult FCFInventoryTransferLedger::RollbackTransfer(
	const FCFInventoryTransactionId& TransactionId)
{
	if (!TransactionId.IsValid())
	{
		return MakeTransferResult(ECFInventoryTransferStatus::InvalidRequest, TransactionId);
	}

	// [v1.0.0] Rollback할 Transaction의 Ledger 배열 인덱스입니다.
	const int32 TransactionIndex = Transactions.IndexOfByPredicate(
		[&TransactionId](const FCFInventoryTransaction& Transaction)
		{
			return Transaction.TransferRequest.TransactionId == TransactionId;
		});
	if (TransactionIndex == INDEX_NONE)
	{
		return MakeTransferResult(ECFInventoryTransferStatus::TransactionMissing, TransactionId);
	}

	// [v1.0.0] Rollback 상태를 전환할 Transaction입니다.
	FCFInventoryTransaction& Transaction = Transactions[TransactionIndex];
	if (Transaction.TransactionState == ECFInventoryTransactionState::Committed)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::CannotRollbackCommitted,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	if (Transaction.TransactionState == ECFInventoryTransactionState::RolledBack)
	{
		return MakeTransferResult(
			ECFInventoryTransferStatus::AlreadyRolledBack,
			TransactionId,
			Transaction.ItemReservationId,
			Transaction.DestinationReservationId);
	}

	for (FCFInventoryReservation& Reservation : Reservations)
	{
		if (Reservation.TransactionId == TransactionId
			&& Reservation.ReservationState == ECFInventoryReservationState::Active)
		{
			Reservation.ReservationState = ECFInventoryReservationState::Cancelled;
		}
	}

	Transaction.TransactionState = ECFInventoryTransactionState::RolledBack;
	return MakeTransferResult(
		ECFInventoryTransferStatus::RolledBack,
		TransactionId,
		Transaction.ItemReservationId,
		Transaction.DestinationReservationId);
}

// [v1.0.0] 하나의 Reservation ID로 같은 Transaction의 모든 Active Reservation을 취소·Rollback하며 반복 호출은 AlreadyCancelled를 반환합니다.
FCFInventoryCancelResult FCFInventoryTransferLedger::CancelReservation(
	const FCFInventoryReservationId& ReservationId)
{
	// [v1.0.0] 호출자에게 반환할 Reservation 취소 결과입니다.
	FCFInventoryCancelResult CancelResult;
	CancelResult.ReservationId = ReservationId;

	if (!ReservationId.IsValid())
	{
		CancelResult.CancelState = ECFInventoryCancelState::InvalidReservationId;
		return CancelResult;
	}

	// [v1.0.0] 취소 Token으로 사용할 Reservation의 Ledger 배열 인덱스입니다.
	const int32 ReservationIndex = Reservations.IndexOfByPredicate(
		[&ReservationId](const FCFInventoryReservation& Reservation)
		{
			return Reservation.ReservationId == ReservationId;
		});
	if (ReservationIndex == INDEX_NONE)
	{
		CancelResult.CancelState = ECFInventoryCancelState::ReservationMissing;
		return CancelResult;
	}

	// [v1.0.0] 취소 Token Reservation의 현재 수명 상태입니다.
	const ECFInventoryReservationState ReservationState = Reservations[ReservationIndex].ReservationState;
	if (ReservationState == ECFInventoryReservationState::Cancelled)
	{
		CancelResult.CancelState = ECFInventoryCancelState::AlreadyCancelled;
		return CancelResult;
	}

	if (ReservationState == ECFInventoryReservationState::Consumed)
	{
		CancelResult.CancelState = ECFInventoryCancelState::CannotCancelConsumed;
		return CancelResult;
	}

	// [v1.0.0] 취소 Token과 같은 Transaction에 속한 모든 Active Reservation을 찾을 Transaction ID입니다.
	const FCFInventoryTransactionId TransactionId = Reservations[ReservationIndex].TransactionId;

	for (FCFInventoryReservation& Reservation : Reservations)
	{
		if (Reservation.TransactionId == TransactionId
			&& Reservation.ReservationState == ECFInventoryReservationState::Active)
		{
			Reservation.ReservationState = ECFInventoryReservationState::Cancelled;
		}
	}

	// [v1.0.0] Reservation 취소와 함께 RolledBack terminal 상태로 전환할 Transaction입니다.
	FCFInventoryTransaction* Transaction = Transactions.FindByPredicate(
		[&TransactionId](const FCFInventoryTransaction& CandidateTransaction)
		{
			return CandidateTransaction.TransferRequest.TransactionId == TransactionId;
		});
	if (Transaction && Transaction->TransactionState == ECFInventoryTransactionState::Prepared)
	{
		Transaction->TransactionState = ECFInventoryTransactionState::RolledBack;
	}

	CancelResult.CancelState = ECFInventoryCancelState::Cancelled;
	return CancelResult;
}


// [v1.0.0] 지정 ItemInstanceId를 잠그는 Active Item Reservation이 있는지 반환합니다.
bool FCFInventoryTransferLedger::IsItemReserved(const FCFItemInstanceId& ItemInstanceId) const
{
	return ItemInstanceId.IsValid()
		&& Reservations.ContainsByPredicate(
			[&ItemInstanceId](const FCFInventoryReservation& Reservation)
			{
				return Reservation.IsActive()
					&& Reservation.ReservationKind == ECFInventoryReservationKind::Item
					&& Reservation.ItemInstanceId == ItemInstanceId;
			});
}

// [v1.0.0] 지정 Destination Container와 Slot을 잠그는 Active Capacity Reservation이 있는지 반환합니다.
bool FCFInventoryTransferLedger::IsDestinationSlotReserved(
	const FCFInventoryContainerId& ContainerId,
	const FName ContainerSlotId) const
{
	return ContainerId.IsValid()
		&& !ContainerSlotId.IsNone()
		&& Reservations.ContainsByPredicate(
			[&ContainerId, ContainerSlotId](const FCFInventoryReservation& Reservation)
			{
				return Reservation.IsActive()
					&& Reservation.ReservationKind == ECFInventoryReservationKind::DestinationCapacity
					&& Reservation.ContainerId == ContainerId
					&& Reservation.ContainerSlotId == ContainerSlotId;
			});
}

// [v1.0.0] 현재 다른 Transfer를 차단하는 Active Reservation 수를 반환합니다.
int32 FCFInventoryTransferLedger::GetActiveReservationCount() const
{
	// [v1.0.0] Ledger에서 확인한 Active Reservation 누적 수입니다.
	int32 ActiveReservationCount = 0;

	for (const FCFInventoryReservation& Reservation : Reservations)
	{
		if (Reservation.IsActive())
		{
			++ActiveReservationCount;
		}
	}

	return ActiveReservationCount;
}


// [v1.0.0] 지정 Reservation의 현재 상태를 반환하며 없으면 Unknown을 반환합니다.
ECFInventoryReservationState FCFInventoryTransferLedger::GetReservationState(
	const FCFInventoryReservationId& ReservationId) const
{
	// [v1.0.0] 조회 대상 ID와 일치하는 Reservation입니다.
	const FCFInventoryReservation* Reservation = Reservations.FindByPredicate(
		[&ReservationId](const FCFInventoryReservation& CandidateReservation)
		{
			return CandidateReservation.ReservationId == ReservationId;
		});
	return Reservation
		? Reservation->ReservationState
		: ECFInventoryReservationState::Unknown;
}

// [v1.0.0] 지정 Transaction의 현재 상태를 반환하며 없으면 Unknown을 반환합니다.
ECFInventoryTransactionState FCFInventoryTransferLedger::GetTransactionState(
	const FCFInventoryTransactionId& TransactionId) const
{
	// [v1.0.0] 조회 대상 ID와 일치하는 Transaction입니다.
	const FCFInventoryTransaction* Transaction = Transactions.FindByPredicate(
		[&TransactionId](const FCFInventoryTransaction& CandidateTransaction)
		{
			return CandidateTransaction.TransferRequest.TransactionId == TransactionId;
		});
	return Transaction
		? Transaction->TransactionState
		: ECFInventoryTransactionState::Unknown;
}
