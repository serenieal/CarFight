# Inventory Foundation Design

- Version: 0.7.0
- Date: 2026-08-02
- Status: Contract Locked / `INV-P0-01~05` Implemented·Verified / `INV-P0-06` Coordinator Foundation Implemented·Verified / Formal FFIT Integration Pending / Asset Not Modified




- Feature: `CF-FQ-035 인벤토리 Foundation`
- Representative Plan: `Document/Plan/InventoryFoundation/InventoryFoundationPlan.md`
- Roadmap: `Document/Plan/InventoryFoundationRoadmap.md`

---

## 1. 설계 목표

인벤토리는 단순한 `TArray<DataAsset*>`가 아니라 다음 불변식을 보장해야 한다.

```text
- 실제 소유 장비는 안정적인 InstanceId를 가진다.
- 한 Instance는 정확히 한 소유 위치에만 존재한다.
- 시간 소모 액션 중에는 Item과 목적지 용량을 예약한다.
- 완료 전에는 실제 소유 위치를 변경하지 않는다.
- 완료 순간 전체 이동을 Commit하거나 전부 Rollback한다.
```

인벤토리는 게임 도메인 수치를 계산하지 않고 소유권과 이동만 책임진다.

---

## 2. 권장 공용 타입

아래 이름 중 Item Identity·Definition, Container·Access Query, Reservation·Atomic Transfer와 Fitting Inventory Adapter 범위는 `INV-P0-01~04` 공식 빌드와 Automation으로 확정했다. ViewData 이후 타입만 후속 후보 상태다.



```text
확정:
FCFItemInstanceId
FCFInventoryItemHandle
FCFInventoryItemInstance
UCFInventoryItemData
UCFEquipmentItemData
UCFDefenseItemData
FCFInventoryOwnerId
FCFInventoryContainerId
FCFInventoryContainerRef
FCFInventoryItemLocation
FCFInventoryContainerEntry
FCFInventoryContainerCapacity
FCFInventoryCapacityRequest
FCFInventoryCapacityResult
FCFInventoryContainerState
FCFInventoryAccessContext
FCFInventoryAccessResult
FCFInventoryLocationResult
FCFInventoryAccessQuery
FCFInventoryReservationId
FCFInventoryTransactionId
FCFInventoryContainerMassSnapshot
FCFInventoryTransferRequest
FCFInventoryReservation
FCFInventoryTransaction
FCFInventoryTransferResult
FCFInventoryCancelResult
FCFInventoryTransferLedger
ECFInventoryFitStatus
FCFInventoryFitMountInput
FCFInventoryFitDefenseInput
FCFInventoryFitRequest
FCFInventoryFitMountBinding
FCFInventoryFitDefenseBinding
FCFInventoryFitResult
FCFInventoryFitAdapter

INV-P0-05 확정:
FCFInventoryCompatibilityHint
FCFInventoryItemViewData
FCFInventoryContainerViewData
FCFInventoryViewSnapshot
FCFInventoryViewBuildResult
ECFInventoryViewBuildStatus
ECFInventoryViewChangeType
FCFInventoryViewChange
FCFInventoryViewChangeSet
FCFInventoryViewBuilder

INV-P0-06 Coordinator Foundation 확정:
ECFFieldFitCompletionStatus
FCFFieldFitCompletionRequest
FCFFieldFitCompletionResult
ICFFieldFitRuntimeTransaction
FCFFieldFitRuntimeAdapter
FCFFieldFitCoordinator
FCFFittingRuntimeCheckpoint

```



모든 파일명·클래스명은 32자 제한을 지킨다.

### 2.1 Item Handle

```text
FCFItemInstanceId ItemInstanceId
FPrimaryAssetId ItemDefinitionId
```

`ItemInstanceId`는 직렬화 가능한 `FGuid`를 감싸며 플레이어가 소유한 실제 개체를 식별한다.
`ItemDefinitionId`는 도메인별 PrimaryAssetType과 명시적인 Definition 이름을 결합해 표시와 Domain DataAsset 해석에 사용한다.
에셋 이름을 Item Instance 식별자로 사용하지 않으며 Definition과 Instance 수명은 분리한다.


### 2.2 Item Instance

`INV-P0-01` 확정 필드:

```text
ItemHandle
Quantity
```

`ContainerId`, `ContainerSlotId`와 `ReservationId`는 현재 Instance 구조에 선행 예약 필드로 넣지 않고 `INV-P0-02~03`의 소유 위치·예약 구조가 소유한다.
장비·방어 패키지는 `Quantity=1`인 고유 Instance로 취급하며 Definition 검증과 Instance 검증에서 이를 강제한다.
미래 Ammo·소모품 Stack은 CF-FQ-031의 실제 AmmoData 계약 이후 별도 확장한다.


P0에서 내구도·랜덤 옵션·개조 상태를 미리 넣지 않는다.

### 2.3 Definition과 Domain Asset

Inventory Definition은 도메인 데이터를 복제하지 않고 참조만 제공한다.

```text
Equipment Item
→ EquipmentPresetData

Defense Item
→ VehicleDefenseData

Ammo Item
→ 미래 AmmoData
```

`INV-P0-01`은 공용 기반과 타입별 Definition을 결합한 구조로 확정했다.

```text
UCFInventoryItemData
= 명시적 ItemDefinitionId, 도메인별 PrimaryAssetType, Quantity 1 공통 계약

UCFEquipmentItemData
= TObjectPtr<UCFEquipmentPresetData>
= PrimaryAssetType CFEquipmentItem

UCFDefenseItemData
= TObjectPtr<UCFVehicleDefenseData>
= PrimaryAssetType CFDefenseItem
```

범용 `UObject*` 또는 소프트 경로 문자열로 Domain Asset을 저장하지 않는다.
Equipment와 Defense가 같은 Definition 이름을 사용하더라도 PrimaryAssetType으로 충돌하지 않는다.
AmmoData가 아직 없으므로 임시 Ammo Item 클래스는 만들지 않았다.

### 2.4 구현·검증 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryTypes.h
- UE/Source/CarFight_Re/Private/CFInventoryTypes.cpp
- UE/Source/CarFight_Re/Public/CFInventoryItemData.h
- UE/Source/CarFight_Re/Private/CFInventoryItemData.cpp

Pawn 없는 Automation:
- CarFight.Inventory.INV_P0_01.Identity — Success
- CarFight.Inventory.INV_P0_01.Definition — Success

공식 Editor Build:
- Job 32cd34dd2424407b94c7377dd19a5fce
- Exit Code 0

전체 회귀:
- CarFight 34/34 Success
- 기존 필수 회귀 17/17 Success
```


---

## 3. Container 계약

`INV-P0-02`의 P0 Container 종류는 다음 두 개로 고정했다.

```text
VehicleCargo
MountedEquipment
```

후속 종류는 현재 enum과 Runtime에 선행 추가하지 않는다.

```text
PlayerStorage
NearbyService
WorldLoot
RewardBuffer
Vendor
```

### 3.1 소유 위치

```text
FCFInventoryContainerRef
- FCFInventoryOwnerId OwnerId
- FCFInventoryContainerId ContainerId
- ECFInventoryContainerType ContainerType

FCFInventoryContainerEntry
- FCFInventoryItemInstance ItemInstance
- FName ContainerSlotId

FCFInventoryItemLocation
- FCFItemInstanceId ItemInstanceId
- FCFInventoryContainerRef ContainerRef
- FName ContainerSlotId
```

`VehicleCargo`의 `ContainerSlotId`는 Cargo 내부의 안정적인 슬롯 키다.
`MountedEquipment`의 `ContainerSlotId`는 `MountProfileId`를 위치 키로 사용할 수 있지만 MountType·SizeLimit·Hardpoint 규칙을 복제하지 않는다.
Owner 또는 Container 위치 정보는 `FCFInventoryItemInstance` 내부에 넣지 않고 Container가 소유한다.

### 3.2 불변식

```text
- ContainerId는 전체 Container 집합에서 고유
- 한 Owner는 P0 ContainerType별로 최대 하나의 Container만 소유
- ItemInstanceId는 전체 Container 집합에서 정확히 한 위치에만 존재
- 한 Container 안에서 ContainerSlotId 중복 금지
- Entries 수는 MaximumSlotCount 이하
- 모든 Entry는 INV-P0-01 Quantity 1 유효 인스턴스
```

위반 상태는 자동 수정하지 않고 Validation과 Item Location Query에서 명시적으로 거부한다.

### 3.3 Capacity

```text
FCFInventoryContainerCapacity
- MaximumSlotCount
- bUseMassLimit
- MaximumMassKg

FCFInventoryCapacityRequest
- OccupiedSlotCount
- OccupiedMassKg
- RequestedSlotCount
- RequestedMassKg
```

슬롯 Capacity는 현재 Entry 수와 추가 요청 슬롯 수를 비교한다.
질량 Capacity는 선택 사항이며 Inventory가 장비·방어 질량을 저장하거나 재계산하지 않는다.
호출자가 `EquipmentPresetData`, `VehicleDefenseData`와 미래 `AmmoData`에서 해석한 현재·요청 질량만 Query 입력으로 전달한다.
현재 Container 계약 자체가 무효이면 Capacity Query도 `InvalidContainer`로 거부한다.

`MaximumMassKg`는 인벤토리 운반 한도이며 차량 피팅 총중량과 같은 값이 아니다.

```text
Inventory Container Mass Capacity
= 호출자가 해석한 아이템 질량을 수용할 수 있는 소유 한도

Vehicle Fitting Total Mass
= 차량에 장착·적재된 실제 구성의 물리 총중량
```

두 계산을 같은 필드나 소유 책임으로 합치지 않는다.

### 3.4 구현·검증 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryContainer.h
- UE/Source/CarFight_Re/Private/CFInventoryContainer.cpp

Pawn 없는 Automation:
- CarFight.Inventory.INV_P0_02.Container — Success
- CarFight.Inventory.INV_P0_02.AccessQuery — Success

공식 Editor Build:
- Job 36260a7a70dd48e899f27dcd038efb2b
- Exit Code 0

전체 회귀:
- CarFight 36/36 Success
- 기존 필수 회귀 17/17 Success
```


---

## 4. Mounted Equipment 경계

`MountedEquipment`는 아이템의 현재 소유 위치를 표현하지만 MountType·SizeLimit·Hardpoint 위치를 정의하지 않는다.

```text
Inventory
= ItemInstance 123이 차량 A의 MountProfile TopTurret에 소속됨

Fitting
= TopTurret가 비어 있는지, 해당 장비가 호환되는지, Snapshot이 유효한지 판정
```

MountProfileId는 Container Slot Key로 참조할 수 있지만 원본 규칙은 `VehicleData.MountProfiles`가 소유한다.

---

## 5. 접근 가능한 Inventory

필드 피팅 P0의 접근 범위는 현재 차량 Owner ID에 속한 다음 두 Container다.

```text
현재 차량 VehicleCargo
현재 차량 MountedEquipment
```

Container 접근 조회 결과:

```text
Accessible
Unavailable
Missing
InvalidQuery
```

ItemInstanceId 소유 위치 조회 결과:

```text
Found
Missing
Duplicate
InvalidQuery
```

다른 차량 Owner의 Container는 같은 배열에 있어도 현재 차량 접근 조회 결과에 포함하지 않는다.
같은 ItemInstanceId가 한 Container의 두 슬롯 또는 여러 Container에 존재하면 단일 위치로 반환하지 않고 `Duplicate`로 판정한다.
`Reserved` 상태는 Reservation이 아직 제외된 `INV-P0-02` enum에 넣지 않고 `INV-P0-03`에서 추가한다.
후속 `NearbyService`는 거리·소유권·서비스 상태를 별도 Provider가 판정한다. Inventory Foundation에 Pawn 또는 월드 거리 계산을 하드코딩하지 않는다.


---

## 6. Reservation 계약

`INV-P0-03`은 하나의 Prepared Transaction마다 다음 두 Reservation을 함께 생성한다.

```text
Item Reservation
- Source ItemInstanceId
- Source ContainerId
- Source ContainerSlotId

Destination Capacity Reservation
- Destination ContainerId
- Destination ContainerSlotId
- ReservedSlotCount = 1
- 호출자가 해석한 ReservedMassKg
```

공통 식별자와 소유 정보:

```text
FCFInventoryReservationId
FCFInventoryTransactionId
FName ActionOwnerId
```

`ActionOwnerId`는 시간 액션 또는 상위 Coordinator를 구분하는 불투명 ID다. Inventory는 액션 종류, 시간 진행과 게임 규칙을 해석하지 않는다.
자동 만료 Timer는 P0 범위에 넣지 않았다. Action 종료자는 어느 한 Reservation ID로 `CancelReservation`을 호출하거나 Transaction ID로 `RollbackTransfer`를 호출하며, 두 경로 모두 같은 Transaction의 두 Reservation을 함께 정리한다.


Reservation 상태:

```text
Active
Cancelled
Consumed
```

예약 불변식:

```text
- 같은 ItemInstanceId는 동시에 하나의 Active Item Reservation만 가짐
- 같은 Destination ContainerSlotId는 동시에 하나의 Active Capacity Reservation만 가짐
- Destination 슬롯과 선택적 외부 해석 질량은 모든 Active Reservation을 합산해 선점
- Prepare 반복 호출은 같은 Reservation ID를 반환하고 추가 Reservation을 만들지 않음
- 어느 한 Reservation ID의 CancelReservation은 같은 Transaction의 모든 Active Reservation을 Cancelled로 만들고 Transaction을 RolledBack으로 전환
- 같은 Reservation ID의 CancelReservation 반복 호출은 Cancelled → AlreadyCancelled의 멱등 결과

- Commit 성공 시 두 Reservation은 Consumed
- Rollback 시 남아 있는 Active Reservation은 모두 Cancelled
```

### 6.1 구현·검증 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryTransfer.h
- UE/Source/CarFight_Re/Private/CFInventoryTransfer.cpp

Pawn 없는 Automation:
- CarFight.Inventory.INV_P0_03.Reservation — Success / Warning 0 / Error 0
- CarFight.Inventory.INV_P0_03.AtomicTransfer — Success / Warning 0 / Error 0

공식 Editor Build:
- Job 32ca1cacb47548d4aa24ee12372baf67

- Exit Code 0

전체 회귀:
- CarFight 38/38 Success
- 기존 필수 회귀 17/17 Success
```

---

## 7. Atomic Transfer 계약

Transaction 상태:

```text
Prepared
Committed
RolledBack
```

`Failed` terminal 상태는 P0에 두지 않았다. Commit 재검증 실패는 Container와 Reservation을 변경하지 않은 `Prepared` 상태로 유지해, 상위 Coordinator가 조건을 수정해 재시도하거나 명시적으로 Rollback할 수 있게 한다.

### 7.1 Prepare

```text
- 현재 Container 집합 불변식 검증
- ItemInstanceId의 정확한 단일 Source 위치 검증
- Destination Container와 Slot 비어 있음 검증
- Item과 Destination Slot 이중 예약 거부
- 실제 Entry + 기존 Active Reservation + 새 요청의 Slot·Mass Capacity 검증
- Prepare Item, Source ContainerRef와 Destination ContainerRef Snapshot 보존
- Container 배열은 변경하지 않음
```

### 7.2 Commit

```text
- Transaction이 Prepared인지 재검증
- Item·Destination Reservation이 모두 Active이고 같은 Transaction 소유인지 검증
- Source Item의 ID·Definition·Quantity·Slot과 ContainerRef 재검증
- Destination ContainerRef·Slot·Slot Capacity·외부 해석 Mass Capacity 재검증
- 원본이 아닌 Candidate Container 복사본에서 Source 제거와 Destination 추가
- Candidate 전체 Container 불변식 검증
- 모든 검증 성공 시에만 원본 Container 배열을 한 번 교체
- Reservation은 Consumed, Transaction은 Committed로 terminal 전환
```

같은 Transaction ID의 Commit 반복 호출은 `AlreadyCommitted`를 반환하며 Container에 추가 변경을 가하지 않는다.

### 7.3 Rollback

```text
Prepared
→ Active Reservation 전체 Cancelled
→ Container 무변경
→ Transaction RolledBack

RolledBack 재호출
→ AlreadyRolledBack
→ 상태 무변경

Committed
→ CannotRollbackCommitted
```

Field Fitting에서는 Inventory Transaction과 Runtime Apply를 조율할 상위 Coordinator가 필요하다.

권장 흐름:

```text
Prepare Inventory Transaction
→ Build Candidate Fitting Snapshot
→ Prepare Runtime Apply
→ Commit Runtime Apply
→ Commit Inventory Transaction
```

Runtime Apply 이후 Inventory Commit이 실패할 수 있으므로 실제 구현에서는 양쪽 모두 사전 검증 가능한 Prepare 단계와 보상 Rollback을 제공해야 한다. 단순히 두 함수를 순서대로 호출하는 방식으로 완료 처리하지 않는다.


---

## 8. Field Fitting Adapter

`INV-P0-04`는 Saved Template의 직접 DataAsset 선택과 실제 Inventory Draft를 분리한다.

```text
Saved Template / Legacy
= UCFVehicleFittingData의 EquipmentPresetData·VehicleDefenseData 직접 선택

Inventory Draft
= FCFInventoryFitRequest의 ItemInstanceId 선택

Adapter Result
= 실제 Instance Binding + 기존 FCFVehicleFittingSnapshot
```

### 8.1 입력 계약

```text
FCFInventoryFitMountInput
- MountProfileId
- ItemInstanceId
- bEnabled

FCFInventoryFitDefenseInput
- SelectionMode
- ItemInstanceId — Override에서만 필수

FCFInventoryFitRequest
- FittingId
- VehicleData
- MissingMountSelectionPolicy
- MountSelections
- DefenseSelection
- AccessContext
```

명시적 빈 Mount는 `bEnabled=false`와 무효 ItemInstanceId를 사용한다. Defense의 `UseVehicleDefault`와 `ExplicitNone`은 ItemInstanceId를 사용하지 않는다.

### 8.2 검증 순서

```text
1. Request와 Container 집합 불변식 검증
2. MountProfileId 중복·미존재 검증
3. Mount와 Defense 전체 ItemInstanceId 중복 선택 검증
4. QueryItemLocation으로 Item 단일 소유 위치 확인
5. 현재 차량 Owner와 AccessContext의 Container 접근 가능 상태 확인
6. FCFInventoryTransferLedger.IsItemReserved로 Active Reservation 거부
7. Container Entry의 ItemHandle.ItemDefinitionId 확인
8. 단일 UCFInventoryItemData Definition 해석
9. Equipment·Defense Domain과 강타입 Definition 계약 확인
10. Transient UCFVehicleFittingData 입력 생성
11. 기존 BuildFittingSnapshot으로 Mount 호환·질량 검증
```

ItemDefinition Registry에서 같은 PrimaryAssetId가 없거나 둘 이상이면 각각 `DefinitionMissing`, `DefinitionDuplicate`로 거부한다. Equipment Mount에는 `UCFEquipmentItemData`, Defense Override에는 `UCFDefenseItemData`만 허용한다.

### 8.3 결정론적 Binding

```text
FCFInventoryFitMountBinding
- MountProfileId
- bEnabled
- ItemInstanceId
- ItemDefinitionId
- SourceLocation
- ResolvedEquipmentPresetData

FCFInventoryFitDefenseBinding
- SelectionMode
- bHasItemBinding
- ItemInstanceId
- ItemDefinitionId
- SourceLocation
- ResolvedDefenseData
```

MountBindings는 요청 배열 순서가 아니라 `VehicleData.MountProfiles` 순서로 생성한다. 같은 Definition을 가진 두 Item도 서로 다른 ItemInstanceId로 구분하며 같은 ItemInstanceId를 둘 이상의 Mount 또는 Defense에 선택할 수 없다.

### 8.4 기존 Fitting 검증 연결

Adapter는 호환성과 질량을 다시 계산하지 않는다.

```text
검증된 Inventory Binding
→ Transient UCFVehicleFittingData
→ UCFVehicleFittingData::BuildFittingSnapshot()
→ FCFVehicleFittingSnapshot
```

Snapshot이 무효이면 `FittingSnapshotInvalid`를 반환하면서 기존 ValidationIssues와 이미 해석된 Binding은 보존한다. 이 결과를 Runtime Apply나 Inventory 이동 Commit에 사용하면 안 된다.

### 8.5 비변경 경계

```text
읽기:
- Inventory Container
- Access Context
- Reservation Ledger
- Item Definition
- VehicleData
- EquipmentPresetData
- VehicleDefenseData

생성:
- Transient UCFVehicleFittingData 후보
- Binding Result
- 기존 Fitting Snapshot 결과

호출 금지:
- CommitTransfer
- VehicleWeaponComp Runtime Apply
- VehicleDefenseComp Runtime Apply
- VehicleMovement 질량·기동 변경
- Field Timed Action
```

### 8.6 구현·검증 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryFitAdapter.h
- UE/Source/CarFight_Re/Private/CFInventoryFitAdapter.cpp

Pawn 없는 Automation:
- UE/Source/CarFight_Re/Private/CFInventoryFitTests.cpp
- CarFight.Inventory.INV_P0_04.FittingAdapter — Success / Warning 0 / Error 0

공식 Editor Build:
- Job 23af46f3a13a4ba88e9878bcdc225a50
- Exit Code 0

전체 회귀:
- CarFight 42/42 Success
- 필수 회귀 20/20 Success
```


---

## 9. 읽기 전용 ViewData와 의미 ChangeSet

`INV-P0-05`는 별도 mutable Inventory UI Component나 Delegate owner를 추가하지 않고, 기존 Container·Definition·TransferLedger를 읽는 불변 Snapshot Builder로 확정했다.

```text
FCFInventoryViewBuilder::BuildSnapshot
Input:
- TArray<FCFInventoryContainerState>
- TArray<UCFInventoryItemData*>
- FCFInventoryTransferLedger
- FCFInventoryAccessContext
- optional TArray<FCFInventoryCompatibilityHint>

Output:
- FCFInventoryViewBuildResult
  - FCFInventoryViewSnapshot
    - FCFInventoryContainerViewData[]
    - FCFInventoryItemViewData[]
```

Item Row 확정 필드:

```text
ItemInstanceId
ItemDefinitionId
DisplayName
ItemDomain
Quantity
ContainerRef
ContainerSlotId
bIsReserved
bIsAccessible
CompatibilityHint
```

Container ViewData 확정 필드:

```text
ContainerRef
bIsAccessible
OccupiedSlotCount
RemainingSlotCount
```

현재 P0 Snapshot은 `AccessContext.CurrentVehicleOwnerId`의 VehicleCargo와 MountedEquipment만 대상으로 한다. 다른 Owner의 Item은 같은 Container 배열에 존재해도 현재 차량 Item 목록에 포함하지 않는다. 접근 불가인 현재 Owner Container는 Container 요약만 제공할 수 있지만 내부 Item Row는 노출하지 않는다.

`CompatibilityHint`는 Fitting이 계산한 결과를 `FCFInventoryCompatibilityHint`로 외부 공급한다. Inventory는 MountType·SizeLimit·질량 호환을 다시 계산하지 않는다.

변경 알림 계약은 mutable Delegate를 새로 소유하는 대신 이전/현재 Snapshot을 비교하는 순수 `DiffSnapshots()` 결과로 확정했다.

```text
ECFInventoryViewChangeType
- ItemAdded
- ItemRemoved
- ItemLocationChanged
- ItemReservationChanged
- ItemPresentationChanged
- ContainerChanged

FCFInventoryViewBuilder::DiffSnapshots
Previous Snapshot + Current Snapshot
→ FCFInventoryViewChangeSet
```

`PrepareTransfer`처럼 Container가 바뀌지 않고 Active Reservation만 생기는 경우 `ItemReservationChanged`만 나타난다. 실제 `CommitTransfer` 뒤에는 위치·Reservation·Source/Destination Container 요약 변화가 구분된다.

`TransactionCompleted`와 `TransactionFailed`는 Snapshot 상태만으로 안전하게 추측할 수 없으므로 M5 ChangeSet에 넣지 않는다. 후속 Field Fitting Coordinator가 Inventory Transaction과 Runtime Apply의 실제 Operation Result를 명시적으로 소유하고 UI에 전달한다.

Blueprint/UMG는 Snapshot과 ChangeSet을 표시만 하며 Container Entry, Reservation 또는 Transaction을 직접 수정하지 않는다.

### 9.1 Field Fitting completion Coordinator

`INV-P0-06`의 첫 기술 기반은 이미 완료 조건에 도달한 단일 Field Equip/Unequip에 대해 Inventory와 Fitting Runtime의 Commit 순서를 원자적으로 조율한다.

```text
FCFFieldFitCoordinator::Complete

Inventory Prepare
→ ICFFieldFitRuntimeTransaction.PrepareRuntime
→ Runtime Commit
→ Inventory Commit
```

실패 보상은 다음처럼 고정한다.

```text
Runtime Prepare/Commit 실패
→ Prepared Runtime 정리
→ Inventory Rollback

Inventory Commit 실패
→ CompensateCommittedRuntime
   = 직전 Applied Runtime Checkpoint 복원
→ Inventory Rollback
→ 부분 장착 0
```

`FCFFieldFitCompletionResult`가 Transaction 완료·실패와 각 단계 상태를 명시적으로 소유하므로 M5 Snapshot Diff는 Transaction 결과를 추측하지 않는다.

현재 실제 `FCFFieldFitRuntimeAdapter`는 `UCFVehicleFittingComp v1.3.0`의 검증 Snapshot 직접 Prepare와 `FCFFittingRuntimeCheckpoint`를 사용한다. Field Mass Reapply는 아직 소유하지 않으므로 이전 Applied Snapshot과 후보 Snapshot의 총질량이 `0.01kg` 이내로 같은 경우만 허용한다. 질량 변경 후보는 `RuntimeMassReapplyUnsupported`로 실패하고 Inventory Prepared Transaction을 Rollback한다.

이 Coordinator Foundation은 FFIT의 completion 하위 계층이다. Permission, Timed Action, Action Reservation과 일반 mass-changing Field Apply는 각각 `FFIT-P0-01`, `FFIT-P0-02`, `FFIT-P0-04`가 소유하며 이번 구현으로 완료 처리하지 않는다.

---

## 10. C++과 Blueprint 분배

### C++

```text
- Item Instance와 ID
- Container 소유권
- Capacity
- Access Query
- Reservation
- Atomic Transfer
- Commit·Rollback
- ItemInstance → Fitting Binding Adapter
- 기존 BuildFittingSnapshot 연결
- 불변식 검증
- 이벤트와 읽기 전용 ViewData

```

### Blueprint / UMG

```text
- 아이템 목록과 아이콘
- 정렬·필터 표시
- 드래그·클릭 입력
- 예약·사용 불가 상태 표시
- Transfer 결과 연출
```

Blueprint가 Item 소유권 배열을 직접 수정하거나 Transaction을 우회하지 않는다.

---

## 11. 결정 잠금

| ID | 결정 | 상태 |
|---|---|---|
| `INV-D-001` | Inventory를 CF-FQ-034 내부가 아닌 별도 CF-FQ-035로 소유한다. | Approved |
| `INV-D-002` | 정적 Definition과 실제 Item Instance를 분리한다. | Approved |
| `INV-D-003` | 장비·방어는 Quantity 1의 고유 Instance로 시작한다. | Approved |
| `INV-D-004` | P0 접근 Container는 현재 차량 Cargo와 MountedEquipment로 제한한다. | Approved |
| `INV-D-005` | 시간 액션 중 실제 이동 대신 Reservation만 만든다. | Approved |
| `INV-D-006` | 완료 순간 Atomic Transfer를 Commit하고 취소 시 원상 유지한다. | Approved |
| `INV-D-007` | Inventory는 Mount 호환·질량 Snapshot·Runtime Apply를 소유하지 않는다. | Approved |
| `INV-D-008` | Domain 질량·전투 수치를 Inventory Definition에 복제하지 않는다. | Approved |
| `INV-D-009` | SaveGame·Loot·Economy는 P0 Foundation에서 제외한다. | Approved |
| `INV-D-010` | ItemInstanceId는 FGuid를 감싼 FCFItemInstanceId로 직렬화·복사 안정성을 보장한다. | Approved / Implemented |
| `INV-D-011` | ItemDefinitionId는 도메인별 FPrimaryAssetId를 사용하고 Definition 이름을 명시적으로 유지한다. | Approved / Implemented |
| `INV-D-012` | Equipment·Defense Definition은 강타입 Domain DataAsset 참조를 사용하며 누락 시 Instance 생성을 거부한다. | Approved / Implemented |
| `INV-D-013` | Container·Reservation 식별자는 INV-P0-01 ItemInstance에 미리 넣지 않는다. | Approved / Implemented |
| `INV-D-014` | OwnerId와 ContainerId는 FGuid를 감싼 별도 강타입 ID로 유지한다. | Approved / Implemented |
| `INV-D-015` | ContainerSlotId는 Container가 소유하며 MountedEquipment에서는 MountProfileId를 위치 키로만 사용한다. | Approved / Implemented |
| `INV-D-016` | 슬롯 Capacity와 선택적 질량 Capacity를 분리하고 질량은 외부 해석값만 Query 입력으로 받는다. | Approved / Implemented |
| `INV-D-017` | 한 Owner는 P0 ContainerType별 하나의 Container만 소유하고 ItemInstanceId는 전체 집합에서 단일 위치만 허용한다. | Approved / Implemented |
| `INV-D-018` | Reserved 접근 상태와 모든 변경 API는 INV-P0-03 전까지 추가하지 않는다. | Approved / Implemented |
| `INV-D-019` | ReservationId와 TransactionId는 FGuid를 감싼 별도 강타입 ID로 유지한다. | Approved / Implemented |
| `INV-D-020` | 하나의 Prepared Transaction은 Item Reservation과 Destination Capacity Reservation을 함께 소유한다. | Approved / Implemented |
| `INV-D-021` | Prepare와 Rollback은 Container를 변경하지 않는다. | Approved / Implemented |
| `INV-D-022` | Commit은 Candidate Container 복사본 전체 Validation 성공 후에만 원본을 교체한다. | Approved / Implemented |
| `INV-D-023` | Commit 재검증 실패는 Prepared 상태를 유지하며 자동 부분 Rollback하지 않는다. | Approved / Implemented |
| `INV-D-024` | 같은 Transaction의 중복 Commit은 AlreadyCommitted로 멱등 종료한다. | Approved / Implemented |
| `INV-D-025` | 자동 만료 Timer는 P0에서 제외하고 Action 종료자는 Reservation 취소 Token 또는 Transaction Rollback으로 두 Reservation을 함께 정리한다. | Approved / Implemented |
| `INV-D-026` | Inventory Draft의 Equipment·Defense 선택은 DataAsset 포인터가 아니라 ItemInstanceId를 입력으로 사용한다. | Approved / Implemented |
| `INV-D-027` | 선택 Item은 현재 차량의 접근 가능한 VehicleCargo 또는 MountedEquipment에 정확히 한 위치로 존재해야 한다. | Approved / Implemented |
| `INV-D-028` | Active Reservation이 있는 ItemInstanceId와 같은 ItemInstanceId의 복수 선택을 Adapter에서 거부한다. | Approved / Implemented |
| `INV-D-029` | ItemDefinitionId는 단일 강타입 Inventory Definition으로 해석하고 Domain DataAsset 참조만 Binding한다. | Approved / Implemented |
| `INV-D-030` | MountBindings는 입력 배열이 아니라 VehicleData.MountProfiles 순서로 생성한다. | Approved / Implemented |
| `INV-D-031` | 호환성과 질량은 기존 UCFVehicleFittingData.BuildFittingSnapshot에 위임하고 Inventory에 복제하지 않는다. | Approved / Implemented |
| `INV-D-032` | Adapter는 Inventory 이동, Weapon·Defense Runtime 적용과 VehicleMovement 변경을 수행하지 않는다. | Approved / Implemented |
| `INV-D-033` | Inventory UI 상태는 별도 mutable Component가 아니라 기존 Runtime을 읽는 불변 `FCFInventoryViewSnapshot`으로 생성한다. | Approved / Implemented |
| `INV-D-034` | P0 ViewData는 현재 차량 Owner의 VehicleCargo·MountedEquipment만 대상으로 하고 다른 Owner Item을 격리한다. | Approved / Implemented |
| `INV-D-035` | CompatibilityHint는 Fitting의 외부 결과만 합성하며 Inventory가 Mount 호환성을 계산하지 않는다. | Approved / Implemented |
| `INV-D-036` | Item·Container·Reservation 표시는 Snapshot ChangeSet으로 표현하고 Transaction 완료·실패 Operation Result는 후속 Coordinator가 명시적으로 소유한다. | Approved / Implemented |
| `INV-D-037` | Field completion은 `Inventory Prepare → Runtime Prepare → Runtime Commit → Inventory Commit` 순서로 조율한다. | Approved / Implemented |
| `INV-D-038` | Runtime Commit 뒤 Inventory Commit 실패는 직전 Applied Runtime Checkpoint 복원과 Inventory Rollback을 모두 수행하며 부분 장착을 허용하지 않는다. | Approved / Implemented |
| `INV-D-039` | 한 Coordinator 호출은 단일 Transfer만 처리하고 Replace는 기존 Field Fitting 계약대로 Unequip 후 별도 Equip으로 수행한다. | Approved / Implemented |
| `INV-D-040` | 현재 실제 Fitting Runtime Adapter는 Field Mass Reapply 전까지 총질량 0.01kg 이내 same-mass 후보만 허용하고 mass-changing 후보를 명시 거부한다. | Approved / Implemented |
| `INV-D-041` | Coordinator Foundation Technical PASS를 FFIT Permission·Timed Action·Mass Reapply 완료로 해석하지 않는다. | Approved / Implemented |





---

## 12. Changelog

### v0.7.0 - 2026-08-14

```text
- INV-P0-06 Coordinator Foundation의 원자 completion·보상 Rollback 계약을 확정했다.
- FCFFieldFitCompletionResult가 실제 Transaction Operation Result를 소유하게 해 Snapshot Diff와 Transaction 결과 책임을 분리했다.
- UCFVehicleFittingComp v1.3.0의 검증 Snapshot 직접 Prepare와 Applied Runtime Checkpoint 복원을 completion 보상 경로로 연결했다.
- 실제 Fitting Runtime Adapter는 Field Mass Reapply 부재를 숨기지 않고 0.01kg 이내 same-mass 후보만 허용하도록 잠갔다.
- Inventory Commit 실패 뒤 Runtime Compensation + Inventory Rollback을 모두 수행하고 어느 한쪽 복구 실패는 RecoveryFailed로 노출한다.
- FFIT-P0-01 Permission, FFIT-P0-02 Timed Action, FFIT-P0-04 Field Mass Reapply는 별도 정식 단계로 유지한다.
- 최종 Build `e2ef556b64484a09ba8b3544c62344e3`, M6 3/3, Inventory 12/12, 전체 CarFight 71/71 Success를 확인했다.
```

### v0.6.0 - 2026-08-14

```text
- INV-P0-05의 읽기 전용 ViewData 계약을 후보에서 확정으로 승격했다.
- 현재 차량 Owner의 VehicleCargo·MountedEquipment를 읽는 FCFInventoryViewSnapshot과 BuildResult/Builder를 구현했다.
- Item Row의 Definition·표시 이름·Domain·위치·Reservation·Access와 외부 CompatibilityHint 계약을 확정했다.
- 이전/현재 Snapshot의 Item·Container·Reservation 의미 변화를 FCFInventoryViewChangeSet으로 계산하도록 확정했다.
- 별도 mutable Inventory UI Component 또는 Inventory-owned Mount 호환 계산을 추가하지 않았다.
- Transaction Completed/Failed는 Snapshot Diff에 넣지 않고 후속 Field Fitting Coordinator의 실제 Operation Result가 소유하도록 잠갔다.
- 공식 Build `30503595ba074c63ba8a6bb87f7a2645`, Inventory 9/9, 전체 CarFight 68/68 Success를 확인했다.
```

### v0.5.0 - 2026-08-02

```text
- ItemInstanceId 기반 Equipment Mount·Defense Override Adapter 입력 계약을 확정했다.
- 현재 차량 Container 소유권·접근 가능 상태와 Active Reservation 거부 순서를 확정했다.
- ItemDefinitionId의 단일 강타입 Equipment·Defense Definition 해석을 확정했다.
- 같은 ItemInstanceId 복수 선택 거부와 같은 Definition의 서로 다른 Instance 구분을 확정했다.
- VehicleData.MountProfiles 순서의 결정론적 Mount Binding을 확정했다.
- Transient UCFVehicleFittingData를 통한 기존 BuildFittingSnapshot 호환·질량 검증 재사용을 확정했다.
- FittingSnapshotInvalid에서 기존 ValidationIssues와 Binding을 보존하되 적용·이동에 사용할 수 없는 정책을 확정했다.
- Adapter의 Inventory·Weapon·Defense·VehicleMovement 비변경 경계를 확정했다.
- Pawn 없는 신규 테스트 1/1, 전체 CarFight 42/42, 필수 회귀 20/20 Success와 공식 Editor Build PASS를 기록했다.
- FIT-P0-00~05, CF-FQ-032 Active와 Unreal Asset은 수정하지 않았다.
```

### v0.4.0 - 2026-08-01


```text
- FCFInventoryReservationId, FCFInventoryTransactionId와 Reservation·Transaction 상태 계약을 확정했다.
- Item Reservation과 Destination Slot·Slot Capacity·외부 해석 Mass Capacity Reservation을 구현했다.
- 같은 Item·Destination Slot·누적 Capacity 이중 예약 거부와 Prepare 멱등성을 확정했다.
- 한 Reservation ID 취소가 같은 Transaction의 두 Active Reservation을 함께 취소하고 RolledBack으로 전환하는 멱등 상태를 확정했다.

- Candidate Container 복사본 기반 Atomic Commit과 실패 시 원본 무변경을 확정했다.
- Commit 실패 후 Prepared 유지와 명시적 재시도·Rollback 정책을 확정했다.
- 중복 Commit AlreadyCommitted, Consumed Reservation 취소 거부와 Committed Rollback 거부를 확정했다.
- Pawn 없는 신규 테스트 2/2, 전체 CarFight 38/38, 기존 필수 회귀 17/17 Success와 공식 Editor Build PASS를 기록했다.
- Fitting Runtime, UI, SaveGame과 Unreal Asset은 수정하지 않았다.
```

### v0.3.0 - 2026-08-01


```text
- FCFInventoryOwnerId, FCFInventoryContainerId와 VehicleCargo·MountedEquipment Container 계약을 확정했다.
- ItemInstance와 소유 위치를 분리하고 Owner·Container·ContainerSlotId가 완전한 위치를 소유하도록 했다.
- 슬롯 수와 선택적 외부 해석 질량을 평가하는 Capacity Query를 확정했다.
- Owner별 ContainerType 단일성, ContainerId·SlotId·ItemInstanceId 중복 방지 불변식을 확정했다.
- 현재 차량 Access Query와 Item Location Found·Missing·Duplicate·InvalidQuery 상태를 확정했다.
- Reserved는 INV-P0-03으로 이관하고 Reservation·Transfer·Fitting Runtime API를 추가하지 않았다.
- Pawn 없는 신규 테스트 2/2, 전체 CarFight 36/36, 기존 필수 회귀 17/17 Success와 공식 Editor Build PASS를 기록했다.
- UI와 Unreal Asset은 수정하지 않았다.
```

### v0.2.0 - 2026-08-01


```text
- FCFItemInstanceId, FCFInventoryItemHandle과 FCFInventoryItemInstance의 실제 계약을 확정했다.
- ItemInstance는 ItemHandle과 Quantity만 소유하며 Container·Reservation은 후속 구조로 분리했다.
- UCFInventoryItemData 공통 기반과 Equipment·Defense 강타입 Definition 구현을 확정했다.
- 명시 ItemDefinitionId와 도메인별 FPrimaryAssetType으로 에셋 이름 변경과 도메인 이름 충돌을 분리했다.
- 잘못된 Domain DataAsset 참조와 Quantity 1 위반을 Automation으로 검증했다.
- Pawn 없는 신규 테스트 2/2, 전체 CarFight 34/34, 기존 필수 회귀 17/17 Success와 공식 Editor Build PASS를 기록했다.
- Blueprint와 Unreal Asset은 수정하지 않았다.
```

### v0.1.0 - 2026-08-01

```text
- Item Definition·Instance·Container·Reservation·Transaction 책임을 정의했다.
- VehicleCargo와 MountedEquipment를 P0 Container로 확정했다.
- 필드 피팅의 ItemInstance 기반 Draft와 Atomic Transfer 연결 계약을 정의했다.
- 장착 호환과 질량 계산을 Inventory로 복제하지 않는 경계를 잠갔다.
```

---

## 13. Migration

```text
- 기존 EquipmentPresetData 직접 참조는 Legacy·Template 계약으로 유지한다.
- ItemInstance 도입 시 기존 FittingSnapshot 필드를 제거하지 않고 InstanceId를 추가 확장한다.
- MountedEquipment는 MountProfile 규칙의 복제본이 아니다.
- Inventory Foundation 구현 전 임시 배열·Widget 변수로 소유권을 대신하지 않는다.
- `INV-P0-02` ContainerState는 현재 소유 상태의 순수 표현이며 외부 코드가 Entries를 직접 이동 함수처럼 수정하지 않는다.
- 질량 Capacity 호출자는 Domain DataAsset에서 해석한 현재 Container 질량과 요청 Item 질량을 전달하며 Container Entry에 질량을 복제하지 않는다.
- 시간 액션 시작 전 `PrepareTransfer`, 취소·실패 시 `RollbackTransfer`, 완료 시 상위 Coordinator를 통한 `CommitTransfer` 순서를 사용한다.
- `CancelReservation`은 어느 한 Reservation ID를 취소 Token으로 받아 같은 Transaction의 두 Active Reservation을 함께 취소·Rollback하며 반복 호출은 `AlreadyCancelled`를 반환한다.

- Commit 실패는 Prepared 상태와 Reservation을 유지하므로 호출자가 재시도 또는 Rollback을 반드시 선택한다.
- Saved Template·Legacy 출격은 기존 UCFVehicleFittingData 직접 참조를 유지하고 실제 Inventory Draft만 FCFInventoryFitAdapter를 사용한다.
- 성공 Adapter 결과의 Binding은 실제 Instance 추적 SSOT이며 FittingSnapshot은 기존 호환·질량 해석 SSOT다.
- FittingSnapshotInvalid 결과는 Runtime Apply·Inventory 이동 Commit 입력으로 사용하지 않는다.
- Adapter는 Runtime Component를 직접 호출하지 않으며 후속 Field Fitting Coordinator가 Binding·Reservation·Runtime Apply·Inventory Commit 순서를 조율한다.
- UI와 후속 Consumer는 Inventory Container/Ledger 내부 배열을 직접 읽어 표시하지 않고 `FCFInventoryViewBuilder::BuildSnapshot()`을 사용한다.
- 변화 표시가 필요하면 이전/현재 Snapshot의 `DiffSnapshots()` 결과를 사용한다. Transaction 완료·실패는 `FCFFieldFitCompletionResult`를 사용한다.
- Field completion 호출자는 FFIT Permission·Timed Action이 완료 조건을 충족한 뒤 Coordinator를 호출한다. Coordinator 자체가 전투·속도·쿨타임·시간 경과를 재판정하지 않는다.
- Field Mass Reapply 구현 전 실제 FittingComp Adapter에서 질량 변경 후보를 우회 적용하지 않는다.
- Ammo Item 타입은 CF-FQ-031의 실제 AmmoData 계약 이후 추가한다.



```
