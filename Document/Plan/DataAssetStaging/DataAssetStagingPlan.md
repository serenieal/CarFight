# CF-FQ-049 Data Asset Staging / Batch Authoring Plan

- 문서 버전: v0.7.0
- 최근 갱신일: 2026-09-09
- 문서 상태: Done / P0 Complete / `DAS-P0-05 Final Acceptance PASS` / Current System Promotion Complete / `P0 0 / P1 0 / P2 0` / Product UE Asset Apply·Save 0 / Historical + Retained Path / G5 Deferred / CF-FQ-039 Active 보호
- Feature: `CF-FQ-049 Data Asset Staging·Batch Authoring`
- 우선순위: P2
- 현재 정확한 Gate: `P0 Complete / Historical + Retained Path / G5 Deferred`
- 완료 Gate: P0-04 test-owned durable fixture에서 Create/Update SavePackage→disk reload→typed semantic readback, drift/dirty/save uncertainty/PartialApplied를 검증했다. P0-05에서 canonical Product Staging 3종, exact selection, same-selection fresh Review, stale hash reject와 Sync partial-write raw-byte rollback을 확보했고 final official Build + OperationalEntry 1/1 + existing DAS 13/13 PASS를 유지한다. Final Acceptance fresh persisted audit에서 `CFMissileGuidePresetData` 3개를 확인한 뒤 `Document/Systems/DataManagement/DataAssetAuthoring.md v1.0.0`으로 Current System Promotion을 완료했다. Product Low/Normal/High `ApplyReviewed`는 closure에서 실행하지 않아 Apply·Save 0이다.
- 현재 단일 Active Feature: `CF-FQ-039 Production UI Visual Rework` 유지

---

## 1. 목적

CarFight의 DataAsset 종류와 파생 데이터 수가 증가하면서, 데이터 한 건을 만들거나 조정할 때마다 Unreal Editor를 기동하고 Editor Automation으로 `.uasset`을 직접 생성·저장하는 작업 단위가 과도하게 무거워질 수 있다.

이 Feature의 목적은 **Editor가 꺼져 있는 동안에도 사람이 읽을 수 있는 외부 Staging 파일에 신규/수정 의도를 계속 쌓아두고, Editor를 사용할 수 있을 때 검토·검증된 항목만 한 번에 Unreal DataAsset으로 반영하는 저작 경로**를 만드는 것이다.

이 Feature는 CarFight의 DataAsset Source of Truth를 외부 파일로 이전하지 않는다.

```text
Editor OFF

AI / USER
  ↓
Staging Draft (JSON 또는 CSV)
  ↓
Git에서 읽기·수정·검토 가능
  ↓
여러 건 누적

────────────────────────────────

Editor ON

Staging 발견
  ↓
Typed Parse / Validation
  ↓
Preview
  ├─ Create
  ├─ Update
  ├─ NoChange
  ├─ Conflict
  └─ Invalid
  ↓
명시적 Reviewed Apply
  ↓
Unreal DataAsset 생성/수정
  ↓
Persisted `.uasset`
  ↓
최종 Unreal Source of Truth
```

핵심 목표는 **Unreal을 제거하는 것**이 아니라, Unreal이 필요한 무거운 Asset materialization 작업을 데이터 한 건마다 반복하지 않고 **검토 가능한 Batch 단위로 모으는 것**이다.

---

## 2. 현재 프로젝트 기준선

### 2.1 CF-FQ-045 Data Asset Management

`CF-FQ-045 CarFight Data Asset Management`는 Done이며 현재 구현 owner는 다음이다.

```text
Document/Systems/DataManagement/DataAssetManagement.md v1.0.0
```

현재 Data Asset Manager의 역할은 다음과 같다.

```text
기존 DataAsset 발견
→ 분류
→ 의미 설명
→ 검사
→ ID/Validation 상태 확인
→ Reference/Referencer 조회
→ 탐색 UI
```

CF-FQ-049는 CF-FQ-045를 재오픈하지 않는다.

Data Asset Manager는 P0에서 **이미 존재하는 Unreal Asset의 관리·관측 계층**으로 유지하며, Staging write/apply owner를 흡수하지 않는다.

### 2.2 CF-FQ-038 Vehicle Data Authoring

`CF-FQ-038 차량 데이터 Authoring 시스템`은 Paused이며 현재 Vehicle Builder의 Backend + Advanced Workspace 역할을 유지한다.

차량 Authoring에는 이미 다음 패턴이 존재한다.

```text
CSV + companion manifest
→ 3-way Preview
→ Conflict/Validation
→ Approval
→ Recipe/Profile Commit
→ Definition Apply
```

또한 Vehicle Builder는 `ResearchDraft.json`, `PhysicsDraft.json` 계열의 외부 Draft를 승인 전 중간 자료로 사용한다.

이 Feature는 이러한 **안전한 Draft/Preview/Apply 패턴을 참고**하지만, 차량 전용 Recipe/Profile/Resolver 계약을 일반 DA에 억지로 합치거나 재구현하지 않는다.

### 2.3 CF-FQ-030 Missile Guidance

`CF-FQ-030 물리 제한형 미사일 비행·유도`는 Done이며 Runtime/Guidance 계약을 재오픈하지 않는다.

현재 `CFMissileGuidePresetData`는 Low / Normal / High 저작 프리셋을 저장하며, 기존 Editor 저작 경로에는 다음 패턴이 존재한다.

```text
CreatePackage
→ NewObject<UCFMissileGuidePresetData>
→ AssetRegistry::AssetCreated
→ MarkPackageDirty
→ SavePackage
```

이 구조는 CF-FQ-049의 첫 Pilot으로 적합하지만, Pilot은 **Authoring 경로만 검증**하며 Missile Runtime/Guidance Law/USER Feel을 다시 설계하지 않는다.

---

## 3. Source of Truth / Authority 원칙

### 3.1 `.uasset`은 최종 Source of Truth다

CF-FQ-049 P0에서도 정상 적용이 끝난 CarFight DataAsset의 최종 권한은 Unreal `.uasset`이다.

```text
Staging JSON/CSV
= 작성 중인 의도 / Batch 입력 / Review 대상

Persisted DataAsset `.uasset`
= 적용 완료 후 최종 Unreal Source of Truth
```

외부 Staging 파일은 Runtime Source가 아니며, Cooked Game이 Staging 파일을 읽어 게임플레이 데이터를 구성하지 않는다.

### 3.2 Staging은 경쟁 SSOT가 아니다

Staging 파일과 `.uasset` 값이 다를 때 단순히 Staging을 우선하여 덮어쓰지 않는다.

기존 Unreal Asset이 Staging baseline 이후 직접 수정됐거나 현재 값이 예상값과 다르면 Preview 단계에서 `Conflict` 또는 별도 Drift 상태로 분류해야 한다.

### 3.3 일반 파일 writer가 `.uasset` binary를 직접 작성하지 않는다

Editor 밖에서는 JSON/CSV 같은 텍스트 Staging만 작성한다.

실제 `.uasset` 생성·수정은 Unreal Editor-side package/UObject/AssetRegistry/serialization 계약을 통해서만 수행한다.

---

## 4. P0 핵심 안전 계약

CF-FQ-049 P0는 다음 원칙을 구현 전부터 보호한다.

1. **Editor-off Staging 작성은 Unreal 프로세스를 요구하지 않는다.**
2. Staging 하나를 수정할 때마다 Editor를 자동 기동하지 않는다.
3. P0 materialization의 기본 실행면은 **이미 실행 중인 Editor의 Editor-side C++ service + 명시적 Apply**다. Headless `UnrealEditor-Cmd`/Commandlet은 P0 필수 조건이 아니다.
4. Apply 전에는 반드시 mutation 0 Preview가 존재해야 한다.
5. Apply는 사용자가 검토한 exact target set에만 수행한다.
6. 기존 `.uasset`의 예상 밖 직접 수정은 silent overwrite하지 않는다.
7. **기존 Target package가 Apply 시작 전에 dirty이면 `TargetDirtyUnowned` 의미의 blocker로 fail-closed하고 mutation 0으로 종료한다.** P0에서는 사용자의 pre-existing unsaved package를 자동 merge/save/clean하지 않는다.
8. 이번 operation이 새로 생성한 package의 dirty만 operation-owned 상태로 허용한다.
9. Update row에는 `BaseSemanticFingerprint`가 필수다. Create row는 baseline 부재를 허용한다.
10. Approval은 `SchemaId + SchemaRevision + AdapterContractRevision + StagingSemanticFingerprint + Type/StableIdentity/ObjectPath + Base/Current fingerprint + PlannedOperation + exact sorted target set + BatchPlanHash`에 binding한다.
11. Approval 뒤 Apply 직전에 모든 target의 current truth를 다시 읽는 global TOCTOU preflight를 수행한다. 하나라도 달라지면 전체 mutation 전에 stale로 차단한다.
12. Approval은 one-shot Apply evidence다. Apply 요청 뒤 global preflight에서 stale/blocker가 발견되어 mutation 0으로 끝나도 같은 approval을 자동 재사용하지 않고 fresh Preview/Approval을 요구한다. global preflight가 전부 PASS하면 첫 mutation 직전에 consumed 상태로 고정한다.
13. 여러 package durable save는 파일시스템 전체 atomic rollback을 약속하지 않는다. deterministic order로 처리하고 첫 실패에서 Stop하며, 이미 durable-confirmed target은 유지하고 나머지는 `NotRun`으로 남긴다.
14. durable-confirmed success 뒤 **known failed-before-durable** 실패가 발생하고 unknown state가 없을 때만 Batch를 `PartialApplied`로 보고한다. `SaveStateUnconfirmed` 또는 `InMemoryStateUnconfirmed`가 발생하면 해당 uncertainty 결과가 `PartialApplied`보다 우선하며 automatic continue/retry/rollback을 수행하지 않는다.
15. `SavePackage=true`만으로 success를 인정하지 않는다. `package clean + persisted package/object 확인 + typed semantic readback이 필요한 타입의 readback`까지 확인한 경우만 `DurableApplied`로 본다.
16. raw save 결과와 durable 상태가 일치하지 않으면 `SaveStateUnconfirmed` 의미로 fail-closed하고 Batch를 중단한다. 이 상태에서는 디스크가 바뀌었을 수 있으므로 성공을 주장하거나 자동 rollback을 주장하지 않는다.
17. P0 Update는 **whole-record typed replacement**다. 지원 writable field 누락은 `Invalid`, partial patch는 Scope Out이다.
18. Stable identity가 있는 타입은 typed identity를 우선하며, 같은 identity가 다른 path에 있으면 자동 신규 생성/이동하지 않고 `TargetMoved` 의미의 Conflict로 처리한다. 대상 path에 다른 class/identity가 있으면 `PathCollision` 의미의 Conflict다.
19. Stable identity가 N/A인 타입은 exact ObjectPath + mandatory Update baseline을 authority로 사용하고 자동 relocation하지 않는다.
20. P0에서 delete는 기본 범위에서 제외한다. Staging에서 행/파일이 사라졌다는 이유만으로 Unreal Asset을 자동 삭제하지 않는다.
21. unrelated package Save/Save All을 수행하지 않는다.
22. automatic retry로 같은 write를 반복하지 않는다.
23. arbitrary Reflection property writer를 만들지 않는다. 지원 타입별 **Typed Adapter / Typed Schema**를 사용한다.
24. CF-FQ-045 Data Asset Manager의 read-first 역할을 writer 역할로 확장하지 않는다. durable commit 뒤 Manager refresh는 post-commit convenience이며 실패해도 durable write를 실패로 되돌리지 않는다.
25. CF-FQ-038 Vehicle Authoring과 Vehicle Builder의 Recipe/Profile/Apply authority를 침범하지 않는다.

---

## 5. 초기 대상 분류

현재 Source 구조 기준의 P0 적합도는 다음과 같이 본다. 이 표는 `DAS-P0-00` 감사에서 다시 current Source와 대조해 확정한다.

| DataAsset 계열 | Staging 적합도 | P0 판단 |
| --- | --- | --- |
| `CFMissileGuidePresetData` | 매우 높음 | 첫 Pilot / 현재 CF-FQ-045 27종 Registry에는 아직 미등록이므로 P0-01 coverage 보강 필요 |
| `CFAmmoData` | 높음 | 숫자/ID 중심이지만 `TSoftObjectPtr<UTexture2D>`가 있어 Post-P0 Hybrid 후보 |
| `CFDamageData` | 매우 높음 | Post-P0 확장 후보 |
| `CFVehicleSensorData` | 높음 | Post-P0 확장 후보 |
| `CFWeaponData` | 중간 | P0 일반화 제외, 후속 재평가 |
| `CFEquipmentPresetData` | 중간 | Asset reference 중심이라 후속 재평가 |
| `CFVehicleFittingData` | 낮음~중간 | 구조/관계 복잡, P0 제외 |
| `CFProjectileData` | 낮음 | Blueprint Class/Mesh/Niagara/다른 DA 참조가 많아 P0 제외 |
| `CFCombatFxData` | 낮음 | Niagara/UE Asset 결합이 강해 P0 제외 |
| `CFVehicleData` | 매우 낮음 | Vehicle Builder/Recipe/Profile 전용 경로 유지, P0 제외 |

P0 완료만으로 Ammo/Damage/Sensor를 자동 편입하지 않는다. Missile Pilot 뒤 확장 가치와 공수를 별도로 판정한다.

---

## 6. 파일 포맷 원칙

P0는 하나의 포맷으로 모든 DataAsset을 강제하지 않는다.

```text
단순 반복형 레코드
→ CSV 후보

중첩 Struct / Config 중심 데이터
→ JSON 후보
```

첫 Pilot인 `CFMissileGuidePresetData`는 `MissileGuideConfig` 중첩 구조가 있으므로 **P0 canonical interchange format을 JSON으로 고정**한다.

Excel은 CSV를 편집하는 사용자 도구로 사용할 수 있지만 `.xlsx` 자체를 P0 canonical interchange format으로 만들지 않는다.

XML은 현재 CarFight에 특별한 필요성이 확인되지 않았으므로 P0 Scope Out이다.

---

## 7. Staging lifecycle

최소 lifecycle은 다음을 목표로 한다.

```text
Draft
  ↓
Parsed
  ↓
Validated
  ↓
Previewed
  ↓
Reviewed Apply
  ↓
Persisted DataAsset
```

Preview의 최소 terminal classification은 다음과 같다.

| 상태 | 의미 |
| --- | --- |
| `Create` | 대상 Unreal Asset이 없고 신규 생성 조건이 유효함 |
| `Update` | 대상 Asset이 있으며 reviewed 변경 후보가 존재함 |
| `NoChange` | Staging 의미와 current Unreal 의미가 동일함 |
| `Conflict` | baseline/current/staging 관계상 안전한 자동 반영을 결정할 수 없음 |
| `Invalid` | schema, identity, 값 범위, path 또는 typed validation 실패 |

Conflict 진단은 최소 `TargetDirtyUnowned`, `BaselineMismatch`, `TargetMoved`, `PathCollision`, `ApprovalStale` 의미를 기계적으로 구분할 수 있어야 한다. exact C++ enum 이름은 P0-01에서 확정하되 문자열 parsing으로 상태를 판정하지 않는다.

Apply 결과는 최소 다음 의미를 구분한다.

```text
DurableApplied
NoChange
BlockedBeforeMutation
FailedBeforeDurableWrite
InMemoryStateUnconfirmed
SaveStateUnconfirmed
PartialApplied
PostCommitWarning
```

`PostCommitWarning`은 durable write 뒤 Manager refresh 같은 후처리만 실패한 상태이며 write failure로 축소하지 않는다.

표시 순서나 localized label은 identity로 사용하지 않는다.

---

## 8. Identity / Schema 설계 요구

`DAS-P0-01`은 다음 항목을 current normative contract로 동결한다.

```text
SchemaId
SchemaRevision
AdapterContractRevision
DataAssetTypeClassPath
StableLogicalId + IdentityPolicy
TargetObjectPath
CanonicalTypedPayload
StagingSemanticFingerprint
BaseSemanticFingerprint
CurrentSemanticFingerprint
PlannedOperation
BatchPlanHash
```

`BaseSemanticFingerprint`는 **Update에 필수**, Create에는 없음이 정상이다. fingerprint는 localized diagnostic, transient UObject state, Asset Registry cache metadata를 제외하고 해당 Typed Adapter가 소유하는 canonical writable payload를 기준으로 계산한다.

Stable Logical ID와 ObjectPath는 같은 역할이 아니다.

```text
Identity Required/Optional 타입
→ typed logical identity가 semantic authority
→ ObjectPath는 저장 위치/target locator

Identity N/A 타입
→ exact ObjectPath + BaseSemanticFingerprint authority
```

Asset 이름만으로 모든 semantic identity를 추론하지 않는다. identity가 같은 Asset을 다른 path에서 발견했다고 자동 rename/move하지 않는다.

Schema revision이 바뀌었을 때 old Staging을 silent reinterpret하지 않고 migration 또는 명시적 rejection 경계를 가진다. Schema가 같더라도 canonicalization/validation 의미가 바뀌면 `AdapterContractRevision`을 올려 old Preview/Approval을 stale 처리한다.

---

## 9. P0 단계

### DAS-P0-00 — Current DA Authoring / Creation Audit

목적: 현재 CarFight의 DA 생성·편집 경로와 실제 DataAsset 종류를 read-only로 다시 확정한다.

확인 범위:

```text
- current UPrimaryDataAsset / Editor authoring DA inventory
- CF-FQ-045 Type Registry / semantic policy 재사용 가능 경계
- 직접 CreatePackage/NewObject/SavePackage authoring 경로
- Vehicle Batch CSV/manifest의 재사용 가능한 패턴과 차량 전용 경계
- Vehicle Builder JSON Draft 경로
- MissileGuidePreset current authoring/idempotence 경로
- 각 DA의 UObject/SoftObject/Class/Niagara/Mesh 참조 복잡도
- 현재 저장/dirty/validation 관례
```

산출물:

```text
Staging Candidate / Hybrid / Native-only 분류
Generic Foundation이 실제로 소유할 최소 책임
기존 owner와 중복 금지 경계
P0-01 설계 입력
```

이 Gate는 Source/Asset mutation 0으로 수행한다.

#### DAS-P0-00 감사 결과 — PASS / 2026-09-08

current Source를 다시 대조한 결과 다음을 확정했다.

```text
CF-FQ-045
- FCFDATypeRegistry가 current concrete DA semantic/identity/validation metadata를 소유
- FCFDATypeAdapter는 loaded UObject read-only identity/validation adapter
- Manager는 writer/save owner가 아님
- 2026-09-03 Registry snapshot은 concrete 27종
- current Source full header scan은 direct UPrimaryDataAsset 20종 + Editor UDataAsset 7종이며, abstract UCFInventoryItemData 아래 concrete 2종을 별도 descriptor로 치환하는 기존 구조를 확인
- 이 대조에서 2026-09-08 추가된 UCFMissileGuidePresetData가 current Registry snapshot 이후 생긴 추가 concrete coverage gap으로 확인됨

CF-FQ-038 Vehicle Batch
- Export baseline + Current fingerprint 3-way Preview 존재
- BatchPlanHash / exact Approval / current fingerprint 재검증 존재
- global TOCTOU preflight / typed transaction / rollback / dirty-state restore 존재
- B1/B2는 disk SavePackage를 의도적으로 수행하지 않음

Vehicle Builder / Apply
- specialized vehicle creation/apply/save owner 유지
- CF-FQ-049가 Vehicle Recipe/Profile/Definition authority를 가져오지 않음

CF-FQ-030 Missile Preset
- UCFMissileGuidePresetData는 passive UPrimaryDataAsset
- PresetId + DisplayName + Description + MissileGuideConfig를 소유
- current Editor authoring은 CreatePackage/NewObject/AssetRegistry/SavePackage 계열
- existing Ensure는 기존 asset 값을 load-time/re-run에 덮지 않는 idempotence evidence
```

추가로 `CFMissileGuidePresetData`는 2026-09-08 추가된 concrete DA라 2026-09-03 기준 `RegisterCurrentCarFightDescriptors()`의 27종에는 아직 포함되지 않은 것을 확인했다. 따라서 P0 Pilot은 기존 Registry coverage를 가정하지 않는다.

P0-01에서 다음 compatibility seam을 동결한다.

```text
CFMissileGuidePresetData semantic descriptor coverage 추가
- ClassPath: /Script/CarFight_Re.CFMissileGuidePresetData
- Stable identity: PresetId
- Identity Policy: Required
- Resolver meaning: Explicit FName
- CF-FQ-045 Registry ValidationPolicy: P0에서는 None 유지
- CF-FQ-049 typed staging adapter: Pilot whole-record schema/value validation 소유
```

이 coverage 보강은 **CF-FQ-045를 writer로 재오픈하는 작업이 아니라, 이후 추가된 concrete DA를 current read-only semantic inventory에 반영하는 호환 유지보수**로 취급한다. Manager의 read-first 역할과 기존 UI 계약은 그대로 유지한다.

`FCFDATypeAdapter::ResolveStableIdentity`는 private read-only 구현이므로 CF-FQ-049가 이를 우회 호출하거나 복사하지 않는다. P0-01에서 descriptor의 public identity policy를 재사용하면서 authoring용 typed adapter가 동일 semantic contract를 따르도록 하거나, 필요하면 작은 공용 read-only identity primitive만 추출한다.

Candidate 재분류도 current Source 기준으로 확정했다.

```text
MissileGuidePreset = P0 Staging Pilot
DamageData         = Post-P0 Staging 유력
VehicleSensorData  = Post-P0 Staging 유력 / Identity N/A이므로 path+baseline형
AmmoData           = Post-P0 Hybrid 유력 / Soft Texture reference 존재
WeaponData         = Hybrid/Native 우선 / UObject refs + migration/PostLoad 존재
Projectile/CombatFx/Vehicle/Fitting = P0 Native/specialized 유지
```

Generic Foundation의 최소 책임은 `typed parse/canonicalize → identity/target resolve → 3-way Preview → exact approval → dirty/TOCTOU preflight → typed mutation → durable confirmation → partial-batch result`까지다. Runtime, Vehicle Recipe/Profile, DA Manager write, arbitrary reflection, Editor auto-lifecycle은 소유하지 않는다.

**DAS-P0-00 판정: PASS / Source mutation 0 / Asset mutation 0.**

### DAS-P0-01 — Staging Authority / Schema Design

목적: 외부 Staging과 Unreal `.uasset` 사이의 권한, identity, schema, conflict, approval, TOCTOU와 결과 의미를 **Source 구현 전에 고정**한다.

현재 단계에서는 Product Source와 UE Asset을 수정하지 않는다. 아래 결정이 P0-01의 normative contract이며 설계검수 통과 전에는 P0-02 구현을 시작하지 않는다.

#### P0-01.1 Staging 저장 위치 / 파일 단위

P0 canonical Staging root는 `main_game` checkout의 다음 Git-reviewable 경로로 고정한다.

```text
<main_game>/Authoring/DataAssetStaging/
```

Pilot 배치는 다음 구조를 사용한다.

```text
Authoring/DataAssetStaging/
└─ MissileGuidePreset/
   ├─ MissileFeel_Low.json
   ├─ MissileFeel_Normal.json
   └─ MissileFeel_High.json
```

이 위치를 선택하는 이유와 경계는 다음과 같다.

```text
- UE/Content 내부가 아니므로 JSON을 Unreal Asset처럼 import하지 않는다.
- UE/Saved, Intermediate, tmp가 아니므로 Editor-off 상태에서도 Git diff/review 대상으로 유지할 수 있다.
- current root .gitignore에서 제외되지 않는다.
- 파일 하나 = DataAsset whole-record 의도 하나로 고정해 충돌·리뷰 단위를 작게 유지한다.
- 파일명과 폴더명은 사람을 위한 organization key일 뿐 semantic identity authority가 아니다.
- 실제 authority는 record 내부 StableLogicalId + TargetObjectPath + typed payload다.
```

파일명 변경은 Asset rename/move를 뜻하지 않는다. Apply approval에는 normalized repository-relative Staging path도 exact source-set evidence로 binding하므로 Preview 뒤 파일을 rename/move하면 fresh Preview가 필요하다.

P0는 하나의 거대한 JSON array에 모든 DA를 넣지 않는다. Batch는 여러 whole-record JSON을 선택·발견한 뒤 Preview가 exact target set으로 조립한다.

#### P0-01.2 Pilot Schema identity / Registry prerequisite

Pilot schema는 다음으로 고정한다.

```text
SchemaId                = CarFight.DataAsset.MissileGuidePreset
SchemaRevision          = 1
AdapterContractRevision = 2
DataAssetTypeClassPath  = /Script/CarFight_Re.CFMissileGuidePresetData
```

`CFMissileGuidePresetData`는 `CF-FQ-045` current Registry snapshot 이후 추가된 concrete DA이므로 **P0-02 구현의 첫 Source prerequisite**로 semantic coverage를 먼저 보강한다. 이 prerequisite는 CF-FQ-045 writer 재오픈이 아니다.

```text
FCFDATypeRegistry descriptor
- ClassPath: /Script/CarFight_Re.CFMissileGuidePresetData
- Domain: Combat
- TypeDisplayName: 미사일 유도 프리셋 데이터
- IdentityPolicy: Required
- IdentityResolverKind: ExplicitFName
- IdentitySourceName: PresetId
- ValidationPolicy: None
- ValidationSourceName: empty

current registry implementation consequence
- CurrentDescriptors Reserve 27 → 28
- Manager user description 1건 추가
- current descriptor-count/coverage focused test expectation 27 → 28
```

`FCFDATypeAdapter::ResolveStableIdentity` private 구현을 CF-FQ-049가 복사하거나 private 우회 호출하지 않는다. Staging adapter는 public descriptor의 identity policy/resolver metadata를 사용하고, Pilot typed adapter가 `UCFMissileGuidePresetData::PresetId`를 같은 `ExplicitFName` 의미로 읽는다.

Registry는 타입의 read-only semantic metadata만 소유한다. Whole-record parse/value validation/fingerprint/write contract는 CF-FQ-049 typed staging adapter가 소유한다.

#### P0-01.3 JSON whole-record envelope

P0 JSON은 strict whole-record다. top-level required field는 다음 8개다.

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

`StagingSemanticFingerprint`, `CurrentSemanticFingerprint`, `PlannedOperation`, `BatchPlanHash`는 사용자가 JSON에 직접 적는 값이 아니라 **typed parse/Preview가 계산하는 derived evidence**다.

규칙:

```text
- unknown top-level field = Invalid
- required field 누락 = Invalid
- Update record: BaseSemanticFingerprint = required sha256 fingerprint
- Create record: BaseSemanticFingerprint = null
- TargetObjectPath = exact full Unreal object path `/Game/.../Asset.Asset`
- P0 Pilot은 `/Engine`, `/Script`, transient object path를 target으로 허용하지 않음
- StableLogicalId는 descriptor identity policy에 따라 typed identity로 parse
- Required identity 타입에서는 top-level `StableLogicalId`와 Payload 안의 실제 identity field가 typed identity 의미로 정확히 같아야 함. Pilot에서는 `StableLogicalId == Payload.PresetId`; 다르면 `Invalid / StableIdentityMismatch`이며 어느 한쪽을 자동 수정하지 않음
- Payload는 지원 writable field 전체를 포함
- partial patch / omitted field means keep-current 의미는 P0에 없음
```

Update Staging을 처음 만들려면 baseline을 한 번 current Unreal Asset에서 캡처해야 한다. 기존 `.uasset`을 한 번도 읽지 않은 상태에서 Base fingerprint를 추측해 Update record를 만드는 경로는 지원하지 않는다. 반대로 Create record는 target 부재가 baseline이므로 Editor-off에서 `BaseSemanticFingerprint=null`로 처음부터 작성할 수 있다.

Pilot whole-record 예시는 다음과 같다. 아래 hash text는 형식 설명용 placeholder다.

```json
{
  "SchemaId": "CarFight.DataAsset.MissileGuidePreset",
  "SchemaRevision": 1,
  "AdapterContractRevision": 2,
  "DataAssetTypeClassPath": "/Script/CarFight_Re.CFMissileGuidePresetData",
  "StableLogicalId": "MissileFeel_Low",
  "TargetObjectPath": "/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low",
  "BaseSemanticFingerprint": "sha256:<64 lowercase hex>",
  "Payload": {
    "PresetId": "MissileFeel_Low",
    "PresetDisplayName": {
      "Kind": "Literal",
      "Text": "저성능 단순 추적"
    },
    "PresetDescription": {
      "Kind": "Literal",
      "Text": "좋은 초기 조준 탄도를 짧게 보존한 뒤 현재 관측 위치만 단순 추적하는 저가형 Pure Pursuit 체감 프리셋입니다."
    },
    "MissileGuideConfig": {
      "bUseGuidance": true,
      "GuideMode": "TargetActor",
      "LostTargetPolicy": "ContinueStraight",
      "NavigationConstant": 3.0,
      "MaximumTurnRateDegPerSec": 35.0,
      "MaximumLateralAccelerationCmPerSecSq": 2000.0,
      "GuidanceResponseTimeSeconds": 0.18,
      "MinimumGuidanceSpeedCmPerSec": 500.0,
      "SeekerFieldOfViewDeg": 60.0,
      "LockBreakAngleDeg": 85.0,
      "TargetLostGraceTimeSeconds": 0.20,
      "SeekerModel": "Stateful",
      "TargetObservationMode": "SampledPositionEstimate",
      "GuidanceLaw": "PurePursuit",
      "GuidanceActivationMode": "Independent",
      "GuidanceActivationDelaySeconds": 0.25,
      "GuidanceActivationDistanceCm": 500.0,
      "LeadTimeSeconds": 0.0,
      "MaxLeadDistanceCm": 0.0,
      "ReacquisitionMode": "None",
      "TargetObservationIntervalSeconds": 0.08,
      "TargetVelocityEstimateResponseTimeSeconds": 0.25,
      "AcquisitionConeHalfAngleDeg": 45.0,
      "TrackingConeHalfAngleDeg": 60.0,
      "ReacquisitionConeHalfAngleDeg": 60.0,
      "ReacquisitionTimeSeconds": 0.0
    }
  }
}
```

`EditCondition`으로 현재 Runtime에서 사용되지 않는 field도 whole-record에서 생략하지 않는다. 저장된 raw authored state 전체가 replacement 대상이므로 조건부 비활성 field도 schema member로 유지한다.

#### P0-01.4 Canonical typed representation

JSON 원문 byte/공백/key order는 semantic authority가 아니다. 반드시 `JSON → strict typed DTO → canonical typed token stream`을 만든 뒤 fingerprint를 계산한다.

공통 규칙:

```text
- UTF-8 JSON
- JSON object key physical order는 무의미
- unknown key는 typo 은폐를 막기 위해 Invalid
- hash input은 localized message, UI label, UObject pointer, Asset Registry cache를 포함하지 않음
- canonical token은 delimiter collision을 피하는 label + length + value 형태의 deterministic stream
- hash algorithm은 SHA-256, 표기는 `sha256:` + 64 lowercase hex
```

Pilot 타입별 canonical 표현은 다음으로 고정한다.

| UE 타입 | JSON 표현 | canonical 의미 |
| --- | --- | --- |
| `FName` | JSON string | typed parse 후 `FName` semantic 비교. Required identity는 `NAME_None`/empty 금지. fingerprint comparison token은 case-insensitive FName 의미에 맞춘 normalized comparison text를 사용하고 표시 casing은 payload DTO에 보존 |
| `FText` | `{ "Kind": "Literal", "Text": "..." }` | P0는 simple literal text만 지원. StringTable 또는 별도 localization identity 보존이 필요한 표현은 `UnsupportedTextRepresentation`으로 fail-closed. fingerprint는 literal text codepoint sequence를 사용 |
| `enum` | enumerator token string | 예: `"PurePursuit"`. display label/정수 ordinal/알 수 없는 값 금지. typed enum의 exact enumerator token으로 canonicalize |
| `struct` | strict JSON object | schema가 선언한 writable member 전체 required. nested member도 같은 typed canonical rule을 재귀 적용 |
| `bool` | JSON boolean | `true/false`만 허용, 문자열 `"true"` 금지 |
| `float` | JSON number | finite 값만 허용. typed `float`로 parse 후 `-0`은 `0`과 동일 semantic으로 canonicalize. fingerprint numeric token은 formatter 차이에 의존하지 않는 normalized typed value를 사용 |

수치 validation은 `GetEffectiveConfig()`처럼 값을 clamp한 결과를 저장 의미로 사용하지 않는다.

```text
raw authored value가 current schema 범위를 벗어남
→ InvalidValue
→ silent clamp/normalize/save 금지
```

이 원칙 때문에 서로 다른 raw authored 값이 같은 effective Runtime 값으로 clamp된다는 이유로 `NoChange`가 되지 않는다.

#### P0-01.5 SchemaRevision / AdapterContractRevision

두 revision은 역할을 분리한다.

```text
SchemaRevision
= JSON 구조가 바뀜
  required field 추가/삭제
  field type/shape 변경
  canonical interchange 표현 변경

AdapterContractRevision
= JSON shape는 같아도 typed 의미가 바뀜
  identity resolution 의미
  canonicalization 의미
  validation 범위
  semantic fingerprint 구성
  target resolution / writable-field ownership 의미
```

P0는 exact current revision만 Apply 가능하다.

```text
unknown SchemaId                  → SchemaUnsupported
older/newer SchemaRevision        → SchemaRevisionUnsupported
AdapterContractRevision mismatch  → AdapterRevisionMismatch
```

old Staging을 silent reinterpret/migrate하지 않는다. 향후 migration이 필요하면 old revision reader + 명시적 migration 결과를 별도 구현하고, migration 뒤 fresh fingerprint/Preview를 만든다.

두 revision 모두 Preview row와 approval/BatchPlanHash에 binding한다. Adapter 의미가 바뀐 뒤 old approval 재사용은 금지한다.

#### P0-01.6 Base / Current / Staging semantic fingerprint

세 fingerprint는 **동일한 canonical payload domain**을 사용해야 직접 비교할 수 있다.

```text
SemanticFingerprint(payload)
= SHA-256(
    SchemaId
    + SchemaRevision
    + AdapterContractRevision
    + DataAssetTypeClassPath
    + canonical typed writable payload token stream)
```

의미:

```text
BaseSemanticFingerprint
= Update Staging을 만들거나 마지막 rebase할 때의 persisted Unreal semantic baseline

CurrentSemanticFingerprint
= Preview/Apply preflight 시 exact current target에서 다시 읽은 semantic state

StagingSemanticFingerprint
= 현재 Staging JSON을 strict typed parse/canonicalize한 desired semantic state
```

`Base/Current/Staging`은 raw file hash가 아니다. JSON whitespace/key order/decimal 표기 차이가 typed semantic zero이면 fingerprint도 같아야 한다.

`StableLogicalId`, `TargetObjectPath`, schema/revision, source Staging path와 PlannedOperation은 payload fingerprint에 억지로 섞지 않고 `BatchPlanHash`/approval에서 별도로 binding한다.

#### P0-01.7 exact 3-way Preview classification

Update baseline이 있는 row는 `Base / Current / Staging` 세 의미를 다음 표로 판정한다.

| Base | Current | Staging | Preview |
| --- | --- | --- | --- |
| 같음 | 같음 | 같음 | `NoChange` |
| Base=Current | Base=Current | Staging만 다름 | `Update` |
| Base!=Current | Current=Staging | Current=Staging | `NoChange` / converged external change, mutation 0 |
| Base!=Current | Staging=Base | Current만 다름 | `Conflict / BaselineMismatch` |
| 셋 모두 다름 | 셋 모두 다름 | 셋 모두 다름 | `Conflict / BaselineMismatch` |

즉 `Base != Current`을 무조건 overwrite 허용하지 않지만, **현재 Unreal이 이미 Staging desired state와 정확히 같아진 converged 경우만 NoChange**로 닫는다.

이 converged `NoChange`는 안전하지만 Staging의 Base가 과거 fingerprint로 남아 있다는 뜻이므로 `BaselineRebaseRequired` informational diagnostic을 함께 표시한다. Preview 자체는 Staging 파일을 자동 수정하지 않는다. 사용자가 이 record를 다시 편집하려면 현재 `CurrentSemanticFingerprint`를 새 Base로 명시적으로 rebase한 fresh Staging record에서 시작해야 한다. rebase는 `Current == Staging`을 다시 확인한 경우에만 허용하며, 그 외에는 old Base를 조용히 갱신하지 않는다.

Create row는 별도 규칙을 사용한다.

```text
Base = null + target/identity 없음
→ Create

Base = null + target 또는 같은 StableIdentity가 이미 존재
→ Conflict / UnexpectedExistingTarget
→ payload가 우연히 같아도 Create ownership을 추정하지 않음

Update baseline 존재 + target 없음
→ Conflict / TargetMissing
```

identity/path 규칙:

```text
same StableLogicalId, different path
→ TargetMoved

requested TargetObjectPath에 different class/identity
→ PathCollision

same Batch 안에서 duplicate StableLogicalId 또는 duplicate TargetObjectPath
→ Invalid / duplicate-target diagnostic
```

pre-existing target package가 dirty이면 persisted/current truth와 save ownership이 안전하지 않으므로 `Conflict / TargetDirtyUnowned`로 Preview하고 approval 대상에서 제외한다.

#### P0-01.8 Generic foundation reuse 선택

Initial Review의 A/B 선택지는 **B**로 동결한다.

```text
B. CF-FQ-049 전용 typed staging service를 구현하되
   CF-FQ-038 FCFBatchImportService의 proven safety semantics/test matrix를 재사용한다.
```

P0에서 `FCFBatchImportService`를 generic layer로 리팩터링하지 않는다. 완료된 Vehicle Authoring의 blast radius를 넓히지 않기 위해서다.

재사용할 의미:

```text
- physical text order와 독립적인 canonical semantic hash
- length-prefixed deterministic token stream
- exact included set
- per-target current fingerprint binding
- old approval 부분 축소 재사용 금지
- global preflight before mutation
- current truth re-read
- no automatic retry
```

재사용하지 않는 것:

```text
- Vehicle DatasetKind / Recipe / Profile / Resolver
- Vehicle CSV manifest 자체
- Vehicle row/cell patch model
- 기존 MD5 hash helper 자체
```

CF-FQ-049는 새 protocol에서 SHA-256을 사용하며 Vehicle Batch의 기존 MD5 contract를 P0 때문에 변경하지 않는다.

#### P0-01.9 BatchPlanHash

Preview에서 mutation candidate는 `Create`/`Update`만 포함한다. `NoChange`는 표시되지만 mutation set에는 들어가지 않는다. Conflict/Invalid가 포함된 선택 set은 approval을 만들 수 없다.

각 candidate를 다음 canonical key로 정렬한다.

```text
DataAssetTypeClassPath
+ StableLogicalId canonical comparison key
+ TargetObjectPath
+ normalized repository-relative Staging path
```

각 target plan token은 최소 다음을 포함한다.

```text
SchemaId
SchemaRevision
AdapterContractRevision
DataAssetTypeClassPath
IdentityPolicy
StableLogicalId canonical key
TargetObjectPath
StagingRelativePath
BaseSemanticFingerprint or <none>
CurrentSemanticFingerprint or <absent-target>
StagingSemanticFingerprint
PlannedOperation(Create|Update)
```

`BatchPlanHash`는 다음을 domain-separated SHA-256으로 계산한다.

```text
Kind = CarFightDAStagingBatchPlan
FormatRevision = 1
IncludedTargetCount
sorted exact target-plan tokens
```

localized diagnostic text, UI sort order, JSON physical key order는 포함하지 않는다. machine-readable blocking diagnostic code는 approval 생성 자체를 막으므로 mutable candidate BatchPlanHash에 넣어 승인 semantics를 혼합하지 않는다.

부분 적용을 원하면 old approval에서 row를 빼지 않는다. **선택 target set을 바꾼 fresh Preview/BatchPlanHash**를 새로 만든다.

#### P0-01.10 Approval / global TOCTOU

Approval은 보안 인증 토큰이 아니라 **사용자가 본 exact Preview와 실제 Apply 대상을 동일하게 묶는 one-shot evidence**다. P0에서 JSON Staging 파일 안에 approval을 써넣지 않는다.

Approval evidence는 최소 다음을 가진다.

```text
SchemaId / SchemaRevision / AdapterContractRevision
BatchPlanHash
exact sorted IncludedTargets
각 target의 Type / Identity / ObjectPath / StagingRelativePath
Base / Current / Staging semantic fingerprints
PlannedOperation
```

Apply는 mutation 전에 entire set에 대해 global TOCTOU preflight를 수행한다.

```text
1. exact Staging files를 디스크에서 다시 읽음
2. strict parse/canonicalize/revision 확인
3. StagingSemanticFingerprint 재계산
4. Registry descriptor/typed identity 재확인
5. target identity/path/class 재resolve
6. existing package dirty 여부 재확인
7. CurrentSemanticFingerprint 재계산
8. Create target의 여전히-부재 / Update target의 여전히-존재 확인
9. entire exact BatchPlanHash 재계산
10. approval evidence와 exact equality 확인
```

하나라도 달라지면 **어떤 target도 mutation하지 않는다.** 대표 code는 `ApprovalStale`이며 identity/path/dirty처럼 더 구체적인 원인이 있으면 해당 diagnostic을 함께 보존한다.

P0 approval lifecycle:

```text
Previewed
→ Reviewed
→ ApplyAttempted
→ Consumed
```

Apply preflight를 실제 요청한 뒤 stale/blocker가 발견돼도 그 approval을 다시 자동 사용하지 않는다. 사용자는 fresh Preview를 받아야 한다. global preflight가 모두 PASS한 뒤 첫 mutation 직전에 approval을 consumed 상태로 고정하고, 이후 write 실패/partial failure에서도 재사용하지 않는다.

자동 retry는 없다.

#### P0-01.11 durable result taxonomy

Preview primary kind는 기존 5종을 유지한다.

```text
Create
Update
NoChange
Conflict
Invalid
```

P0 machine-readable diagnostic code 최소 집합:

```text
MalformedJson
SchemaUnsupported
SchemaRevisionUnsupported
AdapterRevisionMismatch
MissingRequiredField
UnknownField
TypeMismatch
InvalidValue
UnsupportedTextRepresentation
InvalidEnumValue
StableIdentityMissing
StableIdentityMismatch
DuplicateStableIdentity
DuplicateTargetPath
BaselineMissing
BaselineMismatch
BaselineRebaseRequired
UnexpectedExistingTarget
TargetMissing
TargetMoved
PathCollision
TargetDirtyUnowned
ApprovalStale
```

Apply target result:

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

의미:

```text
BlockedBeforeMutation
= global preflight에서 차단되어 target mutation 0

FailedBeforeDurableWrite
= target mutation 단계에서 실패했지만 durable save 호출 전이고
  target-local snapshot + 원래 dirty state 복원이 확인됨

InMemoryStateUnconfirmed
= durable save 전 실패했으나 target-local in-memory rollback/dirty-state 복원을 확정하지 못함
  디스크 durable change를 주장하지 않지만 Editor memory가 오염됐을 수 있으므로 즉시 Stop

DurableApplied
= SavePackage success + package clean + persisted object/package + required typed semantic readback 확인

SaveStateUnconfirmed
= save 호출 이후 disk/package/readback의 최종 상태를 확정하지 못함
  성공/실패/rollback을 추정하지 않고 즉시 Stop

PostCommitWarning
= durable state는 확인됐고 Manager refresh 등 비필수 후처리만 실패
```

Batch aggregate result는 다음을 사용한다.

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

aggregate precedence는 fail-safe로 고정한다.

```text
1. SaveStateUnconfirmed
2. InMemoryStateUnconfirmed
3. PartialApplied
4. FailedBeforeDurableWrite
5. BlockedBeforeMutation
6. PostCommitWarning
7. DurableApplied
8. NoChange
```

따라서 앞 target이 이미 `DurableApplied`였어도 뒤 target이 `SaveStateUnconfirmed`이면 Batch primary result를 단순 `PartialApplied`로 낮추지 않는다. result에는 `DurableAppliedCount`, `NotRunCount`, target별 exact result를 함께 남겨 이미 확정된 durable success는 별도로 보존한다.

`PartialApplied`는 **하나 이상 confirmed DurableApplied가 있고 이후 target이 known failed-before-durable 상태로 끝났으며 unknown state는 없는 경우**에 사용한다.

모든 target은 deterministic order로 처리하고 첫 실패에서 Stop한다. 이미 confirmed durable target은 자동 rollback하지 않으며 나머지는 `NotRun`이다.

Data Asset Manager refresh 실패는 durable write를 되돌리지 않는다. 모든 intended mutation이 durable하게 끝난 뒤 refresh만 실패하면 aggregate `PostCommitWarning`을 사용할 수 있고, 더 강한 failure/uncertainty 결과가 있으면 warning은 side diagnostic으로만 남긴다.

#### P0-01.12 P0-02 구현 진입 조건

P0-01 설계검수에서 최소 다음을 다시 확인한다.

```text
- Staging root가 Git-reviewable이며 UE import/Saved lifecycle과 분리됨
- whole-record schema에 patch ambiguity 없음
- FName/FText/enum/struct/float canonical 의미가 deterministic함
- SchemaRevision / AdapterContractRevision 의미가 겹치지 않음
- Base/Current/Staging fingerprint가 동일 semantic domain을 사용함
- exact 3-way / converged NoChange / Create ownership conflict가 fail-safe함
- Registry coverage prerequisite가 CF-FQ-045 writer 경계를 침범하지 않음
- Vehicle Batch code를 재설계하지 않고 proven semantics만 재사용함
- BatchPlanHash가 exact target set과 approval을 충분히 binding함
- Apply preflight가 Staging과 Unreal current truth를 모두 다시 읽음
- pre-existing dirty target이 mutation 전에 차단됨
- uncertainty result가 PartialApplied보다 우선함
- Product Source mutation 0 / UE Asset mutation 0 상태 유지
```

설계검수 `P0 0 / blocking P1 0` PASS 뒤 exact next는 `DAS-P0-02 Typed Staging Parse + Preview Foundation`이다. P0-02 Source 변경의 첫 prerequisite는 위 `CFMissileGuidePresetData` Registry coverage 28종 보강이다.

#### P0-01.13 Design Review — 2026-09-08

동결안은 current `CF-FQ-045` Registry/Adapter, `CF-FQ-030` Missile Preset Source, `CF-FQ-038` Vehicle Batch safety implementation과 다시 교차검수했다.

최초 P0-01 검수에서 다음 보완점을 발견했다.

```text
P0: 0
P1: 2
P2: 3
Implementation: HOLD until correction
```

P1:

```text
1. top-level StableLogicalId와 whole-record Payload identity field가 불일치할 때 authority가 미정
   → Required identity는 exact typed equality 강제
   → Pilot StableLogicalId != Payload.PresetId = Invalid / StableIdentityMismatch

2. Base != Current == Staging converged NoChange 뒤 old Base를 어떻게 다룰지 미정
   → NoChange + BaselineRebaseRequired
   → Preview mutation 0 유지
   → 다음 편집 전 Current==Staging 재확인 후 explicit rebase 필요
```

P2 projection/terminology:

```text
1. 상위 안전계약의 구명칭 StagingFingerprint
   → StagingSemanticFingerprint로 통일

2. "durable success 뒤 모든 실패 = PartialApplied"라는 과도한 요약
   → known failed-before-durable일 때만 PartialApplied
   → SaveStateUnconfirmed / InMemoryStateUnconfirmed uncertainty가 우선

3. 상위 Apply 결과 목록에 InMemoryStateUnconfirmed 누락
   → current result taxonomy와 정렬
```

교정 후 재검수 결과:

```text
P0: 0
blocking P1: 0
P2: 0
DAS-P0-01: PASS
Product Source mutation: 0
UE Asset mutation/save: 0
Build/Automation: Not Required for document-only design freeze
Next exact Gate: DAS-P0-02 Typed Staging Parse + Preview Foundation
```

최종 판정 근거:

```text
- Staging root가 Git-reviewable main_game Authoring 경로이며 UE Content/Saved lifecycle과 분리됨
- Pilot JSON은 strict whole-record이고 missing/unknown field가 fail-closed임
- FName/FText/enum/struct/bool/float canonical 의미가 deterministic typed contract로 고정됨
- SchemaRevision과 AdapterContractRevision 책임이 분리되고 old approval 재사용이 차단됨
- Base/Current/Staging이 동일 semantic fingerprint domain을 사용함
- converged NoChange를 제외한 unexpected current drift는 overwrite하지 않음
- StableLogicalId와 Payload identity가 양쪽 authority로 갈라지지 않음
- BatchPlanHash가 exact mutation target set, source path, identity/path, B/C/S fingerprint와 operation을 binding함
- Apply가 Staging과 Unreal current truth를 모두 다시 읽는 global TOCTOU를 수행함
- pre-existing dirty target은 mutation 전에 차단됨
- one-shot approval / no automatic retry 의미가 확정됨
- save uncertainty가 PartialApplied보다 강한 terminal 의미로 유지됨
- Vehicle Batch implementation을 재설계하지 않고 proven safety semantics/test matrix만 재사용함
- CFMissileGuidePresetData Registry coverage 27→28은 P0-02 첫 prerequisite로 분리됨
```

### DAS-P0-02 — Typed Staging Parse + Preview Foundation

목적: Editor Asset mutation 없이 Staging을 typed DTO로 parse/validate하고 current target과 비교하는 foundation을 만든다.

포함:

```text
- deterministic parse / canonicalization
- typed whole-record schema validation
- stable diagnostic code
- target identity resolve
- Update BaseSemanticFingerprint required validation
- mutation0 3-way Preview (Base / Current / Staging)
- Create/Update/NoChange/Conflict/Invalid classification
- deterministic per-target proposed payload hash
- exact sorted target set 기반 BatchPlanHash
- Schema/Adapter revision + identity/path + Base/Current fingerprint binding approval
```

Vehicle `FCFBatchImportService`의 Dataset/Recipe/Profile 타입을 일반 DA에 억지로 재사용하지는 않지만, **baseline/current 3-way, exact included set, deterministic hash, approval/current fingerprint 재검증이라는 proven safety semantics는 동일 contract/test matrix로 재사용**한다.

이 단계에서는 `.uasset`을 생성하거나 수정하지 않는다.

#### DAS-P0-02 Implementation / Validation — 2026-09-08

```text
DAS-P0-02: PASS
Registry coverage: 27 → 28
Typed Source Foundation: Implemented
UE Asset Apply/Save: 0
Official Editor Build: PASS
Focused Automation: exact 6 / 6 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Next exact Gate: DAS-P0-03 Exact Materializer + Apply
```

완료 범위:

```text
- CFMissileGuidePresetData를 CF-FQ-045 current Registry의 Required / ExplicitFName(PresetId) descriptor로 편입
- strict whole-record MissileGuidePreset JSON parse와 stable diagnostic
- UE 5.8 shared-key JSON API에 맞춘 deterministic typed canonicalization
- Staging과 current persisted/loaded DataAsset이 동일 semantic domain을 사용하는 SHA-256 fingerprint
- Asset Registry + typed payload 기반 read-only exact target / StableIdentity resolver
- loaded/unsaved target의 package dirty, class, identity와 current fingerprint 관측
- same StableLogicalId duplicate / TargetMoved / PathCollision / dirty fail-closed Preview
- Create / Update / NoChange / Conflict / Invalid mutation0 3-way Preview
- deterministic exact target-set BatchPlanHash
```

검증 증거:

```text
Official Build — Current after Mid-review Correction
- Entry: Tools/BuildEditor.bat
- Build Job: 83747e73cc38407689af72a66faf1905
- Result: Succeeded / Exit Code 0
- Adaptive non-unity compile: CFDAStaging.cpp / CFDAStagingTests.cpp

Focused Automation — Current after Mid-review Correction
- Entry: Tools/RunDAStagingTests.ps1
- Process Job: e984fa6f89b341629b63caefb47ed693
- Execution: exact_list_same_process
- SUCCESS_COUNT=6
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
```

Resolver Automation의 absent case는 실제 MissileFeel Low/Normal/High object의 process-local load 상태에 의존하지 않도록 reserved-never-authored test path/identity를 사용한다. 이 검증을 위해 Product Content Asset을 생성·저장하지 않았다.

P0-02에서는 `SavePackage`, `MarkPackageDirty`, `AssetCreated`, `NewObject`, `CreatePackage`, `Modify`, `SetDirtyFlag` 기반 Staging mutation path를 추가하지 않았으며 **Product UE Asset Apply/Save는 0**이다. P0-03 구현·Apply·Save는 이 checkpoint에서 시작하지 않는다.

#### DAS-P0-02 Mid-review Correction + Re-review — 2026-09-08

최초 중간검수:

```text
P0: 0
P1: 2
P2: 2
P0-03 Entry: HOLD until blocking P1 correction
```

P1 교정:

```text
P1-1 Staging source authority
- repository-relative path의 일반 안전성만 보던 parser를 교정
- `Authoring/DataAssetStaging/` 아래의 canonical lowercase `.json`만 허용
- parse 뒤 mutable StagingRelativePath도 BatchPlanHash 직전 다시 canonicalize/equality 확인

P1-2 Payload ↔ StagingSemanticFingerprint integrity
- Preview가 current typed Payload에서 semantic fingerprint를 다시 계산
- cached StagingSemanticFingerprint와 exact mismatch면 Invalid
- BatchPlanHash도 Create/Update candidate의 Payload↔fingerprint integrity를 defense-in-depth로 다시 확인
```

교정 후 재검수에서 같은 trust-boundary의 인접 우회를 추가 발견해 P1 교정 범위 안에서 함께 닫았다.

```text
- parse 뒤 mutable TargetObjectPath를 바꿔도 Preview에서 canonical Unreal object path를 재검증
- BatchPlanHash가 schema/revision/class, StableLogicalId↔Payload identity,
  TargetObjectPath, canonical StagingRelativePath, Create/Update intent의 Base/Current binding을 재검증
- Preview 이후 forged/mutated candidate가 old approval hash authority로 들어오는 경로를 차단
```

최초 P2 2건은 현재 P0-02 blocker가 아니라 이미 다음 Gate가 소유하는 검증 항목으로 재분류했다.

```text
P2-A loaded-unsaved/unregistered same StableIdentity 탐색 경계
→ 실제 write가 시작되는 DAS-P0-03 global TOCTOU preflight에서 current in-memory + Asset Registry truth를 함께 확정할 책임

P2-B existing persisted Asset positive resolver/readback round-trip
→ DAS-P0-04 Automation Fixture의 Create → Update → persisted readback에서 실제 Asset 기반으로 검증할 책임
```

교정 후 최종 검증:

```text
Official UE 5.8 Editor Build
- Entry: Tools/BuildEditor.bat
- Build Job: 83747e73cc38407689af72a66faf1905
- Result: Succeeded / Exit Code 0

Focused exact Automation
- Entry: Tools/RunDAStagingTests.ps1
- Process Job: e984fa6f89b341629b63caefb47ed693
- Execution: exact_list_same_process
- SUCCESS_COUNT=6
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
```

최종 재검수:

```text
P0: 0
blocking P1: 0
P2: 0
DAS-P0-02: PASS 유지
Product UE Asset Apply/Save: 0
Next exact Gate: DAS-P0-03 Exact Materializer + Apply
```

이 재검수는 P0-03 구현을 시작했다는 뜻이 아니다. 실제 Materializer/Create/Update/Save/approval consumption은 여전히 다음 Gate 소유다.

### DAS-P0-03 — Exact Materializer + Apply

목적: reviewed exact Preview만 Unreal Asset write로 승격하는 Editor-only typed materializer를 구현한다.

포함:

```text
- typed class allowlist/adapter
- exact target package/object create 또는 update
- pre-existing dirty target fail-closed
- Apply 직전 entire target set global TOCTOU preflight
- deterministic target execution order
- target-local transaction/snapshot + dirty handling
- Asset Registry notification for newly created object
- typed post-write validation
- exact target-only SavePackage
- package clean + persisted object/package + 필요한 semantic readback durable confirmation
- first failure Stop
- 하나 이상 confirmed DurableApplied + 이후 known failed-before-durable + uncertainty 0인 경우만 PartialApplied
- SaveStateUnconfirmed / InMemoryStateUnconfirmed은 PartialApplied보다 우선
- no automatic continue/retry/global durable rollback
- no unrelated Save All
- approval consumption + fresh Preview requirement
- machine-readable failure/result taxonomy
```

한 target의 **durable confirmation 전** 실패는 가능한 경우 target-local in-memory snapshot과 원래 dirty state로 복원한다. 그러나 `SavePackage` 호출 뒤 실제 disk 상태를 확인할 수 없는 `SaveStateUnconfirmed`에서는 디스크 변경 가능성이 있으므로 자동 rollback 성공을 주장하지 않고 즉시 중단한다.

이미 `DurableApplied`가 된 이전 target은 뒤 target 실패 때문에 자동 되돌리지 않는다. 다음 수동 시도는 fresh current truth를 기준으로 새 Preview/Approval을 만든다.

범용 arbitrary UObject property writer는 만들지 않는다.

#### DAS-P0-03 Implementation / Validation — 2026-09-08

```text
DAS-P0-03: PASS
Exact Materializer Source: Implemented
Product UE Asset Apply/Save: 0
Persisted Automation Fixture Apply/Save: Not Run — DAS-P0-04 owner
Official UE 5.8 Editor Build: PASS
Focused Automation: exact 8 / 8 PASS
Failure / Missing / Unexpected / Duplicate terminal: 0 / 0 / 0 / 0
Next exact Gate: DAS-P0-04 MissileGuidePreset Pilot
```

구현 범위:

```text
- CFDAStagingApply.h/.cpp를 별도 exact typed materializer owner로 추가
- P0 write allowlist를 CFMissileGuidePresetData 하나로 한정
- Reviewed approval의 Previewed → Reviewed → ApplyAttempted → Consumed one-shot lifecycle 구현
- Schema/Revision/Class/StableIdentity/ObjectPath/Staging path/Base/Current/Staging fingerprint/PlannedOperation/BatchPlanHash exact evidence binding
- Apply 요청 시 canonical Staging JSON을 disk에서 다시 읽고 strict parse/canonicalize
- Asset Registry + loaded-but-unregistered /Game UObject를 합친 StableIdentity current truth
- entire target set current truth / dirty / class / identity / path / Create absence / Update existence / BatchPlanHash global TOCTOU preflight
- global preflight 뒤 각 target mutation 직전 current truth를 한 번 더 재검증해 앞 target save/callback 이후 뒤 target drift도 fail-closed
- deterministic target order + first failure Stop + later target NotRun
- exact CFMissileGuidePresetData CreatePackage/NewObject/AssetCreated/MarkPackageDirty 경로
- Update whole-record typed snapshot + original dirty state 보존
- exact target-only SavePackage / Save All 0
- SavePackage 뒤 package clean + persisted package existence + typed semantic fingerprint readback을 모두 통과한 경우만 DurableApplied
- SaveStateUnconfirmed / InMemoryStateUnconfirmed / PartialApplied fail-safe aggregate taxonomy
- automatic retry / global durable rollback / arbitrary Reflection writer 0
```

P0-02에서 P0-03로 넘긴 loaded-unsaved/unregistered same StableIdentity 공백은 current resolver가 Asset Registry뿐 아니라 process에 load된 `/Game` `CFMissileGuidePresetData`도 합쳐 판정하도록 교정했다. 동일 identity가 다른 path에 load되어 있으면 `TargetMoved` Conflict로 mutation 전에 차단한다.

P0-03 focused Automation은 **Product Content를 저장하지 않는 mutation0 safety/lifecycle fixture**만 사용한다.

```text
- ApprovalLifecycle
  - exact Create Preview → Reviewed approval freeze
  - reviewed Staging source stale/missing → BlockedBeforeMutation / ApprovalStale / mutation 0
  - Apply 요청 뒤 approval Consumed
  - consumed approval automatic retry 차단

- LoadedIdentityPreflight
  - Asset Registry에 등록하지 않은 loaded-only /Game MissileGuidePresetData fixture
  - same StableIdentity를 다른 path에 Create하려는 intent를 TargetMoved Conflict로 차단
  - Product package save 0
```

첫 구현 후 코드 재검수에서는 다음 P1 2건을 발견했다.

```text
P0: 0
P1: 2
P2: 0

P1-1
- entire batch global preflight 뒤 앞 target의 save/callback 등으로 뒤 target current state가 바뀔 수 있으나,
  실제 뒤 target mutation 직전에는 current fingerprint를 다시 확인하지 않았다.

P1-2
- Create 실패 뒤 operation-created Package UObject가 memory에 남을 수 있는데도
  asset rename + dirty clear만으로 original `package absent` 상태를 복원했다고 과대 판정할 수 있었다.
```

교정:

```text
- 모든 target은 실제 mutation 직전에 current resolver + exact 3-way Preview + approval evidence + Create package absence를 다시 확인한다.
- global preflight 이후 drift는 ApprovalStale / BlockedBeforeMutation으로 fail-closed하며,
  앞 target이 이미 DurableApplied라면 frozen aggregate 규칙에 따라 PartialApplied가 될 수 있다.
- Create pre-save rollback은 operation-created package 자체의 absence까지 확인할 수 있어야만 confirmed로 인정한다.
- Package가 남는 상태에서는 FailedBeforeDurableWrite를 과대 주장하지 않고 InMemoryStateUnconfirmed로 보수적으로 승격한다.
- SavePackage 호출 이후 실패는 disk state를 추측하지 않고 SaveStateUnconfirmed로 즉시 중단한다.
```

교정 후 최종 재검수:

```text
P0: 0
blocking P1: 0
P2: 0
DAS-P0-03: PASS
```

최종 검증 증거:

```text
Official UE 5.8 Editor Build
- Entry: Tools/BuildEditor.bat
- Build Job: 3a6592acc9de43738ca4696875cd1132
- Result: Succeeded / Exit Code 0
- Adaptive non-unity compile: CFDAStagingApply.cpp

Focused exact Automation
- Entry: Tools/RunDAStagingTests.ps1 v1.1.0
- Process Job: 2974bdd0d6864743b3e82a1b63620cf5
- Execution: exact_list_same_process
- SUCCESS_COUNT=8
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
```

**P0-03 PASS는 materializer source와 mutation0 safety/lifecycle foundation의 Technical PASS다.** 실제 dedicated test package에서 `Create → Save → Update → Save → persisted readback`, save failure/confirmation failure와 confirmed durable target 뒤 known failure `PartialApplied`를 실행해 증명하는 것은 처음부터 `DAS-P0-04 Automation Fixture`가 소유한다. 따라서 현재 시점에 실제 durable Asset write가 검증됐다고 확대하지 않는다.

Product Low / Normal / High `.uasset`은 이번 Gate에서 수정·저장하지 않았으며 **Product UE Asset Apply/Save는 0**이다.

#### DAS-P0-03 Post-PASS Mid-review Correction + Re-review — 2026-09-08

P0-03 PASS checkpoint를 실제 write 진입 직전에 다시 코드 기준으로 검수했다.

최초 중간검수:

```text
P0: 0
P1: 2
P2: 2
DAS-P0-04 Entry: HOLD
```

교정 내용:

```text
P1-1 DurableApplied persisted evidence
- 기존 SavePackage → clean → package exists → same in-memory UObject fingerprint만으로 DurableApplied를 확정하던 경로를 교정
- exact SavePackage success 뒤 `UPackageTools::ReloadPackages(..., EReloadPackagesInteractionMode::AssumePositive)`로 해당 package 하나를 non-interactive disk reload
- save 전 동결한 exact object path로 reloaded CFMissileGuidePresetData를 다시 resolve/load
- reloaded package identity / clean / persisted existence를 다시 확인
- reloaded UObject에서 unified typed semantic extractor + fingerprint를 재계산해 StagingSemanticFingerprint와 exact 일치할 때만 DurableApplied
- SavePackage 호출 뒤 reload/readback을 확정하지 못하면 SaveStateUnconfirmed로 fail-closed

P1-2 Current projection
- ActiveWork / FeatureQueue를 representative Plan v0.5.1과 `DAS-P0-04 MissileGuidePreset Pilot` next gate로 동기화

P2-1 PartialApplied 표현
- `durable success 뒤 모든 실패`라는 과도한 요약을 제거
- confirmed DurableApplied >= 1 + 이후 known failed-before-durable + uncertainty 0일 때만 PartialApplied
- SaveStateUnconfirmed / InMemoryStateUnconfirmed precedence를 명시

P2-2 UObject → typed semantic authority
- current resolver가 사용하던 FTextInspector 기반 lossless extractor를 `FCFDAStagingService::ExtractMissilePresetPayload` public semantic contract로 승격
- current fingerprint / Update rollback snapshot / pre-save readback / post-save persisted readback이 모두 같은 extractor를 재사용
- Apply 전용 `.ToString()` flattening extractor는 제거
```

교정 후 검증:

```text
Official UE 5.8 Editor Build
- Entry: Tools/BuildEditor.bat
- Build Job: fdd50db886b04387b8aa72c83178310b
- Result: Succeeded / Exit Code 0
- corrected CFDAStaging.cpp / CFDAStagingApply.cpp / CFDAStagingApplyTests.cpp compile PASS
- deprecated bool ReloadPackages overload 제거 후 current EReloadPackagesInteractionMode overload 사용

Focused exact Automation
- Entry: Tools/RunDAStagingTests.ps1 v1.1.0
- Process Job: 45c42b68974341e7a45e0105d230a519
- Execution: exact_list_same_process
- SUCCESS_COUNT=8
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
```

교정 후 재검수:

```text
P0: 0
blocking P1: 0
P2: 0
DAS-P0-03: PASS 유지
DAS-P0-04 Entry: READY
Product UE Asset Apply/Save: 0
```

중요 검증 경계:

```text
- exact 8종 Automation은 P0-02 foundation + P0-03 mutation0 approval/current-truth safety 회귀를 검증한다.
- SavePackage → package disk reload → persisted semantic fingerprint 경로는 이번 교정에서 official UE 5.8 compile validation까지 완료했다.
- 이 durable path를 실제 전용 test package에서 실행하는 Create → Save → Update → Save → persisted readback과 save/confirmation failure/PartialApplied fixture는 계획대로 DAS-P0-04 Automation Fixture가 소유한다.
- 따라서 Product `.uasset`이 실제 수정·저장됐다고 확대하지 않는다.
```

### DAS-P0-04 — MissileGuidePreset Pilot

목적: 완료된 CF-FQ-030의 Runtime 동작을 재오픈하지 않고 `CFMissileGuidePresetData`만 첫 실제 Pilot으로 사용한다.

Pilot은 Product와 Automation Fixture를 분리한다.

```text
Product Pilot
1. 기존 Low / Normal / High를 whole-record JSON Staging schema로 표현할 수 있다.
2. Preview에서 current persisted Preset과 semantic NoChange를 정확히 판정한다.
3. 의도된 Update Preview는 가능하지만 USER 승인 없는 Product `.uasset` mutation은 수행하지 않는다.
4. 기존 `.uasset` direct edit 또는 dirty package가 있으면 stale/dirty blocker가 mutation 전에 잡힌다.
5. 기존 Missile Runtime/DirectTest baseline/USER Feel 결과는 바뀌지 않는다.

Automation Fixture
1. Product Content와 분리된 전용 test package/root에서 신규 Preset Create를 검증한다.
2. 같은 fixture를 Update하고 durable persisted readback을 검증한다.
3. Base 이후 current drift를 주입해 Conflict / mutation 0을 검증한다.
4. pre-existing dirty target을 주입해 TargetDirtyUnowned / mutation 0을 검증한다.
5. Save failure/confirmation failure와 PartialApplied terminal semantics를 검증한다.
6. test-owned fixture cleanup은 Product Asset delete 기능이 아니라 Automation lifecycle 예외로 한정한다.
```

P0 Acceptance를 위해 Product Content에 테스트용 신규 Preset을 영구히 남길 필요는 없다.

기존 `EnsureMissileFeelPresetAsset` 계열 경로는 Pilot 검증 전 삭제하거나 무력화하지 않으며, generic writer owner로 승격하지 않고 current idempotence/reference evidence로만 사용한다.

#### DAS-P0-04 결과 — PASS / 2026-09-08

Product Pilot은 실제 Low / Normal / High persisted DataAsset을 read-only typed payload로 추출해 각 target에 대해 현재 payload 그대로의 whole-record Staging은 `NoChange`, `NavigationConstant`만 안전 범위에서 바꾼 의도 payload는 `Update`로 분류됨을 확인했다. Preview 전후 semantic fingerprint와 package dirty=false를 재확인했으며 Product target에 `BuildReviewedApproval`/`ApplyReviewedBatch`를 호출하거나 `.uasset`을 저장하지 않았다.

persisted Product의 FText는 AssetDump에서 `NSLOCTEXT("", generated-key, source)` 형태임을 확인했다. P0 Literal semantic extractor는 StringTable/source-less/실제 authored namespace는 계속 fail-closed하면서, source-backed empty/package-only authored namespace의 generated stable key만 persistence metadata로 제외하도록 교정했다.

Automation Fixture는 Product와 분리된 `/Game/Test/CarFight/DAStagingP04`에서 다음을 실제 검증했다.

```text
Create
→ exact SavePackage
→ disk reload
→ re-resolve
→ typed persisted semantic readback
→ reviewed Staging fingerprint exact match

Update
→ exact SavePackage
→ disk reload
→ re-resolve
→ typed persisted semantic readback
→ reviewed Staging fingerprint exact match

baseline drift
→ Conflict / BaselineMismatch / mutation 0

pre-existing dirty
→ Conflict / TargetDirtyUnowned / mutation 0

SavePackage outcome uncertainty
→ SaveStateUnconfirmed / DurableAppliedCount 0

post-save confirmation uncertainty
→ SaveStateUnconfirmed / DurableAppliedCount 0

first target DurableApplied + second target known pre-mutation block
→ PartialApplied / DurableAppliedCount 1
```

첫 13종 PASS 뒤 후검증에서 test-owned `.uasset` 6개 residue를 발견했으므로 당시 결과만으로 P0-04를 승격하지 않았다. 원인은 fixture teardown이 loaded package 상태와 physical delete 결과를 충분히 닫지 못한 것이었다. 이를 다음처럼 교정했다.

```text
Automation teardown
AssetRegistry delete notification
→ package unload
→ CollectGarbage
→ exact test-owned Content/Staging root recursive delete
→ directory/file residue 0 assertion
→ cleanup failure 자체를 Automation failure로 승격

Runner gate
Tools/ClearDAStagingP04.ps1 exact pre-clean
→ exact 13종
→ Content/Staging root 존재 시 process 전체 실패
```

최종 fresh evidence:

```text
기존 residue pre-clean:
- Tools/ClearDAStagingP04.ps1
- Process Job: 4858c94ed91d4a94bda74d237ec2e20f
- Exit 0 / DAS_P0_04_FIXTURE_RESIDUE_COUNT=0

Official UE 5.8 Editor Build:
- Build Job: 2b1b77739d6f4541b38720b4a11b9199
- Exit 0 / PASS

Focused exact-list Automation:
- Process Job: 95039bc3c6e04d6e83631ec7d773e093
- exact 13/13 PASS
- failure 0 / missing 0 / unexpected 0 / duplicate-terminal 0
- runner post-residue gate PASS

Git post-check:
- UE/Content/Test/CarFight/DAStagingP04 항목 0
- Product Low/Normal/High .uasset dirty 0

Fresh AssetDump:
- exact deleted fixture root 직접 preparation은 missing-root 상태에서 fail-closed했으므로 PASS evidence로 사용하지 않음
- parent /Game/Test/CarFight + class CFMissileGuidePresetData fresh dataset은 asset_count=3
- exact 3개는 MissileFeel_Low / MissileFeel_Normal / MissileFeel_High Product뿐임
- 따라서 /Game/Test/CarFight/DAStagingP04 persisted fixture residue=0
```

**DAS-P0-04 판정: PASS. Product Low/Normal/High Apply/Save 0. USER 승인 없는 Product save 금지 유지.**

### DAS-P0-05 — Acceptance / Current System Promotion

P0 전체를 다음 기준으로 마감한다.

```text
- Editor-off Staging 작성 경로 확인
- Preview classification 기술 검증
- Conflict / stale protection 검증
- exact batch apply 검증
- persisted AssetDump 또는 동등한 저장 증거 확인
- official UE 5.8 Editor build PASS
- focused/affected Automation PASS
- unrelated Asset/Package save 0 확인
- 대표 USER workflow 확인이 필요한 UI가 있다면 USER Acceptance 분리
- Current Systems owner 승격
```

P0 완료 뒤 Ammo/Damage/Sensor 확장은 자동 착수하지 않고 별도 확장 판단을 수행한다.

#### DAS-P0-05 Acceptance / Promotion 계약검수 — HOLD / 2026-09-08

current Plan·Systems·Source·runner·Git 상태를 read-only로 교차검수했다. DAS-P0-04의 Build/exact13/AssetDump evidence는 새 Source failure가 없으므로 보존했고 Product Low/Normal/High Apply·Save는 0을 유지했다.

판정은 다음과 같다.

```text
P0: 0
P1: 2
P2: 0
Promotion: HOLD
Current Systems promotion: Not Started
Product Low/Normal/High Apply/Save: 0
Current Active CF-FQ-039: unchanged
```

P1-1 — current normative schema projection이 AdapterContractRevision 1로 stale하다.

- current Source authority는 `CFDAStaging.cpp`의 `MissilePresetAdapterRevision = 2`다.
- 하지만 현재 normative P0-01 schema block과 JSON 예시는 아직 `AdapterContractRevision = 1` / `"AdapterContractRevision": 1`을 지시한다.
- v0.3.0 Changelog의 revision 1 기록은 Historical로 보존할 수 있지만, 현재 schema/example이 old revision을 새 Staging 작성 지침처럼 노출하면 current parser가 `AdapterRevisionMismatch`로 거부한다.
- Promotion 전에 현재 normative schema/example만 revision 2로 교정하고 Historical 기록은 당시 상태로 남겨야 한다.

P1-2 — P0-05 acceptance 항목인 Editor-off Staging 작성/운영 경로가 아직 실제 Current workflow로 성립하지 않았다.

- canonical root `Authoring/DataAssetStaging/`는 설계돼 있지만 fresh repository search 기준 Product `MissileGuidePreset/*.json`은 0건이다.
- P0-04 ProductPreview는 canonical path 문자열만 사용해 JSON을 메모리에서 조립하며 Product Staging 파일은 저장하지 않는다.
- non-test Source에서 `BuildReviewedApproval` / `ApplyReviewedBatch`의 caller는 없고 service 정의만 존재한다. 따라서 현재 사용자가 실제로 `여러 Staging 발견/선택 → Preview → 검토 → explicit Apply`를 재사용 가능한 P0 운영 진입으로 실행할 수 없다.
- 특히 기존 Product Update는 mandatory `BaseSemanticFingerprint`와 후속 explicit baseline rebase가 필요하므로, 현재처럼 test helper에만 JSON serialization이 있으면 Editor-off 편집을 시작하거나 다음 편집을 위해 baseline을 갱신하는 실제 운영 절차가 닫히지 않는다.
- 교정은 새 대형 UI를 강제하지 않는다. 최소한 canonical Product staging bootstrap/rebase와 discovery→Preview→reviewed Apply를 연결하는 bounded reusable Editor-side entry를 하나 확정하고, Product Save 없이 mutation0 workflow를 먼저 acceptance해야 한다.

P0-05 재검수 통과 조건은 다음으로 고정한다.

```text
1. current normative AdapterContractRevision 2 schema/example 정렬
2. Editor-off canonical Product Staging bootstrap/rebase 경로 존재
3. reusable discovery/select → Preview → explicit reviewed Apply orchestration entry 존재
4. Product Low/Normal/High는 mutation0 NoChange/Update Preview까지만 acceptance하고 USER 승인 없는 Product Save 0 유지
5. existing exact13 + official Build evidence는 Source가 바뀌면 fresh 재실행, 문서/텍스트만 바뀌면 불필요 반복 금지
6. 통과 뒤에만 Systems/DataManagement/DataAssetAuthoring.md 신규 Current owner와 SystemIndex/FeatureQueue/ActiveWork/Plan projection 승격
7. CF-FQ-045 DataAssetManagement read-first owner와 CF-FQ-039 Active lifecycle 불변
```

#### DAS-P0-05 Contract Review Correction + Re-review — PASS / 2026-09-09

v0.6.2 계약검수의 P1 2건을 최소 범위로 교정한 뒤 fresh Source/Build/Automation evidence와 코드 재검수로 다시 판정했다. **Current System Promotion은 이 재검수 안에서 수행하지 않았고 Product Low/Normal/High UE Asset Apply·Save는 0을 유지했다.**

P1-1 교정:

- current normative schema block을 `AdapterContractRevision = 2`로 정렬했다.
- current whole-record JSON example도 `"AdapterContractRevision": 2`로 정렬했다.
- revision 1은 v0.3.0 당시 Historical contract와 old-Staging fail-closed evidence로만 보존한다. current authoring instruction은 revision 2다.

P1-2 교정:

- canonical Product Staging source 3종을 실제 repository path에 bootstrap했다.
  - `Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json`
  - `Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Normal.json`
  - `Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_High.json`
- `CFDAStagingOps.h/.cpp`를 추가해 Product Staging bootstrap/rebase와 deterministic discovery/Preview를 기존 typed Staging safety core 위에 얇게 연결했다.
- Product Sync는 현재 Product package clean + exact identity + typed fingerprint를 먼저 확인한다. 기존 JSON이 있으면 `NoChange`로 current Product와 이미 수렴한 경우에만 canonical baseline rebase/rewrite를 허용하고 `Update`/`Conflict`/`Invalid`/drift가 있으면 사용자 편집 보호를 위해 자동 덮어쓰지 않는다.
- canonical JSON 생성 뒤에는 Editor가 꺼져 있어도 Git-reviewable JSON을 직접 편집할 수 있다. 단, **current Product truth를 읽어 최초 baseline을 만들거나 명시적 rebase를 수행하는 Sync 자체는 running Editor의 Editor-side service 작업**이다.
- reusable P0 운영 entry는 다음 exact 4단계 console surface로 고정했다.

```text
CarFight.DAStaging.SyncProduct
→ Product read-only current baseline을 canonical JSON에 bootstrap/rebase
→ Product UE Asset Save 0

CarFight.DAStaging.Preview
→ canonical folder discovery
→ strict parse / current resolve / Create·Update·NoChange·Conflict·Invalid
→ blocker와 exact BatchPlanHash 표시

CarFight.DAStaging.Review
→ 직전 Preview 뒤 fresh discovery를 다시 실행
→ fresh BatchPlanHash가 직전 hash와 exact 동일할 때만 BuildReviewedApproval
→ UE Asset Save 0

CarFight.DAStaging.ApplyReviewed
→ 실제 Reviewed one-shot approval이 있을 때만 기존 FCFDAStagingApplyService::ApplyReviewedBatch 호출
→ 이 P0-05 Correction/Acceptance checkpoint에서는 Product에 실행하지 않음
```

운영 surface는 새 대형 Manager UI나 generic writer를 만들지 않는다. persistence safety owner는 기존 `FCFDAStagingApplyService`이며 `SavePackage` 실제 구현도 그 service에만 남는다. 새 `CFDAStagingOps`에는 `MarkPackageDirty`, `NewObject`, `CreatePackage`, generic reflection write를 추가하지 않았다.

검증 evidence:

```text
Official UE 5.8 Editor Build
- Job: 6196bcf0c631413093dfe1d871125f60
- Result: Succeeded / Exit Code 0
- fresh source add를 non-unity compile/link

DAS-P0-05 OperationalEntry exact 1
- Process: 111adc5a0013413ca1e9f4426e618124
- SUCCESS_COUNT=1
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
- Product canonical JSON 3종 revision 2 strict parse
- second Sync changed path 0 idempotence
- discovery exact 3 rows = NoChange 3 / Create 0 / Update 0 / Conflict 0 / Invalid 0
- Product package clean + semantic fingerprint unchanged
- Product ApplyReviewed 실행 0

Existing DAS exact 13 regression
- Process: d2c15a9070b34583a3080b4071c50daa
- SUCCESS_COUNT=13
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
- pre/post DAS-P0-04 test fixture residue 0
```

재검수 판정:

```text
P0: 0
P1: 0
P2: 0
DAS-P0-05 Contract Review Correction + Re-review: PASS
Product Low/Normal/High UE Asset Apply/Save: 0
Current System Promotion: Not Started
Current Active CF-FQ-039: unchanged
Next exact Gate: DAS-P0-05 Acceptance / Current System Promotion
```

#### DAS-P0-05 Post-Correction Mid-review Correction + Re-review — PASS / 2026-09-09

Contract Review Correction PASS 뒤 실제 운영 흐름을 다시 검수한 결과는 최초 `P0 0 / P1 3 / P2 0`이었다. 세 P1을 CF-FQ-049 운영 계층 안에서만 최소 교정하고 fresh Build/Automation/회귀로 다시 검증했다. **Product Low/Normal/High UE Asset Apply·Save는 계속 0이며 Current System Promotion은 아직 시작하지 않았다.**

P1-1 — whole-folder Preview가 exact 사용자 선택 범위를 소유하지 못했다.

- 기존 `CarFight.DAStaging.Preview`는 canonical folder 전체를 discovery해, 사용자가 이번 Batch에서 선택하지 않은 다른 JSON의 `Invalid`/`Conflict`도 Review를 막을 수 있었다.
- `FCFDAStagingOps::DiscoverMissilePresetPreview`에 canonical repository-relative JSON exact-list selection을 추가했다. empty selection만 전체 discovery를 의미한다.
- console Preview는 `StableLogicalId` 또는 canonical relative JSON path 인수를 exact selection으로 해석한다.
- Automation에서 selected Low Update 1건은 unselected Normal Invalid가 존재해도 blocker 없이 Preview되고, Normal을 별도 선택하면 정확히 Invalid 1건으로 분리되는 것을 검증했다.

P1-2 — Preview→Review가 실제 same-selection reusable state machine으로 증명되지 않았다.

- `FCFDAStagingOpsSession`을 추가해 `Preview(selection) → fresh same-selection Review → explicit ApplyReviewed` 순서를 한 운영 세션이 소유하게 했다.
- Review는 Preview 때 동결한 동일 selection으로 fresh discovery를 다시 수행하고, fresh `BatchPlanHash`가 직전 Preview hash와 exact 동일할 때만 `BuildReviewedApproval`을 호출한다.
- Preview 뒤 selected JSON이 바뀌면 stale Review를 거절하고 approval을 만들지 않는다.
- fresh Preview 뒤 Review가 PASS하면 Reviewed approval의 IncludedTargets가 exact selected target만 포함하는 것을 Automation으로 검증했다.
- P0-05 OperationalEntry는 안전 계약을 검증하기 위해 `ApplyReviewed`를 Product에 실행하지 않는다. 실제 materialize/save authority는 기존 `FCFDAStagingApplyService`만 유지한다.

P1-3 — `SyncProduct`가 여러 Product JSON을 순차 rewrite하다 후반 write/readback에서 실패하면 앞선 JSON만 변경된 채 남을 수 있었다.

- Sync preflight에서 각 대상 Staging file의 호출 전 존재 여부와 raw bytes를 snapshot한다.
- 실제 write를 수행한 target만 touched plan으로 기록하고, 어느 단계에서든 write/readback/Product semantic verification이 실패하면 touched file을 역순으로 호출 전 raw bytes 또는 absence 상태로 복원한다.
- rollback 뒤 raw-byte exact readback까지 확인하며, 복원을 확인하지 못하면 success나 rollback-confirmed를 주장하지 않는다.
- test-only fault injection으로 두 번째 changed-file write 성공 직후 failure를 강제했고, 앞서 touched된 모든 Product Staging file이 pre-call raw bytes와 exact 동일하게 복원되는 것을 검증했다.
- `Tools/RunDAStagingOpsTest.ps1`은 Product Low/Normal/High canonical Staging 3종의 pre-run raw bytes를 snapshot하고 post-run exact 동일성을 별도 residue gate로 확인한다. mismatch가 있으면 원본을 복원한 뒤 runner 자체를 실패시킨다.

최종 검증 evidence:

```text
Official UE 5.8 Editor Build
- Job: 7804b91cfac542fd9aee78af0f2d692f
- Result: Succeeded / Exit Code 0

DAS-P0-05 OperationalEntry exact 1
- Process: 9b3f76b7a78b4f3cb6f30e11ab140327
- SUCCESS_COUNT=1
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
- DAS_P0_05_PRODUCT_STAGING_RESIDUE_COUNT=0
- selected Update / unselected Invalid isolation PASS
- stale same-selection Review reject PASS
- fresh Review exact selected approval PASS
- forced partial-write rollback raw-byte exact restore PASS
- Product ApplyReviewed 실행 0

Existing DAS exact 13 regression
- Process: 49e54542dd3f42f9a3eee2c10b09e5ed
- SUCCESS_COUNT=13
- FAILURE_COUNT=0
- MISSING_COUNT=0
- UNEXPECTED_COUNT=0
- DUPLICATE_TERMINAL_COUNT=0
```

재검수 판정:

```text
P0: 0
P1: 0
P2: 0
DAS-P0-05 Post-Correction Mid-review Correction + Re-review: PASS
Product Low/Normal/High UE Asset Apply/Save: 0
Current System Promotion: Not Started
Current Active CF-FQ-039: unchanged
Next exact Gate: DAS-P0-05 Acceptance / Current System Promotion
```

#### DAS-P0-05 Final Acceptance / Current System Promotion — PASS / 2026-09-09

v0.6.4 Post-Correction Mid-review Re-review PASS를 기준으로 current Source, canonical Product Staging, 기존 Build/Automation evidence와 fresh persisted 상태를 다시 감사했다. 이번 Gate는 새로운 Product write를 수행하는 단계가 아니라 P0 완료 정의와 Current Knowledge 승격 가능성을 판정하는 closure Gate다.

Final Acceptance checklist:

```text
Editor-off canonical Staging 작성 경로: PASS
Typed strict parse / Preview classification: PASS
Conflict / stale / dirty fail-closed: PASS
exact selected Batch Review / approval binding: PASS
test-owned durable Create/Update Apply: PASS
SavePackage → disk reload → typed semantic readback: PASS
save uncertainty / PartialApplied taxonomy: PASS
Official UE 5.8 Editor Build: PASS
OperationalEntry exact 1/1: PASS
Existing DAS exact 13/13: PASS
Product Staging residue: 0
unrelated package Save / Save All: 0
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Apply·Save: 0
별도 USER Visual/Feel Acceptance 필요: 없음 — dedicated visual UI가 없는 technical console workflow
```

재사용한 최종 Source validation evidence:

```text
Official UE 5.8 Editor Build
- Job: 7804b91cfac542fd9aee78af0f2d692f
- Result: Succeeded / Exit Code 0

OperationalEntry
- Process: 9b3f76b7a78b4f3cb6f30e11ab140327
- exact 1/1 PASS
- Product Staging residue 0
- Product ApplyReviewed 실행 0

Existing DAS regression
- Process: 49e54542dd3f42f9a3eee2c10b09e5ed
- exact 13/13 PASS
```

Promotion 직전 fresh persisted audit:

```text
AssetDump root: /Game/Test/CarFight
Class: CFMissileGuidePresetData
Asset count: 3
Succeeded: 3
Failed: 0
Fresh preparation: true
```

이번 Acceptance audit에서 Source를 변경하지 않았으므로 official Build/Automation은 불필요하게 반복하지 않았다. fresh AssetDump는 persisted Product scope에 P0-04 fixture residue가 다시 나타나는지 확인하는 독립 acceptance evidence로 사용했다.

Final Acceptance 판정:

```text
P0: 0
P1: 0
P2: 0
DAS-P0-05 Final Acceptance: PASS
CF-FQ-049 P0: Complete
```

Current System Promotion:

```text
G0 Evidence: PASS
G1 Current Knowledge Promotion: PASS
- Document/Systems/DataManagement/DataAssetAuthoring.md v1.0.0
- Document/Systems/SystemIndex.md v1.33.0

G2 Current Route Cleanup: PASS
- FeatureQueue Ready → Done
- ActiveWork Ready row 제거
- Plan Index Ready row 제거

G3 Reference Preservation: PASS
- DataAssetStaging/DataAssetStagingPlan.md v0.7.0 retained

G4 Historical: PASS
- Done → Historical + Retained Path

G5 Optional Physical Move: Deferred
- plan_repo 기존 dirty와 현재 retained reference를 보호하고 별도 maintenance 전에는 이동하지 않음
```

Product Low/Normal/High `.uasset`은 이 closure에서 materialize/save하지 않았다. 현재 단일 Active `CF-FQ-039`도 그대로 유지한다. Ammo/Damage/Sensor 등 후속 타입 확장은 자동 시작하지 않고 새 lifecycle에서 판단한다.

---

## 10. 구현 구조 원칙

구체적인 클래스/파일명은 DAS-P0-00~01에서 current Source와 충돌 여부를 확인한 뒤 확정한다.

현재 예상되는 책임 분리는 다음과 같다.

```text
Editor-independent-ish typed value/schema layer
        ↓
Staging Parser / Validator
        ↓
Preview / Conflict classifier
        ↓
Editor-only Typed Materializer
        ↓
Unreal DataAsset
```

Data Asset Manager는 결과 Asset을 기존 방식대로 재발견·검사할 수 있어야 하지만 write service를 소유하지 않는다. `CFMissileGuidePresetData`처럼 Manager Registry 작성 이후 새로 생긴 concrete DA의 semantic descriptor coverage 보강은 허용하되 writer/save authority는 추가하지 않는다.

Vehicle Builder와 Vehicle Data Authoring의 Recipe/Profile/Resolver/Apply engine을 CF-FQ-049에 흡수하지 않는다.

P0 safety core는 C++가 소유한다.

```text
C++
- typed schema / canonicalization
- identity / fingerprint / Preview
- exact approval / preflight
- typed materializer / SavePackage / durable confirmation
- machine-readable result

Slate/Editor UI 또는 후속 BP-facing surface
- Staging 목록 표시
- Preview 표시
- 명시적 Apply 요청
- 결과/경고 표시
```

Blueprint나 generic Reflection write를 persistence safety owner로 사용하지 않는다.

---

## 11. 주요 위험과 방어

| 위험 | P0 방어 방향 |
| --- | --- |
| JSON/CSV가 사실상 제2 SSOT가 됨 | Apply 완료 후 `.uasset` 최종 authority 유지 |
| Staging이 사용자 UAsset 수정값 덮어씀 | baseline/current/staging conflict 검출 |
| 모든 DA를 Reflection으로 임의 수정 | Typed Adapter / allowlist |
| Schema 변경으로 old Draft 의미가 바뀜 | SchemaId + Revision + migration/reject |
| Staging 파일 삭제가 Asset 삭제로 연결 | P0 delete 금지 |
| Apply 한 번이 unrelated Package를 저장 | exact target package scope |
| Editor 자동기동이 다시 남발됨 | Staging write와 materialization lifecycle 분리 |
| DA Manager가 writer까지 소유해 비대화 | CF-FQ-045 read-first owner 유지 |
| Vehicle Authoring과 중복 | CF-FQ-038/Vehicle Builder authority 제외 |
| Pilot 때문에 Missile Runtime 재검증 폭증 | CF-FQ-030 Runtime/USER Feel 완료 evidence 보호 |

---

## 12. P0 Scope Out

다음은 P0에 포함하지 않는다.

```text
- 모든 CarFight DataAsset 자동 지원
- VehicleData / Vehicle Recipe / Vehicle Profile 일반 Staging 전환
- Runtime 또는 Packaged Game에서 `.uasset` 저작
- Staging 파일을 Runtime Source로 읽기
- `.uasset` raw binary 직접 생성
- arbitrary UObject reflection writer
- Asset 자동 삭제
- 모든 DataAsset을 JSON SSOT로 전환
- Excel `.xlsx` 자체를 canonical storage로 사용
- XML pipeline
- ProjectileData/CombatFxData의 UE reference-heavy 완전 외부 저작
- CF-FQ-045 Data Asset Manager 전면 재설계
- CF-FQ-030 Missile Guidance Runtime/Feel 재설계
```

---

## 13. Current System owner

CF-FQ-049 P0 완료 후 현재 구현 계약은 다음 문서로 승격됐다.

```text
Document/Systems/DataManagement/DataAssetAuthoring.md v1.0.0
```

현재 구현 판단은 이 Systems 문서와 실제 `UE/Source/CarFight_ReEditor/DataAuthoring` Source를 우선한다.

`DataAssetManagement.md`는 기존 read-first Manager current owner로 유지하며 Staging write/apply authority를 흡수하지 않는다.

---

## 14. 완료 시 보호 범위

Current System Promotion은 문서 lifecycle 승격으로 수행했고 새로운 Product materialization을 요구하지 않았다.

```text
Product Low/Normal/High ApplyReviewed: 0
Product Low/Normal/High UE Asset Apply·Save: 0
unrelated package Save / Save All: 0
CF-FQ-030 Runtime/Guidance mutation: 0
CF-FQ-045 재오픈: 0
CF-FQ-038 재오픈: 0
CF-FQ-039 Active 변경: 0
```

main_game의 unrelated `SourceArt/UI/HUD/VehiclePanel/` 및 다른 병렬 dirty는 보호한다.

---

## 15. Historical — Initial Design Review — 2026-09-08

### 15.1 판정

현재 v0.1.0 방향은 유지 가능하지만 구현에 바로 들어갈 수는 없다.

```text
P0: 1
P1: 8
P2: 3
Implementation: HOLD
```

검수는 대표 Plan 문구뿐 아니라 current `CF-FQ-045` Data Asset Manager, `CF-FQ-038` Vehicle Data Authoring Batch, `CF-FQ-030` MissileGuidePreset authoring과 실제 `CarFight_ReEditor` Source를 교차대조했다.

현재 Source에는 이미 Vehicle Data Authoring의 `3-way Preview → exact approval → current fingerprint 재검증 → global TOCTOU preflight → typed transaction/rollback` 계약과 Vehicle Builder의 `SavePackage success → package clean + persisted confirmation → refresh warning 분리` 계약이 존재한다. CF-FQ-049는 이 검증된 안전 의미를 재사용하거나 공통 primitive로 추출해야 하며 별도의 느슨한 유사 구현을 만들지 않는다.

### 15.2 P0 — 기존 target package의 미저장 dirty ownership 누락

현재 Plan은 persisted `.uasset`의 외부 변경과 unrelated `Save All`은 보호하지만 **Editor에 이미 load된 exact target package가 CF-FQ-049 실행 전에 dirty인 경우**를 정의하지 않는다.

이 상태에서 Update materializer가 같은 package를 수정한 뒤 `SavePackage`하면 사용자가 다른 Editor 작업으로 아직 저장하지 않았던 변경까지 함께 영구 저장할 수 있다.

P0 기본 계약은 다음으로 교정한다.

```text
Existing target package가 Apply 시작 전에 dirty
→ TargetDirtyUnowned 계열 blocker
→ mutation 0
→ CF-FQ-049가 자동 저장/병합/정리하지 않음

이번 operation이 신규 생성한 package
→ operation-owned dirty로 허용
```

P0에서 pre-existing dirty package를 merge하는 고급 ownership 모델은 만들지 않는다.

### 15.3 P1-1 — Update baseline fingerprint가 optional로 표현됨

현재 §8의 `필요 시 baseline/current fingerprint`는 Update conflict protection에 충분하지 않다.

Update row에는 최소 `BaseSemanticFingerprint`가 필수여야 한다. Create는 baseline이 없을 수 있지만 Update는 staging 작성 당시의 persisted semantic baseline과 current persisted/loaded truth를 비교할 수 있어야 한다.

```text
Create
→ BaseSemanticFingerprint 없음 허용

Update
→ BaseSemanticFingerprint 필수
→ Base == Current이면 Update/NoChange 판정 가능
→ 당시 Initial Review에서는 Base != Current를 보수적으로 Conflict로 요구
→ Current P0-01.7에서는 `Current == Staging`으로 이미 수렴한 경우만 mutation 0 `NoChange + BaselineRebaseRequired`로 정교화하고, 그 외 Base != Current는 Conflict 유지
```

### 15.4 P1-2 — exact approval binding / TOCTOU 계약이 너무 느슨함

현재 `deterministic preview hash 또는 equivalent approval binding evidence`는 구현 선택 폭이 너무 넓다.

P0 approval은 최소 다음을 exact binding해야 한다.

```text
SchemaId + SchemaRevision
StagingFingerprint
Target Type / Logical Identity / Object Path
CurrentSemanticFingerprint
BaseSemanticFingerprint(해당 시)
Planned Operation(Create/Update)
정렬된 exact target set
BatchPlanHash
```

Approval 이후 Apply 직전 current truth를 다시 읽어 하나라도 달라졌으면 mutation 전에 stale로 차단한다. 한 건이라도 실제 mutation이 성공한 뒤에는 기존 approval을 소비하고, 수동 retry도 fresh Preview부터 다시 시작한다.

### 15.5 P1-3 — durable multi-package Batch partial failure 의미 누락

여러 `.uasset`을 순차 저장하는 Batch는 첫 package가 이미 durable 저장된 뒤 두 번째 package에서 실패할 수 있으므로 파일 시스템 수준의 전체 atomic rollback을 약속할 수 없다.

P0는 다음 의미를 명시해야 한다.

```text
Apply 전 전체 global preflight
→ deterministic target order
→ target별 typed mutation + durable save confirmation
→ 첫 실패에서 Stop
→ 이미 durable 성공한 target은 되돌리지 않음
→ 나머지는 미실행
→ Batch 결과 = PartialApplied 계열
→ 자동 continue/retry/rollback 없음
→ 다음 실행은 fresh Preview
```

메모리상 mutation 단계의 rollback과 durable disk save 이후 rollback을 같은 의미로 취급하지 않는다.

### 15.6 P1-4 — SavePackage 성공과 durable success가 구분되지 않음

현재 Vehicle Builder Current 계약은 raw `SavePackage=true`만으로 저장 성공을 인정하지 않는다.

CF-FQ-049도 target별로 최소 다음을 확인해야 한다.

```text
SavePackage success
+ package clean
+ persisted package/object 존재 확인
+ 필요 시 persisted semantic readback
= DurableApplied
```

raw save는 성공했지만 clean/persisted 상태를 확인하지 못하면 `SaveStateUnconfirmed` 계열로 fail-closed한다. Durable save 뒤 Data Asset Manager refresh 같은 후처리만 실패하면 write failure로 되돌리지 않고 당시 `CommittedRefreshWarning` 계열로 분리한다. Current P0-01.11의 canonical 명칭은 `PostCommitWarning`이다.

### 15.7 P1-5 — Update가 whole-record인지 partial patch인지 미정

JSON에서 누락된 field가 `기존값 유지`인지 `기본값으로 초기화`인지 정의되지 않으면 typed adapter마다 다른 결과가 생길 수 있다.

P0 Pilot은 **whole-record typed replacement**로 고정하는 것이 안전하다.

```text
지원 writable field 전체가 schema상 명시됨
누락 required field = Invalid
부분 patch = P0 Scope Out
```

### 15.8 P1-6 — Stable Logical ID와 Object Path의 우선순위 누락

현재 Plan은 Logical ID와 Target Object Path를 모두 요구하지만 rename/move/path collision 시 어느 쪽을 authority로 쓸지 정의하지 않는다.

CF-FQ-045의 existing `FCFDATypeRegistry` / `FCFDATypeAdapter` stable identity 계약을 재사용한다.

```text
Stable identity 적용 타입
→ typed identity로 existing target 탐색
→ 같은 ID가 다른 path에 있으면 Create 금지 / TargetMoved 계열 Conflict
→ target path에 다른 class/identity가 있으면 PathCollision

Identity N/A 타입
→ exact ObjectPath + BaseSemanticFingerprint authority
→ 자동 relocation 금지
```

### 15.9 P1-7 — 기존 Batch Authoring safety primitive와의 재사용 경계가 약함

현재 Vehicle Data Authoring에는 이미 `FCFBatchImportService` 중심의 3-way Preview, `BatchPlanHash`, exact approval, source fingerprint mismatch, global preflight, typed rollback과 dirty-state 복원이 구현돼 있다.

CF-FQ-049가 이를 단순 참고만 하고 동일 문제를 다시 별도 구현하면 장기적으로 두 개의 approval/conflict engine이 생긴다.

`DAS-P0-00~01`에서 다음 중 하나를 명시적으로 선택해야 한다.

```text
A. 차량 전용 코드를 그대로 재사용하지 않되
   approval/fingerprint/preflight 같은 generic primitive를 공통 계층으로 안전하게 추출

또는

B. CF-FQ-049 전용 typed service를 두되
   기존 proven semantics와 동일한 공통 contract/test matrix를 재사용
```

Vehicle Recipe/Profile 도메인 자체를 CF-FQ-049로 이관하는 것은 금지한다.

### 15.10 P1-8 — Missile Pilot 신규 Asset 검증과 P0 delete 금지가 충돌함

현재 P0-04는 `신규 Preset 하나를 Editor-off Staging으로 추가`하는 검증을 요구하지만 P0는 일반 Asset delete를 금지한다. Product Content에 검증용 Preset을 계속 남기는 구조가 되어서는 안 된다.

Pilot을 다음 두 lane으로 분리한다.

```text
Product Pilot
→ 기존 Low/Normal/High를 NoChange/의도된 Update Preview 대상으로 사용
→ 사용자 승인 없는 Product 값 mutation 0

Automation Fixture
→ 전용 test package/root에서 Create/Update/durable-save 검증
→ test-owned fixture cleanup은 Product delete 기능과 분리된 테스트 수명주기 예외
```

기존 `EnsureMissileFeelPresetAsset`은 generic writer owner가 아니라 current idempotence/reference evidence로만 사용한다.

### 15.11 P2-1 — P0 primary execution surface 명시 필요

사용자 의도와 현재 CarFight Authoring 구조를 기준으로 P0 기본 경로는 **이미 실행 중인 Editor의 Editor-side service + 명시적 Apply**로 고정하는 것이 적절하다.

Editor-off Staging 작성은 Editor를 기동하지 않는다. Headless `UnrealEditor-Cmd`/Commandlet batch materialization은 P0 필수 조건으로 만들지 않고 후속 최적화로 둔다.

### 15.12 P2-2 — Data Asset Manager refresh는 post-commit convenience로 분리

CF-FQ-045 Manager는 writer owner가 아니다. Apply 성공 뒤 changed path를 전달해 normal metadata refresh를 유도할 수 있지만, refresh failure가 durable write 실패를 의미해서는 안 된다.

### 15.13 P2-3 — SchemaRevision 외 adapter semantic revision 포함 검토

Staging schema가 같아도 C++ typed adapter의 canonicalization/validation 의미가 바뀔 수 있다. Preview/approval evidence에 adapter semantic revision 또는 equivalent contract revision을 포함할지 DAS-P0-01에서 확정한다.

### 15.14 재검수 통과 조건

Implementation Gate를 열려면 최소 다음이 필요하다.

```text
1. DAS-P0-00 current Source audit 완료
2. pre-existing dirty target fail-closed 계약 확정
3. Update mandatory baseline 확정
4. exact approval + TOCTOU + approval consumption 확정
5. durable partial batch 결과/재시도 규칙 확정
6. durable save confirmation 결과 taxonomy 확정
7. whole-record P0 update semantics 확정
8. stable identity/path collision 규칙 확정
9. existing Batch Authoring safety primitive 재사용 경계 확정
10. Missile Product Pilot / Automation Fixture 분리 확정
11. 재검수 P0 0 / blocking P1 0
```

DAS-P0-00은 계속 read-only Gate다. 이 Initial Design Review만으로 DAS-P0-00 PASS를 주장하지 않는다.

---

## 16. Historical — Design Correction + Re-review — 2026-09-08

### 16.1 교정 결과

Initial Design Review의 `P0 1 / P1 8 / P2 3`을 normative §4/§7/§8/§9/§10 계약에 직접 흡수했다. Review 문단에만 안전 규칙이 있고 실제 단계 정의가 느슨했던 상태를 제거했다.

DAS-P0-00 Source Audit에서 추가로 발견한 `CFMissileGuidePresetData`의 CF-FQ-045 27종 Registry coverage 공백도 P0-01 compatibility seam으로 교정했다.

### 16.2 재검수

| Initial finding | 교정 결과 | 재검수 |
| --- | --- | --- |
| pre-existing dirty target ownership | existing dirty → fail-closed / mutation 0 | PASS |
| Update baseline optional | Update `BaseSemanticFingerprint` mandatory | PASS |
| approval/TOCTOU 느슨함 | exact fields + BatchPlanHash + global preflight + approval consumption | PASS |
| durable multi-package failure 불명확 | deterministic order + Stop + PartialApplied + no durable rollback | PASS |
| SavePackage와 durable success 혼동 | clean/persisted/readback confirmation 분리 | PASS |
| partial patch 의미 미정 | P0 whole-record only | PASS |
| Stable ID / path precedence 미정 | typed identity authority + TargetMoved/PathCollision | PASS |
| 기존 Batch safety와 중복 위험 | domain code 분리 + proven safety contract/test matrix 재사용 | PASS |
| Missile 신규 Product Asset/삭제 충돌 | Product Pilot / Automation Fixture 분리 | PASS |
| 실행면 미정 | already-running Editor-side C++ service + explicit Apply | PASS |
| Manager refresh failure 의미 미정 | post-commit warning 분리 | PASS |
| Adapter semantic revision 미정 | `AdapterContractRevision` approval/fingerprint contract 추가 | PASS |
| Missile Registry coverage 신규 발견 | `PresetId` ExplicitFName descriptor coverage P0-01 prerequisite | PASS |

재검수 판정:

```text
P0: 0
blocking P1: 0
P2: 0
DAS-P0-00: PASS
Design Correction + Re-review: PASS
Product Source implementation: Not Started
UE Asset mutation/save: 0
Historical next exact Gate at v0.2.0: DAS-P0-01 Staging Authority / Schema Design
```

이 PASS는 구현 완료를 뜻하지 않는다. **설계 blocker가 제거되어 DAS-P0-01 상세 계약 동결로 진행 가능하다는 의미**다.

---

## 17. Changelog

### v0.7.0 - 2026-09-09

- `DAS-P0-05 Final Acceptance`를 current Source·canonical Staging·v0.6.4 final Build/Automation evidence와 fresh persisted AssetDump로 감사해 `P0 0 / P1 0 / P2 0` PASS로 닫고 CF-FQ-049 P0를 Complete 처리했다.
- Current owner `Document/Systems/DataManagement/DataAssetAuthoring.md v1.0.0`과 `SystemIndex v1.33.0`으로 G1을 완료하고 FeatureQueue/ActiveWork/Plan Index current route cleanup, Archive Index reference preservation까지 G0~G4를 닫았다.
- Product Low/Normal/High `ApplyReviewed`는 closure에서 실행하지 않아 Product UE Asset Apply·Save 0을 유지했다. fresh persisted acceptance audit은 `/Game/Test/CarFight`의 `CFMissileGuidePresetData` asset_count=3 / success=3 / failure=0이었다.
- 대표 Plan은 `Historical + Retained Path`, G5 physical move는 Deferred다. 현재 단일 Active `CF-FQ-039`와 CF-FQ-045 read-first Manager ownership은 변경하지 않았다. Ammo/Damage/Sensor 등 타입 확장은 별도 후속 lifecycle로만 연다.

### v0.6.4 - 2026-09-09

- `DAS-P0-05` Post-Correction 중간검수의 `P0 0 / P1 3 / P2 0`을 전건 교정하고 fresh 재검수 `P0 0 / P1 0 / P2 0` PASS로 닫았다. Current System Promotion은 아직 시작하지 않았다.
- exact Staging selection, selection-bound `FCFDAStagingOpsSession`의 fresh Review/stale hash reject, `SyncProduct` touched-file raw-byte rollback을 추가했다. Product Low/Normal/High Apply·Save는 0이다.
- final official UE 5.8 Build `7804b91cfac542fd9aee78af0f2d692f` PASS, OperationalEntry `9b3f76b7a78b4f3cb6f30e11ab140327` exact 1/1 PASS + Product Staging residue 0, existing DAS regression `49e54542dd3f42f9a3eee2c10b09e5ed` exact 13/13 PASS를 확보했다.
- exact next는 계속 `DAS-P0-05 Acceptance / Current System Promotion`이며 CF-FQ-039 Active와 CF-FQ-045 read-first Manager ownership을 유지한다.

### v0.6.3 - 2026-09-09

- `DAS-P0-05 Contract Review Correction + Re-review`에서 v0.6.2 P1 2건을 교정하고 fresh 재검수 `P0 0 / P1 0 / P2 0` PASS로 닫았다. Current System Promotion은 아직 시작하지 않았다.
- current normative schema/example를 AdapterContractRevision 2로 정렬하고 revision 1은 Historical/fail-closed evidence로만 보존했다.
- `CFDAStagingOps.h/.cpp`와 `CFDAStagingOpsTests.cpp`, `Tools/RunDAStagingOpsTest.ps1`을 추가해 converged-only Product bootstrap/rebase, canonical discovery/Preview, fresh-hash Review, explicit `ApplyReviewed` 운영 entry를 최소 C++ facade로 연결했다.
- canonical Product Staging `MissileFeel_Low/Normal/High.json` 3종을 repository에 생성했다. 이 JSON은 생성 후 Editor-off 편집 가능하지만 baseline bootstrap/rebase Sync는 current Product truth를 읽는 Editor-side 작업이다.
- fresh official UE 5.8 Build `6196bcf0c631413093dfe1d871125f60` PASS, OperationalEntry `111adc5a0013413ca1e9f4426e618124` exact 1/1 PASS, existing regression `d2c15a9070b34583a3080b4071c50daa` exact 13/13 PASS와 P0-04 residue 0을 확인했다.
- Product Low/Normal/High `ApplyReviewed`는 실행하지 않았으며 Product UE Asset Apply·Save는 0이다. exact next는 `DAS-P0-05 Acceptance / Current System Promotion`이고 CF-FQ-039 Active 및 CF-FQ-045 read-first Manager ownership을 유지한다.

### v0.6.2 - 2026-09-08

- `DAS-P0-05 Acceptance / Current System Promotion` 착수 전 current Plan·Systems·Source·runner를 계약검수해 `P0 0 / P1 2 / P2 0 / Promotion HOLD`로 판정했다.
- P1-1은 current Source AdapterContractRevision 2와 달리 normative schema/example에 revision 1이 남은 stale contract다. Historical v0.3.0 기록은 보존하고 current 작성 지침만 revision 2로 교정해야 한다.
- P1-2는 canonical `Authoring/DataAssetStaging/` Product JSON이 아직 없고 non-test discovery→Preview→reviewed Apply caller도 없어 Editor-off authoring을 실제 Current workflow로 승격할 수 없는 운영 진입 공백이다.
- Product Low/Normal/High Apply·Save는 0이며 DAS-P0-04 official Build/exact 13/13/fresh AssetDump evidence는 보존한다. Current Active CF-FQ-039와 CF-FQ-045 read-first Manager owner는 변경하지 않았다.
- exact next를 `DAS-P0-05 Contract Review Correction + Re-review`로 고정하고 P1 2건이 닫히기 전 `DataAssetAuthoring.md` Current Systems promotion과 Feature Done 승격을 금지한다.

### v0.6.1 - 2026-09-08

- `DAS-P0-04` Post-PASS 중간검수 `P0 0 / P1 2 / P2 1`을 교정하고 재검수 `P0 0 / blocking P1 0 / P2 0` PASS로 닫았다.
- persisted FText canonicalization 의미 변경을 `AdapterContractRevision 2`로 승격했다. revision 1 Staging/approval은 silent reinterpret하지 않고 `AdapterRevisionMismatch`로 fail-closed하며 current authoring은 revision 2 fresh Preview/fingerprint를 사용한다.
- fixture cleanup을 persisted/on-disk AssetRegistry + resolver-visible loaded UObject + physical Content + Staging의 4-authority residue 0 계약으로 강화했다. loaded-but-unregistered DataAsset을 실제 생성해 residue detector가 fail하는 negative regression도 추가했다.
- cleanup 구현은 package unload → GC → exact Content delete → `ScanModifiedAssetFiles` disk refresh → Staging delete → 4-authority verification 순서로 교정했다. 앞선 교정 시도에서 발견한 watcher resurrection과 loaded package file-handle 실패는 최종 경로에서 제거됐다.
- 최종 official Build Job `0c07458d17ac4b0285ef6339d35962c1` Exit 0, focused Process Job `7de3e15934dd4b14940ff812597c400e` exact 13/13 PASS / failure·missing·unexpected·duplicate-terminal 0이다.
- fresh parent AssetDump `/Game/Test/CarFight`의 `CFMissileGuidePresetData` 총 3개가 정확히 Product Low/Normal/High뿐임을 다시 확인했다. Product `.uasset` Apply/Save는 0이며 exact next는 계속 `DAS-P0-05 Acceptance / Current System Promotion`이다.

### v0.6.0 - 2026-09-08

- `DAS-P0-04 MissileGuidePreset Pilot`을 Product mutation0 lane과 test-owned durable fixture lane으로 분리해 Technical PASS로 닫았다. Product Low/Normal/High는 NoChange/Update Preview만 검증했고 Product Apply/Save는 0이다.
- test-owned fixture에서 Create→Save→disk reload→persisted readback→Update→Save→disk reload→persisted readback, baseline drift, pre-existing dirty, SavePackage/confirmation uncertainty와 `PartialApplied`를 검증했다.
- 첫 exact 13/13 PASS 뒤 `/Game/Test/CarFight/DAStagingP04` residue 6개를 발견해 승격을 보류했고, root-level AssetRegistry delete→package unload→GC→recursive delete→residue assertion과 runner pre/post residue gate를 구현했다. cleanup failure는 이제 Automation failure다.
- 기존 residue cleanup Process `4858c94ed91d4a94bda74d237ec2e20f` Exit 0, fresh official Build `2b1b77739d6f4541b38720b4a11b9199` Exit 0, focused Process `95039bc3c6e04d6e83631ec7d773e093` exact 13/13 PASS를 확보했다.
- final Git status에 test Content residue가 없고 fresh parent AssetDump `/Game/Test/CarFight`의 `CFMissileGuidePresetData` 총 3개가 정확히 Product Low/Normal/High뿐임을 확인해 persisted fixture residue 0을 닫았다.
- P0-04 PASS / Product save 0이며 exact next를 `DAS-P0-05 Acceptance / Current System Promotion`으로 전진했다.

### v0.5.1 - 2026-09-08

- `DAS-P0-03` Post-PASS 중간검수 `P0 0 / P1 2 / P2 2`를 수행하고 P1/P2 전건을 교정했다.
- `DurableApplied`를 same in-memory UObject 확인에서 exact SavePackage 뒤 non-interactive package disk reload + re-resolved typed semantic fingerprint confirmation으로 강화했다. reload/readback 불확정은 `SaveStateUnconfirmed`로 fail-closed한다.
- `FCFDAStagingService::ExtractMissilePresetPayload`를 공용 semantic extractor로 승격해 current resolver, Update rollback snapshot, pre/post-save readback의 FText/typed 의미를 하나로 통합했다.
- `PartialApplied`는 confirmed durable success 뒤 known failed-before-durable이며 uncertainty가 없는 경우로만 한정하고 `SaveStateUnconfirmed / InMemoryStateUnconfirmed` precedence를 다시 명시했다.
- corrected official Build Job `fdd50db886b04387b8aa72c83178310b`은 Exit Code 0, focused Process Job `45c42b68974341e7a45e0105d230a519`은 exact 8/8 PASS / failure·missing·unexpected·duplicate-terminal 0이다.
- 최종 재검수는 `P0 0 / blocking P1 0 / P2 0` PASS이며 Product UE Asset Apply/Save는 0, exact next는 `DAS-P0-04 MissileGuidePreset Pilot`이다. 실제 durable fixture 실행은 P0-04 소유다.

### v0.5.0 - 2026-09-08

- `DAS-P0-03 Exact Materializer + Apply` source와 mutation0 safety/lifecycle foundation을 구현·검증하고 PASS로 닫았다.
- `CFDAStagingApply.h/.cpp`를 추가해 exact CFMissileGuidePresetData allowlist, one-shot Reviewed approval, disk/current global TOCTOU, deterministic Create/Update, target-only SavePackage, typed durable confirmation과 fail-safe result taxonomy를 구현했다.
- P0-02에서 후속으로 넘긴 loaded-but-unregistered same StableIdentity current truth를 Asset Registry + loaded `/Game` object union으로 교정하고 focused Automation으로 `TargetMoved` 차단을 검증했다.
- 첫 코드 재검수에서 `global preflight 이후 뒤 target drift`와 `Create rollback package absence 과대 판정` P1 2건을 발견해 per-target mutation 직전 재검증과 conservative `InMemoryStateUnconfirmed` 판정으로 교정했다. 최종 재검수는 `P0 0 / blocking P1 0 / P2 0` PASS다.
- 최종 공식 Build Job `3a6592acc9de43738ca4696875cd1132`은 Exit Code 0, focused Process Job `2974bdd0d6864743b3e82a1b63620cf5`는 exact 8/8 PASS / failure·missing·unexpected·duplicate-terminal 0이다.
- Product UE Asset Apply/Save는 0이며 persisted Create/Update/readback/save-failure/PartialApplied fixture는 계획대로 `DAS-P0-04 MissileGuidePreset Pilot`이 소유한다. exact next를 P0-04로 전진했다.

### v0.4.1 - 2026-09-08

- `DAS-P0-02` 중간검수 `P0 0 / P1 2 / P2 2`를 수행하고 blocking P1 2건을 교정했다.
- canonical Staging source를 `Authoring/DataAssetStaging/` 아래 lowercase `.json`으로 강제하고, parse 뒤 mutable source/target path와 Create/Update intent binding을 Preview/BatchPlanHash에서 다시 검증하도록 defense-in-depth를 추가했다.
- Payload와 cached `StagingSemanticFingerprint`가 갈라진 mutable DTO를 Preview와 approval hash 양쪽에서 fail-closed하도록 교정했다.
- 최초 P2 2건은 각각 DAS-P0-03 global preflight와 DAS-P0-04 persisted fixture가 이미 소유하는 후속 검증으로 재분류했다.
- 교정 후 공식 Build Job `83747e73cc38407689af72a66faf1905`은 Exit Code 0, focused Process Job `e984fa6f89b341629b63caefb47ed693`은 exact 6/6 PASS / failure·missing·unexpected·duplicate-terminal 0을 기록했다.
- 최종 재검수 `P0 0 / blocking P1 0 / P2 0` PASS이며 Product UE Asset Apply/Save는 0, exact next는 계속 `DAS-P0-03 Exact Materializer + Apply`다.

### v0.4.0 - 2026-09-08

- `DAS-P0-02 Typed Staging Parse + Preview Foundation`을 구현·검증하고 PASS로 닫았다.
- `CFMissileGuidePresetData`를 CF-FQ-045 Registry의 Required / ExplicitFName(`PresetId`) descriptor로 편입해 current coverage를 27→28로 확장했다.
- strict whole-record parse, typed canonical SHA-256, read-only current target/StableIdentity resolver, exact 3-way Preview, duplicate/path/dirty fail-closed와 deterministic `BatchPlanHash` foundation을 구현했다.
- UE 5.8 standalone compile에서 드러난 JSON shared-key API 호환성을 교정했고, unrelated `CFVehicleVisualComp.cpp` C2264 blocker도 runtime contract 변화 없이 exact `GetComponents(..., false)` overload로 최소 교정되어 공식 Editor Build가 PASS했다.
- 최종 공식 Build Job `75f96683c5f94aa8829336dae7268fd3`은 Exit Code 0, `Tools/RunDAStagingTests.ps1` Process Job `8ea596e2f323453e83267b3ecbaedcbd`는 exact 6/6 PASS / failure·missing·unexpected·duplicate-terminal 0을 기록했다.
- Product UE Asset Apply/Save는 0이며 exact next를 `DAS-P0-03 Exact Materializer + Apply`로 전진했다. P0-03 구현·Apply·Save는 아직 시작하지 않았다.

### v0.3.0 - 2026-09-08

- `DAS-P0-01 Staging Authority / Schema Design`을 완료하고 설계검수 교정 후 `P0 0 / blocking P1 0 / P2 0` PASS로 닫았다.
- canonical Staging root를 `<main_game>/Authoring/DataAssetStaging/`로 고정하고 Pilot을 one-file-per-whole-record JSON으로 정의했다.
- `CarFight.DataAsset.MissileGuidePreset` SchemaRevision 1 / AdapterContractRevision 1, strict whole-record envelope, FName/FText/enum/struct/bool/float canonical typed representation을 동결했다.
- Base/Current/Staging SHA-256 semantic fingerprint, exact 3-way Preview, converged NoChange + explicit baseline rebase, StableLogicalId↔Payload identity equality와 Create/TargetMoved/PathCollision/dirty fail-closed 의미를 확정했다.
- exact target-set `BatchPlanHash`, one-shot approval, Staging+Unreal global TOCTOU, no automatic retry와 deterministic stop-on-first-failure를 동결했다.
- result taxonomy에서 `SaveStateUnconfirmed` / `InMemoryStateUnconfirmed` uncertainty를 `PartialApplied`보다 우선하고 durable success와 post-commit warning을 분리했다.
- Vehicle Batch는 P0에서 generic refactor하지 않고 proven safety semantics/test matrix만 재사용하는 Option B를 선택했다.
- `CFMissileGuidePresetData` CF-FQ-045 Registry descriptor coverage 27→28 보강을 `DAS-P0-02` 첫 Source prerequisite로 확정했다. Product Source/UE Asset mutation은 아직 0이다.
- exact next를 `DAS-P0-02 Typed Staging Parse + Preview Foundation`으로 전진했다.

### v0.2.0 - 2026-09-08

- DAS-P0-00 current Source Audit을 read-only로 완료하고 PASS했다. CF-FQ-045 semantic registry/adapter, CF-FQ-038 Batch safety, Vehicle specialized authoring, CF-FQ-030 Missile Preset authoring 경계를 다시 확정했다.
- Initial Review P0/P1/P2를 normative safety/lifecycle/schema/materializer/pilot 계약에 흡수하고 재검수 `P0 0 / blocking P1 0 / P2 0` PASS로 닫았다.
- `CFMissileGuidePresetData`가 current 27종 DA Registry에 아직 포함되지 않은 신규 coverage gap을 발견해 `PresetId` Required/ExplicitFName semantic descriptor 보강을 P0-01 prerequisite로 추가했다. 이는 Manager writer 확장이 아니라 current type coverage 유지보수다.
- Update mandatory baseline, AdapterContractRevision, exact BatchPlanHash approval, global TOCTOU, pre-existing dirty fail-closed, durable confirmation, PartialApplied, whole-record P0, stable identity/path precedence를 고정했다.
- Missile Pilot을 Product NoChange/Preview lane과 Automation Create/Update/Save fixture lane으로 분리했다.
- exact next를 `DAS-P0-01 Staging Authority / Schema Design`으로 전진했다. Product Source/UE Asset implementation은 아직 시작하지 않았다.

### v0.1.1 - 2026-09-08

- Initial Design Review를 current Data Asset Manager, Vehicle Batch Authoring, Vehicle Builder durable save와 MissileGuidePreset Source에 교차대조해 `P0 1 / P1 8 / P2 3 / Implementation HOLD`로 판정했다.
- P0는 pre-existing dirty target package의 미저장 사용자 변경을 CF-FQ-049가 함께 저장할 수 있는 ownership 공백이다. P0 기본은 `existing target dirty → fail-closed / mutation 0`으로 교정한다.
- P1은 mandatory Update baseline, exact approval/TOCTOU, durable partial batch, durable save confirmation, whole-record update semantics, stable identity/path precedence, existing Batch safety primitive 재사용, Missile test fixture lifecycle 8건이다.
- P2는 primary execution surface, Manager refresh post-commit 분리, adapter semantic revision 3건이다.
- exact next는 `DAS-P0-00 Current DA Authoring / Creation Audit`을 유지한다. 이 Audit과 후속 P0/P1 설계 교정·재검수 `P0 0 / blocking P1 0` 전에는 Source/Asset implementation을 시작하지 않는다.

### v0.1.0 - 2026-09-08

- USER 승인으로 `CF-FQ-049 Data Asset Staging·Batch Authoring` 정식 승격 계획을 생성했다.
- 실제 CarFight Source/Systems 감사에서 확인한 CF-FQ-045 Data Asset Manager, CF-FQ-038 Vehicle Batch Authoring, Vehicle Builder JSON Draft, CF-FQ-030 MissileGuidePreset authoring 경계를 기준선으로 반영했다.
- `.uasset`을 최종 Source of Truth로 유지하고 외부 JSON/CSV는 Editor-off 저작을 위한 Staging 입력으로만 사용하는 원칙을 고정했다.
- 첫 Pilot은 완료된 `CFMissileGuidePresetData` 저작 경로로 한정하며 Missile Runtime/USER Feel을 재오픈하지 않는다.
- P0는 `DAS-P0-00` Audit → `P0-01` Authority/Schema → `P0-02` Typed Preview → `P0-03` Materializer → `P0-04` Missile Pilot → `P0-05` Acceptance/Promotion 순으로 정의했다.
- 승격 시점 Product Source/UE Asset/Build/Automation mutation은 0이다.

---

## 18. Migration

- 기존 DataAsset과 현재 `.uasset` 저작 경로는 CF-FQ-049 Pilot이 검증되기 전까지 그대로 유효하다.
- 기존 Low / Normal / High MissileGuidePreset `.uasset`과 현재 idempotent Ensure authoring 경로를 승격만으로 삭제·변경하지 않는다.
- CF-FQ-045의 Data Asset Manager는 기존 read-first Current 계약을 유지한다.
- CF-FQ-038의 Vehicle Authoring과 Vehicle Builder의 Recipe/Profile/Resolver/Apply 계약은 CF-FQ-049로 이관하지 않는다.
- Post-P0에서 새로운 DA 종류를 Staging 대상으로 편입하려면 해당 타입의 Typed Adapter/Schema와 conflict/save 계약을 별도로 검증한다.
