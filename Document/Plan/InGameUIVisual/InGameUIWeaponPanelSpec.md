# CarFight InGame UI Weapon Panel Specification

- 문서 버전: v0.7.4
- 작성일: 2026-08-07
- 최근 갱신일: 2026-08-23
- 문서 상태: Supporting Weapon Visual/Semantic Contract / current-runtime Production WeaponPanel Technical Complete
- 기능 ID: `CF-FQ-032`
- Current Applicability: CF-FQ-039는 현재 runtime truth·Resource/Rail 의미·fake source 금지 계약을 재사용한다. Visual 작업 상태와 Target 승인 범위는 `InGameUIVisualPlan.md`가 소유한다.
- 대표 Plan: `InGameUIVisualPlan.md v0.1.2`
- 시각 콘셉트: `InGameUIVisualConcept.md v0.8.1`
- 전체 Style 규격: `InGameUIStyleSpec.md v0.86.1`
- 패널 Wireframe: `ConceptArt/CFWeaponPanel_1440p.xml`
- 현재 발사 구현: `../Systems/Combat/WeaponFire.md`
- 현재 발사 피드백: `../Systems/Combat/FireFeedback.md`
- 후속 탄약 설계: `AmmoSystemDesign.md`

---

## 1. 목적

이 문서는 승인된 16:9 외부 3인칭 차량 TPS HUD에서 우하단 `WeaponPanel`이 표시할 정보의 우선순위, `464×360` 최대 Slot 경계 안에서 실제 Content를 축소하는 Compact 구조, 선택·비선택 무기 표현, 공통 자원 채널과 발사 가능 상태를 정의한다.

이번 범위:

```text
- 현재 선택 무기의 강한 강조
- 비선택 무기의 축약 표시
- Ammo / Charge / Heat / Cooldown / Reload 공통 표시 채널
- 발사 가능·불가와 현재 행동 상태
- 탄약형·에너지형·런처형·복합형 무기의 채널 선택 규칙
- 16:9 외부 3인칭 TPS HUD 우하단 배치 유지
- 1080p·1440p와 울트라와이드 중앙 16:9 Canvas 대응 원칙
```

이번 범위가 아닌 것:

```text
- CommonUI 도입 판단
- VehicleBattery shared-power Gameplay Runtime 구현
- 실제 per-weapon Icon authoring/source 계약
- 비선택 무기별 Resource Summary Runtime/ViewData 계약
- Gamepad Weapon Select Input Mapping
- 사용자 HUD 배치 편집 기능의 실제 화면·저장 구현
```

이 문서는 사용자 승인된 `D1-WEAPON-PANEL` Compact Visual Direction과 현재 Production 구현 경계를 함께 기록한다. 현재 Selected Card, Dynamic Resource Visual, truthful Text Rail, Keyboard Weapon Select Input과 WeaponCharge/Heat Runtime HUD Resource Projection까지 current-runtime 기준 Technical Complete다. 숫자키 1~9는 실제 selectable weapon 1-based ordinal과 직접 대응하며 Mouse Wheel은 Radar Range/Zoom 예약을 유지한다. WeaponCharge는 실제 per-weapon Runtime만 표시하고 VehicleBattery와 분리한다. Rail USER Visual은 representative persisted multi-weapon content가 생길 때까지 content-dependent Deferred이며, VehicleBattery는 WeaponPanel 미완성 항목이 아니라 external shared-power Gameplay dependency다. richer per-weapon icon/resource source와 gamepad mapping은 별도 확장이다.

---

## 2. 현재 Runtime과 설계 경계

### 2.1 현재 실제로 존재하는 데이터

현재 `Applied Fitting → UCFVehicleWeaponComp → UCFHUDDataProvider`와 `WeaponFire`, `FireFeedback`, `UCFLauncherComp`에서 확인 가능한 핵심 데이터는 다음과 같다.

```text
- Applied Fitting의 실제 weapon-bearing ResolvedMounts 결정론적 순서
- Player-facing SelectableWeapons 목록과 SelectedWeaponIndex
- 각 선택 항목의 EquipmentPresetData.DisplayName 가용 상태와 선택 여부
- 현재 활성 WeaponData와 호환 여부
- 내부 FireOrigin·Ammo identity용 MountProfileId — UI 표시 금지
- 발사 성공·거부·발사 가능 검증 결과
- 선택 무기별 독립 Cooldown / 마지막 승인 발사 시각
- 선택 무기별 실제 WeaponCharge Runtime / Current·Maximum·Ratio / Game-Time 회복 / Insufficient
- 선택 무기별 실제 Heat Runtime / 자연 냉각 / Overheated
- Launcher SingleCycle / Ripple / Salvo와 Sequence 진행 상태
- finite Ammo Runtime의 MagazineCapacity / LoadedAmmoCount / ReserveAmmoCount
- finite Ammo Runtime의 ImmediateUsableAmmoCount / CurrentUsableAmmoCount / CurrentOnboardAmmoCount
- 실제 ReloadState / ReloadDurationSeconds / RemainingReloadTimeSeconds / Action Lock
```

현재 발사 피드백에서 실제로 구분 가능한 대표 상태:

```text
FireSuccess
FireRejected
Cooldown
NoWeapon
AimBlocked
OutOfArcWarning
TurretAligning
MuzzleBlocked
```

### 2.2 현재 존재하지 않는 Runtime / Presentation Source

```text
- Vehicle Battery 직접 소비 기반 공용 전력 Runtime
- 실제 per-weapon Player-facing Icon source
- 비선택 무기 각각의 Ammo/Heat/Cooldown/Reload Resource Summary ViewData
- Gamepad Weapon Select Input Mapping
```

finite Ammo·Reload, WeaponCharge와 Heat는 실제 Runtime Snapshot/WeaponComp 상태만 사용한다. WeaponCharge는 explicit Maximum/Initial/PerShot/Recovery가 유효한 무기에서만 활성이고 all-zero 기존 Asset은 Disabled다. VehicleBattery는 실제 Runtime 전까지 표시하지 않는다. 현재 Rail은 비선택 무기별 Resource Summary source가 없으므로 선택 무기의 자원 상태를 복제하거나 정적 WeaponData에서 추정하지 않는다. 실제 per-weapon icon source도 없으므로 기존 Turret/Ammo/Reload semantic icon이나 Asset 이름을 weapon identity로 사용하지 않는다.

### 2.3 현재 Production 연결 범위

현재 구현 연결 범위에서는 다음을 표시할 수 있다.

```text
- 선택 무기 Header/Selected Card 1개
- 선택 무기의 실제 EquipmentPresetData.DisplayName
- 실제 Ammo / Reserve / WeaponCharge / Reload / Cooldown / LauncherSequence / Heat Compact Resource
- 실제 Weapon Selection 목록의 비선택 무기 Text Rail
- Rail 비선택 0개 → 숨김
- Rail 비선택 1~3개 → 원본 1-based 순번 + 실제 DisplayName
- Rail 비선택 4개 이상 → 앞의 2개 + `+N`
```

현재 선택 무기는 Rail에 중복 표시하지 않는다. Rail 항목의 DisplayName이 없으면 내부 ID fallback 없이 일반 `WEAPON`만 사용한다. 실제 icon source와 비선택 resource summary source가 없으므로 현재 P0 Production Rail에는 icon, 상태 Marker, Resource Summary를 만들지 않는다.
Ammo Runtime이 InfiniteCompatibility 또는 미초기화라면 Ammo·Reserve 채널을 가짜 `0`이나 설정값으로 만들지 않고 숨긴다. WeaponCharge도 실제 활성 Runtime이 있을 때만 Current/Maximum/Ratio를 표시하며 정적 설정으로 현재값을 만들지 않는다. VehicleBattery는 실제 Provider 전 `0`, `100%`, `N/A`로 채우지 않는다.
내부 Asset 이름, Object Path, Debug Summary, `WeaponId`, `EquipmentId`, `MountProfileId`를 플레이어 HUD identity로 노출하지 않는다.

---

## 3. 정보 우선순위

| 우선순위 | 정보 | 표시 원칙 |
|---:|---|---|
| 1 | 현재 선택 무기와 발사 가능 여부 | 항상 가장 강하게 표시 |
| 2 | 현재 행동을 막는 상태 | Reload·Overheat·NoAmmo·NoCharge·AimBlocked·Disabled |
| 3 | 선택 무기의 주 자원 | Ammo 또는 Charge·Battery 중 1개만 대형 표시 |
| 4 | 진행 중 행동 | Launcher Sequence·Cooldown·Reload·Charging |
| 5 | 보조 자원 | Heat와 두 번째 자원, 최대 2개 |
| 6 | 비선택 무기 | 식별과 다음 행동 판단에 필요한 최소 정보만 축약 |

핵심 시선 흐름:

```text
선택 무기 이름
→ 발사 가능 상태
→ 주 자원
→ 진행 중 상태·보조 자원
→ 비선택 무기 요약
```

한 무기의 모든 데이터 필드를 동일한 시각 무게로 표시하지 않는다.

---

## 4. 슬롯과 화면 점유 계약

기준:

```text
WeaponPanel Maximum Slot: 464 × 360
전체 HUD 위치: X 2032 / Y 1016
Anchor: Bottom Right
기준 Canvas: 2560 × 1440 중앙 16:9
```

`464×360`은 Radar·VehiclePanel·Reticle과 충돌하지 않게 하는 최대 배치 경계다. 실제 WeaponPanel이 이 면적을 항상 채우는 고정 크기가 아니다.

### 4.1 투명 Root와 Desired Size 원칙

```text
투명 Root Slot
└─ Auto Size Content Container
   ├─ SelectedWeaponCard
   └─ CompactWeaponRail — 조건부
```

- 실제 Surface는 정보가 존재하는 카드 영역에만 적용한다.
- Content Container는 현재 표시 채널과 Rail 존재 여부에 맞춰 Desired Size로 수축한다.
- 비선택 무기가 없으면 Compact Rail은 `Collapsed` 처리하고 Rail 높이·간격·하단 여백까지 제거한다.
- Secondary Channel이 하나면 남은 채널이 사용 가능한 폭을 채우고, 모두 없으면 Secondary Row 자체를 제거한다.
- Slot Anchor는 유지하되 Content는 우하단 정렬을 사용해 축소 시 화면 가장자리 기준이 흔들리지 않게 한다.
- 패널이 중앙 Reticle Protection 영역으로 확장되지 않는다.

### 4.2 Compact Profile 수치 소유권

이전 Visual Layout의 다음 값은 초기 넓은 Wireframe 참고값이다.

```text
SelectedWeaponCard 432 × 224
CompactTile 136 × 96
```

v0.3.0부터 이 값은 구현 하드코딩이나 불변 계약으로 사용하지 않는다. 실제 기본값은 `DA_CFUIDensity_Compact` 또는 동등한 Density Data Asset이 소유한다.

Compact Revision의 목표는 정보 의미를 제거하는 것이 아니라 빈 여백·보조 Caption·고정 행 높이와 장식을 줄이는 것이다.

### 4.3 WeaponPanel Compact Token 승인 기본값

다음 값은 2560×1440 기준 User Accepted Default Preset이다. 구현 시 `DA_CFUIDensity_Compact` 또는 동등한 WeaponPanel Density 설정이 소유하며 WBP 내부 Literal로 고정하지 않는다.

| Token | Draft 값 | 용도 |
|---|---:|---|
| `WeaponPanelPreferredWidth` | 360 | 기본 실제 Content 폭 |
| `WeaponPanelMinimumWidth` | 320 | 긴 정보가 없을 때 허용 최소 폭 |
| `WeaponPanelMaximumContentWidth` | 400 | Maximum Slot 안에서 허용하는 Content 상한 |
| `SelectedCardPadding` | 12 | Selected Card 내부 여백 |
| `SelectedHeaderHeight` | 28 | Group·Icon·Name 한 줄 |
| `PrimaryChannelHeight` | 52 | 가장 중요한 자원 1개 |
| `SecondaryChannelHeight` | 30 | Secondary A·B 공통 행 높이 |
| `FireStateHeight` | 26 | 대표 Fire State 한 줄 |
| `SectionGap` | 6 | Selected Card 내부 섹션 간격 |
| `RailTopGap` | 8 | Selected Card와 Rail 사이 |
| `CompactTileWidth` | 112 | 비선택 무기 Tile 폭 |
| `CompactTileHeight` | 68 | 비선택 무기 Tile 높이 |
| `CompactTileGap` | 8 | Tile 사이 간격 |
| `SelectedWeaponIconSize` | 20 | 선택 무기 Header 아이콘 |
| `CompactWeaponIconSize` | 18 | Rail Tile 아이콘 |
| `PrimaryBarHeight` | 6 | Primary 보조 Progress Bar |
| `SecondaryBarHeight` | 4 | Secondary Progress Bar |
| `SelectedAccentThickness` | 3 | Selected 부분 외곽선 |
| `SelectedCornerMarkerLength` | 18 | 선택 Corner Marker |

기본 Width 계산:

```text
Selected Card = 360
Compact Rail = 112 × 3 + 8 × 2 = 352
```

따라서 Rail 3개 상태도 Selected Card 폭보다 넓어지지 않는다.

### 4.4 Selected Card 높이 계산

#### Secondary 없음

```text
Top Padding       12
Header            28
Gap                6
Primary           52
Gap                6
Fire State        26
Bottom Padding    12
--------------------
Total            142
```

#### Secondary 1~2개

```text
Top Padding       12
Header            28
Gap                6
Primary           52
Gap                6
Secondary         30
Gap                6
Fire State        26
Bottom Padding    12
--------------------
Total            178
```

#### Rail 3개까지 포함한 최대 기본 Content 높이

```text
Selected Card    178
Rail Top Gap       8
Compact Rail      68
--------------------
Total            254
```

`464×360` Maximum Slot 대비 기본 Full 상태도 `360×254`를 목표로 하므로 기존 넓은 Wireframe보다 화면 점유를 줄인다.

### 4.5 Secondary Row 폭 규칙

Selected Card 360, 좌우 Padding 12 기준 내부 Content 폭은 336이다.

```text
Secondary 0개
→ Row Collapsed

Secondary 1개
→ 336 전체 폭 사용

Secondary 2개
→ 164 + 8 Gap + 164
```

Widget 개수에 따라 고정 Spacer가 남지 않게 자동 Layout으로 구성한다.

### 4.6 Compact Typography 초안

| Role | 1440p Draft | 1080p 유효 하한 | 규칙 |
|---|---:|---:|---|
| Weapon Name | 18 | 15 | 한 줄, 말줄임표 허용 |
| Primary Value | `DisplayL × Compact PrimaryValueScale` ≈ 38 | 30 | 전역 Typography Role 기반 / 카드당 대형 숫자 1개 |
| Primary Label | 14 | 12 | 단위·자원명 |
| Secondary Value | 16 | 13 | 1~2개 보조 채널 |
| Fire State | 14 | 12 | 행동 가능·차단 상태 |
| Tile Group Number | 14 | 12 | 고정 폭 예약 |
| Tile Weapon Name | 14 | 12 | 한 줄 축약 |
| Tile Summary | 14 | 12 | 한 개의 대표 상태·자원 |

Geometry가 1080p에서 0.75로 축소되더라도 위 Typography 하한은 `InGameUIStyleSpec.md`의 규칙을 따라 별도 보정한다.

### 4.7 Compact Content 정책

Compact에서 제거 가능한 것:

```text
- 설명성 Caption
- 중복 단위 문구
- 항상 정상인 상태 Chip
- 비활성 Rail의 예약 높이
- 사용하지 않는 Secondary Row
- 장식 Scanline·불필요한 Frame Segment
```

Compact에서도 제거하지 않는 것:

```text
- 선택 무기 식별
- 발사 가능·불가 대표 상태
- Primary Resource
- 실제 행동을 차단하는 Reload·Heat·Cooldown·Sequence 상태
- Critical / Disabled 상태
- 다른 무기 존재 여부 — Provider가 있을 때
```

### 4.8 Compact Token 커스터마이징 불변 조건

- `WeaponPanelPreferredWidth`, Card Height, Tile Size, Icon Size, Bar Height, Gap과 Typography 값은 기본 Preset이며 하드코딩 불변값이 아니다.
- 전역 Style·Density Data 변경으로 WeaponPanel 기본 외형을 조절할 수 있어야 한다.
- WeaponPanel 전용 값은 전용 Density Token 그룹으로 Override할 수 있지만 Gameplay·Presenter 계약을 바꾸지 않는다.
- 사용자 복제 Blueprint는 같은 View Data 입력을 유지한 채 Header·Resource·Rail Visual을 교체할 수 있어야 한다.
- 승인값 변경을 위해 C++ 재컴파일이 필요하면 설계 위반이다.

---

## 5. 현재 선택 무기 카드

### 5.1 가장 강한 강조

선택 무기 카드는 다음 표현을 함께 사용한다.

```text
- SurfaceRaised 배경
- AccentTactical 3px 부분 외곽선
- 좌상단 또는 우상단 Corner Marker
- 가장 밝은 무기 이름
- 가장 큰 주 자원 숫자
- 고정 FireStateStrip
```

선택 강조를 카드 전체 Cyan Fill이나 지속 Glow로 만들지 않는다. 무기 종류의 고유색보다 `현재 선택됨`이라는 구조적 강조를 우선한다.

### 5.2 SelectedHeader

표시 요소:

```text
Weapon Group Number 또는 짧은 Group Label
Weapon Icon
Weapon Display Name
선택 Marker
```

Compact 배치 초안:

```text
Group Number Reserved Width = 24
Icon = 20
Group/Icon Gap = 6
Icon/Name Gap = 6
Name = Remaining Width / 한 줄
```

- 이름이 남은 폭을 초과하면 말줄임표를 사용한다.
- Header 높이를 늘려 2줄로 만들지 않는다.
- 그룹 번호가 없는 무기는 Reserved Width도 Collapse할 수 있다.

규칙:

- 무기 이름은 한 줄만 사용한다.
- 이름이 길면 말줄임표를 사용하고 카드 폭을 늘리지 않는다.
- 내부 ID 대신 플레이어용 Display Name을 사용한다.
- 실제 Display Name이 없으면 `주 무기`, `보조 무기`, `런처` 같은 안전한 역할명만 허용한다.
- Fire State는 Header에 중복 표시하지 않고 하단 Strip이 소유한다.

### 5.3 PrimaryChannel

선택 무기에서 가장 중요한 자원 하나만 대형으로 표시한다.

Compact 기본 구조:

```text
Horizontal / Overlay
├─ PrimaryValue + Unit
├─ PrimaryLabel
└─ OptionalProgressBar 6
```

- Primary 영역 높이는 52를 기본으로 한다.
- 숫자는 우측 정렬 또는 중앙 우측 정렬을 권장하고 Label은 좌측 상단에 둔다.
- 값과 Label이 중복 의미면 Label을 짧게 줄인다.
- Bar가 필요 없는 Count형 Ammo는 Bar 영역을 Collapse한다.

```text
Ballistic / Ammo Weapon → Ammo
Internal Charge Weapon  → Weapon Charge
Battery Direct Weapon   → Vehicle Battery
Launcher Sequence Active → Sequence Progress
Cooldown-only Equipment → Cooldown 또는 Ready State
```

표현:

```text
PrimaryValue = DisplayL × 현재 Density.PrimaryValueScale
Compact 기본 = 44 × 0.86 ≈ 38
PrimaryLabel = Compact Token 기본 14
PrimaryBarHeight = Compact Token 기본 6
```

전역 Typography나 Density를 바꾸면 Primary 숫자도 자동으로 따라가야 한다. 대형 숫자는 한 카드에서 하나만 사용한다.

### 5.4 SecondaryChannelA·B

- 최대 2개만 표시한다.
- 채널 하나당 Label, 짧은 값과 4 높이의 얇은 Bar 또는 진행선을 사용한다.
- 지원되지 않는 채널은 `Collapsed`한다.
- 한 채널만 남으면 336 내부 폭 전체를 사용한다.
- 두 채널이면 164 / 8 Gap / 164 구조를 사용한다.
- 두 채널이 모두 없으면 Secondary Row 자체와 앞뒤 Gap을 제거한다.
- 세 번째 보조 채널은 상시 HUD에서 제외하고 후속 상세 Panel이 소유한다.

### 5.5 FireStateStrip

FireStateStrip은 선택 무기가 지금 어떤 행동을 할 수 있는지를 한 줄로 설명한다.

예:

```text
발사 가능
재사용 대기 0.8초
재장전 1.4초
탄약 없음
충전 부족
과열
조준 가림
정렬 중
RIPPLE 2 / 4
무기 없음
사용 불가
```

- 상태 아이콘 + 짧은 한국어 문구를 사용한다.
- 내부 RejectReason enum 이름을 그대로 표시하지 않는다.
- 동일한 숫자를 Primary·Secondary Channel과 Strip에 반복하지 않는다.
- Cooldown Bar에 남은 시간이 있으면 Strip은 `재사용 대기`만 표시할 수 있다.
- Critical 상태가 아니면 반복 Pulse를 사용하지 않는다.

---

## 6. 비선택 무기 Compact Rail

### 6.1 최대 표시 개수

```text
비선택 무기 0개 → Rail 숨김
비선택 무기 1~3개 → Tile 1~3개
비선택 무기 4개 이상 → 앞의 2개 + `+N` Overflow Tile
```

정렬은 안정적인 `WeaponGroupIndex` 또는 Provider가 제공한 고정 표시 순서를 사용한다. 현재 선택 무기는 Compact Rail에 중복 표시하지 않는다.

### 6.2 Compact Tile 정보

#### 현재 P0 Production 계약 — Technical PASS

현재 실제 source로 확정된 Tile 정보는 다음뿐이다.

```text
- Provider fixed order의 원본 1-based 순번
- EquipmentPresetData.DisplayName
- 4개 이상 비선택 무기의 `+N` Overflow
```

현재 Production 구조는 `112×68` Text Tile 최대 3개와 `8px` gap을 사용한다. 실제 per-weapon icon source가 없으므로 `CompactWeaponIconSize`를 이용한 weapon icon을 만들지 않는다. 비선택 무기별 Resource Summary ViewData도 없으므로 선택 무기의 Ammo/Heat/Cooldown 상태를 복제하거나 WeaponData 설정값에서 추정하지 않는다. DisplayName이 없으면 `MountProfileId`, `WeaponId`, `EquipmentId`, AssetName 대신 일반 `WEAPON`만 표시한다.

#### Future richer Tile 확장 — Source 준비 후

실제 per-weapon icon과 비선택 resource summary source가 추가되면 기존 승인 방향에 따라 다음을 확장할 수 있다.

```text
- 18px Weapon Icon — `CompactWeaponIconSize` 기본값
- 한 개의 Primary Summary
- 작은 상태 Marker
```

Primary Summary 우선순위 후보는 다음을 유지한다.

```text
1. 행동 차단 상태: 탄약 없음 / 과열 / 재장전 / 사용 불가
2. 진행 상태: RIPPLE 2/4 / 쿨다운 1.2초
3. 주 자원: 20발 / CHG 74% / BAT 62%
4. 별도 상태 없음: 준비
```

단, 실제 per-item Runtime/ViewData source가 생긴 뒤에만 사용한다. Compact Tile에는 두 개 이상의 Bar, 상세 Reload 시간, Reserve 분해와 긴 실패 이유를 넣지 않는다.

### 6.3 비선택 표현 강도

```text
배경: SurfaceSoft 또는 낮은 SurfaceRaised Alpha
외곽선: LineDefault 1~2px
텍스트: TextSecondary
상태 Marker: 실제 Warning·Critical일 때만 상태색
```

비선택 무기의 정상 상태를 Cyan으로 채우지 않는다. 선택 변경 시 새 선택 카드로 이동하는 `160ms` 상태 전환만 허용하고 지속 Scanline·회전·Glow는 사용하지 않는다.

---

## 7. 공통 자원 채널 계약

### 7.1 채널 종류

표현용 공통 채널은 다음을 지원한다.

```text
Ammo
ReserveAmmo
VehicleBattery
WeaponCharge
Heat
Cooldown
Reload
LauncherSequence
None
```

사용자가 요구한 Ammo·Charge·Heat·Cooldown·Reload는 공통 핵심 채널이다. `ReserveAmmo`, `VehicleBattery`, `LauncherSequence`는 기존 전투·피팅 설계와 런처 상태를 정확히 표현하기 위한 확장 채널이다.

### 7.2 공통 필드 후보

향후 UI-P0-03에서 사용할 표현용 계약 후보:

```text
ChannelType
DisplayMode
DisplayState
Priority
CurrentValue
MaximumValue
NormalizedValue
PrimaryText
SecondaryText
RemainingTimeSeconds
bBlocksFire
bIsActive
bIsVisible
```

`DisplayMode` 후보:

```text
Count
CountPair
RatioBar
Percent
TimeRemaining
Progress
Sequence
TextOnly
```

`DisplayState`:

```text
Known
KnownZero
Unavailable
Hidden
Critical
```

Widget이 `Current / Maximum`, 남은 시간, 발사 가능 여부를 Gameplay 원본에서 다시 계산하지 않는다. Presenter가 표시용 값을 완성해 전달한다.

### 7.3 공통 포맷 규칙

| 데이터 | 표시 |
|---|---|
| 탄수 | 소수점 없는 정수 |
| 비율 | 기본 정수 `%` |
| 10초 미만 시간 | 소수점 1자리 |
| 10초 이상 시간 | 정수 초 |
| 명시적 무한 자원 | Provider가 보장할 때만 `∞` |
| Known Zero | `0` |
| Provider 없음 | 채널 숨김 |
| 필수 채널이 일시적으로 깨짐 | `정보 없음` 또는 `N/A` |

`Unavailable`은 해당 무기가 원래 그 채널을 요구하지만 Provider가 비정상인 경우에만 사용한다. 무기가 사용하지 않는 채널을 `N/A` 행으로 만들지 않는다.

---

## 8. 채널별 표시 규칙

### 8.1 Ammo

finite 탄약형 선택 무기의 Primary Ammo는 다음 형식을 기본으로 한다.

```text
5 / 10
```

의미:

```text
왼쪽 = LoadedAmmoCount
오른쪽 = MagazineCapacity
```

같은 탄종의 차량 예비량은 **WeaponPanel 우상단에 라벨 없이 작은 숫자 하나**로 분리한다.

```text
우상단: 10
의미 = ReserveAmmoCount
Label = 없음
```

표시 계약:

- Primary Ammo와 Reserve는 서로 다른 시각 채널이다. `5 / 10` 옆에 Reserve를 다시 붙여 `5 / 10 / 10`처럼 만들지 않는다.
- `MagazineCapacity`는 현재 finite Ammo Runtime Snapshot의 탄창 용량이며 피팅 `MaximumLoadableAmmoCount`가 아니다.
- `ReserveAmmoCount`는 현재 무기와 같은 AmmoId의 실제 차량 공유 예비량만 표시한다.
- `ReserveAmmoCount`가 KnownZero인 경우 채널을 숨기거나 Collapse하지 않고 우상단 숫자 `0`을 그대로 표시한다.
- Launcher Sequence 예약은 장전탄의 소유량을 즉시 소비하지 않는다. 따라서 예약만 걸린 시점에는 `ImmediateUsableAmmoCount`가 감소해도 Primary `Loaded / Capacity`는 Commit 전까지 유지된다.
- `ImmediateUsableAmmoCount`와 `CurrentUsableAmmoCount`는 발사 가능·예약·NoAmmo/Debug 판정용 내부 Runtime/ViewData 값으로 유지하지만 Primary Ammo 숫자에는 사용하지 않는다.
- Reload Commit으로 Reserve가 Loaded로 이동하면 Primary Loaded는 증가하고 우상단 Reserve는 같은 양만큼 감소한다.
- 탄창·예비 개념이 없는 단순 탄약 Provider는 별도 정책으로 한 개 숫자만 표시할 수 있다.
- 서로 교환할 수 없는 탄종을 합산하지 않는다.
- finite Ammo Runtime이 없으면 WeaponData `MagazineSize` 설정값을 현재 탄수처럼 직접 표시하지 않는다.
- `0`은 실제 Known Zero일 때만 표시한다.

### 8.2 Weapon Charge

- 내부 축전기 또는 무기 자체 충전을 표시한다.
- 기본 표현은 `현재/최대` 또는 `%`다.
- 발사 최소 요구량이 있으면 Presenter가 `충전 부족` 상태를 제공한다.
- 차량 배터리와 같은 의미로 합치지 않는다.
- 충전 중 Motion은 Bar의 단방향 진행 정도만 허용하고 지속 강한 Glow를 사용하지 않는다.

### 8.3 Vehicle Battery

- 무기가 차량 전력을 직접 소비한다고 명시된 경우만 표시한다.
- 에너지 무기라는 이름만으로 자동 선택하지 않는다.
- Label은 `배터리`로 표시해 `무기 충전`과 구분한다.
- 차량 전체 에너지 시스템이 별도 HUD를 가지게 되면 WeaponPanel에는 해당 무기 사용 가능성과 관련된 최소 요약만 남긴다.

### 8.4 Heat

- `0`은 냉각 완료, `Maximum`은 과열 방향이다.
- 다른 잔여 자원 Bar와 반대 의미이므로 `열` Label과 온도·열 아이콘을 항상 함께 사용한다.
- Stable → Caution → Danger → Overheated를 색상과 형태로 구분한다.
- 과열 시 FireStateStrip이 `과열`을 소유하고 Heat 채널은 실제 비율을 유지한다.
- 실제 Heat Runtime이 없으면 `HeatPerShot`와 `MaxHeat` 데이터만으로 가짜 현재 Heat를 만들지 않는다.

### 8.5 Cooldown

- 실제 `RemainingCooldownSeconds`와 `TotalCooldownSeconds`를 사용한다.
- 남은 시간이 0이면 일반적으로 채널을 숨기고 FireStateStrip은 `발사 가능`으로 돌아간다.
- Cooldown-only 장비는 Ready 상태에서도 해당 자리의 준비 상태를 유지할 수 있다.
- Launcher의 내부 Projectile마다 별도 전체 Cooldown을 표시하지 않고 Weapon·Volley 단위 Cooldown 하나만 표시한다.

### 8.6 Reload

- 실제 Reload Runtime이 활성일 때만 표시한다.
- `남은 시간 / 전체 시간` 진행과 `재장전` 상태를 표시한다.
- Reload와 Cooldown이 병렬이면 FireStateStrip은 실제 발사를 가장 오래 막는 상태를 Presenter가 선택해 제공한다.
- Widget이 두 시간을 비교해 Gameplay 준비 시각을 계산하지 않는다.
- 현재 Runtime이 없으므로 첫 구현에서 숨긴다.

### 8.7 Launcher Sequence

- Launcher Sequence가 Active면 Ammo보다 Sequence 진행을 PrimaryChannel로 승격할 수 있다.

예:

```text
RIPPLE 2 / 4
남은 발사 2
```

- `SingleCycle`, `Ripple`, `Salvo`를 짧은 Pattern Label로 구분한다.
- 진행 중 예약 탄약과 자유 사용 가능 탄약은 후속 Ammo Runtime이 제공할 때만 표시한다.
- Sequence 중 새 행동이 잠기면 FireStateStrip은 `RIPPLE 2 / 4` 또는 `사격 시퀀스 진행 중`을 표시한다.
- 마지막 Projectile마다 전체 Weapon Cooldown이 다시 시작되는 것처럼 표현하지 않는다.

---

## 9. 발사 가능 상태 계약

### 9.1 단일 대표 상태

FireStateStrip에는 동시에 하나의 대표 상태만 표시한다. 여러 원인이 겹치면 다음 우선순위를 사용한다.

| 우선순위 | 대표 상태 | 예시 문구 |
|---:|---|---|
| 1 | NoWeapon·Disabled·Unavailable | `무기 없음`, `사용 불가`, `정보 없음` |
| 2 | NoAmmo·NoCharge·Overheated | `탄약 없음`, `충전 부족`, `과열` |
| 3 | Reloading·SequenceActive | `재장전 1.4초`, `RIPPLE 2 / 4` |
| 4 | AimBlocked·MuzzleBlocked | `조준 가림`, `총구 가림` |
| 5 | Cooldown | `재사용 대기 0.8초` |
| 6 | Alignment Warning | `정렬 중` |
| 7 | Ready | `발사 가능` |

### 9.2 정렬 중 발사 정책

현재 `bAllowFireWhileAligning` 정책에 따라 `TurretAligning`이 곧 발사 불가를 의미하지 않을 수 있다.

```text
정렬 중 발사 허용
→ FireStateStrip: 발사 가능
→ Amber 보조 Chip: 정렬 중

정렬 중 발사 거부
→ FireStateStrip: 정렬 대기
```

정렬 경고를 무조건 Red `발사 불가`로 덮어쓰지 않는다.

### 9.3 성공 피드백과 지속 상태

- `FireSuccess`는 160~200ms의 짧은 1회 Accent로만 표시한다.
- 성공 피드백이 끝나면 Cooldown·Reload·Sequence 같은 지속 상태로 즉시 복귀한다.
- 발사 성공 문구를 장시간 유지해 현재 준비 상태를 가리지 않는다.
- 상세 RejectReason은 Debug가 소유하고 일반 HUD는 플레이어 행동에 필요한 짧은 이유만 표시한다.

---

## 10. 무기 유형별 표시 규칙

### 10.1 탄약형 무기

예:

```text
기관포
대포
산탄형 무기
일반 Projectile 무기
```

표시:

```text
Primary: Ammo
Secondary A: Heat — 실제 Runtime이 있을 때
Secondary B: Cooldown 또는 Reload — 활성 상태 우선
FireState: Ready / Cooldown / Reload / NoAmmo / AimBlocked
```

Reload가 활성일 때 Cooldown 수치를 중복 대형 표시하지 않는다.

### 10.2 내부 충전형 에너지 무기

표시:

```text
Primary: WeaponCharge
Secondary A: Heat
Secondary B: Cooldown 또는 Charging Progress
FireState: Ready / Charging / NoCharge / Overheated / Cooldown
```

Ammo 행을 만들지 않는다. `WeaponCharge`와 `Heat`를 같은 색상·같은 Bar 의미로 표현하지 않는다.

### 10.3 차량 배터리 직접 소비형 무기

표시:

```text
Primary: VehicleBattery 또는 무기 사용 가능 전력
Secondary A: Heat
Secondary B: Cooldown
FireState: Ready / BatteryLow / Overheated / Cooldown
```

무기 데이터가 차량 배터리 직접 소비를 명시해야 한다. 단순히 레이저·레일건이라는 이름만으로 배터리 채널을 선택하지 않는다.

### 10.4 런처형 무기

Idle:

```text
Primary: Ammo — 실제 Ammo Runtime이 있을 때
Secondary: Cooldown
FireState: 발사 가능 또는 Lock·Aim 관련 상태
```

Sequence Active:

```text
Primary: RIPPLE / SALVO 진행
Secondary A: 남은 Sequence 발사 수
Secondary B: 실제 Ammo 요약 — Provider가 있을 때
FireState: 시퀀스 진행 중
```

Lock 진행은 Reticle·TargetPanel이 주 표시를 소유한다. WeaponPanel에는 `LOCK`, `잠금 대기`, `잠금 완료` 같은 작은 문맥 Chip만 허용하고 동일한 원형 Lock Progress를 중복 표시하지 않는다.

### 10.5 복합형·레일건

가능한 조합:

```text
Ammo + WeaponCharge + Heat
Ammo + VehicleBattery + Heat
WeaponCharge + Cooldown + Heat
```

표시 규칙:

- 데이터가 지정한 `PrimaryResourceType` 하나만 대형 표시한다.
- 나머지는 최대 2개 Secondary Channel로 제한한다.
- 네 번째 채널이 필요하면 고정 HUD에서 생략하고 상세 Panel로 이관한다.
- 무기 이름으로 자원 구성을 추측하지 않는다.

### 10.6 Cooldown 중심 장비·방어장비

- 탄약이나 충전이 없는 장비는 Ready·Cooldown을 Primary로 사용할 수 있다.
- 사용 가능 횟수가 실제로 존재할 때만 Count를 표시한다.
- 효과 지속 시간과 Cooldown을 같은 Progress Bar로 합치지 않는다.

---

## 11. 상태 색상과 형태

| 상태 | 색상 | 추가 형태 |
|---|---|---|
| Selected / Ready | `AccentTactical` | Corner Marker·3px 부분 외곽선 |
| Cooldown / Charging | `StateNotice` | 진행선·시간 |
| Reloading / Aligning | `StateCaution` | Reload·정렬 아이콘 |
| NoAmmo / NoCharge / Heat Danger | `StateDanger` | 빈 자원 아이콘·경고 Notch |
| Overheated / Disabled | `StateCritical` 또는 `StateDisabled` | 경고 아이콘·두꺼운 일부 외곽선 |
| Unselected Normal | `LineDefault` / `TextSecondary` | 얇은 외곽선 |
| Unavailable | `TextDisabled` | `정보 없음`·점선 Frame |

색상만으로 상태를 표현하지 않는다. Ammo·Charge·Heat·Cooldown·Reload는 각기 다른 아이콘과 Label을 사용한다.

---

## 12. Motion과 화면 점유 제한

### 12.1 허용 Motion

```text
선택 변경: 160ms
자원 값 보간: 180ms
발사 성공 Accent: 160~200ms 1회
경고 Accent: 360ms 1회
Critical Pulse: 700ms, 실제 Critical일 때만
```

### 12.2 금지 Motion

```text
- Idle 상태 지속 Scanline
- 의미 없는 회전 원형 장식
- 모든 무기 Tile의 동시 Pulse
- 카드 전체 지속 Glow
- 발사마다 전체 우하단 Panel Flash
```

### 12.3 점유 상한

- Maximum Slot은 `464×360`을 넘지 않는다.
- 실제 Content는 Compact Density의 Desired Size를 사용하고 필요하지 않은 공간을 예약하지 않는다.
- 비선택 무기는 최대 3 Tile 또는 `2 + Overflow`다.
- Selected Card의 대형 숫자는 1개다.
- Secondary Channel은 최대 2개다.
- Fire State 문구는 한 줄이다.
- Tooltip이나 긴 설명은 전투 HUD에 상시 표시하지 않는다.

### 12.4 전체 UI 커스터마이징 계약 적용

WeaponPanel도 다른 CarFight UI와 동일한 구조를 따른다.

```text
Gameplay Runtime
→ WeaponPanel Presenter / ViewModel
→ 교체 가능한 WeaponPanel Visual Widget
→ Style·Layout·Density Data Asset
→ 사용자별 Override
```

필수 분해 후보:

```text
WBP_CFWeaponPanel
├─ WBP_CFWeaponHeader
├─ WBP_CFPrimaryResource
├─ WBP_CFResourceChannel
├─ WBP_CFFireStateStrip
├─ WBP_CFWeaponRail
└─ WBP_CFWeaponTile
```

- 하나의 Canvas 기반 거대 Widget에 모든 요소를 고정 좌표로 배치하지 않는다.
- 각 하위 Visual Widget은 Blueprint Class 교체 또는 상속이 가능해야 한다.
- 색상·폰트·Padding·Tile Size·Bar Height는 Style 또는 Density Data에서 제공한다.
- 채널 Visibility는 Presenter가 제공하고 Widget은 `Collapsed`를 적용해 빈 공간을 제거한다.
- 사용자가 Visual Widget을 교체해도 `FCFWeaponPanelViewData` 계열 입력 계약을 변경하지 않는다.
- 기본 Widget은 수정 불가능한 최종 에셋이 아니라 사용자 커스텀용 기본 Preset이다.

---

## 13. 표현용 View Data 후보

향후 UI-P0-03에서 다음 표현용 구조를 권장한다.

```text
FCFWeaponPanelViewData
FCFWeaponEntryViewData
FCFWeaponResourceViewData
FCFWeaponFireStateViewData
```

### 13.1 FCFWeaponPanelViewData

```text
bWeaponPanelAvailable
SelectedWeapon
CompactWeapons
HiddenCompactWeaponCount
PanelDisplayState
```

### 13.2 FCFWeaponEntryViewData

```text
WeaponInstanceId
WeaponGroupIndex
WeaponDisplayName
WeaponIcon
WeaponCategory
bIsSelected
PrimaryResource
SecondaryResources
FireState
SequenceState
bCanBeSelected
```

### 13.3 FCFWeaponResourceViewData

```text
ChannelType
DisplayMode
DisplayState
Priority
CurrentValue
MaximumValue
NormalizedValue
PrimaryText
SecondaryText
RemainingTimeSeconds
bBlocksFire
bIsVisible
```

### 13.4 FCFWeaponFireStateViewData

```text
bCanFireNow
FireState
DisplayText
StateIcon
Severity
RemainingStateTimeSeconds
bShowAlignmentWarning
bActionLocked
```

이 이름은 설계용 후보이며 이번 작업에서 C++ 파일이나 Unreal Asset을 생성하지 않는다.

---

## 14. 데이터 갱신과 책임

이벤트 기반:

```text
- 선택 무기 변경
- 무기 목록 변경
- Fire Result 변경
- 발사 가능 상태 변경
- Ammo·Charge·Heat 변경
- Reload 시작·완료·취소
- Launcher Sequence 시작·진행·완료·취소
- Action Lock 변경
```

제한 주기 기반:

```text
- Cooldown 남은 시간: 10~20 Hz 또는 UI 보간
- Charge·Heat 연속 변화: 10~20 Hz 또는 이벤트 결과 보간
```

책임 경계:

```text
Gameplay Runtime
→ 발사 가능 판정, 자원 소비, Reload, Heat, Sequence와 Action Lock 소유

Presenter
→ 현재 무기 목록과 자원을 표현 우선순위로 변환
→ Primary 1개, Secondary 최대 2개, 대표 Fire State 결정

Widget
→ 전달된 텍스트·값·상태를 고정 레이아웃에 표시
```

Widget이 Pawn을 직접 Cast하거나 매 Tick Weapon Component를 조회하지 않는다.

---

## 15. 해상도 대응

### 15.1 2560×1440

```text
Maximum Slot        464 × 360
Preferred Content   360 × 142~254
Maximum Content     400 × 360 이하
```

기본 Compact Full 상태는 `360×254` 안에 들어오는 것을 목표로 한다.

### 15.2 1920×1080

Geometry 0.75 기준 환산:

```text
Maximum Slot        348 × 270
Preferred Content   270 × 약 107~191
```

Typography는 단순 0.75가 아니라 4.6의 유효 하한을 적용한다.

1080p 검토:

- Selected Weapon Name과 FireStateStrip이 한 줄로 읽힌다.
- Primary Value가 잘리지 않는다.
- Secondary 2개 상태에서도 각 값이 구분된다.
- Compact Tile의 Group Number·Icon·Primary Summary가 충돌하지 않는다.
- Caption을 줄여도 Fire State와 주 자원 수치는 유지한다.
- Rail 3개가 348 Maximum Slot 폭 안에서 유지된다.

### 15.3 3440×1440 — 21:9 대표

```text
Central HUD Canvas X = 440~3000
WeaponPanel Anchor = Central Canvas Bottom Right
```

- WeaponPanel의 오른쪽 기준은 물리 Viewport 3440이 아니라 중앙 Canvas 오른쪽 3000이다.
- RadarPanel과의 승인 간격을 16:9 기준으로 유지한다.
- 좌우 추가 영역을 이유로 Rail Tile 개수를 늘리지 않는다.

### 15.4 5120×1440 — 32:9 대표

```text
Central HUD Canvas X = 1280~3840
WeaponPanel Anchor = Central Canvas Bottom Right
```

- WeaponPanel을 물리 화면 X=5120 끝으로 이동하지 않는다.
- Persistent Panel의 시선 거리는 16:9와 동일하게 유지한다.
- World Target Marker는 전체 Viewport Projection을 사용하므로 WeaponPanel Anchor 정책과 분리한다.

---

## 16. 검증 기준

### 16.1 WeaponPanel Static Review 상태 세트

최소 다음 상태를 정적 캡처로 검토한다.

| ID | 상태 | 예상 Compact 구조 |
|---|---|---|
| `WP-SR-01` | Selected + Ready, 자원 Provider 없음 | 360×142 목표 / Secondary·Rail Collapse |
| `WP-SR-02` | Primary + Secondary 1개 | Selected 360×178 / Secondary 전체 폭 |
| `WP-SR-03` | Primary + Secondary 2개 | Selected 360×178 / 164·164 분할 |
| `WP-SR-04` | Rail 1개 | Selected + 1 Tile |
| `WP-SR-05` | Rail 3개 | Full 360×254 목표 |
| `WP-SR-06` | 비선택 4개 이상 | 2 Tile + `+N` Overflow |
| `WP-SR-07` | 긴 한글·영문 무기명 | Header 1줄 + 말줄임표 |
| `WP-SR-08` | Ammo 큰 수치 | `999 / 9999` 이상 폭 검토 |
| `WP-SR-09` | Cooldown·Reload 긴 상태 | `재사용 대기 99.9초` 등 1줄 유지 |
| `WP-SR-10` | Launcher Sequence | `RIPPLE 12 / 12` 등 진행 표시 |
| `WP-SR-11` | NoAmmo·Overheated·Disabled | Danger·Critical 형태 구분 |
| `WP-SR-12` | NoWeapon·Unavailable | 가짜 자원 행 없음 |

Static Review용 Mock View Data는 허용하지만 실제 Runtime 구현 증거로 해석하지 않는다.

### 16.2 Compact Token PASS 기준

- Full 기본 상태가 2560×1440에서 `360×254` 목표를 크게 초과하지 않는다.
- 모든 Content는 Maximum Slot `464×360` 안에 들어온다.
- 1080p에서 Rail 3개와 FireStateStrip이 잘리지 않는다.
- 긴 이름 때문에 Header가 2줄로 늘어나지 않는다.
- Secondary 0개에서 30 높이 Row와 Gap이 남지 않는다.
- Rail 0개에서 68 높이와 RailTopGap이 남지 않는다.
- 지원하지 않는 Ammo·Heat·Reload가 `N/A` 고정 행으로 나타나지 않는다.
- Primary Value와 Fire State가 Compact에서도 가장 먼저 읽힌다.
- 21:9·32:9에서 물리 화면 우측 끝으로 이동하지 않는다.

### 16.3 Compact Token FAIL 기준

- 기존 432×224 Selected Card를 사실상 그대로 사용해 화면 점유가 줄지 않음
- 3 Tile Rail이 Selected Card보다 넓어져 정렬이 깨짐
- Collapsed 항목 뒤 고정 Spacer가 남음
- 긴 문자열 때문에 Card 높이가 상태마다 크게 출렁임
- Primary·Secondary 숫자가 서로 같은 시각 무게로 보여 우선순위가 사라짐
- FireStateStrip이 Caption보다 약해 행동 차단 이유가 늦게 읽힘
- 1080p에서 Font가 유효 하한 아래로 내려감

### 16.4 3초 판독

플레이어가 3초 이내에 다음을 읽어야 한다.

```text
현재 선택 무기
지금 발사 가능한가
주 자원이 얼마나 남았는가
Cooldown·Reload·Heat·Sequence 중 무엇이 행동을 막는가
다른 무기 그룹이 존재하는가
```

### 16.5 1초 반응

다음 상태는 1초 이내에 인지돼야 한다.

```text
탄약 없음
충전 부족
과열
재장전 중
조준·총구 가림
Launcher Sequence 진행 중
무기 없음 또는 사용 불가
```

### 16.6 상태 조합 검토

```text
Ammo only
Ammo + Heat
Ammo + Reload
WeaponCharge + Heat
VehicleBattery + Heat
Cooldown only
Launcher Idle
Launcher Ripple / Salvo Active
NoAmmo / NoCharge / Overheated
AimBlocked / MuzzleBlocked
TurretAligning fire allowed / fire blocked
NoWeapon / Unavailable
비선택 0 / 1 / 2 / 3 / 4개 이상
```

### 16.7 실제 데이터 경계 검토

- 다중 무기 Provider가 없을 때 가짜 비선택 Tile이 나타나지 않는다.
- Ammo Runtime이 없을 때 `MagazineSize`가 현재 탄수로 표시되지 않는다.
- Heat Runtime이 없을 때 `HeatPerShot`가 현재 Heat로 표시되지 않는다.
- Reload Runtime이 없을 때 `ReloadTimeSeconds`가 진행 중 상태로 표시되지 않는다.
- 에너지 무기라는 이유만으로 Vehicle Battery를 표시하지 않는다.
- Launcher 내부 Projectile마다 전체 Cooldown이 다시 표시되지 않는다.
- 내부 ID·Asset 경로·Debug 문자열이 플레이어 HUD에 나타나지 않는다.

---

## 17. 승인된 Compact Visual Direction

사용자가 승인한 핵심 구조:

```text
1. 우하단 Maximum Slot 464×360과 Bottom Right Anchor 유지
2. 실제 Content는 Desired Size로 축소하고 Slot 전체를 채우지 않음
3. 이전 432×224 Card·136×96 Tile은 초기 넓은 Wireframe 참고값으로 전환
4. 현재 선택 무기는 Accent·Corner Marker·대형 주 자원으로 최강 강조
5. Primary Channel 1개 + Secondary Channel 최대 2개
6. FireStateStrip 한 줄
7. 비선택 무기 최대 3 Compact Tile
8. 4개 이상은 2개 + `+N` Overflow Tile
9. 지원되지 않는 자원 채널과 행은 Collapse하고 빈 공간도 제거
10. 탄약형·에너지형·런처형별 채널 우선순위
11. Style·Layout·Density Data와 교체 가능한 Blueprint Visual Widget 사용
```

현재 상태:

```text
D1-VEHICLE-PANEL Visual Layout: Accepted / 변경 없음
D1-WEAPON-PANEL Compact Visual Direction: Accepted
D1-WEAPON-PANEL Compact Token: User Accepted / Customizable Default Preset
D1-WEAPON-PANEL Compact Token User Review: Complete
D1-WEAPON-PANEL Runtime·UMG Implementation: current-runtime Technical Complete — Selected Resource + truthful Text Rail + Keyboard Weapon Select Input + WeaponCharge/Heat Runtime HUD / Rail USER Visual representative multi-weapon content 전까지 content-dependent Deferred / VehicleBattery는 external Gameplay dependency / Gamepad Input은 별도 확장
UI-DESIGN-GATE: D1-11 Production Structure PASS / D1-11-ART Visual Polish Deferred·Non-Blocking
```

---

## 18. Changelog

### v0.7.3 - 2026-08-19

- UI-P0-06 범위 교정에 따라 Production WeaponPanel을 current-runtime 기준 `Technical Complete`로 재분류했다. 현재 실제 Runtime이 존재하는 Ammo/Reserve/Reload/Cooldown/LauncherSequence/Heat/WeaponCharge와 Weapon Selection/Rail/Input이 Compact 계약에 연결돼 있으며 Provider 없는 VehicleBattery는 숨기는 것이 올바른 동작이다.
- VehicleBattery는 WeaponPanel의 미완성 기능이 아니라 external shared-power Gameplay dependency로 분리한다. Battery owner/capacity/regen/consumer priority가 별도 Gameplay feature에서 완성되면 기존 `VehicleBattery` ResourceChannel을 통해 표시한다.
- Rail USER Visual은 실제 persisted multi-weapon content가 생길 때까지 content-dependent Deferred이며 artificial fixture를 만들지 않는다. Heat/Charge tuning과 Redline/RPM Visual도 실제 authored content가 준비될 때 검증한다.
- 실행 코드·Widget Asset·WeaponData tuning mutation은 0이다.

### v0.7.2 - 2026-08-19

- WeaponCharge P0 Runtime/HUD Resource Projection Technical PASS를 현재 Production 계약에 반영했다. `MaximumWeaponCharge / InitialWeaponCharge / WeaponChargePerShot / WeaponChargeRecoveryPerSecond`가 유효할 때만 활성화하며 기존 all-zero WeaponData는 Disabled 호환 상태다.
- `UCFVehicleWeaponComp`가 선택 무기별 실제 Charge 상태, Game-Time 회복, next-shot 가능 여부와 accepted-shot 1회 소비를 소유한다. Pawn은 실제 fire validation에서 부족 시 `WeaponChargeInsufficient`를 사용한다. VehicleBattery를 Charge owner/source로 재해석하지 않는다.
- HUD는 actual Current/Maximum/Ratio/Insufficient만 `ResourceChannels::WeaponCharge` Percent로 전달한다. Ammo/Launcher가 없으면 `CHARGE N%` Primary, 기존 Primary가 있으면 Secondary이며 FireState는 `Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY`다. VehicleBattery와 정적 설정 fallback은 0이다.
- final Official Build `2d9f33261d3c427187f75338b8f37f1d` PASS, exact `WeaponChargeRuntimeResourceContract` `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS / Result SHA-256 `df53c9732b3135f9eae230dc9f287d646e542179c0464e5b2ef1794b54e8e137`다. closure source readback에서 Charge precheck/post-consume이 기존 동적 검증된 Heat fire hook과 같은 Pawn fire 함수의 인접 지점임을 확인해 별도 중복 protected-friend dynamic test는 만들지 않았다.
- Production/DataAsset Charge 값 authoring, VehicleBattery, Heat tuning, Redline authoring, Rail USER Visual 상태 변경은 0이다. Rail USER Visual Deferred와 artificial 2무기 fixture 금지를 유지한다.

### v0.7.1 - 2026-08-19

- Player-facing Keyboard Weapon Select Input을 Technical PASS로 반영했다. `IA_SelectWeapon` Axis1D와 숫자 1~9 direct ordinal을 사용하며 Pawn은 `Ordinal-1`만 기존 `RequestSelectWeaponIndex`에 전달한다. Mouse Wheel은 Radar Range/Zoom 예약을 유지하고 gamepad mapping은 임의 지정하지 않는다.
- final Build `dec0757b745342b4b90ff5f17b761eb0` PASS, persisted `IA_SelectWeapon` fp `1A8A0402`, `IMC_Vehicle_Default` fp `52D2D868` / mapping47→56, exact `WeaponSelectInputContract` `daeaf649669d48c79746a65cc44b1b89` 1/1 PASS다.
- Rail USER Visual은 현재 persisted representative가 모두 실제 선택 무기 1개라 Deferred했다. artificial fixture는 test-only mass-valid preset 재사용까지 확인했지만 finite-ammo loadout 작성으로 범위가 확대되어 중단했고 실패 생성 asset은 cleanup error0으로 삭제됐다. 실제 multi-weapon content가 생길 때만 USER Gate를 다시 연다.

### v0.7.0 - 2026-08-19

- 현재 Weapon Selection Runtime/HUD source Technical PASS를 반영해 `Applied Fitting weapon-bearing ResolvedMounts fixed order + SelectedWeaponIndex + EquipmentPresetData.DisplayName`을 현재 Production source로 승격했다. 내부 MountProfileId는 FireOrigin/Ammo identity only이며 UI identity로 쓰지 않는다.
- fresh persisted `WBP_CFWeaponPanel` fingerprint `4A873878`에서 기존 Rail이 Turret/Ammo/Reload semantic Image 3개뿐임을 확인하고 이를 실제 weapon slot으로 재해석하지 않았다.
- truthful P0 Rail은 현재 선택 무기를 Header/Selected Card에만 유지하고 비선택 무기만 표시한다. 비선택 0 hidden, 1~3 original ordinal+DisplayName, 4+ first2+`+N`; DisplayName 부재는 `WEAPON`이며 내부 ID/AssetName fallback은 없다.
- 실제 per-weapon icon과 비선택 resource summary source가 없으므로 과거 승인된 icon/Primary Summary/Marker는 Future richer Tile 확장으로 분리했다. 현재 Production은 112×68 Text Tile 3개 + 8px gap만 사용한다.
- Official Build `941ea18597b3483594be8de3317e68fd` PASS, targeted WeaponPanel-only apply `0617f7cbed97427a942c7a6f59936e22`가 exact asset1만 저장, fresh persisted fingerprint `26CFB205`, exact `WeaponRailVisualContract` `7cc9ddd5b2204725bb3ec45c200d649b` 1/1 PASS를 확보했다. USER Visual은 Pending이다.
- VehicleBattery/WeaponCharge Runtime, Heat tuning, Redline authoring은 변경하지 않았다. 실제 Weapon Select Input Mapping도 별도 Pending이다.

### v0.6.1 - 2026-08-13

- `ReserveAmmoCount` KnownZero는 미표시로 취급하지 않고 WeaponPanel 우상단에 라벨 없는 숫자 `0`으로 계속 표시하도록 계약을 명확히 했다.
- Primary `Loaded / MagazineCapacity`와 Reserve는 독립 채널이며 Reserve가 0이 되어도 Primary와 함께 가시 상태를 유지한다.
- Runtime Provider 자체가 Unavailable인 경우 채널을 숨기는 기존 규칙과 실제 KnownZero `0` 표시 규칙을 명확히 분리했다.

### v0.6.0 - 2026-08-13

- 사용자 기획 변경으로 finite Ammo Primary 표기를 `ImmediateUsable | CurrentUsable`에서 `LoadedAmmoCount / MagazineCapacity`로 변경했다.
- `ReserveAmmoCount`를 WeaponPanel 우상단의 라벨 없는 작은 숫자로 분리했다.
- Launcher 예약만으로 Loaded 표기가 감소하지 않으며 실제 Commit 때만 Loaded 숫자가 감소하는 규칙을 명시했다.
- `ImmediateUsableAmmoCount`와 `CurrentUsableAmmoCount`는 삭제하지 않고 내부 발사 가능·예약·회귀 판정값으로 유지하도록 확정했다.
- finite Ammo·Reload Runtime이 이미 존재하는 현재 구현 상태에 맞춰 Runtime 경계 설명을 갱신했다.

### v0.5.0 - 2026-08-07

- 사용자가 D1-05 WeaponPanel Compact Token 추천안을 전부 승인했다.
- 360 Preferred Width, 320 Minimum, 400 Maximum Content, Selected 142~178, Full 254, Tile 112×68, Gap 8과 Accent·Bar·Icon 기본값을 User Accepted Default Preset으로 승격했다.
- Primary Value의 `36` 독립 고정값을 제거하고 전역 `DisplayL × Compact PrimaryValueScale`, 기본 약 38로 연결했다.
- Compact Tile의 오래된 24px 아이콘 설명을 `CompactWeaponIconSize=18`로 통일했다.
- 승인된 모든 WeaponPanel 수치는 Density·Style Data에서 변경 가능하고 Blueprint Visual을 교체할 수 있는 기본값으로 확정했다.
- D1-05 사용자 검토를 Complete로 전환했으며 실제 UMG·Runtime·Font·Icon·Static Review는 Pending으로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.4.0 - 2026-08-07

- WeaponPanel Compact Direction을 실제 검토 가능한 Token 수준의 Detailed Draft로 확장했다.
- 기본 Content 폭 360, 최소 320, 최대 400과 Selected Card 142~178 높이, Full Rail 포함 254 높이 목표를 정의했다.
- Secondary 0·1·2개에 따른 Row Collapse와 336 전체 폭 또는 164+8+164 분할 규칙을 추가했다.
- Compact Tile을 112×68, Gap 8로 정의해 3개 Rail 폭을 352로 제한했다.
- Selected·Tile Icon, Bar 높이, Accent 두께, Corner Marker와 WeaponPanel 전용 Typography 초안을 정의했다.
- 1920×1080·2560×1440·3440×1440·5120×1440 해상도 규칙과 WP-SR-01~12 Static Review 상태 세트를 추가했다.
- Compact PASS·FAIL 기준을 문서화했으며 수치는 User Review Pending으로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.3.0 - 2026-08-07

- 사용자가 기존 WeaponPanel의 빈 공간과 화면 점유를 줄이는 Compact Revision 방향을 승인했다.
- `464×360`을 실제 고정 패널 크기가 아닌 우하단 최대 Slot 경계로 재정의하고 Content가 Desired Size로 수축하도록 변경했다.
- 기존 `432×224` Selected Card와 `136×96` Tile 수치를 초기 넓은 Wireframe 참고값으로 전환했다.
- 미지원 Secondary Channel과 Compact Rail은 `Collapsed` 처리해 행·간격·하단 여백까지 제거하도록 확정했다.
- 정확한 Compact Padding·Height·Tile·Font 수치는 Density Data Asset과 1080p·1440p Static Review가 소유하도록 남겼다.
- CarFight 전체 UI 커스터마이징 계약을 적용해 WeaponPanel을 교체 가능한 하위 Visual Widget, Style·Layout·Density Data와 User Override 구조로 분해했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.2.0 - 2026-08-07

- 사용자가 D1-WEAPON-PANEL 상세 구조를 승인해 `Visual Layout Accepted`로 승격했다.
- 우하단 `464×360` 투명 Root Slot, `432×224` Selected Card, Primary 1개·Secondary 최대 2개·FireStateStrip 구조를 최종 시각 계약으로 확정했다.
- 비선택 무기는 최대 3개 Compact Tile 또는 `2 + Overflow`로 축약하고, 지원되지 않는 Runtime 채널은 행 자체를 숨기는 규칙을 확정했다.
- Ammo·ReserveAmmo·VehicleBattery·WeaponCharge·Heat·Cooldown·Reload·LauncherSequence 공통 채널과 발사 상태 우선순위를 승인 범위에 포함했다.
- 탄약형·내부 충전형·차량 배터리형·런처형·복합형·Cooldown 중심 장비의 Primary·Secondary 표시 규칙을 유지한다.
- VehiclePanel `896×416`, Radar 위치와 중앙 Reticle Protection 영역은 변경하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았으며 Runtime·UMG 구현과 해상도 검증은 Pending이다.

### v0.1.0 - 2026-08-07

- 16:9 외부 3인칭 TPS 우하단 `464×360` WeaponPanel 상세 초안을 작성했다.
- 전체 Slot을 불투명 Panel로 채우지 않고 Selected Card와 조건부 Compact Rail만 Surface를 사용하는 구조를 정의했다.
- 선택 무기를 `432×224` 카드, 3px Accent, Corner Marker와 대형 Primary Channel로 가장 강하게 강조했다.
- 비선택 무기를 최대 3개의 `136×96` Compact Tile로 제한하고 4개 이상은 `2 + Overflow`로 축약했다.
- Ammo·ReserveAmmo·VehicleBattery·WeaponCharge·Heat·Cooldown·Reload·LauncherSequence 공통 채널을 정의했다.
- Primary 1개, Secondary 최대 2개와 FireStateStrip 한 줄의 정보 밀도 상한을 정했다.
- 탄약형·내부 충전형·차량 배터리형·런처형·복합형·Cooldown 중심 장비의 표시 규칙을 정의했다.
- 현재 실제 Runtime은 활성 무기 1개, 발사 결과·거부 사유, Cooldown과 Launcher Sequence이며 다중 무기·Ammo·Charge·Heat·Reload Runtime은 없음을 분리했다.
- 지원되지 않는 채널을 `0`이나 `N/A`로 채우지 않고 숨기도록 했다.
- 승인된 D1-VEHICLE-PANEL `896×416` 구조와 전체 HUD 배치는 변경하지 않았다.
- CommonUI 판단, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

---

## 19. Migration

- v0.7.3부터 current-runtime WeaponPanel Technical Complete는 VehicleBattery 부재로 되돌리지 않는다. VehicleBattery는 external Gameplay dependency이며 실제 Provider가 생기기 전 `Unavailable/Collapsed`를 유지한다. future Battery feature는 Presenter/Widget에 임시 전용 경로를 만들지 않고 기존 공통 ResourceChannel로 additive 연결한다.
- v0.7.2부터 WeaponCharge는 actual per-weapon Runtime이 활성일 때만 Production Resource로 표시한다. 기존 all-zero WeaponData를 자동 tuning하지 않고 VehicleBattery·Cooldown·Heat에서 Charge 현재값을 추정하지 않는다. 실제 Charge 무기를 authoring할 때 Maximum/Initial/PerShot/Recovery 네 값을 함께 결정하고 별도 USER Visual을 검토한다.
- `NO CHARGE`는 actual next-shot 부족 상태이며 FireState 우선순위는 `Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY`를 사용한다. VehicleBattery는 별도 shared-power Runtime이 생기기 전 숨긴다.
- v0.7.0부터 현재 Production Rail은 actual source가 있는 순번+DisplayName-only Text Tile을 authoritative P0 구현으로 사용한다. 과거 icon/Primary Summary/Marker 계약은 실제 per-weapon icon 및 비선택 resource summary source가 추가될 때까지 Future richer Tile 방향으로 해석한다.
- 현재 선택 무기는 Rail에 중복하지 않고 Header/Selected Card owner를 유지한다. internal MountProfileId/WeaponId/EquipmentId/AssetName과 기존 Turret/Ammo/Reload semantic icon을 weapon identity fallback으로 사용하지 않는다.
- Rail과 Keyboard Weapon Select Input Technical PASS는 관련 결함 없이 반복하지 않는다. Rail USER Visual은 representative persisted multi-weapon content가 생길 때만 재개한다. Gamepad mapping은 별도 조작 결정 전 임의 지정하지 않는다.
- v0.5.0부터 Compact Token은 User Accepted Default Preset이다. 승인된 수치를 Widget Blueprint Literal이나 C++ 상수로 고정하지 않고 Density·Style Data에서 해석한다.
- 기존 432×224·136×96 내부 치수 대신 360 Preferred Width, 142~178 Selected Height, 112×68 Tile과 자동 Collapse 구조를 기본 Preset으로 사용한다.
- Primary Value는 독립 36 고정값이 아니라 `DisplayL × Density.PrimaryValueScale`을 사용하며 Compact 기본은 약 38이다.
- Compact Tile Icon은 18 기본값을 사용하고 과거 24px 설명은 현재 기준이 아니다.
- 사용자 취향에 따른 크기·간격·아이콘·Bar 조정은 Data Asset 또는 명시적 Override로 가능해야 하며 Gameplay·Presenter 계약은 유지한다.
- 1080p는 Geometry 0.75 환산과 별개로 WeaponPanel Typography 유효 하한을 적용한다.
- `WP-SR-01~12`는 시각 검토용 상태 세트이며 Runtime 기능 완료 증거가 아니다.
- 기존 `CFHUDWireframe_1440p.xml`의 WeaponPanel 내부 예시는 배치 확인용 Placeholder로 해석한다.
- v0.3.0부터 `CFWeaponPanel_1440p.xml`은 초기 넓은 배치 참고 자료이며 Compact Revision의 최종 Pixel SSOT가 아니다.
- D1-WEAPON-PANEL 상세 구현은 이 문서 v0.3.0의 Compact·Customization 계약을 우선하고 정확한 수치는 Density Static Review에서 확정한다.
- 승인된 Visual Layout은 향후 Source·Unreal Asset 제작의 입력 계약이지만 현재 구현 완료를 의미하지 않는다.
- 첫 Runtime 연결에서 다중 무기 목록이 없으면 Selected Card만 표시하고 Compact Rail은 숨긴다.
- Ammo·WeaponCharge·Heat·Reload는 실제 Runtime source가 있을 때만 해당 채널을 표시한다. VehicleBattery는 실제 shared-power Runtime이 추가되기 전 표시하지 않는다.
- 현재 Cooldown·FireFeedback·Launcher Sequence의 Gameplay 판정과 우선순위를 UI에서 재계산하지 않는다.
- 승인된 VehiclePanel `896×416`, Radar 위치와 중앙 Reticle Protection 영역은 변경하지 않는다.
- 신규 WeaponPanel Widget은 Style·Layout·Density Data와 명시적 Override를 소비하며 색상·폰트·Padding·Tile Size를 개별 그래프에 고정하지 않는다.
- 기본 WeaponPanel Visual Asset은 사용자 복제·상속·교체가 가능한 Preset으로 제공한다.
