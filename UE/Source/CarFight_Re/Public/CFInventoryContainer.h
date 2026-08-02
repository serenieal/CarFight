// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-02 인벤토리 Container와 Access Query 공용 타입
// Scope: VehicleCargo·MountedEquipment 소유 위치, 슬롯·선택적 질량 Capacity와 Pawn 없는 접근·위치 조회 계약을 제공합니다.
// Changelog:
// - v1.0.0: OwnerId, ContainerId, ContainerRef, ItemLocation, ContainerState, Capacity와 Access Query 계약을 최초 추가.
// Migration:
// - INV-P0-01의 ItemInstanceId, ItemDefinitionId와 Quantity 1 계약은 변경하지 않는다.
// - 질량은 Domain DataAsset 원본을 복제하지 않고 호출자가 해석한 현재·요청 질량을 Capacity Query 입력으로 전달한다.
// - Reservation, Atomic Transfer, Fitting Runtime과 UI 변경 API는 INV-P0-03 이후 별도 타입으로 추가한다.

#pragma once

#include "CoreMinimal.h"
#include "CFInventoryTypes.h"
#include "CFInventoryContainer.generated.h"

/**
 * P0 인벤토리가 지원하는 차량 소유 Container 종류입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryContainerType : uint8
{
	Unknown UMETA(DisplayName="알 수 없음 (Unknown)"),
	VehicleCargo UMETA(DisplayName="차량 화물칸 (Vehicle Cargo)"),
	MountedEquipment UMETA(DisplayName="장착 장비 (Mounted Equipment)")
};

/**
 * Container Capacity Query 결과 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryCapacityState : uint8
{
	Available UMETA(DisplayName="수용 가능 (Available)"),
	SlotLimitExceeded UMETA(DisplayName="슬롯 한도 초과 (Slot Limit Exceeded)"),
	MassLimitExceeded UMETA(DisplayName="질량 한도 초과 (Mass Limit Exceeded)"),
	InvalidContainer UMETA(DisplayName="Container 무효 (Invalid Container)"),
	InvalidRequest UMETA(DisplayName="요청 무효 (Invalid Request)")
};

/**
 * 현재 차량 기준 Container 접근 조회 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryAccessState : uint8
{
	Missing UMETA(DisplayName="없음 (Missing)"),
	Accessible UMETA(DisplayName="접근 가능 (Accessible)"),
	Unavailable UMETA(DisplayName="접근 불가 (Unavailable)"),
	InvalidQuery UMETA(DisplayName="조회 무효 (Invalid Query)")
};

/**
 * ItemInstanceId 소유 위치 조회 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryLocationState : uint8
{
	Missing UMETA(DisplayName="없음 (Missing)"),
	Found UMETA(DisplayName="발견 (Found)"),
	Duplicate UMETA(DisplayName="중복 소유 (Duplicate)"),
	InvalidQuery UMETA(DisplayName="조회 무효 (Invalid Query)")
};

/**
 * 차량 또는 미래 Storage 소유 주체를 안정적으로 식별하는 직렬화 가능한 ID입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryOwnerId
{
	GENERATED_BODY()

	// [v1.0.0] 새로운 고유 Inventory Owner ID를 생성합니다.
	static FCFInventoryOwnerId CreateNew();

	// [v1.0.0] 기존 Guid를 보존하는 Inventory Owner ID를 생성합니다.
	static FCFInventoryOwnerId FromGuid(const FGuid& InGuid);

	// [v1.0.0] 유효한 Owner ID인지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 두 Owner ID가 같은 소유 주체인지 비교합니다.
	bool operator==(const FCFInventoryOwnerId& Other) const;

	// [v1.0.0] 두 Owner ID가 다른 소유 주체인지 비교합니다.
	bool operator!=(const FCFInventoryOwnerId& Other) const;

	// [v1.0.0] TSet·TMap에서 사용할 Owner ID 해시 값을 반환합니다.
	friend uint32 GetTypeHash(const FCFInventoryOwnerId& OwnerId)
	{
		return GetTypeHash(OwnerId.Value);
	}

	// [v1.0.0] 차량 또는 Storage 소유 주체를 식별하는 Guid입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Identity", meta=(DisplayName="Inventory 소유자 Guid (Value)", ToolTip="VehicleCargo와 MountedEquipment가 같은 차량 소유 주체에 속하는지 식별하는 안정적인 Guid입니다."))
	FGuid Value;
};

/**
 * 인벤토리 Container 한 개를 안정적으로 식별하는 직렬화 가능한 ID입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryContainerId
{
	GENERATED_BODY()

	// [v1.0.0] 새로운 고유 Inventory Container ID를 생성합니다.
	static FCFInventoryContainerId CreateNew();

	// [v1.0.0] 기존 Guid를 보존하는 Inventory Container ID를 생성합니다.
	static FCFInventoryContainerId FromGuid(const FGuid& InGuid);

	// [v1.0.0] 유효한 Container ID인지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 두 Container ID가 같은 Container인지 비교합니다.
	bool operator==(const FCFInventoryContainerId& Other) const;

	// [v1.0.0] 두 Container ID가 다른 Container인지 비교합니다.
	bool operator!=(const FCFInventoryContainerId& Other) const;

	// [v1.0.0] TSet·TMap에서 사용할 Container ID 해시 값을 반환합니다.
	friend uint32 GetTypeHash(const FCFInventoryContainerId& ContainerId)
	{
		return GetTypeHash(ContainerId.Value);
	}

	// [v1.0.0] Inventory Container 한 개를 식별하는 Guid입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Identity", meta=(DisplayName="Inventory Container Guid (Value)", ToolTip="VehicleCargo 또는 MountedEquipment Container 한 개를 식별하는 안정적인 Guid입니다."))
	FGuid Value;
};

/**
 * Container의 소유자, 고유 ID와 종류를 함께 보존하는 참조입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryContainerRef
{
	GENERATED_BODY()

	// [v1.0.0] Owner, Container ID와 P0 Container 종류가 모두 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 이 Container를 소유하는 차량 또는 Storage의 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Identity", meta=(DisplayName="Inventory 소유자 ID (OwnerId)", ToolTip="이 Container를 소유하는 차량 또는 미래 Storage의 안정적인 ID입니다."))
	FCFInventoryOwnerId OwnerId;

	// [v1.0.0] 이 Container 한 개의 고유 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Identity", meta=(DisplayName="Inventory Container ID (ContainerId)", ToolTip="이 VehicleCargo 또는 MountedEquipment Container 한 개의 안정적인 ID입니다."))
	FCFInventoryContainerId ContainerId;

	// [v1.0.0] VehicleCargo 또는 MountedEquipment 중 이 Container의 종류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Identity", meta=(DisplayName="Inventory Container 종류 (ContainerType)", ToolTip="P0에서 이 Container가 차량 화물칸인지 장착 장비 소유 위치인지 나타냅니다."))
	ECFInventoryContainerType ContainerType = ECFInventoryContainerType::Unknown;
};

/**
 * 실제 ItemInstance가 Container의 어느 슬롯에 존재하는지 나타냅니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryItemLocation
{
	GENERATED_BODY()

	// [v1.0.0] Item, Container와 Slot 식별자가 모두 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 이 위치에 존재하는 실제 Item Instance ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Location", meta=(DisplayName="아이템 인스턴스 ID (ItemInstanceId)", ToolTip="이 Container 슬롯에 실제로 존재하는 고유 Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] 이 아이템이 속한 소유자와 Container 참조입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Location", meta=(DisplayName="Inventory Container 참조 (ContainerRef)", ToolTip="이 아이템이 현재 속한 Owner, Container ID와 Container 종류입니다."))
	FCFInventoryContainerRef ContainerRef;

	// [v1.0.0] VehicleCargo 슬롯 또는 MountedEquipment의 MountProfileId로 사용할 위치 키입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Location", meta=(DisplayName="Container 슬롯 ID (ContainerSlotId)", ToolTip="VehicleCargo의 안정적인 슬롯 ID 또는 MountedEquipment에서 사용하는 MountProfileId 위치 키입니다."))
	FName ContainerSlotId = NAME_None;
};

/**
 * Container가 실제로 소유하는 Item Instance와 슬롯 위치 한 건입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryContainerEntry
{
	GENERATED_BODY()

	// [v1.0.0] Item Instance와 Container Slot ID가 모두 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] Container가 소유하는 Quantity 1 실제 아이템 인스턴스입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Entry", meta=(DisplayName="아이템 인스턴스 (ItemInstance)", ToolTip="이 Container 슬롯이 소유하는 Quantity 1 Equipment 또는 Defense 실제 인스턴스입니다."))
	FCFInventoryItemInstance ItemInstance;

	// [v1.0.0] VehicleCargo 슬롯 또는 MountedEquipment MountProfileId 위치 키입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Entry", meta=(DisplayName="Container 슬롯 ID (ContainerSlotId)", ToolTip="한 Container 안에서 중복될 수 없는 안정적인 슬롯 위치 키입니다. MountedEquipment에서는 MountProfileId를 사용합니다."))
	FName ContainerSlotId = NAME_None;
};

/**
 * Container가 허용하는 슬롯 수와 선택적 질량 한도 설정입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryContainerCapacity
{
	GENERATED_BODY()

	// [v1.0.0] 슬롯 수와 선택적 질량 한도 설정이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] Container가 동시에 소유할 수 있는 최대 Entry 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(ClampMin="1", DisplayName="최대 슬롯 수 (MaximumSlotCount)", ToolTip="VehicleCargo 또는 MountedEquipment Container가 동시에 소유할 수 있는 최대 Item Entry 수입니다."))
	int32 MaximumSlotCount = 1;

	// [v1.0.0] True이면 슬롯 수와 함께 질량 Capacity Query를 적용합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(DisplayName="질량 한도 사용 (bUseMassLimit)", ToolTip="True이면 호출자가 Domain DataAsset에서 해석한 현재·요청 질량을 MaximumMassKg와 비교합니다."))
	bool bUseMassLimit = false;

	// [v1.0.0] 질량 한도를 사용할 때 Container가 허용하는 최대 해석 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(ClampMin="0.0", Units="kg", EditCondition="bUseMassLimit", DisplayName="최대 질량 kg (MaximumMassKg)", ToolTip="Inventory에 도메인 질량을 복제하지 않고 Capacity Query에서만 비교할 최대 운반 질량입니다."))
	float MaximumMassKg = 0.0f;
};

/**
 * 현재 Capacity 사용량과 추가하려는 요청량입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryCapacityRequest
{
	GENERATED_BODY()

	// [v1.0.0] 현재 사용량과 추가 요청량이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] Container가 현재 소유하는 Entry 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(ClampMin="0", DisplayName="현재 사용 슬롯 수 (OccupiedSlotCount)", ToolTip="Container Entries에서 계산한 현재 사용 슬롯 수입니다."))
	int32 OccupiedSlotCount = 0;

	// [v1.0.0] 현재 Entries의 Domain DataAsset에서 외부 해석한 총질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(ClampMin="0.0", Units="kg", DisplayName="현재 해석 질량 kg (OccupiedMassKg)", ToolTip="Domain DataAsset 원본에서 외부 해석한 현재 Container 총질량입니다. Inventory Item에 저장하거나 복제하지 않습니다."))
	float OccupiedMassKg = 0.0f;

	// [v1.0.0] 추가 요청이 소비할 슬롯 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(ClampMin="1", DisplayName="요청 슬롯 수 (RequestedSlotCount)", ToolTip="Capacity Query에서 새로 추가하려는 Entry가 소비할 슬롯 수입니다."))
	int32 RequestedSlotCount = 1;

	// [v1.0.0] 추가 요청 Item의 Domain DataAsset에서 외부 해석한 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(ClampMin="0.0", Units="kg", DisplayName="요청 해석 질량 kg (RequestedMassKg)", ToolTip="Domain DataAsset 원본에서 외부 해석한 추가 Item 질량입니다. Inventory Definition 또는 Instance에 복제하지 않습니다."))
	float RequestedMassKg = 0.0f;
};

/**
 * Slot과 선택적 Mass Capacity Query의 결정론적 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryCapacityResult
{
	GENERATED_BODY()

	// [v1.0.0] Capacity Query가 요청을 수용할 수 있는지 반환합니다.
	bool CanAccept() const;

	// [v1.0.0] Capacity Query의 최종 판정 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(DisplayName="Capacity 상태 (CapacityState)", ToolTip="수용 가능, 슬롯 초과, 질량 초과 또는 무효 요청 중 Capacity Query 결과입니다."))
	ECFInventoryCapacityState CapacityState = ECFInventoryCapacityState::InvalidContainer;

	// [v1.0.0] 요청을 적용하기 전 남아 있는 슬롯 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(DisplayName="현재 남은 슬롯 수 (RemainingSlotCount)", ToolTip="현재 OccupiedSlotCount를 기준으로 요청 적용 전 남아 있는 슬롯 수입니다."))
	int32 RemainingSlotCount = 0;

	// [v1.0.0] 요청을 적용한 뒤 남게 되는 슬롯 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(DisplayName="요청 후 남은 슬롯 수 (ProjectedRemainingSlotCount)", ToolTip="요청을 수용했을 때 남게 되는 슬롯 수입니다. 초과 상태에서는 0으로 제한됩니다."))
	int32 ProjectedRemainingSlotCount = 0;

	// [v1.0.0] 질량 한도 사용 시 요청 적용 전 남아 있는 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(Units="kg", DisplayName="현재 남은 질량 kg (RemainingMassKg)", ToolTip="질량 한도 사용 시 현재 해석 질량을 기준으로 남은 Capacity입니다."))
	float RemainingMassKg = 0.0f;

	// [v1.0.0] 질량 한도 사용 시 요청을 적용한 뒤 남게 되는 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container|Capacity", meta=(Units="kg", DisplayName="요청 후 남은 질량 kg (ProjectedRemainingMassKg)", ToolTip="질량 요청을 수용했을 때 남게 되는 Capacity입니다. 초과 상태에서는 0으로 제한됩니다."))
	float ProjectedRemainingMassKg = 0.0f;
};

/**
 * VehicleCargo 또는 MountedEquipment 한 개의 순수 소유 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryContainerState
{
	GENERATED_BODY()

	// [v1.0.0] Container ID, Capacity, Entry와 중복 불변식을 검사합니다.
	bool ValidateContainerContract(TArray<FText>& OutValidationErrors) const;

	// [v1.0.0] 현재 Entry 수와 외부 해석 질량으로 추가 Capacity를 평가합니다.
	FCFInventoryCapacityResult EvaluateCapacity(float OccupiedMassKg, int32 RequestedSlotCount = 1, float RequestedMassKg = 0.0f) const;

	// [v1.0.0] 지정 ItemInstanceId가 이 Container에 존재하면 완전한 소유 위치를 반환합니다.
	bool FindItemLocation(const FCFItemInstanceId& ItemInstanceId, FCFInventoryItemLocation& OutLocation) const;

	// [v1.0.0] 현재 Container가 소유하는 Entry 수를 반환합니다.
	int32 GetOccupiedSlotCount() const;

	// [v1.0.0] 이 Container의 소유자, ID와 종류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container", meta=(DisplayName="Inventory Container 참조 (ContainerRef)", ToolTip="VehicleCargo 또는 MountedEquipment의 Owner, Container ID와 종류입니다."))
	FCFInventoryContainerRef ContainerRef;

	// [v1.0.0] 이 Container가 허용하는 슬롯 수와 선택적 질량 한도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container", meta=(DisplayName="Inventory Container Capacity (Capacity)", ToolTip="이 Container의 최대 Entry 수와 선택적 외부 해석 질량 한도입니다."))
	FCFInventoryContainerCapacity Capacity;

	// [v1.0.0] 이 Container가 현재 소유하는 Item Instance와 슬롯 위치 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Container", meta=(DisplayName="Inventory Container Entry 목록 (Entries)", ToolTip="이 Container가 현재 소유하는 Quantity 1 Item Instance와 안정적인 ContainerSlotId 목록입니다."))
	TArray<FCFInventoryContainerEntry> Entries;
};

/**
 * 현재 차량에서 접근을 허용할 P0 Container 종류를 지정하는 순수 조회 Context입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryAccessContext
{
	GENERATED_BODY()

	// [v1.0.0] 현재 차량 Owner ID가 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 현재 플레이어가 사용 중인 차량의 Inventory Owner ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="현재 차량 Inventory 소유자 ID (CurrentVehicleOwnerId)", ToolTip="Access Query가 VehicleCargo와 MountedEquipment를 찾을 현재 차량의 안정적인 Owner ID입니다."))
	FCFInventoryOwnerId CurrentVehicleOwnerId;

	// [v1.0.0] 현재 차량 VehicleCargo를 접근 가능한 결과로 반환할지 결정합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="차량 화물칸 접근 허용 (bAllowVehicleCargo)", ToolTip="True이면 현재 차량의 VehicleCargo를 접근 가능한 Container로 조회할 수 있습니다."))
	bool bAllowVehicleCargo = true;

	// [v1.0.0] 현재 차량 MountedEquipment를 접근 가능한 결과로 반환할지 결정합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="장착 장비 접근 허용 (bAllowMountedEquipment)", ToolTip="True이면 현재 차량의 MountedEquipment 소유 상태를 접근 가능한 Container로 조회할 수 있습니다."))
	bool bAllowMountedEquipment = true;
};

/**
 * 특정 현재 차량 Container 접근 조회 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryAccessResult
{
	GENERATED_BODY()

	// [v1.0.0] Query 결과가 접근 가능한 Container인지 반환합니다.
	bool IsAccessible() const;

	// [v1.0.0] 현재 차량 기준 Container 접근 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="Inventory 접근 상태 (AccessState)", ToolTip="현재 차량에서 요청 Container가 접근 가능, 접근 불가, 없음 또는 무효 조회인지 나타냅니다."))
	ECFInventoryAccessState AccessState = ECFInventoryAccessState::Missing;

	// [v1.0.0] 조회된 Container가 있을 때 해당 참조입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="조회된 Inventory Container 참조 (ContainerRef)", ToolTip="현재 차량 Owner와 요청 종류가 일치해 조회된 Container 참조입니다."))
	FCFInventoryContainerRef ContainerRef;

	// [v1.0.0] 조회된 Container의 현재 사용 슬롯 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="현재 사용 슬롯 수 (OccupiedSlotCount)", ToolTip="조회된 Container가 현재 소유하는 Entry 수입니다."))
	int32 OccupiedSlotCount = 0;

	// [v1.0.0] 조회된 Container의 현재 남은 슬롯 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="현재 남은 슬롯 수 (RemainingSlotCount)", ToolTip="조회된 Container의 MaximumSlotCount에서 현재 Entry 수를 뺀 값입니다."))
	int32 RemainingSlotCount = 0;
};

/**
 * ItemInstanceId의 단일 소유 위치 조회 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryLocationResult
{
	GENERATED_BODY()

	// [v1.0.0] 정확히 한 소유 위치가 발견됐는지 반환합니다.
	bool IsFound() const;

	// [v1.0.0] Item Instance 위치 조회 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="아이템 위치 조회 상태 (LocationState)", ToolTip="ItemInstanceId가 없음, 정확히 한 위치에서 발견, 중복 소유 또는 무효 조회인지 나타냅니다."))
	ECFInventoryLocationState LocationState = ECFInventoryLocationState::Missing;

	// [v1.0.0] 정확히 한 위치가 발견됐을 때 완전한 소유 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="Inventory 아이템 위치 (ItemLocation)", ToolTip="Owner, Container, Container 종류와 Slot ID를 포함하는 실제 Item Instance 소유 위치입니다."))
	FCFInventoryItemLocation ItemLocation;

	// [v1.0.0] Container 집합에서 같은 ItemInstanceId가 발견된 횟수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Access", meta=(DisplayName="발견 위치 수 (MatchingLocationCount)", ToolTip="정상은 1, 없음은 0이며 2 이상이면 중복 소유 불변식 위반입니다."))
	int32 MatchingLocationCount = 0;
};

/**
 * Pawn과 World 없이 Container 집합의 접근·소유 위치를 조회하는 순수 Utility입니다.
 */
struct CARFIGHT_RE_API FCFInventoryAccessQuery
{
	// [v1.0.0] Container ID, Owner별 종류와 ItemInstanceId가 전체 집합에서 중복되지 않는지 검사합니다.
	static bool ValidateContainerSet(const TArray<FCFInventoryContainerState>& Containers, TArray<FText>& OutValidationErrors);

	// [v1.0.0] 현재 차량 Owner와 Container 종류를 기준으로 접근 상태를 결정론적으로 조회합니다.
	static FCFInventoryAccessResult QueryVehicleContainer(
		const TArray<FCFInventoryContainerState>& Containers,
		const FCFInventoryAccessContext& AccessContext,
		ECFInventoryContainerType ContainerType);

	// [v1.0.0] 전체 Container 집합에서 ItemInstanceId의 정확한 단일 소유 위치를 조회합니다.
	static FCFInventoryLocationResult QueryItemLocation(
		const TArray<FCFInventoryContainerState>& Containers,
		const FCFItemInstanceId& ItemInstanceId);
};
