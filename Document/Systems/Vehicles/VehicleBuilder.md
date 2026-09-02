# Vehicle Builder

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-09-02
- 문서 상태: Current Implementation
- 적용 범위: `CF-FQ-040 Guided Vehicle Builder`, Guided Builder Editor Shell, Builder-private Authoring ownership, Data Authoring Backend/Advanced Workspace 연계
- 완료 기반: `VB-P0-09 End-to-End USER Acceptance PASS` + `VB-P0-10 Current System Promotion Complete`

---

## 1. 문서 목적

이 문서는 CarFight에서 **신규 차량 한 대를 정상 제작할 때 사용하는 Guided Vehicle Builder의 현재 구현 구조와 책임 경계**를 기록한다.

미래 Builder 기능 계획이나 과거 단계별 구현 이력을 소유하지 않는다.
완료 당시 상세 Build/Automation/USER evidence는 Historical Plan이 보존하고, 현재 구현 판단은 이 문서와 실제 Source/Asset을 우선한다.

```text
정상 차량 제작 UX
= Vehicle Builder

차량 Authoring 공통 엔진
= Data Authoring Backend

전문가용 수동 편집/복구
= Data Authoring Advanced Workspace

런타임 최종 차량 정의
= UCFVehicleData
```

---

## 2. 현재 핵심 구조

현재 Builder의 상위 흐름은 다음과 같다.

```text
SCFVehicleBuilderTab
→ FCFVehicleBuilderVM
→ 기존 FCFVehicleAuthoringVM / Authoring Service
→ Recipe / Resolver / Registry / SourceTrace / Diff / Validation
→ 명시적 Preview / Approval
→ Profile / Recipe / Evidence authoring
→ Final Review
→ DefinitionApply / Guarded Undo
→ UCFVehicleData
→ ACFVehiclePawn / Chaos Vehicle Runtime
```

주요 Source:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTab.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderVM.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAuthoringService.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAIContract.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleProfileTypes.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleRecipeData.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleUXTypes.h
```

Builder는 새로운 Runtime 차량 정의 체계나 별도 raw writer를 만들지 않는다.
최종 Runtime Authority는 계속 `UCFVehicleData`이며 실제 적용 계약은 `VehicleData.md`와 `VehicleRuntime.md`가 소유한다.

---

## 3. Current ownership

### 3.1 Guided Shell

`SCFVehicleBuilderTab`은 사용자가 정상 제작 흐름을 따라가는 Editor Slate 진입점이다.

현재 책임:

```text
차량/메시 후보 표시와 선택
Stable Step 기반 단계 이동
현재 Step의 상태/Blocker/해결 방법 표시
Reference Draft 검토와 승인 UI
Mesh/Socket 준비 안내
Physics Proposal 검토와 승인 UI
Gameplay Guidance 표시
Final Review / Apply / Undo UI
Technical Benchmark 실행
PIE transient test-drive 준비
USER Driving PASS 명시 입력
Advanced Workspace 진입
```

한 화면에 VehicleData 전체 필드를 펼치는 방식이 아니라 **현재 단계에 필요한 정보와 행동만 노출**한다.

### 3.2 Builder ViewModel

`FCFVehicleBuilderVM`은 transient workflow state와 Step evaluator를 소유한다.

현재 원칙:

```text
persistent truth를 복제하지 않음
fresh Authoring read를 기준으로 Step 상태 재평가
Stable StepId로 semantic step을 식별
fixed index에 의미를 결합하지 않음
stale approval/token을 restart 뒤 자동 부활시키지 않음
새 writer를 만들지 않음
```

Builder ViewModel은 기존 Authoring facade를 재사용하며 persistent mutation authority를 직접 소유하지 않는다.

### 3.3 Persistent Editor Authoring owner

현재 persistent authoring truth는 다음으로 분리된다.

```text
UCFVehicleRecipeData
= Target binding
+ Asset/Profile/Feel/Mass/Hardpoint/Mount intent
+ Applied state
+ Builder provenance receipt
+ vehicle-specific Transmission review receipt
+ vehicle-specific Engine Curve review receipt

UCFVehicleRefEvidence
= 실존 차량 Reference identity/source/claim/conflict/unknown Evidence

Builder-private Profiles
= VehicleBase
+ Drivetrain
+ Handling
+ Performance

UCFVehicleData
= 최종 Runtime 차량 정의
```

Builder-private Profile은 `OwnerRecipeId`가 유효한 차량 전용 Profile이다.
기존 shared/legacy Profile은 Builder-private commit 대상으로 간주하지 않는다.

### 3.4 Intermediate AI Draft

AI가 작성하는 Research/Physics Draft는 Project Saved의 **검토 전 중간 입력**이다.

```text
ResearchDraft
= exact Recipe/Target에 binding된 Reference research 제안

PhysicsDraft
= exact Recipe/Target/accepted Evidence에 binding된 typed vehicle-specific Proposal
```

Draft 파일 자체는 Runtime Authority도, persistent approved authoring truth도 아니다.
Builder가 current identity/fingerprint/hash를 다시 확인하고 Preview를 만든 뒤 USER가 명시 승인해야 persistent owner로 승격된다.

---

## 4. 현재 8-Step Guided Workflow

정상 제작 흐름은 다음 Stable Step 의미로 운영한다.

| Step | 의미 | Current 책임 |
| --- | --- | --- |
| 1 | Identity / Reference | 차량 record 선택·생성, ResearchDraft, Reference Evidence/Companion review, Reference USER acceptance |
| 2 | Mesh Prep | Chassis/Wheel Mesh 준비와 typed AssetIntent 반영 |
| 3 | Socket Guide | Wheel Socket 필수 이름/존재 확인, Hardpoint/Destroyed FX optional guidance |
| 4 | Layout Capture | current Socket truth와 persisted Layout의 readiness/current/stale 판정 |
| 5 | Physics Proposal | accepted Evidence 기반 private 4 Profile typed proposal Preview/Commit |
| 6 | Gameplay Setup | Gameplay/Hardpoint/Fitting/default completeness와 USER Socket guidance |
| 7 | Final Review | fresh Resolve/Validation/Drift/전체 Diff/provenance 검토 후 explicit DefinitionApply, guarded Undo |
| 8 | Driving Test | saved Target exact benchmark, PIE transient test-drive, explicit USER Driving acceptance |

Step 상태는 현재 authoritative truth를 다시 읽어 평가한다.
Background full Resolve나 매-frame authoring refresh를 사용하지 않고 selection/step/explicit refresh/Builder mutation 같은 의미 있는 trigger에서 갱신한다.

---

## 5. Data Authoring 역할 고정

`CF-FQ-038 차량 데이터 Authoring`은 Vehicle Builder 완료 뒤에도 폐기하지 않는다.
Current 역할은 다음으로 고정한다.

### Builder Backend

```text
Recipe
Resolver
Field Registry
SourceTrace
Diff
Validation
typed Preview / Proposal
explicit Approval
Apply
Guarded Undo
External Drift
record creation
Profile authoring facade
```

Vehicle Builder는 위 Backend를 사용해 정상 제작 UX를 제공한다.

### Advanced Workspace

Data Authoring의 기존 Workspace는 다음 전문 작업을 위한 별도 화면으로 유지한다.

```text
shared/legacy Profile 유지보수
고급 Field 확인
External Drift 검토·복구
compatibility/legacy 상태 확인
세부 authoring 진단
일반 사용자 Guided 흐름 밖의 전문가 수동 작업
```

따라서 현재 책임 관계는 다음과 같다.

```text
Vehicle Builder
= 정상 신규 차량 제작 UX와 단계형 진행 owner

Data Authoring
= Builder Backend + Advanced Workspace

VehicleData
= Runtime 최종 결과 owner
```

`CF-FQ-038`의 기존 USER/Technical evidence는 `CF-FQ-040` 완료 evidence로 재포장하지 않는다.
DEL6 compatibility retirement와 UA-08 quantitative comparison 같은 CF-FQ-038 자체 잔여 항목은 별도 Paused lifecycle을 유지한다.

---

## 6. Reference → Vehicle-specific Authoring 계약

### 6.1 기본 Reference

정상 실존 차량 경로는 특정 차량의 정확한 identity를 Primary Reference로 사용한다.

가능하면 다음을 분리해 잠근다.

```text
Manufacturer
Model
Generation
Model Year
Trim
Powertrain
Transmission
Drive Layout
Market / Region
```

다른 연식·트림·시장의 수치를 조용히 혼합하지 않는다.

### 6.2 Provenance

AI proposal의 수치는 의미를 구분한다.

```text
FACT
= 공개 Reference에서 직접 확인

DERIVED
= FACT를 입력으로 결정적으로 계산

GAME_BIAS
= CarFight 플레이 목적의 의도적 조정

Unknown
= 현재 신뢰 가능한 authority가 없는 값
```

Unknown을 차급 평균이나 편의상 숫자로 숨겨 채우지 않는다.
GAME_BIAS는 USER Review 대상임을 유지한다.

### 6.3 Vehicle-specific Engine / Transmission

현재 Builder는 신규 차량을 generic compatibility 값만으로 완료 처리하지 않는다.

Drivetrain Profile은 opt-in된 vehicle-specific Transmission complete typed payload를 소유할 수 있다.

```text
Automatic / AutoReverse
Forward / Reverse Gear Ratios
FinalRatio
ChangeUpRPM
ChangeDownRPM
GearChangeTime
TransmissionEfficiency
```

Performance Profile은 opt-in된 vehicle-specific Engine TorqueCurve를 소유할 수 있다.

```text
bUseEngineTorqueCurve
EngineTorqueCurve
```

Transmission/Engine Curve review는 Evidence/method/proposal hash와 persistent Recipe receipt에 binding되며 Final Review에서 fresh 재검사한다.

---

## 7. Wheel / Mesh / Socket 책임 경계

신규 정상 차량의 Wheel Size Authority는 기존 WSA Current 계약을 그대로 사용한다.

```text
canonical shared Wheel StaticMesh Bounds
+ USER-authored Wheel_Anchor Socket RelativeScale
→ Wheel visual size
→ derived Front/Rear Wheel Radius/Width
→ VehicleData Runtime value
```

USER가 직접 수행하는 영역:

```text
Chassis/Wheel Mesh 준비
Wheel_Anchor_FL/FR/RL/RR 배치·스케일
Hardpoint Socket 배치
외형과 휠하우스에 맞는 최종 시각 판단
```

Builder가 수행하는 영역:

```text
필요 이름 안내
존재/중복/유효성 검사
current Socket/Layout 상태 표시
필요한 Reference 비교 정보 제공
```

Wheel center 자동 검출, Hardpoint 자동 배치, Mesh surface 추론은 Current Builder 범위가 아니다.

상세 Current Runtime 계약은 다음 문서를 따른다.

```text
Document/Systems/Vehicles/VehicleData.md
Document/Systems/Vehicles/VehicleRuntime.md
Document/Systems/Vehicles/WheelSync.md
```

---

## 8. Mutation / Approval / Save 안전 계약

Builder write는 기존 Authoring risk/approval 계약을 따른다.

핵심 원칙:

```text
Preview 먼저
→ exact ProposalHash / Recipe fingerprint / Target DefinitionHash / Resolver revision binding
→ USER explicit approval
→ commit/apply 직전 stale 재검사
→ mismatch면 fail-closed
```

금지:

```text
raw UObject property writer 추가
shared Profile 무단 mutation
Preview 없는 Apply
stale approval 재사용
blind automatic write retry
자동 Save / Save All
USER 승인 없는 DefinitionApply
```

Authoring operation result는 Recipe/Target/Profile/Evidence mutation footprint와 Save 여부를 명시한다.
P0 normal authoring facade는 disk Save를 자동 수행하지 않는다.

Step 7 guarded Undo는 마지막 Builder-owned exact transaction과 post-Apply current state가 일치할 때만 허용한다.

---

## 9. Driving Test / Acceptance 계약

Step 8 Technical Benchmark는 **현재 saved Target exact object path + DefinitionHash**에 binding한다.

High-speed benchmark는 expected TargetHash를 실제 saved VehicleData semantic hash와 benchmark 시작 전에 비교한다.

```text
expected != actual
→ benchmark 시작 전 fail-closed

expected == actual
→ result에 target_hash_verified=true
```

PIE test-drive는 current selected VehicleData의 transient duplicate를 active Player VehiclePawn에 적용한다.
이 transient 적용은 Product Asset 저장이나 DefinitionApply를 대신하지 않는다.

USER Driving PASS는 current Recipe + current TargetHash + current Benchmark RunId에 exact binding된 local acceptance token이다.
Target/benchmark가 바뀌면 기존 acceptance를 current로 인정하지 않는다.

---

## 10. CF-FQ-040 완료 검증 기준선

`VB-P0-09 End-to-End USER Acceptance`와 후속 ESH final audit까지 완료된 대표 Wagon 기준선은 다음과 같다.

```text
RecipeId
= 05F69DD34990A868DEFFD29C1AF5B2F9

TargetHash
= 0e5b48e8dcd39deba441da9237218be6

Step8 RunId
= 256cd822-419e-4a3e-ac23-388b570d78c2

Persisted high-speed
= Peak 206.700 km/h
= FinalGear 6
= T200 31.183 s
= target_hash_verified=true

Affected regression after final hash-preflight correction
= ESH-01 3/3 PASS
= ESH-02 5/5 PASS
= ESH-03 2/2 PASS
= ESH-01~06 Final Audit Clean PASS
```

이 값은 **CF-FQ-040 완료 당시 대표 acceptance/evidence**다.
새 차량의 목표 성능값이나 모든 차량에 복사할 tuning preset이 아니다.

새 관련 failure가 없는 한 ESH-01~06이나 Wagon 재튜닝을 Vehicle Builder 정상 사용의 선행 절차로 반복하지 않는다.

---

## 11. Current Scope Out / 후속 경계

현재 Builder가 소유하지 않는 범위:

```text
Wheel center 자동 geometry 검출
Hardpoint 자동 geometry 배치
UE Editor 내부 LLM/Web crawler
사용자 확인 없는 Save/Apply
가변 shift map Runtime 재설계
실차 전자식 최고속 limiter 강제 적용
브랜드/IP 표현 결정
CF-FQ-038 DEL6/UA-08 자체 잔여 작업
```

새 Builder 기능을 추가할 때는 이 Current 구조를 기본으로 하고, 실제 계약 변경이 필요한 경우에만 새 Feature/Plan lifecycle을 연다.

---

## 12. 관련 Current 문서

```text
Document/Systems/Vehicles/VehicleData.md
Document/Systems/Vehicles/VehicleRuntime.md
Document/Systems/Vehicles/WheelSync.md
Document/Systems/Vehicles/VehicleCoreDecisions.md
```

완료 당시 상세 설계·검증:

```text
Document/Plan/VehicleBuilder/VehicleBuilderPlan.md
Document/Plan/VehicleBuilder/VehicleBuilderRoadmap.md
Document/Plan/Archive/README.md
```

대표 Plan은 `CF-FQ-040 Done → Historical + Retained Path`로 보존한다.

---

## 13. Changelog

### v1.0.0 - 2026-09-02

- `CF-FQ-040 / VB-P0-10 Current System Promotion`으로 Guided Vehicle Builder의 실제 Shell→ViewModel→Data Authoring Backend→VehicleData 구조를 Current Systems에 신규 승격했다.
- 정상 신규 차량 제작 UX는 Vehicle Builder, 공통 Authoring 엔진과 전문가용 화면은 Data Authoring Backend + Advanced Workspace, Runtime 최종 결과는 VehicleData가 소유하도록 역할을 고정했다.
- Reference/Evidence, Builder-private Profile, Transmission/Engine Curve provenance, Preview→explicit approval, Final Review/Undo, exact TargetHash benchmark와 USER Driving acceptance 경계를 Current 계약으로 정리했다.
- VB-P0-09 USER Acceptance와 ESH-01~06 Final Audit Clean PASS를 완료 기준선으로 보존하되 Wagon 수치를 일반 차량 preset으로 해석하지 않도록 분리했다.
