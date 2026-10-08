# CF-FQ-051 Data Asset Multi-Type Onboarding — AmmoData Second Pilot

- 문서 버전: v0.3.22
- 최근 갱신일: 2026-09-10
- 문서 상태: Historical + Retained Path / CF-FQ-051 Technical Complete / DAO-P0-06 Final Audit Correction + Re-review PASS / P0 0 / blocking P1 0 / P2 1 non-blocking
- Feature ID: `CF-FQ-051`
- 우선순위: P2
- 대표 약어: `DAO`
- 현재 정확한 Gate: `Complete / no remaining CF-FQ-051 Gate`
- Current System owner: main_game `Document/Systems/DataManagement/DataAssetAuthoring.md v1.4.1`
- 선행 완료 Feature: `CF-FQ-045`, `CF-FQ-049`, `CF-FQ-050`
- 현재 단일 Active 보존: `CF-FQ-039 Production UI Visual Rework`

---

## 1. 목적

`CF-FQ-049 Data Asset Staging·Batch Authoring`과 `CF-FQ-050 Data Asset Contract Evolution Guard`는 `CFMissileGuidePresetData` 한 타입으로 P0 구조를 증명했다.

이 상태에서 멈추면 CarFight 전체 DataAsset 관리 체계가 아니라 **MissileGuidePreset 전용 authoring/guard 시스템**으로 남는다.

`CF-FQ-051`의 목적은 두 번째 실제 DataAsset 타입인 `CFAmmoData`를 정식 Typed Staging + DACE 대상으로 onboarding하면서 다음 두 가지를 동시에 증명하는 것이다.

```text
1. AmmoData를 Editor-off JSON Staging → Preview → Review → explicit Apply → durable readback 경로로 관리할 수 있다.
2. Missile Pilot에서 만든 코드를 타입마다 복제하지 않고 재사용 가능한 multi-type 구조로 전환할 수 있다.
```

이 Feature의 성공 기준은 단순히 "Ammo JSON을 읽는다"가 아니다.

```text
Missile exact behavior 보존
+ Ammo second typed adapter 완성
+ 공용 orchestration과 타입별 contract의 경계 확정
+ DACE accepted history를 타입별 독립 authority로 확장
+ 세 번째 DA를 붙이기 전에 반복 공수/boilerplate를 측정
```

---

## 2. 왜 AmmoData를 두 번째 Pilot으로 선택하는가

### 2.1 Source 규모가 작고 실제 Runtime에 연결돼 있다

현재 `UCFAmmoData`는 `UPrimaryDataAsset`이며 authored property는 exact8이다.

```text
AmmoId                     FName
AmmoDisplayName            FText
AmmoFamilyId               FName
UnitMassKg                  float
AmmoTags                    TArray<FName>
AmmoIcon                    TSoftObjectPtr<UTexture2D>
MaximumLoadableAmmoCount    int32
bCanBeResupplied            bool
```

Runtime/피팅에서 이미 사용 중이며 다음 기존 계약도 존재한다.

```text
GetEffectiveUnitMassKg
GetEffectiveMaximumLoadableAmmoCount
IsAmmoDataValid
BuildAmmoSummary
```

따라서 단순 실험용 새 타입을 만드는 것이 아니라 **이미 CarFight에 존재하는 실제 DA 타입을 authoring/guard 체계에 편입**하는 작업이다.

### 2.2 Data Asset Manager에 이미 identity/validation 계약이 있다

`CF-FQ-045 Data Asset Management`의 Current Registry/Adapter는 AmmoData를 이미 다음 의미로 알고 있다.

```text
ClassPath:
/Script/CarFight_Re.CFAmmoData

Domain:
Combat

Stable Identity:
Required / ExplicitFName / AmmoId

Duplicate Namespace:
ExactClassPath

Validation:
CustomContract / IsAmmoDataValid
```

따라서 CF-FQ-051은 stable identity를 새로 발명하지 않는다.

Authoring/Staging/DACE의 stable identity는 기존 Data Asset Manager의 `AmmoId` 의미와 일치해야 한다.

### 2.3 Missile에 없던 두 종류의 field semantics를 검증할 수 있다

MissileGuidePreset Pilot은 FName/FText/bool/float/enum/nested struct를 충분히 다뤘지만 AmmoData는 다음 두 가지를 새로 포함한다.

```text
TArray<FName> AmmoTags
TSoftObjectPtr<UTexture2D> AmmoIcon
```

따라서 Ammo는 규모는 작으면서도 multi-type framework가 다음을 실제로 지원할 수 있는지 검증한다.

```text
container element contract
array ordering / duplicate semantics
soft object reference canonical representation
null soft reference
soft reference fingerprint semantics
```

### 2.4 Damage/Projectile/Weapon보다 두 번째 Pilot에 적합하다

`CFDamageData`는 크기는 작지만 `bUseRadialDamage`와 radial field들의 조건부 semantic 관계가 있다.

`CFProjectileData`는 약 228 lines 규모로 nested propulsion/missile/FX config, class/object references와 다양한 movement/collision 정책이 결합돼 있다.

`CFWeaponData`는 약 313 lines 규모이며 Ammo/Reload/Heat/Charge/Launcher/TargetUse/Projectile/FX/Legacy migration이 함께 존재한다.

두 번째 Pilot에서 Projectile/Weapon을 선택하면 다음 두 문제를 분리하기 어렵다.

```text
multi-type framework 자체가 잘못돼서 어려운가?
해당 DataAsset domain이 원래 복잡해서 어려운가?
```

따라서 현재 추천 순서는 다음이다.

```text
1. MissileGuidePreset — 최초 framework Pilot / 완료
2. AmmoData — 두 번째 onboarding + 재사용성 검증 / CF-FQ-051
3. DamageData — 조건부 semantic 검증 후보
4. ProjectileData — nested/reference 대형 contract 후보
5. WeaponData — 복합 domain + legacy migration 최후순위 후보
```

3번 이후 순서는 CF-FQ-051 측정 결과에 따라 다시 결정한다.

---

## 3. 2026-09-09 UE Source Audit

### 3.1 AmmoData Source

Authority:

```text
UE/Source/CarFight_Re/Public/CFAmmoData.h
UE/Source/CarFight_Re/Private/CFAmmoData.cpp
```

확인 결과:

```text
UPrimaryDataAsset
BlueprintType
Authored UPROPERTY exact8
Stable ID = AmmoId
Current validation = AmmoId non-None
UnitMassKg runtime effective getter = finite + max(0)
MaximumLoadableAmmoCount runtime effective getter = max(0)
Runtime ammo count는 UCFVehicleAmmoComp owner
Projectile/Damage ownership은 AmmoData가 소유하지 않음
```

중요한 경계:

- `GetEffectiveUnitMassKg()`의 clamp 결과는 runtime effective value이며 Staging authored semantic은 §6.2에서 finite/nonnegative strict reject + no-clamp로 동결했다.
- `MaximumLoadableAmmoCount`도 §6.2에서 authored strict reject와 Runtime defensive clamp를 분리했다.
- `AmmoTags`는 Source 자체가 순서 의미를 선언하지 않으므로 DAO-P0-00에서 §6.2의 set-like classification semantic으로 명시 동결했다.
- `AmmoIcon`은 Soft Reference이므로 fingerprint가 asset을 load해서 object state를 포함하지 않는다. canonical path/null + Asset Registry read-only target-class validation 의미는 §6.2에서 동결했다.

### 3.2 Persisted AmmoData audit

Fresh AssetDump managed dataset에서 `/Game`의 `CFAmmoData` persisted asset은 exact2가 확인됐다.

```text
/Game/CarFight/Tests/AmmoIntegration/DA_Ammo_HeavyFinite.DA_Ammo_HeavyFinite
/Game/CarFight/Tests/AmmoIntegration/DA_Ammo_RocketFinite.DA_Ammo_RocketFinite
```

둘 다 authored field exact8 / unsupported field 0 / truncated field 0이다.

현재 read-only values:

```text
DA_Ammo_HeavyFinite
- AmmoId = HeavyShell_Finite
- AmmoDisplayName = 중포탄
- AmmoFamilyId = HeavyShell
- UnitMassKg = 2.0
- AmmoTags = []
- AmmoIcon = None
- MaximumLoadableAmmoCount = 30
- bCanBeResupplied = true

DA_Ammo_RocketFinite
- AmmoId = ProtoRocket_Finite
- AmmoDisplayName = 로켓
- AmmoFamilyId = Rocket
- UnitMassKg = 5.0
- AmmoTags = []
- AmmoIcon = None
- MaximumLoadableAmmoCount = 12
- bCanBeResupplied = true
```

현재 persisted exact2는 `/Game/CarFight/Tests/AmmoIntegration/` 아래이므로 **Product canonical target으로 자동 승격하지 않는다.**

P0-00에서 다음 분류를 먼저 확정한다.

```text
Test-owned durable fixture인가?
실제 현재 Gameplay/Product reference가 있는가?
Canonical Staging 대상으로 삼을 자산은 어떤 exact set인가?
새 Product AmmoData가 필요한가?
```

현재 두 asset은 `AmmoTags=[]`, `AmmoIcon=None`이므로 non-empty array / non-null soft reference round-trip 증거가 없다. 해당 의미는 synthetic test-owned fixture로 반드시 검증한다.

### 3.3 기존 Data Management 재사용 지점

현재 `CarFight_ReEditor/Private/DataManagement`에는 AmmoData용 다음 read-first 계약이 이미 있다.

```text
CFDACurrentTypeRegistry.cpp
- CFAmmoData descriptor
- AmmoId identity source
- IsAmmoDataValid validation source

CFDATypeAdapter.cpp
- UCFAmmoData cast
- IsAmmoDataValid execution

CFDACurrentTypeTests.cpp / CFDAHealthTests.cpp
- AmmoData registry/identity/health coverage
```

CF-FQ-051은 이 기능을 복제하지 않는다.

```text
DataAssetManagement
= discover / classify / identity / health / reference read-first

DataAssetAuthoring
= typed staging / preview / review / exact apply

DACE
= source/adapter/semantic evolution guard
```

세 영역의 owner는 분리하되 identity/class semantics는 일치시킨다.

---

## 4. 현재 Missile 전용 결합 분석

### 4.1 Public Staging DTO/API가 Missile payload에 직접 결합돼 있다

`CFDAStaging.h`의 현재 구조는 공통 envelope처럼 보이는 metadata를 갖고 있지만 payload는 직접 다음 타입이다.

```text
FCFDAStagingRecord
└─ FCFDAMissilePresetPayload Payload
```

Service API도 다음처럼 Missile exact다.

```text
GetMissilePresetSchemaId
GetMissilePresetSchemaRevision
GetMissilePresetAdapterRevision
GetMissilePresetClassPath
ParseMissilePresetJson
BuildSemanticFingerprint(FCFDAMissilePresetPayload)
ExtractMissilePresetPayload
ResolveMissilePresetCurrentState
```

따라서 Ammo를 단순 if/else로 붙이면 세 번째 타입부터 분기가 계속 증가한다.

### 4.2 Staging parser/fingerprint에는 공용화 가치가 높은 primitive가 이미 있다

`CFDAStaging.cpp`에서 다음 helper/algorithm은 타입 독립 공용 후보다.

```text
exact JSON field-set validation
required field/type diagnostics
canonical SHA-256
FName semantic canonicalization
Literal FText representation
reflected enum exact token parsing
bool/float/string canonical token encoding
finite float policy
Preview common state machine
Batch duplicate policy
BatchPlanHash binding concepts
```

반대로 다음은 Missile typed adapter 소유로 남겨야 한다.

```text
Missile schema/class/revision constants
Missile payload field parse
MissileGuideConfig exact26 semantic
Missile UObject extractor
Missile current state exact class resolution
Missile serializer/materializer
Missile DACE descriptors/probes
```

### 4.3 Operational layer도 Missile exact target에 결합돼 있다

`CFDAStagingOps.cpp`:

```text
MissilePresetStagingRoot
Missile Product Low/Normal/High target exact3
BuildGuideConfigObject
BuildProductStagingJson(FCFDAMissilePresetPayload)
LoadObject<UCFMissileGuidePresetData>
```

`CFDAStagingApply.cpp`:

```text
write allowlist = UCFMissileGuidePresetData exact1
Missile typed ApplyPayloadToAsset
Missile exact Create/Update class
Missile extractor based durable readback
```

따라서 두 번째 타입을 붙일 때 **Preview/Review/TOCTOU/transaction algorithm 자체를 Ammo용으로 복사하면 실패**다.

공용 orchestration은 공유하고, type-specific parse/extract/materialize만 dispatch되어야 한다.

### 4.4 DACE도 현재 Missile contract exact1이다

현재 `CFDAContractGuard*`는 MissileGuidePreset의:

```text
SourceShape
AdapterShape
SourceAdapterMapping
SemanticContract
production probes
canonical Product Staging exact3
accepted snapshot history
```

를 직접 소유한다.

Ammo는 Missile history에 record를 섞지 않는다.

타입별 contract identity와 accepted history가 독립되어야 한다.

```text
MissileGuidePreset history
≠ AmmoData history
```

기존 `DACE-MissileGuidePreset-S1-A2-Bootstrap`은 수정·재해석·재생성하지 않는다.

---

## 5. 목표 Architecture

### 5.1 원칙: 공용 orchestration + typed adapter/provider

목표는 generic Reflection writer가 아니다.

```text
Shared Authoring Core
├─ Staging envelope metadata
├─ schema/type dispatch
├─ exact selection/discovery
├─ Preview state machine
├─ duplicate validation
├─ BatchPlanHash
├─ Review approval binding
├─ TOCTOU revalidation
├─ write transaction sequencing
├─ rollback/uncertainty aggregation
└─ durable confirmation orchestration

Typed Adapter / Provider
├─ SchemaId / revisions / ClassPath / StagingRoot
├─ typed Payload
├─ strict payload parser
├─ deterministic serializer
├─ semantic fingerprint tokens
├─ UObject → typed payload extractor
├─ typed payload → exact UObject materializer
├─ stable identity mapping
├─ current-state exact type resolution
├─ DACE descriptors
├─ production behavior probes
└─ type-owned accepted snapshot history
```

### 5.2 명시적 금지

다음 방향은 CF-FQ-051 P0 범위에서 금지한다.

```text
Generic arbitrary UPROPERTY Reflection writer
"모든 DataAsset 자동 JSON화" 기능
Property 이름만 보고 임의 materialize
DataAsset Manager에 SavePackage 권한 통합
Ammo onboarding을 이유로 VehicleData Builder/Recipe 경로 흡수
Missile Product Low/Normal/High 자동 Apply/Save
Ammo persisted asset 자동 Apply/Save
기존 accepted snapshot history rewrite
Weapon/Projectile/Damage 동시 onboarding
```

### 5.3 Normative transport — common envelope + per-type typed record + Missile compatibility facade

DAO-P0-00 Correction에서 transport는 **C 계열**로 동결한다. shared core가 arbitrary payload를 소유하지 않고, 공통 evidence와 orchestration에 필요한 최소 envelope/view만 소유한다.

```text
Shared internal common envelope / candidate view
- SchemaId
- SchemaRevision
- AdapterContractRevision
- DataAssetTypeClassPath
- StableLogicalId
- TargetObjectPath
- StagingRelativePath
- Base/Current/Staging semantic fingerprint
- PlannedOperation

Missile typed record
- existing FCFDAStagingRecord / FCFDAMissilePresetPayload 의미 유지

Ammo typed record
- 별도 Ammo typed payload/record
```

강제 규칙:

```text
- 기존 Public FCFDAStagingRecord와 FCFDAStagingService Missile API는 CF-FQ-051 동안 compatibility facade로 유지한다.
- P0-01에서 existing Missile caller를 broad rename/delete/signature break하지 않는다.
- shared orchestration에 payload bytes/void pointer/임의 reflected property map을 보관하지 않는다.
- reviewed approval evidence에는 typed payload를 보존하지 않는다.
- Apply 시 reviewed StagingRelativePath를 fresh re-read한 뒤 exact TypeKey provider를 resolve하고 해당 provider가 typed re-parse/current resolve/materialize한다.
- common core의 first implementation은 Editor Private/internal contract를 우선한다. 새 Public API는 실제 외부 caller 필요성이 증명된 경우만 추가한다.
- third type 확장은 shared state machine 복제보다 provider registration + typed adapter 추가가 중심이어야 한다.
```

즉, generic variant 하나에 모든 DA payload를 넣는 구조나 `FCFDAStagingRecord.Payload`를 즉시 type-erased로 교체하는 구조는 P0에서 채택하지 않는다.

---

## 6. Ammo typed contract에서 P0-00에 반드시 동결할 항목

### 6.1 Schema identity / trusted TypeKey / provider authority

Ammo initial contract identity를 다음으로 동결한다.

```text
SchemaId: CarFight.DataAsset.AmmoData
SchemaRevision: 1
AdapterContractRevision: 1
DataAssetTypeClassPath: /Script/CarFight_Re.CFAmmoData
StableLogicalId source: AmmoId
Canonical StagingRoot: Authoring/DataAssetStaging/AmmoData/
```

Provider dispatch의 trusted identity는 다음 exact pair다.

```text
TypeKey = SchemaId + DataAssetTypeClassPath
```

각 registered provider는 다음 authority를 고정 소유한다.

```text
SchemaId
SchemaRevision
AdapterContractRevision
DataAssetTypeClassPath
Canonical StagingRoot
Stable identity source/policy
strict parse/serialize
semantic fingerprint
UObject extract/materialize
current-state resolver
DACE descriptor/probe set
DACE accepted-history namespace
```

Fail-closed 규칙:

```text
- JSON/DTO의 payload 내용이나 caller가 provider implementation을 임의 선택하지 않는다.
- 공용 dispatch는 exact registered TypeKey를 먼저 resolve한다.
- record의 SchemaId/ClassPath가 등록 TypeKey와 하나라도 다르면 payload mutation path에 들어가기 전에 reject한다.
- SchemaRevision/AdapterContractRevision은 resolved provider current revision과 exact 대조한다.
- StagingRelativePath는 resolved provider Canonical StagingRoot 아래 canonical `.json`이어야 한다.
- unknown TypeKey, root mismatch, revision mismatch는 fail-closed다.
- DACE accepted history는 TypeKey별 독립 chain이다.
- existing Missile `DACE-MissileGuidePreset-S1-A2-Bootstrap`과 그 history namespace는 그대로 유지한다.
```

### 6.2 Ammo exact8 authored semantics — normative

```text
AmmoId
- required non-None
- FName semantic comparison = case-insensitive
- StableLogicalId == Payload.AmmoId by FName semantic

AmmoDisplayName
- Literal FText only
- empty source string 허용
- source-backed Literal의 package-only namespace/stable key는 Missile current policy와 동일하게 persistence metadata로 semantic에서 제외
- StringTable 또는 authored namespace 의미는 P0에서 reject

AmmoFamilyId
- NAME_None 허용
- non-None이면 FName case-insensitive semantic
- 현재 P0에서는 classification metadata이며 gameplay compatibility requirement로 승격하지 않음

UnitMassKg
- authored value = finite float32, >= 0
- parse/fingerprint/materialize에서 clamp 금지
- non-finite/negative는 InvalidValue로 reject
- runtime GetEffectiveUnitMassKg() clamp는 defensive legacy/runtime boundary로 그대로 유지

AmmoTags
- physical JSON = Array<String>
- semantic = set-like classification tags
- 각 element = non-None FName / case-insensitive semantic
- semantic duplicate tag는 reject
- JSON input order는 semantic fingerprint에 영향 0
- fingerprint는 FName canonical text를 만든 뒤 deterministic case-sensitive byte ordering으로 sort하여 포함
- UObject materialize 시에도 같은 canonical deterministic order로 기록

AmmoIcon
- physical JSON = null 또는 canonical Unreal SoftObjectPath string
- null은 valid optional state
- fingerprint에는 canonical path string만 포함하며 referenced asset object state/load state는 포함하지 않음
- strict parser는 path syntax/canonical form까지만 검증하고 asset load를 수행하지 않음
- Preview/provider validation은 non-null path를 Asset Registry read-only로 resolve하여 존재 + UTexture2D compatible class인지 확인
- missing asset 또는 wrong target class는 `InvalidValue` + exact `Payload.AmmoIcon` FieldPath의 blocking issue
- validation을 위해 referenced asset을 LoadObject하지 않음

MaximumLoadableAmmoCount
- authored value = exact finite int32 JSON number, >= 0
- invalid/negative는 reject하고 clamp 저장 금지
- runtime GetEffectiveMaximumLoadableAmmoCount() clamp는 defensive legacy/runtime boundary로 그대로 유지

bCanBeResupplied
- exact bool
```

`UCFAmmoData::IsAmmoDataValid()`는 현재 `AmmoId != NAME_None`만 검사하는 Runtime/Data Asset Manager health 계약으로 유지한다. **Authoring whole-record validity와 동일한 함수로 재정의하지 않는다.**

### 6.3 Product/Test target policy — current exact classification

Fresh persisted AssetDump + dependency evidence 기준으로 현재 target classification을 다음처럼 동결한다.

```text
Test-owned durable fixture exact2
- /Game/CarFight/Tests/AmmoIntegration/DA_Ammo_HeavyFinite.DA_Ammo_HeavyFinite
- /Game/CarFight/Tests/AmmoIntegration/DA_Ammo_RocketFinite.DA_Ammo_RocketFinite
- current referencer count: 0 / 0

Canonical Staging-managed Product target exact0
- 현재 없음

Known unmanaged Product AmmoData exact0
- current audit에서 확인된 것 없음
```

규칙:

```text
- persisted exact2를 Product canonical target으로 자동 승격하지 않는다.
- DAO-P0-03 durable Create/Update는 test-owned fixture authority에서 수행한다.
- DAO-P0-05 multi-type integration은 Product target exact0 상태에서도 PASS 가능해야 한다.
- 실제 Product AmmoData가 필요해지면 exact target/path/ownership을 별도 review하여 canonical set으로 명시 승격한 뒤에만 Product Apply를 허용한다.
- `SyncProduct`라는 Missile 운영 명칭을 target 분류 없이 Ammo에 복제하지 않는다.
```

---

## 7. 단계 계획

## DAO-P0-00 — Architecture / Contract Freeze + Initial Design Audit

목적:

- current Missile implementation을 exact baseline으로 고정한다.
- Ammo exact8 authored contract와 persisted exact2를 분류한다.
- shared core와 typed adapter/provider 경계를 결정한다.
- multi-type payload transport를 결정한다.
- Ammo array/soft-reference/authored numeric semantics를 동결한다.

필수 산출:

```text
current shared-vs-Missile ownership matrix
Ammo exact field contract
Ammo schema/revision candidate
Ammo target classification
payload transport decision
adapter/provider responsibility table
DACE per-type history authority
regression matrix
```

Acceptance:

```text
P0/P1 design blocker 0
common envelope + per-type typed record + Missile compatibility facade 동결
trusted TypeKey/provider/root/history authority 동결
class-scoped StableLogicalId duplicate namespace 동결
Ammo exact8 authored semantic 동결
AmmoTags/AmmoIcon DACE extension contract 동결
Ammo target classification exact2 Test / exact0 Product 동결
Missile accepted bootstrap mutation 0
Missile Product exact3 mutation 0
Ammo persisted .uasset mutation 0
Implementation 0
```

Exact next after PASS:

`DAO-P0-01 Shared Type Dispatch Foundation + Missile Parity Rewire`

---

## DAO-P0-01 — Shared Type Dispatch Foundation + Missile Parity Rewire

목적:

- 기존 Missile behavior를 바꾸지 않고 shared orchestration과 typed provider 경계를 코드에 만든다.
- Ammo를 넣기 전에 Missile을 새 dispatch 경로의 첫 provider로 통과시킨다.

반드시 공용으로 남길 후보:

```text
trusted TypeKey provider dispatch
selection normalization
provider-owned StagingRoot containment
Preview classification
batch duplicate policy
BatchPlanHash envelope binding
Review approval lifecycle
TOCTOU global/per-target preflight
write sequencing / result aggregation
rollback/uncertainty policy
durable readback orchestration
migration gate common state machine
```

Stable identity duplicate namespace는 다음으로 동결한다.

```text
StableIdentityKey = DataAssetTypeClassPath + canonical FName semantic value

same class + same FName semantic = duplicate
other class + same textual ID = allowed
TargetObjectPath duplicate = class와 무관하게 global fail-closed
current-state identity scan = resolved provider exact class namespace 내부에서만 수행
BatchPlanHash = SchemaId + DataAssetTypeClassPath + StableLogicalId를 계속 exact binding
```

반드시 Missile provider에 남길 후보:

```text
Missile payload parse/serialize
Missile FText/GuideConfig semantic
Missile extract/materialize
Missile exact current-state scan
Missile DACE descriptors/probes/history
Missile Product target exact3
```

Acceptance:

```text
기존 Missile canonical JSON bytes semantic 변화 0
기존 Product Low/Normal/High Apply·Save 0
기존 accepted snapshot exact1 그대로
기존 CF-FQ-049/DACE focused regression PASS
공용 algorithm을 Missile provider로 복제 0
```

---

## DAO-P0-02 — Ammo Typed Schema / Parse / Fingerprint / Preview

목적:

- AmmoData read-only typed authoring adapter를 추가한다.
- 아직 UObject/package mutation은 수행하지 않는다.

구현 범위:

```text
FCFDAAmmoPayload 또는 P0-00 결정 typed payload
strict whole-record Ammo JSON
schema/revision/class identity
AmmoId stable identity
FText Literal
FName fields
finite/raw numeric policy
TArray<FName> policy
TSoftObjectPtr<UTexture2D> canonical path/null policy
semantic fingerprint
UObject → Ammo payload extractor
Ammo exact current-state resolver
Preview / BatchPlanHash integration
```

필수 negative tests:

```text
missing/unknown field
wrong type
AmmoId mismatch
non-finite numeric
negative numeric — P0-00 policy 기준
AmmoTags duplicate/order case — P0-00 policy 기준
AmmoIcon malformed path
AmmoIcon wrong target class — 검증 policy 기준
payload↔cached fingerprint split
wrong DataAssetTypeClassPath
wrong Schema/Adapter revision
```

Acceptance:

```text
mutation 0
Ammo persisted exact2 Save 0
Missile regression PASS
Ammo read-only Preview exact candidate PASS
```

---

## DAO-P0-03 — Ammo Reviewed Apply / Durable Typed Writer

목적:

- AmmoData를 existing one-shot Review/TOCTOU/transaction flow에서 typed Create/Update할 수 있게 한다.
- generic Reflection writer를 만들지 않는다.

구현 범위:

```text
Ammo typed materializer
exact CreatePackage/NewObject<UCFAmmoData>
existing exact Ammo target update
pre-save typed semantic readback
exact SavePackage
disk reload
post-save typed semantic readback
rollback / SaveStateUnconfirmed / PartialApplied
```

검증은 **test-owned fixture를 우선**한다.

현재 `/Game/CarFight/Tests/AmmoIntegration` exact2를 임의로 Product apply 대상으로 간주하지 않는다. 기존 asset을 직접 쓰는 검증이 필요하면 P0-00에서 ownership을 명시하고 raw/durable restoration contract를 먼저 확정한다.

Acceptance:

```text
Create durable PASS
Update durable PASS
array non-empty round-trip PASS
soft reference non-null round-trip PASS
stale approval reject PASS
pre-existing dirty target block PASS
failure rollback/uncertainty PASS
unrelated package Save 0
Missile Product exact3 Apply·Save 0
```

---

## DAO-P0-04 — Ammo DACE Contract Provider / Revision / Migration Guard

목적:

- AmmoData를 두 번째 DACE contract identity로 등록한다.
- Missile history와 분리된 Ammo accepted baseline을 만든다.

필수 contract 축:

```text
Ammo SourceShape
Ammo AdapterShape
Ammo SourceAdapterMapping
Ammo SemanticContract
Ammo serializer probe
Ammo parser probe
Ammo fingerprint probe
Ammo extractor↔materializer probe
Ammo revision guard
Ammo migration declaration/resolution/evidence gate
Ammo canonical Staging compatibility gate
```

Ammo가 새로 요구하는 DACE 표현은 다음까지 normative다.

```text
Array element contract
- Adapter descriptor가 `Payload.AmmoTags[]`의 element JSON kind=String / representation=FNameToken을 명시
- non-empty serializer→parser production probe로 실제 element wiring을 증명
- wrong element type / semantic duplicate negative fixture
- order permutation은 same semantic fingerprint가 됨을 regression으로 고정

Soft Object target-class contract
- Source descriptor가 scalar Object/SoftObject property의 stable property kind + target class path를 signature에 포함
- `TSoftObjectPtr<UTexture2D>`의 target class `UTexture2D`가 contract 일부
- target class drift negative fixture 필요
- Adapter descriptor의 AmmoIcon physical representation = StringOrNull
- null/non-null serializer/parser/fingerprint round-trip probe 모두 필요
```

이 DACE 확장은 generic arbitrary reflection writer를 허용하지 않는다. 기존 Missile contract signature가 실제 authored shape/semantic 변화 없이 바뀌지 않도록 affected regression을 둔다.

원칙:

```text
Missile accepted history 수정 0
Ammo accepted history 별도 chain
한 타입의 revision change가 다른 타입 revision을 강제하지 않음
shared-core refactor가 실제 type contract를 바꾸지 않으면 accepted semantic delta 0
```

Ammo bootstrap append는 current Source/Adapter/Semantic contract와 regression이 모두 PASS하고 migration impact가 없는 initial baseline임을 재검수한 뒤에만 수행한다.

---

## DAO-P0-05 — Canonical Ammo Staging Pilot / Multi-Type Integration

목적:

- Product canonical Ammo exact0을 그대로 유지하면서 MissileGuidePreset + AmmoData가 같은 authoring framework에서 독립 TypeKey로 공존하는지를 증명한다.
- 실제 production-registered MissileGuidePreset + AmmoData provider가 provider-neutral operational discovery/selection → shared Preview/Review/TOCTOU/Apply core로 안전하게 연결되는지를 검증한다.
- Product mutation0 acceptance와 mutation candidate가 필요한 Reviewed approval을 서로 다른 acceptance lane으로 분리한다.

### P0-05 고정 acceptance lane

#### A. Canonical Product Set / mutation0 coexistence lane

이 lane은 Product 상태의 **비변경 보존**을 증명한다. Reviewed approval을 만들기 위한 lane이 아니다.

```text
Explicit TypeKey scope
  → provider-owned DACE canonical Staging target set 조회
  → MissileGuidePreset: existing Product canonical exact3를 read-only projection
  → AmmoData: explicit declared exact0를 zero-row 정상 결과로 projection
  → cross-TypeKey coexistence/contract isolation 판정
```

고정 계약:

```text
- Ammo Product canonical target exact0 유지
- fake Product Ammo asset 0
- fake Product Ammo Staging JSON 0
- Product Ammo Review/Apply/Save 0
- Product Missile Low/Normal/High Apply/Save 0
- DACE accepted append 0
- Ammo exact0은 no-op Reviewed approval이 아니라 정상적인 zero-row provider contribution이다.
- selected provider의 canonical target set이 explicit exact0이면 physical canonical directory가 absent/empty여도 이 lane에서는 정상이다.
- selected provider가 canonical target을 non-empty로 선언했는데 그 exact declared file을 읽을 수 없으면 fail-closed한다. 이를 zero-row로 축소하지 않는다.
```

즉 Product acceptance에서 `BuildCommonReviewedApproval()`을 empty/NoChange set에 허용하도록 완화하지 않으며, acceptance를 만들기 위해 Product target을 생성하지 않는다.

#### B. Explicit Paths / disposable mixed integration lane

실제 Review/TOCTOU/provider dispatch는 mutation candidate가 필요한 별도 lane에서 검증한다.

```text
Explicit allowed TypeKey scope
+ caller-supplied exact Staging relative paths
  → path owner를 trusted production provider registry로 선결정
  → owner provider typed ParseCommonCandidate
  → owner provider ResolveCommonCurrentState
  → shared BuildCommonPreview
  → 전체 row에 global duplicate validation
  → common BatchPlanHash / Reviewed approval
  → global preflight
  → 각 target 직전 immediate TOCTOU
  → exact provider ApplyReviewedMutation
```

fixture 계약:

```text
Missile Staging:
Authoring/DataAssetStaging/MissileGuidePreset/__AutomationP05__/

Ammo Staging:
Authoring/DataAssetStaging/AmmoData/__AutomationP05__/

Disposable target root:
/Game/Test/CarFight/DAOP05/
```

- 실제 production registry의 MissileGuidePreset + AmmoData provider를 사용한다. synthetic second provider만으로 P0-05 acceptance를 대체하지 않는다.
- 두 provider 모두 최소 한 개의 Create/Update mutation candidate를 포함해 actual mixed Review evidence를 만든다.
- positive case는 한 번의 mixed `Review()`로 승인된 Missile 1개 + Ammo 1개 disposable Create를 같은 `ApplyReviewed()`에 전달하고, exact provider dispatch 결과로 두 target 모두 durable `Applied`가 되는 것을 확인한 뒤 teardown한다. 이 durable mutation은 `/Game/Test/CarFight/DAOP05/`에만 한정되며 Product mutation과 분리한다.
- stale source/current negative case는 mutation 전에 fail-closed하고 disposable target에 durable residue를 남기지 않아야 한다.
- HeavyFinite/RocketFinite와 Product Low/Normal/High는 fixture로 사용하지 않는다.
- test-owned Staging과 persisted target은 teardown 뒤 residue 0이어야 한다.
- 같은 textual StableLogicalId를 두 actual class에서 사용한 positive case는 false duplicate가 없어야 한다.
- 서로 다른 TypeKey가 같은 TargetObjectPath를 요구하는 negative case는 global duplicate로 fail-closed해야 한다.
- input path 순서가 뒤집혀도 mixed BatchPlanHash/Reviewed target set은 deterministic해야 한다.
- stale source와 stale current truth를 actual 두 provider 경로에서 검증하고 exact approval binding이 깨지면 mutation 전에 차단해야 한다.

### Provider-neutral operational discovery/selection 계약

P0-05는 shared Preview/duplicate/hash/Review/Apply algorithm을 다시 만들지 않는다. 새 범위는 **경로를 trusted provider에 안전하게 귀속시키고 existing common core에 전달하는 Editor Private seam**뿐이다.

경로 owner 결정 규칙:

```text
1. caller의 explicit relative path를 canonical form으로 정규화한다.
2. JSON 파일을 읽기 전에 code-owned production provider registry를 기준으로 path ownership을 판정한다.
3. 각 provider의 CanonicalStagingRoot containment를 기존 NormalizeProviderStagingPath 계약으로 검사한다.
4. exact owner count == 1일 때만 그 provider를 선택한다.
5. owner count == 0 또는 >1이면 fail-closed한다.
6. 선택된 owner가 caller의 explicit allowed TypeKey scope에 없으면 fail-closed한다.
7. owner 확정 뒤에만 JSON을 읽고 해당 provider ParseCommonCandidate를 호출한다.
8. parser 결과 TypeKey/revision/root가 owner provider와 exact 일치하는지는 existing ValidateProviderContract가 다시 확인한다.
```

구현 시 `CFDATypeDispatch`의 code-owned registry는 계속 Private authority로 유지한다. Ops가 raw provider array를 소유/열거하도록 공개하지 않고, 예를 들어 `FindProviderForStagingPath(...)`처럼 **exact path owner 하나만 반환하는 좁은 resolver seam**을 추가하는 방향을 사용한다. 향후 provider root가 중첩되는 실수가 생겨도 exact owner count >1을 검출할 수 있어야 한다.

금지:

```text
- JSON의 SchemaId/DataAssetTypeClassPath를 먼저 읽어 provider를 선택하는 payload-first dispatch
- Authoring/DataAssetStaging 부모 root 전체를 recursive scan한 뒤 payload가 자기 provider를 주장하게 하는 discovery
- mixed API의 empty selection을 '모든 provider root recursive scan'으로 해석
- Ammo exact0을 이유로 HeavyFinite/RocketFinite를 Product canonical target으로 승격
- existing Missile Product exact3 SyncProduct 의미 변경
```

Mixed operational discovery는 다음 두 모드만 허용한다.

```text
Canonical Product Set
- explicit TypeKey scope + provider가 이미 선언한 exact canonical target paths
- directory recursive discovery 없음
- exact0 provider는 zero rows 정상

Explicit Paths
- caller가 exact relative paths + allowed TypeKey scope를 모두 제공
- 각 path를 trusted root ownership으로 provider에 귀속
- missing explicit file은 fail-closed
- directory recursive discovery 없음
```

기존 `DiscoverMissilePresetPreview()`의 `empty selection = Missile canonical root 전체 discovery`, Public Missile Preview/console API와 `SyncProductStaging` Product Low/Normal/High exact3 동작은 **compatibility facade로 그대로 보존**한다. 새 mixed contract가 기존 empty-selection 의미를 재정의하지 않는다.

`FCFDAStagingOpsSession` wiring은 다음처럼 고정한다.

```text
Preview(selected paths empty)
→ 기존 Missile-only DiscoverMissilePresetPreview(empty) 그대로 사용
→ Missile canonical root 전체 discovery 의미 보존

Preview(selected paths non-empty)
→ 새 provider-neutral Explicit Paths normalization/discovery 사용
→ canonical Missile path와 canonical Ammo path를 같은 session selection에 허용
→ exact path owner는 JSON read 전 trusted registry로 결정

Review()
→ 직전 Preview가 사용한 동일 selection mode + 동일 normalized exact path set으로 fresh rediscovery
→ mixed Preview에서 만든 approval과 같은 provider/path scope를 재현

ApplyReviewed()
→ existing shared payload-free Apply/TOCTOU 경로 그대로 사용
```

Console의 StableLogicalId shorthand는 기존 `BuildConsoleSelection()`이 Missile canonical relative path로 변환한 뒤 session에 전달하는 현재 의미를 유지한다. 따라서 console shorthand를 Ammo로 암묵 확장하지 않으며 기존 Missile 사용법도 깨뜨리지 않는다. Mixed Ammo 선택은 P0-05에서 canonical full relative path를 explicit하게 제공한다.

Path-owner resolver의 ambiguous-root fail-closed는 production registry를 오염시키지 않는 test-owned explicit provider-set seam으로도 검증 가능해야 한다. existing `FindExactProviderEntryInSetForTests` 패턴처럼 production registry mutation 없이 overlapping canonical roots를 구성해 `owner count > 1` reject를 증명하는 방향을 사용한다.

### Mixed Apply transaction 의미

P0-05의 multi-type integration은 cross-target all-or-nothing atomic transaction을 의미하지 않는다.

```text
보장:
- exact mixed Reviewed evidence binding
- approval 전체 global preflight
- exact TypeKey provider dispatch
- 각 target mutation 직전 source/current immediate TOCTOU
- target별 typed durable execution
- current aggregate failure/uncertainty taxonomy

보장하지 않음:
- 이미 durable 적용된 앞 target을 뒤 target 실패 때문에 전체 rollback하는 global transaction
```

현재 durable execution은 deterministic target 순서의 sequential apply다. 하나 이상 durable 성공한 뒤 다른 target이 known failure이면 `PartialApplied`가 유효한 terminal taxonomy다. `SaveStateUnconfirmed` / `InMemoryStateUnconfirmed`의 기존 우선순위도 유지한다. P0-05는 이 계약을 변경하지 않는다.

### DACE / isolation acceptance

```text
- Missile DACE accepted history/revision unchanged
- Ammo DACE accepted history/revision unchanged
- DACE accepted append 0
- 한 TypeKey의 Review/Apply가 다른 TypeKey history/revision/declaration을 변경하지 않음
- provider dispatch는 exact SchemaId + DataAssetTypeClassPath
- unknown TypeKey / parser-declared mismatch / owner-scope mismatch fail-closed
```

### P0-05 구현 후 필수 fresh validation

Source implementation이 실제로 발생한 뒤에만 다음을 fresh 실행한다.

```text
Official UE 5.8 Editor Build
DAO-P0-05 focused actual-provider mixed regression
affected CF-FQ-049 exact13 regression
existing Missile DACE exact15 regression
protected exact10 audit
/Game/Test/CarFight/DAOP05 persisted residue exact0
Product canonical Ammo exact0 / fake Product Staging 0 확인
```

현재 Contract Correction 단계에서는 Source mutation이 없으므로 위 검증을 미리 재실행하지 않는다.

---

## DAO-P0-06 — Reuse Measurement / Acceptance / Current System Promotion

목적:

- 두 번째 타입 onboarding이 실제로 재사용 구조가 됐는지 판정한다.
- 다음 DamageData onboarding 전에 framework 보강이 필요한지 결정한다.
- PASS 시 `DataAssetAuthoring.md` Current 계약을 Missile+Ammo multi-type으로 승격한다.

### 재사용성 Acceptance

다음 algorithm을 Ammo 전용으로 복제했다면 FAIL/HOLD다.

```text
Preview state machine
approval lifecycle
TOCTOU sequencing
BatchPlanHash common envelope
transaction result aggregation
rollback/uncertainty state machine
durable confirmation orchestration
accepted chain generic integrity algorithm
migration state machine
```

Ammo-specific 코드로 허용되는 것은 다음이다.

```text
payload definition
field schema
parse/serialize
extract/materialize
field semantics
identity mapping
exact current state provider
DACE descriptor/probe
optional target classification/config
```

### 공수 측정

완료 시 다음을 기록한다.

```text
shared-core 변경 파일 수
Ammo-specific 신규/변경 파일 수
Missile parity를 위해 수정한 파일 수
신규 test 수
복제된 algorithm 유무
세 번째 타입에서 다시 고쳐야 할 core 항목 유무
```

하드코딩된 임의 LoC 목표보다 **세 번째 타입 추가가 core algorithm 재작성 없이 provider 추가 중심으로 가능한가**를 최종 판정 기준으로 사용한다.

결론:

```text
PASS
→ DamageData third onboarding 착수 후보

HOLD
→ Data Asset Onboarding Scaffold/Core 보강 Feature를 먼저 분리
```

Current System Promotion 후보:

```text
Document/Systems/DataManagement/DataAssetAuthoring.md
```

---

## 8. 전체 회귀 Matrix

각 구현 단계에서 영향을 받은 범위만 fresh로 실행하되 최종 P0-06에서는 최소 다음을 함께 닫는다.

```text
Official UE 5.8 Editor Build
기존 CF-FQ-049 Staging focused/affected
기존 CF-FQ-050 DACE focused
기존 Missile OperationalEntry
신규 Ammo typed parse/fingerprint/preview
신규 Ammo durable apply fixture
신규 Ammo DACE
Data Asset Manager Ammo identity/health regression
필요 시 Ammo runtime/fitting focused regression
```

기존 PASS를 무조건 매 단계 전부 반복하지 않는다. 변경 영향이 있는 단계에서만 affected scope를 확대한다.

---

## 9. 보호 조건

CF-FQ-051 전체에서 다음을 기본 보호한다.

```text
현재 단일 Active CF-FQ-039 유지
기존 병렬 dirty clean/revert/stash/reset 0
CFVehicleVisualComp.cpp unrelated dirty 보호
SourceArt/UI/HUD/VehiclePanel/ 보호
Missile Product Low/Normal/High silent mutation 0
Missile accepted bootstrap rewrite 0
Ammo persisted exact2 silent mutation 0
unrelated DataAsset Save 0
VehicleData Builder/Recipe/Profile ownership 변경 0
Data Asset Manager read-first owner에 write authority 추가 0
```

사용자가 별도로 commit/push를 요청하지 않으면 수행하지 않는다.

---

## 10. 이번 Source Audit의 핵심 설계 결론

### 결론 A — Ammo는 두 번째 Pilot로 적합

이유:

```text
작은 authored surface exact8
실제 Runtime/피팅 사용 타입
기존 Data Asset Manager identity/validation 존재
Missile에 없던 Array + Soft Reference 포함
Weapon/Projectile보다 domain complexity가 낮음
```

### 결론 B — 현재 Authoring Core는 아직 multi-type이 아니다

`FCFDAStagingRecord.Payload`, parser/service, Ops serializer/target, Apply writer, DACE가 Missile에 직접 결합돼 있다.

따라서 Ammo를 붙이기 전에 최소한의 dispatch/provider 경계를 만들어야 한다.

### 결론 C — 하지만 대형 generic framework를 먼저 만들면 안 된다

두 번째 타입만으로 필요한 exact abstraction만 만든다.

```text
Missile + Ammo exact2에서 공통인 algorithm만 shared core
Ammo 고유 field semantics는 typed provider
세 번째 타입을 상상해서 임의 Reflection writer 구축 금지
```

### 결론 D — AmmoTags/AmmoIcon이 실제 재사용성 시험점이다

현재 persisted exact2는 둘 다:

```text
AmmoTags = []
AmmoIcon = None
```

이므로 positive non-empty/non-null round-trip은 synthetic fixture가 필요하다.

이 두 field를 제대로 처리하지 않고 "Ammo onboarding PASS"로 종료하지 않는다.

---

## 10.1 2026-09-09 DAO-P0-00 Initial Design Review

### 판정

```text
P0: 0
P1: 6
P2: 2
Implementation: HOLD
DAO-P0-01: HOLD
Exact next: DAO-P0-00 Correction + Re-review
```

이번 검수는 Source/Asset mutation 없이 current Missile Staging/Apply/Ops/DACE, `CFAmmoData`, Data Asset Manager registry/adapter와 fresh persisted AssetDump dependency evidence를 교차감사했다.

Fresh persisted evidence에서 현재 AmmoData exact2는 모두 referencer 0이며 `/Game/CarFight/Tests/AmmoIntegration/` 아래에 있다. 따라서 현재 근거상 두 asset은 Product canonical target이 아니라 test-owned durable fixture로 취급하는 것이 안전하다.

### P1-1 — Multi-type payload transport와 기존 Missile Public compatibility 경계가 동결되지 않음

현재 Public `FCFDAStagingRecord`는 `FCFDAMissilePresetPayload Payload`를 직접 소유하고 `FCFDAStagingService`, `FCFDAStagingOps`, `FCFDAStagingApplyService`도 Missile exact signature에 결합돼 있다.

Plan v0.1.0은 A/B/C transport 후보를 남겼지만 P0-01 시작 전에 어떤 방법으로 기존 Public contract를 보존할지 정하지 않았다. 이 상태에서 `FCFDAStagingRecord`를 generic/variant로 직접 교체하면 CF-FQ-049/050의 compile/runtime contract를 불필요하게 깨뜨릴 수 있다.

교정 요구:

```text
Transport 선택 = C 계열
- minimal common envelope/common candidate view
- Missile typed record와 Ammo typed record는 별도 유지
- approval evidence에는 payload를 보존하지 않음
- Apply 시 exact source를 fresh re-read/re-parse하고 해당 typed provider가 materialize
- 기존 FCFDAStagingRecord/FCFDAStagingService Missile API는 CF-FQ-051 동안 compatibility facade로 유지
- shared core는 Private/internal contract로 먼저 도입
- broad rename/delete/public signature break 금지
```

### P1-2 — Trusted Type Provider dispatch / StagingRoot / DACE history authority가 하나의 fail-closed key로 동결되지 않음

현재 candidate integrity는 Missile SchemaId/Revision/ClassPath를 hard-code하고 Ops는 Missile root를 hard-code한다. Apply의 raw file read 자체는 main_game containment만 확인하므로 multi-type 전환 시 caller/record가 임의 root/provider를 선택하게 두면 안 된다.

교정 요구:

```text
Trusted provider identity = exact registered TypeKey
TypeKey = SchemaId + DataAssetTypeClassPath

각 provider registration이 고정 소유:
- SchemaId
- SchemaRevision
- AdapterContractRevision
- DataAssetTypeClassPath
- canonical StagingRoot
- stable identity source/policy
- parse/fingerprint/extract/materialize/current-state callbacks
- DACE accepted-history namespace

규칙:
- JSON/DTO가 provider를 임의 선택하지 않음
- registry에서 exact TypeKey를 먼저 resolve하고 record의 revisions/root를 그 authority와 대조
- SchemaId/ClassPath 중 하나라도 mismatch면 payload parse/apply 전 fail-closed
- StagingRelativePath는 해당 provider의 fixed root 아래여야 함
- DACE accepted history는 exact TypeKey별 독립 chain
- 기존 Missile bootstrap chain은 그대로 유지
```

### P1-3 — StableLogicalId batch duplicate namespace가 Data Asset Manager 계약과 불일치

현재 `ApplyBatchDuplicateValidation()`은 bare canonical `FName`만 key로 사용한다. 반면 Data Asset Manager의 duplicate namespace는 exact class path다.

따라서 Missile과 Ammo가 우연히 같은 textual ID를 쓰면 multi-type batch에서 false duplicate가 발생할 수 있다.

교정 요구:

```text
StableIdentityKey = DataAssetTypeClassPath + FName semantic canonical value

- same class + same FName semantic = duplicate
- different class + same text = allowed
- TargetObjectPath duplicate는 class와 무관하게 계속 global fail-closed
- current-state identity search도 provider/exact class namespace 안에서 수행
- BatchPlanHash는 기존처럼 SchemaId/ClassPath/StableLogicalId를 모두 binding
```

### P1-4 — Ammo authored numeric policy와 Runtime defensive clamp의 의미가 분리되지 않음

현재 Source truth:

```text
UnitMassKg UPROPERTY ClampMin=0
MaximumLoadableAmmoCount ClampMin=0
IsAmmoDataValid() = AmmoId non-None만 검사
GetEffectiveUnitMassKg() = non-finite/negative를 runtime에서 0으로 방어
GetEffectiveMaximumLoadableAmmoCount() = negative를 runtime에서 0으로 방어
existing contract test는 negative raw 값도 AmmoId가 있으면 IsAmmoDataValid()==true임을 허용
```

따라서 Staging에서 음수를 막는다면 `IsAmmoDataValid` 재사용이라고 표현하면 안 된다.

교정 요구:

```text
Ammo Authoring semantic:
- UnitMassKg = finite float32, >= 0, parse/fingerprint/materialize에서 clamp 금지, invalid는 reject
- MaximumLoadableAmmoCount = exact int32, >= 0, invalid는 reject

Runtime semantic:
- 기존 effective getter clamp는 defensive legacy/runtime boundary로 그대로 유지

Data Asset Manager health:
- 기존 IsAmmoDataValid() = AmmoId only 계약 유지
- Authoring whole-record validity와 동일하다고 주장하지 않음
```

### P1-5 — AmmoTags semantic과 DACE Adapter Array element contract가 부족함

현재 Source에서 `AmmoTags`는 P0 검색·표시 metadata이고 다른 Runtime/Editor consumer는 확인되지 않았다. 그러나 Plan은 ordered list인지 set-like인지 정하지 않았다.

또 현재 DACE Adapter descriptor/JSON observer는 `Array`라는 container kind는 볼 수 있지만 array element의 physical/semantic representation을 별도 descriptor로 증명하지 못한다. empty array만으로는 serializer/parser가 `FName` string element를 올바르게 wiring했는지도 증명할 수 없다.

교정 요구:

```text
AmmoTags semantic:
- physical JSON = Array<String>
- semantic = set-like classification tags
- element identity = FName case-insensitive semantic
- duplicate semantic tag reject
- JSON physical order는 semantic fingerprint에 영향 0
- fingerprint는 canonical FName text를 deterministic case-sensitive byte order로 sort 후 포함
- materialized UObject array도 canonical deterministic order로 기록

DACE:
- Adapter descriptor가 Payload.AmmoTags[] element kind/representation을 표현
- non-empty synthetic serializer/parser probe로 element wiring을 증명
- wrong element type / duplicate / order permutation negative/semantic-equivalence regression 포함
```

### P1-6 — AmmoIcon Soft Reference semantic과 DACE Source target-class drift coverage가 부족함

현재 `AmmoIcon`은 `TSoftObjectPtr<UTexture2D>`지만 DACE Source descriptor의 scalar object/reflected type path 계약은 top-level Soft Object target class를 명시적으로 보존하도록 설계돼 있지 않다. 따라서 미래에 `Texture2D` target type이 바뀌는 drift를 SourceShape가 충분히 구분하지 못할 위험이 있다.

교정 요구:

```text
AmmoIcon JSON:
- null 또는 canonical Unreal SoftObjectPath string
- fingerprint = canonical path string만 포함, referenced asset load/object state 포함 금지
- parser = syntactic path validation only
- Preview/provider validation = non-null path가 Asset Registry에서 resolve되고 Texture2D-compatible인지 read-only 확인
- missing/wrong-class = blocking InvalidValue with exact FieldPath
- null = valid optional state

DACE Source:
- scalar Object/SoftObject property에 stable property kind + target class path를 descriptor signature에 포함
- TSoftObjectPtr<UTexture2D> → 다른 target class 변경 negative regression

DACE Adapter:
- AmmoIcon physical representation = StringOrNull
- null/non-null serializer/parser/fingerprint round-trip probe 모두 필요
```

### P2-1 — DisplayName / FamilyId optional semantic을 explicit하게 고정할 필요

현재 `AmmoDisplayName`과 `AmmoFamilyId`는 `IsAmmoDataValid()` blocker가 아니며 `AmmoFamilyId`는 summary 외 consumer가 없다.

교정 권고:

```text
AmmoDisplayName: empty Literal 허용 / source-backed Literal FText policy는 Missile과 동일
AmmoFamilyId: NAME_None 허용 / non-None이면 FName case-insensitive semantic
```

향후 Gameplay compatibility rule이 생기면 별도 contract revision 대상으로 취급한다.

### P2-2 — 현재 Product canonical Ammo target이 존재하는 것처럼 읽힐 여지를 제거할 필요

Fresh AssetDump에서 persisted Ammo exact2는 referencer 0이고 Test path에 있다. 지금 Product target을 발명할 근거는 없다.

교정 권고:

```text
Current Ammo target classification:
- Test-owned durable fixture exact2 = HeavyFinite / RocketFinite
- Canonical Staging-managed Product target exact set = 0
- Unmanaged Product AmmoData = current audit에서 확인된 것 없음

DAO-P0-05는 Product set이 0인 상태에서도 multi-type integration을 검증할 수 있어야 함
실제 Product AmmoData가 필요해진 시점에 별도 exact target promotion/review 후 등록
```

### 구현 전 재검수 Gate — Initial Review 당시 요구사항

v0.1.1 Initial Review에서 다음 8건을 교정 조건으로 기록했으며 v0.2.0 Correction에서 모두 normative contract로 흡수했다.

```text
[x] common envelope + per-type typed record + Missile compatibility facade
[x] exact trusted TypeKey/provider registry/root/history namespace
[x] class-scoped StableLogicalId duplicate namespace
[x] Ammo numeric Authoring semantic vs Runtime clamp 분리
[x] AmmoTags set-like semantic + DACE array-element descriptor/probe
[x] AmmoIcon SoftObjectPath semantic + DACE target-class drift coverage
[x] DisplayName/FamilyId optional semantics
[x] Ammo exact2 Test-owned / Product exact0 classification
```

Initial Review 당시에는 C++/JSON/.uasset/accepted snapshot을 수정하지 않았고 Build/Automation도 실행하지 않았다.

---

## 10.2 2026-09-09 DAO-P0-00 Correction + Re-review

### 교정 결과

Initial Review의 P1 6건/P2 2건을 다음 authoritative section에 흡수했다.

```text
P1-1 transport/Public compatibility
→ §5.3 common envelope + per-type typed record + existing Missile compatibility facade

P1-2 trusted provider/root/history authority
→ §6.1 exact TypeKey = SchemaId + DataAssetTypeClassPath + provider-owned revision/root/history

P1-3 StableLogicalId namespace
→ DAO-P0-01 contract의 class-scoped StableIdentityKey

P1-4 authored numeric vs runtime clamp
→ §6.2 UnitMassKg / MaximumLoadableAmmoCount strict Authoring reject + Runtime defensive clamp 분리

P1-5 AmmoTags + DACE Array element
→ §6.2 set-like FName semantics + DAO-P0-04 explicit `Payload.AmmoTags[]` Adapter element contract/probe

P1-6 AmmoIcon + DACE target class
→ §6.2 SoftObjectPath/Asset Registry read-only validation + DAO-P0-04 scalar SoftObject target-class Source contract

P2-1 optional DisplayName/FamilyId
→ §6.2 empty Literal / NAME_None 허용 의미 동결

P2-2 target classification
→ §6.3 Test-owned durable fixture exact2 / Product canonical exact0 / known unmanaged Product exact0
```

### Current Source 대조 재검수

교정 뒤 current Source를 다시 읽어 다음을 확인했다.

```text
- Public FCFDAStagingRecord는 여전히 Missile payload direct ownership이므로 compatibility facade 요구가 실제 current baseline과 일치한다.
- Approval target은 typed payload를 저장하지 않고 metadata/fingerprint만 보존하므로 fresh source re-read + provider typed re-parse 설계와 충돌하지 않는다.
- current NormalizeStagingRelativePath는 global Authoring/DataAssetStaging/ prefix까지만 검사하므로 provider-owned exact StagingRoot gate가 필요한 상태다.
- current ApplyBatchDuplicateValidation은 Missile single-type baseline이며 multi-type 전환 전에 class-scoped identity key로 교정해야 한다.
- CFAmmoData Runtime getter는 negative/non-finite defensive clamp를 유지하고 IsAmmoDataValid는 AmmoId-only이므로 strict Authoring validity를 별도 contract로 두는 설계가 기존 Runtime 계약을 보존한다.
- current DACE Reflection token builder는 container inner type은 보존하지만 scalar Object/SoftObject의 GetReflectedTypePath는 target class를 반환하지 않는다. AmmoIcon target-class extension이 실제 필요한 공백이다.
- current DACE JSON observer는 Array 자체만 관측하고 element physical kind를 재귀 관측하지 않는다. AmmoTags[] element descriptor/probe가 실제 필요한 공백이다.
```

### 재검수 판정

```text
P0: 0
blocking P1: 0
P2: 0
DAO-P0-00: PASS
Contract Freeze: Complete
Implementation: Not Started
DAO-P0-01: READY
Exact next: DAO-P0-01 Shared Type Dispatch Foundation + Missile Parity Rewire
```

이 PASS는 **설계/계약 PASS**다. 아직 shared provider dispatch, Ammo typed record, DACE extension 코드는 구현되지 않았다. 해당 구현과 실제 Build/Automation은 DAO-P0-01 이후 단계가 소유한다.

### 보호 확인

```text
C++ Source mutation 0
Build 0
Automation 0
Missile canonical Product Staging JSON mutation 0
Missile Product Low/Normal/High Apply·Save 0
Missile accepted bootstrap/history mutation 0
Ammo persisted exact2 .uasset mutation 0
Ammo Product asset 생성 0
CF-FQ-039 Active 유지
기존 병렬 dirty clean/revert/stash/reset 0
commit/push 0
```

### DAO-P0-01 구현 / 검증 체크포인트

```text
DAO-P0-01: Technical PASS
Shared Type Dispatch Foundation: Complete
Production registered provider: MissileGuidePreset exact1
Missile Public compatibility facade signature delta: 0
Ammo typed payload / DACE Ammo extension: 0 — DAO-P0-02 이후 범위

Official UE 5.8 Editor Build
Job: 6737c1bff29a40a4bfd9bb6be98213b1
Result: PASS / Exit 0

Affected CF-FQ-049 Staging regression
Process: 34de9e0426dc4f33ae05a76195bc6190
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

DACE regression
Process: be73e54a237c4ea8aa20b4e2a5c001e1
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0

Product Low/Normal/High Apply·Save: 0
canonical Product exact3 JSON diff: 0
Product exact3 .uasset diff: 0
accepted snapshot source diff: 0
accepted history: bootstrap exact1 / append 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

구현은 Editor Private `CFDATypeDispatch` + `CFDAMissileProvider`를 공용 dispatch/첫 typed provider 경계로 추가했다. common envelope는 payload를 소유하지 않고 exact `SchemaId + DataAssetTypeClassPath` TypeKey로 trusted code-owned provider를 선택한다. Missile provider는 revision, canonical StagingRoot, Stable Identity source와 production parse/fingerprint/extract/current-state/serialize/materialize callback을 소유하며 기존 Public `FCFDAStagingRecord`/`FCFDAStagingService` Missile API는 compatibility facade로 유지한다.

StableLogicalId duplicate scope는 `exact DataAsset class + FName semantic`으로 전환했고 TargetObjectPath duplicate는 계속 global fail-closed다. Production StagingRoot는 provider-owned exact root만 허용한다. 기존 CF-FQ-049 focused fixture가 사용하던 `__AutomationMissing__`, `__AutomationLoaded__`, `__AutomationP04__`는 `WITH_DEV_AUTOMATION_TESTS` exact allowlist로만 보존해 Production authority를 확장하지 않았다.

초기 regression에서 이 test-owned roots가 새 provider root 검사에 차단되는 회귀를 발견했고 당시에는 `WITH_DEV_AUTOMATION_TESTS` exact allowlist로 최종 fresh Build + exact13 + DACE exact15를 PASS시켰다. 이 evidence는 Missile behavior/signature parity 증거로 보존한다.

### 10.3 2026-09-09 DAO-P0-01 Post-Implementation Mid-review

#### 판정

```text
P0: 0
blocking P1: 4
P2: 2
DAO-P0-01 Technical PASS: Revoked / HOLD
DAO-P0-02: HOLD
Exact next: DAO-P0-01 Correction + Re-review
```

기존 official Build PASS, affected CF-FQ-049 exact13/13, DACE exact15/15는 **Missile parity evidence로 유효**하다. 다만 이번 Gate의 핵심 목적은 Ammo를 넣기 전에 shared orchestration + typed provider 경계를 닫아 이후 타입 추가가 core algorithm 재작성 없이 provider/typed adapter 추가 중심으로 가능하게 만드는 것이다. current Source를 그 기준으로 다시 감사한 결과 다음 blocking gap이 남아 있다.

##### P1-1 — Shared core가 여전히 Missile-only Public record/row에 직접 의존함

`CFDATypeDispatch.h`의 `BuildCommonEnvelope()`는 `FCFDAStagingRecord`를 직접 입력으로 받는다. 그러나 이 Public record는 `FCFDAMissilePresetPayload Payload`를 직접 소유한다. 또한 현재 Preview/Review/Batch hash/Apply lifecycle의 공통 row는 `FCFDAStagingPreviewRow -> FCFDAStagingRecord` 구조다.

따라서 이름상 payload-free common envelope가 생겼어도 shared orchestration의 실제 transport와 state machine은 아직 Missile typed record에 묶여 있다. Ammo를 추가하려면 같은 lifecycle을 복제하거나 이 core를 다시 리팩터링해야 한다.

교정 요구:

```text
- Editor Private payload-free common candidate / preview / approval execution view를 별도 경계로 둔다.
- 기존 Public Missile FCFDAStagingRecord/PreviewRow/API는 compatibility facade로 유지한다.
- typed record는 provider-local parse/extract 단계에서만 존재하고 shared Review/TOCTOU/Batch lifecycle에는 payload를 저장하지 않는다.
- generic void*/property map/opaque payload bag은 도입하지 않는다.
```

##### P1-2 — TypeKey registry가 complete provider authority가 아니라 descriptor lookup에 그침

현재 `GetRegisteredProviders()`는 `TArray<const FCFDATypeProvider*>`이고 `FindExactProvider()`도 metadata descriptor만 반환한다. 실제 parse/fingerprint/extract/current-state/serialize/materialize callback은 별도 `FCFDAMissileTypeProvider`에만 있으며 generic registry 결과에서는 접근할 수 없다.

또한 frozen contract가 provider-owned authority로 요구한 stable identity policy/resolver 의미와 DACE descriptor/probe/history namespace도 `FCFDATypeProvider` registration에 포함돼 있지 않다.

교정 요구:

```text
- exact TypeKey lookup 결과가 descriptor + shared-core가 필요한 provider operation entry를 함께 식별해야 한다.
- callback은 raw JSON/common envelope/common result처럼 shared-safe 입력·출력만 사용하고 typed payload는 provider 내부 stack에 유지한다.
- Stable Identity source뿐 아니라 Required/ExplicitFName 같은 policy 의미도 provider authority에 결속한다.
- DACE per-TypeKey contract/history namespace authority를 provider registration과 분리되지 않게 연결한다.
- JSON/DTO/caller가 implementation을 선택하지 않는 code-owned registry 원칙은 유지한다.
```

##### P1-3 — Fresh Apply / TOCTOU가 registry-selected provider dispatch가 아니라 Missile facade를 직접 호출함

`CFDAStagingApply.cpp::BuildFreshRows()`는 exact TypeKey descriptor를 찾은 뒤 다시 `Provider == MissileProviderDescriptor`를 강제하고, disk source re-read 이후 다음 Missile facade를 직접 호출한다.

```text
FCFDAStagingService::ParseMissilePresetJson
FCFDAStagingService::ResolveMissilePresetCurrentState
FCFDAStagingService::BuildPreview
FCFDAStagingService::ApplyBatchDuplicateValidation
FCFDAStagingService::BuildBatchPlanHash
```

mutation 직전 revalidation과 `ApplyOneTarget()`도 같은 Missile-only row를 사용한다. 따라서 frozen contract의 `Apply 시 exact TypeKey provider resolve -> provider typed re-parse/current resolve/materialize`가 아직 실제 공용 execution path로 닫히지 않았다.

교정 요구:

```text
- approval metadata에서 exact provider를 resolve한 뒤 fresh parse/current resolve를 provider operation entry로 dispatch한다.
- common Preview/duplicate/BatchPlanHash/Review/TOCTOU sequencing은 payload-free common row에서 실행한다.
- materialize는 fresh provider-local typed parse 결과를 사용하되 shared core가 typed payload를 보관하지 않는다.
- 기존 Public Missile Apply API는 이 internal shared path를 감싸는 compatibility facade로 남긴다.
```

##### P1-4 — `WITH_DEV_AUTOMATION_TESTS` sibling-root allowlist가 Development Editor의 provider-owned root 경계를 약화함

현재 `NormalizeProviderStagingPath()`는 compile-time `WITH_DEV_AUTOMATION_TESTS`에서 다음 sibling root를 Missile provider root와 별도로 허용한다.

```text
Authoring/DataAssetStaging/__AutomationMissing__/
Authoring/DataAssetStaging/__AutomationLoaded__/
Authoring/DataAssetStaging/__AutomationP04__/
```

`WITH_DEV_AUTOMATION_TESTS`는 특정 Automation test가 실행 중일 때만 true가 되는 runtime gate가 아니라 해당 Editor binary에 compile되는 dev-test surface다. 따라서 일반 Development Editor authoring에서도 이 sibling root가 validator를 통과할 수 있어 `provider-owned exact StagingRoot`라는 trust boundary가 엄밀히 유지되지 않는다.

교정 요구:

```text
권장: 기존 test fixture를 provider root 하위로 이동
Authoring/DataAssetStaging/MissileGuidePreset/__Automation...__/

또는 exact scoped test override를 runtime test scope 안에서만 활성화하고 기본값은 항상 off

금지: shared production validator에 Missile-specific sibling-root allowlist 상시 compile
```

##### P2-1 — 두 번째 provider 확장 seam을 증명하는 foundation test가 없음

현재 추가 test는 Missile descriptor parity, callback non-null, exact TypeKey lookup, class-scoped identity를 검증한다. 하지만 **두 번째 provider를 추가했을 때 shared common candidate/Review/TOCTOU가 Missile branch 없이 동작할 수 있는지**는 증명하지 않는다.

교정 권고:

```text
- test-only dummy second provider 또는 pure registry fixture를 사용한다.
- same textual StableLogicalId / other class 허용
- provider root isolation
- exact TypeKey dispatch
- mixed-type common candidate BatchPlanHash/duplicate/review seam
을 Ammo 구현 전에 검증한다.
```

##### P2-2 — Registry duplicate TypeKey fail-closed invariant가 없음

현재 registry 순회는 matching provider를 처음 발견하면 즉시 반환한다. code-owned list이므로 현재 exact1에서는 문제가 없지만 provider가 둘 이상이 되는 순간 accidental duplicate TypeKey registration이 있어도 first-match로 조용히 결정된다.

교정 권고:

```text
- registry construction/validation에서 duplicate exact TypeKey를 fail-closed한다.
- duplicate registration negative test를 추가한다.
```

#### Mid-review 결론

```text
Missile behavior/signature parity: PASS evidence preserved
Shared multi-type foundation acceptance: HOLD
Ammo implementation: 시작 금지
DAO-P0-02: HOLD
Exact next: DAO-P0-01 Correction + Re-review
```

이번 Mid-review에서는 C++/JSON/.uasset/accepted snapshot mutation을 수행하지 않았다. 기존 `CFVehicleVisualComp.cpp`, `SourceArt/UI/HUD/VehiclePanel/`, CF-FQ-039와 다른 병렬 dirty는 그대로 보존한다.

### 10.4 2026-09-09 DAO-P0-01 Correction + Re-review

#### 교정 범위

Mid-review의 blocking P1 4건과 P2 2건을 전건 교정했다.

```text
P1-1 shared core Missile record/row 의존
→ payload-free common current/preview row와 common Reviewed Approval/TOCTOU authority로 분리
→ existing Public Missile record/API는 compatibility facade로 유지

P1-2 descriptor-only registry
→ descriptor + shared-safe parse/current/apply operations를 소유하는 complete exact TypeKey provider entry로 승격
→ Stable Identity policy/resolver + DACE owner/history namespace 결속

P1-3 direct Missile fresh Apply/TOCTOU
→ reviewed source fresh re-read 후 exact provider entry parse/current/apply dispatch
→ shared core는 typed payload를 보관하지 않음

P1-4 sibling Automation root allowlist
→ `MissileGuidePreset/__Automation...__/` provider root 하위로 fixture/cleanup/residue gate 이동
→ Development Editor trust boundary를 넓히던 sibling-root allowlist 제거

P2-1 second-provider seam
→ production registry를 변경하지 않는 test-only complete second provider 추가
→ exact TypeKey dispatch / provider-root isolation / other-class same textual StableLogicalId / mixed-type common BatchPlanHash·duplicate·Reviewed Approval 검증

P2-2 duplicate TypeKey invariant
→ registry/set validation과 lookup 모두 duplicate exact TypeKey를 first-match 없이 fail-closed
```

#### Fresh final validation

Correction 이후 latest source/binary로 다시 실행한 최종 evidence다.

```text
Official UE 5.8 Editor Build
Job: 2f8afa3de9144c55b74083ca4eb02aeb
Result: PASS / Exit 0

Affected CF-FQ-049 Staging
Process: ad820e42c46545c182b319c9d872b724
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

DACE
Process: 9857fc2e265a419bb7d75c8df7f9e454
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
```

#### 보호 evidence

```text
Product Low/Normal/High Apply·Save: 0
canonical Product exact3 JSON diff: 0
Product exact3 .uasset diff: 0
accepted snapshot source diff: 0
accepted history: bootstrap exact1 / append 0
Production registered provider: MissileGuidePreset exact1
Ammo typed provider/schema/DACE extension: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

#### 재검수 판정

```text
P0: 0
blocking P1: 0
P2: 0
DAO-P0-01: Technical PASS
Shared Type Dispatch Foundation: Accepted
DAO-P0-02: READY / Not Started
Exact next: DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview
```

이 PASS는 **Ammo 구현 완료**를 뜻하지 않는다. Production registered provider는 계속 MissileGuidePreset exact1이며 `CFAmmoData` typed payload/schema/provider, Preview integration과 DACE Ammo extension은 DAO-P0-02 이후 범위다.

### 10.5 2026-09-09 DAO-P0-02 Pre-Implementation Contract Review

#### 판정

```text
P0: 0
blocking P1: 3
P2: 2
DAO-P0-02 Implementation: HOLD
DAO-P0-01 Technical PASS: Preserved
Exact next: DAO-P0-02 Contract Correction + Re-review
```

이번 검수는 DAO-P0-01 final Source와 `CFAmmoData`, Data Asset Manager Ammo identity/validation, shared Type Dispatch/Apply/Ops 경계를 read-only로 재대조했다. C++/JSON/.uasset/accepted snapshot mutation과 Build/Automation 실행은 0이다.

##### P1-1 — production provider completeness와 P0-02 read-only 단계가 충돌함

현재 `CFDATypeDispatch::ValidateProviderEntry()`는 production provider entry가 valid하려면 다음 operation을 모두 non-null로 요구한다.

```text
ParseCommonCandidate
ResolveCommonCurrentState
ApplyReviewedMutation
```

하지만 DAO-P0-02의 normative 범위는 typed schema/parse/fingerprint/extract/current-state/Preview까지인 **mutation 0 read-only 단계**이며 Ammo writer/materializer/durable Apply는 DAO-P0-03 owner다. Ammo를 실제 production TypeKey registry에 등록해야 common Preview가 exact provider lookup을 통과하지만, 현재 계약 그대로면 P0-03 writer를 선행 구현하거나 의미상 거짓인 mutation callback을 채워야 한다.

교정 요구:

```text
- provider readiness를 최소 ReadOnlyPreviewReady와 ReviewedMutationReady로 명시 분리한다.
- production registry에는 P0-02 Ammo provider를 ReadOnlyPreviewReady로 등록할 수 있어야 한다.
- ParseCommonCandidate + ResolveCommonCurrentState는 read-only provider의 required operation이다.
- ApplyReviewedMutation은 ReviewedMutationReady에서만 required다.
- shared Apply path는 provider가 ReviewedMutationReady가 아니면 source read 또는 mutation 진입 전에 fail-closed한다.
- dummy success callback이나 P0-03 materializer 선행 구현으로 단계를 우회하지 않는다.
- Missile provider는 existing ReviewedMutationReady semantics와 Public compatibility를 그대로 유지한다.
```

##### P1-2 — Missile과 Ammo가 공유해야 할 primitive semantic authority가 `CFDAStaging.cpp` file-local에 갇혀 있음

현재 exact field-set validation, canonical SHA-256 token framing, FName canonicalization, Literal FText parse/readback policy와 기본 JSON scalar parsing이 `CFDAStaging.cpp`의 private namespace implementation에 집중돼 있다. Ammo provider를 별도 source로 추가하면서 이를 복제하면 `AmmoDisplayName`의 Literal FText 의미와 semantic hash byte protocol이 Missile과 독립 구현으로 갈라지고 third provider onboarding boilerplate가 증가한다.

교정 요구:

```text
- provider-neutral primitive만 Editor Private internal helper authority로 분리한다.
- 최소 공유 대상: issue helper, exact-field validation, canonical fingerprint hash/token framing, canonical FName text, Literal FText JSON parse + persisted FText readback, basic bool/string/number validation.
- MissileGuideConfig field/range/enum과 Missile payload shape는 Missile provider에 남긴다.
- Ammo exact8 field policy와 AmmoTags/AmmoIcon semantics는 Ammo provider에 남긴다.
- shared helper extraction 전후 기존 Missile semantic fingerprint bytes와 Public API signature는 exact unchanged여야 한다.
- 새 Public DataAuthoring API는 만들지 않는다.
```

##### P1-3 — AmmoIcon strict parse와 Preview Asset Registry validation seam이 payload-free transport와 아직 결속되지 않음

Frozen contract는 `AmmoIcon` strict parser가 syntax/canonical form만 검사하고 referenced asset을 load하지 않으며, Preview/provider validation에서 Asset Registry metadata로 existence + `UTexture2D` compatibility를 검사하도록 분리한다. 하지만 current common transport는 payload-free이고 `ResolveCommonCurrentState()`에는 desired typed payload가 전달되지 않는다. 따라서 validation 위치를 정하지 않고 구현하면 parser가 Asset Registry까지 떠맡거나 common envelope에 type-specific payload를 다시 넣는 회귀가 생길 수 있다.

교정 요구:

```text
- Ammo typed ParseJson = JSON shape/type + SoftObjectPath syntax/canonical validation only.
- Ammo provider ParseCommonCandidate = typed ParseJson 결과를 provider-local stack에 유지한 상태에서 Preview reference validation을 수행한 뒤 payload-free common envelope만 반환한다.
- AmmoIcon Preview validation = Asset Registry metadata read-only / referenced asset LoadObject 0.
- non-null canonical form = valid Unreal top-level asset object SoftObjectPath, valid long package name, subobject path 없음, round-trip canonical string exact 일치.
- 특정 mount를 `/Game`으로 제한하지 않는다. valid Engine/plugin mount도 동일 canonical rule을 통과할 수 있다.
- missing asset 또는 non-Texture2D-compatible class는 `InvalidValue` + exact `Payload.AmmoIcon` blocking issue다.
- shared common envelope/Preview row에 AmmoIcon 또는 typed payload를 추가하지 않는다.
```

##### P2-1 — Ammo typed payload↔cached fingerprint split negative seam을 구현 전에 명시할 필요

P0-02 negative matrix에는 payload와 cached `StagingSemanticFingerprint`가 갈라지는 경우가 포함돼 있지만 common envelope 자체는 payload-free라 그 불일치를 재계산할 수 없다.

교정 권고:

```text
- Ammo provider-local typed record projection에서 cached fingerprint를 typed payload로 재계산해 exact 대조한다.
- provider-local integrity PASS 뒤에만 common envelope/Preview row로 투영한다.
- 이 검사는 shared core에 typed payload를 저장하는 방식으로 구현하지 않는다.
```

##### P2-2 — AmmoIcon no-load / compatible-class evidence를 test acceptance에 명시할 필요

단순 wrong-class test만 통과해도 구현이 `LoadObject`로 referenced texture를 읽은 뒤 cast하는 방식이면 frozen contract를 위반할 수 있다.

교정 권고:

```text
- null path PASS
- canonical existing Texture2D metadata PASS
- missing path blocking
- existing wrong-class metadata blocking
- referenced asset LoadObject 0을 source review + focused regression evidence로 함께 확인
```

#### 보호 확인

```text
DAO-P0-01 final Build/affected13/DACE15 evidence: preserved
Missile Product Low/Normal/High Apply·Save: 0
canonical Product exact3 JSON/.uasset mutation: 0
accepted snapshot/history mutation: 0
Ammo persisted exact2 mutation/save: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

### 10.6 2026-09-09 DAO-P0-02 Contract Correction + Source Re-review

#### 교정 범위

v0.3.3 Pre-Implementation Contract Review의 blocking P1 3건과 P2 2건을 **Ammo typed schema 본체를 시작하지 않은 상태에서** 우선 교정했다.

```text
P1-1 read-only provider readiness vs mutation capability
→ ECFDAProviderReadiness를 ReadOnlyPreviewReady / ReviewedMutationReady로 분리
→ read-only provider는 ParseCommonCandidate + ResolveCommonCurrentState만 required
→ ReviewedMutationReady에서만 ApplyReviewedMutation required
→ ReadOnlyPreviewReady가 writer callback을 가지는 경우도 invalid registration으로 fail-closed
→ shared Apply global preflight와 immediate pre-mutation path 모두 Staging source read 전에 ValidateProviderMutationReady 실행
→ 기존 Missile provider는 ReviewedMutationReady로 명시해 existing Apply semantics 보존

P1-2 provider-neutral primitive authority
→ Editor Private `CFDACommonPrimitives.h/.cpp` 신규 authority로 issue, exact-field JSON validation, scalar parser, Literal FText parse/readback, canonical FName, fingerprint token/hash primitive를 이동
→ 기존 Missile file-local helper는 compatibility wrapper로 common authority에 위임
→ MissileGuideConfig field/range/enum과 Missile payload shape는 Missile provider에 유지
→ Public DataAuthoring API 추가/변경 0
→ 기존 Missile float semantic의 -0.0f→+0.0f canonicalization도 common authority에서 그대로 보존

P1-3 AmmoIcon strict parse ↔ Preview metadata validation seam
→ ParseCanonicalSoftObjectPath: null policy, valid long package, top-level object only, subobject 금지, exact round-trip canonical string 검증
→ `/Game` mount 제한 없음; Engine/plugin mount도 동일 canonical rule 사용 가능
→ ValidateAssetReferenceMetadata: Asset Registry metadata로 existence + expected base/derived class compatibility 검증
→ referenced asset LoadObject/ResolveObject/GetAsset 호출 0인 provider-neutral helper로 분리
→ common envelope/Preview row에는 AmmoIcon 또는 typed payload를 추가하지 않음
→ 실제 Ammo provider가 이 seam을 호출하는 wiring은 P1 0 이후 Ammo typed implementation 단계가 소유

P2-1 typed payload ↔ cached fingerprint integrity
→ ValidateCachedSemanticFingerprint provider-local seam 추가
→ canonical cached/recomputed exact match만 common projection 허용
→ mismatch negative regression을 기존 focused test에 추가

P2-2 AmmoIcon no-load / compatible-class evidence
→ null path PASS regression
→ canonical `/Engine/...DefaultTexture` Texture2D metadata PASS regression
→ missing canonical asset blocking regression
→ existing wrong-class `/Engine/BasicShapes/Cube.Cube` blocking regression
→ subobject syntax blocking regression
→ source review에서 common reference validator의 LoadObject / ResolveObject / FAssetData::GetAsset 호출 0 확인
→ before/after ResolveObject state equality regression도 추가
```

#### Source 재검수

```text
P0: 0
blocking P1: 0
P2: 0
Contract Correction Source Re-review: PASS
Ammo typed schema/provider 본체: Not Started
Production registered provider: MissileGuidePreset exact1
```

P1-1은 `CFDAStagingApply.cpp`에서 global preflight와 mutation 직전 모두 `ValidateProviderMutationReady()`가 `ReadStagingFile()`보다 먼저 호출되는 것을 재대조했다. 따라서 read-only provider가 Reviewed Apply source re-read 또는 mutation path로 진입할 수 없다.

P1-2는 기존 Missile parser/fingerprint/Literal FText 경로가 provider-neutral helper authority로 재배선됐고, 타입별 payload policy는 그대로 provider-local에 남아 있음을 재대조했다. 공용 helper가 `CFDAStaging.h`의 기존 shared issue/literal transport type을 참조하는 compile dependency는 남지만, primitive implementation authority 자체는 더 이상 Missile file-local이 아니다.

P1-3은 현재 단계가 **validation seam 교정**임을 유지했다. Ammo exact8 record/parser/provider는 아직 생성하지 않았으며, 후속 Ammo `ParseCommonCandidate`가 strict typed parse 뒤 provider-local Asset Registry reference validation을 수행하고 payload-free common envelope만 반환하는 계약으로 진입할 수 있다.

#### Fresh 기술 검증 상태

GoPyMCP build authority mapping을 fresh 재확인했다. `carfight.editor.development` legacy preset은 configured repository ID `main_game`에 결속돼 있으며, Project Registry 내부 repository identity인 `carfight_repo`를 legacy preset의 `repository_id`로 사용하면 `build_preset_invalid`가 재현된다. 따라서 이번 blocker는 CarFight Source/UE compiler 문제가 아니라 **Project Registry identity와 legacy configured repository ID를 혼용한 호출 경계 문제**였다.

이번 검증에서는 GoPyMCP 설정/Source를 수정하지 않고 올바른 legacy preset authority인 `repository_id=main_game`을 사용해 official build job이 정상 생성되는 것을 확인했다.

```text
Official UE 5.8 Editor Build
Job: c191604c1f704717929c91f8d8b92f38
Result: PASS / Exit 0
Engine root: D:\UnrealEngine_Source
Target: CarFight_ReEditor Win64 Development

Affected CF-FQ-049 Staging
Process: 81210438baf4466b96059f77e4ea22b6
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

DACE
Process: 29139903dbec485e9b502dcc690697d1
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
```

Build log에서 current correction Source인 `CFDACommonPrimitives.cpp`, `CFDAMissileProvider.cpp`, `CFDATypeDispatch.cpp`, `CFDAStaging.cpp`, `CFDAStagingApply.cpp`, `CFDAStagingOps.cpp`와 관련 tests가 fresh compile/link됐고 `UnrealEditor-CarFight_ReEditor.dll` 생성까지 성공했다. affected exact13과 DACE exact15는 이 fresh binary 이후 순서대로 실행했다.

#### 재검수 판정

```text
P0: 0
blocking P1: 0
P2: 0
DAO-P0-02 Contract Correction: Source/Contract PASS
DAO-P0-02 Correction Fresh Technical Validation: PASS
Ammo implementation contract gate: UNBLOCKED
Ammo typed schema/provider 본체: Not Started
Exact next: DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview
```

사용자가 요구한 "blocking P1 0 전 Ammo typed schema 구현 금지" 조건은 지켰다. 이번 단계에서 `FCFDAAmmo*`, Ammo typed Staging record/parser/provider, AmmoTags typed adapter와 Ammo DACE descriptor 본체는 추가하지 않았다. 계약상 P1 0에 이어 current correction의 fresh compile/regression evidence까지 확보했으므로, 다음 exact gate부터 Ammo typed schema/parse/fingerprint/preview 구현을 시작할 수 있다.

#### 보호 evidence

```text
Missile Product Low/Normal/High Apply·Save: 0
canonical Product exact3 JSON/.uasset mutation: 0
accepted snapshot/history mutation: 0
Ammo persisted exact2 mutation/save: 0
Ammo typed schema/provider 본체 생성: 0
Production registry Ammo registration: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

---

### 10.7 2026-09-09 DAO-P0-02 Ammo Typed Implementation + Fresh Validation

#### 구현 범위

`DAO-P0-02`의 mutation0 read-only 범위를 다음 Editor Private source로 구현했다.

```text
CFDAAmmoProvider.h/.cpp
- FCFDAAmmoPayload exact8 typed payload
- FCFDAAmmoRecord / strict whole-record parser
- SchemaId = CarFight.DataAsset.AmmoData
- SchemaRevision = 1
- AdapterContractRevision = 1
- ClassPath = /Script/CarFight_Re.CFAmmoData
- Stable Identity = AmmoId
- Canonical StagingRoot = Authoring/DataAssetStaging/AmmoData
- semantic fingerprint
- UObject -> Ammo exact8 extractor
- exact CFAmmoData current-state resolver
- ReadOnlyPreviewReady provider entry

CFDATypeDispatch.cpp
- Production registry에 Ammo exact TypeKey를 ReadOnlyPreviewReady로 등록
- Missile ReviewedMutationReady 의미 유지

CFDAAmmoProviderTests.cpp + Tools/RunDAOTests.ps1
- DAO-P0-02 focused exact4
```

Ammo provider의 `ApplyReviewedMutation`은 `nullptr`이고 readiness는 `ReadOnlyPreviewReady`다. 따라서 current shared Apply preflight에서 Ammo는 Staging source re-read/mutation 진입 전에 fail-closed하며 DAO-P0-03 writer를 선행 구현하지 않았다.

#### Frozen semantic 구현 확인

```text
AmmoId
- required non-None FName
- StableLogicalId와 FName semantic exact 일치

AmmoDisplayName
- Literal FText
- persisted Literal readback은 common primitive 사용

AmmoFamilyId
- NAME_None 허용 FName

UnitMassKg
- finite float >= 0
- negative/non-finite reject
- authored clamp 0

AmmoTags
- Array<String>
- set-like FName semantic
- semantic duplicate reject
- physical order/case variation은 semantic fingerprint 변화 0
- deterministic canonical sort

AmmoIcon
- null 또는 canonical top-level SoftObjectPath
- subobject/noncanonical reject
- fingerprint는 null/path만 포함
- Preview validation은 Asset Registry metadata existence + UTexture2D compatibility
- referenced asset LoadObject/ResolveObject/GetAsset 0
- null/existing Texture2D/missing/wrong-class regression 포함

MaximumLoadableAmmoCount
- exact nonnegative int32
- negative reject / clamp 0

bCanBeResupplied
- exact bool
```

`AmmoIcon` no-load source 감사에서 common `ValidateAssetReferenceMetadata()`는 Asset Registry metadata만 조회하는 것을 재확인했다. `CFDAAmmoProvider.cpp`의 `ResolveObject/GetAsset()` 사용은 requested target `CFAmmoData` semantic readback과 exact-class `AmmoId` identity scan을 위한 것이며 referenced Texture asset에는 사용하지 않는다.

#### Fresh final validation

최종 missing-icon regression 보강 뒤 fresh binary 기준 evidence다.

```text
Official UE 5.8 Editor Build
Job: 32f46c889baf4bfdad6b1eebf7b2e0b1
Result: PASS / Exit 0

DAO-P0-02 focused
Process: 381bf95e6628439585157d0d3e46d0fa
Exact: 4/4 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0

Affected CF-FQ-049 Staging
Process: 2d0e1e80e0244dc0b9f0520ffd249f01
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

DACE
Process: adb7c455699742f980e70a8d22aff4d5
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
```

첫 구현 build `8934c6eb19494ef1bea6f5ed27591d6c`는 production Ammo provider가 아니라 test fixture의 UE API 오기 `TNumericLimits<float>::Infinity()` 때문에 compile 실패했다. 이를 `std::numeric_limits<float>::infinity()`로 교정한 뒤 build `a7e03fbc108c4adb8cd4977dbbfb1220`이 PASS했고, 마지막 missing-icon regression 보강 뒤 위 `32f46...` fresh build와 exact4/13/15를 다시 PASS했다.

#### 보호 evidence

scoped worktree diff를 fresh 확인했다.

```text
Missile Product Low/Normal/High canonical JSON exact3 diff: 0
Missile Product Low/Normal/High .uasset exact3 diff: 0
CFDAContractBase.cpp accepted baseline diff: 0
Ammo persisted HeavyFinite/RocketFinite exact2 .uasset diff: 0
Missile Product Low/Normal/High Apply·Save: 0
Ammo persisted exact2 Save: 0
accepted snapshot/history append/rewrite: 0
canonical Ammo Product target 생성: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

#### 현재 판정

```text
DAO-P0-02 implementation: Complete
Fresh Build/Focused/Affected validation: PASS
Production registry: Missile ReviewedMutationReady + Ammo ReadOnlyPreviewReady exact2
Ammo durable writer/materializer: Not Implemented — DAO-P0-03 owner
Ammo DACE descriptor/probe/history: Not Implemented — DAO-P0-04 owner
DAO-P0-02 Technical Acceptance: 아직 확정하지 않음
Exact next: DAO-P0-02 Post-Implementation Mid-review
```

Fresh tests가 PASS했더라도 Post-Implementation Mid-review에서 abstraction leak, read-only boundary, exact8 semantic coverage와 third-type reuse risk를 current Source 기준으로 다시 검수하기 전 `DAO-P0-02 Technical PASS` 또는 `DAO-P0-03 READY`로 확대하지 않는다.

### 10.8 2026-09-10 DAO-P0-02 Post-Implementation Mid-review

`DataAssetAuthoring.md v1.3.1`을 actual Source의 현재 구현 상태에 먼저 동기화한 뒤 `CFDAAmmoProvider`, `CFDATypeDispatch`, `CFDACommonPrimitives`, `CFDAStaging`, `CFDAStagingApply`와 focused/foundation tests를 current Source 기준으로 재감사했다.

#### 판정

```text
P0: 0
blocking P1: 2
P2: 2
DAO-P0-02 Technical Acceptance: HOLD
DAO-P0-03 writer: HOLD / Not Started
Exact next: DAO-P0-02 Correction + Re-review
```

기존 fresh Build/Automation PASS는 구현 동작과 회귀 부재 evidence로 계속 유효하지만, 아래 architecture/reuse blocker를 해소하지 않으므로 Technical Acceptance로 확대하지 않는다.

#### P1-1 — common TargetObjectPath validation authority가 second provider에서 복제됨

`TargetObjectPath`는 typed payload가 아니라 모든 provider가 공유하는 `FCFDACommonEnvelope` field다. 그러나 current Source에는 같은 canonical `/Game/.../Asset.Asset` 규칙이 다음 두 곳에 별도 구현돼 있다.

```text
CFDAStaging.cpp
- CFDAStagingPrivate::ValidateTargetObjectPath
- existing Missile/common Preview path authority

CFDAAmmoProvider.cpp
- CFDAAmmoProviderPrivate::ValidateTargetObjectPath
- Ammo parse/current resolver용 신규 복제
```

두 구현은 `/Game/` prefix, colon/backslash 차단, package/object separator, `FPackageName::IsValidLongPackageName`, package leaf == object name과 diagnostics까지 사실상 동일하다. 두 번째 실제 provider를 추가했는데도 common envelope validator를 재사용하지 못하고 복제했으므로 third type onboarding 시 동일 코드가 다시 늘어날 구조다.

교정 방향:

```text
- TargetObjectPath canonical validation authority를 CFDACommonPrimitives 같은 provider-neutral common owner로 이동
- 기존 CFDAStaging compatibility path와 CFDAAmmoProvider가 같은 helper를 호출
- shared BuildCommonPreview도 같은 authority를 사용
- Missile behavior/diagnostic 의미 delta 0 유지
```

#### P1-2 — BaseSemanticFingerprint physical contract parsing이 Ammo provider에 다시 복제됨

`BaseSemanticFingerprint` 역시 common envelope field인데 physical `null|string`, canonical `sha256:` 형식과 diagnostic policy가 타입별 parser에 중복돼 있다.

```text
CFDAStaging.cpp
- existing Missile BaseSemanticFingerprint parse/validation

CFDAAmmoProvider.cpp
- CFDAAmmoProviderPrivate::ParseBaseFingerprint
```

두 경로가 `null = Create baseline 없음`, `string = canonical SHA-256`, 다른 JSON type reject라는 같은 공용 계약을 각각 구현한다. 이 상태로 DamageData third provider를 추가하면 envelope parsing boilerplate가 또 복제되며 provider별 diagnostic/acceptance drift 가능성이 생긴다.

교정 방향:

```text
- BaseSemanticFingerprint physical parse + canonical validation을 provider-neutral common helper로 승격
- Missile compatibility parser와 Ammo parser가 동일 helper 사용
- provider-local에는 payload parsing만 남김
```

#### P2-1 — actual Ammo provider의 BatchPlanHash integration regression 부재

DAO-P0-02 normative 구현 범위에는 `Preview / BatchPlanHash integration`이 포함돼 있다. shared `BuildCommonBatchPlanHash()` 자체는 provider-neutral이며 `CFDAStagingTests`의 synthetic second-provider mixed batch로 이미 증명돼 있다.

하지만 actual `CFDAAmmoProvider`에서 생성한 payload-free envelope/Preview row가 `BuildCommonBatchPlanHash()`에 들어가 deterministic hash를 만드는 regression은 current Ammo exact4에 없다. 구현 오류 evidence는 아니지만 second real provider integration acceptance 근거가 부족하다.

교정 방향:

```text
- actual Ammo ParseCommonCandidate/BuildCommonPreview 결과로 BatchPlanHash 생성 PASS
- 가능하면 Missile + actual Ammo mixed rows의 physical order reversal hash equality도 확인
- Product/asset mutation 0 유지
```

#### P2-2 — frozen optional Ammo semantic의 actual focused evidence 부족

P0-00에서 다음 의미를 동결했다.

```text
AmmoDisplayName: empty Literal 허용
AmmoFamilyId: NAME_None 허용
```

Source는 이를 허용하지만 current Ammo focused tests는 non-empty DisplayName + non-None FamilyId fixture만 사용한다. 따라서 exact8 semantic coverage에서 허용 경계가 직접 증명되지 않았다.

교정 방향:

```text
- empty Literal AmmoDisplayName valid regression
- empty string -> NAME_None AmmoFamilyId valid regression
- semantic fingerprint 생성까지 PASS
```

#### PASS로 확인한 핵심 경계

다음 항목은 current Source 재검수에서 문제를 찾지 못했다.

```text
- Ammo exact8 typed payload는 provider-local이며 common envelope에 payload leak 0
- shared BuildCommonPreview가 Ammo에도 동일 3-way Create/Update/NoChange/Conflict authority 제공
- class-scoped StableLogicalId / global TargetObjectPath duplicate policy는 shared core 소유
- ReadOnlyPreviewReady Ammo provider의 ApplyReviewedMutation = nullptr
- shared Apply는 global preflight와 mutation 직전 모두 ValidateProviderMutationReady를 source read보다 먼저 수행
- 따라서 Ammo Reviewed mutation/source re-read 진입 차단 유지
- AmmoTags set-like FName semantic / duplicate reject / order-independent fingerprint 유지
- AmmoIcon referenced Texture는 metadata-only existence/class validation; referenced asset load 0
- current CFAmmoData target readback/AmmoId scan의 UObject load는 referenced Texture load와 분리된 read-only current-state 관측
- Public Missile compatibility API signature delta 0
```

#### 검증 재실행 여부

이번 Mid-review에서 C++/script/asset을 수정하지 않았고 `DataAssetAuthoring.md` current projection과 대표 Plan만 갱신했다. 따라서 v0.3.6에서 확보한 fresh binary evidence를 무의미하게 반복 실행하지 않았다.

```text
Implementation baseline retained:
Build `32f46c889baf4bfdad6b1eebf7b2e0b1` PASS
DAO focused `381bf95e6628439585157d0d3e46d0fa` 4/4 PASS
affected `2d0e1e80e0244dc0b9f0520ffd249f01` 13/13 PASS
DACE `adb7c455699742f980e70a8d22aff4d5` 15/15 PASS
```

C++ correction 뒤에는 반드시 fresh Build → DAO focused → affected13 → DACE15 순서로 다시 검증한다.

#### 보호 상태

```text
DAO-P0-03 writer/materializer implementation: 0
Product Low/Normal/High Apply·Save: 0
canonical Product exact3 mutation: 0
accepted baseline/history mutation: 0
Ammo persisted exact2 mutation/save: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

### 10.9 2026-09-10 DAO-P0-02 Correction + Re-review

v0.3.7 Post-Implementation Mid-review의 blocking P1 2건과 P2 2건을 current Source에서 전건 교정한 뒤 Source 재검수와 fresh UE 5.8 Build/Automation을 수행했다.

#### 교정 결과

```text
P1-1 TargetObjectPath common validation duplication
→ CFDACommonPrimitives::ValidateTargetObjectPath 단일 authority로 통합
→ CFDAStaging Missile compatibility path와 CFDAAmmoProvider가 동일 helper 호출

P1-2 BaseSemanticFingerprint common physical parsing duplication
→ CFDACommonPrimitives::ParseBaseSemanticFingerprint 단일 authority로 통합
→ null/string + canonical sha256 validation/diagnostic policy 공유

P2-1 actual Ammo BatchPlanHash integration evidence 부재
→ 실제 CFDAAmmoProvider가 parse한 두 candidate의 common Preview row로 hash 생성
→ physical row order reversal에도 exact 동일 canonical hash PASS

P2-2 optional Ammo semantic focused evidence 부재
→ empty Literal AmmoDisplayName valid PASS
→ empty string AmmoFamilyId → NAME_None valid PASS
→ semantic fingerprint canonical PASS
```

Source search/review에서 TargetObjectPath와 BaseSemanticFingerprint의 실제 diagnostic/acceptance 구현은 `CFDACommonPrimitives` 각 1곳만 남았고 Missile/Ammo는 delegation만 소유한다. Ammo provider는 계속 `ReadOnlyPreviewReady`이며 `ApplyReviewedMutation=nullptr`이다. DAO-P0-03 writer/materializer/save path는 추가하지 않았다.

#### Source Re-review 판정

```text
P0: 0
blocking P1: 0
P2: 0
Source Re-review: PASS
```

#### Fresh final evidence

```text
Official UE 5.8 Editor Build
Job: 898d428caeba4895bb5b0668af513a6e
Result: PASS / Exit 0
Fresh compile/link: CFDACommonPrimitives.cpp / CFDAAmmoProvider.cpp / CFDAAmmoProviderTests.cpp / CFDAStaging.cpp

DAO focused
Process: b6cb4476ec6d47f4a50214901498b0ed
Exact: 4/4 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0

Affected CF-FQ-049 Staging
Process: d6e52207e9d8436d9fae7c27f2027d80
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

DACE
Process: 8789b7e7a1c2446dae91db3bf3f9f7c2
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
```

#### Final protection audit

보호 exact9 scoped worktree diff는 모두 0이다.

```text
Missile Product canonical JSON exact3 mutation: 0
Missile Product .uasset exact3 mutation: 0
accepted CFDAContractBase.cpp mutation: 0
Ammo persisted HeavyFinite/RocketFinite exact2 mutation: 0
Product Low/Normal/High Apply·Save: 0
accepted history append/rebaseline: 0
DAO-P0-03 writer/materializer implementation: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

#### 최종 판정

```text
DAO-P0-02 Correction + Re-review: Technical PASS
P0: 0
blocking P1: 0
P2: 0
DAO-P0-02: Complete
DAO-P0-03: Historical Ready checkpoint
```

### 10.10 2026-09-10 DAO-P0-03 Pre-Implementation Contract Review + Implementation

DAO-P0-02 Technical PASS baseline에서 Ammo durable writer를 열기 전에 current shared Apply/Missile durable Source와 protected target ownership을 먼저 검수했다.

#### 구현 전 계약검수

```text
P0: 0
blocking P1: 2
P2: 0
Initial implementation gate: HOLD until P1 correction
```

P1-1은 Review/TOCTOU/Batch orchestration은 이미 payload-free shared core였지만 실제 `CreatePackage → typed materialize → pre-save readback → SavePackage → disk reload → persisted typed readback → rollback/uncertainty` body가 `CFDAStagingApply.cpp`의 Missile exact 구현에 남아 있던 문제다. 이를 Ammo에 복사하면 두 번째 타입에서 durable algorithm duplication이 발생하므로 허용하지 않았다.

P1-2는 persisted `HeavyFinite` / `RocketFinite` exact2가 기존 AmmoIntegration test-owned fixture이지만 이번 writer acceptance를 위해 직접 overwrite/save할 authority는 없다는 점이다. 따라서 두 asset을 writer fixture로 사용하지 않고 완전히 별도의 disposable automation root를 사용하도록 동결했다.

```text
Disposable Content root:
/Game/Test/CarFight/DAOAmmoP03/

Disposable Staging root:
Authoring/DataAssetStaging/AmmoData/__AutomationP03__/
```

#### 교정 + 구현

- Editor Private `CFDADurableCore.h/.cpp`를 추가해 typed durable sequencing을 provider-neutral 단일 authority로 분리했다.
- 기존 Missile writer도 새 durable core를 사용하도록 rewire하고 이전 Missile-only durable transaction dead copy를 제거했다.
- typed provider는 payload definition/parse/fingerprint/extract/materialize 의미만 소유하고, durable core는 Create/Update sequencing, exact single-package SavePackage, reload/readback, rollback/uncertainty aggregation을 소유한다.
- Ammo provider를 `ReviewedMutationReady`로 승격하고 `ApplyReviewedMutation` callback을 등록했다.
- Ammo materializer는 exact8 authored field를 deterministic whole-record로 적용하고 AmmoTags는 canonical sort, AmmoIcon은 soft path 그대로 materialize한다.
- `ApplyReviewedMutation`은 reviewed source를 fresh re-parse하고 cached fingerprint, referenced AmmoIcon metadata, common envelope binding을 다시 대조한 뒤에만 durable core에 진입한다.
- generic Reflection writer, arbitrary property map, Save All, Product Ammo auto-create는 추가하지 않았다.

#### DAO-P0-03 focused durable coverage

```text
DurableRoundTrip
- disposable Ammo Create durable PASS
- same target Update durable PASS
- non-empty AmmoTags persisted round-trip PASS
- non-null Texture2D AmmoIcon persisted round-trip PASS
- exact8 semantic fingerprint readback PASS
- package clean PASS

StaleDirtyGuards
- Review 뒤 Staging 변경 → Apply blocked before mutation PASS
- pre-existing dirty target → Conflict / TargetDirtyUnowned PASS

SaveUncertainty
- forced SavePackage outcome uncertainty
→ SaveStateUnconfirmed PASS
→ DurableAppliedCount 0 PASS
```

모든 P0-03 disposable Content/Staging fixture는 각 test teardown에서 삭제되고 최종 Git status에도 residue가 남지 않았다.

#### Fresh final evidence

```text
Official UE 5.8 Editor Build
Job: 0ef9b9c2d9724b749b4226153e8e8ba6
Result: PASS / Exit 0

DAO focused
Process: 719a71370ff142c78850a3bc867c1f52
Exact: 7/7 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0

Affected CF-FQ-049 Staging
Process: bcb9e15f9b8942d886768896a35b0bf2
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

DACE
Process: 977d1bc1566f41d7b805e608cd8e1dcb
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
```

#### Protection audit

보호 exact9 scoped worktree diff는 0이다.

```text
Missile Product canonical JSON exact3 mutation: 0
Missile Product .uasset exact3 mutation: 0
accepted CFDAContractBase.cpp mutation: 0
Ammo persisted HeavyFinite/RocketFinite exact2 mutation/save: 0
Product canonical Ammo target: exact0 유지
DAO-P0-03 disposable fixture residue: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

#### 현재 판정

```text
DAO-P0-03 implementation: Complete
Fresh technical validation: PASS
Post-Implementation Mid-review: Historical pending checkpoint
DAO-P0-03 Technical Acceptance: 아직 미확정
DAO-P0-04 Ammo DACE: NOT STARTED / HOLD
```

### 10.11 2026-09-10 DAO-P0-03 Post-Implementation Mid-review

v0.3.9의 구현 + fresh validation checkpoint를 기준으로 current Source와 Current Systems를 독립 재검수했다. 이번 단계에서는 기능 구현/교정 Source mutation을 수행하지 않았고, 기존 PASS를 승인으로 자동 승격하지 않았다.

#### 재확인한 기술 계약

```text
CFDADurableCore durable transaction authority: single authority 확인
Production provider registry: MissileGuidePreset + AmmoData exact2
Provider readiness: exact2 모두 ReviewedMutationReady
Ammo exact8 deterministic materializer: 구현 확인
Shared Review / approval / TOCTOU: provider-neutral common row 유지
Generic Reflection writer: 없음
Ammo DACE CFDAContract* implementation: 0 / DAO-P0-04 NOT STARTED
```

DataAuthoring 범위 전역 검색에서 typed Staging의 `UPackage::SavePackage`, Create/Update/reload/readback/rollback body는 `CFDADurableCore`에 집중되어 있고 Ammo provider에는 durable transaction 복사본이 없다. Actual `ApplyTypedTarget<>` production caller는 Missile/Ammo exact2이며, 각 provider는 typed materialize/extract/fingerprint 의미만 전달한다.

Ammo provider의 exact8 materializer는 `AmmoId`, Literal `AmmoDisplayName`, `AmmoFamilyId`, `UnitMassKg`, canonical-sorted `AmmoTags`, soft-path `AmmoIcon`, `MaximumLoadableAmmoCount`, `bCanBeResupplied`를 whole-record로 적용한다. Mutation callback은 fresh typed parse, cached fingerprint integrity, AmmoIcon metadata validation, common envelope binding 재검증 뒤 shared durable core에 진입한다.

#### Fresh protection / residue evidence

```text
Fresh AssetDump
Root: /Game/Test/CarFight/DAOAmmoP03
Class: CFAmmoData
Asset count: 0
Failed: 0
Result: disposable persisted residue 0

Protected exact9 scoped Git diff: 0
- Missile Product canonical JSON exact3: mutation 0
- Missile Product .uasset exact3: mutation 0
- accepted CFDAContractBase.cpp: mutation 0
- Ammo persisted HeavyFinite/RocketFinite exact2: mutation 0
```

main_game status에도 DAOAmmoP03 disposable Content/Staging residue가 나타나지 않는다. 기존 병렬 dirty와 CF-FQ-039 Active는 정리/변경하지 않았다.

#### Mid-review findings

**P1-1 — Current Systems §10 Typed Materializer가 stale Missile-only Current contract를 제공함**

`DataAssetAuthoring.md v1.3.3` 상단/최신 상태는 MissileGuidePreset + AmmoData exact2 `ReviewedMutationReady`와 shared `CFDADurableCore`를 올바르게 설명하지만, §10 본문은 아직 `지원 대상은 정확히 ... 하나` / `UCFMissileGuidePresetData` 전용 Create 흐름을 Current 설명으로 유지한다. Current Systems가 현재 구현의 authoritative owner이므로 다음 세션이 Ammo durable writer를 미구현으로 오판하거나 다시 타입별 materializer/durable flow를 복제할 위험이 있다. 실제 runtime defect는 아니지만 Current contract contradiction이므로 **blocking P1**이다.

**P2-1 — CFDAAmmoProvider.cpp readiness 관련 잔여 source comment drift**

실제 `CFDAAmmoProvider::GetProvider()`는 `ReviewedMutationReady`이며 `ApplyReviewedMutation`도 non-null이지만, provider 초기화/current resolver 인접 주석 일부가 아직 `read-only Ammo provider entry`라고 표현한다. 실행 계약에는 영향이 없으나 Source를 읽는 다음 작업자의 readiness 판단을 흐릴 수 있으므로 P2로 교정한다.

#### Binary evidence 처리

이번 mid-review에서는 Source code mutation이 없으므로 직전 v0.3.9 final binary evidence를 불필요하게 재실행하지 않았다.

```text
Official UE 5.8 Build: 0ef9b9c2d9724b749b4226153e8e8ba6 PASS
DAO focused: 719a71370ff142c78850a3bc867c1f52 7/7 PASS
Affected CF-FQ-049: bcb9e15f9b8942d886768896a35b0bf2 13/13 PASS
DACE: 977d1bc1566f41d7b805e608cd8e1dcb 15/15 PASS
```

이 evidence는 여전히 유효하지만 Mid-review의 blocking P1을 해소하지 못하므로 DAO-P0-03 Technical Acceptance로 자동 승격하지 않는다.

#### Mid-review 판정

```text
P0: 0
blocking P1: 1
P2: 1
Result: HOLD
DAO-P0-03 implementation: 유지
DAO-P0-03 prior fresh binary evidence: 유지
DAO-P0-03 Technical Acceptance: NOT PASS
DAO-P0-04 Ammo DACE: NOT STARTED / HOLD
Exact next: DAO-P0-03 Correction + Re-review
```

### 10.12 2026-09-10 DAO-P0-03 Correction + Re-review

v0.3.10 Mid-review의 blocking P1 1건/P2 1건만 최소 교정하고 current Source/Systems/Plan을 독립 재검수했다. DAO-P0-04 구현은 시작하지 않았다.

#### Correction

```text
DataAssetAuthoring.md
- §2 Current provider projection을 MissileGuidePreset + AmmoData exact2 ReviewedMutationReady로 정렬
- §10 Typed Materializer를 exact2 provider + CFDADurableCore single durable authority 계약으로 교정
- §18 / §20 Current checkpoint와 owner/migration projection을 DAO-P0-03 accepted state로 정렬

CFDAAmmoProvider.cpp v1.2.1
- provider initialization의 stale "read-only Ammo provider entry" 주석 교정
- current resolver의 stale "exact read-only entry" 주석 교정
- executable behavior delta 0
```

#### Independent re-review

current Source에서 다음을 다시 확인했다.

```text
CFDADurableCore
= typed Staging Create/Update
= original typed snapshot
= typed whole-record materialize
= pre-save typed semantic readback
= exact single-package SavePackage
= disk reload
= post-save typed semantic readback
= rollback / uncertainty sequencing
= single durable transaction authority

Production provider exact2
- MissileGuidePreset: ReviewedMutationReady
- AmmoData: ReviewedMutationReady

Provider-owned type semantics
- parse / fingerprint / extract / materialize
- durable transaction algorithm 복제 0

Ammo exact8 materializer
- AmmoId
- AmmoDisplayName
- AmmoFamilyId
- UnitMassKg
- AmmoTags canonical sort
- AmmoIcon soft path
- MaximumLoadableAmmoCount
- bCanBeResupplied

Shared Review / approval / TOCTOU
= provider-neutral common row 유지

CFDAContract* AmmoData / DACE-AmmoData / FCFDAAmmo implementation match
= 0
= DAO-P0-04 NOT STARTED
```

#### Fresh validation

```text
Official UE 5.8 Build
Job: bc420d5554e849fdbad32e65296a7188
Result: PASS / exit 0
Actual recompile: CFDAAmmoProvider.cpp

DAO focused exact7
Job: b3773a90332b4d6295670a37f276d44f
Result: 7/7 PASS / exit 0
Failure/Missing/Unexpected/DuplicateTerminal: 0

Affected CF-FQ-049
Prior valid: bcb9e15f9b8942d886768896a35b0bf2 13/13 PASS
Re-run: No — executable semantics delta 0

DACE
Prior valid: 977d1bc1566f41d7b805e608cd8e1dcb 15/15 PASS
Re-run: No — DACE/source contract executable semantics delta 0
```

comment/document-only correction이므로 affected13/DACE15를 습관적으로 다시 실행하지 않았다. 새 UE 5.8 build와 DAO exact7은 current translation unit/binary에서 fresh PASS했다.

#### Fresh protection / residue

```text
AssetDump
Root: /Game/Test/CarFight/DAOAmmoP03
Class: CFAmmoData
Asset count: 0
Failed: 0

Protected exact9 scoped Git diff: 0
- Missile Product canonical JSON exact3
- Missile Product .uasset exact3
- accepted CFDAContractBase.cpp
- Ammo persisted HeavyFinite/RocketFinite exact2

Product canonical Ammo: exact0 유지
CF-FQ-039 Active: unchanged
기존 병렬 dirty: preserved
clean/revert/stash/reset/commit/push: 0
```

#### Correction + Re-review 판정

```text
P0: 0
blocking P1: 0
P2: 0
Result: Technical PASS
DAO-P0-03 Technical Acceptance: PASS
DAO-P0-04 Ammo DACE: Ready / NOT STARTED
Exact next: DAO-P0-04 Ammo DACE Contract Provider / Revision / Migration Guard
```

### 10.13 2026-09-10 DAO-P0-04 Pre-Implementation Contract Review

DAO-P0-03 Technical PASS를 baseline으로 AmmoData의 independent DACE descriptor/probe/history namespace, revision guard와 migration guard 경계를 current Source 기준으로 검수했다. 이 단계는 read-only Source/contract review이며 DAO-P0-04 C++ 구현은 시작하지 않았다.

#### 확인된 재사용 가능 foundation

```text
CFDATypeDispatch provider descriptor
- exact TypeKey = SchemaId + DataAssetTypeClassPath
- SchemaRevision
- AdapterContractRevision
- DaceContractOwnerName
- DaceAcceptedHistoryNamespace

공용으로 재사용 가능한 기존 DACE 알고리즘
- deterministic descriptor row hashing
- BuildContractBundleSignature
- BuildSnapshotSignature
- ValidateRevisionGuard
- ValidateAcceptedSnapshotChain
- ValidateMigrationResolution
- EvaluateMigrationGate
```

다만 current `FCFDAContractGuard`의 Current descriptor/reflection/probe/history/current revision/current migration wrapper는 MissileGuidePreset 하나에 직접 결속돼 있으므로 Ammo를 단순 분기 추가하는 방식은 허용하지 않는다.

#### P1-1 — TypeKey별 DACE provider / history / revision authority 부재

현재 `CFDATypeDispatch`에는 `DaceContractOwnerName`과 `DaceAcceptedHistoryNamespace` metadata가 존재하지만 DACE가 이 metadata를 실제 dispatch authority로 소비하지 않는다. `FCFDAContractGuard::GetAcceptedSnapshots()`, `BuildCurrentSignatures()`, `ValidateCurrentRevisionGuard()`, `EvaluateCurrentMigrationGate()`와 production probe entry는 모두 Missile 전용 단일 entry다.

교정 계약:

```text
- DACE common algorithm과 per-TypeKey contract provider를 분리한다.
- MissileGuidePreset과 AmmoData는 exact TypeKey별 descriptor/probe/history/current declaration/canonical-target authority를 각각 소유한다.
- accepted history는 서로 다른 chain으로 유지하며 한 배열에 Missile/Ammo record를 혼합하지 않는다.
- Ammo bootstrap은 별도 history owner에 생성하고 protected CFDAContractBase.cpp를 수정하지 않는다.
- 기존 FCFDAContractGuard + DACE15는 Missile compatibility facade/regression으로 보존한다.
- 한 TypeKey의 revision/change declaration이 다른 TypeKey의 revision/history gate에 영향을 주지 않는다.
```

#### P1-2 — AmmoTags array element structural observation 계약 미완성

Ammo parser/fingerprint는 이미 `AmmoTags`를 Array<String> + FName semantic set으로 처리하지만 current DACE serializer shape observer는 JSON Array 자체만 관측하고 element를 재귀 관측하지 않는다. 따라서 P0 normative `Payload.AmmoTags[]` element contract를 현재 DACE가 증명할 수 없다.

교정 계약:

```text
Adapter descriptor에 별도 element row를 사용한다.
- AdapterJsonPath: Payload.AmmoTags[]
- JsonValueKind: String
- RepresentationKind: FNameToken

공용 JSON observer는 non-empty Array의 element kind를 synthesized [] path로 관측한다.
Ammo production serializer probe는 non-empty AmmoTags를 사용한다.
negative regression:
- wrong element JSON type reject
- FName semantic duplicate reject
- physical permutation이 semantic fingerprint를 바꾸지 않음
```

기존 descriptor canonical row format 자체를 변경하지 않아 Missile accepted Adapter signature를 건드리지 않는다.

#### P1-3 — AmmoIcon SoftObject target-class Reflection 계약 미완성

`CFAmmoData::AmmoIcon`은 `TSoftObjectPtr<UTexture2D>`이지만 current `GetReflectedTypePath()`는 scalar Object/SoftObject property의 target class path를 반환하지 않는다. 이 상태에서는 `UTexture2D` target-class drift를 DACE SourceShape가 고정할 수 없다.

교정 계약:

```text
AmmoIcon Source descriptor
- PropertyKind: stable SoftObject token
- ReflectedTypePath: canonical UTexture2D class path
- ContainerKind: Scalar

공용 Reflection observer는 scalar Object/SoftObject의 target class path를 existing ReflectedTypePath에 기록한다.
Ammo target-class mismatch negative fixture를 추가한다.
```

새 descriptor column을 추가하지 않으며, Missile authored Source에 scalar Object/SoftObject가 없으므로 current Missile Source descriptor/signature는 exact unchanged여야 한다.

#### P1-4 — per-TypeKey canonical Staging migration gate와 Ammo exact0 의미 미동결

current `ValidateCurrentCanonicalStagingCompatibility()`는 Missile Product exact3 path를 hard-code한다. Ammo Product canonical target은 현재 exact0이므로 이 wrapper를 그대로 재사용하면 exact0을 missing Product처럼 잘못 취급하거나 fake Ammo Product asset/Staging을 만들 유인이 생긴다.

교정 계약:

```text
canonical Staging compatibility target set은 per-TypeKey contract provider가 소유한다.
MissileGuidePreset: current Product exact3 read-only set 유지
AmmoData: current Product canonical exact0을 explicit valid empty target set으로 허용

empty set PASS는 "모든 존재 target이 current parser/revision compatible"이라는 의미다.
이는 Ammo DACE bootstrap/contract probe 자체를 생략한다는 의미가 아니다.
old SchemaRevision / AdapterContractRevision compatibility는 memory-only 또는 test-owned Ammo JSON으로 검증한다.
Product Ammo를 만들거나 HeavyFinite/RocketFinite를 canonical Product로 재분류하지 않는다.
```

#### Revision / migration freeze

```text
Ammo current baseline
SchemaId: CarFight.DataAsset.AmmoData
SchemaRevision: 1
AdapterContractRevision: 1
Class: /Script/CarFight_Re.CFAmmoData
History namespace: DACE-AmmoData

SchemaRevision bump
= Ammo Adapter physical JSON shape 변화에 필요

AdapterContractRevision bump
= Ammo typed semantic contract 변화에 필요

Source/mapping-only safe refactor
= same revision 가능하지만 exact Ammo candidate에 결속된 explicit NoMigration declaration 필요

No actual Ammo contract delta
= declaration nullptr / duplicate accepted append 금지

Ammo change/revision/migration declaration
= Missile accepted history/revision에는 영향 0
```

Ammo production serializer는 P0-04 구현에서 typed provider-owned production path로 추가해야 하지만, 이는 이번 pre-review에서 확인된 정상 구현 항목이며 별도의 blocker로 계산하지 않는다. generic Reflection serializer/writer는 허용하지 않는다.

#### Pre-Implementation 판정

```text
P0: 0
blocking P1: 4
P2: 0
Result: HOLD
DAO-P0-04 implementation: NOT STARTED
Build/Automation: NOT RUN — Source mutation 0
Protected Product/accepted/HeavyFinite/RocketFinite mutation: 0
Exact next: DAO-P0-04 Contract Correction + Re-review
```

P1 4건을 계약/Source 구조로 교정하고 재검수에서 P0/P1 blocker 0이 확인되기 전에는 Ammo DACE descriptor/probe/bootstrap implementation으로 진입하지 않는다.

### 10.14 2026-09-10 DAO-P0-04 Contract Correction + Re-review

v0.3.12 Pre-Implementation Contract Review의 blocking P1 4건을 대상으로 실제 Ammo DACE bootstrap을 만들지 않고 먼저 shared DACE boundary를 교정했다. 교정 뒤 자동 테스트 결과와 별개로 Source를 다시 독립 검수했고, 재검수 중 발견한 `ContractNotReady` provider의 migration/promotion 우회 가능성까지 추가로 fail-closed한 뒤 최종 판정했다.

#### Correction 구현

```text
CFDATypeDispatch v1.5.0
- authoring readiness와 독립된 ECFDADaceReadiness 추가
- ContractNotReady / ContractReady 분리
- per-TypeKey explicit canonical Staging target set 추가
- declared empty set과 undeclared 상태 분리
- provider root containment + duplicate target validation

Missile provider v1.3.0
- DACE ContractReady
- DACE-MissileGuidePreset namespace 유지
- 기존 Product canonical Low/Normal/High exact3를 provider-owned target set으로 이동

Ammo provider v1.3.0
- authoring ReviewedMutationReady 유지
- DACE ContractNotReady
- DACE-AmmoData namespace 유지
- Product canonical exact0을 explicit declared empty target set으로 고정
- accepted Ammo history/bootstrap 추가 0

CFDAContractGuard v1.4.0
- 기존 Missile public/private facade와 DACE15 entry 유지
- descriptor signature hashing의 provider-neutral input seam 추가
- arbitrary native DataAsset direct authored Reflection observation 추가
- Array non-empty element를 `[]` synthesized path로 관측
- scalar SoftObject/Object target class를 ReflectedTypePath로 관측
- provider-bound revision guard / accepted history validation 추가
- provider production parser 기반 canonical Staging compatibility 추가
- provider DACE readiness를 실제 migration append/promotion gate에 fail-closed 결속
```

#### P1-1 Re-review — per-TypeKey DACE authority

`DaceContractOwnerName`/`DaceAcceptedHistoryNamespace` metadata만 존재하던 상태에서 벗어나 DACE readiness, canonical target set, revision/history/migration gate가 selected provider identity를 실제 입력으로 사용한다. `ValidateAcceptedSnapshotChainForProvider()`는 history record의 SchemaId/ClassPath와 SnapshotId namespace를 selected provider에 결속해 cross-TypeKey chain contamination을 차단한다. `ValidateRevisionGuardForProvider()`와 `EvaluateMigrationGateForProvider()`는 `ContractReady`가 아니면 fail-closed한다.

재검수 중 generic `EvaluateMigrationGate()`만 사용하면 Ammo exact0 compatibility PASS가 `ContractNotReady` 상태에서도 Current promotion prerequisite를 True로 만들 수 있는 추가 우회 가능성을 발견했다. 이를 provider-aware gate로 교정하고 final Build/DAO/DACE를 다시 수행했다.

판정: **P1 Closed**.

#### P1-2 Re-review — AmmoTags[] element observation

공용 JSON observer가 non-empty Array의 homogeneous physical element kind를 `Payload.AmmoTags[]` 같은 synthesized path로 관측한다. Correction focused test는 String element PASS, Number element drift FAIL, empty Array가 required element probe evidence를 제공하지 못함을 직접 고정한다.

기존 Ammo production parser/fingerprint regression에는 이미 FName semantic duplicate reject와 physical order/case 변화에 대한 semantic fingerprint invariance가 존재하므로 새 correction exact2와 기존 DAO exact7을 합친 DAO exact9가 계약을 함께 검증한다.

판정: **P1 Closed**.

#### P1-3 Re-review — AmmoIcon SoftObject target-class Reflection

`BuildDirectReflectedSourceShapeDescriptor(*UCFAmmoData::StaticClass())`가 direct authored property exact8을 관측하며 `AmmoIcon`을 다음처럼 확인한다.

```text
PropertyKind: SoftObject
ReflectedTypePath: /Script/Engine.Texture2D
ContainerKind: Scalar
```

`AmmoTags`도 `Array` / `Array<Name>` / `Array`로 관측된다. 기존 Missile authored Source에는 해당 scalar SoftObject가 없고 기존 DACE15가 final Source에서도 15/15 PASS하여 accepted Missile descriptor/signature behavior가 회귀하지 않았다.

판정: **P1 Closed**.

#### P1-4 Re-review — per-TypeKey canonical Staging / Ammo exact0

canonical Staging target set은 provider descriptor가 explicit 소유한다. Missile은 기존 exact3를 그대로 소유하고 Ammo는 declared empty exact0을 소유한다. `ValidateCanonicalStagingCompatibilityForProvider(AmmoProvider)`는 exact0을 valid compatibility state로 처리하지만 `AmmoProvider.DaceReadiness == ContractNotReady`이므로 provider-aware migration gate는 accepted append와 Current promotion을 모두 차단한다.

따라서 exact0은 "현재 존재하는 Product canonical target이 0개"라는 의미만 가지며 Ammo DACE acceptance/bootstrap 완료를 의미하지 않는다. fake Product Ammo 생성, HeavyFinite/RocketFinite 재분류 또는 저장은 수행하지 않았다.

판정: **P1 Closed**.

#### Final fresh validation

```text
Official UE 5.8 Editor Build
- Job: 5c2300543af24ce78b1eddd4d409d43e
- Exit: 0
- Result: PASS

DAO focused exact9
- Process: 503460d34b5944fda2efccda396d5d76
- Success: 9/9
- Failure/Missing/Unexpected/Duplicate: 0
- Result: PASS

Existing Missile DACE exact15
- Process: e91c4f1d19ca401495e138ca35497474
- Success: 15/15
- Failure/Missing/Unexpected/Duplicate: 0
- Result: PASS

Disposable Ammo persisted residue
- /Game/Test/CarFight/DAOAmmoP03
- CFAmmoData asset_count: 0

Protected exact9 scoped Git diff
- Missile Product canonical JSON exact3: 0
- Missile Product uasset exact3: 0
- accepted CFDAContractBase.cpp: 0
- HeavyFinite/RocketFinite exact2: 0
```

이번 correction에서 CF-FQ-049 affected13은 재실행하지 않았다. 직접 변경된 범위는 TypeDispatch/DACE provider boundary와 DACE observation/gate이며, final DAO exact9와 DACE15가 해당 executable regression을 직접 덮는다. 직전 affected13 PASS는 historical evidence로 유지하되 이번 correction의 fresh evidence로 확대하지 않는다.

#### Correction + Re-review 판정

```text
P0: 0
blocking P1: 0
P2: 0
Result: Technical PASS
DAO-P0-04 Contract Correction: Accepted
Ammo DACE descriptor/probe/accepted bootstrap: NOT STARTED
Ammo DACE implementation gate: READY
Exact next: DAO-P0-04 Ammo DACE Descriptor / Probe / Bootstrap Implementation
```

이번 PASS는 **Ammo DACE 구현 완료가 아니라 구현 전 shared contract correction 완료**를 뜻한다. 실제 Ammo SourceShape/AdapterShape/Mapping/Semantic descriptor, production probes와 `DACE-AmmoData` bootstrap accepted history는 다음 Gate가 소유한다.

### 10.15 2026-09-10 DAO-P0-04 Ammo DACE Descriptor / Probe / Bootstrap Implementation

v0.3.13 Contract Correction + Re-review Technical PASS를 baseline으로 실제 AmmoData independent DACE provider-local contract를 구현했다. 이번 단계는 구현 + fresh validation checkpoint이며 Post-Implementation Mid-review 전 Technical Acceptance로 확대하지 않는다.

#### 구현 범위

```text
CFDAAmmoDace.h/.cpp v1.0.0
- AmmoData SourceShape exact8
- Ammo AdapterShape exact19
- SourceAdapterMapping exact16
- SemanticContract exact14
- current four-signature build
- provider-bound accepted history/revision validation
- provider-bound canonical exact0 migration gate
- independent DACE-AmmoData accepted bootstrap exact1

CFDAAmmoProvider v1.4.0 / header v1.2.0
- deterministic whole-record memory serializer 추가
- existing strict parser / fingerprint / extractor / materializer 재사용
- DaceContractOwnerName = CFDAAmmoDace
- DaceAcceptedHistoryNamespace = DACE-AmmoData
- DaceReadiness = ContractReady
- Product canonical target exact0 유지

CFDAContractGuard v1.5.0
- direct Reflection의 FIntProperty를 stable `Int` token으로 추가
- existing Missile authored Source에는 해당 int32 leaf가 없어 Missile accepted descriptor/signature rebaseline 0

CFDAAmmoDaceTests.cpp v1.0.0
- descriptor/bootstrap exact binding
- production serializer→parser→fingerprint→transient materialize→extract probe
- AmmoTags[] wrong type / semantic duplicate / order-case semantic equivalence
- AmmoIcon UTexture2D target-class drift / null+non-null / referenced asset no-load
- old SchemaRevision reject / cross-TypeKey accepted history contamination reject
- Product canonical exact0 + current migration gate
```

#### Ammo descriptor 핵심 계약

```text
SourceShape
AmmoTags = Array / Array<Name> / Array
AmmoIcon = SoftObject / /Script/Engine.Texture2D / Scalar
MaximumLoadableAmmoCount = Int / Scalar

AdapterShape
Payload.AmmoTags = Array / FNameTokenArray / PayloadContainer
Payload.AmmoTags[] = String / FNameToken / Payload
Payload.AmmoIcon = StringOrNull / SoftObjectPath / Payload

SourceAdapterMapping
AmmoTags -> Payload.AmmoTags[] / FNameToken
AmmoIcon -> Payload.AmmoIcon / SoftObjectPath

SemanticContract
AmmoTags = set-like FName semantic / duplicate reject / order-independent fingerprint / canonical sorted materialize
AmmoIcon = null or canonical top-level SoftObjectPath / UTexture2D-compatible Asset Registry validation / referenced asset state excluded from fingerprint
```

#### Bootstrap append gate

`DACE-AmmoData-S1-A1-Bootstrap`은 Missile `CFDAContractBase.cpp`와 분리된 `CFDAAmmoDace.cpp` owner에 candidate exact1로 구성했다. 구현 중 이 record는 다음 항목이 fresh binary에서 모두 닫히기 전에는 accepted 완료 상태로 취급하지 않았다.

```text
1. Native CFAmmoData direct Reflection == SourceShape exact8
2. Adapter serializer physical observation == AdapterShape exact19
3. SourceAdapterMapping coverage PASS
4. Production serializer→parser→fingerprint→materialize→extract semantic round-trip PASS
5. AmmoTags[] non-empty positive + wrong type/duplicate/permutation regression PASS
6. AmmoIcon UTexture2D target-class + null/non-null + no-load regression PASS
7. independent accepted chain identity/signature exact PASS
8. current revision 1/1 == bootstrap revision 1/1
9. Product canonical Ammo exact0 compatibility PASS
10. current change declaration nullptr / duplicate accepted append blocked / migration pending 0
11. existing Missile DACE15 PASS / protected Missile accepted history diff 0
```

Fresh validation에서 위 Gate가 모두 PASS했으므로 final Source에서는 bootstrap exact1을 accepted baseline으로 유지한다. Initial bootstrap impact는 `NoMigration / NotRequired`이며 이후 current contract는 accepted baseline과 no-delta이므로 추가 append를 허용하지 않는다.

#### Fresh validation

```text
Official UE 5.8 Editor Build
Job: 14fc6c12e83a4d7a98154ad6eb59c6ea
Result: PASS / Exit 0
Fresh compile/link includes CFDAAmmoDace.cpp / CFDAAmmoDaceTests.cpp / CFDAAmmoProvider.cpp / CFDAContractGuard.cpp

DAO focused exact13
Process: 69c4d86b35d940c0bba8c51830181e91
Success: 13/13
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Result: PASS

Existing Missile DACE exact15
Process: 357d7ac193e84d0eba57213b220424b0
Success: 15/15
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Result: PASS
```

이번 구현은 shared Staging/Review/TOCTOU execution semantics를 변경하지 않았고 Product Ammo target은 exact0이므로 CF-FQ-049 affected13은 중복 재실행하지 않았다. 기존 affected `bcb9e15f9b8942d886768896a35b0bf2` 13/13은 Historical evidence로만 유지한다.

#### Protection audit

```text
Protected exact9 scoped worktree diff: 0
- Missile Product canonical JSON exact3: 0
- Missile Product uasset exact3: 0
- accepted CFDAContractBase.cpp: 0
- Ammo persisted HeavyFinite/RocketFinite exact2: 0

Product Low/Normal/High Apply·Save: 0
Product canonical Ammo asset/Staging creation: 0
HeavyFinite/RocketFinite save/reclassification: 0
Missile accepted history rewrite/rebaseline: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

#### 현재 판정

```text
DAO-P0-04 implementation: Complete
Ammo independent DACE descriptor/probe/history: Implemented
DACE-AmmoData bootstrap: exact1 accepted after fresh gate PASS
Ammo DACE readiness: ContractReady
Product canonical Ammo: exact0 유지
Fresh Build/DAO13/Missile DACE15: PASS
Post-Implementation Mid-review: Pending
DAO-P0-04 Technical Acceptance: 아직 미확정
DAO-P0-05: HOLD / Not Started
Exact next: DAO-P0-04 Post-Implementation Mid-review
```

### 10.16 2026-09-10 DAO-P0-04 Post-Implementation Mid-review

v0.3.14 implementation + fresh validation checkpoint를 baseline으로 current Source를 독립 재검수했다. 이번 단계는 read-only review이며 C++/script/JSON/uasset/accepted snapshot Source mutation은 수행하지 않았다.

#### 재확인된 정상 구현

```text
CFDAAmmoDace
- SourceShape exact8
- AdapterShape exact19
- SourceAdapterMapping exact16
- SemanticContract exact14
- DACE-AmmoData bootstrap exact1
- provider-bound accepted chain / revision / migration gate

AmmoTags[]
- Source: Array / Array<Name> / Array
- Adapter terminal: Payload.AmmoTags[] / String / FNameToken
- non-empty production serializer observation
- wrong element type reject
- FName semantic duplicate reject
- order/case semantic equivalence fingerprint 유지

AmmoIcon
- Source: SoftObject / /Script/Engine.Texture2D / Scalar
- Adapter: StringOrNull / SoftObjectPath
- null/non-null serializer/parser round-trip
- Asset Registry metadata-only Texture2D compatibility
- wrong class / missing reference reject
- referenced Texture load 0

per-TypeKey isolation
- Ammo owner: CFDAAmmoDace
- Ammo namespace: DACE-AmmoData
- Missile accepted chain을 Ammo provider에 전달하면 reject
- SchemaRevision/AdapterContractRevision mismatch reject

Product canonical Ammo
- target set declared
- exact0 valid empty set
- fake Product asset/Staging 생성 0
- HeavyFinite/RocketFinite Product 재분류 0
```

`CFDAAmmoDace::ValidateCurrentContract()`가 production behavior probe를 직접 호출하지 않는 점도 확인했으나, 기존 Missile DACE 역시 structural/revision aggregate와 focused production behavior Automation을 분리하고 있으므로 이 자체는 blocker로 판정하지 않는다.

#### P1-1 — production semantic fingerprint token coverage가 self-consistency에 의존

현재 `AmmoDaceProductionProbe`는 실제 `BuildSemanticFingerprint()`를 사용해 다음을 확인한다.

```text
source fingerprint
== serializer→parser 후 staging fingerprint
== materialize→extract 후 fingerprint
```

현재 `AppendPayloadTokens()` 구현은 schema/revision/class와 Ammo exact8 semantic token을 실제로 모두 포함하므로 **현재 값은 정상**이다.

문제는 미래 drift 검출이다. 예를 들어 `AppendPayloadTokens()`에서 `AmmoDisplayName`, `AmmoFamilyId`, `UnitMassKg`, `AmmoIcon` 또는 `bCanBeResupplied` token 하나를 실수로 제거해도 serializer/parser/extractor가 모두 동일한 `BuildSemanticFingerprint()`를 사용하기 때문에 양쪽 fingerprint가 함께 바뀌어 위 round-trip equality는 계속 PASS할 수 있다.

이는 Current DACE가 보장하는 `production semantic fingerprint token mismatch` fail-closed와 DAO-P0-04의 normative `Ammo fingerprint probe` 목적에 미달한다.

Correction 요구:

```text
- production fingerprint token label/coverage를 독립 관측하는 seam을 추가하거나
- 동등한 exhaustive per-field semantic mutation -> fingerprint delta 검증을 추가한다.
- schema/revisions/class + Ammo exact8 semantic inclusion을 모두 검증한다.
- AmmoTags Count/[]와 AmmoIcon null/path conditional token도 coverage에 포함한다.
- current correct fingerprint algorithm 자체를 임의 재설계하지 않는다.
```

판정: **blocking P1 1**.

#### P2-1 — accepted Ammo history의 accidental rebaseline 보호 경계가 약함

`DACE-AmmoData-S1-A1-Bootstrap`은 runtime chain/signature validation 자체는 정상이고 Missile history와도 논리적으로 분리돼 있다. 다만 current descriptor와 accepted history implementation이 모두 `CFDAAmmoDace.cpp`에 존재한다.

따라서 미래 contract 변경 시 descriptor signature와 기존 bootstrap record/hash를 같은 변경에서 함께 수정하면 current test의 `current == accepted` 및 snapshot chain signature 검증을 다시 만족시킬 수 있어, append-only 원칙을 실수로 위반하는 accidental rebaseline의 물리적 보호 경계가 기존 Missile `CFDAContractBase.cpp`보다 약하다.

Correction 권고:

```text
- 기존 bootstrap exact1의 값/identity/signature는 변경하지 않는다.
- GetAcceptedSnapshots() implementation만 dedicated append-only production source로 분리한다.
- 예: CFDAAmmoDaceBase.cpp (32자 이하)
- 기존 CFDAAmmoDace public/internal API signature는 유지한다.
- 분리된 accepted owner를 후속 protected audit set에 포함한다.
```

판정: **P2 1**.

#### Validation / protection handling

이번 Mid-review는 Source mutation 0이므로 직전 fresh executable evidence를 중복 실행하지 않았다.

```text
Retained implementation evidence
- Build 14fc6c12e83a4d7a98154ad6eb59c6ea PASS
- DAO 69c4d86b35d940c0bba8c51830181e91 13/13 PASS
- Existing Missile DACE 357d7ac193e84d0eba57213b220424b0 15/15 PASS

Protected exact9 scoped worktree diff: 0
- Missile Product canonical JSON exact3
- Missile Product uasset exact3
- accepted CFDAContractBase.cpp
- Ammo persisted HeavyFinite/RocketFinite exact2

Build/Automation re-run: 0 — Source mutation 0
Product Apply/Save: 0
DAO-P0-05 implementation: 0
```

#### Mid-review 판정

```text
P0: 0
blocking P1: 1
P2: 1
Result: HOLD
DAO-P0-04 implementation: retained
DAO-P0-04 fresh validation: retained PASS
DAO-P0-04 Technical Acceptance: HOLD
DAO-P0-05: HOLD / NOT STARTED
Exact next: DAO-P0-04 Correction + Re-review
```

### 10.17 2026-09-10 DAO-P0-04 Correction + Re-review

v0.3.15 Mid-review HOLD의 blocking P1 1건/P2 1건을 current Source에 최소 교정하고, fresh Build/Automation 뒤 independent Source re-review를 다시 수행했다.

#### P1-1 correction — production fingerprint token coverage 독립 관측

새 provider API나 별도 fingerprint 구현을 추가하지 않고 existing shared `CFDACommonPrimitives::FScopedSemanticTokenProbe`를 재사용했다. 이 probe는 production `AppendStringToken` / `AppendFloatToken` / `AppendBoolToken`이 공통 `AppendRawToken`으로 들어갈 때 실제 emitted label을 thread-local sink에 기록한다.

`CFDAAmmoDaceTests.cpp v1.1.0`은 production Source에서 생성하지 않은 test-owned literal expected manifest를 별도 소유한다.

```text
Required common structural tokens
- SchemaId
- SchemaRevision
- AdapterContractRevision
- DataAssetTypeClassPath

Required Ammo exact8 semantic tokens
- Payload.AmmoId
- Payload.AmmoDisplayName
- Payload.AmmoFamilyId
- Payload.UnitMassKg
- Payload.AmmoTags.Count
- Payload.AmmoTags[] x fixture element count
- Payload.AmmoIcon.IsNull
- Payload.AmmoIcon.Path — non-null일 때만
- Payload.MaximumLoadableAmmoCount
- Payload.bCanBeResupplied
```

현재 non-null AmmoIcon + AmmoTags exact2 production fixture는 exact15 label, null AmmoIcon fixture는 `Payload.AmmoIcon.Path`가 빠진 exact14 label을 요구한다. count/order/duplicate occurrence를 모두 exact 비교하므로 future `AppendPayloadTokens()`에서 required token 하나가 빠지거나 조건부 path emission이 틀어지면 기존 serializer/parser/materializer가 같은 fingerprint 함수를 공유하더라도 test-owned manifest와 불일치하여 fail-closed한다.

기존 serializer→parser fingerprint equality, materialize→extract fingerprint equality, AmmoTags physical/semantic negative regression과 AmmoIcon no-load regression은 그대로 유지했다.

#### P2-1 correction — accepted Ammo history dedicated append-only owner

신규 production source:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAAmmoDaceBase.cpp v1.0.0
```

`FCFDAAmmoDace::GetAcceptedSnapshots()` implementation만 이 파일로 이동했다. 기존 `DACE-AmmoData-S1-A1-Bootstrap` exact1의 모든 값은 변경하지 않았다.

```text
SnapshotId: DACE-AmmoData-S1-A1-Bootstrap
PreviousSnapshotSignature: empty
SchemaId: CarFight.DataAsset.AmmoData
SchemaRevision: 1
AdapterContractRevision: 1
ClassPath: /Script/CarFight_Re.CFAmmoData
Source signature: sha256:cd2aab5d0899a7c8c6def7e46bc90c69041801012b4fe874b25d2dd762d92339
Adapter signature: sha256:a0541028ddf1dd8b760e440f9af80456e7d1d8ae6d133c2727f67b9052d34073
Mapping signature: sha256:9199d6eaaa37fb5a5493f85d230ce3cfff56543928d9b118902d188615097b4b
Semantic signature: sha256:22cab6b7fc8951cd9d556fdfd32781570b17fb5c40f864a4df331a668e48ceff
Migration: NoMigration / NotRequired / evidence empty
Snapshot signature: sha256:b6de5965fefe3ddf25c2318e9e426bb4f67072f3e5d8da4278cfcab15973cdd3
```

mutable SourceShape/AdapterShape/Mapping/Semantic/migration authority는 `CFDAAmmoDace.cpp v1.1.0`에 남고 accepted history initializer는 이 파일에서 제거했다. Public/internal `FCFDAAmmoDace` API signature는 바꾸지 않았다. Focused descriptor test도 bootstrap exact1의 모든 field/signature를 literal expected 값으로 다시 고정한다.

이 새 Base 파일은 이번 교정에서 신규 생성됐기 때문에 HEAD 대비 `diff 0`이라고 표현하지 않는다. fixed readback으로 bootstrap exact1의 원래 값 유지가 확인됐고, 이후 DACE 변경부터 기존 protected exact9에 이 파일을 추가한 **protected exact10**을 사용한다.

#### Fresh validation

```text
Official UE 5.8 Editor Build
- job: d93c392b125d4b338060307f7d22a65e
- Result: PASS / Exit 0
- CFDAAmmoDace.cpp / CFDAAmmoDaceBase.cpp / CFDAAmmoDaceTests.cpp compile+link 확인

DAO focused exact13
- process: 577467e756184582ae84ef5f6c9f4202
- Result: 13/13 PASS
- failure/missing/unexpected/duplicate: 0
- 강화된 AmmoDaceProductionProbe PASS
- immutable Ammo bootstrap descriptor test PASS

Existing Missile DACE exact15
- process: fc7b719448f1454eab99dbda0e405b43
- Result: 15/15 PASS
- failure/missing/unexpected/duplicate: 0
```

Affected CF-FQ-049 exact13은 이번 correction이 Ammo DACE test/history owner separation과 developer-only token coverage observation만 변경하고 shared Staging/Review/Apply/TOCTOU execution semantics를 변경하지 않았으므로 중복 실행하지 않는다.

#### Protection audit

```text
Legacy protected exact9 scoped worktree diff: 0
- Missile Product canonical JSON exact3: 0
- Missile Product uasset exact3: 0
- protected CFDAContractBase.cpp: 0
- Ammo HeavyFinite/RocketFinite exact2: 0

DACE-AmmoData bootstrap value/signature rebaseline: 0
Product canonical Ammo: exact0 유지
Product Low/Normal/High Apply·Save: 0
HeavyFinite/RocketFinite save/reclassification: 0
Missile accepted history mutation: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
DAO-P0-05 implementation: 0
```

#### Independent re-review

```text
P0: 0
blocking P1: 0
P2: 0
Result: Technical PASS

P1 closure:
- production emitted token labels vs independent test-owned expected manifest exact 비교
- schema/revisions/class + Ammo exact8 coverage 확보
- AmmoTags Count/[] 및 AmmoIcon null/path conditional coverage 확보

P2 closure:
- accepted exact1 dedicated CFDAAmmoDaceBase.cpp append-only owner 분리
- bootstrap exact1 fixed literal regression + readback
- mutable current descriptor file와 accepted initializer 물리 분리

DAO-P0-04 Technical Acceptance: PASS
DAO-P0-05: Ready / Not Started
Exact next: DAO-P0-05 — Canonical Ammo Staging Pilot / Multi-Type Integration
```

### 10.18 2026-09-10 DAO-P0-05 Pre-Implementation Contract Review

v0.3.16 / DataAssetAuthoring.md v1.3.10의 DAO-P0-04 Technical PASS를 baseline으로 DAO-P0-05 구현 전 current Source를 read-only로 재감사했다. 이번 단계에서는 C++/script/Staging/JSON/uasset/accepted history 구현 mutation을 수행하지 않았다.

#### 재확인된 multi-type 기반

```text
Production registry
- MissileGuidePreset ReviewedMutationReady
- AmmoData ReviewedMutationReady
- exact TypeKey = SchemaId + DataAssetTypeClassPath

Shared common core
- provider-owned StagingRoot/revision contract validation
- class-scoped StableLogicalId duplicate namespace
- global TargetObjectPath duplicate namespace
- mixed common BatchPlanHash
- mixed common Reviewed approval
- exact provider dispatch
- global preflight + per-target immediate TOCTOU

DACE isolation
- Missile namespace: DACE-MissileGuidePreset
- Ammo namespace: DACE-AmmoData
- selected provider exact TypeKey/revision/history binding
- cross-TypeKey accepted history contamination reject

Product canonical Ammo
- DACE canonical target set explicitly declared
- exact0 valid empty set
- canonical compatibility loop exact0 => valid mutation0 baseline
```

Existing `CFDAStagingTests`에는 Missile common row와 test-only second provider common row를 결합한 mixed regression이 이미 있으며 다른 class에서 같은 textual StableLogicalId를 허용하고, cross-type 동일 TargetObjectPath는 global duplicate로 차단하며, mixed BatchPlanHash/Review가 physical order와 무관하게 deterministic함을 증명한다. Actual Ammo provider는 별도 focused tests에서 common Preview/Review/shared Apply/stale TOCTOU가 이미 증명되어 있다.

그러나 P0-05의 목표는 **실제 production-registered MissileGuidePreset + AmmoData가 operational discovery/selection 단계부터 같은 batch로 들어오는 것**이므로 아래 계약 공백을 구현 전에 닫아야 한다.

#### P1-1 — Product Ammo exact0 mutation0 acceptance와 Reviewed approval 의미가 충돌

Current DACE의 `ValidateCanonicalStagingCompatibilityForProvider()`는 provider가 explicit 선언한 empty canonical set을 valid exact0으로 처리한다. 따라서 Product canonical Ammo exact0 자체는 정상이고 Product asset/Staging을 새로 만들 필요가 없다.

반면 shared `BuildCommonBatchPlanHash()`와 `BuildCommonReviewedApproval()`은 의도적으로 **Create/Update mutation candidate가 하나 이상** 있어야 한다. pure empty/NoChange set에는 approval hash도 Reviewed approval도 만들지 않는다.

따라서 P0-05의 다음 두 요구를 하나의 Product flow로 구현하면 안 된다.

```text
A. Product acceptance lane
- Ammo canonical exact0 유지
- fake Product Ammo/Staging 생성 0
- Product Ammo Review/Apply 요구 0
- Product Low/Normal/High Apply·Save 0
- exact0 provider가 mixed scope에 포함되어도 zero-row provider result를 정상 coexistence로 판정

B. Review/TOCTOU integration lane
- actual registered MissileGuidePreset + AmmoData provider를 사용
- disposable test-owned Staging/target만 사용
- Create/Update mutation candidate가 존재하는 fixture에서 mixed Preview → Review → stale/TOCTOU를 검증
- HeavyFinite/RocketFinite와 Product Low/Normal/High는 fixture로 사용하지 않음
- cleanup/residue0 필수
```

No-op approval을 허용하도록 shared Review 계약을 약화하거나 acceptance를 만들기 위해 fake Product Ammo를 생성하면 기존 P0 계약을 깨뜨린다.

판정: **blocking P1 1**.

#### P1-2 — production operational discovery/selection/session이 아직 Missile-only

Shared `CFDATypeDispatch`와 Apply core는 heterogeneous `FCFDACommonPreviewRow`를 처리하지만 current operational entry는 그렇지 않다.

```text
CFDAStagingOps.cpp
- GetMissileStagingRoot()
- ResolveStagingAbsolutePath() → Missile provider 고정
- NormalizeSelectedPaths() → Missile root 기준
- BuildConsoleSelection() → Missile root 기준
- GetProductTargets() → Low/Normal/High exact3
- SyncProductStaging() → Missile exact3
- DiscoverMissilePresetPreview() → Missile typed discovery
- FCFDAStagingOpsSession::Preview() → DiscoverMissilePresetPreview()
```

따라서 actual Ammo path는 현재 production Ops session의 selection/discovery 단계에 들어갈 수 없고, existing mixed test의 synthetic second provider만으로 DAO-P0-05 operational coexistence를 승인할 수 없다.

Correction contract:

```text
- shared Preview/duplicate/hash/Review/Apply core를 다시 만들지 않는다.
- provider-neutral Editor Private mixed discovery/selection seam을 추가한다.
- exact selected relative path 또는 explicit selected provider roots에서 시작한다.
- selected path는 trusted production registry의 provider-owned canonical StagingRoot containment로 정확히 한 provider에 귀속돼야 한다.
- owner provider 0개 또는 복수면 fail-closed한다.
- path owner를 확정하기 전에 JSON payload의 SchemaId/Class를 신뢰해 provider를 선택하지 않는다.
- 부모 `Authoring/DataAssetStaging` 전체를 무차별 recursive scan한 뒤 payload가 provider를 고르게 하지 않는다.
- provider 확정 뒤 해당 provider ParseCommonCandidate → ResolveCommonCurrentState → BuildCommonPreview를 호출한다.
- 모든 provider row 생성 뒤 global duplicate validation과 common hash/Review를 한 번 수행한다.
- explicit scope에 Ammo provider가 포함됐지만 canonical root가 absent/empty이면 exact0 zero-row result로 정상 처리한다.
- existing Public Missile Preview/console/SyncProduct facade signature와 exact3 behavior는 보존한다.
- Ammo exact0을 이유로 `SyncProductStaging`에 HeavyFinite/RocketFinite 또는 fake Product target을 연결하지 않는다.
```

판정: **blocking P1 1**.

#### P2-1 — mixed Apply는 global atomic transaction이 아님

Current Apply core는 approval 전체를 먼저 global preflight하지만 durable write는 target별 순차 수행한다. 앞 target의 durable write 후 뒤 target이 실패하면 aggregate 결과는 `PartialApplied`가 될 수 있다.

따라서 DAO-P0-05에서 `multi-type integration`을 “Missile+Ammo batch 전체가 하나의 원자적 transaction”으로 표현하면 실제 Source 계약과 달라진다.

Correction 요구:

```text
- cross-target all-or-nothing atomicity는 P0-05 non-goal로 명시한다.
- 보장 대상은 exact mixed Review evidence binding, global preflight, per-target immediate TOCTOU, exact provider dispatch다.
- mixed mutation Apply regression을 수행한다면 성공 경로와 PartialApplied taxonomy를 현재 contract대로 판정한다.
- Product mutation0 acceptance에서는 mixed Product Apply 자체를 실행하지 않는다.
```

판정: **P2 1**.

#### Pre-Implementation Review 판정

```text
P0: 0
blocking P1: 2
P2: 1
Result: HOLD
DAO-P0-05 implementation: NOT STARTED
Exact next: DAO-P0-05 Contract Correction + Re-review
```

이번 review는 Source mutation 0이므로 직전 DAO-P0-04 fresh Build/DAO13/Missile DACE15를 중복 실행하지 않는다. protected exact10의 legacy exact9 scoped Git diff 0을 재확인하고, 신규 append-only `CFDAAmmoDaceBase.cpp`는 current review에서 read-only로 유지한다. Product canonical Ammo exact0, Product Low/Normal/High exact3, HeavyFinite/RocketFinite exact2, CF-FQ-039 Active와 기존 병렬 dirty를 보존한다.

### 10.19 2026-09-10 DAO-P0-05 Contract Correction

v0.3.17의 blocking P1 2 / P2 1을 구현 전에 문서 계약으로 교정했다. Source는 수정하지 않았다.

```text
P1-1 closure
- Product canonical Ammo exact0 mutation0 lane과 Reviewed mutation integration lane을 완전히 분리
- exact0은 zero-row 정상 contribution이며 no-op approval이 아님
- actual Review/TOCTOU는 actual registered Missile+Ammo + disposable exact paths에서만 검증

P1-2 closure
- mixed operational discovery를 Canonical Product Set / Explicit Paths exact2 mode로 제한
- path owner는 JSON read 전 trusted production provider root containment로 exact1 결정
- owner 0 / owner >1 / allowed scope 밖 provider는 fail-closed
- `FCFDAStagingOpsSession`: empty selection은 existing Missile whole-root discovery 보존, non-empty selection만 provider-neutral Explicit Paths로 일반화, Review는 동일 mode/path scope fresh replay
- Console StableLogicalId shorthand는 기존 Missile canonical path 변환 의미 보존 / Ammo implicit shorthand 확장 금지
- ambiguous overlapping roots는 test-owned explicit provider-set seam으로 production registry mutation 없이 reject 증명
- raw registry exposure, payload-first TypeKey, parent-root recursive trust 금지
- existing Missile Public/console/SyncProduct exact3 compatibility 보존

P2-1 closure
- mixed Apply = global preflight + per-target immediate TOCTOU + sequential durable apply
- cross-target all-or-nothing atomicity는 명시적 non-goal
- PartialApplied 및 existing uncertainty precedence 유지
```

Implementation fixture는 actual provider exact2를 사용하되 Product/HeavyFinite/RocketFinite를 건드리지 않는 `__AutomationP05__` + `/Game/Test/CarFight/DAOP05/` disposable 범위로 동결했다. DACE history/revision은 P0-05에서 append/change하지 않는다.

현재 상태는 **Correction Complete / Independent Re-review Pending**이다. P0/P1 0 독립 재검수 전 C++/script/Staging/uasset 구현은 시작하지 않는다.

### 10.20 2026-09-10 DAO-P0-05 Independent Contract Re-review

v0.3.18 correction contract를 current `CFDATypeDispatch`, `CFDAStagingOps`, `CFDAStagingApply`, actual `CFDAMissileProvider`/`CFDAAmmoProvider`, Ammo DACE current gate와 기존 mixed common regression에 독립 재대조했다.

재검수 중 기존 Console shorthand/empty-selection compatibility가 모호할 수 있는 점을 발견해 같은 correction 안에서 추가 고정했다. 그 결과 기존 Missile UX/API 의미를 바꾸지 않고 non-empty canonical exact-path selection만 mixed-provider로 확장하는 구현 경계가 명확해졌다.

```text
P0: 0
blocking P1: 0
P2: 0
Result: Technical PASS

P1-1 closure accepted
- Ammo provider literal Source: CanonicalStagingRoot=Authoring/DataAssetStaging/AmmoData
- DACE canonical target set explicit declared + exact0
- Product exact0은 zero-row mutation0 coexistence lane
- Reviewed approval은 actual-provider disposable mutation lane
- no-op approval / fake Product Ammo·Staging 금지

P1-2 closure accepted
- common duplicate/hash/Review/Apply/TOCTOU core 재작성 없음
- non-empty session selection만 trusted provider-root exact path ownership으로 mixed generalization
- empty session selection + Console StableLogicalId shorthand는 existing Missile semantics 유지
- owner 0/>1/scope mismatch fail-closed
- payload-first routing / parent-root recursive trust / all-provider empty scan 금지
- positive actual-provider mixed Review→Apply는 Missile 1 + Ammo 1 disposable durable Applied까지 증명

P2 closure accepted
- mixed Apply의 global preflight + per-target immediate TOCTOU + sequential durable execution 의미가 current Source와 일치
- PartialApplied/uncertainty taxonomy 유지
- cross-target all-or-nothing atomicity는 명시적 non-goal

DACE / protection
- Missile/Ammo accepted history/revision 변경 요구 0
- Product canonical Ammo exact0 유지
- Product/HeavyFinite/RocketFinite fixture 사용 0
- protected exact10 변경 요구 0
```

이번 Correction + Re-review에서 executable Source mutation은 0이다. 따라서 DAO-P0-04의 final official Build `d93c392b125d4b338060307f7d22a65e`, DAO exact13 `577467e756184582ae84ef5f6c9f4202`, Missile DACE exact15 `fc7b719448f1454eab99dbda0e405b43`은 prior current executable baseline으로 유지하되 이번 문서 계약검수의 fresh 실행 증거로 재표현하지 않는다. 구현이 실제 발생한 뒤 P0-05 focused/affected/DACE/build를 fresh 실행한다.

DAO-P0-05는 이제 **Implementation Ready**다. 아직 implementation은 시작하지 않았다.

### 10.21 2026-09-10 DAO-P0-05 Implementation + Fresh Validation

v0.3.18 / DataAssetAuthoring.md v1.3.12의 Contract Correction + Independent Re-review Technical PASS를 baseline으로 Canonical Ammo Staging Pilot / Multi-Type Integration을 current Source에 구현하고 fresh executable regression과 보호 감사를 완료했다.

#### 구현 범위

```text
CFDATypeDispatch v1.6.0
- code-owned production registry를 외부에 노출하지 않는 FindProviderForStagingPath 추가
- JSON read 전에 full registered provider set의 CanonicalStagingRoot containment로 owner exact1 결정
- owner 0 / owner >1 / explicit allowed TypeKey scope mismatch fail-closed
- allowed scope를 먼저 필터링하지 않아 overlapping root ambiguity를 숨기지 않음
- production registry mutation 없는 explicit provider-set negative seam으로 overlap reject 증명

CFDAStagingOps.h v1.2.0 / CFDAStagingOps.cpp v1.4.0
- session empty selection = 기존 Missile whole-root discovery 그대로 유지
- non-empty Missile-only selection = 기존 typed Missile discovery 그대로 유지
- Ammo path가 포함된 non-empty exact selection만 provider-neutral mixed Explicit Paths mode 사용
- owner exact1 -> JSON read -> owner ParseCommonCandidate -> provider contract -> owner ResolveCommonCurrentState -> existing common Preview 순서
- 모든 mixed row 생성 후 existing global duplicate/hash/Reviewed approval core를 그대로 재사용
- Review는 동일 normalized path set + 동일 discovery mode를 fresh replay
- Console StableLogicalId shorthand는 existing Missile canonical path 의미 유지
- SyncProductStaging Product Low/Normal/High exact3 behavior 유지
- parent Authoring/DataAssetStaging recursive trust / payload-first TypeKey dispatch / raw registry exposure 없음

Mixed Apply
- existing global preflight + per-target immediate TOCTOU + sequential durable apply 유지
- PartialApplied / uncertainty taxonomy 유지
- cross-target all-or-nothing atomicity를 추가하지 않음

Product canonical Ammo
- explicit exact0 유지
- zero-row mutation0 acceptance 유지
- fake Product Ammo asset/Staging 0
- Product Ammo Review/Apply/Save 0
```

#### Actual-provider disposable integration

신규 `CFDAMixedOpsTests.cpp v1.0.2`와 `RunDAOP05Tests.ps1 v1.0.0`은 production registry의 실제 MissileGuidePreset + AmmoData provider exact2를 사용한다. Fixture는 provider-local `__AutomationP05__` Staging과 `/Game/Test/CarFight/DAOP05/` Content root에만 생성하고 종료 시 제거한다.

```text
PathOwnerContract
- actual Missile/Ammo root owner exact1
- allowed TypeKey scope mismatch reject
- unknown parent-root child reject
- overlapping trusted roots owner>1 reject
- Product canonical Ammo exact0 확인

MixedDurable
- Missile 1 + Ammo 1 actual provider Create
- 두 class가 같은 textual StableLogicalId 사용
- class-scoped identity이므로 false duplicate 0
- input path 순서가 달라도 BatchPlanHash 동일
- one mixed Review -> one ApplyReviewed -> DurableApplied exact2

MixedDuplicate
- 서로 다른 TypeKey가 같은 TargetObjectPath를 요구하면 global DuplicateTargetPath로 Review 전 차단
- 같은 textual StableLogicalId는 cross-class duplicate로 오인하지 않음

MixedStale
- Review 후 Missile source semantic drift -> global preflight mutation0 차단
- Review 후 loaded-only Ammo current truth drift -> global preflight mutation0 차단
```

초기 focused run에서 Missile Create fixture의 empty `BaseSemanticFingerprint` physical representation이 production serializer의 empty string과 parser의 Create `null` 계약 사이에서 충돌하는 것을 확인했다. Production parser/serializer를 완화하지 않고 P0-05 test-owned Create fixture에서 serializer 결과의 해당 field만 JSON `null`로 정규화했다.

Affected CF-FQ-049 첫 재실행에서는 `DAS_P0_02.StrictValidation`의 old test-only second-provider fixture가 v1.5+ DACE canonical target-set declaration을 채우지 않아 12/13이었다. Production registry/algorithm은 변경하지 않고 `CFDAStagingTests.cpp v1.8.0` test fixture에 `ContractNotReady + explicit canonical exact0 declaration`만 추가한 뒤 final Build/affected regression을 다시 fresh 실행했다.

#### Final fresh validation

```text
Official UE 5.8 Editor Build
- job: 39e4973ff0ad4e31803c679d272533ff
- Result: PASS / Exit 0

DAO-P0-05 focused actual-provider exact4
- process: 148c4f325b724b2fa7bfd7158819c531
- Result: 4/4 PASS
- failure / missing / unexpected / duplicate terminal: 0 / 0 / 0 / 0

Affected CF-FQ-049 exact13
- process: 0f2f69983f024986aa87a98afee2aed9
- Result: 13/13 PASS
- failure / missing / unexpected / duplicate terminal: 0 / 0 / 0 / 0
- DAS-P0-04 fixture residue count: 0

Existing Missile DACE exact15
- process: 8967abe39f1a4d128c3ce455ede970a5
- Result: 15/15 PASS
- failure / missing / unexpected / duplicate terminal: 0 / 0 / 0 / 0
```

#### Protection / residue audit

```text
Protected exact10 scoped worktree diff: 0
- Missile Product canonical JSON exact3: 0
- Missile Product uasset exact3: 0
- protected CFDAContractBase.cpp: 0
- Ammo HeavyFinite/RocketFinite exact2: 0
- dedicated append-only CFDAAmmoDaceBase.cpp: current fixed bootstrap readback unchanged

DACE-AmmoData-S1-A1-Bootstrap exact1
- revisions 1/1 unchanged
- Source/Adapter/Mapping/Semantic signatures unchanged
- final Snapshot signature unchanged
- NoMigration / NotRequired unchanged

Fresh AssetDump
- /Game/Test/CarFight/DAOP05 + CFAmmoData: exact0
- /Game + CFAmmoData: exact2 only
  - DA_Ammo_HeavyFinite
  - DA_Ammo_RocketFinite

P0-05 provider-local __AutomationP05__ physical residue: 0 by focused runner gate
Product canonical Ammo: exact0 유지
Product Low/Normal/High Apply·Save: 0
fake Product Ammo/Staging: 0
HeavyFinite/RocketFinite save/reclassification: 0
CF-FQ-039 Active: unchanged
기존 병렬 dirty clean/revert/stash/reset: 0
commit/push: 0
```

#### 현재 판정

```text
DAO-P0-05 implementation: Complete
Fresh executable validation: PASS
Actual-provider mixed Preview/Review/Apply: PASS
Product canonical Ammo exact0 zero-row acceptance: PASS
Protected exact10 / disposable residue audit: PASS
Post-Implementation Mid-review: Pending
DAO-P0-05 Technical Acceptance: Pending
DAO-P0-06: HOLD / Not Started
Exact next: DAO-P0-05 Post-Implementation Mid-review
```

이번 checkpoint는 구현 + fresh validation 완료다. 독립 Post-Implementation Mid-review 전에는 DAO-P0-05를 Technical Acceptance로 확대하거나 DAO-P0-06 Reuse Measurement / Current System Promotion을 시작하지 않는다.

### 10.22 2026-09-10 DAO-P0-05 Post-Implementation Mid-review

v0.3.19 / DataAssetAuthoring.md v1.3.13의 Implementation + Fresh Validation PASS를 baseline으로 current Source를 독립 read-only 재검수했다. 이번 Mid-review에서는 C++/script/JSON/Staging/uasset/DACE accepted history mutation을 수행하지 않았다.

```text
P0: 0
blocking P1: 0
P2: 0
Result: Technical PASS
DAO-P0-05 Technical Acceptance: PASS
DAO-P0-06: Ready / Not Started
```

#### exact Staging path owner-before-read

`CFDATypeDispatch::FindProviderForStagingPath()`는 private production provider registry 전체를 대상으로 각 `CanonicalStagingRoot` containment를 먼저 계산하고 owner exact1을 요구한다. Allowed TypeKey scope는 owner 결정 뒤 검사하므로 scope filtering으로 overlapping trusted root ambiguity를 숨길 수 없다. owner 0, owner >1, scope mismatch는 JSON read 전 fail-closed한다.

`CFDAStagingOps` mixed path는 owner 결정 뒤 provider root 기준 canonical path와 main_game containment를 재검증하고 physical file 존재를 확인한 다음에만 JSON을 읽는다. 따라서 parent `Authoring/DataAssetStaging` recursive trust나 payload-declared TypeKey를 먼저 읽어 provider를 선택하는 경로는 없다.

#### provider-neutral mixed Explicit Paths / Review replay

Mixed discovery는 owner provider의 `ParseCommonCandidate` / `ValidateProviderContract` / `ResolveCommonCurrentState`를 사용해 payload-free common row를 만든 뒤 existing common duplicate/hash/Reviewed approval core를 재사용한다. `Review()`는 Preview에서 저장한 normalized path exact set과 discovery mode를 그대로 사용해 fresh replay하고 fresh BatchPlanHash가 이전 Preview와 exact 일치해야 approval을 유지한다.

`GetMixedOperationalAllowedTypeKeys()`의 MissileGuidePreset+AmmoData exact2는 provider-neutral algorithm 내부의 payload 분기가 아니라 새 타입을 자동 승인하지 않는 code-owned admission policy다. third type onboarding 시 production provider 등록과 explicit admission 항목 추가는 필요하지만 common Preview/Review/TOCTOU/Apply algorithm 재작성이나 타입별 orchestration 복제를 요구하지 않는다. 따라서 현재 범위에서는 abstraction leak blocker로 판정하지 않는다.

Public `FCFDAStagingOpsPreview`/`FCFDAStagingPreviewRow`가 historical Missile DTO를 compatibility surface로 유지하는 점도 재검수했다. Mixed mode에서는 typed Missile Payload가 authority가 아니며 common envelope structural projection만 외부 표시용으로 사용하고 Reviewed approval은 내부 common row에서 직접 생성된다. 기존 Public signature를 깨지 않고 compatibility를 유지하기 위한 debt이며 P0-05의 correctness/reuse blocker는 아니다.

#### identity / duplicate / TOCTOU / Apply semantics

Class-scoped StableLogicalId duplicate key는 exact DataAsset class path + FName semantic으로 유지돼 다른 class가 같은 textual ID를 사용할 수 있다. TargetObjectPath duplicate는 TypeKey와 무관한 global blocker다. P0-05 actual-provider focused regression은 동일 textual ID의 Missile+Ammo exact2 durable apply와 cross-TypeKey 동일 TargetObjectPath reject를 직접 증명했다.

Apply는 approval 전체에 대한 global fresh preflight 뒤 각 target mutation 직전에 다시 exact provider lookup, Staging re-read, parse, current resolve, Preview/approval binding을 수행한다. source drift와 loaded-only current drift는 P0-05 focused regression에서 mutation0으로 차단됐다. 이후 execution은 deterministic sequential durable apply이며 기존 `PartialApplied`, `SaveStateUnconfirmed`, `InMemoryStateUnconfirmed` precedence를 유지한다. Existing affected13의 `DAS_P0_04.PartialApplied`가 첫 target durable 성공 + 두 번째 target pre-mutation block → `PartialApplied` / durable count1을 증명한다. Cross-target all-or-nothing atomicity는 계속 non-goal이다.

#### Missile compatibility

```text
Preview(empty)
= existing Missile whole-root discovery

Preview(non-empty Missile-only)
= existing typed Missile exact selection

Preview(non-empty, Ammo 포함)
= provider-neutral mixed Explicit Paths

Console StableLogicalId shorthand
= existing Missile canonical path 의미 유지

SyncProductStaging
= existing Missile Product Low/Normal/High exact3 only
```

기존 Public Ops/Apply signatures를 변경하거나 Ammo shorthand, mixed empty-selection all-provider scan, fake Product Ammo Staging을 추가하지 않았다.

#### evidence / protection handling

Mid-review에서 executable Source mutation은 0이므로 아래 final implementation evidence를 중복 실행하지 않았다.

```text
Official UE 5.8 Build
39e4973ff0ad4e31803c679d272533ff
PASS

DAO-P0-05 focused actual-provider exact4
148c4f325b724b2fa7bfd7158819c531
4/4 PASS

Affected CF-FQ-049 exact13
0f2f69983f024986aa87a98afee2aed9
13/13 PASS

Existing Missile DACE exact15
8967abe39f1a4d128c3ce455ede970a5
15/15 PASS
```

Focused exact4에 missing explicit physical file 전용 case는 별도 terminal test로 존재하지 않지만 production mixed discovery에 `FileExists` pre-read fail-closed가 직접 존재하며, payload-declared wrong TypeKey는 provider negative regression에서 `SchemaUnsupported`로 검증된다. 이 두 항목은 새로운 algorithm branch나 unchecked mutation path가 아니므로 추가 P2 evidence blocker로 분류하지 않는다.

최종 보호 재감사에서 protected exact10 scoped worktree diff는 0이다. Fresh AssetDump의 `/Game` `CFAmmoData`는 HeavyFinite/RocketFinite exact2만 존재한다. Product canonical Ammo exact0, Missile Product exact3, `CFDAContractBase.cpp`, append-only `CFDAAmmoDaceBase.cpp`, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다.

#### Mid-review 결론

DAO-P0-05 implementation은 frozen contract와 일치하며 independent Mid-review에서 추가 P0/P1/P2를 발견하지 않았다. 따라서 **DAO-P0-05 Technical Acceptance = PASS**로 닫는다. 다음 exact Gate는 `DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion`이며 이번 Mid-review에서는 DAO-P0-06 측정/구현/Promotion을 시작하지 않는다.

### 10.23 2026-09-10 DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion

v0.3.20 / DataAssetAuthoring.md v1.3.14의 DAO-P0-05 Technical PASS를 baseline으로 current Source에서 second onboarding의 실제 재사용 구조를 독립 측정했다.

사전 Acceptance/Measurement 판정은 다음과 같다.

```text
P0: 0
blocking P1: 0
P2: 1
Result: PASS
Promotion: Allowed
```

P2 1건은 generic implementation 일부가 legacy Missile compatibility 파일과 물리적으로 혼재하는 유지보수 부채다. correctness, safety 또는 third-type onboarding을 막지 않으며 현재 별도 scaffold/core 리팩터링을 선행하지 않는다.

#### Reuse measurement

현재 물리 Source line은 개발 시간 추정치가 아니라 구조적 반복량 지표로만 사용했다. legacy compatibility와 generic implementation이 같은 파일은 dedicated shared foundation과 분리했다.

| 측정 항목 | 결과 | 판정 |
| --- | ---: | --- |
| dedicated shared foundation | exact6 / 2300 physical lines | 재사용 Current core |
| hybrid shared + Missile compatibility | exact7 / 6363 physical lines | generic/legacy 혼재, pure shared LOC 아님 |
| framework-impact production surface | exact13 files | dedicated6 + hybrid7 |
| Ammo-specific production | exact5 / 1789 physical lines | typed extension 비용 |
| Missile parity/compatibility reviewed surface | exact9 files | hybrid7 + Missile provider exact2, 다른 항목과 overlap |
| CF-FQ-051 feature-owned Automation | exact17 | second onboarding focused coverage |
| prohibited algorithm duplication | 0 | PASS |
| third-type shared core algorithm rewrite | 0 required | PASS |

Dedicated shared exact6은 `CFDACommonPrimitives.*`, `CFDATypeDispatch.*`, `CFDADurableCore.*`다. Hybrid exact7은 `CFDAContractGuard.*`, `CFDAStagingOps.cpp`, `CFDAStagingApply.cpp`, Public Ops/Apply headers와 `CFDAStaging.cpp`이며 compatibility와 generic implementation이 함께 있으므로 6363줄 전체를 shared 신규 구현량으로 계산하지 않는다. Ammo-specific exact5는 `CFDAAmmoProvider.*`, `CFDAAmmoDace.h/.cpp`, `CFDAAmmoDaceBase.cpp`다. 각 분류는 역할상 겹치므로 exact13+exact5+exact9처럼 합산하지 않는다.

CF-FQ-051 feature-owned Automation exact17은 P0-02 4, P0-03 3, P0-04 correction 2, Ammo DACE 4, P0-05 mixed operational 4다. 기존 Missile/CF-FQ-049/050 regression은 이 신규 test count에 포함하지 않는다.

#### prohibited algorithm duplication audit

Ammo onboarding에서 아래 공용 알고리즘을 새 타입 전용으로 복제한 Source는 확인되지 않았다.

```text
Preview state machine: shared
Reviewed approval lifecycle: shared
source/current TOCTOU: shared
BatchPlanHash common envelope: shared
transaction result aggregation: shared
rollback/uncertainty state machine: shared
durable Create/Update/Save/reload/readback: CFDADurableCore shared
DACE signatures/reflection/accepted-chain integrity: FCFDAContractGuard shared
revision/canonical Staging/migration gate: FCFDAContractGuard shared
```

Ammo provider는 `CFDADurableCore::ApplyTypedTarget<UCFAmmoData, FCFDAAmmoPayload>()`를 사용하며 provider 내부에 별도 SavePackage transaction algorithm을 만들지 않는다. Ammo DACE도 descriptor/current wrapper는 타입 전용이지만 signature, Reflection coverage, accepted history chain, revision guard, canonical Staging compatibility와 migration gate는 `FCFDAContractGuard` 공용 알고리즘을 사용한다.

#### third-type reuse 판정

세 번째 DataAsset 타입은 다음 타입 전용 요소가 필요하다.

```text
payload / field schema
parse / serialize
extract / materialize
field semantics
StableLogicalId/current-state resolver
DACE descriptor/probe
독립 append-only accepted history
```

공용 영역에서 필요한 변경은 **production provider registration + explicit operational admission**이다. `GetMixedOperationalAllowedTypeKeys()` 목록 수정은 타입별 Preview/Apply 알고리즘 hard-code가 아니라 신규 provider가 자동 production mutation lane에 진입하지 못하도록 하는 admission policy다.

따라서 third type 추가 시 Preview/Review/TOCTOU/Apply/Durable/DACE core rewrite는 요구하지 않는다. DamageData는 third onboarding 후보로 사용할 수 있지만 자동 착수하지 않고 별도 Feature/lifecycle을 연다.

#### final acceptance evidence handling

DAO-P0-06은 executable Source/Asset/Staging을 변경하지 않는 read-only measurement + Current projection 단계다. Plan의 repeat-minimization 규칙에 따라 직전 accepted executable evidence를 중복 실행하지 않았다.

```text
Official UE 5.8 Build
39e4973ff0ad4e31803c679d272533ff
PASS retained

DAO-P0-05 focused actual-provider
148c4f325b724b2fa7bfd7158819c531
4/4 PASS retained

Affected CF-FQ-049
0f2f69983f024986aa87a98afee2aed9
13/13 PASS retained

Existing Missile DACE
8967abe39f1a4d128c3ce455ede970a5
15/15 PASS retained
```

Existing CF-FQ-049 OperationalEntry 1/1과 CF-FQ-045 Ammo typed identity/health regression은 각 Current owner의 accepted baseline으로 유지한다. Current Source의 `UCFAmmoData::IsAmmoDataValid()`는 `AmmoId != NAME_None` identity health 의미를 유지하며 authoring exact8 whole-record validity를 Runtime health로 확장하지 않았다. P0-06에서 Runtime/Ammo gameplay Source mutation이 없으므로 별도 runtime/fitting regression은 요구하지 않는다.

#### Promotion / closure

```text
DAO-P0-06 Reuse Measurement: PASS
DAO-P0-06 Acceptance: PASS
DAO-P0-06 Current System Promotion: Complete
CF-FQ-051: Technical Complete / Done
Current owner: DataAssetAuthoring.md v1.4.1
DAO-P0-06 Final Audit Correction + Re-review: PASS / P0 0 / blocking P1 0 / P2 1 non-blocking
DamageData third onboarding: Candidate / Not Started
```

Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, protected exact10, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다. 대표 Plan은 **Historical + Retained Path / G5 Deferred**로 내리고 현재 착수 route에서 제거한다.

---

## 11. 완료 조건

```text
[x] DAO-P0-00 Architecture / Contract Freeze + Design Audit PASS
[x] shared core vs typed provider boundary 구현 — DAO-P0-01 Correction + Re-review P0 0 / blocking P1 0 / P2 0 Technical PASS
[x] existing Missile behavior/signature/regression 보존 — final Build `2f8afa3de9144c55b74083ca4eb02aeb` PASS + affected `ad820e42c46545c182b319c9d872b724` 13/13 + DACE `9857fc2e265a419bb7d75c8df7f9e454` 15/15
[x] Ammo typed schema/parse/fingerprint/preview Technical PASS — DAO-P0-02 Correction + Re-review P0 0 / blocking P1 0 / P2 0 / Build + DAO4 + affected13 + DACE15 PASS
[x] Ammo array + soft reference durable semantic round-trip PASS — DAO-P0-03 Correction + Re-review Technical PASS
[x] Ammo typed durable Create/Update fixture PASS — disposable root / DAO-P0-03 Correction + Re-review Technical PASS
[x] DAO-P0-04 pre-bootstrap per-TypeKey DACE Contract Correction + Re-review PASS — Build + DAO9 + DACE15
[x] Ammo DACE independent contract/history implementation + Correction + Re-review Technical PASS — `DACE-AmmoData` bootstrap exact1 immutable / fingerprint independent token coverage / P0 0 / P1 0 / P2 0
[x] Ammo Staging target classification/integration Technical PASS — Product canonical exact0 mutation0 + actual-provider mixed exact4 / affected13 / DACE15 + Post-Implementation Mid-review P0 0 / P1 0 / P2 0
[x] multi-type cross-contamination 0 — P0-05 actual-provider class-scoped identity/global target duplicate/source-current TOCTOU + independent Source re-review PASS
[x] reuse measurement 완료 — dedicated shared exact6/2300 lines + hybrid exact7/6363 lines + Ammo-specific exact5/1789 lines / prohibited duplication 0
[x] DamageData third onboarding 전 core 재작성 필요 여부 판정 — shared core rewrite 0 required / provider registration + explicit admission / DamageData Candidate
[x] Official UE 5.8 Build PASS — DAO-P0-04 Contract Correction final `5c2300543af24ce78b1eddd4d409d43e`
[x] focused DACE-boundary regression PASS — DAO `503460d34b5944fda2efccda396d5d76` 9/9 + DACE `e91c4f1d19ca401495e138ca35497474` 15/15
[x] prior affected CF-FQ-049 evidence retained as Historical — `bcb9e15f9b8942d886768896a35b0bf2` 13/13; not fresh-rerun for P0-04 correction
[x] Current Systems DAO-P0-04 final correction projection synchronized — DataAssetAuthoring.md v1.3.10
[x] DAO-P0-05 Contract Correction + Independent Re-review Technical PASS — P0 0 / blocking P1 0 / P2 0
[x] DAO-P0-05 Implementation + Fresh Validation PASS — Build `39e4973ff0ad4e31803c679d272533ff` + focused `148c4f325b724b2fa7bfd7158819c531` 4/4 + affected `0f2f69983f024986aa87a98afee2aed9` 13/13 + DACE `8967abe39f1a4d128c3ce455ede970a5` 15/15
[x] DAO-P0-05 Post-Implementation Mid-review + Technical Acceptance PASS — P0 0 / blocking P1 0 / P2 0 / Source mutation0 / protected exact10 diff0
[x] DAO-P0-06 Final Audit Correction + Re-review PASS — stale Ammo unsupported/current header projection 교정 / P0 0 / blocking P1 0 / P2 1 non-blocking / Source·Asset·Test mutation0
```

---

## 12. 현재 정식 승격 상태

```text
CF-FQ-051: Done / Technical Complete
Priority: P2
Representative Historical Plan: DataAssetOnboarding/DataAssetOnboardingPlan.md v0.3.22 / Historical + Retained Path / G5 Deferred
UE Source Audit: Complete
Persisted CFAmmoData Audit: Test-owned exact2 / referencer0 / Product canonical exact0
DAO-P0-00 Correction + Re-review: P0 0 / blocking P1 0 / P2 0 / PASS
Contract Freeze: Complete
DAO-P0-01 Correction + Re-review: P0 0 / blocking P1 0 / P2 0 / Technical PASS
Shared multi-type foundation: Accepted
Production registered provider: MissileGuidePreset ReviewedMutationReady + AmmoData ReviewedMutationReady exact2
DAO-P0-02 Correction + Re-review: P0 0 / blocking P1 0 / P2 0 / Technical PASS
DAO-P0-03 implementation: Complete
DAO-P0-03 Post-Implementation Mid-review: Historical HOLD / P0 0 / blocking P1 1 / P2 1
DAO-P0-03 Correction + Re-review: P0 0 / blocking P1 0 / P2 0 / Technical PASS
Correction fresh evidence: Build `bc420d5554e849fdbad32e65296a7188` + DAO `b3773a90332b4d6295670a37f276d44f` 7/7
Unchanged-executable retained evidence: affected `bcb9e15f9b8942d886768896a35b0bf2` 13/13 + DACE `977d1bc1566f41d7b805e608cd8e1dcb` 15/15
Current System owner: DataAssetAuthoring.md v1.4.1
Ammo durable writer: Accepted via typed provider + shared CFDADurableCore
Ammo DACE contract/history: Implemented / CFDAAmmoDace / DACE-AmmoData bootstrap exact1
DAO-P0-04 Pre-Implementation Contract Review: Historical HOLD / P0 0 / blocking P1 4 / P2 0
DAO-P0-04 Contract Correction + Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
Correction final evidence: Build `5c2300543af24ce78b1eddd4d409d43e` + DAO `503460d34b5944fda2efccda396d5d76` 9/9 + DACE `e91c4f1d19ca401495e138ca35497474` 15/15
Ammo DACE descriptor/probe/accepted bootstrap: Implementation Complete / Fresh Validation PASS
Implementation evidence: Build `14fc6c12e83a4d7a98154ad6eb59c6ea` + DAO `69c4d86b35d940c0bba8c51830181e91` 13/13 + existing Missile DACE `357d7ac193e84d0eba57213b220424b0` 15/15
DAO-P0-04 Post-Implementation Mid-review: Historical HOLD / P0 0 / blocking P1 1 / P2 1
DAO-P0-04 Correction + Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
Correction fresh evidence: Build `d93c392b125d4b338060307f7d22a65e` PASS + DAO `577467e756184582ae84ef5f6c9f4202` 13/13 + existing Missile DACE `fc7b719448f1454eab99dbda0e405b43` 15/15
Ammo accepted history owner: `CFDAAmmoDaceBase.cpp` dedicated append-only / existing bootstrap exact1 values+signature unchanged
DAO-P0-04 Technical Acceptance: PASS
DAO-P0-05 Pre-Implementation Contract Review: Historical HOLD / P0 0 / blocking P1 2 / P2 1
DAO-P0-05 Contract Correction + Independent Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
DAO-P0-05 implementation: Complete / Fresh Validation PASS
DAO-P0-05 final implementation evidence: Build `39e4973ff0ad4e31803c679d272533ff` PASS + focused `148c4f325b724b2fa7bfd7158819c531` 4/4 + affected `0f2f69983f024986aa87a98afee2aed9` 13/13 + existing Missile DACE `8967abe39f1a4d128c3ce455ede970a5` 15/15
DAO-P0-05 protection/residue: protected exact10 diff0 + Ammo bootstrap fixed readback unchanged + `/Game/Test/CarFight/DAOP05` Ammo exact0 + persisted CFAmmoData HeavyFinite/RocketFinite exact2 only
DAO-P0-05 Post-Implementation Mid-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
DAO-P0-05 Technical Acceptance: PASS
DAO-P0-06 Pre-Acceptance/Measurement Review: PASS / P0 0 / blocking P1 0 / P2 1 non-blocking
DAO-P0-06 Reuse Measurement / Acceptance: PASS
DAO-P0-06 Current System Promotion: Complete
Third-type core algorithm rewrite: 0 required
DamageData third onboarding: Candidate / Not Started
Current Active: CF-FQ-039 unchanged
Exact next: none / CF-FQ-051 closed
```

---

## 13. Changelog

### v0.3.22 - 2026-09-10

- `DAO-P0-06 Final Audit Correction + Re-review`를 문서-only로 완료했다. v0.3.21/v1.4.0 closure를 독립 재검수한 최초 판정 `P0 0 / blocking P1 1 / P2 2 / HOLD`에서 Current §2의 stale Ammo 미지원 표현과 완료 Feature header 누락을 교정했다.
- `CFAmmoData`는 이미 Current Production provider exact2 중 하나임을 명확히 하고, `CFDamageData`·`CFVehicleSensorData`·기타 신규 DataAsset 타입만 별도 provider-centric onboarding 대상으로 남겼다. hybrid shared/generic implementation과 legacy Missile compatibility의 물리적 owner 혼재는 `P2 1 non-blocking` maintenance debt로 유지한다.
- Source/Asset/Test mutation과 Build/Automation 재실행은 0이다. 직전 accepted Build/focused4/affected13/Missile DACE15 evidence를 그대로 유지하고 protected exact10 diff0을 재확인한다.
- 최종 독립 재검수 판정은 `P0 0 / blocking P1 0 / P2 1 non-blocking / PASS`다. CF-FQ-051은 Done / Technical Complete / Historical + Retained Path / G5 Deferred를 유지하며 남은 CF-FQ-051 Gate는 없다.

Migration: v0.3.22부터 현재 구현 owner는 `DataAssetAuthoring.md v1.4.1`이다. v0.3.21의 Systems Promotion evidence는 당시 Historical 기록으로 보존하되, 현재 owner 포인터는 v1.4.1을 사용한다. 새 DataAsset 타입은 별도 lifecycle에서 onboarding한다.

### v0.3.21 - 2026-09-10

- `DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion`을 current Source 기준으로 완료했다. 사전 contract review는 `P0 0 / blocking P1 0 / P2 1 / PASS`; P2는 generic implementation과 legacy Missile compatibility의 물리적 owner 혼재로 third-type correctness를 막지 않는 non-blocking maintenance debt다.
- second onboarding의 구조 지표는 dedicated shared foundation exact6/2300 physical lines, hybrid shared+compat exact7/6363 physical lines, framework-impact exact13, Ammo-specific production exact5/1789 physical lines, Missile parity/compat reviewed exact9, feature-owned Automation exact17이다. overlap category는 합산하지 않고 LOC를 개발 시간으로 해석하지 않는다.
- AmmoData는 Preview/approval/TOCTOU/BatchPlanHash/result aggregation/rollback·uncertainty/durable save-readback/DACE accepted-chain·migration algorithm을 복제하지 않았다. third type는 typed provider/schema/parser/serializer/extract/materialize/current-state/DACE descriptor/history + provider registration + explicit admission으로 확장하며 core algorithm rewrite는 0 required다.
- DAO-P0-06 executable Source mutation 0이므로 직전 Build `39e4973ff0ad4e31803c679d272533ff`, focused4 `148c4f325b724b2fa7bfd7158819c531`, affected13 `0f2f69983f024986aa87a98afee2aed9`, Missile DACE15 `8967abe39f1a4d128c3ce455ede970a5`를 중복 실행하지 않았다. Existing OperationalEntry와 CF-FQ-045 Ammo identity/health Accepted baseline도 유지한다.
- Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, protected exact10, CF-FQ-039 Active와 병렬 dirty를 보존한 채 `DataAssetAuthoring.md v1.4.0`으로 Current System Promotion을 완료하고 CF-FQ-051을 Technical Complete / Done으로 닫았다. representative Plan은 Historical + Retained Path / G5 Deferred다.
- DamageData는 third onboarding Candidate로만 남기며 자동 착수하지 않는다. hybrid owner P2는 실제 third onboarding에서 반복 수정/충돌이 발생할 때만 별도 maintenance/scaffold Feature로 검토한다.

Migration: v0.3.21부터 이 Plan은 CF-FQ-051의 Historical evidence owner다. 현재 구현 판단은 `DataAssetAuthoring.md v1.4.0`과 actual Source를 우선하며 이 Plan의 과거 Ready/HOLD/next gate를 현재 착수 지시로 사용하지 않는다. 새 DataAsset type은 별도 lifecycle에서 provider-centric onboarding을 연다.

### v0.3.20 - 2026-09-10

- `DAO-P0-05 Post-Implementation Mid-review`를 current Source 기준 read-only로 독립 수행해 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다. exact Staging path owner-before-read, mixed Explicit Paths provider-neutral dispatch, same normalized path/mode fresh Review replay, class-scoped StableLogicalId/global TargetObjectPath duplicate, source/current TOCTOU, sequential/PartialApplied semantics와 Missile compatibility를 모두 implementation contract와 재대조했다.
- path owner는 full production provider set의 CanonicalStagingRoot containment로 JSON read 전에 exact1 결정되고 그 뒤 allowed TypeKey scope를 검사하므로 overlapping-root ambiguity를 scope filtering으로 숨길 수 없다. Mixed discovery는 owner provider의 parse/current operations 뒤 common Preview/duplicate/hash/approval core를 재사용하며 payload-first routing과 parent-root recursive trust는 없다.
- third-type reuse 관점에서 exact2 allowed-TypeKey list는 core algorithm의 타입별 분기가 아니라 새 provider 자동 승격을 막는 explicit admission policy로 판정했다. 세 번째 타입은 provider 등록 + explicit admission이 필요하지만 shared Preview/Review/TOCTOU/Apply algorithm을 재작성하지 않는다. Historical Public Missile Preview DTO도 mixed mode에서는 structural projection만 담당하고 typed payload가 approval authority가 아니므로 blocker가 아니다.
- focused exact4에 missing-file 전용 terminal case는 없지만 production `FileExists` pre-read fail-closed가 직접 존재하고 wrong payload TypeKey는 provider negative regression에서 검증되므로 추가 P2 blocker로 보지 않았다. Existing affected13의 PartialApplied regression도 shared sequential apply 의미를 직접 보장한다.
- 이번 Mid-review의 executable Source mutation은 0이므로 Build `39e4973ff0ad4e31803c679d272533ff`, focused `148c4f325b724b2fa7bfd7158819c531` 4/4, affected `0f2f69983f024986aa87a98afee2aed9` 13/13, Missile DACE `8967abe39f1a4d128c3ce455ede970a5` 15/15를 중복 실행하지 않았다. protected exact10 diff0과 fresh AssetDump CFAmmoData HeavyFinite/RocketFinite exact2 only를 재확인했다.
- DAO-P0-05 Technical Acceptance를 PASS로 닫고 DAO-P0-06 `Reuse Measurement / Acceptance / Current System Promotion`을 Ready / Not Started로 전진시켰다. 이번 단계에서는 DAO-P0-06 측정/구현/Promotion을 시작하지 않았다.

Migration: v0.3.20부터 DAO-P0-05 multi-type operational integration은 Technical Accepted다. Product canonical Ammo exact0과 existing Missile compatibility/sequential Apply 계약은 유지한다. 다음 exact Gate는 `DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion`이며, 이 Gate가 완료되기 전 AmmoData를 최종 Current 지원 타입으로 승격했다고 해석하지 않는다.

### v0.3.19 - 2026-09-10

- `DAO-P0-05 Canonical Ammo Staging Pilot / Multi-Type Integration`을 구현하고 fresh validation을 완료했다. `CFDATypeDispatch v1.6.0`에 production registry를 캡슐화한 exact Staging path owner resolver를 추가해 JSON read 전 full provider-root containment owner exact1을 요구하고 owner0/owner>1/allowed TypeKey scope mismatch를 fail-closed한다.
- `CFDAStagingOps v1.4.0`은 empty selection과 Missile-only selection의 기존 동작을 보존하면서 Ammo가 포함된 non-empty canonical exact paths만 provider-neutral mixed Explicit Paths로 일반화했다. Review는 동일 normalized selection/mode를 fresh replay하고 Console StableLogicalId shorthand와 `SyncProductStaging` Product Low/Normal/High exact3는 변경하지 않았다.
- 신규 actual-provider mixed regression은 disposable `__AutomationP05__` + `/Game/Test/CarFight/DAOP05/`에서 Missile 1 + Ammo 1 one-approval durable exact2, class-scoped same textual StableLogicalId, global duplicate target reject, source/current TOCTOU와 residue0을 검증한다. Product canonical Ammo는 exact0 zero-row mutation0을 유지하며 fake Product/Apply/Save를 만들지 않았다.
- affected13 첫 실행에서 현재 provider completeness와 어긋난 old test-only second-provider fixture를 발견했다. Production contract를 완화하지 않고 `CFDAStagingTests v1.8.0` fixture에 explicit DACE canonical exact0 declaration만 추가한 후 authoritative UE 5.8 Build `39e4973ff0ad4e31803c679d272533ff` PASS, P0-05 focused `148c4f325b724b2fa7bfd7158819c531` 4/4, affected `0f2f69983f024986aa87a98afee2aed9` 13/13, Missile DACE `8967abe39f1a4d128c3ce455ede970a5` 15/15 PASS를 확보했다.
- protected exact10 scoped diff 0, Ammo accepted bootstrap fixed field/signature readback unchanged, `/Game/Test/CarFight/DAOP05` persisted Ammo residue0, 전체 persisted CFAmmoData는 HeavyFinite/RocketFinite exact2만 존재함을 확인했다. Product Apply/Save, HeavyFinite/RocketFinite save/reclassification, CF-FQ-039 lifecycle 변경, 기존 dirty 정리, commit/push는 0이다.
- 이 checkpoint는 `Implementation + Fresh Validation PASS`이며 DAO-P0-05 Technical Acceptance는 아직 Pending이다. exact next는 `DAO-P0-05 Post-Implementation Mid-review`이고 PASS 전 DAO-P0-06을 시작하지 않는다.

Migration: v0.3.19부터 current Source에는 actual MissileGuidePreset+AmmoData mixed non-empty Explicit Paths operational session이 존재한다. 그러나 기존 Missile Public/console/Product Sync semantics는 compatibility surface로 유지한다. DAO-P0-05 Post-Implementation Mid-review가 Technical PASS하기 전에는 이 구현을 final multi-type acceptance나 DAO-P0-06 Current System Promotion 완료로 해석하지 않는다.

### v0.3.18 - 2026-09-10

- `DAO-P0-05 Contract Correction`에서 v0.3.17 Pre-Implementation Review의 blocking P1 2건과 P2 1건을 구현 전에 문서 계약으로 교정했다. Product exact0 mutation0 acceptance와 actual mixed Review/TOCTOU integration lane을 분리했고, exact0은 no-op approval이 아니라 zero-row 정상 contribution으로 고정했다.
- mixed operational discovery는 `Canonical Product Set`과 `Explicit Paths` 두 mode만 허용한다. path owner는 JSON read 전 code-owned production registry의 provider-owned CanonicalStagingRoot containment로 exact1 결정하며 owner 0/>1, allowed TypeKey scope 밖 owner를 fail-closed한다. payload-first TypeKey와 parent-root recursive trust, mixed empty-selection all-provider scan은 금지했다.
- existing Missile `DiscoverMissilePresetPreview(empty = Missile root discovery)`, Public/console API와 `SyncProductStaging` Product exact3는 compatibility facade로 보존한다. `FCFDAStagingOpsSession`은 empty selection을 기존 Missile whole-root mode로 유지하고 non-empty canonical selection만 provider-neutral Explicit Paths mode로 일반화하며 Review가 동일 mode/path scope를 fresh replay하도록 동결했다. Console StableLogicalId shorthand는 Missile canonical path 변환 의미를 유지한다. 실제 mixed regression은 actual registered MissileGuidePreset+AmmoData와 `__AutomationP05__`/`/Game/Test/CarFight/DAOP05/` disposable fixture만 사용한다.
- mixed Apply는 global preflight + per-target immediate TOCTOU + sequential durable apply이며 cross-target all-or-nothing은 non-goal이다. existing `PartialApplied`와 uncertainty precedence를 유지한다.
- 독립 재검수에서 actual Ammo provider의 `CanonicalStagingRoot=Authoring/DataAssetStaging/AmmoData`, explicit DACE canonical target exact0, ReviewedMutationReady/ContractReady를 literal Source로 재확인했다. correction 계약을 current TypeDispatch/Ops/Apply와 대조한 결과 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`이며, positive disposable mixed Review→Apply는 Missile 1 + Ammo 1 exact provider durable Applied까지 증명하도록 acceptance를 강화했다.
- 이번 단계는 executable Source mutation 0이며 protected exact10, Product Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다. DAO-P0-05 implementation은 아직 시작하지 않았으며 이제 Implementation Ready다.

Migration: v0.3.18부터 P0-05 구현은 mixed provider discovery를 parent recursive scan이나 payload-first routing으로 일반화하지 않는다. Product mutation0 lane과 disposable mutation lane을 분리하고, exact path owner resolver는 code-owned provider registry를 캡슐화한 좁은 seam으로 구현한다. Contract Correction + Independent Re-review가 `P0 0 / blocking P1 0 / P2 0` Technical PASS했으므로 다음 exact Gate는 `DAO-P0-05 Canonical Ammo Staging Pilot / Multi-Type Integration Implementation`이다.

### v0.3.17 - 2026-09-10

- `DAO-P0-05 Canonical Ammo Staging Pilot / Multi-Type Integration` 사전 계약검수를 current Source 기준 read-only로 수행해 `P0 0 / blocking P1 2 / P2 1 / HOLD`로 판정했다. P0/P1 0 전 구현 금지 조건에 따라 DAO-P0-05 C++/script/Staging/Product implementation은 시작하지 않았다.
- current `CFDATypeDispatch`/Apply core는 production provider exact2, class-scoped StableLogicalId, global TargetObjectPath duplicate, mixed common hash/Review, exact provider dispatch, global preflight와 per-target TOCTOU를 이미 지원한다. DACE canonical Ammo exact0도 explicit valid empty set이며 selected provider history/revision은 exact TypeKey에 격리돼 있다.
- blocking P1-1은 Product Ammo exact0 mutation0 acceptance와 Create/Update-only Reviewed approval 의미가 혼재한 점이다. Product acceptance는 exact0 유지/fake Product 0/Apply·Save 0 lane으로 분리하고, 실제 mixed Review/TOCTOU는 disposable test-owned Missile+Ammo mutation fixture에서 증명해야 한다. no-op approval을 열거나 Product asset을 acceptance용으로 만들지 않는다.
- blocking P1-2는 `CFDAStagingOpsSession` discovery/selection이 아직 Missile canonical root에 결속돼 실제 registered Ammo와 Missile을 같은 operational batch로 만들 수 없는 점이다. Common core를 복제하지 않고 trusted provider root ownership 기반 provider-neutral mixed discovery/selection seam을 동결해야 하며 existing Missile Public/console/SyncProduct exact3 compatibility는 보존한다.
- P2-1은 mixed Apply가 cross-target atomic transaction이 아니라 per-target sequential durable apply이며 후속 실패 시 `PartialApplied`가 가능하다는 점이다. P0-05 acceptance가 all-or-nothing을 주장하지 않도록 non-goal로 명시해야 한다.
- 이번 review는 Source mutation 0이라 DAO-P0-04 final Build `d93c392b125d4b338060307f7d22a65e`, DAO13 `577467e756184582ae84ef5f6c9f4202`, Missile DACE15 `fc7b719448f1454eab99dbda0e405b43`을 재실행하지 않았다. protected exact10, Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다.

### v0.3.16 - 2026-09-10

- `DAO-P0-04 Correction + Re-review`에서 v0.3.15 Mid-review의 blocking P1 1건/P2 1건을 전건 교정하고 current Source + fresh validation + independent re-review 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다.
- blocking P1은 existing shared `CFDACommonPrimitives::FScopedSemanticTokenProbe`로 actual production fingerprint token emission을 관측하고 `CFDAAmmoDaceTests`의 independent literal expected manifest와 exact 비교하도록 교정했다. non-null AmmoIcon + AmmoTags exact2 fixture는 token exact15, null AmmoIcon fixture는 path token을 제외한 exact14를 요구하며 schema/revisions/class + Ammo exact8, AmmoTags Count/[], AmmoIcon IsNull/conditional Path coverage를 고정한다.
- P2는 기존 `DACE-AmmoData-S1-A1-Bootstrap` exact1의 값/signature를 변경하지 않고 신규 `CFDAAmmoDaceBase.cpp v1.0.0` dedicated append-only production owner로 `GetAcceptedSnapshots()` implementation을 분리했다. `CFDAAmmoDace.cpp v1.1.0`은 mutable current descriptor/migration authority만 유지하고 focused regression은 bootstrap fixed field/signature 전체를 literal expected 값으로 고정한다.
- fresh official UE 5.8 Build `d93c392b125d4b338060307f7d22a65e` PASS, DAO exact13 `577467e756184582ae84ef5f6c9f4202` 13/13 PASS, existing Missile DACE exact15 `fc7b719448f1454eab99dbda0e405b43` 15/15 PASS다. legacy protected exact9 scoped worktree diff는 0이며 새 Ammo Base는 fixed readback 후 이후 protected exact10에 추가한다.
- Product canonical Ammo exact0, Missile Product Low/Normal/High exact3, HeavyFinite/RocketFinite exact2, protected `CFDAContractBase.cpp`, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다. DAO-P0-05 implementation은 시작하지 않았다. exact next는 `DAO-P0-05 — Canonical Ammo Staging Pilot / Multi-Type Integration`이다.

### v0.3.15 - 2026-09-10

- `DAO-P0-04 Post-Implementation Mid-review`를 current Source 기준 read-only로 수행해 `P0 0 / blocking P1 1 / P2 1 / HOLD`로 판정했다. v0.3.14 구현과 fresh Build/DAO13/Missile DACE15 PASS는 유지하지만 Technical Acceptance와 DAO-P0-05 진입은 보류한다.
- SourceShape exact8, AdapterShape exact19, SourceAdapterMapping exact16, SemanticContract exact14, `AmmoTags[]` String/FNameToken physical+semantic contract, `AmmoIcon` UTexture2D target-class/no-load, per-TypeKey revision/history isolation과 Product canonical Ammo exact0 의미는 current Source와 기존 DAO exact13 evidence에 일치한다.
- blocking P1은 Ammo production fingerprint probe가 같은 `BuildSemanticFingerprint()` 결과끼리의 round-trip self-consistency만 비교해 future `AppendPayloadTokens()` token omission을 독립적으로 검출하지 못할 수 있는 점이다. production token coverage observation 또는 exhaustive per-field fingerprint inclusion regression이 필요하다.
- P2는 `DACE-AmmoData` accepted bootstrap exact1과 mutable current descriptors가 같은 `CFDAAmmoDace.cpp`에 공존해 accidental rebaseline 방지 경계가 existing Missile dedicated `CFDAContractBase.cpp`보다 약한 점이다. bootstrap record는 변경하지 않고 dedicated append-only Ammo history owner로 분리하는 방향을 권고한다.
- Mid-review Source mutation은 0이므로 Build `14fc6c12e83a4d7a98154ad6eb59c6ea`, DAO `69c4d86b35d940c0bba8c51830181e91` 13/13, Missile DACE `357d7ac193e84d0eba57213b220424b0` 15/15를 중복 실행하지 않았다. protected exact9 scoped diff 0을 재확인했다.

### v0.3.14 - 2026-09-10

- `DAO-P0-04 Ammo DACE Descriptor / Probe / Bootstrap Implementation`을 완료했다. `CFDAAmmoDace` independent owner에 Ammo exact8 SourceShape, AdapterShape exact19, SourceAdapterMapping exact16, SemanticContract exact14와 `DACE-AmmoData-S1-A1-Bootstrap` exact1을 추가했다.
- Ammo provider에 deterministic whole-record memory serializer를 추가하고 DACE owner를 `CFDAAmmoDace`, readiness를 `ContractReady`로 승격했다. Product canonical target은 explicit exact0을 유지하며 fake Product Ammo/Staging 생성과 HeavyFinite/RocketFinite 재분류/저장은 수행하지 않았다.
- `Payload.AmmoTags[] = String/FNameToken` element contract를 non-empty production serializer probe로 증명하고 wrong element type, FName semantic duplicate, order/case permutation fingerprint equivalence를 고정했다. `AmmoIcon`은 `SoftObject / /Script/Engine.Texture2D / Scalar` Source contract와 null/non-null, target-class drift, referenced asset no-load를 검증했다.
- `CFDAContractGuard v1.5.0`은 Ammo의 `MaximumLoadableAmmoCount` SourceShape를 stable `Int`로 관측하기 위해 FIntProperty token만 최소 추가했다. existing Missile DACE exact15가 fresh binary에서 15/15 PASS해 Missile accepted descriptor/signature behavior가 유지됨을 확인했다.
- fresh official UE 5.8 Build `14fc6c12e83a4d7a98154ad6eb59c6ea` PASS, DAO exact13 `69c4d86b35d940c0bba8c51830181e91` 13/13 PASS, existing Missile DACE exact15 `357d7ac193e84d0eba57213b220424b0` 15/15 PASS다. protected exact9 scoped worktree diff는 0이다.
- bootstrap candidate는 current descriptor/probe/revision/exact0 migration gate와 Missile regression이 모두 PASS한 뒤 final accepted exact1 상태로 유지했다. Initial impact는 `NoMigration / NotRequired`, current declaration은 nullptr이며 no-delta 상태의 duplicate append는 blocked다.
- 이번 checkpoint는 구현 + fresh validation 완료다. Post-Implementation Mid-review 전에는 DAO-P0-04 Technical Acceptance나 DAO-P0-05 Ready로 확대하지 않는다.

### v0.3.13 - 2026-09-10

- `DAO-P0-04 Contract Correction + Re-review`에서 v0.3.12 blocking P1 4건을 전건 교정하고 independent Source 재검수 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다. Actual Ammo DACE descriptor/probe/accepted bootstrap은 아직 시작하지 않았다.
- TypeDispatch v1.5.0에 authoring readiness와 독립된 DACE `ContractNotReady/ContractReady`, explicit provider-owned canonical Staging target set을 추가했다. Missile은 ContractReady + 기존 exact3, Ammo는 ReviewedMutationReady이지만 DACE ContractNotReady + declared Product exact0으로 분리했다.
- `CFDAContractGuard v1.4.0`에 provider-parameterized descriptor signature, direct authored Reflection, serialized Adapter observation, revision/history/canonical Staging/migration gate seam을 추가했다. 기존 Missile facade/API와 DACE15는 그대로 유지했다.
- `AmmoTags[]` non-empty Array element physical kind를 synthesized path로 관측하고 existing semantic duplicate/order-invariance regression과 결합했다. `AmmoIcon`은 Reflection에서 `SoftObject / /Script/Engine.Texture2D / Scalar`로 직접 관측한다.
- 재검수 중 ContractNotReady Ammo가 exact0 compatibility PASS만으로 generic migration promotion prerequisite를 통과할 수 있는 우회 가능성을 추가 발견해 `EvaluateMigrationGateForProvider`와 provider readiness revision guard로 fail-closed했다. 이 교정 뒤 final Build/DAO/DACE를 다시 fresh 실행했다.
- final UE 5.8 Build `5c2300543af24ce78b1eddd4d409d43e` PASS, DAO exact9 `503460d34b5944fda2efccda396d5d76` 9/9 PASS, existing Missile DACE exact15 `e91c4f1d19ca401495e138ca35497474` 15/15 PASS다. P0-03 disposable AssetDump residue 0, protected exact9 diff 0을 재확인했다.
- Product Ammo exact0, Missile Product canonical exact3, accepted `CFDAContractBase.cpp`, HeavyFinite/RocketFinite, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다. commit/push는 수행하지 않았다.

### v0.3.12 - 2026-09-10

- `DAO-P0-04 Ammo DACE Contract Provider / Revision / Migration Guard` 구현 전 계약검수를 current Source 기준으로 수행해 `P0 0 / blocking P1 4 / P2 0 / HOLD`로 판정했다. P1 0 확인 전 구현 금지 조건에 따라 DAO-P0-04 C++ 구현은 시작하지 않았다.
- P1-1은 current `FCFDAContractGuard`의 descriptor/probe/accepted history/current revision/current migration wrapper가 MissileGuidePreset 단일 identity에 묶여 있어 TypeDispatch의 DACE metadata가 실제 per-TypeKey dispatch/history authority가 되지 못하는 문제다. common algorithm과 per-TypeKey contract provider를 분리하고 Missile compatibility facade/DACE15를 보존하도록 correction contract를 고정했다.
- P1-2는 current serializer shape observer가 Array container만 관측해 `Payload.AmmoTags[]` element String/FNameToken 계약을 증명하지 못하는 문제다. synthesized `[]` element path + non-empty production serializer probe + wrong-type/semantic-duplicate/permutation regression을 요구한다.
- P1-3은 scalar `TSoftObjectPtr<UTexture2D>`의 target class path가 current Reflection descriptor에 나타나지 않는 문제다. existing `ReflectedTypePath`에 stable SoftObject target class를 기록하고 target-class drift regression을 추가하되 descriptor row format과 Missile signatures는 변경하지 않는다.
- P1-4는 current canonical Staging migration wrapper가 Missile Product exact3을 hard-code해 Product canonical Ammo exact0을 표현할 수 없는 문제다. per-TypeKey canonical target-set authority를 도입하고 Ammo exact0을 explicit valid empty set으로 취급하되 old-revision 검증은 memory/test-owned Ammo JSON으로 수행한다.
- Ammo baseline은 SchemaRevision 1 / AdapterContractRevision 1 / `DACE-AmmoData` independent history namespace를 유지한다. Ammo change/revision/declaration/history는 Missile chain에 영향 0이어야 하며 protected `CFDAContractBase.cpp`는 Ammo bootstrap 저장소로 사용하지 않는다.
- 이번 단계는 read-only Source/contract review + 문서 projection만 수행했으므로 Build/Automation을 실행하지 않았다. 시작과 종료 보호 audit에서 Missile Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, HeavyFinite/RocketFinite exact2 mutation 0을 유지한다.

### v0.3.11 - 2026-09-10

- `DAO-P0-03 Correction + Re-review`에서 v0.3.10 Mid-review의 blocking P1 1건/P2 1건을 전건 교정하고 current Source/Systems/Plan 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다.
- Current Systems §2/§10/§18/§20을 MissileGuidePreset + AmmoData exact2 `ReviewedMutationReady`와 `CFDADurableCore` 단일 durable transaction authority로 정렬했고, `CFDAAmmoProvider.cpp v1.2.1`은 실행 변경 없이 stale read-only readiness 주석만 교정했다.
- fresh official UE 5.8 Build `bc420d5554e849fdbad32e65296a7188` PASS, DAO focused `b3773a90332b4d6295670a37f276d44f` exact7/7 PASS다. executable semantics delta가 없어 affected CF-FQ-049 `bcb9e15f9b8942d886768896a35b0bf2` 13/13과 DACE `977d1bc1566f41d7b805e608cd8e1dcb` 15/15는 직전 fresh evidence를 유지하고 중복 재실행하지 않았다.
- correction 후 fresh AssetDump에서 `/Game/Test/CarFight/DAOAmmoP03` CFAmmoData asset count 0, 보호 exact9 scoped Git diff 0을 재확인했다. Product canonical Ammo exact0, Missile Product/accepted/HeavyFinite/RocketFinite mutation 0, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다.
- `CFDAContract*`에는 AmmoData DACE implementation match가 0이며 DAO-P0-04는 시작되지 않았다. DAO-P0-03 Technical PASS로 DAO-P0-04가 Ready가 되었을 뿐이다.

### v0.3.10 - 2026-09-10

- `DAO-P0-03 Post-Implementation Mid-review`를 v0.3.9 implementation + fresh validation baseline에서 독립 수행하고 `P0 0 / blocking P1 1 / P2 1 / HOLD`로 판정했다.
- Source 재검수에서 `CFDADurableCore`가 typed Staging durable transaction의 단일 authority이고 MissileGuidePreset/AmmoData production provider exact2가 모두 `ReviewedMutationReady`임을 확인했다. Ammo exact8 materializer와 shared Review/TOCTOU/provider dispatch도 구현 계약과 일치한다.
- blocking P1은 Current Systems §10이 아직 Missile-only typed materializer 하나만 지원하는 것처럼 기술해 실제 exact2 + shared durable core와 모순되는 문제다. P2는 `CFDAAmmoProvider.cpp` 일부 주석에 남은 `read-only` readiness 표현이다.
- fresh AssetDump에서 `/Game/Test/CarFight/DAOAmmoP03` CFAmmoData asset count 0을 확인했고 보호 exact9 scoped diff도 0이다. Product/accepted/HeavyFinite/RocketFinite mutation 0과 CF-FQ-039 Active/기존 병렬 dirty를 보존했다.
- `CFDAContract*`에는 Ammo DACE implementation match가 0이므로 DAO-P0-04는 시작되지 않았다. 이번 review는 Source mutation 0이라 직전 Build/DAO7/affected13/DACE15 PASS를 반복 실행하지 않았으며 해당 evidence는 유지한다.

### v0.3.9 - 2026-09-10

- `DAO-P0-03 Ammo Reviewed Apply / Durable Typed Writer` 구현 전 계약검수에서 `P0 0 / blocking P1 2 / P2 0`을 확인했다. P1은 Missile-bound durable transaction 복제 위험과 protected HeavyFinite/RocketFinite exact2의 writer fixture 오용 위험이었다.
- `CFDADurableCore` provider-neutral typed durable authority를 추가하고 기존 Missile writer도 동일 core로 rewire해 durable sequencing의 타입별 복제를 제거했다. Ammo provider는 exact8 materializer + `ApplyReviewedMutation`을 등록하고 `ReviewedMutationReady`로 승격했다.
- P0-03 durable tests는 `/Game/Test/CarFight/DAOAmmoP03/` + `Authoring/DataAssetStaging/AmmoData/__AutomationP03__/` disposable root만 사용한다. Create/Update, non-empty AmmoTags, non-null AmmoIcon, stale approval, pre-existing dirty target, SaveStateUnconfirmed을 actual Ammo provider 경로에서 검증했다.
- fresh official UE 5.8 Build `0ef9b9c2d9724b749b4226153e8e8ba6` PASS, DAO `719a71370ff142c78850a3bc867c1f52` exact7/7 PASS, affected CF-FQ-049 `bcb9e15f9b8942d886768896a35b0bf2` exact13/13 PASS, DACE `977d1bc1566f41d7b805e608cd8e1dcb` exact15/15 PASS다.
- 보호 exact9 diff는 Missile Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, Ammo persisted HeavyFinite/RocketFinite exact2 모두 0이다. Product canonical Ammo exact0, disposable residue 0, CF-FQ-039 Active와 기존 병렬 dirty, commit/push 0을 유지했다.
- 현재 상태는 `Implementation + Fresh Validation PASS / Post-Implementation Mid-review Pending`이다. DAO-P0-03 Technical PASS를 아직 확정하지 않았고 DAO-P0-04 DACE는 시작하지 않는다.

### v0.3.8 - 2026-09-10

- `DAO-P0-02 Correction + Re-review`에서 v0.3.7 Post-Implementation Mid-review의 blocking P1 2건/P2 2건을 전건 교정하고 current Source 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다.
- `TargetObjectPath` canonical validation과 `BaseSemanticFingerprint` physical null/string + canonical SHA-256 parse/validation을 `CFDACommonPrimitives` 단일 provider-neutral authority로 통합했다. Missile compatibility path와 Ammo provider는 같은 helper를 호출하고 타입별 중복 구현은 제거했다.
- actual Ammo provider Preview row 기반 BatchPlanHash deterministic/order-independent integration regression과 empty Literal DisplayName / NAME_None FamilyId semantic regression을 DAO exact4 안에 보강했다.
- fresh official UE 5.8 Build `898d428caeba4895bb5b0668af513a6e` PASS, DAO focused `b6cb4476ec6d47f4a50214901498b0ed` 4/4 PASS, affected CF-FQ-049 `d6e52207e9d8436d9fae7c27f2027d80` 13/13 PASS, DACE `8789b7e7a1c2446dae91db3bf3f9f7c2` 15/15 PASS다. affected durable fixture residue는 0이다.
- 보호 exact9 scoped diff는 Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, Ammo persisted exact2 모두 0이다. Product Apply·Save 0 / accepted append 0 / Ammo persisted Save 0 / CF-FQ-039 Active와 기존 병렬 dirty 보존 / commit·push 0을 유지했다.
- Ammo provider는 계속 `ReadOnlyPreviewReady` / `ApplyReviewedMutation=nullptr`이다. DAO-P0-03 writer/materializer는 시작하지 않았고 이제 다음 exact gate로 Ready 상태만 전진했다. Current System owner는 `DataAssetAuthoring.md v1.3.2`다.

### v0.3.7 - 2026-09-10

- `DataAssetAuthoring.md v1.3.1`을 actual DAO-P0-02 implementation state에 동기화해 v0.3.6의 Current owner pointer mismatch를 해소한 뒤 Post-Implementation Mid-review를 수행했다.
- current Source 기준 판정은 `P0 0 / blocking P1 2 / P2 2 / HOLD`다. P1은 common envelope field인 TargetObjectPath canonical validation과 BaseSemanticFingerprint physical parse/validation이 Ammo provider에서 기존 Missile path와 중복 구현돼 third provider reuse boundary를 깨는 문제다.
- P2는 actual Ammo provider envelope의 BatchPlanHash integration regression 부재와 P0-00 frozen optional semantic인 empty Literal DisplayName / NAME_None FamilyId의 focused evidence 부재다.
- shared 3-way Preview, class-scoped identity/global path duplicate, Ammo read-only mutation fail-closed, AmmoTags semantic, AmmoIcon metadata-only referenced-asset no-load 경계는 current Source 재감사에서 유지됨을 확인했다.
- 이번 단계는 문서 sync + read-only Source review만 수행했다. C++/script/JSON/.uasset/accepted baseline mutation은 0이며 v0.3.6 fresh Build/DAO4/affected13/DACE15 evidence는 implementation baseline으로 보존한다.
- DAO-P0-03 writer는 시작하지 않았다. exact next는 `DAO-P0-02 Correction + Re-review`이며 P1/P2 교정 뒤 fresh Build → DAO focused → affected13 → DACE15를 다시 수행한다.

### v0.3.6 - 2026-09-09

- `DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview` read-only implementation을 완료했다. Editor Private `CFDAAmmoProvider`가 exact8 typed payload/record, strict parser, semantic fingerprint, UObject extractor/current resolver와 `ReadOnlyPreviewReady` provider를 소유한다.
- Production TypeKey registry는 기존 Missile `ReviewedMutationReady`와 신규 Ammo `ReadOnlyPreviewReady` exact2가 됐다. Ammo `ApplyReviewedMutation=nullptr`를 유지해 DAO-P0-03 writer/materializer를 선행 구현하지 않았고 shared Apply는 Ammo mutation path를 fail-closed한다.
- AmmoTags는 set-like FName semantic/duplicate reject/order-independent fingerprint/canonical sort를 구현했다. AmmoIcon은 null/canonical top-level SoftObjectPath + Asset Registry metadata-only Texture2D compatibility validation이며 referenced asset load 0을 Source review와 existing/missing/wrong-class/no-load regression으로 확인했다.
- final UE 5.8 Build `32f46c889baf4bfdad6b1eebf7b2e0b1` PASS, DAO focused `381bf95e6628439585157d0d3e46d0fa` exact4/4 PASS, affected CF-FQ-049 `2d0e1e80e0244dc0b9f0520ffd249f01` 13/13 PASS, DACE `adb7c455699742f980e70a8d22aff4d5` 15/15 PASS다.
- scoped diff에서 Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, Ammo persisted exact2 모두 diff 0을 확인했다. Product Apply·Save 0 / accepted append 0 / Ammo persisted Save 0 / CF-FQ-039 Active와 기존 병렬 dirty 보존 / commit·push 0이다.
- 이번 상태는 구현 + fresh validation 완료이지 DAO-P0-02 Technical Acceptance 확정이 아니다. exact next는 `DAO-P0-02 Post-Implementation Mid-review`이며 이 review 전 DAO-P0-03 writer로 넘어가지 않는다.

### v0.3.5 - 2026-09-09

- `DAO-P0-02 Correction Fresh Technical Validation`을 current correction Source 기준으로 닫았다. Official UE 5.8 Editor Build `c191604c1f704717929c91f8d8b92f38` PASS / Exit 0, affected CF-FQ-049 `81210438baf4466b96059f77e4ea22b6` exact13/13 PASS, DACE `29139903dbec485e9b502dcc690697d1` exact15/15 PASS다.
- build authority blocker의 원인은 Project Registry 내부 repository identity `carfight_repo`를 legacy `carfight.editor.development` preset의 configured repository ID처럼 사용한 identity namespace 혼용이었다. legacy preset은 `repository_id=main_game`에 결속돼 있으며 이 authority로 official build job 생성과 실행이 정상화됐다. GoPyMCP 설정/Source mutation은 0이다.
- current correction Source인 `CFDACommonPrimitives`와 shared Type Dispatch/Missile compatibility 경로가 fresh compile/link되고 existing CF-FQ-049/DACE regression이 모두 PASS했다. 따라서 Source/Contract PASS에 fresh binary evidence가 결속됐다.
- Ammo typed schema/provider 본체와 Production Ammo registration은 아직 시작하지 않았다. Product Low/Normal/High Apply·Save 0, canonical Product exact3 mutation 0, accepted baseline mutation 0, Ammo persisted asset mutation 0을 유지한다.
- exact next는 `DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview` 구현이다. CF-FQ-039 Active와 기존 병렬 dirty를 그대로 보존하고 commit/push는 수행하지 않았다.

### v0.3.4 - 2026-09-09

- `DAO-P0-02 Contract Correction + Source Re-review`에서 v0.3.3 blocking P1 3건/P2 2건을 교정하고 current Source 기준 `P0 0 / blocking P1 0 / P2 0` Source/Contract PASS로 닫았다.
- provider readiness를 `ReadOnlyPreviewReady`와 `ReviewedMutationReady`로 분리하고 shared Apply가 read-only provider를 Staging source read 전에 fail-closed하도록 했다. Missile provider는 `ReviewedMutationReady`를 유지한다.
- Editor Private `CFDACommonPrimitives` authority를 추가해 JSON/Literal FText/FName/fingerprint primitive를 Missile file-local 구현에서 분리했고, 기존 Missile compatibility path는 common helper 위임으로 유지했다.
- AmmoIcon용 canonical SoftObjectPath strict parse와 Asset Registry metadata-only existence/class validation seam을 추가했다. `/Game` mount 강제 없이 top-level canonical path를 허용하며 common validator에서 referenced asset `LoadObject`/`ResolveObject`/`GetAsset` 호출 0을 source review로 확인했다.
- provider-local cached fingerprint exact integrity seam과 AmmoIcon null/existing Texture2D/missing/wrong-class/subobject/no-load focused regression을 기존 test matrix에 추가했다.
- Ammo typed schema/provider 본체와 Production Ammo registration은 시작하지 않았다. 따라서 사용자가 요구한 P1 0 이전 Ammo 구현 금지 조건을 보존했다.
- current correction의 fresh Build/Automation은 GoPyMCP `carfight.editor.development` preset과 Project repository identity mapping이 맞지 않아 build job 생성 전 차단됐다. 이를 compiler failure로 기록하지 않으며 DAO-P0-01 historical Build/affected13/DACE15 PASS를 이번 correction의 fresh PASS로 재사용하지 않는다.
- exact next는 approved CarFight build authority mapping 복구/확정 후 Official UE 5.8 Build → affected13 → DACE15을 fresh 실행하고, 그 evidence가 닫힌 뒤 `DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview` 구현으로 진입하는 것이다.

### v0.3.3 - 2026-09-09

- `DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview` 구현 전 계약검수를 current `CFAmmoData`, Data Asset Manager, DAO-P0-01 shared Type Dispatch/Apply/Ops Source 기준으로 수행해 `P0 0 / blocking P1 3 / P2 2 / Implementation HOLD`로 판정했다.
- P1은 production provider completeness가 `ApplyReviewedMutation`을 강제해 read-only P0-02와 P0-03 writer 경계를 깨뜨리는 문제, provider-neutral JSON/FText/fingerprint primitive가 Missile `CFDAStaging.cpp` file-local에 갇힌 문제, AmmoIcon strict parse와 Asset Registry Preview validation의 provider-local seam이 payload-free transport와 아직 명시 결속되지 않은 문제다.
- P2는 Ammo typed payload↔cached fingerprint integrity projection seam과 AmmoIcon no-load/compatible-class test evidence 명시 보강이다.
- 이번 단계는 read-only contract review와 문서 projection만 수행했다. DAO-P0-02 C++ implementation/Build/Automation/JSON/.uasset/accepted snapshot mutation은 0이며 DAO-P0-01 final Build/affected13/DACE15 evidence와 Missile Product exact3/accepted baseline을 보존한다.
- exact next는 `DAO-P0-02 Contract Correction + Re-review`이며 blocking P1 0 재검수 전 Ammo C++ 구현을 시작하지 않는다. CF-FQ-039 Active와 기존 병렬 dirty는 그대로 유지한다.

### v0.3.2 - 2026-09-09

- `DAO-P0-01 Correction + Re-review`에서 Mid-review blocking P1 4건/P2 2건을 전건 교정하고 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 shared multi-type foundation을 다시 승인했다.
- payload-free common Preview/Review/TOCTOU authority, complete exact TypeKey provider operation registry, provider-root Automation fixture 이동과 fresh provider dispatch를 반영했다. test-only second provider seam과 duplicate exact TypeKey fail-closed regression도 기존 affected test matrix 안에서 보강했다.
- final official UE 5.8 Build `2f8afa3de9144c55b74083ca4eb02aeb` PASS, affected CF-FQ-049 `ad820e42c46545c182b319c9d872b724` exact13/13 PASS, DACE `9857fc2e265a419bb7d75c8df7f9e454` exact15/15 PASS다. fixture residue는 0이다.
- Product Low/Normal/High Apply·Save 0 / canonical Product exact3 JSON·uasset diff 0 / accepted snapshot source diff 0 / bootstrap exact1 append 0을 유지했다. 현재 Production registered provider는 MissileGuidePreset exact1이다.
- `CFAmmoData` typed provider/schema/Preview/DACE extension은 아직 구현하지 않았다. exact next는 `DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview`이며 CF-FQ-039 Active와 기존 병렬 dirty를 그대로 보존한다.

### v0.3.1 - 2026-09-09

- `DAO-P0-01 Post-Implementation Mid-review`를 current Source 기준으로 수행해 `P0 0 / blocking P1 4 / P2 2 / HOLD`로 판정했다. 기존 v0.3.0의 Technical PASS projection은 취소하고 exact next를 `DAO-P0-01 Correction + Re-review`로 되돌렸다. DAO-P0-02는 HOLD다.
- P1은 shared core의 Missile-only `FCFDAStagingRecord/PreviewRow` 직접 의존, descriptor-only TypeKey registry로 인한 complete provider dispatch 부재, fresh Apply/TOCTOU의 direct Missile facade 호출, Development Editor에 compile되는 sibling Automation root allowlist의 provider-root trust boundary 약화다.
- P2는 second-provider common seam을 증명하는 foundation regression 부재와 duplicate exact TypeKey registration fail-closed invariant 부재다.
- 기존 official UE 5.8 Build PASS + affected CF-FQ-049 13/13 + DACE 15/15는 Missile behavior/signature parity evidence로 계속 유효하다. Product exact3 Apply·Save 0 / JSON·uasset diff 0 / accepted bootstrap exact1 append 0도 무효화하지 않는다.
- 이번 Mid-review는 read-only Source/contract 감사와 상태 문서 투영만 수행했으며 C++/JSON/.uasset/accepted snapshot mutation은 0이다. CF-FQ-039 Active와 기존 병렬 dirty를 유지했다.

### v0.3.0 - 2026-09-09

- `DAO-P0-01 Shared Type Dispatch Foundation + Missile Parity Rewire`를 Technical PASS로 닫았다. Editor Private에 payload-free common envelope, exact `SchemaId + DataAssetTypeClassPath` trusted TypeKey registry, provider-owned StagingRoot와 class-scoped StableLogicalId duplicate namespace를 구현했다.
- `CFDAMissileProvider`를 first typed provider로 구성해 existing production parse/fingerprint/extract/current-state/serialize/materialize callback ownership을 묶었고, 기존 Public `FCFDAStagingRecord`/`FCFDAStagingService`/Ops/Apply Missile API는 signature 변경 없이 compatibility facade로 보존했다.
- Production provider/root 정책을 강화한 첫 regression에서 기존 CF-FQ-049 test-owned Staging roots가 차단되는 문제를 발견했다. Production root를 완화하지 않고 `WITH_DEV_AUTOMATION_TESTS` exact allowlist `__AutomationMissing__` / `__AutomationLoaded__` / `__AutomationP04__`만 복원해 최종 affected regression exact13/13 PASS를 확보했다.
- final official UE 5.8 Build `6737c1bff29a40a4bfd9bb6be98213b1` PASS, affected CF-FQ-049 `34de9e0426dc4f33ae05a76195bc6190` exact13/13 PASS, DACE `be73e54a237c4ea8aa20b4e2a5c001e1` exact15/15 PASS다.
- Public DataAuthoring header diff 0, Product Low/Normal/High exact3 JSON·uasset diff 0, Apply·Save 0, accepted snapshot source diff 0 / bootstrap exact1 / append 0을 확인했다. Ammo typed payload와 DACE Ammo contract extension은 아직 구현하지 않았고 exact next는 `DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview`다.
- 현재 단일 Active `CF-FQ-039`, 기존 병렬 dirty를 변경·정리·revert/stash/reset하지 않았으며 commit/push도 수행하지 않았다.

### v0.2.0 - 2026-09-09

- `DAO-P0-00 Initial Design Review`의 P1 6건/P2 2건을 전건 교정하고 current Source 재대조 후 `P0 0 / blocking P1 0 / P2 0` PASS로 닫았다. exact next는 `DAO-P0-01 Shared Type Dispatch Foundation + Missile Parity Rewire`다.
- transport를 common envelope + per-type typed record로 동결하고 기존 Public `FCFDAStagingRecord`/`FCFDAStagingService` Missile 계약은 CF-FQ-051 동안 compatibility facade로 유지한다. Approval은 payload를 소유하지 않고 Apply가 fresh source를 re-read해 exact provider typed re-parse/materialize하도록 고정했다.
- trusted provider identity를 `SchemaId + DataAssetTypeClassPath` exact TypeKey로 확정하고 provider가 revision, canonical StagingRoot, identity, parse/serialize/fingerprint/extract/materialize/current-state, DACE descriptor/probe/history namespace를 소유하도록 했다.
- StableLogicalId duplicate namespace를 exact class path + FName semantic으로 고정했다. 다른 DA class의 동일 textual ID는 허용하고 TargetObjectPath duplicate는 global fail-closed로 유지한다.
- Ammo exact8 semantic을 동결했다. UnitMassKg/MaximumLoadableAmmoCount는 Authoring에서 nonnegative strict reject/no-clamp, AmmoTags는 set-like case-insensitive FName + duplicate reject + order-independent fingerprint/canonical materialize, AmmoIcon은 null/canonical SoftObjectPath + path-only fingerprint + Asset Registry read-only Texture2D class validation이다. DisplayName empty Literal과 FamilyId NAME_None도 허용했다.
- DACE Ammo extension은 `Payload.AmmoTags[]` element physical contract/probe와 scalar SoftObject target-class Source signature를 필수로 했다. current DACE가 Array element JSON kind와 scalar Object target-class ReflectedTypePath를 아직 표현하지 못하는 실제 공백을 Source 재대조로 확인했다.
- Ammo persisted exact2는 referencer0 Test-owned durable fixture, Product canonical exact0, known unmanaged Product exact0으로 동결했다. Product exact0 상태에서도 P0-05 integration PASS가 가능하며 가짜 Product asset을 만들지 않는다.
- 이번 단계는 문서/설계 교정만 수행했다. C++/Build/Automation/JSON/.uasset/accepted snapshot mutation은 0이고 CF-FQ-039 Active, 기존 병렬 dirty, Missile Product/accepted baseline을 보존했다.

### v0.1.1 - 2026-09-09

- `DAO-P0-00 Initial Design Review`를 current Missile Staging/Apply/Ops/DACE, AmmoData Source/contract tests, Data Asset Manager와 fresh persisted dependency evidence 기준으로 수행해 `P0 0 / P1 6 / P2 2 / Implementation HOLD`로 판정했다.
- P1은 multi-type transport/Public compatibility, trusted TypeKey/provider/root/history authority, class-scoped StableLogicalId duplicate namespace, authored numeric vs runtime clamp 분리, AmmoTags set-like + DACE array-element coverage, AmmoIcon Soft Reference + DACE target-class drift coverage다.
- P2는 DisplayName/FamilyId optional semantic 명시와 현재 Ammo target classification 명확화다. fresh AssetDump에서 persisted exact2는 모두 referencer 0이므로 Test-owned durable fixture exact2 / Product canonical exact0 방향을 권고했다.
- 이번 review는 문서/읽기 전용이며 Source/JSON/.uasset/accepted snapshot mutation 0, Build/Automation 0이다. `DAO-P0-01`은 HOLD하고 exact next를 `DAO-P0-00 Correction + Re-review`로 변경했다.

### v0.1.0 - 2026-09-09

- 사용자 요청에 따라 `CF-FQ-051 Data Asset Multi-Type Onboarding`을 정식 P2 / Ready Plan으로 신규 승격했다.
- UE Source를 감사해 `CFAmmoData` authored exact8, 기존 Data Asset Manager의 AmmoId/IsAmmoDataValid identity·validation, 현재 Missile-specific Staging/Ops/Apply/DACE 결합을 확인했다.
- Fresh AssetDump에서 persisted `CFAmmoData` exact2를 확인하고 현재 값과 Test path ownership을 기록했다. 두 asset 모두 AmmoTags empty / AmmoIcon None이므로 non-empty array/non-null soft reference는 synthetic durable fixture로 검증하도록 고정했다.
- 두 번째 Pilot은 AmmoData로 확정하되 generic Reflection writer를 금지하고 shared orchestration + typed adapter/provider 구조를 목표로 했다. Missile accepted bootstrap과 Product exact3는 보존한다.
- 단계는 DAO-P0-00~06으로 구성하고 exact first gate를 `DAO-P0-00 Architecture / Contract Freeze + Initial Design Audit`으로 지정했다. 구현은 아직 시작하지 않았다.

## 14. Migration

- v0.3.22 Final Audit Correction + Re-review에서 Current owner를 `DataAssetAuthoring.md v1.4.1`로 전진했다. AmmoData는 Current 지원 exact2 중 하나이며 DamageData/VehicleSensorData/기타 신규 타입만 별도 onboarding 대상이다. final verdict는 `P0 0 / blocking P1 0 / P2 1 non-blocking / PASS`다.
- v0.3.21에서 DAO-P0-06 reuse measurement/acceptance와 Current System Promotion을 완료해 CF-FQ-051을 Technical Complete / Historical + Retained Path로 전환했다. MissileGuidePreset+AmmoData exact2 Current 계약은 `DataAssetAuthoring.md v1.4.0`이 소유한다. third type는 provider registration + explicit admission과 타입 전용 adapter/DACE만 추가하며 shared orchestration을 복제하지 않는다.
- v0.3.20에서 DAO-P0-05 Post-Implementation Mid-review가 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫혀 Technical Acceptance가 완료됐다. exact2 admission list는 auto-onboarding 방지 policy이며 third type 추가 시 shared operational algorithm 재작성 없이 provider 등록 + explicit admission으로 확장한다. 다음 Gate는 DAO-P0-06 Ready / Not Started이고 Current System Promotion 완료 전 AmmoData 최종 지원 승격을 주장하지 않는다.
- v0.3.19에서 DAO-P0-05 implementation + fresh validation이 PASS했다. current operational session은 empty/Missile-only compatibility와 Ammo-containing non-empty mixed Explicit Paths를 분리하며 JSON read 전 exact path owner를 확정한다. Product canonical Ammo exact0, sequential/PartialApplied semantics와 protected exact10을 유지한다. 다음 Gate는 `DAO-P0-05 Post-Implementation Mid-review`이며 PASS 전 DAO-P0-06으로 승격하지 않는다.
- v0.3.17에서 DAO-P0-05는 `P0 0 / blocking P1 2 / P2 1 / HOLD`다. `DAO-P0-05 Contract Correction + Re-review`에서 Product exact0 mutation0 lane과 test-owned Review/TOCTOU lane을 분리하고, trusted provider root ownership 기반 provider-neutral mixed operational discovery/selection과 non-atomic mixed Apply semantics를 동결한 뒤 P0/P1 0을 재확인하기 전 구현하지 않는다. protected exact10과 Product canonical Ammo exact0을 유지한다.
- v0.3.16부터 DAO-P0-04는 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`다. Ammo production fingerprint token omission은 shared production probe + independent expected manifest로 fail-closed하고, accepted Ammo exact1은 `CFDAAmmoDaceBase.cpp` dedicated append-only authority가 소유한다. 이후 DACE 보호 감사에는 이 파일을 포함한 protected exact10을 사용한다. 다음 exact Gate는 `DAO-P0-05 — Canonical Ammo Staging Pilot / Multi-Type Integration`이며 아직 Not Started다.
- v0.3.15 Mid-review 결과 DAO-P0-04는 `P0 0 / blocking P1 1 / P2 1 / HOLD`다. 현재 구현과 accepted bootstrap exact1 자체는 유지하지만 fingerprint exact token-coverage P1과 accepted Ammo history dedicated append-only owner separation P2를 `DAO-P0-04 Correction + Re-review`에서 닫기 전에는 Technical Acceptance 또는 DAO-P0-05 진입을 허용하지 않는다.
- v0.3.14부터 Ammo independent DACE descriptor/probe/history implementation과 `DACE-AmmoData` bootstrap exact1은 current Source에 존재하며 fresh Build + DAO13 + existing Missile DACE15가 PASS했다. Ammo provider DACE readiness는 `ContractReady`, Product canonical Ammo는 exact0이다. 다만 Post-Implementation Mid-review 전에는 DAO-P0-04 Technical Acceptance 또는 DAO-P0-05 진입을 허용하지 않는다.
- v0.3.13부터 DAO-P0-04 shared DACE correction은 Technical PASS이며 actual Ammo DACE implementation Gate가 열렸다. 다음 단계는 `DACE-AmmoData` provider-local descriptor/probe/accepted bootstrap만 구현하며, Ammo가 ContractReady로 승격되기 전에는 accepted append/Current promotion을 허용하지 않는다. Product canonical Ammo exact0은 그대로 유지한다.
- v0.3.12부터 DAO-P0-04 구현은 `per-TypeKey DACE provider/history/revision authority`, `AmmoTags[] element observation`, `AmmoIcon SoftObject target-class Reflection`, `per-TypeKey canonical Staging target set + Ammo exact0 semantics`의 blocking P1 4건을 먼저 교정하고 재검수에서 P0/P1 0을 확인한 뒤에만 시작한다. Missile accepted history와 DACE15 compatibility surface는 그대로 보존한다.
- v0.3.11에서 DAO-P0-03 Reviewed durable writer는 Correction + Re-review Technical PASS로 승인됐다. MissileGuidePreset/AmmoData exact2는 `ReviewedMutationReady`이고 durable sequencing은 `CFDADurableCore` 단일 authority를 사용한다. 다음 DAO-P0-04는 Ready / Not Started이며 Ammo DACE 구현/acceptance로 확대 해석하지 않는다.
- v0.3.10에서 DAO-P0-03은 구현과 prior fresh validation을 유지하지만 Mid-review `blocking P1 1 / P2 1` 때문에 Technical Acceptance는 HOLD다. 다음 단계는 Current Systems §10의 stale Missile-only contract와 Ammo provider readiness 잔여 주석을 교정한 뒤 Re-review하는 것이며, PASS 전 DAO-P0-04 DACE를 시작하지 않는다.
- v0.3.9부터 durable Create/Update/Save/reload/readback/rollback sequencing은 `CFDADurableCore` 단일 authority를 사용한다. typed provider는 자기 payload의 parse/fingerprint/extract/materialize 의미를 소유하며 durable transaction algorithm을 타입마다 복제하지 않는다. Ammo provider는 `ReviewedMutationReady`지만 Product canonical Ammo는 exact0이고, DAO-P0-03 Post-Implementation Mid-review PASS 전 DAO-P0-04 DACE로 넘어가지 않는다.
- v0.3.8에서 DAO-P0-02 Correction + Re-review가 Technical PASS했다. 새 provider는 common envelope `TargetObjectPath`와 `BaseSemanticFingerprint` 규칙을 provider-local로 복제하지 않고 `CFDACommonPrimitives` authority를 재사용한다. DAO-P0-03은 Ready지만 아직 writer/materializer/save가 구현된 상태는 아니다.
- v0.3.7의 Post-Implementation Mid-review 이후 DAO-P0-02는 Technical Acceptance HOLD다. third provider 전에 common envelope parsing/validation authority를 정리해야 하며 `TargetObjectPath`와 `BaseSemanticFingerprint`의 타입별 복제를 새 provider 패턴으로 확산하지 않는다. correction/re-review PASS 전 DAO-P0-03 writer를 시작하지 않는다.
- v0.3.6부터 Production TypeKey registry에는 MissileGuidePreset `ReviewedMutationReady`와 AmmoData `ReadOnlyPreviewReady`가 공존한다. 이는 Ammo durable Apply/DACE Current 지원 승격을 뜻하지 않으며 DAO-P0-02 Post-Implementation Mid-review와 후속 DAO-P0-03/04 Gate가 남아 있다. Ammo provider는 `ApplyReviewedMutation=nullptr` 상태를 유지한다.
- v0.3.5에서 DAO-P0-02 correction Source/Contract의 fresh current binary 검증을 완료했다. `carfight.editor.development` legacy build preset은 configured repository ID `main_game`을 사용하며 Project Registry 내부 identity `carfight_repo`를 legacy repository ID로 혼용하지 않는다. Official Build + affected13 + DACE15이 current correction Source로 PASS했으므로 다음 exact gate는 Ammo typed schema/parse/fingerprint/preview 구현이다. Production 정식 Typed Provider는 구현 시작 전 현재 MissileGuidePreset exact1이다.
- 기존 Public Missile API는 CF-FQ-051 동안 compatibility facade로 보존하며 새 타입을 추가하기 위해 이 API를 arbitrary generic payload API로 리네이밍·확장하지 않는다.
- AmmoData는 DAO-P0-06 Current System Promotion을 통과해 Current 지원 타입으로 확정됐다. 이후 DamageData/VehicleSensorData/기타 신규 DataAsset 타입은 자동 지원으로 간주하지 않고 별도 provider-centric lifecycle에서 onboarding한다.
- 기존 Missile Staging/DACE API와 accepted history는 P0-01 parity evidence 없이 리네이밍·삭제·재baseline하지 않는다.
