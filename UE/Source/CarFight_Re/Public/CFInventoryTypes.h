// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-01 인벤토리 아이템 식별자와 고유 인스턴스 공용 타입
// Scope: 안정적인 ItemInstanceId, ItemDefinitionId 핸들, Equipment·Defense Quantity 1 고유 인스턴스 계약을 제공합니다.
// Changelog:
// - v1.0.0: FGuid 기반 ItemInstanceId, FPrimaryAssetId 기반 ItemHandle, Quantity 1 ItemInstance를 최초 추가.
// Migration:
// - 기존 EquipmentPresetData와 VehicleDefenseData 직접 참조 경로는 그대로 유지한다.
// - ContainerId, ReservationId, Transfer 상태는 INV-P0-02~03 전까지 이 타입에 추가하지 않는다.
// - Ammo Stack 타입은 CF-FQ-031 AmmoData 계약 이후 별도로 확장한다.

#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"
#include "CFInventoryTypes.generated.h"

class UCFInventoryItemData;

/**
 * 인벤토리 아이템 Definition이 연결하는 게임 도메인 종류입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryItemDomain : uint8
{
	Unknown UMETA(DisplayName="알 수 없음 (Unknown)"),
	Equipment UMETA(DisplayName="장비 (Equipment)"),
	Defense UMETA(DisplayName="방어 (Defense)")
};

/**
 * 실제 소유 아이템 한 개를 안정적으로 식별하는 직렬화 가능한 ID입니다.
 */
USTRUCT(BlueprintType)
struct FCFItemInstanceId
{
	GENERATED_BODY()

	// [v1.0.0] 새로운 고유 아이템 인스턴스 ID를 생성합니다.
	static FCFItemInstanceId CreateNew();

	// [v1.0.0] 기존 Guid를 보존하는 아이템 인스턴스 ID를 생성합니다.
	static FCFItemInstanceId FromGuid(const FGuid& InGuid);

	// [v1.0.0] 유효한 고유 ID가 들어 있는지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 저장·로그·Automation 비교에 사용할 Guid를 반환합니다.
	const FGuid& GetGuid() const;

	// [v1.0.0] 로그와 디버그에서 사용할 표준 Guid 문자열을 반환합니다.
	FString ToString() const;

	// [v1.0.0] 두 아이템 인스턴스 ID가 같은 실제 개체를 가리키는지 비교합니다.
	bool operator==(const FCFItemInstanceId& Other) const;

	// [v1.0.0] 두 아이템 인스턴스 ID가 서로 다른 실제 개체를 가리키는지 비교합니다.
	bool operator!=(const FCFItemInstanceId& Other) const;

	// [v1.0.0] TSet·TMap에서 사용할 안정적인 해시 값을 반환합니다.
	friend uint32 GetTypeHash(const FCFItemInstanceId& ItemInstanceId)
	{
		return GetTypeHash(ItemInstanceId.Value);
	}

	// [v1.0.0] 실제 소유 아이템 한 개를 식별하는 Guid 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Identity", meta=(DisplayName="아이템 인스턴스 Guid (Value)", ToolTip="실제 소유 아이템 한 개를 식별하는 안정적인 Guid입니다. Definition ID와 별도로 유지됩니다."))
	FGuid Value;
};

/**
 * 실제 아이템 인스턴스와 해당 정적 Definition을 함께 식별하는 핸들입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryItemHandle
{
	GENERATED_BODY()

	// [v1.0.0] 인스턴스 ID와 Definition ID가 모두 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 플레이어가 실제로 소유한 고유 개체의 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Identity", meta=(DisplayName="아이템 인스턴스 ID (ItemInstanceId)", ToolTip="같은 Definition의 아이템을 여러 개 소유해도 각각 다르게 유지되는 실제 개체 ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] 이 인스턴스가 사용하는 정적 Inventory Item Definition의 PrimaryAssetId입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Identity", meta=(DisplayName="아이템 Definition ID (ItemDefinitionId)", ToolTip="표시와 Domain DataAsset 해석에 사용할 Inventory Item Definition의 안정적인 PrimaryAssetId입니다."))
	FPrimaryAssetId ItemDefinitionId;
};

/**
 * Equipment와 Defense가 사용하는 Quantity 1의 실제 소유 아이템 인스턴스입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryItemInstance
{
	GENERATED_BODY()

	// [v1.0.0] 유효한 Definition에서 Quantity 1의 새로운 고유 아이템 인스턴스를 생성합니다.
	static FCFInventoryItemInstance CreateUniqueItem(const UCFInventoryItemData* ItemDefinition);

	// [v1.0.0] 핸들과 Quantity 1 고유 인스턴스 불변식이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 이 인스턴스가 지정 Definition과 동일한 ID를 사용하고 Quantity 1 계약을 지키는지 반환합니다.
	bool IsValidForDefinition(const UCFInventoryItemData* ItemDefinition) const;

	// [v1.0.0] 실제 인스턴스와 정적 Definition을 식별하는 핸들입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Instance", meta=(DisplayName="아이템 핸들 (ItemHandle)", ToolTip="실제 ItemInstanceId와 정적 ItemDefinitionId를 함께 보존하는 핸들입니다."))
	FCFInventoryItemHandle ItemHandle;

	// [v1.0.0] Equipment와 Defense 고유 인스턴스가 보유하는 수량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Instance", meta=(DisplayName="수량 (Quantity)", ToolTip="INV-P0-01의 Equipment와 Defense는 Stack을 허용하지 않으므로 항상 1이어야 합니다."))
	int32 Quantity = 1;
};
