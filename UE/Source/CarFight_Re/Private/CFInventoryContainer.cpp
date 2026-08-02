// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-02 인벤토리 Container와 Access Query 구현
// Scope: Container 불변식, 슬롯·선택적 질량 Capacity, 현재 차량 접근과 Item 위치 조회를 Pawn 없이 구현합니다.
// Changelog:
// - v1.0.0: VehicleCargo·MountedEquipment Container Validation, Capacity와 Access Query 구현을 최초 추가.
// Migration:
// - Container Entries는 현재 소유 상태만 표현하며 이동·예약·Commit·Rollback API를 제공하지 않는다.
// - 질량 원본은 Domain DataAsset에 유지하고 Capacity는 호출자가 해석한 값만 계산한다.

#include "CFInventoryContainer.h"

#define LOCTEXT_NAMESPACE "CFInventoryContainer"

namespace
{
	// [v1.0.0] P0에서 지원하는 차량 Container 종류인지 반환합니다.
	bool IsSupportedContainerType(const ECFInventoryContainerType ContainerType)
	{
		return ContainerType == ECFInventoryContainerType::VehicleCargo
			|| ContainerType == ECFInventoryContainerType::MountedEquipment;
	}

	// [v1.0.0] 현재 Access Context가 지정 Container 종류 접근을 허용하는지 반환합니다.
	bool IsContainerTypeAllowed(
		const FCFInventoryAccessContext& AccessContext,
		const ECFInventoryContainerType ContainerType)
	{
		switch (ContainerType)
		{
		case ECFInventoryContainerType::VehicleCargo:
			return AccessContext.bAllowVehicleCargo;

		case ECFInventoryContainerType::MountedEquipment:
			return AccessContext.bAllowMountedEquipment;

		default:
			return false;
		}
	}
}

// [v1.0.0] 새로운 고유 Inventory Owner ID를 생성합니다.
FCFInventoryOwnerId FCFInventoryOwnerId::CreateNew()
{
	return FromGuid(FGuid::NewGuid());
}

// [v1.0.0] 기존 Guid를 보존하는 Inventory Owner ID를 생성합니다.
FCFInventoryOwnerId FCFInventoryOwnerId::FromGuid(const FGuid& InGuid)
{
	// [v1.0.0] 입력 Guid를 보존할 Inventory Owner ID입니다.
	FCFInventoryOwnerId OwnerId;
	OwnerId.Value = InGuid;
	return OwnerId;
}

// [v1.0.0] 유효한 Owner ID인지 반환합니다.
bool FCFInventoryOwnerId::IsValid() const
{
	return Value.IsValid();
}

// [v1.0.0] 두 Owner ID가 같은 소유 주체인지 비교합니다.
bool FCFInventoryOwnerId::operator==(const FCFInventoryOwnerId& Other) const
{
	return Value == Other.Value;
}

// [v1.0.0] 두 Owner ID가 다른 소유 주체인지 비교합니다.
bool FCFInventoryOwnerId::operator!=(const FCFInventoryOwnerId& Other) const
{
	return !(*this == Other);
}

// [v1.0.0] 새로운 고유 Inventory Container ID를 생성합니다.
FCFInventoryContainerId FCFInventoryContainerId::CreateNew()
{
	return FromGuid(FGuid::NewGuid());
}

// [v1.0.0] 기존 Guid를 보존하는 Inventory Container ID를 생성합니다.
FCFInventoryContainerId FCFInventoryContainerId::FromGuid(const FGuid& InGuid)
{
	// [v1.0.0] 입력 Guid를 보존할 Inventory Container ID입니다.
	FCFInventoryContainerId ContainerId;
	ContainerId.Value = InGuid;
	return ContainerId;
}

// [v1.0.0] 유효한 Container ID인지 반환합니다.
bool FCFInventoryContainerId::IsValid() const
{
	return Value.IsValid();
}

// [v1.0.0] 두 Container ID가 같은 Container인지 비교합니다.
bool FCFInventoryContainerId::operator==(const FCFInventoryContainerId& Other) const
{
	return Value == Other.Value;
}

// [v1.0.0] 두 Container ID가 다른 Container인지 비교합니다.
bool FCFInventoryContainerId::operator!=(const FCFInventoryContainerId& Other) const
{
	return !(*this == Other);
}

// [v1.0.0] Owner, Container ID와 P0 Container 종류가 모두 유효한지 반환합니다.
bool FCFInventoryContainerRef::IsValid() const
{
	return OwnerId.IsValid()
		&& ContainerId.IsValid()
		&& IsSupportedContainerType(ContainerType);
}

// [v1.0.0] Item, Container와 Slot 식별자가 모두 유효한지 반환합니다.
bool FCFInventoryItemLocation::IsValid() const
{
	return ItemInstanceId.IsValid()
		&& ContainerRef.IsValid()
		&& !ContainerSlotId.IsNone();
}

// [v1.0.0] Item Instance와 Container Slot ID가 모두 유효한지 반환합니다.
bool FCFInventoryContainerEntry::IsValid() const
{
	return ItemInstance.IsValid() && !ContainerSlotId.IsNone();
}

// [v1.0.0] 슬롯 수와 선택적 질량 한도 설정이 유효한지 반환합니다.
bool FCFInventoryContainerCapacity::IsValid() const
{
	if (MaximumSlotCount <= 0 || !FMath::IsFinite(MaximumMassKg) || MaximumMassKg < 0.0f)
	{
		return false;
	}

	return !bUseMassLimit || MaximumMassKg > 0.0f;
}

// [v1.0.0] 현재 사용량과 추가 요청량이 유효한지 반환합니다.
bool FCFInventoryCapacityRequest::IsValid() const
{
	return OccupiedSlotCount >= 0
		&& RequestedSlotCount > 0
		&& FMath::IsFinite(OccupiedMassKg)
		&& OccupiedMassKg >= 0.0f
		&& FMath::IsFinite(RequestedMassKg)
		&& RequestedMassKg >= 0.0f;
}

// [v1.0.0] Capacity Query가 요청을 수용할 수 있는지 반환합니다.
bool FCFInventoryCapacityResult::CanAccept() const
{
	return CapacityState == ECFInventoryCapacityState::Available;
}

// [v1.0.0] Container ID, Capacity, Entry와 중복 불변식을 검사합니다.
bool FCFInventoryContainerState::ValidateContainerContract(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (!ContainerRef.IsValid())
	{
		OutValidationErrors.Add(LOCTEXT("InvalidContainerRef", "ContainerRef의 OwnerId, ContainerId와 ContainerType은 모두 유효해야 합니다."));
	}

	if (!Capacity.IsValid())
	{
		OutValidationErrors.Add(LOCTEXT("InvalidContainerCapacity", "Container Capacity는 1개 이상의 슬롯과 유효한 선택적 질량 한도를 가져야 합니다."));
	}

	if (Capacity.IsValid() && Entries.Num() > Capacity.MaximumSlotCount)
	{
		OutValidationErrors.Add(LOCTEXT("ContainerSlotCapacityExceeded", "Container Entries 수가 MaximumSlotCount를 초과했습니다."));
	}

	// [v1.0.0] Container 내부 ItemInstanceId 중복 검사용 집합입니다.
	TSet<FCFItemInstanceId> UniqueItemInstanceIds;

	// [v1.0.0] Container 내부 ContainerSlotId 중복 검사용 집합입니다.
	TSet<FName> UniqueContainerSlotIds;

	for (const FCFInventoryContainerEntry& Entry : Entries)
	{
		if (!Entry.IsValid())
		{
			OutValidationErrors.Add(LOCTEXT("InvalidContainerEntry", "모든 Container Entry에는 유효한 Quantity 1 ItemInstance와 ContainerSlotId가 필요합니다."));
			continue;
		}

		if (UniqueItemInstanceIds.Contains(Entry.ItemInstance.ItemHandle.ItemInstanceId))
		{
			OutValidationErrors.Add(LOCTEXT("DuplicateItemInContainer", "같은 ItemInstanceId가 하나의 Container에 두 번 이상 존재합니다."));
		}
		else
		{
			UniqueItemInstanceIds.Add(Entry.ItemInstance.ItemHandle.ItemInstanceId);
		}

		if (UniqueContainerSlotIds.Contains(Entry.ContainerSlotId))
		{
			OutValidationErrors.Add(LOCTEXT("DuplicateContainerSlot", "같은 ContainerSlotId가 하나의 Container에 두 번 이상 사용됐습니다."));
		}
		else
		{
			UniqueContainerSlotIds.Add(Entry.ContainerSlotId);
		}
	}

	return OutValidationErrors.IsEmpty();
}

// [v1.0.0] 현재 Entry 수와 외부 해석 질량으로 추가 Capacity를 평가합니다.
FCFInventoryCapacityResult FCFInventoryContainerState::EvaluateCapacity(
	const float OccupiedMassKg,
	const int32 RequestedSlotCount,
	const float RequestedMassKg) const
{
	// [v1.0.0] Container 설정과 요청을 평가해 반환할 Capacity 결과입니다.
	FCFInventoryCapacityResult CapacityResult;

	// [v1.0.0] 현재 Entries와 호출자 해석 질량으로 구성한 Capacity 요청입니다.
	FCFInventoryCapacityRequest CapacityRequest;
	CapacityRequest.OccupiedSlotCount = GetOccupiedSlotCount();
	CapacityRequest.OccupiedMassKg = OccupiedMassKg;
	CapacityRequest.RequestedSlotCount = RequestedSlotCount;
	CapacityRequest.RequestedMassKg = RequestedMassKg;

	// [v1.0.0] 현재 Container 상태 자체의 불변식 오류를 수집할 배열입니다.
	TArray<FText> ContainerValidationErrors;
	if (!ValidateContainerContract(ContainerValidationErrors))
	{
		CapacityResult.CapacityState = ECFInventoryCapacityState::InvalidContainer;
		return CapacityResult;
	}

	if (!CapacityRequest.IsValid())
	{
		CapacityResult.CapacityState = ECFInventoryCapacityState::InvalidRequest;
		return CapacityResult;
	}

	CapacityResult.RemainingSlotCount = FMath::Max(0, Capacity.MaximumSlotCount - CapacityRequest.OccupiedSlotCount);

	// [v1.0.0] 요청을 적용한 뒤 예상되는 총 사용 슬롯 수입니다.
	const int32 ProjectedOccupiedSlotCount = CapacityRequest.OccupiedSlotCount + CapacityRequest.RequestedSlotCount;
	CapacityResult.ProjectedRemainingSlotCount = FMath::Max(0, Capacity.MaximumSlotCount - ProjectedOccupiedSlotCount);

	if (ProjectedOccupiedSlotCount > Capacity.MaximumSlotCount)
	{
		CapacityResult.CapacityState = ECFInventoryCapacityState::SlotLimitExceeded;
		return CapacityResult;
	}

	if (Capacity.bUseMassLimit)
	{
		CapacityResult.RemainingMassKg = FMath::Max(0.0f, Capacity.MaximumMassKg - CapacityRequest.OccupiedMassKg);

		// [v1.0.0] 요청을 적용한 뒤 예상되는 외부 해석 총질량입니다.
		const float ProjectedOccupiedMassKg = CapacityRequest.OccupiedMassKg + CapacityRequest.RequestedMassKg;
		CapacityResult.ProjectedRemainingMassKg = FMath::Max(0.0f, Capacity.MaximumMassKg - ProjectedOccupiedMassKg);

		if (ProjectedOccupiedMassKg > Capacity.MaximumMassKg)
		{
			CapacityResult.CapacityState = ECFInventoryCapacityState::MassLimitExceeded;
			return CapacityResult;
		}
	}

	CapacityResult.CapacityState = ECFInventoryCapacityState::Available;
	return CapacityResult;
}

// [v1.0.0] 지정 ItemInstanceId가 이 Container에 존재하면 완전한 소유 위치를 반환합니다.
bool FCFInventoryContainerState::FindItemLocation(
	const FCFItemInstanceId& ItemInstanceId,
	FCFInventoryItemLocation& OutLocation) const
{
	OutLocation = FCFInventoryItemLocation();

	if (!ItemInstanceId.IsValid())
	{
		return false;
	}

	for (const FCFInventoryContainerEntry& Entry : Entries)
	{
		if (Entry.IsValid() && Entry.ItemInstance.ItemHandle.ItemInstanceId == ItemInstanceId)
		{
			OutLocation.ItemInstanceId = ItemInstanceId;
			OutLocation.ContainerRef = ContainerRef;
			OutLocation.ContainerSlotId = Entry.ContainerSlotId;
			return OutLocation.IsValid();
		}
	}

	return false;
}

// [v1.0.0] 현재 Container가 소유하는 Entry 수를 반환합니다.
int32 FCFInventoryContainerState::GetOccupiedSlotCount() const
{
	return Entries.Num();
}

// [v1.0.0] 현재 차량 Owner ID가 유효한지 반환합니다.
bool FCFInventoryAccessContext::IsValid() const
{
	return CurrentVehicleOwnerId.IsValid();
}

// [v1.0.0] Query 결과가 접근 가능한 Container인지 반환합니다.
bool FCFInventoryAccessResult::IsAccessible() const
{
	return AccessState == ECFInventoryAccessState::Accessible;
}

// [v1.0.0] 정확히 한 소유 위치가 발견됐는지 반환합니다.
bool FCFInventoryLocationResult::IsFound() const
{
	return LocationState == ECFInventoryLocationState::Found
		&& MatchingLocationCount == 1
		&& ItemLocation.IsValid();
}

// [v1.0.0] Container ID, Owner별 종류와 ItemInstanceId가 전체 집합에서 중복되지 않는지 검사합니다.
bool FCFInventoryAccessQuery::ValidateContainerSet(
	const TArray<FCFInventoryContainerState>& Containers,
	TArray<FText>& OutValidationErrors)
{
	OutValidationErrors.Reset();

	// [v1.0.0] 전체 Container 집합에서 ContainerId 중복을 검사할 집합입니다.
	TSet<FCFInventoryContainerId> UniqueContainerIds;

	// [v1.0.0] 한 Owner가 같은 ContainerType을 두 개 이상 갖지 않는지 검사할 문자열 집합입니다.
	TSet<FString> UniqueOwnerContainerTypeKeys;

	// [v1.0.0] 전체 Container 집합에서 ItemInstanceId 중복 소유를 검사할 집합입니다.
	TSet<FCFItemInstanceId> UniqueItemInstanceIds;

	for (const FCFInventoryContainerState& Container : Containers)
	{
		// [v1.0.0] 개별 Container 계약 오류를 임시 수집할 배열입니다.
		TArray<FText> ContainerValidationErrors;
		if (!Container.ValidateContainerContract(ContainerValidationErrors))
		{
			OutValidationErrors.Append(ContainerValidationErrors);
		}

		if (Container.ContainerRef.ContainerId.IsValid())
		{
			if (UniqueContainerIds.Contains(Container.ContainerRef.ContainerId))
			{
				OutValidationErrors.Add(LOCTEXT("DuplicateContainerId", "같은 ContainerId가 Container 집합에 두 번 이상 존재합니다."));
			}
			else
			{
				UniqueContainerIds.Add(Container.ContainerRef.ContainerId);
			}
		}

		if (Container.ContainerRef.OwnerId.IsValid() && IsSupportedContainerType(Container.ContainerRef.ContainerType))
		{
			// [v1.0.0] Owner Guid와 ContainerType을 결합한 결정론적 중복 검사 키입니다.
			const FString OwnerContainerTypeKey = FString::Printf(
				TEXT("%s:%d"),
				*Container.ContainerRef.OwnerId.Value.ToString(EGuidFormats::Digits),
				static_cast<int32>(Container.ContainerRef.ContainerType));

			if (UniqueOwnerContainerTypeKeys.Contains(OwnerContainerTypeKey))
			{
				OutValidationErrors.Add(LOCTEXT("DuplicateOwnerContainerType", "한 Inventory Owner가 같은 ContainerType을 두 개 이상 소유할 수 없습니다."));
			}
			else
			{
				UniqueOwnerContainerTypeKeys.Add(OwnerContainerTypeKey);
			}
		}

		for (const FCFInventoryContainerEntry& Entry : Container.Entries)
		{
			if (!Entry.IsValid())
			{
				continue;
			}

			// [v1.0.0] 전체 Container에서 중복 여부를 검사할 실제 Item Instance ID입니다.
			const FCFItemInstanceId& ItemInstanceId = Entry.ItemInstance.ItemHandle.ItemInstanceId;
			if (UniqueItemInstanceIds.Contains(ItemInstanceId))
			{
				OutValidationErrors.Add(LOCTEXT("DuplicateItemOwnership", "같은 ItemInstanceId가 둘 이상의 Inventory Container 위치에 존재합니다."));
			}
			else
			{
				UniqueItemInstanceIds.Add(ItemInstanceId);
			}
		}
	}

	return OutValidationErrors.IsEmpty();
}

// [v1.0.0] 현재 차량 Owner와 Container 종류를 기준으로 접근 상태를 결정론적으로 조회합니다.
FCFInventoryAccessResult FCFInventoryAccessQuery::QueryVehicleContainer(
	const TArray<FCFInventoryContainerState>& Containers,
	const FCFInventoryAccessContext& AccessContext,
	const ECFInventoryContainerType ContainerType)
{
	// [v1.0.0] 현재 차량 Container 접근 상태를 반환할 결과입니다.
	FCFInventoryAccessResult AccessResult;

	if (!AccessContext.IsValid() || !IsSupportedContainerType(ContainerType))
	{
		AccessResult.AccessState = ECFInventoryAccessState::InvalidQuery;
		return AccessResult;
	}

	// [v1.0.0] 현재 차량 Owner와 요청 종류가 일치하는 Container 수입니다.
	int32 MatchingContainerCount = 0;

	for (const FCFInventoryContainerState& Container : Containers)
	{
		if (Container.ContainerRef.OwnerId != AccessContext.CurrentVehicleOwnerId
			|| Container.ContainerRef.ContainerType != ContainerType)
		{
			continue;
		}

		++MatchingContainerCount;
		AccessResult.ContainerRef = Container.ContainerRef;
		AccessResult.OccupiedSlotCount = Container.GetOccupiedSlotCount();
		AccessResult.RemainingSlotCount = FMath::Max(0, Container.Capacity.MaximumSlotCount - Container.GetOccupiedSlotCount());

		// [v1.0.0] 조회 대상 Container의 계약 오류를 수집할 배열입니다.
		TArray<FText> ContainerValidationErrors;
		if (!Container.ValidateContainerContract(ContainerValidationErrors))
		{
			AccessResult.AccessState = ECFInventoryAccessState::Unavailable;
			continue;
		}

		AccessResult.AccessState = IsContainerTypeAllowed(AccessContext, ContainerType)
			? ECFInventoryAccessState::Accessible
			: ECFInventoryAccessState::Unavailable;
	}

	if (MatchingContainerCount == 0)
	{
		AccessResult.AccessState = ECFInventoryAccessState::Missing;
	}
	else if (MatchingContainerCount > 1)
	{
		AccessResult.AccessState = ECFInventoryAccessState::Unavailable;
	}

	return AccessResult;
}

// [v1.0.0] 전체 Container 집합에서 ItemInstanceId의 정확한 단일 소유 위치를 조회합니다.
FCFInventoryLocationResult FCFInventoryAccessQuery::QueryItemLocation(
	const TArray<FCFInventoryContainerState>& Containers,
	const FCFItemInstanceId& ItemInstanceId)
{
	// [v1.0.0] ItemInstanceId 위치 조회 상태를 반환할 결과입니다.
	FCFInventoryLocationResult LocationResult;

	if (!ItemInstanceId.IsValid())
	{
		LocationResult.LocationState = ECFInventoryLocationState::InvalidQuery;
		return LocationResult;
	}

	for (const FCFInventoryContainerState& Container : Containers)
	{
		for (const FCFInventoryContainerEntry& Entry : Container.Entries)
		{
			if (!Entry.IsValid() || Entry.ItemInstance.ItemHandle.ItemInstanceId != ItemInstanceId)
			{
				continue;
			}

			++LocationResult.MatchingLocationCount;
			if (LocationResult.MatchingLocationCount == 1)
			{
				LocationResult.ItemLocation.ItemInstanceId = ItemInstanceId;
				LocationResult.ItemLocation.ContainerRef = Container.ContainerRef;
				LocationResult.ItemLocation.ContainerSlotId = Entry.ContainerSlotId;
			}
		}
	}

	if (LocationResult.MatchingLocationCount == 0)
	{
		LocationResult.LocationState = ECFInventoryLocationState::Missing;
	}
	else if (LocationResult.MatchingLocationCount == 1)
	{
		LocationResult.LocationState = ECFInventoryLocationState::Found;
	}
	else
	{
		LocationResult.LocationState = ECFInventoryLocationState::Duplicate;
		LocationResult.ItemLocation = FCFInventoryItemLocation();
	}

	return LocationResult;
}

#undef LOCTEXT_NAMESPACE
