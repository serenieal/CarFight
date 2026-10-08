# Vehicle Fitting Roadmap

- Version: 0.15.0
- Date: 2026-08-17
- Status: Active Roadmap / `M0~M4` Done / `FFIT-M0~M4` Technical Done / M5 Initial Sortie·Chaos Mass + Official Fixture + Quantitative Mobility Technical PASS·USER Driving Feel Pending / M6 ViewData C++·Blueprint Contract Technical Done·UI Readability Pending
- Feature: `CF-FQ-034 차량 피팅·질량 런타임`
- Representative Plan: `Document/Plan/VehicleFitting/VehicleFittingPlan.md v0.17.0`
- Design: `Document/Plan/VehicleFittingDesign.md`
- Current Active Feature: `CF-FQ-034 차량 피팅·질량 런타임`
- Preserved Paused Feature: `CF-FQ-032` — USER Visual checkpoint

---

## 1. 목표

피팅 시스템을 결정 잠금, 데이터 계약, 검증·질량 Snapshot, 출격 적용, 차량 기동 반영, Preview·Debug와 통합 검증으로 나누어 기존 전투·차량 런타임을 깨지 않는 작은 단계로 구현한다.

```text
Investigation
→ Decision Lock
→ Data Contract
→ Validation / Mass Snapshot
→ Sortie Apply
→ Mass / Mobility
→ ViewData / Debug
→ Inventory Foundation 연동
→ Field Timed Equip / Unequip
→ Test Assets / PIE
→ Integrated Verification
```

---

## 2. 현재 단계

```text
M0 실제 구현 조사와 문서 준비: Done
M1 질량·소유권·적용 정책 결정: Done
M2 Fitting Data Contract: Done — Build·DataContract PASS
M3 Validation·Mass Snapshot: Done — Official Build·Automation PASS
FFIT-M0 Field Fitting Action Contract: Done — Documentation
M4 Sortie Apply Runtime: Done — Official Build·Automation PASS
M5 Vehicle Mass·Mobility: In Progress — Initial Sortie + Field Chaos Mass + Official Light·Default·Heavy Fixture + Quantitative Mobility Technical PASS / FIT-P0-07D USER Driving Feel Pending
M6 ViewData·Debug: Technical Done — C++ ViewData·Blueprint Contract PASS / 16:9·32:9 UI readability Pending
FFIT-M1 Permission·Blocker Query: Done — Code·Build·Automation PASS
FFIT-M2 Timed Action·Reservation: Done — Code·Build·Automation PASS
FFIT-M3 Atomic Equip·Unequip: Technical Done — TimedAction→Coordinator handoff PASS
FFIT-M4 Field Mass Reapply: Technical Done — Chaos Runtime·RecoveryFailed·saved-map PIE PASS
FFIT-M5 Field UI·PIE: Not Started — FIT-P0-06 + CF-FQ-032 USER UI availability
M7 Test Assets·PIE: Technical Fixture + Quantitative Measurement Done — Official Light/Default/Heavy persisted + fresh-PIE metrics captured / USER Mobility PIE Pending
M8 Integration·Systems 승격: Not Started
```

이 로드맵의 현재 단일 Active는 `CF-FQ-034`다. `FIT-P0-07C`까지 원격 기술 체크포인트, 공식 Light 1000kg / Default 1570kg / Heavy 1600kg 저장 Fixture와 동일 조건 정량 계측이 닫혔다. 다음 Fitting Gate는 `FIT-P0-07D USER Driving Feel Comparison`이며, 그 뒤 `FFIT-M5 Field UI·PIE`와 실제 UI 가독성이 남는다. 정량 가속·coast-steering 값이 단순 질량 순서로 단조 변화하지 않았지만 자동 Mobility Scalar를 추가하지 않는다. `CF-FQ-035`와 `CF-FQ-032`는 각각 Field UI/USER와 USER Visual 체크포인트가 보존된 Paused 상태다.

---

## 3. 선행 관계

### 필수 기능 기반

```text
현재 구현
- VehicleData HardpointSlots / MountProfiles
- EquipmentPresetData
- TurretMountData
- WeaponData
- VehicleRuntime

연동 기능
- CF-FQ-031 Ammo Data·Runtime과 재장전·행동 잠금
- CF-FQ-032 UI Root·Screen·Panel·Modal·System Layer
- CF-FQ-033 VehicleDefense Data·Runtime
- CF-FQ-035 Item Instance·VehicleCargo·Reservation·Atomic Transfer
```

### 단계별 게이트

```text
M1~M3
- Ammo Runtime 없이도 피팅 기본 타입·장비 호환·정적 질량 설계 가능
- Ammo 타입을 임시 중복 구현하지 않음

M4
- Weapon 장비 override 적용 경로 확정 필요
- Defense 최종 선택 초기화 경로 확정 필요
- Field Action이 재사용할 Prepare·Commit·Rollback 진입점 필요

M5
- 정확한 Chaos Vehicle Mass 필드·Setter와 적용 시점 확인 필요
- Physics State 또는 Vehicle Simulation 재생성 계약 확인 필요
- VehicleMesh PhysicsAsset과 현재 계산 질량 확인 필요
- 이미 생성된 정지 차량에 대한 재적용 가능 여부 확인 필요

M6
- CF-FQ-032와 ViewData/UI 책임 경계 확인
- Saved Template·Draft·Applied 상태를 구분해 표시

FFIT-M1~M3
- CF-FQ-035 INV-P0-03 Reservation·Atomic Transfer 선행
- CF-FQ-035 INV-P0-04 Fitting Adapter 선행
- Combat·Drive·Weapon·Launcher·Ammo·Defense Blocker Query 필요

FFIT-M4~M5
- FIT-P0-05 Runtime Mass Gate 선행
- CF-FQ-032 비Pause Screen 입력·Focus 계약 사용

M7~M8
- CF-FQ-031 Ammo 연동 또는 명시적 Dependency Block 처리
- CF-FQ-033 사용자 PIE 결과와 Current 상태 재확인
- Field Equip·Unequip 취소·재시도·복제·유실 회귀 포함
```

---

## 4. 단계 요약

| 단계 | Task | 목표 | 종료 기준 |
|---|---|---|---|
| M0 | `FIT-P0-00` | 현재 구현과 격차 조사 | Plan·Design·Roadmap 준비 |
| M1 | `FIT-P0-01` | 질량 SSOT와 적용 정책 잠금 | Blocking 결정 완료 |
| M2 | `FIT-P0-02` | FittingData·공용 타입 | Done — Build·DataContract PASS |
| M3 | `FIT-P0-03` | 호환 검증과 결정론적 Snapshot | Done — Official Build·Automation PASS |
| M4 | `FIT-P0-04` | 출격 초기 구성 적용 | Done — Weapon·Defense 원자 적용·Rollback PASS |
| M5 | `FIT-P0-05` | 총중량과 기동성 연결 | 실제 Chaos 정량 baseline 확보 + USER 주행감 판정 |
| M6 | `FIT-P0-06` | ViewData와 Debug | 동일 Snapshot 표시 PASS |
| M7 | `FIT-P0-07` | 보호형 테스트 자산·PIE | 공식 Fixture + 정량 계측 Technical PASS / USER 비교 Pending |
| M8 | `FIT-P0-08` | 통합 회귀와 승격 | Build·Automation·User PIE PASS |

---

## 5. M0 — 조사와 문서 준비

### 상태

```text
Done / 2026-07-31
```

### 확인한 현재 기반

```text
- HardpointSlot과 MountProfile 분리
- EquipmentPresetData 단일 장비 조합
- TurretMountData.TurretMountWeightKg 데이터 존재·Runtime 미적용
- WeaponData WeaponMass 부재
- VehicleData BaseMass·GrossMass 부재
- Ammo Runtime 미구현 / CF-FQ-031 Ready
- VehicleDefense Runtime 적용 / DefenseMass 부재
- Chaos 피팅 질량 적용 경로 부재
```

### 산출물

```text
VehicleFittingPlan.md
VehicleFittingDesign.md
VehicleFittingRoadmap.md
CombatPlan/13_Fitting.md 갱신
FeatureQueue CF-FQ-034 등록
```

### 제한

승인된 신규 자산 조회가 자산 로드 전에 실패했으므로 신규 에셋 검증 PASS를 기록하지 않는다.

---

## 6. M1 — Decision Lock

### 상태

```text
Done / 2026-08-01
```

### 포함

```text
FIT-P0-01
```

### 완료 결과

```text
- FIT-D-001~014 Decision Locked
- VehicleData.BaseVehicleMassKg와 MaximumGrossMassKg를 피팅 SSOT로 확정
- WeaponData.WeaponMassKg와 VehicleDefenseData.DefenseMassKg 소유권 확정
- FittingData·FittingComp·Snapshot 구조 확정
- Chaos VehicleMovement 공식 Mass 경로 초기화 전 1회 적용 확정
- PhysicsAsset 수정·BodyInstance 단독 Override 금지
- 실제 물리 질량 우선·Mobility Adapter 기본 비활성 확정
- 출격 고정 탄약 질량, Mount Fallback, Defense 3상태 확정
- 전용 DA_VehicleFitting_TestSUV 보호형 테스트 방향 확정
```

### 목표

코드 작성 전에 질량·탑재 한도·런타임 적용의 단일 기준을 확정한다.

### 결정 목록

```text
- BaseVehicleMass SSOT
- MaximumGrossMass 또는 MaximumPayloadMass
- WeaponMass 소유 위치
- DefenseMass 소유 위치
- FittingData 저장 단위
- FittingComp Runtime 소유권
- Missing MountSelection 정책
- 실제 Chaos Mass 적용 API
- Mobility 보정 공식
- Ammo 소비 후 질량 갱신 정책
- 첫 Test VehicleData와 피팅 조합
- UI 책임 경계
```

### 필수 조사

```text
- 기준 차량 PhysicsAsset·물리 루트 질량
- 현재 Chaos Vehicle이 참조하는 질량
- 런타임 Mass Override 후 재초기화 필요성
- 현재 Movement 튜닝의 기준 질량
- 기준 VehicleData의 실제 MountProfile·EquipmentPreset 연결
```

### 종료 기준

```text
- FIT-D-001~014에서 P0 Blocking 항목 결정: PASS
- 결정 내용을 Plan·Design에 반영: PASS
- M2 필드·파일 목록 확정: PASS
- 기존 dirty 보호 경계 확정: PASS
- 정확한 Engine Mass API·PhysicsAsset 수치: M5 사전 Runtime Mass Gate
```

### 중단 조건

```text
- 실제 차량 질량 소유자를 확인할 수 없음
- 질량 변경이 현재 Chaos Vehicle 초기화를 깨뜨림
- CF-FQ-031 타입과 중복되는 Ammo 구조가 필요함
- 사용자 결정 없이 과적 허용 정책을 확정해야 함
```

---

## 7. M2 — Fitting Data Contract

### 상태

```text
Done / Source Applied / Build·Automation PASS / 2026-08-01
```

### Gate 범위 정정

M2는 Chaos Vehicle이나 VehicleMesh 물리 상태를 호출하지 않는 데이터 계약 단계다. 따라서 Engine Mass API·PhysicsAsset 증거는 M2 차단 조건이 아니며 실제 질량 적용 단계인 M5의 Runtime Mass Gate로 이동한다.

### 포함

```text
FIT-P0-02
```

### 구현 파일

```text
CFFittingTypes.h
CFVehicleFittingData.h
CFVehicleFittingData.cpp
CFFittingContractTests.cpp
```

### 구현 결과

```text
- UCFVehicleFittingData
- Mount·Defense 선택 타입
- Validation State·Severity·IssueCode
- FCFVehicleFittingSnapshot 공용 구조
- VehicleData BaseVehicleMassKg / MaximumGrossMassKg
- WeaponData WeaponMassKg와 안전 Getter
- VehicleDefenseData DefenseMassKg와 안전 Getter·DataValidation
- 기존 에셋 호환용 0kg 기본값
- AmmoMassKg 예약 슬롯, Ammo 선택 타입은 CF-FQ-031까지 보류
```

### 검증 결과

```text
- UHT / 공식 Editor Build: PASS
- CarFight.Fitting.FIT_P0_02.DataContract: Success
- Automation Warnings / Errors: 0 / 0
- 전체 CarFight 필수 전투 회귀: 15/15 Success
- Blueprint·DataAsset·Map 변경: 없음
- VehiclePawn·Chaos 질량 적용 변경: 없음
```

### 다음 단계

```text
M3 Validation·Mass Snapshot
- VehicleData MountProfiles / HardpointSlots 해석
- EquipmentPreset·Weapon 호환 검증
- Defense 3상태 해석
- Base·Mount·Weapon·Defense 질량 합산
- GrossMass 한도와 결정론적 ValidationIssues
- Pawn 없는 Automation
```

---

## 8. M3 — Validation과 Mass Snapshot

### 상태

```text
Done / Source Applied / Official Editor Build PASS / Automation PASS / 2026-08-01
```

### 포함

```text
FIT-P0-03
```

### 구현 결과

```text
- UCFVehicleFittingData.BuildFittingSnapshot 순수 함수
- VehicleData.MountProfiles 순서 기반 ResolvedMounts
- MountSelections 배열 순서 독립성
- Duplicate·Unknown·Missing MountSelection 검증
- HardpointSlot 참조 검증
- EquipmentPreset 완성·MountType·Size 검증
- Weapon MountType·Size 검증
- MissingMountSelectionPolicy 3종
- ExplicitEmpty / MissingPolicyEmpty / VehicleDefault / FittingOverride 구분
- Defense UseVehicleDefault / ExplicitNone / Override 3상태
- Base·Mount·Weapon·Defense 질량과 GrossMass 검증
- 결정론적 ValidationIssues와 ValidationState
```

### Automation 결과

```text
CarFight.Fitting.FIT_P0_03.Compatibility: Success / Warnings 0 / Errors 0
CarFight.Fitting.FIT_P0_03.MassSnapshot: Success / Warnings 0 / Errors 0
전체 Automation: 27 Success / Failed 0
필수 회귀: 17/17 Success
```

### 공식 빌드와 후속 검증

```text
- 신규 피팅 파일 UHT·C++ 컴파일: PASS
- UnrealEditor-CarFight_Re.dll 링크: PASS
- 후속 CF-FQ-032 공식 Editor Build 4f2daa07187a41aea7c400386010b2a0: Exit Code 0
- FIT-P0-04 최종 공식 Editor Build 52b5b5572e2a4c739771dbd711a284d3: Exit Code 0
- 최신 전체 CarFight Automation: 40/40 Success
- FIT-P0-03 Compatibility·MassSnapshot: Success / Warning 0 / Error 0
```

### 종료 기준 판정

```text
- UI와 Runtime이 사용할 동일 Snapshot 계약: PASS
- 질량 중복 합산 없음: PASS
- 오류 시 Snapshot.IsValid=false: PASS
- 결정론적 ResolvedMounts·ValidationIssues: PASS
- Pawn 없는 Automation: PASS
- 공식 Editor Build: PASS
```

과거 NetCore DLL 점유는 해소된 외부 차단 이력이며 M3은 Done이다.

---

## 9. M4 — Sortie Apply Runtime

### 상태

```text
Done / Source Applied / Official Editor Build PASS / Automation PASS / 2026-08-01
```

### 포함

```text
FIT-P0-04
```

### 구현 파일

```text
CFVehicleFittingComp.h
CFVehicleFittingComp.cpp
CFFittingRuntimeTests.cpp
CFVehiclePawn.h / .cpp
CFVehicleWeaponComp.h / .cpp
Tools/RunCombatRuntimeTests.ps1
```

### 연결 결과

```text
ACFVehiclePawn
→ 선택적 VehicleFittingData
→ UCFVehicleFittingComp Prepare·Commit
→ UCFVehicleWeaponComp Snapshot EquipmentPresetData Override
→ UCFVehicleDefenseComp 최종 DefenseData 적용
→ 최종 Weapon Runtime에 Launcher 연결
```

`UCFVehicleAmmoComp`, Runtime Mass와 Inventory Adapter는 이번 단계에서 연결하지 않았다.

### 원자 적용 규칙

```text
- Snapshot Invalid면 Weapon·Defense Adapter 호출 없음
- Valid Snapshot에서 Weapon·Defense 입력을 하나의 Prepared 상태로 생성
- Weapon → Defense 순서 Commit
- 부분 실패 시 직전 Applied 또는 VehicleData Legacy 입력으로 전체 복원
- 모두 성공한 경우에만 AppliedFittingSnapshot 교체
- 명시 Rollback은 Prepared 상태만 폐기
- EndPlay·Reset에서 Transient 상태 정리
```

### Automation

```text
CarFight.Fitting.FIT_P0_04.RuntimeApply: Success
CarFight.Fitting.FIT_P0_04.AtomicBoundary: Success
전체 CarFight: 40/40 Success
필수 회귀: 19/19 Success
```

### 공식 빌드

```text
최종 Build Job 52b5b5572e2a4c739771dbd711a284d3
CarFight_ReEditor Win64 Development
Exit Code 0 / Result Succeeded
```

### 종료 기준

```text
- FittingData 없음 Legacy PASS
- Valid Fitting 적용 PASS
- Invalid Fitting 부분 적용 없음 PASS
- Commit 실패 전체 Rollback PASS
- 재초기화 중 중복 상태 없음 PASS
- EndPlay·Reset 안전 PASS
```

---

## 9A. FFIT-M0~M5 — Field Fitting Action Track

### FFIT-M0 — Contract

```text
Done — Documentation
```

확정 규칙:

```text
- 비전투 상태
- 모든 관련 쿨타임 종료
- 충돌하는 전투·재장전·수리·다른 시간 액션 없음
- 데이터 기반 정지 속도와 정지 유지 시간 만족
- Equip은 호환되는 빈 슬롯에만 가능
- Replace는 Unequip 완료 후 별도 Equip
- 시간 액션 중 Reservation만 생성
- 완료 순간 Runtime과 Inventory 원자 Commit
- 이동·전투·피해·상태 변경·예약 손실·화면 종료 시 취소
```

### FFIT-M1 — Permission and Blocker Query

```text
Combat / Drive / Weapon / Launcher / Ammo / Defense Provider
→ 구조화된 BlockReason 수집
→ Fitting Action Permission 생성
```

### FFIT-M2 — Timed Action and Reservation

선행:

```text
CF-FQ-035 INV-P0-03
```

결과:

```text
Preparing → InProgress → Completing → Completed
각 진행 상태에서 Cancelled / Failed 가능
Item·Mount Slot·Destination Capacity 예약
```

### FFIT-M3 — Atomic Equip and Unequip

선행:

```text
FIT-P0-04 Runtime Apply Prepare·Commit·Rollback
CF-FQ-035 INV-P0-04 Fitting Adapter
```

종료 기준:

```text
- 완료 전 실제 Item 위치·장비·Snapshot 무변경
- 성공 시 Cargo ↔ Mounted 소유권과 Applied Snapshot 동시 확정
- 실패·취소 시 복제·유실·부분 적용 없음
```

### FFIT-M4 — Field Mass Reapply

선행:

```text
FIT-P0-05 Runtime Mass Gate
```

정지 차량의 새 TotalVehicleMassKg 적용과 필요 시 Physics State·Simulation 재생성, Transform·속도·Runtime Ready 보존을 검증한다.

### FFIT-M5 — UI and PIE

```text
CF-FQ-032 Screen Layer
→ 피팅 화면
Panel Layer
→ 장비·슬롯 상세
Modal Layer
→ 시간 액션 확인 또는 위험 경고
System Layer
→ 진행·취소·실패 사유
```

Field Fitting 화면은 World Pause를 사용하지 않는다.

---

## 10. M5 — Vehicle Mass와 Mobility

### 포함

```text
FIT-P0-05
```

### 목표

피팅 총중량이 실제 물리 또는 제한된 보정 계층을 통해 체감 차이를 만든다.

### Runtime Mass Gate 결과

```text
Gate: Locked / 2026-08-02
Initial Sortie Source: Code Complete
Official Editor Build: PASS
Pawn-less Automation: PASS
Actual Physics PIE: Pending
Mobility Measurement: Pending
```

확정 구조:

```text
Snapshot.TotalVehicleMassKg
→ PreRegisterAllComponents / Super 이전
→ UChaosVehicleMovementComponent::Mass 1회 기록
→ Chaos 초기 Physics State·Vehicle Simulation 생성
→ BeginPlay VehicleMesh.GetMass·GetPhysicsAsset 검증
→ 성공 시 같은 Cached Snapshot의 Weapon·Defense Commit
```

재생성 정책:

```text
초기 출격 물리 등록 전 적용
→ Physics State·Vehicle Simulation 재생성 없음

같은 질량 재초기화
→ Verify Only

물리 생성 뒤 다른 질량
→ M5 P0에서 거부
→ Field Mass Reapply FFIT-M4로 이관
```

금지:

```text
- 프로젝트 코드의 SetMassOverrideInKg 직접 호출
- VehicleMesh Body-only Override
- PhysicsAsset·MassScale 저장 수정
- BeginPlay 이후 Hot Apply
- 자동 RecreatePhysicsState 우회
```

검증 기준:

```text
Target = Cached Snapshot.TotalVehicleMassKg
Configured = VehicleMovementComponent.Mass
Actual = VehicleMesh.GetMass()
Tolerance = Max(1kg, Target × 1%)
Physics State·Simulate Physics·PhysicsAsset 유효 필수
```

현재 BP 컴포넌트에 명시적 PhysicsAsset Override는 발견되지 않았다. read-only dump에서 SKM_SportsCar와 PA_SportsCar 자산 존재는 확인했지만 SkeletalMesh→PhysicsAsset 직접 연결과 계산 질량은 AssetDump 지원 범위 밖이므로 Runtime 검증으로 확정한다.

### 구현 순서와 상태

```text
1. PreRegister Mass Prepare 상태와 Cached Snapshot 계약 추가 — Done
2. UE 5.8 UChaosVehicleMovementComponent::Mass public 접근 공식 Build 증명 — Done
3. 초기 Target Mass 1회 기록 — Done
4. BeginPlay 실제 VehicleMesh Mass 검증 코드 — Done
5. 검증 성공 뒤 FIT-P0-04 Commit과 Applied Mass 상태 확정 — Done
6. Pawn 없는 상태·Legacy·재초기화 Automation — Done
7. 실제 Physics State PIE 질량 검증 — Pending
8. 기준 주행 회귀 측정 — Pending
9. Light·Default·Heavy 측정 — Pending
10. 물리 질량만으로 부족한 축에만 Mobility Scalar 검토 — Pending
11. 중복 효과 검증 — Pending
```

구현·검증 증거:

```text
Official Build dce20226611944f9b323f94b573d4a74: Exit Code 0
Final Build b7a4e52743c34fa2844ebe94925c6a0d: Exit Code 0
CarFight.Fitting.FIT_P0_05.InitialMass: Success
전체 CarFight Automation: 41/41 Success
필수 회귀: 20/20 Success
```

첫 Automation은 13.6kg float 단언 정밀도 때문에 새 테스트만 실패했으며 0.001kg 비교 허용치를 명시한 후 최종 전체 회귀가 통과했다.

### 측정 항목

```text
- 정지 → 기준 속도 도달 시간
- 기준 속도 → 정지 제동거리
- 동일 속도·동일 입력의 Yaw 변화
- 최고속도 장기 변화 관찰
- 차체 안정성·전복·Wheel Load 이상
```

### 종료 기준

```text
- Default Fitting 기준 회귀 허용 범위 통과
- Light가 Heavy보다 빠른 가속·반응
- Heavy가 명확히 긴 제동거리 또는 낮은 제동 반응
- 비정상 전복·휠 침하·진동 없음
- Tick별 설정 재작성 없음
```

---

## 11. M6 — ViewData와 Debug

### 포함

```text
FIT-P0-06
```

### 현재 상태

```text
C++ ViewData: Technical Done
Blueprint Contract: PASS
UI Widget/Screen: Not Started
16:9·32:9 실제 가독성: USER Pending
```

### 구현 결과

```text
- FCFVehicleFittingViewData
- FCFFittingMountViewRow
- FCFFittingDefenseViewRow
- FCFFittingAmmoViewRow
- FCFFittingMassViewRow
- FCFFittingMobilityViewData
- FCFFittingViewBuilder
```

Builder는 `FCFVehicleFittingSnapshot`의 Mount·Defense·Ammo·Mass·Validation 결과를 다시 계산하지 않고 순서와 값을 그대로 투영한다. 실제 출격 Ammo Count와 MaximumLoadable을 분리하고, 질량 Breakdown은 Snapshot의 Base/Equipment/Ammo/Defense/Payload/Total/MaximumGross 7행을 그대로 표시한다.

Mobility는 현재 추가 Adapter가 없으므로 `PhysicalMassOnly`다.

```text
MassRatio = TotalVehicleMassKg / BaseVehicleMassKg
AccelerationScale = 1.0
BrakingScale = 1.0
SteeringResponseScale = 1.0
bAdditionalAdapterApplied = false
```

1.0은 실제 질량 효과가 없다는 뜻이 아니라 추가 인위적 Mobility Scalar가 없다는 뜻이다.

### 기술 증거

```text
Build: ad3452d3add74785bbdb41c667dce728 / PASS
FIT-P0-06: 0c7ee328d7ef4f659d66d857fecdab2a / 1/1 Success
Fitting: e5140ca056444ea694abb4f73861c935 / 22/22 Success
Inventory: c76221a75d9447dab1b03cc274d43a35 / 12/12 Success
Full CarFight: 59aae9f0ac204e13a957dbbfe8cdbaec / 84/84 Success
Full SHA-256: 4e49080b401ada72e243e8bd1a98e58e9bdd9d3af4471a03b78f7d7ca277c139
```

### 남은 종료 Gate

```text
- 실제 Widget에서 UI 재계산 없음 확인
- 16:9·32:9 기본 가독성 USER 확인
- Progress/Blocker/Validation 표시를 FFIT-M5 Field UI와 통합
```

C++ ViewData·Blueprint Contract는 Technical Done으로 닫지만 UI 가독성을 USER PASS로 대체하지 않는다.

---

## 12. M7 — Test Assets와 사용자 PIE 준비

### 포함

```text
FIT-P0-07
```

### 목표

원본 에셋을 보호한 테스트 조합을 준비한다.

### 보호 원칙

```text
- 원본 DA_TestSUV·DA_TestSedan 직접 변경 금지
- 원본 Launcher·Projectile·Defense 자산 직접 변경 금지
- 별도 Test VehicleData 또는 FittingData 사용
- TestMap 원본 비저장 또는 복제 맵 사용
```

### 필수 피팅

```text
Default
Light
Heavy
Invalid Mount
Overweight
```

### 종료 기준

```text
- AssetDump 또는 안전한 에셋 검증 PASS
- 참조 체인 명확
- 원본 SHA 또는 Apply Report 보호
- 사용자 PIE 절차 문서화
```

---

## 13. M8 — Integrated Verification

### 포함

```text
FIT-P0-08
```

### 자동 회귀

```text
CarFight.Fitting.*
CarFight.Ammo.*
CarFight.Damage.*
CarFight.Launcher.*
CarFight.Projectile.*
CarFight.Vehicle.*
```

실제 테스트 이름은 구현 시 현재 Automation 목록과 충돌하지 않게 확정한다.

### 사용자 PIE 핵심 행렬

```text
1. Default Fitting Legacy 비교
2. Light / Default / Heavy 질량 확인
3. 가속 비교
4. 제동거리 비교
5. 선회 반응 비교
6. 무기 장비 실제 적용
7. 비호환 장비 거부
8. Defense 적용
9. Ammo 적재·질량 일치
10. 발사·Launcher·Projectile·Damage 회귀
```

### 승격 조건

```text
- 공식 Editor Build PASS
- Fitting Automation 전체 PASS
- 의존 시스템 회귀 PASS
- 사용자 PIE PASS
- VehicleFitting.md Current System 작성
- FeatureQueue CF-FQ-034 Done
```

---

## 14. P1 이후 확장

```text
- 연료 탱크와 연료 질량
- 배터리·발전기·전력 예산
- Utility Slot과 연막·플레어·수리·냉각
- 방향별 개별 Armor Plate
- 장착 위치별 Center of Mass 변화
- 탄약 소모·보급에 따른 배치 질량 갱신
- 무기 그룹 구성
- 차고 UX와 비교 화면
- PlayerStorage·NearbyService·WorldLoot 컨테이너 확장
- 경제·구매·판매
- SaveGame 인벤토리·피팅 저장
- 해금·연구·제작
- AI 피팅 생성
- 네트워크 권한·복제
```

필드 피팅에 필요한 최소 Item Instance·VehicleCargo·Reservation·Atomic Transfer는 CF-FQ-035 P0 Foundation으로 선행 구현한다. 상점·루팅·경제·SaveGame 데이터는 미리 넣지 않는다.

---

## 15. 중단 조건

```text
- 현재 VehicleData와 실제 Physics Body 질량의 SSOT를 정할 수 없음
- Chaos 질량 적용이 VehicleRuntime Ready 또는 WheelSync를 불안정하게 만듦
- CF-FQ-031 Ammo 타입과 피팅 타입이 중복됨
- CF-FQ-033 Defense 데이터 계약이 사용자 PIE 결과로 변경됨
- CF-FQ-029 dirty 핵심 함수와 안전한 최소 통합이 불가능함
- UI Root 없이 ViewData보다 화면 구현을 먼저 요구함
- 사용자 결정 없이 과적·Fallback·실시간 질량 갱신 정책을 확정해야 함
```

중단 시 범위를 임의 확대하지 않고 대표 Plan에 차단 사유와 필요한 결정만 기록한다.

---

## 16. Changelog

### v0.15.0 - 2026-08-17

- `FIT-P0-07C Quantitative Mobility Measurement`을 Technical Complete로 반영했다.
- 같은 `DA_VehicleDefense_TestSUV` 플랫폼과 기존 공식 Fixture를 유지하고 Light/Default/Heavy를 각각 fresh PIE lifetime에서 계측해 cross-fixture Chaos 상태 오염을 제거했다.
- 공식 Build `bb04d56e0cd24647b2ef6fcbc7f2bd77` PASS와 process `63bbeaf0202041b79dac8f32f5447923` Success / Metric 3/3을 기록했다.
- 0→30 시간은 2.349081 / 2.252550 / 2.248457초, 제동거리는 3.571402 / 3.657704 / 3.662367m, 2초 coast-steering 누적 Yaw는 16.549116 / 20.418766 / 20.597601도로 확보했다.
- 제동은 질량 증가에 따라 소폭 증가했지만 가속과 coast-steering은 단순 질량 순서의 단조 결과가 아니며 이를 PASS/FAIL 또는 자동 튜닝 조건으로 만들지 않는다.
- `PhysicalMassOnly`를 유지하고 다음 Gate를 `FIT-P0-07D USER Driving Feel Comparison`으로 이동했다. USER 주행감·Field UI·16:9/32:9 시각 Gate는 Pending이다.

Migration: `FIT-P0-07A~07C`와 기존 Fitting 23/23을 반복하지 않는다. 다음은 `VehicleFittingPlan.md v0.17.0 / FIT-P0-07D`이며 같은 공식 Fixture를 사용해 사용자 주행감만 비교한다.

### v0.14.0 - 2026-08-17

- M7의 공식 Light/Default/Heavy Fixture 준비를 Technical Complete로 전환하고 같은 `DA_VehicleDefense_TestSUV` 플랫폼의 1000kg / 1570kg / 1600kg 기준을 고정했다.
- HeavyFinite 저장 payload의 Mount 350kg + Weapon 120kg + Ammo 2kg×15와 VehicleDefault Defense 100kg를 사용해 임의 질량 없이 Heavy 1600kg를 성립시켰다.
- persisted DisplayName·참조 AssetDump와 `CarFight.Fitting.FIT_P0_07.OfficialMobilityFixtures` post-label 1/1 PASS를 보호 증거로 연결했다.
- CF-FQ-034를 현재 Active로 동기화하고 M5의 다음 기술 작업을 `FIT-P0-07C Quantitative Mobility Measurement`로 이동했다.
- USER 주행감·Field UI·16:9/32:9 시각 Gate는 완료로 추정하지 않는다.

Migration: M7 공식 Fixture 제작을 반복하지 않는다. 다음 작업은 `VehicleFittingPlan.md v0.16.0 / FIT-P0-07C`이며 공식 세 Fixture와 저장된 질량 Source를 그대로 사용한다.

### v0.13.0 - 2026-08-15

- M6 `FIT-P0-06 ViewData·Debug`의 C++ ViewData·Blueprint Contract를 Technical Done으로 전환했다.
- Snapshot의 Mount·Defense·Ammo·Mass·Validation 결과를 재계산하지 않는 read-only Builder와 PhysicalMassOnly Mobility Preview를 구현했다.
- Build `ad3452d3add74785bbdb41c667dce728`, FIT-P0-06 1/1, Fitting 22/22, Inventory 12/12, Full CarFight 84/84 Success를 기록했다.
- 실제 Widget이 아직 없어 16:9·32:9 가독성은 Pending으로 보존했다.
- 저장소에 공인 Light·Default·Heavy 전용 피팅 Fixture가 없으므로 임의 질량값으로 Mobility 종료 기준을 만들지 않는다.
- CF-FQ-034/035의 현재 원격 기술 체크포인트는 완료됐으며 남은 Fitting 범위는 정식 테스트 자산·Mobility USER·Field UI/PIE다.

### v0.12.0 - 2026-08-15

- FFIT-M4 Field Mass Reapply를 Technical Done으로 전환했다.
- mass-changing 후보의 Weapon·Defense Runtime, Chaos Movement Mass와 Inventory Commit을 하나의 completion transaction에 포함하고 실패 시 이전 Runtime·질량을 함께 보상 복원한다.
- 내부 Runtime Rollback 또는 Mass 복구 실패는 `RecoveryFailed`로 별도 분류한다.
- 실제 M_VehicleDefensePIE에서 Configured/Actual Mass +50kg 반영과 원복, Physics State·Transform·Runtime Ready 보존을 검증했다.
- 최종 Build PASS, FFIT-P0-04 4/4, Fitting 21/21, Inventory 12/12, Full CarFight 83/83 Success를 기록했다.
- M6 `FIT-P0-06 ViewData·Debug`를 current next로 이동하고 FFIT-M5 UI·PIE는 M6와 사용자 UI 확인 가능 시점 뒤로 유지한다.

### v0.11.0 - 2026-08-15

- FFIT-M2 Timed Action·Reservation을 Done으로 전환했다. Start에서 Reservation만 만들고 InProgress 동안 Item 위치를 유지하며 취소 시 Rollback, 완료 시 Prepared Transaction을 보존하는 수명을 검증했다.
- FFIT-M3 Atomic Equip·Unequip formal handoff를 Technical Done으로 전환했다. TimedAction Completing이 같은 Prepared Transaction을 기존 Coordinator에 넘겨 Equip·Unequip을 원자 Commit한다.
- P0-02 최종 Full CarFight 76/76, P0-03 최종 Full CarFight 79/79 Success를 확인했다.
- mass-changing Field Apply는 여전히 미지원이며 `FFIT-M4 Field Mass Reapply`를 next로 이동했다.

### v0.10.0 - 2026-08-15

- FFIT-M1 Permission·Blocker Query를 Done으로 전환했다.
- Combat·Drive·VehicleRuntime·Weapon·Launcher·Ammo·Defense·Action·Inventory 상태를 구조화 BlockReason으로 합성하는 순수 Query를 구현했다.
- Reservation과 Timed Action 상태 변경은 M2로 유지하고 M1은 읽기 전용 Permission 결과만 생성한다.
- Build `0a42d6369a194a07b1dba2fcc1e98286`, FFIT-P0-01 2/2, Fitting 11/11, Inventory 12/12, Full CarFight 73/73 Success를 기록했다.
- formal next를 `FFIT-M2 Timed Action·Reservation`으로 이동했다.

### v0.9.0 - 2026-08-14

- `CF-FQ-035 M6 Coordinator Foundation`의 Technical PASS와 `UCFVehicleFittingComp v1.3.0` 보상 Checkpoint를 Field Fitting dependency graph에 반영했다.
- Coordinator 기반은 same-mass completion과 failure compensation을 검증했지만 formal `FFIT-M3 Atomic Equip/Unequip` 완료로 승격하지 않는다.
- Inventory 선행 M3~M5가 완료됐으므로 `FFIT-M1 Permission·Blocker Query`를 current next formal step으로 전환하고 stale Inventory Blocked 표기를 제거했다.
- `FFIT-M4 Field Mass Reapply`는 여전히 미구현이며 mass-changing 후보는 현재 Runtime Adapter에서 `RuntimeMassReapplyUnsupported`로 거부한다.
- 최종 Build `e2ef556b64484a09ba8b3544c62344e3`, M6 3/3, Inventory 12/12, Full CarFight 71/71 Success를 기술 증거로 기록했다.

### v0.8.0 - 2026-08-02

```text
- M5를 In Progress로 전환하고 Initial Sortie Runtime Mass Adapter 구현을 완료했다.
- UCFVehicleFittingComp Initial Mass 상태·Cached Snapshot·Legacy fallback·Verify Only·ReapplyRejected를 구현했다.
- ACFVehiclePawn PreRegisterAllComponents의 Super 호출 전 Movement Mass 설정과 BeginPlay 실제 물리 검증을 구현했다.
- 검증 성공 뒤 같은 Cached Snapshot으로 FIT-P0-04 Weapon·Defense Commit을 수행한다.
- UE 5.8 Mass public 접근과 PreRegister 수명은 공식 Editor Build 두 건에서 Exit Code 0으로 증명됐다.
- CarFight.Fitting.FIT_P0_05.InitialMass와 최종 전체 41/41, 필수 20/20 Success를 기록했다.
- SetMassOverrideInKg, Body-only Override, PhysicsAsset·MassScale 수정, Hot Recreate와 Mobility Scalar는 구현하지 않았다.
- 실제 Physics PIE와 Light·Default·Heavy 측정이 남아 있어 M5 Done 판정은 보류했다.
- Blueprint, DataAsset, PhysicsAsset, Map, SaveGame, UI, Ammo와 Inventory Adapter는 수정하지 않았다.
- CF-FQ-032 Active와 Inventory INV-P0-00~03 / INV-P0-04 Not Started를 유지했다.
```

### v0.7.0 - 2026-08-02

```text
- M5 Runtime Mass Gate를 Locked / Implementation Ready로 전환했다.
- 초기 출격 질량을 PreRegisterAllComponents의 Super 호출 전 Movement Mass에 1회 기록하도록 순서를 확정했다.
- Snapshot Total Mass, Movement Mass와 VehicleMesh 실제 Mass의 SSOT·Mirror·Evidence 역할을 분리했다.
- 초기 적용은 Physics State·Vehicle Simulation 재생성이 없고 생성 후 다른 질량은 M5 P0에서 거부하도록 했다.
- SetMassOverrideInKg 직접 호출, Body-only Override, PhysicsAsset·MassScale 수정과 자동 Hot Recreate를 금지했다.
- Max(1kg, Target 1%) 실제 질량 허용 오차와 Physics State·PhysicsAsset 검증 조건을 추가했다.
- 같은 Snapshot 재초기화, 다른 질량 거부, Legacy 유지와 EndPlay 캐시 정리 정책을 잠갔다.
- BP_CFVehiclePawn read-only dump 3/3, SportsCar 폴더 read-only dump 24/24 Success를 기록했다.
- Source, Blueprint, DataAsset, PhysicsAsset, Map과 SaveGame은 수정하지 않았다.
- CF-FQ-032 Active와 Inventory INV-P0-00~03 / INV-P0-04 Not Started를 유지했다.
```

### v0.6.0 - 2026-08-01

```text
- M4 Sortie Apply Runtime을 Done으로 전환했다.
- UCFVehicleFittingComp의 Legacy·Snapshot Prepare, Weapon·Defense Commit, 실패 Rollback과 AppliedFittingSnapshot 수명을 구현했다.
- VehicleWeaponComp Snapshot EquipmentPresetData Override와 VehicleData Legacy Fallback을 구현했다.
- VehiclePawn 초기화 순서를 Fitting Commit 뒤 터렛 시각 적용·Launcher 연결 순서로 변경했다.
- RuntimeApply·AtomicBoundary Pawn 없는 Automation 2/2 Success를 기록했다.
- 최종 공식 Editor Build 52b5b5572e2a4c739771dbd711a284d3 / Exit Code 0을 기록했다.
- 전체 CarFight 40/40, 필수 회귀 19/19 Success를 기록했다.
- M5를 Runtime Mass Gate Ready로 이동하고 Ammo·Inventory Adapter·Field Fitting·UI·Unreal Asset은 변경하지 않았다.
- CF-FQ-032 Active와 Inventory INV-P0-00~03을 보호했다.
```

### v0.5.0 - 2026-08-01

```text
- CF-FQ-035 Inventory Foundation을 별도 선행 기능으로 연결했다.
- 필드 피팅을 FFIT-M0~M5 하위 트랙으로 추가했다.
- 비전투·쿨타임 종료·차량 정지·예약 성공 Gate와 빈 슬롯 전용 Equip 규칙을 반영했다.
- 시간 액션, 취소, Reservation, Atomic Equip·Unequip과 Field Mass Reapply 단계를 분리했다.
- FIT-P0-04 Sortie Apply는 Field Action이 재사용 가능한 Prepare·Commit·Rollback API를 제공하도록 종료 기준을 확장했다.
- 후속 공식 Editor Build와 Automation 32/32 PASS를 근거로 M3을 Done으로 전환했다.
- Source와 Unreal Asset은 수정하지 않았다.
```

### v0.4.0 - 2026-08-01

```text
- M3 Pawn 없는 Compatibility Validation과 Mass Snapshot을 구현했다.
- MountProfile·Hardpoint·장비·무기·방어 선택과 질량·GrossMass 검증을 결정론적으로 해석한다.
- Compatibility와 MassSnapshot Automation이 Success했고 전체 27개 Automation과 필수 회귀 17/17을 통과했다.
- 신규 피팅 파일 컴파일과 CarFight 모듈 링크는 통과했다.
- 기존 UnrealEditor-Cmd.exe 두 프로세스의 NetCore DLL 점유로 공식 Editor 전체 타깃은 Blocked다.
- M3을 Code Complete로 유지하고 M4는 Not Started로 보호했다.
- M5 Runtime Mass Gate는 변경하지 않았다.
```

### v0.3.0 - 2026-08-01

```text
- M2 데이터 계약과 M5 런타임 질량 적용의 Gate를 분리했다.
- M2 공용 피팅 타입, VehicleFittingData와 Base·Weapon·Defense 질량 필드를 구현했다.
- 공식 Editor Build와 Fitting DataContract Automation을 PASS했다.
- 전체 CarFight 필수 전투 회귀 15/15 Success를 확인했다.
- M2를 Done, M3을 Ready로 전환했다.
- Engine Mass API·PhysicsAsset 증거는 M5 Runtime Mass Gate로 이동했다.
```

### v0.2.3 - 2026-08-01

```text
- Roadmap에서 외부 도구 작업 ID, 설정, 런타임 상태와 복구 절차를 제거했다.
- M2 재개 조건을 엔진 API와 자산 기준 증거 확보로 한정했다.
- M2 Blocked와 현재 Active CF-FQ-029는 변경하지 않았다.
```

### v0.2.2 - 2026-08-01

```text
- M2 Gate 해제를 위해 승인된 엔진·자산 읽기 경로를 재확인했다.
- 필요한 증거를 확보하지 못해 M2 Blocked를 유지했다.
```

### v0.2.1 - 2026-08-01

```text
- M2 Pre-Implementation Gate를 수행하고 Blocked로 전환했다.
- Gate 해제 전 C++ Data Contract를 구현하지 않는 순서를 확정했다.
- M0~M1 Done과 현재 Active CF-FQ-029는 변경하지 않았다.
```

### v0.2.0 - 2026-08-01

```text
- FIT-P0-01 / M1 Decision Lock을 Done으로 전환했다.
- FIT-D-001~014의 질량 SSOT, 소유권, Chaos 적용, Mobility, Fallback, Defense, Test Asset과 UI 결정을 확정했다.
- M2 Fitting Data Contract를 Ready로 전환했다.
- 정확한 Engine Mass API와 PhysicsAsset 수치 확인을 M2 사전 읽기 전용 Gate로 유지했다.
- Source와 Unreal Asset은 수정하지 않았다.
```

### v0.1.0 - 2026-07-31

```text
- FIT-P0-00~08을 M0~M8 단계로 구성했다.
- 조사 → 결정 → 데이터 → Snapshot → 적용 → 질량·기동 → ViewData → 테스트 → 승격 순서를 확정했다.
- Ammo·Defense·UI 선행 관계와 단계별 게이트를 명시했다.
- Source와 Asset은 수정하지 않았다.
```

---

## 17. Migration

- v0.9.0부터 `CF-FQ-035 M6 Coordinator Foundation`은 FFIT completion 하위 계층으로 재사용하되 formal 진행 순서 `FFIT-M1 → M2 → M3 → M4`를 건너뛰지 않는다.
- `FFIT-M1`은 현재 원격 환경에서 바로 착수 가능한 C++/Automation 단계다. Permission 결과를 완료한 뒤에만 Timed Action/Reservation으로 이동한다.
- 현재 same-mass gate는 FFIT-M4 전 안전 경계이며 일반 장착 질량 변화 지원으로 해석하지 않는다.
- `CF-FQ-034` Feature 상태는 Active이며 `FIT-P0-07C Quantitative Mobility Measurement`는 Technical Complete다. 다음 Gate는 `FIT-P0-07D USER Driving Feel Comparison`이며 `CF-FQ-035`는 USER Field UI 체크포인트가 보존된 Paused다.



```text
- v0.8.0부터 M5 단계 1~6은 구현·공식 Build·Pawn 없는 Automation 완료 상태다.
- 실제 Chaos Physics PIE와 공식 Fixture 정량 Mobility 측정은 완료됐다. 남은 M5 판정은 USER 주행감과 그 결과에 따른 PhysicalMassOnly 유지/후속 Adapter 결정이며, 그 전에는 M5 Done 또는 CF-FQ-034 Current로 승격하지 않는다.
- Initial Mass 상태와 Target 캐시는 UCFVehicleFittingComp, 엔진 Mass 적용과 VehicleMesh 물리 조회는 ACFVehiclePawn이 소유한다.
- PreRegister의 Prepared Snapshot을 BeginPlay가 재사용하고 Mass 검증 뒤 FIT-P0-04 Commit을 실행한다.
- 초기 Invalid Snapshot은 Legacy fallback, 같은 Target은 Verify Only, 다른 Target은 ReapplyRejected다.
- UE 5.8 public Mass 접근이 Build로 증명됐으므로 BodyInstance 우회 경로를 만들지 않는다.
- v0.7.0부터 M5 구현은 PreRegister Mass Prepare → Physics 생성 → BeginPlay 검증 → FIT-P0-04 Commit 순서를 따른다.
- PreRegister와 BeginPlay는 별도 Snapshot을 만들지 않고 같은 Cached Snapshot을 공유한다.
- Legacy와 Invalid Snapshot은 Movement Mass를 수정하지 않는다.
- 물리 생성 뒤 다른 질량은 Respawn 또는 FFIT-M4 전까지 거부한다.
- EndPlay은 Mass 상태만 정리하고 Physics 재생성을 수행하지 않는다.
- VehicleMesh.GetMass·GetPhysicsAsset은 Runtime 검증값이며 Design SSOT가 아니다.
- 초기 M5에서 SetMassOverrideInKg, PhysicsAsset 수정과 Body-only Override를 사용하지 않는다.
- UE 5.8 Mass public 심볼 컴파일 실패 시 우회하지 않고 M5를 Blocked로 되돌린다.
- v0.6.0부터 M4는 공식 Editor Build와 전체 Automation을 포함해 Done이다.
- M4의 Prepare·Commit·Rollback은 출격과 후속 Field Fitting Coordinator가 재사용할 Runtime 경계다.
- FittingData 미지정은 VehicleData Legacy 경로, 유효 Snapshot은 AppliedFittingSnapshot 경로로 구분한다.
- 무효 Snapshot과 부분 Commit 실패는 새 Applied 상태를 만들지 않는다.
- M5만 Chaos Runtime Mass를 다루며 M4 결과를 물리 질량 적용 완료로 해석하지 않는다.
- v0.5.0부터 M3은 후속 공식 Editor Build와 Automation PASS를 포함해 Done이다.
- M4 Sortie Apply Runtime은 Inventory Foundation과 독립적으로 시작할 수 있지만 Field Item 소유권 이동을 구현하지 않는다.
- FFIT-M2는 CF-FQ-035 INV-P0-03 전에는 시작하지 않는다.
- FFIT-M3는 INV-P0-04와 FIT-P0-04의 Prepare·Commit·Rollback 계약 전에는 시작하지 않는다.
- FFIT-M4는 Runtime Mass Gate 잠금만으로 시작하지 않으며 M5 초기 적용 구현·검증과 FFIT-M3 완료 뒤 별도 Physics 재생성 계약으로 진행한다.
- 필드 Equip은 Occupied Slot 직접 교체를 지원하지 않고 Unequip과 Equip 두 액션으로 유지한다.
- BuildFittingSnapshot은 Pawn·VehicleMovement를 수정하지 않는 순수 해석이다.
- MountSelections 저장 순서가 달라도 VehicleData.MountProfiles 순서의 동일 Snapshot을 생성한다.
- 선택된 실제 장비와 방어의 0kg 질량은 MissingMassSource로 거부한다.
- Ammo 선택 타입은 CF-FQ-031의 계약을 재사용하고 임시 중복 타입을 만들지 않는다.
- M5 Runtime Mass Gate는 초기 적용 API·수명·검증 기준을 잠갔고 VehicleMesh PhysicsAsset 경로와 실제 질량은 M5 Runtime 검증에서 수집한다.
- M4부터 검증된 Snapshot을 기존 Runtime에 연결하되 별도 사용자 승인 후 시작한다.
- M5는 Runtime Mass Gate를 통과한 뒤 실제 Chaos 질량 효과를 측정하고 필요한 보정만 추가한다.
- M7 이전에는 원본 전투 에셋을 수정하지 않는다.
- 사용자 PIE 전에는 Done 또는 Current System으로 승격하지 않는다.
```
