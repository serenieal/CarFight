# CarFight Guided Vehicle Builder Roadmap

- 문서 버전: v0.1.39
- 작성일: 2026-09-02
- 문서 상태: Historical + Archived Path / CF-FQ-040 Done / VB-P0-10 Current System Promotion Complete / VB-P0-09 USER Acceptance PASS / ESH Final Audit Clean PASS / WSA P0 Complete
- Feature: `CF-FQ-040 Guided Vehicle Builder`
- Current owner: `Document/Systems/Vehicles/VehicleBuilder.md v1.0.0`
- Historical placement: `Document/Plan/Archive/VehicleBuilder/` Archived Path / G5 physical move complete
- 상위 계획: `Document/Plan/Archive/VehicleBuilder/VehicleBuilderPlan.md`
- 하위 설계: `Document/Plan/Archive/VehicleBuilder/WheelSizeAuthorityPlan.md v0.1.19`
- 역할: Guided Vehicle Builder의 실제 구현 순서, Gate, 보호 범위, 검증과 USER Acceptance 순서를 정의한다.

---

## 1. Roadmap 원칙

```text
기존 Runtime을 먼저 바꾸지 않는다.
현재 차량 제작 경로와 Data Authoring contract를 먼저 감사한다.
Builder는 정상 제작 UX를 소유하고 Data Authoring은 Backend를 유지한다.
사용자가 쉽게 할 수 있는 Mesh/Socket 작업은 자동화하지 않는다.
실차 Reference 수집·정규화·파라미터 추론은 AI가 담당한다.
Target Apply는 Preview/Validation/명시 승인 뒤에만 수행한다.
```

현재 단일 Active `CF-FQ-039`는 변경하지 않는다. `CF-FQ-040`은 VB-P0-10 완료로 Done이며 이 Roadmap은 Historical + Retained Path로 보존한다.

---

## 2. P0 전체 순서

```text
VB-P0-00 Current Vehicle Creation Contract Audit
VB-P0-01 Reference Research & Evidence Contract
VB-P0-02 Builder Shell / Step State / Resume
VB-P0-03 Mesh & Socket Guidance
VB-P0-04 Reference → Authoring Proposal Bridge
VB-P0-05 Vehicle Record / Layout / Physics Apply Flow
VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance
VB-P0-07 Final Review / Validation / Undo
VB-P0-08 Technical Driving Benchmark
VB-P0-09 End-to-End USER Acceptance
  └─ WSA-P0-00 Wheel Size Authority Design Lock — PASS
  └─ WSA-P0-01 Canonical Default Wheel + Data Schema & Shared Size Utility
  └─ WSA-P0-02 Socket Resolver & Derived Physics
  └─ WSA-P0-03 Runtime Visual / Mesh Fallback
  └─ WSA-P0-04 Validator / Builder Integration
  └─ WSA-P0-05 Legacy Migration
  └─ WSA-P0-06 Technical Validation
  └─ WSA-P0-07 USER Acceptance
VB-P0-10 Current System Promotion / Advanced Authoring Role Lock
```

---

## 3. VB-P0-00 — Current Vehicle Creation Contract Audit

### 목적

코드를 추가하기 전에 현재 차량 한 대를 만들 때 실제 필요한 Source/Asset/Authoring 경로를 전수 확정한다.

### 감사 대상

```text
UCFVehicleData current fields / 118 Registry leaf
ACFVehiclePawn ApplyVehicleDataConfig runtime paths
CreateVehicleRecords Definition+Recipe creation
Profile 5-domain ownership
AI typed contract write restrictions
CaptureVehicleLayoutFromChassisSockets
Wheel Socket naming
Hardpoint LocationSlotId / SocketName / LocationCategory
MountProfile contract
Fitting Base/Gross Mass contract
Defense / Destroyed FX / DriveState defaults
Current Validator
Current UE Toolset / typed ingress 가능 범위
```

### 결정해야 할 항목

1. Reference evidence persisted owner
   - Recipe 확장
   - editor-only companion asset
   - repository JSON sidecar
   - 기타 기존 owner 재사용

2. AI가 vehicle-specific numeric proposal을 commit할 exact semantic contract
   - shared Profile 무단 mutation 금지
   - vehicle-specific/generated authoring owner 정의

3. Builder가 새 public MCP tool 없이 현재 UE capability surface를 재사용할 수 있는지

4. Builder 정상 경로가 생성해야 할 Asset 최소 집합

### 금지

```text
Runtime schema mutation 0
VehicleData write 0
새 Builder Tab 구현 0
새 public MCP tool 0
기존 UA replay 0
```

### PASS 조건

위 항목의 owner와 데이터 흐름이 코드/Systems/Plan에서 모순 없이 고정된다.

### 2026-08-26 감사 결과 — PASS

read-only Source/Systems/UE Toolset 감사로 다음을 확정했다.

```text
Reference evidence owner
= 별도 Editor-only Vehicle Reference Evidence DataAsset

AI vehicle-specific numeric owner
= Builder-private UCFVehicleBaseProfile
+ Builder-private UCFDrivetrainProfile
  - Differential / powered axle / WheelClass
  - 차량별 Transmission numeric authoring의 정식 owner
+ Builder-private UCFHandlingProfile
+ Builder-private UCFPerformanceProfile

기본 DriveState
= Recipe.DriveStateMode ProjectDefault
= 별도 DriveState Profile 생성 안 함

최소 Builder 생성 Asset
= VehicleData + Recipe + Reference Evidence + private Profile 4종
= 총 7개

AI transport
= 기존 GoPyMCP ue.call_write Bridge 재사용
= CarFight Editor typed Builder operation만 후속 추가
= 새 top-level public MCP tool 불필요
```

Current `CreateVehicleRecords`는 Definition+Recipe만 만들고 모든 Profile inference가 0인 계약을 그대로 보존한다. Builder는 이 core를 변경해 암묵 추론하게 만들지 않고, reviewed orchestration으로 Evidence/private Profile 생성을 별도 단계에 추가한다.

`UCFVehicleFittingData`, `UCFVehicleDefenseData`, `UCFCombatFxData`, Equipment/Weapon/Ammo/Sensor Asset, 별도 Pawn Blueprint는 minimum-created set이 아니다. Chassis/Wheel StaticMesh는 USER 제공 asset이며 Wheel Class는 검증된 existing class reference를 사용한다.

Shared Profile AI payload mutation, AdvancedOverride를 normal AI writer로 사용하는 우회, generic DataAsset/Object raw property write, direct VehicleData write는 모두 금지 유지한다.

추가 구현 gap은 두 가지로 고정한다.

```text
1. Reference Evidence DataAsset schema / evidence fingerprint
2. Builder-private Profile ownership marker + typed R1 commit operation
```

둘 다 구현은 후속 Gate에서만 수행한다.

---

## 4. VB-P0-01 — Reference Research & Evidence Contract

### 목적

실존 차량 Reference를 AI가 반복 가능하고 추적 가능한 방식으로 조사·정규화하도록 계약을 만든다.

### 상태 — PASS

상세 계약 owner는 `VehicleRefEvidenceSpec.md v0.1.1`이다.

고정 결과:

```text
Reference identity
= Manufacturer / Model / ModelYear / Market / Powertrain
+ 필요한 Trim / Transmission / Wheel-Tire Variant scope

Source Citation
= Tier A/B/C/D
+ canonical URL / locator
+ origin independence 분리

Research Claim
= FACT / DERIVED / GAME_BIAS atomic claim
+ explicit UnknownFacts

Conflict
= Identity/Variant mismatch는 평균 금지
= SplitIdentity/SplitVariant 우선
= material unresolved conflict는 mapping Block

Confidence
= evidence support confidence
= gameplay quality/feel authority 아님

Evidence fingerprint
= SHA-256
= `CFVREF-1` + `CFVRN-1`
= semantic evidence + exact Target binding canonicalization

AI Proposal binding
= EvidenceFingerprint
+ RecipeFingerprint
+ TargetDefinitionHash
+ Builder-private ProfileFingerprint 4종
+ ProposalHash / explicit approval
```

### Research policy — Frozen

- 제조사/공식 제원을 Tier A로 우선한다.
- 중요한 물리 fact는 가능한 범위에서 추가 source로 corroboration하되 publisher 수와 실제 origin independence를 구분한다.
- 연식/트림/시장/variant를 섞지 않는다.
- 없는 값은 `UnknownFacts`로 남기며 0/default/추측값으로 대체하지 않는다.
- source quantity의 단순 unit conversion은 FACT를 유지하고, 새로운 계산 quantity만 DERIVED로 분류한다.
- GAME_BIAS는 research normalization에서 자동 생성하지 않는다.
- 기사/표 원문 전체를 저장하지 않고 필요한 fact와 citation/locator만 보존한다.

### Research normalization dry-run — PASS

실제 Reference Set:

```text
The 2027 Kia Morning
Korea / Trendy
Smartstream G1.0 gasoline
4-speed automatic
175/65R14 variant
```

공식 제원에서 동일 powertrain의 14-inch curb mass `975 kg`과 16-inch `1,010 kg`이 동시에 존재하는 사례를 검증했다. 이를 source disagreement 평균으로 처리하지 않고 `VariantScopeMismatch → SplitVariant`로 분리해 14-inch identity에는 `975 kg`만 canonical FACT로 채택했다.

공개 근거를 exact identity로 확정하지 못한 GVW, gear ratios, final drive ratio, engine redline, 0-100, top speed 등은 `UnknownFacts`로 남겼다.

`175/65R14 → nominal outer radius 291.55 mm`처럼 새로운 quantity를 계산한 경우에만 DERIVED로 기록했고 research 단계 GAME_BIAS는 0건이다.

Synthetic canonical payload를 사용해 Evidence array reorder는 fingerprint 불변, fact/target/conflict resolution change는 fingerprint 변경임을 확인했다. Preview 뒤 Evidence/Recipe/Target/private Profile이 변경되면 old proposal이 fail-closed되는 binding도 기존 Authoring approval contract와 양립한다.

### 구현 범위

```text
C++ 0
UE Asset 0
VehicleData/Profile/Recipe mutation 0
Editor/Build/Automation 0
Implementation 0 유지
```

### 다음 Gate

```text
VB-P0-02 Builder Shell / Step State / Resume
```

Reference → CarFight/Chaos numeric mapping과 Builder-private Profile typed R1 commit은 `VB-P0-04`에서 구현한다.

---

## 5. VB-P0-02 — Builder Shell / Step State / Resume

### 목적

한 화면에 모든 설정을 펼치지 않는 단계형 Builder 기반을 만들고, 진행 상태와 실제 authoring truth를 분리한 안전한 Editor restart resume 계약을 고정한다.

### 상태 — PASS

상세 계약 owner는 `VehicleBuilderShellSpec.md v0.1.1`이다.

### State owner — Frozen

```text
실제 작업 결과의 persistent truth
= Recipe / Evidence / VehicleData / private Profile / Mesh / Socket / ApplyState

Builder restart resume convenience
= local Editor per-project user settings

현재 화면 / cache / prepared approval / Undo identity
= transient Builder ViewModel
```

`bStep1Complete`, `LastCompletedStep` 같은 progress boolean을 Recipe/Evidence에 저장하지 않는다. Step 결과는 현재 authoritative truth에서 매번 fresh derive한다.

Managed resume primary identity는 current Source에 이미 존재하는 persistent `RecipeId`다. `RecipePath`와 `DefinitionPath`는 fallback/diagnostic hint이며 Asset rename/move만으로 다른 차량으로 판단하지 않는다.

### Current baseline Step set — stable identity / definition-driven composition

```text
IdentityReference
MeshPrep
SocketGuide
LayoutCapture
PhysicsProposal
GameplaySetup
FinalReview
DrivingTest
```

현재 P0 baseline은 위 8개 StepId를 사용한다. 사용자 표시명은 한글로 제공하고 internal StepId는 stable semantic identity로 유지한다. 다만 **8이라는 개수와 표시 순서는 고정 authority가 아니다.** current composition/order/title은 단일 Step Definition 목록에서 파생하며 StepNumber는 presentation 값이다.

### Step state — Frozen

```text
Unavailable
Locked
Ready
Complete
Blocked
Stale
```

- `Unavailable`: 해당 후속 provider/capability가 아직 구현되지 않음.
- `Locked`: capability는 있으나 선행 조건 미충족.
- `Ready`: 진입 가능하며 현재 수행할 작업이 남음.
- `Complete`: current truth fresh evaluation이 completion contract를 만족.
- `Blocked`: hard blocker가 있어 Next 금지.
- `Stale`: 과거 review/completion dependency가 변경되어 재검토 필요.

### Navigation — Frozen

```text
Back
= 이전 Step으로 단순 UI 이동. mutation/Undo 아님.

Next
= current Step fresh read/evaluation을 다시 실행
→ Complete이면 이동
→ Blocked/Stale이면 현재 Step 유지 + 해결법 표시

Direct Step navigation
= Complete 과거 Step / 현재 Step / immediate unlocked next만 허용
= distant future jump 금지
```

### Editor restart resume — Frozen

```text
local ResumeState 읽기
→ ActiveRecipeId exact lookup
→ Recipe/Target binding fresh verify
→ Evidence.TargetRecipeId/fingerprint fresh verify
→ transient context/StepResult 전부 fresh rebuild
→ LastStep이 unlocked이면 reopen
→ 아니면 가장 가까운 이전 actionable Step으로 이동
```

Prepared Apply/Profile/Proposal approval, cached Resolve/Diff/Validation, Undo TransactionId는 restart 뒤 복원하지 않는다.

### Reference review resume token

```text
EvidenceId
EvidenceFingerprint
RecipeId
```

에 exact binding한다. 이 token은 Reference Set USER review 재개용일 뿐 Recipe/Profile/VehicleData write, Apply, Save 권한을 주지 않는다.

### Read-only context

Shell core는 다음 identity/summary만 read-only로 재구성한다.

```text
RecipeId / RecipePath / DefinitionPath
EvidenceId / EvidenceFingerprint / TargetRecipeId
ManageState / RecipeFingerprint / TargetDefinitionHash
ResolverContractRevision / ProfileBindings / AppliedDefinitionHash
External Drift summary
Chassis/Wheel asset path summary
```

Mesh/Socket deep evidence는 VB-P0-03, Reference→Authoring Proposal은 VB-P0-04, Gameplay/Final/Runtime deep context는 각 후속 Gate가 **stable StepId에 대응하는 명시적 evaluator/content**로 추가한다. generic plugin registry는 만들지 않는다.

### Refresh policy

background tick/polling과 매 frame full Resolve를 금지한다. Tab open/resume, selection change, Step 진입, Next, explicit Refresh, Builder-owned mutation 후처럼 의미 있는 trigger에서만 fresh read한다.

### Design dry-run — PASS

```text
정상 restart
Recipe rename/move
Evidence fingerprint 변경
Target external raw edit
Duplicate RecipeId
Prepared Apply를 가진 상태에서 restart
local Builder settings 삭제
```

7개 scenario에서 stale approval 부활 없이 fail-closed 또는 recoverable하게 복원되는 것을 계약 수준으로 확인했다.

### 구현 범위

```text
C++ 0
UE Asset 0
Recipe/Evidence schema mutation 0
VehicleData/Profile mutation 0
Editor/Build/Automation 0
Implementation 0 유지
```

### 다음 Gate

```text
VB-P0-03 Mesh & Socket Guidance
```

VB-P0-03은 `MeshPrep / SocketGuide / LayoutCapture` evaluator/context만 구체화한다. Physics Proposal의 Reference→CarFight/Chaos numeric mapping과 typed Profile commit은 계속 `VB-P0-04` owner다.

---

## 6. VB-P0-03 — Mesh & Socket Guidance — PASS

### 목적

자동 Geometry 분석 없이 USER가 Mesh/Socket을 정확하게 준비할 수 있도록 `MeshPrep / SocketGuide / LayoutCapture`의 **read-only 준비 상태, blocker, warning, stale boundary**를 고정한다.

상세 evaluator 의미 owner는 `VehicleBuilderShellSpec.md v0.1.2`다.

### 재사용 authority

새 Builder Asset scanner를 만들지 않는다.

```text
FCFVehicleAssetReader
→ FCFVehicleAssetSnapshot
  → ChassisObjectPath / bChassisLoaded
  → ChassisSockets[]
  → ChassisLayoutFingerprint
  → WheelFL/FR/RL/RR bounds + MeasureFingerprint
```

Recipe의 `AssetIntent`와 `HardpointIntents`, current Target의 `VehicleLayoutConfig` readback을 함께 사용한다. `CaptureLayoutFromChassisSockets()`는 mutation이므로 VB-P0-03 read-only evaluator가 호출하지 않는다.

### MeshPrep blocker contract

Block:

```text
Chassis Mesh ref 없음 또는 resolve 실패
WheelMeshFL ref 없음 또는 resolve 실패
지정된 Wheel Mesh가 resolve 실패
지정 Wheel bounds가 non-finite / usable dimension 0
```

Warning/Info:

```text
WheelMeshFR/RL/RR 누락
→ current Validator의 FL reuse warning semantics 보존

+X 전방 방향
→ current AssetSnapshot만으로 시각 방향 자동 확정하지 않음
→ USER manual review

Reference 전장/전폭/전고
→ 안내값으로 표시
→ current snapshot에 Chassis bounds가 없으므로 자동 scale PASS를 주장하지 않음
```

P0-03에서 Wheel bounds를 WheelRadius/Width 같은 Physics numeric으로 변환하지 않는다.

### Wheel Socket contract

기본 권장 이름:

```text
Wheel_Anchor_FL
Wheel_Anchor_FR
Wheel_Anchor_RL
Wheel_Anchor_RR
```

None binding은 위 기본 이름으로 resolve한다. 그러나 current `UCFVDAValidator`처럼 **존재하는 custom Wheel socket name은 허용**한다.

Hard Block은 다음 두 종류다.

```text
Wheel role FL/FR/RL/RR 중 하나라도 resolved socket 미존재
4개 role 중 둘 이상이 같은 resolved socket name 공유
```

즉 correctness는 "기본 문자열 강제"가 아니라:

```text
FL / FR / RL / RR
→ 각각 존재하는 distinct socket 4개
```

이다.

### Read-only layout sanity

4/4 Wheel socket local transform에서 다음을 계산해 사용자에게 보여줄 수 있다.

```text
FrontAxleMid        = (FL + FR) / 2
RearAxleMid         = (RL + RR) / 2
AuthoredWheelbaseCm = abs(FrontAxleMid.X - RearAxleMid.X)
FrontTrackCm        = abs(FR.Y - FL.Y)
RearTrackCm         = abs(RR.Y - RL.Y)
```

`+X 전방` 기준으로 role topology가 명백히 뒤집힌 경우 Warning을 낸다.

Accepted Reference에 Wheelbase/Track fact가 있으면:

```text
Reference
Current socket-derived value
Delta
Delta %
```

를 표시한다. P0에서는 아트 비율 차이를 고려해 임의 5%/10% 같은 threshold로 Block하지 않는다.

### Hardpoint guidance boundary

예:

```text
LocationSlotId = Top_01
LocationCategory = Top
SocketName = HP_Top_01
```

P0-03은 current Recipe가 이미 선언한 Hardpoint intent의 identity/socket 상태만 안내한다.

```text
LocationSlotId 없음     → Warning
LocationSlotId 중복     → Warning
HP_ prefix 아님         → Warning
non-empty Socket 미존재 → Warning
SocketName None         → Info / direct LocalTransform 가능
```

Hardpoint가 실제로 필요한 차량인지 P0-03이 추론하지 않는다. P0-06 Gameplay Setup이 explicit requiredness를 소유하고, 후속 owner가 required socket role을 제공했을 때만 missing socket을 conditional Block으로 승격할 수 있다.

### Destroyed FX boundary

기본 권장:

```text
FX_Destroyed
```

current runtime은 socket 누락 시 body bounds fallback을 사용한다. 다만 current `FCFVehicleAssetSnapshot` requested-socket set은 Destroyed FX socket을 읽지 않으므로 P0-03에서는 configured 이름만 guidance로 표시하고 존재 여부는 `Unobserved`로 둔다. missing issue를 추측 생성하지 않으며 Wheel SocketGuide나 LayoutCapture를 Block하지 않는다. 실제 existence 확인은 P0-06 또는 이미 승인된 다른 read-only owner가 제공할 때만 Warning으로 표현한다.

### LayoutCapture read-only evaluator

VB-P0-03은 capture를 실행하지 않고 두 질문만 판정한다.

```text
1. 지금 capture 가능한가?
2. 현재 persisted layout이 current socket truth와 같은가?
```

Capture prerequisite Block:

```text
Chassis unresolved
Wheel socket 4/4 미충족
Wheel role duplicate binding
```

상태:

```text
bUseLayoutOverrides == false
→ Ready / NotCaptured

bUseLayoutOverrides == true
+ persisted BodyWheelSocket 4종 == current resolved socket 4종
+ persisted WheelAnchor location/rotation == current socket local transform
→ Complete / Current

bUseLayoutOverrides == true
+ 위 비교 중 하나라도 다름
→ Stale / CurrentMismatch
```

materialized socket-bound Hardpoint가 존재하면 corresponding transform을 비교할 수 있지만 기본 Warning이다. P0-06 requiredness가 없는데 LayoutCapture 전체를 Block하지 않는다.

### Narrow dependency / stale

```text
MeshPrepStamp
= Chassis identity/load
+ Wheel role identity/load/MeasureFingerprint
+ 실제 sanity 표시에 소비한 Reference dimension subset

SocketGuideStamp
= ChassisLayoutFingerprint
+ Wheel role→resolved socket binding
+ normalized Hardpoint intent identity/socket

LayoutCaptureStamp
= SocketGuideStamp
+ relevant persisted VehicleLayoutConfig leaves
+ 실제 비교한 Hardpoint transform leaves
```

Engine/Transmission/Defense처럼 Mesh/Socket과 무관한 Recipe 변경만으로 P0-03 전체를 Stale 처리하지 않는다.

### USER 안내 UX

외부 Static Mesh Editor 작업은 항상 다음을 같이 표시한다.

```text
무엇을 해야 하는가
어느 Chassis Mesh에서 해야 하는가
정확한 Socket 이름
배치 기준: Wheel center / +X 전방 / Reference Wheelbase·Track
```

Socket 이름은 복사 가능해야 한다. USER가 Static Mesh Editor에서 수정 후 Builder로 돌아오면 `[다시 검사]` 또는 Tab re-activation에서 fresh snapshot을 읽는다.

### Scope Out

```text
Wheel center auto detection 0
Hardpoint auto placement 0
Mesh surface inference 0
automatic socket creation/move 0
capture mutation 0
Save 0
Reference→Physics numeric mapping 0
Transmission schema / TransmissionSetup mapping 0
```

### Design dry-run

```text
Chassis missing                    → MeshPrep Blocked
FL only + other wheel refs empty   → Warning 포함 MeshPrep Complete 가능
Wheel socket 3/4                   → SocketGuide / LayoutCapture Blocked
duplicate Wheel role binding       → SocketGuide / LayoutCapture Blocked
4/4 valid custom socket            → Info only / Complete 가능
Hardpoint socket missing           → Warning / wheel capture readiness 유지
FX_Destroyed snapshot 관측 밖      → Guidance / Unobserved / fallback 유지
captured layout 후 socket 이동     → LayoutCapture Stale
Transmission/Engine-only change    → P0-03 narrow stamp 불변
```

**VB-P0-03 Design PASS / Implementation 0.**

다음 Gate는 `VB-P0-04 Reference → Authoring Proposal Bridge`다. exact Transmission numeric schema와 UE 5.8 Chaos `TransmissionSetup` mapping은 그 Gate에서만 다룬다.

---

## 7. VB-P0-04 — Reference → Authoring Proposal Bridge — PASS

### 목적

accepted Reference Evidence를 Builder-private VehicleBase/Drivetrain/Handling/Performance 4 Profile의 **complete typed prospective payload**로 변환하는 provenance/mapping/stale 계약을 고정한다.

상세 owner:

```text
VehicleBuilderProposalSpec.md v0.1.1
```

### 핵심 결과

```text
accepted Evidence
→ normalized FACT / DERIVED / Unknown
→ complete private Profile prospective payload
→ existing Resolver / Diff / SourceTrace / Validation
→ VB-P0-05 reviewed commit/apply
```

Field disposition:

```text
EvidenceDirect
EvidenceDerived
BaselineInherited
GameBias
PlannedTypedGap
UnsupportedExact
```

Profile은 sparse 생성하지 않는다. existing Profile을 private prospective payload로 clone하거나, Profile이 없으면 current Resolver effective value를 complete seed로 사용한다.

Feel field는 Reference physical value를 `NeutralValue` anchor로 사용한다. Low/High를 FACT로 복제하지 않으며 기존 envelope가 없을 때만 Low=Neutral=High의 Reference-locked envelope를 explicit GAME_BIAS로 제안한다.

### UE 5.8 Transmission exact mapping — 확정

UE 5.8 `FVehicleTransmissionConfig` / `Chaos::FSimpleTransmissionConfig` 기준 mapping:

```text
bUseAutomaticGears
bUseAutoReverse
Forward / Reverse Gear Ratios
FinalRatio → FinalDriveRatio
ChangeUpRPM
ChangeDownRPM
GearChangeTime
TransmissionEfficiency
```

`Chaos::ETransmissionType`은 exact `Manual / Automatic` 두 값만 가지므로 실차 Transmission architecture와 shift-control mode를 분리한다.

```text
4AT → TorqueConverterAutomatic + Automatic + ExpectedForwardGearCount=4
6MT → ManualGearbox + Manual + ExpectedForwardGearCount=6
CVT → simple discrete Chaos transmission으로 exact 표현 불가 / UnsupportedExact
```

`4AT`라는 fact만으로 실제 gear ratios/final drive/shift RPM/efficiency를 생성하지 않는다. Unknown은 Unknown으로 남긴다.

Future typed schema:

```text
Drivetrain + VehicleMovementConfig
- bUseAutomaticGears
- bUseAutoReverse
- TransmissionRatios : dedicated atomic ratio-set
- FinalRatio
- ChangeUpRPM
- ChangeDownRPM
- GearChangeTime
- TransmissionEfficiency

TransmissionRatios
- ForwardGearRatios : TArray<float>
- ReverseGearRatios : TArray<float>
```

Gear count는 별도 중복 저장하지 않고 ratio array length에서 derive한다. Evidence의 expected gear count와 array length가 충돌하면 commit blocker다.

Reverse ratio는 Evidence에서 magnitude+direction으로 normalize한 뒤 Chaos에는 negative ratio로 materialize한다.

### 첫 mapping 경계

```text
VehicleBase
= Curb Mass→Base Mass, GVWR→Max Gross, Height→ChassisHeight
+ future ChassisWidth / Reference tire-derived wheel geometry fallback

Drivetrain
= FWD/RWD/AWD semantic mapping + actual split only
+ Transmission exact mapping

Performance
= Peak Torque→EngineMaxTorque Neutral
+ actual max/limiter RPM·Idle·Redline·Cd 등 same-semantic fact만
+ TorqueCurve shape는 BaselineInherited / fidelity partial

Handling
= suspension/tire/brake/steering taxonomy 이름만으로 Chaos numeric 추론 금지
= same-semantic numeric evidence만 direct/derived mapping
```

COM은 current 4 Profile typed owner가 없어 `PlannedTypedGap`으로 남기며 추정하지 않는다.

### Dry-run

2027 Kia Morning 1.0 / FWD / 4AT / 175/65R14 accepted sample에서:

```text
975 kg → BaseVehicleMassKg proposal
95.124505 Nm → EngineMaxTorque Neutral
FWD → FrontWheelDrive + front powered
4AT → Automatic + expected 4 forward gears
GVWR / actual gear ratios / final drive / redline Unknown 유지
3750 torque-peak RPM / 6200 power-peak RPM을 EngineMaxRPM으로 오용하지 않음
```

**VB-P0-04 Design PASS / Implementation 0.**

C++/UE Asset/Profile commit/VehicleData Apply/Save/Build는 수행하지 않았다.

---

## 8. VB-P0-05 — Vehicle Record / Layout / Physics Apply Flow — TECHNICAL PASS

### 구현 결과

새 차량과 기존 차량 보완을 같은 Builder 흐름으로 연결했다.

```text
Entry A: NewVehicle
→ existing CreateVehicleRecords로 VehicleData + Recipe core 생성
→ Builder companion R2 operation으로 Evidence + private Profile 4종 Missing 생성

Entry B: CompleteExisting
→ managed Recipe/current VehicleData 상태 검사
→ existing valid companion 보존
→ Missing / Stale / Conflict 판정
→ Missing private Profile은 complete initial typed payload로만 seed
→ current/prospective ResolvedDefinitionHash equality가 깨지면 Block
```

기존 `CreateVehicleRecords`의 Definition+Recipe core 계약은 바꾸지 않았다. Builder가 normal path에서 만드는 최소 7 Asset 계약은 core 2개 + companion 5개 orchestration으로 충족한다.

### typed authoring / apply

```text
VehicleData
= ChassisWidth + complete UE 5.8 Transmission fields

Drivetrain Profile
= vehicle-specific Transmission numeric owner

TransmissionRatios
= Forward/Reverse를 하나의 atomic typed field로 취급
= Registry127 / ResolverContractRevision 2

Evidence
= UCFVehicleRefEvidence Editor-only DataAsset
= FACT / DERIVED / GAME_BIAS + Claim/Conflict/Unknown
= SHA-256 + Unicode NFC deterministic fingerprint

AI proposal commit
= fresh Evidence + consumed Canonical Claims
+ Recipe/Target + OwnerRecipeId + 4 Profile fingerprints
+ prospective Shared Resolver result
+ exact AuthoringWrite approval scope
→ private 4 Profile one-transaction commit

Target Apply
= 기존 Resolver / Materializer / FCFVehicleApplyService 단일 writer lane 유지
= raw VehicleData write / shared Profile mutation 우회 없음
```

### runtime / validation

`ApplyVehicleMovementConfig()`은 complete Chassis/Transmission을 먼저 검증하고 실패 시 Movement mutation 0이다. valid setup이 실제 live setup과 다르고 physics state가 존재하며 mesh가 simulating일 때만 linear/angular velocity를 보존해 `RecreatePhysicsState()`를 수행한다. `UCFVDAValidator`도 같은 Chassis/Transmission complete validation을 수행한다.

HUD Gear는 계속 actual Chaos Runtime `GetCurrentGear()`를 읽는다. fake gear 계산은 추가하지 않았다.

### 검증

```text
Official BuildEditor PASS
Focused Automation 2/2 PASS
- Evidence.DeterministicFingerprint
- Foundation.Registry127

Persisted CFVehicleData legacy Transmission audit 11/11 PASS
- _Legacy 포함 모든 대상이 current atomic FCFVehicleTransmissionRatios로 정상 materialize

PhysicsState lifecycle Source audit PASS
- unconditional recreate 없음
- actual setup change + valid live physics 조건에서만 recreate
- velocity preserve
```

Focused test wrapper도 최종 terminal `succeeded / exit_code 0 / SUCCESS_COUNT 2 / FAILURE_COUNT 0`으로 종료됐다.

production Asset에 실제 Builder companion 생성/Profile commit/VehicleData Apply/Save는 validation 중 수행하지 않았다. Raw DA 편집은 계속 normal path가 아니라 Advanced escape hatch다.

---

## 9. VB-P0-06 — Gameplay Defaults / Hardpoint / Fitting Guidance — TECHNICAL PASS

### 구현 결과

Physics authoring을 다시 열지 않고 기존 Authoring truth 위에 Builder Step 6 전용 **R0 read-only completeness/guidance**를 추가했다.

```text
FCFVehicleAuthoringService::ReadBuilderGameplayGuidance()
→ existing Recipe snapshot
→ existing Profile snapshot set
→ existing AssetReader snapshot
→ existing Pure Resolver result
→ current Target VehicleData
→ Builder workflow completeness projection
```

병렬 Builder Validator, raw VehicleData writer, 자동 Apply/Save는 추가하지 않았다.

### 8개 completeness 영역

```text
Durability
Defense
DestroyedFx
Hardpoints
MountProfiles
DriveState
WheelVisual
FittingMass
```

각 영역은 다음 상태만 반환한다.

```text
Complete
Optional
NeedsReview
Blocked
```

`Optional`은 선택 기능 미사용을 정상 상태로 인정하며 Step 전체 blocker가 아니다.

### Gameplay defaults

```text
Durability
= Recipe Explicit 또는 VehicleBase Profile의 유효한 양수 MaxHealth가 필요
= Runtime fallback으로 authoring 누락을 숨기지 않음

Defense / DestroyedFx
= UseProfile / ExplicitAsset / ExplicitNone current semantic contract 유지
= 없음/ExplicitNone은 선택 기능이면 Optional
= 잘못된 type/path만 Blocked
= 임의 Defense/Fx Asset 생성 없음

DriveState
= ProjectDefault는 정상 Complete
= VehicleSpecific일 때만 DriveState Profile required

WheelVisual
= Recipe semantic policy + VehicleBase baseline 재사용
= Wheel center 자동검출 없음
= VB-P0-03 USER/Asset authority 유지

FittingMass
= Base/Gross 둘 다 0이면 current optional/no-fitting 계약 허용
= 사용하는 경우 Base > 0, Gross >= Base
= Gross mass 자동 추정 없음
```

### Hardpoint / Mount USER authority

Hardpoint 실제 위치는 계속 USER가 Static Mesh Editor에서 배치한 Socket이 authority다.

```text
Builder가 하지 않는 것
- Socket 생성
- Socket 이동
- Mesh surface 분석
- Hardpoint 자동배치
- 자동 Save
```

Builder는 stable `LocationSlotId`, `MountProfileId`, `LocationSlotRef`, configured Socket 이름과 current existence만 검사하고 정확한 수동 작업 guidance를 반환한다.

NewVehicle에서 선언된 Hardpoint Socket이 없으면 `NeedsReview`다. `HP_<LocationSlotId>` 형태의 권장 이름을 보여줄 수 있지만 실제 위치는 추론하지 않는다.

### Existing Vehicle Completion

기존 차량은 강제 Socket migration을 하지 않는다.

```text
Existing Target stored Hardpoint
+ 같은 stable LocationSlotId
+ current resolver candidate와 LocalLocation/LocalRotation conflict 없음
→ stored LocalTransform accepted
→ 기존 배치 보존 가능
```

반대로 Existing Hardpoint/Mount collection이 current candidate와 다르면 "아마 보존됨"으로 추정하지 않고 `NeedsReview`로 표시한다.

P0-06은 pending Gameplay diff를 보고만 하며 실제 Definition Apply는 VB-P0-07이 소유한다.

### 검증

```text
Final Official BuildEditor
= PASS
= job 12e537e016b64a179a813a4929f584e8
= exit code 0

Focused Automation
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance
= 1/1 PASS
= SUCCESS_COUNT 1
= FAILURE_COUNT 0
= engine exit code 0
= process job e9f180c8dcd34aa489bb12713bba8a52

Production mutation during validation
= VehicleData 0
= Recipe/Profile persistent write 0
= Defense/Fx/Fitting Asset 0
= StaticMesh Socket 0
= Save 0
```

---

## 10. VB-P0-07 — Final Review / Validation / Undo — Technical PASS

### 구현 결과

Final Review는 별도 Validator나 Target writer를 만들지 않는다.

```text
ReadBuilderFinalReview()
= ResolveVehiclePreview
+ ReadValidation
+ ReviewExternalDrift
+ ReadBuilderGameplayGuidance
+ Shared Resolver FieldDiff
+ BuildDiffHash
+ accepted Reference Evidence provenance
```

Final Review compact summary:

```text
Validation Warning / Blocked / Error
Gameplay NeedsReview / Blocked
External Drift 존재 여부
Target Diff count + exact DiffHash
Consumed canonical Evidence Claim provenance
  FACT N
  DERIVED N
  GAME_BIAS N
Automatic Save = Off
```

현재 persistent schema에는 final field→Claim provenance map이 없으므로 `FACT-backed fields N`처럼 field count를 추정하지 않는다. provenance는 latest accepted Evidence binding의 `ConsumedClaimIds`를 fresh Evidence에서 다시 검증한 canonical Claim count만 표시한다.

### Apply 계약

```text
Target Diff > 0
+ Resolve Success
+ Validation / Gameplay blocker 0
+ External Drift 0
+ fresh Evidence provenance
→ bCanApply
→ existing BuildApplyApprovalProposal() exact DefinitionApply ProposalHash
→ explicit approval
→ Apply 직전 Final Review 전체 fresh 재검사
→ ApplyResolvedVehicle()
→ FCFVehicleApplyService 단일 writer lane
```

raw VehicleData write, shared Profile mutation, automatic retry, automatic Save를 추가하지 않는다.

### Undo 계약

성공한 Builder Apply 직후 exact UE `TransactionId`, Recipe/Target identity, pre-Apply Target Definition hash와 Recipe AppliedState를 transient `FCFBuilderUndoToken`에 묶는다.

```text
current Editor lifetime에서 Builder가 발급한 exact token
+ explicit UndoScopeHash approval
+ token TransactionId == current Undo stack top
→ Unreal standard Undo 허용

intervening Editor transaction 존재
→ StateChanged
→ unrelated transaction을 대신 Undo하지 않음
```

Undo 성공 후 Target full Definition hash와 Recipe AppliedState를 pre-Apply evidence와 exact 비교한다.

### 보호 경계

- `UCFVDAValidator` / Data Authoring Validation / Diff / External Drift / Apply approval을 재사용한다.
- Hardpoint 실제 Socket 위치는 USER authority다.
- auto Socket 생성·이동·배치와 auto Save는 없다.
- Existing Vehicle Completion baseline-preservation은 유지한다.

### 검증

```text
Official BuildEditor
= PASS
= ff4dd50981464710876a0693d2287432
= exit 0

Focused Automation
= CarFight.DataAuthoring.CF_FQ_040.VB_P0_07.FinalReviewApplyUndo
= 1/1 PASS
= SUCCESS_COUNT 1 / FAILURE_COUNT 0 / engine exit 0
= df306775e2f245aabccd307d741f6bda

Flow coverage
= Final Review + provenance
→ explicit Apply
→ post-Apply diff0
→ unrelated Editor transaction
→ guarded Undo StateChanged 차단
→ unrelated standard Undo
→ exact Builder guarded Undo
→ pre-Apply Target/AppliedState 복원
→ pending Diff 복원

Production validation mutation
= persistent production VehicleData/Profile/Evidence/Asset 0
= StaticMesh Socket 0
= Save 0
```

---

## 11. VB-P0-08 — Technical Driving Benchmark

### 목적

AI가 만든 수치가 실제 Runtime에서 Reference 성능 방향과 맞는지 USER 감각보다 먼저 수치로 검증한다.

### 1차 benchmark 후보

```text
Runtime Mass
0→50 km/h
0→100 km/h
Top Speed
100→0 km/h Braking
Turning Radius
Steady-turn Yaw response
```

### 2차 가능 후보

```text
Slip Angle / wheel slip
Lateral acceleration
Suspension travel
Settling time
```

RuntimeRead로 관측 가능한 값은 AI Technical Validation으로 수행한다.
관측 공백이 있으면 Product debug code를 먼저 추가하지 말고 현재 capability gap을 별도 판정한다.

### 구현 결과

```text
Automation
= CarFight.VehicleBuilder.CF_FQ_040.VB_P0_08.TechnicalDrivingBenchmark

Input
= saved VehicleData object path
= optional saved FittingData object path

Runtime
= fresh PIE
= deferred BP_CFVehiclePawn spawn
= BeginPlay 전 VehicleData / optional Fitting 주입
= Fitting 있으면 FittingSnapshot.TotalVehicleMassKg
= Fitting 없으면 VehicleData.BaseVehicleMassKg

Deterministic condition
= UE 5.8 Source-confirmed -UseFixedTimeStep -FPS=60
= metric phase reset마다 production FCFChaosVehicleMassRuntime으로 Chaos PhysicsState rebuild
= forward start explicit gear 1
= Automatic은 Chaos auto-shift, Manual은 configured ChangeUpRPM/ChangeDownRPM technical driver
= 기존 map fixture collision/physics/tick 격리
= peak telemetry는 acceleration/top-speed active phase로만 제한

Output
= UE/Saved/CarFight/VehicleBuilderBenchmarkResult.json
= Reference threshold asserted false
= USER driving feel asserted false
= Save 0
```

최종 fitted repeat canary:

```text
B = b87b3bfb96694c05af00b1923e2e9b83
C = ece88090da5c42bdbc7aaa640da22ecd
둘 다 exit 0
Mass 1570kg
0→50 3.716667s
0→100 19.433334s
Peak 129.324310km/h
100→Idle 2.400002s / 32.669586m
Yaw 21.206821deg
Turning Radius 8.517635m
= exact repeated values
```

no-fitting BaseMass canary:

```text
process 0006a45bd3eb456581a2f125ff612d8e
exit 0
Mass 1000kg
0→50 3.900000s
0→100 unavailable
Peak 89.953712km/h
Yaw 17.858885deg
Turning Radius 8.799048m
```

제한:

```text
100km/h 미도달 차량은 100→Idle braking result를 unavailable로 기록하며 기술 실패가 아님
fitted representative canary는 100km/h 도달과 100→Idle braking runtime branch 관측 완료
test-only 성능 boost는 금지
실제 Reference 비교와 USER 주행감은 VB-P0-09에서 수행
```

### PASS 의미

실차와 똑같은 시뮬레이션을 의미하지 않는다.

```text
Reference fact/range와 큰 방향이 일치하고
CarFight 차량 간 차이를 만들 만큼 수치적 특성이 구분되며
Validator/Runtime invariant가 정상
```

을 의미한다.

---

## 12. VB-P0-09 — End-to-End USER Acceptance

### 시나리오

USER가 신규 차량 1대를 빈 상태에서 제작한다.

단, 실제 신규 차량 E2E에 들어가기 전에 `WheelSizeAuthorityPlan.md`의 WSA-P0-01~06 Technical Gate를 닫는다. 기존 Step 1~8 Technical PASS 자체를 다시 수행하는 것이 아니라, Step 2~5/7의 Wheel Size 계약을 additive 교정한다.

예상 흐름:

```text
Segment 선택
→ AI Reference 조사 승인
→ Canonical Default Wheel X/Z 100cm 준비
→ Mesh 지정
→ USER가 휠하우스를 보면서 Wheel Socket 위치/회전/Scale 직접 작성
→ Socket Scale + actual Wheel Mesh Bounds capture
→ Visual Scale + Physics Radius/Width 동일 source derive
→ AI Physics Proposal review — Socket-derived Wheel Radius/Width는 독립 AI proposal 제외
→ Gameplay defaults review
→ Apply
→ Technical benchmark
→ USER Driving
```

Wheel Size authority는 USER Socket Scale이다. AI/코드가 휠하우스를 보고 적정 타이어 크기를 자동 선택하지 않는다.

### 2026-09-01 Engine / Shift Authoring Hardening — Design Lock

다음 구현 Gate는 ESH-01~06으로 관리한다. 세부 계약 owner는 `VehicleBuilderProposalSpec.md v0.1.10`, Research vocabulary owner는 `VehicleRefEvidenceSpec.md v0.1.3`다.

`ESH-01` TorqueCurve typed owner/schema → `ESH-02` Engine Curve proposal → `ESH-03` WheelTorqueCrossoverShift → `ESH-04` dedicated high-speed benchmark authority → `ESH-05` Wagon retest → `ESH-06` USER Driving PASS 순서다.

현재 checkpoint:

```text
ESH-01  Technical PASS
ESH-02  Actual Wagon Engine Curve Target DefinitionApply PASS
        post-ESH02 checkpoint TargetHash = 83c69e52522dc72649c32477b5aad220
        Persisted bUseEngineTorqueCurve = true
ESH-03  WheelTorqueCrossoverShift@1 Diagnostic Technical PASS
        raw common recommendation = 6222RPM / theory diagnostic 보존
        transient A/B/C = 4500 / 5500 / 6222
        P0 fixed-common GAME_BIAS = 5500RPM USER Review PASS
        Profile/Receipt commit PASS / Target DefinitionApply PASS
        current TargetHash = 0e5b48e8dcd39deba441da9237218be6
        Persisted Product ChangeUpRPM = 5500 / ChangeDownRPM = 2000
        post-Apply exact2 6c204a6a29ba44f19fe2ba6ba327aa3f PASS
ESH-04  Dedicated High-Speed Benchmark Authority Technical PASS
        production no-fitting mass = 1500 configured / 1767.377 actual kg
        baseline 4500 = T100 6.733 / T150 15.967 / T200 58.367 / Peak206.793 / G6
        candidate 5500 = T100 6.100 / T150 13.167 / T200 31.183 / Peak206.700 / G6
        raw 6222 = T100 6.083 / T150 12.850 / T200 31.683 / Peak216.116 / G5
ESH-05  Persisted 5500 Retest Technical PASS
        process 9abcb642fe7241b585a7e7fcc1637296 / override=false
        effective 5500/2000 / T100 6.100 / T150 13.167 / T200 31.183 / Peak206.700 / G6 / Stable true
ESH-06  USER Driving PASS
        TargetHash = 0e5b48e8dcd39deba441da9237218be6
        RunId = 256cd822-419e-4a3e-ac23-388b570d78c2
        RecipeId = 05F69DD34990A868DEFFD29C1AF5B2F9
        benchmark process 1a8e03cfbe804e52a5c98be74b07d24e PASS
        exact local USER acceptance token verified
VB-P0-09 End-to-End USER Acceptance PASS
```

ESH-02 actual Wagon Curve는 350Nm @ 1800–4800rpm과 250PS @ 5400–5700rpm을 FACT anchor로 사용하고, 900/6500은 `SparseAnchorEngineCurveBias@1` GAME_BIAS다. Engine Curve Target Apply까지 완료됐으므로 ESH-03/04 technical lane은 진행 가능하다.

raw 6222RPM은 adjacent wheel-torque least-squares diagnostic으로 보존하되 자동 Product 값이 아니다. P0의 하나뿐인 `ChangeUpRPM`은 high-speed WOT acceleration과 상단 gear usability를 함께 검토해 5500 GAME_BIAS로 USER 승인·Product 적용했다. persisted no-override retest가 transient 5500 결과를 동일하게 재현했으므로 technical 선택은 닫혔다. 제조사 전자식 최고속 제한은 Evidence에만 보존하고 CarFight Runtime에는 적용하지 않는다.

### 2026-09-02 ESH-03/04/05 High-Speed Shift Review — Technical Checkpoint

- production no-fitting mass와 다른 1886kg force-reapply benchmark를 authority에서 제외하고 `M_VehicleBenchmark` longitudinal authority를 확립했다.
- 4500/5500/6222 transient A/B/C는 Product Asset 저장 mutation 없이 수행했고, raw WheelTorque 6222는 theory evidence로 보존했다.
- USER가 fixed-common 5500 GAME_BIAS를 승인해 exact Profile/Receipt commit과 DefinitionApply로 persisted Product를 4500→5500으로 전환했다. 8AT/Final3.20/ChangeDown2000/Engine Curve는 유지했다.
- ESH-05 no-override retest는 current TargetHash `0e5b48e8dcd39deba441da9237218be6`에서 transient 5500과 동일한 T100/T150/T200/Peak/G6 결과를 재현했다. next는 ESH-06 USER Driving PASS다.

### 2026-08-27 Technical Preparation Checkpoint — Step 2~4 PASS

USER Acceptance를 시작하기 위한 Guided Shell runtime 연결을 선행했다.

```text
Step 2 MeshPrep
= fresh FCFVehicleAssetSnapshot
= Chassis + FL Wheel 최소 필수
= optional FR/RL/RR missing 허용 / 지정 시 resolve+bounds 검사

Step 3 SocketGuide
= effective Wheel role name 4개
= current Chassis 4/4 found + distinct
= custom name 허용
= Wheelbase/FrontTrack/RearTrack read-only 표시
= Hardpoint requiredness 추론 0

Step 4 LayoutCapture
= capture 실행 0
= current Target VehicleLayoutConfig vs current AssetSnapshot direct exact equality
= NotCaptured Ready / Current Complete / mismatch Stale
= auto Socket/Save 0

Validation
= Official Build 1b1aa29b80c341c6a6289ac2693413e9 PASS
= BuilderShell focused c6ab98e244634dd1a7fbcb84b290d4e3 1/1 PASS
= baseline Complete / optional Wheel / socket-transform Stale / duplicate-role Blocked 검증
```

이 checkpoint는 USER PASS를 의미하지 않는다. Step 1 Reference/Companion과 Step 5~8 Guided action 연결은 이후 Technical PASS까지 완료되었지만, 실제 신규 차량 E2E 직전에 Wheel Size Authority 설계가 추가되었다. 기존 Step PASS는 반복하지 않고 WSA-P0-01~06 additive 교정 후 실제 신규 차량 1대의 E2E USER 확인으로 간다.

### 2026-08-27 Step Extensibility Hardening — Technical PASS

완성 후 기능 추가·제거·수정의 유지보수 비용을 낮추기 위해 fixed-index 결합만 선제 교정했다.

```text
semantic identity
= ECFVehicleBuilderStepId

single composition owner
= GetVehicleBuilderStepDefinitions()
= current 구성 / 순서 / 제목

state evaluation
= Step별 Evaluate*Step()
= RebuildStepStates()는 orchestration만 담당
= current definition 순서를 순회해 EvaluateStepById(StepId) dispatch

lookup/write
= FindStepView(StepId)
= SetStep(StepId, ...)

navigation
= StepViews.Num()
= literal 8 dependency 0

Step-specific UI
= numeric current index 의미 비교 0
= current StepId 사용

focused regression
= current baseline core Step 존재를 StepId로 검증
= Mesh/Socket/Layout 기존 상태 의미 보존
```

향후 변경 절차:

```text
새 Step 추가
→ StepId + definition row + evaluator + 필요 시 StepId UI/test

순서 변경
→ definition row 이동
→ navigation과 evaluator dispatch 순서가 함께 자동 변경

Step 제거/통합
→ current definition에서 제외
→ 기존 StepId는 다른 의미로 재사용하지 않음

Step 내부 규칙 수정
→ 해당 evaluator + focused test 중심
```

과설계 금지:

```text
generic plugin registry 0
Blueprint workflow engine 0
Step DataAsset 0
persistent progress graph 0
```

검증:

```text
Official Build 8e1c55defcbc49108c4c402964a33477 PASS / exit 0
BuilderShell focused 0e2b3bd9260446c084e0523427283e02 1/1 PASS / failure 0
Product Asset/Socket/Save mutation 0
```

이 hardening은 P0 baseline Step 기능/순서를 바꾸지 않았으며 VB-P0-09 USER E2E는 계속 Pending이다.

### 2026-08-27 Step 1 Reference Evidence / Companion USER Flow — Technical PASS

```text
AI Research handoff
= Project Saved ResearchDraft.json transient JSON
= exact RecipeId + TargetDefinitionPath binding
= invalid schema/identity/evidence payload fail-closed

Evidence
= initial FACT/DERIVED research validation
= empty Evidence creation 금지
= fixed NewEvidenceId + prospective EvidenceFingerprint
= preview ProposalHash와 commit readback exact binding

Companion
= existing valid Evidence/private Profile 보존
= Missing만 R2 one-transaction 생성
= CompleteExisting baseline preservation
= USER explicit OwnershipWrite approval
= auto Save/Apply 0

Shell
= Reference summary
= AI Research Draft 불러오기
= Companion 생성 내용 검토
= 이 Reference Set으로 진행
= token: RecipeId + EvidenceId + EvidenceFingerprint

Resume/Stale
= fresh VM에서 local review token 복원
= exact current fingerprint면 Step 1 Complete
= Evidence semantic fingerprint drift면 Step 1 Stale
= mutation approval은 복원하지 않음

Step boundary
= Profile 4종 completeness는 Step 1 completion hard condition이 아님
= Step 5 Physics Proposal review authority 유지
```

검증:

```text
Official Build 06370173f2c64a1ab4dd3d235bd46c18 PASS / exit 0
BuilderStep1Reference PASS
BuilderShell PASS
BuilderCompanionFlow PASS

broad DataAuthoring 71건 중 unrelated 1 FAIL
= DAUTH_P0_08.Batch.AllowlistProjection
= typed Profile schema leaf-count stale assertion
= current Step 1 affected regression은 모두 PASS
```

이 checkpoint는 Step 1의 Technical PASS이며 USER가 실제 신규 차량을 끝까지 만들고 주행감을 확인한 End-to-End USER PASS는 아니다.

### 2026-08-27 Step 5 Physics Proposal Guided Flow — Technical PASS

```text
AI Physics Draft
= UE/Saved/CarFight/VehicleBuilder/PhysicsDraft.json
= exact Recipe / Target / USER-reviewed Evidence binding
= complete private 4 Profile typed proposal
= consumed canonical Claim IDs + user-facing vehicle characteristic summary

Prerequisite
= Step 1~4 Complete
= current Evidence exact
= private 4 Profile exact Builder owner

Preview / USER approval / Commit
= PreviewBuilderProfiles mutation0
= domain별 current/prospective fingerprint 차이 표시
= explicit USER AuthoringWrite
= CommitBuilderProfiles existing R1 lane
= Target VehicleData Apply 0 / Save 0 / shared Profile mutation 0

Completion / Resume
= persistent BuilderCommitReceipt + current Evidence/Claim/Profile/Resolved hash/Resolver revision exact match → Complete
= fresh VM에서도 persistent current truth로 Complete 재계산
= transient PhysicsDraft/old mutation approval 복원 0
= private Profile/Evidence/receipt drift → Stale

Guided UX
= USER가 Chaos numeric field를 직접 입력하는 정상 flow 아님
= AI summary + 근거 Claim + 변경 Profile domain을 먼저 review
= Definition Apply는 Step 7 owner
```

검증:

```text
Official Build 3d65b8e8b6d04f498ceaf6358618c6a0 PASS / exit 0
broad DataAuthoring 72 performed / 71 PASS / 1 unrelated stale assertion
affected PASS:
- BuilderStep5Physics
- BuilderStep1Reference
- BuilderShell
- BuilderProfileCommit
- BuilderCompanionFlow
unrelated FAIL = DAUTH_P0_08.Batch.AllowlistProjection 79→89 leaf-count expectation stale
```

이 checkpoint는 Step 5 Technical PASS이며 USER 신규 차량 E2E/Driving Feel PASS가 아니다.

### 2026-08-27 Step 6 Gameplay Setup Guided Flow — Technical PASS

```text
Provider
= existing ReadBuilderGameplayGuidance R0
= 8영역 authoritative completeness projection

Prerequisite
= Step 5 current Complete

Shell state
BlockedCount > 0 → Blocked
NeedsReviewCount > 0 → Ready / USER manual review
bCanCompleteGameplayStep → Complete

USER work
= Hardpoint/DestroyedFx Socket 위치는 USER authority
= SocketGuidance 그대로 표시
= 자동 생성·이동 0
= fresh refresh로 재검사

Apply boundary
= Step 6 read-only
= PendingGameplayDiff는 표시만 함
= Target VehicleData Apply는 Step 7 Final Review
= Save 0
```

검증:

```text
Official Build d01056d228214ee8bbd06b154768e96d PASS / exit 0
broad DataAuthoring 72 performed / 71 PASS / 1 unrelated stale assertion
affected PASS:
- VB_P0_06.GameplayGuidance
- VB_P0_09.BuilderStep5Physics (Step6 integration/resume/prerequisite lock 포함)
```

이 checkpoint는 Step 6 Technical PASS이며 USER 신규 차량 E2E PASS가 아니다.

### 2026-08-27 Step 7 Final Review / explicit Apply Guided Flow — Technical PASS

```text
Review
= existing ReadBuilderFinalReview R0
= Validation + Drift + Gameplay + Provenance + 전체 Field Diff

Apply
= USER dialog 직전 fresh review
= exact ProposalHash explicit DefinitionApply
= existing ApplyBuilderFinalReview R3
= backend fresh re-review + stale hash fail-closed
= auto Save/retry 0

Undo
= Apply가 발급한 exact current-lifetime guarded token
= exact top transaction + post-Apply state guard
= unrelated Editor transaction/state drift는 Undo하지 않고 차단

Step state
= blocker/drift → Blocked
= diff + canApply → Ready
= diff0 + canComplete → Complete
```

검증:

```text
Official Build f9a9ced62f1545f884ceefdfd2a966ab PASS / exit 0
focused BuilderStep7FinalReview a7efe7e368cd42f5b57a18c9b66f756f 1/1 PASS
Step 1~6 suite replay 0
```

이 checkpoint는 Step 7 Technical PASS이며 USER 신규 차량 E2E/Driving PASS가 아니다.
### USER PASS 질문

1. 다음에 무엇을 해야 하는지 툴만 보고 알 수 있는가?
2. 필요한 Mesh/Socket 네이밍을 외우지 않아도 되는가?
3. Chaos 세부 수치를 직접 이해하지 않아도 차량을 만들 수 있는가?
4. 버튼과 설정이 한꺼번에 펼쳐져 복잡하게 느껴지지 않는가?
5. 생성된 차량이 Reference Set과 다른 차량에 비해 실제로 구분되는 운전 특성을 갖는가?

USER가 확인하지 않은 항목은 USER PASS로 기록하지 않는다.

---

## 13. VB-P0-10 — Current System Promotion — PASS

### 완료 결과

```text
Vehicle Builder current architecture
→ Document/Systems/Vehicles/VehicleBuilder.md v1.0.0

Data Authoring
→ Builder Backend + Advanced Workspace 역할 고정

FeatureQueue
→ CF-FQ-040 P2 / Done

ActiveWork / Plan Index
→ Ready/current route 제거

대표 Plan / Roadmap
→ Historical + Retained Path

Archive Index
→ retained-path discovery 등록
```

### Historical Gate

```text
G0 Evidence = PASS
G1 Current Knowledge Promotion = PASS
G2 Current Route Cleanup = PASS
G3 Reference Preservation = PASS
G4 Historical = PASS
G5 Optional Physical Move = NOT RUN / 별도 maintenance
```

### 보호

CF-FQ-038의 완료 evidence를 Vehicle Builder 완료 evidence로 재포장하지 않는다.
Builder 자체 End-to-End USER acceptance는 VB-P0-09에서 별도 PASS했으며, CF-FQ-038의 DEL6/UA-08 잔여 lifecycle은 Paused로 유지한다.
ESH를 다시 열거나 Wagon을 재튜닝하지 않는다.

---

## 14. Historical Closure / Current Route

```text
CF-FQ-040 = Done
Current owner = Document/Systems/Vehicles/VehicleBuilder.md v1.0.0
대표 Plan/Roadmap = Historical + Retained Path
Current next gate = 없음
```

VB-P0-07은 existing Validation / External Drift / Diff / explicit DefinitionApply / guarded Undo까지 Technical PASS이며, post-review hardening으로 persistent provenance receipt / deterministic approval replay / post-Apply state guarded Undo / one-resolve Final Review가 추가 검증됐다.

보존 조건:

```text
VB-P0-00~07 PASS replay 금지 — 새 관련 failure가 있을 때만 재개
Builder Profile/Companion Preview→Commit fresh approval hash는 transient UObject allocation identity와 무관하게 deterministic해야 함
Final Review provenance는 persistent BuilderCommitReceipt + current private 4 Profile fingerprint exact match 필요
Guarded Undo는 exact top transaction + current post-Apply Target/semantic Recipe/AppliedState exact match 필요
VB-P0-04 provenance/Unknown/complete seed 계약 유지
Unknown 숫자 자동 보충 금지
sparse private Profile 금지
shared Profile mutation / raw VehicleData write 금지
Hardpoint 실제 Socket 위치 = USER authority
auto Socket 생성·이동·배치 금지
Existing Vehicle Completion baseline-preservation 유지
automatic Save 금지
HUD Gear는 actual Chaos current gear authority 유지
```

VB-P0-08은 Technical PASS다. VB-P0-09 Step 1~8 Guided connection과 actual Wagon End-to-End USER Acceptance도 PASS이며 Step 8은 fixed-60Hz benchmark authority + saved Target exact path/hash/RunId binding + active PIE transient selected-vehicle test-drive + explicit USER Driving PASS를 사용한다. VB-P0-10까지 완료됐으므로 CF-FQ-040 내부 남은 Gate는 없다.

Post-Review Hardening evidence:

```text
Persistent provenance
= Recipe BuilderCommitReceipt가 accepted Evidence/Claim set과 committed private 4 Profile fingerprint를 연결
= receipt-only R1 migration은 Profile payload/revision 0 / Target 0 / Save 0

Deterministic proposal replay
= BuilderProfileCommit prospective path authority = persistent Profile path + typed snapshot
= BuilderCompanion missing-profile prospective path authority = preview prospective persistent path + typed snapshot
= transient UObject path는 SourceSignature/ProposalHash authority에서 제외

Final Review / Undo
= one fresh Resolve → Validation/Drift/Gameplay/Diff projection
= provenance receipt mismatch 또는 Claim subset tamper → blocker
= nontransactional post-Apply Target/AppliedState drift → guarded Undo StateChanged

Latest validation
= Official Build 306b9c9be7f648a4b5a89b6eb13c7f05 PASS
= VB-P0-05 focused 4/4 PASS / 1be0c0853c8a43d1bf0b9063a85c13ce
= VB-P0-06 focused 1/1 PASS / 8a77ba7635024f4d8ac20c91244f65f4
= VB-P0-07 focused 1/1 PASS / 4385151a829643c4b35da5ff3df670e3
= failure 0 / Save 0 / production Asset mutation 0
```

---

## 15. Changelog

### v0.1.39 - 2026-09-02

- G5 Physical Move maintenance로 Roadmap과 supporting VehicleBuilder 문서를 `Document/Plan/Archive/VehicleBuilder/`에 함께 보존했다.
- 완료 Gate와 USER/Technical evidence는 변경하지 않고 Historical placement만 `Archived Path`로 전환했다.
- Current 구현 owner는 `Document/Systems/Vehicles/VehicleBuilder.md v1.0.0`이다.

### v0.1.38 - 2026-09-02

- `VB-P0-10 Current System Promotion` PASS를 반영해 CF-FQ-040을 Done으로 닫고 Roadmap을 Historical + Retained Path로 전환했다.
- Current owner를 `Document/Systems/Vehicles/VehicleBuilder.md v1.0.0`으로 고정하고 Data Authoring 역할을 Builder Backend + Advanced Workspace로 확정했다.
- Historical Gate G0~G4 PASS, G5 physical move NOT RUN으로 판정했다. current plan_repo dirty와 unrelated deletion을 보호하기 위해 파일 이동은 별도 maintenance로 분리한다.
- ActiveWork/FeatureQueue/Plan Index current route가 제거됐으며 CF-FQ-040 내부 next gate는 없다. CF-FQ-038 Paused 잔여 항목과 ESH/Wagon tuning은 재개하지 않는다.

### v0.1.37 - 2026-09-02

- ESH final audit correction을 완료했다. high-speed expected TargetHash는 이제 read-only `CFVehicleHashCheck` + existing Snapshot authority로 actual saved Target과 benchmark 전 exact 비교되며 mismatch는 Exit124 fail-closed한다.
- current Wagon hash positive PASS / wrong-hash pre-benchmark negative PASS 후 ESH-01 3/3, ESH-02 5/5, ESH-03 2/2 affected regression을 순차 PASS했다.
- fresh ESH-05 persisted run은 `target_hash_verified=true`, expected=observed `0e5b48e8dcd39deba441da9237218be6`, Peak206.700 / G6 / T20031.183 / Override false를 동일 재현했다. ESH-01~06을 Final Audit Clean PASS로 고정하고 next VB-P0-10은 유지한다.

### v0.1.36 - 2026-09-02

- ESH-06 actual Wagon USER Driving을 USER가 승인했다. 5단 약204km/h → 6단 → 약206km/h의 실제 체감이 ESH-05 persisted 206.700km/h/G6 benchmark와 정합했고 플레이를 막는 이상으로 판정되지 않았다.
- `USER 주행 PASS`가 RecipeId `05F69DD34990A868DEFFD29C1AF5B2F9` + current TargetHash + fresh RunId exact local token으로 저장된 것을 확인해 ESH-06과 VB-P0-09 End-to-End USER Acceptance를 PASS로 닫았다.
- next Gate를 `VB-P0-10 Current System Promotion / Advanced Authoring Role Lock`으로 전진했다. 이 promotion 전까지 CF-FQ-040은 P2/Ready이며 Done으로 확대하지 않는다.

### v0.1.35 - 2026-09-02

- ESH-06용 fresh Step 8 benchmark를 current persisted 5500 TargetHash에 binding해 PASS했다. RunId는 `256cd822-419e-4a3e-ac23-388b570d78c2`이며 USER feel 자동 판정은 0이다.
- managed Editor Ready까지 준비했으나 자동 PIE start는 current GoPyMCP lifecycle/write policy 경계에서 fail-closed했다. 다음 Gate는 USER가 Play를 시작한 뒤 current PIE에 Wagon을 transient 적용하고 직접 주행 승인하는 단계다.

### v0.1.34 - 2026-09-02

- ESH-03 fixed-common 5500 GAME_BIAS USER Review PASS와 exact Profile/Receipt commit + Target DefinitionApply PASS를 current checkpoint에 반영했다. current TargetHash는 `0e5b48e8dcd39deba441da9237218be6`, persisted ChangeUp/Down은 5500/2000이다.
- ESH-03 post-Apply exact2와 ESH-02/Builder affected exact5 regression이 모두 PASS했으며 raw 6222 recommendation은 theory diagnostic으로 보존한다.
- ESH-05 persisted no-override high-speed retest가 T100 6.100 / T150 13.167 / T200 31.183 / Peak206.700 / G6 / Stable true를 재현해 Technical PASS했다. next는 ESH-06 USER Driving PASS다.

### v0.1.28 - 2026-09-01

- ESH-01~06 Engine/Shift authoring hardening Gate를 추가하고 다음 차량부터 공통 적용한다.
- actual Wagon USER 주행 6단/약 200km/h를 반영해 기존 72.991km/h 결과를 최고속 evidence로 사용하지 않는다.
- 전자식 최고속 제한은 Evidence에만 보존하고 Runtime에는 적용하지 않는다.

### v0.1.27 - 2026-09-01

- actual Wagon Transmission remediation의 USER-approved DefinitionApply를 exact ProposalHash/DiffHash/pre-post DefinitionHash guard로 실행하고 Target+Recipe persistent save까지 완료했다. fresh persisted Target은 8단 forward + reverse3.992 + Final3.20이다.
- post-Apply benchmark `f1b8ca0c-1ec9-4cff-8162-0a180f412fea`에서 PeakSpeedGear 3으로 기존 1단 약 80km/h 고정 incident 해소를 확인했다. 0→50 3.00s / Peak72.991km/h / 0→100 미도달이라 전체 차량 성능은 별도 follow-up이 필요하다.
- post-Apply Transmission focused regression `d0f8bae7daf3487e9aebb57b652eb9bb` exact 9/9 PASS다.
- next Gate는 actual Wagon USER direct driving + 72.99km/h 안정/0→100 미도달 원인 판정이다. 이 확인 전에는 VB-P0-09 USER Driving PASS로 닫지 않는다.

### v0.1.26 - 2026-09-01

- actual Wagon WSA-P0-07 USER PASS를 반영해 Wheel Size Authority P0를 Complete로 전진했다. 이전 `WSA-P0-07 USER Acceptance Ready` projection은 폐기한다.
- actual Wagon은 Step 1~7 Apply/Save와 Step 8 runtime apply까지 완료된 상태를 current roadmap에 반영했다.
- next는 WSA 재검증이 아니라 fresh Resolver/adoption evidence refresh → 이전 WSA adoption fingerprint mismatch 재평가 → Vehicle Builder actual apply 재개다.

### v0.1.25 - 2026-08-29

- actual Wagon offsite AI-only preparation을 완료했다. exact Recipe/Target-bound `ResearchDraft.json`에 2026 Volvo V60 Cross Country KR representative Evidence와 complete private Profile 4종 initial seed를 준비했다.
- `ChassisWidth=185.0cm`는 Volvo 2026 치수표의 explicit body width 1850mm를 사용한다. exact peak torque/gear ratios는 미공개라 Unknown으로 유지하고 `bUseTransmissionConfig=false`로 임의 Transmission authoring을 금지한다.
- Front/Rear WheelClass actual generated class를 확정했다. WSA는 shared Wheel bounds + USER Socket Scale authority를 유지하며 AI Profile의 Reference wheel geometry는 disabled다.
- Draft consistency PASS, persistent UE Asset/Profile/Recipe/Target/Save mutation 0. 다음 순서는 `Step 1 USER Reference review → Companion explicit creation → Step 5 → Step 7 full Diff USER review/Apply → USER Save → Technical Driving + WSA-P0-07 USER PIE → driving feel approval`이다.

### v0.1.24 - 2026-08-28

- 실제 Wagon E2E feedback에서 `DA_Recipe_Wagon.AssetIntent.WheelMesh*` 미지정이 Step 2를 막는 UX 공백을 확인해, Step 2에 explicit StaticMesh picker + existing typed Recipe-only commit 경로를 연결했다. FL 필수 / FR·RL·RR 선택 및 FL fallback semantics는 기존 계약을 보존한다.
- Step 3은 새 semantic Step을 추가하지 않고 stable `SocketGuide`를 `소켓 준비 / Naming` 작업 단계로 강화했다. exact required Socket 이름/존재 판정/복사, current Recipe optional Socket 안내, Chassis Mesh Editor open을 제공하되 Socket 자동 mutation/Save는 0이다.
- 작업 대상 row는 identity 기반 stable accent와 관리 상태 badge를 사용한다. Build `ff8abe01a9fa4e6e90c785618ffccb91`, focused BuilderShell `fb4b91e909254180975b30db00c0e813` PASS. USER actual UI recheck Pending이며 WSA-P0-01 next gate는 변경하지 않는다.

### v0.1.23 - 2026-08-28

- WheelSizeAuthorityPlan v0.1.4의 final Source audit PASS를 반영했다. 신규 Socket mode의 resolver source ownership, mode-aware Gameplay Guidance, Step 5 AI wheel-geometry baseline-preserve가 구현 Gate에 포함된다.
- 다음 Gate는 그대로 WSA-P0-01 code/schema/helper 구현이며 기존 Step1~8 Technical PASS와 canonical Wheel asset PASS를 반복하지 않는다.

### v0.1.22 - 2026-08-28

- `WheelSizeAuthorityPlan.md v0.1.3` 재감사 결과와 canonical shared Wheel live Bounds PASS를 반영했다.
- WSA-P0-01의 Asset normalization sub-gate는 `100×25×100`, center≈0으로 PASS다. 남은 WSA-P0-01은 Data Schema/Registry/helper 구현이다.
- 기존 Step1~8 Technical PASS는 반복하지 않고 WSA-P0-01~06 additive 교정 뒤 actual new vehicle USER E2E로 간다.

### v0.1.21 - 2026-08-28

- USER 결정으로 `WheelSizeAuthorityPlan.md v0.1.1`을 CF-FQ-040/VB-P0-09의 정식 하위 설계로 연결하고 WSA-P0-00~07 Gate를 추가했다. v0.1.1은 existing AssetSnapshot/R6 Measurement/AssetAdoption 재사용으로 구현 범위를 축소한 Source-audited 설계다.
- WSA-P0-00은 Design PASS다. 다음은 `WSA-P0-01 Canonical Default Wheel + Data Schema & Shared Size Utility`이며 기존 Builder Step1~8 Technical PASS는 반복하지 않는다.
- 신규 정상 경로에서 USER Wheel Socket Scale이 타이어 크기 Authority이고, StaticMesh Bounds는 기계적 원본 치수 Source다. Visual Scale과 Physics Radius/Width는 같은 입력에서 파생한다.
- 현재 shared `/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL`은 X/Z 약 79.358cm라 canonical 100cm가 아니다. Wagon 실제 Socket Scale 작성 전에 one-time Default Wheel normalization이 필요하다.
- 기존 `bAutoScaleWheelMeshToRadius`는 Legacy compatibility로만 보존하고 Socket Scale과 동시 적용/곱셈을 금지한다.
- 이번 변경은 문서/설계만 수행했으며 C++/UE Asset/Runtime mutation은 0이다. 실제 신규 차량 E2E/P0-10 promotion은 계속 Pending이다.

### v0.1.20 - 2026-08-27

- VB-P0-09 Step 8 Technical Driving + USER Driving Guided connection을 Technical PASS로 닫았다.
- existing VB-P0-08 runner를 새 benchmark 구현 없이 재사용하고, saved Target exact path + current Target DefinitionHash + fresh RunId를 result provenance/USER acceptance identity로 연결했다.
- dirty Recipe/Target은 benchmark 전에 저장을 요구하되 Builder auto Save는 하지 않는다. current technical result 뒤 selected VehicleData transient duplicate만 active PIE Pawn에 적용한다.
- USER PASS는 actual PIE 적용 성공 뒤 explicit confirmation을 요구하고 exact RecipeId/TargetHash/RunId token으로 resume한다. 새 benchmark RunId 또는 Target drift는 이전 PASS를 재사용하지 않는다.
- Official Build `055dbddf74f342e9b10a92e1fb2542b8`, focused Step8 `aa2a365599fc46d484857894aa0df47a`, runner envelope canary `6135fbff338c47f8944dd5b8b02d5069` 모두 PASS. Step1~7 Technical PASS는 replay하지 않았다.
- 다음 Gate는 실제 신규 차량 1대 Guided Builder E2E + USER Driving Acceptance다. USER가 직접 운전하기 전 VB-P0-09 전체 PASS와 P0-10 promotion은 금지한다.

### v0.1.19 - 2026-08-27

- VB-P0-09 Step 7 Final Review / explicit Apply / guarded Undo Guided Flow를 Technical PASS로 닫았다.
- existing R0 Final Review의 full Diff/provenance를 Guided UX에 연결하고 USER-approved exact ProposalHash만 existing R3 DefinitionApply에 전달한다.
- Apply 직전 fresh re-review와 exact guarded Undo top/post-state 검사를 그대로 보존하며 auto Save/retry와 generic Undo를 추가하지 않았다.
- Official Build `f9a9ced62f1545f884ceefdfd2a966ab` PASS, 새 focused `BuilderStep7FinalReview` `a7efe7e368cd42f5b57a18c9b66f756f` 1/1 PASS. Step1~6는 replay하지 않았다.
- 다음 Gate는 Step 8 Technical Driving + USER Driving Guided connection이다. 전체 USER E2E와 P0-10 promotion은 Pending이다.

### v0.1.18 - 2026-08-27

- VB-P0-09 Step 6 Gameplay Setup Guided Flow를 Technical PASS로 닫았다.
- existing `ReadBuilderGameplayGuidance` R0의 8영역 completeness와 manual Socket guidance를 Guided Shell current Step에 투영했다.
- `NeedsReview`는 USER 수동 확인이 남은 Ready, `Blocked`는 Blocked, `bCanCompleteGameplayStep`은 Complete로 fresh derive한다. Pending Gameplay Diff는 Step 7 Apply owner를 유지한다.
- Official Build `d01056d228214ee8bbd06b154768e96d` PASS, backend `GameplayGuidance` 및 Guided VM Step6 integration/resume/stale prerequisite 회귀 PASS를 확인했다.
- 다음 구현 Gate는 Step 7 Final Review / explicit Apply Guided connection이다. USER E2E/P0-10 promotion은 Pending이다.

### v0.1.17 - 2026-08-27

- VB-P0-09 Step 5 Physics Proposal Guided Flow를 Technical PASS로 닫았다.
- `FCFBuilderPhysicsDraft` exact binding과 complete private 4 Profile proposal을 existing `PreviewBuilderProfiles`/`CommitBuilderProfiles` typed lane에 연결했다.
- current Recipe exact private Profile path를 mutation destination authority로 강제하고 accepted Evidence + consumed Claim + persistent receipt로 provenance/resume/Stale을 fresh derive한다.
- Guided Shell은 AI summary/Claim/domain별 변경 범위를 보여주며 Target Definition Apply는 Step 7, 자동 Save는 금지 상태를 유지한다.
- Final Official Build `3d65b8e8b6d04f498ceaf6358618c6a0` PASS, `BuilderStep5Physics` 포함 affected regression PASS. broad의 unrelated `AllowlistProjection` stale leaf-count 1건은 별도 유지한다.
- 다음 구현 Gate는 Step 6 Gameplay Setup Guided connection이다. 전체 USER E2E와 P0-10 Systems promotion은 Pending이다.
### v0.1.16 - 2026-08-27

- VB-P0-09 Step 1 Reference Evidence / Companion USER flow를 Technical PASS로 닫았다.
- ResearchDraft exact binding, validated initial Evidence payload, fixed EvidenceId/prospective fingerprint approval binding, Existing companion 보존/Missing-only R2 commit을 Guided Shell에 연결했다.
- USER local review token은 RecipeId/EvidenceId/EvidenceFingerprint만 소유하며 mutation approval과 분리했다. Evidence drift는 fresh VM에서도 Stale로 재계산된다.
- Final Official Build `06370173f2c64a1ab4dd3d235bd46c18` PASS, `BuilderStep1Reference` / `BuilderShell` / `BuilderCompanionFlow` PASS를 확인했다.
- broad DataAuthoring의 unrelated `DAUTH_P0_08.Batch.AllowlistProjection` stale leaf-count assertion 1건은 별도 regression으로 남겼다.
- 다음 구현 Gate는 같은 VB-P0-09의 Step 5 Physics Proposal review 연결이다. 전체 USER E2E와 P0-10 Systems promotion은 Pending이다.
### v0.1.15 - 2026-08-27

- VB-P0-09 Step Extensibility Hardening을 Technical PASS로 닫았다. current baseline 8 StepId의 semantic identity와 presentation 개수/순서를 분리했다.
- 단일 definition owner + Step별 evaluator + StepId lookup/write + dynamic navigation 구조로 fixed 숫자/index 의미 결합을 제거했다. final v1.3.1에서는 `EvaluateStepById()`가 definition 순서대로 evaluator를 dispatch해 표시 순서와 실행 순서 owner도 단일화했다.
- 새 Step 추가/순서 변경/제거·통합/기존 규칙 수정의 수정 범위를 국소화했으며 기존 enum StepId는 호환성을 위해 다른 의미로 재사용하지 않는다.
- generic plugin/Blueprint workflow/Step DataAsset/persistent progress graph는 만들지 않는 경계를 유지한다.
- Final Official Build `8e1c55defcbc49108c4c402964a33477` PASS, focused BuilderShell `0e2b3bd9260446c084e0523427283e02` 1/1 PASS. USER E2E Pending과 Systems P0-10 승격 경계는 불변이다.

### v0.1.14 - 2026-08-27

- VB-P0-09 USER Acceptance technical preparation으로 Guided Builder Step 2~4를 current `FCFVehicleAssetSnapshot`/Target readback authority에 실제 연결했다.
- Mesh는 Chassis+FL 최소 필수와 optional Wheel semantics, Socket은 4/4 found+distinct/custom name, Layout은 direct persisted-vs-current exact equality로 Design contract를 구현했다.
- Official Build `1b1aa29b80c341c6a6289ac2693413e9` PASS와 focused BuilderShell `c6ab98e244634dd1a7fbcb84b290d4e3` 1/1 PASS를 확보했다.
- USER Hardpoint authority / auto Socket / arbitrary Reference threshold / Save0 보호를 유지하며 VB-P0-09 전체 USER PASS는 Pending이다.
- next는 같은 VB-P0-09 안에서 Step 1 Reference/Companion과 Step 5~8 Guided action을 연결한 뒤 실제 신규 차량 E2E USER Acceptance를 수행한다.

### v0.1.13 - 2026-08-27

- VB-P0-08 post-PASS measurement hardening으로 Heavy PhysicsState reset과 Light kinematic reset을 분리하고 settle 뒤 Light reset만 수행해 계측 시작 상태를 결정론적으로 고정했다.
- 100→Idle braking은 동일 top-speed trajectory에서 100km/h entry를 재확보한 뒤 Brake 1.0을 적용하는 경로로 교정했다.
- latest Official Build `0a8f33824563433ab32097a612f434d9` PASS. fitted B/C는 0→50, 0→100, Peak, braking time/distance, yaw, turning radius가 exact 동일했고 latest no-fitting BaseMass canary도 PASS했다.
- P0-08 Technical PASS와 next `VB-P0-09 End-to-End USER Acceptance`는 유지한다.

### v0.1.12 - 2026-08-27

- `VB-P0-08 Technical Driving Benchmark`를 Harness Technical PASS로 닫았다.
- RuntimeRead가 직접 Chaos getter/function 실행을 제공하지 않는 현재 capability 경계를 지키고 Product debug getter/HUD를 추가하지 않은 채 기존 Mobility Measurement 패턴을 arbitrary saved VehicleData용 Automation으로 일반화했다.
- optional Fitting total mass / no-fitting BaseVehicleMassKg, 0→50/100, peak speed, RPM/gear, 100→Idle braking, steady yaw, effective turning radius를 fresh PIE 실제 Chaos에서 계측한다.
- phase PhysicsState rebuild, explicit first gear, map fixture isolation, active-phase peak telemetry와 UE 5.8 Source-confirmed `-UseFixedTimeStep -FPS=60`으로 반복 편차를 제거했다.
- fitted repeat A/B exact 동일 결과와 no-fitting BaseMass canary PASS를 확보했다. 대표 canary의 100km/h 미도달 때문에 braking branch 미관측은 명시적으로 남긴다.
- latest Official Build `02090e05471a4af4afd7a0d98d01542a` PASS, production Asset/Socket/Save mutation 0. 다음 Gate는 `VB-P0-09 End-to-End USER Acceptance`다.

### v0.1.11 - 2026-08-27

- VB-P0-05~07 post-review hardening을 완료했다. P0-05 direct write-lane Automation을 추가하고, 이를 통해 transient prospective Profile UObject path가 fresh approval hash를 흔드는 실제 비결정성 결함을 발견·교정했다.
- Builder private Profile commit과 Missing Companion preview 모두 prospective persistent Profile path + typed snapshot으로 Pure Resolver를 실행해 Preview→Commit fresh replay의 ProposalHash 안정성을 보장한다.
- Recipe의 non-semantic `BuilderCommitReceipt`가 accepted Evidence fingerprint / consumed Claim set hash / private 4 Profile fingerprints / Resolver revision을 보존하며 Final Review provenance authority가 caller 입력만 의존하지 않게 했다.
- Final Review는 one fresh Resolve를 재사용하고, guarded Undo는 current post-Apply Target/semantic Recipe/AppliedState가 exact할 때만 수행해 transaction 밖 raw drift까지 fail-closed한다.
- latest Official Build `306b9c9be7f648a4b5a89b6eb13c7f05` PASS, VB-P0-05 4/4 + VB-P0-06 1/1 + VB-P0-07 1/1 sequential focused PASS를 확보했다. next gate는 `VB-P0-08 Technical Driving Benchmark` 그대로다.

### v0.1.10 - 2026-08-27

- `VB-P0-07 Final Review / Validation / Undo`를 Technical PASS로 닫았다.
- existing Resolve/Validation/External Drift/P0-06 Gameplay guidance/Diff를 R0 `ReadBuilderFinalReview()`에서 aggregate하고 Target Diff와 exact DefinitionApply readiness를 한 결과로 제공한다.
- current persistent schema에 field→Claim provenance map이 없으므로 FACT/DERIVED/GAME_BIAS를 field count로 추정하지 않고 accepted Evidence의 consumed canonical Claim count만 정확하게 집계한다.
- `ApplyBuilderFinalReview()`는 fresh Final Review와 exact `DefinitionApply` scope를 재검사한 뒤 기존 `ApplyResolvedVehicle()` → `FCFVehicleApplyService` 단일 writer lane만 사용한다.
- `UndoBuilderFinalApply()`는 Builder가 current Editor lifetime에서 발급한 exact UE TransactionId와 UndoScopeHash를 요구하고 current stack top이 다르면 `StateChanged`로 fail-closed한다. Undo 뒤 Target full hash와 Recipe AppliedState를 pre-Apply evidence와 비교한다.
- Official Build `ff4dd50981464710876a0693d2287432` PASS와 focused `CarFight.DataAuthoring.CF_FQ_040.VB_P0_07.FinalReviewApplyUndo` 1/1 PASS를 확보했다. Hardpoint USER authority / auto Socket 금지 / Save0를 유지한다.
- next gate를 `VB-P0-08 Technical Driving Benchmark`로 전진했다.

### v0.1.9 - 2026-08-27

- `VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`를 Technical PASS로 닫았다.
- R0 `ReadBuilderGameplayGuidance()`로 Durability/Defense/DestroyedFx/Hardpoint/Mount/DriveState/WheelVisual/FittingMass 8영역의 Complete/Optional/NeedsReview/Blocked projection을 구현했다.
- Hardpoint Socket 실제 위치는 USER authority로 유지하고 자동 생성·이동·배치를 금지했다. Existing Vehicle Completion은 current stored LocalTransform과 Resolver diff를 비교해 안전한 기존 배치를 강제 Socket migration 없이 보존한다.
- 선택적인 Defense/DestroyedFx/Fitting 미사용 상태를 정상 Optional로 처리하고 임의 Asset·수치 생성을 하지 않는다. Existing Hardpoint/Mount candidate mismatch는 NeedsReview로 fail-safe 처리한다.
- final Official Build `12e537e016b64a179a813a4929f584e8` PASS, focused `CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance` 1/1 PASS를 확보했다.
- next gate를 `VB-P0-07 Final Review / Validation / Undo`로 전진했다.

### v0.1.8 - 2026-08-26

- `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow`를 Technical PASS로 닫았다.
- atomic TransmissionRatios / Registry127 / Resolver rev2, Reference Evidence SHA-256+NFC, Evidence-bound private 4 Profile atomic commit, existing ApplyService writer lane과 lifecycle-safe Chaos Transmission 적용을 구현했다.
- `NewVehicle`과 `CompleteExisting` 두 Builder entry를 같은 companion completeness flow에 통합했다. Existing valid companion은 보존하고 Missing만 생성하며 shared/foreign owner는 Conflict, 기존 차량은 current/prospective ResolvedDefinitionHash equality로 baseline을 보호한다.
- Official BuildEditor PASS, focused Automation 2/2 PASS, persisted CFVehicleData legacy Transmission audit 11/11 PASS, PhysicsState lifecycle Source audit PASS를 확보했다.
- production Asset create/commit/apply/save는 검증에서 수행하지 않았고 next gate를 `VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`로 전진했다.

### v0.1.7 - 2026-08-26

- VB-P0-04 final source closure에서 local UE 5.8 Source Build의 `FVehicleTransmissionConfig::InitDefaults/FillTransmissionSetup/GetGearRatio`와 `FSimpleTransmissionSim::GetGearRatio`를 exact read-only 확인했다.
- Reverse ratio setup array는 positive magnitude를 저장하고 실제 reverse sign은 `GetGearRatio()`에서 적용한다는 convention을 확정했다.
- `VehicleBuilderProposalSpec.md v0.1.1`로 잘못된 negative-array 선확정을 교정하고 VB-P0-04를 Source uncertainty 0 / TRUE PASS로 닫았다.
- Implementation 0과 `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow` next gate는 유지한다.

### v0.1.6 - 2026-08-26

- `VB-P0-04 Reference → Authoring Proposal Bridge`를 Design PASS로 닫고 상세 owner `VehicleBuilderProposalSpec.md v0.1.0`을 추가했다.
- accepted Evidence에서 Builder-private 4 Profile의 complete prospective payload를 만들고 EvidenceDirect/EvidenceDerived/BaselineInherited/GameBias/PlannedTypedGap/UnsupportedExact provenance를 field 단위로 고정했다.
- Feel physical anchor=Neutral, sparse Profile 금지, existing Profile/current effective seed 기반 complete payload와 Unknown 보존 계약을 확정했다.
- UE 5.8 `FVehicleTransmissionConfig` / `FSimpleTransmissionConfig` / `ETransmissionType` exact mapping과 Manual/Automatic control mode, ratio/final/shift/time/efficiency semantics를 확정했다.
- Transmission architecture는 Chaos enum과 분리하고 CVT exact mapping은 UnsupportedExact로, forward/reverse gear count는 ratio array length derive로 고정했다.
- dedicated atomic Transmission ratio-set과 current Transmission/ChassisWidth/wheel-geometry typed schema gap을 VB-P0-05 implementation obligation으로 전진했다.
- Kia Morning 4AT mapping dry-run에서 known mass/FWD/automatic/torque만 proposal하고 ratios/final/GVWR/redline Unknown을 유지했다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation 0, Implementation 0을 유지하고 next gate를 VB-P0-05로 전진했다.

### v0.1.5 - 2026-08-26

- `VB-P0-03 Mesh & Socket Guidance`를 Design PASS로 닫고 next gate를 `VB-P0-04 Reference → Authoring Proposal Bridge`로 전진했다.
- current `FCFVehicleAssetReader / FCFVehicleAssetSnapshot`을 Builder의 Mesh/Socket read authority로 재사용하고 별도 Asset scanner를 만들지 않도록 고정했다.
- MeshPrep hard blocker, Wheel role 4/4 existence + distinct binding, custom socket 허용, Hardpoint/Destroyed FX warning/requiredness boundary를 확정했다.
- Wheel socket transform에서 Wheelbase/Front·Rear Track을 read-only 계산해 Reference delta를 표시하되 임의 threshold로 Block하지 않도록 했다.
- LayoutCapture를 실행하지 않고 current persisted `VehicleLayoutConfig`와 current socket truth의 fresh equality로 Complete/Stale를 derive하도록 했다.
- narrow dependency stamp로 unrelated Engine/Transmission/Defense change가 P0-03을 불필요하게 Stale 처리하지 않도록 했다.
- C++/Asset/schema/capture/save/build mutation 0, Implementation 0을 유지했다.
- Transmission numeric schema/Chaos mapping은 VB-P0-04 owner로 보존했다.

### v0.1.4 - 2026-08-26

- 실제 다단 변속기 Authoring을 Guided Vehicle Builder의 Drivetrain Domain 정식 범위로 통합하고 별도 Feature 분리를 금지했다.
- Builder-private `UCFDrivetrainProfile`을 Differential뿐 아니라 차량별 Transmission numeric authoring owner로 명확히 했다.
- VB-P0-04에 Reference Evidence → AI Drivetrain Proposal → private Drivetrain Profile → UE 5.8 Chaos `TransmissionSetup` exact mapping obligation을 추가했다.
- type/automatic, forward count/ratios, reverse ratio, final drive, up/downshift 조건, change time, efficiency를 최소 검토 concept로 고정했지만 C++ schema/API 대응은 아직 확정하지 않았다.
- Engine/RPM owner=Performance, Transmission owner=Drivetrain 경계를 유지하고 HUD Gear는 `GetCurrentGear()` runtime authority를 계속 사용하도록 했다.
- 현재 next gate는 `VB-P0-03 Mesh & Socket Guidance` 그대로이며 C++/Asset/Runtime mutation은 수행하지 않았다.

### v0.1.3 - 2026-08-26

- `VB-P0-02 Builder Shell / Step State / Resume`를 Design / Shell-State-Resume Contract PASS로 닫았다.
- 상세 owner `VehicleBuilderShellSpec.md v0.1.0`에 persistent truth / local resume convenience / transient Builder state의 3층 ownership을 고정했다.
- managed resume primary identity를 `RecipeId`로 고정하고 path는 fallback/diagnostic hint로 제한했다.
- fixed 8 Step, 6-state `Unavailable/Locked/Ready/Complete/Blocked/Stale`, fresh Next evaluation, dependency-specific stale와 read-only context 계약을 고정했다.
- Reference review token과 mutation approval을 분리하고 restart 뒤 prepared approval/cache/Undo identity를 모두 폐기하도록 했다.
- C++/UE Asset/schema mutation 없이 Implementation 0을 유지하며 next gate를 `VB-P0-03 Mesh & Socket Guidance`로 전진했다. `VB-P0-04` numeric mapping은 미착수다.

### v0.1.2 - 2026-08-26

- `VB-P0-01 Reference Research & Evidence Contract`를 Design / Research Normalization PASS로 닫았다.
- `VehicleRefEvidenceSpec.md v0.1.0`을 상세 owner로 추가하고 identity/citation/provenance/Unknown/conflict/confidence/fingerprint/proposal-binding 계약을 고정했다.
- 2027 Kia Morning 1.0 gasoline Trendy 14-inch를 실제 dry-run으로 사용해 14/16-inch variant split, Unknown 보존, FACT unit normalization과 DERIVED 계산 경계를 검증했다.
- Evidence stale binding을 기존 Recipe/Target/Profile fingerprint와 ProposalHash approval 체계에 additive하게 결합했다.
- C++/UE Asset mutation 없이 Implementation 0을 유지하고 next gate를 `VB-P0-02 Builder Shell / Step State / Resume`로 전진했다.

### v0.1.1 - 2026-08-26

- `VB-P0-00 Current Vehicle Creation Contract Audit`을 read-only로 PASS했다.
- Reference Evidence owner를 별도 Editor-only companion DataAsset으로 확정했다.
- AI vehicle-specific numeric owner를 Builder-private VehicleBase/Drivetrain/Handling/Performance Profile 4종으로 확정하고 DriveState는 ProjectDefault 기본값을 유지했다.
- 최소 Builder 생성 Asset을 VehicleData + Recipe + Reference Evidence + private Profile 4종 = 7개로 확정했다. Fitting/Defense/Fx/Equipment/Pawn BP는 minimum set에서 제외했다.
- current Definition+Recipe two-record creation, Validator/Resolver/ApplyService와 optional Fitting fallback을 보존한다.
- 기존 GoPyMCP `ue.call_write` transport를 재사용해 새 top-level public MCP tool을 추가하지 않는 경계를 확정했다.
- 다음 Gate를 `VB-P0-01 Reference Research & Evidence Contract`로 전진했다.

### v0.1.0 - 2026-08-26

- `CF-FQ-040 Guided Vehicle Builder` 최초 Roadmap을 생성했다.
- 실차 Reference 조사 → USER Mesh/Socket 준비 → 기존 Capture → AI Derived Physics Proposal → Data Authoring Review/Apply → Technical Driving Benchmark → USER Driving의 P0 순서를 고정했다.
- Wheel 자동검출, Hardpoint 자동배치, UE 내부 LLM/Web crawler를 명시적 Scope Out했다.
- 기존 CF-FQ-038 Backend와 CF-FQ-034 Fitting을 재사용하며 current CF-FQ-039 Active는 변경하지 않는다.
- 구현 시작 전 `VB-P0-00 Current Vehicle Creation Contract Audit`을 강제 Gate로 설정했다.
