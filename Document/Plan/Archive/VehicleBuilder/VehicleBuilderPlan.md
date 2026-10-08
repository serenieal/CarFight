# CarFight Guided Vehicle Builder Plan

- 문서 버전: v0.1.47
- 작성일: 2026-09-02
- 문서 상태: Historical + Archived Path / CF-FQ-040 Done / VB-P0-10 Current System Promotion Complete / VB-P0-09 USER Acceptance PASS / ESH Final Audit Clean PASS
- Feature: `CF-FQ-040 Guided Vehicle Builder`
- Current owner: `Document/Systems/Vehicles/VehicleBuilder.md v1.0.0`
- Historical placement: `Document/Plan/Archive/VehicleBuilder/` Archived Path / G5 physical move complete
- 하위 설계: `WheelSizeAuthorityPlan.md v0.1.19`
- 대상 프로젝트: CarFight
- 대상 엔진: Unreal Engine 5.8 Source Build
- 역할: 실존 차량 Reference 기반 AI Authoring과 단계형 차량 조립 가이드를 결합해, 사용자가 복잡한 VehicleData 구조를 직접 이해하지 않아도 차량 한 대를 완성할 수 있게 하는 상위 제작 Workflow

---

## 1. 목표

Vehicle Builder의 목표는 `VehicleData`의 많은 필드와 여러 제작 절차를 한 화면에 펼쳐놓는 것이 아니다.

정상 사용자는 다음 흐름만 이해하면 된다.

```text
만들 차량 결정
→ AI가 실존 차량 Reference 조사
→ 사용자가 준비한 Mesh와 Socket 점검
→ 기존 Layout/Hardpoint capture 사용
→ AI가 Reference를 VehicleData 기술 수치로 변환한 Proposal 작성
→ Diff / Validation / Review
→ 명시적 Apply
→ Runtime Technical Test
→ USER 최종 주행감 확인
```

핵심 성공 조건:

> 사용자가 Chaos Vehicle 세부 수치를 외우거나 차량 제작 절차를 기억하지 않아도, Builder가 현재 단계에 필요한 준비물·네이밍·검사 기준만 안내하고 AI가 조사·계산·Authoring을 담당한다.

---

## 2. 기존 시스템과의 관계

### 2.1 VehicleData

`UCFVehicleData`는 계속 차량 Runtime의 최종 구성 Authority다.

현재 실제 흐름:

```text
VehicleData
→ ACFVehiclePawn::ApplyVehicleDataConfig()
→ Movement / WheelPhysics / WheelVisual / Layout / Hardpoint / DriveState / Defense / Fitting
→ Chaos + CarFight Runtime
```

Vehicle Builder는 새로운 Runtime 차량 정의 체계를 만들지 않는다.

### 2.2 CF-FQ-038 Data Authoring

기존 Data Authoring은 폐기하지 않는다.

역할을 다음처럼 고정한다.

```text
Vehicle Builder
= 사용자 정상 제작 UX / 단계형 Workflow

Data Authoring
= Recipe / Resolver / SourceTrace / Diff / Validation / Apply / Undo / Drift의 Backend 및 Advanced 유지보수 화면

VehicleData
= Runtime 최종 결과
```

Builder가 Target `UCFVehicleData`를 임의 raw property write하는 별도 Writer를 만들지 않는다.
가능한 변경은 기존 Data Authoring의 Registry / Resolver / Validation / Apply 경계를 재사용한다.

### 2.3 CF-FQ-034 Fitting

`BaseVehicleMassKg`와 장비·탄약·방어 질량은 Fitting Snapshot의 `TotalVehicleMassKg`로 합산될 수 있으며, 현재 초기 물리 생성 전에 Chaos Movement Mass에 적용되는 경로가 존재한다.

Builder는 기본 차량 질량과 최대 허용 총중량을 Reference 기반으로 작성하되, Fitting Runtime 자체를 재구현하지 않는다.

---

## 3. 사용자 / AI / Tool 책임 분리

| 작업 | 기본 책임 |
| --- | --- |
| 어떤 게임 차량을 만들지 결정 | USER |
| 참고할 실존 차종/Segment 방향 결정 | USER + AI |
| 실존 차량 제원 조사 | AI |
| 여러 출처 교차검증·단위 통일 | AI |
| 실차 데이터→CarFight/Chaos 수치 Proposal | AI |
| Chassis/Wheel Mesh 제작·준비 | USER |
| Wheelbase/Track에 맞춘 Mesh 작업 | USER |
| Wheel Anchor Socket 배치 | USER |
| Hardpoint Socket 배치 | USER |
| 필요한 Asset/Socket/ID 네이밍 안내 | Builder |
| 누락·잘못된 이름·유효성 검사 | Builder |
| Socket→Layout/Hardpoint capture | 기존 VehicleData 기능 + Builder |
| Recipe / Resolve / Diff / Validation | Data Authoring Backend |
| Target Apply / Undo | Data Authoring Backend |
| 실제 Runtime 수치·성능 계측 | AI Technical Validation |
| 외형·최종 운전감 판단 | USER |

원칙:

```text
사용자가 이미 쉽고 안정적으로 수행하는 3D 편집은 자동화하지 않는다.
AI는 조사·계산·전문 수치 Authoring처럼 반복 노동과 전문지식 비용이 큰 부분을 담당한다.
Builder는 작업 순서·네이밍·검증을 책임진다.
```

---

## 4. 명시적 Scope Out

P0에서 다음 자동화는 구현하지 않는다.

```text
- Mesh geometry에서 Wheel center 자동 검출
- Wheelbase/Track 자동 추론을 위한 CV/geometry feature detector
- Hardpoint 위치 자동 배치
- Hardpoint용 Mesh surface 분석
- UE Editor 내부 LLM 실행
- UE Editor 내부 Web crawler / 검색 API / API key 관리
- 실차 외형 자동 복제
- VehicleData를 우회하는 raw UObject property writer
- Apply/Save 자동 승인
- 사용자 확인 없는 Save All
```

Builder는 자동 검출 대신 **정확한 준비 기준 + 네이밍 + 존재/범위 검증**을 제공한다.

---

## 5. 실존 차량 Reference Authoring 계약

### 5.1 Vehicle Selection / Reference Set

CarFight의 **기본 제작 경로는 특정 실존 차량 1대를 Primary Reference로 잠그는 방식**이다.

정상 실존차 경로:

```text
USER가 특정 차량 지정
또는
USER가 AI에게 제작하기 좋은 차량 추천 요청
→ AI가 데이터 가용성 조사
→ 정확한 Model Year / Trim / Powertrain / Market 후보를 3~5대 제시
→ USER가 한 대 선택
→ Primary Reference identity lock
→ 필요할 때만 Secondary/TraitOnly Reference 추가
→ Research Evidence
→ Vehicle-specific Proposal
```

단순한 `Sedan`, `Wagon`, `SUV` 같은 장르명만으로 production 차량을 완성하는 것은 기본 경로가 아니다.

실존 차량은 가능한 한 다음 수준까지 identity를 잠근다.

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

같은 모델명이라도 연식·트림·시장·파워트레인이 다르면 서로 다른 Reference identity다. 자료가 없다는 이유로 다른 variant 수치를 조용히 섞지 않는다.

### 5.1.1 AI Vehicle Recommendation Contract

USER가 어떤 차량을 만들지 추천을 요청하면 AI는 단순 인지도나 취향이 아니라 **CarFight Builder에 옮기기 쉬운 정도**를 먼저 조사한다.

추천 후보는 최소 다음 기준으로 평가한다.

```text
Data Availability
- 제조사 공식 제원 존재 여부
- 정확한 연식/트림 식별 가능 여부
- Engine power / torque / RPM band
- Transmission gear count / forward ratios / reverse ratio
- Final drive
- Mass / dimensions / wheelbase / track
- Tire / wheel specification
- 0-100 / top speed 등 성능 자료
- Suspension / steering / brake 자료
- 독립 출처 교차검증 가능성
- 세대/트림 간 데이터 혼동 위험

Builder Confidence
- 핵심 Runtime 수치를 FACT로 직접 채울 수 있는 비율
- DERIVED/GAME_BIAS가 필요한 비율
- variant/source conflict 위험

CarFight Value
- 이미 제작한 차량과 운전감/차급/구동방식이 얼마나 다른가
- 새 Builder contract를 검증하는 데 의미가 있는가
```

추천 출력은 가능한 경우 exact identity와 이유를 함께 제시한다.

```text
2022 BMW M340i xDrive 8AT
Data Availability: High
Builder Confidence: High
CarFight Value: High
Reason: gear ratios/final drive/engine/mass/performance 자료가 풍부하고 기존 차량과 다른 성격을 검증하기 좋음
```

초기 Vehicle Library 확장 단계에서는 **데이터가 풍부하고 variant confusion이 적은 실존 차량을 우선**한다. 자료가 빈약한 차를 먼저 선택해 Builder 결함과 research 결함을 혼동하지 않는다.

### 5.1.2 Fictional Vehicle Fallback

존재하지 않는 차량을 만들 때만 차급/장르 `Vehicle Archetype`이 Primary authority가 된다.

예:

```text
Compact FWD Hatchback
Midsize AWD Wagon
Large Body-on-frame SUV
Light Pickup
Sports Coupe
Performance EV
```

Archetype은 단일 평균값이 아니라 현실적인 **범위/분포**를 가진다.

```text
Midsize AWD Wagon
CurbMass: 1500~1900 kg
Wheelbase: 2700~3000 mm
PeakTorque: 300~500 Nm
AutomaticGearCount: 7~9
0-100: 5.5~9.0 s
```

가상 차량 proposal은 다음 순서로 만든다.

```text
Archetype
+ USER desired character
+ 선택적 실존 Secondary References
→ vehicle-specific Proposal
```

Archetype 평균은 실존 차량의 확인된 FACT를 덮어쓰는 fallback으로 사용하지 않는다.

### 5.1.3 Real Vehicle Unknown Handling

실존 차량에서 필요한 값이 공개되지 않은 경우:

```text
FACT 있음
→ exact FACT 사용

FACT로 계산 가능
→ deterministic DERIVED

Runtime 필수값이 Unknown
→ 해당 차량의 공개 FACT/성능 특성을 입력으로 explicit GAME_BIAS 제안
→ USER Review

차급 평균값
→ 실존 차량 FACT의 대체 authority로 사용 금지
```

예를 들어 고정 ChangeUpRPM이 공개되지 않았다고 해서 단순히 `중형 왜건 평균 RPM`을 넣지 않는다. 해당 차량의 torque band, power band, gear ratios, performance evidence를 이용해 vehicle-specific GAME_BIAS로 제안한다.

### 5.1.4 Branding / Visual Separation

실존 차량 Reference의 기술적 identity와 최종 게임 Asset의 브랜드 표현은 별도 concern이다.

```text
Technical Reference
= 실제 Manufacturer / Model / MY / Trim / Powertrain을 내부 Evidence로 정확히 보존

Game Presentation
= CarFight의 차량명, 엠블럼/배지/브랜드 표기, 외형 표현을 별도로 결정
```

브랜드 표기를 제거하거나 게임용 이름을 사용하더라도 내부 기술 Reference를 익명화해 다른 차량처럼 취급하지 않는다. 외형/IP 판단은 VehicleData 기술 authoring 계약과 분리한다.

### 5.2 Trim / Year / Market Identity

같은 차명도 연식·트림·시장에 따라 수치가 다르므로 Reference fact에는 가능한 경우 다음 identity를 포함한다.

```text
Manufacturer / Model
Generation
Model Year 또는 범위
Trim / Powertrain
Market / Region
```

불명확하면 AI가 임의로 서로 다른 트림 수치를 합치지 않는다.

### 5.3 Source 우선순위

실차 fact 조사 우선순위:

```text
Tier A: 제조사 공식 제원 / Owner Manual / Homologation·Regulatory 자료
Tier B: 신뢰 가능한 자동차 전문 매체의 계측·제원
Tier C: 잘 관리된 자동차 데이터베이스
Tier D: 커뮤니티·비공식 자료 — 보조 evidence만 사용
```

차량 성격에 큰 영향을 주는 중량, Wheelbase, 출력, Torque, 구동방식, Tire/Wheel, Transmission은 가능한 한 독립 출처 2개 이상으로 교차검증한다.

### 5.4 Fact / Derived / Game Bias 분리

모든 수치는 provenance를 구분한다.

```text
FACT
= 실차 공개 자료에서 직접 확인한 수치

DERIVED
= FACT 여러 개와 물리 관계를 이용해 AI가 계산·추정한 수치

GAME_BIAS
= CarFight 플레이 목적을 위해 Reference에서 의도적으로 조정한 수치
```

AI는 `DERIVED` 값을 실차 제조사 공개 수치처럼 표현하지 않는다.

### 5.5 우선 수집할 Reference Fact

가능한 범위에서 다음을 수집한다.

```text
Identity
- 연식 / 세대 / 트림 / 시장

Dimensions / Mass
- Curb Weight
- Gross Vehicle Weight 또는 Payload 관련 수치
- Length / Width / Height
- Wheelbase
- Front / Rear Track

Wheel / Tire
- Tire Size
- Wheel Size

Powertrain
- Drivetrain (FWD/RWD/AWD 등)
- Engine type / displacement
- Maximum Power + RPM / Power Band
- Maximum Torque + RPM / Torque Band
- RPM별 TorqueCurve point가 공개된 경우
- Idle / Redline이 신뢰 가능한 경우
- Transmission type / automatic 여부 / 전진 Gear count
- 전진 Gear Ratios / Reverse Gear Ratio / Final Drive가 공개된 경우
- Upshift / Downshift 조건 또는 RPM이 공개된 경우
- Gear Change Time / Transmission Efficiency가 공개된 경우

Measured Performance
- 0→100 km/h 또는 0→60 mph
- Published Top Speed
- Top Speed가 전자식 limiter인지 여부
- Braking Distance가 신뢰 가능한 계측 자료에 있는 경우
- Turning Circle / Radius

Chassis Context
- Front/Rear Suspension Type
- Steering/Brake 관련 공개 사양
```

모든 항목이 반드시 존재해야 하는 것은 아니다. 누락값은 `Unknown`으로 남긴 뒤 필요한 경우 Derived 추정을 사용한다.

### 5.6 Engine / Shift / Top-Speed 반복 방지 계약

다음 차량부터 Wagon에서 확인한 시행착오를 차량별 수동 디버깅 절차로 반복하지 않는다.

#### 5.6.1 Engine completion

```text
MaxTorque FACT만 있음
+ MaxRPM 있음
+ 공용/BP TorqueCurve 상속
≠ vehicle-specific Engine Complete
```

최소한 Torque band / Power band / MaxPower context를 조사하고, 가능한 경우 실제 TorqueCurve point를 수집한다. 전체 curve FACT가 없으면 `VehicleBuilderProposalSpec`의 provenance 규칙에 따라 DERIVED 또는 explicit GAME_BIAS curve proposal을 만든다.

2026-09-01 구현 checkpoint:

```text
ESH-01
= Technical PASS
= FCFPerformanceProfileData / FCFVehicleMovementConfig vehicle-specific Engine TorqueCurve typed owner
= bUseEngineTorqueCurve opt-in
= Registry 132→134 / Resolver contract 4→5
= runtime Chaos EngineSetup.TorqueCurve materialization
= legacy opt-out compatibility + same-Pawn stale Curve clear + malformed fail-closed
= focused Registry/Resolver/Runtime exact 3/3 PASS

ESH-02 generic
= Technical PASS
= PhysicsDraft schema v3
= EngineCurveReview { EvidenceDirect / EvidenceDerived / BaselineInherited / GameBias }
= EngineCurveProposalHash persistent Builder receipt binding
= Step 5 Preview/Commit + Editor restart resume + Final Review fresh validation
= affected focused exact 4/4 PASS

Actual Wagon ESH-02
= PhysicsDraft v3 mutation0 Preview PASS
= EngineCurveProposalHash 773221966499c6295f2a652a21b270a5
= Blocker 0 / USER warning 1 (Performance.EngineCurveGameBias)
= Product Asset/Profile/Recipe/Target mutation 0 / Save 0
= USER Review Pending
```

Actual Wagon proposal은 원본 dyno Curve FACT가 없으므로 전체 Curve를 FACT/DERIVED로 위장하지 않는다. 350Nm @ 1800–4800rpm torque plateau와 250PS @ 5400–5700rpm power plateau를 FACT anchor로 사용하고, 900rpm low anchor와 6500rpm tail은 `SparseAnchorEngineCurveBias@1` GAME_BIAS로 명시한다.

2026-09-01 USER 승인 후 actual Wagon ESH-02 persistent checkpoint:

```text
USER Engine Curve Review = PASS
EngineCurveProposalHash = 773221966499c6295f2a652a21b270a5
Profile/Receipt persistent commit = PASS
Commit process = 8a5daf3b5b424124a74e6aa8dc5e0c3b
Fresh persisted affected regression = exact 5/5 PASS
Regression process = a6c99a34f2094f27aa65e30fe3356cd2

Fresh Final Review
Warning = 12
Blocker = 0
External Drift = 없음
Target Diff = 2
ResolverRevision = 5
Current TargetHash = e7e9d2d79a349068a366334b7368924b
Prospective TargetHash = 83c69e52522dc72649c32477b5aad220
DiffHash = fcdabd8842b82537ba97247698267ed4
DefinitionApply ProposalHash = 809c5523ce23971793b3a937c2d8c1c2

Diff 1 = VehicleMovementConfig.bUseEngineTorqueCurve False → True
Diff 2 = VehicleMovementConfig.EngineTorqueCurve empty → approved 6-point Curve
```

Profile commit 승인과 Target DefinitionApply 승인은 서로 다른 mutation Gate다. 이후 USER의 별도 DefinitionApply 승인으로 `DA_Vehicle_Wagon`에도 approved 6-point Engine Curve가 적용됐고, fresh persisted evidence에서 `bUseEngineTorqueCurve=true`, Target DefinitionHash `83c69e52522dc72649c32477b5aad220`을 확인했다.

2026-09-01 중간검수에서 발견한 pre-gate ESH-03 조기 production 연결과 nested warning/blocker 미전파는 교정 상태를 유지한다. production `BuildTransmissionDiagnostic()` 기본 호출은 계속 ESH-03 `default-off`이며, explicit ESH-03 diagnostic에서만 WheelTorqueShift를 평가한다. opt-in 경로는 nested warning/blocker parent 전파, positive finite ordered ratio/radius 검증, usable post-shift RPM fail-closed, fixed-common 최대 adjacent torque gap 15% 초과 USER Review warning을 포함한다.

```text
Engine Curve Target DefinitionApply = PASS
Persisted Target DefinitionHash = 83c69e52522dc72649c32477b5aad220
Persisted bUseEngineTorqueCurve = true
ESH-03 raw diagnostic = WheelTorqueCrossoverShift@1 / common 6222RPM
ESH-03 focused = 447c54667fbc41369ed260b0382e4437 exact 2/2 PASS
ESH-03 Product threshold = USER Review Pending
Persisted ChangeUpRPM = 4500
```

raw 6222RPM은 vehicle-specific Engine Curve + adjacent gear wheel-torque gap을 최소화한 이론 diagnostic이며 자동 적용값이 아니다. P0 Runtime이 gear별 shift map 대신 단일 `ChangeUpRPM`만 지원하므로 실제 longitudinal benchmark와 함께 GAME_BIAS를 선택한다.

#### 5.6.2 Shift completion

실제 TCU up/down shift schedule FACT가 없을 때 다음 shortcut을 금지한다.

```text
PeakTorqueRPM → ChangeUpRPM
PeakPowerRPM → ChangeUpRPM
TorqueBandStart → ChangeDownRPM
차급 평균 shift RPM 사용
```

대신 vehicle-specific Engine Curve + actual Gear Ratios + Final Drive로 각 gear pair의 wheel-torque crossover와 post-shift RPM을 계산한다. P0의 단일 `ChangeUpRPM`은 이 diagnostic을 바탕으로 USER-reviewed `WheelTorqueCrossoverShift@1` GAME_BIAS로 선택한다.

#### 5.6.3 Top-speed limiter

실차의 전자식 최고속 제한은 Evidence에는 보존하지만 CarFight Runtime에는 적용하지 않는다.

```text
Published top speed with electronic limiter
→ 실차 FACT/context
→ CarFight physical top-speed target 아님
→ FinalRatio 역산 authority 아님
```

CarFight 최고속은 limiter가 제거된 상태에서 engine curve / gearing / wheel radius / aero / driveline loss가 만드는 결과로 평가한다.

#### 5.6.4 Benchmark authority

고속 주행 결과는 테스트 맵과 계측 환경이 충분하다는 증거가 있어야 차량 성능 evidence로 사용한다.

```text
필수 high-speed benchmark context
- dedicated M_VehicleBenchmark 또는 동등한 직선 fixture
- 충분한 forward distance
- start/end/forward-axis authority
- travelled distance / forward progress
- lane or lateral deviation
- vehicle destruction / KillZ / obstacle/collision detection
```

맵 끝, KillZ, 진행축 이탈, 충돌 여부를 확인하지 않은 상태에서 기록된 `PeakSpeed`를 차량 최고속이나 출력 한계로 해석하지 않는다.

`M_VehicleBenchmark` dedicated authority는 production no-fitting lifecycle과 동일한 Legacy Mass를 보존하도록 교정한 뒤 Technical PASS했다. 이전 automation이 `BaseVehicleMassKg=1886`을 BeginPlay 뒤 강제 Reapply하던 결과는 일반 PIE와 다른 fixture이므로 high-speed authority에서 제외한다. current authority는 Configured Mass 1500kg / VehicleMesh Actual Mass 1767.377kg, Floor centerline-derived start, Steering 0, longitudinal-only world-Z yaw-rate lock, obstacle 0 / RoadEnd false를 사용한다.

```text
4500 baseline : T100 6.733s / T150 15.967s / T200 58.367s / Peak 206.793km/h / FinalGear 6
5500 transient: T100 6.100s / T150 13.167s / T200 31.183s / Peak 206.700km/h / FinalGear 6
6222 transient: T100 6.083s / T150 12.850s / T200 31.683s / Peak 216.116km/h / FinalGear 5
5500 persisted: T100 6.100s / T150 13.167s / T200 31.183s / Peak 206.700km/h / FinalGear 6 / Override false
```

6222에서 6단 미진입 자체는 failure가 아니다. 5단에서 더 높은 WOT 최고속이 나온 것은 6단이 overdrive로 작동할 수 있음을 보여준다. P0 단일 fixed-common threshold는 USER Review에서 **5500RPM GAME_BIAS**로 승인됐고 exact Builder Profile/Receipt commit → Final Review → DefinitionApply를 통해 Product에 적용됐다. current persisted Target은 `ChangeUpRPM=5500`, `ChangeDownRPM=2000`, DefinitionHash `0e5b48e8dcd39deba441da9237218be6`이다. ESH-05 no-override retest가 transient 5500과 동일한 T100/T150/T200/Peak/FinalGear를 재현했고 ESH-06 actual USER Driving도 승인돼 ESH-01~06 전체가 완료됐다.

최종감사에서 high-speed runner의 `ExpectedTargetDefinitionHash`가 metadata로만 기록되고 실제 saved Target과 비교되지 않는 P1을 발견했다. Editor-only `CFVehicleHashCheck` commandlet이 existing `FCFVehicleSnapshotBuilder` authority로 current saved VehicleData DefinitionHash를 계산하고, `RunHighSpeedBench.ps1`가 benchmark 시작 전 expected hash와 exact 비교해 mismatch를 fail-closed하도록 교정했다. wrong-hash `00000000000000000000000000000000`는 Exit124로 benchmark 전 차단됐고 current `0e5b...` persisted run은 `target_hash_verified=true`, observed hash exact match와 함께 기존 `206.700km/h / G6 / T200 31.183s`를 재현했다.

기존 `VB-P0-08 TechnicalDrivingBenchmark`는 일반 Step 8 regression fixture owner를 유지하고, ESH-04 dedicated high-speed authority는 고속/shift 후보 비교에 사용한다.

---

## 6. Reference → VehicleData 변환 원칙

실차 Reference는 VehicleData의 단순 1:1 복사가 아니다.

### 6.1 직접 Anchor 가능한 예

```text
Curb Weight
→ BaseVehicleMassKg 기준

Drivetrain
→ DifferentialType / FrontRearSplit 방향

Wheel/Tire Size
→ 기본 Reference/Sanity 기준
→ `bUseWheelSocketScale=true`인 신규 정상 경로에서는 USER Wheel Socket Scale + actual Wheel Mesh Bounds가 Front/Rear WheelRadius/Width의 최종 derived authoring source
→ Reference Tire Size가 USER Socket Scale을 자동 덮어쓰지 않음

Maximum Torque / RPM / Power curve context
→ EngineMaxTorque / EngineMaxRPM / Engine response 기준

Transmission type / gear count / ratios / final drive / shift context
→ Builder-private Drivetrain Profile의 차량별 Transmission proposal 기준
→ 실제 UE 5.8 Chaos `TransmissionSetup` mapping은 VB-P0-04에서 exact Source/API를 확인한 뒤 확정

Wheelbase / Track
→ 사용자 Mesh·Socket 준비와 Layout sanity 기준
```

### 6.2 AI Derived가 필요한 예

일반 소비자 제원에서 직접 얻기 어려운 값:

```text
CorneringStiffness
FrictionForceMultiplier
WheelLoadRatio
SpringRate
SpringPreload
SuspensionMaxRaise / Drop
SteeringAngleRatio
EngineRevUpMOI
EngineRevDownRate
CenterOfMassOverride
Brake Torque
Drag / Downforce의 CarFight용 적용값
```

이 값은 다음을 함께 사용해 산출한다.

```text
실차 Reference Fact
+ 해당 Segment의 범위
+ CarFight 기존 VehicleData baseline
+ Chaos Vehicle parameter 의미
+ Technical Driving Test 결과
```

중요:

> Reference fact가 없는 Chaos 파라미터를 임의의 "실차 정확값"으로 만들지 않는다. Derived estimate로 명시하고 Runtime benchmark에서 검증한다.

### 6.3 Driving Feel 4축의 역할

기존 Acceleration / Steering / Grip / Suspension 4축은 제거하지 않는다.

새 역할:

```text
Reference 기반 차량 생성의 원천 데이터 X
AI Derived 결과를 간단히 보정하는 semantic bias O
기존 차량 Quick Tune O
USER가 원할 때 사용하는 선택 UI O
```

즉 기본 차량은 실제 Reference에서 만들고, 4축은 이후 게임성 보정에 사용한다.

---

## 7. Guided Workflow

정상 경로는 한 번에 많은 설정을 노출하지 않는다.
각 Step은 **현재 해야 할 일 / 준비물 / 검사 결과 / 다음**만 보여준다.
Advanced Raw 값은 기본 접힘 상태다.

### STEP 1 — 차량 Identity & Reference

USER 입력:

```text
게임 차량 이름
차량 Segment / 용도
특정 실존 Reference 희망 여부
```

AI 작업:

```text
Reference 후보 조사
연식/트림/시장 정규화
주요 Fact + 출처 + confidence 정리
Reference Set 제안
```

USER Gate:

```text
이 Reference Set으로 진행
또는 후보 변경
```

### STEP 2 — Mesh 준비

Builder가 요구한다.

```text
Chassis Mesh
공용 Default Wheel Mesh
전방 +X
실차 Reference 기준 전장/전폭/전고/Wheelbase/Track 안내
Wheel Mesh Bounds X/Y/Z와 canonical radial X/Z 100 cm 규격
```

자동 Wheel detection은 하지 않는다.
USER가 Reference 크기에 맞춰 Mesh를 직접 준비한다.

신규 정상 경로의 Wheel Size 상세 계약은 `WheelSizeAuthorityPlan.md`가 소유한다.
Builder는 persisted Wheel Mesh Bounds를 읽어 실제 원본 크기를 보여주되, 휠하우스에 어울리는 타이어 Scale을 자동 결정하지 않는다.
공용 Default Wheel의 X/Z radial Bounds는 100 cm canonical target으로 one-time normalization한 뒤 차량별 Socket Scale authoring을 시작한다.

### STEP 3 — Socket / Naming Guide

현재 Project Contract를 그대로 안내한다.

Wheel 기본 Socket:

```text
Wheel_Anchor_FL
Wheel_Anchor_FR
Wheel_Anchor_RL
Wheel_Anchor_RR
```

Hardpoint 예:

```text
LocationSlotId: Front_01 / Top_01
SocketName: HP_Front_01 / HP_Top_01
LocationCategory: Front / Back / LeftSide / RightSide / Top
```

Destroyed FX 기본 Socket:

```text
FX_Destroyed
```

Builder가 해야 할 일:

```text
- 선택한 차량 기능에 필요한 ID/Socket 목록 제시
- 각 이름 복사 가능
- 위치와 +X 전방 방향 의미 설명
- Socket 존재 여부 검사
- 잘못된 이름/중복 ID 표시
```

USER가 해야 할 일:

```text
- Mesh Editor에서 Socket 직접 생성·배치
- Wheelbase/Track 기준으로 Wheel Socket 직접 배치
- 휠하우스를 보면서 Wheel Socket Scale 직접 조정
  - X/Z = 타이어 직경 배율
  - Y = 타이어 폭 배율
  - X와 Z는 동일값 원칙
- Hardpoint Socket 직접 배치
```

Wheel Socket Scale은 USER 시각 판단 Authority이며 AI/코드가 자동 선택하지 않는다.

### STEP 4 — Layout / Hardpoint Capture

기존 `Capture Vehicle Layout From Chassis Sockets` mutation 경로 자체는 후속 apply flow에서 재사용한다. VB-P0-03의 준비/상태 평가는 이 함수를 호출하지 않고 current AssetSnapshot과 persisted layout readback만 사용한다.

Builder는 결과를 다음처럼 요약한다.

```text
Wheel Socket 4/4
Wheel Socket Location / Rotation / Scale
Hardpoint Socket N/N
Layout Capture PASS/FAIL
Reference Wheelbase와 authored Layout 차이
Resolved Wheel Mesh Bounds
Socket-derived Front/Rear Radius / Width
누락 / fallback / stale 여부
```

`bUseWheelSocketScale=true`이면 Layout Capture는 Scale도 보존하고, Wheel Mesh Bounds + USER Socket Scale에서 Front/Rear WheelRadius/Width를 derived value로 만든다. 이 네 물리값은 Step 5 AI independent proposal이 덮어쓰지 않는다. 상세 계산/검증 계약은 `WheelSizeAuthorityPlan.md`를 따른다.

### STEP 5 — Reference Physics Proposal

AI가 Reference fact를 VehicleData용 기술 Proposal로 변환한다.

USER 기본 화면에는 Chaos raw table을 먼저 보여주지 않는다.

기본 요약 예:

```text
기준 질량
구동방식
엔진/출력 성격
변속기 / 기어비 / 변속 성격
Wheel/Tire 기준
조향 성격
서스펜션 성격
예상 성능 범위
```

`상세 수치`를 열면 다음 provenance를 표시한다.

```text
Field
Current
Proposed
Source = FACT / DERIVED / GAME_BIAS
근거 요약
Confidence
```

### STEP 6 — Game Runtime 기본 구성

현재 VehicleData가 필요로 하는 게임 시스템 연결을 단계적으로 안내한다.

```text
Durability
DefaultDefenseData
Destroyed FX
Hardpoint / Mount Profile
Fitting Mass 범위
DriveState
필요한 기본 Reference Asset
```

프로젝트 기본값을 사용할 수 있는 항목은 기본값을 제안하되 자동 Apply/Save하지 않는다.

### STEP 7 — Final Review / Apply

기존 Data Authoring Backend를 사용한다.

```text
Resolve
Diff
Source Trace
Validation
External Drift check
Apply Review
Explicit Apply
Undo
```

정상 화면에서는 다음만 우선 노출한다.

```text
Blocked N
Warning N
Reference와 직접 일치하는 값 N
AI Derived 값 N
Game Bias 값 N
Target 변경 예정 N
자동 저장: 안 함
```

### STEP 8 — Technical Driving Test

AI가 가능한 기술 지표를 먼저 검증한다.

우선 후보:

```text
실제 Runtime Mass
0→50 / 0→100 km/h
Top Speed
100→0 km/h Braking Distance
Turning Radius
일정 속도 조향 시 Yaw Rate / lateral response
Slip 관련 관측 가능값
Suspension travel / settling 관측 가능값
```

Reference 자료에 실제 계측값이 있다면 목표 Range와 비교한다.

최종 USER Gate는 숫자 자체가 아니라 실제 운전감·게임 적합성이다.

---

## 8. UX 계약

정상 경로:

```text
Back   [현재 Step]   Next
```

원칙:

```text
- 한 Step에서 주 결정을 1개 수준으로 제한
- 전체 Raw VehicleData grid를 기본 화면으로 사용하지 않음
- 필수 작업이 아닌 버튼을 여러 개 병렬 노출하지 않음
- 다음 단계로 갈 수 없으면 이유와 정확한 해결 작업을 한글로 표시
- USER가 직접 Editor에서 해야 하는 작업은 "무엇을 / 어디에 / 이름 / 기준"을 모두 제공
- Advanced는 선택적으로만 펼침
- Step state는 재개 가능해야 함
- Apply와 Save는 분리
- 자동 Save All 금지
```

Builder가 질문해야 하는 것은 사람이 이해할 수 있는 선택이다.

좋은 예:

```text
"이 차량은 도심형 경차 Reference로 진행할까요, 고성능 경차 쪽으로 진행할까요?"
```

피해야 할 예:

```text
"FrontWheelCorneringStiffness를 얼마로 할까요?"
```

---

## 9. AI 연결 아키텍처

P0에서는 Unreal Editor에 LLM/Web Client를 내장하지 않는다.

```text
ChatGPT / AI Authoring Session
→ Web Research
→ Reference facts + citations + derived proposal
→ CarFight typed Builder/Authoring ingress
→ Builder Review UI
→ Data Authoring Backend
→ VehicleData
```

장점:

```text
- UE에 API key/network stack 불필요
- Web 검색/출처 정책을 게임 코드와 분리
- AI 모델 교체와 Editor 기능을 분리
- Builder는 deterministic validation/review에 집중
```

AI mutation은 자유로운 raw property patch가 아니라 **review 가능한 typed proposal**이어야 한다.

현재 `CFVehicleAIContract`는 raw SetField와 AI Shared Profile writer를 금지하고 있으므로, VB-P0-00에서 Builder 전용 ingress를 다음 원칙으로 설계한다.

```text
- Target VehicleData direct writer 추가 금지
- Preview-before-commit
- exact target / recipe fingerprint binding
- field allowlist / semantic payload
- FACT / DERIVED / GAME_BIAS provenance 포함
- shared Profile을 AI가 몰래 변경하지 않음
- Builder가 소유하는 vehicle-specific authoring record/profile과 shared Profile을 구분
- no auto save
- Undo 가능
```

VB-P0-00 감사 결과 새 GoPyMCP top-level public tool은 필요하지 않다. 현재 `ue.call_write` Bridge가 Toolset Registry의 reviewed write operation을 전달할 수 있으므로 외부 transport는 재사용한다. 향후 Builder 구현에서는 generic DataAsset/Object raw mutation을 정상 AI 경로로 사용하지 않고 CarFight Editor 내부의 typed Builder operation만 추가한다.

---

## 10. C++ / Blueprint / Asset 책임

### C++ Editor Module

권장 책임:

```text
Vehicle Builder step state machine
Builder ViewModel
Reference proposal data types
Naming/checklist generator
Asset/Socket validation
Data Authoring facade integration
Apply review
Automation tests
```

예상 파일명은 실제 구현 전 확정하지만 32자 제한을 유지한다.

예:

```text
CFVehicleBuilderTab.*
CFVehicleBuilderVM.*
CFVehicleRefTypes.*
```

### Runtime C++

기본적으로 변경 최소화.

```text
UCFVehicleData
ACFVehiclePawn runtime apply
Fitting mass
Validator
```

Builder 요구 때문에 Runtime에 중복 authoring 상태를 추가하지 않는다.

### Blueprint / UE Asset

```text
BP_CFVehiclePawn = 공용 런타임 Pawn 구조 유지
차량별 Chassis/Wheel Mesh = USER 준비
StaticMesh Socket = USER 배치
VehicleData / Recipe / 필요한 Profile = Builder/Data Authoring이 생성·연결
```

차량마다 별도 복잡한 Pawn Blueprint를 만드는 방향은 기본값으로 사용하지 않는다.

---

## 11. IP / Reference 사용 경계

이 Plan은 법률 자문을 대신하지 않는다.

제작 원칙은 다음으로 둔다.

```text
- 공개된 사실형 차량 제원과 계측 데이터를 Reference로 사용
- 출처와 모델/연식/트림 identity를 authoring evidence로 기록
- 원문 표/기사 전체를 프로젝트에 복제하지 않음
- 제조사 로고/Badge/브랜드 고유 시각물을 게임 Asset으로 복제하지 않음
- Player-facing 차량명과 디자인은 CarFight 고유 identity 사용
- 특정 실차를 1:1 외형 복제하는 것이 Builder의 목표가 아님
```

---

## 12. 완료 조건

P0 Vehicle Builder 완료는 단순히 탭이 열리는 것으로 판정하지 않는다.

대표 End-to-End Gate:

```text
1. 빈 상태에서 신규 차량 제작 시작
2. AI Reference research proposal 수용
3. USER가 준비한 Mesh 지정
4. Builder가 필요한 Wheel/Hardpoint Socket 이름과 기준 안내
5. USER가 Socket 준비
6. Layout/Hardpoint capture PASS
7. AI Reference-derived physics proposal 생성
8. Diff / Validation / Apply review PASS
9. VehicleData 생성·적용
10. Runtime technical benchmark PASS
11. USER가 실제 주행 후 "차량 하나를 만드는 흐름을 따라갈 수 있었다"고 승인
```

P0 USER Acceptance의 중심 질문:

> "차량 제작 규칙과 Chaos 세부 수치를 외우지 않고도 Builder가 안내하는 순서만 따라 차량 한 대를 끝까지 만들 수 있는가?"

---

## 13. 첫 구현 전 감사 Gate

`VB-P0-00 Current Vehicle Creation Contract Audit`에서 다음을 먼저 닫는다.

```text
- 현재 VehicleData 118 Registry leaf 중 Builder가 다룰 범위
- CreateVehicleRecords가 생성하는 Definition/Recipe exact 경계
- Profile 5-domain 실제 ownership과 vehicle-specific/generated Profile 허용 모델
- 현재 AI contract가 금지하는 write와 Builder에 실제 필요한 typed write
- CaptureLayoutFromChassisSockets current contract
- Hardpoint/Mount/Profile naming current contract
- Fitting/Defense/DriveState 기본값의 안전한 재사용 범위
- 현재 UE Toolset 안에서 Builder typed ingress를 노출할 방법
- Reference evidence의 persisted owner(JSON/Recipe/editor-only asset 등) 결정
```

이 Gate PASS 전에는 새 Runtime schema, 새 AI raw writer, 새 public MCP tool, VehicleData mutation을 하지 않는다.

---

## 14. VB-P0-00 Current Vehicle Creation Contract Audit — PASS

2026-08-26 current Source/Systems read-only 감사로 첫 구현 전 owner를 다음과 같이 확정했다. 구현·Asset 생성·VehicleData mutation·Build는 수행하지 않았다.

### 14.1 Reference evidence persisted owner — 확정

**별도 Editor-only companion DataAsset**을 owner로 사용한다.

향후 구현 클래스의 역할명은 `Vehicle Reference Evidence`이며 실제 클래스/파일명은 VB-P0-01에서 32자 제한 안으로 확정한다.

선택 근거:

```text
Runtime UCFVehicleData
= 최종 실행 데이터이므로 citation/research provenance를 넣지 않음

UCFVehicleRecipeData
= Resolver에 직접 참여하는 persistent authoring intent SSOT
= Recipe fingerprint / Apply approval / drift baseline과 연결됨
= 원문 출처 보강만으로 runtime-effective authoring fingerprint가 불필요하게 흔들리는 구조를 피함

Repository JSON sidecar
= UE Asset identity/rename/reference/validation과 분리돼 정상 Builder workflow owner로 부적합

Editor-only Reference Evidence DataAsset
= Never-Cook 가능
= Reference identity / citations / FACT·DERIVED·GAME_BIAS / confidence를 구조화 가능
= Runtime schema 오염 0
= Builder proposal이 evidence fingerprint를 별도로 binding 가능
```

따라서 Reference Evidence는 VehicleData의 runtime source가 아니라 **AI proposal의 근거 SSOT**다. Resolver는 Evidence Asset을 직접 읽어 임의의 VehicleData 값을 생성하지 않는다. AI가 evidence에서 계산한 reviewed proposal이 typed authoring owner로 들어간 뒤 기존 Resolver가 최종 Definition을 계산한다.

### 14.2 AI vehicle-specific numeric typed commit owner — 확정

AI가 만든 차량별 물리 수치는 **기존 4개 Editor-only Profile class의 차량 전용 Private instance**가 정상 owner다.

```text
UCFVehicleBaseProfile
UCFDrivetrainProfile
UCFHandlingProfile
UCFPerformanceProfile
```

기본 Builder 차량에서는 `UCFDriveStateProfile`을 새로 만들지 않는다. `Recipe.DriveStateMode=ProjectDefault`를 기본으로 유지하며 차량별 DriveState behavior가 실제 필요할 때만 별도 opt-in한다.

현재 118-field Registry와의 정합:

```text
VehicleBase
= Base/Gross Mass, ChassisHeight, WheelVisual baseline, Defense/Fx default source

Drivetrain
= Differential, FrontRearSplit, powered axle, Front/Rear WheelClass
= 차량별 Transmission numeric authoring owner
= Transmission type/automatic 여부, forward gear count/ratios, reverse ratio, final drive, shift condition, gear change time, efficiency의 semantic scope를 소유
= exact typed schema와 UE 5.8 Chaos `TransmissionSetup` mapping은 VB-P0-04에서 확정

Handling
= Steer, Grip, Cornering, Spring, Brake, LoadRatio, Suspension limits, Sweep, SteeringType

Performance
= Torque/RPM/Throttle response, Idle/Redline, EngineBrake, Rev response, Drag/Downforce
```

정상 Builder 경로에서 **기존 shared Profile payload를 AI가 수정하지 않는다.** 새 차량용 Profile 4개는 해당 Recipe 전용 private/generated authoring record로 취급한다.

향후 typed commit 계약은 다음을 강제한다.

```text
- AI caller 허용은 Builder-private Profile만
- Shared Profile payload write 금지 유지
- raw StableFieldPath/UObject SetField API 금지 유지
- Profile Domain별 typed payload만 허용
- exact Owner RecipeId / Target identity / evidence fingerprint binding
- preview-before-commit
- exact proposal hash 승인
- R1 AuthoringRecordWrite
- no auto save
- no automatic retry
- Undo 가능한 단일 authoring transaction
```

현재 Profile metadata에는 private Builder ownership을 증명할 persistent owner marker가 없다. **이 marker와 typed private-profile commit operation의 실제 schema는 VB-P0-04 구현 설계에서 추가해야 할 명시적 gap**이다. 이 gap을 이유로 AdvancedOverride나 Shared Profile writer를 임시 정상 경로로 사용하지 않는다.

현재 Source 기준 추가 gap도 명시한다.

```text
FCFVehicleMovementConfig
= Engine / Differential / Steering 등은 차량별 runtime source를 가짐
= Transmission authoring field 없음

FCFDrivetrainProfileData
= DifferentialType / FrontRearSplit / powered axle / WheelClass만 보유
= Transmission numeric payload 없음

ACFVehiclePawn::ApplyVehicleMovementConfig
= EngineSetup / DifferentialSetup / SteeringSetup 적용
= TransmissionSetup 차량별 적용 없음
```

따라서 **다단 변속기 Authoring은 별도 Feature가 아니라 Guided Vehicle Builder의 Drivetrain Profile 정식 범위에서 보강해야 할 known gap**이다. Engine RPM/Redline/Torque owner는 Performance Profile에 남기고, 변속기 자체의 기어비·Final Drive·변속 조건은 Drivetrain Profile이 소유한다. 변속 조건이 Engine RPM evidence를 입력으로 소비할 수 있어도 owner를 Performance로 옮기지 않는다.

HUD Gear는 기존처럼 실제 Chaos Vehicle runtime의 `GetCurrentGear()`를 읽는 구조를 유지한다. Builder/VehicleData/HUD에 별도 fake gear state나 가짜 기어 계산을 만들지 않는다.

기존 `AdvancedOverrides`는 exceptional/technical escape hatch로만 유지한다. CenterOfMassOverride, Wheel AdditionalOffset처럼 현재 Registry가 Profile owner를 제공하지 않는 기술 필드는 AI가 자동 normal authoring하지 않고 필요 시 별도 reviewed Advanced 단계로 보낸다.

### 14.3 4축 Driving Feel과 Private Profile 관계 — 확정

Reference 기반 기본값을 4축 slider 값으로 역추정하지 않는다.

Builder가 생성한 Handling/Performance Profile에서:

```text
NeutralValue
= Reference + AI Derived 차량 기준값

LowValue / HighValue
= 같은 차량 기준에서 Quick Tune 가능한 안전 범위

Recipe DrivingFeelIntent
= 기본 0.5 / 0.5 / 0.5 / 0.5
```

으로 두는 방향을 사용한다. 따라서 Reference 차량의 canonical baseline과 기존 4축 Quick Tune을 동시에 보존할 수 있다.

### 14.4 Builder 최소 생성 Asset 집합 — 확정: 7개

Builder 정상 신규 차량 1대가 생성해야 하는 **최소 persistent Asset은 7개**다.

```text
1. UCFVehicleData
   - Runtime canonical Definition

2. UCFVehicleRecipeData
   - Editor-only authoring intent / resolver binding

3. Vehicle Reference Evidence DataAsset
   - Editor-only Reference identity / citation / provenance owner

4. UCFVehicleBaseProfile
   - Builder-private vehicle-specific Profile

5. UCFDrivetrainProfile
   - Builder-private vehicle-specific Profile

6. UCFHandlingProfile
   - Builder-private vehicle-specific Profile

7. UCFPerformanceProfile
   - Builder-private vehicle-specific Profile
```

`CreateVehicleRecords` current Source는 Definition+Recipe 2개만 생성하고 5개 Profile을 전부 추론하지 않는 것을 Automation으로 보호한다. Builder 구현 시 이 기존 core를 깨지 않고 **reviewed Builder orchestration이 필요한 private Profile/Evidence 생성 단계를 추가**한다.

### 14.5 USER가 제공하지만 Builder가 생성하지 않는 필수 Reference

```text
Chassis StaticMesh
Wheel StaticMesh
```

Recipe는 4 wheel mesh reference를 가지지만 동일 Wheel Mesh 재사용을 허용할 수 있다. 실제 Definition validator는 Chassis, wheel mesh references, Front/Rear Wheel Class를 required refs로 검사한다.

Wheel Class는 차량마다 새 Blueprint Class를 자동 생성하지 않는다. Drivetrain Profile에서 기존 검증된 Front/Rear Wheel Class를 reference하도록 한다.

공용 `BP_CFVehiclePawn`을 계속 사용하며 차량별 Pawn Blueprint를 최소 생성 집합에 넣지 않는다.

### 14.6 최소 집합에서 제외되는 Optional Asset

다음은 차량 한 대를 생성하기 위한 minimum record가 아니다.

```text
UCFDriveStateProfile
- 기본 ProjectDefault 사용

UCFVehicleFittingData
- Optional. 없으면 VehicleData 기반 legacy/runtime 경로 유지

UCFVehicleDefenseData
- Optional. None이면 기존 직접 내구도 fallback 허용

UCFCombatFxData
- Optional/reusable. Destroyed FX는 explicit/default 정책으로 선택

UCFEquipmentPresetData / Weapon / Ammo / Sensor 관련 Asset
- 장비 기능을 구성할 때 선택적으로 연결

별도 Vehicle Pawn Blueprint
- 생성하지 않음
```

HardpointSlots와 MountProfiles가 비어 있는 것 자체도 current `UCFVDAValidator`에서 오류가 아니다. 따라서 무장이 없는 초기 차량에 가짜 Hardpoint/Equipment Asset을 만들지 않는다.

### 14.7 Validator / Layout / Fitting 재사용 경계 — 확정

```text
Validation
= 기존 UCFVDAValidator 단일 authority 재사용

Wheel/Hardpoint Layout
= Recipe AssetIntent/HardpointIntent + 기존 Chassis Socket AssetDerived resolve 재사용

Fitting
= VehicleData Base/Gross mass를 작성하되 UCFVehicleFittingData 생성은 optional

Definition Apply
= 기존 FCFVehicleAuthoringService → FCFVehicleApplyService 유일 writer 유지
```

Builder 전용 Validator는 만들지 않는다. Builder가 추가로 검사할 수 있는 것은 Step completeness, Reference evidence completeness, Builder-private ownership 같은 workflow 조건뿐이다.

### 14.8 AI transport owner — 확정

현재 GoPyMCP UE surface에 reviewed `ue.call_write` Bridge가 존재하고 Toolset Registry operation을 전달할 수 있으므로 **새 top-level public MCP tool은 추가하지 않는다.**

향후 구조:

```text
ChatGPT
→ existing GoPyMCP ue.call_write transport
→ CarFight Editor typed Builder operation
→ preview / explicit approval
→ Builder-private Profile + Recipe authoring transaction
→ existing Resolver / Validation / Apply
```

Generic `DataAssetTools` / `ObjectTools`의 raw property mutation은 기술적으로 UE Toolset에 존재하더라도 Builder AI normal write route로 사용하지 않는다.

### 14.9 VB-P0-00 판정

**PASS**

세 owner와 transport/validation 경계가 current Source/Systems와 모순 없이 고정됐다.

다음 Gate:

```text
VB-P0-01 Reference Research & Evidence Contract
```

VB-P0-01에서는 Reference Evidence Asset에 저장할 exact 구조와 research normalization/citation/conflict 규칙을 설계한다. 구현은 해당 설계가 닫힌 뒤 진행한다.

---

## 15. VB-P0-01 Reference Research & Evidence Contract — PASS

상세 schema와 dry-run evidence, additive Transmission research vocabulary는 `VehicleRefEvidenceSpec.md v0.1.1`이 소유한다.

이번 Gate에서 다음 계약을 고정했다.

```text
Reference identity
= ModelYear / Market / Powertrain / Trim / Wheel-Tire Variant scope

Citation
= Source Tier + canonical URL/locator + origin independence 분리

Atomic research value
= FACT / DERIVED / GAME_BIAS + Unknown 분리

Conflict
= identity/variant mismatch는 평균하지 않고 split
= material unresolved conflict는 proposal mapping block

Confidence
= evidence support confidence
= game quality/feel score가 아님

Evidence fingerprint
= SHA-256 / CFVREF-1 / CFVRN-1
= semantic research payload + target binding canonicalization

AI proposal binding
= EvidenceFingerprint
+ existing RecipeFingerprint
+ TargetDefinitionHash
+ private ProfileFingerprint 4종
+ ProposalHash / explicit approval
```

실제 `The 2027 Kia Morning` 1.0 gasoline / Trendy / 4AT / 175/65R14 Reference Set으로 research normalization을 dry-run했다. 14-inch 975 kg과 16-inch 1,010 kg은 source conflict 평균이 아니라 `VariantScopeMismatch → SplitVariant`로 분리했고, 14-inch identity의 975 kg만 canonical FACT로 선택했다. 공개 근거를 확정하지 못한 GVW, gear ratios, final drive, redline, 0-100, top speed 등은 `UnknownFacts`로 남겼다.

단순 unit conversion은 FACT provenance를 유지하고, tire size로 계산한 nominal outer radius 같은 새로운 quantity만 DERIVED로 분리했다. Research 단계의 GAME_BIAS는 0건이다.

Synthetic canonical payload로 array order independence와 value/target/conflict change sensitivity를 확인했으며, Preview 뒤 Evidence나 Recipe/Target/private Profile 상태가 달라지면 old proposal을 fail-closed하는 additive stale binding이 기존 Authoring approval contract와 양립함을 확인했다.

이번 Gate의 구현 mutation:

```text
C++ 0
UE Asset 0
VehicleData/Profile/Recipe write 0
Editor/Build/Automation 0
```

다음 Gate:

```text
VB-P0-02 Builder Shell / Step State / Resume
```

Reference → CarFight/Chaos numeric mapping과 Builder-private Profile typed commit은 `VB-P0-04` owner로 유지한다.

---

## 16. VB-P0-02 Builder Shell / Step State / Resume — PASS

상세 Shell/State/Resume 계약과 additive Transmission dependency boundary는 `VehicleBuilderShellSpec.md v0.1.1`이 소유한다.

이번 Gate에서 다음을 고정했다.

```text
실제 completion truth
= Recipe / Evidence / Target / Profile / Mesh / Socket / ApplyState에서 fresh derive
= persistent Step Complete boolean 저장 금지

Editor restart resume convenience
= local Editor per-project user settings
= managed primary identity는 RecipeId
= RecipePath / DefinitionPath는 fallback·diagnostic hint

Transient Builder state
= current context / cache / prepared approval / Undo identity
= Editor restart 뒤 전부 폐기 후 fresh read

Step state
= Unavailable / Locked / Ready / Complete / Blocked / Stale

Navigation
= current P0 baseline은 8 stable StepId
= Step 개수/순서/표시 번호는 definition-driven presentation
= Next는 current Step fresh read 후에만 진행
= distant future Step direct jump 금지

Reference review resume token
= EvidenceId + EvidenceFingerprint + RecipeId
= mutation/apply/save approval 권한 없음
```

현재 P0 baseline의 stable StepId는 `IdentityReference / MeshPrep / SocketGuide / LayoutCapture / PhysicsProposal / GameplaySetup / FinalReview / DrivingTest`다. **StepId는 semantic identity로 안정적으로 유지하지만 8이라는 개수와 표시 순서는 고정 authority가 아니다.** current composition/order/title은 단일 Step Definition 목록이 소유하고, 후속 Step은 generic plugin registry가 아니라 명시적 StepId + 전용 evaluator/content로 추가한다.

Editor restart, Recipe rename/move, Evidence 변경, Target external edit, duplicate RecipeId, prepared Apply 폐기, local settings 삭제 시나리오를 설계 dry-run했고 모두 fail-closed 또는 recoverable하게 닫혔다.

이번 Gate의 구현 mutation:

```text
C++ 0
UE Asset 0
Recipe/Evidence schema mutation 0
VehicleData/Profile mutation 0
Editor/Build/Automation 0
```

VB-P0-03 결과:

```text
MeshPrep / SocketGuide / LayoutCapture read-only evaluator contract PASS
FCFVehicleAssetReader / FCFVehicleAssetSnapshot authority 재사용
Wheel role 4/4 existing + distinct binding hard rule
Hardpoint / Destroyed FX optional-warning boundary 보존
Layout completion = current socket truth와 persisted layout의 fresh equality
C++ / Asset / Capture mutation 0
Implementation 0
```

VB-P0-04 결과:

```text
상세 owner = VehicleBuilderProposalSpec.md v0.1.1
accepted Evidence → complete Builder-private 4 Profile prospective payload
field provenance = EvidenceDirect / EvidenceDerived / BaselineInherited / GameBias / PlannedTypedGap / UnsupportedExact
Feel physical anchor = Neutral / Low·High FACT 위장 금지
sparse Profile 금지 / current effective 또는 existing Profile clone으로 complete seed
UE 5.8 TransmissionSetup exact semantic/API mapping 확정
Transmission ratio-set = dedicated typed atomic struct 설계
Forward/Reverse gear count = ratio array length에서 derive
CVT exact simple-discrete mapping = UnsupportedExact
current ApplyVehicleMovementConfig Transmission assignment gap 확인
C++ / UE Asset / Profile commit / VehicleData Apply 0
Implementation 0
```

VB-P0-05 결과:

```text
VehicleData typed migration
= ChassisWidth + complete UE 5.8 Transmission fields 구현
= ReverseGearRatios positive-magnitude storage 보존

Drivetrain ownership
= Builder-private UCFDrivetrainProfile이 vehicle-specific Transmission numeric owner
= TransmissionRatios는 Forward/Reverse를 분리하지 않는 atomic typed field

Registry / Resolver
= current Registry 127 field
= ResolverContractRevision 2
= Reference wheel geometry는 VehicleBase fallback, adopted Asset measurement가 higher precedence
= invalid ratio/final/shift/time/efficiency 및 non-integer Shift RPM fail-closed

Reference Evidence
= UCFVehicleRefEvidence Editor-only DataAsset 구현
= FACT / DERIVED / GAME_BIAS + conflict / Unknown + consumed canonical Claim binding
= CFVREF-1 / CFVRN-1 deterministic SHA-256
= OpenSSL EVP SHA-256 + ICU Unicode NFC canonicalization

Builder-private commit
= fresh Evidence + Recipe/Target + OwnerRecipeId + 4 current Profile fingerprint
+ prospective Shared Resolver result + exact approval scope에 binding
= VehicleBase/Drivetrain/Handling/Performance complete payload를 one transaction으로 commit
= shared Profile mutation / raw VehicleData write / auto Save 금지

Builder companion / Existing Vehicle Completion
= 기존 CreateVehicleRecords의 VehicleData+Recipe core 계약 보존
= Evidence + private 4 Profile companion을 Missing만 생성
= existing valid companion은 보존, shared/foreign owner는 Conflict
= CompleteExisting 모드는 current/prospective ResolvedDefinitionHash equality를 강제해 companion 보완만으로 기존 주행 특성이 바뀌지 않게 함

Runtime / validation
= ApplyVehicleMovementConfig complete setup fail-closed
= setup 실제 변경 + valid live physics일 때만 velocity 보존 RecreatePhysicsState
= UCFVDAValidator가 Chassis/Transmission complete payload 검증
= HUD Gear는 actual Chaos GetCurrentGear authority 유지
```

검증:

```text
Official BuildEditor
= PASS
= final build job a756de887a604a7198b406d1577bbdd9

Focused Automation
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_05 2/2 PASS
= Evidence.DeterministicFingerprint Success
= Foundation.Registry127 Success
= latest UE log에서 Automation Test Queue Empty 2 tests performed + TestExit status 0 확인
= wrapper terminal succeeded / exit_code 0 / SUCCESS_COUNT 2 / FAILURE_COUNT 0

Persisted legacy Transmission audit
= CFVehicleData 11/11 PASS
= _Legacy / canonical / Scanner / Ammo / Launcher / Missile / Defense DataAsset 모두 current atomic FCFVehicleTransmissionRatios로 정상 materialize
= persisted Asset mutation / Save 0

PhysicsState lifecycle source audit
= PASS
= unconditional recreate 없음 / actual setup change + valid live physics + simulating mesh 조건에서만 recreate
= linear/angular velocity preserve
```

VB-P0-06 결과:

```text
Gameplay Setup R0 guidance
= FCFVehicleAuthoringService::ReadBuilderGameplayGuidance()
= 기존 Recipe / Profile Snapshot / AssetReader / Pure Resolver / current Target 재사용
= 별도 Builder Validator 또는 raw VehicleData writer 추가 없음

8개 completeness 영역
= Durability / Defense / DestroyedFx / Hardpoints / MountProfiles / DriveState / WheelVisual / FittingMass
= Complete / Optional / NeedsReview / Blocked
= Optional은 선택 기능 미사용을 정상 상태로 인정

Durability
= Recipe Explicit 또는 VehicleBase Profile MaxHealth가 유효한 양수여야 Complete
= Runtime fallback 값으로 authoring 누락을 숨기지 않음

Defense / Destroyed FX
= UseProfile / ExplicitAsset / ExplicitNone current semantic contract 유지
= Asset 없음 또는 ExplicitNone은 선택 기능이면 Optional
= 잘못된 Asset type/path는 Blocked
= Destroyed FX Socket이 없으면 runtime body-bounds fallback을 안내하고 자동 Socket 생성 없음

Hardpoint / Mount
= Hardpoint 실제 Socket 위치는 USER authority
= Builder는 Socket 생성·이동·surface 분석·자동배치·Save를 수행하지 않음
= stable LocationSlotId / MountProfileId / LocationSlotRef 구조만 검사
= Missing USER Socket은 NewVehicle에서 NeedsReview
= Existing Vehicle Completion은 current Target stored LocalTransform과 current resolver diff를 비교해 안전하면 강제 Socket migration 없이 보존
= Existing Hardpoint/Mount collection이 current candidate와 다르면 자동 보존 추정 대신 NeedsReview

DriveState
= ProjectDefault는 정상 Complete
= VehicleSpecific일 때만 DriveState Profile required

WheelVisual
= 기존 VehicleBase Profile + Recipe semantic policy 재사용
= Wheel center 자동검출 없음 / VB-P0-03 USER·Asset authority 유지

FittingMass
= Base/Gross 둘 다 0이면 current optional/no-fitting 상태 허용
= 사용 시 Base > 0, Gross >= Base complete contract 검사
= Gross mass 자동 추정 없음

Existing Vehicle Completion
= P0-06은 read-only
= current Target과 resolved candidate의 Gameplay pending diff만 보고
= 실제 Definition Apply는 VB-P0-07 Final Review owner
```

검증:

```text
Final Official BuildEditor
= PASS
= build job 12e537e016b64a179a813a4929f584e8
= exit code 0

Focused Automation
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance
= 1/1 PASS
= SUCCESS_COUNT 1 / FAILURE_COUNT 0 / engine exit code 0
= process job e9f180c8dcd34aa489bb12713bba8a52

검증 mutation
= production VehicleData / Recipe / Profile / Defense / FX / Fitting Asset mutation 0
= StaticMesh Socket mutation 0
= Save 0
```

VB-P0-07 결과:

```text
Final Review R0 aggregate
= FCFVehicleAuthoringService::ReadBuilderFinalReview()
= one fresh ResolveVehiclePreview 결과에서 existing Validation / R16 External Drift / Gameplay Guidance projection을 재사용
= exact Shared Resolver FieldDiff + BuildDiffHash 재사용
= 같은 Final Review refresh에서 Resolver 중복 실행 없음
= 병렬 Validator / Target writer 추가 없음

Final Review compact summary
= Validation Warning / Blocked / Error
= Gameplay NeedsReview / Blocked
= External Drift presence
= Target Diff count + exact DiffHash
= consumed canonical Reference Evidence Claim provenance
  - FACT Claim count
  - DERIVED Claim count
  - GAME_BIAS Claim count
= Recipe persistent BuilderCommitReceipt가 accepted Evidence fingerprint + consumed Claim set hash + current private 4 Profile fingerprints + Resolver revision을 증명해야 provenance available
= caller가 같은 Evidence의 다른 canonical Claim subset을 넘겨도 receipt mismatch로 blocker
= current schema에 persisted field→claim map이 없으므로 provenance를 FACT-backed field count로 위장하지 않음

Apply readiness
= blocker0 + External Drift0 + Resolve Success + Target Diff>0일 때만 bCanApply
= existing BuildApplyApprovalProposal이 exact DefinitionApply ProposalHash 생성
= Apply 직전 Final Review 전체를 fresh 재실행
= approval scope mismatch / state drift fail-closed

Explicit Apply
= FCFVehicleAuthoringService::ApplyBuilderFinalReview()
= existing ApplyResolvedVehicle() → FCFVehicleApplyService::Apply() 단일 Target writer lane만 사용
= raw VehicleData write / shared Profile mutation / auto Save / automatic retry 없음

Guarded Undo
= successful Builder Apply 직후 exact UE TransactionId + owner Recipe/Target + pre-Apply Target/AppliedState + post-Apply Target/AppliedState + semantic Recipe fingerprint를 transient token으로 캡처
= explicit UndoScopeHash approval 필요
= current Editor lifetime에서 Builder가 발급한 token만 허용
= exact transaction이 current Undo stack top이고 current Target/Recipe/AppliedState가 expected post-Apply state와 exact 일치할 때만 GEditor->UndoTransaction()
= intervening Editor transaction 또는 transaction 밖 raw Target/AppliedState drift가 있으면 StateChanged로 fail-closed
= Undo 뒤 full Target Definition hash + semantic Recipe fingerprint + Recipe AppliedState exact pre-Apply readback 검증

Hardpoint authority
= VB-P0-06 USER Socket authority 그대로 유지
= Final Review/Apply/Undo가 Socket 생성·이동·자동배치를 추가하지 않음
```

검증:

```text
Final Official BuildEditor
= PASS
= build job ff4dd50981464710876a0693d2287432
= exit code 0

Focused Automation
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_07.FinalReviewApplyUndo
= 1/1 PASS
= SUCCESS_COUNT 1 / FAILURE_COUNT 0 / engine exit code 0
= process job df306775e2f245aabccd307d741f6bda

Covered flow
= fresh Final Review + consumed Evidence provenance 3종
→ explicit DefinitionApply
→ post-Apply Target Diff 0 / FinalReview complete
→ unrelated Editor transaction 삽입
→ guarded Undo StateChanged 차단
→ unrelated standard Undo
→ exact Builder transaction guarded Undo
→ pre-Apply Target/AppliedState restoration
→ pending Diff 복원

Production validation mutation
= persistent production VehicleData/Profile/Evidence/Asset mutation 0
= StaticMesh Socket mutation 0
= Save 0
```

Post-Review Hardening 결과:

```text
Persistent Builder commit receipt
= UCFVehicleRecipeData::BuilderCommitReceipt
= accepted Profile ProposalHash / Evidence identity+fingerprint / consumed Claim set hash / private 4 Profile fingerprints / prospective resolved hash / Resolver revision 기록
= Recipe semantic fingerprint에는 포함하지 않음
= 기존 정상 Profile payload가 같고 receipt만 없으면 receipt-only R1 commit 허용
= receipt-only commit은 Profile payload/revision mutation 0 / Target 0 / Save 0

Builder private Profile commit hardening
= persistent assignment fingerprint readback
→ PostEditChange
→ final fingerprint + receipt readback
→ 성공 뒤 package dirty
= readback mismatch 시 complete payload/revision/receipt/dirty state rollback + transaction cancel

Deterministic approval hardening
= Builder Profile prospective resolve와 Builder Companion missing-profile prospective resolve 모두 transient UObject path를 SourceSignature authority로 사용하지 않음
= actual/predicted persistent Profile path + typed snapshot으로 Pure Resolver 계산
= Preview → Commit fresh Preview 사이 ProposalHash deterministic replay 보장

Final Review provenance hardening
= fresh Evidence만으로 provenance를 추정하지 않음
= persistent receipt와 current private 4 Profile fingerprint가 모두 일치해야 available
= different canonical Claim subset tamper는 blocker

Guarded Undo hardening
= current Undo stack top 외에 post-Apply Target/semantic Recipe/AppliedState exact state도 Undo 직전 검사
= nontransactional raw drift는 StateChanged로 차단

Latest Official BuildEditor
= PASS
= build job 306b9c9be7f648a4b5a89b6eb13c7f05
= exit code 0

Latest sequential focused Automation
= VB-P0-05 4/4 PASS / process 1be0c0853c8a43d1bf0b9063a85c13ce
  - BuilderCompanionFlow PASS
  - BuilderProfileCommit PASS
  - Evidence.DeterministicFingerprint PASS
  - Foundation.Registry127 PASS
= VB-P0-06 GameplayGuidance 1/1 PASS / process 8a77ba7635024f4d8ac20c91244f65f4
= VB-P0-07 FinalReviewApplyUndo 1/1 PASS / process 4385151a829643c4b35da5ff3df670e3
= all engine exit code 0 / failure count 0

Validation mutation
= production VehicleData/Profile/Evidence/Asset mutation 0
= StaticMesh Socket mutation 0
= Save 0
```

VB-P0-08 Technical Driving Benchmark 결과:

```text
RuntimeRead boundary
= direct property snapshot은 가능하지만 arbitrary getter/function 호출은 현재 contract 밖
= GetForwardSpeed / GetEngineRotationSpeed / GetCurrentGear 호출을 위해 Product debug getter/HUD를 추가하지 않음
= 기존 Chaos mobility measurement pattern을 전용 Automation harness로 일반화

Technical harness
= CarFight.VehicleBuilder.CF_FQ_040.VB_P0_08.TechnicalDrivingBenchmark
= saved VehicleData object path + optional FittingData object path 입력
= fresh PIE / deferred BP_CFVehiclePawn spawn / BeginPlay 전 VehicleData·Fitting 주입
= Fitting 있음: FittingSnapshot.TotalVehicleMassKg
= Fitting 없음: VehicleData.BaseVehicleMassKg
= product Asset/Map/Config Save 0
= 결과 JSON: UE/Saved/CarFight/VehicleBuilderBenchmarkResult.json

Measured metrics
= runtime configured / actual mass
= 0→50 km/h
= 0→100 km/h
= peak speed + top-speed stability
= peak RPM / peak-speed actual gear
= 100 km/h→Idle braking time/distance
= 30 km/h / Steering 0.5 / 2s steady yaw
= low-speed full-steer effective turning radius

Deterministic measurement condition
= UE 5.8 Source-confirmed -UseFixedTimeStep -FPS=60
= independent metric reset separates Heavy PhysicsState rebuild from Light kinematic reset
= Heavy reset → settle → Light reset → drive 순서로 suspension/transmission state 안정화
= production FCFChaosVehicleMassRuntime::ReapplyVehicleMassKg()가 Heavy reset의 lifecycle-safe PhysicsState rebuild를 담당
= forward phase starts explicit gear 1; Automatic continues Chaos auto-shift, Manual uses configured ChangeUpRPM/ChangeDownRPM test driver
= existing map Defense SUV fixture is collision/physics/tick disabled after spawn-transform extraction
= Peak Speed/RPM/Gear telemetry only updates during active acceleration/top-speed phases
= 100→Idle braking은 별도 reset/re-acceleration 없이 같은 top-speed trajectory에서 100km/h entry를 재확보한 뒤 Brake 1.0 적용

Latest Official BuildEditor
= PASS
= build job 0a8f33824563433ab32097a612f434d9
= exit code 0

Fixed60 fitted repeat canary
B process b87b3bfb96694c05af00b1923e2e9b83
C process ece88090da5c42bdbc7aaa640da22ecd
= both exit code 0 / metric count 1
= exact repeated values
  - Mass 1570.000kg
  - 0→50 3.716667s
  - 0→100 19.433334s
  - Peak 129.324310km/h
  - 100→Idle 2.400002s / 32.669586m
  - Yaw 21.206821deg
  - Effective Turning Radius 8.517635m

Fixed60 base-mass canary
= process 0006a45bd3eb456581a2f125ff612d8e
= exit code 0 / metric count 1
= Mass 1000.000kg
= 0→50 3.900000s
= 0→100 unavailable (-1)
= Peak 89.953712km/h
= Yaw 17.858885deg
= Effective Turning Radius 8.799048m

Known limitation
= 100km/h 미도달 차량은 100→Idle braking result를 unavailable로 기록하며 기술 실패로 처리하지 않음
= fitted representative canary에서는 100km/h 도달과 100→Idle braking branch를 실제 runtime으로 관측 완료
= test-only torque/final-ratio boost로 차량 성능을 조작하지 않음
= Reference 적합성·차량간 주행감 구분·USER Driving Feel은 P0-09에서 검증

PASS meaning
= Technical harness / deterministic measurement condition / saved VehicleData + optional Fitting path가 동작함
= Reference fact와의 실제 적합성·차량간 주행감 구분·USER Driving Feel은 아직 PASS 아님
= production VehicleData/Profile/Evidence/Asset/Socket mutation 0 / Save 0
```

VB-P0-09 Guided Shell Step 2~4 integration 결과:

```text
Guided entry
= CarFight.VehicleBuilder 별도 Nomad Tab
= 기존 Vehicle Authoring Advanced Workspace 보존

Step 2 MeshPrep
= fresh FCFVehicleAssetSnapshot authority
= Chassis + FL Wheel 최소 필수
= FR/RL/RR 미지정은 Warning semantics 유지
= 지정 Wheel resolve + usable bounds 검사

Step 3 SocketGuide
= configured name / default Wheel_Anchor_* role binding
= current Chassis 4/4 found + distinct hard rule
= custom name 허용
= Wheelbase / FrontTrack / RearTrack read-only 계산
= USER Hardpoint warning-only / auto placement 0

Step 4 LayoutCapture evaluator
= capture mutation 0
= current Target VehicleLayoutConfig와 fresh current socket identity/location/rotation direct exact equality
= NotCaptured Ready / exact Current Complete / mismatch Stale
= Legacy Pin/Advanced Override precedence가 current socket mismatch를 가리지 않음

Validation
= CFVehicleBuilderVM v1.2.0
= CFVehicleAuthoringVMTests v1.11.0
= Official Build 1b1aa29b80c341c6a6289ac2693413e9 PASS / exit 0
= focused BuilderShell c6ab98e244634dd1a7fbcb84b290d4e3 1/1 PASS / failure 0
= Product Asset/Socket/Save mutation 0
```

이 결과는 VB-P0-09 USER Acceptance를 수행하기 위한 기술 연결 체크포인트다. USER가 실제 신규 차량 1대를 끝까지 제작하고 주행감을 확인하기 전에는 VB-P0-09 PASS로 승격하지 않는다.

VB-P0-09 Step Extensibility Hardening 결과:

```text
Stable identity
= ECFVehicleBuilderStepId
= 제거된 ID를 다른 의미로 재사용하지 않음

Single composition owner
= GetVehicleBuilderStepDefinitions()
= current Step 구성 / 순서 / 제목을 한 곳에서 소유
= StepNumber는 array index + 1로 자동 파생

Evaluator isolation
= RebuildStepStates()는 orchestrator
= current definition 순서를 순회해 EvaluateStepById(StepId) dispatch
= 각 Step은 Evaluate*Step() 함수에서 current state를 독립 평가
= Step 상태 lookup/write는 FindStepView(StepId) / SetStep(StepId)

Slate
= navigation은 StepViews.Num() 기반
= Step-specific UI는 numeric index가 아니라 StepId 기반

Automation
= exact 8/index 기반 assertion 제거
= current baseline core Step 존재를 stable StepId lookup으로 검증

Future maintenance
= 새 Step: StepId + definition row + evaluator + 필요 시 StepId content/test
= 순서 변경: definition row 이동만으로 navigation + evaluator dispatch 순서 동시 변경
= 제거/통합: current definition에서 제외, old StepId는 reserved
= 기존 Step 규칙 수정: 해당 evaluator + focused test 중심

Explicit non-goals
= generic plugin registry 0
= Blueprint workflow engine 0
= Step DataAsset 0
= persistent progress graph 0
```

Validation:

```text
FCFVehicleBuilderVM v1.3.1
CFVehicleBuilderTypes v1.1.0
SCFVehicleBuilderTab v1.2.0
CFVehicleAuthoringVMTests v1.12.0
Official Build 8e1c55defcbc49108c4c402964a33477 PASS / exit 0
focused BuilderShell 0e2b3bd9260446c084e0523427283e02 1/1 PASS / failure 0 / engine exit 0
Product Asset/Socket/Save mutation 0
```

이 hardening은 현재 baseline 8 Step을 기능적으로 변경하지 않고, 완성 후 Step 추가·제거·재배치·규칙 교정을 fixed-index 전역 수정 없이 수행하기 위한 유지보수 경계를 만든다.

VB-P0-09 Step 1 Reference Evidence / Companion USER flow 결과:

```text
AI → Builder transient handoff
= UE/Saved/CarFight/VehicleBuilder/ResearchDraft.json
= FCFBuilderResearchDraft SchemaRevision 1
= exact RecipeId + TargetDefinitionPath binding
= persistent SSOT/approval token이 아님

Initial Reference validation
= exact Primary Reference 1
= stable Reference/Source/Claim/Conflict/Unknown ID
= FACT citation required
= DERIVED input + method/revision required
= initial GAME_BIAS reject
= canonical FACT/DERIVED 최소 1
= unresolved Block conflict reject
= Unknown 숫자 자동 보충 0

Companion R2
= 새 Evidence는 빈 record 생성 금지
= fixed NewEvidenceId + complete initial Evidence payload
= prospective EvidenceFingerprint를 ProposalHash에 binding
= Existing valid Evidence/private Profile 보존
= Missing만 생성
= shared/foreign Profile fail-closed
= CompleteExisting baseline preservation 유지
= explicit USER OwnershipWrite approval
= one transaction / auto Save 0 / VehicleData Apply 0

Guided Shell Step 1
= Reference summary 표시
= AI Research Draft 불러오기
= Companion 생성 내용 mutation0 검토
= explicit USER Yes 뒤 R2 commit
= 이 Reference Set으로 진행
= local review token RecipeId + EvidenceId + EvidenceFingerprint

Step 1 Complete
= managed Recipe/Target valid
+ Evidence exact binding/fingerprint valid
+ blocking conflict/ProposalBlock Unknown 0
+ USER review token이 exact current fingerprint와 일치

Stale
= USER-reviewed Evidence fingerprint 이후 semantic Evidence 변경 시 자동 Stale
= old mutation approval은 restart/resume에서 복원하지 않음

Step boundary
= private 4 Profile companion 존재는 Step 1 completion authority가 아님
= Physics numeric proposal/review는 Step 5 owner 유지
```

Validation:

```text
Final Official Build
= 06370173f2c64a1ab4dd3d235bd46c18 PASS / exit 0

Data Authoring broad run
= 71 tests performed
= Step 1 affected focused tests:
  - CF_FQ_040.VB_P0_09.BuilderStep1Reference PASS
  - CF_FQ_040.VB_P0_09.BuilderShell PASS
  - CF_FQ_040.VB_P0_05.BuilderCompanionFlow PASS

Unrelated existing regression
= DAUTH_P0_08.Batch.AllowlistProjection FAIL
= stale typed-schema leaf-count assertion: expected domain 8/1 and total79, current 13/6 and total89
= Step 1 closure blocker로 확대하지 않음

Protection
= production Product Asset Save 0
= VehicleData Apply 0
= shared Profile mutation 0
= raw VehicleData write 0
= auto Socket/Hardpoint mutation 0
= Git commit/push 0
```

이 결과는 **VB-P0-09 Step 1 Technical PASS**다. 실제 신규 차량 E2E 제작과 USER Driving Feel 확인 전에는 VB-P0-09 전체 USER PASS가 아니다.

VB-P0-09 Step 5 Physics Proposal Guided flow 결과:

```text
AI → Builder transient handoff
= UE/Saved/CarFight/VehicleBuilder/PhysicsDraft.json
= FCFBuilderPhysicsDraft SchemaRevision 1
= exact RecipeId + TargetDefinitionPath + EvidenceId + EvidenceFingerprint binding
= persistent SSOT/approval token이 아님

Prerequisite
= Step 1~4 current Complete
= current Reference Evidence exact binding/fingerprint valid
= Step 1 USER review token exact current
= Builder-private Profile 4/4 exact OwnerRecipeId

Typed proposal authority
= complete FCFBuilderPrivateProfilePayload
= VehicleBase / Drivetrain / Handling / Performance 4 domain
= mutation destination path는 AI Draft path가 아니라 current Recipe exact private binding으로 강제
= ConsumedClaimIds + opaque ProposalCorrelationHash

Preview / Commit
= FCFVehicleAuthoringService::PreviewBuilderProfiles() mutation0
= USER가 human-facing summary / consumed Claim / domain별 변경 여부 / prospective resolved hash 검토
= explicit USER AuthoringWrite approval
= FCFVehicleAuthoringService::CommitBuilderProfiles() existing R1 lane
= shared/foreign Profile write 0
= Target VehicleData mutation 0
= Definition Apply 0 — Step 7 owner
= auto Save / auto retry 0

Persistent completion truth
= Recipe.BuilderCommitReceipt
= accepted Evidence path/id/fingerprint
= canonical consumed Claim set + hash
= private 4 Profile fingerprints
= prospective ResolvedDefinitionHash
= Resolver revision

Step 5 derived state
= prerequisite incomplete → Locked
= missing/foreign private Profile → Blocked
= fresh loaded Draft with pending profile change → Ready
= exact committed receipt/current Profile/Evidence/Resolver match → Complete
= Evidence/Profile/receipt/resolver drift → Stale
= Editor restart 뒤 transient Draft/approval 복원 0
= fresh VM은 persistent current truth에서 Complete/Stale 재계산

Guided Shell
= AI Physics Proposal 불러오기
= Physics Proposal 검토 후 반영
= raw VehicleData grid나 Chaos 세부 숫자 입력을 정상 USER flow로 요구하지 않음
```

Validation:

```text
Final Official Build
= 3d65b8e8b6d04f498ceaf6358618c6a0 PASS / exit 0

Data Authoring broad run
= process 99a062448f2e4173b07e5ce666c932b1
= engine exit 0 / 72 tests performed
= 71 PASS / 1 unrelated FAIL

Affected PASS
= CF_FQ_040.VB_P0_09.BuilderStep5Physics
= CF_FQ_040.VB_P0_09.BuilderStep1Reference
= CF_FQ_040.VB_P0_09.BuilderShell
= CF_FQ_040.VB_P0_05.BuilderProfileCommit
= CF_FQ_040.VB_P0_05.BuilderCompanionFlow

Unrelated existing regression
= DAUTH_P0_08.Batch.AllowlistProjection FAIL
= typed Profile schema leaf-count stale assertion: expected 79 / current 89
= Step 5 blocker로 확대하지 않음

Protection
= Product Asset Save 0
= Target VehicleData Apply 0
= shared Profile mutation 0
= raw VehicleData write 0
= auto Socket/Hardpoint mutation 0
= Git commit/push 0
```

이 결과는 **VB-P0-09 Step 5 Physics Proposal Guided Flow Technical PASS**다. 실제 신규 차량 E2E 제작·Step 7 Apply·Step 8 USER Driving Feel 확인 전에는 VB-P0-09 전체 USER PASS가 아니다.

VB-P0-09 Step 6 Gameplay Setup Guided flow 결과:

```text
Authority
= FCFVehicleAuthoringService::ReadBuilderGameplayGuidance() existing R0
= parallel validator/writer 0
= ResolveVehiclePreview + Recipe/Profile/Asset snapshot + Pure Resolver current truth 재사용

Prerequisite
= Step 5 Physics Proposal current Complete
= Step 5 Stale/Blocked/Ready이면 Step 6 Locked

8-area projection
= Durability
= Defense
= DestroyedFx
= Hardpoints
= MountProfiles
= DriveState
= WheelVisual
= FittingMass

State mapping
= backend BlockedCount > 0 → Step 6 Blocked
= NeedsReviewCount > 0 → Step 6 Ready / USER 수동 확인 필요
= bCanCompleteGameplayStep → Step 6 Complete
= persistent Step6 complete flag 0
= fresh selection/refresh마다 R0 truth에서 재계산

USER Socket authority
= SocketGuidance를 SemanticId / SocketName / chassis found / existing stored transform accepted / instruction으로 표시
= Hardpoint/DestroyedFx Socket 자동 생성·이동 0
= USER가 Static Mesh Editor Socket Manager에서 직접 조정
= 조정 뒤 '현재 상태 다시 확인'으로 fresh R0 재평가

Pending Gameplay Diff
= PendingGameplayDiffCount를 Step 6에 표시
= pending diff 자체는 Step 6 completion blocker 아님
= 실제 Target VehicleData Apply는 Step 7 Final Review owner

Guided Shell
= Gameplay Setup / 8영역 점검 패널
= 영역별 Complete / Optional / NeedsReview / Blocked
= Summary / ResolutionText / RelatedFieldPath
= USER Socket 안내
= 별도 Apply 버튼 0
= global fresh refresh 사용

Protection
= Recipe mutation 0
= Profile mutation 0
= Target mutation 0
= Save 0
= auto retry 0
```

Validation:

```text
Official Build
= d01056d228214ee8bbd06b154768e96d PASS / exit 0

DataAuthoring broad
= process c62c2247d53b43fe8cfcbbbee723dc68
= engine exit 0 / 72 performed / 71 PASS / 1 unrelated FAIL

Affected PASS
= CF_FQ_040.VB_P0_06.GameplayGuidance
= CF_FQ_040.VB_P0_09.BuilderStep5Physics
  - Step 5 commit 뒤 Step 6 8-area Complete
  - R0 mutation0
  - fresh VM Step 6 Complete resume
  - Step 5 Profile drift → Step 6 Locked + old guidance 폐기

Unrelated existing regression
= DAUTH_P0_08.Batch.AllowlistProjection FAIL
= typed Profile leaf-count stale assertion expected79/current89
= Step 6 blocker로 확대하지 않음
```

이 결과는 **VB-P0-09 Step 6 Gameplay Setup Guided Flow Technical PASS**다. 실제 Target Apply와 USER Driving을 완료하기 전에는 VB-P0-09 전체 USER PASS가 아니다.

VB-P0-09 Step 7 Final Review / explicit Apply Guided flow 결과:

```text
Read authority
= FCFVehicleAuthoringService::ReadBuilderFinalReview() existing R0
= one fresh Resolve를 Validation / External Drift / Gameplay / Diff / Provenance에 재사용
= parallel review/validator/writer 0

Prerequisite
= Step 6 Gameplay Setup current Complete
= current Reference Evidence exact path/id/fingerprint
= persistent BuilderCommitReceipt provenance

Guided review
= Warning / Blocker / External Drift
= Reference provenance + consumed Claim category counts
= 전체 FieldDiff canonical path + before/after value
= DiffHash / ProposalHash / prospective ResolvedDefinitionHash / Resolver revision

Explicit DefinitionApply
= USER dialog 직전 fresh R0 Final Review
= USER가 전체 Diff/provenance/mutation boundary 확인
= explicit DefinitionApply approval
= ApplyBuilderFinalReview() existing R3 lane
= backend가 Apply 직전 다시 fresh Final Review
= approval ProposalHash mismatch면 stale 차단
= Target VehicleData + Recipe AppliedState transaction mutation 가능
= auto Save 0 / auto retry 0

Guarded Undo
= successful Builder Apply가 발급한 current Editor lifetime exact FCFBuilderUndoToken만 보관
= exact TransactionId + Recipe/Target identity + post-Apply state + UndoScopeHash
= current UE Undo stack top가 exact Builder Apply transaction일 때만 실행
= transaction 밖 Target/Recipe/AppliedState drift가 있으면 fail-closed
= arbitrary Workspace Undo와 분리
= successful Undo 뒤 token consume + fresh Final Review

Derived Step state
= Step 6 incomplete → Locked
= review error / blocker / external drift → Blocked
= diff>0 + bCanApply → Ready
= diff0 + blocker0 + bCanCompleteFinalReview → Complete
= persistent Step7 complete bool 0
```

Validation:

```text
Final Official Build
= f9a9ced62f1545f884ceefdfd2a966ab PASS / exit 0

Focused Automation only
= a7efe7e368cd42f5b57a18c9b66f756f
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep7FinalReview
= 1/1 PASS / failure 0 / engine exit 0

Verified transition
= fresh Final Review Ready with Target Diff
= R0 Target/Recipe mutation0 / Save0
= explicit DefinitionApply → Target + AppliedState transaction update / Save0 / auto retry0
= post-Apply Final Review diff0 → Step7 Complete
= exact guarded Undo → pre-Apply Target hash restore
= post-Undo diff restored → Step7 Ready

Protection
= Step 1~6 PASS suite replay 0
= Product Asset Save 0
= shared Profile mutation 0
= raw VehicleData writer 0
= auto Socket/Hardpoint mutation 0
= Git commit/push 0
```

이 결과는 **VB-P0-09 Step 7 Final Review / explicit Apply / guarded Undo Guided Flow Technical PASS**다. 실제 Builder 대상 Step 8 Driving과 USER E2E 확인 전에는 VB-P0-09 전체 USER PASS가 아니다.

VB-P0-09 Step 8 Technical Driving + USER Driving Guided flow 결과:

```text
Technical authority
= existing Tools/RunBuilderBench.ps1 / VB-P0-08 fixed 60Hz fresh PIE benchmark 재사용
= 새 physics simulator / 임의 Reference threshold / USER driving feel 자동 판정 0

Saved-state gate
= Step 7 Apply 뒤 Recipe 또는 Target VehicleData Dirty면 benchmark launch 차단
= Builder auto Save 0
= disk에 존재하는 current saved Target만 external benchmark 대상

Exact binding
= current Target VehicleData exact object path
+ current Target DefinitionHash
+ fresh Benchmark RunId
= 세 identity가 모두 일치해야 current Technical result 인정
= Target hash 변경 → Stale
= 새 RunId → 이전 USER Driving PASS 재사용 금지

Guided benchmark UX
= non-blocking child process로 existing runner 실행
= metric: mass / 0→50 / 0→100 / peak / braking / yaw / turning radius / RPM / gear
= 0→100 미도달(-1)은 관측 결과이며 임의 failure로 승격하지 않음
= reference_threshold_asserted=false
= user_driving_feel_asserted=false

USER Driving
= current benchmark exact binding 뒤 active PIE 필요
= selected saved VehicleData를 transient duplicate
= current Player CFVehiclePawn runtime에만 적용
= persistent VehicleData / Recipe / Map / Config mutation 0
= 실제 PIE transient 적용 성공 전 USER PASS API fail-closed
= USER explicit confirmation 뒤 RecipeId + Target DefinitionHash + Benchmark RunId local token 기록
= exact token이 current truth와 모두 일치할 때만 Step 8 Complete

Resume
= fresh VM에서도 exact USER token을 current benchmark/Target과 다시 대조
= 같은 Target이라도 새 benchmark RunId면 Ready로 복귀
= persistent Step8 complete bool 0
```

Validation:

```text
Final Official Build
= 055dbddf74f342e9b10a92e1fb2542b8 PASS / exit 0

Focused Automation only
= aa2a365599fc46d484857894aa0df47a
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep8Driving
= 1/1 PASS / failure 0 / engine exit 0

Runner envelope canary
= process 6135fbff338c47f8944dd5b8b02d5069
= RunId/ExpectedTargetDefinitionHash additive envelope actual execution PASS
= engine exit 0 / metric count 1
= 기존 VB-P0-08 수치 판정 재설계 0
= canary 뒤 canonical Saved metric JSON은 이전 DefenseSUV_BaseMass_Latest 내용으로 복원
= single-slot CFVehicleBuilderBench.log는 마지막 canary 로그이며 과거 로그를 추정 복원하지 않음

Protection
= Step 1~7 Technical PASS replay 0
= Product Asset auto Save 0
= shared Profile/raw VehicleData writer 0
= auto Socket/Hardpoint mutation 0
= benchmark가 USER Driving Feel을 대신 판정 0
= Git commit/push 0
```

이 결과는 **VB-P0-09 Step 8 Guided Flow Technical PASS**다. 실제 신규 차량 한 대를 Builder로 처음부터 끝까지 만들고 사용자가 직접 주행 승인하기 전에는 VB-P0-09 전체 USER PASS가 아니다.

정확한 다음 Gate:

```text
VB-P0-09 End-to-End USER Acceptance 계속
→ actual candidate = /Game/CarFight/Vehicles/Meshes/Wagon/Wagon
→ USER Guided core create: DA_Vehicle_Wagon + DA_Recipe_Wagon
→ USER Wagon mesh prep: Wheel_Anchor_FL/FR/RL/RR actual wheel-center Socket placement
→ Recipe AssetIntent WheelMeshFL = /Game/CarFight/Vehicles/Shared/Tire/Wheel_FL
→ Step 2~4 current truth 확인
→ Reference/Physics/Gameplay/Final Apply 실제 신규 차량 E2E
→ current saved Target Technical Benchmark
→ actual PIE USER Driving
→ USER Driving PASS
→ 그 뒤에만 VB-P0-09 USER Acceptance closure
```

Actual E2E kickoff evidence:

```text
Editor
= canonical Tools/RunEditor.ps1 start
= UE MCP connected / generation 4

New chassis candidates
= Wagon / Van / SubCompact / Pickup / Coupe / Compact / CityCar
= fresh persisted AssetDump에서 전부 static_mesh_socket_count 0

Control comparison
= Sedan socket_count 7
= SUV socket_count 6
= 둘 다 Wheel_Anchor_FL/FR/RL/RR 실제 authored socket 보유
= 따라서 신규 7종 socket 0은 관측 공백이 아니라 실제 준비 전 상태

Chosen actual E2E candidate
= /Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon
= core suggested names: DA_Vehicle_Wagon / DA_Recipe_Wagon
= current Wagon wheel socket 0 → Step 3 expected blocker

Step 2 minimum wheel
= /Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL
= live GetBounds PASS
= min(-49.9995,-12.4996,-49.9995) / max(49.9995,12.4996,49.9995)
= size ≈99.9990 × 24.9992 × 99.9990cm
= canonical 100 × 25 × 100cm PASS
= Bounds Center ≈ 0 PASS
= FR/RL/RR는 신규 normal path에서 FL fallback 예정

Manual authority
= Wheel Socket 4개 위치는 USER가 Wagon actual wheel center에 직접 배치
= +X forward / left=-Y / right=+Y
= HardpointIntents empty는 current Step 6 Optional
= Destroyed FX Socket missing도 runtime bounds-center fallback 가능
= 따라서 이번 첫 manual prep에서 HP/FX Socket을 불필요하게 강제하지 않음

Transport boundary
= Project Registry operations: editor.build/editor.start/editor.stop/ue.read only
= CarFight-specific Builder write operation 없음
= Slate Observe/Click은 server policy blocked
= backend/raw DataAsset 우회 생성 금지
= 실제 Guided UX acceptance이므로 USER가 Builder UI에서 core create/mesh prep 수행
```

E2E Guided UX feedback remediation:

```text
Persisted actual Wagon state
= DA_Vehicle_Wagon / DA_Recipe_Wagon USER Save 완료, fresh Editor dirty=false
= Wagon Chassis Wheel_Anchor_FL/FR/RL/RR 4개 persisted
= Socket Location/Rotation/Scale은 USER-authored authority 유지

Discovered UX gap
= persisted DA_Recipe_Wagon AssetIntent.WheelMeshFL/FR/RL/RR가 비어 있음
= Static Mesh Socket Preview Mesh는 위치 확인용 표시이며 Recipe WheelMesh binding이 아님
= 따라서 기존 evaluator 기준 Step 2 MeshPrep Blocked → Step 3 진입 불가가 정상 판정

Remediation
= 작업 대상 row를 stable identity 기반 accent 색 + 관리 상태 badge로 구분
= Step 2에 Chassis + FL 필수 / FR·RL·RR 선택 StaticMesh picker 추가
= picker 변경은 pending only, [Mesh 설정 반영]에서만 existing typed CommitAssetIntent Recipe-only lane 사용
= FR/RL/RR 미지정 시 기존 FL reuse semantics 유지
= Step 3 표시를 `소켓 준비 / Naming`으로 명확화
= 필수 Wheel Socket 4개 exact 이름/현재 존재 여부/개별 복사 + current Recipe optional Socket 목록/일괄 복사 제공
= [차체 메시 열기]는 Static Mesh Editor만 열며 Socket 생성/이동/저장 자동화 0

Protection
= Wheel auto-detection 0
= Product VehicleData Apply 0
= Builder auto Save 0
= USER Wagon Recipe 자동 mutation 0
= WSA-P0-01 official next implementation gate 유지

Validation
= Official Build ff8abe01a9fa4e6e90c785618ffccb91 PASS / exit 0
= focused BuilderShell fb4b91e909254180975b30db00c0e813 PASS 1/1 / failure 0 / engine exit 0
= Step 2 FL missing → Blocked, typed Recipe-only restore → Complete, Target VehicleData hash 불변 regression 포함
= fresh canonical Editor runtime_bb56feccfa7346e3bd4fc66c59844aa6 Ready / protocol_ready=true
= USER list color + Step 2/3 actual visual/action recheck Pending
```

Actual Wagon offsite AI-only preparation checkpoint:

```text
ResearchDraft
= UE/Saved/CarFight/VehicleBuilder/ResearchDraft.json
= SchemaRevision 1 / exact DA_Recipe_Wagon RecipeId + DA_Vehicle_Wagon Target binding
= JSON/schema/reference/citation/enum/Feel-response consistency PASS

Representative Reference
= 2026 Volvo V60 Cross Country Ultra B5 AWD KR
= generic Wagon mesh의 exact identity 주장 아님 / representative benchmark 역할
= trim-specific length 4785mm vs Wagon mesh length ≈4790.6mm
= explicit body width 1850mm → Chaos ChassisWidth 185.0cm
= 1490mm height / 1886kg running mass / 2350kg maximum mass / AWD / automatic / 250ps / 0-100 6.9s / 180km/h
= exact 2026 KR peak torque / gear ratios 미공개 → Unknown + ProposalWarning 유지

Initial private Profile seed
= VehicleBase / Drivetrain / Handling / Performance complete payload 준비
= Front WheelClass /Game/CarFight/Vehicles/Blueprints/BP_Wheel_Front.BP_Wheel_Front_C
= Rear WheelClass /Game/CarFight/Vehicles/Blueprints/BP_Wheel_Rear.BP_Wheel_Rear_C
= exact gear ratios Unknown이므로 bUseTransmissionConfig=false

WSA protection
= bUseReferenceWheelGeometry=false
= Profile fallback Radius/Width 30/30cm, 6/6cm 값은 비활성 baseline으로 보존
= actual wheel size authority는 shared Wheel bounds ≈100×25×100cm + USER Socket Scale (0.8,1.0,0.8)
= resolved target expectation Radius≈40cm / Width≈25cm
= Step 5 AI proposal이 Radius/Width independent overwrite 금지

Persistent mutation during offsite preparation
= UE Asset/Profile/Recipe/Target 0
= Save 0
= USER approval token 0

Next USER gate
= Step 1 current Reference Set review/accept
→ Companion explicit creation
→ Step 5 Evidence provenance/Profile proposal
→ Step 7 full Diff USER review + explicit Apply
→ USER Save
→ Technical Driving + WSA-P0-07 PIE USER Acceptance
→ USER driving feel approval 뒤 VB-P0-09 closure
```

Actual Wagon Step 8 drivetrain incident checkpoint — 2026-08-31:

```text
Step 7
= actual DA_Vehicle_Wagon explicit Apply 완료
= Target + Recipe USER Save 완료

Step 8 transient PIE apply
= runtime VehicleData 교체 후 기존 Sedan ChassisMesh가 남는 gap 발견
= InitializeVehicleRuntime() 초기에 ApplyVehicleVisualConfig()를 재적용하도록 교정
= Official Build PASS
= VD_P0_04.RuntimeReinitializeVisualContract PASS
= VB_P0_09.BuilderStep8Driving PASS
= USER가 실제 PIE에서 Wagon 차체 전환 정상 확인

Technical Benchmark
= RunId b4ad2a76-78e8-4d1c-9cb9-cc891c1257f2
= TargetHash 4ea8dcc0f941f2dc639c694c23cb20ae
= 0→50 8.650s
= 0→100 미도달
= Peak Speed 80.219 km/h
= Peak RPM / Gear 4504.5 / 1

USER direct driving
= RPM HUD 표기는 없어 직접 수치 확인 불가
= 1단에서 약 80 km/h에 도달한 뒤 2단으로 상향 변속 확인
```

Drivetrain root cause:

```text
actual DA_DT_Wagon
= bUseTransmissionConfig=false

Resolver contract
= false이면 Drivetrain Profile의 Transmission fields를 candidate로 제공하지 않음

fallback
= VehicleData Project Compatibility Default가 effective Transmission으로 남음
= Forward 2.85 / 2.02 / 1.35 / 1.00
= FinalRatio 3.08
= ChangeUpRPM 4500
= ChangeDownRPM 2000

판정
= compatibility baseline이 신규 Wagon의 vehicle-specific Transmission처럼 최종 주행에 사용된 Builder proposal gap
= 80 km/h 자체가 runtime shift trigger인 것은 아님
= 고정 ChangeUpRPM에 도달했을 때 legacy ratio/final/wheel 조합의 결과 차속이 비정상적으로 높아진 것
```

Remediation boundary:

```text
P0 Runtime
= 현재 Chaos automatic + 고정 ChangeUpRPM/ChangeDownRPM 유지
= 가변 shift map / 속도 기반 runtime shift trigger는 이번 remediation Scope Out

Builder
= 신규 Guided Vehicle에서 Compatibility Default를 final vehicle-specific Transmission 완료 근거로 조용히 승계 금지
= exact 실차 ratio가 Unknown이면 Unknown provenance 보존
= 필요한 vehicle-specific Transmission은 DERIVED/GAME_BIAS Proposal + USER review로 명시
= persistent BuilderTransmissionPolicy로 LegacyCompatible / VehicleSpecificRequired를 구분
= NewVehicle/CompleteExisting transient lifecycle mode는 Final Review 신규 차량 판별 authority로 사용 금지
= Transmission Core는 FACT/DERIVED/USER-reviewed GAME_BIAS field-level provenance 필요
= GearChangeTime / TransmissionEfficiency는 P0 Auxiliary BaselineInherited 허용
= Physics Draft + BuilderCommitReceipt에 dedicated Transmission Proposal hash/review metadata binding
= 기어별 ChangeUpRPM 기준 예상 변속 차속 + 변속 직후 RPM + Downshift margin diagnostic 제공
= Resolver/Project Compatibility Default/기존 Legacy 차량 동작은 변경하지 않음

Acceptance
= Wagon 수치만 임의 tuning하지 않음
= Proposal 계약 교정 PASS → source implementation PASS → Official Build PASS → focused Automation → fresh Wagon persisted evidence → exact Wagon policy migration → Wagon Transmission Proposal 재작성 → Technical Benchmark 재실행 → USER direct driving 순서
= 그 전까지 VB-P0-09 USER Driving PASS 보류
```

Product scope correction:

```text
이 작업의 제품 대상 = Wagon이 아니라 Guided Vehicle Builder 전체 신규 차량
Wagon = incident / pre-policy migration canary / actual E2E regression fixture
Generic acceptance = 차종 비종속 synthetic contract가 먼저 PASS해야 함
Wagon migration/benchmark = generic contract PASS 뒤 실제 regression 단계
```

Source implementation checkpoint:

```text
Guided 신규 Recipe = VehicleSpecificRequired
Legacy/일반 Recipe = LegacyCompatible
PhysicsDraft = schema v2 + field-level TransmissionReview
Step 5 = vehicle-specific Core provenance + fixed-shift diagnostic fail-closed
BuilderCommitReceipt = TransmissionPolicy + TransmissionProposalHash + TransmissionReview
Final Review = current Drivetrain/policy/provenance fresh revalidation
Generic Resolver / Compatibility Default = unchanged
```

Current implementation/actual regression checkpoint:

```text
Builder-wide contract
= Source Implementation PASS
= Official Build PASS
= focused Transmission regression PASS

actual Wagon
= VehicleSpecificRequired policy persisted
= refreshed Evidence persisted
= PhysicsDraft v2 private Profile commit persisted
= USER-approved DefinitionApply persisted
= Target DefinitionHash b96d3833c9a18bb5816b34e9dbcc4785
= Forward 8단 + Reverse 3.992 + Final 3.20
= ChangeUp/Down 4500/2000 유지
= fresh Final Review post-Apply Diff 0 / Blocker 0
```

USER-approved Apply harness Official Build `3d0dd45823c44554a17de70c7111b51b` PASS, persistent DefinitionApply process `568ea5023f8748549e361d50209dbb27` PASS다. fresh AssetDump에서 Target 8단/Final3.20과 Recipe `AppliedDefinitionHash=b96d3833c9a18bb5816b34e9dbcc4785`, `AppliedRecipeRevision=12`, current Evidence/receipt binding을 재확인했다.

post-Apply 기존 Technical Driving Benchmark `RunId=f1b8ca0c-1ec9-4cff-8162-0a180f412fea`의 72.991km/h/3단 결과는 **1단 약 80km/h 고정 regression 해소 증거까지만 유효**하다. 이후 USER가 별도 `M_VehicleBenchmark`에서 직접 주행해 6단 진입과 약 200km/h 도달을 확인했으므로 `72.99km/h가 Wagon의 성능 한계`라는 해석은 폐기한다.

USER high-speed evidence:

```text
6단 진입은 비교적 이르게 체감
약 200km/h까지는 지속 가속
200km/h 부근부터 가속 증가가 매우 느려짐
전자식 180km/h 실차 limiter는 CarFight에 적용하지 않음
```

read-only `TorqueCurveInspect`는 inherited BP Chaos curve에서 4244RPM 배율 약 `0.94048`을 확인했다. 따라서 `200km/h 부근에서 TorqueCurve가 과도하게 토크를 잘라 차량이 막힌다`는 단순 가설은 기각한다. 반면 current `ChangeUpRPM=4500`은 실제 TCU FACT가 아니라 이전 `FixedShiftTransmissionBias` GAME_BIAS이며 full-throttle wheel-torque crossover 관점에서 재작성 대상이다.

`M_VehicleBenchmark` 자동 고속시험 시도에서는 약 2.6km 진행 이후 vehicle runtime reference loss/주행면 이탈이 발생해 high-speed metric authority가 되지 못했다. generic Step 8 benchmark는 기존 fixture로 복원하고 destroyed UObject `IsValid` fail-closed와 read-only TorqueCurve inspect만 additive hardening으로 보존한다.

post-Apply Builder Transmission focused process `d0f8bae7daf3487e9aebb57b652eb9bb` exact 9/9 PASS다.

상세 owner는 `VehicleBuilderProposalSpec.md v0.1.10`, Reference vocabulary owner는 `VehicleRefEvidenceSpec.md v0.1.3`다. WSA-P0-07 USER PASS / Wheel Size Authority P0 Complete는 `WheelSizeAuthorityPlan.md v0.1.19`가 별도 소유하며 이번 Transmission closure에서 덮어쓰지 않는다.

---

## VB-P0-10 Current System Promotion — COMPLETE

2026-09-02 기준 `CF-FQ-040`의 Current System Promotion을 완료했다.

```text
G0 Evidence
= PASS
= VB-P0-09 End-to-End USER Acceptance PASS
= WSA P0 Complete
= ESH-01~06 Complete / Final Audit Clean PASS
= current Wagon exact TargetHash/RunId/RecipeId acceptance 보존

G1 Current Knowledge Promotion
= PASS
= Document/Systems/Vehicles/VehicleBuilder.md v1.0.0 신규 Current owner
= SystemIndex / FeatureQueue current owner 동기화
= Data Authoring 역할 = Builder Backend + Advanced Workspace 고정

G2 Current Route Cleanup
= PASS
= ActiveWork Ready row 제거
= FeatureQueue Ready candidate 제거 / Done 전환
= Plan Index Ready route 제거

G3 Reference Preservation
= PASS
= 본 Plan/Roadmap과 supporting VehicleBuilder 문서는 기존 경로에서 완료 evidence를 계속 보존
= Archive Index에 Historical + Retained Path 등록

G4 Historical
= PASS
= 대표 Plan은 현재 착수 기준에서 내려오며 old next-action을 Current 작업으로 사용하지 않음

G5 Optional Physical Move
= NOT RUN
= plan_repo에 기존 dirty VehicleBuilder Plan/Roadmap과 unrelated ConceptArt deletion이 함께 존재
= physical move는 완료 조건이 아니며 unrelated dirty 보호를 위해 별도 maintenance 전까지 Retained Path 유지
```

Current 구현 판단은 `Document/Systems/Vehicles/VehicleBuilder.md v1.0.0`과 실제 Source/Asset을 우선한다.
CF-FQ-038의 DEL6/UA-08 잔여 lifecycle은 별도 Paused 상태를 유지하며 이 완료로 흡수하거나 완료 처리하지 않는다.
새 관련 failure가 없는 한 ESH를 재개하거나 Wagon을 재튜닝하지 않는다.

---

## 17. Changelog

### v0.1.47 - 2026-09-02

- G5 Physical Move maintenance를 완료해 대표 Plan과 supporting VehicleBuilder 문서를 `Document/Plan/Archive/VehicleBuilder/`로 이동했다.
- semantic Done/Historical 상태와 완료 evidence는 변경하지 않고 placement만 `Archived Path`로 전환했다.
- Current 구현 owner는 `Document/Systems/Vehicles/VehicleBuilder.md v1.0.0`이다.

### v0.1.46 - 2026-09-02

- `VB-P0-10 Current System Promotion`을 완료하고 `CF-FQ-040` 대표 Plan을 `Historical + Retained Path`로 전환했다. Current owner는 `Document/Systems/Vehicles/VehicleBuilder.md v1.0.0`이다.
- Historical Gate G0~G4를 PASS로 닫았다. G5 physical move는 완료 필수조건이 아니며 current plan_repo dirty/unrelated deletion 보호 때문에 수행하지 않았다.
- ActiveWork/FeatureQueue/Plan Index current route 제거와 Archive Index retained-path 등록을 closure contract로 고정했다. CF-FQ-038은 Builder Backend + Advanced Workspace 역할만 고정하고 자체 Paused 잔여 lifecycle은 유지한다.

### v0.1.45 - 2026-09-02

- ESH 최종감사에서 high-speed `ExpectedTargetDefinitionHash`가 실제 Asset hash 검증 없이 result metadata로만 기록되는 P1과 stale 설명 P2 3건을 확인·교정했다.
- `CFVehicleHashCheck` read-only commandlet을 추가해 existing `FCFVehicleSnapshotBuilder`로 saved VehicleData semantic DefinitionHash를 계산하고, `RunHighSpeedBench.ps1 v1.1.0`에서 expected hash mismatch를 benchmark 시작 전에 fail-closed하도록 보강했다. wrong-hash는 Exit124로 차단되고 automation log/result JSON은 변경되지 않았다.
- USER 수동 공식 build PASS 뒤 새 commandlet positive/negative regression PASS, ESH-01 exact3 PASS, ESH-02 exact5 PASS, ESH-03 exact2 PASS를 확인했다. fresh persisted Wagon high-speed는 expected/observed `0e5b48e8dcd39deba441da9237218be6`, `target_hash_verified=true`, T100 6.100 / T150 13.167 / T200 31.183 / Peak206.700 / FinalGear6 / Override false를 재현했다.
- 테스트 후 fresh AssetDump에서 Wagon Product Asset 상태가 유지됐고 임시 진단 스크립트 잔존 0을 확인했다. ESH-01~06은 Final Audit Clean PASS로 봉인하며 next Gate는 VB-P0-10이다.

### v0.1.44 - 2026-09-02

- ESH-06 actual Wagon USER Driving을 완료하고 USER가 최종 승인했다. USER 관측은 5단 약 204km/h → 6단 진입 → 약 206km/h에서 가속 여유가 매우 작아지는 형태였으며, 기존 ESH-05 persisted benchmark의 206.700km/h / FinalGear6와 정합해 이상 현상이 아닌 현재 gearing/engine/aero 특성으로 판정했다.
- Builder `USER 주행 PASS` 버튼을 통해 RecipeId `05F69DD34990A868DEFFD29C1AF5B2F9`, TargetHash `0e5b48e8dcd39deba441da9237218be6`, Benchmark RunId `256cd822-419e-4a3e-ac23-388b570d78c2` exact local acceptance token이 실제 저장된 것을 read-only 진단으로 확인했다.
- 이에 ESH-06 USER Driving PASS 및 VB-P0-09 End-to-End USER Acceptance PASS로 닫는다. CF-FQ-040 Feature 자체는 아직 Done이 아니며 다음 Gate는 `VB-P0-10 Current System Promotion / Advanced Authoring Role Lock`이다.

### v0.1.43 - 2026-09-02

- ESH-06 USER Driving 전 current TargetHash `0e5b48e8dcd39deba441da9237218be6`에 exact binding한 fresh Step 8 benchmark를 실행했다. process `1a8e03cfbe804e52a5c98be74b07d24e` PASS, RunId `256cd822-419e-4a3e-ac23-388b570d78c2`, `reference_threshold_asserted=false`, `user_driving_feel_asserted=false`다.
- current managed Editor는 Ready지만 automated PIE start는 GoPyMCP lifecycle interpreter identity mismatch 및 UE write policy block으로 시작되지 않았다. Product/Asset mutation 0이며 next는 USER Play 1회 → active PIE Wagon transient apply → direct driving → explicit USER Driving PASS다.

### v0.1.42 - 2026-09-02

- USER 승인한 fixed-common `ChangeUpRPM=5500`을 existing Builder Profile/Receipt commit → exact Final Review → R3 DefinitionApply 경로로 Product에 적용했다. current Target DefinitionHash는 `0e5b48e8dcd39deba441da9237218be6`, `ChangeDownRPM=2000`, 8AT/Final3.20/6-point Engine Curve는 보존됐다.
- ESH-03 post-Apply exact2 `6c204a6a29ba44f19fe2ba6ba327aa3f`와 ESH-02/Builder affected exact5 `ed5af5d23d8f4ed280d64f0f54418f69`가 모두 PASS했다. raw WheelTorque 6222는 theory diagnostic으로 계속 보존한다.
- ESH-05 persisted no-override high-speed retest `9abcb642fe7241b585a7e7fcc1637296` PASS: `change_up_override=false`, 5500/2000, T100 6.100s / T150 13.167s / T200 31.183s / Peak 206.700km/h / FinalGear 6 / Stable true다. 다음 Gate는 ESH-06 USER Driving PASS다.

### v0.1.41 - 2026-09-02

- Wagon Engine Curve Target DefinitionApply 완료 상태와 persisted Target DefinitionHash `83c69e52522dc72649c32477b5aad220` / `bUseEngineTorqueCurve=true`를 current checkpoint에 반영했다.
- ESH-03 raw `WheelTorqueCrossoverShift@1` 6222RPM diagnostic은 focused `447c54667fbc41369ed260b0382e4437` exact 2/2 PASS로 보존하되 자동 적용값과 분리했다.
- ESH-04 production-parity high-speed authority를 확립했다. 잘못된 1886kg force-reapply fixture를 제외하고 no-fitting Legacy Mass 1500/1767.377kg 기준 4500 baseline 206.793km/h / 6단을 재현했다.
- transient 4500/5500/6222 A/B/C에서 5500이 T200 31.183s로 가장 빠르면서 6단 연결과 baseline 수준 최고속을 유지해 P0 fixed-common GAME_BIAS USER Review 후보로 선정됐다. Product ChangeUpRPM은 계속 4500이며 mutation은 USER 승인 전 금지한다.

### v0.1.40 - 2026-09-01

- ESH-03 중간검수에서 Target DefinitionApply 전 production 경로 조기 연결과 nested WheelTorqueShift warning/blocker 미전파를 P1로 확인하고 dormant draft safety correction을 적용했다.
- production `BuildTransmissionDiagnostic()`은 ESH-03 default-off로 복원하고 explicit test opt-in에서만 실행한다. enabled 경로는 nested diagnostic parent 전파, malformed ratio/radius/post-shift RPM fail-closed와 fixed-common 최대 torque-gap 15% 초과 USER Review warning을 포함한다.
- Official Build `c3813e9aad004b6c94b7c9829f7cd119` PASS, ESH-03 correction exact2 `27773a815a9149589219d93f139a7d13` 2/2 PASS, ESH-02/Step5/FinalReview affected `1aa1a0bc067342d281a9994f39769329` 5/5 PASS다.
- 이 검증은 ESH-03 Technical PASS나 착수 승인이 아니다. Target DefinitionApply 전 ESH-03 `Not Started`, Product Target/Profile/Recipe mutation 0을 유지한다.

### v0.1.37 - 2026-09-01

- actual Wagon USER high-speed 주행에서 6단 진입과 약 200km/h 도달을 확인해 기존 72.991km/h benchmark를 최고속/출력 한계 evidence로 해석한 문구를 폐기했다. 해당 기존 benchmark는 1단 고정 incident 해소 regression evidence로만 보존한다.
- 다음 차량에서 같은 시행착오를 반복하지 않도록 Engine / Shift / Top-Speed 반복 방지 계약을 추가했다. MaxTorque/MaxRPM + inherited BP TorqueCurve 상태는 vehicle-specific Engine Complete가 아니다.
- Torque/Power band 및 curve context를 Research 우선 항목으로 승격하고, actual shift schedule FACT가 없을 때 PeakTorque/PeakPower/차급 평균 RPM을 ChangeUpRPM으로 복사하는 shortcut을 금지했다.
- P0 fixed shift는 vehicle-specific Engine Curve + gear ratios/final drive 기반 wheel-torque crossover diagnostic을 먼저 만들고 단일 threshold를 USER-reviewed `WheelTorqueCrossoverShift@1` GAME_BIAS로 선택하도록 했다.
- 실차 전자식 최고속 제한은 Evidence에는 보존하되 CarFight Runtime에는 적용하지 않는다. limiter가 걸린 PublishedTopSpeed를 순수 물리 최고속이나 FinalRatio 역산 authority로 사용하지 않는다.
- high-speed benchmark는 dedicated map의 충분한 forward distance, progress, lateral deviation, collision/KillZ/destruction 검증 없이는 PeakSpeed를 차량 한계 evidence로 사용할 수 없도록 authority 조건을 명문화했다.
- `TorqueCurveInspect`에서 Wagon 4244RPM 배율 약 0.94048을 확인해 inherited curve 과도한 torque cut 가설을 기각했다. M_VehicleBenchmark 자동시험의 runtime loss/이탈 결과는 vehicle performance failure가 아니라 benchmark fixture gap으로 분리했다.

### v0.1.36 - 2026-09-01

- USER가 exact DefinitionApply ProposalHash `f85a0c69da0518ca7b8427dbe364a589` 진행을 승인했다. 기존 `CFBuilderWagonReview` commandlet에 승인 hash가 있을 때만 existing `ExecutePreparedFinalReviewApply()`를 호출하는 fail-closed apply mode를 추가했고, no-hash 기존 R0 review-only 동작은 보존했다.
- Apply mode는 pre-state Target hash `4ea8dcc0f941f2dc639c694c23cb20ae`, DiffHash `ed4c2b01160ea56fc67dea68cdbb26dd`, exact two-row diff(`FinalRatio`, `TransmissionRatios`), prospective hash `b96d3833c9a18bb5816b34e9dbcc4785`를 모두 재검사한 뒤에만 R3 DefinitionApply를 실행한다. Target+Recipe 두 package save는 pre-Apply bytes backup과 rollback을 사용한다.
- Official Build `3d0dd45823c44554a17de70c7111b51b` PASS, persistent apply process `568ea5023f8748549e361d50209dbb27` PASS다. fresh AssetDump에서 Target 8단 ratios/reverse3.992/final3.20/4500-2000과 Recipe AppliedDefinitionHash/receipt/policy를 확인했다.
- post-Apply benchmark `RunId=f1b8ca0c-1ec9-4cff-8162-0a180f412fea`는 0→50 3.00s, 0→100 미도달, Peak 72.991km/h, PeakSpeedGear 3, PeakEngineRPM 6103.03을 기록했다. 기존 1단 약 80km/h incident는 해소됐지만 전체 최고속/0→100 성능은 추가 확인이 필요하다.
- `PeakEngineRPM`은 active drive 전체의 순간 최대값, `PeakSpeedGear`는 최고속 시점 gear이므로 서로 paired same-frame metric이 아니다. 6103RPM을 곧바로 ChangeUpRPM 4500 무시로 해석하지 않는다.
- post-Apply Builder Transmission focused process `d0f8bae7daf3487e9aebb57b652eb9bb` exact 9/9 PASS다. `VB-P0-09 USER Driving PASS`는 USER 직접 주행과 저속 최고속 원인 follow-up 전까지 HOLD한다.

### v0.1.35 - 2026-09-01

- actual Wagon post-WSA reconciliation, VehicleSpecificRequired policy migration, persistent Evidence Refresh 이후 `PhysicsDraft v2` Step 5 Profile commit을 완료했다. 4 private Profile + Recipe만 저장했으며 Target VehicleData는 아직 4단/Final 3.08 상태로 보존했다.
- Step 5 one-shot은 initial compile field-name 2건과 constructor symbol 누락 1건을 교정한 뒤 Official Build `50a531b1ea074cfea5f8e836841ee624` PASS, process `3ffd2d2da1ac418bb2c93f32e7750980` PASS로 닫았다.
- fresh Drivetrain Profile은 forward 8단 `5.057/3.070/2.050/1.520/1.262/1.000/0.792/0.661`, reverse `3.992`, Final `3.20`, ChangeUp/Down `4500/2000`, `bUseTransmissionConfig=true`로 persisted됐다. Recipe AuthoringRevision은 12이며 VehicleSpecificRequired receipt가 current Evidence `b6204acf...`와 exact binding됐다.
- actual Wagon Step 7 mutation0 Final Review를 추가/실행해 Official Build `ae1e16dbbb314650bec6a3393f24ae65` PASS, review process `3f6cebc2015445a190f35adff6f0950a` PASS를 얻었다.
- Final Review는 Warning 11 / Blocker 0 / External Drift 0 / Target Diff 2 / Apply 가능이다. 실제 Target mutation 예정값은 `FinalRatio 3.08→3.20`과 `TransmissionRatios 4단+Reverse2.86 → 8단+Reverse3.992` 두 항목뿐이다. ChangeUp/Down 4500/2000 등 동일값은 Diff 0이다.
- DefinitionApply ProposalHash는 `f85a0c69da0518ca7b8427dbe364a589`, prospective ResolvedDefinitionHash는 `b96d3833c9a18bb5816b34e9dbcc4785`, DiffHash는 `ed4c2b01160ea56fc67dea68cdbb26dd`다. Apply는 USER explicit DefinitionApply 승인 전까지 수행하지 않는다.

### v0.1.34 - 2026-09-01

- Vehicle Builder의 기본 제작 경로를 `Segment/평균차 우선`이 아니라 **특정 실존 차량의 exact Model Year/Trim/Powertrain/Market Primary Reference lock**으로 명시했다.
- USER가 차량 추천을 요청할 수 있도록 AI Vehicle Recommendation Contract를 추가했다. 추천은 Data Availability / Builder Confidence / CarFight Value를 기준으로 3~5개 exact 후보를 조사·제시한다.
- 초기 Vehicle Library는 제조사 제원, 기어비/final drive, 엔진 band, 질량/치수/타이어/성능 등 FACT가 풍부하고 variant confusion이 적은 차량을 우선하도록 했다.
- 존재하지 않는 차량은 Vehicle Archetype의 현실적 범위/분포 + USER desired character + 선택적 Secondary Reference를 Primary fallback으로 사용한다. Archetype 평균은 실존 차량 FACT를 덮어쓰지 않는다.
- 실존 차량의 Unknown Runtime 값은 차급 평균으로 숨겨 채우지 않고 vehicle-specific DERIVED 또는 explicit GAME_BIAS + USER Review로 처리한다.
- Technical Reference identity와 게임 내 이름/엠블럼/브랜드 표현을 분리해, 외형/브랜딩 결정이 기술 Evidence authority를 훼손하지 않도록 했다.
- actual Wagon current checkpoint를 WSA reconciliation + VehicleSpecificRequired policy migration + persistent Evidence Refresh PASS / PhysicsDraft v2 USER Review Pending으로 동기화했다.

### v0.1.33 - 2026-09-01

- actual Wagon WSA-P0-07 USER PIE에서 Wheel 위치, Right 방향, 굴림 방향, 회전을 PASS해 `WheelSizeAuthorityPlan.md v0.1.19 / Wheel Size Authority P0 Complete`를 current dependency로 승격했다.
- WSA authority reacceptance blocker는 해제됐다. next actual Wagon Builder 작업은 fresh Resolver/adoption evidence refresh로 이전 WSA adoption fingerprint mismatch를 재평가한 뒤 policy/Evidence/Profile/VehicleData apply를 재개한다.
- Transmission/PhysicsDraft v2와 USER Driving Hold는 기존 별도 lane을 유지하며 WSA 완료 상태와 혼합하지 않는다.

### v0.1.32 - 2026-08-31

- actual Wagon용 `ResearchDraft.json`을 later MY26 transmission research로 갱신하고 mutation0 Existing Evidence Refresh dry-run으로 prospective EvidenceFingerprint `b6204acf1d6722009075865204f33b693c96ed002b4c6ba6eed21751b40b535f`를 확정했다.
- `PhysicsDraft.json`을 schema v2로 재작성했다. 8단 forward ratio + reverse ratio + FinalRatio는 reviewed EvidenceDirect, AutoReverse 및 fixed ChangeUp/ChangeDown RPM은 explicit GAME_BIAS, GearChangeTime/TransmissionEfficiency는 P0 auxiliary BaselineInherited로 구분했다.
- actual Wagon `WagonTransmissionDraft`가 prospective transient Evidence 기준 VehicleSpecificRequired TransmissionReview PASS, consumed Claim binding PASS, `TransmissionProposalHash=14ec8925b761f16111251c1abac8c218`를 산출했다.
- dry-run 중 current Resolver가 WSA `AcceptedMeasurementFingerprintMismatch`로 Blocked인데 compatibility/default Wheel Radius를 ShiftSpeed authority처럼 소비하는 공용 결함을 발견했다. `CFBuilderTransUtil`을 fail-closed해 Resolve Success가 아니면 ShiftSpeed를 숨기고 `Transmission.WheelRadiusAuthorityUnavailable` blocker를 표시하며, Gear Ratio/PostShift RPM diagnostic은 유지한다.
- 최종 Official Build `1b0e1fea045140e1a7d0df1fa7a97871` PASS, Builder Transmission focused `412db121c71447fbb936f2e111cf7f4a` exact 9/9 PASS, actual Wagon dry-run `9d7d48c8b47f4e78beb120524a7a9c26` PASS다.
- actual Wagon policy/Evidence/Profile/VehicleData Product Asset mutation은 여전히 0이다. current WSA owner가 reacceptance를 통해 Resolver authority를 Success로 복원하기 전에는 policy migration / Evidence Refresh commit / Step 5 Profile commit / Apply로 진행하지 않는다.

### v0.1.31 - 2026-08-31

- Existing Reference Evidence later research enrichment 공백을 별도 reviewed R1 Evidence Refresh로 닫았다. Companion 생성과 Existing Evidence replacement는 분리되고, refresh는 Save0 / identity binding 보존 / explicit AuthoringWrite를 강제한다.
- Official Build `d822c7a28d374263a4f67658730b8818` PASS, focused process `1d3f63dc32bc4d6b93dae330f38e352f` 9/9 PASS로 공용 Evidence Refresh + Transmission contract를 Technical PASS 처리했다.
- next는 actual Wagon fresh dump → policy-only one-time migration → refreshed Research Evidence → PhysicsDraft v2 actual proposal이다.

### v0.1.30 - 2026-08-31

- Builder-wide Transmission contract를 semantic/value provenance와 structural payload까지 보강하고 Official Build `87fc2d61a6f1401fa5402a9fff6f3354` 및 exact focused 6/6 PASS로 Technical PASS 처리했다.
- generic contract는 특정 Wagon 데이터 없이 검증됐다. 따라서 Wagon은 이제 contract 구현 대상이 아니라 pre-policy migration + actual E2E regression vehicle로만 남는다.
- next는 fresh persisted AssetDump로 actual Wagon Recipe/Evidence/private Profile/receipt current truth를 확보한다. stale dump를 authority로 쓰지 않으며 fresh evidence 전에 Wagon policy나 숫자를 변경하지 않는다.

### v0.1.29 - 2026-08-31

- USER 기준에 따라 Transmission remediation의 범위를 Wagon 전용 수정으로 해석하지 않도록 교정했다. canonical owner는 **모든 신규 Guided Vehicle의 vehicle-specific Transmission authoring contract**이며 Wagon은 결함 발견 incident와 실제 regression/acceptance vehicle이다.
- generic `BuilderTransmissionContract` synthetic Automation과 전용 exact runner를 추가해 FACT/DERIVED/GAME_BIAS 및 주요 fail-closed 조건을 Wagon Asset 없이 검증하도록 focused scope를 보강했다.
- next는 새 test source official Build → generic focused Automation이다. 이 둘이 PASS한 뒤에만 fresh Wagon persisted evidence와 one-time policy migration으로 이동한다.

### v0.1.28 - 2026-08-31

- Wagon Transmission remediation을 Design PASS에서 `Source Implementation PASS / Official Build PASS / Focused Automation Pending`으로 전진했다. 상세 owner는 `VehicleBuilderProposalSpec.md v0.1.4`다.
- 신규 Guided Recipe의 persistent `VehicleSpecificRequired`, Legacy default `LegacyCompatible`, PhysicsDraft schema v2 field-level TransmissionReview, Step 5 fail-closed, persistent receipt hash/review, Final Review fresh revalidation과 fixed-shift diagnostic 구현을 반영했다.
- Official Build `fb3ab376111d4c9283afc2731dba3e89`는 UHT PASS, compile/link 22/22, Exit0으로 성공했다. focused Automation 전이므로 affected regression closure로 확대하지 않는다.
- actual Wagon policy/numeric Transmission mutation은 0이며 fresh persisted evidence → one-time policy migration → Wagon Physics Proposal v2 → Technical Benchmark → USER Driving 순서를 유지한다. `VB-P0-09 USER Driving PASS`는 계속 보류한다.
- WSA current 상태는 별도 `WheelSizeAuthorityPlan.md` owner를 따른다. 이번 문서 동기화로 WSA-P0-07 상태나 Right fallback orientation/spin remediation을 변경하지 않는다.

### v0.1.27 - 2026-08-31

- Wagon Transmission incident의 remediation 설계 재검수를 PASS로 닫고 상세 owner를 `VehicleBuilderProposalSpec.md v0.1.3`으로 전진했다.
- Source 확인 결과 `NewVehicle/CompleteExisting`는 Evidence/private Profile/receipt/Import·Apply history에 따라 전환되는 workflow state라 신규 차량 Final Review authority로 사용할 수 없음을 확정했다. 신규 Guided record에는 persistent `VehicleSpecificRequired`, 기존/Legacy Recipe에는 `LegacyCompatible` 기본값을 사용하는 정책을 채택했다.
- Generic Resolver와 Project Compatibility Default는 변경하지 않고 Builder Step 5/Final Review에서만 vehicle-specific Transmission completion을 fail-closed한다. Core의 BaselineInherited-only 완료는 금지하고 FACT/DERIVED/USER-reviewed GAME_BIAS field-level provenance를 요구한다.
- Current aggregate BuilderCommitReceipt만으로 Transmission field provenance를 증명할 수 없는 gap을 보완하기 위해 dedicated Transmission review/hash receipt와 fixed-shift kinematics diagnostic을 구현 대상으로 확정했다.
- 다음 exact 순서는 source implementation + focused regression → Wagon Recipe policy one-time migration → Wagon Physics Proposal 재작성이다. 이 단계 전에는 Wagon ratio/final/shift RPM을 임의 변경하지 않고 `VB-P0-09 USER Driving PASS`를 계속 보류한다.

### v0.1.26 - 2026-08-31

- actual Wagon E2E 진행 상태를 Step 1~7 Apply+Save 완료 / Step 8 USER Driving Hold로 전진했다. runtime VehicleData 교체 후 Sedan 차체가 남던 문제는 `ApplyVehicleVisualConfig()` 재적용 교정과 focused regression 뒤 USER PIE에서 Wagon 전환 정상으로 확인했다.
- Step 8 Technical Benchmark `RunId=b4ad2a76-78e8-4d1c-9cb9-cc891c1257f2`에서 0→100 미도달, Peak 80.219 km/h, 4504.5RPM / 1단을 기록했고, USER direct driving에서도 약 80 km/h에서 1→2단 변속을 확인했다.
- root cause를 `DA_DT_Wagon.bUseTransmissionConfig=false` → Resolver Transmission candidate 미생성 → Project Compatibility Default 4단 payload가 effective Transmission으로 남는 신규 Guided Vehicle proposal gap으로 확정했다.
- P0 Runtime은 고정 ChangeUp/Down RPM을 유지하고 가변 shift map/속도 기반 runtime shift trigger는 Scope Out으로 유지한다. 신규 Guided Vehicle에서 compatibility fallback을 vehicle-specific Transmission 완료 상태로 인정하지 않는 remediation과 기어별 예상 변속 차속/변속 후 RPM diagnostic 방향은 `VehicleBuilderProposalSpec.md v0.1.2`가 소유한다.
- Wagon의 임의 ratio/torque tuning은 아직 수행하지 않는다. Proposal 계약 교정 → Wagon Transmission Proposal 재작성 → Technical Benchmark → USER direct driving 전에는 `VB-P0-09 USER Driving PASS`를 기록하지 않는다.

### v0.1.25 - 2026-08-29

- actual Wagon의 USER 부재 AI-only preparation을 완료했다. `ResearchDraft.json`을 exact Wagon Recipe/Target에 binding하고 2026 Volvo V60 Cross Country KR representative Evidence + complete private 4 Profile seed를 준비했다.
- Volvo 2026 Support의 explicit body width 1850mm를 Chaos `ChassisWidth=185.0cm`로 사용하고 overall width와 분리했다. peak torque/gear ratios는 current official source 미공개라 Unknown으로 보존하며 exact Transmission payload를 임의 생성하지 않는다.
- actual Front/Rear WheelClass generated class를 seed에 고정하고, WSA는 `bUseReferenceWheelGeometry=false`로 보호했다. shared Wheel 100×25×100cm + USER Socket Scale `(0.8,1.0,0.8)`가 Radius≈40cm / Width≈25cm authority다.
- Draft consistency PASS, persistent UE Asset/Profile/Recipe/Target/Save mutation 0이다. next는 Step 1 USER Reference review부터 시작하며 Step 7 full Diff 승인, USER Save, Technical Driving/WSA-P0-07, driving feel approval 전에는 VB-P0-09를 닫지 않는다.

### v0.1.24 - 2026-08-28

- 실제 Wagon E2E에서 Socket Preview Mesh와 Recipe WheelMesh binding이 별개라 `WheelMeshFL`이 비어 Step 2가 Blocked되는 Guided UX 공백을 발견하고 교정했다.
- 작업 대상은 stable identity별 accent 색 + 관리 상태 badge로 구분하고, Step 2는 Chassis/FL 필수 + FR/RL/RR 선택 object picker와 explicit typed Recipe-only `Mesh 설정 반영`을 제공한다. Step 3은 `소켓 준비 / Naming`으로 명확화하고 필수/선택 Socket 이름·복사·차체 메시 열기를 제공한다.
- Build `ff8abe01a9fa4e6e90c785618ffccb91` PASS, focused `BuilderShell` `fb4b91e909254180975b30db00c0e813` 1/1 PASS다. USER 화면 재확인과 실제 Wagon Recipe Mesh binding은 Pending이며 WSA-P0-01 official next gate는 유지한다.

### v0.1.23 - 2026-08-28

- `WheelSizeAuthorityPlan.md v0.1.4` 최종 Source 감사 PASS를 반영했다. `bUseWheelSocketScale`은 Project default=false + Recipe WheelVisualIntent explicit owner로 고정하고 VehicleBase Profile dependency를 제거한다.
- Socket mode Gameplay Guidance는 Legacy AutoScale clamp/MeasureMode/AutoCenter를 blocker로 사용하지 않으며, Step 5 AI Physics Draft는 private VehicleBase의 Reference wheel geometry 5필드를 baseline-preserve해야 한다.
- Canonical shared Wheel PASS와 기존 Step1~8 Technical PASS는 보존한다. next는 WSA-P0-01 code/schema/helper 구현이다.

### v0.1.22 - 2026-08-28

- `WheelSizeAuthorityPlan.md v0.1.3` 설계 재감사를 반영했다. USER가 실제 shared Wheel_FL로 교체/이동한 뒤 live Bounds가 `99.9990 × 24.9992 × 99.9990cm`, center≈0으로 확인되어 canonical Default Wheel asset sub-gate를 PASS로 닫았다.
- WSA current 설계는 `100×25×100` canonical Wheel Mesh, USER Socket Scale authority, narrow `WheelSizeSourceFingerprint`, Step 4 Scale equality, enum append-only, Legacy AutoScale/AutoCenter/Clamp 비사용을 포함한다.
- 기존 Builder Step1~8 Technical PASS는 보존하며 next는 WSA-P0-01의 C++/Registry/helper 구현이다. 실제 신규 차량 USER E2E는 계속 Pending이다.
- 별도 untracked Wagon/Wheel_FL은 canonical owner가 아니므로 자동 정리하지 않았다.

### v0.1.21 - 2026-08-28

- USER 결정에 따라 `WheelSizeAuthorityPlan.md v0.1.1`을 CF-FQ-040의 정식 하위 Plan으로 등록했다. v0.1.1 Source audit에서 기존 AssetSnapshot/R6 Measurement/AssetAdoption 재사용 설계를 확정했다.
- 신규 정상 차량 제작의 타이어 크기 Authority를 USER-authored `Wheel_Anchor_FL/FR/RL/RR` Socket Scale로 고정하고, AI/코드의 휠하우스 기반 자동 크기 선택을 금지했다.
- Wheel StaticMesh Bounds는 원본 실제 치수 Source로만 사용하며, 공용 Default Wheel radial X/Z를 100cm canonical target으로 one-time normalization한 뒤 Socket Scale에서 Visual Scale과 Physics Radius/Width를 함께 파생하도록 설계했다.
- `bUseWheelSocketScale` 신규 명시 모드, `FCFWheelAnchorPose.RelativeScale`, FL default mesh fallback, axle 좌우 일치, stale/validator, Step2~5/7 통합 계약을 고정했다.
- 기존 `bAutoScaleWheelMeshToRadius`는 신규 정상 경로에서 사용하지 않고 Legacy compatibility로 유지한다. SocketScale과 AutoScale 동시 사용/곱셈은 금지한다.
- 현재 shared `Wheel_FL` Bounds가 X/Z≈79.358cm임을 확인해 100cm normalization 전 Wagon Socket Scale authoring을 진행하지 않도록 next gate를 `WSA-P0-01 Canonical Default Wheel + Data Schema & Shared Size Utility`로 삽입했다.
- C++/UE Asset/Runtime mutation은 이번 Design 단계에서 0이다. 기존 Step1~8 Technical PASS는 반복하지 않는다.

### v0.1.20 - 2026-08-27

- VB-P0-09 Step 8 Technical Driving + USER Driving Guided Flow를 Technical PASS로 닫았다.
- existing VB-P0-08 fixed-60Hz runner를 saved Target exact path + Target DefinitionHash + fresh RunId에 binding하고, 결과 metric을 Guided Shell에 표시한다. runner는 Reference threshold와 USER driving feel을 계속 판정하지 않는다.
- Step 7 Apply 뒤 dirty Recipe/Target은 benchmark launch를 fail-closed하며 Builder auto Save는 0이다. current benchmark 뒤 selected saved VehicleData는 transient duplicate로 active PIE Player VehiclePawn에만 적용한다.
- USER Driving PASS는 실제 PIE transient 적용 성공 뒤 explicit confirmation으로만 기록하며 RecipeId + TargetHash + RunId exact token이 current truth와 일치할 때만 Step 8 Complete다. 새 RunId/Target drift는 이전 PASS를 무효화한다.
- Final Official Build `055dbddf74f342e9b10a92e1fb2542b8` PASS, focused `BuilderStep8Driving` `aa2a365599fc46d484857894aa0df47a` 1/1 PASS, runner envelope actual canary `6135fbff338c47f8944dd5b8b02d5069` PASS. Step1~7 suite는 반복하지 않았다.
- canary가 덮어쓴 canonical Saved metric JSON은 기존 `DefenseSUV_BaseMass_Latest` 내용으로 복원했다. single-slot benchmark log는 마지막 canary 로그를 유지하며 과거 로그를 추정 생성하지 않았다.
- 다음 Gate는 실제 신규 차량 1대의 Guided Builder E2E + USER Driving Acceptance다. P0-10 Systems promotion은 계속 Pending이다.

### v0.1.19 - 2026-08-27

- VB-P0-09 Step 7 Final Review / explicit DefinitionApply / guarded Undo Guided Flow를 Technical PASS로 닫았다.
- existing `ReadBuilderFinalReview` R0의 Validation/Drift/Gameplay/Provenance/전체 Field Diff를 Guided Shell에 노출하고, USER dialog 직전 fresh review를 exact ProposalHash와 함께 준비한다.
- Apply는 existing `ApplyBuilderFinalReview` R3만 사용하며 backend가 Apply 직전 다시 fresh review해 stale approval을 fail-closed한다. Target VehicleData/Recipe AppliedState는 transaction으로 변경될 수 있으나 auto Save/retry는 0이다.
- Undo는 successful Builder Apply가 발급한 exact current-lifetime token만 사용하고 current Undo top/post-Apply state가 달라지면 arbitrary Undo 없이 차단한다.
- Final Official Build `f9a9ced62f1545f884ceefdfd2a966ab` PASS, 새 focused `BuilderStep7FinalReview` `a7efe7e368cd42f5b57a18c9b66f756f` 1/1 PASS. Step1~6 suite는 반복하지 않았다.
- 다음 Gate는 Step 8 Technical Driving + USER Driving Guided connection이며 USER E2E/P0-10 Systems promotion은 Pending이다.

### v0.1.18 - 2026-08-27

- VB-P0-09 Step 6 Gameplay Setup Guided Flow를 Technical PASS로 닫았다.
- 기존 `ReadBuilderGameplayGuidance` R0 authority를 그대로 재사용해 Durability/Defense/DestroyedFx/Hardpoints/MountProfiles/DriveState/WheelVisual/FittingMass 8영역 completeness를 Guided Shell에 연결했다.
- backend `Blocked/NeedsReview/bCanCompleteGameplayStep`를 Shell `Blocked/Ready/Complete`에 대응시키고 persistent Step6 flag 없이 fresh truth에서 derive한다.
- USER Socket authority를 보존해 manual Socket guidance만 표시하며 생성·이동은 하지 않는다. Pending Gameplay Diff는 Step 7 Apply 후보로 표시만 한다.
- Official Build `d01056d228214ee8bbd06b154768e96d` PASS. `VB_P0_06.GameplayGuidance`와 확장된 `BuilderStep5Physics` Step6 integration/resume/stale prerequisite 회귀가 PASS했다.
- broad 72건 중 unrelated `DAUTH_P0_08.Batch.AllowlistProjection` stale 79→89 leaf-count assertion 1건은 별도 유지한다. 다음 Gate는 Step 7 Final Review / explicit Apply Guided connection이다.

### v0.1.17 - 2026-08-27

- VB-P0-09 Step 5 Physics Proposal Guided Flow를 Technical PASS로 닫았다.
- transient `FCFBuilderPhysicsDraft`를 current Recipe/Target/USER-reviewed Evidence에 exact binding하고 complete Builder-private 4 Profile typed payload + consumed Claim set을 Guided Shell에 연결했다.
- mutation destination은 current Recipe exact private 4 Profile binding으로 강제하며 `PreviewBuilderProfiles()` mutation0 → explicit USER AuthoringWrite → existing `CommitBuilderProfiles()` R1 lane만 사용한다.
- Step 5 Complete/Resume/Stale은 persistent `BuilderCommitReceipt` + current Evidence/Claim/private 4 Profile fingerprint/ResolvedDefinitionHash/Resolver revision에서 fresh derive한다. transient Draft와 mutation approval은 restart 뒤 복원하지 않는다.
- Guided Shell은 AI 차량 특성 설명, consumed Claim, Profile domain별 변경 여부와 prospective resolved hash를 보여주며 USER가 Chaos 세부 숫자를 직접 입력하는 정상 flow를 요구하지 않는다.
- Final Official Build `3d65b8e8b6d04f498ceaf6358618c6a0` PASS. broad DataAuthoring 72건에서 `BuilderStep5Physics`, `BuilderStep1Reference`, `BuilderShell`, `BuilderProfileCommit`, `BuilderCompanionFlow` affected PASS를 확인했다.
- broad의 유일한 unrelated `DAUTH_P0_08.Batch.AllowlistProjection` stale 79→89 leaf-count assertion은 별도 regression으로 유지했다.
- Target VehicleData Apply는 Step 7 owner, Save/shared Profile/raw VehicleData/auto Socket·Hardpoint/Git commit·push는 수행하지 않았다. 다음은 Step 6 Gameplay Setup Guided connection이며 USER E2E/P0-10 Systems promotion은 Pending이다.
### v0.1.16 - 2026-08-27

- VB-P0-09 Step 1 Reference Evidence / Companion USER flow를 Technical PASS로 닫았다.
- `FCFBuilderResearchDraft` transient JSON handoff를 exact RecipeId/Target에 binding하고, 새 Evidence를 fixed EvidenceId + complete validated Research payload로만 생성하도록 R2 Companion contract를 강화했다.
- Guided Shell에 Research Draft load, Reference summary, mutation0 Companion review→explicit USER OwnershipWrite commit, exact EvidenceFingerprint local review token UX를 연결했다. token은 Profile/Recipe write·VehicleData Apply·Save 권한이 아니다.
- Step 1 completion은 Recipe/Target/Evidence valid + blocking0 + exact USER fingerprint review로 유지하며 private Profile completeness를 Step 1 authority로 올리지 않았다. Physics numeric review는 Step 5 owner다.
- Final Official Build `06370173f2c64a1ab4dd3d235bd46c18` PASS. broad DataAuthoring에서 `BuilderStep1Reference`, `BuilderShell`, `BuilderCompanionFlow` affected tests PASS를 확인했다.
- broad 71건 중 unrelated `DAUTH_P0_08.Batch.AllowlistProjection` 1건은 typed Profile schema가 이미 79→89 leaf로 확장된 뒤 expectation이 stale한 별도 regression으로 분리했다.
- Product Asset Save / VehicleData Apply / shared Profile mutation / raw VehicleData write / auto Socket·Hardpoint / Git commit·push는 수행하지 않았다. VB-P0-09 전체 USER PASS와 Systems P0-10 승격은 Pending 유지한다.
### v0.1.15 - 2026-08-27

- VB-P0-09 post-integration Step Extensibility Hardening을 Technical PASS로 닫았다. current baseline 8 Step의 semantic identity와 presentation 개수/순서를 분리했다.
- `GetVehicleBuilderStepDefinitions()`를 Step composition/order/title 단일 owner로 두고, `RebuildStepStates()`를 Step별 `Evaluate*Step()` orchestrator로 분해했다. final v1.3.1에서는 `EvaluateStepById()`가 current definition 순서대로 evaluator를 dispatch해 reorder owner도 단일화했다.
- 상태 접근은 stable StepId, navigation은 `StepViews.Num()`, Step-specific UI는 current StepId를 사용하도록 교정해 literal 8과 numeric index 의미 결합을 제거했다.
- focused Automation도 exact-8/index assertion 대신 stable StepId lookup으로 전환했고 기존 Mesh/Socket/Layout baseline·optional-wheel·stale·duplicate-blocker 의미가 유지됨을 재검증했다.
- Final Official Build `8e1c55defcbc49108c4c402964a33477` PASS, focused BuilderShell `0e2b3bd9260446c084e0523427283e02` 1/1 PASS. Product Asset/Socket/Save mutation 0.
- generic plugin/workflow engine을 만들지 않고 explicit StepId + definition + evaluator 구조만 유지한다. VB-P0-09 USER E2E와 Systems 승격 상태는 변경하지 않는다.

### v0.1.14 - 2026-08-27

- VB-P0-09 USER Acceptance 준비로 별도 Guided Builder Shell을 current runtime에 연결하고 Step 2 Mesh / Step 3 Socket / Step 4 Layout read-only evaluator를 `FCFVehicleAssetSnapshot` authority로 구현했다.
- Step 2는 Chassis+FL 최소 필수와 optional FR/RL/RR warning semantics를 보존하고, Step 3은 4-role found+distinct만 hard rule로 사용하며 USER Hardpoint authority와 custom socket 허용을 유지한다.
- Step 4는 current Target persisted `VehicleLayoutConfig`와 fresh AssetSnapshot의 socket identity/location/rotation을 direct exact 비교해 `Ready/Complete/Stale/Blocked`를 파생한다. 별도 capture flag/tolerance/schema는 추가하지 않았다.
- latest Official Build `1b1aa29b80c341c6a6289ac2693413e9` PASS, focused BuilderShell `c6ab98e244634dd1a7fbcb84b290d4e3` 1/1 PASS.
- VB-P0-09 전체 USER PASS는 아직 Pending이며 다음은 Step 1 Reference/Companion 및 Step 5~8 Guided action 연결 뒤 실제 신규 차량 E2E 확인이다. Systems 승격은 P0-10까지 금지 유지한다.

### v0.1.13 - 2026-08-27

- VB-P0-08 post-PASS measurement hardening을 완료했다. Heavy PhysicsState reset과 Light kinematic reset을 분리해 `Heavy reset → settle → Light reset → drive` 순서로 계측 시작 상태를 결정론적으로 고정했다.
- 100→Idle braking은 별도 reset 후 재가속하지 않고 같은 top-speed trajectory에서 100km/h entry를 재확보한 뒤 Brake 1.0을 적용하도록 교정해 phase state divergence를 제거했다.
- latest Official Build `0a8f33824563433ab32097a612f434d9` PASS.
- fitted repeat B/C가 Mass 1570kg / 0→50 3.716667s / 0→100 19.433334s / Peak 129.324310km/h / 100→Idle 2.400002s·32.669586m / Yaw 21.206821deg / Radius 8.517635m로 exact 동일했다.
- latest no-fitting BaseMass canary `0006a45bd3eb456581a2f125ff612d8e`도 Mass1000kg / 0→50 3.9s / Peak89.953712km/h로 PASS했다. 100km/h 미도달 차량의 braking unavailable은 정상 결과다.
- P0-08 Technical PASS와 next `VB-P0-09 End-to-End USER Acceptance`는 유지한다.

### v0.1.12 - 2026-08-27

- `VB-P0-08 Technical Driving Benchmark`를 Harness Technical PASS로 닫았다. RuntimeRead가 arbitrary Chaos getter/function 실행을 제공하지 않는 현재 경계를 존중하고 Product debug surface를 추가하지 않은 채 기존 Chaos mobility 계측 패턴을 arbitrary saved VehicleData용 Automation으로 일반화했다.
- `CFVehicleBenchTests.cpp`와 `RunBuilderBench.ps1`을 추가해 fresh PIE deferred vehicle spawn, optional Fitting total mass / no-fitting BaseVehicleMassKg, 0→50/100, peak speed, RPM/gear, 100→Idle braking, steady-yaw, effective turning radius를 machine-readable 결과로 기록한다.
- 반복 계측 편차를 추적해 phase PhysicsState rebuild, explicit first gear, map fixture 격리, active-phase-only peak telemetry를 적용했고 UE 5.8 Source-confirmed `-UseFixedTimeStep -FPS=60`으로 최종 결정론적 측정 조건을 고정했다.
- fitted Fixed60 A/B가 0→50 3.716667s / Peak 92.998657km/h / Yaw 21.206406deg / Radius 8.502187m로 exact 반복됐고, no-fitting BaseMass canary도 Mass1000kg / 0→50 3.9s / Peak89.895355km/h로 PASS했다.
- representative canary가 100km/h 미도달이라 100→Idle braking branch는 runtime 미관측이며 이를 기술 실패로 포장하지 않는다. Reference 적합성 및 USER 주행감은 `VB-P0-09`에서 검증한다.
- latest Official Build `02090e05471a4af4afd7a0d98d01542a` PASS, production Asset/Socket/Save mutation 0을 유지했다. 다음 Gate는 `VB-P0-09 End-to-End USER Acceptance`다.

### v0.1.11 - 2026-08-27

- VB-P0-05~07 중간 코드검수에서 발견된 provenance/Undo/direct-test/final-readback/Resolver-중복 P1/P2를 post-review hardening으로 교정했다.
- `BuilderCommitReceipt`를 Recipe의 non-semantic persistent metadata로 추가해 accepted Evidence/Claim set과 실제 committed private 4 Profile fingerprint를 연결했다. 동일 Profile payload에 receipt만 없는 기존 차량은 receipt-only R1 commit으로 보완하며 Profile payload/revision과 VehicleData는 건드리지 않는다.
- `CommitBuilderProfiles()`에 assignment/final fingerprint readback, `PostEditChange`, rollback을 추가하고 Profile/Companion prospective resolver가 transient UObject path를 proposal authority로 사용하던 비결정성을 persistent prospective path + typed snapshot 방식으로 제거했다.
- `ReadBuilderFinalReview()`는 one fresh Resolve에서 Validation/Drift/Gameplay/Diff를 projection하고, provenance는 persistent receipt + current private 4 Profile과 exact 일치할 때만 인정한다.
- `UndoBuilderFinalApply()`는 exact top transaction뿐 아니라 current semantic Recipe + post-Apply Target/AppliedState exact state를 Undo 직전 확인해 transaction 밖 drift도 `StateChanged`로 차단한다.
- latest Official Build `306b9c9be7f648a4b5a89b6eb13c7f05` PASS, sequential focused VB-P0-05 4/4 + VB-P0-06 1/1 + VB-P0-07 1/1 PASS를 확보했다. production Asset/Socket/Save mutation 0을 유지했다.
- next gate는 기존대로 `VB-P0-08 Technical Driving Benchmark`이며 VB-P0-00~07 PASS replay 금지는 유지한다.

### v0.1.10 - 2026-08-27

- `VB-P0-07 Final Review / Validation / Undo`를 Technical PASS로 닫았다.
- existing Resolver/`UCFVDAValidator` Validation/External Drift/P0-06 Gameplay guidance/Diff를 R0 `ReadBuilderFinalReview()`에서 aggregate하고 Warning/Blocker/Target Diff와 fresh Apply readiness를 한 결과로 제공하도록 구현했다.
- current schema가 field→claim provenance map을 persist하지 않으므로 FACT/DERIVED/GAME_BIAS를 field count로 과장하지 않고 latest accepted Evidence binding의 **consumed canonical Claim count**로 정확히 표시하도록 했다.
- `ApplyBuilderFinalReview()`는 fresh Final Review와 exact `DefinitionApply` scope를 재검사한 뒤 기존 `ApplyResolvedVehicle()` → `FCFVehicleApplyService` 단일 writer lane만 사용한다. raw VehicleData write, shared Profile mutation, auto Save/retry는 추가하지 않았다.
- `UndoBuilderFinalApply()`는 current lifetime에서 Builder가 발급한 exact UE TransactionId와 UndoScopeHash를 요구하고 current Undo stack top이 다르면 `StateChanged`로 차단한다. 성공 Undo 뒤 Target full hash와 Recipe AppliedState를 pre-Apply evidence와 exact 비교한다.
- final Official Build `ff4dd50981464710876a0693d2287432` PASS와 focused `CarFight.DataAuthoring.CF_FQ_040.VB_P0_07.FinalReviewApplyUndo` 1/1 PASS를 확보했다. Hardpoint USER Socket authority와 production Save0를 유지한다.
- 다음 Gate를 `VB-P0-08 Technical Driving Benchmark`로 전진했다.

### v0.1.9 - 2026-08-27

- `VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`를 Technical PASS로 닫았다.
- 기존 Recipe/Profile Snapshot/AssetReader/Pure Resolver/current Target 위에 R0 `ReadBuilderGameplayGuidance()`를 추가하고 Durability/Defense/DestroyedFx/Hardpoint/Mount/DriveState/WheelVisual/FittingMass 8영역을 Complete/Optional/NeedsReview/Blocked로 판정하도록 했다.
- Hardpoint 실제 Socket 위치는 USER authority로 유지했다. NewVehicle missing Socket은 수동 작업 guidance만 제공하고 자동배치하지 않으며, Existing Vehicle Completion은 current stored LocalTransform이 current candidate와 충돌하지 않으면 강제 Socket migration 없이 보존한다.
- Defense/Destroyed FX/Fitting 미사용 상태를 current optional contract로 인정하고 임의 Asset/수치 생성을 금지했다. Existing Hardpoint/Mount가 candidate와 다르면 자동 보존 추정 대신 NeedsReview로 fail-safe 처리한다.
- final Official Build `12e537e016b64a179a813a4929f584e8` PASS와 focused `CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance` 1/1 PASS를 확보했다. production Asset/Socket/Save mutation은 0이다.
- 다음 Gate를 `VB-P0-07 Final Review / Validation / Undo`로 전진했다.

### v0.1.8 - 2026-08-26

- `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow`를 Technical PASS로 닫았다.
- VehicleData ChassisWidth/Transmission typed schema, atomic `TransmissionRatios`, Registry127/Resolver rev2, Evidence DataAsset + SHA-256/NFC, Evidence-bound private 4 Profile atomic commit과 lifecycle-safe runtime apply를 구현했다.
- Builder companion creation과 `Existing Vehicle Completion`을 정식 경로로 통합해 existing valid companion 보존 / Missing만 생성 / shared·foreign owner Conflict / existing resolved baseline preservation을 고정했다.
- final Official BuildEditor PASS, latest focused Automation terminal 2/2 PASS(exit 0), persisted `CFVehicleData` legacy Transmission audit 11/11 PASS, PhysicsState lifecycle Source audit PASS를 확보했다.
- production VehicleData/Profile/Evidence Asset에 실제 commit/apply/save를 수행하지 않았고 next gate를 `VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`로 전진했다.

### v0.1.7 - 2026-08-26

- VB-P0-04 final source closure에서 `VehicleBuilderProposalSpec.md v0.1.1`로 owner를 전진했다.
- `D:\UnrealEngine_Source` UE 5.8 exact local Source를 read-only 확인해 Reverse ratio storage convention을 `positive magnitude 저장 / GetGearRatio에서 negative direction 적용`으로 확정했다.
- v0.1.0 Proposal Spec의 negative-array 선확정을 폐기하고 VB-P0-04를 Source uncertainty 0 / TRUE PASS로 닫았다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation 0을 유지하며 next gate는 `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow` 그대로다.

### v0.1.6 - 2026-08-26

- `VB-P0-04 Reference → Authoring Proposal Bridge`를 Design PASS로 닫고 상세 owner `VehicleBuilderProposalSpec.md v0.1.0`을 추가했다.
- accepted Reference Evidence를 Builder-private VehicleBase/Drivetrain/Handling/Performance 4 Profile의 **complete prospective payload**로 변환하고 sparse Profile을 금지했다.
- FACT/DERIVED/GAME_BIAS 외에 existing value를 `BaselineInherited`, exact 표현 불가를 `UnsupportedExact`, future schema 필요를 `PlannedTypedGap`로 구분해 provenance를 field 단위로 보존하도록 했다.
- Feel physical Reference는 Neutral anchor로만 사용하고 Low/High를 FACT로 복제하지 않으며, 기존 envelope가 없을 때만 Reference-locked equal envelope를 explicit GAME_BIAS로 제안하도록 했다.
- UE 5.8 `FVehicleTransmissionConfig`, `Chaos::FSimpleTransmissionConfig`, `Chaos::ETransmissionType`을 exact Source/API 기준으로 대조해 automatic/manual, auto reverse, forward/reverse ratios, final ratio, shift RPM, change time, efficiency mapping을 확정했다.
- Transmission architecture와 Chaos shift-control mode를 분리하고 4AT/6MT의 gear count는 Evidence validation semantic으로 사용하며 ratio 자체가 없으면 임의 생성하지 않도록 했다.
- primitive ratio array를 독립 generic leaf로 노출하지 않고 dedicated typed atomic ratio-set으로 묶는 future schema를 확정하고 gear count는 array length에서 derive하도록 했다.
- current `FCFVehicleMovementConfig`/Drivetrain Profile/`ApplyVehicleMovementConfig()`의 Transmission gap, VehicleBase의 ChassisWidth·Reference tire-derived wheel geometry fallback gap을 VB-P0-05 구현 의무로 기록했다.
- 2027 Kia Morning 1.0 4AT dry-run에서 975kg/FWD/4AT/95.124505Nm는 mapping하고 GVWR/gear ratios/final drive/redline Unknown은 보존했다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation 0, Implementation 0을 유지하고 next gate를 VB-P0-05로 전진했다.

### v0.1.5 - 2026-08-26

- `VB-P0-03 Mesh & Socket Guidance`의 read-only evaluator/blocker/stale 계약을 `VehicleBuilderShellSpec.md v0.1.2`에 고정하고 Design PASS로 전진했다.
- current `FCFVehicleAssetReader / FCFVehicleAssetSnapshot`을 Mesh/Socket authority로 재사용하고 새 Builder Asset scanner/병렬 Validator는 만들지 않기로 했다.
- Chassis + primary Wheel 준비, Wheel 4-role existing/distinct binding, custom Wheel socket 허용, Hardpoint/Destroyed FX warning/requiredness boundary를 고정했다.
- socket-derived Wheelbase/Track은 Reference와 Delta를 보여주되 임의 threshold로 Block하지 않고 USER review Warning으로 유지했다.
- LayoutCapture completion은 persistent progress flag가 아니라 current socket transform과 persisted `VehicleLayoutConfig`의 fresh equality에서 derive하고 mismatch를 Stale로 처리하도록 했다.
- 기존 `CaptureLayoutFromChassisSockets()`는 mutation이므로 VB-P0-03 evaluator가 호출하지 않으며 후속 apply flow로 보존했다.
- C++/UE Asset/schema/capture/save mutation 없이 Implementation 0을 유지하고 next gate를 VB-P0-04로 전진했다.
- Transmission numeric schema와 UE 5.8 Chaos `TransmissionSetup` mapping은 VB-P0-04 전까지 미확정 상태를 유지했다.

### v0.1.4 - 2026-08-26

- 실제 다단 변속기 Authoring 요구사항을 별도 Feature가 아니라 Guided Vehicle Builder의 Builder-private `UCFDrivetrainProfile` 정식 범위로 통합했다.
- Reference Evidence → AI Drivetrain Proposal → private Drivetrain Profile → 향후 UE 5.8 Chaos `TransmissionSetup` mapping의 책임 경계를 고정했다.
- Transmission type/automatic 여부, forward gear count/ratios, reverse ratio, final drive, up/downshift 조건, gear change time, efficiency를 required semantic scope로 추가했다.
- current Source의 `FCFVehicleMovementConfig`, `FCFDrivetrainProfileData`, `ApplyVehicleMovementConfig()`에 Transmission vehicle-specific authoring/apply가 없는 gap을 기록하되 schema/API mapping 확정과 구현은 VB-P0-04로 유지했다.
- Engine/Torque/RPM은 Performance Profile, Differential/Transmission은 Drivetrain Profile이라는 4-Profile ownership 경계를 명확히 했다.
- HUD Gear는 실제 Chaos Runtime `GetCurrentGear()` authority를 유지하며 fake gear 계산을 금지했다.
- VB-P0-02 PASS와 현재 next gate `VB-P0-03 Mesh & Socket Guidance`는 변경하지 않았고 C++/Asset/Runtime mutation은 수행하지 않았다.

### v0.1.3 - 2026-08-26

- `VB-P0-02 Builder Shell / Step State / Resume`를 Design / Shell-State-Resume Contract PASS로 닫았다.
- `VehicleBuilderShellSpec.md v0.1.0`을 상세 owner로 추가하고 completion truth를 persistent progress boolean이 아니라 current authoritative Asset truth에서 fresh derive하도록 고정했다.
- Editor restart resume convenience를 local per-project Editor settings로 분리하고 managed primary identity를 persistent `RecipeId`로 고정했다.
- 8개 fixed StepId와 `Unavailable/Locked/Ready/Complete/Blocked/Stale` 상태, fresh Next evaluation, dependency-specific stale propagation과 read-only Builder context를 고정했다.
- Reference review token을 EvidenceId/Fingerprint/RecipeId에 binding하되 mutation approval과 분리하고 prepared approval/Undo/cache는 restart 뒤 전부 폐기하도록 했다.
- C++/UE Asset/schema mutation 없이 Implementation 0을 유지하고 다음 Gate를 `VB-P0-03 Mesh & Socket Guidance`로 전진했다. `VB-P0-04` numeric mapping은 미착수 상태를 유지한다.

### v0.1.2 - 2026-08-26

- `VB-P0-01 Reference Research & Evidence Contract`를 Design / Research Normalization PASS로 닫았다.
- `VehicleRefEvidenceSpec.md v0.1.0`에 Editor-only companion Evidence의 exact logical schema, Reference identity, citation/origin, FACT/DERIVED/GAME_BIAS, Unknown, conflict/confidence와 SHA-256 fingerprint 계약을 고정했다.
- Builder Proposal을 EvidenceFingerprint + 기존 Recipe/Target/private Profile fingerprint + ProposalHash에 binding하는 stale-safe 계약을 고정했다.
- 2027 Kia Morning 1.0 gasoline Trendy 14-inch를 실제 research normalization dry-run으로 사용해 14/16-inch variant split, Unknown 보존, unit normalization과 deterministic fingerprint semantics를 검증했다.
- C++/UE Asset/VehicleData/Profile/Recipe mutation 없이 Implementation 0을 유지하고 다음 Gate를 `VB-P0-02 Builder Shell / Step State / Resume`로 전진했다.

### v0.1.1 - 2026-08-26

- `VB-P0-00 Current Vehicle Creation Contract Audit`을 read-only Source/Systems 감사로 PASS했다.
- Reference evidence persisted owner를 별도 Editor-only companion DataAsset으로 확정했다. Runtime VehicleData/Recipe 본문/JSON sidecar는 evidence owner로 사용하지 않는다.
- AI vehicle-specific numeric owner를 기존 Editor-only VehicleBase/Drivetrain/Handling/Performance Profile의 Builder-private instance 4종으로 확정했다. Shared Profile AI mutation과 raw field writer는 계속 금지한다.
- Builder 신규 차량 최소 생성 Asset을 VehicleData + Recipe + Reference Evidence + private Profile 4종 = 총 7개로 확정했다. DriveState/Fitting/Defense/Fx/Equipment/Pawn BP는 최소 집합에서 제외한다.
- current `CreateVehicleRecords`가 Definition+Recipe만 생성하고 모든 Profile inference가 0인 current contract를 보존하기로 했다.
- 기존 `UCFVDAValidator`, Resolver/ApplyService, optional Fitting fallback과 existing GoPyMCP `ue.call_write` transport를 재사용하며 새 top-level MCP tool은 추가하지 않기로 했다.
- 다음 Gate를 `VB-P0-01 Reference Research & Evidence Contract`로 전진했다.

### v0.1.0 - 2026-08-26

- USER 승인에 따라 Guided Vehicle Builder의 최초 Design Baseline을 고정했다.
- 실존 차량 1~5대의 공개 Reference fact를 AI가 조사·교차검증하고, FACT/DERIVED/GAME_BIAS provenance를 유지한 VehicleData proposal로 변환하는 방향을 고정했다.
- 기존 Data Authoring을 Backend/Advanced UI로 재배치하고 VehicleData를 Runtime Authority로 유지한다.
- Wheel center/Wheelbase 자동검출과 Hardpoint 자동배치는 ROI가 낮아 P0 Scope Out했다. USER가 Mesh와 Socket을 직접 준비하고 Builder가 현재 필요한 네이밍·위치 기준·검증을 제공한다.
- Wheel 기본 Socket `Wheel_Anchor_FL/FR/RL/RR`, Hardpoint `LocationSlotId` 예 `Front_01/Top_01`, Socket 예 `HP_Front_01/HP_Top_01`, Destroyed FX `FX_Destroyed`의 current Source contract를 기준으로 삼았다.
- UE 내부 LLM/Web crawler는 추가하지 않고 ChatGPT research → typed proposal → Builder review → Data Authoring apply 구조를 채택했다.
- 첫 구현 Gate를 `VB-P0-00 Current Vehicle Creation Contract Audit`으로 설정했다.

---

## 18. Migration

- CF-FQ-038 Data Authoring의 기존 Technical/USER evidence는 유지한다. Vehicle Builder 착수 때문에 UA-01~08이나 Deprecated Gate를 replay하지 않는다.
- 기존 `CarFight 차량 데이터 제작` Workspace는 삭제하지 않는다. 향후 Advanced/maintenance surface로 사용할 수 있다.
- 기존 4축 Driving Feel은 삭제하지 않고 Reference 결과의 optional bias/Quick Tune으로 재분류한다.
- 기존 VehicleData/VehiclePawn/Fitting runtime은 Builder 구현 전 현재 Current System을 그대로 유지한다.
