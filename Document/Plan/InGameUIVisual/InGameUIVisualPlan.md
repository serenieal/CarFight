# CarFight InGame UI Visual Rework Plan

- 문서 버전: v0.1.33
- 작성일: 2026-08-23
- 최근 갱신일: 2026-08-28
- 문서 상태: Active
- 기능 ID: `CF-FQ-039`
- 기능명: `Production UI Visual Rework`
- 현재 Gate: `VPR-P0-01 VehiclePanel Actual-Asset Prototype — VT12 SPEED/RPM + ARMOR COMPOSITION READY / USER COMPOSITION REVIEW PENDING / UE IMPORT 0 / PRODUCTION ASSET MUTATION 0`
- Current System 기준: `Document/Systems/UI/InGameUI.md v1.1.10`
- 완료 기반: `CF-FQ-032` Done / `Document/Plan/Archive/InGameUIPlan.md v0.59.36`
- 실행 Roadmap: `InGameUIVisualRoadmap.md v0.1.27`
- HUD Art 제작 규격: `InGameUIHUDArtSpec.md v0.4.18`
- 시각 방향: `InGameUIVisualConcept.md v0.8.1`
- Style 계약: `InGameUIStyleSpec.md v0.86.1`

---

## 1. 목적

이 Plan은 이미 동작하는 CarFight 인게임 UI 기능 구조를 보존하면서, 현재의 기능형·Scaffold 중심 Visual Layer를 실제 게임용 Production UI로 재구성하는 후속 작업을 소유한다.

핵심 목표는 다음 한 문장으로 고정한다.

> **기능 골격은 보존하고, 승인된 Visual Target을 실제 Texture·Material·UMG Designer 구성으로 충실하게 구현한다.**

이 작업은 `CF-FQ-032`를 다시 여는 회귀 수정이 아니다. `CF-FQ-032`에서 완료된 UI Runtime·Provider·Presenter·ViewData·수명·Radar/Target runtime 계산과 기존 Build/Automation/USER PASS를 보존하고, 별도 후속 Feature에서 Visual Production 품질을 닫는다.

---

## 2. 작업을 별도 Feature로 분리하는 이유

기존 UI 작업에서는 기능 검증과 Visual 제작이 같은 흐름에 섞이면서 다음 문제가 발생했다.

```text
- 사용자에게 좋은 평가를 받은 기준 이미지가 실제 구현의 강제 Visual Target으로 승격되지 않았다.
- 기준 이미지의 Frame·Silhouette·Hierarchy·Spacing·Material Language를 Asset Breakdown으로 변환하지 않았다.
- 기능 PASS가 Visual Match PASS보다 먼저 완료 기준처럼 작동했다.
- 초기 Production Scaffold가 Border·ProgressBar·Text·단순 Image 조합을 빠르게 만들었지만, 그 구조가 최종 시각 결과처럼 남은 영역이 있다.
- 시각 구현 편의 때문에 기준 이미지보다 단순한 결과가 남아도 별도 Deviation Gate가 없었다.
```

따라서 이번 Feature는 기능 추가보다 **Visual Target 보존과 Production Art 적용 과정 자체**를 우선한다.

---

## 3. 절대 보호 범위 — KEEP

아래 완료 기반은 새 관련 결함이 확인되지 않는 한 재설계·재구현하지 않는다.

```text
- UCFUISubsystem / UCFUIRootWidget 수명·Layer 구조
- Runtime → UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter → Production Widget 흐름
- Pawn Rebind와 이전 Pawn 수명 비소유 계약
- Speed / RPM / Gear runtime source와 표시 데이터 계약
- Shield / Armor / Integrity runtime source 계약
- Weapon selection / Ammo / Reload / Cooldown / Heat / Charge 의미 계약
- Sensor / Target Knowledge / Radar ViewData 의미 계약
- Radar Contact runtime 위치 계산
- TargetSelect World→Screen / Screen-edge runtime 위치 계산
- Weapon Reticle World→Screen runtime 투영
- WBP_CFArmorSector 6방향 재사용 구조
- Image_RPMGauge + M_UI_RPMGauge + RPMRatio 단일 동적 RPM 구조
- persisted UMG Designer Layout ownership
```

`CF-FQ-032` final Build와 broad regression은 이 Feature 착수만을 이유로 반복하지 않는다. 실제 수정 영향 범위가 생긴 뒤 필요한 focused regression만 다시 수행한다.

---

## 4. 현재 UI 전수 판정 Baseline

2026-08-23 Source + persisted Widget Designer 감사 기준 판정은 다음과 같다.

| 영역 | 판정 | 이번 Feature 처리 |
| --- | --- | --- |
| UI Runtime / Root / Layer | `KEEP` | 변경 금지 기본 |
| Provider / Presenter / ViewData | `KEEP` | 의미 계약 보존 |
| `WBP_CFInGameHUD` | `KEEP / LAYOUT` | 전체 균형·배치 polish만 |
| `WBP_CFSpeedGauge` | `KEEP + ART` | Gauge·Typography·Frame Visual 강화 |
| RPM UI Material | `KEEP` | 현재 동적 구조 유지, 시각 파라미터만 개선 가능 |
| `WBP_CFVehiclePanel` | `ART + LAYOUT` | 실제 Panel Art와 상태 표현 재구성 |
| `WBP_CFArmorBodyMap` | `KEEP + ART/LAYOUT` | 6 Sector 구조 유지, Art·배치 개선 |
| `WBP_CFArmorSector` | `KEEP + ART` | Plate·Ratio 표현 개선 |
| `WBP_CFWeaponPanel` | `ART + LAYOUT` | 의미 슬롯 유지, 시각 위계 재설계 |
| `WBP_CFTargetPanel` | `ART` | Target Intelligence Visual 재설계 |
| `WBP_CFRadarPanel` | `KEEP + ART` | runtime spatial 구조 유지, Frame/Blip/Sweep 개선 |
| `WBP_AimReticle` | `KEEP + ART` | Aim 로직 유지, Reticle Art·Debug성 표현 정리 |
| `WBP_TargetSelect` | `KEEP + ART` | runtime 좌표 유지, Marker/Edge Art 통일 |
| `WBP_CFAlertFeed` | `ART + ANIMATION` | lifecycle 유지, 시각·등장/소멸 표현 개선 |
| `WBP_CFMissionPanel` | `DEFER` | 실제 Mission runtime owner 확정 전 보류 |
| `UCFPauseMenuWidget` Visual Tree | `REBUILD Visual` | Continue/Focus/lifecycle 유지, Designer WBP로 이전 |
| `CFUIHUDProdEditorBridge` | `LIMIT` | Validate/Migration/empty Scaffold만, 신규 Visual 제작 수단으로 사용하지 않음 |

이 판정은 구현 시작 시 다시 처음부터 감사하지 않는다. 특정 Slice에서 실제 Source/Asset이 바뀐 경우에만 해당 영역을 재확인한다.

---

## 5. Visual Target 계약

### 5.1 Visual Target은 단순 참고 이미지가 아니다

사용자가 **실제 구현 기준으로 명시 승인한 이미지**만 해당 Slice의 `Production Visual Target`으로 취급한다. `좋다`, `분위기가 좋다`, `이 방향이 좋다`처럼 시각 방향만 긍정한 표현은 자동으로 Composition이나 Production Target 승인이 되지 않는다.

Visual Target이 등록된 뒤에는 구현 편의를 이유로 다음을 임의 변경하지 않는다.

```text
- 전체 Silhouette
- Panel/Element 비율
- 정보 Hierarchy
- 주요 Frame 형태
- 핵심 색 대비
- Gauge / Armor / Weapon / Radar의 대표 형태 언어
- 주요 장식선과 Material 인상
```

기술적 이유로 변경이 필요하면 구현 전에 `Deviation`을 기록하고 사용자 승인을 받아야 한다.

### 5.2 승인 단계

승인 범위는 아래 세 단계로 분리한다.

```text
Visual Direction Reference
= 분위기 / 색 / 재질 / 형태 언어 참고 또는 승인.
  Composition 일치 의무 없음.

Composition Target
= Panel 구조 / 주요 비율 / 정보 Hierarchy / 대표 형태 승인.
  세부 Art Asset 자체의 픽셀 일치 의무는 없음.

Production Visual Target
= 실제 구현 기준 이미지.
  Registry의 Must Match 항목은 Deviation 승인 없이 임의 변경 금지.
```

한 이미지가 여러 단계를 동시에 충족할 수는 있지만, 사용자 승인 문구가 무엇을 승인했는지 Registry에 명시한다. 승인 범위가 불명확하면 더 좁은 단계로 해석하고 Production Target으로 자동 승격하지 않는다.

### 5.3 Visual Target Registry

실제 Production 작업에 들어가기 전 아래 형식으로 Target을 이 Plan에 등록한다.

| 필드 | 의미 |
| --- | --- |
| `Target ID` | `VT-VEH-01`, `VT-WPN-01` 같은 고정 ID |
| `Scope` | Vehicle / Weapon / Radar / Target / Reticle / Pause 등 |
| `Source` | 저장소 이미지 경로 또는 대화에서 회수한 식별 가능한 원본 |
| `Approval Level` | Direction Reference / Composition Target / Production Visual Target |
| `USER Status` | Candidate / USER APPROVED / Superseded |
| `Must Match` | 반드시 유지할 형태·비율·Hierarchy·재질 인상 |
| `Flexible` | 실제 데이터 길이·동적 상태 때문에 조절 가능한 부분 |
| `Production Breakdown` | 실제 Texture / Material / Widget / Animation 목록 |

대화에만 존재하고 저장소 경로가 아직 없는 과거 승인 이미지는 존재하지 않는 파일 경로를 만들어 기록하지 않는다. `VPR-P0-00`에서 회수·식별한 뒤 Registry에 등록한다.

### 5.4 Current Target Registry — VPR-P0-00

| Target ID | Scope | Source | Approval Level | USER Status | Must Match | Flexible / Pending |
| --- | --- | --- | --- | --- | --- | --- |
| `VDR-VEH-01` | Vehicle HUD visual language | `InGameUIHUDArtSpec.md §4.3 Balanced Combat` | Direction Reference | USER APPROVED | dark navy/near-black layered surface, Cyan system accent, Amber armor/caution, Red critical-only, restrained mechanical depth, shallow/local glow | VehiclePanel composition, gauge geometry, Gear/Armor placement은 이 승인으로 동결하지 않음 |
| `CT-VEH-01` | VehiclePanel composition | `InGameUIVehiclePanelSpec.md v0.20.4` | Composition Target | USER APPROVED | 좌측 SpeedGauge + 우측 ArmorBodyMap **50:50**, 하단 Shield/Integrity full-width 2단, 비대칭 RPM gauge, 단일 Gear Slot, **탑다운 좌향 차량**과 6방향 공통 Armor Sector 의미 배치 | 896×416 기준 `SpeedGauge 422×272 / Gap 12 / ArmorBodyMap 422×272`는 Current composition. Sector 내부 간격·방향 Icon/Label 구현 방식·세부 Frame ornament는 Slot/Production Target 단계에서 조정 가능 |
| `VT-VEH-01` | VehiclePanel full visual | repository canonical copy `SourceArt/UI/HUD/VT/VT_VehPanel01.jpg` / 1280×594 / SHA-256 `cf5b0a57b76f9c41272ee01ed752d263a7b01a78bf02e9cb9dc042d000df6b6b` / 과거 대화 승인본은 1536×713 별도 식별 | **Production Visual Target** | **USER APPROVED / LOCKED** | dark layered mechanical outer/module frame, Speed/RPM : Armor 50:50, 좌측 비대칭 RPM+large digital Speed+single Gear, 우측 탑다운 좌향 차량+6 Armor icon family, 하단 full-width Shield/Integrity, cyan/amber/red hierarchy, honeycomb defense fill, restrained local glow | 현재 repository copy는 USER가 직접 SourceArt에 제공한 canonical source다. Runtime 샘플 숫자·Armor 방향 Text·x1000/절대 RPM 숫자 눈금은 Target에서 그대로 복제하지 않고 이미 승인된 semantic override를 적용. Chevron은 외형만 match하고 기존 fill-end recovery/repair 의미 유지 |
| `CT-HUD-ROOT-01` | 전체 HUD module placement | `ProjectSSOT/CombatPlan/DecisionLog.md CP-D028` | Composition Target | USER APPROVED | Mission TL / Alert TC / Target TR / Vehicle BL / Radar BC / Weapon BR, 중앙 Reticle·Target Marker 보호영역 | 각 Panel 내부 Visual·실제 Art는 별도 Target이 소유 |
| `CT-WPN-01` | WeaponPanel compact composition | `InGameUIWeaponPanelSpec.md v0.7.4` | Composition Target | USER APPROVED | 우하단 정렬, Selected Weapon 우선, Fire State → Primary Resource → Secondary 최대 2 → 조건부 Compact Rail 정보 hierarchy, 빈 슬롯 Collapse, 최대 Slot 464×360 안에서 content 수축 | 실제 pixel size는 Density Data가 소유. per-weapon Icon identity와 representative multi-weapon Rail Visual은 source/content 준비 전 Deferred |
| `CAND-VEH-IMG-01` | 과거 D1 VehiclePanel image | 저장소 루트 `ChatGPT Image 2026년 8월 7일 오전 09_45_34.png` — External Image Vision 직접 관측 | Historical Composition Reference | Superseded Reference | D1-VEHICLE-PANEL의 dark/cyan/amber/red 계층과 당시 구성 확인 | 현재 CT-VEH-01의 RPM/Gear/Armor 배치 및 VT-ARMOR-01과 불일치하므로 Production Target 승격 금지 |
| `REF-P1-HUD-01` | P1 HUD source review | `SourceArt/UI/HUD/P1/Review/P1_HUD_Review.png` / ArtSpec evidence | Technical Reference | Historical Reference | Vehicle Silhouette / Armor 6 / Speed Arc / Target Bracket source 기술 검토 | `USER Production Art Approval` 아님. Import/Target 기준으로 재사용 금지 |
| `REF-P2-VEH-01` | P2 VehiclePanel technical art | `SourceArt/UI/HUD/P2` 9종 + `SourceArt/UI/HUD/P2/Review/` technical review | Technical Reference | Technical PASS / USER Visual PASS 미부여 | persisted VehiclePanelFrame / VehicleSilhouette / Armor 6 / RPMTrack의 당시 기술 상태 | 2026-08-19~20 USER Visual Review가 Deferred였으므로 Production Visual Target으로 승격 금지 |
| `VT-ARMOR-01` | ArmorBodyMap detailed visual | 과거 USER 수정본 이미지 — 현재 세션에서 원본 바이너리 미회수 | Production Visual Target Decision | USER APPROVED / ORIGINAL SOURCE RECOVERY REQUIRED | 차량은 왼쪽을 봄, Front=이미지 좌, Rear=우, Top=좌상, Bottom=우하, Left arrow=아래, Right arrow=위, 사용자 수정본의 전체 배치/형태를 실제 구현 기준으로 사용 | 원본 이미지 픽셀·아이콘 디테일 비교는 재회수 전 수행 금지. 기억 기반 재생성 금지 |
| `VDR-HUD-A` | 전체 HUD working concept | `InGameUIHUDArtSpec.md §4.4 Working Visual Direction Reference A` | Direction Reference | Reference Only | dark/cyan/amber/red hierarchy, panel별 독립 frame, 큰 수치/작은 label hierarchy | 저장소 원본 미확인. 한 장짜리 HUD layout/임의 수치 직접 복제 금지 |
| `CT-RADAR-01` | Radar composition / visual grammar | `InGameUIStyleSpec.md v0.86.1`의 2026-08 사용자 결정 기록 | Composition Target | USER APPROVED | Heading-Up, 아주 옅은 타원 Surface, 25/50/75% 3 Range Ring, Unknown `◆` / Hostile `▼` / Friendly `●` 동일 기본 크기, Player `▲`, in-range 4-Corner Open Bracket, off-range 2-Corner Open Edge Bracket, Surface→Ring→Stalk→Blip→Bracket layer, Sweep은 Scan 중 얇은 Cyan 회전선+짧은 Fade Trail | 현재 HUD Radar ViewData에 없는 Altitude/Z 소비 확장은 CF-FQ-039에서 만들지 않음. 기존 Stalk 장기 디자인은 source가 생길 때까지 Deferred |
| `DR-TARGET-01` | TargetPanel information visual contract | `InGameUIDesign.md v0.35.1` + `InGameUIStyleSpec.md v0.86.1` | Design/Composition Reference | Current Design Contract | 우상단 기본 폭 384, 세로 확장, `Identity/Relation → Tracking → Analysis Progress → Tactical Intelligence`, Compact Intelligence Card, Last Known / Lost / Destroyed 상태 분리, VehiclePanel 실루엣/Armor Plate 복제 금지 | 완성 이미지 Production Target 승인 증거는 미회수. 세부 Frame/Art는 후속 Target 필요 |
| `DR-RETICLE-01` | Command/Turret/Selected Target visual family | `InGameUIStyleSpec.md v0.86.1` | Design/Composition Reference | Current Design Contract | Command Reticle=`Center Dot + 4 Open Bracket` 48×48 계열, Screen-Space Upright, Turret marker와 의미 분리, Selected Target=4-Corner Open Bracket, 중앙 시야 보호, 관계색으로 Command Reticle 자체를 물들이지 않음 | 완성 Production Reticle 이미지 승인 증거는 미회수. 실제 Art family는 VPR-P1-06에서 Target화 |
| `DR-ALERT-01` | AlertFeed compact presentation | `InGameUIDesign.md v0.35.1` + `InGameUIStyleSpec.md v0.86.1` | Design/Composition Reference | Current Design Contract | Top Center, 2560×1440 기본 720×120 slot, 최대 3 Compact alert, 즉시 대응 사건만 표시, 평상시 상태 중복 금지, 비상호작용 HUD, 반복 Pulse/Flash 남용 금지 | 완성 Production Alert visual 승인 증거는 미회수 |
| `DR-PAUSE-01` | Pause Menu visual/migration direction | `InGameUIVisualConcept.md v0.8.1` + `InGameUIStyleSpec.md v0.86.1` + `InGameUIAssetizationSpec.md v0.16.2` | Design/Composition Reference | Current Design Contract | 전투 HUD보다 안정적이고 큰 Menu surface, 제한된 Dim Overlay, 명확한 Primary Button 1개, Hover/Focused 분리, 첫 Primary Button Focus, WBP Primary + Native Fallback | 완성 Pause Production Target 이미지는 미회수. VPR-P0-03에서 별도 Target 승인 필요 |

2026-08-24 USER가 현재 대화에 제공한 1536×713 이미지를 **`VT-VEH-01 Production Visual Target`으로 명시 승인·LOCK**했다. 이전 이름 `CAND-VEH-MASTER-01`은 승인 전 후보를 가리키던 Historical alias이며 Current Target ID로 사용하지 않는다. P1/P2 Review는 계속 Technical/Historical Reference이고 `VT-VEH-01`을 대체하지 않는다.

VehiclePanel Master의 저장소 Source binding은 **완료**됐다. USER가 직접 추가한 `SourceArt/UI/HUD/VT/VT_VehPanel01.jpg`를 현재 repository canonical copy로 사용하며, 과거 1536×713 대화 첨부본과 byte/hash가 동일하다고 주장하지 않는다. 현재 남은 ingress 문제는 RPM-S1 파생 Source Pack이다. 2026-08-25 재감사에서 `SourceArt/UI/HUD` 전체와 Host-global Trash에 `T_UI_RPMMaskPack` / `VT02_*` image evidence가 0건이므로 과거 local production 기록을 current repository source로 취급하지 않는다.

### 5.5 VehiclePanel New Concept Candidate Brief

이 Brief는 `VDR-VEH-01 + CT-VEH-01 + CT-HUD-ROOT-01 + VT-ARMOR-01 decision`을 하나의 시각 후보로 조합하기 위한 생성 계약이다.

#### Canvas / Composition

```text
VehiclePanel aspect target ≈ 896×416 / 약 2.15:1
상단 좌측 50% = Speed / RPM cluster
상단 우측 50% = Armor body map
896×416 내부 기준 = SpeedGauge 422 + Gap 12 + ArmorBodyMap 422
하단 full width = Shield + Integrity 2단
배경은 투명 HUD overlay를 전제로 한 독립 panel frame
```

#### Visual Language — Must Match

```text
- dark navy / near-black layered tactical vehicle panel
- Cyan = speed / system / normal information
- Amber = armor / caution / important defense state
- Red = critical/redline only
- mechanical depth는 얕고 정돈되게, 장식선 남발 금지
- Glow는 좁고 국소적으로 사용
- 자동차 디지털 계기판 + 전투차량 HUD의 균형
- 큰 숫자 / 작은 unit·label의 강한 hierarchy
```

#### Speed / RPM — Must Match

```text
- 3자리 digital speed `076` 형태
- `km/h`는 speed 오른쪽의 작은 보조 단위
- Gear는 `R / N / 실제 전진 Gear 숫자` 단일 slot
- RPM은 원형 dial 금지
- 비대칭 Track: 좌측 세로 → 부드러운 곡선 → 상단 긴 수평
- 동적 표현은 현재 `Image_RPMGauge + M_UI_RPMGauge + T_UI_RPMTrack + RPMRatio`와 호환되는 형태
- 21 tick 인상과 redline 85% 시각 언어는 유지하되 세부 tick ornament는 Concept 단계에서 조절 가능
```

#### Armor — Must Match

```text
- 중앙 차량 실루엣은 왼쪽을 봄
- Front = 이미지 좌 / Rear = 이미지 우
- Right = 상 / Left = 하
- Top = 좌상 / Bottom = 우하
- Left arrow = 아래 / Right arrow = 위
- 6방향 sector가 동일 family로 읽혀야 함
- Armor 값은 각 sector의 동적 fill/bar로 표현 가능하며 고정 수치를 이미지에 굽지 않음
```

#### Shield / Integrity — Must Match

```text
- Armor와 분리된 두 방어 계층
- Shield가 위, Integrity가 아래
- 전체 폭의 낮고 긴 frame/fill treatment
- 기본 ProgressBar primitive 그대로 보이지 않도록 frame/mask/material 또는 art brush 사용
```

#### 금지

```text
- 원형 자동차/항공기 RPM dial로 회귀
- 6방향 Armor를 단순 4방향으로 축소
- Gear를 D/N/R 고정 표기로 회귀
- 완성 HUD 전체를 한 장 bitmap으로 만들어 runtime text/data까지 굽기
- 기본 UMG Border/ProgressBar 색칠만으로 Concept을 흉내내기
- P1/P2 Technical Review 이미지를 USER 승인 Target으로 위장
- 실제 source 없는 장식용 Gameplay 수치·아이콘 의미 추가
```

#### 승인 후 Breakdown 예상

```text
1. VehiclePanel 9-slice Frame
2. RPM Track/Mask + Material polish
3. Speed/Gear typography treatment
4. Vehicle silhouette
5. Armor plate/badge family 6종
6. Armor dynamic fill treatment
7. Shield frame/fill
8. Integrity frame/fill
9. local glow/highlight mask
```

이 Brief로 생성된 이미지는 `CAND-VEH-NEW-01`부터 시작하며 USER가 실제 구현 기준으로 명시 승인한 시점에만 `VT-VEH-01`로 승격한다.

### 5.6 `VT-VEH-01` Logical Breakdown + WBP Skeleton Contract

2026-08-24 USER 제공 1536×713 이미지를 직접 관측해 VehiclePanel Logical Breakdown 기준으로 사용한다. 이 이미지는 현재 세션의 **Master Candidate**이며 아직 `VT-VEH-01 Production Visual Target`으로 자동 승격하지 않는다. 저장소 원본 경로는 아직 없으므로 존재하지 않는 Source 경로를 만들지 않고 SHA-256과 크기로 식별한다.

#### 5.6.1 Logical Breakdown

| Module | Static Art | Dynamic Visual | Runtime Data | 기본 표현 수단 |
| --- | --- | --- | --- | --- |
| Root Frame | 외곽 mechanical frame, inner bevel/line | 선택적 shallow glow | 없음 | `Border` 9-Slice + 필요 시 overlay Texture |
| Speed/RPM | module frame, RPM track/tick/redline base, Gear frame | RPM fill/highlight | `SpeedKmh`, `GearText`, `RPMRatio` | 기존 `Image_RPMGauge` UI Material + TextBlock + Target-derived frame art |
| Armor BodyMap | module frame, tech backdrop, Vehicle silhouette, sector frame/icon family | Armor ratio fill/state tint | 6방향 Armor Ratio | `WBP_CFArmorSector` 6 재사용 + Texture/Progress or Material |
| Shield | frame/icon/track/right accent | cyan fill pattern/glow | Current/Max/Ratio/Visibility | 신규 공통 `WBP_CFDefenseBar` + target-derived fill material |
| Integrity | frame/icon/track/right accent | amber fill pattern/glow | Current/Max/Ratio/Visibility | `WBP_CFDefenseBar` 재사용 |
| Typography | RPM/GEAR/SHIELD/INTEGRITY label, unit | 실제 값 변경 | Speed/Gear/Defense Current·Max | runtime 값은 TextBlock. 고정 label은 TextBlock 기본, Art bake는 별도 승인 없이는 사용하지 않음 |

Runtime 숫자 `076`, Gear 값, Shield/Integrity Current/Max, Armor Ratio는 이미지에 굽지 않는다. Honeycomb, bevel, corner notch, arrow accent 같은 미감은 Target-derived Art/Material이 소유하고 C++ Primitive styling이 Visual authority가 되지 않는다.

#### 5.6.2 Current persisted structure audit

Fresh AssetDump dataset `adset_v1_eca38e6c5af5b0b1b3494b08b9a14b95.dad966306a091080e534784d` 기준:

```text
WBP_CFVehiclePanel
- Border_Surface → VerticalBox_Content
- WBP_CFSpeedGauge / WBP_CFArmorBodyMap
- HorizontalBox_Shield + ProgressBar_Shield
- HorizontalBox_Integrity + ProgressBar_Integrity

WBP_CFSpeedGauge
- CanvasPanel_Root
- Image_RPMGauge
- Text_Speed / Text_SpeedUnit / Text_Gear

WBP_CFArmorBodyMap
- CanvasPanel_Root
- Image_VehicleSilhouette
- WBP_ArmorFront/Right/Rear/Left/Top/Bottom : WBP_CFArmorSector_C

WBP_CFArmorSector
- Image_ArmorPlate + Text_Direction + ProgressBar_Armor
- Current persisted 구조는 compatibility scaffold. 다음 migration에서 별도 Image_DirectionIcon을 additive 추가한다.
```

`WBP_CFDefenseBar`는 현재 저장 Asset에 존재하지 않는다. `CFHUDPresenter v1.25.1`의 `FindNamedWidget`은 부모 `WidgetTree::GetWidgetFromName`만 사용하므로 Defense controls를 신규 nested UserWidget으로 이동하면 기존 Presenter가 자동으로 내부 자식을 찾지 못한다. 따라서 공통 DefenseBar 도입은 후속 Runtime Binding 단계에서 명시적인 binding migration과 함께 수행한다.

#### 5.6.3 WBP Composition Skeleton — target-derived

```text
WBP_CFVehiclePanel
└─ Border_Surface                                  [KEEP name / outer target frame]
   └─ Overlay_Root                                 [NEW]
      ├─ Image_FrameOverlay                        [NEW / optional target detail]
      └─ CanvasPanel_Composition                   [NEW / Designer layout SSOT]
         ├─ SizeBox_SpeedGauge                     [KEEP]
         │  └─ WBP_CFSpeedGauge                    [KEEP]
         ├─ SizeBox_ArmorBodyMap                   [KEEP]
         │  └─ WBP_CFArmorBodyMap                  [KEEP]
         ├─ SizeBox_ShieldBar                      [NEW]
         │  └─ WBP_ShieldDefenseBar : WBP_CFDefenseBar
         └─ SizeBox_IntegrityBar                   [NEW]
            └─ WBP_IntegrityDefenseBar : WBP_CFDefenseBar
```

Top-level `VerticalBox_Content / HorizontalBox_Main / Spacer_*`는 현재 Technical Scaffold evidence로는 보존되지만 Target의 자유 배치와 module ratio를 직접 소유하는 최종 Skeleton으로 사용하지 않는다. 실제 migration 시 `Border_Surface` 이름은 현재 9-Slice consumer/test 호환 때문에 보존하고 그 내부 Composition만 Canvas 기반으로 전환하는 안을 기본으로 한다.

```text
WBP_CFSpeedGauge
└─ CanvasPanel_Root                                [KEEP]
   ├─ Image_SpeedFrame                             [NEW]
   ├─ Image_RPMGauge                               [KEEP runtime MID sink]
   ├─ Text_RPMLabel                                [NEW / static `RPM` only]
   ├─ Text_Speed                                   [KEEP]
   ├─ Text_SpeedUnit                               [KEEP]
   ├─ Image_GearFrame                              [NEW]
   ├─ Text_GearLabel                               [NEW]
   └─ Text_Gear                                    [KEEP]
```

RPM tick/number/redline의 고정 시각은 `Image_RPMGauge`가 소비하는 Track/Material family가 소유하고, 현재 `RPMRatio` 단일 runtime sink는 변경하지 않는다.

```text
WBP_CFArmorBodyMap
└─ CanvasPanel_Root                                [KEEP]
   ├─ Image_ArmorFrame                             [NEW]
   ├─ Image_ArmorBackdrop                          [NEW / optional tech ring-grid]
   ├─ Image_VehicleSilhouette                      [KEEP]
   ├─ WBP_ArmorFront  : WBP_CFArmorSector          [KEEP name/class]
   ├─ WBP_ArmorRight  : WBP_CFArmorSector          [KEEP name/class]
   ├─ WBP_ArmorRear   : WBP_CFArmorSector          [KEEP name/class]
   ├─ WBP_ArmorLeft   : WBP_CFArmorSector          [KEEP name/class]
   ├─ WBP_ArmorTop    : WBP_CFArmorSector          [KEEP name/class]
   └─ WBP_ArmorBottom : WBP_CFArmorSector          [KEEP name/class]
```

별도 `WBP_CFArmorSlot`은 만들지 않는다. `WBP_CFArmorSector` 1종을 canonical common Armor element로 유지하되 Production 내부를 **공통 Plate + 별도 Direction Icon + 세로 Ratio Bar** 조립형으로 전환한다.

```text
WBP_CFArmorSector [migration target]
├─ Image_ArmorPlate       // 공통 mechanical Plate
├─ Image_DirectionIcon    // Arrow 또는 Chevron2 / Designer rotation
├─ Text_Direction         // icon missing fallback/debug only
└─ ProgressBar_Armor      // 기존 runtime ratio sink
```

방향별 완성 Plate Texture 6종을 새 Production 계약으로 만들지 않는다. Source Icon은 `T_UI_ArmorIcon_Arrow`, `T_UI_ArmorIcon_Chevron2` **2종만** 제공하고 canonical 방향은 `→`로 통일한다. 각 `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` 인스턴스의 **위치·크기·Icon 종류·Icon 회전은 UMG Designer가 소유**한다. 화면상 좌/우/상/하 위치는 Gameplay 방향 의미가 아니며 Runtime 의미는 기존 instance name과 Presenter binding이 계속 소유한다.

중앙 `Image_VehicleSilhouette`도 전역 한 장을 최종값으로 고정하지 않는다. 현재 Pawn의 `VehicleData` identity로 `UCFHUDVisualData`의 차량별 silhouette catalog를 조회하고, 미등록 차량만 기존 단일 `VehicleSilhouette`을 compatibility fallback으로 사용한다. UI 요구만으로 `UCFVehicleData` schema/DAUTH Registry를 확장하지 않는다.

```text
WBP_CFDefenseBar                             [NEW reusable Widget]
└─ CanvasPanel_Root
   ├─ Image_DefenseFrame                    [Target-derived static frame + dark track shell]
   ├─ Image_DefenseIcon                     [Shield / Integrity semantic icon]
   ├─ Text_DefenseLabel                     [SHIELD / INTEGRITY]
   ├─ ProgressBar_Defense                   [runtime ratio sink; Target-derived Fill Material]
   ├─ Text_DefenseValue                     [Current / Max]
   └─ Canvas_RecoveryChevrons               [dynamic fill-end chevron presentation]
```

`ProgressBar_Defense`는 값 sink로 재사용할 수 있지만 최종 Track/Fill은 Target-derived brush/UI Material이 소유한다. 신규 `UCFDefenseBarWidget` 같은 Presentation base가 필요하면 `Runtime Binding` 단계에서만 추가하고 Gameplay source를 이동하지 않는다.

#### 5.6.4 Slot Contract Review에서 반드시 닫을 항목

```text
1. **RESOLVED / USER APPROVED:** 상단 Speed/RPM : Armor = 50:50. 896×416 기준 `422 + 12 + 422`. 기존 35~40:55~60은 Historical.
2. **RESOLVED / CURRENT REBASE:** repository canonical Master `1280×594` → `896×416`은 uniform scale `0.7`을 사용한다. `594×0.7=415.8`은 최종 416px로 반올림하며 crop/stretch하지 않는다. 과거 1536×713 / 7⁄12 좌표계는 Historical local evidence에만 사용한다.
3. **RESOLVED / FRAME MASTER RESET:** 기존 `Border_Surface` 9-Slice는 구조/호환 underlay로만 보존한다. full-aspect `Image_FrameOverlay`를 **Vehicle Panel Frame Master**로 승격해 outer shell, layered inner-surface edge language, corner detail, top accent, 상단 50:50 central seam, 하단 Defense deck와 outer shell의 연결 관계를 함께 소유한다. `SpeedFrame/ArmorFrame/GearFrame/DefenseBarFrame`은 이 Master 아래의 subordinate local frame이며 독립된 새 시각 언어를 만들지 않는다.
4. **RESOLVED / USER APPROVED:** Armor는 기존 `WBP_CFArmorSector` 1종을 재사용하되 공통 Plate + 별도 Direction Icon + Ratio Bar 조립 구조를 사용한다. 방향별 완성 Plate 6종은 Production 계약이 아니다.
5. **RESOLVED / USER APPROVED:** Direction Texture는 `Arrow` / `Chevron2` 2종만 제공하고 canonical `→`에서 Designer가 회전한다. Sector의 위치·크기·Icon 종류·회전은 Designer 소유, Runtime 방향 의미는 instance name/Presenter binding 소유. `Text_Direction`은 fallback/debug 전용.
6. **RESOLVED / USER APPROVED:** RPM은 작은 `RPM` Label만 유지. `x1000`, 절대 RPM 숫자 눈금은 제거. 21 Tick + 85% Red Zone + dynamic `RPMRatio`가 의미를 전달.
7. **RESOLVED:** Defense는 static Frame/Track shell + Fill Mask/Hex pattern + UI Material로 분리. Ratio는 기존 ProgressBar sink가 clip하고 semantic color/glow는 Material이 소유.
8. **RESOLVED / USER APPROVED:** Target의 `>>>` 모양만 채택하고 fixed decoration으로 쓰지 않는다. Shield 재생/Integrity 수리 Chevron은 기존 계약대로 `▶/▶▶/▶▶▶`가 현재 Fill 끝점을 따라 움직인다.
9. **RESOLVED:** 신규 `WBP_CFDefenseBar` 실제 삽입과 Presenter nested binding migration은 하나의 atomic Runtime Binding slice로 수행해 중간 파손 상태를 만들지 않는다.
```

Slot Contract Review의 기존 9개 의미 계약과 `VT-VEH-01 Production Visual Target` USER lock은 유지한다. 다만 2026-08-26 직접 비교에서 current P2 `T_UI_VehPanelFrame`이 단일 팔각형 9-Slice에 불과해 Master의 상위 frame hierarchy를 재현하지 못하는 **새 관련 Visual failure**가 확인됐다. 따라서 Frame 항목만 재개하며, repository canonical Master binding은 이미 완료된 상태에서 **Frame Master source 재추출 → frame-only review → UMG assembly** 순으로 진행한다. RPM/Speed/Gear/Silhouette의 세부 재배치는 Frame Master가 고정된 뒤에만 수행한다.

#### 5.6.5 Physical Art Breakdown Contract

##### VT10 Production-First Static Asset Checkpoint — 2026-08-28

VT09 Reference Structure Review를 확인한 뒤 완성 Concept 재생성을 중단하고, 이 Physical Art Breakdown Contract의 가장 큰 정적 Asset부터 실제 Source로 생성했다.

```text
VT10_FrameOverlay.png 896×416
→ planned T_UI_VehFrameOverlay / Image_FrameOverlay
→ canonical VT-VEH-01 outer shell 픽셀을 그대로 사용하고 내부 runtime 영역만 transparent cleanup

VT10_SpeedFrame.png 422×272
→ planned T_UI_SpeedFrame / WBP_CFSpeedGauge.Image_SpeedFrame
→ target-derived static Speed/RPM module surface만 유지
→ RPM track/tick, Speed text/unit, Gear frame/value 영역은 transparent

VT10_ArmorFrame.png 422×272
→ planned T_UI_ArmorFrame / WBP_CFArmorBodyMap.Image_ArmorFrame
→ target-derived static Armor module surface/backdrop만 유지
→ vehicle silhouette, 6 Armor Sector, ARMOR label 영역은 transparent
```

세 Asset 모두 `VT_VehPanel01.jpg` exact SHA를 먼저 검증한 뒤 0.7 scale reference pixel을 직접 사용한다. 임의 장식/새 frame pixel 생성은 0이다. 기존 RPM Material, modular Armor, vehicle-specific silhouette, Defense family는 재제작하지 않는다. `VT10_StaticAssetsReview.png`는 세 실제 Asset을 isolated checkerboard 위에 보여주는 QA board일 뿐 Concept Target이 아니다. USER Static Asset Review 전에는 UE Import/Designer assembly를 수행하지 않는다.

아래는 실제 파일 생성 전 고정하는 Production 파츠 계약이다. `Existing Identity`는 현재 asset/DA slot 이름을 재사용하고 픽셀만 Target-derived Art로 교체한다. `NEW`만 새 asset identity다. 기준 Master는 USER APPROVED / LOCKED `VT-VEH-01`이며 현재 대화 첨부 원본의 저장소 Source binding만 남아 있다.

| Asset / Identity | 상태 | 제작 방식 | WBP Consumer | Runtime 역할 |
| --- | --- | --- | --- | --- |
| `T_UI_VehPanelFrame` | Existing / underlay | 기존 9-Slice 호환 구조 유지. 최종 ornament authority는 아님 | `Border_Surface` | 없음 |
| `T_UI_VehFrameOverlay` | **NEW** | Master 전체 외곽 frame을 target-preserving cleanup한 full-aspect RGBA overlay | `Image_FrameOverlay` | 없음 |
| `T_UI_SpeedFrame` | **NEW** | 좌측 module bevel/inner line만 추출·cleanup | `Image_SpeedFrame` | 없음 |
| `T_UI_ArmorFrame` | **NEW** | 우측 Armor module bevel/inner line만 추출·cleanup | `Image_ArmorFrame` | 없음 |
| `T_UI_GearFrame` | **NEW** | Gear box frame만 추출하고 `GEAR/3` 픽셀은 제거 | `Image_GearFrame` | 없음 |
| `T_UI_RPMTrack` | Existing identity refresh | Master의 비대칭 track silhouette를 기준으로 **text/number 없이** 21-tick grayscale mask 재작성 | `M_UI_RPMGauge` texture input | `RPMRatio` 시각화 기반 |
| `M_UI_RPMGauge` | Existing Material | 현재 single MID 구조 유지. Cyan fill + 85% red zone + shallow glow를 Target에 맞춰 parameter polish | `Image_RPMGauge` | `RPMRatio` |
| `T_UI_VehSil_LF` | Existing / compatibility fallback | 기존 단일 silhouette identity는 migration fallback으로만 보존. 최종 Production은 VehicleData identity별 `T_UI_VehSil_<VehicleId>` 계열을 HUD Visual catalog에서 선택 | `Image_VehicleSilhouette` | 현재 VehicleData별 visual 선택 |
| `T_UI_Armor_*` 6종 | Existing / compatibility fallback | 기존 방향별 Plate 6종은 persisted 호환을 위해 보존하지만 신규 Production Art authority로 사용하지 않음 | 기존 `Image_ArmorPlate` | migration fallback |
| `T_UI_ArmorPlate` | **NEW / common** | VT-VEH-01의 mechanical plate language를 공통 tintable Plate로 정리. 방향 glyph/Text 없음 | `Image_ArmorPlate` | Armor 상태색 tint 대상 |
| `T_UI_ArmorIcon_Arrow` | **NEW / common** | VT-VEH-01 단일 Arrow glyph를 target-preserving 추출. canonical `→` | `Image_DirectionIcon` | 없음 |
| `T_UI_ArmorIcon_Chevron2` | **NEW / common** | VT-VEH-01 double-chevron glyph를 target-preserving 추출. canonical `→` | `Image_DirectionIcon` | 없음 |
| `T_UI_DefBarFrame` | **NEW / common** | Shield/Integrity 공통 dark mechanical frame + track shell. grayscale/tintable, runtime 값·Label 없음 | `Image_DefenseFrame` | 없음 |
| `T_UI_DefFillMask` | **NEW** | slanted-left / inset fill 영역의 grayscale mask | `M_UI_DefFill` | Fill shape |
| `T_UI_DefHex` | **NEW** | 작은 반복 honeycomb grayscale pattern | `M_UI_DefFill` | Fill pattern |
| `M_UI_DefFill` | **NEW UI Material** | Mask × hex pattern + semantic color + narrow edge glow. Gameplay 판정 없음 | `ProgressBar_Defense` Fill Brush | ProgressBar Ratio clip |
| `T_UI_DefChevron` | **NEW** | Master의 chevron line language를 target-preserving icon으로 정리 | `Canvas_RecoveryChevrons` | `RecoveryChevronCount` 표시 |

고정 Label/Runtime 값은 Texture로 만들지 않는다.

```text
TextBlock 유지:
- RPM
- Speed / km/h
- GEAR + 실제 Gear 값
- SHIELD / INTEGRITY
- Defense Current / Maximum

Texture에 넣지 않음:
- x1000
- RPM 절대 숫자 눈금
- Armor FRONT/REAR/LEFT/RIGHT/TOP/BOTTOM Text
- 076 / 3 / 784/1000 / 915/1000 같은 샘플 값
```

Semantic Shield/Integrity Icon은 우선 `DA_CFUIStyle_Default.IconSet`의 기존 의미 아이콘을 사용한다. 실제 Visual Match에서 Target과 family가 명백히 어긋날 때만 Style owner에서 교체하며 HUD 전용 중복 아이콘을 먼저 만들지 않는다.

Master source는 flattened bitmap이므로 Frame/Gear/Silhouette처럼 주변 픽셀이 섞인 파츠는 단순 rectangular crop을 Production Asset으로 사용하지 않는다. **Target-preserving alpha extraction / cleanup**을 수행하고, 없는 디테일을 새 스타일로 임의 생성하지 않는다. AI 편집을 쓰더라도 이 cleanup 범위를 넘는 새 픽셀은 별도 USER 승인 전 Production에 넣지 않는다.

Production 생성 순서는 다음으로 고정한다.

```text
A. VT-VEH-01 explicit USER lock                     = DONE
B. Master Source repository binding                 = DEFERRED / USER LOCAL ACCESS PENDING
C. Frame family extraction/cleanup                  = USER PASS / LOCAL PRE-BINDING EVIDENCE READY
D. Speed/RPM family                                  = RPM-S1 USER PASS
E. Armor modular family                              = RUNTIME/WBP TECHNICAL PASS / COMMON PLATE + DIRECTION 2-ICON + VEHICLE-SPECIFIC SILHOUETTE SOURCE/UE ASSETIZED PASS
F. Defense common frame/mask/hex/chevron + M_UI_DefFill
G. Source-side visual review sheet
H. USER source-art approval
I. Unreal Import / UMG Assembly
J. Runtime Binding migration
```

#### 5.6.6 Frame Family Source Extraction Plan — VT-VEH-01

목적은 flattened `VT-VEH-01`에서 **프레임 시각 언어만 분리**하고 Speed/RPM·Armor·Defense 정보 픽셀을 섞지 않는 것이다. Extraction은 원본 native 1536×713에서 수행하고 WBP 896×416은 표시 좌표계로만 사용한다.

##### F0 — Master source binding / 좌표계

```text
Canonical repository path:
SourceArt/UI/HUD/VT/VT_VehPanel01.jpg

Master Source = 1280×594
WBP Target    = 896×416
Uniform Scale = 0.7
Inverse       = 10/7 ≈ 1.428571
SHA-256       = cf5b0a57b76f9c41272ee01ed752d263a7b01a78bf02e9cb9dc042d000df6b6b
```

`Target X/Y → Source X/Y = Target ÷ 0.7`을 현재 extraction 좌표계로 사용한다. 원본 높이 `594 × 0.7 = 415.8`이므로 최종 416px과의 차이는 반올림으로만 처리하고 stretch/crop하지 않는다. 이 repository canonical source는 2026-08-25 exact hash readback이 완료됐으며 immutable authority로 취급한다. 과거 1536×713 / SHA `d795...`는 F7 Historical local evidence를 재현할 때만 사용하고 Current Production extraction 좌표계로 사용하지 않는다.

##### F1 — `T_UI_VehFrameOverlay`

```text
Source working canvas: full 1536×713
Display canvas:        896×416

Keep:
- outermost mechanical chassis frame / stepped side rail
- layered corner caps / chamfer / nested bevel / perimeter rail
- panel perimeter에 속하는 restrained cyan/blue accent
- 상단 shell과 한 몸으로 읽히는 interrupted top accent rail
- 상단 좌/우 module을 묶는 central seam의 Master-level 부분
- 상단 content 영역과 하단 Defense deck를 나누는 공통 horizontal deck seam
- 하단 Defense deck의 outer shell / base / foot 관계

Alpha out:
- RPM track / Speed digits / Gear 값 같은 dynamic·content pixel
- Armor vehicle/six-sector content
- Shield/Integrity label/value/fill
- 개별 Defense row의 runtime fill 영역
- local module 내부 장식 중 `SpeedFrame/ArmorFrame/GearFrame/DefenseBarFrame`이 따로 소유하는 detail

Frame Master는 단순 outer-border crop이 아니다. **외곽 형상 + 내부 surface 경계 + 코너 + top accent + central seam + lower deck hierarchy**를 한 좌표계에서 고정해 이후 모든 content 배치의 상위 기준이 된다.
```

이 자산은 전체 위치 정합용 full-aspect overlay이므로 tight crop하지 않는다. native canvas를 유지해 `Image_FrameOverlay`를 896×416 전체에 놓았을 때 Target의 외곽 frame 위치가 그대로 정렬돼야 한다.

##### F2 — `T_UI_SpeedFrame`

896×416 계약 `SpeedGauge 20,16 / 422×272`의 source window:

```text
Target Slot:   x20..442 / y16..288
Source Window: x34..758 / y27..494  [outward-rounded]
Native Window: 약 724×467
Display Slot:  422×272

Keep:
- 좌측 module outer/inner bevel line
- module corner notch
- frame에 귀속된 restrained cyan accent

Alpha out:
- RPM track / tick / red zone
- RPM label
- Speed digits / km/h
- Gear frame + Gear text/value
- frame 구조와 무관한 background noise
```

SourceArt는 native window 해상도를 보존하고 UMG에서 422×272로 표시한다. RPM Track을 이 Frame에 합치지 않아 기존 `M_UI_RPMGauge + T_UI_RPMTrack + RPMRatio` 동적 구조를 보호한다.

##### F3 — `T_UI_ArmorFrame`

896×416 계약 `ArmorBodyMap 454,16 / 422×272`의 source window:

```text
Target Slot:   x454..876 / y16..288
Source Window: x778..1502 / y27..494 [outward-rounded]
Native Window: 약 724×467
Display Slot:  422×272

Keep:
- 우측 module outer/inner bevel line
- module corner notch
- frame에 귀속된 매우 옅은 line accent

Alpha out:
- Vehicle wireframe
- 6 Armor icon/card
- Armor ratio bar
- 방향 Text
```

Target의 tech ring/grid가 Frame과 분리 가능한 독립 장식이라면 `Image_ArmorBackdrop` 후보로 Review에서 제안할 수 있지만 USER 승인 전 신규 asset identity를 자동 추가하지 않는다.

##### F4 — `T_UI_GearFrame`

Gear는 SpeedGauge 내부 nested frame이라 tight alpha extraction을 사용한다.

```text
Preliminary source work window:
x580..694 / y248..420

Approx native window:      114×172
Target-scale equivalent:   약 67×100
```

위 좌표는 final crop이 아니라 여유 작업창이다. 첫 alpha mask preview에서 실제 Gear frame pixel bounds를 측정하고 6~10px native transparent bleed를 둔 뒤 final crop을 동결한다.

```text
Keep:
- Gear box outline / bevel / cyan line language

Alpha out:
- `GEAR` sample text
- sample Gear value `3`
- 주변 Speed background
```

고정 `GEAR` label과 실제 Gear 값은 TextBlock으로 유지한다.

##### F5 — Extraction execution rule

flattened Master에서 rectangular crop을 그대로 Production Asset으로 저장하지 않는다.

```text
실행 순서:
Mask 기반 alpha extraction
→ edge/background contamination cleanup
→ 원본 위 pixel-aligned overlay comparison
→ 7/12 축소 mock comparison
```

허용:
- 원본 pixel 선택/마스킹
- alpha/background cleanup
- sample Text/숫자 제거 후 투명 처리
- dark background contamination 제거
- edge alpha 정리

조건부 허용:
- 제거할 Text가 frame line을 가린 경우, 양쪽에서 동일 line 연속성이 명확할 때 그 선만 최소 복원

금지:
- frame 두께/shape 재디자인
- 다른 Concept의 corner/notch 추가
- glow 범위 확대
- 좌우 module을 더 대칭적으로 만들기 위한 임의 수정
```

##### F6 — Source-side Review Sheet Gate

Frame family 4종은 UE Import 전에 한 장의 Review Sheet로 USER가 먼저 확인한다.

```text
Review candidate:
VT_VehFrame_Review.png

구성:
1. VT-VEH-01 원본 축소본
2. T_UI_VehFrameOverlay — checkerboard
3. T_UI_SpeedFrame — checkerboard
4. T_UI_ArmorFrame — checkerboard
5. T_UI_GearFrame — checkerboard
6. 896×416 frame-only mock composite
```

PASS 기준:
- 원본과 corner/notch/line weight가 같은 family로 읽힘
- Speed/Armor module frame이 50:50 Slot에 정렬
- RPM/Armor/Defense 정보 픽셀이 Frame asset에 남지 않음
- 검은 alpha halo 없음
- sample Text/숫자 잔존 없음
- frame-only 재조립 mock이 원본 frame silhouette에서 눈에 띄게 drift하지 않음

USER Frame Review PASS 뒤에만 Unreal Import 후보로 승격하고 다음 Speed/RPM family extraction으로 이동한다.

##### F7 — 2026-08-25 Local Pre-Binding Extraction Evidence

USER가 현재 PC에서 repository source copy를 수행할 수 없는 상태이므로 Source Binding은 억지로 우회하지 않고 **Deferred**로 유지했다. 대신 현재 대화 첨부 원본과 동일 SHA를 갖는 local working source를 기준으로 Frame F1~F4를 deterministic alpha extraction해 USER 검토 가능한 증거를 먼저 완성했다.

```text
VT-VEH-01 local working source
1536×713
SHA-256 d795f09531c41edbeb460a7391a337d77a7e7c582b5c3e661f1032b5d173ccae

F1 VT01_F1_Overlay.png
1536×713 RGBA
SHA-256 82baf36ff1f00166c6d5398fe86fc5d9ce4149ae61d4587c2a9750877eb36645

F2 VT01_F2_Speed.png
724×467 RGBA
SHA-256 47f2aeed00fa5fe832021f5753390148c41351dc25c6fd758d75a3e73cd44bbc

F3 VT01_F3_Armor_v2.png
724×467 RGBA
SHA-256 aeddd7b06ddc554419746ec62b87b9e48d3b5e9e8a0da6a20bae9bbc0606b47f

F4 VT01_F4_Gear.png
114×172 RGBA
SHA-256 0442a1ac31515a97e5b9c44a7169295edc7eaf110cb65930f0b84f9562a1ef4a

Frame-only Mock v2
896×416 RGBA
SHA-256 b92c7b318d3390a245f77e436ba0d98607c5ffab687c60ce24c681de66abf512

Frame Review Sheet v2
1840×1420 RGB
SHA-256 10af0ddfd7432bb07daac3681d95c86d8b0f1aef6049a20b61c2c71d0e99d356
```

F3 v2는 기존 Draft에서 보이던 Armor card/vehicle/ring 잔여 alpha를 제거하기 위해 Master 원본 RGB를 그대로 보존한 채 **상단·우측·하단의 고신뢰 frame band만 alpha로 남기는 보수적 cleanup**을 적용했다. 새 line, corner, bevel, glow를 생성하지 않았고 left divider는 불확실 픽셀을 억지 복원하지 않아 Root/Overlay와의 조립 책임으로 남겼다.

이 local 산출물은 **Pre-Binding Evidence**이지 repository Production Source가 아니다. USER는 `VT01_FrameReview_v2`를 확인하고 당시 Frame Family를 명시 PASS했다. 그러나 2026-08-26 canonical Master와 current P2 frame을 직접 비교하면서 상위 Frame hierarchy 불일치가 새 관련 failure로 확인됐으므로, 당시 PASS는 Historical extraction 품질 evidence로만 보존하고 Current Production Frame Match를 닫는 근거로 사용하지 않는다.

##### F8 — Frame Master Reset Contract — 2026-08-26

이번 재개 범위는 RPM 미세조정이나 content 재배치가 아니라 **Vehicle Panel Frame Master 자체의 시각 구조 고정**이다.

```text
Frame hierarchy
F0  Outer Mechanical Shell
F1  Corner Mechanical Detail
F2  Top Accent Rail
F3  Lower Outer Base / Foot
F4  Subordinate Local Frames: Speed/RPM / Armor / Gear / Defense Row
F5  Dynamic Content: RPM / Speed / Gear / Silhouette / Armor / Defense values
```

**외곽 형상**
- current P2의 단순 대칭 팔각형을 Production authority로 사용하지 않는다.
- VT-VEH-01의 stepped side, chamfered corner, nested rail, 하단 foot/base가 하나의 mechanical chassis silhouette로 읽혀야 한다.
- 좌우 corner/detail을 임의로 완전 대칭화하지 않는다.

**내부 면 / 임시 가이드 금지**
- Frame Master Source에는 상단 좌/우 safe-area 박스, 중앙 세로선, main→Defense 가로선, lower deck 큰 사각형처럼 **레이아웃 확인용 임시 geometry를 렌더링하지 않는다**.
- 422+12+422, Defense 2-row와 같은 safe-area 수치는 문서/Designer layout contract일 뿐 Production Frame pixel이 아니다.
- Speed/RPM, Armor, Gear, Defense Row의 실제 내부 셸은 각 subordinate local frame이 소유한다. Frame Master가 임시 사각형으로 대신 그리지 않는다.
- panel 내부의 dark cockpit surface는 WBP underlay/background가 담당하며 Frame Master PNG가 큰 내부 면을 채우지 않는다.

**코너 디테일**
- 45도 chamfer, 짧은 step plate, double-line bevel을 Master에서 보존한다.
- cyan은 구조를 읽게 하는 얇은 accent로 제한하고 corner 전체를 발광시키지 않는다.

**상단 accent**
- 별도 floating neon bar가 아니라 outer shell에 결합된 interrupted/stepped rail로 취급한다.
- 중앙과 양끝의 break/step이 panel 폭과 50:50 module 분할을 보조하되 content보다 먼저 읽히지 않는다.

**하단 Integrity/Defense 관계**
- Shield와 Integrity는 최종 화면에서 같은 하단 정보층에 2-row로 쌓이지만, 그 관계를 **Frame Master의 큰 임시 deck 박스**로 표현하지 않는다.
- Frame Master는 panel 전체의 lower outer base/foot까지만 소유한다.
- Shield/Integrity의 개별 셸과 두 row 사이 시각 관계는 `T_UI_DefBarFrame` 계열 및 Designer assembly가 소유한다.
- Integrity row가 panel 하단을 시각적으로 지지한다는 composition 원칙은 유지하되, layout guide 자체는 렌더링하지 않는다.

**내부 구획 비례**
- 전체 canvas는 `896×416`을 유지한다.
- 상단 좌/우 content는 기존 USER APPROVED `422 + 12 + 422` 50:50 layout contract를 유지하지만 **해당 경계선을 Frame Master PNG에 그리지 않는다**.
- main-content → lower Defense 영역은 Master 원본 기준 대략 panel 높이의 **67% 부근**을 layout 측정 기준으로만 사용하며, 실제 구분선은 subordinate frame/Designer 조립 단계에서 target에 맞춰 결정한다.
- lower Defense 영역은 두 row + 내부 spacing + bottom structural margin을 확보하되 그 safe-area rectangle은 review/Production pixel로 남기지 않는다.
- RPM/Speed/Gear/Silhouette/Armor Sector의 실제 위치는 outer shell USER PASS 뒤 Designer에서 재배치한다.

**Current implementation gap**
- repository `SourceArt/UI/HUD/P2/T_UI_VehPanelFrame.png`는 512×256 단일 9-Slice octagonal frame이며 Frame Master의 F1~F5 hierarchy를 포함하지 않는다.
- current source/C++에는 `T_UI_VehFrameOverlay` / `Image_FrameOverlay` 실제 구현이 아직 없다.
- 따라서 기존 `Border_Surface + VehiclePanelFrame`은 compatibility underlay로 보존하되 최종 Visual authority가 아니다.

**Gate order**
```text
1. canonical 1280×594 source에서 outer-shell-only Frame Master extraction/cleanup
2. frame-only 896×416 review로 outer shell / corner / top rail / lower outer base 확인
3. USER Frame Master Visual PASS
4. `Image_FrameOverlay` full-aspect Designer layer + Production Texture assembly
5. subordinate Speed/RPM / Armor / Gear / Defense local frame 조립
6. 그 뒤에만 RPM / Speed / Gear / Silhouette / Armor 내부 배치 재정렬
```

**강제 규칙:** safe-area box, center split guide, deck guide처럼 임시 배치 확인용 geometry는 문서/Review annotation으로만 존재할 수 있으며 Production Frame Source/Texture에는 절대 남기지 않는다.

Frame Master PASS 전에는 RPM 미세조정이나 content 위치값을 최종화하지 않는다.

##### F9 — Frame Master v4 Pre-USER-PASS Candidate — 2026-08-26

canonical `VT-VEH-01`에서 Frame Master만 분리하는 deterministic one-shot builder를 작성해 `896×416` Source/Review 후보를 생성했다. AI replacement art나 생성형 이미지가 아니라 **canonical source pixel + 기하 ownership mask + neutral shared-surface fill + deterministic cool structural line**만 사용한다.

```text
Canonical source
SourceArt/UI/HUD/VT/VT_VehPanel01.jpg
1280×594
SHA-256 cf5b0a57b76f9c41272ee01ed752d263a7b01a78bf02e9cb9dc042d000df6b6b

Current Frame candidate
SourceArt/UI/HUD/P2/VT06_FrameMaster_v4.png
896×416
SHA-256 b5f48e99ac761f39adae106b403ab6e917e2c82637f2734929d7d8ef00e516bf

USER review sheet
SourceArt/UI/HUD/P2/Review/VT06_FrameReview_v4.png
1832×860
SHA-256 6225b2c7ebb222943e385e63eec2aff1752bef99bcef83d4cb831cebd9d666b2

Manifest
SourceArt/UI/HUD/P2/VT06_FrameManifest_v4.json
Builder = Tools/BuildFrameMaster.py v1.3.0
Wrapper = Tools/BuildFrameMaster.ps1 v1.0.0
```

AI Source QA 과정:
- `v1`: right Armor card/central seam의 orange content pixel contamination → FAIL.
- `v2`: warm-color filter 후에도 dark orange local Armor frame residue → FAIL.
- `v3`: local Speed/Armor frame source ownership을 제거했지만 main/deck separator source band에 얇은 Armor residue → FAIL.
- `v4`: separator도 deterministic cool structural line으로 전환. dynamic/content pixel contamination이 보이지 않고 outer shell / corner / top rail / shared surface / 50:50 central seam / lower deck outer base가 독립적으로 읽혀 **AI Source QA PASS**.

v4 Frame Master ownership:
- canonical source pixel 보존: outer shell, corner mechanical detail, top rail, lower deck outer/base.
- deterministic master line: shared upper inset edge, upper 50:50 central seam, main→Defense deck separator.
- neutral surface: shared upper cockpit surface + lower common Defense deck surface.
- intentionally excluded: RPM/Speed/Gear content, vehicle silhouette, Armor card/value, Shield/Integrity label/value/fill, Defense row local frame.
- Review 우하단은 upper left/right safe-area와 lower Shield/Integrity 2-row safe-area를 guide로만 표시하며 Production Source에 guide color를 넣지 않는다.

검증 경계:
```text
AI Source QA = PASS
USER Frame Master Visual PASS = PENDING
UE Import = 0
UE Asset mutation = 0
WBP/Designer mutation = 0
RPM/Speed/Gear/Silhouette/Armor reposition = 0
```

2026-08-26 USER는 `VT06_FrameMaster_v4`의 내부 safe-area/central seam/deck rectangle을 **임시 박스가 최종 Frame에 남은 것**으로 판정해 명시 Reject했다. USER 피드백은 “reference에는 그런 임시 박스가 없으며 임시 geometry는 최종 결과에 남기면 안 된다”이다.

이에 builder v1.4.0에서 Frame Master ownership을 outer shell / corner / top rail / lower outer base로 축소하고 모든 내부 임시 geometry를 제거해 v5를 생성했다.

```text
Current Frame candidate
SourceArt/UI/HUD/P2/VT06_FrameMaster_v5.png
896×416
SHA-256 e560caafd4597534769364df4ff5ad8fe7c5fa22e4407f00d6fc012c226a59a5

USER review sheet
SourceArt/UI/HUD/P2/Review/VT06_FrameReview_v5.png
1832×860
SHA-256 45b18df65c2315682f35934b5bc80a6e3419751e0be53b725045cc0e4e48d306

Manifest
SourceArt/UI/HUD/P2/VT06_FrameManifest_v5.json
Builder = Tools/BuildFrameMaster.py v1.4.0
```

v5 직접 Image QA에서 내부 box/seam/deck guide pixel 0, canonical outer mechanical shell/corner/top rail/lower outer base만 남은 것을 확인해 **AI Source QA PASS**했다. Review의 비교 패널도 guide geometry 대신 dimmed canonical target 위에 exact v5 shell만 겹친다.

따라서 다음 exact Gate는 **USER가 `VT06_FrameReview_v5.png`를 보고 outer-shell-only Frame Master를 판정하는 것**이다. USER PASS 전에는 `Image_FrameOverlay` Production assembly나 content 재배치를 시작하지 않는다.

#### 5.6.7 RPM-S1 Source Mask Pack — 2026-08-25

`VT-VEH-01`의 작동형 RPM 미터를 현재 `Image_RPMGauge + M_UI_RPMGauge + RPMRatio` 단일 구조에 맞추기 위해 Local Source Mask Pack을 제작했다. 구형 `ProgressBar_RPMTick00~20` 다중 Widget 경로는 재도입하지 않는다.

Production 의미 계약:

```text
RPM path = 좌측 세로 → 좌상단 1/4 곡선 → 상단 긴 수평
TickCount = 21
RedStartRatio = 0.85
RPM label = TextBlock `RPM`
x1000 = 표시 안 함
절대 RPM 숫자 눈금 = 표시 안 함
Runtime owner = existing M_UI_RPMGauge + RPMRatio
```

초기 local mask 제작에서 곡선 중심선 각도 방향이 반대로 잡혀 path jump가 생기고, anti-aliased segment 일부가 합쳐져 connected-component 17~19개로 검출되는 결함을 자동 QA에서 발견했다. 이를 Production 후보로 승격하지 않고 곡선을 **왼쪽 180° → 위쪽 270°**로 교정해 vertical→curve→horizontal tangent를 연속화했다.

최종 Source Mask QA:

```text
Canvas = 724×467
Tick connected components = 21
Progress levels = 21
Red ticks = 4
RpmRatio 0.00 = 0 ticks
RpmRatio 0.25 = 5 ticks
RpmRatio 0.50 = 10 ticks
RpmRatio 0.85 = 17 ticks
RpmRatio 1.00 = 21 ticks
AUTO QA = PASS
```

분리 Source는 다음 역할로 생성했다.

```text
T_UI_RpmFrame       = target geometry 기반 clean double guide rail
T_UI_RpmTrackOff    = inactive tick mask reference
T_UI_RpmFillMask    = 21단계 grayscale progress map
T_UI_RpmTickMask21  = 21개 독립 binary segment mask
T_UI_RpmRedMask     = 마지막 4 Tick / 0.85+ QA reference mask / Runtime Import 안 함
VT02_RpmLabelRef    = `RPM` typography reference only / UE Import 금지
```

실제 UE Import 후보는 위 Source Mask를 여러 Texture로 소비하지 않고 **채널 패킹 1장**으로 압축한다.

```text
T_UI_RPMMaskPack 724×467 RGBA
R = Progress Map
G = TickMask21
B = Guide Rail Mask
A = 255 constant opaque safety channel
SHA-256 6cc32459a972a79c6aa50983050aa5fdb8656a4efa5a7caa9475fda66def8777

Import contract:
sRGB = Off
Mask용 Linear Texture
M_UI_RPMGauge Texture Sample = 1
TrackOff = G channel × inactive brightness로 Material에서 계산
Red Zone = step(RedStartRatio=0.85, R) × G로 Material에서 파생
PNG zero-alpha RGB 변형 위험을 피하기 위해 A는 항상 255
```

따라서 Production WBP 구조는 새 RPM child Image를 추가하지 않는다.

```text
WBP_CFSpeedGauge.Image_RPMGauge [KEEP]
└─ M_UI_RPMGauge [KEEP identity]
   ├─ T_UI_RPMMaskPack [NEW packed source]
   └─ RPMRatio [KEEP runtime parameter]

Text_RPMLabel = TextBlock `RPM`
```

Local Review evidence:
- `VT02_RpmRuntimePack_Review_FINAL3.png` = production-safe packed mask + runtime ratio preview + QA 결과
- `VT02_RPMMaskPack_Review_FINAL.png` = R/G/B + derived Red channel 확인
- `VT02_RPMSourcePack_Manifest_FINAL.json` = exact file SHA/contract manifest

이 Pack은 **Local Source Candidate / USER Visual PASS**이며 아직 repository SourceArt나 `/Game` Production Asset이 아니다. `VT02_RpmRuntimePack_Review_FINAL3` USER PASS를 보존하고 새 관련 결함이 없는 한 RPM Review를 반복하지 않는다. repository Source Binding + exact hash readback은 UE Import 전에 반드시 닫는다.

#### 5.6.8 ARMOR Modular Structure Revision — 2026-08-25

Frame Family PASS 뒤 만든 `VT03` Armor pack은 방향별 완성 Plate 6종과 단일 공통 Vehicle silhouette를 전제로 한 **local experiment**였다. USER는 이 Pack을 PASS하기 전에 Production 구조를 다음처럼 변경했다. 따라서 `VT03_ArmorReview_v2 / VT03_ArmorMock_v2 / VT03_ArmorManifest_v2`는 Superseded local evidence이며 Production 승격 대상이 아니다.

```text
WBP_CFArmorBodyMap [KEEP]
├─ Image_VehicleSilhouette          // current VehicleData identity에 따라 교체
├─ WBP_ArmorFront  : WBP_CFArmorSector
├─ WBP_ArmorRight  : WBP_CFArmorSector
├─ WBP_ArmorRear   : WBP_CFArmorSector
├─ WBP_ArmorLeft   : WBP_CFArmorSector
├─ WBP_ArmorTop    : WBP_CFArmorSector
└─ WBP_ArmorBottom : WBP_CFArmorSector

WBP_CFArmorSector [migration target]
├─ Image_ArmorPlate                 // common plate
├─ Image_DirectionIcon              // Arrow / Chevron2 + Designer rotation
├─ Text_Direction                   // fallback/debug only
└─ ProgressBar_Armor                // existing runtime ratio sink
```

USER ownership:
- 여섯 Sector의 Position / Size / spacing
- 각 Sector의 `Arrow` 또는 `Chevron2` 선택
- 각 Direction Icon의 rotation
- 차량 silhouette의 Designer slot Position / Size

Runtime ownership:
- `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` instance name → Defense Ratio binding
- `SetArmorPercent()`의 0~1 Ratio/visibility 계약
- 현재 Pawn `VehicleData` identity → HUD silhouette catalog lookup

차량 실루엣은 차종별로 달라져야 하므로 `UCFVehicleData`에 HUD 전용 Texture field를 추가하지 않는다. `UCFHUDVisualData`가 `VehicleData identity → Silhouette Texture` catalog를 소유하고 기존 `VehicleSilhouette`은 미등록 차량용 compatibility fallback으로 유지하는 방향을 채택한다. 이는 현재 paused Data Authoring schema를 UI Art 이유로 확장하지 않기 위한 ownership 분리다.

방향 Icon Source는 **2종만** 제작했다.

```text
VT04_ArmorIcon_Arrow.png      = canonical → single arrow / 128×128 / tintable white-alpha
VT04_ArmorIcon_Chevron2.png   = canonical →→ double chevron / 128×128 / tintable white-alpha
VT04_ArmorIcon2_Review.png    = 두 Source + 0/90/180/270° Designer rotation review
VT04_ArmorIcon2_Manifest.json = local source/rotation contract
```

두 glyph 모두 `VT-VEH-01`의 원본 amber icon pixel을 target-preserving alpha extraction한 것이며 새 icon style을 생성하지 않았다. Direction별 6개 Texture는 만들지 않고 UMG rotation으로 재사용한다.

현재 `CFHUDVisualData.VehicleSilhouette`, `ArmorPlates.Front~Bottom`, `CFArmorSectorWidget`의 direction-baked Plate 계약은 **persisted compatibility scaffold**다. 실제 migration 때 새 common Plate / 2 Icon / vehicle-specific silhouette catalog를 additive 도입하고 기존 필드는 즉시 삭제·rename하지 않는다.

현재 상태는 **ARMOR MODULAR RUNTIME/WBP TECHNICAL PASS / COMMON PLATE + DIRECTION 2-ICON SOURCE BOUND + UE ASSETIZED / VEHICLE-SPECIFIC SILHOUETTE PRODUCTION ART PENDING / MASTER SOURCE BINDING PENDING / USER VISUAL PENDING**이다.

##### Armor 2-Icon Production Assetization PASS — 2026-08-25

```text
Source:
- SourceArt/UI/HUD/VT/VT04_ArmorIcon_Arrow.png
- SourceArt/UI/HUD/VT/VT04_ArmorIcon_Chevron2.png
- Project External Image Vision readback: 128×128 / canonical 우향 Arrow·Chevron2 확인

UE Asset:
- /Game/CarFight/UI/HUD/Visual/T_UI_ArmorIcon_Arrow
- /Game/CarFight/UI/HUD/Visual/T_UI_ArmorIcon_Chevron2
- DA_CFHUDVisual_Default.ArmorDirectionArrow / ArmorDirectionChevron2 exact binding

Validation:
- Official Editor Build 8eed02112b73443a9c4df1d03d9a859b PASS
- CFUIArmorArt Exit 0 / imported=2 / bound=2 / saved_packages=3
- fresh AssetDump adset_v1_af4240b727a9713595181ce45d13e44d.a1c14e4b40026b6373270d82 persisted PASS

Boundary:
- ArmorCommonPlate=None
- VehicleSilhouettes count=0
- 6개 Sector Designer Position/Size/Icon Choice/Rotation mutation=0
- USER Visual PASS 미승격
```

첫 headless 실행에서 `GetSizeX/Y()`가 import 직후 0×0을 반환한 것은 `Texture->Source.GetSizeX/Y()` 검증으로 교정했고, 실패 실행은 persisted ArmorIcon 0개임을 확인한 뒤 final run만 저장됐다.

##### Armor Common Plate Production Assetization PASS — 2026-08-25

```text
Source:
- SourceArt/UI/HUD/VT/VT04_ArmorPlate.png
- 98×130 / 1174 bytes
- SHA-256 986635f29722558f5682db633bb8493a50c04b82f1e9d3b2cb21dab33fa5e5be
- Project External Image Vision exact Checkout readback PASS

UE Asset:
- /Game/CarFight/UI/HUD/Visual/T_UI_ArmorPlate
- DA_CFHUDVisual_Default.ArmorCommonPlate exact binding

Validation:
- CFUIArmorArt v1.1.0 `-PlateOnly`
- Official Editor Build 3183bb86d5e7483e9cde64be70c27551 PASS / Exit 0
- process 19d669f0a2c8415a99305aa13133c59a Exit 0
- marker: imported=1 / bound=1 / saved_packages=2 / existing_icons_validated=2
- fresh AssetDump adset_v1_653b88c9306c201ab9f785c79b7431ce.908692ddcf890cb552fe879e persisted PASS

Boundary:
- ArmorCommonPlate + ArmorDirectionArrow + ArmorDirectionChevron2 = non-null field condition 충족
- HasModularArmorArt() 함수 직접 실행 검증은 별도 수행하지 않음
- VehicleSilhouettes count=0
- 6개 Sector Designer Position/Size/Icon Choice/Rotation mutation=0
- USER Visual PASS 미승격
```

##### Vehicle-specific Silhouette Source Authority + Identity Mapping PASS — 2026-08-25

persisted `CFVehicleData` 3종의 `VehicleVisualConfig.ChassisMesh`를 exact Asset identity 기준으로 다시 읽어 silhouette의 형상 Source Authority를 확정했다.

```text
Runtime catalog key
= exact CFVehicleData asset identity

Shape Source Authority
= VehicleData.VehicleVisualConfig.ChassisMesh exact asset identity

Production source
= authority ChassisMesh를 탑다운·좌향 HUD 규격으로 투영한 mesh-derived silhouette
```

| VehicleData identity | ChassisMesh identity | Production shape source |
| --- | --- | --- |
| `/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan` | `/Game/CarFight/Vehicles/Meshes/Sedan/Sedan.Sedan` | Sedan 1종 |
| `/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV` | `/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV` | SUV 1종 |
| `/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV` | `/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV` | 위 SUV source 공유 |

따라서 Current Content는 **3 VehicleData / 2 unique ChassisMesh / 2 production shape source**다. Runtime catalog는 VehicleData별 entry를 유지하되 `DA_TestSUV`와 `DA_VehicleDefense_TestSUV`가 같은 SUV silhouette Texture를 가리키는 것을 허용한다. 이를 위해 `UCFVehicleData` schema나 별도 UI VehicleType/VisualId를 추가하지 않는다.

Authority 제외:

- `SourceArt/UI/HUD/P2/T_UI_VehSil_LF.png`는 `GenerateUIHUDArtP2.ps1`가 고정 좌표 polygon으로 그린 generic 전투차량 Technical Reference다. 실제 Sedan/SUV Mesh에서 파생되지 않았으므로 특정 VehicleData Production Source Authority가 아니다.
- `VT03_VehSil_LF_v2`는 target-derived silhouette 제작 참고/후보일 뿐 exact ChassisMesh identity와 연결되지 않았다. 따라서 `DA_TestSedan` 등 임의 차량에 매핑하지 않는다.

authority ChassisMesh 기반 `VT05_VehSil_Sedan.png` / `VT05_VehSil_SUV.png`를 실제 **512×256 transparent PNG**로 생성해 형식·방향·서로 다른 형상을 Technical 검증했다. USER가 `문제없는 거면 일단 진행해봐`로 Production 진행을 승인한 뒤 `T_UI_VehSil_Sedan` / `T_UI_VehSil_SUV`를 Assetize하고 `DA_CFHUDVisual_Default.VehicleSilhouettes`를 exact VehicleData identity **3 entry**로 저장했다. Sedan은 Sedan Texture, `DA_TestSUV`와 `DA_VehicleDefense_TestSUV`는 동일 SUV Texture를 공유한다. legacy `VehicleSilhouette=T_UI_VehSil_LF` fallback은 보존한다.

##### Vehicle-specific Silhouette Production Assetization Technical PASS — 2026-08-25

```text
Source:
- SourceArt/UI/HUD/VT/VT05_VehSil_Sedan.png
  - 512×256 PNG RGBA
  - SHA-256 f96c777b9e2af9ac72acea6b926e225e7e32375fd3584f2eddb340483516f0a3
- SourceArt/UI/HUD/VT/VT05_VehSil_SUV.png
  - 512×256 PNG RGBA
  - SHA-256 4d7ad351999637c70f399e3fbf747f8b34b24c016633490230fd42bc370d5231

Production Texture:
- /Game/CarFight/UI/HUD/Visual/T_UI_VehSil_Sedan
- /Game/CarFight/UI/HUD/Visual/T_UI_VehSil_SUV

Persisted catalog:
1. DA_TestSedan → T_UI_VehSil_Sedan
2. DA_TestSUV → T_UI_VehSil_SUV
3. DA_VehicleDefense_TestSUV → T_UI_VehSil_SUV

Compatibility:
- DA_CFHUDVisual_Default.VehicleSilhouette = T_UI_VehSil_LF 유지
- Armor Common Plate / Direction 2-Icon binding mutation 0
- WBP/Designer Layout mutation 0
```

Validation:
- canonical `BuildEditor.bat` + `CFVehSilProd` wrapper process `f7c50e97f58f4ef4bd0e0a64d51474e6` = Exit 0 / PASS
- fresh persisted AssetDump `adset_v1_54b79e58701e4288cd0285c2ced3cda7.e9ce14178c430a4696a0f0ad` = Texture2D 2종 + `VehicleSilhouettes Array[3]` exact mapping PASS
- 별도 catalog-focused Automation test는 현재 Source에 존재하지 않아 새 unrelated broad regression은 실행하지 않았다. compile/build와 persisted binding evidence로 이 content-only slice를 Technical closure한다.

다음 VehiclePanel Physical Art는 **Defense common frame/mask/hex/chevron + `M_UI_DefFill` Source/Production family**다. exact `VT-VEH-01` Master Source repository binding은 이 silhouette closure와 분리된 Pending 경계로 유지한다.

#### 5.6.9 ARMOR Modular Runtime/WBP Technical Closure — 2026-08-25

기존 Designer ownership을 유지한 채 modular 구조의 실행 기반을 실제 코드와 persisted WBP에 적용했다.

```text
Runtime/C++:
- UCFArmorSectorWidget v1.2.1
  - common/legacy Plate + DirectionIconTexture + Text fallback
  - DirectionIconRotationDegrees = per-instance Designer-owned rotation
- UCFHUDPresenter v1.26.1
  - VehicleViewData.VehicleDataAsset → HUDVisualData vehicle silhouette catalog
  - Vehicle identity cache / catalog miss·fallback null 시 stale Brush clear
- CFUIHUDProdEditorBridge h v1.12.1 / cpp v1.15.1
  - existing WBP_CFArmorSector에 Image_DirectionIcon만 additive migration
  - UE 5.8 WidgetVariableNameToGuidMap deterministic GUID 등록

Persisted WBP 당시 Technical Closure:
- WBP_CFArmorBodyMap = 기존 8-node 구조와 six-sector Canvas Slot 보존
- WBP_CFArmorSector = 10-widget 구조 / Overlay_Plate에 Image_DirectionIcon 추가 / 당시 기본 Collapsed
- 이 closure 시점에는 Production Direction Icon Texture가 Source Binding/Import 전이어서 Text_Direction fallback을 유지했다. 이후 common Plate + Direction 2-Icon Production Assetization PASS가 별도 완료됐다.
```

Technical evidence:
- final Official Editor Build `ea1918cf88b0470e91398e9a87840e6b` = PASS / Exit 0
- clean one-asset migration = Exit 0 / `already_migrated=true / saved_assets=0` / ensure 재발 0
- fresh AssetDump dataset `adset_v1_afc4533f958c855c909422e82921af57.6846c1c5a8273c2eae1950b8` = BodyMap Designer Slot 보존 + ArmorSector additive Icon 확인
- `CarFight.UI.UI_P0_06` final focused suite = **17/17 PASS / Fail 0**
- first focused run의 1건 실패는 구형 direction-baked Plate 기대를 새 modular Texture-unbound fallback 계약으로 교정한 뒤 final PASS했다.

이 Technical Closure는 **당시 시점**의 Runtime/WBP foundation만 의미한다. 이후 common Plate / Arrow / Chevron2와 vehicle-specific Sedan/SUV silhouette Texture의 repository Source·Production Import·persisted binding이 별도 PASS로 전진했으므로 Current 상태는 §5.6.8~§5.6.9 이후 최신 closure와 §12 체크포인트를 따른다.

### 5.7 Reference와 Target 구분

```text
Visual Direction Reference
= 분위기·아이디어·색·재질·형태 언어 참고. Composition 일치 의무 없음.

Composition Target
= Panel 구성·비율·Hierarchy·대표 형태 기준.

Production Visual Target
= USER가 실제 구현 기준으로 승인한 목표. Registry의 Must Match 항목은 Deviation 승인 없이 임의 단순화 금지.

Production Asset
= Visual Target을 실제 게임에서 재현하기 위해 분해·제작된 Texture/Material/Icon/Widget 자산.
```

---

## 6. Production UI 제작 Pipeline — LOCKED

CF-FQ-039의 Production UI는 아래 **Target Decomposition Pipeline만** 사용한다. 이 순서는 권장사항이 아니라 현재 Feature의 강제 제작 계약이다.

```text
1. Production Visual Target Lock
   - 실제 구현 기준이 될 Master Concept/Target을 USER가 명시 승인
   - Target Registry에 Source / Must Match / Flexible 기록

2. Logical Breakdown — 실제 PNG를 자르기 전 수행
   - Panel / Frame / Module / Icon / Typography / Decoration 분리
   - Static Art / Dynamic Visual / Runtime Data 분리
   - 재사용 Widget 경계와 각 파츠의 소유권 결정
   - 9-Slice / Mask / UI Material / Text / Image 등 표현 수단 결정

3. WBP Composition Skeleton
   - Logical Breakdown 결과를 기준으로 Widget hierarchy와 상대 배치 구성
   - Target의 비율·Hierarchy·Silhouette를 보존하는 Placeholder Slot만 배치
   - 이 단계의 Border/ProgressBar/임시 Image는 기능·자리 확인용 Scaffold이며 최종 Visual authority가 아님

4. Slot Contract Review
   - 각 Slot의 Bounds / Anchor / Aspect / Crop / 9-Slice margin / Dynamic fill 영역 확정
   - 실제 Art 분해 전에 WBP와 Target의 구조적 대응을 검토

5. Physical Art Breakdown / Production
   - 동일 Master Target에서 실제 Texture / Mask / Frame / Icon / Glow 파츠를 분해·정리·제작
   - WBP가 요구하는 Slot 규격에 맞춰 제작하되 새로운 시각 언어를 임의 창작하지 않음

6. UMG Assembly
   - Placeholder를 실제 Target-derived Art / Material로 교체
   - Designer가 Position / Size / Spacing / Layer composition을 소유

7. Runtime Binding
   - 기존 Provider / ViewData / Presenter의 값·Ratio·상태를 실제 Visual consumer에 연결
   - C++은 Visual을 새로 디자인하지 않음

8. Technical / Pixel Validation
9. Visual Match Review — Master Target과 직접 비교
10. USER Visual PASS
```

핵심 순서는 **`Target Lock → Logical Breakdown → WBP Skeleton → Physical Art Breakdown → Assembly`**다. `Logical Breakdown`과 실제 이미지 파일 분해를 같은 단계로 취급하지 않는다. WBP 전체를 먼저 완성한 뒤 그 구조에 맞춰 컨셉을 재해석하는 것도 금지하고, 반대로 Slot 계약 없이 PNG를 먼저 전부 잘라 놓는 것도 기본 경로로 사용하지 않는다.

### 6.1 Deprecated — 기존 제작 방식 폐기

아래 경로는 2026-08-24부터 CF-FQ-039 Production UI에서 **Deprecated / 사용 금지**다.

```text
- WBP/ProgressBar/Border를 먼저 완성하고 그 외형에 맞춰 Concept을 나중에 끼워 맞추기
- 기본 UMG Primitive에 색·두께·Glow를 조정한 결과를 최종 Production Art로 승인하기
- 승인 Target 대신 AI가 새 패널/프레임/아이콘을 임의 생성하고 그 결과를 바로 Production Asset으로 Import하기
- '비슷한 분위기'의 새 이미지를 만들고 원 Target 대신 Visual authority로 사용하기
- 현재 Technical Scaffold/P2 Technical Reference를 새 Master Target처럼 역승격하기
- 한 장짜리 완성 HUD bitmap에 runtime Text/수치를 굽기
```

AI 이미지 생성은 기본적으로 **새 미감 창작 수단이 아니다.** 승인된 Master Target의 파츠 추출·정리·마스크 보조처럼 원본 미감을 보존하는 보조 작업에만 사용한다. 새로 생성된 시각 자산을 Production에 넣으려면 USER가 해당 결과를 별도 승인해야 한다.

기존 방식으로 이미 만들어진 Widget/Texture/ProgressBar 결과는 삭제 대상이 아니라 **Technical Scaffold / Runtime Contract evidence**로만 보존한다. 이후 Visual 작업에서 그것의 모양을 기준으로 새 Art를 만들지 않으며, Target-derived Art가 준비되면 필요한 범위에서 교체한다.

예외적으로 **이미 Current 구현에 존재하는 Designer-owned reusable structure를 확인·보존·문서화하는 Structure Readiness 작업**은 승인된 Composition Target만으로 수행할 수 있다. 이 예외는 새 Production Art 제작, Texture 교체, Material 외형 변경 또는 `VT-VEH-01` 승격을 허용하지 않는다. VehiclePanel의 현재 Structure Readiness는 `WBP_CFArmorBodyMap Canvas + 탑다운 VehicleSilhouette + WBP_CFArmorSector 6개 재사용`을 기준으로 한다.

기존 Target을 교체할 경우 이전 Target은 `Superseded`로 남기고 새 Target ID를 Current로 지정한다.

---

## 7. 소유권 규칙

### Gameplay C++

```text
- Gameplay 상태와 수치 계산
- 전투·주행·센서 판정
```

### UI Data / Presentation C++

```text
- Provider / ViewData 생성
- Presenter / Presentation state resolver
- Radar/Target/Reticle처럼 runtime 위치 계산이 본질인 spatial projection
```

C++ 사용 여부가 소유권 기준이 아니라 책임이 기준이다. Visual 편의를 이유로 Gameplay 판정을 UMG/BP로 이동하지 않고, 반대로 정적 미적 Layout을 C++로 회수하지 않는다.

### UMG Designer

```text
- 정적 Position / Size / Anchor / Alignment / Padding
- Panel composition
- Text/Image/Material 배치
- 미적 비율과 시각 위계
```

### Texture / UI Material

```text
- 고정 Frame·Icon·Plate·Bracket·Silhouette
- Gauge Track·Mask
- Fill·Glow·Sweep·Damage·Charge 같은 동적 시각 효과
```

### Blueprint

C++ 데이터 계약을 보존하면서 단순 Presentation animation, state transition과 Designer 조립에 BP가 더 자연스러우면 사용한다. Gameplay 판정을 BP Widget으로 이동하지 않는다.

---

## 8. CFUIHUDProdEditorBridge 제한

`CFUIHUDProdEditorBridge`는 이번 Feature에서 다음 용도로만 허용한다.

```text
허용
- 의미 Widget 존재/이름/타입 Validate
- 기존 계약 Migration
- 신규 빈 Asset의 최초 Scaffold
- layout-preserving targeted migration

금지
- 승인된 Visual Target을 C++ WidgetTree 생성으로 대신 그리기
- 기존 Designer Layout을 scaffold 좌표로 다시 덮어쓰기
- Border/ProgressBar/Spacer 조합만으로 최종 Art를 대체하기
- Visual 변경을 위해 기존 Production Widget 전체 rebuild
```

향후 새 Visual은 §6의 **Target Decomposition Pipeline**만 사용한다. `WBP-first 재해석`, `Primitive-first styling`, `AI-generated replacement art` 경로는 Deprecated이며 Historical 문구가 남아 있어도 Current 제작 경로로 복구하지 않는다.

---

## 9. Acceptance Gate

### 9.1 Technical PASS

다음과 같은 기술 사실만 판정한다.

```text
- Build / compile
- Widget binding
- Presenter 데이터 반영
- Material parameter 갱신
- persisted Asset 연결
- runtime spatial projection
- regression
```

### 9.2 Visual Match PASS

Technical PASS와 별도다. 다음을 실제 Visual Target과 비교한다.

```text
- Silhouette
- 주요 비율
- Panel composition
- 정보 Hierarchy
- Frame / line language
- Typography weight
- Color contrast
- Material / glow / depth 인상
- 실제 플레이 배경 위 가독성
- 상태 변화가 디자인 언어 안에서 자연스럽게 보이는지
```

### 9.3 USER PASS

시각 품질은 AI Technical PASS로 대신하지 않는다. 사용자가 실제 Designer/PIE/스크린샷을 보고 승인해야 해당 Slice를 Visual Complete로 닫는다.

---

## 10. 우선순위

### P0

```text
VehiclePanel / SpeedGauge / Armor
WeaponPanel
Pause Menu Visual Rebuild
```

### P1

```text
TargetPanel
RadarPanel
AimReticle
AlertFeed
```

### P2

```text
TargetSelect Marker polish
HUD 전체 consistency / multi-resolution visual review
```

### Deferred

```text
MissionPanel production visual
- 실제 Mission runtime owner 확정 후 진행
```

---

## 11. 완료 조건

`CF-FQ-039`는 다음 조건을 모두 만족해야 Done으로 이동한다.

```text
- 주요 Production HUD Slice마다 Current Production Visual Target 또는 USER가 명시적으로 승인한 대체 Acceptance 기준이 Registry에 존재
- P0/P1 대상의 Production Art Breakdown 완료
- 기존 Runtime·Provider·Presenter 계약 보존
- Designer ownership 회귀 없음
- 주요 Panel/Reticle/Pause가 Target 기반 Production Visual로 전환
- 영향 범위 Build/Automation/Asset evidence PASS
- 최종 1920×1080 기준 USER Visual Match PASS
- 필요한 범위의 16:9 / 21:9 / 32:9 safe layout 검증
- Current System 문서에 최종 Visual ownership 계약 승격
```

`CF-FQ-032` Historical evidence를 이 Plan으로 복사하지 않는다.

---

## 12. 현재 체크포인트

```text
Current Gate: VPR-P0-01 VehiclePanel Production Vertical Slice — FRAME MASTER V5 OUTER-SHELL-ONLY CANDIDATE READY / V4 USER REJECT / AI SOURCE QA PASS / USER VISUAL PASS PENDING / UE IMPORT 0 / CONTENT REPOSITION 0
Production method: Target Decomposition Pipeline LOCKED — Target Lock → Logical Breakdown → WBP Skeleton → Slot Contract → Physical Art Breakdown → Assembly
Deprecated: WBP/Primitive-first 후 Concept 맞춤 / Raw Border·ProgressBar final styling / AI replacement art 직접 Import / Technical Scaffold의 Visual authority 역승격
Source mutation evidence: Armor modular Runtime/WBP foundation + `CFUIArmorArtCmdlet.h/.cpp v1.1.0` PlateOnly assetization path. 기본 2-icon one-shot/no-overwrite 경로는 보존
Content mutation evidence: 기존 `WBP_CFArmorSector.Image_DirectionIcon` additive 구조 보존 + `T_UI_ArmorIcon_Arrow` / `T_UI_ArmorIcon_Chevron2` / `T_UI_ArmorPlate` + `T_UI_VehSil_Sedan` / `T_UI_VehSil_SUV` Production Texture. `DA_CFHUDVisual_Default`는 Armor modular 3 reference + exact VehicleData identity `VehicleSilhouettes` 3 entry를 persisted binding. `WBP_CFArmorBodyMap`/Sector layout mutation 0
Build evidence: Armor modular final `ea1918cf88b0470e91398e9a87840e6b` PASS + 2-icon assetization `8eed02112b73443a9c4df1d03d9a859b` PASS + common Plate assetization `3183bb86d5e7483e9cde64be70c27551` PASS + canonical BuildEditor/`CFVehSilProd` process `f7c50e97f58f4ef4bd0e0a64d51474e6` Exit 0 PASS / 기존 Defense Bar pixel evidence 보존
Automation evidence: `CarFight.UI.UI_P0_06` final 17/17 PASS / Fail 0
Persisted evidence: fresh AssetDump `adset_v1_54b79e58701e4288cd0285c2ced3cda7.e9ce14178c430a4696a0f0ad` — common Plate/Direction 2-Icon binding 보존 + `T_UI_VehSil_Sedan` / `T_UI_VehSil_SUV` 존재 + `VehicleSilhouettes` exact 3 entry 확인
PIE pixel evidence: 기존 `M_VehicleDefensePIE` Defense Pawn의 Shield/Integrity Runtime/Pixel 정상성은 보존 evidence이며 새 modular Production Visual Match authority가 아님
Next exact output: `SourceArt/UI/HUD/P2/Review/VT06_FrameReview_v5.png` USER Visual Review. v4는 내부 임시 box 잔존으로 USER REJECT, v5는 outer-shell-only로 AI Source QA PASS했다. USER 승인 뒤에만 `Image_FrameOverlay` Production Texture/Designer assembly로 이동하며, 내부 local frame 및 RPM/Speed/Gear/Silhouette/Armor 위치 최종화는 그 뒤에 진행한다.

Recovered:
- `VDR-VEH-01` Balanced Combat Direction Reference = USER APPROVED
- `CT-HUD-ROOT-01` 전체 HUD module placement = USER APPROVED
- `CT-VEH-01` VehiclePanel Composition Target = USER APPROVED
- `CT-WPN-01` WeaponPanel Compact Composition = USER APPROVED
- `CT-RADAR-01` Radar Composition / Visual Grammar = USER APPROVED
- `DR-TARGET-01` TargetPanel Current Design Contract = recovered
- `DR-RETICLE-01` Reticle/Target Marker Current Design Contract = recovered
- `DR-ALERT-01` AlertFeed Current Design Contract = recovered
- `DR-PAUSE-01` Pause Visual/Migration Current Design Contract = recovered
- `REF-P1-HUD-01` P1 Review = Technical/Historical Reference only
- `REF-P2-VEH-01` P2 Vehicle Art = Technical PASS / USER Visual PASS 미부여
- `VT-ARMOR-01` ArmorBodyMap Production Target decision = USER APPROVED / original image recovery required
- `CAND-VEH-IMG-01` External Image Vision 직접 관측 완료 / 과거 D1-VEHICLE-PANEL Historical Composition Reference로 분류
- `VDR-HUD-A` Working Visual Direction Reference A = Reference Only

AI Recovery / Editable Structure 판정:
- `CAND-VEH-IMG-01`은 직접 관측 결과 과거 D1-VEHICLE-PANEL이며 현재 Production Target이 아니다.
- 신규 Concept 검토에서 Speed/RPM + Shield/Integrity 방향과 Armor의 탑다운/공통-Sector/editable composition은 USER가 진행 방향으로 확인했다.
- persisted AssetDump 기준 `WBP_CFArmorBodyMap`은 이미 `CanvasPanel_Root + Image_VehicleSilhouette + WBP_CFArmorSector 6개` 구조이며, 각 Sector는 독립 Canvas Slot이라 Designer 재배치가 가능하다.
- `WBP_CFArmorSector`는 현재 `Image_ArmorPlate + Text_Direction + ProgressBar_Armor` persisted 구조이며 새 `WBP_CFArmorSlot`은 만들지 않는다. 다음 migration에서 `Image_DirectionIcon`을 additive 추가해 common Plate + 2-icon Designer rotation 구조로 전환한다.
- `SourceArt/UI/HUD/P2/T_UI_VehSil_LF.png` 직접 관측 결과 현재 VehicleSilhouette도 이미 탑다운이며 좌향 방향 계약을 유지할 수 있다.

Armor Production Visual technical slice:
- 기존 P2 방향별 Armor Texture 6종은 compatibility fallback으로만 보존한다. Current Production은 common Plate + `Arrow/Chevron2` 2종 Source + Designer rotation 구조다.
- `UCFArmorSectorWidget v1.1.1`은 방향 Texture가 있으면 `Text_Direction`을 숨기고 Texture 누락 시에만 fallback Label을 표시한다.
- Armor Ratio는 Presentation에서 `>0.60 Armor`, `>0.30 Caution`, `<=0.30 Critical` 색으로 Plate와 세로 Bar에 함께 적용한다. Gameplay Defense 판정은 변경하지 않는다.
- 현재 0%는 Critical tint + empty Bar까지 구현했다. Spec의 Broken Red X/갈라진 Plate 같은 추가 Art는 아직 USER Visual Target/후속 Art refinement 대상이다.
- first focused run에서 nested Widget lifecycle 때문에 Direction Label 6건이 Visible로 검출됐고, `SetArmorPercent()`가 icon-first visibility를 재보장하도록 교정한 뒤 exact rerun 1/1 PASS했다.
- final Official Build와 Armor/Defense/RPM focused regression은 모두 PASS다.
- 구조/기술 PASS는 전체 VehiclePanel `VT-VEH-01` 승인으로 확대하지 않는다.
- 2026-08-24 USER는 현재 Armor 표시에서 Direction Text가 등록돼 있으나 Texture 존재 시 의도적으로 숨겨지는 icon-first/fallback 구조임을 확인한 뒤 **Armor visual slice PASS**를 명시했다.

Frame + Shield/Integrity Technical 판정:
- 저장 `Border_Surface`는 `DA_CFHUDVisual_Default.VehiclePanelFrame`의 기존 P2 `T_UI_VehPanelFrame`을 **9-Slice Box**로 실제 소비하는 것이 focused Automation으로 확인됐다. Outer Frame용 신규 Content Asset은 만들지 않는다.
- `CFHUDPresenter.cpp v1.25.1`은 기존 `ProgressBar_Shield/Integrity`와 Designer Slot을 유지하면서 StyleData 기반 `SurfaceRaised` dark track + 2px inset semantic fill을 적용하고 Track/Fill/Marquee Brush에 10px intrinsic height를 명시한다.
- Shield는 `Shield` token, Integrity는 `Integrity` token을 Bar·Icon·Value에 일관되게 적용한다. Ratio/Visibility source와 Gameplay Defense 계산은 변경하지 않는다.
- Bar fill은 `LeftToRight + Scale + non-marquee`를 사용하며 기본 UMG ProgressBar primitive 인상을 줄인다.
- 신규 Texture/Widget/Blueprint Content mutation은 0이다.
- initial Official Editor Build `5071957595464725b945c9ad2f704b99` PASS. 실제 PIE에서 Bar 높이 0 pixel regression을 발견한 뒤 `CFHUDPresenter v1.25.1`로 교정했고 pixel-fix Build `d9c33e2a689e42b2a511698fa021df4f` PASS, test-harness 교정 Build `87d13671b2aa466bac88a820d5eda72b` PASS다.
- viewport에 붙지 않은 transient Widget의 `GetDesiredSize()`가 actual PIE와 불일치해 focused test는 실제 회귀 원인인 Background/Fill/Marquee Brush intrinsic height를 직접 검증하도록 `CFHUDDataTests v1.34.2`로 교정했다. 최종 `VehicleDefenseBarVisualContract` 1/1 PASS다.
- fresh `M_VehicleDefensePIE`에서 Baseline Pawn Integrity Bar 복원 후 Defense Pawn 전환까지 확인했다. Snapshot 기준 Shield와 Integrity ProgressBar가 각각 591x10으로 동시에 존재하고, Screenshot 기준 Shield Cyan / Integrity Red Bar가 실제 픽셀로 렌더링됐다.
- `DefenseRuntimeViewData` 1/1 PASS와 `RpmGaugeVisualBindingContract` 1/1 PASS를 보존한다.
- Armor USER PASS는 이번 slice에서 재실행하지 않았다.

Next:
1. 기존 Runtime/Provider/Presenter 의미 계약과 Armor modular/vehicle-specific silhouette/Defense Source Production PASS는 반복하지 않는다.
2. `VT-VEH-01` USER lock과 repository canonical `1280×594 / SHA cf5b...` binding은 반복하지 않는다.
3. 과거 `VT01_FrameReview_v2` USER PASS는 Historical extraction evidence로 보존한다. 2026-08-26 current P2 frame의 상위 hierarchy 불일치가 새 관련 failure이므로 **Current Frame Master만 재개**한다.
4. Frame Master는 외곽 shell → shared inner surface → corner detail → top accent → upper 50:50 seam → lower Defense deck 관계 순으로 먼저 고정한다.
5. Frame Master USER Visual PASS 전에는 RPM 미세조정, Speed/Gear 위치 최종화, Silhouette/Armor 내부 재배치를 수행하지 않는다.
6. Frame Master PASS 뒤 `Image_FrameOverlay` full-aspect Designer assembly를 수행하고, 그 safe-area를 기준으로 RPM/Speed/Gear/Silhouette/Armor를 재배치한다.
```

---

## 13. Changelog

### v0.1.33 - 2026-08-28

- USER가 Outer Frame보다 Speed/RPM과 Armor 구성 유사도가 핵심이라고 재강조해 Prototype 우선순위를 교정했다.
- VT11 cleaned static candidate + existing RPM MaskPack + runtime Text sample + vehicle-specific Sedan silhouette + modular Armor Plate/Icon + existing Defense family만으로 `VT12_VehPanelPrototype.png` 896×416 actual-asset prototype을 생성했다.
- Armor 배치는 Target contract `Front=좌 / Rear=우 / Right=상 / Left=하 / Top=좌상 / Bottom=우하`, vehicle facing left를 적용했고 Armor ratio는 Production contract의 vertical bar로 모사했다.
- `VT12_ReferenceCompare.png`에서 canonical VT-VEH-01과 Prototype을 1:1 scale side-by-side 비교할 수 있게 했다. UE Import/Designer/Production Asset mutation은 0이며 현재 Gate는 USER Speed/RPM + Armor composition review다.

### v0.1.32 - 2026-08-28

- VT10 extraction을 그대로 Production에 쓰지 않고 VT11 Production Cleanup 단계로 전진했다.
- VT10의 geometry/alpha/runtime hole은 그대로 보존하면서 3×3 median luminance, accent cluster 분류, 제한 tactical palette, reference-derived bevel edge lift만 적용해 `VT11_FrameOverlay_Clean`, `VT11_SpeedFrame_Clean`, `VT11_ArmorFrame_Clean`을 생성했다.
- exact alpha verification에서 세 Asset 모두 mismatch 0을 확인했다. RGB color count는 Frame 148→19, Speed 256→20, Armor 151→24로 축소되어 사진성 노이즈/불규칙 명암을 제거했다.
- `VT11_CleanupReview.png`는 VT10 extraction과 VT11 cleaned candidate를 나란히 비교하는 QA board이며 Concept Target이 아니다. USER Cleanup Review 전 UE Import/Designer/Production Asset mutation은 0이다.

### v0.1.31 - 2026-08-28

- USER 확인 뒤 VT09 구조선 자체를 새 디자인으로 승격하지 않고 실제 Production 파츠 제작으로 이동했다.
- canonical VT-VEH-01 픽셀만 사용해 `VT10_FrameOverlay` 896×416, `VT10_SpeedFrame` 422×272, `VT10_ArmorFrame` 422×272 실제 Source 후보를 생성했다.
- Runtime 숫자/텍스트/RPM/Armor Sector/Vehicle silhouette 영역은 transparent cleanup했고, 새 장식/새 frame pixel 생성은 0이다.
- `VT10_StaticAssetsReview.png` isolated QA board와 Asset manifest를 추가했다. UE Import/Designer/Production Asset mutation은 0이며 현재 Gate는 USER Static Asset Review다.

### v0.1.30 - 2026-08-28

- VT07 whole-panel 후보는 USER Visual 기준을 충족하지 못해 successor 제작 대상으로 사용하지 않는다.
- 다음 후보를 임의 창작하지 않고 canonical `VT_VehPanel01.jpg` 실제 픽셀에서 직접 구조선을 추출하는 VT09 Reference Fidelity Recovery로 전환했다.
- `MakeVT09Struct.ps1`에서 exact SHA 확인 → 0.7 scale 896×416 → Sobel + non-maximum suppression + hysteresis 방식으로 `VT09_StructureOverlay_v1.png`와 `VT09_StructureTrace_v1.png`를 생성 PASS했다.
- 임의 geometry/layout box 추가 0, UE Import 0, Production Asset mutation 0이며 현재 Gate는 USER Reference Structure Review다.

### v0.1.29 - 2026-08-27

- URT05에서 검증한 Asset-First 방법을 현재 Active VehiclePanel에 이전했다. Concept-first 역추출 대신 실제 Source → whole-panel composite 순서를 사용한다.
- 기존 RPM/Armor/vehicle-specific silhouette/Defense Production PASS는 재제작하지 않고 그대로 재사용하도록 고정했다.
- `VT07_SpeedFrame.json` / `VT07_ArmorFrame.json` 422×272 독립 Source와 deterministic PNG derivative를 생성했다. corner/accent/rail은 독립 runtime behavior가 없어 각 local frame 한 장에 bake하며 layer proliferation을 금지했다.
- `VT06_FrameMaster_v5` + VT07 Local Frame 2 + current RPM MaskPack + VT04 modular Armor + VT05 Sedan silhouette + VT06 Defense Source만으로 `VT07_VehPanelReview.png` 896×416 Asset-First Review를 생성했다.
- 현재 Gate는 whole-panel USER Visual Review이며 승인 전 UE Import, Designer assembly/reposition, Production Asset mutation은 0이다.

### v0.1.28 - 2026-08-26

- USER가 v4 내부의 상단 safe-area box, central seam, lower deck rectangle을 임시 layout geometry로 판정하고 최종 Frame에 남기지 말 것을 명시했다. v4 USER Visual은 Reject로 기록했다.
- Frame Master ownership을 outer mechanical shell / corner / top rail / lower outer base로 축소했다. 422+12+422, Defense 2-row 등 safe-area는 layout contract로만 유지하고 Production Frame pixel에는 렌더링하지 않는 강제 규칙을 추가했다.
- `BuildFrameMaster.py v1.4.0`으로 `VT06_FrameMaster_v5.png`와 `VT06_FrameReview_v5.png`를 생성했다. v5 직접 Image QA에서 internal temporary geometry 0을 확인해 AI Source QA PASS했다.
- 다음 Gate를 v5 USER Visual Review로 전진했으며 UE Import, WBP/Designer mutation, RPM/Speed/Gear/Silhouette/Armor reposition은 계속 0이다.

### v0.1.27 - 2026-08-26

- canonical `VT-VEH-01` 1280×594를 0.7 scale로 사용하는 deterministic Frame Master builder를 추가하고 v1→v4까지 bounded AI Source QA를 수행했다.
- v1~v3에서 Armor/local-frame source pixel contamination을 단계적으로 검출해 USER 후보에서 제외했다. v4에서는 Frame Master source ownership을 outer shell/corner/top rail/lower base로 좁히고 central seam/main-deck separator를 deterministic cool structural line으로 분리해 dynamic/content contamination을 제거했다.
- `VT06_FrameMaster_v4.png` 896×416 SHA `b5f48e99...516bf`, `VT06_FrameReview_v4.png` 1832×860 SHA `6225b2c7...666b2`, `VT06_FrameManifest_v4.json`을 Pre-USER-PASS candidate evidence로 고정했다.
- AI Source QA만 PASS로 전진했으며 USER Frame Master Visual PASS, UE Import, WBP/Designer mutation, RPM/Speed/Gear/Silhouette/Armor reposition은 모두 0/Pending으로 유지한다.

### v0.1.26 - 2026-08-26

- USER가 RPM 미세조정보다 Vehicle Panel Frame Master 정리를 우선하도록 지시했다. 문제를 단순 child 배치가 아니라 상위 frame design mismatch로 재분류했다.
- repository canonical `VT_VehPanel01.jpg` 1280×594와 current `P2/T_UI_VehPanelFrame.png` 512×256를 Project External Image Vision으로 직접 비교했다. current frame은 단일 대칭 octagonal 9-Slice이며 Master의 layered outer shell, shared inner surface, corner detail, interrupted top accent, upper central seam, lower Defense deck hierarchy를 포함하지 않는 새 관련 Visual failure로 판정했다.
- Current extraction 좌표계를 과거 1536×713 / 7⁄12에서 repository canonical 1280×594 / scale 0.7로 재기준화했다. 과거 1536×713 F1~F4/FrameReview_v2는 Historical local evidence로만 보존한다.
- `F8 Frame Master Reset Contract`를 추가해 Frame hierarchy와 lower Integrity/Defense 관계, 50:50 upper partition, main→Defense 전환선 약 67% 측정 기준을 고정했다.
- Gate를 `Frame Master source/review → USER PASS → Image_FrameOverlay assembly → RPM/Speed/Gear/Silhouette/Armor reposition` 순으로 변경했다. 기존 Runtime/RPM ratio/Armor modular/silhouette/Defense Production PASS는 보존한다.

### v0.1.25 - 2026-08-25

- USER PIE에서 RPM 외형 변화가 없고 SourceArt에도 RPM-S1 Source Pack이 없다는 지적을 받아 current repository/Asset 상태를 재감사했다. `SourceArt/UI/HUD` 전체에서 RPM/VT02/MaskPack 이미지 검색 결과 기존 `P2/T_UI_RPMTrack.png` 1건만 존재했고 `T_UI_RPMMaskPack` / `VT02_*`는 0건이었다. Host-global Trash도 RPM-S1 관련 이미지 0건이었다.
- 따라서 v0.4.10/v0.4.11 및 ActiveWork v3.86/v3.87의 local source/review 기록은 Historical evidence로만 보존하고, Current에서는 RPM-S1을 Source reconstruction + Production Apply Required로 재개했다. 기존 `Image_RPMGauge + M_UI_RPMGauge + RPMRatio` runtime 구조 자체는 PASS로 보존한다.
- USER가 `SourceArt/UI/HUD/VT/VT_VehPanel01.jpg`를 직접 제공했으므로 이를 1280×594 / SHA-256 `cf5b0a57b76f9c41272ee01ed752d263a7b01a78bf02e9cb9dc042d000df6b6b` repository canonical Master로 등록했다. 과거 1536×713 대화 승인본과 동일 바이트라고 주장하지 않는다.
- Defense Source 4종 + `M_UI_DefFill` Production Assetization/VisualData 5-field binding PASS는 보존하고, Defense WBP Assembly는 RPM-S1 Production 적용 뒤 재개한다.

### v0.1.24 - 2026-08-25

- authority ChassisMesh에서 `VT05_VehSil_Sedan.png` / `VT05_VehSil_SUV.png` 512×256 transparent PNG Source를 생성·검증하고 USER 진행 승인 뒤 Production Assetization을 수행했다.
- `T_UI_VehSil_Sedan` / `T_UI_VehSil_SUV`을 생성하고 fresh persisted AssetDump에서 `DA_CFHUDVisual_Default.VehicleSilhouettes` exact 3 entry를 확인했다. Sedan은 Sedan Texture, 두 SUV VehicleData는 동일 SUV Texture를 공유하고 legacy fallback은 보존한다.
- canonical BuildEditor + `CFVehSilProd` process `f7c50e97f58f4ef4bd0e0a64d51474e6` Exit 0과 AssetDump `adset_v1_54b79e58701e4288cd0285c2ced3cda7.e9ce14178c430a4696a0f0ad`를 Technical evidence로 고정했다.
- 다음 Physical Art를 Defense common frame/mask/hex/chevron + `M_UI_DefFill` family로 전진하고 exact `VT-VEH-01` Master Source repository binding은 별도 Pending으로 유지했다.

### v0.1.23 - 2026-08-25

- persisted `CFVehicleData` 3종의 exact `VehicleVisualConfig.ChassisMesh`를 감사해 `DA_TestSedan → Sedan.Sedan`, `DA_TestSUV → SUV.SUV`, `DA_VehicleDefense_TestSUV → SUV.SUV` mapping을 확정했다.
- silhouette Source Authority를 `exact VehicleData identity → exact ChassisMesh identity → mesh-derived silhouette Source`로 고정했다. 현재는 3 VehicleData / 2 unique ChassisMesh이므로 Production shape source는 Sedan/SUV 2종이다.
- P2 generic polygon silhouette와 Chassis identity가 없는 VT03 후보를 특정 차량 Production authority에서 제외했다. 다음 Gate는 Sedan/SUV mesh-derived Source Candidate 2종 + USER Visual Review이며 승인 전 `VehicleSilhouettes` count 0과 Import/Bind 0을 유지한다.
- common Plate + Direction 2-Icon Assetization, Armor migration, RPM-S1, Frame Family는 새 관련 failure 없이 반복하지 않고 exact VT-VEH-01 Master Source binding은 별도 Pending으로 보존한다.

### v0.1.22 - 2026-08-25

- VT04 common Armor Plate를 exact repository Source로 binding하고 `T_UI_ArmorPlate` + `DA_CFHUDVisual_Default.ArmorCommonPlate` Production Assetization을 Technical PASS로 닫았다.
- Official Build `3183bb86d5e7483e9cde64be70c27551`, `CFUIArmorArt -PlateOnly` process `19d669f0a2c8415a99305aa13133c59a`, fresh AssetDump `adset_v1_653b88c9306c201ab9f785c79b7431ce.908692ddcf890cb552fe879e`를 상세 evidence로 기록했다.
- common Plate + Direction 2-Icon 세 필드는 non-null 조건을 충족했지만 `VehicleSilhouettes` count 0은 유지한다. 다음 Gate는 vehicle-specific silhouette Source Authority / VehicleData identity mapping이며 Master Source와 USER Visual은 Pending이다.

### v0.1.21 - 2026-08-25

- Armor canonical `Arrow/Chevron2` 2종 Source를 repository에 binding하고 Production Texture2D 2종으로 Assetize해 `DA_CFHUDVisual_Default`의 두 Direction Icon field에 persisted 연결했다.
- final Official Build `8eed02112b73443a9c4df1d03d9a859b`, `CFUIArmorArt` Exit 0, fresh AssetDump persisted PASS를 상세 evidence로 기록했다.
- `ArmorCommonPlate=None`, `VehicleSilhouettes` count 0, exact VT-VEH-01 Master Source binding Pending과 USER Designer ownership을 보존해 다음 slice를 common Plate + first vehicle-specific silhouette Production Art로 좁혔다.

### v0.1.20 - 2026-08-25

- VehicleData별 silhouette runtime binding과 `WBP_CFArmorSector.Image_DirectionIcon` additive migration을 구현해 Armor modular Runtime/WBP Technical slice를 닫았다. 기존 six-sector Designer Position/Size와 Runtime Armor Ratio binding은 보존했다.
- UE 5.8 Widget GUID 계약을 반영한 clean migration Exit 0 / save0 idempotence, fresh persisted AssetDump, final Official Build `ea1918cf88b0470e91398e9a87840e6b`와 `CarFight.UI.UI_P0_06` 17/17 PASS를 상세 evidence로 고정했다.
- Current를 `Armor Modular Runtime/WBP Technical PASS / 2-Icon Source Pack Ready / Source Binding + Production Art Binding Pending / USER Visual Pending`으로 전진했다. 실제 Art Import/Binding과 USER Visual은 승격하지 않았다.

### v0.1.19 - 2026-08-25

- USER가 `ARMOR-S1` 검토 전에 구조를 재정의했다. 중앙 Vehicle silhouette는 차종/`VehicleData` identity별로 교체하고, Armor 방향 Art는 6개 완성 Texture가 아니라 common Plate + `Arrow/Chevron2` 2종 Source를 각 Sector에서 Designer rotation으로 재사용한다.
- 여섯 `WBP_CFArmorSector`의 Position/Size/Icon 종류/rotation을 UMG Designer 소유로 확정했다. Runtime 방향 의미는 화면 좌표가 아니라 기존 `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` instance name과 Presenter binding이 계속 소유한다.
- `UCFVehicleData` schema를 UI 이유로 확장하지 않고 `UCFHUDVisualData`에 VehicleData별 silhouette catalog를 두며 기존 단일 `VehicleSilhouette`을 fallback으로 보존하는 ownership을 확정했다. 기존 `ArmorPlates.Front~Bottom`도 migration compatibility fallback으로 유지한다.
- `VT04_ArmorIcon_Arrow / VT04_ArmorIcon_Chevron2` 128×128 target-derived local Source와 rotation Review/Manifest를 제작했다. 기존 `VT03` 6-direction pack은 USER PASS 없이 Superseded local experiment로 내렸다.
- Current Gate를 `Armor Modular Structure Locked / 2-Icon Source Pack Ready / Vehicle Silhouette Binding + WBP Migration Pending / Source Binding Deferred`로 전진했다. WBP/C++/UE Asset mutation은 아직 0이다.

### v0.1.18 - 2026-08-25

- USER가 `VT01_FrameReview_v2`의 Frame Family를 명시 PASS했다. Frame F1~F4 local evidence는 새 관련 결함이 없는 한 재검토하지 않고 Source Binding 전까지 Pre-Binding evidence로 보존한다.
- Frame PASS 직후 `ARMOR-S1` local source pack을 제작했다. 첫 Vehicle silhouette extraction의 과도한 threshold를 폐기하고 v2에서 full target wireframe을 복원했으며, common target-derived mechanical plate + 6방향 icon-only family를 만들었다. Glyph는 VT-VEH-01 source pixel의 mirror/rotation만 사용해 Front←← / Right↑ / Rear→→ / Left↓ / Top⌃⌃ / Bottom⌄⌄ 계약을 보존한다.
- Current Gate를 `ARMOR-S1 Local Source Pack Ready / USER Review Pending / Source Binding Deferred`로 전진했다. `VT03_ArmorReview_v2` USER PASS 전에는 repository Production 승격/UE Import를 수행하지 않는다. UE/WBP/Runtime mutation은 0이다.

### v0.1.17 - 2026-08-25

- USER가 `VT02_RpmRuntimePack_Review_FINAL3`를 확인하고 RPM-S1 시각 결과를 명시 PASS했다. AUTO QA PASS와 USER Visual PASS를 분리해 기록하고 RPM-S1 Review는 새 관련 결함이 없는 한 반복하지 않는다.
- Current Gate를 `RPM-S1 USER PASS / Frame USER Review Pending / Source Binding Deferred`로 전진했다. 다음 USER Gate는 `VT01_FrameReview_v2`이며, 통과하면 Armor silhouette + 6 icon-plate source extraction으로 이동한다.
- Production Import 후보는 `T_UI_RPMMaskPack`의 최종 안전 패킹 `R=Progress / G=Tick21 / B=GuideRail / A=255 opaque`, Red는 `RedStartRatio=0.85`에서 파생하는 계약을 유지한다. UE Import/WBP/Runtime mutation은 0이다.

### v0.1.16 - 2026-08-25

- `VT-VEH-01` 작동형 RPM 기준 `RPM-S1 Source Mask Pack`을 local pre-binding 범위에서 제작했다. 첫 두 시도에서 curved path 수식 및 segment 접촉 결함을 자동 QA로 발견해 Production 후보에서 제외하고, vertical→quarter-curve→horizontal tangent를 교정한 최종 Pack만 Current evidence로 채택했다.
- 최종 QA는 21 independent Tick / 21 progress levels / 4 red ticks / Ratio 0.00·0.25·0.50·0.85·1.00 = 0·5·10·17·21 ticks로 PASS했다.
- Production Import 후보를 여러 mask Texture가 아니라 `T_UI_RPMMaskPack` 724×467 RGBA 1장으로 channel-pack했다. PNG zero-alpha RGB 변형 위험을 제거하기 위해 최종 배치는 R=Progress, G=Tick21, B=GuideRail, A=255 constant opaque로 교정했으며 SHA-256은 `6cc32459a972a79c6aa50983050aa5fdb8656a4efa5a7caa9475fda66def8777`이다. Red Zone은 별도 channel 없이 `RedStartRatio=0.85`와 R/G에서 Material이 파생한다.
- 기존 `Image_RPMGauge + M_UI_RPMGauge + RPMRatio` single Material 구조를 유지하고 TrackOff는 G channel에서 계산한다. `RPM`은 TextBlock, `x1000`과 절대 숫자 눈금은 Production에서 제외한다.
- Current Gate를 `RPM-S1 Local Source Pack Ready / Frame+RPM USER Review Pending / Source Binding Deferred`로 전진했다. UE Import/WBP/Runtime mutation은 0이다.

### v0.1.15 - 2026-08-25

- USER가 현재 PC에서 Source Binding을 수행할 수 없어 repository copy를 Deferred로 유지하고, 가능한 AI 작업을 선행했다.
- VT-VEH-01 exact SHA 원본으로 Frame F1/F2/F4를 보존하고 F3 ArmorFrame을 v2로 추가 cleanup했다. F3는 source RGB를 유지한 채 고신뢰 상단·우측·하단 frame alpha만 보존해 Armor card/vehicle/ring contamination을 제거했고 새 시각 요소는 생성하지 않았다.
- 896×416 `FrameMock_v2`와 1840×1420 `FrameReview_v2` 및 파일별 SHA manifest를 생성해 USER Frame Review가 가능한 Local Pre-Binding Evidence를 완성했다.
- Current Gate를 `VT-VEH-01 Locked / Frame F1~F4 Local Extraction Ready / Source Binding Deferred / USER Frame Review Pending`으로 전진했다. UE Import/WBP/Runtime mutation은 0이다.

### v0.1.14 - 2026-08-24

- USER가 1536×713 Master 이미지를 `VT-VEH-01 Production Visual Target`으로 명시 승인해 Registry를 Candidate에서 USER APPROVED / LOCKED로 승격했다. `CAND-VEH-MASTER-01`은 Historical alias로만 남긴다.
- 저장소 Source binding 전에는 대화 첨부 식별값 1536×713 / SHA-256 `d795f09531c41edbeb460a7391a337d77a7e7c582b5c3e661f1032b5d173ccae`를 Target Source evidence로 유지하고, 권장 binding 경로를 `SourceArt/UI/HUD/VT/VT_VehPanel01.png`로 예약했다.
- Frame family source extraction 계획 F0~F6을 추가했다. Master native resolution에서 `VehFrameOverlay / SpeedFrame / ArmorFrame / GearFrame`을 alpha extraction하고 896×416 mock composite Review를 USER가 승인한 뒤에만 Unreal Import로 이동한다.
- Current Gate를 `VT-VEH-01 Locked / Frame Family Extraction Plan Ready / Source Binding Pending`으로 전진했다. 실제 PNG extraction·UE Asset mutation은 아직 0이다.

### v0.1.13 - 2026-08-24

- USER 결정으로 Armor 방향 표시는 Production에서 Text를 제거하고 기존 6방향 icon-only 계약을 사용하도록 확정했다. `Text_Direction`은 missing-texture fallback/debug 전용으로 유지한다.
- RPM은 작은 `RPM` Label만 유지하고 `x1000`과 절대 숫자 눈금을 제거한다. 기존 21 Tick / 85% Red Zone / `RPMRatio` single Material 계약은 보존한다.
- Shield/Integrity Chevron은 Master의 형태 언어만 채택하며 고정 장식으로 쓰지 않는다. 기존 `▶/▶▶/▶▶▶` recovery/repair 의미와 Fill-end 이동 계약을 보존한다.
- 1536×713 → 896×416 uniform scale, Root underlay + full-aspect overlay, single compact ArmorSector family, Defense Frame/Mask/Hex/Material 경계, atomic DefenseBar binding migration을 확정해 Slot Contract Review를 닫았다.
- VehiclePanel Physical Art Breakdown Contract를 작성했다. 기존 asset identity는 가능한 한 재사용하고 신규 파츠를 `T_UI_VehFrameOverlay`, `T_UI_SpeedFrame`, `T_UI_ArmorFrame`, `T_UI_GearFrame`, `T_UI_DefBarFrame`, `T_UI_DefFillMask`, `T_UI_DefHex`, `T_UI_DefChevron`, `M_UI_DefFill`로 한정했다.
- 실제 PNG/Material 생성은 아직 수행하지 않았다. Current Gate를 `Physical Art Breakdown Contract Ready / VT-VEH-01 Lock + Source Binding Pending`으로 전진했다.

### v0.1.12 - 2026-08-24

- USER가 `CAND-VEH-MASTER-01` 기준 상단 Speed/RPM : Armor 비율을 **50:50**으로 명시 승인했다. 기존 `CT-VEH-01` 35~40:55~60 폭 계약을 Current에서 교체했다.
- `InGameUIVehiclePanelSpec.md v0.20.4`의 896×416 좌표 계약을 반영해 `SpeedGauge 422×272 / Gap 12 / ArmorBodyMap 422×272`를 Current Composition으로 고정했다.
- `CAND-VEH-MASTER-01`은 `50:50 COMPOSITION APPROVED`로 전진했지만 전체 `VT-VEH-01 Production Visual Target` 승격은 여전히 Pending이다.
- Slot Contract Review의 폭 Deviation 항목을 RESOLVED로 닫고 남은 검토를 scale mapping / Frame 분해 / Armor variant·label / Defense Material 경계로 좁혔다.
- Current Gate를 `50:50 Top Width Locked / Remaining Slot Contract Review Pending`으로 전진했다.

### v0.1.11 - 2026-08-24

- USER가 현재 대화에 제공한 1536×713 VehiclePanel 이미지를 `CAND-VEH-MASTER-01`로 등록했다. SHA-256 `d795f09531c41edbeb460a7391a337d77a7e7c582b5c3e661f1032b5d173ccae`; Logical Breakdown 기준으로 사용하되 저장소 Source binding과 `VT-VEH-01` 승격은 Pending이다.
- fresh AssetDump로 VehiclePanel/SpeedGauge/ArmorBodyMap/ArmorSector persisted 구조를 대조하고 Target-derived WBP Skeleton Contract를 작성했다. 기존 `WBP_CFArmorSector` 6개와 `Image_RPMGauge` runtime sink는 보존하고 중복 `WBP_CFArmorSlot` 생성은 금지한다.
- Shield/Integrity는 신규 재사용 `WBP_CFDefenseBar` 경계를 기본안으로 잡았으며 현재 Presenter의 parent-only `GetWidgetFromName` 때문에 실제 nested 도입은 Runtime Binding migration과 atomic하게 묶도록 명시했다.
- Master Candidate의 Speed/Armor 상단 비율이 약 50:50으로 보여 기존 `CT-VEH-01` 35~40:55~60과 차이가 있음을 Deviation 후보로 기록했다. exact width는 Slot Contract Review와 USER 판정 전 동결하지 않는다.
- Armor Master Candidate의 icon+label 동시 표현과 현재 `UCFArmorSectorWidget v1.1.1` icon-first Label fallback 계약의 충돌을 발견해 후속 Binding 교정 항목으로 고정했다. 지금 Source/Asset mutation은 수행하지 않았다.
- Current Gate를 `WBP Skeleton Contract Ready / Slot Contract Review Pending`으로 전진했다.

### v0.1.10 - 2026-08-24

- USER 결정으로 CF-FQ-039의 기존 `WBP/Primitive-first → 나중에 Art 보정` 제작 방식을 명시적으로 Deprecated 처리하고 `Target Lock → Logical Breakdown → WBP Skeleton → Physical Art Breakdown → Assembly`를 유일한 Production UI 제작 파이프라인으로 고정했다.
- 실제 이미지 파일 분해와 논리적 분해를 분리했다. 논리적 분해가 먼저이며, 그 결과로 WBP Slot 계약을 잡은 뒤 동일 Master Target에서 실제 Texture/Mask/Frame/Icon 파츠를 제작한다.
- AI가 승인 Target 대신 새 미감을 생성해 바로 Production Asset으로 넣는 경로를 금지했다. AI는 Target-preserving 추출/정리 보조만 기본 허용하며 새 생성 자산은 USER 별도 승인 전 Production Import 금지다.
- 현재 Frame/Shield/Integrity Technical/Pixel PASS는 Runtime/Scaffold evidence로만 보존하고 Visual Match authority에서 내렸다. Current Gate를 `Production Method Reset Locked / Target Logical Breakdown Pending`으로 전환했다.

### v0.1.9 - 2026-08-24

- `CFHUDPresenter v1.25.1` final pixel fix와 `CFHUDDataTests v1.34.2` test-harness 교정을 빌드하고 `VehicleDefenseBarVisualContract` final 1/1 PASS를 확인했다.
- transient Widget `GetDesiredSize()` assertion은 실제 PIE와 불일치해 제거하고 회귀 원인인 Background/Fill/Marquee Brush intrinsic height > 0을 직접 검증하도록 교정했다.
- fresh `M_VehicleDefensePIE`에서 Defense Pawn으로 전환해 SHIELD 100/100과 INTEGRITY 100/100이 각각 591x10 Bar로 동시에 존재함을 Snapshot으로 확인했고, Screenshot에서 Shield Cyan / Integrity Red 실제 픽셀 렌더를 직접 확인했다. Frame/Speed-RPM/Armor도 보존됐다.
- 따라서 Defense Bar pixel regression을 Technical/Pixel PASS로 닫고 Current Gate를 USER Visual Review Pending으로 복구했다. Armor USER PASS와 전체 `VT-VEH-01` Pending 경계는 유지한다.

### v0.1.8 - 2026-08-24

- fresh PIE screenshot에서 Frame/Speed/RPM/Armor는 보존됐지만 Integrity ProgressBar 본체가 실제 픽셀에서 보이지 않는 회귀를 AI가 직접 발견해 기존 Frame + Shield/Integrity Technical PASS를 재검증 Pending으로 되돌렸다.
- RCA 결과 custom `FSlateColorBrush`의 intrinsic desired-size가 0으로 수축하는 경로를 확인했고 `CFHUDPresenter v1.25.1`에서 Track/Fill/Marquee Brush에 10px intrinsic height를 명시했다.
- 첫 height 교정본의 강화 focused test는 Shield/Integrity `DesiredSize.Y == 0`으로 FAIL했으며, Marquee Brush size까지 보강한 최종 교정본 Official Build `d9c33e2a689e42b2a511698fa021df4f`는 PASS했다.
- 최종 교정본의 exact focused rerun과 fresh PIE pixel revalidation 전에는 Technical PASS 또는 USER Visual Gate로 다시 승격하지 않는다. Armor USER PASS는 그대로 보존한다.

### v0.1.7 - 2026-08-24

- VPR-P0-01 Frame + Shield/Integrity production visual technical slice를 구현했다.
- 기존 P2 VehiclePanel 9-Slice Frame이 저장 `Border_Surface`에 실제 연결된 것을 자동 검증해 Frame Content를 중복 제작하지 않았다.
- `CFHUDPresenter.cpp v1.25.0`에서 기존 Shield/Integrity ProgressBar와 Designer Layout을 유지한 채 dark track + 2px inset semantic fill, semantic icon/value color를 적용했다.
- Official Build `5071957595464725b945c9ad2f704b99`, `VehicleDefenseBarVisualContract` 1/1, `DefenseRuntimeViewData` 1/1, `RpmGaugeVisualBindingContract` 1/1 PASS를 확인했다.
- Armor USER PASS를 반복하지 않았고 Content mutation 0을 유지했다. Current Gate는 Frame + Shield/Integrity USER Visual Review Pending으로 전진했다.

### v0.1.6 - 2026-08-24

- USER가 Armor 6-direction icon family와 Texture 존재 시 Direction Text를 숨기는 icon-first/fallback 동작이 의도에 맞음을 확인하고 Armor visual slice를 PASS했다.
- Armor technical evidence는 그대로 보존하며 USER PASS를 전체 VehiclePanel `VT-VEH-01` 승인으로 확대하지 않았다.
- VPR-P0-01 next visual을 VehiclePanel Frame + Shield/Integrity production visual match로 전진했다.

### v0.1.5 - 2026-08-24

- VPR-P0-01 Armor Production Visual technical slice를 실제 구현했다. 기존 P2 방향 Texture 6종과 Designer 배치를 재사용하고 새 Content Asset은 만들지 않았다.
- `CFArmorSectorWidget v1.1.1`에서 Texture 우선/Label fallback과 Armor Ratio 기반 Stable/Caution/Critical tint를 Plate + Bar에 적용했다.
- first focused failure의 nested lifecycle 원인을 교정한 뒤 `ArmorSectorProductionVisualContract` exact rerun 1/1 PASS, `DefenseRuntimeViewData` 1/1 PASS, `RpmGaugeVisualBindingContract` 1/1 PASS를 확인했다.
- final Official Editor Build `c8f67e0d88f647db8c4de7bba485be91` PASS. Content mutation 0, 기존 Speed/RPM과 Defense Runtime 계약 비회귀를 확인했다.
- Current Gate를 `VPR-P0-01 ARMOR TECHNICAL PASS / USER VISUAL REVIEW PENDING`으로 전진하되 전체 `VT-VEH-01`은 USER 명시 승인 전이라 미승격 상태를 유지했다.

### v0.1.4 - 2026-08-24

- 신규 VehiclePanel Concept에 대한 USER 피드백을 Speed/RPM·Shield/Integrity 방향 승인과 Armor editable composition 승인으로 좁게 기록하고 `VT-VEH-01` 자동 승격은 하지 않았다.
- External Image Vision으로 저장소 루트 과거 이미지를 직접 확인해 `CAND-VEH-IMG-01`을 D1-VEHICLE-PANEL Historical Composition Reference로 재분류했다.
- persisted AssetDump로 `WBP_CFArmorBodyMap = Canvas + VehicleSilhouette + WBP_CFArmorSector 6개`와 `WBP_CFArmorSector = Plate + Direction Text + Armor Progress` 구조를 재확인했다.
- 사용자 요청의 `WBP_CFArmorSlot` 신규 생성은 기존 canonical `WBP_CFArmorSector`와 중복이므로 생성하지 않고 기존 재사용 Widget을 common Armor slot으로 유지한다.
- `T_UI_VehSil_LF.png`가 이미 탑다운 실루엣임을 직접 확인했으며, Designer-owned 독립 Canvas Slot을 통한 자유 재배치를 Structure Readiness로 고정했다.
- Production Target 전 구조 확인은 허용하되 새 Production Art/Texture/Material 변경은 `VT-VEH-01` 승인 뒤 수행하도록 Gate를 보존했다.

### v0.1.3 - 2026-08-23

- VPR-P0-00의 접근 가능한 저장소/Plan/ProjectSSOT/Current Systems 및 이전 대화 context Recovery를 완료했다.
- Radar의 다수 2026-08 USER 결정 기록을 `CT-RADAR-01 USER APPROVED Composition Target`으로 회수했다.
- TargetPanel, Reticle/Target Marker, AlertFeed, PauseMenu의 현재 Design/Composition 계약을 `DR-*` Reference로 회수하되 완성 Production 이미지 승인을 추정하지 않았다.
- 전체 VehiclePanel 완성 이미지의 명시 Production Target은 추가 회수되지 않았으므로 AI Recovery를 종료하고 `USER VEHICLE TARGET PENDING`으로 Gate를 좁혔다.
- 앞으로 동일 텍스트/Source 검색을 반복하지 않고, 과거 원본 재제공 또는 §5.5 Brief 기반 신규 Candidate 중 하나로 `VT-VEH-01`을 확정한다.

### v0.1.2 - 2026-08-23

- VPR-P0-00을 실제 시작하고 Vehicle scope의 초기 Target Registry를 생성했다.
- Balanced Combat를 `VDR-VEH-01 Direction Reference`, VehiclePanel semantic/layout 계약을 `CT-VEH-01 Composition Target`으로 분리했다.
- P1/P2 Review는 기술 검토 및 Technical PASS evidence일 뿐 USER Production Target이 아님을 확인해 `REF-P1-HUD-01 / REF-P2-VEH-01`로 분류했다.
- 과거 USER 수정 Armor 이미지의 명시적 구현 결정을 `VT-ARMOR-01`로 복원하되 원본 바이너리 미회수 상태를 blocker로 기록하고 기억 기반 재생성을 금지했다.
- 저장소 루트의 실제 PNG `ChatGPT Image 2026년 8월 7일 오전 09_45_34.png`는 시각 내용과 승인 범위를 매칭하지 못해 `CAND-VEH-IMG-01` Candidate로만 유지했다.
- 과거 Working Visual Direction Reference A는 Reference Only를 유지하며 아직 전체 VehiclePanel Production Visual Target으로 승격하지 않았다.

### v0.1.1 - 2026-08-23

- `좋다/이 방향이 좋다`를 Production Target 승인으로 자동 해석하지 않도록 승인 단계를 Direction Reference / Composition Target / Production Visual Target으로 분리했다.
- Registry에 Approval Level을 추가하고 승인 범위가 불명확하면 좁게 해석하도록 교정했다.
- C++ 소유권을 Gameplay C++과 UI Data/Presentation C++로 분리해 책임 기준을 명확히 했다.
- Target Recovery 순서를 실제 저장소·SourceArt·접근 가능한 대화/파일 우선으로 명시하고 원본 미확인 기억 기반 Target 재생성을 금지하는 Roadmap과 정합시켰다.

### v0.1.0 - 2026-08-23

- `CF-FQ-032` 완료 기반을 보존한 별도 후속 `CF-FQ-039 Production UI Visual Rework` Plan을 생성했다.
- 2026-08-23 Source/persisted Widget 전수 판정을 `KEEP / ART / LAYOUT / REBUILD / LIMIT / DEFER` 기준으로 고정했다.
- 사용자 승인 이미지를 단순 Reference가 아닌 Visual Target으로 등록하고, 임의 단순화를 Deviation Gate 없이 금지하는 계약을 추가했다.
- Functional/Technical PASS와 Visual Match/USER PASS를 분리했다.
- `CFUIHUDProdEditorBridge`를 Validate/Migration/empty Scaffold 역할로 제한하고 새 Visual 제작의 기본 경로에서 제외했다.

---

## 14. Migration

- `CF-FQ-039` 착수는 `CF-FQ-032 Done`을 취소하지 않는다.
- 기존 UI Runtime/Provider/Presenter/Designer ownership evidence는 관련 변경이 없는 한 재검증하지 않는다.
- 기존 `InGameUIHUDArtSpec`, `InGameUIVisualConcept`, `InGameUIStyleSpec`은 폐기하지 않고 세부 제작 규격 owner로 재사용한다.
- 과거 대화에서 승인된 이미지가 저장소 파일로 확인되지 않을 경우 가짜 경로를 기록하지 않고 `VPR-P0-00`에서 먼저 회수·식별한다.
