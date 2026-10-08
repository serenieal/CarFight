# Inventory Foundation Roadmap

- Version: 0.9.0
- Date: 2026-08-02
- Status: Active Roadmap / `M0~M5` Done / `M6` In Progress — Coordinator Foundation Technical PASS / Formal FFIT Integration Pending




- Feature: `CF-FQ-035 인벤토리 Foundation`
- Representative Plan: `Document/Plan/InventoryFoundation/InventoryFoundationPlan.md`
- Design: `Document/Plan/InventoryFoundationDesign.md`
- Current Active Feature: `CF-FQ-035 인벤토리 Foundation`
- Preserved Paused Feature: `CF-FQ-032` — USER Visual checkpoint

---

## 1. 목표 흐름

```text
Contract
→ Item Identity
→ Container and Access
→ Reservation and Atomic Transfer
→ Fitting Adapter
→ ViewData
→ Automation and PIE
```

인벤토리 P0는 필드 피팅을 막는 최소 기반만 구현하고 Loot·Economy·SaveGame을 선구축하지 않는다.

---

## 2. 현재 단계

```text
M0 Contract and Ownership: Done — Documentation
M1 Item Identity and Definition: Done — Code·Automation·Build PASS
M2 Container and Access Query: Done — Code·Automation·Build PASS
M3 Reservation and Atomic Transfer: Done — Code·Automation·Build PASS
M4 Fitting Inventory Adapter: Done — Code·Automation·Build PASS
M5 ViewData and Events: Done — Code·Automation·Build PASS
M6 Integration Verification: In Progress — FFIT-P0-01~04 + FIT-P0-06 Technical Integration PASS / Remote Technical Checkpoint Complete / USER-facing Field UI·PIE Pending
```

---

## 3. 의존 그래프

```text
CF-FQ-035 M1~M4
├─ Item Instance
├─ VehicleCargo
├─ Reservation
├─ Atomic Transfer
└─ Fitting Inventory Adapter

        ↓
CF-FQ-034 FFIT-P0-02 Timed Action
        ↓
CF-FQ-035 M4 Fitting Adapter
        ↓
CF-FQ-034 FFIT-P0-03 Atomic Equip / Unequip
        ↓
CF-FQ-034 FIT-P0-05 Runtime Mass Apply
        ↓
CF-FQ-034 FFIT-P0-04 Field Runtime Reapply
```

CF-FQ-032는 화면과 입력을 제공하지만 Inventory Foundation 완료의 선행 구현은 아니다.

---

## 4. M0 — Contract and Ownership

### 완료 범위

```text
- CF-FQ-035 별도 기능 경계
- Inventory와 Fitting 책임 분리
- P0 Item·Container·Reservation·Transfer 범위
- Loot·Economy·SaveGame 제외
- Field Fitting 의존 관계
```

### 상태

```text
Done — Documentation Only
Source: Not Modified
Asset: Not Modified
Build: Not Required
```

---

## 5. M1 — Item Identity and Definition

### 목표

정적 Definition과 실제 소유 Instance를 분리한다.

### 결과

```text
- ItemInstanceId
- ItemDefinitionId
- Quantity
- Domain Asset 참조 계약
- Equipment·Defense 고유 Instance
```

### 종료 기준

```text
- Instance ID 안정성 Automation — PASS
- 잘못된 Definition 참조 거부 — PASS
- 장비·방어 Quantity 1 계약 — PASS
- Ammo 임시 타입 없음 — PASS
- 공식 Editor Build PASS — Job 32cd34dd2424407b94c7377dd19a5fce / Exit Code 0
```

### 구현 결과

```text
- FCFItemInstanceId: FGuid 기반 생성·복사·직렬화 안정 ID
- FCFInventoryItemHandle: ItemInstanceId + FPrimaryAssetId ItemDefinitionId
- FCFInventoryItemInstance: ItemHandle + Quantity 1 고유 인스턴스
- UCFInventoryItemData: 명시 ItemDefinitionId와 공통 검증
- UCFEquipmentItemData: EquipmentPresetData 강타입 참조
- UCFDefenseItemData: VehicleDefenseData 강타입 참조
- 잘못된 Domain 참조에서 ItemInstance 생성 거부
- Container·Reservation·Transfer·Fitting Runtime 필드 없음
```

### 검증 결과

```text
CarFight.Inventory.INV_P0_01.Identity — Success / Warning 0 / Error 0
CarFight.Inventory.INV_P0_01.Definition — Success / Warning 0 / Error 0
전체 CarFight Automation — 34/34 Success / Failed 0
기존 필수 회귀 — 17/17 Success
공식 Editor Build — 32cd34dd2424407b94c7377dd19a5fce / Exit Code 0
```


---

## 6. M2 — Container and Access Query

### 목표

현재 차량 Cargo와 Mounted 소유 상태를 일관되게 조회한다.

### 결과

```text
- FCFInventoryOwnerId와 FCFInventoryContainerId
- VehicleCargo·MountedEquipment ContainerState
- Owner·Container·ContainerSlotId·ItemInstanceId 소유 위치
- Slot Capacity
- 선택적 외부 해석 Mass Capacity
- Accessible / Unavailable / Missing / InvalidQuery 접근 조회
- Found / Missing / Duplicate / InvalidQuery Item 위치 조회
```

`Reserved`는 M3 Reservation 구현 전까지 Access 상태에 포함하지 않는다.

### 종료 기준

```text
- 하나의 Instance가 전체 Container 집합에서 하나의 위치에만 존재 — PASS
- 한 Owner가 P0 ContainerType별 하나의 Container만 소유 — PASS
- ContainerId와 ContainerSlotId 중복 거부 — PASS
- Slot·선택적 Mass Capacity 초과 거부 — PASS
- 질량 값의 Inventory Definition·Instance 복제 없음 — PASS
- MountProfile 규칙 복제 없음 — PASS
- Pawn 없는 Container·AccessQuery Automation PASS — PASS
- 공식 Editor Build PASS — Job 36260a7a70dd48e899f27dcd038efb2b / Exit Code 0
```

### 구현 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryContainer.h
- UE/Source/CarFight_Re/Private/CFInventoryContainer.cpp

Automation:
- UE/Source/CarFight_Re/Private/CFInventoryContainerTests.cpp

변경 API:
- 없음
- Reservation 없음
- Atomic Transfer 없음
- Fitting Runtime 연결 없음
```

### 검증 결과

```text
CarFight.Inventory.INV_P0_02.Container — Success / Warning 0 / Error 0
CarFight.Inventory.INV_P0_02.AccessQuery — Success / Warning 0 / Error 0
전체 CarFight Automation — 36/36 Success / Failed 0
기존 필수 회귀 — 17/17 Success
공식 Editor Build — 36260a7a70dd48e899f27dcd038efb2b / Exit Code 0
```


---

## 7. M3 — Reservation and Atomic Transfer

### 목표

시간 액션 중 소유권을 안전하게 잠그고 완료 순간 원자 이동한다.

### 결과

```text
- FCFInventoryReservationId와 FCFInventoryTransactionId
- Item Reservation
- Destination Slot·Slot Capacity·외부 해석 Mass Capacity Reservation
- Container 무변경 Transaction Prepare
- Candidate Container 복사본 기반 Atomic Commit
- Transaction Rollback
- Reservation Cancel과 Prepare·Commit·Rollback 멱등 상태
```

### 종료 기준

```text
- 같은 Item과 같은 Destination Slot 이중 예약 거부 — PASS
- 실제 Entry와 모든 Active Reservation을 합산한 Slot·Mass Capacity 초과 거부 — PASS
- Prepare 전후 Source·Destination Container 무변경 — PASS
- 어느 한 Reservation ID 취소로 같은 Transaction 전체 RolledBack·Active Reservation 잔류 없음 — PASS
- Reservation 반복 취소 AlreadyCancelled — PASS
- 명시적 Rollback 후 Active Reservation 잔류 없음 — PASS

- Commit 실패 시 Source·Destination 무변경·Prepared 유지 — PASS
- Commit 성공 시 Source 제거와 Destination 추가를 함께 확정 — PASS
- 중복 Commit AlreadyCommitted와 Item 단일 위치 유지 — PASS
- Consumed Reservation 취소 및 Committed Transaction Rollback 거부 — PASS
- Pawn 없는 Reservation·AtomicTransfer Automation PASS — PASS
- 공식 Editor Build PASS — Job 32ca1cacb47548d4aa24ee12372baf67 / Exit Code 0

```

### 구현 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryTransfer.h
- UE/Source/CarFight_Re/Private/CFInventoryTransfer.cpp

Automation:
- UE/Source/CarFight_Re/Private/CFInventoryTransferTests.cpp

상태:
- Reservation: Active / Cancelled / Consumed
- Transaction: Prepared / Committed / RolledBack
- Commit 실패: Prepared 유지 후 재시도 또는 명시적 Rollback
```

### 검증 결과

```text
CarFight.Inventory.INV_P0_03.Reservation — Success / Warning 0 / Error 0
CarFight.Inventory.INV_P0_03.AtomicTransfer — Success / Warning 0 / Error 0
전체 CarFight Automation — 38/38 Success / Failed 0
기존 필수 회귀 — 17/17 Success
공식 Editor Build — 32ca1cacb47548d4aa24ee12372baf67 / Exit Code 0

```

M3 완료로 `FFIT-P0-02` 착수 Gate는 충족됐다. 실제 Fitting 연결은 M4 범위이며 이번 작업에서는 착수하지 않았다.


---

## 8. M4 — Fitting Inventory Adapter

### 목표

실제 Item Instance를 검증 가능한 Equipment·Defense 피팅 Binding과 기존 Snapshot 입력으로 변환한다.

### 결과

```text
- FCFInventoryFitMountInput / DefenseInput / Request
- FCFInventoryFitMountBinding / DefenseBinding / Result
- Equipment ItemDefinitionId → UCFEquipmentItemData → EquipmentPresetData
- Defense ItemDefinitionId → UCFDefenseItemData → VehicleDefenseData
- 현재 차량 Container 소유권·접근 가능 상태 검증
- Active Reservation Item 선택 거부
- 같은 ItemInstanceId 복수 Mount·Defense 선택 거부
- VehicleData.MountProfiles 순서의 결정론적 Binding
- Transient UCFVehicleFittingData → 기존 BuildFittingSnapshot 연결
```

미래 Ammo Item은 CF-FQ-031의 실제 AmmoData 계약 이후 같은 강타입 해석 방식으로 추가하며 이번 M4에 임시 타입을 만들지 않았다.

### 종료 기준

```text
- 소유하지 않은 Instance 선택 거부 — PASS
- 다른 차량 Owner와 접근 차단 Container Item 거부 — PASS
- Active Reservation Item 선택 거부 — PASS
- 같은 ItemInstanceId 다중 Mount 선택 거부 — PASS
- Definition은 같고 Instance가 다른 장비 구분 — PASS
- EquipmentPresetData·VehicleDefenseData 강타입 해석 — PASS
- 입력 배열 순서와 무관한 Mount Binding — PASS
- 기존 BuildFittingSnapshot 호환·질량 결과 연결 — PASS
- FittingSnapshotInvalid와 기존 ValidationIssues 전달 — PASS
- Inventory Container·Reservation 무변경 — PASS
- Weapon·Defense·VehicleMovement Runtime 직접 호출 없음 — PASS
- Pawn 없는 FittingAdapter Automation — PASS
- 공식 Editor Build PASS — Job 23af46f3a13a4ba88e9878bcdc225a50 / Exit Code 0
```

### 구현 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryFitAdapter.h
- UE/Source/CarFight_Re/Private/CFInventoryFitAdapter.cpp

Automation:
- UE/Source/CarFight_Re/Private/CFInventoryFitTests.cpp
- CarFight.Inventory.INV_P0_04.FittingAdapter

상태:
- Adapter는 읽기·검증·Binding·Snapshot 생성만 수행
- Inventory 이동 Commit 없음
- Fitting Runtime Apply 없음
- Field Timed Action 없음
```

### 검증 결과

```text
CarFight.Inventory.INV_P0_04.FittingAdapter — Success / Warning 0 / Error 0
전체 CarFight Automation — 42/42 Success / Failed 0
필수 회귀 — 20/20 Success
공식 Editor Build — 23af46f3a13a4ba88e9878bcdc225a50 / Exit Code 0
```

M4 완료로 `FFIT-P0-03 Atomic Equip / Unequip`의 Inventory Adapter Gate는 충족됐다. Timed Action, Inventory Commit과 Runtime Apply 조율은 후속 Field Fitting 범위이며 이번 작업에서는 착수하지 않았다.


---

## 9. M5 — ViewData and Events

### 목표

UI와 후속 Coordinator가 Inventory 내부 배열을 직접 수정하지 않고 현재 상태와 의미 변화를 읽는다.

### 구현 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFInventoryViewData.h
- UE/Source/CarFight_Re/Private/CFInventoryViewData.cpp

Automation:
- UE/Source/CarFight_Re/Private/CFInventoryViewTests.cpp
- CarFight.Inventory.INV_P0_05.ViewData
- CarFight.Inventory.INV_P0_05.ChangeSet

Read Model:
- FCFInventoryItemViewData
- FCFInventoryContainerViewData
- FCFInventoryViewSnapshot
- FCFInventoryViewBuildResult
- FCFInventoryViewBuilder.BuildSnapshot

Change Model:
- ECFInventoryViewChangeType
- FCFInventoryViewChange
- FCFInventoryViewChangeSet
- FCFInventoryViewBuilder.DiffSnapshots
```

### 종료 기준

```text
- Inventory Item Row ViewData — PASS
- Container ViewData — PASS
- Active Reservation 표시 — PASS
- Prepare 시 Container 무변경 + ReservationChanged만 발생 — PASS
- Commit 시 Item Location·Reservation·Source/Destination Container 변화 검출 — PASS
- 다른 Owner Item 현재 차량 Snapshot 격리 — PASS
- Fitting CompatibilityHint 외부 합성 지점 — PASS
- Inventory가 Mount 호환성을 자체 계산하지 않음 — PASS
- Transaction 완료·실패를 Snapshot Diff로 추측하지 않음 — PASS
- Pawn 없는 INV-P0-05 Automation 2/2 — PASS
- Inventory 전체 회귀 9/9 — PASS
- 전체 CarFight 회귀 68/68 — PASS
- 공식 Editor Build — PASS / `30503595ba074c63ba8a6bb87f7a2645`
```

### 검증 결과

```text
Targeted M5: `7cb4c11aa51540d6a3ee181ee2ec48f1` / 2/2 Success / 0 Fail
Inventory Regression: `0fd8c724fc5141dba3d064874bc87438` / 9/9 Success / 0 Fail
Inventory JSON SHA-256: `4058779b9fea9349bd88fb2465f341e6e43e8be66967f6ad37b3b5c49b2505ac`
Full CarFight Regression: `5e5f1299f6b449738b6d24607afb32d0` / 68/68 Success / 0 Fail
Full JSON SHA-256: `c952b2631d6556be9ce9572b97d9ae9ec55232d3937962fd88137f0f362f95d1`
Execution Method: `Tools/RunInvAutomation.ps1 v1.0.1` 작업 전용 / Reusable Entry Point로 승격하지 않음
```

M5 완료로 Inventory P0의 읽기 전용 표현 기반은 충족됐다. Transaction Operation Result와 Runtime 적용 조율은 M6 Field Fitting Coordinator가 소유한다.

---

## 10. M6 — Integration Verification

### 현재 상태

```text
Coordinator + Formal Field Fitting Action Integration: Technical PASS
Permission / Timed Action / Reservation / Atomic Equip·Unequip: Technical PASS
General Mass-changing Equip/Unequip Runtime: Technical PASS — actual Chaos PIE 포함
USER-facing Field UI·PIE: Pending / 현재 사용자 직접 시각 확인 불가
```

### 구현 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFFieldFitCoordinator.h
- UE/Source/CarFight_Re/Private/CFFieldFitCoordinator.cpp
- UE/Source/CarFight_Re/Public/CFVehicleFittingComp.h v1.4.0
- UE/Source/CarFight_Re/Private/CFVehicleFittingComp.cpp v1.4.0
- UE/Source/CarFight_Re/Public/CFFittingViewData.h v1.0.0
- UE/Source/CarFight_Re/Private/CFFittingViewData.cpp v1.0.0

Automation:
- UE/Source/CarFight_Re/Private/CFFieldFitTests.cpp v1.1.0
- CarFight.Inventory.INV_P0_06.FieldFitCoordinatorAtomic
- CarFight.Inventory.INV_P0_06.FieldFitRuntimeAdapter
- CarFight.Inventory.INV_P0_06.FieldFitCoordinatorFittingComp
```

### 원자 completion 계약

```text
Inventory Prepare
→ Runtime Prepare + Previous Applied Checkpoint
→ Runtime Commit
→ Inventory Commit

Runtime Prepare/Commit 실패
→ Prepared Runtime 정리
→ Inventory Rollback

Inventory Commit 실패
→ Previous Applied Runtime Checkpoint 보상 복원
→ Inventory Rollback
→ 부분 장착 0
```

한 번의 Coordinator 호출은 단일 Transfer만 다룬다. 빈 Mount Equip과 단일 Unequip에 맞는 기반이며 Replace는 기존 설계대로 Unequip 완료 후 별도 Equip이다.

현재 `FCFFieldFitRuntimeAdapter`는 선택적 `ICFFieldFitMassRuntime`을 통해 mass-changing 후보도 같은 completion transaction에 포함한다. production `FCFChaosVehicleMassRuntime`은 Movement Mass를 변경하고 Chaos Physics State를 재생성한 뒤 VehicleMesh actual mass, Transform과 Runtime Ready를 검증한다. Mass Runtime이 없는 호출만 기존 0.01kg same-mass 안전 경계를 유지한다.

### 기술 검증

```text
Official Build: `e2ef556b64484a09ba8b3544c62344e3` / Exit 0
M6 Targeted: `c02cd6ce84904e42b618f7171150f890` / 3/3 Success / 0 Fail
M6 JSON SHA-256: `c0d7475f715e13a2e9af53e795484ab3c6eb82a8cd037d05505dfd52f57860af`
Inventory Regression: `fa79fd3ee0234cb8859ca4a93ddc9e19` / 12/12 Success / 0 Fail
Inventory SHA-256: `d50c75e8c53381b9e1231016122b0fae7dab8ece3ec8a076efb85cbfd52f3b70`
Full CarFight: `70423e50820941f4886aa95904622fc8` / 71/71 Success / 0 Fail
Full SHA-256: `3857cfacc6178066220c2a2278b55797115bca60ce3880b7be47182d4bbd7c9a`
```

### 남은 M6 Gate

```text
1. FFIT-P0-05 Field Fitting UI and PIE
2. FIT-P0-06 ViewData의 16:9·32:9 실제 화면 가독성 확인
3. 사용자 직접 확인 가능한 시점의 실제 Field Equip/Unequip·취소·진행률 USER 검증
```

FFIT-P0-01~04, actual Chaos Mass PIE와 FIT-P0-06 C++ ViewData·Blueprint Contract는 Technical PASS다. 현재 CF-FQ-035/034 내부의 원격 기술 선행 Gate는 완료됐으며 USER-facing UI·PIE 전에는 INV-P0-06 전체 Done 또는 CF-FQ-035 Done으로 승격하지 않는다.

---

## 11. Changelog

### v0.9.0 - 2026-08-15

```text
- FIT-P0-06 C++ Fitting ViewData·Blueprint Contract Technical PASS를 M6 통합 상태에 반영했다.
- Build PASS, FIT-P0-06 1/1, Fitting 22/22, Inventory 12/12, Full CarFight 84/84 Success를 기록했다.
- M6의 원격 기술 선행은 완료됐고 남은 Gate를 Field UI·실제 화면 가독성·USER Equip/Unequip 검증으로 한정했다.
- USER-facing Gate를 기술 Automation으로 대체하지 않는다.
```

### v0.8.0 - 2026-08-15

```text
- M6 Formal Field Fitting Integration의 FFIT-P0-01~04 Technical PASS를 반영했다.
- Timed Action이 Prepared Inventory Transaction을 Coordinator에 넘기고 mass-changing Chaos Runtime까지 같은 transaction에서 적용·보상한다.
- 실제 ChaosMassPIE +50kg 적용·원복 PASS, Fitting 21/21, Inventory 12/12, Full CarFight 83/83 Success를 기록했다.
- 남은 M6 Gate를 FIT-P0-06 ViewData → FFIT-P0-05 UI·PIE → USER 검증으로 교정했다.
```

### v0.7.0 - 2026-08-14

```text
- M6 Integration Verification을 In Progress로 전환하고 Coordinator Foundation Technical PASS를 기록했다.
- 실제 Inventory Prepare·Runtime Prepare/Commit·Inventory Commit과 실패 보상 Rollback 순서를 구현했다.
- UCFVehicleFittingComp의 검증 Snapshot 직접 Prepare와 Previous Applied Runtime Checkpoint 복원을 연결했다.
- same-mass 실제 FittingComp 조합은 성공하고 mass-changing 후보는 RuntimeMassReapplyUnsupported로 Prepare 거부·Inventory Rollback되는 것을 검증했다.
- 최종 Build `e2ef556b64484a09ba8b3544c62344e3`, M6 3/3, Inventory 12/12, Full CarFight 71/71 Success를 기록했다.
- M6 전체 Done과 FFIT-P0-03 완료 처리는 하지 않는다. 다음 정식 단계는 FFIT-P0-01 Permission·Blocker Query다.
```

### v0.6.0 - 2026-08-14

```text
- M5 ViewData and Events를 Done으로 전환했다.
- 현재 차량 VehicleCargo·MountedEquipment의 읽기 전용 Item/Container Snapshot과 Reservation 표시를 구현했다.
- Fitting CompatibilityHint는 외부 입력 합성 지점으로 유지하고 Inventory의 Mount 호환 계산을 금지했다.
- Snapshot 간 Item·Container·Reservation 의미 ChangeSet을 추가했다.
- Transaction Completed/Failed는 M6 Coordinator의 명시적 Operation Result가 소유하도록 분리했다.
- 공식 Build `30503595ba074c63ba8a6bb87f7a2645` PASS, INV-P0-05 2/2, Inventory 9/9, 전체 CarFight 68/68 Success를 기록했다.
- CF-FQ-035를 단일 Active로 전환하고 CF-FQ-032 USER Visual 체크포인트를 Paused로 보존했다.
- 다음 단계는 M6 Integration Verification / Field Fitting Coordinator다.
```

### v0.5.0 - 2026-08-02

```text
- M4 Fitting Inventory Adapter를 Done으로 전환했다.
- ItemInstanceId 기반 Equipment·Defense 선택 입력과 결정론적 Binding 결과를 구현했다.
- 현재 차량 Container 소유권·접근, Active Reservation과 복수 Item 선택 거부를 구현했다.
- ItemDefinitionId의 강타입 EquipmentPresetData·VehicleDefenseData 해석을 구현했다.
- 기존 BuildFittingSnapshot을 재사용한 호환성·질량 검증 연결을 구현했다.
- Adapter의 Inventory 이동·Weapon·Defense·VehicleMovement Runtime 비변경 경계를 확인했다.
- Pawn 없는 FittingAdapter Automation 1/1 Success를 기록했다.
- 전체 CarFight 42/42, 필수 회귀 20/20 Success와 공식 Editor Build PASS를 기록했다.
- 다음 단계는 M5 ViewData and Events이며 이번 작업에서는 착수하지 않았다.
- FIT-P0-00~05, CF-FQ-032 Active와 UI·SaveGame·Unreal Asset을 변경하지 않았다.
```

### v0.4.0 - 2026-08-01


```text
- M3 Reservation and Atomic Transfer를 Done으로 전환했다.
- Item과 Destination Slot·Slot Capacity·외부 해석 Mass Capacity Reservation을 구현했다.
- 반복 Prepare, Transaction 전체를 정리하는 Reservation 취소 Token, 명시적 Rollback과 중복 Commit의 멱등 상태 전이를 구현했다.

- Candidate Container 복사본 기반 원자 Commit과 실패 시 Source·Destination 무변경을 구현했다.
- Commit 실패 후 Prepared 유지, 명시적 재시도·Rollback과 terminal 상태 보호를 구현했다.
- Pawn 없는 Reservation·AtomicTransfer Automation 2/2 Success를 기록했다.
- 전체 CarFight 38/38, 기존 필수 회귀 17/17 Success와 공식 Editor Build PASS를 기록했다.
- 다음 단계는 M4 Fitting Inventory Adapter이며 이번 작업에서는 착수하지 않았다.
- CF-FQ-032 Active 체크포인트와 Fitting Runtime·UI·SaveGame·Unreal Asset을 변경하지 않았다.
```

### v0.3.0 - 2026-08-01


```text
- M2 Container and Access Query를 Done으로 전환했다.
- VehicleCargo·MountedEquipment의 강타입 Owner·Container·Slot 소유 위치를 구현했다.
- Slot Capacity와 선택적 외부 해석 Mass Capacity를 구현했다.
- 현재 차량 Access Query, 다른 차량 격리와 Item Location 단일·중복 조회를 구현했다.
- ContainerId·Owner별 ContainerType·ContainerSlotId·ItemInstanceId 중복 방지 불변식을 구현했다.
- Pawn 없는 Container·AccessQuery Automation 2/2 Success를 기록했다.
- 전체 CarFight 36/36, 기존 필수 회귀 17/17 Success와 공식 Editor Build PASS를 기록했다.
- 다음 단계는 M3 Reservation and Atomic Transfer이며 이번 작업에서는 착수하지 않았다.
- CF-FQ-032 Active 체크포인트와 Fitting Runtime·UI·Unreal Asset을 변경하지 않았다.
```

### v0.2.0 - 2026-08-01


```text
- M1 Item Identity and Definition을 Done으로 전환했다.
- 안정적인 FGuid ItemInstanceId, FPrimaryAssetId Definition Handle과 Quantity 1 Instance를 구현했다.
- Equipment·Defense 강타입 Definition과 잘못된 Domain 참조 생성 거부를 구현했다.
- Pawn 없는 Identity·Definition Automation 2/2 Success를 기록했다.
- 전체 CarFight 34/34, 기존 필수 회귀 17/17 Success와 공식 Editor Build PASS를 기록했다.
- 다음 단계는 M2 Container and Access Query이며 이번 작업에서는 착수하지 않았다.
- CF-FQ-032 Active 체크포인트와 Blueprint·Unreal Asset을 변경하지 않았다.
```

### v0.1.0 - 2026-08-01

```text
- Inventory Foundation을 M0~M6로 분해했다.
- M3 Reservation·Atomic Transfer를 Field Fitting 시간 액션 Gate로 지정했다.
- M4 Fitting Adapter를 실제 ItemInstance 장착 Gate로 지정했다.
- Loot·Economy·SaveGame은 별도 후속으로 유지했다.
```

---

## 12. Migration

```text
- CF-FQ-034의 기존 Sortie Snapshot 구현은 독립적으로 계속할 수 있다.
- 시간 액션 시작 전 INV-P0-03 `PrepareTransfer` 성공을 요구한다.
- 완료 전에는 Container 소유 위치를 변경하지 않고, 취소 시 어느 한 Reservation ID의 `CancelReservation` 또는 Transaction ID의 `RollbackTransfer`로 두 Reservation을 모두 정리한다.

- Commit 실패는 Source·Destination과 Reservation을 변경하지 않은 Prepared 상태이므로 상위 Coordinator가 재시도 또는 Rollback을 선택한다.
- M2 ContainerState의 Entries를 외부 코드가 직접 이동·예약 API처럼 사용하지 않는다.
- Saved Template·Legacy FittingData의 UObject 참조는 계속 Definition 기반 선택으로 유지한다.
- 실제 Inventory Draft는 M4 Adapter 성공 결과의 ItemInstanceId Binding을 사용한다.
- FittingSnapshotInvalid 결과는 Runtime Apply 또는 Inventory Commit에 전달하지 않는다.
- M5 ViewData and Events는 별도 사용자 승인 전 착수하지 않는다.
- CF-FQ-035를 Active로 전환할 때도 현재 CF-FQ-032 체크포인트를 먼저 보존한다.


```
