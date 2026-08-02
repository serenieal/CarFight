// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CF-FQ-035 INV-P0-01 인벤토리 Item Definition DataAsset 클래스
// Scope: 정적 Inventory Definition과 실제 Item Instance를 분리하고 Equipment·Defense Domain DataAsset을 강타입으로 연결합니다.
// Changelog:
// - v1.0.0: 공용 InventoryItemData, EquipmentItemData, DefenseItemData와 Definition 검증 계약을 최초 추가.
// Migration:
// - 기존 EquipmentPresetData와 VehicleDefenseData는 도메인 규칙·스탯의 원본으로 유지한다.
// - 새 Inventory Item Definition은 도메인 수치를 복제하지 않고 해당 DataAsset을 강타입 참조한다.
// - 실제 .uasset 생성과 연결은 INV-P0-01 범위에 포함하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFInventoryTypes.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "CFInventoryItemData.generated.h"

class UCFEquipmentPresetData;
class UCFVehicleDefenseData;

/**
 * 인벤토리 표시·분류와 Domain DataAsset 해석을 제공하는 정적 Item Definition 기반 클래스입니다.
 */
UCLASS(Abstract, BlueprintType)
class CARFIGHT_RE_API UCFInventoryItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 명시적인 Definition ID와 도메인별 PrimaryAssetType으로 안정적인 PrimaryAssetId를 반환합니다.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// [v1.0.0] 이 Definition이 연결하는 게임 도메인 종류를 반환합니다.
	virtual ECFInventoryItemDomain GetItemDomain() const;

	// [v1.0.0] 이 Definition이 사용할 도메인별 PrimaryAssetType을 반환합니다.
	virtual FPrimaryAssetType GetInventoryPrimaryAssetType() const;

	// [v1.0.0] Equipment와 Defense가 Quantity 1 고유 인스턴스인지 반환합니다.
	bool IsUniqueInstanceDefinition() const;

	// [v1.0.0] 이 Definition에서 생성해야 하는 고정 수량을 반환합니다.
	int32 GetRequiredQuantity() const;

	// [v1.0.0] ID, PrimaryAssetType과 강타입 Domain DataAsset 참조의 유효성을 검사합니다.
	bool ValidateItemDefinitionContract(TArray<FText>& OutValidationErrors) const;

#if WITH_EDITOR
	// [v1.0.0] Unreal Data Validation에서 잘못된 Inventory Item Definition을 보고합니다.
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	// [v1.0.0] 저장과 데이터 조회에서 이 Inventory Item Definition을 식별할 안정적인 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Definition|Identity", meta=(DisplayName="아이템 Definition ID (ItemDefinitionId)", ToolTip="Inventory Item Definition의 안정적인 ID입니다. 에셋 이름이 바뀌어도 저장 데이터가 같은 정의를 식별하도록 명시적으로 유지합니다."))
	FName ItemDefinitionId = NAME_None;

	// [v1.0.0] 에디터, 디버그와 후속 UI에서 표시할 아이템 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Definition|Identity", meta=(DisplayName="아이템 표시 이름 (DisplayName)", ToolTip="에디터, 디버그와 후속 Inventory UI에서 사람이 읽을 아이템 이름입니다."))
	FText DisplayName;

protected:
	// [v1.0.0] 파생 Definition이 소유한 강타입 Domain DataAsset 참조를 검사합니다.
	virtual void ValidateDomainAssetReference(TArray<FText>& OutValidationErrors) const;
};

/**
 * EquipmentPresetData를 강타입으로 참조하는 장비 Item Definition입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFEquipmentItemData : public UCFInventoryItemData
{
	GENERATED_BODY()

public:
	// [v1.0.0] 이 Definition이 Equipment 도메인임을 반환합니다.
	virtual ECFInventoryItemDomain GetItemDomain() const override;

	// [v1.0.0] Equipment Item Definition 전용 PrimaryAssetType을 반환합니다.
	virtual FPrimaryAssetType GetInventoryPrimaryAssetType() const override;

	// [v1.0.0] 이 인벤토리 장비가 해석할 EquipmentPresetData입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Definition|Equipment", meta=(DisplayName="장비 프리셋 데이터 (EquipmentPresetData)", ToolTip="이 Inventory Item Definition이 가리키는 실제 EquipmentPresetData입니다. 무기·터렛·질량 수치는 원본 DataAsset에서 읽습니다."))
	TObjectPtr<UCFEquipmentPresetData> EquipmentPresetData = nullptr;

protected:
	// [v1.0.0] EquipmentPresetData 강타입 참조가 지정되어 있는지 검사합니다.
	virtual void ValidateDomainAssetReference(TArray<FText>& OutValidationErrors) const override;
};

/**
 * VehicleDefenseData를 강타입으로 참조하는 방어 Item Definition입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFDefenseItemData : public UCFInventoryItemData
{
	GENERATED_BODY()

public:
	// [v1.0.0] 이 Definition이 Defense 도메인임을 반환합니다.
	virtual ECFInventoryItemDomain GetItemDomain() const override;

	// [v1.0.0] Defense Item Definition 전용 PrimaryAssetType을 반환합니다.
	virtual FPrimaryAssetType GetInventoryPrimaryAssetType() const override;

	// [v1.0.0] 이 인벤토리 방어 아이템이 해석할 VehicleDefenseData입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|Definition|Defense", meta=(DisplayName="차량 방어 데이터 (VehicleDefenseData)", ToolTip="이 Inventory Item Definition이 가리키는 실제 VehicleDefenseData입니다. 쉴드·장갑·질량 수치는 원본 DataAsset에서 읽습니다."))
	TObjectPtr<UCFVehicleDefenseData> VehicleDefenseData = nullptr;

protected:
	// [v1.0.0] VehicleDefenseData 강타입 참조가 지정되어 있는지 검사합니다.
	virtual void ValidateDomainAssetReference(TArray<FText>& OutValidationErrors) const override;
};
