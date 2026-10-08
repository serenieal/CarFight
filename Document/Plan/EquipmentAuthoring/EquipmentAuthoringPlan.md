# CF-FQ-054 Equipment Authoring / Guided Equipment Builder

- 문서 버전: v0.1.18
- 작성일: 2026-09-16
- 최근 갱신일: 2026-09-29
- 문서 상태: Paused / HOLD — CF-FQ-058 Content Authoring architecture prerequisite / EBA-P0-05 Technical PASS Preserved
- Feature: `CF-FQ-054 Equipment Authoring / Guided Equipment Builder`
- Priority: P2
- 현재 정확한 Gate: `HOLD — CF-FQ-058이 EquipmentPreset / Vehicle / Weapon authoring 경계를 Freeze한 뒤 EBA-P0-05 USER Editor UX를 fresh rebaseline`
- 현재 단일 Active: `CF-FQ-039 Production UI Visual Rework` unchanged
- Current implementation authority: main_game 실제 Source / Asset + 관련 Systems 문서

---

## 1. 목적

이 Feature는 CarFight의 현재 장비 Runtime 계약을 새로 설계하는 기능이 아니다.

목적은 이미 존재하는 `UCFEquipmentPresetData`와 Vehicle/Fitting/Weapon/Sensor Runtime을 그대로 authority로 유지하면서, 사용자가 Unreal Editor에서 Vehicle Builder와 유사한 흐름으로 장비 프리셋을 안전하게 조립·검토·검증·저장할 수 있는 **Editor-only Guided Equipment Builder**를 제공하는 것이다.

```text
현재 수동 경로
여러 DataAsset과 Mount 계약을 사용자가 직접 찾아 연결
        ↓
누락/잘못된 조합을 뒤늦게 Runtime에서 발견

목표 경로
Equipment Builder
        ↓
EquipmentPreset Draft
        ↓
Child Data 선택 + 계약 검증
        ↓
Mount 호환성 확인
        ↓
Before / After Review
        ↓
Fresh Review / TOCTOU Revalidation
        ↓
Explicit `적용 및 저장`
        ↓
CFDADurableCore exact1 durable commit
        ↓
Disk Reload + Persisted Semantic Readback
```

이 Feature의 핵심 가치는 새로운 장비 Runtime framework가 아니라 **현재 장비 계약의 안전한 저작 UX**다.

---

## 2. Accepted Ownership Audit baseline — 2026-09-15

2026-09-15 fresh Source / Current System 교차감사 결과를 이 Plan의 Accepted baseline으로 사용한다. EBA-P0-00에서 같은 Ownership Audit을 처음부터 반복하지 않는다. Current Source contradiction이 발견될 때만 해당 항목을 다시 연다.

현재 `UCFEquipmentPresetData`가 지원하는 실제 장비 조립 구조는 다음과 같다.

```text
EquipmentPresetData
│
├─ Weapon Equipment
│   ├─ TurretMountData
│   └─ WeaponData
│       ├─ AmmoData
│       └─ ProjectileData
│           └─ DamageData
│
└─ Utility Scanner
    └─ VehicleSensorData
```

현재 P0에서 `Defense`, `Power`, `Storage`, `Repair`, `ECM`, `Passive` 같은 신규 generic Equipment category를 발명하지 않는다. Defense는 현재 별도 VehicleDefense/Fitting 경로를 사용하며, Utility의 실제 EquipmentPreset 의미는 Scanner다.

Launcher도 별도 Equipment 종류가 아니다. 현재 Launcher 의미는 WeaponData의 Launcher 설정, ProjectileData의 추진/미사일 설정, TurretMountData의 Muzzle 구조를 통해 표현된다.

---

## 3. Ownership Freeze 후보

### 3.1 Equipment Builder 직접 Write Authority

P0의 persisted write authority는 다음 exact1 class로 제한한다.

```text
UCFEquipmentPresetData
```

Equipment Builder가 직접 저작할 수 있는 현재 필드는 다음 범위다.

```text
EquipmentId
DisplayName
RequiredMountType
RequiredWeaponSize
DefaultTurretMountData
DefaultWeaponData
DefaultSensorData
```

`RequiredWeaponSize`는 generic Equipment 관점에서 이름이 어색하지만 기존 Runtime/serialization 계약이다. 이 Feature 때문에 리네이밍하지 않는다. 사용자 UI에서는 의미에 맞게 `요구 장착 크기`로 표현할 수 있다.

### 3.2 Read / Select / Validate Authority

다음 데이터는 Equipment Builder가 직접 persisted mutation하지 않는다.

```text
WeaponData
TurretMountData
ProjectileData
VehicleSensorData
AmmoData
DamageData
VehicleData MountProfile / Hardpoint
```

Equipment Builder가 수행할 수 있는 역할은 다음으로 제한한다.

```text
- Asset 선택
- Asset 열기
- 현재 reference chain 표시
- 기존 validation 계약 호출/표시
- Equipment 조립 관점의 bounded cross-reference 검사
- Vehicle Mount와 read-only 호환성 검사
```

### 3.3 기존 시스템 재사용

```text
Vehicle Builder
= Hardpoint / MountProfile / MountType / SizeLimit / Vehicle-side DefaultEquipment authority

Data Asset Manager
= DataAsset 발견 / 분류 / 검사 / 참조 조회 / 열기

Data Asset Authoring
= 현재 지원 타입의 Reviewed authoring authority
  특히 AmmoData / DamageData writer를 Equipment Builder가 중복 구현하지 않음

Vehicle Fitting
= 실제 선택 조합 / Snapshot / 질량 / Ammo / Defense / Sensor validation authority

RuntimeApply
= 실제 차량에 장비를 runtime 적용하는 application seam
```

### 3.4 Equipment Builder가 소유하지 않는 것

```text
Vehicle Mount 정의/변경
VehicleFittingData 영구 선택 저장
Applied Fitting Snapshot 직접 조작
Inventory ownership / Reservation / Transfer
Runtime weapon pointer 직접 교체
Runtime application algorithm
Runtime Test Catalog 자동 등록/승격
AmmoData / DamageData parallel writer
모든 UDataAsset을 수정하는 generic writer
```

이 경계 중 하나를 구현 편의를 이유로 침범해야 한다면 즉시 HOLD하고 Ownership/Architecture Review로 돌아간다.

---

## 4. P0 사용자 범위

P0의 사용자 목표는 **EquipmentPresetData 하나를 안전하게 만들고 수정하는 것**이다.

포함 범위:

```text
- 기존 EquipmentPreset 목록 탐색
- 새 EquipmentPreset 생성 시작
- EquipmentId / DisplayName 작성
- UI Draft Mode: 무장 장비 / 스캐너 장비
- RequiredMountType / RequiredWeaponSize 작성
- Weapon 장비: TurretMountData + WeaponData 선택
- Scanner 장비: VehicleSensorData 선택
- Child reference chain과 validation 결과 표시
- Vehicle Mount read-only 호환성 확인
- Before → After Review
- fresh Review / approval digest / stale guard 확인
- 명시적 `적용 및 저장` 단일 durable action
- exact target package Save + disk reload + persisted semantic readback
- 실패 원인과 해결 경로를 사용자에게 구체적으로 표시
```

P0 제외 범위:

```text
- persisted EquipmentType enum 신규 추가
- WeaponData 직접 편집/저장
- TurretMountData 직접 편집/저장
- ProjectileData 직접 편집/저장
- VehicleSensorData 직접 편집/저장
- AmmoData / DamageData 직접 편집/저장
- Defense/Power/Storage/Repair/ECM generic equipment 추가
- Vehicle Builder의 광범위한 구조 리팩터링
- Runtime Catalog 자동 등록
- Inventory/Fitting mutation
- RuntimeApply 직접 재구현
- 범용 3D Preview framework 신설
```

`무장 장비 / 스캐너 장비` 선택은 **Builder transient Draft Mode**다. 별도 persisted `EquipmentType` 필드를 만들지 않는다. 최종 진실은 기존 EquipmentPreset payload 조합이 소유한다.

---

## 5. Vehicle Builder UX Reuse 원칙

Equipment Builder는 Vehicle Builder와 별개 도구지만 사용자가 같은 제작 도구 계열로 느낄 수 있도록 현재 Vehicle Builder의 검증된 UX 패턴을 우선 재사용한다.

후보 패턴:

```text
- Tool Tab / Page shell
- Asset Browser / selection 흐름
- Section header와 단계형 안내
- Draft 상태 표시
- Validation summary / issue row
- Before / After Review
- Review / durable commit readiness
- 단일 `적용 및 저장` action
- 사용자-facing 한글 상태/해결 안내
```

그러나 EBA를 이유로 Vehicle Builder 전체를 공용 framework로 추출하지 않는다.

EBA-P0-00에서 fresh current Source를 읽고 각 후보를 다음 네 분류로 확정한다.

```text
A. Reuse As-Is
B. Light Extraction / Shared Helper가 명확히 이득
C. Equipment-specific New Implementation
D. Do Not Share
```

공용화는 실제 중복과 안정성 이득이 증명되는 최소 영역만 허용한다. Vehicle Builder의 business state, recipe, vehicle apply, hardpoint authoring을 Equipment Builder에 끌어오지 않는다.

---

## 6. Authoring / Durable Save 계약

EBA-P0-00 correction 이후 P0 authoring 흐름은 다음으로 동결한다.

```text
Persisted EquipmentPreset 또는 New Draft
        ↓ Read / User Edit
Transient Builder Draft
        ↓
Strict prospective validation
        ↓
Before / After Review
        ↓
Review binding / approval digest 생성
        ↓ User: `적용 및 저장`
Fresh target / identity / child / validation / approval revalidation
        ↓
CFDADurableCore::ApplyTypedTarget
        ↓
Exact1 EquipmentPreset Create 또는 Update
        ↓
Exact single-package SavePackage
        ↓
Disk Reload
        ↓
Persisted typed semantic fingerprint readback
```

P0 USER mutation action은 `검토`와 `적용 및 저장`으로 분리한다. `Apply`와 `Save`를 두 개의 독립 사용자 action으로 제공하지 않는다. 성공은 단순히 UObject가 dirty가 된 시점이 아니라 **exact target package save + clean state + disk reload + persisted semantic fingerprint confirmation**이 모두 끝난 경우에만 성립한다.

### 6.1 Durable execution authority

```text
Durable sequencing authority
= CFDADurableCore

Equipment-specific ownership
= FCFEquipmentPresetDraft / typed semantic payload
= Extract exact7 authored semantic
= Materialize exact7 authored semantic
= BuildSemanticFingerprint
= prospective validation / review binding
```

Equipment Builder는 `UPackage::SavePackage`, create cleanup, update rollback, reload/readback sequencing을 별도로 구현하지 않는다. `CFDADurableCore`는 provider registry lookup 없이 typed payload callback과 reviewed row를 받아 실행할 수 있으므로, P0에서 EquipmentPreset을 CFDA production provider exact5 / DACE / JSON Staging으로 자동 onboarding하지 않는다.

Equipment adapter가 durable core에 전달하는 reviewed row에서 P0 execution에 binding되는 값은 최소 다음이다.

```text
PreviewKind = Create | Update
TargetObjectPath = reviewed exact target
StagingSemanticFingerprint = reviewed prospective fingerprint
```

shared core public contract가 이후 변경되면 그 current signature에 맞춰 adapter를 조정하되 durable sequencing ownership은 이동하지 않는다.

### 6.2 Create / Update terminal semantics

Create:

```text
- Commit 직전 memory package/object absence 재확인
- disk package absence 재확인
- collision이면 mutation 0
- pre-save semantic readback failure이면 operation-created UObject/registry/dirty state cleanup 확인
- Save 호출 전 failure에서 cleanup이 확인되지 않으면 InMemoryStateUnconfirmed
```

Update:

```text
- exact target class/path 재확인
- selected target package가 dirty면 mutation 0
- mutation 전에 lossless original typed snapshot / original fingerprint 확보
- pre-save failure이면 original state rollback + semantic readback 확인
```

공통:

```text
- Save All 금지
- exact target package 하나만 durable write
- Child DataAsset mutation/save 0
- Save 호출 뒤 outcome을 확정할 수 없으면 SaveStateUnconfirmed
- SaveStateUnconfirmed를 success로 확대하지 않음
- automatic retry = 0
- 성공 뒤 package reload로 old UObject pointer는 stale일 수 있으므로 exact object path에서 다시 resolve
```

### 6.3 EquipmentPreset semantic snapshot / fingerprint

Equipment semantic payload는 persisted authored field exact7만 소유한다.

```text
EquipmentId
DisplayName
RequiredMountType
RequiredWeaponSize
DefaultTurretMountData object path
DefaultWeaponData object path
DefaultSensorData object path
```

Fingerprint representation은 current shared primitive 의미를 재사용한다.

```text
EquipmentId
→ FName semantic equality와 동일한 canonical name text

DisplayName
→ CFDACommonPrimitives::ReadLiteralTextFromAsset 의미
→ source-backed Literal FText source string
→ package-only namespace/key는 persistence metadata로 제외
→ StringTable / explicit authored namespace / source-less generated non-empty FText는 P0 lossless update unsupported로 fail-closed

Enum
→ enum semantic token

UObject reference
→ pointer/address가 아니라 canonical top-level SoftObjectPath

Final fingerprint
→ shared deterministic token framing + canonical SHA-256
```

신규 Draft의 DisplayName은 Builder가 만드는 Literal FText semantic만 허용한다. Existing Update에서 exact7 전체를 lossless extract할 수 없는 FText representation을 만나면 다른 field만 바꾸는 경우에도 P0에서는 mutation하지 않고 사용자에게 unsupported representation을 표시한다. silent `ToString()` flattening은 금지한다.

Equipment authoring semantic contract revision은 **Revision 1**부터 시작하며 위 fingerprint 의미가 바뀌면 revision을 올리고 기존 Review approval을 재사용하지 않는다.

### 6.4 Child reference semantic boundary

Child DataAsset의 내부 내용을 EquipmentPreset semantic fingerprint에 합성하지 않는다.

```text
EquipmentPreset fingerprint
= child canonical object path만 포함

Commit 직전
= child existence / expected class / current native-or-bounded validation을 fresh 재실행
```

따라서 Child Asset이 Review 이후 바뀌어 현재 validation을 통과하지 못하면 Commit은 mutation 0으로 막지만, Child Asset의 모든 내부 byte를 EquipmentPreset identity에 흡수하지 않는다.

---

## 7. Validation 계층

Equipment Builder는 하나의 `정상/오류` 값으로 전체 장비 상태를 뭉개지 않는다.

### 7.0 Strict prospective validation freeze

P0 `적용 및 저장` readiness는 다음 계층을 모두 평가한 결과다.

```text
Layer 1 — Identity / duplicate authority
Layer 2 — EquipmentPreset intrinsic payload contract
Layer 3 — selected Child authority / bounded structural checks
Layer 4 — Builder-only authoring policy
Layer 5 — optional concrete Vehicle Mount compatibility
```

#### Layer 1 — EquipmentId / duplicate

Current DataManagement descriptor를 그대로 authority로 사용한다.

```text
ClassPath = /Script/CarFight_Re.CFEquipmentPresetData
IdentityPolicy = Required
IdentityResolver = ExplicitFName
IdentitySource = EquipmentId
DuplicateNamespace = ExactClassPath
```

Prospective Create/Update에서는 아직 current inventory에 존재하지 않는 Draft identity를 판정해야 하므로 Builder가 별도 generic identity framework를 만들지 않고 다음 bounded orchestration만 수행한다.

```text
1. current complete DataManagement inventory 필요
2. same exact class CFEquipmentPresetData candidate closure 평가
3. current adapter가 resolved한 canonical FName identities와 Draft EquipmentId 비교
4. Update는 exact selected target 자신만 comparison에서 제외
5. candidate evaluation 또는 duplicate namespace coverage가 incomplete이면 Unique라고 추정하지 않고 blocking `IdentityUniquenessUnconfirmed`
6. 동일 canonical FName 발견 시 blocking Duplicate
```

문자열 case-sensitive 비교를 새로 만들지 않는다. `FName` equality와 current DataManagement canonical identity 의미를 따른다.

### 7.1 EquipmentPreset 계약

```text
- EquipmentId identity
- Weapon vs Scanner payload mutual exclusivity/completeness
- RequiredMountType
- RequiredWeaponSize
- HasCompleteEquipmentData / HasCompleteEquipmentDataForMount 등 기존 계약
```

### 7.2 WeaponData

기존 `ValidateWeaponDataContract()` / Editor DataValidation을 재사용한다.

```text
- Weapon identity
- mount type / size compatibility
- mass / fire / ammo / heat / charge 정적 계약
- Projectile / Ammo reference 상태
```

Weapon Runtime state나 Launcher Scheduler state를 Builder validation이 재구현하지 않는다.

### 7.3 VehicleSensorData

기존 Sensor native validation 계약을 재사용한다.

```text
IsSensorDataContractValid
ValidateSensorDataContract
Editor IsDataValid
```

Equipment Builder가 별도 Sensor authoring framework를 만들지 않는다.

### 7.4 TurretMountData / ProjectileData

현재 Data Asset Management 기준으로 이 두 타입은 Weapon/Sensor처럼 완결된 native validation authority를 갖고 있지 않다.

따라서 P0에서 `전체 유효함`을 임의로 선언하지 않는다.

Equipment 조립을 위해 실제로 필요한 bounded checks만 별도 계층으로 표시할 수 있다.

예:

```text
TurretMountData reference 존재
필요한 visual/muzzle reference 상태
선택한 Weapon/Equipment mount 의미와 명백한 모순 여부

ProjectileData reference chain 존재
DefaultDamageData 상태
Weapon FireMode와 명백한 reference 모순 여부
```

Mesh Socket 실재 여부처럼 persisted asset 사실이 필요하면 AssetDump 또는 적절한 Editor evidence를 사용한다. 이를 native DataValidation이 이미 존재하는 것처럼 표시하지 않는다.

### 7.5 AmmoData / DamageData

기존 Data Asset Authoring / Management 경계를 재사용한다. Equipment Builder 안에 parallel durable writer를 만들지 않는다.

### 7.6 Vehicle Mount compatibility

VehicleData를 수정하지 않고 concrete MountProfile이 선택된 경우에만 다음을 read-only로 평가한다.

```text
MountProfile MountType
MountProfile SizeLimit
EquipmentPreset required contract
WeaponData mount contract
UCFEquipmentPresetData::CanUseOnMount(MountType, SizeLimit)
```

중요한 current runtime 의미:

```text
RequiredMountType=None
= 프리셋 자체의 mount type 제한을 생략하는 wildcard

HasCompleteEquipmentDataForMount(None)
= false
```

따라서 `RequiredMountType=None`을 invalid로 만들지 않으며, concrete Mount가 없는 intrinsic validation에서 `HasCompleteEquipmentDataForMount(None)`을 호출해 false를 오류로 해석하지 않는다. Scanner Draft는 current migration 계약대로 `RequiredWeaponSize=None`을 요구한다.

`현재 Mount에 장착 가능`과 `전체 Fitting Snapshot 적용 가능`은 같은 판정이 아니다. Gross Mass, sortie ammo, complete fitting 등 최종 적용 가능성은 Fitting authority가 소유한다.

### 7.7 Review / Approval / TOCTOU binding

Review approval은 UI 화면을 봤다는 사실만으로 Commit 권한이 되지 않는다. Review 생성 시 다음 exact state를 binding한다.

```text
OperationKind = Create | Update
TargetObjectPath
TargetClassPath = /Script/CarFight_Re.CFEquipmentPresetData
CurrentSemanticFingerprint = Update에서 required / Create에서는 absence
ProspectiveSemanticFingerprint
EquipmentAuthoringContractRevision = 1
ReviewProposalDigest
```

`ReviewProposalDigest`는 operation kind, exact target, current/prospective fingerprint, contract revision과 blocking validation summary를 deterministic하게 결합한 digest다. Child reference 내부 byte는 넣지 않고 exact child canonical object path를 prospective fingerprint가 소유한다.

사용자가 `적용 및 저장`을 누르면 mutation 전에 fresh revalidation을 수행한다.

```text
Create
- exact target path canonical validation
- memory/disk target absence
- current complete identity namespace / duplicate check
- child existence/type/validation
- prospective validation
- prospective fingerprint recompute
- ReviewProposalDigest recompute + exact match

Update
- exact target class/path resolve
- package dirty == false
- current semantic fingerprint recompute + reviewed value exact match
- current complete identity namespace / duplicate check
- child existence/type/validation
- prospective validation
- prospective fingerprint recompute
- ReviewProposalDigest recompute + exact match
```

하나라도 다르면 `ReviewStale` 또는 해당 blocking diagnostic으로 mutation 0이다. Review 이후 Draft를 수정해도 기존 approval은 무효이며 fresh Review가 필요하다.

### 7.8 Product target path / naming / protection contract

Fresh AssetDump selection은 `/Game/CarFight` 아래 persisted `CFEquipmentPresetData` exact10 존재를 확인했다. 현재 managed evidence projection limitation 때문에 exact10의 개별 object path는 이 Gate에서 안전하게 projection하지 못했다. 따라서 특정 Product 하위 폴더를 existing canonical root라고 추측하지 않는다.

P0의 canonical target authority는 다음으로 동결한다.

```text
Product Create namespace
= user가 명시적으로 선택한 exact canonical `/Game/CarFight/.../Asset.Asset` object path

Narrow fixed Product sub-root
= 없음

Forbidden Product Create namespace
= `/Game/CarFight/Tests/**`
= EBA dedicated Automation root

Path validation
= CFDACommonPrimitives::ValidateTargetObjectPath와 동일한 canonical Unreal object-path 의미
```

즉 Product 위치는 Builder가 근거 없이 강제 이동하지 않는다. 대신 P0 Create dialog에서 사용자가 package folder와 asset name을 명시적으로 확정해야 한다.

Naming / identity 관계:

```text
Asset object/package name
≠ stable identity authority

Stable identity authority
= EquipmentId

Asset name과 EquipmentId equality 강제
= 0
```

Builder는 편의상 `EQ_<EquipmentId>` 형태를 **editable default suggestion**으로 제시할 수 있지만 correctness/duplicate 판단은 EquipmentId와 exact target path 계약만 사용한다. 실제 asset name은 Unreal package/object naming 규칙을 통과해야 한다.

보호 계약:

```text
- fresh AssetDump에서 확인된 existing persisted EquipmentPreset exact10은 기본 protected set
- Update는 사용자가 exact1 existing target을 명시적으로 선택한 경우만 허용
- Create는 memory package/object 또는 disk package가 이미 존재하면 mutation 0
- 다른 EquipmentPreset / child DataAsset / VehicleData mutation 0
- Product Create가 `/Game/CarFight/Tests/**`를 가리키면 차단
```

Automation disposable namespace는 P0 implementation test에서 `/Game/CarFight/Tests/EquipmentAuthoring/` 아래 exact dedicated target만 사용하며 Product target과 섞지 않는다. 각 mutation test는 자신이 만든 target만 cleanup하고 terminal residue exact0을 확인한다. 기존 `/Game/CarFight/Tests/Missile/EQ_Missile_DirectTest`는 historical/test evidence일 뿐 Product root나 EBA disposable target으로 재사용하지 않는다.

---

## 8. Gate Plan

### EBA-P0-00 — Contract Freeze + Vehicle Builder Reuse Baseline

목적:

```text
- Accepted Ownership Audit을 normative contract로 동결
- current Vehicle Builder implementation의 실제 재사용 후보를 fresh audit
- EquipmentPreset create/update/save authority 동결
- Draft / Review / durable `적용 및 저장` / stale guard 경계 동결
- validation hierarchy와 test surface 확정
```

이 Gate에서는 Product EquipmentPreset 구현/저장을 시작하지 않는다.

PASS 조건:

```text
P0 = 0
blocking P1 = 0
Ownership conflict = 0
Broad Vehicle Builder refactor required = 0
Parallel generic DA writer required = 0
```

### EBA-P0-00 Design Review — 2026-09-16

판정:

```text
P0: 0
blocking P1: 4
P2: 0
Result: HOLD
Product Source implementation: 0
Product Asset mutation: 0
Build / Automation: Not Run — design/document review only
Exact next: EBA-P0-00 Contract Correction + Re-review
```

Fresh review evidence:

```text
UCFEquipmentPresetData authored field: exact7
DataManagement identity: Required / ExplicitFName / EquipmentId
Duplicate namespace: ExactClassPath
DataManagement validation entry: HasCompleteEquipmentData
Persisted CFEquipmentPresetData under fresh /Game/CarFight AssetDump selection: exact10
Current CFDA production provider: MissileGuidePreset + AmmoData + DamageData + VehicleDefenseData exact4
Current shared durable authority: CFDADurableCore
Current Vehicle Builder shell: native Slate / 0.27:0.73 split / fixed context frame / common page scroll
Current Vehicle Mount compatibility seam: UCFEquipmentPresetData::CanUseOnMount
```

AssetDump exact10은 persisted population 존재를 확인하는 baseline으로만 사용한다. 이번 evidence projection에서는 개별 exact object path를 안전하게 읽지 못했으므로 Product canonical creation root를 추측하지 않는다. Source에서 확인된 `/Game/CarFight/Tests/Missile/EQ_Missile_DirectTest`는 Test asset evidence이며 Product root authority가 아니다.

#### P1-1 — Durable Create / Update / Save authority가 아직 동결되지 않음

현재 Plan §6은 `Explicit Apply → package dirty → Explicit Save`를 개념 흐름으로 두고 있으나, current Source에는 이미 `CFDADurableCore`가 provider-neutral typed durable authority로 존재한다.

`CFDADurableCore`는 다음을 단일 sequencing으로 소유한다.

```text
Create target memory/disk collision recheck
CreatePackage / NewObject
pre-save typed semantic readback
exact single-package SavePackage
post-save package clean / persisted existence 확인
exact package disk reload
persisted typed semantic fingerprint readback
Create pre-save failure cleanup
Update original typed snapshot
Update pre-save failure rollback
Save 호출 전 failure rollback
Save 호출 이후 outcome uncertainty 분류
```

따라서 Equipment Builder가 별도의 `SavePackage`, create cleanup, update rollback, reload/readback 엔진을 구현하면 기존 durable authority와 중복된다.

교정 조건:

```text
- EquipmentPreset durable execution authority = CFDADurableCore로 동결한다.
- Equipment 전용 코드는 typed payload / Materialize / Extract / Fingerprint adapter만 소유한다.
- Review 뒤 persisted mutation과 Save를 두 사용자 단계로 분리하지 않는다.
- P0 USER action은 `검토`와 `적용 및 저장`으로 분리한다.
- `적용 및 저장`은 fresh revalidation 뒤 exact1 EquipmentPreset target을 durable commit한다.
- Save 진입 전 실패는 current shared rollback/cleanup 계약으로 원상복구를 확인한다.
- Save가 호출된 뒤 결과를 확정할 수 없으면 성공으로 확대하지 않고 SaveStateUnconfirmed 계열로 fail-closed한다.
- automatic retry = 0이다.
```

P0 기본안은 Equipment-specific typed adapter가 shared durable core를 재사용하는 것이다. Interactive Equipment Builder를 구현한다는 이유만으로 CFDA JSON Staging production provider registry / DACE / mixed operational admission을 자동 exact5로 확장하지 않는다. 실제 API 결속상 full provider onboarding 없이는 shared durable authority를 안전하게 사용할 수 없다는 fresh blocker가 증명될 때만 그 범위를 별도 재검수한다.

#### P1-2 — Review / Approval / TOCTOU binding이 구체적이지 않음

현재 Plan은 Existing Asset stale mismatch fail-closed를 요구하지만 무엇에 binding하는지가 정의되지 않았다.

Current Vehicle Authoring은 `Preview → proposal hash → Commit 직전 fresh Preview → approval scope / current fingerprint/hash 재검사` 패턴을 이미 사용한다. Equipment Builder도 의미만 재사용하고 Vehicle Recipe/Resolver 계약 자체는 가져오지 않는다.

교정 조건:

```text
EquipmentPresetSemanticSnapshot
- EquipmentId
- DisplayName
- RequiredMountType
- RequiredWeaponSize
- DefaultTurretMountData object path
- DefaultWeaponData object path
- DefaultSensorData object path

Review binding
- operation kind: Create | Update
- exact target object path
- exact target class
- current semantic fingerprint (Update)
- prospective semantic fingerprint
- Builder authoring contract revision
- approved review/proposal digest
```

`DisplayName` fingerprint는 pointer/display cache가 아니라 deterministic FText semantic serialization을 사용한다. UObject reference는 pointer가 아니라 canonical object path를 사용한다.

Commit 직전에는 다음을 fresh 재실행한다.

```text
Create
- target package/object memory absence
- target package disk absence
- EquipmentId duplicate namespace check
- prospective validation
- approval digest match

Update
- exact target class/path readback
- current semantic fingerprint match
- EquipmentId duplicate namespace check
- prospective validation
- approval digest match
```

선택한 child asset의 내용 자체를 EquipmentPreset target fingerprint에 합성하지 않는다. Child asset은 write authority 밖이므로 reference path만 EquipmentPreset semantic에 포함하고, Commit 직전 child existence/type/native validation을 fresh 재실행한다.

#### P1-3 — EquipmentPreset authoring validation authority가 불완전하게 정의됨

Current DataManagement descriptor는 이미 다음 authority를 가진다.

```text
Class: /Script/CarFight_Re.CFEquipmentPresetData
Identity: Required / ExplicitFName / EquipmentId
Duplicate namespace: ExactClassPath
Validation policy: CustomContract
Validation entry: HasCompleteEquipmentData
```

그러나 `HasCompleteEquipmentData()`는 Weapon/Scanner payload shape만 검사한다. `EquipmentId` identity/duplicate, declared mount semantics, child native validation과 bounded Turret/Projectile checks까지 완결하지 않는다. 반대로 `UCFEquipmentPresetData`에 native `IsDataValid`가 현재 존재한다고 가정해서도 안 된다.

교정 조건:

```text
Layer 1 — Existing DataManagement identity
- EquipmentId non-None
- ExactClassPath duplicate namespace 재사용

Layer 2 — Existing EquipmentPreset runtime contract
- HasCompleteEquipmentData
- HasCompleteEquipmentDataForMount
- CanUseOnMount

Layer 3 — Child authority
- WeaponData native validation
- VehicleSensorData native validation
- TurretMountData / ProjectileData는 현재 존재하는 범위의 bounded structural check만 수행

Layer 4 — Builder policy
- Weapon/Scanner Draft Mode는 transient UI state일 뿐 persisted enum이 아님
- Scanner payload에서는 RequiredWeaponSize=None current contract 보존
- RequiredMountType=None은 current runtime wildcard 의미를 임의로 금지하지 않음
- DisplayName empty는 current runtime의 'public name unspecified' 의미이므로 Error로 승격하지 않음
```

Builder strict validator는 위 authority를 조합한 Editor-only orchestration으로 두며, 이 Feature 때문에 Runtime DataAsset에 새 validation framework나 불필요한 `IsDataValid` override를 추가하지 않는다.

#### P1-4 — New target canonical path / naming / protection 계약이 미동결

Fresh AssetDump에서 persisted EquipmentPresetData exact10을 확인했지만 이번 evidence projection으로 개별 Product path를 확정하지 못했다. Source에서 확인된 DirectTest preset은 Test root이므로 Product creation root를 대신할 수 없다.

신규 생성 구현 전에 다음을 동결해야 한다.

```text
- 현재 persisted Product EquipmentPreset path inventory
- canonical Product creation root
- asset naming convention
- EquipmentId ↔ object name 관계가 강제인지 독립인지
- package/object collision policy
- existing Product asset protected set
- Automation disposable root
- failed Create cleanup + residue exact0 조건
```

Product root와 naming은 추측으로 정하지 않는다. fresh persisted evidence와 기존 DataManagement naming/identity 정책을 교차검수해 결정한다.

#### Vehicle Builder reuse classification — Review Freeze

현재 fresh Source 기준 P0 재사용 분류는 다음으로 동결한다.

```text
A. Reuse As-Is / Existing Authority
- CarFight_ReEditor Nomad Tab + Window menu 등록 패턴
- Vehicle Builder의 0.27 / 0.73 page-shell UX 패턴
- fixed Target / Step / State / Summary / 지금 할 일 정보 구조
- common single vertical page scroll UX 패턴
- SObjectPropertyEntryBox asset selection 패턴
- UCFEquipmentPresetData::CanUseOnMount
- DataManagement Type Registry identity/duplicate semantics
- CFDADurableCore durable sequencing

B. Light Extraction
- P0-00에서는 exact0
- Vehicle Builder를 수정해 공용 UI framework를 먼저 만들지 않음

C. Equipment-specific New
- SCFEquipmentBuilderTab
- FCFEquipmentBuilderVM
- transient Equipment Draft / semantic snapshot
- Equipment authoring validation orchestration
- Review/proposal binding
- typed Materialize / Extract / Fingerprint adapter

D. Do Not Share
- FCFVehicleBuilderVM
- Vehicle Recipe / Resolver / Definition snapshot machinery
- Hardpoint / Mount authoring business state
- Builder companion creation service
- Vehicle apply / driving benchmark state
```

즉 UI는 Vehicle Builder와 같은 계열로 보이게 만들되, Vehicle Builder business code 상속이나 광범위 공용화는 하지 않는다.

#### Review conclusion

Ownership Audit 방향, Editor-only Native Slate 방향, EquipmentPresetData exact1 write scope는 유지 가능하다. 발견된 blocker는 모두 현재 Accepted infrastructure를 **덜 중복해서 재사용하기 위해 필요한 contract freeze 누락**이며 새로운 framework가 필요하다는 증거는 없다.

따라서 EBA-P0-01은 시작하지 않는다. 먼저 위 P1 exact4를 Plan에 교정하고 재검수해 P0/P1 0을 확인해야 한다.

### EBA-P0-00 Contract Correction + Re-review — 2026-09-16

교정된 normative contract는 §6 durable transaction, §7.0 strict prospective validation, §7.7 Review/TOCTOU binding, §7.8 target path/naming/protection이 소유한다.

P1 closure 판정:

```text
P1-1 Durable Create/Update/Save authority
→ CLOSED
→ CFDADurableCore single durable authority
→ Equipment local typed adapter only
→ CFDA production provider/DACE onboarding 0 required

P1-2 Review / Approval / TOCTOU
→ CLOSED
→ exact7 semantic fingerprint
→ Revision 1 + ReviewProposalDigest
→ Commit 전 current/prospective/child/identity fresh revalidation
→ stale = mutation0

P1-3 Strict authoring validation
→ CLOSED
→ current DataManagement identity/ExactClassPath semantics 재사용
→ current EquipmentPreset + Weapon + Sensor authority 조합
→ Turret/Projectile bounded checks only
→ RequiredMountType=None wildcard 의미 보존
→ DisplayName empty는 Error로 승격하지 않음

P1-4 Product target path / naming / protection
→ CLOSED
→ 특정 기존 Product 하위 폴더를 추측하지 않음
→ exact user-selected `/Game/CarFight/.../Asset.Asset`를 target authority로 동결
→ `/Game/CarFight/Tests/**` Product Create 금지
→ EquipmentId와 object name 독립
→ existing persisted exact10 default protected
→ dedicated Automation root + residue exact0
```

Fresh evidence limitation도 fail-open하지 않는다. Fresh AssetDump가 CFEquipmentPresetData exact10 population을 확인했지만 current managed evidence projection이 개별 object path를 노출하지 못했으므로 **고정 Product sub-root가 없다는 P0 계약**으로 정리했다. 이는 경로를 모른 채 새 canonical folder를 발명하는 대신 사용자가 exact target을 명시하고 core가 collision을 다시 검증하도록 하는 fail-closed 선택이다.

Re-review 판정:

```text
P0: 0
blocking P1: 0
P2: 0
Ownership conflict: 0
Broad Vehicle Builder refactor required: 0
Parallel generic DA writer required: 0
Product Source implementation: 0
Product Asset mutation: 0
Build / Automation: Not Run — contract/document-only correction
Result: Technical Contract PASS
Exact next: EBA-P0-01 Editor Shell / Browser / Draft Model
```

EBA-P0-00은 이 체크포인트로 종료한다. CF-FQ-054는 여전히 `Ready`이며 현재 단일 Active CF-FQ-039를 변경하지 않는다. 사용자가 CF-FQ-054 구현을 실제 착수할 때 EBA-P0-01부터 시작한다.

### EBA-P0-01 — Editor Shell / Browser / Draft Model

```text
- Equipment Builder Editor-only entry
- 기존/신규 EquipmentPreset 선택 UX
- transient Draft Model
- identity/basic mount fields
- persisted mutation 전 명확한 dirty/draft 상태
```

#### EBA-P0-01 Implementation + Fresh Validation — 2026-09-16

구현 범위:

```text
신규 Equipment-specific Source exact6
- Public/EquipmentAuthoring/CFEquipmentBuilderTypes.h
- Public/EquipmentAuthoring/CFEquipmentBuilderVM.h
- Public/EquipmentAuthoring/CFEquipmentBuilderTab.h
- Private/EquipmentAuthoring/CFEquipmentBuilderVM.cpp
- Private/EquipmentAuthoring/CFEquipmentBuilderTab.cpp
- Private/EquipmentAuthoring/CFEquipmentBuilderTests.cpp

기존 Editor module integration exact2
- Private/CarFightReEditor.cpp
- Public/CarFightReEditor.h

Build.cs mutation: 0
Vehicle Builder business Source mutation: 0
Runtime Source mutation: 0
```

구현 결과:

```text
Tab identity
= CarFight.EquipmentBuilder

Window menu
= Window > CarFight > CarFight 장비 제작 가이드

Shell
= Native Slate independent SCFEquipmentBuilderTab
= Vehicle Builder 계열 0.27 / 0.73 split UX
= fixed target/state/지금 할 일 context
= right page body common vertical SScrollBox exact1

Browser
= existing FCFDAManagementVM metadata-only inventory 재사용
= exact CFEquipmentPresetData class projection
= persisted list browsing 자체는 Asset load 0
= 사용자가 exact1 existing preset을 선택한 시점에만 read-only LoadObject

Draft
= FCFEquipmentBuilderVM session-local transient state
= existing exact7 authored field copy
= new draft는 persisted target/object 생성 0
= EquipmentId / DisplayName / RequiredMountType / RequiredWeaponSize edit
= Weapon / Scanner mode는 UI-only transient state
= Scanner mode RequiredWeaponSize=None current contract 보존
= RequiredMountType=None wildcard 의미 보존
= child reference는 현재 read-only 표시만 수행

Persisted mutation
= Apply action 0
= Save action 0
= SavePackage/CreatePackage/NewObject/Modify/MarkPackageDirty execution seam 0
= Product Asset mutation/save 0 in EBA scope
```

EBA-P0-01의 local `HasDraftChanges` 비교는 Editor presentation state일 뿐 EBA-P0-03의 Revision 1 semantic fingerprint / ReviewProposalDigest authority를 대체하지 않는다.

Fresh validation:

```text
Official UE 5.8 Build
- first real build: FAIL
- cause: focused test가 TSharedRef<SWidget>::IsValid()를 잘못 사용한 test-only compile defect
- correction: SNullWidget 비교로 교정
- final build job: 24ed44a335e84ee7a0e1e4e0a7418594
- result: PASS / exit 0

Focused Automation
- filter: CarFight.EquipmentAuthoring.P001
- execution: existing bounded DataAuthoring automation entry를 task-specific filter로 재사용
- process job: a81a62f029704c3d9ebee5f6d0083d37
- BrowserInventory: PASS
- DraftModel: PASS
- NativeSlateTab: PASS
- result: exact3 / 3 PASS / failure 0 / finding 0
```

Protection / concurrency note:

```text
- EBA Source/Automation에서 Product package create/save execution path는 확인되지 않음
- fresh post-validation Git status에서 시작 시 없던 UE/Content/CarFight/Vehicles/Data/Sensor/ untracked path가 별도로 관측됨
- Source 교차확인 결과 이 경로는 기존 Vehicle Authoring Project Basic Sensor 계약
  /Game/CarFight/Vehicles/Data/Sensor/DA_VehicleSensor_Basic.DA_VehicleSensor_Basic와 연결됨
- 세션 시작 시 별도 BasicSensor materialization script/cmdlet dirty가 이미 존재했으므로 concurrent unrelated work로 분류
- 해당 Product path는 EBA 결과로 귀속·수정·삭제하지 않고 그대로 보호
```

판정:

```text
EBA-P0-01: Technical PASS
P0: 0
blocking P1: 0
P2: 0
Official Build: PASS
Focused Automation: 3/3 PASS
EBA-scope Product Asset mutation/save: 0
USER Editor visual/UX acceptance: Not Run — EBA-P0-05 owner
Current Active CF-FQ-039: unchanged
Exact next: EBA-P0-02 Weapon / Scanner Guided Composition + Child Validation
```

### EBA-P0-02 — Weapon / Scanner Guided Composition + Child Validation

```text
- Weapon Draft: TurretMountData + WeaponData
- Scanner Draft: VehicleSensorData
- UI-only composition mode
- child reference / validation presentation
- invalid mixed payload fail-closed
```

#### EBA-P0-02 Implementation + Fresh Validation — 2026-09-16

이번 Gate는 EBA-P0-01에서 만든 독립 Equipment Builder 내부 exact6 Source만 확장했다.

```text
Modified EBA Source exact6
- Public/EquipmentAuthoring/CFEquipmentBuilderTypes.h
- Public/EquipmentAuthoring/CFEquipmentBuilderVM.h
- Public/EquipmentAuthoring/CFEquipmentBuilderTab.h
- Private/EquipmentAuthoring/CFEquipmentBuilderVM.cpp
- Private/EquipmentAuthoring/CFEquipmentBuilderTab.cpp
- Private/EquipmentAuthoring/CFEquipmentBuilderTests.cpp

CarFightReEditor module registration mutation: 0
Build.cs mutation: 0
Runtime Source mutation: 0
Product Content mutation: 0 in EBA scope
```

Guided composition 결과:

```text
Weapon Draft
- TurretMountData exact typed picker
- WeaponData exact typed picker
- selector success는 typed LoadObject + exact class check 뒤 transient SoftObject reference만 갱신
- Sensor selector는 Weapon mode에서 load 전에 fail-closed

Scanner Draft
- VehicleSensorData exact typed picker
- RequiredWeaponSize=None current contract 유지
- Weapon/Turret selector는 Scanner mode에서 load 전에 fail-closed

Mode transition
Weapon → Scanner
- transient TurretMountData clear
- transient WeaponData clear
- RequiredWeaponSize=None

Scanner → Weapon
- transient VehicleSensorData clear
- RequiredWeaponSize=None이면 Large baseline으로 복원

기존 persisted mixed payload
- LoadExistingPreset 단계에서 자동 정리하지 않음
- 현재 persisted 사실을 transient Draft에 보존
- validation에서 blocking Error로 표시
```

Child validation hierarchy:

```text
Layer 1 — Equipment package composition
- Weapon = TurretMountData + WeaponData + SensorData empty
- Scanner = VehicleSensorData only + Weapon/Turret empty + RequiredWeaponSize=None
- mixed / missing required child / Scanner size 위반 = blocking Error

Layer 2 — WeaponData native authority
- ValidateWeaponDataContract(...) 재사용
- native failure = blocking Error

Layer 3 — VehicleSensorData native authority
- ValidateSensorDataContract(...) 재사용
- native failure = blocking Error

Layer 4 — AmmoData native identity check
- IsAmmoDataValid() 재사용
- invalid AmmoData = blocking Error

Layer 5 — TurretMountData bounded check
- exact class/read + TurretMountId 관측
- 완결된 native validator가 없으므로 Limited/Warning만 사용
- 전체 visual/socket 유효함으로 확대하지 않음

Layer 6 — ProjectileData bounded check
- reference + ProjectileId 관측
- 완결된 native validator가 없으므로 Limited/Warning

Layer 7 — DamageData bounded check
- reference + DamageId 관측
- Equipment Builder가 기존 Data Authoring writer를 우회하지 않음
- 완결된 runtime native validator가 없으므로 Limited/Warning
```

UI presentation은 다음 severity를 구분한다.

```text
[통과]       = formal/native authority가 PASS했거나 package contract가 충족됨
[제한 검증]  = 현재 bounded fact만 확인 가능하며 전체 유효 판정 아님
[확인 필요]  = non-blocking missing/identity weakness
[오류]       = Apply/Save 전 반드시 해결해야 할 blocking failure
```

Concrete Vehicle MountProfile compatibility는 EBA-P0-04 owner이므로 이번 child validation에 포함하지 않았다. `RequiredMountType=None` wildcard semantics도 오류로 취급하지 않는다.

Mutation boundary:

```text
child DataAsset Modify: 0
child DataAsset Save: 0
EquipmentPreset Apply: 0
EquipmentPreset Save: 0
SavePackage execution call: 0
CreatePackage execution call: 0
NewObject execution call: 0
Modify execution call: 0
MarkPackageDirty execution call: 0
CFDADurableCore call: 0
RuntimeApply call: 0
```

Fresh validation:

```text
Final Official UE 5.8 Build
- build job: 881eecfcfa7b4c96beddba6939838e76
- target: CarFight_ReEditor Win64 Development
- result: PASS / exit 0

EBA-P0-02 focused contract exact5
- ExistingChildValidation: PASS
- MixedPayloadFailClosed: PASS
- ModeIsolation: PASS
- PersistedSelectorRoundTrip: PASS
- ScannerSizeContract: PASS
- result: 5/5 PASS

Affected EquipmentAuthoring regression
- process job: 6cfddbc20b1741cd821ba91fad5ad287
- P0-01 exact3 + P0-02 exact5
- result: exact8/8 PASS
- failure 0 / missing 0 / unexpected 0 / duplicate-terminal 0
```

`PersistedSelectorRoundTrip`은 실제 persisted EquipmentPreset graph에서 Weapon pair 또는 Scanner child reference를 읽고 새 transient Draft에 exact typed setter로 다시 선택하는 성공 경로를 검증한다. Product/child package는 수정·저장하지 않는다.

판정:

```text
EBA-P0-02 Implementation + Fresh Validation 당시 판정: Technical PASS
P0: 0
blocking P1: 0
P2: 0
Official UE 5.8 Build: PASS
Focused EBA-P0-02: 5/5 PASS
Affected EquipmentAuthoring: 8/8 PASS
EBA-scope Product Asset mutation/save: 0
Child DataAsset write/save: 0
USER Editor visual/UX acceptance: Not Run — EBA-P0-05 owner
당시 exact next: EBA-P0-03 Review / Durable Apply-and-Save
```

#### EBA-P0-02 Post-Implementation Mid-review — 2026-09-16

Fresh Source와 v0.1.3 baseline contract를 독립 재검수했다. 기존 Build PASS와 exact8 Automation PASS는 실행 결과로 보존하지만, 그 evidence가 현재 P0-02 계약 전체를 충분히 닫는지와 P0-03 durable mutation 전제에 안전한지를 별도로 판정했다.

검수 결과:

```text
P0: 0
blocking P1: 2
P2: 1 non-blocking
Verdict: HOLD
Product Source correction in this review: 0
Product Asset mutation/save in this review: 0
Build/Automation rerun: 0 — review-only, Source mutation 없음
EBA-P0-03 start: FORBIDDEN until P0/P1 0 re-review
Exact next: EBA-P0-02 Mid-review Correction + Re-review
```

##### P1-1 — intrinsic EquipmentPreset mount contradiction이 child validation에서 누락됨

Frozen Validation 계층은 Layer 2에 `RequiredMountType / RequiredWeaponSize / HasCompleteEquipmentData / HasCompleteEquipmentDataForMount`를 두고, Turret/Weapon bounded check 예시에 `선택한 Weapon/Equipment mount 의미와 명백한 모순 여부`를 포함한다. Concrete Vehicle MountProfile 선택이 필요한 호환성은 EBA-P0-04 owner지만, **프리셋 자체만으로 증명되는 불가능 조합**은 concrete 차량 없이도 판정 가능하다.

현재 `BuildValidationForDraft()`는 Weapon/Scanner payload shape와 Scanner `RequiredWeaponSize=None`은 검사하지만 `DraftMode + RequiredMountType + selected WeaponData mount contract`의 명백한 모순은 blocking하지 않는다.

대표 반례:

```text
Scanner Draft
- SensorData valid
- Weapon/Turret empty
- RequiredWeaponSize=None
- RequiredMountType=Turret

현재 Builder
→ package complete
→ blocking error 0 가능

현재 Runtime
→ Utility mount: payload shape는 맞아도 RequiredMountType=Turret 때문에 bMountTypeCompatible=false
→ Turret/그 외 non-Utility mount: HasCompleteEquipmentDataForMount가 Weapon payload를 요구하므로 false
→ 결과적으로 어느 mount에도 사용할 수 없는 preset
```

Weapon Draft의 `RequiredMountType=Utility`도 같은 이유로 어느 mount에도 사용할 수 없다. 또한 Weapon Draft가 explicit non-None/non-Utility `RequiredMountType`을 요구하는데 selected `WeaponData::SupportsMountType(RequiredMountType)`이 false라면 preset requirement와 child contract가 직접 모순된다.

교정 조건:

```text
- RequiredMountType=None wildcard는 계속 유효
- Scanner Draft는 RequiredMountType=None 또는 Utility만 intrinsic-valid
- Weapon Draft에서 RequiredMountType=Utility는 blocking Error
- Weapon Draft에서 explicit non-None/non-Utility RequiredMountType이면 selected WeaponData가 그 mount type을 지원하는지 bounded blocking check
- concrete Vehicle MountProfile / SizeLimit 판정은 계속 EBA-P0-04 owner
- 이 교정을 이유로 VehicleData/Fitting/RuntimeApply authority를 가져오지 않음
```

이 항목은 P0-03의 Review/Apply readiness가 현재 validation 결과를 신뢰하게 되기 전에 닫혀야 하므로 blocking P1이다.

##### P1-2 — focused Automation이 Weapon/Scanner 두 principal success path를 결정적으로 증명하지 못함

현재 `PersistedSelectorRoundTrip`은 persisted EquipmentPreset 목록을 순회하다가 **처음 발견한** 완성 Weapon pair 또는 Scanner child 하나를 성공적으로 typed setter에 넣으면 종료한다. 따라서 exact8 PASS여도 그 실행에서 Weapon success와 Scanner success가 모두 검증됐다는 보장은 없다.

또한 `ExistingChildValidation`은 첫 persisted preset을 load한 뒤 `Validation.Items.Num() > 0`만 확인한다. 이는 validation code가 실행됐다는 smoke evidence이지, 해당 persisted graph의 expected `bPackageComplete / bHasBlockingErrors / native child result`가 정확하다는 acceptance evidence는 아니다.

교정 조건:

```text
- Weapon guided composition success path를 deterministic하게 별도 증명
  - TurretMountData typed selection success
  - WeaponData typed selection success
  - expected package/native result 확인

- Scanner guided composition success path를 deterministic하게 별도 증명
  - VehicleSensorData typed selection success
  - RequiredWeaponSize=None
  - expected package/native result 확인

- 최소 한 개의 native child invalid/failure propagation이 Error → bHasBlockingErrors=true로 연결됨을 deterministic하게 증명
- 테스트를 위해 Product/child package를 저장하지 않음
- persisted fixture를 쓸 경우 exact intended fixture를 선택하고 browser ordering에 acceptance 의미를 의존하지 않음
```

이 항목은 구현 자체가 틀렸다는 판정이 아니라 **Technical PASS evidence가 두 핵심 경로를 완결하지 못한 검증 공백**이다. P0-03에 durable mutation을 추가하기 전에 acceptance surface를 결정적으로 고정해야 하므로 blocking P1이다.

##### P2-1 — Draft Mode 전환이 opposite payload selection을 즉시 소거함

현재 `SetDraftMode()`는 mixed payload 생성을 막기 위해 다음을 즉시 수행한다.

```text
Weapon → Scanner
- TurretMountData clear
- WeaponData clear
- RequiredWeaponSize=None

Scanner → Weapon
- SensorData clear
- RequiredWeaponSize=None이면 Large로 변경
```

persisted Asset에는 영향이 없고 existing preset은 Reset으로 source 복구가 가능하므로 P0/P1은 아니다. 다만 신규 Draft에서 사용자가 실수로 mode를 바꾸면 기존 선택값을 복원할 source가 없고, destructive effect 안내는 전환 **후** status message로만 나온다.

비차단 교정 후보:

```text
- opposite payload가 이미 선택된 경우 전환 전 명확한 확인 UX
또는
- transient mode별 selection cache를 두되 최종 semantic Draft에는 active mode payload만 projection
```

범위를 키우지 않기 위해 EBA-P0-02 Correction에서 함께 고치거나 EBA-P0-05 USER UX 전에 명시적으로 닫을 수 있다.

보존 확인:

```text
WeaponData native validator 재사용: 정상
VehicleSensorData full native validator 재사용: 정상
AmmoData identity validation 재사용: 정상
Turret/Projectile/Damage를 full-valid로 과장하지 않음: 정상
existing mixed persisted payload 자동 mutation 없음: 정상
SavePackage/CreatePackage/NewObject/Modify/MarkPackageDirty execution: 0
CFDADurableCore/RuntimeApply call: 0
child DataAsset direct write/save: 0
```

#### EBA-P0-02 Mid-review Correction + Source Re-review — 2026-09-16

blocking P1 2건과 non-blocking P2 1건을 EquipmentAuthoring exact scope 안에서 교정했다. VehicleData/Fitting/RuntimeApply ownership, persisted Equipment schema, child DataAsset writer와 durable save authority는 확장하지 않았다.

교정 결과:

```text
P1-1 intrinsic mount contradiction
- RequiredMountType=None wildcard 보존
- Scanner: None 또는 Utility만 intrinsic-valid
- Scanner + Turret/Fixed 등 explicit non-Utility → blocking Error
- Weapon + Utility → blocking Error
- Weapon + explicit non-None/non-Utility mount가 WeaponData::SupportsMountType() false → blocking Error
- concrete Vehicle MountProfile / SizeLimit 판정은 계속 EBA-P0-04 owner

P1-2 deterministic focused acceptance surface
- WeaponGuidedSuccess 신규
  - transient TurretMountData exact typed selection
  - transient WeaponData exact typed selection
  - package complete + no blocking + Weapon native PASS 기대
- ScannerGuidedSuccess 신규
  - transient VehicleSensorData exact typed selection
  - RequiredWeaponSize=None
  - package complete + no blocking + Sensor native PASS 기대
- NativeChildFailurePropagation 신규
  - transient SensorConfig.UpdateIntervalSec=0 native invalid fixture
  - typed selection은 성공하되 validation Error + bHasBlockingErrors=true 기대
- IntrinsicMountContradiction 신규
  - Scanner/Turret
  - Weapon/Utility
  - Weapon/unsupported explicit Fixed
  exact3 contradiction을 deterministic하게 blocking 기대

P2-1 mode 전환 transient selection loss
- 별도 persisted field나 generic mode framework를 추가하지 않음
- VM session-local mode별 cache만 추가
- active semantic Draft에는 active mode payload만 projection
- Weapon → Scanner → Weapon 시 이전 Turret/Weapon/Size 복원
- Scanner → Weapon → Scanner 시 이전 Sensor 복원
- Product Asset / child Asset mutation 0
```

Source re-review:

```text
P0: 0
blocking P1: 0
P2: 0
Verdict: SOURCE CORRECTION PASS
SavePackage/CreatePackage/MarkPackageDirty/Modify execution: 0
CFDADurableCore call: 0
RuntimeApply call: 0
Child DataAsset direct write/save: 0
EBA-P0-03 implementation: 0
```

Fresh Official UE 5.8 Build는 allowlisted `carfight.editor.development` / `main_game`으로 실행했다.

```text
Build job: 937a8e0bd2ea4cf58be04d948484da60
Target: CarFight_ReEditor Win64 Development
EBA compile evidence:
- CFEquipmentBuilderTests.cpp: COMPILE PASS
- CFEquipmentBuilderVM.cpp: COMPILE PASS
- CFEquipmentBuilderTab.cpp: COMPILE PASS

Final link: FAIL / exit 6
External blocker:
- UCFSUVMigrateCommandlet::UCFSUVMigrateCommandlet() unresolved external
- UCFSUVMigrateCommandlet::Main(...) unresolved external
- CFSUVMigrateCmdlet.h는 current Git worktree diff 0
- current Source search에서 UCFSUVMigrateCommandlet 구현 .cpp / method definition 0
```

`CFSUVMigrateCmdlet.h`는 `TestSUV`/Basic Sensor exact2를 다루는 별도 임시 migration commandlet이며 Equipment Authoring ownership 밖이다. 이번 Gate가 그 구현을 임의 생성하거나 삭제하면 기존 병렬 작업을 침범하므로 수정하지 않았다.

Fresh Automation은 **실행하지 않았다**. 최종 Editor DLL이 current correction Source로 link되지 않았으므로 기존 binary를 실행하면 stale evidence가 되어 acceptance 근거가 될 수 없기 때문이다.

따라서 해당 시점의 gate 판정은 다음과 같았다.

```text
EBA-P0-02 correction source re-review: P0 0 / blocking P1 0 / P2 0
Fresh Official Build: BLOCKED by unrelated linker defect after EBA compile PASS
Fresh Focused P002 expected surface: exact9 / Not Run
Fresh Affected EquipmentAuthoring expected surface: P001 exact3 + P002 exact9 = exact12 / Not Run
당시 Overall EBA-P0-02 Technical Re-acceptance: HOLD / VALIDATION BLOCKED
당시 EBA-P0-03: MUST NOT START
당시 exact next: resolve owning CFSUVMigrateCommandlet linker blocker outside EBA scope, then rerun Official Build → P002 exact9 → affected exact12
```

#### EBA-P0-02 Fresh Validation Recovery + Final Re-review — 2026-09-16

외부 linker blocker 관측 뒤 owning 병렬 작업에서 `UE/Source/CarFight_ReEditor/Private/CFSUVMigrateCmdlet.cpp`의 `UCFSUVMigrateCommandlet` constructor/Main 구현이 fresh Source에 나타난 것을 확인했다. EBA 범위에서는 해당 파일을 수정하지 않았고, blocker 원인 변화가 실제로 증명된 뒤에만 Official Build를 재실행했다.

Fresh validation 최종 결과:

```text
Official UE 5.8 Build
- final build job: 59cc64cd607f4dd1b86dc453883eae92
- target: CarFight_ReEditor Win64 Development
- result: PASS / exit 0

Focused EBA-P0-02
- process job: 1492118763204e42aa1f4b98a928c4f0
- filter: CarFight.EquipmentAuthoring.P002
- result: exact9 / 9 PASS
- failure 0 / missing 0 / unexpected 0 / duplicate-terminal 0
- ExistingChildValidation: PASS
- IntrinsicMountContradiction: PASS
- MixedPayloadFailClosed: PASS
- ModeIsolation: PASS
- NativeChildFailurePropagation: PASS
- PersistedSelectorRoundTrip: PASS
- ScannerGuidedSuccess: PASS
- ScannerSizeContract: PASS
- WeaponGuidedSuccess: PASS

Affected EquipmentAuthoring
- process job: bf36df7a136c4a4faa0fbd2435c7902c
- filter: CarFight.EquipmentAuthoring
- P0-01 exact3 + P0-02 exact9
- result: exact12 / 12 PASS
- failure 0 / missing 0 / unexpected 0 / duplicate-terminal 0
```

최종 independent re-review:

```text
P0: 0
blocking P1: 0
P2: 0
Verdict: Technical PASS / RE-ACCEPTED
Official UE 5.8 Build: PASS
Focused EBA-P0-02: exact9/9 PASS
Affected EquipmentAuthoring: exact12/12 PASS
EBA-scope Product Asset mutation/save: 0
Child DataAsset write/save: 0
SavePackage/CreatePackage/MarkPackageDirty/Modify execution: 0
CFDADurableCore call: 0
RuntimeApply call: 0
EBA-P0-03 implementation: 0
USER Editor visual/UX acceptance: Not Run — EBA-P0-05 owner
Current Active CF-FQ-039: unchanged
Exact next: EBA-P0-03 Review / Durable Apply-and-Save
```

초기 `937a8e0bd2ea4cf58be04d948484da60` link failure는 당시 실제 외부 blocker였으므로 Historical evidence로 보존한다. 최종 PASS는 blocker 구현이 fresh Source에서 확인된 이후 별도 Official Build `59cc64cd607f4dd1b86dc453883eae92`와 fresh Automation으로 다시 확보했으며, 이전 실패를 성공으로 재해석하지 않는다.

#### EBA-P0-03 Pre-Implementation Design Review — 2026-09-16

`EquipmentAuthoringPlan.md v0.1.7`의 Revision 1 semantic snapshot, ReviewProposalDigest, Create/Update fresh TOCTOU, identity/child fresh revalidation, `CFDADurableCore` durable authority와 `SaveStateUnconfirmed` 계약을 actual Source에 교차검수했다. 이번 검수는 durable mutation 구현 전에 수행한 read-only contract review이며 Source/Product Asset mutation은 0이다.

Accepted foundation:

```text
Revision 1 authored semantic snapshot exact7: PASS
- EquipmentId
- DisplayName Literal FText semantic
- RequiredMountType
- RequiredWeaponSize
- DefaultTurretMountData canonical object path
- DefaultWeaponData canonical object path
- DefaultSensorData canonical object path

CFDACommonPrimitives semantic primitive 재사용: PASS
- FName canonical semantic
- Literal FText lossless readback
- canonical target/object path
- deterministic SHA-256

CFDADurableCore provider-neutral typed reuse: PASS
- Create memory/disk collision precheck
- Update dirty-package fail-closed
- pre-save typed semantic readback
- Create cleanup / Update rollback
- exact single-package SavePackage
- disk reload + exact object-path re-resolve
- persisted typed fingerprint confirmation
- Save 진입 이후 확인 실패 = SaveStateUnconfirmed
- automatic retry 0

DataManagement Equipment identity descriptor: PASS for persisted inventory
- Required / ExplicitFName / EquipmentId / ExactClassPath

CFDA production provider exact5 / DACE / JSON Staging onboarding requirement: 0
```

그러나 durable mutation을 허용하기 전에 다음 blocking P1 exact4를 contract에서 닫아야 한다.

**P1-1 — ReviewProposalDigest canonical validation binding 미동결**

현재 `FCFEquipmentDraftValidation`은 `Pass/Limited/Warning/Error`, 사용자 표시 `Title`, `Message`, `bHasBlockingErrors`만 가지며 approval용 stable diagnostic code/token이 없다. 따라서 Plan의 `blocking validation summary`를 현재 그대로 digest에 넣으면 localized 사용자 문구 또는 배열 순서가 durable approval authority가 될 수 있다.

Correction requirement:

```text
ReviewProposalDigest는 UI 문자열을 hash하지 않는다.

Revision 1 canonical digest binding은 최소 다음 exact token을 포함한다.
- OperationKind = Create | Update
- TargetObjectPath
- TargetClassPath = /Script/CarFight_Re.CFEquipmentPresetData
- CurrentSemanticFingerprint = Update exact value | Create <absent-target>
- ProspectiveSemanticFingerprint
- EquipmentAuthoringContractRevision = 1
- stable blocking-validation token set 또는 canonical Ready token

blocking token은 stable code 기반 + deterministic sort
Title/Message/localization text는 digest 제외
blocking error가 0인 proposal만 Reviewed 상태로 승격 가능
```

**P1-2 — prospective EquipmentId uniqueness가 loaded-but-unregistered exact-class object를 누락**

`FCFDAHealthService::ValidateAssets()`의 ExactClassPath duplicate closure는 complete DataManagement inventory 안의 `AssetRecords`만 확장한다. 이는 persisted/registered candidate coverage에는 적합하지만, 다른 object path에 살아 있는 loaded-but-unregistered `/Game/` `UCFEquipmentPresetData`가 같은 `EquipmentId`를 가진 경우를 현재 prospective guard만으로는 배제하지 못한다. `CFDADurableCore`의 Create memory collision은 exact target path collision만 막으므로 다른 path의 duplicate identity를 대신 막지 않는다.

Accepted Data Authoring current resolver는 같은 종류의 gap을 loaded `/Game/` object + AssetRegistry union으로 닫고 있으므로 EBA도 같은 fail-closed 의미를 가져야 한다.

Correction requirement:

```text
prospective identity authority
= complete DataManagement exact-class inventory closure
+ loaded-only exact UCFEquipmentPresetData /Game/ candidate closure

loaded-only candidate 조건
- exact UCFEquipmentPresetData class
- non-transient
- non-CDO / non-archetype
- valid canonical /Game/ object path
- inventory에 이미 represented된 path는 중복 계산하지 않음

identity 비교
- EquipmentId FName semantic equality
- Update는 selected exact target path 자신만 제외
- path/class/EquipmentId를 안전하게 확정하지 못한 candidate 존재 시
  => IdentityUniquenessUnconfirmed / mutation0
```

이를 위해 DataManagement 전체를 새 generic framework로 확장할 필요는 없다. Equipment-specific bounded fresh orchestration으로 current semantics를 보완하는 것이 P0 기본안이다.

**P1-3 — dirty/unsaved Child dependency가 durable approval을 통과할 수 있음**

Revision 1 EquipmentPreset fingerprint는 의도적으로 child 내부 byte를 포함하지 않고 canonical child object path만 포함한다. 그런데 현재 EBA-P0-02 child validation은 loaded in-memory UObject를 검증하면서 child package dirty 여부를 blocking하지 않는다. 이 상태에서 EquipmentPreset만 저장하면 approval이 unsaved child state에 근거할 수 있고, 이후 child dirty가 discard되면 persisted EquipmentPreset이 Review 당시 검증한 dependency state와 달라질 수 있다. Stop Rule상 Equipment Builder가 child를 암묵적으로 저장하는 것도 금지다.

Correction requirement:

```text
Review와 Commit fresh validation에서
현재 readiness 판단에 사용하는 모든 non-null child dependency package는 clean이어야 한다.

Weapon chain
- TurretMountData
- WeaponData
- non-null DefaultAmmoData
- non-null DefaultProjectileData
- non-null DefaultDamageData

Scanner chain
- VehicleSensorData

child package dirty 또는 persisted/current truth 불확정
=> ChildStateUnconfirmed / mutation0
=> Equipment Builder는 child를 저장하지 않음
=> 사용자가 owning authoring/editor flow에서 save/discard한 뒤 fresh Review 필요
```

Turret/Projectile/Damage의 validation maturity를 full-valid로 확대하지 않는 기존 경계는 유지한다. clean requirement는 dependency truth 확정 조건이지 신규 native validator가 아니다.

**P1-4 — one-shot approval consumption / manual retry0 / immediate TOCTOU lifecycle 미동결**

Plan은 Draft edit 시 approval invalidation과 automatic retry0를 요구하지만, Equipment approval의 consumption timing이 아직 normative하게 고정되지 않았다. 같은 Reviewed approval을 stale preflight 실패나 `SaveStateUnconfirmed` 뒤 다시 누를 수 있으면 사실상 old approval retry가 가능하다.

Correction requirement:

```text
approval lifecycle
Unreviewed
→ Reviewed
→ ApplyAttempted
→ Consumed

- `검토` 성공 시에만 Reviewed 생성
- 첫 `적용 및 저장` 진입 즉시 old approval은 one-shot ApplyAttempted
- fresh preflight 실패 / pre-save known failure / DurableApplied / SaveStateUnconfirmed를 포함해
  terminal outcome 뒤 old approval은 반드시 Consumed
- 두 번째 apply는 fresh `검토` 없이는 불가
- Draft/target 변경은 Reviewed approval 즉시 무효화
- SaveStateUnconfirmed는 same-approval manual retry도 금지하고 state inspection/refresh 후 fresh Review 요구

TOCTOU sequencing
- final target/current fingerprint/identity/child/validation/digest revalidation
- 위 결과로 exact fresh common row 구성
- UI pump / latent task / async callback / user prompt 없이
- 같은 synchronous no-yield call path에서 즉시 CFDADurableCore::ApplyTypedTarget 호출
```

shared `CFDAStagingApply`의 one-shot approval / immediate revalidation 패턴은 semantic reference로 사용할 수 있지만 Equipment Builder가 Staging provider registry/JSON workflow를 소유하거나 복제할 필요는 없다.

Design Review verdict:

```text
P0: 0
blocking P1: 4
P2: 0
Verdict: HOLD / CONTRACT CORRECTION REQUIRED

EBA-P0-03 durable mutation implementation: 0
EquipmentAuthoring Source mutation in this review: 0
Product Asset mutation/save: 0
Child DataAsset mutation/save: 0
CFDADurableCore execution: 0
Build/Automation: Not Run — document-only pre-implementation review
Current Active CF-FQ-039: unchanged
Exact next: EBA-P0-03 Contract Correction + Re-review
```

P0/P1 0 재검수 전 `검토`/`적용 및 저장` durable implementation, Product test mutation, `CFDADurableCore::ApplyTypedTarget` 호출을 시작하지 않는다.

#### EBA-P0-03 Contract Correction + Re-review — 2026-09-16

Pre-Implementation Design Review의 blocking P1 exact4를 actual Source precedent와 current EquipmentAuthoring boundary 안에서 교정했다. 이번 Gate는 **계약 동결과 독립 재검수만** 수행하며 durable mutation Source 구현은 시작하지 않는다.

##### Correction 1 — ReviewProposalDigest Revision 1 canonical binding

Review approval의 durable authority는 사용자 표시용 validation 문구가 아니라 semantic proposal 자체다. Current `FCFEquipmentDraftValidation`의 `Title` / `Message` / item physical order는 localization·UX presentation이며 digest authority가 아니다.

Revision 1 `ReviewProposalDigest`는 shared deterministic token framing + canonical SHA-256 의미를 사용해 다음 exact7 binding token을 순서 고정으로 hash한다.

```text
1. OperationKind
   = Create | Update

2. TargetObjectPath
   = CFDACommonPrimitives::ValidateTargetObjectPath를 통과한 canonical exact object path

3. TargetClassPath
   = /Script/CarFight_Re.CFEquipmentPresetData
   = case-sensitive exact constant

4. CurrentSemanticFingerprint
   Update = review 시점 persisted/current exact canonical fingerprint
   Create = literal stable sentinel `AbsentTarget`

5. ProspectiveSemanticFingerprint
   = Revision 1 exact7 prospective semantic의 canonical SHA-256

6. EquipmentAuthoringContractRevision
   = 1

7. ValidationState
   = literal stable token `Ready`
```

`ValidationState=Ready`는 다음 조건을 모두 통과한 proposal에만 생성한다.

```text
- exact7 semantic snapshot/fingerprint 성공
- strict prospective validation의 package completeness PASS
- blocking validation error 0
- prospective EquipmentId uniqueness confirmed
- Review에 사용되는 child dependency persisted/current truth confirmed
```

하나라도 blocking이면 `ReviewProposalDigest`와 Reviewed approval 자체를 생성하지 않는다. 따라서 blocking diagnostic code 목록을 digest authority로 복제하지 않는다.

다음 값은 digest에서 명시적으로 제외한다.

```text
FCFEquipmentValidationItem.Title
FCFEquipmentValidationItem.Message
localized 사용자 문자열
Pass / Limited / Warning presentation row의 physical order
UI section/step text
```

Warning/Limited는 기존 UX 의미를 유지하되 blocking이 아니므로 `ValidationState=Ready` 여부를 바꾸지 않는다. 반대로 Error, identity unconfirmed, child state unconfirmed는 Review 승격을 차단한다.

Commit 직전 fresh truth에서 위 exact7 digest를 같은 token framing으로 다시 계산해 reviewed digest와 case-sensitive exact 일치해야 한다. 불일치하면 `ReviewStale`, mutation 0이다. TargetClassPath가 다른 경우도 digest mismatch 이전에 exact-class preflight에서 fail-closed한다.

이 correction은 generic validation-code framework를 추가하지 않는다. Revision 1에서 durable approval 가능한 유일한 validation terminal state가 `Ready`이므로 stable Ready token 하나가 authority다.

##### Correction 2 — prospective EquipmentId complete identity closure

Review와 Commit fresh preflight의 EquipmentId uniqueness authority는 다음 union closure로 동결한다.

```text
A. complete DataManagement exact-class inventory
+
B. loaded-but-unregistered exact UCFEquipmentPresetData /Game/ objects
```

A의 전제:

```text
- FCFDAManagementVM current inventory IsInventoryComplete() == true
- DataManagement descriptor = Required / ExplicitFName / EquipmentId / ExactClassPath
- persisted candidate class = exact /Script/CarFight_Re.CFEquipmentPresetData
- Asset Registry loading 중이면 uniqueness confirmed를 주장하지 않음
```

B는 current Ammo provider의 accepted `TObjectIterator + AssetRegistry` union semantics를 Equipment-specific bounded orchestration으로 재사용한다.

loaded candidate inclusion 조건:

```text
- TObjectIterator<UCFEquipmentPresetData>로 관측 가능
- GetClass() == UCFEquipmentPresetData::StaticClass() exact class
- RF_ClassDefaultObject 없음
- RF_ArchetypeObject 없음
- RF_Transient 없음
- canonical valid `/Game/.../Asset.Asset` top-level object path
- EquipmentId를 FName semantic으로 확정 가능
```

같은 object path가 A와 B 양쪽에 보이면 path set으로 exact1만 계산한다. identity 비교는 문자열 case-sensitive 신규 규칙이 아니라 current `FName` semantic equality를 사용한다.

```text
Create
- 모든 candidate와 Draft EquipmentId 비교

Update
- exact selected TargetObjectPath 자신만 comparison에서 제외
- 같은 EquipmentId의 다른 path는 Duplicate
```

다음 중 하나라도 발생하면 `IdentityUniquenessUnconfirmed`, mutation 0이다.

```text
- DataManagement inventory incomplete
- Asset Registry loading / coverage 불확정
- candidate exact class/path/EquipmentId를 확정하지 못함
- canonical path validation 실패 candidate가 uniqueness 범위에 존재
```

동일 EquipmentId가 다른 exact-class path에서 발견되면 `Duplicate`, mutation 0이다. 이 closure는 Review와 Commit fresh preflight에서 모두 다시 실행한다.

DataManagement 자체를 새 generic identity framework로 변경하지 않는다. Equipment Builder가 prospective Draft 때문에 필요한 loaded-only gap만 bounded helper로 보완한다.

##### Correction 3 — child persisted/current truth fail-closed

Revision 1 EquipmentPreset fingerprint는 child 내부 semantic을 합성하지 않고 canonical child object path만 bind한다. 따라서 Review와 Commit fresh validation에 사용되는 child는 **persisted/current truth가 확정된 clean dependency**여야 한다.

각 non-null dependency는 validation 전에 다음을 모두 만족해야 한다.

```text
1. canonical child object path 확정
2. exact expected class로 resolve/load 성공
3. Asset Registry에 같은 exact object path/class persisted metadata 존재
4. Asset Registry loading 아님
5. UObject / outermost package가 transient 아님
6. outermost package != null
7. outermost package IsDirty() == false
```

Weapon dependency closure:

```text
DefaultTurretMountData
DefaultWeaponData
DefaultWeaponData->DefaultAmmoData        when non-null
DefaultWeaponData->DefaultProjectileData  when non-null
DefaultProjectileData->DefaultDamageData  when non-null
```

Scanner dependency closure:

```text
DefaultSensorData
```

위 persisted/current truth 확인 뒤에만 기존 native-or-bounded validation을 실행한다.

```text
WeaponData = existing native validation
AmmoData = existing identity/native validation
VehicleSensorData = existing native validation
TurretMountData / ProjectileData / DamageData = 기존 bounded limited validation만 유지
```

Turret/Projectile/Damage를 full-valid로 승격하지 않는다. clean/persisted requirement는 validator maturity가 아니라 Review가 의존한 dependency truth를 durable save와 일치시키는 precondition이다.

child package dirty, loaded-only/unregistered, exact class/path mismatch, registry uncertainty 중 하나라도 있으면 `ChildStateUnconfirmed`, blocking mutation 0이다. Equipment Builder는 child를 save/discard하지 않는다. 사용자가 해당 child의 owning editor/authoring flow에서 save 또는 discard를 완료한 뒤 fresh `검토`를 수행해야 한다.

##### Correction 4 — one-shot approval / retry0 / immediate no-yield TOCTOU

Equipment approval lifecycle을 shared Staging Apply의 accepted one-shot 의미와 동일하게 동결한다.

```text
Unreviewed
→ Reviewed
→ ApplyAttempted
→ Consumed
```

상태 계약:

```text
- 성공한 `검토`만 Reviewed approval을 생성
- Draft semantic / target path / operation kind 변경 시 Reviewed → Unreviewed 즉시 무효화
- `적용 및 저장` handler는 Reviewed exact1을 확인한 직후 ApplyAttempted로 전환
- ApplyAttempted 이후 terminal path는 성공/실패와 무관하게 Consumed
- Consumed approval 재사용 금지
- Reviewed 없이 Apply 클릭 시 mutation 0 / fresh Review 요구
```

첫 Apply attempt의 fresh preflight는 다음 exact 순서의 같은 synchronous call path에서 수행한다.

```text
1. operation kind / target canonical path / exact TargetClassPath
2. Create target absence 또는 Update exact current target + dirty guard
3. Update CurrentSemanticFingerprint 또는 Create AbsentTarget sentinel 재확인
4. complete prospective EquipmentId identity union closure
5. child persisted/current truth closure
6. strict native-or-bounded prospective validation
7. ProspectiveSemanticFingerprint fresh 재계산
8. ReviewProposalDigest fresh 재계산 / exact reviewed digest 비교
9. exact fresh CFDADurableCore row 구성
10. approval을 Consumed로 전환
11. CFDADurableCore::ApplyTypedTarget 즉시 호출
```

8번 final approval revalidation부터 11번 core 호출까지 다음을 허용하지 않는다.

```text
UI event pump
latent action
task scheduling
async callback
user prompt / confirmation dialog
yield 후 재진입
```

즉 final fresh revalidation과 durable core 호출은 같은 stack의 **immediate no-yield path**다.

fresh preflight가 어느 단계에서 실패해도 ApplyAttempted approval을 Consumed로 만들고 mutation 0으로 종료한다. core 진입 뒤 `DurableApplied`, known pre-save failure, `InMemoryStateUnconfirmed`, `SaveStateUnconfirmed` 등 어떤 terminal result도 approval을 되살리지 않는다.

특히 `SaveStateUnconfirmed`는 same-approval manual retry도 금지한다.

```text
SaveStateUnconfirmed
→ automatic retry 0
→ same approval retry 0
→ exact target/disk/current state를 다시 inspect/refresh
→ 사용자 fresh `검토`
→ 새 Reviewed approval만 새 Apply 가능
```

shared Staging Apply의 `Reviewed → ApplyAttempted → Consumed before mutation` pattern은 semantic precedent로만 재사용한다. Equipment Builder가 Staging JSON/provider registry/batch workflow를 복제하지 않는다.

##### Correction Re-review verdict

actual `FCFEquipmentDraftValidation`, Ammo provider의 loaded UObject + AssetRegistry identity union, shared Staging Apply one-shot state machine, `CFDADurableCore` terminal semantics에 다시 대조했다.

```text
P0: 0
blocking P1: 0
P2: 0
Verdict: TECHNICAL CONTRACT PASS / RE-ACCEPTED

P1-1 ReviewProposalDigest canonical Ready token + TargetClassPath: CLOSED
P1-2 loaded-but-unregistered EquipmentPreset identity closure: CLOSED
P1-3 dirty/unpersisted child dependency fail-closed: CLOSED
P1-4 one-shot approval / retry0 / immediate no-yield TOCTOU: CLOSED

EBA-P0-03 durable mutation implementation: 0
EquipmentAuthoring Source mutation in this correction gate: 0
Product Asset mutation/save: 0
Child DataAsset mutation/save: 0
CFDADurableCore execution: 0
Build/Automation: Not Run — document-only contract correction + re-review
Prior EBA-P0-02 Build/focused9/affected12 evidence: preserved historical baseline only
Current Active CF-FQ-039: unchanged
Exact next: EBA-P0-03 Review / Durable Apply-and-Save Implementation + Fresh Validation
```

Implementation Gate에서 최소 다음 regression behavior를 증명한다.

```text
- digest는 Title/Message/localization/item order와 독립
- TargetClassPath 또는 reviewed digest mismatch는 mutation0
- loaded-only duplicate EquipmentId는 mutation0
- dirty 또는 unpersisted child dependency는 mutation0
- stale fresh preflight failure 뒤 approval consumed
- SaveStateUnconfirmed 뒤 approval consumed + same-approval retry0
- final fresh preflight 뒤 immediate core call 이전 yield seam 0
```

이 재검수로 EBA-P0-03 durable implementation 착수 조건은 열렸지만, **이번 Contract Correction Gate 자체에서는 durable implementation을 시작하지 않았다.**

### EBA-P0-03 — Review / Durable Apply-and-Save

```text
- Before / After
- Revision 1 semantic snapshot / ReviewProposalDigest
- `적용 및 저장` readiness
- Create / Update distinction
- Commit 직전 fresh TOCTOU / identity / child / validation recheck
- CFDADurableCore exact EquipmentPreset durable mutation
- exact single-package Save + disk reload + persisted semantic readback
- stale/invalid/dirty/collision mutation0 regressions
- save outcome uncertainty fail-closed / automatic retry0
```

### EBA-P0-04 — Vehicle Mount Compatibility / Cross-Builder UX

#### Pre-Implementation Design Review — 2026-09-16

EBA-P0-03 Technical PASS를 baseline으로 actual `UCFEquipmentPresetData`, `FCFVehicleMountProfile`, `UCFWeaponData`, Vehicle Builder/Authoring VM과 Editor tab registration을 교차검수했다.

```text
P0: 0
blocking P1: 3
P2: 1 non-blocking
Verdict: HOLD / Contract Correction Required
EBA-P0-04 Source implementation: 0
VehicleData mutation: 0
Fitting mutation/execution: 0
RuntimeApply mutation/execution: 0
Product Asset mutation/save: 0
Build/Automation: Not Run — document-only design review
Prior EBA-P0-03 Technical PASS evidence: preserved
Exact next: EBA-P0-04 Contract Correction + Re-review
```

**P1-1 — Vehicle compatibility 결과와 EquipmentPreset durable save gate 분리가 아직 동결되지 않음**

현재 Plan은 `MountType / SizeLimit / Equipment contract diagnostics`만 요구하고 있어 구현자가 `CanUseOnMount()==false`를 기존 `FCFEquipmentDraftValidation::bHasBlockingErrors` 또는 Review/Apply readiness에 합칠 여지가 있다. 그러나 concrete Vehicle/Mount 호환성은 특정 사용 맥락의 read-only 진단이고, 다른 Vehicle에는 정상일 수 있는 EquipmentPreset의 intrinsic authoring validity가 아니다.

Correction에서 다음을 명시적으로 동결해야 한다.

```text
- Vehicle/Mount probe result는 Editor-only transient context다.
- probe state는 EquipmentPreset authored exact7 semantic snapshot에 포함하지 않는다.
- VehicleDataObjectPath / MountProfileId / compatibility result를 ReviewProposalDigest에 포함하지 않는다.
- incompatible Vehicle Mount는 `검토` 또는 `적용 및 저장`을 막지 않는다.
- current probe result는 FCFEquipmentDraftValidation::bHasBlockingErrors를 변경하지 않는다.
- Equipment intrinsic validation이 blocking이면 probe는 CannotEvaluate로만 표시하고 Vehicle source를 mutation하지 않는다.
- final compatibility semantics는 current `UCFEquipmentPresetData::CanUseOnMount(MountType, SizeLimit)`와 의미가 일치해야 한다.
- 사용자 진단은 최소 MountType, SizeLimit, payload-for-mount, RequiredMountType wildcard/exact, RequiredWeaponSize, WeaponData mount/size contract, Sensor validity를 원인별로 분리해 보여준다.
- `현재 Mount에 장착 가능`은 Fitting Snapshot/질량/출격 탄약/전체 Vehicle fitting PASS를 뜻하지 않는다고 UI에 명시한다.
```

**P1-2 — Vehicle/Mount probe의 exact input identity / freshness / invalid source 의미가 아직 동결되지 않음**

`FCFVehicleMountProfile`은 `MountProfileId`, `MountType`, `SizeLimit`, `DefaultEquipmentPresetData` 등을 가지며 Vehicle Builder가 writer authority다. Current native Vehicle validation은 MountProfileId None/duplicate와 LocationSlotRef/Hardpoint 연결 오류를 별도 Vehicle 계약으로 검사한다. Equipment Builder가 이 전체 validation을 복제해서는 안 되지만, 어느 Vehicle/Mount를 읽은 결과인지 안정적으로 특정하고 incomplete source를 conclusive compatibility처럼 표시해서도 안 된다.

Correction에서 다음을 동결해야 한다.

```text
- P0 probe input은 transient `VehicleDataObjectPath + MountProfileId`다.
- EquipmentPreset persisted schema에는 Vehicle identity/MountProfileId를 저장하지 않는다.
- VehicleData 선택은 exact `UCFVehicleData` read-only load/picker로 제한한다.
- Vehicle Builder VM / Vehicle Authoring VM을 Vehicle discovery를 위해 소유하거나 공유하지 않는다.
- MountProfile 선택은 selected VehicleData의 current MountProfiles에서 파생한다.
- MountProfileId None 또는 same Vehicle 내 duplicate이면 CannotEvaluate.
- MountType=None이면 concrete mount compatibility를 평가하지 않고 CannotEvaluate.
- clean persisted VehicleData일 때만 conclusive Compatible/Incompatible을 표시한다.
- dirty/unpersisted/unresolved Vehicle source는 SourceUnconfirmed/확인 불가로 fail-closed하되 EquipmentPreset save gate는 막지 않는다.
- LocationSlotRef/Hardpoint structural validity 전체를 EBA에 복제하지 않고 Vehicle Builder owner로 안내한다.
- SizeLimit=None은 임의로 전역 Invalid 처리하지 않고 current EquipmentPreset/WeaponData semantics로 평가한다. Scanner Utility + RequiredWeaponSize=None 의미를 보존한다.
```

**P1-3 — `Vehicle Builder 열기/연결 UX`의 P0 authority가 모호함**

현재 Editor module은 Vehicle Builder와 Equipment Builder를 독립 Nomad Tab으로 등록하고, Vehicle Builder VM은 Recipe/Hardpoint/companion creation/validation/write 등 큰 business state를 소유한다. Equipment Builder에 선택 Vehicle/Mount를 전달하는 좁은 accepted context handoff seam은 현재 없다. P0에서 이를 구현하려고 shared VM/state bus를 만들면 EBA ownership과 범위가 불필요하게 확장된다.

Correction에서 P0 Cross-Builder UX를 다음으로 제한해야 한다.

```text
- `차량 제작 가이드 열기` 수준의 navigation-only 연결
- registered Vehicle Builder tab을 여는 것 외 business state 공유 0
- Vehicle Builder VM / Recipe / Hardpoint state 공유 또는 상속 0
- selected VehicleData / MountProfile / EquipmentPreset 자동 주입·자동 선택 0
- Vehicle Builder source를 Equipment compatibility 계산 authority로 사용하지 않음
- 필요하면 module-owned navigation-only tab open seam만 최소 노출하며 business API로 확장하지 않음
- reverse navigation / exact-context focus는 P0 필수 아님
```

**P2-1 — Exact-context Cross-Builder handoff**

Equipment Builder에서 선택한 VehicleData/MountProfile을 Vehicle Builder를 열 때 그대로 focus하는 UX는 유용하지만, current Source에는 business state를 침범하지 않는 stable handoff seam이 없다. P0에서는 navigation-only로 끝내고 exact-context focus는 별도 좁은 read-only context 계약이 생긴 뒤 후속 후보로 둔다.

#### P0-04 Correction 후 구현 허용 범위 후보

```text
- Equipment Builder 내부 transient UCFVehicleData picker
- selected VehicleData의 MountProfile read-only 목록
- VehicleDataObjectPath + MountProfileId session-local selection
- 별도 Editor-only mount compatibility result/diagnostic DTO
- MountType / SizeLimit / EquipmentPreset / WeaponData / SensorData read-only evaluation
- Compatible / Incompatible / CannotEvaluate(or SourceUnconfirmed) 명확한 표시
- `Full Fitting 검증 아님` 안내
- navigation-only `차량 제작 가이드 열기`
- VehicleData / Fitting / RuntimeApply mutation 0
```

P0/P1 0 Contract Re-review 전에는 위 후보를 Source로 구현하지 않는다.

#### Contract Correction + Re-review — 2026-09-16

Design Review의 blocking P1 exact3를 actual `UCFEquipmentPresetData::CanUseOnMount`, current VehicleData/Mount ownership과 existing independent Vehicle/Equipment Nomad Tab registration에 다시 대조해 아래 계약으로 동결했다.

```text
P0: 0
blocking P1: 0
P2: 1 non-blocking
Verdict: TECHNICAL CONTRACT PASS / RE-ACCEPTED
EBA-P0-04 Source implementation in this gate: 0
VehicleData mutation: 0
Fitting mutation/execution: 0
RuntimeApply mutation/execution: 0
Product Asset mutation/save: 0
Build/Automation: Not Run — document-only contract correction + re-review
Prior EBA-P0-03 Technical PASS evidence: preserved
Exact next: EBA-P0-04 Vehicle Mount Compatibility / Cross-Builder UX Implementation + Fresh Validation
```

**P1-1 CLOSED — compatibility probe와 Equipment durable save gate 완전 분리**

P0-04 compatibility probe는 특정 Vehicle/Mount 사용 맥락을 설명하는 **Editor-only transient advisory context**로 동결한다. EquipmentPreset 자체의 authored validity 또는 durable approval 의미로 승격하지 않는다.

```text
- probe state는 EquipmentPreset authored exact7 semantic snapshot에 포함하지 않는다.
- VehicleDataObjectPath / MountProfileId / probe result / probe reason은 Current/Prospective semantic fingerprint에 포함하지 않는다.
- VehicleDataObjectPath / MountProfileId / probe result / probe reason은 ReviewProposalDigest에 포함하지 않는다.
- Compatible / Incompatible / CannotEvaluate 어느 결과도 그 자체로 `검토` 또는 `적용 및 저장` readiness를 변경하지 않는다.
- probe 결과는 FCFEquipmentDraftValidation::bHasBlockingErrors를 변경하지 않는다.
- Equipment intrinsic validation이 blocking이면 기존 durable authoring guard는 그대로 작동하고, compatibility 쪽은 `CannotEvaluate(EquipmentIntrinsicInvalid)`로만 투영한다.
- conclusive compatibility의 최종 boolean 의미는 current `UCFEquipmentPresetData::CanUseOnMount(MountType, SizeLimit)`와 같아야 한다.
- 설명용 diagnostic은 payload-for-mount, RequiredMountType wildcard/exact, RequiredWeaponSize 대 SizeLimit, WeaponData mount/size, SensorConfig validity를 원인별로 보여줄 수 있다.
- `Compatible`은 현재 Mount의 Equipment compatibility만 뜻하며 Fitting Snapshot, 질량, 출격 탄약, Defense, 전체 Vehicle fitting, RuntimeApply readiness PASS를 의미하지 않는다.
- probe 자체가 SavePackage / CFDADurableCore / child save / Vehicle mutation을 호출하는 경로는 0이다.
```

따라서 어떤 clean EquipmentPreset Draft가 특정 Vehicle Mount에서 `Incompatible`이어도 다른 Vehicle/Mount에서 사용할 수 있는 authored asset의 durable 저장을 막지 않는다.

**P1-2 CLOSED — transient VehicleDataObjectPath + MountProfileId identity/freshness와 exact3 outcome**

P0 probe input identity는 session-local transient exact2로 동결한다.

```text
VehicleDataObjectPath
MountProfileId
```

Persisted EquipmentPreset schema, exact7 semantic snapshot 또는 VehicleData 자체에는 이 probe identity를 새로 저장하지 않는다.

Probe outcome은 exact3만 사용한다.

```text
Compatible
Incompatible
CannotEvaluate
```

`SourceUnconfirmed` 같은 네 번째 outcome을 추가하지 않는다. 평가 불가 이유는 `CannotEvaluate`의 stable reason으로만 표현한다.

P0 최소 reason contract:

```text
NoVehicleSelected
VehicleSourceUnresolved
VehicleSourceUnpersisted
VehicleSourceDirty
MountProfileMissing
MountProfileIdNone
MountProfileIdDuplicate
MountTypeNone
EquipmentIntrinsicInvalid
```

`Compatible` 또는 `Incompatible`을 반환하려면 모든 prerequisite가 fresh current source에서 충족되어야 한다.

```text
1. VehicleDataObjectPath가 exact UCFVehicleData로 resolve된다.
2. selected Vehicle source가 persisted + clean이다.
3. MountProfileId가 None이 아니다.
4. current VehicleData.MountProfiles 안에서 MountProfileId가 exact1이다.
5. selected MountProfile의 MountType이 None이 아니다.
6. 현재 Equipment Draft의 intrinsic blocking validation이 0이다.
```

위 조건 중 하나라도 충족하지 않으면 `CannotEvaluate(<reason>)`다. prerequisite를 모두 통과하면 current MountProfile의 `MountType + SizeLimit`과 현재 Equipment Draft semantics를 사용해 runtime-equivalent compatibility를 계산하고, final compatible boolean은 `UCFEquipmentPresetData::CanUseOnMount(MountType, SizeLimit)` 의미와 일치시킨다.

Freshness 계약은 다음으로 제한한다.

```text
- probe refresh마다 VehicleDataObjectPath를 다시 resolve한다.
- current VehicleData.MountProfiles에서 MountProfileId를 다시 exact1 lookup한다.
- Vehicle selection이 바뀌면 이전 MountProfile 선택/result를 conclusive 상태로 carry-forward하지 않는다.
- selected Mount가 삭제되거나 ID가 None/duplicate가 되면 즉시 CannotEvaluate다.
- Vehicle package가 dirty/unpersisted가 되면 conclusive state를 유지하지 않고 CannotEvaluate다.
- Equipment Draft, Vehicle selection, Mount selection이 바뀌면 compatibility result를 다시 계산한다.
- explicit refresh/view rebuild에서도 current source를 재해석한다.
- generic asset-change observer/event bus는 P0 필수 계약이 아니다.
```

Vehicle 선택은 exact `UCFVehicleData` read-only picker/load로 제한하고 Mount 목록은 selected VehicleData의 current `MountProfiles`에서 직접 파생한다. Vehicle Builder VM / Vehicle Authoring VM을 discovery 또는 compatibility authority로 소유·공유하지 않는다. LocationSlotRef/Hardpoint 전체 structural validation은 Vehicle Builder owner에 남기며 EBA가 복제하지 않는다.

`SizeLimit=None`은 EBA가 독립 Invalid 규칙을 만들지 않는다. current EquipmentPreset/WeaponData semantics를 그대로 적용하며 Utility Scanner + `RequiredWeaponSize=None` 계약을 보존한다.

**P1-3 CLOSED — P0 Cross-Builder는 navigation-only**

현재 Editor module은 `CarFight.VehicleBuilder`와 `CarFight.EquipmentBuilder`를 독립 Nomad Tab으로 등록하며 existing `OpenBuilderTab()`은 `TryInvokeTab(VehicleBuilderTabName)`만 수행한다. P0 Cross-Builder는 이 precedent를 그대로 사용한 navigation-only UX로 동결한다.

```text
- Equipment Builder의 `차량 제작 가이드 열기` action은 registered Vehicle Builder tab을 여는 것만 수행한다.
- selected VehicleData / MountProfileId / EquipmentPreset 자동 주입·자동 선택 0
- Vehicle Builder VM / Vehicle Authoring VM / Recipe / Hardpoint state 공유·상속 0
- shared state bus / cross-builder business framework 신규 도입 0
- Vehicle Builder VM을 compatibility 계산 authority로 호출 0
- reverse navigation / exact-context focus는 P0 필수 아님
- 필요하면 module-owned tab-open seam만 최소 노출하며 business API로 확장하지 않는다.
```

**P2-1 DEFER — exact-context handoff**

Equipment Builder에서 선택한 VehicleData/MountProfile을 Vehicle Builder에서도 같은 context로 자동 focus하는 기능은 유용하지만 P0 acceptance에는 필요하지 않다. stable read-only handoff seam을 별도로 설계할 때 후속 후보로만 유지한다.

Implementation Gate에서 최소 다음 behavior를 증명한다.

```text
1. clean persisted Vehicle + unique concrete Mount + compatible Weapon Equipment → Compatible
2. RequiredMountType mismatch → Incompatible
3. RequiredWeaponSize > SizeLimit → Incompatible
4. WeaponData mount/size incompatibility → Incompatible
5. valid Utility Scanner + Utility Mount → Compatible
6. no vehicle / unresolved / unpersisted / dirty vehicle → CannotEvaluate
7. missing / None / duplicate MountProfileId 또는 MountType=None → CannotEvaluate
8. intrinsic-invalid Equipment Draft → CannotEvaluate이며 기존 intrinsic save guard만 유지
9. probe outcome/reason 변화가 ReviewProposalDigest / bHasBlockingErrors / Apply readiness를 변경하지 않음
10. Vehicle selection 변경 시 이전 Mount selection/result conclusive carry-forward 0
11. `차량 제작 가이드 열기`는 tab navigation만 수행하고 context injection/business mutation 0
12. VehicleData / Fitting / RuntimeApply mutation·execution 0
```

이 목록은 exact Automation test count를 미리 고정하지 않는 behavior contract다.

재검수 결과 P0/P1 0이므로 EBA-P0-04 Source implementation Gate는 열렸다. **이번 Contract Correction + Re-review Gate 자체에서는 Source 구현을 시작하지 않았다.**

### EBA-P0-05 — End-to-End Technical Acceptance + USER Editor UX

기술검수와 USER Editor UX를 서로 대체하지 않는다.

2026-09-16 Technical Acceptance 당시 Historical 결과:

```text
Verdict: TECHNICAL PASS
P0: 0
blocking P1: 0
새 P2: 0
기존 P0-04 P2-1 exact-context Vehicle/Mount handoff: Deferred / non-blocking 유지

Representative Weapon new flow: PASS
Representative Scanner error → recovery → new flow: PASS
Fresh existing → transient edit → Review → Update flow: PASS
Disposable residue exact0: PASS

Official UE 5.8 Build: PASS — job 73ea790d263a489c8d34bab5c643c946
Focused P005: exact4/4 PASS — process 28d3be19ccd6492da3c76a7f8efc6631
Affected EquipmentAuthoring P001~P005: exact33/33 PASS — process a81dfc01562348c2b8575aa7e7597b3f
Persisted CFEquipmentPresetData baseline: exact10 유지
/Game/CarFight/Tests/EquipmentAuthoring persisted residue: exact0

Product EquipmentPreset write/save by P005 acceptance: 0
Product child DataAsset write/save: 0
VehicleData / Fitting / RuntimeApply mutation·execution: 0
Current Active CF-FQ-039: unchanged
```

첫 P005 acceptance에서는 `ExistingUpdateFlow`와 residue가 PASS했고, Weapon 신규 fixture가 Product source의 authored-namespace `FText`를 lossless Literal semantic으로 재해석하려 한 가정과 Scanner fixture가 기존 persisted Scanner EquipmentPreset 존재를 전제한 가정 때문에 2건이 실패했다. 이는 Product authoring 결함으로 확대하지 않고 test-fixture assumption으로 분류했다. Weapon source는 Product exact7 read-only 불변성만 확인하고 신규 Draft에는 Builder-created Literal DisplayName을 사용하도록 교정했으며, Scanner 신규 flow는 persisted clean/native-valid `VehicleSensorData`를 직접 선택하도록 교정했다. 이후 final Build, focused4, affected33이 모두 PASS했다.

2026-09-17 USER Editor UX correction fresh technical revalidation 결과:

```text
Verdict: TECHNICAL PASS / USER 재검수 준비 완료
Durable contract: Revision 2
Localized DisplayName: preserve-only / 읽기 전용 / durable save rewrite 0
Localization identity: namespace / key / source / 표시 text 보존
Scanner Draft: RequiredMountType=Utility + RequiredWeaponSize=None 고정 / 사용자 편집 불가
Weapon → Scanner → Weapon: mount type / size / TurretMountData / WeaponData session-local cache 복원 PASS
사용자 UI / validation / compatibility / review 문구: 한글 중심 교정 반영
Source duplicate legacy block audit: PASS / accidental duplicate 없음

Official UE 5.8 Build: PASS — job a5ced80ae485450281b3ffdd906d6acb
Focused P005: exact6/6 PASS — process da788fabe43449d5a7c78a90e002ea69
Affected EquipmentAuthoring P001~P005: exact35/35 PASS — process f2a5ab9ffe6f4fd996dae4dcadec1e96
P003.ZDisposableResidue: PASS
P005.ZDisposableResidue: PASS
failure / missing / unexpected / duplicate terminal: 모두 0

Fresh AssetDump /Game/CarFight/Tests/EquipmentAuthoring: NOT RUN to completion
Reason: carfight/unreal capability = Configured / stale=static
Classification: GoPyMCP Project capability infrastructure blocker / EBA Product defect 아님
Automation residue contract: registry + loaded UObject + disk exact0 PASS

이번 USER UX correction / validation이 새로 만든 Product EquipmentPreset mutation: 0
이번 USER UX correction / validation이 새로 만든 child DataAsset mutation: 0
이번 USER UX correction / validation이 새로 만든 VehicleData mutation: 0
Durable authority: CFDADurableCore::ApplyTypedTarget 유지
EquipmentAuthoring production direct SavePackage: 0
Temporary focused/affected runner: 검증 후 quarantine 완료
```

2026-09-17 USER UX correction extension fresh technical revalidation 결과:

```text
Verdict: TECHNICAL PASS / USER 재검수 준비 완료
기존 persisted EquipmentPreset 장비 종류 변경: 차단
신규 EquipmentPreset identity: 에셋 이름 → EquipmentId + Product target 자동 파생
신규 canonical Product root: /Game/CarFight/Weapons/Data/EquipmentPresets
실제 Product root corroboration: fresh managed CarFight Editor Asset Registry read-only exact2
  - /Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon
  - /Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher
게임 표시 이름: player-facing 의미를 UI에서 명시
호환성 결과: 새로고침 action 바로 아래 인접 배치
TurretMountData / WeaponData / VehicleSensorData: 표준 Content Browser child DataAsset 생성 진입 + 제작 순서 안내 추가
Child durable boundary: EquipmentPreset 저장과 child 작성·저장을 분리, Equipment Builder custom child writer 0
신규 Draft 초안 되돌리기: asset name / EquipmentId / auto target exact3 초기화

Official UE 5.8 Build: PASS — job 66d739f5e479430983f7380f4bc5d4d9
Focused P005: exact8/8 PASS — process 074957d02b4d44c19d343bfa691eebb5
Affected EquipmentAuthoring P001~P005: exact37/37 PASS — process 747050417a024a1285e572be47fc8291
P003.ZDisposableResidue: PASS
P005.ZDisposableResidue: PASS
failure / missing / unexpected / duplicate terminal: 모두 0

Fresh AssetDump: BLOCKED by carfight/unreal Configured / stale=static — infrastructure only / Product defect 아님
Accepted read-only live corroboration: exact managed CarFight Editor Asset Registry에서 current Product EquipmentPreset/TurretMount/WeaponData root 확인
이번 correction/validation 신규 Product EquipmentPreset save: 0
이번 correction/validation 신규 child DataAsset save: 0
이번 correction/validation 신규 VehicleData mutation: 0
Durable authority: CFDADurableCore::ApplyTypedTarget 유지
EquipmentAuthoring production direct SavePackage: 0
```

Child DataAsset의 `새로 만들기…`는 Equipment Builder 전용 writer를 추가하는 것이 아니라 Unreal 표준 Content Browser DataAsset factory 흐름으로 위임하는 Editor UX entry다. Equipment Builder는 child 필드 materialize, reviewed durable apply, 자동 SavePackage를 소유하지 않는다. 생성된 child는 해당 데이터 에디터에서 사용자가 작성·저장한 뒤 Equipment Builder가 reference selection + validation만 수행한다.

2026-09-17 USER 결정으로 Equipment Builder는 여기서 HOLD한다. 현재 `장비 구성 데이터 제작 / 선택`이 실제 콘텐츠 제작 workflow를 충분히 안내하지 못하고, Weapon/Turret/Ammo/Projectile/Damage 등 무장 작성 책임을 Equipment Builder 한 화면에 계속 흡수하면 장비 관리/조립 허브가 과도하게 비대해질 위험이 확인됐다.

따라서 **별도 Weapon Equipment Authoring Guide(무장 제작 가이드)를 먼저 구현하고, Equipment Builder는 장비 관리·조립·호환성·최종 Review/Apply 허브로 유지한다.** Weapon Guide가 구현되어 Equipment Builder와의 integration contract가 준비되기 전에는 EBA-P0-05 USER Editor UX를 재개하지 않는다. 기존 Technical PASS evidence는 보존하며 재검증 없이 무효화하지 않는다.

USER Editor UX는 아직 PASS로 판정하지 않았다. HOLD 해제 후 USER는 실제 Editor에서 다음 7가지만 재확인한다.

```text
1. 실제 HeavyCannon 선택 → 검토가 성공한다.
2. 기존 장비에서는 무장/스캐너 종류 버튼이 비활성화되어 종류를 바꿀 수 없다.
3. 게임 표시 이름의 의미가 플레이어-facing 이름으로 이해되고, localized existing DisplayName은 보존 안내와 함께 읽기 전용이다.
4. 새 장비 프리셋에서 에셋 이름을 입력하면 내부 장비 ID와 저장 위치가 자동으로 정해지며 raw ID/path를 직접 관리하지 않는다.
5. Scanner 선택 시 장착 타입은 유틸리티, 장착 크기는 해당 없음으로 고정되어 수정할 수 없다.
6. 호환성 결과가 새로고침 버튼 바로 아래에서 즉시 보이고, child DataAsset의 `새로 만들기…`와 제작 순서 안내가 이해 가능하다.
7. 전체 사용자 문구가 한글 중심으로 자연스럽고 이해 가능하다.
```

USER UX 확인에서는 실제 Product EquipmentPreset 저장이 필수 조건이 아니다. 신규 Weapon/Scanner는 canonical Product root `/Game/CarFight/Weapons/Data/EquipmentPresets` 아래에서 에셋 이름으로 target이 자동 파생되며, 실제 Product Asset이 필요하지 않다면 `적용 및 저장`을 누르지 않는다. child `새로 만들기…`도 UX 확인 목적이면 생성 확정/저장까지 진행할 필요가 없다.

**USER PASS 전 `EBA-P0-06 Current System Promotion / Final Closure Audit`은 BLOCKED이며 시작하지 않는다.**

### EBA-P0-06 — Current System Promotion / Final Closure Audit

```text
- final source/asset ownership audit
- required Systems promotion
- FeatureQueue Done 전환 조건 확인
- Historical/Current route 정리
```

---

## 9. 비차단 후속 후보

P0 core closure 전에 자동으로 범위를 확장하지 않는다.

후속 후보:

```text
- 기존 FCFRuntimeEquipApplyService를 사용한 차량 테스트 장착
- Equipment + Preview Vehicle mounted visualization
- 더 나은 Turret/Projectile structural validation
- Vehicle Builder ↔ Equipment Builder cross-navigation 개선
- 새 Runtime equipment payload가 실제 구현된 이후 추가 장비 카테고리 onboarding
```

Runtime test mount를 추가할 경우에도 Equipment Builder가 Fitting/Runtime authority를 소유하지 않고 기존 `FCFRuntimeEquipApplyService`를 호출하는 frontend로 남는다.

---

## 10. 검증 전략

Source 변경 Gate에서는 현재 CarFight 표준 검증을 사용한다.

```text
Official UE 5.8 Editor Build
→ D:\Work\CarFight_git\Tools\BuildEditor.bat

Focused Automation
→ Equipment Authoring exact scope

Affected Regression
→ Vehicle Builder / EquipmentPreset / Fitting / RuntimeApply / DataManagement 중 실제 changed-path 영향만 선정
```

EBA-P0-03 구현 시 focused Automation 최소 contract surface는 다음을 포함한다.

```text
1. Create durable PASS + disk reload fingerprint exact match
2. Update durable PASS + disk reload fingerprint exact match
3. Create in-memory target collision → mutation0
4. Create disk package collision → mutation0
5. Update dirty package → mutation0
6. reviewed current fingerprint stale → mutation0
7. ReviewProposalDigest mismatch → mutation0
8. duplicate EquipmentId → mutation0
9. duplicate namespace coverage incomplete → uniqueness unconfirmed / mutation0
10. unsupported persisted DisplayName FText representation → lossless update blocked
11. child missing / wrong class / child validator failure → mutation0
12. pre-save typed readback failure → Create cleanup 또는 Update rollback confirmed
13. forced SavePackage uncertainty → SaveStateUnconfirmed / automatic retry0
14. forced post-save confirmation uncertainty → SaveStateUnconfirmed / automatic retry0
15. dedicated Automation target cleanup → residue exact0
```

이 목록은 test name/개수를 미리 고정하는 것이 아니라 반드시 보존해야 할 behavior contract다. 실제 focused test exact count는 구현 changed-path와 fixture 구성 후 확정한다.

Persisted `.uasset` mutation을 검증할 때는 다음을 분리한다.

```text
Persisted Asset/DataAsset 사실
→ AssetDump evidence 우선

현재 Editor/unsaved 상태
→ Accepted UE MCP capability가 필요할 때 사용

사용성/레이아웃/읽기 편함
→ USER Editor UX validation
```

Technical PASS와 USER UX PASS를 합치지 않는다.

문서-only Gate에서는 Source/Asset mutation이 없으면 불필요한 Build/Automation을 반복하지 않는다.

---

## 11. 보호 범위 / Stop Rules

다음 상황이 확인되면 다음 Gate로 자동 전진하지 않는다.

```text
1. EquipmentPresetData exact1을 넘어 arbitrary DataAsset writer가 필요함
2. AmmoData / DamageData 기존 Reviewed writer를 우회하거나 복제해야 함
3. Vehicle Builder를 대규모 공용 framework로 리팩터링해야만 구현 가능함
4. Fitting Snapshot / RuntimeApply algorithm을 Equipment Builder에 복제해야 함
5. Vehicle Mount / Inventory ownership mutation이 Builder save의 부작용으로 필요함
6. Runtime에 없는 generic EquipmentType/category를 먼저 추가해야 함
7. Child DataAsset을 EquipmentPreset Save와 함께 암묵적으로 저장해야 함
8. Existing dirty / Product Asset을 범위 밖에서 수정해야 함
```

이 경우 현재 Gate를 HOLD하고 필요한 architecture/ownership gap만 별도로 검토한다.

기존 병렬 dirty와 Product Asset은 보호한다. disposable test Asset이 필요하면 Gate별로 exact 테스트 root와 cleanup/residue 검증을 먼저 동결한다.

---

## 12. Current Checkpoint

2026-09-17 EBA-P0-05 USER Editor UX correction technical revalidation 완료 / USER 재검수 준비 시점:

```text
Feature ID: CF-FQ-054
Lifecycle: Ready
Priority: P2
Ownership Audit: Accepted
EBA-P0-00 Contract Freeze: PASS
EBA-P0-01 Editor Shell / Browser / Draft Model: Technical PASS
EBA-P0-02 Correction + Re-review: Technical PASS / RE-ACCEPTED
EBA-P0-03 Review / Durable Apply-and-Save: TECHNICAL PASS
EBA-P0-04 Vehicle Mount Compatibility / Cross-Builder UX: TECHNICAL PASS
EBA-P0-05 End-to-End Technical Acceptance: TECHNICAL PASS
EBA-P0-05 USER Editor UX correction technical revalidation: TECHNICAL PASS
Durable Contract Revision: 2
Localized DisplayName preserve-only / read-only: PASS
Scanner Utility + None fixed policy: PASS
Weapon → Scanner → Weapon session-local restore: PASS
한글 중심 USER surface correction: Applied
P0: 0
blocking P1: 0
새 P2: 0
P0-04 P2-1 selected Vehicle/Mount exact-context handoff: DEFER CANDIDATE / non-blocking 유지
Official UE 5.8 Build: PASS — final job 66d739f5e479430983f7380f4bc5d4d9
Focused EBA-P0-05: exact8/8 PASS — process 074957d02b4d44c19d343bfa691eebb5
Affected EquipmentAuthoring P001~P005: exact37/37 PASS — process 747050417a024a1285e572be47fc8291
P003.ZDisposableResidue: PASS
P005.ZDisposableResidue: PASS
Fresh AssetDump: BLOCKED by carfight/unreal Configured / stale=static — infrastructure only / Product defect 아님
이번 correction/validation 신규 Product EquipmentPreset mutation: 0
이번 correction/validation 신규 child DataAsset mutation: 0
이번 correction/validation 신규 VehicleData mutation: 0
Durable authority: CFDADurableCore::ApplyTypedTarget 유지
EquipmentAuthoring production direct SavePackage: 0
USER Editor UX: PENDING / Re-review Ready
EBA-P0-06 Current System Promotion: BLOCKED / Not Started until Weapon Equipment Authoring Guide integration-ready + USER PASS
Current Active CF-FQ-039: unchanged
Exact next: EBA-P0-05 USER Editor UX re-review
```

2026-09-16 P0-05 technical acceptance는 Product authoring logic을 변경하지 않고 `CFEquipmentDurableTests.cpp`의 test-only acceptance surface만 보강했다. Weapon은 persisted Product EquipmentPreset/child graph를 read-only representative source로 사용하고 Product exact7/clean 불변성을 확인한다. Scanner는 persisted clean/native-valid `VehicleSensorData`를 representative child로 사용해 missing Sensor 오류→교정→Review→durable disposable save를 검증한다. existing/update는 disposable Create 뒤 fresh browser/VM으로 persisted target을 다시 발견하여 transient edit→Review→Update→persisted fingerprint readback까지 확인한다.

2026-09-16 첫 focused acceptance의 2건 실패는 Product 결함이 아니라 fixture assumption이었다. 기존 Product Weapon DisplayName의 authored namespace FText를 Literal Staging semantic으로 silent 변환하지 않는 기존 fail-closed 계약과, 기존 persisted Scanner EquipmentPreset exact1을 당연히 전제할 수 없다는 점을 확인하고 fixture만 교정했다. 중간 test-only compile 오류 2건도 include/type-completeness 범위에서만 교정했다.

2026-09-17 USER Editor 실사용에서 발견된 localized DisplayName Review 실패, 내부 영어 용어 과다 노출, Scanner mount policy 편집 가능 문제와 SensorData 탐색 UX를 bounded USER UX correction으로 교정했다. Durable Contract Revision 2는 기존 localized DisplayName을 preserve-only semantic으로 취급해 Review/Edit 시 원본 FText identity를 보존하고 durable materialize에서 해당 필드 rewrite를 생략한다. Scanner는 Builder policy상 Utility + None을 고정하며, Weapon 전환 상태는 session-local cache로 복원한다.

같은 날 USER UX correction extension에서 기존 EquipmentPreset의 Weapon/Scanner 종류 변경을 차단하고, 신규 EquipmentPreset은 에셋 이름으로 내부 EquipmentId와 canonical Product target을 자동 파생하도록 Guided UX를 단순화했다. 게임 표시 이름의 player-facing 의미를 명시하고 compatibility result를 action 바로 아래로 이동했다. TurretMountData / WeaponData / VehicleSensorData에는 표준 Content Browser child DataAsset 생성 진입과 제작 순서 안내를 추가하되 Equipment Builder custom child durable writer는 추가하지 않았다. 마지막 code review에서 신규 Draft `초안 되돌리기`가 auto identity state까지 초기화하도록 교정했고 fresh Build, focused exact8, affected exact37와 residue tests가 모두 PASS했다.

Technical PASS는 USER Editor UX PASS를 대신하지 않는다. USER가 위 7개 항목을 실제 Editor에서 재확인하기 전까지 P0-06으로 전진하지 않는다.

---

## 13. 관련 Current 문서 / Source

Current 판단은 항상 main_game actual Source와 아래 Systems를 우선한다.

```text
Document/Systems/Vehicles/VehicleBuilder.md
Document/Systems/Vehicles/RuntimeApply.md
Document/Systems/Combat/WeaponData.md
Document/Systems/Targeting/SensorContact.md
Document/Systems/DataManagement/DataAssetManagement.md
Document/Systems/DataManagement/DataAssetAuthoring.md

UE/Source/CarFight_Re/Public/CFEquipmentPresetData.h
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Public/CFTurretMountData.h
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Public/CFAmmoData.h
UE/Source/CarFight_Re/Public/CFDamageData.h
UE/Source/CarFight_Re/Public/CFVehicleSensorData.h
UE/Source/CarFight_Re/Public/CFRuntimeEquipApply.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/
UE/Source/CarFight_ReEditor/Private/DataManagement/
```

---

## 14. Changelog

### v0.1.18 - 2026-09-29

- USER가 개별 무장 제작보다 다수 Vehicle/Weapon/DataAsset을 Excel authority + AI + Generic Content Compiler로 운영하는 상위 방향을 승인해 `CF-FQ-058`이 새 prerequisite가 됐다.
- EBA-P0-05까지 확보한 Equipment Builder Technical PASS evidence와 durable contract는 보존한다.
- 기존 "Weapon Guide integration-ready 뒤 USER UX 재개" 기준을 폐기하고, CCAS가 EquipmentPreset / Vehicle / Weapon의 authoring authority와 assembly 경계를 Freeze한 뒤 fresh rebaseline하여 재개하도록 변경했다.
- 이번 변경은 document/lifecycle 재기준점이며 Equipment Source, Product Asset, Runtime/Fitting 계약 mutation은 0이다.

### v0.1.17 - 2026-09-17

- USER review 중 `장비 구성 데이터 제작 / 선택`이 Equipment Builder 내부에 무장 제작 상세 workflow를 계속 흡수하면 툴이 과도하게 비대해질 수 있음을 확인했다.
- Equipment Builder의 최종 책임을 `장비 관리 / 조립 / 호환성 / 최종 Review·Apply` 허브로 제한하고, `Weapon Equipment Authoring Guide(무장 제작 가이드)`를 별도 제작 workflow로 분리하기로 결정했다.
- CF-FQ-054 lifecycle은 EBA-P0-05에서 `Paused / HOLD`로 전환하며, Weapon Equipment Authoring Guide가 구현되고 Equipment Builder integration contract가 준비된 뒤 USER Editor UX를 재개한다.
- 기존 Official Build PASS, focused exact8/8 PASS, affected exact37/37 PASS와 residue PASS evidence는 그대로 보존한다. HOLD는 기술 실패가 아니라 선행 UX/authoring workflow 분리 결정이다.
- EBA-P0-06 Current System Promotion은 `Weapon Equipment Authoring Guide integration-ready + USER PASS` 전까지 계속 Not Started다.

### v0.1.16 - 2026-09-17

- EBA-P0-05 USER UX correction extension으로 existing EquipmentPreset의 Weapon/Scanner 종류 변경을 금지하고, 신규 EquipmentPreset을 `에셋 이름 → EquipmentId + /Game/CarFight/Weapons/Data/EquipmentPresets/<Name>.<Name>` 자동 파생 Guided UX로 전환했다.
- fresh managed CarFight Editor의 read-only Asset Registry에서 Product `CFEquipmentPresetData` exact2가 `/Game/CarFight/Weapons/Data/EquipmentPresets`에 실제 존재함을 확인해 자동 Product root의 current corroboration을 확보했다. fresh AssetDump은 `carfight/unreal Configured / stale=static` blocker로 계속 fail-closed이며 이 live read는 AssetDump 대체 승격이 아니라 bounded corroboration이다.
- `게임 표시 이름`이 Fitting/HUD player-facing 이름임을 UI에서 설명하고, compatibility result를 `호환성 새로고침` 바로 아래로 이동했다. TurretMountData / WeaponData / VehicleSensorData에는 표준 Content Browser DataAsset 생성 진입과 한글 제작 순서 안내를 추가했다. child 내용 materialize/reviewed durable apply/자동 SavePackage는 Equipment Builder가 소유하지 않는다.
- final code review에서 신규 Draft `초안 되돌리기`가 `CreateAssetName / EquipmentId / CreateTargetObjectPath`를 함께 초기화하지 않던 presentation-state 불일치를 발견해 교정하고 `P005.AutoIdentity` 회귀에 reset exact3 검증을 추가했다.
- fresh Official UE 5.8 Build job `66d739f5e479430983f7380f4bc5d4d9` PASS, focused P005 process `074957d02b4d44c19d343bfa691eebb5` exact8/8 PASS, affected process `747050417a024a1285e572be47fc8291` exact37/37 PASS를 확보했다. failure/missing/unexpected/duplicate terminal은 모두 0이며 `P003.ZDisposableResidue`, `P005.ZDisposableResidue`도 PASS다.
- USER Editor UX는 `PENDING / Re-review Ready`이며 actual USER checklist를 exact7로 갱신했다. **USER PASS 전 EBA-P0-06 Current System Promotion은 계속 BLOCKED / Not Started**다.

### v0.1.15 - 2026-09-17

- USER 실제 Editor 검수에서 발견된 localized `DisplayName` Review 실패, 사용자 surface의 내부 영어 용어 과다 노출, Scanner 장착 타입 편집 가능 문제와 SensorData 탐색 난이도를 EBA-P0-05 bounded USER UX correction으로 교정했다.
- Durable Contract를 Revision 2로 올려 기존 localized `DisplayName`을 preserve-only/read-only로 취급하고 durable save에서 해당 `FText` rewrite를 생략한다. namespace/key/source/text identity를 보존하면서 다른 Equipment field Review/Edit는 유지한다.
- Scanner Draft는 `RequiredMountType=Utility` + `RequiredWeaponSize=None`을 Builder policy로 고정하고 두 필드 사용자 편집을 막았다. Weapon → Scanner → Weapon 전환 시 이전 Weapon mount type/size/TurretMountData/WeaponData를 session-local cache에서 복원한다.
- 사용자 UI/validation/compatibility/review 문구를 한글 중심으로 교정했다. current source audit에서 accidental duplicate legacy block은 확인되지 않았고 durable authority는 `CFDADurableCore::ApplyTypedTarget`에 유지되며 EquipmentAuthoring production direct `SavePackage`는 0이다.
- fresh Official UE 5.8 Build job `a5ced80ae485450281b3ffdd906d6acb` PASS, focused P005 process `da788fabe43449d5a7c78a90e002ea69` exact6/6 PASS, affected process `f2a5ab9ffe6f4fd996dae4dcadec1e96` exact35/35 PASS를 확보했다. failure/missing/unexpected/duplicate terminal은 모두 0이며 `P003.ZDisposableResidue`, `P005.ZDisposableResidue`도 PASS다.
- fresh AssetDump 재확인은 `carfight/unreal` capability가 `Configured / stale=static`이라 Project mode에서 fail-closed되었다. 이는 GoPyMCP infrastructure blocker이며 EBA Product 결함으로 확대하지 않는다. 동일 fresh Automation의 residue contract는 registry + loaded UObject + disk exact0을 검증해 PASS했다.
- 이번 correction/validation이 새로 만든 Product EquipmentPreset/child DataAsset/VehicleData mutation은 0이며 임시 runner는 검증 뒤 quarantine했다. USER Editor UX는 `PENDING / Re-review Ready`, exact next는 `EBA-P0-05 USER Editor UX re-review`다. **USER PASS 전 EBA-P0-06 Current System Promotion은 계속 BLOCKED / Not Started**다.

### v0.1.14 - 2026-09-16

- `EBA-P0-05 End-to-End Technical Acceptance`를 fresh representative 흐름으로 완료해 `P0 0 / blocking P1 0 / 새 P2 0 / TECHNICAL PASS`로 전진했다. P0-04의 exact-context handoff P2-1은 기존 비차단 Deferred 후보로 유지한다.
- test-only `CFEquipmentDurableTests.cpp v1.1.1`에 Weapon 신규, Scanner 오류→해결→신규, fresh existing→Update, final residue0 exact4 acceptance를 추가했다. Product authoring/runtime logic은 변경하지 않았다.
- final Official UE 5.8 Build job `73ea790d263a489c8d34bab5c643c946` PASS, focused P005 process `28d3be19ccd6492da3c76a7f8efc6631` exact4/4 PASS, affected process `a81dfc01562348c2b8575aa7e7597b3f` exact33/33 PASS를 확보했다. fresh AssetDump에서 persisted CFEquipmentPresetData exact10 baseline 유지와 `/Game/CarFight/Tests/EquipmentAuthoring` residue exact0을 확인했다.
- 첫 focused acceptance에서 Weapon authored-namespace FText와 persisted Scanner EquipmentPreset 존재를 전제한 fixture assumption 2건이 실패했으나 Product 결함으로 확대하지 않고 bounded test fixture만 교정했다. 중간 test-only compile correction도 Product Source/API 변경 없이 닫았다.
- Product EquipmentPreset/child DataAsset write-save, VehicleData/Fitting/RuntimeApply mutation-execution은 0이다. USER Editor UX는 `Pending / Not Run`이며 exact next는 `EBA-P0-05 USER Editor UX`다. **USER PASS 전 EBA-P0-06 Current System Promotion은 BLOCKED / Not Started**다. Current Active `CF-FQ-039`는 unchanged다.

### v0.1.13 - 2026-09-16

- `EBA-P0-04 Vehicle Mount Compatibility / Cross-Builder UX` Source 구현, fresh Official UE 5.8 Build, focused/affected Automation과 Post-Implementation Mid-review를 완료해 `P0 0 / blocking P1 0 / P2 1 non-blocking / TECHNICAL PASS`로 전진했다.
- Editor-only transient compatibility context를 `VehicleDataObjectPath + MountProfileId` exact2로 구현하고 result는 `Compatible / Incompatible / CannotEvaluate` exact3로 제한했다. Vehicle path fresh resolve, persisted+clean guard, current `MountProfiles` exact1 lookup, MountType concrete guard와 intrinsic Equipment validation을 conclusive result prerequisite로 유지한다.
- final compatible boolean은 transient `UCFEquipmentPresetData`에 current Draft를 투영해 existing `CanUseOnMount(MountType, SizeLimit)` authority로 계산한다. RequiredMountType/RequiredWeaponSize/WeaponData mount·size/runtime rejection은 diagnostic reason으로만 세분화하며 Full Fitting 또는 RuntimeApply PASS로 확대하지 않는다.
- compatibility identity/result/reason은 authored exact7 semantic snapshot, semantic fingerprint, ReviewProposalDigest, `bHasBlockingErrors`, Apply readiness와 분리했다. `DurableReviewIsolation` focused test로 compatibility 변경 전후 digest/approval/intrinsic blocking/Draft dirty state가 동일함을 증명했다.
- Cross-Builder는 `CarFight.VehicleBuilder` tab `TryInvokeTab` navigation-only로 구현했고 Vehicle/Mount/Equipment context injection, shared VM/state bus, reverse navigation은 0이다. VehicleData/Fitting/RuntimeApply mutation·execution과 child DataAsset write/save도 0이다.
- final Official UE 5.8 Build job `56342f29791f4c558a6525abd7d9e172` PASS, focused `CarFight.EquipmentAuthoring.P004` process `ed4ea2475a924e5798dda786042ad965` exact9/9 PASS, affected `CarFight.EquipmentAuthoring` process `e2cb8c582d60464ba1d3d12a0619b28a` exact29/29 PASS, fresh AssetDump disposable EquipmentPreset residue exact0을 확보했다.
- 중간 이력의 test namespace C1075, Editor DLL lock, current Product fixture 부재 기반 SizeMismatch 1건은 각각 test-only syntax/lifecycle/test-fixture 범위에서 교정했고 Product Asset이나 Runtime/Fitting authority를 변경하지 않았다.
- P2-1 exact-context Vehicle/Mount handoff는 비차단 Deferred 후보로 유지한다. USER Editor visual/UX acceptance는 아직 실행하지 않았고 `EBA-P0-05 End-to-End Technical Acceptance + USER Editor UX`가 exact next다. current Active `CF-FQ-039`는 unchanged다.

### v0.1.12 - 2026-09-16

- `EBA-P0-04 Contract Correction + Re-review`에서 Design Review blocking P1 exact3를 current `UCFEquipmentPresetData::CanUseOnMount`, VehicleData ownership과 independent Vehicle/Equipment Nomad Tab precedent에 다시 대조해 `P0 0 / blocking P1 0 / P2 1 non-blocking / TECHNICAL CONTRACT PASS / RE-ACCEPTED`로 닫았다.
- P1-1은 compatibility probe를 Editor-only transient advisory context로 동결하고 VehicleDataObjectPath/MountProfileId/result/reason을 authored exact7, semantic fingerprint, ReviewProposalDigest, `bHasBlockingErrors`, Apply readiness에서 완전히 제외했다. Incompatible 또는 CannotEvaluate는 그 자체로 durable save를 막지 않으며 probe는 Save/CFDADurableCore/Vehicle mutation을 호출하지 않는다.
- P1-2는 probe identity를 transient `VehicleDataObjectPath + MountProfileId` exact2, outcome을 `Compatible / Incompatible / CannotEvaluate` exact3로 동결했다. unresolved/unpersisted/dirty Vehicle, missing/None/duplicate MountProfile, MountType=None, intrinsic-invalid Equipment는 stable reason을 가진 CannotEvaluate로 통합하며 conclusive result 전 fresh resolve + exact1 Mount lookup + clean persisted source를 요구한다.
- conclusive compatibility는 current `UCFEquipmentPresetData::CanUseOnMount(MountType, SizeLimit)` 의미와 같게 유지하고 payload, RequiredMountType, RequiredWeaponSize, WeaponData, SensorConfig 원인을 설명용 diagnostic으로만 투영한다. Full Fitting/질량/Ammo/Defense/RuntimeApply PASS로 확대하지 않는다.
- P1-3은 P0 Cross-Builder를 existing `CarFight.VehicleBuilder` tab을 여는 navigation-only action으로 제한했다. shared VM/state bus, Recipe/Hardpoint business state 공유, selected Vehicle/Mount/Equipment context injection, reverse navigation은 P0에서 금지한다.
- P2-1 exact-context Vehicle/Mount focus handoff는 stable read-only handoff seam 후속 후보로 유지한다.
- 이번 Gate는 document-only contract correction/re-review다. EBA-P0-04 Source implementation, VehicleData/Fitting/RuntimeApply mutation/execution, Product Asset mutation/save와 Build/Automation은 0/Not Run이다. EBA-P0-03 Technical PASS evidence를 보존하고 exact next를 `EBA-P0-04 Vehicle Mount Compatibility / Cross-Builder UX Implementation + Fresh Validation`로 전진했다.

### v0.1.11 - 2026-09-16

- `EBA-P0-04 Vehicle Mount Compatibility / Cross-Builder UX` 착수 전 actual EquipmentPreset/MountProfile/WeaponData/Vehicle Builder Source를 교차검수하고 `P0 0 / blocking P1 3 / P2 1 non-blocking / HOLD`로 판정했다.
- P1-1은 concrete Vehicle/Mount compatibility가 특정 사용 맥락의 advisory result인데 기존 Equipment intrinsic validation/ReviewProposalDigest/Apply readiness와 분리하는 계약이 아직 없어, incompatible mount가 EquipmentPreset durable save를 잘못 차단할 수 있는 gap이다. 별도 transient probe state와 원인별 MountType/SizeLimit/Equipment 진단, full Fitting PASS가 아님을 동결 조건으로 요구했다.
- P1-2는 transient VehicleDataObjectPath + MountProfileId identity, unique/non-None MountProfile 선택, concrete MountType, clean persisted source와 dirty/unresolved SourceUnconfirmed 의미가 아직 없다는 gap이다. VehicleData picker + selected VehicleData MountProfiles read-only 파생만 허용하고 Vehicle Builder/Authoring VM을 probe authority로 공유하지 않도록 요구했다.
- P1-3은 `Vehicle Builder 열기/연결 UX`가 open-only navigation인지 selection/context injection인지 모호한 gap이다. P0에서는 registered Vehicle Builder tab navigation-only, business VM/state 공유 0, selected Vehicle/Mount/Equipment auto-injection 0으로 제한하도록 요구했다.
- P2-1 exact-context Vehicle Builder focus handoff는 유용하지만 stable read-only handoff seam이 현재 없으므로 후속 후보로 분리했다.
- 이번 Gate는 document-only review다. EquipmentAuthoring Source/Asset mutation, VehicleData/Fitting/RuntimeApply mutation/execution, Build/Automation은 0/Not Run이며 EBA-P0-03 Build/P003 exact8/affected20/residue0 Technical PASS evidence를 보존한다. exact next는 `EBA-P0-04 Contract Correction + Re-review`다.

### v0.1.10 - 2026-09-16

- `EBA-P0-03 Review / Durable Apply-and-Save` 구현과 fresh validation을 완료해 `Technical PASS`로 전진했다. Revision 1 exact7 digest, complete inventory + loaded-only identity union, persisted/clean child guard, one-shot Reviewed→ApplyAttempted→Consumed, same-approval retry0와 immediate no-yield TOCTOU를 구현했다.
- durable write는 existing `CFDADurableCore::ApplyTypedTarget` exact1 production seam만 사용한다. EquipmentAuthoring production 코드의 direct `SavePackage` / `MarkPackageDirty` / `Modify`는 0이고 `CreatePackage/NewObject` EquipmentPreset 생성은 loaded-only duplicate Automation fixture에만 존재한다. Child DataAsset write/save와 RuntimeApply execution은 0이다.
- 초기 P003 실행에서 `CreateUpdateDurable` / `DirtyTarget` 2건이 새 VM의 stale browser inventory assumption 때문에 실패했으나 Product durable failure가 아님을 확인하고 actual Create 성공 뒤 same-VM Update lifecycle fixture로 교정했다. EBA Source durable contract는 이 fixture 교정에서 변경하지 않았다.
- fresh Official UE 5.8 Build는 병렬 Sensor `FCFSensorBasicSingleScanCompletionTest`의 private deterministic update friend 누락으로 한 차례 차단됐고 owning Sensor scope에서 test friend exact1만 추가해 정상화했다. final Build job `2e6ea90143784b01868effe20568f77c` PASS이며 Runtime/Blueprint Sensor API는 넓히지 않았다.
- final focused `CarFight.EquipmentAuthoring.P003` process `78e1e4d3b0634b8d9af43cdd6052e091` exact8/8 PASS, affected `CarFight.EquipmentAuthoring` process `33404e4fc23740619e489bb557203594` exact20/20 PASS를 확보했다. failure/missing/unexpected/duplicate terminal은 모두 0이다.
- fresh AssetDump `/Game/CarFight/Tests/EquipmentAuthoring`에서 EquipmentPreset persisted residue exact0을 확인했다. Product EquipmentPreset mutation by Automation 0, child write/save 0을 유지한다.
- lifecycle은 Ready, current Active `CF-FQ-039`는 unchanged다. exact next는 `EBA-P0-04 Vehicle Mount Compatibility / Cross-Builder UX`다.

### v0.1.9 - 2026-09-16

- `EBA-P0-03 Contract Correction + Re-review`에서 Pre-Implementation Design Review의 blocking P1 exact4를 current Source precedent에 맞춰 모두 닫고 `P0 0 / blocking P1 0 / P2 0 / TECHNICAL CONTRACT PASS / RE-ACCEPTED`로 판정했다.
- P1-1은 Revision 1 `ReviewProposalDigest`를 OperationKind / TargetObjectPath / exact TargetClassPath / CurrentSemanticFingerprint(or `AbsentTarget`) / ProspectiveSemanticFingerprint / ContractRevision=1 / `ValidationState=Ready` exact7 token으로 동결했다. UI Title/Message/localization/presentation row order는 digest에서 제외하고 blocking proposal은 Reviewed approval 자체를 만들지 않는다.
- P1-2는 DataManagement complete exact-class inventory와 `TObjectIterator<UCFEquipmentPresetData>` loaded-only `/Game/` exact-class 후보를 object-path union으로 합쳐 prospective EquipmentId uniqueness를 확인하도록 동결했다. inventory/registry/candidate coverage가 불확정이면 `IdentityUniquenessUnconfirmed`, duplicate면 mutation0이다.
- P1-3은 Review/Commit validation에 사용되는 Weapon/Turret/Ammo/Projectile/Damage 또는 Sensor dependency가 exact persisted metadata/class/path + non-transient + clean package truth를 가져야 하며 아니면 `ChildStateUnconfirmed` mutation0으로 막도록 동결했다. child implicit save는 계속 금지한다.
- P1-4는 approval을 `Unreviewed→Reviewed→ApplyAttempted→Consumed` one-shot으로 동결하고 fresh preflight 실패와 `SaveStateUnconfirmed`를 포함한 모든 attempt 뒤 same-approval retry0를 요구한다. final digest revalidation→fresh core row→Consumed→`CFDADurableCore::ApplyTypedTarget`은 같은 synchronous no-yield call path다.
- 이번 Gate는 document-only contract correction/re-review라 EquipmentAuthoring Source/durable mutation, Product/child Asset write/save, `CFDADurableCore` execution과 Build/Automation은 0/Not Run이다. 이전 EBA-P0-02 Build/focused9/affected12는 historical baseline으로만 보존한다.
- exact next는 `EBA-P0-03 Review / Durable Apply-and-Save Implementation + Fresh Validation`이다. CF-FQ-054는 Ready, current Active CF-FQ-039는 unchanged다.

### v0.1.8 - 2026-09-16

- `EBA-P0-03 Review / Durable Apply-and-Save` 착수 전 Design Review를 actual Revision 1 semantic primitive, DataManagement identity closure, `CFDADurableCore`와 accepted Staging TOCTOU 패턴에 교차검수하고 `P0 0 / blocking P1 4 / P2 0 / HOLD`로 판정했다.
- Revision 1 authored exact7 snapshot, Literal FText/FName/path/hash primitive, provider-neutral `CFDADurableCore` Create/Update sequencing, exact single-package Save/reload/readback, `SaveStateUnconfirmed`/automatic retry0 core behavior는 PASS로 확인했다.
- P1-1은 current validation DTO에 approval용 stable code/token이 없어 `ReviewProposalDigest`의 blocking validation summary가 UI Title/Message/order에 의존할 수 있는 gap이다. TargetClassPath를 포함한 exact digest token set과 stable blocking token projection을 correction 조건으로 동결했다.
- P1-2는 complete DataManagement inventory closure가 AssetRecords만 다뤄 다른 path의 loaded-but-unregistered exact EquipmentPreset duplicate identity를 누락할 수 있는 gap이다. persisted inventory + loaded-only `/Game/` exact-class union closure를 요구한다.
- P1-3은 child 내부 semantics가 EquipmentPreset fingerprint에 포함되지 않고 child save가 금지된 상태에서 dirty/unsaved child UObject validation이 durable approval에 사용될 수 있는 gap이다. validation에 실제 사용되는 child dependency package clean requirement와 `ChildStateUnconfirmed` mutation0을 요구한다.
- P1-4는 approval one-shot consumption timing이 없어 stale preflight 또는 `SaveStateUnconfirmed` 뒤 same approval 재시도가 가능한 gap이다. `Unreviewed→Reviewed→ApplyAttempted→Consumed`, same-approval retry0와 final fresh preflight→core immediate no-yield call을 요구한다.
- 이번 Gate는 document-only review라 Source/Product Asset mutation, `CFDADurableCore` execution, Build/Automation은 0/Not Run이다. exact next는 `EBA-P0-03 Contract Correction + Re-review`이며 P0/P1 0 전 durable mutation 구현을 금지한다. Current Active `CF-FQ-039`는 unchanged다.

### v0.1.7 - 2026-09-16

- owning 병렬 작업에서 `CFSUVMigrateCmdlet.cpp`의 constructor/Main 구현이 fresh Source에 나타난 뒤 external linker blocker가 해소됐음을 확인하고 Official UE 5.8 Build를 재실행해 final job `59cc64cd607f4dd1b86dc453883eae92` PASS를 확보했다. EBA는 해당 commandlet을 수정하지 않았다.
- fresh focused `CarFight.EquipmentAuthoring.P002` process `1492118763204e42aa1f4b98a928c4f0`이 exact9/9 PASS했고 failure/missing/unexpected/duplicate-terminal은 모두 0이다.
- fresh affected `CarFight.EquipmentAuthoring` process `bf36df7a136c4a4faa0fbd2435c7902c`이 P0-01 exact3 + P0-02 exact9 = exact12/12 PASS했다.
- EBA-P0-02 final re-review를 `P0 0 / blocking P1 0 / P2 0 / Technical PASS / RE-ACCEPTED`로 닫았다. Product/child Asset write/save, CFDADurableCore/RuntimeApply durable execution과 EBA-P0-03 구현은 0이다.
- 초기 build job `937a8e0bd2ea4cf58be04d948484da60`의 unrelated linker failure는 당시 Historical evidence로 보존하며 최종 PASS와 혼합하지 않는다.
- current exact next를 `EBA-P0-03 Review / Durable Apply-and-Save`로 전진했다. Current Active CF-FQ-039는 unchanged다.

### v0.1.6 - 2026-09-16

- `EBA-P0-02 Mid-review Correction`에서 blocking P1 exact2와 P2 exact1을 EquipmentAuthoring bounded scope 안에서 교정하고 independent Source re-review를 `P0 0 / blocking P1 0 / P2 0 / SOURCE CORRECTION PASS`로 닫았다.
- intrinsic mount contradiction은 Scanner `None|Utility`, Weapon `Utility 금지`, explicit Weapon mount의 `WeaponData::SupportsMountType()` 일치 규칙으로 fail-closed했다. `RequiredMountType=None` wildcard와 concrete Vehicle MountProfile/SizeLimit의 EBA-P0-04 ownership은 보존한다.
- focused acceptance를 transient deterministic fixture 기반 `WeaponGuidedSuccess`, `ScannerGuidedSuccess`, `NativeChildFailurePropagation`, `IntrinsicMountContradiction` exact4로 보강했다. 기존 P002 exact5와 합쳐 expected P002 exact9, P001 exact3과 합친 affected EquipmentAuthoring expected exact12를 확정했다.
- P2 mode 전환 selection loss는 VM session-local mode cache만 추가해 active semantic Draft의 mutual exclusivity를 유지하면서 Weapon/Scanner 이전 선택을 복원하도록 교정했다. persisted schema/framework/Product mutation은 추가하지 않았다.
- fresh Official UE 5.8 Build job `937a8e0bd2ea4cf58be04d948484da60`에서 `CFEquipmentBuilderTests.cpp`, `CFEquipmentBuilderVM.cpp`, `CFEquipmentBuilderTab.cpp` compile은 PASS했으나 별도 `UCFSUVMigrateCommandlet` constructor/Main unresolved external로 최종 link가 FAIL(exit 6)했다. `CFSUVMigrateCmdlet.h` worktree diff는 0이고 current Source에 method implementation은 0으로 확인했다.
- unrelated migration commandlet을 EBA 범위에서 임의 수정하지 않았으며 current DLL이 link되지 않았으므로 stale binary를 피하기 위해 fresh Automation은 Not Run으로 남겼다. Overall EBA-P0-02 Technical Re-acceptance는 `HOLD / VALIDATION BLOCKED`, exact next는 blocker 해결 후 Official Build → P002 exact9 → affected exact12다.
- SavePackage/CreatePackage/MarkPackageDirty/Modify/CFDADurableCore/RuntimeApply execution seam과 Product/child Asset write/save는 계속 0이며 `EBA-P0-03` 구현은 시작하지 않았다.

### v0.1.5 - 2026-09-16

- `EBA-P0-02 Post-Implementation Mid-review`를 fresh Source/runtime contract 기준으로 수행하고 `P0 0 / blocking P1 2 / P2 1 / HOLD`로 판정했다. 기존 Official UE 5.8 Build PASS와 focused5/affected8 PASS evidence는 실행 사실로 보존한다.
- P1-1로 Scanner `RequiredMountType=Turret`, Weapon `RequiredMountType=Utility`, explicit Weapon required mount와 WeaponData compatible mount의 직접 모순처럼 concrete Vehicle 없이 증명 가능한 intrinsic mount contradiction이 현재 child validation에서 blocking되지 않는 점을 확인했다. RequiredMountType=None wildcard와 EBA-P0-04 concrete MountProfile ownership은 보존한다.
- P1-2로 `PersistedSelectorRoundTrip`이 첫 eligible branch에서 종료하고 `ExistingChildValidation`이 Items.Num만 확인해 Weapon/Scanner principal success path와 native-invalid propagation을 결정적으로 모두 증명하지 못하는 focused acceptance gap을 확인했다.
- P2-1로 Draft Mode 전환이 opposite payload selection을 즉시 transient clear하여 신규 Draft에서 전환 전 확인/복구가 없는 UX risk를 기록했다. Product mutation 위험은 없으므로 non-blocking이다.
- mutation seam 재검수에서 SavePackage/CreatePackage/NewObject/Modify/MarkPackageDirty/CFDADurableCore/RuntimeApply execution은 0이고 child DataAsset direct write/save 0을 재확인했다. 이번 Mid-review 자체 Source/Asset mutation과 Build/Automation rerun은 0이다.
- exact next를 `EBA-P0-02 Mid-review Correction + Re-review`로 되돌리고 P0/P1 0 전 `EBA-P0-03 Review / Durable Apply-and-Save` 착수를 금지했다. Current Active CF-FQ-039는 unchanged다.

### v0.1.4 - 2026-09-16

- `EBA-P0-02 Weapon / Scanner Guided Composition + Child Validation`을 구현하고 fresh Technical Validation을 완료해 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 전진했다.
- Weapon Draft에는 exact typed `TurretMountData + WeaponData`, Scanner Draft에는 exact typed `VehicleSensorData` picker를 연결했으며 selection은 transient SoftObject reference만 갱신한다. 반대 mode selector는 Asset load 전에 fail-closed한다.
- UI-only mode 전환 시 반대 payload reference만 transient Draft에서 제거해 신규 mixed payload 생성을 막는다. 기존 persisted mixed payload는 자동 수정하지 않고 validation Error로 노출한다.
- Weapon `ValidateWeaponDataContract`, Sensor `ValidateSensorDataContract`, Ammo `IsAmmoDataValid`을 기존 native authority 그대로 재사용했다. Turret/Projectile/Damage는 formal native validator 부재를 존중해 `제한 검증`/`확인 필요`로 분리하고 전체 유효 판정으로 확대하지 않았다.
- child validation UI는 `[통과] / [제한 검증] / [확인 필요] / [오류]`를 구분하며 concrete Vehicle Mount compatibility는 EBA-P0-04 owner로 유지했다.
- final Official UE 5.8 Build `881eecfcfa7b4c96beddba6939838e76` PASS, EBA-P0-02 focused exact5/5 PASS, affected `CarFight.EquipmentAuthoring` exact8/8 PASS를 확보했다. 최종 affected process job은 `6cfddbc20b1741cd821ba91fad5ad287`이다.
- EquipmentAuthoring에서 durable/runtime mutation execution seam은 0이며 EBA scope Product Asset mutation/save 0, child DataAsset write/save 0을 유지했다. 기존 병렬 Product dirty는 소유하지 않고 보호한다.
- CF-FQ-054 lifecycle은 Ready, current Active CF-FQ-039는 unchanged다. exact next는 `EBA-P0-03 Review / Durable Apply-and-Save`다.

### v0.1.3 - 2026-09-16

- `EBA-P0-01 Editor Shell / Browser / Draft Model`을 구현하고 fresh Technical Validation을 완료해 `P0 0 / blocking P1 0 / P2 0 / Technical PASS`로 전진했다.
- 독립 `SCFEquipmentBuilderTab` / `FCFEquipmentBuilderVM` / transient DTO를 신규 구현하고 `CarFight.EquipmentBuilder` Nomad Tab 및 `Window > CarFight > CarFight 장비 제작 가이드` 진입을 등록했다. Vehicle Builder business code 상속·공용화·수정은 하지 않았다.
- existing `FCFDAManagementVM`의 metadata-only inventory를 browser authority로 재사용하고 exact EquipmentPreset 선택 시에만 read-only load하여 authored exact7을 transient Draft로 복사한다. 신규 Draft는 persisted target/object를 만들지 않는다.
- Weapon/Scanner mode를 persisted enum이 아닌 UI-only state로 유지하고 Scanner `RequiredWeaponSize=None`, `RequiredMountType=None` wildcard current 의미를 보존했다. child reference는 EBA-P0-02 전까지 read-only 표시만 한다.
- EquipmentAuthoring Source에서 durable mutation execution seam `SavePackage/CreatePackage/NewObject/Modify/MarkPackageDirty`를 사용하지 않으며 Apply/Save UI도 0이다. EBA scope Product Asset mutation/save 0을 유지했다.
- first real Official Build의 test-only `TSharedRef::IsValid` compile defect를 `SNullWidget` 비교로 교정한 뒤 final UE 5.8 Build `24ed44a335e84ee7a0e1e4e0a7418594` PASS를 확보했다.
- focused `CarFight.EquipmentAuthoring.P001`은 BrowserInventory / DraftModel / NativeSlateTab exact3/3 PASS, failure 0, finding 0이다.
- post-validation에서 별도 Basic Sensor Product untracked path가 concurrent change로 관측됐으나 EBA Source의 create/save path와 연결되지 않으며 기존 Vehicle Authoring Project Basic Sensor 계약으로 분류했다. 해당 외부 dirty는 수정·삭제하지 않고 보호한다.
- CF-FQ-054 lifecycle은 Ready, current Active CF-FQ-039는 unchanged다. exact next는 `EBA-P0-02 Weapon / Scanner Guided Composition + Child Validation`이다.

### v0.1.2 - 2026-09-16

- `EBA-P0-00 Design Review`의 blocking P1 exact4를 contract correction하고 fresh 재검수해 `P0 0 / blocking P1 0 / P2 0 / Technical Contract PASS`로 종료했다.
- `CFDADurableCore`를 EquipmentPreset Create/Update/Save의 단일 durable authority로 동결하고 P0 사용자 mutation을 `검토 → 적용 및 저장` 단일 durable action으로 교정했다. 별도 SavePackage/rollback/reload 엔진과 CFDA provider exact5/DACE onboarding은 요구하지 않는다.
- Equipment authored exact7 semantic snapshot, Literal FText lossless 의미, canonical SoftObjectPath, Revision 1, ReviewProposalDigest와 Commit 직전 fresh TOCTOU revalidation을 동결했다.
- DataManagement의 Required/ExplicitFName/EquipmentId/ExactClassPath identity semantics를 prospective duplicate guard에도 재사용하고 incomplete namespace coverage를 fail-closed하도록 했다.
- `RequiredMountType=None`은 current runtime wildcard이며 `HasCompleteEquipmentDataForMount(None)==false`와 구분하도록 계약을 교정했다. Scanner `RequiredWeaponSize=None`, DisplayName empty non-error 의미도 보존한다.
- fresh AssetDump persisted EquipmentPreset exact10을 protected baseline으로 유지하되 개별 path projection limitation 때문에 근거 없는 Product sub-root를 발명하지 않았다. Product Create target은 user-selected canonical `/Game/CarFight/.../Asset.Asset`, Test namespace는 금지, EquipmentId와 object name은 독립으로 동결했다.
- Automation disposable namespace를 `/Game/CarFight/Tests/EquipmentAuthoring/`으로 분리하고 mutation fixture residue exact0 behavior를 동결했다.
- EBA-P0-03 focused behavior surface를 durable success/collision/dirty/stale/duplicate/FText/child/rollback/save-uncertainty/residue 기준으로 동결했다.
- Product Source/Asset mutation과 Build/Automation은 0이며 exact next를 `EBA-P0-01 Editor Shell / Browser / Draft Model`로 전진했다. CF-FQ-054는 Ready, current Active CF-FQ-039는 unchanged다.

### v0.1.1 - 2026-09-16

- `EBA-P0-00 Contract Freeze + Vehicle Builder Reuse Baseline` 설계검수를 수행하고 `P0 0 / blocking P1 4 / P2 0 / HOLD`로 기록했다.
- current provider-neutral `CFDADurableCore`를 EquipmentPreset durable sequencing authority로 재사용하고 별도 Save/rollback 엔진을 만들지 않는 교정 조건을 추가했다.
- Vehicle Authoring의 fresh Preview / approval scope / current fingerprint 재검사 패턴을 Equipment-specific semantic snapshot과 TOCTOU binding으로 적용하도록 요구했다.
- DataManagement의 `EquipmentId` identity / ExactClassPath duplicate / `HasCompleteEquipmentData` custom contract와 Weapon/Sensor/Equipment runtime validation을 계층적으로 조합하도록 동결 조건을 추가했다.
- fresh AssetDump에서 persisted CFEquipmentPresetData exact10을 확인했으나 개별 Product path projection은 확정하지 못했으므로 Product creation root/naming/protection을 추측하지 않고 correction 대상으로 남겼다.
- Vehicle Builder 재사용 분류를 `Reuse As-Is / Light Extraction exact0 / Equipment-specific New / Do Not Share`로 동결했다.
- exact next를 `EBA-P0-00 Contract Correction + Re-review`로 전환하고 P0/P1 0 전 EBA-P0-01 착수를 금지했다.

### v0.1.0 - 2026-09-16

- `CF-FQ-054 Equipment Authoring / Guided Equipment Builder`를 P2 / Ready 정식 Plan으로 신규 등록했다.
- 2026-09-15 Equipment Authoring Ownership Audit을 Accepted baseline으로 승격했다.
- Equipment Builder의 P0 write authority를 `UCFEquipmentPresetData` exact1로 제한하고 child DataAsset / Vehicle Mount / Fitting / Inventory / RuntimeApply / Runtime Catalog ownership 경계를 명시했다.
- Vehicle Builder UX를 그대로 복제하거나 광범위 리팩터링하지 않고 EBA-P0-00에서 current implementation 기준 Reuse As-Is / Light Extraction / New / Do Not Share를 fresh 분류하도록 했다.
- exact next를 `EBA-P0-00 Contract Freeze + Vehicle Builder Reuse Baseline`으로 등록했다.

---

## 15. Migration

- 이 Feature 이전에 수동으로 생성·편집된 EquipmentPresetData와 child DataAsset은 자동 migration하지 않는다.
- Equipment Builder가 도입돼도 기존 Runtime/Fitting/VehicleData 계약은 authority를 유지한다.
- 기존 AmmoData/DamageData Data Asset Authoring 지원은 Equipment Builder 내부 writer로 이동하지 않는다.
- 신규 generic EquipmentType enum이나 새로운 Equipment payload category는 이 Plan 승격 자체로 추가되지 않는다.
- `CF-FQ-039` Active lifecycle과 `CF-FQ-041` Ready lifecycle은 CF-FQ-054 승격으로 변경되지 않는다.
