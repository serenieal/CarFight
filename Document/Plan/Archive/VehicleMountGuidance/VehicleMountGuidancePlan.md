# Vehicle Builder Mount Guidance Plan

- Version: 0.2.0
- Date: 2026-09-03
- Status: Completed / Historical + Archived Path / VMG-P0-08 Current System Promotion Complete / G5 Physical Move Complete
- Feature: CF-FQ-043 Vehicle Builder 장비 장착점 Guidance UX
- Priority: P2
- Current Active Feature: CF-FQ-039 Production UI Visual Rework 유지
- Representative Plan: Document/Plan/Archive/VehicleMountGuidance/VehicleMountGuidancePlan.md
- Current System Owner: Document/Systems/Vehicles/VehicleBuilder.md v1.4.1

---

## 1. 목적

Guided Vehicle Builder에서 새 차량을 만들 때 Wheel Socket만큼 명확하게 장비 장착 위치와 장착 규칙을 계획할 수 있게 한다.

핵심 책임은 다음처럼 분리한다.

```text
Step 2
= Chassis / Wheel Mesh 자체 선택·확인·편집 진입

Step 3
= 물리 Hardpoint 위치 계획 + USER Socket 배치

Step 6
= 각 Hardpoint가 허용하는 Mount 규칙 설정

Step 7
= 전체 Diff 검토 + explicit VehicleData DefinitionApply
```

새 Runtime Hardpoint 체계, 자동 Socket 배치, Runtime Apply 변경은 만들지 않는다.

---

## 2. 보호 범위

이번 Feature 때문에 다음 완료/진행 기능을 재오픈하지 않는다.

```text
CF-FQ-040 Guided Vehicle Builder Done
- Wagon E2E
- WSA
- ESH-01~06

CF-FQ-041 Runtime Apply lifecycle
CF-FQ-042 Vehicle Builder 신규 차량 생성 UX Done / Clean PASS

Vehicle Runtime / Chaos tuning
Wheel Size Authority
Vehicle Builder Step 8 runtime benchmark semantics
- 단, P0-07 UAT에서 발견된 USER Driving PASS의 host-local persistence 결함은 Editor-only Recipe receipt로 교정했으며 runtime benchmark truth 자체는 변경하지 않는다.
```

CF-FQ-043의 Product 변경 범위는 Editor-only Builder / Recipe authoring UX에 한정한다.

---

## 3. VMG-P0-00 Current Contract Audit — PASS

Current Source 기준 관계:

- `HardpointSlot.LocationSlotId` = 차량의 물리 장착 위치 stable identity.
- `HardpointSlot.SocketName` = `HP_Top_01` 같은 Chassis StaticMesh Socket.
- `MountProfile.LocationSlotRef` = `SocketName`이 아니라 `LocationSlotId` 참조.
- `MountProfile.MountProfileId` = 피팅/런타임 장착 규칙 stable identity.
- `MountProfile.MountType` = Fixed / Gimbal / Turret / Launcher / Utility.
- `MountProfile.SizeLimit` = None / Small / Medium / Large.
- Socket RelativeLocation/RelativeRotation은 Hardpoint LocalLocation/LocalRotation으로 캡처된다.
- Runtime은 실제 Socket이 존재하면 Socket Transform을 우선 사용한다.
- 기본 축은 +X Forward, +Y Right, +Z Up이다.

Current Step 3은 Wheel Socket을 강하게 검증하지만 Hardpoint는 guidance-only다.
Current Step 6은 Hardpoint/Mount가 비어 있으면 Optional로 완료될 수 있어 신규 차량의 미결정과 의도적 0개를 구분하지 못한다.

Current Editor Authoring에는 `FCFHardpointIntent`, `FCFMountIntent`, `UpsertHardpointIntent`, `UpsertMountIntent`가 이미 있으므로 Raw VehicleData writer는 추가하지 않는다.

---

## 4. VMG-P0-01 Final Design Review + Design Audit Correction — PASS

### 4.1 Design Audit에서 발견한 P0-01 — CompanionMode를 Hardpoint lifecycle owner로 사용하지 않는다

Current `DeriveCompanionMode()`의 `NewVehicle / CompleteExisting`은 차량의 영구 출생 유형이 아니다.

현재 Source에서는 다음 상태가 생기면 `CompleteExisting`으로 전환된다.

```text
BuilderCommitReceipt 존재
또는
private Profile binding 존재
또는
Reference Evidence 존재
```

따라서 Guided 신규 차량도 Step 1 Companion 생성 뒤에는 `CompleteExisting`으로 파생될 수 있다.

FQ-043의 Step 3/6 mandatory guidance를 이 값에 연결하면 신규 차량이 핵심 Gate를 우회할 수 있으므로:

```text
ECFBuilderCompanionMode
= 기존 Companion bootstrap/completion owner 유지

Hardpoint/Mount Guidance lifecycle
= 별도 Recipe-owned Builder policy
```

로 분리한다.

FQ-043은 `DeriveCompanionMode()`의 의미를 확대하거나 기존 동작을 변경하지 않는다.

### 4.2 Design Audit에서 발견한 P0-02 — explicit-none + derived Configured 모델을 폐기한다

v0.1.1의:

```text
persistent: Unspecified / ExplicitNone
derived: HardpointIntents > 0 => Configured
```

모델은 Advanced typed upsert/remove와 조합되면 persistent `ExplicitNone`이 남은 채 Hardpoint가 생겼다가 다시 삭제되는 모순을 만든다.

최종 설계는 completion state가 아니라 **USER의 authoring policy/mode 자체를 persistent하게 소유**한다.

```cpp
ECFBuilderHardpointPlanMode
- LegacyCompatible
- Unspecified
- NoHardpoints
- UseHardpoints
```

`UseHardpoints + HardpointIntents empty`는 모순이 아니다.
“장착 위치를 사용할 계획이지만 아직 하나도 작성하지 않음”이라는 정상 Ready 상태다.

### 4.3 P1 교정 — stable ID / Utility / completion

- MountProfileId에 mutable MountType / SizeLimit을 포함하지 않는다.
- 신규 Standard MountProfileId는 creation-time `Mount_<LocationSlotId>` 제안을 사용한다.
- Utility는 current valid MountType이며 Utility Scanner는 `SizeLimit=None`을 사용할 수 있다.
- Step 6은 stable reference만 보지 않고 MountType/SizeLimit/preset compatibility까지 completion에 포함한다.
- Standard LocationCategory/LocationSlotId는 row 생성 뒤 stable identity로 취급하며 category 변경은 delete + add를 기본으로 한다.
- MountProfileId suggestion은 current Recipe와 Target의 existing identity collision을 모두 검사하고 collision 시 silent rename/suffix를 하지 않는다.

### 4.4 P1 교정 — shared Chassis / stale workflow

Current System의 reused Chassis Mesh는 Socket/WSA/Hardpoint geometry를 공유하고 per-Vehicle Socket override는 Scope Out이다.

따라서 Step 2/3의 Mesh/Socket editor entry에는:

```text
Chassis Socket은 StaticMesh 자산에 저장됩니다.
같은 Chassis Mesh를 사용하는 다른 차량도 이 Socket 위치를 공유합니다.
```

경고를 제공한다.

또한 Hardpoint Plan Mode는 Resolver fingerprint에서 제외되므로 Mode 변경 시 Builder가 이미 준비한 Final Review/Apply approval cache를 명시적으로 무효화하고 Step projection을 fresh rebuild한다.

### 4.5 기존 v0.1.1 교정사항 유지

다음 v0.1.1 교정은 그대로 유지한다.

- Step 2 Mesh 열기는 committed asset이 아니라 pending picker asset을 우선한다.
- Step 3에서 구조 오류는 Blocked, USER Socket 수동 작업 pending은 Ready로 구분한다.
- typed remove는 no-cascade이며 dependency conflict를 fail-closed한다.
- Existing custom/noncanonical naming과 same-Hardpoint multi-Mount는 migration하지 않는다.

---

## 5. Hardpoint Plan Mode Owner

### 5.1 Persistent owner

Runtime `UCFVehicleData`가 아니라 Editor-only `UCFVehicleRecipeData`가 소유한다.

최종 타입:

```cpp
ECFBuilderHardpointPlanMode
- LegacyCompatible
- Unspecified
- NoHardpoints
- UseHardpoints
```

최종 field:

```text
UCFVehicleRecipeData::BuilderHardpointPlanMode
```

serialized default:

```text
LegacyCompatible
```

따라서 FQ-043 이전 Existing Recipe는 로드만 해도 신규 mandatory guidance가 생기지 않는다.

### 5.2 Guided 신규 차량 생성 시 policy

기존 `FCFVehicleRecordCreateRequest`에 additive creation intent를 둔다.

권장 field:

```cpp
bool bRequireExplicitHardpointPlan = false;
```

일반/Legacy record creation:

```text
false
→ BuilderHardpointPlanMode = LegacyCompatible
```

Guided Vehicle Builder 신규 생성:

```text
true
→ BuilderHardpointPlanMode = Unspecified
```

이 bool은 ownership creation approval의 의미 일부이므로 `CreateVehicleRecords` Preview/ProposalHash에도 포함한다.
Preview 이후 flag가 바뀐 stale approval은 기존 R2 approval-scope contract에 의해 fail-closed해야 한다.

기존 `bRequireVehicleSpecificTransmission`에 Hardpoint 의미를 섞지 않는다.
Transmission policy와 Hardpoint Plan Mode는 독립 owner다.

### 5.3 Mode 의미와 Step 상태

```text
LegacyCompatible
= FQ-043 이전 Existing/Legacy Recipe
= current optional Hardpoint/Mount preservation semantics 유지
= explicit plan decision을 강제하지 않음

Unspecified
= Guided 신규 Recipe 초기값
= USER가 아직 0개/사용을 결정하지 않음
= Step 3 Ready / Next disabled

NoHardpoints
= USER가 "장착점 없음"을 명시
= HardpointIntents 0 + MountIntents 0일 때만 정상
= 조건 만족 시 intentional-zero completion 가능

UseHardpoints
= USER가 "장착 위치 사용"을 명시
= Hardpoint 0개도 정상 Ready
= 1개 이상 작성 후 structural/socket completion을 검증
```

### 5.4 Mode 전환 규칙

Standard UI는 먼저 명시적인 Mode 선택을 요구한다.

```text
Unspecified
├─ [장착점 없음] → NoHardpoints
└─ [장착 위치 사용] → UseHardpoints
```

`NoHardpoints` 전환 조건:

```text
HardpointIntents empty
MountIntents empty
```

하나라도 있으면 자동 삭제하지 않고 Mount → Hardpoint 제거 순서를 안내한다.

`UseHardpoints`에서 마지막 Hardpoint를 제거해도:

```text
UseHardpoints + HardpointIntents empty
```

로 남는다.
삭제 행위를 “장착점 없음 승인”으로 해석하지 않는다.

Advanced Workspace가 Mode와 별도로 semantic intent를 수정할 수 있으므로:

```text
Unspecified + intents 존재
→ silent mode 전환 금지
→ Step 3 Ready / explicit "장착 위치 사용" 결정 요구

NoHardpoints + Hardpoint/Mount intent 존재
→ inconsistent / Blocked

UseHardpoints + 0 Hardpoint
→ Ready
```

로 fail-closed한다.

### 5.5 Resolver / fingerprint 경계

`BuilderHardpointPlanMode`는 Builder workflow metadata다.

다음에 포함하지 않는다.

```text
Runtime VehicleData
FCFVehicleRecipeSnapshot의 Resolver semantic input
RecipeFingerprint
ResolvedDefinitionHash
DefinitionApply Diff
Packaged Runtime
```

현재 `BuilderTransmissionPolicy`처럼 Recipe가 Builder 전용 metadata를 소유하는 precedent를 재사용한다.

Mode write는 Slate가 UObject를 직접 수정하지 않고 BuilderVM-owned transaction 경로를 사용한다.

```text
FScopedTransaction
→ Recipe.Modify()
→ exact mode write
→ AuthoringRevision +1
→ MarkPackageDirty
→ PostEditChange
→ exact mode readback
→ Builder prepared review/approval cache invalidate
→ Step projection fresh rebuild
→ no Save
```

generic Resolver semantic fingerprint lane과 분리한다.

### 5.6 CompanionMode 경계

Hardpoint Plan completion의 mandatory/legacy 판단에는 `DeriveCompanionMode()`를 사용하지 않는다.

```text
BuilderHardpointPlanMode == LegacyCompatible
→ Existing/Legacy preservation lane

BuilderHardpointPlanMode == Unspecified / NoHardpoints / UseHardpoints
→ Guided explicit-plan lane
```

Step 1에서 Evidence/Profile/Receipt가 생겨 `ECFBuilderCompanionMode::CompleteExisting`으로 바뀌어도 Hardpoint Plan Mode는 변하지 않는다.

---

## 6. Naming / Standard vs Advanced Contract

### 6.1 LocationSlotId

LocationSlotId는 장비 역할이 아니라 물리 위치 stable identity다.

신규 Standard 기본:

```text
<LocationCategory>_<NN>
```

예:

```text
Top_01
Top_02
Front_01
Back_01
LeftSide_01
RightSide_01
```

물리적으로 필요할 경우 Bottom / Internal도 위치 category로 확장할 수 있다.
`Utility_01`은 신규 Standard 물리 위치 제안으로 사용하지 않는다.

단, 기존 차량이나 Advanced authoring에 이미 존재하는 `Utility_01` 같은 custom/legacy ID는 rename하지 않는다.

### 6.2 번호 생성 규칙

Standard에서 category를 추가할 때 current Recipe와 current Target의 같은 category numeric ID를 함께 확인한다.
CompanionMode에 따라 collision 검사 범위를 축소하지 않는다.

```text
existing Top_01, Top_03
→ next suggestion Top_04
```

최대 사용 번호 + 1 방식으로 생성한다.

삭제 후 자동 renumber/reuse하지 않는다.

Standard row가 생성된 뒤 LocationCategory / LocationSlotId는 stable identity로 취급한다.
category를 바꾸고 싶으면 기본 UX는 기존 row 삭제 → 새 category로 추가다.
silent ID/category rename은 하지 않으며 freeform identity 변경은 Advanced 책임으로 남긴다.

### 6.3 SocketName

신규 Standard Hardpoint 생성 시 한 번:

```text
HP_<LocationSlotId>
```

를 제안한다.

예:

```text
Top_01
→ HP_Top_01
```

`BuildSuggestedHardpointSocketName()` 규칙을 재사용한다.

LocationSlotId나 category가 후에 바뀌어도 기존 SocketName을 매 refresh마다 silent rename하지 않는다.
stable ID 변경 자체도 Standard에서는 가급적 delete + add로 다룬다.

### 6.4 MountProfileId

신규 Standard 1:1 경로의 creation-time suggestion:

```text
Mount_<LocationSlotId>
```

예:

```text
Top_01
→ Mount_Top_01
```

MountType/SizeLimit 변경으로 이 ID를 자동 변경하지 않는다.

생성 전 current Recipe.MountIntents와 current Target.MountProfiles의 MountProfileId를 모두 검사한다.
동일 stable ID가 이미 존재하면 silent suffix/rename으로 숨기지 않고 Standard add를 fail-closed하며 기존 identity를 USER에게 표시한다.

### 6.5 Standard / Advanced 경계

Standard Guided:

- LocationCategory 선택.
- LocationSlotId / SocketName / MountProfileId deterministic suggestion.
- stable ID free-text 상시 편집은 노출하지 않음.
- 신규 Standard는 1 Hardpoint : 1 MountProfile.
- noncanonical existing/custom 값은 표시만 하고 자동 정규화하지 않음.

Advanced Workspace:

- current freeform typed Hardpoint/Mount upsert 유지.
- custom LocationSlotId, SocketName, MountProfileId 허용.
- current validator 수준의 warning/error 표시.
- Standard naming으로 강제 migration하지 않음.
- same-Hardpoint multi-Mount 같은 고급 구조도 자동 삭제/변환하지 않음.

---

## 7. Typed Remove Safety Contract

### 7.1 Operation

기존 `ECFVehicleSemanticOp` 뒤에 append-only로 추가한다.

```text
RemoveMountIntent
RemoveHardpointIntent
```

기존 enum ordinal을 이동시키지 않는다.

`FCFVehicleSemanticChange`에는 삭제 identity 전용 payload를 둔다.

```text
RemoveMountProfileId
RemoveHardpointLocationSlotId
```

가짜 partial struct를 만들어 삭제 ID로 사용하지 않는다.

### 7.2 Existing R1 lane 재사용

두 operation 모두 기존 Recipe-only R1 semantic transaction을 재사용한다.

```text
mutation0 Preview
→ exact ProposalHash
→ AuthoringWrite
→ fresh Recipe fingerprint / Target hash / Resolver revision check
→ FScopedTransaction
→ Recipe mutation
→ readback
→ no Save
```

Target VehicleData는 변경하지 않는다.

### 7.3 RemoveMountIntent

- ID None = InvalidSemanticInput.
- exact MountProfileId가 있으면 그 요소만 제거.
- 이미 없으면 semantic NoChange.
- 다른 MountIntent/Hardpoint에는 영향 없음.

### 7.4 RemoveHardpointIntent

- ID None = InvalidSemanticInput.
- 해당 LocationSlotId를 참조하는 MountIntent가 하나라도 있으면 `DependencyConflict`.
- 참조가 없을 때 exact Hardpoint만 제거.
- 이미 없으면 semantic NoChange.
- StaticMesh Socket은 절대 삭제하지 않음.

### 7.5 Guided delete UX

Hardpoint row의 삭제는 참조 Mount가 있으면 disabled 또는 exact blocker를 표시한다.

```text
"Mount_Top_01이 Top_01을 사용 중입니다.
Step 6에서 장착 규칙을 먼저 제거하세요."
```

자동 cascade delete나 batch delete는 P0에 추가하지 않는다.

HardpointIntent 삭제 성공 뒤에도 같은 이름의 StaticMesh Socket은 자동 삭제하지 않는다.
결과 메시지에는 예를 들어:

```text
Top_01 Recipe 장착 위치를 제거했습니다.
StaticMesh Socket HP_Top_01은 삭제하지 않았습니다.
필요하면 Static Mesh Editor에서 직접 관리하세요.
```

처럼 Recipe semantic 삭제와 shared Mesh asset 편집을 분리해서 안내한다.

---

## 8. Step 2 / Step 3 Mesh Editor Button Contract

### 8.1 Step 2 — Mesh 자체 편집

Current Step 2에는 Object Picker만 있고 asset open 버튼이 없다.

최종 배치:

```text
차체 메시 [필수]
[Object Picker] [차체 Mesh 열기]

FL / 앞왼쪽 [필수]
[Object Picker] [Mesh 열기]

FR / 앞오른쪽 [선택]
[Object Picker] [Mesh 열기]

RL / 뒤왼쪽 [선택]
[Object Picker] [Mesh 열기]

RR / 뒤오른쪽 [선택]
[Object Picker] [Mesh 열기]
```

차체 버튼은 `PendingChassisMeshPath`를 연다.

Wheel 버튼은 해당 role의 pending asset을 연다.
FR/RL/RR이 비어 FL fallback을 사용하는 경우 버튼은 effective FL asset을 열고 Tooltip에 fallback임을 표시한다.

아직 `Mesh 설정 반영` 전인 새 picker asset을 열 수 있어야 한다.

### 8.2 Step 3 — Chassis Socket 편집

현재 generic `차체 메시 열기` 버튼은 두 문맥 버튼으로 교체한다.

Wheel 영역:

```text
[Wheel Socket 편집하기]
```

Hardpoint 영역:

```text
[Hardpoint Socket 편집하기]
```

둘 다 current committed Chassis StaticMesh를 연다.
기존 `UAssetEditorSubsystem::OpenEditorForAsset` backend를 공유한다.

Wheel Tooltip:

```text
현재 Chassis StaticMesh를 엽니다.
Wheel Socket 위치·회전·Scale은 USER가 직접 편집합니다.
Builder는 생성·이동·저장하지 않습니다.
```

Hardpoint Tooltip:

```text
현재 Chassis StaticMesh를 엽니다.
HP_* Socket 위치·회전은 USER가 직접 편집합니다.
+X Forward / +Z Up 기준을 사용하며 Builder는 자동 배치하지 않습니다.
```

Step 3에 Wheel Mesh 자체 편집 버튼을 중복 배치하지 않는다.
Wheel Mesh 자체는 Step 2, Wheel Socket은 Step 3 책임이다.

Step 2/3 Chassis editor entry 주변에는 shared asset 경고를 함께 노출한다.

```text
주의:
Chassis Socket은 차량별 데이터가 아니라 StaticMesh 자산에 저장됩니다.
같은 Chassis Mesh를 사용하는 다른 차량도 이 Socket 위치를 공유합니다.
```

P0에서 Reference 검색으로 공유 차량 수를 계산하거나 per-Vehicle Socket override를 만들지는 않는다.

---

## 9. Step 3 Completion Contract

Wheel Socket current strict validation은 보존한다.

Hardpoint mandatory/legacy lane은 `DeriveCompanionMode()`가 아니라 `BuilderHardpointPlanMode`가 결정한다.

### 9.1 LegacyCompatible

기존 preservation 계약 유지:

- stored LocalTransform 허용.
- SocketName None 허용 가능.
- Chassis에서 Socket missing이어도 stored transform이 보존 가능하면 advisory.
- custom/noncanonical existing 이름은 rename하지 않음.
- 기존 Hardpoint를 자동 recapture하지 않음.
- explicit Hardpoint plan decision을 새 completion blocker로 추가하지 않음.

### 9.2 Unspecified

Wheel 조건이 통과해도:

```text
BuilderHardpointPlanMode = Unspecified
→ Step 3 = Ready
→ Next = disabled
```

HardpointIntent가 Advanced를 통해 이미 존재해도 silent `UseHardpoints` 전환은 하지 않는다.
USER가 명시적으로 다음 중 하나를 선택한다.

```text
[장착점 없음]
[장착 위치 사용]
```

### 9.3 NoHardpoints

정상 intentional-zero:

```text
BuilderHardpointPlanMode = NoHardpoints
HardpointIntents empty
MountIntents empty
```

이면 Hardpoint 부분 Complete다.

HardpointIntent 또는 MountIntent가 하나라도 존재하면 policy/data inconsistency로 Blocked다.
Wheel 조건도 통과할 때만 Step 3 전체 Complete다.

### 9.4 UseHardpoints

`UseHardpoints + HardpointIntents empty`:

```text
Step 3 = Ready
Next = disabled
"장착 위치를 하나 이상 추가하세요."
```

구조 blocker:

- LocationSlotId None.
- duplicate LocationSlotId.
- SocketName None.

이 경우 Step 3 = Blocked.

USER 작업 pending:

- valid exact SocketName이지만 fresh Chassis에 아직 Socket 없음.

이 경우 Step 3 = Ready / Next disabled.

Complete:

- HardpointIntent 1개 이상.
- stable ID 구조 유효.
- 모든 authored Hardpoint Socket이 fresh Chassis에 실제 존재.
- Wheel 조건도 모두 PASS.

이 경우 Step 3 = Complete.

### 9.5 Naming warning과 completion 분리

Advanced/custom SocketName이 `HP_` prefix를 따르지 않더라도 exact Socket이 존재하면 naming warning만 낼 수 있다.

noncanonical prefix만으로 Legacy/Advanced vehicle을 Block하지 않는다.

### 9.6 Rotation

Builder는 Socket Rotation의 “올바른 물리 방향”을 자동 추론하지 않는다.

기술 안내:

```text
+X / Red = Forward / 기본 발사 방향
+Y / Green = Right
+Z / Blue = Up
```

실제 배치 적절성은 USER Acceptance 대상이다.

---

## 10. Step 6 Completion Contract

Step 6은 Step 5 Physics Proposal Complete prerequisite를 유지한다.

Hardpoint/Mount mandatory lane 역시 `BuilderHardpointPlanMode`를 사용한다.

### 10.1 LegacyCompatible

Current preservation logic 유지:

- Recipe HardpointIntents empty + Target HardpointSlots 존재 → 기존 슬롯 보존.
- Recipe MountIntents empty + Target MountProfiles 존재 → 기존 Mount 보존.
- Existing multi-Mount / custom ID / custom Socket을 1:1 Standard로 강제 migration하지 않음.
- current optional Mount semantics를 FQ-043 때문에 mandatory로 바꾸지 않음.

### 10.2 Unspecified

Step 3 explicit decision이 끝나지 않은 상태이므로 Step 6 completion에 도달할 수 없다.

projection/read가 가능하더라도 forward progress는 허용하지 않는다.

### 10.3 NoHardpoints

```text
HardpointIntents = 0
MountIntents = 0
```

이어야 한다.

하나라도 남아 있으면 inconsistent state로 Blocked.
둘 다 비어 있으면 Hardpoint/Mount area는 intentional-zero Complete다.

### 10.4 UseHardpoints — Standard 1:1

Standard Guided completion은 다음을 모두 요구한다.

1. HardpointIntent가 1개 이상.
2. Hardpoint마다 정확히 1개 MountIntent.
3. extra MountIntent 없음.
4. MountProfileId non-None + unique.
5. LocationSlotRef가 exact Hardpoint LocationSlotId 참조.
6. 동일 Hardpoint를 두 Standard Mount가 중복 참조하지 않음.
7. `MountType != None`.
8. Weapon Mount 규칙:
   - Fixed / Gimbal / Turret / Launcher → `SizeLimit != None`.
9. Utility 규칙:
   - Utility → `SizeLimit=None` 허용.
10. DefaultEquipmentPresetData는 optional.
11. preset이 지정됐으면 current EquipmentPreset compatibility를 통과해야 함.
12. MountProfileId가 current Recipe/Target existing identity와 collision하지 않아야 함.

### 10.5 DefaultEquipmentPresetData compatibility

preset이 지정됐을 때:

```text
EquipmentPresetData.CanUseOnMount(MountType, SizeLimit)
```

와 동일 current runtime compatibility contract를 재사용한다.

호환되지 않으면 Step 6 Blocked.

preset이 비어 있는 것은 허용한다.

### 10.6 bExposedModule

기본 `true`를 유지한다.

P0 Standard 핵심 입력으로 전면 노출하지 않고 Advanced 설정으로 유지한다.

---

## 11. Existing Vehicle Preservation Regression Contract

CF-FQ-043은 아래 회귀를 필수 보호한다.

### E1. LegacyCompatible Existing Hardpoint + empty Recipe intent

FQ-043 이전/Legacy Recipe의 `BuilderHardpointPlanMode=LegacyCompatible`에서 Target에 HardpointSlots가 있고 Recipe.HardpointIntents가 비어 있어도:

- Step 3에서 plan decision을 요구하지 않음.
- Step 6에서 existing Hardpoint 보존.
- Final Resolve가 기존 Hardpoint를 삭제하지 않음.
- CompanionMode가 어떤 값으로 파생되더라도 이 preservation rule은 동일함.

### E2. Existing direct LocalTransform

Existing Hardpoint SocketName이 None이어도 stored LocalLocation/LocalRotation을 그대로 보존한다.

Socket migration/recapture 강제 없음.

### E3. Existing missing Socket

Existing SocketName이 Chassis에 없어도 stored LocalTransform이 current candidate에서 보존되면 advisory만 표시한다.

### E4. Existing MountProfiles

Recipe.MountIntents가 비어 있어도 existing MountProfiles exact 값 보존:

- MountProfileId
- LocationSlotRef
- MountType
- SizeLimit
- DefaultEquipmentPresetData
- bExposedModule

### E5. Existing custom/noncanonical names

custom LocationSlotId / SocketName / MountProfileId를 Standard naming으로 자동 rename하지 않는다.

### E6. Existing same-Hardpoint multi-Mount

기존 차량의 동일 Hardpoint 다중 MountProfile은 그대로 보존한다.

신규 Standard 1:1 규칙을 migration rule로 사용하지 않는다.

### E7. Existing에 새 Builder Hardpoint 추가

새로 추가한 exact intent만 Builder-managed로 다루고 unrelated existing slot은 변경하지 않는다.

### E8. Exact remove

RemoveMountIntent / RemoveHardpointIntent는 exact stable ID 하나만 변경한다.
Hardpoint는 참조 Mount가 남아 있으면 삭제 차단한다.

### E9. Step 3/6 Target mutation 0

Recipe authoring 중 Target VehicleData hash/content는 Step 7 explicit Apply 전까지 unchanged다.

### E10. StaticMesh mutation 0

Builder는 Socket create/move/delete/save를 수행하지 않는다.

### E11. Untouched Existing resolved identity

CF-FQ-043 UX를 열고 refresh만 한 Existing 차량은 untouched Hardpoint/Mount resolved Definition과 DefinitionHash가 바뀌지 않아야 한다.

---

## 12. Implementation Ownership

### C++

C++가 소유:

- `ECFBuilderHardpointPlanMode` Editor-only type.
- Recipe Builder metadata field + LegacyCompatible serialized default.
- Guided record creation의 `bRequireExplicitHardpointPlan` + ProposalHash binding.
- Builder-owned Mode transaction/readback + prepared workflow invalidation.
- typed remove operations.
- preview/commit/dependency safety.
- deterministic naming/collision helper.
- Step 3/6 evaluator.
- pending/current asset open helper.
- focused Automation.

### Slate

`SCFVehicleBuilderTab`가 소유:

- None / use-hardpoint USER controls.
- Hardpoint rows.
- Mount rows.
- add/remove buttons.
- Step 2/3 contextual editor buttons.
- Tooltip / guidance presentation.
- USER explicit action 호출.

Slate가 Recipe/VehicleData/StaticMesh UObject field를 직접 수정하지 않는다.

### Blueprint

이번 P0 Editor Builder authoring 기능에 Blueprint 신규 로직은 필요하지 않다.

Runtime gameplay나 Vehicle Pawn Blueprint 책임을 추가하지 않는다.

---

## 13. Expected Source Range

주요 변경 후보:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAuthoringTypes.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAIContract.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleRecipeData.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleUXTypes.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderVM.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTab.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleAuthoringService.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleAuthoringVM.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleUXOps.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderGuide.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderVM.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderTab.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleSnapshotBuilder.cpp
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleSnapshotTypes.h
```

단, BuilderHardpointPlanMode를 Resolver snapshot에 넣지 않는 최종 설계이므로 Snapshot 파일은 테스트/비포함 계약 확인만으로 끝날 수 있다.

현재 dirty `CFVehiclePawn` 및 CF-FQ-041 Runtime Apply 파일은 CF-FQ-043 구현 대상이 아니다.

---

## 14. Scope Out

```text
Hardpoint 자동 geometry 배치
Wheel center 자동검출
Mesh surface 추론
Socket 자동 생성/삭제/이동
- 단, Step 3에서 USER가 exact 이름의 `추가 후 편집`을 명시적으로 눌렀을 때 current Chassis 원점에 Socket 1개를 생성하는 explicit Editor action은 허용한다. 자동 배치/자동 저장/자동 rename은 금지한다.
Socket 자동 Rotation 계산
StaticMesh 자동 Save
새 Runtime Hardpoint 체계
Vehicle Runtime Apply 변경
Fitting Runtime 재설계
Inventory Runtime 재설계
Chaos tuning
Wheel Size Authority 변경
CF-FQ-040 Wagon/ESH 재검증
CF-FQ-041 Runtime Apply 변경
CF-FQ-042 신규 차량 생성 UX 재오픈
Existing Vehicle naming migration
Existing same-Hardpoint multi-Mount normalization
```

---

## 15. 단계 계획

### VMG-P0-00 Current Contract Audit — Complete / PASS

현행 Source/System 감사와 문제 정의 완료.

### VMG-P0-01 Detailed UX / State Contract Design Review — Complete / Design Audit Correction PASS

확정:

- CompanionMode와 Hardpoint Guidance lifecycle 분리.
- Recipe-owned `LegacyCompatible / Unspecified / NoHardpoints / UseHardpoints` Mode.
- FQ-043 이전 Recipe의 LegacyCompatible default preservation.
- Guided 신규 creation flag + ProposalHash binding.
- Mode write의 Builder-owned transaction/readback + prepared review/approval invalidation.
- stable naming / Advanced 경계.
- 생성 후 Standard LocationCategory/LocationSlotId stable lock.
- MountProfileId stable naming + Recipe/Target collision fail-closed.
- typed remove no-cascade contract.
- shared Chassis Mesh Socket warning.
- Step 2 pending asset / Step 3 current Chassis editor entry.
- Step 3/6 Mode 기반 Ready/Blocked/Complete 규칙.
- Step 6 Standard 1:1 규칙 + Utility 예외.
- Existing preservation regression matrix.
- CF-FQ-041 dirty Runtime 경계 비접촉.

Product Source mutation 0 / UE Asset mutation 0.

### VMG-P0-02 Recipe State / Typed Remove Semantics — Complete / Technical PASS

구현 완료:

1. `ECFBuilderHardpointPlanMode = LegacyCompatible / Unspecified / NoHardpoints / UseHardpoints`를 Editor-only Recipe persistent metadata로 추가하고 existing Recipe serialized default를 `LegacyCompatible`으로 보존했다.
2. `FCFVehicleRecordCreateRequest::bRequireExplicitHardpointPlan=false`를 additive 추가하고 Guided Builder creation만 `true → Unspecified`를 기록한다.
3. `bRequireExplicitHardpointPlan=true`만 CreateVehicleRecords ProposalHash에 additive token으로 binding해 기존 false creation hash payload를 보존했다.
4. Builder-owned `CommitHardpointPlanMode()` transaction을 구현했다.
   - `FScopedTransaction → Modify → exact Mode write → AuthoringRevision +1 → MarkPackageDirty → PostEditChange → exact readback`.
   - Resolver semantic fingerprint에는 포함하지 않는다.
   - prepared Final Review / DefinitionApply approval cache는 폐기한다.
   - 이미 완료된 DefinitionApply guarded Undo token은 별도 복구 권한이므로 보존한다.
   - no Save / no Target / no StaticMesh mutation.
5. append-only `RemoveMountIntent / RemoveHardpointIntent`와 exact deletion identity payload를 추가했다.
6. RemoveMount:
   - None ID = InvalidSemanticInput.
   - exact MountProfileId만 제거.
   - already absent = NoChange.
7. RemoveHardpoint:
   - None ID = InvalidSemanticInput.
   - 참조 Mount가 있으면 prospective/persistent 양쪽에서 `DependencyConflict`.
   - no-cascade.
   - exact LocationSlotId만 제거.
   - StaticMesh Socket 삭제 0.
8. AuthoringVM / BuilderVM typed wrapper와 fresh state rebuild를 연결했다.
9. last Hardpoint remove는 `UseHardpoints`를 유지하며 `NoHardpoints`를 자동 승인하지 않는다.
10. NoHardpoints 전환은 HardpointIntents/MountIntents가 모두 empty일 때만 허용한다.

Validation evidence:

```text
Official CarFight_ReEditor Build
- first build 68951250c9ab4b12baca7fcb65657f4a
  - direct include 누락 FCFVehicleResolver compile error 발견
  - 코드 교정
- corrected build f351c681b98f4719bfde109c5f3d827a
  - PASS / Exit Code 0
- post-review build 1a539c28994e47b1a955bfa0233bf7ad
  - PASS / Exit Code 0
- final hash-compatibility build d3e406482e6f4e6a9adac9d65b781638
  - PASS / Exit Code 0

Focused / affected Automation
- CarFight.DataAuthoring.CF_FQ_043.VMG_P0_02.RecipeStateTypedRemove PASS
- CF_FQ_042.VBCUX_P0_02.RecordCreation PASS
- CF_FQ_042.VBCUX_P0_03.Naming PASS
- CF_FQ_042.VBCUX_P0_04.FocusedRegression PASS
- CF_FQ_040.VB_P0_09.BuilderStep1Reference PASS
- DAUTH_P0_11.FrozenUX.MeshCreate PASS

Final broad DataAuthoring rerun
- 92 total
- 90 PASS
- 2 FAIL
- repeated unrelated/out-of-scope failures:
  - CF_FQ_040.VB_P0_09.WagonTransmissionDraft
  - DAUTH_P0_08.Batch.AllowlistProjection
- VMG-P0-02와 직접 affected regression은 모두 PASS
- Engine exit code 0
```

두 broad failure는 VMG-P0-02 변경 파일과 직접 겹치지 않으며 final hash-only compatibility correction 전/후 동일한 두 test가 반복 실패했다.
따라서 VMG-P0-02 Technical PASS를 차단하지 않고 별도 기존 회귀/fixture 문제로 남긴다.

CF-FQ-041 Runtime Apply dirty 파일, Runtime VehicleData, StaticMesh, UE Content Asset은 수정하지 않았다.

### VMG-P0-03 Step 3 Hardpoint Planning UX — Complete / Technical PASS

구현 완료 범위:

1. Step 3 Hardpoint Plan UI를 Recipe-owned `BuilderHardpointPlanMode` authority로 연결했다.
   - `LegacyCompatible`: Existing/custom 구조를 자동 migration하지 않는다.
   - `Unspecified`: explicit USER 결정 전 Ready / forward incomplete.
   - `NoHardpoints`: Hardpoint/Mount가 모두 empty일 때 intentional-zero Complete, semantic row가 남아 있으면 Blocked.
   - `UseHardpoints`: 0개는 Ready, one+는 structural/socket truth로 Ready/Blocked/Complete를 판정한다.
2. Standard physical category add를 추가했다.
   - `Top / Front / Back / LeftSide / RightSide / Bottom / Internal`.
   - `LocationSlotId = <Category>_<NN>`.
   - `SocketName = HP_<LocationSlotId>` creation-time suggestion.
   - current Recipe + current Target의 same-category numeric identity를 함께 검사해 max-used+1을 사용하고 삭제된 번호를 재사용/renumber하지 않는다.
3. Standard row에 exact category / LocationSlotId / stored SocketName / current Chassis Socket 상태 / copy / typed delete를 표시한다.
   - stored `SocketName=None`인 Legacy direct-LocalTransform row는 `HP_*`를 임의 합성하지 않고 `(없음) / stored LocalTransform 보존`으로 표시한다.
   - 삭제는 P0-02 typed no-cascade remove를 사용하고 Mount dependency가 있으면 기존 `DependencyConflict`를 그대로 surface한다.
   - 삭제 뒤 StaticMesh Socket은 자동 삭제하지 않으며 stored SocketName None이면 존재하지 않는 Socket 이름을 안내 문구로 만들어내지 않는다.
4. `EvaluateSocketGuideStep()`을 Mode-authoritative 상태기로 교정했다.
   - Wheel Socket strict validation은 선행 유지.
   - UseHardpoints structural blocker: None/duplicate LocationSlotId, SocketName None.
   - exact SocketName이 valid하지만 current Chassis에 아직 없으면 Step 3은 Ready.
   - 모든 authored Hardpoint exact Socket이 current Chassis에 존재하면 Complete.
   - non-`HP_` custom prefix는 naming advisory이며 단독 blocker가 아니다.
5. Resolver/R3 safety와 Builder 작성 중 상태를 분리했다.
   - shared Resolver의 `HardpointSocketMissing=Blocked`는 변경하지 않았다.
   - Builder에 `CanUseHardpointSocketDraftRead()` narrow gate를 추가해 `UseHardpoints` 작성 중 `HardpointSocketMissing` 및 그에 종속된 LocalLocation/LocalRotation unresolved blocker만 존재할 때만 Step 3 diagnostic read를 허용한다.
   - 다른 Error/Blocked가 하나라도 섞이면 일반 fail-closed를 유지한다.
   - 따라서 Builder는 missing Socket을 Ready로 안내할 수 있지만 shared Resolver는 Blocked이고 R3 `BuildApplyApprovalProposal()`도 `ValidationBlocked`로 거부된다.
6. Step 3 editor entry를 문맥별로 분리했다.
   - `Wheel Socket 편집하기`.
   - `Hardpoint Socket 편집하기`.
   - 둘 다 current committed Chassis StaticMesh를 `UAssetEditorSubsystem`으로 연다.
7. Step 2 pending Mesh editor entry 누락을 correction에서 보완했다.
   - Chassis, FL, FR, RL, RR Object Picker 각각 `열기` 버튼을 제공한다.
   - 아직 `Mesh 설정 반영` 전이어도 current pending picker path를 직접 연다.
   - optional FR/RL/RR이 비어 있으면 current pending FL Mesh fallback을 연다.
   - Chassis 직접 편집은 같은 StaticMesh를 사용하는 다른 차량에도 Mesh/Socket 변경이 공유된다는 경고를 Step 2에 표시한다.
8. Step 3에는 shared Chassis Socket warning과 축 기준을 명시한다.
   - +X / Red = Forward / 기본 발사 방향.
   - +Y / Green = Right.
   - +Z / Blue = Up.
   - Builder는 Socket 생성/이동/회전/저장을 자동 수행하지 않는다.

Validation evidence:

```text
Final Official CarFight_ReEditor Build
- d5f326020bd440428afec63f1927293a
- PASS / Exit Code 0
- Step 2 pending Mesh open correction 포함

Focused / directly affected
- CarFight.DataAuthoring.CF_FQ_043.VMG_P0_03.Step3HardpointPlanning PASS
  - zero Hardpoint: Unspecified Ready / NoHardpoints Complete / UseHardpoints+0 Ready
  - Existing intent + Unspecified: silent mode transition 없이 Ready
  - Existing Top_03 + HP_Top_03: explicit UseHardpoints Complete
  - Recipe+Target max-used+1: Top_04 → Top_05
  - missing HP_Top_04/05: Builder Step 3 Ready
  - exact Socket 추가 후: Complete
  - missing Socket 상태 shared Resolver: Blocked / HardpointSocketMissing 유지
  - missing Socket 상태 R3 DefinitionApply approval proposal: ValidationBlocked
  - Target Definition hash 불변 / Target package clean
- CarFight.DataAuthoring.CF_FQ_043.VMG_P0_02.RecipeStateTypedRemove PASS
- CF_FQ_042.VBCUX_P0_02.RecordCreation PASS
- CF_FQ_042.VBCUX_P0_03.Naming PASS
- CF_FQ_042.VBCUX_P0_04.FocusedRegression PASS

Final broad DataAuthoring
- 93 total
- 91 PASS
- 2 FAIL
- repeated unrelated/out-of-scope baseline failures:
  - CF_FQ_040.VB_P0_09.WagonTransmissionDraft
  - DAUTH_P0_08.Batch.AllowlistProjection
- Engine exit code 0
```

Step 2/3 editor open은 Slate/editor-entry 기능이므로 이번 Gate에서는 compile + broad non-regression으로 닫고, exact USER interaction은 VMG-P0-06/07 시나리오 V/W에서 다시 확인한다.

CF-FQ-041 Runtime Apply dirty, Runtime VehicleData, Product StaticMesh/UE Content Asset은 FQ-043에 의해 mutation하지 않았다.

### VMG-P0-04 Step 6 MountProfile Guided UX — Technical PASS

- `UseHardpoints`에서 Hardpoint별 Standard 1:1 Mount row를 제공한다.
- MountType / SizeLimit / optional EquipmentPreset은 Slate-local transient draft로 유지하고 explicit `장착 규칙 반영`에서만 typed Recipe write를 수행한다.
- 새 Standard identity는 `Mount_<LocationSlotId>`이며 current Recipe/Target collision 시 자동 suffix/rename 없이 fail-closed한다.
- existing 1:1 Mount update는 current MountProfileId와 `bExposedModule`을 stable 보존한다.
- Fixed/Gimbal/Turret/Launcher는 SizeLimit None을 차단하고 Utility는 SizeLimit None을 허용한다.
- EquipmentPreset이 존재하면 current `UCFEquipmentPresetData::CanUseOnMount()` compatibility를 commit 전 검증한다.
- `LegacyCompatible`에서는 custom ID / multi-Mount / Existing Target MountProfiles를 Standard 1:1로 강제 migration하지 않는다.
- Step 6 Hardpoint/Mount policy authority는 `BuilderHardpointPlanMode`이며 `ECFBuilderCompanionMode`와 분리한다. Mode는 Resolver RecipeSnapshot/fingerprint에 넣지 않고 `FCFBuilderGameplayGuidanceRequest` transient projection으로만 R0 Guidance에 전달한다.
- exact Mount 삭제는 existing typed remove lane을 사용하며 Hardpoint/StaticMesh/Target VehicleData를 cascade mutation하지 않고 Save하지 않는다.
- 코드감사에서 stale `Step 6 전체 read-only` 문구를 제거하고 8영역 Gameplay Guidance/Socket 진단만 read-only, Standard Mount panel의 explicit Recipe write/remove만 허용하는 실제 경계로 교정했다.

Technical evidence:

```text
Official Editor build after code audit:
- ad78366e7ebc4217be386a8ed5bc28f2
- PASS / Exit Code 0

Final broad DataAuthoring:
- 98 total
- 96 PASS
- 2 FAIL
- VMG_P0_04.Step6MountPlanning PASS
- CF_FQ_040.VB_P0_06.GameplayGuidance PASS
- VMG_P0_02 / VMG_P0_03 PASS
- repeated unrelated/out-of-scope baseline failures:
  - CF_FQ_040.VB_P0_09.WagonTransmissionDraft
  - DAUTH_P0_08.Batch.AllowlistProjection
- Engine exit code 0
```

Focused coverage includes Target MountProfileId collision, Turret+SizeNone rejection, Utility+None Scanner compatibility, incompatible/compatible EquipmentPreset, stable existing ID update, exact remove, Step 6 Complete→NeedsReview transition, no-target/no-save.

### VMG-P0-05 New / Existing Preservation Guards — Technical PASS

E1~E11 regression을 `CarFight.DataAuthoring.CF_FQ_043.VMG_P0_05.ExistingPreservation` focused Automation으로 고정했다.

검증 범위:

- E1: `LegacyCompatible + empty Recipe Hardpoint/Mount intents`에서 Step 3 plan blocker 없음, Step 6 Existing Hardpoint/Mount preserve, exact resolved DefinitionHash 유지, CompanionMode New/CompleteExisting 독립성 확인.
- E2: `SocketName=None` Existing Hardpoint의 stored LocalLocation/LocalRotation을 accepted authority로 유지.
- E3: authored Socket이 Chassis에 없어도 stored LocalTransform이 유지되면 advisory-only guidance로 보존.
- E4: empty Recipe MountIntents에서도 Existing MountProfile의 6 active leaf를 exact typed canonical value로 보존.
- E5: `Deck_Custom_A / Socket_Custom_Deck_A / LegacyTurretAlpha / LegacyUtilityBeta` 같은 noncanonical identity를 자동 rename하지 않음.
- E6: 동일 Hardpoint에 Turret + Utility multi-Mount 2개를 LegacyCompatible에서 exact 보존.
- E7: Existing pins를 유지한 채 새 `Top_01 / HP_Top_01`만 Builder-managed semantic row로 추가하고 기존 custom Hardpoint/Mount leaf는 exact unchanged.
- E8: 새 `Mount_Top_01` 참조가 남으면 Hardpoint exact remove를 DependencyConflict로 차단하고 Mount → Hardpoint 순서 exact remove PASS.
- E9: 전체 Step 3/6 authoring 동안 Target VehicleData DefinitionHash/package clean 유지.
- E10: custom/future Socket object identity/location/rotation unchanged, direct/missing Socket 자동 생성 0.
- E11: untouched Existing select/refresh 및 add→remove round-trip 뒤 resolved DefinitionHash와 FieldDiff 0 exact 복귀.

Technical evidence:

```text
Official Editor build:
- df8bb1ccb9c740f58b61af1c4b5c5371
- PASS / Exit Code 0

Broad DataAuthoring:
- 99 total
- 97 PASS
- 2 FAIL
- VMG_P0_05.ExistingPreservation PASS
- VMG_P0_02 / P0_03 / P0_04 PASS
- VB_P0_06.GameplayGuidance PASS
- repeated unrelated baseline failures only:
  - CF_FQ_040.VB_P0_09.WagonTransmissionDraft
  - DAUTH_P0_08.Batch.AllowlistProjection
- Engine exit code 0
```

제품 Source 추가 수정 없이 Automation coverage만 추가했고 CF-FQ-041 Runtime Apply, CF-FQ-044 Catalog, Product StaticMesh/Content/Target VehicleData는 비접촉이다.

### VMG-P0-06 Focused + Affected Technical Validation — Technical PASS

A~Y contract matrix를 기존 P0-02~05 regression + 전용 `CarFight.DataAuthoring.CF_FQ_043.VMG_P0_06.ContractMatrix`로 닫았다.

Coverage summary:

```text
A-E : P0-02/P0-03 — Legacy/Guided mode lifecycle, Unspecified explicit decision
F-G : P0-06 ContractMatrix — NoHardpoints intentional-zero Step3/Step6 Complete, out-of-band semantic conflict Blocked
H-L : P0-03 + P0-06 — UseHardpoints Ready/missing/found Socket, None/duplicate Blocked, deterministic stable naming
M-Q : P0-04 — Standard 1:1 Mount, Size/Utility/Preset/collision
R-T : P0-02/P0-05 — dependency-safe exact remove, last Hardpoint keeps UseHardpoints
U   : P0-05 — direct LocalTransform / missing Socket advisory / custom / multi-Mount preserve
V   : source-path audit + official compile — Step2 Chassis exact PendingChassisMeshPath, Wheel exact pending role or pending FL fallback
W   : source-path audit + official compile — Step3 Wheel/Hardpoint buttons use GetCurrentChassisMeshPath → UAssetEditorSubsystem::OpenEditorForAsset; Step2/3 shared-Chassis warning present
X   : P0-02 — Mode change invalidates prepared Final Review/Apply cache
Y   : P0-02~06 — no auto Save / no Target Apply / no StaticMesh mutation
```

V/W의 실제 Asset Editor 창 열림과 USER가 경고/문맥을 이해하는지는 Technical Automation이 억지로 GUI를 조작하지 않고 P0-07 USER Acceptance에서 확인한다.

P0-06 전용 gap test 첫 broad에서 F aggregate가 1건 실패했으나 제품 결함이 아니라 zero fixture에 unrelated VehicleBase/private Profile source가 없어 다른 Gameplay 영역이 Blocked였던 test setup 오류였다. P0-04와 동일한 Builder-private 4 Profile fixture를 연결해 Step6 전체 Complete 의미를 유지한 채 교정했다.

Technical evidence:

```text
Initial P0-06 official Editor build:
- 1cc8fbf765cd4854b04885142521bb46
- PASS / Exit Code 0

Fixture correction official Editor build:
- b2b78f1042854e7abdd202849e94f03d
- PASS / Exit Code 0

Final broad DataAuthoring:
- 100 total
- 98 PASS
- 2 FAIL
- VMG_P0_02 RecipeStateTypedRemove PASS
- VMG_P0_03 Step3HardpointPlanning PASS
- VMG_P0_04 Step6MountPlanning PASS
- VMG_P0_05 ExistingPreservation PASS
- VMG_P0_06 ContractMatrix PASS
- VB_P0_06 GameplayGuidance PASS
- repeated unrelated baseline failures only:
  - CF_FQ_040.VB_P0_09.WagonTransmissionDraft
  - DAUTH_P0_08.Batch.AllowlistProjection
- Engine exit code 0
```

P0-06 product behavior mutation은 0이며 `WITH_DEV_AUTOMATION_TESTS` friend seam + test coverage만 추가했다. CF-FQ-041 Runtime Apply, CF-FQ-044 Catalog, Runtime/Chaos/UE Content/Target VehicleData는 비접촉이다.

### VMG-P0-07 USER Acceptance

USER가 실제 신규 차량에서 확인:

1. 장착점 없음 선택 의미.
2. 원하는 위치를 여러 개 추가하는 흐름.
3. Top_01 / HP_Top_01 / Mount_Top_01 관계 이해.
4. Step 2 Chassis/Wheel Mesh 직접 열기.
5. Step 3의 단일 `Chassis Socket 편집하기` 버튼이 Wheel/Hardpoint 공통 Chassis를 여는지 확인.
6. Wheel/Hardpoint exact SocketName을 읽기 전용 텍스트/복사로 확인하고, 누락 Socket의 `추가 후 편집`이 exact 이름 원점 생성 → Editor open → no-auto-save로 동작하는지 확인.
7. +X Forward / +Y Right / +Z Up 기준.
8. `파괴 FX Socket (선택)`이 차량 파괴 FX용 별도 위치라는 의미를 이해하고 필요 없으면 생략 가능함을 확인.
9. Step 6 MountType/SizeLimit 이해.
10. Utility Scanner 같은 current valid 타입 표현.
11. Existing Vehicle 비강제 migration.
12. 완료 차량 USER Driving PASS가 persistent Target Definition receipt로 재기동/동일 Definition의 새 benchmark RunId에서도 유지되고 Target DefinitionHash drift에서만 다시 요구되는지 확인.

### VMG-P0-08 Current System Promotion

완료 뒤 `Document/Systems/Vehicles/VehicleBuilder.md`에 Current Hardpoint/Mount Guidance 계약을 승격한다.

FeatureQueue → Done
ActiveWork / Plan Index Ready route 제거
Plan → Historical lifecycle

---

## 16. 완료 조건

1. CompanionMode와 Hardpoint Plan lifecycle이 분리되어 있다.
2. FQ-043 이전 Recipe는 LegacyCompatible default로 기존 동작을 보존한다.
3. Guided 신규 Recipe는 Unspecified로 생성되고 Companion 생성 뒤에도 Mode가 유지된다.
4. Unspecified / NoHardpoints / UseHardpoints의 USER 의도와 incomplete 상태를 구분한다.
5. NoHardpoints와 semantic intent가 충돌하면 fail-closed한다.
6. 마지막 Hardpoint 삭제가 NoHardpoints를 자동 승인하지 않는다.
7. LocationSlotId / SocketName / MountProfileId가 stable naming 규칙을 따른다.
8. Standard category/identity는 생성 후 silent rename하지 않는다.
9. MountProfileId Recipe/Target collision을 fail-closed한다.
10. Advanced custom/existing identity를 자동 rename하지 않는다.
11. typed safe remove와 dependency guard가 있다.
12. Builder는 StaticMesh Socket을 자동 생성/배치/삭제하지 않는다. USER explicit `추가 후 편집`만 exact 이름 Socket 1개를 원점 생성할 수 있고 자동 Save/rename/transform 추론은 하지 않는다.
13. reused Chassis Socket 공유 위험을 USER에게 알린다.
14. Step 2 pending Chassis/Wheel Mesh를 직접 열 수 있다.
15. Step 3은 Wheel/Hardpoint 공통 `Chassis Socket 편집하기`, exact 이름 copy, 누락 Socket `추가 후 편집` 진입을 제공한다.
16. Step 3 Guided completion이 explicit Mode + actual Socket truth를 반영한다.
17. Step 6 Standard UseHardpoints 1:1 Mount rule을 검증한다.
18. Utility current compatibility를 잘못 비활성화하지 않는다.
19. Mode 변경이 stale prepared Final Review/Apply approval을 재사용하지 못하게 한다.
20. Existing Vehicle E1~E11 보존 회귀를 통과한다.
21. Step 3/6은 Target VehicleData를 변경하지 않는다.
22. Step 7 explicit DefinitionApply / no-auto-save 계약을 유지한다.
23. focused/affected Technical PASS.
24. USER Acceptance PASS.
25. VehicleBuilder Current System 문서 승격.

---

## 17. Current Checkpoint

```text
Feature: CF-FQ-043
Status: Done / Historical
VMG-P0-00: PASS
VMG-P0-01: Design Audit Correction PASS
VMG-P0-02: Technical PASS
VMG-P0-03: Technical PASS
VMG-P0-04: Technical PASS
VMG-P0-05: Technical PASS
VMG-P0-06: Technical PASS
VMG-P0-07: USER Acceptance PASS
VMG-P0-08: Current System Promotion Complete
Current Gate: None — Feature Done / Historical

Implementation: Editor-only C++ / Slate complete through VMG-P0-07; Current System promotion complete
P0-07 Acceptance: Socket/Naming USER PASS + Wagon USER Driving re-accept/save/Step8 Complete + fresh persisted receipt verification / DataAuthoring 100/100 PASS
Product Source mutation: FQ-043 Editor Source only
UE Asset automatic mutation: 0; USER explicit `추가 후 편집` 시 current Chassis StaticMesh Socket mutation 허용 / no-auto-save
Runtime/Chaos mutation: 0

Protected:
CF-FQ-040 Done
CF-FQ-041 lifecycle / dirty Runtime work unchanged
CF-FQ-042 Done / Clean PASS
WSA / ESH / Wagon evidence preserved
```

---

## 18. Changelog

### Maintenance - 2026-09-06

- G5 Physical Move를 완료해 대표 Plan을 `Document/Plan/Archive/VehicleMountGuidance/VehicleMountGuidancePlan.md`로 이동했다. 완료 evidence와 Current System 계약은 변경하지 않는다.
- Migration: 이전 `Document/Plan/VehicleMountGuidance/VehicleMountGuidancePlan.md`는 당시 Historical 기록에서만 유효하며 현재 탐색 경로는 Archive 경로다.

### v0.2.0 - 2026-09-03

- `VMG-P0-07 USER Acceptance`를 최종 PASS로 닫았다. USER가 Wagon에서 USER Driving PASS → Recipe 저장 → Step 8 Complete를 직접 확인했다.
- fresh managed AssetDump 재생성으로 `DA_Recipe_Wagon.BuilderDrivingAcceptanceReceipt`의 exact Target path, DefinitionHash `0e5b48e8dcd39deba441da9237218be6`, accepted Benchmark RunId 직렬화를 확인했다.
- `VMG-P0-08 Current System Promotion`을 완료해 Hardpoint/Mount/Socket Guidance와 persistent USER Driving acceptance 계약을 `Systems/Vehicles/VehicleBuilder.md v1.2.0`에 승격했다.
- Feature를 Done으로 전환하고 Ready routing에서 제거한다. 대표 Plan은 `Historical + Retained Path`로 현재 `VehicleMountGuidance/` 경로에 보존하며 물리 Archive 이동은 별도 G5 정리 조건에서만 수행한다.
- final technical baseline은 official Editor build PASS와 `CarFight.DataAuthoring 100/100 PASS`. CF-FQ-041/044 병렬 dirty와 Runtime/Chaos 계약은 재오픈하지 않았다.

### v0.1.9 - 2026-09-03

- USER 최종 판단으로 Step 3 Socket/Naming UX를 마감했다. 이후 문제 발생 시 별도 재오픈한다.
- Wheel/Hardpoint `추가 후 편집` enable/status는 cached AssetSnapshot이 아니라 current Chassis `UStaticMesh::FindSocket()` live truth를 사용한다. Socket 존재 시 즉시 비활성, 삭제 시 즉시 활성로 통일했다.
- 오른쪽 상단의 대상/현재 Step/상태/요약/지금 할 일을 하나의 고정 프레임으로 묶고 Step 3 `소켓 준비 / Naming`을 remaining-height 내부 ScrollBox로 전환해 Hardpoint 증가가 상단 프레임을 침범하지 않도록 했다.
- Hardpoint row의 generic `삭제`를 `장착 위치만 삭제`로 변경하고 Recipe row만 삭제되며 Chassis StaticMesh Socket은 유지된다는 안내를 표/tooltip/성공 문구에 명시했다. 삭제 로직/no-cascade 계약은 변경하지 않았다.
- stale projection 문서를 교정해 대표 상태를 `Socket/Naming USER PASS / remaining USER Driving re-acceptance pending`으로 갱신했다. P0-07 USER Driving persistence correction은 Editor-only receipt이며 runtime benchmark semantics는 유지한다.
- latest official Editor build `cb9218c7f74847a6908a4b093d2205f3` PASS / Exit Code 0. 직전 broad `CarFight.DataAuthoring`은 100/100 PASS였고 이후 변경은 Slate layout/Socket live status/wording 범위다.

### v0.1.8 - 2026-09-03

- P0-07 USER 피드백으로 동일 Chassis를 여는 `Wheel Socket 편집하기` / `Hardpoint Socket 편집하기`를 단일 `Chassis Socket 편집하기`로 통합했다.
- Wheel/Hardpoint exact SocketName을 read-only text + copy로 노출하고, missing Socket은 USER explicit `추가 후 편집`으로 current Chassis 원점에 exact 이름 1개를 transaction 생성한 뒤 Static Mesh Editor를 연다. 자동 배치/자동 rename/자동 Save는 하지 않으며 shared Chassis 영향 경고를 유지한다.
- 모호한 `기타 조건부 Socket`을 현재 실제 의미인 `파괴 FX Socket (선택)`으로 교정하고 별도 파괴 FX 위치가 필요하지 않으면 생략할 수 있음을 명시했다.
- Wagon UAT에서 USER Driving PASS가 host-local EditorPerProject token에만 묶여 완료 차량이 되감기는 문제를 발견해 persistent `BuilderDrivingAcceptanceReceipt`를 Recipe non-semantic metadata로 추가했다. PASS authority는 exact Target path + DefinitionHash이며 benchmark RunId는 diagnostic-only다. 같은 Definition에서는 새 RunId/Editor 재기동에도 PASS를 유지하고 Target DefinitionHash drift에서만 stale 처리한다.
- USER 보고 official Editor build PASS. 이후 `CarFight.DataAuthoring` broad 100/100 PASS, `VB_P0_09.BuilderStep8Driving`의 production receipt write/local-token-loss resume/same-Target new-RunId 유지 회귀 PASS를 확인했다.
- CF-FQ-041 Runtime Apply / CF-FQ-044 Catalog dirty와 Runtime/Chaos 코드는 비접촉이며 Gate는 VMG-P0-07 USER Re-Acceptance Pending으로 유지한다.

### v0.1.7 - 2026-09-03

- `VMG-P0-06 Focused + Affected Technical Validation`을 PASS로 닫고 next Gate를 `VMG-P0-07 USER Acceptance`로 전진했다.
- A~Y matrix를 P0-02~05 기존 regression과 신규 `VMG_P0_06.ContractMatrix`로 대조했고 F/G/K gap만 보강했다. NoHardpoints intentional-zero Step3/6, out-of-band conflict fail-closed, None/duplicate identity Blocked를 직접 검증한다.
- V/W는 exact pending/current Mesh path, pending FL fallback, `UAssetEditorSubsystem::OpenEditorForAsset`, Step2/3 shared-Chassis warning을 source-path audit + official compile로 Technical PASS 처리하고 실제 창 동작은 P0-07 USER Acceptance에 남겼다.
- 첫 P0-06 broad의 `F Step6 aggregate` 신규 실패는 unrelated Profile source가 비어 있던 test fixture 오류로 판정했다. Builder-private 4 Profile을 연결해 요구사항을 낮추지 않고 Step6 전체 Complete expectation을 유지해 교정했다.
- final official Editor build `b2b78f1042854e7abdd202849e94f03d` PASS / Exit Code 0. broad DataAuthoring 100건 중 98 PASS, FQ-043 P0-02~06 및 affected GameplayGuidance 모두 PASS했다.
- 반복 실패는 기존 `WagonTransmissionDraft`, `Batch.AllowlistProjection` 2건만 유지된다. P0-06 product behavior mutation은 0이고 test-only seam/Automation만 추가했다. CF-FQ-041/044와 Runtime/Asset dirty는 비접촉이며 commit/push는 수행하지 않았다.

### v0.1.6 - 2026-09-03

- `VMG-P0-05 New / Existing Preservation Guards`를 Technical PASS로 닫고 next Gate를 `VMG-P0-06 Focused + Affected Technical Validation`으로 전진했다.
- E1~E11을 전용 `VMG_P0_05.ExistingPreservation` Automation으로 고정했다. LegacyCompatible empty-intent, direct/missing Socket stored transform, custom identity, same-Hardpoint multi-Mount, isolated new Hardpoint merge, exact dependency remove, Target/StaticMesh mutation0, untouched resolved identity를 검증한다.
- official Editor build `df8bb1ccb9c740f58b61af1c4b5c5371` PASS / Exit Code 0. broad DataAuthoring 99건 중 97 PASS이며 FQ-043 P0-02~05와 directly affected GameplayGuidance가 모두 PASS했다.
- 반복 실패는 기존 `WagonTransmissionDraft`, `Batch.AllowlistProjection` 2건만 유지된다. P0-05에서 Product Source 수정은 없고 test coverage만 추가했으며 CF-FQ-041 Runtime Apply / CF-FQ-044 Catalog / Product Asset을 건드리지 않았다.

### v0.1.5 - 2026-09-03

- `VMG-P0-04 Step 6 MountProfile Guided UX`를 Technical PASS로 닫고 next Gate를 `VMG-P0-05 New / Existing Preservation Guards`로 전진했다.
- Standard 1:1 Mount transient draft → explicit typed commit/remove, stable `Mount_<LocationSlotId>`, Recipe/Target collision fail-closed, MountType/SizeLimit, optional EquipmentPreset + `CanUseOnMount` validation을 구현했다.
- Hardpoint/Mount Guidance authority를 CompanionMode에서 persistent `BuilderHardpointPlanMode`로 분리하고 Resolver fingerprint를 변경하지 않는 transient R0 request projection으로 전달했다. LegacyCompatible custom/multi-Mount/Existing preservation은 유지한다.
- 기존 `VB-P0-06 GameplayGuidance` 회귀의 구형 CompanionMode 가정을 `UseHardpoints` / `LegacyCompatible` explicit request로 교정해 PASS를 복구했다.
- 코드감사에서 Step 6 전체를 read-only라고 설명하던 stale 문구를 교정했다. 최종 official Editor build `ad78366e7ebc4217be386a8ed5bc28f2` PASS / Exit Code 0.
- final broad DataAuthoring 98건 중 96 PASS. FQ-043 P0-02/03/04와 directly affected GameplayGuidance는 모두 PASS했고, 반복 실패 2건 `WagonTransmissionDraft`, `Batch.AllowlistProjection`만 유지됐다.
- CF-FQ-041 Runtime Apply dirty, Product StaticMesh/UE Content, Target VehicleData mutation은 0이며 commit/push는 수행하지 않았다.

### v0.1.4 - 2026-09-02

- `VMG-P0-03 Step 3 Hardpoint Planning UX`를 Technical PASS로 닫고 next Gate를 `VMG-P0-04 Step 6 MountProfile Guided UX`로 전진했다.
- explicit Hardpoint Plan Mode UI, Standard category add/remove, Recipe+Target max-used+1 stable identity, exact Socket state, shared Chassis/axis guidance와 Step 3 contextual Wheel/Hardpoint editor entry를 구현했다.
- `HardpointSocketMissing`은 shared Resolver/R3에서 Blocked로 유지하면서 Builder에서만 narrow draft-read로 Step 3 Ready를 표시하도록 분리했다. focused test에서 shared Resolver Blocked와 R3 `ValidationBlocked`를 함께 고정했다.
- 코드감사에서 Legacy `SocketName=None` row가 가짜 `HP_*`를 표시하던 문제를 교정해 stored SocketName truth/direct LocalTransform 보존을 그대로 표시하도록 했다.
- 설계 대조에서 빠져 있던 Step 2 pending Chassis/FL/FR/RL/RR Mesh open을 추가하고 optional FR/RL/RR은 current pending FL fallback을 열도록 보완했다. shared Chassis edit warning도 Step 2에 추가했다.
- 최종 official Editor build `d5f326020bd440428afec63f1927293a` PASS / Exit Code 0. broad DataAuthoring 93건 중 91 PASS, FQ-043 P0-02/P0-03와 직접 affected 회귀는 모두 PASS했다.
- 반복 실패 2건 `WagonTransmissionDraft`, `Batch.AllowlistProjection`은 P0-02 시점부터 동일한 변경 범위 밖 baseline failure이며 FQ-043 blocker로 확대하지 않았다.
- CF-FQ-041 Runtime Apply dirty, Runtime VehicleData, Product StaticMesh/UE Content Asset mutation은 0이며 commit/push는 수행하지 않았다.

### v0.1.3 - 2026-09-02

- VMG-P0-02를 구현하고 Technical PASS로 닫았다. Recipe-owned 4-state Hardpoint Plan Mode, Guided creation opt-in/ProposalHash binding, Builder-owned transaction/readback, typed no-cascade remove를 current Editor-only authoring lane에 연결했다.
- Mode metadata는 RecipeFingerprint/Runtime semantic input에서 제외하고 no-save/no-target/no-StaticMesh-mutation을 유지했다. Mode 변경은 stale prepared Final Review/Apply approval만 폐기하며 완료된 DefinitionApply guarded Undo token은 보존한다.
- RemoveMountIntent/RemoveHardpointIntent는 append-only enum + exact stable deletion identity를 사용하고 referenced Hardpoint는 prospective/persistent 양쪽에서 DependencyConflict로 차단한다.
- 최종 official CarFight_ReEditor build `d3e406482e6f4e6a9adac9d65b781638` PASS / Exit Code 0.
- final broad DataAuthoring 92건 중 90 PASS, VMG-P0-02 및 직접 affected regression 전부 PASS. 반복 실패 2건 `WagonTransmissionDraft`, `Batch.AllowlistProjection`은 변경 범위 밖의 동일 기존 failure로 기록하고 VMG-P0-02 blocker로 확대하지 않았다.
- CF-FQ-041 Runtime Apply dirty, UE Content Asset, Runtime VehicleData, StaticMesh mutation은 0이며 next Gate를 `VMG-P0-03 Step 3 Hardpoint Planning UX`로 전진했다.

### v0.1.2 - 2026-09-02

- VMG-P0-01 post-design audit에서 신규 Guided 차량도 Step 1 Companion 생성 뒤 `DeriveCompanionMode()=CompleteExisting`으로 전환될 수 있음을 확인해 Hardpoint Guidance authority를 CompanionMode에서 분리했다.
- v0.1.1의 `Unspecified / ExplicitNone + derived Configured`를 폐기하고 Recipe-owned `LegacyCompatible / Unspecified / NoHardpoints / UseHardpoints` persistent Builder Plan Mode로 교정했다.
- FQ-043 이전 Recipe는 LegacyCompatible default로 보존하고 Guided 신규 record creation만 additive `bRequireExplicitHardpointPlan=true` → Unspecified를 기록하며 이 flag를 R2 Create ProposalHash에 포함하도록 확정했다.
- Mode 변경은 Builder-owned transaction/readback을 사용하고 Resolver fingerprint에는 포함하지 않되 stale Final Review/Apply approval cache를 무효화하도록 고정했다.
- Standard LocationCategory/LocationSlotId post-create lock, MountProfileId Recipe/Target collision fail-closed, shared Chassis StaticMesh Socket warning, Hardpoint 삭제 뒤 Socket 잔존 안내를 추가했다.
- Step 3/6 completion과 focused scenarios를 Mode 기반으로 전면 교정했으며 Existing E1~E11, no-cascade/no-save/no-Target/no-StaticMesh-mutation 계약은 유지했다.
- Product Source/UE Asset/Runtime mutation은 0이며 VMG-P0-02 구현 착수 전 설계감사 교정을 완료했다.

### v0.1.1 - 2026-09-02

- VMG-P0-01 Detailed UX / State Contract Design Review를 완료하고 Design PASS / VMG-P0-02 Ready로 전진했다.
- HardpointPlan persistent 3-state를 폐기하고 Recipe-owned `Unspecified / ExplicitNone` Builder metadata + HardpointIntents 기반 derived Configured로 교정했다.
- mutable MountType/SizeLimit을 MountProfileId에 넣지 않고 신규 Standard를 `Mount_<LocationSlotId>` creation-time stable suggestion으로 교정했다.
- Current Source에서 Utility Scanner Mount 지원을 확인해 Utility 미지원 가정을 제거하고 Utility + SizeLimit=None current valid contract를 반영했다.
- typed remove는 existing R1 AuthoringWrite lane을 재사용하고 no-cascade / dependency fail-closed / no StaticMesh socket delete로 확정했다.
- Step 2 pending Mesh open, Step 3 contextual Chassis Socket buttons, NewVehicle Ready/Blocked/Complete 규칙과 Step 6 1:1 completion을 확정했다.
- Existing Vehicle stored LocalTransform/custom naming/multi-Mount preservation E1~E11을 필수 회귀로 고정했다.
- 구현, Product Source, UE Asset, Runtime/Chaos mutation은 0이며 CF-FQ-040/041/042 보호 범위를 유지했다.

### v0.1.0 - 2026-09-02

- USER 승인으로 Vehicle Builder 장비 슬롯 / Hardpoint / Mount Guidance UX를 CF-FQ-043 별도 Ready Feature로 정식 승격했다.
- VMG-P0-00 read-only Source/System 감사 결과와 Step 3/Step 6 책임 분리를 기록했다.
- 신규 차량의 Hardpoint 미결정/명시적 0개 구분, USER Socket 배치, Mount 규칙, typed remove semantics, New/Existing preservation을 최소 범위로 고정했다.
- Step 2/3 Chassis/Wheel Mesh 및 Wheel/Hardpoint Socket 작업용 문맥별 Editor 진입 버튼을 포함했다.
- CF-FQ-042 Done / Clean PASS와 CF-FQ-040/041, Runtime/Chaos 보호 경계를 고정했다.
