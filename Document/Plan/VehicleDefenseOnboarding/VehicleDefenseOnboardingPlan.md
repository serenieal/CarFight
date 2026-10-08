# CF-FQ-053 VehicleDefenseData Routine Onboarding / Process Benchmark

- 문서 버전: v0.1.5
- 작성일: 2026-09-11
- 최근 갱신일: 2026-09-11
- 문서 상태: Historical + Retained Path / Done / Technical Complete / VDR-P0-03 Final Acceptance + Process Benchmark PASS
- Feature: `CF-FQ-053 VehicleDefenseData Routine Onboarding / Process Benchmark`
- Priority: P2
- 현재 정확한 Gate: `없음 — Routine 4-Gate Complete`
- Current System owner: main_game `Document/Systems/DataManagement/DataAssetAuthoring.md v1.6.0`
- Runtime System baseline: main_game `Document/Systems/Combat/VehicleDefense.md v1.0.0`
- Predecessor: `CF-FQ-052 DamageData Third-Type Onboarding / Reuse Verification` Done / Technical Complete
- 현재 단일 Active: `CF-FQ-039 Production UI Visual Rework` unchanged

---

## 1. 목적

이 Feature는 `UCFVehicleDefenseData`를 MissileGuidePreset + AmmoData + DamageData에 이은 네 번째 Production DataAsset Authoring 타입으로 onboarding한다.

동시에 CF-FQ-049~052에서 구축한 shared foundation과 현재 `DataAssetAuthoring.md §2.8`의 Routine 4-Gate가 실제 개발 공수를 줄이는지 검증하는 첫 Process Benchmark다.

이번 Feature의 성공 조건은 단순히 VehicleDefenseData 지원이 동작하는 것이 아니다.

```text
1. VehicleDefenseData fourth-type onboarding이 Technical Accepted 된다.
2. 기존 shared Preview / Review / TOCTOU / Durable / DACE 알고리즘을 재작성하지 않는다.
3. 과거 Feature의 검수 단계를 기계적으로 반복하지 않고 Routine 4-Gate로 닫는다.
4. CF-FQ-052 대비 Gate 수, correction/re-review, build/test 실행, 문서 churn과 실제 작업 시간을 측정해 속도 개선 여부를 명시적으로 판정한다.
```

안전성 기준을 낮추지 않는다. 이미 Accepted된 공용 구조를 반복 증명하지 않고 **VehicleDefenseData가 새로 추가하는 계약과 위험만 검증**한다.

---

## 2. 현재 baseline

### 2.1 Current Authoring foundation

현재 Production Authoring 지원 타입은 다음 세 타입이다.

```text
MissileGuidePreset
AmmoData
DamageData
```

세 타입 모두 `ReviewedMutationReady`이며, DamageData까지 DACE `ContractReady`와 explicit mixed operational admission이 Current 승격돼 있다.

네 번째 이후 타입은 main_game `DataAssetAuthoring.md §2.8`의 Routine Onboarding Standard를 적용한다.

### 2.2 VehicleDefenseData Source baseline

현재 Runtime 타입은 `UCFVehicleDefenseData : UPrimaryDataAsset`이다.

Data Management descriptor는 이미 다음 계약을 소유한다.

```text
ClassPath: /Script/CarFight_Re.CFVehicleDefenseData
Domain: Vehicle
IdentityPolicy: Required
IdentityResolver: ExplicitFName
IdentitySource: DefenseId
ValidationPolicy: NativeDataValidation
ValidationEntry: IsDataValid
DuplicateNamespace: ExactClassPath
```

현재 top-level authored field는 17개다.

```text
DefenseId
DefenseMassKg
bUseShield
MaximumShield
ShieldRegenerationDelaySeconds
ShieldRegenerationPerSecond
ArmorType
ArmorResistance
FrontArmorConfig
LeftArmorConfig
RightArmorConfig
RearArmorConfig
TopArmorConfig
BottomArmorConfig
ShieldComponentDamageScale
ArmorComponentDamageScale
IntegrityComponentDamageScale
```

6방향 Armor field는 `FCFDirectionalArmorConfig`를 사용하며 각 항목은 `MaximumArmor`, `DamageMultiplier`를 가진다.

Gate 1에서는 현재 Native DataValidation과 Authoring strict contract 사이를 교차검수한다. 기존 Runtime defensive clamp를 이유로 Authoring에서 비유한 값이나 잘못된 값을 임의 허용하지 않으며, 반대로 현재 Runtime 의미에 없는 신규 gameplay invariant도 임의 추가하지 않는다.

### 2.3 Persisted protection baseline

2026-09-11 fresh AssetDump에서 `CFVehicleDefenseData` persisted asset은 1개가 확인됐다.

```text
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test
DefenseId = VehicleDefense_Test
```

이 Asset은 현재 VehicleDefense/Fitting 검증과 Runtime 계약에 연결된 **read-only protected baseline**으로 취급한다.

CF-FQ-053의 disposable Create/Update/Save 검증 대상으로 재사용하지 않으며, 명시적 별도 승인 없이 수정·저장하지 않는다. Product canonical Defense Staging target set의 초기 가정은 explicit empty이며, 최종 exact 의미는 VDR-P0-00 Gate 1에서 현재 Runtime/Product 사용 상태와 함께 동결한다.

현재 AssetDump 기준 VehicleDefenseData에는 Object/SoftObject asset reference가 없다. 따라서 이번 Feature에서 새로운 UObject reference authoring 계약을 발명하지 않는다.

---

## 2.4 VDR-P0-00 Pre-Implementation Design Review — 2026-09-11

판정:

```text
P0: 0
blocking P1: 4
P2: 0
Result: HOLD
Source implementation: NOT STARTED
Asset mutation: 0
Build/Automation: 0
Exact next: VDR-P0-00 Contract Correction + Re-review
```

Fresh evidence:

```text
UCFVehicleDefenseData direct authored top-level field: exact17
semantic terminal leaf: exact23
numeric terminal leaf: exact20
FCFDirectionalArmorConfig container: exact6
nested armor leaf: exact12
persisted CFVehicleDefenseData: exact1
protected asset: /Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test
AssetDump reference field: exact0
Authoring/DataAssetStaging VehicleDefenseData source: exact0
```

### P1-1 — Nested Struct Reflection / DACE coverage seam 미동결

현재 shared `FCFDAContractGuard::BuildDirectReflectedSourceShapeDescriptor()`는 direct class의 top-level `CPF_Edit` property만 관측한다. 따라서 VehicleDefenseData에 적용하면 struct container 6개를 포함한 top-level exact17까지만 관측하고 `FrontArmorConfig.MaximumArmor` 같은 nested leaf exact12는 관측하지 않는다.

반면 `ValidateSourceAdapterMappingCoverage()`는 `PropertyKind=Struct` container 자체를 semantic leaf에서 제외한다. VehicleDefenseData의 DACE가 nested leaf를 descriptor에 넣지 않으면 12개 실제 authored leaf의 drift를 놓치고, descriptor에 넣으면 현재 direct Reflection observation exact17과 exact 비교가 실패한다.

기존 Missile path는 `MissileGuideConfig`만 별도 재귀 관측하는 전용 구현을 갖고 있으므로 이를 VehicleDefenseData 전용으로 복제하는 것은 금지한다.

교정 조건:

```text
- Type-neutral nested struct Reflection observation seam을 additive하게 정의한다.
- 기존 Missile/Ammo/Damage accepted descriptor/signature와 current behavior는 변경하지 않는다.
- VehicleDefenseData expected SourceShape는 top-level exact17 + nested exact12 = descriptor exact29를 관측할 수 있어야 한다.
- 기존 common mapping/signature/revision algorithm 자체를 다시 쓰지 않는다.
```

이 조건을 additive seam으로 만족하면 Routine 범위에 유지한다. 기존 accepted ContractGuard 의미 변경이나 per-type reflection algorithm 복제가 필요하다고 판명되면 `Architecture Gap HOLD`로 재분류한다.

### P1-2 — Nested physical JSON / mapping / fingerprint contract 미동결

현재 Plan은 top-level exact17만 열거하고, 6개 `FCFDirectionalArmorConfig`의 JSON physical shape와 semantic terminal leaf 경계를 확정하지 않았다.

교정 시 다음을 명시적으로 동결해야 한다.

```text
Payload top-level authored field: exact17
Payload struct container: exact6
nested leaf: exact12
semantic terminal leaf: exact23

각 ArmorConfig JSON object:
- MaximumArmor: Number / Required / NonNull
- DamageMultiplier: Number / Required / NonNull
- unknown/missing field fail-closed

DACE SourceShape candidate: exact29
DACE AdapterShape candidate: common root exact8 + Payload descriptor exact29 = exact37
DACE SourceAdapterMapping candidate: metadata exact6 + terminal/identity mapping = exact30
semantic fingerprint token candidate: metadata exact4 + payload semantic leaf exact23 = exact27
```

`DefenseId`는 top-level `StableLogicalId`와 `Payload.DefenseId` 양쪽에 FName semantic으로 결속하고, 22개의 나머지 semantic leaf는 Payload에 one-to-one mapping한다.

### P1-3 — Strict validation / inactive-value preservation contract 미동결

Native `ValidateDefenseDataContract()`는 `DefenseMassKg`에만 explicit finite 검사를 하고 다른 float는 대부분 `< 0` 검사다. Authoring JSON 계약은 serialized whole-record 안전성을 위해 이 차이를 명시적으로 다뤄야 한다. Runtime source를 이번 Feature에서 고치는 것은 요구하지 않는다.

교정 시 다음 authoring strict semantic을 동결한다.

```text
DefenseId
- Required FName semantic
- empty / NAME_None reject
- StableLogicalId == Payload.DefenseId by FName semantics

ArmorType
- exact case-sensitive token: Light | Standard | Heavy
- numeric alias / unknown token reject

numeric terminal exact20
- all finite Float32
- all >= 0
- parse/fingerprint/materialize에서 silent clamp 금지

shield cross-field invariant
- bUseShield=true && MaximumShield<=0 && ShieldRegenerationPerSecond>0 => reject
- 그 외 현재 Runtime에 없는 신규 shield invariant는 추가하지 않음

preservation
- bUseShield=false여도 MaximumShield / ShieldRegenerationDelaySeconds / ShieldRegenerationPerSecond의 valid authored raw 값은 보존
- runtime getter가 0을 반환한다는 이유로 materializer가 auto-zero하지 않음
- Shield/Armor/IntegrityComponentDamageScale은 현재 runtime consumer가 없어도 authored/persisted/fingerprint semantic에서 제외하거나 auto-zero하지 않음
- 6방향 ArmorConfig raw valid 값도 whole-record로 exact 보존
```

`FMath::Max` 기반 Runtime defensive getter는 그대로 유지하며 Authoring strict reject와 혼동하지 않는다.

### P1-4 — Provider / DACE / operational admission 전환 순서와 canonical target 미동결

Provider가 registry에 있다는 사실, mutation-ready라는 사실, DACE ContractReady와 mixed operational admission은 서로 다른 authority다. VehicleDefenseData가 어느 시점에 실제 mutation lane으로 들어가는지 exact 순서를 고정해야 한다.

교정 후보는 predecessor와 같은 순서다.

```text
TypeKey
SchemaId = CarFight.DataAsset.VehicleDefenseData
ClassPath = /Script/CarFight_Re.CFVehicleDefenseData
SchemaRevision = 1
AdapterContractRevision = 1
CanonicalStagingRoot = Authoring/DataAssetStaging/VehicleDefenseData
StableIdentity = Required / ExplicitFName / DefenseId

Gate 2 parse/current 단계
Provider = ReadOnlyPreviewReady
DACE = ContractNotReady
Apply callback = absent
Operational admission = absent

Gate 2 durable writer accepted 뒤
Provider = ReviewedMutationReady
DACE = ContractNotReady
Operational admission = absent

Gate 3 DACE bootstrap/current contract accepted 뒤
Provider = ReviewedMutationReady
DACE = ContractReady

Gate 3 mixed regression accepted 뒤
explicit mixed operational admission에 VehicleDefenseData 추가
```

Fresh repository + AssetDump evidence에서 VehicleDefenseData canonical Product Staging source는 exact0이고 persisted VehicleDefenseData는 protected exact1이다. 따라서 initial DACE canonical target set은 **declared explicit exact0**으로 동결하는 것이 후보이며, persisted protected exact1을 canonical target으로 오해하지 않는다.

### Review conclusion

현재 blocker들은 VehicleDefenseData type contract를 구현 전에 정확히 고정하기 위한 항목이다. Preview/Review/TOCTOU/Durable orchestration 재설계 필요성은 발견되지 않았다.

가장 큰 위험은 nested struct DACE observation이다. `Contract Correction + Re-review`에서 P1-1이 additive Type-neutral seam으로 닫히면 Routine Onboarding을 계속한다. 기존 accepted shared semantic을 바꿔야 한다면 즉시 Architecture Gap으로 분류하고 VDR-P0-01을 시작하지 않는다.

### 2.5 VDR-P0-00 Corrected Normative Contract

이 절은 v0.1.1의 blocking P1 exact4를 닫는 **Gate 1 normative freeze**다. 아래 계약은 VDR-P0-01~03 구현·검증의 기준이며, 후속 구현 편의를 위해 의미를 축소하거나 자동 보정하지 않는다.

#### 2.5.1 Source structural contract

VehicleDefenseData direct authored top-level은 exact17이며, 6개 `FCFDirectionalArmorConfig` 내부 authored leaf exact12를 포함한 semantic terminal leaf는 exact23이다.

```text
Source root class:
/Script/CarFight_Re.CFVehicleDefenseData

Nested struct type:
/Script/CarFight_Re.CFDirectionalArmorConfig

Direct top-level authored nodes: exact17
Struct container nodes: exact6
Nested authored leaf nodes: exact12
SourceShape descriptor total: exact29
Semantic terminal leaf: exact23
Numeric terminal leaf: exact20
```

각 방향 struct는 다음 Source row 의미를 갖는다.

```text
<Direction>ArmorConfig
PropertyKind = Struct
ReflectedTypePath = /Script/CarFight_Re.CFDirectionalArmorConfig
ContainerKind = Struct
NestedStructPath = /Script/CarFight_Re.CFDirectionalArmorConfig

<Direction>ArmorConfig.MaximumArmor
PropertyKind = Float
ContainerKind = Scalar
NestedStructPath = /Script/CarFight_Re.CFDirectionalArmorConfig

<Direction>ArmorConfig.DamageMultiplier
PropertyKind = Float
ContainerKind = Scalar
NestedStructPath = /Script/CarFight_Re.CFDirectionalArmorConfig
```

`<Direction>`은 `Front | Left | Right | Rear | Top | Bottom` exact6이다.

#### 2.5.2 Additive Type-neutral nested Reflection seam

현재 `BuildDirectReflectedSourceShapeDescriptor()`의 **direct-only 의미는 변경하지 않는다.** AmmoData/DamageData가 소비하는 기존 direct observation을 재정의하지 않고, Missile의 기존 `BuildReflectedSourceShapeDescriptor()`도 이번 onboarding을 이유로 재배선하지 않는다.

VDR-P0-01에서 필요하면 shared `FCFDAContractGuard`에 다음 성격의 새 Private additive seam을 추가한다.

```text
proposed function:
BuildRecursiveReflectedSourceShapeDescriptor(const UClass& SourceClass, ...)

behavior:
1. SourceClass가 직접 소유한 CPF_Edit property만 top-level node로 관측한다.
2. FStructProperty이면 struct container row를 먼저 보존한다.
3. 해당 UScriptStruct가 직접 소유한 CPF_Edit member를 dot path로 재귀 관측한다.
4. nested scalar row의 NestedStructPath는 그 member를 직접 소유한 struct path다.
5. nested struct container row의 ReflectedTypePath/NestedStructPath는 그 container 자신의 struct path다.
6. Array/Set/Map은 기존 recursive type-token 의미를 유지하고 element member를 임의로 펼치지 않는다.
7. Object/SoftObject reference를 따라가거나 asset을 load하지 않는다.
8. descriptor 생성은 read-only이며 parser/materializer/Preview/Review/TOCTOU/Durable 동작을 소유하지 않는다.
```

이 seam은 **shared semantic rewrite가 아니라 missing generic observation capability의 additive extension**으로 분류한다. 구현 Acceptance 조건은 다음과 같다.

```text
VehicleDefenseData recursive observation = corrected SourceShape exact29
Missile recursive observation canonical rows = existing Missile SourceShape exact30과 exact same
Ammo recursive observation canonical rows = existing direct observation exact8과 exact same
Damage recursive observation canonical rows = existing direct observation exact12와 exact same
existing Missile/Ammo/Damage accepted DACE snapshots/signatures rebaseline = 0
existing direct/Missile public-private call behavior change = 0
VehicleDefenseData 전용 Reflection walker 복제 = 0
```

위 parity가 성립하지 않거나 기존 accepted semantics 변경이 필요하면 즉시 `Architecture Gap HOLD`다.

#### 2.5.3 Strict JSON physical contract

VehicleDefenseData strict adapter는 whole-record required contract를 사용한다. Root common envelope exact8은 기존 provider 계약을 그대로 재사용한다.

```text
Root common descriptor: exact8
Payload top-level descriptor: exact17
Payload nested descriptor: exact12
AdapterShape total: exact37
```

Payload top-level physical contract:

```text
Payload.DefenseId                              String  / NameToken
Payload.DefenseMassKg                          Number  / Float
Payload.bUseShield                             Boolean / Bool
Payload.MaximumShield                          Number  / Float
Payload.ShieldRegenerationDelaySeconds         Number  / Float
Payload.ShieldRegenerationPerSecond            Number  / Float
Payload.ArmorType                              String  / EnumNameToken
Payload.ArmorResistance                        Number  / Float
Payload.FrontArmorConfig                       Object  / WholeRecordObject
Payload.LeftArmorConfig                        Object  / WholeRecordObject
Payload.RightArmorConfig                       Object  / WholeRecordObject
Payload.RearArmorConfig                        Object  / WholeRecordObject
Payload.TopArmorConfig                         Object  / WholeRecordObject
Payload.BottomArmorConfig                      Object  / WholeRecordObject
Payload.ShieldComponentDamageScale             Number  / Float
Payload.ArmorComponentDamageScale              Number  / Float
Payload.IntegrityComponentDamageScale          Number  / Float
```

각 ArmorConfig object는 exact2 child를 가진다.

```text
Payload.<Direction>ArmorConfig.MaximumArmor      Number / Float
Payload.<Direction>ArmorConfig.DamageMultiplier  Number / Float
```

모든 위 field는 `Required / NonNull`이다. Missing/unknown field, 잘못된 JSON kind와 nested child 누락/추가는 strict parse/serializer coverage에서 fail-closed한다. Struct container 자체는 semantic leaf가 아니며 nested exact2 child가 semantic leaf다.

#### 2.5.4 Source↔Adapter mapping contract

공용 `ValidateSourceAdapterMappingCoverage()`의 현재 semantics를 그대로 사용한다.

```text
Adapter-only metadata mapping: exact6
DefenseId -> StableLogicalId: exact1
Payload semantic leaf mapping: exact23
SourceAdapterMapping total: exact30
```

Adapter-only metadata exact6은 다음이다.

```text
SchemaId
SchemaRevision
AdapterContractRevision
DataAssetTypeClassPath
TargetObjectPath
BaseSemanticFingerprint
```

`DefenseId`는 `StableLogicalId`와 `Payload.DefenseId` 양쪽에 `SourceToAdapter / NameToken`으로 mapping한다. 나머지 semantic leaf exact22는 해당 `Payload.*` terminal path에 one-to-one mapping한다.

따라서 Source semantic leaf set exact23과 SourceToAdapter source set exact23, PayloadContainer를 제외한 Adapter terminal path exact30과 mapping Adapter path exact30이 exact 일치해야 한다.

#### 2.5.5 Semantic fingerprint contract

Fingerprint는 existing common metadata exact4와 VehicleDefenseData semantic leaf exact23만 포함한다.

```text
metadata token exact4:
SchemaId
SchemaRevision
AdapterContractRevision
DataAssetTypeClassPath

payload semantic token exact23:
DefenseId
DefenseMassKg
bUseShield
MaximumShield
ShieldRegenerationDelaySeconds
ShieldRegenerationPerSecond
ArmorType
ArmorResistance
FrontArmorConfig.MaximumArmor
FrontArmorConfig.DamageMultiplier
LeftArmorConfig.MaximumArmor
LeftArmorConfig.DamageMultiplier
RightArmorConfig.MaximumArmor
RightArmorConfig.DamageMultiplier
RearArmorConfig.MaximumArmor
RearArmorConfig.DamageMultiplier
TopArmorConfig.MaximumArmor
TopArmorConfig.DamageMultiplier
BottomArmorConfig.MaximumArmor
BottomArmorConfig.DamageMultiplier
ShieldComponentDamageScale
ArmorComponentDamageScale
IntegrityComponentDamageScale

Fingerprint token total: exact27
```

`TargetObjectPath`, `StableLogicalId` envelope duplicate projection, `BaseSemanticFingerprint`는 semantic payload fingerprint에 중복 포함하지 않는다. Float는 existing policy와 같은 IEEE754 Float32 semantic을 사용하고 `-0.0`은 `+0.0`으로 canonicalize한다.

#### 2.5.6 Strict validation and preservation contract

Authoring strict validation은 Runtime defensive getter보다 강하게 입력을 거절할 수 있지만 **새 gameplay formula/invariant를 만들지 않는다.** 모든 serialized numeric value에 finite를 요구하는 것은 JSON/fingerprint/durable representation safety 계약이며 Runtime 밸런스 변경이 아니다.

```text
DefenseId
- required
- NAME_None / empty token reject
- StableLogicalId == Payload.DefenseId by FName semantics
- FName semantic comparison은 case-insensitive
- fingerprint canonical text는 lowercase

ArmorType
- exact case-sensitive token only: Light | Standard | Heavy
- numeric alias reject
- unknown token reject

numeric terminal exact20
- 모두 finite Float32
- 모두 >= 0
- NaN / +Inf / -Inf reject
- parse/fingerprint/materialize silent clamp 금지

shield cross-field invariant
- bUseShield=true
  && MaximumShield<=0
  && ShieldRegenerationPerSecond>0
  => reject
- 이 외 현재 Runtime source에 없는 MaximumShield/Regen 관계를 새로 추가하지 않음

preservation
- bUseShield=false여도 valid MaximumShield 원본 보존
- bUseShield=false여도 valid ShieldRegenerationDelaySeconds 원본 보존
- bUseShield=false여도 valid ShieldRegenerationPerSecond 원본 보존
- Shield/Armor/IntegrityComponentDamageScale authored raw value 보존
- 6방향 MaximumArmor/DamageMultiplier authored raw value 보존
- runtime getter의 0 clamp/disabled projection을 materializer source로 사용하지 않음
```

`DefenseMassKg=0`, `MaximumArmor=0`, `DamageMultiplier=0`, component scale `0`은 현재 Runtime/Native contract에서 합법이므로 허용한다. `bUseShield=false`일 때 shield raw values를 자동 0으로 바꾸지 않는다.

#### 2.5.7 Provider / DACE / admission transition contract

VehicleDefenseData TypeKey와 revision baseline을 다음으로 동결한다.

```text
SchemaId = CarFight.DataAsset.VehicleDefenseData
DataAssetTypeClassPath = /Script/CarFight_Re.CFVehicleDefenseData
SchemaRevision = 1
AdapterContractRevision = 1
CanonicalStagingRoot = Authoring/DataAssetStaging/VehicleDefenseData
StableIdentityPolicy = Required
StableIdentityResolver = ExplicitFName
StableIdentitySource = DefenseId
DaceHistoryNamespace = DACE-VehicleDefenseData
```

전환 순서는 다음처럼 분리한다.

```text
VDR-P0-01 parse/current registration:
Provider = ReadOnlyPreviewReady
DACE = ContractNotReady
Apply callback = absent
Mixed operational admission = absent

VDR-P0-01 provider-local durable writer acceptance 뒤:
Provider = ReviewedMutationReady
DACE = ContractNotReady
Apply callback = present
Mixed operational admission = absent

VDR-P0-02 independent bootstrap/current DACE acceptance 뒤:
Provider = ReviewedMutationReady
DACE = ContractReady
Mixed operational admission = still absent

VDR-P0-02 mixed regression acceptance 뒤:
explicit mixed operational admission = MissileGuidePreset + AmmoData + DamageData + VehicleDefenseData
```

Gate 2의 durable Create/Update 검증은 provider-local operation + existing `CFDADurableCore`로 수행하며, mixed admission을 미리 열기 위한 이유로 사용하지 않는다.

#### 2.5.8 Canonical target / protected persisted boundary

Fresh repository/AssetDump baseline에서 canonical VehicleDefenseData Staging source는 exact0이고 persisted `CFVehicleDefenseData`는 protected exact1이다.

따라서 최초 provider/DACE 계약은 다음을 사용한다.

```text
CanonicalStagingTargetsDeclared = true
Canonical VehicleDefenseData target paths = exact0
Persisted protected VehicleDefenseData = exact1
Protected DA -> canonical target 자동 편입 = forbidden
Protected DA Apply/Save = 0
```

`/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test`는 current-state readback regression에는 사용할 수 있지만 mutation fixture로는 사용할 수 없다. Create/Update/Save 증명은 disposable test-owned target만 사용하고 종료 시 residue exact0을 요구한다.

#### 2.5.9 Gate 1 re-review result

Corrected contract를 current source/shared foundation과 다시 대조한 결과:

```text
P0: 0
blocking P1: 0
P2: 0
Result: PASS
Classification: Routine Onboarding retained
Architecture Gap: NOT ESTABLISHED
Source implementation: NOT STARTED
Asset mutation: 0
Build/Automation: 0
```

P1-1은 direct helper 의미를 보존한 새 Type-neutral recursive Reflection seam의 **additive contract + exact predecessor parity gate**로 닫혔다. 따라서 shared semantic rewrite나 VehicleDefense 전용 walker 복제는 요구되지 않는다.

P1-2는 SourceShape exact29 / AdapterShape exact37 / Mapping exact30 / Fingerprint exact27 및 nested object exact2 구조로 닫혔다.

P1-3은 numeric terminal exact20 finite/non-negative, exact existing shield cross-field invariant, disabled/raw-value preservation과 no-clamp authoring boundary로 닫혔다.

P1-4는 ReadOnlyPreviewReady → ReviewedMutationReady → DACE ContractReady → explicit mixed admission 순서와 canonical declared exact0 / protected persisted exact1 분리로 닫혔다.

현재 코드에서 `ValidateSourceAdapterMappingCoverage`, JSON recursive field observation, provider readiness 분리, canonical exact0 declaration과 explicit mixed admission authority는 이 계약을 수용할 기존 seam을 이미 제공한다. 새로 필요한 것은 nested Source Reflection의 additive observation seam과 VehicleDefenseData type-owned provider/DACE 구현이며, Preview/Review/TOCTOU/Durable orchestration 재설계는 필요하지 않다.

Benchmark checkpoint:

```text
Primary Gate completed: 1 / 4
Actual Correction + Re-review: 1
Correction reason: blocking P1 exact4 from first design review
Speculative/redundant Review Gate added: 0
Official Build executions: 0
Focused regression executions: 0
Affected predecessor/shared regression executions: 0
Source/Asset mutation through Gate 1: 0
```

Gate 1은 Technical Contract PASS로 닫는다. exact next는 `VDR-P0-01 Gate 2 — Typed Provider + Durable`이며, Gate 2 구현 중 recursive Reflection parity가 위 contract를 만족하지 못하면 즉시 Architecture Gap HOLD로 되돌린다.

### 2.6 VDR-P0-01 Gate 2 Implementation / Technical Validation — 2026-09-11

Gate 1 v0.1.2 normative contract를 기준으로 VehicleDefenseData typed provider와 durable path를 구현하고 fresh validation을 완료했다.

구현 범위:

```text
Type-owned production exact2:
- CFDAVehicleDefenseProvider.h v1.0.0
- CFDAVehicleDefenseProvider.cpp v1.0.0

Type-owned C++ tests exact2:
- CFDAVehicleDefenseTests.cpp v1.0.0
- CFDAVehicleDefenseApplyTests.cpp v1.0.0

Focused runner exact1:
- Tools/RunVDRP01Tests.ps1 v1.0.0

Shared additive touch exact4:
- CFDAContractGuard.h v1.6.0
- CFDAContractGuard.cpp v1.6.0
- CFDATypeDispatch.h v1.9.0
- CFDATypeDispatch.cpp v1.9.0
```

Shared touch의 의미는 제한적이다. `CFDAContractGuard`에는 nested USTRUCT authored field를 관측하는 Type-neutral recursive Reflection seam만 additive하게 추가했고 기존 direct helper와 Missile facade는 유지했다. `CFDATypeDispatch`는 fourth provider registration만 추가했으며 Preview/Review/TOCTOU/Durable orchestration이나 mixed operational admission 알고리즘은 변경하지 않았다.

Provider current implementation state:

```text
Production provider registry = exact4
MissileGuidePreset = ReviewedMutationReady / ContractReady
AmmoData = ReviewedMutationReady / ContractReady
DamageData = ReviewedMutationReady / ContractReady
VehicleDefenseData = ReviewedMutationReady / ContractNotReady

VehicleDefense canonical target set declared = true
VehicleDefense canonical target paths = exact0
Mixed operational admission = existing exact3 unchanged
VehicleDefense mixed admission = absent
```

VehicleDefense strict provider는 Gate 1에서 동결한 authored top-level exact17 + nested armor exact12 whole-record를 parse/serialize하고 semantic terminal exact23을 fingerprint한다. `bUseShield=false`일 때도 valid authored shield raw values를 extractor/materializer가 그대로 보존하며 Runtime getter clamp/disabled projection을 authoring source로 사용하지 않는다.

Recursive Reflection parity fresh evidence:

```text
VehicleDefense recursive SourceShape = exact29
Missile recursive = exact30 / existing accepted SourceShape exact same
Ammo recursive = exact8 / existing accepted SourceShape exact same
Damage recursive = exact12 / existing accepted SourceShape exact same
accepted predecessor DACE rebaseline = 0
VehicleDefense-specific Reflection walker duplication = 0
```

Durable/TOCTOU fresh evidence:

```text
Disposable VehicleDefense Create -> durable readback PASS
Disposable VehicleDefense Update -> durable readback PASS
bUseShield=false raw shield values persisted exact preservation PASS
source changed after Review -> BlockedBeforeMutation / mutation0 PASS
current semantic changed after Review -> BlockedBeforeMutation / mutation0 PASS
protected DA_VehicleDefense_Test Apply/Save = 0
Product canonical VehicleDefense Apply/Save = 0
fixture Content/Staging residue = 0
```

Fresh executable validation:

```text
Official UE 5.8 Editor Build
- job: f57daec50d484f7c8fc66ad0d39e4a6c
- Result: PASS / exit 0
- actual build executions: 1

VDR-P0-01 focused exact6
- job: 9376733bcb404167a4b5b979389e68c9
- Result: 6/6 PASS
- failure/missing/unexpected/duplicate terminal: 0/0/0/0

Affected CF-FQ-050 DACE exact15
- job: 6f088e1e354545958217a0a0fe8993ec
- Result: 15/15 PASS

Affected CF-FQ-052 DDO exact14
- job: 869f2e7c9c5f474181104af3497017ef
- Result: 14/14 PASS
- existing three-type operational admission regression 포함
```

첫 build 호출의 잘못된 repository alias와 첫 process 호출의 잘못된 runtime enum은 execution 시작 전 argument validation에서 차단됐으므로 actual Build/Automation 실행 횟수에 포함하지 않는다. 이후 canonical `main_game` + `powershell51` 경로로 실행한 fresh evidence만 Acceptance에 사용한다.

Gate 2 판정:

```text
P0: 0
blocking P1: 0
P2: 0
Result: Technical PASS
Primary Gate completed: 2 / 4
Architecture Gap: NOT ESTABLISHED
Shared algorithm semantic rewrite: 0
Prohibited shared algorithm duplication: 0
Actual Correction + Re-review total: 1
Speculative/redundant Review Gate added: 0
Official Build executions total: 1
Focused regression executions total: 1
Affected predecessor/shared regression executions total: 2
```

이번 Gate에서 새로운 P0/blocking P1이 발견되지 않았으므로 별도 mid-review/correction/re-review Gate를 추가하지 않는다. Routine 4-Gate 계약대로 바로 `VDR-P0-02 Gate 3 — DACE + Operational Admission`으로 전진한다.

Gate 3에서는 VehicleDefenseData DACE descriptor/production probe와 dedicated append-only bootstrap/history를 먼저 Accepted한 뒤 `ContractReady`로 올리고, 그 다음 mixed regression을 통과한 경우에만 explicit mixed operational admission을 exact4로 확장한다.

### 2.7 VDR-P0-02 Gate 3 Implementation / Technical Validation — 2026-09-11

Gate 2 v0.1.3 accepted provider/durable contract를 재작성하지 않고 VehicleDefenseData type-owned DACE와 explicit fourth-type operational admission만 추가했다.

구현 범위:

```text
Type-owned DACE production exact3:
- CFDAVehicleDefenseDace.h v1.0.0
- CFDAVehicleDefenseDace.cpp v1.0.0
- CFDAVehicleDefenseDaceBase.cpp v1.0.0

Type-owned Gate 3 C++ tests exact2:
- CFDAVehicleDefenseDaceTests.cpp v1.0.0
- CFDAVehicleDefenseMixedTests.cpp v1.0.0

Focused runner exact1:
- Tools/RunVDRP02Tests.ps1 v1.1.0

VehicleDefense provider transition:
- CFDAVehicleDefenseProvider.h/.cpp v1.1.0
- ReviewedMutationReady 유지
- DACE ContractNotReady -> ContractReady

Shared admission/projection touch:
- CFDAStagingOps.cpp v1.6.0
- Public/DataAuthoring/CFDAStagingOps.h v1.4.0
- CFDATypeDispatch.h/.cpp v1.10.0

Predecessor test-only projection correction:
- CFDADamageMixedTests.cpp v1.2.0
```

VehicleDefense DACE current contract:

```text
SourceShape = exact29
AdapterShape = exact37
SourceAdapterMapping = exact30
SemanticContract = exact14
Fingerprint production token = exact27
Accepted history = exact1
Accepted bootstrap = DACE-VehicleDefenseData-S1-A1-Bootstrap
SchemaRevision = 1
AdapterContractRevision = 1
Canonical VehicleDefense target set declared = true
Canonical VehicleDefense target paths = exact0
Migration state = no-delta / NoMigration / NotRequired
```

Accepted bootstrap의 component signature와 snapshot chain signature는 production canonicalization으로 fresh 재계산해 hard-coded accepted record와 exact 일치함을 Automation에서 확인했다. history는 `CFDAVehicleDefenseDaceBase.cpp`가 독립 append-only authority로 소유하며 Missile/Ammo/Damage accepted history는 수정하지 않았다.

Operational admission은 DACE exact3 pre-admission Acceptance가 PASS한 뒤에만 다음 explicit code-owned exact4로 전진했다.

```text
0 MissileGuidePreset
1 AmmoData
2 DamageData
3 VehicleDefenseData
```

Provider registry에서 자동 admission하지 않는다. 기존 first-three order와 Missile/Ammo/Damage three-type lifecycle은 보존했고, `CFDADamageMixedTests.cpp`의 current authority projection만 exact4로 갱신했다. 기존 `ThreeTypeDurable / ThreeTypeDuplicate / ThreeTypeStale`는 predecessor exact3 subset regression으로 그대로 유지했다.

Fourth-type mixed regression 결과:

```text
VehicleDefense-only operational admission -> Create1 PASS
VehicleDefense + Damage same textual StableLogicalId -> class-scoped coexistence PASS
VehicleDefense + Damage durable exact2 -> PASS
same TargetObjectPath across TypeKeys -> DuplicateTargetPath / blocked before mutation PASS
same textual ID across different classes -> DuplicateStableIdentity 없음 PASS
post-Review VehicleDefense source semantic change -> whole mixed batch BlockedBeforeMutation / durable0 PASS
Product canonical VehicleDefense Apply/Save = 0
protected DA_VehicleDefense_Test Apply/Save = 0
disposable Content/Staging residue = 0
```

Fresh validation evidence:

```text
Pre-admission Official UE 5.8 Editor Build
- job: 2e943ec5a0374deeb55e5aa5b262d260
- Result: PASS / exit 0

Pre-admission VehicleDefense DACE exact3
- job: 18539603f35a41a9a5e12c470ec12dac
- Result: 3/3 PASS

Final Gate 3 Official UE 5.8 Editor Build
- job: c441c381776e4ea2b027d66ce3221bb1
- Result: PASS / exit 0

VDR-P0-02 final focused exact7
- job: 67be3260fe4d4180a2b6f97a9a581df7
- Result: 7/7 PASS
- failure/missing/unexpected/duplicate terminal: 0/0/0/0

Affected CF-FQ-050 DACE exact15
- job: b8d7015c316444f8a6e295cb21bce15d
- Result: 15/15 PASS

Affected CF-FQ-052 DDO exact14
- job: e361e4be7df841efa02a1b4a7e6fd043
- Result: 14/14 PASS
- updated exact4 OperationalAdmission + predecessor three-type Durable/Duplicate/Stale 포함
```

Gate 3 판정:

```text
P0: 0
blocking P1: 0
P2: 0
Result: Technical PASS
Primary Gate completed: 3 / 4
Architecture Gap: NOT ESTABLISHED
Shared algorithm semantic rewrite: 0
Prohibited shared algorithm duplication: 0
Actual Correction + Re-review total: 1
Speculative/redundant Review Gate added: 0
Official Build executions total: 3
Focused regression executions total: 3
Affected predecessor/shared regression executions total: 4
```

Gate 3에서 새 P0/blocking P1이 발견되지 않았으므로 별도 mid-review/correction/re-review를 추가하지 않는다. exact next는 `VDR-P0-03 Gate 4 — Final Acceptance + Process Benchmark`다.

Gate 4는 새 onboarding 구현을 추가하지 않고 final executable candidate, protected persisted VehicleDefense readback, disposable residue0, shared-core semantic diff, Current System promotion과 Process Benchmark 최종 판정만 소유한다.

### 2.8 VDR-P0-03 Gate 4 Final Acceptance + Process Benchmark — 2026-09-11

Gate 3 final executable candidate 뒤 executable Source mutation이 0이므로 `DataAssetAuthoring.md §2.8.3`의 검증 재사용 원칙에 따라 Build/Automation을 반복하지 않았다. Final executable evidence는 다음 Gate 3 accepted 결과를 그대로 승계한다.

```text
Official UE 5.8 Editor Build
- c441c381776e4ea2b027d66ce3221bb1
- PASS / exit0

VDR-P0-02 focused exact7
- 67be3260fe4d4180a2b6f97a9a581df7
- 7/7 PASS

Affected CF-FQ-050 DACE exact15
- b8d7015c316444f8a6e295cb21bce15d
- 15/15 PASS

Affected CF-FQ-052 DDO exact14
- e361e4be7df841efa02a1b4a7e6fd043
- 14/14 PASS
```

Gate 4 fresh persisted AssetDump에서 protected baseline을 다시 읽었다.

```text
Asset:
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test.DA_VehicleDefense_Test

Class: CFVehicleDefenseData
Persisted asset count: exact1
Authored top-level field: exact17
Reference field: exact0
AssetDump errors: 0
Git .uasset dirty: 0

DefenseId = VehicleDefense_Test
DefenseMassKg = 100
bUseShield = true
MaximumShield = 100
ShieldRegenerationDelaySeconds = 5
ShieldRegenerationPerSecond = 10
ArmorType = Standard
ArmorResistance = 100
Front = 100 / 1.0
Left = 100 / 1.2
Right = 100 / 1.2
Rear = 100 / 1.5
Top = 100 / 1.3
Bottom = 100 / 1.6
ShieldComponentDamageScale = 0
ArmorComponentDamageScale = 0.25
IntegrityComponentDamageScale = 1.0
```

AssetDump generation fingerprint는 readback generation 관측값이며 pre-feature fingerprint equality를 acceptance 기준으로 사용하지 않는다. authored values와 Git `.uasset` dirty0을 authoritative protection evidence로 사용해 protected `DA_VehicleDefense_Test` onboarding Apply·Save 0을 확인했다.

Shared-core semantic diff audit:

```text
CFDADurableCore.cpp: diff0
CFDAStagingApply.cpp: diff0

CFDAContractGuard.cpp/.h:
- Type-neutral nested USTRUCT recursive Reflection additive seam only
- existing direct/Missile facade semantic change0

CFDATypeDispatch.cpp/.h:
- VehicleDefense provider exact4 registration/readiness projection only

CFDAStagingOps.cpp + Public/DataAuthoring/CFDAStagingOps.h:
- explicit operational allowlist exact3 -> exact4 projection only

Shared algorithm semantic rewrite: 0
Prohibited shared algorithm duplication: 0
Architecture Gap: NOT ESTABLISHED
```

Process Benchmark final measurement:

| 항목 | CF-FQ-052 DamageData | CF-FQ-053 VehicleDefenseData | 결과 |
| --- | ---: | ---: | --- |
| Primary Gate | 6 | 4 | 감소 |
| actual Correction + Re-review | 반복 review/correction flow | 1회 / Gate 1 실제 blocking P1 exact4 | defect 기반 삽입 |
| redundant defect-free Review Gate | 복수 세부 review | 0 | 감소 |
| type-owned Production | exact5 / 1,574 LOC | exact5 / 1,613 LOC | +39 / 약 +2.5% |
| feature-owned C++ tests | exact4 / 2,808 LOC | exact4 / 2,266 LOC | -542 / 약 -19.3% |
| shared touched production | exact5 | exact6 | nested USTRUCT additive seam 때문에 +1 |
| shared semantic rewrite | 0 | 0 | 보존 |
| prohibited duplication | 0 | 0 | 보존 |
| ActiveWork projection increments | +16 | +6 | 약 62.5% 감소 |
| FeatureQueue projection increments | +16 | +6 | 약 62.5% 감소 |

VehicleDefense는 DamageData exact12보다 큰 SourceShape exact29(top-level17 + nested12)를 가진다. type-owned Production LOC도 Damage보다 약 2.5% 많으므로 단순한 타입이라서 빨라진 결과로 보지 않는다. 그럼에도 Primary Gate 6→4, defect-free 반복 Review 0, C++ test LOC 약 19.3% 감소, ActiveWork/FeatureQueue projection churn 각각 약 62.5% 감소, shared semantic rewrite0으로 완료했다.

CF-FQ-052의 신뢰 가능한 active-work elapsed baseline을 복원하지 못했으므로 분 단위 wall-clock 비율은 만들지 않는다. 구조적 작업량과 review/test/document churn 기준 최종 분류는 다음과 같다.

```text
Process Benchmark: Faster Confirmed
P0: 0
blocking P1: 0
P2: 0
Primary Gate: 4 / 4 Complete
Actual Correction + Re-review total: 1
Redundant Review Gate: 0
Official Build executions total: 3
Focused regression executions total: 3
Affected predecessor/shared regression executions total: 4
Shared algorithm semantic rewrite: 0
Prohibited shared algorithm duplication: 0
Architecture Gap: NOT ESTABLISHED
```

VehicleDefenseData는 Current fourth production authoring type으로 `Document/Systems/DataManagement/DataAssetAuthoring.md v1.6.0`에 승격했다. Production registry와 mixed operational admission은 MissileGuidePreset + AmmoData + DamageData + VehicleDefenseData explicit exact4이며 VehicleDefense는 `ReviewedMutationReady / DACE ContractReady / accepted history exact1 / canonical target exact0`이다.

Lifecycle closure:

```text
G0 Evidence: PASS
G1 Current Knowledge Promotion: PASS
G2 Current Route Cleanup: PASS
G3 Reference Preservation: PASS
G4 Historical: PASS
G5 Physical Move: Deferred
Placement: Historical + Retained Path
Current owner: Document/Systems/DataManagement/DataAssetAuthoring.md v1.6.0
Next Gate: none
Current single Active CF-FQ-039: unchanged
```

---

## 3. Routine 4-Gate

### VDR-P0-00 — Gate 1: Contract Freeze + Benchmark Baseline

다음을 동결한다.

```text
- authored field / nested struct의 persistence 의미
- DefenseId StableLogicalId semantic
- strict validation과 현재 cross-field invariant
- persisted DA_VehicleDefense_Test 보호 범위
- canonical DACE target 의미
- provider/DACE readiness 전환 순서
- CF-FQ-052 대비 비교 가능한 Process Benchmark 기준
```

이 Gate에서 Source implementation은 시작하지 않는다. 실제 P0/blocking P1이 발견되면 그 항목만 Correction + Re-review하고, 문제도 없는데 별도 Review Gate를 추가하지 않는다.

### VDR-P0-01 — Gate 2: Typed Provider + Durable

VehicleDefenseData type-owned 범위만 구현한다.

```text
- schema / parser / serializer / semantic fingerprint
- extractor / current-state resolver / materializer
- provider registration + readiness
- disposable Create/Update durable readback
- source/current TOCTOU focused regression
```

기존 `CFDADurableCore`와 common Preview/Review/approval 흐름을 재사용한다.

### VDR-P0-02 — Gate 3: DACE + Operational Admission

다음을 구현·검증한다.

```text
- VehicleDefenseData type-owned DACE descriptor / production probe
- dedicated append-only accepted bootstrap/history
- canonical target compatibility / migration state
- explicit mixed operational admission에 VehicleDefenseData 추가
- cross-TypeKey isolation / StableLogicalId / TargetObjectPath duplicate / stale regression
```

기존 Missile/Ammo/Damage accepted history와 provider behavior를 변경하지 않는다.

### VDR-P0-03 — Gate 4: Final Acceptance + Process Benchmark

최종 executable candidate에서 필요한 검증만 수행한다.

```text
- Official UE 5.8 Editor Build
- VDR focused regression
- 실제 변경 영향을 받는 predecessor/shared regression
- protected persisted VehicleDefenseData readback
- disposable fixture residue 0
- shared-core semantic diff audit
- Current System promotion
- Process Benchmark 최종 판정
```

USER Visual/Feel Gate는 기본 범위가 아니다. 이번 onboarding이 Runtime gameplay 의미를 변경해야 한다면 Routine 범위를 벗어난 것으로 보고 별도 판단한다.

---

## 4. Process Benchmark

이번 Feature는 기능 결과와 별개로 작업 효율을 측정한다.

### 4.1 기록 항목

각 Gate는 가능한 범위에서 다음 값을 기록한다.

```text
- Gate 진입/종료 KST
- Primary Gate 완료 수
- Correction + Re-review 발생 수와 실제 defect 사유
- Official Build 실행 수
- focused regression 실행 수
- predecessor/shared affected regression 실행 수
- 대표 Plan / ActiveWork / FeatureQueue / Systems 상태 projection 변경 수
- type-owned production file/LOC 규모
- shared foundation touch file 수와 touch 사유
- shared algorithm semantic rewrite 수
- prohibited shared algorithm duplication 수
```

벽시계 시간은 Editor lifecycle blocker, 사용자 대기, 외부 도구 장애 같은 비개발 대기시간과 가능한 한 구분한다.

### 4.2 CF-FQ-052 비교 baseline

CF-FQ-052는 기반 구축기의 마지막 third-type 검증으로 `DDO-P0-00~05`의 6개 Primary Gate를 사용했고, 여러 Gate에서 Pre/Post review와 Correction + Re-review가 반복됐다.

최종 구조 측정은 다음 Accepted evidence를 비교 기준으로 사용한다.

```text
DamageData type-owned Production: 5 files / 1,574 physical LOC
DamageData feature-owned C++ tests: 4 files / 2,808 LOC
Shared touch: 5 files
Shared core semantic rewrite: 0 required
Prohibited shared algorithm duplication: 0
```

VDR-P0-00에서 Git/document evidence로 비교 가능한 CF-FQ-052 active-work elapsed baseline을 복원할 수 있으면 시간 비교 기준도 고정한다. 비교 가능한 active time을 신뢰성 있게 분리할 수 없으면 임의 시간을 만들지 않고 구조적 작업량 지표와 이번 Feature의 실제 elapsed를 함께 보고한다.

### 4.3 최종 속도 판정

VDR-P0-03은 반드시 다음 중 하나로 결론낸다.

```text
Faster Confirmed
- Routine 4-Gate로 완료
- shared algorithm rewrite 0
- 실제 defect 없는 반복 검수/문서 Gate 확장 0
- CF-FQ-052보다 Gate/검증/문서 churn이 명확히 감소
- 비교 가능한 active elapsed가 있으면 시간도 감소

Inconclusive
- 외부 lifecycle/tool blocker 또는 비교 baseline 부족 때문에 속도 비교가 신뢰할 수 없음

Process Failure
- architecture gap이 없는데 반복 review/document/regression 때문에 장기화

Architecture Gap
- 새 타입 지원을 위해 shared Preview/Review/TOCTOU/Durable/ContractGuard 알고리즘 변경 또는 타입별 복제가 필요
```

“새 DataAsset 타입은 원래 오래 걸린다”는 문구로 Process Failure를 정상화하지 않는다.

---

## 5. Stop Rules

다음 조건이면 현재 Routine Gate를 즉시 HOLD한다.

```text
1. shared Preview / Review / TOCTOU / Durable / ContractGuard 알고리즘의 semantic rewrite가 필요하다.
2. 기존 공용 알고리즘을 VehicleDefenseData 전용으로 복제해야 한다.
3. protected DA_VehicleDefense_Test를 온보딩 편의를 위해 수정/Save해야 한다.
4. Runtime 방어 피해 공식이나 밸런스 의미 변경이 필요하다.
5. 새 Reflection/reference/migration 의미가 발견됐는데 계약 동결 없이 구현을 계속하려 한다.
```

P0 또는 blocking P1이 실제 발견된 경우에만 해당 defect의 Correction + Re-review를 삽입한다.

---

## 6. 보호 범위 / Non-Goals

보호한다.

```text
- 현재 single Active CF-FQ-039
- 기존 main_game 병렬 dirty
- persisted DA_VehicleDefense_Test
- MissileGuidePreset / AmmoData / DamageData Current provider + DACE accepted history
- CF-FQ-049~052 Accepted shared foundation
- VehicleDefense Runtime damage/shield/armor semantics
```

이번 Feature에서 하지 않는다.

```text
- VehicleDefense gameplay 공식 재설계
- 방어 밸런스 튜닝
- 기존 Defense DataAsset 자동 수정/정리
- CombatFx 중복 CombatFxId 교정
- Projectile/Weapon/Sensor 등 다른 타입 동시 onboarding
- hybrid shared/legacy physical ownership debt의 선제 리팩터링
- UI/Visual 작업
```

---

## 7. 현재 완료 상태

`CF-FQ-053`은 P2 / Done / Technical Complete이며 Routine 4-Gate를 모두 완료했다. 현재 구현 owner는 `Document/Systems/DataManagement/DataAssetAuthoring.md v1.6.0`이고 대표 Plan은 Historical + Retained Path로 보존한다.

```text
VDR-P0-00 Gate 1: PASS
VDR-P0-01 Gate 2: Technical PASS
VDR-P0-02 Gate 3: Technical PASS
VDR-P0-03 Gate 4: Final Acceptance PASS
Process Benchmark: Faster Confirmed
Provider: ReviewedMutationReady
DACE: ContractReady
Accepted VehicleDefense history: exact1
Mixed operational admission: explicit exact4
Protected DA_VehicleDefense_Test: read-only / Apply·Save 0
P0 / blocking P1 / P2: 0 / 0 / 0
Current single Active CF-FQ-039: unchanged
Exact next: none
```

완료 당시 Build·Automation·AssetDump·benchmark 상세 evidence는 이 Plan이 소유하고, 현재 동작 판단은 Systems v1.6.0과 실제 Source/Asset을 우선한다.

---

## 8. Changelog / Migration

### v0.1.5 - 2026-09-11

- `VDR-P0-03 Gate 4 — Final Acceptance + Process Benchmark`를 `P0 0 / blocking P1 0 / P2 0 / PASS`로 완료하고 CF-FQ-053을 Done / Technical Complete / Historical + Retained Path로 닫았다.
- Gate 3 final executable candidate 뒤 Source mutation0이므로 final Build `c441c381776e4ea2b027d66ce3221bb1`, VDR7 `67be3260fe4d4180a2b6f97a9a581df7`, DACE15 `b8d7015c316444f8a6e295cb21bce15d`, DDO14 `e361e4be7df841efa02a1b4a7e6fd043` Accepted evidence를 재사용하고 Gate 4에서 Build/Automation을 반복하지 않았다.
- fresh AssetDump에서 protected `DA_VehicleDefense_Test` persisted exact1 / authored exact17 / reference exact0 / errors0과 baseline authored values를 확인했고 Git `.uasset` dirty0으로 onboarding Apply·Save0을 재확인했다.
- shared diff audit은 `CFDADurableCore.cpp`와 `CFDAStagingApply.cpp` diff0, ContractGuard additive recursive Reflection seam, TypeDispatch registration/readiness와 StagingOps explicit allowlist projection만 확인했다. shared algorithm semantic rewrite0 / prohibited duplication0 / Architecture Gap 없음이다.
- Process Benchmark는 `Faster Confirmed`다. VehicleDefense Production은 exact5/1,613 LOC로 Damage exact5/1,574보다 약 2.5% 크고 SourceShape29로 더 복잡하지만 Primary Gate6→4, C++ test LOC 2,808→2,266, ActiveWork/FeatureQueue projection increments 각각 +16→+6, redundant Review0으로 process churn이 감소했다.
- 신뢰 가능한 CF-FQ-052 active elapsed baseline이 없어 wall-clock 비율은 주장하지 않는다.
- VehicleDefenseData를 Current fourth production authoring type으로 `Document/Systems/DataManagement/DataAssetAuthoring.md v1.6.0`에 승격했다. G0~G4 PASS, G5 Deferred, exact next 없음이다.

Migration: CF-FQ-053은 현재 착수 대상이 아니다. 현재 VehicleDefense authoring 동작 판단은 `DataAssetAuthoring.md v1.6.0`과 실제 Source를 우선하고 이 retained Plan은 완료 당시 evidence/benchmark 확인에만 사용한다. fifth+ DataAsset onboarding은 Systems §2.8 Routine 4-Gate를 사용한다.

### v0.1.4 - 2026-09-11

- `VDR-P0-02 Gate 3 — DACE + Operational Admission`을 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 완료했다.
- VehicleDefense DACE SourceShape exact29 / AdapterShape exact37 / SourceAdapterMapping exact30 / SemanticContract exact14와 independent accepted history exact1 `DACE-VehicleDefenseData-S1-A1-Bootstrap`을 추가하고 provider DACE readiness를 `ContractReady`로 승격했다.
- pre-admission Build `2e943ec5a0374deeb55e5aa5b262d260` PASS + VehicleDefense DACE exact3 `18539603f35a41a9a5e12c470ec12dac` 3/3 PASS 뒤에만 mixed operational authority를 explicit exact4로 확장했다.
- final Build `c441c381776e4ea2b027d66ce3221bb1` PASS, VDR Gate3 exact7 `67be3260fe4d4180a2b6f97a9a581df7` 7/7, affected DACE `b8d7015c316444f8a6e295cb21bce15d` 15/15, affected DDO `e361e4be7df841efa02a1b4a7e6fd043` 14/14 PASS를 확보했다.
- fourth-type regression에서 class-scoped same textual StableLogicalId coexistence, global TargetObjectPath duplicate fail-closed, post-Review source stale mutation0와 disposable residue0을 확인했다. Product canonical VehicleDefense와 protected `DA_VehicleDefense_Test` Apply/Save는 0이다.
- shared Preview/Review/TOCTOU/Durable/ContractGuard algorithm semantic rewrite와 per-type algorithm duplication은 0이다. Gate 3에서 새 blocker가 없어 추가 Review/Correction Gate를 만들지 않았다.
- cumulative Process Benchmark는 Primary Gate 3/4, Correction+Re-review total1, redundant Review0, Official Build3, focused regression3, affected regression4다.
- exact next를 `VDR-P0-03 Gate 4 — Final Acceptance + Process Benchmark`로 전진했다.

Migration: VDR-P0-03은 Gate 1~3 accepted implementation을 재작성하지 않는다. final executable validation, protected persisted VehicleDefense readback, disposable residue0, shared-core semantic diff audit, Current System promotion과 Process Benchmark 최종 분류만 수행한다. 실제 P0/blocking P1이 새로 발견된 경우에만 Correction + Re-review를 삽입한다.

### v0.1.3 - 2026-09-11

- `VDR-P0-01 Gate 2 — Typed Provider + Durable` 구현과 fresh validation을 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 완료했다.
- VehicleDefense type-owned provider exact2와 C++ test exact2, focused runner exact1을 추가하고 production provider registry를 exact4로 확장했다. VehicleDefense는 `ReviewedMutationReady / ContractNotReady`, canonical target declared exact0이며 mixed operational admission은 existing exact3를 유지한다.
- shared touch는 `CFDAContractGuard` recursive Reflection additive seam과 `CFDATypeDispatch` provider registration exact4로 한정했다. shared algorithm semantic rewrite와 VehicleDefense 전용 Reflection walker duplication은 0이다.
- recursive Reflection은 VehicleDefense exact29, Missile exact30, Ammo exact8, Damage exact12 predecessor parity를 fresh Automation으로 확인했다.
- Official Build `f57daec50d484f7c8fc66ad0d39e4a6c` PASS, VDR focused `9376733bcb404167a4b5b979389e68c9` 6/6, affected DACE `6f088e1e354545958217a0a0fe8993ec` 15/15, affected DDO `869f2e7c9c5f474181104af3497017ef` 14/14 PASS를 확보했다.
- disposable Create/Update durable readback, disabled shield raw-value preservation, source/current stale mutation0와 fixture residue0을 확인했다. protected/Product VehicleDefense Apply·Save는 0이다.
- Gate 2에서 새 blocker가 없어 추가 review/correction gate를 만들지 않았고 exact next를 `VDR-P0-02 Gate 3 — DACE + Operational Admission`으로 전진했다.

Migration: VDR-P0-02는 Gate 2 provider/parser/durable contract를 재작성하지 않는다. VehicleDefense DACE descriptor/probe + dedicated append-only accepted bootstrap/history를 먼저 Accepted해 `ContractReady`로 전진한 뒤, mixed regression PASS 후에만 explicit operational admission을 exact4로 확장한다. 실제 P0/blocking P1이 새로 발견된 경우에만 Correction + Re-review를 삽입한다.

### v0.1.2 - 2026-09-11

- `VDR-P0-00 Contract Correction + Re-review`를 완료해 `P0 0 / blocking P1 0 / P2 0 / PASS`로 Gate 1을 닫았다.
- Source structural contract를 top-level exact17 + nested exact12 = SourceShape exact29, semantic terminal exact23, numeric terminal exact20으로 동결했다.
- strict physical/mapping/fingerprint 계약을 AdapterShape exact37 / SourceAdapterMapping exact30 / fingerprint token exact27로 동결했다.
- existing direct Reflection/Missile facade를 바꾸지 않는 Type-neutral `BuildRecursiveReflectedSourceShapeDescriptor` additive seam과 Missile exact30 / Ammo exact8 / Damage exact12 predecessor parity를 implementation gate로 고정했다. parity 실패 또는 accepted semantic 변경 필요 시 Architecture Gap HOLD다.
- numeric exact20 finite/non-negative, current shield cross-field invariant, `bUseShield=false` raw shield value 보존, runtime defensive clamp와 authoring materialization 분리를 동결했다.
- provider readiness를 ReadOnlyPreviewReady → ReviewedMutationReady → DACE ContractReady → explicit mixed admission 순서로 분리하고 canonical targets declared exact0 / persisted protected exact1을 동결했다.
- Gate 1 실제 Correction + Re-review는 1회이며 Source/Asset/Build/Automation mutation은 0이다. speculative Review Gate는 추가하지 않았다.
- exact next를 `VDR-P0-01 Gate 2 — Typed Provider + Durable`로 전진했다.

Migration: VDR-P0-01은 v0.1.2 normative contract를 그대로 구현한다. VehicleDefenseData 전용 Reflection walker를 만들지 말고 shared additive recursive seam의 predecessor parity를 먼저 증명한다. Provider-local durable acceptance 전 mixed operational admission을 열지 않으며 protected `DA_VehicleDefense_Test`를 mutation fixture로 사용하지 않는다.

### v0.1.1 - 2026-09-11

- `VDR-P0-00 Pre-Implementation Design Review`를 수행해 `P0 0 / blocking P1 4 / P2 0 / HOLD`로 기록했다.
- fresh AssetDump로 persisted VehicleDefenseData exact1, authored field exact17, reference field exact0와 protected `DA_VehicleDefense_Test`를 재확인했다. repository Authoring에는 VehicleDefenseData Staging source exact0이다.
- blocking P1은 Type-neutral nested Reflection coverage seam, nested JSON/mapping/fingerprint exact contract, strict validation + inactive-value preservation, provider/DACE/admission + canonical exact0 전환 순서다.
- current direct Reflection helper가 nested exact12를 관측하지 않는 capability gap을 확인했다. correction에서 기존 accepted semantics를 바꾸지 않는 additive seam으로 해결 가능하면 Routine을 유지하고, shared semantic rewrite 또는 per-type 복제가 필요하면 Architecture Gap HOLD로 재분류한다.
- Source/Asset/Build/Automation mutation은 수행하지 않았고 `VDR-P0-01`은 시작하지 않았다.

Migration: 재개 시 `VDR-P0-00 Contract Correction + Re-review`부터 수행한다. blocking P1 exact4를 모두 닫기 전 provider/source 구현을 시작하지 않는다. 특히 nested Reflection을 VehicleDefenseData 전용 복제로 해결하지 않는다.

### v0.1.0 - 2026-09-11

- `CF-FQ-053 VehicleDefenseData Routine Onboarding / Process Benchmark`를 P2 / Ready 정식 Plan으로 생성했다.
- `DataAssetAuthoring.md v1.5.15 §2.8`의 4th+ Routine Onboarding Standard를 실행 기준으로 연결했다.
- VehicleDefenseData authored top-level 17 fields, Required/ExplicitFName `DefenseId`, Native DataValidation과 persisted `DA_VehicleDefense_Test` read-only protection baseline을 기록했다.
- 기능 Acceptance와 별개로 CF-FQ-052 대비 Gate/correction/build/test/document/shared-touch/elapsed 지표를 측정하고 `Faster Confirmed / Inconclusive / Process Failure / Architecture Gap` 중 하나로 종료하도록 고정했다.
- Source/Asset/Build/Automation mutation은 수행하지 않았다.

Migration: CF-FQ-053 착수 시 과거 CF-FQ-049~052의 세부 Gate를 복제하지 않는다. `VDR-P0-00 Gate 1 — Contract Freeze + Benchmark Baseline`에서 시작하고, P0/blocking P1이 실제 발견된 경우에만 Correction + Re-review를 삽입한다. shared algorithm semantic rewrite 또는 타입별 복제가 필요하면 Routine 구현을 계속하지 말고 Architecture Gap HOLD로 전환한다.
