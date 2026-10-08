# CarFight InGame UI Roadmap

- 문서 버전: v0.26.35
- 작성일: 2026-07-30
- 최근 갱신일: 2026-08-22
- 문서 상태: Historical / Retained Path / CF-FQ-032 Done / M6 Done / UI-P0-11 Systems Promotion Complete / USER Visual·Zoom Feel Deferred Pending 보존

- 기능 ID: `CF-FQ-032`
- 대표 Plan: `InGameUIPlan.md`
- 시각 콘셉트: `InGameUIVisualConcept.md`
- 제작 규격 초안: `InGameUIStyleSpec.md`
- 자산화 실행 규격: `InGameUIAssetizationSpec.md v0.4.0`
- 1440p Wireframe: `ConceptArt/CFHUDWireframe_1440p.xml`
- Vehicle Panel 상세: `InGameUIVehiclePanelSpec.md`
- Vehicle Panel Wireframe: `ConceptArt/CFVehiclePanel_1440p.xml`

---

## 2026-08-18 Historical Resume Checkpoint

> 아래 블록은 2026-08-18 이후 CF-FQ-032 진행 과정에서 누적된 당시 checkpoint를 보존하는 Historical snapshot이다. 현재 작업 선택은 main_game ProjectSSOT/FeatureQueue/ActiveWork를, 현재 구현 owner는 main_game Systems를 우선한다.

```text
최근 완료 Feature: CF-FQ-032 Done / Post-Closure Remediation Technical PASS / 현재 Active 없음
대표 Historical Plan: InGameUIPlan.md v0.59.35
완료 Gate: UI-P0-03 Production Runtime PIE Closure — USER PASS
완료 Gate: UI-P0-04 AimReticle UISubsystem 통합 — Build·Automation·USER Visual PASS
완료 Gate: UI-P0-05 TargetSelect Marker 통합 — Source·Official Build·focused Automation·USER Visual PASS
보존 Visual Gate: D1-11-ART ArmorBodyMap Designer Editability USER PASS / SpeedGauge·VehiclePanel·전체 Visual Review Deferred·USER PASS 미부여 / completed P2·Designer Ownership evidence 반복 금지
완료 Runtime Gate: UI-P0-07 Target Knowledge — Technical Complete / Official Build `72893293dd0f4dd98fbc93d96f3466dd` PASS / focused `e82f8ef659d34900a46ee32e8f98e94f` 1/1 PASS
완료 Runtime Slice: UI-P0-08A Radar Range Foundation — Technical PASS / Build `07b963ac33614ab2ad43193c8cd950b5` / Radar focused 1/1 / SEN-P0-06 HUD regression 1/1
완료 Runtime Slice: UI-P0-08B Production RadarPanel Consumer — Technical PASS / Image Contact·Range·Player·Selected in/out / Build `f7ac2bea527d4207ac0c45bd0a40d738` / targeted exact9 / UI-P0-08 focused 2/2
완료 Runtime Slice: UI-P0-08 Radar Zoom Input — Technical PASS / Build `18ddc2ba098d44f68721a108e3f43f8f` / exact setup `5ecd196794d9453091c0ed27c03f0e60` 1/1 / persisted MouseScrollUp +1·MouseScrollDown -1 / Zoom·Range·WeaponSelect focused regression 각 1/1 PASS
완료 Runtime Slice: UI-P0-08 Screen-Off Selected Target Edge Marker — Technical PASS / `CFTargetSelectWidget v1.2.0` + `CFUISubsystem v1.9.0` / Camera View Space + Safe Region ray intersection / Build `4eecb33895574b87b4726a544f029835` PASS / ScreenEdge `ed5b04ade60c4fc086576610edfbfbca` 1/1 / TargetMarker regression `9a0a1cd97c804321b36313ac0e085b1b` 1/1 / persisted WBP_TargetSelect 8-node Designer 유지·runtime Edge save0
완료 Runtime Slice: UI-P0-09A View Mode / Direction Foundation — Technical PASS / `CFHUDViewData v1.14.0`, `CFHUDDataProvider v1.9.0/v1.15.0`, `CFHUDDataTests v1.26.1` / Build `a78e59503f6e45ce98f952941ea2969a` PASS / focused `ba3fc0bb5d27448db14c0f28b9e76997` 1/1 PASS / Asset mutation0·Gameplay Camera/Aim mutation0
완료 Runtime Slice: UI-P0-09B Production View Mode Consumer — Technical PASS / ReticleLayer additive Vehicle Direction Image / targeted apply `642fe507a2d44a46a8d65a90d94253f0` exact1 / fresh persisted Root7 + new2 / initial focused 1/1 + regression `094dab7a01d6479286a0e0995062bc17` 1/1 PASS
완료 Runtime Slice: UI-P0-09C Alert Lifecycle — Technical PASS / Notice2s·Warning3s·CriticalPersistent / repeated refresh non-reset / focused `1fc3c160e74644079459ce3dab8074d4` 1/1 PASS
완료 Runtime Slice: UI-P0-09D Nested Style Context — Technical PASS / Root→nested Styled Widget Style·Density·Scale propagation / same-context duplicate refresh0 / focused `693b4a267fb5474e8b7a04b4b6b4e760` 1/1 PASS
UI-P0-09 final combined Official Build: `f209faafb10c4df1a446bf294f1f75ce` PASS / Exit0
완료 Runtime Gate: UI-P0-10 Integration Validation — Technical Complete / Resolution 4종 + Pause guard + final `CarFight.UI` 43/43 + Ammo/Sensor cross-system + fresh AI-owned PIE RuntimeRead Technical Validation PASS / USER Visual·Feel PASS 미부여
보존 USER Follow-up: UI-P0-08 Radar/Edge Visual·Zoom Feel — Deferred/Pending / USER PASS 미부여
완료 Runtime Gate: UI-P0-11 Systems Promotion — `Systems/UI/InGameUI.md v1.0.0` Current 승격 + `AimReticle.md v1.9.0` owner 교정 + `SystemIndex.md v1.21.0` 등록 / Product mutation0
후속 TargetPanel/Radar Sensor Source: main_game SensorContact.md v1.1.0
DAUTH: CF-FQ-038 Paused / DataAuthoringPlan v0.2.28 / Roadmap v0.1.36 / P0-12 USER PASS 2 / UA-03 DriveState RequiredProfileMissing blocker 보존 / UI RPM additive schema bounded compatibility current 118 PASS / USER Acceptance 재개 안 함
```

UI-P0-02~05의 완료된 Build/Automation/USER evidence와 UI-P0-06~10의 완료 Build/persisted/exact Automation·AI Runtime evidence는 관련 결함이 없는 한 반복하지 않는다. CF-FQ-032은 Done이므로 현재 구현은 main_game `Systems/UI/InGameUI.md v1.1.0`, `Systems/UI/AimReticle.md v1.10.0`, `Systems/Targeting/SensorContact.md v1.2.0`과 실제 Source/Asset을 우선하고, 위 snapshot과 아래 milestone별 당시 next-action은 Historical 기록으로만 읽는다.

## 1. 목표

인게임 HUD를 현재 Pawn 소유 개별 Widget 묶음에서 LocalPlayer 소유 UI 프레임워크로 안전하게 확장한다.

`CF-FQ-032`은 UI-P0-02~05 USER PASS, UI-P0-06~10 Technical Complete/AI Runtime Technical Validation과 UI-P0-11 Systems Promotion을 완료한 Done 상태다. post-closure remediation도 Technical PASS했으며 현재 구현 owner는 main_game `Systems/UI/InGameUI.md v1.1.0`, `Systems/UI/AimReticle.md v1.10.0`, Target/Sensor Identity는 `Systems/Targeting/SensorContact.md v1.2.0`이다. UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual, Rail USER Visual·Heat/Charge tuning Visual·Redline/RPM Visual은 사용자의 진도 우선 결정에 따른 비차단 Deferred/Pending이며 PASS로 추정하지 않는다. 이 Roadmap은 Historical + Retained Path이며 새 Active 기능을 선택하지 않는다.


---

## 2. 마일스톤

| Milestone | Task | 목표 | 종료 조건 | 상태 |
|---|---|---|---|---|
| M0 | UI-P0-00A~00B | 코드 조사·계약 확정 | 실제 구조 검토와 15개 구현 계약 승인 | Done |
| M1 | UI-P0-01A~02 | PlayerController, UI Root와 완전 Pause | 입력 수명·World Root·Pause PIE | UI-P0-02 Runtime Lifetime USER PASS |
| M2 | UI-P0-03~05 | View Data와 기존 HUD 단계 이전 | Aim/FireFeedback·TargetSelect 회귀 PASS | Done / UI-P0-03~05 USER PASS |
| D1 | UI-DESIGN-GATE | CarFight UI 디자인 기반 | Style Data·Base Widget·Density·Font·Icon·Static Review 기준 확정 | D1-11 Production Structure PASS / **D1-11-ART Visual Foundation — P2 Technical Applied / USER Visual Review Pending** / D1-12 Not Started / 후속 해상도 Deferred |

| M3 | UI-P0-06 | 차량·무기 HUD | 속도·방어·자원 채널 표시 | Technical Complete / Stage A+B + RPM + Heat + Weapon Selection Runtime/HUD + truthful Rail + Weapon Select Input + WeaponCharge Runtime/HUD Technical PASS / VehicleBattery future Gameplay dependency·현재 Unavailable/Collapsed / Rail USER Visual·Heat/Charge tuning Visual·actual Redline/RPM Visual content-dependent Deferred |
| M4 | UI-P0-07~08 | Target Knowledge와 Radar | `???`, Contact, 화면 밖 방향 | In Progress / UI-P0-07 Technical Complete / UI-P0-08 Technical Complete / USER Visual·Zoom Feel Pending |
| M5 | UI-P0-09 | View Mode·Alert·Style 정제 | ThirdPerson 완료, 경고와 Turret 확장점 | Technical Complete / UI-P0-09A~D Technical PASS |
| F1 | UI-COMMONUI-GATE | 전체 화면 프레임워크 판단 | UMG 유지·혼합 구조·CommonUI 확장 중 결정 | Deferred / P0 HUD 비차단 |
| M6 | UI-P0-10~11 | 통합 검증·Systems 승격 | 빌드·자동 테스트·PIE 기술 검증 + Current Systems 승격 | **Done / UI-P0-10 Technical Complete + UI-P0-11 Systems Promotion Complete** |

---

## 3. M1 — PlayerController, UI Root와 완전 Pause

### UI-P0-01A

상태:

```text
Code Complete / Official Build PASS / Automation PASS / User PIE Pending
```

구현:

- 최소 `ACFPlayerController`
- Controller가 직접 등록한 System·Gameplay·UI Mapping Context 수명
- Pawn과 독립된 Pause·Back 요청
- Input Action 미지정 시 Pause 중 실행 가능한 C++ Fallback Key
- Possessed Pawn 변경 통지
- Mapping Context 중복 등록과 잔류 방지
- GameOnly·GameAndUI·UIOnly, Cursor와 Focus 전환 기반
- 기존 Pawn Gameplay 입력의 삭제 없는 일시 억제·복원

검증 결과:

- `CarFight.UI.UI_P0_01A.ControllerContract`: Success
- 공식 Editor Build: PASS
- 기존 Pawn의 DefaultInputMappingContext와 Gameplay Action Binding: 변경 없음
- 실제 Pawn 교체·Pause 입력 사용자 PIE: Pending

### UI-P0-01B

상태:

```text
Code Complete / Official Build PASS / Automation PASS / User PIE Pending
```

구현:

- `UCFUISubsystem`
- 에셋 없는 C++ `UCFUIRootWidget`
- Game, HUD, Screen, Panel, Menu, Modal, System과 Debug `UCanvasPanel` 레이어
- 레이어별 명시적 ZOrder와 `EnsureLayerTree()` 초기화 계약
- 향후 전체 화면용 Screen·Modal 전환 진입점
- Input Mode·Cursor·Focus 연동
- LocalPlayer당 Subsystem 1개, World당 현재 Root 1개
- World Cleanup 시 이전 Root와 Pawn 약한 참조 정리

검증 결과:

- `CarFight.UI.UI_P0_01B.RootLayerContract`: Success
- 전체 CarFight Automation: 29/29 Success / Failed 0
- 필수 Combat 회귀: 17/17 Success
- 피팅 Compatibility·MassSnapshot: Success
- 기존 AimReticle·TargetSelect 생성 경로: 변경 없음
- 레벨 재진입·Possess 해제 사용자 PIE: Pending

`WBP_CFUIRoot` Unreal Asset은 이번 범위에서 만들지 않았으며, 향후 디자이너 레이아웃이 필요할 때 C++ Root 계약을 유지한 파생 Widget으로 검토한다.

### UI-P0-02

상태:

```text
Code Complete / Build·Automation PASS / Partial USER PASS
```

구현:

- Pause 진입 전 차량 Drive·Look 입력과 Controller 눌린 키 상태 중립화
- `APlayerController::SetPause` 기반 실제 싱글플레이 World Pause
- 에셋 없는 C++ `UCFPauseMenuWidget`
- Menu 레이어 단일 수명과 Continue 기본 Focus
- Pause 재입력·Back·Continue 해제
- Primary Screen 또는 InGame 입력 모드 복원
- 진행 중 Ripple·Salvo, Projectile와 Motor 상태 보존
- World Cleanup·Controller 해제 시 Pause·Menu 정리

자동 검증 결과:

- `CarFight.UI.UI_P0_02.PauseMenuContract`: Success
- `CarFight.UI.UI_P0_02.InputNeutralContract`: Success
- `CarFight.UI.UI_P0_02.ProgressFreezeContract`: Success
- 전체 CarFight Automation: 32/32 Success / Failed 0
- 필수 Combat 회귀: 17/17 Success
- Launcher·Projectile·Propulsion·Fitting 보호 회귀: Success
- 공식 Editor Build: `4f2daa07187a41aea7c400386010b2a0` / Exit Code 0

사용자 PIE Pending:

- 실제 Pause·Continue·Back 입력과 Focus
- 차량 입력 잔류 없음
- Ripple·Salvo가 멈춘 위치에서 재개
- Projectile·Motor·Timer 정지와 재개
- 반복 Pause와 레벨 전환의 Menu·Root 중복 없음

종료 판정:

- 코드·빌드·Automation과 Pause UI 상호작용 사용자 검증 범위는 완료했다.
- Gamepad·입력 잔류·Launcher·Projectile·Timer·Root 수명 사용자 PIE 전에는 M1 전체 PASS 또는 Current System으로 승격하지 않는다.

---

## 4. M2 — View Data와 기존 HUD 단계 이전

### UI-P0-03

상태:

```text
USER PASS / Defense Production Panel + Pawn Rebind + Old Pawn Event Isolation USER Visual PASS
```

구현:

- 차량, 무기, 타겟, Radar와 Alert View Data
- `Unknown`, `Unavailable`, 실제 0 구분
- Authoritative Target Data와 Target Knowledge View 분리
- 게임플레이 이벤트 바인딩과 Pawn 교체 처리
- 이벤트·프레임·제한 주기 갱신 경계

종료 조건:

- Widget이 BP Vehicle로 Cast하지 않음
- Mock과 실제 Provider를 같은 View Data로 교체 가능
- UI가 플레이어 지식보다 많은 정보를 표시하지 않음

### UI-P0-04

상태:

```text
USER PASS / Build ace404b0eb7f4c97b010e96ce85f41ce PASS / focused Automation 1/1 PASS / Reticle 단일 수명·Pawn Rebind USER Visual PASS
```

구현:

- 기존 AimReticle을 HUD Layer에서 관리
- `LegacyPawnOwned`와 `UISubsystemOwned` 전환 게이트
- 기존 상태·색상·문구 유지

종료 조건:

- NoWeapon, AimBlocked, FireSuccess, Cooldown 회귀 PASS
- CenterDot과 WeaponReticle 의미 유지
- Widget 중복 없음

### UI-P0-05

상태:

```text
Source·Official Build·focused Automation PASS / USER Visual Pending
```

구현:

- 기존 TargetSelect HUD를 Root 아래로 이전
- 선택 이벤트와 공간 투영 갱신 분리
- 공개 이름 fallback의 Actor 내부 이름 제거
- Possess 해제와 Target 파괴 시 구독 정리

종료 조건:

- 후보·선택·가림 상태 회귀 PASS
- 미공개 이름은 `???`로 표시됨
- 이전 Pawn 또는 Target의 이벤트가 새 HUD를 갱신하지 않음

---

## 4-1. D1 — UI-DESIGN-GATE

### 목적

신규 HUD를 기능만 동작하는 임시 외형으로 누적하지 않도록 CarFight의 최소 디자인 언어와 공통 시각 자산을 확정한다. 이 Gate는 CommonUI 도입 여부와 무관하다.

### 현재 체크포인트

```text
InGameUIVisualConcept.md v0.8.0
InGameUIStyleSpec.md v0.20.0
InGameUIAssetizationSpec.md v0.4.0
InGameUIVehiclePanelSpec.md v0.2.0
InGameUIWeaponPanelSpec.md v0.5.0
InGameUIStaticReview.md v0.6.0
CFHUDWireframe_1440p.xml = 이전 배치 참고
CFVehiclePanel_1440p.xml = VehiclePanel 승인 Layout 참고
CFWeaponPanel_1440p.xml = 초기 넓은 배치 참고
CFHUD_SR1080_16.xml = D1-07 Phase 1 Review Mockup
CFHUD_SR1440_16.xml / CFHUD_SR1440_21.xml / CFHUD_SR1440_32.xml = Deferred Expansion Reference

Accepted
- 외부 3인칭 TPS·16:9 HUD Placement
- VehiclePanel Visual Layout
- WeaponPanel Compact Visual Direction
- Global UI Customization Contract
- D1-05 Style·Base Widget·Density·Weapon Compact Token
- D1-06 Pretendard + IBM Plex Mono / Solid Core + Tactical Cut

D1-07
- Phase 1 `SR-1080-16` Combat Busy Mockup USER PASS
- 외곽 재배치 수정본 승인 / 패널 크기 유지 / 실효 약 24px 외곽 Margin
- 1080p Typography 유효 하한 반영
- `SR-1440-16`·`SR-1440-21`·`SR-1440-32` Deferred Expansion
- 후속 21:9·32:9 Persistent HUD 중앙 2560×1440 Canvas 계약 보존
- Runtime Capture Not Run

Current / Pending
- D1-08V C++ Style Data Type: PASS / Official Build·StyleDataContract Automation
- D1-09A Font Asset Binding·HUD Layout·Density Type + Subsystem Config Soft Reference: PASS / Official Build·4 Contract Automation
- D1-09B Style·Density·1080 Layout·Font/Icon Asset + Notice + Config: PASS / Readback·Full Regression
- D1-10A C++ Visual Context/Button Bridge: PASS / Official Build·Contract Automation
- D1-10B Base Widget 5종: Editor Bridge Source Compile PASS / Official DLL Link Blocked / Asset Not Created
- D1-11 `WBP_CFInGameHUD` Visual Prototype: Prepared / 기능 HUD 금지
- D1-12 Pause Menu Visual Migration: Prepared / 기존 Pause Runtime 보존
- Font License: Pretendard / IBM Plex Mono 공식 OFL 1.1 Notice 보존·Readback PASS

Non-blocking Follow-up
- 필요 시 같은 1920×1080에서 SR-A Normal / SR-C Critical / SR-D Sparse / SR-E Long Text 추가 Mockup
- `SR-1440-16`·`SR-1440-21`·`SR-1440-32` Layout Profile 확장 Review

Gate PASS: D1-10~12 자산화·Visual 검증 완료 전 금지
```

시각 방향과 D1-07 1080p 대표 Static Review는 사용자 승인됐고 D1-08V·D1-09A·D1-09B 기술/자산화 검증도 PASS했다. 다만 Base Widget·HUD Prototype·Pause Visual인 D1-10~12가 남아 있으므로 D1 전체 PASS로 판정하지 않는다.

### 실행 시점

```text
UI-P0-03 View Data
→ UI-P0-04 AimReticle 수명 이전
→ UI-P0-05 TargetSelect 수명 이전
→ UI-DESIGN-GATE
→ UI-P0-06 신규 차량·무기 HUD 시각 구현
```

UI-P0-03~05는 데이터·수명·회귀를 우선하므로 기존 시각을 보존한 채 진행할 수 있다. 새 차량·무기 HUD를 실제로 배치하기 전에는 이 Gate를 통과해야 한다.

### 산출물

- `InGameUIVisualConcept.md`: Visual Direction·HUD Placement Accepted
- `InGameUIStyleSpec.md v0.20.0`: D1-07~09B PASS / D1-10A PASS / D1-10B Blocked / D1-10 NOT PASS
- `InGameUIAssetizationSpec.md v0.4.0`: D1-09A·09B PASS Evidence / D1-10 Partial Execution Checkpoint / D1-11~12 Prepared
- `InGameUIVehiclePanelSpec.md v0.2.0`: Visual Layout Accepted
- `InGameUIWeaponPanelSpec.md v0.5.0`: Compact Token User Accepted / Customizable Default Preset
- `InGameUIStaticReview.md v0.6.0`: D1-07 Phase 1 `SR-1080-16` USER PASS
- `CFHUD_SR1080_16.xml`: Phase 1 Combat Busy Static Mockup
- `CFHUD_SR1440_16.xml`, `CFHUD_SR1440_21.xml`, `CFHUD_SR1440_32.xml`: Deferred Expansion Reference
- `CFHUDWireframe_1440p.xml`: 승인 HUD Placement 기준
- `CFVehiclePanel_1440p.xml`: VehiclePanel 승인 Visual Layout 기준
- `CFWeaponPanel_1440p.xml`: 초기 넓은 배치 참고 / Compact Pixel SSOT 아님
- `DA_CFUIStyle_Default`·Density 3종·`DA_CFHUDLayout_1080_16`: D1-09B PASS
- `Font_CFUI`·`Font_CFNumeric`·FontFace 8종·Semantic Icon 18종·OFL Notice: D1-09B PASS
- `WBP_CFButtonBase`, `WBP_CFPanelBase`, `WBP_CFStatusBar`, `WBP_CFInfoRow`, `WBP_CFAlertItem`: Asset Not Created / Editor DLL Link 해제 후 D1-10B 재개
- `SR-1080-16` 사용자 Static Review: USER PASS / 2026-08-07
- 1440p·21:9·32:9 실제 확장 Review: Deferred

### 종료 조건

- 버튼과 상호작용 요소가 외형만으로 식별된다.
- 같은 의미의 정보가 같은 색상·폰트·간격 규칙을 사용한다.
- 색상 외 텍스트·아이콘·형태로도 상태를 구분한다.
- 중심 조준 영역과 주변 상태 정보의 시선 우선순위가 문서화된다.
- 공통 스타일 변경으로 개별 Widget의 하드코딩을 최소화할 수 있다.
- Compact·Standard·Expanded가 같은 View Data 의미를 유지한다.
- 미지원 정보가 `Collapsed`되고 빈 Spacer가 남지 않는다.
- WeaponPanel Compact Full 상태가 Maximum Slot 안에서 검토된다.
- D1-07 Phase 1 `SR-1080-16` 실제 Static Review가 PASS한다.
- `SR-1440-16`, `SR-1440-21`, `SR-1440-32` 후속 확장은 Phase 1 Gate를 차단하지 않는다.
- 사용자 승인되지 않은 Draft 수치를 구현 완료로 해석하지 않는다.

---

## 5. M3 — 차량과 무기 HUD

### UI-P0-06

상태:

```text
Technical Complete / Stage A Presentation Projection + Stage B Dynamic Resource Visual Technical PASS 보존
RPM Gauge explicit Redline upstream + Production Visual Binding Technical PASS 보존
Weapon Heat P0 Runtime + accepted-fire integration + HUD Resource Projection Technical PASS
WeaponCharge P0 Runtime + accepted-fire integration + HUD Resource Projection Technical PASS
Applied Fitting Weapon Selection Runtime/HUD source + truthful Production Rail Visual Consumer + Player-facing Weapon Select Input Technical PASS
VehicleBattery = future shared-power Gameplay dependency / current Provider 없음 → Unavailable/Collapsed
Rail USER Visual + actual Heat/Charge tuning USER Visual + Redline authoring/RPM USER Visual = content-dependent Deferred follow-up
formal next Gate = UI-P0-07 Target Knowledge
```

#### 착수 감사 분류

이미 충족 — 기존 PASS 보존, 재구현 금지:

- 숫자형 Speed
- 실제 Shield / 6방향 Armor / Vehicle Integrity
- Weapon Cooldown
- finite Ammo `Loaded / MagazineCapacity` + label-less Reserve
- Reload / NoAmmo 상태
- Ripple / Salvo LauncherSequence
- Current Pawn Rebind / Old Pawn Event Isolation

부분 충족:

- Production SpeedGauge의 Gear Slot은 실제 UE 5.8 Chaos `GetCurrentGear()`를 Provider가 받아 `R / N / 실제 전진 단수`로 표시하는 경로까지 Technical PASS다. RPM upstream도 Technical PASS다. Current Engine RPM은 Chaos `GetEngineRotationSpeed()`, Maximum은 Chaos `EngineSetup.MaxRPM`과 같은 source인 VehicleData `EngineMaxRPM`, Redline은 신규 explicit `VehicleMovementConfig.RedlineStartRPM`에서만 읽는다. Redline 0은 미설정이고 explicit 값은 `EngineIdleRPM < RedlineStartRPM < EngineMaxRPM`을 Validator가 강제한다. Accepted Gauge mapping은 실제 Redline→0.85, EngineMaxRPM→1.0이며 `CurrentRPM/MaxRPM` 또는 `EngineMaxRPM*0.85`를 Redline으로 재해석하지 않는다.
- `EquipmentPresetData.DisplayName`은 기존 Fitting ViewData에서도 사람 읽기용 장비 이름으로 소비되고 있어 Player-facing 계약으로 정합화했고, 현재 호환 활성 프리셋의 비어 있지 않은 DisplayName만 Weapon HUD에 연결했다. 빈 이름 또는 비호환 상태에서는 `Unavailable`이며 내부 `WeaponId`·`EquipmentId`·`MountProfileId`·AssetName fallback은 없다.
- 기존 finite Ammo·ReserveAmmo·Cooldown·Reload·LauncherSequence 실제 ViewData를 additive `FCFWeaponResourceHUDData / ResourceChannels`로 투영하는 공통 Data Foundation은 구현·검증됐다. Production Presenter도 ResourceChannels를 우선 소비하고 채널이 없는 legacy ViewData만 기존 필드를 fallback하도록 migration했다. 이어 Stage A에서 `ECFWeaponResourcePresentationRole / FCFWeaponResourcePresentationEntry / BuildWeaponResourceEntries()`를 추가해 raw ResourceChannels의 배열 순서나 개수를 그대로 Visual Row로 쓰지 않고 Compact 역할을 Projection한다. 기존 `ResolveAmmoPresentation / ResolveReserveAmmoPresentation / ResolveWeaponStatusPresentation / ResolveLauncherSequenceDisplay`와 LauncherSequenceRevision lifecycle을 그대로 재사용하며 Launcher Active/terminal은 Primary, 그동안 Ammo는 Secondary, 일반 상태는 FireState 1개다. ReserveAmmo는 Header owner를 유지한다. Production Asset은 Stage A에서 변경하지 않았다.

실제 미구현·상위 Runtime/Presentation 선행:

- Production `WBP_CFSpeedGauge` visual binding은 Technical PASS다. 기존 21 Tick을 runtime Percent sink로 사용하고 Redline 미설정/invalid에서는 fill0 reset한다. 남은 RPM-specific 범위는 authoritative 대표 VehicleData `RedlineStartRPM` authoring 결정과 USER Visual이며, Redline 수치는 임의 생성하지 않는다.
- Weapon Heat P0 Runtime/HUD Resource Projection은 Technical PASS다. 남은 Heat 범위는 실제 사용할 saved WeaponData의 명시 tuning authoring과 USER Visual이다.
- Player-facing Weapon Selection Runtime/HUD source, truthful Production Rail Visual Consumer와 실제 숫자 1~9 direct ordinal 입력은 Technical PASS다. Applied Fitting fixed order + SelectedWeaponIndex를 사용하고 내부 MountProfileId를 UI에 노출하지 않는다. Rail USER Visual만 representative persisted multi-weapon content가 실제 생길 때까지 Deferred다.
- WeaponCharge P0 Runtime/HUD는 Technical PASS다. VehicleBattery는 UI-P0-06 미구현 항목이 아니라 future shared-power Gameplay feature dependency이며 현재 Provider가 없으므로 Unavailable/Collapsed를 유지한다.
- Stage B Production WeaponPanel Dynamic Resource Visual은 Technical PASS다. `UCFHUDPresenter v1.11.0`이 `FCFWeaponResourcePresentationEntry`를 ViewData 적용당 정확히 한 번 소비하고 `CFUIHUDProdEditorBridge v1.6.0`이 Primary 1 + Secondary 최대 2 + FireState 1 의미 슬롯을 생성한다. raw `ResourceChannels` 직접 0~N Row 생성과 기존 고정 Ammo/Heat/Cooldown/Launcher Row의 동시 잔존은 금지한다. 저장 Asset도 targeted Apply와 fresh AssetDump에서 새 의미 구조로 확인됐다.

현재 계약 교정:

- 실제 `UCFVehicleDefenseComp`가 Shield·Armor를 공급하므로 구형 `Mock Damage Provider`는 구현 대상이 아니다.
- Accepted `InGameUIVehiclePanelSpec`에 따라 고정 VehiclePanel은 SpeedGauge / ArmorBodyMap / ShieldRow / IntegrityRow만 유지한다. 부품 손상은 실제 Module Damage Runtime 이후 Alert/Compact Chip/Vehicle Detail 중 별도 설계한다.
- `HeatPerShot`, `MaxHeat` 같은 정적 WeaponData로 현재 Heat를 추정하지 않는다.
- 실제 Ammo 수량은 CF-FQ-031 Runtime Snapshot만 사용한다.

이번 최소 Source 범위:

- `FCFWeaponHUDData`의 `VehicleBatteryAvailability`는 실제 Provider 부재를 `Unavailable`로 유지하며 가짜 값이나 Widget Row를 만들지 않는다. `WeaponChargeAvailability`는 actual per-weapon Charge Runtime이 있을 때만 Known/KnownZero로 전환한다.
- `UCFHUDDataProvider`는 Current Engine RPM/Current Gear를 `VehicleDriveComp → UChaosWheeledVehicleMovementComponent` 실제 Runtime에서 읽고 속도나 입력에서 추정하지 않는다. RPM Maximum/Redline은 Current Pawn `VehicleData`의 `EngineMaxRPM`/`RedlineStartRPM` authored source만 추가 소비한다.
- Weapon HUD 이름은 현재 호환 `EquipmentPresetData.DisplayName`만 사용하고 내부 ID/AssetName fallback을 금지한다.
- `FCFWeaponResourceHUDData / ResourceChannels`는 실제 Ammo·ReserveAmmo·WeaponCharge·Cooldown·Reload·LauncherSequence·Heat만 additive 투영하며 VehicleBattery 채널은 실제 shared-power Runtime 전 생성하지 않는다.
- Stage A `CarFight.UI.UI_P0_06` focused 6건 `MissingProviderAvailability / ResourceChannelProjection / ResourcePresentationProjection / ResourcePresenterParity / VehicleDriveRuntimeViewData / WeaponDisplayNameRuntimeViewData`는 6/6 PASS / Failure 0, Official Build `4c043a73e3054baab5f725b18cd7de6c` Exit 0 PASS, Result SHA-256 `39be662e7125ef4972574a75b433119283b7a279fbab56e511b140aafd6c70fe`이며 Stage B에서 별도 재연하지 않는다.
- Stage B 신규 `ResourceVisualSlotContract`는 실제 Production HUD의 새 의미 슬롯, legacy Row 부재, Ammo/Cooldown → Launcher Active → terminal 첫 적용 → 같은 terminal ViewData 다음 적용의 lifecycle을 검증한다. 첫 process `ccd0ce6a60f449eb88929ccbdc275d6b`의 FAIL은 Production 기능이 아니라 테스트가 공통 `HitTestInvisible` 표시 정책을 `Visible`로 기대한 5건뿐이어서 `CFHUDDataTests.cpp v1.14.0`에서 expectation만 교정했다. 최종 Official Build `0bc2f086d71b4a57933326757d88b50d` PASS 뒤 process `2dc5eb2a394b4053875f548cec2204bc`가 1/1 PASS / Failure 0, Result SHA-256 `78bef76e45eb93199d1e7bba0bed6d0a91802c513568bcab78568e77508f6611`이다.
- 전체 Production 10 Asset 재작성을 피하는 `WeaponPanel-only` targeted mode로 process `b600bd339b0348b5a3001c01a541484a`가 existing `WBP_CFWeaponPanel` 정확히 1개만 rebuilt/compiled/saved했다. Report SHA-256 `6356715d0ab2503c78c6e31aec31f3b494b6f6e1405958481c7ed9659e06baf1`, other Production mutation 0이다. fresh AssetDump dataset `adset_v1_9ca90ab2e764ee02cda305b45e91bb3b.23e84f05ff5481cd492c956f`에서 fingerprint `CFB0652D`, 새 Compact 의미 구조와 legacy fixed Row 부재를 persisted evidence로 확인했다.
- RPM upstream contract는 `CFVehicleData.h v1.27.0`, `CFHUDViewData.h v1.9.0`, `CFHUDDataProvider.cpp v1.10.0`, `CFHUDPresenter.h v1.11.0 / .cpp v1.12.0`, `CFVDAValidator.cpp v1.5.0`과 DAUTH Current 118 compatibility로 적용했다. Official Build `745dba430bcc47c2925c841a0c5a6686` PASS, 신규 RPM exact Automation 2건 각 1/1 PASS, Registry/Batch bounded compatibility 3건 각 1/1 PASS다. Production Asset mutation은 0이다.
- 기존 Speed·Defense·Cooldown·finite Ammo·Ripple·Salvo USER PASS와 Stage A 6/6·Stage B 검증은 반복하지 않는다.

종료 조건:

- 현재 실제 Runtime이 제공하는 차량/무기 HUD 채널은 Production Presenter 경로에서 유지된다.
- Provider 없는 채널은 추정 없이 Unavailable/Collapsed를 유지한다.
- 향후 VehicleBattery shared-power Runtime이나 richer per-weapon identity/resource source가 생기면 같은 ViewData/Presenter 경계에 actual 값을 additive 연결한다. WeaponCharge/Heat는 이미 이 경계에 연결됐다.
- 무기별 자원 조합은 실제 Resource Provider 계약이 준비된 뒤 전용 무기별 Widget 추가 없이 공통 채널로 표시한다.

---

## 6. M4 — Target Panel과 Radar

### UI-P0-07

구현:

- 고정 Target Panel
- `NO TARGET`
- 미공개 `???`
- 가시 식별과 Scan 공개 전환
- 선택 타겟 파괴·해제 처리

의존:

- CF-FQ-026 선택 대상 계약
- Target Knowledge Provider

### UI-P0-08

현재 상태:

```text
UI-P0-08A Radar Range Foundation — Technical PASS
UI-P0-08B Production RadarPanel Consumer — Technical PASS
Radar Zoom Input — Technical PASS
Screen-Off Selected Target Edge Marker — Technical PASS
현재: UI-P0-08 USER Visual / Zoom Feel Validation
```

구현:

- Heading Up Radar
- Sensor Contact 목록
- 선택 타겟 강조
- Scanner-owned 단계식 Display Range와 Provider-local Zoom
- range-out 선택 Contact Radar edge 방향 표시
- Production RadarPanel dynamic Contact/Range consumer
- 화면 밖 방향 표시
- 탐지 신뢰도와 마지막 위치 확장 슬롯

UI-P0-08A 완료 계약:

- Radar Display Range Profile은 Scanner Data가 소유하고 Sensor 탐지 성능과 분리한다.
- Sensor Runtime은 Range Profile을 Applied copy로 고정한다.
- Provider는 `FCFRadarHUDData`에 Display Range/Maximum Detection Range/Preset 상태와 Heading-Up normalized Contact를 제공한다.
- 범위 밖 일반 Contact와 선택 Contact의 표시 의미를 `bInsideDisplayRange`, `bShowSelectedEdgeMarker`, `SelectedEdgeDirection`으로 분리한다.
- Production Widget은 이 ViewData를 소비하며 World Actor 검색이나 Sensor Detection/Knowledge 재계산을 하지 않는다.

주의:

Radar가 월드 Actor를 직접 검색하는 임시 구현을 최종 구조로 채택하지 않는다.

---

## 7. M5 — View Mode

### UI-P0-09

현재 상태:

- `UI-P0-09A View Mode / Direction Foundation` — Technical PASS
- `UI-P0-09B Production View Mode Consumer` — Technical PASS
- `UI-P0-09C Alert Lifecycle` — Technical PASS
- `UI-P0-09D Nested Style Context` — Technical PASS
- `UI-P0-09` — Technical Complete
- UI-P0-08 Radar/Edge Visual·Zoom Feel — USER Deferred/Pending, USER PASS 미부여

완료 계약:

- `FCFViewModeHUDData`가 기존 VehicleCamera/Aim Runtime의 Camera Mode, Vehicle Heading, Camera 상대 Yaw/Pitch, Turret 상대 Yaw/Pitch와 Turret Aligning을 전달하며 Gameplay Camera/Aim을 변경하지 않는다.
- Production HUD는 기존 ReticleLayer 아래 Vehicle Direction Image만 additive 소비하고 Root 7 direct child 구조와 기존 Command/Turret Reticle을 보존한다.
- Alert는 기존 stable `AlertKey` dedupe/Priority를 유지하면서 Style Notice 2초·Warning 3초·Critical Persistent lifecycle을 소비하고 repeated refresh가 시간을 리셋하지 않는다.
- Root가 해석한 Style/Density/Geometry/Typography Context는 중첩 `CFStyledWidgetBase`로 전달되며 child Content·Gameplay lookup과 same-context 중복 Refresh는 0이다.
- final combined Build `f209faafb10c4df1a446bf294f1f75ce` PASS, 09C `1fc3c160e74644079459ce3dab8074d4`, 09D `693b4a267fb5474e8b7a04b4b6b4e760`, 09B regression `094dab7a01d6479286a0e0995062bc17` 각각 1/1 PASS다.

P0 잔여:

- UI-P0-09 내부 잔여 없음. 다음은 `UI-P0-10 Integration Validation`에서 완료 UI-P0-03~09의 통합 회귀와 미검증 조합만 판정한다.

확장 계약:

- Turret View 전환 이벤트
- 포탑 시점용 정밀 조준 슬롯
- 차량 핵심 상태 축소 레이아웃

운전석 시점은 추가하지 않는다.

---

## 7-1. F1 — UI-COMMONUI-GATE

### 목적

CommonUI를 시각 디자인 도구가 아니라 전체 화면 메뉴의 입력·내비게이션·Back·화면 Stack 수명 프레임워크로 도입할 필요가 있는지 판단한다.

### 실행 시점

타이틀, 차고, 피팅, 임무 선택과 결과 화면 중 첫 전체 화면 Flow의 구현 범위가 확정된 직후, 해당 화면 코드를 만들기 전에 실행한다.

현재 인게임 HUD P0, View Data와 `UI-DESIGN-GATE`를 차단하지 않는다.

### 판단 기준

- 중첩 Screen·Modal Stack과 Back 복원 필요성
- 게임패드 중심 Focus·내비게이션 복잡도
- 입력 장치별 Glyph와 장치 전환 표시 필요성
- Activatable Screen 수명 계약을 공유할 화면 수
- 현재 `UCFUISubsystem`만 유지할 때 발생하는 중복 구현량

### 허용 결론

```text
A. UMG + UCFUISubsystem 유지
B. 상시 전투 HUD는 UMG, 전체 화면 Menu는 CommonUI인 혼합 구조
C. 충분한 근거가 있을 때 전체 화면 기반을 CommonUI로 확장
```

### 불변 조건

- 디자인 개선 완료 여부와 CommonUI 도입 여부를 같은 Gate로 묶지 않는다.
- View Data와 Presenter를 CommonUI 클래스에 종속시키지 않는다.
- `UCFUISubsystem`의 외부 화면 요청 진입점을 유지한다.
- 상시 전투 HUD를 일괄적으로 `CommonActivatableWidget`으로 변환하지 않는다.

---

## 8. M6 — 통합 검증

### UI-P0-10

검증 행렬:

| 범주 | 조건 |
|---|---|
| 해상도 | 1920×1080, 2560×1440, 3440×1440, 5120×1440 |
| 입력 | 키보드·마우스, 게임패드, 장치 전환 |
| 수명 | Pawn 생성, 파괴, 교체, 레벨 재시작 |
| Pause | 전투·AI·Projectile·Timer·열·Lock 정지와 복귀 |
| Reticle | 기존 Aim/FireFeedback 전체 회귀 |
| Target | 선택, 해제, 파괴, 정보 공개 |
| Radar | 접촉 진입, 이탈, 선택 강조, 화면 밖 표시 |
| Weapon | 무기 전환, 자원 채널 변경, 재장전·쿨다운 |

현재 판정:

- 해상도: `ResolutionLayoutContract`가 1920×1080 / 2560×1440 / 3440×1440 / 5120×1440과 Project DPI Scale에서 Production 6 Panel 화면 내 유지·비겹침, ReticleLayer full-stretch를 PASS했다.
- 입력: Keyboard/Mouse·Gamepad mapping coexistence와 기존 Pause/Gamepad USER evidence를 보존한다. 별도 InputDevice/Glyph switching Runtime은 Current에 없어 장치 전환 표시 행은 N/A다.
- 수명: 기존 UI-P0-02~05 USER PASS의 Root/Menu 레벨 재진입, Pawn Rebind·Old Pawn isolation, AimReticle/Target Marker 단일 수명과 final broad contract를 보존한다.
- Pause: Launcher·Projectile·Timer와 P0-06 이후 Weapon Heat·Charge·TargetUse freeze contract가 PASS다. AI Runtime과 Lock-on Gameplay Runtime은 Current에 없어 N/A다.
- Reticle: UI-P0-04 USER PASS를 보존하고 fresh PIE actual pixel에서 중앙 Reticle 단일 생성을 객관적으로 재확인했다.
- Target/Radar: Sensor HUD/DestroyedHold 1/1, ContactLifetime/Reacquire 2/2, UI broad의 Target Knowledge·Radar Range/Zoom/Screen-edge 계약이 PASS다.
- Weapon: Ammo Reload 1/1, Ammo HUD/Rebind 1/1과 UI broad의 Weapon Selection·Cooldown·Heat·Charge·Resource 계약이 PASS다.
- Build/Automation: Official Build `1a632fcdac38424e9750633a95204d76` PASS, final `CarFight.UI` `2fdf9866d08e4ae2aa464c21e6a373bb` 43/43 PASS다. `RunUIAutomation.ps1 v1.2.0`은 UI/Ammo/Sensor만 허용하며 Setup filter를 fail-closed한다.
- fresh PIE: process `14c0d695567b4569814a39229e77fcc9`가 AI-owned/retry0으로 시작됐고 RuntimeRead에서 `TestMap`, LocalPlayer1, `CFPlayerController`, `BP_CFVehiclePawn`, Drive/Aim/Weapon/Ammo/Health/Defense/TargetSelect/Sensor 핵심 Component를 확인했다. actual pixel에서 Vehicle/Radar/Weapon/Target HUD와 단일 Reticle의 실제 생성도 확인했다.
- 이 PIE 결과는 **AI Runtime Technical Validation**이며 USER Visual·UX·조작감·Zoom Feel PASS가 아니다. UI-P0-08와 D1-11-ART의 USER Deferred/Pending은 유지한다.

UI-P0-10 상태: **Technical Complete**

### UI-P0-11 — Complete

승격 결과:

- main_game `Document/Systems/UI/InGameUI.md v1.0.0`을 Current System으로 신규 등록해 검증된 UI Root·HUD·Pause·Radar·Target·Weapon 계약만 기록했다.
- `Document/Systems/UI/AimReticle.md v1.9.0`을 실제 `UCFUISubsystem` HUD Layer singleton·Current Pawn Rebind·Weak Pawn 구조로 교정했다.
- `Document/Systems/SystemIndex.md v1.21.0`에 InGameUI Current owner를 공식 등록했다.
- 실제 Runtime Source가 없는 VehicleBattery, AI/Lock, player-facing future data를 구현된 Current처럼 쓰지 않았다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 비차단 Deferred/Pending으로 보존했다.
- Product Source·Config·Content Asset mutation 0. UI-P0-10 evidence replay 0.
- 판정: **UI-P0-11 Complete / M6 Done / CF-FQ-032 Done / Plan Historical + Retained Path**.

---

## 9. Historical 중간 체크포인트 (Superseded)

Runtime 체크포인트와 디자인 체크포인트를 분리한다.

```text
Runtime
- UI-P0-02 Partial USER PASS
- 남은 Runtime Lifetime PIE Pending
- UI-P0-03 HUD 데이터 계약 Not Started

Design
- D1-01~04 승인 완료
- D1-05 Style·Base Widget·Density·Weapon Compact Token User Accepted
- D1-06 Pretendard + IBM Plex Mono / Solid Core + Tactical Cut Icon User Accepted
- D1-07 `SR-1080-16` USER PASS
- D1-08V Official Build·Automation PASS
- D1-09A Font Binding·HUD Layout·Density C++ + Subsystem Config/Fallback PASS
- 다음 실행은 D1-09B 1080p Style·Density·Layout·Font·Icon Asset + Config 연결
- D1-10~12는 D1-09B PASS 뒤 순차 진행
- SR-A·C·D·E 및 SR-1440-16·SR-1440-21·SR-1440-32 = Non-blocking Follow-up / Deferred Expansion
```

사용자 PIE 결과 없이 `UI-P0-02 PASS`, M1 완료 또는 UI Systems 승격으로 판정하지 않는다.
D1-05 기본 Token, D1-06 Font·Icon 기본 Preset과 D1-07 `SR-1080-16` 사용자 Static Review는 승인 완료이며 D1-08V·D1-09A도 기술 검증 PASS다. 다만 D1-09B~12 실제 Unreal 자산화와 Font/Icon 고지 보존·Visual 검증 전에는 UI-DESIGN-GATE 전체 PASS로 판정하지 않는다.
`UI-P0-03`을 착수할 때도 현재 C++ Pause Menu와 기존 Pawn 소유 AimReticle·TargetSelect 경로를 보존한다.

---

## 10. 권장 착수 순서

현재 프로젝트 우선순위를 유지하면 다음과 같이 해석한다.

```text
현재 Active CF-FQ-029 완료
→ CF-FQ-030 / CF-FQ-031 / CF-FQ-032 중 사용자 선택
```

UI를 선택할 경우에도 한 번에 전체 HUD를 구현하지 않는다.

```text
PlayerController / Input
→ World별 Root
→ Full Pause
→ Data Contract
→ AimReticle 이전
→ TargetSelect 이전
→ UI-DESIGN-GATE
→ Vehicle/Weapon
→ Target Knowledge/Radar
→ Alert/Style/View Mode
→ Integration

별도 전체 화면 Flow 기획
→ UI-COMMONUI-GATE
→ UMG 유지 또는 HUD UMG + Menu CommonUI 혼합 구조 결정
```

피팅은 이 로드맵에 추가하지 않는다.

```text
CF-FQ-032에서 준비
- Screen Layer와 전체 화면 전환 진입점
- 공통 UI 입력·Focus·Style·표현 타입

별도 피팅 Plan에서 기획
- Loadout / Draft / Validate / Commit
- 호환성·중량·성능 계산
- 저장·불러오기
- Garage Preview Actor
- Fitting Screen과 Preview View Data
- 출격 Runtime Snapshot
```

피팅 기능 ID와 Plan은 실제 피팅 기획 착수 시 새로 부여한다.

---

## 11. Changelog

### v0.26.35 - 2026-08-22

- 상단 `2026-08-18 Current Resume Checkpoint`를 Historical snapshot으로 재분류해 내부의 당시 Systems Promotion 버전과 DAUTH PASS 2 checkpoint가 현재 상태로 오인되지 않도록 했다.
- Historical evidence는 그대로 보존하고, 현재 구현 owner 포인터만 InGameUI v1.1.0 / AimReticle v1.10.0 / SensorContact v1.2.0으로 동기화했다.

Migration: snapshot 안의 v1.0/v1.9/v1.1 Systems와 DAUTH 초기 checkpoint는 당시 evidence다. 현재 판단은 main_game Current 문서를 우선한다.

### v0.26.34 - 2026-08-22

- CF-FQ-032 post-closure code review remediation Technical PASS를 Historical checkpoint에 추가했다. P1 2건 + P2 3건만 교정했으며 CF-FQ-032 Done과 USER Deferred 상태는 유지한다.
- final Build `bd3640c616794cd7a54cc8a8afa4b021`, focused UI-P0-07 2/2·UI-P0-08 4/4·Alert 1/1·Sensor HUD 1/1, broad UI 44/44·Sensor 14/14 PASS다.
- Current owner를 main_game `Systems/UI/InGameUI.md v1.1.0`, `Systems/UI/AimReticle.md v1.10.0`, `Systems/Targeting/SensorContact.md v1.2.0`으로 갱신했다.

Migration: 이 Roadmap은 계속 Historical + Retained Path다. remediation 상세 구현은 Current Systems와 실제 Source를 우선한다.

### v0.26.33 - 2026-08-22

- `UI-P0-11 Systems Promotion`을 완료했다. main_game `Systems/UI/InGameUI.md v1.0.0` Current 승격, `AimReticle.md v1.9.0` UISubsystem singleton/Rebind 교정, `SystemIndex.md v1.21.0` 등록을 완료했다.
- M6를 Done으로 닫고 `CF-FQ-032`을 현재 P0 범위 Done으로 전환했다. UI-P0-11은 문서 전용 Current Knowledge Promotion이며 Product Source·Config·Content Asset mutation은 0이다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 사용자의 진도 우선 결정에 따른 비차단 Deferred/Pending을 유지하며 USER PASS로 확대하지 않는다.
- 이 Roadmap은 Historical + Retained Path다. 현재 구현은 Systems와 실제 Source/Asset을 우선하며 새 Active Feature는 main_game FeatureQueue/사용자 선택이 소유한다.

Migration: 아래의 오래된 중간 checkpoint와 next-action은 Historical evidence로만 읽는다. CF-FQ-032을 재개하려면 기존 old next-action을 복원하지 말고 Current Systems를 감사한 뒤 새 lifecycle을 정의한다.

### v0.26.32 - 2026-08-22

- M6의 `UI-P0-10 Integration Validation`을 Technical Complete로 닫고 `UI-P0-11 Systems Promotion`을 다음 Runtime Gate로 이동했다. M6 전체는 UI-P0-11이 남아 있으므로 Done으로 확대하지 않는다.
- `CFHUDDataTests.cpp v1.29.0` ResolutionLayoutContract의 4개 기준 해상도, `CFUIPauseTests.cpp v1.2.0` Weapon Resource Pause guard, final `CarFight.UI` `2fdf9866d08e4ae2aa464c21e6a373bb` 43/43 PASS와 Ammo/Sensor focused regression을 통합 행렬 evidence로 고정했다.
- fresh AI-owned PIE `14c0d695567b4569814a39229e77fcc9`에서 RuntimeRead로 `TestMap` LocalPlayer·Controller·Pawn과 핵심 차량 Component를 확인하고 actual pixel로 Vehicle/Radar/Weapon/Target HUD와 단일 Reticle 생성을 객관적으로 확인했다. 이는 AI Runtime Technical Validation이며 USER Visual·Feel PASS가 아니다.
- Current Product에 AI Controller/BehaviorTree와 Lock-on Gameplay Runtime이 없어 Pause의 AI/Lock은 N/A, 독립 InputDevice/Glyph switching state가 없어 장치 전환 표시는 기존 Enhanced Input coexistence 계약으로 판정했다. UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 Deferred/Pending을 유지한다.

Migration: UI-P0-10 완료 evidence는 새 failure 없이 반복하지 않는다. 다음은 UI-P0-11에서 검증된 Current UI Root/HUD/Pause 계약만 Systems로 승격하며 USER Deferred 항목을 Current 완료 기능으로 기록하지 않는다.

### v0.26.31 - 2026-08-21

- UI-P0-09B~D를 Technical PASS해 UI-P0-09를 Technical Complete로 닫았다. 09B는 ReticleLayer additive Vehicle Direction consumer, 09C는 Style Alert duration lifecycle, 09D는 nested Styled Widget Context propagation이다.
- combined final Build `f209faafb10c4df1a446bf294f1f75ce` PASS, 09C focused `1fc3c160e74644079459ce3dab8074d4`, 09D focused `693b4a267fb5474e8b7a04b4b6b4e760`, 09B regression `094dab7a01d6479286a0e0995062bc17` 각각 1/1 PASS다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel USER Deferred/Pending은 그대로 보존하고 formal next technical Gate를 `UI-P0-10 Integration Validation`으로 이동했다.

Migration: UI-P0-09A~D를 새 failure 없이 반복하지 않는다. M6 UI-P0-10에서 완료 Runtime/Presentation의 통합 회귀와 해상도·입력·수명 조합만 검증한다.

### v0.26.30 - 2026-08-21

- UI-P0-09A View Mode / Direction Foundation을 Technical PASS로 추가했다. 기존 VehicleCamera/Aim Runtime을 새 Gameplay 모드 없이 HUD ViewData로 투영하고 final Build `a78e59503f6e45ce98f952941ea2969a`, focused `ba3fc0bb5d27448db14c0f28b9e76997` 1/1 PASS를 확보했다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel은 USER Deferred/Pending으로 보존하고 현재 기술 Gate를 UI-P0-09B Production View Mode Consumer로 이동했다.

Migration: UI-P0-09A 기술 증거는 새 failure 없이 반복하지 않는다. 09B는 Production HUD의 실제 표시 gap만 감사·구현하고 Alert/Style 또는 Gameplay Camera/Aim을 함께 재구현하지 않는다.

### v0.26.29 - 2026-08-21

- `UI-P0-08 Screen-Off Selected Target Edge Marker`를 Technical PASS로 닫고 UI-P0-08 전체 기술 범위를 Technical Complete로 승격했다. Selected Target은 Camera View Space 기반 방향을 Safe Region 경계와 교차시켜 2-Corner Edge Bracket으로 표시하며 Candidate offscreen hide는 유지한다.
- `UCFUISubsystem v1.9.0`이 공용 HUD Visual/Style Data의 `RadarSelectedEdgeBracket`, `AccentTactical`, `SafeMargin`을 주입하고 `UCFTargetSelectWidget v1.2.0`은 runtime transient `Image_SelectedEdgeMarker`만 생성한다. persisted `WBP_TargetSelect` Designer 구조를 저장 변경하지 않는다.
- Official Build `4eecb33895574b87b4726a544f029835` PASS, 신규 ScreenEdge contract `ed5b04ade60c4fc086576610edfbfbca` 1/1, 기존 TargetMarker regression `9a0a1cd97c804321b36313ac0e085b1b` 1/1 PASS다. fresh post AssetDump에서도 persisted WBP 8-node 구조와 runtime Edge save0을 확인했다.
- 현재 Runtime Gate는 `UI-P0-08 USER Visual / Zoom Feel Validation`이다. 시각 품질·조작감 USER PASS는 추정하지 않으며 통과 후 UI-P0-09로 이동한다.

Migration: UI-P0-08A/B/Zoom/Screen-edge 기술 증거는 새 실패 없이 반복하지 않는다. `RadarSweepMaterial`은 비차단 후속으로 유지한다.

### v0.26.28 - 2026-08-21

- `UI-P0-08 Radar Zoom Input`을 Technical PASS로 동기화했다. Official Build `18ddc2ba098d44f68721a108e3f43f8f`, exact setup `5ecd196794d9453091c0ed27c03f0e60` 1/1, fresh persisted `MouseScrollUp=+1 / MouseScrollDown=-1 / MouseWheelAxis0 / Gamepad0`, read-only Zoom·Range·WeaponSelect focused regression 각 1/1 PASS를 current evidence로 사용한다.
- Zoom은 `Pawn → UCFUISubsystem → UCFHUDDataProvider`로 Provider-local Display Range만 변경하며 Sensor detection range와 Scanner Profile은 수정하지 않는다.
- 현재 Runtime Gate를 `Screen-Off Selected Target Edge Marker`로 이동했다. 기존 `UCFTargetSelectWidget`의 선택 Target offscreen hide를 UI-P0-08 범위에서 camera View Space 기반 방향 표시로 확장하되 Candidate offscreen hide와 TargetSelect Gameplay ownership은 유지한다.
- Radar/Screen-edge 시각 품질과 Zoom 조작감은 USER PASS로 추정하지 않는다.

Migration: UI-P0-08A/B/Zoom 완료 Build·Asset·Automation evidence는 새 실패 없이 반복하지 않는다. 다음 slice는 TargetSelect Gameplay나 Sensor Runtime을 재구현하지 않고 selected target spatial projection만 최소 확장한다.

### v0.26.27 - 2026-08-20

- `UI-P0-08B Production RadarPanel Consumer`를 Technical PASS로 닫았다. Production Radar는 `FCFRadarHUDData`만 소비하는 `UImage` runtime Contact pool, Display Range Text, Player Marker, in-range 4-Corner Selected와 range-out 2-Corner Edge를 사용하며 Text glyph icon과 Sensor Runtime 재계산은 0이다.
- Radar SourceArt 7종 process `a08892dd13a648cca7c1a92f8ab23527` Technical PASS, Official Build `f7ac2bea527d4207ac0c45bd0a40d738` PASS, targeted `-RadarOnly` apply `a059a57d25114561934f824a7949186d` exact 9 Asset, fresh persisted RadarPanel/VisualData readback PASS를 확보했다.
- `CarFight.UI.UI_P0_08` process `9ba3a1d151d04b649d4da510bcab8b80`는 Range Foundation + Radar Contact Consumer 2/2 PASS다. `RadarSweepMaterial`은 Active Scan Sweep 후속 표현이라 B2 종료 조건에 포함하지 않는다.
- UI-P0-08 전체와 USER Visual PASS는 아직 아니다. 현재 Gate는 기존 Mouse Wheel 예약을 실제 Provider-local Zoom In·Out에 연결하고 Production Radar의 가독성·선택 edge·Zoom 조작감을 USER가 확인하는 단계다.

Migration: UI-P0-08A/B의 완료 Build·Asset apply·focused tests를 새 실패 없이 반복하지 않는다. 다음은 실제 Zoom Input 연결이며 게임패드 mapping은 명시 결정 전 추가하지 않는다.

### v0.26.26 - 2026-08-20

- `UI-P0-08A Radar Range Foundation`을 Technical PASS로 닫았다. Scanner-owned Range Profile, Applied-copy isolation, Provider-local Zoom In/Out, Heading-Up normalized Contact와 selected range-out edge direction 계약을 구현했다.
- 최종 Official Build `07b963ac33614ab2ad43193c8cd950b5` PASS, `CarFight.UI.UI_P0_08.RadarRangeFoundationContract` process `e0476b15b86843ef91640752f47cf064` 1/1 PASS, `CarFight.Sensor.SEN_P0_06.HUDSnapshot` regression process `9b97d06df6544cab91fe6cb094ed3cb8` 1/1 PASS다. Content Asset·Sensor detection mutation은 0이다.
- UI-P0-08 전체는 In Progress를 유지한다. 현재 Gate는 `UI-P0-08B Production RadarPanel Consumer Audit`이며 persisted RadarPanel·Presenter·기존 offscreen projection 기반을 감사한 뒤 actual dynamic Contact/Display Range/selected edge visual을 최소 연결한다. 실제 Zoom 입력은 이 consumer audit 뒤에 연결한다.

Migration: UI-P0-08A 검증은 새 실패 없이 반복하지 않는다. Radar Presentation은 `FCFRadarHUDData`만 소비하고 Sensor Runtime이나 World Actor 검색을 Widget에서 재구현하지 않는다.

### v0.26.25 - 2026-08-20

- `UI-P0-07 Target Knowledge`를 Technical Complete로 닫았다. Existing TargetSelect 선택/파괴·해제 수명과 Sensor Snapshot Knowledge를 재사용하고 Production TargetPanel Consumer만 최소 완성했다.
- Official Build `72893293dd0f4dd98fbc93d96f3466dd`는 `CFHUDPresenter.cpp`·`CFHUDDataTests.cpp`를 실제 컴파일해 `Result: Succeeded`, focused `CarFight.UI.UI_P0_07.TargetKnowledgePanelContract` process `e82f8ef659d34900a46ee32e8f98e94f`는 1/1 PASS·Engine Exit 0이다.
- Target Armor는 authoritative source가 없으므로 숨김을 유지한다. D1-11-ART의 SpeedGauge·VehiclePanel·전체 Visual Review는 계속 Deferred이며 USER PASS로 확대하지 않는다.
- 현재 Runtime Gate를 `UI-P0-08 Radar·화면 밖 마커 Foundation Audit`으로 이동했다. 초기 감사에서 Sensor Snapshot→Radar ViewData의 실제 Contact/상대 위치/거리/선택 상태는 이미 존재하지만, 명시 Radar Range/Zoom 부재로 `NormalizedPosition`은 Unavailable이며 Production RadarPanel의 동적 Contact consumer는 아직 없다.

Migration: UI-P0-07 Build/Automation을 새 실패 근거 없이 반복하지 않는다. UI-P0-08은 Sensor Runtime을 재구현하지 않고 persisted RadarPanel 구조, 명시적 Radar 표시 범위/정규화 ownership, 동적 Contact·화면 밖 방향 consumer만 감사·구현한다.

### v0.26.24 - 2026-08-19

- VehiclePanel Visual Foundation의 P2 Technical Apply를 완료했다. SourceArt P2 9종을 Texture2D로 persisted import하고 `DA_CFHUDVisual_Default` Vehicle 9슬롯에 연결했으며 `WBP_CFSpeedGauge`·`WBP_CFArmorBodyMap`·`WBP_CFVehiclePanel`을 Build·Compile·Validate·Save했다.
- Official Build Job `74166fb3ad0442a5b10bbf02c6f69d12`는 `Result: Succeeded`이고 fresh AssetDump에서 Texture2D 9종, VisualData 연결, SpeedGauge 26 nodes, ArmorBodyMap 20 nodes, VehiclePanel 19 nodes를 확인했다. 기존 P1은 Technical reference only, Radar/Target/Weapon visual slot은 미변경이다.
- USER가 PC를 사용할 수 없어 Visual Review는 Pending이다. 다음은 실제 Designer/1920×1080 Production HUD에서 P2 Visual을 사용자 판정하고, PASS 뒤 formal runtime gate `UI-P0-07 Target Knowledge`로 이동한다. 완료된 P2 technical apply와 UI-P0-06 evidence는 새 결함 없이 반복하지 않는다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.20`과 `InGameUIHUDArtSpec.md v0.3.2`를 사용한다. USER Visual 승인 전 UI-P0-07을 자동 착수하지 않는다.

### v0.26.23 - 2026-08-19

- `Balanced Combat`을 VehiclePanel Visual Foundation의 USER APPROVED 시각 방향으로 확정했다.
- 실제 VehiclePanel Concept은 기존 896×416 승인 Layout과 비대칭 21 Tick RPM Gauge, 3자리 Speed, 중앙 좌향 차량+6방향 Armor, Shield/Integrity 전체폭 Bar 계약을 그대로 사용한다. 분위기 시안의 원형 계기판 구조는 채택하지 않는다.
- 기존 P1 9종은 Technical reference baseline으로만 보존하며 다음 Gate는 Actual VehiclePanel Concept USER Review다. 승인 전 Import·DA·Apply 0과 UI-P0-07 Ready/Not Started를 유지한다.

### v0.26.22 - 2026-08-19

- 사용자 결정으로 UI-P0-07 착수 전에 `D1-11-ART Visual Foundation / VehiclePanel Vertical Slice`를 현재 실행 Gate로 배치했다. UI-P0-07은 Ready/Not Started formal next Runtime Gate를 유지한다.
- 기존 P1 프로젝트 내부 생성 파이프라인과 9/9 Technical PASS를 재사용하며, 첫 Vertical Slice는 VehiclePanel 소비 8종의 USER Visual Review → 승인 자산 Import → DA 연결 → targeted Apply/Readback → persisted evidence → 1920×1080 USER Preview 순서로 고정했다.
- USER Visual 승인 전 Unreal Import·DA Connection·Production Apply는 수행하지 않으며 `T_UI_TargetBracket`은 후속 Target/Radar Visual 단계용 source로 보존한다.

### v0.26.21 - 2026-08-19

- UI-P0-06 중간점검에서 Roadmap의 원래 종료조건을 재확인했다. 현재 Runtime이 제공하는 차량/무기 채널을 Production Presenter에 연결하고 Provider 없는 채널을 `Unavailable/Collapsed`로 유지하는 것이 M3 완료조건이며, VehicleBattery Gameplay System 신규 구현은 M3 필수가 아니다.
- VehicleBattery를 future shared-power Gameplay dependency로 분리하고 UI-P0-06을 current-runtime 기준 `Technical Complete`로 전환했다. Battery owner/capacity/regen/consumer priority와 Weapon·Shield·Scanner 소비 계약은 별도 Gameplay feature에서 다룬다.
- Rail USER Visual, Heat/Charge tuning Visual, authoritative Redline/RPM Visual은 content-dependent Deferred follow-up으로 남기고 formal current Gate를 `UI-P0-07 Target Knowledge`로 이동했다. completed UI-P0-06 evidence는 관련 결함 없이 반복하지 않는다.
- `CFHUDDataProvider.h v1.7.0`, `CFHUDPresenter.h v1.15.0` stale Current 주석을 교정했다. 실행 코드·Asset mutation은 0이다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.17`을 사용한다. UI-P0-07~09는 VehicleBattery 구현을 기다리지 않는다. VehicleBattery는 별도 Gameplay power feature가 완성된 뒤 기존 ResourceChannel 계약으로 UI에 연결한다.

### v0.26.20 - 2026-08-19

- UI-P0-06 WeaponCharge P0 Runtime/HUD slice를 Technical PASS로 닫았다. explicit Maximum/Initial/PerShot/Recovery가 유효할 때만 활성이고 existing all-zero WeaponData는 Disabled 호환이다. WeaponComp가 per-weapon Charge와 Game-Time 회복을 소유하며 Pawn은 부족 시 `WeaponChargeInsufficient`, accepted result에서 한 발당 1회 소비한다.
- HUD는 actual `ResourceChannels::WeaponCharge`만 Compact `CHARGE N% / NO CHARGE`로 소비하고 VehicleBattery/정적 설정 fallback은 0이다. Secondary 최대2와 `Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY` 우선순위를 보존한다.
- final Official Build `2d9f33261d3c427187f75338b8f37f1d` PASS, exact `WeaponChargeRuntimeResourceContract` `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS / Result SHA-256 `df53c9732b3135f9eae230dc9f287d646e542179c0464e5b2ef1794b54e8e137`다. closure source readback에서 Charge Pawn hooks가 기존 Heat 동적 검증 경계와 같은 실제 fire 함수의 인접 hook임을 확인해 추가 protected-friend dynamic test는 만들지 않았다.
- saved Charge authoring, VehicleBattery, Heat tuning, Redline authoring, Rail USER Visual 상태는 변경하지 않았다. 현재 남은 실제 Gameplay Runtime은 VehicleBattery 하나다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.16`을 사용한다. WeaponCharge Technical PASS는 반복하지 않고 Production WeaponData의 all-zero Charge를 임의 authoring하지 않는다. VehicleBattery는 별도 shared-power dependency를 가진 후속 Runtime으로 유지한다.

### v0.26.19 - 2026-08-19

- Player-facing Weapon Select Input을 Technical PASS로 전진시켰다. `IA_SelectWeapon` Axis1D + 숫자1~9 direct ordinal, `Ordinal-1 → RequestSelectWeaponIndex`, Mouse Wheel Radar Range/Zoom 예약, gamepad 미지정을 고정했다.
- final Build `dec0757b745342b4b90ff5f17b761eb0` PASS, persisted IA fp `1A8A0402`, IMC fp `52D2D868` / mapping47→56, exact `WeaponSelectInputContract` `daeaf649669d48c79746a65cc44b1b89` 1/1 PASS다.
- Rail USER Visual은 current persisted representative가 모두 1무기라 Deferred했다. artificial 2무기 fixture는 test-only mass-valid preset 재사용까지 확인했으나 finite Ammo loadout까지 별도 구성해야 해 확대 중단했으며 실패 실행 생성 asset cleanup error0이다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.15`을 사용한다. Weapon Select Input Technical PASS는 반복하지 않고 Rail USER Visual은 representative persisted multi-weapon content가 생길 때만 재개한다.

### v0.26.18 - 2026-08-19

- UI-P0-06 truthful Weapon Rail Visual Consumer를 Technical PASS로 닫았다. old Production Rail의 Turret/Ammo/Reload semantic Image3를 weapon identity로 재해석하지 않고 제거했으며 선택 무기는 기존 Header/Selected Card에만 유지한다.
- 새 Rail은 비선택 actual selection만 Provider fixed order로 표시한다: 0 hidden, 1~3 original ordinal + `EquipmentPresetData.DisplayName`, 4+ first2 + `+N`; missing name은 generic `WEAPON`, fake icon/internal ID/per-nonselected resource summary는 0이다.
- Official Build `941ea18597b3483594be8de3317e68fd` PASS, targeted WeaponPanel-only apply `0617f7cbed97427a942c7a6f59936e22` exact asset1 / other Production0, fresh AssetDump saved fingerprint `26CFB205`, exact `WeaponRailVisualContract` `7cc9ddd5b2204725bb3ec45c200d649b` 1/1 PASS를 확보했다.
- 기존 Weapon Selection Runtime/HUD source, Heat/RPM/Stage A+B/USER PASS는 반복하지 않았다. Rail USER Visual과 Weapon Select Input Mapping은 Pending이며 Battery/Charge, Heat tuning, Redline authoring은 건드리지 않았다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.14`와 `InGameUIWeaponPanelSpec.md v0.7.0`을 사용한다. actual icon/resource source 전 name-only Rail을 유지하고 Technical PASS를 반복하지 않는다.

### v0.26.17 - 2026-08-18

- Applied Fitting weapon-bearing `ResolvedMounts` 고정 순서를 Player-facing Weapon Selection source로 확정하고 새 WeaponGroupId 없이 SelectedWeaponIndex 기반 Runtime/HUD source를 구현했다. 내부 MountProfileId HUD 노출은 0이다.
- per-weapon Cooldown/Heat 독립 보존과 비선택 Heat 냉각, Launcher WeaponChanged 정상 취소 후 WeaponComp+single Turret Visual 전환, EquipmentPresetData.DisplayName-only HUD 목록을 연결했다.
- final Build `62ffb62f69524bb18c3a9f11bb9f58f1` PASS, exact `WeaponSelectionRuntimeContract` `ff23cf6ac04e4b2c8cf0084b24173cdb` 1/1 PASS / Failure 0, Result SHA-256 `0ffa955f626d0de3f5ebf7b2ac594d7f8b039d268fe6efb2ab0730abfe77618c`다.
- Production Rail은 기존 의미 아이콘을 weapon slot으로 재해석하지 않고 Collapsed를 유지했다. Asset/Input mutation과 USER Visual 추가는 0이다. 남은 actual Runtime은 Battery/Charge다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.13`을 사용한다. 다음 WeaponPanel slice는 Rail Visual Consumer이며 actual weapon icon source가 없음을 보존하고 가짜 icon/내부 ID를 만들지 않는다.

### v0.26.16 - 2026-08-18

- UI-P0-06 남은 WeaponGroup·VehicleBattery·WeaponCharge·Heat fresh audit에서 Heat를 다음 독립 slice로 선정했다. WeaponGroup player-facing source, VehicleBattery shared runtime, WeaponCharge actual runtime/static input은 계속 미구현으로 분리했다.
- `HeatDissipationPerSecond` 기본 0 + `FCFWeaponHeatRuntime`으로 per-weapon CurrentHeat/냉각/Overheated를 구현하고 실제 승인 한 발당 Heat 1회 누적, MaxHeat `WeaponOverheated`, next-shot-headroom 회복을 연결했다.
- HUD는 실제 Heat Runtime만 Percent Resource Channel로 소비하며 Heat Secondary + `Reload > NoAmmo > Overheated > Cooldown/READY` FireState를 사용한다. Launcher Primary + Ammo/Heat Secondary 최대 2와 기존 Resource Projection lifecycle은 유지한다.
- final Official Build `0ecfed49ab3a4f41b349fc707c44e5f2` PASS, exact `HeatRuntimeResourceContract` `c736d1a6d6134a798a4b84452750e3d6` 1/1 PASS / Failure 0, Result SHA-256 `1abf0dc7a4788d6543c7933abef38433849ae57d569229cfab220f14937abad5`다. existing saved WeaponData는 냉각값 기본 0으로 Disabled이고 Asset mutation은 0이다.
- Heat tuning/USER Visual과 Redline authoring/RPM USER Visual은 Pending이다. Stage A/B, RPM completed evidence와 UI-P0-03~05 USER PASS는 반복하지 않았고 UI-P0-06 전체 Done으로 확대하지 않는다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.12`을 사용한다. Heat Runtime Technical PASS는 관련 결함 없이 반복하지 않고 실제 무기 tuning은 세 Heat authored 값을 함께 결정한다. 다음 미구현 Runtime 후보는 WeaponGroup, VehicleBattery, WeaponCharge이며 내부 ID나 임의 값을 사용하지 않는다.

### v0.26.15 - 2026-08-18

- UI-P0-06 RPM Gauge Production Visual Binding을 Technical PASS로 닫았다. 기존 저장 `WBP_CFSpeedGauge` 21 Tick을 구조 변경 없이 runtime Percent sink로 사용하며 explicit Redline/Maximum mapping이 유효할 때만 fill을 적용한다.
- Redline 0/Unavailable/invalid에서는 모든 Tick fill을 0으로 reset해 Max 기반 fallback과 stale Pawn 표시를 차단한다. persisted representative VehicleData는 Sedan Idle/Max 900/6500, TestSUV·DefenseSUV 900/6020이며 authoritative Redline이 없어 모두 0 Unconfigured를 유지했고 Asset mutation은 0이다.
- test-only C4458 이름 충돌 교정 후 final Official Build `5ff6d404dfa44c61a85e698e27052db5` PASS, exact `RpmGaugeVisualBindingContract` process `7d32d08b7e554e9991fb64e80d472d78` 1/1 PASS / Failure 0, Result SHA-256 `0054abd391216065f732e274b00bc054478c5ab168608523504b614c860fc6d4`다.
- Stage A/B와 UI-P0-03~05 USER PASS는 반복하지 않았다. UI-P0-06 전체 Done이나 USER Visual PASS로 확대하지 않으며 actual Redline authoring·RPM USER Visual과 WeaponGroup/Battery/Charge/Heat Runtime은 Pending이다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.11`을 사용한다. RPM Visual Binding은 관련 결함 없이 반복하지 않고 authoritative Redline 값이 생기기 전 0 Unconfigured를 유지한다. RPM-specific 다음 Gate는 명시 Redline authoring 결정 → USER Visual이다.

### v0.26.14 - 2026-08-18

- UI-P0-06 RPM Gauge explicit Redline upstream contract를 Technical PASS로 닫았다. `EngineMaxRPM`은 Chaos 물리 최대, `RedlineStartRPM`은 별도 authored presentation source이며 0은 미설정이다. `ChangeUpRPM`은 Current CarFight Source에 없다.
- Current RPM=Chaos Runtime / Redline+Maximum=VehicleData source로 분리했고 Presenter는 Redline→0.85 / EngineMaxRPM→1.0 piecewise mapping을 사용한다. 자동 `EngineMaxRPM*0.85` Redline 생성은 금지한다.
- Official Build `745dba430bcc47c2925c841a0c5a6686` PASS, 신규 RPM exact Automation `f805050484af47b8a2653cbb8ae2f639` 및 `af1d9d010c2c433382819698a6dc7eeb` 각각 1/1 PASS다. DAUTH current 118 Registry / Performance numeric 18 / Profile numeric 79 bounded compatibility도 3건 각 1/1 PASS했다.
- Production SpeedGauge visual binding, 대표 VehicleData 실제 Redline authoring과 USER Visual은 Pending이다. Stage A 6/6, Stage B, UI-P0-03~05 USER PASS는 반복하지 않았다.

Migration: 상세 기준은 `InGameUIPlan.md v0.59.10`을 사용한다. 다음 RPM 단계는 실제 Redline을 임의 생성하지 않고 대표 VehicleData authoring → Production visual binding → USER Visual 순서로 진행한다.

### v0.26.13 - 2026-08-18

- UI-P0-06 Dynamic Resource Visual Stage B를 Technical PASS로 닫았다. 최종 Official Build `0bc2f086d71b4a57933326757d88b50d`, WeaponPanel-only Apply `b600bd339b0348b5a3001c01a541484a`, fresh AssetDump dataset `adset_v1_9ca90ab2e764ee02cda305b45e91bb3b.23e84f05ff5481cd492c956f`, 신규 `ResourceVisualSlotContract` `2dc5eb2a394b4053875f548cec2204bc` 1/1 PASS를 확보했다.
- 저장 WeaponPanel은 Header Reserve + Primary 1 + Secondary 최대 2 + FireState 1 의미 구조로 전환됐고 구형 Launcher/Ammo/Heat/Cooldown 전용 Row는 제거됐다. raw `ResourceChannels` 직접 0~N 렌더링 금지와 LauncherSequenceRevision 단일 소비 계약을 유지한다.
- 첫 Stage B Automation FAIL은 `Visible` vs Presenter 공통 `HitTestInvisible` 테스트 expectation 불일치 5건뿐이었으며 Production 구현을 바꾸지 않고 테스트만 v1.14.0으로 교정했다.
- Stage A focused 6/6과 UI-P0-03~05 USER PASS는 반복하지 않았다. RPM Redline, Player-facing WeaponGroup, VehicleBattery, WeaponCharge, Heat Runtime은 계속 Pending이며 UI-P0-06 전체 PASS나 새 USER PASS로 확대하지 않는다.

Migration: `InGameUIPlan.md v0.59.9`를 현재 상세 기준으로 사용한다. Stage B는 관련 결함 없이 반복하지 않고 다음 UI-P0-06 slice는 남은 upstream 계약 중 실제 착수 가능한 항목에서 선택한다.

### v0.26.12 - 2026-08-18

- UI-P0-06 Dynamic Resource Visual Stage B Presenter·Production Bridge·Validator·`ResourceVisualSlotContract` Source를 적용했다. Presenter는 ViewData 적용당 `BuildWeaponResourceEntries()`를 정확히 1회만 소비하고 ReserveAmmo Header owner를 유지한다.
- Production WeaponPanel은 Primary 1 + Secondary 최대 2 + FireState 1 의미 슬롯으로 이전하도록 Bridge를 변경했고 Validator가 구형 Launcher/Ammo/Heat/Cooldown 전용 Row를 금지한다. Battery/Charge/Heat 가짜 Runtime/Row는 만들지 않았다.
- Build `4f7a093c107d42a1a94672a0b92b2bb4`에서 Stage B 변경 TU와 신규 Test compile은 PASS했다. 최종 DLL Link는 ownership-unknown Editor가 `UnrealEditor-CarFight_Re.dll`을 점유해 `LNK1104`, Exit 6이며 Production Asset mutation은 0이다.
- 전체 10 Asset Production Apply 대신 existing `WBP_CFWeaponPanel` 정확히 1개만 Build→Compile→Validate→Save하는 WeaponPanel-only targeted apply mode를 준비했다.
- 다음 순서는 Editor 안전 종료 → final Official Build → WeaponPanel-only Apply → fresh AssetDump persisted readback → Stage B Automation이다. Stage B Technical PASS나 USER PASS로 확대하지 않는다.

Migration: `InGameUIPlan.md v0.59.8`을 현재 상세 기준으로 사용한다. Stage A focused 6/6과 UI-P0-03~05 USER PASS는 관련 결함이 없는 한 반복하지 않는다. DAUTH Paused current는 `DataAuthoringPlan v0.2.26 / Roadmap v0.1.34`를 보존한다.

### v0.26.11 - 2026-08-18

- UI-P0-06 Dynamic Resource Visual Stage A Presenter Projection을 Technical PASS로 닫았다. raw `ResourceChannels` 직접 0~N 렌더링 금지와 `Primary 최대 1 + Secondary 최대 2 + FireState 최대 1` Compact 계약을 C++ Presentation Entry로 고정했다.
- 기존 Ammo/Reserve/Weapon Status/Launcher Resolver 및 `LauncherSequenceRevision` lifecycle을 재사용하고 ReserveAmmo Header owner, Battery/Charge/Heat Runtime 부재를 보존했다. Production Asset 변경은 0이다.
- Official Build `4c043a73e3054baab5f725b18cd7de6c` PASS, focused `CarFight.UI.UI_P0_06` process `73189888eb8042628db1b9499691c17c` 6/6 PASS / Failure 0, Result SHA-256 `39be662e7125ef4972574a75b433119283b7a279fbab56e511b140aafd6c70fe`를 확보했다.
- 다음 UI-only 단계는 Stage B Production Visual Container 적용이다. UI-P0-06 전체 Done, 새 USER Visual PASS, upstream Redline/WeaponGroup/Battery/Charge/Heat 구현으로 확대하지 않는다.

Migration: `InGameUIPlan.md v0.59.7`을 현재 상세 기준으로 사용한다. Stage B는 Presentation Projection을 ViewData 적용당 한 번만 소비하고 기존 고정 Row와 동시 중복 렌더링하지 않는다. Stage A Build/6-test와 UI-P0-03~05 USER PASS는 관련 결함이 없는 한 반복하지 않는다.

### v0.26.3 - 2026-08-18

- UI-P0-05 TargetSelect Marker의 단일 표시, 내부 의미 Text 0, Offscreen 선택 유지·재진입 복구, Clear/Reselect 회귀를 사용자 PIE로 PASS해 M2 UI-P0-03~05를 USER PASS로 닫았다.
- 다음 formal Runtime Gate를 `UI-DESIGN-GATE 현재 비차단 상태 → UI-P0-06 Vehicle·Weapon HUD`로 이동했다. D1-11 Production Structure PASS를 보존하고 D1-11-ART/D1-12는 Non-Blocking Deferred다.
- UI-P0-07 Target Knowledge와 UI-P0-08 Radar는 UI-P0-06 뒤에 유지하며 `SensorContact.md v1.1.0`을 read-only Sensor source로 사용한다.
- DAUTH projection은 최신 `DataAuthoringPlan v0.2.20 / Roadmap v0.1.28 / UA-01 USER PASS / USER PASS 1 / 다음 UA-02` Paused 상태로 교정했다.

Migration: 새 세션은 `InGameUIPlan.md v0.58.9`에서 UI-P0-06 착수 감사를 진행한다. UI-P0-02~05 완료 검증은 관련 결함이 없는 한 반복하지 않는다.

### v0.26.2 - 2026-08-18

- UI-P0-04 AimReticle UISubsystem 통합을 Build·focused Automation·USER Visual PASS로 닫았다.
- UI-P0-05 TargetSelect Marker는 Source·Official Build·focused Automation PASS까지 완료했고 USER Visual만 Pending이다.
- 현재 M2 Gate를 UI-P0-05 USER Visual로 이동했으며 PASS 전 TargetPanel/Radar로 진행하지 않는다.

Migration: `InGameUIPlan.md v0.58.8`을 현재 상세 체크포인트로 사용한다. UI-P0-03/04 완료 증거는 관련 결함이 없는 한 반복하지 않는다.

### v0.26.1 - 2026-08-18

- `M_VehicleDefensePIE`에서 Defense Production Panel의 Shield·방향 Armor·Integrity 실제 피해와 Shield 재생을 사용자 화면으로 확인하고 UI-P0-03 Defense USER Visual을 PASS했다.
- Baseline Pawn 전환에서 Shield 행 제거·Integrity `100/100` 갱신을 확인하고, Old Defense SUV 추가 피해·파괴가 Current Pawn/HUD에 반영되지 않음을 확인해 Pawn Rebind USER Visual과 Old Pawn 이벤트 격리를 PASS했다. UI-P0-03을 USER PASS로 닫았다.
- UI-P0-04 AimReticle UISubsystem 통합 Source를 적용했다. `UCFUISubsystem` HUD Layer 단일 소유 + Current Pawn Rebind + Cleanup, `CFVehiclePawn` direct CreateWidget/AddToViewport 자동 경로 제거, 기존 `WBP_AimReticle` Config 재사용이 현재 구현이다.
- UI-P0-04 Official Build·focused Automation·USER Visual이 남아 있으므로 M2 완료나 UI-P0-05 착수로 확대하지 않는다.
- DAUTH는 `DataAuthoringPlan v0.2.17 / DataAuthoringRoadmap v0.1.25 / P0-12 UA-01 / USER PASS 0` Paused 상태를 유지한다.

Migration: 새 세션은 `InGameUIPlan.md v0.58.7`과 이 Roadmap v0.26.1을 기준으로 UI-P0-04 Build → Automation → USER Visual을 진행한다. UI-P0-03은 관련 결함이 없는 한 반복하지 않는다.

### v0.26.0 - 2026-08-18

- 사용자 우선순위 변경을 반영해 `CF-FQ-032`를 현재 단일 Active UI 작업으로 다시 고정하고 최신 Resume Checkpoint를 문서 상단에 추가했다.
- UI-P0-02 Runtime Lifetime USER PASS, D1-11 Production Structure PASS와 UI-P0-03의 Speed/Weapon/Target/Ammo·Salvo USER PASS를 보존하며 현재 남은 Gate를 `Defense Production Panel USER Visual → Pawn Rebind USER Visual`로 동기화했다.
- 두 USER Gate 완료 뒤 `UI-P0-04 AimReticle UISubsystem 통합 → UI-P0-05 TargetSelect Marker 통합` 순서를 유지한다.
- Scanner/Sensor Current `SensorContact.md v1.1.0`을 후속 TargetPanel/Radar source로 연결하고 DAUTH `P0-12 UA-01`은 Paused로 보존했다.
- 아래 과거 milestone의 당시 상태는 history로 유지하며 현재 착수 판단은 상단 Resume Checkpoint와 `InGameUIPlan.md v0.58.6`을 우선한다.

Migration: 새 세션은 UI-P0-02나 완료된 Launcher USER Gate를 반복하지 않고 Defense/Pawn Rebind USER Visual부터 재개한다.

### v0.25.0 - 2026-08-10


- D1-10A C++ Visual Context/Button Bridge 공식 Build와 두 Contract Automation이 PASS했다.
- D1-10B 정확한 Designer Tree를 위해 최소 Editor Bridge Source를 준비했으나 열린 CarFight Editor의 DLL 점유로 공식 Link가 LNK1104에 차단됐다.
- Base Widget 5종 Asset은 생성하지 않았고 D1-10 전체는 NOT PASS로 유지했다.
- D1-11은 Prepared/Not Started이며 D1-10 PASS 전 착수 금지를 유지한다.
- commit·push는 수행하지 않았다.

### v0.24.0 - 2026-08-10

- D1-09B Font·Icon·Style/Density/1080 Layout Unreal Asset과 Font Notice·Config 연결을 실제 생성했다.
- D1-09B Readback `d09097cc7781470ba9fd4261828a2c14`에서 Asset·License·Config Contract와 `d1_09b_pass=true`를 확인했다.
- 파일 잠금 해제 후 최종 전체 회귀 `e73311a38be043d48d512ab90afd258f`가 51/51·필수 24/24 Success로 통과했다.
- D1-09B를 PASS로 닫고 D1-10을 Not Started / Next로 승격했다.
- UI-DESIGN-GATE 전체 PASS는 D1-10~12 완료 전 금지하며 1440p·21:9·32:9는 계속 Deferred한다.
- commit·push는 수행하지 않았다.

### v0.23.0 - 2026-08-10

- D1-08V를 공식 Editor Build와 `StyleDataContract` Automation PASS로 닫았다.
- D1-09A Font Binding·HUD Layout·Density C++ 및 Subsystem Config Soft Reference·Native Fallback 구현을 완료했다.
- D1-09A 최종 Build Job `9d9aefb3803e4556bd4559afded0f4ed`와 Automation Process `18bea9b5952e4444b618e776cb2e2571`에서 전체 51/51·필수 24/24 Success를 확인했다.
- D1-09A 네 Contract는 모두 Warning 0 / Error 0으로 PASS했다.
- 다음 단계는 D1-09B Unreal Asset·Config이며 D1-10으로 건너뛰지 않는다.
- commit·push는 수행하지 않았다.

### v0.22.0 - 2026-08-07

- `InGameUIAssetizationSpec.md v0.1.0`을 D1-09A~12 exact implementation SSOT로 추가했다.
- Font License Eligibility를 Pretendard·IBM Plex Mono OFL 1.1 PASS로 확정하고 실제 Asset 생성 시 원본 Copyright Notice·OFL 전문 보존을 D1-09B 조건으로 고정했다.
- D1-09A Font Binding은 `UFont` Composite Hard Reference, Subsystem의 Style/Density/Layout DataAsset은 Config Soft Reference 1회 해석 구조로 잠갔다.
- D1-10 C++ Bridge + 5 Base Widget, D1-11 1080p Visual Prototype, D1-12 Pause Native Fallback Visual Migration의 순서와 중단 조건을 고정했다.
- Source·Config·Unreal Asset·Build·Automation·PIE·commit·push는 수행하지 않았다.

### v0.21.0 - 2026-08-07

- 에디터를 종료할 수 없는 현재 조건에서 D1-09~12는 구현하지 않고 exact implementation preparation만 완료했다.
- D1-09A를 Font Asset Binding·HUD Layout·Density C++ 타입과 Subsystem Config Soft Reference 보완으로 고정했다.
- D1-09B Active Asset을 1080p Style·Density·Layout과 실제 Font·P0 Semantic Icon으로 제한하고 후속 해상도 Profile은 Deferred했다.
- D1-10 Base Widget 5종 → D1-11 기능 없는 HUD Visual Prototype → D1-12 기존 Pause Runtime 보존 Visual Migration 순서를 잠갔다.
- 다음 실행 진입점을 D1-08V 공식 Build·Automation PASS로 고정하고 단계별 실패 시 다음 단계로 건너뛰지 않도록 했다.
- Source·Config·Unreal Asset·Build·Automation 추가 실행과 commit·push는 수행하지 않았다.

### v0.20.0 - 2026-08-07

- D1-08 C++ Style Data Type을 신규 소스 3파일로 구현해 UI-DESIGN-GATE 자산화 단계에 진입했다.
- 기존 UI 런타임·Unreal Asset을 변경하지 않고 `UCFUIStyleData` Native Fallback과 `CarFight.UI.D1_08.StyleDataContract` 검증 소스만 추가했다.
- D1-08 상태는 Code Applied / Static Readback PASS / Build·Automation Pending이며 검증 완료 전 D1-09로 승격하지 않는다.
- D1-09A~12 exact preparation은 완료했지만 실제 Source·Config·Unreal Asset 구현과 Font 라이선스 확인은 계속 Pending이다.
- commit·push는 수행하지 않았다.

### v0.19.0 - 2026-08-07

- 사용자가 외곽 재배치된 `SR-1080-16` 수정본을 승인해 D1-07 Phase 1을 USER PASS로 닫았다.
- D1 현재 상태를 D1-05·06 Accepted / D1-07 USER PASS / D1-08~12 Assetization Pending으로 이동했다.
- SR-A·C·D·E와 1440p·21:9·32:9 Layout Profile 검토는 현재 Gate 비차단 후속 검토로 분리했다.
- UI-DESIGN-GATE 전체 PASS는 C++ Style Data, `DA_CFUIStyle`·Density/Layout Asset, Base Widget, Visual Prototype, Pause Visual Migration과 실제 Font·Icon 자산/라이선스 확인 전까지 금지한다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.18.0 - 2026-08-07

- D1-07 사용자 피드백에 따라 `SR-1080-16` 고정 HUD의 1080p Phase 1 외곽 배치 Override를 현재 디자인 체크포인트에 반영했다.
- 패널 크기 유지, 실효 약 24px 외곽 Margin, 중앙 전투 시야 우선 원칙을 `InGameUIStyleSpec.md v0.13.0`과 `InGameUIStaticReview.md v0.5.0`으로 연결했다.
- 재배치된 1080p Mockup의 사용자 PASS/FAIL 판정은 Pending으로 유지했다.
- 1440p·21:9·32:9 Deferred Expansion과 Source·Unreal Asset 미착수 상태는 변경하지 않았다.
- Build·Automation·commit·push는 수행하지 않았다.

### v0.17.0 - 2026-08-07

- D1-07 Phase 1 사용자 Gate를 `SR-1080-16` 1920×1080 / 16:9 하나로 동기화했다.
- `SR-1440-16`, `SR-1440-21`, `SR-1440-32`는 준비 상태를 보존하되 후속 Layout Profile 확장으로 Deferred했다.
- D1 종료 조건도 1080p Phase 1 PASS를 현재 Gate로 사용하고 후속 해상도 확장이 이를 차단하지 않도록 정리했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.16.0 - 2026-08-07

- D1-07용 SR-1080-16·SR-1440-16·SR-1440-21·SR-1440-32 Static Mockup을 준비했다.
- `InGameUIStaticReview.md v0.1.0`을 추가해 해상도별 PASS/FAIL 체크리스트, 필수 상태 세트와 전역 FAIL 조건을 기록했다.
- 네 대표 Mockup은 동일 Combat Busy Mock View Data를 사용해 화면비에 따른 HUD 배치와 시선 이동 차이만 비교하도록 했다.
- 1080p Typography 유효 하한과 21:9·32:9 중앙 2560×1440 Persistent HUD Canvas 계약을 반영했다.
- 현재 상태는 Mockups Ready / User Review Pending이며 Runtime Capture와 UI-DESIGN-GATE PASS는 수행하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.15.0 - 2026-08-07

- 사용자가 D1-06 Font·Icon 추천안을 전부 승인했다.
- 기본 Font Family를 Pretendard + IBM Plex Mono, Icon Style을 Solid Core + Tactical Cut로 확정했다.
- 방어·무기·Target·Lock·Radar Semantic Icon 기본 형태와 24×24 Base Grid, 16/20/28 및 Weapon 18/20 크기 계약을 승인 상태로 기록했다.
- Font와 Icon은 실제 Asset 경로가 아니라 Style/Icon Data의 Family Role·Semantic ID로 교체 가능하게 유지한다.
- 다음 디자인 체크포인트를 D1-07 SR-1080-16·SR-1440-16·SR-1440-21·SR-1440-32 실제 Static Review로 이동했다.
- 실제 Font·Icon Unreal Asset 생성, 라이선스 최종 확인, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.14.0 - 2026-08-07

- 사용자가 D1-05 Style Token·Base Widget·Density Profile·WeaponPanel Compact Token 추천안을 전부 승인했다.
- `InGameUIStyleSpec.md v0.9.0`, `InGameUIWeaponPanelSpec.md v0.5.0`을 D1-05 User Accepted SSOT로 갱신했다.
- 승인값은 수정 불가능한 Pixel 계약이 아니라 Style·Layout·Density Data Asset과 명시적 Override로 바꿀 수 있는 기본 Preset으로 유지한다.
- `ValueS`, Compact Panel 12/36, Density별 StatusBar·InfoRow와 WeaponPanel Tile Icon 18·Role 기반 Primary Value 정합화를 반영했다.
- 다음 디자인 체크포인트를 D1-06 Font·Icon Review → D1-07 실제 Static Review로 이동했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.13.0 - 2026-08-07

- D1-05를 Style Token·Base Widget·Density Profile·WeaponPanel Compact Token Detailed Draft / User Review Pending으로 갱신했다.
- `InGameUIStyleSpec.md v0.8.0`과 `InGameUIWeaponPanelSpec.md v0.4.0`을 현재 D1 상세 검토 문서로 연결했다.
- Compact·Standard·Expanded Density 실제 수치, 1080p Typography 하한과 Base Widget `Collapsed` 규칙을 검토 범위에 추가했다.
- WeaponPanel의 360 Preferred Width, Selected 142~178, Full 254, Tile 112×68 초안을 기록했다.
- 최소 Static Review 행렬을 `SR-1080-16 / SR-1440-16 / SR-1440-21 / SR-1440-32`로 고정하고 실제 캡처는 Not Run으로 유지했다.
- 다음 디자인 단계를 D1-05 사용자 Review → Font·Icon Review → 실제 Static Review로 정리했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.12.0 - 2026-08-07

- `D1-VEHICLE-PANEL`의 560×416 상세 규격과 패널 Wireframe을 작성했다.
- Speed·D/R/N·Drive State·Handbrake, Shield, 6방향 Armor, Integrity와 부품 손상 확장 규칙을 정의했다.
- 실제 Gear 번호와 부품 손상 Runtime이 없는 현재 구현 경계를 반영했다.
- D1-VEHICLE-PANEL을 Draft / User Review Pending으로 갱신하고 D1-WEAPON-PANEL은 Pending으로 유지했다.
- UI-P0-02·UI-P0-03 코드 우선순위와 CommonUI Gate는 변경하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.11.0 - 2026-08-07

- 외부 3인칭 차량 TPS와 16:9 HUD 기본 배치를 D1 승인 체크포인트로 기록했다.
- Mission TL, Alert TC, Target TR, Vehicle BL, Radar BC, Weapon BR 배치를 확정했다.
- 내 차량 정보와 무기창의 내부 상세를 각각 `D1-VEHICLE-PANEL`, `D1-WEAPON-PANEL` 후속 설계로 분리했다.
- Radar를 하단 중앙으로 이동하고 속도는 Vehicle Panel 영역에 포함하도록 이전 Wireframe을 대체했다.
- 후속 사용자 HUD 배치 커스터마이징을 위한 Slot·Layout Profile 확장 경계를 추가했지만 현재 기능 구현 범위에는 포함하지 않았다.
- CommonUI 판단은 다루지 않았으며 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.10.0 - 2026-08-06

- D1에 `InGameUIStyleSpec.md v0.1.0` 상세 제작 규격 초안을 추가했다.
- Color·Typography·Spacing·Shape·Opacity·Motion Token과 Base Widget 5종의 계약을 작성했다.
- `CFHUDWireframe_1440p.xml`에 2560×1440 HUD 좌표·Anchor와 Reticle Protection 영역을 작성했다.
- D1 상태를 Style Spec·1440p Wireframe Draft / User Review Pending으로 갱신했다.
- Unreal Style Asset·Base Widget·Font·Icon과 해상도별 실제 화면 검토는 Pending으로 유지했다.
- CommonUI 판단은 다루지 않았으며 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.9.0 - 2026-08-06

- 디자인 무게 C형 70:20:10 균형을 사용자 승인 상태로 반영했다.
- D1 시각 방향을 Visual Direction Accepted로 승격했다.
- Style Data·Base Widget·Font·Icon·Wireframe과 해상도별 검토는 Production Artifacts Pending으로 유지했다.
- 상위 Combat UI SSOT와 DecisionLog에 승인된 방향만 동기화하고 D1 전체 PASS는 보류했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.8.0 - 2026-08-06

- UI 시각 콘셉트 검토 1~3번의 사용자 승인을 반영했다.
- 차량 탑재형 전술 인터페이스, 스타일라이즈드 평면 UI와 Cyan·Amber·Red Palette 방향을 승인된 체크포인트로 기록했다.
- 디자인 무게 A·B·C는 미결정으로 유지해 D1을 Partial Approval 상태로 두었다.
- Style Asset·Wireframe·Gate PASS와 상위 SSOT Accepted 승격은 수행하지 않았다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.7.0 - 2026-08-06

- `InGameUIVisualConcept.md v0.1.0` Working Draft를 Roadmap에 연결했다.
- `차량 탑재형 전술 인터페이스` 콘셉트 후보와 근미래 차량·전술·스타일라이즈드 렌더링 호환 방향을 기록했다.
- 형태·재질·색상 Token·Typography·Spacing·Icon·Motion과 HUD 요소별 원칙이 초안으로 준비됐음을 반영했다.
- D1 상태를 Concept Draft / User Review Pending으로 변경하고 Style Asset·Wireframe·Gate PASS는 미완료로 유지했다.
- 사용자 승인 전 상위 Combat UI SSOT와 DecisionLog에 Accepted 결정으로 올리지 않도록 체크포인트를 명시했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.6.0 - 2026-08-06

- UI 디자인 품질 개선과 CommonUI 도입 판단을 별도 Gate로 분리했다.
- `UI-DESIGN-GATE`를 UI-P0-03~05 뒤, UI-P0-06 신규 HUD 시각 구현 전의 필수 단계로 추가했다.
- Style Data, Base Widget, 상호작용 상태, 해상도별 Clean Prototype 산출물과 종료 조건을 정의했다.
- `UI-COMMONUI-GATE`를 첫 전체 화면 Flow 구현 전의 별도 판단 단계로 추가하고 현재 인게임 HUD P0를 차단하지 않도록 했다.
- UMG 유지·HUD UMG/Menu CommonUI 혼합·전체 화면 CommonUI 확장의 허용 결론과 Migration 불변 조건을 기록했다.
- 디자인 개선을 CommonUI 도입까지 미루지 않고, 외형 개선만을 CommonUI 도입 근거로 사용하지 않도록 순서를 고정했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.5.0 - 2026-08-01

- `UI-P0-02`를 Code Complete / 공식 Editor Build·Automation PASS / User PIE Pending으로 갱신했다.
- 실제 World Pause, 입력 잔류 제거, C++ Pause Menu와 Continue·Back 수명을 기록했다.
- Launcher Ripple·Salvo, Projectile·Motor와 World Timer 진행 정지 구조 회귀를 기록했다.
- 최종 Build `4f2daa07187a41aea7c400386010b2a0`와 Automation `6f0a9a5ebde5472b8b784a9103ce484d` 증거를 연결했다.
- 전체 CarFight 32/32, 필수 17/17과 피팅 2건 Success를 기록했다.
- 다음 코드 체크포인트를 `UI-P0-03 HUD 데이터 계약`으로 이동했지만 착수하지 않았다.
- 실제 Pause 체감과 재개 동작은 사용자 PIE Pending으로 유지했다.

### v0.4.0 - 2026-08-01

- `UI-P0-01A`의 `ACFPlayerController`, Controller 소유 Mapping Context, Pause·Back 요청, Possession 통지와 Input Mode 기반을 구현했다.
- `UI-P0-01B`의 `UCFUISubsystem`, World별 C++ Root와 8개 CanvasPanel 레이어를 구현했다.
- Screen·Modal 전환과 Cursor·Focus 기반을 추가했다.
- 기존 Pawn 입력과 AimReticle·TargetSelect HUD를 그대로 보존했다.
- 공식 Editor Build `cd935342a3714887af98b26ed0d95ab6`이 PASS했다.
- Combat Runtime Automation `91bc6df11f4d41968a39e5fdd44b79df`에서 29/29 Success, Failed 0을 확인했다.
- `UI-P0-02` 완전 Pause, Unreal UI Asset과 사용자 PIE는 시작하지 않고 다음 단계로 남겼다.
- 피팅 `CF-FQ-034` 구현과 문서·에셋은 수정하지 않았다.

### v0.3.0 - 2026-07-31

- UI-P0-01B에 Screen Layer와 전체 화면 전환 진입점을 추가했다.
- 피팅 구현을 CF-FQ-032 로드맵에서 제외하고 별도 Plan 소유 범위를 기록했다.
- 공통 UI 입력·Style·표현 타입만 공유하고 Loadout, 계산, 저장, Preview와 Snapshot은 분리하도록 확정했다.
- 피팅 기능 ID와 Plan은 실제 기획 착수 시 생성하도록 명시했다.

### v0.2.1 - 2026-07-31

- 차량 HUD와 무기 HUD를 단일 `UI-P0-06` 단계로 통합했다.
- 잘못 분리되어 있던 `UI-P0-06B` 표기를 제거했다.
- Vehicle Integrity와 Shield·Armor Mock의 구현 경계를 M3 종료 조건에 명시했다.

### v0.2.0 - 2026-07-31

- `UI-P0-00A` 코드 조사와 `UI-P0-00B` 구현 계약 확정을 Done으로 기록했다.
- 실제 착수점을 `UI-P0-01A ACFPlayerController·입력 기반`으로 변경했다.
- LocalPlayer Subsystem과 World별 Root 수명을 `UI-P0-01B`로 분리했다.
- AimReticle과 TargetSelect HUD를 별도 단계로 이전하도록 M2를 개정했다.
- Target Knowledge, 화면 밖 방향, Alert 중복 제거와 공통 스타일을 마일스톤에 포함했다.

### v0.1.0 - 2026-07-30

- CF-FQ-032의 M0~M6 구현 순서를 정의했다.
- 완전 Pause와 기존 Reticle 회귀를 초기 마일스톤으로 배치했다.
- 아직 없는 Shield, Sensor, Knowledge와 Ammo Runtime을 Mock 또는 외부 의존으로 명시했다.
- 사용자 PIE와 Systems 승격 종료 조건을 추가했다.
