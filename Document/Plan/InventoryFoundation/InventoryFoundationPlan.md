# Inventory Foundation Implementation Plan

- Version: 0.10.0
- Date: 2026-09-14
- Status: Done / Historical + Retained Path / Rebaseline Complete



- Feature: `CF-FQ-035 인벤토리 Foundation`
- Priority: Historical P1 Feature / `CF-FQ-034 Field Fitting` foundation origin
- Current Lifecycle: `SUPERSEDED / CLOSE` — 현재 프로젝트의 단일 Active Feature는 `CF-FQ-039` 유지
- Historical Context: 당시 `CF-FQ-032` USER Visual checkpoint와 병행 관리됐으나 현재 `CF-FQ-032`는 Done
- Representative Plan: `Document/Plan/InventoryFoundation/InventoryFoundationPlan.md`
- Design: `Document/Plan/InventoryFoundationDesign.md`
- Roadmap: `Document/Plan/InventoryFoundationRoadmap.md`
- Historical Primary Consumer: `CF-FQ-034 차량 피팅·질량 런타임`

---

## 2026-09-14 Final Rebaseline Closure

`CF-FQ-035 인벤토리 Foundation`은 과거 `FFIT-P0-05 Field Fitting UI and PIE`를 그대로 재개하지 않고 현재 Source와 Current Systems를 기준으로 Rebaseline한 결과 **Done / Historical + Retained Path**로 종료한다.

원래 이 Feature가 소유한 독립 요구는 단순 장비 선택 메뉴가 아니라 다음 Foundation이었다.

```text
실제 ItemInstance 소유권
→ VehicleCargo / MountedEquipment Container
→ 접근·Capacity Query
→ Item + Destination Reservation
→ Prepare / Commit / Rollback Atomic Transfer
→ read-only Inventory ViewData
→ ItemInstance 기반 Fitting Adapter
→ Field Fitting completion에서 Runtime + Inventory 원자 경계
```

현재 실제 Source에는 `FCFItemInstanceId`, `FCFInventoryContainerState`, `FCFInventoryAccessQuery`, `FCFInventoryTransferLedger`, `FCFInventoryViewBuilder`, `FCFInventoryFitAdapter`, `FCFFieldFitAction`, `FCFFieldFitCoordinator`가 그대로 존재하며, Item 단일 소유·예약·원자 Transfer·Fitting Adapter·Runtime/Inventory completion/recovery 계약이 유지된다.

반면 현재 Product UI에는 ownership-aware Inventory/FieldFit 화면이 정식 경로로 연결돼 있지 않다. `CF-FQ-041 RuntimeApply`는 차량/장비를 즉시 시험·시연하는 Catalog 기반 UI를 제공하지만, 의도적으로 ItemInstance 소유권·Container·Reservation·Transaction을 만들거나 변경하지 않는다. 따라서 RuntimeApply USER 경로를 Inventory USER PASS로 확대하지 않는다.

Rebaseline 판정:

```text
INV-P0-00~05
= Historical Technical PASS 보존

INV-P0-06 Technical Integration
= FFIT-P0-01~04 + FIT-P0-06 Technical PASS 보존
= Field completion / mass-changing runtime / recovery 분류 evidence 보존

FFIT-P0-05 Field Fitting UI and PIE
= Superseded / Not Executed as CF-FQ-035 closure gate
= USER PASS 아님
= 정식 ownership-aware Inventory/Fitting frontend가 실제 요구가 될 때 새 Product/UI lifecycle
```

SaveGame, 계정/서버 Inventory, PlayerStorage, loot/economy, 거래/Replication 같은 범위는 애초 P0 제외였으며 현재 residual로 소급 편입하지 않는다.

Current owner:

```text
Inventory ownership / Container / Reservation / Atomic Transfer
→ Document/Systems/Vehicles/VehicleInventory.md v1.0.0

Fitting / Initial & Field Mass Runtime
→ Document/Systems/Vehicles/VehicleRuntime.md v1.3.0

현재 즉시 적용·시연 UX
→ CF-FQ-041 RuntimeApply (비소유권 경로)
```

Lifecycle:

```text
G0 = PASS — actual Source + INV-P0-00~05 + INV-P0-06 technical evidence + successor UI boundary 확인
G1 = PASS — 고유 Inventory Foundation 계약을 VehicleInventory.md v1.0.0 Current System으로 승격
G2 = PASS — ActiveWork / ProjectSSOT / Plan Index의 Paused current route 제거
G3 = PASS — InventoryFoundationPlan.md retained path로 Historical evidence 보존
G4 = PASS — semantic Historical
G5 = Deferred — physical Archive move optional
```

이번 Rebaseline은 Source/Asset mutation 없이 Current 계약 승격 + 문서 lifecycle 정리만 수행한다. 새 Build, Automation, PIE, USER PASS는 실행하지 않는다.

---

## 1. 목적

CarFight의 인벤토리를 피팅 내부 배열이나 UI 목록으로 만들지 않고, 플레이어가 실제로 소유하는 아이템 인스턴스와 컨테이너 간 이동을 일관되게 관리하는 공용 Foundation으로 구축한다.

첫 구현의 목적은 상점·루팅·경제 전체가 아니다. 필드 피팅이 요구하는 다음 최소 계약을 제공하는 것이다.

```text
아이템 소유권
→ 접근 가능한 컨테이너 조회
→ 아이템·목적지 예약
→ 시간 소모 액션
→ 완료 순간 원자 이동
→ 취소·실패 시 원상 유지
```

인벤토리 Foundation은 장비가 특정 차량 슬롯에 호환되는지 판정하지 않는다. 장착 호환·피팅 Snapshot·필드 장착 시간은 `CF-FQ-034`가 소유한다.

---

## 2. 별도 기능으로 분리하는 이유

인벤토리는 피팅 외에도 다음 기능이 공통으로 사용한다.

```text
- 탄약 적재와 보급
- 차량 화물
- 필드 루팅
- 보상 지급
- 상점·판매
- 제작·수리
- SaveGame
- 다중 차량 장비 소유권
```

이를 `UCFVehicleFittingComp` 내부에 넣으면 피팅이 아이템 소유권, 저장, 보상과 경제까지 책임지는 구조가 된다. 따라서 다음 경계를 잠근다.

```text
CF-FQ-035 Inventory Foundation
= 아이템 인스턴스, 컨테이너, 접근, 예약, 원자 이동

CF-FQ-034 Vehicle Fitting
= 슬롯 호환, Draft, Snapshot, 장착·해제 액션, Runtime 적용

CF-FQ-032 UI
= 목록·슬롯·진행률·취소 사유 표시와 입력
```

---

## 3. P0 범위

```text
01. 안정적인 ItemInstanceId
02. 정적 Definition 참조와 런타임 Instance 분리
03. 장비 단일 인스턴스와 수량형 Stack의 최소 공용 표현
04. VehicleCargo 컨테이너
05. MountedEquipment 소유 상태를 표현할 연결 계약
06. 컨테이너 SlotCount·선택적 Mass 한도
07. 접근 가능한 인벤토리 컨테이너 조회
08. Item·Source·Destination 예약
09. Commit 또는 Rollback 가능한 원자 Transfer
10. 중복 소유·중복 장착 방지 불변식
11. 읽기 전용 Inventory ViewData
12. Pawn 없는 Automation
```

P0 필드 피팅은 기본적으로 현재 차량의 `VehicleCargo`와 현재 차량의 `MountedEquipment`만 사용한다.

`PlayerStorage`, 주변 보급소와 월드 루팅 컨테이너는 같은 계약으로 후속 확장하되 P0 구현 필수 조건에 넣지 않는다.

---

## 4. P0 제외 범위

```text
- 상점·구매·판매·가격·재화
- 월드 드롭·픽업·루팅 생성 규칙
- 보상 테이블
- 제작·분해·수리
- 아이템 희귀도와 랜덤 옵션
- 내구도·개조·강화
- PlayerStorage 전체 UX
- 다중 차량 공유 창고
- SaveGame·재접속 영구 저장
- 서버 권한·복제·거래
- 아이템 정렬·필터·검색 완성형 UI
- 무게 초과 이동 페널티
```

이 항목은 Foundation 타입에 억지로 완성형 필드를 넣지 않고 실제 후속 기능이 필요할 때 확장한다.

---

## 5. 데이터 책임

```text
Domain DataAsset
= EquipmentPresetData, VehicleDefenseData, 미래 AmmoData 등 게임 규칙·스탯

Inventory Item Definition
= 인벤토리에서 표시·분류하고 어떤 Domain DataAsset을 가리키는지 정의

Inventory Item Instance
= 플레이어가 실제로 소유하는 고유 개체 또는 Stack

Inventory Container
= 현재 Item Instance가 속한 소유 위치

Inventory Reservation
= 시간 소모 액션 동안 Source Item과 Destination Capacity를 잠금

Inventory Transaction
= 완료 순간 모든 이동을 한 번에 Commit하거나 전부 Rollback
```

질량·공격력·방어력 같은 도메인 수치를 Inventory Definition에 복제하지 않는다.

```text
장비 질량
→ TurretMountData + WeaponData

방어 질량
→ VehicleDefenseData

탄약 질량
→ 미래 AmmoData
```

---

## 6. 피팅 연동 계약

필드 장착 시작 시:

```text
1. Fitting이 빈 MountProfile과 아이템 호환성 검증
2. Inventory가 Source Item 존재·접근 가능 여부 검증
3. Inventory가 Item Instance 예약
4. Inventory가 Destination Mount Slot 또는 반환 Container Capacity 예약
5. Fitting Action이 시간 진행
```

완료 시:

```text
1. Fitting이 최종 조건 재검증
2. Inventory Transaction 준비
3. 새 Fitting Snapshot 생성
4. Runtime 원자 적용 시도
5. 성공하면 Inventory Transaction Commit
6. 실패하면 기존 Applied Snapshot 유지와 Transaction Rollback
```

실제 구현에서는 Runtime 적용과 Inventory Commit 순서를 하나의 Coordinator가 관리해야 한다. 인벤토리가 VehicleWeaponComp·VehicleDefenseComp·Chaos를 직접 호출하지 않는다.

---

## 7. 작업 패키지

| ID | 작업 | 결과 | 상태 |
|---|---|---|---|
| `INV-P0-00` | Inventory Foundation 계약 | 책임·범위·불변식·의존 관계 | Done — Documentation |
| `INV-P0-01` | Item Identity와 Definition | ItemHandle·InstanceId·Domain Definition 연결 | Done — Code·Automation·Build PASS |
| `INV-P0-02` | Container와 Access Query | VehicleCargo·Mounted 상태·Capacity·접근 조회 | Done — Code·Automation·Build PASS |
| `INV-P0-03` | Reservation과 Atomic Transfer | 예약·Commit·Rollback·중복 소유 방지 | Done — Code·Automation·Build PASS |

| `INV-P0-04` | Fitting Inventory Adapter | ItemInstance → Equipment·Defense 선택·Binding·기존 Snapshot 변환 | Done — Code·Automation·Build PASS |

| `INV-P0-05` | ViewData와 Events | 읽기 전용 Item/Container Snapshot·Reservation 표시·외부 Fitting Hint·의미 ChangeSet | Done — Code·Automation·Build PASS |
| `INV-P0-06` | Integration Verification | Coordinator 원자 completion·보상 Rollback·실제 FittingComp 조합 / 이후 FFIT Permission·Timed Action·Field Mass Reapply 연동 | Done — Historical Technical Integration PASS / FFIT-P0-05 USER frontend 분리 |

---

## 8. 선행·후속 관계

```text
INV-P0-01~03
→ FFIT-P0-02 Timed Action and Reservation 착수 조건

INV-P0-04
→ FFIT-P0-03 Atomic Equip / Unequip 착수 조건

INV-P0-05
→ Fitting Screen의 Inventory 목록 표시 조건

SaveGame·Loot·Economy
→ CF-FQ-035 P0 완료 후 별도 후속 Feature
```

`FIT-P0-04 Sortie Apply Runtime`은 기존 FittingData 기반으로 먼저 구현할 수 있다. 다만 필드 장착과 실제 소유 장비 연결은 `INV-P0-03~04` 없이는 완료 처리하지 않는다.

---

## 9. 완료 조건

```text
- 하나의 Item Instance가 동시에 두 Container에 존재하지 않음
- 예약된 Item을 다른 Transfer가 사용할 수 없음
- Destination Capacity를 선점한 뒤에만 시간 액션 시작 가능
- 취소 시 Item·Container·Mounted 상태가 시작 전과 동일
- Commit 실패 시 부분 이동 없음
- 같은 Transaction의 중복 Commit이 중복 아이템을 만들지 않음
- Inventory가 장착 호환·무기 Runtime·방어 Runtime을 직접 소유하지 않음
- Pawn 없는 Automation PASS
- 공식 Editor Build PASS
```

---

## 10. 보호 범위

```text
- 기존 EquipmentPresetData·WeaponData·VehicleDefenseData 원본을 인벤토리 저장소로 변경하지 않는다.
- VehicleData MountProfiles와 HardpointSlots를 Inventory에서 재정의하지 않는다.
- CF-FQ-031 Ammo 수량·예약·재장전 Runtime을 Inventory로 이동하지 않는다.
- CF-FQ-034 Snapshot 검증과 질량 계산을 Inventory에 복제하지 않는다.
- CF-FQ-032 UISubsystem이 Inventory 상태를 소유하게 하지 않는다.
- `INV-P0-01`의 Inventory Identity·Definition C++ 5개 파일과 계약을 변경하지 않는다.
- `INV-P0-01` Item Identity·Definition과 `INV-P0-02` Container·Capacity·Access Query 계약을 변경하지 않는다.
- `INV-P0-03` Reservation·Atomic Transfer 계약과 구현을 변경하지 않는다.
- `INV-P0-04` Fitting Adapter의 ItemInstanceId 단일 소유 위치, 현재 차량 접근 가능 상태와 Active Reservation 부재 검증 계약을 변경하지 않는다.
- `INV-P0-05`는 `CFInventoryViewData.h/.cpp`와 Pawn 없는 `CFInventoryViewTests.cpp`만 추가해 기존 Container·Definition·TransferLedger를 읽는 불변 Snapshot Builder와 의미 ChangeSet을 제공한다.
- ViewData는 현재 차량 Owner의 VehicleCargo·MountedEquipment만 다루고 다른 Owner의 Item을 노출하지 않는다.
- `CompatibilityHint`는 외부 Fitting 결과를 합성할 뿐 Inventory가 Mount 호환성을 계산하지 않는다.
- Snapshot Diff는 Item/Container/Reservation 표시 변화만 표현하며 Transaction 완료·실패 Operation Result를 추측하지 않는다. 후속 Field Fitting Coordinator가 해당 명시적 결과를 소유한다.
- Adapter는 ItemInstanceId의 단일 소유 위치, 현재 차량 접근 가능 상태와 Active Reservation 부재를 검증한다.
- ItemDefinitionId는 강타입 `UCFEquipmentItemData` 또는 `UCFDefenseItemData`로 해석하고 기존 EquipmentPresetData·VehicleDefenseData 참조만 Binding한다.
- 같은 ItemInstanceId의 복수 Mount·Defense 중복 선택을 거부하고 Binding은 VehicleData.MountProfiles 순서로 결정론적으로 생성한다.
- Mount 호환과 질량은 Transient `UCFVehicleFittingData`의 기존 `BuildFittingSnapshot()` 결과를 사용하며 Inventory에 계산을 복제하지 않는다.
- Adapter는 Container·Reservation을 변경하거나 Weapon·Defense·VehicleMovement Runtime을 직접 적용하지 않는다.
- Inventory 이동 Commit과 기존 TransferLedger 계약, Field Timed Action, Field Runtime Mass Reapply, UI Widget/Blueprint, Unreal DataAsset, PhysicsAsset, Map과 SaveGame은 `INV-P0-05`에서 수정하지 않는다.
- `INV-P0-06` Formal Integration은 `Inventory Prepare → Timed Action Reservation → Runtime Prepare → Weapon·Defense·필요 시 Chaos Mass Commit → Inventory Commit` 순서를 사용하고 실패 시 직전 Applied Runtime·질량 복원 + Inventory Rollback을 함께 수행한다.
- Coordinator 한 호출은 기존 Field Fitting 계약대로 단일 Equip 또는 Unequip Transfer만 처리한다. Replace는 Unequip 완료 후 별도 Equip이다.
- `FFIT-P0-01~04` 기술 경로가 완료되어 mass-changing 후보도 `ICFFieldFitMassRuntime`이 제공되면 같은 transaction에 포함된다. Mass Runtime이 없는 호출은 계속 0.01kg same-mass 안전 경계를 유지한다.
- Runtime/질량 복구 실패는 `RecoveryFailed`로 명시 노출하며 부분 성공을 정상 Rollback으로 숨기지 않는다.
- `FIT-P0-06 Fitting ViewData and Debug`의 C++ ViewData·Blueprint Contract도 Technical PASS다.
- Historical v0.9.0에서는 Inventory Foundation의 남은 M6 범위를 `FFIT-P0-05 Field Fitting UI and PIE`와 사용자-facing 실제 사용 흐름·가독성 검증으로 기록했다. 2026-09-14 Rebaseline에서 이 USER frontend는 Foundation closure gate에서 분리되어 `Superseded / Not Executed`가 되었으며 USER PASS로 간주하지 않는다.


```


---

## 11. Changelog

### v0.10.0 - 2026-09-14

```text
- CF-FQ-035를 current Source/System 기준으로 Rebaseline해 SUPERSEDED / CLOSE, Done / Historical + Retained Path로 전환했다.
- ItemInstance, VehicleCargo/MountedEquipment, Access/Capacity, Reservation/Atomic Transfer, ViewData, Fitting Adapter와 Field Fit completion/recovery는 실제 Source에 유지되어 있으며 Current owner를 main_game `Document/Systems/Vehicles/VehicleInventory.md v1.0.0`으로 승격했다.
- INV-P0-00~05 및 INV-P0-06의 FFIT-P0-01~04 + FIT-P0-06 기술 evidence는 Historical Technical PASS로 보존하며 반복하지 않는다.
- FFIT-P0-05 Field Fitting UI and PIE는 실제 USER PASS를 수행한 것이 아니라 `Superseded / Not Executed as CF-FQ-035 closure gate`다. formal ownership-aware frontend가 실제 요구가 되면 새 Product/UI lifecycle을 연다.
- CF-FQ-041 RuntimeApply는 현재 장비/차량 즉시 적용 UX를 제공하지만 Inventory ownership/Reservation/Transaction을 조작하지 않는 별도 경로이며 Inventory USER PASS를 대체하지 않는다.
- SaveGame, 계정/서버, PlayerStorage, loot/economy는 기존 P0 제외 범위 그대로 Deferred다.
- Source/Asset mutation 0, Build/Automation/PIE 재실행 0, 새 USER PASS 0이다. G0~G4 PASS / G5 Deferred다.
```

Migration: `InventoryFoundationPlan.md v0.10.0`은 Historical evidence owner다. 새 작업에서 `FFIT-P0-05`를 current CF-FQ-035 next gate로 자동 재개하지 말고 Inventory Foundation 현재 계약은 main_game `VehicleInventory.md v1.0.0`, Fitting/Mass runtime은 `VehicleRuntime.md v1.3.0`, 즉시 적용 UX는 CF-FQ-041 RuntimeApply 경계를 우선한다.

### v0.9.0 - 2026-08-15

```text
- cross-feature FIT-P0-06 Fitting ViewData and Debug의 C++ ViewData·Blueprint Contract Technical PASS를 반영했다.
- FittingSnapshot의 Mount·Defense·Ammo·Mass·Validation 결과를 재계산하지 않는 read-only Fitting ViewData가 준비되어 Inventory/Field UI가 공통 표시 계약을 소비할 수 있게 됐다.
- 공식 Build `ad3452d3add74785bbdb41c667dce728`, FIT-P0-06 1/1, Fitting 22/22, Inventory 12/12, Full CarFight `59aae9f0ac204e13a957dbbfe8cdbaec` 84/84 Success를 기록했다.
- Full CarFight result SHA-256은 `4e49080b401ada72e243e8bd1a98e58e9bdd9d3af4471a03b78f7d7ca277c139`이다.
- 현재 CF-FQ-035/034의 원격 기술 선행 Gate는 모두 닫혔다. 남은 INV-P0-06은 FFIT-P0-05 Field UI·PIE와 실제 사용자 흐름·가독성 검증이다.
- USER-facing Gate가 남아 있으므로 INV-P0-06 또는 CF-FQ-035를 Done으로 승격하지 않는다.
```

### v0.8.0 - 2026-08-15

```text
- INV-P0-06의 cross-feature FFIT-P0-01~04 기술 통합 완료 상태를 반영했다.
- Permission·Timed Action·Reservation·Atomic completion과 mass-changing Chaos Runtime Apply가 하나의 공식 Field Fitting 흐름으로 연결됐다.
- Runtime 또는 Mass 복구 실패는 RecoveryFailed로 분리하고 Inventory Reservation은 Rollback되는 계약을 검증했다.
- 실제 M_VehicleDefensePIE에서 1570/1837.377kg → 1620/1887.377kg → 원복을 확인했다.
- 최종 Build `cd7207084dfd49c4a28b607d8577307a`, FFIT-P0-04 4/4, Fitting 21/21, Inventory 12/12, Full CarFight 83/83 Success를 기록했다.
- 사용자-facing Field Fitting UI·USER PIE가 남아 있으므로 INV-P0-06은 In Progress를 유지하고 다음 원격 기술 선행을 FIT-P0-06 ViewData and Debug로 이동했다.
```

### v0.7.0 - 2026-08-14

```text
- INV-P0-06 / M6 Integration Verification의 Coordinator Foundation을 구현·검증했다.
- UCFVehicleFittingComp v1.3.0에 검증 완료 FittingSnapshot 직접 Prepare, Applied Runtime Checkpoint 캡처와 Inventory Commit 실패 보상 복원을 추가했다.
- FCFFieldFitCoordinator가 Inventory Prepare → Runtime Prepare·Commit → Inventory Commit을 조율하고 Runtime 실패 또는 Inventory Commit 실패에서 명시적 보상 Rollback을 수행한다.
- 실제 FCFFieldFitRuntimeAdapter는 UCFVehicleFittingComp와 ICFFittingRuntimeApplyAdapter를 연결하며 현재 Field Mass Reapply 부재를 숨기지 않고 same-mass 0.01kg 이내 후보만 허용한다.
- mass-changing 후보는 RuntimeMassReapplyUnsupported로 Runtime Prepare에서 거부되고 Prepared Inventory Transaction·Reservation은 Rollback된다.
- FFIT-P0-01 Permission, FFIT-P0-02 Timed Action, FFIT-P0-04 Field Mass Reapply는 아직 미구현이며 이번 M6 기반을 FFIT-P0-03 전체 완료로 승격하지 않는다.
- 최종 공식 UE 5.8 Editor Build `e2ef556b64484a09ba8b3544c62344e3`은 Exit Code 0으로 PASS했다.
- targeted `CarFight.Inventory.INV_P0_06` Automation `c02cd6ce84904e42b618f7171150f890`은 3/3 Success·0 Fail, SHA-256 `c0d7475f715e13a2e9af53e795484ab3c6eb82a8cd037d05505dfd52f57860af`이다.
- Inventory 전체 회귀 `fa79fd3ee0234cb8859ca4a93ddc9e19`은 12/12 Success·0 Fail, SHA-256 `d50c75e8c53381b9e1231016122b0fae7dab8ece3ec8a076efb85cbfd52f3b70`이다.
- 전체 CarFight 회귀 `70423e50820941f4886aa95904622fc8`은 71/71 Success·0 Fail, SHA-256 `3857cfacc6178066220c2a2278b55797115bca60ce3880b7be47182d4bbd7c9a`이다.
- 다음 원격 기술 단계는 형식상 미착수인 CF-FQ-034 FFIT-P0-01 Permission and Blocker Query다.
```

### v0.6.0 - 2026-08-14

```text
- INV-P0-05 ViewData and Events를 구현 완료했다.
- FCFInventoryViewBuilder가 기존 Container·Definition·TransferLedger·AccessContext를 변경하지 않고 현재 차량의 결정론적 읽기 전용 Snapshot을 생성하도록 했다.
- Item Row에 ItemInstanceId·Definition·DisplayName·Domain·Quantity·Container/Slot·Reservation·Access와 외부 Fitting CompatibilityHint를 제공한다.
- Container ViewData에 접근 상태·사용 슬롯·남은 슬롯을 제공하고 다른 Owner의 Item은 현재 차량 Snapshot에서 격리한다.
- 이전/현재 Snapshot을 비교해 ItemAdded/Removed/Location/Reservation/Presentation과 ContainerChanged를 계산하는 의미 ChangeSet을 추가했다.
- Transaction Completed/Failed는 Snapshot 차이로 추측하지 않고 후속 Field Fitting Coordinator의 명시적 Operation Result가 소유하도록 경계를 고정했다.
- 공식 UE 5.8 Editor Build `30503595ba074c63ba8a6bb87f7a2645`는 UHT·신규 Source 직접 Compile·Link를 포함해 Exit Code 0으로 PASS했다.
- targeted `CarFight.Inventory.INV_P0_05` Automation `7cb4c11aa51540d6a3ee181ee2ec48f1`은 2/2 Success·0 Fail이다.
- 전체 Inventory 회귀 `0fd8c724fc5141dba3d064874bc87438`은 9/9 Success·0 Fail이고 결과 JSON SHA-256은 `4058779b9fea9349bd88fb2465f341e6e43e8be66967f6ad37b3b5c49b2505ac`이다.
- 전체 CarFight 회귀 `5e5f1299f6b449738b6d24607afb32d0`은 68/68 Success·0 Fail이고 결과 JSON SHA-256은 `c952b2631d6556be9ce9572b97d9ae9ec55232d3937962fd88137f0f362f95d1`이다.
- 작업 전용 `Tools/RunInvAutomation.ps1 v1.0.1`은 이번 검증의 Execution Method이며 공용 Tool로 승격하지 않는다. 최초 v1.0.0 실행의 PowerShell regex 파싱 오류는 기능 테스트 실패가 아니다.
- CF-FQ-032는 USER Visual 두 Gate를 그대로 보존한 Paused 체크포인트로 전환하고 CF-FQ-035를 현재 단일 Active로 전환한다.
- 다음 단계는 INV-P0-06 Integration Verification이며 Prepared Inventory Transaction·Fitting Runtime Apply·Inventory Commit·보상 Rollback을 조율하는 Field Fitting Coordinator 기술 경계를 먼저 구현한다.
```

### v0.5.0 - 2026-08-02

```text
- INV-P0-04 Fitting Inventory Adapter를 구현 완료했다.
- ItemInstanceId 기반 Equipment Mount 선택과 Defense Override 입력을 추가했다.
- 현재 차량 VehicleCargo·MountedEquipment 소유권과 접근 가능 상태를 검증하고 다른 차량·접근 차단 Item을 거부한다.
- Active Item Reservation이 있는 Instance의 피팅 선택을 거부한다.
- ItemDefinitionId를 UCFEquipmentItemData·UCFDefenseItemData로 단일 해석해 EquipmentPresetData·VehicleDefenseData를 Binding한다.
- 같은 ItemInstanceId의 복수 Mount 또는 Defense 중복 선택을 거부하고 같은 Definition의 서로 다른 Instance는 구분한다.
- Mount Binding은 입력 배열 순서와 무관하게 VehicleData.MountProfiles 순서로 생성한다.
- 기존 UCFVehicleFittingData.BuildFittingSnapshot을 사용해 호환성·질량을 검증하고 MountTypeMismatch 등 기존 문제를 그대로 전달한다.
- Adapter가 Inventory Container·Reservation, Weapon·Defense·VehicleMovement Runtime을 변경하지 않는 경계를 확인했다.
- Pawn 없는 CarFight.Inventory.INV_P0_04.FittingAdapter — Success / Warning 0 / Error 0을 확인했다.
- 전체 CarFight Automation 42/42 Success, 필수 회귀 20/20 Success를 확인했다.
- 공식 Editor Build 23af46f3a13a4ba88e9878bcdc225a50 / Exit Code 0을 확인했다.
- FIT-P0-00~05 구현, CF-FQ-032 Active 체크포인트와 Unreal Asset은 변경하지 않았다.
```

### v0.4.0 - 2026-08-01


```text
- INV-P0-03 Reservation and Atomic Transfer를 구현 완료했다.
- FGuid 기반 ReservationId와 TransactionId, Active·Cancelled·Consumed Reservation 상태를 추가했다.
- 하나의 Prepared Transaction이 Item Reservation과 Destination Slot·Capacity Reservation을 함께 소유하도록 했다.
- 같은 Item, 같은 Destination Slot, 누적 Slot·Mass Capacity 이중 예약을 거부한다.
- Prepare와 Rollback은 Container를 변경하지 않으며 어느 한 Reservation ID 취소도 같은 Transaction의 두 Reservation을 함께 취소·Rollback하고 반복 호출은 멱등적으로 처리한다.

- Commit은 Source·Destination·Reservation·Capacity를 재검증하고 후보 Container 복사본 전체 Validation 후에만 원본을 교체한다.
- Commit 실패 시 Source·Destination 무변경, Transaction Prepared 유지와 명시적 Rollback을 확인했다.
- 같은 Transaction의 중복 Commit은 AlreadyCommitted로 종료되고 Item 복제·추가 이동이 발생하지 않는다.
- Pawn 없는 INV-P0-03 Automation 2/2 Success, 전체 CarFight Automation 38/38 Success를 확인했다.
- 기존 필수 Damage·Fitting·Launcher·Projectile 회귀 17/17 Success를 확인했다.
- 공식 Editor Build 32ca1cacb47548d4aa24ee12372baf67 / Exit Code 0을 확인했다.

- Fitting Runtime, UI, SaveGame과 Unreal Asset은 수정하지 않았다.
- CF-FQ-032 Active 체크포인트는 변경하지 않았다.
```

### v0.3.0 - 2026-08-01


```text
- INV-P0-02 Container and Access Query를 구현 완료했다.
- FGuid 기반 FCFInventoryOwnerId와 FCFInventoryContainerId를 추가했다.
- VehicleCargo와 MountedEquipment의 Owner·Container·ContainerSlotId·ItemInstanceId 소유 위치를 분리했다.
- 슬롯 수와 선택적 질량 한도를 평가하는 순수 Capacity Query를 추가했다.
- 질량 값은 Inventory Definition이나 Instance에 복제하지 않고 호출자가 Domain DataAsset에서 해석한 값만 Query 입력으로 사용한다.
- 현재 차량 Owner의 VehicleCargo·MountedEquipment 접근과 다른 차량 격리, Item 단일 위치·중복 소유 조회를 추가했다.
- Pawn 없는 INV-P0-02 Automation 2/2 Success, 전체 CarFight Automation 36/36 Success를 확인했다.
- 기존 필수 Damage·Fitting·Launcher·Projectile 회귀 17/17 Success를 확인했다.
- 공식 Editor Build 36260a7a70dd48e899f27dcd038efb2b / Exit Code 0을 확인했다.
- Reservation, Atomic Transfer, Fitting Runtime, UI와 Unreal Asset은 수정하지 않았다.
- CF-FQ-032 Active 체크포인트는 변경하지 않았다.
```

### v0.2.0 - 2026-08-01

```text
- INV-P0-01 Item Identity and Definition을 구현 완료했다.
- FGuid 기반 FCFItemInstanceId, FPrimaryAssetId 기반 FCFInventoryItemHandle과 Quantity 1 FCFInventoryItemInstance를 추가했다.
- UCFInventoryItemData 기반과 UCFEquipmentItemData·UCFDefenseItemData 강타입 Definition을 추가했다.
- 잘못된 Domain DataAsset 참조는 Definition 검증과 ItemInstance 생성에서 모두 거부한다.
- Pawn 없는 Inventory Automation 2/2 Success, 전체 CarFight Automation 34/34 Success를 확인했다.
- 기존 필수 Damage·Fitting·Launcher·Projectile 회귀 17/17 Success를 확인했다.
- 공식 Editor Build 32cd34dd2424407b94c7377dd19a5fce / Exit Code 0을 확인했다.
- Blueprint, Unreal DataAsset, PhysicsAsset, Map, SaveGame과 기존 기능 소스는 수정하지 않았다.
- CF-FQ-032 Active 체크포인트는 변경하지 않았다.
```

### v0.1.0 - 2026-08-01

```text
- CF-FQ-035 Inventory Foundation을 별도 P1 Ready 기능으로 정의했다.
- 피팅 P0에 필요한 Item Instance, VehicleCargo, 예약과 원자 Transfer 범위를 확정했다.
- 장착 호환·시간 액션·Runtime 적용은 CF-FQ-034가 소유하도록 경계를 분리했다.
- 상점·루팅·경제·SaveGame·네트워크는 P0 제외로 유지했다.
- Source와 Unreal Asset은 수정하지 않았다.
```

---

## 12. Migration

```text
- 기존 VehicleFittingData의 EquipmentPresetData 참조는 저장된 빌드 템플릿과 Legacy 테스트 입력으로 유지한다.
- 필드 피팅은 실제 소유권 확인을 위해 ItemInstanceId를 사용하는 Draft·Action 계약을 추가한다.
- 기존 VehicleFittingData의 DataAsset 직접 참조는 Saved Template과 Legacy 출격 입력으로 유지한다.
- 실제 Inventory Draft는 `FCFInventoryFitRequest`와 `FCFInventoryFitAdapter::BuildFittingBinding`을 사용해 ItemInstanceId를 검증·해석한다.
- 성공 결과의 MountBindings·DefenseBinding이 실제 Instance 추적을 소유하고 FittingSnapshot은 기존 호환·질량 해석 결과를 소유한다.
- Adapter 결과가 `FittingSnapshotInvalid`이면 Binding을 Runtime Apply 또는 Inventory 이동 Commit에 사용하지 않는다.
- 시간 소모 액션은 INV-P0-03 Ledger의 Prepare 결과가 성공한 뒤에만 시작한다.

- Action 취소 시 어느 한 Reservation ID로 `CancelReservation`을 호출하거나 Transaction ID로 `RollbackTransfer`를 호출하며, 두 경로 모두 Item·Destination Reservation을 함께 정리한다.

- Commit 실패는 자동 부분 Rollback이 아니라 Prepared 무변경 상태이므로 상위 Coordinator가 재시도 또는 Rollback을 명시적으로 선택한다.
- INV-P0-05부터 UI/후속 시스템은 Container·Ledger 내부 배열 대신 `FCFInventoryViewBuilder`가 생성한 읽기 전용 Snapshot을 소비한다.
- Reservation 표시는 `TransferLedger.IsItemReserved()` 결과를 Snapshot에 반영하고, 위치·예약·표시 변화는 `DiffSnapshots()` ChangeSet으로 판정한다.
- Transaction 완료·실패는 Snapshot Diff로 재구성하지 않고 `FCFFieldFitCompletionResult`의 명시적 Operation Result에서 전달한다.
- M6 Formal Integration은 FFIT-P0-01 Permission, P0-02 Timed Action·Reservation, P0-03 Atomic handoff와 P0-04 Chaos Field Mass Reapply까지 Technical PASS다.
- Historical v0.9.0 당시 actual Chaos PIE의 mass-changing 적용·복원과 RecoveryFailed 분류까지 기술 검증됐고, `FIT-P0-06 Fitting ViewData and Debug` C++ ViewData·Blueprint Contract도 Technical PASS였다.
- 당시 남은 cross-feature Gate는 `FFIT-P0-05 Field Fitting UI and PIE`와 사용자 직접 검증이었다.
- 2026-09-14 Rebaseline 이후 `FFIT-P0-05`는 `Superseded / Not Executed`이며 USER PASS가 아니다. formal ownership-aware Inventory/Fitting frontend가 실제 Product 요구가 되면 `VehicleInventory.md v1.0.0` 위에서 새 lifecycle을 연다.
- 현재 단일 Active Feature는 `CF-FQ-039`; CF-FQ-035와 CF-FQ-032를 Active/Paused로 복원하지 않는다.


```



