# CarFight Data Asset Staging / Batch Authoring

- 문서 버전: v1.1.0
- 최근 갱신일: 2026-09-09
- 문서 상태: Current
- 완료 Feature: `CF-FQ-049 Data Asset Staging·Batch Authoring` + `CF-FQ-050 Data Asset Contract Evolution Guard`
- 현재 구현 범위: `CFMissileGuidePresetData` typed Staging / Preview / Reviewed Apply + Contract Evolution Guard P0
- Current System owner: 이 문서 + 실제 `CarFight_ReEditor/DataAuthoring` Source

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

현재 정식 지원 Typed Adapter는 다음 하나다.

```text
UCFMissileGuidePresetData
SchemaId: CarFight.DataAsset.MissileGuidePreset
SchemaRevision: 1
AdapterContractRevision: 2
ClassPath: /Script/CarFight_Re.CFMissileGuidePresetData
```

현재 canonical Product Pilot은 다음 세 개다.

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

P0 완료만으로 `CFAmmoData`, `CFDamageData`, `CFVehicleSensorData` 또는 다른 DataAsset 타입이 자동 지원되는 것은 아니다. 새 타입은 별도 후속 lifecycle에서 Typed Adapter/Schema와 가치·공수를 검토한 뒤 추가한다.

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

## 10. Typed Materializer

P0 materializer는 generic Reflection writer가 아니다.

지원 대상은 정확히 다음 typed class 하나다.

```text
UCFMissileGuidePresetData
```

Create:

```text
CreatePackage
→ NewObject<UCFMissileGuidePresetData>
→ typed whole-record apply
→ typed semantic pre-save readback
→ AssetRegistry::AssetCreated
→ MarkPackageDirty
→ exact SavePackage
```

Update:

```text
existing exact target resolve
→ original typed snapshot
→ whole-record typed apply
→ typed semantic pre-save readback
→ exact SavePackage
```

`Save All`을 사용하지 않는다.

unrelated package를 저장하지 않는다.

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

Reviewed Apply / durable save:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFDAStagingApply.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFDAStagingApply.cpp
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
Current owner: Document/Systems/DataManagement/DataAssetAuthoring.md v1.1.0
Product Low/Normal/High Apply·Save during CF-FQ-049/050 closure: 0
canonical Product Staging mutation during CF-FQ-050 closure: 0
accepted snapshot history: bootstrap exact1 / CF-FQ-050 closure append 0
CF-FQ-039 Active lifecycle: unchanged
```

후속 타입 확장이나 별도 UI는 CF-FQ-049/050의 old next gate를 재사용하지 않고 새로운 Feature/lifecycle로 연다.

---

## 19. Changelog

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

- CF-FQ-050 완료 이후 Staging 지원 DataAsset의 contract evolution 판단은 이 문서와 실제 `CFDAContractGuard*` / typed Staging Source를 함께 우선한다.
- CF-FQ-049 완료 이후 현재 구현 판단은 이 문서와 실제 `UE/Source/CarFight_ReEditor/Public/DataAuthoring/` + `UE/Source/CarFight_ReEditor/Private/DataAuthoring/` Source 경계를 우선한다.
- `Document/Plan/DataAssetStaging/DataAssetStagingPlan.md`는 완료 당시 상세 설계·검수·Build/Automation evidence를 보존하는 Historical 문서로 사용한다.
- 기존 `DataAssetManagement.md`는 계속 read-first Data Asset Manager owner이며 write/apply authority를 이 문서와 합치지 않는다.
- Product Low/Normal/High canonical Staging JSON은 Git-reviewable authoring input으로 유지하지만 persisted `.uasset`이 최종 Source of Truth다.
- 새 DataAsset 타입 확장은 current MissileGuidePreset adapter에 arbitrary Reflection을 붙이지 말고 별도 Typed Adapter/Schema와 검증 lifecycle을 추가한다.
