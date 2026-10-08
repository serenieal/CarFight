# CarFight Vehicle Builder 신규 차량 생성 UX Plan

- 문서 버전: v0.2.1
- 작성일: 2026-09-02
- 최근 갱신일: 2026-09-06
- 문서 상태: Historical / Complete / Archived Path
- Feature: `CF-FQ-042 Vehicle Builder 신규 차량 생성 UX`
- 우선순위: P2
- Current owner 기준: main_game `Document/Systems/Vehicles/VehicleBuilder.md v1.4.1`
- 선행 완료: `CF-FQ-040 Guided Vehicle Builder — Done`
- 현재 Gate: `VBCUX-P0-05 USER Acceptance — USER PASS`
- 다음 Gate: 없음 — `Current System Promotion Complete / Historical + Archived Path / G5 Physical Move Complete`
- Historical placement: `Document/Plan/Archive/VehicleBuilderCreationUX/VehicleBuilderCreationUXPlan.md`
- 역할: CF-FQ-040을 재오픈하지 않고, 다음 차량부터 Guided Builder 안에서 완전 신규 차량을 시작할 수 있도록 제작 진입 UX와 생성 경로를 보완한다.

---

## 1. 목적

CF-FQ-040은 Wagon E2E를 통해 Guided Vehicle Builder의 8-Step 제작 파이프라인, Reference/Proposal/Apply/Driving 검증 계약을 완료했다.

실제 USER 사용 후 다음 제작 진입 문제가 확인됐다.

1. 좌측 `2. 제작 단계` 영역이 작업 내내 비어 있어 8-Step navigation이 표시되지 않는다.
2. Guided Builder의 VehicleData + Recipe 생성 UI가 사실상 "미사용 Mesh-only Candidate"를 먼저 선택한 경우에만 열린다.
3. 따라서 Chassis Mesh도 없는 완전 신규 차량과, 이미 다른 VehicleData가 사용하는 Chassis Mesh를 재사용하는 다른 차량을 정상 Guided 경로에서 시작할 수 없다.

목표:

```text
다음 차량 제작 시작
= Guided Builder 안에서
  [+ 새 차량 만들기]
  → 빈 차량 또는 임의 Chassis Mesh 선택
  → VehicleData + Recipe 생성
  → 8-Step navigation 표시
  → 기존 Step 1~8 제작 흐름으로 진입
```

이 작업은 Wagon 물리 튜닝, ESH, Vehicle Runtime을 다시 여는 작업이 아니다.

---

## 2. USER 피드백과 결함 판정

### 2.1 `2. 제작 단계` 공백

의도된 UI는 다음 8개 Stable Step navigation이다.

```text
1. 차량 / Reference
2. Mesh 준비
3. 소켓 준비 / Naming
4. Layout Capture
5. Physics Proposal
6. Gameplay Setup
7. Final Review
8. Driving Test
```

현재 Source 감사 결과:

- `BuildStepNavigation()`은 Slate Construct 중 호출된다.
- 이 시점에는 `HandleRefreshVehicles()` 전이라 `StepViews.Num()`이 0일 수 있다.
- 이후 `FCFVehicleBuilderVM::RefreshVehicles()`에서 `InitializeSteps()`와 `RebuildStepStates()`가 수행된다.
- 하지만 생성된 `StepNavigationBox` subtree를 다시 구성하지 않는다.

판정:

```text
Step 의미/모델 = 유효
빈 영역 = UI 초기화/갱신 결함
삭제 대상 = 아님
교정 대상 = 맞음
```

### 2.2 신규 차량 생성 진입점이 Mesh-only Candidate에 종속

현재 Guided Builder의 생성 UI 표시 조건은 의미상 다음과 같다.

```text
IsMeshOnlyCandidate == true
AND
CurrentStep == IdentityReference
```

즉 사용자가 "새 차량을 만들겠다"는 의도를 직접 시작하는 selection-independent 진입점이 없다.

### 2.3 이미 사용 중인 Chassis Mesh 재사용 경로 없음

현재 `ListVehicles()`는 기존 `UCFVehicleData`가 사용하는 Chassis Mesh를 `UsedChassisMeshPaths`에 모은 뒤 Mesh-only Candidate에서 제외한다.

따라서 다음과 같은 정상 제작 요구를 Guided Builder에서 시작할 수 없다.

```text
Wagon StaticMesh
├─ DA_Vehicle_Wagon
├─ DA_Vehicle_WagonPolice
└─ DA_Vehicle_WagonArmored
```

같은 Mesh reference를 여러 VehicleData가 공유하는 것은 허용해야 하며, "이미 사용 중"은 생성 Blocker가 아니다.

---

## 3. VBCUX-P0-00 Current Creation Entry Contract Audit — PASS

2026-09-02 read-only Source 감사로 다음을 확정했다.

### 3.1 Step Navigation

주요 Source:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderTab.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderVM.cpp
```

확정:

- `GetVehicleBuilderStepDefinitions()`는 8개 Stable Step을 이미 정의한다.
- Step 상태는 ViewModel에서 authoritative truth로 재평가된다.
- Step model 재설계는 불필요하다.
- Navigation presentation의 초기화/refresh만 교정한다.

### 3.2 Definition + Recipe Backend

주요 Source:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleUXTypes.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleUXOps.cpp
```

`FCFVehicleRecordCreateRequest::ChassisMesh`는 optional이다.

`PreviewVehicleRecords()`와 `CreateVehicleRecords()`는:

- Chassis Mesh null을 금지하지 않는다.
- 다른 VehicleData가 이미 사용하는 Mesh인지 검사하지 않는다.
- 새 `UCFVehicleData + UCFVehicleRecipeData`를 생성한다.
- Recipe.TargetVehicleData를 새 Definition에 binding한다.
- Recipe.AssetIntent.ChassisMesh에 explicit request를 기록한다.
- Profile/차급/물리 값을 자동 추론하지 않는다.
- 자동 DefinitionApply 하지 않는다.
- 자동 Save 하지 않는다.
- explicit OwnershipWrite approval과 Undo transaction을 사용한다.

결론:

```text
빈 차량 생성 Backend = 이미 존재
재사용 Mesh 생성 Backend = 이미 존재
새 raw writer = 불필요
VehicleData Runtime 변경 = 불필요
```

### 3.3 현재 Guided Builder 제한

`FCFVehicleBuilderVM::PrepareSelectedMeshRecordCreate()`는 Mesh-only Candidate selection을 강제한다.

`SCFVehicleBuilderTab::GetMeshCreationVisibility()`도 Mesh-only Candidate + Step 1일 때만 생성 UI를 보인다.

즉 현재 문제는 Backend capability가 아니라 Guided Builder의 진입 UX/ViewModel 경계다.

### 3.4 Mesh-only Candidate의 올바른 역할

현재 Candidate discovery는 "미사용 vehicle mesh를 자동 발견하는 편의 기능"으로 유용하다.

하지만 신규 차량 생성의 유일한 진입점이 아니라 **Quick Start**로 내려야 한다.

---

## 4. Design Lock

### 4.1 명시적 신규 차량 진입

좌측 작업 대상 영역에 selection과 독립적인 action을 둔다.

```text
1. 작업 대상

[ + 새 차량 만들기 ]

──────────────
MANAGED ...
UNMANAGED ...
MESH ...
```

### 4.2 시작 방식

```text
시작 방식
(●) 빈 차량에서 시작
( ) 차체 Mesh로 시작
```

#### 빈 차량에서 시작

```text
ChassisMesh = null
VehicleData + Recipe 생성
이후 Step 2에서 Chassis/Wheel Mesh 지정
```

#### 차체 Mesh로 시작

Object Picker로 임의 `UStaticMesh`를 선택한다.

허용:

- 미사용 Chassis Mesh
- 이미 다른 VehicleData가 사용하는 Chassis Mesh

금지:

- StaticMesh가 아닌 Asset
- 존재하지 않는 Asset
- 동일 package/object identity 충돌

"이미 사용 중인 Mesh"는 정보성 표시는 가능하지만 Blocker로 사용하지 않는다.

중요한 ownership 경계:

```text
신규 Record 생성 직후
= Recipe.AssetIntent.ChassisMesh에만 선택 Mesh 기록
= 새 VehicleData.VehicleVisualConfig.ChassisMesh는 자동 Apply하지 않음

나중의 정상 Builder Step 7 DefinitionApply
= reviewed Recipe/Resolved Definition 기준으로 VehicleData에 반영
```

따라서 reused Mesh Acceptance는 생성 직후 두 VehicleData가 곧바로 같은 Mesh를 참조하는 것으로 판정하지 않는다. 생성 직후에는 새 Recipe intent가 exact reused Mesh를 가리키는지와 기존 VehicleData가 불변인지 확인하고, 새 VehicleData의 실제 ChassisMesh 반영은 정상 DefinitionApply 이후에 검증한다.

### 4.3 Guided 신규 차량 Transmission 정책

CF-FQ-042에서 생성하는 모든 Guided 신규 차량은 기존 CF-FQ-040 계약을 그대로 유지한다.

```text
bRequireVehicleSpecificTransmission = true
→ BuilderTransmissionPolicy = VehicleSpecificRequired
```

Blank Start, Arbitrary Mesh Start, Mesh-only Candidate Quick Start가 모두 같은 정책을 사용한다.
일반 Data Authoring의 generic two-record creation이 `LegacyCompatible`을 유지하는 것은 별도 호환 경로이며, Guided Builder 신규 차량 경로에서는 사용하지 않는다.

### 4.4 Vehicle ID 중심 Naming

일반 경로에서는 package/object 네 칸을 직접 요구하지 않는다.

```text
차량 ID
[ WagonPolice ]
```

기본 제안:

```text
/Game/CarFight/Data/Authoring/DA_Vehicle_WagonPolice
/Game/CarFight/Data/Authoring/DA_Recipe_WagonPolice
```

필요할 때만 `고급 Asset 경로 설정`에서 override한다.

Vehicle ID는 Unreal Asset-safe identifier로 제한한다.

```text
기본 허용: 영문자 / 숫자 / _
예: WagonPolice, Wagon_Police, SUV01
```

공백, 경로 구분자 등 invalid 문자는 UI 단계에서 즉시 설명하고, 최종 생성 가능 여부는 기존 `ValidateNewAssetIdentity()` preview가 authoritative fail-closed 판정을 소유한다.

Naming은 semantic inference가 아니라 deterministic 문자열 제안이다.

### 4.5 Mesh-only Candidate Quick Start

기존 Mesh-only Candidate는 유지한다.

```text
MESH Candidate 선택
→ [이 Mesh로 새 차량 만들기]
→ 일반 New Vehicle Preview/Approval flow
```

최종 구조:

```text
새 차량 만들기
├─ 빈 차량에서 시작
├─ 원하는 Mesh로 시작
│  ├─ 미사용 Mesh
│  └─ 이미 사용 중인 Mesh
└─ Mesh-only Candidate 빠른 시작
```

### 4.6 Step Navigation

8-Step navigation은 vehicle list refresh 결과와 무관하게 항상 안정적으로 존재해야 한다.

```text
Builder VM 생성
→ Stable Step Definitions 8개 즉시 초기화
→ Navigation row 8개 생성
→ 이후 Browser/selection/Recipe state는 각 row의 text/state/enabled만 갱신
```

즉 Builder workflow definition은 Asset Registry Browser 성공 여부와 분리한다.
Vehicle 선택, refresh, Recipe mutation 후에도 empty/stale navigation이 남아서는 안 된다.

No-selection Step 1 guidance도 신규 생성 진입을 반영한다.

```text
기존 차량/메시 후보를 선택하거나
'+ 새 차량 만들기'로 새 차량 제작을 시작하세요.
```

### 4.7 New Vehicle transient state owner

Slate는 입력과 표시를 담당하고, 신규 차량 생성의 semantic state/validation은 Builder ViewModel이 소유한다.

개념적 transient state는 다음 의미를 포함한다.

```text
VehicleId
StartMode = Blank | ChassisMesh
OptionalChassisMesh
DerivedDefinitionIdentity
DerivedRecipeIdentity
```

별도 public type 추가 자체가 목적은 아니다. 구현 형태와 무관하게 `SCFVehicleBuilderTab`에 생성 의미를 흩어진 TextBox state로만 보존하지 않고, BuilderVM이 canonical creation request를 구성해야 한다.

---

## 5. 구현 Gate

### VBCUX-P0-01 — Step Navigation / Explicit New Vehicle Entry UX

범위:

- BuilderVM 생성 시 Stable Step 8개를 Browser refresh보다 먼저 초기화
- 8-Step navigation 최초 표시 교정
- refresh/selection 후 stale/empty 방지
- `+ 새 차량 만들기` 진입 버튼
- BuilderVM-owned transient New Vehicle mode/state
- No-selection Step 1 guidance 교정
- 기존 managed/unmanaged/mesh 목록 유지

PASS:

- fresh BuilderVM에서 `RefreshVehicles()` 호출 전에도 StepViews가 exact 8개다.
- Builder Tab을 처음 열었을 때 제작 단계 8개가 표시된다.
- 차량 선택/목록 새로고침 후에도 유지된다.
- 선택 row 없이 `+ 새 차량 만들기`에 진입할 수 있다.
- No-selection Step 1이 기존 대상 선택과 신규 차량 생성을 모두 정상 행동으로 안내한다.

### VBCUX-P0-02 — Blank / Arbitrary Mesh Record Creation

범위:

- Blank / Arbitrary / Quick Start가 하나의 Builder canonical create request 경로를 사용
- 빈 차량 생성
- 임의 Chassis StaticMesh Object Picker
- 이미 사용 중인 Chassis Mesh 허용
- 모든 Guided 신규 생성에서 `bRequireVehicleSpecificTransmission=true`
- 기존 `CreateVehicleRecords()` Preview → explicit approval → commit 재사용
- 생성 성공 뒤 exact Created Definition/Recipe를 Builder current target으로 adoption
- Browser refresh 뒤 exact created row highlight/selection과 Step state fresh rebuild
- post-create adoption 실패는 record creation 성공과 구분해 명시적으로 보고

PASS:

- `ChassisMesh=null`로 VehicleData+Recipe 생성 가능
- 기존 VehicleData가 사용하는 Mesh로 별도 VehicleData+Recipe 생성 가능
- Blank / Unused / Reused 세 경로 모두 새 Recipe가 `VehicleSpecificRequired`
- Mesh를 지정한 생성 직후 새 Recipe `AssetIntent.ChassisMesh`가 exact Mesh를 가리킨다.
- 생성 직후 새 VehicleData `VehicleVisualConfig.ChassisMesh`를 자동 변경하지 않는다.
- 원본 VehicleData/Recipe 불변
- 생성 성공 뒤 Authoring selection, Builder transient state, Browser row selection, Step state가 exact 새 차량으로 동기화된다.
- post-create Builder adoption만 실패한 경우 생성된 Asset을 임의 롤백하지 않고 `records created / Builder adoption failed`로 구분한다.
- 자동 Save/DefinitionApply 없음

### VBCUX-P0-03 — Vehicle ID Naming / Candidate Quick Start

범위:

- Vehicle ID 한 칸으로 default package/object identity 제안
- 고급 경로 override를 접힘 UI로 격리
- Mesh-only Candidate Quick Start 연결
- collision/invalid identity는 기존 Preview에서 fail-closed

PASS:

- 정상 생성에서 package/object 네 칸을 직접 입력할 필요가 없다.
- Candidate Quick Start도 일반 New Vehicle과 동일한 Preview/approval 계약을 사용한다.

### VBCUX-P0-04 — Focused Regression

검증:

```text
Pre-refresh fresh BuilderVM StepViews == exact 8
Builder Tab initial navigation row == exact 8
No-selection Step 1 guidance includes new vehicle entry
No-selection new vehicle entry
Blank create preview/commit
Unused Mesh create
Already-used Chassis Mesh create
Blank/Unused/Reused all VehicleSpecificRequired
Created Recipe AssetIntent exact Chassis binding
Created VehicleData Chassis remains unapplied until DefinitionApply
Invalid VehicleId / asset/type fail-closed
Path/name collision fail-closed
Original reused-mesh owner unchanged
Post-create exact Builder target adoption
Post-create Browser exact row selection/highlight
Post-create adoption failure reports partial success without hidden rollback
No auto Save
No auto DefinitionApply
Existing Mesh-only Candidate discovery
Existing Step 2 typed AssetIntent
```

기존 CF-FQ-040 전체 ESH/고속 benchmark는 반복하지 않는다.

새 failure가 Vehicle Runtime/Physics와 직접 연결되지 않는 한 Wagon driving regression도 반복하지 않는다.

### VBCUX-P0-05 — USER Acceptance

#### A. 완전 신규

```text
+ 새 차량 만들기
→ 빈 차량에서 시작
→ Vehicle ID 입력
→ 생성 검토/승인
→ 새 VehicleData + Recipe 표시
→ Step 2에서 Mesh 선택 가능
```

#### B. 기존 Mesh 재사용

```text
+ 새 차량 만들기
→ 차체 Mesh로 시작
→ 기존 차량이 사용하는 Chassis Mesh 선택
→ 다른 Vehicle ID로 생성
→ 새 Recipe.AssetIntent.ChassisMesh가 같은 Mesh를 참조
→ 기존 VehicleData/Recipe 불변
→ 새 VehicleData에는 아직 자동 Apply하지 않음
→ 이후 정상 Step 7 DefinitionApply에서 새 VehicleData도 같은 Chassis Mesh를 사용
```

재사용하는 Chassis Mesh의 Mesh-owned geometry도 공유된다는 점을 USER에게 안내한다.

```text
공유됨:
Wheel_Anchor_* Socket 위치/RelativeScale
WSA가 읽는 Socket 기반 wheel geometry
Chassis Mesh에 포함된 다른 Socket/Hardpoint geometry
```

따라서 같은 시각 Mesh를 쓰되 wheelbase/Socket geometry 자체가 다른 차량이 필요하면 별도 Chassis Mesh variant가 필요하다. per-Vehicle Socket override는 CF-FQ-042 Scope Out이다.

#### C. 미사용 Mesh Quick Start

```text
MESH Candidate 선택
→ 이 Mesh로 새 차량 만들기
→ 일반 New Vehicle review/approval
→ 새 managed record로 진입
```

USER 추가 확인:

- 제작 단계 8개가 실제로 보이는가
- 현재 Step/Complete/Blocked/Locked 상태를 이해하기 쉬운가
- 신규 차량 시작 위치를 즉시 알 수 있는가
- package path를 몰라도 일반 제작이 가능한가

---

## 6. Feature Acceptance

```text
1. 새 차량 생성은 Guided Builder 내부의 명시적 정상 기능이다.
2. 완전 빈 VehicleData + Recipe에서 시작할 수 있다.
3. 이미 사용 중인 Chassis Mesh를 재사용해 별도 차량을 만들 수 있다.
4. Mesh-only Candidate는 convenience Quick Start로 유지된다.
5. Vehicle ID 하나로 일반 naming을 시작할 수 있다.
6. 8-Step 제작 단계 navigation이 실제로 항상 표시된다.
7. 모든 Guided 신규 차량은 `VehicleSpecificRequired` Transmission 정책을 유지한다.
8. 생성 단계의 Mesh 지정은 Recipe AssetIntent에 기록하고 VehicleData에는 자동 Apply하지 않는다.
9. 생성 성공 후 exact 새 차량으로 Builder/Browser/Step state가 동기화된다.
10. reused Chassis Mesh는 Mesh-owned Socket/WSA geometry도 공유함을 명시한다.
11. 기존 Preview / explicit approval / no-auto-save / no-auto-apply 계약을 유지한다.
12. 기존 Wagon/ESH/Vehicle Runtime을 재작업하지 않는다.
13. USER가 A/B/C 시나리오를 실제 UI에서 확인한다.
```

완료 후 Current Knowledge는 main_game `Document/Systems/Vehicles/VehicleBuilder.md`의 Creation Entry / Guided Shell 계약에 승격한다.

---

## 7. 보호 범위 / Scope Out

### 보호

```text
CF-FQ-040 = Done 유지
VehicleBuilder Current architecture 유지
Wagon E2E acceptance 유지
WSA P0 Complete 유지
ESH-01~06 Final Audit Clean PASS 유지
CreateVehicleRecords Preview/OwnershipWrite/no-save 계약 재사용
Data Authoring = Builder Backend + Advanced Workspace 유지
```

### Scope Out

```text
Wheel 자동 검출
Socket 자동 배치
Hardpoint 자동 배치
Mesh Asset 복제
per-Vehicle Chassis Socket/WSA geometry override
VehicleData 복제/클론 마법사
실차 Reference 조사 UI 확장
Physics Proposal 알고리즘 재설계
Transmission/Engine Curve 재설계
Benchmark 성능 튜닝
Runtime Apply Menu
```

"같은 Mesh로 새 차량 만들기"는 Mesh를 복제하는 기능이 아니라 여러 VehicleData가 같은 Chassis Mesh reference를 공유하는 기능이다.

---

## 8. 예상 Source 범위

주요 변경 후보:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderTab.cpp
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTab.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderVM.cpp
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderVM.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleAuthoringVMTests.cpp
```

현재 감사 기준으로 semantic 변경을 기본 요구하지 않는 영역:

```text
CFVehicleUXOps.cpp
CFVehicleAuthoringService.cpp
UCFVehicleData Runtime
ACFVehiclePawn Runtime
Engine/Transmission/WSA Runtime
```

구현 중 Backend 변경이 필요해지면 먼저 "일반 생성 계약의 실제 결함"인지 확인한다. 특정 차량명/차종 전용 예외 코드는 추가하지 않는다.

---

## 9. 현재 상태

```text
Feature: CF-FQ-042
Status: Done
Priority: P2
VBCUX-P0-00: PASS
VBCUX-P0-01: Technical PASS
VBCUX-P0-02: Technical PASS
VBCUX-P0-03: Technical PASS
VBCUX-P0-04: Technical PASS
VBCUX-P0-05: USER PASS
Implementation mutation: VBCUX-P0-01~03 Editor Source + Automation / P0-04 Automation + WITH_DEV_AUTOMATION_TESTS seam only
USER UAT transient records: A/B/C 실제 생성 검토·승인 / Product Asset Save 0
Vehicle Runtime mutation: 0
Wagon/ESH reopen: 0
Current System owner: main_game Document/Systems/Vehicles/VehicleBuilder.md v1.1.1
```

현재 단일 Active `CF-FQ-039 Production UI Visual Rework`를 자동 대체하지 않는다.

VBCUX-P0-01 Technical Validation:

```text
Official Editor Build: PASS
Build Job: c584c88e679448b696f715cc2263e6f2
Focused Automation: CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell
Automation Result: SUCCESS_COUNT=1 / FAILURE_COUNT=0
Validation scope: pre-refresh StepViews exact8 / no-selection guidance / explicit New Vehicle transient entry / refresh exact8 preservation / actual Builder Tab spawn smoke
Product Asset Save: 0
VehicleData Apply: 0
Vehicle Runtime / Chaos mutation: 0
```

구현은 `CFVehicleBuilderVM.h/.cpp`, `CFVehicleBuilderTab.h/.cpp`, 기존 BuilderShell regression test에만 한정했다. `+ 새 차량 만들기`는 selection-independent transient 진입만 수행하고 실제 Blank/Arbitrary record creation은 VBCUX-P0-02가 소유한다. CF-FQ-040 Done/Wagon/WSA/ESH 완료 evidence는 재오픈하지 않았다.

VBCUX-P0-02 Technical Validation:

```text
Official Editor Build: PASS
Build Job: a21a3bcce7b449f5a2efda698b465525
Focused Automation 1: CarFight.DataAuthoring.CF_FQ_042.VBCUX_P0_02.RecordCreation — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Affected Automation 2: CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Affected Automation 3: CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Validation scope: Blank creation / unused Mesh Quick Start / reused Chassis creation / VehicleSpecificRequired / Recipe-only Chassis intent / new VehicleData non-Apply / original VehicleData unchanged / exact created Definition+Recipe Builder adoption / pre-refresh Step exact8 regression
Product Asset Save: 0
DefinitionApply: 0
Vehicle Runtime / Chaos mutation: 0
```

P0-02는 기존 `CreateVehicleRecords()` Preview→explicit approval→commit Backend를 재사용한다. Blank/Arbitrary/Reused/Mesh-only Quick Start는 Builder의 공통 Guided creation request로 통합되며 모두 `VehicleSpecificRequired`다. Optional Chassis는 새 Recipe `AssetIntent.ChassisMesh`에만 기록하고 새 VehicleData에는 자동 Apply하지 않는다. 생성 성공 뒤 exact Created Definition+Recipe를 fresh Browser row에서 재확인해 Builder current target과 row selection을 동기화한다. Adoption만 실패하면 생성된 records를 롤백하지 않고 `records created / Builder adoption failed`로 구분한다.

VBCUX-P0-03 Technical Validation:

```text
Official Editor Build: PASS
Build Job: 89447e7577ea40a3a1f019571e9c03c8
Focused Automation 1: CarFight.DataAuthoring.CF_FQ_042.VBCUX_P0_03.Naming — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Affected Automation 2: CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Affected Automation 3: CarFight.DataAuthoring.CF_FQ_042.VBCUX_P0_02.RecordCreation — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Affected Automation 4: CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Validation scope: Vehicle ID single-field default naming / ASCII letters+digits+underscore valid / empty+space+slash+non-ASCII invalid fail-closed / no silent sanitize / deterministic Definition+Recipe package+object proposal / Explicit New Vehicle empty-ID reset / Mesh Candidate stem suggestion / common Preview→approval→commit path / P0-02 creation and Step exact8 regression preservation
Product Asset Save: 0
DefinitionApply: 0
Vehicle Runtime / Chaos mutation: 0
```

P0-03은 일반 제작 화면의 4개 package/object 입력을 기본 UX에서 제거하고 `Vehicle ID` 한 칸을 기본 naming owner로 전환했다. `WagonPolice`를 입력하면 `/Game/CarFight/Data/Authoring/DA_Vehicle_WagonPolice`와 `/Game/CarFight/Data/Authoring/DA_Recipe_WagonPolice`를 deterministic하게 제안한다. 4개 exact identity 입력은 `고급 Asset 경로 설정` 접힘 영역에 override로 유지한다. Vehicle ID 변경 시에만 default identity를 다시 채우므로 사용자가 고급 override를 수정한 뒤 생성 검토를 누르면 그 override가 보존된다. Mesh-only Candidate는 `SM_` prefix를 제거한 Mesh stem을 Vehicle ID 제안값으로 사용하고 `이 Mesh로 새 차량 만들기 — 검토`를 통해 같은 Preview/승인 경로에 들어간다. final collision/path/type authority는 기존 Preview/`ValidateNewAssetIdentity`가 계속 소유한다.

VBCUX-P0-04 Technical Validation:

```text
Official Editor Build: PASS
Final Build Job: 3802dcca8ade47ffaf28100e5c98a32e
Focused Automation Set: CarFight.DataAuthoring.CF_FQ_042 — SUCCESS_COUNT=3 / FAILURE_COUNT=0
  PASS: VBCUX_P0_02.RecordCreation
  PASS: VBCUX_P0_03.Naming
  PASS: VBCUX_P0_04.FocusedRegression
Affected Automation 1: CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Affected Automation 2: CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell — SUCCESS_COUNT=1 / FAILURE_COUNT=0
Validation scope: pre-refresh Step exact8 / no-selection new entry / Blank / unused Mesh / reused Chassis / VehicleSpecificRequired / Recipe-only Chassis intent / VehicleData non-Apply / invalid Vehicle ID / wrong Chassis asset type / path-name collision fail-closed / original reused owner unchanged / exact post-create Builder target / exact fresh Browser row / partial-success no-hidden-rollback / no auto Save / no auto DefinitionApply / Mesh Candidate discovery / Step 2 typed AssetIntent
Product behavior mutation in P0-04: 0
Product Asset Save: 0
DefinitionApply: 0
Vehicle Runtime / Chaos mutation: 0
```

P0-04에서 `FCFVBCUXP004FocusedRegressionTest`를 추가하고 P0-02 success case에 fresh Browser exact row assertion을 보강했다. `AdoptCreatedVehicleRecords()`의 private failure branch는 Public API로 노출하지 않고 `WITH_DEV_AUTOMATION_TESTS` friend seam으로만 검증한다. 최초 adoption-failure fixture는 `/Game` loaded object가 Asset Registry Browser에 실제 발견되어 예상과 달리 adoption 성공했으므로 제품 결함으로 판정하지 않았고, Project Browser에 나타나지 않는 `/Engine/Transient` loaded object로 fixture를 교정한 뒤 final focused run을 PASS했다. 첫 공식 build는 병렬 CF-FQ-041 Runtime Apply의 `CFVehicleDebugPanelWidget` 작업 중간 UHT 상태로 exit 6이었으며, 해당 owner가 v1.34.1로 교정된 뒤 본 작업은 unrelated 파일 mutation 없이 official build를 PASS했다.

VBCUX-P0-05 USER Acceptance:

```text
A Blank Start: USER PASS
- + 새 차량 만들기 진입 확인
- Stable 제작 단계 8개 표시 확인
- Blank 생성 검토/승인/새 target 전환 정상
- Step 2에서 후속 Mesh 지정 경로 정상

B 기존 Chassis Mesh 재사용: USER PASS
- 기존 Wagon Chassis Mesh 재사용 검토창 확인
- 새 VehicleData/Recipe identity, VehicleSpecificRequired 표시 정상
- Recipe AssetIntent-only / 새 VehicleData Chassis auto Apply 안 함 / 자동 저장 안 함 표시 정상
- 생성 후 새 target adoption 정상
- 기존 Step 7 DefinitionApply 의미는 CF-FQ-040 완료 계약과 P0-02~04 regression을 재사용하고 별도 runtime 재작업하지 않음

C 미사용 Mesh Quick Start: USER PASS
- MESH Candidate → 이 Mesh로 새 차량 만들기 → 공통 Review/Approval → 새 managed record 진입 정상

전체 UX: USER PASS
Product Asset Save: 0
DefinitionApply during creation: 0
Vehicle Runtime / Chaos mutation: 0
```

USER는 Vehicle ID를 매번 직접 알고 입력해야 하는 점의 관리 부담을 지적했다. 현재 CarFight에는 차량/무기를 중앙에서 등록·조회하는 통합 데이터 Registry가 없고, 이 Vehicle ID는 Builder의 Definition/Recipe Asset 생성용 naming token에 가깝다. USER는 이 피드백을 비차단 후속 검토로 두고 P0-05를 계속 진행해 A/B/C 전체를 PASS했다. 따라서 CF-FQ-042에서는 추가 naming 재설계를 열지 않고 Current System에 의미와 피드백을 승격한다.

Post-closure final audit에서 P1 1건을 추가 발견했다. 기존 managed 차량이 선택된 상태에서 `+ 새 차량 만들기`로 들어가도 underlying Authoring selection은 남아 있었고, Browser refresh 시 Slate의 exact-row highlight 복원 코드가 old row를 다시 `SetSelection()`하면서 `SelectVehicle()` → `ResetNewVehicleEntryState()`로 신규 제작 모드를 조용히 해제할 수 있었다. 교정은 `BeginNewVehicleEntry()`에서 Authoring selection을 clear하고, Slate refresh도 New Vehicle mode에서는 기존 row restore를 하지 않는 이중 방어로 한정했다. Browser cache/persistent Asset/Save/DefinitionApply/Runtime은 변경하지 않는다.

교정 검증:

```text
USER manual Editor build: PASS
CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell: 1/0 PASS
CarFight.DataAuthoring.CF_FQ_042: 3/0 PASS
CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate: 1/0 PASS
focused/affected total: 5/0 PASS
```

BuilderShell에는 `existing managed selection → BeginNewVehicleEntry → Vehicle ID 입력 → Browser refresh → New Vehicle active / old Authoring selection false / Vehicle ID preserved` exact regression을 추가했다. 따라서 final audit P1은 교정 완료이며 CF-FQ-042 Done/USER PASS는 유지한다.

Current System Promotion:

```text
G0 Evidence: PASS — P0-01~04 Technical + P0-05 USER A/B/C PASS
G1 Current Knowledge Promotion: PASS — Systems/Vehicles/VehicleBuilder.md v1.1.1
G2 Current Route Cleanup: PASS — ActiveWork / Plan Index Ready route 제거
G3 Reference Preservation: PASS — 본 Plan 기존 경로 보존
G4 Historical: PASS — Historical + Retained Path
G5 Physical Move: Deferred — 별도 maintenance에서만 수행
```

---

## 10. Changelog

### Maintenance - 2026-09-06

- G5 Physical Move를 완료해 대표 Plan을 `Document/Plan/Archive/VehicleBuilderCreationUX/VehicleBuilderCreationUXPlan.md`로 이동했다. Feature lifecycle, 완료 evidence와 Current System 계약은 변경하지 않는다.
- Migration: 이전 `Document/Plan/VehicleBuilderCreationUX/VehicleBuilderCreationUXPlan.md` 경로는 당시 Historical 기록에서만 유효하며 현재 탐색 경로는 Archive 경로다.

### v0.2.1 - 2026-09-02

- Post-closure final audit에서 발견한 P1 old-selection Browser refresh restore 회귀를 `BeginNewVehicleEntry()` Authoring selection clear + Slate New Vehicle row-restore guard로 교정했다.
- BuilderShell에 existing managed selection부터 시작하는 exact refresh-preservation 회귀를 추가했고 Vehicle ID/state 보존과 old selection 비복원을 검증한다.
- USER manual Editor build PASS, BuilderShell 1/0, CF_FQ_042 3/0, MeshCreate 1/0으로 focused/affected 5/0 PASS를 확인했다.
- Product Asset/Save/DefinitionApply/Vehicle Runtime mutation은 없고 CF-FQ-042는 Done / Historical + Retained Path를 유지한다. Current owner는 main_game `Systems/Vehicles/VehicleBuilder.md v1.1.1`이다.

### v0.2.0 - 2026-09-02

- `VBCUX-P0-05 USER Acceptance`를 A Blank Start / B 기존 Chassis 재사용 / C 미사용 Mesh Quick Start 모두 USER PASS로 닫고 CF-FQ-042를 Done으로 전환했다.
- USER는 Stable Step 8개 표시, `+ 새 차량 만들기`, 생성 검토/승인과 새 target adoption을 실제 Builder UI에서 확인했다. 생성 단계 Product Asset Save, DefinitionApply와 Vehicle Runtime mutation은 0이다.
- Vehicle ID 직접 입력 관리 부담 피드백은 현재 통합 Vehicle Registry 부재와 creation naming token 성격을 명시한 비차단 후속 UX 항목으로 보존했다.
- Current Knowledge를 main_game `Systems/Vehicles/VehicleBuilder.md v1.1.0`에 승격하고 G0~G4를 PASS로 닫았다. 대표 Plan은 물리 이동 없이 `Historical + Retained Path`로 현재 경로에 보존한다.
- P0-04 build/automation은 관련 Source 변경이 없어 반복하지 않았다. CF-FQ-040 Done/Wagon/WSA/ESH/Vehicle Runtime은 재오픈하지 않았다.

### v0.1.5 - 2026-09-02

- `VBCUX-P0-04 Focused Regression`을 Technical PASS로 닫았다.
- Vehicle record package/name collision, wrong Chassis asset type, post-create adoption failure partial-success/no-hidden-rollback을 새 focused Automation으로 직접 검증했다.
- P0-02 Blank/Reused success case에 exact fresh Browser Definition+Recipe row 존재 assertion을 추가해 selected target뿐 아니라 Browser adoption까지 회귀 보호했다.
- 최종 공식 UE 5.8 Editor Build `3802dcca8ade47ffaf28100e5c98a32e` PASS 후 CF-FQ-042 3건 + MeshCreate + BuilderShell 총 5개 focused/affected test가 모두 PASS했다.
- P0-04 production behavior mutation은 0이며 test-only friend seam은 `WITH_DEV_AUTOMATION_TESTS`에 한정했다. Product Asset Save, DefinitionApply, Vehicle Runtime/Chaos mutation도 0이다.
- 병렬 CF-FQ-041 Runtime Apply의 중간 UHT failure는 해당 owner 교정 후 해소됐고, Runtime Apply/Vehicle/Fitting/UI dirty는 본 작업에서 수정하지 않았다.
- next Gate를 `VBCUX-P0-05 USER Acceptance`로 전진했다. CF-FQ-040 Done/Wagon/WSA/ESH는 재오픈하지 않았다.

### v0.1.4 - 2026-09-02

- `VBCUX-P0-03 Vehicle ID Naming / Candidate Quick Start` 구현과 Technical Validation을 완료했다.
- 일반 Guided 신규 차량 naming을 `Vehicle ID` 한 칸으로 전환하고 `DA_Vehicle_<Id>` / `DA_Recipe_<Id>` package+object identity를 deterministic하게 제안하도록 했다.
- Vehicle ID는 ASCII 영문자·숫자·`_`만 허용하며 빈값, 공백, 경로 구분자와 비ASCII는 sanitize하지 않고 즉시 fail-closed한다. 최종 collision/path/type 검증은 기존 Preview authority를 유지한다.
- 기존 4개 package/object 입력은 삭제하지 않고 접힌 `고급 Asset 경로 설정` override로 이동했다. Vehicle ID 변경 전까지 USER override를 보존한다.
- Mesh-only Candidate는 Mesh stem 기반 Vehicle ID 제안과 `이 Mesh로 새 차량 만들기 — 검토`를 제공하며 일반 신규 차량과 같은 naming helper 및 Preview→approval→commit 경로를 재사용한다.
- UE 5.8 Editor Build PASS와 Naming / MeshCreate / P0-02 RecordCreation / BuilderShell Automation 4건 모두 `1 success / 0 failure`를 확보했다. Product Asset Save, DefinitionApply, Vehicle Runtime/Chaos mutation은 0이다.
- next Gate를 `VBCUX-P0-04 Focused Regression`으로 전진했다. CF-FQ-040 Done/Wagon/WSA/ESH는 재오픈하지 않았다.

### v0.1.3 - 2026-09-02

- `VBCUX-P0-02 Blank / Arbitrary Mesh Record Creation` 구현과 Technical Validation을 완료했다.
- Blank, 임의 StaticMesh, 이미 다른 VehicleData가 사용 중인 Chassis Mesh, 기존 Mesh-only Quick Start를 하나의 Builder canonical Guided create request에 연결하고 모두 `VehicleSpecificRequired`를 강제했다.
- 신규 Step 1에 optional Chassis StaticMesh picker를 추가했으며 clear 상태는 Blank Start로 동작한다. 지정 Mesh는 Recipe AssetIntent에만 기록하고 VehicleData Chassis는 자동 Apply하지 않는다.
- record creation 성공 뒤 exact Created Definition+Recipe를 fresh Browser row로 검증해 Builder current target/highlight를 adoption하며, adoption 실패는 생성 성공과 분리해 보고하고 자동 롤백하지 않는다.
- UE 5.8 Editor Build PASS와 P0-02 RecordCreation / 기존 MeshCreate / BuilderShell exact Automation 3건 모두 `1 success / 0 failure`를 확보했다. Product Asset Save, DefinitionApply, Vehicle Runtime/Chaos mutation은 0이다.
- next Gate를 `VBCUX-P0-03 Vehicle ID Naming / Candidate Quick Start`로 전진했다. CF-FQ-040 Done/Wagon/WSA/ESH는 재오픈하지 않았다.

### v0.1.2 - 2026-09-02

- `VBCUX-P0-01 Step Navigation / Explicit New Vehicle Entry UX` 구현과 Technical Validation을 완료했다.
- BuilderVM 생성 직후 Stable Step 8개를 초기화하고, 좌측 `+ 새 차량 만들기`와 BuilderVM-owned transient New Vehicle state, no-selection Step 1 guidance를 추가했다.
- 기존 managed/unmanaged/mesh 목록과 Mesh-only Quick Start는 유지하고 신규 제작 모드에서는 stale selected-vehicle panel이 노출되지 않도록 분리했다.
- 공식 UE 5.8 Editor Build PASS와 exact `BuilderShell` focused Automation `1 success / 0 failure`를 확보했다. Product Asset Save, VehicleData Apply, Vehicle Runtime/Chaos mutation은 0이다.
- next Gate를 `VBCUX-P0-02 Blank / Arbitrary Mesh Record Creation`으로 전진했다. CF-FQ-040 Done/Wagon/WSA/ESH는 재오픈하지 않았다.

### v0.1.1 - 2026-09-02

- 설계검수 P1 5건/P2 3건을 반영해 구현 전 계약을 교정했다.
- reused Chassis Mesh는 생성 시 `Recipe.AssetIntent.ChassisMesh`에만 기록하고 새 VehicleData에는 자동 Apply하지 않으며, 실제 VehicleData Chassis 반영은 정상 Step 7 DefinitionApply가 소유하도록 명시했다.
- Blank/Arbitrary/Quick Start 모두 Guided 신규 차량이므로 `VehicleSpecificRequired`를 강제하고 regression acceptance에 추가했다.
- record 생성 성공과 post-create Builder adoption을 분리해 exact Created target/Browser row/Step state 동기화와 partial-success reporting을 정의했다.
- reused Chassis Mesh가 Wheel_Anchor/WSA/Hardpoint Socket geometry도 공유함을 명시하고 per-Vehicle Socket override를 Scope Out으로 추가했다.
- Step Navigation 회귀를 post-refresh count가 아니라 fresh BuilderVM pre-refresh exact 8개로 강화하고 No-selection 신규 생성 안내를 추가했다.
- New Vehicle semantic transient state는 BuilderVM이 소유하고 Vehicle ID를 Unreal Asset-safe identifier로 제한하도록 고정했다.

### v0.1.0 - 2026-09-02

- USER 실제 Builder 사용 피드백을 근거로 CF-FQ-040을 재오픈하지 않는 별도 후속 Feature `CF-FQ-042 Vehicle Builder 신규 차량 생성 UX` 계획을 신규 생성했다.
- `2. 제작 단계` 공백을 Step model 부재가 아니라 Slate navigation 초기화/갱신 결함으로 판정했다.
- Backend `CreateVehicleRecords`가 이미 null Chassis와 임의/reused StaticMesh를 수용함을 Source에서 확인해, 새 Runtime/VehicleData writer 없이 Guided Builder 진입 UX만 확장하는 방향을 고정했다.
- `+ 새 차량 만들기`, Blank Start, Arbitrary/Reused Mesh Start, Vehicle ID naming, Mesh Candidate Quick Start와 A/B/C USER Acceptance를 정식 Gate로 정의했다.
- CF-FQ-040 Done, Wagon/WSA/ESH 완료 evidence와 현재 단일 Active CF-FQ-039를 보호한다.
