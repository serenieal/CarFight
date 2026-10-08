# CarFight InGame UI Style Specification

- 문서 버전: v0.86.1
- 작성일: 2026-08-06
- 최근 갱신일: 2026-08-23
- 문서 상태: Supporting Style Contract / CF-FQ-032 Historical implementation checkpoints retained
- 기능 ID: `CF-FQ-032`
- Current Applicability: `CF-FQ-039`는 이 문서의 Color·Typography·Spacing·Shape·Motion 계약을 재사용한다. 과거 D1/UI-P0 상태는 Historical이며 현재 구현은 `Systems/UI/InGameUI.md`, 현재 작업은 `InGameUIVisualPlan.md`가 우선한다.
- 대표 Plan: `InGameUIVisualPlan.md v0.1.2`
- 시각 콘셉트: `InGameUIVisualConcept.md v0.8.1`
- 구조 설계: `InGameUIDesign.md v0.35.1`
- 자산화 실행 규격: `InGameUIAssetizationSpec.md v0.16.1`
- 로드맵: `InGameUIVisualRoadmap.md v0.1.2`
- 1440p Wireframe: `ConceptArt/CFHUDWireframe_1440p.xml`
- Vehicle Panel 상세: `InGameUIVehiclePanelSpec.md`
- Vehicle Panel Wireframe: `ConceptArt/CFVehiclePanel_1440p.xml`
- Weapon Panel 상세: `InGameUIWeaponPanelSpec.md`
- Weapon Panel Wireframe: `ConceptArt/CFWeaponPanel_1440p.xml`

---

## 1. 목적

이 문서는 사용자 승인된 `차량 탑재형 전술 인터페이스`와 C형 70:20:10 디자인 균형을 실제 Unreal UI 자산으로 옮기기 위한 상세 제작 규격을 정의한다.

```text
70% 실용적 전투 정보
20% 차량 디지털 계기판 정체성
10% 세계관 장식과 스타일
```

이번 문서가 구체화하는 범위:

- `DA_CFUIStyle` 상세 Token
- C++ Style Data 타입의 권장 구조
- 공통 Blueprint Base Widget 규격
- 1920×1080 / 16:9 Phase 1 HUD 배치·가독성 기준
- 2560×1440·21:9·32:9 후속 Layout Profile 확장 규칙
- Clean Prototype 검토 기준

이 문서는 디자인 SSOT이지만 일부 자산화 단계는 실제 구현 상태까지 함께 추적한다. HUD 기본 배치, VehiclePanel, WeaponPanel Compact Direction, 전체 UI 커스터마이징 계약, D1-05 Style·Base Widget·Density·Weapon Compact Token, D1-06 Font·Icon 기본 Preset과 D1-07 Phase 1 `SR-1080-16` Static Review는 사용자 승인됐다. Pretendard·IBM Plex Mono Font, Semantic Icon, Style·Density·1080 Layout Asset과 OFL Notice·Config는 D1-09B에서 PASS했다. D1-10A C++ Visual Context/Button Bridge도 Build·Contract PASS지만 D1-10B Base Widget Asset은 열린 Editor의 DLL 점유로 아직 생성하지 못했으므로 `UI-DESIGN-GATE PASS`로 해석하지 않는다.

이 문서의 Token·Base Widget·Layout Profile·Density Profile 규칙은 인게임 HUD에만 한정되지 않는다. Pause, Target, Radar, Alert, Garage, Fitting, Inventory, Mission, Result, Loading과 Tutorial을 포함한 CarFight 전체 UI가 같은 커스터마이징 기반을 공유한다.

---

## 2. 스타일 데이터 소유권

### 2.1 권장 C++ 타입

향후 구현 시 다음 타입을 권장한다.

```text
UCFUIStyleData : UPrimaryDataAsset

FCFUIColorTokens
FCFUITypographyTokens
FCFUISpacingTokens
FCFUIShapeTokens
FCFUIOpacityTokens
FCFUIMotionTokens
FCFUIButtonStyle
FCFUIPanelStyle
FCFUIStatusBarStyle
FCFUIInfoRowStyle
FCFUIAlertStyle
FCFUIIconSet
```

역할:

```text
C++ 타입
→ Token 이름, 타입 안정성, 기본값, 검증과 Blueprint 노출 계약

DA_CFUIStyle
→ CarFight 프로젝트에서 사용하는 실제 Token 값

Blueprint Base Widget
→ DA_CFUIStyle 값을 레이아웃·브러시·애니메이션에 적용
```

### 2.2 Style Data 공급 경로

- `UCFUISubsystem`은 기본 `DA_CFUIStyle` Soft Reference를 한 번만 해석한다.
- `WBP_CFUIRoot` 또는 C++ Root Adapter가 자식 Widget에 Style Data를 전달한다.
- 개별 Widget은 에셋 경로를 직접 하드코딩하거나 매번 Load하지 않는다.
- Widget 생성 후 `ApplyUIStyle`을 한 번 호출하고, 런타임 Theme 변경이 없는 동안 같은 값을 재적용하지 않는다.
- View Data와 Style Data를 같은 구조체에 넣지 않는다.

### 2.3 Blueprint 노출 원칙

- `DA_CFUIStyle` 값은 Blueprint에서 읽기 전용으로 소비한다.
- 개별 Widget에서 색상·폰트·여백을 직접 Override할 때는 명시적인 `bOverride...` 변수를 사용한다.
- Override의 기본값은 모두 `false`다.
- 상태 의미는 View Data가 소유하고, 어떤 색·형태로 표현할지는 Style Data가 소유한다.
- 기본 Widget은 최종 고정 외형이 아니라 복제·상속·교체 가능한 Preset으로 제공한다.
- 주요 기능 Widget Class는 UI 설정 또는 Layout Profile에서 교체할 수 있어야 한다.
- 사용자 Visual Widget이 Presenter의 공개 입력 계약만 지키면 C++ Gameplay 코드 수정 없이 대체될 수 있어야 한다.

### 2.4 전체 UI 설정 데이터 분리

권장 데이터 자산:

```text
DA_CFUIStyle_Default
DA_CFUILayout_16x9
DA_CFUILayout_21x9
DA_CFUILayout_32x9
DA_CFUIDensity_Compact
DA_CFUIDensity_Standard
DA_CFUIDensity_Expanded
```

소유권:

```text
Style Data
→ 색상·폰트·브러시·Shape·Opacity·Motion

Layout Data
→ PanelId·Anchor·Alignment·Offset·DesiredSize·RenderScale·ZOrder·기본 Visibility

Density Data
→ Padding·Spacing·Header Height·Info Row Height·Tile Size·Caption 노출 수준

User Override
→ 위치·Scale·Opacity·Visibility·Density·Theme의 사용자별 덮어쓰기
```

Style, Layout, Density와 View Data를 하나의 거대한 구조체나 Data Asset에 합치지 않는다.

### 2.5 설정 해석 순서와 안전 하한

UI가 최종 표시값을 결정할 때 우선순위는 다음으로 고정한다.

```text
1. 접근성·안전 하한
   - 최소 읽기 크기
   - 최소 상호작용 영역
   - Viewport 경계
   - Reticle Protection 금지 영역

2. User Override
   - 위치·Scale·Opacity·Visibility·Density·Theme

3. Panel / Widget 명시 Override
   - bOverride... 가 true인 값만

4. Density Profile
   - Padding·Gap·행 높이·Icon·Caption 밀도

5. Layout Profile
   - Anchor·Alignment·Offset·DesiredSize·ZOrder

6. Style Profile
   - Color·Typography·Shape·Opacity·Motion

7. Native Safe Fallback
   - Data Asset 누락 시 깨진 화면 대신 최소 가독 기본값
```

`User Override`도 최소 글자 크기, 최소 클릭 영역, 화면 밖 완전 이탈과 중앙 조준 보호영역 침범 같은 안전 하한을 무시하지 않는다.

### 2.6 D1-05 승인 성숙도

```text
Accepted / Default Preset
- 전체 UI 커스터마이징 소유권 구조
- Style / Layout / Density / User Override 분리
- 자동 Layout과 Collapsed 원칙
- Color·Typography·Spacing·Shape·Opacity·Motion 기본값
- Base Widget 입력·상태·스타일 적용 계약
- Compact / Standard / Expanded Density 기본값
- WeaponPanel Compact Token 기본값
- SR-1080-16 Phase 1 검토 기준 / SR-1440-16·SR-1440-21·SR-1440-32 Deferred Expansion 기준

Pending
- D1-09B 1080p Active Profile Data Asset와 실제 Font·Icon Asset 생성
- D1-09B Config Soft Reference 연결과 Asset/Reference Readback
- D1-10 Blueprint Base Widget 생성
- D1-11 `WBP_CFInGameHUD` Visual Prototype
- D1-12 Pause Menu Visual Migration
- 실제 Font Asset 생성 시 원본 Copyright Notice + OFL 전문 프로젝트 내 보존
- Runtime TPS Capture 기반 후속 검증

Completed
- D1-07 Phase 1 `SR-1080-16` Static Review USER PASS
- D1-08V Official Editor Build + `StyleDataContract` PASS
- D1-09A Font Binding·HUD Layout·Density C++ + Subsystem Config/Fallback PASS
- Font License Eligibility: Pretendard / IBM Plex Mono OFL 1.1 PASS
```

v0.9.0의 승인 수치는 **기본 Preset**이지 Widget 내부 고정값이 아니다. 구현 시 Style·Layout·Density Data Asset에서 수정 가능해야 하며, 개별 Widget은 명시적 Override가 필요한 경우를 제외하고 이 값을 직접 Literal로 복제하지 않는다.

---

## 3. Color Tokens

색상은 sRGB Hex와 별도 Alpha로 정의한다. Unreal에서는 `FLinearColor`로 저장하되 문서와 리뷰에서는 Hex를 기준으로 비교한다.

### 3.1 Surface와 Line

| Token | Hex | Alpha | 용도 |
|---|---:|---:|---|
| `SurfaceBase` | `#0B1117` | 0.80 | 일반 HUD 패널 배경 |
| `SurfaceRaised` | `#111B24` | 0.88 | 선택 패널, 헤더와 상위 표면 |
| `SurfaceOverlay` | `#17232D` | 0.94 | Pause·Modal과 강한 분리 표면 |
| `SurfaceSoft` | `#111B24` | 0.56 | 가벼운 그룹 배경과 Radar 내부 |
| `DimOverlay` | `#02070B` | 0.72 | Pause 배경 어둡게 처리 |
| `LineSubtle` | `#324653` | 0.65 | 보조 구분선과 내부 Grid |
| `LineDefault` | `#577080` | 0.82 | 일반 패널 외곽선 |
| `LineStrong` | `#7F98A8` | 0.92 | 선택 전 기본 강조와 핵심 경계 |

### 3.2 Text

| Token | Hex | Alpha | 용도 |
|---|---:|---:|---|
| `TextPrimary` | `#EAF2F7` | 1.00 | 핵심 수치와 주요 문구 |
| `TextSecondary` | `#A3B2BC` | 1.00 | 레이블, 단위와 보조 정보 |
| `TextMuted` | `#7A8A95` | 1.00 | 중요도가 낮은 상태와 설명 |
| `TextDisabled` | `#61717C` | 0.88 | 비활성·Unavailable |
| `TextOnAccent` | `#071015` | 1.00 | 밝은 Accent 면 위 텍스트 |

### 3.3 Accent와 상태

| Token | Hex | Alpha | 의미 |
|---|---:|---:|---|
| `AccentTactical` | `#55C7E8` | 1.00 | 선택, 조준, 시스템 기본 강조 |
| `AccentHover` | `#75D5EF` | 1.00 | Hover와 밝은 강조 |
| `AccentPressed` | `#36A8C9` | 1.00 | Pressed와 확정 입력 반응 |
| `StateNotice` | `#72C7D9` | 1.00 | 정보 공개와 일반 알림 |
| `StateCaution` | `#F1B84B` | 1.00 | 진행, 불안정, 주의 |
| `StateDanger` | `#FF714D` | 1.00 | 손상, 실패 위험, 즉각 대응 |
| `StateCritical` | `#FF3F46` | 1.00 | 치명 상태와 긴급 경고 |
| `StateDisabled` | `#65727B` | 0.90 | 비활성, 파괴, 사용 불가 |
| `StateSuccess` | `#6FC7A2` | 1.00 | 명시적인 완료·복구 결과에만 제한 사용 |

`StateSuccess`는 일반 정상 상태의 기본색이 아니다. 모든 정상 정보를 초록색으로 표시하지 않는다.

### 3.4 방어 계층

| Token | Hex | 표현 형태 |
|---|---:|---|
| `ShieldColor` | `#59D5E6` | 얇은 연속 Fill + 에너지층 아이콘 |
| `ArmorColor` | `#D6A448` | 분절 Fill + 판재 아이콘 |
| `IntegrityColor` | `#F0644F` | 굵은 연속 Fill + 차체 아이콘 |

### 3.5 관계와 지식 상태

| Token | Hex | 용도 |
|---|---:|---|
| `FriendlyColor` | `#53A9FF` | 아군 Contact와 Marker |
| `NeutralColor` | `#D5C777` | 중립 Contact와 Marker |
| `HostileColor` | `#FF644F` | 적대 Contact와 Marker |
| `UnknownColor` | `#A7ADB3` | 미식별 Contact와 `???` |
| `EstimatedColor` | `#B5C2C9` | 추정 정보와 불확실 값 |

선택 상태는 관계색을 바꾸지 않는다. 선택된 대상은 관계색을 유지하고 `AccentTactical` 외곽선 또는 Corner Marker를 추가한다.

### 3.6 대비와 접근성 규칙

- 핵심 텍스트는 패널 배경 대비 4.5:1 이상을 목표로 한다.
- 28pt 이상 대형 수치는 3:1 이상을 최소 기준으로 한다.
- 색상만으로 상태를 표현하지 않고 아이콘·텍스트·형태 중 하나 이상을 함께 사용한다.
- Critical 상태는 Red 색상과 함께 경고 아이콘, 두꺼운 외곽선 또는 반복 Pulse를 사용한다.
- Unknown과 Disabled는 모두 회색 계열이지만 `???`와 `N/A`, 아이콘과 명도를 구분한다.

---

## 4. Typography Tokens

모든 크기는 2560×1440 기준 Slate Design Unit 초안이다. DPI Scale 후 실제 화면 가독성을 검증한다.

| Token | Size | Weight | Line Height | 용도 |
|---|---:|---|---:|---|
| `DisplayXL` | 52 | Bold | 56 | 속도 숫자 |
| `DisplayL` | 44 | Bold | 48 | 가장 중요한 방어·탄약 수치 |
| `ValueM` | 32 | SemiBold | 36 | 거리, 주 자원과 보조 수치 |
| `ValueS` | 20 | SemiBold | 24 | 작은 자원 수치, Armor 숫자와 보조 상태 값 |
| `HeadingL` | 28 | SemiBold | 34 | Pause 제목과 주요 패널 제목 |
| `HeadingM` | 24 | SemiBold | 30 | HUD 패널 헤더 |
| `Body` | 20 | Regular | 26 | 메뉴 문구와 상태 설명 |
| `Label` | 16 | Medium | 22 | HUD 레이블과 단위 |
| `Caption` | 14 | Regular | 20 | 입력 힌트와 보조 설명 |

### 4.1 숫자 규칙

- 수치 전용 Font는 Tabular Number를 지원해야 한다.
- 속도는 최소 세 자리 폭을 예약한다.
- 거리와 탄약 값의 자리수 변화로 패널이 흔들리지 않게 한다.
- 단위는 본 수치보다 한 단계 작은 Token을 사용한다.
- 불필요한 소수점을 제거한다.

### 4.2 Text Role 규칙

```text
DisplayXL / DisplayL
→ 숫자 중심, 한 화면에서 3개 이하

ValueM / ValueS
→ 일반 수치와 소형 보조 수치

HeadingL / HeadingM
→ 패널 제목, 전부 대문자 영문 사용 금지

Body
→ 메뉴와 짧은 상태 설명

Label / Caption
→ HUD 레이블, 단위와 입력 힌트
```

### 4.3 Font Family 승인 기본값

D1-06 사용자 승인 기본 Preset:

```text
UIFontFamily
→ Pretendard

NumericFontFamily
→ IBM Plex Mono

TechnicalFontFamily
→ 기본은 NumericFontFamily 재사용
→ 별도 기술용 폰트가 실제 필요할 때만 Override
```

적용 원칙:

- 한글·일반 영문·메뉴·상태 문구는 `UIFontFamily`를 사용한다.
- 속도·탄약·거리·시간·Armor·방어 수치처럼 자리수 안정성이 중요한 값은 `NumericFontFamily`를 사용한다.
- 숫자 Font는 Tabular 또는 Monospaced 숫자 폭을 유지한다.
- Regular·Medium·SemiBold·Bold Role은 Style Data가 소유하고 개별 Widget이 특정 Font 파일을 직접 지정하지 않는다.
- 14 Design Unit와 1080p 유효 하한에서 획이 뭉개지면 Font 자체 또는 Weight Mapping을 Style Data에서 교체·보정한다.
- 실제 Font 파일을 Unreal Asset으로 가져오기 전 배포 라이선스와 게임 패키지 내 재배포 조건을 원문 기준으로 최종 확인한다.

### 4.3A Font 교체 불변 조건

```text
Typography Role
→ UIFontFamily / NumericFontFamily
→ 실제 Font Face
```

- Widget은 `Pretendard-Regular` 같은 실제 파일명을 직접 소유하지 않는다.
- 기본 Font 교체를 위해 Gameplay C++ 또는 Presenter를 수정하면 설계 위반이다.
- Font Family 변경은 Style Data 또는 Theme Override만으로 전파돼야 한다.
- 사용자가 후속 Custom Theme에서 다른 한글·숫자 Font를 지정해도 Typography Role과 View Data 계약은 유지한다.

### 4.4 1080p 유효 글자 크기 하한

1920×1080에서 Geometry를 0.75로 줄이더라도 Typography를 기계적으로 0.75까지 줄이지 않는다. 다음 유효 화면 크기를 하한으로 사용한다.

| Role | 1440p Draft | 1080p 유효 하한 | 비고 |
|---|---:|---:|---|
| `DisplayXL` | 52 | 38 | 속도 등 최상위 수치 |
| `DisplayL` | 44 | 32 | 주 자원·핵심 방어값 |
| `ValueM` | 32 | 24 | 거리·보조 수치 |
| `ValueS` | 20 | 14 | 작은 자원·Armor·보조 상태 값 |
| `HeadingL` | 28 | 22 | Pause·주요 Screen 제목 |
| `HeadingM` | 24 | 18 | HUD Panel 제목 |
| `Body` | 20 | 16 | 메뉴·상태 설명 |
| `Label` | 16 | 14 | HUD Label·단위 |
| `Caption` | 14 | 12 | 입력 힌트·보조 설명 |

- 21:9·32:9가 1440 높이를 유지하면 Typography는 1440p 기본 크기를 유지한다.
- 한글 Font의 실제 x-height와 획 두께 때문에 숫자·영문과 체감 크기가 다르면 Font Asset 검토에서 ±1~2 Design Unit 보정을 허용한다.
- 글자 크기 하한을 지키기 위해 Panel을 조금 넓히는 것은 허용하지만 중앙 Reticle Protection 영역을 침범해서는 안 된다.

---

## 5. Spacing와 Size Tokens

### 5.1 Spacing Grid

| Token | Value |
|---|---:|
| `SpaceXS` | 4 |
| `SpaceS` | 8 |
| `SpaceM` | 12 |
| `SpaceL` | 16 |
| `SpaceXL` | 24 |
| `Space2XL` | 32 |
| `Space3XL` | 48 |
| `Space4XL` | 64 |
| `Space5XL` | 96 |

### 5.2 Layout Size

| Token | Value | 용도 |
|---|---:|---|
| `SafeMargin` | 64 | 1440p 중앙 캔버스 외곽 / Deferred Expansion 기준 |
| `HUDOuterMargin1080` | 32 Design Unit → 실효 24px | D1-07 Phase 1 고정 HUD 외곽 배치 Override |
| `PanelPadding` | 24 | 일반 HUD 패널 내부 |
| `PanelPaddingCompact` | 12 | Compact Density 기본값 / Data Asset에서 수정 가능 |
| `HeaderHeight` | 48 | HUD 패널 Header |
| `InfoRowHeight` | 36 | 일반 정보 행 |
| `InfoRowHeightLarge` | 44 | 중요 정보 행 |
| `MinimumHitSize` | 48 | 클릭·Focus 최소 크기 |
| `MenuButtonHeight` | 60 | 일반 메뉴 버튼 |
| `PrimaryButtonHeight` | 68 | Pause 주 버튼 |
| `PrimaryButtonWidth` | 360 | Pause 주 버튼 기본 폭 |

### 5.3 레이아웃 간격

- 같은 값 그룹 내부: 8
- 레이블과 값 사이: 12
- 정보 행 사이: 8
- 패널 내부 그룹 사이: 16
- 서로 다른 패널 사이: 24
- 외곽 Safe Zone과 패널 사이: 1440p 기본 64
- D1-07 `SR-1080-16` 고정 HUD 외곽: 32 Design Unit → 0.75 적용 후 실효 24px

### 5.4 Density Profile

```text
Compact
→ 핵심 수치·상태 유지 / Padding·Spacing·Caption·장식 축소

Standard
→ 기본 플레이 가독성과 정보량 균형

Expanded
→ 상세 정보·큰 수치·추가 설명 허용

Custom
→ 사용자 또는 기능별 Override 값 사용
```

전역 Density를 기본으로 사용하되 주요 Panel은 별도 Override를 허용한다. Density 변경은 Gameplay 의미나 View Data 필드를 바꾸지 않고 표현 밀도만 바꾼다.

지원되지 않는 정보나 사용자가 숨긴 정보는 `Hidden`이 아니라 레이아웃 공간도 제거하는 `Collapsed`를 기본으로 한다. 남은 형제 Widget은 자동 Layout을 통해 빈 공간을 채운다.

### 5.5 Density Token 승인 기본값

모든 값은 2560×1440 기준 User Accepted Default Preset이다. 구현 시 Density Data Asset이 소유하며 Widget 내부 Literal로 고정하지 않는다.

| Token | Compact | Standard | Expanded | 규칙 |
|---|---:|---:|---:|---|
| `PanelPadding` | 12 | 24 | 32 | 패널 외곽 내부 여백 |
| `PanelInnerGap` | 8 | 12 | 16 | 같은 정보 그룹 내부 간격 |
| `SectionGap` | 8 | 16 | 24 | 서로 다른 정보 그룹 사이 |
| `InfoRowGap` | 4 | 8 | 12 | 연속 정보 행 사이 |
| `HeaderHeight` | 36 | 48 | 56 | Header가 존재할 때만 |
| `InfoRowHeight` | 28 | 36 | 44 | 일반 정보 행 |
| `InfoRowHeightLarge` | 36 | 44 | 52 | 중요 정보 행 |
| `ChipHeight` | 22 | 28 | 32 | 상태·관계·추정 Chip |
| `IconSmall` | 16 | 20 | 24 | Caption·Tile 보조 아이콘 |
| `IconMedium` | 20 | 24 | 32 | 일반 상태 아이콘 |
| `IconLarge` | 28 | 32 | 40 | Panel 핵심 아이콘 |
| `PrimaryValueScale` | 0.86 | 1.00 | 1.12 | Typography Role에 곱하되 하한 준수 |
| `SecondaryTextScale` | 0.90 | 1.00 | 1.08 | Label·Caption에만 적용 |
| `OutlineScale` | 0.75 | 1.00 | 1.00 | 1px 아래로 내리지 않음 |

Caption 정책:

| Profile | Caption 표시 |
|---|---|
| `Compact` | 핵심 단위·행동 차단 이유만 유지, 설명성 Caption 기본 숨김 |
| `Standard` | 문맥상 필요한 Caption 표시 |
| `Expanded` | 설명 Caption과 보조 상태를 허용 |
| `Custom` | 사용자 Override 값을 사용하되 안전 하한 유지 |

불변값:

- `MinimumHitSize`는 Density에 따라 축소하지 않는다.
- Critical 상태 아이콘과 Fire State 같은 행동 차단 정보는 Compact에서도 제거하지 않는다.
- `Unknown`, `Unavailable`, `KnownZero`의 의미 차이는 모든 Density에서 유지한다.
- Density 변경으로 Gameplay 데이터 요구량을 늘리거나 줄이지 않는다. 같은 View Data에서 표시량만 달라진다.

### 5.6 Density 적용 범위

```text
전역 Density
→ Screen·HUD의 기본값

Panel Density Override
→ VehiclePanel / WeaponPanel / RadarPanel / TargetPanel 등

Widget Local Override
→ 예외가 필요한 요소만 bOverrideDensity=true
```

권장 기본값:

```text
전투 HUD 전체 = Standard
WeaponPanel = Compact
AlertFeed = Compact
Pause / Modal = Standard
Garage / Fitting / Inventory = Standard
튜토리얼·상세 설명 Screen = Expanded 허용
```

이 기본값은 D1-05 사용자 승인 완료다. 향후 실제 Static Review나 사용자 취향에 따라 Data Asset 값만 조정할 수 있어야 하며 View Data 계약이나 Blueprint 구조 변경을 요구해서는 안 된다.

---

## 6. Shape, Opacity와 Motion Tokens

### 6.1 Shape

| Token | Value | 용도 |
|---|---:|---|
| `RadiusSmall` | 4 | HUD 소형 패널과 Chip |
| `RadiusStandard` | 6 | 일반 패널과 버튼 |
| `RadiusLarge` | 8 | Pause·Modal |
| `ChamferStandard` | 12 | 주요 전술 패널 한두 모서리 |
| `OutlineHairline` | 1 | 내부 Grid와 장식선 |
| `OutlineStandard` | 2 | 일반 외곽선 |
| `OutlineFocused` | 3 | Focus·Selected |
| `OutlineCritical` | 4 | Critical 한정 |

### 6.2 Opacity

| Token | Value |
|---|---:|
| `OpacityHUDPanel` | 0.80 |
| `OpacityHUDSoft` | 0.56 |
| `OpacityOverlay` | 0.94 |
| `OpacityDim` | 0.72 |
| `OpacityDisabled` | 0.45 |
| `OpacityDecorative` | 0.60 |

### 6.3 Motion

| Token | Duration | Easing | 용도 |
|---|---:|---|---|
| `MotionHover` | 100 ms | EaseOutCubic | Hover·Focus 진입 |
| `MotionPressed` | 80 ms | EaseOutQuad | 버튼 Pressed 반응 |
| `MotionState` | 160 ms | EaseInOutQuad | 작은 상태 변경 |
| `MotionPanel` | 200 ms | EaseOutCubic | 패널 등장·퇴장 |
| `MotionGauge` | 180 ms | EaseOutQuad | 게이지 값 보간 |
| `MotionAlert` | 360 ms | EaseOutCubic | 경고 1회 강조 |
| `MotionCriticalPulse` | 700 ms | EaseInOutSine | Critical 반복 Pulse |

### 6.4 Motion 제한

- 일반 HUD 요소는 Idle 상태에서 계속 움직이지 않는다.
- Critical Pulse만 반복을 허용한다.
- 반복 빈도는 약 1.4Hz이며 2Hz를 넘지 않는다.
- 화면 전체 Flash와 Camera UI Shake는 기본 규격에 포함하지 않는다.
- Pause Menu 애니메이션은 Pause 중에도 UI 시간으로 진행 가능해야 한다.

---

## 7. Element Style Tokens

### 7.1 Panel Style

```text
Background = SurfaceBase / OpacityHUDPanel
Outline = LineDefault / OutlineStandard
HeaderBackground = SurfaceRaised
HeaderHeight = 48
Padding = 24
CornerRadius = 6
OptionalChamfer = 12
```

패널 변형:

| Variant | 차이 |
|---|---|
| `Standard` | 기본 HUD 패널 |
| `Compact` | Density Profile의 `PanelPadding=12`, `HeaderHeight=36` 사용 |
| `Selected` | AccentTactical Corner Marker와 3px 일부 외곽선 |
| `Critical` | StateCritical 4px Accent, 반복 Pulse 가능 |
| `Menu` | SurfaceOverlay, Radius 8, 더 높은 불투명도 |

### 7.2 Button Style

| State | 배경 | 외곽선 | 텍스트 | 추가 표현 |
|---|---|---|---|---|
| Normal | SurfaceRaised | LineDefault 2px | TextPrimary | 없음 |
| Hover | SurfaceRaised 밝기 +8% | AccentHover 2px | TextPrimary | 좌측 Accent Line |
| Focused | SurfaceRaised | AccentTactical 3px | TextPrimary | Focus Corner Marker |
| Pressed | AccentPressed 0.32 Alpha | AccentPressed 3px | TextPrimary | 0.98 Scale |
| Disabled | SurfaceBase | LineSubtle 1px | TextDisabled | Disabled Icon 또는 `사용 불가` |

Primary Button:

```text
Min Size = 360 × 68
Accent Fill = AccentTactical 0.18 Alpha
Focused = AccentTactical 3px + 좌측 Marker
```

### 7.3 Status Bar Style

| Variant | Compact | Standard | Expanded | Fill | 형태 |
|---|---:|---:|---:|---|---|
| `Shield` | 8 | 10 | 12 | ShieldColor | 연속 Fill + 얇은 상단 Highlight |
| `Armor` | 10 | 12 | 14 | ArmorColor | 10개 분절 Fill, 방향 데이터 의미 아님 |
| `Integrity` | 12 | 16 | 20 | IntegrityColor | 굵은 연속 Fill |
| `Resource` | 8 | 10 | 12 | 상태별 Token | 일반 자원 채널 |
| `Progress` | 6 | 8 | 10 | AccentTactical | Lock·Scan·Reload 진행 |

높이는 Density Data가 소유한다. Armor 분절은 시각적 재질 표현이며 현재 P0에서 6방향 장갑 배치를 직접 의미하지 않는다.

### 7.4 Info Row Style

| Token | Compact | Standard | Expanded |
|---|---:|---:|---:|
| `InfoRowHeight` | 28 | 36 | 44 |
| `InfoRowLabelWidth` | 96 | 120 | 144 |
| `InfoRowIconSize` | 20 | 24 | 32 |
| `InfoRowGap` | 8 | 12 | 16 |

```text
ValueAlignment = Right
UnknownText = ???
UnavailableText = N/A
```

Info Row 크기 역시 Density Data가 소유하며 Widget Blueprint에 96·120·144 같은 값을 직접 복제하지 않는다.

상태 표현:

- `Known`: TextPrimary
- `Estimated`: EstimatedColor + `추정` Chip
- `Unknown`: UnknownColor + `???`
- `Unavailable`: TextDisabled + `N/A`
- `Critical`: StateCritical + 경고 아이콘

### 7.5 Alert Style

| Severity | Height | Accent | 기본 지속 |
|---|---:|---|---:|
| Notice | 48 | StateNotice | 2.0초 |
| Warning | 56 | StateCaution | 3.0초 |
| Critical | 64 | StateCritical | 상태 해제까지 또는 명시 시간 |

- 동시에 최대 3개를 표시한다.
- 같은 `AlertKey`는 기존 항목을 갱신한다.
- Critical은 반복 Pulse를 사용할 수 있다.
- 장기 지속 상태는 Alert Feed에서 계속 쌓지 않고 관련 상태 패널에 남긴다.

### 7.6 Semantic Icon Set 승인 기본값

D1-06 기본 Icon Style:

```text
Style = Solid Core + Tactical Cut
Base Grid = 24 × 24
IconSmall = 16
IconMedium = 20
IconLarge = 28
WeaponCompact = 18
WeaponSelected = 20
```

Semantic ID 예시:

```text
Vehicle
Shield
Armor
Integrity
Engine
Steering
Turret
Ammo
Battery
Charge
Heat
Cooldown
Reload
Target
Lock
RadarContact
Warning
Critical
```

기본 형태:

```text
Shield = Energy Field / Hex·Arc
Armor = Chamfered Plate
Integrity = Vehicle Chassis
Weapon = Side/Profile Silhouette
SelectedTarget = 관계색 Marker + Cyan Outer Bracket
Lock = 2~4개의 독립 Arc/Corner Segment
Radar = 관계색 + 기하 Symbol
```

권장 데이터 흐름:

```text
Semantic Icon ID
→ FCFUIIconSet 또는 동등한 Icon Set Data
→ Brush / Vector / Texture
```

- Gameplay·Presenter·Widget은 실제 Texture/Vector 경로를 직접 소유하지 않는다.
- Icon Set 전체 교체는 Style/Icon Data 변경만으로 가능해야 한다.
- 개별 Widget의 예외 교체는 명시적 `bOverrideIcon` 또는 동등한 Override를 사용한다.
- 외부 Icon 패키지는 라이선스 확인 후에도 CarFight Semantic 형태·Grid·Stroke 규칙에 맞춰 재가공한다.
- D1-07에서 작은 크기 판독성 문제가 생기면 Semantic 의미를 유지한 채 Icon Set Data만 보정한다.

---

## 8. 공통 Base Widget 규격

### 8.1 WBP_CFButtonBase

#### 역할

메뉴와 Modal의 모든 클릭·Focus 요소가 사용하는 공통 버튼이다.

#### Widget Tree 권장

```text
SizeBox
└─ Button
   └─ Overlay
      ├─ BackgroundBorder
      ├─ FocusOutline
      ├─ HorizontalBox
      │  ├─ Icon
      │  ├─ LabelText
      │  ├─ Spacer
      │  └─ InputHint
      └─ StateMarker
```

#### 필수 변수

```text
ButtonText
ButtonIcon
InputHintText
ButtonVariant
bShowIcon
bShowInputHint
bOverrideMinimumSize
```

#### 필수 상태

```text
Normal
Hover
Focused
Pressed
Disabled
```

#### 규칙

- 최소 Hit Size는 48×48이다.
- 기본 높이는 60, Primary는 68이다.
- Focused와 Hover는 동일한 상태가 아니며 각각 구분한다.
- Focus 표시가 색상만 바뀌는 형태가 되지 않게 Corner Marker를 추가한다.
- Button이 비활성일 때 Hover·Pressed 애니메이션을 실행하지 않는다.
- 부모 메뉴는 첫 Primary Button에 명시적으로 Focus를 요청한다.

### 8.2 WBP_CFPanelBase

#### 역할

HUD 정보 그룹의 배경, Header, Padding, Outline과 선택 상태를 통일한다.

#### Widget Tree 권장

```text
Overlay
├─ BackgroundBorder
├─ OutlineBorder
├─ OptionalAccentMarker
└─ VerticalBox
   ├─ HeaderSlot
   └─ ContentSlot
```

#### 필수 변수

```text
PanelTitle
PanelVariant
bShowHeader
bShowAccentMarker
ContentPaddingOverride
```

#### 규칙

- Panel은 Gameplay 데이터를 직접 조회하지 않는다.
- Header가 없는 Panel도 기본 Padding 규칙을 유지한다.
- 선택 상태는 전체 Fill을 관계색으로 바꾸지 않고 Corner Marker와 일부 외곽선만 강조한다.
- 중앙 조준 보호영역에는 PanelBase를 배치하지 않는다.

### 8.3 WBP_CFStatusBar

#### 역할

Shield·Armor·Integrity와 무기 자원을 동일한 입력 계약으로 표현한다.

#### 필수 입력

```text
CurrentValue
MaximumValue
NormalizedValue
PrimaryText
SecondaryText
BarVariant
DisplayState
bShowNumericValue
bShowLabel
```

#### 규칙

- Widget은 `Current / Maximum`을 재계산하지 않고 Presenter가 제공한 NormalizedValue를 우선 사용한다.
- Maximum이 0이거나 Unavailable이면 Fill을 0으로 만들고 `N/A`를 표시한다.
- Known Zero는 정상적인 0 값으로 표시한다.
- 값 감소 애니메이션은 180ms를 기본으로 하되 Critical 진입은 즉시 Accent를 적용한다.
- Armor 분절 수는 Style Token이며 방향별 장갑 의미를 부여하지 않는다.

### 8.4 WBP_CFInfoRow

#### 역할

Target 정보, 무기 정보와 차량 상태의 레이블·값 행을 통일한다.

#### 필수 입력

```text
LabelText
ValueText
KnowledgeState
StateIcon
bShowStateIcon
bReserveValueWidth
```

#### 규칙

- 레이블 폭은 Density Profile의 `InfoRowLabelWidth`를 사용하며 Standard 기본값은 120이다.
- 값 공개 여부가 바뀌어도 행 위치와 전체 패널 높이가 변하지 않는다.
- `Unknown`, `Estimated`, `Known`, `Unavailable`을 별도 시각 상태로 표시한다.
- Actor 내부 이름과 Debug 문자열을 표시하지 않는다.

### 8.5 WBP_CFAlertItem

#### 역할

Notice·Warning·Critical 알림 한 건을 표시한다.

#### 필수 입력

```text
AlertKey
Severity
DisplayText
AlertIcon
Duration
bPersistent
```

#### 규칙

- AlertItem은 자체 중복 제거를 하지 않는다. Alert Feed 또는 Presenter가 `AlertKey`를 관리한다.
- Critical만 반복 Pulse를 허용한다.
- 제거 애니메이션 중 같은 Key가 갱신되면 제거를 취소하고 최신 상태를 표시한다.
- 동시에 3개를 초과하면 우선순위와 생성 시각 기준으로 표시 대상을 결정한다.

### 8.6 Base Widget 공통 입력 계약

모든 공통 Base Widget은 개별 Gameplay Component를 직접 찾지 않고 상위 Root 또는 구성 객체에서 다음 시각 컨텍스트를 전달받는 구조를 권장한다.

```text
StyleData
DensityPreset
PanelId 또는 WidgetRole
bOverrideDensity
bOverrideStyle
bAnimationsEnabled
```

향후 C++ 공통 타입 후보:

```text
ECFUIDensityPreset
FCFUIStyleContext
FCFUIDensityTokens
FCFUIResolvedVisualStyle
```

역할 배분:

```text
C++
→ enum·구조체·기본값 검증·Fallback·해석 순서

Blueprint Base Widget
→ 전달된 Resolved Style을 Brush·Padding·Font·Animation에 적용

기능별 Blueprint Widget
→ Base Widget을 조합하고 View Data를 표시
```

기능별 Widget이 `DA_CFUIStyle`을 매번 직접 Load하거나 동일 Token을 서로 다른 이름으로 복제하지 않는다.

### 8.7 Base Widget 공통 상태 적용 순서

```text
Construct / Initialize
→ Style Context 수신
→ Density 해석
→ Explicit Override 적용
→ ApplyResolvedStyle
→ View Data 적용
→ Visibility / Collapsed 결정
→ 필요 시 상태 Motion 재생
```

- Style 적용과 View Data 적용을 분리한다.
- Theme·Density가 바뀌지 않으면 매 Tick Style을 다시 계산하지 않는다.
- 동일 값이 들어오면 Text·Brush·Color를 불필요하게 다시 Set하지 않는다.
- `Hidden`은 공간 예약이 필요한 특별한 연출에서만 사용하고 일반 정보 부재는 `Collapsed`를 사용한다.
- `Collapsed` 상태의 Widget은 자신의 Animation을 중지하고 불필요한 Tick을 사용하지 않는다.

### 8.8 Base Widget별 Density 반응

| Widget | Compact | Standard | Expanded |
|---|---|---|---|
| `WBP_CFButtonBase` | 텍스트·아이콘 간격 축소, 최소 Hit Size 유지 | 기본 | 보조 설명·입력 힌트 여유 확대 |
| `WBP_CFPanelBase` | Padding·Header 축소, 장식 최소화 | 기본 | Header·Section 간격 확대 |
| `WBP_CFStatusBar` | Bar 높이 축소, 핵심 숫자 유지 | 기본 | Label·SecondaryText 동시 표시 허용 |
| `WBP_CFInfoRow` | Caption 우선 숨김, Label·Value 유지 | 기본 | Estimated·보조 설명 공간 확대 |
| `WBP_CFAlertItem` | 한 줄 우선, 최대 폭 제한 | 기본 | 2줄 설명은 전체 화면 UI에서만 허용 |

### 8.9 커스터마이징 유지 불변 조건

D1-05에서 승인한 모든 수치는 다음 조건을 만족해야 한다.

```text
Global Default
→ Style / Density / Layout Data Asset에서 변경

Panel Override
→ 해당 Panel 설정에서 명시적으로 변경

Widget Override
→ bOverride... 가 true인 예외만 변경

Blueprint Visual
→ 입력 계약을 지키는 다른 Visual Widget Class로 교체 가능
```

- 승인된 Hex, Padding, Header, Row, Icon, Bar, Motion과 WeaponPanel 크기는 **기본값**이며 하드코딩 불변값이 아니다.
- Data Asset 값 변경만으로 다수 Widget에 같은 수정이 전파돼야 한다.
- 기본값 변경을 위해 C++ 재컴파일이나 Gameplay 코드 수정이 필요하면 설계 위반이다.
- Widget Class 교체와 개별 Override는 Presenter·View Data·Gameplay 판정 계약을 바꾸지 않는다.
- Native C++ 기본값은 Data Asset 누락 시 안전 Fallback 용도로만 유지한다.

### 8.10 DisplayState와 Visibility 규칙

```text
NotApplicable / HiddenByUser
→ Collapsed

Unavailable
→ 공간 유지 가능 + N/A 또는 정보 없음

Unknown
→ 공간 유지 + ???

KnownZero
→ 공간 유지 + 0

Known
→ 정상 표시
```

`WBP_CFInfoRow`의 "레이아웃이 흔들리지 않는다" 규칙은 `Unknown ↔ Known ↔ Unavailable`처럼 같은 의미 행의 상태 변화에 적용한다. 해당 정보 자체가 존재하지 않는 `NotApplicable`은 행 전체를 Collapse해 빈 공간을 남기지 않는다.

---

## 9. 2560×1440 HUD Wireframe

Wireframe 파일:

```text
Document/Plan/ConceptArt/CFHUDWireframe_1440p.xml
```

### 9.1 승인된 화면 전제

```text
기본 화면비: 16:9
기준 해상도: 2560 × 1440
기본 카메라: 외부 3인칭 차량 TPS
운전석·FPS 시점: 기본 배치 기준 아님
```

HUD는 플레이 차량과 주변 전장을 충분히 볼 수 있도록 중앙을 비운다. 이전 초안의 좌하단 Radar와 중앙 하단 독립 Speed Cluster 배치는 폐기한다.

### 9.2 승인된 기본 배치

모든 좌표는 2560×1440 기준 왼쪽 위 좌표이며 실제 UMG에서는 Anchor와 Alignment를 함께 사용한다.

| Slot ID | 영역 | X | Y | W | H | Anchor | 승인 범위 |
|---|---|---:|---:|---:|---:|---|---|
| `MissionSummary` | 임무 요약 | 64 | 64 | 480 | 128 | Top Left | 위치 승인 / Compact 내부 계약 Draft / Runtime Provider Pending |
| `AlertFeed` | 경고 | 920 | 64 | 720 | 120 | Top Center | 위치 승인 / Compact 내부 계약 Draft / Runtime Provider Pending |
| `TargetPanel` | 선택 타겟 | 2112 | 64 | 384 | 336 | Top Right | 위치 승인 / Tracking·Identification·Intelligence·Lifecycle 계약 반영 |
| `VehiclePanel` | 내 차량 정보 | 64 | 960 | 896 | 416 | Bottom Left | 위치·Visual Layout Accepted / Runtime·UMG Pending |
| `RadarPanel` | 전투 레이더 | 1070 | 952 | 420 | 424 | Bottom Center | 위치 승인 / Elite Dangerous식 공간 인지 참고 |
| `WeaponPanel` | 내 무기 정보 | 2032 | 1016 | 464 | 360 | Bottom Right | 위치·Compact Token User Accepted / Runtime·UMG Pending |
| `ReticleLayer` | 조준·타겟 마커 | 640 | 250 | 1280 | 660 | Center | 보호영역 승인 / 큰 고정 Panel 금지 |

### 9.3 배치별 확정 계약

#### Mission Summary

- 좌상단 `480×128` Slot을 사용하고 **현재 플레이어가 수행해야 하는 대표 Objective 1개를 즉시 읽는 Compact Mission Card**로 사용한다.
- 현재 실제 C++에는 Mission/Objective Runtime Provider가 없으므로 Gameplay HUD가 임의의 목표·진행률·Timer를 생성하지 않는다. Runtime Provider가 연결되기 전에는 실제 플레이에서 MissionSummary를 `Collapsed`하고, D1 Visual Prototype에서만 Mock View Data를 사용할 수 있다.
- MissionSummary는 Mission System의 목표 선택·완료·실패·진행 판정을 직접 수행하지 않는다. Provider/Presenter가 현재 Primary Objective와 Player-facing 표시값을 제공하고 Widget은 배치와 상태 표현만 담당한다.

**기본 정보 계층**

```text
Header        = 짧은 Mission Name 또는 OBJECTIVE
Primary Row   = 현재 대표 Objective 한 줄
Footer        = 선택적 Progress + 선택적 Timer + 선택적 +N OBJECTIVES
```

- 동시에 여러 Objective가 활성화돼도 Widget이 거리·종류·완료율을 비교해 자체 Priority를 계산하지 않는다. Mission Provider가 `Primary Objective`를 명시한다.
- 대표 Objective 외의 활성 목표는 기본 HUD에서 자동 순환하지 않는다. 추가 목표가 있음을 알려야 하면 Footer 우측에 `+N OBJECTIVES` 또는 동등한 Compact Count를 사용할 수 있다.
- `+N OBJECTIVES`는 숨겨진 Objective의 존재만 알리고 이름을 시간 기반 Carousel로 교체 표시하지 않는다. 전체 목표 목록은 후속 Mission Detail/Screen이 소유한다.

**480×128 기본 Geometry**

```text
Vertical Padding  = 12 + 12
Header Row        = 24
Primary Row       = 44
Footer Row        = 28
Row Gap           = 4 + 4
합계              = 128
```

- 위 수치는 2560×1440 Standard 기본 Token이며 Widget Literal로 고정하지 않는다. 1080p에서는 기존 0.75 Layout Scale과 Typography 하한을 함께 적용한다.
- Footer가 필요 없으면 해당 Row와 인접 Gap을 `Collapsed`해 실제 Card Content 높이를 줄일 수 있다. Slot의 Top Left Anchor는 움직이지 않는다.
- MissionSummary가 없을 때 128 높이의 빈 배경을 남기지 않는다.
- Primary Objective Text는 기본 한 줄이다. Provider의 Short Objective Text를 우선하고 필요하면 Ellipsis를 사용하며 자동 2~3줄 확장·Marquee·가독성 하한 이하 Font 축소를 사용하지 않는다.

**진행 정보**

MissionSummary가 이해하는 진행 표현은 특정 Mission Gameplay 타입을 하드코딩하지 않고 다음 Presentation Variant 수준으로 제한한다.

```text
None
Count           예: 3 / 5
Normalized      예: 72% + 얇은 Progress Bar
TimeRemaining   예: 01:42
StateText       예: HOLDING / ESCORTING
```

- Provider는 Player-facing `ProgressText`, 선택적 `NormalizedProgress`, Timer 값을 제공한다. Widget이 Kill Count, 거리, Actor 수, World 상태를 직접 조회해 진행률을 재계산하지 않는다.
- 정확한 총량을 모르면 `3/5`나 `72%`를 임의 생성하지 않는다. Provider가 정밀 Progress를 제공하지 않으면 StateText 또는 진행 Row 없음으로 표시한다.
- Progress와 Timer가 모두 필요한 경우 Footer 한 줄 안에서 Progress를 좌측, Timer를 우측에 두며 별도 두 번째 Footer를 추가하지 않는다.
- 임무 Timer는 Gameplay Provider가 제공하는 **Game Time 기준 남은 시간**을 표시한다. 싱글플레이 World Pause 중에는 줄어들지 않으며 Widget이 Real Time으로 독자 감소시키지 않는다.
- Timer의 소수점 초를 상시 표시하지 않는다. 기본 Combat HUD에서는 `MM:SS` 또는 Provider가 정의한 짧은 형식을 우선한다.

**Objective 상태 변화**

- Objective가 `Active → Completed / Failed`로 바뀌면 해당 Primary Row에서 짧은 1회 Brightness/Opacity/State Accent 전환을 허용한다. 반복 Blink, Scale Pop, 화면 중앙 Banner를 기본 사용하지 않는다.
- CarFight 전역 `CF-PDL-0009` 정책에 따라 MissionSummary 완료·실패·목표 변경에도 게임/UI 사운드를 사용하지 않는다.
- 완료된 Objective를 몇 초간 유지할지, 즉시 다음 Primary Objective로 전환할지는 Mission Provider가 실제 Objective Lifecycle을 구현할 때 정한다. Widget Timer로 임의의 Gameplay 수명을 만들지 않는다.
- Objective가 바뀌면 새 Primary Text와 Progress를 같은 Slot에 갱신한다. Card 전체가 화면에서 다른 위치로 이동하거나 새 Card를 누적하지 않는다.
- Mission 전체 완료·실패 결과는 후속 Result/Mission Flow가 소유하며 MissionSummary를 장시간 `MISSION COMPLETE` Banner로 사용하지 않는다.

**Alert / World Marker 경계**

- 정상적인 Objective 진행, Count 증가와 Timer 감소는 MissionSummary가 소유하며 Alert Feed에 매번 복제하지 않는다.
- 시간 제한 실패 임박, Objective 보호 대상 Critical처럼 즉시 대응이 필요한 사건은 Mission Gameplay/Presenter가 별도 Warning/Critical Alert를 실제로 제공할 때만 Alert Feed에 올라간다.
- MissionSummary가 Timer 값만 보고 자체적으로 `TIME CRITICAL` Alert를 생성하지 않는다.
- 월드 위치 Marker, 경로 Arrow, Objective Actor Screen Projection은 Game/Reticle Layer가 소유한다. MissionSummary가 World Actor를 찾아 거리·방향 Marker를 직접 생성하지 않는다.

**Interaction / 상세 화면 경계**

- P0 MissionSummary는 클릭·Focus·수동 Objective 전환을 요구하지 않는 비상호작용 HUD다.
- 목표를 확인하기 위해 차량 조작·조준 Focus를 빼앗지 않고 Input Mode나 Cursor를 변경하지 않는다.
- 긴 Mission 설명, 전체 Objective 목록, 보상, 로그, 실패 원인 상세와 Mission History는 후속 Mission Detail/Screen으로 분리한다.
- Mission Runtime이 실제 구현될 때 View Data 타입과 Provider 수명은 별도 Mission 기능이 소유하고 CF-FQ-032 Widget이 Mission Domain을 선행 소유하지 않는다.

#### Alert Feed

- 상단 중앙에 두고 **즉시 행동 판단이 필요한 전투 상태만 짧게 전달하는 비상/상태 Feed**로 사용한다. 전투 로그, 디버그 로그, 일반 시스템 메시지 목록으로 확장하지 않는다.
- Alert Feed는 Gameplay 상태를 직접 판정하지 않는다. Presenter/Provider가 `AlertKey`, `Category`, `Severity`, `DisplayText`, 지속 조건을 결정하고 Widget은 정렬·표현 계약만 소비한다.
- 기본 Severity 의미는 다음으로 고정한다.

```text
Notice
→ 알아두면 좋은 짧은 상태 변화
→ 즉각적인 대응이 필수는 아님

Warning
→ 플레이어 대응이 권장되는 전투 상태
→ 방치하면 전투 효율 또는 생존에 불리할 수 있음

Critical
→ 즉시 대응이 필요한 생존·전투 위험
→ 다른 Severity보다 항상 표시 우선
```

- 같은 사실을 색상만으로 구분하지 않는다. 각 Alert는 최소 `Semantic Icon + 짧은 Text`를 사용하고 Severity Accent를 보조로 적용한다.
- 기본 전투 HUD에서 동시에 표시하는 Alert는 **최대 3개**다. Presenter는 활성 Alert 전체 집합을 유지할 수 있지만 Feed는 현재 우선순위가 높은 3개만 렌더링한다.
- 정렬 우선순위는 **`Critical → Warning → Notice`**다. 같은 Severity 안에서는 기본적으로 더 최근에 활성화된 Alert를 위에 둔다.
- 동일 `AlertKey`의 Text·남은 시간·상태가 갱신되는 경우 새 Row를 추가하지 않고 기존 Alert를 **제자리 갱신**한다. Severity가 변하지 않는 단순 값 갱신 때문에 Row 순서를 매번 재정렬하지 않는다.
- 동일 `AlertKey`가 `Warning → Critical`처럼 Severity 상승하면 즉시 새 우선순위로 승격하고 짧은 1회 강조를 허용한다. Severity 하락은 정상 스타일로 갱신하되 강한 완료/성공 연출을 발생시키지 않는다.
- Alert 조건이 해제된 뒤 같은 조건이 나중에 다시 활성화되면 새로운 Alert 발생으로 취급해 Entry 표현을 다시 사용할 수 있다. 같은 조건이 계속 유지되는 동안에는 매 Tick·매 피격·매 프레임 새 Alert를 생성하지 않는다.
- `AlertKey`는 플레이어에게 보이는 문자열이 아니라 **같은 의미 상태를 안정적으로 식별하는 Semantic Key**다. 무기별·소스별 분리가 필요하면 Stable Source/Slot 의미를 포함하되 내부 Object Path나 Debug 이름을 DisplayText로 노출하지 않는다.

**표시 Geometry / Stack**

- Top Center Anchor는 고정하고 Alert는 **위에서 아래로** 쌓는다. 새 Alert 때문에 Feed 전체 Anchor가 이동하거나 중앙 Reticle 방향으로 크게 확장하지 않는다.
- 2560×1440 기본 `720×120` Slot 안에 3개를 수용하기 위해 HUD Alert Feed는 `WBP_CFAlertItem`의 **Compact Feed Density**를 사용한다.
- 기본 Compact Feed는 `Primary Row 40`, `Secondary Row 32`, Row Gap `4` Design Unit을 기준으로 한다. 3개 표시 시 `40 + 4 + 32 + 4 + 32 = 112`로 120 높이 안에 들어오게 한다.
- 가장 높은 우선순위 Row가 Primary Row가 되고 나머지 두 Row는 Secondary Row를 사용한다. Severity 자체가 Row 높이를 결정하지 않으며 Critical은 Icon·Accent·Motion으로 구분한다.
- Alert가 1개뿐이면 Primary Row 하나만 표시하고 남은 공간을 빈 Placeholder로 채우지 않는다.
- Alert Text는 기본 **한 줄**로 유지한다. 긴 문구는 Provider의 Short Display Text를 우선하고 필요하면 Ellipsis를 사용하며 자동 2~3줄 확장, Marquee/Scrolling Text, Font 가독성 하한 이하 축소를 사용하지 않는다.
- Feed 배경을 큰 고정 Panel로 만들지 않고 각 Alert Row의 필요한 Surface/Accent만 사용한다. Alert가 없으면 Feed 전체가 `Collapsed`될 수 있다.

**Overflow / 활성 집합**

- 활성 Alert가 3개를 넘으면 상위 3개만 표시하고 나머지는 Presenter의 활성 집합에 유지한다.
- 숨겨진 Alert가 존재하면 마지막 표시 Row의 우측 보조 영역에 작은 **`+N` Overflow Count**를 표시할 수 있다. 별도 네 번째 Row를 추가하지 않는다.
- 상위 Alert가 종료되면 아직 유효한 다음 Alert를 Feed에 올린다. 이미 Duration이 만료된 일회성 Alert를 뒤늦게 재생하지 않는다.
- Persistent Alert는 숨겨져 있는 동안에도 상태가 유효하면 다시 표시 후보가 될 수 있다. 단순히 Feed에서 밀렸다는 이유로 Gameplay 상태가 해제된 것으로 취급하지 않는다.

**Duration / Persistent / Pause**

- 기본 지속시간은 기존 Alert Style Token을 따른다: Notice 2.0초, Warning 3.0초, Critical은 상태 해제 또는 명시 Duration까지다.
- Duration은 **Game Time 기준**으로 진행한다. 싱글플레이 World Pause 중에는 Alert 남은 시간이 감소하지 않으며 Resume 후 남은 시간부터 이어진다.
- `bPersistent=true`는 실제 상태가 유지되는 동안 Alert가 만료되지 않는다는 의미다. 일반 장기 상태를 무분별하게 Persistent Alert로 만들어 Feed를 영구 점유하지 않는다.
- 장시간 유지되는 일반 정보는 Vehicle/Weapon/Target/Radar 같은 소유 Panel이 지속 표시하고, Alert Feed는 `새로운 위험 발생`, `Severity 상승`, `즉시 대응 필요` 같은 순간을 우선한다.
- Persistent Critical이 실제로 계속 즉시 대응이 필요한 상태라면 Feed에 남을 수 있다. 상태가 해제되면 Presenter가 즉시 제거하고 Widget이 임의의 추가 유지시간을 만들지 않는다.

**중복 알림 금지 / 소유 Panel 경계**

- Target Tactical Analysis 완료는 Target Panel 내부 완료 피드백만 사용하고 Alert Feed Notice를 기본 생성하지 않는다.
- Active Scan으로 새 Contact가 탐지됐다는 사실은 Radar Blip의 1회 Brightness Flash가 기본 피드백이며 Alert Feed에 같은 사실을 반복 Notice로 추가하지 않는다.
- Target 선택/해제, Last Known 전환, Contact Lost, Destroyed Hold는 각 Target/Radar 상태 표현이 기본 소유자이며 별도 Gameplay 요구가 없는 한 Alert Feed에 동일 사건을 중복 표시하지 않는다.
- Ammo/Heat/Charge/Cooldown/Reload의 평상시 값 변화는 WeaponPanel이 소유한다. 단, Provider가 실제 `OVERHEATED`, `WEAPON OFFLINE`처럼 행동 대응이 필요한 상태를 Warning/Critical로 정의하면 Alert Feed에 올릴 수 있다.
- Shield/Armor/Integrity의 평상시 수치 변화는 VehiclePanel이 소유한다. `SHIELD DEPLETED`, `INTEGRITY CRITICAL`처럼 Provider가 명시적으로 Alert 사건을 제공할 때만 Feed에 표시한다.
- Mission 진행 로그와 긴 목표 설명은 MissionSummary가 소유한다. Alert Feed는 시간 제한 실패 임박·즉각적인 임무 위험처럼 실제 전투 대응이 필요한 사건만 받을 수 있다.

**Motion / Critical 표현**

- 새 Alert Entry는 짧은 Fade/Offset 계열 1회 Motion을 사용할 수 있지만 큰 Scale Pop, 화면 전체 Flash, 중앙 Banner를 기본 사용하지 않는다.
- Critical의 반복 Pulse는 허용하되 **Row 전체 Text를 계속 점멸시키지 않고 Accent Marker/Icon에 제한**한다. 핵심 Text는 항상 안정적으로 읽혀야 한다.
- 같은 AlertKey의 일반 값 갱신에는 Entry Motion을 반복하지 않는다. Severity 상승 또는 Inactive → Active 재진입에서만 강한 상태 변화 피드백을 허용한다.
- Alert 제거는 짧은 Fade Out을 사용할 수 있으며 제거 중 같은 Key가 다시 활성화되면 기존 계약대로 제거를 취소하고 최신 상태를 유지한다.

**Interaction**

- P0 인게임 Alert Feed는 클릭·선택·수동 Dismiss를 요구하지 않는 **비상호작용 HUD**다.
- Alert 확인을 위해 차량 조작·조준 Focus를 빼앗지 않으며 Alert가 생성됐다는 이유로 Input Mode나 Cursor 상태를 변경하지 않는다.
- 후속 Mission Log/Message History가 필요해지면 별도 Screen/Panel 기능으로 설계하고 현재 Alert Feed에 스크롤 기록을 누적하지 않는다.

#### Target Panel

- 우상단에 둔다.
- 대상 차량 이미지는 기본 요소로 사용하지 않는다.
- Target Panel의 정보 공개는 단순한 `스캔 전/후` 이분법이 아니라 Radar/Scanner의 **Tracking State + Identification State + 별도 Target Intelligence State**를 조합해 결정한다.
- Target이 없으면 `NO TARGET` 상태를 Compact하게 표시하고 불필요한 상세 행과 상태 Bar는 Collapse한다.
- Target이 선택되면 Identification 수준이 낮더라도 Panel 자체는 유지해 플레이어가 현재 무엇을 선택했는지 항상 알 수 있게 한다.
- **Tracking/Kinematic 정보는 Identification과 별개**다. Live Contact에서 Sensor가 유효하게 제공하는 거리와 이동 정보는 대상의 정체를 몰라도 표시할 수 있다.
- `ID0 Unknown`의 기본 Header는 `UNKNOWN CONTACT` 또는 `???`를 사용하고 Unknown 관계색/상태를 유지한다.
- `ID0`에서는 `NAME / TYPE / FACTION`을 식별된 것처럼 추정하지 않는다. 필요한 고정 Identity Slot에는 `???`를 표시하거나 Compact Density에서 보조 행을 Collapse할 수 있으나 Unknown이라는 의미 자체는 반드시 남긴다.
- `ID0`에서도 현재 Tracking Data가 유효하면 `DISTANCE`와 Sensor가 제공 가능한 `SPEED` 같은 운동 정보를 표시할 수 있다. 이 정보는 Identity 공개가 아니라 Sensor Tracking 결과다.
- `ID1 Affiliation Known`에서는 최소한 전술 관계를 공개한다. Hostile이면 Header/Relation을 `HOSTILE CONTACT`로 표시하고 Radar와 동일한 Hostile 관계색 의미를 사용한다.
- `ID1`은 **관계가 판명된 단계이지 개별 Entity 정체가 판명된 단계가 아니다.** 따라서 별도 정보 근거가 없으면 `NAME / TYPE / FACTION`은 계속 `???`일 수 있다.
- `ID1`에서 실제 Faction 이름까지 확정되는 별도 Intel이 존재하면 그 정보는 표시할 수 있으나, Faction 이름을 안다는 이유만으로 개별 차량/Entity의 `NAME`까지 추정하지 않는다. 구체 정보는 필드별 Knowledge State를 존중한다.
- 정상 Friendly Contact는 기존 Friendly IFF 계약에 따라 탐지 즉시 기본적으로 `ID2`까지 올라가므로 일반 플레이에서 장시간 `ID1 Friendly` 상태를 기본 흐름으로 만들지 않는다.
- `ID2 Identity Known`에서는 확보된 Identity Data에 따라 `NAME/Callsign`, `TYPE/Class`, `FACTION/Organization` 같은 정체 정보를 공개한다.
- `ID2`도 모든 Identity 필드가 반드시 존재한다는 뜻은 아니다. 해당 Entity에 적용되지 않거나 실제 데이터가 없는 필드는 기존 `Unknown / Unavailable / NotApplicable` 의미 규칙을 사용한다.
- `ID0 → ID1`처럼 관계가 새로 판명되면 Header와 관계 표현은 즉시 갱신하되 Panel 전체를 Flash하거나 크게 재생성하지 않는다.
- `ID1 → ID2`에서는 Radar 기본 Blip 모양을 다시 변경하지 않고 Target Panel의 Identity Text가 자연스럽게 공개되는 방식으로 갱신한다.
- 장착 Scanner가 선택 Target을 분석 중이면 Target Panel에 **Compact Identification Analysis Status**를 표시할 수 있다. Analysis Time이 존재하는 장비에서는 진행도 또는 진행 상태를 보여주되 별도의 큰 Scan Panel을 만들지 않는다.
- Identification Analysis가 완료되면 임시 진행 표시는 제거하고 새로 확보한 Identity 정보가 기존 정보 Slot에 반영된다.
- Scanner가 장착되지 않았거나 현재 분석 조건을 충족하지 않는다는 이유만으로 Target Panel에 상시 `SCANNER MISSING`, `OUT OF RANGE` 같은 진단성 문구를 쌓지 않는다. Gameplay에 필요한 실패/불가 피드백은 후속 Scanner UX에서 최소 표현으로 별도 설계한다.
- **`SHIELD / ARMOR / VEHICLE` 정확 수치와 장착 무기, 부품 손상, 약점 같은 상세 전술 정보는 ID2의 자동 공개 항목이 아니다.** 이 정보는 후속 Target Intelligence/Analysis State가 실제로 제공할 때만 공개한다.
- 따라서 `ID2 Identity Known`이어도 별도 Intelligence가 없으면 `SHIELD / ARMOR / VEHICLE`은 `???` 또는 미공개 상태로 남을 수 있다.
- 육안으로 명백한 화재·파손처럼 World에서 직접 관찰 가능한 상태를 후속에 별도 Visual Intelligence로 제공할 수는 있으나, 이를 곧바로 정확한 HP 수치로 변환하지 않는다.
- 정보 공개 때문에 Panel의 상단 기준점이 이동하지 않도록 **Top Right Anchor와 Header 위치는 고정**한다. 추가 정보는 아래쪽으로 확장하며, 처음부터 최대 크기의 빈 Panel을 예약하지 않는다.
- 같은 의미의 기존 행이 `Unknown → Known → Unavailable`로 바뀔 때는 행 위치를 유지해 값만 교체하고, 해당 정보 범주 자체가 아직 활성화되지 않은 상세 Intelligence Block은 Collapse할 수 있다.
- 기본 정보 우선순위는 `Target Identity/Relation → Distance/Tracking → Identification Analysis → 별도 Tactical Intelligence` 순으로 두어 정체를 모르는 단계에서도 현재 선택 Target 판독이 가능하게 한다.

##### Target Intelligence Acquisition

- `ID2 Identity Known`은 **대상이 누구인지 식별된 상태**일 뿐이며 Target Intelligence가 자동 완료된 상태가 아니다. Shield·Armor·Vehicle Integrity·Weapon·Damage 정보는 별도 Intelligence Source가 실제로 제공해야 공개한다.
- Target Intelligence는 하나의 전역 `Intel Level 1/2/3`으로 묶지 않는다. **정보 필드/범주마다 독립적인 Knowledge State**를 갖게 해 같은 Target에서도 `Shield=Known`, `Weapon=Known`, `Rear Armor=Unknown`, `Module Damage=Unavailable` 같은 혼합 상태를 허용한다.
- Target Intelligence의 기본 Knowledge State는 기존 `WBP_CFInfoRow` 계약을 그대로 사용한다: `Unknown / Estimated / Known / Unavailable`. `KnownZero`와 `NotApplicable`은 기존 DisplayState 의미를 유지한다.
- `Estimated`는 센서·육안·전투 관찰로 상태를 추정할 수 있으나 정확한 값이 확정되지 않은 정보다. `Known`은 해당 정보가 신뢰 가능한 Source에 의해 확인된 상태다. 단, `Known`이 반드시 내부 Gameplay의 원시 절대 수치를 그대로 표시한다는 뜻은 아니다.
- Target Intelligence에는 Knowledge와 별도로 **Freshness** 개념을 둔다. 정적 구성 정보와 현재 전투 상태를 구분해, 과거에 확인한 정보가 실시간 값처럼 계속 갱신되는 오류를 막는다.
- 기본 Freshness 의미는 `Current / Stale` 두 상태로 시작할 수 있다. `Current`는 현재 유효한 Source가 갱신 중인 값, `Stale`은 마지막으로 확인된 값이지만 지금은 실시간 갱신을 보장하지 않는 값이다. 정확한 enum/타입명은 후속 View Data 설계에서 정한다.

Target Intelligence Source 우선 의미:

```text
Direct Visual / Combat Observation
→ 외부에서 명확히 보이는 사실 또는 추정
→ 주로 Estimated, 명백한 사실은 Known 가능

Mission / Database / Prior Intel
→ 차량 형식·기본 장비 같은 정적 정보
→ Known 가능
→ 현재 Shield/Armor/Integrity 같은 실시간 수치를 자동 제공하지 않음

Scanner Tactical Analysis
→ 선택 Target의 현재 전술 상태를 Sensor로 분석
→ Scanner Capability와 조건에 따라 Known 정보 제공

Friendly Telemetry
→ 향후 별도 팀 상태 공유 계약이 있을 때만 사용
→ 현재 Friendly IFF/전용 주파수 계약이 곧바로 전체 전투 Telemetry 공유를 의미하지 않음
```

- Scanner의 **Tactical Analysis는 Identification Analysis 이후의 연속 단계**로 취급한다. 선택 Target이 ID2가 되고 Scanner가 Tactical Analysis Capability를 가지면 별도 두 번째 Analyze 입력을 요구하지 않고 같은 선택 Target에 대해 자동으로 후속 분석을 진행할 수 있다.
- Scanner Tactical Analysis의 기본 조건은 `현재 선택 Target + ID2 + Live Contact + Tactical Analysis 유효거리 + Scanner Occlusion/LOS 조건 + 해당 정보 Capability`다.
- `Last Known Contact`만 남은 상태에서는 새로운 실시간 Tactical Analysis를 진행하지 않는다. 이미 확보한 정적 Intel은 유지할 수 있고, 동적 값은 마지막 샘플을 `Stale`로 표시할 수 있다.
- Tactical Analysis Range는 Identification Range와 동일할 필요가 없으며 기본적으로 **더 짧거나 같은 범위**를 허용한다. 고성능 Scanner는 더 먼 거리·더 짧은 분석시간·더 많은 Intelligence Category를 제공할 수 있다.
- 분석 시간은 Category별 또는 Scanner Profile별 선택 조건이다. 모든 Defense/Weapon/Damage 정보가 한 번에 동시에 해금될 필요가 없으며 Scanner Capability에 따라 독립적으로 공개될 수 있다.
- Target이 분석 범위를 벗어나거나 Scanner Occluder 뒤로 완전히 차폐되면 신규 분석과 동적 정보 갱신은 중단한다.
- **진행 중이던 Scanner Tactical Analysis는 분석 조건을 잃는 순간 즉시 0으로 초기화하지 않고, 현재 누적 진행률에서 서서히 감소한다.**
- Target이 다시 Tactical Analysis 유효거리와 Scanner LOS/Occlusion 조건을 만족하면, 아직 남아 있는 진행률에서 분석을 재개한다.
- 분석 조건을 충분히 오래 회복하지 못해 감쇠가 0에 도달한 경우에만 해당 미완료 Analysis 진행을 완전히 잃고 다음 분석은 처음부터 다시 시작한다.
- 진행률 감쇠 속도는 모든 Scanner에 고정된 전역 상수가 아니라 Scanner Profile 또는 Tactical Analysis Profile이 소유하는 조정값으로 두어 고성능 Scanner가 추적 손실에 더 강한 특성을 가질 수 있게 한다.
- 짧은 LOS 끊김·순간적인 거리 이탈 때문에 즉시 전체 분석이 무효화되는 플레이 감각은 사용하지 않되, Target을 장시간 놓친 상태에서도 미완료 분석 진행률이 영구 보존되지는 않게 한다.
- Category별 Tactical Analysis가 독립 진행되는 경우 감쇠도 해당 Category의 미완료 진행도에 독립적으로 적용할 수 있게 하며, 이미 `Known`으로 확정된 Intelligence 자체를 진행률 감쇠 때문에 Unknown으로 되돌리지는 않는다.
- Scanner 성능은 Tactical Analysis에서 최소 **`AnalysisBuildRate`와 `AnalysisDecayRate`를 분리된 조정축**으로 취급한다. 전자는 조건이 유효할 때 미완료 진행률이 얼마나 빠르게 증가하는지, 후자는 조건을 잃었을 때 얼마나 빠르게 감소하는지를 의미한다. 실제 구현 타입명은 후속 Scanner Gameplay 설계에서 정한다.
- 같은 Scanner/Profile에서 기본적으로 `AnalysisDecayRate < AnalysisBuildRate`가 되도록 튜닝해 잠깐의 가림이나 거리 이탈이 분석 성공 자체보다 더 가혹한 페널티가 되지 않게 한다. 정확한 비율·초 단위 수치는 플레이테스트 조정값으로 남긴다.
- 고성능 Scanner는 `더 빠른 Build`, `더 느린 Decay`, `더 긴 Tactical Analysis Range`, `더 넓은 Category Capability` 중 하나 이상으로 차별화할 수 있으며 모든 성능 차이를 단일 등급 숫자 하나에 강제하지 않는다.
- 이미 완료된 Category는 다시 Build/Decay Progress를 돌리지 않고 Knowledge/Freshness 규칙으로 관리한다. 이후 동적 값 재갱신이 필요한 경우에는 별도의 Refresh/Update 경로를 사용할 수 있게 한다.

###### Tactical Analysis Progress UI

- Target Panel에는 동시에 여러 Category의 큰 진행 Bar를 나열하지 않는다. 기본 Compact HUD에서는 **현재 실제로 분석 중인 미완료 Category 하나의 진행 상태**를 우선 표시한다.
- 분석 진행 중에는 작은 Label + 단일 Progress Bar 또는 동일 수준의 Compact Progress 표현을 사용한다. 예: `ANALYZING ARMOR 62%`.
- 유효 분석 조건을 만족하는 동안 Progress는 정상 방향으로 증가하며 기본 강조는 Scanner/Target 선택 강조와 충돌하지 않는 `AccentTactical` 계열을 사용할 수 있다.
- Target이 Range 밖으로 나가거나 Scanner LOS/Occlusion 조건을 잃어 Progress가 감쇠 중이면 **같은 Progress Bar를 유지한 채 값이 역방향으로 감소**하도록 한다. 새로운 별도 Bar를 만들지 않는다.
- 감쇠 중에는 `INTERRUPTED` 또는 동등한 짧은 상태 Text를 보조로 표시하고, Bar/Label 밝기 또는 Opacity를 한 단계 낮춰 `분석이 계속 진행 중`인 상태와 구분한다.
- 감쇠 중이라는 이유로 반복 점멸, 경고색 Flash, 진동, Pulse, 별도 경고 Panel을 사용하지 않는다. 이는 Critical Alert가 아니라 Scanner 분석 연속성이 끊긴 상태다.
- 조건을 다시 만족하면 별도 재시작 연출 없이 같은 Progress가 남은 값에서 즉시 증가 방향으로 전환되고 보조 상태 Text/명도도 정상 분석 상태로 복귀한다.
- Progress가 0까지 감쇠하면 해당 Compact Analysis Progress를 숨기거나 초기 대기 상태로 되돌릴 수 있으며 `FAILED` 같은 강한 실패 메시지를 기본 HUD에 지속 표시하지 않는다.
- Category 분석이 완료되면 Progress 표현을 제거하고 그 Category에서 새로 확보된 Target Intelligence가 기존 정보 Slot/Block에 반영된다.
- 여러 미완료 Category가 동시에 내부적으로 분석 가능하더라도 기본 HUD는 모든 Progress를 병렬 노출하지 않는다. Target Panel 정보 밀도 제한은 유지한다.

###### Tactical Analysis Category Priority

- 기본 Scanner Tactical Analysis는 **한 번에 하나의 미완료 Intelligence Category를 순차 분석**한다. 플레이어가 전투 중 별도 메뉴에서 Category를 직접 선택하거나 우선순위를 매번 조정해야 하는 방식을 기본 계약으로 사용하지 않는다.
- 기본 자동 분석 순서는 **`Shield → Weapon → Armor → Vehicle → Damage/Module`**로 사용한다.
- 이 순서는 먼저 즉각적인 방어막 상태와 Target의 위협 수단을 파악하고, 이후 방향별 방어 상태·전체 생존 상태·깊은 부품 손상으로 내려가는 기본 Tactical 우선순위다.
- 기본 순서는 전역 하드코딩 Gameplay 법칙이 아니라 **Scanner/Tactical Analysis Profile의 기본 Priority List**로 취급한다. 전문 Scanner는 같은 Category 체계를 사용하면서 순서를 Override하거나 일부 Category Capability를 갖지 않을 수 있다.
- 현재 Scanner가 해당 Category Capability를 갖지 않거나 정보가 `NotApplicable / Unavailable`이면 그 Category를 자동으로 건너뛰고 다음 유효 Category로 진행한다.
- Mission/Database/Visual/기존 Scanner 결과로 Category가 이미 충분히 `Known`인 경우 **초기 획득 Analysis를 다시 반복하지 않고 Skip**한다. `Estimated`는 더 신뢰도 높은 Scanner 분석으로 `Known` 승격 가치가 있으므로 기본적으로 분석 대상에 남긴다.
- 이미 `Known`인 동적 정보의 `Current / Stale` 갱신은 초기 Category 획득 순서를 다시 처음부터 돌리는 작업으로 취급하지 않는다. Category가 한번 분석 완료된 뒤의 실시간 갱신은 해당 Provider의 Refresh/Freshness 경로가 담당해 신규 Intelligence 획득 Queue를 불필요하게 막지 않게 한다.
- 현재 Category가 완료되면 Progress UI를 제거하고 결과를 Target Panel에 반영한 뒤, 같은 Target에서 다음 유효 Category를 자동 선택해 분석을 이어간다.
- 현재 Category가 분석 조건을 잃어 미완료 Progress가 감쇠 중인 경우에는 **Progress가 남아 있는 동안 그 Category가 Queue 우선권을 유지**한다. 조건을 재확보하면 다른 새 Category로 갈아타지 않고 기존 Category부터 재개해 분석이 잦게 튀는 현상을 막는다.
- 감쇠가 0에 도달하면 해당 미완료 Category의 재개 우선권도 사라지며, 다음 유효 분석 시 Priority List를 다시 평가한다.
- 플레이어가 다른 Target을 선택하면 이전 Target은 `Selected Target` 조건을 잃으므로 그 Target의 미완료 Analysis는 기존 Decay 규칙을 따른다. 새 Target은 자신의 Knowledge State와 Priority List를 기준으로 독립적인 분석 Queue를 시작한다.
- 다시 이전 Target을 선택했을 때 아직 미완료 Progress가 남아 있다면 그 Category를 먼저 재개한다. 이미 0까지 감쇠했다면 현재 Knowledge State를 기준으로 Priority List를 처음부터 재평가한다.
- 기본 HUD에는 현재 선택 Target의 **현재 분석 Category 하나만** 표시하며, 다음 예정 Category 목록이나 전체 Queue를 상시 노출하지 않는다.
- P0 기본 계약에서는 플레이어 수동 Category 우선순위 변경, Queue 편집, 특정 Category 강제 Scan 입력을 요구하지 않는다. 향후 특수 Scanner Mode가 실제 Gameplay 필요로 추가될 때만 별도 기능으로 검토한다.

###### Tactical Analysis Completion Feedback

- Tactical Analysis 완료 피드백은 **개별 Field가 아니라 Category 완료 사건 단위로 1회** 발생한다. 예를 들어 Armor 분석으로 여러 방향 정보가 동시에 공개되더라도 각 Row마다 Flash·Sound를 반복하지 않는다.
- Category Progress가 100%에 도달하면 현재 Compact Progress 표현을 즉시 제거하기보다 `MotionState` 계열의 **짧은 1회 완료 상태 전환**을 사용한 뒤 결과 정보가 Target Panel의 기존 Slot/Block에 반영되게 한다.
- 새로 공개되거나 `Estimated → Known`으로 승격된 값은 해당 값/Row 또는 해당 Category Block 내부에서만 **Brightness/Opacity 중심의 짧은 1회 Highlight**를 사용할 수 있다. Scale Pop, Bounce, 확장 Ring, 반복 Glow, 반복 Pulse는 사용하지 않는다.
- Panel 전체 Background, Target Header, Radar Blip, Selected Target Bracket, Stalk에는 Category 완료를 이유로 Flash·Pulse·색상 변경을 전파하지 않는다. Tactical Intelligence 획득은 Identification/Target Selection 상태 변경과 구분한다.
- Category 완료 시 Alert Feed에 별도 Notice를 기본적으로 추가하지 않는다. 완료 정보는 이미 Target Panel 안에서 즉시 읽을 수 있으므로 동일 사실을 화면 상단 경고 영역에 중복 표시하지 않는다.
- CarFight 프로젝트 전역 결정 `CF-PDL-0009`에 따라 Tactical Analysis 완료에도 **게임/UI 사운드를 사용하지 않는다**. 완료 피드백은 Target Panel 내부의 짧은 1회 시각 전환만으로 완결돼야 한다.
- `ScannerAnalysisComplete` 같은 Semantic Sound Event, UI 확인음, 음성 안내와 반복 Beep를 CF-FQ-032 완료 계약에 추가하지 않는다.
- Capability 부족·Known 상태로 자동 Skip된 Category에는 별도 완료 Highlight도 발생시키지 않는다. 실제 분석으로 새로운 Intelligence를 획득하거나 `Estimated → Known` 승격한 경우에만 기존 1회 시각 완료 피드백을 사용한다.
- 한 Category가 완료된 직후 다음 유효 Category 분석은 Gameplay상 바로 시작할 수 있다. 완료 연출 때문에 Scanner 로직 자체에 인위적인 대기시간을 강제하지 않는다.
- 기본 HUD에서는 다음 Category가 시작되면 Compact Progress Label이 새 Category로 자연스럽게 전환된다. 별도의 `QUEUE ADVANCING`, `NEXT ANALYSIS` Text는 표시하지 않는다.
- 마지막 유효 Category까지 완료됐더라도 별도의 큰 `ANALYSIS COMPLETE` Banner나 추가 완료 연출을 기본 계약으로 사용하지 않는다. 마지막 Category의 동일한 1회 시각 피드백 후 Progress UI가 Collapse되고 확보된 Intelligence만 남는다.

###### Target Defense Intelligence Layout

- Target Panel의 방어 상태 표현은 플레이어 자신의 VehiclePanel을 축소 복제하지 않는다. **대상 차량 실루엣, 6방향 Armor Plate 이미지, Shield/Integrity의 Current/Maximum 숫자 표현을 그대로 재사용하지 않는다.**
- Target Panel은 `상대 정보를 빠르게 읽는 Compact Intelligence Card` 역할을 유지한다. 기본 정보 흐름은 `Identity/Relation → Tracking → Analysis Progress → Tactical Intelligence Blocks` 순서다.
- Tactical Intelligence Block은 정보가 실제로 존재할 때만 아래쪽으로 추가된다. 모든 Category가 Unknown인 상태에서 Shield/Armor/Vehicle 빈 Block을 미리 예약하지 않는다.
- Defense 정보의 기본 표시 순서는 **`Shield → Armor → Vehicle Integrity`**로 사용한다. 이 Display 순서는 Scanner의 획득 Priority Queue와 별개이며, 정보가 어떤 순서로 분석됐더라도 Panel 안에서는 같은 위치 문법을 유지한다.

**Shield Row**

- Shield Intelligence가 활성화되면 Target Panel 폭 안에서 **한 줄짜리 얇은 상태 Bar**를 사용한다.
- 기본 구성은 `SHIELD` Label + 얇은 Horizontal Bar + 오른쪽 상태값이다.
- 현재 Ratio가 `Known`이면 `0~100%` Normalized 값을 표시할 수 있고 Bar Fill도 같은 Ratio를 사용한다. 내부 `Current / Maximum` 원시 숫자는 기본 표시하지 않는다.
- `Estimated`는 관찰 근거가 허용하는 범위에서 `ACTIVE`, `DAMAGED`, `LOW` 같은 질적 상태 또는 실제 Provider가 준 추정 범위만 표시한다. UI가 임의의 정밀 Percent를 만들어내지 않는다.
- Shield가 없다는 사실이 `Known`이면 빈 Bar를 0%처럼 오해시키지 않고 `SHIELD  NONE` 같은 명시적 상태로 표시한다.
- Dynamic Shield 정보가 `Stale`이면 마지막 Known 값을 보존하되 Bar/Value의 시각 강도를 낮추고 작은 `STALE` 상태를 함께 표시해 현재값과 구분한다.

**Armor Grid**

- Armor Intelligence가 하나라도 존재하면 **고정 2열 × 3행 Compact Armor Grid**를 활성화한다. 대상 차량 실루엣은 사용하지 않는다.
- 방향 배치는 서로 반대되는 면을 한 행에 묶어 다음 순서로 고정한다.

```text
FRONT  | REAR
LEFT   | RIGHT
TOP    | BOTTOM
```

- 각 Armor Cell은 `Direction Label + Micro Bar + Value/State`의 동일 구조를 사용하며 여섯 방향의 위치를 분석 결과에 따라 재정렬하지 않는다.
- Armor Block이 활성화된 뒤 아직 모르는 방향은 Cell 자체를 제거하지 않고 `???`로 유지해 방향 위치가 흔들리지 않게 한다.
- 방향별 Ratio가 `Known`이면 해당 Cell에 Normalized Percent와 Micro Bar를 표시한다. 내부 Current/Maximum Armor 숫자는 기본 Target Panel에 표시하지 않는다.
- `Estimated` 방향은 `DAMAGED`, `HEAVY`, `INTACT` 같은 Provider가 실제 제공한 질적 상태 또는 추정 범위를 사용하고, 단순 육안 정보에서 정확한 Percent를 임의 생성하지 않는다.
- `Stale` 방향은 마지막 값을 보존하되 Current Cell보다 낮은 시각 강도로 표시한다. 같은 Armor Category 전체가 Stale이면 Block Header의 단일 `STALE` 표시로 중복 Text를 줄일 수 있고, 혼합 Freshness에서는 해당 Cell에 비색상 보조 표기를 제공해야 한다. 정확한 Compact Glyph/Text는 후속 UMG Visual Pass에서 조정한다.
- Armor 6방향을 Compact Panel에 넣기 위해 하나의 평균 Armor Bar로 대체하지 않는다. View Data와 화면 표현 모두 여섯 방향 의미를 유지한다.

**Vehicle Integrity Row**

- Vehicle Integrity가 `Estimated` 또는 `Known`으로 제공되면 Armor Grid 아래에 **한 줄짜리 얇은 상태 Bar**를 사용한다.
- 기본 구성은 `VEHICLE` Label + Horizontal Bar + 오른쪽 상태값이며 Shield Row와 같은 정보 문법을 사용해 학습 비용을 줄인다.
- `Known`이면 Normalized Percent/Ratio를 표시하고, `Estimated`이면 실제 관찰/Provider가 제공한 질적 상태를 표시한다. 정확한 HP Percent를 추정해서 만들지 않는다.
- 명백한 파괴 상태가 Known이면 `DESTROYED`를 우선 표시하고 0% 숫자를 추가로 중복 강조하지 않아도 된다.
- `Stale`이면 Shield와 동일하게 마지막 값을 유지하되 낮은 시각 강도 + `STALE` 의미를 제공한다.

**Panel Density / Expansion**

- Target Panel의 Top Right Anchor와 Header는 고정하고 Defense Block은 아래쪽으로 확장한다. 정보 공개 때 Panel 전체를 재배치하거나 화면 중앙 방향으로 크게 확장하지 않는다.
- 기존 TargetPanel의 Compact 기본 크기는 초기/부분 정보 상태의 기준이며, Tactical Intelligence가 추가될 때 필요한 범위에서 아래로 확장 가능하게 한다. 단, WeaponPanel 등 다른 HUD Slot과 중첩되지 않는 Safe Region 안에서만 확장한다.
- Compact Density에서는 Label·Padding·Bar 높이를 줄일 수 있지만 `Shield / 6-way Armor / Vehicle`의 의미 자체를 삭제하거나 Armor를 평균값 하나로 치환하지 않는다.
- Defense Intelligence가 하나도 확보되지 않았다면 Defense Block 전체를 Collapse한다. 일부 정보만 확보된 경우에는 해당 Block만 표시하며 다른 Category를 `???` 행으로 강제로 채우지 않는다.

###### Shield Intelligence

- `ID2`만으로 Target의 Shield 장착 여부, 최대량, 현재량을 자동 공개하지 않는다.
- 명확한 Shield Impact/Barrier Effect처럼 직접 관찰 가능한 현상이 발생하면 `Shield Installed/Active` 같은 **존재 사실은 Known**으로 승격할 수 있다. 현재 강도는 외형만으로 확정할 수 없으면 `Estimated` 또는 `Unknown`을 유지한다.
- Mission/Database Intel이 해당 차량 형식의 기본 Shield 구성과 명목 성능을 제공하면 **정적 Shield 사양**은 Known으로 표시할 수 있으나 현재 Shield 상태를 그 값으로 추정하지 않는다.
- Scanner Tactical Analysis가 Shield 상태 분석 Capability를 가지며 조건을 만족하면 **현재 Shield 상태/Ratio와 Broken 여부**를 Known으로 제공할 수 있다.
- 적 Target의 기본 HUD는 현재/최대 내부 HP 원문보다 **Normalized Ratio/Percent 또는 상태 Bar**를 우선한다. 절대 `Current/Maximum` 수치는 특별한 Gameplay/Intel 계약이 실제로 제공할 때만 표시한다.
- 적 Target의 Shield Regeneration Delay, 초당 재생량, 내부 Tick 상태 같은 세부 런타임 값은 기본 Tactical Analysis 공개 항목에 포함하지 않는다.

###### Armor Intelligence

- `ID2`만으로 현재 6방향 Armor 상태를 자동 공개하지 않는다.
- 육안 관찰은 실제로 보이는 방향의 외부 파손·장갑 손상만 `Estimated`로 만들 수 있다. 보이지 않는 Rear/Bottom 등 다른 방향 Armor를 같은 추정으로 채우지 않는다.
- Mission/Database Intel은 차량 형식의 **명목 Armor 구성/방향 구조**를 Known으로 제공할 수 있으나 현재 남은 Armor를 자동 제공하지 않는다.
- Scanner Tactical Analysis가 Armor 분석 Capability를 가지면 현재 구현의 `Front / Left / Right / Rear / Top / Bottom` 독립 Armor 상태를 각각 분석 대상으로 사용할 수 있다.
- 분석 완료된 방향 Armor는 현재 상태를 Normalized Ratio/Percent로 Known 표시할 수 있으며, 아직 확인하지 못한 방향은 Unknown/Estimated를 그대로 유지한다.
- 6방향 Armor를 하나의 가짜 평균값으로 합쳐 `Known` 처리하지 않는다. Compact Target Panel에서 요약이 필요하더라도 View Data에는 방향별 Knowledge/State를 보존한다.

###### Vehicle Integrity Intelligence

- `ID2`만으로 Vehicle Integrity 현재값을 자동 공개하지 않는다.
- 연기·화재·심한 차체 파손·주행 불능처럼 직접 관찰 가능한 Damage Cue는 Vehicle 상태를 `Estimated`로 제공할 수 있다. 명백한 `Destroyed` 상태는 Scanner가 없어도 Known으로 취급할 수 있다.
- Scanner Tactical Analysis가 Vehicle 상태 Capability를 가지면 현재 `Vehicle Integrity`의 Normalized Ratio/Percent를 Known으로 제공할 수 있다.
- Target Panel의 적 차량 Integrity는 기본적으로 플레이어 자신의 VehiclePanel처럼 내부 `Current/Maximum` 원시 숫자를 그대로 복제하지 않고 Ratio/상태 중심으로 표시한다.
- Vehicle Integrity와 `Damage/Module Intelligence`는 같은 의미가 아니다. Integrity는 차량 전체 생존 상태이고 Damage Intelligence는 실제 손상 위치·부품·기능 저하에 대한 추가 정보다.

###### Weapon Intelligence

- `ID2`만으로 현재 장착 Weapon 목록을 자동 공개하지 않는다.
- 육안으로 명확히 보이는 Weapon Mount 또는 실제 발사 Signature는 해당 무기의 존재·대략적인 Family를 `Estimated` 또는 명백한 경우 Known으로 만들 수 있다.
- Mission/Database Intel이 차량의 표준 Loadout을 제공할 수 있으나 **현재 실제 장착 Loadout과 동일하다고 자동 가정하지 않는다**. 표준 사양과 실장 상태를 구분한다.
- Scanner Tactical Analysis가 Weapon 분석 Capability를 가지면 **현재 장착된 Weapon의 Player-facing Type/Family, 개수와 확인 가능한 Mount 정보**를 Known으로 제공할 수 있다.
- 내부 Asset Name, Object Path, WeaponId 원문, Debug 문자열은 Target Intelligence에서도 노출하지 않는다.
- 적 Target의 **현재 Ammo 수량, Magazine 잔량, Heat, Charge, Cooldown, Reload 남은 시간, Launcher Sequence 내부 진행도**는 기본 Weapon Intelligence 공개 항목에서 제외한다. 이러한 내부 운영 상태는 향후 특수 EW/Telemetry/Advanced Scanner 기능이 명시적으로 추가될 때만 별도 Capability로 검토한다.
- 적이 실제 발사·재장전·과열 같은 행동을 외부에서 명확히 드러내면 해당 행동 자체는 관찰 정보로 표시할 수 있지만 숨겨진 내부 수치를 역산해 Known으로 만들지 않는다.

###### Target Weapon Intelligence Layout

- Target Panel의 Weapon Intelligence는 플레이어 자신의 WeaponPanel 카드·Ammo·Cooldown UI를 축소 복제하지 않는다. **`무엇이 / 몇 개 / 어디에 장착됐는가`**를 빠르게 읽는 Compact Summary Block으로 사용한다.
- Weapon Block은 실제 Weapon Intelligence가 하나라도 확보됐을 때만 활성화한다. Weapon 정보가 전혀 없으면 빈 `WEAPON ???` 목록을 미리 예약하지 않는다.
- 개별 Weapon Instance를 한 줄씩 나열하지 않고 기본적으로 **Player-facing Weapon Family 기준으로 Grouping**한다. 같은 Family의 Weapon이 여러 개면 하나의 Summary Row로 합친다.
- 기본 Row 형식은 `Family 또는 확인된 Display Type + Count + 선택적 Mount Summary`다. 예: `CANNON ×2`, `MISSILE ×4`, `LASER ×1`.
- 같은 Family 안의 실제 모델/세부 Type이 모두 동일하고 그 정보가 Known이면 Player-facing 구체 이름을 사용할 수 있다. 서로 다른 세부 Type이 섞였거나 정확한 모델이 Unknown이면 상위 Family 이름으로 요약하며 내부 Asset/Definition ID를 표시하지 않는다.
- 정확한 Count가 `Known`일 때만 `×N`을 사용한다. 관찰상 여러 개로 보이지만 수량이 확정되지 않은 `Estimated` 정보는 `MULTIPLE` 또는 Source가 제공한 추정 범위를 사용하고 UI가 임의의 정확한 개수를 만들지 않는다.
- Mount 정보는 실제 Intelligence Source가 확인한 **Player-facing Mount Label**만 보조 정보로 표시한다. 예: `FRONT`, `ROOF`, `REAR`, `LEFT`, `RIGHT` 또는 Gameplay가 정의한 의미 있는 Mount Group. World Transform이나 내부 Mount Profile ID를 UI가 직접 해석해 새 이름을 만들지 않는다.
- 같은 Family가 여러 Mount에 분산돼 있으면 한 Row 안에서 확인된 Mount만 짧게 묶을 수 있다. 예: `CANNON ×2  ·  FRONT / ROOF`.
- Mount가 Unknown이면 Family/Count 자체가 Known이어도 Mount 자리에 `???`를 강제로 붙이지 않고 보조 Mount Text를 생략해 Compact Density를 유지한다.
- 기본 Target Panel에서 Weapon Family Summary Row는 **최대 3개**까지 직접 표시한다.
- 4개 이상의 Family Group이 존재하면 기본 Compact 표현은 **앞의 2개 Summary Row + `+N TYPES` Overflow Row**로 축약한다. 숨겨진 Group의 Knowledge는 View Data에서 유지하며 UI 축약 때문에 Unknown으로 되돌리지 않는다.
- Family Row 정렬은 Scanner/Weapon Provider가 제공하는 안정적인 Display Order를 우선한다. 그런 순서가 없으면 재현 가능한 고정 Family/Group 순서를 사용하며, Widget이 화면 표시를 위해 임의의 `위협도`를 계산하거나 전투 Runtime 값을 기반으로 재정렬하지 않는다.
- `Estimated`와 `Known`이 같은 Family에 동시에 존재하면 서로 다른 Row로 중복하지 않고 하나의 Family Row 안에서 가장 신뢰도 높은 정보와 미확정 보조 필드를 조합한다. 예를 들어 Family/Count는 Known이고 일부 Mount만 Estimated일 수 있다.
- Weapon Intelligence는 기본적으로 **정적 Loadout 정보**로 취급하므로 한번 Known이 된 Family/Count/Mount는 같은 Entity Knowledge Memory가 유지되는 동안 보존할 수 있다. Target이 Last Known으로 전환됐다는 이유만으로 Weapon Summary 전체를 Stale 처리하지 않는다.
- 실제 장비 교체·파괴·탈락처럼 Loadout 자체가 Runtime에서 변하는 시스템이 후속 추가되면 그때 Weapon Intelligence에도 별도 Freshness/Invalidation Source를 연결한다. 현재 P0에서는 존재하지 않는 변화를 추정하지 않는다.
- Weapon Block에는 Ammo, Magazine, Heat, Charge, Cooldown, Reload, Launcher Sequence Bar를 넣지 않는다. 이러한 값은 Target Weapon Summary의 정보 밀도를 높인다는 이유만으로 자동 추가하지 않는다.
- 기본 Weapon Block에는 Weapon Icon을 필수 요소로 요구하지 않는다. 후속 Visual Pass에서 Family Semantic Icon이 정보 밀도를 줄이는 데 실제 도움이 될 때만 선택적으로 추가하되 Text만으로도 의미가 완전해야 한다.
- Overflow Row는 `+2 TYPES`, `+3 TYPES`처럼 **추가로 Known인 Family Group 수**만 알리고 숨겨진 Family 이름을 순환 표시하거나 자동 Carousel하지 않는다. 전체 상세 Loadout View는 후속 Expanded Target Detail이 필요할 때 별도 설계한다.

###### Damage / Module Intelligence

- Damage Intelligence는 `Vehicle Integrity`의 중복 퍼센트가 아니라 **실제 손상 부위·부품·기능 저하 정보**를 위한 별도 범주다.
- 현재 CF-FQ-033 P0 Runtime에는 휠·엔진·터렛 등 독립 부품 내구도와 기능 저하가 구현되어 있지 않으므로, 실제 Provider가 없는 동안 Target Panel이 가짜 Module Damage를 생성하지 않는다.
- 현재 Runtime 단계에서 Module Damage 필드는 `NotApplicable` 또는 Provider 부재 의미의 `Unavailable`로 처리할 수 있으며, 내부 Debug의 마지막 피해 결과를 플레이어용 Damage Intel처럼 노출하지 않는다.
- 현재 P0 HUD에서는 Module Damage Provider가 없다는 이유로 `MODULE DATA UNAVAILABLE` 같은 상시 Block을 표시하지 않는다. 실제 Module Intelligence가 하나도 없으면 Damage/Module Block 전체를 Collapse한다.
- Scanner Profile에 Damage/Module Capability가 있어도 Runtime Provider 자체가 없으면 해당 Category는 `Unavailable`로 Skip하고, 가짜 Analysis Progress나 완료 Highlight를 발생시키지 않는다.
- 육안으로 실제 관찰되는 화재·파손·멈춘 포탑·휠 손실 같은 Gameplay Cue가 후속에 구현되면 관찰된 사실만 `Estimated/Known`으로 제공할 수 있다.
- 후속 Module Damage Runtime과 Scanner Capability가 추가되면 Scanner Tactical Analysis는 `Damaged Module / Severity / Functional State` 같은 정보를 별도 Knowledge State로 공개할 수 있다.
- 약점, 관통 취약부, 내부 Module HP와 정확한 Damage Formula는 기본 Target Intelligence 자동 공개 항목으로 취급하지 않는다.

###### Target Damage / Module Intelligence Layout

- Target Panel의 Module Damage는 **모든 정상 Module을 상시 나열하는 장비 목록이 아니라, 확인된 손상·기능 이상을 요약하는 Compact Damage Summary**로 사용한다.
- 기본 Damage Row 구조는 `Player-facing Module Name + Severity + 선택적 Functional State`다.
- 예시 표현은 `ENGINE · CRITICAL`, `TURRET · DISABLED`, `FRONT LEFT WHEEL · DAMAGED` 같은 짧은 한 줄 형식이다.
- Module 이름은 Runtime/Scanner Provider가 제공하는 Player-facing Display Name을 사용한다. `Wheel_0`, `TurretComponent_2`, 내부 GameplayTag 원문, Object Path, Bone Name을 Widget이 직접 사용자 문구로 변환해 노출하지 않는다.
- 휠 수와 배치가 차량마다 달라질 수 있으므로 Target Panel은 4륜을 하드코딩하지 않는다. `FRONT LEFT WHEEL`, `REAR RIGHT WHEEL` 같은 위치 의미가 필요하면 Provider가 명시적인 Display Label 또는 위치 Metadata를 제공해야 한다.
- Engine·Wheel·Turret는 첫 후속 확장 후보일 뿐 Module Type의 고정 전체 목록이 아니다. 이후 Drive, Power, Sensor, Scanner, Weapon Mount 등 실제 Gameplay Module이 추가돼도 같은 Row 계약을 재사용할 수 있게 한다.

**Severity / Functional State 분리**

- `Severity`는 손상의 심각도를, `Functional State`는 실제 기능 가능 여부를 나타내는 서로 다른 정보다. UI가 둘을 하나의 숫자나 임의 등급으로 합치지 않는다.
- 후속 CF-FQ-033의 `모듈 4단계 상태와 기능 저하`가 구현될 때 기본 Player-facing Severity 후보는 `NOMINAL / DAMAGED / CRITICAL / DISABLED`처럼 매핑할 수 있으나, **현재 CF-FQ-032 문서가 Gameplay Runtime enum 이름이나 임계값을 선행 확정하지 않는다.** 실제 Runtime Provider가 최종 상태와 Display Mapping을 소유한다.
- `DISABLED`는 기능 불가가 실제로 확인된 경우에만 사용한다. 단순히 HP가 낮거나 외형이 심하게 파손돼 보인다는 이유로 UI가 기능 정지를 추정하지 않는다.
- 기능 저하가 수치가 아닌 상태로만 제공되는 경우 `DEGRADED`, `JAMMED`, `IMMOBILE`, `NO TRAVERSE` 같은 Player-facing Functional State를 Provider가 제공할 수 있다. Widget은 내부 실패 사유를 조합해 새로운 상태를 추론하지 않는다.
- 기본 Target Panel은 정확한 Module Current/Maximum HP, 손상 임계값, 남은 수명 예상치와 Damage Formula를 표시하지 않는다. 후속 Scanner가 Module Ratio Capability를 명시적으로 제공하더라도 Compact HUD의 1차 표현은 Severity/Functional State를 우선한다.

**표시 대상과 Density**

- 기본 Damage Summary에는 **실제로 손상 또는 기능 이상이 확인된 Module만** 표시한다. 정상 Module을 `ENGINE NOMINAL`, `WHEEL NOMINAL`처럼 전부 나열하지 않는다.
- 단, Scanner가 해당 Target의 Module Coverage를 충분히 완료해 `현재 확인된 손상 Module 없음`을 신뢰 가능하게 판정한 경우에는 빈 Block과 구분하기 위해 단일 `MODULES · NOMINAL` 또는 동등한 Compact 상태를 표시할 수 있다.
- `MODULES · NOMINAL`은 전체 Module Coverage가 Known이라는 근거가 있을 때만 사용한다. 일부 Module만 확인된 상태에서 보이는 손상이 없다는 이유로 전체 정상 판정을 만들지 않는다.
- 기본 Target Panel은 손상 Module Row를 **최대 3개**까지 직접 표시한다.
- 4개 이상의 손상 Module이 확인되면 `가장 먼저 Provider가 제공한 안정적인 Display Order의 2개 Row + +N DAMAGED` Overflow Row로 축약한다. 숨겨진 Module Knowledge는 View Data에 유지한다.
- Damage Row의 정렬은 Provider가 제공하는 `DamageDisplayPriority` 또는 안정적인 Module Display Order를 우선한다. 그런 값이 없으면 재현 가능한 고정 Module Order를 사용한다.
- Widget이 내부 HP 수치·DPS·플레이어 무장·사격각을 계산해 자체적인 `가장 위험한 손상` 순서를 만들지 않는다.
- `Estimated` Damage는 실제 관찰 근거가 있는 Module만 Row로 표시하고 Severity Text도 Source가 제공한 수준까지만 사용한다. 예: `TURRET · DAMAGED?`처럼 별도 물음표를 남발하기보다 Knowledge Style/Estimated 표현 규칙으로 신뢰도를 구분한다.

**Engine / Wheel / Turret 후속 예시**

- Engine Module Runtime이 구현되면 `ENGINE` Row는 `Severity + Functional State`를 표시할 수 있다. 예: `ENGINE · CRITICAL · DEGRADED`.
- Engine 손상으로 출력 제한·시동 불가 같은 실제 기능 상태가 Gameplay Provider에 존재할 때만 해당 Functional State를 표시하며, 현재 속도가 느리다는 사실만으로 Engine Damage를 추정하지 않는다.
- Wheel Module Runtime이 구현되면 각 Wheel은 독립 Module로 취급할 수 있다. 손상된 Wheel만 `FRONT LEFT WHEEL · DAMAGED`, `REAR RIGHT WHEEL · DISABLED`처럼 표시한다.
- 여러 Wheel이 같은 상태라면 Compact Summary에서 `WHEELS ×2 · DAMAGED`처럼 묶을 수 있으나, 개별 위치가 전술적으로 필요하거나 상태가 다르면 별도 Row를 유지한다. Grouping 여부는 Provider가 제공하는 Module Group/Display Policy를 따르고 Widget이 위치 정보를 버리지 않는다.
- Turret Module Runtime이 구현되면 `TURRET` Row는 회전/조준/발사 기능 상태와 연계할 수 있다. 예: `TURRET · CRITICAL · NO TRAVERSE`.
- Weapon 자체의 장착 여부/Family는 기존 Weapon Intelligence가 소유하고, **Turret Damage는 해당 무기 시스템의 기능 저하**를 의미한다. 같은 정보를 Weapon Summary와 Damage Summary에 중복 복제하지 않는다.

**Freshness / Repair / Recovery**

- Module Damage와 Functional State는 후속 Repair·복구·추가 피격으로 바뀔 수 있는 **동적 Intelligence**로 취급한다.
- Live Scanner/Telemetry 갱신이 끊기면 마지막 Module 상태를 즉시 Unknown으로 지우지 않고 `Stale`로 전환한다. Target Panel에서는 해당 Row의 시각 강도를 낮추고 기존 Freshness 규칙을 적용한다.
- Stale 상태에서 Module이 계속 악화되거나 수리되는 것으로 UI가 추정 갱신하지 않는다.
- 다시 유효 분석 조건을 만족해 Module 상태가 갱신되면 `Current`로 복귀하고 새 상태를 반영한다.
- 후속 Repair Runtime에서 Module이 정상 복구되면 해당 Damage Row는 Provider의 최신 Known 상태를 반영한 뒤 Compact Damage Summary에서 제거할 수 있다. 모든 손상이 해소되고 전체 Coverage가 Known이면 `MODULES · NOMINAL`로 전환할 수 있다.
- Module이 `DISABLED` 또는 `DESTROYED`로 한번 확인됐더라도 향후 Repair/Replacement Gameplay가 존재할 수 있으므로 영구 정적 정보로 고정하지 않는다.

**Expanded Detail 경계**

- 기본 Target Panel에는 최대 3개 Damage Row와 Overflow만 표시하고, 전체 Module 목록·정확한 HP·손상 히스토리·원인별 Damage Log·약점·Repair ETA는 넣지 않는다.
- 전체 손상 계통도나 상세 Module Inspector가 실제 Gameplay 가치가 생기면 후속 Expanded Target Detail에서 별도 설계한다. Compact HUD를 그 상세 화면으로 확장하지 않는다.

###### Target Panel Global Density / Overflow

- Target Panel은 **폭을 늘려 중앙 전투 시야를 침범하지 않고, Top Right Anchor와 기본 폭 `384`를 유지한 채 세로로만 확장**한다.
- 2560×1440 기본 Layout에서 `384×336`은 초기/부분 정보용 Preferred Size로 유지하고, **기본 HUD의 Target Panel 최대 높이는 `640`**으로 제한한다.
- `640`은 Target Panel 자체의 기본 최대 표시 높이이며 Widget 내부 Literal로 고정하지 않고 Layout/Density Token으로 관리한다. 1080p·Ultrawide에서는 기존 Layout Scale/Safe Region 규칙에 따라 환산·검증한다.
- `Y=64`에서 최대 높이 640을 사용해도 현재 우하단 WeaponPanel 시작점 `Y=1016`보다 충분히 위에서 끝나므로 기본 1440p Layout의 우측 HUD Slot과 중첩하지 않는다.
- Target Panel이 최대 높이에 도달하더라도 **ScrollBox, 자동 Carousel, 일정 시간마다 정보가 바뀌는 Page Rotation**을 기본 전투 HUD에 사용하지 않는다. 플레이어가 보고 있던 정보가 자동으로 사라지거나 위치가 바뀌지 않게 한다.

**고정 Block 순서**

```text
1. Header / Relation / Identity
2. Tracking / Kinematic
3. Active Identification or Tactical Analysis Progress
4. Defense Intelligence
   - Shield
   - Armor 6-way Grid
   - Vehicle Integrity
5. Weapon Intelligence
6. Damage / Module Intelligence
```

- 이 순서는 **화면 배치 순서**이며 Scanner 획득 Priority와 별개다. 어떤 Category가 먼저 분석되더라도 이미 표시된 Block들의 상하 순서를 다시 섞지 않는다.
- Header/Relation, 현재 선택 Target 식별 의미와 `DISTANCE` 같은 핵심 Tracking 정보는 **항상 유지하는 Pinned 영역**으로 취급해 Tactical Intelligence가 많아졌다는 이유로 제거하지 않는다.
- 현재 실제로 진행 중인 Identification/Tactical Analysis Progress도 분석 중에는 Pinned 영역 바로 아래에 유지한다. 아래쪽 Intelligence가 많다는 이유로 진행 상태를 숨기지 않는다.

**높이 예산 초과 시 압축 순서**

- Target Panel은 정보가 많아져도 Category 자체를 임의 삭제하기보다 **각 Block 내부에서 이미 정의한 Compact/Overflow 규칙을 먼저 적용**한다.
- 첫 번째 압축은 선택적 보조 Text에서 수행한다. Weapon Mount Summary처럼 없어도 핵심 의미가 유지되는 Secondary Detail은 길이 초과 시 Ellipsis/생략할 수 있다.
- Weapon Block은 기존 계약대로 최대 3 Family Row를 유지하되 전체 높이 예산이 부족하면 `앞 2개 + +N TYPES` Overflow 표현을 우선 사용한다.
- Damage Block도 기존 계약대로 최대 3 Row를 유지하되 높이 예산이 부족하면 `앞 2개 + +N DAMAGED` Overflow 표현을 우선 사용한다.
- Armor는 높이 절약을 이유로 6방향을 4방향으로 줄이거나 평균 Bar 하나로 합치지 않는다. `FRONT|REAR / LEFT|RIGHT / TOP|BOTTOM` 2×3 의미는 보존하고 Cell 내부 Padding·Micro Bar 높이·보조 Text만 Compact Token 범위에서 줄인다.
- Shield와 Vehicle Integrity는 한 줄 Bar 구조를 유지하며 서로 합쳐 하나의 `DEFENSE` 평균 Bar로 만들지 않는다.
- Identity 보조 정보 중 `TYPE / FACTION`처럼 Header보다 낮은 우선순위 필드는 기존 Knowledge를 삭제하지 않은 채 **한 줄 Compact Row 또는 생략 가능한 Secondary Row**로 압축할 수 있다. Target Name/Relationship 자체는 유지한다.
- 압축 때문에 `Known` 정보가 View Data에서 사라지거나 `Unknown`으로 변경되지는 않는다. 이는 오직 Compact HUD Presentation 정책이다.

**절대 유지 우선순위**

- 최대 높이 안에서 정보 충돌이 생기면 다음 의미를 우선 보존한다.

```text
A. 선택 Target이 누구/어떤 관계인지
B. 현재 위치·거리 등 전투 추적 정보
C. 현재 진행 중인 Analysis 상태
D. 현재 생존/방어 상태 — Shield / Armor / Vehicle
E. 장착 Weapon Summary
F. Damage / Module Summary
G. Mount 세부 Label, 반복 보조 설명 같은 Secondary Detail
```

- 위 우선순위는 **Block 순서를 재배치한다는 뜻이 아니라 어떤 세부 정보를 먼저 축약할지 결정하는 기준**이다. 예를 들어 Damage가 Critical이어도 Damage Block을 Header 위로 순간 이동시키지 않는다.
- Critical Damage는 기존 Severity Style로 시각적 중요도를 높일 수 있지만 Panel Geometry를 재정렬하거나 다른 Category를 강제로 밀어내지 않는다.

**문자열과 Localization Overflow**

- Target Name, Vehicle Type, Faction, Weapon Family, Module Display Name은 기본 HUD에서 한 Row를 무제한 두 줄·세 줄로 늘리지 않는다.
- 긴 Player-facing Text는 우선 Provider의 Short Display Name을 사용하고, 그래도 폭을 넘으면 Ellipsis를 허용한다. 내부 Asset Name으로 대체하거나 Font를 가독성 하한 이하로 자동 축소하지 않는다.
- Weapon Mount 목록이 너무 길면 Mount Secondary Text부터 생략하고 `Family + Count`를 우선 유지한다.
- Damage Row가 너무 길면 `Module Name + 가장 중요한 Severity/Functional State`를 남기고 낮은 우선순위 Secondary State를 축약한다. 정확한 Short Label 정책은 후속 UMG Visual Pass에서 검증한다.

**Overflow가 남는 경우**

- 위 Compact 규칙을 모두 적용해도 `640` 높이를 초과하는 특수 Target은 기본 HUD에서 새로운 스크롤/페이지를 만들지 않는다.
- 이 경우 각 Category의 기존 Overflow Row를 유지하고 더 깊은 세부 정보는 후속 `Expanded Target Detail`이 소유한다.
- Compact Target Panel은 Expanded Detail이 아직 구현되지 않았더라도 최소한 `+N TYPES`, `+N DAMAGED`처럼 **더 많은 Known 정보가 존재함을 숨기지 않는 요약 표시**를 유지한다.
- Expanded Target Detail이 후속 추가되더라도 기본 전투 HUD의 384 폭·640 최대 높이·Pinned 정보 계약은 유지하고, 상세 화면은 별도 입력/화면 계층에서 다룬다.

###### Target Tracking State / Selection Lifetime

Target Panel과 Radar의 선택 상태는 Contact Tracking State를 다음처럼 해석한다.

```text
Live Contact
→ Last Known Contact
→ Contact Lost
```

**Live Contact**

- 선택 Target이 `Live Contact`인 동안에는 기존 Selected Target 상태를 정상 유지한다.
- Target Panel의 Tracking/Kinematic 값과 `Current` Tactical Intelligence는 각 Provider의 정상 갱신 주기를 따른다.
- Scanner Identification/Tactical Analysis는 기존 유효 조건을 만족하면 정상 진행할 수 있다.
- Radar에서는 정상 Blip + 기존 Selected Target Bracket을 사용한다.

**Live → Last Known 전환**

- 선택 중인 Contact가 `Last Known`으로 전환됐다는 이유만으로 **Target 선택을 자동 해제하지 않는다.** Last Known 유지기간 동안 현재 선택 Target을 그대로 유지한다.
- Target Panel Header의 Name/Relationship/Identity는 기존 Knowledge를 유지하고, Header 또는 Tracking 영역에 짧은 **`LAST KNOWN` 상태 Text**를 추가해 현재 정보가 실시간 Contact가 아님을 명시한다.
- `LAST KNOWN`은 Target 이름을 `???`로 되돌리거나 기존 Identity를 덮어쓰는 대체 Header가 아니다. 예: `MARAUDER-02`는 유지하고 그 아래/옆에 `LAST KNOWN` 상태를 보조 표시한다.
- Radar에서는 기존 Ghosted Contact 표현을 유지하고, 선택 중이었다면 **Selected Target Bracket도 유지**한다. Blip/Stalk는 Last Known 규칙대로 낮은 시각 강도로 표시하되 Bracket은 `선택 상태가 계속 유지됨`을 읽을 수 있을 정도의 AccentTactical 판독성을 유지한다.
- Last Known 선택 유지가 Contact의 실제 위치를 계속 추적한다는 뜻은 아니다. Ghosted Blip의 World 기준점은 마지막 유효 탐지 위치이며 이후 Target의 실제 이동을 추정·외삽하지 않는다.

**Last Known Tracking 표시**

- Live 상태의 `DISTANCE`를 현재 Target까지의 실시간 거리처럼 계속 표시하지 않는다. Last Known에서는 저장된 마지막 유효 위치를 기준으로 **`LAST POS`** 또는 동등한 Player-facing Label을 사용한다.
- `LAST POS` 거리값은 `플레이어의 현재 위치 → 저장된 Last Known World Position` 사이의 거리이므로 플레이어가 움직이면 갱신될 수 있다. 이는 Target이 움직였다는 뜻이 아니라 마지막 위치까지의 현재 거리다.
- 마지막 관측 속도를 보존할 경우에는 **`LAST SPEED` 또는 명시적 Stale 의미**로만 표시하고 더 이상 실시간 Speed로 갱신하지 않는다. Compact Density에서 오해 가능성이 높으면 Last Speed Row를 숨기는 것을 허용한다.
- Last Known Position과 마지막 Speed를 사용해 HUD가 Target의 현재 위치를 예측해서 Ghost Blip이나 거리값을 전진시키지 않는다. 별도 Tracking Prediction Gameplay가 실제로 추가되기 전에는 외삽 금지다.

**Last Known Intelligence / Analysis**

- Identity, Relationship, Weapon Family/Count/Mount처럼 이미 확보된 정적 Knowledge는 Contact Memory가 유지되는 동안 그대로 보존한다.
- Shield Ratio, 6방향 Armor Ratio, Vehicle Integrity, Module Damage/Functional State 같은 동적 Intelligence는 마지막 유효 샘플에서 멈추고 **`Stale`**로 전환한다.
- Last Known 상태에서는 새로운 Identification/Tactical Analysis를 시작하지 않는다.
- Live 상태에서 이미 진행 중이던 미완료 Analysis가 있었다면 기존 `INTERRUPTED + Decay` 규칙을 그대로 적용한다. 진행률이 남아 있는 동안 Target Panel에 같은 Compact Progress를 표시할 수 있고, 0에 도달하면 숨긴다.
- Last Known 상태에서 Progress가 0이 된 뒤에는 Contact가 다시 Live가 되기 전까지 다음 Category Analysis를 새로 시작하지 않는다.
- 선택 유지 자체는 Scanner 분석 조건이나 Fire-Control 유효성을 우회하지 않는다.

**Weapon / Lock Authority 경계**

- `Selected Target` UI 상태와 `Live Fire-Control Target` 유효성은 같은 의미가 아니다.
- Last Known Contact는 HUD에서 선택 상태를 유지할 수 있지만, **무기 Lock/Tracking/발사 판정은 각 Gameplay 시스템이 요구하는 Live Contact·LOS·Sensor 조건을 별도로 만족해야 한다.**
- UI가 Last Known Target을 계속 표시한다는 이유로 무기 시스템에 현재 Actor Transform, 실시간 Aim Point 또는 자동 Lock 성공을 제공하지 않는다.
- 향후 `마지막 위치 사격`, Area Weapon, Indirect Fire 같은 Gameplay가 추가되면 별도 Fire-Control 계약으로 정의하고 기본 Target Selection 유지 규칙에 암묵적으로 포함하지 않는다.

**Last Known → Live 재획득**

- Last Known 유지기간 안에 같은 Contact가 다시 Live Tracking으로 복귀하면 **선택 상태를 그대로 유지한 채** 즉시 Live Target으로 복귀한다. 플레이어에게 재선택 입력을 요구하지 않는다.
- Radar는 Ghosting을 해제하고 정상 Blip/Stalk 표현으로 복귀하며 기존 Selected Target Bracket은 연속 유지한다.
- Target Panel의 `LAST KNOWN` 상태 Text는 제거되고 `LAST POS`는 다시 정상 `DISTANCE` 의미로 복귀한다.
- 동적 Intelligence는 Provider가 새 샘플을 제공하는 시점에 `Current`로 복귀한다. 새 샘플이 아직 오지 않은 값은 재획득 순간 임의로 Current 처리하지 않는다.
- 남아 있는 미완료 Analysis Progress가 있으면 기존 값에서 정상 Build를 재개한다.
- 같은 Contact의 Last Known → Live 복귀는 `새로운 Target 선택`이나 `완전히 새로운 Contact 발견`으로 취급하지 않으므로 별도 Target Selected 연출, 신규 탐지 Brightness Flash, 완료 Highlight 등을 중복 발생시키지 않는다. Ghost → Live 상태 복귀 자체로 충분히 전달한다.

**Last Known → Contact Lost**

- `Contact Lost` 판정 시점은 UI Timer가 독자적으로 정하지 않는다. Sensor/Contact Tracking Provider가 Last Known 유지시간 만료, Contact Record 무효화 등으로 해당 Tracking Record를 제거하는 사건을 소유한다.
- 현재 선택 Target이 `Contact Lost`가 되면 **그 시점에 Selected Target을 자동 해제**한다. 더 이상 존재하지 않는 Tracking Record를 Target Panel이 계속 현재 Target으로 보존하지 않는다.
- Radar의 Ghosted Blip/Stalk와 Selected Target Bracket/Off-range Edge Bracket은 Contact 제거와 함께 종료한다. 기존 Last Known Fade Out이 적용된다면 Bracket도 선택 해제 상태를 반영해 별도 지속 표시하지 않는다.
- Target Panel은 짧은 1회 **`TARGET LOST` 상태 전환**을 허용한 뒤 `NO TARGET` Compact 상태로 복귀한다. 정확한 지속시간은 `MotionState` 계열 Style Token에서 조정하며 장시간 Banner로 유지하지 않는다.
- `TARGET LOST` 전환은 반복 Blink/Pulse, Critical Alert 색, 화면 중앙 Banner, Alert Feed 중복 Notice를 기본적으로 사용하지 않는다.
- Contact Lost가 발생해도 **자동으로 다음 Contact를 선택하지 않는다.** Target 선택이 예기치 않게 다른 적으로 이동해 조준/분석 의미가 바뀌는 것을 막는다.
- Contact Lost로 Target Panel에서 사라졌더라도 Entity/Identification Knowledge Memory 자체는 별도 수명 정책에 따라 더 오래 유지할 수 있다. UI 선택 해제와 Knowledge Memory 삭제를 같은 사건으로 강제하지 않는다.

**Contact Lost 이후 재탐지**

- Contact Lost 이후 같은 Entity가 다시 탐지되면 Contact Tracking Record는 다시 생성될 수 있고, Entity Resolution이 같은 대상으로 신뢰 가능하게 확인되면 보존된 Identity/정적 Intel을 재사용할 수 있다.
- 그러나 이미 Selected Target이 해제된 뒤이므로 **재탐지만으로 자동 재선택하지 않는다.** 플레이어가 다시 Target Selection 규칙에 따라 선택해야 한다.
- 이전 대상과 동일한 Entity라는 근거가 없으면 과거 Knowledge를 새 Contact에 추정 전이하지 않는다.

**수동 선택 변경**

- 플레이어가 Live 또는 Last Known Target을 수동 해제하면 즉시 `NO TARGET` 상태로 전환하며, 이 동작이 Contact 자체나 해당 Contact의 Knowledge Memory를 삭제하지 않는다.
- 플레이어가 다른 Target을 선택하면 기존 Target의 Selected Bracket만 제거하고 Contact는 자신의 Live/Last Known 상태로 계속 존재한다. 미완료 Scanner Analysis는 기존 선택 상실 Decay 규칙을 따른다.
- P0 기본 Target Cycle은 **Live Contact를 신규 선택 후보로 우선**하고, Ghosted Last Known Contact를 자동 Cycle 후보로 추가하지 않는다. 이미 선택된 Target이 Last Known으로 넘어가는 선택 유지와 `새로운 Ghost Contact 선택`은 다른 동작으로 구분한다.
- 향후 Last Known Contact를 지도/지휘 관점에서 명시적으로 다시 선택해야 할 Gameplay가 생기면 별도 Target Inspect/Command 입력 정책으로 확장할 수 있으나 P0 기본 전투 Target Cycle에는 포함하지 않는다.

**Destroyed와 Contact Lost 구분**

- `Destroyed`가 Sensor/Gameplay에 의해 실제로 Known인 상태와 `Contact Lost`는 같은 의미가 아니다. 전자는 대상 파괴가 확인된 전투 상태이고 후자는 Sensor Tracking 신뢰가 끝난 상태다.
- Sensor Lost, Last Known 만료, 낮은 Vehicle Ratio, 정지 상태만으로 UI가 `Destroyed`를 추정하지 않는다. **Gameplay/Vehicle State/신뢰 가능한 Intelligence Provider가 Destroyed를 명시적으로 Known으로 제공할 때만** Destroyed Target Lifecycle에 진입한다.

###### Destroyed Target Lifecycle

**진입과 선택 유지시간**

- 현재 선택 Target의 `Destroyed`가 Known으로 확정되면 즉시 선택을 해제하지 않고 **짧은 Kill Confirmation Hold** 상태로 전환한다.
- 기본 `DestroyedTargetHoldDuration`은 **1.25초**로 사용한다. 정확한 값은 Style/Gameplay Token에서 조정 가능하게 하며 Widget Blueprint Timer Literal로 하드코딩하지 않는다.
- Hold는 파괴 결과를 읽을 시간을 주기 위한 UI/Selection 수명이며 파괴된 Actor의 Gameplay 수명, Wreck 유지시간, Loot 수명과 같은 값이 아니다.
- 플레이어가 Hold 중 다른 Live Target을 직접 선택하거나 Target 해제를 입력하면 1.25초를 기다리지 않고 즉시 기존 Destroyed Target 선택을 종료한다.

**Destroyed 진입 시 Analysis 종료**

- Destroyed가 확정되는 순간 해당 Target의 진행 중 Identification/Tactical Analysis는 **즉시 종료**한다. 이미 파괴가 확정된 Target에 미완료 분석 Progress를 Decay시키거나 다음 Category를 시작하지 않는다.
- Destroyed 때문에 중단된 미완료 Progress를 Analysis Complete로 처리하거나 완료 Highlight/Intelligence Unlock을 발생시키지 않는다.
- 이미 획득된 Identity/정적 Knowledge는 Knowledge Memory에 유지할 수 있지만, Kill Confirmation Hold 동안 새 Tactical Intelligence를 획득하지 않는다.

**Target Panel 표현**

- Hold 동안 Target Panel은 기존 Name/Identity/Relationship을 유지하고 Header 바로 아래 또는 동일한 Status 계층에 **`DESTROYED`**를 명확하게 표시한다.
- 기본 Destroyed Hold 표현은 전투 결과 확인이 우선이므로 `Analysis Progress`를 제거하고 Defense/Weapon/Damage 상세 Block은 낮은 시각 강도로 Freeze하거나 Compact Collapse할 수 있다. 기본 Preset은 **Identity + `DESTROYED` + 추적 가능한 거리 정보 중심의 Compact Terminal State**를 권장한다.
- Target이 파괴된 Wreck로 계속 Live Tracking되고 있으면 `DISTANCE`는 Wreck 위치까지의 실제 Tracking 거리로 유지할 수 있다. Speed는 Provider가 파괴 후에도 의미 있는 실제 값을 제공하지 않는 한 0으로 만들어 표시하지 않고 숨긴다.
- 파괴 확인 직전에 Contact가 Last Known이었다면 `Destroyed`를 제공한 Source가 실제 파괴 위치를 함께 확정하지 않는 한 `LAST POS`를 현재 위치처럼 바꾸지 않는다. `Destroyed Known`과 `현재 위치 Known`을 분리한다.
- Shield/Armor/Module의 마지막 값은 파괴 사실 때문에 임의로 `0%`, `DISABLED`, `DESTROYED`로 일괄 덮어쓰지 않는다. Vehicle 자체의 `DESTROYED`만 확정하고 하위 정보는 실제 Provider가 제공한 마지막 Knowledge/Freshness를 유지한다.
- `DESTROYED` 상태는 반복 Blink/Pulse, 대형 중앙 Banner, Target Panel 전체 Flash를 사용하지 않는다. 짧은 1회 Brightness/Opacity 상태 전환과 명확한 상태 Text로 충분히 전달한다.

**Radar 표현**

- Kill Confirmation Hold 동안 Radar Contact Record가 존재한다면 기존 Contact Blip과 Selected Target Bracket을 짧게 유지한다.
- 파괴 확인만으로 기존 `◆ / ▼ / ●` 기본 Contact Symbol을 새로운 `X`, 해골, 폭발 아이콘으로 교체하지 않는다. 기존 분류/관계 의미를 보존하면서 Blip/Stalk의 밝기·Opacity를 낮춰 비활성 전투 대상임을 표현할 수 있다.
- Selected Target Bracket도 Hold 동안 유지하되 정상 전투 Target보다 한 단계 낮은 시각 강도로 전환할 수 있다. 반복 수축·점멸·회전 효과는 사용하지 않는다.
- Hold가 끝나 Selected Target이 해제되면 **Selected Target Bracket은 반드시 제거**한다.
- 그 이후 파괴 차량/Wreck를 Radar Contact로 계속 표시할지는 Destroyed Target Selection이 아니라 후속 Wreck/World Contact 정책이 소유한다. Target Lifecycle이 파괴된 Actor를 Hostile Live Contact로 영구 유지하도록 강제하지 않는다.

**Hold 종료 / 자동 해제**

- `DestroyedTargetHoldDuration`이 끝날 때까지 플레이어가 다른 Target을 선택하지 않았다면 현재 Destroyed Target을 **자동 선택 해제**하고 Target Panel을 `NO TARGET` Compact 상태로 전환한다.
- Hold 종료 시 별도 `TARGET LOST`를 표시하지 않는다. Target을 잃은 것이 아니라 파괴 결과 확인이 끝난 것이므로 `DESTROYED → NO TARGET`으로 자연스럽게 전환한다.
- 자동 해제 시 다음 Live Contact를 **자동 선택하지 않는다.** 파괴 직후 조준/Scanner/Fire-Control 대상이 플레이어 의도 없이 다른 Contact로 점프하지 않게 한다.
- Target Cycle 입력을 Hold 중 사용해 다른 Target을 선택하는 것은 허용하며, 이 경우 새 Target이 정상 Selected Target이 되고 Destroyed Hold는 즉시 종료한다.

**파괴 직후 Fire-Control 경계**

- `Destroyed`가 Known이 되는 순간 해당 Target은 기본적으로 더 이상 유효한 공격 대상이 아닌 것으로 Fire-Control Provider가 판정할 수 있어야 하며, UI Hold가 무기 Lock/자동 조준 유효시간을 연장하지 않는다.
- 발사 중인 Projectile, 이미 발사된 Missile, Launcher Sequence의 실제 취소/유지 정책은 각 Weapon Gameplay System이 소유한다. Target Panel이 파괴 이벤트를 이유로 임의 취소하지 않는다.
- Destroyed Target 선택이 1.25초 남아 있어도 새 Shot 허용 여부는 Weapon/Fire-Control의 실제 Target Validity를 따른다.

**Destroyed와 Knowledge Memory**

- 자동 선택 해제 후에도 동일 Entity의 Identity, Faction, 정적 Loadout, `Destroyed Known` 결과 자체는 별도 Knowledge Memory 정책에 따라 보존할 수 있다.
- 파괴 차량이 제거됐다가 후속 Mission Log/Score/Expanded Detail에서 참조되더라도 HUD Selection 수명을 다시 살리지 않는다.
- Destroyed Entity가 어떤 이유로 수리·부활·Respawn될 수 있는 Gameplay가 후속 추가되면 동일 Entity 재활성화와 새 Entity Spawn을 명확히 구분하는 별도 Lifecycle 계약을 추가한다. 현재 기본 전투 HUD는 Destroyed 상태에서 자동으로 Live Target으로 복구시키지 않는다.

###### Intelligence Persistence / Freshness

- 한번 확인한 **정적 정보** — 예: Weapon Type, Shield 장착 여부, 명목 Armor 구조 — 는 같은 Entity에 대한 Knowledge Memory가 유지되는 동안 보존할 수 있다.
- **동적 정보** — 현재 Shield Ratio, 방향별 Armor Ratio, Vehicle Integrity Ratio 등 — 는 Live Analysis가 끊기면 마지막 샘플에서 멈추고 `Stale`로 전환한다. Stale 값을 현재값처럼 계속 보간하거나 추정 갱신하지 않는다.
- Target이 다시 Live Contact가 되고 Scanner 분석 조건을 만족하면 Stale 동적 Intel을 새 샘플로 갱신해 `Current`로 복귀시킨다.
- Contact가 Last Known으로 전환되더라도 확보된 Identity와 정적 Intel은 즉시 Unknown으로 되돌리지 않는다. 반대로 동적 Intel이 오래됐다는 사실도 숨기지 않는다.
- `Estimated → Known`은 정보 Source가 강화된 것이므로 자연스럽게 값/표현을 갱신하고, 단순히 시간이 흘렀다는 이유만으로 Estimated가 자동 Known이 되지 않는다.
- Target Intelligence 공개는 **관찰·Intel·Scanner가 실제로 알아낸 것만 보여준다**는 원칙을 유지하며, UI 편의를 위해 적 차량의 숨겨진 Gameplay State를 직접 읽어 자동 공개하지 않는다.

#### Vehicle Panel

상세 소유 문서:

```text
InGameUIVehiclePanelSpec.md
ConceptArt/CFVehiclePanel_1440p.xml
```

승인된 Visual Layout:

- 좌하단 `896×416` 가로형 Slot을 사용한다.
- 좌측 SpeedGauge는 좌측 세로→부드러운 곡선→상단 긴 수평으로 이어지는 비대칭 RPM Gauge와 큰 3자리 디지털 속도 숫자를 사용한다.
- 속도 오른쪽에는 작은 `km/h`, 그 오른쪽에는 단일 Gear Slot을 두며 `R`, `N`, 실제 전진 기어 단수를 표시한다.
- RPM Gauge는 실제 Engine RPM만 표시하며 Red Zone은 고정 시각 스케일 85%에서 시작한다.
- 우측 ArmorBodyMap은 정중앙에 화면 왼쪽을 바라보는 차량 실루엣을 사용한다.
- Front/Right/Rear/Left는 중앙 차량을 둘러싸는 독립 Armor Plate, Top은 좌상단 Badge, Bottom은 우하단 Badge로 둔다.
- Armor 방향·수치 텍스트는 제거하고 6방향 모두 Plate/Badge 우측 세로 Bar로 `NormalizedArmor`를 표시한다.
- 화면 위치는 좌=Front, 상=Right, 우=Rear, 하=Left Armor에 바인딩한다.
- Shield는 상단 Cluster와 Integrity 사이의 전체 폭 Bar이며 중앙 `999/999`, Fill 끝 `▶`·`▶▶` 재생 대기와 `▶▶▶` 재생 중 표시를 사용한다.
- Vehicle Integrity는 최하단 전체 폭 Bar이며 중앙 `999/999`, Fill 끝 Chevron으로 수리 대기/수리 중을 Shield와 동일한 문법으로 표시한다.
- 부품 손상 목록은 고정 VehiclePanel에서 제외한다.

Visual Layout은 Accepted지만 C++ View Data, UMG Asset, 애니메이션과 해상도 검증은 Pending이다.

#### Radar Panel

- 하단 중앙에 둔다.
- 플레이 차량의 TPS 실루엣과 겹치지 않도록 실제 카메라 캡처에서 Y 위치를 미세 조정한다.
- Elite Dangerous의 중앙 하단 공간 인지 방식을 참고하되 우주선식 자원 UI를 복제하지 않는다.
- 기본 방향은 Heading Up이며 지형 미니맵을 표시하지 않는다.
- Radar Scanner에는 **`N / E / S / W` 방위 문자와 상시 Compass Tick을 표시하지 않는다**.
- Scanner의 방향 판독은 Heading Up 기준의 차량 상대 방향을 우선하며 절대 월드 방위를 Radar 외곽 장식으로 중복 표시하지 않는다.
- Scanner 외곽에는 **별도의 전방 Chevron, `FWD` Text, Forward Arrow, Heading Marker를 추가하지 않는다**.
- 전방 의미는 중앙 플레이어 `▲`가 Heading Up 기준으로 위쪽을 가리키는 구조와 Scanner 자체의 Heading Up 동작만으로 전달한다.
- Scanner 전방 Frame의 기존 개방형 실루엣은 시각적 형태로만 유지하고 별도 전방 기호나 텍스트 의미를 추가하지 않는다.
- 후속 Visual Style 튜닝에서도 별도 Gameplay 요구가 정의되지 않는 한 동일한 전방 정보를 중복하는 외곽 Marker를 기본 표현으로 추가하지 않는다.
- Cardinal Label·방위 눈금·반복 Compass Tick을 제거해 Scanner Frame·Range Ring·Contact 판독에 필요한 시각 여백을 유지한다.
- 후속에 절대 월드 방위 정보가 Gameplay 요구로 필요해질 경우 Radar 기본 Frame에 자동 추가하지 않고 별도 Compass HUD/Navigation 표현으로 검토한다.
- Radar 본체는 위에서 내려다보는 평면 원형 미니맵이 아니라 **원근이 들어간 타원형 3D Scanner** 형태를 사용한다.
- 플레이 차량을 Scanner 중심 기준점으로 두고 주변 Contact의 방향·거리와 고저차를 함께 읽을 수 있는 공간 인지 UI를 목표로 한다.
- Contact의 고저차는 **Blip과 Scanner 기준 평면 사이를 잇는 수직 Stalk**로 표현한다.
- 기준 평면보다 높은 대상은 Blip을 평면 위에 두고 Stalk를 아래로 기준면까지 연결하며, 낮은 대상은 Blip을 평면 아래에 두고 Stalk를 위로 기준면까지 연결한다.
- 동일 고도 또는 무시 가능한 고저차 범위에서는 Stalk를 생략하거나 최소화해 화면 노이즈를 줄인다.
- Stalk 길이는 절대 월드 높이를 그대로 픽셀 변환하지 않고 Radar의 고저차 표시 범위에 맞춰 정규화·Clamp한다.
- 고저차 Stalk의 기본 시각 형태는 **얇은 연속 수직선**으로 사용한다.
- Stalk는 Contact Blip의 중심까지 관통하지 않고 **Blip 외곽 바로 앞에서 종료**한다.
- `◆ / ▼ / ●` Solid Blip 내부에는 Stalk 선을 그리지 않아 기본 분류 실루엣이 선과 겹쳐 뭉개지지 않게 한다.
- Stalk와 Blip 사이 간격은 연결 관계가 끊겨 보이지 않을 정도로 최소화하되 Blip 외곽선/실루엣은 항상 완전하게 읽혀야 한다.
- Unknown·Hostile·Friendly의 서로 다른 Blip 형상에 맞춰 시각적 접점 위치는 형상 외곽을 기준으로 계산하되 세 종류 모두 동일한 `내부 관통 없음` 원칙을 적용한다.
- 정확한 End Gap 픽셀값은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 Stalk가 Solid Blip 내부로 들어가지 않는 원칙은 유지한다.
- Stalk 색상은 해당 Contact의 관계 색상을 그대로 따르되 Blip보다 낮은 밝기·Opacity로 표시해 보조 정보 계층을 유지한다.
- 기본 Stalk에는 점선, 중간 Tick, 눈금, 반복 Marker를 넣지 않는다.
- Stalk가 Scanner 기준 평면과 만나는 접점에는 **별도 Dot, Foot Marker, Base Plate, Anchor Ring을 추가하지 않는다**.
- Stalk 선 자체가 기준 평면에서 바로 끝나게 하며 접점 위치를 추가 기호로 중복 강조하지 않는다.
- 기준 평면 Surface와 Range Ring이 이미 공간 기준을 제공하므로 Contact 수가 많아져도 접점 장식으로 시각 노이즈가 누적되지 않게 한다.
- 후속 Visual Style 튜닝에서도 특별한 Gameplay 상태가 별도로 정의되지 않는 한 Stalk 접점 Marker를 기본 표현으로 추가하지 않는다.
- 선택 Target 여부가 Stalk 자체의 색상 의미를 바꾸지 않으며 선택 강조는 기존 별도 Target Bracket이 담당한다.
- 정확한 선 두께와 Opacity는 후속 Visual Style Preset에서 조정할 수 있게 두되 Contact Blip보다 시각적으로 우선하지 않는 원칙은 유지한다.
- Scanner 정중앙의 플레이어 기준점은 작은 **`▲` 방향 마커**로 표시하고 Heading Up 기준 전방을 가리킨다.
- 플레이어 `▲`의 기본 시각 형태는 **작은 Solid Marker**로 사용하고 별도 외곽선은 두지 않는다.
- 기본 색상은 `AccentTactical`보다 시각 강도가 낮은 **밝은 중립 회청색 계열**을 사용해 항상 식별 가능하되 선택 Target Bracket과 강조 우선순위를 경쟁하지 않게 한다.
- 플레이어 Marker는 평상시 Pulse·점멸·회전·Glow 반복 효과를 사용하지 않는다.
- 플레이어 위치는 Scanner 중심 기준점으로 고정하며 Marker 크기·정확한 회청색 값은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 둔다.
- 색상 조정 시에도 플레이어 Marker가 Hostile/Friendly/Unknown Contact 관계색이나 `AccentTactical` 선택 상태로 오인되지 않는 구분 원칙을 유지한다.
- Contact 기본 Blip 모양은 식별 상태에 따라 고정한다: **미확인 = 작은 마름모 `◆`, 적대 = 역삼각형 `▼`, 아군 = 원 `●`**.
- Contact Blip 3종의 기본 렌더링은 모두 **Solid Filled Symbol**로 사용한다.
- Unknown `◆`, Hostile `▼`, Friendly `●`의 **기본 시각 크기는 동일하게 유지**한다.
- Contact 종류에 따라 Blip 크기를 키우거나 줄여 위협도·중요도·식별 수준을 암시하지 않는다.
- 기본 크기 차등 대신 Contact 분류는 기존 모양과 관계색으로, 선택 상태는 기존 Target Bracket으로 구분한다.
- 실제 공통 Blip Size 값은 후속 Visual Style Preset과 UMG 시안에서 조정 가능하게 두되 3종 동일 크기 원칙은 유지한다.
- 기본 Blip에는 별도 Outline을 두지 않으며 평상시 상시 Glow·Bloom·반복 Pulse를 사용하지 않는다.
- Blip 색상은 기존 관계/지식 상태 색상 계약을 그대로 사용하고 선택 상태가 Blip 색상을 덮어쓰지 않는다.
- 선택 상태는 기존 4-Corner Open Bracket이 담당하고 고저차는 기존 Stalk가 담당해 Blip 자체에 추가 장식 정보를 중첩하지 않는다.
- Scan 완료로 새로 탐지된 Contact에만 기존 확정된 1회 밝기 강조를 허용하며 강조 종료 후 즉시 기본 Solid Filled 상태로 복귀한다.
- 정확한 Blip 크기와 밝기는 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 작은 크기에서도 `◆ / ▼ / ●` 실루엣이 우선 판독되는 원칙을 유지한다.
- 현재 선택 Target은 기존 Blip 모양을 다른 기호로 교체하지 않고 **기존 Blip 바깥에 별도 Target Bracket**을 추가한다.
- 선택 Bracket은 Contact의 기본 분류 모양과 고저차 Stalk를 가리지 않아야 하며, 선택 해제 시 Bracket만 제거한다.
- 표시 범위 안의 선택 Target Bracket 기본 실루엣은 **4개의 독립된 Open Corner Bracket**으로 사용한다.
- In-range Target Bracket은 **Screen-Space Upright**로 유지해 Scanner 타원 Perspective나 Contact의 전후 위치에 따라 기울이거나 찌그러뜨리지 않는다.
- 네 Corner의 각도·비율·형태는 Scanner 안 어느 위치에서도 동일하게 유지하고 Bracket의 화면상 위치만 선택 Blip을 따라 이동한다.
- Radar 기준 평면의 원근 압축은 Contact 위치 계산과 Scanner Surface/Ring 표현에만 적용하며 Target Bracket 자체의 2D 실루엣 비율에는 적용하지 않는다.
- 이 규칙은 작은 크기에서도 선택 상태를 안정적으로 판독하기 위한 것이며 Contact의 실제 방향·거리·고저차 의미를 변경하지 않는다.
- 정확한 Bracket 크기·Corner 길이·선 두께는 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 Screen-Space Upright 원칙은 유지한다.
- Bracket은 완전히 닫힌 사각 테두리를 만들지 않고 Blip 바깥 네 모서리에만 배치해 중앙의 `◆ / ▼ / ●` 분류 기호와 Stalk를 그대로 읽을 수 있게 한다.
- 선택 Bracket의 기본 강조색은 관계색을 덮어쓰지 않는 `AccentTactical` 계열을 사용하며 Contact 자체의 관계색은 기존 Blip이 계속 소유한다.
- 평상시 선택 유지 상태에서는 Bracket을 고정 표시하고 불필요한 회전·맥동·반복 점멸을 사용하지 않는다.
- Bracket 크기·Corner 길이·선 두께·Blip과의 여백은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 중앙 정보 비가림 원칙은 유지한다.
- 표시 범위 밖 선택 Target의 Scanner 외곽 방향 Bracket/표식은 별도 Off-range 표현 계약을 유지하며 이번 4-Corner 실루엣 결정으로 자동 대체하지 않는다.
- 색상은 관계 판독을 보조할 수 있지만 Blip의 1차 식별은 모양으로도 가능해야 한다.
- **Radar Sweep은 상시 장식 효과로 사용하지 않는다.** 평상시 Scanner에서는 Sweep을 표시하지 않는다.
- Scanner의 기본 탐지 체계는 **근거리 Passive Tracking + 원거리 Active Scan**의 두 역할로 구분한다.
- **Passive Tracking**은 별도 Scan 입력 없이 근거리 Contact를 지속적으로 탐지·추적하는 기본 Sensor 동작이다.
- Passive Tracking도 Active Scan과 동일한 기본 Occluder 체계를 공유하며 **지형·대형 구조물에 의해 차폐될 수 있다**.
- 산·언덕·대형 암반·건물 본체처럼 Scanner Occluder로 분류된 큰 Geometry가 Contact를 완전히 가리면 근거리라도 Passive Sensor만으로 실시간 Tracking이 보장되지 않는다.
- 반대로 **플레이어가 직접 가시선(LOS)으로 명확히 볼 수 있는 Contact는 Radar에서도 반드시 탐지되는 것을 기본 원칙**으로 한다.
- 따라서 `눈에는 분명히 보이는데 Radar에는 미탐지로 남는` 상태를 기본 Gameplay로 만들지 않으며, 직접 가시 상태의 Contact는 별도 Active Scan 없이 Live Contact로 취급할 수 있어야 한다.
- 이 직접 가시 탐지는 Scanner의 Passive/Active 거리 규칙과 구분되는 **Visual Acquisition 경로**로 설계해, Scanner 장비 성능과 플레이어가 실제로 볼 수 있는 Contact 판정을 서로 억지로 결합하지 않는다.
- Visual Acquisition의 정확한 최대 거리·판정 주기·화면 FOV 의존 여부는 후속 Target/Scanner Gameplay 설계에서 조정하되, `명확히 직접 보이는 Contact는 탐지 보장` 원칙은 유지한다.
- 직접 LOS가 없는 Contact는 다시 Passive Tracking Range, Active Scan Range, Scanner Occlusion, Last Known 규칙의 적용을 받는다.
- 작은 Object는 기존 확정대로 기본 Scanner 차폐에서 제외하며, Passive와 Active가 동일한 Scanner Occluder 분류를 공유할 수 있게 한다.
- **Active Scan**은 플레이어가 Scan을 실행했을 때 Scanner의 성능을 적극 사용해 Passive Tracking 범위를 넘어선 원거리 Contact를 탐지하는 동작이다.
- Active Scan의 기본 탐지 방향은 **차량 중심 360° 전방향 Scan**으로 사용한다.
- 차량 전방만 탐지하는 Cone/FOV 제한을 기본 계약으로 사용하지 않으며 차량의 좌·우·후방에 있는 원거리 Contact도 Scanner 성능과 기타 탐지 조건을 만족하면 같은 Scan Cycle에서 탐지 대상이 된다.
- Radar의 Heading Up은 차량 전방을 화면 상단에 두는 표시 기준일 뿐 Active Scan의 탐지 방향을 전방으로 제한하는 Gameplay 규칙이 아니다.
- 기존 원형 회전 Sweep은 이 360° Scan 범위를 시각적으로 표현하는 진행 UI로 사용한다.
- Contact의 고저차는 기존 3D Scanner/Stalk 계약에 따라 표현하며, 이번 360° 결정은 수평 방위 전방향 탐지를 확정하는 것이다. 정확한 수직 탐지 범위는 후속 Scanner Gameplay 결정으로 남긴다.
- Active Scan은 **지형과 대형 구조물에 의해 탐지 차폐될 수 있다**.
- 산·언덕·대형 암반·건물 본체처럼 전술적으로 의미 있는 큰 World Geometry가 Scanner와 Contact 사이를 막으면 해당 Contact는 Active Scan에서 차폐될 수 있다.
- 가로등·작은 잔해·소형 소품·다른 차량처럼 전술적 차폐물로 보기 어려운 작은 Object는 기본 Scanner 차폐 판정에서 무시해 사소한 Geometry 때문에 탐지가 불안정해지지 않게 한다.
- 차폐 여부는 Render Occlusion과 동일시하지 않고 Scanner Gameplay용 별도 판정으로 취급하며, 어떤 Actor/Geometry가 Scanner Occluder인지 후속 구현에서 명시적인 Collision Channel·Tag·Profile 등으로 구분할 수 있게 한다.
- 360° Scan이라는 방향 계약은 유지되며, 전방향 중에서도 실제 Scanner Line이 대형 차폐물에 막힌 방향의 Contact만 탐지 실패할 수 있다.
- 작은 Object의 정확한 크기 기준이나 예외 목록은 지금 하드코딩하지 않고 실제 월드 제작·플레이테스트에서 Scanner Occluder 분류로 조정한다.
- Active Scan의 기본 사용 방식은 **입력 1회 → Scan 수행 → 완료 시 탐지 결과 갱신**의 One-shot Action으로 한다.
- 플레이어가 Scan 입력을 계속 누르고 있어야 유지되는 Hold-to-Scan 또는 입력을 유지하는 동안 반복 Scan하는 방식은 기본 계약으로 사용하지 않는다.
- Scan 시작 후에는 Scanner가 정해진 Scan Duration 동안 한 번의 Scan Cycle을 수행하고, 정상 완료 시 그 Cycle의 원거리 탐지 결과를 Contact Tracking State에 반영한다.
- Scan 도중의 Sweep은 진행 상태를 보여주는 시각 표현이며 Sweep Line이 개별 Contact를 지나가는 순간마다 탐지 결과를 확정하는 방식으로 고정하지 않는다.
- Scan이 취소되거나 실패하면 기존 확정 계약대로 Sweep을 즉시 종료하며, 완료되지 않은 Cycle의 결과 Commit 여부는 후속 Scanner Gameplay 정책에서 명시적으로 정한다. 기본 UI 계약은 `정상 완료 시 결과 갱신`을 기준으로 한다.
- Scan Duration은 장착 Scanner 성능 또는 후속 Gameplay Profile에서 조정 가능하게 둔다.
- Active Scan의 기본 사용 제약은 **Scan Duration + 재사용 Cooldown**으로 확정한다.
- 한 번의 Scan Cycle이 완료된 뒤에는 해당 Scanner의 Cooldown이 끝날 때까지 새 Active Scan을 시작할 수 없다.
- Scan Duration과 Cooldown은 Scanner 성능 Profile이 소유하는 조정값으로 두며, 고성능 Scanner가 탐지거리뿐 아니라 Scan 속도·재사용성에서도 차이를 가질 수 있는 확장 여지를 남긴다.
- Energy Cost·Heat·Emission/전자전 노출·탄약성 자원·기타 사용 제한은 이번 기본 계약에 포함하지 않고 후속 장비/전력/전자전 Gameplay 시스템에서 별도로 결정한다.
- Radar UI는 후속 구현에서 `Scanning / Cooldown / Ready` 상태를 구분해 표시할 수 있어야 하되 구체적인 Progress 표현과 수치는 별도 Visual/UI 설계에서 조정한다.
- Active Scan은 **차량 조작과 전투를 잠그지 않는 독립 Scanner Action**으로 사용한다.
- Scan Cycle 진행 중에도 플레이어는 주행·가속/감속·조향을 정상적으로 수행할 수 있다.
- Scan Cycle 진행 중에도 기본 무기 조준·발사·전투 입력을 정상적으로 사용할 수 있으며 Active Scan 자체가 무기 사용 불가 상태를 만들지 않는다.
- Active Scan은 차량을 정지시키거나 속도 제한·조향 제한을 거는 행동으로 사용하지 않고 Scanner 장비가 별도로 수행하는 비동기 Gameplay Cycle로 취급한다.
- 후속 Energy·Heat·Electronic Warfare 시스템이 추가되더라도 기본 입력 Lock을 자동 전제하지 않으며, 별도 Gameplay 요구가 있을 때만 명시적으로 제한을 추가한다.
- Active Scan의 유효 최대 탐지거리는 장착 Scanner의 성능 Profile이 소유하며, 고성능 Scanner일수록 더 먼 거리까지 Active Scan할 수 있다.
- Passive Tracking의 구체적인 근거리 수치와 Scanner별 차등 폭은 후속 Scanner Gameplay 설계에서 정하되, Passive가 근거리 기본 탐지이고 Active Scan이 원거리 확장 탐지라는 역할 분리는 유지한다.
- Active Scan은 이번 결정에서 **탐지 거리 확장**을 핵심 역할로 확정하며, 식별 정보 증가·은폐 해제·전자전 상호작용 같은 추가 효과는 별도 Gameplay 결정 없이 자동 포함하지 않는다.
- Contact의 **Detection/Tracking과 Identification은 서로 독립된 상태축**으로 취급한다. `어디에 있는가`를 추적하는 것과 `누구인가`를 식별하는 것을 같은 진행도로 묶지 않는다.
- Identification은 기본적으로 `ID0 Unknown → ID1 Affiliation Known → ID2 Identity Known`의 정보 단계로 정의하되, 모든 Contact가 일정 시간을 순차 누적해서 각 단계를 반드시 통과해야 하는 고정 Timer 방식으로 사용하지 않는다.
- 각 식별 수단은 자체적인 **조건 기반 Identification Rule**을 가지며, 해당 Rule이 만족되면 필요한 Identification 단계까지 즉시 승격할 수 있다.
- 일부 장착형 Scanner나 정밀 분석 기능은 식별 Rule의 조건으로 분석 시간·유효 거리·LOS·Scanner 성능 등을 요구할 수 있다. 이 경우 시간 누적은 전역 Identification 규칙이 아니라 해당 Sensor/Scanner 기능의 개별 조건이다.
- 차량 기본 Radar와 장착형 Scanner는 동일한 Identification State를 사용할 수 있으나, 어떤 조건에서 어느 단계까지 식별 가능한지는 각 Sensor 성능과 Gameplay Rule이 소유한다.
- **Friendly Contact는 예외적으로 단순한 즉시 식별 경로를 사용한다.** 아군 차량은 서로 전용 통신 주파수/IFF 정보를 공유한다는 세계관 설정을 기본 계약으로 한다.
- Friendly 차량이 현재 Radar/Sensor 체계에서 Contact로 탐지되기만 하면, Scanner 거리 단계나 별도 분석 시간을 요구하지 않고 즉시 Friendly로 판정한다.
- 기본안에서는 Friendly IFF에 등록된 식별 정보도 함께 전달되는 것으로 보아 Friendly Contact는 탐지 즉시 `ID2 Identity Known`까지 승격할 수 있게 한다. 따라서 정상적인 아군 Contact를 장시간 `◆ Unknown`으로 유지하지 않는다.
- Friendly 즉시 식별은 **탐지거리 자체를 무한대로 만드는 규칙이 아니다.** 해당 Contact가 Passive Tracking, Active Scan, Visual Acquisition 등 기존 탐지 경로를 통해 Radar에 들어온 뒤 식별이 즉시 완료된다는 의미다.
- 아군 IFF 부재를 자동 Hostile 판정으로 사용하지 않는다. 비아군 또는 식별 불가 Contact는 다른 Identification Rule이 충족되지 않는 한 `ID0 Unknown`으로 남을 수 있다.
- Hostile/비아군 Contact의 `ID1 Affiliation Known`과 `ID2 Identity Known` 승격 조건은 Mission 정보, 이미 알려진 Faction/Entity 정보, 명확한 적대행위, Visual Recognition, 장착 Scanner 분석 등 후속 Gameplay 식별 수단에서 조합 가능하게 둔다.
- 차량에는 **기본 Radar/Sensor가 상시 탑재**되어 있고, 별도의 고성능 **Scanner는 선택 장착 장비**로 취급한다. 두 장비는 같은 Contact/Identification State를 공유하되 역할을 분리한다.
- 기본 Radar의 주 역할은 `근거리 Passive Tracking + Visual Acquisition 연동 + Friendly IFF + 이미 확보된 전술 관계 정보 표시`다. 기본 Radar만으로도 Contact 탐지·추적·Target 선택과 기본 전투가 가능해야 한다.
- 기본 Radar는 비아군의 원시 Sensor Return만 보고 시간을 누적해 정체를 자동 해독하는 정밀 분석 장비로 사용하지 않는다. 별도 정보가 없는 비아군은 기본적으로 `ID0 Unknown ◆`으로 유지할 수 있다.
- 기본 Radar에서도 **외부 또는 명백한 Gameplay 정보**가 생기면 Scanner 없이 Identification을 즉시 승격할 수 있다. Mission에서 이미 알려준 적, 이미 식별한 Entity의 재탐지, 알려진 Faction/Entity 정보, 명백한 적대행위, 충분히 명확한 Visual Recognition 등이 이에 해당한다.
- 플레이어나 아군을 공격하는 등 명백한 적대행위가 확정되면 실제 차량 정체를 아직 모르더라도 최소 `ID1 Hostile ▼`로 즉시 판정할 수 있다. `Hostile 판정`과 `Identity Known`은 같은 조건을 요구하지 않는다.
- Mission/사전 Intel이 관계와 정체를 모두 제공하는 Contact는 탐지 시 `ID0`이나 `ID1`을 인위적으로 거치지 않고 바로 `ID2`로 연결할 수 있다.
- **장착형 Scanner의 추가 가치는 원거리 Active Scan뿐 아니라 Sensor 기반 Identification Analysis**에 있다. Scanner는 기본 Radar가 Unknown으로 유지하는 Contact를 장비 성능과 조건에 따라 `ID1` 또는 `ID2`까지 분석할 수 있다.
- 기존 360° **Active Scan은 넓은 공간에서 Contact를 찾아내는 Detection Action**으로 유지한다. Active Scan 한 번을 완료했다는 이유만으로 그 Cycle에서 발견한 모든 비아군 Contact의 Identification이 자동 상승하지 않는다.
- Scanner의 **Identification Analysis는 Active Scan과 별도 기능 의미**로 취급한다. `찾아내기`와 `누구인지 분석하기`를 분리해 Scanner가 모든 원거리 정보를 한 번에 공개하는 만능 장비가 되지 않게 한다.
- 기본 조작 복잡도를 늘리지 않기 위해 Scanner Identification Analysis는 초기 기본안에서 **현재 선택 Target을 대상으로 조건이 충족되면 자동 진행**하는 방향을 사용한다. 별도의 반복 Analyze 입력을 기본 계약으로 요구하지 않는다.
- 선택 Target이 Live Contact이고 Scanner의 Identification 유효 조건을 만족하면 분석이 가능하다. 구체 조건에는 Scanner 성능, Identification Range, Scanner Occlusion/LOS, 필요 시 Analysis Time 등을 사용할 수 있다.
- Analysis Time은 모든 Scanner에 강제되는 공통 고정시간이 아니며 Scanner Profile의 선택적 조건이다. 즉시 판별형, 일정 시간 유지형, ID1까지만 가능한 저성능 장비, ID2까지 가능한 고성능 장비를 같은 상태 체계 안에서 표현할 수 있게 한다.
- Scanner가 `ID1`과 `ID2`를 반드시 같은 순간에 제공할 필요는 없다. 장비/조건에 따라 관계만 먼저 판별하고 이후 정체까지 확인할 수 있으며, 충분한 사전 정보나 고성능 조건에서는 필요한 단계를 건너뛰어 바로 `ID2`가 될 수도 있다.
- 선택 Target이 Identification/Analysis 유효 조건을 잃으면 현재 확정된 Analysis Decay 계약을 적용한다. 미완료 진행률은 즉시 초기화하지 않고 서서히 감소하며, 조건 재확보 시 남은 진행률에서 재개하고 0에 도달한 경우에만 처음부터 다시 시작한다.
- Radar Blip은 `ID0 → 관계 판명` 시점에 기본 분류 기호가 바뀐다. 예를 들어 Unknown `◆`이 Hostile로 판명되면 `▼`로 변경한다. 반면 같은 Hostile Contact가 `ID1 → ID2`로 올라가도 Radar 기본 `▼`는 유지하며, 추가 Identity 정보는 주로 Target 정보 영역에서 확장한다.
- `ID2 Identity Known`은 차량/Entity의 정체를 안다는 의미이며 Shield/Armor/Vehicle HP, 무장 구성, 손상 상태 같은 상세 전술 정보가 자동 공개된다는 뜻은 아니다. 이러한 정보는 후속 Target Intelligence/Analysis 계층으로 분리할 수 있게 한다.
- Identification 상태는 Contact가 Live에서 Last Known으로 전환된다고 즉시 초기화하지 않는다. 같은 Entity의 기억이 유지되는 동안 이미 확보한 관계·정체 정보는 기존 Ghosted Contact 표현과 함께 보존할 수 있다.
- Active Scan으로 Passive Tracking 범위 밖에서 탐지된 원거리 Contact는 Scan 종료 후에도 즉시 제거하지 않고 **Last Known Contact** 상태로 일정 시간 유지한다.
- Last Known 상태에서는 대상의 마지막 유효 탐지 위치를 보존하되 Passive Tracking이 닿지 않는 동안 실시간 위치를 계속 갱신하지 않는다.
- Last Known Contact의 기본 Radar 표현은 기존 Contact의 **모양과 관계색을 그대로 유지한 채 전체 밝기/Opacity를 낮춘 `Ghosted Contact`**로 사용한다.
- `◆ / ▼ / ●` 분류 실루엣과 Friendly/Hostile/Unknown 관계색은 Last Known에서도 유지해 Contact 종류 자체가 바뀐 것처럼 보이지 않게 한다.
- Last Known Blip은 마지막 유효 탐지 위치에 고정하고, 해당 Contact의 고저차 Stalk도 마지막 탐지 고도를 기준으로 함께 유지하되 Blip과 동일하게 시각 강도를 낮춘다.
- 정보 신선도 표시는 기본적으로 Ghosting만으로 전달하며 `?`, 시계 아이콘, 점선 원, 별도 경고 Badge 같은 추가 장식을 상시 붙이지 않는다.
- 후속 Active Scan 또는 Passive Tracking으로 같은 Contact가 다시 실시간 갱신 상태가 되면 Ghosting을 해제하고 정상 Contact 밝기로 즉시 복귀한다.
- Last Known 유지시간이 종료되어 Contact가 제거될 때는 짧은 Fade Out을 사용할 수 있으나 반복 Blink/Pulse 같은 지속 애니메이션은 사용하지 않는다.
- 정확한 Ghost Opacity·Brightness·Fade Out Duration은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 `같은 모양/관계색 + 낮은 시각 강도` 원칙은 유지한다.
- 같은 Contact를 후속 Active Scan으로 다시 탐지하면 Last Known 위치와 탐지 시점을 갱신한다.
- Last Known Contact가 Passive Tracking 범위 안으로 들어오면 별도 수동 조작 없이 실시간 Passive Tracking 상태로 전환한다.
- 일정 시간 동안 재탐지되지 않은 Last Known Contact는 신뢰를 잃고 Radar에서 제거되는 방향을 기본안으로 하며, 정확한 유지시간·감쇠 규칙·시각 표현은 후속 Scanner Gameplay/Visual 설계에서 조정한다.
- 선택 Target이었다는 이유만으로 Passive 범위 밖의 원거리 Contact에 영구적인 실시간 위치 추적을 제공하지 않는다.
- 이 Last Known 계약은 **초기 가변안**으로 두며, 후속 플레이테스트에서 실시간 유지·유지시간·재탐지 조건을 변경할 수 있도록 Contact Tracking State와 표시 로직을 분리해서 설계한다.
- Radar Zoom은 여전히 표시 배율일 뿐 Passive/Active 탐지 성능을 바꾸지 않으며, Scanner의 최대 표시 Range는 해당 Scanner가 Active Scan으로 도달 가능한 유효 최대 탐지거리 안에서 구성한다.
- Sweep은 실제 Scan 동작이 시작된 동안에만 표시하며 Scanner 중심을 기준으로 회전하면서 Scan 진행 상태를 시각적으로 전달한다.
- Sweep의 기본 시각 형태는 **Scanner 중심에서 외곽까지 이어지는 얇은 Cyan 회전선**으로 사용한다.
- Sweep Line 바로 뒤에는 회전 방향을 읽을 수 있을 정도의 **아주 짧은 반투명 Fade Trail**만 허용한다.
- Contact·Range Ring·Stalk 판독을 가릴 수 있는 넓은 부채꼴 Fill, 대면적 Gradient Wedge, 지속 발광 면은 기본 Sweep에 사용하지 않는다.
- Sweep Line과 Fade Trail은 Scan 종료·취소·실패 시 함께 즉시 사라지며 Gameplay Scan 상태와 무관하게 잔상이 남지 않는다.
- 정확한 선 두께·Trail 각도/길이·Opacity는 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 얇은 Line 중심 표현 원칙은 유지한다.
- Scan이 종료·취소·실패하면 Sweep도 즉시 종료하고, Gameplay Scan 상태와 무관한 반복 회전 애니메이션은 금지한다.
- Scan 완료로 **새로 탐지된 Contact**는 해당 Blip을 짧게 한 번 밝게 강조한 뒤 정상 밝기로 복귀시킨다.
- 신규 탐지 강조의 기본 애니메이션은 **Brightness Flash 1회**만 사용한다.
- Flash 동안 Blip의 크기·Scale은 변경하지 않고 확대 Pop, Bounce, 확장 Ring, Ripple, 추가 Outline을 사용하지 않는다.
- Flash는 현재 Contact 관계색을 유지한 채 밝기만 짧게 상승시킨 뒤 기본 밝기로 복귀하며 Contact 분류 모양과 의미를 변경하지 않는다.
- 선택 중인 Contact라면 기존 Target Bracket과 Stalk는 그대로 유지하고 신규 탐지 Flash가 이들의 크기·색상·표시 상태를 변경하지 않는다.
- 정확한 Flash 지속시간과 밝기 Peak 값은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 1회·Brightness-only 원칙은 유지한다.
- 신규 탐지 강조는 Contact의 기본 모양, 관계 색상, 선택 Bracket, 고저차 Stalk 의미를 바꾸지 않고 일시적인 시각 강조만 추가한다.
- 같은 Contact에 대해 지속 점멸이나 반복 Pulse를 사용하지 않으며, 이미 추적 중이던 기존 Contact에는 신규 탐지 강조를 다시 적용하지 않는다.
- 타원형 Scanner의 Perspective는 실제 World Geometry를 렌더링하는 장면 캡처가 아니라 HUD용 시각 투영이며, Gameplay Sensor 데이터만 표시한다.
- Scanner Frame은 실제 Texture2D 또는 교체 가능한 Brush 자산으로 구성하고 UMG Border 조각을 여러 개 쌓아 외형을 억지로 만들지 않는다.
- Scanner 외곽 실루엣은 완전히 닫힌 타원 테두리가 아니라 **분절형 전술 타원 Frame**을 기본 형태로 사용한다.
- Heading Up 기준 후방과 좌우 외곽은 Scanner 전체 타원 실루엣을 읽을 수 있을 정도로 비교적 명확하게 유지하고, 플레이어 전방 쪽 Frame 일부는 의도적으로 끊어 개방감을 준다.
- Frame에는 `Solid Core + Tactical Cut` 시각 언어에 맞는 작은 분절·Cut을 허용하되, 장식선을 과도하게 늘려 Range Ring·Contact·Stalk·Target Bracket 판독을 방해하지 않는다.
- Frame은 가능하면 하나의 교체 가능한 시각 자산 단위로 다루어 차량·Scanner Theme에 따라 외형을 교체할 수 있게 하며, 개별 Border 조각 조합을 외형 제작 방식으로 사용하지 않는다.
- Frame의 정확한 Cut 개수·위치·선 두께와 픽셀 치수는 후속 시각 시안에서 조정하되 위 실루엣 원칙은 유지한다.
- Scanner 기준 평면은 **아주 옅은 반투명 타원 Surface**로 표시해 Contact의 고저차 Stalk가 어느 평면을 기준으로 연결되는지 읽을 수 있게 한다.
- 기준 평면 Surface의 기본 밝기 분포는 **중앙이 아주 약간 밝고 외곽으로 갈수록 자연스럽게 사라지는 Soft Center-to-Edge Fade**를 사용한다.
- Fade는 평면의 존재와 깊이감만 보조하며 눈에 띄는 Glow, 밝은 원판, Halo, 강한 Vignette처럼 보이지 않게 한다.
- Surface의 최대 밝기와 전체 Opacity는 Range Ring보다도 낮은 시각 계층을 유지하고 Contact·Stalk·Sweep·Target Bracket보다 앞에 나오지 않는다.
- Scanner의 기본 정보 Layer 순서는 **Surface → Range Ring → Stalk → Blip → Target Bracket**으로 유지한다.
- Surface와 Range Ring은 고도상 Blip이 기준 평면 위·아래 어디에 표현되더라도 항상 Contact 정보보다 뒤쪽 시각 Layer에 남아 Blip과 Stalk를 덮거나 흐리게 만들지 않는다.
- 고도가 낮은 Contact도 3D 공간 의미를 이유로 Surface 뒤에 실제 Occlusion시키지 않으며 Radar에서는 정보 판독성을 World Depth 표현보다 우선한다.
- Target Bracket은 선택 상태 판독을 위해 기본 Contact 계층의 최상단을 유지하되 Blip과 Stalk 자체를 가리지 않는 기존 Open Bracket 계약을 유지한다.
- Sweep은 Scan 중에만 나타나는 별도 동적 효과이며 교차 시에도 Contact·Target 판독을 손상시키지 않도록 실제 UMG ZOrder와 Opacity를 후속 튜닝한다.
- Fade는 원형이 아니라 현재 Scanner Perspective와 동일한 타원형 투영을 따르며 Gameplay 거리·고저차·탐지 계산에는 영향을 주지 않는다.
- 정확한 Center 밝기·Edge 감쇠·Opacity Curve는 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 `아주 약한 Center → Edge Fade / 뚜렷한 Glow 없음` 원칙은 유지한다.
- 기준 평면 Surface는 지형·도로·월드 Geometry를 표현하지 않으며, 미니맵처럼 보일 정도의 패턴이나 정보 밀도를 갖지 않는다.
- 기준 평면 위에는 기존 확정된 25% / 50% / 75% Range Ring 3개만 거리 기준선으로 유지한다.
- 촘촘한 격자, 좌표 Grid, 반복 Mesh Pattern은 기본 Scanner에 사용하지 않는다.
- 기준 평면은 Contact·Stalk·Range Ring보다 시각적으로 약하게 유지해 공간 인지의 바닥 기준만 제공한다.
- Scanner 기준 평면의 초기 Perspective Preset은 **세로축이 가로축의 약 42~48%인 중간 정도로 납작한 타원**을 사용한다.
- 초기 시안 기준값은 약 45%를 중심으로 잡되, 이 비율은 Gameplay/Sensor 계산 규칙이 아니라 순수 Visual Preset이다.
- 타원 눌림 정도는 후속 TPS 카메라 캡처와 실제 UMG 시각 검토에서 조정할 수 있어야 하며, 조정 시 Contact 거리·Sensor 탐지·Range Preset·Stalk 의미를 변경하지 않는다.
- 구현 시 가능하면 Scanner Visual Style 또는 Brush/Material 파라미터에서 조정 가능하게 두고 Gameplay 코드 상수로 고정하지 않는다.
- Scanner 내부에는 거리 감각을 위한 옅은 타원형 **Range Ring 3개**를 둔다.
- 세 Ring은 현재 Radar 표시 범위의 **25% / 50% / 75%** 지점에 배치하고, Scanner 외곽선은 현재 표시 범위의 100%를 의미한다.
- Ring 자체에는 상시 숫자 라벨을 넣지 않고 상대 거리 감각을 제공하는 기준선으로 사용한다.
- Range Ring의 기본 시각 형태는 **끊기지 않은 얇은 연속 타원선**으로 사용한다.
- Ring은 Frame·Contact·Stalk·Target Bracket보다 낮은 명도로 유지해 거리 기준선 역할만 하며, 전투 정보보다 시각적으로 앞에 나오지 않는다.
- 기본 Range Ring에는 Tick, 눈금, 짧은 분절선, 장식 Marker를 추가하지 않는다.
- Frame 자체의 Tactical Cut과 시각적 역할을 분리해 Ring까지 과도하게 분절하지 않으며, 필요 시 선 두께·Opacity는 후속 Visual Style Preset에서 조정 가능하게 둔다.
- Radar 좌상단 Range Text는 **`현재 표시 범위 / Scanner 최대 탐지거리`** 형식으로 작게 표시한다. 예: `RANGE 1.0 / 4.0 km`.
- Range Text는 **Scanner Frame 내부 좌상단**에 한 줄 Compact Text로 배치한다.
- Scanner Frame 내부 **우상단에는 Scan Status 전용 Compact 영역**을 두어 좌상단 Range 정보와 역할을 분리한다.
- Radar 상단 정보 구조는 기본적으로 **좌상단 `Range` / 우상단 `Scan Status`**의 대응 배치를 사용한다.
- Scan Status는 별도 큰 Panel이나 Header Bar를 만들지 않고 한 자리에서 `Ready / Scanning / Cooldown` 상태를 교체 표시한다.
- `Ready` 상태는 평상시 시각 노이즈를 줄이기 위해 숨기거나 매우 약한 `SCAN READY` 수준으로 표시할 수 있다.
- `Scanning` 상태에서는 `SCANNING` 상태 표기와 Scan 진행 정보를 제공하고, 실제 공간적 진행감은 기존 Radar Sweep이 함께 담당한다.
- `Cooldown` 상태에서는 `SCAN 3.2s`처럼 남은 재사용 대기시간을 짧고 직접적으로 표시할 수 있다.
- 후속에 `JAMMED`, `OFFLINE` 등 Scanner 비정상 상태가 추가되면 같은 Scan Status 영역을 확장해 사용하고 Range 정보와 혼합하지 않는다.
- Scan Status의 정확한 Font Size·Offset·Progress 표현 방식은 실제 Radar 시안과 UMG 구현에서 조정하되, `좌상단 Range / 우상단 Scan Status` 정보 구조는 유지한다.
- Range Text 내부 시각 계층은 `RANGE` Label과 `m/km` Unit을 낮은 명도로 두고, 실제 `현재 표시 범위 / Scanner 최대 탐지거리` 숫자값은 한 단계 밝게 표시한다.
- Label·숫자·Unit은 서로 다른 의미색을 사용하지 않고 동일한 중립 색상 계열 안에서 명도 차이만 사용한다.
- 숫자값 강조는 읽기 우선순위만 높이며 실제 Range 상태·Scanner 성능·경고 상태를 추가로 의미하지 않는다.
- 정확한 Label/Unit Opacity와 Numeric 밝기 차이는 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 `Label·Unit 약함 / Numeric 한 단계 밝음` 계층은 유지한다.
- Range Text를 위한 별도 Header Bar, 배경 박스, 캡슐형 Label Surface는 기본 Radar에 사용하지 않는다.
- Text는 Scanner Frame과 Surface 안쪽의 여백에 배치해 타원형 Scanner 실루엣을 방해하지 않으며 Contact·Stalk·Sweep과 겹치지 않게 한다.
- 정확한 Offset·Font Size·Tracking은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 두되 한 줄 Compact 표현과 무배경 원칙은 유지한다.
- 실제 단위는 m/km를 사용하고 Scanner의 탐지·표시 거리 계약을 플레이어가 읽을 수 있어야 한다.
- Scanner의 **실제 유효 최대 탐지거리**는 장착 Scanner 성능과 Gameplay 상태가 소유하며, Radar Zoom은 이 탐지 성능을 변경하지 않는 순수 표시 배율이다.
- Radar Zoom은 연속 배율이 아니라 **단계식 Range Zoom**을 사용한다.
- 마우스 휠 Up 한 단계는 Zoom In으로 다음으로 작은 Range 단계로 이동하고, 마우스 휠 Down 한 단계는 Zoom Out으로 다음으로 큰 Range 단계로 이동한다.
- Zoom 단계는 Scanner의 유효 최대 탐지거리 안에서 미리 정의된 Range Preset을 사용한다. 예를 들어 최대 4 km Scanner는 `0.5 → 1 → 2 → 4 km` 같은 단계 구성을 사용할 수 있다.
- 실제 단계 값은 Scanner 성능 Profile이 소유하며 모든 Scanner에 0.5/1/2/4 km를 강제하지 않는다.
- Zoom Out 상한은 현재 Scanner의 실제 유효 최대 탐지거리이며 그 이상을 표시하지 않는다.
- Contact의 Scanner상 거리 위치는 현재 표시 범위를 기준으로 다시 정규화해 확대·축소되며, Zoom 변경 때문에 Gameplay Contact의 실제 World Distance나 Sensor 탐지 여부를 다시 판정하지 않는다.
- 현재 Radar 표시 범위 밖에 있는 **일반 Contact는 Scanner 내부 Blip에서 숨긴다**.
- 단, **현재 선택 Target이 표시 범위 밖에 있을 경우 Scanner 외곽선에 해당 Target의 방향을 가리키는 작은 Bracket/표식을 유지**한다.
- 외곽 선택 Target 표식은 대상이 현재 표시 범위 밖에 있다는 사실만 전달하며 실제 거리나 탐지 상태를 왜곡하지 않는다.
- 표시 범위 밖 선택 Target의 기본 외곽 표식은 **2-Corner Open Edge Bracket**으로 사용한다.
- Edge Bracket은 Scanner 외곽 타원선에 붙는 두 개의 짧은 Corner Segment로 구성하고 Target이 존재하는 Scanner 바깥 방향으로 열린 형태를 사용한다.
- Off-range Edge Bracket의 Orientation은 **Scanner 외곽 타원상의 현재 Target 방향 지점에 맞춰 회전**시키며, 두 Corner의 열린 쪽은 항상 Scanner 바깥의 실제 Target 방향을 향한다.
- Target 방향이 Scanner 둘레를 따라 이동하면 Edge Bracket의 위치와 각도도 그 방향을 따라 갱신하되, 이는 방향 추적을 위한 Orientation 갱신이지 장식 목적의 지속 회전 Animation이 아니다.
- Edge Bracket의 Corner 비율과 길이는 방향에 따라 찌그러뜨리지 않고 동일한 기본 형상을 유지한 채 전체 Orientation만 바꾼다.
- Scanner 타원의 접선/법선 방향은 후속 구현에서 일관된 기준으로 계산해 Bracket이 외곽선에 자연스럽게 붙고 Open Side가 항상 바깥 Target 방향을 가리키게 한다.
- Off-range 표식에는 적대 Contact의 `▼`와 혼동될 수 있는 독립 삼각형 화살표를 기본 형태로 사용하지 않는다.
- Edge Bracket의 기본 강조색은 범위 안 선택 Target Bracket과 동일한 `AccentTactical` 계열을 사용하고, 반복 Pulse·점멸·회전은 사용하지 않는다.
- Off-range Edge Bracket은 **Scanner Frame보다 항상 앞쪽 시각 Layer**에 표시해 선택 Target 방향 표식이 Frame 분절선에 가려지지 않게 한다.
- Edge Bracket과 Scanner Frame이 같은 화면 위치에서 겹쳐도 Bracket의 판독을 우선하며, 이를 위해 Frame 자체를 동적으로 지우거나 끊어내지는 않는다.
- 기본 순서는 `Scanner Frame → Off-range Edge Bracket`으로 유지하고 정확한 ZOrder 값은 후속 UMG 구현과 Visual Style 튜닝에서 정한다.
- Edge Bracket은 방향만 전달하며 실제 거리를 표현하기 위해 외곽선에서 추가로 밀어내거나 길이를 변화시키지 않는다.
- 정확한 Corner 길이·선 두께·외곽선과의 간격은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 둔다.
- 선택 Target이 다시 표시 범위 안으로 들어오면 외곽 표식을 제거하고 정상 Blip + Target Bracket 표현으로 복귀한다.

#### Weapon Panel

상세 소유 문서:

```text
InGameUIWeaponPanelSpec.md
ConceptArt/CFWeaponPanel_1440p.xml
```

Compact Revision 구조:

- 우하단 `464×360`은 다른 HUD와 충돌하지 않게 하는 최대 Slot·Anchor 경계이며 실제 카드가 반드시 전부 채워야 하는 고정 면적이 아니다.
- 사용자 승인된 축소형 방향에 따라 실제 Content는 `DesiredSize` 기반으로 더 작게 수축하고 불필요한 Header 여백·설명 Caption·고정 행을 제거한다.
- 이전 `432×224` Selected Card와 `136×96` Tile 수치는 초기 넓은 Wireframe의 참고값이며 구현 하드코딩 값으로 사용하지 않는다.
- 선택 무기는 Compact Profile에서도 가장 강한 Accent·Primary Resource·Fire State를 유지한다.
- Secondary Resource는 최대 2개지만 실제 제공 채널 수에 따라 행 자체가 Collapse되고 남은 채널이 자동 재배치된다.
- 비선택 무기는 작은 독립 Tile로 유지하며 Rail이 없으면 Rail 높이와 하단 여백도 함께 제거한다.
- Ammo·Charge·Heat·Cooldown·Reload·LauncherSequence는 공통 채널을 사용하되 지원되지 않는 채널은 `N/A` 행 대신 Collapse한다.
- 탄약형·내부 충전형·차량 배터리형·런처형·복합형의 Primary·Secondary 채널 우선순위는 상세 Spec이 소유한다.
- WeaponPanel Compact Token은 `InGameUIWeaponPanelSpec.md v0.5.0` 기준 사용자 승인된 기본 Preset이다. 현재 Production HUD 자산화 구조는 준비됐지만 실제 Runtime View Data 연결은 UI-P0-03 이후이며, 1440p·21:9·32:9는 후속 Layout Profile 확장 검토로 유지한다.

#### Reticle Layer

Reticle Layer는 외부 3인칭 주행·조준과 Target 관찰을 위한 중앙 보호영역이다. 큰 고정 Panel은 금지하고 **Command Reticle / Turret Reticle / Selected World Target Marker / Objective World Marker / 미래 Lock Indicator / Immediate Fire State / Hit Confirmation / Incoming Damage Direction**을 서로 다른 의미 계층으로 배치한다.

##### Command Reticle — 최종 기본 실루엣

현재 `Image_CenterDot + Image_LeftBracket / RightBracket / TopBracket / BottomBracket` 바인딩과 직접 호환되는 **Center Dot + 4 Open Bracket**을 기본 실루엣으로 확정한다.

```text
             ──

        │    •    │

             ──
```

실제 Brush는 위 ASCII를 그대로 그리는 것이 아니라 중앙 Dot과 네 개의 짧은 독립 Segment가 충분한 빈 공간을 남기는 형태다.

Standard / 2560×1440 기본 Visual Token:

```text
CommandReticleBounds       = 48 × 48
CommandCenterDotSize       = 4 × 4
CommandBracketLength       = 8
CommandBracketStroke       = 2
CommandBracketInnerGap     = 10
```

- 수치는 기본 Preset이며 Style Data에서 조정 가능해야 한다.
- Command Reticle은 **Screen-Space Upright**다. 차량 Roll·Yaw, Target 방향과 함께 회전하지 않는다.
- Command Reticle의 화면 위치는 기존 Camera Aim/플레이어 조준 결과를 그대로 따른다. 디자인 때문에 강제로 Viewport 정중앙에 고정하지 않는다.
- 중앙을 닫힌 원, 큰 Crosshair, 전체 Circle로 채우지 않는다. 차량과 Target의 실제 화면 정보를 읽을 공간을 남긴다.
- `Ready` 기본 시각은 `TextPrimary` 계열을 우선해 Target Selection의 Cyan과 경쟁하지 않는다.
- 관계색, Hostile Red, Friendly Color로 Command Reticle 자체를 물들이지 않는다. Reticle은 Target 관계가 아니라 플레이어 조준 명령을 뜻한다.
- 평상시 Scale Pulse·회전·Breathing Glow를 사용하지 않는다.

##### Turret Reticle — CurrentMuzzleDirection Marker

`Image_WeaponReticle`은 **작은 Split Cross / 4-Tick Micro Marker**를 기본 실루엣으로 사용한다.

```text
          │

       ─     ─

          │
```

Standard 기본 Visual Token:

```text
TurretReticleBounds        = 24 × 24
TurretTickLength           = 6
TurretTickStroke           = 2
TurretCenterGap            = 5
TurretReticleOpacity       = 0.82
```

- 중앙 Dot은 사용하지 않아 Command Reticle과 즉시 구분한다.
- 기본 강조는 `AccentTactical` 계열을 사용하되 Command Reticle보다 작은 면적과 낮은 Opacity를 유지한다.
- `CurrentMuzzleDirection`을 사용자 조준점과 같은 깊이에 투영한 현재 계약만 표현한다.
- `DirectImpact`, `LaunchDirection`, 첫 Blocking Hit, 중력 궤적, 예상 착탄점 의미를 이 Marker에 추가하지 않는다.
- Command와 Turret Reticle 사이에 기본 Tether Line·Laser Line·Connecting Arc를 그리지 않는다. **두 Marker 사이의 실제 화면 간격 자체가 정렬 오차의 직관적 표현**이다.
- 두 Reticle이 거의 겹쳐도 하나를 강제로 숨기지 않는다. Turret Micro Marker를 아래 Layer에 두고 Command Center Dot이 최종 조준 기준으로 위에서 읽히게 한다.

##### Reticle 상태 시각 우선순위

상태 의미는 기존 `ECFVehicleReticleState`와 `FCFVehicleFireFeedbackViewData`가 소유한다. Style은 결과만 표현한다.

```text
Ready
→ 안정적인 기본 Reticle

TurretAligning
→ Turret Reticle + 짧은 ALIGNING 상태를 StateCaution 계열로 보조
→ Command Reticle을 빨간 실패 상태로 바꾸지 않음

MuzzleBlocked / AimBlocked
→ Command Reticle Bracket과 즉시 상태에 StateCaution 강한 표현
→ BLOCKED 의미를 Text/Icon과 함께 제공

Cooldown
→ 짧은 COOLDOWN Text + 선택적 Micro Bar

NoWeapon
→ TextDisabled 계열 + NO WEAPON

FireRejected
→ 실제 일반 거부 사건에서만 StateCritical 계열 짧은 1회 상태

FireSuccess
→ 기존 의미를 유지한 짧은 Brightness 강조 1회
```

- `WeaponNotAligned`는 기존 Current 계약대로 일반 빨간 `FireRejected`로 자동 승격하지 않는다.
- `OutOfArc`는 현재 단독 발사 차단이 아니므로 경고 보조 표현으로 유지한다.
- `FirePending`, `Reloading`은 실제 Provider가 활성 상태를 제공할 때만 표시한다. enum 존재만으로 시각 상태를 만들지 않는다.
- 동일 순간 여러 Text를 겹치지 않고 Presenter가 정한 대표 Immediate State 한 줄을 Command Reticle 아래에 표시한다.

Immediate State 기본 영역:

```text
Max Width                  = 120
Top Gap from Reticle       = 8
Status Row Height          = 20
Cooldown Micro Bar         = 48 × 3
```

- Cooldown Bar는 실제 `CooldownRatio`가 있을 때만 사용한다.
- 내부 RejectReason enum 원문을 화면에 표시하지 않는다.
- 상태 Text는 한 줄을 유지하며 `BLOCKED`, `ALIGNING`, `COOLDOWN 0.8`, `NO WEAPON`처럼 짧은 Player-facing 문구를 사용한다.
- CarFight 전역 `CF-PDL-0009`에 따라 Reticle, FireFeedback과 Lock 관련 상태는 **사운드 없이 시각적으로 완결**한다.

##### Selected World Target Marker

현재 선택 Target의 On-screen 기본 표현은 **4-Corner Open Target Bracket**이다. Command Reticle과 달리 대상의 Screen Bounds 또는 Provider Anchor를 감싸는 외곽 선택 표식이다.

```text
┌            ┐

     TARGET

└            ┘
```

Standard 기본 Visual Token:

```text
TargetMarkerMinSize        = 44 × 44
TargetMarkerMaxSize        = 120 × 120
TargetMarkerBoundsPadding  = 8
TargetMarkerCornerLength   = 10
TargetMarkerStroke         = 2
```

- 실제 Projected Bounds가 지나치게 작거나 커도 Min/Max 범위에서 Clamp한다.
- Target Marker는 `AccentTactical` Selection 의미를 사용하며 Contact 관계색을 덮어쓰지 않는다.
- 관계 정보가 필요하면 Bracket 전체 색을 관계색으로 바꾸기보다 작은 `◆ / ▼ / ●` Semantic Glyph 또는 별도 관계 Marker를 보조로 사용할 수 있다.
- 기본 HUD에서는 Target Name·Faction·Defense 값을 Marker 주변에 반복 표시하지 않는다. 정체와 Intel은 Target Panel이 소유한다.
- 실제 Tracking Distance를 Provider가 제공하면 Bracket 아래에 한 줄 Compact Distance를 선택적으로 표시할 수 있다.
- Target Bracket 중앙은 비워 Command/Turret Reticle과 실제 차량 실루엣을 가리지 않는다.
- Draw Order는 `World Target Marker < Command/Turret Reticle`을 유지한다.

##### Objective World Marker / Mission Navigation

Objective World Marker는 **MissionSummary의 월드 위치 복제판이 아니라 Primary Objective까지의 공간 방향을 알려주는 Navigation 표식**이다. 현재 Mission/Objective Runtime Provider가 없으므로 실제 Gameplay에서 이 Marker를 생성하지 않는다.

```text
현재 Gameplay
Mission Provider 없음
→ MissionSummary Collapsed
→ Objective World Marker Collapsed

D1 Visual Prototype / 별도 Static Mock
→ Mock Objective View Data 사용 가능
→ 실제 Mission Runtime 구현 증거로 취급하지 않음
```

P0 기본 Navigation 대상은 Mission Provider가 명시한 **Primary Objective 1개**로 제한한다. `+N OBJECTIVES`가 있다고 Widget이 거리순·화면 중앙 근접순·생성순으로 다른 Objective Marker를 추가하지 않는다. 후속 Mission 기능이 Secondary Navigation을 명시적으로 요구하기 전까지 여러 Objective Marker를 동시에 증식시키지 않는다.

**Target Marker와 다른 기본 실루엣**

On-screen Objective는 **Open Hex Beacon + 짧은 Bottom Stem**을 기본 형태로 사용한다. Target 선택의 4-Corner Bracket, Radar Unknown의 Diamond, Hostile `▼`, Off-screen Target의 2-Corner Bracket과 실루엣을 공유하지 않는다.

```text
       /   \
      |     |
       \   /
         │
```

Standard / 2560×1440 기본 Visual Token:

```text
ObjectiveBeaconBounds          = 30 × 34
ObjectiveBeaconHexSize         = 24 × 22
ObjectiveBeaconStroke          = 2
ObjectiveBeaconOpenGap         = 6
ObjectiveBeaconStemLength      = 6
ObjectiveDistanceGap           = 6
ObjectiveDistanceMaxWidth      = 72
ObjectiveMarkerOpacity         = 0.92
```

- 위 수치는 기본 Preset이며 Style/Layout Data에서 조정 가능해야 한다.
- 기본 색은 **`StateNotice`** 계열을 사용한다. `AccentTactical`은 Target 선택·조준 의미가 강하므로 Objective 기본색으로 사용하지 않는다.
- Mission이 위험 상태라는 이유만으로 Objective Beacon 전체를 Hostile Red로 바꾸지 않는다. 즉시 대응 위험은 Mission Provider가 Alert Feed에 별도 Warning/Critical 사건을 제공한다.
- Marker는 색상만으로 Objective임을 전달하지 않는다. Open Hex + Stem의 고유 Shape가 기본 비색상 식별자다.
- 현재 Semantic Icon Set에 `Objective` 전용 Icon Asset이 존재한다고 가정하지 않는다. Mission Runtime/Assetization이 실제 범위가 될 때 전용 Brush/Semantic ID를 추가 검토하며, 이번 상세 설계 때문에 D1-11 Production Asset을 변경하지 않는다.

**Primary Objective와 MissionSummary 연결**

- MissionSummary의 Primary Objective와 Objective World Marker는 **같은 Provider의 동일 Primary Objective Identity**를 소비해야 한다.
- MissionSummary에 Primary Objective가 있어도 Navigation Anchor가 없는 Objective는 World Marker를 표시하지 않는다. 예: `5대 격파`, `생존`, `시간까지 버티기`처럼 특정 월드 지점이 필요 없는 목표다.
- 반대로 Widget이 World Actor를 검색해 Marker Anchor를 만들지 않는다. Mission Provider/Presenter가 Player-facing Navigation Anchor를 명시적으로 제공해야 한다.
- Primary Objective 변경은 Summary Text와 Navigation Marker가 서로 다른 Objective를 가리키는 프레임이 생기지 않도록 같은 Presentation Snapshot에서 갱신하는 것을 권장한다.
- MissionSummary를 클릭해 Objective를 순환하거나 Marker를 바꾸는 상호작용은 P0에 두지 않는다.

**위치 정밀도 / 정보 노출**

Objective Marker는 Mission이 플레이어에게 **알려주기로 한 위치만** 표시한다.

```text
Exact Anchor
→ Mission Provider가 정확한 Player-facing 위치를 공개
→ 정상 Open Hex Beacon

Approximate Anchor
→ Mission Provider가 검색 구역 중심·추정 위치 등 근사 Anchor만 공개
→ 분절된 Open Hex / 낮은 정밀도 상태
→ UI가 실제 Actor 위치를 찾아 보정하지 않음

No Navigation Anchor
→ Marker Collapsed
```

- `Approximate`는 현재 Mission Runtime enum을 선행 확정하는 것이 아니라 미래 Provider가 위치 정밀도를 전달할 수 있어야 한다는 표현 계약이다.
- 비밀 Target, Search Objective, Fog-of-War 목적에서 Mission이 정확한 Actor 위치를 공개하지 않았는데 Widget이 `GetActorLocation`이나 TargetSelect 위치를 사용해 정확한 Marker를 만들지 않는다.
- World Geometry 뒤에서도 Marker를 유지할지 여부도 Mission Provider의 Navigation Visibility 정책이 소유한다. Objective Marker가 보인다는 사실은 Sensor LOS, Target Tracking, Lock 또는 Fire 가능 상태를 의미하지 않는다.

**거리 표시**

- 기본 Objective Marker는 실제 Provider/Presenter가 Player-facing `DistanceText` 또는 동등한 표시값을 제공할 때만 한 줄 거리값을 표시한다.
- 기본 예시는 `842 m`, `1.4 km` 같은 Compact Text다. Widget이 월드 Actor를 직접 조회해 거리 정밀도를 결정하지 않는다.
- Mission이 근사 위치만 공개하는 경우 정확한 Meter 값을 자동 계산하지 않는다. 필요하면 Provider가 `~1.4 km`처럼 근사 의미까지 포함한 Text를 제공한다.
- 거리 Text는 Objective 이름·설명과 함께 두 줄 이상으로 확장하지 않는다. Objective 내용은 MissionSummary가 소유한다.

**Objective Off-screen / BehindCamera**

Objective도 Target과 동일한 Camera View Space → Safe Region Ray Intersection 투영 절차를 사용하지만 **Edge 실루엣은 Target과 다르게** 유지한다.

Off-screen Objective는 **Open Edge Hex Beacon**을 사용한다. Hex의 화면 바깥 방향 면을 열어 실제 Objective 방향을 나타내며 독립 삼각 Arrow를 사용하지 않는다.

```text
ObjectiveEdgeBounds            = 30 × 26
ObjectiveEdgeStroke            = 2
ObjectiveEdgeOpenGap           = 8
ObjectiveEdgeStemLength        = 6
ObjectiveEdgeSafeInset         = Layout/SafeRegion Token
ObjectiveEdgeMinSeparation     = 40
ObjectiveEdgeMaxTangentShift   = 56
```

- On-screen과 동일한 `StateNotice` 계열을 사용해 Target Edge Bracket과 관계색 Marker에서 분리한다.
- Marker의 열린 면은 Objective가 존재하는 실제 화면 밖 방향을 향한다.
- Edge 위치는 거리를 표현하기 위해 더 안쪽/바깥쪽으로 움직이거나 Marker 크기를 거리 비례로 변경하지 않는다.
- 거리 Text가 있으면 Safe Region 안쪽 방향에 배치하고 Panel 영역이나 Viewport 밖으로 잘리지 않게 한다.

**Persistent HUD / Safe Region 충돌**

- Objective Projection은 기존 World Marker와 같은 Navigation Safe Region을 사용해 MissionSummary, Alert, TargetPanel, VehiclePanel, Radar와 WeaponPanel 뒤에 Marker를 숨기지 않는다.
- Projected Anchor가 Persistent HUD 예약 영역 안으로 들어오면 단순 원본 Screen Coordinate를 Panel 위에 그리지 않고 Safe Region 경계 Navigation 상태로 처리할 수 있다.
- Objective Marker 때문에 Persistent Panel 위치를 움직이지 않는다.

**Target Marker와 동일 Actor/Entity일 때**

Mission Objective가 현재 Selected Target과 같은 Entity인 경우 두 개의 완전한 Marker를 중첩하지 않는다.

```text
Primary Objective Identity
== Selected Target Identity
→ Selected Target 4-Corner Bracket 유지
→ Objective Open Hex를 작은 Mission Badge로 축소
→ Target Bracket 상단 중앙 바깥에 부착
```

Mission Badge 기본값:

```text
ObjectiveTargetBadgeSize       = 18 × 20
ObjectiveTargetBadgeGap        = 8
```

- Selection Bracket이 Target 선택을, 작은 Open Hex Badge가 Mission Objective임을 각각 유지한다.
- 같은 Entity라는 이유로 Target을 자동 선택하거나 Objective가 Target Lock을 생성하지 않는다.
- Target Marker에 이미 실제 거리값이 표시되는 경우 Objective Badge 아래에 같은 거리값을 중복하지 않는다.
- Associated Identity가 없거나 둘이 같은 Entity라는 신뢰 가능한 Provider 정보가 없으면 Screen 위치가 우연히 겹친다는 이유만으로 두 Marker를 합치지 않는다.

**서로 다른 Target/Objectives의 Screen-space 충돌**

- Selected Target Bracket은 실제 Target Bounds를 우선하므로 Objective 충돌 때문에 이동시키지 않는다.
- Objective Beacon의 World Anchor 자체도 다른 Actor처럼 보이게 크게 이동시키지 않는다.
- 겹침이 발생하면 Draw Order로 Target Marker를 위에 두고, Objective의 **Distance/보조 Text를 먼저 Collapse**한다. 두 Shape는 Open Center/얇은 Stroke로 동시에 식별 가능하게 유지한다.
- Command/Turret Reticle 근처에서는 Objective Icon 위치를 거짓으로 옮기기보다 Distance/Secondary Text를 제거한다. Reticle이 항상 가장 높은 조준 의미를 유지한다.

**Edge Marker 충돌**

Target Off-screen Marker와 Objective Edge Marker가 같은 Safe Region 구간에 모이면 다음 순서를 사용한다.

```text
Selected Target Edge Marker
→ 원래 Ray 교차 위치 유지

Primary Objective Edge Marker
→ 같은 Edge를 따라 Tangential 방향으로 최소 간격 확보
→ 최대 ObjectiveEdgeMaxTangentShift 안에서만 이동
```

- 기본 최소 간격은 `40` Design Unit이다.
- Objective Edge Marker가 이동해도 열린 면/Orientation은 **원래 Objective 방향**을 계속 나타낸다.
- 최대 이동 범위 안에서도 공간이 부족하면 Objective의 Distance Text를 먼저 제거하고 Icon-only Compact 상태로 유지한다.
- P0에는 Primary Objective 1개만 있으므로 `+N` Edge Marker Stack이나 자동 Carousel을 만들지 않는다.

**Reticle / Target / Objective 시각 우선순위**

```text
Command/Turret Reticle + Immediate Fire State
→ 조준/발사 판단 최우선

Selected Target + Lock
→ 전투 대상 선택/Fire-Control

Primary Objective Marker
→ 임무 Navigation

Future Secondary Objective Marker
→ 현재 P0 미사용
```

Objective Marker와 Reticle이 겹쳐도 Aim/Lock/Fire 의미를 만들지 않는다. Objective 위치와 Command Aim이 일치하는 것은 공간적 우연 또는 플레이어 조준 결과일 뿐이다.

**Radar / Sensor 경계**

- Objective World Marker가 존재한다고 Radar Contact를 자동 생성하지 않는다.
- Radar는 계속 Sensor Provider Contact만 소비한다. Sensor 범위 밖 Mission Objective를 Mission Marker 때문에 Radar Hostile/Friendly/Unknown Blip로 위장하지 않는다.
- 후속에 Mission Navigation Overlay를 Radar에 추가해야 한다면 Contact Blip과 다른 별도 Mission Overlay Channel/Shape로 설계한다. 현재 P0 Radar 계약에는 포함하지 않는다.

**Objective 수명 / 완료·실패**

- Objective `Active / Completed / Failed` 판정과 Primary 전환은 Mission Provider가 소유한다.
- Completed/Failed 순간 Marker에 짧은 1회 Brightness/State 전환을 사용할 수 있지만 **얼마나 유지한 뒤 제거할지는 Provider 수명**을 따른다. Widget Timer로 임의 Hold를 만들지 않는다.
- 다음 Objective를 UI가 자동 거리순으로 선택하지 않는다. Provider가 새 Primary Objective를 제공할 때만 Marker가 전환된다.
- Mission 전체 Result는 기존 Outcome/Result Flow가 소유하며 Marker 자체가 `MISSION COMPLETE`, `MISSION FAILED` 중앙 Banner를 만들지 않는다.
- World Pause 중 Mission Game Time과 Objective Lifecycle은 정지한다. Projection Widget이 별도 UI Tick을 사용하더라도 Objective Timer/진행을 Real Time으로 진행시키지 않는다.

**Player Destroyed / Spectate 경계**

- Player Vehicle Destroyed만으로 Objective를 `Failed` 처리하거나 Marker를 즉시 삭제하지 않는다.
- 상위 Player/Outcome Flow가 `bShowCombatHUD=false` 또는 동등한 Terminal HUD 종료를 제공하면 Objective World Marker도 Reticle/Game Navigation 표현과 함께 Collapse한다.
- 향후 Spectate에서 Mission Navigation을 유지해야 하면 Spectate 전용 표시 정책을 별도 Provider가 제공해야 하며 Destroyed Combat HUD 값을 그대로 재사용하지 않는다.

**사운드 / Motion**

- Objective 생성, 화면 밖 전환, 도착, Completed/Failed와 Primary 변경에 CF-PDL-0009에 따라 게임/UI 사운드를 사용하지 않는다.
- 평상시 Objective Marker는 Pulse·회전·Breathing Glow를 사용하지 않는다.
- Primary 변경 또는 Provider가 명시한 완료/실패 사건에서만 짧은 1회 Brightness/Opacity 전환을 허용한다.

##### Selected Target On-screen / Off-screen / BehindCamera

World Marker는 Camera View Space 기반으로 상태를 나눈다.

```text
OnScreen
→ 정상 4-Corner Target Bracket

OffScreen
→ Safe Region 외곽 2-Corner Open Edge Bracket

BehindCamera
→ 투영 좌표를 억지 Clamp하지 않고 View 방향을 Edge 방향으로 변환
→ 동일 2-Corner Edge Bracket
```

Off-screen 기본 Edge Marker:

```text
EdgeBracketBounds          = 32 × 24
EdgeCornerLength           = 9
EdgeBracketStroke          = 2
EdgeSafeInset              = Layout/SafeRegion Token
```

- 기본 형태는 Radar Off-range Selected Target과 같은 **2-Corner Open Selection Language**를 재사용해 학습 비용을 낮춘다.
- 독립 삼각 Arrow를 사용해 Hostile `▼`와 혼동시키지 않는다.
- Edge Bracket의 열린 방향이 실제 Target 방향을 향하게 Orientation을 갱신한다.
- 방향에 따라 Bracket 크기나 길이를 거리 비례로 변화시키지 않는다.
- Live Tracking Distance가 실제 제공될 경우 한 줄 짧은 거리값을 보조할 수 있지만 이름·Intel을 가장자리에 누적하지 않는다.

##### Occluded / Last Known / Contact Lost / Destroyed

- `Occluded`와 `Sensor Lost`를 Widget이 같은 값으로 취급하지 않는다.
- 기본 HUD는 Provider 허가 없이 벽 뒤 실제 Actor 위치를 정확히 따라가는 X-ray Bracket을 생성하지 않는다.
- Sensor/Tracking 계약이 `Last Known`으로 전환되면 마지막 유효 World Position에 **Ghosted Target Marker**를 유지할 수 있으며 위치를 속도로 외삽하지 않는다.
- Last Known World Marker는 기존 Target/Radar 계약처럼 동일 Selection 실루엣을 유지하고 Opacity/Brightness를 낮춘다. 필요하면 작은 `LAST` 상태 Text를 한 번만 추가하고 긴 설명을 붙이지 않는다.
- 같은 Contact를 Live로 재획득하면 Marker는 별도 Target Selected 연출 없이 정상 강도로 복귀한다.
- `Contact Lost`에서는 World/Edge Marker를 모두 제거하며 화면 가장자리에 유령 Arrow를 남기지 않는다.
- Destroyed Kill Confirmation Hold 동안에는 Target Marker를 낮은 시각 강도로 유지할 수 있지만 X·해골·폭발 Icon으로 교체하지 않는다. Hold 종료 시 Marker를 제거한다.

##### Lock Indicator — 미래 Fire-Control 시각 계약

현재 실제 소스에는 별도 Lock Runtime이 확인되지 않았으므로 **Lock Provider가 존재할 때만** 아래 표현을 활성화한다.

Lock은 Command Reticle이 아니라 **Selected World Target Marker 바깥의 독립 4-Corner Lock Segment**로 표시한다.

Acquiring 기본 동작:

```text
Start Outer Offset         = 18
Locked Outer Offset        = 6
LockSegmentLength          = 8
LockSegmentStroke          = 2
```

- `NormalizedAcquireProgress=0→1`이 실제 제공되면 네 Segment가 Target Marker 바깥에서 최종 Lock 위치로 점진적으로 좁혀진다.
- Segment 자체를 계속 회전시키지 않는다. Lock 진행은 **수렴하는 Geometry**로 읽힌다.
- 진행 중에는 실제 Provider 값이 있을 때만 `LOCK 62%`처럼 Compact Percent를 표시한다.
- Provider가 Progress를 제공하지 않으면 시간으로 가짜 Percent를 만들지 않는다.

Locked:

- 네 Lock Segment가 최종 Offset에 정착한다.
- `LOCK` 짧은 Text 또는 동등한 비색상 상태를 함께 제공한다.
- 1회 Brightness Highlight 후 안정적으로 고정하고 반복 Pulse·회전·Blink를 사용하지 않는다.
- 관계색을 Lock 색으로 덮지 않고 `AccentTactical / TextPrimary` 계층을 사용한다.

Blocked / Lost:

- 실제 Provider가 Blocked를 제공하면 Progress를 멈춘 척 유지하지 않고 `LOCK BLOCKED` 또는 Provider의 짧은 Player-facing Reason을 표시한다.
- `Lost`가 실제 사건으로 제공되면 Lock Segment만 짧게 바깥쪽 Fade Out하고 Target Selection Bracket은 그대로 유지할 수 있다.
- Lock Lost를 이유로 Target Panel, Radar, Alert Feed에 동일 메시지를 자동 중복 생성하지 않는다.

공통 경계:

- Selected Target ≠ Locked Target이다.
- Reticle이 Target Marker 안에 들어왔다는 이유만으로 Acquiring/Locked를 만들지 않는다.
- ID2, Scanner Tactical Analysis 완료, Live Contact만으로 Lock을 자동 완료하지 않는다.
- Lock이 필요 없는 무기는 Lock UI 전체를 Collapse하고 상시 `N/A`를 표시하지 않는다.
- Last Known 선택 유지와 Destroyed Hold는 Lock 유효시간을 연장하지 않는다.
- Lock 완료가 Cooldown, Ammo, MuzzleBlocked, 정렬 정책 등 실제 Fire Validation을 우회하지 않는다.
- 후속 다중 Target Lock이 필요해지면 별도 Fire-Control 기능에서 설계하며 P0 단일 Selected Target Marker에 여러 Lock Ring을 중첩하지 않는다.

##### Incoming Damage Direction — 피격 방향 표시

피격 방향 표시는 **내 차량이 실제로 피해를 받은 방향을 즉시 알려주는 중앙 주변 전투 피드백**이다. Target Marker나 Radar Contact를 복제하지 않고 공격자 정체를 공개하지 않는다.

기본 형태는 전체 원을 그리지 않는 **짧은 Inward Damage Arc + 양끝 Wing**을 사용한다.

```text
             ╭━━╮


        [ Command Reticle ]
```

실제 UI는 위 ASCII처럼 고정 상단에 있는 것이 아니라 공격이 들어온 Camera-relative 방향을 따라 Reticle 주위 원주상에 배치한다.

Standard / 2560×1440 기본 Visual Token:

```text
IncomingDamageRadius          = 112
IncomingDamageArcSpanDeg      = 22
IncomingDamageStroke          = 4
IncomingDamageWingLength      = 8
IncomingDamageDuration        = 0.85 s
IncomingDamagePeakHold        = 0.16 s
IncomingDamageMergeAngleDeg   = 18
IncomingDamageMaxDirections   = 4
```

방향 문법:

```text
12시 = 현재 Camera가 보는 전방에서 온 공격
3시  = 화면 오른쪽 방향에서 온 공격
6시  = Camera 뒤쪽에서 온 공격
9시  = 화면 왼쪽 방향에서 온 공격
```

- 실제 세계 입사 방향을 차량 Yaw 기준으로 고정하지 않고 **현재 Camera View 기준 방위**로 변환해 외부 3인칭 Free Look 중에도 화면에서 직관적으로 읽히게 한다.
- 기본 원주 Angle은 공격원 방향의 수평 방위를 사용한다. Widget이 Actor 이름이나 Target Selection을 통해 공격자를 찾지 않는다.
- 수직 공격의 정확한 표현이 필요한 경우 Provider가 `Top / Bottom` 또는 동등한 Vertical Cue를 제공해야 한다. Widget이 임의 임계값으로 Top/Bottom 판정을 새로 만들지 않는다.
- Provider가 명시한 Top 공격은 12시 Arc에 짧은 `TOP` Micro Label, Bottom 공격은 6시 Arc에 `BOTTOM` Micro Label을 선택적으로 붙일 수 있다. 이 Text는 공격자 정체가 아니라 방향 의미만 제공한다.
- 유효 방향이 없으면 잘못된 12시 기본값을 만들지 않고 Direction Arc만 숨긴다. 방어층 변화와 Damage Feedback 자체는 계속 표시할 수 있다.

피해 계층별 시각 강도는 **이번 한 타격에서 실제로 도달한 가장 깊은 방어층**을 기준으로 한다.

```text
Shield까지만 감소
→ ShieldColor / 얇은 Arc

Armor까지 감소
→ ArmorColor / 기본 Arc

Integrity까지 감소
→ IntegrityColor / 더 강한 Arc

DestroyedThisHit
→ StateCritical 계열 / Integrity보다 약간 긴 1회 Fade
```

- 한 타격이 Shield를 깨고 Armor 또는 Integrity까지 통과했다면 Shield 색을 유지하지 않고 **가장 깊게 실제 감소한 계층**을 대표한다.
- 색상만으로 의미를 강제하지 않는다. Shield는 얇은 Single Arc, Armor는 기본 Arc + 짧은 Wing, Integrity는 더 굵은 Arc + 안쪽 짧은 Secondary Tick을 사용한다.
- `bShieldBrokenThisHit`, `bArmorBrokenThisHit` 같은 Break 사건은 Arc 전체를 반복 점멸시키지 않고 해당 VehiclePanel Layer의 1회 Break Highlight와 필요 시 Provider Alert가 소유한다.
- 동일 방향의 연사 피해는 매 Bullet마다 Scale Pop을 다시 재생하지 않는다. 기존 Arc의 수명을 갱신하고 짧은 Brightness Bump만 허용한다.
- 서로 다른 방향의 동시 피격은 최대 4개 Direction Arc까지 유지한다. 같은 방향으로 가까운 사건은 `IncomingDamageMergeAngleDeg` 안에서 같은 Bucket으로 합쳐 과도한 링 조각을 방지한다.
- Direction Arc 수명은 Game Time 기준이며 완전 Pause 중에는 남은 시간이 감소하지 않는다.
- 관계색 Hostile/Friendly를 Direction Arc에 사용하지 않는다. 피격 방향은 관계 판정이 아니라 실제 Damage Event를 뜻한다.
- 공격자가 미식별 상태여도 실제 피해 방향은 표시할 수 있지만 이름, Faction, Weapon ID를 중앙 피격 UI에 자동 공개하지 않는다.

##### Hit Confirmation — 공격자 적중 확인

`FireSuccess`와 Hit Confirmation은 완전히 다른 의미다.

```text
FireSuccess
= 발사 명령이 정상 승인됨

Hit Confirmation
= 내가 만든 공격이 대상의 Shield / Armor / Integrity 중 하나 이상을 실제 감소시킴
```

따라서 `FireSuccess`만으로 Hit Marker를 표시하지 않는다.

Hit Confirmation 기본 형태는 Command Reticle의 수평·수직 Bracket과 구분되는 **4개의 짧은 Diagonal Tick**이다.

```text
       ╲   ╱

         •

       ╱   ╲
```

Standard 기본 Visual Token:

```text
HitConfirmBounds              = 32 × 32
HitConfirmTickLength          = 6
HitConfirmInnerGap            = 7
HitConfirmStroke              = 2
HitConfirmDuration            = 0.24 s
DestroyedConfirmDuration      = 0.34 s
```

- Hit Marker는 Command Reticle을 이동시키거나 Target Marker에 붙지 않고 **현재 Command Reticle 중심**에서 짧게 표시한다.
- Target이 화면 밖으로 나간 뒤 Projectile이 적중해도 실제 Damage Result가 로컬 공격자의 결과로 확인되면 Hit Confirmation은 발생할 수 있다.
- World Geometry 충돌, DamageData 거부, 이미 Destroyed된 대상, 자기 피해 차단처럼 `bAppliedToAnyLayer=false`인 결과에는 Damage Hit Confirmation을 표시하지 않는다.
- 단순 Impact FX 재생을 Hit Confirmation 근거로 사용하지 않는다.

계층별 Marker 형태:

```text
Shield Hit
→ ShieldColor / Single Diagonal Tick 4개

Armor Hit
→ ArmorColor / Diagonal Tick + 짧은 Outer Cap

Integrity Hit
→ IntegrityColor / 굵은 Diagonal Tick + 작은 Center Diamond Outline

Destroyed This Hit
→ Integrity Variant + StateCritical 1회 Brightness + 조금 긴 Fade
```

- 한 타격이 여러 계층을 통과하면 Incoming Damage와 동일하게 **가장 깊게 실제 감소한 계층**을 대표한다.
- Destroyed Marker에 해골, 대형 `KILL`, 화면 중앙 Banner를 추가하지 않는다. Target Panel의 Destroyed Lifecycle과 1.25초 Kill Confirmation Hold가 대상 파괴 확인을 소유한다.
- Hit Marker를 Target Intelligence 획득으로 취급하지 않는다. Shield Hit Marker를 봤다는 이유만으로 Target의 정확 Shield 수치나 Armor 수치가 UI에서 자동 Known이 되지 않는다.
- 관찰 가능한 Shield/Armor 현상을 Knowledge로 승격할지는 Target Intelligence Provider가 별도 판단한다.

##### Floating Damage Number 정책

P0 기본 Combat HUD에는 **Floating Damage Number를 사용하지 않는다.**

이유:

- 정확한 내부 Damage 수치를 중앙 화면에 반복 노출하면 Target Intelligence의 정보 공개 계약을 우회할 수 있다.
- 고연사 무기에서 시각적 노이즈가 커지고 Command/Turret/Target Marker 판독을 방해한다.
- Shield 흡수, Armor 흡수, 관통과 Integrity 적용량을 하나의 숫자로 합치면 실제 방어 구조 의미를 왜곡한다.

후속 Accessibility 또는 Arcade Preset에서 Damage Number를 요구하더라도 기본 HUD와 분리하며 Provider가 공개를 허용한 수치만 사용한다. Widget이 `BaseDamage`, ArmorPenetration 또는 내부 Runtime 결과를 직접 조합해 숫자를 만들지 않는다.

##### Local Vehicle Damage Feedback

내 차량의 방어층 변화는 VehiclePanel과 중앙 Damage Feedback이 역할을 나눠 표시한다.

```text
Incoming Damage Direction
→ 공격이 어디에서 왔는지

VehiclePanel
→ 어느 방어층이 실제 감소했는지

Alert Feed
→ 즉시 대응이 필요한 Break / Critical 사건만
```

VehiclePanel 반응:

- Shield가 실제 감소하면 Shield Bar의 Fill/Edge에 `MotionState` 계열 1회 Brightness Highlight를 사용한다.
- Armor가 실제 감소하면 `ArmorDirection`에 해당하는 Plate/Badge와 세로 Bar만 1회 Highlight한다. 다른 다섯 방향을 함께 Flash하지 않는다.
- Integrity가 실제 감소하면 Integrity Bar를 1회 강하게 Highlight한다.
- 한 타격이 여러 계층을 통과하면 실제 감소한 각 Layer를 같은 사건 안에서 함께 Highlight할 수 있으나 반복 Bounce·Shake를 사용하지 않는다.
- 수치 변화 Animation은 실제 Before/After View Data를 사용하며 Widget이 피해량을 다시 계산하지 않는다.

Integrity Damage 전용 Peripheral Feedback:

```text
IntegrityDamageVignetteDuration = 0.26 s
IntegrityDamageVignettePeakAlpha = 0.14
```

- Shield/Armor만 감소한 일반 타격에는 화면 전체 Damage Vignette를 사용하지 않는다.
- Vehicle Integrity가 실제 감소할 때만 매우 짧고 낮은 Alpha의 주변부 Vignette를 허용한다.
- 화면 중앙을 흰색/빨간색으로 덮는 Full-screen Flash, 장시간 Blur, 반복 Chromatic 효과는 기본 사용하지 않는다.
- 파괴 전환 뒤의 화면 연출은 후속 Death/Destroyed Player Flow가 소유하며 Integrity Vignette를 무한 반복하지 않는다.

UI 범위 밖:

- Camera Shake
- Controller Rumble
- Post Process Damage Effect
- World Impact FX
- Physics Impulse

위 요소는 후속 Gameplay/Feedback 기능이 소유하며 CF-FQ-032 Widget이 직접 실행하지 않는다.

##### Damage Feedback와 Alert 중복 규칙

일반 타격마다 Alert Feed에 메시지를 생성하지 않는다.

```text
일반 Shield Hit
→ Direction Arc + VehiclePanel Shield Highlight

일반 Armor Hit
→ Direction Arc + 해당 Armor Plate Highlight

일반 Integrity Hit
→ Direction Arc + Integrity Highlight + 짧은 Peripheral Vignette

Shield Broken
→ 위 기본 피드백 + Provider가 실제 Warning Alert를 발행할 때만 SHIELD DEPLETED

Armor Broken
→ 해당 Armor Plate Break 상태 + Provider가 전술적으로 중요하다고 발행한 경우에만 Alert

Integrity Critical
→ VehiclePanel + Provider Critical Alert
```

같은 Damage Event를 Reticle Text, Alert Feed, Target Panel과 VehiclePanel에 동일 문장으로 반복하지 않는다.

##### Reticle Layer Global Declutter / 중앙 시야 보호

Reticle Layer의 개별 요소가 모두 정상 동작하더라도 **동시에 모두 Full 표현으로 그리면 중앙 전투 시야가 다시 복잡해질 수 있다.** 따라서 P0는 Widget별 임의 숨김이 아니라 하나의 공통 Presentation Declutter 규칙을 사용한다.

핵심 원칙:

```text
Gameplay / Provider State
→ 그대로 보존

Projection / Presentation
→ 충돌과 시야 점유만 조정

Declutter
≠ Target 해제
≠ Lock 해제
≠ Objective 변경
≠ Damage Event 삭제
```

Declutter는 Gameplay 권한을 바꾸지 않는다. 화면에서 보조 Text를 잠시 숨기거나 Objective를 Shape-only로 축약해도 Target Selection, Mission Primary, Lock State, Damage Result와 Knowledge는 그대로 유지한다.

**P0 동시 표시 구조 상한**

현재 개별 계약을 합치면 Reticle Layer가 직접 다루는 독립 의미는 다음으로 제한된다.

```text
Command Reticle             1
Turret Reticle              1
Selected Target Anchor      최대 1
Primary Objective Anchor    최대 1
Lock Indicator              최대 1 / Selected Target에 부착
Immediate Fire State        최대 1 대표 상태
Hit Confirmation            최대 1 현재 Marker State
Incoming Damage Direction   최대 4 Angle Bucket
```

- Target Candidate 전체, Radar Contact 전체를 World Marker로 증식시키지 않는다.
- Secondary Objective World Marker는 현재 P0에 없다.
- Multi-lock은 현재 P0에 없다.
- Floating Damage Number는 현재 P0에 없다.
- 향후 위 항목이 추가될 때 기존 화면에 조용히 더 얹지 않고 별도 Density/Declutter Gate에서 다시 검토한다.

**Command Reticle 기준 보호 구역**

중앙 보호는 Viewport 정중앙 고정 원이 아니라 **현재 Command Reticle의 실제 Screen Position**을 기준으로 한다. 기존 Camera Aim 결과가 움직이면 보호 구역도 같이 이동한다.

Standard / 2560×1440 기본 Token:

```text
DeclutterAimCoreRadius          = 56
DeclutterCombatRingRadius       = 136
DeclutterStatusLaneWidth        = 144
DeclutterStatusLaneHeight       = 24
DeclutterStatusLaneGap          = 8
DeclutterCollisionPadding       = 8
DeclutterReleasePadding         = 16
DeclutterAuxFadeDuration        = 0.10 s
DeclutterObjectiveCoreOpacity   = 0.72
DeclutterCenterAuxLineBudget    = 1
```

수치는 Style/Density Data에서 조정 가능한 기본 Preset이며 Widget Literal로 고정하지 않는다.

구역 의미:

```text
Aim Core / R <= 56
→ 조준 핵심 도형 영역
→ Command / Turret / Hit Confirm
→ 같은 위치의 Selected Target·Lock Open Geometry는 허용
→ 자유로운 보조 Text·Objective Distance는 금지

Combat Ring / 56 < R <= 136
→ Incoming Damage Arc와 Target/Lock이 교차할 수 있는 전투 정보 영역
→ Marker 보조 Text는 최대 1줄
→ Objective Distance는 기본 축약 대상

Outer Marker Field / R > 136
→ Target·Objective의 정상 Full/Compact 표현 가능

Edge Band
→ Safe Region 경계의 Target/Objectve Edge Marker 전용
```

Aim Core와 Combat Ring은 **Hard Occlusion Mask가 아니다.** Selected Target의 Open Bracket이나 실제 Objective Anchor가 그 안에 들어왔다는 이유로 월드 위치를 거짓 이동시키지 않는다. 대신 낮은 우선순위 Text와 장식부터 제거한다.

**표현 단계**

공통 Declutter는 다음 단계만 사용한다.

```text
Full
→ 고유 Shape + 허용된 보조 Text

Compact
→ 고유 Shape + 가장 중요한 보조 정보 최대 1개

ShapeOnly
→ 고유 Shape만 유지

AttachedBadge
→ 신뢰 가능한 동일 Entity 결합 시 보조 의미를 상위 Marker에 부착

CollapsedBySource
→ Provider/Gameplay가 실제 표시 불가를 제공한 경우만 전체 제거
```

- 공간 부족만으로 Command Reticle, Turret Reticle, Selected Target의 핵심 Selection Shape를 `CollapsedBySource`로 만들지 않는다.
- Objective Primary Shape도 단순 혼잡 때문에 완전히 지우는 대신 먼저 `Full → Compact → ShapeOnly` 순으로 축약한다.
- Objective와 Selected Target이 동일 Entity면 기존 계약대로 `AttachedBadge`가 정상 최종 상태이며 두 Full Marker를 억지 유지하지 않는다.

**고정 의미 우선순위**

충돌 해결 우선순위는 Gameplay 위협도를 매 프레임 계산해서 만들지 않고 Semantic Role로 고정한다.

```text
1. Command Reticle / Turret Reticle
   → 조준 권한 표현

2. Immediate Fire State
   → 지금 발사 행동 가능 여부의 대표 상태

3. Hit Confirmation / Incoming Damage Direction
   → 짧은 실제 Combat Event

4. Selected Target / Lock
   → 선택과 Fire-Control

5. Primary Objective
   → Mission Navigation
```

- 이 순서는 Gameplay 우선순위나 Target Threat Score가 아니다.
- Hit/Incoming은 높은 시각 순위를 가지지만 **수명이 짧은 Overlay**이므로 이들이 발생할 때 Target/Objective Anchor를 옆으로 밀어내지 않는다.
- Persistent Marker의 위치 재배치는 Persistent Marker끼리의 충돌과 Safe Region 충돌에서만 수행한다.

**Command Status Lane 독점**

기존 `Immediate Fire State` 한 줄은 Command Reticle 아래의 Status Lane을 독점한다.

```text
Command Reticle
↓ 8
[ BLOCKED / ALIGNING / COOLDOWN 0.8 / NO WEAPON ]
```

- 이 Lane에 Target Distance, Objective Distance, Lock Progress를 함께 쌓지 않는다.
- World Marker 보조 Text가 Status Lane Bounds와 충돌하면 해당 World Marker Text가 먼저 축약된다.
- Immediate Fire State가 없다고 Objective Text를 이 Lane으로 끌어와 채우지 않는다. 의미별 Anchor 위치를 유지한다.

**Selected Target / Lock 보조 정보 예산**

Selected Target은 핵심 4-Corner Shape를 유지하고, Marker 주변 보조 Text는 한 번에 **최대 1줄**만 직접 표시한다.

기본 우선순위:

```text
실제 Lock Blocked / Failure Reason
→ 실제 Lock Acquire/Locked 상태·Progress
→ LAST / DESTROYED 같은 현재 Target Marker 상태
→ Target Distance
```

- Lock Runtime이 없으면 Lock 항목 전체가 존재하지 않는다.
- Lock Segment Geometry 자체는 Target Shape 바깥에서 유지할 수 있지만 `LOCK 62%`와 `842 m`를 같은 좁은 영역에 동시에 쌓지 않는다.
- Last Known/Destroyed 의미가 Target Panel에서 충분히 전달되고 Marker 공간이 좁으면 World Marker의 상태 Text를 생략할 수 있지만 Ghost/Terminal Shape 의미는 유지한다.
- Target Distance가 숨겨져도 Target Panel/Radar의 실제 데이터와 Tracking State를 삭제하지 않는다.

**Objective 보조 정보 예산**

Objective는 Target보다 낮은 중앙 시각 우선순위를 가진다.

```text
Full
→ Open Hex + 선택적 Distance

Compact
→ Open Hex + 필요한 경우 짧은 Distance 1개

ShapeOnly
→ Open Hex만
```

- Objective가 Aim Core 안에 들어오면 기본 `ShapeOnly`를 사용하고 `DeclutterObjectiveCoreOpacity`로 한 단계 낮은 시각 강도를 적용할 수 있다.
- Combat Ring 안에서는 Selected Target/Lock 보조 Text가 이미 존재하면 Objective Distance를 숨긴다.
- 실제 Mission 위치를 숨기는 것이 아니라 **거리 Text만 줄이는 것**이다.
- 같은 Entity의 Selected Target과 결합된 Objective Badge에는 별도 Objective Distance를 다시 붙이지 않는다.

**중앙 보조 Text 총량**

Command Status Lane을 제외한 Combat Ring 안의 World Marker 보조 Text는 기본 `DeclutterCenterAuxLineBudget=1`을 사용한다.

예:

```text
Target LOCK BLOCKED 표시 중
→ Objective Distance 숨김
→ Target Distance 숨김

Target Distance만 표시 중
→ Objective가 Combat Ring 밖이면 Objective Distance 별도 가능
→ Objective가 Combat Ring 안이면 Objective Distance 숨김
```

Hit Confirmation에는 Text를 추가하지 않는다. Floating Damage Number도 추가하지 않는다.

Incoming Damage의 `TOP / BOTTOM` Micro Label은 여러 Direction Arc가 동시에 살아 있어도 **최대 1개만** 표시한다. 여러 Vertical Cue 후보가 겹치면 가장 깊은 실제 피해 Layer를 우선하고, 같은 Layer면 가장 최근 Event를 사용한다. Arc 자체의 최대 4 Direction 정보는 그대로 유지한다.

**Transient Feedback는 Persistent Layout을 흔들지 않음**

다음 사건은 짧은 Overlay이며 Persistent Marker의 Layout Resolve 입력으로 사용하지 않는다.

```text
Hit Confirmation
Incoming Damage Arc
FireSuccess 1회 Brightness
Damage Vignette
```

즉 자동사격 중 Hit Marker가 반복되거나 연속 피격을 받더라도 Objective가 좌우로 튀고 Target Distance가 매 탄마다 나타났다 사라지는 Reflow를 만들지 않는다.

Transient Feedback는 기존 Draw Order에 따라 위에 잠시 그려지고 자신의 Game Time 수명이 끝나면 사라진다.

**Persistent Marker 충돌 해결 순서**

```text
1. Provider 표시 가능 여부 확정
2. World Anchor → Screen Projection
3. Stable Identity 기반 Target+Objective 결합 여부 확정
4. Persistent HUD / Safe Region 예약 영역 적용
5. Selected Target 위치 확정 — 원래 Anchor/Bounds 유지
6. Objective 위치 확정 — 필요한 경우 기존 Edge Tangential Shift만 허용
7. Target/Lock 보조 Text 예산 적용
8. Objective 보조 Text를 Full → Compact → ShapeOnly로 축약
9. Transient Hit/Incoming Overlay 적용
10. 최종 Draw Order 적용
```

- Screen Bounds가 우연히 겹친다는 이유로 Identity 결합을 하지 않는다.
- World Anchor를 다른 Actor처럼 보이게 옮기는 임의 Screen Offset은 사용하지 않는다.
- On-screen Objective는 중앙 충돌 때문에 Target처럼 Bracket으로 변형하지 않는다.

**Persistent HUD Panel과의 관계**

Target/Objective World Marker는 기존 Navigation Safe Region으로 MissionSummary·AlertFeed·TargetPanel·VehiclePanel·RadarPanel·WeaponPanel을 피한다.

Command/Turret Reticle은 Gameplay Aim 결과이므로 Panel 충돌 회피를 위해 Clamp하거나 옆으로 이동시키지 않는다. 현재 승인 Layout이 중앙 조준 영역을 비워 두는 이유가 이것이다.

향후 Camera/Aim 설계가 Command Reticle을 Persistent Panel 영역까지 이동시킬 수 있게 바뀐다면 Reticle을 Clamp하는 임시 해결보다 **HUD Layout Profile 자체를 다시 Review**한다.

**Edge Density**

P0 Edge Band에 존재 가능한 독립 Navigation Marker는 Selected Target 최대 1 + Primary Objective 최대 1이다.

- 기존 규칙대로 Selected Target Edge Marker는 원래 Ray 교차 위치를 유지한다.
- Objective는 같은 Edge에서 기본 40 Design Unit 간격을 목표로 최대 56까지 Tangential Shift한다.
- 그래도 충돌하면 Objective Distance를 제거하고 Objective ShapeOnly를 유지한다.
- Edge에 `+N`, Stack Bubble, Carousel을 만들지 않는다.
- 향후 Secondary Objective나 Multi-target Edge Marker가 생기면 이 2개 Marker 계약을 그대로 확장하지 않고 별도 Density Gate를 수행한다.

**Declutter Hysteresis / 시각 안정성**

Marker가 경계에서 한두 Pixel 움직일 때 Full/Compact 상태가 매 프레임 토글되지 않게 한다.

```text
충돌 진입 판정
→ Bounds + DeclutterCollisionPadding(8)

충돌 해제 판정
→ 충분히 분리될 때까지 DeclutterReleasePadding(16) 적용
```

- 상태가 바뀔 때 `DeclutterAuxFadeDuration=0.10 s`의 짧은 Opacity 전환을 허용한다.
- Scale Pop, Bounce, 위치 Spring은 사용하지 않는다.
- Declutter 전환은 Marker Gameplay 상태 변화가 아니므로 Alert, Sound, Haptic을 발생시키지 않는다.
- CF-PDL-0009에 따라 별도 UI Sound도 없다.

**Density / Accessibility 경계**

Compact/Standard/Expanded UI Density가 바뀌어도 Semantic 우선순위와 Core 보호 규칙은 유지한다.

- Compact는 Padding·보조 Text·Marker Secondary Detail을 더 일찍 축약할 수 있다.
- Expanded는 여유 공간에서 보조 Text를 더 오래 유지할 수 있지만 Aim Core에 긴 Text를 허용하지 않는다.
- 사용자 HUD Scale이 커져 충돌이 늘어날 경우 Gameplay 정보를 삭제하지 않고 같은 단계적 축약 규칙을 적용한다.
- 후속 Accessibility 옵션에서 Marker 크기를 키우더라도 Target/Objective 모양 의미와 Command Status Lane 독점 규칙을 보존한다.

**Runtime 구현 경계**

이번 Declutter 계약은 UI-P0-03 이후 Presentation Layer에서 구현할 **공통 Spatial Resolve 정책**이며 현재 D1-11 Production HUD에 Runtime Binding을 추가하지 않는다.

```text
각 Widget이 독립 Tick으로 서로 Bounds 조회
→ 금지

공통 Projection / Presentation Resolver
→ 한 프레임의 World Marker Screen Bounds 수집
→ Stable Identity Merge
→ Safe Region / Declutter Resolve
→ 각 Widget에 Resolved Presentation 전달
```

정확한 C++ 클래스명은 UI-P0-03 구현 시 확정한다. Widget이 TargetSelect, Mission Provider, Damage Component를 직접 조회해 자신의 우선순위를 계산하지 않는다.

##### Player Integrity Critical — 파괴 전 경고 상태

`Integrity Critical`은 `Destroyed`가 아니며 아직 플레이어가 전투·주행을 계속할 수 있는 **생존 중 위험 상태**다. Widget이 Integrity Ratio를 보고 임계값을 독립 판정하지 않고 Player/Vehicle Presentation Provider가 Critical 상태를 명시적으로 제공할 때만 아래 시각을 사용한다.

- VehiclePanel의 Integrity Bar는 현재 Fill을 그대로 유지하고 `StateCritical` 계열 Edge/Accent를 추가한다. 남은 값이 존재하는데 Bar를 0처럼 비우지 않는다.
- Critical 진입 순간 `MotionAlert`보다 작은 1회 Brightness 강조를 허용하지만 VehiclePanel 전체를 지속 점멸시키지 않는다.
- 지속 상태에서는 Integrity Label/Icon 또는 Bar Edge의 제한된 `MotionCriticalPulse`만 허용한다. 숫자 Text 자체는 안정적으로 고정한다.
- Alert Feed는 Provider가 `INTEGRITY CRITICAL`을 Critical/Persistent 사건으로 제공한 경우 표시할 수 있다. 같은 상태가 유지되는 동안 매 Hit마다 새 Alert를 만들지 않는다.
- Command/Turret Reticle, Target Marker, Radar와 WeaponPanel은 Critical만으로 숨기거나 Disabled 처리하지 않는다. `Critical`은 아직 `Destroyed`가 아니다.
- 화면 전체 Red Wash, 지속 Blur, 반복 Full-screen Flash를 Critical 상시 표현으로 사용하지 않는다.
- Critical 상태가 회복되면 Provider의 Clear에 따라 VehiclePanel Accent와 Alert를 해제한다. Widget이 자체 Ratio Hysteresis를 계산하지 않는다.

##### Player Vehicle Destroyed — Terminal HUD 전환

플레이어 차량의 실제 `Destroyed`가 Player/Vehicle Lifecycle Provider에서 확정되면 Combat HUD는 **Terminal Presentation**으로 전환할 수 있다. 단, 시각 전환은 실제 Gameplay의 Combat/Input Availability와 같은 권한 상태를 소비해야 하며 `VehicleHealthComp.bDestroyed`만 보고 Widget이 독자적으로 플레이어 조작을 막은 것처럼 연출하지 않는다.

기본 Terminal 시각 순서:

```text
Final Integrity Damage Event
→ 기존 Incoming Damage Arc / Integrity Vignette의 현재 1회 연출은 끝까지 허용
→ INTEGRITY CRITICAL Persistent Alert는 종료
→ VehiclePanel을 DESTROYED Terminal State로 전환
→ 실제 Combat Availability=false가 함께 제공되면 Reticle/Lock/Target Marker/Weapon/Radar를 짧게 Fade
→ 상위 Outcome Flow가 결정될 때까지 Destroyed Terminal 상태 유지
→ Respawn / Spectate / Result 중 실제 Provider 상태로 전환
```

**VehiclePanel Destroyed State**

- Header 또는 Integrity 영역에 **`DESTROYED`**를 명확한 Terminal State Text로 표시한다.
- 실제 Vehicle Integrity가 0으로 확정됐을 때 Integrity Bar를 빈 상태로 표시하고 `0 / Maximum`처럼 기존 Current/Maximum 문법을 유지할 수 있다. Widget이 파괴 사실만으로 내부 값까지 0으로 덮어쓰지 않는다.
- Shield와 6방향 Armor는 **파괴 직전/직후 Provider가 제공한 실제 최종값**을 유지한다. 차량이 Destroyed라는 이유로 남은 Shield·Armor를 전부 0으로 만들지 않는다.
- Armor Highlight는 마지막 실제 `ArmorDirection` 사건까지만 처리하고 Destroyed 이후 가짜 전방/전체 Plate Flash를 만들지 않는다.
- Speed, RPM과 Gear는 World View Terminal 단계에서 Provider가 계속 실제 값을 제공한다면 그대로 표시할 수 있다. 파괴 차량이 물리적으로 구르는 동안 UI가 임의로 `0 km/h`, `RPM 0`, `N`을 만들어내지 않는다.
- `DESTROYED`는 `StateCritical` Accent + Chassis Semantic Icon + 명확한 Text로 전달하고 반복 Blink, 전체 Panel Shake, 무한 Pulse를 사용하지 않는다.

Standard Terminal Token 기본값:

```text
DestroyedTerminalCardWidth        = 320
DestroyedTerminalCardHeight       = 56
DestroyedTerminalIconSize         = 20
DestroyedTerminalAccentThickness  = 4
DestroyedCombatHudFadeDuration     = 0.18 s
DestroyedTerminalEnterDuration     = 0.20 s
DestroyedTerminalOverlayAlpha      = 0.10
```

- `DestroyedTerminalCard`는 화면 중앙을 큰 Game Over Banner로 덮는 카드가 아니라 기존 중앙 보호영역 상단부에 배치할 수 있는 **짧은 상태 Plate**다.
- 기본 표시 Text는 `VEHICLE DESTROYED`를 사용한다. `DEFEAT`, `MISSION FAILED`, `GAME OVER`는 상위 Outcome Provider가 확정하지 않으면 표시하지 않는다.
- 배경 Dim은 매우 낮은 Alpha로 제한하며 파괴 FX와 월드 상황을 볼 수 있어야 한다.
- 카드 자체에 자동 Countdown, 자동 Restart Timer, Retry 버튼을 만들지 않는다.

##### Destroyed에서 Reticle / Target / Weapon / Radar 처리

실제 Gameplay Provider가 `CombatInteractionAllowed=false` 또는 동등한 Terminal 상태를 제공한 경우에만 다음 표현을 적용한다.

```text
Command Reticle
Turret Reticle
Lock Indicator
Immediate Fire State
Selected World Target Marker
Hit Confirmation
WeaponPanel
RadarPanel
→ MotionState 계열 짧은 Fade 후 Collapse 또는 비활성 표시 종료
```

- Health `Destroyed`만 보고 Reticle을 먼저 숨겨 실제 Runtime은 발사 가능한데 화면만 발사 불가처럼 보이게 만드는 상태를 금지한다.
- 기존 Target 선택 데이터를 UI가 강제로 Clear하지 않는다. Terminal Combat HUD가 숨겨져도 TargetSelect Gameplay State의 정리는 해당 Lifecycle/Gameplay 시스템이 소유한다.
- 이미 발사된 Projectile이 파괴 이후 적에게 피해를 주더라도 Terminal Player HUD에서는 새 Outgoing Hit Confirmation을 기본 표시하지 않는다. 실제 피해 판정 자체를 취소하라는 의미는 아니다.
- Destroyed 이후 새 Incoming Damage Arc는 기본 생성하지 않는다. 최초 파괴 Hit에서 시작된 짧은 Arc/Vignette는 현재 1회 수명까지 끝낼 수 있다.
- Radar/Target/Weapon의 마지막 값을 Terminal Summary로 별도 복사해 화면에 남기지 않는다.

##### Alert Feed — Destroyed 전환

- `INTEGRITY CRITICAL` Alert는 Destroyed가 확정되는 순간 Clear한다. `Critical`과 `Destroyed`를 동시에 Persistent하게 쌓지 않는다.
- 기본 HUD에서는 동일 사건을 `VEHICLE DESTROYED` Critical Alert Row로 다시 추가하지 않는다. VehiclePanel Terminal State와 `VEHICLE DESTROYED` 상태 Plate가 결과를 소유한다.
- Weapon/Target/Vehicle의 일반 Combat Alert는 Terminal Combat 상태에서 새로 활성화하지 않는다.
- Mission/System Alert가 파괴 뒤에도 의미가 있는지는 상위 Outcome/Mission Flow가 결정한다. Widget이 임의로 전부 유지하거나 전부 삭제하지 않는다.
- Result Screen이 활성화되면 인게임 Alert Feed는 기본적으로 HUD와 함께 Collapse하고 Result Screen이 필요한 결과 메시지를 별도 소유한다.

##### Player Destroyed와 Result Screen 시각 경계

`Player Vehicle Destroyed`는 **Game Result 자체가 아니다.** 같은 차량 파괴 사건 이후에도 후속 Gameplay는 Respawn, Spectate, Mission Failure, Run Result 또는 다른 상태를 선택할 수 있다.

```text
Vehicle Destroyed Terminal Plate
= 현재 플레이어 차량 상태

Result Screen
= 상위 Mission / Session / Player Outcome Flow가 확정한 전체 결과
```

- Result Screen은 `Screen` Layer의 전체 화면 UI이며 HUD 내부 Terminal Plate를 확대한 형태로 만들지 않는다.
- Result Screen이 활성화되면 Combat HUD는 Collapse하고 결과 화면이 Focus·Action을 소유한다.
- Result의 Title, Summary, Retry/Continue/Exit 같은 Action은 실제 Outcome Provider가 제공해야 한다. `VehicleHealthComp`나 Widget이 차량 파괴만 보고 자동 생성하지 않는다.
- `Destroyed → Result` 전환 시간을 Widget Timer Literal로 소유하지 않는다. 상위 Flow가 상태 전환을 발행할 때만 이동한다.
- 향후 Destroyed/Spectate Camera가 실제 Lifecycle과 연결되더라도 Camera 연출과 Result Screen 전환은 별도 상태로 유지한다.
- CarFight 전역 `CF-PDL-0009`에 따라 Integrity Critical, Destroyed Terminal과 Result 전환에도 게임/UI 사운드를 추가하지 않는다.

### 9.4 승인되지 않은 범위

현재 Wireframe의 패널 내부 예시 숫자, 게이지, 아이콘과 행 구성은 배치 이해를 위한 Placeholder다.

```text
내 차량 정보 상세
→ InGameUIVehiclePanelSpec.md v0.20.0 Visual Layout Accepted / Runtime·UMG Pending

무기창 상세
→ InGameUIWeaponPanelSpec.md v0.4.0 Compact Token Detailed Draft / User Review Pending

최종 Compact Token과 Font·Icon
→ 사용자 Review + 실제 TPS 카메라 캡처 기반 Static Review 뒤 확정
```

`CFWeaponPanel_1440p.xml`의 넓은 초기 내부 치수는 Compact Revision 최종 Pixel SSOT가 아니다.

### 9.5 HUD 내부 Draw Order

Root의 레이어 ZOrder와 별개인 HUD 내부 순서다.

```text
0  Soft Background Surface
10 Persistent Panels
18 Objective World Marker와 Objective Edge Marker
20 World Target Marker와 Off-screen Target Marker
30 Command Reticle·Weapon Reticle·Lock Indicator
34 Hit Confirmation
36 Incoming Damage Direction
40 Immediate Status와 Alert
46 Integrity Peripheral Damage Vignette
50 Critical Full Attention Accent
```

Critical Accent도 Pause·Modal 레이어 위로 올라가지 않는다.

---

## 10. 해상도 확장 규칙

### 10.1 1920×1080

- 2560×1440의 패널 크기와 내부 Geometry는 기본적으로 0.75 배율을 사용한다.
- D1-07 Phase 1 고정 HUD는 사용자 시야 확보 피드백에 따라 `HUDOuterMargin1080=32 Design Unit`, 즉 화면 실효 약 24px 외곽 Margin Override를 사용한다.
- Mission·Vehicle은 좌측, Target·Weapon은 우측으로 더 바깥쪽 배치하고 Alert·Radar는 가로 중심을 유지한 채 각각 상단·하단으로 더 바깥쪽 배치한다.
- 이 1080p Override는 후속 1440p·21:9·32:9 Layout Profile에 자동 전파하지 않고 각 확장 Review에서 재판정한다.
- 14 Design Unit Caption이 실제로 읽히지 않으면 최소 Font Size를 별도 보정한다.
- 패널을 삭제하지 않고 내부 여백과 보조 문구를 우선 축소한다.

### 10.2 3440×1440

```text
Central HUD Canvas Width = 2560
Left Extra Space = 440
Right Extra Space = 440
```

핵심 HUD는 중앙 2560 영역 안에 유지한다. 바깥 영역은 Debug, Photo Mode 또는 비필수 확장에만 사용한다.

### 10.3 5120×1440

```text
Central HUD Canvas Width = 2560
Left Extra Space = 1280
Right Extra Space = 1280
```

차량 상태·Radar·Target·Weapon Panel을 물리 화면 끝으로 이동하지 않는다. 사용자의 시선 이동 거리를 중앙 16:9 기준으로 유지한다.

### 10.4 Anchor 규칙

- `MissionSummary`와 `VehiclePanel`은 중앙 16:9 HUD Canvas의 왼쪽을 기준으로 Anchor한다.
- `TargetPanel`과 `WeaponPanel`은 중앙 16:9 HUD Canvas의 오른쪽을 기준으로 Anchor한다.
- `AlertFeed`와 `RadarPanel`은 중앙 16:9 HUD Canvas의 중심을 기준으로 Anchor한다.
- `ReticleLayer`와 World Marker는 실제 Viewport 중심과 Projection 결과를 사용한다.
- 울트라와이드에서도 핵심 Slot을 물리 화면 끝으로 이동하지 않는다.

### 10.5 전체 UI 커스터마이징 확장 경계

P0에서는 승인된 기본 배치를 사용하고 사용자 편집 화면이나 저장 기능을 구현하지 않는다. 다만 이 기능은 선택적 아이디어가 아니라 전체 UI가 따라야 하는 구조적 확장 경계다. 위치·크기·색상·밀도·표시 여부를 Widget Blueprint 내부 절대값으로만 고정하지 않는다.

권장 확장 타입:

```text
ECFUIPanelId
ECFHUDSlotId
ECFUIDensityPreset
FCFUIPanelLayout
FCFHUDSlotLayout
FCFUIUserPanelOverride
UCFUILayoutData
UCFHUDLayoutData
DA_CFUILayout_Default
DA_CFHUDLayout_Default
```

`FCFHUDSlotLayout` 권장 필드:

```text
SlotId
AnchorMinimum
AnchorMaximum
Alignment
NormalizedPosition
PixelOffset
DesiredSize
RenderScale
bVisibleByDefault
```

원칙:

- 각 주요 UI 모듈은 안정적인 `PanelId` 또는 `SlotId`를 가진다.
- 기본값은 Style·Layout·Density Data Asset이 분리해서 소유한다.
- `WBP_CFInGameHUD`와 각 Screen Root는 Slot Container만 제공하고 모듈 내부 디자인을 소유하지 않는다.
- 기능 Widget은 교체 가능한 Visual Widget Class로 제공한다.
- 후속 사용자 커스텀 배치는 기본 Profile 위에 위치·Scale·Opacity·Visibility·Density Override만 저장한다.
- Profile 버전을 두어 새 UI 모듈이나 Token이 추가돼도 기존 사용자 설정을 마이그레이션할 수 있게 한다.
- 개별 Panel 내부는 자동 Layout을 사용해 숨긴 항목의 빈 공간을 제거한다.
- Canvas 고정 좌표는 Root·Projection·Reticle·Radar Blip 등 좌표 배치가 실제 의미인 범위로 제한한다.
- 이 확장 경계는 현재 커스텀 편집 UI나 저장 기능 구현 완료를 의미하지 않는다.

---

## 11. Clean Prototype 검토 기준

### 11.1 3초 판독 테스트

플레이어가 3초 이내에 다음을 구분해야 한다.

- 현재 속도
- Shield·Armor·Integrity 중 가장 위험한 계층
- 현재 무기와 발사 가능 여부
- 선택 타겟 존재 여부
- 가장 높은 경고 등급

### 11.2 1초 즉시 반응 테스트

다음 상태는 1초 이내에 인지돼야 한다.

- Critical Integrity
- 발사 불가
- Lock 완료·상실
- 화면 밖 적대 Contact
- Pause Menu Primary Button

### 11.3 정적 캡처 검토

- Center Reticle Protection 영역에 큰 패널이 없는가
- 모든 패널이 같은 Outline·Padding·Header 규칙을 사용하는가
- Cyan Accent가 화면 전체에 과도하게 퍼지지 않는가
- Amber와 Red가 실제 상태 의미에만 사용되는가
- 숫자 폭 변화로 레이아웃이 흔들리지 않는가

### 11.4 Static Review 대상 해상도

| Review ID | Viewport | 화면비 | 중앙 HUD Canvas | 현재 역할 |
|---|---:|---:|---:|---|
| `SR-1080-16` | 1920×1080 | 16:9 | 1920×1080 | **D1-07 Phase 1 Active / Geometry 0.75 + Typography 유효 하한** |
| `SR-1440-16` | 2560×1440 | 16:9 | 2560×1440 | Deferred Expansion Reference |
| `SR-1440-21` | 3440×1440 | 약 21:9 | 중앙 2560×1440 | Deferred Expansion Reference |
| `SR-1440-32` | 5120×1440 | 32:9 | 중앙 2560×1440 | Deferred Expansion Reference |

D1-07 Phase 1 사용자 PASS/FAIL Gate는 `SR-1080-16` 하나다. 준비된 1440p·울트라와이드 Mockup은 삭제하지 않고 후속 Layout Profile 확장 참고자료로 보존한다. 21:9·32:9 확장 시에도 지속 HUD의 기준 좌표는 중앙 2560 Canvas 안에 유지한다.

### 11.5 Phase 1 필수 Static Review 장면

현재 Phase 1은 `SR-1080-16`의 Combat Busy를 대표 화면으로 먼저 검토한다. 1080p 기본 HUD가 안정되면 같은 1920×1080 기준에서 아래 상태를 추가 확인하며, 후속 해상도에서는 Layout Profile 확장 시 같은 상태 의미를 재사용한다.

```text
A. Normal Driving
- VehiclePanel 정상
- WeaponPanel 최소 상태
- Target 없음
- Alert 없음

B. Combat Busy
- Target 선택
- WeaponPanel Primary + Secondary 2개 + Rail 3개
- Warning 2개
- Radar Contact 다수

C. Critical
- Integrity Critical
- Weapon 발사 불가
- Critical Alert 1개
- Target 선택 유지

D. Sparse Provider
- Weapon Ammo·Heat·Reload 미지원
- Compact Rail 없음
- 미지원 행이 모두 Collapse

E. Long Text
- 긴 한글 무기명·타겟명
- 4자리 이상 자원 수치
- 10초 미만 소수점 시간
```

실제 Runtime이 아직 없는 데이터는 Mock Static View Data로 캡처할 수 있지만, 이는 Gameplay 구현 증거가 아니라 시각 검토용으로만 기록한다.

### 11.6 해상도별 합격 기준

#### 1920×1080

- 주요 텍스트가 §4.4의 유효 글자 크기 하한 아래로 내려가지 않는다.
- 중앙 Reticle Protection 영역에 고정 패널이 침범하지 않는다.
- 고정 HUD 패널은 Phase 1 기준 화면 외곽 약 24px까지 배치해 패널 크기를 줄이지 않고 중앙 전투 시야를 우선 확보한다.
- VehiclePanel·RadarPanel·WeaponPanel 사이에 시각적 중첩이 없다.
- Caption을 숨겨도 Primary 값, Fire State, Critical 상태와 Target 핵심 정보는 유지된다.
- Interactive Menu의 실효 Hit Area는 최소 약 44×44px을 목표로 한다.

#### Deferred Expansion — 2560×1440

- Style Token의 기준 Design Unit을 그대로 검토한다.
- 패널 외곽 Safe Margin 64 기준을 유지한다.
- 숫자 자리수 변화, `???`, `N/A` 전환으로 Panel Width가 불필요하게 흔들리지 않는다.
- Standard와 Compact Density가 같은 정보 의미를 유지한다.

#### Deferred Expansion — 3440×1440

- 중앙 HUD Canvas의 시작 X는 440, 종료 X는 3000 기준이다.
- 지속 HUD를 물리 화면 최좌·최우측으로 이동하지 않는다.
- 좌우 추가 영역은 비필수 정보 또는 빈 공간으로 남겨도 된다.
- World Marker는 실제 Viewport Projection을 사용하므로 중앙 Canvas에 인위적으로 Clamp하지 않는다.

#### Deferred Expansion — 5120×1440

- 중앙 HUD Canvas의 시작 X는 1280, 종료 X는 3840 기준이다.
- Persistent Panel의 시선 이동 거리는 16:9 배치와 동일해야 한다.
- Reticle·Target World Marker는 전체 Viewport Projection을 사용하되 고정 정보 Panel은 중앙 Canvas를 유지한다.
- 극단적 가로폭 때문에 Radar·Weapon Rail·Alert 개수를 늘리지 않는다.

### 11.7 Density Profile Static Review

Density Profile 3종 비교는 D1-07 Phase 1 Gate를 차단하지 않는 후속 검토로 둔다. 확장 검토를 시작할 때 2560×1440에서 최소 다음 세 장을 동일 Gameplay 상태로 비교한다.

```text
Compact
Standard
Expanded
```

PASS 조건:

- 같은 View Data에서 값과 의미가 달라지지 않는다.
- Compact에서 Caption·여백이 줄어도 행동 차단 이유와 Critical 정보가 사라지지 않는다.
- Expanded가 정보를 추가 표시하더라도 Gameplay Runtime에 새 값을 요구하지 않는다.
- Density 전환 후 Panel의 Anchor·ZOrder·중앙 보호영역 계약이 바뀌지 않는다.
- Collapsed 항목의 빈 공간이 남지 않는다.

### 11.8 전역 Fail 조건

다음 중 하나라도 발생하면 Static Review는 FAIL이다.

- 텍스트 잘림으로 핵심 값·상태를 판독할 수 없음
- 한 줄 고정 항목이 두 줄로 늘어나 Panel 높이를 예측 불가능하게 변경
- Reticle Protection 영역 침범
- Vehicle·Radar·Weapon·Target Panel 상호 중첩
- `Unknown`, `Unavailable`, `KnownZero`가 같은 표현으로 보임
- Cyan·Amber·Red가 상태 의미와 무관하게 장식 목적으로 과다 사용됨
- Density 전환으로 버튼 Hit Area가 최소 기준 아래로 감소
- 21:9·32:9에서 지속 HUD가 물리 화면 끝으로 이동
- 미지원 채널을 Collapse했는데도 고정 Spacer가 남아 빈 블록 생성
- WeaponPanel이 Compact 기준보다 초기 넓은 Wireframe 크기로 되돌아감

---

## 12. 구현 순서 제안

```text
D1-01 Visual Direction Review                         Accepted
→ D1-02 TPS 16:9 HUD Placement Review                Accepted
→ D1-03 VehiclePanel Visual Layout Review             Accepted
→ D1-04 WeaponPanel Compact Direction Review          Accepted
→ D1-05 Style·Base Widget·Density·Compact Token      User Accepted
→ D1-06 Font·Icon Default Preset                      User Accepted
→ D1-07 Phase 1 SR-1080-16 Static Review             USER PASS / 후속 해상도 Deferred
→ D1-08 C++ Style Data Type 구현                     PASS / Official Build·Automation
→ D1-09A Asset Binding·Layout·Density C++ 보완        PASS / Official Build·Automation
→ D1-09B 1080p Style·Density·Layout·Font·Icon Asset  PASS / Asset·License·Config Readback·Regression
→ D1-10 Base Widget 제작                              D1-10A PASS / D1-10B DLL Link Blocked / D1-10 NOT PASS
→ D1-11 WBP_CFInGameHUD Visual Prototype              Prepared / D1-10 PASS 뒤 착수
→ D1-12 Pause Menu Visual Migration                   Prepared / D1-11 PASS 뒤 착수
→ UI-DESIGN-GATE PASS 판정
```

v0.9.0에서 `D1-05` 기본 Token 사용자 승인을 완료했다. 승인값은 외부 설정 가능한 기본 Preset으로 유지하며 Source·Unreal Asset 제작과 실제 Static Capture는 아직 시작하지 않는다.

UI-P0-03~05 데이터·수명 작업은 시각 자산 제작과 독립적으로 진행할 수 있지만, UI-P0-06 신규 차량·무기 HUD 시각 구현 전에는 D1-05~07의 사용자 검토와 정적 검증 결과를 확보해야 한다.

### 12.1 D1-09~12 구현 준비 잠금

2026-08-10 D1-08V·D1-09A에 이어 D1-09B까지 실제 구현·검증 PASS했다. 현재 다음 허용 단계는 `InGameUIAssetizationSpec.md v0.3.0`의 D1-10이며, 이 범위를 벗어나는 `UI-P0-03~09`, 21:9·32:9 확장, 신규 Gameplay 기능은 자동 착수하지 않는다.

#### 12.1.1 실행 순서

```text
D1-08V
- Tools\\BuildEditor.bat 공식 Editor Build
- CarFight.UI.D1_08.StyleDataContract Automation
- 둘 다 PASS해야 다음 단계 허용

D1-09A
- Font Asset Binding 타입 보완
- HUD Layout Data 타입 추가
- Density Data 타입 추가
- UCFUISubsystem 외부 설정 Soft Reference 진입점 추가
- 공식 Build + 관련 Automation PASS

D1-09B
- Pretendard / IBM Plex Mono License Eligibility PASS 기준 적용
- 원본 Copyright Notice + OFL 전문 프로젝트 내 고지 파일 보존
- Font·Semantic Icon Asset 준비
- DA_CFUIStyle_Default
- DA_CFUIDensity_Compact / Standard / Expanded
- DA_CFHUDLayout_1080_16
- DefaultGame.ini의 UI 기본 Soft Reference 연결
- Asset Readback + Reference 검증

D1-10
- 공통 Base Widget 5종 제작
- Style/Density 입력만 소비
- Gameplay Cast·판정 금지

D1-11
- WBP_CFInGameHUD Visual Prototype
- 1080p 승인 Slot과 Base Widget Placeholder만 검증
- Vehicle/Weapon/Target/Radar 실제 기능 구현 금지

D1-12
- WBP_CFPauseMenu Visual Migration
- 기존 Pause World 정지·입력·Continue 계약 보존
- Visual Class 교체와 Native Fallback 검증

→ UI-DESIGN-GATE 최종 판정
```

`D1-09A`는 별도 기능이 아니다. 현재 D1-08 구현에 실제 Font Asset 참조가 없고 `UCFHUDLayoutData`, `UCFUIDensityData`, `FCFHUDSlotLayout`도 존재하지 않아 D1-09 DataAsset을 만들 수 없는 직접 선행조건을 채우는 단계다.

#### 12.1.2 D1-09A 최소 C++ 후보

기존 이름을 무분별하게 바꾸지 않고 다음 최소 타입만 추가한다.

```text
CFUIStyleData.h / .cpp 수정
- ECFUIFontFamilyRole = UI / Numeric
- FCFUIFontAssets
  - UIFontAsset : TObjectPtr<UFont>
  - NumericFontAsset : TObjectPtr<UFont>
  - Regular / Medium / SemiBold / Bold Typeface Role Name
- Style Data Asset 자체는 Subsystem이 Soft Reference로 1회 해석하고 항상 필요한 UFont 두 개는 Style Data가 Hard Reference
- 기존 UIFontFamily / NumericFontFamily 이름은 Preset 식별과 Debug용으로 유지
- 정확한 함수·Scale·Fallback 계약과 실제 D1-09A·D1-09B Evidence는 `InGameUIAssetizationSpec.md v0.3.0`을 따른다.

신규 CFHUDLayoutData.h / .cpp
- ECFHUDSlotId
  - MissionSummary
  - AlertFeed
  - TargetPanel
  - VehiclePanel
  - RadarPanel
  - WeaponPanel
  - ReticleLayer
- FCFHUDSlotLayout
  - SlotId
  - AnchorMinimum
  - AnchorMaximum
  - Alignment
  - PixelOffset
  - DesiredSize
  - RenderScale
  - ZOrder
  - bVisibleByDefault
- UCFHUDLayoutData : UPrimaryDataAsset
  - ProfileId
  - ProfileVersion
  - ReferenceViewportSize
  - SlotLayouts

신규 CFUIDensityData.h / .cpp
- FCFUIDensityTokens
  - PanelPadding
  - PanelInnerGap
  - SectionGap
  - InfoRowGap
  - HeaderHeight
  - InfoRowHeight
  - InfoRowHeightLarge
  - ChipHeight
  - IconSmall / Medium / Large
  - PrimaryValueScale
  - SecondaryTextScale
  - OutlineScale
  - CaptionPolicy
- UCFUIDensityData : UPrimaryDataAsset
  - Preset
  - Tokens

CFUISubsystem.h / .cpp 최소 보완
- Config 기반 Soft Reference로 기본 Style / Density / HUD Layout을 한 번만 해석
- Widget이 콘텐츠 경로를 직접 Load하지 않게 Getter 제공
- Data Asset 누락 시 D1-08 Native Safe Fallback 유지
- 기존 Pause·Root·Pawn 수명 계약은 변경하지 않음
```

Epic 최신 Font API 확인 결과 Style Data의 최상위 강타입 Font 참조는 `UFont`로 고정한다. `UFontFace`는 원본 TTF/OTF payload를 저장하고 Composite `UFont`가 Typeface로 참조한다. `FSlateFontInfo`는 `UFont` Object와 Typeface Name을 결합해 UMG/Slate에서 사용한다. 구현 직전에는 `D:\\UnrealEngine_Source`의 실제 Header와 최종 일치 여부만 재확인한다. 경로 문자열을 Widget이나 Gameplay C++에 하드코딩하는 대안은 허용하지 않는다.

#### 12.1.3 D1-09B 실제 자산 Allowlist

이번 Gate의 P0 Active 자산은 아래로 제한한다.

```text
/Game/CarFight/UI/Style/DA_CFUIStyle_Default

/Game/CarFight/UI/Density/DA_CFUIDensity_Compact
/Game/CarFight/UI/Density/DA_CFUIDensity_Standard
/Game/CarFight/UI/Density/DA_CFUIDensity_Expanded

/Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16

/Game/CarFight/UI/Fonts/...
- Pretendard 실제 Font Asset
- IBM Plex Mono 실제 Font Asset
- 필요한 Font Face Asset

/Game/CarFight/UI/Icons/...
- CarFight가 직접 제작한 P0 Semantic Icon만
```

`DA_CFHUDLayout_1440_16`, 21:9, 32:9는 이번 Gate에서 만들지 않는다. 해당 Profile은 Deferred Expansion Review 때 생성한다.

#### 12.1.4 `DA_CFHUDLayout_1080_16` 승인 초기값

Reference Viewport는 정확히 `1920×1080`이다. 값은 D1-07 USER PASS XML의 현재 외곽 배치와 동일하게 시작한다.

| Slot | Anchor | Alignment | Pixel Offset | Desired Size | ZOrder |
|---|---|---|---:|---:|---:|
| MissionSummary | Top Left | `(0,0)` | `(24,24)` | `375×87` | 10 |
| AlertFeed | Top Center | `(0.5,0)` | `(+6,24)` | `552×81` | 40 |
| TargetPanel | Top Right | `(1,0)` | `(-24,24)` | `312×232.5` | 10 |
| VehiclePanel | Bottom Left | `(0,1)` | `(24,-24)` | `672×312` | 10 |
| RadarPanel | Bottom Center | `(0.5,1)` | `(0,-24)` | `285×270` | 10 |
| WeaponPanel | Bottom Right | `(1,1)` | `(-24,-24)` | `270×190.5` | 10 |
| ReticleLayer | Full Stretch | `(0,0)` | `(0,0)` | Viewport Fill | 30 |

- `ReticleLayer`는 기존 AimReticle 위치를 DataAsset 고정 좌표로 다시 계산하지 않는다. 전체 Viewport 투영 컨테이너만 제공한다.
- 위 값은 1080p Active Profile의 시작값이며 1440p `SafeMargin=64`와 혼합하지 않는다.
- `AlertFeed`의 `+6px`은 현재 USER PASS Mockup의 실제 중심을 그대로 보존한 값이다.
- Slot 내부 기능 Widget은 위치를 소유하지 않는다.

#### 12.1.5 Density Asset 초기값

`DA_CFUIDensity_Compact`, `Standard`, `Expanded`는 §5.5 표를 그대로 복사한다. 별도 튜닝값을 새로 만들지 않는다.

```text
Compact
PanelPadding 12 / PanelInnerGap 8 / SectionGap 8 / InfoRowGap 4
Header 36 / InfoRow 28 / Large 36 / Chip 22
Icon 16 / 20 / 28
Primary 0.86 / Secondary 0.90 / Outline 0.75

Standard
24 / 12 / 16 / 8
48 / 36 / 44 / 28
20 / 24 / 32
1.00 / 1.00 / 1.00

Expanded
32 / 16 / 24 / 12
56 / 44 / 52 / 32
24 / 32 / 40
1.12 / 1.08 / 1.00
```

`MinimumHitSize=48`, `Unknown / Unavailable / KnownZero` 의미, Critical·Fire State 보존은 Density 변경 대상이 아니다.

#### 12.1.6 D1-10 Base Widget 정확한 범위

경로:

```text
/Game/CarFight/UI/Base/WBP_CFButtonBase
/Game/CarFight/UI/Base/WBP_CFPanelBase
/Game/CarFight/UI/Base/WBP_CFStatusBar
/Game/CarFight/UI/Base/WBP_CFInfoRow
/Game/CarFight/UI/Base/WBP_CFAlertItem
```

공통 규칙:

- `StyleData`와 `DensityData`를 외부에서 전달받는다.
- 개별 Widget이 DataAsset을 직접 Load하지 않는다.
- `ApplyUIStyle`과 View Data 적용을 분리한다.
- Gameplay Pawn·Component 직접 Cast 금지.
- 미지원 정보는 `Collapsed`하고 Spacer도 제거한다.
- Hover와 Focused를 별도 상태로 유지한다.

`WBP_CFButtonBase`는 Pause에서 재사용할 수 있도록 실제 내부 `UButton`에 대한 Focus와 Activate 브리지를 제공하는 최소 C++ 부모 후보를 D1-10 구현 시 먼저 검토한다. 이 브리지는 UI 상호작용만 소유하며 Pause 판정은 소유하지 않는다.

#### 12.1.7 D1-11 Visual Prototype 범위

경로:

```text
/Game/CarFight/UI/HUD/WBP_CFInGameHUD
```

허용:

- Root Canvas / PC Safe Zone
- `DA_CFHUDLayout_1080_16`의 7개 Slot Container
- `DA_CFUIStyle_Default`와 Density Profile 적용
- Base Widget을 사용한 Mission·Alert·Target·Vehicle·Radar·Weapon Placeholder
- D1-07과 같은 Mock 문자열·수치

금지:

- `ACFVehiclePawn` 직접 Cast
- VehicleDefense·WeaponFire·Launcher·TargetSelect 실제 조회
- UI-P0-03 View Data를 이 단계에서 임의 구현
- 실제 Vehicle/Weapon/Target/Radar 기능 Widget 제작
- 21:9·32:9 Profile 구현

D1-11의 목적은 **실제 UMG에서 Style·Density·Layout 계약이 D1-07 승인본을 재현하는지**만 확인하는 것이다.

#### 12.1.8 D1-12 Pause Visual Migration 범위

현재 `UCFUISubsystem::CreatePauseMenuWidget()`은 `UCFPauseMenuWidget::StaticClass()`를 직접 생성하고 `EnsurePauseMenuTree()`는 Native Tree를 구성한다. 따라서 Blueprint 파생 Widget을 만드는 것만으로는 Visual Migration이 완료되지 않는다.

D1-12의 최소 수정은 다음으로 제한한다.

```text
/Game/CarFight/UI/Menu/WBP_CFPauseMenu
- UCFPauseMenuWidget 기반
- WBP_CFButtonBase Continue 사용
- SurfaceOverlay / DimOverlay / HeadingL / Primary Button Token 사용

UCFUISubsystem
- 설정된 PauseMenuWidgetClass가 있으면 사용
- 없거나 Load 실패 시 UCFPauseMenuWidget Native Fallback
- 기존 Menu 단일 수명과 OnContinueRequested 연결 유지

UCFPauseMenuWidget
- Blueprint Designer Tree가 있으면 이를 덮어쓰지 않음
- Blueprint Continue Bridge를 기존 OnContinueRequested로 연결
- Native Tree는 Asset 누락 시 Fallback으로 계속 유지
- GetContinueButton(), FocusDefaultButton() 외부 계약은 보존
```

Pause Visual Migration에서 World Pause, 입력 중립화, Launcher·Projectile·Timer 정지 규칙을 수정하지 않는다.

#### 12.1.9 단계별 검증 행렬

| 단계 | 자동/정적 검증 | 사용자/에디터 검증 | PASS 조건 |
|---|---|---|---|
| D1-08V | 공식 Build + `StyleDataContract` | 없음 | Compile/UHT + Automation Success |
| D1-09A | Layout/Density/Font Binding Contract Automation | 없음 | 기본값·중복 Slot·Fallback·Soft Ref 계약 PASS |
| D1-09B | Asset class/reference readback | DataAsset 값 육안 확인 | 1080 Profile·3 Density·Style·Font/Icon 참조 일치 |
| D1-10 | Widget Blueprint compile/readback | Designer 상태별 확인 | 5종 Compile + Style/Density 외부 입력 + Collapsed 계약 |
| D1-11 | Slot bounds 정적 비교 | 1920×1080 Preview | D1-07 위치·크기·24px 외곽·중앙 보호영역 재현 |
| D1-12 | 기존 Pause Automation + 신규 Class/Fallback 계약 | Pause/Continue/Focus PIE | 기존 Pause 동작 회귀 0 + Blueprint Visual 적용 |

전 단계 공통 FAIL:

- 기존 AimReticle·TargetSelect 의미 변경
- Gameplay 계산을 UI로 이동
- 실제 콘텐츠 경로를 Widget C++에 하드코딩
- 기존 dirty 파일을 통째로 덮어쓰기
- 1080p 승인 배치를 이유 없이 재튜닝
- Deferred 21:9·32:9를 현재 Gate 필수 범위로 확장

#### 12.1.10 실행 중단 조건

다음 중 하나면 다음 단계로 넘어가지 않는다.

```text
D1-08 Build 또는 Automation FAIL
D1-09A UHT/Compile FAIL
Font 원본 Copyright Notice 또는 OFL 전문 확보 실패
DataAsset Class/Reference Readback 불일치
Base Widget Blueprint Compile FAIL
1080p Slot이 D1-07 USER PASS 배치와 불일치
Pause Visual 적용 후 기존 Continue·Focus·World Pause 회귀 발생
```

문제 발생 시 해당 단계만 최소 수정하고 D1-09~12 다음 단계로 건너뛰지 않는다.

---

## 13. 현재 미결정 항목

- Pretendard·IBM Plex Mono License Eligibility는 OFL 1.1 기준 PASS / Shipping 패키지에서 라이선스 고지를 실제 노출하는 최종 경로는 Release Packaging 검증으로 유지
- 실제 Unreal `UFont` Composite / `UFontFace` Asset 생성·Fallback Font 구성
- Solid Core + Tactical Cut Icon의 실제 Vector/Texture 제작 방식과 원본 파일 포맷
- 색상 Hex의 인게임 최종 보정
- Reticle 기본 실루엣은 v0.82.0에서 확정 / 실제 TPS Runtime Capture 기반 크기·Stroke 미세 보정
- 실제 UMG Material 사용 여부
- VehiclePanel과 WeaponPanel의 실제 TPS 카메라 캡처 기반 1920×1080 가독성
- 2560×1440·3440×1440·5120×1440 후속 Layout Profile 확장 검토 결과
- Wireframe 패널 크기의 실제 TPS 카메라 FOV 보정
- 사용자 HUD 배치 편집 화면과 저장 기능의 실제 착수 시점

---

## 14. 사용자 검토 상태

### 승인 완료

```text
기본 시점: 외부 3인칭 차량 TPS
기본 화면비: 16:9
좌상단: Mission Summary
상단 중앙: Alert Feed
우상단: Target Panel
좌하단: Vehicle Panel
하단 중앙: Radar Panel
우하단: Weapon Panel
중앙: Reticle·Target Marker 보호영역
```

### 패널 Visual Layout 승인 완료

```text
D1-VEHICLE-PANEL Visual Layout Accepted
- 896×416 가로형
- 좌측 SpeedGauge는 좌측 세로→부드러운 곡선→상단 긴 수평으로 이어지는 비대칭 실제 RPM Gauge + 큰 3자리 디지털 속도 숫자
- RPM Gauge는 숫자·RPM Label 없이 21 Tick, 고정 시각 스케일 85%부터 Red Zone
- 속도 오른쪽에 작은 `km/h`, 그 오른쪽에 단일 Gear Slot을 두고 `R`, `N`, 실제 전진 기어 단수 숫자 표시
- 우측 ArmorBodyMap 중앙에 화면 왼쪽을 바라보는 차량 실루엣 배치
- 좌=Front / 상=Right / 우=Rear / 하=Left Armor, Top은 좌상단 Badge / Bottom은 우하단 Badge
- 6방향 Armor는 Plate/Badge 우측 세로 Bar의 NormalizedArmor로 표시하고 방향·수치 Text는 기본 제거
- Shield는 ArmorBodyMap과 Integrity 사이 전체 폭 Bar, Vehicle Integrity는 최하단 전체 폭 Bar
- Shield/Integrity 중앙 `999/999`, Fill 끝 Chevron으로 재생/수리 대기·진행 상태를 같은 문법으로 표시
- 고정 부품 손상 목록 없음

D1-WEAPON-PANEL Compact Revision Accepted
- 우하단 `464×360`은 최대 Slot 경계이며 실제 Content는 DesiredSize로 축소
- 이전 `432×224` Card·`136×96` Tile 고정값은 초기 Wireframe 참고값으로 전환
- 선택 무기의 강한 강조·Primary 1개·Secondary 최대 2개·FireStateStrip 유지
- 미지원 채널·Rail은 Collapse하고 빈 공간까지 제거
- 비선택 무기는 작은 교체형 Compact Tile 또는 `2 + Overflow`
- Ammo·Charge·Heat·Cooldown·Reload·LauncherSequence 공통 채널
- 탄약형·에너지형·런처형·복합형 표시 규칙
- Compact Token 기본값은 `InGameUIWeaponPanelSpec.md v0.5.0` User Accepted이며 실제 Static Review에서 검증
```

CarFight 전체 UI 커스터마이징 계약, D1-05 Style Token·Base Widget·Density Profile·WeaponPanel Compact Token, D1-06 Font·Icon 기본 Preset과 D1-07 Phase 1 `SR-1080-16` Static Review는 사용자 승인 완료다. Pretendard·IBM Plex Mono는 OFL 1.1 기준 상용 게임 번들 사용 적격성 PASS이며 D1-08V·D1-09A도 공식 Build·Automation PASS다. 상세 실행 계약과 Evidence는 `InGameUIAssetizationSpec.md v0.2.0`이 소유한다. 실제 Font·Icon Unreal Asset, 원본 라이선스 고지 파일, Style·Layout·Density DataAsset과 Base Widget 자산화는 아직 Pending이다. `SR-1440-16`·`SR-1440-21`·`SR-1440-32`는 후속 Layout Profile 확장으로 Deferred한다.

---

## 15. Changelog

### v0.86.0 - 2026-08-12

- CF-FQ-032 중앙 HUD 요소 전체를 대상으로 Reticle Layer Global Declutter / Marker Density / 중앙 시야 보호 계약을 추가했다.
- Declutter를 Gameplay 상태 변경이 아닌 Presentation-only 공간 해결로 정의해 Target Selection, Lock, Mission Primary, Damage Event와 Knowledge를 숨김 때문에 변경하지 않도록 했다.
- P0 동시 표시 구조를 Command 1 / Turret 1 / Selected Target 1 / Primary Objective 1 / Target 부착 Lock 1 / Immediate State 1 / Hit Marker State 1 / Incoming Direction 최대 4 Bucket으로 명시하고 Candidate·Secondary Objective·Multi-lock·Floating Damage Number 증식을 금지했다.
- 현재 Command Reticle 위치를 기준으로 Aim Core 56, Combat Ring 136, Status Lane 144×24와 충돌/해제 Padding 8/16의 기본 Style Token을 정의했다.
- Full → Compact → ShapeOnly → AttachedBadge 단계를 사용하고 공간 부족만으로 Command/Turret/Selected Target 핵심 Shape를 제거하지 않도록 했다.
- Command/Turret → Immediate Fire State → 짧은 Hit/Incoming Event → Selected Target/Lock → Primary Objective의 Semantic 충돌 우선순위를 정의하되 transient 피드백이 Persistent Marker를 이동시키거나 Reflow시키지 않도록 했다.
- Command Reticle 아래 Immediate Fire State Lane을 독점 영역으로 두고 Target/Objective/Lock Text를 같은 위치에 쌓지 않도록 했다.
- Selected Target 주변 보조 Text는 최대 1줄로 제한하고 실제 Lock Failure/Progress → Target 상태 → Distance 순으로 축약하도록 했다. Combat Ring의 World Marker 보조 Text 총량도 기본 1줄로 제한했다.
- Objective는 Aim Core에서 ShapeOnly + 낮은 Opacity를 사용할 수 있고 Target/Lock과 충돌하면 Distance부터 제거하며, 동일 Entity는 기존 Target Bracket + Objective Badge 결합을 유지하도록 했다.
- Incoming TOP/BOTTOM Micro Label은 여러 Arc 중 최대 1개만 유지하되 Arc 방향 최대 4개 정보는 보존하도록 했다.
- Persistent Marker 충돌 해결을 Provider 검증 → Projection → Stable Identity Merge → Safe Region → Target 고정 → Objective 제한 이동 → 보조 Text 축약 → transient overlay 순의 결정적 파이프라인으로 정리했다.
- Marker 경계 떨림을 막기 위해 충돌 진입 8 / 해제 16 Padding과 0.10초 짧은 Opacity 전환을 정의하고 Scale Pop·위치 Spring·Sound를 금지했다.
- Declutter는 UI-P0-03 이후 공통 Projection/Presentation Resolver가 소유하고 각 Widget이 독립 Tick·Actor 검색·Component 조회로 우선순위를 계산하지 않도록 했다.
- 이번 문서 변경은 D1-11 New Production 1920×1080 User Preview Pending / D1-11 NOT PASS, D1-12 Not Started, UI-P0-03 Not Started 상태와 Production Asset을 변경하지 않는다.

### v0.85.0 - 2026-08-12

- CF-FQ-032 다음 HUD 상세 항목으로 MissionSummary와 분리된 Objective World Marker / Mission Navigation 시각 계약을 추가했다.
- 현재 실제 C++에는 Mission/Objective Runtime Provider가 없고 `MissionSummary`·`ReticleLayer` Layout Slot만 존재함을 유지해 실제 Gameplay에서는 MissionSummary와 Objective Marker를 모두 Collapse하고 D1/Static Mock만 Visual 증거로 허용했다.
- P0 World Navigation은 Mission Provider가 명시한 Primary Objective 1개만 표시하고 `+N OBJECTIVES`를 근거로 Widget이 거리순·화면 위치순으로 Secondary Marker를 생성하지 않도록 했다.
- Objective On-screen 실루엣을 `Open Hex Beacon + Bottom Stem`, Off-screen을 Target 2-Corner Bracket과 다른 `Open Edge Hex Beacon`으로 정의하고 기본색을 Target Selection의 AccentTactical이 아닌 StateNotice로 분리했다.
- Exact / Approximate / No Navigation Anchor 의미를 분리해 Mission이 공개하지 않은 정확한 Actor 위치를 Widget이 검색·보정하지 못하게 했고 Through-world 표시 여부도 Mission Provider 권한으로 남겼다.
- MissionSummary Primary와 Marker가 동일 Primary Objective Identity를 공유하고 Navigation Anchor가 없는 목표는 Summary만 표시할 수 있도록 했다.
- 거리 Text는 Provider/Presenter가 Player-facing 값으로 제공할 때만 표시하며 근사 위치에서 UI가 정확한 Meter 값을 만들어내지 않도록 했다.
- Selected Target과 같은 Objective Entity는 두 Full Marker를 중첩하지 않고 Target 4-Corner Bracket + 상단 중앙 작은 Objective Badge로 결합하도록 했다. 단 Screen 위치 우연만으로는 병합하지 않는다.
- 서로 다른 Marker 충돌에서는 Target/Reticle을 이동하지 않고 Objective 보조 Text를 먼저 Collapse하며, Edge 충돌에서는 Selected Target 위치를 고정하고 Objective만 같은 Edge를 따라 최대 56 Design Unit 이동해 40 간격을 확보하도록 했다.
- Objective Marker가 Radar Contact를 자동 생성하지 않도록 Sensor Radar와 Mission Navigation을 분리하고 후속 Radar Mission Overlay는 별도 Channel로 남겼다.
- Player Destroyed만으로 Objective Failed/삭제를 만들지 않고 상위 Combat HUD/Outcome 정책을 따르며 완료·실패·Primary 변경 수명과 CF-PDL-0009 무음 정책을 Mission Provider 경계로 유지했다.
- 이번 설계는 D1-11 Production User Preview Gate와 기존 Production Asset을 변경하지 않는다.

### v0.84.0 - 2026-08-12

- CF-FQ-032 Player Integrity Critical → Player Vehicle Destroyed → 미래 Outcome/Result 전환의 시각 계약을 추가했다.
- Integrity Critical을 생존 중 위험 상태로 분리하고 Provider가 명시할 때만 VehiclePanel Critical Accent와 Persistent Alert를 사용하도록 했다. Reticle·Weapon·Radar는 Critical만으로 비활성화하지 않는다.
- Destroyed Terminal에서 VehiclePanel은 `DESTROYED`를 표시하되 Shield·Armor·Speed·RPM·Gear를 임의 0으로 만들지 않고 실제 Provider의 최종/실시간 값을 유지하도록 했다.
- Player Vehicle Destroyed가 `DEFEAT / GAME OVER / RESULT`를 자동 의미하지 않도록 하고 기본 중앙 Terminal Plate는 `VEHICLE DESTROYED`만 표시하도록 했다.
- 실제 Combat Availability=false가 함께 제공될 때만 Reticle·Lock·Target Marker·Weapon·Radar를 0.18초 계열로 Fade하도록 해 UI-only 가짜 조작불가 상태를 금지했다.
- Destroyed 순간 `INTEGRITY CRITICAL` Alert를 종료하고 기본 `VEHICLE DESTROYED` Alert 중복을 만들지 않으며 VehiclePanel Terminal State와 상태 Plate가 결과를 소유하도록 했다.
- 최초 파괴 Hit의 Incoming/Vignette 1회 연출은 완료를 허용하되 Destroyed 이후 새 Incoming/Outgoing Hit Feedback은 Terminal HUD에서 기본 억제하도록 했다.
- Result Screen은 Screen Layer의 미래 Outcome Flow 소유 화면으로 분리하고 Destroyed→Result 전환 시간, Result Title과 Action을 Widget Timer나 VehicleHealth에서 생성하지 않도록 했다.
- Terminal Plate 320×56, Icon 20, Accent 4, Enter 0.20초, 낮은 Overlay Alpha 0.10의 기본 Preset을 추가했으며 대형 Game Over Banner·반복 Blink·사운드는 사용하지 않는다.

### v0.83.0 - 2026-08-12

- CF-FQ-032 다음 상세 항목으로 Incoming Damage Direction, Hit Confirmation과 Local Damage Feedback 시각 계약을 추가했다.
- FireSuccess를 `발사 명령 승인`, Hit Confirmation을 `실제 방어층 감소 확인`으로 분리해 기존 FireFeedback 의미를 변경하지 않도록 했다.
- 피격 방향은 전체 원형 HUD가 아닌 Reticle 주위 짧은 Inward Damage Arc를 사용하고 Camera-relative 방위로 표시하도록 했다. 동일 방향 연사는 Merge/Refresh하고 서로 다른 방향은 최대 4개까지 유지한다.
- Shield / Armor / Integrity 중 이번 타격에서 가장 깊게 실제 감소한 계층을 Direction Arc와 Hit Marker의 대표 시각 상태로 사용하며 색상뿐 아니라 Stroke/Cap/Diamond 형상도 구분하도록 했다.
- Hit Confirmation은 Command Reticle과 다른 4 Diagonal Tick 형상으로 확정하고 World Impact, Damage Reject와 `bAppliedToAnyLayer=false` 결과에는 표시하지 않도록 했다.
- 기본 Combat HUD의 Floating Damage Number를 금지해 Target Intelligence 공개 규칙, 방어 계층 의미와 중앙 전투 가독성을 보호했다.
- 내 차량 Damage Feedback을 Direction Arc / VehiclePanel Layer Highlight / 필요한 Alert로 분리하고 Armor는 실제 `ArmorDirection` Plate만 Highlight하도록 했다.
- Integrity가 실제 감소한 경우에만 0.26초·Peak Alpha 0.14의 짧은 Peripheral Vignette를 허용하고 Shield/Armor 일반 타격에는 Full-screen Damage Flash를 사용하지 않도록 했다.
- Camera Shake, Controller Rumble, Post Process Damage Effect, World Impact FX와 Physics Impulse를 CF-FQ-032 UI 범위 밖으로 유지했다.
- Damage Feedback 관련 모든 수명은 Game Time/Pause 계약을 따르고 프로젝트 전역 `CF-PDL-0009`에 따라 사운드를 추가하지 않는다.

### v0.82.0 - 2026-08-12

- CF-FQ-032의 다음 HUD 상세 항목으로 Reticle Layer, World Target Marker와 Lock/Fire-Control 시각 계약을 확정했다.
- Command Reticle의 기본 실루엣을 기존 WBP 바인딩과 호환되는 `Center Dot + 4 Open Bracket`으로 확정하고 48×48 기준 Visual Token을 정의했다.
- CurrentMuzzleDirection 기반 Turret Reticle을 24×24 `Split Cross / 4-Tick Micro Marker`로 분리해 Command Reticle·Target Bracket·탄착점과 혼동되지 않도록 했다.
- Command/Turret Reticle 사이 연결선 없이 실제 화면 간격 자체로 정렬 오차를 읽도록 하고 기존 TurretAligning, WeaponNotAligned, MuzzleBlocked, OutOfArc 의미를 보존했다.
- FireFeedback는 Command Reticle 아래 대표 상태 한 줄 + 선택적 Cooldown Micro Bar로 제한하고 실제 Provider가 없는 FirePending/Reloading 상태를 생성하지 않도록 했다.
- Selected World Target Marker를 Projected Bounds 기반 4-Corner Open Bracket으로 정의하고 Target Panel의 Name/Faction/Intel 중복 표시를 제한했다.
- Off-screen/BehindCamera는 Safe Region 경계의 2-Corner Open Edge Bracket을 사용하고 삼각 Arrow를 Hostile Symbol과 혼용하지 않도록 했다.
- Occluded/Last Known/Contact Lost/Destroyed 상태를 기존 Target Lifecycle과 연결해 X-ray Actor 추적·위치 외삽·유령 Edge Marker를 금지했다.
- 현재 Lock Runtime이 없음을 전제로 Lock Indicator를 미래 Provider 계약으로 정의하고 Target Marker 바깥 4-Corner Segment가 실제 Progress에 따라 수렴하는 Acquiring 표현을 채택했다.
- Locked는 1회 Brightness 후 고정, Blocked/Lost는 실제 Provider 사건만 표시하며 반복 Pulse·회전·사운드를 사용하지 않도록 했다.
- Selected Target, Lock, Command Aim, Turret Aim과 실제 Fire Validation을 서로 독립된 상태로 유지하도록 했다.
- Reticle 최종 실루엣 미결정 항목을 종료하고 실제 TPS Runtime Capture에서 크기·Stroke만 미세 조정하도록 남겼다.
- 문서 헤더와 Assetization 기준을 현재 D1-07~10 PASS / D1-11 Production Rework Automated PASS·User Preview Pending / AssetizationSpec v0.11.0 상태로 동기화했다.

### v0.81.0 - 2026-08-12

- Alert Feed 다음 우선순위로 Mission Summary의 Compact 내부 계약을 설계했다.
- 현재 C++에 Mission/Objective Runtime Provider가 없음을 확인해 실제 Gameplay에서는 Provider 연결 전 MissionSummary를 Collapse하고 Visual Prototype에서만 Mock Data를 허용하도록 경계를 명시했다.
- `480×128` 안에서 Header 24 / Primary 44 / Footer 28 / Gap 4×2 / Vertical Padding 12×2 기본 Geometry를 정의하고 Objective 1개 + 선택적 Progress/Timer + `+N OBJECTIVES` 구조로 제한했다.
- Mission Provider가 Primary Objective와 Player-facing 진행값을 소유하고 Widget이 Objective Priority·Kill Count·거리·Timer를 직접 계산하지 않도록 했다.
- Count / Normalized / TimeRemaining / StateText의 Presentation Variant와 Game Time 기반 Mission Timer·Pause 규칙을 정의했다.
- Objective 변경·완료·실패는 같은 Slot의 짧은 시각 전환으로 처리하고 자동 Carousel·대형 Banner·상시 로그·상호작용을 P0에서 제외했다.
- Mission 진행과 Alert Feed, World Objective Marker의 책임 경계를 분리했다.
- 저장소 복원 중 프로젝트 전역 `CF-PDL-0009` 사운드 비지원 정책과 충돌하던 Tactical Analysis 완료음 계약을 발견해 현재 본문을 시각 피드백 전용으로 교정했다. 과거 Changelog의 당시 기록은 역사 이력으로 보존했다.
- 기본 Layout 표의 Target/Weapon 상태 요약과 Mission/Alert 상세 상태를 현재 문서 기준으로 갱신했다.

### v0.80.0 - 2026-08-12

- CF-FQ-032 HUD 상세 설계의 다음 우선순위로 Alert Feed를 구현 가능한 수준까지 구체화했다.
- Alert Feed를 전투 로그가 아닌 즉시 행동 판단용 상태 Feed로 제한하고 `Notice / Warning / Critical`의 의미와 정렬 우선순위를 확정했다.
- 같은 `AlertKey`는 새 Row를 생성하지 않고 제자리 갱신하며 Severity 상승 시에만 재정렬·1회 강조하도록 했다.
- 기본 720×120 Slot에서 최대 3개를 실제 수용할 수 있도록 Compact Feed를 `Primary 40 / Secondary 32 / Gap 4` Design Unit 구조로 정의했다.
- 3개 초과 활성 Alert는 Presenter 집합에 유지하고 상위 3개만 표시하며 마지막 Row의 `+N` 보조 Overflow Count를 허용했다.
- Duration을 Game Time 기준으로 고정해 World Pause 동안 Alert 시간이 소모되지 않도록 했고 Persistent 상태의 사용 범위를 제한했다.
- Tactical Analysis 완료, Active Scan 신규 Contact, Target Lifecycle, 평상시 Weapon/Defense 수치 변화는 각 소유 Panel 피드백을 사용하고 Alert Feed에 중복 통지하지 않도록 했다.
- Critical 반복 Pulse는 Accent Marker/Icon에만 제한하고 Text 전체 점멸·대형 Banner·화면 전체 Flash를 금지했다.
- P0 Alert Feed는 비상호작용 HUD로 유지하고 수동 Dismiss·Scroll History는 후속 별도 기능으로 분리했다.

### v0.79.0 - 2026-08-12

- 문서 동기화 검토에서 `사용자 검토 상태 → 패널 Visual Layout 승인 완료`의 VehiclePanel 요약에 남아 있던 오래된 `원호형 속도계 + 세로 D/N/R` 표현을 최신 Accepted 계약으로 교정했다.
- 현재 승인 요약을 비대칭 실제 RPM Gauge, 3자리 디지털 속도, 21 Tick/85% Red Zone, 단일 Gear Slot, 우측 ArmorBodyMap과 6방향 세로 Bar, Shield/Integrity 공통 Bar 문법으로 동기화했다.
- 과거 Changelog의 이전 결정 기록은 역사 이력으로 보존하고 현재 상태 요약만 수정했다.

### v0.78.0 - 2026-08-12

- 사용자 요청에 따라 Destroyed Target Lifecycle을 Contact Lost와 분리해 확정했다.
- Destroyed는 Sensor Lost나 낮은 HP 추정이 아니라 Gameplay/신뢰 가능한 Provider가 명시적으로 Known으로 제공할 때만 진입하도록 했다.
- 파괴 확인 후 기본 1.25초 `DestroyedTargetHoldDuration` 동안 선택을 유지해 Kill Confirmation을 제공하고 이후 자동 선택 해제하도록 했다.
- Destroyed 진입 즉시 Identification/Tactical Analysis를 종료하고 미완료 Progress Decay·다음 Category 분석·가짜 완료 피드백을 금지했다.
- Target Panel은 Hold 동안 Identity + `DESTROYED` 중심 Compact Terminal State를 사용하고 하위 Shield/Armor/Module 값을 임의 0/Destroyed로 덮어쓰지 않도록 했다.
- Radar는 Hold 동안 기존 Blip/Bracket을 낮은 강도로 유지하되 새로운 X/해골 아이콘으로 기본 Contact Symbol을 교체하지 않도록 했다.
- Hold 종료 시 `TARGET LOST` 없이 `DESTROYED → NO TARGET`으로 전환하고 다음 Contact 자동 선택을 금지했다.
- 플레이어가 Hold 중 다른 Target을 직접 선택/해제하면 즉시 Destroyed Hold를 종료하도록 했다.
- UI Hold와 Fire-Control Target Validity를 분리해 파괴된 Target 선택 잔류가 Lock/발사 유효시간을 연장하지 않도록 했다.

### v0.77.0 - 2026-08-12

- 사용자 요청에 따라 Target Panel의 `Live Contact → Last Known → Contact Lost` 상태 전환과 Selected Target 수명 규칙을 확정했다.
- 선택 Target은 Last Known 동안 자동 해제하지 않고 유지하며 Target Panel에 `LAST KNOWN` 상태를 추가하도록 했다.
- Last Known에서는 실시간 `DISTANCE` 대신 저장된 마지막 World Position까지의 `LAST POS` 의미를 사용하고, Speed는 `LAST SPEED/Stale`로만 보존하거나 Compact에서 숨길 수 있게 했다.
- Identity·Weapon 같은 정적 Intel은 유지하고 Shield/Armor/Vehicle/Module 같은 동적 Intel은 Stale로 전환하도록 했다.
- Last Known에서는 신규 분석을 시작하지 않고 기존 미완료 Analysis만 Interrupted/Decay 규칙으로 감소·재개하도록 했다.
- Last Known Selected Target이 Fire-Control Live Target 권한을 자동 획득하지 않도록 UI Selection과 Weapon/Lock Gameplay 유효성을 분리했다.
- Last Known 기간 안 재획득 시 재선택 없이 같은 Target을 Live로 복귀시키고 Ghosting/Last Known 상태만 제거하도록 했다.
- Contact Lost는 Sensor/Tracking Provider가 소유하며 현재 선택 대상이면 자동 선택 해제 후 짧은 `TARGET LOST → NO TARGET` 전환을 사용하도록 했다.
- Contact Lost 시 다음 Contact 자동 선택을 금지하고, 이후 동일 Entity가 재탐지돼도 자동 재선택하지 않도록 했다.
- 수동 Target 해제/변경은 Contact Knowledge를 삭제하지 않으며 P0 기본 Target Cycle은 신규 선택 후보로 Live Contact를 우선하고 Ghosted Last Known을 자동 Cycle에 포함하지 않도록 했다.
- `Destroyed`와 `Contact Lost`를 별도 상태로 유지해 Sensor Lost만으로 파괴를 추정하지 않도록 했다.
- 과거 Identification Analysis의 미결정 진행률 문구를 현재 확정된 Decay 계약으로 교정했다.

### v0.76.0 - 2026-08-12

- 사용자 진행 승인에 따라 Target Panel 전체 Density와 Overflow 정책을 확정했다.
- 기본 폭 384와 Top Right Anchor를 유지하고 2560×1440 기준 최대 높이를 640으로 제한해 우측 WeaponPanel과 중첩되지 않도록 했다.
- 기본 전투 HUD에는 ScrollBox·자동 Carousel·Page Rotation을 사용하지 않고 안정적인 정보 위치를 유지하도록 했다.
- Block 순서를 `Header/Identity → Tracking → Analysis Progress → Defense → Weapon → Damage`로 고정했다.
- Header/Relation, 핵심 Tracking, 현재 Analysis Progress를 Pinned 정보로 정의해 Tactical Intelligence 증가로 제거되지 않도록 했다.
- 높이 초과 시 Secondary Text → Weapon Overflow → Damage Overflow → Compact Armor/Bar/Identity 보조 Row 순으로 압축하고 Knowledge 자체는 보존하도록 했다.
- Armor 6방향과 Shield/Vehicle 독립 의미는 Density 때문에 평균값으로 합치지 않도록 했다.
- Critical Damage는 Severity Style로 강조하되 Block 순서나 Panel Geometry를 재배치하지 않도록 했다.
- 긴 Localization Text는 Short Display Name/Ellipsis를 사용하고 가독성 하한 이하 Font 축소나 내부 Asset Name 대체를 금지했다.
- Compact 규칙 후에도 넘치는 상세 정보는 `+N` 요약을 남기고 후속 Expanded Target Detail이 소유하도록 경계를 확정했다.

### v0.75.0 - 2026-08-12

- 사용자 요청에 따라 Target Panel의 Damage / Module Intelligence 표시 규칙을 현재 P0 Provider 부재와 후속 Engine/Wheel/Turret 확장까지 포함해 설계했다.
- 현재 Module Damage Runtime이 없을 때는 Damage Block을 Collapse하고 `Unavailable` Category를 가짜 Progress/완료 피드백 없이 Skip하도록 했다.
- 후속 Module Damage는 모든 부품 목록이 아니라 확인된 손상·기능 이상만 `Module Name + Severity + Functional State` Compact Row로 표시하도록 했다.
- CF-FQ-033의 후속 4단계 Module 상태는 UI 후보 의미만 제시하고 실제 Runtime enum·임계값은 선행 확정하지 않도록 경계를 유지했다.
- 기본 Damage Summary는 최대 3 Row, 4개 이상은 2 Row + `+N DAMAGED` Overflow로 축약하도록 했다.
- Engine·Wheel·Turret의 Player-facing 예시와 Wheel 가변 개수·Provider Label·Grouping 규칙을 정의했다.
- Module Damage를 동적 Intelligence로 취급해 Live 갱신 중단 시 Stale, Repair/복구 시 Row 제거 또는 `MODULES · NOMINAL` 전환이 가능하도록 했다.
- 정확한 Module HP·Damage Formula·Damage Log·Weak Point·Repair ETA는 기본 Target Panel에서 제외하고 후속 Expanded Detail 경계로 남겼다.

### v0.74.0 - 2026-08-12

- 사용자 진행 승인에 따라 Target Panel의 Weapon Intelligence Compact Summary 규칙을 확정했다.
- 개별 Weapon Instance를 나열하지 않고 Player-facing Weapon Family 기준으로 Grouping해 `Family/Type + Count + 선택적 Mount Summary` 한 줄 구조를 사용하도록 했다.
- 정확한 Count가 Known일 때만 `×N`을 표시하고 Estimated 정보에서는 `MULTIPLE` 또는 실제 Source의 추정 범위만 사용해 가짜 정확도를 만들지 않도록 했다.
- Mount는 내부 Transform/Profile을 UI가 해석하지 않고 Intelligence Source가 제공한 Player-facing Label만 표시하도록 했다.
- 기본 Compact Weapon Summary는 최대 3 Family Row, 4개 이상은 앞의 2개 + `+N TYPES` Overflow로 축약하도록 했다.
- Weapon Row 정렬은 안정적인 Provider Display Order를 사용하고 Widget이 위협도를 임의 계산해 재정렬하지 않도록 했다.
- Known Weapon Family/Count/Mount는 현재 P0에서 정적 Loadout Knowledge로 보존하고 Last Known 전환만으로 Stale 처리하지 않도록 했다.
- Ammo/Heat/Charge/Cooldown/Reload 등 적 Weapon 내부 운용 상태는 Target Weapon Block에 추가하지 않는 기존 계약을 유지했다.

### v0.73.0 - 2026-08-12

- 사용자 진행 승인에 따라 분석 완료 후 Shield/Armor/Vehicle Intelligence의 Target Panel 실제 배치 규칙을 확정했다.
- Target Panel에서 VehiclePanel의 차량 실루엣·Armor Plate 구조를 복제하지 않고 Compact Intelligence Card 문법을 유지하도록 했다.
- Shield와 Vehicle Integrity는 `Label + 얇은 Horizontal Bar + 상태값` 한 줄 구조로 정했다.
- Armor는 `FRONT|REAR / LEFT|RIGHT / TOP|BOTTOM`의 고정 2열×3행 Compact Grid로 배치하고 여섯 방향을 평균값으로 합치지 않도록 했다.
- Armor Block이 활성화된 뒤 미확인 방향은 `???` Cell로 남겨 방향 위치가 흔들리지 않도록 했다.
- Known에서만 기본적으로 정확한 Normalized Percent를 표시하고 Estimated에서는 실제 Source가 제공한 질적 상태/추정 범위만 사용해 가짜 정밀 수치를 만들지 않도록 했다.
- Dynamic Intel의 Stale 상태는 마지막 값을 보존하되 낮은 시각 강도와 비색상 상태 표기로 Current와 구분하도록 했다.
- Defense Block은 정보가 있을 때만 아래로 확장하고 모든 Unknown Category의 빈 공간을 미리 예약하지 않도록 했다.

### v0.72.0 - 2026-08-12

- 사용자 진행 승인에 따라 Tactical Analysis Category 완료 시각·음향 피드백을 확정했다.
- 완료 피드백을 개별 Field가 아니라 Category당 1회로 묶어 여러 Row가 동시에 공개돼도 Flash·Sound가 반복되지 않도록 했다.
- 새 정보는 Target Panel 내부에서만 짧은 Brightness/Opacity Highlight를 사용하고 Scale Pop·Bounce·반복 Glow/Pulse는 사용하지 않도록 했다.
- Category 완료가 Panel 전체, Radar Blip, Target Bracket, Alert Feed까지 중복 전파되지 않도록 했다.
- 짧고 낮은 우선순위의 Scanner/UI 확인음 1회를 허용하고 `ScannerAnalysisComplete` Semantic Sound Event로 추상화해 실제 Sound Asset은 후속 Audio Style이 소유하도록 했다.
- Known/Unavailable 자동 Skip에는 완료음을 재생하지 않고 실제 Intelligence 획득/승격 완료에만 피드백을 발생시키도록 했다.
- 완료 연출 때문에 Scanner Gameplay에 인위적 대기시간을 넣지 않고 다음 Category를 즉시 시작할 수 있게 했다.
- 전체 Queue 마지막 완료에도 별도 대형 Banner나 추가 완료음을 사용하지 않고 마지막 Category와 동일한 1회 피드백만 사용하도록 했다.

### v0.71.0 - 2026-08-12

- 사용자 진행 승인에 따라 Tactical Analysis의 Category 자동 분석 순서를 확정했다.
- 기본 Priority를 `Shield → Weapon → Armor → Vehicle → Damage/Module`로 두고 한 번에 하나의 미완료 Category를 순차 분석하도록 했다.
- Priority는 Scanner/Tactical Analysis Profile에서 Override 가능한 기본 List로 두어 전문 Scanner의 분석 성향을 확장할 수 있게 했다.
- Capability 없음, NotApplicable/Unavailable, 이미 충분히 Known인 Category는 자동 Skip하고 Estimated 정보는 Known 승격을 위해 분석 대상으로 유지하도록 했다.
- 완료된 동적 Intel의 Current/Stale 갱신은 초기 획득 Queue와 분리해 새 Category 분석을 막지 않도록 했다.
- 감쇠 중 Progress가 남아 있는 Category는 재획득 시 우선 재개하고, 0까지 감쇠한 경우에만 Priority List를 재평가하도록 했다.
- Target 전환 시 이전 Target의 미완료 분석은 기존 Decay를 적용하고 새 Target은 독립 Queue를 시작하도록 했다.
- 기본 HUD에는 현재 Category 하나만 표시하고 수동 Queue 편집·Category 강제 선택은 P0 기본 계약에서 제외했다.

### v0.70.0 - 2026-08-12

- 사용자 진행 승인에 따라 Tactical Analysis의 분석 증가 속도와 손실 시 감쇠 속도를 분리된 Scanner/Profile 조정축으로 정의했다.
- 기본 튜닝 원칙을 `Decay Rate < Build Rate`로 두어 순간적인 차폐가 분석 자체보다 더 큰 페널티가 되지 않도록 했다.
- 고성능 Scanner는 빠른 Build, 느린 Decay, 긴 Analysis Range, 넓은 Category Capability를 독립적으로 조합할 수 있게 했다.
- Target Panel의 Tactical Analysis 진행 UI는 여러 Category Bar를 나열하지 않고 현재 미완료 Category 하나의 Compact Progress를 우선 표시하도록 했다.
- 정상 분석 중에는 Progress 증가, 조건 손실 시에는 같은 Bar가 역방향으로 감소하며 `INTERRUPTED` 계열 짧은 Text와 낮은 명도로 상태를 구분하도록 했다.
- 감쇠 상태에 Blink/Pulse/경고 Flash를 사용하지 않고, 조건 재확보 시 남은 Progress에서 자연스럽게 증가로 전환하도록 했다.
- 진행률 0 도달 시 강한 실패 메시지를 지속하지 않고 Compact Progress를 숨기거나 초기 상태로 되돌릴 수 있게 했다.

### v0.69.0 - 2026-08-12

- 사용자 결정으로 Scanner Tactical Analysis 중 Target이 유효거리 밖으로 나가거나 Scanner Occluder 뒤로 숨는 등 분석 조건을 잃었을 때 **미완료 진행률이 서서히 감소**하도록 확정했다.
- 조건 재확보 시 남아 있는 진행률에서 즉시 분석을 재개하고, 감쇠가 0에 도달한 경우에만 다음 분석을 처음부터 다시 시작하도록 했다.
- Tactical Analysis 진행률을 즉시 초기화하거나 무기한 보존하는 두 극단을 모두 사용하지 않도록 했다.
- 감쇠 속도는 Scanner/Tactical Analysis Profile 조정값으로 두어 장비 성능 차이를 표현할 수 있게 했다.
- 이미 Known으로 확정된 Intelligence는 미완료 Analysis 진행률 감쇠와 분리해 유지하도록 했다.

### v0.68.0 - 2026-08-12

- 사용자 요청에 따라 ID2 이후 `Shield / Armor / Vehicle / Weapon / Damage` Target Intelligence 획득 체계를 설계했다.
- Target Intelligence를 단일 전역 단계로 묶지 않고 정보 필드별 `Unknown / Estimated / Known / Unavailable` Knowledge State로 관리하도록 확정했다.
- Knowledge와 별도로 `Current / Stale` Freshness 개념을 두어 정적 Intel과 실시간 전투 상태를 분리했다.
- Intelligence Source를 Direct Visual/Combat Observation, Mission/Database/Prior Intel, Scanner Tactical Analysis, 후속 Friendly Telemetry로 구분했다.
- Friendly IFF/전용 주파수는 기존대로 즉시 ID2를 제공하지만 전체 Shield/Armor/Weapon/Damage Telemetry까지 자동 공유하는 계약으로 확대하지 않았다.
- Scanner Tactical Analysis는 ID2 이후 선택 Target에 자동으로 이어지는 후속 분석으로 두고, 별도 두 번째 Analyze 입력을 기본 요구하지 않도록 했다.
- Tactical Analysis의 기본 조건을 `Selected + ID2 + Live Contact + Analysis Range + Scanner LOS/Occlusion + Category Capability`로 정의했다.
- Shield는 관찰로 존재 사실, Database로 정적 사양, Scanner로 현재 Ratio/Broken 상태를 얻을 수 있게 하고 적 Target에서는 내부 Current/Maximum 숫자보다 Ratio/Percent를 기본으로 했다.
- Armor는 6방향 독립 Knowledge를 유지하고 Visual은 보이는 방향만 추정, Scanner는 Capability가 있는 방향의 현재 Ratio를 Known으로 제공하도록 했다.
- Vehicle Integrity는 관찰로 Estimated, 명백한 Destroyed는 Known, Scanner로 현재 Ratio를 Known으로 제공하도록 했으며 Module Damage와 의미를 분리했다.
- Weapon Intelligence는 시각 관찰·사전 Intel·Scanner 분석을 분리하고 Scanner가 실제 장착 Weapon Type/Family·개수·Mount 정보를 제공할 수 있게 했다.
- 적 Target의 Ammo/Heat/Charge/Cooldown/Reload/Launcher 내부 상태는 기본 Intelligence에서 제외해 숨겨진 런타임 상태를 만능 Scanner로 공개하지 않도록 했다.
- 현재 독립 Module Damage Runtime이 없으므로 가짜 부품 손상 정보를 만들지 않고, 후속 Runtime/Capability가 생길 때 별도 Damage Intelligence로 확장하도록 했다.
- 정적 Intel은 Knowledge Memory에 보존할 수 있고 동적 Shield/Armor/Integrity 값은 분석이 끊기면 마지막 샘플을 Stale로 유지하며 실시간처럼 추정 갱신하지 않도록 했다.

### v0.67.0 - 2026-08-12

- 사용자 진행 승인에 따라 `ID0 / ID1 / ID2`별 Target Panel 정보 공개 범위를 확정했다.
- 거리·속도 같은 Tracking/Kinematic 정보는 Identification과 분리해, 정체를 모르는 ID0에서도 Sensor Tracking Data가 유효하면 표시할 수 있게 했다.
- ID0는 `UNKNOWN CONTACT`와 `???`를 유지하며 NAME/TYPE/FACTION을 추정하지 않도록 했다.
- ID1은 Hostile/Friendly 같은 전술 관계를 공개하되 개별 Entity 정체와 분리하고, NAME/TYPE/FACTION은 필드별 Knowledge 근거가 없으면 계속 Unknown일 수 있게 했다.
- ID2에서는 NAME/Callsign, TYPE/Class, FACTION/Organization 등 실제 확보된 Identity Data를 공개하도록 했다.
- Friendly는 기존 IFF 계약 때문에 정상 흐름에서는 탐지 즉시 ID2로 가며 장시간 ID1 Friendly를 만들지 않도록 했다.
- Scanner Identification Analysis 중에는 Target Panel에서 Compact 진행 상태를 표시할 수 있고 완료 후 Identity Slot으로 결과를 반영하도록 했다.
- SHIELD/ARMOR/VEHICLE 정확 수치, 무장, 부품 손상과 약점은 ID2 자동 공개에서 제외하고 별도 Target Intelligence/Analysis State가 소유하도록 했다.
- Target Panel은 Top Right/Header를 고정하고 정보가 늘어날 때 아래쪽으로 확장해, 처음부터 최대 빈 공간을 예약하지 않으면서 상태 변화 시 시각적 흔들림을 줄이도록 했다.

### v0.66.0 - 2026-08-12

- 사용자 진행 승인에 따라 기본 Radar와 선택 장착 Scanner의 비아군 Identification 역할을 분리했다.
- 기본 Radar는 근거리 Passive Tracking, Visual Acquisition 연동, Friendly IFF와 이미 확보된 전술 관계 정보 표시를 담당하며 Scanner 없이도 기본 전투가 가능하도록 했다.
- 정보가 없는 비아군 Sensor Return은 기본 Radar만으로 자동 정밀분석하지 않고 `ID0 Unknown ◆`으로 유지할 수 있게 했다.
- Mission/사전 Intel, 기존 Entity 기억, 알려진 관계 정보, 명백한 적대행위, 명확한 Visual Recognition 같은 Gameplay 조건은 Scanner 없이도 해당 단계까지 즉시 Identification을 승격할 수 있게 했다.
- 명백한 적대행위는 정체를 모르더라도 최소 `ID1 Hostile ▼`를 즉시 확정할 수 있게 하여 Hostile 판정과 Identity Known을 분리했다.
- 선택 장착 Scanner에는 기존 원거리 Active Scan 외에 **Sensor 기반 Identification Analysis** 역할을 부여했다.
- 360° Active Scan은 `Contact를 찾아내는 Detection Action`, Identification Analysis는 `선택 Contact가 누구인지 알아내는 기능`으로 분리해 Active Scan 완료가 모든 Contact의 식별정보를 자동 공개하지 않도록 했다.
- 초기 기본안에서 Scanner Identification Analysis는 별도 반복 입력을 추가하지 않고 현재 선택 Target이 유효 조건을 만족할 때 자동 진행하도록 했다.
- Scanner별 Identification Range, Occlusion/LOS, 선택적 Analysis Time과 식별 Capability를 통해 ID1만 가능한 장비부터 ID2까지 가능한 장비까지 확장할 수 있게 했다.
- ID1→ID2 승격은 Radar Hostile/Friendly 기본 기호를 다시 바꾸는 단계가 아니라 Target 정보의 정체 데이터가 확장되는 단계로 정의했다.

### v0.65.0 - 2026-08-12

- 사용자 결정으로 Contact Identification을 Detection/Tracking과 분리된 독립 상태축으로 정의했다.
- `ID0 Unknown / ID1 Affiliation Known / ID2 Identity Known`은 고정 시간 누적 단계가 아니라 조건 기반 승격 단계로 사용하며, 시간은 필요한 Sensor/Scanner Rule의 개별 조건으로만 둘 수 있게 했다.
- 차량 기본 Radar와 장착형 Scanner가 같은 Identification State를 사용하되 각 장비가 어느 조건에서 어느 단계까지 식별 가능한지는 Sensor Gameplay가 소유하도록 했다.
- 아군은 전용 통신 주파수/IFF를 공유한다는 설정으로, Radar에 Contact로 탐지되는 즉시 거리나 별도 분석시간과 무관하게 Friendly 식별이 완료되도록 했다.
- 기본 Friendly IFF는 등록된 식별 정보까지 전달하는 것으로 두어 정상 아군 Contact가 탐지 즉시 `ID2 Identity Known`까지 갈 수 있게 했다.
- Friendly 즉시 식별은 탐지거리 무제한이 아니라 기존 Passive/Active/Visual Acquisition 중 하나로 먼저 Contact가 탐지된 뒤 Identification이 즉시 완료되는 규칙임을 명시했다.
- IFF 부재는 Hostile 판정 근거가 아니며, 비아군 Contact는 별도 식별 조건이 충족되지 않으면 Unknown으로 유지할 수 있게 했다.
- ID2는 Entity 정체 식별까지만 의미하며 HP·무장·손상 상태 등 상세 전술 정보는 후속 Target Intelligence/Analysis 계층으로 분리하도록 했다.

### v0.64.0 - 2026-08-12

- 사용자 결정으로 Passive Tracking도 지형·대형 구조물의 Scanner 차폐를 받도록 확정했다.
- 동시에 **직접 가시선(LOS)으로 명확히 보이는 Contact는 Radar에서 반드시 탐지**되는 상위 원칙을 추가했다.
- 플레이어에게 분명히 보이는 대상이 Radar에서는 미탐지로 남지 않도록 `Visual Acquisition`을 Passive/Active Scanner와 분리된 탐지 경로로 두도록 했다.
- 직접 LOS가 없는 대상은 기존 Passive Range·Active Scan·Occlusion·Last Known 규칙을 그대로 적용한다.
- Visual Acquisition의 정확한 거리·판정 주기·FOV 의존 여부는 후속 Gameplay 튜닝 대상으로 남겼다.

### v0.63.0 - 2026-08-12

- 사용자 결정으로 Active Scan에 **지형·대형 구조물 차폐**를 적용하도록 확정했다.
- 산·언덕·대형 암반·건물 본체 같은 전술적 대형 Geometry는 Scanner와 Contact 사이에서 탐지를 차폐할 수 있게 했다.
- 가로등·작은 잔해·소형 소품·다른 차량 같은 작은 Object는 기본 Scanner 차폐에서 제외해 사소한 Geometry로 인한 탐지 불안정을 피하도록 했다.
- Scanner 차폐는 Render Occlusion과 분리된 Gameplay 판정으로 두고 후속 구현에서 전용 Collision/Tag/Profile로 Occluder를 명시할 수 있게 했다.
- 정확한 소형/대형 기준과 예외는 월드 제작·플레이테스트 단계에서 조정하도록 남겼다.

### v0.62.0 - 2026-08-12

- 사용자 결정으로 Active Scan을 **차량 중심 360° 전방향 Scan**으로 확정했다.
- 전방 Cone/FOV 제한을 기본 계약에서 제외하고 좌·우·후방 원거리 Contact도 같은 Scan Cycle의 탐지 대상으로 포함했다.
- Heading Up은 화면 표시 기준일 뿐 Active Scan 탐지 방향 제한이 아님을 명시했다.
- 기존 회전 Sweep을 360° Scan 진행 표현으로 유지했다.
- 고저차 표현은 기존 3D Scanner/Stalk 계약을 유지하고 수직 탐지 범위와 지형/장애물 차폐는 후속 Gameplay 결정으로 분리했다.

### v0.61.0 - 2026-08-12

- 사용자 결정으로 Last Known Contact의 Radar 표현을 **기존 모양·관계색을 유지한 Ghosted Contact**로 확정했다.
- Last Known Blip과 Stalk는 마지막 탐지 위치·고도를 유지하면서 밝기/Opacity를 낮춰 실시간 Contact와 구분하도록 했다.
- 정보 신선도 표현은 Ghosting을 기본 채널로 사용하고 `?`, 시계, 점선 원, 추가 Badge 같은 상시 장식을 붙이지 않도록 했다.
- 재탐지 또는 Passive Tracking 전환 시 즉시 정상 밝기로 복귀하도록 했다.
- Contact 만료 시 짧은 Fade Out은 허용하되 반복 Blink/Pulse는 사용하지 않도록 했다.

### v0.60.0 - 2026-08-12

- 사용자 결정으로 Radar 상단 정보 구조를 **좌상단 `Range` / 우상단 `Scan Status`**로 확정했다.
- Scan Status는 별도 큰 Panel 없이 한 자리의 Compact 영역에서 `Ready / Scanning / Cooldown` 상태를 교체 표시하도록 했다.
- Ready는 평상시 약하게 또는 숨김 가능, Scanning은 상태·진행 정보와 기존 Sweep을 함께 사용, Cooldown은 남은 재사용 시간을 짧게 표시하는 방향으로 정했다.
- 후속 `JAMMED`·`OFFLINE` 같은 Scanner 상태도 같은 우상단 Status 영역으로 확장할 수 있게 했다.
- 정확한 Font·Offset·Progress 표현은 후속 시안/UMG 튜닝 대상으로 남겼다.

### v0.59.0 - 2026-08-12

- 사용자 결정으로 Active Scan을 **차량 조작과 전투를 잠그지 않는 독립 Scanner Action**으로 확정했다.
- Scan 중에도 주행·가속/감속·조향을 정상적으로 사용할 수 있도록 했다.
- Scan 중에도 기본 무기 조준·발사·전투 입력을 유지하고 Active Scan 자체가 무기 Lock 상태를 만들지 않도록 했다.
- 차량 정지·속도 제한·조향 제한 없이 Scanner가 별도 Cycle로 동작하도록 했다.
- 후속 Energy·Heat·전자전 확장 시에도 입력 제한은 자동 포함하지 않고 별도 Gameplay 결정이 있을 때만 추가하도록 했다.

### v0.58.0 - 2026-08-12

- 사용자 결정으로 Active Scan의 기본 제약을 **Scan Duration + 재사용 Cooldown**으로 확정했다.
- Scan Cycle 완료 후 Cooldown이 끝날 때까지 새 Active Scan을 시작할 수 없도록 했다.
- Scan Duration과 Cooldown을 Scanner 성능 Profile의 조정값으로 두어 장비 성능 차이를 확장할 수 있게 했다.
- Energy·Heat·Emission/전자전 노출 등 추가 비용은 기본 계약에서 제외하고 후속 Gameplay 시스템으로 분리했다.
- 후속 Radar UI가 `Scanning / Cooldown / Ready` 상태를 구분할 수 있도록 상태 계약을 남겼다.

### v0.57.0 - 2026-08-12

- 사용자 결정으로 Active Scan을 **입력 1회 → Scan 수행 → 완료 시 탐지 결과 갱신**의 One-shot Action으로 확정했다.
- Hold-to-Scan 또는 입력 유지 중 반복 Scan을 기본 방식으로 사용하지 않도록 했다.
- 정상 완료 시 한 Scan Cycle의 원거리 탐지 결과를 Contact Tracking State에 반영하도록 했다.
- Sweep은 Scan 진행 표현이며 개별 Contact를 Sweep Line이 통과할 때마다 결과를 확정하는 계약으로 고정하지 않았다.
- Scan Duration은 Scanner/Profile 조정값으로 남기고 Cooldown·Energy Cost·Heat·사용 제한은 별도 Gameplay 결정으로 분리했다.

### v0.56.0 - 2026-08-12

- 사용자 결정으로 Passive 범위 밖에서 Active Scan으로 찾은 원거리 Contact를 **Last Known Contact**로 일정 시간 유지하는 초기안을 채택했다.
- Last Known 상태에서는 마지막 유효 탐지 위치를 유지하되 실시간 위치를 계속 갱신하지 않도록 했다.
- 후속 Active Scan 재탐지 시 위치를 갱신하고 Passive Tracking 범위 진입 시 실시간 Tracking으로 자동 전환하도록 했다.
- 장시간 재탐지되지 않은 Contact는 제거하는 방향을 기본으로 두되 유지시간·감쇠·표현은 후속 튜닝 대상으로 남겼다.
- 이 규칙은 플레이테스트에서 쉽게 변경할 수 있도록 Contact Tracking State와 Radar 표시 로직을 분리하는 가변 Gameplay 계약으로 기록했다.

### v0.55.0 - 2026-08-12

- 사용자 결정으로 Scanner 탐지 체계를 **근거리 Passive Tracking + 원거리 Active Scan**으로 확정했다.
- Passive Tracking은 별도 Scan 없이 근거리 Contact를 지속 탐지·추적하는 기본 Sensor 동작으로 정의했다.
- Active Scan은 Scanner 성능에 따라 Passive 범위를 넘어 원거리 Contact를 탐지하며, 고성능 Scanner일수록 더 먼 거리까지 Scan할 수 있도록 했다.
- 이번 단계에서는 Active Scan의 핵심 역할을 거리 확장 탐지로 한정하고 식별·은폐 해제·전자전 효과는 자동 포함하지 않도록 했다.
- Radar Zoom은 탐지 성능과 분리된 표시 배율로 유지하고 Scanner 최대 표시 Range는 Active Scan 유효 최대거리 안에서 구성하도록 했다.

### v0.54.0 - 2026-08-12

- 사용자 결정으로 Off-range Edge Bracket을 **Scanner Frame보다 항상 앞쪽 Layer**에 표시하도록 확정했다.
- Frame 분절선과 겹치는 경우에도 선택 Target 방향 표식의 판독을 우선하도록 했다.
- Bracket을 보이게 하기 위해 Scanner Frame을 동적으로 삭제·절단하지 않고 `Frame → Edge Bracket` Layer 순서로 해결하도록 했다.
- 정확한 UMG ZOrder 값은 후속 구현·Visual Style 튜닝에서 정하도록 남겼다.

### v0.53.0 - 2026-08-12

- 사용자 결정으로 Off-range `2-Corner Open Edge Bracket`을 **Scanner 외곽의 Target 방향 위치에 맞춰 회전**하도록 확정했다.
- 두 Corner의 열린 쪽은 항상 Scanner 바깥의 실제 Target 방향을 향하도록 했다.
- Target 방향 변화에 따른 위치·각도 갱신은 방향 추적용 Orientation 변경이며 장식 목적의 지속 회전 Animation과 구분하도록 했다.
- Corner 기본 비율·길이는 유지하고 전체 Orientation만 변경하도록 했다.
- 외곽 타원 접선/법선 기준은 후속 구현에서 일관되게 계산하도록 남겼다.

### v0.52.0 - 2026-08-12

- 사용자 결정으로 표시 범위 안의 4-Corner Target Bracket을 **Screen-Space Upright**로 확정했다.
- Scanner Perspective와 Contact 위치에 따라 Bracket을 기울이거나 압축·왜곡하지 않고 네 Corner의 각도·비율·형태를 일정하게 유지하도록 했다.
- Bracket은 화면상 위치만 선택 Blip을 따라 이동하고 Scanner의 원근 압축은 Surface·Ring·Contact 위치 계산 쪽에만 적용하도록 했다.
- 이 규칙이 선택 상태 판독성을 위한 순수 Visual 계약이며 거리·방향·고저차 Gameplay 의미를 바꾸지 않도록 했다.
- 정확한 Bracket 크기·Corner 길이·선 두께는 후속 Visual Style Preset과 UMG 시안에서 조정 가능하게 남겼다.

### v0.51.0 - 2026-08-12

- 사용자 결정으로 Radar 고저차 Stalk를 **Blip 외곽 바로 앞에서 종료**하고 Solid Blip 내부를 관통하지 않도록 확정했다.
- `◆ / ▼ / ●` 내부에 Stalk 선을 그리지 않아 Contact 분류 실루엣이 뭉개지지 않도록 했다.
- Stalk-Blip 간격은 연결성이 유지될 만큼 최소화하되 Blip 실루엣 판독을 우선하도록 했다.
- 세 Blip 형상 모두 각 외곽 경계를 기준으로 접점을 잡고 동일한 `내부 관통 없음` 원칙을 적용하도록 했다.
- 정확한 End Gap 값은 후속 Visual Style Preset과 실제 UMG 시안에서 조정 가능하게 남겼다.

### v0.50.0 - 2026-08-12

- 사용자 결정으로 Radar 기본 시각 Layer 순서를 **Surface → Range Ring → Stalk → Blip → Target Bracket**으로 확정했다.
- Surface와 Range Ring은 Contact 고도와 무관하게 항상 Contact·Stalk 뒤에 유지해 Blip이나 Stalk를 덮거나 흐리게 하지 않도록 했다.
- 낮은 고도의 Contact도 실제 3D Occlusion처럼 Surface 뒤로 숨기지 않고 정보 판독성을 World Depth 표현보다 우선하도록 했다.
- Target Bracket은 Contact 계층 최상단 선택 강조를 유지하되 기존 Open Bracket 비가림 계약을 그대로 유지하도록 했다.
- Scan Sweep은 별도 동적 효과로 두고 교차 시 Contact 판독을 손상시키지 않도록 후속 ZOrder·Opacity 튜닝 대상으로 남겼다.

### v0.49.0 - 2026-08-12

- 사용자 결정으로 Radar Scanner 기준 평면 Surface를 **아주 약한 Center → Edge Soft Fade**로 확정했다.
- 중앙은 아주 약간 밝고 외곽으로 자연스럽게 사라지게 하되 눈에 띄는 Glow·밝은 원판·Halo 표현은 사용하지 않도록 했다.
- Surface는 Range Ring보다도 낮은 시각 우선순위를 유지하도록 했다.
- Fade는 Scanner Perspective와 동일한 타원형 투영을 따르며 Gameplay 거리·고저차·탐지 계산과 분리하도록 했다.
- 정확한 Center 밝기·Edge 감쇠·Opacity Curve는 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.48.0 - 2026-08-12

- 사용자 결정으로 Scanner 외곽에는 **별도 전방 Chevron·`FWD` Text·Forward Arrow·Heading Marker를 넣지 않도록** 확정했다.
- 전방 방향은 중앙 플레이어 `▲`와 Heading Up 구조만으로 전달하도록 했다.
- 기존 전방 Frame 개방형 실루엣은 시각 형태로만 유지하고 별도 전방 의미 기호를 추가하지 않도록 했다.
- 별도 Gameplay 요구가 후속 정의되지 않는 한 동일 정보를 중복하는 외곽 전방 Marker를 기본 Radar에 추가하지 않도록 했다.

### v0.47.0 - 2026-08-12

- 사용자 결정으로 Radar Scanner에는 **`N / E / S / W` 방위 문자와 상시 Compass Tick을 넣지 않도록** 확정했다.
- Heading Up 기준의 차량 상대 방향 판독을 우선하고 절대 월드 방위를 Radar 외곽 장식으로 중복 표시하지 않도록 했다.
- Cardinal Label·방위 눈금·반복 Tick을 제거해 Scanner Frame·Range Ring·Contact 판독 여백을 유지하도록 했다.
- 후속 절대 방위 요구가 생기더라도 Radar 기본 Frame에 자동 추가하지 않고 별도 Compass/Navigation 표현으로 검토하도록 했다.

### v0.46.0 - 2026-08-12

- 사용자 결정으로 Radar 고저차 Stalk의 Scanner 기준 평면 접점에는 **별도 Dot/Foot Marker를 사용하지 않도록** 확정했다.
- Stalk 선 자체가 기준 평면에서 바로 끝나게 하고 접점 위치를 추가 기호로 중복 강조하지 않도록 했다.
- Surface와 Range Ring이 이미 기준면을 제공하므로 다수 Contact 상황에서 접점 장식으로 시각 노이즈가 누적되지 않게 했다.
- 별도 Gameplay 의미가 후속 정의되지 않는 한 Stalk 접점 Marker를 기본 Visual Style로 추가하지 않도록 했다.

### v0.45.0 - 2026-08-12

- 사용자 결정으로 Radar Range Text의 내부 명도 계층을 **Label·Unit 약하게 / 숫자값 한 단계 밝게**로 확정했다.
- `RANGE`와 `m/km`는 낮은 명도로, `현재 표시 범위 / Scanner 최대 탐지거리` 숫자는 더 밝게 표시하도록 했다.
- 서로 다른 의미색을 추가하지 않고 동일한 중립 색상 계열 안에서 명도 차이만 사용하도록 했다.
- 숫자 강조가 Gameplay Range 상태나 경고 의미를 추가로 나타내지 않는 순수 판독성 규칙임을 명시했다.
- 정확한 Opacity와 밝기 차이는 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.44.0 - 2026-08-12

- 사용자 결정으로 Scan 완료 신규 탐지 Contact의 강조를 **Brightness Flash 1회**로 확정했다.
- Blip 크기·Scale 변화, 확대 Pop·Bounce·확장 Ring·Ripple·추가 Outline은 사용하지 않도록 했다.
- 기존 Contact 관계색을 유지한 채 밝기만 짧게 상승한 뒤 기본 상태로 복귀하도록 했다.
- 선택 Target의 Bracket과 Stalk는 신규 탐지 Flash와 독립적으로 유지하도록 했다.
- 정확한 Flash 지속시간과 Peak 밝기는 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.43.0 - 2026-08-12

- 사용자 결정으로 Unknown `◆`, Hostile `▼`, Friendly `●` Contact Blip의 **기본 시각 크기를 동일하게** 확정했다.
- Blip 크기로 위협도·중요도·식별 수준을 암시하지 않고 기존 모양·관계색·Target Bracket 역할을 유지하도록 했다.
- 실제 공통 Blip Size 값은 후속 Visual Style Preset과 UMG 시안에서 조정 가능하게 남겼다.

### v0.42.0 - 2026-08-12

- 사용자 결정으로 Radar Range Text를 **Scanner Frame 내부 좌상단의 한 줄 Compact Text**로 확정했다.
- 기존 `RANGE 현재 표시 범위 / Scanner 최대 탐지거리` 의미와 m/km 단위 계약은 유지했다.
- 별도 Header Bar·배경 박스·캡슐형 Label Surface는 사용하지 않도록 했다.
- Text는 타원형 Scanner 실루엣과 Contact·Stalk·Sweep 판독을 방해하지 않는 내부 여백에 배치하도록 했다.
- 정확한 Offset·Font Size·Tracking은 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.41.0 - 2026-08-12

- 사용자 결정으로 Radar Sweep의 기본 외형을 **얇은 Cyan 회전선 + 아주 짧은 Fade Trail**로 확정했다.
- Sweep은 Scanner 중심에서 외곽까지 이어지고 Line 바로 뒤의 짧은 반투명 Trail만 회전 방향 보조로 허용하도록 했다.
- Contact·Range Ring·Stalk 판독을 방해하는 넓은 부채꼴 Fill·대면적 Gradient Wedge·지속 발광 면은 사용하지 않도록 했다.
- Scan 종료·취소·실패 시 Sweep Line과 Trail 모두 즉시 사라져 Gameplay Scan 상태와 무관한 잔상을 남기지 않도록 했다.
- 정확한 선 두께·Trail 길이/각도·Opacity는 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.40.0 - 2026-08-12

- 사용자 결정으로 Radar Contact Blip 3종 `◆ / ▼ / ●`의 기본 렌더링을 **Solid Filled Symbol**로 확정했다.
- 기본 Blip에는 별도 Outline과 상시 Glow·Bloom·반복 Pulse를 사용하지 않도록 했다.
- Blip의 관계/지식 상태 색상 계약은 유지하고 선택 강조는 기존 4-Corner Open Bracket이 담당하도록 역할을 분리했다.
- Scan 완료 신규 탐지 Contact에는 기존 확정된 1회 밝기 강조만 허용하고 이후 기본 Solid Filled 상태로 복귀하도록 했다.
- 정확한 Blip 크기와 밝기는 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.39.0 - 2026-08-12

- 사용자 결정으로 Radar 중앙 플레이어 `▲`를 **작은 Solid Marker + 밝은 중립 회청색**으로 확정했다.
- 별도 Outline을 사용하지 않고 `AccentTactical` 선택 Target Bracket보다 낮은 시각 우선순위를 갖도록 했다.
- 평상시 Pulse·점멸·회전·반복 Glow 효과는 사용하지 않도록 했다.
- 플레이어 Marker는 Scanner 중심 기준점에 고정하며 정확한 크기와 회청색 값은 후속 Visual Style Preset에서 조정 가능하게 남겼다.
- 후속 색상 조정 시에도 Contact 관계색이나 선택 상태로 오인되지 않는 구분 원칙을 유지하도록 했다.

### v0.38.0 - 2026-08-12

- 사용자 결정으로 표시 Range 밖 선택 Target의 Scanner 외곽 표식을 **2-Corner Open Edge Bracket**으로 확정했다.
- 외곽 타원선에 붙는 두 개의 짧은 Corner를 Target 바깥 방향으로 열어 방향만 전달하도록 했다.
- 적대 Contact `▼`와의 형태 혼동을 피하기 위해 독립 삼각형 화살표는 기본 Off-range 표식으로 사용하지 않도록 했다.
- 범위 안 선택 Bracket과 동일한 `AccentTactical` 계열을 사용하되 반복 Pulse·점멸·회전은 사용하지 않도록 했다.
- 표식 위치·길이로 실제 거리를 암시하지 않고 현재 표시 Range 밖이라는 상태와 방향만 전달하도록 했다.
- 정확한 Corner 길이·선 두께·외곽선 간격은 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.37.0 - 2026-08-12

- 사용자 결정으로 표시 범위 안 선택 Target의 기본 Bracket을 **4개의 독립된 Open Corner Bracket**으로 확정했다.
- Bracket은 닫힌 사각 테두리를 만들지 않고 Blip 바깥 네 모서리만 사용해 기본 `◆ / ▼ / ●` 기호와 고저차 Stalk를 가리지 않도록 했다.
- Bracket은 `AccentTactical` 계열 선택 강조를 사용하되 Contact 관계색은 기존 Blip에 유지하도록 역할을 분리했다.
- 선택 유지 중에는 고정 표시하며 불필요한 회전·맥동·반복 점멸을 사용하지 않도록 했다.
- 표시 범위 밖 선택 Target의 Scanner 외곽 방향 표식은 별도 Off-range 계약으로 유지해 이번 4-Corner 결정과 분리했다.
- Bracket의 정확한 크기·Corner 길이·선 두께·여백은 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.36.0 - 2026-08-12

- 사용자 결정으로 Radar 고저차 Stalk의 기본 외형을 **얇은 연속 수직선**으로 확정했다.
- Stalk는 해당 Contact의 관계 색상을 유지하되 Blip보다 낮은 밝기·Opacity로 표시하도록 했다.
- 기본 Stalk에는 점선·중간 Tick·눈금·반복 Marker를 사용하지 않도록 했다.
- 선택 Target 강조는 Stalk가 아니라 기존 별도 Target Bracket이 담당하도록 역할을 분리했다.
- 정확한 선 두께와 Opacity는 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.35.0 - 2026-08-12

- 사용자 결정으로 Radar Range Ring의 기본 외형을 **얇은 연속 타원선 + 낮은 명도**로 확정했다.
- 25% / 50% / 75% 거리 의미와 100% Scanner 외곽 범위 계약은 변경하지 않았다.
- 기본 Ring에는 Tick·눈금·분절선·장식 Marker를 사용하지 않도록 했다.
- Tactical Cut은 Scanner Frame에 집중시키고 Range Ring은 거리 판독을 위한 조용한 기준선으로 유지하도록 했다.
- 정확한 선 두께와 Opacity는 후속 Visual Style Preset에서 조정 가능하게 남겼다.

### v0.34.0 - 2026-08-12

- 사용자 결정으로 Radar Scanner 타원의 초기 Perspective를 세로축/가로축 약 `42~48%`, 중심값 약 `45%`의 중간 원근감으로 확정했다.
- 이 비율은 Gameplay/Sensor 계약이 아닌 Visual Preset으로 정의해 후속 TPS 캡처와 UMG 시각 검토에서 조정 가능하도록 했다.
- Perspective 조정이 Contact 거리·Sensor 탐지·Range Preset·Stalk 의미에 영향을 주지 않도록 시각 표현과 Gameplay 계산을 분리했다.
- 구현 시 타원 눌림 정도를 가능하면 Scanner Visual Style 또는 Brush/Material 파라미터로 조정하고 Gameplay 코드 상수로 고정하지 않도록 했다.

### v0.33.0 - 2026-08-12

- 사용자 결정으로 Radar Scanner 기준 평면을 아주 옅은 **반투명 타원 Surface**로 확정했다.
- 기준 평면은 고저차 Stalk의 기준면을 읽기 위한 시각적 바닥만 제공하고 지형·도로·월드 Geometry는 표시하지 않는다.
- 기준 평면에는 기존 25% / 50% / 75% Range Ring 3개를 유지하며 촘촘한 Grid·좌표 격자·반복 Mesh Pattern은 사용하지 않는다.
- Surface는 Contact·Stalk·Range Ring보다 시각적으로 약하게 유지해 3D Scanner의 공간 인지 기능을 보조하도록 했다.

### v0.32.0 - 2026-08-12

- 사용자 결정으로 Radar Scanner 외곽을 완전 폐쇄형 타원이 아닌 **분절형 전술 타원 Frame**으로 확정했다.
- Heading Up 기준 후방·좌우 Frame은 전체 타원 실루엣을 읽을 수 있게 유지하고 전방 일부는 의도적으로 개방하도록 했다.
- `Solid Core + Tactical Cut`에 맞는 작은 분절은 허용하지만 Range Ring·Contact·Stalk·Target Bracket 판독을 방해하는 과도한 장식선은 금지했다.
- Frame은 Texture2D/Brush 등 하나의 교체 가능한 시각 자산 단위로 유지하고 Border 조각 조합 방식은 사용하지 않는다.
- 정확한 Cut 개수·위치·선 두께·픽셀 치수는 후속 시각 시안 조정 항목으로 남겼다.

### v0.31.0 - 2026-08-12

- VehiclePanel 기록 감사에서 상세 SSOT의 현재 버전이 `InGameUIVehiclePanelSpec.md v0.20.0`임을 재확인했다.
- StyleSpec의 승인 범위 안내에 남아 있던 `v0.2.0` 상세 문서 참조를 `v0.20.0 Visual Layout Accepted / Runtime·UMG Pending`으로 교정했다.
- 오늘 RadarPanel 결정 반영에 맞춰 문서 최근 갱신일을 2026-08-12로 동기화했다.
- VehiclePanel 상세 SSOT의 사용자 확정 내용 자체에는 누락이나 의미 변경이 없어 추가 수정하지 않았다.

### v0.30.0 - 2026-08-12

- 사용자 결정으로 현재 Radar 표시 범위 밖의 일반 Contact는 Scanner 내부에서 숨기도록 확정했다.
- 현재 선택 Target이 표시 범위 밖일 경우에는 Scanner 외곽선에 해당 방향을 가리키는 작은 Bracket/표식을 유지하도록 했다.
- 선택 Target이 다시 표시 범위 안으로 들어오면 외곽 표식을 제거하고 정상 Blip + Target Bracket으로 복귀하도록 했다.

### v0.29.0 - 2026-08-12

- 사용자 결정으로 Radar Zoom을 연속식이 아닌 단계식 Range Zoom으로 확정했다.
- Wheel Up/Down 한 칸마다 다음 작은/큰 Range Preset으로 이동하도록 했다.
- Zoom 단계 값은 Scanner 성능 Profile이 소유하며 최대 탐지거리 안에서 구성한다. 예시 4 km Scanner는 `0.5 / 1 / 2 / 4 km` 단계를 사용할 수 있다.
- 특정 km 단계값을 모든 Scanner에 하드코딩하지 않고 Scanner 성능별 Preset으로 유지하도록 했다.

### v0.28.0 - 2026-08-12

- 사용자 결정으로 Radar Range Ring을 3개로 확정했다.
- Ring은 현재 표시 범위 기준 25% / 50% / 75%에 배치하고 Scanner 외곽선은 100% 범위를 의미하도록 했다.
- Ring에는 숫자를 상시 표기하지 않고 상대 거리 기준선으로만 사용한다.
- 좌상단 Range 표시는 `현재 표시 범위 / Scanner 최대 탐지거리` 형식으로 확정했다. 예: `RANGE 1.0 / 4.0 km`.

### v0.27.0 - 2026-08-12

- 사용자 결정으로 Radar 좌상단에 현재 Range를 작은 m/km Text로 표시하도록 확정했다.
- Scanner 내부 Range Ring을 현재 표시 범위의 상대 거리 기준선으로 사용하도록 했다.
- 마우스 휠 Up=Zoom In, Wheel Down=Zoom Out으로 Radar 표시 범위를 조절하도록 확정했다.
- Radar Zoom은 Sensor 탐지 성능과 분리된 표시 배율이며, 최대 Zoom Out 범위는 현재 Scanner의 실제 유효 최대 탐지거리로 제한했다.
- Zoom 변경 시 Contact의 화면 위치만 현재 표시 범위 기준으로 재정규화하고 실제 World Distance나 탐지 여부는 변경하지 않도록 했다.

### v0.26.0 - 2026-08-12

- 사용자 결정으로 Scan 완료 시 새로 탐지된 Contact Blip을 짧게 한 번 밝게 강조한 뒤 정상 상태로 복귀하도록 확정했다.
- 신규 탐지 강조는 기본 Blip 모양·관계 색상·Target Bracket·고저차 Stalk를 변경하지 않는 일시적 강조로 제한했다.
- 기존 추적 Contact에는 신규 탐지 강조를 반복 적용하지 않으며 지속 점멸·반복 Pulse는 사용하지 않는다.

### v0.25.0 - 2026-08-12

- 사용자 결정으로 Scanner 중앙의 플레이어 기준 마커를 Heading Up 방향의 작은 `▲`로 확정했다.
- Radar Sweep은 상시 시각 효과가 아니라 실제 Scan 동작 중에만 표시하는 기능 표현으로 확정했다.
- Scan 시작 시에만 Sweep이 Scanner 중심에서 회전하고, Scan 종료·취소·실패 시 즉시 사라지도록 했다.
- Gameplay Scan 상태와 무관한 상시 반복 Sweep 애니메이션은 금지했다.

### v0.24.0 - 2026-08-12

- 사용자 결정으로 Radar Contact 기본 Blip을 `미확인=작은 마름모`, `적대=역삼각형`, `아군=원`으로 확정했다.
- 현재 선택 Target은 기본 Blip 모양을 유지하고 바깥에 별도 Bracket을 추가하는 중첩 표현으로 확정했다.
- 선택 Bracket은 기본 식별 모양과 고저차 Stalk를 가리지 않으며 선택 해제 시 Bracket만 제거하도록 했다.
- 색상만이 아니라 모양 자체로 Contact 관계를 판독할 수 있게 했다.

### v0.23.0 - 2026-08-12

- 사용자 결정으로 Radar Contact의 고저차를 Blip과 기준 평면 사이의 수직 Stalk로 표현하도록 확정했다.
- 높은 Contact는 평면 위 Blip + 하향 Stalk, 낮은 Contact는 평면 아래 Blip + 상향 Stalk 구조를 사용한다.
- 동일 고도 근처에서는 Stalk를 생략·최소화하고, Stalk 길이는 Radar 표시 범위에 맞춰 정규화·Clamp하도록 했다.

### v0.22.0 - 2026-08-12

- 사용자 결정으로 RadarPanel 본체 형태를 평면 원형 Radar가 아닌 **원근형 타원 3D Scanner**로 확정했다.
- Heading Up과 하단 중앙 배치는 유지하며 플레이 차량을 Scanner 중심 기준점으로 사용한다.
- Radar는 지형 미니맵이나 SceneCapture가 아니라 Sensor 데이터 기반 공간 인지 UI이며 방향·거리·고저차 표현을 목표로 한다.
- Scanner Frame은 실제 Texture2D/Brush 자산으로 구성하고 Border 조각을 쌓아 외형을 만드는 방식은 사용하지 않는다.

### v0.21.0 - 2026-08-11

- `InGameUIVehiclePanelSpec.md v0.20.0`의 사용자 확정 내용을 StyleSpec의 현재 VehiclePanel 요약에 동기화했다.
- 예전 속도 원호·세로 D/N/R·Armor 현재/최대 텍스트 요약을 비대칭 실제 RPM Gauge, 단일 Gear Slot, 6방향 Armor 우측 세로 Bar 구조로 교정했다.
- Shield/Integrity의 중앙 `999/999`와 Fill 끝 3단 Chevron 재생·수리 문법을 현재 승인 요약에 반영했다.
- 과거 Changelog의 당시 VehiclePanel 설계 기록은 역사 이력으로 유지했다.

### v0.20.0 - 2026-08-10

- D1-10A 외부 Style/Density/Scale Context Bridge와 Button Interaction/Focus Bridge를 구현하고 공식 Build·두 Contract Automation PASS를 확인했다.
- D1-10B Designer Tree 자동화를 위해 UE 5.8 Python 공개 API를 조사하고, Python에서 비노출인 WidgetTree만 보완하는 최소 Editor Bridge Source를 추가했다.
- Editor Bridge는 컴파일 성공했으나 현재 열린 CarFight Editor가 제품 DLL을 점유해 공식 Link가 LNK1104로 중단됐다.
- Base Widget 5종 Asset은 아직 생성하지 않았으며 D1-10 전체는 NOT PASS, D1-11은 Not Started로 유지한다.
- commit·push는 수행하지 않았다.

### v0.19.0 - 2026-08-10

- D1-09B에서 실제 FontFace 8개·Runtime Composite Font 2개·Semantic Icon 18개·Style/Density/1080 Layout DataAsset을 생성했다.
- Pretendard·IBM Plex Mono 공식 OFL 원문을 `UE/ThirdPartyNotices/Fonts/`에 바이트 그대로 보존하고 Readback에서 공식 원문과 일치함을 검증했다.
- `CFUISubsystem` Config Soft Reference가 Default Style·Standard Density·1080 Layout을 가리키는 것을 새 프로세스 CDO Readback으로 확인했다.
- 최종 전체 CarFight 회귀 Process `e73311a38be043d48d512ab90afd258f`가 51/51·필수 24/24 Success로 통과했다.
- D1-09B를 PASS로 닫고 D1-10은 Not Started로 유지했다. 후속 1440p·21:9·32:9 Profile은 계속 Deferred다.
- commit·push는 수행하지 않았다.

### v0.18.0 - 2026-08-10

- D1-08V를 공식 Editor Build와 `StyleDataContract` Automation PASS로 닫았다.
- D1-09A에서 `UFont` Binding, 1080p HUD Layout Data, Density Data, `UCFUISubsystem` Config Soft Reference·Native CDO Fallback을 실제 구현했다.
- D1-09A 최종 공식 Build Job `9d9aefb3803e4556bd4559afded0f4ed`와 전체 Automation Process `18bea9b5952e4444b618e776cb2e2571`을 PASS 증거로 기록했다.
- 전체 51/51·필수 24/24 Success, D1-09A 네 Contract 각각 Warning 0 / Error 0을 확인했다.
- 실제 Font·Icon·Style/Density/Layout Unreal Asset과 Config 연결은 D1-09B로 유지하며 아직 시작하지 않았다.
- commit·push는 수행하지 않았다.

### v0.17.0 - 2026-08-07

- D1-09A~12의 구현 세부를 `InGameUIAssetizationSpec.md v0.1.0`으로 분리해 StyleSpec은 공통 디자인·Token SSOT 역할을 유지했다.
- Pretendard와 IBM Plex Mono의 공식 라이선스를 확인해 모두 SIL Open Font License 1.1이며 상용 게임 번들 사용 적격성을 PASS로 기록했다.
- Font 재배포 시 Copyright Notice와 OFL 전문 보존이 필요하며 Shipping 패키지의 최종 고지 노출은 Release Packaging 검증으로 분리했다.
- Epic 최신 Font 문서를 기준으로 `UFont` Runtime Composite + `UFontFace` Font Face + `FSlateFontInfo` 사용 구조를 확정했다.
- `FCFUIFontAssets`는 Style Data가 항상 사용하는 `UFont` 두 개를 Hard Reference하고 Style/Density/Layout DataAsset 자체는 `UCFUISubsystem`이 Config Soft Reference로 한 번 해석하도록 계약을 정정했다.
- D1 P0 Font는 Static TTF/OTF 4 Weight, Runtime Cached Composite, Hinting Default, Lazy Load, Metrics를 기본 제작안으로 고정했다.
- Source·Config·Unreal Asset·Build·Automation·PIE·commit·push는 수행하지 않았다.

### v0.16.0 - 2026-08-07

- 사용자가 에디터 종료가 불가능한 상태에서 D1-09~12의 구현 준비만 진행하도록 승인해 Source·Config·Unreal Asset·Build·Automation 추가 실행 없이 준비 계약을 잠갔다.
- 실제 소스 조사 결과 D1-08에는 Font Family 이름만 있고 실제 Font Asset Binding이 없으며 `UCFHUDLayoutData`, `UCFUIDensityData`, `FCFHUDSlotLayout`도 미구현임을 확인했다.
- 위 직접 선행 누락을 별도 기능으로 확장하지 않고 D1-09A Asset Binding·Layout·Density C++ 보완으로 고정했다.
- D1-09B Active 자산을 `DA_CFUIStyle_Default`, Density 3종, `DA_CFHUDLayout_1080_16`, 실제 Font·P0 Semantic Icon으로 한정하고 1440p·21:9·32:9 Profile 생성은 Deferred했다.
- `DA_CFHUDLayout_1080_16`의 7개 Slot Anchor·Offset·DesiredSize·ZOrder를 D1-07 USER PASS Mockup의 실제 1920×1080 값으로 고정했다.
- D1-10 Base Widget 5종, D1-11 기능 없는 `WBP_CFInGameHUD` Visual Prototype, D1-12 기존 Pause 계약을 보존하는 Blueprint Visual Migration의 정확한 Allowlist와 금지 범위를 정의했다.
- 단계별 Build·Automation·Asset Readback·Blueprint Compile·1920×1080 Preview·Pause PIE 검증 행렬과 중단 조건을 추가했다.
- commit·push는 수행하지 않았다.

### v0.15.0 - 2026-08-07

- UI-DESIGN-GATE D1-08 C++ Style Data Type 구현을 시작해 `UCFUIStyleData`와 Color·Typography·Spacing·Shape·Opacity·Motion·Panel·Button·StatusBar·InfoRow·Alert·Semantic Icon 타입을 신규 추가했다.
- D1-05~07 승인값은 DataAsset에서 변경 가능한 기본 Preset으로 유지하고 C++ 값은 DataAsset 누락 시 Native Safe Fallback으로만 사용하도록 구현했다.
- 실제 Font·Icon 경로는 C++에 하드코딩하지 않고 Font Family Role과 Semantic Icon ID·Soft Reference 계약으로 분리했다.
- `CarFight.UI.D1_08.StyleDataContract` Automation 소스를 추가해 기본 Token, 의미 해석과 접근성 안전 하한 검증을 준비했다.
- 기존 `CFPlayerController`, `UCFUISubsystem`, Pause·AimReticle·TargetSelect와 Unreal Asset은 수정하지 않았다.
- 현재 상태는 Code Applied / Static Readback PASS / Official Build·Automation Pending이며 에디터 실행 중이라 공식 빌드는 아직 실행하지 않았다.
- commit·push는 수행하지 않았다.

### v0.14.0 - 2026-08-07

- 사용자가 외곽 재배치가 적용된 `SR-1080-16` 수정본을 승인해 D1-07 Phase 1 Static Review를 USER PASS로 승격했다.
- 1080p Phase 1 디자인 기준은 확정됐으며 SR-A·C·D·E 추가 상태 Mockup과 1440p·21:9·32:9 확장은 현재 Gate 비차단 후속 검토로 유지했다.
- UI-DESIGN-GATE 전체는 D1-08~12의 C++ Style Data, `DA_CFUIStyle`·Layout/Density Asset, Base Widget·Visual Prototype과 실제 Font·Icon 자산/라이선스 확인이 남아 있어 Assetization Pending으로 유지했다.
- Runtime Capture·Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.13.0 - 2026-08-07

- 사용자 시각 피드백에 따라 `SR-1080-16`의 고정 HUD를 안전영역 안에서 가능한 한 화면 외곽에 배치하는 Phase 1 원칙을 추가했다.
- 1440p 기본 `SafeMargin=64`를 보존하면서 1080p 전용 `HUDOuterMargin1080=32 Design Unit`, 실효 약 24px Override를 추가했다.
- Mission·Vehicle은 좌측, Target·Weapon은 우측, Alert·Radar는 상·하 방향으로 바깥쪽 배치하고 패널 크기·내부 정보 밀도는 유지하도록 규정했다.
- 1080p Override는 1440p·21:9·32:9 Deferred Layout Profile에 자동 전파하지 않고 후속 확장 Review에서 재판정하도록 경계를 명시했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.12.0 - 2026-08-07

- D1-07 Phase 1 사용자 Gate를 `SR-1080-16` 1920×1080 / 16:9 하나로 동기화했다.
- `SR-1440-16`, `SR-1440-21`, `SR-1440-32`는 기존 Mockup과 확장 규칙을 보존하되 후속 Layout Profile 검토로 Deferred했다.
- Phase 1 상태 세트는 먼저 Combat Busy를 검토하고 필요 시 같은 1920×1080에서 SR-A·C·D·E를 추가하도록 정리했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.11.0 - 2026-08-07

- D1-07용 `SR-1080-16`, `SR-1440-16`, `SR-1440-21`, `SR-1440-32` Static Mockup을 준비했다.
- `InGameUIStaticReview.md v0.1.0`에 해상도별 좌표·Typography 하한·PASS/FAIL 체크리스트와 전역 FAIL 조건을 연결했다.
- 대표 Mockup은 동일한 Combat Busy Mock View Data를 사용해 화면비 차이만 비교하도록 했다.
- 1080p는 단순 0.75 Typography 축소가 아니라 Heading 22, Body 16, Label 14, Caption 12, Speed 38, Weapon Primary 30 등 유효 하한을 적용했다.
- 21:9·32:9는 중앙 2560×1440 Mockup을 각각 X 440, X 1280에서 재사용해 Persistent HUD 중앙 유지 계약을 파일 구조로 보존했다.
- Mockup Preparation은 완료됐지만 디자인 사용자 판정과 Runtime Capture는 Pending이므로 UI-DESIGN-GATE PASS로 승격하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.10.0 - 2026-08-07

- 사용자가 D1-06 Font·Icon 추천안을 전부 승인했다.
- 기본 UI Font를 Pretendard, 숫자·기술 수치 Font를 IBM Plex Mono로 확정하되 실제 Font 파일은 Widget이 아니라 Style Data의 Family Role이 소유하도록 했다.
- Font Family 교체가 Gameplay·Presenter·View Data·Widget 구조 변경 없이 Style Data 또는 Theme Override만으로 가능해야 한다는 불변 조건을 추가했다.
- Icon 기본 스타일을 `Solid Core + Tactical Cut`, 24×24 Base Grid, 기본 표시 크기 16/20/28과 Weapon 18/20으로 확정했다.
- Shield=Energy Field/Hex·Arc, Armor=Chamfered Plate, Integrity=Vehicle Chassis, Weapon=Side/Profile Silhouette를 기본 Semantic Icon 형태로 승인했다.
- Target는 관계색을 유지한 Marker + Cyan Selection Corner, Lock은 2~4개의 독립 Arc/Corner Segment, Radar Contact는 관계색 + 기하 Symbol 조합을 기본 Preset으로 승인했다.
- Font·Icon은 실제 Asset 경로가 아니라 Semantic Role / Icon ID를 통해 교체 가능하도록 기록했다.
- 실제 Font·Icon Unreal Asset 생성, 배포 라이선스 최종 확인, D1-07 Static Review는 수행하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.9.0 - 2026-08-07

- 사용자가 D1-05 Style Token·Base Widget·Density Profile·WeaponPanel Compact Token 추천안을 전부 승인했다.
- 승인 수치를 Widget 하드코딩 값이 아니라 Style·Layout·Density Data Asset이 소유하는 변경 가능한 기본 Preset으로 확정했다.
- Typography에 `ValueS 20 / SemiBold / LineHeight 24`, 1080p 유효 하한 14를 추가했다.
- Compact Panel의 오래된 `Padding 16 / Header 40`을 Density SSOT `12 / 36`으로 통일했다.
- StatusBar 높이를 Compact·Standard·Expanded별로 정의하고 InfoRow Label Width를 `96 / 120 / 144`로 Density화했다.
- Base Widget에 Data Asset 변경 전파, 명시적 Override, Blueprint Visual Class 교체와 Native Safe Fallback 원칙을 커스터마이징 불변 조건으로 추가했다.
- D1-05를 `User Accepted`로 승격했으며 D1-06 Font·Icon과 D1-07 실제 Static Review는 Pending으로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.8.0 - 2026-08-07

- CarFight 전체 UI의 Style·Layout·Density·User Override 해석 순서와 접근성·Reticle Protection 안전 하한을 상세화했다.
- 1080p 유효 글자 크기 하한과 1440p 기준 Typography 보정 규칙을 추가했다.
- Compact·Standard·Expanded Density의 PanelPadding·Gap·Header·InfoRow·Icon·Typography Scale 실제 기본값 초안을 정의했다.
- 공통 Base Widget에 StyleData·DensityPreset·Override를 전달하는 공통 계약, 상태 적용 순서와 `NotApplicable → Collapsed` 규칙을 추가했다.
- 1920×1080, 2560×1440, 3440×1440, 5120×1440 Static Review ID·필수 장면·해상도별 PASS·전역 FAIL 기준을 문서화했다.
- D1 세부 단계를 Style·Base Widget·Density·Compact Token User Review, Font·Icon Review, Static Review, 이후 Source·Asset 구현 순서로 재정렬했다.
- 현재 상태 설명에서 WeaponPanel을 `Compact Direction Accepted / Compact Token Detailed Draft·User Review Pending`으로 정합화하고 과거 Visual Layout 승인 기록은 Changelog 이력으로만 유지했다.
- 실제 Font·Icon·C++·Blueprint·Data Asset·Static Capture는 생성하거나 실행하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.7.0 - 2026-08-07

- 사용자가 커스터마이징 가능 구조를 WeaponPanel 한정이 아닌 CarFight 전체 UI의 필수 설계 원칙으로 확정했다.
- Style·Layout·Density Data Asset과 User Override를 분리하고 Visual Widget Class 교체, 안정적인 PanelId·SlotId와 기본 Preset 상속·복제 계약을 추가했다.
- Compact·Standard·Expanded·Custom Density Profile과 16:9·21:9·32:9 Layout Profile을 전체 UI 공통 구조로 정의했다.
- 자동 Layout, 미지원 항목 Collapse와 빈 공간 제거, Canvas 고정 좌표 사용 제한을 필수 규칙으로 승격했다.
- WeaponPanel은 기존 넓은 고정 치수보다 화면 점유를 줄이는 Compact Revision 방향으로 승인됐으며 `464×360`은 최대 Slot 경계로 재해석했다.
- 이전 `432×224` Selected Card와 `136×96` Tile 수치는 초기 Wireframe 참고값으로 전환하고 정확한 Compact Token은 Static Review Pending으로 남겼다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.6.0 - 2026-08-07

- 사용자가 `D1-WEAPON-PANEL` 상세 구조를 승인해 `Visual Layout Accepted`로 승격했다.
- `InGameUIWeaponPanelSpec.md v0.2.0`과 `CFWeaponPanel_1440p.xml`을 WeaponPanel의 승인된 시각 계약으로 확정했다.
- 우하단 `464×360` 투명 Root, `432×224` Selected Card, Primary 1개·Secondary 최대 2개·FireStateStrip과 최대 3개 Compact Tile 또는 `2 + Overflow` 구조를 유지한다.
- Ammo·Charge·Heat·Cooldown·Reload·LauncherSequence 공통 채널과 탄약형·에너지형·런처형·복합형 표시 규칙을 승인 범위에 포함했다.
- 미지원 Runtime 채널 숨김, VehiclePanel `896×416`, Radar 위치와 중앙 Reticle Protection 영역을 보호한다.
- Style Token·Base Widget·Font·Icon과 실제 1080p·1440p·울트라와이드 Static Review는 Pending으로 유지한다.
- CommonUI 판단, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.5.0 - 2026-08-07

- `InGameUIWeaponPanelSpec.md v0.1.0`과 `CFWeaponPanel_1440p.xml`을 D1-WEAPON-PANEL 상세 소유 산출물로 추가했다.
- 승인된 우하단 `464×360` Slot 안에서 투명 Root, `432×224` Selected Card와 조건부 Compact Rail 구조를 정의했다.
- 선택 무기는 3px Accent·Corner Marker·대형 Primary Channel로 강조하고 비선택 무기는 최대 3개 Tile 또는 `2 + Overflow`로 축약했다.
- Primary Resource 1개, Secondary 최대 2개와 FireStateStrip 한 줄의 화면 점유 상한을 정의했다.
- Ammo·Charge·Heat·Cooldown·Reload·LauncherSequence 공통 채널과 탄약형·에너지형·런처형·복합형 표시 규칙을 연결했다.
- 현재 다중 무기·Ammo·Charge·Heat·Reload Runtime이 없을 때 가짜 값을 표시하지 않고 관련 Rail·Channel을 숨기도록 했다.
- D1-VEHICLE-PANEL `896×416` Visual Layout과 승인된 전체 TPS HUD 배치는 변경하지 않았다.
- WeaponPanel은 Detail Draft / User Review Pending이며 CommonUI 판단, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.4.0 - 2026-08-07

- 사용자가 D1-VEHICLE-PANEL의 최종 Visual Layout을 승인했다.
- VehiclePanel을 896×416 가로형으로 확장하고 좌측 원호형 속도계·우측 Armor Map 구조를 확정했다.
- Armor 실루엣은 왼쪽을 차량 전방으로 고정하고 좌=Front, 상=Right, 우=Rear, 하=Left로 바인딩했다.
- 방향 텍스트를 제거하고 상부 Badge는 좌상단, 하부 Badge는 우하단에 배치했다.
- Shield를 상단 Cluster와 Integrity 사이, Integrity를 최하단 전체 폭 Bar로 확정했다.
- 부품 손상 목록은 고정 VehiclePanel에서 제외했다.
- Weapon Panel은 다음 상세 설계 대상으로 유지하고 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.3.0 - 2026-08-07

- `InGameUIVehiclePanelSpec.md v0.1.0`과 `CFVehiclePanel_1440p.xml`을 VehiclePanel 상세 소유 산출물로 추가했다.
- 좌하단 560×416 안에서 속도·D/R/N·Drive State·Handbrake, Shield, 6방향 Armor, Integrity와 조건부 부품 손상 영역을 정의했다.
- Front·Left·Right·Rear는 Top View Body Map, Top·Bottom은 우측 별도 행으로 분리했다.
- 각 방향 Armor를 단일 전체 퍼센트로 합치지 않고 방향별 Current/Maximum과 독립 상태로 표시하도록 했다.
- 현재 실제 Gear 번호와 부품 손상 Runtime이 없음을 반영해 가짜 기어·부품 값을 표시하지 않도록 했다.
- Vehicle Panel은 Draft / User Review Pending, Weapon Panel은 Pending으로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.2.0 - 2026-08-07

- 사용자 검토를 통해 HUD 기본 시점을 외부 3인칭 차량 TPS, 기본 화면비를 16:9로 확정했다.
- 좌상단 Mission, 상단 중앙 Alert, 우상단 Target, 좌하단 Vehicle, 하단 중앙 Radar, 우하단 Weapon 배치를 승인 상태로 기록했다.
- 이전 좌하단 Radar와 중앙 하단 독립 Speed Cluster 배치를 폐기하고 속도를 Vehicle Panel 범위로 이동했다.
- 내 차량 정보와 무기창의 내부 구성은 배치 승인에서 분리해 별도 상세 설계 Pending으로 남겼다.
- Target Panel은 기본 차량 이미지를 사용하지 않고 Scan 진척에 따라 정보를 공개하는 Compact 구조를 유지했다.
- 향후 HUD 배치 커스터마이징을 위해 Slot ID와 Layout Profile 기반 확장 경계를 추가했으며 실제 편집·저장 기능은 구현하지 않았다.
- CommonUI 판단은 다루지 않았고 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.1.0 - 2026-08-06

- 승인된 차량 탑재형 전술 인터페이스와 C형 70:20:10 균형을 실제 제작 규격으로 전개했다.
- `DA_CFUIStyle`의 Color·Typography·Spacing·Shape·Opacity·Motion·Element Token을 정의했다.
- `WBP_CFButtonBase`, `WBP_CFPanelBase`, `WBP_CFStatusBar`, `WBP_CFInfoRow`, `WBP_CFAlertItem`의 책임·Widget Tree·입력·상태 규칙을 정의했다.
- 2560×1440 HUD 좌표, 중앙 Reticle Protection 영역과 해상도 확장 규칙을 정의했다.
- `ConceptArt/CFHUDWireframe_1440p.xml` Wireframe 초안을 연결했다.
- UI-DESIGN-GATE는 User Review Pending이며 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

---

## 16. Migration

- v0.10.0부터 D1-06 Font·Icon 기본 Preset은 User Accepted다. Font Family와 Icon Set은 Style/Icon Data에서 교체 가능해야 하며 Widget·Gameplay 코드에 실제 Asset 경로를 직접 하드코딩하지 않는다.
- 기본 UI Font Role은 Pretendard, Numeric Role은 IBM Plex Mono를 사용하되 실제 Font 파일 Import 전 라이선스·재배포 조건을 원문 기준으로 확인한다.
- Icon은 Semantic ID를 먼저 정의하고 실제 Brush·Vector·Texture는 교체 가능한 Icon Set이 소유한다.
- Shield·Armor·Integrity·Weapon·Target·Lock·Radar의 승인 형태는 기본 Preset이며 D1-07 Static Review에서 작은 크기 판독성에 따라 Icon Set Data만 보정할 수 있다.
- v0.9.0부터 D1-05 Style·Base Widget·Density 기본값은 User Accepted다. 다만 승인값은 **기본 Preset**이며 Data Asset에서 조정 가능해야 한다.
- 승인된 Token이 존재하는 값은 신규 Widget Blueprint에 Literal로 중복 하드코딩하지 않는다.
- Compact Panel은 `PanelPadding 12 / HeaderHeight 36`을 Density SSOT로 사용하며 과거 `16 / 40` 값은 현재 기준이 아니다.
- `ValueS`는 20 / SemiBold / LineHeight 24, 1080p 유효 하한 14를 기본값으로 사용한다.
- StatusBar와 InfoRow 크기는 Density Data에서 해석하고 Standard 고정값을 Compact·Expanded Widget에 복사하지 않는다.
- D1-05 승인 이후 시각 취향 조정은 가능한 한 Style·Density·Layout Data Asset 수정으로 처리하고 Gameplay·Presenter 계약 변경을 요구하지 않는다.
- 향후 구현에서는 `NotApplicable`과 사용자 숨김을 `Collapsed`, `Unknown`·`Unavailable`·`KnownZero`를 공간 유지 상태로 구분한다.
- 1080p에서 Geometry 0.75 축소를 사용해도 Typography와 Interactive Hit Area는 문서의 유효 하한을 별도로 적용한다.
- D1-07 Phase 1 Static Review는 `SR-1080-16` 사용자 USER PASS로 완료됐다. `SR-1440-16`, `SR-1440-21`, `SR-1440-32`는 후속 Layout Profile 확장 시 재사용한다.
- Density 전환은 View Data 계약과 Gameplay 의미를 변경하지 않으며 Layout Profile의 Anchor·ZOrder를 변경하지 않는다.
- v0.7.0부터 CarFight 전체 UI는 Gameplay System → Presenter/ViewModel → Blueprint Visual Widget → Style·Layout·Density Data Asset → User Override 소유권을 사용한다.
- 기존 Widget의 직접 색상·폰트·Padding·Position 값은 즉시 삭제하지 않지만 신규 구현과 수정 시 공통 Token 또는 명시적 Override로 이동한다.
- 기존 기능 Widget을 교체 가능한 Visual Class로 감쌀 수 없는 구조를 새로 추가하지 않는다.
- 숨긴 정보가 공간을 남기는 `Hidden` 배치는 신규 기본값으로 사용하지 않고 `Collapsed`와 자동 Layout을 우선한다.
- WeaponPanel의 기존 `432×224`·`136×96` 수치는 더 이상 구현 불변값이 아니며 `InGameUIWeaponPanelSpec.md v0.5.0`의 승인된 Compact 기본 Preset을 사용한다.
- `ConceptArt/CFWeaponPanel_1440p.xml`은 초기 넓은 배치 참고 자료이며 Compact Revision의 최종 Pixel SSOT가 아니다.
- v0.2.0부터 기본 HUD Slot 배치는 승인된 TPS 16:9 구성을 사용한다.
- 이전 Wireframe의 좌하단 Radar와 중앙 하단 독립 Speed Cluster 좌표는 폐기한다.
- `VehiclePanel` Placeholder의 상세 소유권은 승인된 `InGameUIVehiclePanelSpec.md v0.2.0`이 대체한다.
- `WeaponPanel` Placeholder의 현재 상세 소유권은 `InGameUIWeaponPanelSpec.md v0.5.0`이 대체한다. `CFWeaponPanel_1440p.xml`은 초기 넓은 배치 참고 자료이며 Compact Pixel SSOT가 아니다.
- 다중 무기 Provider가 없으면 WeaponPanel의 Compact Rail을 숨기고 Selected Card만 표시한다.
- Ammo·Charge·Heat·Reload Runtime이 없을 때 WeaponData 설정값으로 현재 상태를 추정하지 않는다.
- P0는 `DA_CFHUDLayout_Default`에 해당하는 기본 배치만 사용하고 사용자 커스텀 배치 UI·저장은 후속 요청 전까지 구현하지 않는다.
- 이 문서는 Style 제작 초안이며 기존 AimReticle·TargetSelect·Pause UI를 즉시 교체하지 않는다.
- 구현 시 Style Token은 C++ 타입과 `DA_CFUIStyle`에서 관리하고 Widget 개별 하드코딩을 최소화한다.
- Base Widget은 View Data를 계산하지 않고 표시 상태만 적용한다.
- 2560×1440 Wireframe은 Anchor와 정보 우선순위 기준이며 실제 카메라 캡처 검토 후 위치를 미세 조정한다.
- VehiclePanel Visual Layout, WeaponPanel Compact Direction, Global UI Customization, D1-05 Style·Density·Base Widget·Weapon Compact Token 기본값, D1-06 Font·Icon 기본 Preset과 D1-07 Phase 1 `SR-1080-16` Static Review는 사용자 승인 완료다. 다만 D1-08~12 자산화와 실제 Font·Icon 라이선스·Unreal Asset 확인 전에는 UI-DESIGN-GATE 전체 PASS로 기록하지 않는다. 후속 해상도 확장은 현재 Gate를 차단하지 않는다.
