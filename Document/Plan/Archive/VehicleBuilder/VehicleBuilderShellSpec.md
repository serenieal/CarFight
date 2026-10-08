# CarFight Vehicle Builder Shell Spec

- 문서 버전: v0.1.15
- 작성일: 2026-09-01
- 문서 상태: VB-P0-09 In Progress / Step 1~8 Guided Flow Technical PASS Preserved / Actual Wagon Apply+Save Complete / WSA-P0-07 USER PASS / Wheel Size Authority P0 Complete / Fresh Resolver-Adoption Refresh Pending
- Feature: `CF-FQ-040 Guided Vehicle Builder`
- 상위 계획: `VehicleBuilderPlan.md`
- 적용 Gate: `VB-P0-02 Builder Shell / Step State / Resume` + `VB-P0-03 Mesh & Socket Guidance` + `VB-P0-09 Step 1~8` + `WSA-P0-01~07`
- 선행 계약: `VehicleRefEvidenceSpec.md v0.1.1`
- 하위 계약: `WheelSizeAuthorityPlan.md v0.1.19`
- 역할: Guided Vehicle Builder의 persistent/transient state owner, stable StepId와 definition-driven navigation, blocker/complete state, Editor restart resume identity, read-only context와 후속 Step extension boundary를 고정한다.

---

## 1. 범위와 비범위

이 문서는 VB-P0-01 Reference Evidence schema를 다시 설계하지 않는다.

이번 Gate에서 고정하는 범위:

```text
Builder persistent resume owner
Builder transient session owner
current baseline stable Step identity + definition-driven composition/order
Step state / blocker / warning contract
Back / Next / direct navigation 규칙
Step completion의 derived-state 원칙
Editor restart resume identity와 recovery order
Reference Evidence / Recipe / Target read-only context
stale propagation / refresh policy
VB-P0-03 Mesh-Socket extension point
VB-P0-04 Reference-Proposal extension point
```

VB-P0-02/03 설계 Gate 당시 하지 않았던 것(Historical contract):

```text
C++ class/file 생성 0
UE Asset 생성/수정 0
Recipe schema mutation 0
Evidence schema mutation 0
VehicleData/Profile mutation 0
Reference → CarFight/Chaos numeric mapping 0
Builder typed write operation 구현 0
새 GoPyMCP tool 0
Editor 실행 0
Build/Automation 0
```

---

## 2. 핵심 설계 원칙

Builder의 "진행 상태"를 새 gameplay/authoring truth처럼 저장하지 않는다.

```text
실제 작업 결과의 persistent truth
= Recipe / Evidence / VehicleData / private Profile / Mesh / Socket / Apply State

Builder resume convenience
= local Editor per-project user settings

현재 화면 state / cache / prepared approval
= transient ViewModel
```

따라서 다음 같은 field는 persistent Asset에 만들지 않는다.

```text
bStep1Complete
bStep2Complete
LastCompletedStep
bPhysicsProposalDone
bFinalReviewDone
```

Step Complete/Blocked/Stale은 매 refresh 때 현재 authoritative data에서 다시 계산한다.

이 원칙은 다음 문제를 막는다.

```text
Asset이 바뀌었는데 old Complete=true가 남는 문제
Recipe/Evidence fingerprint가 UI navigation 때문에 변하는 문제
Editor restart 뒤 stale approval을 다시 사용하는 문제
Builder와 Data Authoring이 서로 다른 completion truth를 갖는 문제
```

---

## 3. State owner — 3층 분리

### 3.1 Persistent Authoring / Evidence Truth — 기존 Asset owner

Builder가 완료 여부를 판정할 때 읽는 persistent truth는 기존 owner가 그대로 소유한다.

| Truth | Owner |
| --- | --- |
| Target runtime definition | `UCFVehicleData` |
| Authoring intent / target binding | `UCFVehicleRecipeData` |
| 실차 research/citation/provenance | `UCFVehicleRefEvidence` future asset contract |
| Vehicle-specific numeric authoring | Builder-private Profile 4종 — VB-P0-04 구현 owner. Drivetrain Profile이 Differential + 차량별 Transmission numeric authoring을 소유 |
| Chassis/Wheel source | USER StaticMesh assets. Wheel Mesh Bounds는 실제 원본 치수 Source이며 타이어 크기 디자인 Authority가 아님 |
| Socket state | Chassis StaticMesh socket truth. Wheel Socket Location/Rotation/Scale 중 Scale은 USER 타이어 크기 authoring authority |
| Resolver/Diff/Validation/Apply | existing Data Authoring backend |

Builder Shell은 이 truth를 복제하지 않는다.

### 3.2 Persistent Resume Convenience — Editor-local settings owner

Editor restart 뒤 "어느 차량의 몇 번째 Step을 보고 있었는가"를 되살리는 최소 정보는 **Editor-only per-project user settings**가 소유한다.

미래 권장 identity:

```text
Class: UCFVehicleBuilderSettings
File:  CFVehicleBuilderSettings.h/.cpp
Storage intent: EditorPerProjectUserSettings domain
Cook: 0
Runtime dependency: 0
Git-shared authoring authority: 0
```

파일명은 32자 제한 안이다.

이 설정은 편의 정보이며 삭제되어도 차량 authoring truth가 손실되면 안 된다.
설정 파일이 유실되면 사용자는 Builder Browser에서 기존 Recipe를 다시 선택하면 된다.

### 3.3 Transient Session / UI — Builder VM owner

Editor lifetime 동안만 필요한 모든 cache/approval/navigation presentation은 transient ViewModel이 소유한다.

미래 권장 identity:

```text
FCFVehicleBuilderVM
CFVehicleBuilderVM.h/.cpp
```

다음은 절대 persistent resume 설정에 저장하지 않는다.

```text
Prepared Apply approval
Prepared Profile write approval
BuilderProposal object
ProposalHash를 authorization token처럼 재사용하는 state
Resolve/Diff/Trace/Validation cached rows
Socket scan cache
Runtime benchmark cache
Undo TransactionId
toast/message text
expanded/collapsed widget tree state
```

Editor restart는 이 state를 전부 폐기하고 fresh read로 재구성한다.

---

## 4. Persistent Resume schema

미래 logical struct:

```text
FCFVehicleBuilderResumeState
```

### 4.1 Managed Vehicle resume fields

```text
SchemaRevision
ActiveRecipeId
LastKnownRecipePath
LastKnownDefinitionPath
LastStepId
AcceptedEvidenceId
AcceptedEvidenceFingerprint
```

의미:

| Field | 역할 |
| --- | --- |
| `SchemaRevision` | local resume schema migration |
| `ActiveRecipeId` | managed vehicle의 primary resume identity |
| `LastKnownRecipePath` | rename/move 전 fallback·diagnostic hint |
| `LastKnownDefinitionPath` | target mismatch 진단용 hint |
| `LastStepId` | reopen할 UI 위치. completion authority가 아님 |
| `AcceptedEvidenceId` | USER가 Reference Set을 확인했던 exact Evidence identity |
| `AcceptedEvidenceFingerprint` | Step 1 USER review가 어느 research state에 묶였는지 |

`AcceptedEvidenceFingerprint`는 Definition/Recipe/Profile write approval이 아니다.
Reference Set 확인이라는 **workflow navigation review token**일 뿐이다.
Evidence가 바뀌면 Step 1을 Stale로 만들기 위한 값이다.

### 4.2 Pre-record Draft resume

아직 Recipe/Definition/Evidence가 생성되지 않은 New Vehicle 시작을 Editor restart 뒤 최소 복원하기 위해 local Draft identity를 허용한다.

미래 logical struct:

```text
FCFVehicleBuilderDraftState
```

허용 필드:

```text
DraftId
DraftDisplayName
DraftSegmentIntent
DraftChassisMeshPath
DraftWheelMeshPath
DesiredPackageRoot
LastStepId
```

금지 필드:

```text
Reference claims/citations 전체
derived physics numeric values
VehicleData field overrides
Profile numeric payload
Apply approval
```

Draft settings는 authoring SSOT가 아니다.
VB-P0-05에서 persistent 7-Asset record가 생성되는 순간 managed Recipe resume로 승격하고 Draft는 폐기한다.

P0에서는 **동시에 활성 pre-record Draft 1개만** 지원하는 방향을 사용한다. 여러 draft tab/history manager는 필요가 확인되기 전 추가하지 않는다.

---

## 5. Resume primary identity — RecipeId

현재 Source에서 `UCFVehicleRecipeData::RecipeId`는 persistent GUID이며 일반 Asset duplicate 시 새 ID를 발급한다.
`FCFVehicleListEntry`도 `RecipeId + RecipePath + DefinitionPath`를 이미 반환한다.

따라서 managed Builder Session primary identity는 다음으로 고정한다.

```text
Primary: RecipeId
Secondary: RecipePath
Bound Target diagnostic: DefinitionPath
Evidence binding: Evidence.TargetRecipeId == RecipeId
```

Asset path만 primary identity로 사용하지 않는다.
Rename/move가 있어도 RecipeId가 유지되면 동일 Builder vehicle로 복원할 수 있어야 한다.

---

## 6. Editor restart Resume algorithm

Builder Tab open 시 다음 순서를 사용한다.

### R0 — local resume read

```text
Editor per-project user settings에서 ResumeState 읽기
```

### R1 — RecipeId exact lookup

existing Data Authoring `ListVehicles` 결과에서 `ActiveRecipeId`와 exact match하는 managed row를 찾는다.

판정:

```text
exact 1건
→ resume candidate

0건
→ LastKnownRecipePath fallback lookup

2건 이상
→ identity ambiguity BLOCK
→ 임의 선택 금지
```

Recipe duplicate는 새 RecipeId를 받아야 하는 current contract이므로 같은 RecipeId 다중 발견은 정상 상태로 취급하지 않는다.

### R2 — Recipe / Target binding verification

candidate Recipe를 fresh load/read하고 다음을 확인한다.

```text
Recipe.RecipeId == ActiveRecipeId
Recipe.TargetVehicleData path == resolved DefinitionPath
Target VehicleData가 실제 resolve 가능
```

path hint가 오래됐지만 RecipeId exact candidate가 정상이고 current Target binding도 valid하면 local setting의 LastKnown path는 새 값으로 갱신해도 된다.
이것은 local convenience update이며 Product Asset mutation이 아니다.

### R3 — Evidence binding verification

Reference Evidence를 읽어 다음을 확인한다.

```text
Evidence.TargetRecipeId == Recipe.RecipeId
Evidence.TargetRecipePath == current Recipe path 또는 rename migration으로 current binding 복구 가능
Evidence.TargetDefinitionPath == current target path 또는 current binding과 의미적으로 동일
```

Evidence가 없거나 binding이 맞지 않으면 Step 1/후속 dependent Step은 Block/Stale이다.
임의의 다른 Evidence를 자동 선택하지 않는다.

### R4 — transient context rebuild

resume 설정에서 cached completion을 복원하지 않는다.

```text
fresh Recipe/Target context
fresh Evidence identity/fingerprint
current Asset bindings
필요한 Step provider read
```

로 current Step Definition 목록에 존재하는 Step projection을 전부 다시 계산한다. Step 개수는 persistent contract가 아니다.

### R5 — reopen Step selection

`LastStepId`가 현재 unlocked이면 그대로 연다.

locked/unavailable이면:

```text
가장 가까운 이전 actionable Step
```

으로 안전하게 이동하고 이유를 표시한다.

---

## 7. Stable Step identity + definition-driven composition

현재 P0 USER Acceptance baseline은 아래 8개 StepId다. **8이라는 개수와 표시 순서는 persistent authority가 아니다.** StepId만 semantic identity로 안정적으로 유지한다.

| StepId | 현재 표시 순서/이름 | 주요 persistent truth | 구현 owner |
| --- | --- | --- | --- |
| `IdentityReference` | 1. 차량 / Reference | Recipe identity + Evidence + review fingerprint | P0-01/P0-05 |
| `MeshPrep` | 2. Mesh 준비 | Recipe AssetIntent + Mesh assets | P0-03/P0-05 |
| `SocketGuide` | 3. 소켓 준비 / Naming | Chassis sockets + naming contract | P0-03 |
| `LayoutCapture` | 4. Layout Capture | Recipe layout/adoption + current socket/asset state | P0-03/P0-05 |
| `PhysicsProposal` | 5. Physics Proposal | Evidence-bound Builder Proposal/private Profiles | P0-04 |
| `GameplaySetup` | 6. Gameplay Setup | Recipe defaults/hardpoint/fitting intent | P0-06 |
| `FinalReview` | 7. Final Review / Apply | Resolver/Diff/Validation/AppliedState | P0-07 |
| `DrivingTest` | 8. Driving Test | current runtime benchmark evidence | P0-08/P0-09 |

Current implementation contract:

```text
semantic identity
= ECFVehicleBuilderStepId

current composition / order / title
= GetVehicleBuilderStepDefinitions() 단일 owner

StepNumber
= current definition array index + 1 presentation value
= semantic identity 아님

Step state evaluation
= Step별 Evaluate*Step 함수
= current definition 순서를 순회하며 EvaluateStepById(StepId)로 dispatch

state write / lookup
= stable StepId 기반 FindStepView / SetStep

Slate navigation
= StepViews.Num() 기반
= literal 8 금지

Step-specific UI visibility/action
= CurrentStepIndex 숫자 의미 비교 금지
= Current StepId 비교 사용
```

### 7.1 추가 / 제거 / 순서 변경 규칙

새 semantic Step 추가:

```text
1. 새 ECFVehicleBuilderStepId 추가
2. GetVehicleBuilderStepDefinitions()에 현재 표시 위치/제목 1행 추가
3. 해당 Step evaluator 추가
4. 실제 전용 UI가 필요하면 StepId 기반 content/action 추가
5. StepId 기반 focused test 추가
```

기존 Step 순서 변경:

```text
GetVehicleBuilderStepDefinitions() 행 순서만 변경
→ StepNumber / navigation 순서는 자동 파생
→ evaluator dispatch 순서도 같은 definition 순서에서 자동 파생
→ evaluator identity / resume semantic identity는 유지
```

기존 Step 제거/통합:

```text
current definition 목록에서 제외 가능
과거 resume/log 호환성을 위해 기존 enum StepId를 다른 의미로 재사용하지 않음
removed LastStepId resume fallback은 nearest current actionable Step으로 처리
```

기존 Step 내부 규칙 수정:

```text
해당 Evaluate*Step 함수와 해당 focused test를 우선 수정
다른 Step evaluator / navigation을 불필요하게 수정하지 않음
```

P0에서 하지 않는 과설계:

```text
generic plugin Step registry 0
Blueprint-extensible workflow engine 0
Step DataAsset schema 0
persistent progress graph 0
새 public MCP tool 0
```

따라서 현재 baseline은 8단계지만 향후 차량 제작 요구가 늘어날 때 Step 추가·제거·재배치가 fixed-index 전역 수정으로 번지지 않게 한다.

### 7.2 Current Step 2 / Step 3 USER 작업 UX

실제 Wagon E2E에서 확인한 Shell 작업 계약:

```text
Step 2 Mesh 준비
= Chassis StaticMesh picker [필수]
= WheelMeshFL picker [필수]
= WheelMeshFR/RL/RR picker [선택]
= optional 3종 비움 → 기존 FL reuse semantics
= picker selection은 local pending only
= [Mesh 설정 반영] explicit action → existing typed CommitAssetIntent Recipe-only commit
= Target VehicleData Apply 0 / auto Save 0 / wheel auto-detection 0

Step 3 소켓 준비 / Naming
= current Recipe effective FL/FR/RL/RR Socket exact 이름 표시
= 4개 모두 current Chassis 존재 여부 표시
= 각 이름 개별 Clipboard copy
= current Recipe가 실제 요구하는 Hardpoint / Destroyed FX optional Socket만 별도 안내
= optional Socket 이름 일괄 copy
= [차체 메시 열기] → current Chassis Static Mesh Editor open only
= Socket create/move/scale/save는 USER authority
```

Static Mesh Socket의 `Preview Mesh`는 위치·스케일을 시각적으로 맞추기 위한 Editor 보조 표시일 뿐 `Recipe.AssetIntent.WheelMesh*` binding이 아니다. Builder는 이 둘을 같은 값으로 간주하지 않는다.

Step 2 typed commit regression은 `WheelMeshFL` missing에서 `Blocked`, valid FL 복원에서 `Complete`, 두 경우 Target VehicleData hash 불변을 검증한다.

---

## 8. Step state contract

미래 enum 의미:

```text
Unavailable
Locked
Ready
Complete
Blocked
Stale
```

### Unavailable

후속 capability가 아직 구현/등록되지 않은 상태다.

예:

```text
VB-P0-02 Shell은 존재하지만 VB-P0-04 Proposal provider가 아직 없음
→ PhysicsProposal = Unavailable
```

사용자 데이터 오류와 기능 미구현을 같은 Blocked로 표현하지 않는다.

### Locked

현재 capability는 존재하지만 선행 Step 조건이 아직 만족되지 않았다.

예:

```text
SocketGuide 미완료
→ LayoutCapture Locked
```

### Ready

Step에 진입할 수 있고 현재 해야 할 작업이 남아 있다.

`Ready`가 실질적인 In-Progress 상태 역할도 담당한다. 별도 `InProgress` enum은 만들지 않는다.

### Complete

현재 authoritative truth를 fresh evaluation한 결과 Step completion contract를 만족한다.

`Complete`는 persistent boolean이 아니다.

### Blocked

해당 Step을 수행하려 했지만 hard blocker가 존재한다.
Next는 비활성화한다.

### Stale

과거 review/completion에 사용된 dependency fingerprint 또는 binding이 바뀌어 재검토가 필요한 상태다.

Stale은 "실패"와 다르다.
사용자에게 무엇이 변경됐는지와 어떤 Step부터 다시 확인해야 하는지 보여준다.

---

## 9. Step result schema

미래 logical struct:

```text
FCFVehicleBuilderStepResult
```

필드:

```text
StepId
State
Title
ShortSummary
Blockers[]
Warnings[]
DependencyStamp
LastEvaluatedRevision
```

`LastEvaluatedRevision`은 transient diagnostic이며 persistent completion authority가 아니다.

### Blocker / Warning item

미래 logical struct:

```text
FCFVehicleBuilderIssue
```

필드:

```text
IssueCode
Severity = Block / Warning / Info
Owner = Builder / Evidence / Recipe / Target / Asset / DataAuthoring / Runtime
Summary
ResolutionText
RelatedObjectPath
RelatedFactKey
```

기본 UX:

```text
문제: Wheel_Anchor_RR Socket이 없습니다.
해결: Chassis Mesh Editor에서 정확히 `Wheel_Anchor_RR` 이름으로 Socket을 만든 뒤 [다시 검사]를 누르세요.
```

Builder Shell은 low-level error code만 사용자에게 노출하지 않는다.

---

## 10. Navigation contract

### 10.1 Back

```text
현재 Step > Step 1이면 항상 이전 Step으로 이동 가능
```

Back은 authoring mutation/Undo가 아니다.
단순 UI navigation이다.

### 10.2 Next

Next 클릭 시 cached `Complete`를 바로 믿지 않는다.

```text
RefreshCurrentStep(read-only)
→ Blocker/Stale 재평가
→ Complete이면 Next
→ 아니면 현재 Step 유지 + 해결 작업 표시
```

`Warning`만 존재하고 completion contract가 만족되면 Next를 허용한다.
`Block` 또는 `Stale`이면 Next를 차단한다.

### 10.3 Step 직접 클릭

혼란을 줄이기 위해 P0에서는 다음만 허용한다.

```text
과거 Complete Step으로 이동 O
현재 Step O
현재 Step 바로 다음의 unlocked Step O
그보다 먼 future Step X
```

Advanced workspace처럼 임의의 8개 페이지를 자유 왕복하는 구조로 만들지 않는다.

### 10.4 LastStep persist

Step 이동 성공 뒤 `LastStepId`를 local settings에 기록한다.

이 기록은 save/apply authoring transaction이 아니며 Vehicle Asset dirty를 만들지 않아야 한다.

---

## 11. Dependency / Stale propagation

단순히 "앞 Step이 바뀌면 뒤 Step 전부 초기화"하지 않는다.
각 Step이 실제 의존하는 fingerprint/binding만 사용한다.

P0 dependency 방향:

```text
IdentityReference
→ MeshPrep
→ SocketGuide
→ LayoutCapture

IdentityReference + LayoutCapture
→ PhysicsProposal

SocketGuide + LayoutCapture
→ GameplaySetup

PhysicsProposal + GameplaySetup
→ FinalReview

FinalReview
→ DrivingTest
```

세부 dependency 예:

### Evidence 변경

```text
AcceptedEvidenceFingerprint != current EvidenceFingerprint
→ IdentityReference Stale
→ MeshPrep: dimensions-based sanity가 달라지면 Stale
→ LayoutCapture: Reference Wheelbase/Track comparison이 달라지면 Stale
→ PhysicsProposal Stale
→ FinalReview Stale
→ DrivingTest Stale
```

Socket 이름 자체가 Evidence와 무관하면 SocketGuide의 socket-existence result까지 무조건 무효화하지 않는다.

### Chassis Mesh/path/socket 변경

```text
MeshPrep 재평가
SocketGuide 재평가
LayoutCapture Stale
PhysicsProposal 중 wheel/layout input을 소비했다면 Stale
GameplaySetup의 hardpoint binding이 영향 받으면 Stale
FinalReview/DrivingTest Stale
```

### Recipe semantic change

`RecipeFingerprint`를 소비한 후속 Proposal/Review는 Stale 처리한다.

### Target raw external change

Target hash/External Drift를 소비하는 FinalReview는 Stale/Blocked 처리하고 기존 Data Authoring Drift contract를 사용한다.

---

## 12. Refresh policy — polling 금지

Builder는 background tick에서 매 프레임 Asset/Resolver를 검사하지 않는다.

fresh read trigger:

```text
Builder Tab open/resume
Vehicle/Draft selection 변경
Step 진입
Next 클릭
사용자 [다시 검사] 클릭
Builder-owned mutation 성공 직후
외부 Asset Editor에서 돌아와 Builder Tab이 다시 활성화될 때 필요한 current Step refresh
```

전역 Asset Registry/property changed listener를 무분별하게 연결하거나 full Resolve를 매 UI frame 실행하지 않는다.

Next는 항상 fresh prerequisite read를 거치므로 외부 Mesh/Socket 변경을 old cached PASS로 통과시키지 않는다.

---

## 13. Read-only Builder Context

P0-02 Shell이 읽는 context는 **proposal을 생성하거나 Asset을 변경하지 않는다.**

미래 logical struct:

```text
FCFVehicleBuilderReadContext
```

### 13.1 Resume / Identity section

```text
SessionKind = None / Draft / ManagedRecipe
DraftId
RecipeId
RecipePath
DefinitionPath
```

### 13.2 Evidence section

```text
EvidencePath
EvidenceId
EvidenceFingerprint
EvidenceTargetRecipeId
EvidenceTargetDefinitionPath
AcceptedEvidenceFingerprint
BlockingConflictCount
UnknownFactCount
```

Evidence의 claim 값을 CarFight VehicleData field로 mapping하는 table은 P0-02 context에 넣지 않는다.

### 13.3 Recipe / Target section

existing Data Authoring read surface를 재사용한다.

```text
ManageState
RecipeFingerprint
TargetDefinitionHash
ResolverContractRevision
ProfileBindings
AppliedDefinitionHash
ExternalDrift summary
```

가능한 source:

```text
ListVehicles
ReadVehicleContext
ResolveVehiclePreview의 immutable snapshot metadata
```

Shell이 Resolver/Validator를 직접 구현하지 않는다.

### 13.4 Asset section

```text
ChassisMeshPath
WheelMeshPaths
current AssetIntent summary
```

Socket/layout deep evidence는 P0-03 provider가 추가한다.

### 13.5 Proposal section — P0-04 extension slot

P0-02에서는 비워 둔다.

```text
ProposalProviderAvailable = false
ProposalSummary = none
```

Reference claim→Chaos mapping을 여기서 계산하지 않는다.

Transmission 요구사항도 동일하다.

```text
Shell read dependency
= Evidence에 Transmission 관련 claim/Unknown/conflict가 존재할 수 있음
= Recipe의 Drivetrain Profile binding과 향후 ProfileFingerprint를 Proposal dependency로 소비

Shell 계산 책임
= gear ratio 계산 X
= shift RPM 추론 X
= TransmissionSetup field mapping X
= fake current gear 계산 X
```

Evidence 또는 Builder-private Drivetrain Profile fingerprint가 바뀌어 기존 Physics Proposal의 transmission 근거가 달라지면 `PhysicsProposal`과 그 후속 `FinalReview / DrivingTest`는 dependency-specific Stale 대상이다. exact stale stamp 구성은 P0-04 provider가 확정한다.

### 13.6 Gameplay / Runtime sections

P0-06/P0-08 provider가 후속으로 채운다.
P0-02 core는 fixed slot만 알고 계산 책임을 갖지 않는다.

---

## 14. Read context freshness tiers

불필요하게 모든 Step에서 full Resolve를 반복하지 않도록 read를 두 층으로 나눈다.

### Lightweight Resume Context

Tab open과 navigation에 사용:

```text
ListVehicles row
RecipeId / paths
Evidence binding + fingerprint
Recipe AssetIntent/ProfileBindings summary
```

### Step-specific Deep Context

필요할 때만 수행:

```text
Mesh/Socket inspection     → P0-03
Layout capture evidence    → P0-03/P0-05
Proposal binding/fingerprint → P0-04
Resolve/Diff/Validation    → P0-07
Runtime benchmark          → P0-08
```

Shell open 자체가 무거운 full technical validation을 자동 실행하는 구조를 피한다.

---

## 15. Step-specific completion boundary

P0-02에서는 후속 기능의 세부 알고리즘을 구현하지 않지만, Shell이 기대하는 completion 의미는 고정한다.

### Step 1 — Identity & Reference

Complete 최소 조건:

```text
managed Recipe identity valid
Target binding valid
Evidence binding valid
Evidence blocking conflict 0
USER가 exact current EvidenceFingerprint를 Reference Set으로 review함
```

`AcceptedEvidenceFingerprint != current EvidenceFingerprint`이면 Stale.

### Step 2 — Mesh Prep

기존 P0-03 evaluator Technical PASS는 보존한다. Wheel Size Authority 구현 뒤 additive하게 다음을 확인한다.

```text
Chassis Mesh valid
기준 Wheel Mesh valid
FR/RL/RR가 비어 있으면 FL common/default fallback 가능
resolved Wheel Mesh Bounds valid
Default Wheel radial X/Z canonical 100 cm 규격 확인
Bounds Center / Origin sanity
asset intent와 현재 selection 일치
```

중요:

```text
Bounds = 메시의 실제 원본 치수 Source
Bounds → 적절한 타이어 Scale 자동 결정 = 금지
```

현재 shared Wheel_FL은 live Bounds `99.9990 × 24.9992 × 99.9990cm`, center≈0으로 canonical `100×25×100` PASS다. Step 2 구현은 이 canonical condition을 일반 SocketScaleFromChassis Wheel Mesh에 검증하도록 확장한다.

### Step 3 — Socket Guide

기존 4-role found/distinct Technical PASS는 보존한다. Wheel Size Authority 구현 뒤 다음을 additive하게 확인한다.

```text
필수 Wheel Socket 4/4
이름 정확
중복/누락 없음
Socket Scale finite / 각 축 > 0
Scale.X == Scale.Z 원칙
Scale.X/Z = 타이어 직경 배율
Scale.Y = 타이어 폭 배율
선택 기능의 required socket blocker 없음
```

USER가 실제 휠하우스를 보면서 Scale을 결정한다. Builder/AI가 적정 Scale을 자동 선택하지 않는다.
Hardpoint 자체의 gameplay requirement는 P0-06과 분리한다.

### Step 4 — Layout Capture

current chassis/socket truth와 capture 결과가 동일 dependency stamp에 묶여 있어야 한다.
Wheel Size Authority 구현 뒤 equality/stale dependency에는 Location/Rotation뿐 아니라 **Socket Scale + resolved Wheel Mesh identity/Bounds**도 포함한다.

```text
Socket RelativeScale
+ resolved Wheel Mesh Bounds
→ per-wheel final visual size
→ Front/Rear Radius/Width derived persisted candidate
```

좌우 axle derived size가 다르면 평균/자동 교정하지 않고 Block한다.
Mesh/Socket/Scale/Bounds/size mode가 바뀌면 old capture를 Complete로 유지하지 않는다.

### Step 5 — Physics Proposal

P0-04 provider owner.

Complete의 의미는 Definition Apply 완료가 아니라:

```text
Evidence-bound typed authoring proposal이 reviewed current authoring sources에 반영됨
+ Drivetrain proposal에는 요구되는 차량별 Transmission authoring scope가 포함됨
+ consumed fingerprints fresh
```

Wheel Size Authority 추가 규칙:

```text
bUseWheelSocketScale=true
→ FrontWheelRadius / RearWheelRadius / FrontWheelWidth / RearWheelWidth는
   USER Socket Scale + actual Wheel Mesh Bounds에서 나온 derived authoring field
→ AI Physics Proposal이 독립 Reference 값으로 덮어쓰지 않음
→ Reference Tire Size는 sanity/reference 비교만 제공
```

Target Definition Apply는 Step 7 owner다.

### Step 6 — Gameplay Setup

P0-06 provider owner.
Project default/explicit None이 valid한 항목은 불필요한 Asset 생성을 요구하지 않는다.

### Step 7 — Final Review / Apply

P0-07 owner.

```text
fresh Resolve/Diff/Validation
blocking issue 0
External Drift policy satisfied
explicit Apply 완료
post-apply current truth가 expected state와 동기화
```

### Step 8 — Driving Test

P0-08/P0-09 owner.

Current runtime contract:

```text
prerequisite
= Step 7 current Complete

saved-state
= Recipe + Target VehicleData disk-saved
= Dirty이면 Ready/Stale 안내 후 benchmark launch 차단
= Builder auto Save 0

Technical authority
= existing RunBuilderBench.ps1
= VB-P0-08 fixed 60Hz fresh PIE actual Chaos
= Target VehicleData exact path
+ current Target DefinitionHash
+ fresh Benchmark RunId

Technical result
= path/hash/RunId exact current binding일 때만 인정
= metric은 관측값 표시만 수행
= Reference threshold assertion 0
= USER driving feel assertion 0
= 0→100 미도달 같은 unavailable branch를 임의 technical failure로 승격하지 않음

USER test-drive
= active PIE 필요
= current selected saved VehicleData transient duplicate
= Player CFVehiclePawn runtime에만 적용
= persistent Asset/Recipe/Map/Config mutation 0
= 실제 transient apply 성공 전 USER PASS 차단

USER acceptance
= explicit USER confirmation
= RecipeId + Target DefinitionHash + Benchmark RunId local token
= exact current truth와 모두 일치할 때 Step 8 Complete
= new RunId 또는 Target drift → old PASS invalid

Gear
= HUD/Runtime actual Chaos GetCurrentGear() authority 유지
= Builder 별도 gear state 0
```

---

## 16. USER Reference review token

Step 1의 `이 Reference Set으로 진행`은 일반 Definition Apply approval과 분리한다.

review token:

```text
EvidenceId
EvidenceFingerprint
RecipeId
```

local Builder settings에 보존할 수 있다.

다음이 하나라도 달라지면 token은 자동으로 invalid다.

```text
EvidenceId
EvidenceFingerprint
RecipeId
```

이 token은 다음 권한을 주지 않는다.

```text
Profile write 권한 X
Recipe write 권한 X
VehicleData Apply 권한 X
Save 권한 X
```

따라서 Editor restart 뒤 Reference review는 이어갈 수 있지만 old mutation approval은 절대 부활하지 않는다.

---

## 17. Resume failure / recovery UX

### Recipe를 못 찾음

```text
"이전에 작업하던 Recipe를 찾을 수 없습니다. 차량 목록에서 다시 선택하세요. 차량 데이터는 자동 삭제되지 않았습니다."
```

### RecipeId duplicate

```text
BLOCK
"같은 Recipe ID를 가진 Asset이 둘 이상 발견됐습니다. 자동 선택하지 않습니다."
```

### Target path 변경

RecipeId exact match + Recipe current Target valid이면 local path hint만 갱신한다.

### Evidence missing

```text
Step 1 Blocked
"이 차량의 Reference Evidence가 없습니다. Reference 단계에서 Evidence를 연결하거나 생성해야 합니다."
```

### Evidence changed

```text
Step 1 Stale
"Reference 자료가 마지막 확인 이후 변경됐습니다. 변경된 근거를 다시 검토하세요."
```

### Prepared Apply existed before shutdown

복원하지 않는다.

```text
"이전 세션의 적용 승인은 안전을 위해 폐기되었습니다. Final Review에서 다시 검토하세요."
```

---

## 18. Builder Shell UI contract

기본 구조:

```text
┌ 차량 / Resume identity ┐
│ 1 차량·Reference       │
│ 2 Mesh 준비            │
│ 3 Socket Guide         │
│ 4 Layout Capture       │
│ 5 Physics Proposal     │
│ 6 Gameplay Setup       │
│ 7 Final Review         │
│ 8 Driving Test         │
├────────────────────────┤
│ 현재 Step Content      │
│                        │
│ 문제 / 경고 / 해결법   │
├────────────────────────┤
│ Back      Refresh Next │
└────────────────────────┘
```

P0 원칙:

```text
모든 Step content 동시 노출 금지
raw VehicleData grid 기본 노출 금지
Advanced details 기본 접힘
Blocker는 원인 + 해결 작업을 한글로 표시
Refresh는 read-only
Next는 fresh evaluation 뒤에만 이동
```

현재 Step의 핵심 행동은 가능하면 1개 primary action으로 제한한다.

---

## 19. Extension boundary — generic plugin registry는 만들지 않음

8 Step은 이미 고정돼 있으므로 동적 plugin/provider registry까지 만들 필요가 없다.

P0에서는 Builder VM의 typed fixed extension slot을 사용한다.

개념적 구조:

```text
BuilderCore
- Resume / navigation / common issues
- IdentityReference evaluator
- fixed Step descriptors

MeshSocketContext        // P0-03
PhysicsProposalContext   // P0-04
GameplaySetupContext     // P0-06
FinalReviewContext       // P0-07
DrivingTestContext       // P0-08
```

각 후속 Gate는 자기 context/evaluator만 채운다.
Shell core에 Reference→Chaos 계산이나 runtime benchmark 계산을 넣지 않는다.

---

## 20. Future C++ ownership proposal — 구현 아님

구현 전 예상 파일 identity:

```text
CFVehicleBuilderTypes.h/.cpp
CFVehicleBuilderSettings.h/.cpp
CFVehicleBuilderVM.h/.cpp
CFVehicleBuilderTab.h/.cpp
```

모두 파일명 32자 제한 안이다.

책임:

```text
CFVehicleBuilderTypes
= StepId / StepState / Issue / Context POD

CFVehicleBuilderSettings
= local per-project resume pointer only

CFVehicleBuilderVM
= transient context / navigation / derived evaluation

CFVehicleBuilderTab
= Slate presentation + USER actions
```

Runtime module/Blueprint에는 Builder state를 추가하지 않는다.

---

## 21. Design dry-run — Editor restart / stale scenarios

실제 구현 없이 state contract를 시나리오로 검증했다.

### Case A — Step 3에서 정상 Editor 종료 후 재시작

전제:

```text
RecipeId = R1
EvidenceFingerprint = E1
AcceptedEvidenceFingerprint = E1
LastStep = SocketGuide
```

재시작:

```text
ListVehicles에서 R1 exact 1건
Recipe.Target valid
Evidence.TargetRecipeId = R1
current EvidenceFingerprint = E1
→ Step 1 Complete
→ Step 2 fresh evaluator 결과에 따라 Complete/Ready
→ Step 3 reopen 가능
```

PASS.

### Case B — Recipe Asset rename/move

```text
RecipeId = 동일 R1
LastKnownRecipePath = old path
ListVehicles current row = new path / R1
```

RecipeId exact match를 우선하므로 동일 vehicle로 복원하고 local path hint만 교정한다.

PASS.

### Case C — Evidence 수정 후 재시작

```text
accepted = E1
current = E2
```

결과:

```text
IdentityReference = Stale
PhysicsProposal/FinalReview/DrivingTest dependent state = Stale 또는 Locked
old Apply/Proposal approval = transient이므로 없음
```

PASS.

### Case D — Target raw external edit 후 재시작

Recipe/Evidence identity는 맞더라도 current Target hash/Drift state를 fresh read한다.
Final Review가 old Complete boolean으로 통과하지 않는다.

PASS.

### Case E — RecipeId duplicate 비정상 상태

ListVehicles exact match 2건이면 자동 path heuristic 선택을 하지 않고 Block한다.

PASS.

### Case F — Editor 종료 직전 Prepared Apply 존재

Prepared approval은 transient VM owner이므로 restart 뒤 0건이다.
Final Review에서 다시 fresh Preview/approval을 요구한다.

PASS.

### Case G — local settings 삭제

차량 Asset truth는 손실되지 않는다.
Builder Browser에서 Recipe를 다시 선택하면 current Step state를 fresh derive한다.

PASS.

---

## 22. VB-P0-02 판정

**PASS — Design / Shell-State-Resume Contract**

고정 결과:

```text
1. Builder completion truth는 별도 persistent boolean으로 저장하지 않는다.
2. Actual truth는 Recipe/Evidence/Target/Profile/Mesh/Socket/Apply owner에서 fresh derive한다.
3. Editor restart resume convenience는 local per-project Builder settings가 소유한다.
4. Managed resume primary identity는 RecipeId다.
5. RecipePath/DefinitionPath는 fallback·diagnostic hint다.
6. Reference USER review는 exact EvidenceId/Fingerprint/RecipeId에 binding된 navigation token이다.
7. Prepared mutation approval/cache/Undo identity는 전부 transient이며 restart 뒤 폐기한다.
8. Step state는 Unavailable/Locked/Ready/Complete/Blocked/Stale 6개로 고정한다.
9. Next는 fresh current-step evaluation을 다시 수행한 뒤에만 허용한다.
10. 8 Step은 fixed sequence이며 후속 기능은 typed fixed extension slot으로 연결한다.
11. generic plugin registry나 background polling은 만들지 않는다.
12. VB-P0-02는 Reference→Chaos numeric mapping이나 typed write를 소유하지 않는다.
```

다음 Gate:

```text
VB-P0-03 Mesh & Socket Guidance
```

VB-P0-03은 이 Shell contract의 `MeshPrep / SocketGuide / LayoutCapture` read/evaluation slot만 구체화한다.
`PhysicsProposal` mapping/typed commit은 계속 `VB-P0-04` owner다.

---

## 22.1 VB-P0-03 Mesh / Socket / Layout read-only evaluator contract

VB-P0-03은 새 Asset scanner나 병렬 Validator를 만들지 않는다. current Data Authoring의 `FCFVehicleAssetReader`가 제공하는 immutable `FCFVehicleAssetSnapshot`과 Recipe/Target readback을 Builder workflow 관점에서 평가한다.

### 22.1.1 Authority와 read source

```text
Recipe Asset intent
= ChassisMesh / WheelMeshFL·FR·RL·RR
+ BodyWheelSocketFL·FR·RL·RR
+ HardpointIntents의 LocationSlotId / LocationCategory / SocketName

Asset truth
= FCFVehicleAssetSnapshot
  - ChassisObjectPath / bChassisLoaded
  - ChassisSockets[]
  - ChassisLayoutFingerprint
  - WheelFL/FR/RL/RR ObjectPath / bAssetLoaded / Bounds / MeasureFingerprint

Persisted capture truth
= current Target VehicleData의 VehicleLayoutConfig
+ materialized HardpointSlots 중 현재 Recipe intent와 대응되는 항목

Reference display input
= accepted Evidence의 Wheelbase / Front·Rear Track 등 존재하는 dimension fact
```

`FCFVehicleAssetReader`가 이미 requested socket name을 dedupe/sort하고 found state와 local transform을 읽으므로 Builder 전용 UObject traversal, Asset Registry scan, Mesh surface inference를 추가하지 않는다.

### 22.1.2 P0-03 deep context — transient only

미래 logical context는 다음 의미만 필요하다. persistent schema로 저장하지 않는다.

```text
MeshPrepContext
- ChassisObjectPath / Loaded
- Wheel role별 ObjectPath / Loaded / Bounds / MeasureFingerprint
- ManualAxisReviewRequired
- available Reference dimensions summary

SocketGuideContext
- Wheel role FL/FR/RL/RR → resolved socket name
- role별 Found + local Transform
- pairwise duplicate binding result
- Recipe Hardpoint intent identity/socket summary
- ChassisLayoutFingerprint

LayoutCaptureContext
- CapturePrerequisitesReady
- expected wheel socket names/transforms from current AssetSnapshot
- current persisted VehicleLayoutConfig readback
- corresponding socket-bound Hardpoint readback if materialized
- CaptureMatchState = NotCaptured / Current / Mismatch
```

`DependencyStamp`는 위 current facts에서 transient하게 계산한다. 별도 `LastCaptureFingerprint`나 Step completion boolean을 Recipe/VehicleData에 추가하지 않는다.

### 22.1.3 MeshPrep evaluator

Hard blocker:

```text
Mesh.ChassisMissing
= Recipe ChassisMesh ref 없음

Mesh.ChassisUnresolved
= non-empty Chassis path가 UStaticMesh로 resolve되지 않음

Mesh.PrimaryWheelMissing
= current backend가 최소 기준 Wheel로 요구하는 WheelMeshFL ref 없음/resolve 실패

Mesh.InvalidWheelBounds
= 지정된 Wheel Mesh의 local bounds가 non-finite 또는 사실상 usable dimension 0
```

Warning / Info:

```text
Mesh.OptionalWheelMissing
= FR/RL/RR ref 중 일부 없음
= current Validator의 FL 재사용 가능 warning semantics를 보존
= P0-03에서 새 wheel-reuse persistent schema를 만들지 않음

Mesh.ForwardAxisNeedsUserReview
= +X 전방은 제작 contract이나 current AssetSnapshot만으로 mesh 시각 방향을 확정하지 않음

Mesh.ReferenceScaleReview
= Reference 전장/전폭/전고가 있으면 USER 수동 확인 항목으로 표시
= current AssetSnapshot에는 Chassis bounds가 없으므로 자동 Chassis scale PASS를 주장하지 않음
```

`Wheel MeasureFingerprint`와 Bounds는 "Asset이 바뀌었는가 / 명백히 비정상인가"까지만 사용한다. 이를 WheelRadius, Width, Suspension 등 authoring numeric으로 변환하지 않는다.

MeshPrep `Complete`:

```text
Chassis loaded
+ WheelMeshFL loaded
+ 지정된 다른 Wheel ref는 있으면 모두 resolve 가능
+ 지정 Wheel bounds usable
+ hard blocker 0
```

수동 +X/Reference scale review는 안내 Warning이며 P0-03 Next를 막지 않는다.

### 22.1.4 SocketGuide evaluator

Wheel socket correctness는 "기본 이름을 강제"가 아니라 **4개 역할 binding의 유효성**이다.

```text
Role FL → configured name, None이면 Wheel_Anchor_FL
Role FR → configured name, None이면 Wheel_Anchor_FR
Role RL → configured name, None이면 Wheel_Anchor_RL
Role RR → configured name, None이면 Wheel_Anchor_RR
```

Hard blocker:

```text
Socket.WheelMissing
= 역할 하나라도 resolved socket이 current Chassis에 없음

Socket.WheelRoleDuplicate
= FL/FR/RL/RR 중 둘 이상이 같은 resolved socket name을 공유
```

Info:

```text
Socket.CustomWheelName
= socket은 존재하지만 기본 Wheel_Anchor_FL/FR/RL/RR 이름과 다름
= current UCFVDAValidator semantics대로 허용
```

4/4 socket이 존재하면 다음 read-only sanity를 계산해 표시할 수 있다.

```text
AuthoredWheelbaseCm = abs(FrontAxleMid.X - RearAxleMid.X)
FrontTrackCm        = abs(FR.Y - FL.Y)
RearTrackCm         = abs(RR.Y - RL.Y)
```

`+X 전방` contract에 따라 front/rear와 left/right role topology가 명백히 뒤집힌 경우 `Socket.RoleTopologySuspicious` Warning을 낸다. Reference Wheelbase/Track fact가 있으면 current 값과 Delta/Delta%를 보여주지만 **P0에서는 임의 퍼센트 threshold로 Block하지 않는다.** 아트 비율 차이는 USER review 대상이다.

### 22.1.5 Hardpoint / Destroyed FX guidance boundary

Hardpoint requiredness를 P0-03이 추론하지 않는다. P0-06 Gameplay Setup이 어떤 기능/slot이 실제 필요한지를 소유한다.

현재 Recipe에 선언된 Hardpoint intent는 guidance 대상으로만 검사한다.

```text
Hardpoint.EmptyLocationSlotId     → Warning
Hardpoint.DuplicateLocationSlotId → Warning
Hardpoint.NonStandardSocketName   → Warning (`HP_` 권장)
Hardpoint.SocketMissing           → Warning
Hardpoint.SocketNone              → Info; direct LocalTransform 방식 가능
```

후속 owner가 explicit `RequiredSocketRole`을 제공하는 경우에만 동일 missing socket을 conditional Block으로 승격할 수 있다. P0-03 자체가 차량 용도나 Mount 요구를 추측해 Block으로 올리지 않는다.

Destroyed FX 기본 `FX_Destroyed`는 runtime Bounds fallback이 있다. 다만 **current `FCFVehicleAssetSnapshot` requested socket set에는 Destroyed FX socket이 포함되지 않는다.** 따라서 VB-P0-03이 존재 여부를 읽었다고 가장하지 않는다.

```text
DestroyedFx.ConfiguredName
→ Recipe DefaultDataIntent의 configured name을 guidance로 표시

DestroyedFx.Existence
→ P0-03 current snapshot 기준 Unobserved
→ missing issue를 생성하지 않음
→ Wheel SocketGuide / LayoutCapture hard blocker 아님
```

후속 P0-06 또는 이미 승인된 다른 read-only source가 실제 socket existence를 제공할 때만 Warning으로 표시할 수 있다. 이를 위해 VB-P0-03 설계 단계에서 AssetReader requested-socket schema를 확장하지 않는다.

### 22.1.6 LayoutCapture evaluator — capture 실행 없이 fresh derive

기존 `Capture Vehicle Layout From Chassis Sockets`는 mutation이며 VB-P0-03 evaluator가 호출하지 않는다.

read-only evaluator는 current snapshot으로 **"지금 capture 가능한가"**와 **"현재 persisted 결과가 현재 socket truth와 같은가"**를 판정한다.

Prerequisite Block:

```text
Chassis loaded 아님
또는 Wheel role socket 4/4 미존재
또는 Wheel role duplicate binding 존재
→ LayoutCapture = Blocked
```

상태 판정:

```text
VehicleLayoutConfig.bUseLayoutOverrides == false
→ Ready / NotCaptured

bUseLayoutOverrides == true
+ persisted BodyWheelSocket names가 current resolved names와 일치
+ persisted WheelAnchor FL/FR/RL/RR location/rotation이 current socket local transform과 일치
→ wheel layout Current

bUseLayoutOverrides == true
+ 위 expected/current 중 하나라도 불일치
→ Stale / CurrentMismatch
```

Socket-bound Hardpoint는 materialized target slot이 존재하고 current Recipe intent와 대응될 때만 비교한다. 누락/불일치는 기본 Warning이며 P0-06이 해당 slot을 required로 선언한 경우에만 후속 completion blocker가 될 수 있다.

따라서 `Complete`는 old capture flag가 아니라 current Asset truth와 persisted target truth의 fresh equality에서 파생한다. Chassis path, requested wheel socket identity 또는 socket transform이 바뀌면 `ChassisLayoutFingerprint`가 달라지고 다음 refresh에서 old layout은 `Stale`로 판정된다.

### 22.1.7 Dependency stamps

P0-03 logical stamp는 다음처럼 좁게 구성한다.

```text
MeshPrepStamp
= Chassis identity/load state
+ Wheel role identity/load state/MeasureFingerprint
+ 실제 표시 sanity에 소비한 Evidence dimension subset identity

SocketGuideStamp
= ChassisLayoutFingerprint
+ normalized Wheel role→resolved socket binding
+ normalized Hardpoint intent identity/socket list

LayoutCaptureStamp
= SocketGuideStamp
+ current persisted VehicleLayoutConfig relevant leaves
+ 비교 대상 materialized Hardpoint transform leaves
```

full RecipeFingerprint 하나만으로 P0-03 전체를 무조건 Stale 처리하지 않는다. Transmission, Engine, Defense처럼 Mesh/Socket과 무관한 Recipe 변경은 P0-03 result를 무효화하지 않는다.

### 22.1.8 USER action UX

Builder가 USER에게 요구하는 Mesh Editor 작업은 항상 다음 4개를 함께 표시한다.

```text
무엇을: 예) 뒤오른쪽 Wheel socket 생성
어디에: 현재 Chassis Static Mesh / 소켓 매니저(Socket Manager)
정확한 이름: 예) Wheel_Anchor_RR 또는 Recipe가 명시한 이름
배치 기준: Wheel center / +X 전방 / Reference Wheelbase·Track 비교값
```

각 이름은 복사 가능해야 한다. USER가 Static Mesh Editor에서 수정하고 Builder로 돌아오면 `[다시 검사]` 또는 Tab re-activation에서 fresh AssetSnapshot을 읽는다.

P0-03은 자동 socket 생성/이동, wheel center 검출, hardpoint surface inference, capture mutation, Save를 수행하지 않는다.

### 22.1.9 P0-03 dry-run

계약 수준 시나리오:

```text
A. Chassis 없음
→ MeshPrep Blocked / 이후 SocketGuide·LayoutCapture Locked

B. Chassis + FL wheel, 나머지 wheel ref 없음
→ MeshPrep Warning 포함 Complete 가능 / current backend semantics 보존

C. Wheel socket 3/4
→ SocketGuide Blocked / LayoutCapture Blocked

D. 4 role 중 FL/FR이 같은 socket name
→ SocketGuide Blocked / capture 선행 금지

E. 4/4 custom socket names가 모두 distinct하고 존재
→ Info만 / SocketGuide Complete 가능

F. Hardpoint socket 하나 누락, wheel 4/4 정상
→ Warning / wheel LayoutCapture readiness 유지

G. FX_Destroyed가 current AssetSnapshot 관측 범위 밖
→ Guidance / Existence=Unobserved / runtime fallback 보존 / LayoutCapture 차단 없음

H. 기존 layout capture 후 Chassis socket 위치 변경
→ fresh expected/current mismatch → LayoutCapture Stale

I. Transmission/Engine Recipe 값만 변경
→ P0-03 narrow dependency stamp 불변 → Mesh/Socket/Layout 결과 불필요한 Stale 없음
```

모두 기존 Shell state 의미를 바꾸지 않고 fail-closed 또는 warning-tolerant하게 해석된다.

### 22.1.10 VB-P0-03 판정

**PASS — Design / Mesh-Socket-Layout Read-Only Evaluator Contract**

```text
C++ 0
UE Asset 0
Recipe/VehicleData schema mutation 0
Capture mutation 0
Editor/Build/Automation 0
Implementation 0
```

다음 Gate는 `VB-P0-04 Reference → Authoring Proposal Bridge`다. 이는 Gate 순서 전진일 뿐, VB-P0-03에서 numeric mapping을 수행했다는 뜻이 아니다.

위 `Implementation 0`은 2026-08-26 VB-P0-03 설계 Gate 당시의 판정이다. 실제 Guided Shell 연결 상태는 아래 VB-P0-09 integration checkpoint가 현재 truth다.

### 22.2 VB-P0-09 Guided Shell Runtime Connection — Technical PASS

2026-08-27 VB-P0-09 USER Acceptance 준비에서 기존 설계 계약을 실제 `FCFVehicleBuilderVM`에 연결했다.

```text
Source
= CFVehicleBuilderVM.h/.cpp v1.2.0
= existing FCFVehicleAuthoringVM fresh Resolve/AssetSnapshot 재사용
= new Asset scanner / parallel Validator / persistent Step bool 0

MeshPrep
= Chassis + WheelMeshFL 최소 필수
= FR/RL/RR 미지정은 current Validator Warning semantics대로 허용
= 지정된 Wheel은 resolve + finite/non-zero usable bounds 요구
= +X 전방 / Reference scale은 USER review Warning

SocketGuide
= None binding은 current Wheel_Anchor_FL/FR/RL/RR default로 resolve
= custom name 허용
= 4/4 found + pairwise distinct만 hard completion rule
= Wheelbase/FrontTrack/RearTrack read-only 산출
= Hardpoint warning은 P0-03 completion blocker로 승격하지 않음

LayoutCapture
= capture mutation 실행 0
= current Target VehicleLayoutConfig와 fresh AssetSnapshot을 resolver precedence와 무관하게 direct exact 비교
= bUseLayoutOverrides=false → Ready / NotCaptured
= current identity/location/rotation exact match → Complete
= current Socket transform/name mismatch → Stale
= Hardpoint persisted mismatch → Warning only

Protection
= auto Socket 생성/이동 0
= wheel center 자동검출 0
= arbitrary Reference threshold 0
= Product Asset/VehicleData Save 0
```

최신 검증:

```text
Official BuildEditor
= 1b1aa29b80c341c6a6289ac2693413e9 PASS / exit 0

Focused Automation
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell
= c6ab98e244634dd1a7fbcb84b290d4e3
= 1/1 PASS / failure 0 / engine exit 0

Covered
= baseline Mesh/Socket/Layout Complete
= optional FR/RL/RR Wheel ref 없음에서도 FL 최소 기준 Complete
= captured 이후 Wheel Socket transform 변경 → Layout Stale
= duplicate Wheel role binding → Socket/ Layout Blocked
```

이는 **Step 2~4 runtime evaluator technical slice PASS**이며 `VB-P0-09 End-to-End USER Acceptance` 전체 PASS가 아니다.

### 22.3 VB-P0-09 Step 1 Reference Evidence / Companion USER Flow — Technical PASS

2026-08-27 VB-P0-09 Step 1에서 설계 상태였던 Reference review contract를 실제 Guided Shell runtime flow에 연결했다.

```text
Source
= CFVehicleBuilderVM.h/.cpp v1.4.0
= CFVehicleBuilderTab.h/.cpp v1.3.0
= existing FCFVehicleAuthoringService Builder Companion/Profile facade 재사용
= parallel Evidence/Profile writer 0

AI Research Draft
= UE/Saved/CarFight/VehicleBuilder/ResearchDraft.json
= FCFBuilderResearchDraft SchemaRevision 1
= exact RecipeId + TargetDefinitionPath binding
= initial Evidence payload semantic validation
= persistent SSOT/approval token 아님

Evidence discovery
= Asset Registry에서 UCFVehicleRefEvidence exact RecipeId/RecipePath/TargetPath match
= partial/duplicate binding fail-closed
= fresh BuildEvidenceFingerprint readback exact match required

Companion review/commit
= PreviewBuilderCompanions mutation0
= existing valid Evidence/private Profile 보존
= Missing만 생성
= fixed EvidenceId + validated initial payload
= prospective EvidenceFingerprint를 ProposalHash에 binding
= explicit USER OwnershipWrite approval 뒤 CreateBuilderCompanions
= auto Save 0 / VehicleData Apply 0 / shared Profile mutation 0

Reference review token
= RecipeId + EvidenceId + EvidenceFingerprint
= EditorPerProject local navigation convenience only
= Profile/Recipe write, VehicleData Apply, Save 권한 0
= fresh VM에서 exact current fingerprint일 때만 resume Complete

Step 1 derived state
= Evidence 없음 → Ready
= binding/fingerprint error 또는 blocking conflict → Blocked
= current Evidence valid + USER review 전 → Ready
= old review token과 current fingerprint mismatch → Stale
= exact USER review token match → Complete

Boundary
= private 4 Profile completeness 자체는 Step 1 completion authority가 아님
= vehicle-specific numeric Physics Proposal review/commit은 Step 5 owner 유지
```

Validation:

```text
Official BuildEditor
= 06370173f2c64a1ab4dd3d235bd46c18 PASS / exit 0

Affected Automation PASS
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep1Reference
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderCompanionFlow

Broad DataAuthoring
= 71 total / affected Step 1 tests PASS
= unrelated DAUTH_P0_08.Batch.AllowlistProjection stale leaf-count assertion 1 FAIL
= Step 1 blocker로 확대하지 않음

Protection
= Product Asset Save 0
= VehicleData Apply 0
= auto Socket/Hardpoint mutation 0
= Git commit/push 0
```

이는 **Step 1 Technical PASS**이며 VB-P0-09 전체 USER Acceptance PASS는 아니다.

### 22.4 VB-P0-09 Step 5 Physics Proposal Runtime Connection — Technical PASS

2026-08-27 VB-P0-09 Step 5에서 기존 P0-05 typed Profile commit backend를 Guided Shell 정상 제작 UX에 연결했다.

```text
Source
= CFVehicleAIContract.h v1.13.0
= CFVehicleBuilderVM.h/.cpp v1.5.0
= CFVehicleBuilderTab.h/.cpp v1.4.0
= existing PreviewBuilderProfiles / CommitBuilderProfiles 재사용
= parallel Profile writer / Target writer 0

Transient Physics Draft
= UE/Saved/CarFight/VehicleBuilder/PhysicsDraft.json
= FCFBuilderPhysicsDraft SchemaRevision 1
= RecipeId / TargetDefinitionPath / EvidenceId / ExpectedEvidenceFingerprint
= complete FCFBuilderPrivateProfilePayload
= ConsumedClaimIds / ProposalCorrelationHash
= ProposalLabel / UserFacingSummary
= persistent progress/approval authority 아님

Binding
= Step 1~4 current Complete required
= Step 1 USER-reviewed exact Evidence fingerprint required
= private Profile 4/4 exact OwnerRecipeId required
= Draft가 Profile path를 명시해도 current Recipe exact private binding과 다르면 reject
= 실제 write destination은 current Recipe private binding으로 강제

Review / Commit
= Load Physics Draft는 mutation0
= PreparePhysicsProposal은 fresh state 뒤 PreviewBuilderProfiles mutation0
= USER review에는 AI summary / consumed Claim / domain별 change / prospective resolved hash 표시
= explicit USER Yes 뒤 AuthoringWrite approval
= ExecutePreparedPhysicsProposal은 existing CommitBuilderProfiles R1
= prepared approval one-shot
= Target VehicleData Apply 0
= auto Save / auto retry 0

Derived Step state
= Step 1~4 incomplete → Locked
= missing/foreign private Profile → Blocked
= current accepted Evidence 없음/오류 → Blocked 또는 Locked
= fresh loaded Draft + pending typed change → Ready
= current Profile payload + persistent BuilderCommitReceipt exact → Complete
= Evidence/Profile/receipt/Resolver drift → Stale

Resume
= Editor restart 뒤 loaded PhysicsDraft와 mutation approval 복원 0
= fresh VM은 Recipe BuilderCommitReceipt의 Evidence/Claim/Profile fingerprints + resolved hash + resolver revision을 current facade로 재검증
= current truth가 exact하면 Complete
= mismatch면 Stale

Ownership boundary
= Physics Proposal commit = authoring Profile truth
= Target Definition Apply = Step 7 owner
= Technical benchmark / USER feel = Step 8/P0-09 owner
```

Validation:

```text
Official BuildEditor
= 3d65b8e8b6d04f498ceaf6358618c6a0 PASS / exit 0

Automation
= BuilderStep5Physics PASS
= BuilderStep1Reference PASS
= BuilderShell PASS
= BuilderProfileCommit PASS
= BuilderCompanionFlow PASS

Broad DataAuthoring
= 72 performed / 71 PASS
= unrelated DAUTH_P0_08.Batch.AllowlistProjection 1 FAIL
= stale Profile schema leaf-count expectation 79→89

Protection
= Product Asset Save 0
= Target VehicleData Apply 0
= shared Profile mutation 0
= raw VehicleData write 0
= auto Socket/Hardpoint mutation 0
```

이는 **Step 5 Physics Proposal Guided Flow Technical PASS**이며 VB-P0-09 전체 USER Acceptance PASS는 아니다.

### 22.5 VB-P0-09 Step 6 Gameplay Setup Runtime Connection — Technical PASS

2026-08-27 기존 P0-06 Gameplay Guidance read authority를 별도 writer 없이 Guided Shell Step 6에 연결했다.

```text
Source
= CFVehicleBuilderVM.h/.cpp v1.6.0
= CFVehicleBuilderTab.h/.cpp v1.5.0
= CFVehicleAuthoringVMTests.cpp v1.15.0
= existing ReadBuilderGameplayGuidance R0

Provider contract
= current managed Recipe/Target
= DeriveCompanionMode()
= ResolveVehiclePreview + current snapshots
= 8 fixed gameplay area result
= mutation0 / Save0

Step state
= Step 5 != Complete → Locked
= R0 read error 또는 BlockedCount > 0 → Blocked
= NeedsReviewCount > 0 → Ready
= bCanCompleteGameplayStep → Complete
= persistent completion bool 없음

UI
= current StepId == GameplaySetup에서만 Step 6 panel visible
= 8영역 상태/설명/해결법/field path 표시
= SocketGuidance 표시
= pending gameplay diff 표시
= 별도 mutation action 없음

USER authority
= Hardpoint/DestroyedFx Socket 생성·위치 조정은 USER
= Existing Completion stored local transform 허용 semantics backend 그대로 보존
= Builder가 socket 위치를 자동 보정하지 않음

Stale propagation
= Step 5 receipt/Profile/Evidence drift로 PhysicsProposal이 Stale이면 Step6 Locked
= cached old GameplayGuidanceResult 폐기
= fresh VM/refresh에서 R0 결과 재생성

Apply boundary
= PendingGameplayDiffCount > 0이어도 blocker 아님
= Step 7 Final Review에서 explicit DefinitionApply 검토
```

Validation:

```text
Official BuildEditor
= d01056d228214ee8bbd06b154768e96d PASS / exit 0

Affected Automation PASS
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep5Physics

Broad DataAuthoring
= 72 performed / 71 PASS
= unrelated DAUTH_P0_08.Batch.AllowlistProjection stale leaf-count 1 FAIL

Protection
= Product Asset Save 0
= Target VehicleData Apply 0
= Profile/Recipe mutation 0
= auto Socket/Hardpoint mutation 0
```

이는 **Step 6 Gameplay Setup Guided Flow Technical PASS**이며 VB-P0-09 전체 USER Acceptance PASS는 아니다.

### 22.6 VB-P0-09 Step 7 Final Review Runtime Connection — Technical PASS

2026-08-27 기존 P0-07 Final Review / DefinitionApply / guarded Undo backend를 별도 writer 없이 Guided Shell Step 7에 연결했다.

```text
Source
= CFVehicleBuilderVM.h/.cpp v1.7.0
= CFVehicleBuilderTab.h/.cpp v1.6.0
= CFVehicleAuthoringVMTests.cpp v1.16.0

Review projection
= ReadBuilderFinalReview R0
= Warning / Blocker / External Drift
= Reference provenance
= 모든 FieldDiff canonical path + before/after
= DiffHash / ProposalHash / resolved hash / resolver revision

Apply UX
= 'Final Review 검토 후 적용'
= fresh review → USER Yes/No → DefinitionApply
= existing ApplyBuilderFinalReview R3 only
= Apply 직전 backend fresh re-review
= stale ProposalHash mismatch fail-closed
= Target/Recipe AppliedState transaction mutation 가능
= auto Save/retry 0

Undo UX
= '마지막 Builder Apply 되돌리기'
= successful Apply가 반환한 exact guarded token만 current VM에 보관
= generic Workspace Undo와 분리
= exact Undo stack top + exact post-Apply Target/Recipe/AppliedState guard
= guard mismatch면 mutation0 차단

State / resume
= Step6 Complete prerequisite
= R0 current truth로 Ready/Blocked/Complete derive
= Step7 completion bool 저장 0
= Apply 뒤 fresh diff0 → Complete
= Undo 뒤 fresh diff 복원 → Ready
= Undo token은 persistent resume authority가 아니라 current Editor lifetime capability
```

Validation:

```text
Official BuildEditor
= f9a9ced62f1545f884ceefdfd2a966ab PASS / exit 0

Focused Automation
= BuilderStep7FinalReview 1/1 PASS
= process a7efe7e368cd42f5b57a18c9b66f756f
= engine exit 0 / failure 0

Protection
= Step1~6 regression replay 0
= Product Asset Save 0
= shared Profile mutation 0
= raw VehicleData write 0
= auto Socket/Hardpoint mutation 0
```

이는 **Step 7 Final Review / explicit Apply / guarded Undo Guided Flow Technical PASS**이며 VB-P0-09 전체 USER Acceptance PASS는 아니다.

### 22.7 VB-P0-09 Step 8 Technical Driving + USER Driving Runtime Connection — Technical PASS

2026-08-27 기존 VB-P0-08 benchmark authority를 Guided Shell Step 8에 연결했다.

```text
Source
= CFVehicleBuilderTypes.h v1.2.0
= CFVehicleBuilderVM.h/.cpp v1.8.0
= CFVehicleBuilderTab.h/.cpp v1.7.0
= CFVehicleAuthoringVMTests.cpp v1.17.0
= RunBuilderBench.ps1 v1.3.0

Benchmark UX
= '기술 벤치마크 실행'
= Step 7 current Complete
= Recipe/Target dirty gate
= existing runner non-blocking child process
= saved Target exact object path + TargetHash + fresh RunId
= terminal result fresh readback
= current identity mismatch → Stale/fail-closed

Metric projection
= configured/actual mass
= 0→50 / 0→100
= peak speed / stable
= peak RPM / gear
= braking
= steady yaw
= turning radius / average speed

USER Driving UX
= '현재 PIE에 선택 차량 적용'
= saved VehicleData transient duplicate
= active PIE Player CFVehiclePawn InitializeVehicleRuntime
= persistent mutation/save 0

USER acceptance UX
= 'USER 주행 PASS'
= actual PIE transient apply prepared this session 필수
= explicit checklist confirmation
= RecipeId + TargetHash + RunId exact local token
= fresh VM resume 시 current truth와 재검증
= new RunId/Target drift면 old token invalid

State
= Step7 incomplete → Locked
= dirty saved-state → Ready/Stale with manual Save guidance
= no current benchmark → Ready
= stale/invalid benchmark → Stale
= current benchmark + USER PASS 없음 → Ready
= exact current benchmark + exact USER token → Complete
= persistent bStep8Complete 0
```

Validation:

```text
Final Official Build
= 055dbddf74f342e9b10a92e1fb2542b8 PASS / exit 0

Focused Automation
= BuilderStep8Driving 1/1 PASS
= process aa2a365599fc46d484857894aa0df47a
= engine exit 0 / failure 0

Runner actual envelope canary
= process 6135fbff338c47f8944dd5b8b02d5069
= RunId + ExpectedTargetDefinitionHash emitted
= engine exit 0 / metric count 1

Protection
= Step1~7 suite replay 0
= Product Asset auto Save 0
= raw/shared Profile writer 0
= auto Socket/Hardpoint mutation 0
= arbitrary Reference threshold 0
= benchmark USER feel assertion 0
```

이는 **Step 8 Guided Flow Technical PASS**다. 실제 신규 차량을 Builder로 제작하고 USER가 직접 주행 승인하기 전에는 VB-P0-09 전체 USER Acceptance PASS가 아니다.

---

## 23. Changelog

### v0.1.15 - 2026-09-01

- WSA-P0-07 actual Wagon USER PASS를 current Shell dependency에 반영하고 `WheelSizeAuthorityPlan.md v0.1.19`를 하위 owner로 갱신했다.
- actual Wagon은 AI-only preparation/Step1 pending 상태가 아니라 Step1~7 Apply+Save 및 Step8 runtime apply가 완료된 상태다. Shell의 다음 resume dependency는 fresh Resolver/adoption evidence refresh다.
- Wheel Size Authority의 USER Socket Scale, shared FL fallback, Right orientation/spin, full-transform hot-reinit current 계약은 Systems/WSA owner를 따른다.

### v0.1.14 - 2026-08-29

- actual Wagon offsite AI-only preparation checkpoint를 Shell resume contract에 동기화했다. typed `ResearchDraft.json`은 exact Wagon Recipe/Target binding, representative Evidence, complete private 4 Profile seed와 actual WheelClass를 준비한 상태다.
- WSA-P0-06 Technical PASS / WSA-P0-07 USER Acceptance Ready를 current shell dependency로 승격했다. SocketScaleFromChassis에서 Reference wheel geometry는 disabled이며 Step 5가 Radius/Width shadow authority를 만들 수 없는 기존 guard를 유지한다.
- persistent companion/Profile/Recipe/Target은 아직 생성·적용·저장하지 않았다. current first USER gate는 Step 1 Reference Set review이고, USER review token을 AI가 대신 생성하지 않는다.

### v0.1.13 - 2026-08-28

- actual Wagon E2E에서 Socket Preview Mesh가 Recipe WheelMesh binding을 대신하지 못하는 UX gap을 current Shell contract로 반영했다. Step 2는 Chassis/FL 필수 + FR/RL/RR 선택 picker와 explicit typed Recipe-only commit을 제공한다.
- stable `SocketGuide` identity는 유지하면서 USER-facing title/content를 `소켓 준비 / Naming`으로 강화하고 exact required/optional Socket 이름·복사·Chassis open UX를 명시했다. 자동 Wheel detection/Socket mutation/Save는 계속 금지다.
- 작업 대상 identity accent + manage-state badge를 current Shell presentation contract로 채택했다. Build `ff8abe01a9fa4e6e90c785618ffccb91`, focused BuilderShell `fb4b91e909254180975b30db00c0e813` PASS다.

### v0.1.12 - 2026-08-28

- WheelSizeAuthorityPlan v0.1.4 final audit를 Shell extension contract에 반영했다.
- SocketScaleFromChassis에서는 Legacy AutoScale profile clamp를 Step6 blocker로 사용하지 않고, `bUseWheelSocketScale` source는 Recipe WheelVisualIntent + Project default만 사용한다.
- Step5 AI Draft가 Reference wheel geometry 5필드를 바꾸지 못하도록 baseline-preserve guard를 구현 의무로 추가했다. 기존 Step1~8 Technical PASS는 보존한다.

### v0.1.11 - 2026-08-28

- `WheelSizeAuthorityPlan.md v0.1.3` 재감사를 반영하고 shared Wheel_FL canonical `100×25×100`, center≈0 live PASS를 current Step 2 source truth로 갱신했다.
- Step 4 persisted equality는 기존 Location/Rotation에 RelativeScale exact 비교를 추가해야 하며, Wheel Size stale는 전체 Chassis fingerprint가 아니라 narrow WheelSizeSourceFingerprint를 사용한다.
- Socket mode는 Legacy AutoScale/AutoCenter/ScaleClamp/RadiusMeasureMode를 사용하지 않는다. 기존 Guided Flow Technical PASS는 보존한다.

### v0.1.10 - 2026-08-28

- `WheelSizeAuthorityPlan.md v0.1.1`을 Shell의 Step 2~5/7 additive extension contract로 연결했다. 기존 AssetSnapshot이 RelativeScale/Bounds를 이미 소유하므로 별도 read schema는 추가하지 않는 설계를 따른다. 기존 Step1~8 Guided Flow Technical PASS는 보존하며 아직 새 Scale 기능이 구현됐다고 주장하지 않는다.
- Step 2는 resolved Wheel Mesh Bounds와 canonical radial X/Z 100cm/Origin sanity를, Step 3은 USER-authored Socket Scale validity와 X=Z 규칙을, Step 4는 Scale+Mesh Bounds dependency 및 derived axle size stale를 후속 evaluator 의무로 추가했다.
- `bUseWheelSocketScale=true`이면 Step 5의 Front/Rear Radius/Width를 독립 AI Physics proposal에서 제외하고 Socket-derived field로 취급하도록 경계를 고정했다.
- StaticMesh Bounds는 측정 Source일 뿐 타이어 크기 자동 디자인 입력이 아니며, USER가 휠하우스를 보고 Socket Scale을 결정하는 authority를 유지한다.
- 현재 shared Wheel_FL X/Z≈79.358cm는 canonical 100cm normalization 전이므로 새 Step 2 contract의 future PASS로 선확정하지 않는다.
- 문서 설계만 변경했으며 C++/UE Asset/Save/Runtime mutation은 0이다.

### v0.1.9 - 2026-08-27

- VB-P0-09 Step 8 Technical Driving + USER Driving Runtime Connection을 Technical PASS로 기록했다.
- existing VB-P0-08 fixed-60Hz runner를 saved Target exact path/DefinitionHash/fresh RunId에 binding하고 current result만 Shell에서 인정한다. metric은 Reference threshold/USER feel을 대신 판정하지 않는다.
- Step 7 Apply 뒤 dirty Recipe/Target에는 manual Save를 요구하되 Builder auto Save는 하지 않는다.
- selected saved VehicleData는 active PIE Player CFVehiclePawn에 transient duplicate로만 적용하며 persistent Asset mutation은 0이다. 실제 apply 전 USER PASS는 fail-closed한다.
- USER Driving PASS는 RecipeId + TargetHash + RunId exact local token이며 fresh VM resume에서도 current truth를 재검증하고 새 RunId/Target drift에서 자동 무효화한다.
- Official Build `055dbddf74f342e9b10a92e1fb2542b8`, focused `BuilderStep8Driving` `aa2a365599fc46d484857894aa0df47a`, runner envelope canary `6135fbff338c47f8944dd5b8b02d5069` PASS. Step1~7 suite replay는 0이다.
- 실제 신규 차량 E2E USER Driving 전 전체 VB-P0-09 PASS와 P0-10 promotion은 계속 Pending이다.

### v0.1.8 - 2026-08-27

- VB-P0-09 Step 7 Final Review Runtime Connection을 existing `ReadBuilderFinalReview` / `ApplyBuilderFinalReview` / `UndoBuilderFinalApply` authority로 연결하고 Technical PASS로 기록했다.
- Shell은 Validation/Drift/Provenance/전체 Field Diff를 표시하고 USER가 확인한 exact ProposalHash만 explicit DefinitionApply에 사용한다.
- Apply backend의 fresh re-review/stale hash guard와 Undo의 exact current-lifetime transaction top/post-state guard를 그대로 보존했다. generic Workspace Undo는 사용하지 않는다.
- Official Build `f9a9ced62f1545f884ceefdfd2a966ab` PASS, 새 focused `BuilderStep7FinalReview` 1/1 PASS를 확보했다. Step1~6 suite replay는 0이다.
- Step 8 연결 전까지 USER E2E는 Pending이며 auto Save/raw writer/shared Profile/auto Socket·Hardpoint mutation 금지를 유지한다.

### v0.1.7 - 2026-08-27

- VB-P0-09 Step 6 Gameplay Setup Runtime Connection을 existing `ReadBuilderGameplayGuidance` R0 authority로 연결하고 Technical PASS로 기록했다.
- 8영역 completeness와 SocketGuidance를 current Step에 표시하고 persistent completion flag 없이 current truth에서 `Blocked/Ready/Complete`를 derive한다.
- Step 5가 Stale이면 old Step6 guidance를 폐기하고 Locked로 전파한다. fresh VM은 R0 guidance를 다시 읽는다.
- USER Socket 생성·위치 조정 authority, Step 7 Target Apply ownership, Save0를 유지했다.
- Official Build `d01056d228214ee8bbd06b154768e96d` PASS와 affected `GameplayGuidance` / Guided VM integration regression PASS를 확보했다.

### v0.1.6 - 2026-08-27

- VB-P0-09 Step 5 Physics Proposal Runtime Connection을 Guided Shell에 연결하고 Technical PASS로 기록했다.
- transient `FCFBuilderPhysicsDraft`가 exact current Recipe/Target/USER-reviewed Evidence와 complete private 4 Profile typed payload를 운반하며 persistent progress/approval authority는 갖지 않는다.
- Profile write destination은 current Recipe exact Builder-private binding으로 강제하고 `PreviewBuilderProfiles` mutation0 → explicit USER AuthoringWrite → `CommitBuilderProfiles` existing R1 lane만 사용한다.
- Step 5 Complete/Resume/Stale은 persistent BuilderCommitReceipt와 current Evidence/Claim/private Profile/ResolvedDefinitionHash/Resolver revision을 fresh 재검증해 derive한다. transient Draft와 old mutation approval은 restart 뒤 복원하지 않는다.
- Shell은 AI 차량 특성 설명, consumed Claim, domain별 변경 여부, prospective resolved hash를 보여주며 Target Apply는 Step 7 owner로 유지한다.
- Official Build `3d65b8e8b6d04f498ceaf6358618c6a0` PASS, `BuilderStep5Physics` 및 affected Builder regression PASS를 확보했다. broad의 unrelated `AllowlistProjection` stale 79→89 assertion은 별도 유지한다.
### v0.1.5 - 2026-08-27

- VB-P0-09 Step 1 Reference Evidence / Companion USER flow를 실제 Guided Shell에 연결하고 Technical PASS로 기록했다.
- transient `FCFBuilderResearchDraft` JSON을 exact RecipeId/Target에 binding하고 initial Evidence payload를 semantic validation한 뒤에만 Companion preview가 가능하도록 했다.
- current Evidence discovery는 exact RecipeId/RecipePath/TargetPath와 fresh EvidenceFingerprint를 요구하고 partial/duplicate binding은 fail-closed한다.
- Companion은 existing valid Evidence/private Profile을 보존하고 Missing만 mutation0 preview → explicit USER OwnershipWrite → R2 one-transaction commit한다. auto Save/VehicleData Apply/shared Profile mutation은 하지 않는다.
- Reference review token은 RecipeId/EvidenceId/EvidenceFingerprint local convenience만 소유하며 mutation approval과 분리했다. current Evidence fingerprint drift는 Step 1 Stale로 fresh derive한다.
- private Profile completeness를 Step 1 hard completion으로 올리지 않고 Step 5 Physics Proposal authority를 유지했다.
- Official Build `06370173f2c64a1ab4dd3d235bd46c18` PASS, `BuilderStep1Reference` / `BuilderShell` / `BuilderCompanionFlow` affected Automation PASS를 확보했다.
### v0.1.4 - 2026-08-27

- VB-P0-09 post-integration extensibility hardening으로 current 8-step baseline의 **개수/순서와 stable StepId semantic identity를 분리**했다.
- `GetVehicleBuilderStepDefinitions()`를 current composition/order/title 단일 owner로 두고 `StepNumber`는 definition index에서 자동 파생하도록 고정했다.
- `RebuildStepStates()`의 monolithic index-based 판정을 Step별 `Evaluate*Step()` 함수로 분리하고 상태 read/write를 `FindStepView(StepId)` / `SetStep(StepId, ...)`로 전환했다. `EvaluateStepById()`가 current definition 순서대로 evaluator를 dispatch해 표시 순서와 실행 순서 owner를 단일화한다.
- Slate navigation은 `StepViews.Num()`을 사용하며 Step-specific UI는 numeric index가 아니라 current `StepId`를 사용한다. focused Automation도 exact-8/index assertion 대신 stable StepId lookup으로 전환했다.
- 새 Step 추가·기존 Step 재배치·Step 통합 시 global renumbering을 요구하지 않으며, 제거된 StepId는 과거 resume/log 호환성을 위해 다른 의미로 재사용하지 않는다.
- generic plugin registry / Blueprint workflow engine / Step DataAsset / persistent progress graph는 도입하지 않는다.
- Final Official Build `8e1c55defcbc49108c4c402964a33477` PASS, focused BuilderShell `0e2b3bd9260446c084e0523427283e02` 1/1 PASS. Product Asset/Socket/Save mutation 0.

### v0.1.3 - 2026-08-27

- VB-P0-09 USER Acceptance 준비에서 `CFVehicleBuilderVM v1.2.0`에 VB-P0-03 MeshPrep/SocketGuide/LayoutCapture evaluator를 실제 연결했다.
- Step 2는 Chassis+FL 최소 기준과 optional FR/RL/RR warning semantics, usable Wheel bounds를 보존하고 5/5 강제를 하지 않는다.
- Step 3은 current AssetSnapshot 4-role socket found+distinct를 hard rule로 사용하며 custom name, Wheelbase/Track read-only 산출, Hardpoint warning-only 경계를 유지한다.
- Step 4는 Resolver precedence에 가려지지 않도록 current Target `VehicleLayoutConfig`와 fresh AssetSnapshot socket identity/location/rotation을 direct exact 비교해 Current/Stale을 파생한다.
- Official Build `1b1aa29b80c341c6a6289ac2693413e9` PASS, focused BuilderShell `c6ab98e244634dd1a7fbcb84b290d4e3` 1/1 PASS. Product Asset/Socket/Save mutation 0.
- VB-P0-09 USER Acceptance는 아직 Pending이며 Systems 승격은 수행하지 않았다.

### v0.1.2 - 2026-08-26

- VB-P0-03 `MeshPrep / SocketGuide / LayoutCapture`의 read-only evaluator와 blocker/stale 계약을 current `FCFVehicleAssetReader`/`FCFVehicleAssetSnapshot` authority에 맞춰 고정했다.
- 별도 Builder Asset scanner를 만들지 않고 Chassis socket facts, `ChassisLayoutFingerprint`, Wheel bounds/`MeasureFingerprint`를 재사용하도록 했다.
- Wheel 기본 `Wheel_Anchor_*`는 권장 이름으로 유지하되 custom binding은 허용하고, 4개 역할의 missing/duplicate binding만 hard blocker로 고정했다.
- Hardpoint/Destroyed FX는 requiredness를 P0-03이 추론하지 않으며 current warning/fallback semantics를 보존했다.
- LayoutCapture completion을 persistent flag가 아니라 current socket truth와 persisted `VehicleLayoutConfig`의 fresh equality로 derive하고 mismatch를 Stale로 판정하도록 했다.
- Reference Wheelbase/Track은 socket-derived 값과 Delta를 표시하지만 임의 비율 threshold로 Block하지 않고 USER review Warning으로 유지했다.
- `CaptureLayoutFromChassisSockets()`는 mutation이므로 P0-03 evaluator가 호출하지 않으며 P0-05 mutation flow owner로 남겼다.
- 9개 dry-run scenario를 계약 수준으로 PASS했고 Implementation 0을 유지하며 next gate를 VB-P0-04로 전진했다.

### v0.1.1 - 2026-08-26

- VB-P0-02 PASS를 다시 열지 않고 다단 변속기 Authoring 요구의 owner/context dependency를 Shell contract에 additive하게 통합했다.
- Builder-private Drivetrain Profile이 Differential과 함께 차량별 Transmission numeric authoring을 소유한다는 persistent truth 경계를 명시했다.
- PhysicsProposal extension slot이 Evidence transmission claims + Drivetrain Profile binding/fingerprint를 후속 P0-04 dependency로 소비하도록 했으나 gear ratio/shift 계산과 `TransmissionSetup` mapping은 Shell 책임에서 제외했다.
- Transmission evidence/profile 변경이 PhysicsProposal/FinalReview/DrivingTest stale에 참여할 수 있도록 dependency boundary를 추가했다.
- HUD/DrivingTest는 실제 Chaos `GetCurrentGear()` authority를 유지하고 Builder fake gear state를 금지했다.
- current next gate `VB-P0-03 Mesh & Socket Guidance`와 8-step navigation/state contract는 변경하지 않았다.

### v0.1.0 - 2026-08-26

- VB-P0-02 Builder Shell의 persistent/transient state ownership을 신규 고정했다.
- progress boolean을 Recipe/Evidence에 저장하지 않고 current Asset truth에서 Step Complete/Blocked/Stale를 fresh derive하도록 했다.
- local Editor per-project settings를 restart resume convenience owner로 고정하고 managed primary identity를 `RecipeId`로 결정했다.
- pre-record Draft 1개에 한해 local resume envelope를 허용하되 authoring/research numeric truth 저장을 금지했다.
- 8개 fixed StepId와 `Unavailable/Locked/Ready/Complete/Blocked/Stale` state, issue schema, Back/Next/direct navigation 규칙을 고정했다.
- USER Reference review token을 EvidenceId/Fingerprint/RecipeId에 binding하고 mutation approval과 완전히 분리했다.
- Editor restart 시 prepared Apply/Profile/Proposal approval 및 Undo TransactionId를 모두 폐기하고 fresh read로 상태를 재구성하도록 했다.
- lightweight resume context와 step-specific deep context를 분리하고 background polling/full resolve-on-every-frame을 금지했다.
- P0-03/04/06/07/08을 fixed typed extension slot으로 연결하고 generic plugin registry를 만들지 않기로 했다.
- restart/rename/evidence change/raw drift/duplicate RecipeId/settings deletion 7개 scenario dry-run을 PASS했다.
- C++/UE Asset/Recipe/Evidence/VehicleData/Profile mutation은 수행하지 않았다.

---

## 24. Migration

- 현재 `FCFVehicleAuthoringVM`의 transient selection/cache/approval 계약은 유지한다. Builder가 이를 persistent resume store로 바꾸지 않는다.
- `UCFVehicleRecipeData`에 Step progress field를 추가하지 않는다.
- `UCFVehicleRefEvidence`의 VB-P0-01 semantic fingerprint contract를 Builder navigation state 때문에 변경하지 않는다.
- 기존 Data Authoring `ListVehicles / ReadVehicleContext / ResolveVehiclePreview / Validation / Apply` authority를 Builder가 복제하지 않는다.
- VB-P0-03 read-only evaluator는 current `FCFVehicleAssetReader` snapshot authority를 재사용하고 별도 Asset scanner/Validator를 추가하지 않는다.
- VB-P0-03부터 후속 Step evaluator를 추가할 때 Shell StepState enum과 primary RecipeId resume identity를 임의 변경하지 않는다.
- VB-P0-04 구현 전에는 Proposal provider slot을 `Unavailable`로 두며 raw property write나 임시 mapping으로 채우지 않는다.
- v0.1.1의 Transmission dependency 추가는 VB-P0-02를 재개하거나 VB-P0-03 범위를 확장하지 않는다. exact Drivetrain transmission schema와 UE 5.8 Chaos mapping은 VB-P0-04 owner다.
