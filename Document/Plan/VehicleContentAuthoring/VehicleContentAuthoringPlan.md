# CF-FQ-059 Vehicle CCAS Domain Onboarding / 차량 콘텐츠 생산 저작 체계 Plan

- 문서 버전: v0.1.55
- 최근 갱신일: 2026-10-20
- 문서 상태: Ready / **VCCAS-P0-03A + P0-03B TECHNICAL PASS (LATEST v1.1.1 EXACT3/3 + PERSISTED WAGON PROVIDER READBACK PASS)** / **VCCAS-P0-03C PARTIAL — G1/G2 COMPANY R2 SINGLE-PAIR LIVE PASS / CORE6 CREATE0/6 / G3 RECEIPT & PRODUCTION SAVE NOT ACCEPTED** / P0-02 FULL EVIDENCE HOLD
- Feature: `CF-FQ-059 Vehicle CCAS Domain Onboarding / 차량 콘텐츠 생산 저작 체계`
- Work: `wrk_d969f3af540f4f5da036a4686d425139`
- Project: `CarFight`
- Project ID: `carfight`
- Repository Role: `main_game`
- Branch: `SwitchSourceEngine`
- Official Engine: Unreal Engine 5.8 Source Build
- 상위 기준선:
  - `CF-FQ-058 CarFight Content Catalog & Authoring System` — FEATURE COMPLETE / CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE PASS
  - `Document/Systems/DataManagement/DataAssetAuthoring.md`
  - `Document/Systems/DataManagement/DataAssetManagement.md`
  - `Document/Systems/Vehicles/VehicleBuilder.md`
  - `Document/Systems/Vehicles/VehicleData.md`
  - `Document/Systems/Vehicles/VehicleRuntime.md`

---

## 1. 목적

이 Work의 목적은 현재 CarFight 프로젝트에 이미 존재하는 차량용 Mesh들을 차종별로 분류하고, 각 차종에서 대표 Mesh 최소 1개를 선정한 뒤, 그 Mesh에 가장 적합한 실제 대표 차량 1종의 공개 스펙을 최대한 반영해 **차종별 Production Vehicle 최소 1대씩**을 생성·검증·게임에서 사용할 수 있게 만드는 것이다.

이 Work는 단순한 `VehicleData → Excel` 변환 작업이 아니다.

목표 구조는 다음과 같다.

```text
현재 프로젝트의 차량 Mesh
        ↓
차종 분류
        ↓
차종별 대표 Mesh exact1 이상
        ↓
실제 대표 차량 / 연식 / 트림 선정
        ↓
실차 Reference Spec + Mesh 측정값 + 명시적 Derived/Tuned 값
        ↓
Vehicle CCAS Canonical Authoring Model
        ↓
기존 Vehicle Authoring / Validation / Apply backend
        ↓
Production Vehicle DataAssets
        ↓
Production discovery / Spawn
        ↓
Runtime 주행 기술검증
        ↓
USER Driving / Feel 검수
```

P0의 최종 상태는 **현재 보유 Mesh 기반의 각 대표 차종이 실제 게임에서 Spawn되고 정상 주행 가능한 Production Vehicle로 존재하는 것**이다.

---

## 2. 사용자 작업 모델

### 2.1 GPT가 주 Author다

CarFight는 사용자와 GPT가 함께 진행하는 1인 개발 프로젝트이며, 실제 데이터 제작은 GPT가 수행하는 것을 기본 경로로 한다.

사용자는 다음과 같이 요구한다.

```text
"현재 SUV 메시로 대표 SUV 한 대 만들어."
"이 차는 실제 대표차 스펙을 최대한 적용해."
"차종별로 한 대씩 만들어."
```

GPT는 다음을 수행한다.

```text
Mesh 조사
→ 실차 후보 조사
→ 대표차 선정 근거 작성
→ 실차 스펙 수집
→ Canonical Vehicle 데이터 작성
→ 검증
→ Production DataAsset materialization
→ Runtime 기술검증
→ 결과 보고
```

### 2.2 Excel은 USER workflow가 아니다

CF-FQ-058에서 확립된 Workbook/XLSX 기술을 재사용할 수 있으나, 사용자가 Workbook을 직접 읽거나 편집하는 것을 정상 제작 흐름으로 요구하지 않는다.

```text
USER-facing authoring source
≠ 사람이 Excel 셀을 직접 편집하는 것

Canonical authoring authority
= GPT와 authoring backend가 관리하는 구조화된 Vehicle content data

.xlsx
= 필요 시 사용하는 내부 durable/tabular representation
```

Workbook의 물리 시트명, machine column, metadata 구조는 사용자에게 이해를 요구하지 않는다.

### 2.3 Vehicle Builder UI는 필수 실행 경로가 아니다

기존 Vehicle Builder의 검증·Recipe·Profile·Resolver·Apply 자산은 보존하고 재사용한다.

하지만 GPT가 차량을 만들기 위해 Wizard UI 버튼을 순서대로 눌러야 하는 구조를 정식 authoring contract로 만들지 않는다.

목표는:

```text
GPT
 └─ Vehicle Authoring Backend
       ├─ Canonical Model / CCAS Provider
       ├─ Existing Vehicle Recipe / Profiles
       ├─ Validation
       └─ Apply / Materialization

Vehicle Builder UI
 └─ 같은 Backend를 사용하는 선택적 조회·검수·수동 authoring frontend
```

이다.

UI와 Backend가 별도 truth를 소유하지 않는다.

---

## 3. P0 First Wave 목표

### 3.1 First Wave 정의

현재 프로젝트에 존재하는 차량용 Mesh를 fresh 조사한 후, 실제 발견된 차종 분류를 기준으로 각 차종의 대표 Mesh를 최소 exact1 선정한다.

예상 분류 예시는 다음과 같지만 **현재 Asset inventory를 읽기 전에 고정하지 않는다.**

- Wagon
- Sedan
- Hatchback
- SUV
- Pickup
- Van
- Sports / Coupe
- 기타 현재 Mesh가 실제로 제공하는 차종

존재하지 않는 차종을 채우기 위해 외부 Mesh를 임의 추가하지 않는다.

**신규 USER 실행 우선순위 (13.9): 메시 변경 이전에 Core6 대표 실차 1종씩과 전장/전폭/전고/축거 실측 출처를 우선 선정하고 USER 확인을 받는다.** 세부 제원/차종/출처는 `VehicleRepresentativeSpecRoster.md v0.1.0`에 있으며 형상 적합성은 아직 승인되지 않았다.

**현재 USER 범위:** CF-FQ-059 차량 차체는 `/Game/CarFight/Vehicles/Meshes`의 **Core9 후보만 사용**한다. Legacy Police Car 및 별도 `/Game/Vehicles`의 Offroad/SportsCar를 포함한 외부 차체는 전부 제외한다. **Wagon 기존 제작 경험을 기준 사례로 참조하고, 나머지 Core8의 실제 메시 편집·소켓 배치/스케일 등은 USER가 수행**한다. GPT는 차량별 현황 점검/준비 가이드 및 준비 완료 후 제원·DataAsset 제작/검증을 담당한다. 기존 자산은 임의 삭제·수정하지 않는다. 13.5~13.8절, `VehicleMeshInventory.md v0.1.5` 및 `VehicleMeshPreparationChecklist.md v0.1.1`이 최신 범위 및 차량별 USER 인계 체계이며 13.3~13.4의 구버전 조사·중간 범위는 Historical로만 보존한다.

### 3.2 차종별 완료 단위

차종 하나의 First Wave 완료 단위는 다음 전체를 포함한다.

1. 대표 Project Vehicle Mesh
2. 대표 실제 차량의 제조사 / 모델 / 연식 / 트림
3. 실제 차량 스펙 출처
4. Mesh 자체 측정값
5. 실차 기준 Vehicle authoring payload
6. 필요한 Vehicle 관련 DataAsset
7. Production identity
8. 정상 Spawn
9. 정상 기본 주행
10. USER가 구분 가능한 차량 identity

단순 DataAsset 생성만으로 한 차종 완료라고 판정하지 않는다.

---

## 4. 실차 Reference 원칙

### 4.1 대표 실차는 Mesh에 맞춰 선정한다

실차는 먼저 정하고 Mesh를 억지로 맞추지 않는다.

순서는 다음과 같다.

```text
Project Mesh
→ 외형/비율/차종/휠베이스 추정
→ 실제 차량 후보군
→ 가장 유사하고 공개 제원이 충분한 대표차
→ 정확한 연식/트림 동결
```

대표차 선정 기준:

- 차체 형식
- 전장/전폭/전고 비율
- 휠베이스
- 타이어/휠 비율
- 구동계 형태
- 차량 용도
- 공차중량 범위
- 엔진/변속기 특성
- 공개 자료의 신뢰성과 충분성

브랜드나 성능 수치만 비슷하다는 이유로 외형이 맞지 않는 실차를 선택하지 않는다.

### 4.2 연식과 트림을 식별한다

같은 모델명도 연식/엔진/구동/변속기별 스펙이 다르므로 다음 identity를 가능한 한 고정한다.

```text
Make
Model
Generation
ModelYear
Trim
Engine
Transmission
Drivetrain
Market/Region (필요한 경우)
```

"Toyota Supra"처럼 범위가 넓은 이름만 저장하고 서로 다른 세대의 수치를 혼합하지 않는다.

### 4.3 출처 우선순위

실차 스펙은 가능하면 다음 순서의 신뢰도 높은 자료를 사용한다.

1. 제조사 공식 사양/매뉴얼/보도자료
2. 공신력 있는 차량 제원 데이터
3. 전문 매체의 해당 연식/트림 측정 자료
4. 신뢰 가능한 보조 자료

상충하는 값은 임의 평균하지 않고 출처와 조건을 비교해 선택 근거를 기록한다.

---

## 5. Vehicle 값 Provenance 계약

모든 주요 Vehicle 값은 최소 다음 provenance 중 하나로 분류한다.

### A. ReferenceSpec

실제 대표 차량에서 직접 가져온 값.

예:

- 전장/전폭/전고
- Wheelbase
- Curb Weight
- Gross Vehicle Weight
- Engine power
- Engine torque
- Maximum RPM
- Gear ratios
- Final drive
- Drivetrain
- Tire size
- Top speed
- Turning circle/radius
- Drag coefficient

### B. MeshMeasured

CarFight에 실제 존재하는 Project Mesh에서 측정한 값.

예:

- StaticMesh bounds
- Wheel socket/anchor position
- 현재 mesh visual scale
- wheel mesh radius/width
- hardpoint/socket 위치
- chassis local dimensions

### C. Derived

ReferenceSpec 또는 MeshMeasured에서 결정론적으로 계산한 값.

Derived 값은 반드시 계산 근거를 설명할 수 있어야 한다.

예:

- tire code에서 계산한 wheel radius
- gear/engine data에서 계산 가능한 보조 수치
- reference dimension과 mesh dimension의 scale relationship

### D. CarFightTuned

실차 공개 제원으로 직접 알 수 없고 CarFight/Chaos Runtime을 정상적으로 동작시키기 위해 조정한 값.

예:

- 일부 suspension/solver parameter
- 공개 제원이 없는 steering/response detail
- Chaos-specific damping/control parameter
- 안정적인 gameplay runtime을 위한 제한값

`CarFightTuned`는 실차값으로 위장하지 않는다.

### 5.1 금지

다음은 금지한다.

- 모르는 실제 스펙을 임의 숫자로 채우고 ReferenceSpec으로 표기
- 다른 연식/트림의 값을 설명 없이 혼합
- Mesh 측정값을 실차 제원처럼 표기
- 게임 체감 보정값을 실차 값이라고 기록
- 테스트를 통과시키기 위해 대표 실차 스펙을 silent mutation
- 출처가 사라져도 숫자만 남도록 provenance 폐기

---

## 6. Vehicle Domain 범위

VCCAS-P0-00에서 현재 계약을 fresh rebaseline한 뒤 정확한 provider boundary를 동결한다.

현재 조사 대상은 최소 다음을 포함한다.

### Runtime/Product Vehicle Domain

- `UCFVehicleData`
- `UCFVehicleDefenseData`
- `UCFVehicleSensorData`
- `UCFVehicleCameraData`
- 필요 시 `UCFVehicleFittingData`와의 연결 경계

### Existing Vehicle Authoring Backend

- `UCFVehicleRecipeData`
- `UCFVehicleBaseProfile`
- `UCFDrivetrainProfile`
- `UCFHandlingProfile`
- `UCFPerformanceProfile`
- `UCFDriveStateProfile`
- Vehicle Builder authoring/validation/apply service
- 기존 batch authoring/import/export contract

모든 타입을 별도 Production DataAsset으로 강제하는 것이 목적은 아니다.

P0-00에서 다음을 구분한다.

```text
Authoring-only intermediate/profile
Runtime Product
Reusable shared sub-data
Generated output
External/read-only resource
```

---

## 7. CF-FQ-058 재사용 경계

CF-FQ-059는 CF-FQ-058을 재구현하지 않는다.

재사용 대상:

- canonical content identity 원칙
- Generic Provider/Compiler 계약
- Workbook adapter
- Resource Catalog 개념
- Diff/Review
- dependency graph
- durable mutation / readback / provenance
- generated Product와 authoring authority 분리
- USER가 Workbook을 직접 관리하지 않아도 되는 backend authoring 구조

Vehicle domain 의미는 Vehicle Provider가 소유한다.

Generic Core에 다음 식의 Vehicle 전용 하드코딩을 증식시키지 않는다.

```text
if Vehicle == Wagon ...
if VehicleType == SUV ...
if VehicleField == EngineTorque ...
```

차종과 Vehicle field 의미는 typed provider/schema/authoring service에서 처리한다.

---

## 8. Production Vehicle discovery 경계

CF-FQ-058의 Equipment Production Catalog와 현재 Vehicle Runtime discovery는 동일 authority가 아니다.

따라서 CF-FQ-059 승격 시점에는 새로운 Vehicle Production Catalog를 선결론으로 추가하지 않는다.

VCCAS-P0-00에서 다음을 fresh 조사한다.

- 현재 Runtime Vehicle discovery authority
- `UCFRuntimeTestCatalogData`의 Current 역할
- existing Vehicle Builder Runtime Catalog Promotion 결과
- Product/Production vehicle을 Test Catalog에 계속 둘 수 있는지
- Vehicle용 production membership이 필요한지
- 기존 CCAS publication abstraction을 Vehicle domain에 재사용할 수 있는지

목표는 **Production Vehicle의 single discovery/membership authority**이며, 같은 Vehicle 목록을 수동으로 두 군데 유지하지 않는다.

---

## 9. Mesh Resource 경계

Workbook/Canonical Model에 StaticMesh binary 자체를 넣지 않는다.

```text
Vehicle authored row
→ stable ResourceId / typed resource reference
→ persisted Project StaticMesh
```

Resource validation은 최소 다음을 확인한다.

- resource 존재
- 허용 class
- mesh bounds
- 필요한 wheel/body socket 존재
- 현재 Vehicle authoring contract가 요구하는 semantic socket
- 중복/잘못된 resource binding

Project Asset의 실제 저장 상태는 AssetDump persisted evidence를 우선한다.

---

## 10. P0 Gate

### VCCAS-P0-00 — Fresh Rebaseline + Design Review

목표:

- CF-FQ-058 final contract fresh read
- Vehicle Builder/Recipe/Profile/Apply current backend fresh read
- VehicleData/Defense/Sensor/Camera/Fitting current runtime contract fresh read
- Vehicle discovery/catalog current authority 확인
- Vehicle CCAS typed provider boundary 설계
- GPT direct authoring / Builder optional frontend 경계 확정
- Workbook hidden/internal workflow 확정
- P0 blocker review

완료 조건:

```text
P0 blocker exact0
blocking P1 exact0
Vehicle Domain ownership map frozen
First Wave workflow frozen
```

### VCCAS-P0-01 — Vehicle Mesh Inventory + Category Roster

목표:

- 현재 CarFight Project 차량용 Mesh 전수조사
- test/legacy/source-art/product 후보 분류
- 차종 분류
- 차종별 대표 Mesh exact1 이상 선정
- Mesh별 bounds/socket/wheel evidence 확보
- First Production Vehicle roster 초안

중요:

차종 목록과 exact count는 이 단계의 fresh inventory 결과로 결정한다.

**2026-10-13 USER override:** 현 에셋은 임시 메시이므로 기존 `bounds/socket/shape dedupe` 완료를 Reference Research 시작의 선행조건으로 두지 않는다. P0-01에서 CarFight Core9 9/9 class 검증 및 USER 지정 category roster 확보는 끝났으며, physical input 검수는 최종 교체 메시가 준비될 때 P0-04/06 direct dependency gate로 이관한다. 자세한 현행 결정은 13.10절이 authority다. 기존 historical P0-01 acceptance 증거는 삭제하지 않는다.

### VCCAS-P0-02 — Real Reference Vehicle Selection + Spec Provenance

**USER-deferred temporary mesh의 외형·치수와 독립된 category/vehicle reference track으로 실행한다** (13.10절). 대상 카테고리는 현재 Core6 USER 명부 기준이며 교체 실물 메시와의 final Resource binding은 후속으로 둔다.

- 실제 대표차 후보 조사
- 대표차 exact1 선정
- 연식/트림 동결
- 핵심 실차 제원 수집
- source provenance 기록
- missing spec 분리
- ReferenceSpec / MeshMeasured / Derived / CarFightTuned 분류

사용자에게 모든 수치를 개별 입력시키지 않는다.

대표차 선정에서 의미 있는 선택지가 여러 개이고 게임 identity에 영향을 크게 줄 때만 사용자 결정이 필요한 항목을 명시한다.

### VCCAS-P0-03 — Vehicle Canonical Schema + Typed Provider

목표:

- Vehicle authoring canonical schema
- Vehicle resource references
- profile/sub-data ownership
- reference provenance fields
- typed validation
- current Vehicle Builder backend bridge
- UI-less GPT authoring entry
- diff/readback 계약

기존 VehicleData authority와 겹치는 second writer를 만들지 않는다.

### VCCAS-P0-04 — Production Vehicle Materialization + Publication

목표:

- canonical Vehicle authored content에서 Production Vehicle DataAsset 생성/갱신
- 필요한 related DA 생성/재사용
- durable save/readback
- Production membership/discovery 연결
- overwrite/drift/fail-closed 계약

### VCCAS-P0-05 — First Wave Batch Authoring

VCCAS-P0-01에서 확정한 차종마다 대표 Production Vehicle을 최소 exact1 제작한다.

한 번에 전부 blind mutation하지 않는다.

권장 batch:

```text
Wave A: representative exact1으로 end-to-end proof
→ review
→ Wave B: remaining categories
```

Wagon에서 이미 축적한 실차 기반 제작 경험과 Vehicle Builder validation을 재사용한다.

### VCCAS-P0-06 — Runtime Technical Acceptance

각 First Wave Vehicle에 대해 최소:

- Production asset existence/readback
- discovery exact membership
- spawn
- chassis/wheel materialization
- engine/transmission configuration
- basic forward/reverse
- steering
- mass sanity
- fatal validation/log error 0
- existing equipment/fitting boundary regression 없음

을 기술 검증한다.

시각 품질와 주행감은 USER acceptance와 분리한다.

### VCCAS-P0-07 — USER Vehicle Acceptance

사용자가 각 대표 차종을 실제로 운전하여 최소 다음을 확인한다.

- 차종 외형 identity가 맞는가
- 차량 크기/휠 배치가 심하게 어색하지 않은가
- 가속/최고속도/변속 체감이 대표 실차 의도와 명백히 충돌하지 않는가
- 조향/차체 거동이 해당 차종으로 수용 가능한가
- 카메라/충돌/무장 시스템과 기본 게임 플레이가 가능한가

USER feel correction이 필요하면 실차 ReferenceSpec을 직접 훼손하기보다 `CarFightTuned` 값을 중심으로 교정한다.

### VCCAS-P0-08 — Current System Promotion + Closure

완료 조건:

- First Wave 차종별 Production Vehicle exact1 이상
- canonical authoring authority active
- provenance 보존
- Production discovery active
- technical runtime acceptance PASS
- USER representative driving acceptance PASS
- current Systems 문서 동기화
- UDS Closed/Retired successor
- FeatureQueue Done disposition

---

## 11. P0 완료 기준

CF-FQ-059 P0 Complete는 다음을 모두 만족해야 한다.

1. 현재 프로젝트 Vehicle Mesh inventory를 기준으로 차종 roster가 확정되어 있다.
2. 각 First Wave 차종에 대표 Mesh가 존재한다.
3. 각 대표 Mesh에 실제 대표 차량 exact1이 결속되어 있다.
4. 대표 차량의 연식/트림과 핵심 스펙 출처가 추적 가능하다.
5. 실차값/메시측정/파생값/CarFight 조정값이 구분된다.
6. GPT가 Vehicle Builder UI나 Excel 수동 편집 없이 차량을 authoring할 수 있다.
7. Vehicle authoring 결과가 기존 Vehicle Runtime authority와 호환되는 Production DataAsset으로 materialize된다.
8. Production Vehicle discovery가 single authority로 동작한다.
9. 각 First Wave Vehicle이 게임에서 Spawn된다.
10. 각 First Wave Vehicle이 기본 주행 technical PASS를 만족한다.
11. USER가 대표 차종을 실제 주행 검수한다.
12. 기존 Wagon/Vehicle Builder/RuntimeApply/Fitting 계약을 불필요하게 재구현하지 않는다.

---

## 12. Non-goals / 보호 범위

이번 P0의 목표가 아닌 것:

- 사용자가 Excel Workbook을 이해하거나 직접 편집하도록 만드는 것
- Excel UI 개선 자체
- Vehicle Builder Wizard UI 재설계
- 모든 Vehicle DA를 무조건 Workbook으로 평탄화
- 모든 CarFight DataAsset을 한 번에 CCAS로 이전
- 실차 스펙과 무관한 임의 게임 밸런스 차량 대량 생성
- 새 외부 Vehicle Mesh 수집
- 궤도/호버/오토바이 등 현재 Runtime이 지원하지 않는 신규 locomotion architecture를 억지로 P0에 포함
- existing Wagon acceptance를 처음부터 다시 수행
- CF-FQ-058 Weapon Production pipeline 재구현
- Production Test를 통과시키기 위한 기존 Vehicle/Weapon balance silent mutation

---

## 13. 검증 전략

### 문서/설계 단계

- Git diff 검수
- current Systems/Source cross-check
- stale reference search
- sibling authoring path search

### Source 변경 단계

- 공식 UE 5.8 Editor Build
- 관련 DataAuthoring/Vehicle focused Automation
- affected regression
- persisted generated asset evidence

### Asset 단계

Persisted truth:

```text
AssetDump
```

Current Editor/PIE runtime truth:

```text
GoPyMCP UE MCP / RuntimeRead
```

사람 판단:

```text
USER PIE driving / visual / feel acceptance
```

---

## 13.1 VCCAS-P0-00 Fresh Rebaseline / Pre-Implementation Design Review — 2026-10-12

### 검수 범위와 현재 구현 증거

- Project Registry Primary: `D:\\Work\\CarFight_git` / `SwitchSourceEngine` / `checkout_eefe2944e0954bed9270908eb95d8fd0`. 검수는 read-only Source/Systems/UDS/AssetDump에 한정했다.
- Canonical Work `wrk_d969f3af540f4f5da036a4686d425139`는 parent 없는 단일 initial record `ver_bc12a4a25a804674842f377cffcf93cd`이고 `derived/heads.json`과 `Current.md`는 `Ready / VCCAS-P0-00`로 일치한다.
- CF-FQ-058 CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE / COMPLETE와 기존 canonical `Authoring/Content/CarFight_Content.xlsx` managed exact42, generated Product exact34 / published Equipment exact8은 **기존 검증 기준선**이다. 이 숫자는 Vehicle Production acceptance 숫자가 아니다.
- `CFContentProvider.h`: provider별 schema, import/current snapshot, validation, dependency, diff, reviewed apply/readback seam. `CFContentCompiler.h`: read-only preview/hash/drift; `CFContentResource.h`: stable ResourceId 및 capability. `CFContentPilot.h`: Vehicle Recipe/AssetReader **read-only pilot**. 이를 실제 Vehicle Product writer로 오인하지 않는다.
- `CFVehicleAuthoringService.h`: Recipe/Profile/Evidence/Resolver/Preview/Approval/DefinitionApply facade. `CFVehicleApplyService.h`: **Target UCFVehicleData 독점 writer**, Target과 Recipe AppliedState transaction은 수행하되 package Save는 하지 않는다. `CFVehicleUXTypes.h`의 기존 two-record Create는 `CallerKind = SlateUI/Automation` 계약이며 AI 직접 호출은 현재 허용되지 않는다.
- `CFVehicleRefEvidence.h`: 기존 Editor-only `UCFVehicleRefEvidence`는 제조사/모델/세대/연식/트림/파워트레인/구동/지역, citation, FACT/DERIVED/GAME_BIAS claim, conflict, unknown, fingerprint를 소유한다. 네 가지 CCAS provenance를 별도 경쟁 Evidence DB로 복제하지 않는다.
- `CFVehicleData.h`: `UCFVehicleData`는 차체/휠, layout, hardpoint/mount, movement, base/gross mass, default Defense/Sensor 및 선택 Camera presentation 등 Runtime root이다. 방어/센서/카메라/피팅은 각 기존 typed authority와 연결한다. 기존 `UCFVDAValidator` 및 `ACFVehiclePawn::ApplyVehicleDataConfig()`를 보존한다.
- `CFRuntimeTestCatalogData.h` / `CFRuntimeVehicleApply.cpp` / `RuntimeApply.md`: **Vehicle 후보는 `UCFRuntimeTestCatalogData.AllowedVehicleData` exact membership**. `CFVehicleCatalogPromoService`는 Editor에서 reviewed persistent VehicleData를 idempotent 등록하지만 Save는 호출하지 않는다. Equipment published exact8의 Production Publication Catalog는 별도 Equipment authority다.
- Persisted AssetDump `/Game/CarFight/Vehicles` discovery는 index 기준 31개를 반환하며 그중 Body StaticMesh `Meshes/SUV/SUV`, `Meshes/Wagon/Wagon`과 wheel meshes, 여러 차종별 MaterialInstance가 보인다. **MaterialInstance 폴더 이름만으로 차체 Mesh가 있다고 단정하지 않는다.** `/Game/CarFight/_Legacy`에도 차량 Mesh가 확인되므로 이 결과를 전체 Vehicle inventory나 First Wave 차종 exact count로 사용하지 않는다. P0-01에서 broader exact persisted inventory와 bounds/socket evidence를 fresh 확보한다.

### 현행 책임 경계 — 보존 대상

| Domain | 현재 authority | CF-FQ-059의 허용 확장 |
| --- | --- | --- |
| CCAS 공통 | `FCFContentProviderRegistry`, Compiler, Resource Catalog, Change/Review/Cutover | Vehicle typed provider와 기존 공통 알고리즘 연결; 공통 Core에 차량 field 분기 추가 금지 |
| Vehicle authoring | Recipe / 5 Profile / `UCFVehicleRefEvidence` / `FCFVehicleAuthoringService` / `FCFVehicleApplyService` | UI-less reviewed orchestration/adapter; Target writer 복제 금지 |
| Runtime root | `UCFVehicleData` / `UCFVDAValidator` / VehiclePawn | 매핑 검증 후 Product materialize; Runtime 필드의 병렬 Source of Truth 금지 |
| Related typed DA | Defense, Sensor, Camera, Fitting 각 현재 타입/검증/사용처 | 필수/선택/기존 공유 참조를 구분. 모든 타입을 신규 생성 또는 Workbook flatten 강제 금지 |
| Vehicle discovery | `UCFRuntimeTestCatalogData.AllowedVehicleData` + `FCFVehicleCatalogPromoService` | 우선 기존 Runtime source를 단일 노출 경로로 사용; 새 Vehicle Publication authority는 필요성과 전환/호환성 검증 전 추가 금지 |
| Runtime apply | `FCFRuntimeVehicleApplyService` + Pawn/Fitting | candidate authorization 뒤 기존 same-Pawn Apply/recovery 재사용 |
| Physical resource | persisted Project StaticMesh + sockets + Vehicle AssetReader / CCAS Resource Catalog | Stable ResourceId로 참조하고 capability/bounds/socket 검증. Binary를 Workbook에 복제하지 않음 |

### 검수 판정

`P0 blocker = 0 / blocking P1 = 6 / P2 = 2`. **DESIGN REVIEW HOLD / IMPLEMENTATION NOT OPENED**. 방향 폐기 사유는 발견하지 않았으나 아래 P1이 해결되기 전 `VCCAS-P0-00 PASS`, ownership freeze 및 P0-01 전진을 선언하지 않는다.

#### Blocking P1 — Correction requirement

1. **P1-01 Vehicle Provider와 field ownership**: `ICFContentProvider`의 Vehicle Product production implementation은 기존 read-only pilot과 구분해 설계해야 한다. `UCFVehicleData` 각 필드와 Recipe/Profile/Evidence/Defense/Sensor/Camera/Fitting을 `CCASManaged / Builder-owned / shared external / generated`로 분할하고, 한 필드에 CCAS와 Builder가 동시에 authoritative write하지 않도록 matrix와 fingerprint scope를 명시한다. 현재 기존 `VehicleDefenseData` JSON Staging provider의 별도 등록/권한과도 혼동하지 않는다.
2. **P1-02 UI-less trusted GPT authoring admission**: 현재 `FCFVehicleRecordCreateRequest.CallerKind`는 SlateUI/Automation 한정이다. AI라고 위장하거나 기존 approval guard를 우회하지 않고, reviewed typed operation과 exact identity/role/permissions/approval scope를 재사용하는 별도 허용 경계를 정의한다. UI는 선택적 frontend이지 정상 GPT 실행의 필수 요소가 아니다.
3. **P1-03 multi-asset materialization + durable save**: `CreateVehicleRecords`는 Definition+Recipe만 생성하며 `ApplyResolvedVehicle`은 dirty transaction만 만든다. Evidence/Profiles/related DA/VehicleData/Runtime Catalog의 생성·Apply·Save·disk reload·fingerprint·partial-failure/recovery·rollback 불가능 구간을 단계별로 명시한다. 기존 Target writer/CCAS cutover store를 재사용하되 Editor package persistence 없이 Production 완료 판정 금지.
4. **P1-04 Reference provenance adapter**: `ReferenceSpec/MeshMeasured/Derived/CarFightTuned`를 기존 Evidence의 `FACT/DERIVED/GAME_BIAS`와 정확히 매핑한다. `MeshMeasured`는 실차 citation이 아닌 persisted Mesh measurement 별도 claim/evidence source로 구분하고, field별 reference identity/claim/citation/unit/formula/revision/unknown/conflict 및 canonical `UCFVehicleData` target field mapping을 동결한다. 타 트림 혼합과 silent tuning 거부.
5. **P1-05 single Production Vehicle discovery**: 현재 `RuntimeTestCatalog`가 차량 후보 유일 source라는 사실을 보존할지, explicit reviewed migration으로 별도 Vehicle Production publication을 승격할지 결정해야 한다. First Wave 기본안은 기존 `AllowedVehicleData`와 `FCFVehicleCatalogPromoService`를 재사용하고 persisted membership을 증명하는 것. Equipment `UCFProdEquipCatalogData`를 Vehicle provider로 곧장 사용하거나 같은 목록을 두 군데 유지하지 않는다.
6. **P1-06 canonical Workbook/Builder cross-authority drift**: 기존 `CarFight_Content.xlsx`는 CF-FQ-058 managed exact42의 sole authoring authority다. Vehicle가 추가될 때 사용 Workbook identity, `ContentTypeId/ContentId`↔VehicleData/Recipe/Asset path, `WorkbookSemanticHash` + Recipe/Evidence/Target/Resource fingerprint, ownership drift, explicit review/cutover admission, legacy unmanaged preservation을 결정한다. CCAS-managed 필드에서 raw DA 편집을 조용히 덮어쓰지 않는다.

#### Non-blocking P2 / 다음 Gate에 대한 범위

1. **P2-01 First Wave category exact count deferred**: `/Game/CarFight/Vehicles` 단일 root의 31 indexed assets를 전수조사로 오인하지 않는다. P0-01에서 Project의 Legacy/Tests/Production 경로와 StaticMesh/Blueprint/Material을 분류한 뒤 실제 존재하는 대표 차종만 확정한다. Chassis geometry, wheel/socket completeness, 사용자 소켓 배치가 필요한 사례는 first-wave eligibility/blocker로 명시한다.
2. **P2-02 Reference/spec presentation과 driving acceptance**: 사용자는 셀/필드 입력 작업자가 아니다. Vehicle reference candidate 선택 근거, 누락/보정 근거와 USER 주행 수락 정보를 읽기 쉬운 요약으로 제공하되 중복 Excel UI를 만들지 않는다.

### Correction 뒤 필수 Fresh Design Re-review

- 위 P1 exact6별 ownership·API·persistence·migration·test precondition이 서로 모순 없이 frozen되었는지 재검수한다.
- `Source → Review → Recipe/Profile/Evidence → existing Apply → durable Asset Save/readback → Vehicle Catalog membership → Runtime Apply`에 대해 exact 승인/실패 경계와 소비자·테스트·fixture 영향을 추적한다.
- Body Mesh identity, wheel/sockets, measured geometry는 persisted AssetDump 근거를 우선하며 missing readiness는 추정값으로 우회하지 않는다.
- 통과 기준: `P0=0 / blocking P1=0 / Vehicle Domain ownership map frozen / First Wave execution workflow frozen`; 이때에만 `VCCAS-P0-01`을 연다.
- 본 검수 수행 중 Source·UE Asset·Workbook·Build·Automation·PIE 변경은 **0**. 기존 Dirty와 독립 저장소 submodule은 보존한다.

---

## 13.2 VCCAS-P0-00 — P1×6 Contract Correction / Frozen Design v0.1.2 (2026-10-12)

이 절은 13.1의 **당시 HOLD 평가를 삭제하지 않고 교정하는 최신 설계 계약**이다. 아래의 API/Writer/패키지 저장 경로는 **후속 구현 목표**이며, 현재 이미 구현되어 동작한다고 주장하지 않는다. 이 단계에서 C++/UE Asset/Workbook/Runtime mutation은 0이다. 2026-10-12 재검수는 설계 정합성만 검증한다.

### 13.2.1 C1 — Vehicle Provider / field ownership (P1-01)

**계층을 3개로 분리**한다.

1. **Authoring truth**: CCAS-managed 대상으로 승인·cutover된 차량의 **선언적 필드 값과 명시적인 reference/provenance**는 기존 canonical `Authoring/Content/CarFight_Content.xlsx`의 `vehicle` type record가 소유한다. 미승격 차량은 기존 Recipe/Profile/Builder authority를 유지한다.
2. **Typed interpretation / writing authority**: Vehicle Provider는 `ICFContentProvider`의 schema/validation/diff/dependency/readback을 구현하고, 필드 뜻·source 우선순위는 기존 Vehicle `FieldRegistry → Recipe/Profile → SnapshotBuilder → Resolver → Validator`에 위임한다. `UCFVehicleData` 변경은 **`FCFVehicleAuthoringService → FCFVehicleApplyService` 단일 Target writer**만 사용한다. Provider가 raw reflection setter 또는 별도 Target writer를 만들지 않는다.
3. **Runtime materialization**: persistent `UCFVehicleData`가 차량별 runtime root다. `ACFVehiclePawn`과 `UCFVDAValidator`/Fitting Runtime의 해석·검증 의미는 변경하지 않는다.

| Field group / 자산 | 선언적 소유권 (CCAS-managed 신규 차량 기준) | 실제 작성·적용자 | 금지/예외 |
| --- | --- | --- | --- |
| Stable vehicle identity, selected chassis/wheel `ResourceId`, representative real-car identity | Vehicle CCAS record | CCAS Resource resolver → existing Recipe AssetIntent / Vehicle Authoring | StaticMesh binary나 Runtime identity를 Workbook에 새로 저장하지 않음 |
| `VehicleVisualConfig`, `VehicleLayoutConfig`, WheelAnchor pose/scale, wheel visual/physics source | 선정 ResourceId와 채택된 측정/Scale intent는 CCAS; **현장 Mesh Socket/geometry 및 사용자 배치 결과는 Builder/Asset-owned** | 기존 AssetReader/Measurement approval/Recipe/Resolver/Apply | Asset socket 자동 생성·무승인 이동 금지. Socket scale > reference-wheel fallback의 기존 precedence 유지 |
| `VehicleMovementConfig`, Drivetrain/Handling/Performance/DriveState/Profile fields | 명시적으로 schema에 등록된 `CCASManaged` 값 및 provenance만 CCAS; 나머지는 `ExternalManaged(VehicleBuilder)` | 기존 5-domain profile, Recipe, Resolver, `FCFVehicleApplyService` | Provider의 arbitrary field write/미정값 채우기 금지 |
| `BaseVehicleMassKg`, `MaximumGrossMassKg`, `ReferenceMaxSpeedKmh` | 해당 값이 필드별로 승인된 경우만 CCASManaged; 실차/측정/튜닝 근거 별도 명시 | VehicleBase/Performance/accepted driving-evidence 계약 → Resolver/Apply | Curb Weight를 Base Mass, GVWR을 Max Gross로 **검토 없이 등치하지 않음**. `ReferenceMaxSpeedKmh`는 물리 최고속도 제한이 아니며 현행 accepted driving/DefinitionHash 근거를 무시하지 않음 |
| `HardpointSlots`, `MountProfiles`, 수동 socket location, equipment default | 기본 `ExternalManaged(VehicleBuilder)`; explicit reviewed domain onboarding 전에는 CCAS가 수정하지 않음 | 기존 Builder semantic intent / fitting validation | Editor에 저장된 사용자 Hardpoint/socket 임의 재배치 및 고정 Weapon 기본 장착 추론 금지 |
| `DefaultDefenseData`, `DefaultSensorData`, 선택 `CameraPresentationDataOverride` 등 | **Vehicle record는 기존 shared/sub-data Asset reference binding만 소유**할 수 있음. 하위 DA payload는 기존 개별 typed owner | 기존 각 DA provider/validator, Vehicle Authoring의 typed binding/Apply | 모든 서브 DA 자동 생성·복제·Workbook flatten 금지. Sensor Basic/fallback, Camera Aim와 Presentation 분리 보존 |
| `UCFVehicleFittingData` / Fitting Snapshot / gross validation / equipment selection | 차량 정의/허용 슬롯의 참조만 Vehicle; 실제 sortie equipment/fitting state는 외부 runtime authority | 기존 `FCFRuntimeVehicleApplyService`, Fitting Snapshot/Prepare/Commit/Recovery | Vehicle Provider가 장비·질량의 별도 Runtime 적용 경로 생성 금지 |
| `UCFVehicleRefEvidence`, private/shared profiles, `UCFVehicleRecipeData` | CCAS가 승인한 내용의 **projection/receipt**와 기존 Builder 외부 소유 필드를 구분 | 기존 Evidence/Recipe/Profile typed service와 승인·version guard | 동일 field에 두 authoring writer 허용 금지; 컷오버 뒤 CCASManaged direct Builder edit fail-closed |

**필드별 단일 authority 규칙:** `FCFContentSheetDescriptor`/provider-local stable field schema에서 `CCASManaged`, `ExternalManaged(VehicleBuilder)`, `ExternalReadOnly`, `GeneratedProjection` 중 exact1을 소유한다. 복합 struct/collection은 field-path 또는 stable child key까지 세분화해 전체 struct 덮어쓰기 금지. `CCASManaged`는 `Vehicle Writer`가 없어졌다는 뜻이 아니라 **write intent의 근원만 CCAS**라는 뜻이다. Builder 직접 편집이 필요한 field는 별도 source/ChangeSet route로 승인하지 않는 한 CCAS-managed 집합에 넣지 않는다.

**Sub-provider 소유권:** 기존 JSON Staging `VehicleDefenseData`의 `CFDATypeDispatch`/DACE/durable registry와 상위 CCAS `ICFContentProvider`는 다른 층이다. 하위 typed DA 생성이 필요할 때는 기존 provider operational admission을 별도로 검증하고 adapter로 호출한다. Sensor/Camera/Fitting이 기존 DA provider에 자동 등록됐다고 가정하지 않는다.

### 13.2.2 C2 — Trusted GPT UI-less authoring admission (P1-02)

`ECFAuthoringCallerKind::AI`는 이미 존재하지만 **diagnostic identity이며 권한 증표가 아니다**. `FCFVehicleAuthoringService::PreviewVehicleRecords()`는 현재 `SlateUI/Automation`만 허용한다. 이 보호를 AI를 Automation이라고 속여 통과하지 않는다.

후속 P0-03의 **Vehicle-domain UI-less orchestration adapter**는 기존 Public `FCFVehicleAuthoringService`를 사용하고, 다음 타입화된 입력을 요구한다.

```text
Project identity = carfight / main_game / exact Checkout
Source = canonical Workbook candidate + typed Vehicle ContentKey
Target = exact persistent /Game/... VehicleData + Recipe path
Requested action = Read / Review / R1 Recipe / R2 Create / R3 DefinitionApply / Publish
Authority = capability/approved-scope from the actual managed caller
Approval = current immutable ReviewPackageFingerprint + ManifestFingerprint
Preconditions = Workbook hash + Catalog snapshot + Provider schema
              + RecipeId/revision/fingerprint + target DefinitionHash
              + Reference Evidence fingerprint + Resource capability fingerprint
Outcome = stable error code + changed package list + bSavePerformed + persisted readback
```

- 기존 `R0/R1/R2/R3` 위험도와 `AuthoringWrite/OwnershipWrite/DefinitionApply` 승인 종류를 보존한다. AI는 각 operation의 **해당 승인** 없이는 생성·적용할 수 없다. 승인된 reviewed batch scope 밖의 asset/field는 거부한다.
- **R2 Create**는 `PreviewVehicleRecords/CreateVehicleRecords`의 **정식 trusted AI route를 후속 구현에서 additive로 열어야 한다.** 이때 existing collision/type check, `bRequireVehicleSpecificTransmission`, `bRequireExplicitHardpointPlan`, proposal hash, exact approval/fresh replay를 재사용한다. UI/Automation 동작을 완화하거나 새 raw `NewObject` writer로 복제하지 않는다.
- R1은 기존 `PreviewRecipeChange/CommitRecipeChange`, Evidence refresh, private Profile preview/commit를 재사용하고 R3은 `BuildApplyApprovalProposal/ApplyResolvedVehicle`를 재사용한다. 기존 Builder Step 버튼/Slate를 호출하지 않는다.
- 적법한 managed caller capability가 없거나 새 P0-03 API/admission이 미구현이면 `UnsupportedOperation / ApprovalRequired`로 **fail-closed**한다. 현재 GPT가 완전한 AI R2 production mutation을 할 수 있다고 주장하지 않는다.
- UI는 동일 backend의 선택적인 조회·수동 검수 frontend다. Workbook 셀 편집, Slate Wizard 조작, 임의 셸 실행, Raw Asset write가 필수 경로가 되어서는 안 된다.

### 13.2.3 C3 — Multi-asset materialization, durable save, recovery (P1-03)

현재 `CreateVehicleRecords`는 Definition+Recipe 두 UObject를 dirty로 만들 뿐이고, `FCFVehicleApplyService`와 `FCFVehicleCatalogPromoService`도 **Save를 수행하지 않는다**. 따라서 저장은 상위 Vehicle Production transaction orchestrator가 별도로 맡아야 한다. 단일 UObject transaction을 cross-package atomic commit으로 해석하지 않는다.

**검수 중 발견한 identity 의존 순환 교정:** 기존 `FCFVehicleRecordCreateRequest`의 R0 Preview는 `RecipeId`를 발급하지 않고, 실제 `CreateVehicleRecords` 후에 persistent `RecipeId`를 읽을 수 있다. 그런데 CCAS Vehicle `ContentId`는 해당 RecipeId를 요구한다. 그러므로 **ContentKey를 포함하는 최종 CCAS ReviewPackage를 R2 creation 전 생성했다고 주장하면 안 된다.** S0의 독립된 R2 생성 승인 → S1의 미저장 provisional pair → S2의 최종 CCAS review 순서로 정확히 분리한다. S1에서 Work/Package state 변화가 생기므로 S2는 이를 반영한 새 snapshot/approval로 다시 계산한다.

**순서 (후속 P0-04 구현 목표; Write 전까지 dry-run/review만):**

```text
S0: Canonical Workbook + current Resource/Target/Existing Recipe/Catalog
    read-only truth 검사 + candidate package path/mesh + R2 two-record creation
    proposal 생성 및 exact OwnershipWrite 승인 (아직 Workbook manifest 미확정)
S1: **Approved R2 Definition+Recipe Create** (non-saved/inactive candidate)
    → 실제 RecipeId/Target path readback, candidate operation journal에 결속
S2: 그 RecipeId로 vehicle_<RecipeId> ContentKey 확정 → validated AI ChangeSet
    → immutable ReviewPackage/Manifest + full stale re-preflight 승인
    → 미승인 또는 실패면 unsaved candidate는 unpublished/RecoveryRequired
S3: Approved R2 Evidence/private Profile 생성 또는 검증된 reuse → Reviewed
    R1 Recipe/Profile/Evidence commit → fresh Resolver → R3 DefinitionApply
    → UCFVDAValidator / linked dependency validation
S4: Dependency-first exact package Save (evidence/profiles/related DA first,
    Recipe + Target after) → disk reload / semantic readback per package
S5: Existing CCAS staged Workbook promotion only after **all Product readbacks**
    and approved desired fingerprint exact match
S6: Existing FCFVehicleCatalogPromoService idempotent membership update
    → Catalog exact-package Save / disk reload / duplicate0 / exact1 readback
S7: Publication + final Vehicle/Recipe/Resource/provenance consistency readback
    → transaction Verified. No automatic runtime apply or UE play.
```

- **Package scope**: `/Game` persistent exact paths, expected old state/new identity, referenced asset set, Recipe/Evidence/profile owner binding, writer & package save ownership을 manifest에 명시한다. No implicit Update of shared Profile, existing managed Weapon, global defaults, unrelated dirty package. 새 Vehicle Profile 5-domain binding은 현재 Builder의 4 private + optional shared/DriveState contract를 그대로 사용한다; 무조건 신규 Profile exact5로 가정하지 않는다.
- **Pre-save**: old/new collision, Recipe target binding, source provenance, semantic field diff, User Socket readiness, `UCFVDAValidator`, gross mass and Basic Sensor fallback, camera/aim separation 확인. Mutable unsaved user-owned package와 충돌하면 중단한다.
- **Durable semantics**: 기존 `CFDADurableCore`/CCAS cutover의 per-target save/readback/uncertainty taxonomy와 journaling 패턴은 *검증 후 재사용*한다. **CF-FQ-058의 생산 전용 일회성 cutover entry는 Current normal operation에서 제거돼 read-only `ProductionCutoverVerification`으로 대체되었다. 이를 상시 writer로 재가동하지 않는다.** `FCFContentCutoverCoordinator`의 generic contract는 실제 Vehicle onboarding extension에 맞게 재검증한 뒤 사용하며, 신규 Vehicle 승인·저장 호출부는 별도 domain orchestration이 소유한다. `FCFVehicleApplyService`의 Target writer를 새로운 durable core가 직접 대체하는 것은 금지. 현재 exact multi-package Vehicle orchestrator는 **구현되지 않았다**.
- **S1/S2 거부·취소**: 임시 Definition+Recipe는 아직 disk-saved/Product-published 상태가 아니다. S2 승인 실패 시 해당 후보를 `UnpublishedCandidate / RecoveryRequired`로 표시하고 다음 동일 경로 작업을 차단한다. Cleanup은 생성 transaction의 exact ownership과 다른 참조/편집 없음이 증명된 경우에만 별도의 approved Undo/abort 경계로 처리하며, 임의 force delete/unsaved discard 금지.
- **절대 원자성 과장 금지**: 서로 다른 `.uasset` Save 도중 실패하면 이미 저장된 package는 자동 삭제·되돌리기·재시도하지 않는다. durable journal은 `Prepared / PartiallyApplied / ProductReadbackVerified / WorkbookCommitted / PublishedVerified / RecoveryRequired`를 구별하고, crash/SaveStateUnconfirmed 시 현장 disk truth를 재조사해 명시적 동일 승인 scope 복구만 허용한다. 불확실한 저장은 성공으로 표시하지 않는다.
- **Publish ordering**: S4의 저장 성공만으로 Vehicle Catalog에 노출하지 않는다. S5 이후 S6에서만 게시한다. S5 성공·S6 실패이면 authoritative Workbook은 보존하고 `RecoveryRequired / unpublished`로 남겨, 현재 runtime 후보 노출과 완료 판정을 금지한다. S6의 Catalog 단독 저장 성공도 S7까지 First Wave 완료가 아니다.
- **Idempotency**: 승인 manifest + expected pre-state와 actual persisted fingerprints가 exact 같으면 no-op/AlreadyCurrent. 예상 외 외부 변경은 Fresh Review를 요구하고 silent rebasing/overwrite/AutoRecovery를 거부한다. Retire와 물리 삭제/이동은 분리하며 P0-04 정상 실행의 물리 delete/rename/move = 0.

### 13.2.4 C4 — Provenance bridge (P1-04)

하나의 값을 `ReferenceSpec / MeshMeasured / Derived / CarFightTuned`로 분류하는 것은 **CCAS Vehicle record의 값 출처 분류**이고, 기존 `UCFVehicleRefEvidence`는 **실차 research claim과 출처의 authoritative companion**이다. 서로 다른 Evidence DB를 신설하지 않는다.

| Vehicle value provenance | Current owner/receipt | 필수 결속 |
| --- | --- | --- |
| `ReferenceSpec` | `UCFVehicleRefEvidence`의 accepted `FACT` / `FCFRefClaim` | Exact primary `ReferenceVehicleId`, Make/Model/Generation/ModelYear/Trim/Powertrain/Market, `ClaimId`, `CitationIds`, `SourceValueText`, canonical unit, claim resolution, EvidenceFingerprint |
| `MeshMeasured` | 기존 Vehicle AssetReader / accepted Mesh measurement receipt (실차 FACT와 **별개**) | Exact `ResourceId` + static mesh object path + persisted asset/capability fingerprint + socket/bounds/scale measurement method/unit |
| `Derived` | existing `DERIVED` claim (real-reference-derived) 또는 Vehicle Provider-local deterministic derivation (mesh-derived) | input claim/mesh measurement IDs, method ID + revision + parameters, rounding/unit policy, canonical output; input 변경 시 stale |
| `CarFightTuned` | 기존 Builder tuning approval / `GAME_BIAS` 성격의 변경 기록 | Tuning proposal/decision ID, previous value, expected reference/mesh provenance, reason, reviewer scope, target DefinitionHash, explicit accepted version |

- **실차 `FACT`와 `MeshMeasured`는 같은 종류가 아니다.** `MeshMeasured`를 citation 없는 `FACT`로 위장하거나 제조사 웹 출처로 포장하지 않는다. 필요한 경우 existing measurement receipt를 provider-local typed reference로 연결한다.
- 기존 `ValidateInitialResearchPayload`는 initial research 단계에서 `GAME_BIAS`를 허용하지 않는다. 튜닝값은 **research 후** 정상 Builder review 경계에서만 명시적으로 생성/승인한다.
- `UnknownFact`와 `Conflict`는 값 없음을 숨기지 않는다. Mandatory safety/runtime field의 identity 불명·해결되지 않은 충돌·유효하지 않은 단위는 blocking; optional 값의 누락은 explicit absent/NotPublished reason으로 표시하고 실제 값처럼 생성하지 않는다.
- Source collision/multi-trim mixing을 허용하지 않는다. 서로 다른 연식·트림의 값을 쓰려면 해당 identity에 대한 별도 claim/intent와 user-visible derivation/rejection 이유가 필요하다.
- **field mapping**은 provider stable field path ↔ `FCFVehicleFieldPath`/profile-domain field ↔ `UCFVehicleData` typed resolver projection을 1:1 또는 명시적 transform으로 등록한다. `km/h ↔ cm/s`, `kg`, `N·m`, tyre code → radius 등은 unit + formula revision/fingerprint를 요구한다. 매핑 불가/unknown physical field는 numeric placeholder 없이 차단한다.
- 모든 값은 원본 real-spec identity와 target Vehicle Identity가 별개임을 보존한다. 게임용 CarFight tuning으로 실제 제원 source를 덮어쓰지 않는다.

### 13.2.5 C5 — Single Vehicle Runtime discovery (P1-05)

**First Wave 결론: 신규 Vehicle Production Publication Catalog를 만들지 않는다.**

- **유일한 현재 Runtime vehicle membership/discovery source**는 Project Settings에서 resolve하는 `UCFRuntimeTestCatalogData::AllowedVehicleData`다. 이름이 Test Catalog라는 이유로 임의 병렬 production list를 만들지 않는다.
- Editor에서 approved `UCFVehicleData`를 등록하는 **유일한 membership writer**는 `FCFVehicleCatalogPromoService`다. Provider가 `AllowedVehicleData` 배열을 직접 수정하지 않는다. Empty/invalid/duplicate/catalog-unavailable은 existing full validation에 따라 fail-closed.
- 기존 Catalog가 노출하는 legacy/test vehicle membership은 보존한다. 별도 **CCAS-managed First Wave roster/provenance**는 authoring/acceptance 목록일 뿐 runtime membership의 두 번째 authority가 아니다.
- 신규 차량은 S5 뒤 S6에서 exact1 member가 되며, Catalog disk reload에서 `VehicleData` hard reference와 0 duplicate를 검증한다. `FCFRuntimeVehicleApplyService::ValidateCatalogVehicleCandidate/ApplyCatalogVehicle`와 `CFRuntimeApplyWidget`는 현행 membership을 소비한다. No runtime widget/provider branch required for Vehicle First Wave.
- `UCFProdEquipCatalogData`는 CF-FQ-058 **Equipment** publication authority로 유지한다. Vehicle candidate에 Equipment catalog를 재사용하거나 Equipment fallback 동작을 바꾸지 않는다.
- 더 큰 정식 Garage/Vehicle Product Catalog가 필요해지는 후속 단계에서는 별도 Feature와 **단일 source migration + 기존 Runtime consumers·tests 동시 갱신**이 필요하다. CF-FQ-059 P0에서 이를 선행하지 않는다.

### 13.2.6 C6 — Canonical Workbook, identity and cross-writer drift (P1-06)

1. **기준 Workbook은 기존 `Authoring/Content/CarFight_Content.xlsx` exact1**이다. 이미 CCAS 관리 중인 42개 record와 Weapon Publication exact8을 덮어쓰거나 재-cutover하지 않는다. 신규 Vehicle domain row는 **승인된 R2 후보 생성/RecipeId readback 후** 기존 `AI ChangeSet → Review Package → staged Workbook → typed Vehicle onboarding/cutover extension` 경로로 추가한다. CF-FQ-058의 removed one-shot production cutover entry를 재호출하지 않는다. AI raw `.xlsx` binary 직접 수정/overwrite 금지.
2. **Current Vehicle catalog key 보존**: existing `CFContentCatalog.cpp` read projection의 `FCFContentKey(ContentTypeId.Value = "vehicle", ContentId = "vehicle_" + RecipeId.ToString(Digits).ToLower())`를 **Vehicle managed canonical ContentKey와 일치시킨다.** `RecipeId`는 persistent identity다. Product asset path/사용자 표시 이름을 새 ContentId로 사용하지 않는다. 이미 존재하는 managed RecipeId와 key 충돌·다른 target binding은 blocking이다.
3. **P0-01 representative roster**는 candidate path/reference pair만 제시한다. 실제 `RecipeId`는 P0-04에서 approved R2 two-record creation(S1)을 수행한 후 readback으로만 확정한다. 그 이전의 R2 Preview에는 `ContentKey` 없는 create proposal/path binding만 존재한다. 최종 `vehicle_<RecipeId>` managed record/CCAS immutable ReviewPackage(S2)는 **실제 RecipeId 확인 이후** 작성·승인한다. 첫 publish 전에 `ContentKey ↔ RecipeId ↔ exact TargetVehicleData object path ↔ original ResourceId` 1:1 binding과 schema revision을 manifest/provenance에 기록한다. 새 텍스트 파일에 임의 RecipeId를 선발급하거나 UI 표시 이름/패키지 rename을 암묵적 재식별로 사용하지 않는다.
4. **Cutover 전** existing unmanaged VehicleData/Recipe/Profile과 CFBatch/Builder는 그 field의 Current authority다. **Cutover 후** `CCASManaged` field intent는 Workbook만 authority이며 기존 Builder는 동일 field를 수정할 경우 `ChangeSet → Review`로 route하거나 read-only/fail-closed한다. `ExternalManaged(VehicleBuilder)`는 Builder가 쓰고 CCAS가 read/compare만 한다. 같은 field를 두 경로에서 저장할 수 없다.
5. **Two-layer+Vehicle stale guard**: existing `BaseWorkbookSemanticHash` + `BaseCatalogSnapshotFingerprint` + `ProviderSchemaFingerprint` + reviewed `Manifest/ReviewPackageFingerprint`를 보존하고, Vehicle-specific `RecipeId/RecipeFingerprint/Revision`, `DefinitionHash`, `EvidenceFingerprint`, `Profile/SourceSignature`, `ResourceCatalogCapabilityFingerprint`, `exact target path`, `Catalog membership prestate`를 mutation 전 재검증한다. Target scoped `CCASManaged` readback drift와 `ExternalManaged` preservation을 둘 다 검사한다.
6. **Drift 시 행동**: CCAS-managed Product direct edit → `Drifted / Blocked`; Workbook으로 되돌릴지, **Product→Workbook reviewed Change Proposal**로 명시적으로 채택할지 결정한다. 자동 overwrite/rebase/implicit import 금지. Builder-owned manual socket/tuning 변경은 보호하되 CCAS source/asset fingerprint가 바뀌면 affected derivation/preview를 stale로 처리한다.
7. **Compatibility**: 기존 `vehicle_RecipeId` Catalog/Compare consumer, Vehicle Authoring/Builder, CFBatch, RuntimeTestCatalog와 existing Equipment Workbook/Publication이 동일 identity and authority 계약을 사용하도록 P0-03 구현과 같은 semantic change 단위에서 callers/tests/fixtures/references를 정렬한다. 기존 CCAS-managed exact42 snapshot을 새 Vehicle 값으로 오해하거나 재발행하지 않는다.

### 13.2.7 Frozen First Wave execution protocol / Gate-by-Gate verification

```text
P0-01  AssetDump persisted Mesh inventory (전체 경로, legacy/test/product 분리)
       → 카테고리별 대표 chassis exact1+, wheel/socket/scale readiness
       → USER가 Excel을 편집할 필요 없는 이름/그림/근거 목록
P0-02  대표차 make/model/year/trim/market exact1 + cited fact/source conflict
       → Mesh measurement receipts / provenance/unknown decision
P0-03  vehicle Provider schema/field ownership + trusted AI R2 admission
       → typed mapping/preview/diff/fingerprint + pre-mutation focused tests
P0-04  기존 writer 기반 R1/R2/R3 + durable journal/save/readback/cutover
       → 기존 Catalog writer를 마지막에 호출·저장하는 publish proof
P0-05  Wave A one representative end-to-end proof → explicit review
       → Wave B rest of actual discovered categories (blind bulk mutation 금지)
P0-06  persisted dependency/readback, Cook/discovery/spawn/forward/reverse/
       steering/mass/fitting regressions (기술 증거와 USER feel 분리)
P0-07  USER driving/visual/feel acceptance; tuning은 CarFightTuned만 reviewed
P0-08  Current Systems promotion / immutable UDS close / FeatureQueue Done
```

**예정 검증 항목 / 이번 단계에는 미실행:**

| Focused scenario | 기대 결과 |
| --- | --- |
| Provider exact field ownership, nested struct/array child key, `CCASManaged` / ExternalManaged mixed | 대상 소유 필드만 변경, 기타 field+package unchanged |
| AI R2 creation permission/approval, spoofed `SlateUI`/Automation, duplicate path or `RecipeId` | 승인된 정상 경로만 허용; 우회·collision `mutation0` |
| Invalid Reference identity/conflicting citations/unknown mandatory field/units | source를 바꾸거나 가짜 값 생성하지 않고 validation block |
| Resource missing/wrong type/missing wheel sockets/change mid-review | preview/apply stale 또는 blocked, Product mutation0 |
| Compound package Save failure, disk reload uncertainty, journal/resume | `RecoveryRequired` / no false success / no automatic physical delete |
| Workbook commit success but Catalog publish fail, stale Catalog/member duplicate | `unpublished` / runtime membership 불변 / no Verified |
| Existing unmanaged Wagon/Builder save, first-wave managed field edit, Equipment Catalog exact8 | 기존 authority/Vehicle catalog/Equipment workflow 회귀 없음 |
| Existing `vehicle_RecipeId` Catalog compare, R1/R2/R3/Builder/RuntimeApply/DA staging callers | sibling stale assumption 없음, single writer preserved |

**No-go:** CCAS Generic Compiler/Resource/Weapon, existing `UCFVehicleData` runtime meaning, `FCFVehicleApplyService`, Fitting Mass Guard, default Sensor fallback, CCAS Equipment publication, user-owned dirty package를 편의상 수정하지 않는다. 새 GUI/Workbook 탭이나 새 Catalog/Provider server tool을 선결론으로 만들지 않는다. 실제 C++/Automation/Editor technical acceptance는 각 후속 Gate에서만 수행한다.

### 13.2.8 Fresh Design Re-review (v0.1.2)

- **P1-01**: field-path 단일 authoring authority + existing Target writer + sub-data independent provider 경계 → **설계 해결**.
- **P1-02**: AI diagnostic identity≠write authority; 기존 R0~R3/approval과 additional trusted R2 entry를 분리 → **설계 해결**. 실제 구현 전에는 AI R2 차단을 유지.
- **P1-03**: R2 read-only preview는 RecipeId 미발급이므로 R2 candidate creation → RecipeId readback → final Workbook Review 순환을 해소했고, multi-package는 non-atomic, product-save → Workbook commit → Catalog publish, RecoveryRequired 경계를 확정 → **설계 해결**. 실제 durable 구현/테스트는 P0-04.
- **P1-04**: Reference FACT vs Mesh measurement 분리, DERIVED/GAME_BIAS 후속 적용, missing/conflict 처리 → **설계 해결**.
- **P1-05**: 신규 Vehicle publication catalog 없이 `AllowedVehicleData` exact1 + 기존 promotion service → **설계 해결**.
- **P1-06**: `vehicle_<RecipeId>` 기존 identity를 R2 생성 후에만 확정하고, canonical Workbook exact1, typed Vehicle onboarding extension, cutover-scoped ownership 및 stale guard → **설계 해결**.

**교차 의존성 최종 확인:**
- C1→C2→C3에서는 CCAS가 intent를 관리해도 Target writer는 기존 것 하나이며 save는 higher-level orchestration이 한다.
- C3→C5에서는 Catalog publish가 final durable stage여서 Workbook 미승격 Product를 Runtime에 노출하지 않는다.
- C4→C6에서는 Citation/Evidence/Mesh fingerprint가 Workbook/Recipe/Target review identity에 모두 포함돼 research/asset drift를 막는다.
- C5→C6에서는 `RuntimeTestCatalog`만 current runtime membership이며 CCAS roster는 authoring/acceptance projection으로 남는다.
- P0-01 inventory 및 P0-02 research에서는 **C++·UE asset·Workbook Product mutation이 없다**.

```text
VCCAS-P0-00 Design Re-review
P0 blocker: 0
Blocking P1: 0 (v0.1.1의 6건 모두 계약 교정)
P2: 2 non-blocking (category inventory exact count, USER information/feel presentation)
Verdict: DESIGN PASS / CONTRACT FROZEN
Implementation / Build / Automation / UE Product Mutation: NOT OPENED
Exact next Gate: VCCAS-P0-01 — Vehicle Mesh Inventory + Category Roster
```

## 13.3 VCCAS-P0-01 — Historical Vehicle Mesh Inventory / Category Roster Evidence (2026-10-12; current scope superseded by 13.5)

- 상세 evidence owner: `Document/Plan/VehicleContentAuthoring/VehicleMeshInventory.md v0.1.0`.
- **Repository cross-inventory:** `CarFightMCP_Admin` managed Git tracked `.uasset` confirms `/Game/CarFight/Vehicles/Meshes`에 CityCar/Compact/Coupe/Pickup/SUV/Sedan/SubCompact/Van/Wagon body-named package **exact9**. Dedicated wheel set exact4는 SUV/Sedan/Wagon, shared tire wheel exact4. 별도 Legacy Police combined body+wheel4, external/example Offroad `SM_Offroad_Body`+tire, SportsCar `SM_SportsCar`+wheel/glass도 보존.
- **Persisted AssetDump:** `/Game` StaticMesh indexed exact37 (returned37, omitted0), 이 중 vehicle body `SUV/SUV`, `Wagon/Wagon`, `_Legacy/police_car/.../Combined_Body` **exact3**와 wheel exact12만 type verified. Git tracked body-named package **exact12**(core9/legacy1/template2) 중 나머지 **exact9는 AssetDump class 미검증**. Stored index fingerprint: `7abf97d379deb846cf317eebcebe061c9bfeffb38a87afe4e08b2d905cc3c48b`; 이는 fresh Editor rescan이 아니다.
- **Socket-derived measurements (not body geometry):** SUV `Wheel_Anchor_*` exact4 + `HP_*` exact2; axle X separation `258cm`, left-right Y separation `152cm`, scale 1. Wagon `Wheel_Anchor_*` exact4, X separation `280cm`, Y separation `168cm`, socket X/Z scale `0.8`, `HP_*` socket0. Police `StaticMesh` sockets0. Sedan authored `DA_TestSedan` anchors `289cm/150cm` are **DA-authored intent, NOT measured Sedan StaticMesh evidence**. All body bounds and wheel local bounds remain unverified.
- **Current Recipe / Data authority:** `DA_Recipe_Wagon` and `DA_Vehicle_Wagon` use `/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL`, which is Git-tracked but absent indexed AssetDump. Wagon socket `PreviewStaticMesh` references include Legacy Police FR; this is preview metadata, not proof of actual wheel runtime binding. `DA_Recipe_TestSedan` and `DA_TestSedan` bind indexed-missing Sedan body/wheel refs. Do not infer broken packages solely from absent stored-index rows.
- **Live evidence blocker:** Unreal Editor MCP `ERR_UE_MCP_TIMEOUT`, then `ERR_UE_MCP_UNAVAILABLE`. Accepted `ue.batchdump_safe` `managed_dataset=true` fresh preparation yielded `ue_managed_dataset_prepare_failed`; targeted Sedan/CityCar/SportsCar AssetDump source absent. Production asset mutation was not used to overcome missing read-only evidence.
- **Boundary:** folder label exact9 does **not** establish nine unique visually distinct categories: e.g. CityCar/SubCompact/SUV materials overlap, Coupe/Sedan materials overlap. Police legacy and Offroad/SportsCar auxiliary alternatives are NOT independent Production obligations until class/visual overlap gate.
- **Historical v0.1.4 provisional representatives (selection scope superseded by 13.5):** one tracked candidate per 9 core folders, with SUV/Wagon as indexed StaticMesh and Sedan as authored DA/Recipe. Earlier inventory considered Police and Offroad/SportsCar as optional alternatives; **all these external/Legacy chassis are now USER EXCLUDED from CF-FQ-059**. No final core9 First Wave category exact count or real vehicle reference selected.

### P0-01 Fresh evidence acceptance still open

1. `I-01`: current USER scope 13.5에 따라 approved read-only Editor AssetRegistry / refreshed AssetDump가 **core9 only** actual `StaticMesh` class/load validity 및 core9 wheel dependency를 검증한다. 기존 Legacy1/template2 확장 범위는 historical assumption으로 폐기했다.
2. `I-02`: per-body local bounds, per-wheel local bounds/radius/width, actual sockets/fingerprint + missing wheel/Hardpoint fallback verified. Existing `FCFVehicleAssetReader::BuildAssetSnapshot` is read-only wheel bounds/socket reader but does not itself supply chassis body bounds.
3. `I-03`: genuine geometry/silhouette-based category dedupe and one representative per **actual** distinct category; don't equate material/folder names with taxonomy.

**Verdict: VCCAS-P0-01 PARTIAL EVIDENCE / GATE HOLD (evidence blockers I-01~I-03 exact3).** `VCCAS-P0-00 v0.1.3 DESIGN PASS / C1~C6` is preserved and unaffected. Not a new design failure; never call P0-01 PASS or advance P0-02 before fresh measurements. No Production/Workbook/C++/Blueprint/UE Asset/Build/Automation/PIE mutation.

**Exact continuation inside same P0-01:** restore *existing* approved read-only UE connection (without GoPyMCP/AssetDump plugin changes in this CarFight session) → fresh class/socket/bounds evidence for all candidate bodies/wheels → category dedupe/representative exactN freeze → P0-01 Fresh Technical Review.

---

## 13.4 VCCAS-P0-01 — Historical USER intermediate scope (2026-10-12; superseded by Core9-only 13.5)

이 절은 13.3의 **당시 범위로 수집한 Historical AssetDump 사실**을 지우지 않고, 2026-10-12 USER의 최신 선택 범위를 확정한다.

- **Legacy Police Car / `/Game/CarFight/_Legacy/police_car`는 CF-FQ-059 현재 First Wave 대표 차량 선택, Production Vehicle 제작, 재사용/복구 및 후속 차종 중복 검수 대상에서 제외한다.** 경찰차 파일을 삭제·수정하거나 다른 기능에서 제거하지 않는다. 재포함은 USER의 별도 명시적 결정만으로 가능하다.
- **2026-10-12 당시 중간 선정 범위(2026-10-13 현재 폐기됨)**는 tracked body-named 후보 exact11 = CarFight Core exact9 + Sample Offroad/SportsCar exact2였다. **지금의 유효 범위는 13.5절 Core9 ONLY**이다. 옛 exact11은 실제 유형이 검증된 `StaticMesh` 개수나 최종 서로 다른 차종 수가 아니다.
- 과거 AssetDump `/Game` indexed `StaticMesh` exact37, 차량 차체 발견 exact3(SUV/Wagon/**제외된 Police**)라는 원본 기록은 변경하지 않는다. **현재 선정 범위의 AssetDump verified Body StaticMesh는 SUV/Wagon exact2**, 범위 내 class 아직 미검증 후보 exact9(core7+sample2).
- `DA_Recipe_Wagon`/Body Socket preview에서 관찰된 Legacy Police Wheel 참조는 Wagon 현 상태 정합성의 Historical Evidence로만 유지한다. 경찰차가 다시 후보가 되거나 경찰차 Asset을 수정해도 된다는 뜻이 아니다.
- **Historical 2026-10-12 당시** P0-01 I-01 class coverage / I-02 bounds·socket 측정 / I-03 category dedupe 범위는 후보11이었다. 이 요구는 **13.5 Core9 ONLY**로 대체되며 template 및 경찰차 차체는 측정 대상이 아니다. Evidence HOLD exact3은 그대로 유지한다.
- 상세 후보별 로스터/증거/이전 조사와 현재 범위의 구별: `VehicleMeshInventory.md v0.1.1`.

**판정:** 범위 교정 적용, `VCCAS-P0-01 PARTIAL EVIDENCE / GATE HOLD` 유지. `VCCAS-P0-00 v0.1.3 DESIGN PASS` 보존. Production/Workbook/Source/UE Asset mutation 0.

---

## 13.5 VCCAS-P0-01 — USER final chassis selection scope: CarFight Core9 ONLY (2026-10-13)

USER가 2026-10-13 **CF-FQ-059 차량 차체는 CarFight 차량 차체 후보만 사용**하도록 결정했다. 이 최신 규칙은 13.3의 Historical 발견 범위와 13.4의 중간 범위(Police 제외, optional template2 포함)를 **대체**한다.

- **현재 First Wave 차체 선정의 유일한 root는 `/Game/CarFight/Vehicles/Meshes/`이다.** Git tracked body-named candidate **exact9**: `CityCar`, `Compact`, `Coupe`, `Pickup`, `SUV`, `Sedan`, `SubCompact`, `Van`, `Wagon`. 각각 기존 추적 `.uasset`의 후보이며 Unreal `StaticMesh` class/외형이 모두 입증된 것은 아니다.
- **완전히 대상 제외**: `/Game/CarFight/_Legacy/police_car` Legacy Police Car, `/Game/Vehicles/OffroadCar`, `/Game/Vehicles/SportsCar`, `/Game/Models/SportsCar` 등 현재 CarFight Core9 root 밖의 다른 차체. 외부 차량으로 카테고리의 빈 부분을 채우거나 대체하지 않는다. 기존 에셋은 삭제·변경하지 않는다.
- **증거 등급/현황**: 현재 root의 stored AssetDump `StaticMesh` 검증된 차체는 `SUV`·`Wagon` **exact2**. 나머지 `CityCar/Compact/Coupe/Pickup/Sedan/SubCompact/Van` **exact7**은 Git-tracked body package이나 최신 Unreal Class·Socket·Bounds 미검증. **최종 실제 차종 수와 Production 적격 수 미확정**.
- `/Game/CarFight/Vehicles/Shared/Tire`의 wheel resources는 core9의 기존 **wheel dependency**로 조사할 수 있으나 새로운 차체 후보가 아니다. Wagon preview의 경찰차 휠 참조는 현 상태 분석의 기존 의존성 증거로만 기록하며 경찰차 차체를 다시 후보로 올리지 않는다.
- Historic stored index `/Game`의 total StaticMesh exact37 / observed Vehicle Body exact3에는 사용자 제외 Police1이 포함된다. Historical 증거는 그대로 두고 **현재 선정 범위 Body verified exact2**를 authority로 구분한다. 이전 후보 exact11/12는 Historical counts일 뿐 Current roster가 아니다.
- **P0-01 I-01~I-03**의 Fresh AssetRegistry/AssetDump loaded-class, body/wheel bounds, wheel/socket and body-shape dedupe 범위는 **Core9 + Core9의 연결된 필요한 부품**만이다. 폐기된 template vehicle body를 다시 검증하기 위한 UE 작업을 추가하지 않는다.
- 대표 세부 명부: `VehicleMeshInventory.md v0.1.2`. `VCCAS-P0-00 v0.1.3 DESIGN PASS` 보존, `VCCAS-P0-01 PARTIAL EVIDENCE / GATE HOLD` 유지. Production/Workbook/C++/Blueprint/UE Asset mutation0.

**Exact next:** Core9 클래스/Bounds/Socket fresh-read → 차종/실제 모델의 실물 중복 제거 → Core9 내 카테고리별 대표 후보 확정 → P0-01 fresh technical review. P0-02 실차 스펙 authoring은 NOT OPENED.

---

## 13.6 VCCAS-P0-01 — Wagon Reference Workflow + USER Mesh Preparation Handoff (2026-10-13)

USER의 제작 역할·순서 결정을 추가한다. **Wagon은 이미 제작/실차 참고 자료/주행 검증까지 경험한 유일한 기준 사례**이며, 다른 CarFight Core 차량들은 아직 메시 관련 작업이 충분히 진행되지 않았다는 USER 보고를 보존한다. 이 규칙은 기존 `P0-00 v0.1.3 DESIGN PASS`, `Core9 ONLY`, `R0~R3` Authoring/Runtime 단일 권한 경계를 변경하지 않는다.

### Wagon에서 재사용할 것과 재사용하지 않을 것

- **재사용할 제작 프로세스**: 대상 StaticMesh/휠 자산 확인 → 차체 Socket/휠 Geometry와 Preview 증거 확인 → 해당 Mesh에 가까운 실제 차종/연식/트림 및 `UCFVehicleRefEvidence` 확보 → 해당 차량만의 Recipe/Profile/Resolver 검토 → `FCFVehicleApplyService` Target 적용 → durable Save/reload/Vehicle membership 검증 → 런타임/USER 운전감 수락.
- **실제 남아 있는 기준 자산**: `/Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon`, `/Game/CarFight/Data/Authoring/DA_Recipe_Wagon`, `/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon`, Recipe에 기록된 Wagon Builder Evidence/Driving Acceptance receipts. 이는 **Wagon의 이미 작성된 참고 기록**으로, 신규 차량 Production Catalog 등록 또는 CCAS Vehicle Provider 준비 완료의 증명으로 사용하지 않는다.
- **기존 StaticMesh authoring contract**: `Wheel_Anchor_FL/FR/RL/RR` Socket의 위치·RelativeScale이 차량별 휠 기준점/시각 스케일 source다. `VehicleBuilder.md` / `VehicleData.md` Current System 기준, 같은 Body StaticMesh를 공유하면 Socket Geometry도 공유한다. 필요 시 별도 Mesh variant가 필요하다. `UCFVehicleData`의 wheel visual/physics, HardpointSlots/MountProfiles, 질량, 구동계는 차종별 고유 값이므로 Wagon 수치·휠 크기·소켓 좌표를 다른 차량에 복사하지 않는다.
- **관찰된 Wagon 미해결 세부 사항**: 과거 AssetDump에서 Wagon body `HP_*` 소켓 0, Recipe/VehicleData에는 hardpoint intent가 있고 Wheel FL은 `Shared/Tire`를 참조한다. Builder/Runtime의 유효한 fallback 가능성은 별도 검증 대상이다. **Wagon의 모든 메시 구성이 정상임을 무조건 보증한다는 뜻이 아니다.**

### 실제 담당 구분

| 작업 단위 | 기본 담당 | 허용 범위 및 인계 기준 |
| --- | --- | --- |
| Core9 StaticMesh/휠/소켓 상태 읽기·문제 목록 작성 | GPT | 승인된 read-only Unreal/AssetDump/AssetReader를 사용하고 각 차량의 `현재 관찰값`과 `부족한 준비`를 구분한다. |
| **웨건 외 차량의 실제 차체 Mesh authoring**: 중심·축·스케일/휠 하우스 정리, `Wheel_Anchor_*` 4개 위치·RelativeScale, 필요한 hardpoint/socket 추가·조정, 필요한 휠 자산 준비 | **USER** | 실제 Unreal Editor 메시 에셋 편집 및 Save는 USER 작업으로 둔다. GPT는 구체적인 위치/수치 제안을 증거 없이 강제하지 않고, 먼저 차량별 가이드·검수 항목을 제공한다. `HP_*`는 무장이 필요한 차량의 승인된 Hardpoint 계획에 따라 필요한 경우에만 요구한다. |
| USER Mesh 작업 후 fresh Socket/Bounds/휠 연동 readback, category dedupe/대표 선정 | GPT | 실제 저장된 값에 근거한 증거를 다시 확보하고 `MeshReady`/ `MeshPreparationRequired`/ `Unverified`로 구분. USER 편집 완료 주장만으로 검증 PASS하지 않는다. |
| 실차 후보 선정·공개 제원·Evidence/Recipe/VehicleData/Workbook Backend·Production/Runtime 기술 검증 | GPT | P0-02 이후 각 승인 Gate에서 기존 타입화된 Authoring/Apply/Save/Catalog 단일 authority를 통해 수행한다. USER가 Excel을 직접 다루거나 Builder Wizard를 강제 사용하지 않는다. |
| 최종 실제 운전감/손맛 수락 | USER | USER Driving acceptance는 자동화 PASS와 구별한다. |

### Core9 진행 방식 — USER Mesh 준비로 전체 조사를 멈추지 않는다

1. **Wagon reference study**: 현재 Recipe/Target, 기본 차량 Reference Evidence, Socket/wheel/physics/Builder Driving receipts를 읽어 **다른 Core8에 재사용할 제작 절차와 메시 작업 점검표**를 만든다. Wagon은 Reference Case이고 값 복사용 Template가 아니다.
2. **Core8 read-only readiness**: 각 body 후보의 AssetClass/Bound/Socket/휠 리소스를 조회 가능한 범위까지 검사한다. 사용자 보고상 메시 준비가 적다는 사실을 기억하되, 이미 존재하는 SUV Socket 등 verified 증거를 무효화하지 않는다.
3. **차량별 USER Mesh Preparation Handoff**: 실제 없는 Wheel Anchor/socket, 휠 크기/위치, 중심/축, 선택 Hardpoint 등 확인된 누락만 정확한 asset path와 함께 전달한다. 준비되지 않은 차량은 `MeshPreparationRequired`로 기록하며 임의 수치, Wagon 기반 덮어쓰기, 자동 메시 수정을 하지 않는다.
4. **다른 차량 계속 조사 가능**: 한 차량의 USER 편집이 진행되지 않았더라도 나머지 Core 차량의 read-only inventory·Reference 후보 자료 조사 준비 자체를 막지 않는다. 그러나 해당 차량이 실제 Mesh 준비/검수되기 전 Production Vehicle authoring으로 승격하거나 P0-01 전체 PASS를 선언하지 않는다.
5. USER가 해당 차체/휠 메시를 저장한 후 fresh AssetRegistry/AssetDump/Socket/Bounds 측정 → `MeshReady` review → 후속 authoring gate 진입 여부를 판정한다. 개별 **차량마다** 완료/미준비를 구분한다.

**P0-01 current decision:** Core9 ONLY, Wagon-first reference, USER owns physical Mesh preparation on other cars, GPT owns analysis/handoff/typed data production. I-01~I-03 remain **GATE HOLD** until required read-only evidence and class/category/geometry re-review. No Editor/Production DataAsset/Workbook/Source changes in this record.

---

## 13.7 VCCAS-P0-01 — Wagon Reference Readback + Core8 Handoff Checklist (2026-10-13)

**이번 후속 실행에서 완료한 단위는 Wagon reference stored-evidence recheck 및 Core8 사용자 메시 준비 체크리스트 작성**이다. P0-01의 세 증거 게이트를 임의 종료하거나 UE 차체를 수정하는 작업이 아니다.

1. **Fresh managed Git read-only**: `UE/Content/CarFight/Vehicles/Meshes` 내 Core9 body-named `.uasset` exact9, dedicated Wheel packages SUV/Sedan/Wagon 각4 및 Shared Tire4 추적 존재를 재확인했다. 이는 실제 AssetRegistry class/bounds/socket 증명이 아니다.
2. **Stored AssetDump 재조회**: Wagon/SUV Body `StaticMesh`와 각각 `Wheel_Anchor_*` socket4, SUV `HP_*` socket2를 다시 읽었다. Wagon stored Socket loc/scale FL(152,-84,17), FR(152,84,17), RL(-128,-84,17), RR(-128,84,17), scale(0.8,1,0.8)은 원본 Wagon 데이터이며 다른 차종 값으로 복제하지 않는다.
3. **Wagon authored-reference chain recheck**: stored `DA_Recipe_Wagon`, `DA_Vehicle_Wagon`, `DA_Ref_Wagon`와 `BuilderDrivingAcceptanceReceipt`를 확인했다. `WheelVisualIntent.Mode=SocketScaleFromChassis`, FL shared tire Wheel + FR/RL/RR fallback, USER authored socket scale, 실제 Wagon의 2026 Volvo V60 Cross Country Ultra B5 AWD KR reference/Unknown 분리 및 private4 profiles가 **재사용할 절차의 입증된 예**다. 이 결과는 신규 차종의 authoring gate PASS 증거가 아니다.
4. **Historical evidence temporal check**: 초기 Wagon Builder E2E 당시 `Wagon/Van/SubCompact/Pickup/Coupe/Compact/CityCar socket0`, `Sedan socket7`, `SUV socket6`이 관측됐다. 이후 Wagon socket4가 편집 저장돼 있음이 stored AssetDump에서 확인된다. **따라서 나머지 6차량의 과거 0개 Socket을 current absence로 단정하지 않는다.** current missing status는 fresh Editor read가 필요하다.
5. **UE read-only request**: `ue_status`가 `ERR_UE_MCP_TIMEOUT`으로 응답했다. Live AssetRegistry, 모든 Core9 Body bounds, Wheel bounds, 7개 indexed-missing chassis class/현재 socket 증거는 못 얻었다. 반복적인 재시작·GoPyMCP/AssetDump 수정으로 우회하지 않는다.
6. **USER handoff document**: `Document/Plan/VehicleContentAuthoring/VehicleMeshPreparationChecklist.md v0.1.0`을 생성했다. Wagon reference data, Core8 차량별 historical/current 근거·우선 점검, UE Socket Manager 작업 가이드, 측정 PASS 조건, `Unverified/MeshPreparationRequired/MeshReady` 기준을 담았다.

**정확한 현재 판정:** Wagon reference evidence study **PASS (stored evidence only)** / Core8 provisional USER checklist **PREPARED** / P0-01 Fresh Mesh Class/Bounds/Socket/Category gate **HOLD I-01~I-03**. No Core8 has been declared `MeshReady` or `MeshPreparationRequired` from stale evidence alone; UE Asset/Product/Workbook/C++/BP/Map mutation0.

**Exact next (same P0-01):** existing approved UE read-only connection restored → `/Game/CarFight/Vehicles/Meshes` Core9 current `StaticMesh` class + Socket + full Body/Wheel bounds 조회 → per Core8 confirmed missing/ready classification, exact path/name/expected check handed to USER → USER edits only needed mesh assets and saves → GPT fresh readback → Core9 actual geometry/silhouette duplicate judgment + P0-01 fresh technical review. P0-02 not opened.

---

## 13.8 VCCAS-P0-01 — Fresh LIVE UE Recovery, Core9 Class + Bounds + Socket Rebaseline (2026-10-13)

USER가 UE 연결 복구 사실을 알린 뒤 **실제 UE MCP `ue_status` connected / initialized, `ue_list_toolsets` 정상**을 확인했다. CarFight exact `/Game/CarFight/Vehicles/Meshes/Wagon/Wagon`를 `AssetTools.get_asset_class`로 확인하고 Core9 전체를 Live 조회했다. 이어 managed `ue.batchdump_safe` (`root=/Game/CarFight/Vehicles/Meshes`, `class_filter=StaticMesh`, `changed_only=false`, `include_details=true`)가 **exact21/21 PASS**, 새 dataset `adset_v1_356bb3fbfc606b87a694564a577f07aa.3e489c82d5fdba214fe539db`에서 Core9 Body exact9 + dedicated Wheels exact12의 class와 Socket 목록을 획득했다. UE 에셋은 편집하지 않았다.

**Body Class exact9/9 PASS; live local Bounds/Socket summary:**

| 후보 | Local Body size X×Y×Z (cm) | 현재 Chassis Socket |
| --- | --- | ---: |
| CityCar | 99.805×54.492×35.352 | 0 |
| Compact | 99.805×48.633×29.492 | 0 |
| Coupe | **45.117×99.805×26.367** (long axis Y) | 0 |
| Pickup | 99.805×43.164×28.320 | 0 |
| SUV | 453.274×210.152×149.596 | **6 = Wheel4+HP2** |
| Sedan | 488.710×203.800×125.390 | **7 = Wheel4+HP2+FX1** |
| SubCompact | 99.805×49.414×33.008 | 0 |
| Van | 99.805×45.117×32.227 | 0 |
| Wagon | 487.092×233.052×138.746 | **7 = Wheel4+HP3** |

**USER MeshPreparationRequired exact6**: CityCar, Compact, Coupe, Pickup, SubCompact, Van는 실제 `Wheel_Anchor_FL/FR/RL/RR` 전부 부재를 확인했다. 이 중 다섯은 X축 길이 ~100cm, Coupe는 Y축 길이 ~100cm이므로 소켓 추가보다 **메시 임포트 크기/축/피벗/차체 방향 실물 검토를 우선**한다. Body local-space bounds는 in-game scale이나 실차 크기를 뜻하지 않아 자동 `×4` 또는 회전을 처방하지 않는다.

**SUV/Sedan**은 Wheel Anchor4가 이미 있고 **Wagon**도 Wheel Anchor4와 HP3이 있다. 과거 old AssetDump Wagon Socket4/HP0 증거는 현 상태에서 틀렸으므로 13.3/13.6/13.7 및 과거 `VehicleMeshInventory` snapshot을 Historical로 분리한다. `Wagon` HP current exact3 `HP_Top_01,HP_Front_01,HP_Top_02`이며 이전의 Wagon HP Socket 없다는 추정은 더 이상 현재 오류 판정 근거가 아니다.

**Wheel actual Bounds live FL examples:** shared `Wheel_FL` 100×25×100cm, dedicated Wagon `Wheel_FL` 100×25×100, SUV `Wheel_FL` 79.36×35.48×79.36, Sedan `Wheel_FL` 68.25×30.51×68.25. Dedicated wheel Class exact12 PASS, all wheel extents/orientation and effective wheel binding은 미완료. Body Bounds 및 Socket도 형태/차량 카테고리의 유일성 증거는 아니다.

**Current gate verdict:** `I-01 Core9 class coverage PASS`; `I-02 physical chassis/wheel/Socket validation HOLD` (six user physical prep confirmed; Wheel visual/ref/axis pending); `I-03 category/silhouette dedupe HOLD`. **P0-01 overall HOLD**, P0-02 NOT OPENED, P0-00 Design PASS preserved.

**Current exact next:** `VehicleMeshPreparationChecklist.md v0.1.1`에 기록된 six Core BODY assets에서 USER가 **scale/axis/mesh geometry 확인** 후 누락한 네 Wheel Anchor Socket 작성 및 저장 → GPT fresh Socket/Bounds/visual fallback/Body Category validation. SUV/Sedan 기존 Socket은 불필요한 중복 편집 금지. 외부 Police/Template 차량은 계속 제외.

---

## 13.9 VCCAS-P0-01 — USER Reference-First Core6 Real Vehicle Sizing (2026-10-13)

USER는 CityCar/Compact/Coupe/Pickup/SubCompact/Van **여섯 차체의 메시·소켓을 편집하기 전에 실제 대표차 1종씩의 크기와 공개 제원부터 결정**하도록 실행 우선순위를 수정했다. 이는 기존 P0-01의 사용자 실제 Mesh 수정 handoff **앞에** 필요한 설계·근거 선별 작업을 수행한다는 뜻이지, P0-02 Product/Reference DA authoring Gate를 조기 개방한다는 뜻이 아니다.

**신규 SSOT:** `Document/Plan/VehicleContentAuthoring/VehicleRepresentativeSpecRoster.md v0.1.0`.
- Six representative real vehicles **selected as review candidates**, official vehicle Length/Width/Height/Wheelbase 4×6 values confirmed from manufacturer public specifications: Kia Morning MY2027 KR (3595/1595/1485/2400mm), Maruti Suzuki Jimny five-door Alpha AT India June-2024 catalogue (3985/1645/1720/2590), Toyota GR86 MY2026 MT Oman (4265/1775/1310/2575; wheelbase via same-platform Toyota AU OEM evidence), Toyota HiLux SR5 4×4 AT AU current (5320/1885/1865/3085), Hyundai Venue 1.6 IVT 17-inch KR (4040/1770/1585/2520), Hyundai STARIA Hybrid Cargo 5-seat 17-inch KR (5255/1995/2000/3275).
- Complete trim/market/year qualifier, manufacturer direct URL, available weight/powertrain/tires, uncertainties/unknowns and local AABB vs real body exterior ratio checks are in above roster. **True USER-approved model-to-stylized-mesh visual correspondence is PENDING**. No one-to-one real car identity can be inferred from an imported source Material name.
- Original live Core6 Body Bounds are around 100cm on their longest local axis; Coupe longest axis is Y not X. The public OEM target ratios differ materially from current imported geometry, so **do not blindly rescale 4–5×, non-uniformly distort, or rotate any original Body**. User must review imported mesh orientation/scale/pivot and decide how it should physically match the chosen reference size.
- Core6 user MeshPreparationRequired Socket0 remains confirmed; SUV/Sedan/Wagon already have four Wheel Anchors. Other Core3 vehicle content is NOT added or repurposed. This step only fixes **proposed reference dimension baselines**, not gameplay WheelAnchor locations, Chaos WheelRadius, mass application, or derived geometry.
- **First blocking action:** USER review/accept six representative real car identities and size targets; if a choice does not suit actual body silhouette, replace that specific model and its sourced values before editing. Then Core6 USER body/forward/scale/sockets; GPT fresh geometry, actual wheel measurements and category dedupe. No Excel or Builder UI burden to USER.
- **Status:** Manufacturer six specification 4-tuples SOURCE VERIFIED; candidate identities USER ACCEPTANCE PENDING; I-01 Core9 AssetClass PASS; I-02/I-03 HOLD; P0-01 overall HOLD, P0-02 NOT OPENED; v0.1.3 P0-00 DESIGN PASS preserved; Production/UE Assets/Source/Workbook mutation0.

---

## 13.10 USER DECISION — Temporary Vehicle Meshes Deferred, Reference/Data-First Lane Open (2026-10-13)

USER 명시 결론: **현재 6종 포함 차량 메시들은 임시로 뽑아 둔 것이며, 이후 직접 새 메시로 교체한다. 현재 메시의 외형, 로컬 크기, 전방 축, Socket 유무는 실차 선정·스펙 조사·Vehicle Canonical Schema 준비에 관여하지 않는다.** 사용자 소유인 메시 교체·크기·Socket/휠 위치 편집은 별도 후속 작업으로 둔다. 기존 `VehicleMeshInventory.md`의 관찰값은 당시 placeholder의 역사적 Snapshot으로만 유지한다.

**P0-01 scope handoff:** Core9 Body 경로/클래스 9/9와 사용자 정의 차량 카테고리 명부 확보는 완료해 Reference Lane 진입에 필요한 **roster gate satisfied**. 과거 `I-02` 현재 임시 메시/휠/소켓 물리 완성도 및 `I-03` 현재 임시 실루엣 중복 판정은 **USER-DEFERRED / NON-BLOCKING for P0-02 reference research and P0-03 schema design**. 해당 요구를 삭제하거나 `MeshReady`/Runtime PASS로 위조하지 않는다. **P0-04 실물 자산 Materialization/Validation, P0-06 Runtime Technical Acceptance의 해당 입력 시점에는 USER가 교체한 실제 차체/휠/소켓 + fresh evidence가 필요**하다. 임시 메시의 geometry dedupe는 최종 카테고리 명부 선정 기준으로 재사용하지 않는다.

**P0-02 CURRENT ACTIVE:** 기존 지정된 6개 실차(Kia Morning / Suzuki Jimny 5-door / Toyota GR86 / Toyota HiLux SR5 / Hyundai Venue / Hyundai STARIA Hybrid Cargo)를 **Working Reference Identities**로 유지하고, 사용자에게 현재 메시와 유사한 실차로 재선정하라고 요청하지 않는다. 정확한 연식·시장·트림과 원본 FACT/Unknown/derived/tuned 경계는 `VehicleReferenceEvidenceDraft.md v0.1.5`에 제조사 링크와 함께 기록했다. P0-02 Source-scoped Claim 수락 및 실제 typed Validator 실행 결과는 `VehicleCore6EvidencePayloads.md v0.1.2`의 23 Canonical/61 Candidate가 최신이다. `VehicleRepresentativeSpecRoster.md v0.1.2`는 요약 및 지정 대표차 후보 명부이고, `VehicleReferenceEvidenceDraft.md`는 P0-02 상세 Claim 입력 명세다. **Current draft is not user-approved final production evidence nor persisted `UCFVehicleRefEvidence`.** 전체 P0-02 PASS는 claim/identity review, missing mandatory input, provenance/binding 검사 후만 판정한다.

**DataAuthoring scope now allowed:** reference FACT 수집·정규화·unknown 정리, Vehicle Provider field ownership/schema, UI-less AI R2 approved authoring boundary와 review/diff/readback 설계·구현 검수는 메시 없이 수행할 수 있다. 현행 `VehicleBuilder.md`는 `Blank Chassis` 신규 차량 Creation path를 지원한다. 다만 기존 승인된 `FCFVehicleAuthoringService`/Recipe/Profile/Resolver/Apply/Save contract가 허용하지 않는 `AI CallerKind`로 가장하거나, 기존 Production VehicleData/Workbook을 raw edit 하는 것은 금지한다. CarFight Config/Body/Wheel에 근거 없는 Dummy 값을 생성하지 않는다.

**Next exact gate:** `VCCAS-P0-02` six reference claim review + capability/unknown assessment → `VCCAS-P0-03` canonical schema/typed UI-less provider work → approved durable materialization at P0-04 with **valid replacement physical asset dependencies**, then final user mesh and runtime integration. If P0-03 cannot safely finish a value group without geometry, mark just that field `DeferredPhysicalInput`; continue unrelated field groups. User does not need to edit any workbook.

**Current verdict:** P0-01 reference roster ready with physical integration deferred by USER; P0-02 **ACTIVE / RESEARCH DRAFT AVAILABLE**, not PASS; P0-03 implementation **NOT OPENED**. Legacy frozen P0-00 Design PASS preserved. Existing StaticMesh/Sockets/UE Assets/Production/Workbook/Source unchanged in this directive.

---

## 13.11 VCCAS-P0-02 Source Review + P0-03 Pre-Implementation Source Audit (2026-10-13)

**P0-02 research audit complete for sourced dimensions:** `VehicleReferenceSourceReview.md v0.1.2` freshly rechecked official six manufacturer/maker-distributor publications and recorded exact **24 exterior dimension/wheelbase FACT values (6×4; research Canonical 23 / cross-market Candidate 1)**, source ID, locale/grade, typed `FCFRefClaim` staging field identity and explicit UnknownFact. The current `VehicleReferenceEvidenceDraft.md v0.1.5` is research draft, **not saved `UCFVehicleRefEvidence`**, and these rows are **six separate primary Evidence scopes**, not six Primaries in one Evidence UObject. Cross-market Toyota Oman GR86 output and Toyota Korea wheelbase stay as different-source claims; pickup 2155kg max and 3080–3120kg GVM range stay non-scalar; STARIA hybrid ICE/system power stay separate. Jimny technical page10 was verified, not confused with the shorter three-door variant. **P0-02 SOURCE REVIEW PASS / FULL EVIDENCE ACCEPTANCE HOLD** pending accepted exact identity/trim and authoritative per-car typed payload/review.

**P0-03 source-aware pre-implementation review complete:** `VehicleProviderPreReview.md v0.1.0` inspected actual `ICFContentProvider`, `FCFContentFieldDescriptor`/enum, vehicle Existing Resource Pilot, `UCFVehicleRefEvidence` and `PreviewVehicleRecords/CreateVehicleRecords` source. It identified two implementation-sensitive corrections:
- **P0-03-C1:** Source enum `ECFContentFieldOwnership` has `CCASManaged/ExternalManaged/LegacyPreserve/Derived/ReadOnly/Deprecated` (and invalid Unspecified). Conceptual old Plan prose `ExternalReadOnly` belongs to **record-level** `ECFContentManagementState`; `GeneratedProjection` is **not** a concrete field owner enum. Schema must use actual enum.
- **P0-03-C2:** `FCFVehicleAuthoringService::PreviewVehicleRecords` rejects callers except SlateUI/Automation. `AI` caller-kind is **not** authorization, so new provider cannot spoof `Automation` or simply weaken condition. A capability/approval-bound trusted AI R2 typed adapter with exact OwnerWrite scope is needed, reusing existing collision/re-preview/proposal/checks. No `VehicleData` raw writer and no independent persistent source of truth.

**Integration separation:** six source research candidates can be prepared with no chassis, wheel or sockets; `vehicle_<RecipeId>` canonical ContentKey exists only after approved R2 two-record creation readback, not in pre-Recipe research. Blueprint/StaticMesh or UE Asset mutation is not part of this review; USER final replacement mesh/socket handling remains deferred for P0-04/Product and P0-06/Runtime validation.

**Review-only verdict:** P0-02 source facts **24/24 verified**, full Evidence acceptance **HOLD**; P0-03 source contract **READ/REVIEW PASS with exact2 explicit code implementation corrections**, P0-03 code/tests/build **NOT RUN**. P0-00 Design PASS preserved. Work remains Ready / VCCAS-P0-02 with next gate `P0-03A reviewed vehicle schema/ownership implementation and focused tests` (without opening R2 before approved capability seam).

---

## 13.12 VCCAS-P0-03A — read-only Vehicle schema 최초 코드 작성 (2026-10-15)

사용자 요청으로 실제 차량 제작 진행을 재개했다. 직전 사용자 준비 자산은 CityCar/Compact/Coupe/Pickup/SubCompact/Van에서 Wheel Socket4 + Hardpoint Socket2씩, 기존 Sedan/SUV/Wagon에서도 필수 Wheel Socket4와 별도 Hardpoint가 새 AssetDump/UE evidence로 확인됐다. **메시/소켓 Asset 사용자 변경은 보존한다.** 현 메시와 추후 교체 메시의 게임 물리·실차 일치 및 런타임 승인까지 검증된 것은 아니다.

P0-03A 최소 코드 산출물:
- 신규 `Public/DataAuthoring/CFVehicleCCASProvider.h`, `Private/DataAuthoring/CFVehicleCCASProvider.cpp`: 기존 `ICFContentProvider`에 대한 `vehicle` exact type/SchemaRevision1, `Vehicle.Main` + Source/Claim/ClaimCitation/UnknownFact 4개 child의 read-only 정규화 schema.
- `ExternalManaged(VehicleBuilder)` 차체 카테고리·리소스와 `ReadOnly/Derived` 참고 제원/Evidence 구분. FACT citation 없는 claim, 잘못된 Source/Claim/Unknown ID, canonical `vehicle_<RecipeId>` 아닌 identity, Managed record를 차단.
- 신규 `Private/DataAuthoring/CFVehicleCCASProviderTests.cpp`: schema/registry, duplicate, 정확한 GUID identity, FACT citation, explicit UnknownFact, fail-closed Apply/Import 테스트.
- **중요:** 새로운 Provider는 현 단계에서 Production registry 자동 등록/실제 Excel Workbook 수정/Legacy Product import/Source Evidence UAsset 제작/AI R2/Target VehicleData 생성·저장을 열지 않는다. BuildCurrentSnapshot/ImportExisting/ApplyReviewed 등 P0-03B~P0-04 미구현 기능은 성공처럼 가짜 빈 기록을 만들지 않고 실패한다.

**검증 상태 (2026-10-15 갱신):** USER가 당시 P0-03A 소스를 포함하는 공식 UE 5.8 Editor 빌드 성공을 직접 보고했다. **BUILD USER-REPORTED PASS**로 접수하되, AI가 빌드 로그를 독립적으로 회수/검증한 것은 아니다. CarFightMCP_Admin 관리형 `build.run` Project identity는 `capability_not_available`, legacy build preset은 `build_preset_invalid`로 미시작이었다. 후속 focused Automation을 관리형 실행하려 했으나 generic `process.run`에서 `process_repository_not_found`, UE MCP `ue.status`에서는 `ERR_UE_MCP_TIMEOUT`이 발생해 **Automation NOT RUN**이다. 임의 raw shell/타 권한 우회는 하지 않았다.

**남은 동일 gate 작업:** 아래 v1.0.3/v1.0.2 correction의 컴파일은 기존 USER 빌드로 입증되지 않는다. 새로 변경한 소스의 공식 빌드 검증 → 허용된 기존 테스트 실행 경로 확보 → `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` focused Automation 실행 → 직접 영향 CCAS/Vehicle 회귀 → 실제 결과를 반영한 코드 검수 및 P0-03A 수락. P0-03B readback/import 및 P0-03C capability-bound AI R2 후 P0-04 6종 Recipe/VehicleData/Workbook 저작을 진행한다.

---

### 13.12.1 P0-03A Claim identity 보강 (2026-10-15)

기존 P0-03A `FCFVehicleCCASProvider::ValidateRecord`를 소스 재검수하던 중, FACT/DERIVED Claim이 `ClaimId`와 `Provenance`를 포함하고 citation이 존재하면 정작 그 주장의 **`ReferenceVehicleId` 또는 `FactKey`를 생략해도 통과**하는 semantic bug를 발견했다. 다차량 등록 시 출처를 어느 실차·제원에 결속해야 하는지 모호해지므로 실제 필드 identity의 누락을 차단한다.

- `CFVehicleCCASProvider.cpp v1.0.3`: ClaimId 고유성 + ReferenceVehicleId/FactKey non-empty를 같은 validation unit으로 검사. 기존 schema/Source/Citation/Unknown/권한 경계 불변.
- `CFVehicleCCASProviderTests.cpp v1.0.2`: 정상 Claim fixture의 ID/FactKey를 명시하고, 둘 중 하나가 없으면 `VehicleClaimIdentityInvalid` 반환, 원본 정상 Claim 유지, 기존 citation/Unknown/Apply fail-closed를 검증하는 focused negative tests 추가.
- **검증 상태:** 수정된 파일의 관리자-managed exact readback **PASS**, UE MCP는 `ERR_UE_MCP_UNAVAILABLE`로 연결할 수 없어 Automation 실행 **NOT RUN**. 기존 관리형 스크립트 실행 또한 앞서 legacy repository 선택이 차단됐다. **USER의 이전 공식 Editor Build 성공은 이번 v1.0.3/v1.0.2 이후 소스를 컴파일한 증거가 아니다.** 최신 변경 공식 Build/Automation은 **NOT RUN**. 이번 변경에서 UE Asset, Workbook, Repo 공용 Core, Catalog writer 수정 없음.
- 정확한 다음: 신규 검증 코드의 공식 UE 5.8 빌드 → 기존 `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` 단일 focused Automation → 인접 CCAS registry/vehicle validation 영향 회귀 → P0-03B 진입 판단. 관리형 테스트 실행을 위해 별도 툴이나 반복적인 임시 실행 시스템을 만들지 않는다.

---

### 13.12.2 P0-03B 실제 소스 사전검수 — readback/diff physical dependency 분리 (2026-10-15)

이 절은 **P0-03A 최신 코드 빌드 및 focused Automation 미실시 상태에서 행한 P0-03B 설계/소스 검수**다. 아래 항목을 P0-03B 구현 완료·테스트 PASS로 해석하지 않는다.

- `FCFContentCatalogVM::RefreshLiveCatalog`(`CFContentCatalog.cpp`)는 모든 `UCFVehicleRecipeData`를 AssetRegistry에서 열거하고 이미 `vehicle_<RecipeId.ToString(Digits).ToLower()>` key를 내보낸다. 이는 기존 Catalog의 **UI/Resource 읽기 projection**이지 CCAS Provider에 대한 fully typed `ImportExisting`/스냅샷 구현이 아니다. 기존 Catalog를 변경하거나 두 번째 Vehicle identity registry를 만들지 않는다.
- `FCFVehicleResourcePilot::Build`(`CFContentPilot.cpp`)는 `BuildRecipeSnapshot`에 더해 `FCFVehicleAssetReader::BuildAssetSnapshot`을 호출하고 실제 소켓·메시 resource/capability를 검사한다. 따라서 **최종 차체 또는 소켓이 없는 research-only/PreRecipe 입력에 대한 필수 import prerequisite로 사용하면 잘못된 blocking이 발생**한다. P0-03B의 논리 Recipe/Evidence/Target readback과 optional physical resource validation은 **같은 source identity를 보존하면서 다른 eligibility**로 평가한다. Pilot은 물리 준비가 된 경우에만 추가 resource proof에 사용한다.
- `UCFVehicleRecipeData::RecipeId`, exact bound `TargetVehicleData`, `UCFVehicleRefEvidence`의 `TargetRecipeId/TargetRecipePath/TargetDefinitionPath/EvidenceFingerprint` 및 실제 FACT/Unknown/Source가 있는 경우에만 source-aware record로 projection한다. Source가 없거나 unresolved인 경우에는 임의 FACT·fingerprint·숫자0을 생성하지 않는다. **ResearchCandidate에는 여전히 Product ContentKey를 발급하지 않는다.**
- 기존 `FCFContentCompiler::BuildDiffsAndFingerprints`는 `ImportExisting`에서 읽은 Product record와 `BuildCurrentSnapshot`이 즉시 재조회한 record의 canonical projection fingerprint가 동일해야 하고, Provider diff에는 canonical SHA-256 current/desired fingerprint가 존재해야 함을 요구한다. 여기에 맞춰 실제 current snapshot의 출처 fingerprint/target identity를 도메인 authority의 **same-state readback**으로 제공하고, 호출마다 바뀌는 UObject 포인터/AssetRegistry 순서/임시 메모리값에 의존하지 않는다.
- `Vehicle.Main`의 `ExternalManaged(VehicleBuilder)` 차체·하드포인트·Layout과 `ReadOnly` Reference FACT는 차이 비교에서 독립 workbook writer로 승격하지 않는다. CCAS-managed 필드가 후속 도입되기 전에는 외부 관리 필드가 달라져도 무단 Target writer를 실행하거나 새로운 Resource/RecipeId를 생성해서는 안 된다. `P0-03A` fail-closed mutation boundary를 유지한다.
- **영향받는 소비자와 focused tests:** existing CCAS Provider registry 및 Compiler `CurrentSnapshotChanged`/`ProviderCurrentFingerprintMismatch`/duplicate guard, `VehicleContentCatalog` 기존 `vehicle_<RecipeId>` key, VehicleBuilder `Recipe` 및 shared-mesh/no-final-mesh branch. 동일 StaticMesh를 쓰는 두 Recipe는 동일 `ChassisResource`를 참조하더라도 서로 다른 key/RecipeFingerprint를 가져야 한다. 다른 차량의 사용자 메시/Recipe/Target/Workbook writer는 미변경.
- **구현 선행 게이트:** `CFVehicleCCASProvider.cpp v1.0.3`와 `CFVehicleCCASProviderTests.cpp v1.0.2` 공식 빌드 및 `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` 실행 결과. 현재 UE MCP는 `ERR_UE_MCP_UNAVAILABLE`; 관리형 Project build capability와 legacy process script registration도 확인되지 않아 자동 테스트를 성공으로 선언할 수 없다. 실행 도구 추가/GoPyMCP 변경으로 이 작업을 확장하지 않는다.

### 13.12.3 다른 세션의 최신 Editor 빌드 증거 확인 (2026-10-15)

**현재 최신 상태 — 과거 §13.12.1~.2의 NOT RUN 기록을 대체:** USER가 다른 세션에서 공식 UE 5.8 Editor 빌드를 성공시켰다고 보고했다. CarFightMCP_Admin를 통한 **현 저장소의 실제 파일 readback**에서 아래 순서를 확인했다.

1. `CFVehicleCCASProvider.cpp v1.0.3` 소스 mtime: **2026-10-15 10:19:20 KST**.
2. `CFVehicleCCASProviderTests.cpp v1.0.2` 소스 mtime: **2026-10-15 10:19:33 KST**.
3. `UE/Binaries/Win64/CarFight_ReEditor.target` (Win64/Development/Editor, UE5.8) 갱신 mtime: **2026-10-15 11:09:36 KST**.
4. `UE/Saved/CarFight/DataAuthoringAutomationResult.json` mtime: **2026-10-15 11:12:49 KST**, `EngineExitCode=0`, `status=success`, `success_count=1`, `failure_count=0`. 실행한 테스트는 **`CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate`**이며, `UE/Saved/Logs/CFDataAuthoringAutomation.log`의 11:12 시작 정보와 일치한다.

**판정:** 최신 P0-03A 소스 수정 뒤 Editor target receipt 재생성 및 후속 Editor Automation 실행이 확인되어 USER 빌드 성공 보고와 교차 일치한다. 다만 **직접 BuildEditor.bat의 최종 exit code/UnrealBuildTool summary를 회수한 것은 아니다**. `VehicleSchema` focused test는 실행 결과가 없으므로 **NOT RUN**이고 위 타 기능의 1/1 PASS를 CF-FQ-059의 focused Automation PASS로 전용하지 않는다. **동일 코드의 불필요한 반복 빌드는 요구하지 않는다.**

**다음 정확한 Gate:** `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` focused Automation 및 직접 영향 CCAS regression 수행 → P0-03A 수락 → P0-03B 구현. 새 차량 6종 Product/Workbook/UE Asset은 이번 증거 검수 중 변경하지 않았다.

---

### 13.12.4 P0-03A VehicleSchema 실제 실행 실패 및 focused correction (2026-10-15)

USER가 기존 `Tools/RunDataAuthoringTests.ps1 -ExactTestNames "CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema" -SuppressModelContextProtocolLog`을 직접 실행했다. 제출한 실제 로그와 `UE/Saved/CarFight/DataAuthoringAutomationResult.json`을 교차 확인했다.

- `EXECUTION_MODE=exact_list_same_process`, `ENGINE_EXIT_CODE=0`, 요청/발견 **1/1**, `SUCCESS_COUNT=0`, `FAILURE_COUNT=1`, `MISSING/UNEXPECTED/DUPLICATE=0`.
- 2026-10-15 14:22:50 KST(UE 로그 05:22:50 UTC), `CFVehicleCCASProviderTests.cpp:159`의 **`Expected 'Mixed case Vehicle is rejected' to be false`**가 유일하게 명시된 해당 테스트 실패 이벤트이다. 테스트 실행 전 출력된 반복 `LogAutomationTest: Error: Condition failed`는 본 테스트의 개별 검증 15건 실패를 의미하지 않는다. 추가 실패를 임의로 단정하지 않는다.
- **근본 원인:** `FCFVehicleCCASProvider::ValidateRecord`가 ContentType `FString`을 `== TEXT("vehicle")`로 비교해 대소문자 구분을 강제하지 않았다. RowId에서도 `!=`로 같은 canonical case-only 불일치를 놓칠 수 있었다. 기존 CCAS Provider Registry는 `FString::Equals(..., ESearchCase::CaseSensitive)`를 사용한다.
- **교정:** `CFVehicleCCASProvider.cpp v1.0.4`에서 ContentType와 RowId를 모두 명시적 `ESearchCase::CaseSensitive` 비교로 바꿨다. `CFVehicleCCASProviderTests.cpp v1.0.3`에서 mixed-case ContentType의 정확한 `InvalidVehicleContentKey` 진단과 case-only RowId의 `VehicleRowIdMismatch` 진단을 검증하는 negative tests를 보강했다. Header/interface, shared CCAS Core/Registry, Product/Workbook/Recipe, UE Asset 및 타 세션 Dirty 변경 없음.
- **현재 판정:** **PRE-CORRECTION AUTOMATION FAIL (0/1), CORRECTION SOURCE READBACK PASS, POST-CORRECTION BUILD/TEST NOT RUN.** 기존 빌드·로그의 PASS를 새로운 v1.0.4/v1.0.3의 PASS로 승격하지 않는다.
- **Next:** 최신 Provider·Test 소스를 공식 UE 5.8로 빌드 → 같은 exact test 1건 재실행 → failure/missing/unexpected/duplicate=0 + success=1 + run exit=0 확인 → 연관 CCAS/Vehicle identity 검증 → P0-03B 진입. 빌드와 테스트를 불필요하게 반복하거나 새로운 runner를 신설하지 않는다.

---

### 13.12.5 P0-03A VehicleSchema 교정 후 집중 테스트 PASS (2026-10-15)

USER가 최신 P0-03A case-sensitive 교정 후 기존 `Tools/RunDataAuthoringTests.ps1`에서 정확히 `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema`를 재실행했다. 현 저장소의 `UE/Saved/CarFight/DataAuthoringAutomationResult.json` **원본 readback**으로 다음 결과를 확인했다.

- `schema_version=carfight_data_authoring_automation_result_v2`
- `status=success`, `execution_mode=exact_list_same_process`, `requested_test_count=1`
- `ENGINE_EXIT_CODE=0`
- `SUCCESS_COUNT=1`, `FAILURE_COUNT=0`, `MISSING_COUNT=0`, `UNEXPECTED_COUNT=0`, `DUPLICATE_TERMINAL_COUNT=0`
- `PASS=CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema`

**결론:** P0-03A의 해당 focused Automation **PASS 1/1**; 지난 v1.0.3/v1.0.2의 실패 이벤트는 Historical로 유지하고 현재 실패로 간주하지 않는다. 새 코드 `CFVehicleCCASProvider.cpp v1.0.4`, `CFVehicleCCASProviderTests.cpp v1.0.3`에 대한 성공 기록이며 직접 UBT compile summary의 독립 조회는 수행하지 않았으나 새 Automation에서 테스트가 발견되고 실행됐다.

**P0-03A 최종 수락 전 남은 범위:** 바로 영향을 받는 기존 CCAS Provider Registry 및 canonical key/identity 회귀 결과 재확인, code/doc/diff final review. 이 단일 테스트 PASS를 전체 CCAS/Vehicle Builder/Runtime 또는 향후 P0-03B import·diff PASS로 확대하지 않는다. P0-02 실차 exact trim·Evidence 최종 승인과 P0-04 VehicleData/Workbook 제작은 별개로 남아 있다. User·other session Dirty, UE Asset, 기존 Product/Workbook은 이 기록 중 수정하지 않았다.

---

### 13.12.6 P0-03A 인접 CCAS 소스 회귀·최종 기술검수 (2026-10-15)

**검수 대상:** `CFVehicleCCASProvider.cpp v1.0.5`, `CFVehicleCCASProviderTests.cpp v1.0.4`, Provider interface `CFContentProvider.h v1.1.0`, registry `CFContentProvider.cpp v1.2.0`, canonical type/key `CFContentTypes.h v1.2.0`, CCAS Core/Compiler, `Tools/RunDataAuthoringTests.ps1 v1.3.1`, actual preserved Git status. 사용자 및 다른 세션의 VehicleBuilder/StaticMesh/Workbook dirty는 **읽기 전용 조사만 수행하고 일절 수정하지 않았다**.

**PASS — 정적 계약/영향 검수:**

1. `vehicle_<RecipeId digits lowercase>` identity, `RowId`의 exact case-sensitive 대조, zero/invalid GUID 차단이 `FCFContentKey`의 case-sensitive identity 및 현재 Catalog의 `vehicle_` rule과 일치한다. 기존 단일 `FCFContentProviderRegistry`에 별도 instance로 검증되지만 **Production 자동 등록/타 타입 변경은 없다**.
2. `Vehicle.Main`/4 child schema의 ownership은 `ECFContentFieldOwnership::ExternalManaged(VehicleBuilder)` / `ReadOnly` / `Derived`로 구분되고, `FCFContentManagementState::ExternalReadOnly` Product record는 계속 read-only다. 실제 차체 Mesh가 없어도 schema·Evidence 검증은 실행할 수 있다.
3. `SourceId`/ClaimId 중복, FACT citation→실제 Source/Claim 연결, Claim의 `ReferenceVehicleId`/FactKey, explicit UnknownFact identity, GAME_BIAS exclusion을 source-aware 방식으로 검증한다.
4. `ImportExisting`, `BuildCurrentSnapshot`, `BuildDependencyEdges`, `BuildDiff`, `BuildReviewedMutationPlan`, `ApplyReviewed`, `ReadbackFingerprint`는 P0-03A 동안 모두 fail-closed하며 CCAS Workbook/VehicleData/Recipe/Product의 신규 writer를 열지 않는다.
5. `FCFVehicleCCASProvider` 참조 검색에서 Production consumer/registry 자동 등록은 없고 Header/CPP 및 synthetic test만 참조한다. Project Editor source 변경은 이 두 새 파일에만 국한된다.

**발견·국소 교정:** static final review에서 Claim `Provenance == TEXT("FACT")` / `!= TEXT("DERIVED")`도 `FString` 비교 기본 case에 의존해 소문자 `fact`/`derived`를 정식 `ECFContentValueType::Enum` 토큰으로 오인할 수 있음을 발견했다. `CFVehicleCCASProvider.cpp v1.0.5`에서 `FACT`·`DERIVED` 모두 `Equals(..., ESearchCase::CaseSensitive)`로 비교하고, `CFVehicleCCASProviderTests.cpp v1.0.4`에서 정확한 `FACT`/`DERIVED` 승인 및 lowercase `fact`/`derived`/`GAME_BIAS` 차단을 focused in-memory regression에 추가했다. 같은 semantic unit의 주변 비교를 재감사했으며 key/row는 기존 명시적 case-sensitive를 유지하고 NameId는 공용 Core의 canonical lower-case 해시 계약을 보존한다. **추가 Infrastructure/Provider/API/공용 Core/UE Asset 수정 없음**.

**실제 회귀 게이트 — NOT RUN / PASS를 가장하지 않음:** 기존 `VehicleSchema` 1/1 PASS는 **교정 직전 Provider v1.0.4/Tests v1.0.3 판본**의 확정 결과다. 현 최신 v1.0.5/v1.0.4는 관리형 source write/readback **PASS**, **공식 Editor Build/Automation NOT RUN**이다. 이번 회귀 대상은 정확히 다음 **5개**다.

- `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` — 최신 provenance token correction + 기존 focused canonical identity 재검증
- `CarFight.CCAS.CF_FQ_058.P0_01.ProviderRegistry` — 기존 provider registry exact registration/duplicate/fail-closed
- `CarFight.CCAS.CF_FQ_058.P0_02.ProviderValidation` — blocking semantic issue와 Apply 0
- `CarFight.CCAS.CF_FQ_058.P0_02.SchemaCompatibility` — Provider schema mismatch와 Apply 0
- `CarFight.CCAS.CF_FQ_058.P0_02.SnapshotConsistency` — 불일치 current readback/second import fail-closed

모두 확인된 `IMPLEMENT_SIMPLE_AUTOMATION_TEST` 이름이며 in-memory/읽기 전용 focused tests이다. 기존 `Tools/RunDataAuthoringTests.ps1`의 `-ExactTestNames`는 한 Editor process에서 64개 이하의 exact test를 실행하고 JSON v2에 누락/예상외/중복 여부까지 기록한다. **완료 기준:** 정확히 5/5 PASS, EngineExitCode0, failure/missing/unexpected/duplicate 0, latest source build 확인, final diff/readback 이상0. 현재 세션의 Admin 관리형 process repository는 미등록이고 UE MCP 연결이 없어 이 실행을 대신 성공했다고 주장하지 않는다. **P0-03A 최종 Technical Acceptance는 이 5건 실측까지 HOLD**이며 P0-03B Production import/diff는 아직 열지 않는다.

---

### 13.12.7 P0-03A 최종 5건 회귀 실측 및 Technical Acceptance (2026-10-15)

USER가 최신 `CFVehicleCCASProvider.cpp v1.0.5` / `CFVehicleCCASProviderTests.cpp v1.0.4` 기준으로 공식 UE 5.8 Editor `Tools/RunDataAuthoringTests.ps1 -ExactTestNames` exact5를 한 프로세스에서 실행했다. CarFightMCP_Admin로 동일 저장소의 **`UE/Saved/CarFight/DataAuthoringAutomationResult.json`을 독립 재조회**했고 다음 결과가 일치한다.

- `status=success`, `execution_mode=exact_list_same_process`, `requested_test_count=5`, `engine_exit_code=0`
- `success_count=5`, `failure_count=0`, `missing_count=0`, `unexpected_count=0`, `duplicate_terminal_count=0`
- **PASS:** `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema`
- **PASS:** `CarFight.CCAS.CF_FQ_058.P0_01.ProviderRegistry`
- **PASS:** `CarFight.CCAS.CF_FQ_058.P0_02.ProviderValidation`
- **PASS:** `CarFight.CCAS.CF_FQ_058.P0_02.SchemaCompatibility`
- **PASS:** `CarFight.CCAS.CF_FQ_058.P0_02.SnapshotConsistency`

소스 파일 헤더를 다시 읽어 `v1.0.5/v1.0.4` 일치를 확인했으며, 신규 Production Provider 자동 등록과 Product/Workbook/VehicleData mutation0의 기존 fail-closed 경계를 정적 검수했다. 이전 §13.12.4 최초 실패, §13.12.5 1건 PASS, §13.12.6 당시 pending은 **그 시점의 Historical 증거**로 보존한다. 이번 exact5 terminal receipt가 가장 최신 수락 증거다. 별도 UBT 최종 exit log는 직접 회수하지 않았지만 USER 공식 빌드 및 **최신 코드에서 실행된 Engine Automation PASS 5/5**를 교차 수락한다. 기존 빌드·테스트를 다시 실행하지 않는다.

**결론 — VCCAS-P0-03A TECHNICAL PASS / CLOSED (phase only).** 최초 Vehicle schema/ownership/identity/Claim citation/UnknownFacts/기존 Registry/Compiler 안전 경계는 이번 단계에서 수락됐다. 이 수락은 P0-02 exact reference trim/Evidence의 최종 승인, P0-03B typed import/readback/diff, P0-03C trusted R2, P0-04 Production 6 VehicleData/Workbook, 최종 메시·Socket 및 Runtime 검증을 완료했다는 의미가 **아니다**.

**Next phase — VCCAS-P0-03B READY (implementation NOT STARTED):** 기존 Recipe/Target/Evidence authority의 immutable identity를 읽는 `ImportExisting`, same-state `BuildCurrentSnapshot`, source-aware current/desired `BuildDiff`를 기존 Compiler/Registry의 canonical fingerprint 계약에 맞게 단계적으로 구현한다. 앞선 §13.12.2의 `FCFVehicleResourcePilot` 물리 Mesh/Socket 선행 차단을 research-only source readback에 적용하지 않는다. 새 차량/메시별 하드코딩이나 병렬 writer를 추가하지 않고 Product mutation fail-closed를 유지한다. P0-02 final Evidence HOLD는 독립적으로 유지한다.

---

### 13.12.8 P0-03B Typed Source Readback / Current / Diff 구현 checkpoint (2026-10-15)

**기준선:** P0-03A 최종 `v1.0.5/v1.0.4` 및 USER exact5 **5/5 PASS**는 보존한다. 이번 §13.12.8은 다음 단계인 **P0-03B의 새 소스 변경**이므로 앞선 PASS로 최신 코드까지 검증됐다고 주장하지 않는다.

**적용 소스 (관리형 repo write/readback):**

- `CFVehicleCCASProvider.h v1.1.0`: persisted Recipe/Evidence와 별도 synthetic tests가 사용하는 read-only `BuildReadOnlyRecordFromSources` projection 진입점 공개; 기존 Provider interface 시그니처 유지.
- `CFVehicleCCASProvider.cpp v1.1.0`: schema revision **2** (Recipe/Target/Evidence AssetPath, Chassis AssetPath, Claim Boolean/Range value 보존); 기존 `FCFVehicleResourcePilot`와 **같은** `vehicle.<RecipeId lowercase digits>.chassis` ResourceId 사용. 실제 메시 경로는 별도 `ChassisAssetPath` read-only snapshot field로 표현하므로 같은 메시를 공유하는 차량도 개별 RecipeId/ResourceId를 유지하고 한 차량의 메시 교체를 fingerprint/diff가 감지할 수 있다.
- 기존 AssetRegistry `UCFVehicleRecipeData` 및 정확히 매칭한 `UCFVehicleRefEvidence`를 읽는 `ImportExisting`: RecipeId 중복·오류, Evidence 중복/잘못된 Recipe/Definition binding, stale Evidence semantic fingerprint 및 미저장(Dirty) source를 fail-closed. **일부 성공한 records만 반환하지 않는다.** Evidence가 아직 없는 Recipe는 FACT/0/EvidenceFingerprint를 생성하지 않고 Recipe 자체의 read-only 정보만 반환한다.
- 기존 Source `ReferenceVehicles` Primary, `Sources`, `Claims` Canonical FACT/DERIVED, `ClaimCitations`, `UnknownFacts`를 typed in-memory Record에 투영한다. Source fact가 없는 수치를 생성하지 않으며 Main 4치수는 Primary의 Canonical numeric Claim과 정확한 `mm` 단위에서만 채운다. 중복 Primary/치수 및 unresolved blocking Conflict는 차단한다. `GAME_BIAS`와 Candidate/Rejected Claim을 OEM FACT로 승격하지 않는다.
- `BuildCurrentSnapshot`: 현재 persisted source를 **재조회**하고 기존 `FCFContentSemanticHasher`로 deterministic SHA-256 readback fingerprint 산출. `BuildDiff`: Current/Desired identity·fingerprint 검사 뒤 scalar field와 source child collection별 changed paths를 제공. `ReadbackFingerprint`: 동일 authoritative snapshot 재사용. `BuildDependencyEdges`: 현재 source dependency가 동일 Vehicle Evidence 내부에만 존재해 허위 ContentKey graph edge 0을 반환(semantic validation 통과 조건). `BuildReviewedMutationPlan` 및 `ApplyReviewed`는 **계속 fail-closed**.
- `CFVehicleCCASProviderTests.cpp v1.1.0`: 기존 P0-03A `VehicleSchema` 유지(단 schema revision 2 확인) 및 새로운 독립 `CarFight.CCAS.CF_FQ_059.P0_03B.VehicleReadDiff` 등록. No-mesh/No-Evidence, 동일 메시 공유와 분리된 Recipe/ResourceId, 정상 Source/Citation/Unknown, stale/mismatched Evidence 차단, canonical fingerprint 동일/변경 diff, 승인 flag를 이용한 mutation0 회귀를 transient fixtures로 구성. **사용자 AssetRegistry/Workbook 저장 없음.**

**검증 상태 (정확한 분리):** 세 파일의 승인된 exact managed patch/readback **PASS**. 새 P0-03B 소스에 대한 공식 UE 5.8 Source Editor **BUILD NOT RUN**, `VehicleReadDiff`와 영향 CCAS focused Automation **NOT RUN**, persisted Recipe/Evidence live readback **NOT RUN**. `P0-03B TECHNICAL PASS`로 승격하지 않는다. 기존 P0-03A 5/5 수락은 Historical baseline으로 보존한다.

**다음 정확한 Gate — 공식 UE 5.8 빌드 → same-process exact9 회귀:**

1. `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema`
2. `CarFight.CCAS.CF_FQ_059.P0_03B.VehicleReadDiff`
3. `CarFight.CCAS.CF_FQ_058.P0_01.ProviderRegistry`
4. `CarFight.CCAS.CF_FQ_058.P0_02.ProviderValidation`
5. `CarFight.CCAS.CF_FQ_058.P0_02.DiffDrift`
6. `CarFight.CCAS.CF_FQ_058.P0_02.FailClosed`
7. `CarFight.CCAS.CF_FQ_058.P0_02.FingerprintGuard`
8. `CarFight.CCAS.CF_FQ_058.P0_02.SchemaCompatibility`
9. `CarFight.CCAS.CF_FQ_058.P0_02.SnapshotConsistency`

요구: EngineExitCode0 + SUCCESS9/FAILURE0/MISSING0/UNEXPECTED0/DUPLICATE0. 그다음 실제 persisted Recipe/Evidence의 nonmutating readback과 snapshot consistency, 별도 P0-02 Evidence HOLD에 따른 missing source 표시, source/Asset 변경에 따른 stale 감지를 검증한다. 새 보안/Project 실행 우회 도구는 만들지 않는다.

---

### 13.12.9 P0-03B 공식 Editor USER Build PASS 확인 (2026-10-15)

USER가 현재 P0-03B Provider/Header/Tests v1.1.0에 대해 공식 UE 5.8 Source Editor 빌드를 직접 실행해 **성공**했다고 보고했다. CarFightMCP_Admin로 `UE/Binaries/Win64/CarFight_ReEditor.target` Win64/Development Editor receipt 수정 시각 **2026-10-15 15:55:34 KST**를 재조회했으며, 현재 `CFVehicleCCASProvider.cpp` 소스 수정 **15:43:39 KST**, `CFVehicleCCASProviderTests.cpp` 소스 수정 **15:43:29 KST**보다 나중이다. 최신 산출물과 USER 보고는 일치한다. 독립적인 UnrealBuildTool 최종 종료코드 전문은 회수하지 않았으므로 `USER BUILD PASS + updated official target receipt corroborated`로 구분한다. **같은 빌드를 다시 실행하지 않는다.**

현재 `UE/Saved/CarFight/DataAuthoringAutomationResult.json`은 별개 `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate`의 이전 실행 결과이고 **P0-03B focused Automation 결과가 아니다**. VehicleReadDiff 포함 exact9는 **NOT RUN**이다. 다음 단계는 기존 `Tools/RunDataAuthoringTests.ps1 -ExactTestNames`를 통해 §13.12.8 exact9를 한 Editor 프로세스에서 실행하여 `ENGINE_EXIT_CODE=0`, `SUCCESS_COUNT=9`, 실패/누락/예상외/중복 0을 검증하는 것이다. 이어 persisted Recipe/Evidence read-only smoke 및 최종 diff/slop 검수를 수행하고 **P0-03B TECHNICAL ACCEPTANCE 여부를 판정**한다. 실제 DataAsset/Workbook 변경·Git stage/commit/push 없음.

---

### 13.12.10 P0-03B exact9 실제 실행 PASS 및 persisted-source 검증 경계 (2026-10-16)

USER가 `Tools/RunDataAuthoringTests.ps1 -ExactTestNames $tests -SuppressModelContextProtocolLog`을 실행한 stdout과 현재 CarFight 저장소의 `UE/Saved/CarFight/DataAuthoringAutomationResult.json`을 **독립 재조회·교차 검증**했다. 최신 P0-03B Provider/Header/Tests `v1.1.0`의 USER 공식 UE5.8 Editor Build PASS 및 target receipt 2026-10-15 15:55:34 KST는 이전 §13.12.9의 승인된 기준선이다.

- `schema_version=carfight_data_authoring_automation_result_v2`, `status=success`, `execution_mode=exact_list_same_process`, `requested_test_count=9`
- `engine_exit_code=0`, `success_count=9`, `failure_count=0`, `missing_count=0`, `unexpected_count=0`, `duplicate_terminal_count=0`
- `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` — **PASS**
- `CarFight.CCAS.CF_FQ_059.P0_03B.VehicleReadDiff` — **PASS**
- `CarFight.CCAS.CF_FQ_058.P0_01.ProviderRegistry` — **PASS**
- `CarFight.CCAS.CF_FQ_058.P0_02.ProviderValidation` — **PASS**
- `CarFight.CCAS.CF_FQ_058.P0_02.DiffDrift` — **PASS**
- `CarFight.CCAS.CF_FQ_058.P0_02.FailClosed` — **PASS**
- `CarFight.CCAS.CF_FQ_058.P0_02.FingerprintGuard` — **PASS**
- `CarFight.CCAS.CF_FQ_058.P0_02.SchemaCompatibility` — **PASS**
- `CarFight.CCAS.CF_FQ_058.P0_02.SnapshotConsistency` — **PASS**

**최종 영향·Slop 정적검수:** `ImportExisting`는 단일 기존 AssetRegistry의 저장된 `UCFVehicleRecipeData`/`UCFVehicleRefEvidence`를 읽고 RecipeId/Evidence fingerprint/dirty/binding을 검증한다. `BuildCurrentSnapshot`은 현재 source를 다시 조회하며, `BuildDiff`는 기존 CCAS canonical fingerprint와 typed Source child collection을 비교한다. `BuildReviewedMutationPlan`과 `ApplyReviewed`는 여전히 **fail-closed**. 새 Product/Workbook writer·registry·차종별 분기·전용 관리 도구를 추가하지 않았고 다른 세션 Dirty에 쓰기 접근하지 않았다. 이번 검수에서 새로운 변경 필요성이 확인되지 않아 추가 C++ 수정이나 중복 빌드·테스트는 하지 않았다.

**정확한 판정:** `P0-03B OFFICIAL BUILD USER PASS + SOURCE RECEIPT CORROBORATED`, `P0-03B FOCUSED AUTOMATION EXACT9 PASS`, `POST-IMPLEMENTATION STATIC REVIEW PASS`. 다만 실제 persisted Recipe/Evidence를 `ImportExisting`·`BuildCurrentSnapshot`으로 조회한 **라이브 read-only smoke는 NOT RUN / BLOCKED**다. UE Editor read-only MCP `ue_status` 및 `ue_list_toolsets`가 현재 내부 오류를 반환해 실제 asset 조회 경로를 사용할 수 없었다. Synthetic `VehicleReadDiff` PASS만으로 실제 모든 저장 자산의 binding/fingerprint/dirty 상태를 수락했다고 해석하지 않는다. 따라서 §13.12.8에 정의된 **P0-03B 전체 Technical Acceptance는 persisted smoke가 복구·실측될 때까지 HOLD**다. 신규 테스트나 임시 실행 경로를 만들지 않는다.

**다음 실행:** UE 연결이 정상화되면 저장된 Recipe/Evidence의 현재 인스턴스를 읽기 전용으로 조회하여 RecipeId, Target binding, Source/Citation/Unknown projection, current fingerprint 동일성 및 미저장 source 차단을 관찰한다. P0-02의 Core6 최종 실차 trim 승인 및 durable Evidence 생성이 아직 HOLD라면, **해당 미존재 source를 만들어내지 않고** 이미 저장된 Recipe의 no-Evidence readback과 P0-02 미완성 상태를 구분한다. P0-03C trusted AI R2, P0-04 6종 Production VehicleData/Workbook 제작은 별도 후속 단계다.

---

### 13.12.11 P0-03B 실제 저장 자산 fresh readback / FactKey 계약 불일치 발견 (2026-10-16)

**조사 방식과 freshness:** `CarFightMCP_Admin.ue_batchdump_safe`를 **`changed_only=false`, `rebuild_index=true`, `managed_dataset=true`**로 공식 CarFight 프로젝트에 실행했다. 옛 AssetDump index-only 조회에서는 Recipe exact2였지만, fresh dataset `adset_v1_4bf2142a2f4022e0574189deb953f79f.b4d70b730877d3f842ec4c06`의 `/Game/CarFight/Data/Authoring` 범위에서 `UCFVehicleRecipeData` **4/4 successfully saved-data read**를 확보했다. 별도 exact `UCFVehicleRefEvidence` fresh dataset `adset_v1_debf91509270f6f8099f5db1577e1d7f.d2bc377d23c264345a18be23`에서 **1/1** 성공했고, 전역 `UCFVehicleData` fresh dataset `adset_v1_61838d7304299ed6e833a41b8b7f43a3.cee4962fa4a8f158fa510d36`에서 **12/12**를 수집해 네 Recipe의 정확한 TargetVehicleData 경로가 모두 실제 저장된 `UCFVehicleData`라는 점을 교차 확인했다. 모든 세 작업의 `return_code=0`, `failed_count=0`, `fresh_preparation=true`; 현재 Git/UE asset 직접 mutation 없음.

**저장 Recipe roster / ID 검증:**

| Recipe | RecipeId (정확한 Digits) | Target 상태 | Evidence |
| --- | --- | --- | --- |
| `DA_Recipe_DefenseSUV` | `02493A1546EE2400458647BEE7FB8886` | `DA_VehicleDefense_TestSUV` 존재 | 없음; source fact 임의 생성 금지 |
| `DA_Recipe_TestSUV` | `B5BFD0DC426517FDB5465EB662EFF9C3` | `DA_TestSUV` 존재 | 없음 |
| `DA_Recipe_TestSedan` | `111E93394D0BA63B689A399C26295867` | `DA_TestSedan` 존재 | 없음 |
| `DA_Recipe_Wagon` | `05F69DD34990A868DEFFD29C1AF5B2F9` | `DA_Vehicle_Wagon` 존재 | `DA_Ref_Wagon` 정확 결속 |

4개 RecipeId는 **unique/valid**. DefenseSUV·TestSUV는 같은 SUV Chassis path를 참조하지만 서로 다른 RecipeId를 소유한다. 이에 따라 동일 메시에 대한 독립 차량 identity/read-only binding 계약도 **저장값 기준 PASS**다. 첫 3개 Recipe의 `BuilderCommitReceipt.EvidencePath=None`와 zero EvidenceId는 **Evidence 없음**을 의미하며, Source 사실을 임의 생성하거나 실패로 단정하지 않는다.

**Wagon Evidence fresh 확인:** `DA_Ref_Wagon`의 TargetRecipeId = `05F69DD34990A868DEFFD29C1AF5B2F9`, TargetRecipePath = `/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon`, TargetDefinitionPath = `/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon`가 정확히 일치한다. EvidenceId = `743FE516484A39E65DAC0E9D18EA8DAB`이며 EvidenceFingerprint = `b6204acf1d6722009075865204f33b693c96ed002b4c6ba6eed21751b40b535f`가 저장 Recipe BuilderCommitReceipt의 값과 일치한다. **References 3, Sources 5, Claims 20, UnknownFacts 5, Conflicts 0**가 직전 인덱스가 아닌 fresh data_asset_values로 확인됐다. 단 AssetDump Claim nested item preview는 최대 8/20이고, 이 stored fingerprint 값을 `UCFVehicleRefEvidence::BuildEvidenceFingerprint` 호출로 fresh 재계산한 검증은 아니다. 정확한 saved source value 동일성 PASS와 Provider runtime fingerprint PASS를 혼동하지 않는다.

**추가 발견 — 직접 영향받는 실제 FactKey 호환성 누락 (미교정):** 기존 Wagon canonical Source Claim은 `Dimensions.Length`, `Dimensions.Width`, `Dimensions.BodyWidth`, `Dimensions.Height`, `Dimensions.Wheelbase`를 사용한다. 실제 코드 `FCFVehicleCCASProviderPrivate::AddPrimaryDimensions`는 `ExteriorLengthMm`, `ExteriorWidthMm`, `ExteriorHeightMm`, `WheelbaseMm` 네 이름만 검사한다. `BuildReadOnlyRecordFromSources`가 Wagon에 대해 성공하더라도 **`Vehicle.Main`의 주요 치수가 Source Claim에서 투영되지 않을 수 있다는 정적 계약 불일치**를 발견했다. `Vehicle.Claims` 내 원본 Source/Fingerprint 보존 자체와는 구별한다. 특히 `Dimensions.Width`와 `Dimensions.BodyWidth`는 의미가 다를 수 있으므로 무검증 alias·가짜 값·임의 우선순위를 지정하지 않는다. 후속 교정에서는 실제 field semantics(차체 폭/미러 포함 여부), Primary/FactKey/UnitId를 명시해 정확한 매핑·중복/단위 거부 및 synthetic+persisted regression을 함께 검증해야 한다.

**기술 수락 게이트 상태:** `P0-03B BUILD USER PASS`, `focused exact9/9 PASS`, `fresh persisted 4 Recipe + 1 Evidence + 12 VehicleData readback PASS`까지 완료. 그러나 UE Editor live read-only connector `ue_status`가 내부 오류를 반환했고, **새 C++ Provider `ImportExisting`/ `BuildCurrentSnapshot`/ `ReadbackFingerprint`를 실제 저장 자산에 호출한 live smoke는 NOT RUN / BLOCKED**다. 실제 FactKey mapping mismatch도 아직 미교정이다. **P0-03B TECHNICAL ACCEPTANCE는 두 항목 해결·재검증 전 HOLD.** 이번 사용자 요청은 저장 자산 읽기 검증이므로 C++·UE asset·Workbook을 임의 수정하지 않았다. 단순 cached dump 재사용이나 9/9 synthetic PASS만으로 이 결함을 덮지 않는다.

---

### 13.12.12 USER 기준차량 범위 확정 — Wagon only / 기존 3개 Test Recipe 분리 (2026-10-16)

**사용자 확정 결정:** CF-FQ-059 P0-03B에서 **실차 제원·OEM Evidence·FactKey 매핑·정확한 fingerprint/current/diff 수락에 사용할 기존 차량 기준선은 Wagon 하나**다. 기존 `DA_Recipe_TestSedan`, `DA_Recipe_TestSUV`, `DA_Recipe_DefenseSUV`는 **실제 게임에 사용할 기준 차량이나 실차 스펙 검증 대상이 아닌 테스트 자산**이다. 테스트용 임의 수치를 제조사 확인 FACT로 해석하거나, 값 교정·실차 매핑·Reference Evidence 신규 작성·Production promotion을 요구하지 않는다. 테스트 자산을 삭제하거나 수정하지 않는다.

**검증 게이트를 분리한다:**

1. **Wagon acceptance (엄격):** `DA_Recipe_Wagon` → `DA_Ref_Wagon` → `DA_Vehicle_Wagon`의 exact saved binding/ID, Source/Claim/Unknown, OEM FactKey `Dimensions.*`와 Provider scalar fields 의미 대응(`Dimensions.Width` 대 `Dimensions.BodyWidth` 구분), unit, evidence fingerprint 최신성, `ImportExisting`/ `BuildCurrentSnapshot`/ `ReadbackFingerprint`/ `BuildDiff` 실제 읽기 경로를 검증한다. Source가 명시적으로 모르는 값은 Unknown으로 남긴다.
2. **Test assets (최소 안전성):** `DA_Recipe_TestSedan`, `DA_Recipe_TestSUV`, `DA_Recipe_DefenseSUV`는 RecipeId 유효/고유, persisted asset load, 정확한 자신만의 Target path, no-Evidence를 잘못된 FACT로 생성하지 않음, 동일 메시 공유 시 독립 Recipe identity 등의 **read-only 구조/호환성**만 확인한다. 해당 테스트 차의 차량 수치·차량 감각·차종 대표성·제조사 출처·Production readiness는 CF-FQ-059 P0-03B 수락 기준이 아니다.
3. **현재 ImportExisting 구현의 직접 영향 확인:** `FCFVehicleCCASProvider::ImportExisting`는 AssetRegistry의 **모든** `UCFVehicleRecipeData`를 읽어 모든 Record를 `BuildReadOnlyRecordFromSources`로 검증하며, 하나라도 실패하면 전체 import를 중단한다. 따라서 테스트 자산을 **real-world spec validation에서는 제외**하지만, 현재 reader가 실제 Wagon까지 읽지 못하게 만드는 파생 오류가 없는지 확인해야 한다. 테스트 자산을 자동으로 삭제·수정·숨기거나 **Wagon 전용 하드코딩으로 운영 로직을 분기하지 않는다**. 실패가 실제 발생한 경우 오류/필터링 scope와 기존 Catalog authority를 분석한 뒤 필요한 semantic unit만 교정한다. 새 Production 스코프를 테스트 에셋의 존재 여부에 의존하게 만들지 않는다.

**장래 범위와 분리:** Wagon만 기준이라는 결정은 **현재 저장된 기존 Recipe들 중 실차 검증용 기준 차량**에 관한 것이다. 후속 P0-04의 새로운 대표 차량 6종은 별도 실제 대표 모델·연식·트림·출처에 근거하여 제작해야 하며 Wagon의 수치나 세 테스트 Recipe의 임의 수치를 복사하지 않는다. 기존 Core6/Core9 first-wave 계획과 새 차량 추가 가능성을 취소하거나 축소한 것으로 해석하지 않는다.

**현재 상태:** 기존 UE5.8 USER Build PASS, CCAS focused exact9 9/9 PASS, fresh saved 4 Recipe/1 Evidence/12 VehicleData readback PASS 유지. Wagon `Dimensions.*` mapping compatibility 교정 및 실제 Provider live smoke는 **HOLD**. 이번 USER 범위 교정은 수락 기준·문서만 변경하며 C++/BP/Unreal Assets/Workbook 및 별도 Dirty를 수정하지 않는다.

---

### 13.12.13 Wagon-only 실제 FactKey mapping 교정·persisted Provider smoke 구현 (2026-10-16)

**범위:** USER가 확정한 현재 실차 수락 기준 차량은 `DA_Recipe_Wagon` 하나다. `DA_Recipe_TestSedan`, `DA_Recipe_TestSUV`, `DA_Recipe_DefenseSUV`의 임의 수치는 그대로 보존하고 **실차 OEM 값으로 검증하지 않는다**. 이 3개 테스트 Recipe는 현재 `ImportExisting`가 전체 순회하므로 최소한의 RecipeId·Source 없음·Target·read-only 구조 검증에만 참여한다. Wagon 제외/차종별 하드코딩을 Production Provider에는 추가하지 않았다.

**직접 영향된 코드:**

- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleCCASProvider.cpp v1.1.1` — `AddPrimaryDimensions`에 기존 `ExteriorLengthMm`/`ExteriorWidthMm`/`ExteriorHeightMm`/`WheelbaseMm`와 실제 저장된 Wagon Source `Dimensions.Length`/`Dimensions.BodyWidth`/`Dimensions.Height`/`Dimensions.Wheelbase`의 **명시적인 동일 목적 field mapping**을 구현. 모든 FactKey 비교에 `ESearchCase::CaseSensitive` 적용. `Dimensions.Width`는 `Dimensions.BodyWidth`와 서로 다른 폭 항목이므로 **절대 임의 변환하지 않고 원본 `Vehicle.Claims`에만 보존**. Primary + Canonical + FACT/DERIVED numeric + exact `mm`인 경우만 Main scalar로 투영, legacy와 new FactKey가 같은 Main column에 중복 연결되면 fail-closed. 저장된 Evidence source/fingerprint, Workbook, 기존 VehicleBuilder authority 및 reviewed mutation/Apply 코드는 무변경이다.
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleCCASProviderTests.cpp v1.1.1` — 기존 독립 synthetic `CarFight.CCAS.CF_FQ_059.P0_03B.VehicleReadDiff`에 Wagon 실측과 동일한 `Dimensions.*` FactKey4 + 별도 `Dimensions.Width`를 보강. Main4 값 mapping, BodyWidth ≠ Width 보존, wrong unit `cm` 거부, legacy/new alias duplicate 거부, 기존 stale Evidence fingerprint·diff/Apply 차단 회귀를 검증한다.
- 같은 Tests source에 독립된 **`CarFight.CCAS.CF_FQ_059.P0_03B.WagonPersistedReadback`** 등록. 기존 `DA_Recipe_Wagon`과 `DA_Ref_Wagon` 정확한 Editor fixture 경로를 **테스트 내부에서만** 사용한다. 저장 asset을 `LoadObject`로 읽고 package dirty0, source→Recipe/Target binding, Primary canonical 4치수와 Main 값, Provider의 실제 `ImportExisting`→`BuildCurrentSnapshot`→`ReadbackFingerprint`→same-state `BuildDiff`를 관통 검증하며 저장·수정·Apply하지 않는다. 기존 테스트 Recipe는 실차 수치가 아니라 **identity/read-only load**만 검증한다. 실제 원본/경로/FactKey가 바뀌었다면 이 통합 테스트의 구체적 실패는 Evidence/계약 검수 대상이며, 테스트를 무조건 PASS시키기 위한 fixture 조작은 금지한다.

**현재 검증 상태:**

- **PASS:** 직접 관리형 코드 수정/readback, 관련 Provider/Schema/Source/Asset/UDS 영향 검수. P0-03A exact5 및 이전 P0-03B exact9 9/9 PASS는 **이전 v1.1.0 기준선**으로 그대로 보존. Fresh AssetDump `Recipe4/4 + Evidence1/1 + VehicleData12/12`와 Wagon Source/receipt 결속 확인 역시 보존한다.
- **NOT RUN:** 새 `v1.1.1`에 대한 공식 UE 5.8 Source Editor Build, 수정 `VehicleReadDiff` 및 신규 **`WagonPersistedReadback`** Automation. 이를 통과했다고 주장하지 않는다. 기존 관리형 Build/Process에서 Project `unreal/build_editor`는 `operation_not_available`, legacy `carfight.editor.development` + `carfight_repo`는 `build_preset_invalid`; 둘 다 **started=false**다. UE ReadOnly `ue_status`도 내부 오류이므로 런타임을 직접 검증하지 못했다. GoPyMCP 코드는 작업 범위 밖이며 수정하지 않았다.

**다음 단일 공식 게이트 (새 소스에 한정):** 사용자 또는 승인된 정상 관리형 실행경로로 **v1.1.1 Editor 빌드 성공**을 확인한 뒤 다음 **exact3** 자동화 테스트를 기존 `Tools/RunDataAuthoringTests.ps1 -ExactTestNames`에서 1회 실행한다.

1. `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` — Provider 기본 계약 재검증
2. `CarFight.CCAS.CF_FQ_059.P0_03B.VehicleReadDiff` — 신규 mapping/negative source·diff
3. `CarFight.CCAS.CF_FQ_059.P0_03B.WagonPersistedReadback` — 저장된 Wagon 기반 실제 Provider full read path

수락: EngineExitCode0, SUCCESS3, FAILURE0, MISSING0, UNEXPECTED0, DUPLICATE0, 실제 failure log 0. 기존 공용 CCAS 7건은 이번 변경에서 공용 interface/Core/Compiler가 전혀 수정되지 않아 이전 exact9 PASS를 유지하고 **무조건 중복 실행하지 않는다**. 새 live Wagon test에서 실제 source 문제 또는 shared Test Recipe fail-closed 문제가 발견되면 해당 semantic unit을 먼저 교정·재검증한다. 최종 source/diff/slop 검수 후에만 `P0-03B TECHNICAL PASS`를 판단한다. 다른 UE Asset/Workbook/Product, 사용자 Dirty 및 Git stage/commit/push 없음.

---

### 13.12.14 P0-03B Wagon persisted Provider end-to-end exact3 PASS / phase Technical Acceptance (2026-10-16)

**최신 source:** `CFVehicleCCASProvider.cpp v1.1.1` / `CFVehicleCCASProviderTests.cpp v1.1.1`. USER는 기존 `Tools/RunDataAuthoringTests.ps1 -ExactTestNames`에 다음 exact3를 제출했고, CarFightMCP_Admin로 동일 저장소 `UE/Saved/CarFight/DataAuthoringAutomationResult.json`을 **독립 재조회**해 같은 결과를 확인했다. `CFDataAuthoringAutomation.log`에서 공식 Unreal Engine **5.8**와 exact3 `Automation RunTests` 실행 명령도 확인했다.

- `schema_version=carfight_data_authoring_automation_result_v2`, `status=success`, `execution_mode=exact_list_same_process`, `requested_test_count=3`
- `engine_exit_code=0`, `success_count=3`, `failure_count=0`, `missing_count=0`, `unexpected_count=0`, `duplicate_terminal_count=0`
- `CarFight.CCAS.CF_FQ_059.P0_03A.VehicleSchema` — **PASS**
- `CarFight.CCAS.CF_FQ_059.P0_03B.VehicleReadDiff` — **PASS**
- `CarFight.CCAS.CF_FQ_059.P0_03B.WagonPersistedReadback` — **PASS**

**지난 HOLD 해소:** `VehicleReadDiff`는 최신 Wagon-shaped `Dimensions.Length/BodyWidth/Height/Wheelbase`의 Main scalar 정확한 대응, 독립 `Dimensions.Width` 보존, wrong unit `cm`·legacy/new 동일 목적 alias 중복 거부를 검증한다. `WagonPersistedReadback`는 저장된 `DA_Recipe_Wagon`과 `DA_Ref_Wagon`을 `LoadObject`로 읽고, Primary canonical4 Source Claim↔Main 숫자 일치, saved asset dirty0, **실제 Provider** `ImportExisting` → `BuildCurrentSnapshot` → `ReadbackFingerprint` → identical `BuildDiff` changed paths0을 같은 Editor Automation에서 확인한다. `ImportExisting`는 TestSedan/TestSUV/DefenseSUV도 최소 구조적 read-only item으로 처리하므로 이들 테스트 자산이 Wagon을 막지 않았음이 관찰됐다. 해당 세 차량의 수치가 실제 OEM 제원이라는 판정은 **없다**. Provider 코드 내 차량별 특수 운영 분기, 임의 Source FACT/0, Product mutation writer 추가도 없다.

**근거 범위 구분:** 현재 `v1.1.1`의 UE 5.8 Editor **Automation 실행 PASS는 직접 검증**했으며, 이번 소스의 독립 UBT 빌드 종료 전문은 별도 확보하지 않았다. 이전 `v1.1.0` exact9/9와 fresh AssetDump Recipe4/Evidence1/VehicleData12는 별도의 선행 PASS 기록으로 보존한다. Phase 수락에 요구한 최신 source-focused exact3와 persisted actual Provider smoke가 모두 통과했으므로, **VCCAS-P0-03B TECHNICAL PASS / CLOSED (phase only)** 로 판정한다. 독립 UI/PIE 운전감, P0-02 신규 Core6 대표차 최종 trim·영구 Evidence 수락, P0-03C trusted AI R2 admission, P0-04 Production 차량 생성/Workbook 저장·등록·배포를 승인한 것은 **아니다**. Code, UE Asset, Workbook, 다른 세션 Dirty, Git stage/commit/push 변경 없음.

**다음 Phase — VCCAS-P0-03C READY / 구현 미착수:** 기존 `FCFVehicleAuthoringService::PreviewVehicleRecords/CreateVehicleRecords`의 `SlateUI/Automation` 인가를 속이거나 리뷰를 우회하지 않고, 승인된 UI-less trusted AI R2의 **권한·승인·정확한 caller/scope·review 결속**을 설계 검수하고 기존 Owner/Writer에 additive하게 구현한다. P0-02의 별도 Core6 Reference Evidence 최종 HOLD는 유지하며, 나중의 Production 생성·Worksheet·Publication으로 조용히 앞당기지 않는다.

---

## 13.13 반복 차량 추가·메시 교체의 지속 가능한 저작 계약 (2026-10-15)

**USER 유지보수 요구:** 이번 Core6/Core9만 제작하면 끝나는 일회성 batch 구현을 금지한다. 앞으로 차종, 실차 대표 모델, 차량 개체, StaticMesh가 추가/교체되어도 **차량별 C++ 분기, 임시 제작 스크립트 복제, 새 Registry/Provider/툴 신설 없이 동일한 Vehicle 저작·검증·등록 절차를 반복**할 수 있어야 한다. 이 계약은 프로젝트 전용 추측성 유연성이나 필요 없는 일반화가 아니라, 재사용성·책임 분리·변경 용이성·검증 가능성이라는 일반적인 소프트웨어 품질 요건이다. `Core9`와 현재 6종은 **첫 실행 roster/검증 범위**일 뿐 향후 등록 가능한 차량 수·종류의 상한이 아니다.

### 13.13.1 실제 기존 구현 기반

- **이미 갖춘 기반:** `UCFVehicleData`와 `UCFVehicleRecipeData`는 차량별로 별도 identity를 소유하고 차체는 `ChassisMesh` reference로 연결된다. `FCFVehicleAuthoringService::PreviewVehicleRecords/CreateVehicleRecords`는 매번 고유 Definition/Recipe를 만들며, `ACFVehiclePawn`의 `UCFVehicleVisualComp::ApplyVehicleVisualConfig`는 주어진 `UCFVehicleData`의 메시를 사용한다. `CFVDATuningTests.cpp`에는 runtime VehicleData 재초기화 후 메시 교체 focused regression이 존재한다.
- **동일 메시 재사용:** 기존 `SCFVehicleBuilderTab` 신규 차량의 `SObjectPropertyEntryBox`는 이미 사용 중인 `UStaticMesh`도 선택할 수 있으며 R2 생성 경로는 기존 참조 여부만으로 거부하지 않는다. **이번 작업 도중 새로 나타난 별도 Dirty**의 `FCFVehicleAuthoringService::ListVehicles`에는 과거 '사용 중인 Chassis는 Mesh-only Candidate에서 제외' 조건을 삭제하고, 읽기 전용 `bReferencedByExistingVehicleData`로 현재 참조 여부를 표시하는 변경이 있다. 즉 현재 작업 디렉터리의 후보 표시 코드에도 재사용 가능한 메시가 노출되도록 수정되어 있다. 다만 이 Dirty는 CF-FQ-059가 만든 것이 아니며 빌드·집중 테스트 수락 여부를 확인하지 못했다. 따라서 동작 PASS로 승격하거나 해당 파일을 이 Work에서 수정하지 않는다. 후보 row(StaticMesh 선택)와 기존 VehicleData/Recipe row(실제 차량 identity)는 서로 다른 항목이며 **메시가 이미 사용 중이라는 이유로 새 차량 생성을 차단하지 않는다**.
- **현재 구현 및 잔여:** `FCFVehicleCCASProvider`는 P0-03B에서 읽기 전용 Product import/current snapshot/diff를 구현하고 focused exact9 Automation PASS했다. **Wagon persisted Provider smoke까지 최신 UE5.8 exact3/3 Automation PASS로 기술 수락**됐으며 Product reviewed mutation plan/apply는 계속 fail-closed다. `CreateVehicleRecords`는 two-record unsaved creation이며 P0-03C AI admission, P0-04 multi-package durable save/Workbook/catalog membership, 첫 차량 full runtime acceptance는 아직 미완료다. **기반이 존재한다는 이유로 반복 생산 완성 PASS를 주장하지 않는다.**

### 13.13.2 향후 변경 때 지켜야 할 불변조건

1. **차량 identity ≠ 차종 ≠ 차체 메시 identity.** 여러 차량이 동일한 차종·StaticMesh를 사용할 수 있다. 차량별 `RecipeId`, `VehicleData`, Reference Evidence/Gameplay Profile은 독립 유지한다. 새 차량마다 C++ enum/하드코딩 path/switch 분기를 추가하지 않는다. 실제 차종별 다른 runtime 동작이 명시적으로 요구될 때만 별도 계약으로 검토한다.
2. **새 차량 추가 = 검증된 데이터 입력을 반복.** Category/Reference identity/Asset binding/typed authoring source만 다르며, Schema/Validator/Resolver/Target Writer/Approval/Catalog promotion은 기존 단일 경로를 재사용한다. `AI ChangeSet`과 Workbook/Review/Existing Apply의 authority를 우회하는 one-off JSON/CPP/XLSX writer를 만들지 않는다.
3. **차체 메시 교체 = 기존 차량 데이터의 reviewed resource binding 변경.** RecipeId와 이미 승인된 실차 FACT, 플레이 성능·Defense/Sensor/Profile 값을 자동으로 새로 발급하거나 초기화하지 않는다. 기존 Source → Reference/Recipe → Resolver → Apply의 동일 경로로 대상 `ChassisMesh`를 교체하고, 예상 영향이 있는 Body bounds/Socket/휠 앵커·Wheel Visual/Hardpoint/Layout/Collision/적용 fingerprint를 fresh 검사한다. 메시 변경으로 기존 캐시·측정 receipt가 stale해졌으면 해당 부분만 다시 승인·측정한다.
4. **같은 StaticMesh 에셋을 공유한다면** 한 곳의 원본 Mesh/socket 수정은 참조하는 모든 차량에 영향을 준다. 특정 차량의 외형만 교체하려면 그 차량의 resource reference를 독립 에셋으로 바꾸고 다른 차량은 보존한다. 에셋 공유와 참조 교체는 서로 다른 작업이며 둘을 혼동한 silent bulk mutation을 허용하지 않는다.
5. **미완성 데이터는 미완성으로 남긴다.** 새로운 실차의 실제 제원 미확정/미검증, 메시 미준비·소켓 부족, 특정 Hybrid/IVT/4WD 표현 불지원은 field별 explicit Unknown/Deferred/Blocking 경계로 처리한다. P0-02 reference research와 P0-03 schema preparation은 계속할 수 있으나 Production apply/publish는 관련 mandatory 검증 전 차단한다.
6. **추상화보다 검증된 재사용 우선.** 차량별 if/switch, 중복 provider, 별도 생산 워커, 신규 MCP 도구, 변환용 임시 파일/특정 차량 전용 스크립트를 단지 6종 처리 편의 때문에 신설하지 않는다. 실제 공통 caller 여러 곳에서 반복되는 로직만 기존 서비스 책임 안에서 추출한다.
7. **추가와 변경이 모두 안전하고 반복 가능해야 한다.** 고유 ID 충돌/중복 생성은 fail-closed, 동일 reviewed desired state의 재적용은 no-op, Source/Resource/Recipe/Target 변경은 fresh review를 요구한다. 부분 저장·Workbook 미승격·Catalog 미등록을 성공으로 승격하지 않는다.

### 13.13.3 P0-03B~P0-06 필수 재사용 회귀 수락 시나리오

| 시나리오 | 필수 관찰 결과 |
| --- | --- |
| 기존 Core9 이외 **새 차량 1대**를 새 메시로 추가 | 기존 source 변경 없이 동일 authoring/approval/save/readback/candidate 등록 절차로 생성. 현재 first-wave roster 고정은 등록 차단 조건으로 쓰지 않음 |
| 서로 다른 VehicleData 2개가 **동일 StaticMesh** 사용 | 두 RecipeId와 Reference/Gameplay identity는 별개, 공용 Mesh 참조가 원인으로 생성·목록·Save/Runtime 단계가 거부되지 않음 |
| 기존 차량의 메시 A→B 교체 | Vehicle RecipeId/비물리 reference preserved, 새 Resource binding review, 영향받는 socket/layout/wheel/physics와 fingerprint만 재검증 |
| 공유 메시 A가 다른 차량에도 참조된 상태에서 한 차량만 B로 교체 | 다른 차량의 A binding·Recipe·VehicleData/공용 원본 메시 데이터 변경 0 |
| 차량 재실행/동일 ChangeSet 재적용 | AlreadyCurrent/no-op 또는 current-state 진단, 새 Product/중복 Catalog member 생성 0 |
| 필수 소켓·Resource·제원 충돌 또는 권한/미저장 오류 | 그 차량만 정확한 Block/RecoveryRequired, 기존 published 차량·Workbook·사용자 수정 asset 보호 |
| 새 차량 추가 뒤 정상 Spawn/Driving 검증 | 기존 차량·장비 사용 경로 회귀 없음, 새 차량이 단일 Runtime Catalog membership으로 조회·적용됨 |

**수락 판정:** P0-03A USER Build PASS는 이 반복 생산 시나리오의 구현/테스트 PASS가 아니다. 최소 **새 차량 추가 + 이미 사용 중인 Mesh 재사용 + 기존 차량 Mesh 교체 + 중복·부분 실패 보호**를 P0-04/P0-06에서 집중 검증해야 '차량 추가·교체 쉬움'을 구현 완료로 선언할 수 있다. 이를 위해 새 Feature/새 도구를 자동 승격하지 않으며 현재 CF-FQ-059의 직접 수락 기준으로 적용한다.

---

### 13.12.15 P0-03C Trusted AI R2 설계 작성·교정 (2026-10-16)

**상세 설계:** `Document/Plan/VehicleContentAuthoring/VehicleAIAdmissionDesign.md v0.1.7` (**CARFIGHT INTERNAL DESIGN RE-REVIEW PASS, P0/P1 6/6 / C-0 HOST TRUSTED R2 ENTRY HOLD; 실제 R2 구현·기술 수락 아님**).

- 사용자의 "SUV 한 대 만들어" 지시 한 번으로 정상 신규 1쌍 작업을 진행하며 **동일 작업에 대한 중복 승인 UI를 만들지 않는다**. 차종별 N대 지시는 확정된 N개의 신규 1쌍 operation으로 분해하고, 기존 자산 덮어쓰기·삭제·명부 외 생성은 허용하지 않는다.
- **기존 관리형 Host 호출의 실제 인증·Project/Checkout/Runtime·R2 operation 허용 범위**를 권한 기준으로 사용한다. AI가 만든 `CallerKind`, `bOwnershipWriteApproved`, `ProposalHash`, `UserIntentRef` 문자열은 독립 권한 증거가 아니다. 최종 ProposalHash를 사용자가 차량 조사 전부터 승인했다고 주장하거나 **미확인 UserIntentRef·별도 서명 서버·승인 DB**를 강제하지 않는다.
- **현행 `.gopymcp/project.json`에 P0-03C 전용 R2 operation 등록은 확인되지 않았다.** 다른 UE write surface까지 없다고 단정하지 않고 C-0에서 실제 exposed schema/caller 범위/기존 SlateUI·Automation 위장 경로/operation receipt를 **읽기 전용** 확인한다. 불명확하면 `AI R2 Create` fail-closed, 필요한 GoPyMCP 계약 사항만 별도 소유 세션에 전달한다. 장애 수정은 이 Work에서 진행하지 않는다.
- **C-1 구현**: AI 기본 package root `/Game/CarFight/Data/Authoring/` + `DA_Vehicle_<Id>`·`DA_Recipe_<Id>` 4-field identity를 공통 `FCFVehicleRecordIdentity`에 실제 분리했고, 기존 `FCFVehicleBuilderVM`의 ValidateVehicleCreationId/BuildDefaultVehicleRecordIdentity는 helper wrapper로 보존했다. AI R0 Preview만 canonical path/name을 검사하며 UI Advanced override와 기존 Create caller 검사는 보존한다. C++ `BuildRecordCreateHash`는 **참조 경로·옵션** 해시이지 Mesh/Profile 내용 fingerprint가 아니다. 참조 payload/OEM/물리 검증을 P0-03C PASS로 가장하지 않으며 P0-04 durable authoring review가 소유한다.
- 교정 실행 순서: **C-0 실제 managed Host entry/승인 경계 조사 → C-1 생성 범위·identity 공통화 필요성 결정 → C-2 적법한 entry가 확인될 때만 최소 기존 단일 writer 연결 → C-3 Host 기존 operation receipt 기반 중복/실패 처리 → C-4 focused regression 및 실제 Host T-10/11**. 신규 DB/토큰/임의 UE write tool·권한 UI를 선구현하지 않는다.
- 결과는 **unsaved/unpublished** 신규 Definition+Recipe 1쌍 및 생성 후 읽은 RecipeId. 동일 OperationId/응답 분실은 기존 Host contract를 우선하고, 불확실한 partial create는 자동 retry/delete/Undo/Save 없이 실제 상태를 보고한다. T-01~14 각각 source 가능한 회귀와 Host 연동 수락을 분리한다.
- **교정 후 재검수:** 현행 `CreateVehicleRecords` caller/approval guard, `BuildRecordCreateHash` 필드, Builder VehicleId/identity 구현, `.gopymcp/project.json` 공개 operation 등록 경계와 설계 T-01~14를 대조. **P0/P1 6/6 교정 정합성 PASS(설계 범위)**. C-0의 실제 외부 Host write permission·runtime/receipt 확인은 **NOT VERIFIED/HOLD**, 정상 AI R2 생성의 통합 기술 수락은 미실시.
- 이후 **C-1 코드 구현**은 기존 Builder 기본 ID/경로 생성 로직을 공용 helper에 위임하고 AI Preview에만 canonical identity 검사를 붙였다. `CFVehicleBuilderVM.cpp v1.39.3`, `CFVehicleUXOps.cpp v1.7.0`, 신규 `CFVehicleRecordIdentity.h/.cpp v1.0.0`, `CFVehicleAIAdmissionTests.cpp v1.1.0`. 공식 UE5.8 managed Editor Build `c35c72f769de422d81e8bb86fdab4148` **exit0 PASS**. 집중 Automation은 결과 JSON/로그 덮어쓰기 보호 때문에 **NOT RUN**, 실제 AI R2 Create는 계속 BLOCKED. Source/Systems 버전 구분 유지.
- **C-1 집중 Automation 안전 실행 시도(2026-10-16):** 기존 증거의 무단 삭제/덮어쓰기를 피하기 위해 `Tools/RunDataAuthoringTests.ps1 v1.3.2`에 선택적 `-EvidenceId`를 추가했다. 기본 호출의 경로·결과 schema·overwrite 의미는 보존하고, 선택 시 `UE/Saved/CarFight/DataAuthoringAutomationResult_<EvidenceId>.json` 및 `UE/Saved/Logs/CFDataAuthoringAutomation_<EvidenceId>.log`에 분리 저장하며 같은 이름의 기존 파일이 발견되면 시작 전 차단한다. 테스트 요청 이름은 `CarFight.CCAS.CF_FQ_059.P0_03C.VehicleAIPreview` 및 `CarFight.DataAuthoring.CF_FQ_042.VBCUX_P0_03.Naming`이다. 관리형 `process.run`의 Project+Legacy 혼합 입력은 `project_target_conflict`로 미시작; 정확한 `repository_id=carfight_repo`를 사용한 legacy generic script 경로는 `process_repository_not_found`로 미시작. UE `ue.status/ue.list_toolsets`도 내부 오류. 따라서 **Automation NOT RUN**, 결과 JSON/로그 신규 생성 및 기존 증거 변경 없음. 실행 테스트를 위해서만 별도 Host 인증 시스템/UE write tool 또는 일회성 Project operation을 임의 등록하지 않는다. 재개 조건은 CarFight Project Registry에 공식적으로 지원되는 bounded Automation test 실행 계약 또는 기존 UE 테스트 실행 경로가 준비되는 것. 이 단계는 GoPyMCP 장애 복구 범위 밖.
- **C-1 집중 Automation 최종 증거(2026-10-17):** 기존 PER-01에 의해 검증된 legacy `repository_id=main_game`과 `process.run → process.status`를 재사용하자 보존형 runner 2건이 실제 실행됐다. `VehicleAIPreview`: process `a76cd6ac18e74f5db09afd1a06785355`, EngineExitCode0, 1/1 PASS, 격리 JSON `DataAuthoringAutomationResult_CF059_P0_03C_AI_20261017_2119.json`, SHA256 `ae83f147819cd22ea5790214f9b4943da4b06928f9fe3fb7bd7f603117a013f5`. `VBCUX_P0_03.Naming`: process `bd6fce11d6db4e5ab91d464d0a5c8de4`, EngineExitCode0, 1/1 PASS, 격리 JSON `DataAuthoringAutomationResult_CF059_C1_Naming_20261017_2119.json`, SHA256 `bb7067e8be346cbafbe99dec883c29626efa83ffa4d685db1116baaa4abccb07`. 두 JSON 실제 readback 완료, Failure/Missing/Unexpected/Duplicate 0. 이전 `carfight_repo` legacy 호출은 Project Registry ID와 혼동한 **호출자 입력 오류**였으며 GoPyMCP 실행 경로가 고장났다는 판정은 취소한다. 테스트 2건은 C-1 R0/Builder naming 범위이며 C-0 Trusted AI R2 Create 인가와 End-to-End T-10/11 수락은 별도 HOLD. 이번 갱신은 문서만 변경, Git/UE Asset/Workbook 변경 없음.
- 직전 설계 교정은 **문서 교정 전용**이었다. 선행 AI R0 Preview 공식 UE5.8 Editor Build `160b950da4374d1db0b35d79c327412a` exit0 PASS, 신규 VehicleAIPreview Automation NOT RUN, 실제 trusted R2 생성 NOT OPENED. P0-03A/B Technical PASS, P0-02 Evidence HOLD, P0-04 Save/Workbook/Catalog/Publish HOLD는 그대로 보존한다.

---

### 13.12.16 P0-03C C-0 실제 Host 공개 계약 읽기 전용 감사 (2026-10-17)

**판정: C-0 READ-ONLY INVESTIGATION COMPLETE / TRUSTED AI R2 ADMISSION BLOCKED — MISSING PROVEN HOST CONTRACT.** CarFight 소스 구현 실패나 PER-01 process 실행 장애로 분류하지 않는다.

- **Project / UDS 사실:** Company `CarFightMCP_Admin`에서 `carfight/main_game`, `SwitchSourceEngine`, Checkout `checkout_eefe2944e0954bed9270908eb95d8fd0` 확인. 해당 Work의 immutable record 26개를 fresh read하고 부모 관계를 재계산한 결과 maximal head **exact1=`ver_a059b00120261016d3c5600000000025`**, `heads.json`과 일치. Current projection의 오래된 C-1 이전 설명은 canonical 구현/evidence가 아니다.
- **실제 등록 operation:** `.gopymcp/project.json`의 `carfight/unreal` operation은 `editor.build`, `editor.start`, `editor.stop`, `ue.read`, `pie.start`, `pie.stop`뿐. `CreateNewVehicleRecords` 또는 동일 의미의 P0-03C R2 등록 없음. 이것만으로 Host의 모든 잠재 내부 기능이 없다고 단정하지 않는다.
- **실제 공개 UE 표면:** Company `CarFightMCP_UE_ReadOnly` 및 `CarFightMCP_UE_WriteProbe`가 동일한 **53개** toolset을 반환했고, CarFight 전용 Vehicle R2 create toolset/dispatcher는 노출되지 않았다. `DataAssetTools.create`, `AssetTools.save_assets`, 일반 `ue_call_write`는 존재하지만 기존 `FCFVehicleAuthoringService::CreateVehicleRecords` 단일 writer와 연계된 신뢰된 R2 admission을 증명하지 않는다. 일반 에셋 생성이나 Slate/UI 조작으로 우회하지 않는다.
- **Project/Checkout/Runtime 권한:** `ue_status`는 unscoped `127.0.0.1:8100` Bridge connected를 반환했으나 exact Project-capability-Runtime instance로 귀속된 승인 증거는 반환하지 않았다. 공개 `ue_call_write`에는 `project_id/capability_id/runtime_instance_id` 선택자와 `apply_approved` Boolean이 있으나, 이 필드의 사용자 제공만으로 authenticated caller의 `OwnershipWrite / CreateNewVehicleRecords / root / pair-count=1` 인가를 증명할 수 없다. `diag.selftest_core`의 legacy UE 환경 확인도 managed R2 authorization의 증거가 아니다.
- **외부 spoof / replay:** 현재 공개 UE schema에서 `FCFVehicleAuthoringService::CreateVehicleRecords`를 직접 invoke하는 경로는 확인되지 않았다. 따라서 **외부 caller의 SlateUI/Automation 위조가 성공한다고 확인된 것도 아니고, 모든 간접 우회가 차단됐다고 입증된 것도 아니다.** C++의 AI 직접 Create 거부와 자기선언 Boolean/Hash 거부는 기존 `VehicleAIPreview` Automation 범위에서만 검증됐다. 범용 `ue_call_write`에는 차량용 `ClientOperationId`·동일 요청 terminal receipt/replay 계약이 공개되지 않았으며, `process.run`의 실행 ID·중복 처리와 동일시하지 않는다. 응답 분실·부분 실패 복구 계약 역시 **NOT VERIFIED**.
- **소유권과 정확한 해소 조건:** GoPyMCP 관리형 Host/Project Runtime/UE Bridge owner가 기존 지원 범위에서 신뢰된 Project-scoped bounded Vehicle R2 operation을 제공할 수 있는지 확인해야 한다. 실행 주체 인증, exact `carfight/main_game` Checkout·Ready Runtime lifetime, `OwnershipWrite` operation·canonical root·단일 pair, 외부 caller spoof 차단, stable operation receipt/lost-response/partial-failure 계약이 실제 노출되고 읽기 전용으로 검증돼야 CarFight C-2 단일 writer adapter 설계·구현 Gate를 연다. 새 토큰 서버·승인 DB·반복 UI·범용 UE write tool은 선구현하지 않는다. GoPyMCP 수정이나 장애 수리는 독립 owner 세션에 귀속한다.
- **범위/검증:** C-0는 등록·schema·현재 코드·UDS에 대한 **읽기 전용** 감사. C-1 UE5.8 Build 및 focused 2/2 PASS를 변경하지 않았고, C-2/3 source 작업, T-10/11 실제 Host R2 create, 에셋 생성·Save·Workbook·Catalog·Publish, 신규 빌드/Automation은 **NOT RUN / NOT APPLICABLE(이번 문서 감사)**. P0-03A/B 수락과 P0-02 별도 HOLD 보존. UDS lifecycle은 Ready/P0-03C 그대로이며 record/projection mutation 없음.

---

### 13.12.17 P0-03C C-2/3 기존 호출 경계 사전검수·독립 범위 분리 (2026-10-17)

**판정: CARFIGHT C-2/3 READ-ONLY BOUNDARY REVIEW COMPLETE / ACTUAL R2 IMPLEMENTATION BLOCKED.** 상세 검사·테스트 분리는 `VehicleAIAdmissionDesign.md v0.1.5 §7.2`가 소유한다.

- **기존 writer 유지:** `CFVehicleUXOps.cpp::CreateVehicleRecords`는 `PreviewVehicleRecords`의 SlateUI/Automation caller 검사 → fresh proposal/hash/approval → `FScopedTransaction` 아래 신규 Definition+Recipe만 생성한다. `CFVehicleP11VM.cpp`와 `CFVehicleBuilderVM.cpp`의 기존 UI 생성/adoption 경로를 변경하지 않는다. `PreviewAIVehicleRecords`는 C-1 canonical R0만 수행하며 AI 직접 Create는 계속 차단한다.
- **독립 진행 가능:** `CFVehicleRecordIdentity`, AI R0 canonical/root·package collision·Chassis type·hash mutation0 경계의 부족한 음성 케이스만 기존 `CFVehicleAIAdmissionTests.cpp` coverage와 대조; `CFVehicleAuthoringVMTests.cpp`의 기존 생성 후 Dirty/Save0/Target binding 회귀 selector·에셋 격리/cleanup 확인; 기존 T-01/02/06/07/08/13 테스트 매핑 정리. 과거 C-1 focused 2/2 PASS 반복 실행이나 신규 공용 프레임워크는 기본 불필요.
- **Host 계약 이후에만 구현:** 기존 `CreateVehicleRecords` 단일 primitive 추출 여부, non-spoofable AI domain adapter, Project/Checkout/Runtime/OwnershipWrite/root/단일 쌍 권한, OperationId dedupe/response-loss terminal receipt와 실제 T-10/11은 여전히 외부 contract 차단. `CallerKind=Automation/SlateUI` 위장이나 `bOwnershipWriteApproved`/`ProposalHash`만 이용한 경로 금지.
- **새 검수 주의:** `FCFVehicleRecordCreateRequest`에는 ClientOperationId가 없으며 공통 `FCFAuthoringOpResult`에 dedupe 필드가 있다는 사실만으로 해당 Create replay를 증명할 수 없다. `FScopedTransaction::Cancel`이 일부 만들어진 UObject/package까지 원자적 취소한다고 가정하지 않는다. 실제 partial failure readback·unknown 분류는 Host 계약이 정해진 후 집중 검증한다.
- **실행/보호:** 기존 코드·테스트·Plan/Systems/UDS 읽기 전용 검수, 이 대표 Plan/상세 설계 2문서만 체크포인트 문구 반영. 새 C++/UE Asset/Save/Workbook/Catalog/Publish/GoPyMCP/Git 작업0. 새 UE5.8 Build 및 Automation NOT RUN; 기존 C-1 2/2와 P0-03A/B PASS 유지. UDS Work는 기존 `Ready`/`VCCAS-P0-03C`이므로 lifecycle successor/projection 변경 없음.

---

### 13.12.18 P0-03C C-2/3 R0 음성 회귀 최소 보강 (2026-10-18)

**판정: TEST-ONLY SOURCE CHANGE COMPLETE / OFFICIAL UE5.8 BUILD LINK BLOCKED / NEW AUTOMATION NOT RUN / HOST R2 CREATE BLOCKED.** 상세 소스·실행 증거는 `VehicleAIAdmissionDesign.md v0.1.6 §7.3`에 기록한다.

- `CFVehicleAIAdmissionTests.cpp v1.2.0`의 기존 exact `CarFight.CCAS.CF_FQ_059.P0_03C.VehicleAIPreview` 테스트 하나에 빠진 3건만 추가: canonical 미저장 빈 Definition package 충돌 `StateChanged`, 비-StaticMesh transient Chassis 참조 `WrongAssetType`, explicit Hardpoint 정책 단독 변경에 따른 ProposalHash 차이. 고유 fixture를 메모리에서만 만들고 Save0/garbage 처리. 기존 AI 직접 Create 차단 및 UI 경로 보존.
- Company `CarFightMCP_Admin` 공식 UE5.8 `carfight.editor.development`, legacy repository_id=`main_game` build `41bacc7d43b3476d8f083f6ef25db4b8`: 변경 테스트 소스 compile stage 통과 뒤 `UnrealEditor-CarFight_ReEditor.dll` Link에서 현재 실행 중인 UnrealEditor.exe의 file lock 때문에 UBA/NoUba 모두 실패. **전체 BUILD FAILED exit6**, 코드 컴파일 오류 발견0이지만 빌드 PASS 아님. Current Editor의 저장 안 된 자산·소유권이 미확인이라 강제 종료/재시작 없음.
- v1.2.0 모듈 DLL이 갱신되지 않아 변경된 테스트의 Automation은 **NOT RUN**. 기존 v1.1.0 C-1 focused 2/2 PASS는 Historical로 보존. 현재 에디터 DLL 점유가 안전하게 해제된 뒤 공식 build 재실행 및 새로운 `-EvidenceId`를 사용한 VehicleAIPreview exact1 단독 Automation과 JSON readback을 수행할 것. 기존 사용자 에셋·증거 JSON·UE 설정·GoPyMCP 소스는 변경하지 않음.
- UDS state=`Ready`, phase=`VCCAS-P0-03C` 유지, lifecycle record/projection 변경하지 않음. Git stage/commit/push 없음. Trusted AI R2 권한·writer adapter·OperationId receipt·실제 T-10/11은 외부 Host 계약 BLOCKED 유지.

---

### 13.12.19 P0-03C C-2/3 기존 UI·Builder 차량 생성 회귀 테스트 독립 검수 (2026-10-19)

**CarFight 범위 결과: T-13 REGRESSION SELECTORS SOURCE-VERIFIED / ACTUAL AUTOMATION NOT RUN.** 상세 코드 근거와 격리 조건은 `VehicleAIAdmissionDesign.md v0.1.7 §7.4`가 소유한다.

- `CFVehicleAuthoringVMTests.cpp`의 실제 등록 selector `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate`: 기존 생성 서비스 Preview→R2 승인 Hash→Definition/Recipe 단일 writer, Target/Chassis binding, 기본 Sensor, Profile 자동 추론 없음, Dirty/Save0/자동 Retry0 및 Guided Builder 기존 경로 회귀.
- 실제 등록 selector `CarFight.DataAuthoring.CF_FQ_042.VBCUX_P0_02.RecordCreation`: Blank Start/재사용 Chassis로 기존 `ExecutePreparedNewVehicleRecordCreate`, Browser adoption 및 원본 VehicleData Chassis 보존 회귀.
- 두 테스트는 기존 생산 파일이 아닌 고유 `/Game/CarFight/Tests/` 테스트 UObject를 만들고 Asset Registry에서 등록 제거/garbage 처리하지만, 공통 `CleanupRegisteredAsset`이 생성 package의 Dirty flag/메모리 package 생명주기까지 원상복구하는 것은 증명하지 못한다. **공유 사용자 Editor에서 실행하지 않고** 별도의 unattended Automation Editor process와 unique EvidenceId로만 future focused 실행한다. 기존 source/code/asset/test runner는 이번에 변경하지 않았으며 T-13 전체 PASS 또는 새 빌드 PASS를 주장하지 않는다.
- 현재 C-2/3 production writer·기존 UI 호출 경로는 그대로이므로 T-13의 두 기존 회귀를 당장 중복 실행하지 않는다. 추후 Trusted R2 내부 연결의 실질적 writer 영향이 생기면 이 exact2와 VehicleAIPreview v1.2.0을 분리해 집중 검증한다. Host 외부 의존 계약은 별도 소유, 현 세션에서는 프로젝트 밖 시스템 복구 작업을 수행하지 않음.
- UDS `Ready`/`VCCAS-P0-03C` lifecycle 유지. Product C++/UE Asset/Workbook/Catalog/Save/Publish 0, Git stage/commit/push 0. 기존 미검증 R0 v1.2.0 Automation 및 T-10/11 실제 AI R2 실행은 계속 NOT RUN.

---

### 13.12.20 P0-02 Core6 제조사 ReferenceSpec·Evidence 정합성 재검수 (2026-10-19)

**판정: SIX-REFERENCE OEM 24/24 DIMENSION FACT SOURCE RECHECK PASS / CITATION REGISTRY RECONCILED / VENUE REAR TRACK CONFLICT FOUND / FULL EVIDENCE ACCEPTANCE HOLD.** 현재 CarFight phase는 `Ready / VCCAS-P0-03C` 그대로이며 이 P0-02 연구 검수는 독립 후속 단위다.

- **실측 근거:** Kia Morning KR 2027 spec+price, India NEXA Jimny5 Alpha AT June2024 PDF (technical p10), Toyota Oman GR86 2026 PDF + independent Toyota KR GR86 wheelbase source, Toyota Australia HiLux SR5 current range, Hyundai KR Venue official 17-inch PDF, Hyundai KR STARIA Hybrid Cargo official page. 각 차량의 전장/전폭/전고/축거 6×4=24 FACT 원자료가 기존 연구값과 일치한다. 여전히 원래 국가·세부 트림을 분리하며 현재 임시 메시와의 실루엣 일치나 Product 품질 인증은 포함하지 않는다.
- **문서 간 출처 정합성 교정:** Jimny 과거 URL `/assets/cars/jimny/documents/NEXA-Jimny-Brochure.pdf`는 직접 조회 실패했고 `/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf` OEM PDF는 확인됐다. SourceId `src-nexa-jimny5-brochure` 유지. STARIA의 동의어 `src-hyundai-staria-cargo-hybrid`를 `src-hyundai-stariahybridcargo-kr`로 통일했다. 두 교정은 신규 `FCFRefSourceCitation` 저장/기존 GUID·fingerprint 변경이 아닌 문서상 연구 키 정리다.
- **새로 발견된 특정 FACT 충돌:** 현대 Venue 17-inch **rear track**은 같은 제조사 PDF의 제원표 `1546mm` vs 같은 페이지의 치수 그림 `1545mm`. 이는 ExteriorLength/Width/Height/Wheelbase 24건 **외부** 값으로 해당 24/24 PASS에는 영향이 없다. 그러나 RearTrack을 정확한 Scalar FACT로 사용하거나 P0-02 전체 ACCEPTED로 승격해서는 안 된다. `ECFRefUnknownReason::ConflictingUnresolved` 및 필요한 source-internal `FCFRefConflict` 제안만 문서화하고 실제 Payload/Asset은 만들지 않는다.
- **정확한 분리:** GR86의 `2575mm` Wheelbase는 Oman에 실리지 않았으므로 Toyota Korea 원문을 다른 market citation으로 사용하고, Oman 233HP/한국 231PS 및 공차중량을 섞지 않는다. HiLux 2155kg는 `max curb`, 3080~3120kg은 GVM `range`; STARIA ICE 180PS와 Hybrid system245PS를 구분한다. STARIA 페이지의 관측 시점은 정확한 제조사 모델연식 확정 근거가 아니다. Venue 17-inch의 exact sale trim도 아직 미확정이다.
- **현재 Evidence 코드 대조:** `CFVehicleRefEvidence.h/.cpp v1.3.0`의 `FCFRefVehicleIdentity`, `FCFRefSourceCitation`, `FCFRefClaim`, `FCFRefUnknownFact`, `FCFRefConflict`, `ValidateInitialResearchPayload` 필드를 대조했다. 6개의 독립 Primary payload를 나중에 작성할 수 있으나 이번에는 6개의 typed payload를 만들어 validator에서 실행한 것이 **아니다**. 각 claim의 `CitationIds`가 해당 SourceId/Market/Locator에 결속되었는지 **별도 원문/readback 감사**하고 (`ValidateInitialResearchPayload`는 존재하는 SourceId 검사까지만 자동 수행, 개별 Claim↔cited Source의 VehicleId/시장/트림 결속까지 자동 검사하지 않음), exact1 Primary·unique IDs·Canonical FACT 하나 이상·unresolved Block 없음 조건 충족 전 영구 `UCFVehicleRefEvidence` 생성/저장·P0-02 final acceptance는 HOLD.
- **변경/검증 범위:** 이번 문서 정합성 교정은 `VehicleReferenceEvidenceDraft.md v0.1.2`, `VehicleReferenceSourceReview.md v0.1.1`, `VehicleRepresentativeSpecRoster.md v0.1.2`, 대표 Plan v0.1.40에 한정한다. CarFight UE C++/BP, 에셋, Source Evidence DataAsset, Recipe/Profile, Workbook/Catalog, GoPyMCP, 빌드/Automation, Git stage/commit/push 0. 이전 Dirty 보호. Source Review PASS와 typed validation NOT RUN 구분.

---

### 13.12.21 P0-02 Core6 독립 typed Reference Research Payload 6개 입력안 및 정적 검수 (2026-10-19)

**판정: CORE6 DRAFT INPUT SPEC COMPLETE / STRUCTURAL STATIC AUDIT PASS / ACTUAL UE VALIDATOR NOT RUN / P0-02 FINAL EVIDENCE ACCEPTANCE HOLD.** 세부 원본은 `Document/Plan/VehicleContentAuthoring/VehicleCore6EvidencePayloads.md v0.1.0`; Source·Claim 기준선은 `VehicleReferenceEvidenceDraft.md v0.1.3` 및 `VehicleReferenceSourceReview.md v0.1.1`.

- **입력안 6개:** Morning KR 2027 14-inch Trendy, Jimny India 5-door Alpha AT, Toyota GR86 2026 Oman+Toyota KR Wheelbase 보조, HiLux SR5 4x4 AU current, Hyundai Venue KR 17-inch IVT, Hyundai STARIA Hybrid Cargo5 KR current. 각각 독립 `FCFVehicleRefEvidencePayload`에 대응. 총 **Primary6, Source8, FACT Claim84(전장·전폭·전고·축거 exact24 포함), UnknownFact24, Venue Conflict1**.
- **실제 계약 대조:** `CFVehicleRefEvidence.h/.cpp v1.3.0`의 `FCFRefVehicleIdentity`, `FCFRefSourceCitation`, `FCFRefClaim`, `FCFRefUnknownFact`, `FCFRefConflict`, `FCFVehicleRefEvidencePayload` 및 `ValidateInitialResearchPayload()` 필드/검사 경계를 확인. Claim별 typed `IntegerValue/NumberValue/RangeMinValue+RangeMaxValue/TextValue`, native UnitId, FACT/Candidate와 정확한 SourceId, 세부 Unknown reason/blocking, Venue 1546(table)/1545(drawing)의 동일 출처 후보2+Warning conflict+Unknown1 표현.
- **문서 읽기 정적 검사 PASS:** 6개 절/ClaimId unique84/SourceId unique8/UnknownFactId unique24, 6종×치수4, 전체 CitationIds와 Unknown SearchedSourceIds가 각 자기 Payload Source에 존재, Source ReferenceVehicleIds가 동일 Primary, 타입 필드 형식 전부 확인. GR86 한국 2575mm는 오만 exact trim fact가 아니라 cross-market 보조 Candidate로 유지. HiLux max curb 2155/GVM range3080~3120은 exact curb/GVWR 미확정, STARIA system245/ICE180/motor54kW 구분.
- **실제 UE Validator PASS 아님:** 현재 84개 Claim은 모두 `ResolutionState=Candidate`; 기존 Validator는 Canonical FACT/DERIVED를 최소1개 요구하므로 이 상태로는 reject 예상이다. 임의로 Canonical을 선택해 통과시키지 않는다. `ValidateInitialResearchPayload`는 Claim Citation의 존재는 확인해도 cited Source의 실제 시장·트림·claim과의 일치까지 검증하지 않으므로 최종 승인 직전 개별 추가 감사 필요. Venue 후륜 윤거는 원본 충돌 해결 전 Canonical 금지.
- **후속 정확한 순서:** 정확한 vehicle year/grade/options·market identity 수락 → 각 차량 claim/source/unknown/conflict 리뷰 → 승인된 Canonical FACT 선정 → 독립 실제 UE typed validator 실행과 결과 readback → 기존 Evidence owner의 합법적인 companion authoring/Save 단계 별도 실행. 지금은 문서 연구 입력안만 완료했다. 유저 교체용 임시 Body Mesh·소켓은 이 gate의 선행조건 아님.
- **보호:** CarFight UE C++/BP, UObject/DataAsset, VehicleData/Recipe/Profile, Asset/Workbook/Catalog, GoPyMCP, 빌드/Automation, UDS Work lifecycle, Git stage/commit/push 변경0; 기존 Dirty 보존. Current phase=`VCCAS-P0-03C` 유지.

---

### 13.12.22 P0-02 Core6 외형 치수 FACT Canonical 수락 검수 (2026-10-19)

**판정: SOURCE-SCOPED DIMENSION CANONICAL 23/24 / OMAN GR86 WHEELBASE CROSS-MARKET CANDIDATE 1/24 / REMAINING FACT CANDIDATE 60 / STATIC REVIEW PASS / ACTUAL UE VALIDATOR NOT RUN / FINAL P0-02 EVIDENCE HOLD.** `VehicleCore6EvidencePayloads.md v0.1.1`가 84개 Claim별 정확한 `ResolutionState` 및 차량별 조건을 소유하며, Source Review v0.1.2와 Evidence Draft v0.1.4에 검수 근거를 교차 기록한다.

- **공식 출처 재조회 후 Canonical 선정:** Morning KR 2027 Trendy 14-inch 4/4, Jimny 5-door Alpha AT India 4/4, GR86 Oman 2026 6MT 자체 공식 치수 L/W/H 3/4, HiLux AU SR5 4x4 AT V-Active 4/4 (`Dimensions approximate` 원문 qualifier 유지), Venue KR 17-inch IVT/no roof-rack 4/4, STARIA Hybrid Cargo5 KR 17-inch 4/4. 총 제조사 조건부 치수 FACT **23 Canonical**. GR86 Wheelbase 2575mm는 Toyota **Korea** 별도 원문으로만 확인되었기에 **Oman exact 6MT 주장으로 승격하지 않고 Candidate 유지**.
- **잔여 Candidate 61:** GR86 KR 보조 Wheelbase1 및 6종 엔진/질량/타이어/윤거 등 나머지 FACT60. 현대 Venue 동일 브로슈어 후륜 윤거 **1546 spec table vs 1545 dimension illustration** 후보2+Unknown1+Warning Conflict1 유지. HiLux 최대 curb2155kg/GVM range3080~3120은 exact vehicle scalar가 아님. STARIA system245PS/ICE180PS/motor54kW 분리. 연식·트림 미확정은 연구 Canonical 채택과 별개로 유지.
- **의미 경계:** 이 연구용 Canonical은 `FCFRefClaimResolution`의 의미 있는 FACT 선택이지만 **사용자 최종 모델/트림 승인·Saved UCFVehicleRefEvidence 생성·P0-04 Product Authoring 개방·Runtime 승인과 동일하지 않다**. 현재 `ValidateInitialResearchPayload()`의 'Canonical FACT≥1' 조건을 문서상 6/6 충족하도록 수정했으나, 공식 UE typed payload validation / Source market matching / Acceptance는 **NOT RUN/HOLD**. Source/CitationId는 입력안의 기존 ID를 유지.
- **변경과 보호:** `VehicleCore6EvidencePayloads.md v0.1.1`, `VehicleReferenceEvidenceDraft.md v0.1.4`, `VehicleReferenceSourceReview.md v0.1.2`, 대표 Plan v0.1.42만 변경. UE C++/BP/Asset/Workbook/Catalog/GoPyMCP/Build/Automation/UDS lifecycle/Git stage·commit·push 변경0, 기존 Dirty 보호. Current Work `Ready/VCCAS-P0-03C` 보존.
- **독립 다음 검증:** 승인된 연구 Claim을 기반으로 실제 C++ `ValidateInitialResearchPayload()`를 **별도 임시 메모리 객체·정확한 source market 검수** 범위에서 집중 실행하고 JSON readback을 판정. 실행 실패 시 CarFight 내부 입력안/테스트만 교정; P0-03C Trusted R2와 외부 환경 복구는 연결하지 않는다.

---

### 13.12.23 P0-02 Core6 typed `ValidateInitialResearchPayload()` 집중검증 완료 (2026-10-19)

**판정: CORE6 SIX REAL TYPED PAYLOADS VALIDATOR PASS / NEGATIVE GUARDS 11/11 PASS / UE5.8 BUILD PASS / FOCUSED 1/1 PASS; USER FINAL IDENTITY & PERSISTENT EVIDENCE AUTHORING HOLD.** 상세 6개 Payload와 Source/Unknown/Conflict는 `VehicleCore6EvidencePayloads.md v0.1.2` 및 `VehicleReferenceEvidenceDraft.md v0.1.5` 참조.

- **CarFight 소스 변경은 시험 1개:** `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleCore6EvidenceTests.cpp v1.0.0` 추가. Core6 `ReferenceVehicles Primary6`, Source8, Claim84 (`Canonical23/Candidate61`), Unknown24, Venue `Warning`/`UnresolvedBlock` Conflict1을 실제 `FCFVehicleRefEvidencePayload` 6개의 독립 구조체에 각자 재현했다. User Evidence/VehicleData/Recipe/Blueprint/Workbook은 수정·생성하지 않고 transient UObject `UCFVehicleRefEvidence::ValidateInitialResearchPayload`만 호출했다.
- **실제 정상·부정 검증:** 6/6 정상 Payload 통과; 11개의 비정상 입력(모든 Canonical 제거, 없는 Citation Source, 다른 Claim ReferenceId, 다른 Source ReferenceId, SourceId 중복, ClaimId 중복, Unknown searched Source 누락, Primary0, GAME_BIAS 포함, Venue unresolved Block, Venue 충돌 후보 ClaimId 누락) 모두 기존 Validator에서 거부. 이 결과는 전체 exact1 UE Automation test의 PASS로 수락, 케이스별 별도 JSON terminal receipt는 아님.
- **공식 빌드:** Company CarFightMCP_Admin `build.run` legacy preset `carfight.editor.development`/repository `main_game`, job `8cf022cd62ac46cda8510296acfe34f3`, 기존 신규 테스트 포함 compile/Link DONE, **exit0**.
- **검증/보존:** legacy `process.run` repository_id=`main_game`, script `Tools/RunDataAuthoringTests.ps1 -ExactTestNames CarFight.CCAS.CF_FQ_059.P0_02.Core6PayloadValidation -SuppressModelContextProtocolLog -EvidenceId CF059_P002_Core6_20261019_Validation01`, process `2e70e468bcf64aabb66e76e22c4077c2`, UE EngineExitCode0, exact1 SUCCESS1/FAILURE0/MISSING0/UNEXPECTED0/DUPLICATE0. 격리 JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_P002_Core6_20261019_Validation01.json` readback `schema_v2`, SHA-256 `389adb6954fa539b4ef2ede8111cec0e3f232b7edd49bc3d3b09cf77db84cf7f`. 기존 Test evidence 덮어쓰기0.
- **기술 PASS의 명확한 한계:** 해당 Validator의 SourceId 존재/ReferenceVehicleId 집합 조건은 source의 실제 Market/Trim과 Claim의 자연언어 Fact 적합성을 검증하지 않는다. `UnknownFact.BlockingUse=ProposalBlock`이 있어도 Research Payload 형식으로는 수락 가능하며, 따라서 물리 시뮬레이션/영구 최종 데이터 값 생성 허가가 아니다. GR86 한국 축거는 Oman Primary Candidate, Venue 후륜 윤거1545/1546 unresolved Candidate+Conflict, HiLux approx dimension 및 weight range/maximum, STARIA hybrid separate source FACT 유지. 여섯 최종 세부 trim/옵션 USER 승인 및 companion persisted Evidence 생성·P0-04 authoring은 **HOLD**.
- **변경 소유권:** C++ test1 + P0-02 관련 문서만, UE 에셋/기존 Writer/P0-03C Trusted AI R2/GoPyMCP 수정0. Work `Ready/VCCAS-P0-03C` lifecycle 그대로, 기존 다른 Dirty 보호, Git stage·commit·push0. 다음은 Source Market/Trim acceptance와 실제 authoring gate를 좁혀 진행; 중복으로 Validator 재구현하지 않음.

---

### 13.12.24 P0-02 Core6 Reference Identity·Citation 84건 최종 수락 검수 (2026-10-19)

**판정: CLAIM-TO-SOURCE/IDENTITY AUDIT COMPLETE (84/84) / RESEARCH CANONICAL23 PRESERVED / CANDIDATE61 PRESERVED / CITE/OWNER FIX2 / KNOWN UNRESOLVED TRACK CANDIDATE2 / RESEARCH AUTHORING PREPARATION 5 CONDITIONAL READY + 1 BLOCKED / PERSISTENT SAVE0.** 상세 결과·84개 행별 판정·8개 Source 검수·24개 Unknown 보존은 `VehicleCore6CitationAudit.md v0.1.0`이 소유한다.

- **초점:** 실제 정적/외부 OEM Source와 6개 Working ReferenceIdentity의 Manufacturer·Model·ModelYearQualifier·Trim·Powertrain·Transmission·Wheel/Tire·MarketRegion, 모든 FACT84의 `ClaimId→CitationId→SourceId→OriginGroupId/SourceScopeNote`를 교차검수. 공식 UE5.8 Core6 validator 기존 Build `8cf022cd62ac46cda8510296acfe34f3` exit0, exact1 focused Automation process `2e70e468bcf64aabb66e76e22c4077c2` 6 normal/11 negative PASS 증거는 **그대로 유지하고 새 실행·구현 없음**.
- **대응 상태:** R=research Canonical23(기존 유지), Q=정확한 범위/조건을 붙인 Candidate57, F=저장 전에 수정할 Citation/SourceOwner 결속2, U=같은 현대 Venue OEM 자료 내부의 rear-track 1546/1545 Candidate2. **FACT 총 84건 모두 분류**, Unknown24/Conflict1은 삭제·해결한 것으로 처리하지 않음. `R`은 원문 기반 연구 수락이지 USER의 최종 트림·Production 승인과 별개.
- **실제 발견 1:** Kia Morning `clm-1-tirecode`의 정확한 2027 Trendy 기본 `175/65R14`는 Kia 가격/트림 공식 페이지가 직접 근거인데 현재 `CitationIds`는 스펙 Source만 참조한다. 기존 `src-kia-morning27-price`가 같은 Payload 안에 존재하면서 Claim은 하나도 인용하지 않는다. 이 Candidate의 CitationId를 출처 `src-kia-morning27-price`에 결속하는 변경을 **후속 실제 authoring 입력/테스트 수정 단위에서만** 수행한다.
- **실제 발견 2:** Toyota GR86 `clm-3-wheelbasemm`=2575은 KR Toyota 모델 자료로 사실이지만 Oman 2026 6MT Primary의 exact 출처가 아니다. 현 KR citation `ReferenceVehicleIds`가 OM Primary를 직접 가리켜 source ownership 의미가 부정확하다. 제품/임시 Research Evidence에 쓰기 전 한국 Secondary 연구 identity의 정합한 소유 분리 또는 오만 자체 wheelbase 증거 확보 등의 결정이 필요하다. 사용자 승인 없이 선택 모델을 한국 시장 차량으로 바꾸거나 Oman exact Canonical로 승격하지 않는다.
- **남겨둔 조건:** Venue `TrackRearMm` 1546/1545 같은 출처 Warning conflict + Candidate2 + Unknown `ConflictingUnresolved`; 17-inch IVT의 정확한 판매 등급/ModelYear UNKNOWN. HiLux SR5 제원은 published **approximate**, curb2155kg **maximum**, GVM3080–3120kg **range**이며 exact Curb/Gross 값은 ProposalBlock Unknown. Jimny 인도 5-door Alpha AT 브로슈어는 2024-06 발표시점이지 MY가 아니고 실제 4인승이다. STARIA Cargo5 KR 17-inch 245PS system과 ICE180PS/motor54kW는 별도 FACT로 유지; current page의 2026-10 조회 시점과 OEM ModelYear는 별개.
- **차량별 Evidence 준비 판정:** Morning(J Kia 타이어 Citation 수정), Jimny(MY Unknown 유지), HiLux(Approx/Max/Range 유지), Venue(17-inch IVT research scope+Conflict/Unknown 보존), STARIA(current/MY Unknown+hybrid power 분리) = **5종 CONDITIONAL READY for preparing a research companion**. GR86(OM Primary/KR Citation ownership 혼재)=**1종 BLOCKED for faithful persisted research**. 이는 `UCFVehicleRefEvidence` 여섯 개가 생성되거나 저장됐다는 뜻이 아니다. 실제 저장은 six 모두 Owner `EvidenceId/TargetRecipeId/Path` binding·저장 및 readback 계약·USER 최종 identity/trim 수락 검수 **미완료**로 WAIT.
- **영향과 보호:** 이번 작업은 신규 문서 `VehicleCore6CitationAudit.md v0.1.0` + Plan 교정만. 기존 `CFVehicleCore6EvidenceTests.cpp v1.0.0` 및 Validator/승인 상태·UE C++/BP·Asset/Workbook/Catalog·GoPyMCP·Git stage/commit/push 0. Source/Citation의 미래 변경은 테스트에 반영한 다음 공식 UE5.8 재빌드/새 EvidenceId exact1 Automation으로 새 입력에 대한 검증을 받는다. UDS `Ready/VCCAS-P0-03C` lifecycle 불변. 사용자 임시 메시·물리 소켓은 이번 증거 승인 조건으로 재등장시키지 않는다.

---

### 13.12.25 P0-02 Morning Citation + GR86 Secondary 보정·실제 재검증·Evidence 바인딩 계약 (2026-10-19)

**판정: CITE OWNERSHIP CORRECTED / OFFICIAL UE5.8 BUILD PASS / CHANGED FOCUSED VALIDATOR PASS / EXISTING BUILDER COMPANION OWNER CONTRACT REVIEW COMPLETE / PERSISTENT EVIDENCE SAVE0 / USER FINAL P0-02 ACCEPTANCE HOLD.** `VehicleCore6EvidencePayloads.md v0.1.4`, `VehicleCore6CitationAudit.md v0.1.1` 및 테스트 `CFVehicleCore6EvidenceTests.cpp v1.1.0`를 현재 입력안·검증 근거로 사용한다.

- **교정:** Morning `clm-1-tirecode`의 `CitationId`를 Kia 가격표 `src-kia-morning27-price`로 변경했다. GR86 Oman `ref059-coupe-gr86mt26om`은 Core6 Primary 그대로, `ref059-coupe-gr86kr-supp`를 KR Secondary(정확한 MY Unknown)로 추가했다. KR `src-toyota-gr86-kr-wheelbase.ReferenceVehicleIds` 및 `clm-3-wheelbasemm.ReferenceVehicleId`는 Secondary로만 결속한다. Korea Wheelbase2575은 여전히 Candidate이며 Oman의 exact 수치로 승격되지 않는다. `FCFVehicleRefEvidencePayload` 총 6개(Primary6/Secondary1/Source8/FACT84 Canonical23/Candidate61/Unknown24/Conflict1) 유지.
- **직접 영향 테스트:** 새 Source ownership consistency assertion을 84개 Claim에 대해 검증. fixture에 섞인 문서 설명문을 WheelTireVariant 값에서 제거하고, GR86 Secondary 추가 시 Primary 참조 안정성을 위해 TArray 용량을 확보했다. 기존 Validator implementation, VehicleData/Recipe/Other Writer 변경 없음.
- **실제 UE 증거:** 공식 Editor build `32768245e29b4dba9d9fdc0d88f5ed5c`, exit0 (new test compile + module DLL link PASS). `Tools/RunDataAuthoringTests.ps1 -ExactTestNames CarFight.CCAS.CF_FQ_059.P0_02.Core6PayloadValidation -SuppressModelContextProtocolLog -EvidenceId CF059_P002_CiteOwners_20261019_V2`, process `ea72db5bfe2a41ab87a3c8af5729dd3b`, exit0/exact1 success1/failure0/missing0/unexpected0/duplicate0. New JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_P002_CiteOwners_20261019_V2.json` actual readback SHA256 `0d3f8c8145bc97fba24b8e5b059aaf6d854ea79e7909b4e9231e87bdae1396f3`. Historical v1.0.0 PASS applies only to previous input.
- **Existing Evidence ownership/source:** `UCFVehicleRefEvidence` owns persistent `EvidenceId`, `TargetRecipeId`, `TargetRecipePath`, `TargetDefinitionPath`, and semantic `EvidenceFingerprint`. Payload itself owns none of them. Existing `PreviewBuilderCompanions` requires managed valid Recipe GUID + loadable bound Target VehicleData, noncolliding new asset name/path, exact `NewEvidenceId`, complete `InitialEvidencePayload` and computes prospective fingerprint with target binding. `CreateBuilderCompanions` requires **fresh OwnershipWrite R2 approval**, matching ProposalHash/Recipe and Target freshness, then creates the Evidence + any missing private Profile companion(s), binds Recipe and marks Dirty **without Save**. Existing Evidence with new Research payload cannot be overwritten through this path; ambiguous duplicate or partial Recipe/Target binding and stale Evidence fingerprint reject. The existing Builder VM’s persisted Evidence discovery is AssetRegistry + exact Recipe ID/path/Target definition + fingerprint. No special external Host operation was added.
- **실제 저작 진입 판정:** 6개 연구 입력은 이제 Identity/Citation 차원의 **6/6 CONDITIONALLY READY** for companion owner review. 그러나 관리형 Core6 Recipe/Target, unique EvidenceId, companion package identities, optional missing Builder-private profile data, owner approval 및 persistent Save/readback은 **0/6 provisioned or accepted in this task**. 단독 Evidence DataAsset 저장 금지. P0-04 Production/Workbook/Catalog 및 P0-03C trusted AI R2 Create 미실행, USER 최종 grade/physics/mesh 수락 HOLD. UDS `Ready/VCCAS-P0-03C` 유지, 외부 GoPyMCP 및 기존 UE asset 변경0, Git stage/commit/push0.

---

### 13.12.26 Core6 6종 Definition+Recipe 기존 단일 Writer 생성 준비 및 실제 R0 Preview PASS (2026-10-19)

**판정: CORE6 6 CANONICAL PAIRS INPUT READY / 12 NEW PACKAGE NAMES COLLISION-FREE IN FRESH ASSETDUMP + LIVE PREVIEW / UE5.8 BUILD PASS / EXACT1 R0 AUTOMATION PASS (SIX PREVIEWS) / ACTUAL R2 CREATE0, SAVE0, COMPANION0.** 새 독립 입력안·경로·정확한 Request 필드는 `VehicleCore6RecordCreatePrep.md v0.1.0`이 소유한다.

- **실제 소스 계약:** 기존 `FCFVehicleRecordIdentity::BuildDefaultIdentity`가 `VehicleId`를 canonical `/Game/CarFight/Data/Authoring/DA_Vehicle_<Id>`/`DA_Recipe_<Id>` identity로 만든다. `PreviewAIVehicleRecords`(AI R0)와 `PreviewVehicleRecords`(UI/Automation R0)는 같은 충돌/해시/패키지 조건을 검사하며 `CreateVehicleRecords`는 여전히 SlateUI/Automation + fresh OwnershipWrite only; `bSavePerformed=false`. 새로운 Record Writer·서버 권한/Hash gate 없음.
- **실제 후보 6개:** `Morning27_KR`, `Jimny5_AlphaAT_IN`, `GR86_2026_OM`, `HiLux_SR5_AU`, `Venue17_KR`, `StariaHybridCargo5_KR`. 각자 Definition+Recipe 1쌍, 총 **12개 신규 package/object**. 기존 Wagon/테스트 Recipe, 기존 Evidence GUID와 Production 자산에 병합하지 않는다. 새 물리 메시와 차체/소켓은 USER 후속 작업이므로 `ChassisMesh=null`, `ProfileBindings=empty`, `bRequireVehicleSpecificTransmission=true`, `bRequireExplicitHardpointPlan=true`, Basic Sensor default. OEM FACT 23 Canonical/61 Candidate 수치는 Create로 자동 적용되지 않는다.
- **실물 현황/충돌:** 직전 fresh `/Game` 분류 Recipe4/Wagon evidence1/VehicleData12, 이번 fresh `/Game/CarFight/Data/Authoring` root AssetDump `14/14 PASS` dataset fingerprint `d7a6227620404778081481dd547479e8cfb010c7e514fdb1524bcf1d96fdec`, 12 이름 겹침0. UE R0 Preview가 실행 당시 메모리/디스크/객체 충돌까지 PASS, Preview에서 새 패키지 생성0 확인. 실행 직전에는 다시 fresh-preview해야 한다.
- **새로운 test-only 코드:** `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFCore6CreatePreviewTests.cpp v1.0.0`에서 six ID의 Builder naming, AI/UI R0 동일 exact two record paths / ProposalHash / null Chassis/Profile / OwnershipWrite R2 scope / Save0 / `FindPackage=null`을 검증. **공식 UE5.8 Editor Build** `3b10cc8ec4ef4f07ac403e3747f0dbd5` exit0/compile+Link PASS. **focused Automation** `CarFight.CCAS.CF_FQ_059.P0_04.Core6RecordPreview`, process `3b48af0e8a674371a78b9b4028eef694` exit0/exact1 success1/failure0/missing0/unexpected0/duplicate0, six embedded previews PASS. New `EvidenceId=CF059_Core6_RecordPreview_20261019_V1`, result `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_Core6_RecordPreview_20261019_V1.json`, SHA256 `02e3f8bfd2ef0fb7106382b71c09ad21c7a6f1d8dc9d52cd4c9d7d5b79024201` actual JSON readback PASS.
- **실제 생성 미실행:** trusted AI R2 공개/인증 Host contract와 실제 Creation commit은 별개이며, User 요청만으로 AI→Automation caller identity 위장/가짜 승인·해시 전달 금지. 생성이 열리면 각 단일쌍에 fresh Preview→허용된 R2 Create→Definition+Recipe exact readback→별도 명시적 Save/readback 순서. EvidenceId/Recipe/Target/Companion 및 P0-04 Workbook/Catalog/Publish는 추후 개별 후속. 기존 dirty, UE 사용자 자산, GoPyMCP, P0-03C Work `Ready/VCCAS-P0-03C` 보호, Git stage/commit/push0.

---

### 13.12.27 Core6 6종 R2 Create 실행 시도 — Editor 정상 복구 / Trusted Authoring Entry 부재 (2026-10-19)

**판정: PROJECT MANAGED EDITOR START & UE BRIDGE READY PASS / TRUSTED CORE6 RECORD R2 CREATE NOT EXPOSED / PRODUCTION CREATE0 PAIRS / SAVE0 / ACTUAL CORE6 PATH READBACK 0/12.** 상세 검증과 exact Runtime/operation 결과는 `VehicleCore6RecordCreatePrep.md v0.1.1` §8 참조.

- USER가 Core6 6종의 R2 Create 실행을 명시 요청했다. 공식 CarFightMCP_Admin `process.run` Project `carfight/unreal/editor.start`, process `4346b78a686f499faac1afe6b1e15683` exit0, exact managed Runtime `runtime_50d8c839234241c0b9ed3d4d3ebc59e0`, Editor PID46680, `project_runtime_ready=true`, UE Bridge 연결 성공. Editor는 정상 시작 상태로 유지, 다른 사용자 Editor를 강제종료하거나 이어받지 않았다.
- 현 Ready UE Runtime의 exposed toolsets 53개를 조회했으나 **CarFight existing CreateVehicleRecords R2 전용 callable operation이 없고** generic `DataAssetTools.create`는 관리형 두 레코드 생성/연결 계약을 충족하지 않아 미사용. Slate Inspector observer 쓰기는 `ERR_UE_TOOL_BLOCKED` (server policy)이며 read-only Snapshot 성공이 write 허용을 의미하지 않는다. C++ `CreateVehicleRecords`는 기존 SlateUI/Automation만 허용, current AI R0 Preview는 commit 불가. `Automation` caller spoof / generic UObject writer / 임의 Python 실행 우회 없음.
- 현재 런타임 `AssetTools.find_assets` 직독: `/Game/CarFight/Data/Authoring` Existing Definition `DA_Vehicle_Wagon` 하나 및 4개 기존 Recipe(Wagon, TestSedan, TestSUV, DefenseSUV), 여섯 Core6 제안 ID의 Definition/Recipe 12개 **0/12**. **실제 단일 writer R2 호출 0, 생성0/6, Save0/6, Companion0/6**. 기존 R0 six/6 Preview PASS와 UE5.8 Build PASS 증거는 유지. Source/UE asset/GoPyMCP/Workbook/Git stage·commit·push 변동 없음.
- **정확한 다음 작업:** P0-03C에서 기존 관리형 trusted caller의 Project/Checkout/Runtime/R2 operation/terminal receipt를 확인하고 정식 **single writer** `CreateVehicleRecords` 내부 진입을 제공한 다음, 이미 준비된 6개의 입력을 개별 exact single pair로 실행하며 unknown outcome은 먼저 readback 처리. 새 DataAsset raw writer/신규 사용자 반복 승인 인프라를 만들지 않음. 사용자 요청은 이미 6쌍 생성 목표를 명시했으며 권한 없는 호출만 미실행이다.

---

### 13.12.28 P0-03C C-0 UE MCP 프로젝트 Toolset 확장·관리형 R2 호출 정책 집중 검수 (2026-10-19)

**판정: CUSTOM TOOLSET REGISTRY DISCOVERY MECHANISM VERIFIED / DYNAMIC SERVER R2 WRITE DISPATCH NOT PROVEN / NO CARFIGHT R2 TOOL EXPOSED / C-2/3 TRUSTED CREATE STILL HOLD.** 상세 source-backed 범위·정확한 G1~G3 Host owner 질문은 `VehicleAIAdmissionDesign.md v0.1.8` §7.5 참조.

- **UE Bridge 현재 상태:** official CarFight exact Project+Capability+Runtime `runtime_50d8c839234241c0b9ed3d4d3ebc59e0`가 `status=connected`. `ue_list_toolsets` 목록 **53**개 및 `ue_describe_toolset`에서 tool/schema/generation discovery 확인. `SlateInspectorToolset`의 runtime description은 Toolset을 **`UToolsetRegistry`에 등록하면 `ModelContextProtocol`이 자동 탐지**한다고 직접 명시한다. UE 측 프로젝트 Toolset discover 메커니즘은 존재함. 단, CarFight 직접 `UToolsetRegistry` extension 코드 컴파일/실제 등록은 NOT RUN.
- **Host 정책 실측:** Allowed R0 `AssetTools.find_assets`는 현재 Recipe4(Wagon/TestSedan/TestSUV/DefenseSUV) 실제 readback PASS. 반대로 같은 목록에 명시된 `PluginToolset.ListEnabledPlugins`, `PluginToolset.ListDiscoveredPlugins`, `AgentSkillToolset.ListSkills`는 **각각 `ERR_UE_TOOL_BLOCKED`**. 즉 `list/describe` 노출 ≠ `call_read/call_write` 허용. 단순히 CarFight Toolset 작성 및 등록만으로 R2 Create 가능하다고 주장하지 않는다.
- **프로젝트 등록:** 현재 `.uproject`의 `ModelContextProtocol`, `MCPClientToolset`, `AllToolsets` enabled. `CarFight_ReEditor.Build.cs` direct dependency는 아직 없다. Project Registry `.gopymcp/project.json`은 `ue.read` 및 Editor/PIE lifecycle만 explicit 등록, Core6 Create operation 미등록. 이 확인은 **일부 generic UE write가 이미 존재할 수 있음**을 부정하는 증거가 아니며 현재 동적 신규 R2 Toolset call admission 미증명임을 의미한다.
- **G1~G3 exact 외부 확인 범위:** (G1) CarFight project-local `UToolsetRegistry` registration의 실제 plugin API/module dependency/서버 discover+allowlist refresh 계약, (G2) 신규 하나의 `CreateNewVehicleRecords` R2 callable tool이 검증된 caller+Project/Checkout/Ready Runtime에 정식 binding되는 Host 권한 경계, (G3) 단일 요청 terminal receipt+same OperationId replay/response lost/unknown partial handling. 기존 관리형 generic 계약만 확인하고 GoPyMCP에 CarFight 도메인 코드·전용 원장을 추가하지 않는다.
- **후속 CarFight 범위:** G1~G3 충족되면 Editor-only thin Adapter와 기존 two-record Writer의 공통 private 생성 부분만 재사용하며 기존 UI/Automation route를 불변으로 유지한다. 새 Writer/독립 승인 DB/프로덕션 테스트 스푸핑 금지. 현재 정식 custom R2 Toolset/Host write integration **NOT IMPLEMENTED/NOT RUN**, Core6 Create0/6, Save0/6, old Preview6/6 PASS 보존. 코드/UE 자산/GoPyMCP/Git stage·commit·push 0, 기존 Dirty/UDS Ready 보호.

---

### 13.12.29 P0-03C C-0 GoPyMCP 독립 G1~G3 조사 보고서 수령·분석 반영 (2026-10-20)

**판정: G1 TOOLSET REGISTRATION PARTIAL / G2 MANAGED R2 ADMISSION BLOCKED / G3 CALL RESULT RECOVERY PARTIAL; NO NEW GO-PY-MCP R2 FRAMEWORK NECESSARY BASED ON CURRENT EVIDENCE; CARFIGHT TRUSTED CREATE 0/6.** 원문 주체는 별도 GoPyMCP Read-only 조사(Company `CarFightMCP_Admin`, GoPyMCP main HEAD `437c08b00ffa760a1d57e6ab85a85c5ddd828a30`)이며 현 CarFight C++와 정합화한 근거와 소유권은 `VehicleAIAdmissionDesign.md v0.1.9` §7.6에 상세 기록했다. 이 CarFight 세션은 GoPyMCP Source를 독립 직접 조회한 것으로 간주하지 않는다.

- **G1:** 현재 UE MCP는 Toolset 목록과 새 Schema를 upstream에서 동적으로 발견하는 범용 경로를 보유. 그러나 실제 `UToolsetRegistry` C++ 등록 API, Build.cs, 모듈 재등록 및 Editor refresh 조건은 **UNVERIFIED**. 정확한 CarFight Editor Toolset의 소스/플러그인 등록 실험 없이 구현 가능 최종 PASS라 할 수 없음.
- **G2:** 기존 Host는 Gateway OAuth `ue.write`, 검증된 signed request context, exact Toolset/Tool allowlist `Config/mcp_ue_policy.json`, `apply_approved`, Registry Ready Runtime/Checkout binding, Coordinator Epoch, EditorWrite Lease, Audit/AI_PROTECTED_DIRTY를 사용한다. 신규 Toolset은 기본 `default_blocked`, 정확한 메서드 허용 정책 및 정책 로딩 후 승인된 Bridge 갱신이 필요하다. **현재 ChatGPT 연결의 실제 ue.write Scope와 Host 인증 상태를 UE CarFight Toolset에 비위조 형태로 전달하는 계약은 미검증**. Host의 UE generic 쓰기 승인은 CarFight 도메인 OwnershipWrite와 동일하지 않으며 사용자 JSON Boolean/Hash·direct UE localhost 접근을 인증 신원으로 취급 금지.
- **G3:** 기존 `UEBridgeFacade`는 started/succeeded/failed/outcome_unknown/blocked, `UESessionManager.invoke_write`는 one-shot, transport 불명확 시 `ERR_UE_CALL_OUTCOME_UNKNOWN`을 표현한다. 그러나 caller-stable OperationId, `ue.write.status`/`process.status` 재사용, 동일 요청 멱등 replay/receipt는 제공된 조사에서 **미확인**. 새 DB가 아니라 deterministic Definition+Recipe path와 exact UObject class/Recipe GUID/Target readback, unknown에서 자동 중복 재실행 금지로 도메인 복구를 설계한다. Both assets present만으로 이번 호출 완료를 단정하지 않는다.
- **소유 작업 분리:** CarFight/UE: 공급자 plugin의 registry registration API/모듈 확인, 이후 필요 최소 Editor-only Adapter와 기존 `CreateVehicleRecords` shared primitive만 구현. GoPyMCP 범용: 등록될 정확한 Toolset/Tool 정책 허용·현 app `ue.write` scope 실측·인증된 Host 호출과 임의 localhost Toolset 호출의 신뢰 분리 계약 확인. 경계 간 신뢰 증거가 없으면 그것만 최소 공용 계약 교정 여부 판정. 기존 GoPyMCP ProjectWriteRouter/Bridge/Registry/Lease/Audit 재구현하지 않음.
- **의미적 결론:** GoPyMCP 보고서는 C-0을 진행시키지만 C-2/3 Trusted R2 Create admission을 해제하는 증거는 아니다. Core6 R0 Preview6/6, 기존 Validator6/6, official UE5.8 source build PASS는 보존, 실제 Core6 Definition+Recipe 생성0/6, Save0/6, Evidence Companion0/6. 이번 반영은 문서만; CarFight/GoPyMCP code·UE dataasset·runtime 변경/Build/Automation/Git stage·commit·push 없음. 기존 dirty/UDS `Ready/P0-03C` 유지.

---

### 13.12.30 P0-03C G1 UE5.8 Toolset Registry 최소 설계검수 (2026-10-20)

**판정: G1 DESIGN PASS WITH LOCAL ENGINE SOURCE/LIFECYCLE CONFIRMATION REQUIRED / IMPLEMENTATION, BUILD, LIVE REGISTRATION NOT RUN / G2 BLOCKED OWNED SEPARATELY.** 최신 상세는 `VehicleAIAdmissionDesign.md v0.1.10` §7.7.

- **UE5.8 공식 공급자 API:** `UToolsetDefinition` 파생 C++ `UCLASS(BlueprintType, Hidden)`, static `UFUNCTION(meta=(AICallable))`; `UToolsetRegistry`의 `IsAvailable/RegisterToolsetClass/UnregisterToolsetClass/IsToolsetClassRegistered`. 엔진 Plugin module=`ToolsetRegistry`, 공개 헤더 `ToolsetRegistry/ToolsetDefinition.h`, `ToolsetRegistry/UToolsetRegistry.h`. 공식 UE MCP console refresh=`ModelContextProtocol.RefreshTools`; 신규 `UFUNCTION` 추가는 Engine 5.8 공식 문서상 완전 Editor restart 필요. 실제 설치된 공급자 `ToolsetRegistrySubsystem` initialize·auto discovery·등록 종료 시점은 아직 미실측.
- **최소 CarFight 통합:** 별도 프로젝트 Plugin/서버/Writer를 만들지 않고 기존 `CarFight_ReEditor` Editor-only module의 `FCarFightReEditorModule::StartupModule/ShutdownModule`에 lifetime 경계만 통합. 신규 private `UCFVehicleAuthoringToolset`은 **R0 등록·발견 확인 전용**으로 제한, `CarFight_ReEditor.Build.cs`의 Private ToolsetRegistry dependency 추가는 구현 때만. 현재 runtime `CarFight_Re`, 기존 Builder 기능·Asset/Recipe writer 불변. 자동 발견 vs 직접 Register는 installed Engine source 확인 후 한 가지 방식으로 선택, duplicate registration/late init/unregister 보호.
- **보안 경계:** UE MCP localhost 직접 호출은 GoPyMCP의 G2 verified write 권한이 아니다. G2 신뢰 연결 전 R2 `CreateVehicleRecords` wrapper를 `AICallable`로 등록하지 않는다. G1 read-only `ue.list_toolsets/describe_toolset`에서 새 Toolset 1개·정확한 R0 명세를 확인하면 registry G1 기술 검증; server `ue_call_read/write`의 allow/deny와 Domain R2 admission은 별개.
- **G1 다음 수행:** UE 설치 엔진의 등록/초기화/모듈 의존성 source 확인 → 기존 Editor 모듈 읽기 전용 등록의 국소 구현 → 공식 UE5.8 Build → 승인된 fresh Editor/Runtime에서 `list/describe` 결과 및 재등록·종료 수명 readback. 현재 exact Project Runtime 조회는 `runtime_provenance_unknown`으로 `ERR_UE_PROJECT_RUNTIME_INVALID`였으며 **기존 Runtime Ready로 재해석하지 않음**. 어떤 Editor도 이 검수 때문에 재시작하거나 사용자 소유 실행을 중지하지 않았다.
- **기존 체크포인트 보호:** G2 GoPyMCP 주관 R2 권한·정책 및 G3 역할분담 불변. C-2/C-3 Trusted Create 미수락, Core6 Preview6/6 과거 PASS/실제 Create0/6 Save0/6 유지. 이번 변경은 문서 전용, CarFight C++/UE asset/GoPyMCP/Runtime/Git stage·commit·push 없음.

---

### 13.12.31 G1 프로젝트 R0 Toolset 소스 구현 / 컴파일 PASS / Editor DLL 점유로 링크 BLOCKED (2026-10-20)

**판정: G1 SOURCE IMPLEMENTED / UHT & CPP PASS / FULL LINK FAILED EXIT6 (EDITOR DLL LOCK) / REGISTRY LIVE DISCOVERY AND AUTOMATION NOT RUN / G2 R2 AUTH UNCHANGED.** 상세 보고: `VehicleAIAdmissionDesign.md v0.1.11` §7.8.

- 기존 `CarFight_ReEditor`에만 새 `CFVehicleAuthoringToolset.h/.cpp v1.0.0` 구현. UE5.8 `UToolsetDefinition`의 단일 R0 `PreviewVehicleRecordIdentity(VehicleId)`를 노출해 기존 Builder identity helper가 계산한 Definition/Recipe 네 경로를 읽기 전용 구조체로 반환. 별도 Toolset/Registry 서버, G2 Trust 증표, 직접 Writer/Save 기능 추가 없음.
- `CarFight_ReEditor.Build.cs v1.10.6`에 Editor-only `ToolsetRegistry` Private 모듈 의존성, `CarFight_Re.uproject`에 ToolsetRegistry plugin 직접 enabled 항목 추가. 빌드 로그에서 실제 직접 의존성 누락 경고가 확인된 뒤 manifest 교정했으므로 현재 현행 manifest로 재빌드 필요.
- 집중 Registry Automation `CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration`는 클래스 등록·R0 schema/무R2/VehicleId 정상·비정상 검증을 포함하도록 코드를 추가했지만 **실제 실행은 NOT RUN**.
- 공식 UE5.8 Editor job `47a7a0a43f004f9b82d676e0b92955ad`: UHT 3 generated files, 신규 CPP를 포함한 C++ 컴파일과 static lib link 성공. Editor.exe가 `UnrealEditor-CarFight_ReEditor.dll` 점유해 UBA error9001 / NoUba LNK1104, 종료 **exit6 FAIL**. 편집기 소유권 불명과 정확 Runtime `runtime_provenance_unknown`은 보호, 강제 Editor 종료·blind rebuild 금지.
- 다음은 정확한 관리형 Editor 소유권 확인 → 공식 경로로 DLL 잠금 해제 가능한 경우 빌드 exit0 → 격리 Automation exact1/1 JSON readback → 정확한 Project Runtime `ue.list_toolsets/ue.describe_toolset`에서 Toolset+R0 method 실측. G2 호스트 정책·호출 신원, G3 범용 receipt 변경없음. Core6 Definition/Recipe Create0/6, Save0/6.

---

### 13.12.32 G1 사용자 공식 빌드 성공 보고·동시 변경·실제 UE 발견 상태 재조회 (2026-10-20)

**판정: USER BUILD PASS REPORTED / CURRENT G1/G2 SOURCE VERSION DRIFT OBSERVED / UE MCP LIVE DISCOVERY BLOCKED (UNAVAILABLE) / G1 FOCUSED AUTOMATION NOT RUN.** 상세 `VehicleAIAdmissionDesign.md v0.1.12` §7.9.

- USER가 직접 공식 Editor 빌드 성공을 보고했다. 앞선 Admin 빌드 job `47a7a0a43f004f9b82d676e0b92955ad`의 `LNK1104`/exit6는 당시 기록 그대로 보존하며, 별도 USER PASS를 현재 상태로 인정한다. 본 세션에서 USER 빌드 로그·해시를 독립 검증하지는 않았다.
- 최신 `CFVehicleAuthoringToolset.h/.cpp`는 읽기 전용 G1 v1.0.0보다 후속 **v1.1.0** 소스이며 one-use proof와 `CreateNewVehicleRecords` R2 진입이 추가됐다. 이는 별도 실행 신뢰 경계(G2)의 동시 변경으로 식별해 손대지 않음. 사용자 빌드가 해당 v1.1.0의 정확한 바이트를 포함했는지는 아직 미확인.
- 실제 `ue.status`와 `ue.list_toolsets`는 `ERR_UE_MCP_UNAVAILABLE`이므로 새 Toolset의 발견·Schema·G1 Registry PASS는 미증명. G1 focused Automation은 최신 소스/바이너리 버전 정합성과 별도 격리 실행 필요하여 미실행. Editor/GoPyMCP 서비스 무단 재시작·차량 자산 생성·Save/Commit/Push 없음.
- **남은 G1 검증:** UE Ready 재연결 시 `ue.list_toolsets`와 `ue.describe_toolset` 실제 이름/스키마/등록 중복 readback → 소스·바이너리 동일성 확인 후 `CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration` 집중 Automation exact1 검증. G2의 R2 인증·실행 허가는 별도 수락.

---

### 13.12.33 G1 실제 2단계 등록·Automation 검증 FAIL — 원인 경계 확정 (2026-10-20)

**판정: SOURCE+USER BUILD PASS REPORTED / MANAGED LIVE REGISTRY FAIL / FOCUSED G1 AUTOMATION FAIL 0/1 / R2 TRUSTED CREATE NOT ACCEPTED.** 상세 독립 로그/실행 증거는 `VehicleAIAdmissionDesign.md v0.1.13` §7.10.

- **검증 ① (Live):** 관리형 `editor.start` job `c87bbef577264a54af3543b75757c492` exit0, Project+Checkout exact Runtime `runtime_9573e55ffd894df2a4fe003de9ca6e0a`, Editor PID24032, protocol Ready PASS. `ue.status=connected`; `ue.list_toolsets(max_items=120)` **53개 중 CarFight custom Toolset 0**. `ue.describe_toolset` 불가(해당 Toolset 이름 목록에 부재). 이 세션 생성 Editor는 exact `editor.stop` job `3e8e882b66904d58a5c1f206d0be2de7` exit0, Save0, force kill0로 정리.
- **검증 ② (Automation):** 공식 `Tools/RunDataAuthoringTests.ps1` exact `CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration` 단일 실행 job `5a4a3b7405354b2ea3184cba4221ed53` **script exit1**. 격리 JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_G1_Check_20261020_01.json`: engine_exit0, success0/failure1/missing0/unexpected0/duplicate0. 명시 오류 `CFVehicleAuthoringToolset.cpp(200) Expected 'G1 CarFight Toolset registered once at Editor startup' to be true`. UE Registry `IsAvailable` 진단 오류는 없으며 CarFight `IsToolsetClassRegistered` false 확정.
- **최소 수정 후보:** 현 `CarFightReEditor.cpp` 모듈에 명시적인 `RegisterToolsetClass`와 소유 등록의 종료 해제 코드가 없으며 auto-registration 기대가 실패했다. Engine 등록 준비 시점만 좁게 확인하고 한번의 explicit registration + clean unregister 도입(별도 registry 시스템·Polling 도입 금지). **현재 Toolset v1.1.0에 G2의 `CreateNewVehicleRecords` R2 `AICallable`가 함께 있어** 즉시 등록하면 localhost 쓰기 tool도 발견 대상이 됨. 따라서 G2 proof/도메인 신뢰 연결 수락과 등록 노출 시점을 조정한 뒤에만 교정·재검증. 기본 Host 정책 deny와 UE 내부 localhost callable 노출은 다른 경계.
- **이번 작업 영향:** 기존 Code/Assets/GoPyMCP source 변경 없음, 새 automation evidence/report만 발생. G1 최종 PASS 미수락. Core6 actual Create/Save0; G2/G3 별도 소유 유지.

---

### 13.12.34 P0-03C G1/G2 통합검증 완료와 R0 회귀 교정 (2026-10-20)

**최신 판정: G1 Registry Live+Focused PASS / G2 Company 인증 관리형 R2 1쌍 실제 Create PASS / Saved Recipe→Definition persisted binding PASS / AI R0 focused PASS / G3 범용 idempotent receipt NOT PROVEN / Core6 R2 Create0/6 / Product Save 자동화 미수락.** 상세 `VehicleAIAdmissionDesign.md v0.1.14` §7.11, 현재 구현 `Document/Systems/Vehicles/VehicleR2Proof.md v0.1.5`.

- G2 별도 작업의 관리형 인증 실호출로 `G2R2Smoke20261020A` Definition+Recipe 1쌍 생성 성공, `bSavePerformed=false`. 사용자가 정확한 두 에셋을 직접 저장; 이번 회차는 새 AssetDump **Recipe5/5 + Definition2/2** 재실측을 통해 persisted Recipe `TargetVehicleData`가 정확한 Definition object path를 참조함을 확인. 핵심 경계 `ue.write`/Project Runtime/EditorWrite Lease/Bridge one-use proof와 도메인 Writer 재사용은 보존. 별도 GoPyMCP 집중 회귀 기록 **198 PASS**.
- CarFight G1 dedicated Automation `CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration`: Process `5644a800538e4bcf82cde8c04a69fe91` / unique JSON `DataAuthoringAutomationResult_CF059_G1G2_Closeout_20261020_01.json`, **exact1/1 PASS, EngineExitCode0, missing/unexpected/duplicate0**; registered R0+proof-backed R2 Schema 및 직접 proof 없는 R2 거부 검증.
- P0-03C R0 `VehicleAIPreview` 최초 테스트 `c7ce29eefcdd4ce1923f7112febee782` **FAIL 0/1**은 UE5.8 **추상 UObject `NewObject<UObject>` 시험 Fixture Ensure**가 원인. `CFVehicleAIAdmissionTests.cpp v1.2.1`에서 transient concrete `UCFVehicleRecipeData` fixture로 교정(새 도메인 Writer/asset mutation 없음). 공식 `carfight.editor.development` build job `bdffe5bcbc1247ae868c9ad51b016fe9` **exit0 test TU compile + DLL link PASS**; 재시험 `3fb3ceae95874e9e90206b8006c7f32d`, unique JSON `DataAuthoringAutomationResult_CF059_R0Regression_20261020_02.json` **exact1/1 PASS, EngineExitCode0, missing/unexpected/duplicate0**.
- 동일 날 별도 official build job `4cd1156e9fbb4c26923098af8cf7f928`은 exit0이지만 `Target is up to date`로 0 compile. 후속 컴파일은 `CFVehicleAIAdmissionTests.cpp`만 수행하므로 `CarFightReEditor.cpp`의 pre-fix `OnPostEngineInit` C4996 경고 소멸을 독립 증명하지 않았다. 다른 세션 소유 소스 `CarFightReEditor.cpp v1.10.3`의 개별 재컴파일 확인은 해당 세션에서 마감; 혼합 변경을 중복 수정하거나 불필요한 전체 rebuild를 새로 만들지 않음.
- **마감/잔여:** G1 세부기술 수락 PASS, 회사 대상 G2 1쌍 R2 실행 기술 PASS. 타 환경/다른 ChatGPT 클라이언트 OAuth는 미시험, GoPyMCP G2 Git closure는 다른 세션 소유. G3 임의 결과/중복 write receipt는 별도. **Core6 6대 실생성/Recipe 저장·Evidence companion/Workbook/Catalog/Publish P0-04 이후 단계는 미수행**. User-owned saved smoke asset 2개 untracked를 보호하며 stage/commit/push 승인 없이 수행하지 않음.

---

### 13.15 R2 기술 검증 완료와 P0-03C 경계 정리 (2026-10-20)

- **단일 R2 생성 계약 기술 검증 PASS:** Company 관리형 Ready Editor에서 서버 발급 one-use proof를 소비한 `CreateNewVehicleRecords` 정확한 1회 실행으로 Definition·Recipe 각1 생성. 사용자 저장 후 별도 disk-backed AssetDump에서 양쪽 클래스(`CFVehicleData`, `CFVehicleRecipeData`) 및 `Recipe.TargetVehicleData`의 정확한 저장 참조 재검증 PASS. 기존 AI CallerKind/JSON 자체 승인은 허용되지 않음.
- **컴파일·직접 영향 회귀 PASS:** Unreal Engine 5.8 source `CarFightReEditor.cpp v1.10.3` 실제 재컴파일 및 DLL 링크 exit0, `GetOnPostEngineInit()` C4996 경고 재발 없음. G1ToolsetRegistration/VehicleAIPreview 2/2, 기존 MeshCreate/RecordCreation 2/2, 합계 **4/4 PASS**. 정확한 Job·JSON 증거: `Document/Systems/Vehicles/VehicleR2Proof.md v0.1.6`.
- **범위 구분:** 검증용 VehicleId `G2R2Smoke20261020A`는 실제 Core6 생산 차종이 아니며 해당 두 `.uasset`은 사용자 저장한 smoke 증거다. 범용 G3의 지속형 완료 영수증·멱등 재실행, 다른 Host/ChatGPT OAuth 권한, Core6 6종 실생성(0/6), P0-02 OEM Evidence 승인(HOLD), P0-04 저장·Workbook·게시/런타임 수락은 독립 미완료로 유지. 현재 Phase는 VCCAS-P0-03C Ready/PARTIAL이며, 단일 쌍 R2 기술 경로만 독립 완료. 승인 없는 새 Production Create/Save는 하지 않는다.

## 14. 현재 체크포인트

```text
Feature: CF-FQ-059
State: Ready
Phase: VCCAS-P0-03C PARTIAL IMPLEMENTATION / P0-03B HISTORICAL PASS; G1 Toolset registered+focused 1/1 PASS; G2 Company authenticated managed one-pair R2 Create PASS with user-saved Recipe binding PASS. Core6 production Create0/6 / G3 receipt and P0-04 Save/Publish not accepted
Checkpoint: 2026-10-20 Company G2 R2 one-pair smoke Create PASSED with Bridge server-issued one-use proof, direct spoof/approval negative probes PASS, exact new Toolset discovered among 54 and 2 methods described. Two user-saved G2 smoke .uasset files read back via fresh AssetDump (Recipe5/5+Definition2/2), Recipe.TargetVehicleData persisted reference PASS. CarFight focused G1ToolsetRegistration job 5644a800538e4bcf82cde8c04a69fe91 exact1/1 PASS, VehicleAIPreview first test invalid abstract UObject ensure FAIL then corrected test-only v1.2.1 + official UE5.8 build bdffe5bcbc1247ae868c9ad51b016fe9 exit0, retest job 3fb3ceae95874e9e90206b8006c7f32d exact1/1 PASS. CarFightReEditor.cpp per-TU C4996 verification owned by other session; no extra cleanup/push in this turn. P0-03C Core6 actual Create0/6 Save0/6 Evidence0/6; G3 general write receipt PARTIAL
Preserved: VCCAS-P0-00 DESIGN PASS / existing VehicleData sole writer, R2 approval and Workbook authority
Source Changes: P0-04 prep newly added CFCore6CreatePreviewTests.cpp v1.0.0 (R0 Preview-only, Create0). P0-02 CFVehicleCore6EvidenceTests.cpp v1.1.0 (transient typed validation, Morning/GR86 source fixed). P0-03B Provider header v1.1.0 / CPP+Tests v1.1.1 preserved; P0-03C FCFVehicleAuthoringService.h v1.11.0, CFVehicleUXOps.cpp v1.7.0, CFVehicleAIAdmissionTests.cpp v1.2.1 (R0 test-only abstract UObject fixture corrected, WrongAssetType coverage preserved), CFVehicleRecordIdentity.h/.cpp v1.0.0, CFVehicleBuilderVM.cpp v1.39.3 (AI Preview canonical identity, existing Create writer unchanged)
UE Asset Changes: Company G2 smoke Definition+Recipe (`G2R2Smoke20261020A`) 2 saved files **user-persisted**/Git untracked; this closure did not create/Save/modify binary assets, user-modified six Body meshes protected
Build: current official UE5.8 Editor test-only correction job `bdffe5bcbc1247ae868c9ad51b016fe9` exit0 PASS with `CFVehicleAIAdmissionTests.cpp` compile+DLL link; previous job `4cd1156e9fbb4c26923098af8cf7f928` exit0 but 0 actions (cached). Separate other-owner CarFightReEditor.cpp C4996 TU compile not verified. Historical Editor build `3b10cc8ec4ef4f07ac403e3747f0dbd5` exit0 PASS, new P0-04 prep CFCore6CreatePreviewTests.cpp compiled and module DLL linked. Preceding P0-02 correction official build `32768245e29b4dba9d9fdc0d88f5ed5c` exit0 PASS. Earlier C-2/3 `41bacc7d43b3476d8f083f6ef25db4b8` DLL lock exit6 is historical, C-1 `c35c72f769de422d81e8bb86fdab4148` and P0-03B exact3 historical PASS preserved. The current module build does not imply trusted P0-03C AI R2 Create or Product Save.
Automation: Latest new P0-04 Core6RecordPreview focused exact1/1 PASS process `3b48af0e8a674371a78b9b4028eef694` (6 canonical ID AI/UI R0 pairs, collision0/Save0) JSON SHA `02e3f8bfd2ef0fb7106382b71c09ad21c7a6f1d8dc9d52cd4c9d7d5b79024201`. P0-02 changed-payload Core6PayloadValidation v1.1.0 exact1/1 PASS process `ea72db5bfe2a41ab87a3c8af5729dd3b` JSON SHA `0d3f8c8145bc97fba24b8e5b059aaf6d854ea79e7909b4e9231e87bdae1396f3`. P0-03C C-2/3 VehicleAIPreview test v1.2.1 exact1/1 PASS (original abstract UObject fixture fail historical, corrected and JSON readback); G1ToolsetRegistration exact1/1 PASS; C-1 v1.1.0 2/2 historical PASS. Runner isolated EvidenceId, Trusted R2 T-10/11 NOT RUN; prior P0-03B exact3 + CCAS exact9 historical PASS
USER PIE: Not Run / Not Applicable
C-0 Host Gate: G1 REGISTERED/FOCUSED PASS; G2 Company verified one-pair authenticated managed R2 Create/Readback PASS; G3 generic idempotent write receipt still unproven. Core6 actual production 0/6 and P0-04 Save/Publish still NOT RUN, final P0-03C user/production acceptance PARTIAL
```

### Exact next

**VCCAS-P0-03C — G1 live Registry/Schema + dedicated focused exact1/1 PASS, Company G2 authenticated managed R2 single-pair Create and saved Recipe→Definition binding PASS, CarFight VehicleAIPreview v1.2.1 exact1/1 PASS를 보존한다. 다음은 G2 소유 세션의 CarFightReEditor.cpp 경고 개별 컴파일·GoPyMCP 기술/Git 마감 결과를 접수하고, CarFight P0-03C의 Core6 생산용 정확한 Create 범위/경로·G3 결과불명/중복 회수 계약을 검수한 뒤 사용자 승인된 Core6 단위 생성으로 진행한다. P0-04 저장·Evidence·Workbook·Publish는 다음 독립 단계로 유지한다. User-owned saved G2 smoke assets 2개와 다른 Dirty를 보호하며 stage/commit/push를 임의 실행하지 않는다.**

13.5~13.11절의 Historical 경위와, 2026-10-19 `VehicleReferenceSourceReview.md v0.1.1` 및 `VehicleProviderPreReview.md v0.1.0` 검수 결과, `VehicleMeshInventory.md v0.1.7`, `VehicleMeshPreparationChecklist.md v0.1.3`, `VehicleRepresentativeSpecRoster.md v0.1.2`, `VehicleReferenceEvidenceDraft.md v0.1.2`을 기준으로 USER가 CF-FQ-059 차체 선정 범위를 `/Game/CarFight/Vehicles/Meshes` **Core9 exact9**로 동결했다. Legacy Police Car와 CarFight Core9 외부 Offroad/SportsCar 차체는 제외한다. LIVE UE Editor Class/Bounds/Socket readback과 fresh managed AssetDump exact21/21 결과 **Core9 Body StaticMesh class9/9, Body bounds9/9, chassis socket inventory9/9**이다. **I-01 Class PASS**. 이전 P0-01 I-02 (현재 temporary Socket0 및 물리 검수)와 I-03 (임시 외형 중복 판정)은 **USER의 13.10 신규 결정으로 Deferred/Non-blocking for P0-02**이며, 실물 자산 Product/Runtime 검증 의무는 남는다. 과거 AssetDump indexed Body exact3(경찰차 포함) 및 intermediate 후보11/12는 Historical 조사 결과이며 Current 선정 명부가 아니다. Git tracked filename을 실제 StaticMesh나 Production-ready category로 승격하지 않는다.

**현재 임시 차체 메시와 실차 외형을 매칭할 필요는 없다**. Wagon Reference + Core8 Checklist 및 UE read-only evidence는 Historical placeholder 조사로 보존하고 Core6 실차 기준은 연구용으로 계속 사용한다. 사용자가 최종 메시 교체·크기·Socket 편집을 책임지고 나중에 실제 UE 자산과 연결한다. `VehicleReferenceEvidenceDraft.md v0.1.2`의 six vehicle provenance/unknown draft와 `VehicleReferenceSourceReview.md v0.1.1`의 24개 치수 claim source review, `VehicleProviderPreReview.md v0.1.0`의 실제 C++ source review를 완료했다. **P0-02 source facts PASS이지만 exact trim/identity acceptance와 saved `UCFVehicleRefEvidence`는 HOLD**다. **P0-03A VehicleSchema 및 직접 영향 CCAS exact5가 EngineExitCode0, PASS 5/5로 실제 실행·증거 재조회까지 완료됐고 P0-03B typed import/current/diff 및 실제 Wagon persisted readback도 최신 exact3 Automation PASS로 기술 수락됐다. 과거 구현 범위는 P0-03C AI R0 Preview + C-1 Build 및 집중 Automation 2/2 PASS였으나, 최신 2026-10-20 Company G2 R2 단일 Create/저장 Recipe binding PASS 및 별도 CarFight G1/R0 focused 각 1/1 PASS로 갱신됐다. 범용 G3 idempotent receipt와 Core6 6종 실제 생성·P0-04 Product materialization은 여전히 미수락이다. P0-02 Reference Evidence 최종 승인 HOLD는 별개다.** AI R2는 신뢰된 Host operation/adapter 계약이 검증되기 전 fail-closed하고, User physical meshes/Socket는 계속 후순위다. CityCar/Compact/Coupe/Pickup/SubCompact/Van 6종은 현재 임시 차체에서 실제 Wheel Socket4 + Hardpoint Socket2씩 확인됐으며, 추가로 Sedan/SUV/Wagon도 네 Wheel Anchor를 확인했다. 현재 소켓이 있다고 해서 추후 최종 교체 메시/차체 물리 Acceptance를 면제하지 않는다. SUV/Sedan에는 기존 Wheel Anchor4, Wagon에도 Wheel Anchor4+HP3이 있으므로 재작성하지 않는다. GPT는 지금 ReferenceSpec/Unknown/Claim provenance와 다음 Vehicle typed schema/authoring을 진행하고, 실제 Wheel visual/physics/final replacement mesh 검사와 Runtime 검증은 자산 통합 시점에 재개한다. P0-04 Product materialization과 P0-06 Runtime에 필요한 유효한 물리 자산은 가상 값으로 통과시키지 않는다. P0-00 설계 PASS는 유지하며 AI R2 path, Vehicle Provider materialization, Workbook mutation, Catalog publication, runtime operation을 이미 구현했다고 주장하지 않는다. P0-03A의 이전 판본 1/1 PASS는 Historical이고 최신 v1.0.5/v1.0.4의 최종 5/5 PASS가 Current이다. Schema-only phase 마감; P0-03B read-only import/current/diff v1.1.1 exact3 Automation과 실제 Wagon persisted source readback은 PASS이며, 다른 독립 UE MCP connection 경로의 장애 조사와는 분리한다. P0-03C trusted AI R2 단일 생성 경로는 Company 환경에서 실측 PASS이나 Core6 생산6/6은 미실행이고, P0-04 영구 Product authoring Writer/Publisher는 여전히 미구현이다.

---

## 15. Changelog

### v0.1.55 - 2026-10-20

- G1/G2 단일쌍 R2 실증·사용자 저장 AssetDump 검증·정확한 `CarFightReEditor.cpp` UE5.8 컴파일 C4996 0·직접 영향 Automation 4/4 PASS 수락을 추가한다. P0-03C 전체와 Core6 0/6, G3/P0-04 미완료는 유지한다. `VehicleR2Proof.md v0.1.6` 참조.

### v0.1.54 - 2026-10-20

- CarFight G1/G2 integration evidence reconciled to current checkpoint: G1 registration + dedicated exact1/1 PASS, Company G2 authenticated one-pair R2 smoke PASS, independent saved Recipe→Definition ref/class AssetDump PASS, R0 test-only abstract UObject fixture corrected v1.2.1 and dedicated exact1/1 PASS, official UE5.8 Editor test TU compile+DLL link exit0. Other session owns warning-check TU/GoPyMCP dirty; no Git commit/push. Core6 actual 0/6 and G3 generic recovery pending. Details `VehicleAIAdmissionDesign.md v0.1.14` §7.11 / `VehicleR2Proof.md v0.1.5`.

### v0.1.53 - 2026-10-20

- G1 two requested checks run sequentially: exact managed Project Editor Ready and list53/custom0; official clean Editor stop; focused standalone Automation 0/1 FAIL with `UToolsetRegistry::IsToolsetClassRegistered` false and isolated result JSON readback. G1 explicit registration correction needed, but G2 R2 method exposure means trust boundary must be reviewed before registration. Details `VehicleAIAdmissionDesign.md v0.1.13` §7.10. No C++/assets/GoPyMCP edits, Git staging/commit/push0.

### v0.1.52 - 2026-10-20

- USER official Editor Build PASS reported after historical Admin DLL lock exit6; source now CFVehicleAuthoringToolset v1.1.0 with another session's G2 proof-backed R2 method, not altered or accepted by G1. Current UE status/list_toolsets unavailable, focused G1 Automation NOT RUN; direct changes limited to documentation, no Editor runtime/Asset/Git mutation. Details `VehicleAIAdmissionDesign.md v0.1.12` §7.9.

### v0.1.51 - 2026-10-20

- G1 read-only Toolset source added in existing CarFight_ReEditor, Build.cs/.uproject ToolsetRegistry dependency integrated, isolated registry Automation registered in source. UHT and CPP compile PASS, first full official build exit6 due current UnrealEditor DLL lock. Source change done, live Registry/Automation NOT RUN. G2/G3/GoPyMCP/R2 unchanged. Detailed evidence `VehicleAIAdmissionDesign.md v0.1.11` §7.8.

### v0.1.50 - 2026-10-20

- G1 UE5.8 Toolset Registry design review: Epic official C++ register/unregister/base/tool metadata API + Editor refresh/restart evidence, existing CarFightReEditor module owner inspected, single R0-only Toolset in Editor module favored; installed engine lifecycle/actual compile+registration NOT RUN. G2/G3 split preserved, no Engine/GoPyMCP/CarFight C++ mutation, Core6 Create0. Details `VehicleAIAdmissionDesign.md v0.1.10`.

### v0.1.49 - 2026-10-20

- Integrated GoPyMCP independent G1 PARTIAL / G2 BLOCKED / G3 PARTIAL source-policy-runtime findings into CarFight C-0 checkpoint. Existing `ue.call_write` generic write route, exact policy default_blocked, domain OwnershipWrite trust gap, one-shot `outcome_unknown` without durable idempotent receipt, source ownership and narrow next decision documented in `VehicleAIAdmissionDesign.md v0.1.9`. No C++/UE/GoPyMCP changes or actual R2 Create/Save.

### v0.1.48 - 2026-10-19

- P0-03C C-0 actual UE MCP extension/Host policy audit: Ready runtime 53 toolsets, `UToolsetRegistry` discovery source, custom CarFight R2 exposed0, allowed `AssetTools.find_assets` read vs 3 registered `ERR_UE_TOOL_BLOCKED` reads. Documented G1 custom toolset registration/allowlist, G2 verified R2 caller/project/checkout/runtime, G3 idempotent receipt/unknown outcome exact questions in `VehicleAIAdmissionDesign.md v0.1.8`; code, assets, runtime/editor, GoPyMCP unchanged. Actual Core6 Create/Save 0.

### v0.1.47 - 2026-10-19

- USER Core6 R2 Create 실행 요청 후 managed Editor start job `4346b78a686f499faac1afe6b1e15683` PASS and UE Bridge Ready. R2 CarFight Authoring operation not exposed in 53 toolsets, Slate UI write blocked; AssetRegistry confirmed Core6 0/12, actual R2 Create0/Save0, no generic asset/Automation caller bypass. `VehicleCore6RecordCreatePrep.md v0.1.1` §8 recorded exact ownership issue. Existing test/build PASS and prior Dirty preserved.

### v0.1.46 - 2026-10-19

- Core6 6 vehicle / 12 canonical Definition+Recipe package candidates prepared with exact source-contract request flags. Fresh AssetDump Authoring14/14 no name collisions; added test-only `CFCore6CreatePreviewTests.cpp v1.0.0`; official UE5.8 build PASS and focused isolated Core6 R0 AI+UI Preview six/6 PASS, JSON readback SHA. See `VehicleCore6RecordCreatePrep.md v0.1.0`; Create/Save/Profile/Companion/GoPyMCP/Trusted R2 writes none.

### v0.1.45 - 2026-10-19

- Core6 Morning TireCode Citation 직접 가격표 결속, GR86 Korea Secondary reference + Source/Claim ownership 정합화. Test fixture v1.1.0, current official UE5.8 build PASS and isolated exact1 Validator PASS/JSON readback. Existing EvidenceId/Recipe/Definition companion ownership approval/Preview/Create and Save0 contract audited; research preparation 6 conditional ready, Core6 persistent Evidence Save0 and final USER authorization HOLD. Production assets and GoPyMCP unchanged.

### v0.1.44 - 2026-10-19

- P0-02 6-vehicle final Research Identity/Citation provenance audit: complete per-Claim84 decision matrix / OEM Source8/Unknown24/Conflict1; retained Canonical23/Candidate61, actual Morning `TireCode`→price Source citation mismatch and GR86 KR supplemental Source owner→Oman Primary issue identified; Venue 1546/1545 conflict, HiLux approximate/max/range, STARIA ICE/system, unknown ModelYears preserved. Research Evidence preparation 5 conditional ready / GR86 1 blocked; final Persist/USER acceptance HOLD. New `VehicleCore6CitationAudit.md v0.1.0`; validator code/build/test/Asset/GoPyMCP/Git unchanged.

### v0.1.43 - 2026-10-19

- P0-02 six actual `FCFVehicleRefEvidencePayload` transient C++ fixture and focused validator Automation. New `CFVehicleCore6EvidenceTests.cpp v1.0.0`; normal6/6, invalid11/11. Official UE5.8 Editor Build `8cf022cd62ac46cda8510296acfe34f3` exit0, process `2e70e468bcf64aabb66e76e22c4077c2` exact1/1 PASS and unique EvidenceId JSON SHA readback. Persistent Product/Evidence Save0, GoPyMCP/Trusted R2 code0; final identity/source-market/Evidence USER gate remains HOLD.

### v0.1.42 - 2026-10-19

- P0-02 Core6 24 외형 치수 중 제조사 동일 사양 범위 근거 23건만 research Canonical로 채택, GR86 Korea cross-market Wheelbase 1건은 Oman Primary Candidate 유지. 나머지 FACT60 Candidate, Venue rear track 1546/1545 Warning conflict/Unknown1, Toyota AU HiLux approximate 치수와 max/range mass qualifier 보존. `VehicleCore6EvidencePayloads.md v0.1.1`와 SourceReview/Draft 정합성 동기화. Actual UE Validator·Persisted Evidence NOT RUN, P0-02 Full Acceptance HOLD; 문서 외 변경 없음.

### v0.1.41 - 2026-10-19

- P0-02 Core6 `FCFVehicleRefEvidencePayload` 독립 6건을 `VehicleCore6EvidencePayloads.md v0.1.0`로 작성해 실제 Evidence C++ 타입과 Source/Facts/Unknown/Conflict의 ID·Citation 관계를 정적 검사. Primary6/Source8/FACT84(24 치수)/UnknownFact24/Conflict1; source/claim/unknown unique와 차량별 인용 관계 PASS. All Candidate로 실제 UE Validator NOT RUN·Final Evidence HOLD 유지. 문서만, C++/Asset/GoPyMCP/Git 작업 없음.

### v0.1.40 - 2026-10-19

- P0-02 Core6 source/Evidence consistency audit: six OEM references exterior L/W/H/WB exact24/24 fresh recheck; Jimny working official PDF URL and STARIA SourceId alias reconciled; Venue brochure 17-inch rear track 1545/1546 in-source conflict surfaced, never canonicalize without resolution; GR86 Oman/KR cross-market, HiLux max/range, STARIA ICE/system separation preserved. Full six typed Evidence payload/UE authoring/P0-02 USER acceptance HOLD, independent P0-03C phase preserved. Documentation-only, UE/GoPyMCP/build/Automation/Work lifecycle/Git changes none.

### v0.1.39 - 2026-10-19

- CF-FQ-059 P0-03C의 기존 UI·Builder 차량 생성 T-13 회귀 시험을 source-level로 독립 감사. 실제 Automation selector 두 건, Definition+Recipe 생성·Builder adoption/Chassis 재사용·Save0·원본 보존 검증 범위를 확인했고 공통 test fixture cleanup의 package Dirty 미보장 경계를 기록. 격리된 unattended 테스트 실행 조건만 확정하고 C++/에셋/빌드/Automation/타 프로젝트 작업은 하지 않음. 기존 상태·미검증 범위·Dirty 보호.

### v0.1.38 - 2026-10-18

- C-2/3 Host-independent R0 음성 회귀를 기존 `CFVehicleAIAdmissionTests.cpp v1.2.0` 한 파일에서만 3건 최소 보강했다. 변경 소스 compile 진행 성공 뒤 실행 중 Editor의 `UnrealEditor-CarFight_ReEditor.dll` 점유로 official UE5.8 build job `41bacc7d43b3476d8f083f6ef25db4b8` exit6 FAIL. 새 Automation NOT RUN, 기존 C-1 2/2 historical PASS 보존; UE 에셋·기존 데이터·GoPyMCP 수정0, Host R2 gate 유지.

### v0.1.37 - 2026-10-17

- C-2/3 CarFight 기존 writer·UI/Builder caller·AI mutation0 Preview·회귀 테스트를 읽기 전용으로 대조하고, Host-independent 입력/회귀 점검과 Host-dependent trusted R2 admission/receipt 경계를 분리했다. 기존 Create 호출자 제한, single writer/Save0, unknown partial state 보호 유지; C++/UE Asset/GoPyMCP 수정 및 신규 테스트·빌드 없음. UDS Ready/P0-03C 보존.

### v0.1.36 - 2026-10-17

- P0-03C C-0 Company 관리형 Host/UE Bridge 공개 계약을 읽기 전용 검사. `.gopymcp/project.json`의 등록 operation 6종, UE ReadOnly/WriteProbe exposed toolset 53종, `ue_status`, 현재 C++ Create gate와 `diag.selftest_core`를 실제 대조한 결과 신뢰된 Vehicle R2 Create operation·exact authority·terminal receipt 계약이 **미증명 / BLOCKED**임을 확정했다. Generic Asset Create / `apply_approved` / legacy process ID를 R2 admission으로 대체하지 않는다.
- UDS Work immutable record 26개의 maximal head exact1을 fresh 검증, 기존 v25와 `heads.json` 일치. Current projection은 과거 checkpoint이므로 lifecycle을 재작성하지 않았다. 기존 C-1 공식 UE5.8 빌드+Automation 2/2 PASS 및 P0-03A/B 수락을 보존하고 C-2/3·T-10/11은 미실행; GoPyMCP 별도 소유 계약 확인 뒤 최소 기존 writer 연결. 문서 범위 수정만 수행.

### v0.1.35 - 2026-10-17

- 기존 PER-01 legacy `main_game` process 성공 이력을 재검토한 결과, 직전 `carfight_repo`의 `process_repository_not_found`는 Project repository_id를 legacy `repository_id`에 잘못 넣은 호출 오류였음을 확정했다. `main_game`으로 현재의 `Tools/RunDataAuthoringTests.ps1 v1.3.2`가 정상 시작·종료됨을 실제 검증했다.
- C-1 정확한 focused 2건(`VehicleAIPreview`/`VBCUX_P0_03.Naming`)을 각각 새 EvidenceId로 분리 실행하여 **2/2 PASS / EngineExitCode0 / 실패·누락·중복 0 / 두 JSON readback PASS**. 기존 고정 결과 파일 덮어쓰기 없이 PER 실행 계약 그대로 사용했고 신규 기능/Tool은 추가하지 않았다.
- 공식 UE5.8 Build `c35c72f769de422d81e8bb86fdab4148` PASS 유지. C-0 trusted AI R2 인가 미확인, C-2/3 실제 AI Create 미구현, P0-04 Save/Publish 미착수. 이력 v0.1.34의 당시 BLOCKED는 Historical로 유지하되 Current checkpoint와 exact next를 수정.

### v0.1.34 - 2026-10-16

- C-1 증거 보존형 Automation 실행 준비: `Tools/RunDataAuthoringTests.ps1 v1.3.2`에 opt-in `-EvidenceId` 격리 JSON·로그 경로와 기존 이름 충돌 fail-closed 추가. 기본 사용자의 출력·schema·행동은 유지하며 다른 세션의 기본 결과를 건드리지 않음.
- `process.run` generic script는 `carfight_repo`의 legacy configured-repository namespace에서 `process_repository_not_found`로 **미시작**했고, Project+Legacy 혼합 호출도 `project_target_conflict`로 **미시작**. UE read-only status/list도 내부 오류. 기존 등록 `carfight/unreal` Project에는 테스트 process operation이 없어서 **focused exact2 NOT RUN**; 시험 결과를 가정하거나 새 Project operation을 근거 없이 등록하지 않음.
- C-1 공식 UE5.8 Editor Build PASS 기준선 보존. 실행 계약 차단의 범위를 CarFight 테스트 실행 등록 문제로 한정하고 C-0 R2 Host admission HOLD와 구분. GoPyMCP/UE Asset/Workbook/Git commit·push 변경 없음.

### v0.1.33 - 2026-10-16

- P0-03C C-1 구현: `CFVehicleRecordIdentity.h/.cpp` 신규, 기존 `CFVehicleBuilderVM.cpp`의 VehicleId/기본 identity를 동일 helper로 이동, `CFVehicleUXOps.cpp` AI mutation0 Preview에 canonical path/name 검증 추가, AI 회귀 test v1.1.0에 /Game 외부·other /Game root·Recipe 이름·VehicleId invalid 거부 검증 보강. SlateUI/Automation Advanced/기존 R2 Create/Save contract 보존.
- UE5.8 Source Editor 공식 managed Build `c35c72f769de422d81e8bb86fdab4148` **Succeeded exit0**. focused Automation **NOT RUN**: runner가 다른 세션 `UE/Saved/CarFight/DataAuthoringAutomationResult.json`와 `UE/Saved/Logs/CFDataAuthoringAutomation.log`를 무조건 삭제/재작성하므로 증거를 보호함.
- C-0 기존 Host trusted AI R2 write 계약 HOLD, C-2/3 실제 writer 연결 미구현. UI 중복 승인 추가 없음, GoPyMCP/UE Product/Workbook mutation 없음, Git commit/push 없음.

### v0.1.32 - 2026-10-16

- `VehicleAIAdmissionDesign.md v0.1.1`에 P0 3건(P0-01 사용자 지시 vs 관리형 호출 권한, P0-02 미확인 실제 R2 entry, P0-03 SlateUI/Automation 외부 위장 경계) 및 P1 3건(P1-01 hash에 참조 내용 미포함, P1-02 Host 중복/부분실패 receipt 소유, P1-03 기존 Builder naming 공유) 교정 반영.
- C-0 기존 manage write contract 확인을 구현·리팩터링의 선행 조건으로 이동. 추가 사용자 확인 UI/미확인 UserIntentRef·토큰 시스템 강제 폐기. 정확한 ProposalHash는 R0 이후 fresh 비교하고, 최종 내용 승인/OEM 품질은 P0-04로 분리.
- 이 기록은 설계만 변경. **교정 후 CarFight 내부 설계 재검수 P0/P1 6/6 PASS; 실제 Host R2 entry 계약 C-0 HOLD. 실제 R2 연결 구현·집중 Automation은 NOT RUN**. GoPyMCP 장애 수리/GoPyMCP 코드/UE Product Asset/Workbook/Git stage·commit·push 변경 없음.

### v0.1.31 - 2026-10-16

- `VehicleAIAdmissionDesign.md v0.1.0` 신규 작성: 인증된 사용자의 신규 차량 제작 지시를 중복 승인 질문 없이 1쌍 R2 작성 범위로 연결하고, 기존 SlateUI/Automation gate를 보존하는 Host/CarFight trusted admission·단일 writer·error/replay/partial-state·test T-01~14를 설계.
- 실제 Host가 사용자 origin 및 exact R2 scope를 증명·전달할 수 있는지 미확정으로 분리. 제공하지 못하면 AI Create fail-closed; GoPyMCP 코드/장애 작업은 이 Work 범위 외. **DESIGN DRAFT / REVIEW NOT RUN / R2 IMPLEMENTATION NOT RUN**.
- P0-03C Preview + official Build PASS, P0-03A/B 수락, P0-02 Evidence HOLD, P0-04 저장/게시 미착수 및 기존 Dirty 보호 유지.

### v0.1.30 - 2026-10-16

- P0-03C source 첫 단위: `PreviewAIVehicleRecords` mutation0 R2 proposal을 독립 AI identity에서 제공하며 기존 `PreviewVehicleRecords`(SlateUI/Automation)와 path/type/collision/hash 핵심 로직을 공유. AI를 Automation으로 위장하지 않고 기존 `CreateVehicleRecords`의 UI/Automation-only guard 및 R2 approval을 보존.
- 신규 경로의 `/Game/` 패키지 범위 보호 및 `CFVehicleAIAdmissionTests.cpp` negative/compatibility regression 추가. AI 자기선언 approval로 R2 Create 실행은 여전히 불가능하며 권한/사용자 명령이 실제로 승인됐다는 증명이 아닌 읽기 전용 리뷰 기능이다.
- Canonical `Tools/BuildEditor.bat`의 Project `carfight/unreal/editor.build` 관리형 UE 5.8 Build `160b950da4374d1db0b35d79c327412a` **PASS / exit0**. P0-03C `CarFight.CCAS.CF_FQ_059.P0_03C.VehicleAIPreview` focused Automation **NOT RUN**, 새 기능 Technical Acceptance 미성립.
- P0-03C 잔여: 정확한 Project/Checkout 및 actual managed-caller authority와 OwnershipWrite review scope를 신뢰 경로에서 수락·결속하는 bounded adapter 구현 → AI Create R2 재검사 및 spoof/stale/duplicate 회귀 → 테스트 결과 확인. UE MCP 장애 조사/GoPyMCP 코드 수정은 이 Work 범위 밖. P0-02 OEM Evidence HOLD, P0-04 저장/Workbook/Publication 미착수, UE Product Asset/Workbook 0, Git stage/commit/push 0.

### v0.1.29 - 2026-10-16

- USER 실행 최신 v1.1.1 exact3와 저장소 `DataAuthoringAutomationResult.json` 교차 확인: **VehicleSchema / VehicleReadDiff / WagonPersistedReadback 3/3 PASS**, EngineExitCode0, failure/missing/unexpected/duplicate0. 공식 UE 5.8 Automation 실행 로그도 확인했으며 독립 UBT 최종 종료 전문은 별도 회수하지 않음.
- 실제 Wagon persisted Recipe/Evidence binding·Canonical4 dimensions mapping·Provider ImportExisting/BuildCurrentSnapshot/ReadbackFingerprint/BuildDiff·dirty0 통합 검사 PASS. 테스트 3종 실차 숫자 승인 아님; Wagon-only OEM 수락 기준 유지.
- P0-03B source-focused/build-integration 기술 게이트 충족 → **P0-03B TECHNICAL PASS/CLOSED (phase)**; P0-03C trusted R2 admission 설계·구현 준비 상태, P0-02 final Core6 Evidence HOLD 및 Production 생성·Workbook writer 별도 미착수.
- 이번 증거 기록은 Plan/UDS만 갱신, C++/UE Asset/Workbook/타 세션 Dirty/Git stage·commit·push 없음.

### v0.1.28 - 2026-10-16

- 실제 Wagon `Dimensions.Length/BodyWidth/Height/Wheelbase`와 기존 `Exterior*Mm/WheelbaseMm`의 정확한 case-sensitive canonical field mapping을 Provider v1.1.1에 추가, source Width/BodyWidth 혼동 및 duplicate alias/wrong mm unit을 fail-closed. 기존 Evidence/Workbook/VehicleData/Recipe mutation 없음.
- Tests v1.1.1에 synthetic 정합성·negative regression 보강 및 **기존 Wagon persisted source를 사용하는 별도 `WagonPersistedReadback`** 등록. 원본 TestSedan/TestSUV/DefenseSUV의 추정 수치는 실차 검증에서 제외하고 generic Import 구조 안정성만 확인.
- 관리형 빌드 시도는 Project operation 미등록 및 legacy preset-repository 불일치로 모두 started=false, UE 읽기 도구도 내부 오류. **최신 v1.1.1 build+focused exact3 NOT RUN**; 과거 v1.1.0 exact9 PASS는 이력으로 보존. 다음 official UE 5.8 build + three tests + final review에 수락 판정 위임.

### v0.1.27 - 2026-10-16

- USER 범위 확정: **실제 기존 차량 중 Wagon만 실차 스펙·OEM Evidence·FactKey mapping 검증 기준**. TestSedan/TestSUV/DefenseSUV는 임의 수치를 쓰는 테스트 전용 차량이므로 실차 수치 검증·Evidence 보강·Production 승격 대상에서 제외하고 read-only 구조적 안전성만 확인.
- 현재 `ImportExisting`는 저장된 Recipe를 전부 순회해 실패 한 건에 전체 import가 fail-closed 되는 실제 코드 계약을 확인. 테스트 자산을 무단 제거/하드코딩 필터하지 않으며 Wagon live readback을 방해하는 문제 여부만 검증한다.
- 장래 Core6 Production 신규 차종은 별도 OEM 대표차 source를 이용한다는 목표는 유지. P0-03B Wagon Dimensions FactKey mismatch와 실제 Provider live smoke HOLD 유지. 문서/UDS scope correction only, C++/UE Asset/Workbook/Git stage·commit·push 없음.

### v0.1.26 - 2026-10-16

- Managed AssetDump를 `changed_only=false/rebuild_index=true/managed_dataset=true`로 새 수집. 이전 cache Recipe2가 아니라 **Recipe4/4 + Evidence1/1 + VehicleData12/12** fresh source readback PASS, distinct valid RecipeId4 및 Target paths exact4 존재 확인.
- Wagon Evidence의 TargetRecipeId/TargetRecipePath/DefinitionPath·EvidenceId·fingerprint가 저장 Recipe/Builder receipt와 일치하며 Sources5/Claims20/Unknown5/Conflict0을 확인. 단 nested preview 8/20이고 Provider runtime fingerprint 재계산 NOT RUN.
- 실제 Wagon FactKey `Dimensions.*`와 Provider `Exterior*Mm/WheelbaseMm` projection 키 불일치를 발견해 P0-03B compatibility 교정·실제 Provider live 호출 수락 게이트 HOLD. Width/BodyWidth를 임의 병합하지 않음. 사용자 요청 범위는 read-only 검증이며 C++/UE Asset/Workbook 변경 없음.

### v0.1.25 - 2026-10-16

- User terminal exact9 결과와 CarFight 저장소의 v2 `DataAuthoringAutomationResult.json`을 교차 조회: **SUCCESS9/FAILURE0/MISSING0/UNEXPECTED0/DUPLICATE0/EngineExit0**. `VehicleReadDiff`, `VehicleSchema`와 인접 CCAS 7건 모두 PASS.
- 기존 Source/Consumer 및 writer fail-closed 경계 최종 정적·Slop 검수 PASS. 별도 신규 Production Provider/Writer/Tool, Core 수정, 사용자 Dirty 변경 없음.
- UE read-only `ue_status`·`ue_list_toolsets` 내부 오류로 persisted Recipe/Evidence live smoke는 BLOCKED; exact9만으로 전체 P0-03B Technical Acceptance까지 확대하지 않음. 실제 자산이 준비될 때 기존 경로에서 검증하고 재빌드·동일 test 반복은 생략.

### v0.1.24 - 2026-10-15

- USER의 P0-03B v1.1.0 공식 UE5.8 Source Editor BUILD PASS 보고와 source mtime보다 늦은 Win64 Development Editor target receipt(15:55:34 KST)를 교차 확인. 직접 UBT exit code 미회수는 별도 표시.
- 같은 빌드 재실행 불필요. 현재 result JSON은 타 DAUTH MeshCreate 테스트 결과이므로 VehicleReadDiff/exact9는 아직 NOT RUN. 다음 Gate를 P0-03B exact9→persisted read-only smoke→최종검수로 갱신.
- 다른 세션 Dirty, 차량 에셋/Workbook, 신규 Product write, Git stage/commit/push 없음.

### v0.1.23 - 2026-10-15

- P0-03A exact5 PASS 보존한 채 P0-03B read-only typed Recipe/Evidence/current/diff 소스 v1.1.0 적용. 기존 AssetRegistry/CCAS Core/VehicleBuilder writer 재사용, Production write 미개방, schema rev2 Source Boolean/Range 및 exact Recipe/Target/Chassis path identity 추가.
- 기존 ResourcePilot의 Recipe GUID별 stable ChassisResourceId를 재사용하고 실제 AssetPath만 diff field로 표시. 별도 Resource ID 시스템/Registry 신설 없음.
- Transient `VehicleReadDiff` focused test를 별도 등록. 최신 빌드/자동화는 아직 NOT RUN이며 다음 실측 exact9 테스트 및 persisted smoke test가 P0-03B 수락 선행조건이다.
- 사용자 6종 차량 Body Mesh·기타 Dirty/Workbook/Engine/Product 미변경, Git stage/commit/push 없음.

### v0.1.22 - 2026-10-15

- USER 실행 결과와 canonical `UE/Saved/CarFight/DataAuthoringAutomationResult.json` readback을 교차 검증해 latest Provider v1.0.5/Tests v1.0.4 대상 exact5 5/5 PASS, EngineExitCode0, 실패/누락/예상외/중복0 확정.
- P0-03A static source final review 및 adjacent CCAS regression 수락 조건 충족 → **P0-03A TECHNICAL PASS / CLOSED (phase)**. 단일 P0-03A 1/1 구 판본 PASS와 historical FAIL은 이력으로 보존.
- 현 Work는 `VCCAS-P0-03B READY`로 진입: typed Recipe/Target/Evidence import/current snapshot/diff 구현은 미착수, P0-02 final Evidence HOLD와 AI R2/Production 6종 제작은 별개. Unreal Asset/Workbook/타 세션 Dirty 변경 및 Git stage/commit/push 없음.

### v0.1.21 - 2026-10-15

- P0-03A source/Registry/Compiler/Consumer 최종 정적검수 PASS, Production 자동등록·쓰기 경로 무변경 및 user/other session Dirty 보호 확인.
- Claim Provenance FACT/DERIVED 문자열 비교의 implicit case-insensitivity를 직접 영향 결함으로 발견해 Provider v1.0.5와 focused Tests v1.0.4를 단일 semantic unit으로 교정. Lowercase fact/derived와 GAME_BIAS negative regressions 보강.
- 실제 adjacent test 대상을 ProviderRegistry/ProviderValidation/SchemaCompatibility/SnapshotConsistency + VehicleSchema exact5로 확정; 이전 VehicleSchema 1/1 PASS는 보존하지만 최신 source build/5-test 재실행은 NOT RUN. P0-03A Technical Acceptance HOLD.

### v0.1.20 - 2026-10-15

- USER 재실행 및 실제 결과 JSON readback으로 **VehicleSchema focused Automation PASS 1/1, EngineExitCode0, 실패·누락·예상외·중복 종료 0**을 확인했다.
- 실패했던 mixed-case Vehicle 검증은 correction v1.0.4/v1.0.3 기준 PASS; 과거 FAIL은 역사적 기록으로 보존하고 현재 상태를 갱신했다.
- 인접 CCAS Registry/canonical identity 회귀와 최종 검수는 아직 미실시이며 P0-03B Product read/diff, P0-04 6종 VehicleData/Workbook authoring은 열지 않았다. UE Asset/사용자 Dirty/기존 Workbook 변경 없음.

### v0.1.19 - 2026-10-15

- USER가 실제로 실행한 CF-FQ-059 VehicleSchema exact1의 결과 **FAIL 0/1, Engine Exit 0**를 확인했다. 유일한 테스트 실패 이벤트는 `CFVehicleCCASProviderTests.cpp:159`의 Mixed case Vehicle rejection이며, 실행기 성공과 테스트 성공을 혼동하지 않는다.
- `CFVehicleCCASProvider.cpp v1.0.4`에서 ContentType/RowId canonical case-sensitive 보장, `CFVehicleCCASProviderTests.cpp v1.0.3`에서 정확한 diagnostics와 case-only RowId negative regression 추가. Exact managed write/readback PASS; latest build/retest NOT RUN.
- 기존 Builder/CCAS Registry/Workbook/Recipe/사용자 Asset/다른 Dirty 보호. stage/commit/push 없음.

### v0.1.18 - 2026-10-15

- USER가 다른 세션에서 수행한 최신 UE 5.8 Editor 빌드 성공 보고를 실제 source 수정 시각, 11:09:36 target receipt 갱신과 11:12 후속 Editor Automation 결과로 교차 확인했다. 직접 빌드 exit code/log는 미회수이므로 USER PASS+artifact corroboration으로 구분.
- 후속 1/1 PASS는 DAUTH MeshCreate 테스트이며 **CF-FQ-059 VehicleSchema focused Automation은 NOT RUN**이다. 빌드 재반복 없이 집중 검증이 다음 Gate임을 명시했다. 새 Production write 없음.

### v0.1.17 - 2026-10-15

- P0-03B actual-source pre-review: existing Recipe Catalog `vehicle_<RecipeId>` projection vs canonical Provider import 구분. ResourcePilot의 실제 Mesh/Socket hard dependency를 research-only readback과 분리해야 한다는 implementation pitfall 확인.
- Existing Compiler의 ImportExisting↔BuildCurrentSnapshot same-state fingerprint 및 canonical SHA-256 diff 계약, VehicleBuilder owner/non-writer, shared mesh identity에 대한 focused test 기준 확정.
- P0-03A latest correction build/Automation NOT RUN; P0-03B 코드/Product/Workbook/Engine/UE Asset mutation0. 다음 Gate 변경 없음.
- 작업 중 새로 발견된 VehicleAuthoringService/Builder/AIContract/UI 등 exact7 파일 Dirty의 읽기 전용 diff를 확인했다. 다른 작업 범위의 `bReferencedByExistingVehicleData` 및 기존 메시의 Candidate 재노출 변경이므로 보호했고, §13.13의 과거 후보 제외 전제를 Current dirty 상태와 분리하여 정정했다. 빌드/회귀 수락은 미확인.

### v0.1.16 - 2026-10-15

- P0-03A provider source audit에서 `ReferenceVehicleId`·`FactKey`가 없는 Claim을 허용하던 semantic omission 발견, provider v1.0.3 및 synthetic focused tests v1.0.2로 보강.
- 현재 UE MCP `ERR_UE_MCP_UNAVAILABLE`, 이전 process-run repository 선택 차단으로 Automation 미실시. 기존 USER 빌드 PASS는 해당 교정 **이전 소스**에만 적용되며 최신 교정 빌드/테스트는 NOT RUN으로 정확히 분리.
- 별도 Tool/Provider/Writer/Vehicle Asset/Workbook 변경 없이 P0-03A 범위 유지, 사용자 Dirty 및 기존 Catalog 보존.

### v0.1.15 - 2026-10-15

- USER 반복 저작 요구를 현재 CF-FQ-059 코드/계획과 대조해 §13.13 신규 수락 기준으로 고정. Core9 first wave는 제품 종류 상한이 아니며 새로운 차량·동일 메시 공유·기존 메시 교체·반복 no-op을 별도 시스템 없이 지원하도록 정의.
- 실제 현재 구현에서 Vehicle/Recipe와 ChassisMesh 분리, 신규 차량의 명시적 StaticMesh Picker 재사용, Mesh-only Candidate의 '미사용 차체' 표시 정책과 R2 생성 허용 정책 구분 확인.
- 차량별 하드코딩 및 병렬 writer 추가 금지, Existing Apply/Provider/Workbook/Catalog 및 승인·stale guard 재사용을 반복 생산 품질 기준으로 강화. P0-03A Build USER PASS 및 Automation 미실시, 신규 Vehicle Product 아직 생성 안 됨은 유지.

### v0.1.14 - 2026-10-15

- USER가 직접 수행한 공식 UE 5.8 Editor 소스 빌드 성공을 USER-REPORTED PASS로 접수. AI의 빌드 로그 독립 검증은 없음.
- 다음 단계는 빌드 재수행이 아니라 focused P0-03A Automation이며, 관리형 process repository 미등록 및 UE MCP status timeout으로 실행하지 못했음을 정확히 기록.
- §14 과거 Research-only checkpoint를 현재 P0-03A code-written / USER-build-PASS / Automation-NOT-RUN 상태로 정합화. 기존 소켓 준비 Core9 검증 반영, UE Product/Workbook mutation 없음.

### v0.1.13 - 2026-10-15

- USER 사전 준비한 Core6 차체 소켓 6/6과 기존 3개 차량 3/3의 소켓 정합성 readback을 반영해 후속 물리 의존성을 상태 구분했다.
- P0-03A 신규 Vehicle schema/read-only Provider와 집중 Automation 소스를 작성했다. FACT citation 연결, Unknown 구분, VehicleBuilder ownership 및 mutation fail-closed.
- 공식 빌드 관리형 Project capability 미등록/Legacy preset identity 불일치로 공식 Build/Automation **BLOCKED/NOT RUN**; Provider Product import/AI R2/Workbook/6개 실제 VehicleData authoring **NOT OPENED**.
- 기존 사용자 StaticMesh 6종 dirty, 기존 시스템/Workbook/공유 Product Assets 보존; stage/commit/push 없음.

### v0.1.12 - 2026-10-13

- P0-02 official source cross-review: `VehicleReferenceSourceReview.md v0.1.0`에서 Core6 실차 외형치수/축거 총24개 FACT 출처 PASS, 전체 source identity/claim/Unknown acceptance는 별개 HOLD로 판정했다. 원본 실차와 임시 Body geometry/Socket를 혼동하지 않는다.
- P0-03 Pre-Implementation actual Source contract review: `VehicleProviderPreReview.md v0.1.0`. 실제 `ECFContentFieldOwnership` enum vs Plan conceptual labels 차이, UI-less AI R2 caller admission/approval boundary 등 exact2 구현 필수 교정 지점을 확인했다. Lowercase `vehicle` + post-R2 RecipeId key와 Source/Apply ownership 분리 유지.
- P0-03 code/build/Automation/UE asset/Workbook mutation **NOT RUN**; Plan/UDS documentation only, existing Dirty protected; no stage/commit/push.

### v0.1.11 - 2026-10-13

- USER 결정: 기존 Core6 차체는 **임시 메시**이며 이후 USER가 교체·크기·Socket 작업한다. 메시에 맞추어 실차 대표 모델을 다시 고르거나 현재 AABB/Socket0을 ReferenceSpec 및 schema 준비의 blocker로 삼지 않는다.
- P0-01 Resource/Category Roster handoff accepted for reference research; I-02 physical and I-03 temporary silhouette tests USER-DEFERRED to real replacement asset integration, but not waived for Product/Runtime safety. **P0-02 reference research ACTIVE**; `VehicleReferenceEvidenceDraft.md v0.1.0`에 6종 Manufacturer FACT and Unknown map 작성. P0-03 implementation and actual Production/Workbook mutations not started.
- P0-00 v0.1.3 Design PASS, existing direct writer and approval boundaries preserved; unrelated Dirty protected.

### v0.1.10 - 2026-10-13

- USER 의도에 맞춰 Core6 actual mesh/socket 작업에 선행하는 공식 실차 대표 모델/크기 기준 선정 단계를 P0-01에 포함했다. `VehicleRepresentativeSpecRoster.md v0.1.0`에는 **6개 차종의 제조사 공식 외형 4종 수치 24개 verified**, 년식·시장·트림, 기초 Engine/Mass/Wheel Reference와 local UE AABB 비교/분리 provenance를 기록했다.
- **대표 실차와 stylized Core6 mesh 간 correspondence는 USER ACCEPTANCE PENDING**. 제조사 제원 verified와 메시 매칭/실제 크기 적용 승인/Production Authoring을 혼동하지 않는다. Unreal Mesh 편집 전 USER 확인/축·단위·외형 검수를 의무화하고 I-01 PASS, I-02/I-03 HOLD 및 P0-02 NOT OPENED 유지.
- No UE Asset/Production DataAsset/Workbook/Source changes, no stage/commit/push.

### v0.1.9 - 2026-10-13

- USER UE 연결 복구 사실 확인 후 `ue.status=connected`, 실제 CarFight Core9 AssetRegistry StaticMesh exact9/9, local Body Bounds exact9/9, fresh managed AssetDump 21/21 Socket readback 수행. Current Wagon Socket7(HP3), SUV6, Sedan7 및 Core6 socket0을 확정했다. Old AssetDump class2/Wagon socket4/HP0은 Historical로만 보존한다.
- I-01 Core9 class coverage PASS; I-02 six USER physical MeshPreparationRequired 및 remaining wheel/visual bounds HOLD; I-03 silhouette/category dedupe HOLD. VehicleMeshInventory v0.1.5 및 USER checklist v0.1.1 연동. No Production/Workbook/Source/UE asset mutation, no Git commit/push.

### v0.1.8 - 2026-10-13

- Core9 package managed Git 및 stored AssetDump Wagon/SUV Socket 재조회, `DA_Recipe_Wagon`/`DA_Vehicle_Wagon`/`DA_Ref_Wagon`/Driving receipt 재검증으로 Wagon-first workflow evidence를 수집했다. 초기 Builder E2E 역사적 Socket0 관찰과 이후 Wagon socket4 snapshot 차이를 확인해 Core6의 과거 누락을 current 결함으로 오판하지 않도록 했다.
- `VehicleMeshPreparationChecklist.md v0.1.0`을 USER용 차량별 준비 명부로 생성. `VehicleMeshInventory.md v0.1.4`와 연결. UE MCP `ERR_UE_MCP_TIMEOUT`으로 live class/bounds/socket 조사 I-01~I-03은 HOLD. P0-02 NOT OPENED.
- No UE Mesh/Production DataAsset/Workbook/C++/BP/Map mutation, no Git stage/commit/push.

### v0.1.7 - 2026-10-13

- USER의 Wagon-first reference 정책과 **Core8 실제 StaticMesh/Socket authoring은 USER 담당**이라는 제작 역할을 13.6절에 반영했다. GPT는 Wagon 사례 분석·차량별 메시 준비 가이드·fresh measurement re-review·후속 reference/typed data 제작/기술검증을 담당한다.
- Wagon의 Wheel Anchor/Shared Wheel/Recipe/Builder Evidence/Driving receipts는 재사용 절차의 선례이지 다른 차량으로 복사할 치수·튜닝 값이 아니며, Wagon의 미검증 상세도 숨기지 않는다.
- 각 Core 차량별 `MeshReady/MeshPreparationRequired/Unverified` 분류를 신설하고 준비되지 않은 개별 차량이 다른 차량 read-only 조사를 막지 않도록 구분했다. 기존 Core9 ONLY, P0-01 I-01~I-03 HOLD, P0-00 Design PASS, Production/Workbook/Source/UE Asset mutation0을 유지한다.

### v0.1.6 - 2026-10-13

- USER가 CF-FQ-059 차체 선정 범위를 **CarFight Core9 ONLY** (`/Game/CarFight/Vehicles/Meshes`)로 최종 축소했다. 기존 Police 제외에 더해 `/Game/Vehicles/OffroadCar` 및 `/Game/Vehicles/SportsCar` 등 외부 template/sample 차량 차체도 제외한다.
- Current selection candidate body paths exact9, stored AssetDump class-verified body exact2(SUV/Wagon), unverified core body exact7. Historical discovery exact37/차체3, intermediate candidate11/12는 원본 보존하면서 Current scope로 오인되지 않게 분리했다.
- Inventory v0.1.2 및 UDS Current를 동일하게 정렬. I-01~I-03 P0-01 GATE HOLD와 P0-00 Design PASS를 유지하고 Production/Workbook/Source/UE Asset mutation 없음.

### v0.1.5 - 2026-10-12

- USER의 최신 범위 결정으로 Legacy Police Car를 CF-FQ-059 대표 차량 선정/제작/복구 대상에서 제외했다. 경찰차 Asset 자체는 변경하지 않았으며 종전 AssetDump 증거는 Historical로 유지한다.
- 현재 선정 후보 tracked body-named exact11(core9 + optional templates2), 현재 범위에서 AssetDump verified Body exact2(SUV/Wagon), class 미검증 후보 exact9로 재분류했다. Historical source-index exact37/차체3은 원본 수치 그대로 둔다.
- `VehicleMeshInventory.md v0.1.1`과 범위 정합화. P0-01 I-01~I-03 HOLD / P0-02 NOT OPENED / P0-00 DESIGN PASS 유지, Product/Source/UE Asset/Workbook mutation 없음.

### v0.1.4 - 2026-10-12

- VCCAS-P0-01 tracked repository + stored AssetDump + existing Recipe/VehicleData cross-inventory를 수행하고 상세 `VehicleMeshInventory.md v0.1.0`을 생성했다.
- Core body-named tracked package exact9, Legacy1, template2와 indexed StaticMesh verified body exact3의 차이를 증거 등급별로 기록했다. SUV/Wagon socket anchor geometry와 Sedan authored-DA layout을 구분하고 모든 Body/Wheel bounds 미검증 상태를 명시했다.
- Unreal Editor MCP unavailable / managed dataset refresh failure로 인해 I-01~I-03 fresh evidence gate HOLD. P0-01 및 P0-02 PASS/착수 선언 금지. P0-00 Design PASS 보존.
- Production Asset/Workbook/Source/Blueprint/Build/Automation/PIE mutation 0, stage/commit/push 없음.

### v0.1.3 - 2026-10-12

- v0.1.2 초안의 Fresh Re-review에서 발견한 `RecipeId` 발급↔ContentKey final Review 순환을 R2 approved unsaved candidate → actual RecipeId readback → 최종 CCAS Review 순서로 교정했다.
- CF-FQ-058 one-shot production cutover entry가 제거된 Current 상태를 반영해 generic contract 재사용과 신규 Vehicle-domain onboarding 실행 경계를 분리했다.
- P1 교정·재검수 판정은 DESIGN PASS 유지. 구현/Production mutation/Build/Automation 실행 없음.

### v0.1.2 - 2026-10-12

- v0.1.1 Design Review HOLD의 blocking P1 exact6을 C1~C6 계약으로 교정하고 affected Source/Systems와 재교차 검토했다.
- Vehicle Provider field-path ownership, existing R0~R3 typed UI-less AI admission, multi-package persistence/recovery, Reference/Mesh provenance, existing single RuntimeTestCatalog membership, canonical Workbook/Recipe identity/drift 경계를 동결했다.
- **VCCAS-P0-00 DESIGN PASS / CONTRACT FROZEN / P0 0 / blocking P1 0 / P2 2 non-blocking**. exact next는 P0-01 persisted Vehicle Mesh Inventory이며 아직 미착수다.
- Source/UE Asset/Workbook/Build/Automation/PIE mutation 0; Git stage/commit/push 없음.

### v0.1.1 - 2026-10-12

- Current Source/Systems/UDS/AssetDump 기반 VCCAS-P0-00 Fresh Rebaseline과 Pre-Implementation Design Review를 수행했다.
- `P0 0 / blocking P1 6 / P2 2 / DESIGN REVIEW HOLD`를 기록하고, 기존 Vehicle Writer·Evidence·RuntimeTestCatalog·CCAS 공통 Core의 authority를 분리했다.
- UI-less GPT authoring admission, Vehicle typed provider/field ownership, durable multi-asset materialization, reference provenance, single Vehicle discovery, Workbook/Builder cross-authority drift를 correction prerequisite로 확정했다.
- Source/UE Asset/Workbook/Build/Automation/PIE mutation은 수행하지 않았다.

### v0.1.0 - 2026-10-12

- USER 요청으로 `CF-FQ-059 Vehicle CCAS Domain Onboarding / 차량 콘텐츠 생산 저작 체계`를 정식 계획으로 승격했다.
- 1단계 목표를 현재 Project의 차량 Mesh를 차종별로 분류하고 각 차종 대표 Mesh exact1에 실제 대표 차량의 공개 스펙을 최대한 적용한 Production Vehicle을 생성하는 것으로 고정했다.
- GPT를 primary author로, Workbook은 내부 canonical/tabular representation으로, Vehicle Builder는 필수 Wizard가 아닌 optional frontend로 정의했다.
- 실차/메시/파생/게임조정 값을 `ReferenceSpec / MeshMeasured / Derived / CarFightTuned` provenance로 분리했다.
- VCCAS-P0-00~08 Gate를 정의하고 exact next를 Fresh Rebaseline + Pre-Implementation Design Review로 고정했다.
- 이번 승격에서는 Source/UE Asset/Build/Automation/PIE mutation을 수행하지 않는다.
