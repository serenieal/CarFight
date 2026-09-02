# CarFight — 03_FeatureQueue

> 문서 버전: v1.56.62
> 최근 갱신일(Asia/Seoul): 2026-09-02
> 문서 상태: Current
> 역할: CarFight의 **Feature 후보 / 착수 판단 / 현재 상태 / 완료 후 Current owner**를 한 곳에서 관리한다.

---

## 1. 목적과 경계

FeatureQueue는 다음 질문에 답한다.

```text
- 어떤 Feature가 존재하는가?
- 우선순위와 상태는 무엇인가?
- 지금 재개한다면 어디서 시작하는가?
- 완료된 구현은 어느 Systems 문서가 소유하는가?
```

FeatureQueue는 상세 설계서나 검증 로그가 아니다.

```text
후보/상태 → FeatureQueue
상세 설계·Build·Automation·PIE·USER evidence → 대표 Plan
현재 구현 → Systems
세션 복원 → ActiveWork
```

---

## 2. 상태 정의

| 상태 | 의미 |
| --- | --- |
| `Candidate` | 아직 착수하지 않은 후보 |
| `Ready` | 선행 조건이 준비됐지만 현재 Active가 아님 |
| `Active` | 현재 진행 중인 단일 주력 Feature |
| `Paused` | 완료 체크포인트를 보존하고 일시중지 |
| `Blocked` | 외부 선행 조건 때문에 진행 불가 |
| `Done` | 현재 범위 완료, Systems 승격 또는 Current 계약 반영 완료 |
| `Deferred` | 현재 사이클에서 보류 |
| `Rejected` | 기각 |

현재 단일 `Active`는 **`CF-FQ-039 Production UI Visual Rework`**다.

---

## 3. 우선순위 정의

| 우선순위 | 의미 |
| --- | --- |
| `P0` | 다음 개발/검증을 직접 막는 핵심 기반 |
| `P1` | 핵심 게임 루프에 필요한 가까운 작업 |
| `P2` | 생산성·운영·품질 개선 |
| `P3` | 편의·폴리싱 |
| `Icebox` | 장기 아이디어/멀티플레이 보류 |

---

## 4. Feature Queue

| ID | Feature | Pri | 상태 | 현재 checkpoint / 비고 | 완료 후 Current owner |
| --- | --- | --- | --- | --- | --- |
| `CF-FQ-001` | 서버 권한 발사 요청 | Icebox | Deferred | 현재 싱글 범위 제외 | Network/Combat 후속 |
| `CF-FQ-002` | 조준/발사 피드백 분리 | Icebox | Deferred | 로컬 피드백은 FQ-017로 대체 | 현재 FQ-017 Systems |
| `CF-FQ-003` | 체력/대미지 최소 구조 | P2 | Deferred | FQ-018/FQ-033으로 실질 이관 | HitDamage / VehicleDefense |
| `CF-FQ-004` | 리스폰 최소 구조 | Icebox | Deferred | 멀티플레이 보류 | Network/Respawn 후속 |
| `CF-FQ-005` | 전투 결과 기록 | Icebox | Deferred | 현재 싱글 핵심 루프 이후 | Combat/MatchResult 후속 |
| `CF-FQ-006` | 테스트 계정/상태 초기화 도구 | Icebox | Deferred | 서버 운영 범위 보류 | Admin/TestReset 후속 |
| `CF-FQ-007` | 차량 로드아웃 저장 | Icebox | Deferred | 장기 저장/서버 범위 보류 | Data/VehicleLoadout 후속 |
| `CF-FQ-008` | 무장 데이터 정의 | P2 | Done | Current WeaponData 계약 유지 | `Systems/Combat/WeaponData.md v1.1.0` |
| `CF-FQ-009` | 운영 로그 조회 기준 | Icebox | Deferred | 서버 운영 범위 보류 | Admin/LogView 후속 |
| `CF-FQ-010` | 세션/로비 기초 | Icebox | Deferred | Dedicated Server 이후 | Network/Session 후속 |
| `CF-FQ-011` | 싱글 실행 기준선 전환 | P0 | Done | 현재 싱글 기준선 | ProjectRuntimeConfig / VehicleRuntime |
| `CF-FQ-012` | 1대 차량 주행감 고도화 | P1 | Candidate | 사용자 선택 시 착수 | VehicleDrive / VehicleSteering |
| `CF-FQ-013` | 카메라/로컬 조준 고도화 | P0 | Done | 현재 로컬 Aim 기반 | VehicleCamera / VehicleAim / AimReticle |
| `CF-FQ-014` | WheelSync 시각 품질 폴리싱 | P1 | Candidate | 사용자 선택 시 착수 | WheelSync |
| `CF-FQ-015` | 차량 데이터 튜닝 패스 | P1 | Paused | `VD-P0-04 USER Tuning Pending` | VehicleData |
| `CF-FQ-016` | 차량 무기 조준 및 발사 | P0 | Done | 완료 기반 | WeaponFire / VehicleAim |
| `CF-FQ-017` | Reticle / FireFeedback UI | P0 | Done | 완료 기반 | FireFeedback / AimReticle |
| `CF-FQ-018` | 피격 판정 및 피해 처리 | P0 | Done | 완료 기반 | HitDamage |
| `CF-FQ-019` | 주행/전투 반복 테스트 | P1 | Deferred | 런처·미사일 이후 통합 회귀 범위 재설계 | TestChecklist / CoreLoop 후속 |
| `CF-FQ-020` | 조작감/전투 템포/피드백 개선 | P1 | Candidate | 사용자 선택 시 착수 | CombatFeel 후속 |
| `CF-FQ-021` | 핵심 게임 루프 검증 | P1 | Candidate | 사용자 선택 시 착수 | CoreLoop / TestChecklist |
| `CF-FQ-022` | 조준점·터렛·총구 정렬 | P0 | Done | 완료 기반 | VehicleAim / WeaponFire / AimReticle |
| `CF-FQ-023` | 고속 Projectile 연속 충돌 | P0 | Done | 완료 기반 | Projectile / DamageHitContext |
| `CF-FQ-024` | 전투 FX 구현 | P0 | Done | USER PIE PASS / 게임 사운드 제외 | CombatFx |
| `CF-FQ-025` | 이중 레티클 및 터렛방향 시각화 | P0 | Done | 완료 기반 | AimReticle / VehicleAim |
| `CF-FQ-026` | 타겟 선택 시스템 | P1 | Paused | `TS-P0-08 USER PIE Pending` | TargetSelect 관련 Current 후속 |
| `CF-FQ-027` | 투사체 비행 FX | P1 | Done | USER PIE PASS / CF-TC-023 PASS | Projectile |
| `CF-FQ-028` | 발사체 추진 시스템 | P1 | Done | USER PIE PASS / CF-TC-024 PASS | Projectile |
| `CF-FQ-029` | 모듈형 런처 및 발사 인계 | P1 | Paused | `LauncherMissilePlan.md v0.14.0` / `LM-P0-06 USER PIE` | Launcher / WeaponFire / Projectile 후속 |
| `CF-FQ-030` | 물리 제한형 미사일 비행·유도 | P1 | Ready | Persisted Technical PASS / `CF-TC-027 Manual PIE Pending` | Missile / Projectile 후속 |
| `CF-FQ-031` | 차량 탄약·재장전 런타임 | P1 | Done | AMMO-P0-00~08 + USER PIE PASS | `Systems/Combat/Ammo.md v1.0.0` |
| `CF-FQ-032` | 인게임 전투 HUD 및 UI 프레임워크 | P1 | Done | UI-P0-11 Systems Promotion + 2026-08-22 post-closure remediation PASS. Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 비차단 Deferred/Pending | `Systems/UI/InGameUI.md v1.1.5`, `UI/AimReticle.md v1.10.0`, `Targeting/SensorContact.md v1.2.0` |
| `CF-FQ-033` | 차량 방어·손상 런타임 | P0 | Done | DR-P0-00~07 + USER PIE PASS | VehicleDefense / HitDamage |
| `CF-FQ-034` | 차량 피팅·질량 런타임 | P1 | Paused | `Document/Plan/VehicleFitting/VehicleFittingPlan.md v0.17.0` / `FIT-P0-07D USER Driving Feel Comparison` | VehicleFitting / VehicleData / VehicleRuntime 후속 |
| `CF-FQ-035` | 인벤토리 Foundation | P1 | Paused | Technical checkpoint 보존 / USER Field UI·Mobility Pending | InventoryFoundation + 관련 Systems 후속 |
| `CF-FQ-036` | 차량 센서·Contact Intelligence Runtime | P1 | Done | Sensor Runtime 완료, Scanner와 Current 통합 | `Systems/Targeting/SensorContact.md v1.2.0` |
| `CF-FQ-037` | 차량 스캐너 입력·장비 통합 | P1 | Done | SCAN-P0-00~07 / USER PIE PASS | `Systems/Targeting/SensorContact.md v1.2.0` |
| `CF-FQ-038` | 차량 데이터 Authoring 시스템 | P2 | Paused | `Document/Plan/DataAuthoring/DataAuthoringPlan.md v0.2.52` / `Document/Plan/DataAuthoring/DataAuthoringRoadmap.md v0.1.56` / Deprecated transition Technical Complete / `Systems/Vehicles/VehicleBuilder.md v1.0.0` 기준 Builder Backend + Advanced Workspace 역할 고정 / P0-12 USER PASS 7/8 / UA-08 quantitative comparison Deferred / DEL6 compatibility retirement Pending | 완료 시 VehicleDataAuthoring 신규 Systems 후보 |
| `CF-FQ-039` | Production UI Visual Rework | P1 | Active | `Document/Plan/InGameUIVisual/InGameUIVisualPlan.md v0.1.29` / `VPR-P0-01 VT07 VehiclePanel Asset-First whole-panel Review Ready / USER Visual PASS Pending / UE Import 0 / Production Asset mutation 0` | 완료 시 `Systems/UI/InGameUI.md` Visual ownership 갱신 + Production UI Asset 기준 |
| `CF-FQ-040` | Guided Vehicle Builder | P2 | Done | VB-P0-10 Current System Promotion Complete / VB-P0-09 End-to-End USER Acceptance PASS / WSA P0 Complete / ESH-01~06 Final Audit Clean PASS / representative Plan은 Historical + Retained Path | `Systems/Vehicles/VehicleBuilder.md v1.0.0` |
| `CF-FQ-041` | 런타임 콘텐츠 적용 메뉴 | P2 | Ready | `Document/Plan/RuntimeApply/RuntimeApplyPlan.md v0.1.5` / `RTA-P0-01 Technical PASS` / next `RTA-P0-02 Vehicle Runtime Apply` / Hard Reference Runtime Catalog+bounded Cook+actual runtime load PASS / Builder Step 8 same-Pawn Vehicle reinitialize 재사용 / Equipment post-apply seam Pending | 완료 시 Runtime Apply 재사용 계약을 관련 Vehicle/Fitting/UI Systems에 승격 |

---

## 5. 현재 재개 후보

현재 Active는 `CF-FQ-039 / VPR-P0-01 VT07 VehiclePanel Asset-First whole-panel Review Ready / USER Visual PASS Pending`이다.

아래 항목은 현재 Active를 자동 대체하지 않는 **Paused/Ready 재개 후보**다.

| Feature | 상태 | 정확한 다음 Gate |
| --- | --- | --- |
| `CF-FQ-030` | Ready | `CF-TC-027 Manual PIE` |
| `CF-FQ-041` | Ready | `RTA-P0-02 Vehicle Runtime Apply` |
| `CF-FQ-029` | Paused | `LM-P0-06 USER PIE` |
| `CF-FQ-038` | Paused | non-blocking `DEL6 compatibility retirement` 또는 `UA-08 quantitative comparison Deferred` |
| `CF-FQ-034` | Paused | `FIT-P0-07D USER Driving Feel Comparison` |
| `CF-FQ-035` | Paused | USER Field UI·Mobility |
| `CF-FQ-026` | Paused | `TS-P0-08 USER PIE` |
| `CF-FQ-015` | Paused | `VD-P0-04 USER Tuning` |

Candidate는 `CF-FQ-012`, `014`, `020`, `021`이다.

---

## 6. 상태 변경 규칙

Feature 상태가 바뀔 때만 이 문서를 갱신한다.

```text
Candidate → Ready/Active
Ready → Active
Active → Paused/Done/Blocked
Paused → Active/Done
Done → 원칙적으로 유지; 실제 회귀 결함이 Feature 재개를 요구할 때만 별도 판단
```

Build ID, Automation UUID, 개별 AssetDump generation, 긴 RCA, USER가 누른 버튼 순서는 FeatureQueue에 기록하지 않는다. 그 정보는 대표 Plan이 소유한다.

Feature가 Done되면 Current System 링크와 남은 Deferred/Pending 경계만 기록한다.

---

## 7. Changelog

### v1.56.62 - 2026-09-02

- `CF-FQ-040 / VB-P0-10 Current System Promotion`을 완료해 상태를 P2 / Done으로 전환하고 Current owner를 `Systems/Vehicles/VehicleBuilder.md v1.0.0`으로 고정했다.
- CF-FQ-040을 Ready 재개 후보에서 제거했다. 대표 VehicleBuilder Plan/Roadmap은 semantic Historical + Retained Path로 전환하며 물리 이동은 수행하지 않는다.
- `CF-FQ-038`은 Done으로 확대하지 않고 Paused를 유지하되, 현재 역할을 Vehicle Builder가 소비하는 Backend + 전문가용 Advanced Workspace로 명시적으로 고정했다.

### v1.56.61 - 2026-09-02

- CF-FQ-040 ESH final audit correction 완료를 반영했다. high-speed TargetHash는 benchmark 전 actual saved Target과 exact 검증되며 stale hash는 fail-closed한다.
- representative owner를 VehicleBuilderPlan v0.1.45 / Roadmap v0.1.37로 동기화하고 ESH-01~06 Final Audit Clean PASS를 기록했다. P2/Ready 및 next VB-P0-10 상태는 유지한다.

### v1.56.60 - 2026-09-02

- CF-FQ-040 actual Wagon의 ESH-06 USER Driving PASS와 VB-P0-09 End-to-End USER Acceptance PASS를 반영했다. representative owner는 VehicleBuilderPlan v0.1.44 / Roadmap v0.1.36이다.
- next Gate는 VB-P0-10 Current System Promotion이다. CF-FQ-040은 P2/Ready를 유지하고 promotion 완료 전 Done으로 확대하지 않는다. current Active CF-FQ-039는 변경하지 않았다.

### v1.56.59 - 2026-09-02

- CF-FQ-040 current owner를 VehicleBuilderPlan v0.1.42 / Roadmap v0.1.34 / WheelSizeAuthorityPlan v0.1.19로 동기화했다.
- USER-approved 5500 fixed-common Target Apply와 ESH-05 persisted no-override retest Technical PASS를 반영했다. CF-FQ-040은 P2/Ready를 유지하고 next Gate만 ESH-06 USER Driving PASS로 전진한다. current Active CF-FQ-039는 변경하지 않았다.

### v1.56.58 - 2026-09-01

- `CF-FQ-041 / RTA-P0-01` post-review의 Catalog 수동 편집성 교정과 최종 Technical 재검증 PASS를 반영해 대표 Plan을 `RuntimeApplyPlan.md v0.1.5`로 갱신했다.
- Feature 상태는 P2/Ready, next `RTA-P0-02 Vehicle Runtime Apply` 그대로이며 current Active CF-FQ-039 및 Builder 상태는 변경하지 않았다.

### v1.56.57 - 2026-09-01

- `CF-FQ-041 / RTA-P0-01`을 Technical PASS로 전진하고 대표 Plan을 `RuntimeApplyPlan.md v0.1.4`로 갱신했다.
- 등록형 Runtime Catalog/Settings, `/Game/CarFight/Debug` bounded Cook, persisted Vehicle 3 / Equipment 2 hard refs와 Config 기반 actual runtime load Automation 1/1 PASS를 반영했다.
- next Gate는 `RTA-P0-02 Vehicle Runtime Apply`이며 CF-FQ-041은 P2/Ready를 유지한다. 현재 단일 Active CF-FQ-039와 Builder 작업 상태는 변경하지 않았다.

### v1.56.56 - 2026-09-01

- `CF-FQ-041 / RTA-P0-00` read-only 감사 PASS를 반영하고 대표 Plan을 `RuntimeApplyPlan.md v0.1.3`으로 갱신했다.
- 전체 Asset 자동 검색 대신 Hard Reference Runtime Catalog를 P0 Selection Source로 고정하고 next Gate를 `RTA-P0-01 Runtime Catalog / Packaged Load Contract`로 전진했다.
- Equipment Runtime은 기존 Fitting/Chaos Mass 계약을 재사용하되 Ammo/Turret/Launcher 후처리 동기화 seam이 필요함을 확인했다. CF-FQ-041은 P2/Ready, Implementation 0을 유지하며 현재 Active CF-FQ-039는 변경하지 않았다.

### v1.56.55 - 2026-09-01

- USER 승인으로 `CF-FQ-041 런타임 콘텐츠 적용 메뉴`를 P2 / Ready로 신규 등록했다. 대표 Plan은 `Document/Plan/RuntimeApply/RuntimeApplyPlan.md v0.1.0`이다.
- Existing VehicleData / EquipmentPresetData를 PIE 및 시연용 Packaged Build에서 선택·명시 Apply하는 범위, 기존 VehicleDebug UI 재사용, Fitting/Inventory Runtime 재사용 경계와 Scope Out을 고정했다.
- 첫 Gate는 read-only `RTA-P0-00 Current Runtime Apply Contract Audit`이며 구현은 아직 0이다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v1.56.54 - 2026-08-29

- CF-FQ-040 actual Wagon의 USER 부재 AI-only preparation 완료와 최신 owner를 current projection에 반영했다. VehicleBuilderPlan/Roadmap v0.1.25, ShellSpec v0.1.14, WheelSizeAuthorityPlan v0.1.11이 현재 기준이다.
- WSA-P0-06 Technical PASS / WSA-P0-07 USER Acceptance Ready를 반영하고 과거 `WSA-P0-01` current next projection을 제거했다. actual Wagon ResearchDraft는 consistency PASS지만 persistent UE Asset/Profile/Recipe/Target/Save mutation은 추가 0이다.
- 정확한 다음 Gate는 Step 1 USER Reference Review이며 이후 Companion → Step 5/7 → USER Save → Technical Driving/WSA-P0-07 → driving feel approval 순이다. CF-FQ-040 P2/Ready와 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.53 - 2026-08-28

- CF-FQ-040 actual Wagon E2E Guided UX remediation Technical PASS와 대표 owner Plan/Roadmap v0.1.24, ShellSpec v0.1.13을 current projection에 반영했다.
- USER UI recheck/실제 Wagon Recipe WheelMesh 반영은 Pending이다. P2/Ready, WSA-P0-01 next gate, 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.52 - 2026-08-28

- CF-FQ-040 Wheel Size Authority final Source audit PASS와 최신 owner 버전을 반영했다.
- next는 WSA-P0-01 code/schema/helper + resolver/guide ownership guard 구현이며 canonical asset/기존 Step1~8 Technical PASS는 보존한다.
- Feature 우선순위/Ready 상태, 실제 USER E2E Pending, 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.51 - 2026-08-28

- CF-FQ-040 Wheel Size Authority를 v0.1.3으로 재감사하고 shared Wheel_FL canonical `100×25×100`, center≈0 live PASS를 current checkpoint에 반영했다.
- 대표 owner를 VehicleBuilderPlan/Roadmap v0.1.22, ShellSpec v0.1.11로 동기화했다. next WSA-P0-01은 Asset normalization이 아니라 code/schema/helper 구현이다.
- CF-FQ-040 P2/Ready, 기존 Step1~8 Technical PASS, 실제 신규 차량 USER E2E Pending, 단일 Active CF-FQ-039는 그대로다.

### v1.56.50 - 2026-08-28

- CF-FQ-040에 `WheelSizeAuthorityPlan.md v0.1.1`을 정식 하위 Plan으로 등록하고 WSA-P0-00 Design PASS를 current checkpoint에 반영했다.
- 대표 owner를 VehicleBuilderPlan/Roadmap v0.1.21, ShellSpec v0.1.10으로 전진하고 next gate를 `WSA-P0-01 Canonical Default Wheel + Data Schema & Shared Size Utility`로 변경했다.
- 실제 신규 Wagon E2E와 기존 Step1~8 Technical PASS 상태는 보존하며 CF-FQ-040 P2/Ready, P0-10 Systems promotion, 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.49 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 8 Technical Driving + USER Driving Guided Flow` Technical PASS를 current checkpoint에 반영했다. 상세 evidence는 대표 Plan/Roadmap v0.1.20, ShellSpec v0.1.9가 소유한다.
- existing VB-P0-08 fixed-60Hz benchmark를 saved Target exact path + Target DefinitionHash + fresh RunId에 binding하고, actual PIE transient selected-vehicle apply 뒤에만 explicit USER Driving PASS를 허용한다. auto Save/Reference threshold 자동 판정/USER feel 자동 판정은 0이다.
- next gate를 실제 신규 차량 1대 Guided Builder E2E + USER Driving Acceptance로 전진했다. CF-FQ-040 P2/Ready, P0-10 Systems promotion, 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.48 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 7 Final Review / explicit Apply / guarded Undo Guided Flow` Technical PASS를 current checkpoint에 반영했다. 상세 evidence는 대표 Plan/Roadmap v0.1.19, ShellSpec v0.1.8이 소유한다.
- existing `ReadBuilderFinalReview` R0 full review를 Guided Shell에 연결하고 USER-approved exact ProposalHash만 R3 DefinitionApply에 전달하며 successful Apply의 exact guarded Undo token만 current lifetime에서 사용한다.
- next implementation gate를 `Step 8 Technical Driving + USER Driving Guided connection`으로 전진했다. USER E2E Pending, CF-FQ-040 P2/Ready, P0-10 Systems promotion, 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.47 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 6 Gameplay Setup Guided Flow` Technical PASS를 current checkpoint에 반영했다. 대표 Plan/Roadmap v0.1.18, ShellSpec v0.1.7이 detailed evidence를 소유한다.
- existing `ReadBuilderGameplayGuidance` R0의 8영역 completeness와 USER Socket guidance를 Guided Shell에 연결하고 Pending Gameplay Diff Apply는 Step 7 owner로 유지했다.
- next implementation gate를 `Step 7 Final Review / explicit Apply Guided connection`으로 전진했다. USER E2E Pending, CF-FQ-040 P2/Ready, P0-10 Systems promotion, 단일 Active CF-FQ-039는 변경하지 않았다.
- v1.56.46에서 반영된 동시 CF-FQ-039 VT07 current projection은 보존했다.

### v1.56.45 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step 5 Physics Proposal Guided Flow` Technical PASS를 current checkpoint에 반영했다. 대표 Plan/Roadmap v0.1.17, ShellSpec v0.1.6이 detailed contract/evidence를 소유한다.
- current accepted Reference Evidence + Builder-private 4 Profile typed proposal을 Guided Shell의 mutation0 review → explicit USER R1 commit에 연결했고, persistent BuilderCommitReceipt 기반 Complete/Stale resume을 확보했다.
- next implementation gate를 `Step 6 Gameplay Setup Guided connection`으로 전진했다. USER E2E Pending, CF-FQ-040 P2/Ready, Systems 승격 P0-10, 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.44 - 2026-08-27

- `CF-FQ-040 / VB-P0-09 Step Extensibility Hardening` Technical PASS를 current checkpoint에 반영했다. 대표 Plan/Roadmap v0.1.15, ShellSpec v0.1.4가 detailed contract/evidence를 소유한다.
- current baseline Step의 stable StepId와 definition-driven composition/order를 분리해 완성 후 Step 추가·제거·재배치·규칙 수정의 영향 범위를 국소화했다.
- USER E2E Pending, CF-FQ-040 P2/Ready, Systems 승격 P0-10, 단일 Active CF-FQ-039는 변경하지 않았다.

### v1.56.43 - 2026-08-27

- `CF-FQ-040 / VB-P0-09` USER Acceptance 준비에서 Guided Shell Step 2~4 runtime evaluator가 technical PASS했다. 상세 Build/Automation evidence는 대표 Plan v0.1.14가 소유한다.
- current checkpoint를 `VB-P0-09 In Progress / Guided Shell Step 2~4 Technical PASS / USER E2E Pending`으로 갱신했다. CF-FQ-040은 P2 / Ready이며 단일 Active CF-FQ-039는 변경하지 않았다.
- Systems 승격은 VB-P0-10까지 수행하지 않는다.

### v1.56.42 - 2026-08-27

- `CF-FQ-040 / VB-P0-08` post-PASS measurement hardening을 완료하고 대표 Plan/Roadmap을 v0.1.13으로 전진했다.
- fixed 60Hz fresh PIE benchmark는 braking까지 포함한 fitted repeat exact 재현성과 latest no-fitting BaseMass PASS를 확보했다. 상세 수치와 build/process evidence는 대표 Plan이 소유한다.
- next checkpoint는 `VB-P0-09 End-to-End USER Acceptance` 그대로이며 CF-FQ-040은 P2 / Ready, 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v1.56.41 - 2026-08-27

- `CF-FQ-040 / VB-P0-08 Technical Driving Benchmark`를 Harness Technical PASS로 전진했다. arbitrary saved VehicleData + optional Fitting/no-fitting BaseMass를 fresh PIE actual Chaos에서 fixed 60Hz로 측정하는 전용 Automation/runner가 구현됐다.
- fitted repeat exact 재현성과 no-fitting BaseMass PASS를 확보했으며 detailed build/process/metric evidence는 대표 Plan v0.1.12가 소유한다. 대표 canary의 100km/h 미도달로 braking runtime branch 미관측은 제한으로 유지한다.
- next checkpoint를 `VB-P0-09 End-to-End USER Acceptance`로 이동했다. CF-FQ-040은 계속 P2 / Ready이며 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v1.56.40 - 2026-08-27

- `CF-FQ-040 / VB-P0-05~07` post-review hardening을 완료했다. persistent BuilderCommitReceipt, deterministic Profile/Companion approval replay, one-resolve Final Review, post-Apply state guarded Undo와 P0-05 direct write-lane Automation이 current source에 반영됐다.
- 대표 Plan/Roadmap 포인터를 v0.1.11로 전진했고 latest Official Build + sequential focused VB-P0-05 4/4, VB-P0-06 1/1, VB-P0-07 1/1 PASS 상세 evidence는 대표 Plan이 소유한다.
- next checkpoint는 `VB-P0-08 Technical Driving Benchmark` 그대로이며 CF-FQ-040은 P2 / Ready, 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v1.56.39 - 2026-08-27

- `CF-FQ-040 / VB-P0-07 Final Review / Validation / Undo`를 Technical PASS로 전진했다. existing Validation/External Drift/Diff 재사용, accepted Evidence consumed Claim provenance, explicit DefinitionApply와 exact transaction guarded Undo가 구현됐으며 상세 검증 evidence는 대표 Plan이 소유한다.
- 대표 Plan/Roadmap 포인터를 v0.1.10으로 전진하고 next checkpoint를 `VB-P0-08 Technical Driving Benchmark`로 변경했다.
- CF-FQ-040은 계속 P2 / Ready이며 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v1.56.38 - 2026-08-27

- `CF-FQ-040 / VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`를 Technical PASS로 전진했다. Durability/Defense/DestroyedFx/Hardpoint/Mount/DriveState/WheelVisual/FittingMass 8영역 R0 completeness와 USER Hardpoint Socket authority가 구현됐으며 상세 검증 evidence는 대표 Plan이 소유한다.
- 대표 Plan/Roadmap 포인터를 v0.1.9로 전진하고 next checkpoint를 `VB-P0-07 Final Review / Validation / Undo`로 변경했다.
- CF-FQ-040은 계속 P2 / Ready이며 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v1.56.37 - 2026-08-26

- `CF-FQ-040 / VB-P0-05`를 Technical PASS로 전진했다. typed Transmission/Evidence/private Profile commit과 baseline-safe Existing Vehicle Completion이 구현됐으며 상세 검증 evidence는 대표 Plan이 소유한다.
- 대표 Plan/Roadmap 포인터를 v0.1.8로 전진하고 next checkpoint를 `VB-P0-06 Gameplay Defaults / Hardpoint / Fitting Guidance`로 변경했다.
- CF-FQ-040은 계속 P2 / Ready이며 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v1.56.36 - 2026-08-26

- `CF-FQ-040 / VB-P0-04` final source closure에서 local UE 5.8 exact Source의 Reverse ratio convention을 확인해 positive magnitude setup storage + `GetGearRatio()` reverse sign 적용으로 확정했다.
- detailed owner를 `VehicleBuilderProposalSpec.md v0.1.1`, 대표 Plan/Roadmap을 v0.1.7로 전진하고 VB-P0-04를 Source Closure TRUE PASS로 닫았다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation 없이 Implementation 0을 유지하고 next gate는 `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow`다.

### v1.56.35 - 2026-08-26

- `CF-FQ-040 / VB-P0-04 Reference → Authoring Proposal Bridge`를 Design PASS로 전진하고 detailed owner `VehicleBuilderProposalSpec.md v0.1.0`을 연결했다.
- complete Builder-private 4 Profile proposal, provenance/Unknown 보존, UE 5.8 Transmission exact mapping과 typed ratio-set 설계를 확정했다.
- C++/UE Asset/Profile commit/VehicleData Apply/Save/Build mutation 없이 Implementation 0을 유지하고 next gate를 `VB-P0-05 Vehicle Record / Layout / Physics Apply Flow`로 갱신했다.

### v1.56.34 - 2026-08-26

- `CF-FQ-040 / VB-P0-03 Mesh & Socket Guidance`를 Design PASS로 전진하고 next gate를 `VB-P0-04 Reference → Authoring Proposal Bridge`로 갱신했다.
- current AssetReader snapshot authority 재사용, Wheel role 4/4 existing/distinct blocker, optional Hardpoint/Destroyed FX warning boundary와 Layout fresh equality/stale 판정을 고정했다.
- Implementation 0과 Transmission Drivetrain scope를 유지하며 exact Transmission numeric schema/Chaos mapping은 VB-P0-04 전까지 미착수 상태로 보존했다.

### v1.56.33 - 2026-08-26

- `CF-FQ-040`의 실제 다단 변속기 Authoring을 별도 기능으로 분리하지 않고 Builder-private Drivetrain Profile의 정식 차량별 numeric authoring 범위로 통합했다.
- Evidence → AI Drivetrain Proposal → private Drivetrain Profile → 향후 UE 5.8 Chaos `TransmissionSetup` mapping 경계를 추가하고 HUD Gear의 actual `GetCurrentGear()` authority를 보존했다.
- exact transmission schema/API mapping과 C++/Asset/runtime 적용은 VB-P0-04까지 보류했으며 VB-P0-02 PASS, next `VB-P0-03`, P2/Ready/Implementation 0과 current single Active CF-FQ-039는 변경하지 않았다.

### v1.56.32 - 2026-08-26

- `CF-FQ-040 / VB-P0-02 Builder Shell / Step State / Resume`를 Design / Shell-State-Resume Contract PASS로 전진했다.
- 상세 owner `VehicleBuilderShellSpec.md v0.1.0`에 authoritative Step truth, local Editor resume convenience, transient approval/cache 분리와 fixed 8-step navigation을 고정했다.
- managed primary resume identity를 `RecipeId`로 고정하고 Evidence review token과 mutation approval을 분리했으며 restart 뒤 prepared approvals를 복원하지 않도록 했다.
- CF-FQ-040은 계속 P2 / Ready / Implementation 0이며 next gate는 `VB-P0-03 Mesh & Socket Guidance`다. `VB-P0-04` numeric mapping은 미착수이고 current single Active CF-FQ-039는 변경하지 않았다.

### v1.56.31 - 2026-08-26

- `CF-FQ-040 / VB-P0-01 Reference Research & Evidence Contract`를 Design / Research Normalization PASS로 전진했다.
- 상세 owner `VehicleRefEvidenceSpec.md v0.1.0`에 Reference identity/citation/provenance/Unknown/conflict/confidence, EvidenceFingerprint와 AI Proposal binding 계약을 고정했다.
- 실제 2027 Kia Morning 1.0 gasoline Trendy 14-inch dry-run으로 14/16-inch variant split과 Unknown 보존을 검증했다.
- CF-FQ-040은 계속 P2 / Ready / Implementation 0이며 next gate는 `VB-P0-02 Builder Shell / Step State / Resume`다. current single Active CF-FQ-039는 변경하지 않았다.

### v1.56.30 - 2026-08-26

- `CF-FQ-040 / VB-P0-00 Current Vehicle Creation Contract Audit`을 read-only Source/Systems/UE Toolset 감사로 PASS했다.
- Reference Evidence=별도 Editor-only companion DataAsset, AI numeric owner=Builder-private VehicleBase/Drivetrain/Handling/Performance Profile 4종, minimum created Asset=7개를 확정했다.
- CF-FQ-040은 계속 P2 / Ready / Implementation 0이며 next gate를 `VB-P0-01 Reference Research & Evidence Contract`로 전진했다. current single Active CF-FQ-039는 변경하지 않았다.

### v1.56.29 - 2026-08-26

- USER 승인으로 `CF-FQ-040 Guided Vehicle Builder`를 P2 / Ready로 신규 등록했다. 실존 차량 Reference 기반 AI Authoring, USER 직접 Mesh/Wheelbase/Socket 준비, Builder의 네이밍·검증·단계형 UX를 기본 방향으로 고정했다. Wheel 자동검출·Hardpoint 자동배치는 Scope Out이며 첫 Gate는 `VB-P0-00 Current Vehicle Creation Contract Audit`이다.
- `CF-FQ-038`은 Builder의 Data Authoring Backend + Advanced Workspace 역할로 current checkpoint를 갱신했다. 기존 Technical/USER evidence와 DEL6/UA-08 deferred 상태는 보존한다.
- current single Active `CF-FQ-039`은 상태를 변경하지 않고 ActiveWork의 최신 Frame Master v5 checkpoint로 projection만 동기화했다.

### v1.56.28 - 2026-08-25

- authority ChassisMesh 기반 Sedan/SUV Source 2종을 512×256 transparent PNG로 Technical 검증하고 USER 진행 승인 뒤 Production Texture로 Assetize했다.
- persisted `DA_CFHUDVisual_Default.VehicleSilhouettes`를 exact VehicleData 3 entry로 저장했다. Sedan은 `T_UI_VehSil_Sedan`, 일반/Defense SUV는 동일 `T_UI_VehSil_SUV`를 공유하며 legacy fallback은 보존한다.
- 대표 Plan을 v0.1.24로 전진하고 다음 checkpoint를 Defense source family production으로 이동했다. exact `VT-VEH-01` Master Source binding은 별도 Pending이다.

### v1.56.27 - 2026-08-25

- CF-FQ-039 persisted VehicleData 감사로 `DA_TestSedan → Sedan.Sedan`, `DA_TestSUV → SUV.SUV`, `DA_VehicleDefense_TestSUV → SUV.SUV`을 확정해 3 VehicleData / 2 unique ChassisMesh 구조를 고정했다.
- vehicle-specific silhouette Source Authority를 `VehicleData identity → VehicleVisualConfig.ChassisMesh → mesh-derived silhouette Source`로 닫았다. P2 generic silhouette와 exact Chassis identity 없는 VT03 후보는 특정 차량 Production authority로 사용하지 않는다.
- 대표 Plan을 v0.1.23으로 전진하고 다음 checkpoint를 Sedan/SUV mesh-derived Source Candidate + USER Review로 이동했다. 승인 전 `VehicleSilhouettes` count 0과 Master Source Pending을 유지한다.

### v1.56.26 - 2026-08-25

- CF-FQ-039 VT04 common Armor Plate를 repository Source + UE Production Texture + HUD Visual Data binding까지 Technical PASS로 전진했다.
- 대표 Plan 포인터를 v0.1.22로 갱신하고 next checkpoint를 vehicle-specific silhouette Source Authority / VehicleData identity mapping으로 좁혔다. `VehicleSilhouettes` count 0, exact Master Source와 USER Visual Pending은 유지한다.

### v1.56.25 - 2026-08-25

- CF-FQ-039 VT04 Direction Arrow/Chevron2 2종을 repository Source + UE Production Texture + HUD Visual Data binding까지 Technical PASS로 전진했다.
- 대표 Plan 포인터를 v0.1.21로 갱신하고 다음 checkpoint를 common Plate + first vehicle-specific silhouette Production Art binding으로 좁혔다. exact Master Source와 USER Visual은 Pending으로 유지한다.

### v1.56.24 - 2026-08-25

- CF-FQ-039 Vehicle silhouette runtime binding + ArmorSector additive DirectionIcon migration을 Technical PASS로 전진했다. 기존 six-sector Designer ownership과 Runtime Armor Ratio 의미는 유지한다.
- 대표 Plan을 v0.1.20으로 갱신하고 next checkpoint를 `Source Binding + Production Art Binding Pending / USER Visual Pending`으로 변경했다. Feature는 계속 Active이며 다른 Feature 상태는 변경하지 않았다.

### v1.56.23 - 2026-08-25

- CF-FQ-039 Armor 구조를 USER 승인에 따라 vehicle-specific silhouette + common Plate + `Arrow/Chevron2` 2-icon + Designer placement/rotation 구조로 재정의했다.
- `VT03` six-direction local pack은 USER PASS 없이 Superseded됐고 `VT04` 2-icon local Source Pack이 Current evidence다. 기존 six Armor instance name/Presenter Ratio binding은 보존한다.
- 대표 Plan 포인터를 v0.1.19로 전진하고 next checkpoint를 `Vehicle Silhouette Binding + WBP Migration Pending / Source Binding Deferred`로 변경했다. 다른 Feature 상태는 변경하지 않았다.

### v1.56.22 - 2026-08-25

- CF-FQ-039 Frame Family는 `VT01_FrameReview_v2` USER PASS로 닫혔다. RPM-S1과 Frame Review를 새 관련 결함 없이 반복하지 않는다.
- 다음 checkpoint를 `ARMOR-S1 Local Source Pack Ready / USER Review Pending / Source Binding Deferred`로 전진했다. `VT03_ArmorReview_v2` PASS 뒤 Defense source family로 이동한다.
- 대표 Plan 포인터를 v0.1.18로 동기화했으며 다른 Feature 상태는 변경하지 않았다.

### v1.56.21 - 2026-08-25

- CF-FQ-039 RPM-S1은 AUTO QA PASS에 이어 USER Visual PASS까지 완료했다. 대표 Plan 포인터를 v0.1.17로 전진하고 next checkpoint를 `Frame USER Review Pending / Source Binding Deferred`로 좁혔다.
- RPM Review는 새 관련 결함이 없는 한 반복하지 않는다. Frame PASS 뒤 Armor silhouette + 6 icon-plate source extraction으로 이동하며 UE Import는 Source Binding 완료 전 금지한다.
- 다른 Feature 상태는 변경하지 않았다.

### v1.56.20 - 2026-08-25

- CF-FQ-039 작동형 RPM-S1 Local Source Pack과 packed mask AUTO QA PASS를 반영해 대표 Plan을 v0.1.16으로 전진했다.
- next checkpoint를 FrameReview_v2 + RPM Runtime Pack USER Visual Review로 변경했다. repository Source Binding은 Deferred이며 UE Import/WBP/Runtime mutation은 아직 시작하지 않았다.
- 다른 Feature 상태는 변경하지 않았다.

### v1.56.19 - 2026-08-25

- CF-FQ-039 Frame F1~F4 Local Pre-Binding Extraction Evidence와 F3 ArmorFrame v2 cleanup, FrameReview_v2 준비 상태를 반영해 대표 Plan을 v0.1.15로 전진했다.
- USER local PC access 제약으로 repository Source Binding은 Deferred 유지하고 next checkpoint를 USER Frame Review로 변경했다. UE Import/WBP mutation은 아직 시작하지 않았다.
- 다른 Feature 상태는 변경하지 않았다.

### v1.56.18 - 2026-08-24

- CF-FQ-039 `VT-VEH-01` USER APPROVED / LOCKED와 Frame family F0~F6 extraction plan ready를 반영해 대표 Plan을 v0.1.14로 전진했다.
- next checkpoint를 Master source repository binding + exact hash readback으로 변경했다. 실제 Frame PNG extraction/UE Import는 아직 시작하지 않았으며 다른 Feature 상태는 변경하지 않았다.

### v1.56.17 - 2026-08-24

- CF-FQ-039 Slot Contract closure와 Physical Art Breakdown Contract Ready 상태를 반영해 대표 Plan을 v0.1.13으로 전진했다.
- next checkpoint를 `VT-VEH-01 explicit lock + Master source repository binding`으로 변경했다. 실제 Art/UE Asset 생성은 아직 시작하지 않았으며 다른 Feature 상태는 변경하지 않았다.

### v1.56.16 - 2026-08-24

- CF-FQ-039 USER 결정으로 VehiclePanel 상단 Speed/RPM : Armor 50:50 Composition을 고정하고 대표 Plan을 v0.1.12로 전진했다.
- 896×416 기준 `422 + Gap 12 + 422` width lock을 반영했다. 남은 Slot Contract Review와 `VT-VEH-01` 승격은 Pending이며 다른 Feature 상태는 변경하지 않았다.

### v1.56.15 - 2026-08-24

- CF-FQ-039 `CAND-VEH-MASTER-01` Logical Breakdown과 WBP Skeleton Contract 완료를 반영해 대표 Plan을 v0.1.11로 전진했다.
- next checkpoint를 `Slot Contract Review`로 변경했다. `VT-VEH-01` 승격과 Physical Art production은 아직 Pending이며 다른 Feature 상태는 변경하지 않았다.

### v1.56.14 - 2026-08-24

- CF-FQ-039의 기존 WBP/Primitive-first 제작 방식 Deprecated와 Target Decomposition Pipeline LOCK을 반영해 대표 Plan을 v0.1.10으로 전진했다.
- Current checkpoint를 `Production Method Reset Locked / Target Logical Breakdown Pending`으로 변경했다. Armor USER PASS와 Frame/Shield/Integrity Technical·Pixel evidence는 보존하되 현재 Primitive/P2 결과는 Production Visual authority가 아니다.
- 다른 Feature 상태와 Current owner는 변경하지 않았다.

### v1.56.13 - 2026-08-24

- CF-FQ-039 Defense Bar final focused + actual Defense Pawn pixel revalidation PASS를 반영해 대표 Plan을 v0.1.9로 전진하고 next checkpoint를 USER Visual Review Pending으로 복구했다.
- Current UI owner projection을 `InGameUI.md v1.1.5`로 동기화했다. Armor USER PASS와 전체 `VT-VEH-01` Pending 경계는 유지한다.
- CF-FQ-038 v1.56.12 상태는 변경하지 않았다.

### v1.56.12 - 2026-08-24

- `CF-FQ-038` Legacy Wizard Deprecated transition을 Technical Complete로 닫았다. shared Official Editor Build PASS 이후 focused `Workspace.TabRegistration` 1/1 Success와 fresh Editor `창(Window)`에서 Current Workspace 존재 / Legacy Wizard 메뉴 부재를 확인했다.
- `LegacyManagedGuard`는 이번 change set의 guard Source mutation0이라 기존 PASS를 보존했다. DEL6 required-caller0와 UA-08 quantitative comparison만 non-blocking Pending/Deferred로 남기고 CF-FQ-038은 Paused, 현재 단일 Active `CF-FQ-039`는 유지한다.

### v1.56.11 - 2026-08-24

- CF-FQ-039 fresh PIE pixel review에서 Integrity Bar 소실이 확인돼 Frame + Shield/Integrity Technical PASS를 재검증 Pending으로 조정했다.
- 최종 Defense Bar pixel fix Official Build는 PASS했으나 exact focused rerun + fresh PIE pixel revalidation 전이라 대표 Plan을 v0.1.8로 전진하고 USER Visual Gate 승격은 보류했다.
- Current UI owner projection을 `InGameUI.md v1.1.4`로 동기화했다. Armor USER PASS는 보존한다.

### v1.56.10 - 2026-08-24

- CF-FQ-039을 Frame + Shield/Integrity Technical PASS / USER Visual Review Pending으로 전진하고 대표 Plan을 v0.1.7, Current UI owner를 `InGameUI.md v1.1.3`으로 동기화했다.
- Armor USER PASS와 전체 `VT-VEH-01` Pending 경계는 유지했다.
- 이번 full official Editor Build PASS로 CF-FQ-038의 unrelated UI compile blocker가 해소된 사실만 반영하고, CF-FQ-038 focused validation/DEL6는 해당 Paused 작업에 남겼다.

### v1.56.9 - 2026-08-24

- `CF-FQ-038` Deprecated Gate DG1~DG5를 PASS하고 Legacy Wizard 기본 Window 메뉴 숨김 Source를 적용했다. hidden Wizard spawner는 DEL6 전 compatibility로 유지한다.
- exact official Build/focused validation은 현재 Active `CF-FQ-039` dirty UI compile 오류 때문에 Pending이다. CF-FQ-038을 Done으로 확대하지 않고 Paused 유지하며 physical Wizard deletion은 금지한다.

### v1.56.8 - 2026-08-24

- `CF-FQ-038 / DAUTH-P0-12`을 UA-08 quantitative feel comparison Deferred 경계를 포함한 `Closed with Deferred Feel Comparison`으로 종료했다. USER PASS는 7/8 유지하고 UA-08 Runtime Technical PASS / USER Inconclusive를 분리 기록했다.
- 사용자 다음 단계 진행 승인에 따라 CF-FQ-038의 다음 checkpoint를 `DG/DEL Gate / Deprecated Gate DG1~DG5 Audit`으로 이동했다. `SCFVDAWizardTab` physical deletion은 DEL1~DEL7 전까지 금지하며 현재 단일 Active `CF-FQ-039`는 유지한다.

### v1.56.7 - 2026-08-24

- CF-FQ-039 Armor visual slice USER PASS를 반영하고 대표 Plan 포인터를 v0.1.6으로 동기화했다.
- 다음 checkpoint를 VehiclePanel Frame + Shield/Integrity visual match로 전진했으며 전체 `VT-VEH-01`은 Pending을 유지했다.

### v1.56.6 - 2026-08-24

- CF-FQ-039 checkpoint를 VPR-P0-01 Armor Technical PASS / USER Visual Review Pending으로 전진하고 CF-FQ-032 Current owner projection을 `InGameUI.md v1.1.2`로 동기화했다.
- Armor Presentation technical slice PASS를 반영하되 전체 `VT-VEH-01`은 USER 명시 승인 전이라 Pending을 유지했다.
- 기존 CF-FQ-038 UA-08 재개 상태와 다른 Queue 항목은 변경하지 않았다.

### v1.56.5 - 2026-08-24

- `CF-FQ-038 / DAUTH-P0-12 UA-07 Driving Feel Authoring` USER PASS와 P0-12 USER PASS 7을 반영했다.
- 다음 정확한 Gate를 `UA-08 Driving Feel Runtime Comparison`으로 전환하고 대표 Plan/Roadmap 포인터를 v0.2.48/v0.1.53으로 동기화했다. 현재 단일 Active `CF-FQ-039`는 유지한다.

### v1.56.4 - 2026-08-24

- CF-FQ-039 Vehicle editable Structure Readiness PASS와 `VT-VEH-01` USER 명시 승인 대기 상태를 Current checkpoint에 반영했다.
- CF-FQ-032 Current owner 포인터를 `InGameUI.md v1.1.1`로 동기화했다.
- 기존 `WBP_CFArmorSector` 6개 재사용을 유지하고 신규 Armor slot Widget을 중복 생성하지 않는 방향은 상세 Plan이 소유한다.

### v1.56.3 - 2026-08-23

- CF-FQ-039 VPR-P0-00의 AI Recovery 완료와 전체 VehiclePanel `VT-VEH-01` USER 확정 대기 상태를 반영했다.

### v1.56.2 - 2026-08-23

- CF-FQ-039 VPR-P0-00 실제 Recovery 진행 상태와 Plan v0.1.2를 반영했다.
- 전체 VehiclePanel Production Visual Target은 아직 source recovery 중이며 P1/P2 Technical Review를 USER Target으로 오승격하지 않도록 상태를 명시했다.

### v1.56.1 - 2026-08-23

- CF-FQ-039 pre-start 설계 교정 후 대표 Plan 포인터를 v0.1.1로 동기화했다.

### v1.56.0 - 2026-08-23

- 사용자 선택에 따라 `CF-FQ-039 Production UI Visual Rework`를 P1 / Active로 신규 등록했다.
- 첫 checkpoint를 `InGameUIVisualPlan.md v0.1.0 / VPR-P0-00 Visual Target Recovery & Registry`로 고정했다.
- `CF-FQ-032`는 Done을 유지하고 Visual Rework를 별도 Feature로 분리해 기존 Runtime·Build·Automation·USER evidence를 보호했다.

Migration: UI Visual 작업은 CF-FQ-032을 재개하지 않고 CF-FQ-039의 `VPR-*` Gate를 사용한다.

### v1.55.0 - 2026-08-22

- v1.54.62의 2,000줄 이상 누적된 Feature별 P0 진행 로그, Build/Automation UUID, 과거 Current projection을 제거하고 실제 Queue 역할로 정상화했다.
- 전체 Feature ID와 상태는 유지하면서 각 항목을 현재 checkpoint와 Current owner 중심으로 재구성했다.
- `CF-FQ-032 Done / 현재 Active 없음`, `CF-FQ-038 UA-01~06 PASS → UA-07`, `CF-FQ-034 FIT-P0-07D`, `CF-FQ-029 LM-P0-06`, `CF-FQ-030 CF-TC-027 Manual PIE` 최신 상태를 반영했다.
- Sensor/UI Current owner를 `InGameUI v1.1.0`, `AimReticle v1.10.0`, `SensorContact v1.2.0` 기준으로 동기화했다.

Migration: 삭제된 상세 진행/evidence는 각 대표 Plan, Systems와 Git history에서 조회한다. FeatureQueue 압축은 완료 evidence를 무효화하거나 USER Gate를 다시 여는 변경이 아니다.
