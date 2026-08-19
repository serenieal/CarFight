# CarFight Active Work

- 문서 버전: v3.32


- 최근 갱신일: 2026-08-19
- 문서 상태: Current
- 역할: CarFight 게임 프로젝트에서 현재 실제로 진행 중인 작업을 선택하고 대표 Plan으로 연결하는 세션 복원 projection

---

## 1. 운영 원칙

이 문서는 `main_game`의 현재 작업 선택만 관리한다.
AssetDump와 GoPyMCP의 내부 lifecycle은 각 독립 저장소 문서를 사용한다.

```text
FeatureQueue
= 기능 우선순위와 Active/Paused/Ready/Done 상태

ActiveWork
= 현재 Active/Paused 작업 선택 + 대표 Plan + next gate projection

대표 Plan
= 상세 체크포인트와 validation/evidence owner

Systems
= 완료 기능의 현재 구현

Archive Index
= Historical Plan 탐색
```

이 문서에는 대표 Plan의 전체 Build ID, Automation run history, USER PIE 세부 로그와 과거 override를 복제하지 않는다.

---

## 2. 현재 단일 Active

현재 단일 Active 작업은 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`다. UI-P0-03~05 기존 HUD 데이터·수명 이전은 USER PASS했고 UI-P0-06 차량·무기 HUD는 current-runtime 기준 Technical Complete이며, 현재 formal next runtime Gate는 UI-DESIGN-GATE의 비차단 상태를 보존한 `UI-P0-07 Target Knowledge`다.

```text
CF-FQ-032 대표 Plan: Document/Plan/InGameUIPlan.md v0.59.17
완료 Gate: UI-P0-03 Production Runtime PIE Closure — USER PASS
완료 Gate: UI-P0-04 AimReticle UISubsystem Integration — Build·Automation·USER Visual PASS
완료 Gate: UI-P0-05 TargetSelect Marker Integration — Source·Official Build·focused Automation·USER Visual PASS
현재 Gate: UI-P0-07 Target Knowledge — Ready to Start / UI-P0-06 Vehicle/Weapon HUD는 current-runtime 기준 Technical Complete / Stage A+B Dynamic Resource Visual, RPM Gauge Visual Binding, Weapon Heat P0 Runtime, Applied Fitting Player-facing Weapon Selection Runtime + HUD source, truthful Weapon Rail Visual Consumer, Player-facing Weapon Select Input, WeaponCharge P0 Runtime + HUD Resource Projection Technical PASS 보존 / VehicleBattery는 UI-P0-06 미완료 항목이 아니라 실제 shared-power Gameplay Runtime 부재 시 `Unavailable/Collapsed`가 정답인 future dependency로 분리 / saved WeaponData Charge·Heat tuning과 authoritative Redline 값은 임의 authoring하지 않음 / Rail USER Visual은 representative persisted multi-weapon 차량·피팅이 실제 생길 때까지 content-dependent Deferred·artificial fixture 금지 / Heat·Charge tuning USER Visual과 Redline/RPM USER Visual도 content-dependent Deferred follow-up이며 UI-P0-06 Technical Complete를 차단하지 않음 / final Charge Build `2d9f33261d3c427187f75338b8f37f1d` PASS / exact `WeaponChargeRuntimeResourceContract` process `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS / 다음 formal runtime Gate=UI-P0-07 Target Knowledge
보존 USER PASS: UI-P0-02 Runtime Lifetime / Speed / Weapon Cooldown / Target 선택·해제 / Ammo·Ripple·Salvo Presentation / Defense Production Panel / Pawn Rebind·Old Pawn Event Isolation / AimReticle 단일 수명·Pawn Rebind / Target Marker 단일 표시·Offscreen 재진입·Clear/Reselect
보존 Technical PASS: Defense Runtime→Provider ViewData / Pawn Rebind Old Pawn 이벤트 해제 / Target Marker Game Layer 단일 수명·Weak Pawn·Null Rebind 계약
Target/Radar: CF-FQ-037 Scanner Done + SensorContact.md v1.1.0 Current를 source로 사용하고 Widget에서 Sensor Gameplay를 재계산하지 않음
금지: UI-P0-03 USER Gate를 자동화로 PASS 추정 / Border Mosaic 방식 복귀 / Gameplay 계산을 Widget으로 이동
```

### 보존된 CF-FQ-038 Paused 체크포인트

```text
CF-FQ-038 대표 Plan: Document/Plan/DataAuthoringPlan.md v0.2.32
상세 Roadmap: Document/Plan/DataAuthoringRoadmap.md v0.1.40

완료 Gate: DAUTH-P0-11 Technical Validation — Technical PASS
Core Technical Validation: PASS
Frozen 24.90~24.94 Completeness: PASS
보존 Gate: DAUTH-P0-12 USER Authoring Acceptance — Paused
P0-12 USER Acceptance PASS: 3 / UA-01~03 USER PASS / UA-04 External Drift 3-way Technical flow PASS but USER UX Remediation Required / Last Applied925·Raw950·Authoring925 3-way, unreviewed Apply block, Keep Authoring, final925 recovery 정상 / USER는 세 값·선택지의 개별 의미는 이해하지만 전체 Authoring 복구 흐름을 거시적으로 복잡하다고 판정 / 다음 Gate=Frozen safety contract 유지한 macro-flow UX 단순화 → targeted regression → UA-04 bounded USER recheck

Technical PASS: P0-08A Editor-only Recipe + 5 Profile / Never-Cook / Common Types
Technical PASS: P0-08B Historical baseline Stable Field Path / Field Value Codec / 117 Registry Coverage / Current additive schema compatibility: 118 Registry bidirectional coverage PASS
Technical PASS: P0-08C Immutable Recipe/Profile/Definition/Project Default Snapshot + RequiredDependencies
Technical PASS: P0-08D Chassis Socket / Wheel Bounds Asset Snapshot Reader + resolver-relevant fingerprint
Technical PASS: P0-08E Snapshot-only Pure Resolver / Frozen R0~R16 / deterministic source precedence / Trace·Diff·Stale foundation
Technical PASS: P0-08F transient Definition Materializer / Stable-ID arrays / FieldCodec import / existing UCFVDAValidator / readback hash consistency
Technical PASS: P0-08G Existing Definition Import / Legacy Pin·hidden serialized partition / group·field Adoption Preview / Recipe-only commit
Technical PASS: P0-08H TOCTOU-safe FCFVehicleApplyService / transient preflight / dependency-safe exact Diff / AppliedState / atomic rollback / no-auto-save
Technical PASS: P0-08I FCFVehicleAuthoringService facade / R0~R3 typed contract / prospective Recipe preview / guarded Recipe write / shared Apply lane / no-auto-retry·no-auto-save
Technical PASS: P0-08J Historical baseline projection-only FCFBatchColumnRegistry / Recipe 7 + Profile 78 numeric allowlist / 117 resolved projection / canonical UTF-8 CSV / immutable baseline manifest / ExportSetHash / Current additive compatibility: Performance 18·Profile numeric 79 / 118 resolved projection PASS
Technical PASS: P0-08K canonical CSV+manifest import parse / current-Unreal 3-way / Ownership conflict / transient Recipe·Profile prospective Resolver preview / BatchPlanHash / persistent mutation0
Technical PASS: P0-08L exact Batch approval binding / B1 Recipe+B2 shared Profile source commit / global TOCTOU preflight / one transaction / all-or-nothing rollback / old approval invalidation
Technical PASS: P0-08M fresh R3 BatchApplyPlan / exact B3 approval / all-target global preflight / canonical per-Vehicle ApplyService / Stop-On-First-Failure partial result
Technical PASS: P0-09 single-Vehicle Workspace / Browser·Selection / Initial Import / Recipe Intent / Resolve·Diff·Trace·Validation / Apply·standard Undo / Raw DA Open
Technical PASS: P0-10 Reference Compare / Assets+Layout semantic migration / Measurement+Adoption / 4-axis Driving Feel / Mount+Defaults / managed Wizard guard / standard Undo
P0-11 Core PASS: representative Recipe edit→approval invalidation→fresh Resolve·Trace·Diff·Validation→Apply→Undo→re-Apply→Raw Drift fail-closed / applied VehicleData Inventory+Fitting consumer regression
Final Build: 98dfceab797942218bd09c844e398ceb / Exit Code 0
Focused P0-11 Automation: b53998e6cacf48a1a554d784b77e013c / 6/6 PASS / 0 FAIL
Focused Result JSON SHA-256: bce3414ec30cb4612209ff1f39f649990140950b73d4136929a3fa8a4be47263
Inventory representative pre-closure evidence: 4dfa10d20f00451abcab209b0db49d40 / 1/1 PASS
Fitting representative pre-closure evidence: 1c66a7139313458f9c5ab68ff03a7b0f / 2/2 PASS
Targeted Automation: 77b45525549b4866995b3d972fb4a9fa / 63/63 PASS / 0 FAIL
Result JSON SHA-256: a4ef815caceb5fb5f413b849393f2d5e8ac1e45929f7f5a3cf1f50347bc0bb1f
Reusable Runner: Tools/RunDataAuthoringTests.ps1
Runtime UCFVehicleData / Inventory / Fitting 변경: 0
Content Asset 변경: 0
Asset Snapshot Reader / Existing UCFVDAValidator 변경: 0
SCFVDAWizardTab 삭제/대체: 0 / 기존 CarFight.VehicleDAWizard 병행 유지
Legacy managed-target guard: Layout Capture / Quick Tune Apply / Quick Tune Revert 비활성 + handler 재검사
Vehicle Authoring Workspace: CarFight.VehicleAuthoring / P0-10 parity + Nomad Tab spawn Automation PASS
P0-10 USER Visual/Usability/Driving Feel Acceptance: 미판정 / P0-12까지 보존
P0-11 Frozen closure: Handling/Performance Adoption / Shared Profile edit+impact / External Drift 3-way recovery / Mesh-only Candidate+Create Vehicle From Mesh — Technical PASS
P0-11 production scope: Common Authoring facade/Workspace + Editor-only Applied trace metadata / Runtime VehicleData·Inventory·Fitting production mutation 0
P0-12 USER Acceptance: Paused / UA-01~03 USER PASS / USER PASS 3 / UA-04 Technical flow PASS·USER UX Remediation Required — 개별 3-way 값과 recovery option 의미는 이해 가능하나 전체 Authoring recovery flow가 거시적으로 복잡하다는 USER feedback 확보 / 현재 UA-04 시험 후 `DA_TestSedan` + `DA_Recipe_TestSedan` live dirty / 다음 bounded Gate=baseline 보존 여부 정리 후 macro-flow UX remediation

DG/DEL Gate: 미개방 / P0-12 완료 전 개방 금지
Batch main page: 0
B1·B2 source commit: Technical PASS / Target mutation 0 / Auto Save 0
B3 Definition Apply: Technical PASS / per-Vehicle atomic / global transaction·rollback 0 / Auto Save 0
Production Target writer: FCFVehicleApplyService only
BatchOperationId/idempotency generic envelope: Frozen 26.66~26.69 follow-up / first external Batch transport-client integration 전 재검토
Automatic Retry / Automatic Save: 0 / 0
```

`CF-FQ-034 차량 피팅·질량 런타임`은 `FIT-P0-07A~07C`의 공식 Light/Default/Heavy 기술 증거와 정량 Mobility baseline을 그대로 보존한 `Paused`다. 재개 지점은 `VehicleFittingPlan.md v0.17.0 / FIT-P0-07D USER Driving Feel Comparison`이며 USER 주행감 PASS를 추정하지 않는다.

`CF-FQ-029 LM-P0-06A Failure Policy Technical Closure`는 Technical PASS로 종료했고, 남은 `LM-P0-06 Launcher Integration USER PIE`는 사용자 직접 검증 체크포인트로 보존한다. `CF-FQ-030 Persisted Missile Test Asset Technical Verification`도 PASS로 닫았지만 전체 Feature는 `Ready / CF-TC-027 Manual PIE Pending`을 유지한다. USER PIE가 필요한 기능을 자동으로 Active로 유지하거나 다른 Ready 기능을 사용자 선택 없이 승격하지 않는다.

```text
CF-FQ-029 대표 Plan: Document/Plan/LauncherMissilePlan.md v0.14.0
완료 Gate: LM-P0-06A Failure Policy Technical Closure — Technical PASS
최신 Build: 6a633eb4cf21481d8ae26ec908d05660 / Exit Code 0
Launcher Automation: 4/4 PASS
Ammo LauncherLock: 1/1 PASS
다음 체크포인트: LM-P0-06 MuzzleBlocked·Ejection·Carrier Velocity USER PIE
Content Asset / Blueprint / USER PIE 이번 작업: 0
```

### v3.32 - 2026-08-19

- `CF-FQ-038 / DAUTH-P0-12 UA-04`에서 External Drift 3-way의 기술 동작은 PASS했다: Last Applied 925 / Raw 950 / Authoring 925 비교, unreviewed Apply 차단, Keep Authoring review, explicit final Apply, External Drift 0 복구를 확인했다.
- USER는 세 값과 세 recovery 선택지의 개별 의미는 이해한다고 했지만 전체 Authoring 흐름은 거시적으로 복잡하다고 판정했다. 따라서 UA-04 USER PASS를 부여하지 않고 `USER UX Remediation Required`, P0-12 USER PASS 3을 유지한다.
- 다음은 Frozen source-decision/target-apply separation·fail-closed·no-auto-save를 유지하면서 사용자가 내부 단계를 직접 조립해 따라가는 느낌을 줄이는 macro-flow UX remediation이다.

### CF-FQ-029 재개 기준

1. LM-P0-01~05와 LM-P0-06A Technical Closure를 반복하지 않는다.
2. 다음 실제 Gate는 `LauncherMissilePlan.md v0.14.0 / LM-P0-06 Launcher Integration USER PIE`다.
3. MuzzleBlocked·AngledEjection·VerticalEjection·CarrierVelocityRatio·Direct 기본값 복구를 사용자가 직접 확인하기 전 PASS로 추정하지 않는다.
4. `FCFLauncherSequenceRuntime::RecordShotResult(false)`의 ContinueRemaining / StopSequence 기술 상태 전이는 이미 검증됐으므로 전용 결함이 없는 한 재구현하지 않는다.
5. 기존 Manual Cancel + Ammo Reservation/ActionLock cleanup도 `AMMO_P0_04.LauncherLock` 보호 회귀 PASS를 보존한다.
6. `CF-FQ-037 Scanner`는 `SCAN-P0-06 USER PIE PASS`와 `SCAN-P0-07 Current System Integration`을 완료해 Done이다. 현재 구현 owner는 `Document/Systems/Targeting/SensorContact.md v1.1.0`이며 기존 Plan은 Historical + Retained Path로 보존한다.


Current projection update — 2026-08-16: CF-FQ-029 LM-P0-06A를 기술적으로 닫았고, CF-FQ-029 전체 Done이나 LM-P0-06 USER PIE PASS로 확대하지 않았다. 현재는 사용자 선택 전 단일 Active를 비운다.

### 이미 보호하는 완료 증거

- `UI-P0-02` Runtime Lifetime USER PASS.
- UI-P0-03 USER PASS: 실제 Speed 증가, Weapon Cooldown `READY → 남은 초 → READY`, Target 선택·해제, Ammo·Ripple·Salvo Presentation, Defense Production Panel과 Pawn Rebind·Old Pawn Event Isolation.
- `CF-FQ-031` Ammo는 Done / Current System이며 Heavy·Ripple Ammo USER PIE를 반복하지 않는다.
- `CF-FQ-033` Defense는 Done / Current System이며 Shield·Armor·Integrity 공식을 UI 작업에서 변경하지 않는다.
- D1-11 Production Structure Gate는 PASS이며 D1-11-ART 세부 시각 폴리시·Art Import·DA 연결은 기능 개발 비차단 후순위다.

### 보호 범위

```text
CF-FQ-031 Ammo Current
CF-FQ-033 VehicleDefense / HitDamage Current
CF-FQ-026 TargetSelect TS-P0-08 Paused checkpoint / Remote Technical 3/3 + LOS Prefilter PASS / USER PIE Pending
CF-FQ-030 Missile Ready checkpoint / Persisted Asset Technical Verification PASS / CF-TC-027 Manual PIE Pending
CF-FQ-034 Fitting Paused checkpoint / FIT-P0-07A~07C Official Fixture + Quantitative Mobility Technical Complete / FIT-P0-07D USER Driving Feel Comparison 재개 / 기존 FFIT-P0-01~04 + FIT-P0-06 Technical PASS 보호
CF-FQ-035 Inventory Paused checkpoint / Remote Technical Complete / USER Field UI·Mobility Pending
CF-FQ-015 VehicleData Paused checkpoint / VD-P0-00~03 Remote Technical Done / VD-P0-04 USER Tuning Pending
승인 VehiclePanel / WeaponPanel / Reticle 의미 계약
기존 dirty source / assets / user work
```

---

## 3. Paused 작업

| Work ID | 상태 | 대표 Plan | 재개 지점 |
| --- | --- | --- | --- |
| `CF-FQ-034` | Paused | `Document/Plan/VehicleFittingPlan.md v0.17.0` | FIT-P0-07D USER Driving Feel Comparison / 공식 Light·Default·Heavy 정량 baseline 보존 |
| `CF-FQ-029` | Paused | `Document/Plan/LauncherMissilePlan.md v0.14.0` | LM-P0-06 USER PIE — MuzzleBlocked·Angled/Vertical Ejection·Carrier Velocity·Direct 기본값 복구 |
| `CF-FQ-038` | Paused | `Document/Plan/DataAuthoringPlan.md v0.2.31` | DAUTH-P0-12 USER Authoring Acceptance / USER PASS 3 / single-field baseline Apply 후 UA-04 External Drift 3-way Recovery |
| `CF-FQ-035` | Paused | `Document/Plan/InventoryFoundationPlan.md v0.9.0` | Remote Technical Complete / Field UI·Mobility USER 검증 |


| `CF-FQ-026` | Paused | `Document/Plan/TargetSelectPlan.md v0.12.5` | 동일 차량 인식 영역 → 7°/1200m 범위 체감 → debug Sphere → HUD 겹침 → 16:9/32:9 USER PIE |
| `CF-FQ-015` | Paused | `Document/Plan/VehicleDataTuningPlan.md v0.2.0` | VD-P0-04 USER Tuning / Sedan·SUV 실제 주행감 비교 |

Paused 작업의 상세 Build/Automation/PIE evidence는 각 대표 Plan이 소유한다.

---

## 4. Ready 기능은 ActiveWork에 복제하지 않음

현재 Ready 후보는 `Document/ProjectSSOT/03_FeatureQueue.md`가 소유한다.

```text
CF-FQ-030 Missile
```

사용자가 작업 전환을 선택하면 FeatureQueue와 해당 대표 Plan을 fresh read한 뒤 Active로 승격한다.

---

## 5. 완료 작업

완료 기능의 상세 검증 history는 ActiveWork에 유지하지 않는다.

현재 구현:

```text
Document/Systems/SystemIndex.md
```

완료 당시 Plan/Acceptance history:

```text
Document/Plan/Archive/README.md
```

대표적으로 `CF-FQ-036/037 SensorContact+Scanner`, `CF-FQ-008 WeaponData`, `CF-FQ-031 Ammo`, `CF-FQ-033 Vehicle Defense`, `CF-FQ-027/028 Projectile`, `CF-FQ-024/025/022/023/018/017`은 Systems 승격 후 Historical route로 관리한다.


---

## 6. 세션 복원 규칙

사용자가 `이전 작업 이어서 진행해`라고 요청하면:

```text
1. 이 문서에서 현재 Active 선택
2. FeatureQueue 상태 확인
3. 대표 Plan 최신 checkpoint 확인
4. 관련 Systems / 실제 코드·에셋 교차검증
5. 대표 Plan의 next gate에서 계속
```

특정 Work ID를 지정하면 해당 작업을 우선한다.
대표 Plan과 이 projection이 충돌하면 대표 Plan과 FeatureQueue를 기준으로 이 문서를 stale projection으로 교정한다.

---

## 7. Changelog / Migration

### v3.31 - 2026-08-19

- `CF-FQ-032 / UI-P0-06` 범위를 중간점검해 완료 조건을 원래 Roadmap 계약에 맞게 교정했다. UI-P0-06은 **현재 실제 Runtime이 존재하는 차량/무기 채널을 Production Presenter에 연결하고, Provider 없는 채널은 추정 없이 Unavailable/Collapsed로 유지**하면 Technical Complete다. 따라서 별도 shared-power Gameplay System이 아직 없는 VehicleBattery를 UI-P0-06 안에서 새로 구현하는 것은 완료조건이 아니다.
- VehicleBattery는 future Gameplay feature dependency로 분리했다. 현재 UI의 `VehicleBatteryAvailability=Unavailable`과 ResourceChannel 미생성이 올바른 상태이며, 향후 Battery owner/capacity/regen/consumer priority/Weapon·Shield·Scanner 소비 계약이 별도 Gameplay 기능으로 구현되면 기존 ResourceChannel 경계에 additive 연결한다. WeaponCharge를 Battery로 재해석하지 않는다.
- UI-P0-06을 current-runtime 기준 `Technical Complete`로 닫고 formal current Gate를 `UI-P0-07 Target Knowledge`로 전진시켰다. Rail USER Visual, saved Heat/Charge tuning USER Visual, authoritative Redline authoring/RPM USER Visual은 content-dependent Deferred follow-up으로 유지하며 UI-P0-06 Technical Complete를 차단하지 않는다. artificial 2무기 fixture 금지와 임의 tuning/Redline authoring 금지는 유지한다.
- `CFHUDDataProvider.h v1.7.0`, `CFHUDPresenter.h v1.15.0`의 stale Current 주석을 교정해 WeaponCharge/Heat actual Runtime 존재와 VehicleBattery만 future dependency라는 현재 사실을 반영했다. 실행 코드 변경은 0이다.

Migration: 다음 세션은 UI-P0-06의 Battery 구현을 시작하지 않는다. 완료된 Stage A/B·RPM·Heat·Weapon Selection·Rail·Input·WeaponCharge evidence는 관련 결함 없이 반복하지 않고 `UI-P0-07 Target Knowledge`부터 착수한다. VehicleBattery가 실제 Gameplay feature로 기획될 때는 UI 하위 slice가 아니라 shared-power owner/consumer 계약을 가진 별도 Gameplay 범위로 설계한 뒤 HUD에 연결한다.

### v3.30 - 2026-08-19

- `CF-FQ-032 / UI-P0-06 WeaponCharge` slice를 Technical PASS로 closure했다. VehicleBattery는 shared vehicle power owner·소비 우선순위가 필요한 별도 큰 Runtime으로 분리하고, WeaponCharge만 기존 `WeaponData → WeaponComp → Pawn Fire → HUD` 경계 안에서 구현했다.
- `CFWeaponChargeRuntime v1.0.0`과 `UCFWeaponData v1.13.0`의 explicit `Maximum/Initial/PerShot/Recovery`를 추가했다. 네 값 all-zero인 기존 Asset은 Charge Disabled이며 기존 발사를 제한하지 않는다. 활성 Charge는 `UCFVehicleWeaponComp v1.25.0`이 선택 무기별로 독립 보존하고 비선택 포함 Game-Time으로 회복한다. 다음 표준 한 발 소비량보다 부족하면 Pawn이 `WeaponChargeInsufficient`로 차단하고 실제 accepted result에서 정확히 1회 소비한다.
- HUD는 `UCFHUDDataProvider v1.13.0 → FCFWeaponHUDData v1.12.0 → ResourceChannels::WeaponCharge → UCFHUDPresenter v1.16.0`으로 actual Current/Maximum/Ratio/Insufficient만 전달한다. Ammo/Launcher가 없으면 `CHARGE N%` Primary, 기존 Primary가 있으면 Secondary이며 Secondary 최대2를 유지한다. FireState는 `Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY`다. VehicleBattery·정적 설정·Cooldown fallback은 0이다.
- final Official Build `2d9f33261d3c427187f75338b8f37f1d` Exit 0 PASS, exact `WeaponChargeRuntimeResourceContract` process `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS / Failure0 / Engine Exit0, Result SHA-256 `df53c9732b3135f9eae230dc9f287d646e542179c0464e5b2ef1794b54e8e137`를 closure evidence로 고정한다. 테스트는 pure Charge 상태, 실제 WeaponComp 소비·회복, HUD Channel, Compact Primary/Secondary와 NoAmmo/NoCharge/Overheated 우선순위를 검증했다.
- closure source readback에서 Pawn의 Charge precheck `CanActiveWeaponAcceptChargeShot → WeaponChargeInsufficient`가 기존 Heat precheck 바로 다음에, accepted-shot `RecordAcceptedWeaponShotCharge`가 기존 Heat accepted hook 바로 다음에 동일 `ValidateFireCommandInternal / ApplyFireResultInternal` 경계로 연결된 것을 확인했다. 해당 Heat fire path는 기존 Heat exact에서 이미 protected friend 기반 동적 검증된 경계이고 Charge 상태 자체는 신규 exact에서 실제 WeaponComp로 실행 검증됐으므로, 제품 protected API 노출이나 Charge 전용 중복 friend test는 추가하지 않았다.
- Production/DataAsset Charge 값 authoring, VehicleBattery, Heat tuning, Redline authoring, Rail USER Visual 상태 변경은 0이다. Rail USER Visual은 representative persisted multi-weapon content가 실제 생길 때까지 Deferred이고 artificial 2무기 fixture 금지를 유지한다.

Migration: WeaponCharge P0 Runtime/HUD Technical PASS는 관련 결함 없이 반복하지 않는다. 기존 WeaponData all-zero Charge는 Disabled 호환 상태이며 실제 Charge 무기를 만들 때만 네 explicit authored 값을 함께 결정하고 별도 USER Visual을 검토한다. 다음 미구현 Gameplay Runtime은 VehicleBattery 하나이며, shared-power owner/consumer 우선순위를 먼저 설계하지 않고 WeaponCharge 값을 Battery로 재해석하지 않는다.

### v3.29 - 2026-08-19

- UA-03 B2 결과인 `DA_Profile_UA_TestHandling` 저장을 live dirty=false와 fresh AssetDump persisted `FrontWheelMaxBrakeTorque=2000`, `AuthoringRevision=1`로 확인했다.
- current `DA_Recipe_TestSedan`의 AppliedState는 아직 empty라 UA-04 External Drift 판정 기준점이 없다. 다음은 single-field baseline Apply를 만든 뒤 같은 field raw edit로 3-way USER 검사를 진행한다.

### v3.28 - 2026-08-19

- `CF-FQ-038 / DAUTH-P0-12 UA-03`을 USER PASS로 닫았다. transient affected Recipe inventory 교정 후 final official build `3fa7b4d4cca14d37b7fea50631e4fa07` Exit 0, focused `SharedProfileImpact` process `db57ea0701234e65b8e5921b1dce1e96` 1/1 PASS를 확보했다.
- fresh persisted readback에서 `DA_Recipe_TestSedan`의 Handling=`DA_Profile_UA_TestHandling`, DriveState=`DA_Profile_UA_TestDriveState`, `DriveStateMode=VehicleSpecific`을 확인했다. USER B2 one-shot은 `FrontWheelMaxBrakeTorque=2000`, affected Vehicle1, expected VehicleData change0, Auto Apply0, Auto Save0으로 preview와 actual commit을 PASS했다.
- commit 직후 live dirty 경계는 Handling Profile=true / Recipe=false / VehicleData=false / DriveState Profile=false다. transient Recipe 재등장 0, Frozen B2 validation 완화 0이다.
- P0-12 USER PASS를 3으로 전진했고 다음 bounded Gate는 `DA_Profile_UA_TestHandling` exact 1 asset 저장 후 `UA-04 External Drift 3-way Recovery`다. P0-08~11과 UA-01~03은 새 실패 근거 없이 반복하지 않는다.

Migration: UA-03을 재실행하지 않는다. 현재 user-interacted Editor lifetime의 Handling Profile dirty 상태를 임의 자동 저장/폐기하지 않고 사용자가 exact asset 저장한 뒤 UA-04를 시작한다.

### v3.26 - 2026-08-19

- UI-P0-06 Player-facing Weapon Select Input을 Technical PASS로 닫았다. current `IMC_Vehicle_Default`의 기존 47 mapping을 보존하고 새 `IA_SelectWeapon` Axis1D에 숫자키 1~9를 각각 실제 `SelectableWeapons` 1-based ordinal 1~9로 직접 매핑했다. Pawn handler는 ordinal 정수값만 `RequestSelectWeaponIndex(Ordinal-1)`에 전달하며 새 WeaponGroupId/cycle cursor를 만들지 않는다. Mouse Wheel은 Radar Range/Zoom 예약을 보존하고 게임패드 mapping은 임의 지정하지 않았다.
- final Input Build `dec0757b745342b4b90ff5f17b761eb0` PASS, `WeaponSelectInputAssetSetup` process `189ac8704dea46c8bebf5b970d94a122` 1/1 PASS, persisted readback에서 `IA_SelectWeapon` fp `1A8A0402` Axis1D/ConsumeInput=true, `IMC_Vehicle_Default` fp `52D2D868`, mapping 47→56과 숫자 1~9 ordinal/MouseWheel0을 확인했다. exact read-only `WeaponSelectInputContract` process `daeaf649669d48c79746a65cc44b1b89` 1/1 PASS / Result SHA-256 `f61e41f8185ebe9053bad188c476277f0e0ddeb83aab048f0977f92bd18345dd`다.
- Rail USER Visual Gate용 persisted representative를 fresh 감사했지만 `DA_TestSedan`, `DA_TestSUV`, Ammo Heavy/Ripple/Salvo Fitting은 모두 실제 선택 무기 1개라 1↔2 Rail swap USER 검증이 불가능했다. 기존 Production HeavyCannon/RocketLauncher는 `WeaponMassKg=0`으로 fixture Snapshot이 `MISSING_MASS_SOURCE` fail-closed됐고, mass-valid test-only `Preset_HeavyFinite`/`Preset_RippleFinite` 재사용 가능성은 확인했으나 두 finite AmmoId의 출격 loadout까지 별도 작성해야 해 fixture scope가 확대됐다.
- 사용자 지시에 따라 fixture 확대를 중단했다. 실패 실행에서 생성된 test Vehicle/Fitting은 cleanup error0으로 삭제됐고 persisted 2무기 fixture는 0이다. Rail USER Visual은 **representative persisted multi-weapon content가 실제 생길 때까지 Deferred**하며 Technical PASS나 USER PASS로 추정하지 않는다. 기존 Production WeaponData·Ammo/Fitting·Battery/Charge·Heat tuning·Redline authoring 변경은 0이다.

Migration: Weapon Select Input Technical PASS는 관련 결함 없이 반복하지 않는다. 숫자 1~9 direct ordinal 계약과 Mouse Wheel Radar 예약을 유지한다. Rail USER Visual을 다시 열 조건은 실제 persisted multi-weapon Vehicle/Fitting이 생기는 것이며, 그 전에는 전용 artificial fixture를 추가 확장하지 않는다. 실제 content가 생기면 `1↔2` 선택 시 Header/Rail swap, 범위 밖 숫자 no-change, Mouse Wheel no weapon change, 내부 ID 노출0만 USER Gate로 확인한다.

### v3.25 - 2026-08-19

- `CF-FQ-032 / UI-P0-06` Production WeaponRail을 fresh persisted audit했다. pre-apply saved fingerprint `4A873878`에서 `HorizontalBox_WeaponRail → Image_WeaponRail1/2/3`이 각각 Turret/Ammo/Reload 의미 placeholder이고 Rail은 Collapsed, binding/animation/input 0임을 확인했다. 실제 Weapon Selection ViewData는 0-N `SelectableWeapons + SelectedWeaponIndex + DisplayName`이므로 기존 3 Image를 weapon slot으로 재해석하지 않았다.
- `UCFHUDPresenter v1.14.0/v1.15.0`과 `CFUIHUDProdEditorBridge v1.5.0/v1.8.0`으로 truthful Text Rail을 구현했다. 현재 선택 무기는 기존 Header/Selected Card에만 남고 Rail에는 비선택만 Provider fixed order로 표시한다. 비선택 0은 Rail Collapsed, 1~3은 원본 1-based 순번 + 실제 `EquipmentPresetData.DisplayName`, 4+는 앞 2개 + `+N`이다. 이름 부재는 내부 ID fallback 없이 일반 `WEAPON`만 사용한다. 실제 weapon icon source와 비선택 weapon별 Resource Summary source가 없으므로 fake icon/summary는 만들지 않았다.
- Production Bridge는 old `Image_WeaponRail1/2/3`을 제거하고 `SizeBox_WeaponRail1~3` 112x68 + `Text_WeaponRail1~3` + 8px gap을 생성한다. Designer default는 Rail/Tile 모두 Collapsed이며 Runtime Presenter만 실제 비선택 selection에서 연다. Panel surface/Compact Resource 구조와 Gameplay cast/binding0 계약은 유지했다.
- Official Build `941ea18597b3483594be8de3317e68fd` Exit 0 PASS. 기존 targeted `Tools/RunUIHUDProduction.ps1 -WeaponPanelOnly` process `0617f7cbed97427a942c7a6f59936e22`가 `/Game/CarFight/UI/HUD/Panels/WBP_CFWeaponPanel` 정확히 1개만 rebuild/compile/save했고 다른 Production Asset mutation은 0, Report SHA-256은 `6356715d0ab2503c78c6e31aec31f3b494b6f6e1405958481c7ed9659e06baf1`이다.
- post-apply fresh AssetDump `adset_v1_1eab789eececf0f2a4654058a9470eea.111e1855e4b1bd6ebb43b955`에서 saved fingerprint `26CFB205`, node30, Rail Text Tile3 존재, old Rail Image3 부재, Rail/Tile Collapsed, binding/animation/input0을 persisted evidence로 확인했다. exact `WeaponRailVisualContract` process `7cc9ddd5b2204725bb3ec45c200d649b`는 selected exclusion/selection swap/name fallback/5-weapon `+2`/single-weapon collapse를 실제 Production Widget에 적용해 1/1 PASS, Result SHA-256 `83eb3d88d23f2c5baa06b3c326fbef30535048edd38d89b2e18768de2fa958c6`다.
- Weapon Selection Runtime/HUD source, Heat/RPM/Stage A+B/기존 USER PASS는 반복하지 않았다. Battery/Charge Runtime, Heat tuning, Redline authoring은 변경 0이다. 이번 slice의 persisted Asset mutation은 WeaponPanel 정확히 1개이며 USER Visual PASS는 추가하지 않았다.

Migration: truthful Weapon Rail Visual Consumer Technical PASS는 관련 결함 없이 반복하지 않는다. 다음 UI-P0-06 Weapon Selection 범위는 실제 입력 매핑과 USER Visual이다. 정식 per-weapon icon source가 별도 authoring contract로 생기기 전 현재 name-only Rail을 유지하고 Turret/Ammo/Reload semantic icon이나 내부 ID를 weapon identity로 재사용하지 않는다. 비선택 무기별 resource summary도 실제 per-item Runtime/ViewData source 전에는 만들지 않는다.

### v3.24 - 2026-08-18

- UI-P0-06 Player-facing WeaponGroup fresh audit에서 실제 authoritative 집합 source가 `UCFVehicleFittingComp`의 Applied `FCFVehicleFittingSnapshot::ResolvedMounts`에 이미 존재함을 확인했다. Weapon-bearing mount의 결정론적 순서를 Player-facing fixed display order로 사용하며 새 Group ID를 만들지 않는다. 내부 `MountProfileId`는 FireOrigin/Ammo identity로만 유지하고 HUD에는 전달하지 않는다.
- `CFWeaponSelectTypes.h`, `UCFVehicleFittingComp v1.6.0`, `UCFVehicleWeaponComp v1.24.0`, `ACFVehiclePawn v2.151.0`, `CFHUDViewData.h v1.11.0`, `UCFHUDDataProvider v1.12.0`으로 실제 선택 목록·SelectedWeaponIndex·DisplayName-only HUD source와 안전 전환을 구현했다. 각 무기의 Cooldown/Heat는 독립 보존하고 비선택 Heat도 자연 냉각한다. Launcher active 중 전환은 기존 `WeaponChanged` cancel 계약으로 예약/Action Lock을 정리한 뒤 전환한다.
- Production `WeaponRail`의 기존 세 Image는 실제 weapon slot이 아니라 Turret/Ammo/Reload 의미 아이콘이므로 이번 Runtime slice에서 Rail을 열거나 아이콘을 가짜 weapon slot으로 재해석하지 않았다. Rail은 계속 Collapsed이며 실제 Visual Consumer는 별도 후속 slice다. 입력 Action/키도 이번 slice에서 추가하지 않았다.
- 첫 source build `f6362c6f8d9f443cbda2ef3e707af13d`은 수정 TU compile까지 PASS 후 당시 실행 Editor의 NetCore DLL lock으로 Link만 LNK1104 종료했다. lock 해제 후 final Official Build `62ffb62f69524bb18c3a9f11bb9f58f1` Exit 0 PASS, exact `WeaponSelectionRuntimeContract` process `ff23cf6ac04e4b2c8cf0084b24173cdb` 1/1 PASS / Failure 0, Result SHA-256 `0ffa955f626d0de3f5ebf7b2ac594d7f8b039d268fe6efb2ab0730abfe77618c`를 확보했다.
- Heat/RPM/Stage A+B/기존 USER PASS는 재실행하지 않았고 Content·Blueprint·Input·DataAsset mutation은 0이다. saved Heat tuning과 `RedlineStartRPM=0`도 변경하지 않았다. 실제 미구현 Gameplay Runtime은 이제 VehicleBattery·WeaponCharge다.

Migration: Weapon Selection Runtime Technical PASS는 관련 결함 없이 반복하지 않는다. 다음 Weapon UI slice는 persisted Production Rail을 fresh 감사해 실제 Weapon Selection ViewData를 truthful하게 소비하는 Visual Consumer를 설계한다. 현재 3개 의미 아이콘을 무기 슬롯으로 재사용하지 말고, 실제 weapon icon source가 없다는 사실을 보존한다. Battery/Charge는 별도 Gameplay Runtime 전 추정하지 않는다.

### v3.23 - 2026-08-18

- `CF-FQ-032 / UI-P0-06` 남은 WeaponGroup·VehicleBattery·WeaponCharge·Heat fresh Runtime audit에서 WeaponGroup은 player-facing 목록/선택 Index가 없고 내부 MountProfile/WeaponGroupId만 존재, VehicleBattery는 HUD placeholder와 장기 설계만 있고 공용 전력 Runtime 없음, WeaponCharge는 독립 자원 설계만 있고 현재값·소비·회복 Runtime/정적 입력 없음으로 분리했다. Heat는 기존 `HeatPerShot / MaxHeat`와 P0 설계가 있어 가장 독립적인 다음 slice로 선정했다.
- `HeatDissipationPerSecond` 기본 0과 `FCFWeaponHeatRuntime`을 추가했다. 세 Heat 입력이 모두 유한한 양수일 때만 활성화하며 CurrentHeat·자연 냉각·Overheated는 `UCFVehicleWeaponComp`가 소유한다. MaxHeat 도달 뒤 임의 회복 percentage 대신 `max(MaxHeat - HeatPerShot, 0)`까지 냉각되면 다음 표준 한 발을 허용한다.
- `ACFVehiclePawn`은 과열을 `WeaponOverheated`로 발사 검증하고 실제 승인된 각 발사 결과에서 Heat를 정확히 1회 누적한다. HUD는 실제 WeaponComp Heat만 `ResourceChannels::Heat` Percent로 전달하고 Compact Projection은 Heat Secondary, FireState는 `Reload > NoAmmo > Overheated > Cooldown/READY`를 사용한다. Launcher Active에서는 기존 Primary + Ammo/Heat Secondary 최대 2와 단일 lifecycle을 유지한다.
- 첫 Heat Build `a52dc87510e04b44a71bd14220aca82d`는 지원되지 않는 UHT `Units=/s` metadata 한 건으로 FAIL했고 Heat 규칙 변경 없이 제거했다. 최종 Official Build `0ecfed49ab3a4f41b349fc707c44e5f2` Exit 0 PASS, exact `HeatRuntimeResourceContract` `c736d1a6d6134a798a4b84452750e3d6` 1/1 PASS / Failure 0, Result SHA-256 `1abf0dc7a4788d6543c7933abef38433849ae57d569229cfab220f14937abad5`다. transient Pawn의 `ExecuteAcceptedFireCommand → ApplyFireResult` 4발 Heat 누적과 WeaponComp Tick 냉각까지 실행 검증했다.
- Content/Blueprint/DataAsset mutation은 0이다. 기존 saved WeaponData는 새 `HeatDissipationPerSecond=0` 호환 기본값으로 Heat가 Disabled이므로 현재 발사 동작을 자동 변경하지 않는다. Heat tuning/USER Visual은 Pending이며 UI-P0-06 전체 PASS로 확대하지 않는다. RPM, Stage A/B, UI-P0-03~05 완료 evidence는 반복하지 않았다.

Migration: Heat Runtime Technical PASS는 관련 결함 없이 반복하지 않는다. 실제 Heat를 사용할 무기는 후속 명시 tuning에서 세 값을 함께 authoring하고 USER Visual을 별도로 확인한다. 현재 남은 미구현 Runtime은 player-facing WeaponGroup, VehicleBattery, WeaponCharge이며 내부 ID나 다른 설정값으로 추정하지 않는다. `RedlineStartRPM=0`도 계속 유지한다.

### v3.22 - 2026-08-18

- `CF-FQ-032 / UI-P0-06 RPM Gauge Production Visual Binding`을 Technical PASS로 닫았다. 기존 저장 `WBP_CFSpeedGauge`의 `ProgressBar_RPMTick00~20` 21개를 구조 변경 없이 Runtime Percent sink로 사용하고 explicit Redline/Maximum mapping 결과만 적용한다.
- Redline이 0/Unavailable/invalid이면 `EngineMaxRPM` 비율 fallback을 만들지 않고 21 Tick fill을 모두 0으로 reset해 이전 Pawn/차량의 RPM 표시를 붙잡지 않는다. persisted representative VehicleData는 `DA_TestSedan` Idle 900 / Max 6500, `DA_TestSUV`와 `DA_VehicleDefense_TestSUV` Idle 900 / Max 6020이며 authoritative Redline은 없으므로 Asset 값은 0 Unconfigured로 유지했다.
- 직전 Build `8c90ab8f8d7e430b9510bc12df5974ae`는 신규 test helper 인자 `TestName`의 C4458 이름 충돌 1건만 FAIL했고 Production Presenter/Bridge compile은 PASS했다. 인자를 `TickPercentTestLabel`로 교정한 final Official Build `5ff6d404dfa44c61a85e698e27052db5`는 Exit 0 PASS했다.
- 신규 exact `CarFight.UI.UI_P0_06.RpmGaugeVisualBindingContract` process `7d32d08b7e554e9991fb64e80d472d78`는 1/1 PASS / Failure 0 / Engine Exit 0이다. Result JSON SHA-256은 `0054abd391216065f732e274b00bc054478c5ab168608523504b614c860fc6d4`다. Production Asset mutation과 VehicleData Redline authoring은 0이며 Stage A/B와 UI-P0-03~05 USER PASS는 반복하지 않았다.

Migration: RPM Visual Binding은 관련 결함이 없는 한 다시 열지 않는다. authoritative 차량 설계값이 생기기 전 `RedlineStartRPM=0`을 유지하고, RPM-specific 다음 Gate는 명시 Redline authoring 결정 후 USER Visual이다. 그와 별개로 UI-P0-06의 WeaponGroup·VehicleBattery·WeaponCharge·Heat Runtime Pending을 계속 보존한다.

### v3.21 - 2026-08-18

- `CF-FQ-032 / UI-P0-06 RPM Gauge explicit Redline upstream contract`를 Technical PASS로 닫았다. `EngineMaxRPM`은 Chaos `EngineSetup.MaxRPM` 물리 상한으로 유지하고 `RedlineStartRPM`을 별도 authored field로 추가했으며 `ChangeUpRPM`은 Current CarFight Source에 존재하지 않음을 확인했다.
- `RedlineStartRPM=0`은 기존 VehicleData 호환을 위한 미설정 상태다. 이 경우 HUD Redline은 `Unavailable`이며 `EngineMaxRPM`, Idle RPM 또는 변속값에서 보정·추정하지 않는다. 명시값만 `EngineIdleRPM < RedlineStartRPM < EngineMaxRPM`을 Validator가 강제한다.
- HUD source는 Current RPM=`UChaosWheeledVehicleMovementComponent::GetEngineRotationSpeed()`, Redline/Maximum=`VehiclePawn.VehicleData`로 분리했다. Presenter는 실제 Redline을 0.85, EngineMaxRPM을 1.0 화면 위치로 piecewise mapping한다. Production SpeedGauge Asset binding은 이번 slice에서 변경하지 않았다.
- Official Build `745dba430bcc47c2925c841a0c5a6686` Exit 0 PASS. 신규 `RpmGaugePresentationContract` process `f805050484af47b8a2653cbb8ae2f639` 1/1 PASS, `RpmGaugeRuntimeSourceContract` process `af1d9d010c2c433382819698a6dc7eeb` 1/1 PASS다. Prefix runner `cd6fe78564094981b1c10e6f6b8df75f`는 Unreal 내부 2건 Success였으나 exact-path 결과 집계 방식 때문에 wrapper만 Exit 1이었고 최종 증거는 두 exact run으로 대체했다.
- `UCFVehicleData` additive leaf로 Current Authoring schema가 117→118이 됐다. P0-08 당시 117 baseline evidence는 Historical로 보존하고, bounded compatibility에서 Registry `dacb88cb09644d0287cfe7a7f3137b51`, Batch resolved projection `03aad228b1784f1b908ed6e82ddced3e`, Allowlist `c9d07c42428642c186743e8c26043dcb`가 각각 1/1 PASS했다. Current Performance numeric은 18, 5 Profile numeric total은 79다.
- `CF-FQ-038` USER Acceptance는 재개하지 않았고 Paused / USER PASS 2 / UA-03 DriveState RequiredProfileMissing 체크포인트를 그대로 보존한다. Stage A 6/6, Stage B, UI-P0-03~05 USER PASS도 반복하지 않았다.

Migration: 다음 UI-P0-06 RPM 작업은 upstream 계약을 다시 열지 않는다. 실제 차량의 `RedlineStartRPM`을 임의 생성하지 말고 대표 VehicleData의 현재 Idle/Max와 차량 설계 의도를 확인한 뒤 명시 authoring한다. 그 후 Production SpeedGauge visual binding과 USER Visual을 별도 Gate로 검증한다.

### v3.20 - 2026-08-18

- `CF-FQ-032 / UI-P0-06 Dynamic Resource Visual Stage B`를 Technical PASS로 닫았다. 사용자 종료 뒤 final Official Build `595dd89ff8044c5cbb4670c3cac12469`이 Exit 0 PASS했고, 신규 Stage B test expectation 정합화 후 최종 Official Build `0bc2f086d71b4a57933326757d88b50d`도 Exit 0 PASS했다.
- targeted `WeaponPanel-only` Apply `b600bd339b0348b5a3001c01a541484a`는 existing `/Game/CarFight/UI/HUD/Panels/WBP_CFWeaponPanel` 정확히 1개만 rebuilt/compiled/saved했고 다른 Production Asset mutation은 0이었다. Report SHA-256은 `6356715d0ab2503c78c6e31aec31f3b494b6f6e1405958481c7ed9659e06baf1`이다.
- fresh AssetDump dataset `adset_v1_9ca90ab2e764ee02cda305b45e91bb3b.23e84f05ff5481cd492c956f`에서 저장 WeaponPanel이 Header Reserve + `VerticalBox_ResourcePresentation` + Primary 1 + Secondary A/B + FireState 1 + Collapsed WeaponRail 구조로 전환됐고 구형 Launcher/Ammo/Heat/Cooldown 전용 Row가 사라졌음을 persisted evidence로 확인했다. Asset fingerprint는 `CFB0652D`다.
- 첫 신규 Stage B Automation `ccd0ce6a60f449eb88929ccbdc275d6b`은 실제 기능 값이 아니라 `Visible` vs Presenter 공통 `HitTestInvisible` expectation 5건 때문에 FAIL했다. Production 구현은 변경하지 않고 `CFHUDDataTests.cpp v1.14.0`에서 expectation만 교정했다.
- 최종 `CarFight.UI.UI_P0_06.ResourceVisualSlotContract` process `2dc5eb2a394b4053875f548cec2204bc`는 1/1 PASS / Failure 0, Result SHA-256 `78bef76e45eb93199d1e7bba0bed6d0a91802c513568bcab78568e77508f6611`이다. Stage A focused 6/6은 반복하지 않았다.
- `RedlineStartRPM`, Player-facing `WeaponGroup`, `VehicleBattery`, `WeaponCharge`, 현재 `Heat` Runtime은 계속 upstream Pending이며 Stage B PASS를 UI-P0-06 전체 PASS나 새 USER Visual PASS로 확대하지 않는다.

Migration: 다음 UI-P0-06 작업은 Dynamic Resource Visual Stage B를 다시 열지 않고 남은 upstream 계약 중 실제 착수 가능한 항목을 선택한다. 기존 Ammo/Reload/Cooldown/Launcher USER PASS와 LauncherSequenceRevision lifecycle은 보호한다. `CF-FQ-038` Paused 체크포인트는 v3.19의 최신 DAUTH 상태를 유지한다.

### v3.19 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-03` USER follow-up을 문서 checkpoint로 정리했다. 실제 Recipe identity는 `DA_Recipe_TestSedan`이며 Profile Binding remediation의 0-profile fail-closed, `DA_Profile_UA_TestHandling` discovery, Recipe-only binding review와 actual binding을 사용자 화면으로 PASS했다.
- Handling `Profile.Handling.FrontWheelMaxBrakeTorque=2000` B2 preview는 affected Vehicle 1 / 예상 VehicleData 변경 0 / Auto Apply 0 / Auto Save 0을 명확히 표시해 Affected Vehicle Impact Preview USER PASS다.
- actual B2 commit은 `Shared Profile prospective validation이 commit 직전 Block됐습니다`로 mutation0 종료했고 `DA_TestSedan` dirty=false를 live read로 확인했다. `2000` source value는 commit되지 않았다.
- Source/Frozen 26.49 감사에서 `DriveStateMode=VehicleSpecific`인데 DriveState Profile Snapshot이 없어 발생한 `RequiredProfileMissing`이 B2 prospective validation blocker임을 확인했다. 이는 정상 fail-closed이므로 Core gate를 약화하지 않는다.
- 다음 PC Gate는 `DA_Profile_UA_TestDriveState` 생성 → Recipe `주행 상태` binding → `적용 차단 1` 해소 → 동일 Handling B2 actual commit 재검사다. UA-03 전체 USER PASS와 USER PASS count 증가는 보류한다.
- PC 종료 전 `DA_Recipe_TestSedan`과 `DA_Profile_UA_TestHandling` 두 Authoring Asset만 저장하고 일반 종료하도록 안내했으며 실제 save/close 완료는 별도 확인 전 추정하지 않는다. 전역 단일 Active `CF-FQ-032`와 UI-P0-06 Stage B 상태는 변경하지 않았다.

Migration: DAUTH 재개 시 `DataAuthoringPlan.md v0.2.27 / DataAuthoringRoadmap.md v0.1.35`에서 DriveState blocker 해소부터 시작한다. UA-01~02, Initial Import, Profile Binding remediation 기술/USER recheck와 현재 Handling impact preview를 반복하지 않는다.

### v3.18 - 2026-08-18

- `CF-FQ-032 / UI-P0-06 Dynamic Resource Visual Stage B` Source를 적용했다. Production Presenter는 Resource Visual 갱신 시 `BuildWeaponResourceEntries()`를 ViewData 적용당 정확히 1회만 호출하고 Header Reserve는 별도 owner로 유지한다. 기존 Launcher/Ammo/Heat/Cooldown 고정 Row 직접 적용은 제거했다.
- Production Editor Bridge를 `Primary 1 + Secondary 최대 2 + FireState 1` 의미 슬롯 구조로 전환하고 Validator가 구형 Launcher/Ammo/Heat/Cooldown Row 잔존을 거부하도록 보강했다. Player-facing WeaponGroup Runtime 전 Rail은 기본 Collapsed이며 Battery/Charge/Heat 가짜 Row는 만들지 않는다.
- 저장 Asset 적용 전 fresh AssetDump에서 현재 `WBP_CFWeaponPanel`이 Header + Launcher/Ammo/Heat/Cooldown 고정 Row + Rail 구조임을 재확인했다. Stage B Production Asset mutation은 아직 0이다.
- `CFHUDDataTests.cpp v1.13.0`에 `ResourceVisualSlotContract`를 추가해 실제 Production HUD에서 Ammo/Cooldown → Launcher Active → terminal 첫 적용 1회 → 같은 terminal ViewData 다음 적용 시 Ammo/Cooldown 복귀와 legacy Row 부재를 검증하도록 했다. Build `4f7a093c107d42a1a94672a0b92b2bb4`에서 해당 Test TU compile은 PASS했다.
- 최종 DLL Link는 현재 ownership-unknown `UnrealEditor.exe`가 `UnrealEditor-CarFight_Re.dll`을 점유해 `LNK1104`, Exit 6으로 차단됐다. 자동 종료는 하지 않았으며 새 DLL 링크 전 구버전 Bridge로 Asset을 수정하지 않는다.
- 기존 전체 10 Asset Production Apply를 재사용하지 않고 `ApplyUIHUDProduction.py v1.2.0 / RunUIHUDProduction.ps1 v1.1.0`에 기존 `WBP_CFWeaponPanel` 정확히 1개만 Build→Compile→Validate→Save하는 `WeaponPanel-only` targeted mode를 추가했다.

Migration: Editor가 안전하게 종료되면 Stage A 6/6을 별도 재연하지 않고 Stage B final Official Build부터 재개한다. Build PASS 뒤 targeted WeaponPanel-only Apply → fresh AssetDump persisted readback → 신규 `ResourceVisualSlotContract` 중심 Stage B 회귀를 진행한다. `CF-FQ-038`의 최신 `DataAuthoringPlan.md v0.2.26 / DataAuthoringRoadmap.md v0.1.34` Paused checkpoint는 변경하지 않는다.

### v3.17 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-03`를 bounded follow-up으로 재개해 `DA_Recipe_UA_TestSedan` 생성 성공, Recipe dirty=true, Target `DA_TestSedan` dirty=false와 Browser `관리됨` 전환을 사용자 화면/live evidence로 확인했다. Initial Import는 재실행하지 않았다.
- 첫 Recipe/Shared Profile USER 화면에서 5 Profile binding이 비어 있는데 existing Profile 선택/명시적 Recipe binding UI가 없어 UA-03 Shared Profile subflow를 FAIL로 유지했다. 실제 Authoring Shared Profile Asset도 현재 0개다.
- existing `ListProfiles` + `BindVehicleProfile` typed R1 contract를 재사용해 5 Domain Profile 후보/current binding/explicit Recipe-only bind/Open Profile UI를 보강했다. VehicleData direct mutation, Shared Profile direct writer, Auto Apply/Save/Retry는 추가하지 않았다.
- first remediation Build `eb2eb56867ba4557ad325fc04cf4f7a4`의 missing forward declaration compile FAIL을 교정했고 final Official Build `74d818f4c7a149b89d730c2a615912c2` Exit 0 PASS, focused `SharedProfileImpact` process `c81052d602ae4f869d6aecb431b28d6c` 1/1 PASS를 확보했다.
- USER PASS는 2 유지한다. 현재 user-owned Editor의 unsaved `DA_Recipe_UA_TestSedan`을 Browser가 임의 저장/폐기/재시작하지 않았으며 다음은 Recipe 보존에 대한 사용자 명시 동작 후 fresh Editor UA-03 recheck다. 전역 단일 Active `CF-FQ-032` 상태는 변경하지 않았다.

Migration: UA-01~02, Initial Import actual, P0-11 전체 Automation을 반복하지 않는다. 다음 DAUTH USER follow-up은 `DataAuthoringPlan.md v0.2.26 / DataAuthoringRoadmap.md v0.1.34`에서 fresh binary의 Profile binding → Shared Profile change review → affected Vehicle impact만 확인한다.

### v3.16 - 2026-08-18

- `UI-P0-06 Dynamic Resource Visual Stage A`를 Presenter-only Source로 구현했다. raw `ResourceChannels` 배열을 Visual Row로 직접 렌더링하지 않고 `FCFWeaponResourcePresentationEntry`와 `BuildWeaponResourceEntries()`가 Compact 역할을 `Primary 최대 1 + Secondary 최대 2 + FireState 최대 1`로 Projection한다.
- 기존 `ResolveAmmoPresentation / ResolveReserveAmmoPresentation / ResolveWeaponStatusPresentation / ResolveLauncherSequenceDisplay`와 `LauncherSequenceRevision` lifecycle을 재사용했다. Launcher Active/terminal Snapshot은 Primary, 해당 시 Ammo는 Secondary, 일반 상태는 기존 단일 FireState로 유지한다. ReserveAmmo는 기존 Header 우측 owner에 남겨 Presentation Entry에 넣지 않는다.
- VehicleBattery·WeaponCharge·Heat는 실제 Runtime Provider가 없는 상태를 그대로 보존해 Entry를 생성하지 않았고 Production `WBP_CFWeaponPanel`/Content Asset/Editor Bridge 변경은 0이다.
- 첫 Build `7588e3170de34cecadba55ad3f804433`은 Presenter/Test compile까지 PASS 후 ownership-unknown Editor DLL lock으로 `LNK1104` 종료했다. 사용자가 Editor를 안전하게 종료한 뒤 final Official Build `4c043a73e3054baab5f725b18cd7de6c` Exit 0 PASS했다.
- focused `CarFight.UI.UI_P0_06` process `73189888eb8042628db1b9499691c17c`에서 기존 5건 + 신규 `ResourcePresentationProjection`이 6/6 PASS / Failure 0이다. Result JSON SHA-256은 `39be662e7125ef4972574a75b433119283b7a279fbab56e511b140aafd6c70fe`다.
- UI-P0-06 전체 Done이나 새 USER Visual PASS로 확대하지 않는다. 다음 UI-only 단계는 Stage B Production Visual Container 적용이며 upstream `RedlineStartRPM / WeaponGroup / Battery / Charge / Heat` 부재는 그대로다.

Migration: 새 세션은 `CF-FQ-032 / InGameUIPlan.md v0.59.7 / InGameUIRoadmap.md v0.26.11`에서 Stage B Production Visual 적용을 검토한다. Stage A Build/6-test와 UI-P0-03~05 USER PASS는 관련 결함이 없는 한 반복하지 않는다.

### v3.15 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-03` 착수 중 새 세션 전환 체크포인트를 기록했다.
- 관리됨 차량이 0개여서 `DA_TestSedan`에 `DA_Recipe_UA_TestSedan` 새 Editor-only Recipe를 만드는 `제작 관리 시작` actual commit을 사용자가 실행했다.
- 세션 전환 요청 시점에는 생성 중이므로 성공/실패, 관리됨 전환, dirty state를 추정하지 않는다. 다음 세션은 현재 결과 확인부터 시작하고 blind 재실행하지 않는다.
- 대표 Plan/Roadmap은 `DataAuthoringPlan.md v0.2.25 / DataAuthoringRoadmap.md v0.1.33`, USER PASS는 2 유지다. 전역 단일 Active `CF-FQ-032`와 CF-FQ-038 Paused 상태는 변경하지 않았다.

Migration: 새 세션에서 UA-01~02를 반복하지 않는다. `DA_Recipe_UA_TestSedan` terminal 결과 확인 → 성공 시 UA-03 Recipe/Shared Profile/Affected Vehicle Impact USER 검사 순서로 재개한다.

### v3.14 - 2026-08-18

- `DAUTH-P0-12 / UA-02 Browser·Mesh-only·Create·Existing Import`를 사용자 직접 확인으로 USER PASS 처리했다.
- `CF-FQ-038`은 Paused를 유지하면서 USER checkpoint를 `PASS 2 / 다음 UA-03 Recipe·Shared Profile·Affected Vehicle Impact`로 전진시켰다.
- 대표 Plan/Roadmap projection을 `DataAuthoringPlan.md v0.2.24 / DataAuthoringRoadmap.md v0.1.32`로 갱신했다. DG/DEL/Wizard deletion은 계속 미개방이다.

Migration: UA-01~02는 반복하지 않는다. bounded DAUTH USER follow-up은 UA-03부터 진행한다.

### v3.13 - 2026-08-18

- 사용자가 ownership-unknown Editor를 안전하게 종료한 뒤 UI-P0-06 Presenter parity migration 최종 Official Build `d436ee40c530474d8507a7f66453da72`가 Exit 0 PASS했다.
- `CarFight.UI.UI_P0_06` focused process `76d293cd645b4eee8ca2a78fe9b830e7`에서 `MissingProviderAvailability / ResourceChannelProjection / ResourcePresenterParity / VehicleDriveRuntimeViewData / WeaponDisplayNameRuntimeViewData` 5/5 PASS, Failure 0을 확보했다. Result JSON SHA-256은 `cc614fce367436f13f33b1b812a5140d58346b39269603fceba412c4a4afbd3e`이다.
- 따라서 `ResourceChannels → Production Presenter` parity migration은 Technical PASS로 닫는다. 기존 UI-P0-03~05 USER PASS와 Ammo/Ripple/Salvo USER Visual은 반복하지 않았다.
- UI-P0-06 전체는 아직 Done이 아니다. 남은 upstream Runtime/Data Contract는 `RedlineStartRPM`, Player-facing WeaponGroup/WeaponGroupIndex·다중 무기 선택, VehicleBattery, WeaponCharge, 현재 Heat다. Production WeaponPanel의 완전 동적 0~N Resource Visual Container도 별도 후속 UI 구조로 남는다.

Migration: 새 세션은 `CF-FQ-032 / InGameUIPlan.md v0.59.6 / InGameUIRoadmap.md v0.26.10`을 기준으로 복원한다. Presenter parity Build/5-test를 반복하지 말고 남은 upstream Runtime/Data Contract와 동적 Resource Visual 구조를 분리해 다음 범위를 선택한다.

### v3.12 - 2026-08-18

- `UI-P0-06 ResourceChannels → Production Presenter` parity migration Source를 적용했다. Ammo·Reserve·Reload·NoAmmo·Cooldown·Launcher는 ResourceChannels를 우선 소비하고 채널 자체가 없는 legacy ViewData만 기존 필드 fallback을 사용한다. 문구·우선순위·LauncherSequenceRevision lifecycle과 Production UMG 구조는 변경하지 않았다.
- 실제 Heat Resource Channel이 향후 존재할 때만 기존 Heat Row를 표시하도록 Presenter를 열어두되, 현재 Battery·Charge·Heat Runtime을 만들거나 정적 설정에서 값을 추정하지 않았다.
- `ResourcePresenterParity` focused 회귀를 추가했다. 첫 Build `da95d88087954fa5bb423ae1453fc4e3`에서 test-only 잘못된 Reload enum `Idle`을 발견해 실제 `Ready`로 교정했고, 다음 Build `cfe11ae3d4ce4b27933c6941016265b6`에서 `CFHUDDataTests.cpp`와 Presenter compile은 PASS했다. 최종 Link는 현재 ownership-unknown `UnrealEditor.exe`의 DLL 점유로 `LNK1104`, Exit 6이다.
- 실행 중 Editor를 AI-owned로 증명할 수 없어 자동 종료하지 않았고, 새 Presenter binary가 링크되기 전 `CarFight.UI.UI_P0_06` 5-test Automation PASS를 추정하지 않는다.

Migration: Editor가 안전하게 종료된 뒤 동일 Source로 Official Build를 다시 수행하고 `CarFight.UI.UI_P0_06` focused 5건만 실행한다. UI-P0-03~05 USER PASS와 기존 Speed·Defense·Ammo·Ripple·Salvo USER 검증은 반복하지 않는다.

### v3.11 - 2026-08-18

- `UI-P0-06 Weapon Runtime Audit`에서 `EquipmentPresetData.DisplayName`이 이미 Fitting ViewData의 사람 읽기용 장비 이름으로 소비됨을 확인하고, 호환되는 현재 활성 EquipmentPreset의 비어 있지 않은 DisplayName만 Weapon HUD Player-facing 이름으로 연결했다. `WeaponId`·`EquipmentId`·`MountProfileId`·AssetName fallback은 만들지 않았다.
- 기존 finite Ammo·ReserveAmmo·Cooldown·Reload·LauncherSequence 실제 ViewData를 삭제·대체하지 않고 additive `FCFWeaponResourceHUDData / ResourceChannels`로 투영하는 공통 Resource Foundation을 추가했다. `VehicleBattery`·`WeaponCharge`·`Heat`는 실제 Runtime Provider가 없어 채널 자체를 생성하지 않는다.
- `WeaponGroup`은 현재 `MountProfileId` 계열 내부 식별자와 구분되는 Player-facing Group/Index Runtime이 없어 미구현으로 유지한다. RPM Gauge도 `RedlineStartRPM` 부재를 보존하고 임의 비율을 만들지 않는다.
- 최종 Official Build `b8c605f9d2ae483198d6a63d1b8e4987` PASS, focused `CarFight.UI.UI_P0_06` process `634d1a78b99b42be886f445068f75292` 4/4 PASS / Failure 0, Result JSON SHA-256 `b3d592cf59e6820245c1e89d4e68e7d593039282d06ca8bc883a5afc2b7c2fec`을 확보했다. UI-P0-03~05 USER PASS는 반복하지 않았다.

Migration: 새 세션은 `CF-FQ-032 / InGameUIPlan.md v0.59.4 / InGameUIRoadmap.md v0.26.8`의 UI-P0-06에서 이어간다. 기존 legacy Weapon Presenter 경로는 이전 USER PASS 보호를 위해 유지되며, 다음 UI-only 후보는 `ResourceChannels → Production Presenter` parity migration이다. `WeaponGroup / VehicleBattery / WeaponCharge / Heat`는 실제 Gameplay Runtime이 생기기 전 값이나 채널을 추정하지 않는다.

### v3.06 - 2026-08-18

- `UI-P0-05 TargetSelect Marker Integration`의 USER Visual을 완료했다. 사용자 PIE에서 선택 Marker 정확히 1개, 내부 Actor 이름·거리·Track 중복 텍스트 0, 화면 밖 숨김+선택 유지, 화면 재진입 Marker 단일 복구, 우클릭 해제와 가운데 버튼 재선택 단일 Marker 복구를 PASS했다.
- 기존 Official Build `242dfee7186d46aaa98e3c24b25c0d09` PASS와 focused Automation `3834e606f2b54efb9ac5c09ea410b4f4` 1/1 PASS를 함께 최종 UI-P0-05 evidence로 고정했다.
- UI-P0-03~05 기존 HUD 데이터·수명 이전을 USER PASS로 닫고 formal dependency에 따라 다음 Runtime Gate를 `UI-DESIGN-GATE 현재 비차단 상태 확인 → UI-P0-06 차량·무기 HUD`로 이동했다. D1-11 Production Structure는 PASS이고 D1-11-ART 세부 폴리시·D1-12는 Non-Blocking Deferred 상태를 유지한다.
- `CF-FQ-038`의 더 최신 Paused checkpoint `DataAuthoringPlan.md v0.2.20 / DataAuthoringRoadmap.md v0.1.28 / UA-01 USER PASS / USER PASS 1 / 다음 UA-02`는 변경하지 않았다.

Migration: 새 세션은 `CF-FQ-032 / InGameUIPlan.md v0.58.9 / InGameUIRoadmap.md v0.26.3`에서 UI-P0-06 착수 감사부터 시작한다. UI-P0-03~05의 Build/Automation/USER evidence는 관련 결함이 없는 한 반복하지 않는다. UI-P0-07 Target Knowledge와 UI-P0-08 Radar는 UI-P0-06 뒤의 formal 순서를 유지한다.

### v3.05 - 2026-08-18

- `DAUTH-P0-12 / UA-01 Workspace First Impression`을 사용자의 새 Workspace 직접 재확인으로 USER PASS 처리했다.
- `CF-FQ-038`은 전역 우선순위상 Paused를 유지하지만 보존 checkpoint를 `USER PASS 1 / 다음 UA-02 Browser·Mesh-only·Create·Existing Import`로 전진시켰다.
- 대표 Plan/Roadmap projection을 `DataAuthoringPlan.md v0.2.20 / DataAuthoringRoadmap.md v0.1.28`로 갱신했다. DG/DEL/Wizard deletion은 계속 미개방이다.

Migration: UA-01은 재실행하지 않는다. bounded DAUTH USER follow-up을 계속할 때는 UA-02부터 진행한다.

### v3.04 - 2026-08-18

- UI-P0-04 AimReticle UISubsystem 통합은 최종 Build `ace404b0eb7f4c97b010e96ce85f41ce` PASS, focused Automation `74f4bdfd283243e49369ae25afdce2f5` 1/1 PASS와 USER Visual에서 Reticle 단일 인스턴스·Defense→Baseline Pawn Rebind·중복/Old Pawn 잔류 없음 확인으로 USER PASS했다.
- UI-P0-05 TargetSelect Marker 통합 Source를 적용했다. `UCFUISubsystem v1.7.0`이 기존 `WBP_TargetSelect`을 Game Layer 단일 수명으로 소유하고 Current Pawn Rebind/cleanup을 수행하며, `CFVehiclePawn v2.149.0`의 direct `CreateWidget/AddToViewport` 자동 경로를 제거했다.
- `UCFTargetSelectWidget v1.1.0`은 Weak Pawn Binding, Marker-only 표현, 내부 Actor Name/거리/관계/Track Text 제거, 이벤트 기반 의미 캐시 + Projection-only Tick, 화면 재진입 Marker 복구를 적용했다.
- UI-P0-05 Official Build `242dfee7186d46aaa98e3c24b25c0d09` PASS, focused Automation `3834e606f2b54efb9ac5c09ea410b4f4`의 `CarFight.UI.UI_P0_05.TargetMarkerLayerContract` 1/1 PASS를 확보했다. USER Visual은 아직 Pending이며 UI-P0-05 완료로 확대하지 않는다.
- `CF-FQ-038`은 `DataAuthoringPlan.md v0.2.17 / DataAuthoringRoadmap.md v0.1.25 / P0-12 UA-01 / USER PASS 0` Paused 상태를 그대로 보존한다.

Migration: 새 세션은 `CF-FQ-032 / InGameUIPlan.md v0.58.8 / InGameUIRoadmap.md v0.26.2`에서 UI-P0-05 USER Visual을 진행한다. UI-P0-03/04는 관련 결함이 없는 한 반복하지 않으며, UI-P0-05 USER Visual PASS 전 TargetPanel/Radar 다음 Gate로 넘어가지 않는다.

### v3.03 - 2026-08-18

- `M_VehicleDefensePIE`에서 Defense Production Panel의 Shield `0/100`, 방향 Armor 손상, Integrity `40/100`과 Shield `100/100` 재생을 사용자 화면으로 확인해 Defense USER Visual을 PASS했다.
- Baseline Pawn Rebind 후 Shield 행 제거·Integrity `100/100`을 확인하고, Old Defense SUV를 추가 피해로 파괴해도 Current Pawn/HUD가 변하지 않음을 확인해 Pawn Rebind와 Old Pawn Event Isolation USER Visual을 PASS했다. UI-P0-03은 USER PASS로 닫았다.
- 현재 Gate를 UI-P0-04 AimReticle UISubsystem Integration으로 이동했다. Source는 `UCFUISubsystem v1.6.0` HUD Layer 단일 Reticle 소유·Current Pawn Rebind·Cleanup, `CFVehiclePawn v2.148.0` direct Reticle CreateWidget/AddToViewport 자동 경로 제거, `DefaultAimReticleWidgetClass` Config와 `CFUIFoundationTests v1.2.0` 계약 보강까지 적용했다.
- UI-P0-04 Official Build·focused Automation·USER Visual은 Pending이며 PASS 전 UI-P0-05로 넘어가지 않는다.
- `CF-FQ-038` Paused projection의 대표 Plan/Roadmap을 실제 current인 `DataAuthoringPlan.md v0.2.17 / DataAuthoringRoadmap.md v0.1.25`로 교정했다. UA-01 / USER PASS 0은 그대로 보존한다.

Migration: 새 세션은 `CF-FQ-032 / InGameUIPlan.md v0.58.7 / InGameUIRoadmap.md v0.26.1`에서 UI-P0-04 Build·Automation·USER Visual을 이어간다. UI-P0-03은 관련 결함이 없는 한 반복하지 않으며 DAUTH는 명시적 재개 전 Paused다.

### v3.02 - 2026-08-18

- 사용자 우선순위 변경에 따라 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`를 현재 단일 Active로 복원했다.
- 즉시 재개 Gate는 `UI-P0-03 Defense Production Panel USER Visual → Pawn Rebind USER Visual`이며 기존 UI-P0-02/03 기술·USER PASS는 관련 결함이 없는 한 반복하지 않는다.
- `CF-FQ-038`은 P0-08A~M/P0-09~11 Technical PASS와 `DAUTH-P0-12 USER PASS 0 / UA-01` 체크포인트를 그대로 보존한 Paused로 전환했다. DG/DEL Gate는 계속 미개방이다.
- 완료된 `CF-FQ-037 Scanner`의 `SensorContact.md v1.1.0`은 이후 TargetPanel/Radar UI가 소비할 Current Sensor source로 사용하되 Radar/TargetPanel USER Visual을 완료로 추정하지 않는다.

Migration: 새 세션은 `CF-FQ-032 / InGameUIPlan.md v0.58.6`에서 당시 UI-P0-03 남은 USER Visual Gate부터 시작한다. DAUTH는 현재 `DataAuthoringPlan.md v0.2.17 / DataAuthoringRoadmap.md v0.1.25 / P0-12 UA-01` 상태를 보존하고 UI 우선 작업이 끝나기 전 자동 재개하지 않는다.

### v3.01 - 2026-08-18


- `CF-FQ-037 / SCAN-P0-06 USER PIE Acceptance`를 사용자 확인으로 PASS 처리하고 `SCAN-P0-07 Current System Integration`까지 닫아 Feature를 Done으로 전환했다.
- Scanner USER PIE에서 V 단발 5초 Active Scan, 자동 종료, 활성 중 V 반복 무연장과 선택 Target의 `???` 해제를 확인했다. USER Acceptance용 임시 관측 코드는 제거했고 최종 Build `fcf52353d1f5440392d5e1c379f09ee3` PASS를 확보했다.
- 현재 구현 owner는 `Document/Systems/Targeting/SensorContact.md v1.1.0`이며 `ScannerIntegrationPlan.md`는 Historical + Retained Path로 전환한다.
- `CF-FQ-038 / DAUTH-P0-12 In Progress`를 포함한 다른 Active/Paused/Ready 상태는 변경하지 않았다.

Migration: CF-FQ-037은 ActiveWork 재개 대상이 아니다. Scanner 현재 구현은 `SensorContact.md v1.1.0`을 우선하고, 완료 당시 fixture RCA와 USER Acceptance는 Historical `ScannerIntegrationPlan.md`에서 조회한다. Radar/TargetPanel 시각·Sensor energy/heat·AI/Network는 별도 후속 Feature로 유지한다.

### v3.00 - 2026-08-18


- `DAUTH-P0-12 USER Authoring Acceptance`를 실제 사용자 검증 단계로 착수했다.
- P0-08A~M/P0-09~11 Technical PASS는 반복하지 않고 Frozen 24.99의 사용성·가독성·입력 편의·Driving Feel USER 판단만 진행한다.
- Roadmap v0.1.24의 UA-01~08을 current checklist로 연결했으며 현재 USER PASS는 0이다.
- DG/DEL Gate와 Wizard 삭제는 P0-12 완료 전까지 미개방 상태를 유지한다.

Migration: 다음 세션/응답은 UA-01부터 사용자 직접 확인 결과만 기록한다. 기술 자동화 성공을 USER PASS로 변환하지 않는다.

### v2.99 - 2026-08-18

- `DAUTH-P0-11 Frozen UX Completeness Closure`를 Technical PASS로 닫았다. Handling/Performance Adoption, Shared Profile B2 edit+affected impact, External Drift 3-way recovery, Mesh-only Candidate/Create Vehicle From Mesh normal Workspace route를 existing Core 재사용으로 구현했다.
- Drift prospective simulation duplicate가 `PostDuplicate()`에서 새 RecipeId를 발급해 approval hash가 비결정적이던 defect를 원본 RecipeId 복원으로 교정했고 approval binding은 약화하지 않았다.
- final Build `98dfceab797942218bd09c844e398ceb` PASS, focused P0-11 `b53998e6cacf48a1a554d784b77e013c` 6/6 PASS, full Data Authoring `77b45525549b4866995b3d972fb4a9fa` 63/63 PASS를 current evidence로 고정했다.
- full result JSON SHA-256은 `a4ef815caceb5fb5f413b849393f2d5e8ac1e45929f7f5a3cf1f50347bc0bb1f`다.
- Runtime `UCFVehicleData`, Inventory/Fitting production source, Content Asset, Wizard 삭제, DG/DEL은 변경/개방하지 않았다. 다음 dependency는 P0-12지만 이번 작업에서는 착수하지 않았다.

Migration: P0-08A~M/P0-09/P0-10/P0-11 Technical PASS를 반복하지 않는다. 다음 실제 사용자 검증 작업을 선택할 때만 P0-12를 별도로 착수하고, USER UX/Driving Feel PASS는 그 전까지 미판정으로 유지한다.

### v2.98 - 2026-08-18

- `DAUTH-P0-11` missing E2E coverage만 보강해 representative `AuthoringE2E`와 `ConsumerRegression`을 추가했다. production Authoring/Runtime/Inventory/Fitting source는 변경하지 않았다.
- final Build `ae0e92c60a094be09b036ae12b106717` PASS, focused P0-11 2/2 PASS, Inventory representative 1/1 PASS, Fitting representative 2/2 PASS, full Data Authoring `071d20db24f24c15bb2cb6c553973433` 59/59 PASS를 확보했다.
- Section 14와 Frozen 24.98 최소 technical behavior는 PASS지만, 명시적으로 감사한 Frozen 24.90~24.94에서 Handling/Performance Adoption UI, normal Shared Profile edit/impact, External Drift 3-way recovery, Mesh-only Create flow 미구현을 확인했다.
- 따라서 P0-08/P0-09/P0-10 Technical PASS는 유지하고 P0-11은 `In Progress / Core PASS / Completeness Blocked`, P0-12는 Not Ready로 유지한다.
- 다음 착수는 `DataAuthoringRoadmap.md v0.1.22 / DAUTH-P0-11 Frozen UX Completeness Closure`다. Wizard 삭제/DG/DEL/USER Acceptance는 열지 않는다.

Migration: 기존 완료 Automation은 반복하지 않고 Frozen 24.90~24.94 production gap closure와 그에 필요한 focused validation부터 이어간다.

### v2.97 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-10 Existing Wizard Migration`을 Technical PASS로 닫았다.
- 새 Vehicle Authoring Workspace에 Assets/Layout semantic intent + derived socket readback, explicit Wheel Measurement decision, Frozen 4축 Driving Feel/preset, read-only 117-field Reference Compare, Stable-ID Hardpoint/Mount/Default intent와 Workspace-owned Unreal standard Undo를 parity했다.
- P0-10 Common Authoring facade는 기존 Snapshot/Resolver/Import Core를 재사용하며 새 Workspace/ViewModel은 Core를 직접 호출하지 않는다. Target writer는 계속 `FCFVehicleApplyService` 하나다.
- legacy `SCFVDAWizardTab`은 managed Recipe Target에서 Layout Capture / Quick Tune Apply / Quick Tune Revert를 비활성화하고 handler에서도 managed 상태를 재검사한다. Validate/Compare/Raw Open/Copy와 unmanaged legacy path는 유지한다.
- Wizard 삭제, DG/DEL Gate, Batch main page는 열지 않았고 Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset은 변경하지 않았다.
- final Build `03fa78af4e124a3db33893ca8bfe436d` PASS, focused P0-10 `3e6c924318124fbebc742e2400d83308` 4/4 PASS, full Data Authoring `c25dbb8e10eb4bdda5733ead78aa4a43` 57/57 PASS, JSON SHA `90c01eda8b1df96f03d55feb1e3000f99db16f8a717baa697f5c15cdb9f6884b`를 current evidence로 연결했다.
- 대표 Plan `DataAuthoringPlan.md v0.2.13`, Roadmap `DataAuthoringRoadmap.md v0.1.21`, 다음 Gate `DAUTH-P0-11 Technical Validation`로 이동했다.

Migration: P0-08A~M/P0-09/P0-10을 반복하지 않고 P0-11 Technical Validation에서 E2E/non-regression을 진행한다. P0-12 USER Acceptance와 DG/DEL 전 Wizard 삭제는 금지한다.

### v2.96 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-09 Vehicle Authoring MVP`를 Technical PASS로 닫았다.
- `FCFVehicleAuthoringVM` + `SCFVehicleAuthoringTab`으로 single-Vehicle Browser/Selection, Initial Import, Recipe basic intent, Resolve Preview, Pending Diff, Source Trace, Validation, Apply/standard Undo와 Raw DA Open의 최소 Workspace를 구현했다.
- UI/ViewModel은 `FCFVehicleAuthoringService` facade만 사용하고 Core Snapshot/Resolver/Import/Apply/Validator/Batch를 직접 호출하지 않는다. Target mutation authority는 계속 `FCFVehicleApplyService` 하나다.
- 기존 `CarFight.VehicleDAWizard` / `SCFVDAWizardTab`을 그대로 유지하고 새 `CarFight.VehicleAuthoring` Nomad Tab을 병행 등록했다. P0-10 parity 범위와 Batch main page는 구현하지 않았다.
- final Build `76fc476c8ccb4daf895a3b567fe0c992` PASS, focused P0-09 `ef56e0b60ae944c19bf481fa070503c9` 6/6 PASS, full Data Authoring `fba6b0793f90450caf2a38eb82844e7c` 53/53 PASS, JSON SHA `697230dbce1c9a6281a0e56c4aa292544d3becd28b701a25285a487a50592654`를 current evidence로 연결했다.
- `Workspace.TabRegistration`에서 신규/기존 두 Nomad Tab spawner 공존과 신규 Workspace 실제 Slate content 생성을 검증했다. USER 시각/사용성 PASS로 확대하지 않는다.
- 대표 Plan `DataAuthoringPlan.md v0.2.12`, Roadmap `DataAuthoringRoadmap.md v0.1.20`, 다음 Gate `DAUTH-P0-10 Existing Wizard Migration`으로 이동했다.

Migration: P0-08A~M과 P0-09를 반복하지 않고 P0-10에서 기존 Wizard 기능 parity/managed-target guard를 진행한다. Wizard 삭제는 별도 DG/DEL Gate 전까지 금지한다.

### v2.95 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08M B3 Batch Definition Apply Foundation`을 Technical PASS로 닫고 `DAUTH-P0-08 Implementation Foundation` 전체를 Technical PASS로 종료했다.
- fresh R3 evidence, deterministic BatchApplyPlanHash, exact ordered Target/per-target evidence approval, first-mutation 전 all-target global preflight와 TargetPath ascending per-Vehicle ApplyService orchestration을 구현했다.
- NoChange/ShadowOnly/ExternalDrift/Resolve Error·Blocked/Validation Blocked는 B3 ineligible이며 External Drift bulk resolution은 없다.
- Stop On First Failure에서 Applied/Failed/NotStarted를 보존하고 batch global transaction/rollback, already-applied rollback, automatic retry/save는 0이다. Target mutation authority는 기존 `FCFVehicleApplyService`만 유지한다.
- final Build `10027d18360e48f399a5b439f280c24c` PASS, focused M process `a49d21b6b9b94d648142d5a555a79c95` 4/4 PASS, final targeted process `cb2ae69e9c834657a553fe52c00f5a96` 47/47 PASS, JSON SHA-256 `472ef5bb8f5fdeb46ebd1b7e68f1fc5a66acba065828ff025167b475aaf25595`를 current evidence로 연결했다.
- Frozen 26.66~26.69 BatchOperationId/generic envelope는 M dependency가 아니며 첫 실제 external Batch transport/client 통합 전에 재검토할 follow-up으로 보존했다.
- 대표 Plan `DataAuthoringPlan.md v0.2.11`, Roadmap `DataAuthoringRoadmap.md v0.1.19`, 다음 Gate `DAUTH-P0-09 Vehicle Authoring MVP`로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-08A~M과 P0-08 Foundation을 반복하지 않고 `DAUTH-P0-09 Vehicle Authoring MVP`에서 시작한다. Batch UI는 Frozen 26.70대로 P0-09 필수가 아니다.

### v2.94 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`을 Technical PASS로 반영했다.
- exact BatchPlanHash/included RowId/proposed patch/current fingerprint approval binding, all-source global TOCTOU preflight, B1 Recipe/B2 shared Profile typed commit과 one logical transaction/all-or-nothing rollback을 구현했다.
- External Drift/ShadowOnly를 source commit blocker로 승격하지 않고 B1 Recipe validation과 B2 affected Recipe inventory/validation을 Frozen 26.47/26.49 경계대로 fresh 재검사한다.
- final Build `2e2923f31d5a4f3fa703f3b7623c0741` PASS, targeted process `3853d42dfb344bfd8eb6e516903a8d78` 43/43 PASS, result JSON SHA-256 `eef2051ec4b3ccb5dce10b8b76761299885d18a8bf7f2c9db42cc2967320abcf`를 current evidence로 연결했다.
- successful B1/B2 뒤 old Preview/Approval은 stale하며 fresh read/resolve가 필요하다. B3/Target mutation/UI/file save는 아직 0이다.
- 대표 Plan `DataAuthoringPlan.md v0.2.10`, Roadmap `DataAuthoringRoadmap.md v0.1.18`, 다음 Gate `DAUTH-P0-08M B3 Batch Definition Apply Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-08A~L을 반복하지 않고 Frozen Section 26.54~26.65의 P0-08M에서 시작한다. `P0-08M`은 기존 Frozen B3 구간에 이번 Roadmap에서 부여한 implementation checkpoint label이다.

### v2.93 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`을 Technical PASS로 반영했다.
- companion manifest/current Registry 검증, canonical CSV parse, duplicate/read-only guard와 Export/Edited/Current 3-way classification을 구현했다. Fingerprint mismatch는 signal-only이고 ownership/source mode 변경은 explicit conflict다.
- SafeCandidate Recipe/Profile edit는 transient duplicate에만 적용한 뒤 existing Authoring facade/Pure Resolver로 prospective preview하며 persistent Recipe/Profile/Target mutation은 0이다.
- final Build `a3d37d4c8520442d8ceb09a72eb6a68f` PASS와 targeted process `7d2db5a9baa54dd9abf25857138d3994` 37/37 PASS, result JSON SHA-256 `bbdf497d472e5ec2fbe4458e075907897ca79b2ac984d43ba3290cff3c55eeec`를 current evidence로 연결했다.
- Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader를 변경하지 않았고 B1/B2 commit, B3 Apply, UI, file save는 0이다.
- 대표 Plan `DataAuthoringPlan.md v0.2.9`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.17`, 다음 Gate `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D/E/F/G/H/I/J/K를 반복하지 않고 P0-08L에서 시작한다. B3는 Section 26.54+ 별도 후속 Gate로 유지한다.

### v2.92 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`을 Technical PASS로 반영했다.
- Batch Registry를 existing Recipe/Profile typed schema와 Frozen 117 Field Registry의 projection으로 구현하고 Spreadsheet/CSV를 새 Authoring Source 또는 SSOT로 만들지 않았다.
- Recipe numeric 7개와 Profile typed scalar numeric 78개, reserved `__cf_`, stable ColumnId, typed mutation metadata, source-mode editability를 구현했다.
- canonical BOM-less UTF-8 CSV, fixed-order `.cfbatch.json` baseline manifest와 deterministic ExportSetHash를 구현했다.
- final Build `3249098c1b99487a8fd173694573bd5e` PASS와 targeted process `534486583a1d4689bc5137505a03f806` 32/32 PASS, result JSON SHA-256 `84fc15de1ad23b4e13891d91db09fa3668a49c900a438dec17b70d818a45d36f`를 current evidence로 연결했다.
- Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader를 변경하지 않았고 Import Session/B1·B2/B3/UI/file save/persistent Batch mutation은 0이다.
- 대표 Plan `DataAuthoringPlan.md v0.2.8`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.16`, 다음 Gate `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D/E/F/G/H/I/J를 반복하지 않고 P0-08K에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.91 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`을 Technical PASS로 반영했다.
- `CFVehicleAIContract`와 `FCFVehicleAuthoringService`를 추가해 Section 25의 R0~R3 risk/approval/result envelope와 UI/AI 공용 single-Vehicle facade를 구현했다.
- R0 List/Read/Resolve/Diff/Trace/Validation/Drift는 기존 SnapshotBuilder/AssetReader/Pure Resolver authority를 그대로 사용하고 persistent mutation은 0이다.
- `PreviewRecipeChange`는 persistent mutation 없이 prospective Recipe/Resolve/ProposalHash를 만들며, R1 commit은 exact AuthoringWrite + expected Recipe/Target/revision + fresh proposal scope 아래 Recipe만 transaction mutation한다.
- `VehicleArchetypeId`가 Section 22.27 Frozen resolver RecipeFingerprint input 밖이라는 사실을 재확인하고 hash 계약은 변경하지 않았다. facade v1.1.0에서 operation-specific typed desired-state equality로 non-resolver semantic NoChange를 처리했다.
- bounded ClientOperationId dedupe를 추가해 same request replay는 second mutation 0, same id/different request는 conflict이며 automatic retry는 0이다.
- R3 `ApplyResolvedVehicle`는 ExpectedDiffHash/DefinitionApply approval을 검사한 뒤 existing `FCFVehicleApplyService::Apply` actual 1회만 호출한다. Raw SetField/direct Target writer/force/skip-validation/Save 경로는 없다.
- final Build `252fdbd097f943379f2a1e2a942bedef` PASS와 targeted process `f2011a206c964fadb269d13c4dba3c86` 27/27 PASS, result JSON SHA-256 `b9bfea5be21f1bcc521e0bcfa8b8a43c017e18a4dec65a208bfd57a68805bfbf`를 current evidence로 연결했다.
- Runtime schema, Inventory/Fitting, Wizard, Content Asset, Asset Reader, existing Validator, UI/CSV/Batch는 변경하지 않았다.
- 대표 Plan `DataAuthoringPlan.md v0.2.7`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.15`, 다음 Gate `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D/E/F/G/H/I를 반복하지 않고 P0-08J에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.90 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08H Apply Transaction Foundation`을 Technical PASS로 반영했다.
- `FCFVehicleApplyService`를 production Target `UCFVehicleData` 유일 writer로 추가하고 Recipe/Source/Target/Resolved/ResolverRevision TOCTOU precondition, fresh Resolve/Diff equality, transient duplicate preflight를 구현했다.
- Section 22.31 dependency order로 current-only Stable-ID removal, Hardpoint/Mount add와 exact leaf write를 수행하고 existing `UCFVDAValidator`, Resolver-owned readback hash를 persistent mutation 전후로 재검증한다.
- successful Apply는 Target+Recipe `FScopedTransaction`, `FCFVehicleAppliedState`, Package Dirty/PostEditChange까지만 수행하고 Save는 하지 않는다.
- A8 actual Target mutation 뒤 controlled failure를 포함한 rollback Automation에서 Target full hash, stable arrays, Recipe AppliedState와 pre-apply dirty flags 복원을 확인했다.
- final Build `b88831b71797415fac9dc0a23d12c39e` PASS와 targeted process `6f261c0b76f04d0faa6b9e855865818e` 23/23 PASS를 current evidence로 연결했다.
- Runtime schema, Inventory/Fitting, Wizard, Content Asset, Asset Reader, existing Validator source는 변경하지 않았고 UI/CSV/Batch 구현도 0이다.
- 대표 Plan `DataAuthoringPlan.md v0.2.6`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.14`, 다음 Gate `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D/E/F/G/H를 반복하지 않고 P0-08I에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.89 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08G Existing Definition Import / Adoption Foundation`을 Technical PASS로 반영했다.
- Current Definition exact field 전체를 Recipe ImportState의 normal Legacy Pin과 Mount hidden LegacySerialized passthrough로 lossless 분리하고 direct semantic candidate copy를 구현했다.
- Movement raw 값에서 Driving Feel/Profile을 authoritative하게 역산하거나 자동 binding하지 않으며 Legacy Pin이 초기 ownership을 계속 보존한다.
- group/field Adoption Preview는 persistent Recipe mutation 없이 selected Pin만 virtual release하고, approved Commit은 stale fingerprint precondition 아래 Recipe Pin/AdoptedGroups/ManageState/revision만 transaction으로 갱신한다.
- final Build `b201d87cd13f4987a0907e08c8f00a6c` PASS와 targeted process `3b52a251ab244096b78bb872a06c0069` 20/20 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Validator는 변경하지 않았고 Target Definition mutation / Apply / UI / CSV도 0이다.
- 대표 Plan `DataAuthoringPlan.md v0.2.5`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.13`, 다음 Gate `DAUTH-P0-08H Apply Transaction Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D/E/F/G를 반복하지 않고 P0-08H에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.88 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08F Definition Materializer / Validation Foundation`을 Technical PASS로 반영했다.
- Resolver R15에서 `SortedResolvedFields`를 RF_Transient `UCFVehicleData`로 복원하고 Stable-ID Hardpoint/Mount exact selector 재구성, FieldCodec checked import, materialized readback hash 검증을 구현했다.
- 기존 `UCFVDAValidator::ValidateVehicleData(Candidate, nullptr)`를 수정 없이 연결해 `DefinitionValidation`을 실제 결과로 채우고 Definition Error/Blocked와 Resolver internal Error를 분리했다.
- final Build `e5d925c6531e4ae6ac58aa356e4078eb` PASS와 targeted process `4b21d25b780c4a398de7a1b47c595c5e` 17/17 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader와 기존 Runtime Validator source는 변경하지 않았고 Apply / UI / CSV도 구현하지 않았다.
- 대표 Plan `DataAuthoringPlan.md v0.2.4`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.12`, 다음 Gate `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D/E/F를 반복하지 않고 P0-08G에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.87 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08E Pure Resolver Foundation`을 Technical PASS로 반영했다.
- Snapshot-only `FCFVehicleResolveRequest/Result`, Frozen R0~R16, stage-independent source precedence, Proposal/Adoption, Source Trace/Hash, R14 Diff와 R16 Stale/Drift foundation을 구현했다.
- R15 transient Definition Materializer + `UCFVDAValidator`는 아직 Deferred이며 다음 `DAUTH-P0-08F Definition Materializer / Validation Foundation`으로 분리했다.
- official Build `db202f0797af41fe86a859609cb4dfd9` PASS와 targeted process `93d445f78cbe4eaabca1478eb6d27078` 15/15 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset과 Asset Snapshot Reader는 변경하지 않았고 Apply / UI / CSV도 구현하지 않았다.
- 대표 Plan `DataAuthoringPlan.md v0.2.3`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.11`로 projection을 갱신했으며 `DAUTH-P0-08`은 계속 In Progress다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D/E를 반복하지 않고 P0-08F에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.86 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08D Asset Snapshot Reader Foundation`을 Technical PASS로 반영했다.
- Chassis requested socket facts, 4개 Wheel local bounds와 resolver-relevant asset fingerprint를 immutable `FCFVehicleAssetSnapshot`으로 구현했다.
- official Build `780d30c4a44f48feb179f7f6da5480e1` PASS와 targeted process `ab2cd64308484ce0ae0e24cb1143ca33` 9/9 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset은 변경하지 않았고 Resolver / Apply / UI / CSV는 아직 구현하지 않았다.
- 대표 Plan `DataAuthoringPlan.md v0.2.2`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.10`, 다음 Gate `DAUTH-P0-08E Pure Resolver Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C/D를 반복하지 않고 P0-08E에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.85 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08C Immutable Snapshot Foundation`을 Technical PASS로 반영했다.
- Recipe Snapshot, 5 Profile Snapshot Set, Registry-expanded Definition Snapshot, 117-pattern Project Compatibility Default Snapshot과 deterministic fingerprint/hash를 구현했다.
- Registry `RequiredDependencies`를 Frozen Section 22.17 계약에 맞춰 채우고 targeted Automation에서 dependency / Recipe / Profile / Definition+ProjectDefault Snapshot을 검증했다.
- final official Build `6c81b2149de44d00a99554a44d2135aa` PASS, targeted process `340050d2c911445da3634ebb92acc014` 8/8 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset은 변경하지 않았고 Asset Snapshot Reader / Resolver / Apply / UI / CSV는 아직 구현하지 않았다.
- 대표 Plan `DataAuthoringPlan.md v0.2.1`, 상세 Roadmap `DataAuthoringRoadmap.md v0.1.9`, 다음 Gate `DAUTH-P0-08D Asset Snapshot Reader Foundation`으로 projection을 이동했다.

Migration: CF-FQ-038 재개 시 P0-01~07과 P0-08A/B/C를 반복하지 않고 P0-08D에서 시작한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.84 - 2026-08-17

- 사용자 요청에 따라 `CF-FQ-038 차량 데이터 Authoring 시스템`을 단일 Active로 정식 등록했다.
- `DAUTH-P0-08A/B` 첫 Foundation slice에서 Editor-only Recipe + 5 Profile, `IsEditorOnly()` Never-Cook, Authoring common types, Stable Field Path / Field Value Codec, current `UCFVehicleData` 117 leaf Registry 양방향 coverage를 구현했다.
- final official Build `5c745a31d19b448d9b1049877bc877ca` PASS와 targeted `CarFight.DataAuthoring.DAUTH_P0_08.Foundation` process `ab9de3e8c8cd410a8de0641178690dff` 4/4 PASS를 현재 기술 증거로 연결했다.
- `Tools/RunDataAuthoringTests.ps1`를 Data Authoring targeted Automation 재실행 진입점으로 추가했다.
- Runtime `UCFVehicleData`, Inventory/Fitting 소비 경로, `SCFVDAWizardTab`, Content Asset은 변경하지 않았다.
- `CF-FQ-034`는 Done 처리하지 않고 `FIT-P0-07D USER Driving Feel Comparison`을 보존한 Paused로 전환했다.
- 다음 Gate는 `DAUTH-P0-08C Immutable Snapshot Foundation`이다.

Migration: CF-FQ-038 재개 시 P0-01~07 설계를 다시 열지 않고 P0-08A/B PASS를 보존한다. CF-FQ-034 재개 시 FIT-P0-07A~07C를 반복하지 않고 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 시작한다.

### v2.83 - 2026-08-17

- CF-FQ-034 `FIT-P0-07C Quantitative Mobility Measurement`을 Technical Complete로 반영했다.
- 기존 공식 Light 1000kg / Default 1570kg / Heavy 1600kg를 fixture별 fresh PIE lifetime에서 같은 프로토콜로 계측했다.
- 최종 official Build `bb04d56e0cd24647b2ef6fcbc7f2bd77` PASS와 process `63bbeaf0202041b79dac8f32f5447923` Success / Metric 3/3을 현재 체크포인트에 연결했다.
- 가속·제동·coast-steering 수치는 확보했지만 기대 순서를 PASS 조건으로 추가하지 않았고 `ordering_asserted=false`, USER 주행감은 0/Pending으로 유지했다.
- 자동 Mobility Scalar와 새 질량값은 추가하지 않고 다음 Gate를 `FIT-P0-07D USER Driving Feel Comparison`으로 이동했다.

Migration: CF-FQ-034 재개 시 FIT-P0-07A~07C와 기존 Fitting 23/23을 반복하지 않고 `VehicleFittingPlan.md v0.17.0 / FIT-P0-07D`에서 같은 공식 Fixture를 사용해 사용자 직접 주행감만 비교한다.

### v2.82 - 2026-08-17

- CF-FQ-034 `FIT-P0-07B Heavy Payload Resolution + Official Fixture Preparation`을 Technical Complete로 반영했다.
- 같은 `DA_VehicleDefense_TestSUV` 플랫폼에 Light 1000kg / Default 1570kg / Heavy 1600kg 공식 Fixture를 저장하고 임의 질량 추가 없이 순서를 고정했다.
- verify_existing `c33a36b21a8f4079b38bba7c51e12864` PASS, fresh AssetDump persisted DisplayName·참조 PASS, post-label `OfficialMobilityFixtures` `5ab277165e9d4aef9e05b779106c808f` 1/1 PASS를 확인했다.
- USER PIE·USER 주행감은 0/Pending으로 유지하고 다음 Gate를 `FIT-P0-07C Quantitative Mobility Measurement`로 이동했다.

Migration: CF-FQ-034 재개 시 FIT-P0-07A~07B를 반복하지 않고 `VehicleFittingPlan.md v0.16.0 / FIT-P0-07C`에서 시작한다. 공식 Fixture의 Production 질량 Source는 임의 변경하지 않는다.

### v2.81 - 2026-08-17

- 사용자 선택에 따라 CF-FQ-034 차량 피팅·질량 런타임을 단일 Active로 전환했다.
- `VehicleFittingPlan.md v0.15.0 / FIT-P0-07A Fixture Readiness Audit`을 완료 체크포인트로 연결했다.
- AssetDump에서 CityCar·Compact·Coupe·Pickup·SubCompact·Van·Wagon은 StaticMesh-only Visual 후보, DA_TestSedan·DA_TestSUV는 0/0kg VehicleData baseline, DA_VehicleDefense_TestSUV는 1000/2500kg Fitting-ready technical platform으로 분리했다.
- Light=1000kg, Default=1570kg은 같은 기술 플랫폼에서 임의 질량 없이 준비 가능하고 Heavy는 같은 플랫폼의 persisted payload가 1570kg 초과인지 확인하는 FIT-P0-07B로 이동했다.
- Source·Content Asset·Build·USER PIE는 변경하거나 실행하지 않았다.

Migration: CF-FQ-034 재개 시 FIT-P0-07A를 반복하지 않고 `VehicleFittingPlan.md v0.15.0 / FIT-P0-07B Heavy Payload Resolution`에서 시작한다. Visual-only 메시 차량은 VehicleData 생성 전 Fixture 후보에서 제외한다.

### v2.80 - 2026-08-17

- CF-FQ-030의 과거 WinError 87 Persisted Asset readback 공백을 current AssetDump로 독립 검증해 Technical PASS로 닫았다.
- `MissileGuidancePlan.md v0.5.0`을 대표 Plan으로 연결하고 fresh DirectRuntimeContract `29d0ef16dd5e4d56945ceae26eec6937` 1/1 PASS를 보호 evidence로 기록했다.
- Missile test folder 20/20, Maps World 12/12 PASS와 4개 계약 DataAsset 저장값·hard reference chain을 확인했다.
- MissileDirectTest World package는 33 Actor와 Missile test vehicle/RocketLauncher socket 구성을 확인했지만 전체 Actor label은 public readback 범위 밖이므로 5개 MissileTarget label 자체를 PASS로 추정하지 않았다.
- Runtime Source·Content Asset·Blueprint·Map 저장 변경과 USER PIE는 0이며 CF-FQ-030은 Done이 아니라 Ready / Manual PIE Pending이다.
- 현재 단일 Active는 계속 비운다.

Migration: CF-FQ-030 재개 시 `MissileGuidancePlan.md v0.5.0 / CF-TC-027 Manual PIE`에서 시작한다. Persisted Asset Technical Verification은 관련 Source/Asset 변경이 없는 한 반복하지 않는다.

### v2.79 - 2026-08-16

- CF-FQ-029 `LM-P0-06A Failure Policy Technical Closure`를 Technical PASS로 종료했다.
- 최종 closure 공식 Build `6a633eb4cf21481d8ae26ec908d05660` PASS, `CarFight.Launcher` Process `7ab7a286e5484cab845bb0cbfd5ac04e` 4/4 PASS와 `AMMO_P0_04.LauncherLock` Process `1781accd3d9c47a59421fb1d2228e4ab` 1/1 PASS를 대표 Plan evidence로 연결했다.
- Launcher Runtime은 재구현하지 않았고 테스트에 StopSequence terminal 이후 추가 Dispatch 불가 보호 assert만 보강했다.
- CF-FQ-029는 `LM-P0-06 USER PIE` 체크포인트가 보존된 Paused로 전환했으며 현재 단일 Active는 비웠다.
- MuzzleBlocked·Ejection·Carrier Velocity USER PIE와 CF-FQ-037 `SCAN-P0-06 USER PIE`는 계속 Pending이다.

Migration: CF-FQ-029 재개 시 `LauncherMissilePlan.md v0.14.0 / LM-P0-06`부터 시작한다. LM-P0-06A Build/Automation은 결함이 없는 한 반복하지 않는다.

### v2.78 - 2026-08-16

- 사용자 선택에 따라 USER PIE가 필요한 CF-FQ-037을 P0-06 체크포인트가 보존된 Paused로 전환하고 CF-FQ-029를 단일 Active로 승격했다.
- 새 next gate를 `LauncherMissilePlan.md v0.13.0 / LM-P0-06A Failure Policy Technical Closure`로 고정했다.
- `ContinueRemaining / StopSequence`는 현재 `FCFLauncherSequenceRuntime`의 순수 상태 전이를 asset-free Automation으로 검증하고, 기존 Manual Cancel + Ammo Reservation cleanup 테스트는 보호 증거로 사용한다.
- MuzzleBlocked·Ejection·Carrier Velocity USER PIE는 그대로 Pending이며 CF-FQ-029 전체 Done으로 승격하지 않는다.

Migration: 새 세션은 CF-FQ-029 Failure Policy Technical Closure부터 시작한다. CF-FQ-037은 화요일 USER PIE 가능 시 `ScannerIntegrationPlan.md v0.7.1 / SCAN-P0-06`에서 그대로 재개한다.

### v2.77 - 2026-08-16

- CF-FQ-037 대표 Plan projection을 `ScannerIntegrationPlan.md v0.7.1`로 동기화했다.
- P0-06A Scanner 전용 fixture readiness 완료 상태를 ActiveWork에 반영했으며 USER PIE는 미실행 상태로 유지했다.
- P0-06 USER 범위를 `V` 단발 입력, 5초 timed scan 자동 종료, 반복 입력 무연장, scanner-less safe reject, TargetSelect/HUD 기본 비회귀로 동기화했다.
- P0-04 hot reapply/Contact·Knowledge 보존은 기존 Technical Acceptance로 유지하고 미구현 Field Fitting USER UI를 CF-FQ-037 P0-06 완료 조건으로 끌어오지 않는다.

Migration: CF-FQ-037 재개 시 `ScannerIntegrationPlan.md v0.7.1`의 P0-06에서 시작한다. P0-06A fixture readiness는 반복하지 않고 실제 USER PIE만 남아 있다.

### v2.76 - 2026-08-16

- `CF-FQ-037 SCAN-P0-05 Technical Validation`을 Technical Done으로 종료하고 대표 Plan을 `ScannerIntegrationPlan.md v0.7.0`으로 갱신했다.
- P0-04 final Build `20ca14bdc6094650bc11209f4ce320fa`, FittingIntegration 1/1, Fitting 22/22, Sensor 14/14, Inventory 12/12 PASS를 반복 없이 Technical Acceptance evidence로 재사용했다.
- fresh asset-free TargetSelect RuntimeContract process `19077228b82f4c37a1113c51fe18eaa7` 1/1 PASS와 Scanner Input=Pawn, Sensor runtime=SensorComp, 장비 apply=FittingComp, 선택=TargetSelect 책임 경계 정적 감사를 PASS로 판정했다.
- TargetSelect Input/HUD 저장 가능 테스트는 dirty Content 보호를 위해 실행하지 않았고 USER/Asset PASS로 확대하지 않았다.
- 다음 Gate는 `SCAN-P0-06 USER PIE Acceptance`이며 실제 `V` 조작과 Scanner 성능·Contact 보존은 사용자 직접 확인 전 PASS 처리하지 않는다.

Migration: CF-FQ-037 재개 시 `ScannerIntegrationPlan.md v0.7.0`의 P0-06부터 시작한다. P0-05까지의 기술 검증을 반복하지 않고 사용자 직접 PIE 체크리스트만 수행한다.

### v2.75 - 2026-08-16

- `CF-FQ-037 SCAN-P0-04 Fitting Integration`을 Technical Done으로 종료하고 대표 Plan을 `ScannerIntegrationPlan.md v0.6.0`으로 갱신했다.
- `ResolvedSensorData → ApplySensorData()`를 기존 Fitting Runtime Commit/Checkpoint/Compensation에 Sensor participant로 통합하고 scanner-less Fallback, Legacy Source 보존과 실패 복원을 확정했다.
- official Build `20ca14bdc6094650bc11209f4ce320fa` PASS와 FittingIntegration 1/1, Fitting 22/22, Sensor 14/14, Inventory 12/12 PASS를 P0-04 closure evidence로 보존한다.
- 다음 Gate를 `SCAN-P0-05 Technical Validation`으로 이동했다. P0-05는 위 검증을 반복하지 않고 asset-free TargetSelect 보호 회귀와 정적 책임 경계 감사를 추가한다.
- USER PIE, 실제 `V` 조작감과 탐지 체감은 P0-06에 유지하며 기존 CF-FQ-032/026/015/029/035 USER Pending·Paused 및 CF-FQ-030/034 Ready 상태를 변경하지 않았다.

Migration: CF-FQ-037 재개 시 `ScannerIntegrationPlan.md v0.6.0`의 P0-05부터 시작한다. P0-04 Build/Fitting/Sensor/Inventory evidence를 반복하지 않고 남은 TargetSelect 기술 보호 회귀와 Acceptance 감사만 수행한다.

### v2.74 - 2026-08-16

- `CF-FQ-037 SCAN-P0-03 Input Command Integration`을 Technical Done으로 종료하고 대표 Plan을 `ScannerIntegrationPlan.md v0.5.0`으로 갱신했다.
- P0 Active Scan 입력을 `/Game/CarFight/Input/IA_ActiveScan` Boolean + Pressed, `IMC_Vehicle_Default`의 `V` 단일 매핑으로 확정했다. `V` 1회 입력은 `ActiveScanDurationSec` 동안 실행된 뒤 Sensor Runtime에서 자동 종료된다.
- `ACFVehiclePawn`이 Enhanced Input bind와 Gameplay command 변환을 소유하고 `UCFVehicleSensorComp`의 직접 Input bind는 금지한다. 별도 Stop 키는 만들지 않고 `RequestStopActiveScan()`을 시스템·장비 전환용 command로 유지했다.
- official Build `b84e8dabcb784b30a005fb120af5cf4d` PASS, validation process `4d9a1e5ad12f416db18b1a069694c241` PASS, InputAsset 1/1 + InputCommand 1/1 + Sensor 14/14 + TargetSelect RuntimeContract 1/1 PASS를 closure evidence로 확정했다.
- persisted AssetDump에서 IA_ActiveScan boolean/consume/Pressed와 `IMC_Vehicle_Default` mapping 47개 중 `IA_ActiveScan <- V`를 확인했다.
- USER PIE PASS는 추가하지 않았으며 실제 입력 체감은 P0-06에 유지한다.
- 다음 Gate를 `SCAN-P0-04 Fitting Integration`으로 이동하고 실제 `ResolvedSensorData → VehicleSensorComp` 및 Field Fitting Sensor participant 통합을 P0-04에 유지했다.
- 기존 CF-FQ-032/026/015/029/035 USER Pending·Paused 및 CF-FQ-030/034 Ready 체크포인트는 그대로 보호했다.

Migration: CF-FQ-037 재개 시 `ScannerIntegrationPlan.md v0.5.0`의 P0-04부터 시작한다. P0-03의 IA_ActiveScan/V 단발 입력 계약을 변경하지 않고 P0-01 ResolvedSensorData를 P0-02 ApplySensorData에 연결한다.

### v2.73 - 2026-08-16

- `CF-FQ-037 SCAN-P0-02 Runtime Config Apply`를 Technical Done으로 종료하고 대표 Plan을 `ScannerIntegrationPlan.md v0.4.0`으로 갱신했다.
- `UCFVehicleSensorComp::ApplySensorData()`의 non-destructive hot reapply와 Applied Config 사본을 확정해 invalid 원자 거부, scanner-less fallback, Contact/Knowledge/Analysis 보존, range 감소 lifecycle reconcile과 Active Scan remaining 비증가를 적용했다.
- official Build `d670926be7c4498dbd15b12601f8505b` PASS, `CarFight.Scanner.SCAN_P0_02.ConfigApply` 1/1 PASS와 `CarFight.Sensor` 14/14 PASS를 closure evidence로 확정했다.
- Sensor Automation의 main Editor는 TestExit status 0 뒤 종료됐고 task-local outer wait 문제는 validation 결과와 분리했으며 finalizer `76b2577325a64cf2a3a0b64069474abe`에서 remaining CarFight Editor 0을 확인했다.
- Content Asset·Blueprint·InputAction·Scanner tuning·PIE mutation과 USER PASS 추가는 0이다.
- 다음 Gate를 `SCAN-P0-03 Input Command Integration`으로 이동하고 실제 FittingSnapshot/Field Fitting Sensor 통합은 `SCAN-P0-04`에 유지했다.
- 기존 CF-FQ-032/026/015/029/035 USER Pending·Paused 및 CF-FQ-030/034 Ready 체크포인트는 그대로 보호했다.

Migration: CF-FQ-037 재개 시 `ScannerIntegrationPlan.md v0.4.0`의 P0-03부터 시작한다. Sensor Component가 InputAction을 직접 Bind하지 않으며 P0-04 전에는 FittingSnapshot Source를 Vehicle Sensor에 연결하지 않는다.

### v2.72 - 2026-08-16

- `CF-FQ-037 SCAN-P0-00 Foundation Audit`을 read-only로 완료하고 대표 Plan을 `ScannerIntegrationPlan.md v0.2.0`으로 갱신했다.
- Enhanced Input owner=`ACFVehiclePawn`, Scanner 정적 성능 payload=`UCFVehicleSensorData`, Sensor Runtime owner=`UCFVehicleSensorComp` 책임 경계를 확정했다.
- `UCFVehicleData`에는 SensorData 참조가 없으며 `ECFVehicleMountType::Utility`는 존재하지만 현재 EquipmentPreset/Fitting 계약이 Turret+Weapon 완성을 강제해 Scanner를 그대로 표현할 수 없음을 확인했다.
- 다음 Gate를 `SCAN-P0-01 Scanner Data Contract`로 이동하고 기존 EquipmentPreset→FittingSnapshot 경로의 최소 SensorData payload 확장만 허용했다.
- `InitializeSensorRuntime()`의 destructive reinitialize 의미와 Field Fitting Sensor participant 부재를 기록하고 hot reapply/InputAction은 각각 P0-02/P0-03 이후로 분리했다.
- 기존 USER Pending/Paused/Ready 체크포인트와 작업 시작 전 dirty/untracked 상태를 그대로 보호했다.

Migration: CF-FQ-037 재개 시 `ScannerIntegrationPlan.md v0.2.0`의 P0-01부터 시작한다. 새 Scanner DataAsset·임의 tuning·InputAction asset은 만들지 않는다.

### v2.71 - 2026-08-16

- 사용자 선택에 따라 `CF-FQ-037 차량 스캐너 입력·장비 통합`을 새 단일 Active로 등록했다.
- 대표 Plan을 `Document/Plan/ScannerIntegrationPlan.md v0.1.0`, 첫 Gate를 Source/Asset mutation 없는 `SCAN-P0-00 Foundation Audit`으로 고정했다.
- 완료된 CF-FQ-036 SensorContact는 Done / Current System으로 유지하고 old SEN-P0 Gate를 재개하지 않는다.
- Scanner Input owner와 기존 SensorData/VehicleData/Fitting 재사용 가능성을 먼저 감사하며 새 DataAsset·임의 tuning·InputAction asset을 선제 생성하지 않는다.
- CF-FQ-032 Radar/TargetPanel USER Visual과 기존 Paused/Ready USER 체크포인트를 모두 보존했다.

Migration: 새 세션의 현재 작업은 `ScannerIntegrationPlan.md`에서 복원하며 Sensor Runtime 의미는 `Systems/Targeting/SensorContact.md`를 계속 우선한다.

### v2.70 - 2026-08-15

- `CF-FQ-036 SEN-P0-07 Technical Acceptance`를 PASS로 종료하고 `Document/Systems/Targeting/SensorContact.md v1.0.0`을 Current System으로 승격했다.
- 최종 Acceptance는 기존 official Build `7dff9da7aaa24e76b0871348762c9d93` PASS, `CarFight.Sensor` `85d008613a104df9b7107795995c6ac5` 14/14 PASS와 UI asset-free 1/1+1/1 PASS를 우선 evidence로 사용하고 Source 재구현·재빌드를 하지 않았다.
- actor-free public contract, Sensor/TargetSelect/HUD 책임 분리, lifecycle·Knowledge·DestroyedHold·Snapshot integration 정적 감사를 모두 PASS로 판정했다.
- CF-FQ-036을 Done으로 내리고 현재 단일 Active를 비웠다. Paused/Ready 기능은 사용자 선택 없이 자동 승격하지 않는다.
- CF-FQ-032 Radar/TargetPanel USER Visual, Radar Range/Zoom·동적 Blip, CF-FQ-026 TS-P0-08 USER PIE와 다른 USER Pending 체크포인트는 완료로 추정하지 않았다.

Migration: CF-FQ-036의 현재 구현은 `Systems/Targeting/SensorContact.md`를 우선하고 `SensorContactPlan.md`는 Historical + Retained Path 완료 기록으로 읽는다.

### v2.69 - 2026-08-15

- CF-FQ-036 `SEN-P0-06 Public Snapshot Integration`을 Technical Done으로 반영했다.
- TargetSelect는 선택 존재·유효성·TrackState owner를 유지하고 Target Knowledge/Radar Contact는 actor-free `FCFSensorSnapshot`만 Player-facing 의미 source로 사용하도록 HUD Provider/ViewData를 연결했다.
- Detected identity 비누출을 유지하기 위해 기존 Actor를 ContactId에만 연결하는 `TryGetContactIdForActor` read-only bridge를 추가했으며 HUD는 Actor metadata·현재 위치·private Runtime Contact를 직접 소비하지 않는다.
- Radar 실제 상대 위치·거리는 Snapshot에서 제공하되 Range/Zoom 계약이 없어 normalized position은 Unavailable로 유지하고 정적 Radar placeholder Canvas는 계속 숨긴다.
- 최종 official Build `7dff9da7aaa24e76b0871348762c9d93` PASS와 `CarFight.Sensor` Process `85d008613a104df9b7107795995c6ac5` 14/14 PASS를 대표 Plan evidence로 연결했다.
- 관련 UI asset-free 회귀 `ViewDataAvailability` `67d114255ec943bb8ded2642a40a581c` 1/1, `ProviderPawnRebind` `395c93df5fb947a1a979ca8fa0c74665` 1/1 PASS를 보존했다.
- 대표 Plan을 `SensorContactPlan.md v0.8.0`, 현재 Gate를 `SEN-P0-07 Technical Acceptance`로 이동했다.
- TargetSelect/CFVehiclePawn/Content Asset/WBP_TargetSelect와 CF-FQ-032·026 USER Pending 체크포인트는 변경하지 않았다.

### v2.68 - 2026-08-15

- CF-FQ-036 `SEN-P0-05 Destroyed Contact`의 authoritative destruction owner를 `UCFVehicleHealthComp`로 감사하고 Sensor가 `OnVehicleDestroyed / IsDestroyed`만 독립 소비하도록 고정했다.
- 기존 Sensor Contact를 같은 ContactId·Knowledge·AnalysisProgress·마지막 신뢰 위치를 보존한 DestroyedHold로 전환하고 `DestroyedHoldTimeSec` 만료를 bounded Actor cursor와 독립적으로 진행하도록 구현했다.
- weak Actor invalid/Actor Destroy만으로 파괴를 추정하지 않고 기존 LastKnown→Lost 경로를 유지했으며 pre-destroyed 미관측 Actor에서 새 Destroyed Contact를 생성하지 않도록 했다.
- official Build `954dc0ab844b40f48f2674f2a0ae672a` PASS와 `CarFight.Sensor` Process `4ba274cc90814e5b90d40c33820b3bd0` 13/13 PASS를 대표 Plan 증거로 연결했다.
- `SEN-P0-05`를 Technical Done으로 승격하고 대표 Plan을 `SensorContactPlan.md v0.7.0`, 현재 Gate를 `SEN-P0-06 Public Snapshot Integration`으로 이동했다.
- TargetSelect/HUD/CFVehiclePawn/Project Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았다.

### v2.67 - 2026-08-15

- 사용자 Editor 종료 후 official Build `786e47df68fe43f9a924c9c6fbd86726`을 실행해 NetCore/CarFight_Re DLL 최종 Link까지 Exit Code 0으로 PASS했다.
- 새 linked binary에서 `CarFight.Sensor` 전체 회귀 Process `6fcb3d2dbd5f42489febfff6040611af`가 11 Success / 0 Failure로 PASS했다.
- 기존 P0-01~03 8개와 신규 P0-04 ActiveRange/AnalysisDecay/AnalysisProgress 3개가 모두 PASS했고 TargetSelect/HUD/InputAction/DestroyedHold 금지 경계도 0으로 재확인했다.
- `SEN-P0-04 Active Scan Analysis`를 Technical Done으로 승격하고 대표 Plan을 `SensorContactPlan.md v0.6.0`, 현재 Gate를 `SEN-P0-05 Destroyed Contact`로 이동했다.
- TargetSelect/HUD/CFVehiclePawn/Project Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았다.

### v2.66 - 2026-08-15

- CF-FQ-036 `SEN-P0-04 Active Scan Analysis`의 Input/Scanner owner 감사를 완료하고 Sensor는 InputAction이 아니라 Start/Stop Runtime command API와 실행 상태·탐지·Analysis·Knowledge만 소유하도록 고정했다.
- Active Scan 실행 중 장거리 전방향 Contact Detection, ECC_Visibility 직접 가시 Tactical Analysis gain, invalid 상태 decay, Identified/DetailedScan Knowledge 승격 Source와 asset-free 3개 Automation을 구현했다.
- P0-03 ContactId/lifecycle/reacquire와 TargetSelect/HUD/Content/DestroyedHold 경계를 유지했고 `CFVehiclePawn`은 변경하지 않았다.
- 현재 정확한 Source official Build `d1cd746d5f3340e1bb1cfe43885cf5e2`은 UHT와 CFSensorTypes/ActiveScanTests/VehicleSensorComp/VehicleSensorData 및 기존 Sensor C++ compile까지 통과했으나 실행 중 `UnrealEditor.exe` DLL lock의 `LNK1104`로 Link 실패했다.
- 현재 Editor는 이번 세션 AI-owned evidence가 없어 자동 종료하지 않았고, P0-04를 Technical Done으로 승격하지 않았다.
- 현재 Gate를 `SEN-P0-04 Final Build + Sensor Regression`으로 유지하며 Editor 종료 후 official Build → `CarFight.Sensor` 11개 전체 회귀 순으로 재개한다.

### v2.65 - 2026-08-15

- CF-FQ-036 `SEN-P0-03 Contact Lifetime`을 Technical Done으로 반영했다.
- 탐지 상실 즉시 제거를 `Live → LastKnown → Lost → Removed`로 교체하고 LastKnown 위치·마지막 관측 시각을 마지막 신뢰 값으로 고정했다.
- `FreshnessSeconds / ContactMemoryTimeSec`을 bounded Actor cursor와 분리해 매 Sensor update 진행하고 Lost를 최소 한 Snapshot에 게시한 뒤 다음 update에서 제거하도록 했다.
- Lost 게시 전 동일 Actor 재획득 시 기존 ContactId와 Sensor Knowledge/AnalysisProgress를 유지하는 Live 복귀를 고정했다.
- 최종 `CFVehicleSensorComp.h v1.2.1` formatting-only 정리 후 정확한 Source에서 Build `fdd231a10ff648839b14b6a2468b295a` PASS와 `CarFight.Sensor` `45c96b20b2984984b753ff69c9b49d3f` 8/8 PASS를 대표 Plan 증거로 연결했다.
- TargetSelect/HUD/Project Config/Content Asset/DestroyedHold/Active Scan과 기존 USER Pending 체크포인트는 변경하지 않았다.
- 다음 Gate를 `SEN-P0-04 Active Scan Analysis`로 이동했다.

### v2.64 - 2026-08-15

- CF-FQ-036 `SEN-P0-02 Passive Detection`을 Technical Done으로 반영했다.
- Sensor private weak Actor Runtime Contact, Level Actor persistent cursor, `MaxActorScansPerUpdate` bounded budget과 Passive/Visual Detection을 적용했다.
- Passive 근거리 전방향 탐지는 LOS 없이, Passive 밖 Visual fallback은 `ECC_Visibility` 직접 가시성으로만 확인하며 TargetSelect Trace Channel을 사용하지 않는다.
- 동일 Actor 재관측에서 ContactId를 유지하고 Source Identified가 public Sensor Knowledge로 누출되지 않도록 `Detected` 시작 계약을 유지했다.
- 최종 Build `0e5a272bd3f047cf97574914cb9fce92` PASS와 `CarFight.Sensor` `288b8cc1c5224f4bbf57fbf7b23654cd` 6/6 PASS를 대표 Plan 증거로 연결했다.
- TargetSelect/HUD/Project Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았다.
- 다음 Gate를 `SEN-P0-03 Contact Lifetime`으로 이동했다.

### v2.63 - 2026-08-15

- CF-FQ-036 `SEN-P0-00 Foundation Audit`과 `SEN-P0-01 Contact Data Contract`를 Technical Done으로 반영했다.
- Sensor 독립 ContactId, Actor-free Snapshot, `ECFSensorContactState`, SensorConfig/DataAsset 클래스와 `UCFVehicleSensorComp` Pawn 기본 소유권을 적용했다.
- 공식 Build `e8c46e25c4aa464da7f9f07730dcba1a` PASS와 `CarFight.Sensor.SEN_P0_01` 3/3 PASS를 대표 Plan 증거로 연결했다.
- TargetSelect/HUD/Collision Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았다.
- 다음 Gate를 `SEN-P0-02 Passive Detection`으로 이동했다.

### v2.62 - 2026-08-15

- 사용자 결정에 따라 `CF-FQ-036 차량 센서·Contact Intelligence Runtime`을 새 단일 Active로 등록했다.
- 대표 Plan을 `SensorContactPlan.md v0.1.0`으로 연결하고 새 세션 첫 Gate를 Source mutation 없는 `SEN-P0-00 Foundation Audit`으로 고정했다.
- TargetSelect=선택 / Sensor=탐지·지식 / HUD=표시 책임 분리와 기존 USER Pending 체크포인트 보호 조건을 기록했다.
- 이번 인계 준비에서는 Source·Config·Content Asset·Build·PIE를 변경하거나 실행하지 않았다.

### v2.61 - 2026-08-15

- CF-FQ-008 WD-P0-03 Current System Integration을 완료하고 `Document/Systems/Combat/WeaponData.md v1.0.0`을 Current System으로 승격했다.
- `WeaponFire.md v1.6.0`의 Magazine/Reload 미구현 설명을 CF-FQ-031 Ammo Runtime의 현재 책임 경계로 교정했다.
- CF-FQ-008을 Done / Historical + Retained Path로 전환하고 현재 단일 Active를 비웠다.
- CF-FQ-015·026·032 등 기존 USER Pending 체크포인트는 그대로 보호하며 새 Feature를 자동 Active로 선택하지 않는다.

### v2.60 - 2026-08-15

- CF-FQ-008 WD-P0-01 Static Data Contract와 WD-P0-02 Representative Assets를 Technical Done으로 반영했다.
- 최종 Build `53e2dbaf04a3401b8ed89c906b308cdc` PASS와 `CarFight.WeaponData` `eefe58aa87d74dcaa79a1764a5f7611b` 2/2 PASS를 기록했다.
- 대표 WeaponData는 Load-only로 검증했고 Content Asset mutation은 0이다.
- WD-P0-03 Current System Integration은 다음 단계로 유지하고 아직 시작하지 않았다.

### v2.59 - 2026-08-15

- 원격 개발 전략을 계속해 CF-FQ-015는 VD-P0-00~03 Remote Technical Done / VD-P0-04 USER Tuning Pending 체크포인트가 보존된 Paused로 전환했다.
- `CF-FQ-008 무장 데이터 정의`를 단일 Active로 승격하고 `WeaponDataPlan.md v0.1.0`을 대표 Plan으로 등록했다.
- 기존 UCFWeaponData를 재설계하지 않고 WD-P0-01 정적 DataValidation부터 진행한다.

### v2.58 - 2026-08-15

- CF-FQ-015 VD-P0-00~03을 Remote Technical Done으로 반영했다.
- 공식 Build `0cffed2f02674b6692d4f8af811d9036` PASS와 `CarFight.VehicleData` `59091b9559db46859c3a521be72396bf` 3/3 PASS를 기록했다.
- UCFVDAValidator 현재 계약, Representative Compare와 실제 ACFVehiclePawn Runtime Apply 계약을 기술 검증했으며 VehicleData Content Asset 값은 변경하지 않았다.
- 남은 VD-P0-04 USER Tuning은 PIE 가능 시점까지 Pending으로 보존한다.

### v2.57 - 2026-08-15

- 사용자가 PIE를 볼 수 없는 기간의 원격 작업으로 `CF-FQ-015 차량 데이터 튜닝 패스`를 단일 Active로 전환하고 `CF-FQ-026 TargetSelect`는 기존 TS-P0-08 USER PIE 체크포인트가 보존된 Paused로 이동했다.
- `VehicleDataTuningPlan.md v0.1.0`을 대표 Plan으로 등록하고 VD-P0-00 Foundation Audit을 완료했다.
- DA_TestSedan/DA_TestSUV와 VehicleData 적용 경로를 read-only 감사했으며 실제 주행 수치는 변경하지 않았다.
- next gate를 `VD-P0-01 Validator Contract`로 고정했다.

### v2.56 - 2026-08-15

- TS-P0-08 최신 Build `806a8dba54434dc38b1b83baa310c50c` PASS와 targeted `eeaa81f44b504d33be091a2116868ac7` 3/3 PASS를 반영했다.
- 저장 BP CDO의 실제 fallback 설정 경로와 TargetPoint `SM_Body` 자동 정렬을 Load-only Automation으로 확정했다.
- 런타임 검색 진단과 proximity 밖 비-Direct 대상의 LOS Trace만 생략하는 의미 보존 사전필터를 반영했다. 전체 Actor 20Hz 순회는 남은 scalability 항목이다.
- USER 범위 체감·debug 가시성·HUD 겹침·동일 차량 재검증은 PASS로 승격하지 않았다.

### v2.55 - 2026-08-15

- 사용자 지정에 따라 단일 Active를 `CF-FQ-026 TargetSelect / TS-P0-08`로 전환하고 CF-FQ-035는 기존 USER Field UI·Mobility 체크포인트가 보존된 Paused로 이동했다.
- TargetSelect 공식 Build PASS, TS-P0-08 SingleTargetBoundary 1/1과 자산 비변경 TS-P0-01·02·03·04·07 각 1/1 PASS를 기록했다.
- debug Sphere 표시 수명 교정과 Debug 반경 데이터 분리를 반영했지만 실제 시각성·범위 체감·HUD 겹침·동일 차량 USER 재검증은 Pending으로 유지한다.
- 선행 dirty `WBP_TargetSelect.uasset`를 보호하고 이번 원격 작업에서 Content Asset 저장·덮어쓰기를 수행하지 않았다.

### v2.54 - 2026-08-15

- `FIT-P0-06 Fitting ViewData and Debug`의 C++ ViewData·Blueprint Contract를 Technical Done으로 반영했다.
- Build `ad3452d3add74785bbdb41c667dce728`, FIT-P0-06 1/1, Fitting 22/22, Inventory 12/12, Full CarFight 84/84 Success를 현재 증거로 갱신했다.
- CF-FQ-035/034의 현재 원격 기술 선행 Gate는 완료됐고 남은 범위를 Field UI·실제 화면 가독성·Light/Default/Heavy Mobility·USER 실제 사용 검증으로 제한했다.
- 새로운 Feature Active 전환은 사용자 선택 없이 자동 수행하지 않는다.
- CF-FQ-032 USER Visual Paused 체크포인트는 그대로 보호한다.

### v2.53 - 2026-08-15

- cross-feature `FFIT-P0-01~04` Permission·Timed Action·Atomic completion·Chaos Field Mass Reapply를 Technical PASS로 반영했다.
- 실제 ChaosMassPIE에서 +50kg Configured/Actual Mass 반영과 원래 질량 복원, Physics State·Transform·Runtime Ready 보존을 확인했다.
- 최종 Build `cd7207084dfd49c4a28b607d8577307a`, FFIT-P0-04 4/4, Fitting 21/21, Inventory 12/12, Full CarFight 83/83 Success를 현재 증거로 갱신했다.
- 다음 원격 기술 Gate를 `FIT-P0-06 Fitting ViewData and Debug`로 이동했다.
- CF-FQ-032 USER Visual과 CF-FQ-034 Light·Default·Heavy Mobility/Field UI USER 검증은 자동 PASS 처리하지 않고 그대로 보존한다.

### v2.52 - 2026-08-14

- `INV-P0-06 / M6` Coordinator Foundation의 원자 completion과 failure compensation을 구현·검증했다.
- 최종 Build `e2ef556b64484a09ba8b3544c62344e3`, M6 3/3, Inventory 12/12, Full CarFight 71/71 Success를 현재 증거로 갱신했다.
- M6 전체 Done이나 formal FFIT-P0-03 완료 처리는 하지 않는다. 실제 FittingComp는 Field Mass Reapply 전 same-mass 후보만 허용한다.
- 현재 next gate를 cross-feature `FFIT-P0-01 Permission and Blocker Query`로 이동했다.
- CF-FQ-032 USER Visual Paused 체크포인트는 그대로 보존한다.

### v2.51 - 2026-08-14

- 사용자가 직접 화면을 확인할 수 없는 기간 동안 기술 개발을 계속하기 위해 `CF-FQ-032`의 USER Visual 체크포인트를 Paused로 보존하고 `CF-FQ-035`를 단일 Active로 전환했다.
- `INV-P0-05 / M5`를 읽기 전용 Inventory Snapshot·ChangeSet으로 완료했다.
- 공식 Build `30503595ba074c63ba8a6bb87f7a2645`, Inventory 9/9, 전체 CarFight 68/68 Success를 현재 Active 증거로 기록했다.
- 현재 next gate는 `INV-P0-06 / M6 Integration Verification`이며 Field Fitting Coordinator의 기술 통합부터 진행한다.
- CF-FQ-032 Defense/Pawn Rebind USER Visual은 취소·완료 처리하지 않고 재개 지점을 그대로 보호한다.

### v2.50 - 2026-08-14

- `CFHUDDataTests v1.6.0`으로 실제 Defense Runtime→ViewData, 저장 맵 PIE Defense ViewData와 Pawn Rebind Old Pawn 이벤트 해제를 자동 회귀화했다.
- 최종 공식 Build `681c91810da34066bb398ad1b0989f1e` 성공, targeted `CarFight.UI.UI_P0_03` Automation `f6b08a970bbf4ec282893e86e92e72ac` 5/5 Success·0 Fail을 기록했다.
- 기술 검증은 사용자 시각 확인과 분리하고 current next gate를 `Defense Production Panel USER Visual → Pawn Rebind USER Visual`로 좁혔다. UI-P0-03은 아직 완료하지 않는다.
- 대표 Plan을 `InGameUIPlan.md v0.58.5`로 동기화했다.

### v2.49 - 2026-08-14

- `/Game/Maps/TestMap_AmmoSalvo` 격리 fixture에서 Launcher Presentation USER PIE 5/5 PASS를 기록했다.
- 기존 `TestMap_DRSalvo`는 Ammo Runtime 도입 전 무한탄 Launcher 회귀 맵이라 UI finite Ammo 검증 대상에서 제외하고 기존 회귀 의미를 유지한다.
- `CF-FQ-032` 대표 Plan을 `InGameUIPlan.md v0.58.4`로 동기화하고 next gate를 Defense 실제 Shield·Armor·Integrity 변화 USER PIE → Pawn Rebind로 이동했다.
- Source 변경이 없어 기존 Official Build와 Full Automation 64/64 PASS를 재실행하지 않았다.

### v2.48 - 2026-08-13

- UI-P0-03 정적 구조 감사를 수행해 Provider Rebind·Old Pawn/Timer 해제와 Presenter 공통 Launcher lifecycle에 현재 Blocking 구조 결함이 없음을 확인했다. Source·Unreal Asset은 변경하지 않았다.
- UI-P0-04 AimReticle 통합은 `InGameUIPlan.md v0.58.1`에 사전 설계만 준비했다. 기존 HUD Layer 재사용, UISubsystem 단일 수명, Pawn direct Viewport 생성과 병행 금지, Weak Pawn Binding, 기존 `WBP_AimReticle` Visual 재사용이 핵심 경계이며 실제 UI-P0-04는 아직 Not Started다.
- next gate는 변경 없이 `TestMap_DRSalvo` USER PIE → Launcher USER PASS 후 Defense 실제 변화 → Pawn Rebind다. 코드 변경이 없어 기존 Build·64/64 Automation을 재실행하지 않는다.

### v2.47 - 2026-08-13

- Salvo 전용 v1.5.0·v1.6.0 시간 Hold를 폐기하고 `LauncherSequenceRevision` 기반 공통 Weapon/Launcher Presentation lifecycle 교정을 실제 코드에 적용했다.
- HeavyCannon SingleCycle은 공통 Weapon Status, Ripple·Salvo는 동일한 Active→terminal Snapshot→Cooldown→READY 전이를 사용하며 FirePattern을 수명 정책 분기에서 제거했다.
- 공식 Editor Build `6978e029bfaa48c7addfc9acd87484b1` PASS, 전체 Automation `00ee23a01aa7480cabc557f312780ae8` 64/64 PASS·필수 32/32 Success·실패 0을 기록했다.
- `TestMap_DRSalvo` USER PIE는 Ready/Pending이며 USER 확인 전 UI-P0-03/CF-FQ-032 완료 처리를 금지한다.

### v2.46 - 2026-08-13

- 1,700줄 이상 누적된 완료 작업·Build·Automation·USER PIE·historical override 복제를 제거하고 ActiveWork를 세션 복원 projection으로 축약했다.
- `CF-FQ-032 / InGameUIPlan v0.57.0`을 단일 Active로 동기화하고 Salvo 전용 Hold 폐기 후 공통 Weapon/Launcher Presentation lifecycle 교정을 next gate로 고정했다.
- Paused `CF-FQ-029`, `CF-FQ-026`만 체크포인트로 유지하고 Ready 기능은 FeatureQueue가 소유하도록 중복을 제거했다.
- 완료 기능은 Systems와 `Document/Plan/Archive/README.md` Historical route로 내렸다.

Migration: 상세 작업 evidence는 삭제된 것이 아니라 각 대표 Plan과 Systems에 유지된다. 현재 세션 복원은 이 문서에서 work_id·대표 Plan·next gate만 선택한 뒤 representative owner를 읽는다.