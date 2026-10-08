# CarFight Vehicle Reference Evidence Spec

- 문서 버전: v0.1.3
- 작성일: 2026-09-01
- 문서 상태: Current Contract / VB-P0-01 Design Complete / Research Normalization Dry-Run PASS / Persistent Evidence + Existing Evidence Refresh Implemented
- Feature: `CF-FQ-040 Guided Vehicle Builder`
- 상위 계획: `VehicleBuilderPlan.md`
- 적용 Gate: `VB-P0-01 Reference Research & Evidence Contract`
- 역할: Vehicle Reference Evidence의 Editor-only persistent schema, research normalization, provenance/conflict/confidence, deterministic fingerprint와 AI proposal binding 계약을 고정한다.

---

## 1. 범위와 비범위

이 문서는 VB-P0-00에서 이미 확정한 owner를 다시 감사하지 않는다.

이번 문서가 고정하는 것은 다음뿐이다.

```text
Vehicle Reference Evidence exact logical schema
Reference Vehicle identity scope
Source / citation normalization
FACT / DERIVED / GAME_BIAS provenance
Unknown fact 처리
Conflict / variant 분리
Confidence 산정 의미
Evidence fingerprint canonicalization
AI Builder proposal binding
실제 경차 1건 research normalization dry-run
```

이번 Gate에서 하지 않는 것:

```text
C++ class 구현 0
UE DataAsset 생성 0
VehicleData/Profile/Recipe mutation 0
Builder Tab 구현 0
새 GoPyMCP tool 0
UE Editor 기동 0
Build/Automation 실행 0
Production balance 수치 확정 0
```

---

## 2. 미래 구현 identity

향후 구현 시 권장 class/file identity는 다음으로 고정한다.

```text
Class: UCFVehicleRefEvidence
File:  CFVehicleRefEvidence.h/.cpp
```

모두 32자 제한 안이다.

이 Asset은 `UDataAsset` 기반 Editor-only companion record이며 `IsEditorOnly()=true` / Never-Cook 의도를 갖는다.
Runtime `UCFVehicleData`에는 citation/research provenance를 추가하지 않는다.

---

## 3. Root DataAsset exact logical schema

`UCFVehicleRefEvidence`는 아래 의미 필드를 보관한다.

| Field | Logical Type | 역할 | Evidence Fingerprint |
| --- | --- | --- | --- |
| `EvidenceId` | GUID | Evidence record 자체의 persistent identity. 일반 Asset duplicate 시 새 ID 발급 | 포함 |
| `SchemaRevision` | int | Evidence schema 계약 revision | 포함 |
| `NormalizationPolicyRevision` | name/string | 단위·confidence·conflict canonicalization 정책 revision | 포함 |
| `TargetRecipeId` | GUID | companion이 속한 persistent Recipe identity. Target primary identity | 포함 |
| `TargetRecipePath` | soft object path | 사람/도구 진단 및 exact object lookup | 포함 |
| `TargetDefinitionPath` | soft object path | Recipe가 binding한 Runtime VehicleData identity | 포함 |
| `ReferenceVehicles` | array | 1~5개 Reference Vehicle identity | 포함 |
| `Sources` | array | normalized citation records | 포함 |
| `Claims` | array | normalized FACT/DERIVED/GAME_BIAS atomic claims | 포함 |
| `Conflicts` | array | candidate disagreement/variant conflict와 resolution | 포함 |
| `UnknownFacts` | array | 조사했지만 exact evidence를 확보하지 못한 fact | 포함 |
| `EvidenceFingerprint` | SHA-256 hex | 위 semantic payload의 generated read-only fingerprint | 자기 자신 제외 |
| `AuthoringRevision` | int | 사람이 읽는 diagnostic edit sequence | 제외 |
| `LastResearchAtUtc` | datetime | 마지막 조사 시각 진단 | 제외 |
| `ResearchNotes` | text | 사람이 읽는 조사 메모 | 제외 |

`TargetRecipeId`를 primary target identity로 사용한다. 현재 `UCFVehicleData`에는 별도 persistent VehicleId가 없으므로 `DefinitionPath`를 diagnostic/exact binding path로 함께 보존한다.

Evidence는 Reference database가 아니라 **한 Builder vehicle의 companion research SSOT**다. 다른 게임 차량이 같은 실차 자료를 재사용하더라도 별도 Evidence record와 target binding을 갖는다.

---

## 4. Reference Vehicle Identity

미래 logical struct 이름:

```text
FCFRefVehicleIdentity
```

필수/선택 필드:

| Field | Required | 설명 |
| --- | --- | --- |
| `ReferenceVehicleId` | Yes | Evidence 내부 stable ID |
| `Role` | Yes | `Primary / Secondary / TraitOnly` |
| `Manufacturer` | Yes | 제조사 |
| `Model` | Yes | 모델명 |
| `Generation` | No | 세대 코드. 근거가 없으면 비움 |
| `ModelYearStart` | Yes* | 단일 연식이면 Start=End |
| `ModelYearEnd` | Yes* | 연식 범위. 정확히 모르면 qualifier 사용 |
| `ModelYearQualifier` | Yes | `Exact / Range / CurrentAsObserved / Unknown` |
| `Trim` | No | 트림. Trim-dependent fact 사용 시 사실상 필수 |
| `Powertrain` | Yes | 엔진/모터/연료 identity |
| `Transmission` | No | MT/AT/DCT/CVT 및 단수. 관련 fact 사용 시 필수 |
| `DriveLayout` | No | FWD/RWD/AWD 등. 출처가 없으면 비움 |
| `MarketRegion` | Yes | KR/EU/US 등 |
| `BodyVariant` | No | 승용/밴/차체 variant |
| `WheelTireVariant` | No | 예: `175/65R14`; 중량/track 등 variant-dependent fact에는 필수 |
| `IdentityConfidence` | Yes | identity 자체의 evidence confidence |

원칙:

```text
같은 모델명이라는 이유로 연식/트림/시장/타이어 variant를 합치지 않는다.
Identity가 다르면 평균하지 않고 별 Reference Vehicle 또는 Variant scope로 분리한다.
Generation code가 공식 자료에 없으면 database 출처만으로 채울 수 있으나 confidence를 낮춘다.
```

### 4.1 Production Vehicle Reference Strategy

CarFight production 차량의 기본 Reference authority는 **특정 실존 차량의 exact identity**다.

```text
기본:
Primary = 특정 Manufacturer / Model / Model Year / Trim / Powertrain / Market

보조:
Secondary = Primary의 누락 FACT를 보강하는 동일/근접 identity
TraitOnly = 특정 성격만 참고하는 별도 차량

예외:
실존하지 않는 차량
→ Vehicle Archetype이 Primary authority
→ 실존 차량은 Secondary/TraitOnly로 선택 사용
```

단순 Segment/Body Style 평균은 실존차 production Evidence의 Primary Reference가 아니다.

USER가 차량 후보 추천을 요청할 수 있으며, AI는 후보를 exact identity 수준으로 조사한 뒤 다음 기준으로 선별한다.

```text
Data Availability
- 공식/규제/신뢰 출처의 exact identity coverage
- Mass / Dimensions / Wheel / Engine / Transmission / Performance FACT coverage
- Forward gear ratios / reverse / final drive availability
- 독립 cross-check 가능성
- variant confusion 위험

Builder Confidence
- FACT direct mapping coverage
- DERIVED 필요량
- GAME_BIAS 필요량
- source conflict / Unknown 비율

CarFight Value
- 기존 차량과 다른 drive layout / mass / power / transmission / handling character
- 새 Vehicle Builder regression fixture로서의 가치
```

`Data Availability`가 높은 차량을 우선 선택하는 것은 단순 편의가 아니라 **Builder contract와 research quality를 분리해서 검증하기 위한 제작 전략**이다.

### 4.2 Fictional Vehicle / Archetype Evidence

실존하지 않는 차량은 exact Primary Reference Vehicle이 없으므로 별도 Archetype evidence를 사용한다.

Archetype은 단일 평균값이 아니라 차급별 현실적 범위/분포를 가진다.

```text
예:
Midsize AWD Wagon
CurbMass range
Wheelbase range
Torque range
Gear-count range
Performance range
```

가상 차량의 vehicle-specific proposal은:

```text
Archetype range
+ USER desired character
+ optional real Secondary/TraitOnly Reference
→ GAME_BIAS/DERIVED를 포함한 bounded proposal
```

로 만든다.

Archetype 값은 실존 차량에서 이미 확인된 FACT보다 우선할 수 없다.

### 4.3 Unknown / Average-value Rule

실존 Primary Reference가 있는 차량에서 공개되지 않은 값을 처리할 때 다음을 강제한다.

```text
Exact source FACT
→ FACT

FACT에서 deterministic 계산 가능
→ DERIVED

Runtime 필수이나 source가 없음
→ UnknownFact 유지
→ 해당 차량의 확인된 FACT를 입력으로 vehicle-specific GAME_BIAS
→ USER Review

차급/장르 평균
→ 실존 차량의 missing FACT를 조용히 대체하는 authority로 사용 금지
```

따라서 예를 들어 실제 automatic shift RPM이 공개되지 않았을 때 `해당 차급 평균 ChangeUpRPM`을 FACT처럼 넣지 않는다. Engine torque/power band, gear ratios, performance 등 해당 차량 Evidence를 입력으로 별도 GAME_BIAS method를 사용한다.

### 4.4 Technical Identity vs Game Branding

Evidence 내부에서는 실제 Manufacturer/Model identity를 정확히 유지한다.

게임 내 차량명·엠블럼·배지·브랜드 표기와 외형 presentation은 별도 제작 concern이다. 게임 presentation에서 브랜드 표기를 제거해도 Evidence의 Primary Reference identity를 익명화하거나 변경하지 않는다.

이 분리는 **기술 재현 근거를 추적 가능하게 유지하기 위한 authoring 계약**이며, 외형/IP 판단 자체를 이 Evidence schema가 대신하지 않는다.

---

## 5. Source / Citation schema

미래 logical struct 이름:

```text
FCFRefSourceCitation
```

필드:

| Field | 설명 |
| --- | --- |
| `SourceId` | Evidence 내부 stable source ID |
| `Tier` | `A / B / C / D` |
| `SourceKind` | `ManufacturerSpec / ManufacturerPrice / Manual / Regulatory / MediaMeasurement / VehicleDatabase / Community / Other` |
| `Publisher` | 발행 주체 |
| `DocumentTitle` | 문서/페이지 제목 |
| `CanonicalUrl` | tracking/fragment를 제거한 canonical URL |
| `PublishedDate` | 알 수 있을 때 ISO date |
| `UpdatedDate` | 알 수 있을 때 ISO date |
| `AccessedAtUtc` | 조사 시각. fingerprint 제외 |
| `Locator` | page/section/table/heading 등 필요한 위치 정보 |
| `ReferenceVehicleIds` | 이 source가 실제 커버하는 identity |
| `OriginGroupId` | 동일 원자료 재게시를 독립 출처로 오인하지 않기 위한 origin family |
| `OriginIndependence` | `SameOrigin / IndependentOrigin / OriginUnknown` |
| `SourceScopeNote` | 연식/트림/시장 제한을 설명하는 짧은 메모. fingerprint 제외 |

Tier 의미:

```text
A = 제조사 공식 제원 / Manual / Homologation·Regulatory
B = 신뢰 가능한 자동차 전문 매체의 독립 계측/제원
C = 유지 관리되는 차량 DB/포털
D = 커뮤니티/비공식 보조 자료
```

다른 publisher 두 곳에 같은 표가 있다고 해서 자동으로 `IndependentOrigin`으로 판정하지 않는다. 원자료 독립성이 확인되지 않으면 `OriginUnknown`이며 strict independent cross-check count에는 넣지 않는다.

원문 기사/표 전체를 Asset에 복제하지 않는다. 필요한 fact와 locator/citation만 보존한다.

---

## 6. Atomic Claim schema

미래 logical struct 이름:

```text
FCFRefClaim
```

모든 수치는 한 개의 atomic claim으로 정규화한다.

| Field | 설명 |
| --- | --- |
| `ClaimId` | Evidence 내부 stable claim ID |
| `ReferenceVehicleId` | 어떤 실차 identity의 claim인지 |
| `FactKey` | Target VehicleData field path가 아닌 research semantic key |
| `ValueKind` | `Number / Integer / Boolean / CanonicalText / Range` |
| `NormalizedValue` | Number일 때 canonical value |
| `NormalizedMin/Max` | Range일 때 canonical bounds |
| `CanonicalTextValue` | categorical/text value |
| `UnitId` | `kg`, `mm`, `kW`, `N*m`, `rpm` 등 canonical unit |
| `SourceValueText` | 원 출처 표기 보존용 짧은 값 표현. 전체 문장/표 복제 금지 |
| `Provenance` | `FACT / DERIVED / GAME_BIAS` |
| `CitationIds` | direct FACT를 뒷받침하는 source IDs |
| `InputClaimIds` | DERIVED/GAME_BIAS 입력 claim IDs |
| `MethodId` | DERIVED 계산 또는 GAME_BIAS policy의 stable method ID |
| `MethodRevision` | 계산/정책 revision |
| `MethodParameters` | canonical bounded parameters |
| `ConfidenceScore` | 0.0~1.0 support confidence |
| `ConflictId` | conflict resolution에 속하면 해당 ID |
| `ResolutionState` | `Canonical / Candidate / Rejected` |

### 6.1 FactKey 규칙

FactKey는 research vocabulary다.

예:

```text
Dimensions.Length
Dimensions.Width
Dimensions.Height
Dimensions.Wheelbase
Dimensions.FrontTrack
Dimensions.RearTrack
Mass.Curb
Mass.Gross
Wheel.TireSize
Wheel.NominalOuterRadius
Engine.Displacement
Engine.MaxPower
Engine.MaxPowerRpm
Engine.PowerBand
Engine.MaxTorque
Engine.MaxTorqueRpm
Engine.TorqueBand
Engine.TorqueCurvePoint
Drivetrain.DriveLayout
Drivetrain.TransmissionType
Drivetrain.GearCount
Drivetrain.GearRatios
Drivetrain.ReverseGearRatio
Drivetrain.FinalDriveRatio
Drivetrain.UpshiftCondition
Drivetrain.DownshiftCondition
Drivetrain.GearChangeTime
Drivetrain.TransmissionEfficiency
Chassis.FrontSuspensionType
Chassis.RearSuspensionType
Performance.ZeroTo100
Performance.TopSpeed
Performance.PublishedTopSpeed
Performance.TopSpeedLimiter
```

이 키를 `VehicleMovementConfig.X`나 Chaos `TransmissionSetup.X` 같은 Runtime raw field path와 동일시하지 않는다. Reference→CarFight typed mapping은 VB-P0-04의 책임이다.

### 6.1.1 Engine curve / limiter evidence semantic scope

다음 차량에서 `MaxTorque` 한 숫자만 수집하고 엔진 authoring을 완료한 것으로 오해하지 않도록 가능한 경우 RPM별 powertrain context를 별도 claim으로 보존한다.

```text
Engine.MaxTorque
Engine.MaxTorqueRpm 또는 Engine.TorqueBand
Engine.MaxPower
Engine.MaxPowerRpm 또는 Engine.PowerBand
Engine.TorqueCurvePoint[]
Engine redline / limiter RPM이 source에 명시된 경우 해당 semantic
```

`Engine.TorqueCurvePoint`는 공개된 실제 curve/dyno point를 atomic claim으로 표현하기 위한 research vocabulary다. 전체 curve를 한 문자열이나 이미지로 Evidence에 복제하지 않고 RPM + Torque pair 단위로 정규화한다.

전체 curve FACT가 없고 peak/band spec만 있는 경우 Evidence는 없는 curve point를 만들어내지 않는다. 후속 Proposal 단계가 FACT anchor를 입력으로 DERIVED/GAME_BIAS curve를 만들 수 있지만 그것은 Reference FACT가 아니다.

최고속 자료는 다음처럼 구분한다.

```text
Performance.PublishedTopSpeed
= 제조사/신뢰 출처가 발표한 실제 차량 최고속 수치

Performance.TopSpeedLimiter
= Electronic / Mechanical / Unknown / None 등 제한 context
```

전자식 제한이 있는 published top speed는 차량의 순수 power-limited top speed와 동일한 semantic이 아니다. Evidence에는 둘을 분리해 보존한다. CarFight Runtime에서 limiter를 적용할지 여부는 Proposal/Gameplay policy owner이며 Evidence가 결정하지 않는다.

### 6.1.2 Transmission evidence semantic scope

다단 변속기 자료는 Guided Vehicle Builder의 Drivetrain research 범위에 포함한다.

Evidence가 표현할 수 있어야 하는 최소 의미는 다음과 같다.

```text
Transmission type / automatic 여부
Forward gear count
Forward gear ratios
Reverse gear ratio
Final drive ratio
Upshift 조건 또는 RPM
Downshift 조건 또는 RPM
Gear change time
Transmission efficiency
```

이 목록은 **research semantic vocabulary**다. C++ 배열 구조, Profile field layout, Chaos property 이름이나 1:1 mapping을 이 문서에서 확정하지 않는다.

provenance 규칙은 다른 claim과 동일하다.

```text
공식/신뢰 source에 직접 명시된 gear ratio / final drive / shift RPM
→ FACT

공개 FACT를 입력으로 계산한 ratio/shift estimate
→ DERIVED
→ InputClaimIds + MethodId/Revision 필수

CarFight 주행감을 위해 의도적으로 조정한 shift point/change time/efficiency
→ GAME_BIAS
→ Research normalization 단계 자동 생성 금지
```

특히 다음을 금지한다.

```text
자료가 없는데 일반적인 6AT 값으로 채우기
Gear count만 보고 gear ratio 배열을 임의 생성하기
Engine Redline을 그대로 Upshift RPM이라고 사실처럼 기록하기
Chaos 기본 TransmissionSetup 값을 실차 FACT로 역해석하기
```

`GearRatios`처럼 gear 순서가 의미인 Evidence는 각 전진 gear identity/order를 보존해야 한다. exact logical container/schema는 VB-P0-04에서 typed Drivetrain payload와 함께 확정한다.

Transmission efficiency처럼 공개 근거가 드문 값은 기본값으로 숨겨 채우지 않고 `UnknownFacts`에 남긴다. AI Proposal이 필요로 할 때만 DERIVED/GAME_BIAS 전환을 명시적으로 검토한다.

### 6.2 Unit normalization과 provenance

단순 단위 변환은 새로운 물리 사실을 만든 것이 아니므로 provenance를 `DERIVED`로 바꾸지 않는다.

예:

```text
Source: 9.7 kgf·m @ 3,750 rpm
Normalized: 95.124505 N·m @ 3,750 rpm
Provenance: FACT
```

반면 타이어 규격으로 외경/반지름을 계산하면 새로운 quantity이므로 `DERIVED`다.

```text
Source FACT: 175/65R14
Derived: nominal tire outer radius = 291.55 mm
MethodId: TireRadiusFromMetricSize
```

---

## 7. Provenance contract

### FACT

```text
출처에서 직접 관측한 quantity/category
CitationIds 1개 이상 필수
Unit normalization 허용
InputClaimIds는 기본적으로 비움
```

### DERIVED

```text
FACT 또는 다른 DERIVED claim으로부터 계산한 새로운 quantity
InputClaimIds 1개 이상 필수
MethodId + MethodRevision 필수
CitationIds는 직접 source가 없으면 비워도 됨
입력 claim이 바뀌면 fingerprint가 바뀜
```

### GAME_BIAS

```text
CarFight 플레이 목적의 의도적 조정
InputClaimIds 1개 이상 권장
MethodId/Reason 필수
실차 사실처럼 표시 금지
Research normalization 단계에서는 자동 생성 금지
USER/Builder가 design intent를 승인한 뒤에만 생성 가능
```

따라서 순수 Reference Research dry-run의 정상 결과는 `FACT + DERIVED`, `GAME_BIAS=0`이다.

---

## 8. Unknown fact contract

근거를 못 찾았다고 값을 추정해 채우지 않는다.

미래 logical struct:

```text
FCFRefUnknownFact
```

필드:

```text
UnknownFactId
ReferenceVehicleId
FactKey
SearchedSourceIds[]
Reason = NotPublished / IdentityAmbiguous / ConflictingUnresolved / OutOfScope
BlockingUse = None / ProposalWarning / ProposalBlock
Note
```

Unknown은 `0`, 빈 숫자, compatibility default와 동일한 의미가 아니다.
Builder Proposal이 어떤 target field를 만들 때 필수 input으로 선언한 FactKey가 Unknown이면 그 mapping만 fail-closed한다.

---

## 9. Conflict / Variant contract

미래 logical struct:

```text
FCFRefConflict
```

필드:

| Field | 설명 |
| --- | --- |
| `ConflictId` | stable local ID |
| `ReferenceVehicleId` | 대상 identity |
| `FactKey` | 충돌한 research semantic key |
| `CandidateClaimIds` | 후보 claim 목록 |
| `ConflictType` | 충돌 종류 |
| `Severity` | `Info / Warning / Block` |
| `ResolutionPolicy` | 해결 방식 |
| `ResolutionClaimId` | 최종 canonical claim. 없으면 unresolved |
| `ResolutionReasonCode` | deterministic reason code |
| `ResolutionNote` | 사람이 읽는 메모. fingerprint 제외 |

`ConflictType`:

```text
IdentityMismatch
VariantScopeMismatch
SourceDisagreement
UnitSemanticMismatch
MeasurementSpread
OutlierCandidate
```

`ResolutionPolicy`:

```text
SplitIdentity
SplitVariant
PreferExactIdentity
PreferHigherTier
UseDerivedRange
RejectOutlier
UnresolvedBlock
```

중요 규칙:

```text
Identity/Variant mismatch는 평균 금지.
중량/휠베이스/출력/토크/구동방식처럼 physics anchor에 unresolved material conflict가 있으면 Proposal mapping을 Block.
여러 독립 실측값이 정상적인 measurement spread를 보이면 FACT 후보들을 보존하고 DERIVED Range claim을 만들 수 있음.
```

---

## 10. Confidence contract

`ConfidenceScore`는 "게임에서 이 값이 좋은가"가 아니라 **근거가 이 claim을 얼마나 확실히 지지하는가**를 뜻한다.

### 10.1 FACT base score

```text
Tier A = 0.90
Tier B = 0.80
Tier C = 0.65
Tier D = 0.40
```

조정:

```text
+0.05 exact ModelYear/Market/Powertrain/Variant identity match
+0.04 verified IndependentOrigin corroboration
+0.01 publisher가 다른 동일값 corroboration, 원자료 independence는 Unknown
-0.15 partial/ambiguous identity
```

최종 score는 0.0~0.99 clamp.

Unresolved material conflict는 score를 억지로 낮춰 통과시키지 않고 별도 `Block`으로 처리한다.

### 10.2 DERIVED

```text
DerivedConfidence = min(InputClaim confidence) × MethodReliability
```

P0 method reliability 예:

```text
Deterministic geometry formula = 0.98
Exact algebraic ratio          = 0.98
Assumption-bearing estimate    = 0.75 이하
```

단순 unit conversion은 FACT provenance를 유지하므로 이 규칙을 적용하지 않는다.

### 10.3 GAME_BIAS

GAME_BIAS의 score는 실차 진실성 점수가 아니다. baseline/input evidence가 얼마나 안정적으로 binding됐는지를 표시한다.
게임 디자인의 적합성은 USER driving feel gate에서 따로 판정한다.

### 10.4 표시 band

```text
Very High >= 0.90
High      >= 0.80
Medium    >= 0.60
Low       >= 0.40
Very Low  <  0.40
```

Score는 advisory이고 Block/Allow authority는 identity/conflict/required-input rule이 소유한다.

---

## 11. Evidence Fingerprint contract

### 11.1 Algorithm

```text
Algorithm: SHA-256
Domain separator: "CFVehicleRefEvidence\0"
Contract revision: CFVREF-1
Normalization policy: CFVRN-1
Output: lower-case 64 hex characters
```

### 11.2 Canonical payload 포함

```text
SchemaRevision
NormalizationPolicyRevision
EvidenceId
TargetRecipeId
TargetRecipePath
TargetDefinitionPath
ReferenceVehicles semantic fields
Sources semantic citation fields
Claims semantic fields
Conflicts semantic resolution fields
UnknownFacts semantic fields
```

### 11.3 제외

```text
EvidenceFingerprint 자기 자신
AuthoringRevision
LastResearchAtUtc / AccessedAtUtc
ResearchNotes / SourceScopeNote / ResolutionNote
Editor array order
localized DisplayName
UI expansion/selection state
```

### 11.4 Canonicalization

```text
UTF-8 + Unicode NFC
object key deterministic order
ReferenceVehicleId / SourceId / ClaimId / ConflictId / UnknownFactId 기준 array sort
CitationIds/InputClaimIds는 ID sort
float/double은 locale-independent round-trip canonical representation
UnitId는 canonical unit vocabulary 사용
URL은 scheme/host normalization + tracking/fragment 제거
null/unknown과 0을 구분
```

의미가 같은 record를 UI에서 재정렬하거나 access timestamp만 갱신해도 fingerprint는 같아야 한다.
값, identity, citation semantic locator, conflict resolution 또는 target binding이 바뀌면 fingerprint는 달라져야 한다.

`EvidenceFingerprint`는 AI가 신뢰 가능한 문자열로 써주는 값이 아니라 Editor-side canonical builder가 계산하는 generated read-only 값이다.

---

## 12. AI Proposal Binding contract

Evidence와 실제 Authoring Proposal의 hash를 분리한다.

```text
EvidenceFingerprint
= 어떤 조사/정규화 상태를 사용했는가

BuilderProposalHash
= 그 Evidence를 사용해 어느 Target/Recipe/private Profile에 어떤 typed authoring payload를 제안했는가
```

미래 logical wrapper:

```text
FCFVehicleBuilderProposal
```

필수 binding:

```text
ProposalContractRevision
ProposalId
OperationName
EvidencePath
EvidenceId
ExpectedEvidenceFingerprint
ExpectedRecipeId
ExpectedRecipeFingerprint
ExpectedTargetDefinitionPath
ExpectedTargetDefinitionHash
ExpectedResolverContractRevision
PrivateProfileBindings[4]
ConsumedClaimIds[]
TypedDomainPayloads
ProposalHash
RequiredApprovalClass = AuthoringWrite
bSavePerformed = false
```

각 `PrivateProfileBinding`:

```text
Domain
ProfilePath
ExpectedProfileFingerprint
ExpectedOwnerRecipeId
```

VB-P0-00에서 확인된 private ownership marker는 아직 Source에 없으므로 `ExpectedOwnerRecipeId`의 persistent 구현은 VB-P0-04에서 추가할 gap이다. 이 gap을 shared Profile write나 raw property patch로 우회하지 않는다.

### 12.1 기존 Authoring contract 재사용

현재 `FCFAuthoringProposal` / `FCFAuthoringCallContext`의 다음 값을 그대로 재사용한다.

```text
ApprovalScopeHash = BuilderProposalHash
ExpectedRecipeFingerprint
ExpectedTargetDefinitionHash
ExpectedResolverContractRevision
```

Builder layer가 추가하는 stale precondition은 다음이다.

```text
ExpectedEvidenceFingerprint
ExpectedProfileFingerprint[4]
ExpectedOwnerRecipeId[4]
```

### 12.2 Commit fail-closed 조건

아래 하나라도 다르면 commit하지 않는다.

```text
EvidenceId/fingerprint changed
Target RecipeId changed
Recipe fingerprint changed
Target Definition path/hash changed
Resolver contract revision changed
private Profile path/fingerprint changed
private owner RecipeId mismatch
Consumed Claim이 더 이상 Canonical이 아님
Consumed Claim에 unresolved Block conflict 발생
required Fact가 Unknown으로 변경
ProposalHash / approval scope mismatch
```

stale 발생 시 old approval을 자동 재사용하지 않고 새 Preview를 만든다.
자동 retry 0, auto save 0을 유지한다.

---

## 13. Research normalization workflow

AI research 정상 흐름:

```text
1. Reference candidate identity 결정
2. ModelYear / Market / Powertrain / Trim / Wheel Variant scope 잠금
3. Tier A 우선 source 수집
4. Tier B/C로 중요한 fact 교차검증
5. Source origin independence 분류
6. Raw fact를 canonical unit으로 normalize
7. Variant/identity mismatch를 conflict로 분리
8. Derived fact는 explicit method/input binding으로 생성
9. Unknown fact를 explicit record로 남김
10. Blocking conflict/required Unknown 검사
11. EvidenceFingerprint 계산
12. USER/Builder Review
13. 이후 VB-P0-04가 Evidence-bound typed Proposal을 생성
```

Research 단계 자체는 VehicleData/Profile을 쓰지 않는다.

---

## 14. Research Normalization Dry-Run — PASS

### 14.1 Reference Set

Production balance truth가 아닌 contract 검증용 실제 경차 1건을 사용했다.

```text
ReferenceVehicleId:
REF-KIA-MORNING-2027-KR-10G-4AT-14

Manufacturer: Kia
Model: Morning
Generation: JA
Model Year: 2027
Trim: Trendy
Powertrain: Smartstream G1.0 gasoline
Transmission: 4-speed automatic
Market: Korea
Wheel/Tire Variant: 175/65R14
Role: Primary
```

`Generation=JA`는 manufacturer page가 아니라 Danawa DB identity에서 확인되므로 identity confidence에서 그 부분은 Tier C 근거로 취급한다.

### 14.2 Sources

`SRC-KIA-SPEC` — Tier A

```text
Publisher: Kia
Title: Morning specification
URL: https://www.kia.com/kr/vehicles/morning/specification
Observed identity: The 2027 Morning / 1.0 gasoline
Facts used: dimensions, wheelbase/track, power, torque, displacement, 14/16-inch curb mass, 4AT, tire size
OriginGroupId: KIA_OFFICIAL
OriginIndependence: SameOrigin with other Kia pages
```

`SRC-KIA-PRICE` — Tier A

```text
Publisher: Kia
Title: Morning price/options
URL: https://www.kia.com/kr/vehicles/morning/price
Observed date basis: 2026-08-01
Facts used: 2027 Morning Trendy, Smartstream G1.0, 4AT, 175/65R14
OriginGroupId: KIA_OFFICIAL
```

`SRC-CARNOON` — Tier C

```text
Publisher: Carnoon
Title: The New Morning 2027 gasoline 1.0
URL: https://www.carnoon.co.kr/newcar/vehicle/11562-117116
Facts used: dimensions, 998 cc, 76 PS@6200, 9.7 kgf·m@3750, 975 kg 14-inch, 4AT
OriginGroupId: CARNOON_DB
OriginIndependence: OriginUnknown
```

`SRC-DANAWA` — Tier C

```text
Publisher: Danawa Auto
Title: The New Morning (JA) 2027 specs
URL: https://mauto.danawa.com/service/ajax_spec_mobile.php?Lineup=53808&Type=spec
Facts used: JA identity, FF, 4AT, MacPherson front / torsion-beam rear
OriginGroupId: DANAWA_DB
OriginIndependence: OriginUnknown
```

The 2027 Morning의 출시일 2026-05-18은 현대자동차그룹 공식 뉴스로 identity를 추가 확인했다.

### 14.3 Canonical FACT claims

| FactKey | Canonical Value | Provenance | Confidence | 근거 |
| --- | ---: | --- | --- | --- |
| `Dimensions.Length` | 3595 mm | FACT | Very High | Kia + Carnoon |
| `Dimensions.Width` | 1595 mm | FACT | Very High | Kia + Carnoon |
| `Dimensions.Height` | 1485 mm | FACT | Very High | Kia + Carnoon |
| `Dimensions.Wheelbase` | 2400 mm | FACT | Very High | Kia + Carnoon |
| `Dimensions.FrontTrack` | 1406 mm | FACT | Very High | Kia 14-inch scope |
| `Dimensions.RearTrack` | 1415 mm | FACT | Very High | Kia 14-inch scope |
| `Mass.Curb` | 975 kg | FACT | Very High | Kia + Carnoon, 14-inch scope |
| `Engine.Displacement` | 998 cc | FACT | Very High | Kia + Carnoon |
| `Engine.MaxPower` | 55.897905 kW @ 6200 rpm | FACT | Very High | source raw 76 PS @ 6200 |
| `Engine.MaxTorque` | 95.124505 N·m @ 3750 rpm | FACT | Very High | source raw 9.7 kgf·m @ 3750 |
| `Drivetrain.TransmissionType` | 4AT | FACT | Very High | Kia + Carnoon + Danawa |
| `Wheel.TireSize` | 175/65R14 | FACT | Very High | Kia spec + Kia Trendy price |
| `Drivetrain.DriveLayout` | FWD | FACT | Medium | Danawa exact 2027 scope only |
| `Chassis.FrontSuspensionType` | MacPherson strut | FACT | Medium | Danawa exact 2027 scope only |
| `Chassis.RearSuspensionType` | torsion beam | FACT | Medium | Danawa exact 2027 scope only |

`76 PS → 55.897905 kW`, `9.7 kgf·m → 95.124505 N·m`는 exact unit normalization이므로 FACT를 유지한다.

### 14.4 DERIVED claims

`Wheel.NominalOuterRadius`:

```text
Input: 175/65R14
Sidewall = 175 × 0.65 = 113.75 mm
Rim diameter = 14 × 25.4 = 355.6 mm
Outer diameter = 355.6 + 2 × 113.75 = 583.1 mm
Nominal outer radius = 291.55 mm
Provenance = DERIVED
Method = TireRadiusFromMetricSize@1
```

`Performance.PowerToCurbMass` dry-run diagnostic:

```text
Input power = 55.897905 kW
Input curb mass = 0.975 t
Result = 57.331185 kW/t
Provenance = DERIVED
Method = PowerToCurbMass@1
```

이 값들은 곧바로 Chaos WheelRadius/EngineTorque 등 Runtime 값을 의미하지 않는다. Reference→CarFight mapping은 VB-P0-04에서 결정한다.

### 14.5 GAME_BIAS

```text
Count = 0
```

Research normalization만 수행했으므로 의도적 게임 조정값을 만들지 않았다. 이 결과가 정상이다.

### 14.6 Variant conflict dry-run

공식 Kia source에는 같은 1.0 gasoline에 다음이 동시에 존재한다.

```text
14-inch curb mass = 975 kg
16-inch curb mass = 1,010 kg

14-inch front/rear track = 1,406 / 1,415 mm
16-inch front/rear track = 1,394 / 1,403 mm

14-inch tire = 175/65R14
16-inch tire = 195/45R16
```

이를 source disagreement로 평균하지 않았다.

```text
ConflictType = VariantScopeMismatch
ResolutionPolicy = SplitVariant
Selected Reference identity = 14-inch / 175/65R14
Canonical curb mass = 975 kg
Unresolved conflict = 0
```

**PASS:** wheel/tire variant가 Reference Identity에 없으면 잘못된 합성이 발생할 수 있다는 점을 실제 데이터로 검증했고, exact variant scope로 정상 분리됐다.

### 14.7 Unknown fact dry-run

이번 source set에서 exact identity로 확정하지 못한 항목 예:

```text
Mass.Gross
Drivetrain.GearRatios
Drivetrain.FinalDriveRatio
Engine.RedlineRpm
Performance.ZeroTo100
Performance.TopSpeed
Performance.Braking100To0
Chassis.TurningRadius
```

이 항목은 0이나 추정값으로 채우지 않고 `UnknownFacts`에 남겼다.

**PASS:** "실차에 없는 값 = AI가 적당히 생성"이 아니라 이후 VB-P0-04에서 필요한 경우에만 `DERIVED` 또는 `GAME_BIAS`로 명시적으로 전환할 수 있다.

---

## 15. Fingerprint dry-run

Contract 검증을 위해 synthetic `EvidenceId/RecipeId/TargetPath`와 위 normalized subset으로 canonical payload를 구성했다.

```text
Contract: CFVREF-1
Normalization: CFVRN-1
Sample EvidenceFingerprint:
2d184efdcde75d80692d63d44a58aa28b9071f12c2460d7f4b98fde19a8f9ca5
```

이 hash는 Production Asset hash가 아니라 **canonicalization proof용 dry-run 값**이다.

검증:

```text
Sources array reverse       → same fingerprint PASS
Claims array reverse        → same fingerprint PASS
Access-time-only change     → fingerprint 대상에서 제외 PASS
Curb mass 975 → 1010 change → fingerprint changed PASS
Target binding change       → fingerprint changed by contract PASS
Conflict resolution change  → fingerprint changed by contract PASS
```

975→1010 변경 sample hash:

```text
e48bd87f24c2bf7e0930baed28987d9eab0a920a00cc349aa704ff18ffce0466
```

---

## 16. AI Proposal binding dry-run

실제 Asset/Profile mutation 없이 binding semantics만 검증했다.

가상 Proposal은 다음 상태를 snapshot한다.

```text
ExpectedEvidenceFingerprint = dry-run EvidenceFingerprint
ExpectedRecipeId             = exact companion RecipeId
ExpectedRecipeFingerprint    = preview baseline
ExpectedTargetDefinitionHash = preview baseline
ExpectedProfileFingerprint   = Base/Drivetrain/Handling/Performance 각각의 preview baseline
ConsumedClaimIds             = canonical claims only
ApprovalScopeHash            = BuilderProposalHash
```

Case 결과:

```text
A. Evidence unchanged + Recipe/Target/Profile unchanged
   → Commit precondition eligible PASS

B. Evidence fact 975→1010 after Preview
   → Evidence fingerprint mismatch → old proposal BLOCK PASS

C. Reference variant 14→16 after Preview
   → identity/fingerprint mismatch → old proposal BLOCK PASS

D. Consumed claim becomes unresolved material conflict
   → proposal BLOCK PASS

E. Recipe/Profile가 외부에서 변경됨
   → 기존 Recipe/Profile fingerprint contract로 BLOCK PASS

F. USER approval 없이 ProposalHash만 전달
   → existing approval contract로 BLOCK PASS
```

따라서 Evidence stale protection을 기존 Data Authoring stale/approval 체계 위에 additive하게 결합할 수 있다.

---

## 17. VB-P0-01 판정

**PASS — Design / Research Normalization**

고정된 계약:

```text
1. Reference identity는 ModelYear/Market/Powertrain/Trim/Variant scope를 가진다.
2. Citation은 Tier와 origin independence를 분리한다.
3. FACT/DERIVED/GAME_BIAS를 atomic claim 단위로 보존한다.
4. 단순 unit conversion은 FACT를 유지한다.
5. 없는 값은 UnknownFacts로 남긴다.
6. Identity/Variant mismatch는 평균하지 않고 split한다.
7. Confidence는 evidence support이고 conflict Block을 대체하지 않는다.
8. SHA-256 EvidenceFingerprint는 semantic research + target binding을 deterministic canonicalize한다.
9. Builder Proposal은 Evidence + existing Recipe/Target/Profile fingerprint + ProposalHash에 동시에 binding된다.
10. Research 단계는 Profile/VehicleData mutation을 수행하지 않는다.
```

실제 Kia Morning 2027 1.0 gasoline Trendy 14-inch Reference Set으로 normalization, variant split, Unknown 보존, deterministic fingerprint, stale proposal binding을 dry-run했고 계약상 모순을 발견하지 못했다.

다음 Gate:

```text
VB-P0-02 Builder Shell / Step State / Resume
```

VB-P0-02에서도 아직 Reference→Chaos numeric mapping을 구현하지 않는다. 그 mapping과 Builder-private Profile typed proposal commit은 VB-P0-04 owner다.

---

## 18. Changelog

### v0.1.3 - 2026-09-01

- 다음 차량부터 엔진 curve fidelity를 처음부터 조사할 수 있도록 `Engine.TorqueBand`, `Engine.PowerBand`, `Engine.TorqueCurvePoint` research vocabulary를 추가했다.
- `MaxTorque` scalar만 확보하고 vehicle-specific engine curve FACT가 있는 것처럼 취급하는 것을 금지했다. curve point가 없으면 Evidence에는 absence/Unknown을 유지하고 Proposal 단계의 DERIVED/GAME_BIAS와 분리한다.
- `Performance.PublishedTopSpeed`와 `Performance.TopSpeedLimiter`를 분리했다. 전자식 limiter가 걸린 published 최고속은 순수 power-limited top speed와 동일 semantic이 아니며 FinalRatio/engine physical inference의 직접 authority가 아니다.
- 전자식 최고속 제한은 Evidence FACT/context로 보존하되 Runtime 적용 여부는 Proposal/Gameplay policy가 소유하도록 authority 경계를 명확히 했다.

### v0.1.2 - 2026-09-01

- production 기본 Reference를 특정 실존 차량의 exact Model Year/Trim/Powertrain/Market Primary identity로 명확히 고정했다.
- USER의 차량 추천 요청을 정식 workflow로 인정하고, AI가 Data Availability / Builder Confidence / CarFight Value를 기준으로 exact 후보를 조사·추천하도록 계약을 추가했다.
- 제조사 제원, engine band, transmission gear ratios/final drive, mass/dimensions/wheel/performance와 variant confusion을 추천 핵심 evidence로 정의했다.
- 실존하지 않는 차량은 Vehicle Archetype의 현실적 range/distribution을 Primary fallback으로 사용하고, 실존 Reference는 Secondary/TraitOnly로 선택 사용할 수 있게 했다.
- 차급 평균값이 실존 차량의 missing FACT를 조용히 대체하는 것을 금지하고, Unknown → vehicle-specific DERIVED/GAME_BIAS → USER Review 순서를 명문화했다.
- 실제 차량 technical identity와 게임 내 이름/엠블럼/브랜드 presentation을 분리해 Evidence traceability를 유지하도록 했다.

### v0.1.1 - 2026-08-26

- VB-P0-01 PASS를 다시 열지 않고 실제 다단 변속기 Authoring 요구를 Reference Evidence vocabulary에 additive하게 통합했다.
- Transmission type/automatic, forward gear count/ratios, reverse ratio, final drive, up/downshift condition, gear change time, efficiency를 Drivetrain research semantic scope로 명시했다.
- Transmission claim도 기존 FACT/DERIVED/GAME_BIAS, Unknown, conflict, fingerprint 계약을 그대로 따르도록 했고 근거 없는 일반 변속기 값 생성과 Chaos default의 FACT 역해석을 금지했다.
- Evidence FactKey와 Chaos `TransmissionSetup` raw property identity를 분리하고 exact typed schema/API mapping은 VB-P0-04 owner로 유지했다.
- 기존 Kia Morning dry-run 결과는 당시 실제 조사 evidence로 그대로 보존하며 새 Transmission 항목을 소급해 조사·확정한 것처럼 기록하지 않았다.
- C++/UE Asset/Profile/VehicleData mutation은 수행하지 않았다.

### v0.1.0 - 2026-08-26

- VB-P0-01 Vehicle Reference Evidence exact logical schema를 신규 고정했다.
- Reference identity에 ModelYear/Market/Powertrain/Trim/WheelTireVariant scope를 포함하고 variant/identity mixing을 금지했다.
- Source Tier와 origin independence를 분리하고 citation 원문 전체 저장을 금지했다.
- FACT/DERIVED/GAME_BIAS atomic claim, Unknown fact, conflict/resolution, deterministic confidence 의미를 고정했다.
- SHA-256 `CFVREF-1 / CFVRN-1` Evidence Fingerprint canonicalization과 volatile field 제외 규칙을 고정했다.
- Builder Proposal을 EvidenceFingerprint + RecipeFingerprint + TargetDefinitionHash + private Profile fingerprints + ProposalHash에 결합하는 stale-safe 계약을 고정했다.
- The 2027 Kia Morning 1.0 gasoline Trendy 14-inch를 실제 research normalization dry-run으로 사용해 14/16-inch variant split, Unknown 보존, unit normalization, DERIVED tire radius, fingerprint order-independence와 stale binding을 PASS했다.
- C++/UE Asset/VehicleData/Profile/Recipe mutation은 수행하지 않았다.

---

## 19. Migration

- v0.1.3의 Engine Curve/Limiter vocabulary 추가는 기존 Evidence schema나 VB-P0-01 PASS를 무효화하지 않는다.
- v0.1.2의 Vehicle Selection/Reference Strategy 추가는 기존 Evidence schema나 VB-P0-01 PASS를 무효화하지 않는다.
- v0.1.1의 Transmission vocabulary 추가는 VB-P0-01 PASS를 무효화하거나 research dry-run 재실행을 요구하지 않는다.
- 기존 VehicleData/Recipe/Profile schema는 변경하지 않는다.
- 기존 `FCFAuthoringProposal`과 `FCFAuthoringCallContext`는 폐기하지 않고 Builder wrapper가 stale precondition을 additive하게 확장한다.
- 기존 shared Profile write 금지, raw SetField 금지, no-auto-save/no-auto-retry 정책은 그대로 유지한다.
- 이 문서의 class/struct 이름은 VB-P0-04 구현 전에 public/private placement와 final USTRUCT exposure를 한 번 검수하되 semantic field 계약을 임의 축약하지 않는다.
