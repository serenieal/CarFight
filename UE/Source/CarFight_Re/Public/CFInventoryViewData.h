// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-14
// Description: CF-FQ-035 INV-P0-05 읽기 전용 Inventory ViewData와 의미 ChangeSet 계약
// Scope: 기존 Container·Definition·Reservation을 변경하지 않고 현재 차량 UI용 Snapshot과 이전 Snapshot 대비 의미 변화를 생성합니다.
// Changelog:
// - v1.0.0: Item/Container ViewData, Snapshot Build 결과, 외부 Fitting Compatibility Hint 합성, Reservation·Location·Container 의미 ChangeSet을 최초 추가.
// Migration:
// - INV-P0-01~04 Item·Container·Transfer·Fitting Adapter 계약은 변경하지 않습니다.
// - Builder는 현재 차량 Owner의 VehicleCargo·MountedEquipment만 읽으며 다른 Owner의 Item을 UI Snapshot에 포함하지 않습니다.
// - CompatibilityHint는 Fitting이 외부에서 제공한 값만 합성하고 Inventory가 Mount 호환성을 계산하지 않습니다.
// - Transaction 완료·실패 이벤트는 Snapshot diff로 추측하지 않고 후속 Field Fitting Coordinator의 명시적 Operation Result가 소유합니다.
// - Blueprint/UMG는 이 ViewData를 표시할 수 있지만 Container Entry·Reservation·Transaction을 직접 수정하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFInventoryContainer.h"
#include "CFInventoryItemData.h"
#include "CFInventoryTransfer.h"
#include "CFInventoryViewData.generated.h"

/**
 * Inventory 읽기 전용 Snapshot 생성 결과 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryViewBuildStatus : uint8
{
	None UMETA(DisplayName="결과 없음 (None)"),
	Success UMETA(DisplayName="성공 (Success)"),
	InvalidAccessContext UMETA(DisplayName="접근 Context 무효 (Invalid Access Context)"),
	InvalidContainerSet UMETA(DisplayName="Container 집합 무효 (Invalid Container Set)"),
	DefinitionMissing UMETA(DisplayName="Definition 없음 (Definition Missing)"),
	DefinitionDuplicate UMETA(DisplayName="Definition 중복 (Definition Duplicate)"),
	DefinitionContractInvalid UMETA(DisplayName="Definition 계약 무효 (Definition Contract Invalid)"),
	ItemDefinitionMismatch UMETA(DisplayName="Item·Definition 불일치 (Item Definition Mismatch)")
};

/**
 * Inventory Snapshot 사이에서 UI 소비자가 알아야 하는 의미 변화 종류입니다.
 */
UENUM(BlueprintType)
enum class ECFInventoryViewChangeType : uint8
{
	ItemAdded UMETA(DisplayName="아이템 추가 (Item Added)"),
	ItemRemoved UMETA(DisplayName="아이템 제거 (Item Removed)"),
	ItemLocationChanged UMETA(DisplayName="아이템 위치 변경 (Item Location Changed)"),
	ItemReservationChanged UMETA(DisplayName="아이템 예약 변경 (Item Reservation Changed)"),
	ItemPresentationChanged UMETA(DisplayName="아이템 표시 정보 변경 (Item Presentation Changed)"),
	ContainerChanged UMETA(DisplayName="Container 변경 (Container Changed)")
};

/**
 * 외부 Fitting이 특정 Item Instance에 제공하는 UI용 호환성 힌트입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryCompatibilityHint
{
	GENERATED_BODY()

	// [v1.0.0] 어느 실제 Item Instance에 대한 힌트인지 반환할 안정적인 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Compatibility", meta=(DisplayName="아이템 인스턴스 ID (ItemInstanceId)", ToolTip="이 호환성 힌트를 적용할 실제 Inventory Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] Fitting이 계산해 Inventory ViewData에 합성할 사용자 표시용 힌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Compatibility", meta=(DisplayName="피팅 호환성 힌트 (CompatibilityHint)", ToolTip="Fitting이 계산한 읽기 전용 호환성 안내입니다. Inventory는 이 문자열을 계산하거나 해석하지 않습니다."))
	FText CompatibilityHint;
};

/**
 * Inventory UI 한 행이 표시할 실제 Item Instance의 읽기 전용 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryItemViewData
{
	GENERATED_BODY()

	// [v1.0.0] Item ID, Definition, 소유 위치를 포함한 읽기 전용 행 계약이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 같은 Item Instance인지 비교할 안정적인 실제 소유 Item ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Item", meta=(DisplayName="아이템 인스턴스 ID (ItemInstanceId)", ToolTip="같은 Definition을 가진 여러 아이템도 구분하는 실제 소유 Item Instance ID입니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] 이 행이 표시하는 정적 Inventory Definition의 Primary Asset ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Item", meta=(DisplayName="아이템 Definition ID (ItemDefinitionId)", ToolTip="DisplayName과 ItemDomain을 해석한 Inventory Item Definition ID입니다."))
	FPrimaryAssetId ItemDefinitionId;

	// [v1.0.0] Inventory Definition에서 읽은 사용자 표시용 아이템 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Item", meta=(DisplayName="아이템 표시 이름 (DisplayName)", ToolTip="Inventory Item Definition의 DisplayName을 그대로 사용하는 사용자 표시 이름입니다."))
	FText DisplayName;

	// [v1.0.0] Equipment 또는 Defense 등 Inventory Definition이 제공하는 게임 도메인 종류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Item", meta=(DisplayName="아이템 도메인 (ItemDomain)", ToolTip="Inventory Definition이 연결하는 Equipment 또는 Defense 등 강타입 도메인 종류입니다."))
	ECFInventoryItemDomain ItemDomain = ECFInventoryItemDomain::Unknown;

	// [v1.0.0] 실제 Item Instance의 현재 수량입니다. 현재 Equipment·Defense P0는 1입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Item", meta=(DisplayName="수량 (Quantity)", ToolTip="실제 Item Instance가 보유한 수량입니다. 현재 Equipment와 Defense P0는 항상 1입니다."))
	int32 Quantity = 0;

	// [v1.0.0] 현재 실제 Item이 속한 Owner·Container·ContainerType 참조입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Location", meta=(DisplayName="Inventory Container 참조 (ContainerRef)", ToolTip="이 Item Instance가 현재 실제로 속한 Owner, Container ID와 Container 종류입니다."))
	FCFInventoryContainerRef ContainerRef;

	// [v1.0.0] VehicleCargo 슬롯 또는 MountedEquipment MountProfileId 위치 키입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Location", meta=(DisplayName="Container 슬롯 ID (ContainerSlotId)", ToolTip="VehicleCargo 슬롯 또는 MountedEquipment의 MountProfileId로 사용하는 현재 소유 위치 키입니다."))
	FName ContainerSlotId = NAME_None;

	// [v1.0.0] Active Item Reservation이 있어 다른 Transfer가 현재 Item을 사용할 수 없는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|State", meta=(DisplayName="예약 중 (bIsReserved)", ToolTip="True이면 현재 Item Instance가 Active Inventory Reservation으로 잠겨 있습니다."))
	bool bIsReserved = false;

	// [v1.0.0] 현재 AccessContext에서 이 Item의 Container가 접근 가능한지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|State", meta=(DisplayName="접근 가능 (bIsAccessible)", ToolTip="True이면 현재 차량 AccessContext에서 이 Item이 속한 Container에 접근할 수 있습니다."))
	bool bIsAccessible = false;

	// [v1.0.0] Inventory 밖의 Fitting이 계산해 제공한 선택적 호환성 안내입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Compatibility", meta=(DisplayName="피팅 호환성 힌트 (CompatibilityHint)", ToolTip="Fitting이 외부에서 계산해 전달한 읽기 전용 힌트입니다. Inventory는 Mount 호환성을 자체 계산하지 않습니다."))
	FText CompatibilityHint;
};

/**
 * 현재 차량에서 UI가 표시할 Container 한 개의 읽기 전용 요약입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryContainerViewData
{
	GENERATED_BODY()

	// [v1.0.0] Container 참조와 슬롯 수가 읽기 전용 표시 계약으로 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 현재 차량에서 조회된 Container의 Owner·ID·종류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Container", meta=(DisplayName="Inventory Container 참조 (ContainerRef)", ToolTip="현재 차량에서 UI가 표시하는 VehicleCargo 또는 MountedEquipment Container 참조입니다."))
	FCFInventoryContainerRef ContainerRef;

	// [v1.0.0] 현재 AccessContext에서 이 Container에 접근할 수 있는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Container", meta=(DisplayName="접근 가능 (bIsAccessible)", ToolTip="현재 차량의 AccessContext에서 이 Container를 사용할 수 있으면 True입니다."))
	bool bIsAccessible = false;

	// [v1.0.0] 현재 Container가 소유한 실제 Item Entry 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Container", meta=(DisplayName="현재 사용 슬롯 수 (OccupiedSlotCount)", ToolTip="현재 Container가 실제로 소유한 Item Entry 수입니다."))
	int32 OccupiedSlotCount = 0;

	// [v1.0.0] 현재 Container의 MaximumSlotCount에서 Entry 수를 뺀 남은 슬롯 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Container", meta=(DisplayName="현재 남은 슬롯 수 (RemainingSlotCount)", ToolTip="현재 Container에서 추가 Item을 수용할 수 있는 남은 슬롯 수입니다."))
	int32 RemainingSlotCount = 0;
};

/**
 * 현재 차량 Inventory의 결정론적 읽기 전용 UI Snapshot입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryViewSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] 현재 차량 Owner와 모든 Container·Item ViewData가 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] 이 Snapshot이 설명하는 현재 차량 Inventory Owner ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Snapshot", meta=(DisplayName="현재 차량 Inventory 소유자 ID (CurrentVehicleOwnerId)", ToolTip="이 읽기 전용 Snapshot이 설명하는 현재 차량의 Inventory Owner ID입니다."))
	FCFInventoryOwnerId CurrentVehicleOwnerId;

	// [v1.0.0] VehicleCargo → MountedEquipment 순으로 정렬된 현재 차량 Container 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Snapshot", meta=(DisplayName="Container ViewData 목록 (Containers)", ToolTip="현재 차량의 VehicleCargo와 MountedEquipment를 고정 순서로 제공하는 읽기 전용 Container 목록입니다."))
	TArray<FCFInventoryContainerViewData> Containers;

	// [v1.0.0] Container 종류·Slot ID·ItemInstanceId 순으로 결정론적으로 정렬된 현재 차량 Item 행입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Snapshot", meta=(DisplayName="Item ViewData 목록 (Items)", ToolTip="UI가 Inventory 내부 배열을 직접 조회하지 않고 표시할 수 있는 결정론적 Item 행 목록입니다."))
	TArray<FCFInventoryItemViewData> Items;
};

/**
 * Inventory ViewData Snapshot 생성의 명시적 성공·실패 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryViewBuildResult
{
	GENERATED_BODY()

	// [v1.0.0] ViewData Snapshot을 UI에 사용할 수 있는 정상 결과인지 반환합니다.
	bool IsSuccessful() const;

	// [v1.0.0] Snapshot 생성의 최종 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Result", meta=(DisplayName="ViewData 생성 상태 (BuildStatus)", ToolTip="읽기 전용 Inventory Snapshot 생성 성공 또는 명시적인 실패 원인을 나타냅니다."))
	ECFInventoryViewBuildStatus BuildStatus = ECFInventoryViewBuildStatus::None;

	// [v1.0.0] 실패가 특정 Item Instance의 Definition 해석과 관련될 때 해당 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Result", meta=(DisplayName="실패 Item Instance ID (FailureItemInstanceId)", ToolTip="Definition 누락·중복·계약 위반 등 Snapshot 생성 실패와 관련된 실제 Item Instance ID입니다."))
	FCFItemInstanceId FailureItemInstanceId;

	// [v1.0.0] 성공했을 때 UI가 소비할 현재 차량의 결정론적 읽기 전용 Snapshot입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Result", meta=(DisplayName="Inventory View Snapshot (Snapshot)", ToolTip="기존 Inventory 상태를 변경하지 않고 생성한 현재 차량의 읽기 전용 UI Snapshot입니다."))
	FCFInventoryViewSnapshot Snapshot;
};

/**
 * 이전/현재 Inventory Snapshot 사이의 의미 변화 한 건입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryViewChange
{
	GENERATED_BODY()

	// [v1.0.0] 이 변화가 설명하는 의미 변화 종류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Change", meta=(DisplayName="Inventory 변화 종류 (ChangeType)", ToolTip="Item 추가·제거·위치·예약·표시 또는 Container 요약 변화 중 하나입니다."))
	ECFInventoryViewChangeType ChangeType = ECFInventoryViewChangeType::ContainerChanged;

	// [v1.0.0] Item 관련 변화일 때 해당 실제 Item Instance ID이며 Container 전용 변화에서는 무효입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Change", meta=(DisplayName="아이템 인스턴스 ID (ItemInstanceId)", ToolTip="Item 관련 변화가 설명하는 실제 Item Instance ID입니다. Container 전용 변화에서는 비어 있습니다."))
	FCFItemInstanceId ItemInstanceId;

	// [v1.0.0] Container 관련 변화 또는 Item의 현재/이전 위치를 대표하는 Container ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Change", meta=(DisplayName="Inventory Container ID (ContainerId)", ToolTip="이 변화와 직접 관련된 Inventory Container ID입니다."))
	FCFInventoryContainerId ContainerId;
};

/**
 * 이전 Snapshot에서 현재 Snapshot으로 바뀐 모든 의미 변화 목록입니다.
 */
USTRUCT(BlueprintType)
struct FCFInventoryViewChangeSet
{
	GENERATED_BODY()

	// [v1.0.0] UI를 갱신할 의미 변화가 하나 이상 있는지 반환합니다.
	bool HasChanges() const { return Changes.Num() > 0; }

	// [v1.0.0] 결정론적 순서로 정렬된 Inventory 의미 변화 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Inventory|ViewData|Change", meta=(DisplayName="Inventory 변화 목록 (Changes)", ToolTip="이전 Snapshot과 현재 Snapshot의 차이를 Item·Container 의미 단위로 나눈 읽기 전용 변화 목록입니다."))
	TArray<FCFInventoryViewChange> Changes;
};

/**
 * 기존 Inventory 상태를 변경하지 않고 UI용 Snapshot과 의미 ChangeSet을 생성하는 순수 Builder입니다.
 */
struct CARFIGHT_RE_API FCFInventoryViewBuilder
{
	// [v1.0.0] 현재 차량의 접근 가능한 P0 Container와 실제 Item을 결정론적 읽기 전용 Snapshot으로 변환합니다.
	static FCFInventoryViewBuildResult BuildSnapshot(
		const TArray<FCFInventoryContainerState>& Containers,
		const TArray<UCFInventoryItemData*>& ItemDefinitions,
		const FCFInventoryTransferLedger& TransferLedger,
		const FCFInventoryAccessContext& AccessContext,
		const TArray<FCFInventoryCompatibilityHint>& CompatibilityHints = TArray<FCFInventoryCompatibilityHint>());

	// [v1.0.0] 이전/현재 Snapshot의 Item 위치·예약·표시와 Container 요약 차이를 의미 ChangeSet으로 계산합니다.
	static FCFInventoryViewChangeSet DiffSnapshots(
		const FCFInventoryViewSnapshot& PreviousSnapshot,
		const FCFInventoryViewSnapshot& CurrentSnapshot);
};
