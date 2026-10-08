# CarFight Vehicle Builder Proposal Spec

- 문서 버전: v0.1.12
- 작성일: 2026-09-01
- 문서 상태: VB-P0-04 Source Closure PASS / VB-P0-09 Builder-wide Transmission Contract PASS / ESH-01 TorqueCurve Typed Owner Technical PASS / ESH-02 Engine Curve Proposal Contract Technical PASS / Actual Wagon USER Review + Profile/Receipt Persistent Commit PASS / Target DefinitionApply Review Ready / ESH-03 WheelTorqueCrossoverShift Not Started
- Feature: `CF-FQ-040 Guided Vehicle Builder`
- 상위 계획: `VehicleBuilderPlan.md`
- 적용 Gate: `VB-P0-04 Reference → Authoring Proposal Bridge`
- 선행 계약: `VehicleRefEvidenceSpec.md v0.1.3`, `VehicleBuilderShellSpec.md v0.1.14`
- 대상 엔진: Unreal Engine 5.8 Source Build
- 역할: accepted Reference Evidence를 Builder-private VehicleBase/Drivetrain/Handling/Performance Profile 4종의 complete typed proposal로 변환하는 provenance, mapping, stale/readiness와 UE 5.8 Chaos Engine/Transmission mapping 계약을 고정한다.

---

## 1. 목적과 범위

VB-P0-04는 AI가 조사한 실차 데이터를 곧바로 VehicleData raw field에 쓰는 기능이 아니다.

정상 데이터 흐름은 다음으로 고정한다.

```text
Accepted Reference Evidence
→ normalized vehicle facts
→ transient Builder typed proposal
→ Builder-private Profile 4종의 prospective payload
→ 기존 Resolver / Diff / SourceTrace / Validation
→ VB-P0-05 explicit reviewed commit/apply
```

이번 Gate가 확정하는 것:

```text
1. Proposal identity / fingerprint / stale binding
2. complete profile seed contract
3. FACT / DERIVED / GAME_BIAS / inherited value 분리
4. VehicleBase / Drivetrain / Handling / Performance mapping
5. Feel response의 Neutral / Low / High ownership
6. UE 5.8 Chaos Transmission semantic/schema/API mapping
7. 현재 Source에 필요한 future typed schema delta
8. commit/apply 전 readiness/blocker
```

이번 Gate가 하지 않는 것:

```text
C++ 구현 0
UE Asset 생성 0
Profile 생성/저장 0
Profile commit 0
Recipe mutation 0
VehicleData Apply 0
Save 0
Editor/PIE 0
Build/Automation 0
```

---

## 2. Authority와 입력

Proposal은 다음 current facts를 fresh read해서 만든다.

```text
Evidence authority
= accepted EvidenceId + EvidenceFingerprint
+ normalized ClaimId / provenance / confidence / unit / Unknown

Recipe authority
= RecipeId + RecipeFingerprint
+ 현재 4 Profile binding identity

Target authority
= current Target DefinitionHash / drift state

Profile seed authority
= 현재 bound Profile fingerprint set
+ 현재 Resolver effective values

선택적 physical evidence
= VB-P0-03에서 실제 mapping이 소비하는 Asset measurement/fingerprint만
```

Reference Evidence는 Proposal 근거이지 Resolver의 직접 runtime source가 아니다.

EvidenceFingerprint가 accepted review token과 다르면 Proposal을 생성하거나 재사용하지 않는다.

---

## 3. Proposal envelope — transient only

향후 logical Proposal은 다음 identity를 가진다. 이 구조는 이번 Gate의 설계 계약이며 아직 persistent schema를 만들지 않는다.

```text
ProposalId
ProposalRevision
MappingContractRevision = VB-P0-04.v1

EvidenceId
EvidenceFingerprint
RecipeId
RecipeFingerprint
TargetDefinitionHash
ProfileSeedFingerprintSet
ConsumedAssetFingerprintSet

VehicleBaseProposal
DrivetrainProposal
HandlingProposal
PerformanceProposal

Issues[]
ProposalHash
```

각 field proposal의 최소 의미:

```text
SemanticKey
TargetDomain
TargetFieldPath
ValueType
ProposedValue

Disposition
= EvidenceDirect
| EvidenceDerived
| BaselineInherited
| GameBias
| PlannedTypedGap
| UnsupportedExact

EvidenceClaimIds[]
ConversionRuleId
Confidence
ReviewRequired
BlockReason
```

`ProposalHash`는 위 identity와 canonical typed payload, provenance/disposition을 함께 hash한다.

숫자만 같아도 provenance 또는 conversion rule이 달라지면 같은 Proposal로 보지 않는다.

---

## 4. Sparse Profile 금지 — Complete Seed Contract

현재 Resolver R2는 Profile이 존재하면 해당 domain의 typed payload를 primary source로 사용한다. Feel field도 `FCFFeelResponse.NeutralValue`가 R2 baseline이 된다.

따라서 Builder-private Profile을 Evidence가 있는 필드 몇 개만 채운 sparse payload로 만들면 constructor default/0이 기존 Project/Target 값을 덮을 수 있다.

**Builder Proposal은 항상 complete prospective Profile payload를 만든다.**

Seed 우선순위:

```text
1. 현재 동일 Recipe가 이미 Builder-private Profile을 사용
   → 그 exact typed payload clone

2. 현재 shared/external Profile을 사용
   → exact typed payload를 transient private proposal로 clone
   → shared Profile mutation 0

3. Profile이 없음
   → current Resolver effective target values로 complete payload seed
```

3번에서 Feel field는 current effective scalar밖에 authority가 없으므로 처음에는:

```text
LowValue = current effective value
NeutralValue = current effective value
HighValue = current effective value
```

로 constant seed한다.

이 seed는 Reference fact가 아니라 `BaselineInherited`이다.

`TorqueMassScale`, `SuspensionMassScale` 같은 authoring helper rule은 existing Profile clone이면 그대로 보존한다. 새 Profile seed에서는 physical Reference anchor가 몰래 재스케일되지 않도록 기본 비활성 상태로 제안하며 이는 `GameBias: DisableImplicitMassScaling@1`로 표시한다.

---

## 5. Feel Response mapping 계약

현재 Handling/Performance의 주요 필드는 `FCFFeelResponse = Low / Neutral / High` 구조다.

Reference fact를 세 값에 모두 FACT라고 복제하는 것은 금지한다.

### 5.1 Neutral authority

실차 Reference에서 직접/파생 가능한 물리값은 **NeutralValue**를 기준 anchor로 쓴다.

예:

```text
실차 Peak Torque 95.1 Nm
→ EngineMaxTorqueByFeel.NeutralValue = 95.1
```

### 5.2 Low / High

기본 규칙:

```text
기존 Profile clone이 존재
→ Low / High를 기존 값 그대로 BaselineInherited
→ Neutral만 Reference mapping

새 Profile / 기존 envelope authority 없음
→ Reference-mapped Feel field는 Low = Neutral = High
→ Low/High disposition = GameBias
→ Rule = ReferenceLockedEnvelope@1
```

`ReferenceLockedEnvelope@1`은 임의 ±10% 같은 숫자를 만들어내지 않기 위한 P0 기본값이다.

후속 Quick Tune에서 차량별 Low/High를 만들 수 있지만 그 값은 Reference FACT로 승격하지 않는다.

기존 envelope를 보존한 상태에서 새 Neutral이 Low/High 범위를 명백히 벗어나면 `Feel.EnvelopeReviewRequired` Warning을 낸다. 자동 비율 재스케일은 하지 않는다.

---

## 6. VehicleBase Profile mapping

### 6.1 현재 직접/파생 가능한 항목

| Reference semantic | VehicleBase proposal | 분류 | 규칙 |
| --- | --- | --- | --- |
| Curb Mass kg | `BaseVehicleMassKg` | DERIVED | `CurbMassToBaseVehicleMass@1` — stock trim을 게임 장비 추가 전 baseline으로 사용한다는 assumption 명시 |
| GVWR / Maximum Permissible Mass kg | `MaximumGrossMassKg` | FACT/DERIVED | 단위 정규화 후 직접 대응 |
| Overall Height mm | `ChassisHeight` cm | DERIVED | `BodyHeightToChaosDragHeight@1` |
| Overall Width mm | future `ChassisWidth` cm | DERIVED / PlannedTypedGap | `BodyWidthToChaosDragWidth@1` |
| Tire size | future Front/Rear Wheel Radius/Width fallback | DERIVED / PlannedTypedGap | `TireCodeToWheelGeometry@1` |

`ChassisHeight/Width`는 실차 외곽 치수를 Chaos drag box 입력으로 사용하는 semantic adaptation이므로 FACT가 아니라 DERIVED다.

Overall Length는 current simple Chaos drag setup의 VehicleBuilder target field가 아니므로 진단 Reference로만 유지한다.

### 6.2 Mass pair blocker

현재 Validator는 `BaseVehicleMassKg`와 `MaximumGrossMassKg` 중 한쪽만 설정된 상태를 Error로 본다.

따라서 Curb Mass만 조사됐고 Max Gross가 0/Unknown이면:

```text
BaseVehicleMassKg proposal 생성 가능
하지만 VehicleBaseProfile TechnicalCommitReady = false
Issue = Mass.MaximumGrossRequired
```

USER가 명시적으로 GAME_BIAS MaximumGrossMass를 승인하거나 실제 GVWR Evidence가 확보되기 전 975/0 같은 invalid pair를 commit하지 않는다.

### 6.3 Wheel/Tire ownership delta

현재 runtime `VehicleMovementConfig`에는 Wheel Radius/Width가 있지만 Registry owner는 `AssetMeasurementProposal`이며 VehicleBase Profile에는 numeric Radius/Width가 없다.

Reference tire size를 Builder-private Profile의 persistent physical fallback으로 쓰기 위해 VB-P0-05 구현 시 다음 typed delta가 필요하다.

```text
FCFVehicleBaseProfileData future fields
- FrontWheelRadius
- RearWheelRadius
- FrontWheelWidth
- RearWheelWidth

Registry ownership
- PrimaryProfileDomain = VehicleBase
- Asset-derived accepted measurement가 있으면 그것이 reference fallback보다 우선
- current BaseProfileMeasurementPolicy semantics 재사용 가능 여부를 implementation test로 확인
```

즉 실제 Mesh measurement가 있으면 Mesh truth를 이긴다는 뜻이 아니라, **Asset measurement > Reference tire-derived fallback** 순서를 유지한다.

---

## 7. Drivetrain Profile mapping

### 7.1 Drive layout / differential

| Reference semantic | Drivetrain proposal | 분류 |
| --- | --- | --- |
| FWD | Differential = FrontWheelDrive, Front engine=true, Rear engine=false | FACT semantic mapping |
| RWD | Differential = RearWheelDrive, Front=false, Rear=true | FACT semantic mapping |
| AWD/4WD | Differential = AllWheelDrive, Front=true, Rear=true | FACT semantic mapping |
| Actual front/rear torque split | `FrontRearSplit` | FACT/DERIVED |

UE 5.8 Chaos `FrontRearSplit`은 4W differential에서만 의미가 있다. FWD/RWD에서는 Reference value를 억지로 만들지 않고 seed를 `NotApplicable/BaselineInherited`로 유지한다.

AWD라는 사실만으로 50:50을 만들지 않는다.

WheelClass는 실차 Reference fact가 아니므로 current profile seed를 유지한다.

---

## 8. UE 5.8 exact Transmission contract

### 8.1 Source authority — 2026-08-26 확인

UE 5.8 generated API/source contract 기준:

```text
/Engine/Plugins/Experimental/ChaosVehiclesPlugin/
  Source/ChaosVehicles/Public/ChaosWheeledVehicleMovementComponent.h

FVehicleTransmissionConfig
UChaosWheeledVehicleMovementComponent::TransmissionSetup
```

Core physics contract:

```text
/Engine/Source/Runtime/Experimental/ChaosVehicles/
  ChaosVehiclesCore/Public/TransmissionSystem.h

Chaos::FSimpleTransmissionConfig
Chaos::ETransmissionType
Chaos::FSimpleTransmissionSim
```

`Chaos::ETransmissionType`는 UE 5.8에서 exact하게 다음 두 값만 갖는다.

```text
Manual
Automatic
```

따라서 AT/DCT/AMT/CVT 같은 실차 architecture taxonomy를 Chaos enum 자체에 보존한다고 주장하지 않는다.

### 8.2 Public `FVehicleTransmissionConfig`

UE 5.8 public setup fields:

```text
bUseAutomaticGears      bool
bUseAutoReverse         bool
ChangeDownRPM           float
ChangeUpRPM             float
FinalRatio              float
ForwardGearRatios       TArray<float>
GearChangeTime          float seconds
ReverseGearRatios       TArray<float>
TransmissionEfficiency  float
```

내부 physics config는:

```text
AutoReverse             bool
ChangeDownRPM           uint32
ChangeUpRPM             uint32
FinalDriveRatio         float
ForwardRatios           TArray<float>
GearChangeTime          float
ReverseRatios           TArray<float>
TransmissionEfficiency  float
TransmissionType        ETransmissionType
```

### 8.3 CarFight future typed schema — 확정

Drivetrain Profile과 `FCFVehicleMovementConfig`에 동일 semantic leaf를 제공한다.

```text
bUseAutomaticGears
bUseAutoReverse
TransmissionRatios : typed atomic ratio-set struct
FinalRatio
ChangeUpRPM
ChangeDownRPM
GearChangeTime
TransmissionEfficiency
```

`TransmissionRatios` logical content:

```text
ForwardGearRatios : TArray<float>
ReverseGearRatios : TArray<float>
```

ForwardGearCount는 중복 저장하지 않는다.

```text
ForwardGearCount = ForwardGearRatios.Num()
ReverseGearCount = ReverseGearRatios.Num()
```

Evidence의 gear count는 expected-count validation semantic으로 사용한다.

### 8.4 Ratio array를 typed atomic struct로 묶는 이유

현재 FieldCodec은 generic property ExportText/ImportText를 지원하므로 `TArray<float>` 자체를 직렬화할 수 있다. 그러나 current type signature는 array inner type까지 강하게 표현하지 않고, Registry의 collection 의미는 Stable-ID array와 결합되어 있다.

따라서 P0 신규 Transmission은 primitive array를 여러 generic leaf로 노출하기보다 **dedicated typed ratio-set USTRUCT를 atomic leaf로 취급**하는 것을 canonical 설계로 한다.

VB-P0-05 구현 요구:

```text
- ratio-set USTRUCT를 Registry atomic struct allowlist에 추가
- Profile와 VehicleMovementConfig에 exact same typed `TransmissionRatios` leaf
- FieldCodec roundtrip / Snapshot hash / Registry bidirectional coverage test
```

Proposal UI는 atomic payload 내부의 1단, 2단, ... Reverse 값을 개별 행처럼 보여줄 수 있다. Data Authoring identity는 ratio-set 전체를 하나의 typed field로 취급한다.

### 8.5 CarFight → UE 5.8 mapping

| CarFight semantic | UE 5.8 `TransmissionSetup` | physics 의미 |
| --- | --- | --- |
| `bUseAutomaticGears` | `bUseAutomaticGears` | internal `ETransmissionType::Automatic/Manual` |
| `bUseAutoReverse` | `bUseAutoReverse` | `AutoReverse` |
| `TransmissionRatios.ForwardGearRatios` | `ForwardGearRatios` | `ForwardRatios` |
| `TransmissionRatios.ReverseGearRatios` | `ReverseGearRatios` | `ReverseRatios` |
| `FinalRatio` | `FinalRatio` | `FinalDriveRatio` |
| `ChangeUpRPM` | `ChangeUpRPM` | internal uint32 RPM threshold |
| `ChangeDownRPM` | `ChangeDownRPM` | internal uint32 RPM threshold |
| `GearChangeTime` | `GearChangeTime` | seconds |
| `TransmissionEfficiency` | `TransmissionEfficiency` | drivetrain loss multiplier |

RPM proposal은 유한한 0 이상 값이어야 하고 physics conversion 시 uint32 representable 범위를 벗어나면 Block한다. fractional RPM은 canonical validation/rounding policy가 필요하며 기본 Proposal은 integer-RPM 의미로 정규화한다.

### 8.6 Reverse ratio storage/sign convention — UE 5.8 exact source closure

Evidence normalizer는 reverse ratio의 방향과 magnitude를 분리한다.

```text
Evidence normalized semantic
ReverseGearRatioMagnitude > 0
Direction = Reverse
```

**CarFight/Chaos setup에 저장하는 값도 positive magnitude다.** Reverse 방향의 음수 부호는 ratio 배열에 저장하지 않고 `GetGearRatio()` 계산 시 적용한다.

UE 5.8 exact local Source Build `D:\UnrealEngine_Source` 확인 결과:

```text
ChaosWheeledVehicleMovementComponent.h
- FVehicleTransmissionConfig::InitDefaults()
  ReverseGearRatios.Add(2.86f)
  → positive magnitude 저장

- FVehicleTransmissionConfig::FillTransmissionSetup()
  PTransmissionConfig.ReverseRatios.Add(Ratio)
  → ReverseGearRatios 값을 부호 변경 없이 그대로 복사

- FVehicleTransmissionConfig::GetGearRatio(InGear < 0)
  return -ReverseGearRatios[Abs(InGear)-1] * FinalRatio
  → 실제 reverse direction 부호는 계산 시 적용

TransmissionSystem.cpp
- FSimpleTransmissionSim::GetGearRatio(InGear < 0)
  return -Setup().ReverseRatios[Abs(InGear)-1] * Setup().FinalDriveRatio
  → Physics Core도 동일 convention 사용
```

따라서 canonical mapping은 다음으로 고정한다.

```text
ReferenceReverseRatioMagnitude = abs(source reverse ratio)
CarFight TransmissionRatios.ReverseGearRatios[] = ReferenceReverseRatioMagnitude
UE FVehicleTransmissionConfig.ReverseGearRatios[] = same positive magnitude
Chaos FSimpleTransmissionConfig.ReverseRatios[] = same positive magnitude
Effective reverse gear ratio = -storedMagnitude * FinalRatio
```

RuleId:

```text
ReverseRatioMagnitudeStored@1
```

원문이 `-2.86`처럼 이미 음수로 표기돼도 Evidence normalization에서 `Magnitude=2.86 / Direction=Reverse`로 canonicalize한다. 이후 Builder는 positive magnitude `2.86`을 setup array에 저장한다. **materialization 단계에서 미리 음수로 바꾸면 `GetGearRatio()`가 다시 `-`를 적용해 잘못된 정방향 ratio가 되므로 금지한다.**

Source evidence:

```text
Local UE 5.8 root: D:\UnrealEngine_Source
Movement header exact lines observed: 367-421
Transmission core exact lines observed: 41-52
Read-only process evidence:
- 45492aa2fa1d44b6a9430151bdcddb11
- b10b2068731f4d6fa361bd9345addd46
Engine Source mutation 0
```


### 8.7 Transmission architecture와 control mode 분리

Evidence semantic은 최소 다음 두 축을 분리한다.

```text
TransmissionArchitecture
= ManualGearbox / TorqueConverterAutomatic / DCT / AMT / CVT / SingleSpeed / Other / Unknown

ShiftControlMode
= Manual / Automatic / Unknown
```

Chaos `ETransmissionType` mapping owner는 **ShiftControlMode**다.

예:

```text
4AT
→ Architecture = TorqueConverterAutomatic
→ ShiftControlMode = Automatic
→ ExpectedForwardGearCount = 4

6MT
→ Architecture = ManualGearbox
→ ShiftControlMode = Manual
→ ExpectedForwardGearCount = 6
```

DCT라는 단어만 보고 자동/수동 control을 추정하지 않는다. Source가 automatic operation을 명시하면 Automatic으로 normalize한다.

### 8.8 Architecture fidelity 한계

Simple Chaos Transmission은 Manual/Automatic control과 ratio/shift/loss를 모델링하지만 실차 architecture-specific behavior 전체를 표현하지 않는다.

```text
Torque-converter slip/lock-up
DCT dual-clutch behavior
AMT clutch behavior
CVT continuous ratio control
```

등은 `FVehicleTransmissionConfig`의 P0 exact mapping 대상이 아니다.

따라서:

```text
Conventional AT / DCT / AMT
→ control/ratio mapping은 가능
→ architecture fidelity = Partial

CVT
→ discrete ForwardGearRatios로 exact 표현 불가
→ UnsupportedExact
→ USER가 명시 승인한 GAME_BIAS discrete approximation 전에는 commit-ready 아님

Single Speed
→ 1개 Forward ratio + FinalRatio로 표현 가능
```

### 8.9 Missing transmission fact 정책

금지:

```text
4AT라는 이유만으로 4개 임의 기어비 생성
Peak Torque RPM을 ChangeDownRPM으로 사용
Peak Power RPM을 ChangeUpRPM으로 사용
Top Speed만으로 FinalRatio 역산
AT라는 이유만으로 bUseAutoReverse=true
0.94를 모든 차량의 FACT efficiency로 사용
```

허용:

```text
실제 gear ratio table → direct ratio proposal
실제 final drive → direct FinalRatio
실제 calibrated shift RPM → direct ChangeUp/DownRPM
실제 shift duration → direct GearChangeTime
실제 mechanical efficiency → direct efficiency
```

공개 spec이 없으면 seed 자체는 `BaselineInherited`로 남길 수 있다. USER가 의도적으로 값을 정하면 `GameBias`다. 단, `VehicleSpecificRequired` 신규 Guided Vehicle에서는 Transmission Core의 `BaselineInherited` seed만으로 TechnicalCommitReady 또는 Final Review 완료 상태가 될 수 없다.

### 8.10 Expected gear count contradiction blocker

Evidence가 `4AT`처럼 forward gear count를 확정했는데 seed/proposal ratio array가 4개가 아니면 알려진 실차 사실과 충돌한다.

```text
ExpectedForwardGearCount = 4
ForwardGearRatios.Num() != 4
→ Transmission.GearCountConflict
→ Drivetrain TechnicalCommitReady = false
```

실제 ratios를 더 조사하거나 USER가 명시적으로 GAME_BIAS 4단 ratio-set을 승인해야 한다.

### 8.11 신규 Guided Vehicle의 Compatibility Default leakage 금지 — Wagon E2E incident

2026-08-31 actual Wagon `VB-P0-09` USER E2E에서 기존 `Missing transmission fact` 정책의 실제 허점을 확인했다.

확인된 current truth:

```text
DA_DT_Wagon
- bUseTransmissionConfig = false
- payload 내부 seed/default = Forward 2.85 / 2.02 / 1.35 / 1.00
- FinalRatio = 3.08
- ChangeUpRPM = 4500
- ChangeDownRPM = 2000

Resolver
- bUseTransmissionConfig=false이면 Drivetrain Profile의 Transmission field candidate를 의도적으로 생성하지 않음
- 따라서 해당 target field는 Project Compatibility Default로 fallback

Project Compatibility Default
- Forward 2.85 / 2.02 / 1.35 / 1.00
- FinalRatio 3.08
- ChangeUpRPM 4500
- ChangeDownRPM 2000
```

이 값들은 차량별 적합성 Proposal이 아니라 기존 UE 5.8/CarFight 자산 동작 보존을 위한 compatibility baseline이다.

actual PIE USER evidence:

```text
Wagon automatic transmission
1단 주행
→ 약 80 km/h 부근에서 2단으로 상향 변속
```

이는 `80 km/h`를 runtime shift trigger로 사용한 것이 아니다. 현재 고정 `ChangeUpRPM=4500`에 도달했을 때 1단 ratio × FinalRatio × Wheel Radius 조합의 결과 차속이 이미 약 80 km/h였다는 의미다.

따라서 신규 Guided Vehicle에는 다음 remediation 원칙을 적용한다.

```text
Legacy / 기존 VehicleData
→ bUseTransmissionConfig=false 허용
→ Project Compatibility Default 보존 가능

신규 Guided Vehicle
→ Compatibility Default는 complete payload seed / 호환 fallback으로만 허용
→ 차량별 Transmission이 준비된 최종 상태로 간주 금지
→ exact 실차 ratio가 Unknown이어도 Unknown provenance는 보존
→ 필요한 경우 차량 성격과 Reference를 근거로 vehicle-specific DERIVED/GAME_BIAS Transmission Proposal을 별도 USER review 대상으로 생성
→ Compatibility Default가 조용히 Step 7/8 차량별 Transmission처럼 확정되는 경로 금지
```

P0 Runtime 범위는 확대하지 않는다.

```text
유지
- Chaos Automatic Gears
- 단일 ChangeUpRPM
- 단일 ChangeDownRPM
- vehicle-specific Forward/Reverse ratios
- FinalRatio

현재 Scope Out
- throttle/load별 가변 shift map
- 속도 기반 runtime shift trigger
- TCU 다차원 shift schedule
```

대신 Builder 설계 검증값으로 다음 diagnostic preview를 추가하는 방향을 검토한다.

```text
각 forward gear의 ChangeUpRPM 기준 예상 차속
각 상향 변속 직후 예상 RPM
```

이 차속은 runtime 변속 조건이 아니라 `기어비 + FinalRatio + Wheel Radius + ChangeUpRPM` 조합의 sanity check다. 이번 Wagon처럼 `1→2 예상 변속 차속 ≈ 80 km/h`인 비정상 조합을 PIE 이전에 노출하는 것이 목적이다.

정확한 Block/Warning gate와 Proposal 생성 규칙은 후속 remediation 설계에서 확정한다. 이 기록만으로 임의 ratio/FinalRatio/ShiftRPM을 생성하거나 Target을 수정하지 않는다.

`VB-P0-09 USER Driving PASS`는 이 Drivetrain Proposal gap 교정 및 재검증 전까지 보류한다.

### 8.12 Persistent Guided Transmission policy

`NewVehicle / CompleteExisting`는 persistent 차량 출신 정보가 아니다. Current Builder Source에서 Evidence/private Profile/BuilderCommitReceipt/Import·Apply history가 생기면 원래 Guided 신규 차량도 `CompleteExisting`으로 전환될 수 있으므로 Final Review의 신규 차량 판별 authority로 사용할 수 없다.

따라서 Recipe에 Editor-only persistent policy를 둔다.

```text
BuilderTransmissionPolicy
= LegacyCompatible
| VehicleSpecificRequired
```

정책:

```text
LegacyCompatible
→ 기존 Recipe / Existing Import / Legacy vehicle의 기본값
→ bUseTransmissionConfig=false + Project Compatibility Default 허용
→ 기존 차량 runtime/Resolver 동작 보존

VehicleSpecificRequired
→ Guided Builder의 Mesh-only → VehicleData+Recipe 신규 생성 경로에서만 설정
→ Final Review까지 vehicle-specific Transmission Core가 reviewed proposal로 완료되어야 함
→ Compatibility Default는 seed/runtime compatibility fallback일 뿐 completion authority가 아님
```

기존 Recipe의 migration default는 `LegacyCompatible`다. Generic Resolver와 Project Compatibility Default는 이 policy를 해석하지 않는다. Gate는 Builder Step 5 / Final Review가 소유한다.

actual Wagon은 이 policy 도입 전 Guided Builder로 생성된 E2E 대상이므로 구현 후 exact Wagon Recipe만 one-time `VehicleSpecificRequired`로 승격하고, 이 승격만으로 Transmission 수치를 변경하지 않는다.

### 8.13 Transmission Core / Auxiliary completion contract

신규 Guided Vehicle의 Core:

```text
bUseTransmissionConfig = true
bUseAutomaticGears
bUseAutoReverse
TransmissionRatios.ForwardGearRatios
TransmissionRatios.ReverseGearRatios
FinalRatio
ChangeUpRPM
ChangeDownRPM
```

`VehicleSpecificRequired`에서는 각 Core semantic이 current reviewed Transmission Proposal에 exact binding되고 disposition이 다음 중 하나여야 한다.

```text
EvidenceDirect   = FACT
EvidenceDerived  = DERIVED
GameBias         = USER-reviewed GAME_BIAS
```

`BaselineInherited`만 남은 Core는 Final Review blocker다.

현재 CarFight P0 input contract에는 manual shift가 없으므로 Builder vehicle-specific Transmission은 `bUseAutomaticGears=true`, `bUseAutoReverse=true`를 요구한다. Reference가 manual gearbox라면 이를 FACT로 위장하지 않고 `GAME_BIAS / CarFightAutomaticControlApproximation@1`로 명시한다.

Auxiliary:

```text
GearChangeTime
TransmissionEfficiency
```

은 실차 Evidence가 없을 때 P0에서 `BaselineInherited`를 허용한다. 이 경우 `Transmission.AuxBaselineInherited` Warning을 낼 수 있으나 Core completion을 막지 않는다.

### 8.14 FACT / UNKNOWN / DERIVED / GAME_BIAS Generation Contract

`UNKNOWN`은 provenance enum이 아니다. Current Source처럼 `FCFRefUnknownFact`로 별도 보존한다.

```text
FACT
= 동일 semantic의 실제 공개/검증 데이터
= 단위 normalize와 reverse magnitude normalize는 허용

UNKNOWN
= 조사했으나 값 확정 실패
= 숫자를 만들지 않음
= later GAME_BIAS가 있어도 actual fact는 계속 UNKNOWN

DERIVED
= accepted FACT에서 deterministic 계산만 수행
= ConversionRule/MethodRevision/InputClaimIds가 재현 가능해야 함

GAME_BIAS
= 실차 exact 값이 없지만 게임/Chaos 고정 변속 계약을 만족하도록 의도적으로 설계한 값
= MethodId/Revision/InputClaimIds 또는 UnknownFactIds/MethodParameters 필요
= USER review required
```

DERIVED 허용 예:

```text
실제 Overall Ratio + actual Final Drive → Gear Ratio
실제 Gear Ratio + actual Overall Ratio → Final Drive
실제 speed-per-1000RPM + authoritative Wheel Radius → Overall Ratio
unit/sign normalization
```

DERIVED 금지 예:

```text
4AT라는 taxonomy만으로 ratio-set 생성
Peak Torque RPM을 ChangeDownRPM으로 복사
Peak Power RPM을 ChangeUpRPM으로 복사
차량 중량/차급만 보고 FinalRatio 추정
```

이런 값이 필요하면 모두 explicit GAME_BIAS다.

### 8.15 Field-level Transmission review provenance

Current `BuilderCommitReceipt`의 consumed Claim aggregate만으로는 어떤 Claim이 어느 Transmission field를 결정했는지 증명할 수 없다. Mass/Torque FACT만 소비하고 Transmission은 Compatibility Default인 상태도 aggregate FACT count만으로 구분되지 않는다.

따라서 Physics Draft와 persistent Builder receipt에 Transmission 전용 reviewed metadata를 binding한다.

Logical contract:

```text
TransmissionProposalReview
- SchemaRevision
- ProposalHash
- Components[]
    - SemanticKey
    - Disposition
    - EvidenceClaimIds[]
    - UnknownFactIds[]
    - MethodId
    - MethodRevision
    - MethodParameters

BuilderCommitReceipt
- TransmissionProposalHash
- TransmissionProposalSchemaRevision
```

`TransmissionProposalHash`는 component metadata와 complete Drivetrain Transmission payload를 함께 canonical hash한다. 값이 Compatibility Default와 우연히 같아도 reviewed field provenance/hash가 current Drivetrain fingerprint와 exact binding되면 허용하며, 숫자 equality 자체는 completion 근거가 아니다.

Final Review는 `VehicleSpecificRequired`일 때 current Drivetrain Profile + persistent TransmissionProposalHash를 fresh 검증한다. mismatch/missing이면 `Transmission.ProvenanceUnbound` blocker다.

### 8.16 Fixed-shift GAME_BIAS proposal method

고정 `ChangeUpRPM / ChangeDownRPM` runtime contract를 그대로 유지하면서 exact ratio가 Unknown이면 canonical GAME_BIAS method를 사용한다.

```text
MethodId = FixedShiftTransmissionBias
MethodRevision = 1
```

순서:

```text
1. Forward gear count 결정
   - Evidence가 있으면 FACT expected count
   - 없으면 proposed count 자체가 GAME_BIAS

2. ChangeUp/Down RPM 결정
   - calibrated source가 있으면 FACT
   - ChangeUp FACT가 없고 vehicle-specific Engine Curve + gear ratios가 충분하면 `WheelTorqueCrossoverShift@1`로 gear-pair crossover를 계산하고 공통 threshold를 explicit GAME_BIAS로 선택
   - ChangeDown FACT가 없으면 lugging / torque-band reentry / redline / hysteresis를 고려한 별도 GAME_BIAS
   - Peak Torque RPM / Peak Power RPM / 차급 평균 RPM 단순 복사 금지

3. 각 upshift의 target post-shift RPM 결정
   - GAME_BIAS
   - GearNext / GearCurrent = TargetPostShiftRPM / ChangeUpRPM

4. authoritative Wheel Radius와 intended shift-speed sanity target으로 OverallRatio 계산 가능
   - exact physical input에서 순수 계산이면 DERIVED
   - intended target speed 선택이 포함되면 GAME_BIAS

5. GearRatio × FinalRatio 분해
   - 한 축이 FACT이면 다른 축을 충분한 FACT에서 DERIVED 가능
   - 둘 다 Unknown이면 GearRatio와 FinalRatio 모두 GAME_BIAS

6. Reverse ratio는 별도 provenance로 review
```

Top speed 하나만으로 FinalRatio를 역산해 FACT/DERIVED라고 주장하지 않는다.

### 8.17 Shift kinematics diagnostic contract

Builder Step 5와 Final Review는 다음 read-only diagnostic을 제공한다.

기어 `i`의 ChangeUpRPM 기준 무슬립 이론 차속:

```text
ShiftSpeedKmh_i
= ChangeUpRPM * 2*pi*WheelRadiusCm*60
  / (GearRatio_i * FinalRatio * 100000)
```

`i → i+1` 상향변속 직후 예상 RPM:

```text
PostShiftRPM_i
= ChangeUpRPM * GearRatio_(i+1) / GearRatio_i
```

추가 표시:

```text
OverallRatio = GearRatio * FinalRatio
RpmRetention = NextGearRatio / CurrentGearRatio
DownshiftMarginRPM = PostShiftRPM - ChangeDownRPM
```

이 값은 runtime shift trigger가 아니라 고정 shift threshold 조합의 sanity diagnostic이다. TransmissionEfficiency는 회전비 계산에 사용하지 않는다.

Wheel Radius authority는 WSA가 소유하며 이 diagnostic은 current effective powered-wheel radius를 read-only 소비한다. WSA-P0-07 계약/상태를 변경하지 않는다. Radius를 확정할 수 없으면 ShiftSpeed는 `Unavailable`로 표시하고 Transmission provenance 자체를 그 이유만으로 Block하지 않는다.

Blocker:

```text
Transmission.VehicleSpecificRequired
Transmission.CoreProposalMissing
Transmission.ProvenanceUnbound
Transmission.GearCountConflict
Transmission.RatioOrderInvalid
Transmission.InvalidShiftRPM
Transmission.ShiftThresholdUnreachable
Transmission.PostShiftBelowDownRPM
```

Warning:

```text
Transmission.AuxBaselineInherited
Transmission.ReferenceFidelityPartial
Transmission.ShiftThresholdNearLimit
Transmission.PostShiftNearDownRPM
Transmission.UnusualShiftKinematics
```

`1단 예상 변속 차속이 80km/h`처럼 절대 차속 하나만으로 generic Block threshold를 만들지 않는다. 차량 종류에 따라 긴 1단이 실제로 가능하기 때문이다. 대신 provenance 누락과 물리적 threshold contradiction은 fail-closed한다.

### 8.18 Builder-wide 적용 범위 — Wagon은 incident/regression fixture

이 remediation의 제품 범위는 Wagon 한 대가 아니다.

```text
실제 owner
= Guided Vehicle Builder가 앞으로 생성하는 모든 신규 차량의 Transmission authoring contract

Wagon의 역할
= compatibility fallback leakage를 처음 드러낸 incident vehicle
= policy 도입 이전 Guided vehicle에 대한 one-time migration canary
= generic contract 적용 뒤 실제 E2E/주행 regression vehicle
```

따라서 `VehicleSpecificRequired`와 FACT/DERIVED/GAME_BIAS provenance, review/hash, fixed-shift diagnostic, Step 5/Final Review fail-closed는 Sedan/Wagon/SUV/Truck/Sports 등 특정 차체 형식이나 실제 모델명에 의존해서는 안 된다.

Focused regression도 Wagon Asset을 성공 조건으로 삼지 않는다. 차종 비종속 synthetic fixture에서 다음을 먼저 증명한다.

```text
LegacyCompatible fallback preservation
VehicleSpecificRequired fallback rejection
FACT-backed Core acceptance
DERIVED-backed Core acceptance
Unknown-preserving explicit GAME_BIAS acceptance
BaselineInherited Core rejection
stale/unconsumed provenance rejection
gear-count contradiction rejection
forward-ratio order rejection
invalid ChangeUp/ChangeDown threshold rejection
TransmissionProposalHash deterministic payload binding
Guided creation policy assignment
Step 5 / Step 7 / Step 8 affected-flow preservation
```

Wagon-specific migration과 benchmark는 이 generic contract가 Technical PASS한 뒤 수행하는 실제 regression/acceptance 단계다.

### 8.19 2026-08-31 Builder-wide Transmission Contract Technical PASS

Wagon incident에서 시작한 Transmission remediation은 차종 비종속 Builder contract로 승격하고 Source/Build/Focused Automation까지 닫았다.

최종 fail-closed 범위:

```text
LegacyCompatible
→ 기존 compatibility fallback 보존

VehicleSpecificRequired
→ bUseTransmissionConfig=true 필수
→ Automatic Gears + Auto Reverse current P0 control contract 필수
→ Forward/Reverse ratio set non-empty / finite / positive
→ FinalRatio finite / positive
→ reviewed ExpectedForwardGearCount 필수 + ratio count exact match
→ forward ratio descending order
→ finite ChangeUpRPM > 0
→ finite ChangeDownRPM >= 0
→ ChangeDownRPM < ChangeUpRPM

EvidenceDirect
→ exact Transmission research FactKey
→ FACT Claim
→ proposed payload와 canonical value exact semantic match

EvidenceDerived
→ exact output FactKey
→ DERIVED Claim
→ MethodId/Revision exact binding
→ consumed canonical FACT InputClaimIds 필요
→ derived output과 proposed payload exact semantic match

GameBias
→ MethodId/Revision/Parameters 필요
→ 해당 Transmission semantic의 FACT 또는 Unknown input 필요
→ ForwardGearRatios GAME_BIAS는 GearCount FACT/Unknown origin 추가 필요
→ 실제 fact Unknown은 숫자로 덮어쓰지 않고 UnknownFacts에 계속 보존
```

고정 변속 diagnostic은 runtime shift condition을 추가하지 않고 다음만 read-only 표시한다.

```text
GearRatio
OverallRatio
Shift@ChangeUpRPM no-slip speed
PostShiftRPM
RpmRetention = NextGearRatio / CurrentGearRatio
DownshiftMarginRPM
```

Technical evidence:

```text
Official Build
= 87fc2d61a6f1401fa5402a9fff6f3354
= UHT 4 generated files
= 21/21 compile/link
= Exit 0 / Succeeded

Focused runner
= Tools/RunBuilderTransTests.ps1 v1.0.0
= process f90bccf4ab4c45e8ac3d84a0e604fbea
= 6/6 PASS

PASS set
1. BuilderTransmissionContract
2. BuilderProfileCommit
3. BuilderShell
4. BuilderStep5Physics
5. BuilderStep7FinalReview
6. BuilderStep8Driving
```

`BuilderTransmissionContract`는 Wagon Asset 없이 synthetic vehicle fixture로 semantic/value mismatch, stale DERIVED input, GAME_BIAS gear-count origin 누락, invalid ratio/final/shift payload까지 negative regression한다.

따라서 Builder-wide contract 자체는 Technical PASS다. 다음은 generic source를 더 확장하는 단계가 아니라 **pre-policy actual Wagon을 이 contract에 실제 regression 적용**하는 단계다.

### 8.20 2026-08-31 Source Implementation Checkpoint

위 remediation contract는 Source에 구현됐다.

구현 완료:

```text
Guided Mesh-only 신규 record creation
→ BuilderTransmissionPolicy = VehicleSpecificRequired

Generic/Legacy record creation
→ BuilderTransmissionPolicy = LegacyCompatible

PhysicsDraft
→ SchemaRevision 2
→ TransmissionReview field-level provenance 포함

Step 5 Profile Preview
→ VehicleSpecificRequired + bUseTransmissionConfig=false fail-closed
→ Transmission Core provenance fresh Evidence 검증
→ TransmissionProposalHash 계산
→ fixed-shift diagnostic 표시

BuilderCommitReceipt
→ 승인 당시 TransmissionPolicy
→ TransmissionProposalHash
→ persistent TransmissionReview 저장

Final Review
→ current Drivetrain payload + receipt review/hash fresh 재계산
→ policy raw drift / payload drift / provenance stale fail-closed
→ ShiftSpeed / PostShiftRPM / DownshiftMargin diagnostic 합산
```

공용 private implementation은 `CFBuilderTransUtil.h v1.0.0`이 소유하며 Generic Resolver와 Project Compatibility Default는 변경하지 않았다.

Official Build:

```text
Build Job = fb3ab376111d4c9283afc2731dba3e89
Target = CarFight_ReEditor / Win64 Development
UHT = PASS / 6 generated files
Compile+Link = 22/22 PASS
ExitCode = 0
Result = Succeeded
```

이 checkpoint의 의미는 **Source Implementation + official compile/link readiness PASS**다. 다음은 affected focused Automation이며, Automation PASS 전에는 Technical regression closure로 확대하지 않는다.

actual Wagon에는 아직 다음 mutation을 수행하지 않았다.

```text
BuilderTransmissionPolicy migration 0
Transmission ratio/final/shift RPM 변경 0
PhysicsDraft v2 actual Wagon proposal commit 0
VehicleData re-Apply 0
Technical Benchmark rerun 0
USER Driving recheck 0
```

기존 AssetDump index에서 Wagon Recipe가 private Profile 0/4 + empty receipt로 보인 결과는 current USER Save 상태와 모순되므로 stale evidence로 분류한다. Wagon mutation 전 fresh persisted AssetDump evidence를 다시 확보한다.

---

## 9. Performance Profile mapping

### 9.1 Engine

| Reference semantic | Performance proposal | 분류/제약 |
| --- | --- | --- |
| Peak Torque Nm | `EngineMaxTorqueByFeel.NeutralValue` | FACT direct after unit normalize |
| Actual maximum/limiter RPM | `EngineMaxRPMByFeel.NeutralValue` | FACT/DERIVED only when semantics exact |
| Actual Idle RPM | `EngineIdleRPM` | FACT direct |
| Explicit redline-start RPM | `RedlineStartRPM` | FACT/DERIVED CarFight semantic |
| Cd | `DragCoefficient` | FACT direct |
| measured/specified downforce coefficient | `DownforceCoefficient` | FACT direct if same semantic is established |

금지 mapping:

```text
Peak Power RPM != EngineMaxRPM
Peak Torque RPM != EngineMaxRPM
Max Power kW != EngineMaxTorque
차량 크기만으로 DragCoefficient 추정 금지
spoiler 유무만으로 DownforceCoefficient 추정 금지
```

### 9.2 Vehicle-specific Engine Curve completion contract

UE 5.8 `FVehicleEngineConfig::MaxTorque`는 독립적인 전 RPM 고정 토크가 아니라 `TorqueCurve`의 정규화 값에 곱해지는 scalar다.

따라서 다음 상태를 차량별 엔진 authoring 완료로 간주하지 않는다.

```text
EngineMaxTorque = 실차 FACT
EngineMaxRPM = 실차/DERIVED
TorqueCurve = BP_CFVehiclePawn / Project baseline inherited
```

이 상태에서는 Peak Torque 숫자가 맞아도 RPM별 출력과 고속 가속이 다른 차량의 공용 Curve에 의해 결정된다.

현재 Source가 vehicle-specific TorqueCurve persistent owner를 아직 갖지 않으면 다음 issue를 유지한다.

```text
Issue = Performance.EngineCurveFidelityPartial
Peak torque scalar = Reference-derived
Torque curve shape = BaselineInherited
Technical completion = Partial
```

다음 차량부터 Research/Proposal은 최소한 다음 engine anchors를 먼저 찾는다.

```text
Engine.MaxTorque
Engine.MaxTorqueRpm 또는 TorqueBand
Engine.MaxPower
Engine.MaxPowerRpm 또는 PowerBand
Engine.Redline / limiter RPM이 실제로 공개된 경우
Engine.TorqueCurvePoint[]가 공개된 경우
```

#### 9.2.1 Engine Curve provenance

Curve 생성 우선순위:

```text
1. 실제 RPM별 torque curve / dyno FACT가 충분함
   → EvidenceDirect 또는 단위 normalize DERIVED curve

2. 전체 curve는 없지만
   Peak Torque + Torque Band + Peak Power + Power RPM + Redline 등 충분한 FACT anchor가 있음
   → deterministic bounded DERIVED curve proposal
   → MethodId / Revision / InputClaimIds / interpolation rule 명시

3. Runtime에 curve가 필요하지만 anchor가 부족함
   → BaselineInherited를 vehicle-specific completion으로 위장 금지
   → explicit GAME_BIAS curve proposal + USER Review
```

DERIVED curve는 제조사 원본 torque curve라고 표현하지 않는다. Sparse FACT anchor를 만족하도록 계산한 CarFight용 엔진 근사치다.

2026-09-01 ESH-01/02 implementation checkpoint:

```text
Runtime value owner
= FCFPerformanceProfileData.bUseEngineTorqueCurve
+ FCFPerformanceProfileData.EngineTorqueCurve
→ FCFVehicleMovementConfig
→ ACFVehiclePawn
→ Chaos EngineSetup.TorqueCurve

PhysicsDraft
= schema v3
= EngineCurveReview 포함

Persistent receipt
= EngineCurveProposalHash
+ EngineCurveReview

Fresh validation
= Step 5 Preview/Commit
+ receipt resume
+ Final Review
```

Disposition validation:

```text
EvidenceDirect
= canonical FACT Engine.TorqueCurvePoint[]와 normalized proposed Curve exact semantic/value binding

EvidenceDerived
= Torque magnitude + Torque RPM/band + Power magnitude + Power RPM/band canonical FACT anchor 필수
= MethodId / MethodRevision / MethodParameters 필수

GameBias
= Engine 관련 canonical Evidence 또는 Unknown input 최소 1개
= MethodId / MethodRevision / MethodParameters 필수
= USER Review warning 유지

BaselineInherited
= bUseEngineTorqueCurve=false legacy compatibility에서 Warning만 허용
= enabled vehicle-specific Curve completion 근거로는 금지
```

Actual Wagon ESH-02 proposal:

```text
EngineCurveProposalHash = 773221966499c6295f2a652a21b270a5
MethodId = SparseAnchorEngineCurveBias
Disposition = GameBias
Curve =
900:0.60
1800:1.00
4800:1.00
5400:0.929033824
5700:0.880137307
6500:0.65

FACT anchor
= 350Nm @ 1800–4800rpm
= 250PS @ 5400–5700rpm

GAME_BIAS
= 900rpm low anchor
= 6500rpm tail
= current Profile 6500rpm runtime boundary는 real redline FACT로 승격하지 않음
```

Actual Wagon mutation0 dry-run과 pre-commit affected focused regression exact 4/4가 PASS했다. 이후 USER가 Engine Curve를 승인했고 기존 Builder 4-Profile atomic commit으로 private Profile + Recipe receipt를 persistent 저장했다. commit process `8a5daf3b5b424124a74e6aa8dc5e0c3b` PASS, fresh Editor persisted readback을 포함한 affected regression `a6c99a34f2094f27aa65e30fe3356cd2` exact 5/5 PASS다.

Fresh Final Review는 Resolver revision 5에서 Warning12 / Blocker0 / Diff2이며 exact diff는 `VehicleMovementConfig.bUseEngineTorqueCurve False→True`, `VehicleMovementConfig.EngineTorqueCurve empty→approved 6-point`다. current TargetHash `e7e9d2d79a349068a366334b7368924b`, prospective TargetHash `83c69e52522dc72649c32477b5aad220`, DiffHash `fcdabd8842b82537ba97247698267ed4`, DefinitionApply ProposalHash `809c5523ce23971793b3a937c2d8c1c2`다. Profile commit은 Target Apply 승인이 아니므로 별도 USER DefinitionApply 승인 전에는 Target mutation/save와 ESH-03 ShiftRPM derivation을 수행하지 않는다.

Curve 검증은 최소 다음을 확인한다.

```text
- MaxTorque scalar와 curve peak의 semantic 일치
- Torque band 내 torque retention
- Peak Power RPM에서 P = T * omega 관계가 MaxPower FACT와 합리적으로 일치
- Redline/MaxRPM 부근의 torque falloff가 유한하고 음수가 아님
- curve discontinuity / local spike 없음
```

#### 9.2.2 Full-throttle fixed ChangeUpRPM derivation

현재 P0 Runtime은 gear별/스로틀별 TCU shift map을 구현하지 않고 하나의 `ChangeUpRPM`을 사용한다.

실제 calibrated upshift RPM FACT가 있으면 그것이 우선한다. 없을 때 다음 값을 그대로 복사하는 것은 금지한다.

```text
Peak Torque RPM → ChangeUpRPM 금지
Peak Power RPM → ChangeUpRPM 금지
Torque band end → ChangeUpRPM 금지
차급 평균 RPM → ChangeUpRPM 금지
```

대신 full-throttle acceleration용 candidate는 각 gear pair에서 wheel-torque crossover를 계산한다.

```text
CurrentWheelTorque(RPM)
= EngineTorque(RPM)
  * CurrentGearRatio
  * FinalRatio
  * TransmissionEfficiency

PostShiftRPM(RPM)
= RPM * NextGearRatio / CurrentGearRatio

NextWheelTorque(RPM)
= EngineTorque(PostShiftRPM)
  * NextGearRatio
  * FinalRatio
  * TransmissionEfficiency
```

각 `i→i+1`에서 `NextWheelTorque >= CurrentWheelTorque`가 되는 최초 유효 RPM 또는 redline/usable-power limit를 gear-pair candidate로 삼는다.

P0는 단일 `ChangeUpRPM`만 지원하므로 모든 gear-pair candidate를 그대로 runtime에 넣지 않는다. Builder는 다음 diagnostic을 먼저 표시한다.

```text
GearPair
CrossoverRPM
ShiftSpeedKmh
PostShiftRPM
CurrentWheelTorque
NextWheelTorque
TorqueRetention
```

그 뒤 **차량 전체에 사용할 하나의 공통 threshold**를 선택한다. 이 threshold는 계산 자체가 deterministic이어도 실제 TCU FACT가 아니라 CarFight의 full-throttle 성능 정책 선택이므로 기본 provenance는:

```text
Disposition = GameBias
MethodId = WheelTorqueCrossoverShift
MethodRevision = 1
ReviewRequired = true
```

로 유지한다.

실제 shift schedule FACT가 없는데 `WheelTorqueCrossoverShift` 결과가 존재한다는 이유만으로 그 값을 실차 변속점이라고 표현하지 않는다.

#### 9.2.3 ChangeDownRPM separation

`ChangeDownRPM`은 `ChangeUpRPM`의 단순 절반, Peak Torque RPM, Torque band 시작점을 그대로 복사하지 않는다.

실제 downshift FACT가 없으면 다음을 함께 고려한 별도 GAME_BIAS다.

```text
- post-downshift RPM이 redline을 넘지 않음
- current gear에서 lugging 방지
- torque band / usable power band 재진입
- ChangeDownRPM < ChangeUpRPM
- shift hunting을 피할 hysteresis margin
```

P0가 단일 ChangeDownRPM만 지원한다는 한계를 provenance와 diagnostic에 명시한다.

#### 9.2.4 Electronic top-speed limiter policy

실차의 published top speed가 전자식 limiter 때문에 제한된 경우 그 사실은 Evidence에 보존하지만 **CarFight Runtime에는 전자식 최고속 제한을 적용하지 않는다.**

```text
Evidence
- Performance.PublishedTopSpeed = FACT
- Performance.TopSpeedLimiter = FACT / limiter context

Runtime
- ElectronicTopSpeedLimiter = Disabled / not authored
- physical top speed = engine curve + gearing + final drive + tire radius + aero + rolling/driveline loss 결과
```

따라서 전자제한이 걸린 published top speed를 다음 용도로 사용하지 않는다.

```text
- FinalRatio 역산 authority
- Engine power 부족 판정의 단독 근거
- CarFight 물리 최고속 강제값
- benchmark PASS/FAIL의 hard ceiling
```

CarFight는 안전 규제/제조사 정책을 재현하는 것이 아니라 limiter가 제거된 순수 vehicle physics를 평가한다.

### 9.3 Engine response / input

다음은 실차 catalog spec에서 직접 얻지 않는다.

```text
ThrottleInputScale
EngineBrakeEffect
EngineRevUpMOI
EngineRevDownRate
TorqueMassScale
```

기본은 `BaselineInherited`, USER가 조정하면 `GameBias`다.

---

## 10. Handling Profile mapping

P0 원칙은 **실차 taxonomy를 임의 Chaos 숫자로 번역하지 않는다.**

다음 Reference만으로 numeric을 만들지 않는다.

```text
MacPherson / Multi-link / Torsion Beam
Tire brand/category
Ventilated disc / drum이라는 형식명
EPS / MDPS라는 조향 방식명
차량 segment 이름
```

따라서 SpringRate, Preload, Friction, CorneringStiffness, BrakeTorque, SteeringAngle을 catalog taxonomy만으로 생성하지 않는다.

직접 수치 Evidence가 실제로 있는 경우에만 mapping한다.

```text
actual max steer angle
actual front/rear brake torque
actual spring rate
actual preload/equivalent force
validated tire friction/cornering coefficient with same Chaos semantic
```

Turning circle/radius를 단순 `FrontWheelMaxSteerAngle`로 역산하는 것은 track/Ackermann/측정 convention assumption이 필요하므로 P0 기본 direct mapping에서 제외한다. 필요하면 별도 DERIVED rule로 후속 확장한다.

결과적으로 일반 공개 차량 spec에서는 Handling Profile의 상당 부분이 `BaselineInherited`일 수 있다. 이것은 실패가 아니라 provenance를 정직하게 유지하는 정상 상태다.

---

## 11. COM / Aero / unsupported mapping 경계

Current first mapping list에 COM이 포함돼 있으나 current 4 Profile에는 COM typed owner가 없다.

```text
VehicleMovementConfig.CenterOfMassOverride
= current direct runtime field
= 4 private Profile owner 없음
```

공개 실차 CG evidence도 흔하지 않다.

VB-P0-04에서는 COM을 `PlannedTypedGap`로 기록하되 추정값을 만들지 않는다. P0-05 구현에 Profile field를 추가할지는 실제 accepted CG use case가 있을 때만 결정한다.

반대로 `ChassisWidth`는 UE 5.8 Chaos drag computation의 명시적 setup field이고 실차 Overall Width evidence가 흔하므로 P0-05 schema delta에 포함한다.

---

## 12. Proposal readiness와 blocker

각 Domain은 두 readiness를 분리한다.

```text
TechnicalCommitReady
= typed payload 자체가 current/future schema validation을 통과하고 알려진 Evidence와 모순되지 않음

ReferenceFidelityReady
= 목표한 vehicle-specific Reference coverage가 충분함
```

예:

```text
LegacyCompatible + Transmission ratios Unknown
→ seeded baseline으로 TechnicalCommitReady 가능
→ ReferenceFidelityPartial

VehicleSpecificRequired + Transmission Core Unknown/BaselineInherited only
→ TechnicalCommitReady false
→ FACT/DERIVED 또는 USER-reviewed GAME_BIAS Transmission Proposal 필요

4AT fact + ratio array 5개
→ known contradiction
→ TechnicalCommitReady false

CVT + simple discrete transmission without approximation approval
→ UnsupportedExact
→ TechnicalCommitReady false
```

대표 blocker:

```text
Proposal.EvidenceNotAccepted
Proposal.EvidenceFingerprintStale
Proposal.RecipeFingerprintStale
Proposal.TargetDrifted
Proposal.ProfileSeedStale
Proposal.TypedOwnerAmbiguous

Mass.MaximumGrossRequired
Transmission.GearCountConflict
Transmission.UnsupportedCVTExact
Transmission.InvalidRatio
Transmission.InvalidFinalRatio
Transmission.InvalidShiftRPM
Transmission.InvalidEfficiency
```

Warning:

```text
Feel.EnvelopeReviewRequired
Performance.EngineCurveFidelityPartial
Handling.ReferenceCoveragePartial
Aero.ReferenceCoveragePartial
```

---

## 13. Stale contract

Proposal 생성 뒤 다음 중 하나라도 바뀌면 old review/approval을 재사용하지 않는다.

```text
EvidenceFingerprint
RecipeFingerprint
TargetDefinitionHash
seed Profile fingerprint
proposal mapping contract revision
실제로 소비한 Asset measurement fingerprint
```

`ProposalHash` mismatch는 fail-closed다.

Editor restart 뒤 Proposal object/approval은 transient이므로 폐기하고 fresh rebuild한다.

---

## 14. 2027 Kia Morning 1.0 4AT normalization dry-run

VB-P0-01 accepted sample을 mapping-only로 dry-run한다. Asset/Profile mutation은 없다.

Known Evidence:

```text
Curb Mass = 975 kg
Overall H = 1485 mm
Overall W = 1595 mm
Wheelbase = 2400 mm
Tire = 175/65R14
DriveLayout = FWD
Peak Torque = 95.124505 Nm @ 3750 RPM
Max Power = 55.897905 kW @ 6200 RPM
Transmission = 4AT
GVWR = Unknown
Gear ratios = Unknown
Final drive = Unknown
Redline/Max RPM = Unknown
```

Mapping result:

```text
VehicleBase
- BaseVehicleMassKg = 975      DERIVED CurbMassToBaseVehicleMass@1
- MaximumGrossMassKg = Unknown → Mass.MaximumGrossRequired if seed is 0
- ChassisHeight = 148.5 cm     DERIVED
- future ChassisWidth = 159.5 cm DERIVED
- tire-derived radius = 29.155 cm DERIVED fallback
- tire-derived width = 17.5 cm DERIVED fallback

Drivetrain
- Differential = FWD           FACT semantic mapping
- Front powered = true
- Rear powered = false
- bUseAutomaticGears = true    FACT semantic mapping from 4AT
- ExpectedForwardGearCount = 4
- ForwardGearRatios = Unknown
- Reverse ratio = Unknown
- FinalRatio = Unknown
→ seed ratio count가 4가 아니면 GearCountConflict

Performance
- EngineMaxTorque Neutral = 95.124505 Nm
- 3750 RPM은 peak-torque location diagnostic
- 6200 RPM은 max-power location diagnostic
- EngineMaxRPM으로 사용하지 않음
- Max Power kW를 별도 Chaos scalar로 억지 mapping하지 않음
- EngineCurveFidelityPartial

Handling
- MacPherson / Torsion Beam taxonomy만으로 SpringRate 생성하지 않음
- Tire size만으로 Grip/CorneringStiffness 생성하지 않음
- seed values BaselineInherited
```

이 dry-run은 `Unknown을 숫자로 채우지 않는다`, `known fact와 seed가 충돌하면 Block한다`, `4 Profile payload는 complete seed로 유지한다`는 계약을 만족한다.

---

## 15. VB-P0-04 판정

**PASS — Design / Reference → Typed Authoring Proposal Bridge Contract**

고정 결과:

```text
1. Proposal은 persistent progress가 아니라 transient stale-bound object다.
2. Builder-private 4 Profile proposal은 sparse가 아니라 complete payload다.
3. FACT/DERIVED/GAME_BIAS/BaselineInherited를 field별로 분리한다.
4. Feel physical anchor는 Neutral이며 Low/High를 FACT로 위장하지 않는다.
5. Mass/geometry/engine/drivetrain의 보수적 mapping을 확정했다.
6. Handling taxonomy→numeric 자동 추론은 P0에서 금지했다.
7. UE 5.8 FVehicleTransmissionConfig / FSimpleTransmissionConfig / FSimpleTransmissionSim exact mapping을 local Source Build로 확정했다.
8. Reverse ratio는 setup array에 positive magnitude로 저장하고 reverse 방향 부호는 `GetGearRatio()`가 적용한다는 exact convention을 확정했다.
9. Transmission architecture와 Manual/Automatic shift control을 분리했다.
10. Forward gear count는 ratio array length로 derive하고 중복 저장하지 않는다.
11. Ratio arrays는 dedicated typed atomic ratio-set으로 설계한다.
12. CVT exact mapping은 UnsupportedExact다.
13. current CarFight Apply path에 TransmissionSetup assignment가 없는 gap을 보존했다.
14. runtime hot transmission setter를 가정하지 않고 P0-05에서 lifecycle-safe setup/rebuild를 검증한다.
15. ChassisWidth와 Reference wheel geometry Profile fallback은 P0-05 future schema delta다.
16. C++/Asset/Profile/VehicleData mutation은 0이다.
```

다음 Gate:

```text
VB-P0-05 Vehicle Record / Layout / Physics Apply Flow
```

---

## 16. Changelog

### v0.1.10 - 2026-09-01

- actual Wagon USER high-speed review에서 `6단은 일찍 진입하지만 200km/h까지 가속이 길고 이후 매우 둔함`을 확인한 뒤, 이를 단일 수치 tuning이 아니라 Builder-wide Engine/Shift authoring gap으로 승격했다.
- UE 5.8 Chaos에서 `MaxTorque`가 `TorqueCurve` multiplier와 결합되는 점을 completion contract로 명문화했다. vehicle-specific MaxTorque/MaxRPM만 적용하고 공용 TorqueCurve를 상속하는 상태는 `Performance.EngineCurveFidelityPartial`이며 신규 차량 엔진 완료 근거가 아니다.
- Engine Curve provenance를 `실제 curve FACT → 충분한 FACT anchor 기반 deterministic DERIVED curve → 부족 시 explicit GAME_BIAS + USER Review` 순으로 고정했다. DERIVED curve를 제조사 원본 curve로 표현하는 것을 금지했다.
- 실제 TCU shift FACT가 없을 때 Peak Torque/Peak Power RPM을 ChangeUpRPM으로 복사하는 것을 금지하고 `WheelTorqueCrossoverShift@1`을 추가했다. 각 gear pair의 Current/Next Wheel Torque crossover, ShiftSpeed, PostShiftRPM을 먼저 계산한 뒤 P0 단일 ChangeUpRPM을 USER-reviewed GAME_BIAS로 선택한다.
- ChangeDownRPM을 ChangeUpRPM/PeakTorque의 단순 파생으로 취급하지 않고 lugging, torque-band reentry, redline, hysteresis를 고려한 별도 GAME_BIAS로 분리했다.
- 제조사 전자식 최고속 제한은 Evidence에는 FACT/context로 보존하되 CarFight Runtime에는 적용하지 않는 정책을 추가했다. PublishedTopSpeed limiter 값은 FinalRatio 역산이나 물리 최고속 hard target으로 사용할 수 없다.
- Wagon direct TorqueCurve inspect에서 4244RPM multiplier 약 `0.94048`을 확인해 `200km/h 부근에서 inherited curve가 토크를 과도하게 잘라서 막힌다`는 가설을 기각했다. Wagon은 regression evidence이며 generic 계약 숫자로 고정하지 않는다.

### v0.1.9 - 2026-09-01

- actual Wagon WSA-P0-07 USER PASS로 이전 `WSA Authority Reacceptance Blocked` 상태를 해제했다. Wheel Size Authority P0는 `WheelSizeAuthorityPlan.md v0.1.19`에서 Complete다.
- v0.1.8에서 관측한 WSA adoption fingerprint mismatch는 당시 persisted Resolver evidence이며, 현재 apply blocker로 재사용하지 않는다. next는 fresh Resolver/adoption evidence refresh로 current mismatch 여부를 재평가한다.
- PhysicsDraft v2 provenance/ratio diagnostic Technical PASS는 유지하고, WSA-dependent ShiftSpeed는 fresh authoritative Resolver Success가 확보된 뒤 다시 계산한다.

### v0.1.8 - 2026-08-31

- actual Wagon prospective Evidence `b6204acf1d6722009075865204f33b693c96ed002b4c6ba6eed21751b40b535f`에 exact binding한 PhysicsDraft schema v2를 작성하고 transient prospective Evidence로 full TransmissionReview를 검증했다.
- proposal payload는 forward 8단 `5.057/3.070/2.050/1.520/1.262/1.000/0.792/0.661`, reverse `3.992`, FinalRatio `3.20`, ChangeUp/Down `4500/2000`이다. Direct Evidence와 GAME_BIAS/auxiliary baseline provenance를 field별로 분리한다.
- `TransmissionProposalHash=14ec8925b761f16111251c1abac8c218`; 1→2 PostShiftRPM 약 `2732`, retention 약 `0.607`, downshift margin 약 `732RPM`이며 ratio-only diagnostic은 WSA와 독립적으로 유효하다.
- WSA-dependent ShiftSpeed는 authoritative Resolver Success에서만 계산한다. Blocked/Error ResolveResult의 compatibility/default Wheel Radius는 절대 WSA authority로 해석하지 않고 `Transmission.WheelRadiusAuthorityUnavailable` blocker를 생성한다.
- actual Wagon current Resolver는 WSA adoption fingerprint mismatch로 Blocked이므로 ShiftSpeed는 intentionally unavailable이다. 이는 PhysicsDraft provenance 실패가 아니라 별도 WSA-P0-07 reacceptance dependency다.
- final generic + actual validation은 Official Build PASS / focused 9/9 PASS / Wagon dry-run PASS이며 Product Asset mutation/Save 0을 유지한다.

### v0.1.7 - 2026-08-31

- Existing Reference Evidence가 이미 존재하는 차량도 later research를 안전하게 반영할 수 있도록 별도 reviewed R1 `PreviewBuilderEvidenceRefresh → CommitBuilderEvidenceRefresh` 경로를 추가했다. Existing Companion overwrite 금지는 유지한다.
- Evidence Refresh는 complete Research payload replacement, current/prospective fingerprint, explicit AuthoringWrite, EvidenceId/Recipe/Target binding 보존, Profile/VehicleData mutation0, Save0를 강제한다.
- 초기 focused test에서 Evidence diagnostic revision이 +2 되는 중복 owner 결함을 검출했고 `PostEditChangeProperty`의 기존 +1 owner만 사용하도록 교정했다.
- Official Build `d822c7a28d374263a4f67658730b8818` PASS 후 focused runner `1d3f63dc32bc4d6b93dae330f38e352f`가 9/9 PASS했다. Evidence Refresh service/VM과 기존 Builder Step1/5/7/8 회귀까지 포함한다.
- 다음은 actual Wagon fresh persisted truth에서 pre-policy `LegacyCompatible → VehicleSpecificRequired` one-time migration 후 새 Evidence/Physics proposal을 적용하는 단계다.

### v0.1.6 - 2026-08-31

- Builder-wide Transmission contract를 semantic/value provenance까지 fail-closed하도록 보강했다. unrelated FACT 재사용, Evidence 값과 proposal 값 불일치, DERIVED consumed FACT input 누락, GAME_BIAS semantic/gear-count origin 누락을 차단한다.
- Forward/Reverse ratio set, FinalRatio, reviewed gear count, ChangeUp/Down RPM의 structural validity를 추가하고 fixed-shift diagnostic에 `RpmRetention`을 실제 projection으로 맞췄다.
- Official Build `87fc2d61a6f1401fa5402a9fff6f3354` PASS, exact focused process `f90bccf4ab4c45e8ac3d84a0e604fbea` 6/6 PASS로 **Builder-wide Transmission Contract Technical PASS**를 확정했다.
- next는 fresh persisted AssetDump로 actual Wagon current truth를 다시 확보한 뒤, pre-policy Wagon만 one-time `VehicleSpecificRequired` migration하고 새 PhysicsDraft v2 proposal을 만드는 actual regression 단계다.

### v0.1.5 - 2026-08-31

- remediation owner를 Wagon 한 대가 아니라 **Guided Vehicle Builder 전체 신규 차량의 Transmission authoring contract**로 명시 교정했다. Wagon은 incident / pre-policy migration canary / actual E2E regression fixture로만 분류한다.
- Wagon Asset에 의존하지 않는 `BuilderTransmissionContract` synthetic Automation을 추가해 Legacy/VehicleSpecific policy, FACT/DERIVED/GAME_BIAS positive cases, Baseline/stale provenance/gear-count/ratio-order/shift-threshold negative cases와 deterministic Transmission hash를 직접 검증하도록 범위를 확대했다.
- focused runner `Tools/RunBuilderTransTests.ps1 v1.0.0`의 exact scope를 generic contract + Guided creation + Step5/7/8 affected flow로 고정했다. actual Wagon mutation/benchmark는 generic focused PASS 이후 단계로 유지한다.

### v0.1.4 - 2026-08-31

- v0.1.3 remediation 설계를 Source에 구현했다. Guided 신규 record는 `VehicleSpecificRequired`, 일반/Legacy record는 `LegacyCompatible`를 persistent Recipe policy로 사용한다.
- PhysicsDraft를 schema revision 2로 전진해 field-level `TransmissionReview`를 Step 5 request에 연결하고, VehicleSpecificRequired에서 Core BaselineInherited-only 또는 `bUseTransmissionConfig=false`를 fail-closed한다.
- `BuilderCommitReceipt`에 승인 당시 TransmissionPolicy/TransmissionProposalHash/TransmissionReview를 저장하고 Final Review가 current Drivetrain payload와 fresh 재계산해 policy raw drift, payload drift, provenance stale를 차단하도록 구현했다.
- `CFBuilderTransUtil.h v1.0.0`에 fixed-shift kinematic diagnostic과 common validation/hash를 구현해 Step 5와 Final Review가 동일 계약을 사용한다. Generic Resolver/Project Compatibility Default/WSA authority는 변경하지 않았다.
- Official Build `fb3ab376111d4c9283afc2731dba3e89`는 UHT PASS, compile/link 22/22, Exit0으로 성공했다. focused Automation은 아직 Pending이며 actual Wagon numeric/policy migration은 수행하지 않았다.
- stale AssetDump index의 Wagon 0/4 Profile + empty receipt는 current USER Save 상태와 모순되어 migration evidence로 사용하지 않는다. Wagon 변경 전 fresh persisted evidence를 다시 확보한다.

### v0.1.3 - 2026-08-31

- Wagon incident remediation 설계 재검수를 PASS로 닫았다. `NewVehicle/CompleteExisting`는 lifecycle 중 전환되는 상태라 신규 차량 Final Review authority로 사용할 수 없음을 Source에서 확인하고 persistent `BuilderTransmissionPolicy = LegacyCompatible / VehicleSpecificRequired`를 canonical gate owner로 확정했다.
- Resolver/Compatibility Default는 Legacy compatibility를 위해 변경하지 않는다. 신규 Guided record creation만 `VehicleSpecificRequired`를 부여하고 기존 Recipe default는 `LegacyCompatible`로 보존한다.
- 신규 Guided Vehicle Transmission Core는 `bUseTransmissionConfig=true`와 FACT/DERIVED/USER-reviewed GAME_BIAS field-level proposal을 요구하며 Core의 `BaselineInherited` only 완료를 금지했다. GearChangeTime/Efficiency는 P0 Auxiliary로 BaselineInherited를 허용한다.
- Current BuilderCommitReceipt의 aggregate Claim count만으로 Transmission field provenance를 증명할 수 없는 gap을 확인해 Physics Draft + receipt에 dedicated `TransmissionProposalReview/Hash`를 binding하도록 확정했다.
- exact ratio Unknown 시 `FixedShiftTransmissionBias@1` GAME_BIAS 생성 규칙과 `ShiftSpeedKmh / PostShiftRPM / OverallRatio / DownshiftMarginRPM` diagnostic 공식을 확정했다. 절대 shift-speed 자체는 generic blocker가 아니며 provenance/gear-count/ratio-order/shift-threshold contradiction만 fail-closed한다.
- WSA-P0-07은 별도 owner로 유지하며 diagnostic은 current effective Wheel Radius를 read-only 소비한다. Wagon numeric tuning은 아직 수행하지 않았고 `VB-P0-09 USER Driving PASS`는 계속 보류한다.

### v0.1.2 - 2026-08-31

- actual Wagon `VB-P0-09` USER E2E에서 `bUseTransmissionConfig=false`인 신규 차량이 vehicle-specific Transmission candidate를 만들지 않은 채 Project Compatibility Default `2.85/2.02/1.35/1.00 + Final 3.08 + Up/Down 4500/2000`으로 최종 주행하게 되는 gap을 기록했다.
- actual PIE에서 1단이 약 80 km/h까지 유지된 뒤 2단으로 변속됨을 USER evidence로 확인했다. 이는 속도 기반 변속이 아니라 고정 ChangeUpRPM과 지나치게 긴 compatibility gear/final 조합의 결과로 분류했다.
- 기존 `Missing transmission fact → BaselineInherited`는 Legacy/seed 용도로 보존하되, 신규 Guided Vehicle의 최종 vehicle-specific Transmission 완료 근거로 사용할 수 없도록 remediation 원칙을 추가했다.
- P0 Runtime은 단일 ChangeUp/Down RPM을 유지하고 가변 shift map/속도 기반 runtime trigger는 Scope Out으로 유지한다. Builder에는 기어별 예상 상향변속 차속과 변속 직후 RPM을 sanity diagnostic으로 검토하도록 기록했다.
- 정확한 blocker/warning 및 ratio proposal 알고리즘은 후속 설계 owner이며 이번 기록 단계에서는 C++/Profile/Recipe/VehicleData/Save mutation을 수행하지 않는다. `VB-P0-09 USER Driving PASS`는 보류한다.

### v0.1.1 - 2026-08-26

- VB-P0-04 final source closure에서 `D:\UnrealEngine_Source`의 actual UE 5.8 `FVehicleTransmissionConfig::InitDefaults/FillTransmissionSetup/GetGearRatio`와 `FSimpleTransmissionSim::GetGearRatio`를 read-only로 직접 확인했다.
- v0.1.0의 잘못된 선확정인 `Reverse ratio를 setup array에 negative로 materialize` 규칙을 폐기했다.
- exact convention을 `ReverseGearRatios/ReverseRatios = positive magnitude`, `reverse sign = GetGearRatio()에서 적용`으로 교정했다.
- Evidence가 음수 reverse ratio를 제공해도 magnitude+direction으로 normalize하고, Builder/Chaos setup에는 positive magnitude만 저장하도록 고정했다.
- 이 교정으로 VB-P0-04의 남은 Source uncertainty가 0이 되어 `Source Closure PASS / TRUE PASS`로 닫았다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation은 수행하지 않았다.

### v0.1.0 - 2026-08-26

- VB-P0-04 Reference → Authoring Proposal Bridge의 first design owner를 신규 작성했다.
- accepted Evidence → complete Builder-private 4 Profile proposal → 기존 Resolver/Review lane의 stale-safe contract를 고정했다.
- sparse Profile 금지와 current effective value 기반 complete seed, Feel Neutral anchor / Low·High provenance 정책을 확정했다.
- VehicleBase/Drivetrain/Handling/Performance별 direct/derived/inherited/unsupported mapping을 구분했다.
- UE 5.8 `FVehicleTransmissionConfig`, `FSimpleTransmissionConfig`, `ETransmissionType`, `FSimpleTransmissionSim` contract를 Source/API 기준으로 대조했다.
- Transmission control mode, ratio-set, final drive, shift thresholds/time, efficiency와 reverse sign convention을 확정했다.
- gear count는 ratio array length에서 파생하고 known count와 array length 충돌을 blocker로 정했다.
- primitive ratio arrays의 weak reflection type ambiguity를 피하기 위해 dedicated atomic ratio-set USTRUCT 설계를 선택했다.
- current CarFight에 없는 Transmission fields, ChassisWidth, VehicleBase wheel geometry fallback을 VB-P0-05 future typed schema delta로 기록했다.
- 2027 Kia Morning sample dry-run에서 4AT의 count/type만 확정하고 ratio/final drive/RPM Unknown을 보존했다.
- Implementation 0을 유지했다.

---

## 17. Migration

- VB-P0-05 구현 전까지 current `FCFVehicleMovementConfig`, 4 Profile payload, Registry descriptor count와 Apply behavior는 변경하지 않는다.
- 새 Transmission schema가 구현되면 Field Registry, Snapshot/Codec, Resolver, Materializer, Apply/Readback, stale hash와 affected tests를 한 migration으로 동기화한다.
- 기존 shared Profile은 Builder가 직접 수정하지 않는다. 필요하면 private prospective payload로 clone한 뒤 explicit reviewed commit을 사용한다.
- 기존 Profile의 Low/High response와 mass-scale rule은 source clone 시 보존하며 Evidence FACT로 재분류하지 않는다.
- 기존 HUD Gear runtime authority는 실제 Chaos current gear를 계속 사용한다.
- 이 문서의 Source mapping은 UE 5.8 기준이다. Engine version 변경 시 Transmission contract를 새 엔진 Source와 다시 대조한다.
- v0.1.1부터 Reverse ratio setup array는 positive magnitude가 canonical이다. v0.1.0의 `ReverseRatioMagnitudeToChaosSigned@1` 또는 negative-array 해석은 폐기하며 사용하지 않는다.
