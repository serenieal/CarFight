# Vehicle Builder Hardpoint Authoring Integrity Plan

- Version: 0.2.1
- Date: 2026-09-05
- Feature: CF-FQ-047 Vehicle Builder Hardpoint Authoring Integrity
- Priority: P1
- Status: Done / P0 Complete / VBHAI-P0-07G USER Re-Acceptance PASS / fresh persisted Driving receipt readback PASS / VBHAI-P0-08 Current System Promotion Complete / Historical + Archived Path / G5 Physical Move Complete
- Current Active Feature: CF-FQ-039 Production UI Visual Rework 유지
- Representative Plan: Document/Plan/Archive/VehicleBuilderHardpointIntegrity/VehicleBuilderHardpointIntegrityPlan.md
- Current System Owner: Document/Systems/Vehicles/VehicleBuilder.md v1.4.1
- Historical Upstream: CF-FQ-043 Done / Historical
- Product Recovery Target: /Game/CarFight/Data/Authoring/DA_Vehicle_Wagon

---

## 1. 목적

Vehicle Builder에서 StaticMesh의 Hardpoint-like Socket과 실제 Runtime Hardpoint/Mount semantic data가 어긋난 채 차량 제작이 완료되는 상황을 방지한다.

CF-FQ-043은 재오픈하지 않는다. 기존 UseHardpoints, LegacyCompatible preservation, no-cascade remove, shared StaticMesh Socket 보존 계약을 유지한다.

## 2. Incident Baseline

fresh evidence 기준 Wagon Chassis에는 Wheel_Anchor_FL/FR/RL/RR와 HP_Top_01이 존재한다.

HP_Top_01:
- RelativeLocation = (-50, 0, 130)
- RelativeRotation = (0, 0, 0)
- Preview Mesh = Base_Standard

persisted DA_Recipe_Wagon:
- BuilderHardpointPlanMode = LegacyCompatible
- HardpointIntents = 0
- MountIntents = 0

persisted DA_Vehicle_Wagon:
- HardpointSlots = 0
- MountProfiles = 0

따라서 RuntimeApply의 대상 Mount/Profile 없음은 현재 VehicleData를 정확히 반영한 정상 consumer 결과다.

---

## 3. RCA 분류

### 보존해야 하는 정상 계약

1. Socket은 물리 위치이고 Hardpoint/Mount는 별도 semantic이다.
2. Hardpoint 삭제가 shared StaticMesh Socket을 자동 삭제하지 않는다.
3. LegacyCompatible existing/custom/multi-Mount 구조를 Standard 1:1로 silent migration하지 않는다.
4. Recipe/StaticMesh/DefinitionApply는 자동 저장하지 않는다.

### 발견된 gap

A. Unbound Hardpoint-like Socket visibility
- Chassis에 HP_Top_01이 있어도 현재 차량 semantic에 연결되지 않았다는 사실을 Builder가 명확히 보여주지 않는다.

B. Final runtime semantic visibility
- Step 7/8에서 HardpointSlots / MountProfiles 최종 결과가 0인지 쉽게 확인하기 어렵다.

C. Production UAT fixture restoration
- CF-FQ-043에서 Hardpoint 삭제 후 Socket 잔존을 검증했지만 Product Wagon의 intended semantic state를 exact restore + Save + persisted readback하는 closure Gate가 없었다.

현재 residue의 역사적 직접 원인은 완전한 event log가 없으므로 다음을 구분한다.
- 현재 Asset state는 확정 사실.
- remove가 Socket을 남기는 계약은 확정 사실.
- Recipe/StaticMesh save lifecycle 분리는 확정 사실.
- 당시 UAT residue가 현재 상태를 만들었다는 것은 강한 가설.

---

## 4. 안전 원칙

### 자동 semantic migration 금지

HP_* 발견만으로 자동 UseHardpoints, 자동 HardpointIntent, 자동 MountIntent를 만들지 않는다.

shared Chassis에서 다른 차량용 Socket일 수 있고 MountType/SizeLimit은 Socket geometry만으로 결정할 수 없다.

### Resolver AssetSnapshot과 orphan inventory 분리

unbound Socket 탐지를 위해 existing `FCFVehicleAssetSnapshot.ChassisSockets`를 전체 Chassis Socket inventory로 확장하지 않는다.

현재 AssetSnapshot은 Resolver가 실제로 요청한 Wheel Socket + Recipe Hardpoint Socket만 읽고 `ChassisLayoutFingerprint`에 포함한다. CF-FQ-047의 orphan 진단은 이 fingerprint/Resolve lane과 완전히 분리된 Editor-only read-only inventory를 사용한다.

권장 typed owner:
- `FCFBuilderChassisSocketInventory`
- `FCFBuilderChassisSocketInventoryEntry`
- `ReadCurrentChassisSocketInventory()` 또는 동등한 Builder-private R0 seam

이 inventory는 다음에 참여하지 않는다.
- Resolver input
- ChassisLayoutFingerprint
- RecipeFingerprint
- ResolvedDefinitionHash
- DefinitionApply Diff
- Driving acceptance freshness

unbound Socket의 추가/삭제만으로 위 semantic fingerprint/hash가 변하면 설계 실패다.

### USER explicit adoption

unbound Standard candidate는 read-only로 표시하고 USER가 명시적으로 현재 차량 장착점으로 사용할 때만 typed authoring을 수행한다.

mode transition과 adoption은 **두 단계**로 분리한다.

1. `LegacyCompatible / Unspecified / NoHardpoints`에서는 candidate를 advisory로 볼 수 있지만 adoption mutation 버튼은 활성화하지 않는다.
2. USER가 기존 `장착 위치 사용` 결정으로 `CommitHardpointPlanMode(UseHardpoints)`를 먼저 명시 실행한다.
3. fresh refresh 뒤 `UseHardpoints`에서만 `[이 Socket을 장착 위치로 사용]` adoption action을 활성화한다.
4. adoption operation은 mode를 바꾸지 않고 exact existing Socket binding만 Recipe에 typed write한다.

이렇게 하면 mode transaction 성공 뒤 adoption 내부 실패를 하나의 복합 transaction으로 rollback해야 하는 새 권한을 만들지 않는다. 기존 CF-FQ-043 E7 `LegacyCompatible → explicit UseHardpoints → new Standard row` preservation 회귀를 그대로 재사용한다.

adoption stable ID는 SocketName에서 exact Standard grammar로 파생하며 max-used+1 신규 생성 규칙을 사용하지 않는다.

표시 의미 예:
- HP_Top_01
- Chassis에는 존재
- 현재 차량 Hardpoint에 연결되지 않음
- 이 상태만으로는 장비 장착 불가
- UseHardpoints 상태에서만: 이 Socket을 장착 위치로 사용

### shared Chassis 보호

- Socket 자동 삭제/이동/회전 금지
- unrelated vehicle semantic mutation 금지
- per-Vehicle Socket override 신규 도입 금지
- existing Socket adoption 시 duplicate Socket 생성 금지

---

## 5. 목표 흐름

### Step 3 — Unbound Socket Integrity

current Chassis의 전체 Socket inventory는 Resolver AssetSnapshot과 분리된 Builder-private read-only lane에서만 읽는다.

classification은 prefix 하나가 아니라 Standard grammar + current semantic collision을 함께 본다.

#### StandardAdoptable

다음을 모두 만족해야 한다.
- SocketName이 exact `HP_<LocationSlotId>` 형태.
- `<LocationSlotId>`가 canonical Standard `<KnownCategory>_<Index>` grammar를 만족.
- KnownCategory는 current Standard category 집합 `Top / Front / Back / LeftSide / RightSide / Bottom / Internal` 중 하나.
- Index는 양의 10진 정수이고, `FString::Printf("%s_%02d", Category, Index)`와 exact round-trip한 LocationSlotId만 canonical로 인정한다. 따라서 `Top_01`, `Top_10`, `Top_100`은 허용하지만 `Top_1`, `Top_001`은 NonCanonicalHardpointLike로 분류한다.
- Recipe HardpointIntent가 exact SocketName을 사용하지 않음.
- Target HardpointSlots도 exact SocketName을 사용하지 않음.
- 파생 LocationSlotId가 Recipe/Target의 다른 Socket binding과 identity collision하지 않음.

#### NonCanonicalHardpointLike

`HP_` prefix는 있지만 Standard grammar를 만족하지 않는 Socket.
- USER 정보/advisory로만 표시 가능.
- Standard adoption action 노출 금지.
- Legacy/custom identity를 silent normalize하지 않음.

#### AlreadyRecipeBound / AlreadyTargetBound

현재 semantic이 이미 exact Socket을 사용 중이면 orphan candidate가 아니다.

#### IdentityCollision

SocketName에서 파생한 LocationSlotId가 Recipe/Target의 다른 binding 또는 stable identity와 충돌하면 adoption을 fail-closed한다.

#### UnrelatedSocket

Hardpoint candidate가 아닌 일반 Chassis Socket은 CF-FQ-047 UI에 올리지 않는다.

기존 `AddStandardHardpoint(Category)`는 max-used+1 신규 생성 API이므로 existing Socket adoption에 재사용하지 않는다. adoption은 exact SocketName에서 Standard grammar를 parse해 exact LocationSlotId를 고정한 뒤 collision을 검사하는 별도 typed operation/seam을 사용한다.

### Step 6 — Mount completeness

기존 UseHardpoints completion 계약을 유지한다.
- Hardpoint 1 / Mount 0 → incomplete
- Hardpoint 1 / Mount 1 → exact 1:1 + Type/Size/Preset validation 후 Complete

unbound Socket 존재 자체는 completion blocker가 아니다. blocker는 authored semantic incompleteness와 invalid relation이다.

mode별 unbound Socket severity:
- LegacyCompatible: advisory only. 기존 차량 migration 강제 금지.
- Unspecified: plan decision을 돕는 참고 정보. 기존 Unspecified blocker는 mode 미결정 자체가 소유.
- NoHardpoints: 이 차량에서는 사용하지 않는 Chassis Socket 정보. unbound 존재만으로 Block 금지.
- UseHardpoints: 추가 adoption 가능한 candidate. authored Hardpoint/Mount completion과 별도로 취급하며 unbound 존재만으로 Block 금지.

USER에게 Hardpoint↔Mount 대응 관계를 명확하게 보여주되 current Guidance result를 completion authority로 유지한다.

### Step 7 — Current vs Prospective semantic readback

Step 7에는 서로 다른 truth를 섞지 않는다.

Apply 전 Final Review는 최소 다음을 분리해 보여준다.
- 현재 VehicleData: current Target readback 기준 HardpointSlots / MountProfiles.
- 적용 후 결과: approved/fresh Prospective Resolve 기준 HardpointSlots / MountProfiles.
- stable relation: 예 `Top_01 → HP_Top_01 → Mount_Top_01`.

예:
- 현재 VehicleData: Hardpoint 0 / Mount 0
- 적용 후: Hardpoint 1 / Mount 1

Recipe array count를 Runtime 적용 결과처럼 표시하지 않는다. DefinitionApply owner는 기존 Step 7을 유지한다.

### Step 8 — Current Target semantic summary

Driving Test contract는 재설계하지 않는다.

Step 8의 장착 summary는 saved/current Target VehicleData readback 기준으로만 표시한다.
- current Target HardpointSlots count
- current Target MountProfiles count
- 필요 시 exact stable relation summary

Prospective Recipe/Resolve count를 Step 8 current Runtime 결과처럼 표시하지 않는다. Runtime Pawn live readback은 P0 Scope에 추가하지 않는다.

CF-FQ-047은 typed integrity fact / completion owner이고, CF-FQ-046은 공통 USER-facing Presentation architecture owner다. 별도 중복 Presentation 체계를 만들지 않는다.

---

## 6. Wagon Product Recovery

현재 HP_Top_01 USER-authored transform을 재사용한다.

### VBHAI-D1 — Wagon Roof Mount Policy Decision

Product recovery mutation 전에 USER/Design이 최소 다음을 명시 확정한다.
- MountType: Fixed / Gimbal / Turret / Launcher / Utility
- SizeLimit: Small / Medium / Large, 단 Utility는 current contract에 따라 None 허용
- DefaultEquipmentPreset: optional

MountType/SizeLimit은 Socket geometry나 이름만으로 추측하지 않는다.

목표 Recipe:
- BuilderHardpointPlanMode = UseHardpoints
- HardpointIntent.LocationSlotId = Top_01
- HardpointIntent.SocketName = HP_Top_01
- MountIntent.MountProfileId = Mount_Top_01
- MountIntent.LocationSlotRef = Top_01
- MountType / SizeLimit = VBHAI-D1 승인값

Step 7 Apply 뒤 목표:
- DA_Vehicle_Wagon.HardpointSlots = 1
- DA_Vehicle_Wagon.MountProfiles = 1

DefinitionHash가 변경되면 기존 USER Driving PASS가 stale 처리되어야 하므로 새 Technical Benchmark + USER Driving PASS 재승인까지 recovery completion에 포함한다.

---

## 7. Product UAT Restoration Contract

Product Asset을 destructive UAT에 사용할 경우:

Pre-UAT intended product snapshot
→ destructive test
→ behavior PASS
→ intended Product state exact restore
→ explicit Save
→ fresh AssetDump persisted readback
→ closure

snapshot 최소 범위:
- Recipe.BuilderHardpointPlanMode
- Recipe.HardpointIntents exact array
- Recipe.MountIntents exact array
- VehicleData.HardpointSlots exact array
- VehicleData.MountProfiles exact array
- relevant HP_* StaticMesh Socket exact names + transform
- current Target DefinitionHash
- USER Driving acceptance current/stale 여부

Historical P0-06에서는 고장 baseline `0/0`에서 intended Product state `1/1`로 복구했고, 당시 Product UAT restoration baseline은 fresh persisted 1/1이었다. 이후 P0-07 USER UAT에서 USER가 두 번째 장착점/마운트를 Builder로 의도적으로 추가했고, USER가 해당 추가를 실제 Product에 유지하기로 명시 승인했다. 따라서 **현재 Product baseline은 semantic-alignment 검증을 통과한 2/2**이며 1/1로 강제 복구하지 않는다. 이후 Product UAT restoration은 테스트 시작 직전의 current accepted baseline exact snapshot을 사용하며 특정 개수(1/1)를 영구 전제로 두지 않는다.

원칙:
1. remove/undo/collision/negative path는 가능하면 test-only fixture에서 수행한다.
2. Product Wagon은 최종 정상 E2E acceptance에 우선 사용한다.
3. Product Asset을 negative fixture로 사용했다면 restoration evidence 없이는 USER Acceptance를 닫지 않는다.
4. shared Chassis StaticMesh residue도 intended Product state 기준으로 판정한다.

---

## 8. 책임 배분

### C++

- unbound Standard HP_* candidate deterministic read-only classification
- Recipe/Target exact binding comparison
- explicit existing Socket adoption typed lane
- collision/dependency/mode guard
- Step 3/6 integrity facts
- Step 7/8 typed runtime semantic facts
- focused/affected Automation

### Slate / Presentation

- unbound candidate 표시
- explicit adoption action
- Hardpoint↔Mount 대응 상태
- Final/runtime summary

공통 formatter/layout/error presentation은 CF-FQ-046 owner를 재사용한다.

### Blueprint

P0에서 신규 gameplay Blueprint 로직은 추가하지 않는다.

---

## 9. Scope Out

- Hardpoint 자동 geometry 배치
- Socket prefix만으로 silent migration
- Socket 자동 삭제/위치/회전 보정
- per-Vehicle Socket geometry override
- 새 Runtime Hardpoint 시스템
- RuntimeApply/Fitting/Inventory 재설계
- MountType/SizeLimit 자동 추론
- CF-FQ-043 Historical 재개
- CF-FQ-046 Presentation 전체 재구현
- 자동 Save

---

## 10. 병렬 작업 보호

현재 CF-FQ-046이 다음 shared Builder Source를 수정 중이다.

- CFVehicleBuilderTab.cpp/.h
- CFVehicleBuilderVM.cpp/.h
- CFVehicleBuilderTypes.h
- CFVehicleBuilderTests.cpp
- CFVehicleBuilderPresent.cpp/.h

따라서:
- VBHAI-P0-00 / P0-01 read-only audit/design은 즉시 가능하다.
- P0-02 implementation 전 fresh main_game status/diff 재확인은 필수다.
- CF-FQ-046 compile/implementation checkpoint를 보호한다.
- CF-FQ-045 DataManagement, CF-FQ-041 RuntimeApply, CF-FQ-039 UI dirty는 비접촉한다.
- Editor restart/ownership takeover는 금지하고 attach-first 정책을 유지한다.

---

## 11. 단계 계획

### VBHAI-P0-00 — Current Contract + Incident Evidence Audit

목표:
- current Socket/Hardpoint/Mount/DefinitionApply/RuntimeApply owner를 Source 기준 재확정
- Wagon persisted evidence 고정
- FQ-043 UAT remove/no-cascade/save lifecycle과 current residue의 causal boundary 분리
- CF-FQ-046 shared Source overlap 확정
- 구현 mutation 0

Exit:
- P0/P1 설계 전제 모순 0
- 확정 사실과 역사적 가설 분리
- Product recovery target exact path 고정

### VBHAI-P0-01 — Detailed Design Review

확정 대상:
- unbound candidate exact classification
- LegacyCompatible warning/adoption 상태기
- existing HP_* adoption stable LocationSlotId 규칙
- mode transition approval semantics
- custom/nonstandard Socket false-positive 방지
- Step 7/8 typed summary owner
- CF-FQ-046 presentation integration 경계
- Product UAT restoration Gate

Design Audit 후 P0/P1 0 전 구현 금지.

### VBHAI-P0-02 — Unbound Socket Integrity + Adoption — Implementation Complete

구현:
- 신규 Editor-only owner `CFVehicleBuilderHardpointIntegrity.h/.cpp` 추가.
- Resolver AssetSnapshot과 분리된 Builder-private read-only Chassis Socket inventory 구현.
- `StandardAdoptable / NonCanonicalHardpointLike / AlreadyRecipeBound / AlreadyTargetBound / IdentityCollision / UnrelatedSocket` deterministic classification 구현.
- exact canonical `HP_<KnownCategory>_<Index>` parse + `%02d` round-trip validation 구현.
- `ReadCurrentChassisSocketInventory()` read-only Builder seam 추가.
- `AdoptExistingStandardHardpointSocket()` exact existing Socket adoption typed wrapper 추가.
- existing `AddStandardHardpoint(Category)` max-used+1 신규 생성 API와 adoption API를 분리 유지.
- Step 3에서 StandardAdoptable / NonCanonicalHardpointLike / IdentityCollision만 USER-facing integrity row로 표시.
- `LegacyCompatible / Unspecified / NoHardpoints`에서는 adoption 버튼 disabled, USER explicit `UseHardpoints` 전환 뒤에만 adoption 허용.
- existing Socket adoption은 `UpsertHardpointIntent`만 사용하며 StaticMesh transform / Target VehicleData / Save mutation 0.
- unbound Socket 존재 자체 completion blocker 0. 기존 Gameplay Guidance completion authority 변경 0.

Source audit:
- 신규 helper에 `Modify / MarkPackageDirty / PostEditChange / SavePackage / FScopedTransaction` 0.
- 신규 helper의 `FCFVehicleAssetSnapshot` / `ChassisLayoutFingerprint` / `ResolvedDefinitionHash` 참조는 boundary 주석 외 0.
- CF-FQ-046 `ReadCurrentLayoutFacts / ReadCurrentPrivateProfilePayload / Presentation` hunk를 보존.

Compile evidence:
- build job `8dbc3d87250a40fb87c52cf8c2adae06`.
- C++ compile `[1/11]~[8/11]` PASS: 신규 helper, BuilderVM, BuilderTab 포함.
- static library link `[9/11]` PASS.
- final Editor DLL link는 실행 중인 `UnrealEditor.exe`가 `UnrealEditor-CarFight_ReEditor.dll`을 보유해 `LNK1104`로 차단됨.
- Editor를 종료/재기동해 ownership을 뺏지 않는다. official linked binary PASS는 계획대로 P0-05 Technical Validation에서 다시 확보한다.

Product mutation:
- Wagon Recipe / VehicleData / StaticMesh 신규 mutation 0.
- RuntimeApply mutation 0.

### VBHAI-P0-03 — Mount Completion + Final Runtime Readback — Implementation Complete

구현:
- `FCFBuilderHardpointSemanticFacts`와 `FCFBuilderHardpointMountRelationFact` typed readback DTO를 `CFVehicleBuilderHardpointIntegrity` owner에 추가.
- Step 6는 Recipe authored Hardpoint↔Mount relation만 표시하고 기존 Gameplay Guidance를 completion authority로 유지.
- Step 7은 current Target VehicleData count와 fresh FinalReview structural FieldDiff 기반 prospective count를 분리 표시.
- Step 8은 current Target VehicleData HardpointSlots/MountProfiles count만 표시하고 Recipe/prospective count를 Runtime 결과처럼 사용하지 않음.
- Target UObject가 없는 경우 `bHasCurrentTarget=false`로 처리해 false `0/0` 표시를 금지.
- CF-FQ-046 `CFVehicleBuilderPresentation` owner에 pure formatter만 확장하고 별도 Presentation architecture를 만들지 않음.

검증:
- 첫 build `72c893ddf7984dd69c13432530cf8ad2`에서 enum 표기 `ECFAuthoringOpStatus::Success` 1건을 발견하고 actual `Succeeded`로 교정.
- corrected official Editor build `ad458f2ab93744d696317886db0542ae` Exit 0 / DLL link PASS.
- P0-04 test source 추가 뒤 official Editor build `2ecd8e1ba4f0461bb837f002937820a6` Exit 0 / test compile + DLL link PASS.

### VBHAI-P0-04 — Restoration Guard + Automation — Focused Complete

추가:
- `CFVehicleBuilderHardpointIntegrityTests.cpp`
- `Tools/RunHardpointIntegrityTests.ps1`
- `Tools/RunHardpointIntegrityAffectedTests.ps1`

Focused Automation job `08c40cebf651489c94d7cd4805dfbb1e`:
- InventoryClassification PASS
- SemanticReadback PASS
- PresentationReadback PASS
- **3/3 PASS**

정적 mutation boundary 재검수:
- helper `MarkPackageDirty / SavePackage / PostEditChange / Modify` 0
- helper `ChassisLayoutFingerprint / ResolvedDefinitionHash` mutation/read authority 0
- focused fixture Product Save mutation 0

최소 coverage:
- A. UseHardpoints Hardpoint1/Mount0 → incomplete
- B. Hardpoint1/Mount1 → Complete
- C. LegacyCompatible + unbound Standard HP_* → advisory / auto semantic mutation 0
- D. NoHardpoints + unbound Standard HP_* → unbound 존재만으로 Block 0
- E. shared Chassis extra HP_* → unrelated vehicle mutation 0
- F. unbound HP_* add/remove → Resolver AssetSnapshot requested set unchanged / ChassisLayoutFingerprint unchanged / ResolvedDefinitionHash unchanged / current Driving acceptance unchanged
- G. NonCanonical `HP_*` → Standard adoption action 0
- H. exact Standard grammar candidate → parsed LocationSlotId exact / identity collision fail-closed
- I. existing HP_* adoption → Socket duplicate 0 / exact existing transform unchanged
- J. Hardpoint remove → Recipe semantic delete / Socket preserve
- K. Step7 pre-Apply → current Target 0/0과 prospective 1/1을 분리 표시
- L. Step7 Apply → VehicleData HardpointSlots/MountProfiles exact
- M. Step8 → saved/current Target VehicleData count authority 사용
- N. DefinitionHash drift → previous Driving PASS stale
- O. Product fixture restoration snapshot/restore contract

### VBHAI-P0-05 — Focused + Affected Technical Validation — PASS

Technical evidence:
- official Editor build `2ecd8e1ba4f0461bb837f002937820a6` Exit 0 / DLL link PASS.
- CF-FQ-047 focused Automation 3/3 PASS.
- affected runner job `0c6ff459b6504ae481f3d3c09bc3e10d` terminal log readback에서 `VBHAI_AFFECTED_AUTOMATION=PASS` 확인.
- CF-FQ-043 affected 5/5 PASS: RecipeStateTypedRemove / Step3HardpointPlanning / Step6MountPlanning / ExistingPreservation / ContractMatrix.
- CF-FQ-040 Builder Step7FinalReview / Step8Driving 2/2 PASS.
- CF-FQ-046 PresentationStates / FinalReviewDiff / DrivingSummary 3/3 PASS.
- affected total 10/10 PASS.
- CF-FQ-041 RuntimeApply Source overlap은 0이므로 별도 RuntimeApply replay는 요구하지 않음.

Product Wagon mutation 전에 다음을 PASS해야 한다.
- official Editor build
- CF-FQ-047 focused Automation
- CF-FQ-043 Hardpoint/Mount focused suites
- CF-FQ-040 Builder Step 3/6/7/8 relevant suites
- CF-FQ-046 affected Presentation tests if implementation overlap exists
- CF-FQ-041 RuntimeApply relevant mount readback only if source overlap 발생

기존 PASS는 실제 overlap/new failure 없는 범위까지 무조건 broad replay하지 않는다.

**P0-05 PASS 전 Production Wagon Recipe/VehicleData recovery mutation 금지.**

### VBHAI-P0-06 — Wagon Product Recovery — Technical PASS / USER Driving Re-Acceptance PASS / Complete

VBHAI-D1 approved:
- MountType = Turret
- SizeLimit = Large
- DefaultEquipmentPreset = None
- 근거: persisted HeavyCannon(`Prototype Roof Cannon Kit`)과 RocketLauncher preset 모두 RequiredMountType=Turret / RequiredWeaponSize=Large.

Product recovery persisted readback:
- `DA_Recipe_Wagon.BuilderHardpointPlanMode = UseHardpoints`.
- Recipe HardpointIntent exact 1: `Top_01 → HP_Top_01`.
- Recipe MountIntent exact 1: `Mount_Top_01 → Top_01 / Turret / Large / DefaultPreset=None`.
- `DA_Vehicle_Wagon.HardpointSlots = 1`, exact `Top_01 → HP_Top_01`.
- `DA_Vehicle_Wagon.MountProfiles = 1`, exact `Mount_Top_01 → Top_01 / Turret / Large`.
- current applied Target DefinitionHash = `8d780eb07ee7ab4672233fd3d15a0db4`.
- old USER Driving receipt TargetDefinitionHash = `0e5b48e8dcd39deba441da9237218be6`이므로 current Definition에 대해 정상적으로 stale이다.
- fresh persisted 1/1 state를 이후 Product UAT restoration baseline으로 고정한다.

RuntimeApply closure:
- actual Wagon RuntimeApply UI에서 `Mount_Top_01` option exact 1 / selection PASS.
- Production HeavyCannon과 RocketLauncher는 둘 다 `TurretMountWeightKg=350`, `WeaponMassKg=120`이다.
- Wagon은 `BaseVehicleMassKg=1886`, `MaximumGrossMassKg=2350`이므로 production 대형 터렛 적용 후보 총중량은 `2356kg`이며 `GrossMassExceeded`로 ValidationFailed되는 것이 current Fitting contract상 정상이다.
- `MaximumGrossMassKg=2350`은 VehicleBase Reference/`CLAIM-V60-GROSS-MASS` authority를 보존한다. CF-FQ-047을 닫기 위해 2356으로 임의 상향하지 않는다.
- Runtime active-Mount handoff는 product mass compatibility와 분리한 pure policy `CFRuntimeEquipApplyPolicy::ResolveCandidateActiveMount`가 `PreserveCurrent / HandoffToTarget / NoActiveWeapon / RejectNonWeaponTarget` resolution을 반환한다. current active weapon Mount가 candidate에 남으면 보존하고, 사라진 경우 USER target이 weapon-bearing일 때만 target으로 전환한다. 기존 active weapon이 있었는데 candidate에서 사라지고 target도 비무장이면 Fitting Prepare 전에 `RejectNonWeaponTarget`으로 명시 fail-closed한다.
- Mid-review P1 교정으로 downstream `BuildSnapshotRuntimeInput()`의 weapon-bearing 후보 존재 여부에 암묵 의존하던 경계를 제거했다. reject는 transient candidate 생성 뒤, Fitting Prepare/Commit과 Pawn runtime mutation 전에 `ValidationFailed`로 종료된다.
- `CarFight.RuntimeApply.CF_FQ_047.ActiveMountHandoffPolicy`는 current 보존 / target handoff / weapon-bearing 후보 0 + non-weapon target explicit reject / 원래 active weapon 없음의 4개 정책 분기를 PASS했다.
- `CarFight.RuntimeApply.CF_FQ_047.WagonMountEquipmentPIE`는 persisted Mount 표시/선택과 overweight Equipment fail-closed + runtime state preservation을 실제 PIE에서 재PASS했다.
- corrected RuntimeApply 전체 affected suite process `e1dfea68d9b94c36bcf96e8f795f1820`: **16/16 PASS / failure 0**, result JSON SHA-256 `29eef74aaa1617b313829fbc9a14f82b64d4e560a129175649ff9f35d5cd1732`.
- corrected official Editor build job `85f2cb2bd8de460c91d0d3ccaea8014f`: Exit 0 / corrected RuntimeApply policy·service·test compile + `UnrealEditor-CarFight_Re.dll` / `UnrealEditor-CarFight_ReEditor.dll` link PASS.

Step 8 Technical Benchmark:
- canonical `RunBuilderBench.ps1`, fixed 60Hz, saved Wagon Target, Fitting override 없음.
- RunId = `a9e28515-05ea-4cf9-bc0a-80a49adfc018`.
- ExpectedTargetDefinitionHash = `8d780eb07ee7ab4672233fd3d15a0db4`.
- runner status `success`, EngineExitCode=0, metric exact 1, reference threshold asserted=false, USER driving feel asserted=false.
- configured mass 1886kg, actual mass 2153.377kg, 0→50 2.750s, 0→100 미도달, peak 89.176km/h, peak RPM 6080.209 / gear 3, effective turning radius 7.722m.
- 0→100/100kmh braking 미도달은 runner 계약상 관측 결과이며 기술 실패가 아니다.

USER Driving Re-Acceptance closure evidence:
- USER가 기존 Editor를 사용해 Play를 시작했고 Step 8의 current saved Wagon을 active PIE Player Pawn에 transient 적용했다.
- USER가 직접 가속/변속/조향/제동/바퀴·차체 거동을 확인한 뒤 `PASS`를 명시했으며, 주행감이 이전보다 마음에 든다는 체감 판단을 남겼다.
- USER가 Step 8 `주행 테스트 통과` 확인을 명시 실행했다. live Slate readback에서 Step 8이 `[완료] / 상태: 완료`로 전환되고 Editor가 정확히 `1개 저장되지 않음`으로 바뀌어 production acceptance receipt write + Recipe dirty 경계를 확인했다.
- USER가 Recipe 변경을 explicit Save했다. 이후 USER가 Editor를 직접 종료/재기동했고 fresh Editor readback에서 `모두 저장됨`을 확인했다. AI는 ownership 확보를 위한 Editor restart/stop을 수행하지 않았다.
- `AcceptCurrentUserDriving()` production success path는 exact current `TargetVehicleDataPath`, `TargetDefinitionHash`, `AcceptedBenchmarkRunId`를 Recipe persistent receipt에 기록한 뒤 동일 identity로 legacy host-local token도 저장한다. current tool policy는 native nested Recipe receipt field를 직접 projection하지 않아 `.uasset` 내부 3필드 값을 별도 getter로 재노출하지 않았다.
- fresh saved acceptance token readback은 RecipeId `05F69DD34990A868DEFFD29C1AF5B2F9`, `TargetDefinitionHash=8d780eb07ee7ab4672233fd3d15a0db4`, `BenchmarkRunId=a9e28515-05ea-4cf9-bc0a-80a49adfc018` exact match다. 이 token은 persistent receipt와 동일 `AcceptCurrentUserDriving()` success path에서 함께 기록된다.
- fresh AssetDump는 저장된 exact `DA_Recipe_Wagon`을 `CFVehicleRecipeData`로 정상 로드했지만 current `data_asset_values` projection은 native nested receipt를 노출하지 않았다. 따라서 nested `.uasset` 값을 AssetDump가 직접 보여줬다고 과장하지 않고, production success path + Step 8 Complete/dirty transition + USER explicit Save + fresh `모두 저장됨` + exact identity token을 P0-06 closure evidence로 사용한다.
- RunId는 acceptance validity authority가 아니라 PASS 당시 technical benchmark provenance다. DefinitionHash/GrossMass/Benchmark semantic은 이번 USER 승인으로 변경되지 않았으므로 benchmark 재실행은 요구하지 않는다.

MountType/SizeLimit은 VBHAI-D1 승인값 `Turret / Large`를 그대로 유지한다.

### VBHAI-P0-07 — USER Acceptance — FAIL / Correction Design

2026-09-04 USER UAT에서 P0-06 직접 주행 PASS와 persisted Wagon 1/1 상태는 유지됐지만, 최종 제작 흐름의 사용성/상태 계약에서 다음 blocker가 확인됐다.

1. **CF-FQ-046 external blocker — 공통 Page/Expander/Scroll 양식 불통일**
   - Step 5 `물리 설정 상세 보기`와 Step 8 `진단 정보`는 서로 다른 결함이 아니라 같은 공통 Page shell 결함의 재현 사례다.
   - 047에서 두 화면을 개별 ScrollBox로 땜질하지 않는다.
   - CF-FQ-046이 소유하는 Step 1~8 공통 Page Height / Scroll / Overflow 계약을 선행 적용한다. 하나의 primary vertical scroll owner, expanded detail/diagnostic를 같은 content scroll region에 포함, footer action/navigation 상시 접근 가능이 acceptance 기준이다.

2. **P1 — Hardpoint/Mount-only structural drift가 Step 5 Physics를 불필요하게 stale 처리**
   - 현재 `PreviewBuilderProfiles()`의 persistent receipt exact-match는 `BuilderCommitReceipt.ProspectiveResolvedDefinitionHash == Preview.ProspectiveResolvedDefinitionHash`까지 요구한다.
   - 따라서 Physics private 4 Profile, Evidence, Transmission, Engine Curve, Resolver가 모두 그대로여도 Step 3/6의 Hardpoint/Mount 변경이 full resolved DefinitionHash를 바꾸면 `EvaluatePhysicsProposalStep()`이 Step 5를 Stale로 내린다.
   - `PrepareCurrentPhysicsReceiptRefresh()`가 이미 `HardpointSlots/MountProfiles`만 달라진 경우 Profile payload unchanged를 확인한 뒤 receipt-only rebind를 허용하고 있어, 현재 구조도 이 변화가 Physics Profile 재작성 사유가 아님을 별도 예외로 인정하고 있다.
   - USER 지적대로 Chassis Socket 생성 자체는 좌표/부착 기준점을 추가하는 것이며 질량·엔진·변속기·서스펜션을 바꾸지 않는다. Recipe Hardpoint/Mount semantic 추가도 현재 기본 차량 Physics Profile을 자동 변경하거나 실제 Equipment 질량을 올리는 write가 아니다.

3. **P1 — Step 8 PIE Apply readiness가 단일 truth/사용자 이유 없이 분산됨**
   - `현재 PIE에 선택 차량 적용`은 현재 `ViewModel valid + benchmark not running + Step8 + HasDrivingBenchmarkResult()`만으로 enable된다. PIE 존재 자체는 enable 조건이 아니며 실제 PIE World/PlayerController/CFVehiclePawn/saved Recipe/Target/hash 검사는 클릭 뒤 `ApplySelectedVehicleToActivePIE()`에서 수행된다.
   - UAT 화면은 current benchmark/USER Driving binding 요약이 보이는데 버튼은 비활성처럼 관측됐다. 당시 exact bool 원인은 live state evidence로 확정하지 않았으므로 benchmark 부재라고 추측하지 않는다.
   - 결함은 원인 하나보다 구조적으로 더 크다. enable predicate와 click-time guard가 분리돼 있고 disabled reason을 USER에게 보여주는 typed readiness가 없어, 어떤 prerequisite가 막았는지 화면만으로 알 수 없다.

#### VBHAI-P0-07 Design Review — P0 0 / P1 4 / P2 3

v0.1.10 correction design을 current Source와 CF-FQ-046 ownership contract에 재대조한 결과 방향 자체는 타당하지만 구현 전 다음 교정이 필요하다.

**P1-1 — Generic Profile Commit Preview에 Step 5 compatibility policy를 직접 소유시키지 않는다.**

`FCFBuilderProfileCommitPreview`는 `PreviewBuilderProfiles()`와 `CommitBuilderProfiles()`가 공유하는 generic AuthoringWrite preview/approval contract다. current `Operation.Status=NoChange`는 private 4 Profile payload와 persistent receipt 전체가 exact 같다는 transaction 의미를 소유한다. Hardpoint/Mount-only 예외를 이 generic preview의 NoChange 의미로 섞으면 commit/no-change transaction semantics가 달라질 수 있다.

교정 계약:
- `FCFBuilderProfileCommitPreview.Operation.Status`와 `ReceiptMatchesFreshProposal()`의 exact 의미는 유지한다.
- `PhysicsEquivalentStructuralDrift` 때문에 `Operation.Status`를 NoChange로 바꾸지 않는다.
- Step 5 compatibility는 별도 read-only evaluator/result가 소유한다. 예: `EvaluatePhysicsReceiptCompatibility(...)` + `FCFBuilderPhysicsReceiptCompatibilityResult`.
- full receipt provenance 비교는 VM에서 부분 재구현하지 않는다. `CFVehicleBuilderCommit.cpp`의 existing exact receipt comparison을 분해/재사용해 **resolved DefinitionHash를 제외한 모든 Physics provenance**를 한 authority에서 판정한다.
- current `PrepareCurrentPhysicsReceiptRefresh()`의 local provenance subset은 compatibility authority로 승격하지 않는다. 현재 subset에는 EvidencePath/EvidenceId/ConsumedClaimIds(+hash)와 Transmission/Engine review self-consistency 재계산이 빠져 있으므로 복사하면 안 된다.

**P1-2 — Equivalent structural drift의 baseline identity를 existing refresh boundary와 exact 동일하게 고정한다.**

field allowlist만 보고 `HardpointSlots/MountProfiles`면 통과시키면 안 된다. current `ValidatePhysicsReceiptRefreshBoundary()`가 이미 다음 안전 경계를 소유한다.
- receipt의 accepted `ProspectiveResolvedDefinitionHash`가 current Target DefinitionHash와 exact 같음.
- receipt hash와 current prospective hash는 다름.
- current Target 대비 pending FieldDiff가 비어 있지 않음.
- 모든 pending diff collection이 `HardpointSlots` 또는 `MountProfiles`임.

교정 계약:
- `PhysicsEquivalentStructuralDrift`는 위 existing boundary를 그대로 재사용하거나 의미가 exact 동일한 pure helper를 단일 owner로 사용해야 한다.
- receipt baseline != current Target, empty/unknown diff, unrelated collection이 하나라도 있으면 fail-closed `PhysicsStale` 또는 `Blocked`다.
- 같은 allowlist 로직을 compatibility와 receipt-refresh 두 군데에 복제하지 않는다.

**P1-3 — Step 8 stable VM preflight와 Tab-owned benchmark process 상태를 분리한다.**

current benchmark child process lifetime은 `SCFVehicleBuilderTab::bDrivingBenchmarkRunning` + `DrivingBenchmarkProcess`가 소유한다. 따라서 VM의 단일 `ReadDrivingApplyState()`에 `BenchmarkRunning`까지 넣으면 Tab orchestration state를 VM으로 역주입하거나 동일 상태를 이중 보관해야 한다.

교정 계약:
- VM은 **stable data preflight**만 소유한다. current benchmark exact identity, Recipe/Target 존재, dirty/save-on-disk, AppliedState/current Definition identity 등을 판정한다.
- Tab은 `bDrivingBenchmarkRunning`, current Step 같은 UI orchestration state를 VM stable preflight 위에 합성해 최종 버튼 enable/presentation을 만든다.
- `ApplySelectedVehicleToActivePIE()`는 VM stable preflight를 재사용한 뒤 PIE World/PlayerController/Pawn 같은 volatile runtime guard를 수행한다.
- benchmark process state를 이 목적만으로 VM으로 이동하지 않는다.

권장 분리는 예를 들면 다음과 같다.

```text
FCFVehicleDrivingApplyPreflight      // VM stable truth
ECFVehicleDrivingApplyBlocker        // BenchmarkUnavailable/Stale, Recipe/Target state 등

SCFVehicleBuilderTab::CanApply...    // !bDrivingBenchmarkRunning + Step8 + Preflight.CanApply()
Presentation                          // Tab state + typed blocker를 USER 한국어로 표시
```

**P1-4 — `saved Target`을 dirty=false 하나로 축약하지 않고 disk existence + fresh current identity까지 exact 고정한다.**

`PrepareDrivingBenchmarkLaunch()`는 이미 Recipe/Target `IsDirty()==false`뿐 아니라 `FPackageName::DoesPackageExist()`와 `AppliedState.AppliedDefinitionHash == current TargetDefinitionHash`를 확인한다. 반면 current `ApplySelectedVehicleToActivePIE()`는 dirty와 benchmark hash를 다시 보지만 disk existence는 직접 확인하지 않는다.

교정 계약:
- stable Driving Apply preflight는 Recipe/Target load 성공, dirty=false, 두 package의 disk 존재를 모두 요구한다.
- `bHasDrivingBenchmarkResult` cached bool만 신뢰하지 않고 `BuildDrivingTargetIdentity()`를 fresh 호출해 current Target path/hash와 benchmark path/hash를 exact 비교한다.
- Recipe `AppliedState.AppliedDefinitionHash`도 current Target hash와 exact인지 확인해 Final Review 적용 상태가 유지되는지 증명한다.
- 위 read 자체가 실패하면 generic `Blocked`만 주지 말고 Presentation이 복구 행동을 정할 수 있는 typed `StateReadFailed` 또는 동등 reason을 둔다.

**P2-1 — `bCanApply`와 enum을 두 개의 writable truth로 만들지 않는다.**
- `CanApply()`는 `Blocker == None/Ready`에서 계산하는 derived accessor로 둔다.
- struct에 bool을 보관해야 한다면 factory/evaluator만 값을 만들고 caller가 임의 조합하지 못하게 한다.

**P2-2 — 046×047 fresh overlap preflight 파일 범위를 명시한다.**
- 최소 `CFVehicleBuilderVM.cpp/.h`, `CFVehicleBuilderTab.cpp/.h`, `CFVehicleBuilderPresentation.cpp/.h`를 fresh diff 대상으로 본다.
- 047A 뒤 046이 VM read-only seam을 추가할 수 있으므로 Tab 2파일만 shared owner로 간주하지 않는다.

**P2-3 — Current projection의 stale CF-FQ-045 상태를 교정한다.**
- 대표 Plan `Protected`와 완료조건의 `CF-FQ-045 Ready` 표현은 current ProjectSSOT상 CF-FQ-045 Done과 다르다.
- correction 때 `CF-FQ-045 Done / historical/current DataManagement owner 보호`로 문서만 동기화한다. 기능 lifecycle을 다시 열지 않는다.

설계 재검수 합격 기준:
- generic commit preview/NoChange semantics 보존.
- full receipt provenance exact comparator 단일 owner.
- existing receipt→current Target→prospective structural boundary exact 재사용.
- VM stable preflight와 Tab process/UI state 분리.
- Driving Apply saved-state = dirty false + package exists + current identity exact.
- derived CanApply single truth.
- 046×047 shared-file set 명시.
- P0/P1 0.

#### VBHAI-P0-07A — Physics Impact Boundary

목표는 persistent receipt의 provenance를 약화하지 않으면서 **Physics 의미상 유효성**을 full Definition identity와 분리하는 것이다.

권장 typed contract:

```text
ECFBuilderPhysicsReceiptCompatibility
- Exact
- PhysicsEquivalentStructuralDrift
- PhysicsStale
- Blocked

FCFBuilderPhysicsReceiptCompatibilityResult
- Compatibility
- Diagnostic
```

이 compatibility는 `FCFBuilderProfileCommitPreview`의 필드가 아니다. `PreviewBuilderProfiles()` / `CommitBuilderProfiles()`가 공유하는 generic AuthoringWrite transaction contract와 Step 5 eligibility를 분리한다.

기존 transaction authority는 그대로 보존한다.
- `FCFBuilderProfileCommitPreview.Operation.Status`의 `NoChange` 의미를 변경하지 않는다.
- `ReceiptMatchesFreshProposal()`은 Profile payload와 **full persistent receipt 전체가 exact**인지 판정하는 기존 comparator로 유지한다.
- commit assignment readback / post-edit final readback도 계속 full exact comparator를 사용한다.
- `PhysicsEquivalentStructuralDrift`를 이유로 generic Preview의 `Operation.Status`를 `NoChange`로 만들지 않는다.

Step 5용 Physics compatibility는 별도 mutation0 evaluator가 소유한다. 권장 위치는 기존 Builder Profile commit/service lane의 read-only seam이며 이름은 `EvaluatePhysicsReceiptCompatibility(...)` 또는 동등한 직관적 명칭으로 한다.

Physics provenance exact 판정은 VM이 receipt 일부 필드를 재구성하지 않는다. service 내부의 existing full comparator 의미를 다음 두 층으로 분해해 재사용한다.

```text
ReceiptMatchesFreshPhysicsProvenance(...)
- Receipt basic validity
- Evidence path / EvidenceId / fingerprint
- canonical ConsumedClaimIds + hash
- VehicleBase / Drivetrain / Handling / Performance fingerprint
- Transmission policy / proposal hash / receipt review self-consistency
- Engine Curve proposal hash / receipt review self-consistency
- Resolver contract revision
- ProspectiveResolvedDefinitionHash는 제외

ReceiptMatchesFreshProposal(...)
- ReceiptMatchesFreshPhysicsProvenance(...)
- ProspectiveResolvedDefinitionHash exact
```

즉 기존 `ReceiptMatchesFreshProposal()`의 결과 의미는 변하지 않고 내부 중복만 분리한다. 현재 `PrepareCurrentPhysicsReceiptRefresh()`에 존재하는 local provenance subset은 compatibility authority로 승격하지 않으며, implementation에서 위 service-owned full Physics provenance comparator를 호출하도록 교정한다.

Physics provenance가 exact일 때만 full resolved Definition binding을 다음 순서로 분류한다.
- receipt full resolved hash == current prospective resolved hash → `Exact`.
- 그 외에는 existing structural boundary authority를 사용해 `PhysicsEquivalentStructuralDrift` 가능 여부를 평가한다.
- Physics provenance가 하나라도 변경 → `PhysicsStale`.
- current identity/diff를 안전하게 읽거나 분류할 수 없음 → `Blocked` fail-closed.

structural boundary는 `ValidatePhysicsReceiptRefreshBoundary()`와 별도 allowlist를 중복 구현하지 않는다. 기존 bool API를 보존하면서 내부에 typed pure evaluator를 단일 owner로 둘 수 있다.

```text
EvaluatePhysicsStructuralDriftBoundary(...)
- Equivalent
- NotEquivalent
- Blocked

ValidatePhysicsReceiptRefreshBoundary(...)
- 기존 함수 시그니처 보존
- 위 typed evaluator 결과를 bool + OutError로 projection
```

`Equivalent`의 exact 조건은 기존 함수 의미와 동일하다.
- receipt `ProspectiveResolvedDefinitionHash`가 **current Target DefinitionHash와 exact 같음**.
- receipt hash와 current prospective resolved hash는 다름.
- current Target 대비 pending FieldDiff가 비어 있지 않음.
- 모든 pending diff collection이 `HardpointSlots` 또는 `MountProfiles`임.

receipt baseline != current Target, empty/unknown diff, unrelated collection 포함은 equivalent로 인정하지 않는다. 판정 가능한 non-equivalent는 `PhysicsStale`, 읽기/identity 자체를 확정할 수 없으면 `Blocked`다.

현재 CF-FQ-047 범위의 Physics 비영향 structural diff는 `HardpointSlots` / `MountProfiles`다. 단, 이를 영구적으로 `Mount는 절대 Physics와 무관`이라고 하드코딩하지 않는다. 향후 default equipment/fitting이 실제 질량·CoM·physics payload를 변경하는 semantic으로 승격되면 해당 dependency는 PhysicsImpact로 재분류해야 한다.

Step contract:
- `Exact` → Step 5 Complete.
- `PhysicsEquivalentStructuralDrift` → **Step 5 Complete 유지**. USER를 Step 5로 되돌리지 않는다. 진단에는 `장착 구조만 변경되어 기존 물리 설정은 계속 유효`를 남길 수 있다.
- `PhysicsStale` → Step 5 Stale, fresh AI Physics review 필요.
- `Blocked` → Step 5 Blocked/diagnostic.
- Step 6은 별도 exception을 만들지 않고 Step 5의 typed Complete projection만 소비한다.

보존:
- `BuilderCommitReceipt.ProspectiveResolvedDefinitionHash` 필드는 삭제하지 않는다. 당시 accepted full resolved state provenance로 보존한다.
- receipt를 Hardpoint/Mount 변경 때 자동 재작성/자동 저장하지 않는다.
- 기존 `현재 물리 설정 재검증` receipt-only write는 legacy/migration/debug compatibility로 유지할 수 있으나 `PhysicsEquivalentStructuralDrift`의 정상 USER 진행 Gate로 요구하지 않는다.
- Final Review는 current fresh Resolve/Diff와 Evidence/Profile/Transmission/Engine/Resolver provenance를 독립 검증하므로 그대로 유지한다.
- Step 8 Technical Benchmark / USER Driving receipt의 **exact current DefinitionHash** 계약은 이번 교정에서 완화하지 않는다. Physics Step eligibility와 Driving acceptance freshness는 다른 문제다.

필수 regression:
- generic Profile Preview exact semantics: full receipt resolved hash까지 exact일 때만 기존 NoChange.
- exact unchanged receipt → Exact / Step5 Complete.
- HardpointSlots-only diff → PhysicsEquivalentStructuralDrift / Step5 Complete / Step6 진행 가능.
- MountProfiles-only diff → 동일.
- Hardpoint+Mount structural diff → 동일.
- receipt baseline != current Target → equivalent 금지.
- empty pending diff / unrelated or unknown Definition diff → fail-closed PhysicsStale 또는 Blocked.
- EvidencePath/EvidenceId/ConsumedClaimIds/TransmissionReview/EngineCurveReview/Profile/Resolver 중 하나라도 drift → PhysicsStale.
- `ValidatePhysicsReceiptRefreshBoundary()`와 Step 5 compatibility가 동일 typed structural boundary owner를 소비함.
- equivalent structural drift 판정은 Recipe/Profile/VehicleData/receipt mutation 및 Save 0.

#### VBHAI-P0-07B — Step 8 Driving Apply Readiness — Technical PASS

버튼 enable과 실행 guard가 stable data prerequisite를 서로 다르게 재구현하지 않도록 **VM stable preflight 하나**를 둔다. 단, Tab이 이미 소유하는 benchmark process/UI orchestration state는 VM으로 이동하거나 이중 보관하지 않는다.

VM 권장 typed contract:

```text
ECFVehicleDrivingApplyBlocker
- None
- BenchmarkUnavailable
- BenchmarkStale
- RecipeUnavailable
- TargetUnavailable
- RecipeUnsaved
- TargetUnsaved
- RecipeNotPersisted
- TargetNotPersisted
- AppliedStateStale
- StateReadFailed

FCFVehicleDrivingApplyPreflight
- Blocker
- Diagnostic
- CanApply() = Blocker == None   // derived single truth
```

`bCanApply`와 enum을 독립 writable truth로 두지 않는다. `CanApply()`는 blocker에서 계산되는 derived accessor로 고정한다.

VM 권장 API는 `ReadDrivingApplyPreflight()` 또는 동등한 read-only 함수다. 이 함수는 **stable data truth만** 판정한다.
- current validated benchmark result가 존재하는가.
- Recipe / Target을 load할 수 있는가.
- Recipe / Target package가 dirty가 아닌가.
- Recipe / Target package가 실제 disk에 존재하는가 (`FPackageName::DoesPackageExist`).
- live current Target UObject에서 fresh immutable Definition Snapshot을 만들어 current Target path/hash를 다시 얻을 수 있는가. Authoring ViewModel의 cached DefinitionHash를 stable Apply authority로 재사용하지 않는다.
- benchmark `Metric.VehicleDataPath`와 current Target path가 exact 같은가.
- benchmark `ExpectedTargetDefinitionHash`와 current Target DefinitionHash가 exact 같은가.
- Recipe `AppliedState.AppliedDefinitionHash`가 current Target DefinitionHash와 exact 같은가.

`bHasDrivingBenchmarkResult` bool 하나만 validity authority로 사용하지 않는다. 다만 Slate `IsEnabled` 평가마다 benchmark JSON 파일을 다시 읽는 것도 금지한다.
- existing `RefreshDrivingBenchmarkState()`가 selection refresh / benchmark process completion 등 기존 명시적 refresh boundary에서 JSON을 읽고 typed benchmark result를 current로 검증한다.
- 필요하면 VM에 transient typed benchmark validation state를 유지해 `Unavailable / Current / StaleOrInvalid`을 구분한다.
- `ReadDrivingApplyPreflight()`는 그 **이미 검증된 current benchmark result object**와 fresh Target/package/AppliedState를 비교한다.
- filesystem JSON parse를 매 Slate tick마다 수행하지 않는다.

Tab은 current source의 orchestration owner를 유지한다.
- `bDrivingBenchmarkRunning` / `DrivingBenchmarkProcess`는 `SCFVehicleBuilderTab`에 남긴다.
- current Step이 DrivingTest인지도 Tab의 UI enable 조건에서 합성한다.
- `CanApplyDrivingTargetToPIE()`는 `!bDrivingBenchmarkRunning && Step8 && ViewModel->ReadDrivingApplyPreflight().CanApply()`를 사용한다.
- `BenchmarkRunning` / `WrongStep`은 VM blocker가 아니라 Tab-level presentation reason이다.

`ApplySelectedVehicleToActivePIE()`는 동일 VM stable preflight를 첫 guard로 재사용한 뒤 실행 시점의 volatile runtime만 검사한다.
- PIE World
- local PlayerController
- current Pawn type
- transient VehicleData duplicate/runtime initialize

PIE가 아직 없다는 이유만으로 stable prerequisite가 Ready인 적용 버튼을 영구 비활성화하지 않는다. 클릭 시 `Play를 시작하세요`처럼 exact recovery를 제공한다.

USER-facing 한국어 이유 문구는 CF-FQ-046 Presentation owner 규칙을 따른다.
- Tab process/step reason + VM typed blocker를 하나의 사용자용 설명으로 합성한다.
- raw hash/path/error는 `진단 정보`로만 보낸다.
- 버튼 바로 근처에 disabled reason / 다음 행동을 표시하며 Tooltip만으로 숨기지 않는다.

필수 regression:
- benchmark missing / stale-invalid / Recipe dirty / Target dirty / Recipe package missing / Target package missing / AppliedState stale / state read failure별 typed blocker exact.
- validated benchmark current + Recipe/Target persisted/current identity exact → VM preflight `CanApply()==true`.
- Tab benchmark running이면 VM Ready여도 button disabled이며 reason은 `측정 완료 대기` 계열 USER 문구.
- wrong Step이면 VM Ready와 무관하게 Tab UI가 적용 action을 노출/enable하지 않음.
- UI enable과 `ApplySelectedVehicleToActivePIE()`의 **stable preflight** 결과 일치.
- Play 없음 / Pawn type mismatch는 click-time failure로 정확한 USER recovery text 제공.
- current benchmark/accepted summary가 존재하면서 disabled일 경우 화면에 exact blocker 이유가 반드시 보임.
- `IsEnabled` 반복 평가가 benchmark JSON disk reread를 발생시키지 않음.

#### VBHAI-P0-07C — 046×047 Integration / Technical Validation — PASS

046×047 overlap은 Tab 2파일만으로 축약하지 않는다. Step1~8 common page shell, Presentation translation과 minimal VM seam이 병행될 수 있으므로 구현 전/통합 직전에 다음 **6개 shared file**의 fresh diff를 exact 재확인한다.

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderVM.cpp
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderVM.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderTab.cpp
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTab.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderPresent.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderPresent.h
```

실제 current file name은 `CFVehicleBuilderPresent.*`이며 새 `Presentation.*` 계열을 만들지 않는다.

권장 순서:
1. 047A의 service/VM Physics compatibility + typed structural boundary + pure tests를 먼저 구현할 수 있다. 이 단계는 공통 Page shell을 수정하지 않는다.
2. CF-FQ-046이 Step1~8 공통 Page shell/Scroll correction을 완료한다.
3. 위 6개 shared-file fresh diff를 다시 읽고 046 변경을 authority로 보존한다.
4. 047B의 VM stable Driving preflight와 Step8 reason presentation을 최신 VM/Tab/Present 위에 최소 통합한다. 046 common scroll/page shell을 047에서 재작성하지 않는다.
5. official UE 5.8 Editor build.
6. CF-FQ-047 focused + Step5/6/8 affected + CF-FQ-046 affected regression.
7. source re-review P0/P1 0.

이번 correction이 Wagon Recipe/VehicleData semantic DefinitionHash를 변경하지 않는다면 아래 evidence를 반복하지 않는다.
- P0-06 fixed-60Hz benchmark.
- P0-06 USER direct Driving PASS.
- Wagon persisted Hardpoint/Mount 1/1 recovery.
- RuntimeApply Mount_Top_01 technical PASS.

#### VBHAI-P0-07D — USER Re-Acceptance

USER는 수정 후 다음만 다시 본다.
- 장착 위치/장착 규칙만 변경해도 Step 5로 불필요하게 되돌아가지 않는가.
- 실제 Physics dependency 변경 때만 Step 5 재검토가 요구되는가.
- Step 8 적용 버튼이 비활성일 때 화면에 이유와 다음 행동이 보이는가.
- stable prerequisite가 충족되면 PIE 적용 버튼을 정상 사용할 수 있는가.
- CF-FQ-046 공통 Page shell 적용 뒤 Step 1~8의 긴 상세/진단 영역을 펼쳐도 동일한 스크롤 방식으로 끝까지 접근 가능한가.

기존 P0-07 technical acceptance 항목은 유지한다.
1. unbound HP_Top_01 의미를 이해할 수 있는가.
2. Socket만 있다고 장착 가능한 것이 아님을 알 수 있는가.
3. existing Socket adoption이 새 Socket을 만들지 않는가.
4. Top_01 ↔ HP_Top_01 ↔ Mount_Top_01 관계를 이해할 수 있는가.
5. Step 6에서 missing Mount가 완료로 오인되지 않는가.
6. Step 7/8에서 실제 Runtime Hardpoint/Mount 결과를 확인할 수 있는가.
7. Wagon RuntimeApply에서 target Mount/Profile이 실제 표시되는가.
8. DefinitionHash 변경에 대해 USER Driving PASS 재승인이 요구되는가.
9. UAT 종료 뒤 Product Asset이 intended persisted state로 복구되었는가.

### VBHAI-P0-08 — Current System Promotion

- Systems/Vehicles/VehicleBuilder.md에 Hardpoint Authoring Integrity 계약 승격
- FeatureQueue → Done
- ActiveWork / Plan Index Ready route 제거
- 대표 Plan Historical + Retained Path
- Wagon persisted recovery evidence 보존

---

## 12. 완료 조건

1. Socket과 Runtime Hardpoint 차이가 USER에게 명확하다.
2. unbound Chassis Socket inventory가 Resolver AssetSnapshot과 분리된 Editor-only R0 lane이다.
3. unbound Socket add/remove만으로 ChassisLayoutFingerprint / ResolvedDefinitionHash / current Driving acceptance가 변하지 않는다.
4. Standard adoption candidate는 exact canonical `HP_<KnownCategory>_<Index>` + `%02d` round-trip grammar와 Recipe/Target collision 검사를 통과해야 한다.
5. noncanonical HP_*는 advisory만 제공하고 Standard adoption action을 노출하지 않는다.
6. unbound Socket 존재 자체는 어떤 Plan Mode에서도 completion blocker가 아니다.
7. USER explicit mode 전환 없이 LegacyCompatible/Unspecified/NoHardpoints에서 adoption mutation을 수행하지 않는다.
8. existing Socket adoption은 exact parsed LocationSlotId를 사용하고 Socket을 중복 생성하지 않는다.
9. shared Chassis의 unrelated Socket/차량을 변경하지 않는다.
10. UseHardpoints에서 Hardpoint 1 / Mount 0은 완료될 수 없다.
11. Step 7은 current Target과 prospective Resolve semantic count를 분리해서 보여준다.
12. Step 8은 saved/current Target VehicleData readback을 장착 summary authority로 사용한다.
13. VBHAI-P0-05 official Build + focused/affected Automation PASS 전 Production Wagon recovery mutation을 하지 않는다.
14. VBHAI-D1에서 Wagon MountType/SizeLimit을 USER/Design이 명시 확정한다.
15. Wagon의 current accepted Hardpoint/Mount 배열이 Recipe Intent와 VehicleData에서 개수·stable ID·Socket/Location relation 기준으로 exact 대응하며, canonical `Top_01 → HP_Top_01` / `Mount_Top_01 → Top_01`이 중복 없이 유지된다.
16. Wagon RuntimeApply에 target Mount/Profile이 표시된다.
17. DefinitionHash 변경 시 기존 USER Driving PASS가 stale 처리된다.
18. Product destructive UAT snapshot은 Recipe/VehicleData/StaticMesh/DefinitionHash/Driving acceptance를 포함한다.
19. Product restoration baseline은 고장난 0/0이나 영구 고정 1/1이 아니라 **각 destructive UAT 시작 직전 USER가 승인한 current persisted semantic baseline**이다. 현재 Wagon baseline은 2/2다.
20. destructive UAT 뒤에는 해당 current accepted baseline으로 exact restoration + explicit Save + persisted readback을 수행한다. USER가 UAT 중 생성한 valid semantic을 Product 변경으로 명시 승격한 경우에는 새 baseline으로 전진할 수 있다.
21. CF-FQ-043 Legacy/custom/multi-Mount/no-cascade/shared-Chassis 계약을 보존한다.
22. CF-FQ-046 Presentation owner와 중복 아키텍처를 만들지 않는다.
23. CF-FQ-041/039 병렬 작업과 CF-FQ-045 Done의 current DataManagement owner/완료 evidence를 침범하거나 재오픈하지 않는다.
24. 기술 주행 측정 실행 중에는 actual `Builder → PowerShell → UnrealEditor` chain 끝까지 전달된 exact RunId에 binding된 coarse 단계 진행률과 경과 시간을 표시하며, 가변 차량 성능을 무시한 가짜 정밀 ETA를 표시하지 않는다.
25. progress sidecar write 실패는 같은 phase group을 매 frame 재시도하지 않고 run당 최대 7회 phase-group write attempt로 bounded하며 benchmark 본체 PASS/FAIL을 바꾸지 않는다.
26. PowerShell process terminal 관측 순간 running progress cache/timer를 종료하고 final result JSON + exit code를 terminal authority로 사용한다. `7/7 결과 정리` 자체를 PASS로 해석하지 않는다.
27. Runtime `CarFight_Re` benchmark writer는 `CarFight_ReEditor` DTO/header를 참조하지 않고 runtime-private schema writer를 사용한다.
28. `주행 테스트 통과` 뒤 current persistent acceptance receipt가 exact current Target에 일치하고 Recipe가 dirty이면 같은 Step 8에서 USER가 exact current Recipe package 하나를 명시 저장할 수 있다. 이 저장은 receipt 한 필드가 아니라 해당 Recipe package의 현재 미저장 변경 전체를 저장한다.
29. Recipe Save 즉시 성공은 `UPackage::SavePackage()==true`와 return 뒤 package clean을 별도 확인하며, package 존재 여부를 새 receipt durable-write 증거로 사용하거나 `SetDirtyFlag(false)`로 성공을 가장하지 않는다.
30. Save All/Target/StaticMesh/Catalog 자동 저장을 수행하지 않으며, fresh persisted receipt readback은 P0-07G durable closure evidence로 별도 확인한다.
31. Step 7 `완료`는 Final Review semantic Diff 0만으로 성립하지 않는다. exact current Target VehicleData + current Recipe가 disk에 존재하고 clean이며 Recipe AppliedState가 fresh current Target DefinitionHash와 exact 일치해야 한다.
32. Step 7 USER primary commit action은 semantic Apply가 필요하면 existing FinalReview Apply를 수행한 뒤 exact Target → Recipe 순서로 저장하고, semantic Diff가 이미 0이지만 pair가 dirty이면 같은 Step 7 action에서 persist-only recovery를 수행한다. Step 8에 별도 pre-benchmark Save 버튼을 만들지 않는다.
33. Step 7 durable commit은 current Target VehicleData와 current Recipe **두 package만** 저장한다. StaticMesh, private Profile, Evidence, Runtime Catalog, 다른 Recipe/VehicleData, Save All은 이번 correction의 저장 권한에 포함하지 않는다.
34. Step 7 Apply/Save/Undo 중 일부만 성공한 경우 `완료`로 표시하지 않으며 partial persistence를 자동 rollback/재시도하지 않는다. 같은 Step 7에서 exact recovery 또는 guarded Undo를 수행한다.
35. USER Acceptance PASS.
36. Current System Promotion 완료.

---

## 13. Current Checkpoint

Feature: CF-FQ-047
Status: Done / P0 Complete / VBHAI-P0-07G USER Re-Acceptance PASS / P0-08 Current System Promotion Complete / Historical + Retained Path
Priority: P1
Current Gate: None — current implementation authority는 `Document/Systems/Vehicles/VehicleBuilder.md v1.4.0`이며 이 Plan은 Historical evidence를 기존 경로에서 보존한다.

Current Product truth:
- Wagon Chassis canonical `HP_Top_01`은 preserved 상태다.
- P0-06 Historical baseline은 Recipe/VehicleData 1/1, Target DefinitionHash `8d780eb07ee7ab4672233fd3d15a0db4`, Benchmark RunId `a9e28515-05ea-4cf9-bc0a-80a49adfc018`, USER Driving PASS였다.
- P0-07 USER UAT에서 USER가 두 번째 Hardpoint/Mount를 Builder로 의도적으로 추가했고, 정상 생성·사용 가능하면 유지하겠다고 명시 승인했다.
- 현재 persisted Wagon은 Recipe `HardpointIntents=2 / MountIntents=2`, VehicleData `HardpointSlots=2 / MountProfiles=2`다.
- 확장형 `ActualWagon.PostLoadMountIntegrity`는 Recipe↔VehicleData count alignment, 각 Intent의 exact 1회 Target semantic 대응, canonical Top_01/Mount_Top_01 중복 0, legacy RoofTurret injection 0, Recipe AppliedState DefinitionHash == current Target DefinitionHash를 모두 PASS했다.
- 따라서 current 2/2는 unknown drift/residue가 아니라 USER가 유지 승인한 current semantic Product baseline이다. `RunWagonMountRecovery.ps1` exact1 복구를 실행하지 않는다.
- current 2/2는 P0-06 1/1 이후 semantic Definition 변경이므로 historical Benchmark/USER Driving acceptance를 current fresh라고 확대하지 않는다. P0-07B typed stable preflight는 current live Target fresh DefinitionHash, persisted/dirty 상태, AppliedState와 validated benchmark identity를 하나의 blocker authority로 비교하고 UI enable과 production PIE Apply guard가 동일 결과를 사용한다.
- P0-07D USER UAT에서 USER가 current 2/2 대상으로 fresh 기술 주행 측정을 완료했고, 그 결과 `현재 PIE에 선택 차량 적용`이 활성화된 것을 확인한 뒤 직접 PIE 주행을 수행하고 `주행 테스트 통과`를 명시 승인했다. 따라서 current 2/2에 대한 USER driving 판단 자체는 다시 Pending으로 되돌리지 않는다.
- P0-07E에서 Step 8 exact current Recipe explicit Save와 benchmark coarse progress/elapsed를 구현·검증했다. Save All/Target/StaticMesh/Catalog 자동 저장 권한은 추가하지 않았다.
- P0-07H에서 Step 7 semantic PASS와 durable Target/Recipe handoff를 분리하고, USER Driving receipt-only downstream dirty를 `PostDrivingReceiptSavePending`으로 처리해 Step 7을 retroactive 미완료로 되감지 않도록 구현했다.
- fresh Editor restart에서 Step 7이 Step 8보다 먼저 평가되는 순환은 persistent USER Driving receipt-only candidate를 benchmark cache와 독립적으로 먼저 판정하고, 그 narrow candidate에서만 기존 persisted benchmark JSON을 fresh current Target path/hash에 재검증해 transient cache를 복원하도록 교정했다. 이 복원은 read-only이며 Product Asset mutation/Save/benchmark 실행을 수행하지 않는다.
- current correction은 Wagon Definition semantic을 변경하지 않았으므로 accepted Wagon 2/2 baseline과 기존 USER direct Driving judgement를 보존했다.
- P0-07G USER Re-Acceptance에서 USER가 최종 흐름을 **PASS**했다. 이어 fresh AssetDump persisted readback에서 `BuilderDrivingAcceptanceReceipt.TargetVehicleDataPath=/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon`, `TargetDefinitionHash=8aad29d04e008fcd2acb2e6470cd87f1`, `AcceptedBenchmarkRunId=e5c8153e-4b31-7c8b-60c1-149f13550561`을 확인했다.
- 같은 persisted Recipe의 `AppliedState.AppliedDefinitionHash`도 `8aad29d04e008fcd2acb2e6470cd87f1`로 exact 일치했고 `HardpointIntents=2 / MountIntents=2`, `UseHardpoints`를 확인했다. 따라서 USER PASS와 durable receipt closure를 모두 충족했다.
- P0-08에서 Current 계약을 `Document/Systems/Vehicles/VehicleBuilder.md v1.4.0`으로 승격하고 FeatureQueue/ActiveWork/Plan Index의 Ready route를 제거한다. benchmark/주행감 자체는 추가 재실행하지 않는다.

Implementation mutation: CF-FQ-047 integrity + RuntimeApply explicit active-Mount resolution/fail-closed correction present
Product semantic baseline: USER-authored Recipe/VehicleData 2/2 persisted and semantic-alignment PASS
Editor restart for ownership: 0

Protected:
- CF-FQ-039 Active
- CF-FQ-041 Ready RuntimeApply
- CF-FQ-043 Done/Historical
- CF-FQ-045 Done / current DataManagement owner와 closure evidence 보호
- CF-FQ-046 Ready / shared Builder Source dirty

---



## 14. VBHAI-P0-07D UAT Outcome + P0-07E~G Correction Design

### 14.1 P0-07D USER Re-Acceptance Outcome

P0-07D는 USER가 current 2/2 Wagon으로 실제 fresh Benchmark → PIE Apply → direct Driving → `주행 테스트 통과`까지 수행했으므로 driving feel 판단 자체는 PASS다. 그러나 acceptance 직후 Recipe가 dirty가 되면서 Step 8 action이 막혔고, 같은 화면에 필요한 명시적 Save action이 없었다. 또한 기술 주행 측정 child process 실행 중에는 terminal 여부 외의 진행 정보가 없어 USER가 정상 진행/정지 여부를 판단하기 어렵다.

따라서 P0-07D 전체 closure는 다음처럼 분리한다.

- current 2/2 fresh technical benchmark: USER workflow에서 완료 확인.
- current 2/2 PIE transient Apply: 활성화 및 적용 확인.
- current 2/2 direct driving judgement: USER PASS.
- current Recipe USER Driving receipt: in-memory write success path + dirty 전환 확인, durable Save는 Pending.
- Step 8 progress visibility: FAIL / correction required.
- Step 8 explicit save recovery: FAIL / correction required.
- CF-FQ-046 common Page Shell/Scroll UAT issue: 별도 046 owner 유지, 047에서 개별 nested ScrollBox 땜질 금지.

### 14.2 VBHAI-P0-07E — Step 8 Progress + Explicit Recipe Save Implementation

#### A. 기술 주행 측정 진행 상태 계약

현재 `SCFVehicleBuilderTab::Tick()`은 benchmark child process가 살아 있는 동안 즉시 return하고 terminal 뒤에만 결과를 읽는다. `RunBuilderBench.ps1`도 final result JSON을 process 종료 뒤에만 쓴다. 따라서 P0-07E는 terminal result authority를 바꾸지 않고 **관측 전용 progress sidecar**를 추가한다.

progress sidecar:

```text
UE/Saved/CarFight/VehicleBuilderBenchmarkProgress.json
schema_version = carfight_vehicle_builder_benchmark_progress_v1
run_id
phase_code
phase_group
phase_index
phase_count
updated_utc
```

규칙:

1. actual process owner는 기존 그대로 `Builder Tab → powershell.exe RunBuilderBench.ps1 → UnrealEditor.exe`다. Tab이 UnrealEditor를 직접 실행하거나 process ownership을 이중화하지 않는다.
2. VM의 `PrepareDrivingBenchmarkLaunch()`는 기존처럼 fresh RunId를 만들고 PowerShell wrapper에 `-RunId <RunId>`를 전달한다.
3. `RunBuilderBench.ps1`이 canonical progress path `UE/Saved/CarFight/VehicleBuilderBenchmarkProgress.json`을 소유하고 launch 직전 stale sidecar를 제거한다.
4. PowerShell wrapper는 자신이 받은 exact RunId와 canonical progress absolute path를 UnrealEditor command line에 `-CFBuilderBenchmarkRunId=<RunId>` / `-CFBuilderBenchmarkProgressPath=<absolute path>`로 명시 전달한다.
5. runtime `CFVehicleBenchTests.cpp` writer는 위 두 command-line 값을 직접 parse한다. progress path는 Project `Saved/CarFight` 아래 canonical path인지 확인하고, RunId/path가 없거나 유효하지 않으면 progress 기록만 비활성화하며 benchmark 본체는 계속 실행한다.
6. benchmark runtime은 current internal phase를 USER-facing 7 group으로 투영한다.
   - 1/7 준비: `WaitForPIE`, `SettleBeforeAcceleration`
   - 2/7 가속: `MeasureAcceleration`
   - 3/7 최고속도: `MeasureTopSpeed`
   - 4/7 제동: `PrepareBraking`, `ReachBrakingSpeed`, `MeasureBraking`
   - 5/7 조향: `PrepareYaw`, `SettleBeforeYaw`, `ReachYawSpeed`, `MeasureYaw`
   - 6/7 회전반경: `PrepareTurning`, `SettleBeforeTurning`, `ReachTurningSpeed`, `MeasureTurning`
   - 7/7 결과 정리: `Finish`
7. runtime writer는 projected group이 바뀔 때 `LastAttemptedPhaseGroup`을 **filesystem write 전에** 갱신하고 group당 최대 한 번만 write를 시도한다. write 실패 뒤 같은 group을 매 frame 재시도하지 않으며 한 run의 progress write attempt는 최대 7회다. 다음 distinct group에서는 다시 한 번 시도할 수 있다.
8. progress 파일은 temporary write → replace/move와 동등한 atomic replacement를 사용해 reader가 partial JSON을 정상 상태로 오인하지 않게 한다. progress write 실패는 benchmark 본체의 PASS/FAIL을 바꾸지 않는다.
9. Editor typed progress DTO/parser는 `CarFight_ReEditor`에만 존재한다. Runtime writer는 Editor header/DTO를 include하지 않고 runtime-private schema literal/field writer만 사용한다. `CarFight_Re → CarFight_ReEditor` 역방향 dependency를 만들지 않는다.
10. Tab은 PowerShell child process가 살아 있는 동안 약 0.5초 bounded interval로 sidecar를 polling하고 `snapshot.run_id == RunningDrivingBenchmarkRunId`인 snapshot만 current progress로 소비한다. Slate attribute 평가마다 disk JSON을 읽지 않는다.
11. sidecar가 아직 없거나 일시 parse 실패해도 benchmark를 실패 처리하지 않고 `측정 진행 중 · 상세 단계 확인 대기` + monotonic 경과시간 fallback을 표시한다.
12. `DrivingBenchmarkStartedAtSeconds` 같은 Tab-local monotonic start time으로 경과시간을 계산한다. 차량별 phase reach time이 달라지므로 `남은 37초` 같은 가짜 정밀 ETA는 만들지 않는다.
13. PowerShell process가 terminal이 된 프레임에는 progress cache/timer를 먼저 running 상태에서 해제한 뒤 기존 final result refresh + exit code authority로 전환한다. 같은 RunId의 마지막 snapshot이 `7/7 결과 정리`여도 그 자체를 성공으로 해석하지 않는다.
14. terminal 뒤 sidecar는 진단 residue로 남아도 되며 다음 benchmark launch가 stale sidecar를 정리한다.

USER 기본 표시 예:

```text
기술 주행 측정 중
[진행 막대]
현재: 제동 성능 측정
진행: 4 / 7 단계
경과 시간: 00:42
차량 성능에 따라 단계별 시간은 달라집니다. 완료되면 자동으로 갱신됩니다.
```

#### B. exact current Recipe 명시 저장 계약

`주행 테스트 통과`는 Recipe receipt를 persistent object에 쓰되 자동 저장하지 않는 기존 안전 계약을 유지한다. P0-07E는 **USER가 버튼을 직접 눌렀을 때만 exact current Recipe package 하나를 저장하는 action**을 추가한다.

필수 preflight:

1. current managed Recipe object가 존재하고 현재 Builder selection의 exact Recipe identity와 일치한다.
2. `BuilderDrivingAcceptanceReceipt`가 valid하며 receipt의 Target path/hash가 fresh current Target path/hash와 exact 일치한다. legacy host-local token만 있고 persistent receipt가 없으면 이 버튼이 current USER acceptance를 저장한다고 주장하지 않고 Blocked 처리한다.
3. Recipe package가 transient/invalid가 아니며 valid `/Game/...` long package name이다.
4. current Recipe package가 dirty일 때 `저장 필요` action을 활성화한다. clean이면 `저장 불필요/이미 저장됨` no-op 상태다.
5. package filename은 package long name에서 `.uasset` 경로를 deterministic하게 계산하고 기존 UE 5.8 `FSavePackageArgs + UPackage::SavePackage(Package, Recipe, Filename, SaveArgs)` 패턴을 재사용한다.

writer boundary:

```text
USER click
→ exact current Recipe UPackage only
→ UPackage::SavePackage(...) return 확인
→ return 뒤 exact Recipe package IsDirty()==false 확인
→ current Builder state refresh
```

즉시 save outcome은 하나의 bool로 뭉개지 않고 다음 의미를 구분한다.

- `Saved`: SavePackage true + package clean.
- `NoSaveNeeded`: exact current receipt는 유효하지만 package가 이미 clean.
- `Blocked`: Recipe/selection/receipt/Target/package preflight 불충족.
- `SaveFailed`: SavePackage false.
- `SaveStateUnconfirmed`: SavePackage true지만 return 뒤 package가 여전히 dirty.
- `SavedRefreshWarning`: SavePackage true + package clean까지 성립했으나 후속 `RefreshCurrentState()`만 실패.

`DoesPackageExist()`는 기존 Recipe 파일의 disk 존재 확인에는 사용할 수 있지만 **방금 갱신된 receipt가 새로 durable write됐다는 증거로 사용하지 않는다.** Save 성공을 가장하기 위한 `SetDirtyFlag(false)` 또는 동등한 수동 dirty clear도 금지한다. `Saved`와 fresh persisted receipt readback은 서로 다른 claim이며, latter는 P0-07G durable closure에서 별도 확인한다.

금지:

- Save All 금지
- VehicleData Target 동시 저장 금지
- StaticMesh 동시 저장 금지
- Runtime Catalog 동시 저장 금지
- 다른 차량 Recipe 저장 금지
- source-control checkout 자동화 금지
- 자동 retry 금지
- USER click 없는 자동 저장 금지

UI:

```text
주행 테스트 통과 기록됨
차량 제작 기록: 저장 필요
[차량 제작 기록 저장]
현재 차량의 제작 데이터(Recipe) 1개에 있는 미저장 변경 전체를 저장합니다.
VehicleData, StaticMesh, 데모 차량 목록 등 다른 Asset은 저장하지 않습니다.
```

버튼명은 `차량 제작 기록 저장`을 유지할 수 있지만 receipt 한 필드만 저장하는 것처럼 설명하지 않는다. 저장 성공 뒤에는 `차량 제작 기록 저장 완료`로 갱신한다. 저장 자체는 성공했는데 후속 `RefreshCurrentState()`가 실패한 경우 persistent success를 전체 failure로 뒤집지 않고 **저장 완료 + 화면 상태 다시 확인 필요** warning으로 분리한다. 이는 post-P0-06 false-negative 교정 원칙을 그대로 재사용한다.

Runtime Catalog는 별도 mutation owner다. 이번 USER screenshot에서는 `등록됨 / 목록 저장 상태 정상`이므로 P0-07E에서 Catalog save 권한을 새로 만들지 않는다. 향후 Catalog dirty가 실제 UX blocker로 확인되면 별도 targeted Catalog save를 검토하되 Recipe와 하나의 Save All action으로 묶지 않는다.

#### B-1. current dirty acceptance receipt lifecycle protection

새 Save 버튼 구현 전인 현재에는 current 2/2 USER PASS receipt가 Editor memory의 dirty `DA_Recipe_Wagon`에만 남아 있을 수 있다. 따라서 P0-07E 구현 lifecycle은 다음 Gate를 선행한다.

1. Source/문서 편집은 Editor lifecycle을 건드리지 않고 진행할 수 있다.
2. **첫 official build/restart/linked Editor binary replacement 전에** exact current Recipe acceptance receipt가 durable하게 보호됐는지 확인한다.
3. 해당 시점에 Recipe가 unsaved일 가능성이 있고 Editor DLL lock 때문에 build가 막히면 자동 종료하지 않고 중단한다.
4. USER가 현재 Editor에서 exact `DA_Recipe_Wagon` 하나를 직접 저장하거나, 별도로 승인된 exact single-asset save authority가 있을 때만 그 저장을 사용한다.
5. Save All, 자동 Editor stop/restart, discard-unsaved는 금지한다.
6. Git에서 `DA_Recipe_Wagon.uasset`가 modified라는 사실만으로 current in-memory receipt가 durable하다고 확대하지 않는다.

#### C. CF-FQ-046 ownership 경계

- Step 1~8 common page height/scroll/overflow는 `VehicleBuilderInfoUXPlan.md v0.1.6`의 P0-05B~D owner를 유지한다.
- P0-07E는 Step 8의 **행동 의미와 progress/save state**만 추가한다.
- `CFVehicleBuilderPresent.*`는 기존 Editor-private USER projection helper를 재사용하며 별도 047 presentation architecture를 만들지 않는다.
- Step 5/8 진단·상세 스크롤 문제를 047에서 nested ScrollBox 추가로 해결하지 않는다.

#### D. 구현 예정 파일

1. `UE/Source/CarFight_Re/Private/CFVehicleBenchTests.cpp` — runtime-private phase→7 group projection + progress JSON writer. `CFVehicleBuilderTypes.h` 등 ReEditor header는 include하지 않는다.
2. `Tools/RunBuilderBench.ps1` — canonical progress sidecar path 소유, stale progress 정리, received RunId + absolute progress path를 UnrealEditor command line까지 전달.
3. `UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTypes.h` — **Editor reader/presentation용** typed benchmark progress DTO/phase group + Recipe save outcome. Runtime module과 공유하지 않는다.
4. `UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderVM.h`
5. `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderVM.cpp` — progress sidecar read/parser/cache seam + exact current receipt/Recipe save preflight/result. disk poll cadence 자체는 Tab orchestration이 소유한다.
6. `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderPresent.h`
7. `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderPresent.cpp` — USER progress/save wording projection. whole-Recipe write scope를 숨기지 않는다.
8. `UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTab.h`
9. `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderTab.cpp` — PowerShell process lifetime 유지, 약 0.5초 progress polling/cache, monotonic elapsed/progress bar, process terminal 우선 전환, exact Recipe save button/orchestration.
10. `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderHardpointIntegrityTests.cpp` 및 필요 시 `CFVBPresentTests.cpp` — run identity/schema/stale/partial/fallback, bounded writer contract, save preflight/outcome/presentation regression.
11. `Tools/RunHardpointIntegrityTests.ps1` — 신규 focused test가 추가될 때만 목록 확장.

Blueprint/DataAsset schema 추가는 필요 없다. 이번 correction은 Editor C++ + benchmark runtime test code + existing PowerShell runner 범위로 유지한다.

### 14.3 Initial Design Review — Superseded by 14.3A Mid-review

Current Source, runner, benchmark state machine, CF-FQ-046 common Page Shell owner와 재대조한 correction design review 결과:

- P0: 0
- P1: 0
- P2: 0

PASS 조건은 다음 10개를 포함한다.

1. exact RunId stale progress rejection.
2. atomic progress replacement 또는 partial-read tolerant 동등 경계.
3. progress write failure가 benchmark 본체를 실패시키지 않음.
4. bounded 0.5초 polling, Slate attribute별 disk read 금지.
5. 가짜 exact ETA 금지.
6. exact current Recipe 하나만 explicit Save.
7. Save success + refresh failure를 persistent failure로 오보고하지 않음.
8. 현재 dirty USER Driving receipt를 first build/restart 전에 보호.
9. CF-FQ-046 common scroll owner를 침범하지 않음.
10. Catalog save를 이번 writer 권한에 포함하지 않음.

### 14.3A Implementation-entry Mid-review

v0.1.17 설계를 current `SCFVehicleBuilderTab::Tick()`, `PrepareDrivingBenchmarkLaunch()`, `AcceptCurrentUserDriving()`, `RunBuilderBench.ps1`, `CFVehicleBenchTests.cpp` phase machine과 exact package save 패턴에 다시 대조했다. 최초 P0/P1/P2=0 Design PASS는 구현 착수 근거로 유지할 수 없으며 다음과 같이 교정 Gate를 연다.

- P0: 0
- P1: 4
- P2: 3
- 판정: **FAIL / Implementation HOLD**

#### P1-1 — Progress identity transport가 실제 process chain과 맞지 않음

실제 실행 체인은 `Builder Tab → powershell.exe RunBuilderBench.ps1 → UnrealEditor.exe`다. 현재 Builder가 생성한 RunId는 PowerShell wrapper에는 전달되지만 UnrealEditor command line에는 전달되지 않는다. 따라서 benchmark C++이 progress sidecar에 current `run_id`를 기록하려면 correction에서 PowerShell runner가 exact RunId와 progress path를 UnrealEditor argument로 명시 전달하는 계약을 확정해야 한다. Tab이 UnrealEditor를 직접 실행하는 것으로 재해석하거나 process ownership을 이중화하지 않는다.

교정 요구:

```text
Builder/VM
→ powershell.exe -RunId <RunId>
→ RunBuilderBench.ps1
→ -CFBuilderBenchmarkRunId=<RunId>
→ -CFBuilderBenchmarkProgressPath=<exact path>
→ UnrealEditor benchmark
```

Builder reader는 `snapshot.run_id == RunningDrivingBenchmarkRunId`일 때만 snapshot을 current progress로 인정한다.

#### P1-2 — Recipe Save는 receipt 한 필드가 아니라 Recipe package 전체 저장임

`UPackage::SavePackage()`는 `BuilderDrivingAcceptanceReceipt`만 저장하지 않고 exact current Recipe package의 현재 미저장 변경 전체를 저장한다. 따라서 `차량 제작 기록 저장`이라는 이름을 유지하더라도 USER 설명은 이 write scope를 숨기면 안 된다.

교정 USER wording 기준:

```text
현재 차량의 제작 데이터(Recipe) 1개에 있는 미저장 변경 전체를 저장합니다.
VehicleData, StaticMesh, 데모 차량 목록 등 다른 Asset은 저장하지 않습니다.
```

receipt-only save처럼 오인시키는 표현은 금지한다.

#### P1-3 — post-Save package existence는 새 receipt durable write 증거가 아님

Recipe package는 Save 전부터 disk에 존재한다. 따라서 `DoesPackageExist()` 또는 동등한 package 존재 확인만으로 새 acceptance receipt가 저장됐다고 증명할 수 없다.

즉시 action success는 최소 다음을 분리한다.

1. `UPackage::SavePackage(...) == true`
2. return 뒤 exact Recipe package가 dirty가 아닌지 확인
3. 성공을 가장하기 위해 `SetDirtyFlag(false)` 같은 수동 dirty clear 금지

그리고 P0-07G durable closure는 hot reload/unload를 강제하지 않고 별도 fresh persisted evidence로 확인한다. Save API success/current package clean과 fresh persisted receipt evidence를 하나의 claim으로 합치지 않는다.

#### P1-4 — 현재 dirty USER PASS receipt를 구현 중 lifecycle보다 먼저 보호해야 함

새 Save 버튼은 아직 구현되지 않았기 때문에 지금 dirty일 수 있는 current 2/2 acceptance receipt를 그 버튼으로 보호할 수 없다. Source 편집 자체는 Editor를 건드리지 않고 진행할 수 있지만, linked build 때문에 Editor stop/restart가 필요해지는 시점에는 먼저 exact current `DA_Recipe_Wagon`을 기존 현행 경로로 명시 저장해야 한다.

- Save All 금지.
- 자동 Editor stop/restart 금지.
- exact Recipe가 아직 unsaved인데 DLL lock 때문에 build가 막히면 그 지점에서 중단한다.
- USER가 현행 Editor에서 exact Recipe를 직접 저장하거나, 별도 승인된 exact single-asset save authority가 있을 때만 그 저장을 사용한다.
- Git의 `.uasset modified`만으로 in-memory acceptance receipt가 durable이라고 확대하지 않는다.

#### P2-1 — progress write failure retry storm 방지

`phase group이 바뀔 때만 write` 계약은 write 실패 시에도 last-attempted group 또는 bounded retry 상태를 갱신해야 한다. 그렇지 않으면 filesystem failure가 난 뒤 매 frame 같은 sidecar write를 반복해 benchmark timing을 오염시킬 수 있다.

#### P2-2 — terminal process가 progress cache보다 항상 우선

같은 RunId 안에서도 child crash/failure 직전 sidecar가 `4/7` 또는 `7/7 결과 정리`에 남을 수 있다. PowerShell process terminal을 관측한 순간 running progress cache/timer를 종료하고 final result + exit code authority로 전환한다. `7/7` 자체를 PASS로 해석하지 않는다.

#### P2-3 — Runtime module에서 ReEditor DTO 역참조 금지

`CFVehicleBenchTests.cpp`는 runtime/game module `CarFight_Re` 소속이고 제안한 `CFVehicleBuilderTypes.h`는 `CarFight_ReEditor` 소속이다. progress writer 때문에 `CarFight_Re → CarFight_ReEditor` 역방향 dependency를 만들면 안 된다.

교정 방향:

- runtime benchmark writer는 runtime-local 또는 module-neutral schema field를 직접 기록한다.
- Editor typed DTO는 sidecar parser/presentation에만 사용한다.
- schema string/field compatibility는 focused test로 고정한다.

### 14.3B Mid-review preservation

이번 mid-review는 설계 검수만 수행했다.

- Source/C++/PS1 mutation: 0
- Product Asset mutation/save: 0
- Build/Automation: 0
- Benchmark/USER Driving replay: 0
- current 2/2 USER direct Driving PASS: 보존
- P0-07A/B/C Technical PASS와 기존 final build/focused 8/8/affected 10/10/Step5 1/1: 보존
- CF-FQ-046 common Page Shell/Scroll owner: 변경 없음

exact next는 **`VBHAI-P0-07E Design Correction + Re-review`**다. P1 4건을 교정하고 P0/P1 0을 다시 확인하기 전 source implementation에 들어가지 않는다.

### 14.3C Design Correction + Source-based Re-review

v0.1.18의 P1/P2를 위 계약으로 교정한 뒤 current `PrepareDrivingBenchmarkLaunch()`, `HandleRunDrivingBenchmark()`, `SCFVehicleBuilderTab::Tick()`, `RunBuilderBench.ps1`, `CFVehicleBenchTests.cpp` state machine, `AcceptCurrentUserDriving()`와 existing UE 5.8 `UPackage::SavePackage` pattern에 다시 대조했다.

재검수 결과:

- P0: 0
- P1: 0
- P2: 0
- 판정: **PASS / Implementation Gate OPEN**

closure:

1. P1-1 PASS — RunId authority가 실제 `Builder → PowerShell → UnrealEditor` chain과 정확히 일치하며 wrapper가 RunId/progress path를 Editor까지 전달한다. process owner는 기존 Tab/PowerShell 구조를 유지한다.
2. P1-2 PASS — explicit save가 receipt-only가 아니라 **exact current Recipe package의 미저장 변경 전체**를 저장함을 USER 문구와 writer contract에 명시했다. 다른 Asset Save 권한은 추가하지 않는다.
3. P1-3 PASS — immediate `SavePackage true + package clean`과 P0-07G fresh persisted receipt readback을 분리했다. package existence나 manual dirty clear를 durable 증거로 사용하지 않는다.
4. P1-4 PASS — first build/restart/binary replacement 전 current dirty acceptance receipt 보호 Gate를 명시했다. unsaved 가능 상태에서 DLL lock을 이유로 Editor를 자동 종료하지 않는다.
5. P2-1 PASS — `LastAttemptedPhaseGroup`을 write 전 갱신해 같은 phase의 per-frame retry storm을 차단하며 one-run write attempt를 최대 7회로 bound한다.
6. P2-2 PASS — PowerShell process terminal이 running progress보다 항상 우선하며 terminal 뒤 `7/7` snapshot은 성공 authority가 아니다.
7. P2-3 PASS — runtime writer는 `CarFight_Re` private schema writer로 유지하고 ReEditor DTO를 참조하지 않는다. Editor DTO는 parser/presentation에만 사용한다.
8. current internal phase machine의 `WaitForPIE/Acceleration/TopSpeed/Braking/Yaw/Turning/Finish`를 7 USER group으로 손실 없이 coarse projection할 수 있음을 current Source에서 확인했다.
9. existing SavePackage pattern(`FSavePackageArgs`, deterministic `.uasset` filename, `UPackage::SavePackage`)이 ReEditor에 이미 존재하므로 신규 저장 프레임워크가 필요하지 않다.
10. CF-FQ-046 common Page Shell/Scroll ownership, current 2/2 USER Driving PASS, P0-07A/B/C Technical evidence는 그대로 보존된다.

이번 PASS는 **설계/구현 착수 가능 판정**이다. C++/PS1 implementation, Build, Automation, fresh persisted readback이 완료됐다는 뜻이 아니다. exact next는 `VBHAI-P0-07E Step 8 Progress + Explicit Recipe Save Implementation`이다.

### 14.4 VBHAI-P0-07F — Correction Technical Validation — PASS

P0-07E Step 8 Progress + Explicit Recipe Save 구현은 Technical Validation까지 닫혔다.

- 첫 official build `2cf9d8ba5d104f33b5c46e52a97debfd`는 runtime benchmark progress JSON writer의 `Json` link dependency 누락으로 FAIL했다. `CarFight_Re.Build.cs v1.7.0`에서 `Target.bBuildEditor` Private `Json` dependency만 추가해 packaged/runtime dependency 확장을 피했다.
- corrected official build `70765dae25d545438176293a31c7a95f` Exit 0 PASS.
- focused test source 포함 final official build `9d789a07245a4bdda36bf81eba0fb138` Exit 0 PASS.
- exact `BuilderStep8Driving` process `8c8c8c8b78b84225bf893420d0664efb` **1/1 PASS**. progress schema/RunId/phase/count fail-closed와 Recipe Save preflight/writer의 `/Temp` PackageInvalid Save0 boundary를 검증했다.
- actual Wagon progress transport process `9ce95e023e4445dda48028d652656971`, RunId `3b8a7d31-2de7-46ef-9b11-7e0cb213b61a`, expected Target hash `8aad29d04e008fcd2acb2e6470cd87f1`에서 running sidecar `top_speed 3/7`, terminal sidecar `finalizing 7/7`, final result success / EngineExitCode 0 / exact hash binding을 확인했다.
- pre/post persisted Recipe evidence에서 USER Driving receipt와 accepted 2/2 Product baseline이 benchmark 실행으로 변경되지 않았음을 확인했다.
- Step 8 post-driving `차량 제작 기록 저장`은 **USER Driving PASS를 기록한 뒤 새로 dirty가 된 current Recipe 한 package를 저장하는 기능**으로 유지한다.

### 14.5 VBHAI-P0-07G — USER Re-Acceptance — HOLD

P0-07E/F 뒤 USER가 실제 제작 흐름을 다시 밟는 중 Step 7→8 stage contract defect를 발견했다.

재현:

1. USER가 UAT 목적으로 Socket/Hardpoint/Mount를 하나 더 추가하고 Step 1~7을 정상 진행했다.
2. 화면은 Step 7 `최종 검토 [완료]`를 표시했다.
3. Step 8은 `VehicleData에 저장되지 않은 변경이 있습니다`를 표시하고 기술 주행 측정을 시작할 수 없었다.
4. current Source에서도 `EvaluateFinalReviewStep()`은 `FinalReviewResult.bCanCompleteFinalReview`의 semantic Diff 0만으로 Step 7을 `Complete` 처리하지만, `PrepareDrivingBenchmarkLaunch()`는 Recipe/Target dirty=false + disk existence + AppliedState/current Target hash exact를 추가 요구한다.

따라서 결함은 **Step 8 저장 버튼 누락이 아니라 Step 7 완료 정의가 downstream handoff보다 약한 것**이다.

UAT 화면의 장착 구성 3/3은 이번 재현을 위한 live test state로만 기록한다. USER가 새 Product baseline으로 유지 승인하기 전까지 기존 accepted 2/2 baseline을 자동 3/3으로 승격하지 않는다.

### 14.6 VBHAI-P0-07H — Step 7 Durable Final Commit Design Correction

#### A. Stage invariant

다음 invariant를 새 기준으로 고정한다.

```text
Step 7 Complete
=> Final Review semantic Complete
&& exact current Target VehicleData persisted + clean
&& exact current Recipe persisted + clean
&& Recipe.AppliedState.AppliedDefinitionHash == fresh current Target DefinitionHash
=> 정상 Step 8 진입 직후 saved-state 이유로 benchmark가 막히지 않음
```

`FCFBuilderFinalReviewResult.bCanCompleteFinalReview`은 앞으로 **semantic zero-diff fact**일 뿐 Step 7 전체 `Complete` authority가 아니다.

`PrepareDrivingBenchmarkLaunch()`의 dirty/package/AppliedState guard는 제거하거나 완화하지 않는다. 이 guard는 외부 수정·비정상 진입에 대한 defense-in-depth로 남고, 정상 Builder 흐름에서는 Step 7 Complete가 이미 같은 handoff 조건을 만족해 USER가 이 blocker를 보지 않는 것이 목표다.

#### B. 하나의 Step 7 primary commit action

Step 8에 `주행 측정 준비 저장` 같은 새 버튼을 만들지 않는다.

Step 7의 기존 Final Review primary action을 durable commit으로 확장한다.

권장 typed action:

```text
ECFBuilderFinalCommitAction
- ApplyAndPersist                 // semantic diff 있음: existing R3 Apply 뒤 durable pair 저장
- PersistDirtyPair                // semantic diff 0 + AppliedState exact + Target/Recipe 중 하나 이상 dirty
- FinalizeAppliedStateAndPersist  // semantic diff 0 + Target exact + AppliedState만 stale인 repairable 상태
- None                            // semantic complete + pair durable ready
- Blocked                         // identity/provenance/package 상태를 안전하게 확정할 수 없음
```

권장 read-only projection:

```text
FCFBuilderFinalCommitPreflight
- Action
- bSemanticComplete
- bTargetDirty
- bRecipeDirty
- bTargetPersisted
- bRecipePersisted
- bAppliedStateCurrent
- Diagnostic
- IsComplete() = Action == None && 위 durable invariant 충족
```

UI의 primary action 위치는 하나를 유지한다.

- semantic diff 있음: `최종 변경 적용 및 저장`
- semantic diff 0 + pair dirty: `최종 상태 저장`
- pair durable ready: action 불필요, Step 7 `[완료]`
- unsafe state: `[확인 필요]` 또는 Blocked reason

따라서 USER가 Step 7을 완료하기 위해 Step 8로 이동한 뒤 다시 저장 작업을 찾는 흐름은 금지한다.

#### C. explicit persistence authority

일반적인 silent auto-save 금지 원칙은 유지한다. 다만 USER가 Step 7 primary commit action을 직접 누르고 confirmation에서 저장 범위를 승인한 경우, 그 action은 **명시적 Apply + Persist 권한**이다.

confirmation은 다음을 숨기지 않는다.

```text
최종 변경 적용 및 저장

현재 차량의 최종 변경을 적용하고 아래 두 Asset의 현재 미저장 변경 전체를 저장합니다.
- 현재 VehicleData
- 현재 차량 제작 기록(Recipe)

저장하지 않음:
- StaticMesh
- 차량 전용 Physics Profile
- Reference Evidence
- Runtime Catalog
- 다른 차량 Recipe/VehicleData
- Save All
```

이번 correction의 durable handoff set은 **Step 7 R3이 직접 mutating하고 Step 8 saved benchmark가 직접 요구하는 exact current Target VehicleData + current Recipe 두 package**로 제한한다. 이전 Step의 Profile/Evidence/StaticMesh save lifecycle을 Step 7에서 암묵적으로 확장하지 않는다.

#### D. Apply + Save order

semantic diff가 있는 정상 경로:

```text
fresh Final Review
→ USER 승인
→ existing ApplyBuilderFinalReview R3
→ fresh Target/Recipe identity + AppliedState 재검증
→ Target VehicleData SavePackage
→ Target Save true + clean 확인
→ Recipe SavePackage
→ Recipe Save true + clean 확인
→ fresh in-memory handoff invariant 재검증
→ RefreshCurrentState
→ Step 7 Complete
```

**Target → Recipe 순서**를 고정한다. Recipe의 AppliedState가 Target DefinitionHash를 가리키므로 Recipe를 먼저 저장해 아직 durable하지 않은 Target 상태를 가리키는 창을 만들지 않는다.

Save success는 각 package별 `SavePackage == true + IsDirty()==false`로 확인한다. `DoesPackageExist()`만으로 방금 write된 bytes를 성공으로 간주하지 않고, 수동 `SetDirtyFlag(false)`로 성공을 가장하지 않는다.

#### E. semantic diff 0 recovery는 `PersistDirtyPair`와 `FinalizeAppliedStateAndPersist`로 분리한다

Apply가 이미 성공해 semantic diff는 0이어도 durable handoff가 준비되지 않았다면 Step 7을 `[완료]`로 만들지 않는다.

첫 번째 lane은 **AppliedState가 current exact인 상태에서 package dirty만 남은 경우**다.

```text
PersistDirtyPair
- Target dirty + Recipe clean → Target만 Save
- Target clean + Recipe dirty → Recipe만 Save
- 둘 다 dirty → Target → Recipe 순서로 Save
- 둘 다 clean → 이 action 대상 아님
```

이 lane은 DefinitionApply를 다시 실행하지 않는다. clean package를 불필요하게 rewrite하지 않으며, pair 전체 identity/safety는 save 직전에 fresh 검증한다.

두 번째 lane은 **Target은 fresh Resolve와 semantic exact지만 Recipe AppliedState만 stale인 경우**다. 특히 `Target Save 성공 → Recipe Save 실패 → Editor 종료/재기동` 뒤 disk에서 둘 다 clean으로 보이지만 AppliedState만 옛 상태인 partial persistence를 복구한다.

```text
FinalizeAppliedStateAndPersist
1. fresh Final Review semantic PASS / Diff 0 확인
2. live Target fresh DefinitionHash == fresh resolved DefinitionHash 확인
3. fresh Recipe fingerprint / SourceSignature / ResolverRevision 확인
4. Target이 dirty면 Target을 먼저 저장
5. 다시 fresh state 확인
6. Apply Service-owned finalize seam이 existing BuildAppliedState() authority로 Recipe AppliedState만 재구축
7. Target mutation 0 / Target dirty 신규 발생 0
8. Recipe dirty 표시
9. Recipe만 explicit Save
10. shared saved-handoff preflight로 durable ready 재확인
```

`BuildAppliedState()` 의미를 VM에 복제하지 않는다. 현재 `ApplyBuilderFinalReview()`는 `bCanApply`/Target Diff가 있어야 하고 current `ApplyInternal()`은 Target mutation을 전제로 결과를 기록하므로 no-diff recovery에 억지로 재사용하지 않는다. 별도의 service-owned AppliedState finalize/repair seam을 사용한다.

Target hash가 fresh Resolve와 다르거나 Recipe fingerprint/source/revision이 예상과 다르면 repair하지 않고 fail-closed한다.

#### F. partial persistence / failure policy

pair save는 파일 2개라 원자적 transaction이 아니다. 따라서 partial success를 숨기지 않는다.

권장 outcome:

```text
ApplyFailed
FinalizeAppliedStateFailed
TargetSaveFailed
TargetSavedRecipeSaveFailed
SaveStateUnconfirmed
Committed
CommittedRefreshWarning
Blocked
```

규칙:

- Target save 실패 → Recipe save 시도 금지.
- Target save 성공 + Recipe save 실패 → Target을 자동 rollback/재저장하지 않는다. Step 7은 Ready/`저장 마무리 필요`, same primary action은 `PersistDirtyPair`로 dirty Recipe만 마저 저장한다.
- pair save 성공 + refresh 실패 → persistent commit success를 실패로 뒤집지 않고 `저장 완료 / 화면 상태 다시 확인 필요` warning으로 분리한다. fresh evaluator가 handoff invariant를 다시 읽기 전까지 화면의 Step 7 Complete projection을 추측하지 않는다.
- 자동 retry / Save All / unrelated package save 금지.

#### G. guarded Undo도 durable boundary를 따른다

현재 Step 7은 successful Apply가 발급한 exact guarded Undo token을 제공한다. Step 7이 durable commit으로 바뀐 뒤 Undo가 memory-only이면 다시 disk와 memory가 어긋난다.

따라서 USER가 `방금 적용한 변경 되돌리기`를 명시 승인하면:

```text
guarded Undo
→ reverted Target/Recipe identity 확인
→ Target Save
→ Recipe Save
→ pair clean/current 확인
→ Refresh
```

까지 같은 explicit action에 포함한다.

Undo persistence가 일부 실패하면 Step 7을 Complete로 표시하지 않으며 자동 rollback/retry하지 않는다. 기존 guarded token의 stale/external-change fail-closed 조건은 유지한다.

#### H. Step state projection

Step 7 상태 의미를 다음처럼 고정한다.

- `Complete`: semantic Complete + durable pair invariant 모두 충족.
- `Ready`: Apply가 필요하거나 safe persist-only가 필요함.
- `Stale`: Final Review proposal/provenance가 달라져 fresh review 필요.
- `Blocked`: Recipe/Target/package/identity를 안전하게 확정할 수 없음.

`Final Review PASS / Target Diff 0`이라는 문장은 더 이상 단독으로 `[완료]`를 뜻하지 않는다.

Step 7 summary 예:

```text
최종 내용은 적용됐지만 저장 마무리가 필요합니다.
VehicleData: 저장 필요
차량 제작 기록: 저장 필요
아래 '최종 상태 저장'을 완료하면 8단계 기술 주행 측정으로 바로 진행할 수 있습니다.
```

#### I. Step 8 post-driving Recipe Save와의 관계

P0-07E의 Step 8 `차량 제작 기록 저장`은 제거하지 않는다.

두 저장의 의미는 명확히 다르다.

```text
Step 7 durable commit
= 주행 측정 전에 Authoring 결과(Target + Recipe)를 확정/저장

Step 8 차량 제작 기록 저장
= 실제 USER Driving PASS 뒤 새로 생성된 acceptance receipt를 Recipe에 저장
```

Step 8 pre-benchmark 저장 버튼은 추가하지 않는다.

#### J. implementation owner / planned files

설계 재검수 PASS 뒤 최소 변경 후보:

- `CFVehicleBuilderTypes.h` — Final Commit / Saved Handoff typed preflight·blocker·outcome.
- `CFVehicleBuilderVM.h/.cpp` — single fresh Saved Handoff authority, Step 7 action 합성, Step 8 pre-benchmark projection/launch guard 통합, durable undo orchestration.
- `CFVehicleApplyService.cpp` + 실제 public declaration owner — existing private `BuildAppliedState()`를 재사용하는 Recipe-AppliedState-only finalize seam. Target mutation/save는 하지 않음.
- `CFVehicleAuthoringService.h` + `CFVehicleBuilderReview.cpp` — Builder에서 service-owned finalize를 호출해야 할 경우 exact approval/fresh review facade만 최소 노출. 기존 generic Final Review semantic 의미는 유지.
- `CFVehicleBuilderTab.h/.cpp` — 기존 Step 7 primary action/confirmation/status 문구 교정. 새 Step 8 pre-benchmark save action은 추가하지 않음.
- `CFVehicleBuilderPresent.h/.cpp` — 필요할 때 USER-facing durable commit/readiness formatter만 최소 확장. 046 common Page Shell/Scroll layout은 수정하지 않음.
- `CFVehicleAuthoringVMTests.cpp` + existing Apply/Step7/Step8 tests — completion/persistence/restart recovery/failure matrix.

generic `ApplyBuilderFinalReview` 및 `DefinitionApply` 자체에 silent Save side effect를 넣지 않는다. no-diff AppliedState repair는 existing ApplyInternal을 억지로 재사용하지 않고 service-owned finalize seam으로 분리한다. Step 7과 Step 8은 동일한 Saved Handoff read authority를 소비하되 runner/process orchestration과 post-driving Recipe Save policy는 각각 기존 owner를 유지한다.

#### K. mandatory regression

1. semantic Diff 0 + both clean + persisted + RecipeFingerprint/SourceSignature/DefinitionHash/ResolverRevision AppliedState exact → Step 7 Complete.
2. semantic Diff 0 + Target dirty + Recipe clean + AppliedState exact → `PersistDirtyPair`, Target save 1 / Recipe save 0.
3. semantic Diff 0 + Target clean + Recipe dirty + AppliedState exact → Recipe save 1 / Target save 0.
4. semantic Diff 0 + both dirty + AppliedState exact → Target → Recipe 순서로 save.
5. semantic diff 있음 → `ApplyAndPersist`, existing Apply success + Target→Recipe durable save 뒤 Complete.
6. Target Save failure → Recipe Save 0 / Complete 금지.
7. Target Save success + Recipe Save failure → partial outcome / Complete 금지 / same-lifetime retry는 dirty Recipe만 저장하고 DefinitionApply replay 0.
8. 7번 뒤 fresh restart로 Target clean/new + Recipe clean/old AppliedState → `FinalizeAppliedStateAndPersist`; Target write 0, service-owned BuildAppliedState authority로 Recipe AppliedState 재구축 + Recipe save.
9. finalize preflight에서 Target fresh hash != fresh resolved DefinitionHash → fail-closed / mutation 0.
10. finalize preflight에서 Recipe fingerprint/source signature/resolver revision mismatch → fail-closed / mutation 0.
11. SavePackage true지만 package dirty → SaveStateUnconfirmed / Complete 금지.
12. pair Save success + refresh failure → persistent success 보존 + refresh warning, false-negative 금지.
13. USER confirmation 뒤 selection/Recipe/Target/hash가 변경됨 → old approval write/save 0.
14. Recipe 또는 Target이 transient/non-`/Game`/invalid long package → Blocked / save 0.
15. Step 7 Complete 상태에서는 정상 `PrepareDrivingBenchmarkLaunch()` saved-state gate가 Recipe/Target dirty·package missing·AppliedState stale 이유로 실패하지 않음.
16. Step 8에 current benchmark가 없고 Recipe 또는 Target이 pre-benchmark dirty면 화면도 benchmark-ready로 표시하지 않음.
17. exact current benchmark 뒤 persistent USER Driving receipt만 Recipe에 새로 생겨 dirty인 경우 benchmark current는 유지하고 Step 8 post-driving Recipe Save pending으로 표시.
18. current benchmark 뒤 Target dirty/hash drift가 생기면 benchmark stale/block.
19. `ReadDrivingApplyPreflight()`가 same Saved Handoff pair facts를 합성/재사용해 버튼 enable과 production Apply guard가 계속 일치.
20. guarded Undo success 뒤 reverted pair는 dirty 상태로 재평가되고 필요한 Target→Recipe persistence를 수행하되 pre-Apply semantic diff가 다시 생기면 Step 7 Complete를 강제하지 않음.
21. Undo save partial failure → Complete 금지 / no auto rollback/retry.
22. Save All / StaticMesh / Profile / Evidence / Catalog / unrelated Recipe·VehicleData mutation/save 0.
23. 기존 Step 8 progress + post-driving Recipe explicit Save regression 보존.
24. Saved Handoff authority는 cached `BuildDrivingTargetIdentity()`를 사용하지 않고 live UObject fresh hash를 사용하며 Builder refresh 없이 Target semantic mutation을 즉시 감지.
25. `RefreshDrivingBenchmarkState()` / `HasCurrentUserDrivingAcceptance()` / `AcceptCurrentUserDriving()`도 current benchmark/receipt Target identity에 cached hash를 사용하지 않고 fresh Target identity를 사용.
26. USER Driving PASS 승인 직후 Recipe receipt write + `RebuildStepStates()`에서 `PostDrivingReceiptSavePending`이 성립하면 Step 7은 Complete를 유지하고 Step 8은 잠기지 않으며 exact Recipe Save action이 계속 reachable.
27. post-driving Recipe dirty라도 semantic fingerprint/source/resolved hash 또는 Target/AppliedState가 drift하면 downstream exception을 거부하고 fail-closed.

### 14.7 VBHAI-P0-07H Design Review — P0 0 / P1 4 / P2 3 / HOLD

v0.1.20을 current `ReadBuilderFinalReview()`, `ApplyBuilderFinalReview()`, `UndoBuilderFinalApply()`, `FCFVehicleApplyService::ApplyInternal()`, `EvaluateFinalReviewStep()`, `PrepareDrivingBenchmarkLaunch()`, `EvaluateDrivingTestStep()`, `ReadDrivingApplyPreflight()` 및 P0-07E Recipe Save writer와 다시 대조했다.

판정:

- P0: 0
- P1: 4
- P2: 3
- Implementation: **HOLD**

큰 방향인 `Step 7 Complete => Step 8 saved benchmark handoff ready`와 Step 8 pre-benchmark 추가 Save 버튼 금지는 유지한다. 다만 아래 설계 공백을 교정한 뒤 재검수해야 한다.

#### P1-1 — Step 7과 Step 8 saved-state를 하나의 fresh authority로 통합해야 한다

v0.1.20은 Step 7 durable evaluator를 새로 만들면서 existing `PrepareDrivingBenchmarkLaunch()` guard의 의미를 보존한다고 적었지만, current Source는 이미 서로 다른 Target identity authority를 사용한다.

- `PrepareDrivingBenchmarkLaunch()` → `BuildDrivingTargetIdentity()` → Authoring ViewModel의 cached `CurrentDefinition.DefinitionHash`.
- `ReadDrivingApplyPreflight()` / Step 8 Recipe Save preflight → `BuildFreshDrivingTargetIdentity()` → live Target UObject의 fresh immutable Definition Snapshot.

P0-07B에서 cached identity false-ready를 이미 한 번 교정했으므로 Step 7에 세 번째 독립 saved-state 판정기를 추가하면 안 된다.

교정 요구:

```text
FCFBuilderSavedHandoffPreflight 또는 동등한 단일 read-only authority
- exact current selection / Recipe identity
- exact Target identity
- live Target UObject에서 fresh DefinitionHash
- Recipe/Target package safety
- Recipe/Target dirty
- Recipe/Target persisted existence
- Recipe AppliedState exactness
- diagnostic
```

Step 7 Complete 판정과 `PrepareDrivingBenchmarkLaunch()`의 **pre-benchmark saved-state 부분**이 같은 evaluator를 소비해야 한다. `PrepareDrivingBenchmarkLaunch()`의 runner/path/RunId orchestration은 그대로 유지한다.

#### P1-2 — Target-save/Recipe-save partial persistence의 fresh-restart recovery authority가 없다

현재 설계는 같은 Editor lifetime에서:

```text
Target Save 성공
→ Recipe Save 실패
→ Recipe는 memory에서 dirty
→ PersistCurrent로 Recipe save 재시도
```

까지는 복구할 수 있다.

그러나 그 사이 Editor/process가 종료되면 disk에는:

```text
Target = 새 Definition
Recipe = 이전 AppliedState
```

가 남을 수 있다. fresh load 뒤에는 둘 다 package clean일 수 있지만 `AppliedState`만 stale하다. 이 상태는 단순 `PersistCurrent`로 고칠 수 없고, `ReadBuilderFinalReview()`의 semantic Diff가 0이면 current `ApplyBuilderFinalReview()`도 정상 Apply proposal을 만들지 않는다.

또 `FCFVehicleApplyService`가 소유하는 `BuildAppliedState()` authority를 VM에서 복사 구현하면 안 된다.

교정 요구:

- **fresh-restart partial recovery lane**을 명시한다.
- Target이 fresh Resolve 결과와 exact semantic match이고 Recipe semantic fingerprint/source provenance가 current일 때만 AppliedState를 authoritative Apply service logic에서 재구축/재bind한다.
- `BuildAppliedState()` 의미를 VM에서 중복 구현하지 않는다. existing Apply service authority를 재사용할 service-owned repair/finalize seam 또는 no-diff finalize lane을 설계한다.
- stale provenance, Target mismatch, unknown state이면 fail-closed.
- recovery 뒤 Recipe를 explicit Save하고 pair handoff preflight가 Complete를 확인해야 한다.

#### P1-3 — Step 7 save writer에 exact pair package safety + TOCTOU revalidation 계약이 부족하다

v0.1.20은 저장 범위를 두 package로 제한했지만 writer preflight에서 두 package 모두에 대한 안전 조건을 아직 고정하지 않았다. Existing Step 8 Recipe Save는 이미 `/Game` package, non-transient, valid long package, current selection exactness를 검사한다.

Step 7은 그보다 범위가 넓으므로 최소 다음을 fresh mutation 직전에 다시 검증해야 한다.

- current selection의 Recipe path == loaded Recipe.
- Recipe의 Target path == save 대상 Target.
- Recipe/Target 모두 non-transient `/Game/*` valid long package.
- live Target fresh hash와 Final Review/finalize expectation exact.
- Recipe fingerprint/provenance/AppliedState expectation exact.
- USER confirmation 이후 selection/identity/action이 바뀌었으면 old approval로 save 금지.

USER confirmation은 두 package의 **현재 미저장 변경 전체**를 저장한다는 기존 문구를 유지한다. writer는 unrelated package를 저장하지 않는다.

#### P1-4 — Step 8 화면 상태와 실제 benchmark launch guard도 같은 saved-pair 의미로 맞춰야 한다

현재 `EvaluateDrivingTestStep()`은 Target dirty만 막고 Recipe dirty는 허용하지만, `PrepareDrivingBenchmarkLaunch()`는 benchmark 시작 전에 Recipe와 Target 둘 다 clean이어야 한다.

따라서 Step 7을 고쳐도 Step 8 진입 후 Recipe가 다시 dirty가 되면:

```text
화면: Benchmark 실행 Ready
클릭: Recipe dirty라 launch 거부
```

가 다시 발생할 수 있다.

교정 요구:

- **current benchmark가 아직 없을 때** Step 8 Ready projection도 Step 7과 같은 saved pair preflight를 사용한다.
- current benchmark가 이미 exact Target에 binding된 뒤 USER Driving PASS receipt 때문에 Recipe만 dirty가 된 경우에는 기존 benchmark를 무효화하지 않는다. 이 dirty는 Step 8 post-driving `차량 제작 기록 저장` lane이 소유한다.
- 즉 `pre-benchmark authoring dirty`와 `post-driving acceptance receipt dirty`를 동일하게 취급하지 않는다.

#### P2-1 — PersistCurrent는 clean package를 불필요하게 rewrite하지 않는다

ApplyAndPersist 직후에는 Target/Recipe가 모두 dirty이므로 Target→Recipe 순서가 그대로 성립한다. PersistCurrent recovery에서는 dirty package만 실제 `SavePackage`한다.

- Target dirty + Recipe clean → Target만 save.
- Target clean + Recipe dirty → Recipe만 save.
- 둘 다 dirty → Target→Recipe.
- 둘 다 clean + AppliedState stale → P1-2 recovery lane이며 단순 Save가 아니다.

모든 경우 pair 전체의 identity/safety는 preflight에서 확인한다.

#### P2-2 — generic Final Review의 `Complete` 용어를 semantic-only로 명확히 한다

`FCFBuilderFinalReviewResult.bCanCompleteFinalReview`은 기존 service 계약상 **Target Diff 0 + blocker 0**인 semantic fact로 유지한다. 이를 Step 전체 Complete 의미로 바꾸지 않는다.

다만 current `ReadBuilderFinalReview()` message의 `다음 Driving Test로 진행할 수 있습니다`는 새 stage contract와 충돌한다. service message 또는 Builder Presentation에서 다음처럼 구분해야 한다.

```text
Final Review semantic PASS
!=
Step 7 durable Complete
```

Step 7 전체 Complete authority는 saved handoff preflight와 합성한 VM projection 하나가 소유한다.

#### P2-3 — Save mechanics는 공유하되 Step 7/Step 8 policy를 합치지 않는다

현재 Step 8 Recipe Save는 `SavePackage true + package clean + refresh-warning separation`을 이미 구현했다. Step 7에서 같은 mechanics를 복사해 두 번째 구현으로 만들지 않는다.

권장:

- private/internal single-package explicit save primitive만 공유.
- Step 7 pair handoff policy/preflight/outcome은 별도.
- Step 8 post-driving acceptance Recipe Save policy/preflight/outcome도 별도 유지.

즉 저장 **기계적 실행 코드**만 재사용하고 두 기능의 승인 조건·저장 범위·USER 의미는 합치지 않는다.

#### Review closure

보존되는 설계:

1. Step 8 pre-benchmark Save 버튼 추가 금지.
2. 정상 flow에서 Step 7 primary action 하나가 Apply/Persist를 끝내고 Step 8로 handoff.
3. generic DefinitionApply에 silent auto-save side effect 추가 금지.
4. Save All / StaticMesh / Profile / Evidence / Catalog / unrelated Asset save 금지.
5. Target→Recipe ordering은 둘 다 dirty일 때 유지.
6. guarded Undo는 optional capability이며 Step 7 Complete의 필수 조건이 아니다. Undo를 실제 실행한 경우에는 reverted pair의 durable recovery를 끝내기 전 Complete로 복귀하지 않는다.
7. current accepted Wagon 2/2 baseline과 P0-07E/F technical evidence는 보존한다. UAT live 3/3은 USER 승인 전 baseline 승격하지 않는다.

exact next는 **`VBHAI-P0-07H P1/P2 Design Correction + Re-review`**다. P0/P1 0 전 Source implementation을 시작하지 않는다.

### 14.8 VBHAI-P0-07H Design Correction v0.1.22 — authoritative correction

이 절은 14.7의 P1/P2 findings를 해소하며, 앞선 14.x 내용과 충돌할 경우 **14.8을 current implementation contract로 우선**한다.

#### A. Single Fresh Saved Handoff Authority

Step 7과 Step 8의 pre-benchmark saved-state truth는 VM의 하나의 read-only authority가 소유한다. 명칭은 구현 시 `FCFBuilderSavedHandoffPreflight` / `ReadSavedHandoffPreflight()` 또는 동등한 직관적 이름을 사용한다.

이 authority는 cached Authoring ViewModel DefinitionHash를 신뢰하지 않고 current live Target UObject의 immutable Definition Snapshot에서 fresh hash를 계산한다. 또한 fresh Final Review/Resolve authority를 재사용하며 별도 Resolver를 구현하지 않는다.

최소 판정 범위:

```text
- current selection Recipe path == loaded Recipe
- Recipe Target path == loaded Target
- Recipe/Target non-transient + /Game/* + valid long package
- fresh Recipe semantic fingerprint
- fresh Resolve SourceSignature / ResolvedDefinitionHash / ResolverRevision
- live Target fresh full DefinitionHash
- Target fresh hash == fresh resolved DefinitionHash
- Recipe AppliedState:
    AppliedRecipeFingerprint
    AppliedSourceSignature
    AppliedDefinitionHash
    ResolverContractRevision
  가 fresh authority와 exact match하는지
- Recipe/Target dirty
- Recipe/Target disk persistence
- diagnostic / typed blocker
```

current Source의 `ReadBuilderFinalReview()`는 `ResolveVehiclePreview()`의 common result envelope를 그대로 보존하며 `Review.Operation.CurrentRecipeFingerprint`, `CurrentTargetDefinitionHash`, `CurrentSourceSignature`, `CurrentResolvedDefinitionHash`, `ResolverContractRevision`을 이미 제공한다. 따라서 이 identity를 새로 복제하거나 `FCFBuilderFinalReviewResult`에 중복 field를 추가할 필요가 없다. live Target hash만 `BuildFreshDrivingTargetIdentity()`와 동일 snapshot authority로 다시 읽어 exact 비교한다.

소비자:

1. `EvaluateFinalReviewStep()` — semantic Final Review PASS와 phase-aware Saved Handoff projection을 합성해서만 Step 7 `Complete`.
2. `PrepareDrivingBenchmarkLaunch()` — runner/path/RunId orchestration 전에 같은 **strict pre-benchmark** saved-state 결과를 재사용.
3. `RefreshDrivingBenchmarkState()` / `EvaluateDrivingTestStep()` — current benchmark의 Target binding도 cached identity가 아니라 same fresh Target identity를 사용.
4. `ReadDrivingApplyPreflight()` — benchmark 조건을 추가하기 전 pair saved-state facts는 같은 authority를 합성/재사용.
5. `HasCurrentUserDrivingAcceptance()` / `AcceptCurrentUserDriving()` — persistent receipt read/write와 benchmark Target 비교도 same fresh Target identity를 사용. 현재 cached `BuildDrivingTargetIdentity()`를 acceptance authority로 남기지 않는다.

#### B. Final Commit action state machine

```text
semantic diff 있음
→ ApplyAndPersist

semantic diff 0 + Target clean/persisted + AppliedState exact
+ exact current benchmark + fresh persistent USER Driving acceptance receipt
+ Recipe dirty
→ PostDrivingReceiptSavePending / Step 7 Complete 유지 / Step 8 Recipe Save owner

semantic diff 0 + AppliedState exact + package dirty
→ PersistDirtyPair

semantic diff 0 + Target == fresh Resolve + AppliedState stale but repairable
→ FinalizeAppliedStateAndPersist

semantic diff 0 + AppliedState exact + both clean/persisted
→ None / Step 7 Complete

그 외 identity/provenance/Target mismatch/unsafe package
→ Blocked
```

`PostDrivingReceiptSavePending`은 **downstream mutation exception**이며 일반 Recipe dirty 우회가 아니다. exact current benchmark가 fresh Target에 binding되고, persistent USER Driving receipt가 fresh Target에 exact binding되고, Final Review semantic Diff 0 + Target clean/persisted + AppliedState exact가 모두 유지될 때만 성립한다. 이 상태에서는 Step 7 primary save action을 다시 노출하지 않고 Step 8의 existing `차량 제작 기록 저장` action이 Recipe dirty를 소유한다.

`FCFBuilderFinalReviewResult.bCanCompleteFinalReview`은 계속 **semantic no-diff/blocker0 fact**다. Step 7 `Complete` 자체로 재정의하지 않는다. USER-facing 문구도 `Final Review semantic PASS`와 `Step 7 durable Complete`를 구분하고, semantic PASS만으로 `다음 Driving Test로 진행 가능`이라고 말하지 않는다.

#### C. service-owned AppliedState finalize / fresh-restart recovery

fresh restart partial persistence를 위해 low-level Apply authority에 Recipe-AppliedState-only finalize seam을 추가한다.

필수 계약:

```text
fresh Resolve + semantic Diff 0
live Target fresh hash == resolved hash
expected Recipe fingerprint/source/revision exact
→ existing private BuildAppliedState()로 NewAppliedState 구성
→ explicit transaction에서 Recipe->AppliedState만 교체
→ Recipe dirty + PostEditChange
→ Target mutation 0
→ Target dirty 신규 발생 0
→ Save는 수행하지 않음
```

Builder UI에서 이 write를 호출할 때는 existing Authoring approval/fresh-review facade 원칙을 유지한다. 위험도는 authoritative AppliedState provenance를 바꾸므로 explicit DefinitionApply 계열 승인으로 취급할 수 있으나, **Target field Diff Apply와는 다른 exact scope**로 분리한다.

`ApplyBuilderFinalReview()`는 current Source상 `FreshReview.bCanApply`가 필요하고, `ApplyInternal()`은 actual Target Apply transaction/Target mutation 결과를 전제로 하므로 no-diff finalize에 재사용하지 않는다.

fresh restart에서 same stale AppliedState가 다시 나타나도 위 조건을 fresh 재검증한 뒤 같은 finalize lane을 안전하게 반복할 수 있다. Target/provenance가 달라졌으면 자동 repair하지 않는다.

#### D. exact pair writer + TOCTOU

Step 7 USER confirmation은 current Target VehicleData와 current Recipe **두 package의 현재 미저장 변경 전체**를 저장할 수 있다는 write scope를 명시한다. Save All 및 unrelated package는 권한 밖이다.

USER confirmation 뒤 실제 Apply/finalize/save 직전과 각 phase 경계에서 fresh preflight를 다시 읽는다. selection, Recipe path, Target path, expected fresh hash/fingerprint/source/revision/action이 달라지면 old confirmation을 재사용하지 않는다.

저장 mechanics는 internal single-package helper로 공유할 수 있다.

```text
SavePackage true
+ package clean 확인
+ manual dirty clear 금지
+ refresh failure는 persistent success와 분리
```

정책은 분리한다.

- Step 7: Authoring durable handoff Target/Recipe pair.
- Step 8: USER Driving acceptance receipt가 들어 있는 exact current Recipe 하나.

#### E. save ordering / partial recovery

둘 다 dirty일 때 순서는 항상 `Target → Recipe`다. clean package는 rewrite하지 않는다.

`FinalizeAppliedStateAndPersist`에서 Target이 dirty라면 Target을 먼저 저장한 뒤 fresh revalidation을 하고 AppliedState finalize를 수행한다. Target이 이미 clean/persisted라면 Target write 0이다.

Target save 성공 + Recipe save 실패는 Target을 자동 rollback하지 않는다. 같은 lifetime에서는 Recipe dirty를 마저 저장하고, fresh restart에서는 C의 finalize lane으로 복구한다.

#### F. Step 7/8 pre/post benchmark phase 분리와 downstream monotonicity

current benchmark가 **없을 때**:

- Step 7 `Complete`는 strict Saved Handoff ready를 요구한다.
- Step 8 display와 actual benchmark launch도 같은 strict Saved Handoff preflight를 사용한다.
- pre-benchmark Recipe/Target dirty, package missing, AppliedState stale이면 Step 7 Complete/Step 8 benchmark-ready를 모두 허용하지 않는다.

current benchmark가 **fresh Target에 exact binding된 뒤**:

- Target dirty/hash drift 또는 Final Review semantic drift는 benchmark/Step 7 currentness를 무효화한다.
- Recipe dirty를 무조건 허용하지 않는다.
- USER가 `주행 테스트 통과`를 눌러 persistent acceptance receipt가 fresh Target에 exact binding되고 Recipe만 dirty가 된 경우, `AcceptCurrentUserDriving()`의 `RebuildStepStates()`가 Step 7을 다시 미완료로 되돌려 Step 8을 잠그면 안 된다.
- 따라서 exact current benchmark + fresh persistent receipt + semantic Diff 0 + Target clean/persisted + AppliedState exact이면 `PostDrivingReceiptSavePending`으로 분류한다. **Step 7은 이미 성립했던 durable authoring handoff Complete를 유지**하고 Step 8만 `차량 제작 기록 저장 필요`를 표시한다.
- 이 exception은 `HasCurrentUserDrivingAcceptance()`의 legacy local token이나 cached Target hash만으로 성립시키지 않는다. persistent receipt와 live Target fresh identity가 authority다.
- Recipe semantic fingerprint/source/resolved state가 바뀌었거나 Target/AppliedState가 stale이면 post-driving exception을 적용하지 않는다.
- dirty acceptance receipt 상태에서 benchmark 재실행을 허용할 필요는 없다. 기존 exact benchmark를 유지하며 Recipe Save를 먼저 마무리한다.

따라서 stage invariant는 phase-aware하게 정의한다.

```text
Pre-benchmark:
Step 7 Complete => strict saved Target/Recipe handoff ready => benchmark launch saved-state blocker 0

Post-driving acceptance:
exact benchmark + exact persistent acceptance receipt가 만든 downstream Recipe dirty
=> Step 7 Complete를 retroactive 취소하지 않음
=> Step 8 explicit Recipe Save가 durable closure를 소유
```

#### G. guarded Undo

Guarded Undo token은 current Editor lifetime capability로 유지하며 restart persistence를 요구하지 않는다. USER가 Undo를 실행하면 current Source처럼 reverted Target/Recipe가 dirty가 되며, 이후 pair persistence는 같은 package safety/order/mechanics를 사용한다.

Undo가 pre-Apply semantic diff를 복원했다면 Step 7은 다시 `ApplyAndPersist` Ready일 수 있다. Undo 후 무조건 Complete를 강제하지 않는다. partial save failure는 fail-closed하고 auto rollback/retry하지 않는다.

#### H. implementation overlap / C++·BP boundary

이 기능은 Editor authoring/service/VM/Slate 상태 관리이므로 **C++로 구현하며 Blueprint 추가는 0**이다.

구현 직전 fresh diff를 최소 다음 shared files에서 다시 확인한다.

- `CFVehicleBuilderVM.cpp/.h`
- `CFVehicleBuilderTab.cpp/.h`
- `CFVehicleBuilderPresent.cpp/.h`
- `CFVehicleBuilderTypes.h`
- `CFVehicleBuilderReview.cpp`
- `CFVehicleAuthoringService.h`
- `CFVehicleApplyService.cpp` 및 실제 declaration owner
- 관련 focused/affected tests

CF-FQ-046이 common Page Shell/Scroll 및 Presentation seam을 병렬 수정할 수 있으므로 layout architecture를 047에서 재작성하지 않는다.

#### I. design correction completion criteria

- P1-1: single fresh Saved Handoff authority로 해소.
- P1-2: service-owned AppliedState finalize + fresh-restart recovery로 해소.
- P1-3: exact pair package safety + USER approval 이후 TOCTOU revalidation으로 해소.
- P1-4: Step8 pre-benchmark display/launch shared gate + post-driving receipt dirty 예외를 정확히 제한하고, `AcceptCurrentUserDriving()->RebuildStepStates()` 뒤에도 Step7 Complete/Step8 접근이 retroactive lock되지 않도록 phase-aware projection으로 해소.
- P2-1: dirty package only save로 해소.
- P2-2: FinalReview semantic PASS와 Step7 durable Complete 분리로 해소.
- P2-3: save mechanics only 공유 / policy 분리로 해소.

Source-based re-review에서 P0/P1/P2 0이면 implementation Gate를 연다.

### 14.9 VBHAI-P0-07H Source-based Design Re-review — P0 0 / P1 0 / P2 0 PASS

v0.1.22 교정안을 current `CFVehicleBuilderVM.cpp`, `CFVehicleBuilderReview.cpp`, `CFVehicleApplyService.cpp`, `CFVehicleAuthoringService.cpp`, `CFVehicleAIContract.h`와 다시 대조했다.

최종 판정:

```text
P0 = 0
P1 = 0
P2 = 0
Design = PASS
Implementation Gate = OPEN
```

확인 사항:

1. **single fresh identity authority 구현 가능** — `ResolveVehiclePreview()`는 이미 `FCFAuthoringOpResult`에 current Recipe fingerprint, Target hash, SourceSignature, ResolvedDefinitionHash, ResolverContractRevision을 채운다. 따라서 Saved Handoff는 `Review.Operation` + live Target snapshot을 조합하면 되고 별도 Resolver/중복 identity source가 필요 없다.
2. **fresh-restart repair owner가 명확함** — `CFVehicleApplyService.cpp`의 private `BuildAppliedState()`가 authoritative construction을 이미 소유한다. 전용 finalize seam이 이를 재사용하면 VM에 AppliedState 조립 로직을 복제하지 않는다.
3. **기존 Apply 경로와 의미 충돌 없음** — `ApplyBuilderFinalReview()`는 fresh `bCanApply`를 요구하고 existing ApplyInternal은 actual Target Apply transaction을 전제로 하므로 no-diff repair를 분리하는 설계가 current Source와 일치한다.
4. **TOCTOU 및 save scope가 current Step8 writer와 정렬됨** — Step8 `ReadCurrentRecipeSavePreflight()` / `SaveCurrentRecipeAfterDrivingAcceptance()`가 이미 exact selection, live Target fresh hash, safe `/Game` package, mutation 직전 preflight, `SavePackage true + clean`, refresh warning 분리를 사용한다. Step7은 이 mechanics만 공유하고 pair policy를 별도로 둔다.
5. **Step8 self-lock 교정 완료** — current `AcceptCurrentUserDriving()`가 receipt write 뒤 `RebuildStepStates()`를 호출하므로 단순 RecipeDirty→Step7 incomplete 설계는 실제 self-lock을 만들 수 있었다. `PostDrivingReceiptSavePending` phase를 Step7 projection에도 적용해 exact benchmark/receipt/semantic/Target/AppliedState가 current이면 Step7 Complete를 유지하고 Step8 Save가 downstream closure를 소유하도록 교정했다.
6. **cached Target identity 잔여 호출 범위도 implementation scope에 포함** — current `PrepareDrivingBenchmarkLaunch()`, `RefreshDrivingBenchmarkState()`, `HasCurrentUserDrivingAcceptance()`, `AcceptCurrentUserDriving()`, `EvaluateDrivingTestStep()` 일부가 `BuildDrivingTargetIdentity()`를 사용한다. durable/benchmark/acceptance truth에서는 모두 live Target fresh identity 또는 shared Saved Handoff result를 소비하도록 설계가 명시되어 false-ready 재발 경로가 남지 않는다.
7. **046 ownership 충돌 없음** — 047은 Step7/8 state/action meaning과 최소 Presentation 문구만 다루고 common Page Shell/scroll/layout은 CF-FQ-046 owner에 유지한다.
8. **C++/BP boundary 적절** — 순수 Editor authoring/service/VM/Slate 기능이므로 C++ implementation, Blueprint 추가 0이 적절하다.

구현 직전 fresh shared-file diff를 다시 확인한다. 특히 VM/Tab/Present `.cpp/.h`, `CFVehicleBuilderTypes.h`, `CFVehicleBuilderReview.cpp`, Authoring/Apply service owner와 관련 tests의 병렬 dirty를 보호한다.

구현 뒤 필요한 검증은 K의 27개 matrix를 기준으로 focused/affected regression을 구성하되, 의미가 변하지 않은 기존 P0-07E/F build/progress/Recipe Save evidence와 accepted Wagon 2/2 baseline을 이유 없이 반복하지 않는다. C++ semantic 변경 뒤 필요한 official UE 5.8 build와 직접 영향 tests만 수행한다.

### 14.10 VBHAI-P0-07H Implementation + Technical Validation — PASS

P0-07H durable final commit 구현 뒤 fresh-restart 순환을 추가 교정하고 동일 Source로 official build/focused/affected validation을 완료했다.

구현 핵심:

1. `CFVehicleBuilderVM.cpp v1.37.1` / `CFVehicleBuilderVM.h v1.33.1`에서 benchmark cache와 무관한 `IsPostDrivingReceiptCandidate()`를 추가했다.
2. candidate는 semantic Final Review PASS, Target==Resolved, Target clean/persisted, Recipe persisted+dirty, AppliedState current, persistent USER Driving receipt의 exact Target path/hash를 모두 요구한다.
3. fresh restart에서 `EvaluateFinalReviewStep()`은 이 narrow candidate일 때만 기존 benchmark result JSON을 `RefreshDrivingBenchmarkState()`로 fresh current Target에 재검증해 transient cache를 복원한 뒤 final commit preflight를 계산한다.
4. benchmark refresh 실패 시 `PostDrivingReceiptSavePending`을 부여하지 않아 fail-closed하며, 이 경로는 Asset mutation/Save/benchmark process launch를 수행하지 않는다.
5. `PrepareDrivingBenchmarkLaunch()`의 strict pre-benchmark durable gate는 유지되어 post-driving receipt save pending을 새 benchmark 실행 준비로 오인하지 않는다.
6. `CFVehicleAuthoringVMTests.cpp v1.40.0`에 benchmark cache가 비어 있는 fresh-restart 순서, exact persisted result 복원, stale receipt rejection 회귀를 추가했다. `/Temp` legacy Step 8 fixture는 test-only prerequisite projection으로 production durable package gate와 분리했다.

Validation:

```text
Official UE 5.8 Editor Build
- Job: 180ca05c0d314f1ba1473d09048877db
- Exit Code: 0
- Result: PASS

Focused Automation
- Process: 8b8e6699732e4b22b08b86f0fb508f9d
- Result: 8/8 PASS

Affected Automation
- Process: 1179bd5839b84da78ee8ac29176a1ce5
- Result: 10/10 PASS

Final Source Re-review
- P0: 0
- P1: 0
- P2: 0
- Result: PASS
```

검수 결론:

- persistent receipt candidate는 exact current Target/durable handoff에 좁게 한정되어 stale receipt나 unrelated Recipe dirty를 허용하지 않는다.
- persisted benchmark 복원은 read-only transient cache refresh이고 새 benchmark 실행이나 Product write 권한을 만들지 않는다.
- restore 실패는 terminal success로 확대하지 않고 기존 final commit blocker 경로로 되돌아가므로 fail-closed다.
- Step 7 durable completion과 Step 8 post-driving Recipe Save ownership이 분리되어 fresh restart에서도 상호 잠금 순환이 남지 않는다.
- common Page Shell/scroll/layout은 계속 CF-FQ-046 owner이며 047에서 중복 UI architecture를 추가하지 않았다.

이번 P0-07H correction에서 Wagon Product Asset mutation/save, 새 technical benchmark 실행, USER Driving 재실행은 0이다. 기존 병렬 dirty와 USER-authored Wagon 2/2 baseline을 보존했다.

P0-07G USER Re-Acceptance와 fresh persisted closure가 PASS했고 P0-08 Current System Promotion까지 완료되므로 **CF-FQ-047은 Done**이다.

### 14.11 Post-Closure Final Audit Remediation — PASS

CF-FQ-047의 semantic lifecycle은 **Done / Historical + Retained Path** 그대로 유지한다. 종료 후 final audit에서 Step 7 persistence reporting, guarded Undo durability와 mandatory durable transaction regression coverage를 보강했으며, 이 remediation은 Feature 재개나 Product Wagon migration이 아니다.

Current contract delta:

1. exact package save는 raw `SavePackage` success만으로 durable success가 아니다. package clean + persisted confirmation까지 확인해야 하며 확인 실패는 `SaveStateUnconfirmed`로 fail-closed한다.
2. exact durable write가 끝난 뒤 Builder refresh만 실패한 경우에는 persistent write를 되돌리지 않고 `CommittedRefreshWarning`으로 분리한다.
3. guarded Undo에서 실제 Target/Recipe mutation을 수행한 package는 dirty flag 누락 여부와 무관하게 persistence-needed로 취급하고 reverted `Target → Recipe` exact pair를 저장한다.
4. partial save failure는 자동 rollback/retry로 숨기지 않고 실제 partial/dirty truth를 유지하며 Step 7 Complete를 강제하지 않는다.
5. post-closure Current System owner는 `Document/Systems/Vehicles/VehicleBuilder.md v1.4.1`이다. v0.2.0 당시 P0-08의 v1.4.0 승격 기록은 Historical evidence로 유지한다.

Final audit evidence는 이미 완료된 결과를 문서에 동기화한 것이며 **v0.2.1 작성 과정에서 Build/Test/Benchmark/Driving을 다시 실행하지 않았다.**

```text
Official UE 5.8 Editor Build
- Job: ecfa3f8aca094113907a68fab9556ee8
- Exit Code: 0
- Result: PASS

Dedicated P0-07H Durable Final Commit Automation
- Process: ca704e4bca804303be09ecad5b6707a8
- Result: 1/1 PASS

Focused Automation
- Process: a8e3cb6c0301418597f89eccab3ada38
- Result: 9/9 PASS

Affected Automation
- Process: 39880b39186d41b7a637cfeddab30a39
- Result: 10/10 PASS

Mandatory validation matrix
- Result: 27/27 PASS

Final Source/Diff Re-review
- P0: 0
- P1: 0
- P2: 0
- Result: PASS
```

Protection:

- Product Wagon semantic/Asset mutation/save: 0
- Save All: 0
- 새 technical benchmark 실행: 0
- USER Driving 재실행: 0
- current USER-approved Wagon Hardpoint/Mount baseline 2/2와 persistent Driving acceptance는 그대로 보존
- CF-FQ-046 common Step 1~8 Page Shell/scroll/overflow ownership은 변경하지 않음

이 post-closure remediation과 detailed validation evidence까지 대표 Historical Plan이 소유하므로 이전 `plan_repo` evidence-sync blocker를 이유로 CF-FQ-047을 재오픈하거나 검증을 반복하지 않는다.

---

## 15. Changelog

### Maintenance - 2026-09-06

- G5 Physical Move를 완료해 대표 Plan을 `Document/Plan/Archive/VehicleBuilderHardpointIntegrity/VehicleBuilderHardpointIntegrityPlan.md`로 이동했다. post-closure remediation evidence와 USER-approved Wagon 2/2 baseline은 그대로 보존한다.
- Migration: 이전 `Document/Plan/VehicleBuilderHardpointIntegrity/VehicleBuilderHardpointIntegrityPlan.md`는 당시 Historical 기록에서만 유효하며 현재 탐색 경로는 Archive 경로다.

### v0.2.1 - 2026-09-05

- post-closure final audit remediation의 detailed evidence를 동기화하고 Current System owner pointer를 `Document/Systems/Vehicles/VehicleBuilder.md v1.4.1`로 갱신했다. v0.2.0의 P0-08 당시 v1.4.0 승격 기록은 Historical evidence로 유지한다.
- exact package save의 clean + persisted confirmation, `SaveStateUnconfirmed` / `CommittedRefreshWarning` 분리, guarded Undo의 persistence-needed exact `Target → Recipe` pair와 partial-save fail-closed 계약을 14.11에 기록했다.
- 기존 완료 evidence인 official UE 5.8 build PASS, dedicated P0-07H 1/1, focused 9/9, affected 10/10, mandatory matrix 27/27, final Source/Diff re-review P0/P1/P2 0 PASS를 대표 Plan에 보존했다. 이번 문서 동기화에서는 Build/Test/Benchmark/Driving을 다시 실행하지 않았다.
- Product Wagon mutation/save, Save All, 새 benchmark, USER Driving replay는 0이며 USER-approved Wagon 2/2와 persistent Driving acceptance를 그대로 보존한다. CF-FQ-047 lifecycle은 Done / Historical + Retained Path 그대로다.

### v0.2.0 - 2026-09-05

- `VBHAI-P0-07G USER Re-Acceptance`를 USER 최종 **PASS**로 닫았다. P0-07H technical baseline은 official UE 5.8 build `180ca05c0d314f1ba1473d09048877db` Exit 0, focused 8/8, affected 10/10, final source re-review P0/P1/P2 0 PASS를 그대로 보존한다.
- fresh AssetDump persisted readback에서 Wagon Recipe의 USER Driving receipt가 current Target path, DefinitionHash `8aad29d04e008fcd2acb2e6470cd87f1`, accepted benchmark RunId `e5c8153e-4b31-7c8b-60c1-149f13550561`을 보존하고, AppliedState hash도 같은 DefinitionHash와 exact 일치함을 확인했다. current USER-approved Hardpoint/Mount baseline은 2/2다.
- `VBHAI-P0-08 Current System Promotion`으로 Hardpoint Authoring Integrity, Step 5 Physics impact boundary, Step 7 durable final commit, Step 8 fresh readiness/persistent receipt 계약을 `Document/Systems/Vehicles/VehicleBuilder.md v1.4.0`에 승격했다.
- CF-FQ-047을 Done으로 전환하고 대표 Plan은 **Historical + Retained Path**로 현재 경로를 보존한다. G0~G4 완료, G5 physical move는 불필요하여 Deferred다. CF-FQ-046 dependency는 해제되며 common Page Shell/scroll 후속은 046이 계속 소유한다.

### v0.1.23 - 2026-09-05

- P0-07H Step 7 Durable Final Commit 구현 후 fresh restart에서 Step 7이 Step 8 benchmark transient cache보다 먼저 평가되어 persistent USER Driving receipt-only dirty를 놓치는 순환을 발견하고 교정했다. benchmark-independent exact receipt candidate에서만 existing persisted benchmark result를 fresh Target path/hash로 재검증한다.
- `CFVehicleBuilderVM.cpp v1.37.1`, `CFVehicleBuilderVM.h v1.33.1`, `CFVehicleAuthoringVMTests.cpp v1.40.0`을 기준으로 official UE 5.8 Editor build `180ca05c0d314f1ba1473d09048877db` Exit 0, focused `8b8e6699732e4b22b08b86f0fb508f9d` 8/8, affected `1179bd5839b84da78ee8ac29176a1ce5` 10/10 PASS를 확보했다.
- final source re-review는 **P0 0 / P1 0 / P2 0 PASS**다. narrow candidate 외 상태는 fail-closed하고, cache restore는 read-only이며 Product Asset mutation/Save/benchmark launch authority를 추가하지 않는다.
- Wagon Product Asset mutation/save, 새 benchmark, USER Driving 재실행은 0이다. USER-authored accepted Wagon 2/2 baseline과 기존 driving judgement를 보존하며 exact next를 `VBHAI-P0-07G USER Re-Acceptance continuation`으로 전진했다.

### v0.1.22 - 2026-09-04

- P0-07H v0.1.21 설계검수의 P1 4 / P2 3을 교정했다. Step7/Step8 pre-benchmark truth를 live Target UObject fresh hash + fresh Final Review/Resolve 기반 single Saved Handoff authority로 통합하며 cached `BuildDrivingTargetIdentity()`를 durable handoff authority에서 제외한다.
- semantic diff0 recovery를 `PersistDirtyPair`와 `FinalizeAppliedStateAndPersist`로 분리했다. Target-save/Recipe-save partial persistence 뒤 fresh restart에서 AppliedState만 stale한 상태는 VM이 값을 복제하지 않고 low-level Apply Service가 existing private `BuildAppliedState()`를 재사용하는 Recipe-AppliedState-only finalize seam으로 복구한다. Target mutation/save side effect는 금지한다.
- USER confirmation 뒤 exact selection/Recipe/Target/package/hash/fingerprint/source/revision을 각 mutation/save phase 직전에 fresh 재검증하고, dirty package만 실제 저장한다. 둘 다 dirty일 때만 Target→Recipe 순서를 적용한다.
- Step8은 current benchmark가 없을 때 display와 launch가 same Saved Handoff gate를 사용한다. source re-review 중 `AcceptCurrentUserDriving()->RebuildStepStates()`가 post-driving Recipe dirty 때문에 Step7을 retroactive 미완료로 만들어 Step8을 잠글 수 있는 상호작용을 추가 발견해 교정했다. exact benchmark + fresh persistent acceptance receipt + semantic/Target/AppliedState exact 상태는 `PostDrivingReceiptSavePending`으로 분류해 Step7 Complete를 유지하고 Step8 Recipe Save가 closure를 소유한다.
- generic Final Review `bCanCompleteFinalReview`은 semantic no-diff fact로 보존하고 Step7 durable Complete와 USER 문구를 분리한다. Step7/Step8은 Save mechanics만 공유하고 승인 조건·write scope·outcome policy는 분리한다.
- C++/BP/Asset/Build/Automation/Benchmark/USER Driving 실행은 0이다. accepted Wagon 2/2 baseline, P0-07E/F technical evidence와 CF-FQ-046 common Page Shell ownership을 보존한다.
- correction 직후 source-based re-review에서 `AcceptCurrentUserDriving()->RebuildStepStates()`가 downstream receipt dirty로 Step7을 retroactive 미완료 처리할 수 있는 상호작용을 추가 발견했다. `PostDrivingReceiptSavePending` phase, Step7 completion monotonicity, fresh acceptance/benchmark identity 사용을 추가 교정한 뒤 재검수했다.
- 최종 source-based Design Re-review는 **P0 0 / P1 0 / P2 0 PASS**다. `FCFAuthoringOpResult`의 existing fresh identity fields와 Apply Service `BuildAppliedState()` authority로 설계가 current Source에 구현 가능함을 확인했고 exact next를 `VBHAI-P0-07H Step 7 Durable Final Commit Implementation`으로 전진했다.

### v0.1.21 - 2026-09-04

- P0-07H v0.1.20 durable final commit 설계를 current FinalReview/Apply/Undo/Step8/Recipe Save Source와 재검수했다. 결과는 **P0 0 / P1 4 / P2 3 / Implementation HOLD**다.
- P1-1은 Step7 durable completion과 Step8 benchmark launch가 cached/fresh Target identity를 따로 쓰지 않고 live Target fresh DefinitionHash 기반 단일 saved-handoff preflight를 공유하도록 요구한다.
- P1-2는 `Target save 성공 → Recipe save 실패 → fresh restart`에서 disk Target과 Recipe AppliedState가 분리될 수 있는데 단순 PersistCurrent/기존 FinalReview Apply로는 안전 복구가 불가능한 공백이다. AppliedState construction authority를 VM에 복제하지 않고 Apply service-owned repair/finalize lane을 설계해야 한다.
- P1-3은 exact Recipe/Target selection/path/package safety와 USER approval 이후 fresh TOCTOU revalidation을 두-package writer에 강제하는 사항이다. P1-4는 current Step8 화면이 Target dirty만 보지만 benchmark launch는 Recipe+Target clean을 요구하는 기존 mismatch를 shared pre-benchmark pair gate로 통일하는 사항이다.
- P2는 PersistCurrent에서 dirty package만 save, generic Final Review `bCanCompleteFinalReview`를 semantic-only fact로 유지하면서 downstream-ready 문구를 분리, Step7/Step8의 save mechanics만 private helper로 공유하고 policy는 분리하는 3건이다.
- 이번 검수에서 C++/PS1/Asset/Build/Automation/Benchmark/USER Driving 실행은 0이다. current 2/2 accepted baseline과 P0-07E/F evidence는 보존하며 live UAT 3/3은 Product baseline으로 승격하지 않는다.
- exact next는 `VBHAI-P0-07H P1/P2 Design Correction + Re-review`다.

### v0.1.20 - 2026-09-04

- P0-07E Step 8 progress/explicit Recipe Save 구현과 P0-07F Technical Validation의 실제 최신 증거를 Current checkpoint에 반영했다. final official build `9d789a07245a4bdda36bf81eba0fb138` PASS, Step8 focused `8c8c8c8b78b84225bf893420d0664efb` 1/1 PASS, actual progress transport `9ce95e023e4445dda48028d652656971` success를 보존한다.
- P0-07G USER UAT에서 USER가 테스트용 Socket/Hardpoint/Mount를 추가해 Step 7까지 완료한 뒤 Step 8 기술 측정이 `VehicleData 미저장 변경`으로 막히는 stage-contract defect를 재현했다. current Source도 `EvaluateFinalReviewStep()`은 semantic Diff 0만으로 Complete를 만들고 `PrepareDrivingBenchmarkLaunch()`는 별도로 Recipe/Target saved-state를 요구해 모순이 확인됐다.
- USER 지적에 따라 `Step 8에 주행 측정 준비 저장 버튼 추가` 방향을 폐기했다. 정확한 결함을 **Step 7 durable completion 누락**으로 재분류했다.
- P0-07H 설계에서 `Step7 Complete => semantic Complete + exact Target/Recipe persisted/clean + AppliedState/current Target hash exact` invariant를 고정했다. existing Step 7 primary action이 ApplyAndPersist 또는 PersistCurrent를 수행하며 Step 8에 새 pre-benchmark Save 버튼을 만들지 않는다.
- Step 7 durable handoff save set은 current Target VehicleData + current Recipe 두 package로 제한하고 Target→Recipe 순서를 고정했다. partial save, refresh warning, guarded Undo도 durable pair boundary를 따르며 Save All/StaticMesh/Profile/Evidence/Catalog/unrelated Asset 저장은 금지한다.
- 이번 요청은 **문서 설계 교정만** 수행했다. C++/PS1/Asset mutation, Build, Automation, Benchmark/USER Driving replay는 0이다. UAT 화면의 3/3 장착 구성은 test state이며 USER 승인 없이 accepted Product baseline 2/2를 대체하지 않는다.
- exact next는 `VBHAI-P0-07H Step 7 Durable Final Commit Design Re-review`이며 P0/P1 0 확인 전 implementation HOLD다.

### v0.1.19 - 2026-09-04

- v0.1.18 implementation-entry mid-review의 **P1 4 / P2 3**을 current Source와 exact process/save authority에 맞춰 전부 교정했다. 교정 후 source-based 재검수 결과는 **P0 0 / P1 0 / P2 0 PASS**이며 implementation Gate를 다시 열었다.
- progress identity는 actual `Builder → PowerShell → UnrealEditor` chain을 유지한다. VM이 RunId를 wrapper에 전달하고 `RunBuilderBench.ps1`이 canonical progress path를 소유해 exact RunId/path를 UnrealEditor command line까지 전달한다. runtime writer는 ReEditor DTO를 참조하지 않는 private schema writer다.
- 7단계 progress writer는 `LastAttemptedPhaseGroup`을 write 전에 갱신해 same-group retry storm을 막고 run당 최대 7 write attempt로 제한한다. Tab은 process-running 동안 0.5초 bounded polling을 수행하며 PowerShell terminal 순간 progress authority를 종료하고 final result JSON + exit code로 전환한다.
- explicit Save는 current persistent receipt가 fresh Target에 exact binding된 current Recipe package 하나에만 허용한다. USER에게 이 action이 receipt 한 필드가 아니라 Recipe package의 **미저장 변경 전체**를 저장함을 명시하고 다른 VehicleData/StaticMesh/Catalog Save는 금지한다.
- immediate Save success는 `SavePackage true + package clean`으로 닫고 package existence/manual dirty clear를 durable evidence로 사용하지 않는다. fresh persisted receipt readback은 P0-07G에서 별도 확인한다. Save success + refresh failure는 `SavedRefreshWarning`으로 보존한다.
- current 2/2 USER PASS receipt가 아직 dirty memory에 있을 수 있으므로 first build/restart/binary replacement 전에 exact Recipe 보존 Gate를 유지한다. 자동 Editor 종료/Save All은 금지한다.
- 이번 교정에서 Source/C++/PS1/Product Asset mutation, Build, Automation, Benchmark/USER Driving replay는 0이다. current 2/2 USER Driving PASS와 P0-07A/B/C evidence는 보존하며 exact next는 `VBHAI-P0-07E Step 8 Progress + Explicit Recipe Save Implementation`이다.

### v0.1.18 - 2026-09-04

- P0-07E 구현 착수 전 current process/save source를 재대조한 중간검수에서 최초 Design PASS를 철회하고 **P0 0 / P1 4 / P2 3 / Implementation HOLD**로 교정했다.
- P1은 실제 `Builder → PowerShell → UnrealEditor` chain에 RunId/progress path 전달 계약 누락, `UPackage::SavePackage`가 receipt-only가 아니라 Recipe package 전체를 저장한다는 USER write-scope 누락, package existence가 새 receipt durable write를 증명하지 못하는 검증 경계, 현재 dirty USER PASS receipt를 first lifecycle/build replacement보다 먼저 보호해야 하는 operational Gate 4건이다.
- P2는 progress write 실패의 per-frame retry storm 방지, terminal result authority가 same-run stale progress를 즉시 덮어야 하는 규칙, runtime `CarFight_Re`에서 Editor `CarFight_ReEditor` DTO를 역참조하지 않는 module dependency 경계 3건이다.
- current 2/2 USER Driving PASS와 P0-07A/B/C Technical PASS, 기존 final build/focused 8/8/affected 10/10/Step5 1/1은 그대로 보존한다. Source/Asset/Build/Automation/Benchmark replay는 이번 검수에서 0이다.
- exact next를 `VBHAI-P0-07E Design Correction + Re-review`로 변경했다. P0/P1 0 재확인 전 source implementation을 시작하지 않는다.

### v0.1.17 - 2026-09-04

- P0-07D USER UAT에서 current USER-authored Wagon 2/2 대상으로 fresh 기술 주행 측정 → PIE 적용 → USER direct Driving → `주행 테스트 통과`까지 실제 완료했다. current 2/2 driving judgement 자체는 USER PASS로 보존한다.
- acceptance success 뒤 Recipe가 dirty가 되어 저장이 필요했지만 Step 8에 exact Recipe save action이 없어 flow가 끊겼고, benchmark child process 실행 중에도 terminal까지 단계/경과 정보가 없어 대기 UX가 불투명한 두 추가 blocker를 확인했다. P0-07D 전체 closure는 interrupted로 유지한다.
- P0-07E correction design을 확정했다. benchmark는 exact RunId progress sidecar + 7단계 coarse progress + elapsed를 사용하고 가짜 ETA를 금지한다. Save는 USER click으로 exact current Recipe package 하나만 저장하며 Save All/Target/StaticMesh/Catalog 자동 저장을 금지한다.
- Save persistence 성공 뒤 refresh failure는 warning으로 분리하고, progress sidecar write/parse 실패는 benchmark terminal authority를 훼손하지 않는다. 046 common Page Shell/Scroll owner는 그대로 유지한다.
- 설계 재검수 결과는 **P0 0 / P1 0 / P2 0 PASS**다. 구현/Build/Automation은 아직 0이며 exact next는 `VBHAI-P0-07E Step 8 Progress + Explicit Recipe Save Implementation`이다. 현재 Editor에 dirty USER Driving receipt가 남아 있을 수 있으므로 first build/restart 전 보존을 확인하고 자동 종료/Save All을 하지 않는다.

### v0.1.16 - 2026-09-04

- `VBHAI-P0-07B Driving Apply Readiness`를 구현했다. `ECFVehicleDrivingApplyBlocker + FCFVehicleDrivingApplyPreflight`가 Recipe/Target load, dirty, disk persistence, AppliedState, validated benchmark identity를 stable single authority로 판정하고 Step 8 버튼 enable과 `ApplySelectedVehicleToActivePIE()` production guard가 같은 preflight를 사용한다. 버튼 근처에는 raw hash/path를 숨긴 한국어 blocker/next-action을 표시하며 PIE/Pawn은 click-time volatile guard로 분리했다.
- 구현 후 source re-review에서 P1 1건을 발견했다. 최초 preflight가 Authoring ViewModel의 cached DefinitionHash를 재사용해 외부에서 VehicleData를 저장한 뒤 Builder refresh 전 false-ready가 될 수 있었으므로, live Target UObject의 fresh immutable Definition Snapshot에서 path/hash를 직접 만드는 `BuildFreshDrivingTargetIdentity()`로 교정했다. Builder refresh 없이 semantic field를 변경해도 fresh hash가 즉시 달라지는 회귀를 추가했다.
- 교정본 official UE 5.8 Editor build Job `69adc7a695df4da497e46757d277de03`은 Exit 0 PASS다. `Tools/RunHardpointIntegrityTests.ps1` process `2ff9398c398c421bb3fa65f552dab13f` **8/8 PASS**, affected process `a73d2045eeeb4c2f9a455a29fd7b12b1` **10/10 PASS**, exact Step 5 Physics process `b36658d3619548268d7cff0bf329e0ed` **1/1 PASS**다.
- `VBHAI-P0-07C 046×047 Integration / Technical Validation`도 PASS로 닫았다. 046 common Page Shell/Scroll ownership은 재작성하지 않았고 최신 VM/Tab/Present 위에 047 Step 8 readiness만 최소 통합했다. 최종 source re-review는 **P0 0 / P1 0**이다.
- 이번 07B/07C에서 Wagon Product Asset mutation/save, Technical Benchmark 재실행, USER Driving 재실행은 0이다. current accepted Wagon semantic baseline은 2/2를 유지하며 historical P0-06 1/1 Benchmark/USER Driving은 historical evidence로만 보존한다. **current Driving freshness는 아직 pending**이며 exact next는 `VBHAI-P0-07D USER Re-Acceptance`다.

### v0.1.15 - 2026-09-04

- USER가 P0-07 UAT에서 두 번째 Hardpoint/Mount를 테스트 목적으로 직접 추가했으며, 정상 생성·사용 가능한 상태라면 실제 Product에 그대로 유지하겠다고 명시 승인했다. 이에 v0.1.14의 `unknown Product Baseline Drift` 분류를 취소하고 current Wagon **2/2 USER-authored semantic baseline**으로 전진했다.
- historical exact1 고정 fixture를 확장 가능한 semantic integrity fixture로 교정했다. Recipe/VehicleData Hardpoint/Mount count alignment, 모든 Recipe Intent의 Target exact 1회 대응, canonical `Top_01 → HP_Top_01`과 `Mount_Top_01 → Top_01` 중복 금지, legacy RoofTurret injection 금지, Recipe AppliedState hash == current Target DefinitionHash를 검증한다. 세 번째/네 번째 장착점을 정상 추가해도 개수 자체로 false failure하지 않는다.
- 테스트 교정 중 사용한 `TArray::CountByPredicate`가 UE TArray API에 없어 첫 official build `da0832dd52bc49ceb2a74589be0d357d`가 test compile error로 실패했다. explicit range loop로 교정한 뒤 official UE 5.8 Editor build `f74cc2bb14f546f2a84dd130fc44fcbd` Exit 0 PASS했다.
- current persisted 2/2에서 `CarFight.DataAuthoring.CF_FQ_047.P0_06.ActualWagon.PostLoadMountIntegrity` process `b357b6ef4a7c40c28ecfae33d7aa5b31`은 **1/1 PASS / failure 0**이다. Wagon Product Asset 자체 mutation/save는 이번 교정에서 0이며 exact1 recovery commandlet은 실행하지 않았다.
- P0-07A Technical PASS는 유지한다. current 2/2는 historical 1/1 이후 semantic Definition 변경이므로 P0-06 Benchmark/USER Driving PASS는 historical evidence로 보존하되 current freshness로 재사용하지 않는다. exact next를 `VBHAI-P0-07B Driving Apply Readiness`로 전진해 현재 버튼 blocker/freshness를 typed preflight로 설명하게 한다. Benchmark/USER Driving 재실행은 07B가 필요한 current action을 명확히 한 뒤에만 수행한다.

### v0.1.14 - 2026-09-04

- `VBHAI-P0-07A Physics Impact Boundary` 구현과 교정 검증을 **Technical PASS / source re-review P0 0 / P1 0**으로 닫았다. generic `PreviewBuilderProfiles` / `CommitBuilderProfiles`의 full resolved DefinitionHash exact transaction 의미는 유지하고, Step 5만 별도 Physics provenance + typed structural compatibility를 소비한다.
- 최초 affected 검증에서 정상 `Step 5 승인 완료 → Step 7 Apply 대기` 상태를 stale로 오판하는 회귀를 발견했다. 승인 receipt hash가 current prospective resolved hash와 같으면 Current Target hash가 아직 이전 값이어도 `Exact / Complete`로 유지하도록 교정했고, VM도 이 상태에서 pending diff를 불필요하게 읽지 않고 즉시 Exact로 닫도록 맞췄다. `Receipt != prospective`일 때만 current Target baseline + pending diff를 사용해 Hardpoint/Mount-only structural equivalence를 판정한다.
- 최종 official UE 5.8 incremental build Job `77189256e0104ece8e809b37190d7522`는 Exit 0 PASS다. 최종 VM 변경 뒤 `BuilderStep7FinalReview` 1/1, `BuilderStep8Driving` 1/1, CF-FQ-046 Presentation 3/3이 PASS했다. CF-FQ-043 affected 5/5도 앞선 동일 07A 변경 범위에서 PASS했다. pure `P0_07A.PhysicsReceiptCompatibility`는 evaluator 교정 뒤 1/1 PASS였고, 마지막 VM-only 변경 후 재호출 1회는 test assertion failure가 아니라 `SUCCESS_COUNT=0 / FAILURE_COUNT=0` runner no-run이므로 PASS evidence를 무효화하지 않는다.
- focused Product fixture `CF_FQ_047.P0_06.ActualWagon.PostLoadMountIntegrity`는 **현재 persisted Wagon이 Recipe HardpointIntents=2 / MountIntents=2, VehicleData HardpointSlots=2 / MountProfiles=2**라서 historical intended exact1 fixture와 충돌해 FAIL했다. current 문서에서 2/2를 의도 상태로 승격한 근거를 찾지 못했으므로 Product Baseline Drift로 분리한다.
- `Tools/RunWagonMountRecovery.ps1`은 exact1로 실제 Recipe+Target을 복구·저장하는 destructive Product recovery entry이므로 2/2의 의도를 모르는 상태에서 자동 실행하지 않는다. Wagon Product Asset mutation/save는 이번 07A closure에서 0이다.
- P0-06 당시 DefinitionHash `8d780eb07ee7ab4672233fd3d15a0db4`, Benchmark RunId `a9e28515-05ea-4cf9-bc0a-80a49adfc018`, USER Driving PASS는 historical evidence로 보존한다. 다만 current persisted Product가 2/2로 drift했으므로 **현재 Driving receipt freshness는 미확정**이며, baseline을 먼저 확정하기 전 Benchmark/USER Driving을 반복하지 않는다.
- exact next는 `Wagon Product Baseline Drift Triage (current 2/2가 의도 변경인지 UAT/병렬 residue인지 확정)`이다. intended 1/1 복원 또는 2/2 신규 의도 승격과 그에 따른 current DefinitionHash/Driving freshness를 닫은 뒤에만 `VBHAI-P0-07B Driving Apply Readiness`로 진행한다.

### v0.1.13 - 2026-09-04

- `VBHAI-P0-07A Physics Impact Boundary` source implementation을 완료했다. generic `ReceiptMatchesFreshProposal()`의 full resolved DefinitionHash exact transaction 의미는 보존하고, resolved DefinitionHash만 제외한 `ReceiptMatchesFreshPhysicsProvenance()` + `IsBuilderPhysicsReceiptProvenanceCurrent()` read-only seam을 추가했다.
- `ECFBuilderPhysicsReceiptCompatibility(Exact / PhysicsEquivalentStructuralDrift / PhysicsStale / Blocked)`와 `ECFBuilderPhysicsStructuralDriftBoundary(Equivalent / NotEquivalent / Blocked)`를 추가했다. HardpointSlots/MountProfiles-only pending diff는 Step 5 Complete를 유지하며 receipt 자동 rewrite/Save는 하지 않는다. 기존 `ValidatePhysicsReceiptRefreshBoundary()`도 동일 typed structural evaluator의 bool projection으로 전환했다.
- `FCFVehicleBuilderVM::EvaluateCurrentPhysicsReceiptCompatibility()`가 fresh Profile preview + Physics provenance + current Target/prospective Definition identity + shared `ReadPendingDiff()`를 결합한다. Step 5 persistent receipt branch는 generic Preview `NoChange`를 Step authority로 재해석하지 않고 typed compatibility를 소비한다. Step 6은 기존 Step 5 Complete projection을 그대로 사용하고 Step 8 exact DefinitionHash 계약은 변경하지 않았다.
- focused Automation `CarFight.DataAuthoring.CF_FQ_047.P0_07A.PhysicsReceiptCompatibility`를 추가해 exact, Hardpoint-only, Mount-only, combined, provenance stale, Target drift, unrelated drift, missing diff/identity fail-closed matrix를 고정했다. `Tools/RunHardpointIntegrityTests.ps1` focused 목록에도 추가했다.
- source-only 재검수는 **P0 0 / P1 0**이다. official UE 5.8 Editor build Job `04bdc09127a4466285f3047ae212512f`에서 변경 C++ (`CFVehicleBuilderHardpointIntegrity.cpp`, tests, `CFVehicleBuilderCommit.cpp`, `CFVehicleBuilderVM.cpp`) compile은 모두 PASS했으나, 실행 중인 `UnrealEditor.exe`가 `UnrealEditor-CarFight_Re.dll` 및 `UnrealEditor-CarFight_ReEditor.dll`을 점유해 final link가 LNK1104 / Exit 6으로 종료됐다. 이는 코드 compile failure가 아니며 Editor를 자동 종료하지 않는다.
- 따라서 P0-07A Technical PASS는 아직 선언하지 않는다. USER가 현재 Editor 작업을 필요한 경우 저장한 뒤 수동 종료하면 official incremental build → focused → affected regression → final source re-review를 이어간다. P0-06 Wagon 1/1, GrossMass 2350kg, DefinitionHash/Benchmark/USER Driving PASS는 semantic Definition 변경이 없어 반복하지 않는다.

### v0.1.12 - 2026-09-04

- v0.1.11 설계검수의 **P1 4 / P2 3**을 전부 교정하고 current Source 계약에 다시 대조했다. 교정 후 재검수 결과는 **P0 0 / P1 0 / P2 0 PASS**다. 구현은 아직 시작하지 않았다.
- P1-1 교정: Step 5 Physics compatibility를 generic `FCFBuilderProfileCommitPreview`에서 분리했다. existing `ReceiptMatchesFreshProposal()`의 full exact transaction 의미는 유지하고, resolved DefinitionHash를 제외한 모든 Physics provenance를 확인하는 service-owned comparator를 분해·재사용한 별도 read-only compatibility evaluator가 Step 5를 소유하도록 확정했다.
- P1-2 교정: Hardpoint/Mount structural equivalence는 existing `ValidatePhysicsReceiptRefreshBoundary()` 의미를 단일 typed structural-boundary owner로 승격해 재사용한다. receipt baseline=current Target, nonempty pending diff, HardpointSlots/MountProfiles-only 조건을 exact 유지하고 bool legacy API는 typed result의 projection으로 보존한다.
- P1-3 교정: Step 8은 VM stable data preflight와 Tab-owned `bDrivingBenchmarkRunning/DrivingBenchmarkProcess` UI orchestration을 분리했다. `WrongStep/BenchmarkRunning`을 VM blocker에 넣지 않고 Tab에서 합성하며, actual Apply는 동일 VM stable preflight 후 PIE/Controller/Pawn volatile guard만 수행한다.
- P1-4 교정: Driving Apply stable preflight는 Recipe/Target load, dirty=false, package disk existence, fresh Target path/hash, benchmark path/hash, Recipe AppliedState exact를 요구한다. cached bool만 authority로 사용하지 않되 Slate `IsEnabled`마다 benchmark JSON을 다시 읽지 않고 existing explicit benchmark refresh boundary의 validated result object를 사용한다.
- P2 교정: `CanApply()`를 blocker-derived single truth로 고정하고, 046×047 overlap preflight를 VM/Tab/실제 `CFVehicleBuilderPresent.*` 6파일로 명시했으며, CF-FQ-045를 Done/current DataManagement owner 보호 상태로 교정했다.
- Correction Design Gate를 PASS로 닫고 exact next를 `VBHAI-P0-07A Physics Impact Boundary Implementation`으로 전진했다. P0-06 Wagon 1/1, GrossMass 2350kg, DefinitionHash/Benchmark/USER Driving PASS는 semantic drift가 없어 반복하지 않는다.

### v0.1.11 - 2026-09-04

- v0.1.10 P0-07 correction design을 current `PreviewBuilderProfiles/CommitBuilderProfiles`, `PrepareCurrentPhysicsReceiptRefresh`, `ValidatePhysicsReceiptRefreshBoundary`, Step5 evaluator와 Step8 benchmark/apply source 및 CF-FQ-046 page-shell ownership과 재대조했다. 설계검수 결과는 **P0 0 / P1 4 / P2 3**이다.
- P1-1은 Step5 compatibility를 generic `FCFBuilderProfileCommitPreview.Operation.Status/NoChange`에 섞지 않고 별도 read-only evaluator로 분리하며, full receipt provenance exact comparator를 Commit service authority에서 재사용하도록 요구한다.
- P1-2는 `PhysicsEquivalentStructuralDrift`가 existing `ValidatePhysicsReceiptRefreshBoundary`의 receipt baseline=current Target + nonempty Hardpoint/Mount-only pending diff 경계를 exact 재사용하도록 고정한다.
- P1-3은 Tab-owned `bDrivingBenchmarkRunning/DrivingBenchmarkProcess`를 VM readiness로 이중 소유하지 않고 VM stable data preflight + Tab UI/process orchestration 합성으로 분리한다.
- P1-4는 Driving Apply saved-state를 dirty=false뿐 아니라 Recipe/Target disk existence, fresh Target path/hash, Recipe AppliedState exact까지 검증하고 cached benchmark bool만 신뢰하지 않도록 요구한다.
- P2는 derived CanApply single truth, 046×047 shared-file preflight 범위 명시, stale CF-FQ-045 Ready 문서 표현 교정 3건이다. 구현은 아직 시작하지 않으며 next는 `VBHAI-P0-07 Design Correction`이다.

### v0.1.10 - 2026-09-04

- `VBHAI-P0-07 USER Acceptance`를 USER 피드백 기준 **FAIL**로 전환했다. P0-06 USER Driving PASS와 Wagon persisted 1/1/DefinitionHash/Benchmark는 그대로 보존한다.
- Step 5/Step 8의 긴 접힘 콘텐츠 문제는 서로 다른 결함이 아니라 하나의 공통 Page shell/Scroll 결함으로 재분류했다. 이 문제는 CF-FQ-046의 Step1~8 공통 `Page Height / Scroll / Overflow` 계약이 소유하며 047에서 개별 ScrollBox 땜질을 금지한다.
- P1 RCA 1: Physics receipt exact-match가 full `ProspectiveResolvedDefinitionHash`까지 요구해 Hardpoint/Mount-only structural drift도 Step 5를 stale 처리한다. 기존 receipt provenance는 보존하되 Step validity를 `Exact / PhysicsEquivalentStructuralDrift / PhysicsStale / Blocked` typed compatibility로 분리하는 P0-07A 설계를 고정했다.
- P1 RCA 2: Step 8 Apply 버튼은 현재 benchmark/step bool로 enable되지만 saved target/dirty/hash/PIE/Pawn guard는 click-time 함수에 분산되어 있고 disabled reason projection이 없다. 단일 read-only `DrivingApplyState`를 UI enable과 action preflight가 공유하고 USER에게 exact reason/recovery를 노출하는 P0-07B 설계를 고정했다.
- Step 8 UAT 당시 exact disabled root cause는 live bool evidence 없이 추측하지 않는다. 당시 화면에 benchmark/USER acceptance binding 요약이 있었으므로 `benchmark가 없었다`고 단정하지 않으며, correction은 어떤 blocker든 같은 typed readiness에서 설명 가능하게 만든다.
- 구현 순서는 047A service/VM → CF-FQ-046 common page shell → fresh shared-file preflight → 047B Step8 presentation integration → official build/focused+affected/re-review → P0-07D USER Re-Acceptance로 고정했다.

### v0.1.9 - 2026-09-04

- post-P0-06 중간검수 P1 2건을 교정했다. Existing Socket adoption과 Physics/Profile commit 모두 persistent mutation 성공 뒤 후속 ViewModel refresh 실패를 전체 operation failure로 뒤집지 않고 transient `LastPostCommitRefreshWarning`으로 분리한다.
- USER surface는 실제 write가 성공한 경우 `변경 완료 + 화면 상태 다시 확인 필요`로 표시하며, writer/rollback/Save authority와 기존 함수 시그니처는 변경하지 않았다. Product Asset/VehicleData 신규 mutation과 자동 저장은 0이다.
- 공식 UE 5.8 Editor Development build job `102a9d519fda490297d628d0cfb3ae97` Exit 0 PASS. `Tools/RunHardpointIntegrityTests.ps1` focused process `af0d85ddf86b4bdfaac0f0a501bc7e7c`에서 InventoryClassification / SemanticReadback / PresentationReadback / MountLegacyFallback / PhysicsReceiptRefreshBoundary / 신규 `MidReview.PostCommitRefreshOutcome` / ActualWagon.PostLoadMountIntegrity를 **7/7 PASS**했다.
- 재검수 결과는 **P0 0 / P1 0 / P2 0 PASS**다. P0-07 HOLD를 해제하고 exact next를 `VBHAI-P0-07 USER Acceptance READY`로 전진한다.
- P0-07 9개 항목 중 existing Socket 중복 생성 없음, Step 6 completion guard, Step 7/8 runtime semantic readback, RuntimeApply Mount 표시, DefinitionHash drift 시 Driving stale, Product persisted 1/1 recovery는 기존 technical/P0-06 evidence로 보존한다. 이번 USER gate는 주행 재실행이 아니라 unbound Socket/Hardpoint/Mount 관계를 사용자 관점에서 이해·확인하는 UX acceptance다.
- Wagon Hardpoint/Mount 1/1, GrossMass 2350kg, DefinitionHash `8d780eb07ee7ab4672233fd3d15a0db4`, Benchmark RunId `a9e28515-05ea-4cf9-bc0a-80a49adfc018`, USER Driving Re-Acceptance PASS는 semantic drift가 없어 반복하지 않는다.

### v0.1.8 - 2026-09-04

- P0-06 closure 뒤 P0-07 진입 전 current Source/Plan/CF-FQ-046 Presentation contract를 중간검수했다. 결과는 **P0 0 / P1 2 / 추가 P2 0**이며 P0-07 USER Acceptance는 두 P1 교정·재검수 전 HOLD한다.
- **P1-1 Existing Socket adoption false-negative:** `AdoptExistingStandardHardpointSocket()`은 `UpsertHardpointIntent()`가 Recipe typed mutation을 이미 성공시킨 뒤 `RefreshCurrentState()`가 실패하면 전체 operation을 false로 반환한다. 실제 Recipe 변경은 성공했는데 USER surface가 연결 실패로 오인할 수 있으므로 mutation success와 post-mutation refresh failure를 분리해야 한다. 성공한 Recipe write를 rollback하지 않는 한 재시도·상태 안내가 실제 truth와 일치해야 한다.
- **P1-2 Physics/Profile commit false-negative:** `ExecutePreparedPhysicsProposal()`은 `CommitBuilderProfiles()`가 receipt/profile commit과 exact readback까지 성공한 뒤 `RefreshPreview()` 또는 `RefreshReferenceEvidenceState()`가 실패하면 false를 반환한다. 특히 `현재 물리 설정 재검증`은 receipt-only Recipe mutation이 이미 성공했는데 UI가 재검증 실패라고 표시할 수 있다. commit truth와 post-commit UI/reference refresh warning을 별도 결과로 분리해야 한다.
- 두 P1 모두 commit service 자체의 transaction/rollback 결함은 아니다. persistent mutation 성공 이후의 ViewModel refresh/reporting 경계 문제이며, correction에는 해당 성공+refresh-failure regression을 추가한다. CF-FQ-046 terminology dictionary와 current Hardpoint/Socket user-facing 표현은 계약상 허용되어 별도 결함으로 잡지 않았다.
- P0-06 USER Driving Re-Acceptance PASS, Wagon DefinitionHash `8d780eb07ee7ab4672233fd3d15a0db4`, Benchmark RunId `a9e28515-05ea-4cf9-bc0a-80a49adfc018`, GrossMass 2350kg와 persisted Hardpoint/Mount 1/1은 이번 read-only 검수로 변경되지 않았다. benchmark와 USER Driving을 재실행하지 않는다.

### v0.1.7 - 2026-09-04

- `VBHAI-P0-06` USER Driving Re-Acceptance를 PASS로 닫았다. USER가 current saved Wagon을 PIE에 적용해 직접 주행한 뒤 명시 PASS했고, `주행 테스트 통과` 실행 후 live Step 8이 `[완료]`로 전환되며 Recipe 1개가 dirty가 되는 production acceptance 경계를 확인했다.
- USER explicit Save 뒤 USER가 직접 Editor를 종료/재기동했고 fresh Editor에서 `모두 저장됨`을 확인했다. acceptance token은 RecipeId `05F69DD34990A868DEFFD29C1AF5B2F9`, current Target DefinitionHash `8d780eb07ee7ab4672233fd3d15a0db4`, Benchmark RunId `a9e28515-05ea-4cf9-bc0a-80a49adfc018` exact match다.
- current tool policy는 native nested Recipe receipt field의 direct projection을 제공하지 않으므로 AssetDump가 3필드를 직접 읽었다고 기록하지 않는다. 대신 persistent receipt와 host-local token을 동일 success path에서 함께 쓰는 production code, Step 8 Complete/dirty transition, USER explicit Save, fresh all-saved readback과 exact token identity를 종합 closure evidence로 고정했다.
- DefinitionHash, GrossMass 2350kg, existing fixed-60Hz Benchmark는 변경하지 않았다. exact next Gate는 `VBHAI-P0-07 USER Acceptance`이며 Feature는 Ready 유지한다.

### v0.1.6 - 2026-09-04

- 중간검수 `P0 0 / P1 1 / P2 1`의 P1을 교정했다. 기존 active weapon이 candidate에서 사라지고 target도 비무장인 경우 downstream의 `BuildSnapshotRuntimeInput()` 거부 여부에 기대지 않고 `ResolveCandidateActiveMount()`가 `RejectNonWeaponTarget`을 반환해 Fitting Prepare 전에 `ValidationFailed`로 종료한다.
- pure policy는 `PreserveCurrent / HandoffToTarget / NoActiveWeapon / RejectNonWeaponTarget` resolution을 명시하며, 원래 active weapon이 없던 scanner-only/non-weapon candidate는 불필요하게 차단하지 않는다.
- corrected official Editor build `85f2cb2bd8de460c91d0d3ccaea8014f` Exit 0 / DLL link PASS, RuntimeApply affected process `e1dfea68d9b94c36bcf96e8f795f1820`에서 16/16 PASS / failure 0을 재확인했다. `ActiveMountHandoffPolicy`와 `WagonMountEquipmentPIE`도 동일 실행에서 PASS했다.
- P2를 교정해 USER PASS 후 fresh persisted receipt readback은 current `TargetDefinitionHash`뿐 아니라 PASS 당시 `AcceptedBenchmarkRunId=a9e28515-05ea-4cf9-bc0a-80a49adfc018`도 함께 확인하도록 고정했다. RunId는 same DefinitionHash USER PASS validity를 무효화하는 authority가 아니라 provenance다.
- Wagon Asset/GrossMass/DefinitionHash/기존 Benchmark에는 mutation을 가하지 않았다. `MaximumGrossMassKg=2350`과 HeavyCannon 2356kg `GrossMassExceeded` 계약은 그대로 보존한다.

### v0.1.5 - 2026-09-03

- Wagon Product Recovery의 persisted Recipe/VehicleData를 fresh readback해 `UseHardpoints / Hardpoint 1 / Mount 1`, current applied DefinitionHash `8d780eb07ee7ab4672233fd3d15a0db4`, old Driving receipt hash `0e5b48e8dcd39deba441da9237218be6` stale를 재확인했다.
- actual RuntimeApply에서 `Mount_Top_01` exact option 표시/선택을 PASS했다. HeavyCannon/RocketLauncher는 각각 TurretMount 350kg + Weapon 120kg이어서 Wagon 1886kg 기준 총 2356kg로 `MaximumGrossMassKg=2350`을 6kg 초과한다. 이를 Hardpoint 결함으로 보지 않고 정상 `GrossMassExceeded` Fitting validation으로 보존했다.
- Wagon GrossMass 2350은 VehicleBase Reference와 `CLAIM-V60-GROSS-MASS` authority를 유지하며 CF-FQ-047 편의상 2356으로 변경하지 않았다.
- active weapon Mount handoff를 private pure policy로 분리하고 mass-independent regression을 추가했다. official Editor build `e70fa339ab574f31af30344bb7eed83f` PASS, RuntimeApply 전체 affected suite `b54cdb73449744bc9bf7ca470fd5c037` 16/16 PASS했다.
- canonical fixed-60Hz Step 8 Technical Benchmark RunId `a9e28515-05ea-4cf9-bc0a-80a49adfc018`이 current TargetHash `8d780eb07ee7ab4672233fd3d15a0db4`에 exact binding되어 PASS했다. runner는 Reference threshold/USER driving feel을 판정하지 않았다.
- P0-06의 남은 Gate는 USER direct Driving 재승인뿐이다. USER PASS 뒤 current DefinitionHash+Benchmark RunId persistent receipt를 기록하고 Recipe explicit Save + fresh persisted readback해야 한다.

### v0.1.4 - 2026-09-03

- `VBHAI-P0-05` affected runner terminal log를 회수해 CF-FQ-043 5/5 + CF-FQ-040 2/2 + CF-FQ-046 3/3 = affected 10/10 PASS, focused 3/3 및 official linked build PASS와 함께 Technical Validation을 닫았다.
- USER `진행해` 승인으로 `VBHAI-D1`을 `MountType=Turret / SizeLimit=Large / DefaultEquipmentPreset=None`으로 확정했다. persisted HeavyCannon과 RocketLauncher preset 모두 Turret/Large 요구를 확인했다.
- P0-06 자동 Editor start는 `runtime_protected_dirty`로 두 번 process start 전 차단됐다. diag에서 incomplete write/recovery candidate 0과 UE MCP unavailable을 확인했으며 보호 규칙을 우회하지 않는다.
- Current Gate는 USER가 Editor를 수동 시작한 뒤 existing runtime에 attach/reuse하여 Wagon Recipe/VehicleData exact Product Recovery를 수행하는 것이다. Wagon Product 신규 mutation은 아직 0이다.

### v0.1.3 - 2026-09-03

- `VBHAI-P0-03`을 구현해 Step 6 authored relation, Step 7 current-vs-prospective, Step 8 current Target semantic readback을 CF-FQ-046 Presentation owner 위에 연결했다.
- corrected official build `ad458f2ab93744d696317886db0542ae`와 test-source 포함 build `2ecd8e1ba4f0461bb837f002937820a6`가 모두 Exit 0 / DLL link PASS했다.
- `VBHAI-P0-04` focused fixture/runner를 추가했고 job `08c40cebf651489c94d7cd4805dfbb1e`에서 3/3 PASS했다.
- `VBHAI-P0-05` affected runner `0c6ff459b6504ae481f3d3c09bc3e10d`를 시작했고 CF-FQ-043 affected 5/5는 PASS했다. CF-FQ-040 Step7/8 및 CF-FQ-046 presentation affected는 같은 runner에서 순차 검증 중이다.
- Product Wagon Recipe/VehicleData/StaticMesh 신규 mutation은 계속 0이며 P0-05 전체 PASS 전 P0-06 recovery는 금지한다.

### v0.1.2 - 2026-09-03

- `VBHAI-P0-02 Unbound Socket Integrity + Adoption` 구현을 완료했다. Resolver와 분리된 `CFVehicleBuilderHardpointIntegrity` inventory/classifier, canonical grammar/collision-aware classification, UseHardpoints-only exact existing Socket adoption을 연결했다.
- Wagon 같은 `HP_Top_01` orphan은 LegacyCompatible에서 advisory로 보이고 자동 semantic mutation은 하지 않는다. explicit UseHardpoints 전환 뒤에만 `이 Socket 사용` action으로 Recipe HardpointIntent를 작성한다.
- 신규 helper는 mutation/save/fingerprint authority 0이며 existing `UpsertHardpointIntent` Recipe-only write lane만 재사용한다. CF-FQ-046 parallel presentation hunk는 보존했다.
- build `8dbc3d87250a40fb87c52cf8c2adae06`에서 신규 helper/BuilderVM/BuilderTab 포함 C++ compile과 static lib link는 PASS했다. 실행 중 Editor의 DLL lock 때문에 final DLL link만 LNK1104로 차단됐으며 Editor restart는 하지 않았다. official linked build는 P0-05에서 재검증한다.
- Product Wagon/RuntimeApply mutation은 0이다. exact next는 `VBHAI-P0-03 Mount Completion + Final Runtime Readback`이며 P0-03 시작 전 CF-FQ-046 latest shared diff를 다시 확인한다.

### v0.1.1 - 2026-09-03

- Design Audit `P0 0 / P1 5 / P2 2` 교정을 반영했다.
- unbound Chassis Socket inventory를 Resolver AssetSnapshot/ChassisLayoutFingerprint/ResolvedDefinitionHash/Driving acceptance와 완전히 분리된 Builder-private Editor-only R0 lane으로 고정했다.
- prefix-only HP_* 판정을 폐기하고 exact `HP_<KnownCategory>_<NN>` grammar + Recipe/Target collision 기반 StandardAdoptable classification을 고정했다. noncanonical HP_*는 advisory만 허용한다.
- LegacyCompatible/Unspecified/NoHardpoints에서는 adoption mutation을 금지하고 USER가 먼저 explicit UseHardpoints mode를 선택한 뒤 fresh refresh에서 adoption하는 2-step contract를 확정했다.
- unbound Socket 존재 자체를 completion blocker로 사용하지 않고 mode별 advisory/candidate 정보로 분리했다.
- Step 7은 current Target vs prospective Resolve를 분리하고 Step 8은 saved/current Target VehicleData를 summary authority로 고정했다.
- 순서를 `P0-04 Automation → P0-05 Technical Validation → P0-06 Wagon Product Recovery`로 교정해 Production Asset mutation 전에 Build/Automation PASS를 요구한다.
- `VBHAI-D1 Wagon Roof Mount Policy` USER/Design decision Gate와 Product UAT snapshot exact 범위, P0-06 intended 1/1 restoration baseline을 추가했다.
- 교정 후 재검수 결과 `P0 0 / P1 0`으로 PASS했다. 추가 P2로 canonical Standard identity round-trip을 명확히 해 `Top_01/Top_10/Top_100`은 허용하고 `Top_1/Top_001`은 NonCanonicalHardpointLike로 분류하도록 고정했다.
- Current Gate를 `VBHAI-P0-02 Unbound Socket Integrity + Adoption`으로 전환했다. 구현 전 CF-FQ-046 shared Builder dirty fresh overlap 재확인은 계속 mandatory다.

### v0.1.0 - 2026-09-03

- USER 승인으로 CF-FQ-047을 P1 / Ready로 정식 승격했다.
