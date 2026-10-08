# CarFight — Data Asset Contract Evolution Guard Plan

- 문서 버전: v0.7.0
- 최근 갱신일: 2026-09-09
- 문서 상태: Historical / P0 Complete / DACE-P0-06 Final Acceptance PASS / Current System Promotion Complete / G5 Deferred / Retained Path
- Feature ID: `CF-FQ-050`
- Feature 이름: `Data Asset Contract Evolution Guard`
- 우선순위: P2
- 선행 Current System: `CF-FQ-049 Data Asset Staging·Batch Authoring` Done
- 현재 Current owner: main_game `Document/Systems/DataManagement/DataAssetAuthoring.md v1.1.0`
- 첫 Pilot: `UCFMissileGuidePresetData`
- 현재 정확한 Gate: `None — P0 Complete / Historical`

---

## 1. 목적

CF-FQ-049는 Editor-off JSON Staging → strict typed Preview → fresh Review → explicit ApplyReviewed → exact SavePackage → disk reload semantic readback의 안전한 DataAsset authoring 기반을 완료했다.

하지만 현재 P0는 `CFMissileGuidePresetData` 한 타입에 대해 현재 시점의 schema/adapter 계약을 안전하게 적용하는 데 집중했고, 이후 개발 과정에서 DataAsset C++ authoring 구조가 진화할 때 Staging Adapter·Revision·canonical JSON이 뒤처지는 상태를 자동으로 잡는 유지보수 축은 별도 계약으로 만들지 않았다.

CF-FQ-050의 목적은 다음 한 문장으로 고정한다.

> **Staging을 지원하는 DataAsset의 C++ authoring contract가 바뀌었는데 Typed Adapter / Revision / canonical Staging migration 판단이 뒤처진 상태는 Automation PASS가 절대 나오지 않게 한다.**

이 Feature는 CF-FQ-049의 durable write 시스템을 다시 만드는 작업이 아니다.

---

## 2. 문제 정의

현재 MissileGuidePreset에는 다음 Current 구조가 존재한다.

```text
UCFMissileGuidePresetData
- PresetId
- PresetDisplayName
- PresetDescription
- MissileGuideConfig
```

그리고 CF-FQ-049 Staging은 별도의 typed DTO와 strict JSON schema를 가진다.

```text
FCFDAMissilePresetPayload
- PresetId
- PresetDisplayName
- PresetDescription
- MissileGuideConfig

SchemaRevision = 1
AdapterContractRevision = 2
```

향후 미사일 개발 중 예를 들어 다음과 같은 새 authored data가 필요해질 수 있다.

```text
MissileGuideConfig.SeekerResponseTimeSeconds 추가
```

현재 구조만으로는 개발자가 다음 연쇄 변경을 모두 기억해야 한다.

```text
C++ DataAsset / nested authored struct 변경
→ typed Source↔Adapter mapping 판단
→ strict JSON physical shape / parser 판단
→ Product→Staging canonical serializer 판단
→ extractor / semantic fingerprint 판단
→ materializer coverage 판단
→ SchemaRevision 또는 AdapterContractRevision 판단
→ canonical Staging migration 판단
→ Product migration review 필요 여부 판단
→ focused regression
```

현재 materializer는 `MissileGuideConfig`를 whole-struct assignment하므로 nested field 추가가 곧 per-field materializer 수정이라는 뜻은 아니다. Guard는 실제 execution coverage를 검증하며, 존재하지 않는 수동 작업을 요구하지 않는다.

이 중 하나가 누락돼도 compile 자체는 성공할 가능성이 있다.

CF-FQ-050은 이 contract evolution 누락을 fail-closed하는 것이 목적이다.

---

## 3. 현재 확인된 Source Truth

### 3.1 Product DataAsset

현재 `UCFMissileGuidePresetData`는 passive data container다.

```text
UE/Source/CarFight_Re/Public/CFMissileGuidePresetData.h
```

현재 top-level authoring property:

```text
PresetId: FName
PresetDisplayName: FText
PresetDescription: FText
MissileGuideConfig: FCFMissileGuideConfig
```

### 3.2 Staging Adapter

현재 typed Staging public contract:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFDAStaging.h
```

현재 MissileGuidePreset:

```text
SchemaId = CarFight.DataAsset.MissileGuidePreset
SchemaRevision = 1
AdapterContractRevision = 2
```

현재 old revision은 silent migration하지 않고 `SchemaRevisionUnsupported` 또는 `AdapterRevisionMismatch`로 fail-closed한다.

### 3.3 Current System 보호

CF-FQ-050은 다음 CF-FQ-049 완료 계약을 재구현하지 않는다.

```text
strict JSON parse
semantic fingerprint
3-way Preview
exact selection
fresh Review
one-shot approval
TOCTOU preflight
TargetDirtyUnowned
exact SavePackage
disk reload semantic readback
SaveStateUnconfirmed / PartialApplied
SyncProduct raw-byte rollback
```

새 failure evidence가 없는 한 기존 Build/Automation evidence도 단순 승격을 이유로 반복하지 않는다.

---

## 4. 핵심 설계 원칙

### 4.1 모든 EditAnywhere를 자동 Staging-owned로 보지 않는다

Reflection에서 `EditAnywhere`라고 보인다는 이유만으로 모든 property를 Staging writable contract에 자동 포함하지 않는다.

DataAsset에는 다음처럼 Editor에서 보이더라도 Staging이 소유하면 안 되는 값이 생길 수 있다.

```text
legacy compatibility field
transient authoring helper
runtime-generated metadata
다른 전용 authoring workflow가 소유하는 field
```

따라서 CF-FQ-050은 명시적인 **Staging-owned authoring root/field contract**를 가진다.

첫 Pilot에서는 MissileGuidePreset 전체 writable record와 그 안의 `MissileGuideConfig` authored field set을 기준으로 한다.

### 4.2 Structural Drift는 자동 검출한다

C++ reflected property tree와 Adapter가 선언한 writable contract를 deterministic canonical representation으로 비교한다.

검출 대상 예:

```text
field 추가
field 삭제
field rename
property type 변경
nested struct field 추가/삭제/rename/type 변경
array/map/set/container shape 변경
Staging-owned root 변경
```

현재 Adapter가 구조 변화를 반영하지 않았다면 focused Automation은 FAIL해야 한다.

### 4.3 Semantic Meaning 전체를 코드가 자동 추론한다고 주장하지 않는다

다음 변화는 Reflection만으로 완전 자동 추론할 수 없다.

```text
같은 float지만 단위 의미 변경
허용 range 변경
identity 의미 변경
fingerprint canonicalization 의미 변경
FText semantic 해석 변경
resolver/path precedence 변경
validation 규칙 변경
```

P0가 자동 보장하는 범위는 다음으로 제한한다.

```text
Structural change
→ native Reflection과 explicit Source descriptor의 drift를 자동 검출

Declared semantic contract change
→ explicit semantic policy/signature가 accepted baseline과 달라졌는데
   AdapterContractRevision이 전진하지 않으면 FAIL

Descriptor가 표현하는 semantic policy
→ 가능한 항목은 behavior probe로 실제 parser/fingerprint/extractor 동작과 대조
```

반대로 개발자가 실제 semantic implementation을 바꾸고 semantic descriptor와 관련 focused semantic regression을 모두 고치지 않은 arbitrary code change까지 Guard가 스스로 추론한다고 주장하지 않는다. 그 영역은 affected semantic Automation + code review 책임으로 남긴다.

따라서 P0 보장은 **"명시적으로 선언된 semantic contract 변화의 revision 누락과, descriptor로 표현 가능한 semantic policy의 실행 불일치 차단"**이다.

### 4.4 자동 migration은 P0 목표가 아니다

old JSON을 새 schema로 자동 변환하지 않는다.

새 field의 실제 값도 자동으로 추측하지 않는다.

Product `.uasset`도 자동 저장하지 않는다.

P0가 자동화할 것은:

```text
변화 감지
revision 요구
migration required 판정
누락 차단
```

까지다.

---

## 5. 확정 Contract Model — DACE-P0-00 Correction

DACE-P0-00 재검수에서 구현 surface와 authority를 다음으로 확정한다.

### 5.1 Staging-owned Source Contract

첫 Pilot의 Source ownership은 exact 다음 범위다.

```text
UCFMissileGuidePresetData
- PresetId: FName
- PresetDisplayName: FText
- PresetDescription: FText
- MissileGuideConfig: FCFMissileGuideConfig

FCFMissileGuideConfig
- current authored UPROPERTY exact 26
```

Source structural signature는 실제 native Reflection에서 위 explicit root만 순회해 만든다. `EditAnywhere` 전체 자동 포함은 금지한다.

canonical token은 최소 다음을 가진다.

```text
SourceRootClassPath
SourcePropertyPath
PropertyKind
ReflectedTypePath
ContainerKind
NestedStructPath
```

정렬은 full source property path 기준 ordinal deterministic sort다. 출력은 `SourceShapeSignature`다.

### 5.2 Adapter Physical Shape Contract

JSON physical shape는 Source와 독립된 descriptor로 표현한다.

구분:

```text
Adapter envelope
- SchemaId
- SchemaRevision
- AdapterContractRevision
- DataAssetTypeClassPath
- StableLogicalId
- TargetObjectPath
- BaseSemanticFingerprint
- Payload

Payload writable shape
- PresetId
- PresetDisplayName.{Kind,Text}
- PresetDescription.{Kind,Text}
- MissileGuideConfig.<exact 26 fields>
```

각 Adapter field descriptor는 최소 다음을 가진다.

```text
AdapterJsonPath
JsonValueKind
PresencePolicy = Required/Optional
NullPolicy = NonNull/Nullable
RepresentationKind
SemanticRole
```

`BaseSemanticFingerprint`처럼 **field 자체는 required지만 Create에서는 JSON null을 허용**하는 계약을 `Required + Nullable`로 정확히 표현한다. field 존재 여부와 null 허용 여부를 하나의 Optional 개념으로 뭉개지 않는다.

출력은 `AdapterShapeSignature`다.

### 5.3 Explicit Source↔Adapter Mapping Descriptor

`SourceShapeSignature == AdapterShapeSignature` 직접 비교는 사용하지 않는다.

대신 mapping descriptor가 Source authored leaf와 Adapter representation의 관계를 명시한다.

예:

```text
PresetId
→ Payload.PresetId / NameToken

PresetDisplayName
→ Payload.PresetDisplayName / LiteralFText
   - Kind = constant "Literal"
   - Text = source string

MissileGuideConfig.GuidanceLaw
→ Payload.MissileGuideConfig.GuidanceLaw / EnumNameToken
```

한 Source property가 여러 JSON child로 표현되는 것을 허용하고, envelope처럼 Source property가 없는 Adapter-only metadata도 별도 role로 허용한다.

mapping을 deterministic canonicalize한 결과는 `SourceAdapterMappingSignature`다.

합격 조건은 hash 직접 동등 비교가 아니라 다음 세 관계다.

```text
Reflected Source shape == Source descriptor
Adapter descriptor == actual strict JSON behavior
Source↔Adapter mapping의 source side == Staging-owned Source authored leaf set
Source↔Adapter mapping의 adapter side == writable Adapter semantic field set
```

### 5.4 Descriptor ↔ 실제 Execution Surface Coverage

Descriptor는 계약 기준이지만 parser/serializer/fingerprint/materializer를 generic reflection writer로 교체하지 않는다.

대신 actual production behavior를 다음 probe로 대조한다.

```text
A. Canonical Serializer Probe
- CFDAStagingOps.cpp의 실제 Product→Staging serializer를 그대로 호출
- sentinel payload를 JSON으로 만든 뒤 actual JSON tree path/type를 수집
- Adapter descriptor와 exact 비교

B. Strict Parser Probe
- production serializer output이 current parser에서 valid여야 함
- descriptor의 required writable field를 하나씩 제거하면 MissingRequiredField
- descriptor type과 다른 JSON type이면 TypeMismatch/Invalid
- unknown sibling field를 넣으면 UnknownField

C. Fingerprint Probe
- production semantic token append 경로가 실제로 emit한 semantic token label set을 관측
- `FingerprintTokenDescriptor`와 exact 비교
- descriptor는 fixed contract token `SchemaId / SchemaRevision / AdapterContractRevision / DataAssetTypeClassPath`와 mapped payload semantic leaf token을 모두 포함
- hash byte algorithm/ordering 자체는 기존 CF-FQ-049 semantic contract를 재작성하지 않음

D. Extractor ↔ Materializer Probe
- test-owned transient asset/payload에 각 source authored leaf를 구별 가능한 sentinel 값으로 채움
- production materializer 적용 → production extractor readback
- typed source-string/FName/enum/float/bool semantic equality와 final semantic fingerprint equality 확인
- current `MissileGuideConfig` whole-struct assignment은 nested exact26을 한 번에 포괄하므로 per-field materializer list를 새로 만들지 않음
```

이 네 probe가 descriptor와 동시에 통과해야 한다. 단순 coverage-name 배열만 맞추는 것으로 PASS할 수 없다.

기존 `CFDAStagingPilotTests.cpp`의 fixture serializer는 Historical CF-FQ-049 test helper일 뿐 새 contract authority로 승격하지 않는다. CF-FQ-050은 production `CFDAStagingOps.cpp` serializer를 직접 관측하며 기존 exact test identity를 재작성하지 않는다.

### 5.5 Explicit Semantic Contract Descriptor

semantic descriptor에는 Reflection으로 얻을 수 없는 **명시적 정책**만 둔다.

첫 Pilot 최소 대상:

```text
Literal FText source-string representation policy
FName semantic case policy
Enum exact source token/case policy
numeric finite/range policy
StableLogicalId ↔ Payload.PresetId identity policy
semantic fingerprint inclusion/token policy
resolver/path precedence 중 Staging semantic contract에 포함되는 versioned policy token
```

가능한 정책은 behavior probe에서 실제 구현과 대조한다. 예를 들어 numeric range는 descriptor의 min/max에서 boundary/overflow fixture를 만들어 parser behavior와 비교한다.

이 descriptor의 deterministic hash가 `SemanticContractSignature`다.

arbitrary implementation meaning change를 이 signature가 자동 발견한다고 주장하지 않는다.

### 5.6 Accepted Revision Snapshot Authority

accepted baseline은 **test fixture가 아니라 production Editor module의 별도 source file**이 소유한다.

P0 구현 배치 기본안:

```text
신규
Private/DataAuthoring/CFDAContractGuard.h
Private/DataAuthoring/CFDAContractGuard.cpp
Private/DataAuthoring/CFDAContractBase.cpp
Private/DataAuthoring/CFDAContractGuardTests.cpp

narrow existing-source probe 연결
Private/DataAuthoring/CFDAStaging.cpp
Private/DataAuthoring/CFDAStagingOps.cpp
Private/DataAuthoring/CFDAStagingApply.cpp
```

모든 신규 파일명은 32자 이하이며 Public API/Blueprint surface를 추가하지 않는다.

`CFDAContractGuard.h`는 CarFight_ReEditor **Private module-internal header**다. existing source 3개에는 production helper를 새 generic framework로 이동시키지 않고 다음 narrow probe adapter만 연결한다. probe entry는 `WITH_DEV_AUTOMATION_TESTS` 경계 안에서만 노출해 Shipping/runtime API나 Blueprint surface를 만들지 않는다.

```text
CFDAStaging.cpp
- Source reflection/strict parser/fingerprint token/extractor 실제 behavior 관측

CFDAStagingOps.cpp
- 현재 anonymous `BuildProductStagingJson` production serializer를 호출하는 private contract probe

CFDAStagingApply.cpp
- 현재 `ApplyPayloadToAsset` production materializer를 호출하는 private contract probe
```

probe는 Product target path를 소유하지 않고 test-owned transient DTO/UObject만 받는다. Product Sync/Apply/Save entry를 호출할 수 없다. 따라서 Guard를 위해 serializer/materializer를 Public service로 승격하거나 CF-FQ-049 operational API를 넓히지 않는다.

역할:

```text
CFDAContractGuard.*
- current descriptor
- reflection/signature/probe orchestration
- developer-only issue/result

CFDAContractBase.cpp
- accepted snapshot append-only record authority
- current bootstrap baseline: SchemaRevision 1 / AdapterContractRevision 2

CFDAContractGuardTests.cpp
- CF-FQ-050 focused Automation only
```

accepted snapshot 최소 필드:

```text
SnapshotId
PreviousSnapshotSignature
SchemaId
SchemaRevision
AdapterContractRevision
SourceShapeSignature
AdapterShapeSignature
SourceAdapterMappingSignature
SemanticContractSignature
MigrationImpact
MigrationResolution
MigrationEvidenceId
```

current `SchemaId / SchemaRevision / AdapterContractRevision / DataAssetTypeClassPath`의 runtime authority는 기존 `FCFDAStagingService` getter/constant를 유지한다. current descriptor가 revision 숫자를 별도 authoritative copy로 다시 소유하지 않는다. accepted snapshot은 당시 승인값을 history로 기록하고 Guard가 current service value와 비교한다.

현재 descriptor와 accepted baseline을 같은 struct literal 한 곳에서 동시에 덮어쓰는 구조는 금지한다.

새 accepted snapshot은 기존 record를 수정하지 않고 append한다. Automation은 revision monotonicity, `PreviousSnapshotSignature` chain과 snapshot id uniqueness를 검사한다. Git history를 무시하고 과거 record를 고의로 재작성하는 행위까지 runtime code가 방지한다고 주장하지 않으며, append-only historical edit 여부는 code review/Git diff 책임이다.

### 5.7 Migration Impact Declaration + Resolution

contract signature가 latest accepted snapshot과 달라진 경우 migration impact 선언이 필수다.

accepted snapshot에 들어가기 **전**의 current change declaration은 `CFDAContractGuard.cpp`의 별도 candidate record가 소유한다. accepted history와 같은 authority에 섞지 않는다.

개념 구조:

```text
CurrentChangeDeclaration
- BaseSnapshotId
- CandidateContractSignature
- Impact
- Resolution
- MigrationEvidenceId
```

`BaseSnapshotId`는 반드시 latest accepted snapshot을 가리키며 stale base이면 FAIL한다. `CandidateContractSignature`는 current Source/Adapter/Mapping/Semantic signature 묶음에서 계산되어 caller가 임의 문자열로 승인할 수 없다. migration이 완료되고 acceptance 조건을 만족한 뒤에만 이 declaration의 결과가 새 accepted snapshot record로 복사된다.

```text
Impact
- NoMigration
- StagingMigrationRequired
- ProductMigrationReviewRequired
- StagingAndProductMigrationReviewRequired

Resolution
- NotRequired
- Pending
- Resolved
```

bootstrap에서는 current contract가 bootstrap accepted baseline과 동일하므로 별도 pending change declaration을 요구하지 않는다.

규칙:

```text
contract 변화 + CurrentChangeDeclaration 없음
→ Guard FAIL

CurrentChangeDeclaration.BaseSnapshotId != latest accepted SnapshotId
→ Guard FAIL

CurrentChangeDeclaration.CandidateContractSignature != actual current contract signature
→ Guard FAIL

contract 변화 + Impact 미선언
→ Guard FAIL

SchemaRevision 또는 AdapterContractRevision 전진
→ 최소 StagingMigrationRequired 이상을 요구

Staging migration required + canonical exact3 old revision/invalid
→ StagingMigrationPending / Promotion HOLD

Product migration review required + Resolution != Resolved
→ ProductMigrationReviewPending / Promotion HOLD

Resolved인데 MigrationEvidenceId 비어 있음
→ Guard FAIL

MigrationEvidenceId 존재 여부는 machine gate이며, 그 ID가 가리키는 실제 Product/USER/durable evidence의 충분성은 해당 Feature acceptance review가 판정한다. Guard가 문서 의미까지 자동 승인한다고 주장하지 않는다.
```

Guard는 migration 상태를 읽고 판정할 뿐 `SyncProduct`, `ApplyReviewed`, `SavePackage`를 호출하지 않는다.

---

## 6. Revision 규칙

### 6.1 SchemaRevision

JSON physical/typed shape가 바뀌는 변화다.

예:

```text
field 추가/삭제/rename
JSON type 변경
nested object shape 변경
required/optional 구조 변경
representation kind 변경
```

이 경우 새 schema contract snapshot에는 이전 accepted snapshot보다 큰 `SchemaRevision`이 필요하다.

old canonical JSON은 자동 재해석하지 않는다.

### 6.2 AdapterContractRevision

JSON shape는 같아도 typed 의미/validation/canonicalization이 바뀌는 변화다.

예:

```text
FText persistence canonicalization 의미 변경
identity 정책 변경
semantic fingerprint 의미 변경
validation range 의미 변경
resolver precedence 변경
same JSON token의 interpreted meaning 변경
```

이 경우 `AdapterContractRevision` 전진이 필요하다.

### 6.3 둘 다 바뀌는 경우

shape와 semantic meaning이 동시에 바뀌면 둘 다 전진할 수 있다.

P0는 "한 변경에는 revision 하나만" 같은 인위적인 제한을 두지 않는다.

---

## 7. Drift Guard 요구사항

CF-FQ-050은 기존 `ECFDAStagingIssueCode`를 확장하지 않는다.

Private-only developer contract enum/result를 별도로 둔다.

```text
ECFDAContractIssueCode
- SourceAuthoringContractDrift
- AdapterShapeDrift
- SourceAdapterMappingDrift
- SerializerCoverageMismatch
- ParserCoverageMismatch
- FingerprintCoverageMismatch
- MaterializerCoverageMismatch
- SemanticBehaviorMismatch
- SchemaRevisionBumpRequired
- AdapterRevisionBumpRequired
- MigrationImpactUndeclared
- StagingMigrationPending
- ProductMigrationReviewPending
- AcceptedSnapshotChainInvalid
```

정확한 C++ enum spelling은 P0-01 구현 시 이 의미를 유지하는 범위에서 확정한다.

Automation namespace는 기존 CF-FQ-049와 분리한다.

```text
CarFight.DataManagement.CF_FQ_050.DACE_P0_01.*
CarFight.DataManagement.CF_FQ_050.DACE_P0_02.*
...
```

기존 CF-FQ-049 exact 12/13/OperationalEntry identity는 rename/reparent하지 않는다.

사용자 JSON 오타/Preview 진단과 개발자 Source contract drift는 서로 다른 lifecycle이므로 같은 enum/result surface를 공유하지 않는다.

---

## 8. Canonical Staging 검사

현재 Product canonical Staging:

```text
Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json
Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Normal.json
Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_High.json
```

새 accepted schema/adapter revision이 생기면 다음을 확인한다.

```text
canonical JSON exact3가 current revision인가?
current strict parser가 valid하게 읽는가?
old revision이 silent reinterpret되지 않는가?
필요 migration이 아직 미실행이면 명확히 MigrationRequired 상태인가?
```

canonical Product JSON을 test가 임의로 고쳐서 PASS시키지 않는다.

---

## 9. Product Migration 경계

C++ DataAsset 구조 변경이 항상 persisted Product save를 요구하는 것은 아니다.

예를 들어 C++ default만으로 안전한 backward-compatible field가 추가될 수도 있고, 반대로 실제 Low/Normal/High마다 개별 값 authoring이 필요한 변화도 있을 수 있다.

따라서 P0는 다음을 자동 판단한다고 주장하지 않는다.

```text
"새 field니까 Product 3종 값을 AI가 알아서 정함"
```

대신 contract change마다 §5.7의 migration impact와 resolution을 명시한다.

`ProductMigrationReviewRequired` 또는 `StagingAndProductMigrationReviewRequired`이면 해당 contract change를 소유한 Feature lifecycle에서:

```text
Product value 설계
→ Staging migration
→ Preview
→ Review
→ USER/현재 write policy에 맞는 explicit Apply
→ durable readback/evidence
→ MigrationResolution = Resolved + MigrationEvidenceId
```

를 별도 수행한다.

`Resolution != Resolved`인 동안 새 accepted snapshot append와 Current System promotion은 금지한다.

CF-FQ-050 자체의 bootstrap guard 구현은 current SchemaRevision 1 / AdapterContractRevision 2를 그대로 baseline으로 삼으므로 Product Low/Normal/High 값을 바꾸거나 Save할 필요가 없다.

---

## 10. P0 단계

### DACE-P0-00 — Initial Design Review

목적:

- current Source / CF-FQ-049 Current System / existing tests를 교차감사한다.
- Source reflection에서 어떤 property를 Staging-owned로 볼지 authority를 확정한다.
- structural signature와 semantic declaration의 경계를 확정한다.
- 기존 `CFDAStaging`을 과도하게 generic refactor하지 않는 최소 구현 경계를 확정한다.

Acceptance:

```text
P0 blocker 0
blocking P1 0
구현 surface와 ownership 확정
Product/Asset mutation 0
```

#### DACE-P0-00 Initial Design Review 결과 — 2026-09-09

current `MissileGuidePreset` Source / `CFDAStaging` / canonical Product Staging / 기존 Automation source를 read-only로 교차감사했다.

감사 baseline:

```text
UCFMissileGuidePresetData top-level authored property: exact 4
FCFMissileGuideConfig authored UPROPERTY: exact 26
SchemaRevision: 1
AdapterContractRevision: 2
canonical Product Staging: Low / Normal / High exact 3
current Product Staging JSON: 모두 SchemaRevision 1 / AdapterContractRevision 2
Product ApplyReviewed: 0
Product UE Asset Save: 0
Product Staging JSON write: 0
Build/Automation execution: 0
CF-FQ-039 lifecycle: Active 유지
```

current Adapter의 실제 반복 surface는 단순 `RequiredFields` 하나가 아니다.

```text
CFDAStaging.cpp
- exact RequiredFields
- typed parse
- typed value/range validation
- semantic fingerprint token projection
- persisted Asset → typed payload extractor

CFDAStagingApply.cpp
- top-level typed materialization
- MissileGuideConfig whole-struct assignment

CFDAStagingOps.cpp
- Product → canonical Staging JSON serializer
- MissileGuideConfig exact 26-field JSON projection

Automation source
- valid/reordered JSON fixture와 Pilot serializer가 동일 field set을 별도로 반복
```

검수 판정:

```text
P0: 0
P1: 6
P2: 2
Implementation: HOLD
Exact next: DACE-P0-00 Correction + Re-review
```

blocking P1:

1. **P1-1 — canonical serializer가 Adapter evolution surface에서 빠져 있다.**
   - v0.1.0의 연쇄 변경 설명은 parser / extractor / fingerprint / materializer를 중심으로 적혀 있지만, 실제 `CFDAStagingOps.cpp::BuildGuideConfigObject`와 `BuildProductStagingJson`도 current Product를 canonical Staging으로 내보내는 Adapter authority다.
   - 이 surface를 Guard가 보지 않으면 Source/parser가 새 field를 받아도 `SyncProduct`가 stale JSON을 생성하는 거짓 PASS가 가능하다.

2. **P1-2 — `SourceShapeSignature == AdapterShapeSignature` 관계가 그대로는 성립하지 않는다.**
   - native Source type과 JSON representation은 구조가 동일하지 않다. 특히 `FText`는 Source에서는 한 property지만 JSON에서는 `{Kind, Text}` object이며, Staging root에는 native Source에 없는 schema/identity envelope도 존재한다.
   - Source signature와 Adapter signature는 독립적으로 계산하고, 둘 사이를 explicit source-to-adapter mapping descriptor가 연결해야 한다. hash/string 직접 동등 비교를 합격 조건으로 두면 안 된다.

3. **P1-3 — 새 descriptor와 실제 실행 코드 사이 duplicate authority 차단 방식이 아직 없다.**
   - descriptor가 `RequiredFields`만 대체해도 manual parse / range validation / fingerprint / Ops serializer가 빠질 수 있다.
   - P0는 generic reflection writer로 재작성하지 않되, descriptor가 exact field set authority가 되고 실제 parser/fingerprint/serializer coverage가 descriptor와 어긋나면 focused Automation이 실패하는 방법을 명시해야 한다.

4. **P1-4 — semantic meaning 미선언을 전부 자동 차단한다는 문구는 현재 기술적으로 과도하다.**
   - Reflection은 arbitrary semantic code change를 감지할 수 없고, 개발자가 semantic descriptor 자체를 갱신하지 않은 경우 Guard가 의미 변경 사실을 알아낼 일반적인 방법도 없다.
   - P0 보장은 `explicit semantic declaration/signature가 변경됐는데 AdapterContractRevision이 그대로인 경우 fail-closed`까지로 한정하고, arbitrary semantic code change는 focused semantic regression + review 책임으로 분리해야 한다.

5. **P1-5 — accepted revision snapshot의 권위 원본과 append 규칙이 미확정이다.**
   - revision bump 누락을 검출하려면 current descriptor와 독립된 accepted baseline이 필요하다. descriptor와 expected hash를 같은 자리에서 같이 덮어쓰면 drift를 스스로 승인하는 거짓 PASS가 가능하다.
   - P0-01 전에 production Editor module의 immutable accepted snapshot/history를 authoritative baseline으로 두고 test fixture는 그것을 소비하는 구조인지 명확히 해야 한다.

6. **P1-6 — migration impact가 어느 Gate를 막는지 operational contract가 부족하다.**
   - `MigrationImpactUndeclared`와 `ProductMigrationReviewRequired`를 선언하는 것만으로는 실제 promotion 차단 지점이 정해지지 않는다.
   - undeclared impact는 Guard FAIL, Product migration review가 필요한 accepted change는 Product migration lifecycle 완료 전 DACE promotion 금지로 고정하고, Guard 자체는 `ApplyReviewed`/Save를 호출하지 않는 경계를 명시해야 한다.

P2:

1. **P2-1 — materializer 변경을 모든 nested field evolution의 필수 단계처럼 적은 부분은 current Source와 다르다.**
   - 현재 `CFDAStagingApply.cpp`는 `Asset.MissileGuideConfig = Payload.MissileGuideConfig` whole-struct assignment이므로 nested field 추가가 곧 per-field materializer 변경을 뜻하지 않는다.
   - Guard는 실제 coverage를 판정해야 하며, 존재하지 않는 수동 materializer 작업을 억지로 요구하지 않아야 한다.

2. **P2-2 — developer drift issue/result와 Automation isolation의 concrete boundary가 미확정이다.**
   - `ECFDAStagingIssueCode`에 섞지 않는 방향은 맞지만, 별도 developer-only result/issue contract와 새 CF-FQ-050 test namespace/file을 확정해야 기존 CF-FQ-049 exact regression identity를 불필요하게 재작성하지 않는다.

교정 전 금지:

```text
DACE Source implementation 착수 금지
CFDAStaging/Apply/Ops 코드 수정 금지
canonical Product Staging migration 금지
Product Low/Normal/High ApplyReviewed 금지
Product Low/Normal/High UE Asset Save 금지
기존 CF-FQ-049 Automation 재작성 금지
```

#### DACE-P0-00 Correction + Re-review — PASS

P1 6건/P2 2건을 §4~§9 current normative contract에 반영하고 current Source와 다시 대조했다.

교정 대응:

```text
P1-1 canonical serializer 누락
→ production CFDAStagingOps serializer actual-shape probe를 mandatory execution coverage로 고정

P1-2 SourceShape == AdapterShape 오류
→ Source / Adapter / Source↔Adapter Mapping signature 3-way model로 교정

P1-3 descriptor와 execution duplicate authority
→ serializer/parser/fingerprint/extractor↔materializer actual behavior probe가 descriptor와 exact 일치해야 PASS

P1-4 semantic 자동 추론 과장
→ declared semantic contract + behavior-checkable policy만 자동 보장, arbitrary code meaning은 affected regression/review로 제한

P1-5 accepted snapshot authority 미확정
→ production Editor Private `CFDAContractBase.cpp` append-only baseline authority로 확정

P1-6 migration promotion gate 미확정
→ accepted history와 분리된 CurrentChangeDeclaration + BaseSnapshotId/CandidateContractSignature + Impact/Resolution/EvidenceId를 확정하고 Pending이면 snapshot append/Current promotion HOLD

P2-1 materializer per-field 오해
→ current whole-struct assignment를 behavior roundtrip으로 검증, nested per-field materializer list 요구 제거

P2-2 issue/test isolation
→ private `ECFDAContractIssueCode` 계열 + `CF_FQ_050.DACE_*` Automation namespace로 분리
```

재검수 판정:

```text
P0: 0
blocking P1: 0
P2: 0
DACE-P0-00: PASS
Implementation: Not Started
Product/Asset mutation: 0
Product Staging JSON mutation: 0
Build/Automation execution: 0
CF-FQ-039: Active 유지
Exact next: DACE-P0-01 Contract Descriptor / Snapshot Foundation
```

### DACE-P0-01 — Contract Descriptor / Snapshot Foundation

목적:

- MissileGuidePreset Pilot의 explicit authoring contract descriptor를 만든다.
- deterministic Source/Adapter signatures를 만든다.
- accepted revision snapshot/history를 machine-readable하게 만든다.

Acceptance:

```text
현재 SchemaRevision 1 / AdapterContractRevision 2를 bootstrap accepted baseline으로 정확히 표현
SourceShape / AdapterShape / SourceAdapterMapping / SemanticContract signature deterministic
accepted snapshot chain exact 1 bootstrap record PASS
production serializer/parser/fingerprint/extractor↔materializer probe 진입 확보
기존 canonical JSON semantic 결과 변화 0
기존 Apply/Save behavior 변화 0
Product Low/Normal/High mutation/save 0
```

#### DACE-P0-01 구현 결과 — 2026-09-09

`DACE-P0-00 Correction + Re-review PASS`에서 확정한 descriptor/snapshot/probe 경계를 실제 `CarFight_ReEditor` Private Source에 구현했다.

구현 surface:

```text
신규
Private/DataAuthoring/CFDAContractGuard.h
Private/DataAuthoring/CFDAContractGuard.cpp
Private/DataAuthoring/CFDAContractBase.cpp
Private/DataAuthoring/CFDAContractGuardTests.cpp
Tools/RunDAContractTests.ps1

narrow test-only probe 연결
Private/DataAuthoring/CFDAStaging.cpp v1.7.0
Private/DataAuthoring/CFDAStagingOps.cpp v1.2.0
Private/DataAuthoring/CFDAStagingApply.cpp v1.3.0
```

현재 descriptor baseline:

```text
SourceShape: exact 30 node
- UCFMissileGuidePresetData top-level exact4
- FCFMissileGuideConfig nested authored exact26

AdapterShape: exact 42 physical JSON node
SourceAdapterMapping: exact 38 mapping row
SemanticContract: exact 26 explicit semantic rule
```

canonical signature ordering은 UE `FString`의 implicit ordering에 맡기지 않고 **case-sensitive ordinal 비교를 명시**한다. 첫 focused run에서 implicit case-insensitive ordering이 bootstrap hash와 어긋나는 것을 검출했고 `CFDAContractGuard.cpp v1.0.1`에서 canonical row sort를 교정했다.

bootstrap accepted snapshot:

```text
SnapshotId: DACE-MissileGuidePreset-S1-A2-Bootstrap
PreviousSnapshotSignature: empty
SchemaRevision: 1
AdapterContractRevision: 2
SourceShapeSignature: sha256:9a01449e0dfe2cf512527eccffa715bf6f62bcd3993e579f00d4cd87c9b0f8ac
AdapterShapeSignature: sha256:87d3997689a44bdfe2ce7c0442d52aa2b14181b4f83f80a5550861ba72869ab9
SourceAdapterMappingSignature: sha256:b9bdaa43e693004b0cadae700c548b4c5552dd7e5b6cbf8e4c0728516ce70f87
SemanticContractSignature: sha256:8ea27c8e053d89ad6781cd8815a53f5bb6b9a2231b743ce88b2118f9cf25153a
SnapshotSignature: sha256:ec198d5eb5f8199b1ec5a99bbbde80ec5ae30916fee6fea89e72875c710aee69
MigrationImpact: NoMigration
MigrationResolution: NotRequired
```

accepted history는 test fixture가 아니라 production Editor Private `CFDAContractBase.cpp`가 exact 1 bootstrap record를 소유한다. current descriptor와 accepted baseline을 같은 파일/struct literal로 함께 수정하는 구조는 사용하지 않는다.

production private probe 진입:

```text
serializer
→ CFDAStagingOpsPrivate::BuildProductStagingJson 직접 호출

parser
→ FCFDAStagingService::ParseMissilePresetJson 직접 호출

fingerprint
→ FCFDAStagingService::BuildSemanticFingerprint 직접 호출
→ WITH_DEV_AUTOMATION_TESTS temporary sink로 actual token label sequence 관측

extractor
→ FCFDAStagingService::ExtractMissilePresetPayload 직접 호출

materializer ↔ extractor
→ test-owned transient UCFMissileGuidePresetData
→ actual ApplyPayloadToAsset
→ actual extractor readback
→ semantic fingerprint equality
```

probe는 `WITH_DEV_AUTOMATION_TESTS` Private 경계이며 Public/Blueprint API를 추가하지 않았다. Product target package, `SyncProduct`, `ApplyReviewed`, `SavePackage` authority를 소유하지 않는다.

Validation:

```text
Official UE 5.8 Editor Build
- Job: 18f03bf1806e430598e4fa4de7dcc019
- Result: PASS / Exit Code 0

DACE-P0-01 focused Automation
- Process: c635c6f9b27f4722945b37bde2b548fc
- Exact: 3/3 PASS
- Failure: 0
- Missing: 0
- Unexpected: 0
- Duplicate terminal: 0
- BootstrapSnapshot PASS
- ContractDescriptor PASS
- ProductionProbe PASS
```

보호 결과:

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
Product exact3 JSON diff: 0
Product exact3 .uasset diff: 0
CF-FQ-039: Active 유지
기존 병렬 dirty: clean/revert/stash/overwrite 0
```

P0-01은 descriptor/snapshot/probe **foundation**까지 닫는다. Source reflection drift와 descriptor↔execution exact negative regression은 다음 `DACE-P0-02 Structural Drift Detection`에서 구현하며 P0-01 PASS를 full evolution guard 완료로 확대하지 않는다.

### DACE-P0-02 — Structural Drift Detection

목적:

- native DataAsset/nested struct authored field drift를 자동 검출한다.
- Source descriptor / Adapter descriptor / mapping / actual execution behavior 중 하나라도 어긋나면 focused Automation FAIL.

필수 negative regression:

```text
가상/fixture source field 추가
field 제거
field rename 또는 path mismatch
property type mismatch
nested struct drift
mapping source-side 누락
mapping adapter-side 누락
serializer field 누락/추가/type mismatch
parser required field 누락 허용 또는 unknown field 허용
fingerprint token field 누락
materializer/extractor sentinel roundtrip mismatch
```

실제 Product class에 test용 field를 남기지 않는다.

#### DACE-P0-02 구현 결과 — 2026-09-09

`DACE-P0-01 PASS`의 exact descriptor/snapshot/private probe foundation 위에 structural drift fail-closed 검증을 구현했다.

구현 surface:

```text
CFDAContractGuard.h/.cpp v1.1.0
- developer-only FCFDAContractGuardResult / FCFDAContractGuardIssue
- native Reflection Source observer
- Source / Adapter exact shape validator
- Source↔Adapter 양방향 mapping coverage validator
- production serializer JSON physical-shape validator
- production fingerprint exact token descriptor/coverage validator
- materializer→extractor full semantic roundtrip validator

CFDAContractDriftTests.cpp v1.0.1
- DACE-P0-02 focused negative regression exact 5 test namespace

Tools/RunDAContractTests.ps1 v1.1.0
- P0-01 exact3 + P0-02 exact5 = exact8 same-process runner
```

native Source Reflection은 실제 `UCFMissileGuidePresetData::StaticClass()`와 reflected `MissileGuideConfig` struct의 direct authored `CPF_Edit` property를 읽는다. Product class에 test-only property를 추가하지 않고 actual Reflection result 복사본을 변형하는 방식으로 다음을 fail-closed 증명했다.

```text
current actual Source: top-level 4 + nested config 26 = exact30 PASS
virtual field addition: FAIL
field removal: FAIL
rename/path mismatch: FAIL
property type mismatch: FAIL
nested struct identity drift: FAIL
```

Adapter / mapping coverage:

```text
current Adapter physical descriptor: exact42 PASS
current Source↔Adapter mapping: exact38 PASS
Adapter field kind drift: FAIL
mapping source-side omission: FAIL
mapping adapter-side omission: FAIL
duplicate AdapterJsonPath mapping: FAIL
```

actual execution coverage:

```text
production BuildProductStagingJson output vs Adapter exact42: PASS
serializer field omission / wrong type / unknown field fixture: FAIL

production strict parser:
- Adapter required field exact42 각각 removal → Invalid + MissingRequiredField
- Adapter field exact42 각각 incompatible JSON type → Invalid + TypeMismatch
- strict object boundary exact5(root / Payload / DisplayName / Description / Config) unknown sibling → Invalid + UnknownField

production semantic fingerprint:
- metadata 4 + payload top-level 3 + config 26 = exact33 token/order PASS
- token omission / addition / reorder fixture → FAIL

production materializer↔extractor:
- direct transient extractor semantic readback PASS
- Source semantic leaf top-level 3 + config 26 = exact29 sentinel mutation 각각 materialize→extract→semantic fingerprint roundtrip PASS
- synthetic corrupted readback → MaterializerCoverageMismatch FAIL
```

첫 Automation 실행은 duplicate-mapping negative fixture가 `TArray` 내부 element reference를 같은 배열 `Add`에 전달해 UE self-alias assertion으로 종료됐다. Guard/Product 결함이 아니라 test fixture 결함이며 `CFDAContractDriftTests.cpp v1.0.1`에서 duplicate row를 독립 값 복사한 뒤 fresh Build/Automation으로 재검증했다.

최종 검증:

```text
Official UE 5.8 Editor Build: 6d20e32fa3aa4c7e886f0db46acbfd24 / Exit 0 / PASS
DACE focused Automation: 5e1df0863a994702b09d082c0127364f / exact 8/8 PASS
- P0-01 baseline: 3/3 PASS
- P0-02 structural drift: 5/5 PASS
Failure / Missing / Unexpected / DuplicateTerminal: 0 / 0 / 0 / 0
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
Product exact3 JSON diff: 0
Product exact3 .uasset diff: 0
CF-FQ-039: Active 유지
기존 병렬 dirty: clean/revert/stash/overwrite 0
```

위 Build/exact8 결과는 **교정 전 구현 validation evidence**로 보존한다. 아래 Post-Implementation Mid-review에서 structural/execution coverage의 blocking gap이 확인됐으므로 `DACE-P0-02` 최종 PASS 및 `DACE-P0-03` 진입 근거로는 아직 사용하지 않는다.

#### DACE-P0-02 Post-Implementation Mid-review — 2026-09-09

current `CFDAContractGuard`, actual production probes, `CFDAContractDriftTests`와 §4~§5 normative contract를 교차감사했다.

검수 판정:

```text
P0: 0
P1: 3
P2: 2
DACE-P0-02 final acceptance: HOLD
Exact next: DACE-P0-02 Correction + Re-review
Product Low/Normal/High ApplyReviewed / Save: 0 / 0
canonical Product Staging mutation: 0
CF-FQ-039: Active 유지
```

blocking P1:

1. **P1-1 — Materializer↔Extractor sentinel matrix가 Source descriptor leaf set에 결속되지 않았다.**
   - `BuildPayloadMutations()`는 Source semantic leaf exact29를 수동으로 별도 나열하고, 테스트는 `Mutations.Num() == 29`만 확인한다.
   - 향후 Source/Adapter/mapping/parser/fingerprint가 새 authored leaf를 정상 반영해도 이 수동 mutation 목록을 갱신하지 않으면, 새 leaf에 대한 materializer↔extractor sentinel probe가 없는 상태가 자동으로 검출되지 않는다.
   - §5.4 D의 `각 source authored leaf` 요구를 만족하려면 Source descriptor에서 파생한 semantic leaf path set과 mutation `SourcePropertyPath` set을 exact 양방향 비교하고 duplicate도 차단해야 한다. count literal 자체를 coverage authority로 사용하면 안 된다.

2. **P1-2 — Serializer probe가 physical shape는 exact 검증하지만 Source→JSON value wiring을 모든 leaf에 대해 증명하지 않는다.**
   - current P0-02 serializer 검증은 actual JSON의 path/type/unknown field를 Adapter descriptor와 비교하고, P0-01은 단일 baseline payload의 serializer→parser fingerprint equality를 본다.
   - 하지만 baseline에는 서로 같은 값이 실제로 존재한다. 예를 들어 `SeekerFieldOfViewDeg`와 `TrackingConeHalfAngleDeg`는 둘 다 `60`, `MinimumGuidanceSpeedCmPerSec`와 `GuidanceActivationDistanceCm`는 둘 다 `500`, `GuidanceActivationDelaySeconds`와 `TargetVelocityEstimateResponseTimeSeconds`는 둘 다 `0.25`다. 같은 타입·같은 값 field끼리 serializer source wiring이 뒤바뀌면 current physical-shape + 단일-baseline fingerprint 검증이 거짓 PASS할 수 있다.
   - mapping descriptor가 선언한 Source leaf마다 distinguishable sentinel을 하나씩 넣어 production serializer→production parser readback이 정확히 그 leaf만 보존하는지 확인하는 descriptor-bound matrix가 필요하다.

3. **P1-3 — Reflection structural token이 Array/Set/Map 내부 type identity를 보존하지 않는다.**
   - `GetPropertyKind()`/`GetContainerKind()`는 Array/Set/Map outer container는 구분하지만 `GetReflectedTypePath()`는 Enum/ByteEnum/Struct만 처리한다.
   - 따라서 향후 accepted Source contract에 `TArray`, `TSet`, `TMap`이 들어온 뒤 outer container는 같은 채 element/key/value type만 바뀌는 변화는 canonical Source row가 동일하게 남을 수 있다.
   - §4.2의 `property type 변경`, `array/map/set/container shape 변경` 자동 검출 보장을 위해 container element/key/value의 recursive stable type token을 Source structural descriptor/signature에 포함해야 한다. 현재 Pilot에 container field가 없다는 사실은 미래 evolution guard의 이 검출 공백을 해소하지 않는다.

P2:

1. **P2-1 — Parser field-by-field negative matrix가 issue code는 확인하지만 exact failing `FieldPath`는 확인하지 않는다.**
   - `FCFDAStagingIssue`는 machine code와 함께 `FieldPath`를 제공한다.
   - 현재 각 descriptor field removal/type mismatch는 `MissingRequiredField` 또는 `TypeMismatch`가 하나라도 존재하는지만 보므로, 잘못된 field 진단이 나와도 invalid/code 조건만 맞으면 PASS할 수 있다.
   - 각 fixture에서 expected descriptor path와 동일한 issue path가 존재하는지 확인하면 exact per-field coverage와 diagnostic contract가 함께 고정된다.

2. **P2-2 — Fingerprint token 관측 sink가 process-global raw pointer state다.**
   - `WITH_DEV_AUTOMATION_TESTS` 한정이고 현재 exact-list same-process 직렬 실행에서는 문제 없이 PASS했다.
   - 다만 nested/concurrent probe 또는 비정상 조기 종료에 취약한 전역 mutable state이므로 scoped RAII 또는 최소 thread-local/scoped restoration 형태가 더 안전하다.
   - 현재 Product/runtime surface에는 노출되지 않으므로 blocking으로 보지는 않는다.

교정 원칙:

```text
- Product class/test-only property 추가 금지
- Product Low/Normal/High ApplyReviewed / Save 0 유지
- canonical Product Staging write 0 유지
- Source descriptor leaf set을 materializer sentinel coverage authority로 사용
- mapping Source leaf set을 serializer per-leaf sentinel coverage authority로 사용
- container reflected type은 recursive stable token으로 표현
- parser negative fixture는 exact FieldPath까지 검증
- fingerprint probe sink는 dev-only scoped isolation으로 보강
- 교정 뒤 fresh official Build + DACE focused Automation 재실행
- 기존 CF-FQ-049 exact regression은 direct affected change가 없으면 P0-05까지 불필요 반복 금지
```

`DACE-P0-03 Revision Guard`는 위 P1 3건/P2 2건 교정 후 재검수 `P0 0 / blocking P1 0`을 통과하기 전 시작하지 않는다.

#### DACE-P0-02 Correction + Re-review — PASS

v0.3.1 Post-Implementation Mid-review의 P1 3건/P2 2건을 Source/descriptor/actual production probe 경계 안에서 교정하고 fresh Build + focused Automation 뒤 재검수했다.

교정 대응:

```text
P1-1 materializer↔extractor sentinel의 Source descriptor 비결속
→ SourceShape에서 Struct container를 제외한 semantic leaf path set을 직접 산출
→ BuildPayloadMutations SourcePropertyPath set과 exact 양방향 비교
→ duplicate sentinel path도 unique-count mismatch로 FAIL
→ exact29 숫자는 current baseline assertion일 뿐 coverage authority가 아님

P1-2 serializer Source→JSON value wiring 공백
→ SourceAdapterMapping의 SourceToAdapter unique Source leaf set을 authority로 사용
→ 동일 exact29 distinguishable payload mutation을 production BuildProductStagingJson에 입력
→ production strict parser로 다시 읽고 expected mutated semantic fingerprint와 exact 비교
→ 같은 값·같은 타입 field wiring cross-swap도 per-leaf sentinel에서 FAIL

P1-3 Array/Set/Map inner type identity 미포함
→ Reflection container type token을 recursive stable form으로 확장
→ Array<element> / Set<element> / Map<key,value>
→ nested container와 Enum/Struct/Object identity까지 재귀 포함
→ current Pilot에는 container field가 없어 bootstrap current signature는 불변
→ 같은 outer Map에서 value inner type만 바꾼 future-contract fixture가 SourceAuthoringContractDrift로 FAIL

P2-1 parser exact FieldPath 미검증
→ required-field removal / incompatible type exact42 각각 code + exact Descriptor.AdapterJsonPath 검증
→ strict object boundary exact5 unknown sibling도 exact unknown path 검증

P2-2 fingerprint probe process-global raw sink
→ WITH_DEV_AUTOMATION_TESTS 전용 thread_local sink + scoped RAII binding/restoration
→ 성공/실패 return과 무관하게 이전 sink 복원
→ same-thread nested probe는 fail-closed
```

코드 버전:

```text
CFDAContractGuard.cpp v1.1.1
CFDAContractDriftTests.cpp v1.1.0
CFDAStaging.cpp v1.7.1
```

fresh validation:

```text
Official UE 5.8 Editor Build
- Job: ae2df8748108459d9d74f8acfe14da79
- Result: PASS / Exit Code 0
- Recompiled: CFDAContractDriftTests.cpp / CFDAStaging.cpp / CFDAContractGuard.cpp

DACE focused Automation
- Process: c2b7f592e8584aeea5325445f050d61d
- Execution: exact_list_same_process
- Requested: 8
- Success: 8
- Failure: 0
- Missing: 0
- Unexpected: 0
- DuplicateTerminal: 0
- P0-01 baseline exact3: PASS
- P0-02 structural/execution exact5: PASS
```

P0-01의 `BootstrapSnapshot / ContractDescriptor / ProductionProbe`가 fresh run에서 모두 PASS했으므로 current exact30/42/38/26 descriptor와 accepted Schema 1 / Adapter 2 bootstrap signature chain은 이번 P0-02 교정으로 변하지 않았다.

보호 결과:

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
Product exact3 JSON scoped diff: 0
새 Product .uasset dirty/status entry: 0
CF-FQ-039: Active 유지
기존 병렬 dirty: clean/revert/stash/overwrite 0
CF-FQ-049 full affected regression: 재실행 0 — direct operational behavior change가 없어 P0-05 owner 유지
```

재검수 판정:

```text
P0: 0
blocking P1: 0
P2: 0
DACE-P0-02: PASS
DACE-P0-03: 진입 가능
Exact next: DACE-P0-03 Revision Guard
```

### DACE-P0-03 — Revision Guard

목적:

- Adapter physical shape 변화인데 SchemaRevision이 그대로인 경우 FAIL.
- declared SemanticContractSignature 변화인데 AdapterContractRevision이 그대로인 경우 FAIL.
- source/mapping/adapter/semantic contract 변화가 있는데 migration impact가 undeclared면 FAIL.
- revision bump가 있으면 최소 `StagingMigrationRequired` 이상을 요구한다.
- accepted snapshot revision은 monotonic이어야 하고 previous snapshot chain이 끊기면 FAIL.

Source shape 또는 mapping만 바뀌고 Adapter physical shape/semantic meaning/revision이 동일한 candidate는 revision bump를 기계적으로 강요하지 않는다. 다만 `NoMigration`은 Guard가 category만으로 safe라고 자동 추론한 결과가 아니라 개발자가 safe native refactor라고 **explicit declaration**한 판단이며, 필요하면 같은 Source/Mapping-only 변화에도 더 보수적인 `ProductMigrationReviewRequired`를 선언할 수 있다.

기존 AdapterRevision 1 → 2 FText case는 historical semantic-change 예시로만 사용하며 기존 CF-FQ-049 evidence 자체를 무의미하게 재실행하지 않는다.

#### DACE-P0-03 Implementation + Post-Implementation Re-review — PASS

P0-02 Correction + Re-review PASS baseline 위에서 revision/migration declaration guard를 production Editor Private 범위에 구현하고, memory-only future-contract fixture와 actual current baseline을 함께 검증했다.

구현 계약:

```text
CurrentChangeDeclaration
- BaseSnapshotId
- CandidateContractSignature
- bImpactDeclared
- Impact
- Resolution
- MigrationEvidenceId

CandidateContractSignature
= Guard가 SourceShape / AdapterShape / SourceAdapterMapping / SemanticContract exact4 signature를 fixed-key order로 결합한 SHA-256

actual current baseline
= accepted latest SchemaRevision 1 / AdapterContractRevision 2와 exact 동일
= pending CurrentChangeDeclaration 없음(nullptr)
```

fail-closed revision/declaration 규칙:

```text
AdapterShapeSignature change + SchemaRevision 미전진
→ SchemaRevisionBumpRequired

SemanticContractSignature change + AdapterContractRevision 미전진
→ AdapterRevisionBumpRequired

Source / Mapping / Adapter / Semantic 또는 revision 변화 + declaration 없음
→ MigrationImpactUndeclared

CurrentChangeDeclaration.BaseSnapshotId != latest accepted SnapshotId
→ AcceptedSnapshotChainInvalid

caller supplied CandidateContractSignature != Guard-computed actual bundle signature
→ MigrationImpactUndeclared

bImpactDeclared=false
→ MigrationImpactUndeclared

Adapter/Semantic 변화 또는 Schema/Adapter revision 전진
→ 최소 StagingMigrationRequired 계열 impact 필요

Source/Mapping-only safe native refactor
+ Adapter/Semantic/revision unchanged
→ revision bump 강제 0
→ explicit NoMigration declaration 필수
```

accepted snapshot chain 검증:

```text
history empty → FAIL
SnapshotId empty / duplicate → FAIL
stored SnapshotSignature != deterministic record signature → FAIL
bootstrap PreviousSnapshotSignature non-empty → FAIL
next PreviousSnapshotSignature != immediately previous SnapshotSignature → FAIL
same history에서 SchemaId / DataAssetTypeClassPath identity change → FAIL
SchemaRevision 또는 AdapterContractRevision decrease → FAIL
adjacent accepted record마다 revision/migration semantic consistency 재검증
```

P0-01 BootstrapSnapshot test도 future accepted append를 막지 않도록 `history exact1` 요구를 폐기했다. Historical first bootstrap의 identity / Schema 1 / Adapter 2 / class / empty previous를 frozen contract로 유지하면서 전체 append-only chain을 검증한다. Production `CFDAContractBase.cpp`에는 이번 단계에서 **새 accepted snapshot을 추가하지 않았으며 current history는 bootstrap exact1 그대로**다.

신규 focused Automation:

```text
CarFight.DataManagement.CF_FQ_050.DACE_P0_03.RevisionBumps
CarFight.DataManagement.CF_FQ_050.DACE_P0_03.MigrationDeclaration
CarFight.DataManagement.CF_FQ_050.DACE_P0_03.AcceptedSnapshotChain
CarFight.DataManagement.CF_FQ_050.DACE_P0_03.CurrentBaseline
```

첫 implementation validation은 official Build `d993ab71227c451f9f8cd9f567efa15b` PASS + DACE exact12 `4f716c193d434affbc7a372d911aacf8` PASS였다. 이후 post-implementation self-review에서 synthetic accepted append fixture가 current exact1 history에서는 통과하지만 future exact2+ history에서 latest record만 떼어 새 chain의 first record로 사용하는 **test-only future-history assumption** 1건을 발견했다. Product/Guard runtime 결함은 아니지만 append-only evolution test가 미래 정상 append를 방해할 수 있어 즉시 교정했다.

교정:

```text
CFDAContractRevTests.cpp v1.0.1
- synthetic valid/broken/duplicate/revision-regression/tampered chain 모두
  production accepted history 전체 copy → candidate append 방식으로 변경
CFDAContractGuard.h
- Bootstrap validator 주석을 frozen first bootstrap + full append-only chain 실제 의미와 정렬
```

최종 fresh validation:

```text
Official UE 5.8 Editor Build
- Job: cab634bae876405ca8342604c3662f67
- Result: PASS / Exit Code 0

DACE focused Automation
- Process: 23d3905af62b43c8824d4a0328c86c0d
- Execution: exact_list_same_process
- Requested: 12
- Success: 12
- Failure / Missing / Unexpected / DuplicateTerminal: 0 / 0 / 0 / 0
- P0-01 baseline: 3/3 PASS
- P0-02 structural/execution: 5/5 PASS
- P0-03 revision guard: 4/4 PASS
```

Post-Implementation Re-review 판정:

```text
P0: 0
blocking P1: 0
P2: 0
DACE-P0-03: Technical PASS
Exact next: DACE-P0-04 Migration Impact Guard
```

보호 결과:

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
canonical Product Low/Normal/High exact3 JSON scoped diff: 0
새 Product .uasset dirty/status entry: 0
production accepted snapshot append: 0 — current bootstrap exact1 유지
CF-FQ-039: Active 유지
기존 병렬 dirty clean/revert/stash/overwrite: 0
CF-FQ-049 full affected regression: 재실행 0 — P0-05 owner 유지
```

`MigrationResolution`, `MigrationEvidenceId`, current canonical Staging exact3 compatibility와 Pending 상태의 accepted snapshot/promotion 차단은 다음 `DACE-P0-04`가 실제 operational gate로 닫는다. P0-03은 revision/declaration/accepted-chain correctness를 먼저 닫았으며 Product write authority를 추가하지 않았다.

#### DACE-P0-03 Mid-review — HOLD

P0-03 Technical PASS 이후 구현을 fail-closed 관점에서 다시 교차감사했다. 기존 official Build PASS와 DACE exact12 PASS는 **교정 전 구현 evidence**로 보존하지만, 아래 P1/P2가 남아 있으므로 P0-03 Final Acceptance와 P0-04 진입 근거로 확대하지 않는다.

```text
P0: 0
P1: 2
P2: 2
DACE-P0-03 Final Acceptance: HOLD
DACE-P0-04: HOLD
Exact next: DACE-P0-03 Correction + Re-review
```

**P1-1 — Source/Mapping-only change를 전부 safe native refactor로 자동 분류**

현재 `ValidateRevisionGuard`는 다음 category 조건만으로 safe native refactor를 자동 판정한다.

```text
(SourceChanged || MappingChanged)
&& !AdapterChanged
&& !SemanticChanged
&& !AnyRevisionChanged
```

그리고 이 조건이면 `Impact != NoMigration`을 실패시킨다. 그러나 Plan의 의도는 **실제로 safe하다고 판단된 native refactor**에 revision bump를 강요하지 않는 것이지, Source/Mapping-only 변화 전체를 Guard가 자동으로 safe하다고 추론하는 것이 아니다.

Source/Mapping-only 변화라도 persisted Product value review가 필요할 수 있으며, JSON physical shape/semantic descriptor가 그대로라는 사실만으로 Product migration 필요성이 없다고 단정할 수 없다. 현재 구현은 이런 경우 개발자가 더 보수적인 `ProductMigrationReviewRequired`를 선언하려 해도 오히려 실패시킬 수 있다.

교정 기준:

```text
Source/Mapping-only change 자체 ≠ 자동 safe 판정
explicit NoMigration = 개발자가 safe native refactor라고 명시한 판단
ProductMigrationReviewRequired = Source/Mapping-only change에서도 허용 가능한 보수적 선언
Guard는 NoMigration 선언의 structural prerequisite만 검증하고 Product migration 불필요를 자동 추론하지 않음
현재 non-NoMigration을 무조건 실패시키는 safe-refactor fixture도 함께 교정
```

**P1-2 — accepted snapshot mandatory component signature integrity가 fail-closed가 아님**

`ValidateAcceptedSnapshotChain`은 record의 `SnapshotSignature`, previous chain, identity와 revision monotonicity는 검증하지만 새 accepted record의 다음 four component signature가 non-empty canonical SHA-256인지 직접 검증하지 않는다.

```text
SourceShapeSignature
AdapterShapeSignature
SourceAdapterMappingSignature
SemanticContractSignature
```

따라서 실수로 component signature가 empty/malformed여도 그 상태로 `SnapshotSignature`를 다시 계산하면 self-consistent record가 될 수 있고, migration 조합에 따라 chain 검증을 통과할 가능성이 있다. 이는 다음 contract 비교의 accepted baseline authority를 약화시킨다.

교정 기준:

```text
accepted four component signature exact4 모두 필수
canonical `sha256:` + 64 hex 형태 검증
empty / malformed component signature negative fixture 추가
```

이 finding은 과거 accepted record를 고의로 재작성하고 모든 후속 hash를 다시 계산하는 행위까지 runtime code가 막아야 한다는 뜻이 아니다. §5.6에서 확정한 대로 intentional historical edit는 Git diff/code review 책임이며, 이번 교정은 **새 accepted record의 필수 contract authority가 malformed 상태로 승인되는 accidental path**를 막는 것이다.

**P2-1 — no-delta stale CurrentChangeDeclaration이 무시됨**

`ValidateRevisionGuard`는 contract/revision delta가 없으면 declaration 검증 전에 PASS return한다. 현재 `GetCurrentChangeDeclaration()==nullptr` baseline은 정상이나, 향후 acceptance/rollback 뒤 stale declaration이 남아도 current contract가 latest accepted와 같으면 이를 무시할 수 있다.

교정 기준:

```text
no active delta → CurrentChangeDeclaration은 absent/null이어야 함
stale non-null declaration negative fixture 추가
```

**P2-2 — revision negative matrix가 구현 계약 전체를 직접 덮지 못함**

현재 code는 SchemaRevision/AdapterContractRevision 감소와 revision-only advance를 다루지만 Automation은 SchemaRevision 감소만 직접 fixture로 검증하고, contract signature 변화가 없는 pure revision-only bump의 declaration/minimum Staging impact 경로도 직접 고정하지 않는다.

교정 기준:

```text
AdapterContractRevision decrease fixture 추가
pure Schema/Adapter revision-only advance + declaration 없음 → FAIL
pure revision-only advance + NoMigration/Product-only impact → FAIL
pure revision-only advance + explicit StagingMigrationRequired 이상 → PASS
```

중간검수 보호 결과:

```text
Source correction: 0
Build/Automation rerun: 0 — review-only
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
production accepted snapshot append: 0
CF-FQ-039 Active: 유지
기존 병렬 dirty clean/revert/stash: 0
```

#### DACE-P0-03 Correction + Re-review — PASS

Mid-review의 `P1 2 / P2 2`를 current Source에 최소 범위로 교정하고 fresh Build/Automation 및 post-correction code review로 재검수했다.

교정 결과:

```text
P1-1 Source/Mapping-only safe 자동 추론
→ `bSafeNativeRefactor` category 판정을 제거
→ explicit NoMigration은 Adapter/Semantic/revision 변화가 없는 Source/Mapping-only candidate에서만 structural admissibility를 검증
→ Source/Mapping-only + ProductMigrationReviewRequired를 허용해 보수적 Product review 경로 보존

P1-2 accepted exact4 component signature integrity
→ SourceShapeSignature / AdapterShapeSignature / SourceAdapterMappingSignature / SemanticContractSignature 모두
  lowercase canonical `sha256:` + 64 hex를 필수화
→ empty component + self-consistent SnapshotSignature fixture FAIL
→ uppercase/noncanonical component + self-consistent SnapshotSignature fixture FAIL

P2-1 no-delta stale CurrentChangeDeclaration
→ current contract/revision이 latest accepted와 exact 동일할 때 non-null declaration이 남으면 FAIL
→ actual baseline은 GetCurrentChangeDeclaration()==nullptr 유지

P2-2 revision negative matrix
→ AdapterContractRevision decrease 직접 fixture 추가
→ pure SchemaRevision advance: declaration 없음 FAIL / NoMigration FAIL / Product-only FAIL / StagingMigrationRequired PASS
→ pure AdapterContractRevision advance + StagingMigrationRequired PASS
```

accepted snapshot test fixture 자체가 새 canonical integrity gate를 가짜로 위반하지 않도록 synthetic valid Source/Mapping signature도 임의 suffix가 아니라 **다른 유효 canonical SHA-256**으로 교체했다. 따라서 empty/malformed/tamper/revision/chain failure가 각각 의도한 원인에 의해 검출된다.

fresh validation:

```text
Official UE 5.8 Editor Build
- Job: 2f1003126fa1435f8087af24a947305d
- Result: PASS / Exit Code 0
- CarFight_ReEditor compile/link PASS

DACE focused Automation
- Process: 3aa77b32ed014a13ac813ab00b3be81e
- Execution: exact_list_same_process
- Requested: 12
- Success: 12
- Failure / Missing / Unexpected / DuplicateTerminal: 0 / 0 / 0 / 0
- P0-01 baseline: 3/3 PASS
- P0-02 structural/execution: 5/5 PASS
- P0-03 revision guard: 4/4 PASS
- result schema: carfight_data_authoring_automation_result_v2 / status success
```

post-correction re-review 판정:

```text
P0: 0
blocking P1: 0
P2: 0
DACE-P0-03: Correction + Re-review PASS
DACE-P0-04: 진입 가능
Exact next: DACE-P0-04 Migration Impact Guard
```

보호 결과:

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
canonical Product Low/Normal/High exact3 JSON scoped diff: 0
새 Product .uasset dirty/status entry: 0
production accepted snapshot append: 0 — CFDAContractBase.cpp bootstrap exact1 유지
CF-FQ-039: Active 유지
기존 병렬 dirty clean/revert/stash/overwrite: 0
commit / push: 0
```

P0-03은 revision/declaration/accepted-chain correctness까지만 닫았다. `MigrationResolution`, `MigrationEvidenceId`, current canonical Staging exact3의 revision/strict-parse compatibility와 Pending 상태의 accepted snapshot append/Current System promotion 차단은 계획대로 `DACE-P0-04`가 소유한다.

### DACE-P0-04 — Migration Impact Guard

목적:

- current canonical Staging exact3의 revision/strict parse 호환성을 read-only 검사한다.
- Staging migration 필요 / Product migration review 필요 / resolution을 명시적으로 구분한다.
- migration required를 자동 apply/save와 분리한다.

Acceptance:

```text
old revision silent migration 0
Product auto-save 0
canonical Staging silent overwrite 0
StagingMigrationPending / ProductMigrationReviewPending machine-readable
Pending 상태에서 accepted snapshot append 0
Pending 상태에서 Current System promotion 0
Resolved + evidence 조건 명확
```

#### DACE-P0-04 Implementation + Post-Implementation Re-review — Technical PASS

P0-03의 revision/declaration/accepted-chain foundation 위에 **write authority를 추가하지 않고** current canonical Product Staging exact3의 read-only compatibility와 migration lifecycle machine gate를 구현했다.

구현:

```text
CFDAContractGuard.h/.cpp v1.3.0
- FCFDAMigrationGateResult
  - Validation
  - bAcceptedSnapshotAppendAllowed
  - bCurrentSystemPromotionAllowed

ValidateStagingJsonCompatibility
- actual production FCFDAStagingService::ParseMissilePresetJson 사용
- parse/revision/schema/class mismatch → StagingMigrationPending

ValidateCurrentCanonicalStagingCompatibility
- exact3만 <main_game>/Authoring/... 에서 FFileHelper::LoadFileToString으로 read-only 로드
- SyncProduct / ApplyReviewed / SavePackage 호출 0

ValidateMigrationResolution
- NoMigration → Resolution=NotRequired + evidence empty 강제
- migration/review impact → NotRequired 금지
- Resolved → non-empty MigrationEvidenceId 강제
- staging required + unresolved/incompatible → StagingMigrationPending
- Product review required + unresolved → ProductMigrationReviewPending

EvaluateMigrationGate / EvaluateCurrentMigrationGate
- Pending/invalid 상태에서는 accepted append=false + Current promotion=false
- no-delta accepted baseline은 duplicate append=false
- no-delta + current exact3 compatible + current revision guard PASS면 Current promotion prerequisite=true
- actual current operational gate는 revision/declaration guard까지 합쳐 fail-closed

ValidateAcceptedSnapshotChain
- accepted record 자체도 Pending/invalid Resolution/Evidence를 포함하면 FAIL
```

canonical path는 Unreal `ProjectDir()`가 `<main_game>/UE/`라는 실제 구조를 반영해 기존 CF-FQ-049 방식과 동일하게 parent `<main_game>/`을 resolve한다. 구현 중 source self-review에서 include 배치와 path root를 Build 전에 교정했으며 Product file mutation은 발생하지 않았다.

신규 focused Automation:

```text
CarFight.DataManagement.CF_FQ_050.DACE_P0_04.CanonicalStaging
CarFight.DataManagement.CF_FQ_050.DACE_P0_04.ResolutionEvidence
CarFight.DataManagement.CF_FQ_050.DACE_P0_04.PromotionAppend
```

negative/positive matrix는 모두 memory-only fixture다.

```text
current canonical Low/Normal/High exact3 strict parse → PASS
old SchemaRevision memory fixture → SchemaRevisionUnsupported + StagingMigrationPending
old AdapterContractRevision memory fixture → AdapterRevisionMismatch + StagingMigrationPending
Staging Pending → append/promotion block
Product Pending → append/promotion block
combined Pending → StagingMigrationPending + ProductMigrationReviewPending
Resolved + evidence empty → FAIL
NoMigration + Pending Resolution → FAIL
NoMigration + non-empty evidence → FAIL
Staging Resolved + evidence + compatible canonical → PASS
combined Staging+Product Resolved + evidence + compatible canonical → PASS
Resolved declaration + incompatible canonical → append/promotion block
Pending accepted synthetic record → accepted chain FAIL
Resolved Product review + evidence synthetic record → accepted chain PASS
actual no-delta current baseline → duplicate append false / Current promotion prerequisite true
```

최종 fresh validation:

```text
Official UE 5.8 Editor Build
- Job: 36dca8bb69ef4ed99271139f2542a79a
- Result: PASS / Exit Code 0
- CFDAContractMigTests.cpp compile + CarFight_ReEditor DLL link PASS

DACE focused Automation
- Process: a4702819c89b402285ddf1f0452fd844
- Execution: exact_list_same_process
- Requested: 15
- Success: 15
- Failure / Missing / Unexpected / DuplicateTerminal: 0 / 0 / 0 / 0
- P0-01: 3/3 PASS
- P0-02: 5/5 PASS
- P0-03: 4/4 PASS
- P0-04: 3/3 PASS
- result schema: carfight_data_authoring_automation_result_v2 / status success
- result SHA-256: aa5af84229ff3b61b9868334b4fa36b41633689015315f0af383a6d9d483efd6
```

Post-Implementation Re-review:

```text
P0: 0
blocking P1: 0
P2: 0
DACE-P0-04: Technical PASS
Exact next: DACE-P0-05 Integration / Regression
```

보호 결과:

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
canonical Product Low/Normal/High exact3 JSON/.uasset scoped diff: 0
production accepted snapshot append: 0 — bootstrap exact1 유지
CF-FQ-039: Active 유지
기존 병렬 dirty clean/revert/stash/overwrite: 0
commit / push: 0
```

`MigrationEvidenceId`는 P0-04에서 **존재 여부만 machine gate**로 검증한다. evidence 내용이 실제 migration/review를 충분히 증명하는지는 P0-05/P0-06의 integration/acceptance review가 소유하며 임의 문자열 존재만으로 실제 Current System Promotion을 수행하지 않는다.

### DACE-P0-05 — Integration / Regression

목적:

- CF-FQ-049 기존 exact parse/Preview/Apply 계약과 회귀 통합한다.
- guard 추가가 Product Staging operational flow를 깨뜨리지 않는지 검증한다.

최소 검증:

```text
Official UE 5.8 Editor Build
DACE focused guard tests
affected CF-FQ-049 Staging tests
OperationalEntry 영향이 있으면 exact operational test
Product Low/Normal/High UE Asset mutation/save 0
```

#### DACE-P0-05 Integration / Regression — Technical PASS

P0-05는 새 Source/runner 구현 없이 P0-04까지의 current binary와 CF-FQ-049 Current operational entry를 그대로 사용해 **DACE Guard + 기존 Staging/Preview/Apply safety**를 fresh 통합 회귀했다.

실행 순서와 결과:

```text
1. Official UE 5.8 Editor Build
- Job: 29df6158b2114e6f806a2cb9ace5100f
- Result: PASS / Exit Code 0
- Engine: D:\UnrealEngine_Source

2. DACE focused guard
- Runner: Tools/RunDAContractTests.ps1
- Process: aff1246873cb402f813267c04cfd0892
- exact 15/15 PASS
- Failure / Missing / Unexpected / DuplicateTerminal: 0 / 0 / 0 / 0

3. affected CF-FQ-049 Staging
- Runner: Tools/RunDAStagingTests.ps1
- Process: e1b4d2629d7744e8b152e7428340ce36
- exact 13/13 PASS
- Failure / Missing / Unexpected / DuplicateTerminal: 0 / 0 / 0 / 0
- DAS-P0-04 test-owned Content/Staging fixture residue: 0

4. CF-FQ-049 OperationalEntry
- Runner: Tools/RunDAStagingOpsTest.ps1
- Process: 531f99e9da92407ba94fb507c91d1d9c
- exact 1/1 PASS
- Failure / Missing / Unexpected / DuplicateTerminal: 0 / 0 / 0 / 0
- DAS_P0_05_PRODUCT_STAGING_RESIDUE_COUNT=0
```

통합 회귀가 직접 보존한 Current 계약:

```text
strict canonical parse / validation PASS
Preview matrix / BatchPlanHash PASS
one-shot approval lifecycle PASS
loaded identity preflight PASS
Product mutation0 Preview PASS
durable test-owned Create/Update + disk reload semantic readback PASS
conflict / uncertainty / PartialApplied safety PASS
selection-bound Preview→Review / stale review reject / forced partial-write rollback PASS
DACE source/adapter/mapping/serializer/parser/fingerprint/materializer/revision/migration gate PASS
```

보호 감사:

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging final mutation: 0
OperationalEntry pre-run↔post-run canonical exact3 raw bytes: exact 동일
Product Low/Normal/High exact3 JSON/.uasset scoped Git diff: 0
production accepted snapshot append: 0
accepted history: DACE-MissileGuidePreset-S1-A2-Bootstrap exact1 유지
CF-FQ-039: single Active 유지
기존 병렬 dirty clean/revert/stash/overwrite: 0
P0-05 Source/C++/runner mutation: 0
commit / push: 0
```

Post-Integration Review:

```text
P0: 0
blocking P1: 0
P2: 0
DACE-P0-05: Technical PASS
Exact next: DACE-P0-06 Acceptance / Current System Promotion
```

P0-05는 CF-FQ-049 durable writer를 재구현하거나 Product를 실제 Apply/Save하는 단계가 아니다. P0-06에서 이 fresh integration evidence를 근거로 Current `DataAssetAuthoring.md`에 Contract Evolution Guard 운영 계약을 승격하고 CF-FQ-050 lifecycle을 최종 판정한다.

### DACE-P0-06 — Acceptance / Current System Promotion

목적:

- current `DataAssetAuthoring.md`에 Contract Evolution Guard 운영 계약을 승격한다.
- CF-FQ-050을 Done으로 전환한다.
- 다른 DA 타입 onboarding의 선행조건으로 사용 가능한지 최종 판정한다.

#### DACE-P0-06 Initial Acceptance Audit — Promotion HOLD

P0-05 fresh Build/Automation evidence, current Source, canonical Product exact3, production accepted snapshot history와 Current 문서 projection을 교차감사했다.

```text
P0: 0
P1: 1
P2: 0
Promotion: HOLD
```

P1-1 — representative Plan §18 stale current projection:

```text
상단 Current: v0.6.0 / DACE-P0-05 Technical PASS / DACE-P0-06 current
§18 stale: v0.5.0 / P0-04 final evidence / DACE-P0-05 Pending
```

실제 구현·Build·Automation·Product/accepted 상태와 충돌하는 코드는 없지만, Current System Promotion 직전 대표 Plan 내부에 서로 다른 세대의 current checkpoint가 공존하면 다음 세션 복원 기준이 모순된다. 따라서 §18을 P0-05 fresh evidence와 DACE-P0-06 current state로 정렬하기 전에는 Promotion을 진행하지 않는다.

보호 감사 결과:

```text
Product Low/Normal/High exact3 JSON/.uasset scoped Git diff: 0
Product Low/Normal/High ApplyReviewed / UE Asset Save: 0 / 0
canonical Product Staging final mutation: 0
accepted snapshot history: bootstrap exact1 / append 0
CF-FQ-039: single Active 유지
기존 병렬 dirty clean/revert/stash: 0
```

Correction은 representative Plan projection만 교정하며 Source/Asset/Build/Automation을 재실행하거나 수정하지 않는다.

#### DACE-P0-06 Correction + Re-review / Final Acceptance — PASS

§18 stale current projection을 P0-05 fresh evidence에 맞춘 뒤 representative Plan 상단, §18, actual Source, accepted history, Product exact6 protection과 Current Systems promotion 내용을 다시 교차검수했다.

```text
P0: 0
blocking P1: 0
P2: 0
Final Acceptance: PASS
```

승격 결과:

```text
G0 Evidence: PASS
- official UE 5.8 Build PASS
- DACE focused exact15/15 PASS
- affected CF-FQ-049 exact13/13 PASS
- OperationalEntry exact1/1 PASS

G1 Current Knowledge Promotion: PASS
- Document/Systems/DataManagement/DataAssetAuthoring.md v1.1.0
- Document/Systems/SystemIndex.md v1.34.0

G2 Current Route Cleanup: PASS
- FeatureQueue Done
- ActiveWork Ready route 제거
- Plan Index Ready route 제거

G3 Reference Preservation: PASS
- DAContractEvolution/DAContractEvolutionPlan.md v0.7.0 retained
- Archive Index에 Historical + Retained Path 등록

G4 Semantic Historical: PASS

G5 Physical Move: Deferred
- plan_repo existing dirty와 retained evidence를 보호하기 위해 closure에서 move/rename하지 않음
```

두 번째 Staging 지원 DA onboarding의 **선행 guard interface/lifecycle로 사용 가능**하다고 판정한다. 단, 새 DA가 자동 지원되는 것은 아니며 Typed Adapter/Schema/descriptor/accepted baseline/production probe provider는 별도 lifecycle에서 구현해야 한다.

최종 보호 판정:

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
canonical Product Staging mutation: 0
Product exact3 JSON/.uasset scoped Git diff: 0
accepted snapshot append: 0
accepted history: bootstrap exact1 유지
CF-FQ-039: single Active 유지
기존 병렬 dirty clean/revert/stash/overwrite: 0
commit / push: 0
```

---

## 11. P0 완료 기준

모두 만족해야 한다.

```text
[x] 현재 MissileGuidePreset contract baseline을 machine-readable하게 표현
[x] Staging-owned C++ structural drift 자동 검출
[x] Adapter shape drift 자동 검출
[x] SchemaRevision bump 누락 자동 차단
[x] AdapterContractRevision bump 누락 자동 차단
[x] migration impact declaration 누락 자동 차단
[x] canonical Staging old revision silent reinterpret 0
[x] Product auto-migration/save 0
[x] 기존 CF-FQ-049 Preview/Review/Apply safety 회귀 없음 — P0-05 exact13 + OperationalEntry 1/1 PASS
[x] Official UE 5.8 Build PASS
[x] focused/affected Automation PASS — DACE exact15 + affected exact13 + OperationalEntry exact1 PASS
[x] Current Systems 승격 — `DataAssetAuthoring.md v1.1.0` / `SystemIndex.md v1.34.0`
```

---

## 12. Non-Goals

CF-FQ-050 P0에서는 하지 않는다.

```text
모든 CarFight DataAsset를 Staging 지원 타입으로 전환
Ammo/Damage/Sensor Adapter 신규 구현
generic Reflection writer
구버전 JSON 자동 변환 엔진
새 field 값 자동 추측
Product DataAsset 무단 Apply/Save
VehicleData를 Staging 경로로 통합
Runtime이 JSON을 직접 읽는 구조
CF-FQ-045 Data Asset Manager write authority 확대
CF-FQ-049 durable Apply engine 재작성
```

---

## 13. 다른 DA 타입 확장과의 관계

CF-FQ-050은 다른 DA Adapter를 직접 늘리는 Feature가 아니다.

순서는 다음을 권장한다.

```text
CF-FQ-050 Contract Evolution Guard 완료
↓
두 번째 대표 DA Adapter onboarding
↓
반복 boilerplate/공수 측정
↓
필요 시 Typed Adapter Scaffold 공통화
↓
가치 높은 DA 타입만 단계적으로 확장
```

이렇게 해야 DA 타입이 늘어난 뒤 contract evolution을 역으로 정리하는 비용을 피할 수 있다.

---

## 14. 보호 범위

이번 정식 승격 및 초기 설계 단계에서:

```text
CF-FQ-049 lifecycle: Done 유지
CF-FQ-039 lifecycle: Active 유지
CF-FQ-048 및 다른 병렬 dirty: 보호
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Save: 0
Product Staging JSON mutation: 0 — 구현/마이그레이션 Gate 전
Build/Automation: 0 — 정식 승격 자체는 문서 작업
```

현재 main_game의 unrelated dirty를 clean/revert/stash하지 않는다.

---

## 15. 예상 Current owner

P0 완료 시 새 Systems 문서를 별도로 늘리기보다 현재 owner를 확장하는 것을 기본안으로 한다.

```text
Document/Systems/DataManagement/DataAssetAuthoring.md
```

CF-FQ-050은 CF-FQ-049 Current System의 유지보수 계약을 보강하기 때문이다.

다만 Initial Design Review에서 ownership이 별도 Systems 문서를 요구할 만큼 독립적인 것으로 판정되면 그때만 변경한다.

---

## 16. 위험 요소

### 16.1 Reflection 과신

Reflection structural scan이 semantic meaning까지 안다고 가정하면 안 된다.

대응:

```text
structural drift = automatic
semantic change = explicit declaration + revision guard
```

### 16.2 모든 UPROPERTY 자동 포함

대응:

```text
explicit Staging-owned authoring contract
```

### 16.3 Guard 자체가 대형 generic framework가 되는 위험

대응:

```text
MissileGuidePreset 1종으로 최소 구현
기존 typed parser/materializer 유지
다른 DA onboarding은 P0 scope out
```

### 16.4 Product migration과 guard 구현의 혼합

대응:

```text
migration impact 판정과 실제 Product write를 분리
CF-FQ-050 guard 구현 중 Product save 0 기본
```

---

## 17. DACE-P0-00 설계검수 확정 답변

1. Staging-owned Source는 `UCFMissileGuidePresetData` exact top-level 4와 `FCFMissileGuideConfig` current authored UPROPERTY exact26이다. 무차별 EditAnywhere scan은 하지 않는다.
2. Source signature token은 full source property path + reflected kind/type/container/nested struct identity를 사용하고 ordinal deterministic sort한다.
3. FText 같은 비-1:1 표현은 Source↔Adapter mapping descriptor의 `RepresentationKind=LiteralFText`처럼 표현하며 `{Kind,Text}` child를 Adapter physical shape로 별도 소유한다.
4. Descriptor는 generic parser generator가 아니다. production serializer/parser/fingerprint/extractor/materializer의 **actual behavior probe**가 descriptor와 exact 일치해야 하므로 목록만 맞춘 duplicate authority PASS를 허용하지 않는다.
5. accepted snapshot은 test fixture가 아니라 production Editor Private `CFDAContractBase.cpp` append-only record가 소유한다.
6. semantic meaning은 explicit semantic policy descriptor + `SemanticContractSignature`로 선언한다. 변경됐는데 AdapterContractRevision이 그대로면 FAIL한다. arbitrary 미선언 code meaning 자동 추론은 비목표다.
7. developer migration/drift는 private `ECFDAContractIssueCode` 계열 result로 노출하고 기존 `ECFDAStagingIssueCode`와 분리한다.
8. contract 변화인데 migration impact 미선언이면 Guard FAIL한다. Product review required인데 Resolution이 Resolved가 아니면 accepted snapshot append와 Current promotion을 차단한다.
9. 재사용 최소 interface는 `Source descriptor + Adapter descriptor + Mapping + Semantic policy + accepted snapshot + behavior probe provider`다. 두 번째 DA onboarding 전 generic writer/framework로 확대하지 않는다.
10. 기존 CF-FQ-049 Automation test names와 OperationalEntry는 유지한다. CF-FQ-050은 별도 `CF_FQ_050.DACE_*` focused namespace를 사용하고 P0-05에서 실제 affected 범위만 재실행한다.

---

## 18. 정식 승격 상태

```text
CF-FQ-050: Done / P0 Complete
Priority: P2
Representative Historical Plan: DAContractEvolution/DAContractEvolutionPlan.md v0.7.0
Current owner: main_game Document/Systems/DataManagement/DataAssetAuthoring.md v1.1.0
DACE-P0-00 Correction + Re-review: PASS / P0 0 / blocking P1 0 / P2 0
DACE-P0-01 Contract Descriptor / Snapshot Foundation: PASS
DACE-P0-02 Correction + Re-review: PASS / P0 0 / blocking P1 0 / P2 0
DACE-P0-03 Correction + Re-review: PASS / P0 0 / blocking P1 0 / P2 0
DACE-P0-04 Migration Impact Guard: Technical PASS / P0 0 / blocking P1 0 / P2 0
DACE-P0-05 Integration / Regression: Technical PASS / P0 0 / blocking P1 0 / P2 0
DACE-P0-06 Final Acceptance: PASS / P0 0 / blocking P1 0 / P2 0
Final Official Build: 29df6158b2114e6f806a2cb9ace5100f / Exit 0 / PASS
Final DACE focused Automation: aff1246873cb402f813267c04cfd0892 / exact 15/15 PASS
Affected CF-FQ-049 Staging: e1b4d2629d7744e8b152e7428340ce36 / exact 13/13 PASS
OperationalEntry: 531f99e9da92407ba94fb507c91d1d9c / exact 1/1 PASS
Accepted snapshot history: bootstrap exact1 유지 / append 0
Product Low/Normal/High ApplyReviewed / Save: 0 / 0
Product Staging JSON mutation: 0
Current Active: CF-FQ-039 unchanged
Current System Promotion: Complete
Historical: G0~G4 PASS / G5 Deferred / Retained Path
Exact next: None — P0 Complete
```

---

## 19. Changelog

### v0.7.0 - 2026-09-09

- `DACE-P0-06` 초기 Acceptance P1 1건인 §18 stale current projection을 교정한 뒤 재검수 `P0 0 / blocking P1 0 / P2 0` Final Acceptance PASS로 닫았다.
- Current Knowledge를 main_game `Document/Systems/DataManagement/DataAssetAuthoring.md v1.1.0`과 `Document/Systems/SystemIndex.md v1.34.0`에 승격해 Contract Evolution Guard의 descriptor/signature, structural/behavior drift, revision/migration/promotion gate와 다른 DA onboarding 경계를 Current 계약으로 고정했다.
- G0~G4를 PASS로 닫아 CF-FQ-050을 Done → Historical + Retained Path로 전환했다. G5 physical move는 plan_repo existing dirty와 retained evidence 보호를 위해 Deferred했다.
- final evidence는 Build `29df6158b2114e6f806a2cb9ace5100f` PASS, DACE exact15/15, affected CF-FQ-049 exact13/13, OperationalEntry exact1/1이며 Product Low/Normal/High Apply·Save 0 / canonical Product Staging mutation 0 / accepted snapshot append 0을 유지한다.
- 두 번째 Staging 지원 DA의 guard 선행조건은 준비됐지만 자동 adapter 지원은 아니며 별도 Typed Adapter/Schema/descriptor/probe lifecycle이 필요하다. 현재 single Active `CF-FQ-039`는 변경하지 않았다.

### v0.6.1 - 2026-09-09

- `DACE-P0-06 Initial Acceptance Audit`에서 구현·회귀 blocking defect는 없었으나 representative Plan §18이 v0.5.0/P0-05 Pending 세대로 남아 상단 v0.6.0/P0-06 current와 충돌하는 P1 1건을 발견해 Promotion HOLD했다.
- §18을 P0-05 fresh official Build `29df6158b2114e6f806a2cb9ace5100f`, DACE exact15 `aff1246873cb402f813267c04cfd0892`, affected CF-FQ-049 exact13 `e1b4d2629d7744e8b152e7428340ce36`, OperationalEntry exact1 `531f99e9da92407ba94fb507c91d1d9c` 기준으로 교정했다.
- Product Low/Normal/High Apply·Save 0 / canonical Product Staging mutation 0 / exact3 JSON·uasset diff 0 / accepted snapshot append 0 / CF-FQ-039 Active와 기존 병렬 dirty를 유지했다. Source/Asset/Build/Automation mutation은 0이다.
- exact next는 `DACE-P0-06 Acceptance Projection Correction + Re-review`다. 재검수 PASS 전에는 Current System Promotion/Done 전환을 수행하지 않는다.

### v0.6.0 - 2026-09-09

- `DACE-P0-05 Integration / Regression`을 fresh official UE 5.8 Build와 current runner 3축 통합 회귀로 수행하고 `P0 0 / blocking P1 0 / P2 0` Technical PASS로 닫았다. Source/C++/runner 구현 mutation은 0이다.
- official Build `29df6158b2114e6f806a2cb9ace5100f` PASS 뒤 DACE focused `aff1246873cb402f813267c04cfd0892` exact15/15, affected CF-FQ-049 Staging `e1b4d2629d7744e8b152e7428340ce36` exact13/13, OperationalEntry `531f99e9da92407ba94fb507c91d1d9c` exact1/1을 모두 failure/missing/unexpected/duplicate terminal 0으로 통과했다.
- CF-FQ-049 durable fixture residue 0과 OperationalEntry canonical Product Staging raw-byte residue 0을 확인했다. Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0이며 final exact3 JSON/.uasset scoped Git diff도 0이다.
- production accepted snapshot append 0으로 bootstrap exact1을 유지하고 현재 단일 Active `CF-FQ-039`와 기존 병렬 dirty를 보존했다. P0-05에서 clean/revert/stash/commit/push는 수행하지 않았다.
- representative Plan을 v0.6.0으로 전진하고 exact next를 `DACE-P0-06 Acceptance / Current System Promotion`으로 변경했다. Current Systems promotion과 CF-FQ-050 Done 판정은 P0-06 owner다.

### v0.5.0 - 2026-09-09

- `DACE-P0-04 Migration Impact Guard`를 구현하고 post-implementation re-review `P0 0 / blocking P1 0 / P2 0` Technical PASS로 닫았다.
- current canonical Product Staging Low/Normal/High exact3을 main_game disk에서 read-only로 읽어 actual production strict parser에 전달하고 SchemaRevision/AdapterContractRevision/schema/class compatibility를 검증한다. old revision은 silent migration 없이 `StagingMigrationPending`으로 투영된다.
- `FCFDAMigrationGateResult`와 Resolution/Evidence validation을 추가해 `StagingMigrationPending`/`ProductMigrationReviewPending`, Resolved+non-empty Evidence 조건, Pending accepted snapshot append/Current System promotion 차단을 machine-readable하게 고정했다. no-delta baseline은 duplicate append를 허용하지 않되 current promotion prerequisite는 PASS할 수 있도록 분리했다.
- accepted snapshot chain도 Pending/invalid Resolution/Evidence record를 fail-closed한다. `MigrationEvidenceId` 내용의 실제 충분성은 machine gate가 추론하지 않고 P0-05/P0-06 review 책임으로 유지한다.
- post-implementation self-review에서 NoMigration invalid Resolution/Evidence와 combined Resolved+evidence matrix를 기존 P0-04 test에 보강했고 최종 official UE 5.8 Build `36dca8bb69ef4ed99271139f2542a79a` PASS + DACE focused `a4702819c89b402285ddf1f0452fd844` exact 15/15 PASS를 확보했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / canonical Product Staging mutation 0 / exact3 JSON/.uasset scoped diff 0 / production accepted snapshot append 0이며 bootstrap exact1, 현재 단일 Active `CF-FQ-039`와 기존 병렬 dirty를 보존했다.
- exact next를 `DACE-P0-05 Integration / Regression`으로 전진했다. affected CF-FQ-049 Preview/Review/Apply/Operational regression은 계획대로 P0-05가 소유하며 이번 단계에서는 재실행하지 않았다.

### v0.4.2 - 2026-09-09

- `DACE-P0-03` 중간검수의 `P1 2 / P2 2`를 전건 교정하고 재검수 `P0 0 / blocking P1 0 / P2 0` PASS로 닫았다.
- Source/Mapping-only change를 category만으로 safe라고 자동 추론하지 않고 `NoMigration`을 explicit developer judgment로 분리했다. Adapter/Semantic/revision 불변은 NoMigration의 structural admissibility만 검증하며 `ProductMigrationReviewRequired` 같은 더 보수적인 declaration을 허용한다.
- accepted snapshot의 Source/Adapter/Mapping/Semantic exact4 component signature를 lowercase canonical `sha256:<64hex>`로 fail-closed하고, self-consistent record hash를 가진 empty/uppercase malformed fixture도 차단했다.
- no-delta stale `CurrentChangeDeclaration`, AdapterContractRevision decrease, pure Schema/Adapter revision-only advance의 declaration/minimum Staging impact matrix를 Automation으로 고정했다.
- synthetic accepted valid fixture는 임의 suffix 대신 다른 canonical SHA-256을 사용해 new integrity gate와 각 negative failure 원인을 분리했다.
- fresh official UE 5.8 Build `2f1003126fa1435f8087af24a947305d` Exit 0 PASS와 DACE focused `3aa77b32ed014a13ac813ab00b3be81e` exact 12/12 PASS를 확보했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / canonical Product Staging mutation 0 / exact3 JSON scoped diff 0 / production accepted snapshot append 0이며 `CFDAContractBase.cpp` bootstrap exact1, 현재 단일 Active `CF-FQ-039`와 기존 병렬 dirty를 보존했다.
- exact next를 `DACE-P0-04 Migration Impact Guard`로 전진했다. P0-04의 Resolution/Evidence/canonical exact3 compatibility/promotion HOLD는 아직 구현 완료로 간주하지 않는다.

### v0.4.1 - 2026-09-09

- `DACE-P0-03` 중간검수를 수행해 `P0 0 / P1 2 / P2 2 / Final Acceptance HOLD`로 판정했다.
- P1-1은 Source/Mapping-only 변화 전체를 category만으로 safe native refactor로 추론해 `NoMigration` 외의 보수적 Product migration 선언까지 차단하는 문제다. safe 판단을 explicit declaration으로 되돌리고 Source/Mapping-only에서도 `ProductMigrationReviewRequired`를 허용해야 한다.
- P1-2는 새 accepted snapshot의 four component signature가 empty/malformed여도 self-consistent `SnapshotSignature`를 만들면 chain validation을 통과할 수 있는 mandatory contract integrity 공백이다. accepted exact4 signature의 canonical SHA-256 형식을 fail-closed해야 한다.
- P2-1은 no-delta 상태의 stale `CurrentChangeDeclaration`을 early return이 무시하는 lifecycle hygiene 공백, P2-2는 AdapterContractRevision decrease와 pure revision-only advance의 declaration/minimum impact negative matrix 미고정이다.
- 기존 Build `cab634bae876405ca8342604c3662f67` PASS와 DACE `23d3905af62b43c8824d4a0328c86c0d` exact 12/12 PASS는 교정 전 implementation evidence로 보존하되 P0-03 Final Acceptance 근거로 확대하지 않는다.
- 이번 중간검수는 read-only/code-review 단계라 Source 교정, Build/Automation 재실행, Product Apply/Save, canonical Product Staging mutation, accepted snapshot append를 수행하지 않았다. `CF-FQ-039` Active와 기존 병렬 dirty를 보존했다.
- exact next를 `DACE-P0-03 Correction + Re-review`로 되돌리고 `DACE-P0-04 Migration Impact Guard`는 HOLD했다.

### v0.4.0 - 2026-09-09

- `DACE-P0-03 Revision Guard`를 구현하고 post-implementation re-review `P0 0 / blocking P1 0 / P2 0` Technical PASS로 닫았다.
- Adapter physical shape 변화의 SchemaRevision bump 누락, declared SemanticContract 변화의 AdapterContractRevision bump 누락, Source/Mapping/Adapter/Semantic 또는 revision 변화의 migration declaration 누락을 fail-closed했다.
- `CurrentChangeDeclaration`을 latest `BaseSnapshotId`와 Guard-computed four-signature `CandidateContractSignature`에 결속하고 explicit `bImpactDeclared`로 default NoMigration과 선언된 NoMigration을 구분했다. Adapter/semantic 변화 또는 revision 전진은 최소 Staging migration impact를 요구한다.
- Adapter/Semantic/revision이 불변인 Source/Mapping-only safe native refactor는 revision bump를 강제하지 않되 explicit `NoMigration`을 필수화했다.
- accepted snapshot history의 unique SnapshotId, deterministic record signature, immediately-previous signature chain, Schema/Class identity, revision monotonicity와 adjacent revision/migration consistency를 검증한다. current production history에는 새 snapshot을 append하지 않아 Schema1/Adapter2 bootstrap exact1을 유지했다.
- P0-01 BootstrapSnapshot test를 future append-compatible frozen-first-record + full-chain validation으로 교정했다. 첫 exact12 PASS 후 post-implementation review에서 synthetic chain이 future exact2+ history를 보존하지 않는 test-only assumption을 발견해 production history 전체 copy + candidate append 방식으로 교정했다.
- 최종 official UE 5.8 Build `cab634bae876405ca8342604c3662f67` Exit 0 PASS와 DACE focused `23d3905af62b43c8824d4a0328c86c0d` exact 12/12 PASS를 확보했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / canonical Product Staging mutation 0 / exact3 JSON scoped diff 0, production accepted snapshot append 0이며 현재 단일 Active `CF-FQ-039`와 기존 병렬 dirty를 보존했다.
- `MigrationResolution`/`MigrationEvidenceId`, canonical Staging exact3 compatibility와 Pending promotion/append block은 `DACE-P0-04 Migration Impact Guard`가 이어서 닫는다.

### v0.3.2 - 2026-09-09

- `DACE-P0-02` 중간검수의 P1 3건/P2 2건을 전건 교정하고 재검수 `P0 0 / blocking P1 0 / P2 0` PASS로 닫았다.
- materializer↔extractor sentinel exact29를 SourceShape semantic leaf set과 exact 결속하고 duplicate path를 fail-closed했다. exact29 숫자는 current baseline assertion만 담당하며 coverage authority는 Source descriptor가 소유한다.
- SourceAdapterMapping의 `SourceToAdapter` leaf set과 serializer per-leaf distinguishable sentinel matrix를 exact 결속해 production serializer→parser semantic wiring을 leaf별로 검증한다.
- Reflection Array/Set/Map의 element/key/value와 nested container type을 recursive stable token으로 포함하고 same-outer-container inner-type drift fixture를 추가했다. current Pilot descriptor/bootstrap signature는 변하지 않았다.
- parser negative matrix를 issue code + exact FieldPath로 강화하고 fingerprint token observation을 dev-only thread_local + scoped RAII lifetime으로 격리했다.
- fresh official UE 5.8 Build `ae2df8748108459d9d74f8acfe14da79` Exit 0 PASS와 DACE runner `c2b7f592e8584aeea5325445f050d61d` exact 8/8 PASS를 확보했다. P0-01 baseline exact3도 함께 PASS해 accepted bootstrap chain 불변을 재확인했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / canonical Product Staging mutation 0 / exact3 JSON scoped diff 0이며 새 Product `.uasset` dirty entry 0이다. 현재 단일 Active `CF-FQ-039`와 기존 병렬 dirty를 보존했다.
- exact next를 `DACE-P0-03 Revision Guard`로 전진했다. CF-FQ-049 full affected regression은 direct operational behavior change가 없어 계획대로 P0-05가 소유한다.

### v0.3.1 - 2026-09-09

- `DACE-P0-02 Post-Implementation Mid-review`를 수행해 `P0 0 / P1 3 / P2 2 / Final Acceptance HOLD`로 판정했다.
- P1은 materializer↔extractor exact29 sentinel 목록의 Source descriptor 비결속, serializer actual physical shape 검증의 per-leaf value wiring 공백, Array/Set/Map element/key/value reflected type identity 미포함이다.
- P2는 parser negative matrix의 exact FieldPath 미검증과 dev-only fingerprint token sink의 process-global raw pointer isolation이다.
- 기존 official Build `6d20e32fa3aa4c7e886f0db46acbfd24` PASS와 focused exact 8/8 PASS는 교정 전 구현 evidence로 보존하되 P0-02 최종 PASS 근거로 확대하지 않는다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / canonical Product Staging mutation 0과 현재 단일 Active `CF-FQ-039`, 기존 병렬 dirty를 유지했다.
- exact next를 `DACE-P0-02 Correction + Re-review`로 되돌리고 `DACE-P0-03 Revision Guard`는 HOLD했다.

### v0.3.0 - 2026-09-09

- `DACE-P0-02 Structural Drift Detection`을 구현하고 Technical PASS로 닫았다.
- native Reflection actual Source exact30을 descriptor와 직접 대조하고, 가상 field add/remove/rename/type/nested-struct drift fixture를 모두 fail-closed했다.
- Adapter exact42 / mapping exact38, actual production serializer exact42, strict parser required/type/unknown matrix, fingerprint exact33 token/order, materializer↔extractor Source semantic leaf exact29 sentinel roundtrip을 신규 `CF_FQ_050.DACE_P0_02.*` 5종으로 검증했다.
- 첫 Automation에서 duplicate mapping fixture의 TArray self-alias assertion을 발견해 test-only fixture를 값 복사 방식으로 교정했다. fresh official UE 5.8 Build `6d20e32fa3aa4c7e886f0db46acbfd24` PASS와 DACE runner `5e1df0863a994702b09d082c0127364f` exact 8/8 PASS를 확보했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / canonical Product Staging mutation 0이며 exact3 JSON/.uasset scoped diff 0을 확인했다. 현재 단일 Active `CF-FQ-039`와 기존 병렬 dirty를 보존했다.
- exact next를 `DACE-P0-03 Revision Guard`로 전진했다. revision bump / CurrentChangeDeclaration / accepted snapshot chain evolution / migration impact gate는 아직 완료로 간주하지 않는다.

### v0.2.1 - 2026-09-09

- `DACE-P0-01 Contract Descriptor / Snapshot Foundation`을 구현하고 Technical PASS로 닫았다.
- `CFDAContractGuard.h/.cpp`에 SourceShape exact30 / AdapterShape exact42 / SourceAdapterMapping exact38 / SemanticContract exact26 descriptor와 deterministic SHA-256 signature foundation을 추가했다.
- `CFDAContractBase.cpp`에 SchemaRevision 1 / AdapterContractRevision 2의 production-owned append-only bootstrap accepted snapshot exact1과 previous-signature chain 검증을 추가했다.
- `CFDAStaging.cpp`, `CFDAStagingOps.cpp`, `CFDAStagingApply.cpp`에는 `WITH_DEV_AUTOMATION_TESTS` 범위의 narrow private probe만 연결해 actual production serializer/parser/fingerprint/extractor/materializer를 직접 관측하며 Public/Blueprint/write authority를 확대하지 않았다.
- 첫 focused Automation이 UE `FString` implicit case-insensitive ordering과 accepted hash 불일치를 검출해 descriptor canonical sort를 explicit case-sensitive ordinal 비교로 교정했고, final official UE 5.8 Build `18f03bf1806e430598e4fa4de7dcc019` PASS와 focused `c635c6f9b27f4722945b37bde2b548fc` exact 3/3 PASS를 확보했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / canonical Product Staging mutation 0이며 exact3 JSON/.uasset scoped diff 0을 확인했다. 현재 단일 Active `CF-FQ-039`와 기존 병렬 dirty를 보존했다.
- exact next를 `DACE-P0-02 Structural Drift Detection`으로 전진했다. P0-02의 reflection drift/negative coverage와 P0-03 revision/migration declaration guard는 아직 완료로 간주하지 않는다.

### v0.2.0 - 2026-09-09

- DACE-P0-00 Initial Design Review의 `P1 6 / P2 2`를 current Source에 맞춰 전건 교정하고 재검수 `P0 0 / blocking P1 0 / P2 0` PASS로 닫았다.
- Source/Adapter signature 직접 동등 비교를 폐기하고 `SourceShape / AdapterShape / SourceAdapterMapping / SemanticContract` 4-signature model을 확정했다.
- production `CFDAStagingOps` canonical serializer, strict parser, semantic fingerprint, extractor↔materializer를 descriptor와 actual behavior로 대조하는 mandatory probe contract를 확정해 목록-only fake PASS를 차단했다.
- semantic Guard 보장을 declared semantic policy와 behavior-checkable 범위로 축소하고 arbitrary 미선언 code meaning은 affected regression + review 책임으로 분리했다.
- accepted baseline authority를 production Editor Private `CFDAContractBase.cpp` append-only snapshot chain으로 확정했다. 아직 accepted되지 않은 변화는 `CFDAContractGuard.cpp`의 별도 `CurrentChangeDeclaration`이 latest BaseSnapshotId와 actual CandidateContractSignature를 결합해 소유하며, migration Impact/Resolution/EvidenceId가 Pending이면 snapshot append 및 Current System promotion을 차단하도록 고정했다.
- current whole-struct MissileGuideConfig materializer를 반영해 nested per-field materializer list 요구를 제거하고 sentinel behavior roundtrip으로 coverage하도록 했다.
- developer issue/result는 기존 `ECFDAStagingIssueCode`와 분리하고 `CarFight.DataManagement.CF_FQ_050.DACE_*` Automation namespace를 사용하도록 확정했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / Product Staging JSON mutation 0 / Source implementation 0 / Build 0 / Automation execution 0이며 현재 단일 Active `CF-FQ-039`와 unrelated dirty를 보존했다.
- exact next를 `DACE-P0-01 Contract Descriptor / Snapshot Foundation`으로 전진했다.

### v0.1.1 - 2026-09-09

- `DACE-P0-00 Initial Design Review`에서 current `UCFMissileGuidePresetData` / `FCFMissileGuideConfig`, `CFDAStaging.cpp`, `CFDAStagingApply.cpp`, `CFDAStagingOps.cpp`, canonical Product Staging exact3와 관련 Automation source를 read-only 교차감사했다.
- current top-level authored property exact4, `FCFMissileGuideConfig` authored UPROPERTY exact26, SchemaRevision 1 / AdapterContractRevision 2를 확인했다.
- 검수 결과는 `P0 0 / P1 6 / P2 2 / Implementation HOLD`다. canonical serializer 누락, Source↔Adapter signature mapping 불명확, descriptor↔실행 surface duplicate authority, semantic guarantee 과장, accepted snapshot authority, migration-impact promotion gate를 blocking P1으로 기록했다.
- current whole-struct materializer 특성과 developer issue/test isolation 경계는 P2로 기록했다.
- Product Low/Normal/High ApplyReviewed 0 / UE Asset Save 0 / Product Staging JSON write 0 / Build 0 / Automation execution 0이며 현재 단일 Active `CF-FQ-039`와 unrelated dirty를 변경하지 않았다.
- exact next를 `DACE-P0-00 Correction + Re-review`로 전환했다.

### v0.1.0 - 2026-09-09

- USER 요청으로 `CF-FQ-050 Data Asset Contract Evolution Guard`를 P2 / Ready 정식 후속 Feature로 승격했다.
- CF-FQ-049를 재오픈하지 않고 완료된 Data Asset Staging Current System의 schema evolution / adapter drift 누락을 fail-closed하는 P0 방향을 정의했다.
- structural drift는 자동 검출하고 semantic meaning 변화는 explicit declaration + AdapterContractRevision guard로 다루는 경계를 고정했다.
- SchemaRevision / AdapterContractRevision / migration impact declaration / canonical Staging compatibility를 machine-checkable하게 만드는 P0 단계를 정의했다.
- generic Reflection writer, old JSON 자동 migration, 새 Product 값 자동 추측, Product Low/Normal/High 무단 Apply/Save, 다른 DA 타입 onboarding은 P0 scope에서 제외했다.
- exact next gate는 `DACE-P0-00 Initial Design Review`이며 이번 승격에서는 Source/Asset/Build/Automation mutation을 수행하지 않는다.

---

## 20. Migration

- CF-FQ-049 `DataAssetStagingPlan.md`의 완료 evidence와 `DataAssetAuthoring.md` Current 계약은 그대로 유효하다.
- 이후 Staging 지원 DataAsset의 C++ authoring contract 변경은 CF-FQ-050이 정의할 Contract Evolution Guard를 통과해야 하는 방향으로 확장한다.
- CF-FQ-050 완료 전까지 기존 MissileGuidePreset schema evolution은 현재의 수동 revision discipline을 유지하며, 자동 guard가 이미 존재한다고 가정하지 않는다.
- 다른 DataAsset 타입을 Staging에 추가하는 작업은 CF-FQ-050 완료 전 자동 착수하지 않는다.
