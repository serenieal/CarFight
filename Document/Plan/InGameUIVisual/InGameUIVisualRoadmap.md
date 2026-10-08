# CarFight InGame UI Visual Rework Roadmap

- 문서 버전: v0.1.27
- 작성일: 2026-08-23
- 최근 갱신일: 2026-08-26
- 문서 상태: Active
- 기능 ID: `CF-FQ-039`
- 대표 Plan: `InGameUIVisualPlan.md v0.1.28`
- 현재 단계: `VPR-P0-01 VehiclePanel Production Vertical Slice — Frame Master v5 Outer-Shell-Only Candidate Ready / v4 USER Reject / AI Source QA PASS / USER Visual PASS Pending / UE Import 0 / Content Reposition 0`

---

## 1. 목적

이 Roadmap은 `CF-FQ-039 Production UI Visual Rework`의 실행 순서와 Gate만 소유한다. 상세 Visual 규칙은 `InGameUIVisualPlan.md`, Texture/Material 제작 규격은 `InGameUIHUDArtSpec.md`, 현재 구현 사실은 main_game `Systems/UI/InGameUI.md`를 우선한다.

완료된 `CF-FQ-032` 기능·Build·Automation evidence를 단계별로 다시 복제하지 않는다.

---

## 2. 전체 실행 순서

```text
VPR-P0-00  Visual Target Recovery & Registry
    ↓
VPR-P0-01  VehiclePanel Production Vertical Slice
    ↓
VPR-P0-02  WeaponPanel Production Vertical Slice
    ↓
VPR-P0-03  Pause Menu Visual Rebuild
    ↓
VPR-P1-04  TargetPanel Production Visual
    ↓
VPR-P1-05  Radar Production Visual
    ↓
VPR-P1-06  AimReticle + Target Marker Visual Family
    ↓
VPR-P1-07  AlertFeed Visual + Animation
    ↓
VPR-P2-08  HUD Global Consistency / Resolution Review
    ↓
VPR-P2-09  Final Acceptance / Systems Promotion
```

MissionPanel production visual은 실제 Mission runtime owner 확정 전 이 Roadmap blocker가 아니다.

---

## 3. 공통 Gate — Target Decomposition Pipeline LOCKED

각 Visual Slice는 아래 순서를 **건너뛰거나 역전하지 않는다.**

```text
A. Production Visual Target Lock
B. Logical Breakdown
   - Static Art / Dynamic Visual / Runtime Data
   - Frame / Module / Icon / Typography / Decoration
   - reusable Widget boundary / 표현 수단
C. WBP Composition Skeleton
   - Target 비율·Hierarchy를 보존하는 Placeholder Slot만 구성
D. Slot Contract Review
   - Bounds / Anchor / Aspect / Crop / 9-Slice / Mask / Dynamic Fill
E. Physical Art Breakdown / Production
   - 동일 Master Target에서 Texture/Mask/Frame/Icon 파츠 제작
F. UMG Assembly
G. Runtime Binding 보존 확인
H. Technical / Pixel Validation
I. Visual Match Review
J. USER Visual PASS
```

핵심 강제 순서는 **`Target Lock → Logical Breakdown → WBP Skeleton → Physical Art Breakdown → Assembly`**다. `Logical Breakdown` 전에 WBP 최종 구조/미감을 먼저 확정하지 않고, `Slot Contract Review` 전에 PNG를 전부 물리 분해하지 않는다.

다음 구형 경로는 Deprecated다.

```text
WBP/Primitive-first → 나중에 Concept을 후맞춤
Raw Border/ProgressBar styling → Production Visual 승인
AI 신규 미감 생성 → 곧바로 Production Asset Import
P1/P2/현재 Scaffold → Master Target 역승격
```

기존 구형 결과는 Runtime/Technical Scaffold evidence로는 보존할 수 있지만 다음 Art의 Visual authority가 아니다. Historical 문서의 old 제작 절차도 이 Gate를 되돌리지 않는다.

단, 새 Art를 만들지 않는 **Structure Readiness**는 승인된 Composition Target으로 먼저 수행할 수 있다. 허용 범위는 현재 Designer-owned reusable structure의 존재·편집성 확인과 중복 구조 제거 판단까지이며, Texture/Material 외형 변경·Production Asset 교체·최종 Visual Match 주장은 포함하지 않는다.

Visual Match에서 Target과 의도적으로 달라져야 하는 경우 `Deviation`을 기록하고 USER 승인을 받은 뒤 다음 Gate로 이동한다.

---

## 4. VPR-P0-00 — Visual Target Recovery & Registry

상태: **COMPLETE — VT-VEH-01 USER APPROVED / LOCKED**

목적:
- 과거 대화·저장소·SourceArt에 존재하는 UI Concept 후보를 회수한다.
- 사용자가 실제로 `이 모양으로 가자`고 승인한 이미지와 단순 참고 이미지를 구분한다.
- 첫 Production Slice에 사용할 Current Visual Target을 고정한다.

작업:

```text
1. Repository / SourceArt의 실제 파일부터 후보 목록화
2. 접근 가능한 대화·파일 source에서 추가 후보 회수
3. Vehicle / Weapon / Radar / Target / Reticle / Pause Scope로 분류
4. 당시 USER 결정 문구와 승인 범위를 교차검증
5. Direction Reference / Composition Target / Production Visual Target로 분류
6. Target ID와 Must Match / Flexible 속성 기록
7. VehiclePanel Current Production Visual Target 1개 선택
8. 원본을 회수하지 못한 이미지는 기억으로 재생성하지 않고 필요 시 USER 재제공 대상으로 남김
```

완료 조건:
- AI Recovery: 접근 가능한 저장소·문서·기존 결정 기록에서 승인 범위 분류 완료
- USER Gate: 전체 VehiclePanel `VT-VEH-01` 확정 — **PASS / LOCKED 2026-08-24**
- 존재하지 않는 저장소 경로 주장 0
- Direction/Composition/Production 승인 범위 혼동 0
- 원본 미확인 기억 기반 Target 재생성 0

AI Recovery는 완료됐다. 2026-08-24에는 External Image Vision으로 저장소의 과거 D1 VehiclePanel 이미지를 직접 확인해 Historical Reference로 분류했고, 신규 Concept USER 피드백으로 Speed/RPM·Shield/Integrity 방향과 `탑다운 좌향 차량 + 6개 공통 Armor Sector + Designer 자유 재배치` composition을 확인했다.

Structure Readiness 감사 결과, 현재 persisted `WBP_CFArmorBodyMap`은 이미 `CanvasPanel_Root + Image_VehicleSilhouette + WBP_CFArmorSector 6개`이며 각 Sector가 독립 Canvas Slot이다. `WBP_CFArmorSector` 자체도 `Image_ArmorPlate + Text_Direction + ProgressBar_Armor` 공통 구조다. 따라서 별도 `WBP_CFArmorSlot`을 생성하지 않는다. `SourceArt/UI/HUD/P2/T_UI_VehSil_LF.png`도 현재 탑다운 실루엣으로 확인됐다.

Source/Asset mutation: 없음. 현재 구조 확인과 문서 동기화만 수행했다.

---

## 5. VPR-P0-01 — VehiclePanel Production Vertical Slice

상태: **IN PROGRESS — ARMOR MODULAR RUNTIME/WBP TECHNICAL PASS / COMMON PLATE + DIRECTION 2-ICON + VEHICLE-SPECIFIC SILHOUETTE SOURCE/UE ASSETIZATION PASS / DEFENSE SOURCE FAMILY NEXT / MASTER SOURCE BINDING PENDING**

보호:

```text
- Speed/RPM/Gear Provider/Presenter 계약 유지
- Image_RPMGauge + M_UI_RPMGauge + RPMRatio 유지
- WBP_CFArmorSector 6개 재사용 유지 — canonical common Armor slot
- WBP_CFArmorBodyMap의 Canvas 기반 독립 Sector 배치 유지
- 중앙 VehicleSilhouette는 탑다운 + 좌향 의미 유지
- Designer-owned layout 유지
```

Structure Readiness PASS:

```text
- WBP_CFArmorBodyMap persisted root = CanvasPanel_Root
- Image_VehicleSilhouette + WBP_ArmorFront/Right/Rear/Left/Top/Bottom 존재
- 6개 Armor는 모두 같은 WBP_CFArmorSector_C 재사용
- 각 Armor Sector는 독립 CanvasPanelSlot이라 USER Designer 재배치 가능
- WBP_CFArmorSector 내부 = Image_ArmorPlate + Text_Direction + ProgressBar_Armor
- 현재 SourceArt VehicleSilhouette = 탑다운 형태
- 신규 WBP_CFArmorSlot 생성 불필요 / 중복 금지
```

Armor Technical PASS:

```text
- 기존 P2 Armor 방향 Texture 6종 재사용 / 신규 Content Asset 0
- 방향 Texture 존재 시 Text_Direction 숨김, Texture 누락 시 fallback Label
- Armor Ratio >0.60 = Armor / >0.30 = Caution / <=0.30 = Critical tint
- Plate + ProgressBar_Armor가 동일 Presentation 상태색 소비
- 0% = Critical tint + empty Bar; Broken X/갈라진 Plate Art는 후속 Visual refinement
- Official Editor Build final c8f67e0d88f647db8c4de7bba485be91 PASS
- ArmorSectorProductionVisualContract 1/1 PASS
- DefenseRuntimeViewData 1/1 PASS
- RpmGaugeVisualBindingContract 1/1 PASS
```

Armor USER PASS:

```text
- USER가 6-direction icon family와 현재 배치/표시를 확인
- 등록된 Direction Text는 오류로 누락된 것이 아니라 Texture 존재 시 의도적으로 숨기는 fallback 구조임을 확인
- icon-first / Text fallback 동작이 USER 의도와 일치한다고 명시 PASS
- 이 PASS는 Armor visual slice에만 적용하며 전체 VehiclePanel VT-VEH-01 승인으로 확대하지 않음
```

Frame + Shield/Integrity Technical/Pixel PASS:

```text
- 기존 DA_CFHUDVisual_Default.VehiclePanelFrame → Border_Surface 9-Slice Box 실제 소비 확인
- 신규 Frame/Bar Texture Content Asset 0
- 기존 ProgressBar_Shield/Integrity Widget·Designer Slot·Runtime Ratio source 유지
- SurfaceRaised dark track + 2px inset semantic fill
- Shield token → Shield Bar/Icon/Value
- Integrity token → Integrity Bar/Icon/Value
- LeftToRight + Scale + non-marquee
- CFHUDPresenter v1.25.1: Track/Fill/Marquee Brush intrinsic height 10px
- CFHUDDataTests v1.34.2: transient DesiredSize 대신 3개 Brush intrinsic height 직접 검증
- Defense Bar pixel-fix Build d9c33e2a689e42b2a511698fa021df4f PASS
- test-harness final Build 87d13671b2aa466bac88a820d5eda72b PASS
- VehicleDefenseBarVisualContract final 1/1 PASS
- M_VehicleDefensePIE Defense Pawn: SHIELD 100/100 591x10 + INTEGRITY 100/100 591x10 동시 실제 렌더 확인
- Frame / Speed-RPM / Armor 보존 확인
- DefenseRuntimeViewData 1/1 PASS
- RpmGaugeVisualBindingContract 1/1 PASS
- Armor USER PASS replay 0
```

주요 Breakdown 후보:

```text
Vehicle Panel Frame
Speed/RPM Track + Mask + Glow
Speed/Gear Typography treatment
Vehicle Silhouette
Armor Plate Family 6종
Armor Ratio visual
Shield / Integrity frame/fill treatment
```

완료 조건:
- Vehicle Target 대비 Visual Match USER PASS
- 기존 runtime data path 비회귀
- 기존 Designer 배치가 코드 scaffold로 덮이지 않음

---

## 6. VPR-P0-02 — WeaponPanel Production Vertical Slice

상태: **NOT STARTED**

보호:

```text
- Primary / Secondary Resource 의미 슬롯 유지
- Ammo / Reload / Cooldown / Heat / Charge resolver 유지
- Weapon Rail actual runtime truth 유지
- fake selectable weapon 금지
```

주요 Breakdown 후보:

```text
Weapon Header Frame
Weapon Icon Slot Visual — 실제 per-weapon player-facing Icon Source 연결은 별도 source가 생길 때까지 Deferred
Primary Resource visual
Secondary Resource visual
Reload/Cooldown/Heat/Charge state material
Weapon Rail Card
Selection / Focus frame
```

완료 조건:
- 기능 슬롯 수를 아트 편의를 위해 임의 축소하지 않음
- 실제 source가 없는 per-weapon Icon identity를 Semantic Icon·Asset 이름·내부 ID로 위조하지 않음
- Current Target Visual Match USER PASS

---

## 7. VPR-P0-03 — Pause Menu Visual Rebuild

상태: **NOT STARTED**

현재 판정:
- `UCFPauseMenuWidget`의 C++ WidgetTree Visual은 Prototype/Technical Foundation이다.

보호:

```text
Continue 요청 이벤트
Focus 동작
Pause lifetime / Menu Layer 계약
```

변경 방향:

```text
Primary Visual
→ Designer-owned WBP_CFPauseMenu

Safe Fallback
→ 기존 UCFPauseMenuWidget Native Tree 유지
```

기존 `InGameUIAssetizationSpec.md §9 D1-12`의 안전한 migration을 재사용한다. `WBP_CFPauseMenu`는 `UCFPauseMenuWidget` 파생으로 만들고, UISubsystem이 Config Soft Class를 1회 Resolve한다. Designer Tree/Bind 실패 시 기존 Native Pause Menu로 fail-safe한다.

완료 조건:
- 버튼/프레임/배경/타이포그래피를 WBP Designer가 소유
- Continue/Focus/Pause lifetime은 기존 C++ 계약 유지
- WBP Class/Designer binding 실패 시 Native Fallback 동작
- Pause 기능 회귀 없음
- USER Visual PASS

---

## 8. VPR-P1-04 — TargetPanel Production Visual

상태: **NOT STARTED**

핵심:
- Tracking / Identification / Knowledge의 정보 단계가 한눈에 구분돼야 한다.
- `Unknown`을 숨기지 않고 시각적으로 설계한다.
- Target runtime 의미는 Provider가 소유한다.

후보:

```text
Target Frame
Relation accent
Identification reveal treatment
Scan material/progress
Armor/Defense knowledge presentation
Lost/Destroyed state treatment
```

---

## 9. VPR-P1-05 — Radar Production Visual

상태: **NOT STARTED**

보호:

```text
Heading-Up
runtime Contact positioning
Range/Zoom 의미
relation Image contact
selected range-out edge 의미
```

후보:

```text
Radar Frame
3 Range Rings / Grid
Scan Sweep Material
Friendly / Hostile / Unknown visual family
Player marker
Selected / Edge bracket
Contact acquire/lost animation

Deferred — 이 Visual Feature에서 새 Runtime Source를 만들지 않음:
- Radar Altitude/Z presentation: 현재 HUD Radar ViewData source 없음
```

Radar 위치를 Designer 고정 좌표로 바꾸거나 runtime spatial 계산을 제거하지 않는다.

---

## 10. VPR-P1-06 — AimReticle + Target Marker Visual Family

상태: **NOT STARTED**

보호:

```text
AimReticle state resolver
Weapon Reticle world projection
TargetSelect world projection
Screen-edge safe-region projection
IFF relation meaning
```

핵심:
- 현재 CenterDot + 4 Bracket 기능 구조가 최종 Art를 대신하지 않도록 한다.
- Main Reticle / Turret Reticle / Selected Target / Edge Marker가 같은 Visual Family로 읽혀야 한다.
- 개발/진단용 텍스트가 Production 전투 화면보다 앞서지 않도록 정리한다.

---

## 11. VPR-P1-07 — AlertFeed Visual + Animation

상태: **NOT STARTED**

보호:
- AlertKey, priority, duration, suppression lifecycle 유지.

후보:

```text
Severity Frame
Warning/Critical Icon Family
Slide/Fade
Pulse
Critical accent
compact stacking
```

오디오가 없는 프로젝트이므로 Critical 상태는 시각만으로 충분히 인식돼야 한다.

---

## 12. VPR-P2-08 — Global Consistency / Resolution Review

상태: **NOT STARTED**

검증:

```text
- 전체 HUD가 하나의 Visual Language로 보이는가
- Vehicle / Weapon / Target / Radar의 정보 위계가 충돌하지 않는가
- 중앙 전투 시야를 침범하지 않는가
- 1920×1080 기본 Visual Match
- 필요한 16:9 / 21:9 / 32:9 safe layout
- DPI / text readability
- hostile/friendly/unknown contrast
- 장식이 정보보다 먼저 보이지 않는가
```

Visual tuning 수치는 가능한 Style/HUD Visual Data/Material Parameter가 소유하고 Presentation 미감을 C++ 상수에 새로 고정하지 않는다.

각 Slice에서 Designer Layout 또는 Desired Size를 변경한 경우 해당 Slice 종료 전에 기존 ResolutionLayoutContract 영향 범위 smoke를 수행한다. P2-08은 이를 대신하는 첫 검사가 아니라 전체 HUD의 최종 통합 Visual/Resolution Review다.

---

## 13. VPR-P2-09 — Final Acceptance / Systems Promotion

상태: **NOT STARTED**

완료 Gate:

```text
1. P0/P1 Visual Slice USER PASS
2. 영향 범위 Build / Automation / persisted evidence PASS
3. CF-FQ-032 완료 evidence와 새 Feature evidence 경계 유지
4. Systems/UI/InGameUI에 최종 Production Visual ownership 계약 승격
5. FeatureQueue / ActiveWork / Plan Index closure
6. 상세 Plan은 Historical 전환
```

---

## 14. 반복 금지 / 비범위

```text
- CF-FQ-032 완료 Build/Automation을 계획 착수만으로 replay하지 않는다.
- Provider/Presenter/ViewData를 Visual 재작업이라는 이유로 재설계하지 않는다.
- Runtime spatial UI를 정적 Designer 좌표로 바꾸지 않는다.
- 기존 Designer Layout을 full scaffold rebuild로 덮지 않는다.
- 승인 Target이 있는데 구현 편의를 이유로 별도 저품질 대체안을 자동 채택하지 않는다.
- MissionPanel production visual은 이번 핵심 경로에서 제외한다.
```

---

## 15. Current Resume Checkpoint

```text
Feature: CF-FQ-039
Plan: InGameUIVisualPlan.md v0.1.28
Roadmap: InGameUIVisualRoadmap.md v0.1.27
Current: VPR-P0-01 — Frame Master v5 Outer-Shell-Only Candidate Ready / v4 USER Reject / AI Source QA PASS / USER Visual PASS Pending / UE Import 0 / Content Reposition 0
Structure readiness: PASS — existing WBP_CFArmorSector 6 reuse + BodyMap Canvas + top-down silhouette
Armor modular runtime/WBP: TECHNICAL PASS — VehicleData별 silhouette binding + additive Image_DirectionIcon migration / BodyMap six-sector Designer Slot 보존
Frame/Defense Bars: Technical/Pixel PASS는 Runtime/Scaffold evidence로만 보존 — Production Visual authority 아님
Production method: Target Decomposition Pipeline LOCKED / WBP-first·Primitive-first·AI replacement art Deprecated
Production Target: `VT-VEH-01` USER APPROVED + LOCKED / repository canonical `SourceArt/UI/HUD/VT/VT_VehPanel01.jpg` 1280×594 / SHA-256 `cf5b0a57...df6b6b` binding PASS
WBP Skeleton: VehiclePanel Canvas composition + existing SpeedGauge/ArmorBodyMap/ArmorSector 보존 + 신규 reusable DefenseBar boundary 기본안
Slot Contract: PASS / FRAME REBASE — canonical 1280×594 → 896×416 scale 0.7 / root 9-Slice underlay + full-aspect Frame Master overlay / upper 422+12+422 50:50 / lower Defense deck hierarchy / icon-only Armor / RPM label only / atomic DefenseBar migration
Physical Art Breakdown: READY / REVISED — Frame/RPM 계약 보존 + Armor modular common Plate/2-icon/vehicle-specific silhouette catalog + Defense frame/mask/pattern/material 후속
Historical Frame Extraction: USER PASS — 과거 1536×713 F1~F4 / FrameReview_v2는 extraction 품질 evidence로 보존. 2026-08-26 current P2 단일 octagonal frame과 canonical Master의 상위 hierarchy 불일치가 새 related failure로 확인되어 Current Frame Master만 재개
Frame Master v4: USER REJECT — 내부 safe-area box / central seam / lower deck rectangle가 임시 layout geometry인데 Frame Source에 남아 reference와 불일치. Production authority 아님
Frame Master v5: AI SOURCE QA PASS / USER PENDING — `VT06_FrameMaster_v5.png` 896×416 SHA `e560caaf...a59a5` + `Review/VT06_FrameReview_v5.png` 1832×860 SHA `45b18df6...8d306`; outer shell/corner/top rail/lower outer base만 소유하고 internal temporary geometry 0
RPM-S1: USER PASS — 724×467 / 21 independent Tick / 21 Progress levels / Red 4 Tick / Ratio 0·0.25·0.50·0.85·1.00 = 0·5·10·17·21 / AUTO QA PASS + USER Visual PASS
Armor Modular Source: TECHNICAL PASS — common Plate + `Arrow/Chevron2` 2-icon repository Source + UE Production Texture + HUD Visual Data binding 완료 / old VT03 6-direction pack Superseded
RPM Production Candidate: `T_UI_RPMMaskPack` RGBA 1장 — R Progress / G Tick21 / B GuideRail / A 255 opaque / Red는 R+`RedStartRatio=0.85`에서 파생 / existing `M_UI_RPMGauge + RPMRatio` 유지
Source Binding: PASS for current Frame input — canonical `VT-VEH-01` 1280×594 repository source + exact hash 확인. VT04 Armor common/2-icon, VT05 Sedan/SUV silhouette와 Defense Production Source/Asset evidence도 기존 PASS 보존
Silhouette Authority/Production: PASS — `DA_TestSedan → Sedan.Sedan → T_UI_VehSil_Sedan`, `DA_TestSUV → SUV.SUV → T_UI_VehSil_SUV`, `DA_VehicleDefense_TestSUV → SUV.SUV → T_UI_VehSil_SUV`; persisted 3 VehicleData / 2 unique ChassisMesh / 2 Production Texture / catalog 3 entry
Next exact output: USER가 `SourceArt/UI/HUD/P2/Review/VT06_FrameReview_v5.png`를 검토해 outer-shell-only Frame Master의 외곽 shell / corner / top rail / lower outer base를 판정한다. PASS 전에는 UE Import·`Image_FrameOverlay` assembly·subordinate local frame 조립·RPM/Speed/Gear/Silhouette/Armor 재배치를 시작하지 않는다.
Editor: 현재 closure에 live Editor 재기동 불필요 / persisted AssetDump로 final structure 검증
Build: Armor modular final ea1918cf88b0470e91398e9a87840e6b PASS / Direction 2-Icon assetization 8eed02112b73443a9c4df1d03d9a859b PASS / common Plate assetization 3183bb86d5e7483e9cde64be70c27551 PASS / canonical BuildEditor + CFVehSilProd process f7c50e97f58f4ef4bd0e0a64d51474e6 Exit 0 PASS
Automation: CarFight.UI.UI_P0_06 17/17 PASS / Fail 0 / silhouette catalog 전용 focused Automation은 현재 Source에 없음
Content mutation: WBP_CFArmorSector exact package에 Image_DirectionIcon additive 1건 / WBP_CFArmorBodyMap 0 / Production Armor Texture 3종(Plate/Arrow/Chevron2) + Vehicle silhouette Texture 2종(Sedan/SUV) + DA binding / VehicleSilhouettes count 3
```

---

## 16. Changelog

### v0.1.27 - 2026-08-26

- USER 피드백으로 v4를 내부 임시 box/seam/deck guide 잔존 Visual failure로 Reject 처리했다.
- Frame Master ownership을 outer shell/corner/top rail/lower outer base로 축소하고 safe-area geometry는 layout contract에만 남기는 Plan v0.1.28 규칙을 projection했다.
- v5 `VT06_FrameMaster_v5.png` / `VT06_FrameReview_v5.png`를 AI Source QA PASS / USER Pending으로 전진했다.
- 다음 exact gate는 v5 USER Visual Review이며 UE Import와 내부 content/local-frame 재배치는 계속 0이다.

### v0.1.26 - 2026-08-26

- canonical target에서 deterministic Frame Master v1~v4를 생성·AI QA하고 v1~v3의 Armor/local-frame contamination을 USER 후보에서 제외했다.
- v4는 master-level source ownership을 outer shell/corner/top rail/lower base로 좁히고 central seam/main-deck separator를 deterministic cool line으로 분리해 AI Source QA PASS했다.
- `VT06_FrameMaster_v4.png` / `VT06_FrameReview_v4.png` / `VT06_FrameManifest_v4.json`을 Pre-USER-PASS candidate로 projection하고 다음 exact gate를 USER Frame Master Visual Review로 전진했다.
- UE Import, WBP/Designer mutation, RPM/Speed/Gear/Silhouette/Armor reposition은 0으로 유지한다.

### v0.1.25 - 2026-08-26

- 대표 Plan v0.1.26의 Frame Master Reset을 Current 실행 순서로 projection했다.
- current P2 9-Slice frame과 canonical `VT-VEH-01` 직접 비교에서 확인된 상위 Frame hierarchy 불일치를 새 related Visual failure로 반영했다. 과거 FrameReview_v2 PASS는 Historical extraction evidence로 보존한다.
- Current 좌표계를 repository canonical 1280×594 → 896×416 scale 0.7로 교체하고 Master source binding Pending 문구를 제거했다.
- 다음 exact output을 Defense/RPM이 아니라 Frame Master source/review → USER PASS → `Image_FrameOverlay` assembly로 변경했다. RPM/Speed/Gear/Silhouette/Armor 재배치는 그 뒤로 Deferred했다.

### v0.1.24 - 2026-08-25

- authority ChassisMesh 기반 VT05 Sedan/SUV 512×256 transparent PNG Source를 생성·검증하고 USER 진행 승인 뒤 `T_UI_VehSil_Sedan` / `T_UI_VehSil_SUV` Production Texture로 Assetize했다.
- persisted `DA_CFHUDVisual_Default.VehicleSilhouettes` exact 3 entry를 fresh AssetDump로 확인했다. Sedan은 Sedan Texture, 두 SUV VehicleData는 동일 SUV Texture를 공유하고 legacy fallback은 보존한다.
- canonical BuildEditor + `CFVehSilProd` process `f7c50e97f58f4ef4bd0e0a64d51474e6` Exit 0을 Technical evidence로 고정했다.
- 다음 exact output을 Defense common frame/mask/hex/chevron + `M_UI_DefFill` Source/Production family로 이동하고 exact `VT-VEH-01` Master Source binding은 별도 Pending으로 유지했다.

### v0.1.23 - 2026-08-25

- persisted VehicleData 3종과 ChassisMesh identity를 재확인해 `DA_TestSedan → Sedan.Sedan`, `DA_TestSUV → SUV.SUV`, `DA_VehicleDefense_TestSUV → SUV.SUV` mapping을 고정했다.
- vehicle-specific silhouette Source Authority / identity mapping을 PASS로 닫고 현재 형상 cardinality를 3 VehicleData / 2 unique ChassisMesh로 확정했다. P2 generic silhouette와 VT03 후보는 특정 차량 Production authority가 아니다.
- next exact output을 Sedan/SUV mesh-derived Source Candidate 2종 제작 + USER Visual Review로 전진했다. 승인 전 `VehicleSilhouettes` count 0, Import/Bind 0, exact Master Source Pending을 유지한다.

### v0.1.22 - 2026-08-25

- VT04 common Armor Plate를 repository Source → `T_UI_ArmorPlate` → `DA_CFHUDVisual_Default.ArmorCommonPlate`까지 Technical PASS로 닫았다.
- 다음 exact output을 vehicle-specific silhouette Source Authority / VehicleData identity mapping으로 좁혔다. `VehicleSilhouettes` count 0은 exact identity가 확정될 때까지 유지한다.
- Direction 2-Icon, common Plate, Armor migration, RPM-S1, Frame Family는 새 관련 failure 없이 반복하지 않는다. Master Source와 USER Visual은 Pending이다.

### v0.1.21 - 2026-08-25

- Armor Direction Arrow/Chevron2 2종을 repository Source와 UE Production Texture로 실제 binding하고 persisted DataAsset 연결까지 Technical PASS로 닫았다.
- 다음 Gate를 common Plate + first vehicle-specific silhouette Production Art binding으로 좁히고, exact VT-VEH-01 Master Source binding과 USER Visual은 별도 Pending으로 유지했다.

### v0.1.20 - 2026-08-25

- Armor modular VehicleData별 silhouette runtime binding과 `WBP_CFArmorSector.Image_DirectionIcon` additive migration을 Technical PASS로 전진했다. BodyMap six-sector Designer Slot은 그대로 보존했다.
- clean migration Exit 0 / save0, fresh AssetDump, final Official Build와 `CarFight.UI.UI_P0_06` 17/17 PASS를 현재 checkpoint에 반영했다.
- 다음 Gate를 Source Binding + common Plate/2-icon/차량별 silhouette Production Art Import·Binding → Technical/Pixel → USER Visual로 이동했다. 실제 Art와 USER Visual은 아직 Pending이다.

### v0.1.19 - 2026-08-25

- USER가 ARMOR-S1 검토 전에 Production 구조를 재정의했다. Vehicle silhouette는 `VehicleData` identity별로 교체하고, 방향 Art는 common Plate + canonical `→`의 `Arrow/Chevron2` 2종을 각 Sector에서 Designer rotation으로 재사용한다.
- 6개 `WBP_CFArmorSector`의 화면 Position/Size/Icon 종류/rotation을 Designer ownership으로 고정했다. Runtime 방향 의미는 기존 instance name과 Presenter binding이 소유한다.
- `VT04_ArmorIcon_Arrow`, `VT04_ArmorIcon_Chevron2`, rotation Review/Manifest를 local source evidence로 추가했다. 기존 `VT03` six-direction pack은 USER PASS 없이 Superseded됐다.
- Current를 `Armor Modular Structure Locked / 2-Icon Source Pack Ready / Vehicle Silhouette Binding + WBP Migration Pending / Source Binding Deferred`로 전진했다. 다음 구현은 HUD Visual catalog + ArmorSector additive icon migration이며 UE Import/WBP/C++ mutation은 아직 0이다.

### v0.1.18 - 2026-08-25

- USER가 `VT01_FrameReview_v2` Frame Family를 PASS했다. Frame local evidence는 Pre-Binding 상태로 보존하고 새 관련 결함이 없는 한 Review를 반복하지 않는다.
- `ARMOR-S1` local source pack을 제작했다. v1 silhouette의 과도한 line threshold를 폐기하고 v2에서 target full wireframe을 보존했으며, common tintable mechanical plate + 6 source-derived icon-only variant를 만들었다. 방향 glyph는 VT-VEH-01 source pixel mirror/rotation만 사용한다.
- Current를 `ARMOR-S1 Local Source Pack Ready / USER Review Pending / Source Binding Deferred`로 전진했다. 다음 exact USER Gate는 `VT03_ArmorReview_v2`; PASS 뒤 Defense source family로 이동한다. UE Import/WBP/Runtime mutation은 0이다.

### v0.1.17 - 2026-08-25

- USER가 `VT02_RpmRuntimePack_Review_FINAL3`를 확인해 RPM-S1 Visual을 PASS했다. AUTO QA PASS와 USER Visual PASS를 분리 기록하고 새 관련 결함이 없으면 RPM Review를 반복하지 않는다.
- Current를 `RPM-S1 USER PASS / Frame USER Review Pending / Source Binding Deferred`로 전진했다. 다음 exact USER Gate는 `VT01_FrameReview_v2`이며 PASS 뒤 Armor silhouette + 6 icon-plate source extraction으로 이동한다.
- `T_UI_RPMMaskPack`의 최종 safe packing과 기존 `M_UI_RPMGauge + RPMRatio` runtime 구조는 변경하지 않는다. UE Import/WBP/Runtime mutation은 0이다.

### v0.1.16 - 2026-08-25

- 작동형 RPM 기준 `RPM-S1 Source Mask Pack`을 local pre-binding 범위에서 완료했다. 초기 curved-path/segment 결함은 자동 QA에서 검출해 폐기하고 최종 경로를 세로→좌상단 1/4 곡선→상단 수평으로 교정했다.
- 최종 Source QA는 21 independent Tick / 21 Progress levels / Red 4 Tick / Ratio 0·0.25·0.50·0.85·1.00 = 0·5·10·17·21 점등으로 PASS했다.
- Runtime Import 후보를 `T_UI_RPMMaskPack` 724×467 RGBA 한 장으로 channel-pack했다. PNG zero-alpha RGB 위험을 제거하기 위해 최종 배치는 R=Progress, G=Tick21, B=GuideRail, A=255 opaque로 교정했고 Red는 R+`RedStartRatio=0.85`에서 파생한다. 기존 `Image_RPMGauge + M_UI_RPMGauge + RPMRatio` 단일 구조를 유지하며 21개 개별 Widget을 만들지 않는다.
- Current를 `RPM-S1 Local Source Pack Ready / Frame+RPM USER Review Pending / Source Binding Deferred`로 전진했다. UE Import/WBP/Runtime mutation은 0이다.

### v0.1.15 - 2026-08-25

- USER local PC access 제약으로 repository Source Binding은 Deferred 처리했지만 VT-VEH-01 exact 원본을 사용한 Frame F1~F4 Local Pre-Binding Extraction Evidence를 완료했다.
- F3 ArmorFrame을 v2로 보수적 alpha cleanup해 Armor card/vehicle/ring contamination을 제거하고 `FrameMock_v2`, `FrameReview_v2`, SHA manifest를 생성했다. UE Import/WBP mutation은 0이다.
- VPR-P0-00의 stale `VT-VEH-01 Pending` projection을 `USER APPROVED / LOCKED` Complete로 교정했다.
- Current를 `Frame F1~F4 Local Extraction Ready / Source Binding Deferred / USER Frame Review Pending`으로 전진했다.

### v0.1.14 - 2026-08-24

- USER가 현재 Master를 `VT-VEH-01 Production Visual Target`으로 명시 승인·LOCK해 Candidate 상태를 종료했다.
- 대표 Plan의 Frame family F0~F6 extraction plan을 projection했다. native 1536×713에서 outer overlay / SpeedFrame / ArmorFrame / GearFrame을 target-preserving alpha extraction하고 Review Sheet USER PASS 전에는 UE Import하지 않는다.
- Current를 `VT-VEH-01 Locked / Frame Family Extraction Plan Ready / Source Binding Pending`으로 전진했다. 다음 exact step은 repository source binding + hash readback이다.

### v0.1.13 - 2026-08-24

- USER가 Armor icon-only, RPM은 `RPM` label only, Shield/Integrity Chevron은 기존 recovery/repair 의미 유지 방식을 승인해 남은 Slot Contract를 닫았다.
- 1536×713 → 896×416 `7/12` uniform mapping, root 9-Slice underlay + full-aspect overlay, single compact ArmorSector family, Defense frame/mask/hex/material 경계를 확정했다.
- Physical Art Breakdown Contract를 작성해 기존 asset identity refresh와 신규 Target-derived asset 9종의 역할을 고정했다. 실제 PNG/Material 생성·Import는 아직 수행하지 않았다.
- Current를 `Physical Art Breakdown Contract Ready / VT-VEH-01 Lock + Source Binding Pending`으로 전진했다.

### v0.1.12 - 2026-08-24

- USER가 `CAND-VEH-MASTER-01` 상단 Speed/RPM : Armor를 50:50으로 명시 승인해 기존 35~40:55~60 Deviation 후보를 닫았다.
- 896×416 기준 `422 + Gap 12 + 422` Slot width를 Current Composition으로 고정하고 `InGameUIVehiclePanelSpec.md v0.20.4`에 반영했다.
- 남은 Slot Contract Review를 uniform scale mapping / Frame decomposition / Armor plate·label / Defense fill·material 경계로 축소했다. `VT-VEH-01`은 계속 Pending이다.

### v0.1.11 - 2026-08-24

- USER 제공 1536×713 VehiclePanel 이미지를 `CAND-VEH-MASTER-01` Logical Breakdown Master Candidate로 등록하고 WBP Skeleton Contract까지 작성했다.
- fresh AssetDump 기준 기존 `WBP_CFSpeedGauge`, `WBP_CFArmorBodyMap`, `WBP_CFArmorSector`를 보존하고 신규 reusable `WBP_CFDefenseBar` 경계를 기본안으로 잡았다. 실제 nested 도입은 Presenter binding migration과 atomic하게 수행한다.
- Candidate 상단 50:50 인상과 기존 `CT-VEH-01` 35~40:55~60의 폭 차이, Armor icon+label 표현과 현 icon-first fallback 계약의 차이를 Slot Contract Review 항목으로 승격했다.
- 현재 Gate를 `WBP Skeleton Contract Ready / Slot Contract Review Pending`으로 전진했다. Source/UE Asset mutation은 0이다.

### v0.1.10 - 2026-08-24

- USER 결정으로 공통 Visual Gate를 `Target Lock → Logical Breakdown → WBP Skeleton → Slot Contract → Physical Art Breakdown → Assembly` 순서로 강제했다.
- `WBP/Primitive-first 후 Concept 맞춤`, Raw ProgressBar/Border final styling, AI replacement art 직접 Import, Technical Scaffold의 Master Target 역승격을 Deprecated 처리했다.
- 기존 Frame/Shield/Integrity Technical/Pixel PASS는 Runtime/Scaffold evidence로 보존하되 Visual Match 진행 근거에서는 내렸다.
- VPR-P0-01 Current를 `Production Method Reset Locked / Target Logical Breakdown Pending`으로 변경하고 다음 exact output을 VehiclePanel Master Target의 논리적 분해로 고정했다.

### v0.1.9 - 2026-08-24

- Defense Bar final pixel fix와 test-harness 교정 후 `VehicleDefenseBarVisualContract` 1/1 PASS를 확인했다.
- fresh `M_VehicleDefensePIE` Defense Pawn에서 Shield/Integrity 두 Row가 각각 591x10 Bar로 동시에 실제 렌더되는 것을 Snapshot/Screenshot으로 확인했다.
- Frame/Speed-RPM/Armor 보존까지 확인해 VPR-P0-01을 Technical/Pixel PASS로 복구하고 next gate를 USER Visual Review로 이동했다.
- Armor USER PASS와 전체 `VT-VEH-01` Pending 경계는 유지한다.

### v0.1.8 - 2026-08-24

- fresh PIE pixel review에서 Integrity ProgressBar 본체가 실제 화면에서 사라지는 회귀를 발견해 Frame + Shield/Integrity Technical PASS를 재검증 Pending으로 되돌렸다.
- `CFHUDPresenter v1.25.1`에서 custom ProgressBar Track/Fill/Marquee Brush에 10px intrinsic height를 부여했다.
- 첫 height 교정 focused는 DesiredSize.Y 0으로 FAIL했고 Marquee Brush까지 보강한 최종 교정본 Official Build는 PASS. exact focused rerun + fresh PIE pixel revalidation이 다음 Gate다.
- Armor USER PASS는 반복하지 않는다.

### v0.1.7 - 2026-08-24

- VPR-P0-01 Frame + Shield/Integrity production visual technical slice를 PASS했다.
- 저장 VehiclePanel의 기존 P2 9-Slice Frame 연결을 재사용하고 Shield/Integrity는 기존 ProgressBar/Designer Slot을 보존한 runtime StyleData presentation으로 교정했다.
- final Build + Frame/Defense Bar focused + Defense Runtime + RPM 보호 회귀를 PASS하고 Armor USER PASS는 replay하지 않았다.
- 현재 next gate를 fresh Editor에서의 USER Frame + Shield/Integrity Visual Review로 좁혔다.

### v0.1.6 - 2026-08-24

- USER가 Armor 방향 Text가 Texture 존재 시 의도적으로 숨겨지는 fallback 구조임을 확인하고 Armor visual slice를 USER PASS했다.
- Armor technical PASS와 USER PASS를 결합해 해당 slice를 반복 금지 범위로 닫았다.
- VPR-P0-01 다음 작업을 VehiclePanel Frame + Shield/Integrity production visual match로 전진했으며 전체 `VT-VEH-01`은 여전히 미승격 상태다.

### v0.1.5 - 2026-08-24

- VPR-P0-01을 실제 IN PROGRESS로 전환하고 Armor Production Visual technical slice를 완료했다.
- 기존 P2 Armor 6종과 Designer 배치를 유지하면서 `CFArmorSectorWidget v1.1.1`에 icon-first/Label fallback과 Ratio 기반 Armor/Caution/Critical tint를 적용했다.
- first focused failure를 lifecycle-safe `SetArmorPercent()` visibility 재보장으로 교정한 뒤 exact focused 1/1 PASS, Defense 1/1 PASS, RPM 1/1 PASS와 final Official Build PASS를 확인했다.
- Content Asset mutation 0을 유지하고 현재 Gate를 USER Visual Review Pending으로 전진했다. 전체 `VT-VEH-01`은 USER 명시 승인 전이라 미승격 상태다.

### v0.1.4 - 2026-08-24

- 신규 VehiclePanel Concept USER 피드백을 editable composition 승인으로 반영하되 `VT-VEH-01` Production Target은 미승격 상태로 유지했다.
- persisted AssetDump로 `WBP_CFArmorBodyMap`의 Canvas + 탑다운 VehicleSilhouette + 동일 `WBP_CFArmorSector` 6개 재사용 구조와 독립 Canvas Slot 편집성을 확인했다.
- 기존 `WBP_CFArmorSector`가 canonical common Armor slot이므로 별도 `WBP_CFArmorSlot` 신규 생성을 금지해 중복 구조를 방지했다.
- Production Target 이전에는 새 Art를 만들지 않는 Structure Readiness 감사만 허용하는 예외를 공통 Gate에 추가했다.
- VPR-P0-01을 `STRUCTURE READINESS PASS / PRODUCTION TARGET PENDING`으로 명시하고 실제 Production Art 착수는 계속 `VT-VEH-01` 승인 뒤로 유지했다.

### v0.1.3 - 2026-08-23

- VPR-P0-00의 AI Recovery를 완료하고 Current Gate를 `USER VEHICLE TARGET PENDING`으로 좁혔다.
- 동일 텍스트/Source 검색 반복을 금지하고, 과거 승인 원본 재제공 또는 Plan §5.5 신규 Concept Candidate 중 하나로 `VT-VEH-01`을 확정하도록 했다.
- 대표 Plan v0.1.3과 동기화했다.

### v0.1.2 - 2026-08-23

- VPR-P0-00을 READY에서 IN PROGRESS로 전환하고 대표 Plan v0.1.2와 동기화했다.
- 초기 Vehicle Registry에서 Direction Reference와 Composition Target이 회수됐고 Production Visual Target 이미지는 아직 미확정인 상태를 반영했다.

### v0.1.1 - 2026-08-23

- VPR-P0-00 Recovery를 저장소/SourceArt → 접근 가능한 대화·파일 → 필요 시 USER 재제공 순으로 고정하고 기억 기반 Target 재생성을 금지했다.
- Weapon Icon은 Visual slot 설계와 실제 per-weapon identity source 연결을 분리해 가짜 Icon source 생성을 막았다.
- Pause Menu는 기존 D1-12의 WBP Primary + Native Safe Fallback migration을 재사용하도록 교정했다.
- Radar Altitude는 현재 HUD source 부재로 Deferred 처리해 Visual Rework가 새 Runtime feature로 팽창하지 않도록 했다.
- Layout 변경 Slice마다 ResolutionLayoutContract 영향 범위 smoke를 추가하고 P2-08은 최종 통합 Review로 유지했다.

### v0.1.0 - 2026-08-23

- Production UI Visual Rework의 P0~P2 실행 순서를 신규 정의했다.
- 첫 Gate를 과거 승인 이미지 회수와 Visual Target Registry 고정으로 설정했다.
- Vehicle → Weapon → Pause → Target → Radar → Reticle/Marker → Alert → Global Review 순으로 dependency를 고정했다.
- 각 Slice에 Target 등록 → Breakdown → Art → Designer → Technical → Visual Match → USER PASS 공통 Gate를 적용했다.

---

## 17. Migration

- 기존 `InGameUIRoadmap.md`는 `CF-FQ-032` Historical roadmap이며 이 문서로 덮어쓰지 않는다.
- 새 Visual 작업은 이 Roadmap의 `VPR-*` ID를 사용한다.
- 기존 Visual/Art Spec의 세부 규격은 재작성하지 않고 링크해 사용한다.
