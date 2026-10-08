# CarFight InGame UI Plan

- 문서 버전: v0.59.36
- 작성일: 2026-07-30
- 최근 갱신일: 2026-08-22
- 문서 상태: Historical / Retained Path / CF-FQ-032 Done / Post-Closure Code Review Remediation Technical PASS / Current owner `../Systems/UI/InGameUI.md v1.1.0` / USER Visual·Zoom Feel Deferred Pending 보존

- 기능 ID: `CF-FQ-032`
- 대표 범위: 인게임 전투 HUD, UI 프레임워크와 싱글플레이 완전 일시정지
- 상위 기획: `../ProjectSSOT/CombatPlan/14_CombatUI.md`
- 상세 설계: `InGameUIDesign.md`
- 시각 콘셉트: `InGameUIVisualConcept.md`
- 제작 규격 초안: `InGameUIStyleSpec.md`
- 자산화 실행 규격: `InGameUIAssetizationSpec.md v0.16.0`
- HUD 전용 아트 명세: `InGameUIHUDArtSpec.md v0.3.8`
- 1440p Wireframe: `ConceptArt/CFHUDWireframe_1440p.xml`
- Vehicle Panel 상세: `InGameUIVehiclePanelSpec.md`
- Vehicle Panel Wireframe: `ConceptArt/CFVehiclePanel_1440p.xml`
- Weapon Panel 상세: `InGameUIWeaponPanelSpec.md`
- Weapon Panel Wireframe: `ConceptArt/CFWeaponPanel_1440p.xml`
- 로드맵: `InGameUIRoadmap.md`

---

## 1. 목적

현재 CarFight의 검증된 조준 레티클과 전투 피드백을 보존하면서, 차량 Pawn에 직접 종속되지 않는 인게임 UI 기반을 만든다.

현재는 인게임 UI만 구현한다. UI Root, 레이어와 화면 수명은 향후 타이틀, 차고, 피팅, 임무 선택, 로딩과 결과 화면을 추가할 수 있도록 설계하되, 실제 피팅 도메인과 피팅 화면은 별도 기능과 별도 Plan으로 기획한다.

---

## 2. 현재 상태

```text
Feature: CF-FQ-032
Status: Done / Historical / Retained Path
Documentation: Current Knowledge Promoted to Systems
UI-P0-01A: Code Complete / Build·Automation PASS / User PIE Partial PASS
UI-P0-01B: Code Complete / Build·Automation PASS / User PIE Partial PASS
UI-P0-02: Code Complete / Build·Automation PASS / Runtime Lifetime USER PASS
UI-DESIGN-GATE: D1-05·D1-06 User Accepted / D1-07~10 PASS / D1-11 Production Structure Gate PASS / Root 1 + Panel 6 + Element 2 + HUD Visual Data 1 / Fifth Preview Structure Sanity Accepted / `CFUIHUDProdEditorBridge v1.2.0` Empty RPM Track Fix·Validator PASS / Official Build `cef0c064cd434848b1fcb94459fbccb5` PASS / Production Apply `f1c32cab41b544d6b27709315029bb04` PASS / Final Saved Readback `c0bdf6a72df3412da78057406e843c0c` PASS / Fresh AssetDump 9/9 PASS / D1-11-ART-01 Source PNG 9/9 Generated·Technical PASS·User Visual Review Ready/Pending / Approved Asset 0·Import 0·DA Connection 0 / D1-12 Not Started / Font License Notice Preserved / 1440p·21:9·32:9 Deferred Expansion
UI-P0-03: USER PASS / ViewData·Provider·Presenter·Production HUD 연결 / Speed·Weapon Cooldown·Target 선택/해제 USER PASS / Ripple·Ammo 실제 WeaponPanel 경로 USER PASS / `TestMap_AmmoSalvo` Launcher USER PIE 5/5 PASS / `CFHUDDataTests v1.6.0` Defense Runtime·PIE ViewData + Pawn Rebind Old Pawn 구독 해제 / Final Official Build `681c91810da34066bb398ad1b0989f1e` PASS / targeted `CarFight.UI.UI_P0_03` Automation `f6b08a970bbf4ec282893e86e92e72ac` 5/5 Success·0 Fail / `M_VehicleDefensePIE` Defense Production Panel Shield 0·Armor 손상·Integrity 40 및 Shield 100 재생 USER Visual PASS / Baseline Pawn Rebind Integrity 100·Shield 숨김 + Old Defense SUV 추가 피해·파괴 중 Current Pawn HUD 무변화 USER Visual PASS
UI-P0-04: USER PASS / `UCFUISubsystem v1.6.0` 기존 `WBP_AimReticle` HUD Layer 단일 소유 + Current Pawn Rebind·Cleanup / `CFVehiclePawn v2.148.0` direct CreateWidget·AddToViewport 제거 / `UCFAimReticleWidget v1.9.0` Weak Pawn Binding / 최종 Build `ace404b0eb7f4c97b010e96ce85f41ce` PASS / focused Automation `74f4bdfd283243e49369ae25afdce2f5` 1/1 PASS / USER Visual Reticle 1세트·Defense→Baseline Rebind·중복/Old Pawn 잔류 없음 PASS
UI-P0-05: USER PASS / `UCFUISubsystem v1.7.0` 기존 `WBP_TargetSelect` Game Layer 단일 소유 + Current Pawn Rebind·Cleanup / `CFVehiclePawn v2.149.0` direct TargetSelect CreateWidget·AddToViewport 제거 / `UCFTargetSelectWidget v1.1.0` Weak Pawn Binding·Marker-only·내부 Actor Name/의미 Text 제거·Event semantic cache + Projection-only Tick / `DefaultTargetSelectWidgetClass` Config 연결 / Build `242dfee7186d46aaa98e3c24b25c0d09` PASS / `CarFight.UI.UI_P0_05.TargetMarkerLayerContract` `3834e606f2b54efb9ac5c09ea410b4f4` 1/1 PASS / USER Visual에서 단일 Marker·내부 이름/중복 Text 0·Offscreen 숨김/선택 유지·재진입 단일 복구·Clear/Reselect PASS
Closure: UI-P0-11 Systems Promotion Complete + Post-Closure Code Review Remediation Technical PASS / `../Systems/UI/InGameUI.md v1.1.0` Current owner / `../Systems/UI/AimReticle.md v1.10.0` Aim·FireFeedback detail owner / `../Systems/Targeting/SensorContact.md v1.2.0` Target/Sensor Identity owner / D1-11-ART와 UI-P0-08 Radar·Edge Visual·Zoom Feel은 비차단 Deferred/Pending·USER PASS 미부여 보존
Source Changes: Applied — PlayerController, LocalPlayer UI Subsystem, C++ UI Root, AddToViewport(100) 계층, Continue Mouse·Enter·Escape 해제, Foundation Automation, GameMode 연결, Vehicle Core·Combat Runtime Ready 분리와 입력 Context 소유권 명시
Asset Changes: Applied — D1-09B FontFace 8·Runtime Font 2·Semantic Icon 18·Style/Density/1080 Layout DataAsset 5·Font Notice 2·CFUISubsystem Config Soft Reference 3 + D1-10B `/Game/CarFight/UI/Base/` Base Widget Blueprint 5종 + D1-11 Production HUD Root 1·Panel 6·Element 2·`DA_CFHUDVisual_Default` 1
Current Active Feature: None — next feature selection is owned by main_game FeatureQueue/User decision
Previous Completed Features: CF-FQ-031 Done / AMMO-P0-00~08 Done / Heavy·Ripple USER PIE PASS / Ammo Systems Current + CF-FQ-033 Done / DR-P0-00~07 Done / DR-PIE-00~06 USER PASS / VehicleDefense·HitDamage Systems Current
Protected Checkpoints: CF-FQ-029 LM-P0-06·CF-FQ-026 TS-P0-08·CF-FQ-034 FIT-P0-07D·CF-FQ-035 USER Field UI/Mobility·CF-FQ-038 DAUTH-P0-12 USER PASS 6 / 다음 UA-07 Driving Feel Authoring / CF-FQ-030 Missile Manual PIE Ready / CF-FQ-037 Scanner Done

```

현재 구현된 관련 기반:

- `UCFVehicleCameraComp` 외부 3인칭 자유 조준과 Aim Trace
- `UCFVehicleAimComp` 조준 상태
- `UCFAimReticleWidget`과 `WBP_AimReticle`
- FireFeedback, 이중 레티클과 터렛 정렬 표시
- `UCFUISubsystem`의 LocalPlayer 공통 UI Root와 실제 싱글플레이 World Pause
- `UCFVehicleDefenseComp`의 Shield·6방향 Armor·Vehicle Integrity Runtime과 Legacy Fallback
- `UCFTargetSelectComp`의 P0 코드와 HUD 기반, TS-P0-08 Paused
- `bVehicleCoreRuntimeReady`와 `bVehicleCombatRuntimeReady`의 분리된 준비 상태
- 차량 Gameplay 입력은 Pawn `DefaultInputMappingContext`가 소유하며 Controller `GameplayInputMappingContext`는 원자 이전 전 None 유지

UI-P0-03 현재 적용된 Runtime 연결:

- `FCFInGameUIViewData` 아래 Vehicle·Weapon·Defense·Target·Radar·Alert ViewData와 `Unknown / Unavailable / KnownZero / Known` 명시 상태
- `UCFHUDDataProvider`가 `UCFUISubsystem.OnCurrentPawnChanged`를 구독해 Old Pawn 이벤트·Timer를 해제하고 New Pawn Gameplay Runtime에 Rebind
- Vehicle Speed, Shield·6방향 Armor·Integrity, Weapon Cooldown·Launcher Sequence와 TargetSelect 공개 `FCFTargetDisplayInfo`를 실제 Runtime에서 읽기 전용 변환
- `UCFHUDPresenter`가 ViewData만 소비해 기존 Production `WBP_CFInGameHUD` 의미 Widget에 반영하며 Gameplay Pawn/Component Cast를 하지 않음
- Production HUD Class는 `DefaultInGameHUDWidgetClass` Config로 `UCFUISubsystem` HUD Layer에 생성되며 기존 `CFStyledWidgetBase` Parent와 D1-11 Asset 구조를 유지
- finite Ammo는 `UCFVehicleAmmoComp → FCFAmmoRuntimeSnapshot → UCFHUDDataProvider → FCFWeaponHUDData` 실제 Runtime을 사용해 `Loaded / MagazineCapacity + label-less Reserve`, Reload와 NoAmmo 상태를 제공한다. Engine RPM·Gear는 UI-P0-06에서 실제 UE 5.8 Chaos Runtime으로 연결됐다. Heat는 `UCFWeaponData explicit Heat inputs → FCFWeaponHeatRuntime/UCFVehicleWeaponComp → UCFHUDDataProvider → ResourceChannels::Heat`, WeaponCharge는 `UCFWeaponData explicit Maximum/Initial/PerShot/Recovery → FCFWeaponChargeRuntime/UCFVehicleWeaponComp → UCFHUDDataProvider → ResourceChannels::WeaponCharge` 실제 Runtime으로 연결됐다. Target Identity·Distance·AnalysisProgress는 `Sensor Snapshot → UCFHUDDataProvider → FCFTargetHUDData`로 실제 source가 존재한다. VehicleBattery·Target Armor/Module Intelligence는 실제 Provider가 없으면 추정하지 않고 `Unavailable`; Radar Contact는 Sensor Snapshot만 소비하며 가짜 Contact 0
- `UCFLauncherComp v1.3.0`의 `OnLauncherSequenceChanged`를 `UCFHUDDataProvider v1.5.0`이 Pawn Rebind 수명에 맞춰 구독·해제한다. Provider는 실제 Launcher 이벤트에서만 `LauncherSequenceRevision`을 증가시켜 Ammo 예약 해제 이벤트나 10Hz Timer Refresh를 의미 상태 전이와 분리한다. Completed/Cancelled terminal 이벤트는 SequenceCompleted Cooldown 기록과 Ammo 예약 정리 뒤 최종 Snapshot으로 전달된다.
- `UCFHUDPresenter v1.7.0`은 Salvo 전용 초 단위 Hold를 소유하지 않는다. Active Sequence를 관측한 뒤 같은 Revision의 부수 Refresh에서는 마지막 Active Presentation을 보존하고, 실제 terminal Revision에서 최종 `RIPPLE/SALVO N / Total` Snapshot을 한 ViewData 주기 표시한 다음 공통 Weapon Status의 Cooldown→READY로 전환한다. FirePattern은 RIPPLE/SALVO 문구 선택에만 사용한다.
- `CFUIHUDProdEditorBridge v1.3.0`은 `HorizontalBox_LauncherSequence`, `Text_WeaponLauncherSequence`, `ProgressBar_LauncherSequence`의 Production 의미 슬롯과 Validator를 소유한다.
- `CFHUDDataTests v1.6.0`은 기본 가용 상태와 Launcher Presentation 회귀에 더해 실제 `VehicleDefenseComp` 피해·Shield 재생 → Provider Defense ViewData, `OnCurrentPawnChanged` Rebind 뒤 Old Pawn Defense 이벤트 구독 해제, 현재 Pawn 이벤트 즉시 반영을 검증한다. `DefensePIEViewData`는 저장된 `/Game/Maps/M_VehicleDefensePIE`를 실제 PIE로 열고 정식 Defense SUV에 `Shield 100 → 0 / Front Armor Ratio 1.0 → 0.5 / Integrity Ratio 1.0 → 0.5` 피해를 적용해 Provider ViewData 전달을 검증한다.

아직 구현되지 않았거나 후속 Provider/Presentation이 필요한 관련 기능:

- 부품 손상 Runtime
- Sensor/Scanner Runtime은 `SensorContact.md v1.2.0` Current이며 Production Radar의 동적 Image Contact·Display Range·Player Marker·in-range Selected·range-out Selected Edge consumer, Mouse Wheel Radar Zoom Input과 화면 밖 선택 Target의 camera View Space 기반 Screen-edge Marker까지 Technical PASS다. USER Visual/Zoom 조작감 검증과 Active Scan Sweep Material은 후속이다.
- Sensor Analysis/Knowledge Runtime 자체는 Current지만 TargetPanel의 최종 Intelligence Presentation과 USER Visual은 미완료
- 장비 Lock-on Runtime

- UI-P0-08 Runtime 구현은 Technical Complete다. 선택 Target Edge와 Radar Zoom의 시각 품질·조작감 USER 판정은 Deferred/Pending follow-up으로 보존하며 USER PASS로 확대하지 않는다. `UI-P0-09A~09D`는 기존 VehicleCamera/Aim read-only ViewData, Production ViewMode consumer, Alert Style duration lifecycle, Nested Style Context까지 모두 Technical PASS해 UI-P0-09 Technical Complete다. `UI-P0-10 Integration Validation`도 4개 기준 해상도, Pause·Reticle·Target·Radar·Weapon cross-system Automation과 fresh AI-owned PIE RuntimeRead를 통과해 Technical Complete다. 이 PIE는 AI Runtime Technical Validation이며 USER Visual·Feel PASS가 아니다. `UI-P0-11 Systems Promotion`은 완료됐고 현재 구현 판단은 `../Systems/UI/InGameUI.md`를 우선한다. VehicleBattery는 별도 shared-power Gameplay dependency로 유지하고 WeaponCharge/Heat tuning·USER Visual은 content-dependent Deferred다.
- Player-facing Weapon Selection Runtime/HUD source, truthful WeaponRail Visual Consumer와 실제 Keyboard Weapon Select Input은 Technical PASS다. Applied Fitting weapon-bearing `ResolvedMounts` fixed order + `SelectedWeaponIndex`를 사용하며 새 WeaponGroupId는 만들지 않는다. `IA_SelectWeapon` Axis1D에서 숫자 1~9가 해당 1-based ordinal을 직접 전달하고 Pawn은 `Ordinal-1`만 기존 `RequestSelectWeaponIndex`에 위임한다. Mouse Wheel은 Radar Range/Zoom 예약을 보존하고 게임패드는 임의 mapping하지 않는다. Rail USER Visual은 실제 persisted multi-weapon content가 생길 때까지 Deferred다.
- `InGameUIVisualConcept.md v0.8.0`의 Visual Direction과 D1-06 Font·Icon 기본 Preset은 승인 완료
- `InGameUIStyleSpec.md v0.86.0`이 최신 Style/Reticle/Declutter 계약을, `InGameUIVehiclePanelSpec.md v0.20.0`이 최신 VehiclePanel Visual 계약을, `InGameUIAssetizationSpec.md v0.16.0`이 D1-09A~12 실행 상태와 Structure-first D1-11 PASS 기준·검증 증거를 소유한다. D1-08V·D1-09A·D1-09B·D1-10은 PASS다. D1-11은 네 번의 사용자 Preview에서 FAIL했고 기존 flattened `Canvas_Mock_*` Border Mosaic은 active 제작 경로에서 폐기했다. Production 구조는 `WBP_CFInGameHUD` Root 1개, Mission/Alert/Target/Vehicle/Radar/Weapon Panel 6개, `WBP_CFSpeedGauge`·`WBP_CFArmorBodyMap` Element 2개, `DA_CFHUDVisual_Default` 1개로 실제 자산화됐다. 2026-08-12 Pre-Preview Review에서 발견한 Gear D·Armor 숫자/세로 Bar 누락·가짜 Speed/RPM Fallback은 `CFUIHUDProdEditorBridge v1.1.0`으로 교정했다. SpeedGauge는 Gear `N`, 실제 현재 RPM 값을 만들지 않는 고정 21 Tick/85% Red Zone 비대칭 Tachometer Scale과 전용 `Image_RPMTrackArt`를 사용하고, ArmorBodyMap은 6방향 숫자를 제거한 뒤 각 Plate/Badge 우측의 6개 BottomToTop 세로 Bar만 사용한다. 기존 v1.1.0 검증 증거에 더해 `CFUIHUDProdEditorBridge v1.2.0`은 `SpeedArcTrack=None` 또는 Load 실패 시 `Image_RPMTrackArt`를 `Collapsed`로 저장하고 Brush Resource/Visibility Validator를 적용한다. 사용자 종료 후 공식 Build `cef0c064cd434848b1fcb94459fbccb5`, Production Apply `f1c32cab41b544d6b27709315029bb04`, 최종 Saved Readback `c0bdf6a72df3412da78057406e843c0c`, Fresh AssetDump dataset `adset_v1_aa5d0dfdce87eb6a015641240dbc3cdd.cf961b240264a26f8e54bd19`이 모두 PASS했다. Readback 도구 `ApplyUIHUDProduction.py v1.1.0`은 Structure-first 계약에 따라 `d1_11_structure_pass=true`, `d1_11_pass=true`, `d1_11_art_user_approval=Pending`을 분리 기록한다. AssetDump는 SpeedGauge 21 Tick·Gear N·`Image_RPMTrackArt visibility=Collapsed`, ArmorBodyMap 6방향 세로 Bar, Root 여섯 Panel + ReticleLayer를 독립 확인했다. 따라서 **D1-11 Production Structure Gate는 PASS**다. 2026-08-13 사용자 결정에 따라 D1-11-ART-01 Source Art 사용자 Visual Review와 세부 배치·픽셀 폴리시는 기능 개발을 차단하지 않는 후순위 작업으로 전환했다. 이후 UI-P0-02 Runtime Lifetime PIE 전 항목이 USER PASS됐고 UI-P0-03 Source를 실제 적용했다. 현재 Gameplay Runtime은 `UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter`를 거쳐 기존 Production `WBP_CFInGameHUD`에 연결되며 Production Widget 자체는 Gameplay Pawn/Component를 직접 조회하지 않는다. Launcher USER Gate는 finite Ammo와 Salvo를 함께 사용하는 `/Game/Maps/TestMap_AmmoSalvo`에서 5/5 PASS로 닫혔다. 이후 `CFHUDDataTests v1.6.0`을 보강해 실제 Defense Runtime과 저장 `M_VehicleDefensePIE`의 PIE 복제 SUV가 Provider Defense ViewData로 전달되는 경로, Pawn Rebind 뒤 Old Pawn Defense 이벤트 해제와 Current Pawn 이벤트 반영을 자동 검증했다. 최종 공식 Build `681c91810da34066bb398ad1b0989f1e`은 Exit Code 0이고 targeted `CarFight.UI.UI_P0_03` Automation `f6b08a970bbf4ec282893e86e92e72ac`는 5/5 Success·0 Fail이다. 2026-08-18 USER PIE에서 Defense Production Panel은 Shield `0/100`, 방향 Armor 손상, Integrity `40/100`과 이후 Shield `100/100` 재생을 실제 Production HUD에서 확인해 PASS했다. 이어 Baseline Pawn으로 Rebind했을 때 Shield 행이 사라지고 Integrity `100/100`으로 전환됐으며, Old Defense SUV에 추가 피해를 적용해 파괴해도 Current Pawn과 HUD가 변하지 않아 Old Pawn 이벤트 격리까지 USER PASS했다. UI-P0-03은 종료하고 현재 Gate는 UI-P0-04 AimReticle UISubsystem 통합의 Build·Automation·USER Visual이다.
- `DA_CFUIStyle`과 공통 Base Widget 5종을 포함한 CarFight UI 디자인 시스템 기반
- CommonUI 플러그인·모듈·Activatable Screen Stack은 미도입 상태이며 별도 도입 Gate에서 판단

### 2026-08-19 Visual Foundation 재진입 체크포인트

상태: `D1-11-ART Remaining USER Visual Deferred / ArmorBodyMap Designer Editability USER PASS / UI-P0-07 Target Knowledge In Progress`

사용자가 UI-P0-07 Target Knowledge 구현에 들어가기 전에 현재 Production HUD가 최종 게임 UI로 수렴할 수 있는 시각 기반을 먼저 검증하기로 결정했다. 이 결정은 UI-P0-06 Technical Complete나 D1-11 Production Structure PASS를 다시 여는 것이 아니다.

```text
VF-00 Art Ingress
→ PASS — Tools/GenerateUIHUDArtP1.ps1 v1.0.1이 SourceArt/UI/HUD/P1에 직접 PNG를 생성

VF-01 Visual Language / Asset Contract
→ PASS — InGameUIVisualConcept + InGameUIStyleSpec + InGameUIHUDArtSpec 기존 Accepted/Technical 계약 재사용

VF-02 Visual Direction
→ USER APPROVED — Balanced Combat
→ Dark layered frame + Cyan system accent + Amber armor/caution accent + restrained glow
→ 직전 원형 계기판/임의 배치는 분위기 Reference일 뿐 구조로 채택하지 않음

VF-02B Actual VehiclePanel Concept
→ COMPLETE FOR TECHNICAL APPLY — InGameUIVehiclePanelSpec v0.20.0 실제 896×416 구조 고정
→ SpeedGauge / ArmorBodyMap / ShieldRow / IntegrityRow 위치·의미 계약 변경 없음
→ 기존 P1 9종은 Technical reference baseline only

VF-03 VehiclePanel Production Vertical Slice
→ TECHNICAL APPLIED / USER VISUAL IN PROGRESS
→ Armor: `WBP_CFArmorSector` 재사용 Element를 추가해 방향별 `Image_ArmorPlate + Text_Direction + ProgressBar_Armor`를 한 묶음으로 소유하고 `WBP_CFArmorBodyMap`은 Vehicle + Sector 6개 공간 관계만 소유
→ RPM: 구형 `Image_RPMTrackArt + ProgressBar_RPMTick00~20` 21개 구조를 제거하고 `Image_RPMGauge + M_UI_RPMGauge + T_UI_RPMTrack + RPMRatio` 단일 Material 표현으로 전환
→ RPM Official Build `87be4ca3923e4a1583f74d14462d39a6` PASS / final Material If-pin fix targeted apply `4775e7f4439f4f72a0e5dc38671fe14d` PASS / focused RPM Automation `54330f1edb7e4a2c9679700fe5c01224` 3/3 PASS
→ USER Designer에서 RPM Track/Tick 실제 표시 확인 PASS. 21개 UMG ProgressBar 재도입 금지
→ USER가 ArmorBodyMap Sector 배치를 직접 조정했으며 현재 실제 편집 과정에서 위치·크기 조절을 C++ CanvasSlot/SizeBox override가 과도하게 제한하는 UX 결함을 확인
→ `Designer Layout Ownership Remediation` TECHNICAL PASS: `CFUIHUDProdEditorBridge.h v1.9.0 / .cpp v1.12.0`은 기존 RootWidget이 있는 Production Widget/Root Build를 fail-closed하고 신규/빈 Asset만 최초 Scaffold 허용. Validator는 Widget 존재·이름·타입·데이터 의미만 보호하며 Position·Size·Anchor·Alignment·Padding·AutoSize는 persisted UMG Designer Asset이 소유
→ `ApplyUIHUDProduction.py v1.5.0 / RunUIHUDProduction.ps1 v1.4.0`은 기존 Widget을 Validate-only로 처리. Official Build `9125557b4e214a4ebe2bd642099929aa` PASS / saved readback `b543596ea3404a8084b46e5349ce7e84` PASS·saved0 / `ArmorMapOnly` `ad6e8d64733249b0be26d8ab9506acca` PASS·layout preserved2·rebuilt0·compiled0·saved0·mutated0
→ USER가 저장한 `WBP_CFArmorBodyMap` Designer Layout은 targeted validation 과정에서도 재구축·저장되지 않음
→ fresh Editor `85cbeb26ecbd435594e6a0df508e6aa6` started / Ready / local 8100 Editor-owned / protocol ready. 다음 USER 조작부터 이 lifetime은 사용자 작업 보호 대상으로 취급

후속
→ `WBP_CFArmorBodyMap` 위치·크기 Designer 편집성은 USER 직접 확인 PASS
→ 사용자가 `WBP_CFSpeedGauge`·`WBP_CFVehiclePanel` 개별 편집성 및 전체 VehiclePanel Visual Review를 건너뛰고 Runtime 작업 진행을 선택했으므로 해당 항목은 Deferred이며 USER PASS로 올리지 않음
→ 현재 Runtime Gate는 UI-P0-07 Target Knowledge. Existing TargetSelect + Sensor Snapshot + FCFTargetHUDData를 재사용하고 Production TargetPanel Consumer만 최소 완성
→ T_UI_TargetBracket은 Target/Radar Visual 단계까지 Technical PASS Source로 보존 가능
```

보호 조건:

- `WBP_CFInGameHUD` Root 1 + Panel 6 + Element 2 + `DA_CFHUDVisual_Default` 구조를 유지하고 Border Mosaic 방식으로 회귀하지 않는다.
- Gameplay → Provider → Presenter → ViewData → Production Widget 계약을 Visual 작업 때문에 변경하지 않는다.
- 숫자·문구·Gameplay 상태를 Texture에 굽지 않는다.
- 완료된 P2 import/DA 연결과 RPM/Armor technical evidence는 새 failure evidence 없이 반복하지 않는다.
- `CFUIHUDProdEditorBridge`의 scaffold/build 경로는 이미 존재하는 Widget의 Designer 위치·크기를 재작성하지 않는다. 처음 없는 Widget을 만드는 기본 Scaffold와 persisted Designer Layout ownership 분리는 Technical PASS 상태로 보존한다.
- `WBP_CFArmorSector` 내부 구조, `WBP_CFArmorBodyMap` 6방향 의미, `WBP_CFSpeedGauge` 단일 RPM Material 의미 구조는 C++/Validator가 보호하되 픽셀 좌표와 크기를 장기 계약으로 하드코딩하지 않는다.
- USER가 직접 조정 중인 unsaved/dirty Designer 레이아웃은 기존 사용자 작업으로 보호하며 AI가 임의 저장·폐기·재생성하지 않는다.
- 남은 USER Visual Review는 Deferred 상태로 보존하며 PASS로 추정하지 않는다. 2026-08-20 사용자 명시 결정에 따라 이 Deferred 항목은 UI-P0-07 Runtime 진행을 더 이상 차단하지 않는다.

### 2026-08-18 UI 최우선 재개 체크포인트

상태: `CF-FQ-032 Single Active / UI-P0-03~05 USER PASS / UI-P0-06 Vehicle·Weapon HUD Current`

사용자가 CarFight의 현재 최우선 사항을 인게임 UI로 재지정했다. 이 우선순위 변경은 이미 통과한 UI 기술 Gate를 다시 여는 것이 아니다.

```text
완료
1. UI-P0-03 Defense Production Panel USER Visual — PASS
2. UI-P0-03 Pawn Rebind + Old Pawn Event Isolation USER Visual — PASS

완료
3. UI-P0-04 AimReticle UISubsystem 통합 — Build·Automation·USER Visual PASS

완료
4. UI-P0-05 TargetSelect Marker 통합 — Source·Official Build·focused Automation·USER Visual PASS

현재
5. UI-DESIGN-GATE의 D1-11 Production Structure PASS / Art Polish Deferred Non-Blocking 상태를 보존하고 UI-P0-06 차량·무기 HUD 착수

후속
6. UI-P0-07 Target Knowledge
7. UI-P0-08 Radar·화면 밖 마커
```

보호 조건:

- UI-P0-02 Runtime Lifetime USER PASS, Speed/Weapon Cooldown/Target 선택·해제 USER PASS, Ammo·Ripple·Salvo Presentation USER PASS와 Defense/Pawn Rebind 기술 Automation을 관련 Source 결함 없이 반복하지 않는다.
- Production HUD 구조는 기존 `Root 1 + Panel 6 + Element 2 + HUD Visual Data 1`을 유지하며 폐기된 flattened `Canvas_Mock_*` Border Mosaic 방식으로 돌아가지 않는다.
- 2026-08-18 당시 `CF-FQ-037 Scanner`는 Done이었고 후속 TargetPanel/Radar는 main_game `Document/Systems/Targeting/SensorContact.md v1.1.0` snapshot의 actor-free Snapshot/Knowledge 계약을 read-only source로 사용했다. 최신 Current owner는 문서 상단 Closure와 main_game `SensorContact.md v1.2.0`을 우선하며 Widget에서 Detection, LOS, Knowledge, Contact lifecycle을 재계산하지 않는 계약은 유지한다.
- Scanner 완료가 Radar Range/Zoom, 동적 Blip, TargetPanel/Radar USER Visual을 완료로 만들지는 않는다. 이들은 CF-FQ-032의 실제 후속 UI 작업이다.
- 2026-08-18 당시 `CF-FQ-038 DAUTH` checkpoint는 `DataAuthoringPlan.md v0.2.28 / DataAuthoringRoadmap.md v0.1.36 / P0-12 USER PASS 2 / UA-03 DriveState RequiredProfileMissing blocker`였다. 최신 Current checkpoint는 main_game ActiveWork/FeatureQueue와 `DataAuthoringPlan.md v0.2.47 / DataAuthoringRoadmap.md v0.1.52 / USER PASS 6 / 다음 UA-07`을 우선한다. 이번 RPM slice의 `RedlineStartRPM` additive leaf는 bounded compatibility만 수행했으며 당시 DAUTH USER Acceptance를 재개하지 않았다.

### UI-P0-06 Vehicle/Weapon HUD 착수 감사 — 2026-08-18

상태: `Technical Complete / Stage A+B / RPM Gauge / Weapon Heat / Applied Fitting Weapon Selection Runtime+HUD source / truthful Weapon Rail Visual Consumer / Player-facing Weapon Select Input / WeaponCharge P0 Runtime+HUD Resource Projection Technical PASS / Charge final Build 2d9f33261d3c427187f75338b8f37f1d PASS / exact WeaponChargeRuntimeResourceContract 8bf4e80d9f1b4c88b0fd557053b27810 1/1 PASS / explicit Charge all-zero Disabled / VehicleBattery는 future shared-power Gameplay dependency·현재 Unavailable/Collapsed / Rail USER Visual representative persisted multi-weapon content 전까지 Deferred / Heat·Charge tuning USER Visual·authoritative Redline/RPM USER Visual content-dependent Deferred / formal next Gate UI-P0-07 Target Knowledge`

- Fresh AssetDump로 Production `WBP_CFVehiclePanel`, `WBP_CFWeaponPanel`, `WBP_CFSpeedGauge`, `WBP_CFArmorBodyMap` 저장 상태를 다시 읽었다. VehiclePanel은 SpeedGauge·ArmorBodyMap·Shield·Integrity 네 영역, WeaponPanel은 Title·Reserve·LauncherSequence·Ammo·Heat shell·Cooldown/Status·WeaponRail 구조를 보존한다.
- 이미 충족된 Runtime 경로는 Speed, 실제 Shield·6방향 Armor·Integrity, Weapon Cooldown, finite `Loaded / MagazineCapacity + Reserve`, Reload, Ripple·Salvo와 Pawn Rebind/Old Pawn Event Isolation이다. UI-P0-03~05 USER PASS를 이번 Gate에서 반복하지 않는다.
- Vehicle Drive 추가 감사에서 `UCFVehicleDriveComp`가 실제 `UChaosWheeledVehicleMovementComponent`를 보유함을 확인했고 UE 5.8 공식 Chaos API의 `GetEngineRotationSpeed()`와 `GetCurrentGear()`를 `UCFHUDDataProvider v1.7.0`에서 읽기 전용으로 연결했다. Gear는 음수=`R`, 0=`N`, 양수=실제 전진 단수로 기존 Presenter/Production Gear Slot에 전달한다. Engine RPM은 `FCFVehicleHUDData.EngineRpm`까지 도달하고, Production SpeedGauge는 explicit Redline/Maximum mapping이 유효할 때만 기존 21 Tick Percent를 갱신한다. Redline이 0/Unavailable/invalid이면 21 Tick fill을 0으로 reset하며 `CurrentRPM/MaxRPM` 또는 Max 비율 fallback은 만들지 않는다.
- Weapon identity 재감사에서 `UCFVehicleWeaponComp`가 현재 호환 활성 `UCFEquipmentPresetData`를 public Getter로 제공하고, `CFFittingViewData`가 이미 `EquipmentPresetData.DisplayName`을 사람 읽기용 장비 표시 이름으로 사용하고 있음을 확인했다. 이에 `EquipmentPresetData.DisplayName` 계약을 Fitting·Combat HUD 공용 Player-facing 장비 이름으로 명확화하고, 호환되는 활성 프리셋의 비어 있지 않은 DisplayName만 `FCFWeaponHUDData.DisplayName`으로 연결했다. 빈 이름은 `Unavailable`이며 `WeaponId`·`EquipmentId`·`MountProfileId`·AssetName fallback은 금지한다.
- RPM Gauge upstream 계약과 Production Visual Binding은 Technical PASS다. `FCFVehicleMovementConfig.RedlineStartRPM`은 `EngineMaxRPM`과 독립된 explicit authored field이고 0은 backward-compatible 미설정이다. Current RPM은 Chaos `GetEngineRotationSpeed()`, Maximum은 VehicleData `EngineMaxRPM`, Redline은 VehicleData `RedlineStartRPM`에서만 가져오며 Presenter는 실제 Redline을 화면 0.85, EngineMaxRPM을 1.0에 piecewise mapping한다. 기존 저장 `WBP_CFSpeedGauge`의 `ProgressBar_RPMTick00~20` 21개를 구조 변경 없이 Runtime Percent sink로 사용하고 Redline 미설정/invalid에서는 21 Tick fill=0으로 fail-closed reset한다. persisted representative VehicleData에는 authoritative Redline source가 없으므로 Asset 값은 모두 0 Unconfigured를 유지했고 실제 Redline authoring 결정과 RPM USER Visual은 Pending이다.
- 남은 WeaponGroup·VehicleBattery·WeaponCharge·Heat fresh audit에서 WeaponGroup은 player-facing 목록/선택 Index Runtime이 없고 내부 `WeaponGroupId/MountProfileId`만 존재, VehicleBattery는 HUD placeholder와 장기 shared-power 설계만 있고 실제 공용 전력 Runtime 없음, WeaponCharge는 독립 자원 설계만 있고 current/max/consume/recover Runtime·정적 입력 없음으로 확인했다. Heat는 기존 `HeatPerShot / MaxHeat`와 P0 설계를 재사용할 수 있어 가장 독립적인 slice로 선정했다.
- Weapon Heat는 `HeatDissipationPerSecond` 기본 0과 `FCFWeaponHeatRuntime`을 추가했다. 세 explicit Heat 입력이 모두 유한한 양수일 때만 활성화하고 `UCFVehicleWeaponComp`가 CurrentHeat·자연 냉각·Overheated를 소유한다. 승인된 실제 한 발마다 `ApplyFireResultInternal`에서 Heat를 정확히 1회 누적하며 MaxHeat 도달 시 `WeaponOverheated`로 해당 무기만 차단한다. 별도 숨은 recovery percentage 없이 `max(MaxHeat - HeatPerShot, 0)`까지 냉각되면 다음 표준 한 발을 다시 허용한다.
- HUD는 실제 WeaponComp Heat만 `ResourceChannels::Heat` Percent로 Projection한다. Heat Technical PASS와 기존 Compact/Launcher 계약은 그대로 보존한다.
- Player-facing Weapon Selection fresh audit에서 Applied `FCFVehicleFittingSnapshot::ResolvedMounts`가 현재 차량의 실제 mounted weapon 집합과 결정론적 순서를 이미 보존함을 확인했다. 이 weapon-bearing 고정 순서를 표시/선택 순서로 사용하고 새 WeaponGroupId는 만들지 않는다. 내부 `MountProfileId`는 기존 FireOrigin·Ammo identity에만 사용하고 HUD에는 전달하지 않는다.
- `UCFVehicleFittingComp → UCFVehicleWeaponComp → ACFVehiclePawn → UCFHUDDataProvider`로 실제 selection runtime을 연결했다. 각 선택 무기는 독립 Cooldown/Heat를 보존하고 비선택 Heat도 자연 냉각한다. `RequestSelectWeaponIndex`는 Launcher active 시 기존 `WeaponChanged` cancel cleanup 후 WeaponComp와 현재 single active Turret Visual을 같은 선택으로 갱신한다. HUD 목록은 `EquipmentPresetData.DisplayName + bSelected`만 포함한다.
- final Build `62ffb62f69524bb18c3a9f11bb9f58f1` PASS, exact `WeaponSelectionRuntimeContract` `ff23cf6ac04e4b2c8cf0084b24173cdb` 1/1 PASS / Result SHA-256 `0ffa955f626d0de3f5ebf7b2ac594d7f8b039d268fe6efb2ab0730abfe77618c`다. 테스트는 Fitting 2무기 고정 순서→실제 Pawn 선택→A/B Cooldown·Heat 격리·비선택 냉각→Provider Selected Index/DisplayName 갱신까지 실행했다.
- Production `WeaponRail` fresh persisted audit에서 기존 자식 세 Image가 실제 weapon slot이 아니라 Turret/Ammo/Reload 의미 placeholder임을 확인했다. truthful Visual Consumer에서 이 세 Image를 제거하고 112×68 `SizeBox_WeaponRail1~3 + Text_WeaponRail1~3`, 8px gap 구조로 교체했다. 선택 무기는 기존 Header/Selected Card에만 유지하고 Rail에는 비선택만 Provider fixed order로 표시한다. 비선택 0은 Rail Collapsed, 1~3은 원본 1-based 순번 + 실제 `EquipmentPresetData.DisplayName`, 4+는 앞 2개 + `+N`; 이름 부재는 internal ID fallback 없이 `WEAPON`이다. 실제 weapon icon source와 비선택 무기별 Resource Summary source가 없으므로 icon/summary는 현재 P0에서 생략한다.
- truthful Rail final Build `941ea18597b3483594be8de3317e68fd` PASS와 exact `WeaponRailVisualContract` `7cc9ddd5b2204725bb3ec45c200d649b` 1/1 PASS는 보존한다. 이어 Player-facing Weapon Select Input을 `IA_SelectWeapon` Axis1D + 숫자 1~9 direct ordinal로 연결했다. Mouse Wheel은 Radar Range/Zoom 예약을 보존하고 게임패드 mapping은 임의 지정하지 않았다. final Build `dec0757b745342b4b90ff5f17b761eb0` PASS, `WeaponSelectInputAssetSetup` `189ac8704dea46c8bebf5b970d94a122` 1/1 PASS, persisted `IA_SelectWeapon` fp `1A8A0402`, `IMC_Vehicle_Default` fp `52D2D868` / mapping47→56, exact read-only `WeaponSelectInputContract` `daeaf649669d48c79746a65cc44b1b89` 1/1 PASS / Result SHA-256 `f61e41f8185ebe9053bad188c476277f0e0ddeb83aab048f0977f92bd18345dd`다.
- Rail USER Visual용 existing content를 fresh 감사했지만 `DA_TestSedan`, `DA_TestSUV`, Ammo Heavy/Ripple/Salvo Fitting 모두 실제 선택 무기 1개다. artificial 2무기 fixture는 Production HeavyCannon/RocketLauncher `WeaponMassKg=0`으로 `MISSING_MASS_SOURCE`, mass-valid test-only `Preset_HeavyFinite`/`Preset_RippleFinite`로 바꾸면 finite AmmoId 두 종류의 출격 loadout까지 별도 구성해야 했다. 사용자 지시에 따라 fixture 확대를 중단했고 실패 실행 생성 asset cleanup error0, persisted fixture0이다. 따라서 Rail USER Visual은 **representative persisted multi-weapon content가 실제 생길 때까지 Deferred**다. USER PASS로 추정하지 않는다.
- WeaponCharge closure에서 `CFWeaponChargeRuntime v1.0.0`, `UCFWeaponData v1.13.0`, `UCFVehicleWeaponComp v1.25.0`, `ACFVehiclePawn v2.153.0`, `FCFWeaponHUDData v1.12.0`, `UCFHUDDataProvider v1.13.0`, `UCFHUDPresenter v1.16.0`을 current 구현으로 고정했다. Maximum/Initial/PerShot/Recovery 네 explicit 값이 유효할 때만 Charge가 활성되고 선택 무기별 상태를 독립 보존·Game-Time 회복한다. 부족 시 `WeaponChargeInsufficient`, accepted result에서 한 발당 정확히 1회 소비한다. HUD는 actual Current/Maximum/Ratio/Insufficient만 `WeaponCharge` Percent 채널로 전달하고 Ammo/Launcher가 없으면 `CHARGE N%` Primary, 있으면 Secondary로 사용한다. FireState 우선순위는 `Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY`다. final Build `2d9f33261d3c427187f75338b8f37f1d`와 exact `WeaponChargeRuntimeResourceContract` `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS를 보호 evidence로 사용한다. closure source readback에서 Charge precheck/post-consume이 기존 동적 검증된 Heat hook의 동일 Pawn fire 함수 인접 지점임을 확인해 추가 protected-friend test는 만들지 않았다. Production/DataAsset Charge authoring과 VehicleBattery 변경은 0이다.
- `InGameUIVehiclePanelSpec`의 Accepted 계약에 따라 고정 VehiclePanel은 SpeedGauge / ArmorBodyMap / ShieldRow / IntegrityRow 네 영역만 소유한다. 실제 Module Damage Runtime이 없는 현재 부품 손상 목록·가짜 정상 상태를 만들지 않으며 후속 Alert/Compact Chip/Vehicle Detail 설계로 분리한다.
- Roadmap의 구형 `Mock Damage Provider` 문구는 현재 실제 `UCFVehicleDefenseComp → UCFHUDDataProvider → FCFDefenseHUDData → UCFHUDPresenter` 경로에 의해 치환됐다. UI-P0-06에서 Defense Mock을 다시 만들지 않는다.
- 첫 최소 Source는 `CFHUDViewData.h v1.5.0`에 `VehicleBatteryAvailability`, `WeaponChargeAvailability`를 additive 추가해 둘 다 Provider 부재 시 `Unavailable`을 명시하고 값 필드·가짜 Provider·Widget Row를 만들지 않았다. Drive slice는 실제 Chaos Engine RPM/Current Gear를 연결했다. Weapon foundation은 `CFHUDViewData.h v1.8.0`의 `ECFWeaponResourceChannelType / ECFWeaponResourceDisplayMode / FCFWeaponResourceHUDData / ResourceChannels`, 신규 `CFHUDViewData.cpp v1.0.0` additive projection, `CFHUDDataProvider.cpp v1.9.0`의 활성 EquipmentPreset DisplayName 연결, `CFEquipmentPresetData.h v1.3.0` Player-facing DisplayName 계약 정합화를 적용했다. Presenter parity는 `CFHUDPresenter.h v1.8.0 / CFHUDPresenter.cpp v1.9.0`에서 ResourceChannels를 Ammo·Reserve·Reload·NoAmmo·Cooldown·Launcher의 우선 Source로 소비하고 legacy fallback을 보존했다. Dynamic Resource Visual Stage A는 `CFHUDPresenter.h v1.9.0 / CFHUDPresenter.cpp v1.10.0`에 `ECFWeaponResourcePresentationRole`, `FCFWeaponResourcePresentationEntry`, `BuildWeaponResourceEntries()`를 추가해 기존 `ResolveAmmoPresentation / ResolveReserveAmmoPresentation / ResolveWeaponStatusPresentation / ResolveLauncherSequenceDisplay`와 LauncherSequenceRevision lifecycle을 재사용한다. Launcher Active/terminal은 Primary, 그동안 Ammo는 Secondary, 일반 상태는 단일 FireState다. Production Asset은 변경하지 않았다. `CFHUDDataTests.cpp v1.12.0`에 `ResourcePresentationProjection`을 추가했다.
- 첫 Official Build job `dd384ae037ab45e78ba7c8b45b725f39`은 수정 관련 TU compile을 통과했으나 당시 실행 중인 user/ownership-unknown Unreal Editor DLL lock 때문에 Link에서 `LNK1104`, Exit 6으로 종료했다. focused test 분리 후 compile-check `fe669c6a804249778fc8e6498e5b7645`에서도 `CFHUDDataTests.cpp` compile은 PASS하고 같은 lock에서만 Link가 실패했다. 사용자가 Editor를 안전하게 종료한 뒤 final Official Build `ffff500a53184d5294f24dbb42e4523d`가 Exit Code 0 PASS했다.
- UE MCP AutomationTestToolset은 현재 server policy에서 차단돼 테스트 실행 실패로 해석하지 않았고, 기존 `Tools/RunDataAuthoringTests.ps1`의 `-TestFilter` 진입을 execution method로만 재사용한다. Presenter migration 기준 Build `d436ee40c530474d8507a7f66453da72` + focused 5/5 PASS는 역사 증거로 보존한다. Stage A 첫 Build `7588e3170de34cecadba55ad3f804433`은 변경 TU compile PASS 후 ownership-unknown Editor DLL lock `LNK1104`에서만 실패했고, 사용자가 Editor를 안전하게 종료한 뒤 final Official Build `4c043a73e3054baab5f725b18cd7de6c` Exit 0 PASS를 확보했다. 최종 `CarFight.UI.UI_P0_06` process `73189888eb8042628db1b9499691c17c`에서 `MissingProviderAvailability / ResourceChannelProjection / ResourcePresentationProjection / ResourcePresenterParity / VehicleDriveRuntimeViewData / WeaponDisplayNameRuntimeViewData` 6/6 PASS / Failure 0, Result JSON SHA-256 `39be662e7125ef4972574a75b433119283b7a279fbab56e511b140aafd6c70fe`다. 이 Stage A evidence는 Stage B에서 별도 재연하지 않는다.
- Stage B 적용 전 fresh audit에서 저장 `WBP_CFWeaponPanel`이 Header Reserve + Launcher/Ammo/Heat/Cooldown 고정 Row + WeaponRail 구조임을 재확인했다. `UCFHUDPresenter v1.11.0`은 Resource Visual에서 `BuildWeaponResourceEntries()`를 ViewData 적용당 정확히 1회만 호출하고, Header Reserve만 별도 Resolver를 사용하며 기존 Launcher/Ammo/Heat/Cooldown Row 직접 적용을 제거했다.
- `CFUIHUDProdEditorBridge.h v1.3.0 / .cpp v1.6.0`은 WeaponPanel을 `VerticalBox_ResourcePresentation → Primary 1 + Secondary A/B + FireState 1` 의미 슬롯으로 생성하고, Validator가 구형 Launcher/Ammo/Heat/Cooldown 전용 Row의 동시 잔존을 거부한다. WeaponRail은 Player-facing WeaponGroup Runtime 전 기본 Collapsed다.
- `CFHUDDataTests.cpp v1.14.0`의 신규 `ResourceVisualSlotContract`는 실제 Production HUD에서 `Ammo/Cooldown → Launcher Active → terminal 첫 적용 1회 → 같은 terminal ViewData 다음 적용에서 Ammo/Cooldown 복귀`를 검증한다. 첫 실행 `ccd0ce6a60f449eb88929ccbdc275d6b`은 기능 값이 아니라 visible Widget의 공통 Presenter 정책 `HitTestInvisible`을 테스트가 `Visible`로 기대한 5건만 FAIL했고, Production 구현을 변경하지 않은 채 expectation만 교정했다. 최종 Official Build `0bc2f086d71b4a57933326757d88b50d` Exit 0 PASS 뒤 process `2dc5eb2a394b4053875f548cec2204bc`가 1/1 PASS / Failure 0, Result SHA-256 `78bef76e45eb93199d1e7bba0bed6d0a91802c513568bcab78568e77508f6611`이다.
- `ApplyUIHUDProduction.py v1.2.0 / RunUIHUDProduction.ps1 v1.1.0`의 `WeaponPanel-only` targeted mode로 process `b600bd339b0348b5a3001c01a541484a`를 실행해 existing `WBP_CFWeaponPanel` 정확히 1개만 rebuilt/compiled/saved했다. Report SHA-256은 `6356715d0ab2503c78c6e31aec31f3b494b6f6e1405958481c7ed9659e06baf1`이며 다른 Production Asset mutation은 0이다.
- fresh AssetDump dataset `adset_v1_9ca90ab2e764ee02cda305b45e91bb3b.23e84f05ff5481cd492c956f`에서 저장 WeaponPanel fingerprint `CFB0652D`, Header Reserve + `VerticalBox_ResourcePresentation` + Primary 1 + Secondary A/B + FireState 1 + Collapsed WeaponRail 구조와 구형 Launcher/Ammo/Heat/Cooldown 전용 Row 부재를 persisted evidence로 확인했다. Stage A focused 6/6과 UI-P0-03~05 USER PASS는 반복하지 않았다.

### UI-P0-04 사전 정적 감사 / 구현 설계 준비 — 2026-08-13


상태: `Source Applied / Official Build·Automation Pending / USER Visual Pending`

이번 준비는 UI-P0-03의 남은 USER PIE Gate를 건너뛰어 UI-P0-04 구현을 시작한 것이 아니다. 현재 Runtime·Source·Unreal Asset을 변경하지 않고, UI-P0-03 정적 구조 감사와 다음 단계의 소유권 이전 경계만 확정했다.

- UI-P0-03 정적 감사에서 `UCFHUDDataProvider`의 Old Pawn 이벤트·World Timer 해제와 New Pawn Rebind가 대칭이고, `UCFHUDPresenter`가 `BindingGeneration` 변경 시 Launcher Presentation 상태를 초기화하며 Production Widget이 Gameplay를 직접 조회하지 않는 것을 확인했다. 현재 공통 Launcher lifecycle을 막는 정적 구조 결함은 발견하지 않았다.
- `LauncherSequenceRevision` 기반 terminal Snapshot은 한 번의 Presenter/ViewData 적용으로 전달되므로 실제 Slate 화면에서 사용자가 인지 가능한지는 정적 분석이나 Automation만으로 확정하지 않았다. 이후 `/Game/Maps/TestMap_AmmoSalvo` USER PIE에서 SALVO 표시, terminal `SALVO 4 / 4`, Sequence 중 Cooldown·READY 비중복, terminal 이후 Cooldown→READY, 상단 AlertFeed 비중복을 사용자 5/5 PASS로 확인했다. Salvo 전용 시간 Hold는 다시 추가하지 않는다.
- 현재 `ACFVehiclePawn`은 `AimReticleWidgetClass`, `AimReticleWidgetInstance`, `bShowAimReticle`, `AimReticleZOrder`를 소유하고 `CreateWidget → SetVehiclePawnRef(this) → AddToViewport`로 `WBP_AimReticle`을 직접 생성한다. `UCFUISubsystem` 소유 Reticle을 병행 생성하면 이중 Widget이 되므로 UI-P0-04 구현에서는 생성 소유권을 한 변경에서 원자적으로 이전해야 한다.
- Fresh AssetDump에서 `/Game/CarFight/UI/WBP_AimReticle`은 `UCFAimReticleWidget` Parent를 사용하고 `Image_CenterDot`, 네 방향 Bracket, FireFeedback Text, `Image_WeaponReticle`을 보존한다. Blueprint EventGraph에는 Construct·PreConstruct·Tick 이벤트만 있고 Function Call·Variable Read·Variable Write Symbol은 0이므로 Gameplay/Lifetime 로직을 Blueprint에 새로 만들지 않고 기존 WBP를 그대로 Visual Layer로 재사용할 수 있다.
- UI-P0-04의 목표 소유권은 `UCFUISubsystem → ECFUILayer::HUD → WBP_AimReticle`이다. 별도 9번째 Root Layer를 만들지 않으며 `Game` Layer는 월드 위치 Marker 용도로 유지한다. Production HUD와 Reticle의 앞뒤 관계는 같은 HUD Layer 내부 Local ZOrder로 결정한다.
- `UCFUISubsystem`은 Reticle Widget Class와 Reticle Instance의 수명을 소유하고 Current Pawn 변경 시 기존 Widget을 재생성하지 않고 새 Pawn으로 Rebind한다. World Cleanup·Controller Unregister·Root 해제 전에는 Reticle의 Pawn 참조를 Null로 만들고 Parent에서 제거한다.
- `UCFAimReticleWidget`은 현재의 프레임 기반 Aim·Turret World Projection과 FireFeedback 의미를 유지한다. UISubsystem 소유 Widget이 Pawn보다 오래 살 수 있으므로 `VehiclePawnRef`는 UI-P0-04에서 Weak Pawn Binding으로 전환하거나 그와 동등한 Old Pawn 비보존 계약을 가져야 한다.
- `ACFVehiclePawn`의 BeginPlay·SetupPlayerInputComponent direct Reticle 생성 호출은 UISubsystem 경로를 활성화하는 같은 변경에서 비활성화한다. 다만 `BP_CFVehiclePawn`이 현재 `AimReticleWidgetClass=/Game/CarFight/UI/WBP_AimReticle`을 직렬화하고 있으므로 Legacy Property/API를 즉시 삭제하지 않고 Config 이관과 저장 Asset readback이 끝날 때까지 호환 경계로 보존할 수 있다.
- Blueprint 책임은 기존 `WBP_AimReticle`의 Layout·Anchor·Image·Style·Animation으로 유지한다. Pawn/Component Cast, Gameplay 판정, Widget 수명 소유권은 Blueprint에 추가하지 않는다.
- UI-P0-04 구현 후 검증은 Root HUD Layer Reticle 정확히 1개, Null→Pawn A→Pawn B→Null Rebind, Old Pawn 비보존, World 재진입 단일 수명, Legacy direct `AddToViewport` 비활성, CenterDot·WeaponReticle·FireFeedback 의미 보존을 포함한다.

UI-P0-03의 두 USER Visual Gate가 모두 PASS한 뒤 UI-P0-04 Source를 착수했다. `UCFUISubsystem v1.6.0`이 Config의 기존 `WBP_AimReticle` Class를 HUD Layer ZOrder 10에 단일 생성하고 Current Pawn 변경 시 같은 Widget을 Rebind하며 Cleanup 전에 Pawn 참조를 Null 처리한다. `CFVehiclePawn v2.148.0`은 BeginPlay·SetupPlayerInputComponent의 direct Reticle 생성과 `AddToViewport` 자동 경로를 제거하고 Legacy Class/Instance/API만 직렬화·호환 경계로 보존한다. `DefaultGame.ini`에 `DefaultAimReticleWidgetClass`를 연결했고 `CFUIFoundationTests v1.2.0`에 HUD Layer ZOrder 계약을 보강했다. 저장 WBP의 Layout·CenterDot·WeaponReticle·FireFeedback 시각 의미는 변경하지 않았다. 현재는 Official Build·focused Automation·단일 Reticle/Pawn Rebind USER Visual이 Pending이며 PASS 전 UI-P0-05로 넘어가지 않는다.

### UI-P0-05 사전 정적 감사 / 구현 설계 준비 — 2026-08-14

상태: `Source·Official Build·focused Automation PASS / USER Visual Pending`

이번 준비는 Paused `CF-FQ-026 / TS-P0-08`을 재개하거나 UI-P0-03·04를 건너뛰어 TargetSelect를 구현한 것이 아니다. 현재 TargetSelect Gameplay 계약과 저장 Asset을 읽기 전용으로 감사하고, CF-FQ-032에서 수행할 HUD 수명·표현 이전 경계만 확정했다.

- `UCFTargetSelectComp`는 후보 탐색, 선택 확정·해제, 선택 수명, Occlusion/Track 상태와 장비 조회의 Gameplay owner로 그대로 유지한다. UI-P0-05는 선택 상태를 UI Subsystem이나 Widget으로 이동하지 않는다.
- 현재 `ACFVehiclePawn`은 `TargetSelectWidgetClass`, `TargetSelectWidgetInstance`, `bShowTargetSelectHud`, `TargetSelectHudZOrder`를 소유하고 BeginPlay·SetupPlayerInputComponent에서 `CreateWidget → SetVehiclePawnRef(this) → AddToViewport(20)`를 직접 수행한다. 실제 UI-P0-05에서는 UISubsystem 경로 활성화와 Pawn direct Viewport 생성 비활성화를 같은 변경에서 원자적으로 처리해 중복 Marker Widget을 허용하지 않는다.
- Legacy `UCFTargetSelectWidget`은 단순 Visual이 아니다. 현재 `TargetSelectComp` 이벤트를 직접 구독하고 후보·선택 Actor, 거리, Track 상태를 읽으며 `NativeTick`에서 월드→화면 투영과 Text 갱신까지 수행한다. 따라서 UI-P0-05는 생성 소유권 이전과 함께 **의미 정보와 공간 투영 책임을 분리**한다.
- 선택 Target의 Player-facing 의미 정보는 이미 `UCFHUDDataProvider → FCFTargetHUDData → UCFHUDPresenter → WBP_CFTargetPanel` 경로가 소유한다. `IdentityAvailability`, `DisplayName`, `Relation`, `Category`, `InformationLevel`, `TrackState`와 향후 Distance는 이 Production TargetPanel 경로만 표시한다.
- `WBP_TargetSelect` / `UCFTargetSelectWidget`은 UI-P0-05 이후 **후보·선택 월드 Marker 전용 표현**으로 축소한다. 이름·거리·상세 Track Text를 Gameplay Actor에서 다시 조립하지 않고, 후보/선택/가림의 최소 Marker 상태와 월드 위치만 사용한다.
- 의미 상태는 TargetSelect 이벤트가 바뀔 때 캐시하고, 프레임마다 필요한 작업은 현재 후보·선택의 약한 Actor/TargetPoint에서 월드 위치를 해석해 화면 좌표로 투영하는 범위로 제한한다. 이는 `의미 데이터=이벤트 중심 / 공간 투영=프레임 중심` 기존 계약과 일치한다.
- UI Root의 `ECFUILayer::Game`은 코드상 `월드 위치 기반 마커` 전용 레이어이므로 TargetSelect Marker의 목표 소유 위치는 `UCFUISubsystem → UCFUIRootWidget::GameLayer → WBP_TargetSelect`로 고정한다. Production `WBP_CFTargetPanel`과 AimReticle은 HUD Layer에 남긴다.
- Production HUD 내부 `ReticleLayer` Slot은 HUD Layout Profile의 화면 공간 Slot이며 Root의 `Game` Layer와 다른 개념이다. TargetSelect 월드 Marker 수명을 Production TargetPanel이나 ReticleLayer에 합치지 않는다.
- Fresh AssetDump에서 `/Game/CarFight/UI/WBP_TargetSelect`은 `UCFTargetSelectWidget` Parent, 8개 Widget Tree, Candidate/Selected Root와 Text 슬롯을 보존하고 Blueprint Function Call·Variable Read·Variable Write Symbol은 모두 0이다. Blueprint Graph에 Gameplay 로직이 없으므로 Asset을 새로 만들지 않고 기존 WBP를 Marker Visual로 재사용할 수 있다.
- Legacy `UCFTargetSelectWidget::BuildTargetInfoText`는 `DisplayInfo.DisplayName`이 비면 `TargetActor->GetName()`을 사용하므로 내부 Unreal Actor 이름이 UI에 노출될 수 있다. UI-P0-05 실제 구현에서는 이 fallback을 제거하고 Marker가 Actor 이름을 표시하지 않게 한다.
- 추가 감사에서 `ACFVehiclePawn::GetTargetDisplayInfo_Implementation()`도 현재 `DisplayName = GetName()`을 Player-facing `FCFTargetDisplayInfo`에 넣고 있음을 확인했다. 따라서 단순 Widget fallback 제거만으로 내부 이름 유출을 완전히 닫을 수 없다. 명시적인 공개 이름 Provider가 없는 현재 P0에서는 내부 Actor 이름을 DisplayName으로 승격하지 않고, 공개 이름이 없으면 `FCFTargetHUDData.IdentityAvailability=Unknown`과 TargetPanel의 `???` 정책을 사용한다. 향후 Vehicle/Target Identity 데이터가 생기면 그 공개 DisplayName만 연결한다.
- `TargetId`는 내부 추적용 안정 키로 사용할 수 있지만 Player-facing 이름의 대체값으로 렌더링하지 않는다.
- Legacy `VehiclePawnRef`는 현재 강한 `TObjectPtr`이므로 UISubsystem 소유 Widget이 Pawn보다 오래 살 수 있는 구조와 맞지 않는다. 실제 UI-P0-05에서 Weak Pawn Binding 또는 동등한 명시적 Old Pawn 비보존 계약으로 전환하고, Rebind 시 Old `TargetSelectComp` Delegate를 모두 해제한 뒤 New Pawn만 구독한다.
- 기존 `Text_CandidateInfo`, `Text_SelectedInfo`, `Text_SelectedTrackState`는 첫 통합에서 Player-facing 의미 정보를 중복 표시하지 않도록 `Collapsed` 또는 Marker-only 시각 슬롯으로 취급한다. 최종 이름·관계·지식 정보는 Production TargetPanel이 단일 소유한다.
- 후보와 선택이 같은 Actor일 때 중복 Marker 억제, 화면 밖일 때 Marker만 숨기고 선택 상태는 유지, Occluded 선택의 별도 형태, 대상 무효화 시 안전 해제라는 TS-P0-06 계약은 보존한다.
- Screen-edge Marker는 UI-P0-08 범위이므로 UI-P0-05에서 새 Clamp/Edge Arrow를 추가하지 않는다. 16:9·울트라와이드 Projection 회귀도 실제 통합 이후 검증하되 TS-P0-08 후보 범위·튜닝 작업으로 확장하지 않는다.
- UI-P0-05 실제 구현 후 검증은 Root Game Layer TargetSelect Marker 정확히 1개, Legacy direct `AddToViewport` 비활성, Null→Pawn A→Pawn B→Null Rebind, Old Pawn/Delegate 비보존, World 재진입 단일 수명, 후보/선택/가림 형태 보존, 동일 Actor 중복 억제, 화면 밖 선택 유지, 내부 Actor 이름 0 노출, Production TargetPanel과 의미 Text 중복 0, 기존 가운데 버튼 선택·오른쪽 버튼 해제 회귀 보존을 포함한다.

UI-P0-05 Source 구현은 `UCFUISubsystem v1.7.0 → Game Layer → WBP_TargetSelect` 단일 소유, Current Pawn Rebind/cleanup, `CFVehiclePawn v2.149.0`의 direct Widget 생성 제거, `UCFTargetSelectWidget v1.1.0` Weak Pawn Binding과 Marker-only 표현으로 적용했다. 의미 정보는 Production TargetPanel이 단일 소유하고 Marker는 내부 Actor Name·거리·관계·Track Text를 표시하지 않는다. 의미 상태는 TargetSelect 이벤트에서 캐시하고 Tick은 현재 약한 Actor의 TargetPoint 화면 투영만 수행하며 화면 밖→재진입 Marker를 복구한다. Official Build `242dfee7186d46aaa98e3c24b25c0d09` PASS, focused `CarFight.UI.UI_P0_05.TargetMarkerLayerContract` `3834e606f2b54efb9ac5c09ea410b4f4` 1/1 PASS다. 현재 남은 Gate는 USER Visual이며, PASS 전 TargetPanel/Radar 다음 단계로 넘어가지 않는다. `CF-FQ-026 / TS-P0-08`의 후보 범위·가독성·튜닝은 별도 Paused 체크포인트로 보호한다.

### TargetPanel / Radar ViewData 구현 계약 준비 — 2026-08-14

상태: `Design Contract Prepared / Source·Config·Asset Not Modified / Runtime Provider Not Implemented`

`InGameUIDesign.md v0.35.0`에서 기존 Target/Radar 사용자 결정을 실제 Source 확장에 사용할 데이터 계약으로 내렸다.

- TargetPanel은 `Tracking / Identity / Analysis / Defense / Weapon / Module` Sub-ViewData 구조를 사용한다. 기존 `FCFTargetHUDData v1.0.0`은 실제 구현 시 additive 확장하고 즉시 삭제·리네임하지 않는다.
- `Visible/Occluded/Estimated/SignalLost` 추적 품질과 `Live/LastKnown/DestroyedHold` Contact 수명을 독립 상태축으로 유지한다.
- Target Intelligence는 `Unknown / Estimated / Known / Unavailable / NotApplicable` Knowledge와 동적 값의 `Current / Stale` Freshness를 분리한다.
- Tactical Analysis는 Category, Building/Decaying 상태, Progress와 Completion Revision을 별도 제공해 10Hz Refresh가 완료 Highlight를 반복 발생시키지 않게 한다.
- Defense는 Shield / 6방향 Armor / Vehicle Ratio·질적 상태를 보존하고 Weapon은 Family Summary, Module은 Player-facing Damage Summary만 제공한다. 실제 Provider 없는 Module Damage는 계속 Collapse한다.
- Radar는 `Sensor Runtime → HUD Adapter/Presentation Resolver → FCFRadarHUDData → WBP_CFRadarPanel` 경로를 사용한다. Widget은 Actor 검색·LOS·Sensor 탐지·Contact Lost Timer·Cooldown 계산을 하지 않는다.
- `ContactId`는 Sensor Tracking Record, `ResolvedTargetId`는 신뢰 가능한 Entity Resolution 결과로 분리한다. ID0 Unknown Contact는 TargetId 없이 존재할 수 있다.
- Radar Contact는 Live/LastKnown/DestroyedHold, 정규화 위치·고저차, Display Range 내/외, Selected Edge Marker와 신규 탐지 Revision을 display-ready 상태로 받는다.
- Radar Scan은 `Unavailable / Ready / Scanning / Cooldown`, 실제 Progress/Cooldown과 Scan Cycle Revision을 받으며 Sweep은 Scanning 시각 표현만 담당한다.
- `DisplayRangeMeters`와 `MaximumDetectionRangeMeters`를 분리하고 Wheel 단계식 Zoom은 순수 표시 범위만 바꾼다. Range Ring 3개는 Zoom 단계 수가 아니라 현재 표시 범위 25/50/75% 기준선이다.
- Passive Tracking, Visual Acquisition, 360° Active Scan, Scanner Occlusion, Last Known 만료, Range Preset, Tactical Analysis Build/Decay는 후속 Sensor/Scanner Gameplay Provider가 소유한다.

이 계약은 UI-P0-07~08 구현을 준비한 것이며 현재 실행 순서를 앞당기지 않는다. Sensor/Knowledge Runtime이 실제로 없으므로 Production Radar는 계속 `Unavailable`이고 Target Intelligence 신규 Block도 실제 Source가 생길 때까지 값을 생성하지 않는다.

---

## 3. 완료 목표

`CF-FQ-032`의 P0 완료 조건은 다음과 같다.

1. `ACFPlayerController`가 Pawn 수명과 독립된 Pause·UI 공통 입력을 소유한다.
2. 인게임 HUD와 Pause Menu가 `UCFUISubsystem`의 World별 UI Root 아래에서 관리된다.
3. Pawn 교체, Possess 해제, 파괴와 레벨 재진입에서 중복 Widget과 이전 데이터 구독이 남지 않는다.
4. 싱글플레이 Pause 시 게임플레이 전체가 정지하고 Pause 진입 전 입력 잔류가 제거된다.
5. 현재 AimReticle 의미와 발사 피드백 회귀가 유지된다.
6. 숫자형 속도와 차량 상태를 Widget의 직접 Pawn Cast 없이 표시한다.
7. Shield / Armor / Vehicle Integrity와 부품 상태 View Data 계약이 존재한다.
8. `Unknown`, `Unavailable`, `KnownZero`를 구분한다.
9. 탄약, 배터리, 충전, 열과 쿨다운을 공통 UI 채널로 표시할 수 있다.
10. 선택 타겟 패널이 고정 영역을 사용하고 미공개 정보를 `???`로 표시한다.
11. 게임플레이용 실제 타겟 데이터와 플레이어에게 공개 가능한 Target Knowledge 데이터를 분리한다.
12. Radar가 월드 Actor 검색이 아닌 Sensor Contact 데이터를 소비한다.
13. 의미 데이터는 이벤트 중심, 월드 투영은 프레임 중심, 연속 수치는 제한된 주기로 갱신한다.
14. 시각 경고가 우선순위와 중복 제거 계약을 가진다.
15. 외부 3인칭 HUD가 16:9, 21:9와 32:9에서 안전영역을 유지한다.
16. HUD·Pause·Target·Radar·Alert·Garage·Fitting·Inventory·Mission·Result·Loading·Tutorial UI가 공통 Style·Layout·Density·User Override 계약을 따른다.
17. 각 주요 UI 모듈이 안정적인 PanelId·SlotId와 교체 가능한 Blueprint Visual Widget Class를 가진다.
18. 지원되지 않거나 숨긴 정보는 `Collapsed` 처리되고 자동 Layout이 빈 공간까지 제거한다.
19. 색상·폰트·간격·크기·위치·Motion이 개별 Widget에 무분별하게 하드코딩되지 않는다.
20. 구현된 범위가 공식 빌드, 자동 테스트와 사용자 PIE를 통과한다.
21. 완료된 기능만 Systems 문서로 승격한다.

---

## 4. 구현하지 않는 범위

- 타이틀, 메인 메뉴, 차고와 피팅 콘텐츠
- 임무 선택, 로딩과 결과 화면
- 운전석 시점
- 완성형 포탑 시점
- 멀티플레이 Pause
- 다중 타겟과 다중 락온
- 정밀 스텔스와 전자전
- 완성형 방향별 장갑 UI
- 완성형 부품 분석과 약점 UI
- 게임 또는 UI 사운드

---

## 5. 보호 범위

- `Image_CenterDot` 조준 레티클 의미를 변경하지 않는다.
- `Image_WeaponReticle` 터렛 레티클 의미를 변경하지 않는다.
- 현재 Aim / FireFeedback 상태와 사용자 PIE 결과를 보존한다.
- TargetSelect 선택 상태 소유권을 UI로 이동하지 않는다.
- WeaponFire, Launcher, Projectile, Damage와 Pool 판정을 UI에서 계산하지 않는다.
- Paused `CF-FQ-029`, `CF-FQ-026`과 Ready `CF-FQ-030`, `CF-FQ-034`, `CF-FQ-035`의 체크포인트를 보호한다. 완료된 `CF-FQ-031`의 Ammo Runtime과 `Document/Systems/Combat/Ammo.md` 계약을 UI에서 재구현하거나 변경하지 않는다.
- 완료된 `CF-FQ-033`의 `VehicleDefense.md`, `HitDamage.md`, Shield·6방향 Armor·Integrity 계산과 DR-PIE 회귀를 UI에서 재구현하거나 변경하지 않는다.
- 기존 미커밋 변경과 unrelated 자산을 수정하지 않는다.

---

## 6. 아키텍처 방향

### C++ 책임

- LocalPlayer 소유 UI Subsystem
- UI Root 생성, 제거와 화면 수명
- 레이어와 화면 전환
- Pause 정책 진입·해제
- UI View Data 구조체
- 게임플레이 이벤트를 UI 데이터로 변환하는 Presenter 또는 Provider
- 데이터 유효성, 수명과 테스트 가능한 계약

### Blueprint 책임

- Widget 레이아웃과 앵커
- 상태별 표시와 숨김
- 색상, 아이콘과 애니메이션
- 안전영역과 해상도 대응
- 디자이너 조정 가능한 스타일 값

### 전체 UI 커스터마이징 필수 계약

이 계약은 WeaponPanel 한정이 아니라 CarFight 전체 UI에 적용한다.

```text
Gameplay System
→ Presenter / ViewModel
→ 교체 가능한 Blueprint Visual Widget
→ Style / Layout / Density Data Asset
→ User Override
```

필수 원칙:

- 하나의 거대한 Widget이 전체 UI 기능·배치·스타일·Gameplay 조회를 동시에 소유하지 않는다.
- Root는 Layer와 Slot Container만 소유하고 Vehicle·Weapon·Radar·Target·Alert와 각 Screen은 독립 모듈로 구성한다.
- 각 주요 모듈은 안정적인 `PanelId` 또는 `SlotId`와 교체 가능한 Visual Widget Class를 가진다.
- C++은 상태·판정·View Data와 수명을 소유하고 Blueprint는 배치·브러시·아이콘·폰트·애니메이션을 소유한다.
- 색상·Font·Padding·Spacing·Size·Position·Opacity·Motion은 Style·Layout·Density Data Asset의 기본값으로 관리한다.
- 개별 Override는 명시적인 `bOverride...` 또는 User Override 구조를 사용한다.
- 미지원 채널과 사용자가 숨긴 요소는 `Collapsed` 처리해 행·간격·예약 공간까지 제거한다.
- Panel 내부는 자동 Layout을 우선하고 Canvas 고정 좌표는 Root·Reticle·Projection·Radar Blip처럼 좌표 자체가 의미인 범위로 제한한다.
- `Compact`, `Standard`, `Expanded`, `Custom` Density와 16:9·21:9·32:9 Layout Profile을 확장 가능하게 유지한다.
- 기본 Widget과 Profile은 수정 불가능한 최종본이 아니라 복제·상속·교체 가능한 기본 Preset이다.
- P0에서 사용자 HUD 편집 화면과 저장을 구현하지 않더라도 위치·Scale·Opacity·Visibility·Density Override와 Profile Version을 후속 추가할 수 있어야 한다.

### 권장 클래스와 자산 이름

```text
C++
ACFPlayerController
UCFUISubsystem
UCFInGameHUDWidget
UCFPauseMenuWidget
FCFVehicleHUDData
FCFWeaponHUDData
FCFTargetHUDData
FCFRadarContactHUDData
FCFResourceViewData
FCFCombatAlertViewData

Blueprint
WBP_CFUIRoot
WBP_CFInGameHUD
WBP_CFVehicleHUD
WBP_CFWeaponHUD
WBP_CFTargetPanel
WBP_CFRadar
WBP_CFPauseMenu
WBP_CFAlertFeed
```

모든 제안 파일명은 32자를 넘지 않는다.

### UI 디자인 품질 개선 트랙

UI 디자인 품질은 CommonUI 도입 여부와 분리해 관리한다. 현재 P0에서도 기능만 존재하는 임시 외형을 누적하지 않고, 최소한의 시각 계층·가독성·상호작용 인지성을 갖춘 `Clean Prototype`을 목표로 한다.

```text
디자인 품질이 해결하는 문제
- 정보 우선순위와 시선 흐름
- 버튼·패널·게이지의 시각적 인지성
- 색상·폰트·여백·아이콘·애니메이션 일관성
- Hover / Focus / Pressed / Disabled 상태 구분
- 16:9 / 21:9 / 32:9 Safe Zone과 가독성
```

권장 산출물:

```text
DA_CFUIStyle
WBP_CFButtonBase
WBP_CFPanelBase
WBP_CFStatusBar
WBP_CFInfoRow
WBP_CFAlertItem
```

C++은 View Data와 상태 의미를 소유하고, Blueprint/UMG는 레이아웃·스타일·애니메이션을 소유한다. 에셋 없는 C++ WidgetTree는 기능과 수명 검증용으로 사용할 수 있지만 최종 디자인 기준으로 간주하지 않는다.

### CommonUI 도입 Gate

CommonUI는 디자인 품질 도구가 아니라 전체 화면 UI의 입력 라우팅, 게임패드 내비게이션, Back 처리, Activatable Screen 수명과 화면 Stack을 표준화하는 프레임워크 후보로 취급한다.

현재 결정:

```text
인게임 전투 HUD P0
→ UMG + UCFUISubsystem 유지

타이틀·차고·피팅·임무·결과 등 첫 전체 화면 Flow 기획
→ UI-COMMONUI-GATE 실행
→ UMG 유지 또는 HUD UMG + Menu CommonUI 혼합 구조 결정
```

CommonUI 미도입은 HUD 디자인을 투박하게 유지할 이유가 아니며, 디자인 개선을 CommonUI 도입 시점까지 미루지 않는다. 반대로 외형 개선만을 이유로 CommonUI를 도입하지 않는다.

CommonUI 도입 여부가 바뀌어도 다음 계약은 유지한다.

- `UCFUISubsystem`의 LocalPlayer·World 수명 진입점
- C++ View Data와 Presenter/Provider
- Gameplay 계산과 UI 표현의 분리
- 상시 전투 HUD가 일반 UMG Widget으로 동작할 수 있는 구조

---

## 7. 구현 전 확정 계약

다음 항목은 `CF-FQ-032` 구현 중 임의로 변경하지 않는 기본 계약이다.

1. UI는 현재 `UMG + ULocalPlayerSubsystem` 구조를 사용한다.
2. CommonUI는 현재 P0에 도입하지 않는다. 향후 전체 게임플로우 요구가 커질 때 별도 기능으로 재검토한다.
3. 최소 기능의 `ACFPlayerController`를 추가한다.
4. Pause·Back·System·UI 공통 입력은 PlayerController가 소유한다. 차량 Gameplay 입력은 Pawn `DefaultInputMappingContext`가 소유하며 Controller로 원자 이전하기 전에는 Controller Gameplay Context를 `None`으로 유지한다.
5. HUD Widget 수명은 `UCFUISubsystem`이 소유한다.
6. Gameplay Pawn이 Widget을 직접 생성하는 현재 경로는 전환 게이트를 거쳐 단계적으로 제거한다.
7. Subsystem과 Presenter의 Pawn·Actor 참조는 약한 참조를 기본으로 한다.
8. Subsystem은 LocalPlayer 수명을 가지지만 실제 UI Root Widget은 World마다 새로 만들고 이전 World Root를 제거한다.
9. 의미 데이터는 이벤트 기반, 공간 투영은 프레임 기반으로 갱신한다.
10. 속도와 거리 같은 연속 수치는 제한된 주기로 갱신할 수 있다.
11. 게임플레이 판정용 Target Data와 플레이어 공개용 Target Knowledge View를 분리한다.
12. `Unknown`, `Unavailable`, `KnownZero`를 명시적으로 구분한다.
13. `UCFVehicleDefenseComp`가 초기화된 차량은 Shield·6방향 Armor·Vehicle Integrity를 실제 Runtime 값으로 표시한다. Legacy Fallback, 데이터 미지정과 아직 구현되지 않은 부품 손상은 각각 구분된 `Unavailable` 또는 Legacy 상태로 표시한다.
14. 화면 밖 마커는 단순 좌표 Clamp가 아닌 카메라 View Space 기반 방향 투영으로 구현한다.
15. 시각 경고는 우선순위, 지속 시간과 `AlertKey` 기반 중복 제거를 가진다.

### 7.1 피팅 호환 경계

`CF-FQ-032`는 피팅 기능을 구현하지 않는다. 다만 후속 피팅 기능이 UI 기반을 다시 만들지 않도록 다음 공통 계약만 준비한다.

```text
CF-FQ-032 소유
- UI Root의 Screen Layer
- 전체 화면 Screen 전환 진입점
- 공통 UI 입력과 Focus 정책
- 공통 UI Style
- ResourceType과 StatType 같은 표현용 공통 타입

후속 피팅 기능 소유
- Loadout 데이터와 저장
- Draft → Validate → Commit / Cancel 편집 세션
- 하드포인트·장비 호환성 검사
- 총중량·가속·제동·선회 등 예상 성능 계산
- 탄약·연료·배터리 적재
- 피팅 전용 View Data와 Widget
- Garage Preview Actor
- 출격용 Loadout Snapshot
```

추가 원칙:

1. `UCFUISubsystem`은 화면 수명만 관리하며 Loadout 데이터를 소유하지 않는다.
2. InGame HUD View Data와 Fitting Preview View Data를 분리한다.
3. 피팅 Widget은 실제 Gameplay Pawn을 직접 수정하지 않는다.
4. 확정된 Loadout은 출격 시 Runtime Snapshot으로 변환한다.
5. Garage Preview Actor와 Gameplay Pawn을 분리한다.
6. P0에서는 전투 중 피팅을 허용하지 않는다.
7. CommonUI 도입 여부는 실제 피팅·차고 기획 착수 시 다시 판단한다.
8. 피팅 기능 ID와 별도 Plan은 피팅 기획 착수 시 부여하며 이번 작업에서는 생성하지 않는다.

---

## 8. 작업 단계

| Task | 이름 | 상태 | 핵심 결과 |
|---|---|---|---|
| `UI-P0-00A` | 실제 코드 구조 조사 | Done | Pawn 소유 HUD, 입력, Tick, Target·Health 계약과 Pause 영향 검토 |
| `UI-P0-00B` | 구현 전 계약 확정 | Done | PlayerController, 입력 수명, UI 소유권, 공개 상태와 World 수명 확정 |
| `UI-P0-01A` | PlayerController·입력 기반 | Code Complete / Build·Automation PASS / User PIE Pending | Pawn 독립 Pause·Back 요청, Controller 소유 System·UI Context와 Pawn 소유 Gameplay Context 분리, Possession 통지, Input Mode·Cursor·Focus 기반 |
| `UI-P0-01B` | UI Root와 레이어 | Code Complete / Build·Automation PASS / User PIE Pending | LocalPlayer Subsystem, World별 C++ Root, Game/HUD/Screen/Panel/Menu/Modal/System/Debug 레이어와 화면 전환 진입점 |
| `UI-P0-02` | 완전 Pause | Done / USER PASS | 실제 World Pause, Escape·Gamepad Pause/Back, Continue, 입력 잔류 제거, Ripple·Salvo 동일 Sequence 재개, Projectile·추진·World Timer 정지, 반복 Pause·레벨 재진입 Root/Menu 수명 USER PASS |
| `UI-P0-03` | HUD 데이터 계약 | Done / USER PASS | Vehicle/Weapon/Defense/Target/Radar/Alert ViewData, Provider·Presenter, `OnCurrentPawnChanged` Rebind와 Production HUD 연결 및 Production PIE PASS |
| `UI-P0-04` | AimReticle 통합 | Done / USER PASS | 기존 의미·피드백 유지, UISubsystem HUD Layer 단일 수명과 Pawn Rebind PASS |
| `UI-P0-05` | TargetSelect HUD 통합 | Done / USER PASS | Game Layer 단일 Marker, Actor 이름 유출 제거, Projection-only Tick, Offscreen 재진입·Clear/Reselect PASS |
| `UI-P0-06` | 차량·무기 HUD | Technical Complete / Stage A+B + RPM + Heat + Weapon Selection Runtime/HUD + truthful Weapon Rail Visual + Weapon Select Input + WeaponCharge Runtime/HUD Technical PASS / VehicleBattery는 future Gameplay dependency·현재 Unavailable/Collapsed / Rail USER Visual·Heat/Charge tuning Visual·Redline/RPM Visual content-dependent Deferred | 숫자 속도, 방어 상태, 공통 자원 채널과 Compact Weapon Resource/Selection Presentation |
| `UI-P0-07` | 타겟 지식 패널 | Technical Complete | 고정 패널, `NO TARGET`, `???`, 식별·거리·Scan 공개, 선택 파괴·해제 수명 |
| `UI-P0-08` | Radar·화면 밖 마커 | Technical Complete / USER Visual·Zoom Feel Pending | Scanner Range/Zoom Foundation, Production Radar consumer, 실제 Mouse Wheel Zoom, Selected Target Camera View Space Screen-edge Marker Technical PASS. USER 시각·조작감 판정 잔여 |
| `UI-P0-09` | View Mode·경고·스타일 | Technical Complete / 09A~09D Technical PASS | 외부 3인칭 HUD의 차체·카메라·터렛 방향 소비, Alert duration/persistent lifecycle, 중첩 공통 Style Context 적용 |
| `UI-P0-10` | 통합 검증 | Technical Complete / AI Runtime Technical Validation | 4개 기준 해상도·Pause·Reticle·Target·Radar·Weapon cross-system 회귀, final UI 43/43, Ammo/Sensor focused, fresh PIE RuntimeRead·actual pixel HUD presence PASS. USER Visual·Feel PASS는 별도 |
| `UI-P0-11` | Systems 승격 | Complete | `InGameUI.md v1.0.0` Current 승격 + `AimReticle.md v1.9.0` owner 교정 + `SystemIndex.md v1.21.0` 등록 / Product mutation0 |

별도 Gate:

| Gate | 이름 | 실행 시점 | 상태 | 종료 결과 |
|---|---|---|---|---|
| `UI-DESIGN-GATE` | CarFight UI 디자인 기반 | UI-P0-03~05 구조 작업 뒤, UI-P0-06 신규 HUD 시각 구현 전 | D1-07~10 PASS / D1-11 Production Structure Gate PASS / D1-11-ART Visual Polish Deferred·User Approval Pending / D1-12 Not Started / 후속 해상도 Deferred | 기능·Runtime 연결은 진행 가능하며 Art Import·DA 연결과 D1-12는 별도 승인 전 시작하지 않음 |
| `UI-COMMONUI-GATE` | CommonUI 도입 판단 | 타이틀·차고·피팅·임무·결과 중 첫 전체 화면 Flow 구현 전 | Deferred / P0 HUD 비차단 | UMG 유지, 혼합 구조 또는 CommonUI 도입 결정 |

---

### UI-P0-09A View Mode / Direction Foundation — Technical PASS

- `FCFViewModeHUDData`를 추가해 `CameraMode`, `VehicleHeadingDegrees`, `CameraRelativeYawDegrees`, `CameraPitchDegrees`, `bTurretDirectionAvailable`, `TurretRelativeYawDegrees`, `TurretPitchDegrees`, `bTurretAligning`을 명시적으로 전달한다.
- `UCFHUDDataProvider`는 기존 `UCFVehicleCameraComp::GetCameraRuntimeState()`·`GetCurrentAimDirection()`과 `UCFVehicleAimComp::GetWeaponAimSolution().CurrentMuzzleDirection`만 읽는다. 새 Camera/Turret Gameplay 모드, 입력, 운전석 시점과 Gameplay mutation은 추가하지 않는다.
- `CFHUDViewData.h v1.14.0`, `CFHUDDataProvider.h v1.9.0 / .cpp v1.15.0`, `CFHUDDataTests.cpp v1.26.1` 기준이다.
- final Official Build `a78e59503f6e45ce98f952941ea2969a` PASS, focused `CarFight.UI.UI_P0_09.ViewModeDirectionFoundation` process `ba3fc0bb5d27448db14c0f28b9e76997` 1/1 PASS다. Content/Blueprint/DataAsset mutation은 0이다.
- 09A evidence는 후속 09B~D에서 재구현하지 않고 그대로 보존했다.

### UI-P0-09B Production View Mode Consumer — Technical PASS

- 기존 `WBP_CFInGameHUD` Root direct child 7개와 기존 ReticleLayer Slot을 유지하고 ReticleLayer 아래 `CanvasPanel_ViewDirection + Image_ViewVehicleDirection`만 additive로 추가했다.
- Vehicle Direction은 `FCFViewModeHUDData.CameraRelativeYawDegrees`를 소비하는 카메라 기준 차체 방향 표시이며 World Compass가 아니다. 기존 Command/Turret Reticle과 Gameplay Camera/Aim을 변경하지 않는다.
- targeted apply `642fe507a2d44a46a8d65a90d94253f0`가 정확히 `WBP_CFInGameHUD` 1 Asset만 저장했고 `root_tree_rebuilt=false`, `existing_root_slot_layout_reapplied=false`다.
- fresh persisted AssetDump `adset_v1_c383efcb0fecbbea174c60d8a3f07450.6450115accd261964a805ef9`에서 Root 7 direct child 유지와 ReticleLayer 아래 새 2 node만 확인했다.
- initial focused `ViewModeProductionConsumerContract` `e5c99cc0624248369930917bc3da47be` 1/1 PASS, 09C/D 뒤 regression `094dab7a01d6479286a0e0995062bc17` 1/1 PASS다.

### UI-P0-09C Alert Lifecycle — Technical PASS

- Provider의 기존 stable `AlertKey` dedupe와 Priority 계약은 유지하고 Presenter가 기존 `UCFUIStyleData.AlertStyle`을 실제 Presentation lifecycle에 소비한다.
- Notice는 2초, Warning은 3초, Critical은 Persistent 기본값을 사용한다. 같은 AlertKey의 반복 ViewData refresh는 최초 활성 Game-Time을 유지해 duration을 리셋하지 않는다.
- AlertKey가 실제 상태에서 사라지면 lifecycle을 제거하고 이후 재발생 때 새 duration을 시작한다. `LauncherSequence`는 계속 global AlertFeed에서 제외한다.
- `CFHUDPresenter.h v1.22.0 / .cpp v1.23.0`, `CFHUDDataTests.cpp v1.28.0` 기준이며 focused `AlertStyleLifecycleContract` process `1fc3c160e74644079459ce3dab8074d4` 1/1 PASS다.

### UI-P0-09D Nested Style Context — Technical PASS

- `UCFUISubsystem`이 Root에 해석한 Style/Density/Geometry/Typography Context를 `UCFStyledWidgetBase`가 WidgetTree 내부 중첩 `CFStyledWidgetBase`로 전달한다.
- 중첩 Panel은 Content 경로나 Gameplay를 직접 조회하지 않고 전달받은 Context만 소비한다. 동일 Context 재적용은 기존 early-return으로 중복 Visual Refresh를 만들지 않는다.
- `CFStyledWidgetBase.h/.cpp v1.1.0` 기준이며 실제 저장 Production HUD의 `WBP_CFVehiclePanel`, `WBP_CFAlertFeed`를 사용한 focused `NestedStyleContextContract` process `693b4a267fb5474e8b7a04b4b6b4e760` 1/1 PASS다.

### UI-P0-09 Technical Closure

- final combined Official Build `f209faafb10c4df1a446bf294f1f75ce` Exit 0 PASS다.
- UI-P0-09는 Technical Complete다. 이를 USER Visual PASS로 확대하지 않으며 UI-P0-08 Radar/Edge Visual·Zoom Feel Deferred/Pending도 그대로 유지한다.
- 다음 기술 Gate는 `UI-P0-10 Integration Validation`이다. UI-P0-03~09의 완료 기능을 재구현하지 않고 해상도·입력·수명·Pause·Reticle·Target·Radar·Weapon 통합 회귀의 실제 빈칸만 닫는다.

### UI-P0-10 Integration Validation — Technical Complete

- 해상도: `CFHUDDataTests.cpp v1.29.0`의 `CarFight.UI.UI_P0_10.ResolutionLayoutContract`가 현재 Project DPI Scale로 저장 `WBP_CFInGameHUD` Designer Root Slot을 해석해 1920×1080, 2560×1440, 3440×1440, 5120×1440에서 Production Panel 6개가 화면 안에 유지되고 서로 겹치지 않으며 ReticleLayer가 full-stretch임을 검증했다.
- Pause: `CFUIPauseTests.cpp v1.2.0`의 `ProgressFreezeContract`에 `UCFVehicleWeaponComp.PrimaryComponentTick.bTickEvenWhenPaused=false` 계약을 추가해 P0-06 이후 Heat 냉각, WeaponCharge 회복, TargetUse 주기 재평가가 World Pause에서 진행되지 않음을 기존 Launcher·Projectile·Timer freeze 계약과 함께 보호한다. Cooldown은 기존 `GetWorld()->GetTimeSeconds()` Game-Time 축을 유지한다.
- 입력: Keyboard/Mouse와 Gamepad는 기존 Enhanced Input mapping이 동시 coexist한다. Current Product에는 CommonInput/InputDevice/CurrentInputType/Glyph switching state가 없으므로 별도 장치 전환 UI state는 P0-10 N/A이며 새 시스템을 만들지 않았다.
- 수명·Reticle: UI-P0-02~05에서 USER PASS한 Root/Menu 반복 Pause·레벨 재진입, Pawn Rebind/Old Pawn isolation, 단일 AimReticle·Target Marker 수명을 보존했고 final `CarFight.UI` broad가 관련 Controller/Root/Rebind/TargetMarker/ViewMode 계약을 모두 통과했다. fresh PIE 실제 픽셀에서도 중앙 Reticle은 한 세트만 관측됐다.
- Target/Radar: `CarFight.Sensor.SEN_P0_06.HUDSnapshot` process `b998cfd70e8f4588804d7515fbbf946f` 1/1 PASS로 선택 Target 파괴 clear와 Sensor `DestroyedHold` Radar 보존의 독립 수명을 확인했고, `CarFight.Sensor.SEN_P0_03` process `39fa2a00184f497180a162f9b7a1ca7d` 2/2 PASS로 ContactLifetime과 Reacquire 동일 ContactId·Knowledge 보존을 확인했다. UI broad의 Radar Range/Zoom/Screen-edge/Target Knowledge도 모두 PASS다.
- Weapon: `CarFight.Ammo.AMMO_P0_05.Reload` process `1026fe688f654fbcbc202c995b8d9df7` 1/1 PASS, `CarFight.Ammo.AMMO_P0_06.HUD` process `acde34b532b7410db9f55ff1a217ad57` 1/1 PASS이며 final UI broad의 WeaponSelect·per-weapon Cooldown·Heat·Charge·Resource Presentation 계약도 PASS다.
- AI/Lock Pause: Current Product Source에는 실제 `AIController/BehaviorTree` Runtime과 Lock-on Gameplay Runtime이 없으므로 두 행은 N/A다. 검증을 위해 새 AI/Lock 시스템이나 Mock을 Product에 추가하지 않았다.
- Runner: `RunUIAutomation.ps1 v1.2.0`은 P0-10 read-only cross-system 회귀를 위해 `CarFight.UI`, `CarFight.Ammo`, `CarFight.Sensor`만 allowlist하고 `CarFight.Setup` 및 상위 `CarFight` filter를 fail-closed한다. 실제 `CarFight.Setup` canary process `7d3e8c75760543b5acf1c6f5d83c2314`가 Editor 실행 전 0.255초 내 거부되어 setup mutation 격리를 확인했다.
- Official Build: `1a632fcdac38424e9750633a95204d76` Exit 0 PASS다. final `CarFight.UI` process `2fdf9866d08e4ae2aa464c21e6a373bb`은 43/43 PASS, Failure 0이다.
- fresh runtime: stale legacy PIE ownership record는 실제 PIE0·과거 Editor PID임을 확인한 뒤 영구삭제하지 않고 Host-global quarantine으로 격리했다. fresh AI-owned Editor Runtime `runtime_c486b32759b9449da39878894ae3b306`, PID 9344에서 PIE process `14c0d695567b4569814a39229e77fcc9`가 retry0으로 시작됐다. Accepted RuntimeRead는 `TestMap` PIE, LocalPlayer 1, `CFPlayerController`, `BP_CFVehiclePawn`과 Drive/Aim/Weapon/Ammo/Health/Defense/TargetSelect/Sensor 핵심 Component를 직접 확인했다.
- actual pixel evidence: fresh PIE screenshot에서 Vehicle HUD, Radar, Weapon HUD, Target Panel `NO TARGET`, 단일 중앙 Reticle이 실제 생성된 것을 객관적으로 확인했다. Editor dock 때문에 공식 기준보다 좁은 비표준 viewport에서는 Weapon 이름이 우측에서 일부 잘려 보였으나 이는 4개 공식 해상도 Automation 실패가 아니며 이번 Gate에서 임의 레이아웃 튜닝으로 확대하지 않는다.
- cleanup: AI-owned PIE `eeb991c352074cb3874b875a27680eb7`와 Editor `c5aafcf877144f87b9e0a1ec31a719e6`를 retry0/save0/force-kill0으로 종료했다.
- 판정: UI-P0-10은 **Technical Complete**다. 위 fresh PIE는 **AI Runtime Technical Validation**이며 USER Visual·UX·Zoom Feel·조작감 PASS로 확대하지 않는다. UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual Review는 Deferred/Pending을 유지한다.
- 다음 기술 Gate는 `UI-P0-11 Systems Promotion`이다. 검증된 Current UI Root/HUD/Pause 계약만 Systems로 승격하고 Deferred USER 항목을 완료된 Current System으로 쓰지 않는다.

### UI-P0-11 Systems Promotion — Complete

- `Document/Systems/UI/InGameUI.md v1.0.0`을 신규 Current System으로 승격했다. LocalPlayer `UCFUISubsystem → UCFUIRootWidget` 8 Layer 수명, Production `UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter` 데이터 경계, 실제 Pause·HUD·Radar·Target·Weapon 현재 계약을 기록했다.
- `Document/Systems/UI/AimReticle.md v1.9.0`은 과거 Pawn direct CreateWidget/AddToViewport 설명을 Current에서 제거하고 실제 `UCFUISubsystem` HUD Layer Z10 singleton·Current Pawn Rebind·Weak Pawn 계약으로 교정했다. 기존 Aim/FireFeedback/Turret Reticle USER PASS 의미는 변경하지 않았다.
- `Document/Systems/SystemIndex.md v1.21.0`에 `InGameUI.md`를 UI Current owner로 공식 등록했다.
- 이 Gate는 문서 전용 Current Knowledge Promotion이다. Product Source·Config·Content Asset mutation은 0이며 UI-P0-10 Build·Automation·fresh AI Runtime evidence는 새 failure 없이 반복하지 않았다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART SpeedGauge·VehiclePanel·전체 Visual Review는 사용자의 진도 우선 결정에 따른 비차단 Deferred/Pending이다. 이 closure를 USER Visual/Feel PASS로 확대하지 않는다.
- 판정: **UI-P0-11 Complete / M6 Done / CF-FQ-032 Done**. 현재 구현 owner는 Systems이며 이 Plan은 Historical + Retained Path로 유지한다.

## 9. 의존 관계

```text
UI-P0-00A Code Review
→ UI-P0-00B Contract Lock
→ UI-P0-01A PlayerController / Input
→ UI-P0-01B World-owned Root
→ UI-P0-02 Full Pause
→ UI-P0-03 Data Contract
→ UI-P0-04 AimReticle Integration
→ UI-P0-05 TargetSelect Integration
→ UI-DESIGN-GATE
→ UI-P0-06~09 Feature Widgets
→ UI-P0-10 Integration
→ UI-P0-11 Systems Promotion

별도 전체 화면 Flow
→ UI-COMMONUI-GATE
→ UMG 유지 또는 HUD UMG + Menu CommonUI 혼합 구조 결정
```

외부 기능 의존:

| 기능 | UI 진행 방식 |
|---|---|
| CF-FQ-026 TargetSelect | 현재 선택 계약 사용, TS-P0-08 전에는 Mock/기존 HUD 회귀 보호 |
| CF-FQ-033 Shield/Armor Damage | 구현된 VehicleDefense Runtime을 읽기 전용 View Data로 변환하고 Legacy Fallback·Unavailable을 구분 |
| CF-FQ-030 Missile | Lock View Data 계약만 준비, 실제 유도·Lock 완료를 추측하지 않음 |
| CF-FQ-031 Ammo | 공통 자원 채널을 준비하되 실제 탄수는 Ammo Runtime을 소유자로 사용 |
| Sensor/Knowledge | Radar와 Target Panel은 Provider 인터페이스로 분리하고 Mock 가능 |

---

## 10. 검증

### 공식 빌드

```text
Tools\BuildEditor.bat
```

### 자동 테스트 후보

- LocalPlayer당 Subsystem 1개와 World당 Root 1개
- Possess 해제·Pawn 교체·레벨 전환 시 데이터 소스와 이벤트 구독 전환
- Mapping Context 중복 등록·잔류 방지
- Pause 진입·해제, 중복 호출과 입력 잔류 제거
- Pause 중 게임 타이머와 Launcher·Projectile 진행 정지
- `Unknown`, `Unavailable`, `KnownZero` 변환
- Target Knowledge가 실제 게임플레이 정보를 초과 공개하지 않음
- 자원 채널 타입별 표시 데이터
- Radar Contact 정규화와 화면 뒤·화면 밖 방향 투영
- Alert 우선순위와 `AlertKey` 중복 제거

### 사용자 PIE

- 외부 3인칭 주행·조준·발사 회귀
- Pause 중 차량, AI, Projectile, Lock, Reload, Heat와 Timer 완전 정지
- Pause 해제 후 정상 복귀
- 1920×1080, 2560×1440, 3440×1440, 5120×1440
- Target 선택·해제·파괴·정보 공개
- 무기 변경과 자원 상태 변화
- Pawn 파괴와 레벨 재시작 후 Widget 중복 없음

---

## 11. 완료 판정 금지 조건

다음 상태에서는 `CF-FQ-032 Done` 또는 UI Current System으로 판정하지 않는다.

- 문서만 생성됨
- Mock 데이터만 표시됨
- 공식 Editor 빌드 미실행
- 기존 AimReticle 회귀 미확인
- Pause 중 한 개 이상의 게임플레이 진행이 계속됨
- 필수 기능 USER Gate 미완료. 단, 사용자가 명시적으로 비차단 Deferred로 전환한 Visual·Zoom Feel·세부 Polish는 이 조건에서 제외
- Systems 문서 미승격

---

## 12. Historical 진행 기록 / 당시 다음 작업

> 이 섹션은 CF-FQ-032 진행 중 각 시점의 next-action과 USER/Technical checkpoint를 보존하는 Historical evidence다. 현재 작업 선택에는 사용하지 않으며 현재 상태는 문서 상단 Closure와 main_game FeatureQueue/ActiveWork/Systems를 우선한다.

`UI-P0-01A~02` C++ 구현과 자동화는 완료했다. 사용자 Standalone에서 Escape Pause, 실제 World 정지, Pause Root가 Legacy Reticle·Target HUD보다 위에 표시되는 것, 계속하기 마우스 클릭, Enter·Escape 해제와 해제 후 차량·카메라 입력 복귀를 확인했다. 첫 재검증에서 발견된 Root 계층과 입력 결함은 `AddToViewport(100)`과 Controller Enter 확인 Fallback으로 최소 수정했다.

UI-DESIGN-GATE 승인 완료 범위:

```text
D1-VEHICLE-PANEL Visual Layout Accepted
D1-WEAPON-PANEL Compact Visual Direction Accepted
- 우하단 464×360은 최대 Slot 경계
- 실제 Content는 Desired Size로 축소
- 이전 432×224 Card·136×96 Tile은 초기 넓은 Wireframe 참고값
- Primary Resource 1개 + Secondary 최대 2개 + FireStateStrip
- 미지원 Channel·Rail은 Collapse하고 빈 공간까지 제거
- 비선택 무기 최대 3 Compact Tile, 4개 이상은 2개 + Overflow
- 탄약형·에너지형·런처형·복합형 표시 규칙

GLOBAL-UI-CUSTOMIZATION CONTRACT Accepted
- Gameplay → Presenter/ViewModel → Blueprint Visual → Style·Layout·Density Data → User Override
- 안정적인 PanelId·SlotId와 Visual Widget Class 교체
- Compact·Standard·Expanded·Custom Density
- 16:9·21:9·32:9 Layout Profile
- 자동 Layout·Collapsed 빈 공간 제거·Canvas 사용 제한
```

D1-05와 D1-06 사용자 승인을 완료했고 D1-07용 `InGameUIStaticReview.md v0.6.0`의 외곽 재배치 `SR-1080-16` 수정본도 USER PASS를 받았다. 패널 크기는 유지하고 Phase 1 외곽 Margin을 실효 약 24px로 줄인 1920×1080 / 16:9 구성을 현재 기본 HUD 배치로 채택한다. SR-A/C/D/E 추가 Mockup은 문제 의심 시 선택 검토로 남기며 현재 D1-07 PASS를 차단하지 않는다. `SR-1440-16`·`SR-1440-21`·`SR-1440-32`는 후속 Layout Profile 확장으로 Deferred한다. D1-08~10과 D1-11 Production Structure는 PASS했고, `InGameUIHUDArtSpec.md v0.3.6` 기준 P2 Technical Apply·persisted readback·Designer Layout Ownership Remediation도 Technical PASS다. `WBP_CFArmorBodyMap` 위치·크기 Designer 편집성은 USER PASS했으며 `WBP_CFSpeedGauge`·`WBP_CFVehiclePanel` 개별 편집성과 전체 VehiclePanel Visual Review는 USER 결정으로 Deferred되어 USER Visual PASS로 추정하지 않는다. 이 Deferred는 더 이상 Runtime 진행을 차단하지 않으며 현재 Gate는 `UI-P0-07 Target Knowledge`다. 새 실패 근거 없이 Texture Import·VisualData 연결·Production Widget rebuild를 반복하지 않고 D1-12와 남은 Visual Review는 별도 후속으로 보존한다.

런타임 재개 지점:

```text
UI-P0-02 USER PASS
- Gamepad Pause·Back
- 입력 유지 중 Pause 후 잔류 없음
- Launcher Ripple·Salvo Pause 정지 후 동일 Sequence Index 재개
- Projectile 이동·ProjectileMotor 추진·World Timer Pause 정지
- 반복 Pause·레벨 재진입 Menu·Root 중복 없음

UI-P0-03 적용 완료 Source
- FCFInGameUIViewData: Vehicle / Weapon / Defense / Target / Radar / Alert
- UCFHUDDataProvider v1.2.0: Gameplay Runtime 읽기·Health/Defense/Launcher/Target 이벤트 구독·10Hz World Timer 연속값 갱신 / 정상 Launcher Sequence Alert 복제 제거
- UCFHUDPresenter v1.2.0: ViewData → 기존 WBP_CFInGameHUD 의미 요소 적용 / 정상 Launcher Sequence는 WeaponPanel Row·Progress 소유 / AlertFeed 전역 Warning 역할 유지
- UCFLauncherComp v1.3.0: 기존 발사 계산을 유지하고 시퀀스 상태 변경 이벤트 + terminal Cooldown 이후 Broadcast 순서를 제공
- UCFUISubsystem: Provider/Presenter/Production HUD 수명과 OnCurrentPawnChanged Rebind 진입점
- Unknown / Unavailable / KnownZero / Known 구분
- Gameplay Target Data와 Player-facing Target Knowledge 분리
- Widget 직접 Pawn/Component Cast 0

UI-P0-03 자동 검증 완료
- 최종 공식 Editor Build `681c91810da34066bb398ad1b0989f1e`: `CFHUDDataTests.cpp` 직접 Compile·제품 .lib·DLL Link·Metadata / Exit 0
- targeted Automation `f6b08a970bbf4ec282893e86e92e72ac`: `CarFight.UI.UI_P0_03` 5/5 Success / 0 Fail / Engine Exit 0
- `DefensePIEViewData`: 저장 `/Game/Maps/M_VehicleDefensePIE` 실제 PIE 복제 Defense SUV에서 Shield·Front Armor·Integrity 실제 Runtime → Provider ViewData 전달 Success
- `DefenseRuntimeViewData`: 실제 Defense 피해·Shield 재생 → Provider ViewData Success
- `ProviderPawnRebind`: Rebind 뒤 Old Pawn 실제 Defense 이벤트는 Provider Revision을 변경하지 않고 Current Pawn 이벤트만 즉시 반영 Success
- `ViewDataAvailability`, `WeaponLauncherSequencePresentation`: 기존 계약 Success 유지
- Evidence JSON: `UE/Saved/CarFight/UIAutomationResult.json` / SHA-256 `3991406d636663ead0b2249846a44b7b86ba5d2122656fb6b3dc09339e0568d7`
- Execution Method: 작업 전용 `Tools/RunUIAutomation.ps1 v1.0.3`; Reusable Entry Point: 공용 진입점으로 승격하지 않음

남은 UI-P0-03 USER Visual Gate
- USER PASS 완료: Speed 실제 증가 / Weapon Cooldown `READY → 남은 초 → READY` / Target 선택·해제
- USER PASS 완료: WeaponPanel `RIPPLE/SALVO N / Total` Presentation / terminal `SALVO 4 / 4` / Sequence 중 Cooldown·READY 중복 없음 / terminal 이후 Cooldown→READY / 상단 정상 Sequence Alert 없음
- Technical Success: Defense 실제 Shield·Armor·Integrity → Provider ViewData / Pawn Rebind Old Pawn 이벤트 해제·Current Pawn 반영
- USER Visual Pending: Production DefensePanel 실제 Shield·Armor·Integrity 변화
- USER Visual Pending: Pawn 교체 후 Production HUD가 새 Pawn만 표시하고 이전 Pawn 상태를 보존하지 않음
- 사용자 전체 PASS 전 UI-P0-03 완료·Systems 승격 금지
```

확인 완료한 사용자 PIE 범위:

```text
- UI-P0-02 Runtime Lifetime 전체 USER PASS
- Escape·Gamepad Pause/Back·Continue
- 실제 World Pause와 입력 잔류 제거
- Ripple·Salvo·Projectile·추진·World Timer 정지·재개
- 반복 Pause·레벨 재진입 Root/Menu 단일 수명
- UI-P0-03 Partial USER PASS: Speed 실제 증가
- UI-P0-03 Partial USER PASS: Weapon Cooldown READY → 남은 초 → READY
- UI-P0-03 Partial USER PASS: Target 마우스 가운데 버튼 선택 / 마우스 오른쪽 버튼 해제
- UI-P0-03 Launcher: 첫 Alert 역할 불일치와 Terminal 전이 교정 후 `/Game/Maps/TestMap_AmmoSalvo` USER PIE 5/5 PASS
```

아직 완료로 판정하지 않는 범위:

```text
- UI-P0-06 차량·무기 HUD
- UI-P0-07 Target Knowledge
- UI-P0-08 Radar·화면 밖 마커
- UI-P0-09 View Mode·Alert·Style 정제
- UI-P0-10 통합 검증
- UI-P0-11 Systems 승격
- D1-11-ART USER Visual Review·필요 시 P2 최소 Visual Polish와 D1-12
- CF-FQ-032 전체 완료와 Systems 승격
```

---

## 13. Changelog

### v0.59.36 - 2026-08-22

- 문서 정상화 Health Check로 상단 Protected Checkpoint의 DAUTH 상태를 USER PASS 6 / 다음 UA-07로 동기화하고 SensorContact Current owner를 v1.2.0으로 교정했다. UI-P0-06 진행 기록 안의 DAUTH PASS 2·SensorContact v1.1.0은 2026-08-18 당시 snapshot임을 명시했다.
- `다음 작업` 섹션을 Historical 진행 기록으로 명시해 내부의 과거 next-action·당시 Current 표현이 현재 작업으로 오인되지 않도록 했다.
- 기존 UI Build/Automation/USER evidence와 당시 Systems Promotion 버전은 삭제하거나 재작성하지 않았다.

Migration: 현재 CF-FQ-032 상태와 owner는 문서 상단 Closure 및 main_game Current 문서를 우선한다. 아래 과거 next-action은 Historical evidence다.

### v0.59.35 - 2026-08-22

- CF-FQ-032 post-closure code review remediation을 Historical evidence로 추가했다. P1 Target Identity·Alert suppression lifecycle과 P2 Screen-edge ViewData copy·AimReticle duplicate refresh·Provider Snapshot/Profile dedupe를 교정했다.
- final Official Build `bd3640c616794cd7a54cc8a8afa4b021` PASS, focused UI-P0-07 2/2·UI-P0-08 4/4·Alert 1/1·Sensor HUD 1/1, broad `CarFight.UI` 44/44·`CarFight.Sensor` 14/14 PASS다.
- Current owner는 main_game `Systems/UI/InGameUI.md v1.1.0`, `Systems/UI/AimReticle.md v1.10.0`, `Systems/Targeting/SensorContact.md v1.2.0`으로 갱신됐다. 이 Plan은 Historical을 유지하며 USER Visual/Zoom Feel Deferred 상태는 변경하지 않는다.

Migration: remediation 이후 현재 구현 판단은 위 Systems와 실제 Source를 우선한다. 과거 UI-P0-09C의 active-time Alert 설명이나 Actor-name identity fallback을 Current 계약으로 사용하지 않는다.

### v0.59.34 - 2026-08-22

- `UI-P0-11 Systems Promotion`을 완료했다. main_game의 `Document/Systems/UI/InGameUI.md v1.0.0`을 Current owner로 신규 승격하고 `AimReticle.md v1.9.0`을 실제 UISubsystem singleton/Rebind 계약으로 교정했으며 `SystemIndex.md v1.21.0`에 등록했다.
- UI-P0-10의 Build·43/43 UI broad·Ammo/Sensor regression·fresh AI Runtime Technical Validation은 replay하지 않고 Current Knowledge만 승격했다. UI-P0-11 Product Source·Config·Content Asset mutation은 0이다.
- UI-P0-11 Complete로 M6를 닫고 `CF-FQ-032`을 현재 P0 범위 Done으로 판정했다. 이 Plan은 Historical + Retained Path로 전환하며 현재 구현 판단은 Systems와 실제 Source/Asset을 우선한다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 사용자의 진도 우선 결정에 따른 비차단 Deferred/Pending을 유지하며 USER PASS로 확대하지 않는다. 현재 Active 기능은 이 Plan이 정하지 않는다.

Migration: 완료 후 `InGameUIPlan.md`의 old next-action은 Current 작업 선택에 사용하지 않는다. 다시 열 때는 main_game FeatureQueue와 `Systems/UI/InGameUI.md`를 먼저 확인해 새 lifecycle을 정의한다. 물리 파일 경로는 기존 링크 보호를 위해 유지한다.

### v0.59.33 - 2026-08-22

- `UI-P0-10 Integration Validation`을 Technical Complete로 닫았다. `CFHUDDataTests.cpp v1.29.0` ResolutionLayoutContract, `CFUIPauseTests.cpp v1.2.0` Weapon Resource Pause guard, `RunUIAutomation.ps1 v1.2.0`의 UI/Ammo/Sensor read-only allowlist와 Setup fail-closed를 current P0-10 기술 계약으로 기록했다.
- Official Build `1a632fcdac38424e9750633a95204d76` PASS, final `CarFight.UI` `2fdf9866d08e4ae2aa464c21e6a373bb` 43/43 PASS, Ammo Reload/HUD 1/1+1/1, Sensor HUD 1/1, Sensor Lifetime/Reacquire 2/2 PASS다.
- fresh AI-owned PIE `14c0d695567b4569814a39229e77fcc9`에서 RuntimeRead로 `TestMap` LocalPlayer·Controller·Pawn과 핵심 Component 구성을 직접 확인하고 actual pixel screenshot으로 Vehicle/Radar/Weapon/Target HUD와 단일 Reticle 생성을 객관적으로 확인했다. 이는 AI Runtime Technical Validation이며 USER Visual/Feel PASS는 아니다.
- Current에 실제 AI·Lock-on Gameplay Runtime이 없어 Pause 행의 AI/Lock은 N/A, 별도 장치 전환/Glyph state도 없어 입력 장치 전환은 기존 Enhanced Input mapping coexistence로 판정했다. UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 Deferred/Pending을 유지한다. 다음 기술 Gate는 `UI-P0-11 Systems Promotion`이다.

Migration: UI-P0-10 Build·broad/focused Automation·fresh AI Runtime evidence는 새 failure 없이 반복하지 않는다. UI-P0-11은 검증된 Current 계약만 Systems로 승격하고 USER Visual/Feel Deferred 항목을 완료로 추정하지 않는다.

### v0.59.32 - 2026-08-21

- UI-P0-09B Production View Mode Consumer를 기존 ReticleLayer 아래 Vehicle Direction Image additive 구조로 Technical PASS했다. targeted apply `642fe507a2d44a46a8d65a90d94253f0`는 `WBP_CFInGameHUD` 1 Asset만 저장했고 fresh AssetDump에서 Root direct child 7개와 기존 Slot layout 유지까지 확인했다.
- UI-P0-09C는 existing Style Alert duration을 실제 AlertKey lifecycle에 연결해 Notice 2초·Warning 3초·Critical Persistent와 repeated refresh non-reset을 구현했고 focused `1fc3c160e74644079459ce3dab8074d4` 1/1 PASS다.
- UI-P0-09D는 Root Visual Context를 중첩 `CFStyledWidgetBase`로 전달하고 동일 Context 중복 Refresh를 방지했으며 focused `693b4a267fb5474e8b7a04b4b6b4e760` 1/1 PASS다. 09B regression `094dab7a01d6479286a0e0995062bc17`도 1/1 PASS다.
- final combined Official Build `f209faafb10c4df1a446bf294f1f75ce` Exit0 PASS로 UI-P0-09를 Technical Complete로 닫고 다음 기술 Gate를 `UI-P0-10 Integration Validation`으로 이동했다. UI-P0-08 USER Visual·Zoom Feel은 Deferred/Pending을 유지한다.

Migration: UI-P0-09A~D evidence는 새 failure 없이 반복하지 않는다. UI-P0-10은 완료 기능의 재구현이 아니라 통합 검증 행렬의 실제 미검증 조합만 다룬다.

### v0.59.31 - 2026-08-21

- `UI-P0-09A View Mode / Direction Foundation`을 Technical PASS로 기록했다. `FCFViewModeHUDData`와 Provider read-only projection이 기존 VehicleCamera/Aim Runtime의 Camera Mode·Vehicle Heading·Camera/Turret 상대 방향·Turret Aligning을 전달한다.
- final Official Build `a78e59503f6e45ce98f952941ea2969a` PASS, focused `CarFight.UI.UI_P0_09.ViewModeDirectionFoundation` process `ba3fc0bb5d27448db14c0f28b9e76997` 1/1 PASS다. Content Asset과 Gameplay Camera/Aim mutation은 0이다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel은 USER Deferred/Pending으로 보존하고 현재 기술 Gate를 `UI-P0-09B Production View Mode Consumer`로 이동했다.

Migration: UI-P0-09A 기술 증거는 새 failure evidence 없이 반복하지 않는다. 09B는 persisted Production HUD와 Presenter의 실제 표시 gap만 최소 연결하며 새 Camera/Turret Gameplay Mode·입력·운전석 시점은 만들지 않는다.

### v0.59.30 - 2026-08-21

- `UI-P0-08 Screen-Off Selected Target Edge Marker`를 구현·검증해 UI-P0-08 기술 범위를 Technical Complete로 닫았다. `UCFTargetSelectWidget v1.2.0`은 Selected Target이 Safe Region 밖 또는 BehindCamera일 때 Camera View Space 방향을 Screen 2D 방향으로 변환하고 Safe Region ray intersection으로 2-Corner Open Edge Bracket의 위치·회전을 계산한다. Candidate offscreen hide와 기존 TargetSelect 선택 상태·TargetPanel 의미 정보 소유권은 유지한다.
- `UCFUISubsystem v1.9.0`은 Config의 `DA_CFHUDVisual_Default`를 해석해 `RadarSelectedEdgeBracket`, Style `AccentTactical`, `SafeMargin`을 TargetSelect에 주입한다. Widget은 콘텐츠 경로를 직접 Load하지 않으며 `Image_SelectedEdgeMarker`는 persisted WBP를 재구축하지 않는 runtime transient Image다. 기존 on-screen World Marker 표현은 이번 slice에서 변경하지 않았다.
- Official Build `4eecb33895574b87b4726a544f029835` PASS. `CarFight.UI.UI_P0_08.ScreenEdgeTargetMarkerContract` `ed5b04ade60c4fc086576610edfbfbca` 1/1 PASS, 기존 `CarFight.UI.UI_P0_05.TargetMarkerLayerContract` `9a0a1cd97c804321b36313ac0e085b1b` 1/1 regression PASS다.
- fresh post-build AssetDump `adset_v1_61f45e593a10851eb5c09ee20348ed7b.fcdea497420d1f9ecb4d8154`에서 `WBP_TargetSelect` persisted Designer가 Canvas Root + Candidate 3 + Selected 4의 기존 8 nodes를 그대로 유지하고 runtime `Image_SelectedEdgeMarker`가 저장되지 않았음을 확인했다. Parent C++ 변경으로 fingerprint는 달라질 수 있으므로 동일 fingerprint를 보존 조건으로 사용하지 않는다.
- UI-P0-08은 **Technical Complete / USER Visual·Zoom Feel Pending**이다. USER는 화면 이탈 방향, BehindCamera 전환, 재진입, Candidate hide, Zoom 방향·단계감, Radar/Edge 시각 품질을 실제 PIE에서 판정한다. `RadarSweepMaterial=None`은 Active Scan 시각 표현의 비차단 후속이다.

Migration: UI-P0-08A/B/Zoom/Screen-edge 완료 Build·Automation·persisted evidence는 새 실패 없이 반복하지 않는다. USER Visual/Feel 통과 후 UI-P0-09로 이동하며, 이번 slice를 이유로 기존 on-screen Target Marker 시각이나 TargetSelect Gameplay를 재구현하지 않는다.

### v0.59.29 - 2026-08-20

- 실제 Radar Zoom Input을 Technical PASS로 닫았다. `CFVehiclePawn.h v2.153.0 / .cpp v2.154.0`은 `/Game/CarFight/Input/IA_RadarZoom` Axis1D를 로드·바인딩하고 양수/음수만 현재 LocalPlayer `UCFUISubsystem v1.8.0`에 전달한다. UISubsystem은 기존 `UCFHUDDataProvider::RequestRadarZoomIn/Out()`에 위임하므로 Sensor detection range와 Scanner Profile은 변경되지 않는다.
- Official Build `18ddc2ba098d44f68721a108e3f43f8f` PASS, `RadarZoomInputAssetSetup` `5ecd196794d9453091c0ed27c03f0e60` 1/1 PASS로 `IA_RadarZoom + IMC_Vehicle_Default` exact 2 Asset을 생성·보정했다. fresh AssetDump에서 Axis1D/ConsumeInput, MouseScrollUp +1, MouseScrollDown Scalar -1, MouseWheelAxis0, Gamepad0, 전체 mapping 58을 persisted 확인했다.
- read-only `RadarZoomInputContract` `ef8336a7095943a0883da6e71b0ea34f`, `RadarRangeFoundationContract` `04bb436bc70644ab848fd9c799410f35`, 기존 `WeaponSelectInputContract` `60b018c9147e442e8df1d1d98f4485f3`가 각각 1/1 PASS다. 숫자키 Weapon Select와 Mouse Wheel Radar Zoom 소유권 충돌은 없다.
- 다음 technical slice는 Screen-Off Selected Target Edge Marker다. 현재 `UCFTargetSelectWidget::ProjectMarkerRoot`는 viewport 밖 또는 projection 실패에서 marker를 숨기므로 선택 상태는 유지되지만 방향 표현이 없다. UI-P0-08 계약대로 selected target만 camera View Space 기반 edge direction을 표시하고 Candidate는 기존 offscreen hide를 유지한다. 단순 raw screen coordinate clamp는 금지한다.
- Radar Zoom 조작감·Radar/Screen-edge Visual 품질은 USER PASS로 추정하지 않는다. `RadarSweepMaterial`은 Active Scan 시각 표현 후속으로 유지한다.

Migration: `RadarZoomInputAssetSetup`은 저장 mutation이므로 새 실패 없이 반복하지 않는다. UI-P0-08A/B/Zoom 완료 evidence를 보존하고 다음 slice는 TargetSelect Gameplay를 재구현하지 않은 spatial projection 확장만 수행한다.

### v0.59.28 - 2026-08-20

- `UI-P0-08B Production RadarPanel Consumer`를 Technical PASS로 닫았다. `CFHUDPresenter.cpp v1.21.0 / .h v1.20.0`은 `FCFRadarHUDData`만 소비해 actual Contact를 전용 `UImage` runtime pool로 표시하고 Display Range Text, Player Marker, in-range 4-Corner Selected와 range-out 2-Corner Edge를 적용한다. TextBlock 문자기호 아이콘과 World Actor/Sensor 재계산은 사용하지 않는다.
- `GenerateUIHUDArtP2.ps1 v1.2.0 -RadarOnly` process `a08892dd13a648cca7c1a92f8ab23527`에서 Radar Frame/Friendly/Hostile/Unknown/Player/TargetBracket/Edge 7종이 Technical PASS했다. Official Build `f7ac2bea527d4207ac0c45bd0a40d738`는 관련 Presenter/Bridge/Test를 UE 5.8에서 실제 컴파일·링크해 PASS했다.
- `ApplyUIHUDProduction.py v1.6.0 / RunUIHUDProduction.ps1 v1.5.0 -RadarOnly` process `a059a57d25114561934f824a7949186d`는 Radar Texture 7 + `DA_CFHUDVisual_Default` + 기존 `WBP_CFRadarPanel` 정확히 9개만 저장했다. Report는 RadarPanel Tree rebuild0, 기존 Widget Slot Layout 재적용0, additive semantic widget only를 확인했다.
- fresh AssetDump에서 `WBP_CFRadarPanel` 17 Widget과 `Image_RadarFrame / Image_RadarPlayer / Image_RadarSelectedEdge / Text_RadarRange`, 기존 Contact/Selected 슬롯 보존을 persisted 확인했고 VisualData의 Radar 7 Texture 참조도 저장됐다. `RadarSweepMaterial=None`은 Active Scan Sweep 후속 표현이라 B2 비차단이다.
- focused `CarFight.UI.UI_P0_08` process `9ba3a1d151d04b649d4da510bcab8b80`는 `RadarRangeFoundationContract`와 `RadarContactConsumerContract` 2/2 PASS·Failure0이다. UI-P0-08 전체 완료나 USER Visual PASS로 확대하지 않으며 현재 Gate는 실제 Radar Zoom Input 연결 + Production/USER Visual 검증이다.

Migration: UI-P0-08A/B Build·SourceArt·targeted apply·persisted readback·focused 2/2는 새 실패 근거 없이 반복하지 않는다. 다음 구현은 Mouse Wheel 입력을 기존 Provider-local Zoom API에 연결하고 게임패드 mapping은 별도 결정 전 임의 추가하지 않는다. 시각 품질·조작감은 USER가 판정한다.

### v0.59.27 - 2026-08-20

- `UI-P0-08A Radar Range Foundation`을 Technical PASS로 닫았다. Scanner Data가 오름차순 `RadarDisplayRangePresetsCm`과 명시 default index를 소유하고 Sensor Runtime은 이를 Applied copy로 고정하며, HUD Provider는 Sensor 탐지 성능을 바꾸지 않는 Provider-local Zoom In/Out과 Heading-Up `NormalizedPosition`을 제공한다.
- `FCFRadarHUDData`는 현재 Display Range, 실제 Maximum Detection Range, Preset index/count와 Zoom 가능 상태를, `FCFRadarContactHUDData`는 range-inside 여부와 선택 Contact의 range-out edge 방향을 명시적으로 제공한다. Range Profile이 없는 기존 scanner-less 경로는 `NormalizedPositionAvailability=Unavailable`을 유지한다.
- 첫 Official Build `fa1d22b52673480486d1032984192431`은 UE 5.8 UHT가 `TArray<float>`의 `Units="cm"` metadata를 거부해 Exit 6으로 실패했고, 배열 metadata의 Units만 제거하고 ToolTip에 cm 의미를 유지했다. 최종 Official Build `07b963ac33614ab2ad43193c8cd950b5`는 `Result: Succeeded`로 PASS했다.
- focused `CarFight.UI.UI_P0_08.RadarRangeFoundationContract` process `e0476b15b86843ef91640752f47cf064`는 1/1 PASS·Engine Exit 0, 기존 `CarFight.Sensor.SEN_P0_06.HUDSnapshot` regression process `9b97d06df6544cab91fe6cb094ed3cb8`도 1/1 PASS·Engine Exit 0이다. Content Asset mutation과 Sensor detection mutation은 0이다.
- UI-P0-08 전체 완료로 확대하지 않는다. 현재 Gate는 `UI-P0-08B Production RadarPanel Consumer Audit`이며 persisted `WBP_CFRadarPanel`, Presenter와 기존 offscreen projection 기반을 감사해 dynamic Contact·Display Range·selected edge presentation 최소 범위를 확정한다. 실제 Mouse Wheel Zoom 입력은 consumer audit 뒤 별도 연결한다.

Migration: UI-P0-08A Build/focused/Sensor HUD regression은 관련 실패 근거 없이 반복하지 않는다. Radar Widget에서 World Actor 검색, LOS·Detection·Knowledge 재계산 또는 Sensor 사거리 fallback을 만들지 않고 `FCFRadarHUDData`만 소비하는 Production consumer로 진행한다.

### v0.59.26 - 2026-08-20

- UI-P0-07 Target Knowledge를 Technical Complete로 닫았다. `CFHUDPresenter.cpp v1.19.0 / .h v1.18.0`은 actual `FCFTargetHUDData`만 소비해 `NO TARGET`, 미식별 `???`, Identified DisplayName, Snapshot Distance, AnalysisProgress를 Production TargetPanel에 표시하고 Sensor Contact Unknown에서는 stale 거리/Scan을 숨긴다. Target Armor는 authoritative source가 없어 계속 숨긴다.
- Official Build `72893293dd0f4dd98fbc93d96f3466dd`가 `CFHUDPresenter.cpp`와 `CFHUDDataTests.cpp`를 실제 컴파일해 `Result: Succeeded / Total execution time 314.06s`로 PASS했다. focused `CarFight.UI.UI_P0_07.TargetKnowledgePanelContract` process `e82f8ef659d34900a46ee32e8f98e94f`는 Engine Exit 0 / 1 Success / 0 Failure다.
- 선택 Target 파괴·해제는 기존 `UCFTargetSelectComp`의 Actor/VehicleHealth lifetime에서 `Destroyed` clear를 수행하고 `UCFHUDDataProvider::HandleSelectedTargetCleared`가 즉시 ViewData를 refresh한다. Sensor DestroyedHold는 이 선택 clear와 독립이므로 UI-P0-07에서 Gameplay lifecycle을 재구현하지 않는다.
- 현재 Runtime Gate를 `UI-P0-08 Radar·화면 밖 마커 Foundation Audit`으로 전환했다. 초기 actual은 actor-free Sensor Snapshot→Radar ViewData Contact/RelativePosition/Distance/Selected가 이미 구현돼 있고, 명시 Radar Range/Zoom 부재 때문에 `NormalizedPosition`은 의도적으로 Unavailable이며 Production RadarPanel dynamic Contact consumer는 아직 없다.
- D1-11-ART의 SpeedGauge·VehiclePanel·전체 Visual Review는 계속 Deferred이며 이번 Technical Complete를 USER Visual PASS로 확대하지 않는다.

Migration: UI-P0-07 Build/Automation과 TargetSelect lifecycle evidence는 관련 실패 없이 반복하지 않는다. UI-P0-08은 Sensor 탐지를 다시 만들지 않고 persisted RadarPanel, 표시 범위/정규화 ownership, dynamic Contact와 화면 밖 방향 consumer만 최소 구현한다.

### v0.59.25 - 2026-08-20

- USER가 `WBP_CFArmorBodyMap`의 위치·크기 Designer 편집성을 실제 확인해 PASS했다. 이후 `WBP_CFSpeedGauge`·`WBP_CFVehiclePanel` 개별 편집성 및 전체 VehiclePanel Visual Review는 건너뛰고 Runtime 작업으로 진행하기로 결정했으므로 **Deferred / USER PASS 미부여**로 보존한다.
- UI-P0-07 Target Knowledge 착수 감사에서 상류 Runtime이 이미 구현돼 있음을 확인했다. `TargetSelect`가 선택/Track 수명을, `Sensor Snapshot`이 `InformationLevel/KnownDisplayName/Distance/AnalysisProgress`를, `UCFHUDDataProvider`가 `FCFTargetHUDData` 합성을 이미 소유한다. 새 Gameplay/Provider 계약은 만들지 않는다.
- `CFHUDPresenter.cpp v1.19.0 / .h v1.18.0`에서 Production TargetPanel이 실제 Sensor Contact가 Known일 때만 Snapshot 기반 Distance와 AnalysisProgress를 표시하고 Identified 이상 공개 DisplayName을 사용하도록 연결했다. Target Armor는 authoritative source가 없어 계속 숨긴다. `CFHUDDataTests.cpp v1.23.0`에 저장 Production HUD 기반 `CarFight.UI.UI_P0_07.TargetKnowledgePanelContract`를 추가했다.
- 현재 Editor는 USER가 Designer를 직접 조작한 lifetime이므로 자동 discard/force stop하지 않는다. UI-P0-07 Source는 Implemented지만 Official Build와 focused Automation은 Editor가 안전하게 종료된 뒤 Pending이다.

Migration: D1-11-ART 남은 Visual Review를 PASS로 재해석하지 않는다. 다음 재개는 USER가 현재 Editor를 정상 종료한 뒤 UI-P0-07 Official Build → `TargetKnowledgePanelContract` focused Automation 순서로 진행하며 TargetPanel Asset rebuild/save는 하지 않는다.

### v0.59.24 - 2026-08-20

- 새 세션 인계 상태를 고정했다. Designer Layout Ownership Remediation의 Build/Readback/ArmorMap save0 Technical PASS는 반복하지 않고 다음 작업을 USER Designer Layout/Visual Review로 유지한다.
- canonical `carfight.editor.start` process `85cbeb26ecbd435594e6a0df508e6aa6`로 fresh Editor를 시작했고 Editor Ready, local 8100 Editor ownership, protocol ready를 확인했다. USER가 이 lifetime에서 Designer 편집을 시작하면 이후 자동 discard/force stop 대상이 아니다.
- 다음 USER 검증 순서는 `WBP_CFArmorBodyMap → WBP_CFSpeedGauge → WBP_CFVehiclePanel`이다. 위치·크기 조절이 막히는 Widget/Slot이 있으면 그 지점만 최소 교정하고, 모두 편집 가능하면 전체 Balanced Combat VehiclePanel Visual Review로 진행한다. UI-P0-07은 Ready/Not Started를 유지한다.

Migration: 새 세션은 completed build/readback/ArmorMap targeted validation, RPM/Armor technical apply, CarFight.UI 34/34를 반복하지 않는다. 현재 Editor와 persisted Designer Layout을 보존해 USER Gate부터 이어간다.

### v0.59.23 - 2026-08-20

- `D1-11-ART Designer Layout Ownership Remediation`을 Technical PASS로 닫았다. `CFUIHUDProdEditorBridge.h v1.9.0 / .cpp v1.12.0`에서 기존 RootWidget이 있는 Production Widget/Root의 Build를 fail-closed하고 신규/빈 Asset만 최초 Scaffold하도록 제한했다. 저장된 UMG의 Position·Size·Anchor·Alignment·Padding·AutoSize는 Designer Asset이 소유하며 Validator는 이름·타입·의미 구조를 보호한다.
- `ApplyUIHUDProduction.py v1.5.0 / RunUIHUDProduction.ps1 v1.4.0`은 기존 Widget을 Build→Compile→Save하지 않고 Validate-only로 처리한다. Official Build `9125557b4e214a4ebe2bd642099929aa` PASS, 새 바이너리 saved readback `b543596ea3404a8084b46e5349ce7e84` PASS / saved0을 확보했다.
- `ArmorMapOnly` process `ad6e8d64733249b0be26d8ab9506acca`는 existing ArmorSector/ArmorBodyMap을 `layout_preserved_assets` 2개로 보고하면서 rebuilt0 / compiled0 / saved0 / exact mutated0으로 PASS했다. 따라서 USER가 저장한 ArmorBodyMap 배치를 targeted apply 경로가 덮어쓰지 않는 것을 실제 실행으로 검증했다.
- 다음 Gate는 SpeedGauge / ArmorBodyMap / VehiclePanel의 USER Designer Layout 편집성과 전체 VehiclePanel Visual Review다. USER Visual PASS는 아직 올리지 않으며 UI-P0-07은 Ready/Not Started를 유지한다.

Migration: 완료된 RPM/Armor technical apply와 CarFight.UI 34/34는 반복하지 않는다. 신규 Production Widget 생성에만 Scaffold 기본값을 사용하고 기존 Widget에는 full/targeted apply로 Designer Layout을 재적용하지 않는다.

### v0.59.22 - 2026-08-20

- VehiclePanel USER Visual 중 구조적 UX 결함을 확인했다. `WBP_CFArmorSector` 재사용 구조를 도입해 ArmorBodyMap의 방향별 Image/Label/ProgressBar 직접 조립을 제거했고, 구형 SpeedGauge 21개 `ProgressBar_RPMTick*`도 제거해 `Image_RPMGauge + M_UI_RPMGauge + T_UI_RPMTrack + RPMRatio` 단일 동적 Material 구조로 전환했다.
- RPM SourceArt는 `R=Progress / G=Intensity / B=Redline / A=Coverage` 데이터 Texture로 전환했고 Material compile failure의 RCA가 UE 5.8 `MaterialExpressionIf` 핀 이름 불일치임을 확인했다. `ApplyUIHUDProduction.py v1.4.1`에서 실제 `A > B / A == B / A < B` 핀과 fail-closed connection 검사로 교정했다. Official Build `87be4ca3923e4a1583f74d14462d39a6` PASS, final targeted apply `4775e7f4439f4f72a0e5dc38671fe14d` PASS, RPM focused Automation `54330f1edb7e4a2c9679700fe5c01224` 3/3 PASS다. USER Designer에서도 Track/Tick 표시를 실제 확인했다.
- ArmorBodyMap은 USER가 Sector 배치를 직접 조정하면서 현재 Bridge가 `AddCanvasChild(... SetPosition/SetSize/AutoSize=false)`, ArmorSector의 `68×58 / 58×58 / 8×58` SizeBox override, VehiclePanel의 Speed/Armor SizeBox `238×190 / 382×190`처럼 레이아웃 값을 C++에서 과도하게 고정해 Designer 편집성을 훼손하는 문제를 발견했다.
- 다음 slice를 `Designer Layout Ownership Remediation`으로 고정한다. C++/Validator는 Widget 존재·이름·타입·데이터 의미만 보호하고 위치·크기·Anchor·Alignment·Padding·AutoSize는 persisted UMG Designer Asset이 소유한다. 기존 Asset은 자동 rebuild에서 레이아웃을 보존하고, 신규 Asset에만 기본 Scaffold가 필요할 때 생성한다. USER가 현재 조정 중인 ArmorBodyMap 레이아웃을 하드코딩 좌표로 역복사하거나 자동 재생성하지 않는다.

Migration: 새 세션은 완료된 RPM/Armor technical apply와 focused validation을 반복하지 않는다. 먼저 현재 user-edited `WBP_CFArmorBodyMap`을 보호한 채 Bridge/targeted apply의 destructive rebuild 경로와 SizeBox/CanvasSlot hardcoding을 감사하고 Designer Layout Ownership을 교정한다. USER Visual PASS 전 UI-P0-07을 열지 않는다.

### v0.59.21 - 2026-08-19

- VehiclePanel P2 Technical Hardening을 완료했다. `CFHUDArtP2Cmdlet v1.1.0`이 P2 Texture 9종에 UI Group, UserInterface2D(RGBA), NoMipmaps, sRGB를 exact 강제·검증하고 `CFUIHUDProdEditorBridge v1.9.1`은 P2 Frame source boundary와 일치하는 9-Slice Margin `0.09375`를 사용한다. `GenerateUIHUDArtP2.ps1 v1.0.1`은 Source-only review와 Unreal Apply 상태를 분리한다.
- P2 SourceArt 9/9 PASS, Official Build `c810db5d48af4039b1a500b2629a475c` PASS, corrected P2 reapply `fece00c20b08429988f7489e528949de` Exit 0 PASS와 fresh persisted AssetDump를 확보했다. 기존 VehiclePanel/Armor 위치·Runtime 의미는 변경하지 않았다.
- CF-FQ-032 전체 UI Source 중간점검에서 UISubsystem lifetime, Provider gameplay ownership·rebind/unbind, Presenter ViewData-only consumption, Sensor Snapshot Target/Radar source, Pawn legacy compatibility UI methods, Weapon player-facing identity 경계를 감사했고 새 기능/ownership defect를 발견하지 않았다. `CarFight.UI` fresh regression `dfbf29a014fe4c48a3bc64bc606ef116`은 34/34 PASS·0 FAIL이다.
- UI 소스의 누적 indentation/stale-comment 문제는 non-blocking hygiene debt로 분류했으며 기능 변화 없는 대규모 mass-format은 이번 감사에서 수행하지 않았다. Target Knowledge 최종 Presentation과 Radar dynamic Contact/Range/Zoom은 각각 UI-P0-07/08 범위로 계속 미착수다.
- USER Visual Review는 Pending이다. 이 Technical audit 결과를 USER Visual PASS로 확대하지 않고, 다음 사용자 가능 시 실제 VehiclePanel 화면만 판정한다.

Migration: P2 Hardening과 34/34 UI regression은 새 실패 근거 없이 반복하지 않는다. USER Visual PASS 전 UI-P0-07을 자동 착수하지 않으며 formatting debt는 별도 필요성이 있을 때만 독립 정리한다.

### v0.59.20 - 2026-08-19

- VehiclePanel Visual Foundation P2 Technical Apply를 완료했다. SourceArt/UI/HUD/P2 9종을 `/Game/CarFight/UI/HUD/Visual` Texture2D 9종으로 import하고 `DA_CFHUDVisual_Default`의 `VehiclePanelFrame`, `VehicleSilhouette`, Armor 6방향, `SpeedArcTrack`을 연결했다. Radar/Target/Weapon visual slot은 변경하지 않았다.
- `WBP_CFSpeedGauge`는 `Image_RPMTrackArt + RPM Tick 21 + Speed/Unit/Gear`의 26-node 구조, `WBP_CFArmorBodyMap`은 중앙 좌향 Vehicle + 6방향 Image/Label/세로 Bar의 20-node 구조, `WBP_CFVehiclePanel`은 Surface + SpeedGauge/ArmorBodyMap + Shield/Integrity의 19-node 구조로 persisted 저장됐고 fresh AssetDump에서 확인했다.
- Editor-only `CFHUDArtP2Cmdlet v1.0.0`을 추가했고 Official Build Job `74166fb3ad0442a5b10bbf02c6f69d12`는 `Result: Succeeded`다. 첫 `RunHUDArtP2.ps1 v1.0.0` wrapper는 PowerShell 5.1의 GUI process `$LASTEXITCODE` 미설정 때문에 wrapper Exit1이었지만 마지막 저장 대상 VehiclePanel까지 persisted 상태가 확인돼 실제 commandlet Apply 완료를 판정했다. commandlet은 재실행하지 않았고 wrapper만 `Start-Process -Wait -PassThru` 기반 v1.0.1로 교정했으며 post-fix 실행은 Not Run이다.
- USER가 PC를 사용할 수 없는 상태라 실제 시각 품질은 `USER Visual Pending`으로 보존한다. 다음 Gate는 `WBP_CFVehiclePanel` Designer 또는 1920×1080 Production HUD에서 Frame·RPM Track·Armor 가독성과 Balanced Combat 전체 인상을 사용자 확인하는 것이다. UI-P0-07은 Ready/Not Started를 유지한다.

Migration: P2 import·VisualData 연결·세 Widget persisted 구조와 build evidence를 새 실패 근거 없이 반복하지 않는다. 다음 PC 가능 시 USER Visual Review만 수행하고, 결함이 있으면 P2 art/배치만 최소 교정한다. USER Visual 승인 전 UI-P0-07 Runtime을 자동 착수하지 않는다.

### v0.59.19 - 2026-08-19

- Visual Foundation의 분위기 방향을 사용자 승인 `Balanced Combat`으로 고정했다. Cyan 기본 시스템 Accent, Amber Armor/Caution Accent, dark layered frame, restrained glow를 기본 시각 언어로 사용한다.
- 직전 분위기 시안의 원형 Speed/RPM 계기판과 임의 정보 배치는 채택하지 않으며 `InGameUIVehiclePanelSpec v0.20.0`의 896×416 구조와 승인 SpeedGauge/ArmorBodyMap/Shield/Integrity 계약을 절대 우선한다.
- 기존 P1 9종은 9/9 Technical PASS reference baseline으로만 보존하고 USER Production Art 승인으로 확대하지 않는다. 다음 체크포인트는 Actual VehiclePanel Concept USER Review다.
- Actual Concept 승인 전 Unreal Import·DA 연결·Production Apply는 금지하며 UI-P0-07 Target Knowledge는 Ready/Not Started로 보존한다.

### v0.59.18 - 2026-08-19

- 사용자 결정에 따라 formal next Runtime Gate `UI-P0-07 Target Knowledge`를 Ready/Not Started로 보존하고, 그 직전에 `D1-11-ART Visual Foundation / VehiclePanel Vertical Slice`를 현재 실행 Gate로 배치했다.
- 기존 `Tools/GenerateUIHUDArtP1.ps1 v1.0.1`의 프로젝트 내부 직접 PNG 생성 경로를 Art Ingress PASS로 재사용하고 새 파일 전달 파이프라인 구현은 요구하지 않는다.
- P1 9종 중 VehiclePanel이 이미 소비하는 VehicleSilhouette 1 + Armor Plate 6 + SpeedArcTrack 1의 8종만 첫 Vertical Slice 범위로 고정했다. Technical PASS와 USER Visual Approval을 분리하며 승인 전 Import·DA 연결·Production Apply는 금지한다.
- 승인 후 `Texture2D Import → DA_CFHUDVisual_Default → targeted Apply/Readback → persisted evidence → 1920×1080 USER Preview` 순서를 고정했다. `T_UI_TargetBracket`은 Target/Radar Visual 단계까지 Technical PASS Source로 보존할 수 있다.

### v0.59.17 - 2026-08-19

- UI-P0-06 중간점검에서 범위를 원래 종료 계약에 맞게 교정했다. 현재 Gameplay Runtime이 제공하는 차량/무기 채널을 Production Presenter에 연결하고, Provider 없는 채널은 추정 없이 `Unavailable/Collapsed`로 유지하면 UI-P0-06의 기술 범위는 충족된다.
- VehicleBattery는 현재 실제 Gameplay Runtime이 없으므로 UI가 숨기는 것이 올바르며, 이를 UI-P0-06 내부에서 새로 구현하지 않는다. Battery owner/capacity/regen/consumer priority와 Weapon·Shield·Scanner 소비는 향후 별도 shared-power Gameplay feature에서 설계하고 완성된 Runtime만 기존 `VehicleBattery` ResourceChannel에 연결한다.
- UI-P0-06을 current-runtime 기준 `Technical Complete`로 전환하고 formal runtime next Gate를 `UI-P0-07 Target Knowledge`로 이동했다. Rail USER Visual, Heat/Charge saved tuning USER Visual, authoritative Redline authoring/RPM USER Visual은 content-dependent Deferred follow-up이며 Technical Complete를 차단하지 않는다.
- `CFHUDDataProvider.h v1.7.0`, `CFHUDPresenter.h v1.15.0`의 stale 주석도 현재 WeaponCharge/Heat Runtime 존재와 VehicleBattery future dependency에 맞춰 교정했다. 실행 코드·Asset mutation은 0이다.

Migration: UI-P0-06 completed evidence는 관련 결함 없이 반복하지 않는다. 다음 runtime 작업은 UI-P0-07 Target Knowledge에서 시작한다. VehicleBattery는 UI 작업을 확장해 임의 구현하지 않고 별도 Gameplay power feature가 실제 착수될 때 설계한다.

### v0.59.16 - 2026-08-19

- UI-P0-06 WeaponCharge를 가장 독립적인 남은 Runtime slice로 구현·closure했다. VehicleBattery shared-power 계약은 열지 않고 WeaponCharge만 `WeaponData → WeaponComp → Pawn Fire → HUD` 경계에서 닫았다. explicit Maximum/Initial/PerShot/Recovery가 유효할 때만 활성이고 existing all-zero Asset은 Disabled 호환이다.
- 선택 무기별 Charge 독립 보존, 비선택 포함 Game-Time 회복, next-shot 부족 `WeaponChargeInsufficient`, accepted-shot 1회 소비를 연결했다. HUD는 actual `ResourceChannels::WeaponCharge`만 `CHARGE N% / NO CHARGE`로 표시하며 VehicleBattery/정적 설정/Cooldown fallback은 0이다. Compact Secondary 최대2와 `Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY`를 유지한다.
- final Official Build `2d9f33261d3c427187f75338b8f37f1d` PASS, exact `WeaponChargeRuntimeResourceContract` process `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS / Result SHA-256 `df53c9732b3135f9eae230dc9f287d646e542179c0464e5b2ef1794b54e8e137`다. closure readback에서 Pawn Charge precheck/post-consume이 기존 Heat 동적 검증 hook과 동일 fire 함수에 인접 연결된 것을 확인했으므로 제품 protected API 확대나 중복 friend dynamic test는 추가하지 않았다.
- saved WeaponData Charge authoring, VehicleBattery, Heat tuning, Redline authoring은 변경하지 않았다. Rail USER Visual Deferred와 artificial 2무기 fixture 금지도 유지한다. 현재 남은 실제 Gameplay Runtime은 VehicleBattery 하나다.

Migration: WeaponCharge Technical PASS는 관련 결함 없이 반복하지 않는다. 실제 Charge 무기 authoring 시 네 explicit 값을 함께 결정하고 현재 all-zero Production Asset을 임의 채우지 않는다. VehicleBattery는 별도 shared-power 설계 없이 Charge 값으로 대체하지 않는다. 다음 UI-P0-06 technical 후보를 고를 때 VehicleBattery의 broad dependency를 먼저 고려한다.

### v0.59.15 - 2026-08-19

- Player-facing Weapon Select Input을 Technical PASS로 갱신했다. `IA_SelectWeapon` Axis1D와 `IMC_Vehicle_Default` 숫자키 1~9 direct ordinal mapping을 사용하고 Pawn은 `Ordinal-1`만 기존 selection command에 전달한다. Mouse Wheel Radar 예약과 게임패드 미지정을 보존했다.
- final Build `dec0757b745342b4b90ff5f17b761eb0` PASS, `WeaponSelectInputAssetSetup` `189ac8704dea46c8bebf5b970d94a122` 1/1 PASS, persisted IA fp `1A8A0402`, IMC fp `52D2D868` / mapping47→56, exact `WeaponSelectInputContract` `daeaf649669d48c79746a65cc44b1b89` 1/1 PASS / Result SHA-256 `f61e41f8185ebe9053bad188c476277f0e0ddeb83aab048f0977f92bd18345dd`다.
- Rail USER Visual은 representative persisted multi-weapon content 부재로 Deferred했다. artificial fixture는 mass-valid test-only preset까지 확인했으나 finite-ammo loadout 추가가 필요해 범위 확대 전에 중단했고 실패 실행 생성 asset은 cleanup error0으로 삭제됐다. USER Visual PASS는 추정하지 않는다.

Migration: Weapon Select Input Technical PASS는 반복하지 않는다. Rail USER Visual은 실제 multi-weapon Vehicle/Fitting이 생길 때 재개하고 그 전에는 전용 fixture를 확장하지 않는다. 숫자1~9 ordinal과 Mouse Wheel Radar 예약을 유지한다.

### v0.59.14 - 2026-08-19

- `UI-P0-06` Production WeaponRail fresh persisted audit에서 saved fingerprint `4A873878`의 Rail이 Turret/Ammo/Reload semantic Image3 placeholder임을 확인했다. 실제 selection ViewData는 0-N list + selected index이므로 기존 이미지를 무기 슬롯으로 재해석하지 않았다.
- `UCFHUDPresenter v1.14.0/v1.15.0`, `CFUIHUDProdEditorBridge v1.5.0/v1.8.0`으로 current selected를 Rail에서 제외하고 비선택만 fixed order로 표시하는 truthful name-only Rail을 구현했다. 0 hidden, 1~3 ordinal+DisplayName, 4+ first2+`+N`; missing name은 `WEAPON`, internal ID/icon/resource fake fallback은 0이다.
- Official Build `941ea18597b3483594be8de3317e68fd` PASS, targeted WeaponPanel-only apply `0617f7cbed97427a942c7a6f59936e22` exact asset1 / other Production0, fresh AssetDump saved fingerprint `26CFB205`, exact `WeaponRailVisualContract` `7cc9ddd5b2204725bb3ec45c200d649b` 1/1 PASS / Result SHA-256 `83eb3d88d23f2c5baa06b3c326fbef30535048edd38d89b2e18768de2fa958c6`다.
- Weapon Selection Runtime/HUD source와 Heat/RPM/Stage A+B/기존 USER PASS는 반복하지 않았다. Battery/Charge Runtime, Heat tuning, Redline authoring은 변경하지 않았다. Rail USER Visual과 실제 Weapon Select Input Mapping은 Pending이다.

Migration: truthful Text Rail Technical PASS는 관련 결함 없이 반복하지 않는다. 정식 per-weapon icon 또는 비선택 resource summary를 추가하려면 먼저 실제 authoring/runtime/ViewData source를 별도 계약으로 만든 뒤 확장한다. 현재 name-only Rail에 내부 ID나 기존 Turret/Ammo/Reload semantic icon을 재사용하지 않는다.

### v0.59.13 - 2026-08-18

- Applied Fitting의 실제 weapon-bearing `ResolvedMounts` 고정 순서를 Player-facing Weapon Selection source로 확정하고 새 WeaponGroupId 없이 `SelectedWeaponIndex` 기반 선택 Runtime을 구현했다. 내부 MountProfileId는 Runtime identity only이며 HUD 노출은 0이다.
- 선택 무기별 Cooldown/Heat 독립 보존과 비선택 Heat 자연 냉각, Launcher `WeaponChanged` 정상 취소 후 WeaponComp+single Turret Visual 전환, DisplayName-only HUD 목록을 연결했다.
- final Official Build `62ffb62f69524bb18c3a9f11bb9f58f1` PASS, exact `WeaponSelectionRuntimeContract` `ff23cf6ac04e4b2c8cf0084b24173cdb` 1/1 PASS / Failure 0, Result SHA-256 `0ffa955f626d0de3f5ebf7b2ac594d7f8b039d268fe6efb2ab0730abfe77618c`다.
- Production WeaponRail은 기존 Turret/Ammo/Reload 의미 아이콘을 weapon slot으로 위장하지 않고 Collapsed 유지했다. Content/Blueprint/Input/DataAsset mutation 0, USER Visual 추가 0이다. Heat/RPM/Stage A+B/기존 USER PASS는 반복하지 않았다.

Migration: Weapon Selection Runtime/HUD source Technical PASS는 반복하지 않는다. 다음 WeaponPanel slice는 fresh persisted Rail 구조를 기준으로 truthful Visual Consumer를 구현하며 실제 icon source가 없다는 사실을 보존한다. Battery/Charge는 실제 Runtime 전 값이나 UI를 추정하지 않는다.

### v0.59.12 - 2026-08-18

- UI-P0-06 남은 WeaponGroup·VehicleBattery·WeaponCharge·Heat Runtime을 fresh 감사해 Heat를 가장 독립적인 다음 slice로 선정했다. WeaponGroup은 player-facing list/index 부재, VehicleBattery는 shared power Runtime 부재, WeaponCharge는 actual Runtime/static input 부재를 확인했다.
- `HeatDissipationPerSecond` 기본 0 + `FCFWeaponHeatRuntime`을 추가하고 WeaponComp per-weapon CurrentHeat/냉각/Overheated, Pawn `WeaponOverheated` 발사 차단과 승인 한 발당 Heat 1회 누적을 구현했다. 회복은 `max(MaxHeat - HeatPerShot, 0)` next-shot-headroom 기준이며 임의 percentage를 만들지 않는다.
- HUD는 실제 Heat Runtime만 Percent Resource Channel로 연결한다. Compact Heat Secondary와 `Reload > NoAmmo > Overheated > Cooldown/READY` FireState를 사용하며 Launcher Active는 Launcher Primary + Ammo/Heat Secondary 최대 2로 기존 Resource Projection/Launcher lifecycle을 보호한다.
- 첫 Build `a52dc87510e04b44a71bd14220aca82d` UHT unsupported Units metadata 1건을 교정한 뒤 final Official Build `0ecfed49ab3a4f41b349fc707c44e5f2` PASS, exact `HeatRuntimeResourceContract` `c736d1a6d6134a798a4b84452750e3d6` 1/1 PASS / Failure 0, Result SHA-256 `1abf0dc7a4788d6543c7933abef38433849ae57d569229cfab220f14937abad5`를 확보했다.
- existing saved WeaponData는 새 냉각값 기본 0으로 Heat Runtime Disabled이며 Content/Blueprint/DataAsset mutation은 0이다. Heat tuning authoring·USER Visual은 Pending이다. RPM/Stage A/B/UI-P0-03~05 완료 검증은 반복하지 않았고 UI-P0-06 전체 PASS로 확대하지 않는다.

Migration: Heat Runtime Technical PASS는 관련 결함 없이 반복하지 않는다. Heat를 실제 사용할 무기의 세 authored 값을 후속 tuning에서 함께 결정하고 USER Visual을 별도 확인한다. 남은 미구현 Runtime은 player-facing WeaponGroup, VehicleBattery, WeaponCharge이며 내부 ID/다른 설정에서 추정하지 않는다. `RedlineStartRPM=0`도 유지한다.

### v0.59.11 - 2026-08-18

- UI-P0-06 RPM Gauge Production Visual Binding을 Technical PASS로 닫았다. 저장 `WBP_CFSpeedGauge`의 기존 21 Tick 구조를 그대로 사용하고 `UCFHUDPresenter`가 explicit Redline/Maximum mapping 결과만 Runtime Percent에 반영한다. Redline 0/Unavailable/invalid에서는 모든 Tick fill을 0으로 reset해 Max 기반 fallback과 stale Pawn 표시를 차단한다.
- fresh persisted 감사에서 `DA_TestSedan` EngineIdleRPM 900 / EngineMaxRPM 6500, `DA_TestSUV`와 `DA_VehicleDefense_TestSUV` 900 / 6020을 확인했지만 authoritative `RedlineStartRPM` 설계값은 없었다. 따라서 세 Asset 모두 0 Unconfigured를 유지했고 Production/VehicleData Asset mutation은 0이다.
- 직전 Build `8c90ab8f8d7e430b9510bc12df5974ae`는 신규 test helper `TestName`의 C4458 이름 충돌 한 건만 FAIL했다. `TickPercentTestLabel`로 교정한 final Official Build `5ff6d404dfa44c61a85e698e27052db5`는 Exit 0 PASS했다.
- exact `CarFight.UI.UI_P0_06.RpmGaugeVisualBindingContract` process `7d32d08b7e554e9991fb64e80d472d78`는 1/1 PASS / Failure 0 / Engine Exit 0이며 Result JSON SHA-256은 `0054abd391216065f732e274b00bc054478c5ab168608523504b614c860fc6d4`다. Stage A/B와 UI-P0-03~05 USER PASS는 반복하지 않았다. UI-P0-06 전체 PASS나 새 USER Visual PASS로 확대하지 않는다.

Migration: RPM Visual Binding은 관련 결함이 없는 한 다시 열지 않는다. authoritative 차량 설계값이 생기기 전 `RedlineStartRPM=0`을 유지한다. RPM-specific 후속은 명시 Redline authoring 결정 → USER Visual이며, 별도로 WeaponGroup·VehicleBattery·WeaponCharge·Heat Runtime Pending을 계속 관리한다.

### v0.59.10 - 2026-08-18

- UI-P0-06 RPM Gauge explicit Redline upstream contract를 Technical PASS로 닫았다. `EngineMaxRPM`은 Chaos `EngineSetup.MaxRPM` 물리 상한으로 유지하고 `RedlineStartRPM`을 별도 authored field로 추가했다. 0은 미설정이며 EngineMaxRPM·Idle·변속 설정에서 fallback 추정하지 않는다. `ChangeUpRPM`은 Current CarFight Source에 없다.
- HUD source를 Current RPM=Chaos Runtime, Redline/Maximum=VehicleData authored source로 분리했고 Presenter는 실제 Redline→0.85, EngineMaxRPM→1.0 piecewise mapping을 구현했다. Production SpeedGauge Asset binding과 대표 VehicleData의 실제 Redline 값 authoring은 이번 slice에서 하지 않았다.
- Official Build `745dba430bcc47c2925c841a0c5a6686` PASS, `RpmGaugePresentationContract` `f805050484af47b8a2653cbb8ae2f639` 1/1 PASS, `RpmGaugeRuntimeSourceContract` `af1d9d010c2c433382819698a6dc7eeb` 1/1 PASS다. Prefix filter run은 Unreal 내부 2건 Success였으나 runner exact-path 집계 특성으로 wrapper만 Exit 1이어서 최종 evidence에서 제외하고 두 exact run을 사용한다.
- `RedlineStartRPM` additive leaf에 따라 Current DAUTH compatibility surface는 118 Registry / Performance numeric 18 / 5 Profile numeric 79다. P0-08 original 117/78 evidence는 Historical로 보존하며 bounded Registry/Batch compatibility 3건을 각 1/1 PASS했다. DAUTH USER Acceptance는 재개하지 않았다.
- Stage A 6/6, Stage B Dynamic Resource Visual, UI-P0-03~05 USER PASS는 반복하지 않았다. UI-P0-06 전체 PASS나 USER Visual PASS로 확대하지 않는다.

Migration: 다음 RPM slice는 이 upstream 계약을 다시 열지 않는다. 실제 대표 VehicleData의 Idle/Max와 차량 설계 의도를 확인한 명시 `RedlineStartRPM` authoring → Production SpeedGauge visual binding → USER Visual 순서로 진행한다. `EngineMaxRPM * 0.85` 자동값 생성은 금지한다.

### v0.59.9 - 2026-08-18

- UI-P0-06 Dynamic Resource Visual Stage B를 Technical PASS로 닫았다. 최종 Official Build `0bc2f086d71b4a57933326757d88b50d`, WeaponPanel-only Apply `b600bd339b0348b5a3001c01a541484a`, fresh AssetDump dataset `adset_v1_9ca90ab2e764ee02cda305b45e91bb3b.23e84f05ff5481cd492c956f`, 신규 `ResourceVisualSlotContract` `2dc5eb2a394b4053875f548cec2204bc` 1/1 PASS를 확보했다.
- 저장 `WBP_CFWeaponPanel`은 Header Reserve + Primary 1 + Secondary 최대 2 + FireState 1 의미 Container로 이전됐고 구형 Launcher/Ammo/Heat/Cooldown 전용 Row는 제거됐다. raw `ResourceChannels` 직접 0~N 렌더링 금지와 LauncherSequenceRevision 단일 소비 계약은 유지한다.
- 첫 Stage B 단일 Automation FAIL은 `Visible`과 Presenter 공통 `HitTestInvisible`의 테스트 expectation 불일치 5건뿐이었으며 `CFHUDDataTests.cpp v1.14.0`에서 테스트만 교정했다. Production 구현은 변경하지 않았다.
- Stage A focused 6/6과 UI-P0-03~05 USER PASS는 반복하지 않았다. `RedlineStartRPM`, Player-facing WeaponGroup, VehicleBattery, WeaponCharge, 현재 Heat Runtime은 계속 Pending이며 UI-P0-06 전체 PASS나 새 USER Visual PASS로 확대하지 않는다.

Migration: Dynamic Resource Visual Stage B는 관련 결함이 없는 한 다시 열지 않는다. 다음 UI-P0-06 slice는 남은 upstream Runtime/계약 중 실제 착수 가능한 항목을 선택하되 기존 Ammo/Reload/Cooldown/Launcher USER PASS와 LauncherSequenceRevision lifecycle을 보호한다.

### v0.59.8 - 2026-08-18

- UI-P0-06 Dynamic Resource Visual Stage B Source를 적용했다. Presenter는 Resource Visual에서 `BuildWeaponResourceEntries()`를 ViewData 적용당 정확히 1회 소비하고 ReserveAmmo Header owner를 분리 유지한다.
- Production WeaponPanel Bridge/Validator를 `Primary 1 + Secondary 최대 2 + FireState 1` 의미 슬롯 계약으로 전환하고 구형 Launcher/Ammo/Heat/Cooldown 전용 Row의 중복 잔존을 금지했다. Battery/Charge/Heat 가짜 Runtime/Row는 추가하지 않았다.
- 신규 `ResourceVisualSlotContract`를 추가해 실제 Production Widget의 새 슬롯, legacy Row 부재와 Launcher terminal 1회 lifecycle을 검증하도록 했다. Build `4f7a093c107d42a1a94672a0b92b2bb4`에서 Test/Stage B TU compile은 PASS했고 최종 DLL Link만 ownership-unknown Editor 점유로 `LNK1104`, Exit 6이다.
- Production Asset mutation은 0이다. 전체 10 Asset 재작성을 피하기 위해 기존 `WBP_CFWeaponPanel` 하나만 Build→Compile→Validate→Save하는 WeaponPanel-only targeted apply mode를 준비했다.
- Stage B Technical PASS, UI-P0-06 전체 PASS 또는 새 USER Visual PASS로 확대하지 않는다.

Migration: ownership-unknown Editor가 안전하게 종료된 뒤 final Official Build → WeaponPanel-only targeted Apply → fresh AssetDump persisted readback → Stage B `ResourceVisualSlotContract` 중심 회귀 순서로 재개한다. Stage A focused 6/6과 UI-P0-03~05 USER PASS는 관련 결함이 없는 한 반복하지 않는다. DAUTH Paused checkpoint는 `DataAuthoringPlan.md v0.2.26 / DataAuthoringRoadmap.md v0.1.34`를 우선한다.

### v0.59.7 - 2026-08-18

- UI-P0-06 Dynamic Resource Visual Stage A를 Presenter-only로 구현했다. raw `ResourceChannels` 0~N 직접 Visual 렌더링을 금지하고 `Primary 최대 1 + Secondary 최대 2 + FireState 최대 1` Compact Presentation Entry Projection을 추가했다.
- 기존 Ammo/Reserve/Weapon Status/Launcher Resolver와 `LauncherSequenceRevision` lifecycle을 재사용했으며 ReserveAmmo는 Header owner를 유지한다. 현재 Runtime 없는 Battery/Charge/Heat는 Entry를 생성하지 않는다.
- `CFHUDPresenter.h v1.9.0`, `CFHUDPresenter.cpp v1.10.0`, `CFHUDDataTests.cpp v1.12.0`을 적용했고 Production Asset/Editor Bridge 변경은 0이다.
- Official Build `4c043a73e3054baab5f725b18cd7de6c` PASS, focused `CarFight.UI.UI_P0_06` `73189888eb8042628db1b9499691c17c` 6/6 PASS / Failure 0, Result SHA-256 `39be662e7125ef4972574a75b433119283b7a279fbab56e511b140aafd6c70fe`를 확보했다.
- Stage A Technical PASS를 UI-P0-06 전체 PASS나 USER Visual PASS로 확대하지 않는다. 다음 UI-only 단계는 Stage B Production Visual Container 적용이다.

Migration: Stage B는 Presentation Entry를 한 ViewData 적용에서 정확히 한 번 소비해 기존 Launcher lifecycle이 중복 진행되지 않도록 한다. Stage A Build/6-test와 UI-P0-03~05 USER PASS는 관련 결함이 없는 한 반복하지 않는다.

### v0.58.9 - 2026-08-18

- UI-P0-05 TargetSelect Marker USER Visual을 사용자 PIE로 완료했다. 선택 Marker 정확히 1개, 내부 Actor 이름·거리·Track 중복 Text 0, 화면 밖 숨김+선택 유지, 화면 재진입 단일 Marker 복구, 우클릭 Clear와 가운데 버튼 Reselect 단일 Marker 복구를 PASS했다.
- Official Build `242dfee7186d46aaa98e3c24b25c0d09` PASS와 focused `CarFight.UI.UI_P0_05.TargetMarkerLayerContract` process `3834e606f2b54efb9ac5c09ea410b4f4` 1/1 PASS를 USER Visual과 결합해 UI-P0-05를 USER PASS로 닫았다.
- formal dependency를 재확인해 다음 Runtime Gate를 `UI-DESIGN-GATE 현재 비차단 상태 → UI-P0-06 차량·무기 HUD`로 이동했다. D1-11 Production Structure는 PASS이며 D1-11-ART Visual Polish·D1-12는 Non-Blocking Deferred다.
- UI-P0-07 Target Knowledge와 UI-P0-08 Radar는 UI-P0-06 뒤의 순서를 유지하며 `SensorContact.md v1.1.0` actor-free snapshot을 read-only source로 사용한다.
- DAUTH는 최신 Paused checkpoint `DataAuthoringPlan.md v0.2.20 / DataAuthoringRoadmap.md v0.1.28 / UA-01 USER PASS / USER PASS 1 / 다음 UA-02`를 보존한다.

Migration: 새 세션은 v0.58.9에서 UI-P0-03~05 완료 증거를 반복하지 않고 UI-P0-06 착수 감사부터 시작한다. Radar Range/Zoom·동적 Blip·TargetPanel/Radar USER Visual은 완료로 추정하지 않는다.

### v0.58.8 - 2026-08-18

- UI-P0-04 AimReticle UISubsystem 통합을 최종 Build·focused Automation·USER Visual PASS로 닫고 UI-P0-05 TargetSelect Marker Source를 적용했다.
- UI-P0-05는 `UCFUISubsystem v1.7.0` Game Layer 단일 소유, `CFVehiclePawn v2.149.0` direct 생성 제거, `UCFTargetSelectWidget v1.1.0` Weak Pawn·Marker-only·Projection-only Tick 계약과 Official Build/focused Automation PASS까지 진행했고 USER Visual을 Pending으로 유지했다.

### v0.58.7 - 2026-08-18

- `M_VehicleDefensePIE`에서 Defense Production Panel USER Visual을 완료했다. 정식 Defense SUV 피해 직후 Shield `0/100`, 방향 Armor 손상, Integrity `40/100`, 이후 Shield `100/100` 재생을 실제 Production HUD에서 확인했다.
- Baseline Pawn으로 Rebind했을 때 Shield 행 제거·Integrity `100/100` 전환을 확인했고, Old Defense SUV를 추가 피해로 파괴해도 Current Pawn/HUD가 변하지 않아 Old Pawn 이벤트 격리까지 USER PASS했다. 이에 UI-P0-03을 USER PASS로 닫았다.
- UI-P0-04 Source를 착수해 `UCFUISubsystem v1.6.0`에 기존 `WBP_AimReticle`의 HUD Layer 단일 소유, Current Pawn Rebind와 Cleanup 수명을 추가했다. `CFVehiclePawn v2.148.0`의 direct AimReticle CreateWidget/AddToViewport 자동 경로는 제거하고 Legacy Property/API는 호환 경계로 유지했다.
- `DefaultAimReticleWidgetClass` Config와 `CFUIFoundationTests v1.2.0`의 HUD Layer ZOrder 계약을 추가했다. 저장 Widget Asset과 Aim/FireFeedback Gameplay 의미는 변경하지 않았다.
- 현재 UI-P0-04 상태는 Source Applied / Official Build·Automation·USER Visual Pending이며 이를 통과하기 전 UI-P0-05로 넘어가지 않는다.

Migration: 새 세션은 이 문서 v0.58.7에서 UI-P0-03 USER PASS를 보존하고 UI-P0-04 AimReticle의 Official Build → focused Automation → 단일 Widget/Pawn Rebind USER Visual을 진행한다. UI-P0-05와 TargetPanel/Radar는 UI-P0-04 PASS 전 착수하지 않는다.

### v0.58.6 - 2026-08-18

- 사용자 우선순위 변경에 따라 `CF-FQ-032`를 프로젝트의 현재 단일 Active / UI 최우선 Plan으로 복원했다.
- 현재 Gate를 기존과 동일한 `UI-P0-03 Defense Production Panel USER Visual → Pawn Rebind USER Visual`로 고정하고 UI-P0-02/03의 완료된 기술·USER evidence를 반복하지 않도록 명시했다.
- 두 USER Visual Gate 완료 뒤 기존 순서대로 `UI-P0-04 AimReticle UISubsystem 통합 → UI-P0-05 TargetSelect Marker 통합`으로 진행한다. Gate를 건너뛰지 않는다.
- 완료된 `CF-FQ-037 Scanner`의 Current owner `SensorContact.md v1.1.0`을 후속 TargetPanel/Radar의 read-only Sensor source로 연결했다. Radar Range/Zoom·동적 Blip·TargetPanel/Radar USER Visual은 계속 미완료다.
- `CF-FQ-038 DAUTH`는 `P0-12 USER PASS 0 / UA-01` 상태로 Paused 보존하며 이 UI 우선순위 변경으로 DAUTH Technical PASS를 무효화하지 않는다.

Migration: 새 세션은 이 문서 v0.58.6을 읽고 UI-P0-03 남은 Defense/Pawn Rebind USER Visual부터 재개한다. 관련 결함이 없는 한 기존 Build/Automation/USER PASS를 반복하지 않으며 DAUTH는 사용자 명시 재개 전 Paused다.

### v0.58.5 - 2026-08-14


- `CFHUDDataTests.cpp`를 v1.6.0으로 보강했다. Production Gameplay/HUD Source는 변경하지 않고 실제 `VehicleDefenseComp` 피해·Shield 재생 → Provider Defense ViewData와 `OnCurrentPawnChanged` Rebind 뒤 Old Pawn Defense 이벤트 해제·Current Pawn 이벤트 반영을 회귀화했다.
- 저장 `/Game/Maps/M_VehicleDefensePIE`를 실제 PIE로 여는 `CarFight.UI.UI_P0_03.DefensePIEViewData`를 추가해 정식 Defense SUV의 Shield·Front Armor·Integrity Runtime이 Provider ViewData로 전달되는 기술 경로를 검증했다.
- 최종 공식 UE 5.8 Editor Build `681c91810da34066bb398ad1b0989f1e`은 Exit Code 0으로 PASS했다.
- 작업 전용 `Tools/RunUIAutomation.ps1 v1.0.3`으로 targeted `CarFight.UI.UI_P0_03` Automation `f6b08a970bbf4ec282893e86e92e72ac`를 실행해 5/5 Success·0 Fail을 확인했다. 결과 JSON SHA-256은 `3991406d636663ead0b2249846a44b7b86ba5d2122656fb6b3dc09339e0568d7`이다.
- 작업 전용 runner는 이번 검증의 Execution Method일 뿐 공용 저장소 Tool로 승격하지 않는다. 초기 runner 교정 실행은 테스트 실패가 아니라 결과 회수 경로 보정 이력으로만 취급한다.
- 사용자 직접 시각 확인이 없으므로 Defense/Pawn Rebind를 USER PASS로 승격하지 않는다. 현재 next gate는 `Defense Production Panel USER Visual → Pawn Rebind USER Visual`이며 둘 다 PASS하기 전 UI-P0-03과 CF-FQ-032 완료 처리를 금지한다.

### v0.58.4 - 2026-08-14

- 기존 `TestMap_DRSalvo`가 Ammo Runtime 도입 전 무한탄 Launcher 회귀 자산을 사용해 WeaponPanel finite Ammo 계약을 검증할 수 없음을 RCA로 확정했다.
- 기존 Salvo 발사 패턴과 검증된 finite Ammo 자산을 조합한 `/Game/Maps/TestMap_AmmoSalvo`, `DA_Wpn_SalvoFinite`, `Preset_SalvoFinite`, `DA_Fit_SalvoFinite` 격리 fixture를 추가했다. Production HUD/C++과 기존 `TestMap_DRSalvo`·`TestMap_AmmoRipple`은 변경하지 않았다.
- fresh AssetDump로 `Salvo 4발 / 동시 4 / SequenceCompleted Cooldown / Magazine 4 / InitialLoaded 4 / Sortie Ammo 7 / finite Ammo / Infinite Debug false`와 `Fitting → Preset → Weapon → Ammo` 연결을 검증했다.
- 사용자 PIE에서 Launcher Presentation 5개 항목을 전부 PASS했다: SALVO 표시, terminal `SALVO 4 / 4` 인지, Sequence 중 Cooldown·READY 비중복, terminal 이후 Cooldown→READY, 상단 AlertFeed 비중복.
- Launcher USER Gate를 닫고 UI-P0-03의 next gate를 `Defense 실제 Shield·Armor·Integrity 변화 USER PIE → Pawn Rebind USER PIE`로 이동했다. 이 둘이 PASS하기 전 UI-P0-03과 CF-FQ-032를 완료 처리하지 않는다.
- Source 변경이 없으므로 기존 Official Build `6978e029bfaa48c7addfc9acd87484b1`과 Full Automation `00ee23a01aa7480cabc557f312780ae8` 64/64 PASS를 보존하고 재실행하지 않았다.
- 같은 v0.58.4 정합성 교정으로 `남은 UI-P0-03 Production PIE Gate`와 `확인 완료한 사용자 PIE 범위`에 남아 있던 Launcher Pending 문구를 실제 USER 5/5 PASS 상태로 수정했다.

### v0.58.3 - 2026-08-14

- `InGameUIDesign.md v0.35.0`에 TargetPanel 상세 ViewData와 Radar/Sensor ViewData의 구현 준비 계약을 확정했다.
- Target의 추적 품질과 Contact 수명, Knowledge와 Freshness, Tactical Analysis Progress/Completion Revision, Defense/Weapon/Module Intelligence 구조를 분리했다.
- Radar의 ContactId/ResolvedTargetId, Live/LastKnown/DestroyedHold, 정규화 위치·고저차·Off-range Selected Marker, Scan State/Revision, Display Range/Maximum Detection Range 계약을 정의했다.
- Passive/Visual/Active Scan 탐지, Occlusion, Last Known 만료와 Tactical Analysis는 Gameplay Provider가 소유하고 Widget은 display-ready 데이터만 소비하도록 경계를 고정했다.
- UI-P0-07~08은 실제 구현 Not Started이며 Source·Config·Unreal Asset·Build·Automation을 변경하지 않았다. 현재 next gate는 계속 `TestMap_DRSalvo` USER PIE다.

### v0.58.2 - 2026-08-14

- UI-P0-05 TargetSelect HUD 통합을 Source·Config·Unreal Asset 수정 없이 정적 감사해 Pawn direct `AddToViewport(20)` 수명, `UCFTargetSelectWidget`의 직접 Component 구독·거리 계산·월드 투영·Text 조립과 `WBP_TargetSelect` 저장 구조를 확인했다.
- TargetSelect Gameplay 선택 상태는 `UCFTargetSelectComp`가 계속 단일 소유하고, 선택 Target 의미 정보는 기존 `FCFTargetHUDData → WBP_CFTargetPanel`, 후보·선택 월드 Marker는 `UCFUISubsystem → Game Layer → WBP_TargetSelect`가 소유하도록 역할을 분리했다.
- Legacy Widget의 `Actor->GetName()` fallback뿐 아니라 `ACFVehiclePawn::GetTargetDisplayInfo_Implementation()`의 `DisplayName=GetName()`도 내부 이름 노출 경로임을 확인했다. 실제 통합에서는 명시적인 공개 이름 Provider가 없으면 이름을 Unknown/`???`로 유지하고 내부 Actor 이름·TargetId를 Player-facing 이름으로 사용하지 않는다.
- Marker는 TargetSelect 이벤트로 최소 상태를 캐시하고 Tick에서는 약한 Actor/TargetPoint의 월드 위치 투영만 수행하는 방향으로 준비했다. Weak Pawn Binding, Old Delegate 해제, 같은 Actor 후보/선택 중복 억제, 화면 밖 선택 유지와 Occluded 형태를 보존한다.
- UI-P0-05는 계속 `Not Started`이며 Paused `CF-FQ-026 / TS-P0-08`을 재개하지 않았다. 현재 실행 Gate는 `TestMap_DRSalvo` → Defense 실제 변화 → Pawn Rebind로 변경하지 않았고 Build·Automation·commit·push도 수행하지 않았다.

### v0.58.1 - 2026-08-13

- UI-P0-03 Source·Asset을 변경하지 않는 정적 구조 감사를 수행해 Provider Old Pawn/Timer 해제, `OnCurrentPawnChanged` Rebind, Presenter `BindingGeneration` 초기화와 공통 Launcher lifecycle에 현재 Blocking 구조 결함이 없음을 확인했다.
- terminal Snapshot의 실제 화면 인지성은 정적 감사로 PASS 처리하지 않고 `TestMap_DRSalvo` USER PIE를 현재 Gate로 유지했다. Salvo 전용 초 단위 Hold는 다시 도입하지 않는다.
- UI-P0-04 사전 설계를 준비했다. 현재 Pawn direct `AddToViewport` Reticle과 UISubsystem 소유 Reticle의 병행 생성 금지, 기존 HUD Layer 재사용, Weak Pawn Binding, Current Pawn Rebind, World Cleanup 단일 수명, 기존 `WBP_AimReticle` Visual 재사용을 구현 경계로 고정했다.
- Fresh AssetDump에서 `WBP_AimReticle`의 Visual Tree와 C++ Parent를 확인했고 Blueprint Function Call·Variable Read·Variable Write Symbol이 0임을 확인했다. `BP_CFVehiclePawn`은 현재 `AimReticleWidgetClass=WBP_AimReticle`, `bShowAimReticle=true`, `AimReticleZOrder=10`을 보유하므로 실제 UI-P0-04에서 Config/소유권 이관을 원자적으로 수행한다.
- UI-P0-04 상태는 `Not Started` 그대로이며 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다. 현재 next gate는 계속 `TestMap_DRSalvo` USER PIE다.

### v0.58.0 - 2026-08-13

- 사용자 결정에 따라 Salvo 전용 Presentation Hold v1.5.0·v1.6.0 접근을 구현 경로에서 제거하고 HeavyCannon·Ripple·Salvo를 공통 Weapon/Launcher Presentation lifecycle로 교정했다.
- `FCFWeaponHUDData.LauncherSequenceRevision`과 `UCFHUDDataProvider v1.5.0` 이벤트 Revision을 추가해 실제 Launcher 상태 이벤트와 Ammo/10Hz 부수 Refresh를 분리했다.
- `UCFHUDPresenter v1.7.0`은 Ripple·Salvo 공통 Active→terminal Snapshot→Cooldown→READY 전이를 사용하고 FirePattern을 RIPPLE/SALVO 표시 문구에만 사용한다. HeavyCannon SingleCycle은 같은 공통 Weapon Status의 Cooldown→READY 경로를 사용한다.
- `CFHUDDataTests v1.4.0`으로 Ripple·Salvo 동일 lifecycle, terminal 전 부수 Refresh 보존, HeavyCannon SingleCycle 상태 경로를 회귀화했다.
- 공식 Build `6978e029bfaa48c7addfc9acd87484b1` PASS, 전체 Automation `00ee23a01aa7480cabc557f312780ae8` 64/64 PASS·필수 32/32 Success·실패 0. `TestMap_DRSalvo` USER PIE는 Ready/Pending이며 아직 UI-P0-03을 완료 처리하지 않는다.

### v0.57.0 - 2026-08-13

- 사용자가 Salvo를 HeavyCannon·Ripple과 다른 UI 예외 경로로 처리한 점을 지적했고, `MinimumSalvoPresentationHoldSeconds` 같은 Salvo 전용 Presentation Hold 접근을 승인하지 않았다.
- Salvo의 차이는 Gameplay 발사 타이밍뿐이며 WeaponPanel 상태 정책은 무기별 예외 없이 공통 계약을 따라야 한다고 확정했다.
- 따라서 `UCFHUDPresenter v1.5.0/v1.6.0`의 Salvo 전용 Hold는 현재 빌드·Automation이 PASS했더라도 **설계상 폐기 대상**으로 기록한다. v1.6.0 기준 USER PIE 재검증은 수행하지 않는다.
- 다음 세션의 첫 작업은 Salvo 전용 Hold 상태·함수·변수를 제거하고, SingleCycle·Ripple·Salvo가 공통 Weapon Action Presentation lifecycle을 사용하도록 구조를 교정하는 것이다. `FirePattern`은 `RIPPLE`/`SALVO` 문구와 진행 수치 결정에만 사용하고 Presentation 수명 정책 분기에 사용하지 않는다.
- 공통 Launcher Sequence가 한 Game Thread 처리 묶음에서 terminal까지 끝나더라도 UI가 마지막 유효 Sequence 상태를 잃지 않고, 이후 공통적으로 `Sequence → Cooldown → READY`로 전환하도록 설계·구현·Automation을 다시 잡는다.
- HeavyCannon의 SingleCycle은 기존 공통 `Cooldown → READY`, Ripple·Salvo는 공통 Launcher Sequence Presentation을 사용하며 향후 Missile/Burst/Charge 등 새 무기 추가 시 Presenter에 무기별 예외 분기를 누적하지 않는 방향을 고정한다.
- CF-FQ-031 Done / Ammo Current와 Heavy·Ripple Ammo USER PASS는 그대로 보호한다. 공통 Launcher Presentation 교정과 USER PASS 전에는 Defense·Pawn Rebind로 넘어가지 않는다.

### v0.56.2 - 2026-08-13

- Unreal Editor 종료 후 `UCFHUDPresenter v1.6.0` Salvo 완료 Hold 0.75초 변경을 공식 UE 5.8 Editor Build `056b96bd8f7f415fb77af511f5c2f5e1`로 검증했다. `CFHUDDataTests.cpp`, `CFHUDPresenter.cpp`, `UnrealEditor-CarFight_Re.dll`, Metadata까지 Result Succeeded / Exit 0이다.
- 전체 Combat Automation `486f2ebc89ab4af6a34912680460554b`은 Editor Exit 0, Required 32/32 Success, 전체 CarFight 64건 Failed 0, `passed=true`로 PASS했다.
- 따라서 현재 Gate는 `/Game/Maps/TestMap_DRSalvo` USER PIE item 2 `SALVO 4 / 4` 가시성 재검증 Ready다. 이전에 미판정으로 남긴 item 1·3~5는 item 2 PASS 전에는 다시 요구하지 않는다.
- item 2 재검증에서 `SALVO 4 / 4`가 약 0.75초 동안 인지 가능하게 보이는지만 우선 확인한다. PASS면 나머지 Salvo Presentation 항목을 이어서 판정하고, FAIL이면 시간 증가 반복보다 실제 Widget Visibility/Event 수명 증거를 추가 조사한다.
- CF-FQ-031 Done / Ammo Current와 기존 Heavy·Ripple USER PASS는 그대로 보호한다. Salvo 전체 USER PASS 전 Defense·Pawn Rebind로 넘어가지 않는다.

### v0.56.1 - 2026-08-13

- TestMap_DRSalvo 첫 USER PIE에서 사용자가 항목 2 `WeaponPanel SALVO 4 / 4 가시성`을 NP로 확인했다. 첨부 화면에서도 발사 후 WeaponPanel은 Cooldown `2.7 s` 상태이며 Salvo Sequence Row는 보이지 않았다. 항목 1·3~5는 이 시점에서 PASS로 추정하지 않고 미판정으로 유지한다.
- 코드 교차검증 결과 Launcher Completed Snapshot은 `ActiveConfig.FirePattern`, Total, Accepted를 유지하고 Provider가 terminal 상태에서도 이를 ViewData로 전달하므로 데이터 소실이나 Widget 슬롯 부재가 원인은 아니다. 기존 Ripple USER PASS로 같은 Sequence Row 저장 구조도 이미 확인돼 있다.
- v1.5.0의 Salvo 완료 Presentation Hold 0.25초가 실제 사용자 인지 기준으로 너무 짧은 것이 현재 최소 원인으로 판정됐다. Gameplay Salvo Scheduler·4발 동시 발사·Cooldown 시작 시점·Ammo·Ripple은 변경하지 않고 `UCFHUDPresenter v1.6.0`에서 Hold만 0.75초로 확대했다.
- `MinimumSalvoPresentationHoldSeconds >= 0.5` 컴파일 계약을 Presenter에 추가해 향후 USER 인지 이전 수준으로 짧아지는 회귀를 막는다.
- 현재 상태는 `Salvo USER PIE item 2 FAIL/NP / Hold v1.6.0 Source Applied / Build Pending / Automation Pending / USER recheck Pending`이다. Editor 종료 후 공식 UE 5.8 Build와 전체 Combat Automation PASS 전에는 재PIE를 요구하지 않는다.
- CF-FQ-031 Done / Ammo Current와 기존 Heavy·Ripple USER PASS는 그대로 보호한다. Salvo Gate PASS 전 Defense·Pawn Rebind로 넘어가지 않는다.

### v0.56.0 - 2026-08-13

- Unreal Editor 종료 후 공식 UE 5.8 Editor Build `758591f00154409884d613c7b4c43918`을 재실행해 `UnrealEditor-NetCore.dll`, `UnrealEditor-CarFight_Re.dll`, Metadata까지 Exit 0으로 PASS했다.
- 전체 Combat Automation `e67964daa052414088292efed2a245cc`은 Editor Exit 0, Required 32/32 Success, 전체 CarFight 64건 Failed 0, `passed=true`로 PASS했다. AMMO-P0-01~08, Damage, Fitting, Launcher, Projectile, Propulsion, Missile 필수 회귀를 모두 보호했다.
- Salvo Presentation Hold v1.5.0은 공식 Build·전체 Automation Gate를 통과했으며 `/Game/Maps/TestMap_DRSalvo` USER PIE 준비 완료 상태다.
- USER PIE에서는 4발 동시 Salvo 유지, `SALVO 4 / 4` 짧은 가시성, Sequence 표시 중 Cooldown/READY 중복 없음, `SALVO → 실제 Cooldown → READY`, READY 순간 깜빡임 없음, 정상 Sequence AlertFeed 비노출을 확인한다.
- Salvo USER PASS 전에는 Defense·Pawn Rebind로 넘어가지 않으며 UI-P0-03 또는 CF-FQ-032를 완료 처리하지 않는다. CF-FQ-031 Done / Ammo Current와 Ammo USER PASS는 반복하지 않는다.

### v0.55.1 - 2026-08-13

- 이전 Build Job `a74444db7f944bb49b477b03b28fe04e`의 영속 로그에서 Salvo Presentation 변경 관련 Compile은 PASS했으나 실행 중인 Unreal Editor가 Engine/Game DLL을 점유해 최종 링크가 `LNK1104`로 실패한 것을 확인했다.
- Build 상태를 `terminal Pending`에서 `Blocked — Editor DLL Lock`으로 교정했다. 코드 실패로 분류하지 않으며 Editor 종료 후 공식 Build 재실행 전 Automation·USER PIE는 진행하지 않는다.
- CF-FQ-031 Done / Ammo Current와 기존 Ammo USER PASS는 그대로 유지한다.

### v0.55.0 - 2026-08-13

- Launcher Presentation USER PIE 진입 전 실제 Scheduler를 재검토한 결과, 4발 Salvo는 첫 발 승인 뒤 같은 `StartFireSequenceAfterFirstAcceptedShot` 호출 안에서 나머지 3발 Dispatch와 terminal Broadcast까지 끝나 Slate 렌더 전에 `SALVO N / 4`가 Cooldown 상태로 덮일 수 있음을 확인했다.
- 이는 Gameplay Salvo 발사 타이밍 결함이 아니라 USER가 승인된 Sequence Presentation을 실제로 볼 수 없는 Presentation 수명 결함으로 분류했다.
- `UCFHUDPresenter v1.5.0`에 Salvo 전용 0.25초 최소 Presentation Hold를 추가했다. Active Salvo 또는 같은 처리 묶음에서 완료된 terminal Snapshot만 `SALVO N / Total`로 유지하며 Ripple과 실제 Launcher Scheduler·Projectile·Damage·Cooldown 기록 시점은 변경하지 않는다.
- Hold는 World Game-Time을 사용하고 WeaponId 및 `BindingGeneration`으로 경계를 제한해 Pause 중 시간이 진행되지 않으며 Pawn Rebind 또는 Widget 교체 시 이전 차량 표시가 새 HUD로 넘어가지 않도록 했다.
- `CFHUDDataTests v1.3.0`에 Active-only Resolver가 완료 Salvo를 직접 노출하지 않는 계약과 terminal Snapshot `SALVO 4 / 4`, Progress 1.0 변환 회귀를 추가했다.
- 변경 파일은 기존 미추적 Source라 worktree `git diff`가 비어 있으며 UTF-8 readback으로 `CFHUDPresenter.h/.cpp v1.5.0`, `CFHUDDataTests.cpp v1.3.0` 실제 반영을 확인했다.
- 공식 UE 5.8 Editor Build Job `a74444db7f944bb49b477b03b28fe04e`의 영속 로그를 다음 요청에서 회수했다. `CFHUDDataTests.cpp`, `CFHUDPresenter.cpp`를 포함한 관련 Compile 단계와 `UnrealEditor-CarFight_Re.lib` 생성은 성공했으나 실행 중인 `UnrealEditor.exe`가 `UnrealEditor-NetCore.dll`과 `UnrealEditor-CarFight_Re.dll`을 점유해 UBA 재시도와 NoUba 링크 모두 `LNK1104`로 실패했다. 이는 Source Compile 오류가 아니라 Editor DLL Lock이다.
- 따라서 현재 상태는 `Salvo Presentation Hold Source Applied / Compile PASS / Official Build Blocked — Editor DLL Lock / Automation Not Run / TestMap_DRSalvo USER PIE Not Started`다. Unreal Editor를 완전히 종료한 뒤 공식 Build를 새 Job으로 재실행하고 PASS일 때만 Automation으로 진행한다.
- CF-FQ-031 Done / Ammo Current와 Heavy·Ripple USER PASS는 그대로 보존하고 반복하지 않는다. UI-P0-03과 CF-FQ-032도 아직 완료 처리하지 않는다.

### v0.54.0 - 2026-08-13

- `CF-FQ-031`이 AMMO-P0-00~08, Heavy·Ripple USER PIE를 완료하고 `Document/Systems/Combat/Ammo.md v1.0.0` Current System으로 승격돼 현재 Active가 다시 CF-FQ-032로 복귀했다.
- UI-P0-03 Weapon ViewData는 더 이상 Ammo를 Unavailable로 취급하지 않고 `UCFVehicleAmmoComp`의 실제 finite Snapshot을 통해 `Loaded / MagazineCapacity + Reserve`, Reload와 NoAmmo를 소비한다고 현재 상태를 교정했다.
- TestMap_AmmoRipple에서 실제 3발 Partial Ripple와 WeaponPanel `RIPPLE N / 3`, Auto Reload, 두 번째 4발 Ripple, 정상 Sequence AlertFeed 비노출을 사용자 확인했다. 이는 Ammo·Ripple 실제 경로의 추가 USER 증거로 보존한다.
- 다만 CF-FQ-032의 Launcher Presentation 전체 Gate는 Salvo와 terminal Cooldown→READY까지 포함하므로 Ammo PIE만으로 UI-P0-03 전체를 완료 처리하지 않는다. 남은 Production Runtime 검증은 Launcher 전체 회귀, Defense 실제 변화와 Pawn Rebind다.
- 코드·Unreal Asset은 추가 수정하지 않았고 CF-FQ-031 완료 문서 승격만 반영했다.

### v0.53.0 - 2026-08-13

- `UCFLauncherComp v1.3.0`에서 마지막 Projectile 또는 Cancel로 Sequence가 Completed/Cancelled가 되는 시점의 상태 이벤트를 Cooldown 기록보다 먼저 보내던 전이를 교정했다. `SequenceCompleted` 정책 Cooldown을 `VehicleWeaponComp`에 기록하고 최종 Summary를 갱신한 뒤 terminal `OnLauncherSequenceChanged`를 한 번 Broadcast하므로 WeaponPanel은 `Sequence → READY 순간 노출 → Cooldown`이 아니라 `Sequence → Cooldown → READY`로 전이한다.
- Active 시작과 중간 발사 진행 이벤트는 즉시 유지하며 발사 수, Ripple 0.15초 간격, Muzzle 순서, Projectile 실행, Damage, FX와 Cooldown 공식은 변경하지 않았다.
- 최종 공식 UE 5.8 Build `39afeedd4d60476f91d7e1cb018dafe7`이 `CFLauncherComp.cpp`, 영향 Source Compile, 제품 `.lib`·`.dll` Link·Metadata를 Exit 0으로 PASS했다.
- 최종 Automation Process `2b25687c47f14ab581e8c1ebf0be5fbe`는 Editor Exit 0, 전체 CarFight 56건 Failed 0, 필수 Combat·Launcher 24/24 Success다. `WeaponLauncherSequencePresentation`, `ViewDataAvailability`, `ProviderPawnRebind` 모두 Success이며 Sequence Presentation은 Warning 0·Error 0이다.
- 직전 Production Apply `7609e56969634a059de7f5cf189a0233`, Saved Readback `0fe1109085e14b12ae8db35b098f936d`, Fresh AssetDump `adset_v1_2337145ce672db44e2a3c9ef85e9a8ea.e4d1e280558bb93047dabe04`의 WeaponPanel 26 Node와 Sequence 세 의미 슬롯은 그대로 유효하며 terminal 교정은 C++ 전이만 변경해 추가 Asset Apply가 필요하지 않다.
- 다음 Production PIE에서 WeaponPanel `RIPPLE/SALVO N/Total`, Progress, Sequence 중 Cooldown/READY 비노출, 완료 직후 READY 깜빡임 없이 Cooldown→READY, 상단 정상 Sequence Alert 없음의 USER PASS를 확인한다. 이후 Defense 실제 변화와 Pawn Rebind를 검증하며 그 전에는 UI-P0-03을 완료·Systems 승격하지 않는다.
- D1-11-ART, Art Import·DA 연결과 D1-12는 시작하지 않았고 기존 unrelated dirty 변경을 보호했으며 commit·push는 수행하지 않았다.

### v0.52.0 - 2026-08-13

- 사용자 PIE에서 정상 Ripple Sequence의 `RIPPLE 2 / 4`가 상단 AlertFeed에 실제 표시돼 Launcher Runtime→Provider→Presenter 전달 자체는 확인됐다. 그러나 정상 발사 진행은 WeaponPanel에서 표기해야 한다는 사용자 피드백을 `InGameUIWeaponPanelSpec.md v0.5.0`과 대조한 결과, 해당 명세가 이미 Launcher Sequence Active를 PrimaryChannel·FireState의 `RIPPLE N / Total` 진행 상태로 승인해 둔 것을 재확인했다.
- 따라서 새 디자인 변경이 아니라 구현 정합성 결함으로 분류했다. `UCFHUDDataProvider v1.2.0`은 정상 Ripple/Salvo의 `LauncherSequence` Alert 생성을 제거하고 Weapon ViewData만 유지한다. `UCFHUDPresenter v1.2.0`은 Sequence Active 동안 WeaponPanel 전용 Row/Progress를 표시하고 Cooldown/READY 중복을 숨긴 뒤 Sequence 완료 후 Cooldown→READY로 전환한다. AlertFeed에서는 정상 Sequence를 제거했다.
- `CFUIHUDProdEditorBridge v1.3.0`에 `HorizontalBox_LauncherSequence`, `Text_WeaponLauncherSequence`, `ProgressBar_LauncherSequence`와 Validator를 추가했다. `CFHUDDataTests v1.2.0`은 구형 `AlertSemanticRouting` 대신 `WeaponLauncherSequencePresentation`으로 Ripple 2/4=0.5, Salvo 3/4=0.75, Idle Collapse를 검증한다. Ammo/Heat Runtime 부재 시 채널 숨김과 가짜 탄약 금지는 유지한다.
- 공식 UE 5.8 Build `2cb895a63fa6425784dcc9c08d8b8491`가 UHT, 변경 HUD Source·Production Bridge·테스트 Compile과 제품 `.lib`·`.dll` Link를 Exit 0으로 PASS했다. Production Apply `7609e56969634a059de7f5cf189a0233`와 Saved Readback `0fe1109085e14b12ae8db35b098f936d`도 Exit 0, exact 10 Asset·Semantic Child Structure PASS다.
- Fresh AssetDump dataset `adset_v1_2337145ce672db44e2a3c9ef85e9a8ea.e4d1e280558bb93047dabe04`은 HUD WidgetBlueprint 9/9 Success, `WBP_CFWeaponPanel` 26 Node와 Launcher Sequence 세 의미 슬롯의 저장을 확인했다. 전체 Automation `19449259f9bd47feacad124c62078576`은 Editor Exit 0, 전체 CarFight 56건 Failed 0, 필수 Combat·Launcher 24/24 Success이며 새 `WeaponLauncherSequencePresentation`이 보고서에 포함됐다.
- 다음 Production PIE에서 WeaponPanel의 Sequence 진행, Sequence 중 Cooldown/READY 비노출, 완료 후 Cooldown→READY 전환과 상단 정상 Sequence Alert 제거를 사용자 확인한다. 이후 Defense 실제 변화와 Pawn Rebind를 검증한다. 전체 USER PASS 전 UI-P0-03 완료·Systems 승격하지 않는다.
- D1-11-ART, Art Import·DA 연결과 D1-12는 시작하지 않았고 기존 unrelated dirty 변경을 보호했으며 commit·push는 수행하지 않았다.

### v0.51.0 - 2026-08-13

- Production HUD 첫 UI-P0-03 사용자 PIE에서 Speed 실제 증가, Weapon Cooldown `READY → 남은 초 → READY`, Target 마우스 가운데 버튼 선택과 마우스 오른쪽 버튼 해제를 USER PASS로 확인했다.
- 같은 PIE에서 Ripple 발사 중 Alert Feed의 `RIPPLE`과 `현재 수 / 4`가 표시되지 않아 해당 항목을 FAIL로 유지했다.
- 조사 결과 Production `WBP_CFAlertFeed`는 Warning과 Ripple 전용 슬롯을 별도로 가지지만 Presenter가 Alert 배열 순번으로 슬롯을 선택했고, Launcher 시퀀스는 HUD에 직접 상태 이벤트를 제공하지 않아 10Hz Polling에만 의존하고 있었다.
- `UCFLauncherComp v1.2.0`에 읽기 전용 `OnLauncherSequenceChanged`, `UCFHUDDataProvider v1.1.0`에 Pawn Rebind 수명과 대칭인 Launcher 이벤트 구독/해제, `UCFHUDPresenter v1.1.0`에 `AlertKey=LauncherSequence` 의미 기반 Ripple 슬롯 라우팅, `CFHUDDataTests v1.1.0`에 `AlertSemanticRouting` 회귀를 추가했다. Gameplay 발사 수·간격·Damage·Cooldown 계산과 Production WBP/아트는 변경하지 않았다.
- 공식 UE 5.8 Build `50a86f1f73ed47429a037552d1e04c7b`가 UHT, 변경 Launcher/HUD Source와 테스트 Compile, 제품 `.lib`·`.dll` Link를 Exit 0으로 PASS했다.
- 전체 Automation Process `784f0da794e4443b882cdbbb4626b383`은 Editor Exit 0, 전체 CarFight 56건 Failed 0, 필수 Combat·Launcher 24/24 Success다. `AlertSemanticRouting`, `ViewDataAvailability`, `ProviderPawnRebind` 모두 Success이며 Rebind에는 기존 Transient Pawn `SM_Body` 부재 Warning 2건만 존재한다.
- UI-P0-03 상태를 `Production Runtime PIE Partial USER PASS / Launcher Alert Fix Build·Automation PASS / Alert 재PIE·Defense·Pawn Rebind Pending`으로 갱신했다. 전체 USER PASS 전 완료나 Systems 승격을 하지 않는다.
- 기존 Production HUD Asset, D1-11-ART 승인 상태와 unrelated dirty 변경은 보호했고 commit·push는 수행하지 않았다.

### v0.50.0 - 2026-08-13

- 사용자 에디터 종료 후 공식 Build `4bf57b5d92e24d629541f5fd11e2bf7b`에서 이전 LNK1104가 해소되고 제품 `UnrealEditor-CarFight_Re.dll` Link가 Exit 0으로 PASS했다.
- 첫 Automation `81b93d40b212473483f40e2c3c770f29`은 필수 Combat 24/24와 `ViewDataAvailability`는 Success였지만 `ProviderPawnRebind` 테스트가 `UCFUISubsystem ClassWithin=LocalPlayer`를 무시한 Fixture Outer 때문에 Ensure로 실패했다. 첫 교정 뒤에는 `ULocalPlayer ClassWithin=Engine` 경계가 드러나 두 번째 Automation `b5ff3c0803854dd6b545322ec343a309`도 동일하게 제품 외 Fixture 문제로 중단됐다.
- `CFHUDDataTests v1.0.2`에서 Automation Fixture만 `GEngine → ULocalPlayer → UCFUISubsystem` 실제 수명 체인으로 교정했다. `UCFHUDDataProvider`, `UCFHUDPresenter`, Gameplay Runtime과 Production Widget Source는 수정하지 않았다.
- 최종 공식 Build `14a95c08b2f34a13b9bd1d95f4fc6b2f`가 테스트 Compile과 제품 `.lib`·`.dll` Link·Metadata를 Exit 0으로 PASS했다.
- 최종 Automation `c43a855369ac406eac913eaa54806f33`은 Editor Exit 0, 전체 CarFight 55건 Failed 0(`Success 54 + SuccessWithWarnings 1`), 필수 Combat 24/24 Success다. `ViewDataAvailability`와 `ProviderPawnRebind` 모두 `state=Success`; Rebind의 Warning 2건은 Transient `CFVehiclePawn_0/1`에 `SM_Body`가 없다는 시각 충돌 Fixture 경고이며 Error는 0이다.
- UI-P0-03 상태를 `Source·Official Build·Automation PASS / Production Runtime PIE READY·Pending`으로 갱신했다. 사용자 요청에 따라 이번 세션에서는 실제 Production PIE를 시작하지 않았고 USER PASS 전에는 UI-P0-03 완료나 Systems 승격을 하지 않는다.
- 기존 Production HUD Asset, Gameplay 공용 파일, D1-11-ART 승인 상태와 unrelated dirty 변경은 보호했고 commit·push는 수행하지 않았다.

### v0.49.0 - 2026-08-13

- 사용자가 UI-P0-02 남은 Runtime Lifetime PIE 전 항목을 PASS했다고 확인해 Gamepad Pause·Back, 입력 잔류 제거, Ripple·Salvo 동일 Sequence 재개, Projectile·추진·World Timer 정지와 반복 Pause·레벨 재진입 수명을 포함해 `UI-P0-02 = USER PASS`로 닫았다.
- UI-P0-03을 실제 착수해 `CFHUDViewData v1.0.0`, `UCFHUDDataProvider v1.0.0`, `UCFHUDPresenter v1.0.0`, `CFHUDDataTests v1.0.0`을 추가하고 `UCFUISubsystem v1.5.0`과 `DefaultInGameHUDWidgetClass` Config를 연결했다.
- Runtime 흐름을 `Gameplay Runtime → UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter → 기존 WBP_CFInGameHUD`로 구현했다. Production Blueprint Parent와 D1-11 Asset Tree는 변경하지 않았다.
- Provider는 `OnCurrentPawnChanged`를 통해 Old Pawn의 Health/Defense/Target 이벤트와 10Hz World Timer를 정리한 뒤 New Pawn으로 Rebind한다. Widget/Presenter의 Pawn·Component Cast는 금지하며 정적 검색에서 `ACFVehiclePawn` Cast는 Provider Rebind 1곳에만 존재함을 확인했다.
- 실제 Runtime이 있는 Vehicle Speed, Shield·6방향 Armor·Integrity, Weapon Cooldown·Launcher Sequence, TargetSelect 공개 DisplayInfo만 사용한다. Engine RPM·Gear·Ammo·Heat·Radar Contact·Target Distance·Module Intelligence는 실제 Provider 부재 시 `Unavailable`이며 Radar 후보를 TargetSelect에서 조작해 만들지 않는다.
- 첫 공식 Build에서 UISubsystem include/Initialize 조립 중복을 발견해 최소 교정했다. 두 번째 Build `a5109b1f084145e2a557ba9b7c1620ca`는 신규 Source와 `CFUISubsystem.cpp` Compile, `UnrealEditor-CarFight_Re.lib` Link까지 PASS했으나 실행 중 `UnrealEditor.exe`가 제품 DLL을 점유해 최종 DLL Link가 LNK1104 / Exit 6으로 차단됐다. 코드 Compile 실패가 아니다.
- 신규 Automation `CarFight.UI.UI_P0_03.ViewDataAvailability`, `CarFight.UI.UI_P0_03.ProviderPawnRebind`는 Source Compile PASS이나 새 DLL Link 전 실행 Pending이다. UI-P0-03은 아직 완료가 아니며 Editor 정상 종료 → 공식 Link → 전체 Automation → Production Runtime PIE 순으로 검증한다.
- 기존 Gameplay 공용 파일, Production HUD Asset, Defense·Target·Launcher 자산, D1-11-ART 승인 상태와 unrelated dirty 변경은 보호했고 commit·push는 수행하지 않았다.

### v0.48.0 - 2026-08-13

- 사용자 결정으로 CF-FQ-032의 우선순위를 **비주얼 폴리시보다 계획 기능 구현·Runtime 연결**로 전환했다. D1-11 Production Structure Gate PASS는 유지하고 D1-11-ART-01, 세부 이미지 형태·배치·픽셀 조정은 기능 개발 비차단 후순위로 분류했다.
- 실제 저장소를 기능 관점에서 재점검했다. `ACFPlayerController`가 기본 `CFSingleGameMode` PlayerController로 연결되어 Pause·Back·Mapping Context·Possess 수명·입력 중립화·실제 World Pause를 구현하고, `UCFUISubsystem`이 LocalPlayer UI Root·8 Layer·Screen/Modal/Pause 수명·`OnCurrentPawnChanged`·Style/Density/Layout 해석을 소유함을 확인했다.
- 실제 C++ 검색에서 Vehicle/Weapon/Target/Radar/Alert ViewData Runtime 계층과 Production `WBP_CFInGameHUD` Gameplay Binding은 아직 존재하지 않음을 확인했으며, 이는 계획상 `UI-P0-03 = Not Started`와 일치한다. 현재 Designer 값은 실제 Runtime 증거로 취급하지 않는다.
- 전체 `CarFight` Automation Process `0a9f9944c01f483fbd63e0c0d4b570f5`를 재실행해 전체 53/53 Success·Failed 0, 필수 Combat 24/24 Success를 확인했다. UI 관련 `UI_P0_01A.ControllerContract`, `UI_P0_01B.RootLayerContract`, `UI_P0_02.InputNeutralContract`, `PauseMenuContract`, `ProgressFreezeContract`도 모두 Success다.
- 다음 실제 착수점을 `UI-P0-02`의 남은 Gamepad·입력 잔류·Launcher/Projectile/WorldTimer·반복 Pause·레벨 재진입 사용자 PIE로 유지한다. 이 범위가 USER PASS되면 즉시 `UI-P0-03` ViewData/Provider 연결 구조를 구현한다.
- UI-P0-03의 핵심 목표는 Gameplay Runtime이 Widget을 직접 조종하거나 Widget이 Pawn/Component를 직접 Cast하지 않고 `Gameplay Runtime → Provider/Presenter → ViewData → Production Widget`으로 연결되게 하는 것이다. Pawn 교체·Respawn·향후 Spectate에서 `OnCurrentPawnChanged` 기반 Rebind가 가능해야 한다.
- Source·Asset은 수정하지 않았고 Automation 결과만 갱신했다. Art Import·`DA_CFHUDVisual_Default` 연결, D1-12와 UI-P0-03 구현은 이번 세션에서 시작하지 않았다.

### v0.47.0 - 2026-08-13

- 에디터 종료 후 공식 UE 5.8 Build `cef0c064cd434848b1fcb94459fbccb5`가 제품 DLL Link까지 Exit 0으로 PASS했다.
- Production Apply `f1c32cab41b544d6b27709315029bb04`와 Saved Readback `3b10248701f94c9a9b490c1aaa0d5e13`가 Exit 0 PASS했다.
- 사용자 결정으로 조정한 Structure-first PASS 기준을 공식 보고서와 일치시키기 위해 `ApplyUIHUDProduction.py v1.1.0`으로 갱신했다. 기존 exact Asset/구조/금지 계약은 유지하고 `preview_1920x1080_user_gate=StructureSanityAccepted`, `d1_11_structure_pass=true`, `d1_11_pass=true`, `d1_11_art_user_approval=Pending`을 별도 기록한다.
- 최종 Saved Readback `c0bdf6a72df3412da78057406e843c0c`이 새 보고 계약으로 Exit 0 PASS했다.
- Fresh AssetDump dataset `adset_v1_aa5d0dfdce87eb6a015641240dbc3cdd.cf961b240264a26f8e54bd19`은 WidgetBlueprint 9/9 Success를 기록하고 `Image_RPMTrackArt visibility=Collapsed`, RPM Tick 21개·Gear N, Armor 6방향 세로 Bar, Root 여섯 의미 Panel + ReticleLayer를 재확인했다.
- Fifth Preview에서 이미 확인한 D1-07 외곽 배치·중앙 전투 시야 구조 sanity와 위 자동 증거를 합쳐 **D1-11 Production Structure Gate를 PASS**로 닫았다.
- 최종 HUD Art 형태·세부 배치·픽셀 폴리시는 D1-11-ART로 분리한다. 현재 D1-11-ART-01 Source PNG 9/9 Technical PASS는 유지하며 사용자 Visual Review가 다음 단계다.
- 사용자 승인 전 Approved Asset 0·Unreal Import 0·`DA_CFHUDVisual_Default` 연결 0을 유지하고 D1-12·UI-P0-03은 시작하지 않았다.
- 기존 미커밋 변경과 unrelated Asset을 보호했으며 commit·push는 수행하지 않았다.

### v0.46.0 - 2026-08-13

- 사용자 결정에 따라 D1-11을 최종 미적 완성도 Gate가 아니라 **Production 내부 구조·의미 계약 Gate**로 재정의했다. 이미지 형태, 세부 배치, 간격과 픽셀 폴리시는 D1-11-ART 및 후속 사용자 Editor 조정으로 분리한다.
- Fifth Preview의 D1-07 외곽 배치·중앙 전투 시야·Root/Panel/Element 의미 분리는 구조 sanity PASS로 유지하며, Placeholder 아트의 미적 완성도만으로 D1-11을 차단하지 않는다.
- `CFUIHUDProdEditorBridge v1.2.0`에서 `SpeedArcTrack=None` 또는 Texture Load 실패 시 `Image_RPMTrackArt`를 `Collapsed`로 저장하도록 수정했고, Brush Resource와 Visibility가 일치하지 않으면 Validator가 FAIL하도록 보강했다.
- 공식 Build Job `d7f30ed2ced04f0a8c4387b0d73c65f7`에서 UHT와 변경 C++ Compile은 PASS했다. 실행 중인 `UnrealEditor.exe`의 `UnrealEditor-CarFight_Re.dll` 잠금 때문에 Link가 LNK1104 / Exit 6으로 중단됐으며 코드 Compile 실패로 분류하지 않는다.
- UE에서 현재 열린 Asset은 `WBP_CFInGameHUD` 하나이고 `is_dirty=false`임을 확인했지만 unrelated Level/Package의 저장 상태를 추측하지 않고 Editor를 강제 종료하지 않았다.
- 공식 Build가 완전히 PASS한 뒤 Production Apply → Saved Readback → Fresh AssetDump까지 PASS해야 D1-11 Structure Gate를 닫는다. 현재 `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started다.
- D1-11-ART-01 Source 9종은 계속 승인 전 상태이며 사용자 승인 전 Import·`DA_CFHUDVisual_Default` 연결을 금지한다.

### v0.45.0 - 2026-08-13

- 사용자가 새 Production `WBP_CFInGameHUD` 1920×1080 Designer Preview를 제공해 다섯 번째 D1-11 시각 검토를 수행했다.
- 실제 화면에서 D1-07 외곽 배치, 중앙 전투 시야 보호와 Production 의미 단위 Panel 분리는 유지됐으나 VehiclePanel SpeedGauge의 큰 청록색 사각형 때문에 시각 승인에 실패했다.
- Fresh AssetDump에서 `DA_CFHUDVisual_Default.SpeedArcTrack=None`이고 `WBP_CFSpeedGauge.Image_RPMTrackArt`가 존재하는 상태를 대조해, 전용 Texture가 없는 Image가 숨겨지지 않은 채 AccentTactical Tint의 기본 Brush로 렌더링된 것이 가장 직접적인 원인임을 확인했다.
- VehicleSilhouette와 6방향 Armor Plate 전용 Art도 아직 None이라 화면에서 Semantic Fallback Placeholder가 노출된다. 21 RPM Tick·`076/km/h/N`·6방향 세로 Armor Bar 구조 자체는 저장 Tree와 화면에서 확인된다.
- 따라서 Fifth Production 1920×1080 Preview = FAIL, `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.
- 다음 D1-11 작업은 빈 `Image_RPMTrackArt`의 렌더링 안전 처리와 Validator 보강을 우선하고, D1-11-ART-01 사용자 승인→ART-02 연결 여부를 분리해 다시 Preview한다.
- 이번 판정에서는 Source·Config·Unreal Asset을 수정하지 않았고 commit·push는 수행하지 않았다.

### v0.44.0 - 2026-08-12

- D1-11 Pre-Preview Contract Review FAIL 세 항목을 `InGameUIVehiclePanelSpec.md v0.20.0`과 `InGameUIStyleSpec.md v0.86.0` 기준으로 최소 교정했다.
- `CFUIHUDProdEditorBridge v1.1.0`에서 SpeedGauge의 `D`·가짜 Speed/RPM ProgressBar·Cooldown Semantic Speed Arc 대체를 제거하고 `N` 단일 Gear Slot, 21 Tick, 85% Red Zone, 비대칭 RPM Scale과 실제 HUDVisualData 전용 Track 슬롯으로 변경했다.
- ArmorBodyMap은 6방향 Current/Max Text를 제거하고 Front/Right/Rear/Left/Top/Bottom Plate/Badge 우측에 각각 8×46 BottomToTop `ProgressBar_Armor*`를 추가했다.
- Bridge Validator에 Gear D/구형 Fallback/21 Tick/6 Armor Bar/Text_Armor 회귀 검출을 추가해 동일 Pre-Preview FAIL이 다시 저장되지 않도록 했다.
- 공식 UE 5.8 Build `9416a5877d8e47a5a80ac552d5e7e76d`가 UHT·`CFUIHUDProdEditorBridge.cpp` Compile·제품 DLL Link를 Exit 0으로 PASS했다.
- Production Apply `a3fe8f3981f542c483d607d7e88eca93`, Saved Readback `9972afbf11df44a7aac5d731e9010a3a`가 Exit 0 PASS했고 exact 10 Asset·Semantic/Image Structure PASS·Border Mosaic/Gameplay Cast/Runtime Binding/D1-12/UI-P0-03 0을 유지했다.
- Fresh AssetDump dataset `adset_v1_b17fbe9392114d7f155e6c74ad48f141.4ee866a52a57c78493666817`은 HUD WidgetBlueprint 9/9 Success와 실제 저장 SpeedGauge/ArmorBodyMap/Root Designer Tree를 확인했다.
- Pre-Preview FAIL은 Corrected로 닫고 새 Production 1920×1080 Designer/User Preview를 READY·Pending으로 전환했다. User PASS 전에는 `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started다.
- 기존 미커밋 변경과 unrelated 에셋은 보호했고 commit·push는 수행하지 않았다.

### v0.43.0 - 2026-08-12

- D1-11 새 Production 1920×1080 Designer/User Preview Gate를 재개해 저장된 Production HUD를 fresh AssetDump와 공식 Production Readback으로 교차검증했다.
- Root는 D1-07 승인 LayoutData 기반 7 Slot, 여섯 의미 Panel과 Full Stretch ReticleLayer를 유지하고 Border Mosaic·Gameplay Cast·Runtime Binding 없이 Production 의미 단위 구조를 유지했다.
- 공식 `RunUIHUDProduction.ps1 -Readback` Process `3d301f3b3d4c4066a626b6081e010e6e` / Exit 0이 exact 10 Asset, Root 1·Panel 6·Element 2·HUD Visual Data 1과 D1-12/UI-P0-03 0을 재확인했다.
- 최신 `InGameUIVehiclePanelSpec.md v0.20.0`과 저장 Designer Tree 대조에서 `D` 전진 Gear, 6방향 Armor 수치 Text와 세로 Armor Bar 누락, 가짜 Speed/RPM Fallback 표현을 확인해 **Pre-Preview Contract Review FAIL**로 판정했다.
- 따라서 D1-07 승인 배치·Production 구조·중앙 Reticle 보호영역은 Structural PASS를 유지하지만 새 1920×1080 User Preview는 D1-11 내부 교정 전 Blocked다. `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.
- 이번 Gate에서는 Source·Config·Unreal Asset을 수정하지 않았고 D1-11-ART-01/02 상태도 변경하지 않았다. 기존 미커밋 변경과 에셋을 보호했으며 commit·push는 수행하지 않았다.

### v0.42.0 - 2026-08-11

- D1-11-ART-01을 착수해 `Tools/GenerateUIHUDArtP1.ps1 v1.0.1`과 `SourceArt/UI/HUD/P1` 승인 전 Source Art 경로를 추가했다.
- `InGameUIHUDArtSpec.md v0.2.0` 기준 P1 9종을 생성하고 9/9 기대 해상도·투명 배경·Alpha·비어 있지 않은 픽셀·검은 반투명 Edge 위험 검사를 Technical PASS했다.
- 사용자 검토용 `SourceArt/UI/HUD/P1/Review/P1_HUD_Review.png`와 `art01_review.json`을 생성했다. 현재 9종 모두 `UserVisualApproval=false`, `ImportAllowed=false`다.
- 사용자 승인과 기술 검토를 분리해 Approved Asset 0, Unreal Import 0, `DA_CFHUDVisual_Default` 연결 0, Production Apply 0을 유지했다.
- 다음 Gate를 P1 9종 개별 User Visual Review로 고정했다. 승인된 파일만 D1-11-ART-02로 넘기며 D1-11·D1-12·UI-P0-03 상태는 변경하지 않았다.

### v0.41.0 - 2026-08-11

- D1-11 Production HUD 구조 위에 사용할 전용 그래픽 자산의 SSOT로 `InGameUIHUDArtSpec.md v0.1.0`을 신규 추가했다.
- 전용 HUD Art와 기존 Semantic Icon의 책임을 분리하고, `DA_CFHUDVisual_Default` 각 필드의 실제 Production Bridge 소비 여부를 문서화했다.
- 즉시 연결 가능한 P1 Texture 9종(`VehicleSilhouette`, 6방향 Armor, `SpeedArcTrack`, `SelectedTargetBracket`)과 권장 Naming·해상도·PNG/Alpha·UE UI Import 규격을 고정했다.
- `SpeedArcMaterial`, `RadarFrame`, `RadarSweepMaterial`, `SelectedWeaponFrame`은 현재 Production Widget 소비 경로가 없어 P2 Hookup 선행 대상으로 분리했다.
- 현재 D1-11 Prototype은 DA 변경 뒤 Production Apply를 다시 실행해야 Designer Brush에 반영되는 실제 Workflow를 명시했다. 이번 작업은 문서 전용이며 Source·Config·Unreal Asset을 변경하지 않았다.
- 2026-08-11 생성 HUD 콘셉트 이미지는 Working Visual Direction Reference A로만 기록했으며 USER PASS 또는 Repository Asset으로 승격하지 않았다.

### v0.40.0 - 2026-08-10

- D1-11 Production Rework를 실제 UMG 자산까지 적용했다. `WBP_CFInGameHUD` Root 1, Mission/Alert/Target/Vehicle/Radar/Weapon Panel 6, `WBP_CFSpeedGauge`·`WBP_CFArmorBodyMap` Element 2, `DA_CFHUDVisual_Default` 1의 exact 10 Asset 구조다.
- `CFUIHUDProdEditorBridge v1.0.0`, `ApplyUIHUDProduction.py v1.0.0`, `RunUIHUDProduction.ps1 v1.0.0`을 추가했다. Panel은 실제 Surface Border 1개만 허용하고 Element는 Border 0, Image 기반 Semantic 시각 슬롯과 TextBlock·ProgressBar를 사용하며 Mock/Mosaic 이름과 Runtime Graph를 검증에서 차단한다.
- 공식 Build `2e63b4a2ee77448a95bd645ac23b47cb`, Production Bridge Build `5653c410a2674232aa5477f12437c91d`, Probe `ac83c6588cdb49dca292e0653774d08b`, DryRun `3e3ff6fa152942fca0f555083779632c`, Apply `3c7dfff73b4042f895e98989c825c9d1`, Saved Readback `eac37017a42f4a46989c98691d3fdb9f`가 PASS했다.
- 독립 AssetDump는 WidgetBlueprint 9/9·HUD 전체 10/10 Success, 보호 회귀 `05faa8326c7848a5b1ca18525b59540c`는 전체 53/53·필수 24/24 Success다. Border Mosaic·Gameplay Cast·Runtime Event Binding·D1-12·UI-P0-03는 0이다.
- 자동 검증은 닫혔지만 새 Production 1920×1080 사용자 Preview가 Pending이므로 `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.

### v0.39.0 - 2026-08-10

- D1-11 Border Mosaic 거부 후 Production Visual Composition Rework를 실제 소스 기반으로 착수했다. Production Widget 경계는 Root `WBP_CFInGameHUD`, 기능 Panel 6개, 의미 Element `WBP_CFSpeedGauge`·`WBP_CFArmorBodyMap` 2개와 HUD 전용 Visual DataAsset 1개로 고정한다.
- 기존 D1-09B `DA_CFUIStyle_Default.IconSet`의 Semantic Icon 18개를 실제 Image 구성에서 재사용하기 위해 `UCFUIStyleData v1.2.0`에 Semantic ID→Soft Icon Asset Resolver와 무효/중복 매핑 검증을 추가했다. 별도 Icon DataAsset은 만들지 않는다.
- 신규 `UCFHUDVisualData v1.0.0`은 Vehicle Silhouette, 6방향 Armor Plate, Speed Arc, Radar Frame/Sweep, Selected Target/Weapon Frame 같은 HUD 전용 비Semantic 아트만 Soft Reference로 중앙 관리한다. Gameplay/ViewData/동기 Load 책임은 없다.
- `CFUIStyleDataTests v1.1.0`에 Semantic Resolver와 중복 ID 회귀를 추가했다. Phase A는 Source만 수정했고 Unreal Asset 변경은 0이다.
- 공식 Build `f28b9035c71441459d6a3ec85da853dc`는 UHT 및 관련 C++ Compile을 모두 통과했으나 열린 `UnrealEditor.exe`가 CarFight/AssetDump DLL을 점유해 LNK1104로 Link가 차단됐다. 다음 실행은 Editor 종료 후 공식 Build Link 확인이며, 그 전에는 Production Widget Assetization을 시작하지 않는다.
- `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.

### v0.38.0 - 2026-08-10

- 네 번째 D1-11 1920×1080 User Preview에서 시각적 근접도는 향상됐지만, `Canvas_Mock_VehiclePanel` 내부의 차체·바퀴·Armor Plate·Speed Arc 등 대부분의 시각 요소를 다수의 `Border`와 절대 좌표로 조립한 방식이 Production UI 설계 의도와 다르다는 사용자 피드백으로 FAIL 판정했다.
- 현재 구현은 `InGameUIVehiclePanelSpec`이 이미 정의한 `WBP_CFSpeedGauge`, `WBP_CFArmorBodyMap` 의미 단위 UMG 구조와 Blueprint/UMG의 도형·Icon·Typography 소유 방향을 제대로 따르지 못했다.
- D1-11 후속은 Border 위치 보정이 아니라 Visual Composition Rework다. Text는 TextBlock, 아이콘·차량 실루엣·장갑 형상·게이지 장식은 Image/Brush 또는 UI Material, 동적 값은 ProgressBar/UI Material, Border는 실제 패널 배경·Outline·Container에만 사용한다.
- v1.3.0의 자동 Build·Readback·AssetDump·53/53 회귀 증거는 구조 안전성 역사 증거로 보존하지만 사용자 Visual Gate를 대체하지 않는다. `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.

### v0.37.0 - 2026-08-10

- 세 번째 D1-11 1920×1080 User Re-preview에서 `NamedSlot_*` 잔상 제거와 Slot/중앙 시야는 통과했지만 VehiclePanel의 Armor 값 위치가 승인 의미와 어긋나고 Speed Arc·왼쪽 전방 차량 형상이 충분히 읽히지 않아 USER FAIL로 판정했다.
- `CFUIHUDEditorBridge v1.3.0`은 Vehicle Armor를 `좌 Front=30/100 / 상 Right=70/100 / 우 Rear=60/80 / 하 Left=63/100 / 좌상단 Top=84/100 / 우하단 Bottom=72/100`으로 정확히 배치하고 각 의미별 값과 Canvas Rect를 자동 검증한다. Speed Gauge는 더 조밀한 Arc Segment로, 차체는 Nose Shoulder·Tail·Cabin Outline·Wheel로 왼쪽 전방 방향 판독성을 강화했다.
- v1.3.0 Source Build `4e455496158f4c9bb25ee9851bb65718`, DryRun `abf8def22ad54d69a174671f926b2b9c`, Rebuild Apply `3dcf10ee81fc4e1b938849ff1260bc5d`를 PASS했다. AssetDump 병행 작업 중 Engine Module Manifest BuildId 불일치가 발생해 Readback이 일시 Blocked됐으나, 공식 Build `608a05d40bab49e7abfed3073ee1914b`에서 AssetDump 모듈까지 재Compile·Link해 정합성을 복구했다.
- 복구 후 Saved Readback `e8a74175779c41a3a6a9186bcbf6f087`, HUD AssetDump WidgetBlueprint 1/1, 보호 회귀 `01dc9e734d4946208d5de53ccc3f39fa` 53/53·필수 24/24를 PASS했다. D1-11 Source 정적 검사에서 Gameplay 직접 조회·직접 Asset Load·Runtime Dynamic Binding은 0이다.
- 네 번째 1920×1080 사용자 Preview 전까지 `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.

### v0.36.0 - 2026-08-10

- 두 번째 D1-11 1920×1080 User Re-preview에서 Slot 배치·중앙 시야·Target·Radar Contact·Weapon Compact는 승인 방향에 근접했지만 VehiclePanel의 좌측 원호형 속도계와 왼쪽 전방 차량 Armor Silhouette가 빠져 있고 Radar·Target·Weapon 위에 중첩 D1-10 Base Widget의 `NamedSlot_*` Designer Placeholder가 남아 있어 USER FAIL로 판정했다.
- `CFUIHUDEditorBridge v1.2.0`은 Vehicle Speed Gauge를 정적 원호 Segment로 보강하고 Nose·Cabin·Wheel 기반 왼쪽 전방 차량 Silhouette와 D1-07 Armor 위치 의미를 복원했다. D1-10 Base Widget과 Target InfoRow는 Class 재사용 계약은 유지하면서 Collapsed·RenderOpacity 0·화면 밖 Canvas Storage로 이동해 Designer Placeholder가 Mock 위에 겹치지 않도록 했다.
- 공식 Build `69132f9359744251a2eeb8b55dec5322`, DryRun `ea3666f226e94ecdadc57774c001a27f`, Rebuild Apply `27682e43292b4121b24172119f84faa2`, Saved Readback `47e3be11d32c4784bc21b39bdd0924f4`, HUD AssetDump 1/1, 보호 회귀 `c4f6d70abcb74270bf70d18595922558` 53/53·필수 24/24를 PASS했다.
- 직접 Gameplay 조회·Asset Load·Runtime Binding은 계속 0이며 D1-12·UI-P0-03는 시작하지 않았다. 세 번째 1920×1080 사용자 Preview 전까지 `D1-11 = NOT PASS`를 유지한다.

### v0.35.0 - 2026-08-10

- 사용자가 제공한 첫 D1-11 1920×1080 Designer Screenshot을 D1-07 `SR-1080-16`과 대조한 결과, 7개 Root Slot 위치·크기는 대체로 맞지만 내부 Visual이 Slot 좌상단에 축소되고 Vehicle·Radar·Weapon·Target이 승인 Mockup의 공간 구성을 재현하지 못해 USER Preview FAIL로 판정했다.
- `CFUIHUDEditorBridge v1.1.0`에서 Overlay Fill을 명시하고 내부를 Canvas 기반으로 재구성했다. Vehicle은 좌측 Speed/DNR + 우측 6방향 Armor + Shield/Integrity, Radar는 공간형 Contact Field, Weapon은 Compact 선택 무기 + Ammo/Heat/Cooldown/State + Rail 3개, Target은 전체 폭 Info/Scan 구조를 갖는다.
- `ApplyUIHUDPrototype.py v1.1.0`은 기존 `WBP_CFInGameHUD`를 삭제·재생성하지 않고 동일 Asset의 WidgetTree만 재구축한다. D1-09B Style/Density/Layout과 D1-10 Base Widget은 읽기 전용이다.
- 공식 Build `f28ff1c21e5d4e34824fc43e0f1f0cfc`, DryRun `71f2015ca8a94ea993475e94b689dce2`, Rebuild Apply `c531a16ea2ee48f6adc2c290ce50f302`, Saved Readback `8560bcbda6204ad0a106abe360e4dbed`, 독립 AssetDump 1/1과 보호 회귀 `10baf383a2204fb680dc65a91534c4d2` 53/53·필수 24/24를 PASS했다.
- 자동 Visual Fidelity Contract는 PASS했으나 수정 후 사용자 1920×1080 재Preview는 아직 Pending이므로 `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.

### v0.34.0 - 2026-08-10

- D1-11만 재개해 Editor 전용 `CFUIHUDEditorBridge`, `ApplyUIHUDPrototype.py`, `RunUIHUDPrototype.ps1`을 추가하고 `/Game/CarFight/UI/HUD/WBP_CFInGameHUD` 한 개만 생성했다.
- D1-09B Style·Standard/Compact Density·1080 Layout과 D1-10 Panel/Alert/InfoRow Base를 읽기 전용 입력으로 사용해 정확한 7-Slot, D1-07 Mock 값, Typography Floor, Weapon Compact와 중앙 시야 보호를 정적 검증한다. Gameplay 조회·UI-P0-03 View Data·Runtime Event Binding은 추가하지 않았다.
- Probe `a72fac6ee3ae4e199041773f2711a446`, DryRun `c020e0f2f39144e5a595c1aab31aed51`, Apply `1831d3f2ce8740e6aff814662d1cf421`, 최종 Readback `2b283ba7bd2842b483bccabafcbf7bc7`을 PASS했다.
- 첫 Apply `d1f181a339334f519a636ad71c35e576`은 UE가 신규 Widget Blueprint에 자동 생성한 미연결 기본 EventGraph 3노드를 Runtime Logic으로 과잉 판정해 Save 전 중단됐으며, 기본 `K2Node_Event`가 모든 Pin 미연결일 때만 허용하고 다른 Node 또는 연결 Pin은 계속 FAIL하도록 validator를 정밀화했다.
- 최종 공식 Build `e79da92637754ce280a956e7624fc6c6`이 `CFUIHUDEditorBridge v1.0.3` Compile·제품 DLL Link Exit 0, 독립 AssetDump는 HUD WidgetBlueprint 1/1 Success·Reference 0·Widget Binding 0·Animation 0을 확인했다.
- 최종 보호 회귀 `fd7b421ba5ba4963ae0d4a19f3273906`은 53/53 Success·필수 24/24 Success·실패 0이다.
- 자동/정적 범위는 PASS했지만 1920×1080 Designer/User Preview를 아직 수행하지 않아 `D1-11 = NOT PASS`를 유지한다. 특히 Style/Density 변경만으로 전체 표현이 함께 갱신되는지는 Preview에서 확인해야 한다.
- D1-12와 UI-P0-03은 Not Started를 유지하고 기존 D1-09B/D1-10 및 unrelated dirty 자산을 보호했다.

### v0.33.0 - 2026-08-10

- D1-10B를 재개해 공식 Editor Build Job `ca38c51a4e264ca79e8fa018eba4584f`에서 `CFUIBaseEditorBridge` UHT·Compile과 제품 DLL Link를 Exit 0으로 확인했다.
- UE 5.8 Python의 Bool + Out FString 반환 마샬링 차이를 bool-only Build/Validate Bridge로 최소 보정했으며 기존 상세 실패 함수 시그니처는 유지했다.
- Probe `ac14d231ec2248a29147187402061748`, DryRun `83b87a0b61fd40beb676d827de16ce61`, Apply `2b5959aca44443829e6f58fd68ad1f76`, 별도 Readback `93d974ce5c854c6da46ee5cbb87942d6`을 PASS했다.
- `/Game/CarFight/UI/Base/`의 정확한 Base Widget 5종만 생성해 5/5 Compile·정확 WidgetTree·Native Parent·`Button_Interaction` Bind·Style/Density Context를 확인했고 Gameplay Cast·DataAsset 직접 Load·Collapsed Spacer·Graph CallFunction은 모두 0이다.
- AssetDump에서 WidgetBlueprint 5/5 Success, 전체 보호 회귀 Process `3c8fcd4a7438478db83eefbbd0719de0`에서 53/53·필수 24/24 Success·실패 0을 확인했다. 기존 TargetHud와 D1-10A 두 Contract도 Success다.
- `D1-10 = PASS`로 닫고 `D1-11 = Prepared / Not Started`를 유지한다. 이번 작업에서는 D1-11을 시작하지 않았다.
- 기존 미커밋 변경과 D1-09B 자산을 보호했고 commit·push는 수행하지 않았다.

### v0.32.0 - 2026-08-10

- D1-10A C++ Visual Context/Button Bridge를 구현하고 공식 Editor Build와 두 D1-10A Contract Automation PASS를 확인했다.
- D1-10B를 위한 Widget Blueprint Python Reflection과 최소 WidgetTree Editor Bridge Source를 준비했다.
- 현재 열린 CarFight Editor의 제품 DLL 점유로 Bridge 공식 Link가 LNK1104에 차단되어 Base Widget Asset 생성 전 중단했다.
- D1-10은 NOT PASS이며 D1-11은 시작하지 않았다. 재개 조건은 Editor 정상 종료 후 Bridge 공식 Build PASS다.
- commit·push는 수행하지 않았다.

### v0.31.0 - 2026-08-10

- D1-09B 실제 FontFace·Runtime Font·Semantic Icon·Style/Density/1080 Layout Asset과 Font Notice·Config 연결을 완료했다.
- DryRun `ab53b08384df4e3b9b1a4d83354c4a63`, Apply `7611bbc6382546198fb9ea37dbe9b771`, Readback `d09097cc7781470ba9fd4261828a2c14`을 PASS Evidence로 기록했다.
- Readback에서 Asset·License·Config Contract와 `d1_09b_pass=true`를 확인했다.
- `WBP_TargetSelect.uasset` 파일 잠금으로 중간 전체 회귀가 50/51에 머물렀으나 TargetSelect는 수정하지 않았고, 잠금 해제 후 `e73311a38be043d48d512ab90afd258f`에서 전체 51/51·필수 24/24 Success를 확인했다.
- D1-09B를 PASS로 닫고 D1-10은 Not Started로 유지했다. UI-DESIGN-GATE 전체는 D1-10~12 완료 전 PASS로 올리지 않는다.
- commit·push는 수행하지 않았다.

### v0.30.0 - 2026-08-10

- D1-08V 공식 Editor Build와 `CarFight.UI.D1_08.StyleDataContract` Automation을 PASS로 닫았다.
- D1-09A에서 `UFont` Binding, 1080p HUD Layout Data, Density Data와 `UCFUISubsystem` Config Soft Reference·Native Fallback을 실제 구현했다.
- 최종 D1-09A 공식 Build Job `9d9aefb3803e4556bd4559afded0f4ed` PASS, Automation Process `18bea9b5952e4444b618e776cb2e2571` 전체 51/51·필수 24/24 Success를 확인했다.
- D1-09A 네 Contract는 모두 Success / Warning 0 / Error 0이다.
- 중간 Automation 실패는 제품 코드가 아니라 UE 5.8 TArray 자기참조와 ClassWithin Outer를 잘못 구성한 테스트 Fixture 문제였고 v1.0.3에서 해결했다.
- D1-09B Unreal Asset·Config는 Not Started로 유지하고 commit·push는 수행하지 않았다.

### v0.29.0 - 2026-08-07

- D1-09A~12의 실제 실행 상세를 `InGameUIAssetizationSpec.md v0.1.0`으로 분리하고 대표 Plan에서 현재 실행 SSOT로 연결했다.
- Pretendard와 IBM Plex Mono의 공식 OFL 1.1 라이선스를 확인해 상용 게임 번들 사용 적격성을 PASS로 기록했다.
- Epic 최신 Font API 기준 `UFont` Runtime Composite + `UFontFace` Font Face + `FSlateFontInfo` 경로를 확정하고 D1-09A의 Font Binding 타입을 잠갔다.
- D1-10은 C++ Visual Context/Focus Bridge + Blueprint Base Widget 5종, D1-11은 기능 없는 1080p UMG Prototype, D1-12는 Native Fallback을 보존하는 Pause Visual Migration으로 정확한 범위를 고정했다.
- Source·Config·Unreal Asset·Build·Automation·PIE·commit·push는 수행하지 않았다.

### v0.28.0 - 2026-08-07

- 사용자 요청에 따라 에디터 종료가 불가능한 현재 세션에서는 D1-09~12의 exact implementation preparation만 진행하고 다른 UI/runtime 기능으로 확장하지 않도록 범위를 잠갔다.
- 실제 소스 조사에서 Font Asset Binding과 `UCFHUDLayoutData`·`UCFUIDensityData`가 아직 없음을 확인해 D1-09A 직접 선행조건으로 분리했다.
- D1-09B는 `DA_CFUIStyle_Default`, Density 3종, `DA_CFHUDLayout_1080_16`, 실제 Font·P0 Semantic Icon만 허용하고 1440p·21:9·32:9는 Deferred했다.
- D1-10 Base Widget 5종, D1-11 기능 없는 `WBP_CFInGameHUD` Visual Prototype, D1-12 기존 Pause 계약 보존 Visual Migration의 순서·Allowlist·검증·중단 조건을 `InGameUIStyleSpec.md v0.16.0`에 고정했다.
- Source·Config·Unreal Asset·Build·Automation 추가 실행은 하지 않았고 commit·push도 수행하지 않았다.

### v0.27.0 - 2026-08-07

- UI-DESIGN-GATE의 다음 단계인 D1-08 C++ Style Data Type 소스를 신규 구현했다.
- `CFUIStyleData.h/.cpp`에 승인된 Style Token·Font Family Role·Semantic Icon·Native Safe Fallback·검증 계약을 추가하고 `CFUIStyleDataTests.cpp`에 D1-08 Automation을 추가했다.
- 기존 UI 런타임 통합 파일과 Unreal Asset은 수정하지 않아 D1-08을 독립된 기반 타입 변경으로 제한했다.
- 현재 D1-08은 Code Applied / Static Readback PASS이며 공식 Editor Build와 Automation 실행 전에는 PASS 또는 D1-09 착수 완료로 판정하지 않는다.
- 에디터가 실행 중이므로 공식 빌드는 실행하지 않았고 commit·push도 수행하지 않았다.

### v0.26.0 - 2026-08-07

- 사용자가 외곽 재배치된 `SR-1080-16` 수정본을 승인해 D1-07 Phase 1을 USER PASS로 닫았다.
- 현재 기본 HUD 배치는 패널 크기를 유지하면서 1080p 외곽 Margin을 실효 약 24px로 사용하는 구성이다.
- `InGameUIStyleSpec.md v0.14.0`, `InGameUIStaticReview.md v0.6.0`을 현재 승인 체크포인트로 연결했다.
- UI-DESIGN-GATE 전체는 D1-08~12 Style Data·Layout/Density Asset·Base Widget·Visual Prototype과 실제 Font·Icon Asset/라이선스 확인이 남아 있어 Assetization Pending으로 유지했다.
- SR-A·C·D·E와 1440p·21:9·32:9는 현재 Gate 비차단 후속 검토로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.25.0 - 2026-08-07

- 사용자 D1-07 시각 피드백에 따라 `SR-1080-16`의 고정 HUD 패널을 안전영역 안에서 더 바깥쪽으로 재배치했다.
- 패널 크기와 정보 밀도는 유지하고 1080p Phase 1 외곽 Margin을 실효 약 24px로 줄여 중앙 전투 시야 확보를 우선했다.
- `InGameUIStyleSpec.md v0.13.0`, `InGameUIStaticReview.md v0.5.0`을 현재 체크포인트로 연결했다.
- 1440p·21:9·32:9는 Deferred를 유지하고 1080p 외곽 배치 Override를 자동 전파하지 않는다.
- 사용자 재검토, Runtime Capture와 UI-DESIGN-GATE PASS는 수행하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.24.0 - 2026-08-07

- D1-07 Phase 1의 실제 사용자 Gate를 `SR-1080-16` 1920×1080 / 16:9 하나로 동기화했다.
- 2560×1440·3440×1440·5120×1440 Static Mockup은 삭제하지 않고 후속 Layout Profile 확장 참고자료로 Deferred했다.
- 필요 시 SR-A·C·D·E 상태 검토도 같은 1920×1080 기준에서 먼저 수행하도록 정리했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.23.0 - 2026-08-07

- D1-07용 1920×1080, 2560×1440, 3440×1440, 5120×1440 Static Mockup 4개를 준비했다.
- `InGameUIStaticReview.md v0.1.0`에 해상도별 검토 좌표, PASS/FAIL 체크리스트, SR-A~E 상태 세트와 전역 FAIL 조건을 기록했다.
- 대표 Mockup은 동일 Combat Busy 상태를 사용하며 Pretendard + IBM Plex Mono와 Solid Core + Tactical Cut 기본 Preset을 반영했다.
- 21:9·32:9는 Persistent HUD를 중앙 2560×1440 Canvas에 고정하고 좌우 추가 폭에는 월드만 확장했다.
- 현재 상태는 Static Mockups Ready / User Review Pending이며 Runtime Capture와 UI-DESIGN-GATE PASS는 수행하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.22.0 - 2026-08-07

- 사용자가 D1-06 Font·Icon 추천안을 전부 승인했다.
- 기본 UI Font를 Pretendard, Numeric/Technical Font를 IBM Plex Mono로 확정하고 Style Data의 Font Family Role로 교체 가능하게 유지했다.
- Icon을 `Solid Core + Tactical Cut`, 24×24 Base Grid, 16/20/28 기본 크기와 Weapon 18/20으로 확정했다.
- Shield=Energy Field, Armor=Chamfered Plate, Integrity=Vehicle Chassis, Weapon=Side/Profile Silhouette, Target=관계색 Marker+Cyan Bracket, Lock=독립 Segment, Radar=관계색+기하 Symbol을 기본 Preset으로 승인했다.
- 실제 Font·Icon Asset 경로 대신 Semantic Role/ID를 사용하고 전체 세트 교체가 Gameplay·Presenter·View Data 변경을 요구하지 않도록 했다.
- UI-DESIGN-GATE를 D1-05·D1-06 Accepted / D1-07 Static Review Not Run으로 갱신했다.
- 실제 Font·Icon Unreal Asset 생성, 라이선스 최종 확인, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.21.0 - 2026-08-07

- 사용자가 D1-05 Style Token·Base Widget·Density Profile·WeaponPanel Compact Token 추천안을 전부 승인했다.
- `InGameUIStyleSpec.md v0.9.0`, `InGameUIWeaponPanelSpec.md v0.5.0`을 User Accepted 기본 Preset SSOT로 연결했다.
- `ValueS`, Compact Panel 12/36, Density별 StatusBar, InfoRow Label 96/120/144, Weapon Tile Icon 18과 Role 기반 Primary Value를 승인 변경으로 반영했다.
- 승인 수치는 C++·WBP 하드코딩 불변값이 아니라 Style·Layout·Density Data Asset 및 명시적 Override로 변경 가능한 기본값으로 고정했다.
- UI-DESIGN-GATE를 D1-05 Accepted / D1-06 Font·Icon Pending / D1-07 Static Review Not Run으로 갱신했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.20.0 - 2026-08-07

- `InGameUIStyleSpec.md v0.8.0`에 CarFight 전체 UI Style Token·Base Widget 공통 계약·Compact/Standard/Expanded Density Profile의 상세 수치 초안을 작성했다.
- 1080p 유효 Typography 하한, Style/Layout/Density/User Override 해석 순서와 접근성·Reticle Protection 안전 하한을 문서화했다.
- `InGameUIWeaponPanelSpec.md v0.4.0`에 기본 폭 360, Selected 높이 142~178, Full 높이 254, Tile 112×68과 Secondary 0/1/2 자동 Layout을 포함한 Compact Token 상세 초안을 작성했다.
- 전역 Static Review 최소 행렬을 `SR-1080-16 / SR-1440-16 / SR-1440-21 / SR-1440-32`, WeaponPanel 상태 세트를 `WP-SR-01~12`로 정의했다.
- 이번 수치는 Detailed Draft / User Review Pending이며 UI-DESIGN-GATE PASS나 Runtime·UMG 구현 완료로 승격하지 않았다.
- 실제 Font·Icon 선정, Static Capture, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.19.0 - 2026-08-07

- 사용자가 커스터마이징 가능한 구조를 WeaponPanel 한정이 아닌 CarFight 전체 UI의 필수 설계 계약으로 확정했다.
- Gameplay System → Presenter/ViewModel → Blueprint Visual Widget → Style·Layout·Density Data Asset → User Override의 소유권 계층을 Plan 완료 조건에 추가했다.
- 안정적인 PanelId·SlotId, 교체 가능한 Visual Widget Class, 기본 Preset 상속·복제와 명시적 Override 계약을 확정했다.
- Compact·Standard·Expanded·Custom Density와 16:9·21:9·32:9 Layout Profile, 후속 사용자 위치·Scale·Opacity·Visibility 저장 확장 경계를 기록했다.
- 자동 Layout과 `Collapsed` 기반 빈 공간 제거를 필수로 하고 Canvas 고정 좌표 사용을 Root·Projection·Reticle·Radar Blip 범위로 제한했다.
- WeaponPanel은 화면 점유를 줄이는 Compact Revision 방향으로 승인했으며 `464×360`은 최대 Slot, 기존 `432×224`·`136×96`은 초기 Wireframe 참고값으로 재정의했다.
- 정확한 Compact Token은 Style·1080p·1440p Static Review Pending이며 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.18.0 - 2026-08-07

- 사용자가 `D1-WEAPON-PANEL` 상세 구조를 승인해 `Visual Layout Accepted`로 승격했다.
- `InGameUIWeaponPanelSpec.md v0.2.0`과 `CFWeaponPanel_1440p.xml`을 우하단 WeaponPanel의 승인된 시각 계약으로 확정했다.
- `464×360` 투명 Root Slot, `432×224` Selected Card, Primary 1개·Secondary 최대 2개·FireStateStrip, 최대 3개 Compact Tile 또는 `2 + Overflow` 구조를 유지한다.
- Ammo·Charge·Heat·Cooldown·Reload·LauncherSequence 공통 채널과 탄약형·에너지형·런처형·복합형 표시 규칙을 승인 범위에 포함했다.
- 현재 미구현 Runtime 채널 숨김, VehiclePanel `896×416`, Radar 위치와 중앙 Reticle Protection 영역을 그대로 보호한다.
- UI-DESIGN-GATE 전체 PASS는 Style Token·Base Widget·Font·Icon과 해상도 Static Review 완료 전까지 보류한다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.17.0 - 2026-08-07

- `D1-WEAPON-PANEL` 상세 초안으로 `InGameUIWeaponPanelSpec.md v0.1.0`과 `CFWeaponPanel_1440p.xml`을 작성했다.
- 승인된 우하단 `464×360` Slot 안에서 전체 배경을 채우지 않는 투명 Root, `432×224` Selected Card와 조건부 Compact Rail 구조를 정의했다.
- 현재 선택 무기는 3px Accent·Corner Marker·대형 Primary Channel로 가장 강하게 강조하고 비선택 무기는 최대 3개 Tile 또는 `2 + Overflow`로 축약했다.
- Ammo·Charge·Heat·Cooldown·Reload·LauncherSequence와 발사 가능 상태의 공통 채널·표시 우선순위를 정의했다.
- 탄약형·에너지형·런처형·복합형 무기별 Primary·Secondary Channel 규칙과 화면 점유 상한을 정의했다.
- 현재 실제 Runtime은 활성 무기 1개, FireFeedback·Cooldown·Launcher Sequence이며 다중 무기·Ammo·Charge·Heat·Reload Runtime은 없음을 분리했다.
- 지원되지 않는 채널은 가짜 `0`이나 `N/A` 행 대신 숨기도록 했다.
- D1-VEHICLE-PANEL의 승인된 `896×416` 구조와 전체 16:9 TPS HUD 배치는 변경하지 않았다.
- CommonUI 판단, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.16.0 - 2026-08-07

- D1-VEHICLE-PANEL Visual Layout을 사용자 승인 상태로 승격했다.
- 896×416 가로형, 원호형 속도계, 왼쪽 전방 Armor 실루엣, Shield·Integrity 전체 폭 Bar 계약을 확정했다.
- 부품 손상 목록을 고정 VehiclePanel에서 제외했다.
- 다음 UI-DESIGN-GATE 상세 단계를 `D1-WEAPON-PANEL`로 이동했다.
- UI-P0-02 Partial USER PASS와 UI-P0-03 Not Started는 변경하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.15.0 - 2026-08-07

- `D1-VEHICLE-PANEL` 상세 초안으로 `InGameUIVehiclePanelSpec.md v0.1.0`과 560×416 Wireframe을 작성했다.
- 속도·D/R/N·Drive State·Handbrake, Shield, Front·Left·Right·Rear Body Map, Top·Bottom Armor, Integrity와 부품 손상 확장 규칙을 정의했다.
- 현재 실제 Gear 번호와 부품 손상 Runtime이 없음을 확인해 첫 구현에서 가짜 값을 표시하지 않도록 했다.
- Vehicle Panel 상태를 Draft / User Review Pending으로 갱신하고 Weapon Panel은 Pending·미변경으로 유지했다.
- UI-P0-02와 UI-P0-03 코드 상태는 변경하지 않았으며 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.14.0 - 2026-08-07

- 기본 화면을 외부 3인칭 차량 TPS와 16:9 기준으로 확정했다.
- Mission 좌상단, Alert 상단 중앙, Target 우상단, Vehicle 좌하단, Radar 하단 중앙, Weapon 우하단 배치를 승인했다.
- 기존 좌하단 Radar와 중앙 하단 독립 Speed Cluster 배치를 폐기하고 속도를 Vehicle Panel 범위에 포함했다.
- 내 차량 정보와 무기창은 위치만 승인하고 내부 구조를 별도 상세 설계로 분리했다.
- 각 HUD 모듈에 안정적인 Slot ID와 기본 Layout Profile을 두어 후속 사용자 배치 커스터마이징 확장을 고려하도록 했다.
- CommonUI 판단은 다루지 않았고 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.13.0 - 2026-08-06

- `InGameUIStyleSpec.md v0.1.0`에 DA_CFUIStyle 상세 Token과 공통 Base Widget 5종 규격을 작성했다.
- `ConceptArt/CFHUDWireframe_1440p.xml`에 2560×1440 HUD Wireframe과 좌표·Anchor·Reticle Protection을 작성했다.
- UI-DESIGN-GATE 상태를 Style Spec·1440p Wireframe Draft / User Review Pending으로 갱신했다.
- Unreal Style Asset·Base Widget·Font·Icon과 16:9·21:9·32:9 실제 화면 검토는 미완료로 유지했다.
- CommonUI 판단은 다루지 않았으며 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.12.0 - 2026-08-06

- 사용자가 디자인 무게 `C. 기존 70:20:10 균형 유지`를 승인했다.
- 실용적 전투 정보 70%, 차량 계기판 정체성 20%, 세계관 장식 10%를 Visual Direction Accepted 기준으로 기록했다.
- UI-DESIGN-GATE 상태를 Visual Direction Accepted / Production Artifacts Pending으로 갱신했다.
- Style Data·Base Widget·Font·Icon·Wireframe과 해상도별 검토가 남아 있으므로 Gate PASS는 보류했다.
- 승인된 시각 방향을 상위 `14_CombatUI.md`와 DecisionLog에 동기화하도록 Migration 기준을 갱신했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.11.0 - 2026-08-06

- 사용자가 차량 탑재형 전술 인터페이스 콘셉트, 스타일라이즈드 평면 UI와 Cyan·Amber·Red Palette 방향을 승인했다.
- 디자인 무게 A·B·C는 선택이 표시되지 않아 Pending으로 유지했다.
- UI-DESIGN-GATE 상태를 Direction Partially Approved / Design Weight Pending으로 갱신했다.
- Style Data·Base Widget·Wireframe과 상위 SSOT Accepted 승격은 아직 수행하지 않았다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.10.0 - 2026-08-06

- `InGameUIVisualConcept.md v0.1.0` Working Draft를 생성하고 대표 Plan에서 공식 참조하도록 연결했다.
- 콘셉트 후보를 `차량 탑재형 전술 인터페이스`로 정의하고 근미래 차량·전술·스타일라이즈드 렌더링 호환 방향을 구체화했다.
- 형태, 재질, 색상 Token, Typography, Spacing, Icon, Motion, 화면 밀도와 HUD 요소별 시각 원칙을 초안으로 기록했다.
- `UI-DESIGN-GATE` 상태를 Concept Draft / User Review Pending으로 변경했으며 완료나 Accepted 결정으로 판정하지 않았다.
- 실제 Font·Icon·Style Asset·Wireframe과 최종 Palette는 사용자 승인 뒤 확정하도록 유지했다.
- 상위 Combat UI SSOT와 DecisionLog는 아직 변경하지 않았고 Source·Asset·Build·Automation·commit·push도 수행하지 않았다.

### v0.9.0 - 2026-08-06

- UI 디자인 품질 개선과 CommonUI 도입 여부를 서로 독립된 계획 이슈로 분리했다.
- 디자인 품질은 현재 P0 HUD 경로에서 `UI-DESIGN-GATE`로 관리하고, 정보 계층·가독성·상호작용 인지성·Style Data·Base Widget을 CommonUI 없이도 구현하도록 확정했다.
- CommonUI는 외형 개선 도구가 아니라 전체 화면 UI의 입력·내비게이션·Back·Activatable Stack 프레임워크 후보로 정의했다.
- `UI-COMMONUI-GATE`는 타이틀·차고·피팅·임무·결과 중 첫 전체 화면 Flow 구현 전에 실행하며 현재 인게임 HUD P0를 차단하지 않는다.
- 향후 CommonUI 도입 시에도 UCFUISubsystem 진입점, View Data·Presenter와 Gameplay/UI 책임 분리를 유지하도록 Migration 경계를 기록했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.8.0 - 2026-08-06

- 사용자 Standalone에서 Escape Pause, 실제 World 정지, Legacy Reticle·Target HUD 아래 배치, 계속하기 마우스 클릭, Enter·Escape 해제와 차량·카메라 입력 복귀를 확인했다.
- 첫 사용자 재검증에서 확인된 UI Root 계층 문제를 `AddToPlayerScreen`에서 `AddToViewport(100)`으로 전환해 수정했다.
- 계속하기 버튼의 가시성과 클릭 영역을 보강하고 Controller가 Pause 중 Enter·게임패드 확인 입력을 Continue Fallback으로 처리하도록 수정했다.
- 사용자가 직접 `Tools\BuildEditor.bat` 빌드를 통과했고 Combat Automation `595e0c1ca0f640b69f63964bdfd22277`에서 전체 46/46·필수 24/24 Success·실패 0을 확인했다.
- UI-P0-02는 Pause UI 상호작용 범위만 USER PASS로 기록하고 Gamepad·입력 유지 잔류·Ripple·Salvo·Projectile·추진·World Timer·반복 Pause·레벨 재진입 수명 검증은 Pending으로 유지했다.
- UI-P0-03, AimReticle·TargetSelect Root 이전과 Unreal Asset 수정은 시작하지 않았고 commit·push도 수행하지 않았다.

### v0.7.0 - 2026-08-06

- `CF-FQ-033 차량 방어·손상 런타임`의 DR-P0-00~07, DR-PIE-00~06 USER PASS와 VehicleDefense·HitDamage Current System 승격 완료를 반영했다.
- 데미지 시스템 완료 후 재개한다는 기존 우선순위에 따라 `CF-FQ-032`를 현재 단일 Active Plan으로 복원했다.
- 재개 순서를 UI-P0-02 실제 Pause·Focus·Launcher·Projectile·Timer 사용자 PIE로 고정하고, PASS 뒤 UI-P0-03 HUD 데이터 계약으로 진행하도록 했다.
- UI-P0-03 Defense View Data는 `VehicleDefenseComp` 실제 상태와 `VehicleDefense.md v1.0.0`을 읽기 전용으로 사용하며 피해 계산을 UI로 이동하지 않는다.
- 기존 UI Foundation, Build·Automation 증거, Pawn Gameplay Context 소유권과 보호된 Launcher·Missile·Fitting·Inventory·TargetSelect 체크포인트를 유지했다.
- 코드·에셋·빌드·Automation은 변경하거나 재실행하지 않았고 commit·push도 수행하지 않았다.

### v0.6.0 - 2026-08-02

- 차량 Runtime 준비 상태를 `bVehicleCoreRuntimeReady`와 `bVehicleCombatRuntimeReady`로 분리하고 기존 `bVehicleRuntimeReady`를 Core 호환값으로 유지한 계약을 반영했다.
- 차량 Gameplay Mapping Context는 Pawn이 소유하고 Controller는 Pause·Back·System·UI 입력을 소유하도록 현재 입력 경계를 고정했다.
- 구현된 `UCFVehicleDefenseComp`의 Shield·6방향 Armor·Vehicle Integrity를 UI-P0-03 실제 View Data 입력으로 사용하도록 오래된 Mock 전제를 제거했다.
- `CF-FQ-034 FIT-P0-05`와 `CF-FQ-035 INV-P0-04` 완료 상태를 보호 범위에 반영하고 Field Fitting Coordinator 미구현 상태를 분리했다.
- UI-P0-03, 기존 AimReticle·TargetSelect Root 이전과 Unreal Asset 수정은 수행하지 않았다.

### v0.5.0 - 2026-08-01

- `ACFPlayerController`에 Pause 전 입력 중립화, 눌린 키 Flush와 실제 싱글플레이 World Pause 적용·해제 API를 추가했다.
- `ACFVehiclePawn::ClearGameplayInputForPause()`와 `UCFVehicleDriveComp::ClearDriveInputs()`로 이동·조향·브레이크·핸드브레이크·Look 입력 잔류를 제거했다.
- 차량 물리 Velocity와 진행 중 Launcher Ripple·Salvo 상태는 취소하지 않고 World Pause 동안 보존하도록 했다.
- `UCFPauseMenuWidget`을 에셋 없는 C++ WidgetTree로 추가하고 Menu 레이어, Continue 버튼, Pause·Back 해제 수명을 연결했다.
- `UCFUISubsystem`이 Pause Menu 생성·제거와 화면 상태를, `ACFPlayerController`가 실제 World Pause를 소유하도록 구현했다.
- Launcher·Projectile·ProjectileMotor가 Pause 중 별도 Tick을 허용하지 않고 게임 진행 Timer가 World TimerManager를 사용하는 구조 회귀를 추가했다.
- 최종 공식 Editor Build `4f2daa07187a41aea7c400386010b2a0`가 Exit Code 0으로 통과했다.
- 최종 Combat Runtime Automation `6f0a9a5ebde5472b8b784a9103ce484d`에서 전체 32/32 Success, Failed 0과 필수 회귀 17/17 Success를 확인했다.
- 신규 `PauseMenuContract`, `InputNeutralContract`, `ProgressFreezeContract` 3건이 Success했다.
- 피팅 `CF-FQ-034 FIT-P0-03` Compatibility·MassSnapshot 회귀와 관련 소스·문서·에셋을 그대로 보존했다.
- 실제 Pause 조작·Focus·Ripple·Projectile 정지 체감은 사용자 PIE Pending으로 남겼다.
- `UI-P0-03`은 시작하지 않았다.

### v0.4.0 - 2026-08-01

- `ACFPlayerController`를 추가하고 `ACFSingleGameMode`의 기본 PlayerController로 연결했다.
- Controller가 자신이 등록한 System·Gameplay·UI Mapping Context만 제거하도록 소유권을 분리했다.
- Pause·Back Input Action과 에셋 미지정 시 Pause 중 실행 가능한 안전 Fallback Key를 추가했다.
- UI 입력 모드에서 기존 Pawn의 Gameplay 입력을 삭제하지 않고 일시 억제·복원하도록 했다.
- `UCFUISubsystem`과 World별 `UCFUIRootWidget`을 추가하고 Game·HUD·Screen·Panel·Menu·Modal·System·Debug CanvasPanel 레이어를 구현했다.
- 주요 Screen·Modal 전환, Input Mode, Cursor와 Focus 기반을 추가했다.
- 기존 Pawn의 Mapping Context, AimReticle과 TargetSelect 생성 경로를 변경하지 않았다.
- 공식 Editor Build `cd935342a3714887af98b26ed0d95ab6`이 Exit Code 0으로 통과했다.
- Combat Runtime Automation `91bc6df11f4d41968a39e5fdd44b79df`에서 전체 29/29 Success, Failed 0과 필수 회귀 17/17 Success를 확인했다.
- 피팅 `CF-FQ-034` 관련 소스·문서·에셋과 모든 Unreal Asset은 수정하지 않았다.
- 사용자 PIE와 `UI-P0-02` 완전 Pause는 미수행 상태로 남겼다.

### v0.3.0 - 2026-07-31

- 피팅 기능을 `CF-FQ-032` 범위에서 제외하고 별도 기능·별도 Plan으로 기획하도록 확정했다.
- 후속 전체 화면을 위한 Screen Layer와 공통 화면 전환 진입점을 UI 기반 계약에 추가했다.
- UI Subsystem과 Loadout 소유권, InGame HUD와 Fitting View Data, Gameplay Pawn과 Garage Preview Actor를 분리했다.
- 피팅의 Draft·검증·적용, 계산, 저장과 출격 Snapshot은 후속 피팅 기능이 소유하도록 기록했다.
- 피팅 기능 ID와 Plan은 실제 기획 착수 시 생성하며 현재는 만들지 않는다고 명시했다.

### v0.2.0 - 2026-07-31

- 실제 코드 검토 결과를 `UI-P0-00A`로 기록했다.
- 사용자가 승인한 15개 구현 전 계약을 `UI-P0-00B Done`으로 확정했다.
- 최소 `ACFPlayerController`, 입력 Mapping Context 수명과 World별 UI Root 단계를 추가했다.
- 기존 Pawn 소유 AimReticle과 TargetSelect HUD를 각각 단계 이전하도록 로드맵을 분리했다.
- `Unknown`, `Unavailable`, `KnownZero`, Target Knowledge, 화면 밖 투영과 Alert 중복 제거 계약을 완료 조건에 추가했다.
- 다음 실제 착수점을 `UI-P0-01A`로 변경했다.

### v0.1.0 - 2026-07-30

- CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크 Ready Plan을 생성했다.
- 현재 인게임 범위와 향후 전체 게임플로우 확장 경계를 분리했다.
- LocalPlayer Root, 완전 Pause, 데이터 계약, Reticle 통합, 차량·무기·타겟·Radar 단계와 검증 기준을 정의했다.
- 현재 Active CF-FQ-029를 변경하지 않고 Source/Asset Not Modified 상태로 등록했다.

---

## 14. Migration

### v0.58.5 적용 안내

- Defense Runtime·저장 맵 PIE → Provider ViewData와 Pawn Rebind Old Pawn 이벤트 해제의 기술 검증은 완료했다. 관련 Source가 다시 바뀌지 않는 한 USER Visual 확인을 위해 targeted Automation을 반복하지 않는다.
- `Defense PIE Automation Success`와 `Pawn Rebind Technical Automation Success`는 USER PASS가 아니다. 현재 남은 Gate는 `Defense Production Panel USER Visual → Pawn Rebind USER Visual` 두 항목뿐이다.
- 두 USER Visual Gate가 모두 PASS하기 전 UI-P0-03·CF-FQ-032 완료 처리, Systems 승격과 UI-P0-04 Source 착수를 금지한다.
- `Tools/RunUIAutomation.ps1 v1.0.3`은 이번 검증의 작업 전용 Execution Method이며 별도 공용화 결정 없이 공식 범용 Tool로 해석하지 않는다.
- v0.58.4의 `Defense 실제 Shield·Armor·Integrity 변화 USER PIE → Pawn Rebind USER PIE` 문구는 당시 체크포인트 이력이며 v0.58.5 current next gate를 덮어쓰지 않는다.

### v0.58.4 적용 안내

- Launcher USER Gate는 닫혔다. 이후 세션에서 `TestMap_DRSalvo`를 UI-P0-03 선행 Gate로 다시 요구하지 않는다.
- finite Ammo와 Salvo를 함께 검증해야 할 때는 `/Game/Maps/TestMap_AmmoSalvo` 격리 fixture를 사용한다. `TestMap_DRSalvo`는 기존 무한탄 Launcher 회귀 의미를 유지한다.
- 현재 next gate는 Defense 실제 Shield·Armor·Integrity 변화 USER PIE이며 PASS 후 Pawn Rebind USER PIE로 이동한다.
- Defense·Pawn Rebind 전체 USER PASS 전 UI-P0-03 완료·Systems 승격과 UI-P0-04 Source 착수는 금지한다.
- 이번 fixture 생성용 `Tools/CreateAmmoSalvoFixture.py`와 `Tools/RunAmmoSalvoFixture.ps1`은 작업 전용 실행 수단이며 별도 공용화 결정 전 공식 범용 Tool로 해석하지 않는다.

### v0.58.3 적용 안내

- Target/Radar ViewData 구현 준비 계약은 현재 USER Gate를 건너뛴 실제 구현이 아니다. `TestMap_DRSalvo → Defense 실제 변화 → Pawn Rebind → UI-P0-04 → UI-P0-05` 순서를 유지한다.
- 후속 Target Knowledge 구현은 `InGameUIDesign.md v0.35.0`의 Sub-ViewData와 additive migration을 기준으로 하며 기존 `FCFTargetHUDData` Flat 필드를 즉시 삭제하지 않는다.
- Radar/Sensor Runtime이 실제로 구현되기 전 `FCFRadarHUDData.Availability=Unavailable` 정책과 가짜 Contact 0을 유지한다.
- Scanner Gameplay가 구현될 때 Sensor 탐지·Last Known·Active Scan·Tactical Analysis 판정은 Gameplay Provider에 두고 HUD Widget에는 계산을 추가하지 않는다.
- Range Zoom은 Display Range만 변경하고 실제 Sensor Detection Range를 변경하지 않는다.

### v0.58.2 적용 안내

- 현재 실행 Gate는 변경하지 않는다. `TestMap_DRSalvo` Launcher USER PASS → Defense 실제 변화 → Pawn Rebind로 UI-P0-03 USER Gate를 먼저 닫고, 그 뒤 UI-P0-04 실제 통합·검증을 완료한 다음 UI-P0-05를 착수한다.
- UI-P0-05에서 `UCFTargetSelectComp`의 후보 탐색·선택·해제·수명·장비 조회 정책은 변경하지 않는다. CF-FQ-026 TS-P0-08 튜닝도 함께 수행하지 않는다.
- TargetSelect 월드 Marker는 Root `Game` Layer에 UISubsystem 단일 수명으로 두고, 같은 변경에서 Pawn의 `CreateTargetSelectWidget/AddToViewport` 경로를 비활성화한다.
- `WBP_TargetSelect`은 Marker Visual로 재사용하되 이름·거리·상세 Track Text의 독립 Gameplay 조회를 제거한다. 선택 Target의 Player-facing 의미 정보는 Production `WBP_CFTargetPanel`의 `FCFTargetHUDData` 경로가 단일 소유한다.
- 내부 Actor `GetName()`과 `TargetId`를 Player-facing 이름 fallback으로 사용하지 않는다. 명시적 공개 DisplayName이 없으면 Identity는 Unknown/`???`를 사용한다.
- UISubsystem 소유 Marker는 Weak Pawn Binding과 대칭 Delegate 해제로 Old Pawn을 보존하지 않아야 하며 Tick은 프레임 기반 월드 투영에만 사용한다.

### v0.58.1 적용 안내

- 현재 실행 Gate는 변경하지 않는다. 먼저 `TestMap_DRSalvo` Launcher Presentation USER PIE를 완료하고, Launcher USER PASS 뒤 Defense 실제 변화와 Pawn Rebind를 검증한다.
- UI-P0-04 사전 설계는 구현 승인이 아니다. UI-P0-03 USER Gate가 끝나기 전 Pawn의 Reticle 생성 경로나 `WBP_AimReticle` Asset을 변경하지 않는다.
- UI-P0-04 구현 시 `UCFUISubsystem`이 기존 HUD Layer에서 Reticle 수명을 소유하고 `OnCurrentPawnChanged` 계열 수명으로 Rebind한다. 같은 변경에서 Pawn direct `AddToViewport` 생성 경로를 비활성화해 중복 Reticle을 허용하지 않는다.
- `Image_CenterDot`, `Image_WeaponReticle`, Aim/FireFeedback 의미와 프레임 기반 월드 투영을 보존한다. WBP는 Visual 책임만 유지하며 Gameplay Cast·판정을 추가하지 않는다.
- UISubsystem 소유 Widget은 Pawn보다 오래 살 수 있으므로 Old Pawn을 강하게 붙잡지 않는 Weak Binding 또는 동등한 명시적 비보존 계약을 검증한다.

### v0.22.0 적용 안내

- D1-06 Font·Icon 기본 Preset은 User Accepted이며 `InGameUIStyleSpec.md v0.10.0`과 `InGameUIVisualConcept.md v0.8.0`을 기준으로 한다.
- Pretendard·IBM Plex Mono의 실제 Font Asset을 Import하기 전 라이선스·게임 패키지 재배포 조건을 최종 확인한다.
- Widget은 실제 Font 파일·Icon Texture 경로를 직접 참조하지 않고 Font Family Role과 Semantic Icon ID를 사용한다.
- D1-07에서 판독성 문제가 있으면 Semantic 의미와 D1-05 계약을 유지한 채 Font/Icon Data만 조정한다.
- 실제 Static Review 전에는 UI-DESIGN-GATE PASS로 기록하지 않는다.

### v0.21.0 적용 안내

- D1-05 승인값은 구현 기본값이지만 수정 가능한 Preset이다. Token이 존재하는 값을 WBP Literal 또는 기능 C++ 상수로 재복제하지 않는다.
- Style·Density·Layout 변경은 Data Asset 또는 명시적 Override로 수행하고 Gameplay·Presenter·View Data 계약을 유지한다.
- `InGameUIStyleSpec.md v0.9.0`과 `InGameUIWeaponPanelSpec.md v0.5.0`이 현재 D1-05 수치 SSOT다.
- 실제 Font·Icon과 정적 캡처는 아직 미확정이므로 UI-DESIGN-GATE PASS로 기록하지 않는다.

### v0.20.0 적용 안내

- 다음 UI 디자인 리뷰는 `InGameUIStyleSpec.md v0.8.0`과 `InGameUIWeaponPanelSpec.md v0.4.0`을 우선 사용한다.
- v0.8.0·v0.4.0의 수치는 Detailed Draft이므로 사용자 승인 전 기존 Blueprint·C++·Data Asset의 구현값으로 고정하거나 에셋을 일괄 변경하지 않는다.
- Compact·Standard·Expanded Density는 같은 View Data 의미를 유지하고 Padding·Gap·행 높이·Caption 밀도만 바꾸는 계약으로 검토한다.
- 미지원 정보는 `Collapsed`, `Unknown`·`Unavailable`·`KnownZero`는 의미가 있는 행 상태로 구분한다.
- WeaponPanel의 Compact 후보는 360 Preferred Width, 142~178 Selected Height, 112×68 Tile이며 `464×360`은 계속 Maximum Slot 경계다.
- 실제 정적 검토 전에는 `UI-DESIGN-GATE PASS`로 기록하지 않는다.

### v0.19.0 적용 안내

- 신규 또는 수정되는 모든 CarFight UI는 `InGameUIDesign.md v0.13.0`의 전체 UI 커스터마이징 불변 계약과 `InGameUIStyleSpec.md v0.7.0`을 함께 적용한다.
- 개별 Widget에 직접 고정된 색상·폰트·Padding·Position은 즉시 일괄 삭제하지 않지만 신규 변경부터 Style·Layout·Density Data 또는 명시적 Override로 이동한다.
- 숨긴 항목이 공간을 남기는 구조를 신규 기본값으로 사용하지 않고 `Collapsed`와 자동 Layout을 우선한다.
- WeaponPanel의 기존 `432×224` Card·`136×96` Tile은 구현 불변값이 아니며 Compact Static Review에서 기본 Token을 확정한다.
- `ConceptArt/CFWeaponPanel_1440p.xml`은 초기 넓은 배치 참고 자료로 유지하며 Compact Revision의 최종 Pixel SSOT로 사용하지 않는다.
- P0에서 사용자 UI 편집 화면·저장은 구현하지 않지만 PanelId·SlotId·Profile Version과 User Override 추가를 막는 구조를 만들지 않는다.

### v0.18.0 적용 안내

- `InGameUIWeaponPanelSpec.md v0.2.0`과 `ConceptArt/CFWeaponPanel_1440p.xml`을 D1-WEAPON-PANEL의 승인된 Visual Layout 기준으로 사용한다.
- 승인된 구조는 향후 View Data·Presenter·UMG 구현의 입력 계약이지만 현재 Source·Unreal Asset 구현 완료를 의미하지 않는다.
- 첫 Runtime 연결에서 다중 무기 Provider가 없으면 Selected Card만 표시하고 Compact Rail은 숨긴다.
- Ammo·Charge·Heat·Reload Runtime이 없을 때 설정값으로 현재 상태를 추정하지 않는다.
- VehiclePanel `896×416`, WeaponPanel `464×360`, Radar 위치와 중앙 Reticle Protection 영역을 변경하지 않는다.
- UI-DESIGN-GATE는 Style Token·Base Widget·Font·Icon과 해상도 Static Review 완료 전까지 PASS로 기록하지 않는다.

### v0.17.0 적용 안내

- WeaponPanel 상세 검토는 `InGameUIWeaponPanelSpec.md`와 `ConceptArt/CFWeaponPanel_1440p.xml`을 우선한다.
- 기존 전체 HUD Wireframe의 WeaponPanel 내부 예시는 위치 확인용 Placeholder로 해석한다.
- v0.18.0부터 `D1-WEAPON-PANEL Visual Layout Accepted`를 사용하되, Source·Unreal Asset 구현과 해상도 검증은 별도 단계로 유지한다.
- 현재 다중 무기 Provider가 없으므로 첫 Runtime 연결에서는 Selected Card만 표시하고 Compact Rail은 숨긴다.
- Ammo·Charge·Heat·Reload Runtime이 없을 때 설정값으로 현재 상태를 추정하지 않고 해당 채널을 숨긴다.
- 승인된 VehiclePanel `896×416`, Radar 위치와 중앙 Reticle Protection 영역은 변경하지 않는다.

### v0.12.0 적용 안내

- 신규 HUD와 메뉴 시각 설계는 `차량 탑재형 전술 인터페이스`와 C형 70:20:10 균형을 기본값으로 사용한다.
- 조준·락온·Radar는 국소적으로 전술성을 강화할 수 있지만 전체 UI를 군용 HUD 중심으로 변경하지 않는다.
- Visual Direction Accepted는 UI-DESIGN-GATE PASS와 다르며 Style Data·Base Widget·Wireframe·해상도 검토를 완료해야 한다.
- CommonUI 도입 여부는 별도 `UI-COMMONUI-GATE`가 계속 소유한다.

### v0.10.0 적용 안내

- 시각 콘셉트 검토는 `InGameUIVisualConcept.md`를 기준으로 수행한다.
- 현재 문서는 Working Draft이므로 기존 HUD와 Pause UI 에셋에 일괄 적용하지 않는다.
- 사용자 승인 전에는 `UI-DESIGN-GATE PASS`, 최종 Palette 또는 공통 Style Asset 완료로 기록하지 않는다.
- 승인 후 상위 `14_CombatUI.md`와 DecisionLog에 Accepted 시각 방향을 동기화한다.
- UI-P0-03~05 데이터·수명 작업은 기존 시각을 보존한 채 진행할 수 있지만 UI-P0-06 신규 HUD 제작 전에는 승인된 콘셉트와 Style 계약이 필요하다.

### v0.9.0 적용 안내

- UI 디자인 개선은 CommonUI 결정과 무관하게 `UI-DESIGN-GATE`에서 진행한다.
- UI-P0-03~05는 데이터와 기존 HUD 수명 이전을 우선하며, UI-P0-06 신규 차량·무기 HUD 시각 구현 전에 Style Data와 Base Widget 기준을 확정한다.
- CommonUI는 현재 플러그인·모듈 의존성을 추가하지 않으며 첫 전체 화면 Flow 전에 `UI-COMMONUI-GATE`를 별도로 수행한다.
- CommonUI를 도입하더라도 상시 전투 HUD의 일반 UMG 사용, View Data·Presenter와 UCFUISubsystem의 공개 진입점은 유지한다.
- 디자인 품질 부족을 CommonUI 미도입 탓으로 처리하거나, 외형 개선만을 이유로 CommonUI를 도입하지 않는다.

### v0.8.0 적용 안내

- UI Root는 현재 싱글플레이 기준에서 Legacy Pawn HUD와 같은 `AddToViewport` 계층에 ZOrder 100으로 등록한다.
- Pause Menu의 Enter 확인은 버튼 Focus에만 의존하지 않고 PlayerController Fallback을 유지한다.
- UI-P0-02 전체 PASS로 판정하기 전 Gamepad, 입력 유지 잔류, Launcher·Projectile·Timer와 Root 수명 사용자 PIE를 반드시 완료한다.
- 남은 UI-P0-02 사용자 PIE가 PASS하기 전에는 UI-P0-03 코드를 착수하지 않는다.

### v0.7.0 적용 안내

- 새 세션은 UI-P0-02 사용자 PIE부터 시작하며 UI-P0-03 코드를 먼저 착수하지 않는다.
- UI-P0-02가 PASS하면 Vehicle·Weapon·Defense·Target·Radar·Alert View Data 계약으로 진행한다.
- Defense HUD는 `VehicleDefenseComp`와 `VehicleHealthComp`를 읽기 전용으로 소비하고 Shield·Armor·관통·Integrity를 재계산하지 않는다.
- `CF-FQ-033`은 Done이므로 데미지 Plan으로 Active를 되돌리지 않으며 영향받은 회귀가 있을 때만 관련 PIE를 재실행한다.

- UI-P0-03 이후 전투 HUD와 전투 명령은 `bVehicleCombatRuntimeReady`를 사용하고, 차량 기본 주행·피격 가능 여부는 `bVehicleCoreRuntimeReady`를 사용한다.
- 기존 `bVehicleRuntimeReady`는 Blueprint·Tick·Debug 호환을 위해 Core 준비값과 동일하게 유지한다.
- Controller `GameplayInputMappingContext`는 Pawn의 자동 Gameplay Context 등록을 같은 변경에서 비활성화하기 전까지 `None`으로 유지한다.
- Shield·Armor View Data는 `UCFVehicleDefenseComp` 실제 상태를 사용하며 부품 손상이나 미구현 자원에만 `Unavailable`을 사용한다.
- 기존 Pawn 소유 AimReticle은 UI-P0-04 전까지 Current System으로 유지한다.
- 기존 Pawn 소유 TargetSelect HUD는 UI-P0-05 전까지 Current System으로 유지한다.
- 새 Root 구현 시 기존 Widget을 즉시 삭제하지 않고 `LegacyPawnOwned`와 `UISubsystemOwned` 전환 게이트를 사용한다.
- Pawn의 기존 Mapping Context 등록 경로는 UI-P0-01A에서 제거·복원 수명 계약을 추가하기 전까지 임의 삭제하지 않는다.
- `UCFUISubsystem`은 LocalPlayer와 함께 유지되더라도 이전 World의 Root, Pawn과 Component 구독을 보존하지 않는다.
- Shield, Radar, Knowledge와 Lock-on Mock은 UI 검증용이며 실제 게임플레이 구현 완료 증거가 아니다.
- Screen Layer와 공통 UI 타입을 준비하더라도 피팅 기능 구현 완료로 해석하지 않는다.
- Loadout, 피팅 계산, 저장, Garage Preview와 Fitting Widget은 별도 피팅 Plan이 생성되기 전까지 `CF-FQ-032`에 추가하지 않는다.
