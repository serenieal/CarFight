# CarFight Data Asset Staging / Batch Authoring

- 문서 버전: v1.4.1
- 최근 갱신일: 2026-09-10
- 문서 상태: Current
- 완료 Feature: `CF-FQ-049 Data Asset Staging·Batch Authoring` + `CF-FQ-050 Data Asset Contract Evolution Guard` + `CF-FQ-051 Data Asset Multi-Type Onboarding`
- Current extension checkpoint: `CF-FQ-051 Complete / DAO-P0-06 Final Audit Correction + Re-review PASS / P0 0 / blocking P1 0 / P2 1 non-blocking`
- 현재 구현 범위: `CFMissileGuidePresetData` typed Staging / Preview / Reviewed Apply + Contract Evolution Guard P0 + Editor Private shared Type Dispatch + shared typed durable core / Production provider MissileGuidePreset + AmmoData exact2 `ReviewedMutationReady` / AmmoData independent DACE descriptor + production probe + `DACE-AmmoData` bootstrap exact1 / provider-neutral non-empty mixed Explicit Paths operational session implemented
- Current System owner: 이 문서 + 실제 `UE/Source/CarFight_ReEditor/Public/DataAuthoring/` + `UE/Source/CarFight_ReEditor/Private/DataAuthoring/` Source

---

## 1. 문서 목적

이 문서는 CarFight에서 DataAsset을 매번 Unreal Editor 안에서 한 건씩 직접 만들고 수정하는 대신, **Editor가 꺼져 있을 때 JSON Staging 파일에 저작 의도를 누적하고, Editor가 필요할 때 검토한 exact 항목만 안전하게 Unreal DataAsset으로 반영하는 현재 P0 저작 시스템**을 설명한다.

이 시스템은 Unreal DataAsset의 Source of Truth를 JSON으로 이전하지 않는다.

```text
Editor OFF
AI / USER
  ↓
Staging JSON 작성·수정
  ↓
Git diff / 텍스트 리뷰

Editor ON
  ↓
Typed Parse / Validation
  ↓
Preview
  ↓
Review
  ↓
명시적 ApplyReviewed
  ↓
Exact DataAsset Package Save
  ↓
Disk Reload + Typed Semantic Readback
```

정상 적용이 끝난 뒤 최종 Source of Truth는 계속 persisted Unreal `.uasset`이다.

---

## 2. 현재 P0 범위

현재 Production registry에 등록된 Typed Provider는 **MissileGuidePreset + AmmoData exact2**이며 둘 다 `ReviewedMutationReady`다.

```text
UCFMissileGuidePresetData
SchemaId: CarFight.DataAsset.MissileGuidePreset
SchemaRevision: 1
AdapterContractRevision: 2
ClassPath: /Script/CarFight_Re.CFMissileGuidePresetData
Readiness: ReviewedMutationReady

UCFAmmoData
SchemaId: CarFight.DataAsset.AmmoData
SchemaRevision: 1
AdapterContractRevision: 1
ClassPath: /Script/CarFight_Re.CFAmmoData
Readiness: ReviewedMutationReady
```

MissileGuidePreset의 현재 canonical Product Pilot은 다음 세 개다.

```text
MissileFeel_Low
MissileFeel_Normal
MissileFeel_High
```

canonical Staging 경로:

```text
Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json
Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Normal.json
Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_High.json
```

`CFAmmoData`는 CF-FQ-051을 통해 Current 지원 타입으로 승격됐다. `CFDamageData`, `CFVehicleSensorData` 및 그 밖의 신규 DataAsset 타입은 자동 지원되지 않으며, 각각 별도 후속 lifecycle에서 typed provider/schema/DACE와 가치·공수를 검토한 뒤 onboarding한다.

### 2.1 Shared Type Dispatch Foundation — CF-FQ-051 DAO-P0-01

Current Editor Private authoring source에는 multi-type onboarding을 위한 Type Dispatch foundation이 존재하며 `DAO-P0-01 Correction + Re-review`에서 **P0 0 / blocking P1 0 / P2 0 Technical PASS**로 승인됐다.

```text
CFDATypeDispatch
= payload-free common envelope
= exact SchemaId + DataAssetTypeClassPath trusted TypeKey registry
= provider-owned canonical StagingRoot validation
= class-scoped StableLogicalId duplicate key

CFDAMissileProvider
= current first typed provider
= schema/class/revision/root/Stable Identity authority
= parse / fingerprint / extract / current-state / serialize / materialize callback ownership
```

registry와 provider implementation 선택권은 production code가 소유한다. JSON/DTO가 arbitrary callback이나 implementation 이름을 지정하지 않는다.

현재 Production registry의 provider entry는 **MissileGuidePreset `ReviewedMutationReady` + AmmoData `ReviewedMutationReady` exact2**다. 각 exact TypeKey entry는 descriptor와 readiness에 맞는 shared-safe operation을 함께 소유하고 Stable Identity policy/resolver와 DACE owner/history namespace authority까지 결속한다. AmmoData도 DAO-P0-03에서 exact typed materializer와 `ApplyReviewedMutation`을 등록했으며 shared Review/TOCTOU와 provider-neutral durable core를 통과한 경우에만 mutation할 수 있다. 기존 Public `FCFDAStagingRecord`, `FCFDAStagingService`, `FCFDAStagingOps`, `FCFDAStagingApplyService` Missile API는 compatibility facade로 유지된다.

Shared Preview/duplicate/BatchPlanHash/Reviewed Approval/TOCTOU는 payload-free common row에서 실행하고, typed record/payload는 provider-local parse/extract/materialize 경계 안에만 유지한다. Apply는 reviewed source를 fresh re-read한 뒤 exact TypeKey provider entry로 parse/current resolve/materialize를 dispatch하며 shared core가 Missile typed payload를 보관하지 않는다.

StableLogicalId duplicate namespace는 `exact DataAsset class + FName semantic`이다. 따라서 다른 DA class가 같은 textual ID를 쓰는 것은 namespace 충돌이 아니지만 같은 class의 FName-equivalent ID는 duplicate다. `TargetObjectPath` duplicate는 계속 type과 무관한 global blocker다.

Staging source는 selected provider가 소유하는 exact canonical root 아래에 있어야 한다. 기존 sibling Automation fixture는 `Authoring/DataAssetStaging/MissileGuidePreset/__Automation...__/` provider root 하위로 이동했고 runner cleanup/residue gate도 동일 root로 정렬했다. Production validator의 Missile-specific sibling-root allowlist는 제거되어 Development Editor에서도 provider-owned root trust boundary를 유지한다.

`CFAmmoData`의 exact8 typed payload/schema/parser/fingerprint/extractor/current-state resolver는 DAO-P0-02에서 구현됐고, DAO-P0-03에서 exact8 deterministic materializer와 Reviewed mutation callback을 추가했다. Missile과 Ammo 모두 `CFDADurableCore`의 Create/Update, pre-save typed readback, exact single-package SavePackage, disk reload, post-save typed readback, rollback/uncertainty sequencing을 재사용한다. DAO-P0-04 Contract Correction + Re-review에서 per-TypeKey DACE authority, `AmmoTags[]` element observation, `AmmoIcon` SoftObject target-class Reflection, per-TypeKey canonical Staging target-set / Ammo exact0 contract가 **P0 0 / blocking P1 0 / P2 0 Technical PASS**로 승인된 뒤, 실제 Ammo DACE 구현에서 `CFDAAmmoDace`가 SourceShape/AdapterShape/SourceAdapterMapping/SemanticContract와 independent `DACE-AmmoData-S1-A1-Bootstrap` exact1을 소유한다. Ammo provider의 DACE readiness는 `ContractReady`로 승격됐고 Product canonical Ammo는 explicit exact0을 유지한다. production serializer→parser→fingerprint→transient materialize→extract probe, `AmmoTags[]` wrong-type/duplicate/order-equivalence, `AmmoIcon` UTexture2D target-class drift/no-load, old revision과 cross-TypeKey history isolation이 fresh DAO exact13에서 PASS했다. DAO-P0-04 Post-Implementation Mid-review의 blocking P1 1건과 P2 1건은 Correction + Re-review에서 모두 닫혔다. Production fingerprint는 기존 shared `CFDACommonPrimitives::FScopedSemanticTokenProbe`로 실제 production token emission을 관측하고 test-owned independent expected manifest와 exact 비교한다. non-null icon + AmmoTags exact2 fixture는 schema/revisions/class + Ammo exact8 semantic을 포함한 token 15개를, null icon fixture는 `Payload.AmmoIcon.Path`를 제외한 token 14개를 요구하므로 future `AppendPayloadTokens()` omission이 self-consistency만으로 통과할 수 없다. `DACE-AmmoData-S1-A1-Bootstrap` exact1은 값/signature를 변경하지 않고 dedicated append-only `CFDAAmmoDaceBase.cpp`로 분리했으며 current descriptor/migration authority인 `CFDAAmmoDace.cpp`와 물리적으로 분리했다. fresh Build + DAO13 + existing Missile DACE15와 independent Source re-review 결과 **P0 0 / blocking P1 0 / P2 0 / Technical PASS**로 DAO-P0-04를 닫았고, 당시 다음 Gate가 `DAO-P0-05 — Canonical Ammo Staging Pilot / Multi-Type Integration`이었다.

### 2.2 Multi-Type Operational Explicit Paths — DAO-P0-05

DAO-P0-05 구현 이후 Current operational session은 기존 Missile compatibility surface를 유지하면서 실제 registered MissileGuidePreset + AmmoData provider exact2를 같은 non-empty explicit selection에서 다룰 수 있다.

```text
FCFDAStagingOpsSession::Preview(empty)
= 기존 Missile whole-root discovery

FCFDAStagingOpsSession::Preview(non-empty / Missile-only)
= 기존 typed Missile exact selection discovery

FCFDAStagingOpsSession::Preview(non-empty / Ammo 포함)
= provider-neutral mixed Explicit Paths discovery
```

Mixed path의 authority는 JSON payload가 아니라 code-owned production registry의 provider root다. `CFDATypeDispatch::FindProviderForStagingPath()`는 JSON read 전에 full registered provider set의 `CanonicalStagingRoot` containment를 검사하고 owner exact1을 요구한다. owner 0, owner >1 또는 caller가 명시한 allowed TypeKey scope 밖의 owner는 fail-closed한다. Allowed scope를 먼저 필터링하지 않기 때문에 overlapping trusted root ambiguity를 숨기지 않는다.

```text
Exact input path
  ↓
full production provider-root containment
  ↓
owner exact1
  ↓
allowed TypeKey scope check
  ↓
JSON read
  ↓
owner provider ParseCommonCandidate
  ↓
ValidateProviderContract
  ↓
owner provider ResolveCommonCurrentState
  ↓
shared payload-free Preview / duplicate / hash / Review core
```

따라서 payload-first TypeKey dispatch, parent `Authoring/DataAssetStaging` recursive trust와 raw provider registry exposure는 Current 계약에 존재하지 않는다. `Review()`는 Preview가 사용한 동일 normalized path set과 동일 discovery mode를 fresh replay한 뒤 existing common Reviewed approval을 만든다.

Console StableLogicalId shorthand는 기존 Missile canonical path 변환 의미를 유지하며 Ammo shorthand로 암묵 확장하지 않는다. `SyncProductStaging`도 Missile Product Low/Normal/High canonical exact3만 다루는 기존 behavior를 유지한다. Product canonical Ammo는 explicit exact0이며 mixed support를 증명하기 위해 fake Product Ammo asset/Staging을 만들지 않는다.

Mixed `ApplyReviewed`는 새 transaction model을 만들지 않는다. Existing shared Apply의 `approval 전체 global preflight → target별 immediate TOCTOU → sequential durable apply` 순서를 그대로 사용한다. 따라서 후속 target 실패 시 `PartialApplied`가 가능하며 cross-target all-or-nothing atomicity는 Current 목표가 아니다.

DAO-P0-05 actual-provider regression은 provider-local `__AutomationP05__` Staging과 `/Game/Test/CarFight/DAOP05/` disposable Content root만 사용했다. 같은 textual StableLogicalId의 Missile 1 + Ammo 1을 한 Review/Apply에서 durable exact2로 저장하고, cross-TypeKey 동일 TargetObjectPath는 global duplicate로 차단하며 source/current stale state를 mutation0으로 차단하는 것을 검증했다.

최종 fresh evidence는 다음과 같다.

```text
Official UE 5.8 Build
39e4973ff0ad4e31803c679d272533ff
PASS

DAO-P0-05 actual-provider focused exact4
148c4f325b724b2fa7bfd7158819c531
4/4 PASS

Affected CF-FQ-049 exact13
0f2f69983f024986aa87a98afee2aed9
13/13 PASS

Existing Missile DACE exact15
8967abe39f1a4d128c3ce455ede970a5
15/15 PASS
```

Protected exact10 scoped diff는 0이며 `CFDAAmmoDaceBase.cpp`의 `DACE-AmmoData-S1-A1-Bootstrap` exact1 fixed field/signature readback도 변경이 없다. Fresh AssetDump에서 `/Game/Test/CarFight/DAOP05` Ammo residue는 exact0이고 전체 persisted `CFAmmoData`는 기존 HeavyFinite/RocketFinite exact2만 존재한다. Product canonical Ammo exact0, Missile Product exact3 Apply·Save 0과 CF-FQ-039 Active lifecycle도 유지한다.

현재 이 구현은 **DAO-P0-06 Current System Promotion까지 Technical Accepted**다. MissileGuidePreset + AmmoData exact2는 Production authoring/guard Current 지원 타입이며, 새 타입은 typed provider 중심으로 추가한다.

### 2.3 Reuse Measurement / Third-Type Extension Contract — DAO-P0-06

DAO-P0-06은 MissileGuidePreset→AmmoData 두 번째 onboarding의 실제 Current Source를 다시 측정해 **금지된 shared algorithm 복제 0 / third-type core algorithm rewrite 0**으로 승인했다.

측정은 현재 physical Source line을 개발 시간으로 오해하지 않고 구조적 반복량 지표로만 사용한다. legacy Missile compatibility와 generic implementation이 같은 파일에 공존하는 영역은 dedicated shared foundation과 분리해서 기록한다.

| 측정 항목 | 결과 | 해석 |
| --- | ---: | --- |
| dedicated shared foundation | exact6 / 2300 physical lines | `CFDACommonPrimitives.*`, `CFDATypeDispatch.*`, `CFDADurableCore.*` |
| hybrid shared + Missile compatibility production surface | exact7 / 6363 physical lines | `CFDAContractGuard.*`, `CFDAStagingOps.cpp`, `CFDAStagingApply.cpp`, Public Ops/Apply headers, `CFDAStaging.cpp`; 전부를 pure shared LOC로 합산하지 않는다 |
| production framework-impact surface | exact13 files | dedicated6 + hybrid7; 물리적 영향 surface이며 타입별 신규 구현량과 중복 합산하지 않는다 |
| Ammo-specific production | exact5 / 1789 physical lines | `CFDAAmmoProvider.*`, `CFDAAmmoDace.h/.cpp`, append-only `CFDAAmmoDaceBase.cpp` |
| Missile parity/compatibility surface reviewed | exact9 files | hybrid7 + `CFDAMissileProvider.*`; framework 분류와 겹치므로 별도 합산 금지 |
| CF-FQ-051 feature-owned Automation | exact17 | P0-02 4 + P0-03 3 + P0-04 correction 2 + Ammo DACE 4 + P0-05 4 |
| prohibited shared algorithm duplication | 0 | Preview/approval/TOCTOU/hash/aggregation/rollback/durable/DACE chain·migration 복제 없음 |
| third-type shared core algorithm rewrite | 0 required | provider registration + explicit operational admission만 shared configuration touch로 요구 |

AmmoData가 타입 전용으로 새로 소유하는 것은 payload/field schema, parse/serialize, extract/materialize, field semantic, stable identity/current-state resolver, DACE descriptor/probe와 accepted history다. 반대로 다음 orchestration은 shared authority를 그대로 재사용한다.

```text
Preview / duplicate / BatchPlanHash
Reviewed approval lifecycle
source/current TOCTOU sequencing
transaction result aggregation
rollback / save uncertainty state machine
durable Create/Update/Save/reload/readback orchestration
DACE signature / Reflection coverage / accepted-chain integrity
revision guard / canonical Staging compatibility / migration gate
```

따라서 세 번째 DataAsset 타입은 최소 다음 순서로 추가한다.

```text
1. 타입 전용 payload/schema/parser/serializer/extract/materialize/current-state 구현
2. exact TypeKey production provider 등록
3. DACE descriptor/probe + 독립 append-only accepted history 구성
4. mixed operational explicit admission에 TypeKey 추가
5. 타입 전용 focused regression + 기존 shared affected regression
```

`GetMixedOperationalAllowedTypeKeys()`의 explicit list는 payload별 orchestration hard-code가 아니라 새 provider가 자동으로 production mutation lane에 들어오지 못하게 하는 admission policy다. 새 타입 추가 시 이 목록을 명시적으로 수정해야 하지만 Preview/Review/TOCTOU/Apply/Durable/DACE 알고리즘을 다시 쓰지 않는다.

사전 Acceptance/Measurement 독립 검수 결과는 **P0 0 / blocking P1 0 / P2 1 / PASS**다. P2 1건은 generic implementation 일부가 `CFDAStaging.cpp`, `CFDAContractGuard.*` 등 legacy Missile compatibility owner와 물리적으로 혼재하는 유지보수 부채다. 현재 correctness나 third-type onboarding을 막지 않으므로 선제 리팩터링하지 않는다. 향후 DamageData onboarding에서 같은 hybrid owner의 반복 수정·충돌이 실제로 발생할 때만 dedicated shared file extraction을 별도 maintenance/scaffold Feature로 검토한다.

Current Promotion은 기존 executable Source를 변경하지 않는 read-only measurement + document projection 작업이다. 따라서 직전 accepted executable evidence를 반복 실행하지 않고 유지한다: official UE 5.8 Build `39e4973ff0ad4e31803c679d272533ff`, DAO-P0-05 focused 4/4 `148c4f325b724b2fa7bfd7158819c531`, affected13 `0f2f69983f024986aa87a98afee2aed9`, Missile DACE15 `8967abe39f1a4d128c3ce455ede970a5`. Existing CF-FQ-049 OperationalEntry 1/1과 CF-FQ-045 Ammo typed identity/health baseline도 해당 Current owner의 Accepted regression으로 유지한다. P0-06은 Runtime/Ammo Source를 변경하지 않았고 `UCFAmmoData::IsAmmoDataValid() = AmmoId != NAME_None`의 Runtime health 의미를 authoring exact8 validity로 확대하지 않았으므로 추가 runtime/fitting 회귀는 요구하지 않는다.

`CFVehicleData`는 이 경로의 일반화 대상이 아니다. Vehicle Builder / Recipe / Profile / Definition Apply 계약을 계속 사용한다.

---

## 3. Authority / Source of Truth

### 3.1 최종 Source of Truth

```text
Staging JSON
= 작성 중인 의도 / 검토 가능한 Batch 입력

Persisted .uasset
= Apply 완료 후 최종 Unreal Source of Truth
```

Cooked Game과 Runtime은 Staging JSON을 읽어 게임플레이 데이터를 구성하지 않는다.

### 3.2 Staging은 silent overwrite 권한이 아니다

Staging baseline 이후 Unreal Asset이 직접 변경됐거나 현재 값과 baseline 관계가 안전하지 않으면 자동 덮어쓰지 않는다.

Preview는 최소 다음 상태를 사용한다.

| 상태 | 의미 |
| --- | --- |
| `Create` | Target Asset이 없고 신규 생성 조건이 유효함 |
| `Update` | Target Asset이 있고 안전한 whole-record 변경 후보가 있음 |
| `NoChange` | Staging과 current Unreal semantic 값이 동일함 |
| `Conflict` | baseline/current/staging 관계상 안전한 자동 반영을 결정할 수 없음 |
| `Invalid` | schema, type, identity, path 또는 값 검증 실패 |

### 3.3 Data Asset Manager와의 경계

`CF-FQ-045`의 `DataAssetManagement.md`는 기존 DataAsset을 발견·분류·검사·참조 조회하는 **read-first 관리 시스템**이다.

CF-FQ-049는 별도 Editor-side authoring/write owner다.

```text
DataAssetManagement
= 발견 / 관리 / 검사 / 참조 조회

DataAssetAuthoring
= Staging / Preview / Review / exact typed Apply
```

Data Asset Manager가 CF-FQ-049의 SavePackage 권한을 흡수하지 않는다.

---

## 4. Canonical JSON 계약

MissileGuidePreset P0 JSON은 whole-record strict JSON이다.

필수 top-level field는 정확히 다음 8개다.

```text
SchemaId
SchemaRevision
AdapterContractRevision
DataAssetTypeClassPath
StableLogicalId
TargetObjectPath
BaseSemanticFingerprint
Payload
```

Unknown field 또는 required field 누락은 `Invalid`다.

Update에는 `BaseSemanticFingerprint`가 필수다. Create는 target이 실제로 없을 때 baseline 부재를 허용한다.

현재 identity 계약:

```text
StableLogicalId == Payload.PresetId
```

Typed representation의 주요 규칙:

```text
FName
= JSON string
= semantic 비교는 FName 의미 기준

FText
= JSON interchange는 { "Kind": "Literal", "Text": "..." }
= source-backed Literal의 문자열 의미만 authored semantic으로 취급
= Unreal 저장 과정에서 자동 부여된 package-only namespace / stable key는 persistence metadata로 보고 semantic identity에서 제외
= StringTable, 명시적 authored namespace localization identity, source string을 lossless하게 보존할 수 없는 generated/formatted FText는 P0에서 fail-closed

enum
= reflected exact source token

bool
= JSON boolean

float
= finite authored numeric value

nested struct
= strict whole-record object
```

P0 Update는 partial patch가 아니라 **whole-record typed replacement**다.

---

## 5. Semantic Fingerprint

Staging 안전성은 raw JSON byte 비교만으로 결정하지 않는다.

Typed writable payload를 canonical semantic token으로 변환한 뒤 SHA-256 fingerprint를 사용한다.

```text
sha256:<64 lowercase hex>
```

fingerprint에는 다음 의미가 포함된다.

```text
SchemaId / SchemaRevision / AdapterContractRevision
DataAssetTypeClassPath
Typed writable payload
```

JSON field ordering과 whitespace는 semantic 변경이 아니다.

Runtime clamp나 transient effective value는 authored semantic fingerprint의 기준이 아니다.

---

## 6. 3-way Preview 계약

Update의 핵심 값은 다음 세 개다.

```text
B = BaseSemanticFingerprint
C = Current Unreal semantic fingerprint
S = Staging semantic fingerprint
```

현재 분류 계약:

```text
B == C == S
→ NoChange

B == C, S != C
→ Update

B != C, C == S
→ NoChange + BaselineRebaseRequired 안내

B != C, S == B
→ Conflict / BaselineMismatch

B, C, S 모두 다름
→ Conflict / BaselineMismatch
```

Create는 baseline이 없고 target/identity가 실제로 없을 때만 `Create`다.

주요 fail-closed 진단:

```text
UnexpectedExistingTarget
TargetMissing
TargetMoved
PathCollision
TargetDirtyUnowned
DuplicateStableIdentity
DuplicateTargetPath
ApprovalStale
```

기존 target package가 Apply 전에 dirty이면 사용자의 미저장 변경을 대신 저장하지 않고 `TargetDirtyUnowned` 의미로 차단한다.

---

## 7. Exact Selection 운영 계약

P0 운영 entry는 canonical folder 전체뿐 아니라 **이번 Batch에서 실제 검토할 exact JSON 선택 목록**을 지원한다.

선택 목록은 canonical repository-relative path로 normalize하고 중복을 차단한 뒤 deterministic sort한다.

empty selection만 전체 canonical MissileGuidePreset JSON discovery를 의미한다.

따라서 선택하지 않은 다른 JSON의 `Invalid` 또는 `Conflict`가 이번 exact Batch의 Review를 불필요하게 막지 않는다.

---

## 8. 현재 운영 흐름

### 8.1 최초 baseline 생성 또는 명시적 rebase

Editor가 current Product `.uasset` truth를 읽을 수 있는 상태에서 다음 console entry를 사용한다.

```text
CarFight.DAStaging.SyncProduct
```

이 명령은 Product Low/Normal/High current semantic 값을 canonical JSON baseline으로 bootstrap/rebase한다.

중요한 제한:

- Product `.uasset`을 저장하지 않는다.
- Product package가 dirty이면 실행을 차단한다.
- 기존 JSON에 사용자 편집 또는 drift가 있으면 자동 덮어쓰지 않는다.
- existing Staging이 current Product와 이미 `NoChange`로 수렴한 경우에만 canonical rewrite/rebase를 허용한다.
- 여러 Staging file 중 후반 write/readback 검증이 실패하면 이번 호출에서 touched된 file을 호출 전 raw bytes/absence 상태로 rollback한다.

### 8.2 Editor가 꺼진 동안 저작

Sync로 baseline이 준비된 뒤 JSON 자체는 Editor가 없어도 수정할 수 있다.

이 단계에서는 `.uasset`을 건드리지 않는다.

### 8.3 Preview

전체 canonical folder:

```text
CarFight.DAStaging.Preview
```

StableLogicalId exact selection 예:

```text
CarFight.DAStaging.Preview MissileFeel_Low
```

여러 exact selection도 지원한다.

Preview는 mutation 0이다.

출력에서 확인할 핵심은 다음이다.

```text
Create / Update / NoChange / Conflict / Invalid
blocker 여부
BatchPlanHash
StableLogicalId
TargetObjectPath
StagingRelativePath
```

### 8.4 Review

```text
CarFight.DAStaging.Review
```

Review는 마지막 Preview의 **같은 selection**을 fresh discovery한다.

fresh `BatchPlanHash`가 직전 Preview와 exact 동일한 경우에만 Reviewed approval을 만든다.

Preview 뒤 JSON 또는 current Unreal truth가 바뀌면 stale approval을 만들지 않는다.

### 8.5 실제 Apply

```text
CarFight.DAStaging.ApplyReviewed
```

이 명령만 실제 reviewed exact target을 materialize/save할 수 있다.

실행 전에는 반드시 Preview와 Review 결과를 확인해야 한다.

CF-FQ-049 P0 Acceptance에서는 Product Low/Normal/High에 이 명령을 실행하지 않았다. 따라서 **CF-FQ-049 closure 기준 Product Low/Normal/High Apply·Save는 0**이다.

---

## 9. Reviewed Approval / TOCTOU 보호

Reviewed approval은 단순 버튼 상태가 아니라 exact evidence에 binding된다.

주요 binding 값:

```text
SchemaId
SchemaRevision
AdapterContractRevision
DataAssetTypeClassPath
StableLogicalId
TargetObjectPath
StagingRelativePath
BaseSemanticFingerprint
CurrentSemanticFingerprint
StagingSemanticFingerprint
PlannedOperation
BatchPlanHash
exact sorted target set
```

Apply 직전에는 disk Staging과 current Unreal truth를 다시 읽는 global preflight를 수행한다.

그 뒤 각 target mutation 직전에도 current truth를 한 번 더 재검증한다.

하나라도 달라지면 stale approval로 fail-closed한다.

approval은 one-shot이다. stale/blocker로 mutation 0 종료돼도 같은 approval을 자동 재사용하지 않는다.

---

## 10. Typed Materializer / Durable Writer

현재 materializer는 generic Reflection writer가 아니다. Production registry의 **MissileGuidePreset + AmmoData exact2 `ReviewedMutationReady` provider**가 각 타입의 payload 의미만 소유하고, durable transaction algorithm은 `CFDADurableCore` 단일 authority가 소유한다.

```text
MissileGuidePreset provider
- Missile typed materialize / extract / fingerprint

AmmoData provider
- Ammo exact8 typed materialize / extract / fingerprint

CFDADurableCore
- CreatePackage / exact typed NewObject
- existing exact target Update
- original typed snapshot
- typed whole-record apply
- pre-save typed semantic readback
- AssetRegistry::AssetCreated for Create
- MarkPackageDirty
- exact single-package SavePackage
- persisted package disk reload
- post-save typed semantic readback
- rollback / InMemoryStateUnconfirmed / SaveStateUnconfirmed sequencing
```

Create와 Update 모두 exact TypeKey provider의 Reviewed mutation callback을 통해 같은 shared durable core로 진입한다. provider는 `UCFAmmoData`나 `UCFMissileGuidePresetData` 같은 exact UObject 타입과 payload 의미를 전달하지만 Save/reload/rollback algorithm을 타입별로 복제하지 않는다.

`Save All`을 사용하지 않으며 unrelated package를 저장하지 않는다.

---

## 11. Durable Save 기준

`UPackage::SavePackage()`가 `true`를 반환한 것만으로 성공으로 인정하지 않는다.

`DurableApplied`가 되려면 다음을 모두 확인해야 한다.

```text
exact package SavePackage
→ package clean
→ persisted package 존재
→ 방금 저장한 exact package disk reload
→ exact object path 재resolve
→ reloaded package clean
→ typed payload 재추출
→ expected semantic fingerprint exact 일치
```

reload/readback 상태가 불확실하면 성공을 주장하지 않고 `SaveStateUnconfirmed`로 중단한다.

---

## 12. Batch 실패 의미

현재 target 결과 taxonomy:

```text
NotRun
NoChange
BlockedBeforeMutation
FailedBeforeDurableWrite
InMemoryStateUnconfirmed
DurableApplied
SaveStateUnconfirmed
PostCommitWarning
```

Batch 결과는 최소 다음을 구분한다.

```text
NoChange
DurableApplied
BlockedBeforeMutation
FailedBeforeDurableWrite
InMemoryStateUnconfirmed
SaveStateUnconfirmed
PartialApplied
PostCommitWarning
```

`PartialApplied`는 다음 조건에서만 사용한다.

```text
하나 이상의 target이 DurableApplied
+
후속 target이 known failed-before-durable
+
unknown state 없음
```

`SaveStateUnconfirmed` 또는 `InMemoryStateUnconfirmed`가 있으면 uncertainty 결과가 `PartialApplied`보다 우선한다.

Batch는 첫 실패에서 멈추며 자동 retry나 전체 global rollback을 약속하지 않는다.

---

## 13. Product Sync rollback

`SyncProduct`는 Unreal Asset write가 아니라 Staging text bootstrap/rebase 작업이지만, 여러 JSON을 순서대로 갱신할 수 있으므로 partial text write도 보호한다.

현재 구현은 Sync 시작 전에 각 Product Staging file의 다음 상태를 snapshot한다.

```text
file 존재 여부
raw bytes
```

write/readback/Product semantic verification 중 어느 단계에서 실패해도 이번 호출에서 touched된 file을 역순 복원한다.

복원 뒤 raw-byte exact readback까지 확인한다.

복원을 확인하지 못하면 rollback confirmed를 주장하지 않는다.

---

## 13.1 Contract Evolution Guard

`CF-FQ-050` 완료 이후 Staging 지원 DataAsset의 C++ authoring contract가 바뀔 때는 기존 typed Staging이 조용히 뒤처진 상태로 남을 수 없다.

현재 Pilot은 `CFMissileGuidePresetData` 한 타입이며, Guard는 Public/Blueprint API가 아니라 `CarFight_ReEditor` Private developer contract다.

### 13.1.1 Current machine-readable baseline

현재 MissileGuidePreset contract는 다음 네 descriptor 축으로 고정한다.

```text
SourceShape
= exact 30 node
= UCFMissileGuidePresetData top-level exact4 + FCFMissileGuideConfig authored exact26

AdapterShape
= strict JSON physical shape exact42 node

SourceAdapterMapping
= explicit mapping exact38 row

SemanticContract
= explicit semantic policy exact26 rule
```

각 descriptor는 case-sensitive ordinal deterministic order로 canonical row를 만들고 SHA-256 signature를 계산한다. Reflection만으로 semantic meaning 전체를 자동 추론하지 않으며, semantic policy 변화는 명시적 `SemanticContract` 변화로 관리한다.

현재 accepted baseline authority는 test fixture가 아니라 다음 production Editor Private source다.

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractBase.cpp
```

현재 history는 다음 bootstrap exact1이다.

```text
SnapshotId: DACE-MissileGuidePreset-S1-A2-Bootstrap
SchemaRevision: 1
AdapterContractRevision: 2
Accepted record count: 1
```

accepted history는 append-only다. 기존 record를 수정해서 새 contract를 승인하지 않는다.

### 13.1.2 Structural / behavior drift guard

Guard는 다음 current 계약의 불일치를 fail-closed한다.

```text
Native Reflection Source shape drift
Adapter JSON physical shape drift
Source↔Adapter mapping coverage drift
production serializer coverage mismatch
production strict parser coverage mismatch
production semantic fingerprint token mismatch
production materializer↔extractor semantic roundtrip mismatch
accepted snapshot chain/integrity mismatch
```

Descriptor 목록 자체만 맞춰 놓고 production implementation이 빠진 상태가 PASS하지 않도록 serializer/parser/fingerprint/extractor/materializer의 실제 production 경로를 developer-only probe로 검증한다.

### 13.1.3 Revision guard

Accepted latest와 current candidate가 다르면 `CurrentChangeDeclaration`이 latest accepted `BaseSnapshotId`와 Guard가 계산한 `CandidateContractSignature`에 exact binding되어야 한다.

```text
Adapter physical shape change
→ SchemaRevision bump 필요

Declared semantic contract change
→ AdapterContractRevision bump 필요

Source/Mapping-only change
→ revision을 무조건 올리지는 않음
→ 대신 explicit migration impact declaration은 필요
```

`NoMigration`은 Guard가 변화 종류만 보고 자동으로 safe라고 추론한 값이 아니다. 개발자가 명시적으로 판단하는 migration declaration이며, 더 보수적인 Product review impact를 선택할 수 있다.

현재 migration impact:

```text
NoMigration
StagingMigrationRequired
ProductMigrationReviewRequired
StagingAndProductMigrationReviewRequired
```

현재 resolution:

```text
NotRequired
Pending
Resolved
```

### 13.1.4 Migration / promotion gate

canonical Product Staging Low/Normal/High exact3은 Guard가 **read-only**로 읽어 current production strict parser와 revision compatibility를 검증한다.

old revision 또는 incompatible staging을 current 계약으로 silent reinterpret하지 않는다. 필요한 migration/review가 Pending이면 다음을 차단한다.

```text
new accepted snapshot append
Current System promotion
```

`Resolved`가 필요한 impact는 non-empty `MigrationEvidenceId`가 있어야 한다. Evidence 문자열의 존재는 machine gate이고, 실제 migration/review 내용의 충분성은 integration/acceptance review가 판단한다.

Guard는 다음 write authority를 소유하지 않는다.

```text
SyncProduct
ApplyReviewed
SavePackage
Product DataAsset mutation
canonical Product Staging mutation
```

따라서 contract 변화 감지는 Product를 자동 수정하거나 저장하는 기능이 아니다.

### 13.1.5 Contract 변경 시 운영 순서

Staging 지원 DA의 authored contract를 변경할 때는 다음 순서를 사용한다.

```text
Source / Adapter / Semantic 변경
→ DACE focused guard 실행
→ 필요한 SchemaRevision / AdapterContractRevision 결정
→ migration impact 명시
→ 필요 시 old Staging migration / Product review 수행
→ Resolution + Evidence 확정
→ Guard와 affected Staging regression PASS
→ 그 뒤에만 새 accepted snapshot append 검토
```

contract 변화가 없으면 accepted snapshot을 중복 append하지 않는다.

### 13.1.6 다른 DA 타입 onboarding 경계

CF-FQ-050 P0 완료로 **두 번째 Staging 지원 DA가 따라야 할 guard interface와 lifecycle 선행조건은 준비됐다.** 하지만 새 타입이 자동 지원되는 것은 아니다.

새 타입은 별도 lifecycle에서 최소 다음을 제공해야 한다.

```text
Typed Adapter / Schema
Source descriptor
Adapter descriptor
Source↔Adapter mapping
Semantic policy
accepted baseline
production behavior probe provider
focused + affected regression
```

두 번째 DA를 실제 onboarding한 뒤 반복 boilerplate와 공수를 측정하기 전에는 generic Reflection writer나 대형 scaffold framework로 확대하지 않는다.

---

## 14. Current Source 위치

Typed Staging / Preview core:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFDAStaging.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStaging.cpp
```

Reviewed Apply / provider-neutral durable save:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFDAStagingApply.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStagingApply.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDADurableCore.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDADurableCore.cpp
```

Operational entry / Product Sync / exact selection session:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFDAStagingOps.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStagingOps.cpp
```

Focused Automation:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStagingTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStagingApplyTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStagingPilotTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStagingOpsTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAAmmoProviderTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAAmmoApplyTests.cpp
```

Ammo typed provider / durable writer:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAAmmoProvider.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAAmmoProvider.cpp
Tools/RunDAOTests.ps1
```

Contract Evolution Guard / accepted history:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractGuard.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractGuard.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractBase.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractGuardTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractDriftTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractRevTests.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAContractMigTests.cpp
```

Runners:

```text
Tools/RunDAStagingTests.ps1
Tools/RunDAStagingOpsTest.ps1
Tools/RunDAContractTests.ps1
Tools/ClearDAStagingP04.ps1
```

---

## 15. 검증 기준선

CF-FQ-049 P0 closure의 대표 기술 evidence는 다음이다.

### 15.1 Official UE 5.8 Build

```text
Job: 7804b91cfac542fd9aee78af0f2d692f
Result: PASS / Exit Code 0
```

### 15.2 OperationalEntry

```text
Process: 9b3f76b7a78b4f3cb6f30e11ab140327
Exact: 1/1 PASS
Failure: 0
Missing: 0
Unexpected: 0
Duplicate terminal: 0
Product Staging residue: 0
```

검증 범위:

- exact selection과 unselected Invalid isolation
- stale same-selection Review reject
- fresh exact selected Reviewed approval
- forced partial-write Sync rollback raw-byte exact restore
- Product package clean / semantic mutation 0
- Product `ApplyReviewed` 실행 0

### 15.3 Existing DAS regression

```text
Process: 49e54542dd3f42f9a3eee2c10b09e5ed
Exact: 13/13 PASS
Failure: 0
Missing: 0
Unexpected: 0
Duplicate terminal: 0
```

이 regression은 typed parse/Preview, approval lifecycle, loaded identity preflight와 P0-04 test-owned durable Create/Update/save uncertainty/PartialApplied fixture를 포함한다.

### 15.4 Fresh persisted acceptance audit

2026-09-09 Current System Promotion 직전 fresh AssetDump에서:

```text
Root: /Game/Test/CarFight
Class: CFMissileGuidePresetData
Asset count: 3
Succeeded: 3
Failed: 0
Fresh preparation: true
```

canonical Staging은 Low/Normal/High exact Product target 세 개에 binding돼 있다.

P0-04 durable fixture residue가 Product scope에 추가된 징후는 없다.

### 15.5 CF-FQ-050 Contract Evolution Guard acceptance baseline

CF-FQ-050 P0 최종 통합 회귀는 CF-FQ-049 Current operation과 DACE Guard를 같은 current binary 기준으로 검증했다.

```text
Official UE 5.8 Build
Job: 29df6158b2114e6f806a2cb9ace5100f
Result: PASS / Exit 0

DACE focused
Process: aff1246873cb402f813267c04cfd0892
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0

affected CF-FQ-049 Staging
Process: e1b4d2629d7744e8b152e7428340ce36
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

CF-FQ-049 OperationalEntry
Process: 531f99e9da92407ba94fb507c91d1d9c
Exact: 1/1 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Product Staging residue: 0
```

CF-FQ-050 closure에서는 Product Low/Normal/High `ApplyReviewed` 또는 UE Asset Save를 실행하지 않았다. canonical Product exact3 JSON과 `.uasset` scoped Git diff는 0이며 production accepted history도 bootstrap exact1 / append 0을 유지한다.

### 15.6 CF-FQ-051 DAO-P0-01 Pre-Correction Missile Parity — Historical Evidence

```text
Official UE 5.8 Build
Job: 6737c1bff29a40a4bfd9bb6be98213b1
Result: PASS / Exit 0

Affected CF-FQ-049 Staging
Process: 34de9e0426dc4f33ae05a76195bc6190
Exact: 13/13 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Test-owned durable fixture residue: 0

DACE
Process: be73e54a237c4ea8aa20b4e2a5c001e1
Exact: 15/15 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0

Public DataAuthoring header diff: 0
Product Low/Normal/High Apply·Save: 0
canonical Product exact3 JSON diff: 0
Product exact3 .uasset diff: 0
accepted snapshot source diff: 0
accepted history: bootstrap exact1 / append 0
```

첫 affected regression에서는 새 provider-owned root 검사가 기존 test-owned fixture roots까지 차단하는 회귀를 발견했다. `WITH_DEV_AUTOMATION_TESTS` sibling-root allowlist를 추가한 뒤 fresh Build와 exact13/15는 PASS했다. 이후 DAO-P0-01 Mid-review에서 이 allowlist가 Development Editor trust boundary를 넓히는 blocking P1로 재분류됐다. 따라서 위 evidence는 **교정 전 Missile parity Historical evidence**로만 보존한다.

### 15.7 CF-FQ-051 DAO-P0-01 Correction + Re-review Final Evidence

Mid-review의 blocking P1 4건과 P2 2건을 전건 교정한 latest source/binary 기준 evidence다.

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

Product Low/Normal/High Apply·Save: 0
canonical Product exact3 JSON diff: 0
Product exact3 .uasset diff: 0
accepted snapshot source diff: 0
accepted history: bootstrap exact1 / append 0
Ammo typed provider/schema/DACE extension: 0
```

Foundation regression에는 production registry complete validation, test-only second provider exact dispatch/root isolation, other-class same textual StableLogicalId 허용, mixed-type common BatchPlanHash/duplicate/Reviewed Approval seam과 duplicate exact TypeKey fail-closed가 포함된다. Public Missile API는 compatibility facade로 보존한다.

---

## 16. USER Acceptance 경계

CF-FQ-049 P0는 별도 Slate Manager UI나 시각 품질을 제공하는 Feature가 아니다.

Current 운영면은 Editor console + 로그 기반의 technical authoring workflow다.

따라서 P0 closure에는 별도 USER Visual/Feel Acceptance가 필요하지 않으며, 실제 Source·Automation·persisted evidence로 Technical Acceptance를 닫는다.

향후 일반 사용자를 위한 별도 UI를 추가한다면 그 UI의 이해도·버튼 동작·표시 문구·스크롤 등은 별도 USER Acceptance owner를 가져야 한다.

---

## 17. 비책임 / Scope Out

현재 시스템은 다음을 자동 수행하지 않는다.

- Staging file 삭제를 Unreal Asset 삭제로 해석하지 않는다.
- Product Low/Normal/High를 자동 Apply하지 않는다.
- `Save All`을 수행하지 않는다.
- dirty target package를 자동 merge/save/clean하지 않는다.
- stale approval을 자동 재승인하지 않는다.
- 실패 Batch를 자동 retry하지 않는다.
- arbitrary Reflection property writer를 제공하지 않는다.
- CSV/XLSX/XML을 MissileGuidePreset P0 canonical format으로 사용하지 않는다.
- Runtime이 Staging JSON을 읽지 않는다.
- Ammo/Damage/Sensor를 P0 완료만으로 자동 지원하지 않는다.
- VehicleData / Vehicle Builder authoring authority를 대체하지 않는다.

---

## 18. 완료 상태

```text
CF-FQ-049: Done / Data Asset Staging·Batch Authoring P0 Complete
CF-FQ-050: Done / Data Asset Contract Evolution Guard P0 Complete
DAS-P0-05 Final Acceptance: PASS
DACE-P0-06 Final Acceptance: PASS
Current System Promotion: Complete
CF-FQ-051 DAO-P0-01 Correction + Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
Shared multi-type foundation acceptance: PASS
DAO-P0-02 Ammo typed implementation: Complete
DAO-P0-02 Correction + Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
DAO-P0-02 final fresh evidence: Build `898d428caeba4895bb5b0668af513a6e` + DAO focused `b6cb4476ec6d47f4a50214901498b0ed` 4/4 + affected `d6e52207e9d8436d9fae7c27f2027d80` 13/13 + DACE `8789b7e7a1c2446dae91db3bf3f9f7c2` 15/15
Production registered provider: MissileGuidePreset ReviewedMutationReady + AmmoData ReviewedMutationReady exact2
DAO-P0-03 implementation: Complete
DAO-P0-03 Correction + Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
Ammo durable writer/materializer/save: Accepted via provider-local exact8 materializer + shared CFDADurableCore single durable authority
Ammo DACE descriptor/probe/accepted history: Implemented / `CFDAAmmoDace` independent owner / `DACE-AmmoData-S1-A1-Bootstrap` exact1
DAO-P0-04 Pre-Implementation Contract Review: Historical HOLD / P0 0 / blocking P1 4 / P2 0
DAO-P0-04 Contract Correction + Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
DAO-P0-04 accepted shared correction: per-TypeKey DACE readiness/history/revision/canonical-target gate + AmmoTags[] element observation + AmmoIcon SoftObject target-class Reflection
DAO-P0-04 correction final evidence: Build `5c2300543af24ce78b1eddd4d409d43e` PASS + DAO `503460d34b5944fda2efccda396d5d76` 9/9 PASS + DACE `e91c4f1d19ca401495e138ca35497474` 15/15 PASS
DAO-P0-04 Ammo DACE implementation: Complete / Fresh Validation PASS
DAO-P0-04 implementation fresh evidence: Build `14fc6c12e83a4d7a98154ad6eb59c6ea` PASS + DAO `69c4d86b35d940c0bba8c51830181e91` 13/13 PASS + existing Missile DACE `357d7ac193e84d0eba57213b220424b0` 15/15 PASS
DAO-P0-04 Post-Implementation Mid-review: Historical HOLD / P0 0 / blocking P1 1 / P2 1
DAO-P0-04 Correction + Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
DAO-P0-04 P1 closure: shared `FScopedSemanticTokenProbe` + test-owned exact token manifest로 production fingerprint token omission 독립 검출 / non-null 15 / null 14
DAO-P0-04 P2 closure: immutable `DACE-AmmoData-S1-A1-Bootstrap` exact1을 dedicated append-only `CFDAAmmoDaceBase.cpp`로 분리 / 기존 값·signature delta 0
DAO-P0-04 correction fresh evidence: Build `d93c392b125d4b338060307f7d22a65e` PASS + DAO `577467e756184582ae84ef5f6c9f4202` 13/13 PASS + existing Missile DACE `fc7b719448f1454eab99dbda0e405b43` 15/15 PASS
DAO-P0-04 migration state: initial Ammo bootstrap `NoMigration` / current no-delta declaration nullptr / duplicate accepted append blocked / Product canonical Ammo exact0
DAO-P0-04 Technical Acceptance: PASS
DAO-P0-05 Pre-Implementation Contract Review: Historical HOLD / P0 0 / blocking P1 2 / P2 1
DAO-P0-05 Contract Correction + Independent Re-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
DAO-P0-05 P1-1 correction: Product canonical Ammo exact0 mutation0 lane은 zero-row coexistence/read-only protection으로 고정하고 Reviewed approval은 disposable actual-provider mutation lane으로 분리 / fake Product Ammo·Staging 및 no-op approval 금지
DAO-P0-05 P1-2 correction: mixed operational discovery는 Canonical Product Set / Explicit Paths exact2 mode만 허용하고 JSON read 전 trusted production provider CanonicalStagingRoot ownership exact1을 결정 / owner0·owner>1·scope mismatch fail-closed / payload-first 및 parent-root recursive trust 금지
DAO-P0-05 compatibility correction: existing Missile empty-selection discovery, Public/console API, SyncProductStaging Product exact3 behavior 유지
DAO-P0-05 P2 correction: global preflight + per-target immediate TOCTOU + sequential durable apply / PartialApplied 가능 / cross-target all-or-nothing atomicity non-goal
DAO-P0-05 actual-provider disposable integration: PASS / provider-local `__AutomationP05__` + `/Game/Test/CarFight/DAOP05/` / Missile1 + Ammo1 durable exact2 / class-scoped same textual StableLogicalId / global target duplicate / source-current TOCTOU / teardown residue0
DAO-P0-05 implementation: Complete / Fresh Validation PASS
DAO-P0-05 final evidence: Build `39e4973ff0ad4e31803c679d272533ff` PASS + focused `148c4f325b724b2fa7bfd7158819c531` 4/4 + affected `0f2f69983f024986aa87a98afee2aed9` 13/13 + existing Missile DACE `8967abe39f1a4d128c3ce455ede970a5` 15/15
DAO-P0-05 protection/residue: protected exact10 scoped diff0 + Ammo bootstrap exact1 fixed readback unchanged + `/Game/Test/CarFight/DAOP05` Ammo exact0 + persisted CFAmmoData HeavyFinite/RocketFinite exact2 only
DAO-P0-05 Post-Implementation Mid-review: Technical PASS / P0 0 / blocking P1 0 / P2 0
DAO-P0-05 Technical Acceptance: PASS
DAO-P0-05 mid-review execution: Source mutation 0 / Build-focused4-affected13-DACE15 not rerun
DAO-P0-05 third-type reuse review: shared Preview/Review/TOCTOU/Apply rewrite 0 required / exact2 allowed list is explicit admission policy
DAO-P0-06 Pre-Acceptance/Measurement Review: PASS / P0 0 / blocking P1 0 / P2 1 non-blocking
DAO-P0-06 Reuse Measurement: PASS / prohibited algorithm duplication 0 / third-type core rewrite 0 required
DAO-P0-06 Current System Promotion: Complete
CF-FQ-051: Done / Technical Complete
DamageData third onboarding: Candidate / Not Started
Exact next: none for CF-FQ-051; new DataAsset type uses a separate lifecycle
DAO-P0-03 correction fresh evidence: Build `bc420d5554e849fdbad32e65296a7188` PASS + DAO `b3773a90332b4d6295670a37f276d44f` 7/7 PASS
DAO-P0-03 unchanged-executable affected evidence: affected `bcb9e15f9b8942d886768896a35b0bf2` 13/13 + DACE `977d1bc1566f41d7b805e608cd8e1dcb` 15/15 remain valid; not rerun for comment/document-only correction
DAO-P0-03 disposable fixture residue: fresh AssetDump `/Game/Test/CarFight/DAOAmmoP03` CFAmmoData exact0
Current owner: Document/Systems/DataManagement/DataAssetAuthoring.md v1.4.1
Product Low/Normal/High Apply·Save during CF-FQ-049/050/051-P0-01: 0
canonical Product Staging mutation during CF-FQ-050 closure: 0
accepted snapshot history: bootstrap exact1 / CF-FQ-050 closure append 0
CF-FQ-039 Active lifecycle: unchanged
```

후속 타입 확장이나 별도 UI는 CF-FQ-049/050의 old next gate를 재사용하지 않고 새로운 Feature/lifecycle로 연다.

---

## 19. Changelog

### v1.4.1 - 2026-09-10

- `CF-FQ-051 DAO-P0-06 Final Audit Correction + Re-review`에서 v1.4.0 최종검수의 `P0 0 / blocking P1 1 / P2 2 / HOLD`를 문서-only로 교정했다. Current §2에서 이미 승격된 `CFAmmoData`를 미지원 타입 목록에서 제거하고, `CFDamageData`·`CFVehicleSensorData`·기타 신규 DataAsset 타입만 별도 provider-centric onboarding 대상으로 명확히 했다.
- 상단 완료 Feature에 `CF-FQ-051 Data Asset Multi-Type Onboarding`을 추가했다. 기존 hybrid shared/generic implementation과 legacy Missile compatibility의 물리적 owner 혼재는 correctness를 막지 않는 `P2 1 non-blocking` maintenance debt로 유지한다.
- Source/Asset/Test mutation과 Build/Automation 재실행은 0이다. 직전 accepted executable evidence와 Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, protected exact10, CF-FQ-039 Active를 그대로 보존한다.
- 교정 후 독립 재검수 기준 `P0 0 / blocking P1 0 / P2 1 non-blocking / PASS`이며 CF-FQ-051은 Done / Technical Complete 상태를 유지한다.

Migration: v1.4.1은 v1.4.0의 실행 계약을 변경하지 않는 post-closure Current 문서 교정본이다. AmmoData는 Current 지원 exact2 중 하나이며, 아직 자동 지원되지 않는 신규 타입은 `CFDamageData`, `CFVehicleSensorData` 및 기타 새 DataAsset 타입이다. hybrid shared/legacy physical ownership P2는 실제 반복 수정 비용이 확인될 때만 별도 maintenance로 다룬다.

### v1.4.0 - 2026-09-10

- `CF-FQ-051 DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion`을 current Source 기준으로 완료했다. 사전 Acceptance/Measurement 검수는 `P0 0 / blocking P1 0 / P2 1 / PASS`이며 P2는 legacy Missile compatibility와 generic implementation의 물리적 owner 혼재뿐인 non-blocking maintenance debt다.
- dedicated shared foundation exact6/2300 physical lines, hybrid shared+compat exact7/6363 physical lines, Ammo-specific production exact5/1789 physical lines, Missile parity/compat reviewed exact9, feature-owned Automation exact17을 구조 지표로 기록했다. 분류가 겹치는 항목은 합산하지 않으며 LOC를 개발 시간으로 사용하지 않는다.
- Ammo second onboarding은 Preview/Reviewed approval/TOCTOU/BatchPlanHash/result aggregation/rollback·uncertainty/durable orchestration/DACE accepted-chain·migration algorithm을 복제하지 않았다. `CFDADurableCore`, `CFDATypeDispatch`, `FCFDAContractGuard` shared authority를 재사용한다.
- third DataAsset type는 typed provider/schema/parser/serializer/extract/materialize/current-state + DACE descriptor/history를 타입 전용으로 추가하고 production provider registration + explicit operational admission을 수행하면 된다. shared Preview/Review/TOCTOU/Apply/Durable/DACE core algorithm rewrite는 요구하지 않는다.
- DAO-P0-06 executable Source mutation은 0이라 직전 official Build `39e4973ff0ad4e31803c679d272533ff`, focused4 `148c4f325b724b2fa7bfd7158819c531`, affected13 `0f2f69983f024986aa87a98afee2aed9`, Missile DACE15 `8967abe39f1a4d128c3ce455ede970a5`를 중복 실행하지 않았다. existing OperationalEntry와 CF-FQ-045 Ammo identity/health Accepted baseline도 유지한다.
- Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, protected exact10, CF-FQ-039 Active와 기존 병렬 dirty를 보존한 채 CF-FQ-051을 Technical Complete로 닫고 MissileGuidePreset+AmmoData exact2 multi-type authoring/guard를 Current System으로 승격했다.

Migration: v1.4.0부터 `DataAssetAuthoring.md`는 MissileGuidePreset + AmmoData exact2의 Current multi-type authoring/guard 계약을 소유한다. 새 타입은 provider-centric onboarding을 사용하고 common orchestration을 복제하지 않는다. DamageData는 다음 후보지만 자동 착수하지 않으며 별도 Feature/lifecycle에서 검수한다. hybrid shared/legacy physical ownership P2는 실제 third-type 반복 수정 비용이 확인될 때만 별도 maintenance로 승격한다.

### v1.3.14 - 2026-09-10

- `CF-FQ-051 DAO-P0-05 Post-Implementation Mid-review`를 current Source 기준 read-only로 독립 수행해 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫고 DAO-P0-05 Technical Acceptance를 PASS로 승격했다.
- `CFDATypeDispatch::FindProviderForStagingPath()`가 full production provider set의 CanonicalStagingRoot containment로 JSON read 전에 owner exact1을 결정하고 owner0/owner>1/allowed TypeKey scope mismatch를 fail-closed하는 것을 재확인했다. Allowed scope는 owner 결정 뒤 검사하므로 overlapping root ambiguity를 숨기지 않는다.
- Mixed Explicit Paths는 owner provider의 parse/current callback 뒤 payload-free common Preview/duplicate/hash/Reviewed approval core를 재사용한다. `FCFDAStagingOpsSession::Review()`는 Preview에서 동결한 normalized exact path set과 동일 discovery mode를 fresh replay하며 BatchPlanHash mismatch를 차단한다.
- Class-scoped StableLogicalId는 exact DataAsset class + FName semantic으로 유지하고 TargetObjectPath duplicate는 global blocker다. Apply는 global fresh preflight 뒤 target별 immediate TOCTOU와 sequential durable write를 수행하며 existing affected13의 PartialApplied regression과 P0-05 source/current stale regression이 current semantics를 증명한다.
- Existing Missile empty-selection whole-root discovery, non-empty Missile-only typed path, Public API, Console StableLogicalId shorthand와 SyncProductStaging Product Low/Normal/High exact3 compatibility가 유지됨을 재검수했다. Mixed mode의 Public Missile Preview DTO는 structural projection만 담당하고 typed payload는 approval authority가 아니다.
- Third-type reuse 관점에서 MissileGuidePreset+AmmoData exact2 allowed list는 payload/provider-specific orchestration branch가 아니라 새 타입의 자동 승격을 막는 code-owned admission policy다. 세 번째 타입은 provider registration + explicit admission을 요구하지만 shared Preview/Review/TOCTOU/Apply algorithm 재작성은 필요하지 않으므로 abstraction leak blocker로 분류하지 않았다.
- focused exact4에 missing-file 전용 terminal case는 없지만 production mixed discovery의 explicit `FileExists` pre-read fail-closed와 provider wrong-TypeKey negative regression을 current Source와 기존 evidence로 확인해 추가 P2 blocker로 보지 않았다.
- Mid-review executable Source mutation은 0이므로 final Build `39e4973ff0ad4e31803c679d272533ff`, focused `148c4f325b724b2fa7bfd7158819c531` 4/4, affected `0f2f69983f024986aa87a98afee2aed9` 13/13, Missile DACE `8967abe39f1a4d128c3ce455ede970a5` 15/15를 중복 실행하지 않았다. protected exact10 scoped diff0과 fresh AssetDump CFAmmoData HeavyFinite/RocketFinite exact2 only를 재확인했다.
- 다음 exact Gate는 `DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion`이며 이번 Mid-review에서는 P0-06 측정/구현/Promotion을 시작하지 않았다.

Migration: v1.3.14부터 DAO-P0-05 multi-type operational integration은 Technical Accepted다. 새 타입 추가는 provider registration + explicit admission policy를 통해 확장하고 common orchestration을 타입별로 복제하지 않는다. AmmoData의 최종 Current 지원 승격은 DAO-P0-06 Current System Promotion 완료가 소유한다.

### v1.3.13 - 2026-09-10

- `CF-FQ-051 DAO-P0-05 Canonical Ammo Staging Pilot / Multi-Type Integration`을 Current Source에 구현하고 fresh validation을 완료했다. `CFDATypeDispatch v1.6.0`의 exact Staging path owner resolver는 JSON read 전에 full production provider registry의 CanonicalStagingRoot containment로 owner exact1을 결정하고 owner0/owner>1/allowed TypeKey scope mismatch를 fail-closed하며 raw registry를 노출하지 않는다.
- `CFDAStagingOps v1.4.0`은 empty selection의 기존 Missile whole-root discovery와 non-empty Missile-only typed selection을 보존하고, Ammo가 포함된 non-empty canonical exact paths만 provider-neutral mixed Explicit Paths mode로 처리한다. Review는 동일 normalized path set/mode를 fresh replay하며 Console StableLogicalId shorthand와 SyncProductStaging Product Low/Normal/High exact3 동작은 유지한다.
- Actual-provider disposable integration은 `/Game/Test/CarFight/DAOP05/` + provider-local `__AutomationP05__`에서 Missile 1 + Ammo 1 same textual StableLogicalId를 class-scoped identity로 허용하고 한 Reviewed approval/ApplyReviewed에서 durable exact2가 되는 것을 증명했다. Cross-TypeKey 동일 TargetObjectPath는 global duplicate로 차단하고 source/current stale state는 mutation0으로 fail-closed한다.
- 첫 affected13 실행에서 old test-only second-provider fixture가 v1.5+ DACE canonical target-set declaration을 누락한 것을 발견했다. Production provider/algorithm을 완화하지 않고 `CFDAStagingTests v1.8.0` test fixture만 `ContractNotReady + explicit canonical exact0 declaration`으로 현재 provider completeness contract에 정렬했다.
- 최종 authoritative UE 5.8 Build `39e4973ff0ad4e31803c679d272533ff` PASS, DAO-P0-05 focused `148c4f325b724b2fa7bfd7158819c531` 4/4 PASS, affected CF-FQ-049 `0f2f69983f024986aa87a98afee2aed9` 13/13 PASS, existing Missile DACE `8967abe39f1a4d128c3ce455ede970a5` 15/15 PASS다.
- protected exact10 scoped diff 0, `CFDAAmmoDaceBase.cpp` bootstrap exact1 fixed field/signature readback unchanged, `/Game/Test/CarFight/DAOP05` persisted Ammo exact0, 전체 persisted CFAmmoData HeavyFinite/RocketFinite exact2만 존재함을 확인했다. Product canonical Ammo exact0, Product Low/Normal/High Apply·Save 0, fake Product Ammo/Staging 0, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다.
- 현재 checkpoint는 `Implementation + Fresh Validation PASS / Post-Implementation Mid-review Pending`이다. Technical Acceptance와 DAO-P0-06 진입은 아직 허용하지 않는다.

Migration: v1.3.13부터 actual MissileGuidePreset+AmmoData mixed non-empty Explicit Paths authoring session이 Current Source에 존재한다. New type operational selection은 payload-first routing이나 parent-root recursive discovery가 아니라 code-owned provider-root exact owner resolution을 사용한다. 기존 Missile Public/console/Product Sync behavior와 sequential/PartialApplied Apply semantics는 유지하며, Post-Implementation Mid-review PASS 전에는 DAO-P0-05 Technical Acceptance 또는 DAO-P0-06 Current System Promotion을 주장하지 않는다.

### v1.3.12 - 2026-09-10

- `CF-FQ-051 DAO-P0-05 Contract Correction`에서 v1.3.11 사전검수의 blocking P1 2건과 P2 1건을 구현 전에 계약 수준으로 교정했다. **Current Source는 아직 Missile-only Ops discovery 상태**이며 이 문서는 다음 구현이 지켜야 할 frozen contract를 기록한다.
- Product acceptance는 explicit Ammo canonical exact0을 zero-row 정상 contribution으로 취급한다. fake Product Ammo asset/Staging, Product Ammo Review/Apply/Save와 Missile Product Low/Normal/High Apply/Save는 0이다. Empty/NoChange에 Reviewed approval을 발급하도록 existing common Review를 완화하지 않는다.
- 실제 mixed Review/TOCTOU/provider dispatch는 actual production registry의 MissileGuidePreset+AmmoData를 사용하되 provider root 아래 `__AutomationP05__` exact paths와 `/Game/Test/CarFight/DAOP05/` disposable target만 사용한다. Positive acceptance는 한 mixed Review에서 Missile 1개 + Ammo 1개 disposable Create를 승인하고 같은 ApplyReviewed에서 actual exact provider dispatch로 두 target 모두 durable Applied가 되는 것까지 확인한다. HeavyFinite/RocketFinite와 Product exact3는 fixture에서 제외하고 cleanup 후 residue0을 요구한다.
- planned mixed operational discovery는 `Canonical Product Set` / `Explicit Paths` 두 mode로 제한한다. path owner는 JSON read 전에 code-owned production registry의 CanonicalStagingRoot containment로 exact1 결정하고 owner 0/>1 또는 caller allowed TypeKey scope 밖이면 fail-closed한다. Ops가 raw registry를 공개 소유하지 않고 TypeDispatch의 좁은 path-owner resolver seam을 사용한다. payload-first TypeKey, parent `Authoring/DataAssetStaging` recursive scan, mixed empty-selection all-provider scan은 금지한다. `FCFDAStagingOpsSession`은 empty selection을 existing Missile whole-root mode로 유지하고 non-empty canonical selection만 mixed Explicit Paths mode로 일반화하며 `Review()`가 동일 normalized path set/mode를 fresh replay한다. ambiguous overlapping root reject는 production registry mutation 없는 test-owned explicit provider-set seam으로 증명한다.
- 기존 Missile `DiscoverMissilePresetPreview(empty selection = Missile root discovery)`, Public/console API와 `SyncProductStaging` Product exact3 behavior는 compatibility facade로 유지한다. 새 mixed mode가 기존 의미를 재정의하지 않는다. Console StableLogicalId shorthand도 existing `BuildConsoleSelection()`의 Missile canonical path 변환 의미를 유지하며 Ammo shorthand로 암묵 확장하지 않는다.
- Mixed Apply는 current Source와 동일하게 approval 전체 global preflight 뒤 target별 immediate TOCTOU + sequential durable execution이다. cross-target all-or-nothing은 non-goal이고 `PartialApplied`, `SaveStateUnconfirmed`, `InMemoryStateUnconfirmed` taxonomy/precedence는 유지한다.
- 독립 재검수에서 actual Ammo provider literal Source의 `CanonicalStagingRoot=Authoring/DataAssetStaging/AmmoData`, `bDaceCanonicalStagingTargetSetDeclared=true`, canonical target exact0, `ReviewedMutationReady`, DACE `ContractReady`를 재확인하고 current TypeDispatch/Ops/Apply와 correction contract를 대조했다. 추가 blocking issue는 없으며 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`다.
- 이번 correction + re-review는 문서 mutation만 수행했고 executable Source/Build/Automation/Staging/uasset/DACE accepted history mutation은 0이다. protected exact10, Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, CF-FQ-039 Active와 기존 병렬 dirty를 보존한다. DAO-P0-05 implementation은 아직 시작하지 않았고 이제 Ready다.

Migration: v1.3.12의 mixed discovery/acceptance 내용은 아직 구현 완료 상태가 아니라 DAO-P0-05의 frozen next implementation contract다. Product mutation0 lane과 disposable mutation lane을 분리하고, future path-owner resolver는 raw registry exposure 없이 exact one-provider ownership만 반환해야 한다. Contract Correction + Independent Re-review가 `P0 0 / blocking P1 0 / P2 0` Technical PASS했으므로 다음 exact Gate는 `DAO-P0-05 Canonical Ammo Staging Pilot / Multi-Type Integration Implementation`이다.

### v1.3.11 - 2026-09-10

- `CF-FQ-051 DAO-P0-05 Canonical Ammo Staging Pilot / Multi-Type Integration` 구현 전 계약검수를 current Source 기준 read-only로 수행해 `P0 0 / blocking P1 2 / P2 1 / HOLD`로 판정했다. P0/P1 0 전 구현 금지 조건에 따라 DAO-P0-05 C++/script/Staging/Product mutation은 시작하지 않았다.
- shared `CFDATypeDispatch`는 production provider exact2를 등록하고 class-scoped StableLogicalId, global TargetObjectPath duplicate, mixed common BatchPlanHash/Review, exact provider dispatch와 Apply 직전 TOCTOU를 이미 payload-free로 지원한다. 기존 mixed common regression도 다른 class의 동일 textual StableLogicalId 허용, cross-type 동일 TargetObjectPath 차단과 deterministic hash/Review를 증명한다. DACE history/revision도 selected exact TypeKey에 결속되어 Missile history를 Ammo provider에 전달하면 reject하는 현재 계약을 유지한다.
- blocking P1-1은 Product canonical Ammo explicit exact0의 `mutation0 acceptance`와 current Review authority를 같은 acceptance lane으로 읽을 수 있는 계약 모순이다. `BuildCommonBatchPlanHash`/`BuildCommonReviewedApproval`은 Create/Update가 하나 이상 있어야 하며 pure NoChange/empty set에는 Review approval 자체가 필요하지 않다. Correction에서는 Product lane을 `Ammo exact0 유지 + fake Product Ammo/Staging 0 + Product Apply/Save 0`의 mutation0 coexistence로 분리하고, 실제 Review/TOCTOU 증명은 disposable test-owned Missile+Ammo mutation candidate lane에서 수행하도록 동결해야 한다.
- blocking P1-2는 production operational discovery/selection이 아직 `CFDAStagingOpsSession`/`DiscoverMissilePresetPreview`/Missile canonical root에 결속돼 실제 registered MissileGuidePreset+AmmoData를 같은 operational session으로 선택·발견할 수 없는 점이다. Common core를 재작성하지 말고 trusted registry의 provider-owned canonical roots로 exact path owner를 결정하는 provider-neutral mixed discovery/selection 계층을 추가하되 기존 Missile Public/console compatibility facade와 `SyncProductStaging` exact3 동작은 보존해야 한다. Ammo root가 empty/absent인 exact0 상태는 유효한 zero-row provider result로 처리하고 부모 root 전체 recursive trust나 payload-first TypeKey inference는 금지한다.
- P2-1은 current shared Apply가 target별 global preflight 후 순차 durable write를 수행하며 후속 target 실패 시 `PartialApplied`를 허용한다는 점이다. P0-05 mixed integration을 cross-target all-or-nothing atomicity로 오해하지 않도록 correction contract에 non-goal로 명시하고, Product mutation0 acceptance에서는 mixed Product Apply를 실행하지 않는다.
- 이번 review는 Source mutation 0이므로 DAO-P0-04 final fresh Build `d93c392b125d4b338060307f7d22a65e`, DAO13 `577467e756184582ae84ef5f6c9f4202`, Missile DACE15 `fc7b719448f1454eab99dbda0e405b43`을 재실행하지 않는다. protected exact10 중 legacy exact9 scoped diff 0을 확인하고 신규 append-only `CFDAAmmoDaceBase.cpp`는 read-only baseline 그대로 보존한다. CF-FQ-039 Active와 기존 병렬 dirty도 변경하지 않는다.

Migration: v1.3.11에서 DAO-P0-05는 implementation-ready가 아니다. `DAO-P0-05 Contract Correction + Re-review`에서 mutation0 Product lane과 test-owned Review/TOCTOU lane을 분리하고 provider-neutral operational discovery/selection 및 mixed Apply non-atomic semantics를 동결한 뒤 P0/P1 0을 재확인해야 구현을 시작한다. Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, `CFDAContractBase.cpp`, `CFDAAmmoDaceBase.cpp`를 보존한다.

### v1.3.10 - 2026-09-10

- `CF-FQ-051 DAO-P0-04 Correction + Re-review`에서 v1.3.9 Mid-review의 blocking P1 1건/P2 1건을 전건 교정하고 current Source + fresh validation + independent re-review 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다.
- P1은 새 provider API를 만들지 않고 existing shared `CFDACommonPrimitives::FScopedSemanticTokenProbe`를 재사용해 실제 `AppendPayloadTokens()` production token emission을 관측하도록 교정했다. `CFDAAmmoDaceTests`가 production code에서 파생하지 않은 literal expected manifest를 소유하고 non-null AmmoIcon + AmmoTags exact2에서 token 15개, null AmmoIcon에서 path token을 제외한 14개를 exact order/count로 검증한다. SchemaId/SchemaRevision/AdapterContractRevision/DataAssetTypeClassPath와 Ammo exact8 semantic, AmmoTags Count/[] 및 AmmoIcon IsNull/conditional Path가 모두 coverage에 포함된다.
- P2는 기존 `DACE-AmmoData-S1-A1-Bootstrap` exact1의 SnapshotId, revisions, class path, component signature 4종, migration 값과 final snapshot signature를 한 글자도 재기준화하지 않고 신규 `CFDAAmmoDaceBase.cpp v1.0.0` dedicated append-only production owner로 이동했다. `CFDAAmmoDace.cpp v1.1.0`은 mutable current descriptor/migration authority만 유지하며 `GetAcceptedSnapshots()` implementation을 소유하지 않는다. focused regression도 bootstrap fixed literal 전 필드를 독립 고정한다.
- fresh official UE 5.8 Build `d93c392b125d4b338060307f7d22a65e` PASS, DAO exact13 `577467e756184582ae84ef5f6c9f4202` 13/13 PASS, existing Missile DACE exact15 `fc7b719448f1454eab99dbda0e405b43` 15/15 PASS다. 마지막 protected legacy exact9 scoped worktree diff는 0이며 새 `CFDAAmmoDaceBase.cpp`는 fixed readback 완료 후 이후 DACE 변경의 protected set에 추가한다.
- Product canonical Ammo exact0, Product Low/Normal/High exact3, HeavyFinite/RocketFinite exact2, protected Missile `CFDAContractBase.cpp`, CF-FQ-039 Active와 기존 병렬 dirty를 보존했다. DAO-P0-05 구현은 시작하지 않았다.

Migration: v1.3.10부터 DAO-P0-04는 Technical PASS다. Ammo accepted history 변경 시 `CFDAAmmoDaceBase.cpp`를 append-only authority로 취급하고 기존 bootstrap record를 수정/rebaseline하지 않는다. production fingerprint 변경 시 token manifest regression을 함께 통과해야 한다. 다음 exact Gate는 `DAO-P0-05 — Canonical Ammo Staging Pilot / Multi-Type Integration`이며 Product canonical Ammo exact0 상태에서 mutation0 multi-type coexistence를 기본 acceptance로 사용한다.

### v1.3.9 - 2026-09-10

- `CF-FQ-051 DAO-P0-04 Post-Implementation Mid-review`를 current Source 기준 read-only로 수행해 `P0 0 / blocking P1 1 / P2 1 / HOLD`로 판정했다. 구현 자체와 직전 fresh Build/DAO13/Missile DACE15 PASS는 유지하지만 Technical Acceptance와 DAO-P0-05 진입은 보류한다.
- SourceShape exact8, AdapterShape exact19, SourceAdapterMapping exact16, SemanticContract exact14, `AmmoTags[]` String/FNameToken element contract, `AmmoIcon` SoftObject + `/Script/Engine.Texture2D` target-class/no-load, Schema/Adapter revision reject, cross-TypeKey accepted-history reject와 Product canonical Ammo explicit exact0은 current Source/combined DAO regression에서 유지됨을 재확인했다.
- blocking P1 1건은 Ammo production fingerprint probe가 `BuildSemanticFingerprint()`로 만든 값을 serializer→parser 및 materialize→extract 뒤 다시 같은 함수로 비교하는 self-consistency 구조라서, 향후 `AppendPayloadTokens()`에서 semantic token 하나가 누락되어도 양쪽 결과가 함께 바뀌어 DACE의 `production semantic fingerprint token mismatch` drift를 독립적으로 검출하지 못할 수 있는 문제다. Correction에서는 production token label/coverage observation 또는 동등한 exhaustive per-field fingerprint inclusion 검증을 추가해야 한다.
- P2 1건은 independent Ammo accepted exact1을 mutable current descriptors와 같은 `CFDAAmmoDace.cpp`가 함께 소유하는 구조다. Runtime chain validation은 정상이나 descriptor와 bootstrap signature를 동시에 수정하면 accidental rebaseline을 놓치기 쉬우므로, Missile `CFDAContractBase.cpp`처럼 dedicated append-only production owner로 분리하고 이후 protected audit 대상으로 고정하는 방향을 권고한다.
- 이번 Mid-review는 Source mutation 0이다. 따라서 implementation fresh Build `14fc6c12e83a4d7a98154ad6eb59c6ea`, DAO exact13 `69c4d86b35d940c0bba8c51830181e91`, existing Missile DACE exact15 `357d7ac193e84d0eba57213b220424b0`을 중복 실행하지 않았고 protected exact9 scoped worktree diff 0을 재확인했다.

Migration: v1.3.9에서 Ammo DACE implementation과 bootstrap exact1은 유지되지만 DAO-P0-04 Technical Acceptance는 HOLD다. `DAO-P0-04 Correction + Re-review`에서 fingerprint token coverage P1을 닫고 accepted-history separation P2를 교정·재검수하기 전에는 DAO-P0-05를 시작하지 않는다. Product canonical Ammo exact0, existing Missile DACE/history, Product Low/Normal/High exact3와 HeavyFinite/RocketFinite exact2는 계속 보존한다.

### v1.3.8 - 2026-09-10

- `CF-FQ-051 DAO-P0-04 Ammo DACE Descriptor / Probe / Bootstrap Implementation`을 current Source에 구현했다. Editor Private `CFDAAmmoDace`가 AmmoData exact TypeKey의 SourceShape/AdapterShape/SourceAdapterMapping/SemanticContract와 independent `DACE-AmmoData-S1-A1-Bootstrap` exact1을 소유하며 protected Missile `CFDAContractBase.cpp` history와 물리적으로 분리된다.
- Ammo production provider에 deterministic whole-record memory serializer를 추가하고 DACE readiness를 `ContractReady`로 승격했다. Product canonical Ammo target set은 계속 explicit exact0이며 Product/Staging 파일 생성·저장 권한을 DACE에 추가하지 않았다.
- Adapter contract는 `Payload.AmmoTags[] = String / FNameToken`을 terminal element row로 고정하고 non-empty production serializer probe로 실제 wiring을 증명한다. `AmmoIcon` Source contract는 `SoftObject / /Script/Engine.Texture2D / Scalar`이며 referenced Texture load 없이 metadata/semantic round-trip과 target-class drift를 검증한다.
- fresh official UE 5.8 Build `14fc6c12e83a4d7a98154ad6eb59c6ea` PASS, DAO exact13 `69c4d86b35d940c0bba8c51830181e91` 13/13 PASS, existing Missile DACE exact15 `357d7ac193e84d0eba57213b220424b0` 15/15 PASS다. protected exact9 scoped worktree diff는 0이다.
- Ammo bootstrap은 initial `NoMigration / NotRequired` baseline이며 current descriptor와 exact 일치한다. current no-delta state에서는 change declaration이 nullptr이고 duplicate accepted append는 차단된다. 이번 checkpoint는 구현+fresh validation 완료이며 `DAO-P0-04 Post-Implementation Mid-review` 전 Technical Acceptance로 확대하지 않는다.

Migration: v1.3.8부터 Ammo DACE 구현 자체는 Current Source에 존재하고 provider는 `ContractReady`다. 그러나 DAO-P0-04 Technical Acceptance는 Post-Implementation Mid-review가 소유한다. 다음 review 전에는 P0-05로 넘어가지 않으며, Missile DACE15/accepted history, Product Low/Normal/High exact3, HeavyFinite/RocketFinite exact2와 Product canonical Ammo exact0을 그대로 보존한다.

### v1.3.7 - 2026-09-10

- `CF-FQ-051 DAO-P0-04 Contract Correction + Re-review`에서 v1.3.6의 blocking P1 4건을 shared DACE boundary에서 전건 교정하고 independent Source re-review 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다. 실제 Ammo DACE descriptor/probe/accepted bootstrap은 아직 시작하지 않았다.
- per-TypeKey DACE readiness/history/revision/canonical Staging target authority를 current contract로 반영했다. MissileGuidePreset은 기존 DACE `ContractReady` + Product exact3을 유지하고, AmmoData는 authoring `ReviewedMutationReady`이지만 DACE `ContractNotReady` + declared Product canonical exact0으로 분리한다.
- `AmmoTags[]` non-empty Array element physical observation과 `AmmoIcon`의 `SoftObject / /Script/Engine.Texture2D / Scalar` target-class Reflection을 shared correction contract로 반영했다. Ammo `ContractNotReady` 상태에서는 exact0 compatibility만으로 accepted append/Current promotion prerequisite를 통과하지 못하도록 provider-aware gate가 fail-closed한다.
- accepted final evidence는 official UE 5.8 Build `5c2300543af24ce78b1eddd4d409d43e` PASS, DAO exact9 `503460d34b5944fda2efccda396d5d76` 9/9 PASS, existing Missile DACE exact15 `e91c4f1d19ca401495e138ca35497474` 15/15 PASS다. protected exact9 diff 0과 DAO-P0-03 disposable persisted residue 0을 final correction evidence로 유지한다.
- 이번 v1.3.7 Current projection에서는 Source를 추가 수정하지 않았고 Build/Automation도 중복 실행하지 않았다. CF-FQ-049 affected13은 직전 Historical PASS를 유지하며 이번 correction의 fresh evidence로 확대하지 않는다.

Migration: v1.3.7부터 DAO-P0-04 shared DACE correction은 Accepted Current contract다. 다음 exact Gate는 `DAO-P0-04 Ammo DACE Descriptor / Probe / Bootstrap Implementation`이며, Ammo가 `ContractReady`로 승격되기 전에는 accepted append/Current promotion을 허용하지 않는다. Product canonical Ammo exact0, 기존 Missile DACE15/accepted history와 HeavyFinite/RocketFinite exact2는 그대로 보존한다.

### v1.3.6 - 2026-09-10

- `CF-FQ-051 DAO-P0-04` 구현 전 계약검수를 current Source 기준으로 수행해 `P0 0 / blocking P1 4 / P2 0 / HOLD`로 기록했다. P1 0 확인 전 구현 금지 조건에 따라 Ammo DACE Source 구현과 accepted bootstrap append는 시작하지 않았다.
- current DACE의 generic validation primitives는 재사용 가능하지만 `FCFDAContractGuard`의 Current descriptor/reflection/probe/accepted history/current revision/current canonical Staging wrapper는 MissileGuidePreset 단일 identity에 결속돼 있다. DAO-P0-04 correction은 common DACE algorithm과 exact TypeKey별 contract provider/history/current declaration/canonical-target authority를 분리해야 한다.
- AmmoTags는 current parser/fingerprint에 Array<String>/FName set semantic이 이미 존재하지만 DACE JSON observer가 array element를 관측하지 않으므로 `Payload.AmmoTags[]` synthesized element contract가 필요하다. AmmoIcon은 `TSoftObjectPtr<UTexture2D>` target class를 existing `ReflectedTypePath`에 기록할 scalar SoftObject Reflection 보강이 필요하다.
- Product canonical Ammo는 현재 exact0이다. 따라서 Missile exact3 hard-coded migration wrapper를 Ammo에 재사용하지 않고 per-TypeKey canonical target set을 사용하며, Ammo exact0은 explicit valid empty set으로 해석한다. old revision regression은 memory/test-owned Ammo JSON으로 수행하며 fake Product Ammo 또는 HeavyFinite/RocketFinite 재분류를 금지한다.
- Ammo current contract baseline은 SchemaRevision 1 / AdapterContractRevision 1 / history namespace `DACE-AmmoData`다. Ammo accepted history는 Missile chain과 분리하고 protected `CFDAContractBase.cpp`를 Ammo bootstrap owner로 사용하지 않는다. 한 TypeKey의 revision/declaration/history가 다른 TypeKey에 전파되지 않아야 한다.
- 이번 변경은 Source 구현이 아닌 read-only review + Current projection 동기화이므로 Build/Automation을 재실행하지 않았다. DAO-P0-03 fresh Build/DAO7와 기존 affected13/DACE15 evidence는 무효화되지 않는다.

Migration: v1.3.6에서 DAO-P0-04는 implementation-ready가 아니다. representative Plan v0.3.12의 blocking P1 4건을 correction/re-review로 닫은 뒤에만 Ammo DACE descriptor/probe/history bootstrap을 구현한다. 기존 Missile DACE15와 accepted history는 compatibility baseline으로 그대로 보존한다.

### v1.3.5 - 2026-09-10

- `CF-FQ-051 DAO-P0-03 Correction + Re-review`에서 Mid-review의 blocking P1 1건/P2 1건을 전건 교정하고 current Source/Systems/Plan 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다.
- §2, §10, §18과 §20의 Current projection을 실제 Source에 맞춰 MissileGuidePreset + AmmoData exact2 `ReviewedMutationReady`와 `CFDADurableCore` 단일 durable transaction authority로 정렬했다. Provider는 typed materialize/extract/fingerprint 의미만 소유하고 Create/Update/Save/reload/readback/rollback algorithm은 타입별로 복제하지 않는다.
- `CFDAAmmoProvider.cpp v1.2.1`은 실행 동작 변경 없이 provider initialization/current resolver의 stale `read-only` readiness 주석만 current `ReviewedMutationReady` 계약으로 교정했다.
- fresh official UE 5.8 Build `bc420d5554e849fdbad32e65296a7188` PASS, DAO focused `b3773a90332b4d6295670a37f276d44f` exact7/7 PASS다. 실행 의미가 바뀌지 않은 comment/document-only correction이므로 affected CF-FQ-049 `bcb9e15f9b8942d886768896a35b0bf2` 13/13과 DACE `977d1bc1566f41d7b805e608cd8e1dcb` 15/15는 직전 동일 executable semantics의 fresh evidence를 유지하고 중복 재실행하지 않았다.
- correction 후 fresh AssetDump에서 `/Game/Test/CarFight/DAOAmmoP03` CFAmmoData asset count 0을 확인했고, 보호 exact9 scoped Git diff도 0으로 Missile Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, Ammo persisted HeavyFinite/RocketFinite exact2 mutation 0을 재확인했다.
- `CFDAContract*` current Source의 AmmoData/DACE-AmmoData/FCFDAAmmo match는 0으로 DAO-P0-04는 시작되지 않았다. DAO-P0-03 Technical PASS로 다음 exact Gate인 DAO-P0-04가 Ready가 되었을 뿐이다.

Migration: v1.3.5부터 AmmoData Reviewed durable writer는 DAO-P0-03의 Accepted Current contract다. MissileGuidePreset/AmmoData exact2는 모두 `ReviewedMutationReady`이고 durable sequencing은 `CFDADurableCore` 단일 authority를 재사용한다. 다음 DAO-P0-04 Ready 상태를 Ammo DACE 구현/acceptance로 해석하지 않는다.

### v1.3.4 - 2026-09-10

- `CF-FQ-051 DAO-P0-03 Post-Implementation Mid-review`를 current Source/Systems 기준으로 독립 수행했다. 판정은 `P0 0 / blocking P1 1 / P2 1 / HOLD`다.
- Runtime/authoring Source에서는 `CFDADurableCore`가 typed Staging durable Create/Update, exact single-package SavePackage, disk reload, typed semantic readback, rollback/uncertainty sequencing의 단일 authority임을 재확인했다. MissileGuidePreset과 AmmoData production provider exact2는 모두 `ReviewedMutationReady`이며 provider-local typed materializer/fingerprint/extractor만 각 타입 의미를 소유한다.
- blocking P1 1건은 이 Current Systems 문서 §10 `Typed Materializer`가 여전히 `지원 대상은 ... 하나` / `UCFMissileGuidePresetData` 전용 Create 흐름을 Current 설명으로 남겨 실제 exact2 + shared durable core Source와 모순되는 문제다. Current owner 문서가 다음 세션에 잘못된 구현 계약을 제공할 수 있으므로 Correction + Re-review 전 Technical PASS를 차단한다.
- P2 1건은 `CFDAAmmoProvider.cpp`의 `GetProvider()`/current resolver 인접 주석에 `read-only Ammo provider entry` 표현이 남아 실제 `ReviewedMutationReady` + non-null `ApplyReviewedMutation` 상태와 어긋나는 source-comment projection drift다. 실행 동작에는 영향이 없지만 교정 대상이다.
- fresh AssetDump에서 `/Game/Test/CarFight/DAOAmmoP03` + `CFAmmoData` asset count 0을 확인해 disposable durable fixture persisted residue 0을 재확인했다. 보호 exact9 scoped Git diff도 0으로 Missile Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, Ammo persisted HeavyFinite/RocketFinite exact2 mutation 0을 유지한다.
- `CFDAContract*` current Source에는 AmmoData/DACE-AmmoData/FCFDAAmmo implementation match가 0이므로 DAO-P0-04 Ammo DACE는 시작되지 않았다. 이번 mid-review는 Source mutation 0이므로 직전 fresh Build `0ef9b9c2d9724b749b4226153e8e8ba6`, DAO 7/7, affected 13/13, DACE 15/15 evidence를 무효화하지 않으며 중복 재실행하지 않았다.

Migration: v1.3.4는 DAO-P0-03 구현 자체를 되돌리지 않는다. 실제 Source는 Ammo durable writer까지 구현된 상태지만 Current Systems §10과 Ammo provider 잔여 주석을 current exact2 계약에 맞춘 뒤 재검수하기 전에는 DAO-P0-03 Technical Acceptance를 주장하거나 DAO-P0-04를 시작하지 않는다.

### v1.3.3 - 2026-09-10

- `CF-FQ-051 DAO-P0-03 Ammo Reviewed Apply / Durable Typed Writer` 구현 전 계약검수에서 `P0 0 / blocking P1 2 / P2 0`을 확인했다. P1은 Missile에 묶여 있던 durable transaction algorithm을 Ammo에 복제할 위험과 protected HeavyFinite/RocketFinite exact2를 writer fixture로 직접 사용할 위험이었다.
- provider-neutral `CFDADurableCore`를 추가해 CreatePackage/NewObject, pre-save typed readback, exact single-package SavePackage, disk reload, persisted typed semantic readback, Update rollback과 save uncertainty taxonomy를 단일 authority로 이동했다. 기존 Missile writer도 같은 core로 rewire해 Missile-only durable dead copy를 제거했다.
- Ammo provider는 `ReviewedMutationReady`로 승격하고 exact8 deterministic materializer + `ApplyReviewedMutation` callback을 등록했다. Shared Review/TOCTOU에서 fresh source/current truth가 승인 evidence와 일치한 경우에만 Ammo typed payload가 durable core에 전달된다.
- DAO-P0-03 durable regression은 `/Game/Test/CarFight/DAOAmmoP03/` + `Authoring/DataAssetStaging/AmmoData/__AutomationP03__/` disposable root만 사용한다. non-empty AmmoTags + non-null Texture2D AmmoIcon의 Create→Update durable round-trip, stale approval, pre-existing dirty target, forced SaveStateUnconfirmed을 actual Ammo provider 경로로 검증하고 residue 0을 확인했다.
- fresh official UE 5.8 Build `0ef9b9c2d9724b749b4226153e8e8ba6` PASS, DAO focused `719a71370ff142c78850a3bc867c1f52` 7/7 PASS, affected CF-FQ-049 `bcb9e15f9b8942d886768896a35b0bf2` 13/13 PASS, DACE `977d1bc1566f41d7b805e608cd8e1dcb` 15/15 PASS다.
- 보호 exact9 scoped diff는 Missile Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, Ammo persisted HeavyFinite/RocketFinite exact2 모두 0이다. Product canonical Ammo는 계속 exact0이며 CF-FQ-039 Active, 기존 병렬 dirty, commit/push 0을 보존했다.
- 현재 상태는 `Implementation + Fresh Validation PASS / Post-Implementation Mid-review Pending`이다. DAO-P0-03 Technical Acceptance나 DAO-P0-04 착수를 아직 주장하지 않는다.

Migration: v1.3.3부터 typed durable sequencing은 `CFDADurableCore` 단일 authority를 사용하고 provider는 typed materialize/extract/fingerprint 의미만 소유한다. AmmoData는 `ReviewedMutationReady`지만 Product canonical Ammo target은 여전히 exact0이며, DAO-P0-03 Post-Implementation Mid-review PASS 전 DAO-P0-04 DACE로 넘어가지 않는다.

### v1.3.2 - 2026-09-10

- `CF-FQ-051 DAO-P0-02 Correction + Re-review`를 current Source 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 닫았다. Post-Implementation Mid-review의 P1 2건과 P2 2건을 전건 교정했다.
- common envelope field인 `TargetObjectPath` canonical validation과 `BaseSemanticFingerprint` null/string + canonical SHA-256 physical parse/validation을 `CFDACommonPrimitives` 단일 provider-neutral authority로 통합했다. 기존 Missile compatibility path와 Ammo provider는 동일 helper를 호출하며 타입별 diagnostic/acceptance drift 지점을 제거했다.
- actual `CFDAAmmoProvider`가 만든 두 payload-free Preview row를 `BuildCommonBatchPlanHash()`에 투입해 physical row order reversal에도 같은 canonical hash가 나오는 integration regression을 추가했다. `AmmoDisplayName` empty Literal과 `AmmoFamilyId=NAME_None` 허용 semantic도 actual focused regression으로 고정했다.
- fresh official UE 5.8 Build `898d428caeba4895bb5b0668af513a6e` PASS, DAO focused `b6cb4476ec6d47f4a50214901498b0ed` 4/4 PASS, affected CF-FQ-049 `d6e52207e9d8436d9fae7c27f2027d80` 13/13 PASS, DACE `8789b7e7a1c2446dae91db3bf3f9f7c2` 15/15 PASS다.
- 보호 scoped diff에서 Missile Product canonical JSON/uasset exact3, accepted `CFDAContractBase.cpp`, Ammo persisted exact2가 모두 diff 0임을 확인했다. Ammo provider는 계속 `ReadOnlyPreviewReady` / `ApplyReviewedMutation=nullptr`이며 DAO-P0-03 writer/materializer는 아직 구현하지 않았다.

Migration: v1.3.2부터 새 typed provider는 common envelope `TargetObjectPath`와 `BaseSemanticFingerprint` 규칙을 provider-local로 복제하지 않고 `CFDACommonPrimitives` authority를 재사용한다. DAO-P0-02 Technical PASS로 다음 DAO-P0-03 설계·구현 Gate는 Ready지만, 이는 Ammo durable Apply/Save가 현재 구현됐다는 의미가 아니다.

### v1.3.1 - 2026-09-10

- `CF-FQ-051 DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview`의 실제 구현 상태를 Current System에 동기화했다. `CFDAAmmoProvider`가 CFAmmoData exact8 typed payload/record, strict parser, semantic fingerprint, UObject extractor/current-state resolver와 `ReadOnlyPreviewReady` provider를 소유한다.
- Production registry는 MissileGuidePreset `ReviewedMutationReady` + AmmoData `ReadOnlyPreviewReady` exact2다. Ammo `ApplyReviewedMutation=nullptr`을 유지하므로 현재 단계는 read-only typed Preview 지원이며 durable Apply/Save는 아직 지원하지 않는다.
- AmmoTags set-like FName semantic/duplicate reject/order-independent fingerprint, AmmoIcon canonical SoftObjectPath + Asset Registry metadata-only Texture2D compatibility validation을 현재 구현 계약으로 기록했다.
- DAO-P0-02 final fresh evidence는 official UE 5.8 Build `32f46c889baf4bfdad6b1eebf7b2e0b1` PASS, focused `381bf95e6628439585157d0d3e46d0fa` 4/4 PASS, affected CF-FQ-049 `2d0e1e80e0244dc0b9f0520ffd249f01` 13/13 PASS, DACE `adb7c455699742f980e70a8d22aff4d5` 15/15 PASS다.
- Product canonical exact3, accepted baseline, Ammo persisted exact2 mutation은 0으로 유지됐다. 이 문서 갱신은 DAO-P0-02 Post-Implementation Mid-review 전 current implementation projection이며 DAO-P0-03 writer 승인을 의미하지 않는다.

Migration: v1.3.1부터 AmmoData는 `ReadOnlyPreviewReady` 타입으로 현재 구현에 존재한다. 이를 durable authoring 완료 또는 DACE accepted 타입으로 확대 해석하지 않는다. Ammo durable Apply/Save는 DAO-P0-03, independent DACE contract/history는 DAO-P0-04가 소유하며 각 Gate PASS 전까지 현재 read-only 경계를 유지한다.

### v1.3.0 - 2026-09-09

- `CF-FQ-051 DAO-P0-01 Correction + Re-review`에서 Mid-review blocking P1 4건/P2 2건을 전건 교정하고 current Source + fresh final validation 기준 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 shared multi-type foundation을 승인했다.
- shared Preview/duplicate/BatchPlanHash/Reviewed Approval/TOCTOU를 payload-free common row로 이동하고 complete exact TypeKey provider operation registry를 확정했다. typed payload는 provider-local 경계에만 유지하며 existing Public Missile API는 compatibility facade로 보존한다.
- fresh Apply/TOCTOU는 reviewed source를 다시 읽고 exact provider entry의 parse/current/apply operation으로 dispatch한다. 기존 sibling Automation fixture는 Missile provider root 하위로 이동했고 Development Editor trust boundary를 넓히던 compile-time sibling allowlist는 제거했다.
- second-provider foundation seam과 duplicate exact TypeKey fail-closed regression을 보강했다. same textual StableLogicalId는 다른 exact class namespace에서 허용하고 TargetObjectPath duplicate는 계속 global blocker다.
- final official UE 5.8 Build `2f8afa3de9144c55b74083ca4eb02aeb` PASS, affected Staging `ad820e42c46545c182b319c9d872b724` exact13/13 PASS, DACE `9857fc2e265a419bb7d75c8df7f9e454` exact15/15 PASS다. Product exact3 JSON/uasset diff 0 / Apply·Save 0 / accepted snapshot source diff 0 / bootstrap exact1 append 0을 유지했다.
- Ammo typed provider/schema/DACE extension은 아직 구현하지 않았다. exact next는 representative Plan v0.3.2의 `DAO-P0-02 Ammo Typed Schema / Parse / Fingerprint / Preview`다.

Migration: v1.3.0부터 DAO-P0-01 shared Type Dispatch foundation은 후속 typed DataAsset onboarding이 재사용할 수 있는 Current internal contract다. Production registered provider는 여전히 MissileGuidePreset exact1이며 AmmoData는 DAO-P0-02 이후 해당 typed provider/contract가 PASS하기 전까지 Current 지원 타입으로 해석하지 않는다.

### v1.2.1 - 2026-09-09

- `CF-FQ-051 DAO-P0-01 Post-Implementation Mid-review` 결과를 반영해 v1.2.0의 `Technical PASS` 표현을 취소하고 `P0 0 / blocking P1 4 / P2 2 / HOLD`로 교정했다.
- current Type Dispatch source는 실제 존재하지만 shared core가 Missile-only `FCFDAStagingRecord/PreviewRow`에 직접 의존하고, registry가 descriptor-only이며, fresh Apply/TOCTOU가 direct Missile facade를 호출하므로 reusable multi-type foundation으로 아직 승인하지 않는다.
- `WITH_DEV_AUTOMATION_TESTS` sibling fixture root allowlist는 Development Editor trust boundary를 넓힐 수 있어 provider-owned exact-root 계약의 임시 미해결 항목으로 기록했다.
- official Build PASS + affected Staging 13/13 + DACE 15/15, Public header diff 0, Product exact3 Apply·Save 0 / JSON·uasset diff 0 / accepted bootstrap exact1 append 0은 Missile parity evidence로 보존한다.
- DAO-P0-02는 HOLD이며 exact next는 representative Plan v0.3.1의 `DAO-P0-01 Correction + Re-review`다.

Migration: v1.2.1은 CF-FQ-049/050의 완료 Current 계약을 변경하지 않는다. CF-FQ-051 Type Dispatch source는 correction/re-review PASS 전까지 provisional implementation으로 취급하며 새 타입 onboarding의 승인된 재사용 contract로 사용하지 않는다.

### v1.2.0 - 2026-09-09

- `CF-FQ-051 DAO-P0-01 Shared Type Dispatch Foundation + Missile Parity Rewire` Technical PASS를 Current implementation checkpoint로 반영했다. Editor Private `CFDATypeDispatch`와 `CFDAMissileProvider`가 payload-free common envelope, exact trusted TypeKey registry, provider-owned StagingRoot, class-scoped StableLogicalId와 Missile typed callback ownership을 제공한다.
- 기존 Public Missile Staging/Ops/Apply API는 signature delta 0 compatibility facade로 유지하며 current Production registered provider는 MissileGuidePreset exact1이다. Ammo는 아직 Current 지원 타입으로 승격하지 않는다.
- 새 provider-root enforcement가 기존 focused Automation fixture roots를 차단한 regression은 Production 경계를 완화하지 않고 `WITH_DEV_AUTOMATION_TESTS` exact allowlist로 교정했다.
- final UE 5.8 Build `6737c1bff29a40a4bfd9bb6be98213b1` PASS, affected Staging `34de9e0426dc4f33ae05a76195bc6190` 13/13 PASS, DACE `be73e54a237c4ea8aa20b4e2a5c001e1` 15/15 PASS다.
- Product exact3 JSON/uasset diff 0, Apply·Save 0, accepted snapshot source diff 0 / bootstrap exact1 / append 0과 CF-FQ-039 Active·기존 병렬 dirty 보존을 확인했다.

Migration: v1.2.0부터 새 DataAsset type onboarding은 arbitrary Reflection writer나 기존 Missile Public DTO 확장이 아니라 trusted TypeKey provider + 별도 typed record/adapter를 추가하는 방향을 사용한다. 현재 이 foundation의 Production provider는 MissileGuidePreset exact1이며 Ammo는 CF-FQ-051 후속 Gate가 구현한다.

### v1.1.0 - 2026-09-09

- Post-closure 최종검수 P2 문서 교정으로 §20의 부정확한 `CarFight_ReEditor/DataAuthoring` 축약 경로를 실제 `Public/DataAuthoring/` + `Private/DataAuthoring/` Source 경계로 교정했다. 구현·Source·Asset·검증 evidence와 Current owner version은 변경하지 않는다.
- `CF-FQ-050 Data Asset Contract Evolution Guard` P0 완료 계약을 기존 Data Asset Authoring Current System에 승격했다.
- MissileGuidePreset의 SourceShape exact30 / AdapterShape exact42 / SourceAdapterMapping exact38 / SemanticContract exact26 descriptor, deterministic four-signature, production-owned append-only accepted history와 production behavior probe 기반 drift guard를 Current 계약으로 추가했다.
- SchemaRevision/AdapterContractRevision bump 누락, current change declaration binding, migration impact/Resolution/Evidence, canonical Product Staging old-revision/incompatibility와 Pending accepted append/Current promotion 차단을 현재 운영 규칙으로 기록했다.
- final P0-05 evidence는 official Build PASS + DACE exact15/15 + affected CF-FQ-049 exact13/13 + OperationalEntry exact1/1이며 fixture/Product Staging residue는 0이다.
- CF-FQ-050 closure에서도 Product Low/Normal/High ApplyReviewed/UE Asset Save 0, canonical Product Staging mutation 0, exact3 JSON·uasset diff 0, accepted snapshot append 0을 유지했다. CF-FQ-039 Active lifecycle과 기존 병렬 dirty는 변경하지 않았다.
- 다른 DA 타입은 이 Guard interface를 선행조건으로 사용할 수 있지만 자동 onboarding되지 않으며 Typed Adapter/Schema/descriptor/probe를 별도 lifecycle에서 구현해야 한다.

Migration: v1.1.0부터 Staging 지원 DataAsset의 C++ authoring contract를 변경할 때는 기존 parser/materializer 수정만으로 종료하지 않는다. DACE structural/behavior guard, revision 판단, explicit migration impact, 필요한 Staging migration/Product review와 affected regression을 통과한 뒤에만 새 accepted snapshot append를 검토한다. Guard 자체는 Product write/save 권한을 가지지 않는다.

### v1.0.1 - 2026-09-09

- CF-FQ-049 post-promotion 최종검수에서 확인한 Current Systems FText 표현 P1 1건을 교정했다.
- AdapterContractRevision 2 Source와 동일하게 source-backed Literal만 authored semantic으로 취급하고, Unreal 저장 과정에서 자동 부여된 package-only namespace / stable key는 persistence metadata로 semantic identity에서 제외한다고 명시했다.
- StringTable, 명시적 authored namespace localization identity, source string을 lossless하게 보존할 수 없는 generated/formatted FText는 계속 fail-closed한다.
- Source/Asset/Build/Automation 계약은 변경하지 않았고 Product Low/Normal/High Apply·Save 0과 CF-FQ-039 Active lifecycle을 유지한다.

Migration: v1.0.0의 `namespace-key localization representation` 포괄 표현은 폐기한다. 현재 FText 판단은 AdapterContractRevision 2 Source의 package-only persistence metadata 예외와 authored localization identity 차단을 구분해서 해석한다.

### v1.0.0 - 2026-09-09

- `CF-FQ-049 Data Asset Staging·Batch Authoring`의 P0 완료 계약을 Current System으로 신규 승격했다.
- Editor-off canonical JSON, typed strict schema, semantic fingerprint, 3-way Preview, exact selection, selection-bound fresh Review, one-shot approval와 TOCTOU 보호를 Current 계약으로 기록했다.
- typed `CFMissileGuidePresetData` materializer, exact single-package SavePackage, disk reload + typed semantic readback, uncertainty/PartialApplied taxonomy를 현재 durable write 계약으로 기록했다.
- `SyncProduct`의 converged-only baseline bootstrap/rebase와 touched-file raw-byte rollback, console operation entry 4종을 현재 운영 절차로 승격했다.
- official UE 5.8 Build PASS, OperationalEntry 1/1, existing DAS 13/13과 Current System Promotion 직전 fresh persisted AssetDump 3/3을 완료 evidence로 연결했다.
- CF-FQ-049 closure에서는 Product Low/Normal/High `ApplyReviewed`를 실행하지 않아 Product Apply·Save 0을 유지했고, 현재 단일 Active `CF-FQ-039` lifecycle도 변경하지 않았다.

---

## 20. Migration

- CF-FQ-051 DAO-P0-06 v1.4.1 Final Audit Correction + Re-review는 Current §2의 stale Ammo 미지원 표현과 완료 Feature header만 교정했다. 실행 계약은 v1.4.0과 동일하며 최종 판정은 `P0 0 / blocking P1 0 / P2 1 non-blocking / PASS`다. AmmoData는 Current 지원 exact2에 포함되고 DamageData/VehicleSensorData/기타 신규 타입은 별도 onboarding 대상이다.
- CF-FQ-051 DAO-P0-06 v1.4.0에서 second onboarding reuse measurement와 Current System Promotion을 완료했다. MissileGuidePreset+AmmoData exact2는 Current 지원이며 third type는 typed provider + DACE owner + provider registration + explicit admission으로 추가하고 shared Preview/Review/TOCTOU/Apply/Durable/DACE core를 복제하지 않는다. hybrid legacy/shared physical ownership P2는 non-blocking이며 실제 반복 비용이 확인될 때만 별도 maintenance한다.
- CF-FQ-051 DAO-P0-05 v1.3.14 Post-Implementation Mid-review는 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`이며 Technical Acceptance가 완료됐다. Third type는 shared operational algorithm을 재작성하지 않고 provider registration + explicit admission으로 확장한다. 다음 exact Gate는 DAO-P0-06 Ready / Not Started이며 Current System Promotion 완료 전 AmmoData 최종 지원 승격을 주장하지 않는다.
- CF-FQ-051 DAO-P0-05 v1.3.13부터 actual MissileGuidePreset+AmmoData provider-neutral mixed Explicit Paths operational session이 구현되어 fresh Build + focused4 + affected13 + Missile DACE15를 PASS했다. JSON read 전 exact provider-root owner resolution, same path/mode fresh Review, Product Ammo exact0와 existing sequential/PartialApplied semantics를 유지한다. 다음 exact Gate는 `DAO-P0-05 Post-Implementation Mid-review`이며 PASS 전 DAO-P0-06으로 승격하지 않는다.
- CF-FQ-051 DAO-P0-04 v1.3.10 Correction + Re-review 결과는 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`다. production fingerprint는 shared token probe + independent expected manifest로 omission을 fail-closed하고, accepted Ammo exact1은 dedicated `CFDAAmmoDaceBase.cpp` append-only owner가 소유한다. 다음 exact Gate는 `DAO-P0-05 — Canonical Ammo Staging Pilot / Multi-Type Integration`이며 아직 Not Started다.
- CF-FQ-051 DAO-P0-04 v1.3.9 Mid-review 결과는 `P0 0 / blocking P1 1 / P2 1 / HOLD`다. 현재 구현과 직전 fresh validation은 유지하지만 production fingerprint exact token-coverage drift observation P1과 accepted Ammo history dedicated append-only owner separation P2를 Correction + Re-review로 닫기 전에는 Technical Acceptance/DAO-P0-05 진입을 허용하지 않는다.
- CF-FQ-051 DAO-P0-04 v1.3.8부터 Ammo independent DACE descriptor/probe/bootstrap implementation은 완료됐고 fresh Build + DAO13 + existing Missile DACE15가 PASS했다. Ammo provider는 DACE `ContractReady`, independent history는 `DACE-AmmoData` exact1, Product canonical target은 explicit exact0이다. 다만 Post-Implementation Mid-review 전에는 DAO-P0-04 Technical Acceptance 또는 DAO-P0-05 Ready로 확대하지 않는다.
- CF-FQ-051 DAO-P0-04 v1.3.7부터 shared DACE correction은 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`다. per-TypeKey DACE readiness/history/revision/canonical-target gate, `AmmoTags[]` element observation과 `AmmoIcon` SoftObject target-class Reflection은 구현 전 공용 contract로 승인됐으며, 다음 단계는 provider-local Ammo descriptor/probe/`DACE-AmmoData` bootstrap 구현이다. Ammo DACE가 `ContractReady`가 되기 전에는 accepted append/Current promotion을 허용하지 않고 Product canonical Ammo exact0을 유지한다.
- CF-FQ-051 DAO-P0-04 v1.3.6 pre-review 결과, Ammo DACE 구현 전 `per-TypeKey DACE authority`, `AmmoTags[] element observation`, `AmmoIcon SoftObject target-class Reflection`, `per-TypeKey canonical Staging target set + Ammo exact0 semantics`의 blocking P1 4건을 먼저 교정한다. 기존 Missile `FCFDAContractGuard`/DACE15/accepted history는 compatibility baseline이며 Ammo history와 혼합하지 않는다.
- CF-FQ-051 DAO-P0-02 v1.3.2부터 common envelope `TargetObjectPath` canonical validation과 `BaseSemanticFingerprint` physical parse/validation은 `CFDACommonPrimitives` 단일 authority가 소유한다. 후속 provider는 이 규칙을 타입별로 다시 구현하지 않는다.
- CF-FQ-051 DAO-P0-01 shared foundation은 v1.3.0 Correction + Re-review Technical PASS 이후 승인된 Current internal 재사용 계약이다. 후속 타입은 complete exact TypeKey provider entry + 별도 typed record/adapter를 추가하며 기존 Missile Public compatibility facade를 generic arbitrary payload surface로 바꾸지 않는다.
- current Production provider registry는 MissileGuidePreset `ReviewedMutationReady` + AmmoData `ReviewedMutationReady` exact2다. DAO-P0-03 Correction + Re-review는 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`이며 durable transaction sequencing은 `CFDADurableCore` 단일 authority다. Product canonical Ammo는 exact0이고 Ammo DACE는 independent `CFDAAmmoDace` + `DACE-AmmoData` bootstrap exact1로 구현됐다. DAO-P0-04 Post-Implementation Mid-review 전에는 이 구현 상태를 Technical Acceptance 또는 P0-05 진입 승인으로 확대 해석하지 않는다.
- CF-FQ-050 완료 이후 Staging 지원 DataAsset의 contract evolution 판단은 이 문서와 실제 `CFDAContractGuard*` / typed Staging Source를 함께 우선한다.
- CF-FQ-049 완료 이후 현재 구현 판단은 이 문서와 실제 `UE/Source/CarFight_ReEditor/Public/DataAuthoring/` + `UE/Source/CarFight_ReEditor/Private/DataAuthoring/` Source 경계를 우선한다.
- `Document/Plan/DataAssetStaging/DataAssetStagingPlan.md`는 완료 당시 상세 설계·검수·Build/Automation evidence를 보존하는 Historical 문서로 사용한다.
- 기존 `DataAssetManagement.md`는 계속 read-first Data Asset Manager owner이며 write/apply authority를 이 문서와 합치지 않는다.
- Product Low/Normal/High canonical Staging JSON은 Git-reviewable authoring input으로 유지하지만 persisted `.uasset`이 최종 Source of Truth다.
- 새 DataAsset 타입 확장은 current MissileGuidePreset adapter에 arbitrary Reflection을 붙이지 말고 별도 Typed Adapter/Schema와 검증 lifecycle을 추가한다.
