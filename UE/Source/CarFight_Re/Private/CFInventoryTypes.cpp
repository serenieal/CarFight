// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-01 인벤토리 아이템 식별자와 고유 인스턴스 공용 타입 구현
// Scope: Guid 생성·복원, Handle 검증, Definition 기반 Quantity 1 인스턴스 생성을 구현합니다.
// Changelog:
// - v1.0.0: ItemInstanceId와 ItemHandle, Quantity 1 ItemInstance 구현을 최초 추가.
// Migration:
// - 인스턴스 생성은 Inventory Item Definition을 입력으로 사용하며 Domain DataAsset 자체를 소유권 ID로 사용하지 않는다.
// - Container와 Reservation 상태는 후속 단계에서 별도 구조로 추가한다.

#include "CFInventoryTypes.h"

#include "CFInventoryItemData.h"

// [v1.0.0] 새로운 고유 아이템 인스턴스 ID를 생성합니다.
FCFItemInstanceId FCFItemInstanceId::CreateNew()
{
	return FromGuid(FGuid::NewGuid());
}

// [v1.0.0] 기존 Guid를 보존하는 아이템 인스턴스 ID를 생성합니다.
FCFItemInstanceId FCFItemInstanceId::FromGuid(const FGuid& InGuid)
{
	// [v1.0.0] 입력 Guid를 그대로 보존할 아이템 인스턴스 ID입니다.
	FCFItemInstanceId ItemInstanceId;
	ItemInstanceId.Value = InGuid;
	return ItemInstanceId;
}

// [v1.0.0] 유효한 고유 ID가 들어 있는지 반환합니다.
bool FCFItemInstanceId::IsValid() const
{
	return Value.IsValid();
}

// [v1.0.0] 저장·로그·Automation 비교에 사용할 Guid를 반환합니다.
const FGuid& FCFItemInstanceId::GetGuid() const
{
	return Value;
}

// [v1.0.0] 로그와 디버그에서 사용할 표준 Guid 문자열을 반환합니다.
FString FCFItemInstanceId::ToString() const
{
	return Value.ToString(EGuidFormats::DigitsWithHyphensLower);
}

// [v1.0.0] 두 아이템 인스턴스 ID가 같은 실제 개체를 가리키는지 비교합니다.
bool FCFItemInstanceId::operator==(const FCFItemInstanceId& Other) const
{
	return Value == Other.Value;
}

// [v1.0.0] 두 아이템 인스턴스 ID가 서로 다른 실제 개체를 가리키는지 비교합니다.
bool FCFItemInstanceId::operator!=(const FCFItemInstanceId& Other) const
{
	return !(*this == Other);
}

// [v1.0.0] 인스턴스 ID와 Definition ID가 모두 유효한지 반환합니다.
bool FCFInventoryItemHandle::IsValid() const
{
	return ItemInstanceId.IsValid() && ItemDefinitionId.IsValid();
}

// [v1.0.0] 유효한 Definition에서 Quantity 1의 새로운 고유 아이템 인스턴스를 생성합니다.
FCFInventoryItemInstance FCFInventoryItemInstance::CreateUniqueItem(const UCFInventoryItemData* ItemDefinition)
{
	// [v1.0.0] 입력 Definition을 바탕으로 생성할 고유 아이템 인스턴스입니다.
	FCFInventoryItemInstance ItemInstance;

	if (!ItemDefinition)
	{
		ItemInstance.Quantity = 0;
		return ItemInstance;
	}

	// [v1.0.0] 입력 Definition의 ID와 강타입 Domain DataAsset 참조 오류를 수집할 배열입니다.
	TArray<FText> DefinitionValidationErrors;
	if (!ItemDefinition->ValidateItemDefinitionContract(DefinitionValidationErrors))
	{
		ItemInstance.Quantity = 0;
		return ItemInstance;
	}

	// [v1.0.0] 입력 Inventory Item Definition의 안정적인 PrimaryAssetId입니다.
	const FPrimaryAssetId ItemDefinitionId = ItemDefinition->GetPrimaryAssetId();
	if (!ItemDefinitionId.IsValid() || !ItemDefinition->IsUniqueInstanceDefinition())
	{
		ItemInstance.Quantity = 0;
		return ItemInstance;
	}

	ItemInstance.ItemHandle.ItemInstanceId = FCFItemInstanceId::CreateNew();
	ItemInstance.ItemHandle.ItemDefinitionId = ItemDefinitionId;
	ItemInstance.Quantity = ItemDefinition->GetRequiredQuantity();
	return ItemInstance;
}

// [v1.0.0] 핸들과 Quantity 1 고유 인스턴스 불변식이 유효한지 반환합니다.
bool FCFInventoryItemInstance::IsValid() const
{
	return ItemHandle.IsValid() && Quantity == 1;
}

// [v1.0.0] 이 인스턴스가 지정 Definition과 동일한 ID를 사용하고 Quantity 1 계약을 지키는지 반환합니다.
bool FCFInventoryItemInstance::IsValidForDefinition(const UCFInventoryItemData* ItemDefinition) const
{
	if (!ItemDefinition || !IsValid() || !ItemDefinition->IsUniqueInstanceDefinition())
	{
		return false;
	}

	// [v1.0.0] 비교 대상 Definition의 ID와 강타입 Domain DataAsset 참조 오류를 수집할 배열입니다.
	TArray<FText> DefinitionValidationErrors;
	if (!ItemDefinition->ValidateItemDefinitionContract(DefinitionValidationErrors))
	{
		return false;
	}

	return ItemHandle.ItemDefinitionId == ItemDefinition->GetPrimaryAssetId()
		&& Quantity == ItemDefinition->GetRequiredQuantity();
}
