# Vehicle Inventory Foundation

- Version: 1.0.0
- Date: 2026-09-14
- Status: Current Foundation / Formal USER Frontend Deferred
- Origin: `CF-FQ-035 인벤토리 Foundation`

---

## 1. 목적

이 문서는 CarFight의 **실제 소유 Item Instance와 차량 Inventory Container 사이의 소유권·예약·원자 이동 계약**을 Current System으로 기록한다.

Inventory는 장비 선택 UI나 Fitting 배열의 별칭이 아니다. 현재 Source에 존재하는 Foundation의 핵심 흐름은 다음과 같다.

```text
Inventory Item Definition
→ unique ItemInstanceId
→ VehicleCargo / MountedEquipment 소유 위치
→ Access / Capacity Query
→ Item + Destination Reservation
→ Prepare / Commit / Rollback Atomic Transfer
→ read-only Inventory ViewData
→ ItemInstance 기반 Fitting Adapter
→ Field Fitting completion transaction
```

현재 구현은 Foundation과 Field Fitting 기술 경계까지 제공하지만, **정식 ownership-aware Inventory/Fitting USER 화면과 이를 장기 보유하는 gameplay state owner는 Current 완료 범위로 주장하지 않는다.**

---

## 2. Current Source Authority

현재 Foundation의 주요 Source authority는 다음과 같다.

```text
UE/Source/CarFight_Re/Public/CFInventoryTypes.h
UE/Source/CarFight_Re/Public/CFInventoryContainer.h
UE/Source/CarFight_Re/Public/CFInventoryTransfer.h
UE/Source/CarFight_Re/Public/CFInventoryViewData.h
UE/Source/CarFight_Re/Public/CFInventoryFitAdapter.h
UE/Source/CarFight_Re/Public/CFFieldFitAction.h
UE/Source/CarFight_Re/Public/CFFieldFitCoordinator.h

UE/Source/CarFight_Re/Private/CFInventoryContainer.cpp
UE/Source/CarFight_Re/Private/CFInventoryTransfer.cpp
UE/Source/CarFight_Re/Private/CFInventoryViewData.cpp
UE/Source/CarFight_Re/Private/CFInventoryFitAdapter.cpp
UE/Source/CarFight_Re/Private/CFFieldFitAction.cpp
UE/Source/CarFight_Re/Private/CFFieldFitCoordinator.cpp
```

대표 Historical Plan의 Build/Automation/PIE evidence는 `Document/Plan/InventoryFoundation/InventoryFoundationPlan.md v0.10.0`이 보존한다. 이 Current 문서는 과거 job ID와 반복 검증 로그를 복제하지 않는다.

---

## 3. 소유권과 Container 계약

### 3.1 Item Instance

`FCFItemInstanceId`는 같은 Definition을 사용하는 여러 실제 소유 개체를 구분하는 안정적인 ID다. `FCFInventoryItemInstance`는 정적 `ItemDefinitionId`와 실제 `ItemInstanceId`를 분리한다.

현재 Equipment/Defense P0는 Quantity 1 고유 인스턴스 계약을 사용한다. Definition의 게임 스탯을 Inventory Instance에 복제하지 않는다.

### 3.2 Vehicle Container

현재 P0 Container 종류는 다음 둘이다.

```text
VehicleCargo
MountedEquipment
```

`FCFInventoryContainerState`는 Owner, Container ID, Slot ID, 실제 Item Instance Entry와 SlotCount/선택적 Mass Capacity를 소유한다.

질량은 Inventory가 자체 SSOT로 저장하지 않는다. Domain DataAsset/Fitting에서 외부 해석한 현재·요청 질량을 Capacity Query 입력으로만 사용한다.

### 3.3 단일 소유 불변식

`FCFInventoryAccessQuery`는 Container 집합에서 ItemInstanceId가 정확히 한 위치에만 존재하는지 검증한다. 같은 실제 Item이 VehicleCargo와 MountedEquipment 또는 두 Slot에 동시에 존재하는 상태는 정상 Current 계약이 아니다.

---

## 4. Reservation / Atomic Transfer

`FCFInventoryTransferLedger`는 Pawn과 World 없이 Reservation과 Transaction 수명을 소유한다.

정상 Transfer는 다음 순서를 따른다.

```text
PrepareTransfer
→ Source Item 검증
→ Destination Slot/Capacity 검증
→ Item Reservation + Destination Capacity Reservation
→ Container 원본은 아직 변경하지 않음

CommitTransfer
→ Prepared 조건 재검증
→ 후보 Container 복사본에서 Source 제거 + Destination 추가
→ 전체 Container 계약 검증
→ 성공한 경우에만 원본 Container 집합을 한 번 교체

RollbackTransfer / CancelReservation
→ Prepared Reservation 정리
→ Container 원본은 시작 전 상태 유지
```

같은 Transaction의 중복 Commit은 아이템 복제를 만들지 않아야 한다. Commit 실패를 부분 성공으로 숨기지 않으며, 상위 Coordinator가 명시적으로 rollback/recovery를 처리한다.

---

## 5. Fitting / Runtime 경계

Inventory와 Vehicle Fitting의 책임을 합치지 않는다.

```text
Inventory
= 실제 Item Instance 소유권
= Container 접근·Capacity
= Reservation
= Atomic Transfer
= read-only Inventory ViewData

Vehicle Fitting
= Mount/Hardpoint 호환
= Fitting Snapshot
= Equipment/Defense/Ammo/Mass 해석
= Runtime Apply 준비

Field Fit Coordinator
= Runtime 적용과 Inventory Commit의 completion 순서
= 실패 시 Runtime/Mass recovery + Inventory rollback 조율
```

`FCFInventoryFitAdapter`는 ItemInstanceId의 단일 소유 위치, 현재 차량 접근 가능 상태, Active Reservation 부재와 강타입 Definition을 검증한 뒤 기존 Fitting Snapshot 생성 경로에 연결한다. Inventory가 Mount 호환이나 총질량을 다시 계산하지 않는다.

현재 `VehicleRuntime.md`가 Initial/Field Fitting Runtime과 실제 Mass 적용·복구를 소유한다. Inventory는 Weapon/Defense/Chaos Runtime을 직접 변경하지 않는다.

---

## 6. UI와 RuntimeApply 경계

`FCFInventoryViewBuilder`는 현재 차량의 VehicleCargo/MountedEquipment를 읽어 결정론적 read-only Snapshot과 의미 ChangeSet을 만든다. UI는 Container Entry, Reservation, Transaction 내부 상태를 직접 수정하지 않는다.

하지만 현재 Product UI에서 `Inventory`, `VehicleCargo`, `MountedEquipment`, `FieldFit`을 직접 소유하는 정식 USER Widget 경로는 확인되지 않았다. 과거 `FFIT-P0-05 Field Fitting UI and PIE`는 실제 USER PASS를 수행한 것으로 취급하지 않는다.

현재 `CF-FQ-041 RuntimeApply`는 장비/차량을 즉시 시험·시연하기 위한 별도 경로다.

```text
RuntimeApply
= Catalog 기반 선택
= ownership 확인 없음
= Inventory Container / Reservation / Transaction mutation 없음
= 기존 Fitting Runtime Apply 재사용

Formal Inventory/Fitting UI
= owned ItemInstance 선택
= ownership / reservation / capacity / compatibility
= Field Fitting timed action
= Atomic ownership handoff
```

따라서 RuntimeApply는 **현재 장비 선택 UX 필요를 충족하지만 Inventory Foundation의 소유권 의미를 대체하거나 폐기하지 않는다.** 반대로 RuntimeApply USER PASS를 Inventory USER UI PASS로 확대하지도 않는다.

정식 ownership-aware Field Fitting/Inventory USER 화면이 실제 게임 요구로 다시 필요해질 때는 이 Current Foundation 위에 별도 Product/UI lifecycle을 연다.

---

## 7. 현재 비책임 / Deferred

다음은 현재 Foundation 완료 범위가 아니다.

```text
- PlayerStorage 전체 UX
- 주변 보급소 / 월드 Loot Container
- 상점·구매·판매·경제
- 제작·분해·수리
- SaveGame / 재접속 영구 저장
- 계정 기반 장기 Inventory
- 서버 권한 / Replication / 거래
- 다중 차량 공유 창고
- 완성형 정렬·필터·검색 UI
- 무게 초과 이동 페널티
```

이 항목은 미래 확장 가능성일 뿐 `CF-FQ-035` closure를 막는 residual implementation으로 취급하지 않는다.

정식 Field Fitting USER UI/PIE도 **Deferred Product/UI debt**이며 현재 USER PASS가 아니다.

---

## 8. Rebaseline 판정

2026-09-14 Rebaseline에서 다음을 확인했다.

```text
INV-P0-00~05
= Historical Technical PASS 보존

INV-P0-06 technical integration
= FFIT-P0-01~04 + FIT-P0-06 Technical PASS 보존
= Field Fitting completion / mass-changing runtime / recovery classification까지 기존 evidence 보존

FFIT-P0-05 Field Fitting UI and PIE
= Superseded / Not Executed as CF-FQ-035 closure gate
= USER PASS 아님
= 미래 Formal Inventory/Fitting frontend가 필요할 때 새 lifecycle
```

따라서 `CF-FQ-035` Feature lifecycle은 `SUPERSEDED / CLOSE`로 종료하고, 고유 Foundation 계약은 이 Current System으로 승격한다.

---

## 9. Changelog

### v1.0.0 - 2026-09-14

- `CF-FQ-035 Inventory Foundation` Rebaseline closure에서 실제 Source의 ItemInstance, VehicleCargo/MountedEquipment, Access/Capacity, Reservation/Atomic Transfer, ViewData, Fitting Adapter와 Field Fit completion 경계를 Current System으로 승격했다.
- RuntimeApply가 현재 장비 선택 UX를 담당하지만 ownership/Reservation/Inventory mutation을 의도적으로 수행하지 않는 별도 경로임을 명시했다.
- 과거 `FFIT-P0-05 Field Fitting UI and PIE`는 USER PASS가 아닌 `Superseded / Not Executed`로 분리하고 정식 ownership-aware UI가 실제로 필요할 때 새 lifecycle을 열도록 했다.
- SaveGame, 계정/서버, PlayerStorage, loot/economy를 현재 Foundation에 소급 추가하지 않았다.
- Source/Asset mutation, Build/Automation/PIE 재실행은 0이다.

Migration: Inventory 소유권·Container·Reservation·Atomic Transfer의 현재 판단은 이 문서와 실제 Source를 우선한다. `RuntimeApply`를 Inventory ownership 경로로 해석하지 말고, Historical `InventoryFoundationPlan`의 `FFIT-P0-05`를 current next gate로 자동 재개하지 않는다.
