// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-02
// Description: CF-FQ-035 INV-P0-04 Inventory Item Instance 기반 Fitting Adapter 공용 계약
// Scope: 실제 ItemInstance 선택의 소유·접근·예약·Definition 해석과 기존 BuildFittingSnapshot 연결 결과를 Pawn 없이 제공합니다.
// Changelog:
// - v1.0.0: Equipment·Defense Item 선택 입력, 결정론적 Binding, Adapter 상태와 순수 BuildFittingBinding Utility를 최초 추가.
// Migration:
// - INV-P0-01~03 Item·Container·Reservation·Atomic Transfer 계약과 FIT-P0-00~05 구현을 변경하지 않는다.
// - Adapter는 UCFVehicleFittingData의 Transient 후보 입력만 만들고 BuildFittingSnapshot을 호출하며 Weapon·Defense·VehicleMovement Runtime을 직접 변경하지 않는다.
// - Inventory 이동 Commit, 시간 액션, Runtime Apply, ViewData, UI, SaveGame과 Unreal Asset은 이 계약이 소유하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFInventoryTransfer.h"
#include "CFFittingTypes.h"
#include "CFInventoryFitAdapter.generated.h"

class UCFInventoryItemData;
class UCFVehicleData;

/**
 * Inventory Item Instance를 Fitting 입력으로 변환한 최종 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryFitStatus : uint8
{
	None UMETA(DisplayName="결과 없음 (None)"),
	Success UMETA(DisplayName="성공 (Success)"),
	InvalidRequest UMETA(DisplayName="요청 무효 (Invalid Request)"),
	InvalidContainerSet UMETA(DisplayName="Container 집합 무효 (Invalid Container Set)"),
	ItemMissing UMETA(DisplayName="아이템 없음 (Item Missing)"),
	ItemDuplicate UMETA(DisplayName="아이템 중복 소유 (Item Duplicate)"),
	ItemInaccessible UMETA(DisplayName="아이템 접근 불가 (Item Inaccessible)"),
	ItemReserved UMETA(DisplayName="아이템 예약 중 (Item Reserved)"),
	DefinitionMissing UMETA(DisplayName="Definition 없음 (Definition Missing)"),
	DefinitionDuplicate UMETA(DisplayName="Definition 중복 (Definition Duplicate)"),
	DefinitionContractInvalid UMETA(DisplayName="Definition 계약 무효 (Definition Contract Invalid)"),
	ItemDefinitionMismatch UMETA(DisplayName="Item·Definition 불일치 (Item Definition Mismatch)"),
	ItemDomainMismatch UMETA(DisplayName="Item 도메인 불일치 (Item Domain Mismatch)"),
	DuplicateMountSelection UMETA(DisplayName="Mount 선택 중복 (Duplicate Mount Selection)"),
	DuplicateItemSelection UMETA(DisplayName="Item 선택 중복 (Duplicate Item Selection)"),
	UnknownMountProfile UMETA(DisplayName="알 수 없는 Mount Profile (Unknown Mount Profile)"),
	FittingSnapshotInvalid UMETA(DisplayName="Fitting Snapshot 무효 (Fitting Snapshot Invalid)")
};

/**
 * 실제 Equipment Item Instance를 한 MountProfile에 연결하는 Adapter 입력입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryFitMountInput
{
	GENERATED_BODY()

	// [v1.0.0] MountProfile ID와 활성·빈 장착에 따른 ItemInstanceId 계약이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 이 실제 Item Instance를 연결할 VehicleData MountProfile ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="장착 프로파일 ID (MountProfileId)", ToolTip="VehicleData.MountProfiles에서 실제 Equipment Item Instance를 연결할 MountProfileId입니다."))
	FName MountProfileId = NAME_None;

	// [v1.0.0] 활성 장착에서 선택한 실제 Equipment Item Instance ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(EditCondition="bEnabled", DisplayName="장비 Item Instance ID (ItemInstanceId)", ToolTip="접근 가능한 Inventory Container에 존재하고 예약되지 않아야 하는 실제 Equipment Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] False이면 ItemInstance 없이 해당 MountProfile을 명시적으로 빈 장착으로 변환합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="장착 사용 (bEnabled)", ToolTip="True이면 실제 Equipment Item Instance를 선택합니다. False이면 ItemInstanceId 없이 명시적 빈 장착으로 변환합니다."))
	bool bEnabled = true;
};

/**
 * 실제 Defense Item Instance 또는 기존 방어 선택 방식을 지정하는 Adapter 입력입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryFitDefenseInput
{
	GENERATED_BODY()

	// [v1.0.0] 선택 방식과 Override ItemInstanceId 계약이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 차량 기본 방어, 방어 없음 또는 실제 Defense Item Override 중 선택 방식입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="방어 선택 방식 (SelectionMode)", ToolTip="UseVehicleDefault와 ExplicitNone은 ItemInstanceId를 사용하지 않고 Override만 실제 Defense Item Instance를 요구합니다."))
	ECFDefenseSelectionMode SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	// [v1.0.0] Override 방식에서 선택한 실제 Defense Item Instance ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(EditCondition="SelectionMode == ECFDefenseSelectionMode::Override", EditConditionHides, DisplayName="방어 Item Instance ID (ItemInstanceId)", ToolTip="접근 가능한 Inventory Container에 존재하고 예약되지 않아야 하는 실제 Defense Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;
};

/**
 * Inventory Item Instance 선택을 기존 Vehicle Fitting Snapshot 입력으로 변환하기 위한 전체 요청입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryFitRequest
{
	GENERATED_BODY()

	// [v1.0.0] Fitting ID, VehicleData, 접근 Context와 모든 선택 입력의 기본 계약이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 생성할 Transient 후보 Fitting과 Snapshot을 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="피팅 ID (FittingId)", ToolTip="Adapter가 생성하는 후보 VehicleFittingSnapshot을 식별할 ID입니다."))
	FName FittingId = NAME_None;

	// [v1.0.0] MountProfile, Hardpoint, 기본 구성과 질량 기준을 제공하는 차량 플랫폼입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="차량 데이터 (VehicleData)", ToolTip="기존 BuildFittingSnapshot이 Mount 호환과 질량을 검증할 기준 VehicleData입니다."))
	TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	// [v1.0.0] Mount 입력이 없는 VehicleData 프로파일을 해석할 기존 피팅 정책입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="누락 Mount 선택 정책 (MissingMountSelectionPolicy)", ToolTip="실제 Item Instance 입력이 없는 MountProfile을 차량 기본값, 빈 장착 또는 오류로 해석할 기존 피팅 정책입니다."))
	ECFMissingMountPolicy MissingMountSelectionPolicy = ECFMissingMountPolicy::UseVehicleDefault;

	// [v1.0.0] ItemInstanceId 기반 장비 또는 명시적 빈 장착 선택 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="Inventory Mount 선택 목록 (MountSelections)", ToolTip="실제 Equipment Item Instance를 MountProfile에 연결하거나 명시적 빈 장착을 지정하는 목록입니다."))
	TArray<FCFInventoryFitMountInput> MountSelections;

	// [v1.0.0] 차량 기본 방어, 명시적 없음 또는 실제 Defense Item Override 선택입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="Inventory 방어 선택 (DefenseSelection)", ToolTip="Override에서만 실제 Defense Item Instance를 선택하는 방어 입력입니다."))
	FCFInventoryFitDefenseInput DefenseSelection;

	// [v1.0.0] 현재 차량 Owner와 접근 가능한 VehicleCargo·MountedEquipment 종류를 지정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|Inventory|FittingAdapter|Input", meta=(DisplayName="Inventory 접근 Context (AccessContext)", ToolTip="선택한 Item Instance가 현재 차량의 접근 가능한 Container에 있는지 검증할 Context입니다."))
	FCFInventoryAccessContext AccessContext;
};

/**
 * 하나의 MountProfile과 실제 Equipment Item Instance 사이의 결정론적 Binding입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryFitMountBinding
{
	GENERATED_BODY()

	// [v1.0.0] 실제 또는 명시적 빈 장착을 연결한 MountProfile ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="장착 프로파일 ID (MountProfileId)", ToolTip="VehicleData.MountProfiles 순서로 정렬된 Binding의 MountProfileId입니다."))
	FName MountProfileId = NAME_None;

	// [v1.0.0] 이 Binding이 실제 Item Instance 장착인지 명시적 빈 장착인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="장착 사용 (bEnabled)", ToolTip="True이면 ItemInstanceId와 EquipmentPresetData가 유효하며 False이면 명시적 빈 장착 Binding입니다."))
	bool bEnabled = false;

	// [v1.0.0] 이 MountProfile에 연결된 실제 Equipment Item Instance ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="장비 Item Instance ID (ItemInstanceId)", ToolTip="같은 Definition을 가진 여러 아이템도 구분하는 실제 Equipment Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] 실제 Item Instance가 참조하는 Inventory Equipment Definition ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="아이템 Definition ID (ItemDefinitionId)", ToolTip="EquipmentPresetData를 해석한 Inventory Equipment Item Definition의 PrimaryAssetId입니다."))
	FPrimaryAssetId ItemDefinitionId;

	// [v1.0.0] 검증 시점에 Item Instance가 존재한 접근 가능한 Inventory 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="Source Inventory 위치 (SourceLocation)", ToolTip="Binding 검증 시점의 Owner, Container와 Slot을 보존한 실제 Item Instance 소유 위치입니다."))
	FCFInventoryItemLocation SourceLocation;

	// [v1.0.0] Inventory Equipment Definition에서 강타입으로 해석한 기존 피팅 입력 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="해석된 장비 프리셋 (ResolvedEquipmentPresetData)", ToolTip="Inventory Definition이 참조하며 기존 BuildFittingSnapshot에 전달된 EquipmentPresetData입니다."))
	TObjectPtr<UCFEquipmentPresetData> ResolvedEquipmentPresetData = nullptr;
};

/**
 * 방어 선택 방식과 선택적 실제 Defense Item Instance 사이의 결정론적 Binding입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryFitDefenseBinding
{
	GENERATED_BODY()

	// [v1.0.0] 기존 피팅 Snapshot에 전달한 최종 방어 선택 방식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="방어 선택 방식 (SelectionMode)", ToolTip="UseVehicleDefault, ExplicitNone 또는 실제 Defense Item Override 중 최종 Binding 방식입니다."))
	ECFDefenseSelectionMode SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	// [v1.0.0] Override에서 실제 Defense Item Binding이 존재하는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="실제 방어 Item Binding 있음 (bHasItemBinding)", ToolTip="Override 방식에서 ItemInstanceId와 VehicleDefenseData가 유효하게 해석됐을 때 True입니다."))
	bool bHasItemBinding = false;

	// [v1.0.0] Override에서 연결된 실제 Defense Item Instance ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="방어 Item Instance ID (ItemInstanceId)", ToolTip="같은 Defense Definition을 가진 여러 아이템도 구분하는 실제 Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] 실제 Defense Item Instance가 참조하는 Inventory Definition ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="아이템 Definition ID (ItemDefinitionId)", ToolTip="VehicleDefenseData를 해석한 Inventory Defense Item Definition의 PrimaryAssetId입니다."))
	FPrimaryAssetId ItemDefinitionId;

	// [v1.0.0] 검증 시점에 Defense Item Instance가 존재한 접근 가능한 Inventory 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="Source Inventory 위치 (SourceLocation)", ToolTip="Override Defense Item Binding 검증 시점의 실제 소유 위치입니다."))
	FCFInventoryItemLocation SourceLocation;

	// [v1.0.0] Inventory Defense Definition에서 강타입으로 해석한 기존 피팅 입력 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Binding", meta=(DisplayName="해석된 방어 데이터 (ResolvedDefenseData)", ToolTip="Inventory Definition이 참조하며 기존 BuildFittingSnapshot에 전달된 VehicleDefenseData입니다."))
	TObjectPtr<UCFVehicleDefenseData> ResolvedDefenseData = nullptr;
};

/**
 * Item Instance Binding과 기존 BuildFittingSnapshot 결과를 함께 보존하는 Adapter 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryFitResult
{
	GENERATED_BODY()

	// [v1.0.0] Inventory Binding과 기존 Fitting Snapshot 검증이 모두 성공했는지 반환합니다.
	bool IsSuccessful() const;

	// [v1.0.0] Inventory-to-Fitting Adapter의 최종 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Result", meta=(DisplayName="Adapter 상태 (AdapterStatus)", ToolTip="소유·접근·예약·Definition·중복 선택 또는 기존 Fitting Snapshot 검증 결과를 나타냅니다."))
	ECFInventoryFitStatus AdapterStatus = ECFInventoryFitStatus::None;

	// [v1.0.0] 실패가 특정 Item Instance와 관련될 때 해당 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Result", meta=(DisplayName="실패 Item Instance ID (FailureItemInstanceId)", ToolTip="Item 소유·접근·예약·Definition 또는 중복 선택 실패와 관련된 실제 Item Instance ID입니다."))
	FCFItemInstanceId FailureItemInstanceId;

	// [v1.0.0] 실패가 특정 MountProfile과 관련될 때 해당 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Result", meta=(DisplayName="실패 MountProfile ID (FailureMountProfileId)", ToolTip="Mount 선택 중복, 알 수 없는 프로파일 또는 해당 Item Binding 실패와 관련된 MountProfileId입니다."))
	FName FailureMountProfileId = NAME_None;

	// [v1.0.0] VehicleData.MountProfiles 순서로 생성된 실제 Item 또는 빈 장착 Binding 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Result", meta=(DisplayName="Mount Binding 목록 (MountBindings)", ToolTip="입력 배열 순서와 무관하게 VehicleData.MountProfiles 순서로 생성된 결정론적 Binding입니다."))
	TArray<FCFInventoryFitMountBinding> MountBindings;

	// [v1.0.0] 방어 선택 방식과 선택적 실제 Defense Item Instance Binding입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Result", meta=(DisplayName="Defense Binding (DefenseBinding)", ToolTip="차량 기본 방어, 명시적 없음 또는 실제 Defense Item Override를 보존합니다."))
	FCFInventoryFitDefenseBinding DefenseBinding;

	// [v1.0.0] Adapter가 기존 BuildFittingSnapshot을 호출해 얻은 호환성·질량 검증 결과입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|FittingAdapter|Result", meta=(DisplayName="차량 피팅 Snapshot (FittingSnapshot)", ToolTip="Inventory가 호환성과 질량을 재계산하지 않고 기존 UCFVehicleFittingData.BuildFittingSnapshot에서 받은 결과입니다."))
	FCFVehicleFittingSnapshot FittingSnapshot;
};

/**
 * Pawn과 Runtime 컴포넌트 없이 Inventory Item Instance 선택을 기존 Fitting Snapshot 입력으로 변환하는 순수 Adapter입니다.
 */
struct CARFIGHT_RE_API FCFInventoryFitAdapter
{
	// [v1.0.0] 실제 Item 소유·접근·예약과 Definition을 검증하고 결정론적 Binding과 기존 Fitting Snapshot을 생성합니다.
	static FCFInventoryFitResult BuildFittingBinding(
		const TArray<FCFInventoryContainerState>& Containers,
		const TArray<UCFInventoryItemData*>& ItemDefinitions,
		const FCFInventoryTransferLedger& TransferLedger,
		const FCFInventoryFitRequest& FitRequest);
};
