# Runtime Content Apply Plan

- Version: 0.1.18
- Date: 2026-09-14
- Status: Ready Plan / RTA-P0-05 Historical USER PASS preserved / Multi-Mount Empty-State remediation Build PASS + fresh RuntimeApply 16/16 PASS / RTA-P0-06 Ready
- Feature: `CF-FQ-041 런타임 콘텐츠 적용 메뉴`
- Priority: P2
- Current Active Feature: `CF-FQ-039 Production UI Visual Rework` 유지
- Representative Plan: `Document/Plan/RuntimeApply/RuntimeApplyPlan.md`
- Related Current Systems: `Document/Systems/Vehicles/RuntimeApply.md v1.0.0`, `Document/Systems/Vehicles/VehicleRuntime.md v1.3.0`, `Document/Systems/Vehicles/VehicleInventory.md v1.0.0`, `Document/Systems/Vehicles/VehicleData.md`, `Document/Systems/Combat/WeaponData.md`
- Reuse Dependencies: `CF-FQ-034 차량 피팅·질량 런타임`, `CF-FQ-035 인벤토리 Foundation`, 기존 VehicleDebug Panel
- Production Dependency: `CF-FQ-040 Guided Vehicle Builder`가 만든 VehicleData도 동일 입력으로 사용할 수 있으며, `CF-FQ-044` 완료 기준 Builder USER PASS 뒤 Default Runtime Catalog에 persisted promotion된 VehicleData를 Packaged Demo candidate로 사용할 수 있음

---

## 1. 목적

CarFight 실행 중 이미 제작되어 있는 차량과 장비를 선택해 현재 플레이어에게 적용하고 즉시 시험할 수 있는 개발·시연용 Runtime 기능을 만든다.

이 기능은 차량이나 무기를 제작하는 Builder가 아니다.

```text
이미 제작된 VehicleData / EquipmentPresetData
→ Runtime 선택
→ 명시적 Apply
→ 현재 플레이어 차량·장비에 적용
→ 즉시 운전·발사 시험
```

P0의 직접 목적은 다음 두 가지다.

1. 개발 중 여러 차량·무장 조합을 빠르게 바꾸어 시험한다.
2. PIE뿐 아니라 시연용 Packaged Build에서도 같은 흐름을 사용할 수 있게 한다.

향후 정식 Fitting/Inventory UI가 생기면 이번 기능의 Runtime 적용 경로를 재사용할 수 있어야 한다. 다만 P0에서 정식 Fitting/Inventory 기능을 다시 만들지는 않는다.

---

## 2. 확정 설계 원칙

### 2.1 Existing Asset Apply 전용

P0는 이미 존재하는 자산을 선택·적용하는 기능만 제공한다.

```text
Vehicle 생성/편집/저장 = Scope Out
Weapon 생성/편집/저장 = Scope Out
Runtime 수치 튜닝 = Scope Out
새 Loadout 영구 저장 = Scope Out
```

향후 Runtime Parameter Override를 붙일 가능성은 열어두되 현재 구조나 일정의 선행 조건으로 만들지 않는다.

### 2.2 PIE 전용이 아닌 실제 Runtime 기능

지원 목표:

```text
PIE
Standalone
Packaged Development 또는 시연 대상 Package
```

Editor Utility, Content Browser, `WITH_EDITOR` 전용 API와 Editor Asset 검색에 의존하지 않는다.

정확한 시연 Package Configuration과 노출 Gate는 RTA-P0-00/01에서 current packaging 설정을 읽고 확정한다.

### 2.3 기존 VehicleDebug UI를 임시 Frontend로 재사용

P0에서는 별도의 정식 Fitting UI나 Demo 전용 아트 UI를 만들지 않는다.

현재 `UCFVehicleDebugPanelWidget` / VehicleDebug Panel의 기존 상호작용 모드와 Navigation 구조를 우선 재사용한다.

예상 최소 표시:

```text
Runtime Apply
- Current Vehicle
- Selected Vehicle
- Apply Vehicle
- Target Mount/Profile
- Current Equipment
- Selected Equipment
- Apply Equipment
- Last Result
```

현재 Dynamic Section/Field 경로는 주로 read-only 표시용이므로 ComboBox/Button 같은 입력 Control을 어느 계층에 최소 추가할지는 RTA-P0-00에서 실제 Widget 구조를 감사한 뒤 정한다.

UI가 임시 Frontend라는 이유로 Runtime 적용 로직을 Widget 안에 구현하지 않는다.

### 2.4 새로운 병렬 Runtime Authority를 만들지 않는다

이번 기능은 기존 도메인 적용 경로를 호출하는 얇은 orchestration이어야 한다.

```text
VehicleDebug UI
→ Runtime Apply 요청
→ 기존 Vehicle Runtime / Fitting Runtime Authority
→ 현재 Pawn
```

다음은 금지한다.

```text
Debug Widget이 Engine/Transmission 값을 직접 변경
Debug Widget이 WeaponComp 내부 필드를 직접 재작성
Debug 전용 Vehicle/Weapon 적용 규칙을 별도로 복제
Inventory/Fitting 규칙을 Debug 코드에 재구현
```

공용 Runtime seam이 실제로 부족한 경우에만 최소 Adapter/Coordinator를 추가한다.

### 2.5 Vehicle과 Equipment의 현재 Authority를 존중

Vehicle 입력은 `UCFVehicleData`를 사용한다.

Vehicle Builder Step 8에서 이미 실제 USER test-drive용 차량 전환 경로를 구현·사용했다. 현재 saved Target VehicleData를 transient duplicate한 뒤 현재 조종 중인 `ACFVehiclePawn`에 넣고 `InitializeVehicleRuntime()`을 재호출하는 방식이다.

```text
Persistent VehicleData
→ transient runtime duplicate
→ Current VehiclePawn.VehicleFittingData = nullptr
→ Current VehiclePawn.VehicleData = transient VehicleData
→ Current VehiclePawn.InitializeVehicleRuntime()
```

`InitializeVehicleRuntime()`은 `CarFight_Re` Runtime 모듈의 `WITH_EDITOR` 바깥 경로이며 VehicleVisual / VehicleDataConfig / Layout / TurretVisual / Drive / WheelSync / Aim / Health / Fitting / Weapon / Ammo / Launcher / Defense / Sensor를 재초기화한다. Guided Builder Step 8 runtime 교체 뒤 ChassisMesh 잔류 문제도 별도 regression으로 교정·보호되고 있다.

따라서 CF-FQ-041 P0의 canonical Vehicle Apply 전략은 **현재 Pawn 유지 + transient VehicleData + `InitializeVehicleRuntime()` 재초기화**로 고정한다. 새 Pawn Spawn/Possess/Replacement는 현재 요구에 필요하지 않으며 P0 Scope Out이다.

RTA-P0-00에서는 이 전략을 다시 선택하지 않는다. Builder의 Editor 전용 PIE lookup / benchmark guard / Recipe binding을 제거하고 Packaged Runtime에서 current player Pawn과 Runtime Catalog VehicleData를 입력으로 같은 재초기화 seam을 안전하게 호출하는 최소 Runtime wrapper 위치만 확정한다.

무장/장비 입력은 raw `UCFWeaponData`를 직접 적용하지 않는다.

현재 정식 장착 계약을 따른다.

```text
Target MountProfileId
+
UCFEquipmentPresetData
→ Fitting Snapshot
→ UCFVehicleFittingComp Prepare
→ Runtime Commit / Rollback
```

`EquipmentPresetData`가 TurretMountData + WeaponData를 소유하는 현재 계약을 유지한다.

### 2.6 Inventory와 Fitting 재사용 경계

Debug/Demo와 정식 Fitting의 차이는 후보 Source와 권한 검증이다.

```text
Debug/Demo
Catalog에서 허용된 Asset 선택
→ 소유권 검사 없이 시험
→ 기존 Runtime Fitting Apply 사용

정식 Fitting/Inventory
보유 Item Instance 선택
→ 소유권/예약/슬롯/호환성 검증
→ 기존 Runtime Fitting Apply 사용
```

Debug 기능은 Inventory Item/Container/Reservation을 생성·이동·Commit하지 않는다.

피팅 런타임의 Snapshot Validation, Prepare, Commit, Rollback과 현재 Applied 상태는 가능한 한 그대로 재사용한다.

### 2.7 Explicit Apply와 Current/Selected 분리

선택만으로 Runtime을 변경하지 않는다.

```text
Current Vehicle = 실제 적용 상태
Selected Vehicle = UI에서 고른 후보

Current Equipment = 실제 적용 상태
Selected Equipment = UI에서 고른 후보
```

Apply 성공 후에만 Current 표시를 새 상태로 바꾼다.

### 2.8 실패는 기존 정상 상태를 보존

새 Vehicle/Equipment 적용 실패가 기존 정상 Runtime을 파괴해서는 안 된다.

```text
Candidate Validation 실패
→ Current 유지

Prepare 실패
→ Current 유지

Commit 실패
→ 가능한 기존 Runtime Rollback 계약 사용
→ Current 유지
```

부분 적용 성공 상태를 P0 정상 결과로 허용하지 않는다.

### 2.9 P0 Vehicle / Equipment Apply는 독립 Transaction

P0에서는 한 번의 버튼으로 전체 차량+장비 Loadout을 원자 교체하는 기능을 만들지 않는다.

```text
Apply Vehicle
= 차량 전환 단위

Apply Equipment
= 선택 Mount/Profile의 장비 전환 단위
```

Vehicle 전환 성공 뒤 현재 Mount/Fitting 상태를 Runtime truth에서 다시 읽어 UI를 갱신한다.

향후 정식 Loadout Apply가 필요하면 별도 요구사항으로 확장한다.

### 2.10 Runtime 상태는 일시적

P0 적용 결과는 현재 실행 수명에만 존재한다.

```text
SaveGame 저장 없음
VehicleData 저장 없음
EquipmentPreset 저장 없음
Builder Recipe 저장 없음
Inventory 영구 상태 변경 없음
```

재실행하면 정상 게임 초기 구성으로 돌아가는 것을 기본값으로 한다.

---

## 3. Packaged Asset / Catalog 방향

Packaged Build에서는 Editor Asset 검색에 의존하지 않는다.

시연과 개발에서 노출할 자산을 명시적으로 제한할 수 있는 **등록형 Runtime Catalog/Registry** 경로를 사용한다.

P0의 기본 운영 원칙은 **등록된 Asset만 목록에 노출**하는 것이다. 프로젝트 전체 Content 폴더를 Runtime에서 자동 검색해 모든 VehicleData/EquipmentPresetData를 수집하는 기능은 만들지 않는다.

개념:

```text
Runtime Test Catalog
├─ Allowed VehicleData
└─ Allowed EquipmentPresetData
```

목적:

- Packaged Cook/Load 보장
- 미완성/Fixture/Deprecated Asset의 무분별한 노출 방지
- Demo 후보를 명시적으로 통제
- UI가 전체 Content를 Runtime scan하지 않게 함
- 아직 Garage/Inventory/File Management가 없는 현재 단계의 최소 Selection Source 제공

P0에서는 새 차량/장비가 완성되면 Catalog에 수동 등록하는 것으로 충분하다. Vehicle Builder가 자동 등록하거나 별도 파일관리 시스템을 만드는 것은 Scope Out이다.

Catalog는 Runtime Apply의 영구 Authority가 아니라 현재 개발·시연용 **Selection Source**다. 향후 Inventory/Garage가 생기면 후보 Source를 교체할 수 있으며 Runtime Apply 경로는 그대로 재사용한다.

```text
현재: Runtime Test Catalog → Runtime Apply
미래: Inventory / Garage    → Runtime Apply
```

UI 표시명은 기존 VehicleData/EquipmentPresetData의 현재 Identity/DisplayName 계약을 우선 사용하고, 적절한 표시 필드가 없으면 P0에서는 Asset 이름을 fallback으로 사용한다. 이 메뉴 때문에 VehicleData Schema에 새 DisplayName 필드를 선행 추가하지 않는다.

단, 현재 Asset Manager / PrimaryAsset / 기존 Registry로 같은 요구를 이미 충족할 수 있다면 새 Catalog DataAsset을 중복 생성하지 않는다.

Hard Reference / Soft Reference / PrimaryAssetId 중 정확한 저장 방식은 RTA-P0-00/01 Source·Packaging 감사 뒤 확정한다.

---

## 4. 현재 구현에서 확인된 재사용 기반

### 4.1 Vehicle

Current Runtime 경로:

```text
UCFVehicleData
→ transient runtime duplicate
→ ACFVehiclePawn.VehicleData
→ ACFVehiclePawn.InitializeVehicleRuntime()
→ VehicleVisual / VehicleDataConfig / Layout / TurretVisual
→ Drive / WheelSync / Aim / Health
→ Fitting / Weapon / Ammo / Launcher / Defense / Sensor
```

Vehicle Builder Step 8이 이미 이 same-Pawn reinitialize 경로를 USER test-drive에 사용한다. Wheel Visual과 ChassisMesh의 runtime 교체 잔류 문제는 기존 regression으로 보호되고 있다.

RTA-P0-00은 차량 교체 전략 자체를 다시 설계하지 않고, Editor 전용 Builder wrapper와 Runtime 공용 apply seam의 경계를 확정한다.

### 4.2 Fitting / Equipment

현재 `UCFVehicleFittingComp`에는 다음 기반이 존재한다.

```text
PrepareSortieFittingSnapshot()
CommitPreparedSortieFitting()
CommitPreparedSortieFittingToVehicle()
CaptureAppliedRuntimeCheckpoint()
RestoreAppliedRuntimeCheckpoint()
RollbackPreparedSortieFitting()
GetAppliedFittingSnapshot()
```

현재 Fitting Snapshot은 MountProfile별 최종 `EquipmentPresetData`, WeaponData, TurretMountData, SensorData와 질량을 해석한다.

따라서 P0 무장 교체는 이 계약을 우회하지 않는다.

### 4.3 Inventory

Inventory Foundation은 Item Instance, Container, Reservation, Atomic Transfer를 소유한다.

이번 Debug/Demo 기능은 Asset 시험을 위해 Inventory ownership을 요구하지 않지만 Inventory state를 조작하지도 않는다.

### 4.4 VehicleDebug Panel

현재 `UCFVehicleDebugPanelWidget`은:

- Runtime에서 동작하는 UUserWidget
- Dynamic Navigation / Selected Section
- Panel Interaction Mode
- Pawn Snapshot 기반 갱신

을 이미 제공한다.

기존 표시용 Field 모델은 read-only 중심이므로 선택 Control을 가장 적은 변경으로 추가하는 지점을 RTA-P0-00에서 확정한다.

---

## 5. P0 범위

### 공통

- 기존 VehicleDebug UI 재사용
- 메뉴의 Runtime 접근
- Packaged Demo 환경 동작
- Current / Selected 분리
- Explicit Apply
- Apply 결과 표시
- Invalid Candidate fail-closed
- Runtime 상태 readback
- 반복 Apply 후 stale 상태 없음

### Vehicle

- 허용된 VehicleData 목록
- Current VehicleData 표시
- VehicleData 선택
- Apply
- 현재 Pawn/Controller 관계를 유지한 채 성공 후 정상 조종
- 차량별 Visual/Movement/Wheel/Fitting 관련 잔류 상태 검사

### Equipment

- 현재 차량의 적용 가능한 Mount/Profile 선택
- 허용된 EquipmentPresetData 목록
- Current Equipment 표시
- Equipment 선택
- Fitting Validation
- Apply
- 성공 후 정상 조준/발사
- 실패 시 기존 장비 유지

---

## 6. P0 제외 범위

```text
Vehicle Builder
Weapon Builder
VehicleData/WeaponData/EquipmentPresetData 생성
Asset 저장
Runtime Torque/RPM/Suspension/Damage/FireRate 직접 편집
Loadout Preset 영구 저장
정식 Inventory UI
정식 Fitting UI
Garage/File Management 시스템
프로젝트 전체 Vehicle/Equipment Asset 자동 검색
Builder의 Runtime Test Catalog 자동 등록
Inventory Item ownership 생성·이동
상점/경제/보상
완성형 시연 UI Art
차량/장비 Preview Scene
검색·Favorite·정렬 완성형 UX
전체 Vehicle+Equipment 원자 Loadout Apply
멀티플레이 Replication/Server Authority
Shipping 공개 정책 완성
```

---

## 7. 작업 패키지

| ID | 작업 | 결과 | 상태 |
| --- | --- | --- | --- |
| `RTA-P0-00` | Current Runtime Apply Contract Audit | Builder Step 8 Vehicle reinitialize 재사용, 등록형 Catalog 필요성, Equipment/Fitting 적용 gap, Debug UI 삽입점, Package/Cook 현재 계약 확정 | **PASS** |
| `RTA-P0-01` | Runtime Catalog / Packaged Load Contract | 명시 Catalog DataAsset + bounded Cook 보장 + 초기 허용 Vehicle/Equipment 등록 | **Technical PASS** |
| `RTA-P0-02` | Vehicle Runtime Apply | Validation + Explicit Apply + 성공/실패 readback + 기존 상태 보존 | **Technical PASS** |
| `RTA-P0-03` | Equipment Slot Runtime Apply | Mount/Profile + EquipmentPreset 기반 Fitting Prepare/Commit/Rollback + Mass/Ammo/Turret/Launcher refresh + 실패 복구 | **Technical PASS** |
| `RTA-P0-04` | Existing VehicleDebug UI Integration | 기존 Debug Panel에 최소 선택/Apply/상태 UI 연결 | **Technical PASS** |
| `RTA-P0-05` | PIE E2E | 차량·장비 반복 교체와 실패 복구 Technical/User 검증 | **USER PASS / Closed** |
| `RTA-P0-06` | Packaged Demo E2E | Editor 비의존 Package에서 Catalog load + Vehicle/Equipment Apply 검증 | **Ready / Multi-Mount remediation fresh RuntimeApply 16/16 PASS / Builder-produced Wagon candidate** |
| `RTA-P0-07` | Closure / Reuse Contract Promotion | 검증된 Runtime seam을 Systems에 승격하고 Fitting/Inventory 후속 재사용 경계 기록 | Pending |

---

## 8. RTA-P0-00 감사 질문

구현 전 아래를 실제 Source/Asset/Packaging 기준으로 답한다.

### Vehicle

1. Builder Step 8의 `VehicleData transient duplicate → current Pawn → InitializeVehicleRuntime()` 경로에서 Editor 전용인 부분과 Runtime 공용인 부분을 정확히 분리한다.
2. Packaged Runtime에서는 PIE World lookup, Technical Benchmark, Recipe binding 없이 current local Player VehiclePawn + Runtime Catalog VehicleData를 입력으로 같은 재초기화 seam을 호출할 최소 Runtime wrapper 위치를 확정한다.
3. 기존 Step 8/VD-P0-04 regression이 Chassis/Movement/Wheel/Fitting 초기화 범위를 어디까지 보호하는지 확인하고 CF-FQ-041에 필요한 추가 affected regression만 식별한다.
4. Vehicle Apply 시 `VehicleFittingData=nullptr`로 Base Vehicle 상태를 만드는 현재 Step 8 의미를 P0 기본 계약으로 유지할지, 이후 Equipment Apply가 새 Snapshot을 적용하는 순서를 명시한다.
5. 반복 Vehicle A→B→A에서 transient object lifetime, 기존 Applied Fitting/Ammo/Launcher 상태와 Runtime readback이 stale 없이 재초기화되는지만 추가 검증한다.

### Equipment/Fitting

1. 기존 `VehicleFittingData::BuildFittingSnapshot` 또는 다른 현재 Helper로 한 Mount/Profile만 바꾼 transient Candidate Snapshot을 만들 수 있는가?
2. 새 DataAsset 생성 없이 Runtime Draft → Snapshot 생성이 가능한가?
3. 기존 Applied Snapshot에서 한 Mount만 교체한 Candidate를 만들 때 Defense/Sensor/Ammo/Mass를 손실하지 않는가?
4. `PrepareSortieFittingSnapshot → CommitPreparedSortieFittingToVehicle`이 현재 실행 중 차량의 field-style reapply에 충분한가?
5. 실패 시 기존 Applied Runtime 복원이 실제 Vehicle runtime까지 보장되는가?

### UI

1. 현재 VehicleDebug Panel dynamic Section에 interactive control row를 추가하는 최소 구조는 무엇인가?
2. 기존 WBP tree를 크게 수정하지 않고 C++ child widget 또는 단일 Runtime Apply Section으로 확장 가능한가?
3. 기존 Panel open/close와 interaction mode를 그대로 사용할 수 있는가?

### Package/Cook

1. 시연 대상 Package Configuration은 무엇인가?
2. 현재 VehicleData/EquipmentPresetData의 PrimaryAsset/Cook 경로는 무엇인가?
3. 등록된 Asset만 노출하는 P0 Selection Source를 기존 Registry가 제공할 수 있는가, 아니면 최소 Runtime Test Catalog가 필요한가?
4. Hard Reference / Soft Reference / PrimaryAssetId 중 어떤 방식이 Cook 보장과 단순성을 가장 잘 만족하는가?
5. Debug Panel이 target Package에서 strip되지 않고 노출 Gate로 제어 가능한가?

---

## 9. 검증 기준

### 자동/기술 검증

- Source diff review
- 공식 UE 5.8 Editor Build
- Vehicle Apply focused Automation
- Fitting/Weapon 기존 affected regression
- Debug Panel affected UI regression
- Catalog/Cook reference validation
- 패키징 성공

### PIE E2E

최소:

```text
Vehicle A → B → A
Equipment A → B → Empty/Default → A
Invalid Vehicle candidate
Incompatible Equipment candidate
Apply failure 후 Current state 유지
Vehicle 변경 뒤 Current Equipment/Fitting UI refresh
```

### Packaged Demo E2E

CF-FQ-044 closure handoff 기준 대표 Vehicle candidate:

```text
/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon
→ Builder exact USER Driving PASS 보유
→ Default Runtime Catalog AllowedVehicleData에 USER explicit Save 완료
→ fresh persisted AssetDump에서 Catalog Vehicles=4 중 Wagon exact membership 1개 확인
```

이 handoff는 RTA-P0-06의 Packaged E2E PASS를 대신하지 않는다. 실제 packaged executable에서 Catalog load와 Vehicle/Equipment Apply가 성공해야 RTA-P0-06을 닫는다.

최소:

```text
Package 실행
→ 기존 Debug UI 진입
→ Catalog Vehicle 표시
→ Vehicle Apply
→ 정상 운전
→ Mount/Profile + Equipment 선택
→ Equipment Apply
→ 정상 조준/발사
→ 실패 시 기존 상태 유지
→ 재실행 시 영구 변경 없음
```

USER 판정은 메뉴 사용성, 차량 전환 체감, 실제 시연 흐름을 확인한다.
Technical fact는 가능한 자동 테스트와 Runtime readback으로 먼저 닫는다.

---

## 10. 완료 조건

CF-FQ-041 P0 완료는 다음을 모두 만족해야 한다.

- PIE와 시연 대상 Packaged Build에서 동일한 Existing Asset Apply 기능이 동작한다.
- Editor-only API 없이 후보 Asset을 load할 수 있다.
- VehicleData를 바꾸고 정상적으로 운전할 수 있다.
- Mount/Profile별 EquipmentPresetData를 바꾸고 정상적으로 조준/발사할 수 있다.
- Debug UI가 도메인 값을 직접 수정하지 않는다.
- Equipment Apply가 기존 Fitting Validation/Runtime Commit/Rollback 경로를 재사용한다.
- Debug/Demo 적용이 Inventory state를 변경하지 않는다.
- 실패한 Apply는 기존 정상 상태를 보존한다.
- Current와 Selected가 성공 전까지 혼동되지 않는다.
- Asset/SaveGame/Persistent Loadout을 저장하지 않는다.
- P0 구현을 위해 정식 Fitting/Inventory UI나 Builder를 추가하지 않는다.

---

## 11. 변경 보호와 착수 조건

현재 main_game worktree에는 CF-FQ-039/040 등 다른 작업의 미커밋 Source/Asset 변경이 존재한다.

특히 `CFVehiclePawn.h/.cpp`는 현재 dirty 공용 통합 파일이다.

따라서 RTA-P0-00은 read-only 감사로 시작한다.

```text
- 기존 dirty 변경을 정리/되돌리지 않음
- Pawn 수정 필요성을 먼저 최소화
- Fitting/Debug UI의 clean seam 우선 재사용
- 공용 Pawn 변경이 필요하면 현재 dirty 내용과 exact overlap을 먼저 분리
- 구현 전 affected regression 목록 확정
```

현재 Editor 실행이나 UE Asset mutation은 RTA-P0-00의 필수 조건이 아니다.

---

## 12. RTA-P0-00 Current Runtime Apply Contract Audit — PASS

### 12.1 Vehicle Apply Authority

Vehicle Builder Step 8의 다음 경로를 CF-FQ-041 Vehicle Apply의 canonical authority로 재사용한다.

```text
Catalog VehicleData
→ transient duplicate
→ current ACFVehiclePawn.VehicleFittingData = nullptr
→ current ACFVehiclePawn.VehicleData = transient VehicleData
→ InitializeVehicleRuntime()
```

새 Pawn Spawn/Possess는 필요하지 않다. Runtime wrapper는 Builder의 PIE World lookup, Recipe/Benchmark guard와 Editor dependency를 포함하지 않는다.

Vehicle Apply 실패 시에는 이전 VehicleData/VehicleFittingData를 보존해 재초기화 복구를 시도하고, Apply 실패와 Recovery 실패를 구분하는 결과를 반환한다. Debug Widget이 직접 Pawn 필드를 변경하지 않는다.

### 12.2 Runtime Catalog / Cook 판정

Asset evidence에서 현재 `CFVehicleData`는 12개, `CFEquipmentPresetData`는 10개가 발견됐고 이 안에는 Production 후보뿐 아니라 `_Legacy`와 `Tests/*` Fixture가 함께 존재한다. 따라서 전체 Class 자동 검색은 Debug/Demo 노출 정책으로 부적합하다.

현재 `DefaultGame.ini`의 `AssetManagerSettings`는 Map / PrimaryAssetLabel / GameFeatureData만 명시 scan하며 `CFVehicleData`와 `CFEquipmentPresetData`를 Runtime enumeration용 PrimaryAsset type으로 등록하지 않는다.

따라서 P0는 다음을 고정한다.

```text
UCFRuntimeTestCatalogData : UPrimaryDataAsset
- TArray<TObjectPtr<UCFVehicleData>> AllowedVehicleData
- TArray<TObjectPtr<UCFEquipmentPresetData>> AllowedEquipmentPresetData
```

- **Hard Reference**를 P0 기본값으로 사용한다. 현재 대상 수가 작고 동기 선택 UI이며 별도 Asset Manager enumeration/async load를 만드는 이점이 없다.
- Catalog에 등록된 Asset만 UI에 표시한다.
- Runtime 전체 Asset Registry scan은 하지 않는다.
- Catalog 자체와 신규 Debug child UI는 `/Game/CarFight/Debug/` 아래에 두고 RTA-P0-01에서 이 bounded directory의 Cook 보장 방식을 설정한다. Catalog hard reference가 실제 Vehicle/Equipment dependency를 끌고 오게 한다.
- 기존 Vehicle/Equipment가 `UPrimaryDataAsset`이라는 사실은 identity에 활용할 수 있지만 현재 AssetManager scan에 의존해 목록을 만들지 않는다.

초기 Catalog는 Production/시연 대상으로 명시 승인된 Asset만 넣고 `Tests/*`, `_Legacy/*`는 기본 제외한다.

### 12.3 Equipment Candidate Snapshot Authority

`FCFInventoryFitAdapter`는 이미 transient `UCFVehicleFittingData` → `BuildFittingSnapshot()` 패턴을 사용하지만 ItemInstance 소유권/접근/예약에 결합되어 있으므로 Debug Catalog가 직접 호출하지 않는다.

Debug/Demo는 Inventory를 거치지 않고 다음 Fitting domain contract만 재사용한다.

```text
current VehicleData
+ current transient/persistent VehicleFittingData가 있으면 그 선택을 복사
+ Target MountProfileId의 EquipmentPresetData만 교체
→ transient UCFVehicleFittingData
→ BuildFittingSnapshot()
→ Snapshot Validation
```

현재 FittingData가 같은 VehicleData에 연결되어 있으면 MountSelections / InitialSortieAmmoLoads / DefenseSelection을 보존해 다른 슬롯과 탄약·방어 선택을 잃지 않는다. 현재 FittingData가 없으면 VehicleData Default + MissingMountSelectionPolicy=UseVehicleDefault를 기준으로 시작한다.

finite ammo 장비인데 보존 가능한 유효 `InitialSortieAmmoLoads`가 없으면 P0가 탄약 수를 임의 생성하지 않고 **Validation 실패로 차단**한다. Ammo 편집 UI는 P0 Scope Out이다.

### 12.4 Equipment Runtime Apply에서 확인된 Integration Gap

현재 `UCFVehicleFittingComp`는 다음을 이미 제공한다.

```text
PrepareSortieFittingSnapshot()
CommitPreparedSortieFittingToVehicle()
CaptureAppliedRuntimeCheckpoint()
Rollback / Restore 기반
```

`FCFChaosVehicleMassRuntime`은 Field Fitting용 실제 Chaos Mass hot reapply/restore를 제공한다.

그러나 현재 `CommitPreparedSortieFittingToVehicle()`의 실제 participant는 Weapon / Defense / Sensor이며, `InitializeVehicleRuntime()`에서 Fitting Commit 뒤 별도로 수행되는 **Ammo Runtime 초기화, Turret Visual 재적용, Launcher Runtime 초기화와 최종 CombatReady 갱신**까지 하나의 packaged-runtime public operation으로 묶은 진입점은 없다.

또한 `FCFFieldFitRuntimeAdapter`는 이전 `AppliedFittingSnapshot`을 요구하므로 Builder 방식 Vehicle Apply 직후의 Legacy Fitting 상태에서 첫 Debug Equipment Apply를 그대로 맡길 수 없다.

따라서 RTA-P0-03은 새 Fitting 규칙을 만들지 않고, 기존 participant를 묶는 **최소 Runtime Content Apply Coordinator/Service**를 추가한다.

책임:

```text
Candidate Fitting Snapshot validation
→ existing Fitting Prepare/Commit/Rollback
→ existing Chaos Mass hot reapply/restore
→ Ammo / TurretVisual / Launcher post-Fitting refresh
→ final Runtime readback
```

이 Service는 Inventory 이동/예약을 소유하지 않으며 향후 정식 Fitting/Inventory frontend도 재사용할 수 있는 Runtime seam으로 둔다. 기존 `CFFieldFitCoordinator`의 Inventory transaction을 Debug에서 가짜로 생성해 우회하지 않는다.

### 12.5 VehicleDebug UI 삽입점

현재 `WBP_VehicleDebugPanel`은 `VerticalBox_NavHost` + `VerticalBox_SelectedSectionHost` 구조이고 Navigation은 `TopLevelSectionArray`에서 자동 생성된다. 기존 `UCFVehicleDebugSectionWidget` / `UCFVehicleDebugFieldRowWidget`은 read-only Text 표시 모델이다.

따라서 generic FieldRow에 ComboBox/Button 의미를 억지로 추가하지 않는다.

P0 UI 구조:

```text
VehicleDebug Navigation
└─ Runtime Apply
   └─ dedicated UCFVehicleDebugRuntimeApplyWidget / WBP_VehicleDebugRuntimeApply
      ├─ Vehicle ComboBox
      ├─ Apply Vehicle Button
      ├─ Mount/Profile ComboBox
      ├─ Equipment ComboBox
      ├─ Apply Equipment Button
      └─ Current / Selected / Last Result Text
```

Panel은 `RuntimeApply` TopLevel Section을 Navigation에 추가하고 해당 Section 선택 시 기존 `VerticalBox_SelectedSectionHost`에 전용 child widget을 표시한다. 루트 Panel 전체를 새 UI로 교체하지 않는다.

역할 분배:

```text
C++ = Catalog option mapping, validation, apply request/result, current runtime readback
BP/WBP = ComboBox/Button/Text의 최소 배치와 시각 조정
```

기존 Panel Interaction Mode를 그대로 사용하며 P0에서 새 전역 Debug hotkey를 만들지 않는다.

### 12.6 예상 Source/Asset 변경 경계

RTA-P0-01~04에서 우선 예상하는 변경은 다음과 같다.

```text
신규 C++
- Runtime Test Catalog DataAsset class
- Runtime Content Apply Service/Result contract
- VehicleDebug RuntimeApply child widget C++ class

최소 기존 C++
- CFVehicleDebugPanelWidget: RuntimeApply Navigation/child host 연결
- 필요한 경우 Fitting/Combat post-apply 공용 seam만 최소 추출

신규 UE Asset
- /Game/CarFight/Debug/Data/DA_CFRuntimeTestCatalog_Default
- /Game/CarFight/Debug/UI/WBP_VehicleDebugRuntimeApply

Config
- bounded Debug directory Cook guarantee
```

현재 dirty `CFVehiclePawn.h/.cpp`는 **기본 수정 대상으로 잡지 않는다**. 새 Service가 현재 공개 Runtime API로 닫히지 않는 정확한 post-Fitting refresh seam이 있을 때만 최소 변경 여부를 별도 판단한다.

### 12.7 Affected Regression

구현 전 affected 회귀 묶음은 다음으로 고정한다.

```text
VehicleData runtime reinitialize / Chassis visual regression
Vehicle Fitting Snapshot + Runtime rollback regression
Field Fit mass transaction regression
Weapon selection/runtime regression
Ammo fitting/runtime regression
Launcher runtime regression
Scanner fitting regression
VehicleDebug Panel navigation/interaction regression
```

기존 관련 PASS를 무조건 broad replay하지 않고 실제 변경 파일 기준 affected set만 실행한다.

---

## 13. RTA-P0-01 Runtime Catalog / Packaged Load Contract — Technical PASS

### 13.1 구현

신규 Runtime 계약:

```text
UCFRuntimeTestCatalogData : UPrimaryDataAsset
- AllowedVehicleData : hard reference array
- AllowedEquipmentPresetData : hard reference array
- empty / null / duplicate fail-closed validation
- bounded summary

UCFRuntimeTestSettings : Config=Game
- DefaultCatalog : soft config locator
- LoadDefaultCatalog()
```

Runtime UI나 Apply Service는 Asset Registry 전체 검색이나 Content Browser API를 사용할 필요가 없다.

```text
DefaultGame.ini
→ DefaultCatalog soft path
→ persisted Runtime Test Catalog
→ hard referenced Vehicle / Equipment 목록
```

### 13.2 Cook 계약

`DefaultGame.ini`에 다음 bounded directory를 Always Cook 대상으로 추가했다.

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
+DirectoriesToAlwaysCook=(Path="/Game/CarFight/Debug")
```

기본 Catalog는 `/Game/CarFight/Debug/Data/DA_CFRuntimeTestCatalog_Default`이며 AssetDump dependency evidence에서 정확히 hard dependency 5개를 가진다.

차량:

```text
DA_TestSedan
DA_TestSUV
DA_Vehicle_Wagon
```

장비:

```text
HeavyCannon
RocketLauncher
```

`/Game/CarFight/Tests/*`와 `/Game/CarFight/_Legacy/*` 참조는 기본 Catalog에 없다.

이 단계에서 실제 전체 Packaged Build 실행까지 수행하지 않는다. P0-01은 **bounded directory cook inclusion + persisted hard dependency + runtime locator/load 계약**을 소유하며, 실제 packaged executable에서의 E2E load/apply는 `RTA-P0-06`이 최종 소유한다.

### 13.3 검증 Evidence

- Official UE 5.8 Editor Build `3af0a8e58f9248d9a934a6b77df91201` — PASS.
- persisted Asset 생성 뒤 actual runtime-load 보강 Build `5cc793d67fc2447facde70adb0d55605` — PASS / Exit 0.
- Catalog 수동 등록 `EditAnywhere` post-review 교정 후 **최종 Official Build `17cf5dca1d8c4d738a3326774f835c34` — PASS / Exit 0**.
- focused Automation `CarFight.RuntimeApply.RTA_P0_01.CatalogContract` — final run `2d4d2ea632054e6bb470aeeaa35ee8ff`, **1/1 PASS**.
- final Automation은 `UCFRuntimeTestSettings::LoadDefaultCatalog()`로 persisted Catalog를 실제 로드하고 Vehicle 3 / Equipment 2 / runtime validation을 확인한다.
- fresh AssetDump persisted evidence: Catalog values Vehicle 3 / Equipment 2, hard dependency 5, soft dependency 0이며 `AllowedVehicleData` / `AllowedEquipmentPresetData`가 asset instance에서 모두 `editable=true`임을 확인했다.
- 일회성 Catalog 생성 helper는 실행 후 repository에서 Trash 격리했다. Product tool로 승격하지 않는다.

Live Editor lifecycle은 Catalog 생성 시 MCP Ready postcondition timeout이 2회 있었으나, 이 기능의 Source/Build/Runtime Catalog 계약 실패가 아니며 infrastructure 수리로 범위를 확장하지 않았다. 저장 Asset은 bounded UE headless commandlet로 생성한 뒤 AssetDump persisted evidence와 runtime load Automation으로 별도 검증했다.

### 13.4 변경 보호 결과

RTA-P0-01은 다음 기존 Builder/Runtime 공용 파일을 수정하지 않았다.

```text
CFVehiclePawn.h/.cpp
Vehicle Builder Source
VehicleData schema
기존 VehicleData Asset
기존 EquipmentPresetData Asset
VehicleDebug Widget
```

따라서 현재 CF-FQ-040 Builder 작업과의 direct source overlap은 없다.

---

## 14. RTA-P0-02 Vehicle Runtime Apply — Technical PASS

### 14.1 충돌 없는 Runtime Operation 경계

RTA-P0-02 착수 직전 최신 dirty diff를 다시 확인했다.

- `CFVehiclePawn.h/.cpp`는 clean이었고 수정하지 않았다.
- Vehicle Builder는 CF-FQ-042 신규 차량 진입 UX 작업으로 `CFVehicleBuilderVM/Tab`과 Builder test가 dirty였으므로 모두 보호했다.
- RTA-P0-02는 Builder Source와 Pawn Source를 수정하지 않고 `CarFight_Re` Runtime 모듈의 신규 독립 Service로 구현했다.

신규 파일:

```text
UE/Source/CarFight_Re/Public/CFRuntimeVehicleApply.h
UE/Source/CarFight_Re/Private/CFRuntimeVehicleApply.cpp
UE/Source/CarFight_Re/Private/CFRuntimeVehicleApplyTests.cpp
Tools/RunRuntimeApplyTests.ps1
```

### 14.2 Vehicle Runtime Apply 계약

`FCFRuntimeVehicleApplyService`는 Builder Step 8의 canonical same-Pawn 경로를 그대로 재사용한다.

```text
Catalog VehicleData validation
→ persistent VehicleData를 transient duplicate
→ previous VehicleData / VehicleFittingData checkpoint
→ current Pawn.VehicleFittingData = nullptr
→ current Pawn.VehicleData = transient VehicleData
→ InitializeVehicleRuntime()
→ final Pawn runtime readback
```

Catalog는 candidate authorization만 담당하고 실제 Runtime operation은 `ApplyVehicleRuntime()`으로 분리했다. 향후 Inventory/Garage가 selection source가 되어도 같은 Runtime operation을 재사용할 수 있다.

checkpoint는 동기 operation 동안 이전 `VehicleData`와 `VehicleFittingData`를 strong reference로 보존한다. 후보 적용이 실패하면 두 pointer를 원래 값으로 되돌린 뒤 동일 `InitializeVehicleRuntime()`을 다시 호출해 부분 적용된 Runtime component 상태까지 이전 source 기준으로 재초기화한다.

최종 결과는 다음을 구분한다.

```text
Succeeded
ValidationFailed
ApplyFailed
RecoveryFailed
```

`ApplyFailed`는 candidate apply 실패를 의미하며 `bRecoveryAttempted` / `bRecoverySucceeded`로 복구 여부를 별도 표현한다. 복구 재초기화까지 실패한 경우에만 `RecoveryFailed`다.

runtime readback에는 요청 Asset 경로, 이전/현재 VehicleData/Fitting object 경로, `bRuntimeReady`, transient copy 활성 여부와 `VehicleDebug.RuntimeSummary`를 포함한다. 성공 판정은 `InitializeVehicleRuntime()` 반환값만 신뢰하지 않고 실제 current pointer identity + transient flag + RuntimeReady를 함께 확인한다.

### 14.3 실제 실패/복구 검증

기존 `UCFVehicleFittingComp` Initial Mass 안전계약을 우회하지 않았다. 이미 Snapshot Mass가 구성된 Pawn에서 Step 8 의미인 `VehicleFittingData=nullptr` 전환이 mass-change-to-legacy로 거부되는 실제 경로를 이용해 candidate apply 실패를 만들었다.

- candidate apply 실패 → 이전 VehicleData/Fitting 복원 → 재초기화 성공: `ApplyFailed`
- candidate apply 실패 → 이전 pointer 복원은 됐지만 재초기화 실패: `RecoveryFailed`

따라서 RTA가 Fitting 질량 안전장치를 무시하거나 강제로 Reset해서 성공시키는 별도 authority를 만들지 않는다.

### 14.4 Build / Focused / Affected Regression

공식 UE 5.8 Editor Build:

- initial implementation build `dc8be1df22ea450aaa1cec4fa181df1e` PASS / Exit 0
- final metadata correction build `e2cff380b5d945e4bf97ccbacea06bcc` PASS / Exit 0

final binary 기준 focused Automation:

```text
CarFight.RuntimeApply.RTA_P0_02.VehicleValidation           PASS
CarFight.RuntimeApply.RTA_P0_02.VehicleApplySuccess        PASS
CarFight.RuntimeApply.RTA_P0_02.ApplyFailedRecovery        PASS
CarFight.RuntimeApply.RTA_P0_02.RecoveryFailed             PASS
```

결과: **4/4 PASS / 0 Failure**.

직접 affected regression:

```text
CarFight.Fitting.FIT_P0_05.InitialMass                         PASS
CarFight.VehicleData.VD_P0_04.RuntimeReinitializeVisualContract PASS
CarFight.Fitting.FIT_P0_05.DefensePIEPipeline                  PASS
```

기존 공용 `RunDataAuthoringTests.ps1` 첫 호출은 다른 concurrent 작업이 `CFDataAuthoringAutomation.log`를 점유해 실행 전 file-lock으로 실패했다. 이는 RTA test failure가 아니며 다른 작업을 종료하거나 로그를 삭제하지 않고 RuntimeApply 전용 로그/결과를 사용하는 `RunRuntimeApplyTests.ps1`로 격리했다. Product Runtime 계약이나 Builder 작업에는 변경이 없다.

### 14.5 변경 보호 결과와 다음 Gate

최종 상태에서 `CFVehiclePawn.h/.cpp`는 RTA-P0-02 변경 0이며, CF-FQ-042 Builder dirty 변경은 그대로 보존했다. `Document/ActiveWork.md`, `03_FeatureQueue.md`, CF-FQ-042 Plan도 concurrent dirty이므로 RTA-P0-02에서 수정하지 않는다.

다음 Gate:

```text
RTA-P0-03 Equipment Slot Runtime Apply
```

RTA-P0-03은 현재 Vehicle Runtime Apply Service에 차량+장비 원자 transaction을 억지로 합치지 않고, 기존 Mount/Profile + EquipmentPreset + Fitting Prepare/Commit/Rollback authority를 별도 transaction으로 연결한다.

---

## 15. RTA-P0-03 Equipment Slot Runtime Apply — Technical PASS

### 15.1 구현 경계

RTA-P0-03 착수/마감 시 최신 dirty 상태를 다시 확인했다.

- CF-FQ-042 Vehicle Builder의 VM/Tab/test dirty 변경은 그대로 보호했다.
- `Document/ActiveWork.md`, `03_FeatureQueue.md`, VehicleBuilderCreationUX Plan은 concurrent dirty이므로 수정하지 않았다.
- RTA-P0-03은 기존 Fitting/Vehicle Runtime authority를 재사용하며 새 Inventory authority나 병렬 장착 시스템을 만들지 않았다.
- `RunRuntimeApplyTests.ps1`은 broad regression 동안 임시로 `CarFight.Fitting` filter를 사용한 뒤 기본값 `CarFight.RuntimeApply`로 원복했다.

신규 Runtime 파일:

```text
UE/Source/CarFight_Re/Public/CFRuntimeEquipApply.h
UE/Source/CarFight_Re/Private/CFRuntimeEquipApply.cpp
UE/Source/CarFight_Re/Private/CFRuntimeEquipApplyTests.cpp
```

기존 공용 seam 최소 변경:

```text
CFVehicleFittingComp
- RestoreAppliedRuntimeCheckpointToVehicle()

CFVehiclePawn
- RefreshFittingDependentRuntime()
```

### 15.2 Equipment Runtime Apply 계약

`FCFRuntimeEquipApplyService`는 등록된 `EquipmentPresetData`를 단일 Mount/Profile에 적용한다.

```text
Catalog exact Equipment authorization
→ current VehicleData/Fitting 기준 transient VehicleFittingData 생성
→ 대상 MountProfile의 EquipmentPresetData만 교체
→ BuildFittingSnapshot() validation
→ PrepareSortieFittingSnapshot()
→ CommitPreparedSortieFittingToVehicle()
→ 후보 TotalVehicleMassKg가 다르면 Chaos Mass hot reapply
→ Pawn.VehicleFittingData = transient candidate
→ RefreshFittingDependentRuntime()
→ final Fitting / Equipment / Mass / Runtime readback
```

현재 FittingData가 같은 VehicleData를 가리키면 기존 MountSelections, InitialSortieAmmoLoads, DefenseSelection을 복사해 보존한다. 같은 VehicleData의 FittingData가 없으면 VehicleData default mount 해석에서 transient 후보를 만든다.

Fitting Commit 뒤 전체 `InitializeVehicleRuntime()`을 다시 호출하지 않는다. 새 `RefreshFittingDependentRuntime()` seam은 현재 Applied Snapshot 기준으로 다음만 다시 구성한다.

```text
Ammo Runtime
Turret Visual
Launcher Runtime
CombatReady readback
```

Drive, WheelSync, Health, Fitting Initial Mass 자체를 불필요하게 다시 초기화하지 않는다.

### 15.3 Checkpoint / Recovery

mutation 전 checkpoint는 다음을 함께 보존한다.

```text
previous VehicleFittingData UObject identity
previous Fitting Applied Runtime checkpoint
previous Chaos configured mass
```

후보 Fitting Commit 이후 Mass apply, dependent runtime refresh 또는 final readback이 실패하면:

```text
previous Fitting Applied Runtime restore
→ previous VehicleFittingData pointer restore
→ 필요 시 previous Chaos mass restore
→ previous Applied Fitting 기준 dependent runtime refresh
→ final pointer / mass / runtime readback
```

결과 분류는 Vehicle Apply와 동일한 의미를 유지한다.

```text
Succeeded
ValidationFailed
ApplyFailed
RecoveryFailed
```

`ApplyFailed`는 후보 적용이 실패했지만 이전 상태 복구 성공 여부를 `bRecoverySucceeded`로 별도 표시한다. 복구 자체가 실패한 경우만 `RecoveryFailed`다.

P0 Debug/Demo 경계상 순간 전투 상태 전체를 checkpoint하지 않는다. 장비 Apply/Recovery 뒤 Ammo Runtime은 Applied Fitting Snapshot의 `InitialSortieAmmoLoads` 기준으로 다시 구성되므로 현재 탄창/예비탄/Reload 진행 상태를 byte-for-byte 보존하지 않는다.

### 15.4 Build / Focused Automation

초기 공식 Build에서 `CFRuntimeEquipApply.cpp`의 `UCFVehicleDefenseComp` complete type include 누락을 발견해 `CFVehicleDefenseComp.h`를 명시 include로 교정했다.

최종 공식 UE 5.8 Editor Build:

- build job `3b7a3d250f9549a39fb7d6d05a2a5454`
- `CarFight_ReEditor Win64 Development`
- **PASS / Exit 0**

final binary 기준 `CarFight.RuntimeApply` Automation:

```text
RTA-P0-01 CatalogContract                         PASS
RTA-P0-02 VehicleValidation                       PASS
RTA-P0-02 VehicleApplySuccess                     PASS
RTA-P0-02 ApplyFailedRecovery                     PASS
RTA-P0-02 RecoveryFailed                          PASS
RTA-P0-03 EquipmentValidation                     PASS
RTA-P0-03 EquipmentApplySuccess                   PASS
RTA-P0-03 ApplyFailedRecovery                     PASS
```

결과: **8/8 PASS / 0 Failure**.

P0-03 focused 3건은 다음을 각각 검증한다.

- Catalog exact Equipment membership / invalid Catalog fail-closed
- 실제 BP VehiclePawn + persisted SUV Snapshot에서 same-mass transient Equipment hot apply
- 후보 Fitting Commit 후 Chaos mass-change 실패를 이용한 실제 compensation recovery

### 15.5 Affected Fitting Regression

`CarFight.Fitting` broad regression 24건을 실행했다.

직접 영향 핵심 경로:

```text
FIT_P0_04.AtomicBoundary                         PASS
FIT_P0_05.InitialMass                            PASS
FIT_P0_05.DefensePIEPipeline                     PASS
FFIT_P0_04.ChaosMassPIE                         PASS
FFIT_P0_04.FieldRuntimeMassTransaction           PASS
FFIT_P0_04.MassRecoveryClassification            PASS
FFIT_P0_04.RuntimeRecoveryClassification         PASS
```

전체 결과는 **23/24 PASS**였다.

유일한 실패는 `CarFight.Fitting.FIT_P0_07C.QuantitativeMobility`이며 실제 실패 메시지는 다음 lifecycle timeout이다.

```text
FIT-P0-07C: 30초 안에 PIE World Actor 초기화가 완료되지 않았습니다.
```

같은 broad run에서 RTA-P0-03이 직접 변경한 Fitting atomic/mass/recovery/Defense 경로는 모두 PASS했고, QuantitativeMobility failure는 장비 Apply assertion, Fitting rollback assertion 또는 Mass recovery assertion 실패가 아니다. 따라서 P0-03 Product code를 이 계측 timeout에 맞춰 수정하지 않고 별도 mobility-test lifecycle 이슈로 남긴다.

### 15.6 변경 보호 결과와 다음 Gate

RTA-P0-03은 CF-FQ-042 Vehicle Builder Source를 수정하지 않았다. 기존 concurrent dirty `ActiveWork`, FeatureQueue, VehicleBuilderCreationUX Plan과 SourceArt/AssetDump 상태도 보존했다.

다음 Gate:

```text
RTA-P0-04 Existing VehicleDebug UI Integration
```

RTA-P0-04는 이미 검증된 Vehicle/Equipment Runtime service를 Widget 안에 다시 구현하지 않고, 기존 VehicleDebug Panel에 Catalog 선택, Current/Selected 표시, Explicit Apply와 Last Result만 최소 연결한다.

---

## 16. RTA-P0-04 Existing VehicleDebug UI Integration — Implementation Complete / Final Validation Blocked

### 16.1 구현 결과

기존 `UCFVehicleDebugPanelWidget`의 Dynamic Navigation / Selected Section 구조를 유지하면서 `RuntimeApply` 전용 interactive child를 추가했다.

신규 파일:

```text
UE/Source/CarFight_Re/Public/UI/CFRuntimeApplyWidget.h
UE/Source/CarFight_Re/Private/UI/CFRuntimeApplyWidget.cpp
UE/Source/CarFight_Re/Private/UI/CFRuntimeApplyUITests.cpp
```

기존 Panel 최소 변경:

```text
CFVehicleDebugPanelWidget
- RuntimeApply Navigation Section 추가
- RuntimeApply 선택 시 dedicated C++ child 표시
- Overview 등 기존 Generic Section 복귀 시 기존 cached child 재부착
- Panel VehiclePawnRef를 RuntimeApply child와 공유
```

WBP Asset은 생성·수정하지 않았다. `UCFRuntimeApplyWidget`은 C++ `WidgetTree`로 다음 최소 UI만 제공한다.

```text
Catalog 상태
Current Vehicle / Selected Vehicle
Vehicle 선택 + Explicit Apply
Target Mount/Profile
Current Equipment / Selected Equipment
Equipment 선택 + compatibility 표시 + Explicit Apply
Last Result
```

선택 변경은 Runtime mutation을 수행하지 않는다. Vehicle Apply는 `FCFRuntimeVehicleApplyService`, Equipment Apply는 `FCFRuntimeEquipApplyService`에만 위임하며 Widget 안에 별도 Vehicle/Fitting mutation authority를 만들지 않았다.

### 16.2 UI 자동화 계약

`CFRuntimeApplyUITests.cpp v1.0.4`는 다음 3개 focused test를 소유한다.

```text
CarFight.RuntimeApply.RTA_P0_04.PanelNavigation
CarFight.RuntimeApply.RTA_P0_04.ExplicitVehicleApply
CarFight.RuntimeApply.RTA_P0_04.ExplicitEquipmentApply
```

검증 경계:

- `PanelNavigation`: RuntimeApply top-level Navigation 등록, 선택, 기존 Overview 복귀
- `ExplicitVehicleApply`: Selected 변경만으로 Current 불변, Explicit Apply 뒤 transient Vehicle Runtime 전환
- `ExplicitEquipmentApply`: exact Catalog candidate 선택만으로 Current 불변, service 호출 결과를 UI가 그대로 표시하며 성공 시 Current 갱신 / `ApplyFailed` 시 이전 Current와 Fitting source 보존

P0-04는 UI 연결 단계이므로 default Catalog 장비가 현재 Fitting data 계약을 아직 만족하지 못하는 경우를 테스트 편의용 Product 완화로 숨기지 않는다. 실제 반복 Vehicle/Equipment 성공 E2E와 data readiness는 RTA-P0-05가 소유한다.

### 16.3 발견된 P0-05 E2E Data Readiness Gap

Persisted AssetDump에서 현재 기본 Runtime Catalog의 Vehicle/Equipment를 재확인했다.

차량:

```text
DA_TestSedan
- BaseVehicleMassKg = 0
- MaximumGrossMassKg = 0
- MountProfile 있음

DA_TestSUV
- BaseVehicleMassKg = 0
- MaximumGrossMassKg = 0
- MountProfile 있음

DA_Vehicle_Wagon
- BaseVehicleMassKg = 0
- MaximumGrossMassKg = 0
- MountProfiles 없음
```

현재 Fitting 계약은 VehicleData가 명시 Fitting source로 연결될 때 Base/Maximum mass가 0보다 커야 하며, Equipment/Turret/Weapon의 필수 질량도 0kg이면 `MissingMassSource` Error로 처리한다.

따라서 현재 기본 Catalog는 Vehicle 선택/Vehicle Runtime Apply에는 사용할 수 있지만, Catalog Vehicle → Mount/Profile → Catalog Equipment의 성공 E2E를 그대로 보장하는 data set은 아직 아니다.

반면 기존 `DA_VehicleDefense_TestSUV`는 persisted evidence상 Base 1000kg / Maximum 2500kg / `RoofTurret_MediumOrLarge` Mount를 가지며 P0-03 Fitting Runtime service 검증 fixture로 정상 동작한다. 이 차이는 Product service 실패가 아니라 P0-05가 정리해야 할 시연 Catalog data readiness 문제로 분리한다.

### 16.4 Build / Automation 상태

P0-04 Product UI 구현 자체는 다음 full Editor Build에서 이미 compile/link PASS했다.

```text
15aa96326a454c93b34d386b42372e30
CarFight_ReEditor Win64 Development
PASS / Exit 0
```

이후 Equipment UI test fixture를 P0-04 단계 책임에 맞게 v1.0.4로 교정했다. 최종 test source는 build `7133c6d23dc641258b5b0326342c1ced`에서:

```text
Compile [x64] CFRuntimeApplyUITests.cpp PASS
Link UnrealEditor-CarFight_Re.lib PASS
```

까지 완료됐다.

그러나 최종 DLL link는 작업 시작 주체를 증명할 수 없는 기존 `UnrealEditor.exe`가 `UnrealEditor-CarFight_Re.dll`을 점유해 실패했다.

첫 차단:

```text
7133c6d23dc641258b5b0326342c1ced
LNK1104 / UnrealEditor-CarFight_Re.dll in use
```

관리 provenance 확인:

```text
UE MCP status = ERR_UE_MCP_UNAVAILABLE
→ current Editor를 canonical managed Runtime으로 증명하지 못함
→ 자동 force/discard/stop 금지
```

시간을 둔 단일 재시도:

```text
60bd79d620f0404b8337554d1ef537eb
same DLL lock after 20s retry
LNK1104 / Exit 6
```

따라서 더 이상의 blind build retry나 Editor 강제 종료는 수행하지 않는다.

마지막 fully-linked P0-04 binary에서 `PanelNavigation`과 `ExplicitVehicleApply`는 PASS했다. 당시 `ExplicitEquipmentApply` 1건은 Product UI 실패가 아니라 이후 제거한 fixture 가정 때문에 실패했고, 최종 v1.0.4 test는 source compile까지 PASS했으나 새 DLL link가 차단되어 11/11 final Automation은 아직 실행할 수 없다.

### 16.5 현재 Gate

현재 판정:

```text
RTA-P0-04 Product Implementation: Complete
RTA-P0-04 Final Official Build after UITest v1.0.4: Blocked by external/unmanaged Editor DLL lock
RTA-P0-04 Final RuntimeApply 11/11 Automation: Not Run on final binary
RTA-P0-05 PIE E2E: Do Not Start Yet
```

재개 조건은 단순하다.

```text
사용자 또는 해당 Editor owner가 정상적으로 Editor 종료
→ official Build 1회
→ CarFight.RuntimeApply 11/11
→ PASS면 RTA-P0-04 Technical PASS
→ RTA-P0-05로 전진
```

CF-FQ-042 Vehicle Builder Source, concurrent `ActiveWork`, FeatureQueue, VehicleBuilderCreationUX Plan과 SourceArt/AssetDump dirty는 수정·정리·되돌리지 않았다.

### 16.6 Final Validation Closure

사용자가 기존 Editor를 정상 종료한 뒤 최종 검증을 재개했다.

최종 official build:

```text
2a678b29fe324484b07e96c778b04424
CarFight_ReEditor Win64 Development
PASS / Exit 0
```

첫 final RuntimeApply 전체 실행은 10/11 PASS였고 `ExplicitEquipmentApply`만 실패했다. 원인은 Product UI/service가 아니라 테스트가 downstream Fitting candidate `ValidationFailed`를 Catalog authorization 실패로 잘못 해석한 것이었다.

교정 원칙:

```text
exact Catalog membership validation
!=
Fitting candidate mass/data validation
```

`CFRuntimeApplyUITests.cpp v1.0.5`에서 exact Catalog authorization을 `ValidateCatalogEquipmentCandidate()`로 별도 증명하고, 현재 Catalog 장비가 필수 Fitting 질량 계약을 만족하지 못하는 경우 `ValidationFailed`를 정상 fail-closed 결과로 인정했다. 이 경우에도 이전 Current Equipment와 VehicleFittingData source identity가 유지되고 UI Last Result가 `검증 실패`를 그대로 표시하는지 검증한다.

최종 교정본 official build:

```text
de1f8438f4094f6881b48095fe59aaf0
CarFight_ReEditor Win64 Development
PASS / Exit 0
```

최종 RuntimeApply 전체 자동화:

```text
c3ed1dbbc61843cf94303b82a6c60f7a
ENGINE_EXIT_CODE=0
SUCCESS_COUNT=11
FAILURE_COUNT=0
```

PASS 목록:

```text
RTA-P0-01 CatalogContract
RTA-P0-02 ApplyFailedRecovery
RTA-P0-02 RecoveryFailed
RTA-P0-02 VehicleApplySuccess
RTA-P0-02 VehicleValidation
RTA-P0-03 ApplyFailedRecovery
RTA-P0-03 EquipmentApplySuccess
RTA-P0-03 EquipmentValidation
RTA-P0-04 ExplicitEquipmentApply
RTA-P0-04 ExplicitVehicleApply
RTA-P0-04 PanelNavigation
```

따라서 현재 판정은:

```text
RTA-P0-04 Existing VehicleDebug UI Integration = Technical PASS
RTA-P0-05 PIE E2E = Ready — Next
```

P0-05 착수 시에는 P0-04에서 확인한 기본 Catalog Equipment data readiness gap을 먼저 해결하거나, 이미 유효성이 증명된 시연용 Vehicle/Fitting/Equipment 조합을 Catalog E2E fixture로 명시해야 한다. Product Fitting validation을 완화해서 통과시키는 방식은 사용하지 않는다.

---

## 17. RTA-P0-05 PIE E2E — Data Readiness Audit / Runtime Blocker Checkpoint

### 17.1 P0-05 최소 E2E 계약

P0-05는 P0-04 UI 연결을 반복 검증하는 단계가 아니라 실제 같은 Pawn에서 다음 연쇄를 검증한다.

```text
Vehicle A → B → A
→ Vehicle Apply 뒤 Legacy Fitting Runtime 유지
→ Mount/Profile 선택
→ Equipment A → B → Default → A
→ Legacy Fitting Runtime에서 transient Snapshot Fitting으로 승격
→ Current Equipment/Fitting/UI refresh
→ invalid/incompatible 후보는 fail-closed
→ 실패 뒤 직전 Current state 유지
```

Vehicle Apply가 `VehicleFittingData=nullptr`로 전환하더라도 `InitializeVehicleRuntime()`이 `PrepareInitialSortieFitting(nullptr, VehicleData, ...)`을 통해 Legacy Fitting Runtime을 Commit한다. 따라서 `HasAppliedRuntimeInput=true`는 유지된다.

`FCFRuntimeEquipApplyService::BuildTransientEquipmentCandidate()`는 current FittingData가 없거나 current VehicleData와 일치하지 않을 때 새 transient `UCFVehicleFittingData`를 만들고 `MissingMountSelectionPolicy=UseVehicleDefault`로 Vehicle default 계약을 이어받는다. 따라서 P0-05를 위해 P0-02/P0-03 service 구조를 다시 설계할 필요는 없다.

### 17.2 Persisted Data Readiness Audit

현재 기본 Runtime Catalog의 차량 3종은 Vehicle Apply 자체에는 사용할 수 있지만 성공형 Equipment E2E 기준으로는 부족하다.

```text
DA_TestSedan
- BaseVehicleMassKg = 0
- MaximumGrossMassKg = 0

DA_TestSUV
- BaseVehicleMassKg = 0
- MaximumGrossMassKg = 0

DA_Vehicle_Wagon
- Base/Maximum mass = 0
- MountProfiles 없음
```

반면 다음 기존 VehicleData는 성공형 P0-05 후보로 사용할 수 있다.

```text
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV

BaseVehicleMassKg = 1000
MaximumGrossMassKg = 2500
DefaultDefenseData = DA_VehicleDefense_Test
Mount = RoofTurret_MediumOrLarge
Default Equipment = RocketLauncher
DefenseMassKg = 100
```

생산용 Equipment의 TurretMount mass는 이미 유효하다.

```text
DA_CannonBody.TurretMountWeightKg = 350
DA_RocketBody.TurretMountWeightKg = 350
```

실제 blocker는 두 production WeaponData의 `WeaponMassKg=0`이다.

```text
DA_ProtoTurretCannon.WeaponMassKg = 0
DA_RocketLauncher.WeaponMassKg = 0
```

전체 persisted `CFWeaponData`를 감사한 결과 현재 유일한 non-zero 검증 기준은 다음이다.

```text
/Game/CarFight/Tests/VehicleDefense/Data/DA_Wpn_DefenseTest
WeaponMassKg = 120
```

따라서 P0-05 demo baseline은 새 임의 수치를 만들지 않고 기존 검증값 120kg을 두 prototype production weapon에 동일 적용한다. 두 장비의 TurretMount mass도 동일 350kg이므로 HeavyCannon ↔ RocketLauncher 교체 자체는 총질량 변화 없이 장비 Runtime 전환을 검증할 수 있다.

두 production WeaponData는 C++ 기본 계약상 `bUseInfiniteAmmoForDebug=true`, `DefaultAmmoData=nullptr` 조합을 유지하므로 P0-05 baseline에서 별도 sortie ammo mass를 만들지 않는다.

예상 Snapshot 총질량:

```text
Vehicle 1000
+ TurretMount 350
+ Weapon 120
+ Defense 100
= 1570 kg
```

### 17.3 승인된 최소 Asset Mutation 범위

P0-05 E2E 착수에 필요한 Asset mutation은 아래 3건으로 고정한다.

```text
1. /Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon
   WeaponMassKg: 0 → 120

2. /Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher
   WeaponMassKg: 0 → 120

3. /Game/CarFight/Debug/Data/DA_CFRuntimeTestCatalog_Default
   AllowedVehicleData에
   /Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV
   exact hard reference 1건 추가
```

보호 범위:

- `DA_TestSedan`, `DA_TestSUV`, `DA_Vehicle_Wagon`의 주행/Chaos tuning은 변경하지 않는다.
- TurretMount 350kg와 Defense 100kg는 이미 유효하므로 변경하지 않는다.
- 기존 Catalog Equipment 순서와 HeavyCannon/RocketLauncher 등록은 유지한다.
- Test fixture를 기본 Catalog에 넣지 않는다.
- Fitting `MissingMassSource` validation을 완화하지 않는다.
- CF-FQ-042 Builder dirty Source/Asset은 건드리지 않는다.

### 17.4 UE Runtime Coordination Closure / Persistent Mutation Capability Blocker

초기 failed start 뒤 남은 Project Runtime residue는 GoPyMCP `GOPY-RUNTIME-PROVENANCE-UNKNOWN` 교정에서 별도 해결됐다. 교정 소스가 반영된 GoPyMCP local runtime을 approved `gopymcp.local.restart_once`로 1회 활성화한 뒤 CarFight Consumer acceptance를 다시 실행했다.

```text
GoPyMCP restart job: a04c9e4e139a456da803d9d074bc7a2b
status: succeeded / Exit 0
new Core bridge session: core-session-d533e0a594f8
```

exact Project Editor start 결과:

```text
process job: 33bd0090a3c845a5bc606005892cd69e
project_id: carfight
capability_id: unreal
runtime_instance_id: runtime_7742c740df7a4bc386001ae1bac7241c
status: succeeded / Exit 0
editor_process_id: 39396
editor_ready: true
port_8100_owned_by_editor: true
protocol_ready: true
```

따라서 `runtime_coordination_busy` / `runtime_provenance_unknown`은 현재 CarFight Consumer 기준으로 해소되었고 Runtime Coordination blocker는 CLOSED다.

이후 exact 3 Asset mutation을 위해 live UE write capability를 확인했다. `ObjectTools.list_properties`는 current read policy에서 차단됐지만, 기존 persisted AssetDump audit으로 `WeaponMassKg`와 Catalog property 계약은 이미 확정되어 있으므로 exact `DA_ProtoTurretCannon.WeaponMassKg=120` 1건을 reviewed write probe로 실행했다.

결과:

```text
workflow = existing_data_asset_property_round_trip
forward_write_count = 1
forward_readback_verified = true
inverse_write_count = 1
cleanup_verified = true
dirty_state_preserved = true
saved = false
```

즉 현재 exposed `CarFightMCP_UE_WriteProbe`는 실제 persistent authoring surface가 아니라 `set → readback → inverse → cleanup` probe 계약이며 변경을 저장하지 않는다. 첫 WeaponMassKg 호출도 원복되었고 현재 Asset mutation은 0건이다.

따라서 새 blocker는 Product/Asset/Runtime Coordination 문제가 아니라 **현재 Accepted/Exposed persistent UE Asset mutation capability 부재**로 분리한다. 이를 우회하기 위한 Slate UI 자동화, raw commandlet, `.uasset` 직접 수정, generic process/script 우회는 사용하지 않는다.

### 17.5 Data Prep / Regression / 실제 PIE Technical Closure

USER가 승인된 exact 3 Asset을 Editor에서 직접 수정·저장한 뒤 fresh AssetDump persisted evidence로 다음을 확인했다.

```text
DA_ProtoTurretCannon.WeaponMassKg = 120
DA_RocketLauncher.WeaponMassKg = 120
DA_CFRuntimeTestCatalog_Default.AllowedVehicleData = 4
- DA_TestSedan
- DA_TestSUV
- DA_Vehicle_Wagon
- DA_VehicleDefense_TestSUV
AllowedEquipmentPresetData = HeavyCannon / RocketLauncher 2개 유지
```

Weapon persisted dataset:

```text
adset_v1_11a9afca6067679f97cf9957631cbf87.ba4538dfcb4bc668aff97fd5
```

Catalog persisted dataset:

```text
adset_v1_9cea1be52c1f9cd742e339551daf6eff.8c25321503f7854647ba5cd5
```

데이터 저장 직후 official build `cf9341230c2542a58a6a050e0ea2e626`는 PASS / Exit 0이었다. 첫 RuntimeApply 전체 regression `a1281c5750e24357bb1528d3f48ab16f`은 Product failure가 아니라 기존 P0-01 test가 Catalog Vehicle 수를 3으로 고정한 stale expectation 때문에 10/11이었다.

`CFRuntimeTestCatalogTests.cpp v1.0.2`에서 단순 수치만 4로 바꾸지 않고 `DA_VehicleDefense_TestSUV` exact membership도 함께 검증하도록 교정했다.

교정 후 official build:

```text
ea632a2cc25244a2a6a5486e586cb3e9
CarFight_ReEditor Win64 Development
PASS / Exit 0
```

교정 후 기존 RuntimeApply regression:

```text
5aa771f0bac8450197669d3dec5ec943
ENGINE_EXIT_CODE=0
SUCCESS_COUNT=11
FAILURE_COUNT=0
```

그 뒤 live managed Editor를 다시 시작하려는 canonical Project lifecycle은 물리 Editor가 이미 없는 상태에서 `runtime_protected_dirty`로 차단됐다. 이 잔여 보호 상태는 Product/Asset failure로 보지 않고 GoPyMCP Runtime Coordination 운영 이슈로 분리했다. 보호 상태를 지우거나 raw start/kill로 우회하지 않았다.

P0-05 Technical E2E는 기존 `CFFieldFitMassTests.cpp`가 사용하던 실제 PIE Automation 패턴을 재사용해 live Registry blocker와 분리했다.

신규 테스트:

```text
UE/Source/CarFight_Re/Private/UI/CFRuntimeApplyPIETests.cpp v1.0.0
CarFight.RuntimeApply.RTA_P0_05.PIEE2E
```

실제 검증 연쇄:

```text
FEditorLoadMap(/Game/Maps/M_VehicleDefensePIE)
→ FStartPIECommand(false)
→ actual EWorldType::PIE World
→ fresh BP_CFVehiclePawn
→ DefenseSUV → TestSUV → DefenseSUV
→ final Legacy Fitting Runtime 확인
→ RoofTurret_MediumOrLarge
→ HeavyCannon → RocketLauncher → HeavyCannon
→ Legacy → transient Snapshot 승격
→ Chaos configured mass 1570kg 확인
→ Catalog 밖 transient Equipment 요청
→ ValidationFailed
→ Fitting source / current Equipment / configured mass 보존
→ FEndPlayMapCommand()
```

P0 RuntimeApply UI에는 별도 Equipment clear/default action이 존재하지 않으므로 테스트 전용 가짜 UX를 추가하지 않았다. 현재 실제 UI가 제공하는 Catalog Equipment 반복 교체만 검증했다.

PIE E2E 추가 후 official build:

```text
70ac76bdddd045039ee68d45f40f86dc
CarFight_ReEditor Win64 Development
PASS / Exit 0
```

focused actual PIE:

```text
57f4933794dc4363b95c5a1df657d581
CarFight.RuntimeApply.RTA_P0_05.PIEE2E
SUCCESS_COUNT=1
FAILURE_COUNT=0
```

최종 전체 RuntimeApply regression:

```text
95f9df7bfad248c09013d7b48c3043a3
ENGINE_EXIT_CODE=0
SUCCESS_COUNT=12
FAILURE_COUNT=0
```

따라서 P0-05의 Vehicle/Equipment 반복 적용, 실제 PIE World, Legacy→Snapshot 전환, 1570kg 질량, invalid fail-closed와 기존 P0-01~04 regression은 모두 **Technical PASS**다.

### 17.6 2026-09-02 당시 Gate (Historical)

```text
RTA-P0-05 Service Architecture Audit = PASS
RTA-P0-05 Persisted Data Readiness = PASS
RTA-P0-05 Exact Data Prep = PASS
RTA-P0-05 Runtime Provenance Consumer Acceptance = PASS
RTA-P0-05 Actual PIE Automation = PASS
RTA-P0-05 RuntimeApply Full Regression = 12/12 PASS
RTA-P0-05 Technical = PASS
RTA-P0-05 USER PIE Visual/Interaction = PASS
RTA-P0-05 = CLOSED
GoPyMCP live lifecycle runtime_protected_dirty residue = Separate Infrastructure Issue
RTA-P0-06 Packaged Demo = READY
```

당시 USER가 실제 RuntimeApply 화면에서 Vehicle 선택/적용, Equipment dropdown의 HeavyCannon/RocketLauncher 2개 표시, DefenseSUV 초기 Current Equipment readback, Equipment 적용 흐름을 확인해 P0-05 USER Gate를 PASS했다. 당시 다음 Product Gate는 RTA-P0-06 Packaged Demo E2E였다. 이 2026-09-02 evidence는 Historical로 보존하며 현재 2026-09-14 checkpoint는 문서 상단 Status와 v0.1.18 Changelog를 따른다. `runtime_protected_dirty`는 canonical live lifecycle을 다시 막는 경우 GoPyMCP owner에서 별도 교정한다.

---

## 18. Changelog

### v0.1.18 - 2026-09-14

- USER가 Builder-produced `DA_Vehicle_Wagon` 2-Mount 상태에서 `Mount_Top_01`에 장비를 Runtime Apply할 때 비대상 `Mount_Front_01`의 정상 빈 장착이 `MissingEquipmentPreset`으로 오인되어 전체 Fitting Snapshot validation이 실패하는 post-closure 회귀를 확인했다.
- `CFRuntimeEquipApply.cpp v1.3.0`에서 Legacy→Snapshot 첫 장비 적용 시 VehicleData의 모든 Mount를 완전하게 materialize한다. 기본 장비가 있는 Mount는 활성 선택, 기본 장비가 없는 Mount는 `ExplicitEmpty`로 만들고 전체 상태 구성 뒤 누락은 `TreatAsError`로 fail-closed한다.
- 기존 `VehicleFittingData` 선택 상태는 같은 VehicleData 포인터만으로 authoritative하다고 보지 않고 실제 `VehicleFittingComp`에 Applied Snapshot이 있을 때만 보존하도록 authority를 교정했다.
- `CFRuntimeApplyPIETests.cpp v1.3.0`은 USER-approved Wagon Hardpoint/Mount 2/2 baseline으로 갱신했다. production HeavyCannon은 정상 Empty Mount 오류가 아닌 실제 GrossMass guard에 도달해야 하며, 질량만 낮춘 transient 후보는 Top Mount에 성공하고 Front Mount는 `ExplicitEmpty`로 유지되어야 한다.
- `Document/Systems/Vehicles/RuntimeApply.md v1.0.0`을 Current System으로 승격해 현재 Catalog/VehicleDebug frontend와 재사용 가능한 Runtime Apply application seam을 분리했다. 향후 Garage/Inventory는 소유권·후보 정책을 추가하되 Fitting Snapshot/Prepare/Commit/Recovery 알고리즘을 중복 구현하지 않는다.
- Official UE 5.8 Editor Build `f81a594cb3df4309899d35a4e6fea11d`는 PASS다. 이어 fresh RuntimeApply Automation Process Job `a96777df47924589a909ddf4cd4d965b`에서 `EngineExitCode=0 / Success=16 / Failure=0`을 확보했고 `CarFight.RuntimeApply.CF_FQ_047.WagonMountEquipmentPIE`를 포함한 전체 RuntimeApply regression이 PASS했다. 기존 RTA-P0-05 USER PASS와 과거 RuntimeApply 14/14 PASS는 Historical evidence로 별도 보존한다.
- Multi-Mount Empty-State remediation은 fresh RuntimeApply 16/16으로 **Technical PASS**다. Exact next gate는 기존 `RTA-P0-06 Packaged Demo`다. RTA-P0-05 당시 USER PASS 자체는 Historical evidence로 보존한다.

### v0.1.17 - 2026-09-03

- `CF-FQ-044 / VRCP-P0-06` closure handoff를 반영했다. Builder-produced `DA_Vehicle_Wagon`이 exact USER Driving PASS 뒤 Default Runtime Catalog에 promotion되고 USER explicit Save까지 완료됐다.
- fresh persisted AssetDump에서 `AllowedVehicleData` 4개 중 Wagon exact membership 1개를 확인해 RTA-P0-06 Packaged Demo의 대표 Builder-produced Vehicle candidate로 고정했다.
- RuntimeApply allowlist/authorization과 실제 Vehicle/Equipment Apply 계약은 계속 CF-FQ-041이 소유한다. 이 handoff는 packaged executable E2E PASS를 의미하지 않으며 RTA-P0-06은 Ready 상태다.

### v0.1.16 - 2026-09-02

- USER가 수정 후 PIE에서 `DA_VehicleDefense_TestSUV` 초기 Current Equipment가 `Prototype Rocket Launcher Kit`로 정확히 표시되는 것을 확인했다.
- 앞선 USER 확인으로 Equipment dropdown이 정상적으로 열리고 `Prototype Roof Cannon Kit` / `Prototype Rocket Launcher Kit` 두 항목이 표시되는 것도 확인됐다.
- Vehicle Runtime Apply, Equipment 선택/표시/적용 UX를 포함한 RTA-P0-05 USER Gate를 PASS로 닫았다.
- Technical evidence는 RTA-P0-04 focused 4/4, 최종 RuntimeApply 13/13 PASS를 유지한다.
- RTA-P0-05를 Closed로 전환하고 RTA-P0-06 Packaged Demo를 Ready로 전진했다.

### v0.1.15 - 2026-09-02

- USER PIE에서 `DA_VehicleDefense_TestSUV`를 처음 Runtime Apply했을 때 실제 RocketLauncher가 시각/Weapon Runtime에 활성화되어 있는데도 `현재 장비: Snapshot 없음 (Legacy/기본 Runtime)`으로 표시되는 readback 불일치를 확인했다.
- 원인은 `CFRuntimeApplyWidget::FindCurrentAppliedEquipment()`가 Applied Fitting Snapshot만 authoritative source로 읽고 Snapshot이 없는 Legacy Runtime의 실제 `VehicleWeaponComp` 활성 장비를 보지 않는 구조로 확정했다.
- `CFRuntimeApplyWidget.cpp v1.0.2`에서 Current Equipment readback을 `Applied Snapshot exact Mount 우선 → Snapshot 없음이면 VehicleWeaponComp의 ActiveMountProfileId/ActiveEquipmentPresetData 차선 → 실제 장비 없음` 순서로 교정했다. Snapshot authority는 유지하고 Legacy fallback은 실제 Weapon Runtime readback만 사용한다.
- `CFRuntimeApplyUITests.cpp v1.0.6`에 `CarFight.RuntimeApply.RTA_P0_04.LegacyEquipmentReadback`을 추가해 `DA_VehicleDefense_TestSUV`의 Snapshot 없는 Legacy Runtime에서 실제 RocketLauncher가 Current Equipment UI에 표시되고 `Snapshot 없음` 문구가 사라지는 계약을 검증한다.
- USER가 수정 후 공식 Editor build 성공을 보고했다. focused RTA-P0-04 `e9c538aebfe442b2ab1b6833d050c130` 4/4 PASS, 최종 RuntimeApply `49101e0040b2444b8b00064246da4b3d` 13/13 PASS를 확인했다.
- Technical regression은 PASS이며 실제 USER PIE에서 초기 `현재 장비: Prototype Rocket Launcher Kit` 표시를 재확인하는 Gate만 남긴다.

### v0.1.14 - 2026-09-02

- USER PIE에서 Equipment ComboBox가 클릭해도 열리지 않는 현상을 재현 증거로 접수했다.
- 원인을 `UCFVehicleDebugPanelWidget::NativeTick()`의 매 프레임 `RefreshSelectedSectionWidget()` → `UCFRuntimeApplyWidget::SetVehiclePawnRef()` → `RefreshRuntimeApplyState()` 연쇄에서 동일 VehicleData에도 `RebuildEquipmentOptions()`가 매 Tick 호출되어 `UComboBoxString::ClearOptions()`가 열린 popup을 즉시 닫는 UI lifecycle 버그로 확정했다.
- `CFRuntimeApplyWidget.cpp v1.0.1`에서 동일 VehicleData의 일반 Refresh에서는 Equipment options를 재구성하지 않고, VehicleData 변경 또는 사용자의 Mount 변경 시에만 rebuild하도록 최소 교정했다.
- USER가 수정 후 공식 Editor build 성공을 보고했다. 추가 AI regression으로 RTA-P0-04 focused `d7582e5b71fa4a338ba01781a89be452` 3/3 PASS, RTA-P0-05 actual PIE `6110f74b5d9042b1bab9b429ba23cf7f` 1/1 PASS, 최종 RuntimeApply `3b8f5515407a4026bfac7f97a42b0da2` 12/12 PASS를 확인했다.
- Product/Runtime regression은 Technical PASS이며, 실제 Equipment dropdown이 열린 상태를 유지하고 HeavyCannon/RocketLauncher 2개가 표시되는지는 USER PIE 재확인을 Pending으로 유지한다.

### v0.1.13 - 2026-09-02

- USER가 P0-05 exact 3 Asset을 수동 저장한 뒤 AssetDump persisted evidence로 production WeaponMassKg 120/120, Catalog Vehicle 4개와 DA_VehicleDefense_TestSUV exact 등록, Equipment 2개 보존을 확인했다.
- 데이터 변경 뒤 기존 CatalogContract의 차량 수 3 고정 expectation을 stale test로 분류하고 `CFRuntimeTestCatalogTests.cpp v1.0.2`에서 차량 4개 + DA_VehicleDefense_TestSUV exact membership 검증으로 교정했다.
- 교정 후 official build `ea632a2cc25244a2a6a5486e586cb3e9` PASS, 기존 RuntimeApply `5aa771f0bac8450197669d3dec5ec943` 11/11 PASS를 확인했다.
- canonical live Editor 재시작이 `runtime_protected_dirty`로 막힌 상태는 Product failure와 분리하고 보호 해제/강제 우회를 하지 않았다.
- 기존 actual PIE automation 패턴을 재사용하는 `CFRuntimeApplyPIETests.cpp v1.0.0`을 추가해 DefenseSUV→TestSUV→DefenseSUV, HeavyCannon→RocketLauncher→HeavyCannon, Legacy→Snapshot, 1570kg mass와 invalid Catalog Equipment fail-closed 보존을 실제 PIE World에서 검증했다.
- PIE E2E build `70ac76bdddd045039ee68d45f40f86dc` PASS, focused `57f4933794dc4363b95c5a1df657d581` 1/1 PASS, 최종 RuntimeApply `95f9df7bfad248c09013d7b48c3043a3` 12/12 PASS를 확인했다.
- RTA-P0-05를 Technical PASS로 전진하고 USER PIE Visual/Interaction 확인만 Pending으로 유지한다. RTA-P0-06은 USER Gate 결정 전 HOLD다.

### v0.1.12 - 2026-09-02

- GoPyMCP provenance 교정 소스를 approved local restart 1회로 활성화하고 CarFight exact Project Consumer acceptance를 재실행했다.
- `carfight/unreal/editor.start`가 새 managed Runtime `runtime_7742c740df7a4bc386001ae1bac7241c`를 생성하고 Editor PID 39396의 8100 listener ownership과 protocol Ready를 증명하며 PASS했다.
- 기존 `runtime_coordination_busy` / `runtime_provenance_unknown` blocker를 CLOSED로 전환했다.
- P0-05 exact Asset mutation 착수에서 current `UE_WriteProbe`가 persistent authoring이 아니라 forward write/readback 뒤 inverse cleanup을 수행하는 probe-only 계약임을 확인했다.
- `DA_ProtoTurretCannon.WeaponMassKg=120` probe는 `forward_write_count=1 / inverse_write_count=1 / cleanup_verified=true / saved=false`로 종료되어 실제 Asset 변경 0건임을 확인했다.
- exact 3 Asset data prep 계약은 그대로 LOCKED로 유지하며, persistent UE Asset mutation capability 확보 또는 USER exact 수동 저장 전에는 AssetDump readback/PIE E2E로 전진하지 않는다.

### v0.1.11 - 2026-09-02

- 사용자 GoPyMCP 재시작 후 Core health와 exact CarFight Project Checkout authority가 정상임을 재확인했다.
- 기존 `runtime_coordination_busy`/`stale=runtime` 상태는 더 이상 재현되지 않았으나, canonical `carfight.editor.start`와 exact `carfight/unreal/editor.start`가 모두 pre-dispatch `runtime_provenance_unknown`으로 차단됐다.
- `git.repo_info(project_id=carfight)`로 `checkout_eefe2944e0954bed9270908eb95d8fd0`, repository_role `main_game`의 exact Project authority가 정상임을 확인했지만 Editor lifecycle provenance는 확립되지 않았다.
- UE endpoint는 `ERR_UE_MCP_UNAVAILABLE` 상태이고, 추가 start 반복이나 generic script 우회, user-owned/unknown Editor 강제 종료는 수행하지 않았다.
- P0-05 Asset/Data contract는 변경 없이 LOCKED 상태로 유지하고 Asset mutation/PIE E2E만 provenance 정상화 이후로 보류한다.

### v0.1.10 - 2026-09-02

- RTA-P0-05 PIE E2E 착수 전 Vehicle→Equipment 연쇄 authority를 감사해 Vehicle Apply 뒤 Legacy Fitting Runtime에서 Equipment Apply가 transient Snapshot Fitting으로 정상 승격 가능한 구조임을 확정했다.
- 기본 Catalog 차량의 fitting mass/mount gap과 production HeavyCannon/RocketLauncher의 WeaponMassKg=0을 성공형 Equipment E2E의 실제 data readiness blocker로 확정했다.
- persisted CFWeaponData 전수 감사에서 유일한 기존 non-zero 검증값인 DefenseTestLauncher 120kg을 두 prototype production weapon의 P0 demo baseline으로 재사용하기로 고정했다.
- DA_VehicleDefense_TestSUV 1000/2500kg + Turret 350kg + Weapon 120kg + Defense 100kg = 1570kg expected Snapshot을 P0-05 성공형 기준으로 확정했다.
- Asset mutation 범위를 production WeaponData 2건의 WeaponMassKg 120 설정과 default Runtime Catalog의 DA_VehicleDefense_TestSUV hard reference 추가 1건으로 제한했다.
- canonical Editor start가 8100 Ready timeout 후 AI-owned cleanup은 성공했으나 Project Registry가 Configured/stale runtime 상태로 남아 fresh start가 runtime_coordination_busy로 차단됨을 기록했다.
- GoPyMCP Core/UE env 자체는 diagnostics PASS이며 raw kill/commandlet 우회 없이 Asset mutation과 PIE E2E를 Pending으로 유지한다.

### v0.1.9 - 2026-09-02

- 사용자 정상 Editor 종료 후 P0-04 final validation blocker를 해소하고 Existing VehicleDebug UI Integration을 Technical PASS로 닫았다.
- official build `2a678b29fe324484b07e96c778b04424` PASS 후 첫 전체 RuntimeApply에서 10/11 PASS를 확인했고, 유일 실패를 Catalog authorization과 downstream Fitting candidate validation을 혼동한 UITest 가정으로 진단했다.
- `CFRuntimeApplyUITests.cpp v1.0.5`에서 exact Catalog authorization을 별도 검증하고, 필수 질량 데이터 부족에 따른 `ValidationFailed`도 mutation 없는 정상 fail-closed 결과로 검증하도록 교정했다.
- 최종 official build `de1f8438f4094f6881b48095fe59aaf0` PASS, RuntimeApply 전체 `c3ed1dbbc61843cf94303b82a6c60f7a` 11/11 PASS를 확인했다.
- RTA-P0-05 PIE E2E를 Ready — Next로 전진했다. 기본 Catalog의 Equipment 성공 E2E data readiness gap은 P0-05 선행 준비로 유지한다.

### v0.1.8 - 2026-09-02

- RTA-P0-04 VehicleDebug UI Integration 구현을 완료했지만 최종 UITest v1.0.4 binary link가 user-owned/unknown Editor DLL lock에 막혀 Technical PASS로 확대하지 않았다.
- 기존 VehicleDebug Dynamic Navigation에 `RuntimeApply` dedicated C++ child를 추가하고 Current/Selected 분리, Vehicle/Mount/Equipment 선택, Explicit Apply, compatibility와 Last Result를 구현했다.
- WBP Asset mutation 없이 C++ WidgetTree를 사용했으며 Runtime mutation은 기존 Vehicle/Equipment Apply service에만 위임했다.
- 최종 UI test를 Navigation / Explicit Vehicle / Explicit Equipment 3개로 구성하고 Equipment test는 성공과 안전한 ApplyFailed 양쪽 service 결과를 검증하도록 단계 책임에 맞게 교정했다.
- Product UI 구현은 full build `15aa96326a454c93b34d386b42372e30` PASS. v1.0.4 UITest source는 `7133c6d23dc641258b5b0326342c1ced`에서 compile PASS했으나 DLL link가 external Editor lock으로 차단됐다.
- 단일 재시도 `60bd79d620f0404b8337554d1ef537eb`도 동일 LNK1104로 실패했고 UE MCP unavailable이라 managed provenance를 증명하지 못해 Editor를 자동 종료하지 않았다.
- 기본 Catalog의 Sedan/SUV mass 0, Wagon mass 0 + Mount 없음 등 Equipment 성공 E2E data readiness gap을 P0-05 선행 이슈로 기록했다.
- P0-04 final build + RuntimeApply 11/11 전에는 RTA-P0-05를 시작하지 않는다.

### v0.1.7 - 2026-09-02

- RTA-P0-03 Equipment Slot Runtime Apply를 Technical PASS로 닫고 next Gate를 RTA-P0-04 Existing VehicleDebug UI Integration으로 전진했다.
- `FCFRuntimeEquipApplyService`를 추가해 exact Catalog authorization, transient Fitting candidate, 단일 Mount 교체, Fitting Prepare/Commit, Chaos Mass reapply, dependent runtime refresh와 final readback을 구현했다.
- 이전 VehicleFittingData identity + Applied Fitting Runtime + configured mass checkpoint와 compensation recovery를 구현하고 ApplyFailed/RecoveryFailed를 구분했다.
- `CFVehicleFittingComp`에는 기존 vehicle-facing Adapter를 재사용하는 checkpoint restore wrapper만 추가하고, `CFVehiclePawn`에는 Ammo/TurretVisual/Launcher/CombatReady 전용 refresh seam만 추가했다.
- final UE 5.8 Build `3b7a3d250f9549a39fb7d6d05a2a5454` PASS, RuntimeApply 8/8 PASS를 확인했다.
- Fitting broad regression은 23/24 PASS였으며 직접 영향 atomic/mass/recovery/Defense 경로는 전부 PASS했다. 유일 실패 QuantitativeMobility는 30초 PIE World Actor 초기화 timeout으로 분리 기록했다.
- broad regression을 위해 임시 변경한 `RunRuntimeApplyTests.ps1` 기본 filter는 `CarFight.RuntimeApply`로 원복했다.
- 순간 탄창/예비탄/Reload 상태는 P0 transaction checkpoint 범위가 아니며 Apply/Recovery 후 Ammo는 Applied Snapshot의 InitialSortieAmmoLoads 기준으로 재구성됨을 명시했다.

### v0.1.6 - 2026-09-02

- RTA-P0-02 Vehicle Runtime Apply를 Technical PASS로 닫고 next Gate를 RTA-P0-03 Equipment Slot Runtime Apply로 전진했다.
- CFVehiclePawn과 concurrent CF-FQ-042 Builder Source를 수정하지 않는 독립 `FCFRuntimeVehicleApplyService`를 추가했다.
- Builder Step 8의 transient VehicleData + same-Pawn `InitializeVehicleRuntime()` 경로를 재사용하고 previous VehicleData/Fitting checkpoint, 실패 복구, ApplyFailed/RecoveryFailed 구분과 bounded runtime readback을 구현했다.
- final UE 5.8 Build PASS, RTA focused 4/4 PASS, InitialMass / RuntimeReinitializeVisual / DefensePIEPipeline affected regression 3/3 PASS를 확인했다.
- 공용 Automation 로그 file-lock은 concurrent work를 건드리지 않고 `RunRuntimeApplyTests.ps1` 전용 로그로 격리했다.

### v0.1.5 - 2026-09-01

- RTA-P0-01 final code review에서 Catalog 배열이 `EditDefaultsOnly`라 DataAsset instance에서 수동 편집 불가한 문제를 발견했다.
- USER가 새 차량/장비를 Catalog에 수동 등록하는 P0 운영 계약에 맞춰 두 배열을 `EditAnywhere`로 교정했다.
- 최종 Official Build `17cf5dca1d8c4d738a3326774f835c34` PASS, focused `2d4d2ea632054e6bb470aeeaa35ee8ff` 1/1 PASS 후 fresh AssetDump에서 두 배열 `editable=true`, Vehicle 3 / Equipment 2, hard references 5를 재확인했다.
- RTA-P0-01 Technical PASS와 next `RTA-P0-02 Vehicle Runtime Apply` 상태는 유지한다.

### v0.1.4 - 2026-09-01

- `RTA-P0-01 Runtime Catalog / Packaged Load Contract`을 Technical PASS로 닫았다.
- `UCFRuntimeTestCatalogData`와 `UCFRuntimeTestSettings`를 추가해 명시 hard-reference 목록과 Config 기반 runtime locator/load 경로를 구현했다.
- `/Game/CarFight/Debug` bounded Always Cook 설정과 기본 Catalog Asset을 추가했다.
- 기본 Catalog는 TestSedan/TestSUV/Wagon + HeavyCannon/RocketLauncher만 등록하며 persisted AssetDump hard dependency 5개를 확인했다. Tests/Legacy 자동검색은 없다.
- final Official Build PASS와 `CarFight.RuntimeApply.RTA_P0_01.CatalogContract` 1/1 PASS에서 persisted Catalog actual runtime load, Vehicle 3 / Equipment 2와 validation을 확인했다.
- 실제 full packaged executable E2E는 기존 계획대로 RTA-P0-06 소유로 남기고 next Gate를 `RTA-P0-02 Vehicle Runtime Apply`로 전진했다.
- CFVehiclePawn/Vehicle Builder/VehicleData schema와 기존 Vehicle/Equipment Asset은 수정하지 않았다.

### v0.1.3 - 2026-09-01

- `RTA-P0-00 Current Runtime Apply Contract Audit`을 read-only Source/Config/Asset evidence로 PASS했다.
- Builder Step 8 same-Pawn Vehicle reinitialize를 canonical Vehicle Apply authority로 재확인하고 failure recovery 요구를 추가했다.
- 현재 VehicleData 12 / EquipmentPresetData 10에 Test·Legacy가 혼재하며 AssetManager가 해당 타입을 runtime scan하지 않음을 확인해 등록형 Hard Reference Catalog를 P0 방식으로 확정했다.
- InventoryFitAdapter는 Inventory binding 때문에 직접 재사용하지 않고 내부의 transient VehicleFittingData → BuildFittingSnapshot domain pattern만 재사용하도록 경계를 확정했다.
- Fitting Commit + Chaos Mass는 존재하지만 Ammo/Launcher/Turret post-Fitting refresh를 묶은 packaged public operation이 없고 FieldFitRuntimeAdapter가 Legacy 첫 apply를 받지 못하는 Integration Gap을 확인했다. RTA-P0-03에서 이를 최소 Runtime Content Apply Service로 닫도록 지정했다.
- 기존 VehicleDebug generic FieldRow는 read-only로 보존하고 `RuntimeApply` Navigation + dedicated child widget을 최소 UI 삽입 방식으로 확정했다.
- next gate를 `RTA-P0-01 Runtime Catalog / Packaged Load Contract`로 전진했으며 Product Source/Asset 구현은 아직 0이다.

### v0.1.2 - 2026-09-01

- P0 Asset 공급 방식을 등록형 Runtime Catalog/Registry로 명확히 고정했다. 등록된 VehicleData/EquipmentPresetData만 노출하며 프로젝트 전체 Runtime 자동 검색은 하지 않는다.
- 아직 Garage/Inventory/File Management가 없는 현재 단계에서 Catalog를 최소 Selection Source로 사용하고, 향후 Inventory/Garage가 후보 Source를 대체해도 Runtime Apply 경로는 유지하도록 경계를 명시했다.
- P0는 Catalog 수동 등록을 사용하며 Builder 자동 등록과 별도 파일관리 시스템은 Scope Out했다.
- 표시명은 기존 Identity/DisplayName을 우선하고 없으면 Asset 이름을 사용하며, 이 기능을 위해 VehicleData Schema를 선행 확장하지 않도록 했다.
- Vehicle current implementation 설명의 stale `전체 차량 교체 별도 감사` 문구와 P0 `Possess` 표현을 제거하고 same-Pawn `InitializeVehicleRuntime()` 계약과 일치시켰다.

### v0.1.1 - 2026-09-01

- USER 지적으로 기존 Guided Vehicle Builder Step 8의 실제 차량 전환 경로를 재확인했다.
- Step 8은 current Player VehiclePawn을 유지한 채 saved VehicleData transient duplicate를 `VehicleData`에 지정하고 `VehicleFittingData=nullptr` 뒤 `InitializeVehicleRuntime()`을 재호출하는 방식이며, 이 Runtime 초기화 함수는 Editor 전용이 아니다.
- CF-FQ-041 Vehicle P0 전략을 동일-Pawn transient VehicleData reinitialize 재사용으로 고정하고 Pawn Spawn/Possess/Replacement 후보 설계를 제거했다.
- RTA-P0-00 Vehicle 감사 범위를 전략 선택이 아니라 Builder Editor wrapper에서 Runtime 공용 seam을 분리하고 Packaged-safe 호출 위치·추가 regression을 확정하는 작업으로 축소했다.

### v0.1.0 - 2026-09-01

- USER 결정으로 CF-FQ-041을 정식 Ready Plan으로 신규 승격했다.
- 기능을 차량/무기 제작이 아니라 Existing Asset Runtime Apply로 고정했다.
- PIE 전용이 아니라 시연용 Packaged Build를 P0 완료 조건에 포함했다.
- 기존 VehicleDebug Panel을 임시 Frontend로 재사용하고 별도 정식 UI 제작을 Scope Out했다.
- Runtime 적용 Backend는 Debug UI에 종속하지 않고 VehicleData/Fitting의 현재 Authority를 재사용하도록 고정했다.
- Weapon 직접 교체 대신 MountProfile + EquipmentPresetData + Fitting Snapshot 계약을 사용하도록 정리했다.
- Inventory/Fitting 후속 재사용 가능성을 보존하되 Inventory 소유권·정식 Field UI·Runtime 튜닝·Asset Authoring은 P0에서 제외했다.
- 실패 시 기존 상태 보존, Current/Selected 분리, Explicit Apply, Packaged Cook/Load 보장을 필수 계약으로 추가했다.
- 첫 Gate를 read-only `RTA-P0-00 Current Runtime Apply Contract Audit`으로 지정했다.
