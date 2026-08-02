// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-01 인벤토리 Item Definition DataAsset 구현
// Scope: 안정적인 Definition PrimaryAssetId, Quantity 1 정책, Equipment·Defense 강타입 참조 검증을 구현합니다.
// Changelog:
// - v1.0.0: 공용 Definition 계약과 Equipment·Defense 타입별 검증을 최초 추가.
// Migration:
// - Inventory Definition은 Domain DataAsset의 ID·질량·전투 수치를 복제하지 않는다.
// - 실제 Inventory Definition .uasset은 후속 승인 범위에서 생성한다.

#include "CFInventoryItemData.h"

#include "CFEquipmentPresetData.h"
#include "CFVehicleDefenseData.h"

#define LOCTEXT_NAMESPACE "CFInventoryItemData"

namespace
{
	// [v1.0.0] 추상 기반 Definition이 사용하는 공용 PrimaryAssetType입니다.
	const FPrimaryAssetType InventoryItemAssetType(TEXT("CFInventoryItem"));

	// [v1.0.0] Equipment Item Definition 전용 PrimaryAssetType입니다.
	const FPrimaryAssetType EquipmentItemAssetType(TEXT("CFEquipmentItem"));

	// [v1.0.0] Defense Item Definition 전용 PrimaryAssetType입니다.
	const FPrimaryAssetType DefenseItemAssetType(TEXT("CFDefenseItem"));
}

// [v1.0.0] 명시적인 Definition ID와 도메인별 PrimaryAssetType으로 안정적인 PrimaryAssetId를 반환합니다.
FPrimaryAssetId UCFInventoryItemData::GetPrimaryAssetId() const
{
	if (ItemDefinitionId.IsNone())
	{
		return FPrimaryAssetId();
	}

	// [v1.0.0] 파생 Definition의 도메인에 따라 선택된 PrimaryAssetType입니다.
	const FPrimaryAssetType PrimaryAssetType = GetInventoryPrimaryAssetType();
	if (!PrimaryAssetType.IsValid())
	{
		return FPrimaryAssetId();
	}

	return FPrimaryAssetId(PrimaryAssetType, ItemDefinitionId);
}

// [v1.0.0] 이 Definition이 연결하는 게임 도메인 종류를 반환합니다.
ECFInventoryItemDomain UCFInventoryItemData::GetItemDomain() const
{
	return ECFInventoryItemDomain::Unknown;
}

// [v1.0.0] 이 Definition이 사용할 도메인별 PrimaryAssetType을 반환합니다.
FPrimaryAssetType UCFInventoryItemData::GetInventoryPrimaryAssetType() const
{
	return InventoryItemAssetType;
}

// [v1.0.0] Equipment와 Defense가 Quantity 1 고유 인스턴스인지 반환합니다.
bool UCFInventoryItemData::IsUniqueInstanceDefinition() const
{
	return GetItemDomain() == ECFInventoryItemDomain::Equipment
		|| GetItemDomain() == ECFInventoryItemDomain::Defense;
}

// [v1.0.0] 이 Definition에서 생성해야 하는 고정 수량을 반환합니다.
int32 UCFInventoryItemData::GetRequiredQuantity() const
{
	return IsUniqueInstanceDefinition() ? 1 : 0;
}

// [v1.0.0] ID, PrimaryAssetType과 강타입 Domain DataAsset 참조의 유효성을 검사합니다.
bool UCFInventoryItemData::ValidateItemDefinitionContract(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (ItemDefinitionId.IsNone())
	{
		OutValidationErrors.Add(LOCTEXT("MissingItemDefinitionId", "ItemDefinitionId는 None일 수 없습니다."));
	}

	if (!GetInventoryPrimaryAssetType().IsValid())
	{
		OutValidationErrors.Add(LOCTEXT("MissingPrimaryAssetType", "Inventory Item Definition의 PrimaryAssetType은 유효해야 합니다."));
	}

	if (!IsUniqueInstanceDefinition() || GetRequiredQuantity() != 1)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidUniqueQuantityContract", "Equipment와 Defense Item Definition은 Quantity 1 고유 인스턴스여야 합니다."));
	}

	ValidateDomainAssetReference(OutValidationErrors);
	return OutValidationErrors.IsEmpty();
}

#if WITH_EDITOR
// [v1.0.0] Unreal Data Validation에서 잘못된 Inventory Item Definition을 보고합니다.
EDataValidationResult UCFInventoryItemData::IsDataValid(FDataValidationContext& Context) const
{
	// [v1.0.0] 상위 PrimaryDataAsset이 반환한 기본 검증 결과입니다.
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);

	// [v1.0.0] Inventory Item Definition 계약에서 발견된 오류 목록입니다.
	TArray<FText> ValidationErrors;
	if (!ValidateItemDefinitionContract(ValidationErrors))
	{
		for (const FText& ValidationError : ValidationErrors)
		{
			Context.AddError(ValidationError);
		}

		return EDataValidationResult::Invalid;
	}

	if (ValidationResult == EDataValidationResult::NotValidated)
	{
		ValidationResult = EDataValidationResult::Valid;
	}

	return ValidationResult;
}
#endif

// [v1.0.0] 파생 Definition이 소유한 강타입 Domain DataAsset 참조를 검사합니다.
void UCFInventoryItemData::ValidateDomainAssetReference(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Add(LOCTEXT("MissingTypedDefinitionClass", "Inventory Item Definition은 Equipment 또는 Defense 타입별 클래스를 사용해야 합니다."));
}

// [v1.0.0] 이 Definition이 Equipment 도메인임을 반환합니다.
ECFInventoryItemDomain UCFEquipmentItemData::GetItemDomain() const
{
	return ECFInventoryItemDomain::Equipment;
}

// [v1.0.0] Equipment Item Definition 전용 PrimaryAssetType을 반환합니다.
FPrimaryAssetType UCFEquipmentItemData::GetInventoryPrimaryAssetType() const
{
	return EquipmentItemAssetType;
}

// [v1.0.0] EquipmentPresetData 강타입 참조가 지정되어 있는지 검사합니다.
void UCFEquipmentItemData::ValidateDomainAssetReference(TArray<FText>& OutValidationErrors) const
{
	if (!EquipmentPresetData)
	{
		OutValidationErrors.Add(LOCTEXT("MissingEquipmentPresetData", "Equipment Item Definition에는 EquipmentPresetData가 필요합니다."));
	}
}

// [v1.0.0] 이 Definition이 Defense 도메인임을 반환합니다.
ECFInventoryItemDomain UCFDefenseItemData::GetItemDomain() const
{
	return ECFInventoryItemDomain::Defense;
}

// [v1.0.0] Defense Item Definition 전용 PrimaryAssetType을 반환합니다.
FPrimaryAssetType UCFDefenseItemData::GetInventoryPrimaryAssetType() const
{
	return DefenseItemAssetType;
}

// [v1.0.0] VehicleDefenseData 강타입 참조가 지정되어 있는지 검사합니다.
void UCFDefenseItemData::ValidateDomainAssetReference(TArray<FText>& OutValidationErrors) const
{
	if (!VehicleDefenseData)
	{
		OutValidationErrors.Add(LOCTEXT("MissingVehicleDefenseData", "Defense Item Definition에는 VehicleDefenseData가 필요합니다."));
	}
}

#undef LOCTEXT_NAMESPACE
