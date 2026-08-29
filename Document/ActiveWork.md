# CarFight Active Work

- 문서 버전: v4.44
- 최근 갱신일: 2026-08-29
- 문서 상태: Current
- 역할: CarFight 게임 프로젝트에서 현재 실제로 진행 중인 작업을 선택하고 대표 Plan으로 연결하는 **세션 복원 projection**

---

## 1. 운영 원칙

```text
FeatureQueue = 기능 우선순위와 상태
ActiveWork = 현재 Active/Paused/Ready의 복원 포인터
대표 Plan = 상세 체크포인트와 validation/evidence owner
Systems = 완료 기능의 현재 구현 owner
Archive/Git = Historical 기록
```

이 문서에는 대표 Plan의 전체 Build ID, Automation run history, USER PIE 세부 로그, 단계별 완료 목록과 과거 Changelog를 복제하지 않는다.

특정 Feature ID나 작업명이 사용자 요청에 있으면 그 작업을 우선 복원한다. 단순히 마지막에 작성된 항목을 자동 Active로 간주하지 않는다.

---

## 2. 현재 단일 Active

### CF-FQ-039 Production UI Visual Rework — Active

```text
대표 Plan: Document/Plan/InGameUIVisual/InGameUIVisualPlan.md v0.1.33
Roadmap: Document/Plan/InGameUIVisual/InGameUIVisualRoadmap.md v0.1.27
Current Gate: VPR-P0-01 VehiclePanel Actual-Asset Prototype — VT12 SPEED/RPM + ARMOR COMPOSITION READY / USER COMPOSITION REVIEW PENDING / UE IMPORT 0 / PRODUCTION ASSET MUTATION 0
Supporting Art Spec: Document/Plan/InGameUIVisual/InGameUIHUDArtSpec.md v0.4.18
Current System 기준: Document/Systems/UI/InGameUI.md v1.1.10
완료 기반: CF-FQ-032 Done / Document/Plan/Archive/InGameUIPlan.md v0.59.36
```

VPR-P0-00 Recovery와 Vehicle editable Structure Readiness는 완료됐다. `VT-VEH-01` canonical source는 `SourceArt/UI/HUD/VT/VT_VehPanel01.jpg` 1280×594 / SHA `cf5b0a57...df6b6b`이다. v4는 내부 safe-area box/central seam/lower deck rectangle가 임시 layout geometry인데 Frame Source에 남아 USER가 명시 Reject했다. 이에 Frame Master ownership을 outer mechanical shell/corner/top rail/lower outer base로 축소하고 내부 임시 geometry 0인 `SourceArt/UI/HUD/P2/VT06_FrameMaster_v5.png` + `Review/VT06_FrameReview_v5.png`를 생성했다. v5는 직접 Image QA에서 **AI Source QA PASS / USER Visual PASS Pending**이다. safe-area는 문서/Designer layout contract일 뿐 Production Frame pixel로 남기지 않는다. USER PASS 전에는 UE Import, `Image_FrameOverlay` assembly, subordinate local frame 조립, RPM/Speed/Gear/Silhouette/Armor 위치 최종화를 수행하지 않는다. 기존 RPM runtime 구조, Armor modular/common Plate+2-icon, vehicle-specific silhouette Production, Defense Source/Production Assetization PASS는 반복하지 않는다.

`CF-FQ-032`의 Runtime·Provider·Presenter·ViewData·Designer ownership·완료 evidence는 보호하며 이 Visual Rework 착수만을 이유로 반복하지 않는다.

UI Resource 방법론 실험 `URT05_Plan.md v0.8`은 Method Validation Complete로 보존한다. VT07/VT08의 임의 재설계 방향은 successor 기반으로 사용하지 않는다. VT09 reference structure → VT10 target-pixel extraction → VT11 production cleanup을 거쳐, 현재는 실제 runtime/source family만 조립한 VT12 VehiclePanel Prototype으로 전진했다. Outer Frame 정밀도보다 Speed/RPM 계기판 hierarchy와 Armor 6-sector composition을 우선한다. `VT12_ReferenceCompare`로 canonical VT-VEH-01과 1:1 비교하며 현재 USER composition review pending, UE Import/Designer/Production Asset mutation 0이다.

---

## 3. 최근 완료

### CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크 — Done

```text
상태: Done
Historical Plan: Document/Plan/Archive/InGameUIPlan.md v0.59.36
Current owner: Document/Systems/UI/InGameUI.md v1.1.5
Aim/FireFeedback owner: Document/Systems/UI/AimReticle.md v1.10.0
Sensor/Target Identity owner: Document/Systems/Targeting/SensorContact.md v1.2.0
```

보존 판정:

- UI-P0-02~05 USER PASS.
- UI-P0-06~10 Technical Complete / AI Runtime Technical Validation.
- UI-P0-11 Systems Promotion Complete.
- 2026-08-22 post-closure remediation P1 2건 + P2 3건 Technical PASS.
- final Official Build `bd3640c616794cd7a54cc8a8afa4b021`, broad `CarFight.UI` 44/44, `CarFight.Sensor` 14/14 PASS는 대표 Plan evidence로 보존하며 새 관련 failure 없이 반복하지 않는다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART SpeedGauge·VehiclePanel·전체 Visual Review는 비차단 Deferred/Pending이며 USER PASS로 확대하지 않는다.

---

## 4. Paused / Ready 복원 체크포인트

| Feature | 상태 | 대표 owner / 재개 지점 | 반복 금지 범위 |
| --- | --- | --- | --- |
| `CF-FQ-038` 차량 데이터 Authoring | Paused | `Document/Plan/DataAuthoring/DataAuthoringPlan.md v0.2.52` + `Document/Plan/DataAuthoring/DataAuthoringRoadmap.md v0.1.56` → Deprecated transition Technical Complete / CF-FQ-040 Builder Backend·Advanced Workspace 역할 / 다음 non-blocking `DEL6 compatibility retirement` 또는 `UA-08 quantitative comparison Deferred` | P0-12 USER PASS 7/8 유지. UA-01~08·P0-08~11·DG1~DG5·deprecation validation replay 금지. DEL6 Pending이라 physical Wizard deletion 금지 |
| `CF-FQ-034` 차량 피팅·질량 | Paused | `Document/Plan/VehicleFitting/VehicleFittingPlan.md v0.17.0` → `FIT-P0-07D USER Driving Feel Comparison` | FIT-P0-07A~07C 정량 Mobility evidence 반복 금지 |
| `CF-FQ-029` 모듈형 런처 | Paused | `Document/Plan/LauncherMissile/LauncherMissilePlan.md v0.14.0` → `LM-P0-06 USER PIE` | LM-P0-06A Failure Policy Technical PASS 반복 금지 |
| `CF-FQ-030` 미사일 비행·유도 | Ready | `CF-TC-027 Manual PIE Pending` | Persisted Asset Technical Verification과 Direct Runtime 기술 증거 반복 금지 |
| `CF-FQ-040` Guided Vehicle Builder | Ready | `Document/Plan/VehicleBuilder/VehicleBuilderPlan.md v0.1.25` + `Document/Plan/VehicleBuilder/VehicleBuilderRoadmap.md v0.1.25` + `Document/Plan/VehicleBuilder/WheelSizeAuthorityPlan.md v0.1.11` + `Document/Plan/VehicleBuilder/VehicleBuilderProposalSpec.md v0.1.1` + `Document/Plan/VehicleBuilder/VehicleBuilderShellSpec.md v0.1.14` + `Document/Plan/VehicleBuilder/VehicleRefEvidenceSpec.md v0.1.1` → `VB-P0-09 In Progress / actual Wagon AI-only preparation complete / WSA-P0-06 Technical PASS / WSA-P0-07 USER Acceptance Ready / Step 1~8 Technical PASS preserved` | Actual Wagon Recipe는 shared `Wheel_FL` + `SocketScaleFromChassis` + Socket-derived Radius/Width adoption 4건 persisted. AI ResearchDraft는 2026 Volvo V60 Cross Country KR representative reference, private Profile 4종 initial seed, actual Front/Rear WheelClass와 WSA-safe wheel authority로 준비 완료했고 schema/consistency PASS다. UE Asset/Recipe/Target/Save mutation은 추가 수행하지 않았다. next는 출근 후 Step 1 Reference Set USER review → Companion explicit 생성 → Step 5 provenance/Profile proposal → Step 7 full Diff USER review/Apply → USER Save → Technical Driving + WSA-P0-07 PIE USER Acceptance. USER Scale 없는 legacy 자동 migration/AI tire-size decision/raw writer 금지 |
| `CF-FQ-035` 인벤토리 Foundation | Paused | FeatureQueue/대표 Plan → USER Field UI·Mobility | 기존 Inventory/Fitting Technical checkpoint 반복 금지 |
| `CF-FQ-026` 타겟 선택 | Paused | FeatureQueue/대표 Plan → `TS-P0-08 USER PIE` | TS-P0-00~07 및 Remote Technical evidence 반복 금지 |
| `CF-FQ-015` 차량 데이터 튜닝 | Paused | FeatureQueue/대표 Plan → `VD-P0-04 USER Tuning` | VD-P0-00~03 Technical evidence 반복 금지 |

---

## 5. Candidate / Deferred

```text
Candidate: CF-FQ-012 1대 차량 주행감 고도화
Candidate: CF-FQ-014 WheelSync 시각 품질 폴리싱
Candidate: CF-FQ-020 조작감/전투 템포/피드백 개선
Candidate: CF-FQ-021 핵심 게임 루프 검증
Deferred:  CF-FQ-019 주행/전투 반복 테스트 — 런처·미사일 이후 통합 회귀 범위 재설계
```

Candidate는 사용자 선택 전 자동 Active로 승격하지 않는다.

---

## 6. 세션 복원 절차

Feature 재개 시 다음 순서만 사용한다.

```text
1. ActiveWork에서 Feature 상태와 대표 owner 확인
2. FeatureQueue에서 현재 상태/우선순위 교차검증
3. 대표 Plan의 최신 checkpoint와 evidence 확인
4. 관련 Systems와 실제 Source/Asset로 현재 구현 확인
5. 완료 evidence는 새 failure가 없으면 반복하지 않고 next gate부터 재개
```

`ActiveWork`와 대표 Plan이 충돌하면 실제 Git/Source/Asset, Systems, FeatureQueue와 대표 Plan을 다시 확인하고 stale projection을 교정한다.

---

## 7. 문서 유지 규칙

Feature 작업 중에는 현재 next gate를 갱신할 수 있다. Feature가 Done되면 상세 진행 로그는 대표 Plan에 남기고 ActiveWork에서는 제거한다.

ActiveWork에 남겨도 되는 완료 정보는 **최근 완료 Feature 1건의 핵심 owner/보호 판정** 정도다. 완료 Feature가 새로 생기면 이전 최근 완료 블록은 Systems/Plan 링크로 대체하거나 제거한다.

ActiveWork가 다시 상세 Build/Automation/USER 로그를 누적하거나 서로 다른 세대의 Current 상태를 포함하면 문서 Health Check 대상으로 본다.

---

## 8. Changelog

### v4.44 - 2026-08-29

- Document/Plan 물리 구조 정리에 따라 Active/Paused/Ready 대표 Plan 포인터를 기능 폴더 경로로 교정했다.
- CF-FQ-039/034/038/040의 Feature 상태·Gate·Acceptance 의미는 변경하지 않았고 경로 migration만 수행했다.
- 완료·대체된 구형 Plan은 Archive로 이동하고 Generated Intermediate 잔여물은 Trash 격리했다.

### v4.43 - 2026-08-29

- CF-FQ-040 actual Wagon의 USER 부재 AI-only 준비를 완료했다. `ResearchDraft.json`에 2026 Volvo V60 Cross Country KR representative Reference Evidence와 private VehicleBase/Drivetrain/Handling/Performance initial seed를 준비하고 exact Wagon Recipe/Target binding을 유지했다.
- Volvo 2026 치수표의 explicit body width 1850mm를 Chaos `ChassisWidth=185.0cm` 기준으로 사용하고, 전체 폭 1893/1895mm와 분리했다. exact peak torque/gear ratios는 미공개라 Unknown/ProposalWarning으로 보존했고 `bUseTransmissionConfig=false`로 임의 기어비 생성을 막았다.
- WSA authority는 `bUseReferenceWheelGeometry=false` + current shared Wheel bounds/User Socket Scale 유지다. Front/Rear WheelClass actual generated class를 확정했으며 Draft JSON/schema/reference/citation/enum/feel-response consistency 검사 PASS다. persistent UE Asset/Profile/Recipe/Target/Save mutation은 0이며 next는 Step 1 USER Reference review다.

### v4.42 - 2026-08-28

- CF-FQ-040 WSA-P0-06 Technical Validation을 post-review 교정까지 포함해 최종 유지했다. `WheelSizeAuthorityPlan.md v0.1.11 / WSA-P0-07 USER Acceptance Ready`로 복원 포인터를 전진했다.
- deferred Apply guard를 exact core 4 Profile + Front/Rear WheelClass 누락 signature로 강화하고 negative `DeferredApplyGuard` regression을 추가했다. current final Build `e2cfa435b5764dc282ec2b2e973175b6` Exit0, focused process `d63625118ad94599a56e4151bb9b6dc6` exact 10/10 PASS다.
- 검증 중 DataAuthoring test runner의 `Start-Process -Wait` descendant-process hang을 기존 accepted exact-process `WaitForExit()` 패턴으로 교정했다. Wagon migration/Asset Save는 재실행하지 않았다.

### v4.41 - 2026-08-28

- CF-FQ-040 WSA-P0-05 Legacy Migration을 Technical PASS로 닫고 `WheelSizeAuthorityPlan.md v0.1.9 / WSA-P0-06 Ready`로 전진했다.
- actual Wagon의 USER Socket Scale `(0.8,1.0,0.8)`을 Category A evidence로 확정하고 Recipe에 shared Wheel FL, `SocketScaleFromChassis`, Radius/Width adoption 4건을 persisted 저장했다. Profile 4종/WheelClass 2종은 후속 Builder Step 소유라 WSA가 임의 생성하지 않았고 Target Apply는 Step 7로 defer했다.
- 공용 Resolver SoftObject→hard Object canonical hash mismatch를 교정하고 Official Build `05115a84e3c34c4c80efe1695f998174` Exit0, focused process `798f21a197254d82932a5ab0b8f31868` exact 9/9 PASS, Wagon migration `c66f7579d3114ba3afb29502f7f5dd6c` Exit0을 확인했다.
- persisted AssetDump에서 Recipe migration 저장과 `DA_Vehicle_Wagon` 미적용 baseline을 각각 확인했다. legacy/test/SUV 자동 migration은 0이다.

### v4.40 - 2026-08-28

- CF-FQ-040 WSA-P0-04 Validator / Builder Integration을 Technical PASS로 닫고 WheelSizeAuthorityPlan v0.1.8 / WSA-P0-05 Legacy Migration으로 전진했다.
- Official Build `6215416c66304a048ba83cbe1d5a8ca2` Exit0, focused process `30add52f8ba94f4b8f206356e7c33faa`에서 Registry/Utility/Resolver/Runtime/BuilderShell/Gameplay/ProfileCommit/Validator exact 8 tests 모두 PASS했다.
- 검증 중 발견된 Resolver FL-only shared Wheel fallback 누락과 blocked-preview에서 Step3 실제 원인이 가려지는 Builder diagnostic gap을 교정하고 회귀검증했다.
- P0-05는 먼저 current AutoScale 자산과 Wagon USER Socket Scale readiness를 read-only 감사하며, USER 작성 Scale이 없는 차량을 임의 migration하지 않는다. unrelated dirty와 CF-FQ-039 Active는 보존한다.

### v4.39 - 2026-08-28

- CF-FQ-040 WSA-P0-03 Runtime Visual / Mesh Fallback을 Technical PASS로 닫고 WheelSizeAuthorityPlan v0.1.8로 전진했다.
- USER fresh Build PASS와 focused process `304cd2dbfaed46cda7a1db7c0da9feb9` Exit0에서 WSA exact 4 tests PASS를 확인했다.
- runtime shared FL fallback, Wheel_Mesh exact Socket Scale, Wheel_Anchor scale 비소유, WheelSync steering/suspension/spin 후 scale 보존을 actual transient Pawn으로 검증했다.
- next gate는 WSA-P0-04 Validator / Builder Integration이며 기존 Step1~8 Technical PASS와 Wagon UX Pending, unrelated dirty는 보존한다.

### v4.38 - 2026-08-28

- CF-FQ-040 WSA-P0-02 Socket Resolver & Derived Physics를 Technical PASS로 닫고 WheelSizeAuthorityPlan v0.1.7로 전진했다.
- StaticMeshSocket RelativeScale capture, R5 Scale AssetDerived, R3 Socket mode flag, R6 Bounds+Scale Radius/Width, narrow size fingerprint, axle/X-Z fail-closed와 ResolverRevision4를 적용했다.
- Official Build `1ebcc7850e3e4c83adbae3de3590d0ec` Exit0, focused process `7be4965e5c804f79ae9b87355701d860`에서 WSA exact 3 tests 모두 PASS다.
- next gate는 WSA-P0-03 Runtime Visual / Mesh Fallback이다. Wheel_Anchor scale 적용 금지와 USER visual authority를 유지하며 unrelated dirty/Wagon UX Pending은 보존한다.

### v4.37 - 2026-08-28

- CF-FQ-040 WSA-P0-01을 Technical PASS로 닫았다. fresh official Build `c74519c538404520b31896c77cfceba4` Exit0, Runtime/Editor DLL Link PASS다.
- focused headless runner `Tools/RunWheelSizeTests.ps1`로 Registry132 1/1 + SchemaUtilityLegacy 1/1 PASS를 확인했다. legacy DA_TestSUV actual load에서 RelativeScale OneVector / SocketScale mode false / 기존 false path 보존을 검증했다.
- interactive editor.start UE MCP readiness 실패는 재시도 중단하고 WSA 기능 correctness와 분리했다. broad Step1~8/DataAuthoring replay는 하지 않았다.
- next gate는 WSA-P0-02 Socket Resolver & Derived Physics다. Wagon UX remediation의 USER UX recheck Pending과 unrelated dirty는 그대로 보존한다.

### v4.36 - 2026-08-28

- CF-FQ-040 WSA-P0-01 schema/helper 구현을 적용하고 WheelSizeAuthorityPlan v0.1.5로 checkpoint를 전진했다. RelativeScale/bUseWheelSocketScale/append-only mode/Registry132/CFWheelSizeUtils/ResolverContractRevision3가 적용됐다.
- Official Build `6918bebc22074c33842a3f8701c0a851`은 UHT와 WSA 포함 source compile은 PASS했으나 기존 UnrealEditor가 Runtime/Editor DLL을 점유해 LNK1104 Exit6로 Link만 실패했다. source compile error로 분류하지 않는다.
- 기존 Project Runtime은 reuse 상태이고 다른 USER/session의 unsaved work 0을 증명하지 못했으므로 자동 종료하지 않았다. 안전 종료 뒤 fresh Build + WSA focused Automation만 남으며 broad Step1~8 replay는 금지한다.
- 동시 진행된 Wagon UX remediation의 Plan/Roadmap v0.1.24, ShellSpec v0.1.13과 USER UX recheck Pending 상태는 그대로 보존했다.

### v4.35 - 2026-08-28

- CF-FQ-040 actual Wagon E2E feedback에서 작업 대상 가독성과 Step 2 WheelMesh 지정/Step 3 Socket 작업 UX 공백을 교정해 Technical PASS로 닫았다. 대표 owner는 Plan/Roadmap v0.1.24, ShellSpec v0.1.13이다.
- USER actual UI recheck와 Wagon Recipe WheelMesh explicit 반영은 Pending이다. WSA-P0-01 official next implementation gate, CF-FQ-040 Ready, 단일 Active CF-FQ-039와 unrelated dirty는 변경하지 않았다.

### v4.34 - 2026-08-28

- CF-FQ-040 Wheel Size Authority final Source audit를 PASS로 닫고 owner를 Plan/Roadmap v0.1.23, WheelSizeAuthorityPlan v0.1.4, ShellSpec v0.1.12로 동기화했다.
- 신규 audit correction은 bUseWheelSocketScale의 ProjectDefault+Recipe-only source, Socket mode Guide의 Legacy clamp 제외, Step5 AI Reference wheel geometry baseline-preserve다.
- canonical Wheel PASS와 기존 Step1~8 Technical PASS는 반복하지 않는다. 단일 Active CF-FQ-039와 unrelated dirty 변경은 보존했다.

### v4.33 - 2026-08-28

- CF-FQ-040 Wheel Size Authority를 v0.1.3 기준으로 재감사했다. actual shared Wheel_FL live Bounds `99.9990×24.9992×99.9990cm`, center≈0으로 canonical asset sub-gate PASS다.
- 대표 Plan/Roadmap을 v0.1.22, ShellSpec v0.1.11로 동기화하고 next를 WSA-P0-01 code/schema/helper implementation으로 좁혔다.
- narrow WheelSizeSourceFingerprint, Step4 Scale equality, enum append-only, Legacy AutoScale/AutoCenter/Clamp 분리를 설계에 반영했다. 기존 Step1~8 Technical PASS는 반복하지 않는다.
- 단일 Active CF-FQ-039와 unrelated dirty 작업은 변경하지 않았다.

### v4.32 - 2026-08-28

- CF-FQ-040에 정식 하위 `WheelSizeAuthorityPlan.md v0.1.1`을 등록하고 대표 Plan/Roadmap을 v0.1.21, ShellSpec을 v0.1.10으로 동기화했다.
- WSA-P0-00 Design PASS / next WSA-P0-01로 복원 지점을 변경했다. USER Wheel Socket Scale이 타이어 크기 Authority이며 StaticMesh Bounds는 원본 치수 Source만 담당한다.
- current shared Wheel_FL X/Z≈79.358cm라 Wagon 실제 Socket Scale 작성 전에 canonical X/Z=100cm normalization이 필요하다. 기존 Step1~8 Technical PASS는 반복하지 않는다.
- 단일 Active CF-FQ-039와 다른 Ready/Paused 작업은 변경하지 않았다. 이번 동기화에서 C++/UE Asset/Runtime mutation은 0이다.

### v4.31 - 2026-08-28

- CF-FQ-039의 우선순위를 Outer Frame 미세조정보다 Speed/RPM + Armor composition fidelity로 명시 교정했다.
- existing Production Source/Runtime family만 사용한 `VT12_VehPanelPrototype`과 canonical reference side-by-side `VT12_ReferenceCompare`를 생성 PASS하고 OneDrive Review에 exact copy했다.
- 현재 Gate는 USER composition review이며 UE Import/Designer/Production Asset mutation 0이다. 다른 Active/Ready 작업은 변경하지 않았다.

### v4.30 - 2026-08-28

- CF-FQ-039 VT10 extraction을 그대로 쓰지 않고 VT11 Production Cleanup으로 전진했다.
- Frame/Speed/Armor 3종의 geometry와 alpha를 exact 보존하면서 사진성 noise/irregular shading을 제한 palette로 정리했고 alpha mismatch 0을 검증했다.
- `VT11_CleanupReview` comparison QA를 OneDrive Review에 exact copy했다. USER Cleanup Review 전 UE Import/Designer/Production Asset mutation은 0이다.
- CF-FQ-040 Ready projection과 unrelated dirty 변경은 보존했다.

### v4.29 - 2026-08-28

- CF-FQ-039를 VT09 Reference Structure evidence에서 실제 Production-first Static Asset 제작으로 전진했다.
- canonical VT-VEH-01 픽셀만 사용한 VT10 FrameOverlay/SpeedFrame/ArmorFrame 실제 Source 3종과 isolated QA board를 생성 PASS했다.
- runtime 영역은 transparent cleanup하고 임의 visual pixel 추가 0, UE Import/Designer/Production Asset mutation 0을 유지했다.
- 현재 Gate는 USER Static Asset Review다. CF-FQ-040 Ready projection과 다른 dirty 작업은 변경하지 않았다.

### v4.27 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 8 Technical Driving + USER Driving Guided Flow` Technical PASS를 Ready 복원 projection에 동기화했다.
- 대표 Plan/Roadmap을 v0.1.20, ShellSpec을 v0.1.9로 전진하고 next gate를 `actual new vehicle Guided E2E + USER Driving Acceptance`로 고정했다.
- Step 8은 existing VB-P0-08 fixed-60Hz runner의 saved Target exact path/hash/RunId binding을 사용하고, selected VehicleData는 active PIE Pawn에 transient duplicate로만 적용한다. 실제 PIE 적용 전 USER PASS는 차단한다.
- Step1~7 Technical PASS는 반복하지 않았고 final build/focused Step8/runner envelope canary는 대표 Plan evidence로만 보존한다. canary 뒤 canonical Saved metric JSON은 기존 DefenseSUV 내용으로 복원했다.
- 기존 단일 Active CF-FQ-039는 변경하지 않았다. 실제 신규 차량 USER E2E와 P0-10 Systems promotion은 Pending이다.

### v4.26 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 7 Final Review / explicit Apply / guarded Undo Guided Flow` Technical PASS를 Ready 복원 projection에 동기화했다.
- 대표 Plan/Roadmap v0.1.19, ShellSpec v0.1.8로 전진하고 next gate를 `Step 8 Technical Driving + USER Driving Guided connection`으로 고정했다.
- Step7은 existing R0 review + exact R3 DefinitionApply + Builder-owned guarded Undo만 사용하며 auto Save/retry와 generic Undo를 추가하지 않았다.
- 기존 CF-FQ-039 Active checkpoint는 변경하지 않았다. VB-P0-09 USER E2E와 P0-10 Systems promotion은 Pending이다.

### v4.25 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 6 Gameplay Setup Guided Flow` Technical PASS를 Ready 복원 projection에 동기화했다.
- 대표 Plan/Roadmap을 v0.1.18, ShellSpec을 v0.1.7로 전진하고 next implementation gate를 `Step 7 Final Review / explicit Apply Guided connection`으로 고정했다.
- Step 6은 existing R0 Gameplay Guidance의 8영역 completeness/USER Socket 안내/pending diff를 read-only로 표시하며 Target Apply와 Save를 수행하지 않는다.
- 기존 v4.24 CF-FQ-039 VT07 checkpoint와 단일 Active 상태를 보존했다. VB-P0-09 USER E2E와 P0-10 Systems promotion은 Pending이다.

### v4.24 - 2026-08-27

- URT05를 Method Validation Complete로 닫고 잘못 확장된 WeaponPanel Production handoff/C++ 확장을 취소·원상복구했다.
- Asset-First 방법을 현재 Active CF-FQ-039 VehiclePanel로 이전했다.
- VT07 Speed/Armor Local Frame Source 2종과 422×272 PNG derivative를 생성 PASS했다.
- VT06 outer frame + current RPM/Armor/Silhouette/Defense Source를 재사용한 `VT07_VehPanelReview.png` 896×416 generation/readback PASS를 확보했다.
- next gate는 whole-panel USER Visual Review이며 UE Import/Production Asset mutation 0을 유지한다.
- CF-FQ-040 Ready projection은 변경하지 않았다.

### v4.23 - 2026-08-27

- URT05 Authoring Source 5종을 Runtime용 Frame 1 + Accent 1 두 PNG로 실제 패킹했다.
- `URT05_WeaponResFrame.png` / `URT05_WeaponResAccent.png`은 720×68 Project Image readback과 generation PASS를 확보했다.
- Production Handoff를 `URT05_ProdHandoff.json v0.2.0`, Plan을 v0.7로 전진하고 next gate를 UE Import + additive WeaponPanel implementation으로 이동했다.
- final in-game USER Visual PASS는 Pending이고 현재 UE Import/Production Asset mutation은 0이다.
- 기존 v4.21 CF-FQ-040 Step 5 projection과 단일 Active CF-FQ-039를 보존했다.

### v4.22 - 2026-08-27

- URT05 Visual Richness Polish 결과를 USER `Visual Baseline Candidate Accepted`로 반영하고 Production Handoff 설계를 완료했다.
- `URT05_ProdHandoff.json v0.1.0`에서 Authoring Source 5종을 Runtime Frame 1 + Accent 1 두 Asset으로 Packing하도록 확정했다.
- 기존 WeaponCharge Runtime/ViewData/Presenter/ResourcePresentation/ProgressBar/FireState semantic은 재사용하고 별도 Gameplay owner를 만들지 않는다.
- final in-game USER Visual PASS는 Pending이고 현재 UE Import/Production mutation은 0이다.
- 기존 v4.21 CF-FQ-040 Step 5 projection과 단일 Active CF-FQ-039는 보존했다.

### v4.21 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 5 Physics Proposal Guided Flow` Technical PASS를 Ready 복원 projection에 동기화했다.
- 대표 Plan/Roadmap을 v0.1.17, ShellSpec을 v0.1.6으로 전진하고 next implementation gate를 `Step 6 Gameplay Setup Guided connection`으로 고정했다.
- Step 5는 transient PhysicsDraft → current accepted Evidence/private 4 Profile typed mutation0 preview → explicit USER AuthoringWrite commit → persistent BuilderCommitReceipt current-truth resume/Stale 구조다.
- Target VehicleData Apply는 Step 7, USER Hardpoint/Socket authority와 shared Profile/raw VehicleData/auto Save 금지는 유지한다.
- 기존 v4.18~20 checkpoint와 단일 Active `CF-FQ-039`는 보존했다. VB-P0-09 전체 USER E2E와 Systems P0-10 승격은 Pending이다.
### v4.20 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 1 Reference Evidence / Companion USER Flow` Technical PASS를 Ready 복원 projection에 동기화했다.
- 대표 Plan/Roadmap을 v0.1.16, ShellSpec을 v0.1.5로 전진하고 next implementation gate를 `Step 5 Physics Proposal review`로 고정했다.
- Step 1은 ResearchDraft exact binding → Evidence discovery → Companion mutation0 review/explicit USER commit → exact EvidenceFingerprint local review token까지 연결됐다. private Profile completeness는 Step 1 hard completion이 아니며 Step 5 numeric proposal authority를 유지한다.
- 기존 v4.18~19 URT05 checkpoint와 단일 Active `CF-FQ-039`는 보존했다. VB-P0-09 전체 USER E2E와 Systems P0-10 승격은 Pending이다.
### v4.19 - 2026-08-27

- URT05 Asset-only Composite에 대한 USER 평가를 `SO-SO / USER HOLD`로 기록하고 Production Visual PASS로 확대하지 않았다.
- 다음 Gate를 새 독립 레이어 추가가 아니라 기존 Source 내부 Visual Richness Polish 1회로 제한했다.
- 1회 Polish 뒤에도 SO-SO면 `Realizable but Visual Appeal Limited`로 종료하고 Production 적용을 강행하지 않는 Exit Rule을 보존했다.
- SurfaceNoise Deferred, CF-FQ-039 Frame Master v5 USER Pending, Production/UE mutation 0을 유지했다.

### v4.18 - 2026-08-27

- URT05 Project bounded derivative generation을 Accepted `powershell51` generic-script lane에서 PASS로 닫았다.
- `RunURT05Sources.ps1 v1.0.1`이 Explicit Path JSON Source 5종에서 SVG 5종을 생성하고 XML parse / 720×68 / baked text0 / filter0을 검증한다.
- Windows PowerShell 5.1 UTF-8-no-BOM generated desc corruption을 ASCII-only desc로 교정하고 fresh readback PASS했다.
- `RecommendedBalance` 색 계층 + 실제 Source 5종 + Production Proven Runtime primitive만 사용한 Normal/Insufficient Asset-only Composite Preview를 준비했고 USER Visual Review Pending으로 이동했다.
- SurfaceNoise Deferred, CF-FQ-039 Frame Master v5 USER Pending, Production/UE mutation 0을 보존했다.

### v4.17 - 2026-08-27

- URT05 Structural 3종 exact-source raster QA를 완료해 TECH PASS / Visual Identity Insufficient로 판정했다.
- ROI가 높은 `CornerDetail`과 `SubtleGlowStrip` 독립 Source를 생성하고 decorative individual raster QA PASS했으며 `SurfaceNoise`는 Deferred했다.
- `MakeURT05Sources.py v1.2.0`을 Structural 3 + Decorative 2 총 5종 derivative 대상으로 확장했다.
- Project `process.run` generic-script가 connector argument schema 단계에서 server 실행 전에 거부되어 Project bounded derivative generation은 Pending으로 남겼고 Concept Composite는 생성하지 않았다.
- 기존 v4.16 CF-FQ-040 Step Extensibility Hardening projection과 CF-FQ-039 Frame Master v5 USER Pending, Production/UE mutation 0을 보존했다.

### v4.16 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step Extensibility Hardening`을 Technical PASS로 반영하고 대표 Plan/Roadmap을 v0.1.15, ShellSpec을 v0.1.4로 전진했다.
- current baseline Step의 stable semantic StepId와 presentation 개수/순서를 분리하고, 단일 definition owner + Step별 evaluator + StepId 기반 UI/test + dynamic navigation으로 fixed-index 결합을 제거했다.
- Official Build/BuilderShell focused PASS 상세 evidence는 대표 Plan이 소유한다. USER E2E는 Pending, CF-FQ-040은 Ready, 단일 Active CF-FQ-039와 v4.15 UI Resource checkpoint는 변경하지 않았다.

### v4.15 - 2026-08-27

- URT05 Weapon Charge Module Asset Inventory와 720×68 Common Canvas를 최종 Lock했다.
- `FrameBase / InnerRail / AccentStrip` 3종을 독립 Explicit Path JSON Source로 생성하고 `URT05_StructSpec.json v0.2.0`을 geometry authority로 고정했다.
- direct `.svg` write 차단에 따라 SVG/PNG는 JSON에서 생성되는 derivative-only로 분리하고 `MakeURT05Sources.py v1.1.0`을 단방향 generator로 교정했다.
- Concept Composite는 아직 생성하지 않았으며 다음 Gate를 structural derivative generation + raster Visual QA로 이동했다.
- CF-FQ-039 Frame Master v5 USER Pending과 Production/UE mutation 0을 보존했다.

### v4.14 - 2026-08-27

- UI Resource 방법론 실험의 다음 재개 지점을 `URT05 Asset-First UI Concept Trial`로 기록했다.
- Trial 01~04의 교훈을 반영해 Concept-first 역추출 대신 실제 `FrameBase / InnerRail / AccentStrip` Source 제작 → Asset-only Concept Composite 순서로 전환했다.
- 이 방법론 checkpoint는 CF-FQ-039의 현재 VPR-P0-01 Frame Master v5 USER Pending 상태를 대체하지 않으며 USER PASS 전 Production/UE mutation 0을 유지한다.

### v4.13 - 2026-08-27

- `CF-FQ-040 / VB-P0-09` USER Acceptance 준비에서 Guided Builder Step 2 Mesh / Step 3 Socket / Step 4 Layout runtime evaluator를 fresh AssetSnapshot/current Target authority로 연결하고 대표 Plan/Roadmap을 v0.1.14, ShellSpec을 v0.1.3으로 동기화했다.
- optional FR/RL/RR Wheel 허용, Wheel role 4/4 found+distinct, socket-transform change Layout Stale, duplicate-role Blocked가 focused BuilderShell 1/1 PASS했다. 상세 build/process evidence는 대표 Plan이 소유한다.
- VB-P0-09 전체 USER PASS는 아직 Pending이며 다음은 Step 1 Reference/Companion + Step 5~8 Guided action 연결 뒤 신규 차량 E2E USER 확인이다. CF-FQ-040은 Ready, 단일 Active CF-FQ-039는 변경하지 않았다.

### v4.12 - 2026-08-27

- `CF-FQ-040 / VB-P0-08` post-PASS measurement hardening을 완료하고 대표 Plan/Roadmap 포인터를 v0.1.13으로 동기화했다.
- fixed 60Hz fresh PIE fitted repeat에서 0→50/100, Peak, 100→Idle braking time/distance, Yaw, Turning Radius가 exact 동일했고 no-fitting BaseMass 경로도 최신 코드에서 PASS했다.
- 다음 재개 지점은 `VB-P0-09 End-to-End USER Acceptance` 그대로이며 CF-FQ-040은 Ready, 단일 Active CF-FQ-039는 변경하지 않았다.

### v4.11 - 2026-08-27

- `CF-FQ-040 / VB-P0-08 Technical Driving Benchmark`를 Harness Technical PASS로 전진하고 대표 Plan/Roadmap 포인터를 v0.1.12로 동기화했다.
- fresh PIE actual Chaos benchmark는 UE 5.8 Source-confirmed fixed 60Hz, per-metric PhysicsState rebuild, explicit first gear, fixture isolation을 사용하며 fitted repeat exact 동일 결과와 no-fitting BaseMass PASS를 확보했다.
- 100km/h 미도달 대표 canary의 braking branch 미관측은 제한으로 남기고 다음 재개 지점을 `VB-P0-09 End-to-End USER Acceptance`로 이동했다. CF-FQ-040은 계속 Ready이며 단일 Active CF-FQ-039는 변경하지 않았다.

### v4.10 - 2026-08-27

- `CF-FQ-040 / VB-P0-05~07` 중간 코드검수 hardening을 완료하고 대표 Plan/Roadmap 포인터를 v0.1.11로 동기화했다.
- persistent BuilderCommitReceipt, deterministic Profile/Companion fresh approval replay, one-resolve Final Review, post-Apply Target/AppliedState guarded Undo와 P0-05 direct write-lane Automation을 추가했다.
- latest Official Build PASS, sequential focused VB-P0-05 4/4 + VB-P0-06 1/1 + VB-P0-07 1/1 PASS를 대표 Plan evidence로 보존한다. 재개 지점은 기존대로 `VB-P0-08 Technical Driving Benchmark`이며 단일 Active CF-FQ-039는 변경하지 않았다.

### v4.09 - 2026-08-27

- `CF-FQ-040 / VB-P0-07 Final Review / Validation / Undo`를 Technical PASS로 전진하고 대표 Plan/Roadmap 포인터를 v0.1.10으로 동기화했다.
- Final Review가 existing Validation/External Drift/Diff와 P0-06 Gameplay guidance를 재사용하고, accepted Evidence consumed Claim provenance + explicit DefinitionApply + exact top transaction guarded Undo를 제공하는 현재 경계를 대표 Plan evidence로 보존한다.
- 다음 재개 지점을 `VB-P0-08 Technical Driving Benchmark`로 이동했다. CF-FQ-040은 계속 Ready이며 단일 Active CF-FQ-039는 변경하지 않았다.

### v4.08 - 2026-08-27

- `CF-FQ-040 / VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`를 Technical PASS로 전진하고 대표 Plan/Roadmap 포인터를 v0.1.9로 동기화했다.
- Durability/Defense/DestroyedFx/Hardpoint/Mount/DriveState/WheelVisual/FittingMass 8영역의 R0 completeness guidance와 USER Hardpoint Socket authority, Existing Vehicle stored transform baseline-preservation을 대표 Plan evidence로 보존한다.
- 다음 재개 지점을 `VB-P0-07 Final Review / Validation / Undo`로 이동했다. CF-FQ-040은 계속 Ready이며 단일 Active CF-FQ-039는 변경하지 않았다.

### v4.07 - 2026-08-26

- `CF-FQ-040 / VB-P0-05`를 Technical PASS로 전진하고 대표 Plan/Roadmap 포인터를 v0.1.8로 동기화했다.
- typed Transmission authoring, Evidence-bound private 4 Profile commit, companion creation과 baseline-safe Existing Vehicle Completion 구현/검증을 대표 Plan evidence로 보존한다.
- 다음 재개 지점을 `VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`로 이동했다. CF-FQ-040은 계속 Ready이며 단일 Active CF-FQ-039는 변경하지 않았다.

### v4.06 - 2026-08-26

- `CF-FQ-040 / VB-P0-04` final source closure에서 local UE 5.8 exact Source를 확인해 Reverse ratio setup array는 positive magnitude를 저장하고 `GetGearRatio()`가 reverse 부호를 적용하는 convention으로 확정했다.
- detailed owner를 `VehicleBuilderProposalSpec.md v0.1.1`, 대표 Plan/Roadmap을 v0.1.7로 전진하고 VB-P0-04를 Source Closure TRUE PASS로 닫았다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation 없이 Implementation 0을 유지하며 next gate는 `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow`다.

### v4.05 - 2026-08-26

- `CF-FQ-040 / VB-P0-04 Reference → Authoring Proposal Bridge`를 Design PASS로 전진하고 상세 owner `VehicleBuilderProposalSpec.md v0.1.0`을 연결했다.
- complete Builder-private 4 Profile prospective payload, field provenance/Unknown 보존, Feel Neutral anchor와 UE 5.8 exact Transmission mapping/typed ratio-set 설계를 확정했다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation 없이 Implementation 0을 유지하고 next gate를 `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow`로 이동했다.

### v4.04 - 2026-08-26

- `CF-FQ-040 / VB-P0-03 Mesh & Socket Guidance`를 Design PASS로 전진하고 대표 owner를 Plan/Roadmap v0.1.5, ShellSpec v0.1.2로 동기화했다.
- `FCFVehicleAssetReader / FCFVehicleAssetSnapshot` 재사용, Wheel role 4/4 existing+distinct hard rule, optional Hardpoint/Destroyed FX boundary와 Layout fresh equality/stale 계약을 고정했다.
- C++/Asset/schema/capture mutation 없이 Implementation 0을 유지하고 next gate를 `VB-P0-04 Reference → Authoring Proposal Bridge`로 이동했다. Transmission numeric schema/Chaos mapping은 아직 미착수다.

### v4.03 - 2026-08-26

- `CF-FQ-040`에 실제 다단 변속기 Authoring 요구를 Builder-private Drivetrain Profile 정식 범위로 통합하고 대표 owner 포인터를 Plan/Roadmap v0.1.4, ShellSpec/RefEvidenceSpec v0.1.1로 동기화했다.
- Reference Evidence → AI Drivetrain Proposal → private Drivetrain Profile → 향후 UE 5.8 Chaos `TransmissionSetup` mapping 경계를 추가했지만 exact schema/API mapping과 구현은 VB-P0-04까지 보류했다.
- VB-P0-02 PASS와 current next gate `VB-P0-03 Mesh & Socket Guidance`, Implementation 0, single Active CF-FQ-039는 변경하지 않았다.

### v4.02 - 2026-08-26

- `CF-FQ-040 / VB-P0-02 Builder Shell / Step State / Resume`를 Design / Shell-State-Resume Contract PASS로 전진했다.
- 상세 owner를 `VehicleBuilderShellSpec.md v0.1.0`으로 연결하고 persistent authoring truth, local Editor resume convenience, transient approval/cache의 책임을 분리했다.
- fixed 8 Step, 6-state blocker/stale 계약과 `RecipeId` primary resume identity, restart 뒤 old mutation approval 폐기 원칙을 고정했다.
- CF-FQ-040은 계속 Ready / Implementation 0이며 next gate는 `VB-P0-03 Mesh & Socket Guidance`다. `VB-P0-04` numeric mapping은 미착수이고 현재 single Active CF-FQ-039는 변경하지 않았다.

### v4.01 - 2026-08-26

- `CF-FQ-040 / VB-P0-01 Reference Research & Evidence Contract`를 Design / Research Normalization PASS로 전진했다.
- 상세 owner를 `VehicleRefEvidenceSpec.md v0.1.0`으로 연결하고 identity/citation/provenance/Unknown/conflict/confidence/fingerprint/proposal-binding 계약과 실제 경차 dry-run PASS를 보존했다.
- CF-FQ-040은 계속 Ready / Implementation 0이며 next gate는 `VB-P0-02 Builder Shell / Step State / Resume`다. 현재 single Active CF-FQ-039는 변경하지 않았다.

### v4.00 - 2026-08-26

- `CF-FQ-040 / VB-P0-00 Current Vehicle Creation Contract Audit`을 read-only로 PASS했다.
- Reference Evidence owner, Builder-private Profile 4종 AI numeric owner, minimum created Asset 7개와 existing `ue.call_write` transport 재사용 경계를 대표 Plan에 고정했다.
- CF-FQ-040은 계속 Ready / Implementation 0이고 next gate는 `VB-P0-01 Reference Research & Evidence Contract`다. 현재 single Active CF-FQ-039는 변경하지 않았다.

### v3.99 - 2026-08-26

- USER 승인으로 `CF-FQ-040 Guided Vehicle Builder`를 Ready 복원 항목으로 추가했다. 실존 차량 Reference 기반 AI Authoring + USER 직접 Mesh/Socket 준비 + 단계형 제작 UX를 Design Baseline으로 고정했고 첫 Gate는 `VB-P0-00 Current Vehicle Creation Contract Audit`이다.
- `CF-FQ-038`은 Builder의 Recipe/Resolver/Diff/Validation/Apply/Undo Backend + Advanced Workspace 역할로 projection을 갱신했다. 기존 evidence와 DEL6/UA-08 deferred 경계는 유지한다.
- 현재 single Active `CF-FQ-039`는 변경하지 않았다.

### v3.98 - 2026-08-26

- USER가 v4 내부 임시 box/seam/deck geometry를 최종 Frame에 남기면 안 된다고 판정해 v4 USER Visual을 Reject 처리했다.
- Frame Master를 outer-shell-only ownership으로 교정하고 `VT06_FrameMaster_v5.png` / `VT06_FrameReview_v5.png`를 생성해 AI Source QA PASS했다.
- 대표 Plan/Roadmap을 v0.1.28/v0.1.27로 동기화했다. USER PASS 전 UE Import·local frame assembly·content reposition은 0이다.

### v3.97 - 2026-08-26

- Frame Master v1~v4 deterministic Source/Review를 생성·AI QA해 v1~v3 contamination 후보를 제외하고 v4를 Pre-USER-PASS candidate로 고정했다.
- `VT06_FrameMaster_v4.png` / `Review/VT06_FrameReview_v4.png`는 AI Source QA PASS이며 USER Visual PASS는 Pending이다.
- 대표 Plan/Roadmap 포인터를 v0.1.27/v0.1.26으로 동기화했다. UE Import와 RPM/Speed/Gear/Silhouette/Armor reposition은 0이다.

### v3.96 - 2026-08-26

- USER 지시로 VPR-P0-01 우선순위를 RPM 미세조정에서 Vehicle Panel Frame Master로 변경했다.
- canonical `VT-VEH-01` 1280×594와 current P2 9-Slice frame 직접 비교에서 상위 frame hierarchy 불일치를 새 related Visual failure로 확인했다. 과거 FrameReview PASS는 Historical extraction evidence로 보존한다.
- 대표 Plan/Roadmap 포인터를 v0.1.26/v0.1.25로 동기화하고 다음 Gate를 Frame Master source/review → USER PASS → `Image_FrameOverlay` assembly로 변경했다. RPM/Speed/Gear/Silhouette/Armor 재배치는 그 뒤로 Deferred했다.

### v3.95 - 2026-08-25

- USER PIE에서 RPM 외형이 변하지 않았고 SourceArt 폴더에도 RPM-S1 Source Pack이 없다는 지적을 받아 실제 저장소를 재감사했다. `SourceArt/UI/HUD` 전체의 RPM/VT02/MaskPack 정적 이미지 검색 결과는 기존 `P2/T_UI_RPMTrack.png` 1건뿐이며 `T_UI_RPMMaskPack`, `VT02_*`는 0건이었다.
- Host-global GoPyMCP Trash도 같은 이름 범위로 감사했으며 RPM-S1 관련 이미지 0건을 확인했다. 따라서 과거 v3.86/v3.87의 local production/USER review 기록은 현재 persisted repository Source evidence가 아니며 Current 완료 판정에 사용할 수 없다고 교정했다.
- Current Gate를 RPM-S1 Source reconstruction + Production Apply Required로 재개했다. 기존 `Image_RPMGauge + M_UI_RPMGauge + RPMRatio` runtime 구조는 PASS로 보존하되 새 RPM Visual은 아직 실제 PIE에 적용되지 않았다.
- 현재 repository-bound VehiclePanel Master는 USER가 추가한 `SourceArt/UI/HUD/VT/VT_VehPanel01.jpg`이며, Defense Source 4종 + `M_UI_DefFill` Production Assetization/5-field binding PASS는 보존하고 WBP Defense Assembly는 RPM 적용 뒤 재개한다.

### v3.94 - 2026-08-25

- authority ChassisMesh 기반 `VT05_VehSil_Sedan.png` / `VT05_VehSil_SUV.png` 512×256 transparent PNG Source를 Technical PASS로 확인하고 USER 진행 승인 뒤 Production Assetization을 수행했다.
- `T_UI_VehSil_Sedan` / `T_UI_VehSil_SUV`을 생성하고 persisted `DA_CFHUDVisual_Default.VehicleSilhouettes`를 exact VehicleData identity 3 entry로 저장했다. Sedan은 Sedan Texture, 두 SUV VehicleData는 동일 SUV Texture를 공유하며 legacy `VehicleSilhouette` fallback은 보존했다.
- Current Gate를 Defense source family production으로 전진하고 exact `VT-VEH-01` Master Source repository binding은 별도 Pending으로 유지했다. 대표 Plan/Roadmap/ArtSpec/System 포인터를 v0.1.24/v0.1.24/v0.4.18/v1.1.10으로 동기화했다.

### v3.93 - 2026-08-25

- 현재 persisted `CFVehicleData` 3종을 재확인해 `DA_TestSedan → Sedan.Sedan`, `DA_TestSUV → SUV.SUV`, `DA_VehicleDefense_TestSUV → SUV.SUV` exact mapping을 고정했다.
- vehicle-specific silhouette Source Authority를 `VehicleData identity → VehicleVisualConfig.ChassisMesh → mesh-derived silhouette Source`로 확정했다. 3 VehicleData가 2 unique ChassisMesh를 사용하므로 Sedan/SUV Source 2종만 필요하며 SUV 두 VehicleData는 동일 Texture를 공유할 수 있다.
- P2 generic silhouette와 exact Chassis identity 없는 VT03 후보를 Production authority에서 제외하고 `VehicleSilhouettes` count 0을 유지했다. 다음 Gate는 Sedan/SUV mesh-derived Source Candidate + USER Review이며 승인 전 Import/Bind는 수행하지 않는다.
- 대표 Plan/Roadmap/ArtSpec/System 포인터를 v0.1.23/v0.1.23/v0.4.17/v1.1.9로 전진했다.

### v3.92 - 2026-08-25

- CF-FQ-039 VT04 common Armor Plate를 repository Source → `T_UI_ArmorPlate` → `DA_CFHUDVisual_Default.ArmorCommonPlate`까지 persisted Assetize해 common Plate + Direction 2-Icon Production triplet의 필드 조건을 충족했다.
- Current owner를 `InGameUI.md v1.1.8`, 대표 Plan/Roadmap/ArtSpec을 v0.1.22/v0.1.22/v0.4.16으로 전진했다.
- `VehicleSilhouettes` count 0과 Designer-owned Sector 배치·크기·Icon 선택·회전은 보존했다. 다음 Gate는 vehicle-specific silhouette Source Authority / identity mapping이며 Master Source와 USER Visual은 Pending이다.

### v3.91 - 2026-08-25

- CF-FQ-039 VT04 Arrow/Chevron2 2종을 repository Source와 UE Production Texture로 Assetize하고 `DA_CFHUDVisual_Default`의 두 Direction Icon field에 persisted 연결했다.
- Current owner를 `InGameUI.md v1.1.7`, 대표 Plan/Roadmap/ArtSpec을 v0.1.21/v0.1.21/v0.4.15로 전진했다.
- common Plate, first vehicle-specific silhouette, exact Master Source와 USER Visual은 Pending으로 유지하고 6개 Sector의 Designer-owned 배치·크기·Icon 선택·회전은 변경하지 않았다.

### v3.90 - 2026-08-25

- CF-FQ-039 VehicleData별 silhouette runtime binding과 `WBP_CFArmorSector.Image_DirectionIcon` additive migration을 Technical PASS로 닫았다. 기존 BodyMap 6개 Sector name/class/Designer Slot은 보존됐다.
- UE 5.8 Widget GUID 계약을 반영한 clean migration이 Exit 0 / 변경0으로 idempotent PASS했고, fresh persisted AssetDump와 `CarFight.UI.UI_P0_06` 17/17 PASS를 확인했다.
- 대표 Plan/Roadmap/HUD Art Spec/System 포인터를 v0.1.20/v0.1.20/v0.4.14/v1.1.6으로 전진했다. Source Binding + Production Art Import/Binding + 새 modular USER Visual PASS는 Pending이다.

### v3.89 - 2026-08-25

- CF-FQ-039 Armor 구조를 USER 결정에 맞춰 재정의했다. Vehicle silhouette는 `VehicleData` identity별로 교체하고, 방향 Art는 common Plate + `Arrow/Chevron2` 2종 Icon을 Designer에서 배치·회전하는 구조다.
- 기존 `VT03` six-direction local pack은 USER PASS 없이 Superseded하고 `VT04` 2-icon Source/Review/Manifest를 Current local evidence로 전환했다. 기존 `WBP_CFArmorSector` 6개 instance name과 Presenter Armor Ratio binding은 유지한다.
- 대표 Plan/Roadmap/HUD Art Spec 포인터를 v0.1.19/v0.1.19/v0.4.13으로 동기화하고 Current Gate를 `Armor Modular Structure Locked / 2-Icon Source Pack Ready / Vehicle Silhouette Binding + WBP Migration Pending / Source Binding Deferred`로 전진했다. UE Import/WBP/C++ mutation은 0이다.

### v3.88 - 2026-08-25

- CF-FQ-039 `VT01_FrameReview_v2` Frame Family USER PASS를 반영했다. RPM-S1과 Frame Review는 새 관련 결함이 없는 한 반복하지 않는다.
- `ARMOR-S1` local source pack을 준비해 full target wireframe silhouette + common target-derived tintable Plate + 6방향 icon-only family를 Current next gate로 올렸다. 현재 USER Gate는 `VT03_ArmorReview_v2`다.
- 대표 Plan/Roadmap/HUD Art Spec 포인터를 v0.1.18/v0.1.18/v0.4.12로 동기화했다. Source Binding Deferred / UE Import·WBP·Runtime mutation 0을 유지한다.

### v3.87 - 2026-08-25

- CF-FQ-039 RPM-S1은 AUTO QA PASS에 이어 `VT02_RpmRuntimePack_Review_FINAL3` USER Visual PASS까지 완료했다. RPM Review는 새 관련 결함이 없는 한 반복하지 않는다.
- 대표 Plan/Roadmap/HUD Art Spec 포인터를 v0.1.17/v0.1.17/v0.4.11로 동기화하고 Current Gate를 `RPM-S1 USER PASS / Frame USER Review Pending / Source Binding Deferred`로 전진했다.
- 다음 USER Gate는 `VT01_FrameReview_v2`이며 PASS 뒤 Armor silhouette + 6 icon-plate source extraction으로 이동한다. UE Import/WBP/Runtime mutation은 0이다.

### v3.86 - 2026-08-25

- CF-FQ-039 작동형 `RPM-S1 Source Mask Pack` local production을 완료하고 21 Tick / 21 Progress / Red 4 Tick / 0·5·10·17·21 progression AUTO QA PASS를 세션 복원 projection에 반영했다.
- `T_UI_RPMMaskPack` 724×467 RGBA packed candidate를 준비해 R=Progress, G=Tick21, B=Red, A=GuideRail로 고정하고 기존 `Image_RPMGauge + M_UI_RPMGauge + RPMRatio` 구조를 보존했다.
- 대표 Plan/Roadmap/HUD Art Spec을 v0.1.16/v0.1.16/v0.4.10으로 동기화했다. Current Gate는 Frame+RPM USER Review Pending이며 Source Binding Deferred / UE Import·WBP·Runtime mutation 0을 유지한다.

### v3.85 - 2026-08-25

- CF-FQ-039 Frame F1~F4 Local Pre-Binding Extraction Evidence 완료를 세션 복원 projection에 반영했다. F3 ArmorFrame v2 cleanup과 FrameMock_v2/FrameReview_v2/SHA manifest가 준비됐다.
- USER local PC access 제약으로 repository Source Binding은 Deferred 유지하며, 현재 Gate를 `Frame F1~F4 Local Extraction Ready / Source Binding Deferred / USER Frame Review Pending`으로 전진했다.
- 대표 Plan/Roadmap/HUD Art Spec을 v0.1.15/v0.1.15/v0.4.9로 동기화했다. UE Import/WBP/Runtime mutation은 0이다.

### v3.84 - 2026-08-24

- CF-FQ-039 `VT-VEH-01` USER APPROVED / LOCKED와 Frame family F0~F6 extraction plan ready를 세션 복원 projection에 반영했다.
- 대표 Plan/Roadmap/HUD Art Spec을 v0.1.14/v0.1.14/v0.4.8로 동기화하고 Current Gate를 `VT-VEH-01 Locked / Frame Family Extraction Plan Ready / Source Binding Pending`으로 전진했다.
- 실제 source copy/extraction/UE Asset mutation은 아직 0이다.

### v3.83 - 2026-08-24

- CF-FQ-039 Slot Contract와 Physical Art Breakdown Contract 완료를 반영해 대표 Plan/Roadmap/HUD Art Spec을 v0.1.13/v0.1.13/v0.4.7로 전진했다.
- Current Gate를 `Physical Art Breakdown Contract Ready / VT-VEH-01 Lock + Source Binding Pending`으로 변경했다. 실제 Art/UE Asset/Source mutation은 아직 0이다.

### v3.82 - 2026-08-24

- CF-FQ-039 USER 승인으로 VehiclePanel 상단 Speed/RPM : Armor를 50:50으로 고정하고 대표 Plan/Roadmap/HUD Art Spec을 v0.1.12/v0.1.12/v0.4.6으로 동기화했다.
- 896×416 기준 `422 + Gap 12 + 422` Composition을 Current로 투영했다. `VT-VEH-01` 승격과 Physical Art production은 아직 Pending이다.

### v3.81 - 2026-08-24

- CF-FQ-039 `CAND-VEH-MASTER-01` Logical Breakdown과 WBP Skeleton Contract 완료를 반영해 대표 Plan/Roadmap을 v0.1.11/v0.1.11로 전진했다.
- Current Gate를 `WBP Skeleton Contract Ready / Slot Contract Review Pending`으로 변경했다. UE Asset/Source mutation은 없으며 기존 Runtime/Technical evidence를 보존한다.

### v3.80 - 2026-08-24

- CF-FQ-039 USER 결정으로 기존 WBP/Primitive-first 제작 방식을 Deprecated하고 Target Decomposition Pipeline을 Current 제작 계약으로 고정했다.
- Plan/Roadmap/HUD Art Spec 포인터를 v0.1.10/v0.1.10/v0.4.5로 동기화하고 Current Gate를 `Production Method Reset Locked / Target Logical Breakdown Pending`으로 전환했다.
- 기존 Armor USER PASS와 Frame/Shield/Integrity Technical·Pixel evidence는 보존하되 현재 Primitive/P2 결과를 Production Visual authority로 사용하지 않는다. 병렬 CF-FQ-038 상태는 변경하지 않았다.

### v3.79 - 2026-08-24

- CF-FQ-039 Defense Bar final focused + fresh Defense Pawn pixel revalidation을 PASS해 Current Gate를 Frame + Shield/Integrity Technical/Pixel PASS / USER Visual Review Pending으로 복구했다.
- 대표 Plan/Roadmap/System 포인터를 v0.1.9/v0.1.9/v1.1.5로 동기화했다. Armor USER PASS와 전체 `VT-VEH-01` Pending 경계는 유지한다.
- 병렬 CF-FQ-038 v3.78 상태는 변경하지 않았다.

### v3.78 - 2026-08-24

- `CF-FQ-038` Legacy Wizard Deprecated transition을 Technical Complete로 닫았다. shared Official Editor Build PASS 상태에서 `Workspace.TabRegistration` 1/1 Success와 fresh Editor `창(Window)`의 Current Workspace 존재 / Legacy Wizard 메뉴 부재를 확인했다.
- `LegacyManagedGuard`는 이번 change set에서 guard Source mutation0이라 기존 PASS를 보존했다. DEL6 required-caller0 Pending과 UA-08 quantitative comparison Deferred는 non-blocking 잔여로 남기고 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v3.77 - 2026-08-24

- CF-FQ-039 fresh PIE pixel review에서 Integrity Bar 소실을 발견해 Frame + Shield/Integrity Technical PASS를 재검증 Pending으로 교정했다.
- final pixel fix는 `CFHUDPresenter v1.25.1` Track/Fill/Marquee 10px intrinsic size이며 Official Build `d9c33e2a689e42b2a511698fa021df4f` PASS. focused rerun + fresh PIE pixel revalidation은 Pending이다.
- Plan/Roadmap/System 포인터를 v0.1.8/v0.1.8/v1.1.4로 동기화했다. Armor USER PASS는 보존한다.

### v3.76 - 2026-08-24

- CF-FQ-039 Frame + Shield/Integrity production visual을 Technical PASS로 전진하고 Plan/Roadmap/System 포인터를 v0.1.7/v0.1.7/v1.1.3으로 동기화했다.
- 다음 Gate를 fresh Editor USER Visual Review로 좁혔으며 Armor USER PASS와 전체 `VT-VEH-01` Pending 경계는 유지했다.
- 병렬 CF-FQ-038 v3.75 변경은 보존하되, 이번 full official Editor Build PASS로 unrelated UI compile blocker가 사라진 사실만 projection에 최소 반영했다. CF-FQ-038 focused validation은 해당 재개 세션에 남겼다.

### v3.75 - 2026-08-24

- `CF-FQ-038` Deprecated Gate DG1~DG5를 PASS하고 Legacy Wizard 기본 Window 메뉴 숨김 Source를 적용했다. `CarFight.VehicleDAWizard` hidden spawner는 DEL6 전까지 보존한다.
- official build는 현재 Active `CF-FQ-039`의 dirty UI compile 오류로 차단돼 Data Authoring exact-build/focused validation만 Pending으로 남겼다. UI Source는 건드리지 않았고 현재 단일 Active `CF-FQ-039`는 유지한다.

### v3.74 - 2026-08-24

- `CF-FQ-038 / DAUTH-P0-12 UA-08`은 transient Sports 4축→Runtime Movement 13 field Technical PASS를 확보했으나 USER 주행 체감은 Inconclusive로 남겼다. 정량 계측 비교는 Deferred이며 USER PASS는 7/8 유지한다.
- 사용자 승인에 따라 P0-12 USER Acceptance cycle을 `Closed with Deferred Feel Comparison`으로 종료하고 다음 재개 지점을 `DG/DEL Gate / Deprecated Gate DG1~DG5 Audit`로 이동했다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v3.73 - 2026-08-24

- CF-FQ-039 Armor visual slice USER PASS를 반영해 Plan/Roadmap 포인터를 v0.1.6으로 동기화했다.
- 다음 VPR-P0-01 Gate를 VehiclePanel Frame + Shield/Integrity visual match로 전진했다.
- 전체 `VT-VEH-01`은 USER 전체 승인 전이라 Pending을 유지했다.

### v3.72 - 2026-08-24

- CF-FQ-039을 VPR-P0-01 Armor Technical PASS / USER Visual Review Pending으로 전진했다.
- 대표 Plan/Roadmap/HUDArtSpec/System 포인터를 v0.1.5/v0.1.5/v0.4.4/v1.1.2로 동기화하고 완료 CF-FQ-032의 Current owner projection도 최신 UI System owner로 맞췄다.
- Build/Automation 상세는 대표 Plan에만 두고 ActiveWork에는 Technical PASS와 next USER Gate만 projection했다.

### v3.71 - 2026-08-24

- `CF-FQ-038 / DAUTH-P0-12 UA-07 Driving Feel Authoring`을 USER PASS로 닫고 P0-12 USER PASS를 7로 전진했다.
- 대표 Plan/Roadmap 포인터를 v0.2.48/v0.1.53으로 동기화하고 다음 재개 Gate를 `UA-08 Driving Feel Runtime Comparison`으로 변경했다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v3.70 - 2026-08-24

- CF-FQ-039의 Vehicle editable Structure Readiness PASS를 반영하고 Plan/Roadmap/HUDArtSpec/System 포인터를 v0.1.4/v0.1.4/v0.4.3/v1.1.1로 동기화했다.
- 기존 `WBP_CFArmorSector` 6개 재사용이 canonical common Armor slot임을 확인해 신규 `WBP_CFArmorSlot` 중복 생성을 막았다.
- USER가 Designer에서 배치를 조정할 수 있는 상태를 확인했지만 `VT-VEH-01` Production Visual Target은 명시 승인 전이라 Pending을 유지한다.

### v3.69 - 2026-08-23

- CF-FQ-039 VPR-P0-00 AI Recovery 완료와 `USER VEHICLE TARGET PENDING` 상태를 반영하고 Plan/Roadmap 포인터를 v0.1.3으로 동기화했다.

### v3.68 - 2026-08-23

- CF-FQ-039 VPR-P0-00을 실제 IN PROGRESS로 전환하고 Plan/Roadmap/HUDArtSpec current 포인터를 v0.1.2/v0.1.2/v0.4.2로 동기화했다.
- Vehicle scope에서 Direction/Composition/Technical Reference/Armor Target decision을 분리했으며 전체 VehiclePanel 원본 Target은 source recovery 중임을 기록했다.

### v3.67 - 2026-08-23

- CF-FQ-039 pre-start design re-review 교정 후 대표 Plan/Roadmap/HUD Art Spec 포인터를 v0.1.1/v0.1.1/v0.4.1로 동기화했다.
- Current Gate는 VPR-P0-00을 유지하며 코드·Asset 작업은 아직 시작하지 않았다.

### v3.66 - 2026-08-23

- 사용자 선택에 따라 `CF-FQ-039 Production UI Visual Rework`를 현재 단일 Active로 등록했다.
- 대표 Plan/Roadmap과 `VPR-P0-00 Visual Target Recovery & Registry`를 세션 복원 포인터로 연결했다.
- `CF-FQ-032 Done`과 완료 evidence는 그대로 보존하고 새 Visual Feature가 이를 재개하거나 무효화하지 않도록 경계를 명시했다.

Migration: 다음 세션의 UI Visual 작업은 `InGameUIVisualPlan.md / VPR-P0-00`에서 복원한다. Historical `CF-FQ-032` old next-action을 현재 Gate로 사용하지 않는다.

### v3.65 - 2026-08-22

- v3.64까지 누적된 UI-P0/DAUTH-P0 단계별 Build·Automation·AssetDump·PIE·Changelog 대형 복제를 제거하고 세션 복원 projection 역할로 정상화했다.
- `CF-FQ-032 Done / 현재 Active 없음`을 유지하고 최근 완료 owner와 반복 금지 evidence만 보존했다.
- `CF-FQ-038`, `034`, `029`, `030`, `035`, `026`, `015`를 실제 재개 Gate 중심으로 축약했다.
- Candidate/Deferred와 세션 복원 절차를 명시해 다음 세션이 상세 과거 로그를 ActiveWork에서 찾지 않도록 했다.

Migration: v3.64 이하에 있던 상세 Build/Automation/USER evidence는 각 대표 Plan, Systems와 Git history에서 조회한다. ActiveWork에서 삭제됐다는 이유로 해당 evidence가 무효화되거나 재검증 대상이 되지 않는다.
