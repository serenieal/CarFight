# CarFight InGame HUD Art Asset Spec

- 문서 버전: v0.4.18
- 작성일: 2026-08-11
- 최근 갱신일: 2026-08-25
- 문서 상태: Current Supporting Art Spec / CF-FQ-039 Production UI Visual Rework
- 기능 ID: `CF-FQ-039` / 완료 기반 `CF-FQ-032`
- 상위 Plan: `InGameUIVisualPlan.md v0.1.24`
- 실행 Roadmap: `InGameUIVisualRoadmap.md v0.1.24`
- Production Structure Reference: `InGameUIAssetizationSpec.md v0.16.2`
- Style SSOT: `InGameUIStyleSpec.md v0.86.1`
- Vehicle 의미/배치 SSOT: `InGameUIVehiclePanelSpec.md v0.20.6`
- 현재 대상 DataAsset: `/Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default`

---

## 1. 목적

이 문서는 D1-11 Production HUD에서 사용하는 **전용 시각 아트 자산의 제작·Import·연결 기준**을 소유한다.

이 문서가 해결하는 문제는 다음과 같다.

```text
- Border/Canvas 조각을 그림처럼 조립하는 방식으로 회귀하지 않는다.
- Text, Icon, Vehicle Silhouette, Armor Plate, Gauge, Radar Frame의 책임을 분리한다.
- 이미지 파일명·크기·포맷·색 처리·투명 배경 규칙을 통일한다.
- DA_CFHUDVisual_Default의 각 필드가 어떤 자산을 받는지 고정한다.
- 전용 아트 교체 시 Widget 구조를 다시 설계하지 않는다.
- 현재 Production Bridge가 실제로 소비하는 슬롯과 아직 미연결인 슬롯을 구분한다.
```

이 문서는 HUD Layout, Gameplay ViewData, Widget 수명이나 Runtime 판정을 소유하지 않는다.

---

## 2. 문서 책임 경계

| 문서 | 소유 책임 |
| --- | --- |
| `InGameUIVisualPlan.md` | CF-FQ-039 Visual Rework 상태·Target Registry·완료 조건·다음 단계 |
| `InGameUIAssetizationSpec.md` | Production UMG 구조·Widget 경계·Assetization 실행 계약 |
| `InGameUIStyleSpec.md` | Color·Font·Typography·Spacing·Semantic Icon Token |
| `InGameUIVehiclePanelSpec.md` | VehiclePanel의 정보 의미·방향·배치 계약 |
| **이 문서** | HUD 전용 Texture/UI Material의 제작 규격·파일명·DA 연결·아트 검증 |

핵심 원칙:

```text
Widget Structure ≠ Art Asset
Gameplay Data ≠ Art Asset
Style Token ≠ 개별 HUD 전용 그림
```

---

## 3. Production 시각 구성 원칙

### 3.1 올바른 표현 수단

| 표현 대상 | 기본 수단 |
| --- | --- |
| 글자·숫자·단위 | `TextBlock` |
| Semantic Icon | `Image` + `DA_CFUIStyle_Default.IconSet` |
| 차량 실루엣 | `Image` + VehicleData identity별 `Texture2D` catalog / 단일 fallback |
| 6방향 Armor | 공통 Plate `Image` + 별도 Direction Icon `Image` + 2종 Texture 회전 재사용 |
| 정적 Speed Arc Frame/Track | `Image` + `Texture2D` |
| 동적 원호·Sweep·마스크 효과 | `Image` + User Interface Domain Material |
| Shield·Integrity·Heat·Cooldown·Scan 같은 선형 비율 | Runtime Ratio sink는 `ProgressBar`를 사용할 수 있으나 최종 외형은 승인 Target에서 분해한 Frame/Mask/UI Material이 소유. Raw ProgressBar 외형은 Production Visual 금지 |
| 패널 배경·실제 Container | `Border` 또는 9-Slice 가능한 `Image` |
| Radar Contact처럼 좌표 자체가 Runtime 의미인 배치 | 제한된 `CanvasPanel` |
| Armor Sector 조립·재배치 | `CanvasPanel` + Designer ownership. 화면 좌표는 Runtime ArmorDirection 의미가 아님 |

### 3.2 Designer Layout Ownership

UMG Designer Asset이 실제 레이아웃의 SSOT다. C++/Editor Bridge/Validator는 Widget의 존재·이름·타입·데이터 의미를 보호할 수 있지만 이미 존재하는 Production Widget의 위치·크기·Anchor·Alignment·Padding·AutoSize를 장기적으로 하드코딩하거나 재적용 때 덮어쓰지 않는다.

```text
C++ / Presenter / Validator
= Widget role·name·class·runtime value contract

Texture / UI Material
= 실제 그림·게이지 시각 표현

UMG Designer Asset
= Position·Size·Anchor·Alignment·Padding·AutoSize·visual composition
```

초기 Asset이 없는 경우의 Scaffold 기본 위치·크기는 생성 편의를 위한 최초값일 뿐 Current Layout SSOT가 아니다. `CFUIHUDProdEditorBridge.h v1.12.1 / .cpp v1.15.1`은 기존 RootWidget이 있는 Production Widget/Root의 full Build를 fail-closed로 유지하면서, CF-FQ-039 ArmorSector에는 기존 Designer Tree를 보존하는 targeted additive `Image_DirectionIcon` migration을 제공한다. 이 migration은 UE 5.8 WidgetVariable GUID를 구조 compile 전에 등록하며 `ApplyUIHUDProduction.py v1.5.0`의 기존 Widget Validate-only 원칙을 깨지 않는다. 따라서 저장된 Designer Layout에는 scaffold Position/Size/Anchor/Alignment/Padding/AutoSize를 재적용하지 않는다.

`WBP_CFArmorSector`처럼 재사용 Element 내부도 동일하다. Image/Label/ProgressBar라는 의미 구조는 보호하되 `68×58`, `58×58`, `8×58` 같은 픽셀 크기를 C++ 계약으로 고정하지 않는다. `WBP_CFArmorBodyMap`과 `WBP_CFSpeedGauge`의 자식 배치도 Designer가 조정할 수 있어야 한다.

CF-FQ-039에서 `WBP_CFArmorSector`는 별도 `WBP_CFArmorSlot`을 추가하지 않는 **canonical common Armor slot**으로 유지한다. `WBP_CFArmorBodyMap`의 여섯 Sector 인스턴스와 `Image_VehicleSilhouette`은 각각 독립 Canvas Slot을 사용하며, 방향 의미/Widget 이름만 보호하고 실제 Position·Size·간격은 USER가 Designer에서 재배치할 수 있다.

### 3.3 금지

다음 방식은 CF-FQ-039 Production HUD에서 금지한다.

```text
- 작은 Border 여러 개를 배치해 차량·바퀴·원호·아이콘·장식선을 그린다.
- TextBlock 대신 문자 기호(◆ ● ▲ ■ 등)를 실제 아이콘으로 사용한다.
- 숫자·상태값·미션 문구를 Texture에 구워 넣는다.
- 개별 Widget에 색·폰트·Spacing 값을 새로 하드코딩해 Style SSOT를 우회한다.
- 하나의 큰 이미지에 Panel 전체 UI와 runtime Text/수치까지 합쳐서 넣는다.
- Gameplay 상태를 Texture 또는 Material 자체에서 판정한다.
- WBP/Border/ProgressBar 구조를 먼저 완성한 뒤 그 구조에 맞춰 승인 Concept을 재해석한다.
- Raw Border/ProgressBar의 색·두께·Glow 조정 결과를 최종 Production Art로 승인한다.
- 승인 Master Target 대신 AI가 새 패널·프레임·아이콘을 임의 생성해 바로 Production Asset으로 사용한다.
- 기존 P1/P2 Technical Reference 또는 현재 Scaffold 화면을 Master Target처럼 역승격한다.
```

### 3.4 Target Decomposition Production Method — LOCKED

CF-FQ-039의 유일한 기본 제작 순서는 다음과 같다.

```text
Master Target Lock
→ Logical Breakdown
→ WBP Composition Skeleton
→ Slot Contract Review
→ Physical Art Breakdown / Production
→ UMG Assembly
→ Runtime Binding
→ Technical / Pixel Validation
→ Visual Match
→ USER PASS
```

`Logical Breakdown`은 실제 PNG 파일을 자르는 작업이 아니다. 먼저 Master Target 안에서 Frame, Module, Icon, Typography, Decoration, Static Art, Dynamic Visual, Runtime Data를 논리적으로 분리하고 재사용 Widget 경계와 표현 수단을 정한다.

그 결과를 토대로 WBP에는 **Placeholder Slot과 구조만** 만든다. WBP의 Placeholder는 위치·비율·Hierarchy 검증용이며 그 자체의 기본 UMG 외형은 최종 미감이 아니다. Slot Bounds/Aspect/9-Slice/Mask/Dynamic Fill 계약을 확인한 뒤에만 동일 Master Target에서 실제 Texture/Mask/Frame/Icon 파츠를 분해·정리한다.

AI 이미지 생성은 승인 Target을 대체하는 새로운 Art Direction을 만드는 용도로 사용하지 않는다. 기본 허용 범위는 Target-preserving crop/extraction/cleanup/mask 보조다. AI가 새 픽셀을 생성한 결과를 Production Asset으로 넣으려면 USER가 해당 결과를 별도 승인해야 한다.

기존 방식으로 만들어진 Widget/Texture/ProgressBar는 필요 시 Runtime sink·기능 검증·rollback용 Scaffold로 보존할 수 있으나, **새 Art의 모양을 결정하는 Visual authority로 사용하지 않는다.** Historical 문서에 구형 생성/적용 절차가 남아 있어도 이 Locked Method를 덮어쓰지 않는다.

---

## 4. 시각 방향

### 4.1 기본 키워드

```text
Near-future tactical
Clean sci-fi
Vehicle combat HUD
기능 중심
고대비
낮은 시각 소음
교체 가능한 그래픽 부품
```

### 4.2 기본 색 역할

정확한 색값은 `InGameUIStyleSpec.md`와 `DA_CFUIStyle_Default`가 소유한다.
Texture는 가능한 한 **흰색/회색 기반 Tintable Art**로 제작하고 UMG Tint 또는 UI Material Parameter로 의미 색을 입힌다.

```text
Cyan / Blue  = 일반 시스템·선택·속도·Radar
Amber        = 주의·Armor 경고·Heat
Red / Orange = 적대·위험·파괴
White / Gray = 정보·중립 Frame·실루엣
```

### 4.3 2026-08-19 승인 Visual Direction — Balanced Combat

사용자 승인 기본 방향:

```text
Balanced Combat
- Dark navy / near-black layered panel base
- Cyan = 기본 시스템·속도·일반 정보 Accent
- Amber = Armor·주의·중요 상태 Accent
- Red = Critical·파괴 한정
- 금속성/기계식 프레임의 깊이감은 사용하되 과도한 장식 금지
- Glow는 얕고 국소적으로만 사용
- 전체 인상은 전투 차량 HUD + 정밀 디지털 계기판의 균형
```

이 승인은 **분위기·재질·색·프레임 언어**에 대한 승인이다. 직전 Visual Reference에 포함된 원형 Speed/RPM 계기판, 임의 Gear 배치, 임의 Armor 배치는 채택하지 않는다. 실제 VehiclePanel Layout은 `InGameUIVehiclePanelSpec v0.20.4`가 우선하며, 2026-08-24 USER 결정으로 상단 Speed/RPM : Armor는 50:50이다.

실제 Concept 고정 구조:

```text
VehiclePanel 896×416
- SpeedGauge 20,16 / 422×272
  - 3자리 Speed 000~999
  - km/h = Speed 오른쪽의 작은 단위
  - 단일 Gear Slot
  - 좌측 세로 → 부드러운 곡선 → 상단 긴 수평의 비대칭 RPM Track
  - 21 Tick / Red Zone 85%
- ArmorBodyMap 454,16 / 422×272
  - VehicleData identity별 탑다운 좌향 차량 실루엣
  - WBP_CFArmorSector 6개 재사용
  - 각 Sector Position/Size/Icon 종류/Icon rotation은 Designer 소유
  - Runtime Front/Right/Rear/Left/Top/Bottom 의미는 instance name/Presenter binding 소유
  - 6방향 모두 별도 Armor Ratio Bar 유지
- ShieldRow 20,296 / 856×44
- IntegrityRow 20,348 / 856×52
```

기존 P1 9종의 지위:

```text
Technical PASS reference baseline
≠ USER Production Art Approval
≠ 그대로 Import해야 하는 최종 자산
```

현재 P2 Production Art는 위 실제 구조를 유지한 채 Technical Apply까지 완료됐다. 다음 Gate는 실제 Designer 또는 1920×1080 Production HUD에서 `Balanced Combat` 시각 품질·가독성·배치를 사용자 판정하는 USER Visual Review다.

### 4.4 현재 생성된 HUD 콘셉트 이미지의 지위

2026-08-11 현재 대화에서 생성한 전체 HUD 이미지는 **Working Visual Direction Reference A**로만 사용한다.

현재 저장소 자산이 아니며, USER PASS나 최종 Style SSOT로 승격하지 않는다.

이 Reference에서 채택 후보로 보는 요소:

```text
- Dark navy/near-black panel base
- Cyan primary system line
- Amber caution accent
- Red hostile accent
- 큰 숫자와 작은 보조 레이블의 강한 정보 계층
- 패널별 독립 Frame
- Vehicle / Radar / Weapon을 서로 다른 시각 중심으로 구분
```

직접 복제하지 않는 요소:

```text
- 이미지 안에 구워진 한글·영문·숫자
- 실제 Gameplay 데이터처럼 보이는 임의 수치
- 과도한 장식선·Glow
- Widget 구조와 맞지 않는 한 장짜리 완성 HUD 이미지
```

저장소에 Reference 이미지를 편입할 경우 권장 파일명은 `CFHUD_VisualDir_A.png`이며 실제 파일이 추가되기 전에는 문서 경로를 존재한다고 주장하지 않는다.

### 4.5 CF-FQ-039 Visual Target 승격 규칙

`CF-FQ-039`부터 승인 범위를 `Visual Direction Reference / Composition Target / Production Visual Target`으로 분리한다.

```text
Visual Direction Reference
= 분위기·색·재질·형태 언어 참고 또는 승인.
  Composition 일치 의무 없음.

Composition Target
= Panel 구조·주요 비율·정보 Hierarchy·대표 형태 승인.

Production Visual Target
= USER가 실제 구현 기준으로 명시 승인한 이미지.
  Target Registry의 Must Match 항목은 Deviation 승인 없이 임의 단순화 금지.

Production Art
= Production Visual Target을 실제 Texture / Material / Icon / Widget / Animation으로 분해한 게임 자산.
```

`좋다`, `분위기가 좋다`, `이 방향이 좋다`는 Production Visual Target 승인으로 자동 해석하지 않는다. Production Visual Target은 `InGameUIVisualPlan.md`의 Target Registry에 ID·Scope·Source·Approval Level·Must Match·Flexible·Production Breakdown을 등록한 뒤 사용한다.

기술적 제약으로 Production Visual Target과 달라져야 하면 먼저 Deviation을 기록하고 USER 승인을 받는다. 기능·Build PASS는 Visual Match PASS를 대신하지 않는다.

과거 대화에만 존재하고 저장소 원본이 확인되지 않은 USER 승인 이미지는 가짜 경로를 만들어 연결하지 않는다. `VPR-P0-00 Visual Target Recovery & Registry`에서 실제 원본을 회수·식별한 뒤 Target으로 승격한다.

---

## 5. 폴더와 Naming

### 5.1 권장 Content 경로

```text
/Game/CarFight/UI/HUD/Visual/
├─ DA_CFHUDVisual_Default
├─ Textures/
├─ Materials/
└─ Instances/
```

### 5.2 Naming

```text
Texture2D       = T_UI_<Purpose>
UI Material     = M_UI_<Purpose>
MaterialInstance= MI_UI_<Purpose>
```

새 파일명과 클래스명은 32자를 넘기지 않는다.

---

## 6. DA_CFHUDVisual_Default 현재 계약

C++ 타입:

```text
UCFHUDVisualData
```

현재 필드와 D1-11 Production Bridge 소비 상태는 다음과 같다.

| DataAsset 필드 | 자산 종류 | 현재 Production Bridge 소비 | 비고 |
| --- | --- | --- | --- |
| `VehiclePanelFrame` | Texture2D | **예** | `WBP_CFVehiclePanel.Border_Surface` 9-Slice Frame |
| `VehicleSilhouette` | Texture2D | **예** | `WBP_CFArmorBodyMap.Image_VehicleSilhouette` |
| `ArmorPlates.FrontPlate` | Texture2D | **예** | Front Image |
| `ArmorPlates.RightPlate` | Texture2D | **예** | Right Image |
| `ArmorPlates.RearPlate` | Texture2D | **예** | Rear Image |
| `ArmorPlates.LeftPlate` | Texture2D | **예** | Left Image |
| `ArmorPlates.TopPlate` | Texture2D | **예** | Top Image |
| `ArmorPlates.BottomPlate` | Texture2D | **예** | Bottom Image |
| `SpeedArcTrack` | Texture2D | **간접** | `M_UI_RPMGauge`의 데이터 Texture 입력 `T_UI_RPMTrack`; Widget이 Texture를 직접 Brush로 그리지 않음 |
| `SpeedArcMaterial` | UI Material | **예** | `WBP_CFSpeedGauge.Image_RPMGauge` Brush / Runtime `RPMRatio` 1개 소비 |
| `RadarFrame` | Texture2D | **예** | `WBP_CFRadarPanel.Image_RadarFrame` / Grid·3 Range Ring·외곽 Frame |
| `RadarFriendlyBlip` | Texture2D | **예** | Friendly runtime Contact `UImage` Brush Template |
| `RadarHostileBlip` | Texture2D | **예** | Hostile runtime Contact `UImage` Brush Template |
| `RadarUnknownBlip` | Texture2D | **예** | Unknown/Neutral runtime Contact가 공유하는 Diamond Brush Template |
| `RadarPlayerMarker` | Texture2D | **예** | `WBP_CFRadarPanel.Image_RadarPlayer` |
| `RadarSelectedEdgeBracket` | Texture2D | **예** | range-out 선택 Contact용 `Image_RadarSelectedEdge` 2-Corner Open Bracket |
| `RadarSweepMaterial` | UI Material | **아니오** | `None`; Active Scan Sweep 후속 표현이며 UI-P0-08B 비차단 |
| `SelectedTargetBracket` | Texture2D | **예** | in-range 선택 Contact용 `Image_RadarSelected` 4-Corner Bracket |
| `SelectedWeaponFrame` | Texture2D | **아니오** | 필드만 준비됨. 별도 hookup 필요 |

중요:

```text
DataAsset 필드 존재
≠
현재 Widget이 이미 그 필드를 소비한다.
```

미소비 슬롯은 아트를 먼저 만들어도 화면에 자동 반영되지 않는다. 해당 슬롯의 Production Widget hookup을 별도 D1-11 작업으로 구현한 뒤 연결한다.

---

## 7. P1 — 즉시 제작 가능한 전용 Texture 9종

현재 Production Bridge가 **이미 소비하는 슬롯**부터 제작한다.

| 우선 | 파일명 | DA 슬롯 | 권장 원본 크기 | 포맷 |
| --- | --- | --- | --- | --- |
| P1 | `T_UI_VehSil_LF` | `VehicleSilhouette` | 512×256 | PNG RGBA |
| P1 | `T_UI_Armor_Front` | `ArmorPlates.FrontPlate` | 128×128 | PNG RGBA |
| P1 | `T_UI_Armor_Right` | `ArmorPlates.RightPlate` | 128×128 | PNG RGBA |
| P1 | `T_UI_Armor_Rear` | `ArmorPlates.RearPlate` | 128×128 | PNG RGBA |
| P1 | `T_UI_Armor_Left` | `ArmorPlates.LeftPlate` | 128×128 | PNG RGBA |
| P1 | `T_UI_Armor_Top` | `ArmorPlates.TopPlate` | 128×128 | PNG RGBA |
| P1 | `T_UI_Armor_Bottom` | `ArmorPlates.BottomPlate` | 128×128 | PNG RGBA |
| P1 | `T_UI_SpeedArc` | `SpeedArcTrack` | 512×512 | PNG RGBA |
| P1 | `T_UI_TargetBracket` | `SelectedTargetBracket` | 256×256 | PNG RGBA |

P1의 목적은 임시 Semantic Fallback을 실제 HUD 전용 아트로 교체하는 것이다.

---

## 8. P1 자산별 상세 규격

### 8.1 T_UI_VehSil_LF

목적:
- Armor Body Map 중앙의 방향 기준점.
- 장갑 상태를 읽을 때 차량의 앞/뒤/좌/우를 즉시 인식하게 한다.

규격:

```text
Canvas: 512×256
Background: Transparent
View: **Top-down vehicle**, front points to screen-left
Color: White/Light Gray 기반 Tintable
Text: 금지
Numerics: 금지
```

형태 기준:
- 차량은 **명확한 탑다운 View**여야 하며 사이드뷰로 표현하지 않는다.
- 차량 전방이 **화면 왼쪽**으로 명확히 읽혀야 한다.
- 최종 Production silhouette는 공통 1장이 아니라 차량별 `VehicleData` identity에 대응하는 자산이다. 각 차량의 차체 형태 차이가 Armor Map에서 구분돼야 한다.
- `UCFHUDVisualData`가 VehicleData별 silhouette catalog를 소유하고 기존 단일 `VehicleSilhouette`은 미등록 차량용 compatibility fallback으로 남긴다.
- UI Art 요구만으로 `UCFVehicleData` schema를 확장하지 않는다.
- 작은 크기에서도 차체 외곽과 앞/뒤 방향이 뭉개지지 않아야 한다.

#### 8.1.1 Vehicle-specific silhouette Source Authority — PASS 2026-08-25

차량별 silhouette의 모양은 기존 generic HUD 그림이나 임의 Concept이 아니라 **현재 VehicleData가 실제로 사용하는 ChassisMesh**에서 파생한다.

```text
Runtime selection key
= exact CFVehicleData asset identity

Shape authority
= VehicleData.VehicleVisualConfig.ChassisMesh

Source candidate
= authority ChassisMesh의 top-down projection을 HUD 규격으로 정리한 RGBA silhouette

Orientation
= Vehicle +X Front → screen-left
= Vehicle +Y Right → screen-up
= Vehicle -X Rear  → screen-right
= Vehicle -Y Left  → screen-down
```

현재 exact mapping:

| VehicleData | ChassisMesh | Production Texture family |
| --- | --- | --- |
| `DA_TestSedan` | `Sedan.Sedan` | `T_UI_VehSil_Sedan` |
| `DA_TestSUV` | `SUV.SUV` | `T_UI_VehSil_SUV` |
| `DA_VehicleDefense_TestSUV` | `SUV.SUV` | `T_UI_VehSil_SUV` 공유 |

따라서 Production Source는 Sedan/SUV **2종**이며 Runtime catalog는 VehicleData **3 entry**다. 두 SUV entry는 동일 `T_UI_VehSil_SUV`를 참조한다.

승인·생성된 Source와 Production Texture는 다음과 같다.

```text
Source:
VT05_VehSil_Sedan.png  = 512×256 transparent PNG RGBA
VT05_VehSil_SUV.png    = 512×256 transparent PNG RGBA

Production:
T_UI_VehSil_Sedan
T_UI_VehSil_SUV

Persisted catalog:
DA_TestSedan              → T_UI_VehSil_Sedan
DA_TestSUV                → T_UI_VehSil_SUV
DA_VehicleDefense_TestSUV → T_UI_VehSil_SUV
```

USER는 실제 Source가 정상 transparent PNG임을 확인한 뒤 `문제없는 거면 일단 진행해봐`로 Production 진행을 승인했다. canonical BuildEditor + `CFVehSilProd` process `f7c50e97f58f4ef4bd0e0a64d51474e6`이 Exit 0으로 완료됐고 fresh AssetDump `adset_v1_54b79e58701e4288cd0285c2ced3cda7.e9ce14178c430a4696a0f0ad`에서 Texture2D 2종과 `VehicleSilhouettes Array[3]` exact mapping을 persisted 확인했다. 기존 단일 `VehicleSilhouette=T_UI_VehSil_LF`는 미등록 차량 compatibility fallback으로 유지한다.

금지:

- `SourceArt/UI/HUD/P2/T_UI_VehSil_LF.png` generic polygon을 Sedan/SUV source로 rename해서 사용
- exact ChassisMesh identity가 없는 `VT03_VehSil_LF_v2`를 임의 VehicleData에 bind
- Sedan/SUV 차체 차이를 없애기 위해 하나의 generic silhouette로 통합
- 향후 신규 차량 silhouette을 USER 진행 승인 없이 Production Texture/catalog에 bind

Production Source는 512×256 transparent HUD canvas와 실제 Mesh aspect를 유지한다. line/detail cleanup이 필요해도 차량의 실제 top-down outer silhouette과 앞/뒤 구분을 바꾸는 재디자인은 USER 승인 없이 수행하지 않는다.

### 8.2 Armor Common Plate + Direction Icon 2종

Production 자산은 방향별 완성 Plate 6장을 만들지 않는다.

```text
Common Plate:
T_UI_ArmorPlate
- Transparent RGBA
- White/Gray Tintable
- Mechanical plate/frame only
- Direction glyph / Text / Ratio bake 금지

Direction Icon:
T_UI_ArmorIcon_Arrow
T_UI_ArmorIcon_Chevron2
- 128×128 Transparent
- White Tintable
- canonical orientation = screen-right `→`
- VT-VEH-01 source glyph target-preserving extraction
```

`WBP_CFArmorSector`는 `Image_ArmorPlate + Image_DirectionIcon + Text_Direction fallback + ProgressBar_Armor` 조립 구조를 목표로 한다. 각 Sector instance에서 USER가 `Arrow` 또는 `Chevron2`를 선택하고 UMG Designer의 Render Transform Rotation으로 원하는 방향을 만든다. Front/Right/Rear/Left/Top/Bottom별 별도 Texture identity를 신규 Production 계약으로 만들지 않는다.

6방향의 실제 Gameplay 의미는 Icon 회전이나 화면 위치가 아니라 `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` instance name과 Presenter binding이 소유한다. 따라서 USER가 Sector 위치를 자유롭게 옮겨도 Runtime Armor Ratio 연결은 바뀌지 않는다.

`Text_Direction`은 Icon 미설정/누락 시 fallback/debug Label로만 표시한다. `ProgressBar_Armor`는 기존 `NormalizedArmor` ratio sink를 유지하고 Plate/Icon Texture에는 상태값을 굽지 않는다.

### 8.3 T_UI_SpeedArc

규격:

```text
Canvas: 512×512
Background: Transparent
Content: 부분 원호 Track, Tick Mark, 최소 장식
Text/Number: 금지
Fill Value: Texture에 구워 넣지 않음
```

역할:
- `WBP_CFSpeedGauge`의 계기판 형태를 제공한다.
- 실제 속도 `076`, `km/h`, 단일 Gear Slot(`R` / `N` / 실제 전진 Gear 숫자)은 계속 TextBlock이다.

Historical 주의:
- 아래 정적 Track/P2 분리 설명은 초기 D1-11 제작 당시 기준이다.
- Current Production RPM은 `Image_RPMGauge + M_UI_RPMGauge + T_UI_RPMTrack + RPMRatio` 단일 Material 구조가 우선하며 구형 `Image_RPMTrackArt + ProgressBar_RPMTick00~20` 경로를 재도입하지 않는다.

### 8.4 T_UI_TargetBracket

규격:

```text
Canvas: 256×256
Background: Transparent
Center: 비움
Shape: 4-corner targeting bracket
Base Color: White Tintable
Text: 금지
```

역할:
- Radar의 Selected Contact 강조.
- 향후 World Target Marker와 시각 Family를 공유할 수 있다.

---

## 9. P2 — 잔여 Hookup 선행 자산

RPM Material과 UI-P0-08B Radar 정적/Contact Visual은 Technical Apply를 완료했다. 현재 DataAsset 필드는 있지만 Production Widget이 아직 소비하지 않는 잔여 자산은 다음과 같다.

| 파일/에셋명 | DA 슬롯 | 선행 구현 |
| --- | --- | --- |
| `M_UI_RadarSweep` | `RadarSweepMaterial` | Active Scan Scanning 상태에서만 Sweep Image/Material 회전·시간 계약 연결 |
| `T_UI_WeaponFrame` | `SelectedWeaponFrame` | `WBP_CFWeaponPanel` Selected Frame Image hookup |

잔여 P2 자산은 해당 Runtime/Presentation 의미가 실제로 필요할 때 소비 경로를 먼저 구현·검증한 뒤 제작한다.

### 9.1 CF-FQ-039 VehiclePanel Physical Art Breakdown

USER APPROVED / LOCKED `VT-VEH-01`의 Logical/WBP/Slot 계약을 실제 source art 작업으로 옮길 때 아래 asset identity를 사용한다. Production Target 승인은 완료됐다. repository Source Binding은 USER local PC access가 가능할 때까지 Deferred이며, 현재는 exact-hash local Master를 사용한 Pre-Binding extraction evidence만 허용한다.

#### Existing identity refresh

```text
T_UI_VehPanelFrame
- 기존 Border_Surface 9-Slice underlay 호환용
- Target ornament 최종 authority는 아님

T_UI_RPMTrack
- 비대칭 RPM Track mask
- 21 Tick / 숫자 없음 / x1000 없음

M_UI_RPMGauge
- 기존 single runtime Material 유지
- RPMRatio + 85% Red Zone + Cyan/Red presentation polish

T_UI_VehSil_LF
- 기존 단일 identity는 compatibility fallback으로 보존
- 최종 Production은 VehicleData identity별 `T_UI_VehSil_<VehicleId>` 계열을 HUD Visual catalog에서 선택

T_UI_Armor_Front / Right / Rear / Left / Top / Bottom
- 기존 DA slot/asset identity는 migration compatibility fallback으로 보존
- 신규 Production Visual authority로 사용하지 않음

T_UI_ArmorPlate
- 신규 common tintable mechanical Plate

T_UI_ArmorIcon_Arrow
T_UI_ArmorIcon_Chevron2
- 신규 2-icon Source family
- canonical orientation = `→`
- 각 Sector에서 Designer rotation으로 재사용
```

#### New target-derived assets

| Asset | 종류 | 책임 |
| --- | --- | --- |
| `T_UI_VehFrameOverlay` | RGBA Texture | 896×416 전체 aspect를 보존하는 outer ornament overlay |
| `T_UI_SpeedFrame` | RGBA Texture | 좌측 422×272 Speed/RPM module frame |
| `T_UI_ArmorFrame` | RGBA Texture | 우측 422×272 Armor module frame |
| `T_UI_GearFrame` | RGBA Texture | Gear box frame only. `GEAR`/실제 값은 TextBlock |
| `T_UI_DefBarFrame` | RGBA Texture | Shield/Integrity 공통 mechanical frame + dark track shell |
| `T_UI_DefFillMask` | Linear Mask | Defense fill inset/slanted-left silhouette |
| `T_UI_DefHex` | Linear/Gray Texture | 반복 honeycomb pattern |
| `T_UI_DefChevron` | RGBA/Mask Texture | recovery/repair chevron visual shape |
| `M_UI_DefFill` | UI Material | Mask × Hex + semantic color + local edge glow |

#### No raster asset

```text
RPM                    → TextBlock
Speed / km/h           → TextBlock
GEAR / Gear value      → TextBlock
SHIELD / INTEGRITY     → TextBlock
Defense Current/Max    → TextBlock
Armor direction words  → Production 표시 안 함
```

#### Defense Material boundary

`ProgressBar_Defense`는 Ratio clipping만 담당한다. `M_UI_DefFill`은 Gameplay 상태를 계산하지 않고 다음 입력만 소비한다.

```text
Fill presentation:
- T_UI_DefFillMask
- T_UI_DefHex
- Semantic Color
- Edge Glow Strength

Ratio:
- UProgressBar Percent가 소유
```

Chevron은 `T_UI_DefChevron`을 1~3개 표시하며 Bar 오른쪽에 고정하지 않는다. 기존 `RecoveryChevronCount`와 Fill Ratio를 사용해 현재 Fill 끝점에 붙여 이동한다.

#### Flattened Master cleanup rule

현재 Master는 layered 원본이 아닌 flattened bitmap이다. 따라서 frame/gear/silhouette를 rectangular crop 그대로 쓰지 않는다.

```text
허용:
- target-preserving alpha extraction
- 배경/문자/샘플 수치 제거 cleanup
- edge/alpha cleanup
- mask 변환

금지:
- 비어 있는 영역을 새로운 스타일로 임의 재디자인
- 다른 AI concept의 장식 요소 혼합
- Target에 없는 shape/detail 추가
```

Source-side Review Sheet에서 실제 분해 파츠를 USER가 확인한 뒤 Unreal Import를 허용한다.

### 9.2 VT-VEH-01 Frame Family Extraction

상세 좌표·mask·Review Gate와 Local Pre-Binding evidence는 `InGameUIVisualPlan.md v0.1.19 §5.6.6~5.6.8`이 소유한다. Art Spec에서는 자산 경계만 고정한다.

```text
Master native: 1536×713
WBP display: 896×416
Scale: 7/12

Frame family:
- T_UI_VehFrameOverlay = full native canvas / outer perimeter only
- T_UI_SpeedFrame      = source x34..758, y27..494 / display 422×272
- T_UI_ArmorFrame      = source x778..1502, y27..494 / display 422×272
- T_UI_GearFrame       = preliminary x580..694, y248..420 / alpha-tight crop after preview
```

Source extraction 결과는 원본 픽셀의 위치·line weight·corner language를 보존한다. Text/샘플 숫자/다른 Module 정보는 alpha out하고, 제거 대상이 기존 frame line을 가린 경우에만 명확한 동일 선의 최소 연속성 복원을 허용한다. 새 ornament 생성은 금지한다.

Frame family는 `VT01_FrameReview_v2.png`의 original / checkerboard parts / 896×416 frame-only composite를 USER가 확인해 **USER PASS**했다. 현재 local pre-binding 산출물은 F1 `82baf36f...6645`, F2 `47f2aeed...4bbc`, F3 v2 `aeddd7b0...6b47f`, F4 `0442a1ac...ef4a` SHA로 고정되며 새 관련 결함이 없는 한 Frame Review를 반복하지 않는다. repository SourceArt 승격 전에는 Production Asset으로 취급하지 않는다.

### 9.3 RPM-S1 Runtime Source Mask Pack

`VT-VEH-01` Speed/RPM module을 실제 작동형 `RPMRatio` consumer로 옮기기 위한 Local Source Pack이다. 현재 Runtime identity `Image_RPMGauge + M_UI_RPMGauge + RPMRatio`를 유지하며 구형 `ProgressBar_RPMTick00~20` 21개 Widget 구조는 재도입하지 않는다.

#### Source geometry / QA

```text
Source canvas = 724×467
Path = 좌측 세로 → 좌상단 1/4 곡선 → 상단 긴 수평
TickCount = 21
Progress levels = 21
RedStartRatio = 0.85
Red Tick = 4

QA:
RpmRatio 0.00 = 0 Tick
RpmRatio 0.25 = 5 Tick
RpmRatio 0.50 = 10 Tick
RpmRatio 0.85 = 17 Tick
RpmRatio 1.00 = 21 Tick
AUTO QA = PASS
```

초기 제작본은 curve angle 방향과 segment 접촉 때문에 21개가 실제 독립 component로 유지되지 않아 폐기했다. 최종 Pack만 Current Source Candidate로 사용한다.

#### Working source masks

```text
T_UI_RpmFrame      = clean double guide rail source
T_UI_RpmTrackOff   = inactive tick source mask
T_UI_RpmFillMask   = 21-level grayscale progress map
T_UI_RpmTickMask21 = exactly 21 independent segments
T_UI_RpmRedMask    = last 4 Tick / 0.85+ QA reference mask / Runtime Import 안 함
VT02_RpmLabelRef   = `RPM` typography reference only / Import 금지
```

위 working mask는 제작·QA 책임을 분리하기 위한 source-side 산출물이다. Runtime에서 여러 Texture Sample로 각각 소비하지 않는다.

#### Production Import candidate — `T_UI_RPMMaskPack`

```text
Asset: T_UI_RPMMaskPack
Canvas: 724×467
Format: RGBA PNG
sRGB: Off
Compression/Usage: Linear UI Mask Texture
SHA-256: 6cc32459a972a79c6aa50983050aa5fdb8656a4efa5a7caa9475fda66def8777

Channel packing:
R = 21-step Progress Map
G = TickMask21
B = Guide Rail Mask
A = constant 255 opaque safety channel

Derived in Material:
Red Zone = step(RedStartRatio=0.85, R) × G
```

`TrackOff`는 별도 Texture Sample을 사용하지 않고 `G × InactiveBrightness`로 `M_UI_RPMGauge`에서 계산한다. Red Zone도 별도 Texture/Channel 없이 R의 per-pixel progress와 `RedStartRatio`에서 파생한다. PNG zero-alpha RGB 처리로 R/G/B mask가 변형될 위험을 피하기 위해 A는 항상 255다. 따라서 최종 런타임의 새 Mask Texture Sample은 1개다.

```text
WBP_CFSpeedGauge.Image_RPMGauge [KEEP]
└─ M_UI_RPMGauge [KEEP identity]
   ├─ T_UI_RPMMaskPack [new packed texture input]
   └─ RPMRatio [KEEP scalar parameter]

Text_RPMLabel = TextBlock `RPM`
x1000 = 표시 안 함
절대 RPM 숫자 눈금 = 표시 안 함
```

`VT02_RpmLabelRef`는 Target typography 참고용이며 UE Texture로 Import하지 않는다. 실제 `RPM` 글자는 Style/Font owner를 사용하는 TextBlock이다.

현재 상태는 **Local Source Candidate / USER Visual PASS / repository Source Binding Deferred**다. `VT02_RpmRuntimePack_Review_FINAL3.png`는 USER PASS로 닫았으며 새 관련 결함이 없는 한 반복하지 않는다. repository source binding + exact hash readback을 완료한 뒤에만 UE Import 후보로 승격한다.

### 9.4 Armor Modular Source / Migration Contract

기존 `VT03` local pack은 USER PASS 전에 구조가 변경돼 **Superseded local experiment**다. 중앙 Vehicle wireframe extraction은 특정 차량용 silhouette 제작 기법 참고로만 남고, 방향별 완성 Plate 6종은 Production 승격하지 않는다.

현재 local Source evidence:

```text
VT04_ArmorIcon_Arrow.png
- 128×128
- canonical `→`
- tintable white-alpha

VT04_ArmorIcon_Chevron2.png
- 128×128
- canonical `→→`
- tintable white-alpha

VT04_ArmorIcon2_Review.png
- 각 Source의 0 / 90 / 180 / 270° Designer rotation 확인

VT04_ArmorIcon2_Manifest.json
- exact local source/rotation contract
```

Production target structure:

```text
WBP_CFArmorBodyMap
├─ Image_VehicleSilhouette   // VehicleData identity별 Texture 선택
└─ WBP_CFArmorSector × 6     // Position/Size Designer-owned

WBP_CFArmorSector
├─ Image_ArmorPlate          // common Plate
├─ Image_DirectionIcon       // Arrow / Chevron2 + Designer rotation
├─ Text_Direction            // fallback/debug only
└─ ProgressBar_Armor         // existing runtime ratio sink
```

HUD visual data migration은 vehicle-specific silhouette catalog, common Plate, Arrow/Chevron2 reference를 additive 추가하고 기존 `VehicleSilhouette`과 `ArmorPlates.Front~Bottom`은 compatibility fallback으로 보존한다. `UCFVehicleData`는 HUD Art 때문에 변경하지 않는다.

현재 상태는 **Armor Modular Runtime/WBP Technical PASS / Common Plate + Direction Arrow·Chevron2 repository Source Bound + UE Production Assetization PASS / Vehicle-specific Silhouette Production Art Pending / exact Master Source Binding Pending / USER Visual Pending**이다.

Direction 2-Icon Production source/asset 계약:

```text
SourceArt/UI/HUD/VT/VT04_ArmorIcon_Arrow.png
→ /Game/CarFight/UI/HUD/Visual/T_UI_ArmorIcon_Arrow
→ DA_CFHUDVisual_Default.ArmorDirectionArrow

SourceArt/UI/HUD/VT/VT04_ArmorIcon_Chevron2.png
→ /Game/CarFight/UI/HUD/Visual/T_UI_ArmorIcon_Chevron2
→ DA_CFHUDVisual_Default.ArmorDirectionChevron2

128×128 / TEXTUREGROUP_UI / TC_EditorIcon / NoMipmaps / sRGB=true
Designer Position/Size/Icon Choice/Rotation = USER-owned / unchanged
```

`ArmorCommonPlate`, `ArmorDirectionArrow`, `ArmorDirectionChevron2` 세 Production entry는 모두 non-null persisted binding이 확인됐다. 따라서 `HasModularArmorArt()`가 검사하는 field 조건은 충족됐지만 함수 자체를 별도 실행 검증하지 않았으므로 함수 실행 PASS로 확대하지 않는다. `VehicleSilhouettes`는 count 0을 유지한다.

현재 persisted technical structure:

```text
WBP_CFArmorBodyMap
- 기존 Image_VehicleSilhouette + WBP_ArmorFront/Right/Rear/Left/Top/Bottom 유지
- six-sector Canvas Position/Size mutation 0

WBP_CFArmorSector
- Overlay_Plate에 Image_DirectionIcon additive 저장
- Production Direction Icon Texture 미바인딩 상태에서는 Image_DirectionIcon = Collapsed
- Text_Direction = fallback/debug 표시
```

Runtime은 VehicleData identity별 silhouette catalog를 소비할 수 있고 modular Direction Icon rotation도 지원한다. common Plate / `VT04_ArmorIcon_Arrow` / `VT04_ArmorIcon_Chevron2`는 repository Source Binding·UE Import·DA binding까지 PASS했다. 남은 Production Art는 VehicleData identity가 확정된 vehicle-specific silhouette이며, exact identity가 없는 Concept silhouette는 임의 차량에 연결하지 않는다. 따라서 이 Technical PASS를 Production Art 적용 또는 USER Visual PASS로 해석하지 않는다.

---

## 10. Unreal Import 규격

### 10.1 Texture2D 기본

UE 5.8 기준 권장 기본값:

```text
Texture Group: UI
Compression Settings: UserInterface2D (RGBA)
Mip Gen Settings: NoMipmaps
Alpha: Straight Alpha / 투명 배경
```

색상 Texture:
- `sRGB = On` 기본.

Material Mask 전용 Texture:
- `sRGB = Off`를 기본으로 검토한다.

### 10.2 알파 가장자리

- 투명 영역의 RGB가 검은색으로 오염되어 밝은 UI에서 검은 Halo가 생기지 않도록 한다.
- 얇은 Line Art는 1px 단위보다 충분한 원본 두께를 확보한다.
- Export 후 25% 크기에서도 윤곽이 읽히는지 확인한다.

### 10.3 UI Material

`M_UI_SpeedArc`, `M_UI_RadarSweep` 같은 HUD Material은 다음을 기본으로 한다.

```text
Material Domain = User Interface
Blend Mode = Translucent 또는 AlphaComposite 검토
Gameplay 판정 로직 = 금지
Scalar/Vector Parameter 기반 시각 표현만 허용
```

---

## 11. Art 연결 Workflow

현재 Production HUD에서는 **persisted UMG Designer Asset이 Layout SSOT**다. 일반 `-Apply`는 기존 Widget Tree를 다시 생성해 Brush와 Layout을 함께 덮어쓰는 갱신 수단으로 사용하지 않는다.

기존 Production Widget의 Art 교체 절차는 다음을 따른다.

```text
1. PNG/Material 제작 또는 수정
2. /Game/CarFight/UI/HUD/Visual/에 Import/갱신
3. UI Texture/Material 규격 확인
4. DA_CFHUDVisual_Default의 해당 필드 연결 확인
5. 이미 연결된 Runtime Material/Reference만 바뀌는 경우 non-Designer Asset targeted apply 사용
6. 저장 Widget의 Brush 자체를 바꿔야 하면 Slot 전용 layout-preserving visual update 또는 UMG Designer에서 명시적으로 변경
7. Tools/RunUIHUDProduction.ps1 -Readback으로 의미 구조 Validate
8. 필요 시 AssetDump persisted 확인
9. WBP_CFInGameHUD 1920×1080 Designer User Preview
```

중요:

```text
기존 Widget의 Art 갱신을 위해 scaffold Build를 다시 실행하지 않는다.
현재 공용 Bridge Build는 신규/빈 Asset 최초 Scaffold 전용이다.
기존 Asset용 targeted 경로는 Position·Size·Anchor·Alignment·Padding·AutoSize를 수정하지 않아야 한다.
해당 Art 슬롯에 layout-preserving updater가 아직 없으면 full rebuild로 우회하지 않고 별도 최소 visual update 경로를 만든다.
```

현재 동적 RPM 경로는 `-RpmGaugeOnly`가 Texture/Material/VisualData만 갱신하고 기존 SpeedGauge는 Validate-only로 보호하는 예외 없는 대표 구현이다.

---

## 12. Semantic Icon과 전용 HUD Art의 차이

### Semantic Icon

소유:
- `DA_CFUIStyle_Default.IconSet`

예:
- Vehicle
- Armor
- Shield
- Integrity
- Ammo
- Heat
- Cooldown
- Reload
- Target
- RadarContact
- Warning
- Critical

역할:
- 여러 UI에서 공통 의미를 표현하는 작은 아이콘.

### HUD 전용 Art

소유:
- `DA_CFHUDVisual_Default`

예:
- Vehicle Body Silhouette
- 방향별 Armor Plate
- Speed Gauge Arc
- Radar Frame/Sweep
- Selected Target Bracket
- Selected Weapon Frame

역할:
- 특정 HUD 모듈의 고유한 시각 구조를 표현하는 교체 가능한 그래픽 부품.

Semantic Icon을 확대해서 전용 HUD Art를 대신하지 않는다.

---

## 13. 품질 검증 기준

각 P1 Texture는 다음을 통과해야 한다.

### 구조
- Text/Number가 Texture에 포함되지 않는다.
- 투명 배경을 사용한다.
- Widget Tree 변경 없이 교체 가능하다.
- 해당 DA 슬롯과 1:1로 대응한다.

### 가독성
- 1920×1080 Designer 실제 표시 크기에서 형태가 읽힌다.
- HUD 중앙 전투 시야보다 시각적으로 강하지 않다.
- 주요 숫자보다 장식이 먼저 눈에 들어오지 않는다.

### Style
- 같은 Family 자산은 Line Weight와 Corner 언어가 일치한다.
- Style Token Tint가 적용돼도 형태가 무너지지 않는다.
- Cyan/Amber/Red로 Tint했을 때 각각 시스템/주의/적대 의미가 유지된다.

### 기술
- UI Texture Import 설정 확인.
- 잘못된 검은 Alpha Halo 없음.
- 과도한 Texture Resolution 없음.
- 새 자산 연결 뒤 Production Apply/Readback PASS.
- 전체 CarFight 보호 회귀 유지.

---

## 14. D1-11 Art 단계

```text
D1-11-ART-00  HUD Art Asset Spec 문서화                     = DONE
D1-11-ART-01  P1 Texture 9종 제작                          = SOURCE 9/9 GENERATED / TECHNICAL PASS / REFERENCE BASELINE
D1-11-ART-02  P2 Texture 9종 Import + Vehicle VisualData 연결 = TECHNICAL PASS / PERSISTED
D1-11-ART-03  Production Apply/Readback/AssetDump            = TECHNICAL PASS / PERSISTED
D1-11-ART-04  Designer·1920×1080 User Preview               = USER VISUAL PENDING
D1-11-ART-05  P2 Hookup 필요성/범위 결정                     = NOT STARTED
```

`D1-11` 최종 PASS 전 `D1-12`로 이동하지 않는다.

### 14.1 D1-11-ART-01 현재 체크포인트

```text
Source Root: SourceArt/UI/HUD/P1
Generator: Tools/GenerateUIHUDArtP1.ps1 v1.0.1
Technical Review: SourceArt/UI/HUD/P1/Review/art01_review.json
User Review Sheet: SourceArt/UI/HUD/P1/Review/P1_HUD_Review.png
Generated: 9/9
Technical PASS: 9/9
Black Alpha Edge Risk: 0/9
User Visual Approval: 0/9 / Pending
ApprovedAssets: []
Unreal Import: 0
DA Connection: 0
Production Apply: 0
```

현재 생성된 9종:

```text
T_UI_VehSil_LF.png
T_UI_Armor_Front.png
T_UI_Armor_Right.png
T_UI_Armor_Rear.png
T_UI_Armor_Left.png
T_UI_Armor_Top.png
T_UI_Armor_Bottom.png
T_UI_SpeedArc.png
T_UI_TargetBracket.png
```

### 14.2 2026-08-19 Visual Foundation / VehiclePanel Vertical Slice

사용자 결정으로 UI-P0-07 Target Knowledge 착수 전에 D1-11-ART를 한 번 재개해 실제 게임 UI 시각 기반을 검증한다. 기존 P1 Source와 기술 검토를 다시 생성하지 않는다.

첫 Vertical Slice 범위:

```text
VehiclePanel 소비 8종
- T_UI_VehSil_LF.png
- T_UI_Armor_Front.png
- T_UI_Armor_Right.png
- T_UI_Armor_Rear.png
- T_UI_Armor_Left.png
- T_UI_Armor_Top.png
- T_UI_Armor_Bottom.png
- T_UI_SpeedArc.png

후속 보존
- T_UI_TargetBracket.png
  → Target/Radar Visual 단계에서 사용 가능
```

현재 Gate:

```text
Art Ingress = PASS
→ Tools/GenerateUIHUDArtP1.ps1 v1.0.1이 저장소 SourceArt/UI/HUD/P1에 직접 생성

Technical Review = PASS
→ Review/art01_review.json 9/9 TechnicalPass=true

P1 Technical Reference = PRESERVED
→ Review/P1_HUD_Review.png은 Historical/Reference baseline으로만 사용

Designer Layout Ownership Remediation = TECHNICAL PASS
→ `CFUIHUDProdEditorBridge.h v1.9.0 / .cpp v1.12.0`: 기존 Designer Tree Build fail-closed / 신규·빈 Asset만 최초 Scaffold
→ `ApplyUIHUDProduction.py v1.5.0 / RunUIHUDProduction.ps1 v1.4.0`: 기존 Widget Validate-only
→ Official Build `9125557b4e214a4ebe2bd642099929aa` PASS
→ saved readback `b543596ea3404a8084b46e5349ce7e84` PASS / saved0
→ `ArmorMapOnly` `ad6e8d64733249b0be26d8ab9506acca` PASS / layout preserved2 / rebuilt0 / compiled0 / saved0 / exact mutated0

P2 USER Visual Review = DEFERRED / USER PASS 미부여
→ WBP_CFArmorBodyMap 위치·크기 Designer 편집성은 USER 직접 확인 PASS
→ SpeedGauge·VehiclePanel 개별 편집성 + Frame·RPM Track·좌향 Vehicle·Armor 6방향 배치·가독성과 전체 Balanced Combat 인상 판정은 USER 결정으로 후속 Deferred

P2 Technical Apply = PASS
- SourceArt/UI/HUD/P2 9종 → `/Game/CarFight/UI/HUD/Visual` Texture2D 9종 persisted import
- DA_CFHUDVisual_Default → VehiclePanelFrame / VehicleSilhouette / ArmorPlates 6 / SpeedArcTrack 연결
- WBP_CFSpeedGauge / WBP_CFArmorBodyMap / WBP_CFVehiclePanel Build·Compile·Validate·Save
- Fresh AssetDump persisted readback PASS
- Official Build Job `74166fb3ad0442a5b10bbf02c6f69d12` Result Succeeded
```

현재 남은 순서:

```text
1. Visual: SpeedGauge·VehiclePanel 개별 편집성과 전체 Balanced Combat Visual Review는 Deferred / USER PASS 미부여
2. Runtime: 사용자 지시에 따라 UI-P0-07 Target Knowledge 진행
3. Visual을 다시 열 때는 기존 persisted Designer Layout을 보존하고 발견된 결함만 P2 art·배치 범위에서 최소 교정
```

Visual 작업은 `Gameplay → Provider → Presenter → ViewData → Production Widget` Runtime 계약을 변경하지 않는다. 시각 품질을 위해 Gameplay 판정이나 데이터 계산을 Widget/Texture/Material로 이동하지 않는다.

판정 규칙:

- `Technical PASS`는 해상도·Alpha·투명 배경·비어 있지 않은 실제 픽셀·검은 반투명 Edge 위험·Text 비포함·Tintable White/Gray 제작 계약을 통과했다는 의미다.
- 완전 투명 픽셀의 RGB는 PNG/GDI+ 디코더에서 보존되지 않을 수 있으므로 `TransparentWhiteCorners`는 PASS 조건으로 사용하지 않는다. 실제 검은 Halo 위험은 반투명 Edge RGB로 별도 검사한다.
- 기술 검토는 아트 승인과 동일하지 않다. 현재 `UserVisualApproval=false`, `ImportAllowed=false`를 9종 모두 유지한다.
- 사용자는 9종을 개별 승인하거나 수정 요청할 수 있다. ART-02는 승인된 파일만 Import·DA 연결 대상으로 받는다.
- 승인되지 않은 파일을 Semantic Fallback 교체 대상으로 자동 사용하지 않는다.

---

## 15. 보호 범위

- D1-07 승인 Slot 위치와 중앙 전투 시야를 변경하지 않는다.
- `WBP_CFInGameHUD` Production Root 1 + Panel 6 + Element 2 구조를 이미지 편의를 위해 다시 합치지 않는다.
- Gameplay Actor/Component를 Art Widget에서 직접 조회하지 않는다.
- UI-P0-03 ViewData를 이 Art 단계에서 선행 구현하지 않는다.
- 기존 AimReticle·TargetSelect·Pause UI와 unrelated dirty 에셋을 변경하지 않는다.
- P1 Art 제작을 이유로 D1-12를 시작하지 않는다.

---

## 16. Changelog

### v0.4.18 - 2026-08-25

- authority ChassisMesh 기반 `VT05_VehSil_Sedan.png` / `VT05_VehSil_SUV.png` Source를 실제 512×256 transparent PNG RGBA로 생성·검증하고 USER 진행 승인 뒤 Production Texture 2종으로 Assetize했다.
- persisted `DA_CFHUDVisual_Default.VehicleSilhouettes` exact VehicleData 3 entry를 fresh AssetDump로 확인했으며 두 SUV VehicleData는 동일 `T_UI_VehSil_SUV`를 공유한다. legacy `VehicleSilhouette` fallback은 보존한다.
- canonical BuildEditor + `CFVehSilProd` process `f7c50e97f58f4ef4bd0e0a64d51474e6` Exit 0과 AssetDump dataset `adset_v1_54b79e58701e4288cd0285c2ced3cda7.e9ce14178c430a4696a0f0ad`를 Technical evidence로 고정했다.
- 다음 VehiclePanel Production Art family를 Defense common frame/mask/hex/chevron + `M_UI_DefFill`로 전진하고 exact `VT-VEH-01` Master Source repository binding은 별도 Pending으로 유지했다.

### v0.4.17 - 2026-08-25

- vehicle-specific silhouette의 형상 Source Authority를 `VehicleData.VehicleVisualConfig.ChassisMesh`로 확정하고 현재 3 VehicleData / 2 unique ChassisMesh mapping을 Art 계약에 추가했다.
- `DA_TestSedan`은 `T_UI_VehSil_Sedan`, `DA_TestSUV`와 `DA_VehicleDefense_TestSUV`는 공유 `T_UI_VehSil_SUV` family로 진행한다. runtime catalog key는 기존 exact VehicleData identity를 유지한다.
- P2 generic silhouette와 Chassis identity 없는 VT03 후보를 특정 차량 authority에서 제외했다. planned Source Candidate는 `VT05_VehSil_Sedan.png`, `VT05_VehSil_SUV.png`이며 USER Visual Review 전에는 Import/Bind하지 않고 `VehicleSilhouettes` count 0을 유지한다.
- 상위 Plan/Roadmap 포인터를 v0.1.23/v0.1.23으로 동기화했다.

### v0.4.16 - 2026-08-25

- `VT04_ArmorPlate.png` 98×130 exact Source를 `T_UI_ArmorPlate`로 Assetize하고 `DA_CFHUDVisual_Default.ArmorCommonPlate`에 persisted binding했다.
- common Plate + Arrow + Chevron2 세 modular Armor field의 non-null 조건이 충족됐음을 반영했다. `HasModularArmorArt()` 함수 자체 실행 검증은 별도 미수행으로 경계를 유지한다.
- `VehicleSilhouettes` count 0을 유지하고 다음 Art Gate를 vehicle-specific silhouette Source Authority / VehicleData identity mapping으로 좁혔다.

### v0.4.15 - 2026-08-25

- VT04 canonical Arrow/Chevron2 2종을 repository Source → Production Texture2D → HUD Visual Data field까지 실제 연결한 현재 Art 계약을 반영했다.
- 두 Icon은 128×128 UI Texture로 Assetize됐고 USER-owned per-sector 배치/크기/Icon 선택/회전은 건드리지 않았다.
- common Plate와 vehicle-specific silhouette Art, exact VT-VEH-01 Master Source는 Pending으로 명시해 partial Assetization을 전체 Armor Production 완료로 확대하지 않았다.

### v0.4.14 - 2026-08-25

- Armor modular 구조의 runtime/WBP Technical closure를 반영했다. `WBP_CFArmorSector.Image_DirectionIcon`은 additive persisted 상태이고 BodyMap six-sector Designer Slot은 보존됐다.
- `CFUIHUDProdEditorBridge.h v1.12.1 / .cpp v1.15.1`의 UE 5.8 GUID-safe targeted migration과 VehicleData별 silhouette runtime consumer를 현재 Art hookup 기반으로 기록했다.
- 실제 common Plate/Arrow/Chevron2/차량별 silhouette Production Texture의 Source Binding·Import·DA binding과 USER Visual은 Pending으로 유지했다.

### v0.4.13 - 2026-08-25

- USER 결정으로 Armor Art를 6방향 완성 Plate Texture에서 `T_UI_ArmorPlate` 공통 Plate + `T_UI_ArmorIcon_Arrow` / `T_UI_ArmorIcon_Chevron2` 2종 조립 구조로 변경했다. canonical icon 방향은 `→`이며 각 Sector의 종류/회전/위치/크기는 UMG Designer가 소유한다.
- Vehicle silhouette는 공통 1장이 아니라 `VehicleData` identity별 HUD silhouette catalog에서 선택하도록 변경했다. `UCFVehicleData` schema는 건드리지 않고 `UCFHUDVisualData`가 catalog를 소유하며 기존 단일 `VehicleSilhouette`은 fallback으로 보존한다.
- `VT04` 2-icon local Source/Review/Manifest를 Current evidence로 등록하고 `VT03` six-direction pack은 USER PASS 없이 Superseded로 내렸다. 기존 `ArmorPlates.Front~Bottom`은 migration compatibility fallback이다.
- Plan/Roadmap/VehiclePanelSpec 포인터를 v0.1.19/v0.1.19/v0.20.6으로 동기화했다. WBP/C++/UE Import mutation은 아직 0이다.

### v0.4.12 - 2026-08-25

- USER가 `VT01_FrameReview_v2` Frame Family를 PASS했다. Frame source family는 새 관련 결함이 없는 한 review를 반복하지 않고 Pre-Binding evidence로 보존한다.
- `ARMOR-S1` source pack을 추가했다. `VT03_VehSil_LF_v2` full wireframe silhouette와 target Rear-card 기반 common tintable mechanical Plate, 6방향 icon-only variant를 준비했다. 방향 glyph는 VT-VEH-01 source pixel mirror/rotation만 사용하며 Text bake는 0이다.
- 기존 `WBP_CFArmorSector`의 `Image_ArmorPlate + Text_Direction fallback + ProgressBar_Armor` runtime 경계를 유지한다. Plan/Roadmap 포인터를 v0.1.18/v0.1.18로 동기화했고 Source Binding/UE Import/WBP mutation은 0이다.

### v0.4.11 - 2026-08-25

- USER가 `VT02_RpmRuntimePack_Review_FINAL3`를 확인해 RPM-S1 Visual을 PASS했다. 이 PASS는 작동형 RPM source/mask family에 한정하며 Frame 및 전체 VehiclePanel PASS로 확대하지 않는다.
- Production Import 후보는 `T_UI_RPMMaskPack` 최종 안전 패킹 `R=Progress / G=Tick21 / B=GuideRail / A=255 opaque`를 유지하고 Red Zone은 `RedStartRatio=0.85`에서 파생한다. `RPM`은 TextBlock, `x1000`/절대 숫자 눈금은 계속 제외한다.
- Plan/Roadmap 포인터를 v0.1.17/v0.1.17로 동기화했다. repository Source Binding, UE Import, WBP/Material mutation은 아직 수행하지 않았다.

### v0.4.10 - 2026-08-25

- `VT-VEH-01` 작동형 RPM용 `RPM-S1 Source Mask Pack`을 추가했다. 최종 QA는 21 independent Tick / 21 Progress levels / 마지막 4 Tick Red Zone / Ratio 0·0.25·0.50·0.85·1.00 = 0·5·10·17·21 Tick으로 PASS했다.
- Production Import 후보를 `T_UI_RPMMaskPack` 724×467 RGBA 한 장으로 channel-pack했다. 최종 안전 배치는 R=Progress, G=Tick21, B=GuideRail, A=255 constant opaque이며 sRGB Off Linear Mask로 사용한다. Red Zone은 `RedStartRatio=0.85`와 R/G에서 Material이 파생한다.
- `TrackOff`는 G channel에서 Material 계산하고 기존 `Image_RPMGauge + M_UI_RPMGauge + RPMRatio` single runtime 구조를 유지한다. `RPM`은 TextBlock, `x1000`과 절대 숫자 눈금은 제외한다.
- Local source/review 상태만 기록했으며 repository Source Binding, UE Import, WBP/Material mutation은 아직 수행하지 않았다. Plan/Roadmap 포인터를 v0.1.16/v0.1.16으로 동기화했다.

### v0.4.9 - 2026-08-25

- USER local PC access 제약을 반영해 repository Source Binding은 Deferred로 유지하고 exact-hash VT-VEH-01 기반 Local Pre-Binding Frame extraction evidence를 허용했다.
- F3 ArmorFrame v2의 보수적 alpha cleanup과 F1~F4 SHA, `FrameMock_v2`, `FrameReview_v2`를 Art evidence로 기록했다. 새 ornament/line 생성은 없으며 UE Import는 아직 금지 상태다.
- Plan/Roadmap 포인터를 v0.1.15/v0.1.15로 동기화했다.

### v0.4.8 - 2026-08-24

- USER가 Master를 `VT-VEH-01 Production Visual Target`으로 명시 승인·LOCK한 상태를 반영했다. Candidate 기반 문구를 Current에서 제거했다.
- Frame family extraction 경계를 `T_UI_VehFrameOverlay / T_UI_SpeedFrame / T_UI_ArmorFrame / T_UI_GearFrame`으로 고정하고 native 1536×713 → display 896×416 좌표 계약을 Art Spec에 projection했다.
- Frame 결과는 `VT_VehFrame_Review.png` USER source-art review 전에는 UE Import하지 않는다. 실제 source binding/extraction은 아직 Pending이다.
- Plan/Roadmap 포인터를 v0.1.14/v0.1.14로 동기화했다.

### v0.4.7 - 2026-08-24

- CF-FQ-039 Slot Contract closure를 반영해 Armor를 icon-only production family로 고정하고 `Text_Direction`을 missing-texture fallback/debug 전용으로 제한했다.
- `CAND-VEH-MASTER-01` 기반 VehiclePanel Physical Art Breakdown을 추가했다. 기존 `T_UI_RPMTrack / M_UI_RPMGauge / T_UI_VehSil_LF / Armor 6종` identity는 refresh하고 신규 asset은 Frame/Defense에 필요한 9종으로 한정했다.
- Defense fill을 `T_UI_DefFillMask + T_UI_DefHex + M_UI_DefFill`로 분리하고 ProgressBar는 Ratio clip만 소유하도록 고정했다. Chevron은 fixed ornament가 아니라 기존 recovery/repair state의 fill-end 동적 표현을 유지한다.
- flattened Master는 target-preserving extraction/cleanup만 허용하고 다른 스타일을 생성·혼합하지 않는 source-art 규칙을 추가했다.
- Plan/Roadmap/Vehicle Spec 포인터를 v0.1.13/v0.1.13/v0.20.5로 동기화했다. 실제 Art 생성·UE Import는 아직 수행하지 않았다.

### v0.4.6 - 2026-08-24

- USER 승인으로 VehiclePanel 상단 Speed/RPM : Armor Composition을 50:50으로 교체했다. 896×416 기준 `SpeedGauge 422×272 / Gap 12 / ArmorBodyMap 422×272`를 Current Art slot 기준으로 반영했다.
- Vehicle 의미/배치 owner를 `InGameUIVehiclePanelSpec.md v0.20.4`, 상위 Plan/Roadmap을 v0.1.12/v0.1.12로 동기화했다.
- 이 변경은 Art slot bounds 계약만 갱신하며 Texture/Material/UE Asset mutation은 수행하지 않는다. Master Candidate 전체 `VT-VEH-01` 승격은 Pending이다.

### v0.4.5 - 2026-08-24

- USER 결정으로 `WBP/Primitive-first → Concept 후맞춤` 경로를 명시적으로 Deprecated 처리하고 `Master Target Lock → Logical Breakdown → WBP Skeleton → Slot Contract → Physical Art Breakdown → Assembly`를 CF-FQ-039의 유일한 Production Art 제작 방식으로 고정했다.
- 선형 Runtime Ratio sink로 ProgressBar를 사용할 수는 있으나 Raw ProgressBar 외형을 최종 Production Visual로 승인하지 않도록 역할을 분리했다.
- 승인 Target을 대체하는 AI 신규 미감 생성과 직접 Import를 금지했다. AI는 기본적으로 Target-preserving 추출/정리/마스크 보조만 허용하며 새 생성 픽셀은 USER 별도 승인 전 Production 사용 금지다.
- 기존 P1/P2/Primitive 기반 결과는 Technical Scaffold/Reference로만 보존하고 새 Visual authority로 역승격하지 않는 Migration 규칙을 추가했다.
- 상위 Plan/Roadmap 포인터를 v0.1.10으로 동기화했다.

### v0.4.4 - 2026-08-24

- CF-FQ-039 Armor technical slice의 실제 구현을 반영해 기존 P2 6방향 Texture를 canonical icon family로 확정했다.
- 방향 Texture/Icon 우선 + `Text_Direction` fallback 계약을 명시하고, Plate Texture에 단어 Label·수치·상태색을 굽지 않는 역할 분리를 보강했다.
- `WBP_CFArmorSector`가 Runtime Armor Ratio에 따라 Plate와 세로 Bar를 동일 Presentation 상태색으로 Tint하는 현재 소비 계약을 Art Spec에 반영했다.
- 신규 Armor Texture/Content Asset은 만들지 않았으며 Broken X/갈라진 Plate 같은 추가 Art는 USER Visual Review 이후 후속 refinement로 남겼다.

### v0.4.3 - 2026-08-24

- CF-FQ-039 VehiclePanel editable composition 결정을 반영해 `WBP_CFArmorSector`를 canonical common Armor slot으로 명시하고 별도 `WBP_CFArmorSlot` 중복 생성을 금지했다.
- `WBP_CFArmorBodyMap`의 6개 Sector와 VehicleSilhouette가 독립 Canvas Slot으로 USER Designer 재배치 가능해야 한다는 ownership을 재확인했다.
- `T_UI_VehSil_LF` 규격을 약한 top-down 허용에서 **명시적 Top-down + screen-left front**로 강화했다. 현재 SourceArt PNG 직접 관측도 이 구조와 일치한다.
- Armor Plate Texture에는 방향 텍스트를 굽지 않고, 방향 Icon/Label은 별도 Widget 표현으로 유지해 사용자 편집성을 보존하도록 역할을 분리했다.

### v0.4.2 - 2026-08-23

- CF-FQ-039 Plan/Roadmap v0.1.2와 Supporting 문서 최신 버전으로 Current 포인터를 동기화했다.
- 넓게 정의돼 있던 Visual Target 승격 규칙을 Direction Reference / Composition Target / Production Visual Target 3단계 계약으로 교정했다.
- `좋다/이 방향이 좋다`를 Production Target 승인으로 자동 해석하지 않도록 Plan의 승인 범위 계약과 일치시켰다.

### v0.4.1 - 2026-08-23

- 새 Visual Plan/Roadmap v0.1.1 포인터로 동기화했다.
- `InGameUIAssetizationSpec`을 현재 구현 SSOT가 아닌 Production Structure Reference로 명확히 했다.
- 구형 `D/N/R` 문구를 현재 단일 Gear Slot 계약으로 교정했다.
- 초기 정적 RPM Track/P2 설명을 Historical로 한정하고 현재 단일 RPM Material 구조를 명시했다.

### v0.4.0 - 2026-08-23

- `CF-FQ-032` 완료 기반을 보존한 후속 `CF-FQ-039 Production UI Visual Rework`의 Supporting Art Spec으로 owner 관계를 갱신했다.
- 단순 Reference와 USER 승인 Visual Target을 분리하고, Target Registry 등록 후 Frame·Silhouette·비율·Hierarchy·Material 인상을 임의 단순화하지 않는 규칙을 추가했다.
- Target Deviation은 구현 전에 USER 승인을 요구하고 Functional/Technical PASS가 Visual Match PASS를 대신하지 않도록 고정했다.
- 과거 대화에만 존재하는 승인 이미지는 실제 원본 회수 전 가짜 저장소 경로를 만들지 않으며 `VPR-P0-00`에서 회수·식별하도록 연결했다.

Migration: 기존 D1-11-ART Technical PASS와 Designer Layout Ownership evidence는 유지한다. `CF-FQ-039`에서는 이 문서의 제작 규격을 재사용하되 새 작업 상태와 Target Registry는 `InGameUIVisualPlan.md`가 소유한다.

### v0.3.8 - 2026-08-20

- `UI-P0-08B Production RadarPanel Consumer`의 Radar 전용 Visual Technical Apply를 반영했다. `GenerateUIHUDArtP2.ps1 v1.2.0 -RadarOnly`가 Text glyph 없이 `T_UI_RadarFrame`, Friendly/Hostile/Unknown Blip, Player Marker, 4-Corner Target Bracket, 2-Corner Edge Bracket 7종을 생성했고 process `a08892dd13a648cca7c1a92f8ab23527`에서 7/7 Technical PASS했다.
- `DA_CFHUDVisual_Default`에 `RadarFrame / RadarFriendlyBlip / RadarHostileBlip / RadarUnknownBlip / RadarPlayerMarker / SelectedTargetBracket / RadarSelectedEdgeBracket`가 persisted 연결됐다. Neutral은 Unknown Diamond Texture를 재사용하고 Style Token 색으로 구분한다.
- `WBP_CFRadarPanel`은 layout-preserving targeted apply `a059a57d25114561934f824a7949186d`에서 기존 Contact/Selected Slot을 재배치하지 않고 `Image_RadarFrame / Image_RadarPlayer / Image_RadarSelectedEdge / Text_RadarRange`만 additive 추가했다. fresh AssetDump에서 17 Widget 저장 상태와 VisualData 참조를 확인했다.
- `RadarSweepMaterial`은 여전히 `None`이며 Active Scan Sweep 후속 표현으로 분리한다. 이번 Technical Apply와 focused 2/2 PASS를 USER Visual 승인으로 확대하지 않으며 VehiclePanel의 Deferred Visual Review 상태도 변경하지 않는다.

Migration: Radar Contact를 TextBlock 문자기호로 되돌리지 않는다. 전용 `UImage` Brush Template와 Designer-owned size/layout을 유지하며 실제 다음 사용자-facing 검증은 Radar Zoom Input과 Production Radar 가독성·선택 edge·조작감이다.

### v0.3.7 - 2026-08-20

- 상위 Runtime current projection만 동기화했다. UI-P0-07 Target Knowledge는 Technical Complete이며 현재 Runtime Gate는 UI-P0-08 Radar·화면 밖 마커다.
- 이 변경은 D1-11-ART의 시각 판정을 바꾸지 않는다. ArmorBodyMap Designer Editability만 USER PASS이고 SpeedGauge·VehiclePanel·전체 Visual Review는 계속 Deferred / USER Visual PASS 미부여다.
- Radar 시각 자산의 실제 변경은 UI-P0-08 Foundation Audit에서 persisted RadarPanel 구조와 표시 계약을 확정한 뒤 별도 최소 범위로 수행한다.

Migration: UI-P0-07 closure를 VehiclePanel Visual PASS로 해석하지 않는다. 기존 Designer Layout Ownership과 P2 technical evidence는 보존한다.

### v0.3.6 - 2026-08-20

- USER가 `WBP_CFArmorBodyMap`의 위치·크기 Designer 편집성이 충분함을 직접 확인했다. 이 항목만 USER PASS이며 Designer Layout Ownership Technical PASS를 다시 열지 않는다.
- 이어진 `WBP_CFSpeedGauge`·`WBP_CFVehiclePanel` 개별 편집성 및 전체 VehiclePanel Visual Review는 USER가 건너뛰고 Runtime 작업으로 진행하기로 결정했다. 따라서 Visual Review 상태는 **Deferred / USER Visual PASS 미부여**로 보존한다.
- 이 Deferred는 2026-08-20 USER 결정에 따라 UI-P0-07 진행을 차단하지 않지만, 향후 시각 결함이 확인되면 기존 persisted Layout을 보호한 채 해당 Widget/Slot 또는 P2 Art만 최소 교정한다.

Migration: 다음 Runtime 세션은 Visual PASS를 추정하지 않고 UI-P0-07을 진행한다. Visual Review 재개 시 completed Designer Ownership remediation과 P2 technical apply를 반복하지 않는다.

### v0.3.5 - 2026-08-20

- `Designer Layout Ownership Remediation`을 Technical PASS로 닫았다. `CFUIHUDProdEditorBridge.h v1.9.0 / .cpp v1.12.0`은 기존 RootWidget이 있는 Production Widget/Root의 Build를 fail-closed하고 신규/빈 Asset에서만 최초 Scaffold를 허용한다. 의미 구조는 C++/Validator가 보호하고 Position·Size·Anchor·Alignment·Padding·AutoSize는 persisted UMG Designer Asset이 소유한다.
- `ApplyUIHUDProduction.py v1.5.0 / RunUIHUDProduction.ps1 v1.4.0`은 기존 Widget을 Validate-only로 처리한다. Official Build `9125557b4e214a4ebe2bd642099929aa` PASS, saved readback `b543596ea3404a8084b46e5349ce7e84` PASS / saved0이다.
- 실제 `ArmorMapOnly` process `ad6e8d64733249b0be26d8ab9506acca`에서 기존 ArmorSector/ArmorBodyMap 2개가 layout-preserved로 판정됐고 rebuilt0 / compiled0 / saved0 / exact mutated0으로 종료됐다. USER가 저장한 ArmorBodyMap 배치는 targeted validation 과정에서 덮어쓰지 않았다.
- 현재 RPM 계약에 맞춰 `SpeedArcTrack`은 `M_UI_RPMGauge` 데이터 Texture의 간접 입력, `SpeedArcMaterial`은 `Image_RPMGauge`가 실제 소비하는 Material로 Current 표를 교정했다. 기존 Asset Art 갱신 Workflow도 full scaffold rebuild가 아니라 layout-preserving targeted visual update / Designer 변경 + Validate를 사용하도록 갱신했다.
- 다음 USER Gate는 SpeedGauge / ArmorBodyMap / VehiclePanel의 Designer 편집성과 전체 Balanced Combat Visual Review다. UI-P0-07은 계속 Not Started다.

Migration: 저장된 Production Widget의 Art 또는 Layout을 갱신하기 위해 `BuildProductionWidgetResult`/`BuildProductionRootResult`를 재실행하지 않는다. 신규/빈 Asset에만 Scaffold를 사용하고 기존 Asset은 Validate-only + 필요한 경우 별도 layout-preserving visual updater를 사용한다.

### v0.3.4 - 2026-08-20

- Armor 방향 UI를 재사용 `WBP_CFArmorSector`로 묶어 `Image_ArmorPlate + Text_Direction + ProgressBar_Armor`를 한 의미 Element로 정리했다. BodyMap은 차량 실루엣 + Sector 6개의 공간 관계만 소유하도록 단순화했다.
- SpeedGauge는 구형 `Image_RPMTrackArt + ProgressBar_RPMTick00~20`을 폐기하고 `Image_RPMGauge` 한 장이 `M_UI_RPMGauge + T_UI_RPMTrack`을 사용하며 Presenter의 단일 `RPMRatio`를 소비하는 구조로 전환했다. USER Designer에서 비활성 Track/Tick 실제 표시를 확인했다.
- USER가 ArmorBodyMap을 직접 재배치하는 과정에서 C++ `CanvasPanelSlot` 위치/크기와 nested SizeBox override가 Designer 편집을 과도하게 제한하는 문제가 확인됐다. 이에 `Designer Layout Ownership`을 명시적으로 추가했다: C++는 의미 구조/데이터 계약, UMG Designer는 위치·크기·Anchor·Alignment·Padding·AutoSize를 소유한다.
- 기존 user-edited Designer Layout을 자동 rebuild에서 덮어쓰거나 C++ 좌표로 역복사하지 않는다. 신규 Asset 최초 Scaffold와 기존 Asset 유지·검증 경로를 분리하는 것이 다음 구현 Gate다.

Migration: 다음 세션은 art/meaning 구조를 다시 설계하지 않고 Designer Layout Ownership 교정부터 진행한다. 현재 USER 조정 중인 ArmorBodyMap Asset은 임의 save/discard/rebuild하지 않으며 UI-P0-07은 계속 Not Started다.

### v0.3.3 - 2026-08-19

- P2 Hardening에서 UE 5.8 UI Texture import 계약을 실제 commandlet 저장 경로에 강제했다. `CFHUDArtP2Cmdlet v1.1.0`은 `TEXTUREGROUP_UI`, `TC_EditorIcon(UserInterface2D RGBA)`, `TMGS_NoMipmaps`, `sRGB=true`를 설정하고 exact 검사 실패 시 package save 전에 fail-closed한다.
- P2 Frame 512×256의 설계 내부 경계 48px/24px가 양 축 모두 `0.09375`이므로 `CFUIHUDProdEditorBridge v1.9.1`의 Box Brush Margin을 기존 0.18에서 0.09375로 교정했다. Source geometry와 9-Slice 소비 계약을 일치시킨 기술 교정이며 최종 미감은 USER Visual Review가 계속 소유한다.
- `GenerateUIHUDArtP2.ps1 v1.0.1`은 `p2_review.json`을 `SourceTechnicalReviewOnly`로 명시해 Unreal Import/DA Apply 현재 상태와 분리했다. P2 9/9 Source 재검증, Official Build `c810db5d48af4039b1a500b2629a475c` PASS, corrected reapply `fece00c20b08429988f7489e528949de` Exit 0 PASS, fresh persisted AssetDump PASS를 확보했다.
- 이 Hardening은 VehiclePanel 정보 구조·Armor 배치·Gameplay/ViewData/Presenter 계약을 변경하지 않는다. USER Visual Review는 Pending이고 UI-P0-07은 Not Started다.

Migration: 기존 persisted P2는 corrected commandlet로 한 번 교정 완료했으므로 새 실패 근거 없이 reimport/rebuild하지 않는다. Source review JSON은 Unreal Apply truth owner로 사용하지 않는다.

### v0.3.2 - 2026-08-19

- P2 VehiclePanel Production Art Technical Apply를 완료했다. `T_UI_VehPanelFrame`, `T_UI_VehSil_LF`, Armor 6종, `T_UI_RPMTrack` 9종을 `/Game/CarFight/UI/HUD/Visual`에 Texture2D로 저장하고 `DA_CFHUDVisual_Default` Vehicle 9슬롯에 연결했다.
- `WBP_CFSpeedGauge.Image_RPMTrackArt`, `WBP_CFArmorBodyMap`의 중앙 좌향 Vehicle + 6방향 Badge/Label/세로 Bar, `WBP_CFVehiclePanel.Border_Surface` Frame과 두 Element 조립을 persisted readback으로 확인했다. `VehiclePanelFrame` 소비 계약을 DA 표에 추가하고 stale `Image_SpeedArcArt` 표기를 actual `Image_RPMTrackArt`로 교정했다.
- `CFHUDArtP2Cmdlet v1.0.0` official build는 PASS했다. 첫 wrapper는 PowerShell 5.1 GUI process `$LASTEXITCODE` 문제로 wrapper만 Exit1이었으나 마지막 저장 Asset까지 persisted해 실제 Apply 완료를 확인했고 commandlet을 재실행하지 않았다. `RunHUDArtP2.ps1`은 v1.0.1로 교정했지만 post-fix 실행은 Not Run이다.
- USER가 PC를 사용할 수 없어 USER Visual Review는 Pending이다. Technical PASS를 시각 승인으로 확대하지 않고, 다음 USER 검토에서 실제 프레임·RPM·Armor 가독성과 Balanced Combat 인상을 판정한다.

Migration: P2 Technical Apply를 새 실패 근거 없이 반복하지 않는다. P1은 계속 Technical reference baseline이고, 후속 시각 결함은 P2 Art/배치 범위에서 최소 교정한다. USER Visual PASS 뒤 UI-P0-07을 재개한다.

### v0.3.1 - 2026-08-19

- 사용자 승인으로 Visual Direction을 `Balanced Combat`으로 고정했다. Cyan system accent + Amber armor/caution accent + dark layered frame + restrained glow를 기본 시각 언어로 사용한다.
- 분위기 Reference의 원형 Speed/RPM 계기판과 임의 배치는 채택하지 않고 `InGameUIVehiclePanelSpec v0.20.0`의 실제 896×416 구조를 절대 우선하도록 명시했다.
- 기존 P1 9종은 Technical reference baseline으로만 보존하고 Production Art 승인으로 확대하지 않는다. 현재 다음 Gate를 `Actual VehiclePanel Concept USER Review`로 교정했다.
- Actual Concept 승인 전 Unreal Import·DA 연결·Production Apply는 계속 금지한다.

### v0.3.0 - 2026-08-19

- 사용자 결정에 따라 UI-P0-07 전에 `D1-11-ART Visual Foundation / VehiclePanel Vertical Slice`를 Current Gate로 재개했다.
- 기존 `GenerateUIHUDArtP1.ps1 v1.0.1`의 저장소 직접 생성 경로를 Art Ingress PASS로 확정하고 별도 이미지 전달 기능을 선행조건으로 두지 않는다.
- P1 9종 Technical PASS를 재사용하고 첫 Vertical Slice를 VehiclePanel이 이미 소비하는 8종으로 좁혔다. `T_UI_TargetBracket`은 후속 Target/Radar Visual 단계용으로 보존한다.
- USER Visual 승인 전 Unreal Import·DA 연결·Production Apply를 금지하고, 승인 후 `Import → DA → targeted Apply/Readback → persisted evidence → 1920×1080 USER Preview` 순서를 고정했다.

### v0.2.0 - 2026-08-11

- `Tools/GenerateUIHUDArtP1.ps1 v1.0.1`로 P1 Texture 9종의 승인 전 Source PNG를 `SourceArt/UI/HUD/P1`에 생성했다.
- Vehicle Silhouette 1, 방향별 Armor Plate 6, Speed Arc Track 1, Selected Target Bracket 1을 파일별 기대 해상도와 투명 배경 계약으로 검증해 9/9 Technical PASS를 확인했다.
- 반투명 Alpha Edge의 검은 RGB 픽셀을 별도 검사했으며 9종 모두 Black Alpha Edge Risk 0이다.
- 사용자 검토용 `Review/P1_HUD_Review.png`와 기계 판독용 `Review/art01_review.json`을 생성했다.
- 기술 검토와 사용자 아트 승인을 분리했다. 현재 ApprovedAssets는 0개이며 Unreal Import·DA 연결·Production Apply는 수행하지 않았다.
- D1-11-ART-02를 `BLOCKED — APPROVED ASSET 0 / USER APPROVAL REQUIRED`로 명시해 승인되지 않은 자산이 Import 단계로 넘어가지 않도록 했다.

### v0.1.0 - 2026-08-11

- D1-11 Production HUD의 전용 Texture/UI Material 제작 규격을 신규 문서화했다.
- Border Mosaic 재발 방지, Text/Image/Material/ProgressBar 책임 분리와 Semantic Icon 대 HUD 전용 Art 경계를 고정했다.
- `DA_CFHUDVisual_Default`의 13개 현재 필드와 Production Bridge 실제 소비 여부를 구분해 기록했다.
- 즉시 화면에 반영 가능한 P1 Texture 9종과 권장 파일명·원본 크기·포맷·스타일·Import 규격을 고정했다.
- `SpeedArcMaterial`, `RadarFrame`, `RadarSweepMaterial`, `SelectedWeaponFrame`은 현재 Widget 소비 경로가 아직 없으므로 P2 Hookup 선행 대상으로 분리했다.
- 현재 D1-11 Prototype은 DA 변경 후 `RunUIHUDProduction.ps1 -Apply`를 다시 실행해야 저장된 Designer Brush에 반영된다는 실제 Assetization Workflow를 명시했다.
- 2026-08-11 대화에서 생성된 전체 HUD 이미지는 Working Visual Direction Reference A로만 기록하고 아직 Repository Asset 또는 USER PASS 기준으로 승격하지 않았다.
